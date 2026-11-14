// myping.c — ICMP echo bằng raw socket (cần sudo hoặc CAP_NET_RAW)
#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static uint16_t checksum(const void *data, size_t len) {       // Internet checksum, RFC 1071
    const uint16_t *p = data;
    uint32_t sum = 0;
    for (; len > 1; len -= 2) sum += *p++;
    if (len) sum += *(const uint8_t *)p;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);       // gấp phần tràn về 16 bit
    return (uint16_t)~sum;
}

static double now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s host [count]\n", argv[0]); return 1; }
    int count = argc > 2 ? atoi(argv[2]) : 4;

    struct addrinfo hints = { .ai_family = AF_INET }, *res;
    if (getaddrinfo(argv[1], NULL, &hints, &res) != 0) { fprintf(stderr, "cannot resolve %s\n", argv[1]); return 1; }
    struct sockaddr_in dst = *(struct sockaddr_in *)res->ai_addr;
    freeaddrinfo(res);

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (s < 0) { perror("socket (cần sudo?)"); return 1; }
    struct timeval tv = { .tv_sec = 1 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);

    uint16_t id = (uint16_t)(getpid() & 0xFFFF);
    for (int seq = 1; seq <= count; seq++) {
        unsigned char pkt[64] = {0};
        struct icmphdr *icmp = (struct icmphdr *)pkt;
        icmp->type = ICMP_ECHO;                                  // type 8, code 0
        icmp->un.echo.id = htons(id);
        icmp->un.echo.sequence = htons((uint16_t)seq);
        memcpy(pkt + sizeof *icmp, "core-net-lab", 12);          // payload tuỳ ý
        icmp->checksum = checksum(pkt, sizeof pkt);              // tính khi field checksum = 0

        double t0 = now_ms();
        if (sendto(s, pkt, sizeof pkt, 0, (struct sockaddr *)&dst, sizeof dst) < 0) { perror("sendto"); return 1; }
        for (;;) {
            unsigned char buf[1500];
            ssize_t n = recv(s, buf, sizeof buf, 0);              // raw socket: nhận cả IP header
            if (n < 0) { printf("seq=%d timeout\n", seq); break; }
            struct iphdr *ip = (struct iphdr *)buf;
            size_t ihl = ip->ihl * 4u;                            // IHL tính bằng word 4 byte
            struct icmphdr *r = (struct icmphdr *)(buf + ihl);
            if (r->type == ICMP_ECHOREPLY && ntohs(r->un.echo.id) == id &&
                ntohs(r->un.echo.sequence) == seq) {
                char from[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip->saddr, from, sizeof from);
                printf("%zd bytes from %s: seq=%d ttl=%d time=%.2f ms\n",
                       n - (ssize_t)ihl, from, seq, ip->ttl, now_ms() - t0);
                break;
            }
            // ICMP khác (của ping khác, hoặc chính request khi ping localhost): bỏ qua, đọc tiếp
        }
        sleep(1);
    }
    close(s);
    return 0;
}
