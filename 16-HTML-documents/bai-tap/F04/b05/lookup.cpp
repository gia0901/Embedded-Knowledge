// lookup: tìm một khoá trong 1 triệu bản ghi bằng bốn cách, trên cùng dữ liệu.
//   quét tuần tự (bảng không có index), std::map (cây nhị phân cân bằng, mỗi node một lần cấp phát),
//   mảng đã sắp xếp + tìm nhị phân, std::unordered_map (bảng băm).
// Thêm hai phép đo: quét tìm khoá KHÔNG có, và truy vấn khoảng 1000 khoá liên tiếp.
// Build: g++ -O2 -std=c++20 -Wall -o lookup lookup.cpp
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <map>
#include <random>
#include <unordered_map>
#include <vector>

struct Row { uint64_t id; uint64_t value; };

template <class F> double ns_per_op(int ops, F f) {
    auto t0 = std::chrono::steady_clock::now();
    f();
    return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / ops;
}

int main() {
    const int N = 1'000'000, Q = 200'000, QS = 200;
    std::mt19937_64 rng(42);
    std::vector<Row> heap(N);                       // "heap file": bản ghi không theo thứ tự
    for (int i = 0; i < N; i++) heap[i] = {rng() | 1, (uint64_t)i};   // id lẻ; số chẵn chắc chắn không có

    std::map<uint64_t, uint64_t> tree;
    std::unordered_map<uint64_t, uint64_t> hash;
    hash.reserve(N);
    for (auto& r : heap) { tree[r.id] = r.value; hash[r.id] = r.value; }
    std::vector<Row> sorted = heap;
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.id < b.id; });

    std::vector<uint64_t> keys(Q);                  // khoá cần tìm, đều có trong bảng
    for (auto& k : keys) k = heap[rng() % N].id;
    volatile uint64_t sink = 0;

    double scan = ns_per_op(QS, [&] {               // quét chỉ 200 lần vì quá chậm
        for (int q = 0; q < QS; q++)
            for (auto& r : heap) if (r.id == keys[q]) { sink = sink + r.value; break; }
    });
    double miss = ns_per_op(QS, [&] {               // khoá không có: phải đi hết bảng mới biết
        for (int q = 0; q < QS; q++)
            for (auto& r : heap) if (r.id == keys[q] + 1) { sink = sink + r.value; break; }
    });
    double t_tree = ns_per_op(Q, [&] { for (auto k : keys) sink = sink + tree.find(k)->second; });
    double t_bin = ns_per_op(Q, [&] {
        for (auto k : keys) {
            auto it = std::lower_bound(sorted.begin(), sorted.end(), k,
                                       [](const Row& r, uint64_t key) { return r.id < key; });
            sink = sink + it->value;
        }
    });
    double t_hash = ns_per_op(Q, [&] { for (auto k : keys) sink = sink + hash.find(k)->second; });

    // Khoảng [lo, hi) chứa đúng 1000 khoá. Cây và mảng sắp xếp đi tới lo rồi đọc tiếp.
    // Bảng băm không có thứ tự: chỉ còn cách duyệt hết và lọc.
    uint64_t lo = sorted[N / 2].id, hi = sorted[N / 2 + 1000].id;
    size_t c_tree = 0, c_hash = 0;
    double r_tree = ns_per_op(1, [&] { for (auto it = tree.lower_bound(lo); it != tree.end() && it->first < hi; ++it) c_tree++; });
    double r_hash = ns_per_op(1, [&] { for (auto& [k, v] : hash) if (k >= lo && k < hi) c_hash++; });

    std::printf("tim 1 khoa trong %d ban ghi (ns/lan):\n", N);
    std::printf("  quet tuan tu, khoa co      %10.0f\n", scan);
    std::printf("  quet tuan tu, khoa KHONG co %9.0f\n", miss);
    std::printf("  std::map (cay)             %10.0f\n", t_tree);
    std::printf("  mang sap xep, nhi phan     %10.0f\n", t_bin);
    std::printf("  unordered_map (bam)        %10.0f\n", t_hash);
    std::printf("khoang 1000 khoa: cay %zu khoa, %.0f us; bam %zu khoa, %.0f us\n",
                c_tree, r_tree / 1000, c_hash, r_hash / 1000);
}
