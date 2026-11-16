// bench.h: đo thời gian dùng chung cho F01. Chạy hàm `reps` lần, in median và min tính bằng ns mỗi phần tử.
#pragma once
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

template <class F>
double bench_ns(const char* name, size_t items, int reps, F&& fn) {
    std::vector<double> t;
    fn();                                                    // chạy nháp một lần: nạp cache, page fault
    for (int r = 0; r < reps; ++r) {
        auto a = std::chrono::steady_clock::now();
        fn();
        auto b = std::chrono::steady_clock::now();
        t.push_back(std::chrono::duration<double, std::nano>(b - a).count() / items);
    }
    std::sort(t.begin(), t.end());
    std::printf("%-22s median %8.2f ns  min %8.2f ns  max %8.2f ns  (%d lan)\n", name, t[t.size() / 2], t[0], t.back(), reps);
    return t[t.size() / 2];
}

// Ngăn compiler bỏ đi phép tính mà kết quả không dùng tới.
template <class T> inline void keep(const T& v) { asm volatile("" : : "g"(&v) : "memory"); }
