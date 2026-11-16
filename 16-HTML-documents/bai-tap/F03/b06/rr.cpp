// rr: đo độ trễ request/response nhỏ qua TCP, để thấy Nagle gặp delayed ACK.
// Build: g++ -O2 -std=c++20 -Wall -o rr rr.cpp
// Server: ./rr srv PORT                 đọc đủ 16 byte thì trả 1 byte
// Client: ./rr cli IP PORT N [--split] [--nodelay]
//   --split    gửi mỗi request bằng hai write(): header 4 byte rồi body 12 byte (kiểu code hay gặp)
//   --nodelay  đặt TCP_NODELAY (tắt Nagle)
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;

static bool read_full(int fd, char* p, size_t n) {
    while (n) { ssize_t r = read(fd, p, n); if (r <= 0) return false; p += r; n -= r; }
    return true;
}

int main(int argc, char** argv) {
    if (argc >= 3 && !strcmp(argv[1], "srv")) {
        int ls = socket(AF_INET, SOCK_STREAM, 0), one = 1;
        setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(atoi(argv[2]));
        if (bind(ls, (sockaddr*)&a, sizeof a) < 0) { perror("bind"); return 1; }
        listen(ls, 8);
        for (;;) {
            int c = accept(ls, nullptr, nullptr);
            char req[16];
            while (read_full(c, req, 16)) if (write(c, "k", 1) != 1) break;
            close(c);
        }
    }
    if (argc < 5 || strcmp(argv[1], "cli")) { fprintf(stderr, "dung: rr srv PORT | rr cli IP PORT N [--split] [--nodelay]\n"); return 1; }
    int n = atoi(argv[4]); bool split = false, nodelay = false;
    for (int i = 5; i < argc; i++) { if (!strcmp(argv[i], "--split")) split = true; if (!strcmp(argv[i], "--nodelay")) nodelay = true; }
    int fd = socket(AF_INET, SOCK_STREAM, 0), one = 1;
    if (nodelay) setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[2], &a.sin_addr);
    if (connect(fd, (sockaddr*)&a, sizeof a) < 0) { perror("connect"); return 1; }
    char req[16] = "HDR:body-12byte", resp;
    std::vector<double> us;
    for (int i = 0; i < n; i++) {
        auto t0 = Clock::now();
        if (split) {
            if (write(fd, req, 4) != 4 || write(fd, req + 4, 12) != 12) { perror("write"); return 1; }
        } else if (write(fd, req, 16) != 16) { perror("write"); return 1; }
        if (!read_full(fd, &resp, 1)) { fprintf(stderr, "server dong\n"); return 1; }
        us.push_back(std::chrono::duration<double, std::micro>(Clock::now() - t0).count());
    }
    std::sort(us.begin(), us.end());
    double sum = 0; for (double v : us) sum += v;
    printf("%d request, %s, %s: trung binh %.1f us, trung vi %.1f us, lon nhat %.1f us\n", n,
           split ? "2 write/request" : "1 write/request", nodelay ? "TCP_NODELAY" : "Nagle bat",
           sum / n, us[n / 2], us.back());
}
