// telemd v1: nhận telemetry qua UDP, đếm bản tin theo device. Một thread, recv() chặn.
// Đây là ví dụ chạy xuyên suốt F02: các bài sau thêm thread (Bài 2), lock (Bài 4–5),
// ghi snapshot bền (Bài 7), event loop epoll (Bài 8) và chạy trong container (Bài 9).
// Build: g++ -O2 -std=c++20 -o telemd telemd.cpp
// Chạy:  ./telemd [port=9000] [dừng sau N bản tin, mặc định chạy mãi]
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 9000;
    long max = argc > 2 ? std::atol(argv[2]) : -1;
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0) { std::perror("socket"); return 1; }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // chỉ nghe loopback: lab không mở port ra mạng
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) { std::perror("bind"); return 1; }
    std::printf("telemd v1 pid=%d nghe 127.0.0.1:%d\n", getpid(), port);
    std::fflush(stdout);

    std::map<int, long> per_dev;
    long total = 0, bad = 0;
    char buf[512];
    while (max < 0 || total + bad < max) {
        ssize_t n = recv(fd, buf, sizeof buf - 1, 0);          // 1 bản tin = 1 system call
        if (n < 0) {
            if (errno == EINTR) continue;                        // bị signal ngắt: thử lại, không phải lỗi
            std::perror("recv");
            break;
        }
        buf[n] = '\0';
        int dev; double temp;
        if (std::sscanf(buf, "dev=%d temp=%lf", &dev, &temp) != 2) { ++bad; continue; }
        ++per_dev[dev];
        ++total;
    }
    for (auto [dev, cnt] : per_dev) std::printf("dev %d: %ld\n", dev, cnt);
    std::printf("tong %ld ban tin hop le, %ld ban tin loi\n", total, bad);
    close(fd);
}
