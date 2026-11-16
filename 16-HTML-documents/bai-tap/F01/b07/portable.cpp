// portable: ba đoạn code "chạy đúng trên PC" và chạy lại trên máy big-endian (s390x qua qemu).
//   magic_fast : bỏ ntohs, so với hằng đã đảo sẵn 0x1d7e (một "tối ưu" hay gặp)
//   bit-field  : đọc byte flags qua struct bit-field; thứ tự bit trong bit-field do compiler và ABI quyết định
//   gửi struct : chép thẳng struct ra dây; byte order và padding đi theo máy gửi
// Lời giải Lab 7.4. Build: g++ -O2 -std=c++20 -I.. -o portable portable.cpp
//   s390x-linux-gnu-g++ -O2 -std=c++20 -I.. -static -o portable.s390x portable.cpp && qemu-s390x ./portable.s390x
#include "frame.h"
#include <cstdio>

static bool magic_fast(const uint8_t* p) { uint16_t m; std::memcpy(&m, p, 2); return m == 0x1d7e; }
static bool magic_ok(const uint8_t* p) { uint16_t m; std::memcpy(&m, p, 2); return ntohs(m) == kMagic; }

struct FlagBits { uint8_t urgent : 1, retry : 1, reserved : 6; };   // ý định: bit 0 (giá trị 0x01) là urgent

struct Report { uint16_t dev; int32_t avg; };                        // gửi thẳng ra dây: 2 + (2 padding) + 4

static size_t serialize(const Report& r, uint8_t* out) {             // cách đúng: 6 byte big-endian, không padding
    uint16_t d = htons(r.dev); uint32_t a = htonl(uint32_t(r.avg));
    std::memcpy(out, &d, 2); std::memcpy(out + 2, &a, 4);
    return 6;
}

int main() {
    auto buf = make_frames(1, 4, 7);
    uint8_t* p = buf.data();
    p[5] = 0x01;                                                     // theo đặc tả: flags bit 0 = urgent
    std::printf("magic_fast %d, magic_ok %d\n", magic_fast(p), magic_ok(p));

    FlagBits fb; std::memcpy(&fb, p + 5, 1);
    std::printf("flags 0x%02x: bit-field urgent=%d retry=%d | phep AND: urgent=%d retry=%d\n",
                p[5], fb.urgent, fb.retry, p[5] & 0x01, (p[5] >> 1) & 1);

    Report r{0x0102, 2500};
    uint8_t wire[sizeof r]; std::memcpy(wire, &r, sizeof r);
    std::printf("Report sizeof %zu, tren day:", sizeof r);
    for (uint8_t c : wire) std::printf(" %02x", c);
    std::printf("\n");
    uint8_t out[6]; size_t k = serialize(r, out);
    std::printf("serialize %zu byte:", k);
    for (size_t i = 0; i < k; ++i) std::printf(" %02x", out[i]);
    std::printf("\n");
}
