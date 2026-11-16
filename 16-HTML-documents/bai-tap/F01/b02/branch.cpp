// branch: framestat v0 trên ba cách sắp xếp cùng một tập frame: ngẫu nhiên, xen kẽ theo chu kỳ, gom theo loại.
// Cùng code, cùng số frame mỗi loại; chỉ thứ tự khác. Lời giải Lab 2.1.
// Build: g++ -O2 -std=c++20 -I.. -o branch branch.cpp
// Chạy: ./branch [số frame=4000000] [số lần đo=11] [chỉ chạy một kiểu: r c g R G; mặc định tất cả]
#include "bench.h"
#include "frame.h"
#include <algorithm>
#include <cstdlib>

struct DevStat { long n = 0, sum = 0; int32_t min = INT32_MAX, max = INT32_MIN; long alarms = 0; };

// Vòng lặp của framestat v0, giữ nguyên
static void run(const std::vector<uint8_t>& buf, std::vector<DevStat>& st) {
    for (auto& s : st) s = DevStat{};
    Frame f;
    for (size_t i = 0; i < buf.size() / kWire; ++i) {
        if (!parse(&buf[i * kWire], f)) continue;
        DevStat& s = st[f.dev];
        if (f.type == ALARM) { ++s.alarms; continue; }
        if (f.type != TEMP) continue;
        ++s.n; s.sum += f.value;
        if (f.value < s.min) s.min = f.value;
        if (f.value > s.max) s.max = f.value;
    }
    keep(st);
}

// Bản không nhánh theo loại: luôn làm đủ việc, dùng điều kiện làm hệ số 0/1. Lời giải Lab 2.2.
static void run_bl(const std::vector<uint8_t>& buf, std::vector<DevStat>& st) {
    for (auto& s : st) s = DevStat{};
    Frame f;
    for (size_t i = 0; i < buf.size() / kWire; ++i) {
        if (!parse(&buf[i * kWire], f)) continue;      // frame hỏng hiếm: nhánh này luôn đoán đúng
        DevStat& s = st[f.dev];
        const int32_t m = -int32_t(f.type == TEMP);    // m = -1 (mọi bit 1) nếu là nhiệt độ, ngược lại 0
        s.alarms += f.type == ALARM;
        s.n -= m;
        s.sum += f.value & m;                          // nhiệt độ: cộng value; loại khác: cộng 0
        s.min = std::min(s.min, (f.value & m) | (INT32_MAX & ~m));   // loại khác: so với INT32_MAX, không đổi gì
        s.max = std::max(s.max, (f.value & m) | (INT32_MIN & ~m));
    }
    keep(st);
}

// Sắp lại frame theo khóa (giữ nguyên nội dung từng frame)
template <class Key> std::vector<uint8_t> reorder(const std::vector<uint8_t>& buf, Key key) {
    size_t n = buf.size() / kWire;
    std::vector<size_t> idx(n);
    for (size_t i = 0; i < n; ++i) idx[i] = i;
    std::stable_sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { return key(a) < key(b); });
    std::vector<uint8_t> out(buf.size());
    for (size_t i = 0; i < n; ++i) std::memcpy(&out[i * kWire], &buf[idx[i] * kWire], kWire);
    return out;
}

int main(int argc, char** argv) {
    const size_t n = argc > 1 ? std::atol(argv[1]) : 4'000'000;
    const int reps = argc > 2 ? std::atoi(argv[2]) : 11;
    const char only = argc > 3 ? argv[3][0] : 0;
    auto want = [&](char c) { return !only || only == c; };
    auto rnd = make_frames(n, 64);
    auto type_of = [&](size_t i) { return rnd[i * kWire + 4]; };
    // xen kẽ: frame thứ k của mỗi loại đứng cạnh nhau -> T H B T H B ... (ALARM rải đều)
    std::vector<long> rank(n); { long c[4] = {}; for (size_t i = 0; i < n; ++i) rank[i] = c[type_of(i)]++; }
    auto cyc = reorder(rnd, [&](size_t i) { return rank[i] * 4 + type_of(i); });
    auto grp = reorder(rnd, [&](size_t i) { return type_of(i); });
    std::vector<DevStat> st(64);
    auto total = [&] { long t = 0; for (auto& s : st) t += s.sum; return t; };
    long t1 = 0, t2 = 0, t3 = 0;
    if (want('r')) { bench_ns("ngau nhien", n, reps, [&] { run(rnd, st); }); t1 = total(); }
    if (want('c')) { bench_ns("xen ke theo chu ky", n, reps, [&] { run(cyc, st); }); t2 = total(); }
    if (want('g')) { bench_ns("gom theo loai", n, reps, [&] { run(grp, st); }); t3 = total(); }
    long b1 = 0, b3 = 0;
    if (want('R')) { bench_ns("khong nhanh, ngau nhien", n, reps, [&] { run_bl(rnd, st); }); b1 = total(); }
    if (want('G')) { bench_ns("khong nhanh, gom loai", n, reps, [&] { run_bl(grp, st); }); b3 = total(); }
    std::printf("tong nhiet do (phai bang nhau): %ld %ld %ld %ld %ld\n", t1, t2, t3, b1, b3);
}
