// wake: frame đến thì worker biết sau bao lâu? So ba cách chờ, giống ba cách driver chờ card mạng:
//   spin  : quay đọc một biến atomic (polling liên tục, như DPDK)
//   sleep : ngủ trên condition_variable, được đánh thức (futex), như chờ ngắt rồi kernel đánh thức thread
//   timer : ngủ 50 µs rồi xem có gì không (polling theo chu kỳ)
// NIC ghi thời điểm phát vào biến, worker đo độ trễ tới lúc thấy. 20.000 frame, cách nhau khoảng 100 µs.
// Lời giải Lab 6.3. Build: g++ -O2 -std=c++20 -pthread -o wake wake.cpp      Chạy: ./wake
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>

using clk = std::chrono::steady_clock;
static long now_ns() { return std::chrono::duration_cast<std::chrono::nanoseconds>(clk::now().time_since_epoch()).count(); }
static void pin(int cpu) { cpu_set_t s; CPU_ZERO(&s); CPU_SET(cpu, &s); pthread_setaffinity_np(pthread_self(), sizeof s, &s); }

int main() {
    const int N = 20'000;
    for (int mode = 0; mode < 3; ++mode) {
        std::atomic<long> stamp{0};                       // thời điểm phát frame gần nhất (0: chưa có)
        std::mutex mu; std::condition_variable cv;
        std::vector<long> lat; lat.reserve(N);
        std::thread worker([&] {
            pin(2);
            for (int i = 0; i < N; ++i) {
                long t = 0;
                if (mode == 0) { while ((t = stamp.exchange(0, std::memory_order_acquire)) == 0) {} }
                else if (mode == 1) { std::unique_lock lk(mu); cv.wait(lk, [&] { return (t = stamp.exchange(0)) != 0; }); }
                else { while ((t = stamp.exchange(0)) == 0) std::this_thread::sleep_for(std::chrono::microseconds(50)); }
                lat.push_back(now_ns() - t);
            }
        });
        pin(0);
        for (int i = 0; i < N; ++i) {
            auto until = clk::now() + std::chrono::microseconds(100);
            while (clk::now() < until) {}                     // khoảng cách giữa hai frame
            if (mode == 1) { { std::lock_guard g(mu); stamp.store(now_ns()); } cv.notify_one(); }
            else stamp.store(now_ns(), std::memory_order_release);
            while (stamp.load() != 0) {}                      // chờ worker nhận rồi mới phát frame sau
        }
        worker.join();
        std::sort(lat.begin(), lat.end());
        const char* name[] = {"spin", "sleep (futex)", "timer 50 us"};
        std::printf("%-14s median %8.0f ns   p99 %8.0f ns   max %9.0f ns\n", name[mode],
                    double(lat[N / 2]), double(lat[N * 99 / 100]), double(lat.back()));
    }
}
