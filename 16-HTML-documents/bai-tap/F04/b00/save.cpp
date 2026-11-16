// save: lưu danh sách 2000 thiết bị theo hai cách, chậm có chủ ý để kịp kill -9 giữa chừng.
//   --json   ghi đè cả file devices.json (mở với O_TRUNC rồi ghi từng dòng), cách "tự lưu file" hay gặp
//   --sqlite UPDATE 2000 dòng trong devices.db trong một transaction
// Build: g++ -O2 -std=c++20 -Wall -o save save.cpp -lsqlite3
// Chạy:  ./save --json|--sqlite VERSION      (mỗi thiết bị nhận firmware = VERSION; ~1 ms mỗi thiết bị)
#include <sqlite3.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s --json|--sqlite VERSION\n", argv[0]); return 1; }
    std::string ver = argv[2];
    const int N = 2000;
    if (!strcmp(argv[1], "--json")) {
        FILE* f = fopen("devices.json", "w");             // "w" = O_TRUNC: file cũ mất ngay tại đây
        fprintf(f, "[\n");
        for (int i = 0; i < N; i++) {
            fprintf(f, "  {\"id\": \"dev-%04d\", \"fw\": \"%s\"}%s\n", i, ver.c_str(), i + 1 < N ? "," : "");
            fflush(f);                                     // để thấy file lớn dần
            usleep(1000);
        }
        fprintf(f, "]\n");
        fclose(f);
    } else {
        sqlite3* db;
        sqlite3_open("devices.db", &db);
        sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS devices(id TEXT PRIMARY KEY, fw TEXT)", 0, 0, 0);
        sqlite3_exec(db, "BEGIN", 0, 0, 0);
        sqlite3_stmt* st;
        sqlite3_prepare_v2(db, "INSERT INTO devices VALUES(?1, ?2) ON CONFLICT(id) DO UPDATE SET fw = ?2", -1, &st, 0);
        char id[16];
        for (int i = 0; i < N; i++) {
            snprintf(id, sizeof id, "dev-%04d", i);
            sqlite3_bind_text(st, 1, id, -1, SQLITE_STATIC);
            sqlite3_bind_text(st, 2, ver.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(st); sqlite3_reset(st);
            usleep(1000);
        }
        sqlite3_finalize(st);
        sqlite3_exec(db, "COMMIT", 0, 0, 0);
        sqlite3_close(db);
    }
    printf("da luu %d thiet bi, fw %s\n", N, ver.c_str());
}
