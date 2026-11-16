// iphdr: giải mã header IPv4/IPv6 và kiểm checksum IPv4 (RFC 1071).
// Hai chế độ:  ./iphdr <interface> <so goi>     đọc gói thật bằng AF_PACKET (root)
//              ./iphdr --hex "4500 002c ..."      giải mã byte dán vào (không cần root)
// Build: g++ -O2 -std=c++20 -o iphdr iphdr.cpp
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Internet checksum: cộng các từ 16 bit theo bù một, gập phần nhớ, đảo bit.
uint16_t inet_csum(const uint8_t* p, size_t n) {
    uint32_t s = 0;
    for (size_t i = 0; i + 1 < n; i += 2) s += (uint32_t)(p[i] << 8 | p[i + 1]);
    if (n & 1) s += (uint32_t)(p[n - 1] << 8);
    while (s >> 16) s = (s & 0xffff) + (s >> 16);
    return (uint16_t)~s;
}
static uint16_t be16(const uint8_t* p) { return (uint16_t)(p[0] << 8 | p[1]); }

void decode(const uint8_t* ip, size_t len) {
    int ver = ip[0] >> 4;
    if (ver == 4 && len >= 20) {
        int ihl = (ip[0] & 0x0f) * 4;
        uint16_t frag = be16(ip + 6);
        uint16_t stored = be16(ip + 10);
        bool ok = inet_csum(ip, ihl) == 0;             // checksum đúng thì tổng cả header (gồm checksum) ra 0
        printf("IPv4 ihl=%d tos=0x%02x len=%u id=%u flags=%s%s off=%u ttl=%u proto=%u csum=0x%04x %s  %u.%u.%u.%u -> %u.%u.%u.%u\n",
               ihl, ip[1], be16(ip + 2), be16(ip + 4), frag & 0x4000 ? "DF" : "", frag & 0x2000 ? "MF" : "",
               (frag & 0x1fff) * 8, ip[8], ip[9], stored, ok ? "dung" : "SAI",
               ip[12], ip[13], ip[14], ip[15], ip[16], ip[17], ip[18], ip[19]);
    } else if (ver == 6 && len >= 40) {
        uint32_t w = (uint32_t)ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3];
        char s[INET6_ADDRSTRLEN], d[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, ip + 8, s, sizeof s); inet_ntop(AF_INET6, ip + 24, d, sizeof d);
        printf("IPv6 tclass=0x%02x flow=0x%05x payload=%u next=%u hop=%u  %s -> %s  (khong co checksum)\n",
               (w >> 20) & 0xff, w & 0xfffff, be16(ip + 4), ip[6], ip[7], s, d);
    } else {
        printf("khong phai IPv4/IPv6 (version %d, %zu byte)\n", ver, len);
    }
}

int main(int argc, char** argv) {
    if (argc >= 3 && !strcmp(argv[1], "--hex")) {
        std::vector<uint8_t> b;
        for (const char* p = argv[2]; *p;) {
            if (!isxdigit((unsigned char)*p)) { p++; continue; }
            char t[3] = {p[0], p[1], 0}; b.push_back((uint8_t)strtol(t, nullptr, 16)); p += 2;
        }
        decode(b.data(), b.size());
        return 0;
    }
    if (argc < 2) { fprintf(stderr, "dung: %s <if> [n] | --hex \"...\"\n", argv[0]); return 1; }
    int fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));   // SOCK_DGRAM: kernel bóc sẵn header Ethernet
    sockaddr_ll sll{}; sll.sll_family = AF_PACKET; sll.sll_protocol = htons(ETH_P_IP);
    sll.sll_ifindex = if_nametoindex(argv[1]);
    if (fd < 0 || bind(fd, (sockaddr*)&sll, sizeof sll) < 0) { perror("AF_PACKET"); return 1; }
    int want = argc > 2 ? atoi(argv[2]) : 5;
    uint8_t b[65536];
    for (int i = 0; i < want; i++) {
        ssize_t n = recv(fd, b, sizeof b, 0);
        if (n > 0) decode(b, n);
    }
}
