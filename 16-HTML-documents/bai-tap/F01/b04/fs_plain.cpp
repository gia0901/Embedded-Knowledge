// fs_plain: đối chứng cho falseshare. 4 thread, mỗi thread tăng bộ đếm riêng 50 triệu lần, hai kiểu:
// atomic fetch_add (lock add) và ghi thường qua volatile. Bộ đếm sát nhau hoặc cách nhau 64 byte.
// Build: g++ -O2 -std=c++20 -pthread -o fs_plain fs_plain.cpp      Chạy: ./fs_plain
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

struct A { std::atomic<long> n{0}; };
struct alignas(64) Apad { std::atomic<long> n{0}; };
struct V { volatile long n = 0; };
struct alignas(64) Vpad { volatile long n = 0; };

template <class S> double run_ms(int T) {
    std::vector<S> s(T);
    auto a = std::chrono::steady_clock::now();
    std::vector<std::thread> th;
    for (int t = 0; t < T; ++t)
        th.emplace_back([&, t] {
            for (long i = 0; i < 50'000'000; ++i) {
                if constexpr (requires { s[t].n.fetch_add(1); }) s[t].n.fetch_add(1, std::memory_order_relaxed);
                else s[t].n = s[t].n + 1;
            }
        });
    for (auto& x : th) x.join();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - a).count();
}

int main() {
    std::printf("atomic   sat nhau %6.0f ms   cach 64 B %6.0f ms\n", run_ms<A>(4), run_ms<Apad>(4));
    std::printf("volatile sat nhau %6.0f ms   cach 64 B %6.0f ms\n", run_ms<V>(4), run_ms<Vpad>(4));
}
