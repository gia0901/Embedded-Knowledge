// falseshare: K thread, mỗi thread tăng biến đếm RIÊNG của mình N lần. Hai cách xếp các biến:
// sát nhau (cùng cache line 64 byte) và mỗi biến một cache line (alignas(64)). Lời giải Lab 4.3.
// Build: g++ -O2 -std=c++20 -pthread -o falseshare falseshare.cpp
// Chạy:  ./falseshare <K thread> <N>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

struct Packed { std::atomic<long> v{0}; };                 // 8 byte: 8 biến chung một cache line
struct alignas(64) Padded { std::atomic<long> v{0}; };     // 64 byte: mỗi biến một cache line

template <class Slot> double bench(int k, long n) {
    std::vector<Slot> slots(k);
    auto t0 = std::chrono::steady_clock::now();
    std::vector<std::thread> ts;
    for (int i = 0; i < k; ++i)
        ts.emplace_back([&, i] { for (long j = 0; j < n; ++j) slots[i].v.fetch_add(1, std::memory_order_relaxed); });
    for (auto& t : ts) t.join();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 4;
    long n = argc > 2 ? std::atol(argv[2]) : 20'000'000;
    std::printf("%d thread x %ld lan: sat nhau %.0f ms, moi bien mot cache line %.0f ms\n",
                k, n, bench<Packed>(k, n), bench<Padded>(k, n));
}
