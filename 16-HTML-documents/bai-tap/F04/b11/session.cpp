// session: bảng session theo TEID cho fast path của UPF. Mỗi gói uplink: tra session theo TEID, cộng bộ đếm.
//   umap     std::unordered_map<TEID, Session>: mỗi entry một node cấp phát riêng, bucket chỉ giữ con trỏ
//   flat     bảng băm địa chỉ mở (linear probing), entry 40 byte nằm liền trong một mảng
//   prefetch flat, nhưng prefetch ô của gói i + 8 trong lúc xử lý gói i (như rte_hash_lookup_bulk của DPDK)
//   split    tách nóng lạnh: bảng chỉ giữ phần cần để tra và chuyển gói (16 byte), bộ đếm ở mảng riêng
// Build: g++ -O2 -std=c++20 -Wall -o session session.cpp
// Chạy:  ./session N_SESSION N_GOI
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Session {                 // mọi thứ của một session trong một struct
    uint32_t teid;               // khoá; 0 = ô trống
    uint32_t gnb_ip, ue_ip;
    uint8_t  qfi, pad[3];
    uint64_t ul_bytes, ul_pkts;  // bộ đếm cho URR
    uint64_t mbr_bps;            // giới hạn tốc độ cho QER
};
struct Hot { uint32_t teid, gnb_ip, ue_ip, idx; };   // 16 byte: 4 entry mỗi cache line 64 byte
struct Counters { uint64_t ul_bytes, ul_pkts; };

static uint32_t mix(uint32_t k) { k ^= k >> 16; k *= 0x85ebca6bu; k ^= k >> 13; k *= 0xc2b2ae35u; k ^= k >> 16; return k; }

template <class E> class Flat {   // dung lượng là luỹ thừa 2, tải ≤ 50%
    std::vector<E> s_; uint32_t mask_;
public:
    explicit Flat(size_t n) { size_t c = 1; while (c < 2 * n) c <<= 1; s_.assign(c, E{}); mask_ = (uint32_t)c - 1; }
    void insert(const E& e) { for (uint32_t i = mix(e.teid) & mask_;; i = (i + 1) & mask_) if (!s_[i].teid) { s_[i] = e; return; } }
    void prefetch(uint32_t k) const { __builtin_prefetch(&s_[mix(k) & mask_]); }
    E* find(uint32_t k) {
        for (uint32_t i = mix(k) & mask_;; i = (i + 1) & mask_) { if (s_[i].teid == k) return &s_[i]; if (!s_[i].teid) return nullptr; }
    }
    size_t bytes() const { return s_.size() * sizeof(E); }
};

int main(int argc, char** argv) {
    int N = argc > 1 ? atoi(argv[1]) : 1000000, Q = argc > 2 ? atoi(argv[2]) : 5000000;
    std::mt19937 rng(7);
    std::vector<uint32_t> teids(N);
    std::unordered_set<uint32_t> seen;                   // TEID phải khác nhau và khác 0
    for (auto& t : teids) do t = rng(); while (t == 0 || !seen.insert(t).second);
    Flat<Session> flat(N); Flat<Hot> hot(N); std::vector<Counters> cnt(N);
    std::unordered_map<uint32_t, Session> umap; umap.reserve(N);
    for (int i = 0; i < N; i++) {
        Session s{}; s.teid = teids[i]; s.ue_ip = 0x0A2D0000u + i; s.qfi = 9;
        flat.insert(s); umap.emplace(s.teid, s); hot.insert(Hot{s.teid, 0, s.ue_ip, (uint32_t)i});
    }
    std::vector<uint32_t> pkts(Q);                       // TEID của các gói đến, ngẫu nhiên
    for (auto& p : pkts) p = teids[rng() % N];

    auto run = [&](const char* name, auto&& body) {
        auto t0 = std::chrono::steady_clock::now();
        for (int i = 0; i < Q; i++) body(i);
        double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / Q;
        printf("%-9s %6.1f ns/goi\n", name, ns);
    };
    printf("%d session, %d goi; bang flat %.1f MB, bang split %.1f MB + bo dem %.1f MB\n", N, Q,
           flat.bytes() / 1048576.0, hot.bytes() / 1048576.0, cnt.size() * sizeof(Counters) / 1048576.0);
    run("umap", [&](int i) { auto& s = umap.find(pkts[i])->second; s.ul_pkts++; s.ul_bytes += 1400; });
    run("flat", [&](int i) { auto* s = flat.find(pkts[i]); s->ul_pkts++; s->ul_bytes += 1400; });
    run("prefetch", [&](int i) {
        if (i + 8 < Q) flat.prefetch(pkts[i + 8]);
        auto* s = flat.find(pkts[i]); s->ul_pkts++; s->ul_bytes += 1400; });
    run("split", [&](int i) {
        if (i + 8 < Q) hot.prefetch(pkts[i + 8]);
        auto* h = hot.find(pkts[i]); auto& c = cnt[h->idx]; c.ul_pkts++; c.ul_bytes += 1400; });
    long check = 0;                                      // tổng số gói phải bằng 4 × Q: mỗi biến thể đếm một lần
    for (auto& [k, s] : umap) check += s.ul_pkts;
    for (int i = 0; i < N; i++) check += flat.find(teids[i])->ul_pkts + cnt[i].ul_pkts;
    printf("kiem tra: %ld goi da dem (= 4 x %d)\n", check, Q);
}
