// wire.h: đóng khung tin trên TCP cho F03 Bài 7. Mỗi tin: 4 byte độ dài (big-endian) + payload.
#pragma once
#include <cstdint>
#include <string>

constexpr uint32_t kMaxMsg = 1 << 20;            // tin dài hơn 1 MB coi là dữ liệu hỏng

inline void put_msg(std::string& out, const char* p, uint32_t n) {
    char h[4] = {char(n >> 24), char(n >> 16), char(n >> 8), char(n)};
    out.append(h, 4);
    out.append(p, n);
}

// Nạp byte tuỳ ý bằng feed(), lấy ra từng tin hoàn chỉnh bằng next().
struct Decoder {
    std::string buf;
    size_t off = 0;                              // byte đầu chưa xử lý; tránh erase đầu chuỗi mỗi tin
    void feed(const char* p, size_t n) {
        if (off > 0 && off * 2 > buf.size()) { buf.erase(0, off); off = 0; }
        buf.append(p, n);
    }
    // 1: có tin trong msg; 0: cần thêm byte; -1: độ dài vượt kMaxMsg
    int next(std::string& msg) {
        if (buf.size() - off < 4) return 0;
        const unsigned char* h = (const unsigned char*)buf.data() + off;
        uint32_t n = uint32_t(h[0]) << 24 | h[1] << 16 | h[2] << 8 | h[3];
        if (n > kMaxMsg) return -1;
        if (buf.size() - off < 4 + size_t(n)) return 0;
        msg.assign(buf, off + 4, n);
        off += 4 + n;
        return 1;
    }
    size_t pending() const { return buf.size() - off; }
};
