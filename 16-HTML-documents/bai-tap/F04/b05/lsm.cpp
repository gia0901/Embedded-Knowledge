// lsm: cùng 1 triệu cặp khoá, giá trị ghi vào SQLite (B-tree) và RocksDB (LSM-tree), rồi xoá một nửa.
// Đo số byte tiến trình thật sự ghi xuống (write_bytes trong /proc/self/io), kích thước trên đĩa, thời gian.
//   ./lsm sqlite DIR     B-tree: bảng WITHOUT ROWID, WAL, synchronous=NORMAL
//   ./lsm rocks DIR      LSM: cấu hình mặc định, WAL bật, không fsync mỗi lô (tương đương NORMAL)
// Khoá: 16 ký tự hex ngẫu nhiên; giá trị: 100 byte ngẫu nhiên (không nén được). Lô 1000 cặp mỗi transaction (WriteBatch).
// RocksDB được thu nhỏ cấu hình (memtable 4 MB, tầng L1 16 MB) để 116 MB dữ liệu đã đủ thấy nhiều tầng.
// Build: g++ -O2 -std=c++20 -Wall -o lsm lsm.cpp -lsqlite3 -lrocksdb
#include <rocksdb/db.h>
#include <rocksdb/write_batch.h>
#include <sqlite3.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;
static const int N = 1'000'000, BATCH = 1000, VLEN = 100;

static long write_bytes() {                       // byte tiến trình này đã đẩy xuống tầng block
    std::ifstream f("/proc/self/io"); std::string k; long v;
    while (f >> k >> v) if (k == "write_bytes:") return v;
    return -1;
}
static long dir_bytes(const std::string& d) {
    long s = 0;
    for (auto& e : fs::recursive_directory_iterator(d)) if (e.is_regular_file()) s += e.file_size();
    return s;
}
static double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
static std::vector<std::string> make_keys() {
    std::mt19937_64 rng(1);
    std::vector<std::string> keys(N);
    char b[17];
    for (auto& k : keys) { snprintf(b, sizeof b, "%016llx", (unsigned long long)rng()); k = b; }
    return keys;
}
static void report(const char* step, double t, long w0, const std::string& dir) {
    long user = (long)N * (16 + VLEN);            // byte ứng dụng đưa vào: 116 MB
    long w = write_bytes() - w0;
    printf("%-10s %6.1f s  ghi xuong %5ld MB (%4.1f lan du lieu)  tren dia %4ld MB\n",
           step, t, w >> 20, (double)w / user, dir_bytes(dir) >> 20);
}

