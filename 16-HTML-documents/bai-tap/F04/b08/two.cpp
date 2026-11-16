// two: hai phiên A và B chạy xen kẽ theo một kịch bản cố định, in từng bước như hai cửa sổ psql.
//   deadlock   A sửa thiết bị 1 rồi 2, B sửa 2 rồi 1: chờ vòng, PostgreSQL huỷ một bên
//   ordered    cả hai sửa theo thứ tự id tăng dần: B chỉ chờ, không deadlock
//   skew LEVEL quy tắc "mỗi thiết bị tối đa một lệnh update_fw chưa gửi"; A và B cùng kiểm tra rồi cùng chèn
//              LEVEL: "read committed" | "repeatable read" | "serializable"
// Câu có thể bị chặn được gửi bất đồng bộ (PQsendQuery) để kịch bản đi tiếp; "..." là đang chờ.
// Build: g++ -O2 -std=c++20 -Wall -I/usr/include/postgresql -o two two.cpp -lpq
// Chạy:  PGHOST=127.0.0.1 PGDATABASE=fleet PGUSER=fleet_app PGPASSWORD=lab ./two deadlock|ordered|skew LEVEL
#include <libpq-fe.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

static PGconn *A, *B;
static auto t0 = std::chrono::steady_clock::now();
static double now_s() { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); }
static const char* name(PGconn* c) { return c == A ? "A" : "B"; }

// In kết quả của một câu: số dòng, giá trị một ô, hoặc lỗi kèm SQLSTATE.
static void show(PGconn* c, PGresult* r) {
    ExecStatusType s = PQresultStatus(r);
    if (s == PGRES_TUPLES_OK && PQntuples(r) == 1 && PQnfields(r) == 1)
        printf("%5.2fs %s  -> %s\n", now_s(), name(c), PQgetvalue(r, 0, 0));
    else if (s == PGRES_TUPLES_OK || s == PGRES_COMMAND_OK)
        printf("%5.2fs %s  -> %s\n", now_s(), name(c), PQcmdStatus(r));
    else {
        std::string m = PQresultErrorField(r, PG_DIAG_MESSAGE_PRIMARY);
        printf("%5.2fs %s  -> ERROR %s: %s\n", now_s(), name(c), PQresultErrorField(r, PG_DIAG_SQLSTATE), m.c_str());
    }
}
static void step(PGconn* c, const std::string& sql) {               // chạy và chờ kết quả
    printf("%5.2fs %s> %s\n", now_s(), name(c), sql.c_str());
    PGresult* r = PQexec(c, sql.c_str()); show(c, r); PQclear(r);
}
static void send(PGconn* c, const std::string& sql) {               // gửi, không chờ
    printf("%5.2fs %s> %s ...\n", now_s(), name(c), sql.c_str());
    PQsendQuery(c, sql.c_str());
}
static void collect(PGconn* c) {                                     // chờ kết quả của câu đã gửi
    while (PGresult* r = PQgetResult(c)) { show(c, r); PQclear(r); }
}
static bool waiting(PGconn* c) { PQconsumeInput(c); return PQisBusy(c); }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s deadlock|ordered|skew LEVEL\n", argv[0]); return 1; }
    std::string mode = argv[1];
    A = PQconnectdb(""); B = PQconnectdb("");
    if (PQstatus(A) != CONNECTION_OK || PQstatus(B) != CONNECTION_OK) { fprintf(stderr, "%s", PQerrorMessage(A)); return 1; }
    PQclear(PQexec(A, "SET client_min_messages = warning"));
    PQclear(PQexec(B, "SET client_min_messages = warning"));

    if (mode == "deadlock" || mode == "ordered") {
        bool ord = mode == "ordered";
        const char* u1 = "UPDATE devices SET last_seen = now() WHERE id = 1";
        const char* u2 = "UPDATE devices SET last_seen = now() WHERE id = 2";
        step(A, "BEGIN"); step(B, "BEGIN");
        step(A, u1);
        if (ord) send(B, u1); else step(B, u2);                       // ordered: B cũng lấy dòng 1 trước, phải chờ A
        if (!ord) send(A, u2);                                         // A chờ dòng 2 mà B giữ
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        printf("%5.2fs    A dang cho: %s, B dang cho: %s\n", now_s(), waiting(A) ? "co" : "khong", waiting(B) ? "co" : "khong");
        if (!ord) {
            send(B, u1);                                               // B chờ dòng 1 mà A giữ: vòng chờ
            collect(A); collect(B);
            step(A, "COMMIT"); step(B, "COMMIT");
        } else {
            step(A, u2); step(A, "COMMIT");                            // A xong, nhả khoá: B chạy tiếp
            collect(B); step(B, u2); step(B, "COMMIT");
        }
    } else if (mode == "skew") {
        std::string lvl = argc > 2 ? argv[2] : "read committed";
        PQclear(PQexec(A, "DELETE FROM commands WHERE device_id = 7 AND kind = 'update_fw'"));
        const char* chk = "SELECT count(*) FROM commands WHERE device_id = 7 AND kind = 'update_fw' AND sent_at IS NULL";
        const char* ins = "INSERT INTO commands(device_id, kind, payload) VALUES (7, 'update_fw', '{\"to\": \"2.2.0\"}')";
        step(A, "BEGIN ISOLATION LEVEL " + lvl); step(B, "BEGIN ISOLATION LEVEL " + lvl);
        step(A, chk); step(B, chk);                                    // cả hai thấy 0: được phép chèn
        step(A, ins);
        send(B, ins);                                                  // có UNIQUE index thì B phải chờ A
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        printf("%5.2fs    B dang cho: %s\n", now_s(), waiting(B) ? "co" : "khong");
        step(A, "COMMIT"); collect(B); step(B, "COMMIT");
        PGresult* r = PQexec(A, chk);
        printf("ket qua: thiet bi 7 co %s lenh update_fw chua gui\n", PQgetvalue(r, 0, 0));
        PQclear(r);
    } else { fprintf(stderr, "che do khong ro: %s\n", mode.c_str()); return 1; }
    PQfinish(A); PQfinish(B);
}
