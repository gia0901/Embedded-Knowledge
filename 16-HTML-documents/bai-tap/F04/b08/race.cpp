// race: T luồng, mỗi luồng một kết nối PostgreSQL, cùng tăng một bộ đếm N lần.
// Bộ đếm: số lượt tải bản firmware 2.2.0, bảng fw_downloads(version, n, ver).
//   naive   đọc n, tính n+1 ở ứng dụng, ghi lại giá trị tuyệt đối (read committed, autocommit)
//   atomic  UPDATE ... SET n = n + 1: database tự đọc và ghi trong một câu
//   lock    BEGIN; SELECT ... FOR UPDATE; UPDATE n = giá trị đọc + 1; COMMIT   (khoá bi quan)
//   cas     đọc n và ver; UPDATE ... WHERE ver = ver đã đọc; 0 dòng thì đọc lại, thử lại (khoá lạc quan)
//   rr      như naive nhưng trong BEGIN ISOLATION LEVEL REPEATABLE READ; lỗi 40001 thì thử lại
// In giá trị cuối, số lần tăng bị mất, số lần thử lại, và thời gian.
// Build: g++ -O2 -std=c++20 -Wall -pthread -I/usr/include/postgresql -o race race.cpp -lpq
// Chạy:  PGHOST=127.0.0.1 PGDATABASE=fleet PGUSER=fleet_app PGPASSWORD=lab ./race MODE T N
#include <libpq-fe.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

static std::atomic<long> retries{0};

// Chạy một câu; trả về kết quả, hoặc nullptr nếu lỗi là xung đột tuần tự hoá (40001) để người gọi thử lại.
static PGresult* run(PGconn* c, const char* sql, int n = 0, const char* const* p = nullptr) {
    PGresult* r = PQexecParams(c, sql, n, nullptr, p, nullptr, nullptr, 0);
    ExecStatusType s = PQresultStatus(r);
    if (s == PGRES_COMMAND_OK || s == PGRES_TUPLES_OK) return r;
    const char* code = PQresultErrorField(r, PG_DIAG_SQLSTATE);
    if (code && !strcmp(code, "40001")) { PQclear(r); return nullptr; }
    fprintf(stderr, "loi: %s", PQresultErrorMessage(r)); exit(1);
}
static void exec(PGconn* c, const char* sql) { if (PGresult* r = run(c, sql)) PQclear(r); }
static std::string get1(PGresult* r, int col = 0) { std::string v = PQgetvalue(r, 0, col); PQclear(r); return v; }

static void inc(PGconn* c, const std::string& mode) {
    if (mode == "atomic") {
        exec(c, "UPDATE fw_downloads SET n = n + 1 WHERE version = '2.2.0'");
    } else if (mode == "naive") {
        std::string v = std::to_string(std::stol(get1(run(c, "SELECT n FROM fw_downloads WHERE version = '2.2.0'"))) + 1);
        const char* p[1] = {v.c_str()};
        PQclear(run(c, "UPDATE fw_downloads SET n = $1 WHERE version = '2.2.0'", 1, p));
    } else if (mode == "lock") {
        exec(c, "BEGIN");
        std::string v = std::to_string(std::stol(get1(run(c, "SELECT n FROM fw_downloads WHERE version = '2.2.0' FOR UPDATE"))) + 1);
        const char* p[1] = {v.c_str()};
        PQclear(run(c, "UPDATE fw_downloads SET n = $1 WHERE version = '2.2.0'", 1, p));
        exec(c, "COMMIT");
    } else if (mode == "cas") {
        for (;;) {
            PGresult* r = run(c, "SELECT n, ver FROM fw_downloads WHERE version = '2.2.0'");
            std::string n = std::to_string(std::stol(PQgetvalue(r, 0, 0)) + 1), ver = PQgetvalue(r, 0, 1);
            PQclear(r);
            const char* p[2] = {n.c_str(), ver.c_str()};
            r = run(c, "UPDATE fw_downloads SET n = $1, ver = ver + 1 WHERE version = '2.2.0' AND ver = $2", 2, p);
            bool done = !strcmp(PQcmdTuples(r), "1");      // số dòng bị sửa: 0 nghĩa là ai đó đã đổi ver
            PQclear(r);
            if (done) return;
            retries++;
        }
    } else if (mode == "rr") {
        for (;;) {
            exec(c, "BEGIN ISOLATION LEVEL REPEATABLE READ");
            PGresult* r = run(c, "SELECT n FROM fw_downloads WHERE version = '2.2.0'");
            std::string v = std::to_string(std::stol(get1(r)) + 1);
            const char* p[1] = {v.c_str()};
            r = run(c, "UPDATE fw_downloads SET n = $1 WHERE version = '2.2.0'", 1, p);
            if (r) { PQclear(r); if ((r = run(c, "COMMIT"))) { PQclear(r); return; } }
            exec(c, "ROLLBACK");                            // 40001: transaction đã hỏng, huỷ rồi làm lại từ đầu
            retries++;
        }
    } else { fprintf(stderr, "che do khong ro: %s\n", mode.c_str()); exit(1); }
}

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s naive|atomic|lock|cas|rr THREADS N\n", argv[0]); return 1; }
    std::string mode = argv[1];
    int T = atoi(argv[2]), N = atoi(argv[3]);
    PGconn* c = PQconnectdb("");
    if (PQstatus(c) != CONNECTION_OK) { fprintf(stderr, "%s", PQerrorMessage(c)); return 1; }
    exec(c, "UPDATE fw_downloads SET n = 0, ver = 0 WHERE version = '2.2.0'");
    auto t0 = std::chrono::steady_clock::now();
    std::vector<std::thread> th;
    for (int t = 0; t < T; t++) th.emplace_back([&] {
        PGconn* k = PQconnectdb("");
        if (PQstatus(k) != CONNECTION_OK) { fprintf(stderr, "%s", PQerrorMessage(k)); exit(1); }
        for (int i = 0; i < N; i++) inc(k, mode);
        PQfinish(k);
    });
    for (auto& x : th) x.join();
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    long n = std::stol(get1(run(c, "SELECT n FROM fw_downloads WHERE version = '2.2.0'")));
    long want = (long)T * N;
    printf("%-6s %2d luong: n = %6ld / %6ld, mat %5ld, thu lai %5ld, %6.0f ms\n",
           mode.c_str(), T, n, want, want - n, retries.load(), ms);
    PQfinish(c);
}
