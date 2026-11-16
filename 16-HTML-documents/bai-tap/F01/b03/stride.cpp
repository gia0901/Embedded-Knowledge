// stride: (1) cộng một mảng 64 MB với bước nhảy 1, 2, 4… phần tử: thời gian MỖI LẦN ĐỌC tăng cho tới khi
// mỗi lần đọc rơi vào một cache line mới (bước 16 int = 64 byte). (2) Duyệt ma trận 4096×4096 theo hàng và
// theo cột. Lời giải Lab 3.3. Build: g++ -O2 -std=c++20 -I.. -o stride stride.cpp      Chạy: ./stride
#include "bench.h"
#include <cstdint>

int main() {
    const size_t N = 16 << 20;                                   // 16 Mi int = 64 MB
    std::vector<int32_t> a(N, 1);
    for (size_t s : {1, 2, 4, 8, 16}) {
        char name[48]; std::snprintf(name, sizeof name, "buoc %2zu int (%3zu B)", s, s * 4);
        long sum = 0;
        bench_ns(name, N / s, 7, [&] { long t = 0; for (size_t i = 0; i < N; i += s) t += a[i]; sum = t; keep(sum); });
    }
    const size_t R = 4096, C = 4096;                             // ma trận R×C lưu theo hàng (row-major)
    long s1 = 0, s2 = 0;
    bench_ns("ma tran theo hang", R * C, 7, [&] {
        long t = 0; for (size_t r = 0; r < R; ++r) for (size_t c = 0; c < C; ++c) t += a[r * C + c]; s1 = t; keep(s1); });
    bench_ns("ma tran theo cot", R * C, 7, [&] {
        long t = 0; for (size_t c = 0; c < C; ++c) for (size_t r = 0; r < R; ++r) t += a[r * C + c]; s2 = t; keep(s2); });
    std::printf("tong: %ld %ld\n", s1, s2);
}
