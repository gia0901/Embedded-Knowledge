// uart.h: phần dùng chung của Lab 10.2: mở cổng serial ở chế độ raw, CRC-16, COBS.
#pragma once
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <vector>

// Mở tty, tắt mọi xử lý của line discipline (cfmakeraw), 8N1, tốc độ baud (pty bỏ qua tốc độ).
inline int open_raw(const char* path, speed_t baud = B115200) {
    int fd = open(path, O_RDWR | O_NOCTTY);
    if (fd < 0) { perror(path); return -1; }
    termios t{};
    tcgetattr(fd, &t);
    cfmakeraw(&t);                     // không ICANON, ICRNL, IXON, ISIG, OPOST, ECHO
    t.c_cflag |= CLOCAL | CREAD;
    t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;   // read() trả về ngay khi có ít nhất 1 byte
    cfsetispeed(&t, baud); cfsetospeed(&t, baud);
    tcsetattr(fd, TCSANOW, &t);
    return fd;
}

// CRC-16/CCITT-FALSE: đa thức 0x1021, khởi tạo 0xFFFF.
inline uint16_t crc16(const uint8_t* p, size_t n) {
    uint16_t c = 0xFFFF;
    while (n--) {
        c ^= uint16_t(*p++) << 8;
        for (int i = 0; i < 8; i++) c = (c & 0x8000) ? (c << 1) ^ 0x1021 : c << 1;
    }
    return c;
}

// COBS: thay mọi byte 0 để dùng 0x00 làm dấu kết thúc khung. Thêm tối đa 1 byte mỗi 254 byte.
inline std::vector<uint8_t> cobs_encode(const uint8_t* p, size_t n) {
    std::vector<uint8_t> out(1);
    size_t code_at = 0; uint8_t code = 1;
    for (size_t i = 0; i < n; i++) {
        if (p[i] == 0) { out[code_at] = code; code_at = out.size(); out.push_back(0); code = 1; continue; }
        out.push_back(p[i]);
        if (++code == 0xFF) { out[code_at] = code; code_at = out.size(); out.push_back(0); code = 1; }
    }
    out[code_at] = code;
    return out;                        // chưa có 0x00 kết thúc
}

// Giải COBS (không gồm 0x00 kết thúc). false nếu khung hỏng.
inline bool cobs_decode(const std::vector<uint8_t>& in, std::vector<uint8_t>& out) {
    out.clear();
    size_t i = 0;
    while (i < in.size()) {
        uint8_t code = in[i++];
        if (code == 0 || i + code - 1 > in.size()) return false;
        for (int k = 1; k < code; k++) out.push_back(in[i++]);
        if (code != 0xFF && i < in.size()) out.push_back(0);
    }
    return true;
}
