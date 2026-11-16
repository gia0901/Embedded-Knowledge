// final: framestat từ "bản đầu tiên ai cũng viết" tới bản đã áp dụng F01, từng bước một, cùng dữ liệu.
// 16 triệu frame (256 MB) trong file frames.bin, 4096 device. Mỗi bước giữ mọi thay đổi của bước trước.
//   v0: read() 16 byte mỗi lần, ép con trỏ sang struct, std::map, struct thống kê xếp tùy ý   (Bài 6, 7, 3, 7)
//   v1: + read() 64 KB mỗi lần                                                             (Bài 6: gom lô)
//   v2: + parse bằng memcpy + ntoh                                                           (Bài 7: đúng, di động)
//   v3: + bảng vector theo dev thay cho std::map                                            (Bài 3: cache)
//   v4: + struct thống kê xếp lại 56 → 40 byte                                                (Bài 7: padding)
//   v5: + 4 thread, mỗi thread một đoạn file và một bảng riêng, gộp cuối                     (Bài 4: shard)
// Lời giải Lab 9.1. Build: g++ -O2 -std=c++20 -I.. -pthread -o final final.cpp      Chạy: ./final  |  ./final scale  |  ./final all N
#include "bench.h"
#include "frame.h"
#include <climits>
#include <fcntl.h>
#include <map>
#include <thread>
#include <string_view>
#include <unistd.h>

struct WireHdr { uint16_t magic, dev; uint8_t type, flags; uint16_t len; uint32_t seq; int32_t value; };
struct StatBad { uint8_t seen; int64_t sum; uint16_t last_seq_lo; int64_t n; int32_t min; uint8_t flags; int32_t max; int64_t alarms; };
struct StatGood { int64_t sum, n, alarms; int32_t min, max; uint16_t last_seq_lo; uint8_t seen, flags; };

static bool parse_cast(const uint8_t* p, Frame& f) {
    auto* h = reinterpret_cast<const WireHdr*>(p);
    if (ntohs(h->magic) != kMagic) return false;
    f.dev = ntohs(h->dev); f.type = h->type; f.flags = h->flags;
    f.seq = ntohl(h->seq); f.value = int32_t(ntohl(uint32_t(h->value)));
    return ntohs(h->len) == 4;
}
template <class S> static void update(S& s, const Frame& f) {
    if (!s.seen) { s.min = INT32_MAX; s.max = INT32_MIN; }
    s.seen = 1; s.last_seq_lo = uint16_t(f.seq); s.flags |= f.flags;
    if (f.type == ALARM) { ++s.alarms; return; }
    if (f.type != TEMP) return;
    ++s.n; s.sum += f.value; s.min = std::min(s.min, f.value); s.max = std::max(s.max, f.value);
}

constexpr int kDevs = 4096;
static size_t kN = 16'000'000;

// Một lần chạy cho một đoạn [from, to) của file. Chunk: số byte mỗi lần read. Table: std::map hoặc vector.
template <bool Cast, class Table> static void scan(int fd, size_t from, size_t to, size_t chunk, Table& t) {
    std::vector<uint8_t> b(chunk);
    Frame f;
    for (size_t off = from; off < to;) {
        ssize_t got = pread(fd, b.data(), std::min(chunk, to - off), off);
        if (got <= 0) break;
        for (ssize_t i = 0; i + ssize_t(kWire) <= got; i += kWire)
            if (Cast ? parse_cast(&b[i], f) : parse(&b[i], f)) update(t[f.dev], f);
        off += got;
    }
}
template <class Table> static long total(const Table& t) {
    long s = 0;
    if constexpr (requires { t.begin()->second; }) { for (auto& [k, v] : t) s += v.sum; }
    else { for (auto& v : t) s += v.sum; }
    return s;
}

// v5 với T thread, dùng cho Lab 9.4 (đo khả năng mở rộng).
static void scan_threads(int fd, size_t bytes, int T, long& out) {
    std::vector<std::vector<StatGood>> part(T, std::vector<StatGood>(kDevs));
    std::vector<std::thread> th;
    for (int k = 0; k < T; ++k)
        th.emplace_back([&, k] { scan<false>(fd, bytes / kWire * k / T * kWire, bytes / kWire * (k + 1) / T * kWire, 65536, part[k]); });
    for (auto& x : th) x.join();
    std::vector<StatGood> g(kDevs);                          // gộp: kDevs × T phép cộng
    for (int d = 0; d < kDevs; ++d) for (int k = 0; k < T; ++k) g[d].sum += part[k][d].sum;
    out = total(g);
}

int main(int argc, char** argv) {
    // ./final            : chạy v0..v5
    // ./final scale      : chỉ v5 với 1, 2, 3, 4 thread (Lab 9.4)
    // ./final all N      : v0..v5 với N frame (Lab 9.3, chạy dưới qemu cần N nhỏ)
    const bool scale = argc > 1 && std::string_view(argv[1]) == "scale";
    if (argc > 2) kN = std::atol(argv[2]);
    {   auto buf = make_frames(kN, kDevs);
        int fd = open("frames.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (write(fd, buf.data(), buf.size()) != ssize_t(buf.size())) { perror("write"); return 1; }
        close(fd); }
    const int fd = open("frames.bin", O_RDONLY);
    const size_t bytes = kN * kWire;
    long sums[6] = {};
    double t[6];
    if (scale) {
        double t1 = 0;
        for (int T = 1; T <= 4; ++T) {
            char name[32]; std::snprintf(name, sizeof name, "v5, %d thread", T);
            double x = bench_ns(name, kN, 5, [&] { scan_threads(fd, bytes, T, sums[0]); });
            if (T == 1) t1 = x;
            std::printf("    nhanh gap %.2f lan so voi 1 thread, tong %ld\n", t1 / x, sums[0]);
        }
        close(fd); unlink("frames.bin");
        return 0;
    }
    t[0] = bench_ns("v0 ban dau", kN, 3, [&] { std::map<uint16_t, StatBad> m; scan<true>(fd, 0, bytes, 16, m); sums[0] = total(m); });
    t[1] = bench_ns("v1 + read 64 KB", kN, 3, [&] { std::map<uint16_t, StatBad> m; scan<true>(fd, 0, bytes, 65536, m); sums[1] = total(m); });
    t[2] = bench_ns("v2 + memcpy/ntoh", kN, 3, [&] { std::map<uint16_t, StatBad> m; scan<false>(fd, 0, bytes, 65536, m); sums[2] = total(m); });
    t[3] = bench_ns("v3 + vector", kN, 3, [&] { std::vector<StatBad> v(kDevs); scan<false>(fd, 0, bytes, 65536, v); sums[3] = total(v); });
    t[4] = bench_ns("v4 + struct 40 B", kN, 3, [&] { std::vector<StatGood> v(kDevs); scan<false>(fd, 0, bytes, 65536, v); sums[4] = total(v); });
    t[5] = bench_ns("v5 + 4 thread", kN, 3, [&] { scan_threads(fd, bytes, 4, sums[5]); });
    close(fd); unlink("frames.bin");                      // xóa file 256 MB
    std::printf("tong nhiet do:");
    for (long s : sums) std::printf(" %ld", s);
    std::printf("\nv0 -> v5: nhanh gap %.0f lan\n", t[0] / t[5]);
}
