// mispredict: đo cái giá của một lần đoán sai nhánh. Một nhánh lấy với xác suất p (dữ liệu ngẫu nhiên),
// tăng p từ 0 tới 50%: thời gian tăng gần tuyến tính theo tỉ lệ đoán sai. Độ dốc = chi phí một lần sai.
// Lời giải Lab 2.4. Build: g++ -O2 -std=c++20 -I.. -o mispredict mispredict.cpp     Chạy: ./mispredict
#include "bench.h"
#include <algorithm>
#include <random>

__attribute__((noinline)) long walk(const uint8_t* x, size_t n, uint8_t thr) {
    long s = 0;
    for (size_t i = 0; i < n; ++i) {
        if (x[i] < thr) {            // nhánh lấy với xác suất thr/256
            s += x[i];
            asm volatile("");        // lệnh asm rỗng chỉ ở một phía: compiler không được đổi nhánh thành cmov
        }
    }
    return s;
}

int main() {
    const size_t n = 4'000'000;
    std::vector<uint8_t> x(n);
    std::mt19937 rng(7);
    for (auto& v : x) v = uint8_t(rng());
    std::printf("  p(lay)   ns/phan tu\n");
    for (int pct : {0, 5, 10, 20, 30, 40, 50}) {
        uint8_t thr = uint8_t(pct * 256 / 100);
        char name[32]; std::snprintf(name, sizeof name, "p = %2d%%", pct);
        bench_ns(name, n, 11, [&] { long s = walk(x.data(), n, thr); keep(s); });
    }
    std::sort(x.begin(), x.end());                 // đối chứng: cùng dữ liệu, đã sắp xếp
    bench_ns("p = 50%, da sap xep", n, 11, [&] { long s = walk(x.data(), n, 128); keep(s); });
}
