// wear: firmware lưu bộ đếm năng lượng mỗi giây một lần. Đo số byte THẬT xuống thiết bị lưu trữ cho mỗi lần lưu,
// đọc từ bộ đếm sector của chính thiết bị chứa thư mục hiện tại (/sys/dev/block/MAJ:MIN/stat, cột 7).
//   sqlite  SQLite WAL, synchronous=FULL, mỗi lần lưu một UPDATE (một transaction)
//   minute  giữ bộ đếm trong RAM, cứ 60 lần mới UPDATE một lần (mất tối đa 1 phút khi mất điện)
//   log     log nối đuôi: bản ghi 16 byte (seq, giá trị, CRC32) + fdatasync mỗi lần
//   ring    như log nhưng ghi vòng trong một file 64 KB đã cấp sẵn: kích thước file không đổi
//   lmdb    LMDB, mỗi lần lưu một transaction ghi (copy-on-write, sync khi commit)
// Build: g++ -O2 -std=c++20 -Wall -o wear wear.cpp -lsqlite3 -llmdb
// Chạy:  ./wear sqlite|minute|log|ring|lmdb N
#include <fcntl.h>
#include <lmdb.h>
#include <sqlite3.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

static long long sectors_written() {                 // cột 7 của .../stat: số sector 512 byte đã ghi
    struct stat st; stat(".", &st);
    char p[64]; snprintf(p, sizeof p, "/sys/dev/block/%u:%u/stat", major(st.st_dev), minor(st.st_dev));
    std::ifstream f(p); long long v[7] = {};
    for (auto& x : v) f >> x;
    return v[6];
}
static uint32_t crc32(const void* p, size_t n) {
    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < n; i++) { c ^= ((const uint8_t*)p)[i]; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320 & -(c & 1)); }
    return ~c;
}
static void ck(int rc, const char* what) { if (rc) { fprintf(stderr, "%s: loi %d\n", what, rc); exit(1); } }

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s sqlite|minute|log|ring|lmdb N\n", argv[0]); return 1; }
    std::string mode = argv[1]; long N = atol(argv[2]);
    if (system("rm -rf wear.db wear.db-wal wear.db-shm wear.log wear.mdb")) return 1;
    sqlite3* db = nullptr; sqlite3_stmt* st = nullptr; int fd = -1; MDB_env* env = nullptr; MDB_dbi dbi = 0;
    if (mode == "sqlite" || mode == "minute") {                 // chuẩn bị trước khi đo
        sqlite3_open("wear.db", &db);
        sqlite3_exec(db, "PRAGMA journal_mode=WAL; PRAGMA synchronous=FULL;"
                         "CREATE TABLE counters(name TEXT PRIMARY KEY, n INTEGER NOT NULL);"
                         "INSERT INTO counters VALUES ('energy_wh', 0);", nullptr, nullptr, nullptr);
        sqlite3_prepare_v2(db, "UPDATE counters SET n = ? WHERE name = 'energy_wh'", -1, &st, nullptr);
    } else if (mode == "log") {
        fd = open("wear.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else if (mode == "ring") {
        fd = open("wear.log", O_WRONLY | O_CREAT, 0644);
        static char zero[65536];
        if (write(fd, zero, sizeof zero) != sizeof zero || fsync(fd)) { perror("ring"); return 1; }
    } else if (mode == "lmdb") {
        mkdir("wear.mdb", 0755);
        ck(mdb_env_create(&env), "env"); mdb_env_set_mapsize(env, 16 << 20);
        ck(mdb_env_open(env, "wear.mdb", 0, 0644), "open");
        MDB_txn* t; mdb_txn_begin(env, nullptr, 0, &t); mdb_dbi_open(t, nullptr, 0, &dbi); mdb_txn_commit(t);
    } else { fprintf(stderr, "che do khong ro: %s\n", argv[1]); return 1; }

    sync();
    long long s0 = sectors_written();
    long saves = 0;
    for (long i = 1; i <= N; i++) {                              // i = giá trị bộ đếm ở giây thứ i
        if (mode == "sqlite" || (mode == "minute" && i % 60 == 0)) {
            sqlite3_bind_int64(st, 1, i); ck(sqlite3_step(st) != SQLITE_DONE, "update"); sqlite3_reset(st); saves++;
        } else if (mode == "log" || mode == "ring") {
            struct { uint64_t seq; uint32_t val; uint32_t crc; } r{(uint64_t)i, (uint32_t)i, 0};
            r.crc = crc32(&r, 12);                                // khởi động lại: lấy bản ghi CRC đúng có seq lớn nhất
            off_t at = (i % 4096) * (off_t)sizeof r;              // ring: 4096 ô 16 byte
            ssize_t w = mode == "log" ? write(fd, &r, sizeof r) : pwrite(fd, &r, sizeof r, at);
            if (w != sizeof r || fdatasync(fd)) { perror("log"); return 1; }
            saves++;
        } else if (mode == "lmdb") {
            MDB_txn* t; mdb_txn_begin(env, nullptr, 0, &t);
            MDB_val k{9, (void*)"energy_wh"}, v{sizeof i, &i};
            ck(mdb_put(t, dbi, &k, &v, 0), "put"); ck(mdb_txn_commit(t), "commit"); saves++;
        }
    }
    if (db) { sqlite3_finalize(st); sqlite3_close(db); }        // đóng: SQLite checkpoint WAL vào file chính
    if (fd >= 0) close(fd);
    if (env) mdb_env_close(env);
    sync();
    double bytes = (sectors_written() - s0) * 512.0;
    printf("%-6s %5ld giay, %5ld lan ghi xuong: %8.0f KB, %6.0f byte moi giay -> %6.1f GB/nam\n",
           mode.c_str(), N, saves, bytes / 1024, bytes / N, bytes / N * 365 * 86400 / 1e9);
}
