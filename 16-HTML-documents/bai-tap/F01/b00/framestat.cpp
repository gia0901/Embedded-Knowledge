// framestat v0: parse một buffer frame telemetry, cộng dồn thống kê theo device, đo ns mỗi frame.
// Dùng cho Làm trước và Lab 0.3. Build: g++ -O2 -std=c++20 -I.. -o framestat framestat.cpp
// Chạy:  ./framestat [số frame=4000000] [số device=64] [số lần đo=11]
#include "bench.h"
#include "frame.h"
#include <cstdio>
#include <cstdlib>

struct DevStat { long n = 0, sum = 0; int32_t min = INT32_MAX, max = INT32_MIN; long alarms = 0; };

int main(int argc, char** argv) {
    size_t n = argc > 1 ? std::atol(argv[1]) : 4'000'000;
    int devs = argc > 2 ? std::atoi(argv[2]) : 64;
    int reps = argc > 3 ? std::atoi(argv[3]) : 11;
    auto buf = make_frames(n, devs);
    std::vector<DevStat> st(devs);
    long bad = 0;
    auto run = [&] {
        for (auto& s : st) s = DevStat{};
        bad = 0;
        Frame f;
        for (size_t i = 0; i < n; ++i) {
            if (!parse(&buf[i * kWire], f)) { ++bad; continue; }
            DevStat& s = st[f.dev];
            if (f.type == ALARM) { ++s.alarms; continue; }
            if (f.type != TEMP) continue;
            ++s.n; s.sum += f.value;
            if (f.value < s.min) s.min = f.value;
            if (f.value > s.max) s.max = f.value;
        }
        keep(st);
    };
    double ns = bench_ns("framestat v0", n, reps, run);
    long temps = 0, alarms = 0;
    for (auto& s : st) { temps += s.n; alarms += s.alarms; }
    std::printf("%zu frame, %d device: %ld nhiet do, %ld canh bao, %ld hong; %.1f trieu frame/s\n",
                n, devs, temps, alarms, bad, 1e3 / ns);
    std::printf("dev 0: n=%ld tb=%.2f C min=%.2f max=%.2f\n", st[0].n, st[0].sum / 100.0 / st[0].n, st[0].min / 100.0, st[0].max / 100.0);
}
