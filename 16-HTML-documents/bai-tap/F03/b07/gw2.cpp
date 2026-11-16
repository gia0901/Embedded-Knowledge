// gw2: gateway v2. Nhận frame cảm biến qua UDP (Bài 4), gom thành lô, đẩy lên backend qua TCP.
// Một thread, epoll, socket non-blocking. Hàng đợi gửi có giới hạn; backend mất thì tự nối lại.
// Build: g++ -O2 -std=c++20 -Wall -o gw2 gw2.cpp
// Chạy:  ./gw2 UDP_PORT BACKEND_IP BACKEND_PORT [tuỳ chọn]
//   --batch N     tối đa N frame mỗi lô (mặc định 64)
//   --flush-ms T  lô chưa đầy vẫn gửi sau T ms (mặc định 20)
//   --qmax B      giới hạn hàng đợi gửi B byte; đầy thì bỏ lô cũ nhất (mặc định 1000000; 0 = không giới hạn)
//   --sndbuf B    đặt SO_SNDBUF của socket TCP (mặc định: autotuning, tới tcp_wmem max)
// Mỗi giây in một dòng thống kê. Ctrl-C in tổng kết.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <string>
#include "../b04/frame.h"
#include "wire.h"

static double now_ms() { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }
static volatile sig_atomic_t stop = 0;

enum State { DOWN, CONNECTING, UP };
static const char* sname[] = {"DOWN", "CONNECTING", "UP"};

