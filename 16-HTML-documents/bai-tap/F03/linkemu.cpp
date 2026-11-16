// linkemu: dây mạng giả lập giữa hai TUN (emu0 trong netns gw và emu0 trong netns srv).
// Thay cho `tc netem` khi kernel không có netem. Mỗi hướng: hàng đợi FIFO giới hạn,
// tốc độ đường truyền (rate), độ trễ lan truyền (delay), tỉ lệ mất gói ngẫu nhiên (loss).
// Build: g++ -O2 -std=c++20 -o linkemu linkemu.cpp
// Chạy:  ./linkemu --delay 20 --loss 1 --rate 10000 --queue 100     (ms, %, kbit/s, gói)
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <random>
#include <string>
#include <vector>

using Clock = std::chrono::steady_clock;
static volatile sig_atomic_t stop = 0;

static int open_tun_in(const char* ns, const char* dev) {
    std::string path = std::string("/var/run/netns/") + ns;
    int nsfd = open(path.c_str(), O_RDONLY);
    if (nsfd < 0 || setns(nsfd, CLONE_NEWNET) < 0) { perror(path.c_str()); exit(1); }
    close(nsfd);
    int fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
    ifreq ifr{};
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;               // gói IP thuần, không có header phụ
    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);
    if (fd < 0 || ioctl(fd, TUNSETIFF, &ifr) < 0) { perror("TUNSETIFF"); exit(1); }
    return fd;
}

struct Pkt { Clock::time_point due; std::vector<char> data; };
struct Dir {                                           // một hướng truyền
    int in, out; const char* name;
    std::deque<Pkt> q; size_t qbytes = 0;
    Clock::time_point line_free = Clock::now();        // lúc đường truyền rảnh
    long rx = 0, lost = 0, tail = 0, tx = 0;
};

int main(int argc, char** argv) {
    double delay_ms = 0, loss = 0, rate_kbit = 0; size_t qlim = 1000;
    for (int i = 1; i + 1 < argc; i += 2) {
        std::string k = argv[i]; double v = atof(argv[i + 1]);
        if (k == "--delay") delay_ms = v; else if (k == "--loss") loss = v;
        else if (k == "--rate") rate_kbit = v; else if (k == "--queue") qlim = (size_t)v;
    }
    int a = open_tun_in("gw", "emu0"), b = open_tun_in("srv", "emu0");
    signal(SIGINT, [](int) { stop = 1; }); signal(SIGTERM, [](int) { stop = 1; });
    Dir d[2] = {{a, b, "gw->srv"}, {b, a, "srv->gw"}};
    std::mt19937 rng(42); std::uniform_real_distribution<double> U(0, 100);
    auto delay = std::chrono::microseconds((long)(delay_ms * 1000));
    printf("linkemu: delay %.1f ms, loss %.2f%%, rate %s, queue %zu goi\n", delay_ms, loss,
           rate_kbit > 0 ? (std::to_string((long)rate_kbit) + " kbit/s").c_str() : "khong gioi han", qlim);
    fflush(stdout);
    char buf[65536];
    while (!stop) {
        auto now = Clock::now();
        int timeout = 100;
        for (auto& x : d) {                            // giao các gói đã tới hạn
            while (!x.q.empty() && x.q.front().due <= now) {
                if (write(x.out, x.q.front().data.data(), x.q.front().data.size()) < 0) x.tail++;
                x.qbytes -= x.q.front().data.size(); x.q.pop_front(); x.tx++;
            }
            if (!x.q.empty()) {
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(x.q.front().due - now).count();
                timeout = std::min<int>(timeout, (int)std::max<long>(ms, 0));
            }
        }
        pollfd p[2] = {{a, POLLIN, 0}, {b, POLLIN, 0}};
        if (poll(p, 2, timeout) < 0) continue;
        for (int i = 0; i < 2; i++) {
            if (!(p[i].revents & POLLIN)) continue;
            Dir& x = d[i];
            ssize_t n;
            while ((n = read(x.in, buf, sizeof buf)) > 0) {
                x.rx++;
                if (U(rng) < loss) { x.lost++; continue; }          // mất ngẫu nhiên
                if (x.q.size() >= qlim) { x.tail++; continue; }     // hàng đợi đầy: drop-tail
                auto t = Clock::now();
                auto start = std::max(t, x.line_free);
                auto ser = rate_kbit > 0 ? std::chrono::microseconds((long)(n * 8 * 1000.0 / rate_kbit))
                                         : std::chrono::microseconds(0);
                x.line_free = start + ser;                           // transmission delay
                x.q.push_back({x.line_free + delay, std::vector<char>(buf, buf + n)});
                x.qbytes += n;
            }
        }
    }
    for (auto& x : d)
        printf("%s: nhan %ld, mat ngau nhien %ld, tran hang doi %ld, da giao %ld\n", x.name, x.rx, x.lost, x.tail, x.tx);
}
