// bufw: gateway ghi số đo vào hàng đợi cục bộ SQLite (buf.db), mỗi transaction một lô B dòng.
// Build: g++ -O2 -std=c++20 -Wall -o bufw bufw.cpp -lsqlite3
// Chạy:  ./bufw B N       (ghi N dòng, lô B dòng mỗi COMMIT; in số dòng mỗi giây)
#include <sqlite3.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s B N\n", argv[0]); return 1; }
    int B = atoi(argv[1]), N = atoi(argv[2]);
    sqlite3* db;
    sqlite3_open("buf.db", &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS q(id INTEGER PRIMARY KEY, dev INT, seq INT, temp REAL)", 0, 0, 0);
    sqlite3_stmt* st;
    sqlite3_prepare_v2(db, "INSERT INTO q(dev, seq, temp) VALUES(?, ?, ?)", -1, &st, 0);
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) {
        if (i % B == 0) sqlite3_exec(db, "BEGIN", 0, 0, 0);
        sqlite3_bind_int(st, 1, i % 7); sqlite3_bind_int(st, 2, i); sqlite3_bind_double(st, 3, 25.0 + i % 10);
        sqlite3_step(st); sqlite3_reset(st);
        if (i % B == B - 1 || i == N - 1) sqlite3_exec(db, "COMMIT", 0, 0, 0);
    }
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    printf("lo %4d: %d dong trong %.2f s = %.0f dong/s\n", B, N, s, N / s);
    sqlite3_finalize(st);
    sqlite3_close(db);
}
