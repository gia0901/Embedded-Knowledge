// Bài tập F04-5: B+tree trong bộ nhớ (khóa và giá trị uint64_t), không cần xóa.
//   insert(k, v): thêm; nếu k đã có thì cập nhật giá trị và trả false.
//   find(k):      con trỏ tới giá trị hoặc nullptr.
//   range(lo, hi, f): gọi f(k, v) cho mọi k trong [lo, hi) theo thứ tự tăng, đi dọc các lá nối nhau.
// Mỗi node chứa tối đa M khóa. Lá đầy thì tách đôi, đẩy khóa đầu của nửa phải lên cha; cha đầy thì tách tiếp.
// Test so với std::map trên 300.000 thao tác ngẫu nhiên, kiểm tra chiều cao cây, rồi đo tốc độ tìm.
// Biên dịch: g++ -std=c++20 -O2 ex5_bptree.cpp -o ex5 && ./ex5    (gỡ lỗi: -O0 -g -fsanitize=address)
#include <chrono>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <cstdio>
#include <functional>
#include <map>
#include <random>
#include <vector>

// ===================== PHẦN CỦA BẠN =====================
template <int M>
class BPlusTree {
    static_assert(M >= 3);
public:
    BPlusTree() = default;
    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;
    ~BPlusTree() { destroy(root_, height_); }

    bool insert(uint64_t k, uint64_t v) {
        (void)k; (void)v;
        return false;  // TODO
    }
    const uint64_t* find(uint64_t k) const {
        (void)k;
        return nullptr;  // TODO
    }
    void range(uint64_t lo, uint64_t hi, const std::function<void(uint64_t, uint64_t)>& f) const {
        (void)lo; (void)hi; (void)f;  // TODO
    }
    size_t size() const { return size_; }
    int height() const { return height_; }

private:
    struct Leaf { int n = 0; uint64_t keys[M]; uint64_t vals[M]; Leaf* next = nullptr; };
    struct Inner { int n = 0; uint64_t keys[M]; void* child[M + 1]; };   // child[i] chứa khóa < keys[i] <= child[i+1]
    void* root_ = nullptr;
    int height_ = 0;                                       // 1 = gốc là lá
    size_t size_ = 0;

    static void destroy(void* n, int h) {
        if (!n) return;
        if (h == 1) { delete static_cast<Leaf*>(n); return; }
        auto* in = static_cast<Inner*>(n);
        for (int i = 0; i <= in->n; i++) destroy(in->child[i], h - 1);
        delete in;
    }
};
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
template <int M> static int check(int ops, uint64_t keyspace, unsigned seed) {
    BPlusTree<M> t; std::map<uint64_t, uint64_t> ref;
    std::mt19937_64 rng(seed);
    int fails = 0;
    for (int i = 0; i < ops && fails < 5; i++) {
        uint64_t k = rng() % keyspace, v = rng();
        bool fresh = !ref.count(k);
        ref[k] = v;
        if (t.insert(k, v) != fresh) { std::printf("  FAIL M=%d: insert(%llu) trả sai giá trị\n", M, (unsigned long long)k); fails++; }
        if (i % 97 == 0) {
            uint64_t q = rng() % keyspace;
            const uint64_t* p = t.find(q);
            auto it = ref.find(q);
            if ((p == nullptr) != (it == ref.end()) || (p && *p != it->second)) { std::printf("  FAIL M=%d: find(%llu) sai\n", M, (unsigned long long)q); fails++; }
        }
    }
    if (t.size() != ref.size()) { std::printf("  FAIL M=%d: size %zu, đúng là %zu\n", M, t.size(), ref.size()); fails++; }
    for (int r = 0; r < 200 && !fails; r++) {
        uint64_t lo = rng() % keyspace, hi = lo + rng() % (keyspace / 50 + 1);
        std::vector<std::pair<uint64_t, uint64_t>> got, want(ref.lower_bound(lo), ref.lower_bound(hi));
        t.range(lo, hi, [&](uint64_t k, uint64_t v) { got.emplace_back(k, v); });
        if (got != want) { std::printf("  FAIL M=%d: range [%llu, %llu) sai\n", M, (unsigned long long)lo, (unsigned long long)hi); fails++; }
    }
    double maxh = 1 + std::ceil(std::log((double)ref.size() + 1) / std::log((M + 1) / 2.0));
    if (t.height() < 1 || t.height() > maxh) { std::printf("  FAIL M=%d: chiều cao %d, phải <= %.0f\n", M, t.height(), maxh); fails++; }
    return fails;
}
int main() {
    int fails = check<3>(20000, 5000, 1) + check<4>(50000, 20000, 2) + check<32>(300000, 1000000, 3) + check<64>(300000, 50000, 4);
    if (!fails) {                                          // so tốc độ tìm với std::map trên 1 triệu khóa
        BPlusTree<32> t; std::map<uint64_t, uint64_t> m;
        std::mt19937_64 rng(7);
        std::vector<uint64_t> keys(1000000);
        for (auto& k : keys) { k = rng(); t.insert(k, k); m[k] = k; }
        std::shuffle(keys.begin(), keys.end(), rng);
        uint64_t s = 0;
        auto t0 = std::chrono::steady_clock::now();
        for (auto k : keys) s += *t.find(k);
        auto t1 = std::chrono::steady_clock::now();
        for (auto k : keys) s += m.find(k)->second;
        auto t2 = std::chrono::steady_clock::now();
        std::printf("tìm 1 triệu khóa: B+tree M=32 %.0f ns/lần (cao %d tầng), std::map %.0f ns/lần  [%llu]\n",
                    std::chrono::duration<double, std::nano>(t1 - t0).count() / 1e6, t.height(),
                    std::chrono::duration<double, std::nano>(t2 - t1).count() / 1e6, (unsigned long long)(s & 1));
    }
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
