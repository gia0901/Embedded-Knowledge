// spawn_cost: so chi phí tạo + chờ một thread với tạo + chờ một process (fork + waitpid),
// khi process cha nhỏ và khi cha đã chạm 256 MB. Lời giải Lab 2.2.
// Build: g++ -O2 -std=c++20 -pthread -o spawn_cost spawn_cost.cpp
#include <chrono>
#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

static void* noop(void*) { return nullptr; }

template <class F> double avg_us(int n, F f) {
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < n; ++i) f();
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count() / n;
}

static void report(const char* tag) {
    double th = avg_us(2000, [] { pthread_t t; pthread_create(&t, nullptr, noop, nullptr); pthread_join(t, nullptr); });
    double pr = avg_us(500, [] { pid_t p = fork(); if (p == 0) _exit(0); waitpid(p, nullptr, 0); });
    std::printf("%-22s thread: %6.1f us   process: %7.1f us\n", tag, th, pr);
}

int main() {
    report("cha nho (~4 MB)");
    size_t sz = 256u << 20;
    char* big = static_cast<char*>(mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    std::memset(big, 1, sz);                         // chạm hết: page table có 65536 mục cần chép khi fork
    report("cha da cham 256 MB");
}
