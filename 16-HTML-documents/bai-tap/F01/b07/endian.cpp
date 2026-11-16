// endian: cùng một frame trên dây, đọc ra theo ba cách, trên máy little-endian (x86-64) và big-endian (s390x).
// Lời giải Lab 7.1 và 7.4. Build: g++ -O2 -std=c++20 -I.. -o endian endian.cpp      Chạy: ./endian
// Bản big-endian: s390x-linux-gnu-g++ -O2 -std=c++20 -I.. -static -o endian.s390x endian.cpp && qemu-s390x ./endian.s390x
#include "frame.h"
#include <bit>
#include <cstdio>

int main() {
    std::printf("may nay: %s-endian\n", std::endian::native == std::endian::little ? "little" : "big");
    uint32_t x = 0x12345678;
    const auto* b = reinterpret_cast<const uint8_t*>(&x);       // xem byte của một số trong bộ nhớ (hợp lệ: uint8_t)
    std::printf("0x12345678 trong bo nho: %02x %02x %02x %02x\n", b[0], b[1], b[2], b[3]);

    auto buf = make_frames(1, 4, 7);                             // một frame, giống hệt trên mọi máy
    const uint8_t* p = buf.data();
    std::printf("frame tren day:");
    for (int i = 0; i < 16; ++i) std::printf(" %02x", p[i]);
    std::printf("\n");

    uint16_t m; uint32_t v;
    std::memcpy(&m, p, 2); std::memcpy(&v, p + 12, 4);
    std::printf("memcpy, khong doi : magic 0x%04x  value %d\n", m, int32_t(v));
    std::printf("memcpy + ntoh     : magic 0x%04x  value %d\n", ntohs(m), int32_t(ntohl(v)));
    uint32_t s = uint32_t(p[12]) << 24 | uint32_t(p[13]) << 16 | uint32_t(p[14]) << 8 | p[15];
    std::printf("dich byte         : magic 0x%04x  value %d\n", unsigned(p[0] << 8 | p[1]), int32_t(s));
}
