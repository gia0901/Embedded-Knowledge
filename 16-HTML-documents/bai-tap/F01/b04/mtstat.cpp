// mtstat: framestat chạy bằng T thread trên 16 triệu frame, ba cách gom thống kê:
//   shared+mutex : một bảng chung, khóa một mutex cho mỗi frame
//   shared+atomic: một bảng chung, các trường là std::atomic
//   rieng+gop    : mỗi thread một bảng riêng, gộp lại sau cùng
// Lời giải Lab 4.2. Build: g++ -O2 -std=c++20 -I.. -pthread -o mtstat mtstat.cpp     Chạy: ./mtstat
#include "bench.h"
#include "frame.h"
#include <atomic>
#include <mutex>
#include <thread>

struct DevStat { long n = 0, sum = 0; int32_t min = INT32_MAX, max = INT32_MIN; long alarms = 0; };
struct AtomicStat { std::atomic<long> n{0}, sum{0}, alarms{0}; std::atomic<int32_t> min{INT32_MAX}, max{INT32_MIN}; };

template <class Body> void parallel(int T, size_t n, Body body) {
    std::vector<std::thread> th;
    for (int t = 0; t < T; ++t) th.emplace_back(body, t, t * n / T, (t + 1) * n / T);
    for (auto& x : th) x.join();
}

static void update(DevStat& s, const Frame& f) {
    if (f.type == ALARM) { ++s.alarms; return; }
    if (f.type != TEMP) return;
    ++s.n; s.sum += f.value; s.min = std::min(s.min, f.value); s.max = std::max(s.max, f.value);
}

int main() {
    const size_t n = 16'000'000;
    const int devs = 64;
    auto buf = make_frames(n, devs);
    for (int T : {1, 2, 4}) {
        std::printf("--- %d thread\n", T);
        long sums[3] = {};
        {   std::vector<DevStat> st(devs); std::mutex mu;
            bench_ns("shared+mutex", n, 5, [&] {
                for (auto& s : st) s = DevStat{};
                parallel(T, n, [&](int, size_t a, size_t b) {
                    Frame f;
                    for (size_t i = a; i < b; ++i) if (parse(&buf[i * kWire], f)) { std::lock_guard g(mu); update(st[f.dev], f); }
                });
            });
            for (auto& s : st) sums[0] += s.sum; }
        {   std::vector<AtomicStat> st(devs);
            bench_ns("shared+atomic", n, 5, [&] {
                for (auto& s : st) { s.n = 0; s.sum = 0; s.alarms = 0; s.min = INT32_MAX; s.max = INT32_MIN; }
                parallel(T, n, [&](int, size_t a, size_t b) {
                    Frame f;
                    for (size_t i = a; i < b; ++i) {
                        if (!parse(&buf[i * kWire], f)) continue;
                        AtomicStat& s = st[f.dev];
                        if (f.type == ALARM) { s.alarms.fetch_add(1, std::memory_order_relaxed); continue; }
                        if (f.type != TEMP) continue;
                        s.n.fetch_add(1, std::memory_order_relaxed);
                        s.sum.fetch_add(f.value, std::memory_order_relaxed);
                        int32_t m = s.min.load(std::memory_order_relaxed);   // min/max bằng vòng CAS
                        while (f.value < m && !s.min.compare_exchange_weak(m, f.value, std::memory_order_relaxed)) {}
                        m = s.max.load(std::memory_order_relaxed);
                        while (f.value > m && !s.max.compare_exchange_weak(m, f.value, std::memory_order_relaxed)) {}
                    }
                });
            });
            for (auto& s : st) sums[1] += s.sum; }
        {   std::vector<std::vector<DevStat>> local(T, std::vector<DevStat>(devs)); std::vector<DevStat> st(devs);
            bench_ns("rieng+gop", n, 5, [&] {
                parallel(T, n, [&](int t, size_t a, size_t b) {
                    auto& my = local[t];
                    for (auto& s : my) s = DevStat{};
                    Frame f;
                    for (size_t i = a; i < b; ++i) if (parse(&buf[i * kWire], f)) update(my[f.dev], f);
                });
                for (int d = 0; d < devs; ++d) {                 // gộp: 64 × T phép cộng, không đáng kể
                    DevStat g;
                    for (int t = 0; t < T; ++t) {
                        const DevStat& s = local[t][d];
                        g.n += s.n; g.sum += s.sum; g.alarms += s.alarms;
                        g.min = std::min(g.min, s.min); g.max = std::max(g.max, s.max);
                    }
                    st[d] = g;
                }
            });
            for (auto& s : st) sums[2] += s.sum; }
        std::printf("tong nhiet do: %ld %ld %ld\n", sums[0], sums[1], sums[2]);
    }
}