struct Stats { long rx = 0, bad = 0, queued = 0, sent = 0, dropped = 0, reconnects = 0, eagain = 0; };

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s udp_port backend_ip backend_port [--batch N --flush-ms T --qmax B --sndbuf B]\n", argv[0]); return 1; }
    size_t batch = 64, qmax = 1000000; double flush_ms = 20; int sndbuf = 0;
    for (int i = 4; i < argc; i++) {
        std::string a = argv[i];
        auto val = [&] { return i + 1 < argc ? atol(argv[++i]) : 0L; };
        if (a == "--batch") batch = val(); else if (a == "--flush-ms") flush_ms = val(); else if (a == "--qmax") qmax = val();
        else if (a == "--sndbuf") sndbuf = val();
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, [](int) { stop = 1; });
    sockaddr_in be{}; be.sin_family = AF_INET; be.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[2], &be.sin_addr);

    int us = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    sockaddr_in ua{}; ua.sin_family = AF_INET; ua.sin_port = htons(atoi(argv[1]));
    if (bind(us, (sockaddr*)&ua, sizeof ua) < 0) { perror("bind udp"); return 1; }
    int rb = 4 << 20; setsockopt(us, SOL_SOCKET, SO_RCVBUF, &rb, sizeof rb);

    int ep = epoll_create1(0);
    epoll_event ev{}; ev.events = EPOLLIN; ev.data.fd = us; epoll_ctl(ep, EPOLL_CTL_ADD, us, &ev);

    State st = DOWN; int ts = -1; double retry_at = 0, backoff = 100;
    std::deque<std::string> q;        // các lô đã đóng khung, chờ gửi
    size_t qbytes = 0, head_off = 0;  // head_off: số byte của q.front() đã gửi
    size_t qpeak = 0;
    std::string cur; size_t cur_n = 0; double cur_t = 0;   // lô đang gom
    Stats s, last; double t0 = now_ms(), tick = t0 + 1000;

    auto set_out = [&](bool want) {
        epoll_event e{}; e.events = EPOLLIN | EPOLLRDHUP | (want ? EPOLLOUT : 0); e.data.fd = ts;
        epoll_ctl(ep, EPOLL_CTL_MOD, ts, &e);
    };
    auto go_down = [&](const char* why) {
        printf("backend mat (%s), %zu byte con trong hang doi, noi lai sau %.0f ms\n", why, qbytes, backoff);
        fflush(stdout);
        epoll_ctl(ep, EPOLL_CTL_DEL, ts, nullptr); close(ts); ts = -1; st = DOWN;
        if (head_off) { qbytes += head_off; head_off = 0; }   // lô gửi dở: gửi lại từ đầu trên kết nối mới
        retry_at = now_ms() + backoff; backoff = std::min(backoff * 2, 2000.0);
    };
    auto try_connect = [&] {
        ts = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sndbuf) setsockopt(ts, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof sndbuf);
        int r = connect(ts, (sockaddr*)&be, sizeof be);
        st = (r == 0) ? UP : CONNECTING;
        if (r < 0 && errno != EINPROGRESS) { s.reconnects++; go_down(strerror(errno)); return; }
        epoll_event e{}; e.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT; e.data.fd = ts;
        epoll_ctl(ep, EPOLL_CTL_ADD, ts, &e);
    };
    auto flush_q = [&]() -> bool {    // ghi từ hàng đợi tới khi hết hoặc EAGAIN
        while (!q.empty()) {
            std::string& b = q.front();
            ssize_t w = send(ts, b.data() + head_off, b.size() - head_off, MSG_NOSIGNAL);
            if (w < 0) {
                if (errno == EINTR) continue;
                if (errno == EAGAIN) { s.eagain++; set_out(true); return true; }
                go_down(strerror(errno)); return false;
            }
            head_off += w; qbytes -= w;
            if (head_off == b.size()) { s.sent += (b.size() - 4) / kFrame; q.pop_front(); head_off = 0; }
        }
        set_out(false); return true;
    };
    auto seal = [&] {                 // đóng lô đang gom và đưa vào hàng đợi
        if (!cur_n) return;
        std::string m; put_msg(m, cur.data(), cur.size());
        q.push_back(std::move(m)); qbytes += q.back().size(); s.queued += cur_n;
        while (qmax && qbytes > qmax && q.size() > 1) {   // đầy: bỏ lô cũ nhất (trừ lô đang gửi dở)
            auto it = head_off ? q.begin() + 1 : q.begin();
            s.dropped += (it->size() - 4) / kFrame; qbytes -= it->size(); q.erase(it);
        }
        qpeak = std::max(qpeak, qbytes);
        cur.clear(); cur_n = 0;
        if (st == UP) flush_q();
    };

    printf("gw2: UDP %s -> TCP %s:%s, lo %zu frame / %.0f ms, qmax %zu, sndbuf %s\n", argv[1], argv[2], argv[3], batch, flush_ms, qmax,
           sndbuf ? std::to_string(sndbuf).c_str() : "auto");
    fflush(stdout);
    epoll_event evs[16]; uint8_t buf[2048];
    while (!stop) {
        double t = now_ms();
        if (st == DOWN && t >= retry_at) try_connect();
        int timeout = 50;
        if (cur_n) timeout = std::max(0, int(cur_t + flush_ms - t));
        int n = epoll_wait(ep, evs, 16, timeout);
        for (int i = 0; i < n; i++) {
            if (evs[i].data.fd == us) {
                for (;;) {
                    ssize_t r = recv(us, buf, sizeof buf, 0);
                    if (r < 0) break;
                    for (ssize_t o = 0; o + kFrame <= r; o += kFrame) {
                        Frame f;
                        if (!get_frame(buf + o, f)) { s.bad++; continue; }
                        if (!cur_n) cur_t = now_ms();
                        cur.append((char*)buf + o, kFrame); cur_n++; s.rx++;
                        if (cur_n >= batch) seal();
                    }
                }
            } else if (evs[i].data.fd == ts) {
                uint32_t e = evs[i].events;
                if (st == CONNECTING && (e & (EPOLLOUT | EPOLLERR | EPOLLHUP))) {
                    int err = 0; socklen_t l = sizeof err; getsockopt(ts, SOL_SOCKET, SO_ERROR, &err, &l);
                    if (err) { s.reconnects++; go_down(strerror(err)); continue; }
                    st = UP; backoff = 100;
                    printf("da noi backend, %zu byte cho gui\n", qbytes); fflush(stdout);
                    flush_q(); continue;
                }
                if (e & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) { s.reconnects++; go_down("backend dong"); continue; }
                if (e & EPOLLOUT) flush_q();
            }
        }
        t = now_ms();
        if (cur_n && t >= cur_t + flush_ms) seal();
        if (t >= tick) {
            printf("t=%3.0f rx %6ld/s gui %6ld/s bo %6ld/s | hang doi %4zu KB | %s\n", (t - t0) / 1000,
                   s.rx - last.rx, s.sent - last.sent, s.dropped - last.dropped, qbytes / 1024, sname[st]);
            fflush(stdout); last = s; tick += 1000;
        }
    }
    seal(); if (st == UP) flush_q();
    rusage ru; getrusage(RUSAGE_SELF, &ru);
    printf("tong: nhan %ld frame, sai %ld | da ghi vao socket %ld, bo do day %ld, con trong hang doi %zu byte | noi lai %ld, EAGAIN %ld | hang doi dinh %zu KB, RSS max %ld MB\n",
           s.rx, s.bad, s.sent, s.dropped, qbytes, s.reconnects, s.eagain, qpeak / 1024, ru.ru_maxrss / 1024);
}
