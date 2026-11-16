// echosrv: server echo TCP một thread, epoll, socket non-blocking, khung theo wire.h.
// Build: g++ -O2 -std=c++20 -Wall -o echosrv echosrv.cpp
// Chạy:  ./echosrv PORT [tuỳ chọn]
//   --naive        coi mỗi read() là một tin (sai); không trả lời, chỉ đếm
//   --et           epoll edge-triggered (mặc định level-triggered)
//   --once         mỗi sự kiện chỉ read() một lần 4096 byte (đúng với LT, sai với ET)
//   --outmax B     ngừng đọc kết nối khi bộ đệm gửi của nó vượt B byte (0 = không giới hạn, mặc định)
// Khi một kết nối đóng, in số byte, số read(), số tin và số lần epoll_wait từ đầu.
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include "wire.h"

struct Conn {
    Decoder dec;
    std::string out;                 // byte đã sinh ra nhưng kernel chưa nhận (bộ đệm gửi phía ứng dụng)
    size_t out_off = 0;
    long bytes = 0, reads = 0, msgs = 0, badframe = 0;
    bool reading = true, want_out = false;
};

static bool naive = false, et = false, once = false;
static size_t outmax = 0;
static int ep;
static long waits = 0, accept_err = 0;
static std::unordered_map<int, Conn> conns;

static long rss_kb() {
    rusage ru; getrusage(RUSAGE_SELF, &ru); return ru.ru_maxrss;
}

static void update(int fd, Conn& c) {           // đăng ký lại sự kiện theo trạng thái kết nối
    epoll_event ev{};
    ev.events = (c.reading ? EPOLLIN : 0) | (c.want_out ? EPOLLOUT : 0) | (et ? EPOLLET : 0) | (c.reading ? EPOLLRDHUP : 0);
    ev.data.fd = fd;
    epoll_ctl(ep, EPOLL_CTL_MOD, fd, &ev);
}

static void close_conn(int fd, Conn& c, const char* why) {
    printf("ket noi %d dong (%s): %ld byte, %ld read(), %ld tin, sai khung %ld | epoll_wait %ld lan, RSS max %ld MB\n",
           fd, why, c.bytes, c.reads, c.msgs, c.badframe, waits, rss_kb() / 1024);
    fflush(stdout);
    epoll_ctl(ep, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    conns.erase(fd);
}

// Ghi từ bộ đệm gửi cho tới khi hết hoặc EAGAIN. false nếu kết nối hỏng.
static bool flush_out(int fd, Conn& c) {
    while (c.out_off < c.out.size()) {
        ssize_t w = send(fd, c.out.data() + c.out_off, c.out.size() - c.out_off, MSG_NOSIGNAL);
        if (w > 0) { c.out_off += w; continue; }
        if (w < 0 && errno == EINTR) continue;
        if (w < 0 && errno == EAGAIN) break;
        return false;
    }
    if (c.out_off == c.out.size()) { c.out.clear(); c.out_off = 0; }
    else if (c.out_off > (1 << 20)) { c.out.erase(0, c.out_off); c.out_off = 0; }
    size_t left = c.out.size() - c.out_off;
    bool want = left > 0;
    bool rd = outmax == 0 || left < outmax / 2 || (c.reading && left <= outmax);
    if (want != c.want_out || rd != c.reading) { c.want_out = want; c.reading = rd; update(fd, c); }
    return true;
}

// Đọc. false nếu kết nối phải đóng.
static bool on_read(int fd, Conn& c) {
    char buf[4096];
    for (;;) {
        if (!c.reading) return true;
        ssize_t r = read(fd, buf, sizeof buf);
        if (r == 0) return false;                          // EOF
        if (r < 0) {
            if (errno == EINTR) continue;
            return errno == EAGAIN;                        // EAGAIN: hết dữ liệu, chờ epoll
        }
        c.bytes += r; c.reads++;
        if (naive) {                                       // sai: coi cả lần đọc là một tin
            c.msgs++;
            uint32_t n = r >= 4 ? uint32_t((unsigned char)buf[0]) << 24 | (unsigned char)buf[1] << 16 |
                                  (unsigned char)buf[2] << 8 | (unsigned char)buf[3] : 0;
            if (r < 4 || n + 4 != uint32_t(r)) c.badframe++;
        } else {
            c.dec.feed(buf, r);
            std::string m; int k;
            while ((k = c.dec.next(m)) == 1) { c.msgs++; put_msg(c.out, m.data(), m.size()); }
            if (k < 0) { c.badframe++; return false; }
            if (!flush_out(fd, c)) return false;
        }
        if (once) return true;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s port [--naive --et --once --outmax B]\n", argv[0]); return 1; }
    for (int i = 2; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--naive") naive = true; else if (a == "--et") et = true; else if (a == "--once") once = true;
        else if (a == "--outmax" && i + 1 < argc) outmax = atol(argv[++i]);
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    signal(SIGPIPE, SIG_IGN);
    int ls = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0), one = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(atoi(argv[1]));
    if (bind(ls, (sockaddr*)&a, sizeof a) < 0 || listen(ls, 4096) < 0) { perror("bind/listen"); return 1; }
    ep = epoll_create1(0);
    epoll_event ev{}; ev.events = EPOLLIN; ev.data.fd = ls;
    epoll_ctl(ep, EPOLL_CTL_ADD, ls, &ev);
    rlimit rl; getrlimit(RLIMIT_NOFILE, &rl);
    printf("nghe cong %s, %s%s%s, outmax %zu, gioi han fd %ld\n", argv[1], et ? "ET" : "LT", once ? " once" : "",
           naive ? " naive" : "", outmax, (long)rl.rlim_cur);
    fflush(stdout);
    epoll_event evs[64];
    long last_report = 0;
    for (;;) {
        int n = epoll_wait(ep, evs, 64, -1);
        waits++;
        for (int i = 0; i < n; i++) {
            int fd = evs[i].data.fd;
            if (fd == ls) {
                for (;;) {
                    int c = accept4(ls, nullptr, nullptr, SOCK_NONBLOCK);
                    if (c < 0) {
                        if (errno != EAGAIN) {
                            accept_err++;
                            if (accept_err - last_report >= 100000 || accept_err == 1) {
                                printf("accept loi: %s (da %ld lan, epoll_wait %ld lan)\n", strerror(errno), accept_err, waits);
                                fflush(stdout); last_report = accept_err;
                            }
                        }
                        break;
                    }
                    conns[c];
                    epoll_event e{}; e.events = EPOLLIN | EPOLLRDHUP | (et ? EPOLLET : 0); e.data.fd = c;
                    epoll_ctl(ep, EPOLL_CTL_ADD, c, &e);
                }
                continue;
            }
            auto it = conns.find(fd);
            if (it == conns.end()) continue;
            Conn& c = it->second;
            if (evs[i].events & EPOLLOUT) { if (!flush_out(fd, c)) { close_conn(fd, c, "loi ghi"); continue; } }
            if (evs[i].events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                if (!on_read(fd, c)) close_conn(fd, c, c.badframe && !naive ? "khung hong" : "EOF");
            }
        }
    }
}
