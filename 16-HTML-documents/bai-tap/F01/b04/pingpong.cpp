// pingpong: hai thread ghim vào hai CPU, chuyền qua lại một biến atomic. Mỗi lượt, cache line chứa biến
// phải chuyển từ nhân này sang nhân kia (MESI). Thời gian một chiều ≈ độ trễ chuyển line giữa hai nhân.
// Lời giải Lab 4.4. Build: g++ -O2 -std=c++20 -pthread -o pingpong pingpong.cpp    Chạy: ./pingpong  hoặc  ./pingpong c0 c1 [N]
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>
#include <sched.h>
#include <thread>

static void pin(int cpu) {
    cpu_set_t s; CPU_ZERO(&s); CPU_SET(cpu, &s);
    pthread_setaffinity_np(pthread_self(), sizeof s, &s);
}

double one_way_ns(int c0, int c1, int N = 200'000) {
    alignas(64) std::atomic<int> flag{0};
    std::thread other([&] {
        pin(c1);
        for (int i = 0; i < N; ++i) {
            while (flag.load(std::memory_order_acquire) != 2 * i + 1) {}   // chờ lượt lẻ
            flag.store(2 * i + 2, std::memory_order_release);              // trả về
        }
    });
    pin(c0);
    auto a = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        flag.store(2 * i + 1, std::memory_order_release);
        while (flag.load(std::memory_order_acquire) != 2 * i + 2) {}
    }
    auto b = std::chrono::steady_clock::now();
    other.join();
    return std::chrono::duration<double, std::nano>(b - a).count() / (2.0 * N);
}

int main(int argc, char** argv) {
    if (argc >= 3) {                                    // ./pingpong c0 c1 [N]: chỉ đo một cặp
        int N = argc > 3 ? std::atoi(argv[3]) : 200'000;
        std::printf("CPU %s -> %s: %.1f ns\n", argv[1], argv[2], one_way_ns(std::atoi(argv[1]), std::atoi(argv[2]), N));
        return 0;
    }
    unsigned n = std::thread::hardware_concurrency();
    std::printf("mot chieu (ns), %u CPU\n     ", n);
    for (unsigned j = 0; j < n; ++j) std::printf("%8u", j);
    std::printf("\n");
    for (unsigned i = 0; i < n; ++i) {
        std::printf("%4u ", i);
        for (unsigned j = 0; j < n; ++j) i == j ? (void)std::printf("%8s", "-") : (void)std::printf("%8.1f", one_way_ns(i, j));
        std::printf("\n");
    }
}
