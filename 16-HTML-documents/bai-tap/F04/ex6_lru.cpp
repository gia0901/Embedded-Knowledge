// Bài tập F04-6: Cache LRU có TTL, mọi thao tác O(1) — thứ Redis làm với maxmemory-policy allkeys-lru và EXPIRE,
// và thứ một NF dùng để cache hồ sơ thuê bao đọc từ UDM.
//   get(k, now): giá trị nếu có và chưa hết hạn (hết hạn khi now >= expire_at), đồng thời đánh dấu k là "mới dùng".
//                Mục hết hạn bị xóa khi gặp và trả nullopt.
//   put(k, v, ttl, now): thêm hoặc cập nhật (cũng tính là "mới dùng"), expire_at = now + ttl.
//                Khi đầy (size == capacity) và k chưa có: bỏ mục ít được dùng gần đây nhất.
// Gợi ý: std::list giữ thứ tự dùng (đầu = mới nhất) + std::unordered_map<khóa, iterator vào list>.
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined ex6_lru.cpp -o ex6 && ./ex6
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <list>
#include <optional>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// ===================== PHẦN CỦA BẠN =====================
class LruCache {
public:
    explicit LruCache(size_t capacity) : cap_(capacity) {}
    std::optional<std::string> get(uint64_t k, int64_t now) {
        (void)k; (void)now;
        return std::nullopt;  // TODO
    }
    void put(uint64_t k, std::string v, int64_t ttl, int64_t now) {
        (void)k; (void)v; (void)ttl; (void)now;  // TODO
    }
    size_t size() const {
        return 0;  // TODO
    }
private:
    size_t cap_;
};
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
// Mô hình tham chiếu chậm (O(n)) để so kết quả.
struct Ref {
    struct E { uint64_t k; std::string v; int64_t exp; };
    std::vector<E> order;                                  // đầu = mới dùng nhất
    size_t cap;
    std::optional<std::string> get(uint64_t k, int64_t now) {
        for (size_t i = 0; i < order.size(); i++) if (order[i].k == k) {
            E e = order[i]; order.erase(order.begin() + (long)i);
            if (now >= e.exp) return std::nullopt;
            order.insert(order.begin(), e); return e.v;
        }
        return std::nullopt;
    }
    void put(uint64_t k, const std::string& v, int64_t ttl, int64_t now) {
        for (size_t i = 0; i < order.size(); i++) if (order[i].k == k) { order.erase(order.begin() + (long)i); break; }
        if (order.size() == cap) order.pop_back();
        order.insert(order.begin(), E{k, v, now + ttl});
    }
};
int main() {
    int fails = 0;
    LruCache c(2);
    c.put(1, "a", 100, 0); c.put(2, "b", 100, 0);
    c.get(1, 1);                                           // 1 mới dùng, 2 cũ nhất
    c.put(3, "c", 100, 2);                                 // bỏ 2
    if (c.get(2, 3) || c.get(1, 3) != "a" || c.get(3, 3) != "c") { std::printf("  FAIL: thứ tự LRU sai\n"); fails++; }
    c.put(1, "a2", 5, 10);
    if (c.get(1, 14) != "a2" || c.get(1, 15) || c.size() != 1) { std::printf("  FAIL: TTL sai\n"); fails++; }

    std::mt19937_64 rng(11);
    LruCache lc(64); Ref ref{{}, 64};
    int64_t now = 0;
    for (int i = 0; i < 200000 && fails < 3; i++) {
        now += (int64_t)(rng() % 3);
        uint64_t k = rng() % 150;
        if (rng() % 3 == 0) {
            std::string v = std::to_string(rng() % 1000);
            int64_t ttl = 1 + (int64_t)(rng() % 400);
            lc.put(k, v, ttl, now); ref.put(k, v, ttl, now);
        } else if (lc.get(k, now) != ref.get(k, now)) {
            std::printf("  FAIL thao tác %d: get(%llu) khác mô hình tham chiếu\n", i, (unsigned long long)k); fails++;
        }
    }
    LruCache big(1 << 20);                                 // O(1): 2 triệu thao tác phải xong nhanh
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < 2000000; i++) { big.put((uint64_t)i, "x", 1000000000, i); big.get((uint64_t)(i / 2), i); }
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("2 triệu put + get: %.2f s, size %zu\n", s, big.size());
    if (big.size() != (1u << 20)) { std::printf("  FAIL: cache lớn phải đầy đúng capacity\n"); fails++; }
    if (s > 20) { std::printf("  FAIL: quá chậm, có thao tác không phải O(1)?\n"); fails++; }
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
