// telemd v1.1: như v1 (Bài 0) và thêm dừng sạch khi nhận SIGTERM/SIGINT. Lời giải Lab 1.4.
// Handler chỉ đặt cờ (việc duy nhất an toàn trong handler); vòng chính thấy recv trả EINTR thì kiểm tra cờ.
// Build: g++ -O2 -std=c++20 -o telemd telemd.cpp
// Chạy:  ./telemd [port=9000]
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t g_stop = 0;
static void on_stop(int) { g_stop = 1; }       // KHÔNG printf, malloc, lock ở đây: không async-signal-safe

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 9000;

    struct sigaction sa{};
    sa.sa_handler = on_stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                             // không SA_RESTART: muốn recv trả EINTR để vòng lặp thấy cờ
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
    signal(SIGPIPE, SIG_IGN);                    // daemon mạng: ghi vào kết nối đã đóng không được giết process

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) { std::perror("bind"); return 1; }
    std::printf("telemd v1.1 pid=%d nghe 127.0.0.1:%d\n", getpid(), port);
    std::fflush(stdout);

    std::map<int, long> per_dev;
    long total = 0;
    char buf[512];
    while (!g_stop) {
        ssize_t n = recv(fd, buf, sizeof buf - 1, 0);
        if (n < 0) {
            if (errno == EINTR) continue;        // quay lại while: nếu g_stop đã bật thì thoát
            std::perror("recv");
            return 1;
        }
        buf[n] = '\0';
        int dev; double temp;
        if (std::sscanf(buf, "dev=%d temp=%lf", &dev, &temp) == 2) { ++per_dev[dev]; ++total; }
    }
    // dọn dẹp ngoài handler: ở đây gọi gì cũng được
    for (auto [dev, cnt] : per_dev) std::printf("dev %d: %ld\n", dev, cnt);
    std::printf("dung sach: %ld ban tin\n", total);
    close(fd);
    return 0;
}
