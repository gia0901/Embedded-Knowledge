// race: K thread cùng tăng một bộ đếm N lần mỗi thread, theo bốn cách. In kết quả, số lần cộng bị mất và ns mỗi lần cộng.
// Lời giải Lab 4.1. Build: g++ -O2 -std=c++20 -pthread -o race race.cpp
// Chạy:  ./race <K thread> <N mỗi thread> [tên một cách: load+store|fetch_add|mutex|spin|rieng+gop]
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

static const char* g_only = nullptr;

template <class F> void run(const char* name, int k, long n, F body, auto read) {
    if (g_only && std::strcmp(g_only, name) != 0) return;
    auto t0 = std::chrono::steady_clock::now();
    std::vector<std::thread> ts;
    for (int i = 0; i < k; ++i) ts.emplace_back([&] { for (long j = 0; j < n; ++j) body(); });
    for (auto& t : ts) t.join();
    double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / (double(k) * n);
    long got = read(), want = k * n;
    std::printf("%-12s ket qua %10ld / %ld  mat %5.1f%%  %6.2f ns/lan\n", name, got, want, 100.0 * (want - got) / want, ns);
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 4;
    long n = argc > 2 ? std::atol(argv[2]) : 10'000'000;
    if (argc > 3) g_only = argv[3];

    std::atomic<long> a{0};                       // load rồi store riêng: đúng kiểu "đọc, cộng, ghi" của counter++
    run("load+store", k, n, [&] { a.store(a.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed); }, [&] { return a.load(); });

    std::atomic<long> b{0};
    run("fetch_add", k, n, [&] { b.fetch_add(1, std::memory_order_relaxed); }, [&] { return b.load(); });   // một lệnh lock xadd

    long c = 0; std::mutex m;
    run("mutex", k, n, [&] { std::lock_guard g(m); ++c; }, [&] { return c; });

    long d = 0; std::atomic_flag f = ATOMIC_FLAG_INIT;      // spinlock tối giản: quay tới khi chiếm được cờ
    run("spin", k, n, [&] { while (f.test_and_set(std::memory_order_acquire)) {} ++d; f.clear(std::memory_order_release); }, [&] { return d; });

    if (g_only && std::strcmp(g_only, "rieng+gop") != 0) return 0;
    std::vector<long> part(k * 16, 0);            // mỗi thread một biến riêng cách nhau 128 byte, cộng dồn ở cuối
    std::atomic<int> id{0};
    auto t0 = std::chrono::steady_clock::now();
    std::vector<std::thread> ts;
    for (int i = 0; i < k; ++i) ts.emplace_back([&] { long& my = part[id++ * 16]; for (long j = 0; j < n; ++j) my = my + 1; });
    for (auto& t : ts) t.join();
    long sum = 0; for (int i = 0; i < k; ++i) sum += part[i * 16];
    double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / (double(k) * n);
    std::printf("%-12s ket qua %10ld / %ld  mat %5.1f%%  %6.2f ns/lan\n", "rieng+gop", sum, k * n, 100.0 * (k * n - sum) / (k * n), ns);
}
