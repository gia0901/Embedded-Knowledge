// measure: ba cái bẫy khi tự đo thời gian một đoạn code. Lời giải Lab 0.4.
// Build: g++ -O2 -std=c++20 -I.. -o measure measure.cpp      Chạy: ./measure
#include "bench.h"
#include <chrono>
#include <cstdio>
#include <memory>

constexpr size_t N = 16 << 20;                       // 16 Mi số int = 64 MB

static double ms_since(std::chrono::steady_clock::time_point a) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - a).count();
}

int main() {
    // Bẫy 1: lần chạy đầu tiên trả thêm chi phí page fault (kernel cấp trang RAM khi chạm lần đầu)
    std::unique_ptr<int[]> a(new int[N]);            // chưa khởi tạo: chưa có trang RAM nào
    for (int pass = 1; pass <= 3; ++pass) {
        auto t = std::chrono::steady_clock::now();
        for (size_t i = 0; i < N; ++i) a[i] = int(i);
        keep(a[N / 2]);
        std::printf("bay 1  ghi 64 MB lan %d: %7.2f ms\n", pass, ms_since(t));
    }
    // Bẫy 2: kết quả không được dùng thì compiler bỏ cả vòng lặp
    auto sum_unused = [&] { long s = 0; for (size_t i = 0; i < N; ++i) s += a[i]; };
    auto sum_kept = [&] { long s = 0; for (size_t i = 0; i < N; ++i) s += a[i]; keep(s); };
    bench_ns("bay 2  khong keep()", N, 11, sum_unused);
    bench_ns("bay 2  co keep()", N, 11, sum_kept);
    // Bẫy 3: nhiễu giữa các lần đo. In cả 21 lần đã sắp xếp để thấy độ trải.
    std::vector<double> t;
    for (int r = 0; r < 21; ++r) {
        auto s = std::chrono::steady_clock::now();
        sum_kept();
        t.push_back(ms_since(s));
    }
    std::sort(t.begin(), t.end());
    std::printf("bay 3  21 lan (ms): min %.2f  p25 %.2f  median %.2f  p75 %.2f  max %.2f\n",
                t[0], t[5], t[10], t[15], t[20]);
}
