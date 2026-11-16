// ethsniff: đọc frame Ethernet thô từ một interface bằng AF_PACKET và giải mã header lớp 2.
// Build: g++ -O2 -std=c++20 -o ethsniff ethsniff.cpp
// Chạy (root): ./ethsniff veth-dev 6
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void mac(char* out, const uint8_t* m) {
    snprintf(out, 18, "%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4], m[5]);
}
static uint16_t be16(const uint8_t* p) { return (uint16_t)(p[0] << 8 | p[1]); }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s <interface> [so frame]\n", argv[0]); return 1; }
    int want = argc > 2 ? atoi(argv[2]) : 10;
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));     // mọi ethertype
    if (fd < 0) { perror("socket(AF_PACKET) can quyen root/CAP_NET_RAW"); return 1; }
    sockaddr_ll sll{}; sll.sll_family = AF_PACKET; sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = if_nametoindex(argv[1]);
    if (!sll.sll_ifindex || bind(fd, (sockaddr*)&sll, sizeof sll) < 0) { perror(argv[1]); return 1; }
    uint8_t f[2048];
    for (int n = 0; n < want;) {
        sockaddr_ll from{}; socklen_t fl = sizeof from;
        ssize_t len = recvfrom(fd, f, sizeof f, 0, (sockaddr*)&from, &fl);
        if (len < 14) continue;
        char d[18], s[18]; mac(d, f); mac(s, f + 6);
        uint16_t type = be16(f + 12); int off = 14;
        const char* dir = from.sll_pkttype == PACKET_OUTGOING ? "ra " : "vao";
        printf("%s %3zd B  %s -> %s  type 0x%04x", dir, len, s, d, type);
        if (type == 0x8100) {                                   // thẻ VLAN 802.1Q: 4 byte chèn giữa
            printf(" vlan %u", be16(f + 14) & 0x0fff);
            type = be16(f + 16); off = 18;
            printf(" inner 0x%04x", type);
        }
        if (type == 0x0806 && len >= off + 28) {               // ARP cho IPv4 trên Ethernet
            const uint8_t* a = f + off; char sm[18];
            mac(sm, a + 8);
            printf("  ARP %s %u.%u.%u.%u (%s) hoi/bao %u.%u.%u.%u", be16(a + 6) == 1 ? "request" : "reply  ",
                   a[14], a[15], a[16], a[17], sm, a[24], a[25], a[26], a[27]);
        } else if (type == 0x0800 && len >= off + 20) {        // IPv4: chỉ in vài trường, Bài 2 đi sâu
            const uint8_t* ip = f + off;
            printf("  IPv4 proto %u %u.%u.%u.%u -> %u.%u.%u.%u", ip[9], ip[12], ip[13], ip[14], ip[15],
                   ip[16], ip[17], ip[18], ip[19]);
        } else if (type == 0x86dd) {
            printf("  IPv6");
        }
        if (d[0] == 'f' && !strcmp(d, "ff:ff:ff:ff:ff:ff")) printf("  [broadcast]");
        else if (f[0] & 1) printf("  [multicast]");
        printf("\n");
        n++;
    }
}