static void levels(rocksdb::DB* db) {          // số file SST ở mỗi tầng L0..L6
    std::string lv;
    printf("so file L0..L6:");
    for (int l = 0; l < 7; l++) { db->GetProperty("rocksdb.num-files-at-level" + std::to_string(l), &lv); printf(" %s", lv.c_str()); }
    printf("\n");
}

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s sqlite|rocks DIR\n", argv[0]); return 1; }
    std::string mode = argv[1], dir = argv[2];
    fs::remove_all(dir); fs::create_directories(dir);
    auto keys = make_keys();
    std::mt19937 rng(2);
    std::mt19937_64 vr(3);
    std::vector<std::string> vals(N, std::string(VLEN, 0));
    for (auto& v : vals) for (auto& c : v) c = (char)vr();

    if (mode == "sqlite") {
        sqlite3* db; sqlite3_open((dir + "/kv.db").c_str(), &db);
        sqlite3_exec(db, "PRAGMA journal_mode=WAL; PRAGMA synchronous=NORMAL;"
                         "CREATE TABLE kv(k TEXT PRIMARY KEY, v BLOB) WITHOUT ROWID;", 0, 0, 0);
        sqlite3_stmt *ins, *del, *get;
        sqlite3_prepare_v2(db, "INSERT INTO kv VALUES(?, ?)", -1, &ins, 0);
        sqlite3_prepare_v2(db, "DELETE FROM kv WHERE k = ?", -1, &del, 0);
        sqlite3_prepare_v2(db, "SELECT length(v) FROM kv WHERE k = ?", -1, &get, 0);
        long w0 = write_bytes(); double t0 = now();
        for (int i = 0; i < N; i++) {
            if (i % BATCH == 0) sqlite3_exec(db, "BEGIN", 0, 0, 0);
            sqlite3_bind_text(ins, 1, keys[i].c_str(), 16, SQLITE_STATIC);
            sqlite3_bind_blob(ins, 2, vals[i].data(), VLEN, SQLITE_STATIC);
            sqlite3_step(ins); sqlite3_reset(ins);
            if (i % BATCH == BATCH - 1) sqlite3_exec(db, "COMMIT", 0, 0, 0);
        }
        sqlite3_exec(db, "PRAGMA wal_checkpoint(TRUNCATE)", 0, 0, 0);
        report("chen", now() - t0, w0, dir);
        int found = 0; t0 = now();
        for (int i = 0; i < 100000; i++) {
            sqlite3_bind_text(get, 1, keys[rng() % N].c_str(), 16, SQLITE_STATIC);
            found += sqlite3_step(get) == SQLITE_ROW; sqlite3_reset(get);
        }
        printf("doc 100000 khoa: %d thay, %.2f us/lan\n", found, (now() - t0) * 10);
        w0 = write_bytes(); t0 = now();
        for (int i = 0; i < N; i += 2) {
            if (i % BATCH == 0) sqlite3_exec(db, "BEGIN", 0, 0, 0);
            sqlite3_bind_text(del, 1, keys[i].c_str(), 16, SQLITE_STATIC);
            sqlite3_step(del); sqlite3_reset(del);
            if (i % BATCH == BATCH - 2) sqlite3_exec(db, "COMMIT", 0, 0, 0);
        }
        sqlite3_exec(db, "PRAGMA wal_checkpoint(TRUNCATE)", 0, 0, 0);
        report("xoa 1/2", now() - t0, w0, dir);
        sqlite3_stmt* fl; sqlite3_prepare_v2(db, "PRAGMA freelist_count", -1, &fl, 0); sqlite3_step(fl);
        printf("page trong (freelist): %d\n", sqlite3_column_int(fl, 0)); sqlite3_finalize(fl);
        sqlite3_bind_text(get, 1, keys[0].c_str(), 16, SQLITE_STATIC);
        printf("khoa da xoa: %s\n", sqlite3_step(get) == SQLITE_ROW ? "VAN THAY" : "khong thay"); sqlite3_reset(get);
        w0 = write_bytes(); t0 = now();
        sqlite3_exec(db, "VACUUM; PRAGMA wal_checkpoint(TRUNCATE)", 0, 0, 0);
        report("VACUUM", now() - t0, w0, dir);
        for (auto* s : {ins, del, get}) sqlite3_finalize(s);
        sqlite3_close(db);
    } else {
        rocksdb::DB* db; rocksdb::Options o; o.create_if_missing = true;
        o.write_buffer_size = 4 << 20; o.target_file_size_base = 4 << 20; o.max_bytes_for_level_base = 16 << 20;
        if (!rocksdb::DB::Open(o, dir, &db).ok()) { fprintf(stderr, "khong mo duoc %s\n", dir.c_str()); return 1; }
        rocksdb::WriteOptions wo; rocksdb::ReadOptions ro;
        long w0 = write_bytes(); double t0 = now();
        rocksdb::WriteBatch b;
        for (int i = 0; i < N; i++) {
            b.Put(keys[i], vals[i]);
            if (i % BATCH == BATCH - 1) { db->Write(wo, &b); b.Clear(); }
        }
        db->Flush(rocksdb::FlushOptions());
        db->WaitForCompact(rocksdb::WaitForCompactOptions());
        report("chen", now() - t0, w0, dir);
        std::string v;
        levels(db);
        int found = 0; t0 = now();
        for (int i = 0; i < 100000; i++) found += db->Get(ro, keys[rng() % N], &v).ok();
        printf("doc 100000 khoa: %d thay, %.2f us/lan\n", found, (now() - t0) * 10);
        w0 = write_bytes(); t0 = now();
        for (int i = 0; i < N; i += 2) {
            b.Delete(keys[i]);                       // tombstone: một bản ghi "đã xoá", không xoá gì trên đĩa
            if (i % BATCH == BATCH - 2) { db->Write(wo, &b); b.Clear(); }
        }
        db->Flush(rocksdb::FlushOptions());           // không chờ compaction: xem tombstone nằm trên đĩa
        report("xoa 1/2", now() - t0, w0, dir);
        printf("khoa da xoa: %s\n", db->Get(ro, keys[0], &v).ok() ? "VAN THAY" : "khong thay");
        w0 = write_bytes(); t0 = now();
        db->CompactRange(rocksdb::CompactRangeOptions(), nullptr, nullptr);
        report("compact", now() - t0, w0, dir);
        levels(db);
        delete db;
    }
}
