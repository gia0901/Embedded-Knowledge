// Bài tập F03-5: Bọc và bóc header GTP-U (TS 29.281) có extension header PDU Session Container (TS 38.415).
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined ex5_gtpu.cpp -o ex5 && ./ex5
//
// Header GTP-U:
//   byte 0: version (3 bit, = 1) | PT (1 bit, = 1) | spare | E | S | PN      -> 0x30 không option, 0x34 khi có extension
//   byte 1: message type (0xFF = G-PDU mang gói người dùng, 0x01 = Echo Request)
//   byte 2-3: length = số byte SAU 8 byte đầu (gồm 4 byte option nếu có, extension và payload)
//   byte 4-7: TEID
//   Nếu E, S hoặc PN bật: thêm 4 byte: sequence (2), N-PDU number (1), next extension header type (1)
// Mỗi extension header: [length tính theo đơn vị 4 byte][nội dung][next extension header type]; type 0 = hết.
// PDU Session Container (type 0x85), length = 1:
//   byte 0: PDU type ở 4 bit cao (0 = downlink, 1 = uplink)   byte 1: QFI ở 6 bit thấp   byte 2: next type
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

enum class Gtp { OK, ECHO_REQUEST, TOO_SHORT, BAD_VERSION, BAD_LENGTH, BAD_EXT, UNSUPPORTED_TYPE };
struct GtpuInfo {
    uint32_t teid;
    int qfi;                  // -1 nếu không có PDU Session Container
    bool uplink;
    const uint8_t* inner;     // gói IP bên trong, trỏ vào pkt
    size_t inner_len;
};

// ===================== PHẦN CỦA BẠN =====================
// Ghi header GTP-U + gói inner vào out. qfi < 0: không có extension (header 8 byte).
// qfi >= 0: có 4 byte option (seq = 0, N-PDU = 0, next = 0x85) + PDU Session Container 4 byte (header 16 byte).
// Trả tổng số byte đã ghi, hoặc 0 nếu cap không đủ.
size_t gtpu_encap(uint8_t* out, size_t cap, uint32_t teid, int qfi, bool uplink, const uint8_t* inner, size_t n) {
    size_t hl = qfi < 0 ? 8 : 16;
    if (cap < hl + n || hl - 8 + n > 0xFFFF) return 0;
    out[0] = qfi < 0 ? 0x30 : 0x34;
    out[1] = 0xFF;
    size_t len = hl - 8 + n;
    out[2] = (uint8_t)(len >> 8); out[3] = (uint8_t)len;
    out[4] = (uint8_t)(teid >> 24); out[5] = (uint8_t)(teid >> 16); out[6] = (uint8_t)(teid >> 8); out[7] = (uint8_t)teid;
    if (qfi >= 0) {
        out[8] = out[9] = out[10] = 0;  // sequence, N-PDU
        out[11] = 0x85;                 // next: PDU Session Container
        out[12] = 1;                    // 1 x 4 byte
        out[13] = uplink ? 0x10 : 0x00;
        out[14] = (uint8_t)(qfi & 0x3F);
        out[15] = 0;                    // hết extension
    }
    std::memcpy(out + hl, inner, n);
    return hl + n;
}

