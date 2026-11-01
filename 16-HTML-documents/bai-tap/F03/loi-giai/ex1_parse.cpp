// Bài tập F03-1: Phân tích gói IPv4 + UDP và kiểm tra tính hợp lệ.
// Viết hàm parse_ipv4_udp(). Phần TEST ở dưới không sửa.
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined ex1_parse.cpp -o ex1 && ./ex1
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

enum class Err { OK, TOO_SHORT, BAD_VERSION, BAD_IHL, BAD_TOTAL_LEN, BAD_CHECKSUM, FRAGMENT, NOT_UDP, BAD_UDP_LEN };

struct UdpInfo {
    uint32_t src_ip, dst_ip;        // thứ tự byte của máy (host order)
    uint16_t src_port, dst_port;    // host order
    uint8_t  ttl;
    const uint8_t* payload;         // trỏ vào bên trong buf, không cấp phát
    size_t payload_len;
};

// ===================== PHẦN CỦA BẠN =====================
// Checksum Internet (RFC 1071) trên n byte: cộng các từ 16 bit big-endian theo one's complement, rồi đảo bit.
// Gợi ý: n lẻ thì byte cuối được coi là byte cao của một từ 16 bit có byte thấp bằng 0.
uint16_t inet_checksum(const uint8_t* p, size_t n) {
    uint32_t sum = 0;
    for (size_t i = 0; i + 1 < n; i += 2) sum += (uint32_t)(p[i] << 8 | p[i + 1]);
    if (n & 1) sum += (uint32_t)p[n - 1] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}

