// frame.h: định dạng frame telemetry dùng chung cho F01 (giống một header packet mạng).
// Trên dây: 16 byte, mọi trường nhiều byte là big-endian (network byte order).
//   0  magic  u16 = 0x7E1D       4  type  u8 (0 nhiệt độ, 1 độ ẩm, 2 cảnh báo, 3 heartbeat)
//   2  dev    u16                5  flags u8
//   6  len    u16 = 4 (độ dài payload)   8  seq u32   12  value i32 (nhiệt độ × 100, °C)
#pragma once
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <random>
#include <vector>

constexpr size_t kWire = 16;
constexpr uint16_t kMagic = 0x7E1D;
enum : uint8_t { TEMP = 0, HUM = 1, ALARM = 2, BEAT = 3 };

struct Frame {            // dạng đã parse, byte order của máy
    uint16_t dev;
    uint8_t type, flags;
    uint32_t seq;
    int32_t value;
};

// Sinh n frame cho `devs` device. alarm_pct: phần trăm frame loại ALARM, phần còn lại chia đều TEMP/HUM/BEAT.
inline std::vector<uint8_t> make_frames(size_t n, int devs, uint32_t seed = 1, int alarm_pct = 2) {
    std::vector<uint8_t> buf(n * kWire);
    std::mt19937 rng(seed);
    for (size_t i = 0; i < n; ++i) {
        uint8_t* p = &buf[i * kWire];
        static const uint8_t other[3] = {TEMP, HUM, BEAT};
        uint8_t type = int(rng() % 100) < alarm_pct ? ALARM : other[rng() % 3];
        uint16_t m = htons(kMagic), d = htons(uint16_t(rng() % devs)), len = htons(4);
        uint32_t seq = htonl(uint32_t(i));
        int32_t v = int32_t(2000 + rng() % 1500);                 // 20,00 tới 34,99 °C
        uint32_t vn = htonl(uint32_t(v));
        std::memcpy(p, &m, 2); std::memcpy(p + 2, &d, 2);
        p[4] = type; p[5] = 0;
        std::memcpy(p + 6, &len, 2); std::memcpy(p + 8, &seq, 4); std::memcpy(p + 12, &vn, 4);
    }
    return buf;
}

// Parse một frame. Dùng memcpy rồi ntoh: đúng với mọi alignment và mọi byte order của máy (Bài 7).
inline bool parse(const uint8_t* p, Frame& f) {
    uint16_t m, d, len; uint32_t seq, v;
    std::memcpy(&m, p, 2);
    if (ntohs(m) != kMagic) return false;
    std::memcpy(&d, p + 2, 2); std::memcpy(&len, p + 6, 2);
    std::memcpy(&seq, p + 8, 4); std::memcpy(&v, p + 12, 4);
    f.dev = ntohs(d); f.type = p[4]; f.flags = p[5];
    f.seq = ntohl(seq); f.value = int32_t(ntohl(v));
    return ntohs(len) == 4;
}
