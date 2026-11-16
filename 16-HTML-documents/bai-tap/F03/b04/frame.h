// frame.h: frame cảm biến 16 byte dùng chung cho F03 (big-endian trên dây).
// magic 0x7E1D | dev (2) | seq (4) | type (1) | flags (1) | temp x100 (2) | value (4)
#pragma once
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>

constexpr uint16_t kMagic = 0x7E1D;
constexpr int kFrame = 16;

struct Frame { uint16_t dev; uint32_t seq; uint8_t type, flags; int16_t temp; uint32_t value; };

inline void put_frame(uint8_t* p, const Frame& f) {
    uint16_t m = htons(kMagic), d = htons(f.dev), t = htons((uint16_t)f.temp);
    uint32_t s = htonl(f.seq), v = htonl(f.value);
    memcpy(p, &m, 2); memcpy(p + 2, &d, 2); memcpy(p + 4, &s, 4);
    p[8] = f.type; p[9] = f.flags; memcpy(p + 10, &t, 2); memcpy(p + 12, &v, 4);
}

inline bool get_frame(const uint8_t* p, Frame& f) {     // false nếu sai magic
    uint16_t m, d, t; uint32_t s, v;
    memcpy(&m, p, 2);
    if (ntohs(m) != kMagic) return false;
    memcpy(&d, p + 2, 2); memcpy(&s, p + 4, 4); memcpy(&t, p + 10, 2); memcpy(&v, p + 12, 4);
    f.dev = ntohs(d); f.seq = ntohl(s); f.type = p[8]; f.flags = p[9]; f.temp = (int16_t)ntohs(t); f.value = ntohl(v);
    return true;
}
