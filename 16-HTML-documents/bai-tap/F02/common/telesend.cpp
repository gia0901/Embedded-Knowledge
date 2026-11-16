// telesend: giả lập nhiều thiết bị gửi telemetry tới telemd qua UDP (dùng chung cho mọi bài F02).
// Build: g++ -O2 -std=c++20 -o telesend telesend.cpp
// Chạy:  ./telesend <số bản tin> [số device=4] [port=9000] [nghỉ giữa 2 bản tin, µs=0]
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "dùng: %s N [devices] [port] [gap_us]\n", argv[0]); return 2; }
    long n = std::atol(argv[1]);
    int devs = argc > 2 ? std::atoi(argv[2]) : 4;
    int port = argc > 3 ? std::atoi(argv[3]) : 9000;
    int gap = argc > 4 ? std::atoi(argv[4]) : 0;
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    sockaddr_in to{};
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    char msg[64];
    for (long i = 0; i < n; ++i) {
        int len = std::snprintf(msg, sizeof msg, "dev=%ld temp=%.1f seq=%ld", i % devs, 20.0 + i % 50 / 10.0, i);
        // UDP không có flow control: gửi quá nhanh thì receiver có thể rơi packet (Bài 8)
        if (sendto(fd, msg, len, 0, reinterpret_cast<sockaddr*>(&to), sizeof to) < 0) { std::perror("sendto"); return 1; }
        if (gap) usleep(gap);
    }
    close(fd);
}
