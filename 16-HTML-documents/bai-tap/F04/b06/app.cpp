// app: hai lỗi hiệu năng hay gặp ở phía ứng dụng, đo qua TCP tới PostgreSQL.
//   n1      lấy danh sách thiết bị, rồi mỗi thiết bị một câu lấy số đo mới nhất (N+1)
//   one     cùng kết quả bằng một câu (LATERAL)
//   offset  xuất cả bảng readings theo trang 1000 dòng: LIMIT 1000 OFFSET k
//   keyset  cùng việc đó: WHERE (device_id, ts) > (khoá cuối của trang trước) LIMIT 1000
// In số câu đã gửi (mỗi câu một lượt đi về), số dòng, một tổng kiểm tra, và thời gian.
// Build: g++ -O2 -std=c++20 -Wall -I/usr/include/postgresql -o app app.cpp -lpq
// Chạy:  PGHOST=127.0.0.1 PGDATABASE=fleet PGUSER=fleet_app PGPASSWORD=lab ./app n1|one|offset|keyset
#include <libpq-fe.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static PGconn* c;
static int queries = 0;

static PGresult* q(const char* sql, int n = 0, const char* const* params = nullptr) {
    queries++;
    PGresult* r = PQexecParams(c, sql, n, nullptr, params, nullptr, nullptr, 0);
    if (PQresultStatus(r) != PGRES_TUPLES_OK) { fprintf(stderr, "loi: %s", PQresultErrorMessage(r)); exit(1); }
    return r;
}
static double ms_since(std::chrono::steady_clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s n1|one|offset|keyset\n", argv[0]); return 1; }
    c = PQconnectdb("");
    if (PQstatus(c) != CONNECTION_OK) { fprintf(stderr, "%s", PQerrorMessage(c)); return 1; }
    std::string mode = argv[1];
    long rows = 0; double sum = 0, last_page = 0;
    auto t0 = std::chrono::steady_clock::now();

    if (mode == "n1") {
        PGresult* ids = q("SELECT id FROM devices ORDER BY id");
        for (int i = 0; i < PQntuples(ids); i++) {
            const char* p[1] = {PQgetvalue(ids, i, 0)};
            PGresult* r = q("SELECT ts, temp FROM readings WHERE device_id = $1 ORDER BY ts DESC LIMIT 1", 1, p);
            if (PQntuples(r)) { rows++; sum += atof(PQgetvalue(r, 0, 1)); }
            PQclear(r);
        }
        PQclear(ids);
    } else if (mode == "one") {
        PGresult* r = q("SELECT d.id, x.ts, x.temp FROM devices d CROSS JOIN LATERAL "
                        "(SELECT ts, temp FROM readings WHERE device_id = d.id ORDER BY ts DESC LIMIT 1) x");
        for (int i = 0; i < PQntuples(r); i++) { rows++; sum += atof(PQgetvalue(r, i, 2)); }
        PQclear(r);
    } else if (mode == "offset" || mode == "keyset") {
        std::string dev, ts;                          // khoá của dòng cuối trang trước
        for (long off = 0;; off += 1000) {
            auto tp = std::chrono::steady_clock::now();
            PGresult* r;
            if (mode == "offset") {
                std::string o = std::to_string(off);
                const char* p[1] = {o.c_str()};
                r = q("SELECT device_id, ts, temp FROM readings ORDER BY device_id, ts LIMIT 1000 OFFSET $1", 1, p);
            } else if (off == 0) {
                r = q("SELECT device_id, ts, temp FROM readings ORDER BY device_id, ts LIMIT 1000");
            } else {
                const char* p[2] = {dev.c_str(), ts.c_str()};
                r = q("SELECT device_id, ts, temp FROM readings WHERE (device_id, ts) > ($1, $2) "
                      "ORDER BY device_id, ts LIMIT 1000", 2, p);
            }
            int n = PQntuples(r);
            for (int i = 0; i < n; i++) sum += atof(PQgetvalue(r, i, 2));
            rows += n;
            if (n) { dev = PQgetvalue(r, n - 1, 0); ts = PQgetvalue(r, n - 1, 1); }
            PQclear(r);
            last_page = ms_since(tp);
            if (n < 1000) break;
        }
    } else { fprintf(stderr, "che do khong ro: %s\n", argv[1]); return 1; }

    printf("%-6s %4d cau, %6ld dong, tong temp %.2f, %7.1f ms", mode.c_str(), queries, rows, sum, ms_since(t0));
    if (last_page > 0) printf(", trang cuoi %.2f ms", last_page);
    printf("\n");
    PQfinish(c);
}
