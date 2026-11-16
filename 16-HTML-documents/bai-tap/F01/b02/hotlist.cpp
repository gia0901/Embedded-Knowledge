// hotlist: lọc chỉ số các frame nhiệt độ vượt ngưỡng (gửi cảnh báo). Bản có nhánh và bản không nhánh.
// Lời giải Lab 2.2. Build: g++ -O2 -std=c++20 -I.. -o hotlist hotlist.cpp
// Chạy: ./hotlist [ngưỡng, centi-°C = 3300]
#include "bench.h"
#include "frame.h"
#include <cstdlib>

static size_t filter_branch(const std::vector<uint8_t>& buf, uint32_t* out, int32_t thr) {
    size_t k = 0, n = buf.size() / kWire;
    for (size_t i = 0; i < n; ++i) {
        const uint8_t* p = &buf[i * kWire];
        uint32_t v; std::memcpy(&v, p + 12, 4);
        if (p[4] == TEMP && int32_t(ntohl(v)) > thr) out[k++] = uint32_t(i);   // nhánh khó đoán
    }
    return k;
}

static size_t filter_nobranch(const std::vector<uint8_t>& buf, uint32_t* out, int32_t thr) {
    size_t k = 0, n = buf.size() / kWire;
    for (size_t i = 0; i < n; ++i) {
        const uint8_t* p = &buf[i * kWire];
        uint32_t v; std::memcpy(&v, p + 12, 4);
        out[k] = uint32_t(i);                                                   // luôn ghi
        k += (p[4] == TEMP) & (int32_t(ntohl(v)) > thr);                         // chỉ tiến khi đạt
    }
    return k;
}

int main(int argc, char** argv) {
    const int32_t thr = argc > 1 ? std::atoi(argv[1]) : 3300;
    const size_t n = 4'000'000;
    auto buf = make_frames(n, 64);
    std::vector<uint32_t> out(n + 1);
    size_t k1 = 0, k2 = 0;
    bench_ns("co nhanh", n, 11, [&] { k1 = filter_branch(buf, out.data(), thr); keep(out); });
    bench_ns("khong nhanh", n, 11, [&] { k2 = filter_nobranch(buf, out.data(), thr); keep(out); });
    std::printf("nguong %.2f C: %zu / %zu frame dat (%.1f%%), hai ban %s\n", thr / 100.0, k1, n, 100.0 * k1 / n,
                k1 == k2 ? "khop" : "KHAC");
}