// Kiểm tra theo thứ tự: n < 8 -> TOO_SHORT; version != 1 hoặc PT != 1 -> BAD_VERSION;
// 8 + length > n -> BAD_LENGTH (byte thừa sau đó bỏ qua); type 0x01 -> ECHO_REQUEST; type khác 0xFF -> UNSUPPORTED_TYPE.
// Có E/S/PN: cần thêm 4 byte trong phạm vi length, nếu không -> BAD_LENGTH. Chỉ đi theo chuỗi extension khi E bật.
// Duyệt chuỗi extension: length = 0 hoặc vượt phạm vi -> BAD_EXT; type khác 0x85 thì bỏ qua (nhảy qua);
// type 0x85 thì đọc PDU type và QFI. Phần còn lại là inner.
Gtp gtpu_decap(const uint8_t* pkt, size_t n, GtpuInfo* out) {
    if (n < 8) return Gtp::TOO_SHORT;
    if ((pkt[0] >> 5) != 1 || !(pkt[0] & 0x10)) return Gtp::BAD_VERSION;
    size_t end = 8 + (size_t)(pkt[2] << 8 | pkt[3]);
    if (end > n) return Gtp::BAD_LENGTH;
    if (pkt[1] == 0x01) return Gtp::ECHO_REQUEST;
    if (pkt[1] != 0xFF) return Gtp::UNSUPPORTED_TYPE;
    out->teid = (uint32_t)pkt[4] << 24 | (uint32_t)pkt[5] << 16 | (uint32_t)pkt[6] << 8 | pkt[7];
    out->qfi = -1; out->uplink = false;
    size_t pos = 8;
    if (pkt[0] & 0x07) {
        if (pos + 4 > end) return Gtp::BAD_LENGTH;
        uint8_t next = (pkt[0] & 0x04) ? pkt[pos + 3] : 0;
        pos += 4;
        while (next != 0) {
            if (pos >= end) return Gtp::BAD_EXT;
            size_t elen = (size_t)pkt[pos] * 4;
            if (elen == 0 || pos + elen > end) return Gtp::BAD_EXT;
            if (next == 0x85 && elen >= 4) {
                out->uplink = (pkt[pos + 1] >> 4) == 1;
                out->qfi = pkt[pos + 2] & 0x3F;
            }
            next = pkt[pos + elen - 1];
            pos += elen;
        }
    }
    out->inner = pkt + pos;
    out->inner_len = end - pos;
    return Gtp::OK;
}
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("  FAIL dòng %d: %s\n", __LINE__, #c); fails++; } } while (0)

int main() {
    const uint8_t ip[] = {0x45, 0, 0, 20, 1, 2, 3, 4, 64, 17, 0, 0, 10, 45, 0, 2, 8, 8, 8, 8};
    uint8_t buf[256];
    GtpuInfo g{};

    size_t n = gtpu_encap(buf, sizeof buf, 0x00ABCDEF, -1, false, ip, sizeof ip);
    const uint8_t h8[] = {0x30, 0xFF, 0x00, 0x14, 0x00, 0xAB, 0xCD, 0xEF};
    CHECK(n == 28 && std::memcmp(buf, h8, 8) == 0);
    CHECK(gtpu_decap(buf, n, &g) == Gtp::OK && g.teid == 0xABCDEF && g.qfi == -1 && g.inner_len == 20 &&
          std::memcmp(g.inner, ip, 20) == 0);

    n = gtpu_encap(buf, sizeof buf, 1, 9, true, ip, sizeof ip);
    const uint8_t h16[] = {0x34, 0xFF, 0x00, 0x1C, 0, 0, 0, 1, 0, 0, 0, 0x85, 0x01, 0x10, 0x09, 0x00};
    CHECK(n == 36 && std::memcmp(buf, h16, 16) == 0);
    CHECK(gtpu_decap(buf, n, &g) == Gtp::OK && g.teid == 1 && g.qfi == 9 && g.uplink && g.inner_len == 20);
    CHECK(gtpu_encap(buf, 35, 1, 9, true, ip, sizeof ip) == 0);           // thiếu chỗ

    // Có thêm padding Ethernet sau gói: phải dựa vào trường length
    uint8_t padded[64] = {};
    n = gtpu_encap(padded, sizeof padded, 7, 5, false, ip, sizeof ip);
    CHECK(gtpu_decap(padded, n + 10, &g) == Gtp::OK && g.inner_len == 20 && g.qfi == 5 && !g.uplink);

    // Chuỗi extension: một header lạ (type 0x40, dài 4 byte) đứng trước PDU Session Container
    const uint8_t chain[] = {0x34, 0xFF, 0x00, 0x0E, 0, 0, 0, 2,  0, 0, 0, 0x40,
                             0x01, 0x12, 0x34, 0x85,   0x01, 0x10, 0x03, 0x00,   0xAA, 0xBB};
    CHECK(gtpu_decap(chain, sizeof chain, &g) == Gtp::OK && g.qfi == 3 && g.inner_len == 2 && g.inner[0] == 0xAA);

    // Chỉ có S (sequence), không E: không được đi theo byte next
    const uint8_t seqonly[] = {0x32, 0xFF, 0x00, 0x06, 0, 0, 0, 3,  0x00, 0x05, 0, 0x85,  0xCC, 0xDD};
    CHECK(gtpu_decap(seqonly, sizeof seqonly, &g) == Gtp::OK && g.qfi == -1 && g.inner_len == 2 && g.inner[0] == 0xCC);

    const uint8_t echo[] = {0x32, 0x01, 0x00, 0x04, 0, 0, 0, 0, 0x12, 0x34, 0, 0};
    CHECK(gtpu_decap(echo, sizeof echo, &g) == Gtp::ECHO_REQUEST);
    CHECK(gtpu_decap(h8, 7, &g) == Gtp::TOO_SHORT);
    const uint8_t v2[] = {0x50, 0xFF, 0, 0, 0, 0, 0, 0};                    // version 2 (GTP-C v2)
    CHECK(gtpu_decap(v2, 8, &g) == Gtp::BAD_VERSION);
    const uint8_t pt0[] = {0x20, 0xFF, 0, 0, 0, 0, 0, 0};                   // PT = 0 (GTP')
    CHECK(gtpu_decap(pt0, 8, &g) == Gtp::BAD_VERSION);
    const uint8_t longlen[] = {0x30, 0xFF, 0x00, 0x10, 0, 0, 0, 1, 1, 2};
    CHECK(gtpu_decap(longlen, sizeof longlen, &g) == Gtp::BAD_LENGTH);
    const uint8_t noopt[] = {0x34, 0xFF, 0x00, 0x02, 0, 0, 0, 1, 0, 0};
    CHECK(gtpu_decap(noopt, sizeof noopt, &g) == Gtp::BAD_LENGTH);
    const uint8_t ext0[] = {0x34, 0xFF, 0x00, 0x08, 0, 0, 0, 1, 0, 0, 0, 0x85, 0x00, 0, 0, 0};
    CHECK(gtpu_decap(ext0, sizeof ext0, &g) == Gtp::BAD_EXT);
    const uint8_t extlong[] = {0x34, 0xFF, 0x00, 0x08, 0, 0, 0, 1, 0, 0, 0, 0x85, 0x05, 0, 0, 0};
    CHECK(gtpu_decap(extlong, sizeof extlong, &g) == Gtp::BAD_EXT);
    const uint8_t gtpc[] = {0x30, 0x10, 0, 0, 0, 0, 0, 0};
    CHECK(gtpu_decap(gtpc, 8, &g) == Gtp::UNSUPPORTED_TYPE);

    uint32_t s = 7;                                                         // fuzz, ASan bắt đọc ngoài buffer
    for (int i = 0; i < 50000; i++) {
        std::vector<uint8_t> r(s % 40);
        for (auto& b : r) { s = s * 1664525u + 1013904223u; b = (uint8_t)(s >> 24); }
        if (r.size() > 1) { r[0] = (uint8_t)(0x30 | (r[0] & 7)); r[1] = 0xFF; }
        gtpu_decap(r.data(), r.size(), &g);
        s = s * 1664525u + 1013904223u;
    }
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
