// counter: tăng một bộ đếm N lần theo kiểu đọc, cộng, ghi. Chạy hai tiến trình cùng lúc để thấy lost update.
//   --file   bộ đếm là một số trong file count.txt (đọc cả file, cộng 1, ghi đè)
//   --sqlite bộ đếm là một dòng trong counter.db, mỗi lần một câu UPDATE n = n + 1
// Build: g++ -O2 -std=c++20 -Wall -o counter counter.cpp -lsqlite3
// Chạy:  ./counter --file|--sqlite N
#include <sqlite3.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s --file|--sqlite N\n", argv[0]); return 1; }
    int n = atoi(argv[2]);
    if (!strcmp(argv[1], "--file")) {
        for (int i = 0; i < n; i++) {
            long v = 0;
            if (FILE* f = fopen("count.txt", "r")) { if (fscanf(f, "%ld", &v) != 1) v = 0; fclose(f); }
            FILE* f = fopen("count.txt", "w");
            fprintf(f, "%ld\n", v + 1);
            fclose(f);
        }
    } else {
        sqlite3* db;
        sqlite3_open("counter.db", &db);
        sqlite3_busy_timeout(db, 5000);                    // bị khoá thì chờ tới 5 s thay vì báo lỗi ngay
        for (int i = 0; i < n; i++)
            if (sqlite3_exec(db, "UPDATE c SET n = n + 1", 0, 0, 0) != SQLITE_OK) { fprintf(stderr, "%s\n", sqlite3_errmsg(db)); return 1; }
        sqlite3_close(db);
    }
}
