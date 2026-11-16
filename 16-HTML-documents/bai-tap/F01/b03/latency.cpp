// latency: đường cong độ trễ bộ nhớ bằng pointer chasing. Mỗi phần tử chiếm đúng một cache line và trỏ tới
// phần tử kế tiếp theo một hoán vị ngẫu nhiên, nên lần đọc sau phụ thuộc lần đọc trước và prefetcher không đoán được.
// Lời giải Lab 3.1. Build: g++ -O2 -std=c++20 -o latency latency.cpp      Chạy: ./latency [KB tối đa=1048576]
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

struct alignas(64) Node { Node* next; char pad[56]; };     // 64 byte = 1 cache line

double chase_ns(size_t bytes, bool shuffle) {
    size_t n = bytes / sizeof(Node);
    std::vector<Node> v(n);
    std::vector<size_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    if (shuffle) std::shuffle(order.begin() + 1, order.end(), std::mt19937(1));
    for (size_t i = 0; i < n; ++i) v[order[i]].next = &v[order[(i + 1) % n]];
    Node* p = &v[order[0]];
    for (size_t i = 0; i < n; ++i) p = p->next;              // chạy nháp một vòng: nạp cache, page fault
    const long steps = 20'000'000;
    auto a = std::chrono::steady_clock::now();
    for (long i = 0; i < steps; ++i) p = p->next;
    auto b = std::chrono::steady_clock::now();
    if (p == nullptr) std::puts("");                          // dùng p để compiler không bỏ vòng lặp
    return std::chrono::duration<double, std::nano>(b - a).count() / steps;
}

int main(int argc, char** argv) {
    size_t max_kb = argc > 1 ? std::atol(argv[1]) : 1 << 20;
    std::printf("%10s %12s %12s\n", "kich thuoc", "ngau nhien", "tuan tu");
    for (size_t kb = 4; kb <= max_kb; kb *= 2) {
        double r = chase_ns(kb * 1024, true), s = chase_ns(kb * 1024, false);
        if (kb < 1024) std::printf("%7zu KB %9.2f ns %9.2f ns\n", kb, r, s);
        else std::printf("%7zu MB %9.2f ns %9.2f ns\n", kb / 1024, r, s);
    }
}
