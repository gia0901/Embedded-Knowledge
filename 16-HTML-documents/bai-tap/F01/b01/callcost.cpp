// callcost: cùng việc parse 4 triệu frame, gọi parse theo ba cách: inline, qua hàm ở file khác
// (out-parameter), qua hàm ở file khác trả struct theo giá trị. Lời giải Lab 1.2.
// Build: g++ -O2 -std=c++20 -I.. -o callcost callcost.cpp parse1.cpp      Chạy: ./callcost
#include "bench.h"
#include "frame.h"

bool parse_out(const uint8_t* p, Frame& f);   // định nghĩa trong parse1.cpp: compiler không inline được
Frame parse_ret(const uint8_t* p);

int main() {
    const size_t n = 4'000'000;
    auto buf = make_frames(n, 64);
    long sum = 0;
    bench_ns("inline parse()", n, 11, [&] {
        sum = 0; Frame f;
        for (size_t i = 0; i < n; ++i) if (parse(&buf[i * kWire], f)) sum += f.value;
        keep(sum);
    });
    long s1 = sum;
    bench_ns("goi parse_out()", n, 11, [&] {
        sum = 0; Frame f;
        for (size_t i = 0; i < n; ++i) if (parse_out(&buf[i * kWire], f)) sum += f.value;
        keep(sum);
    });
    long s2 = sum;
    bench_ns("goi parse_ret()", n, 11, [&] {
        sum = 0;
        for (size_t i = 0; i < n; ++i) sum += parse_ret(&buf[i * kWire]).value;
        keep(sum);
    });
    std::printf("tong: %ld %ld %ld\n", s1, s2, sum);
}
