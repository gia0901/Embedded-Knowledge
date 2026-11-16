// assoc: conflict miss. Đi vòng qua k cache line cách nhau đúng S byte, lặp lại nhiều lần.
// L1d của máy lab: 48 KB, 12 way, 64 set; hai địa chỉ cách nhau bội của 4096 byte rơi vào cùng một set.
// Với S = 4096, quá 12 line là tràn set dù tổng dữ liệu chỉ vài trăm byte. Với S = 4160 (4096 + 64) các line
// rải sang set khác nhau. Lời giải Lab 3.4. Build: g++ -O2 -std=c++20 -o assoc assoc.cpp     Chạy: ./assoc
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <vector>

double walk_ns(size_t k, size_t stride) {
    std::vector<char> mem(k * stride + 64);
    auto at = [&](size_t i) { return reinterpret_cast<void**>(&mem[i * stride]); };
    for (size_t i = 0; i < k; ++i) *at(i) = at((i + 1) % k);   // vòng con trỏ qua k line
    void** p = at(0);
    for (size_t i = 0; i < 1000; ++i) p = static_cast<void**>(*p);
    const long steps = 20'000'000;
    auto a = std::chrono::steady_clock::now();
    for (long i = 0; i < steps; ++i) p = static_cast<void**>(*p);
    auto b = std::chrono::steady_clock::now();
    if (!p) std::puts("");
    return std::chrono::duration<double, std::nano>(b - a).count() / steps;
}

int main() {
    std::printf("%4s %16s %16s\n", "k", "buoc 4096 B", "buoc 4160 B");
    for (size_t k : {4, 8, 12, 13, 16, 24, 32, 48, 64})
        std::printf("%4zu %13.2f ns %13.2f ns\n", k, walk_ns(k, 4096), walk_ns(k, 4160));
}
