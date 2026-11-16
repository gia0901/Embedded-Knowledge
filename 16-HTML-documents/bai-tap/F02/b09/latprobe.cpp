// latprobe: N thread bận chạy vòng lặp, 1 thread "probe" ngủ 1 ms rồi đo nó thức dậy trễ bao nhiêu.
// Giống một pod có thread xử lý nặng và một thread phải trả lời nhanh (heartbeat, PFCP, HTTP health check).
// Lời giải Lab 9.2. Build: g++ -O2 -std=c++20 -pthread -o latprobe latprobe.cpp
// Chạy:  ./latprobe <số thread bận> [giây=5]
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <time.h>
#include <vector>

using clk = std::chrono::steady_clock;

int main(int argc, char** argv) {
    int busy = argc > 1 ? std::atoi(argv[1]) : 2;
    double secs = argc > 2 ? std::atof(argv[2]) : 5;
    std::atomic<bool> stop{false};
    std::vector<long> work(busy);
    std::vector<std::thread> ts;
    for (int i = 0; i < busy; ++i)
        ts.emplace_back([&, i] { long n = 0; while (!stop.load(std::memory_order_relaxed)) ++n; work[i] = n; });

    std::vector<double> late;                            // độ trễ thức dậy, ms
    auto end = clk::now() + std::chrono::duration<double>(secs);
    while (clk::now() < end) {
        auto t0 = clk::now();
        timespec req{0, 1'000'000};                     // ngủ 1 ms
        clock_nanosleep(CLOCK_MONOTONIC, 0, &req, nullptr);
        double ms = std::chrono::duration<double, std::milli>(clk::now() - t0).count();
        late.push_back(ms - 1.0);
    }
    stop = true;
    for (auto& t : ts) t.join();
    std::sort(late.begin(), late.end());
    auto pct = [&](double p) { return late[std::min(late.size() - 1, size_t(p * late.size()))]; };
    long over10 = std::count_if(late.begin(), late.end(), [](double x) { return x > 10; });
    double total = 0; for (long w : work) total += w;
    std::printf("ban=%d probe %zu lan: tre p50 %.2f ms, p99 %.2f ms, max %.1f ms, %ld lan tre > 10 ms; viec %.2f ty vong\n",
                busy, late.size(), pct(0.5), pct(0.99), late.back(), over10, total / 1e9);
}
