// Bài tập F03-6: Bảng định tuyến IPv4 với longest prefix match, dùng binary trie.
// insert(prefix, len, nh): thêm hoặc thay route. remove(prefix, len): xóa đúng route đó, trả false nếu không có.
// lookup(ip): next hop của route có prefix dài nhất khớp ip, hoặc -1 nếu không có route nào.
// Bit của prefix ngoài len bit đầu phải được bỏ qua (10.1.2.3/16 nghĩa là 10.1.0.0/16).
// Biên dịch: g++ -std=c++20 -O2 ex6_lpm.cpp -o ex6 && ./ex6     (thêm -g -fsanitize=address khi gỡ lỗi)
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <vector>

// ===================== PHẦN CỦA BẠN =====================
class Lpm {
public:
    void insert(uint32_t prefix, int len, int nh) {
        (void)prefix; (void)len; (void)nh;  // TODO
    }
    bool remove(uint32_t prefix, int len) {
        (void)prefix; (void)len;
        return false;  // TODO
    }
    int lookup(uint32_t ip) const {
        (void)ip;
        return -1;  // TODO
    }
};
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
struct Route { uint32_t p; int len, nh; };
static uint32_t mask(int len) { return len == 0 ? 0 : 0xFFFFFFFFu << (32 - len); }
static int brute(const std::vector<Route>& rs, uint32_t ip) {
    int best = -1, blen = -1;
    for (auto& r : rs) if ((ip & mask(r.len)) == (r.p & mask(r.len)) && r.len > blen) { best = r.nh; blen = r.len; }
    return best;
}
static void upsert(std::vector<Route>& rs, Route r) {
    for (auto& x : rs) if (x.len == r.len && (x.p & mask(x.len)) == (r.p & mask(r.len))) { x.nh = r.nh; return; }
    rs.push_back(r);
}
int main() {
    int fails = 0;
    Lpm t;
    t.insert(0x0A000000, 8, 1);            // 10.0.0.0/8
    t.insert(0x0A010000, 16, 2);           // 10.1.0.0/16
    t.insert(0x0A010203, 32, 3);           // 10.1.2.3/32
    if (t.lookup(0x0A010203) != 3 || t.lookup(0x0A010204) != 2 || t.lookup(0x0A020000) != 1 || t.lookup(0x0B000000) != -1) {
        std::printf("  FAIL: ví dụ cơ bản 10/8, 10.1/16, 10.1.2.3/32\n"); fails++; }
    t.insert(0, 0, 9);                      // default route
    t.insert(0x0A01FFFF, 16, 4);            // 10.1.255.255/16 = 10.1.0.0/16, thay next hop
    if (t.lookup(0x0B000000) != 9 || t.lookup(0x0A010204) != 4) { std::printf("  FAIL: default route hoặc thay route\n"); fails++; }
    if (!t.remove(0x0A010000, 16) || t.remove(0x0A010000, 16) || t.lookup(0x0A010204) != 1 || t.lookup(0x0A010203) != 3) {
        std::printf("  FAIL: remove\n"); fails++; }

    std::mt19937 rng(5);
    Lpm big;
    std::vector<Route> rs;
    for (int i = 0; i < 3000; i++) {        // nhiều prefix chồng lên nhau quanh 10.x và 192.168.x
        uint32_t base = (i % 2 ? 0x0A000000u : 0xC0A80000u) | (rng() & 0x00FFFFFFu);
        int len = (int)(8 + rng() % 25);
        Route r{base, len, i};
        big.insert(r.p, r.len, r.nh); upsert(rs, r);
    }
    for (int i = 0; i < 500; i++) {         // xóa ngẫu nhiên
        size_t k = rng() % rs.size();
        if (!big.remove(rs[k].p, rs[k].len)) { std::printf("  FAIL: remove route đang có trả false\n"); fails++; break; }
        rs.erase(rs.begin() + (long)k);
    }
    int wrong = 0;
    std::vector<uint32_t> ips(200000);
    for (auto& ip : ips) ip = (rng() % 2 ? 0x0A000000u : 0xC0A80000u) | (rng() & 0x00FFFFFFu);
    for (int i = 0; i < 20000; i++) if (big.lookup(ips[i]) != brute(rs, ips[i])) wrong++;
    if (wrong) { std::printf("  FAIL: %d/20000 lookup khác kết quả duyệt toàn bộ\n", wrong); fails++; }

    auto t0 = std::chrono::steady_clock::now();
    long sum = 0;
    for (auto ip : ips) sum += big.lookup(ip);
    double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / (double)ips.size();
    std::printf("lookup: %.0f ns/lần với %zu route (checksum %ld)\n", ns, rs.size(), sum);
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
