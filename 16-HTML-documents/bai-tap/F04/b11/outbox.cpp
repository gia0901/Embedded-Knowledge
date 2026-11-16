// outbox: store-and-forward trên gateway. Số đo ghi vào bảng pending của gw.db (SQLite, Bài 9) trước,
// rồi một vòng gửi lấy từng lô theo thứ tự id, ghi lên fleet (PostgreSQL) bằng MỘT câu INSERT cho cả lô,
// và chỉ xoá khỏi pending SAU KHI backend đã commit. Mất mạng: số đo nằm lại trong pending, gửi sau.
//   ./outbox collect N          thêm N số đo vào pending (thiết bị 1..190, mỗi phút một số đo từ 17/11 00:00)
//   ./outbox send [B]           gửi theo lô B dòng (mặc định 500): INSERT ... ON CONFLICT DO NOTHING
//   ./outbox send B crash       như trên nhưng chết ngay sau khi backend commit lô đầu, trước khi xoá cục bộ
//   ./outbox send B plain       INSERT không có ON CONFLICT
// Gửi lại một lô là bình thường (at-least-once); khoá chính (device_id, ts) cộng ON CONFLICT DO NOTHING làm nó vô hại.
// Build: g++ -O2 -std=c++20 -Wall -I/usr/include/postgresql -o outbox outbox.cpp -lsqlite3 -lpq
// Chạy:  PGHOST=127.0.0.1 PGDATABASE=fleet PGUSER=fleet_app PGPASSWORD=lab ./outbox ...
#include <libpq-fe.h>
#include <sqlite3.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static sqlite3* db;
static void lexec(const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) { fprintf(stderr, "sqlite: %s\n", err); exit(1); }
}
static long lcount() {
    sqlite3_stmt* st; sqlite3_prepare_v2(db, "SELECT count(*) FROM pending", -1, &st, nullptr);
    sqlite3_step(st); long n = sqlite3_column_int64(st, 0); sqlite3_finalize(st); return n;
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s collect N | send [B] [crash|plain]\n", argv[0]); return 1; }
    sqlite3_open("gw.db", &db);
    lexec("PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL; PRAGMA busy_timeout=2000");
    std::string cmd = argv[1];

    if (cmd == "collect") {
        long n = argc > 2 ? atol(argv[2]) : 1000;
        sqlite3_stmt* st;
        sqlite3_prepare_v2(db, "INSERT INTO pending(device_id, ts, temp) VALUES (?, ?, ?)", -1, &st, nullptr);
        lexec("BEGIN");
        for (long i = 0; i < n; i++) {
            sqlite3_bind_int(st, 1, 1 + i % 190);
            sqlite3_bind_int64(st, 2, 1794848400 + (i / 190) * 60);   // 17/11/2026 00:00 +07, mỗi phút một lượt
            sqlite3_bind_double(st, 3, 25.0 + (i % 50) / 10.0);
            sqlite3_step(st); sqlite3_reset(st);
        }
        lexec("COMMIT");
        sqlite3_finalize(st);
        printf("pending: %ld ban ghi\n", lcount());
        return 0;
    }
    if (cmd != "send") { fprintf(stderr, "lenh khong ro: %s\n", argv[1]); return 1; }
    int B = argc > 2 ? atoi(argv[2]) : 500;
    std::string opt = argc > 3 ? argv[3] : "";

    PGconn* pg = PQconnectdb("connect_timeout=2");
    if (PQstatus(pg) != CONNECTION_OK) {                    // mất mạng: không mất gì, số đo vẫn chờ trong pending
        printf("khong ket noi duoc backend, giu %ld ban ghi trong pending\n", lcount());
        return 2;
    }
    // Cả lô đi trong một câu: ba mảng tham số, unnest thành các dòng.
    std::string sql = "INSERT INTO readings(device_id, ts, temp) "
                      "SELECT d, to_timestamp(t), v FROM unnest($1::int[], $2::bigint[], $3::real[]) AS x(d, t, v)";
    if (opt != "plain") sql += " ON CONFLICT (device_id, ts) DO NOTHING";
    sqlite3_stmt* sel;
    sqlite3_prepare_v2(db, "SELECT id, device_id, ts, temp FROM pending ORDER BY id LIMIT ?", -1, &sel, nullptr);
    long batches = 0, sent = 0, inserted = 0;
    auto t0 = std::chrono::steady_clock::now();
    for (;;) {
        std::string ds = "{", ts = "{", vs = "{";
        long last = -1; int n = 0;
        sqlite3_bind_int(sel, 1, B);
        while (sqlite3_step(sel) == SQLITE_ROW) {
            if (n++) { ds += ','; ts += ','; vs += ','; }
            last = sqlite3_column_int64(sel, 0);
            ds += std::to_string(sqlite3_column_int(sel, 1));
            ts += std::to_string(sqlite3_column_int64(sel, 2));
            vs += std::to_string(sqlite3_column_double(sel, 3));
        }
        sqlite3_reset(sel);
        if (n == 0) break;
        ds += '}'; ts += '}'; vs += '}';
        const char* p[3] = {ds.c_str(), ts.c_str(), vs.c_str()};
        PGresult* r = PQexecParams(pg, sql.c_str(), 3, nullptr, p, nullptr, nullptr, 0);   // một transaction, một commit
        if (PQresultStatus(r) != PGRES_COMMAND_OK) {        // backend từ chối: giữ lô, dừng, thử lại lần sau
            printf("lo %ld bi tu choi: %s", batches + 1, PQresultErrorMessage(r));
            PQclear(r); break;
        }
        inserted += atol(PQcmdTuples(r)); sent += n; batches++;
        PQclear(r);
        if (opt == "crash") { printf("chet sau khi backend commit lo 1 (%d dong), chua xoa cuc bo\n", n); fflush(stdout); _exit(9); }
        std::string del = "DELETE FROM pending WHERE id <= " + std::to_string(last);   // chỉ xoá sau khi backend đã commit
        lexec(del.c_str());
    }
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    printf("gui %ld lo, %ld dong, backend them moi %ld, bo qua trung %ld, con pending %ld, %.0f ms\n",
           batches, sent, inserted, sent - inserted, lcount(), ms);
    sqlite3_finalize(sel); sqlite3_close(db); PQfinish(pg);
}
