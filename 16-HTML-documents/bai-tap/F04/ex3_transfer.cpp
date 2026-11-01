// Bài tập F04-3: Chuyển quota giữa hai thuê bao trong SQLite, an toàn khi nhiều thread cùng chạy.
// Mỗi thread có kết nối riêng tới cùng một file database. Viết transfer():
//   - Trừ amount từ from, cộng vào to, trong MỘT transaction.
//   - Nếu from không đủ quota (balance < amount): không đổi gì, trả INSUFFICIENT.
//   - Gặp SQLITE_BUSY (thread khác đang ghi): rollback nếu cần và thử lại cả transaction.
// Bất biến được kiểm tra: tổng quota không đổi, không ai âm, số lần chuyển thành công khớp với bảng log.
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined -pthread ex3_transfer.cpp -o ex3 -lsqlite3 && ./ex3
#include <sqlite3.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <random>
#include <thread>
#include <unistd.h>
#include <vector>

enum class Tx { OK, INSUFFICIENT, ERROR };

// ===================== PHẦN CỦA BẠN =====================
// Bảng: quota(id INTEGER PRIMARY KEY, balance INTEGER NOT NULL)
//       transfer_log(id INTEGER PRIMARY KEY, src INTEGER, dst INTEGER, amount INTEGER)  -- ghi 1 dòng cho mỗi lần chuyển thành công
// Gợi ý: BEGIN IMMEDIATE lấy khóa ghi ngay từ đầu, tránh deadlock khi hai transaction cùng nâng từ đọc lên ghi.
//        Một UPDATE có điều kiện "AND balance >= ?" rồi kiểm tra sqlite3_changes() thay được bước SELECT riêng.
Tx transfer(sqlite3* db, int from, int to, int amount) {
    // Phiên bản ngây thơ: đọc, kiểm tra ở ứng dụng, rồi ghi; không có transaction, không xử lý BUSY.
    sqlite3_stmt* st;
    sqlite3_prepare_v2(db, "SELECT balance FROM quota WHERE id = ?", -1, &st, nullptr);
    sqlite3_bind_int(st, 1, from);
    int bal = sqlite3_step(st) == SQLITE_ROW ? sqlite3_column_int(st, 0) : 0;
    sqlite3_finalize(st);
    if (bal < amount) return Tx::INSUFFICIENT;
    char sql[256];
    std::snprintf(sql, sizeof sql, "UPDATE quota SET balance = %d WHERE id = %d", bal - amount, from);
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    std::snprintf(sql, sizeof sql, "UPDATE quota SET balance = balance + %d WHERE id = %d", amount, to);
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    std::snprintf(sql, sizeof sql, "INSERT INTO transfer_log(src, dst, amount) VALUES (%d, %d, %d)", from, to, amount);
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    return Tx::OK;  // TODO
}
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static long long q1(sqlite3* db, const char* sql) {
    sqlite3_stmt* st; sqlite3_prepare_v2(db, sql, -1, &st, nullptr);
    long long v = sqlite3_step(st) == SQLITE_ROW ? sqlite3_column_int64(st, 0) : -1;
    sqlite3_finalize(st); return v;
}
int main() {
    const char* path = "ex3_transfer.db";
    unlink(path); unlink("ex3_transfer.db-wal"); unlink("ex3_transfer.db-shm");
    sqlite3* init; sqlite3_open(path, &init);
    sqlite3_exec(init, "PRAGMA journal_mode=WAL;"
                       "CREATE TABLE quota(id INTEGER PRIMARY KEY, balance INTEGER NOT NULL);"
                       "CREATE TABLE transfer_log(id INTEGER PRIMARY KEY, src INTEGER, dst INTEGER, amount INTEGER);"
                       "WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM n WHERE i < 10)"
                       " INSERT INTO quota SELECT i, 100 FROM n;", nullptr, nullptr, nullptr);
    const int T = 8, OPS = 300;
    std::atomic<long> ok{0}, insuff{0}, err{0};
    std::vector<std::thread> ts;
    for (int t = 0; t < T; t++) ts.emplace_back([&, t] {
        sqlite3* db; sqlite3_open(path, &db);
        sqlite3_busy_timeout(db, 0);                         // BUSY trả về ngay: code của bạn phải tự xử lý
        std::mt19937 rng(t);
        for (int i = 0; i < OPS; i++) {
            int a = 1 + (int)(rng() % 10), b = 1 + (int)(rng() % 10);
            if (a == b) continue;
            switch (transfer(db, a, b, 1 + (int)(rng() % 60))) {
                case Tx::OK: ok++; break;
                case Tx::INSUFFICIENT: insuff++; break;
                default: err++;
            }
        }
        sqlite3_close(db);
    });
    for (auto& th : ts) th.join();
    long long total = q1(init, "SELECT SUM(balance) FROM quota");
    long long neg = q1(init, "SELECT COUNT(*) FROM quota WHERE balance < 0");
    long long logs = q1(init, "SELECT COUNT(*) FROM transfer_log");
    sqlite3_close(init);
    unlink(path); unlink("ex3_transfer.db-wal"); unlink("ex3_transfer.db-shm");
    std::printf("thành công %ld, không đủ quota %ld, lỗi %ld; tổng quota %lld, dòng log %lld\n",
                ok.load(), insuff.load(), err.load(), total, logs);
    int fails = 0;
    if (total != 1000) { std::printf("  FAIL: tổng quota phải luôn là 1000\n"); fails++; }
    if (neg) { std::printf("  FAIL: %lld thuê bao bị âm quota\n", neg); fails++; }
    if (err) { std::printf("  FAIL: %ld lần chuyển trả ERROR (chưa xử lý SQLITE_BUSY?)\n", err.load()); fails++; }
    if (logs != ok) { std::printf("  FAIL: số dòng log khác số lần báo thành công\n"); fails++; }
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
