// asmtest: kiểm tra sum_values (assembly) với bản C++ tham chiếu, rồi đo tốc độ. Lời giải Lab 1.4.
// Build: g++ -O2 -std=c++20 -I.. -o asmtest asmtest.cpp sum_values.S
//        g++ -O2 -std=c++20 -I.. -DBROKEN -o asmtest_bad asmtest.cpp sum_values.S
#include "bench.h"
#include "frame.h"

extern "C" long sum_values(const uint8_t* buf, size_t n);

long sum_ref(const uint8_t* buf, size_t n) {
    long s = 0;
    for (size_t i = 0; i < n; ++i) { uint32_t v; std::memcpy(&v, buf + i * kWire + 12, 4); s += int32_t(ntohl(v)); }
    return s;
}

int main() {
    auto buf = make_frames(1'000'000, 64);
    int fail = 0;
    long total = 0;                                  // biến sống qua các lần gọi: compiler hay đặt vào thanh ghi callee-saved
    for (size_t n : {0ul, 1ul, 7ul, 1000ul, 1'000'000ul}) {
        long a = sum_values(buf.data(), n), b = sum_ref(buf.data(), n);
        total += n;
        std::printf("n=%-8zu asm=%-12ld c++=%-12ld %s\n", n, a, b, a == b ? "khop" : "SAI");
        fail += a != b;
    }
    std::printf("tong n da thu: %ld (dung phai la 1001008)\n", total);
    const size_t n = 1'000'000;
    long s = 0;
    bench_ns("sum_values (asm)", n, 21, [&] { s = sum_values(buf.data(), n); keep(s); });
    bench_ns("sum_ref (c++ -O2)", n, 21, [&] { s = sum_ref(buf.data(), n); keep(s); });
    return fail;
}
