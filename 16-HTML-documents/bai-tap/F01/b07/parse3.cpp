// parse3: ba cách parse header frame big-endian.
//   cast   : ép con trỏ buffer sang struct rồi ntoh từng trường (hay gặp, nhưng là hành vi không xác định)
//   memcpy : memcpy từng trường rồi ntoh (frame.h)
//   shift  : ghép từng byte bằng dịch bit, không phụ thuộc byte order của máy
// Lời giải Lab 7.2. Build: g++ -O2 -std=c++20 -I.. -o parse3 parse3.cpp      Chạy: ./parse3 [offset=0]
// Kiểm tra UB: g++ -O1 -g -std=c++20 -I.. -fsanitize=undefined -o parse3_ub parse3.cpp && ./parse3_ub 1
#include "bench.h"
#include "frame.h"
#include <cstdlib>

struct WireHdr {                     // đúng bố cục trên dây: 2+2+1+1+2+4+4 = 16 byte, không padding
    uint16_t magic, dev;
    uint8_t type, flags;
    uint16_t len;
    uint32_t seq;
    int32_t value;
};
static_assert(sizeof(WireHdr) == 16 && alignof(WireHdr) == 4);

__attribute__((noinline)) bool parse_cast(const uint8_t* p, Frame& f) {
    auto* h = reinterpret_cast<const WireHdr*>(p);              // UB nếu p không căn 4, và vi phạm strict aliasing
    if (ntohs(h->magic) != kMagic) return false;
    f.dev = ntohs(h->dev); f.type = h->type; f.flags = h->flags;
    f.seq = ntohl(h->seq); f.value = int32_t(ntohl(uint32_t(h->value)));
    return ntohs(h->len) == 4;
}
__attribute__((noinline)) bool parse_memcpy(const uint8_t* p, Frame& f) { return parse(p, f); }
static inline uint16_t be16(const uint8_t* p) { return uint16_t(p[0] << 8 | p[1]); }
static inline uint32_t be32(const uint8_t* p) { return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | p[3]; }
__attribute__((noinline)) bool parse_shift(const uint8_t* p, Frame& f) {
    if (be16(p) != kMagic) return false;
    f.dev = be16(p + 2); f.type = p[4]; f.flags = p[5];
    f.seq = be32(p + 8); f.value = int32_t(be32(p + 12));
    return be16(p + 6) == 4;
}

int main(int argc, char** argv) {
    const size_t off = argc > 1 ? std::atoi(argv[1]) : 0;      // 1: buffer lệch 1 byte so với căn 4
    const size_t n = 4'000'000;
    auto src = make_frames(n, 64);
    std::vector<uint8_t> buf(src.size() + 8);
    std::memcpy(buf.data() + off, src.data(), src.size());
    const uint8_t* base = buf.data() + off;
    long sums[3] = {};
    auto run = [&](auto fn, long& out) {
        long s = 0; Frame f;
        for (size_t i = 0; i < n; ++i) if (fn(base + i * kWire, f) && f.type == TEMP) s += f.value;
        out = s;
    };
    bench_ns("cast", n, 7, [&] { run(parse_cast, sums[0]); });
    bench_ns("memcpy + ntoh", n, 7, [&] { run(parse_memcpy, sums[1]); });
    bench_ns("dich byte", n, 7, [&] { run(parse_shift, sums[2]); });
    bench_ns("cast, do lai lan 2", n, 7, [&] { run(parse_cast, sums[0]); });   // cùng hàm, đo sau cùng
    std::printf("offset %zu, tong: %ld %ld %ld\n", off, sums[0], sums[1], sums[2]);
}
