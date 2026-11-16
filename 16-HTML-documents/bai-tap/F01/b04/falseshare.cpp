// falseshare: T thread, mỗi thread đếm frame cảnh báo trong lô frame của riêng nó (64 KB, nằm sẵn trong L2,
// đếm lặp lại nhiều lần) vào một bộ đếm atomic RIÊNG (kiểu bộ đếm thống kê hay gặp). Ba cách đặt bộ đếm:
//   (a) các bộ đếm nằm sát nhau trong một mảng: chung một cache line (false sharing)
//   (b) mỗi bộ đếm một cache line (alignas(64))
//   (c) đếm vào biến cục bộ, cuối cùng mới ghi ra
// Lời giải Lab 4.1. Build: g++ -O2 -std=c++20 -I.. -pthread -o falseshare falseshare.cpp
// Chạy: ./falseshare [số thread=4]
#include "bench.h"
#include "frame.h"
#include <cstdlib>
#include <thread>

#include <atomic>

struct Packed { std::atomic<long> n{0}; };               // 8 byte: 8 bộ đếm chung một line
struct alignas(64) Padded { std::atomic<long> n{0}; };   // 64 byte: mỗi bộ đếm một line
struct Local { long n = 0; void add(long v) { n += v; } };

constexpr size_t kBatch = 4096;                     // 4096 frame × 16 B = 64 KB mỗi thread
constexpr int kRounds = 400;                         // mỗi thread đếm lại lô của mình 400 lần

template <class Body> void parallel(int T, Body body) {
    std::vector<std::thread> th;
    for (int t = 0; t < T; ++t) th.emplace_back(body, t);
    for (auto& x : th) x.join();
}

int main(int argc, char** argv) {
    const int T = argc > 1 ? std::atoi(argv[1]) : 4;
    std::vector<std::vector<uint8_t>> batch;
    for (int t = 0; t < T; ++t) batch.push_back(make_frames(kBatch, 64, t + 1, 50));   // 50% là cảnh báo
    const size_t total = size_t(T) * kBatch * kRounds;
    auto add = [](auto& slot) {
        if constexpr (requires { slot.n.fetch_add(1); }) slot.n.fetch_add(1, std::memory_order_relaxed);
        else slot.add(1);
    };
    auto work = [&](int t, auto& slot) {
        const uint8_t* p = batch[t].data();
        for (int r = 0; r < kRounds; ++r)
            for (size_t i = 0; i < kBatch; ++i)
                if (p[i * kWire + 4] == ALARM) add(slot);          // mỗi frame cảnh báo: +1
    };
    long r[3] = {};
    std::vector<Packed> a(T);
    bench_ns("(a) mang sat nhau", total, 7, [&] { for (auto& x : a) x.n = 0; parallel(T, [&](int t) { work(t, a[t]); }); });
    for (auto& x : a) r[0] += x.n;
    std::vector<Padded> b(T);
    bench_ns("(b) alignas(64)", total, 7, [&] { for (auto& x : b) x.n = 0; parallel(T, [&](int t) { work(t, b[t]); }); });
    for (auto& x : b) r[1] += x.n;
    std::vector<long> c(T);
    bench_ns("(c) bien cuc bo", total, 7, [&] {
        parallel(T, [&](int t) { Local k; work(t, k); c[t] = k.n; });
    });
    for (auto x : c) r[2] += x;
    std::printf("%d thread, so canh bao: %ld %ld %ld\n", T, r[0], r[1], r[2]);
}
