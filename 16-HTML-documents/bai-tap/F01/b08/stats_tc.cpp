// stats_tc: giống stats.cpp, nhưng stat_soa_mask được build hai bản (máy x86-64 nào cũng chạy được, và AVX2)
// bằng target_clones; lúc nạp chương trình, glibc hỏi CPU rồi chọn bản phù hợp.
// Lời giải Lab 8.4. Build: g++ -O3 -std=c++20 -I.. -o stats_tc stats_tc.cpp   (không cần -march)
// Thử trên CPU cũ: qemu-x86_64 -cpu Westmere ./stats_tc 8192
#include "bench.h"
#include "frame.h"
#include <climits>
#include <cstdlib>

struct Rec { int64_t ts; uint32_t seq; uint16_t dev; uint8_t type, flags; int32_t value; uint32_t crc; };
static_assert(sizeof(Rec) == 24);
struct Stat { long n = 0, sum = 0; int32_t mn = INT32_MAX, mx = INT32_MIN; };

__attribute__((noinline)) Stat stat_aos(const Rec* r, size_t n) {
    long cnt = 0, sum = 0; int32_t mn = INT32_MAX, mx = INT32_MIN;
    for (size_t i = 0; i < n; ++i) {
        const bool t = r[i].type == TEMP;
        const int32_t v = r[i].value;
        cnt += t; sum += t ? v : 0;
        mn = std::min(mn, t ? v : INT32_MAX); mx = std::max(mx, t ? v : INT32_MIN);
    }
    return {cnt, sum, mn, mx};
}
__attribute__((noinline)) Stat stat_soa(const uint8_t* type, const int32_t* val, size_t n) {
    long cnt = 0, sum = 0; int32_t mn = INT32_MAX, mx = INT32_MIN;
    for (size_t i = 0; i < n; ++i) {
        const bool t = type[i] == TEMP;
        const int32_t v = val[i];
        cnt += t; sum += t ? v : 0;
        mn = std::min(mn, t ? v : INT32_MAX); mx = std::max(mx, t ? v : INT32_MIN);
    }
    return {cnt, sum, mn, mx};
}

// Cùng phép tính, viết bằng mặt nạ (Bài 2): m = -1 nếu là TEMP, 0 nếu không. Compiler vector hóa được dạng này.
__attribute__((noinline, target_clones("default", "arch=x86-64-v3"))) Stat stat_soa_mask(const uint8_t* type, const int32_t* val, size_t n) {
    long cnt = 0, sum = 0; int32_t mn = INT32_MAX, mx = INT32_MIN;
    for (size_t i = 0; i < n; ++i) {
        const int32_t m = -int32_t(type[i] == TEMP);
        const int32_t v = val[i];
        cnt -= m; sum += v & m;
        mn = std::min(mn, (v & m) | (INT32_MAX & ~m));
        mx = std::max(mx, (v & m) | (INT32_MIN & ~m));
    }
    return {cnt, sum, mn, mx};
}
__attribute__((noinline)) Stat stat_aos_mask(const Rec* r, size_t n) {
    long cnt = 0, sum = 0; int32_t mn = INT32_MAX, mx = INT32_MIN;
    for (size_t i = 0; i < n; ++i) {
        const int32_t m = -int32_t(r[i].type == TEMP);
        const int32_t v = r[i].value;
        cnt -= m; sum += v & m;
        mn = std::min(mn, (v & m) | (INT32_MAX & ~m));
        mx = std::max(mx, (v & m) | (INT32_MIN & ~m));
    }
    return {cnt, sum, mn, mx};
}

int main(int argc, char** argv) {
    std::vector<size_t> sizes = {8'192, 262'144, 16'000'000};   // 8K và 256K: SoA nằm trong L1/L2; 16M: vượt cache
    if (argc > 1) sizes = {size_t(std::atol(argv[1]))};
    const size_t total = 256'000'000;                        // tổng số phần tử xử lý mỗi lần đo
    for (size_t n : sizes) {
        auto buf = make_frames(n, 64);
        std::vector<Rec> aos(n); std::vector<uint8_t> type(n); std::vector<int32_t> val(n);
        Frame f;
        for (size_t i = 0; i < n; ++i) {
            parse(&buf[i * kWire], f);
            aos[i] = {long(i), f.seq, f.dev, f.type, f.flags, f.value, 0};
            type[i] = f.type; val[i] = f.value;
        }
        const size_t reps = total / n;
        Stat a, s, am, sm;
        std::printf("--- %zu ban ghi: AoS %zu KB, SoA %zu KB\n", n, n * sizeof(Rec) / 1024, n * 5 / 1024);
        bench_ns("AoS", reps * n, 5, [&] { for (size_t k = 0; k < reps; ++k) { a = stat_aos(aos.data(), n); keep(a); } });
        bench_ns("SoA", reps * n, 5, [&] { for (size_t k = 0; k < reps; ++k) { s = stat_soa(type.data(), val.data(), n); keep(s); } });
        bench_ns("AoS, mat na", reps * n, 5, [&] { for (size_t k = 0; k < reps; ++k) { am = stat_aos_mask(aos.data(), n); keep(am); } });
        bench_ns("SoA, mat na", reps * n, 5, [&] { for (size_t k = 0; k < reps; ++k) { sm = stat_soa_mask(type.data(), val.data(), n); keep(sm); } });
        for (const Stat* x : {&a, &s, &am, &sm}) std::printf("  n %ld sum %ld min %d max %d\n", x->n, x->sum, x->mn, x->mx);
    }
}