// Kiểm tra theo thứ tự sau, gặp lỗi đầu tiên thì trả mã lỗi đó:
//  1. len < 20                                   -> TOO_SHORT
//  2. version != 4                               -> BAD_VERSION
//  3. IHL < 5 hoặc IHL*4 > len                   -> BAD_IHL
//  4. total_length < IHL*4 hoặc > len            -> BAD_TOTAL_LEN   (byte thừa sau total_length là padding Ethernet, bỏ qua)
//  5. checksum header sai                        -> BAD_CHECKSUM    (checksum của cả header kể cả trường checksum phải bằng 0)
//  6. cờ MF bật hoặc fragment offset != 0        -> FRAGMENT
//  7. protocol != 17                             -> NOT_UDP
//  8. phần còn lại < 8 byte, hoặc trường length của UDP < 8 hoặc > phần còn lại -> BAD_UDP_LEN
// Hợp lệ: điền out (payload dài đúng UDP length - 8) và trả OK. Không kiểm tra checksum UDP.
Err parse_ipv4_udp(const uint8_t* buf, size_t len, UdpInfo* out) {
    if (len < 20) return Err::TOO_SHORT;
    if ((buf[0] >> 4) != 4) return Err::BAD_VERSION;
    size_t ihl = (size_t)(buf[0] & 0x0F) * 4;
    if (ihl < 20 || ihl > len) return Err::BAD_IHL;
    size_t total = (size_t)(buf[2] << 8 | buf[3]);
    if (total < ihl || total > len) return Err::BAD_TOTAL_LEN;
    if (inet_checksum(buf, ihl) != 0) return Err::BAD_CHECKSUM;
    uint16_t frag = (uint16_t)(buf[6] << 8 | buf[7]);
    if ((frag & 0x2000) || (frag & 0x1FFF)) return Err::FRAGMENT;
    if (buf[9] != 17) return Err::NOT_UDP;
    const uint8_t* u = buf + ihl;
    size_t rest = total - ihl;
    if (rest < 8) return Err::BAD_UDP_LEN;
    size_t ulen = (size_t)(u[4] << 8 | u[5]);
    if (ulen < 8 || ulen > rest) return Err::BAD_UDP_LEN;
    uint32_t s, d;
    std::memcpy(&s, buf + 12, 4);
    std::memcpy(&d, buf + 16, 4);
    out->src_ip = ntohl(s);
    out->dst_ip = ntohl(d);
    out->src_port = (uint16_t)(u[0] << 8 | u[1]);
    out->dst_port = (uint16_t)(u[2] << 8 | u[3]);
    out->ttl = buf[8];
    out->payload = u + 8;
    out->payload_len = ulen - 8;
    return Err::OK;
}
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("  FAIL dòng %d: %s\n", __LINE__, #c); fails++; } } while (0)

static std::vector<uint8_t> make_pkt(const char* payload, uint8_t opts_words = 0) {
    size_t ihl = 20 + opts_words * 4u, plen = std::strlen(payload);
    std::vector<uint8_t> p(ihl + 8 + plen, 0);
    p[0] = (uint8_t)(0x40 | (5 + opts_words));
    size_t total = p.size();
    p[2] = (uint8_t)(total >> 8); p[3] = (uint8_t)total;
    p[4] = 0x12; p[5] = 0x34; p[6] = 0x40;              // DF
    p[8] = 64; p[9] = 17;
    uint8_t src[4] = {10, 45, 0, 2}, dst[4] = {8, 8, 8, 8};
    std::memcpy(&p[12], src, 4); std::memcpy(&p[16], dst, 4);
    uint16_t c = inet_checksum(p.data(), ihl);
    p[10] = (uint8_t)(c >> 8); p[11] = (uint8_t)c;
    uint8_t* u = &p[ihl];
    u[0] = 0xC3; u[1] = 0x50;                            // 50000
    u[2] = 0x00; u[3] = 0x35;                            // 53
    u[4] = (uint8_t)((8 + plen) >> 8); u[5] = (uint8_t)(8 + plen);
    std::memcpy(u + 8, payload, plen);
    return p;
}
static void refix(std::vector<uint8_t>& p) {             // tính lại checksum sau khi sửa header
    size_t ihl = (size_t)(p[0] & 15) * 4;
    p[10] = p[11] = 0;
    uint16_t c = inet_checksum(p.data(), ihl);
    p[10] = (uint8_t)(c >> 8); p[11] = (uint8_t)c;
}

int main() {
    // Checksum: ví dụ header trong RFC/Wikipedia có checksum 0xB861
    uint8_t h[20] = {0x45,0x00,0x00,0x73,0x00,0x00,0x40,0x00,0x40,0x11,0x00,0x00,0xc0,0xa8,0x00,0x01,0xc0,0xa8,0x00,0xc7};
    CHECK(inet_checksum(h, 20) == 0xB861);
    uint8_t odd[3] = {0x01, 0x02, 0x03};
    CHECK(inet_checksum(odd, 3) == (uint16_t)~(0x0102 + 0x0300));

    UdpInfo u{};
    auto p = make_pkt("hello");
    CHECK(parse_ipv4_udp(p.data(), p.size(), &u) == Err::OK);
    CHECK(u.src_ip == 0x0A2D0002 && u.dst_ip == 0x08080808);
    CHECK(u.src_port == 50000 && u.dst_port == 53 && u.ttl == 64);
    CHECK(u.payload_len == 5 && std::memcmp(u.payload, "hello", 5) == 0);

    auto po = make_pkt("opt", 2);                         // header có option, IHL = 7
    CHECK(parse_ipv4_udp(po.data(), po.size(), &u) == Err::OK && u.payload_len == 3);

    auto pad = make_pkt("x"); pad.resize(pad.size() + 17, 0xEE);   // padding Ethernet
    CHECK(parse_ipv4_udp(pad.data(), pad.size(), &u) == Err::OK && u.payload_len == 1);

    CHECK(parse_ipv4_udp(p.data(), 19, &u) == Err::TOO_SHORT);
    auto v6 = p; v6[0] = 0x65; CHECK(parse_ipv4_udp(v6.data(), v6.size(), &u) == Err::BAD_VERSION);
    auto ih = p; ih[0] = 0x44; CHECK(parse_ipv4_udp(ih.data(), ih.size(), &u) == Err::BAD_IHL);
    auto ih2 = p; ih2[0] = 0x4F; CHECK(parse_ipv4_udp(ih2.data(), 40, &u) == Err::BAD_IHL);
    auto tl = p; tl[3] = (uint8_t)(tl[3] + 1); refix(tl);
    CHECK(parse_ipv4_udp(tl.data(), tl.size(), &u) == Err::BAD_TOTAL_LEN);
    auto cs = p; cs[8] = 63;                              // đổi TTL mà không sửa checksum
    CHECK(parse_ipv4_udp(cs.data(), cs.size(), &u) == Err::BAD_CHECKSUM);
    auto mf = p; mf[6] = 0x20; refix(mf); CHECK(parse_ipv4_udp(mf.data(), mf.size(), &u) == Err::FRAGMENT);
    auto off = p; off[6] = 0x00; off[7] = 0x10; refix(off); CHECK(parse_ipv4_udp(off.data(), off.size(), &u) == Err::FRAGMENT);
    auto tcp = p; tcp[9] = 6; refix(tcp); CHECK(parse_ipv4_udp(tcp.data(), tcp.size(), &u) == Err::NOT_UDP);
    auto ul = p; ul[25] = 7; CHECK(parse_ipv4_udp(ul.data(), ul.size(), &u) == Err::BAD_UDP_LEN);
    auto ul2 = p; ul2[25] = 200; CHECK(parse_ipv4_udp(ul2.data(), ul2.size(), &u) == Err::BAD_UDP_LEN);
    auto sh = make_pkt(""); sh.resize(24); sh[3] = 24; refix(sh);
    CHECK(parse_ipv4_udp(sh.data(), sh.size(), &u) == Err::BAD_UDP_LEN);

    // Fuzz: dữ liệu ngẫu nhiên không được làm chương trình đọc ra ngoài buffer (ASan sẽ báo)
    uint32_t seed = 1;
    for (int i = 0; i < 20000; i++) {
        std::vector<uint8_t> r(seed % 80);
        for (auto& b : r) { seed = seed * 1103515245u + 12345u; b = (uint8_t)(seed >> 16); }
        if (r.size() > 0) r[0] = (uint8_t)(0x40 | (r[0] & 15));
        parse_ipv4_udp(r.data(), r.size(), &u);
        seed = seed * 1103515245u + 12345u;
    }
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
