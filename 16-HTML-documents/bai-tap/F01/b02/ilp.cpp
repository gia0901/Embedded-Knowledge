// ilp: tính trung bình nhiệt độ (double) bằng 1, 2, 4, 8 biến cộng dồn. Phép cộng double có độ trễ
// ~4 chu kỳ; nhiều chuỗi độc lập cho phép CPU chồng các phép cộng lên nhau. Lời giải Lab 2.3.
// Build: g++ -O2 -std=c++20 -I.. -o ilp ilp.cpp      Chạy: ./ilp
#include "bench.h"
#include "frame.h"

template <int K> double avg(const std::vector<double>& t) {
    double acc[K] = {};
    size_t i = 0, n = t.size();
    for (; i + K <= n; i += K)
        for (int k = 0; k < K; ++k) acc[k] += t[i + k];        // K chuỗi cộng độc lập
    for (; i < n; ++i) acc[0] += t[i];
    double s = 0;
    for (int k = 0; k < K; ++k) s += acc[k];
    return s / n;
}

int main() {
    const size_t n = 200'000;                                   // 1,6 MB: vừa L2, để không bị giới hạn bởi RAM
    auto buf = make_frames(n, 64);
    std::vector<double> t(n);
    Frame f;
    for (size_t i = 0; i < n; ++i) { parse(&buf[i * kWire], f); t[i] = f.value / 100.0; }
    double r[4];
    bench_ns("1 bien cong don", n, 21, [&] { r[0] = avg<1>(t); keep(r); });
    bench_ns("2 bien", n, 21, [&] { r[1] = avg<2>(t); keep(r); });
    bench_ns("4 bien", n, 21, [&] { r[2] = avg<4>(t); keep(r); });
    bench_ns("8 bien", n, 21, [&] { r[3] = avg<8>(t); keep(r); });
    std::printf("trung binh: %.10f %.10f %.10f %.10f\n", r[0], r[1], r[2], r[3]);
}
