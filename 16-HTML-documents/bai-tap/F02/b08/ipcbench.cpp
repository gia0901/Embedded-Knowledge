// ipcbench: thời gian một vòng ping-pong 64 byte giữa hai process (cha và con sau fork).
// Lời giải Lab 8.3. Build: g++ -O2 -std=c++20 -o ipcbench ipcbench.cpp
// Chạy:  ./ipcbench pipe|unix|unixdgram|tcp|shm_eventfd|shm_spin [số vòng=100000]
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

constexpr int MSG = 64;
struct Chan { int tx, rx; };                            // một đầu của kênh: ghi vào tx, đọc từ rx

static void xfer_send(int fd, const char* b) { if (write(fd, b, MSG) != MSG) _exit(3); }
static void xfer_recv(int fd, char* b) {
    for (int got = 0; got < MSG;) {                     // stream có thể trả ít hơn MSG byte
        ssize_t r = read(fd, b + got, MSG - got);
        if (r <= 0) _exit(4);
        got += r;
    }
}

struct Shm {                                            // vùng nhớ chung giữa hai process
    std::atomic<int> turn;                              // 0: cha ghi, 1: con ghi
    char buf[MSG];
};

int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "pipe";
    long n = argc > 2 ? std::atol(argv[2]) : 100000;
    Chan parent{}, child{};
    Shm* shm = nullptr;
    int efd_to_child = -1, efd_to_parent = -1;
    int lst = -1;

    if (mode == "pipe") {
        int a[2], b[2];
        if (pipe(a) || pipe(b)) return 1;
        parent = {a[1], b[0]}; child = {b[1], a[0]};
    } else if (mode == "unix" || mode == "unixdgram") {
        int sv[2];
        socketpair(AF_UNIX, mode == "unix" ? SOCK_STREAM : SOCK_DGRAM, 0, sv);
        parent = {sv[0], sv[0]}; child = {sv[1], sv[1]};
    } else if (mode == "tcp") {
        lst = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in a{}; a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        socklen_t len = sizeof a;
        bind(lst, (sockaddr*)&a, sizeof a); listen(lst, 1); getsockname(lst, (sockaddr*)&a, &len);
        int c = socket(AF_INET, SOCK_STREAM, 0);
        connect(c, (sockaddr*)&a, sizeof a);
        int s = accept(lst, nullptr, nullptr);
        int one = 1;
        setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);   // tắt Nagle: gửi ngay gói nhỏ
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
        parent = {c, c}; child = {s, s};
    } else if (mode == "shm_eventfd" || mode == "shm_spin") {
        shm = static_cast<Shm*>(mmap(nullptr, sizeof(Shm), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
        new (shm) Shm{};
        efd_to_child = eventfd(0, 0);                   // bộ đếm 64 bit: write cộng, read lấy và về 0
        efd_to_parent = eventfd(0, 0);
    } else { std::fprintf(stderr, "che do khong ro: %s\n", mode.c_str()); return 2; }

    bool spin = mode == "shm_spin";
    pid_t pid = fork();
    if (pid == 0) {                                     // con: nhận rồi gửi lại
        char b[MSG];
        for (long i = 0; i < n; ++i) {
            if (shm) {
                if (spin) while (shm->turn.load(std::memory_order_acquire) != 1) {}
                else { uint64_t v; if (read(efd_to_child, &v, 8) != 8) _exit(5); }
                std::memcpy(b, shm->buf, MSG); b[0]++;
                std::memcpy(shm->buf, b, MSG);
                if (spin) shm->turn.store(0, std::memory_order_release);
                else { uint64_t one = 1; if (write(efd_to_parent, &one, 8) != 8) _exit(6); }
            } else {
                xfer_recv(child.rx, b); b[0]++; xfer_send(child.tx, b);
            }
        }
        _exit(0);
    }
    char b[MSG] = {};
    auto t0 = std::chrono::steady_clock::now();
    for (long i = 0; i < n; ++i) {
        if (shm) {
            std::memcpy(shm->buf, b, MSG);
            if (spin) { shm->turn.store(1, std::memory_order_release); while (shm->turn.load(std::memory_order_acquire) != 0) {} }
            else { uint64_t one = 1, v; if (write(efd_to_child, &one, 8) != 8 || read(efd_to_parent, &v, 8) != 8) return 1; }
            std::memcpy(b, shm->buf, MSG);
        } else {
            xfer_send(parent.tx, b); xfer_recv(parent.rx, b);
        }
    }
    auto t1 = std::chrono::steady_clock::now();
    int st; waitpid(pid, &st, 0);
    double us = std::chrono::duration<double, std::micro>(t1 - t0).count() / n;
    std::printf("%-12s %7.2f us/vong  (%ld vong, kiem tra %s)\n", mode.c_str(), us, n,
                (unsigned char)b[0] == n % 256 ? "OK" : "SAI");
    return 0;
}
