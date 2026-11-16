// So sánh chi phí: function call, system call rẻ nhất (getppid), clock_gettime qua vDSO
// và cùng clock_gettime nhưng ép vào kernel bằng syscall().
// Build: g++ -O2 -std=c++20 -o syscall_cost syscall_cost.cpp
#include <chrono>
#include <cstdio>
#include <ctime>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((noinline)) long plain_function(long x) { return x + 1; }

int main() {
    const int N = 2'000'000;
    long sink = 0;
    timespec ts;
    auto now = [] { return std::chrono::steady_clock::now(); };
    auto ns = [&](auto a, auto b) { return std::chrono::duration<double, std::nano>(b - a).count() / N; };

    auto t0 = now();
    for (int i = 0; i < N; ++i) sink += plain_function(i);
    auto t1 = now();
    for (int i = 0; i < N; ++i) sink += syscall(SYS_getppid);              // luôn vào kernel
    auto t2 = now();
    for (int i = 0; i < N; ++i) { clock_gettime(CLOCK_MONOTONIC, &ts); sink += ts.tv_nsec; }   // vDSO
    auto t3 = now();
    for (int i = 0; i < N; ++i) { syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts); sink += ts.tv_nsec; }
    auto t4 = now();

    std::printf("function call            : %6.1f ns\n", ns(t0, t1));
    std::printf("syscall getppid          : %6.1f ns\n", ns(t1, t2));
    std::printf("clock_gettime (vDSO)     : %6.1f ns\n", ns(t2, t3));
    std::printf("clock_gettime (syscall)  : %6.1f ns\n", ns(t3, t4));
    std::printf("(sink=%ld)\n", sink % 7);   // in ra để compiler không bỏ vòng lặp
}
