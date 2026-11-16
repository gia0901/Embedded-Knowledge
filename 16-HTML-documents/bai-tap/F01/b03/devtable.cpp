// devtable: bảng thống kê theo device của framestat cài bằng ba cấu trúc: mảng (vector, chỉ số = dev),
// std::map, std::unordered_map. Đo với 64, 4096, 65536 device. Lời giải Lab 3.2.
// Build: g++ -O2 -std=c++20 -I.. -o devtable devtable.cpp      Chạy: ./devtable
#include "bench.h"
#include "frame.h"
#include <map>
#include <unordered_map>

struct DevStat { long n = 0, sum = 0; int32_t min = INT32_MAX, max = INT32_MIN; long alarms = 0; };

template <class Table> void run(const std::vector<uint8_t>& buf, Table& st) {
    Frame f;
    for (size_t i = 0; i < buf.size() / kWire; ++i) {
        if (!parse(&buf[i * kWire], f)) continue;
        DevStat& s = st[f.dev];                         // chỗ duy nhất khác nhau giữa ba bản
        if (f.type == ALARM) { ++s.alarms; continue; }
        if (f.type != TEMP) continue;
        ++s.n; s.sum += f.value;
        if (f.value < s.min) s.min = f.value;
        if (f.value > s.max) s.max = f.value;
    }
}

int main() {
    const size_t n = 4'000'000;
    for (int devs : {64, 4096, 65536}) {
        auto buf = make_frames(n, devs);
        std::printf("--- %d device, bang ~%zu KB (mang)\n", devs, devs * sizeof(DevStat) / 1024);
        long t[3] = {};
        std::vector<DevStat> a(devs);
        bench_ns("vector", n, 7, [&] { for (auto& s : a) s = DevStat{}; run(buf, a); keep(a); });
        for (auto& s : a) t[0] += s.sum;
        std::map<uint16_t, DevStat> m;
        bench_ns("std::map", n, 7, [&] { m.clear(); run(buf, m); keep(m); });
        for (auto& [k, s] : m) t[1] += s.sum;
        std::unordered_map<uint16_t, DevStat> u;
        bench_ns("std::unordered_map", n, 7, [&] { u.clear(); run(buf, u); keep(u); });
        for (auto& [k, s] : u) t[2] += s.sum;
        std::printf("tong: %ld %ld %ld\n", t[0], t[1], t[2]);
    }
    // Đối chứng: cấp phát bảng 65536 ô nhưng traffic chỉ từ 64 device. Cái quyết định là working set.
    auto buf = make_frames(n, 64);
    std::vector<DevStat> big(65536);
    bench_ns("vector 65536 o, 64 dev", n, 7, [&] { for (auto& s : big) s = DevStat{}; run(buf, big); keep(big); });
}
