// udpsend: gửi frame cảm biến 16 byte qua UDP, mỗi datagram chứa `batch` frame.
// Build: g++ -O2 -std=c++20 -o udpsend udpsend.cpp
// Chạy:  ./udpsend 10.0.2.2 5000 <số frame> [batch=1]
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Frame cảm biến 16 byte, các trường big-endian (network byte order):
// magic 0x7E1D | dev_id (2) | seq (4) | type (1) | flags (1) | temp x100 (2) | value (4)
static void make_frame(uint8_t* p, uint16_t dev, uint32_t seq) {
    uint16_t magic = htons(0x7E1D), d = htons(dev), t = htons((uint16_t)(2500 + seq % 300));
    uint32_t s = htonl(seq), v = htonl(seq * 7);
    memcpy(p, &magic, 2); memcpy(p + 2, &d, 2); memcpy(p + 4, &s, 4);
    p[8] = 1; p[9] = 0; memcpy(p + 10, &t, 2); memcpy(p + 12, &v, 4);
}

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s ip port so_frame [batch]\n", argv[0]); return 1; }
    int n = atoi(argv[3]), batch = argc > 4 ? atoi(argv[4]) : 1;
    sockaddr_in dst{}; dst.sin_family = AF_INET; dst.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &dst.sin_addr);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    std::vector<uint8_t> buf(16 * batch);
    long dgrams = 0, bytes = 0;
    for (int i = 0; i < n; i += batch) {
        int k = std::min(batch, n - i);
        for (int j = 0; j < k; j++) make_frame(&buf[16 * j], 7, i + j);
        if (sendto(fd, buf.data(), 16 * k, 0, (sockaddr*)&dst, sizeof dst) < 0) perror("sendto");
        dgrams++; bytes += 16 * k;
    }
    printf("gui %d frame trong %ld datagram, %ld byte du lieu\n", n, dgrams, bytes);
}
