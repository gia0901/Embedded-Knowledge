// layout7: cùng các trường thống kê, hai thứ tự khai báo. Compiler chèn padding để mỗi trường căn theo kích thước
// của nó, nên thứ tự quyết định sizeof. Đo framestat với 32.768 device (bảng cỡ L2, Bài 3) cho hai bố cục.
// Lời giải Lab 7.3. Build: g++ -O2 -std=c++20 -I.. -o layout7 layout7.cpp     Chạy: ./layout7 [số device=32768]
#include "bench.h"
#include "frame.h"
#include <cstddef>
#include <cstdlib>

struct StatBad {               // thứ tự "theo nghĩa": cờ cạnh nhau với thứ nó mô tả
    uint8_t seen;              // đã có frame nào chưa
    int64_t sum;
    uint16_t last_seq_lo;
    int64_t n;
    int32_t min;
    uint8_t flags;
    int32_t max;
    int64_t alarms;
};
struct StatGood {              // cùng trường, xếp từ lớn tới nhỏ
    int64_t sum, n, alarms;
    int32_t min, max;
    uint16_t last_seq_lo;
    uint8_t seen, flags;
};

#define OFF(T, m) std::printf("  %-12s offset %2zu  size %zu\n", #m, offsetof(T, m), sizeof(T::m))

template <class S> void update(S& s, const Frame& f) {
    s.seen = 1; s.last_seq_lo = uint16_t(f.seq); s.flags |= f.flags;
    if (f.type == ALARM) { ++s.alarms; return; }
    if (f.type != TEMP) return;
    ++s.n; s.sum += f.value; s.min = std::min(s.min, f.value); s.max = std::max(s.max, f.value);
}

int main(int argc, char** argv) {
    std::printf("StatBad  sizeof %zu, alignof %zu\n", sizeof(StatBad), alignof(StatBad));
    OFF(StatBad, seen); OFF(StatBad, sum); OFF(StatBad, last_seq_lo); OFF(StatBad, n);
    OFF(StatBad, min); OFF(StatBad, flags); OFF(StatBad, max); OFF(StatBad, alarms);
    std::printf("StatGood sizeof %zu, alignof %zu\n", sizeof(StatGood), alignof(StatGood));

    const int devs = argc > 1 ? std::atoi(argv[1]) : 32768;
    const size_t n = 8'000'000;
    auto buf = make_frames(n, devs);
    long r[2] = {};
    auto bench = [&](auto tag, const char* name, long& out) {
        using S = decltype(tag);
        std::vector<S> st(devs);
        bench_ns(name, n, 7, [&] {
            for (auto& s : st) { s = S{}; s.min = INT32_MAX; s.max = INT32_MIN; }
            Frame f;
            for (size_t i = 0; i < n; ++i) if (parse(&buf[i * kWire], f)) update(st[f.dev], f);
        });
        long sum = 0; for (auto& s : st) sum += s.sum; out = sum;
        std::printf("  bang %zu KB\n", sizeof(S) * devs / 1024);
    };
    bench(StatBad{}, "StatBad", r[0]);
    bench(StatGood{}, "StatGood", r[1]);
    std::printf("tong: %ld %ld\n", r[0], r[1]);
}
