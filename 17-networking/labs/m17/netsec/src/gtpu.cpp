#include "netsec/gtpu.hpp"

namespace netsec {

std::optional<GtpuHeader> parse_gtpu(const std::uint8_t* d, std::size_t len) {
    if (d == nullptr || len < 8) return std::nullopt;
    GtpuHeader h{};
    h.version = d[0] >> 5;
    h.pt = (d[0] >> 4) & 1;
    h.flags = d[0] & 0x07;                                        // E (0x04), S (0x02), PN (0x01)
    if (h.version != 1 || h.pt != 1) return std::nullopt;
    h.msg_type = d[1];
    h.length = static_cast<std::uint16_t>(d[2] << 8 | d[3]);
    h.teid = std::uint32_t(d[4]) << 24 | std::uint32_t(d[5]) << 16 | std::uint32_t(d[6]) << 8 | d[7];
    const std::size_t end = 8 + std::size_t(h.length);
    if (end > len) return std::nullopt;                           // FIX 1: length khai báo vượt dữ liệu thật
    std::size_t off = 8;
    if (h.flags != 0) {
        if (end < 12) return std::nullopt;                        // FIX 2: thiếu 4 byte tuỳ chọn
        if (h.flags & 0x02) h.seq = static_cast<std::uint16_t>(d[8] << 8 | d[9]);
        std::uint8_t next = (h.flags & 0x04) ? d[11] : 0;
        off = 12;
        int count = 0;
        while (next != 0) {
            if (off >= end || ++count > 16) return std::nullopt;  // FIX 3: hết dữ liệu, hoặc chuỗi extension vô lý
            const std::size_t n = std::size_t(d[off]) * 4;
            if (n == 0 || off + n > end) return std::nullopt;     // FIX 4: độ dài 0 (lặp vô hạn) hoặc vượt packet
            h.ext_types.push_back(next);
            next = d[off + n - 1];
            off += n;
        }
    }
    h.payload_offset = off;
    return h;
}

}  // namespace netsec
