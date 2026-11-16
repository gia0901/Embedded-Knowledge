// gw1: gateway phiên bản 1, nhận frame cảm biến qua UDP và thống kê theo thiết bị.
// Build: g++ -O2 -std=c++20 -Wall -o gw1 gw1.cpp
// Chạy:  ./gw1 [--port 5000] [--mmsg K] [--work-us U] [--rcvbuf B] [--idle MS] [--plain]
//   --mmsg K     dùng recvmmsg nhận tối đa K datagram mỗi syscall (mặc định: recvfrom, 1 datagram)
//   --work-us U  giả lập xử lý U micro giây cho mỗi frame (busy-wait)
//   --rcvbuf B   đặt SO_RCVBUF = B byte trước khi bind
//   --idle MS    in thống kê và thoát khi không có dữ liệu MS mili giây (mặc định 1000)
//   --plain      chỉ đếm theo seq tăng dần đơn giản (không cửa sổ): để so với bản đầy đủ
#include <poll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "frame.h"

using Clock = std::chrono::steady_clock;

// Đọc một bộ đếm UDP của kernel (trong network namespace hiện tại) từ /proc/net/snmp.
static long udp_counter(const std::string& name) {
    std::ifstream f("/proc/net/snmp"); std::string hdr, val;
    while (std::getline(f, hdr) && std::getline(f, val)) {
        if (hdr.rfind("Udp:", 0) != 0) continue;
        std::istringstream h(hdr), v(val); std::string k, x;
        while (h >> k && v >> x) if (k == name) return atol(x.c_str());
    }
    return -1;
}

// Theo dõi seq của một thiết bị bằng cửa sổ trượt 64 bit (như chống replay của IPsec, RTP).
// bit i của win = đã nhận seq (max - i).
struct Dev {
    bool init = false; uint32_t max = 0; uint64_t win = 0;
    long frames = 0, lost = 0, dup = 0, reorder = 0, late = 0, restart = 0;
    void see(uint32_t s) {
        frames++;
        if (!init) { init = true; max = s; win = 1; return; }
        if (s > max) {
            uint32_t d = s - max;
            lost += d - 1;                                   // tạm coi là mất; về muộn sẽ trừ lại
            win = d >= 64 ? 1 : (win << d) | 1;
            max = s;
            return;
        }
        uint32_t d = max - s;
        if (d >= 64) {
            if (s < 16) { restart++; max = s; win = 1; return; }      // seq nhỏ, xa cửa sổ: thiết bị đếm lại từ 0
            late++; return;                                  // quá cũ, ngoài cửa sổ
        }
        if (win >> d & 1) { dup++; return; }
        win |= uint64_t(1) << d; lost--; reorder++;
    }
};

struct PlainDev { bool init = false; uint32_t next = 0; long frames = 0, lost = 0;
    void see(uint32_t s) { frames++; if (init && s > next) lost += s - next; init = true; next = s + 1; } };

int main(int argc, char** argv) {
    int port = 5000, mmsg = 0, rcvbuf = 0, idle = 1000; long work_us = 0; bool plain = false;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        auto val = [&] { return i + 1 < argc ? atol(argv[++i]) : 0L; };
        if (a == "--port") port = val(); else if (a == "--mmsg") mmsg = val(); else if (a == "--work-us") work_us = val();
        else if (a == "--rcvbuf") rcvbuf = val(); else if (a == "--idle") idle = val(); else if (a == "--plain") plain = true;
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (rcvbuf && setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf) < 0) perror("SO_RCVBUF");
    int eff; socklen_t el = sizeof eff; getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &eff, &el);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (sockaddr*)&a, sizeof a) < 0) { perror("bind"); return 1; }
    printf("gw1: nghe UDP :%d, SO_RCVBUF thuc te %d byte, %s\n", port, eff,
           mmsg ? ("recvmmsg " + std::to_string(mmsg)).c_str() : "recvfrom");
    fflush(stdout);

    const int K = mmsg ? mmsg : 1, BUF = 2048;
    std::vector<uint8_t> store(K * BUF);
    std::vector<iovec> iov(K); std::vector<mmsghdr> msgs(K); std::vector<sockaddr_in> from(K);
    for (int i = 0; i < K; i++) {
        iov[i] = {&store[i * BUF], (size_t)BUF};
        msgs[i].msg_hdr = {}; msgs[i].msg_hdr.msg_iov = &iov[i]; msgs[i].msg_hdr.msg_iovlen = 1;
        msgs[i].msg_hdr.msg_name = &from[i]; msgs[i].msg_hdr.msg_namelen = sizeof from[i];
    }
    std::map<uint16_t, Dev> devs; std::map<uint16_t, PlainDev> pdevs;
    long dgrams = 0, calls = 0, polls = 0, bad = 0, trunc = 0;
    long drops0 = udp_counter("RcvbufErrors");
    Clock::time_point t_first{}, t_last{};

    for (;;) {
        int got;
        if (mmsg) {
            for (int i = 0; i < K; i++) msgs[i].msg_hdr.msg_namelen = sizeof from[i];
            got = recvmmsg(fd, msgs.data(), K, MSG_DONTWAIT, nullptr);
        } else {
            socklen_t fl = sizeof from[0];
            ssize_t n = recvfrom(fd, store.data(), BUF, MSG_DONTWAIT, (sockaddr*)&from[0], &fl);
            got = n < 0 ? -1 : 1;
            msgs[0].msg_len = n < 0 ? 0 : n;
        }
        calls++;
        if (got <= 0) {                                      // hết dữ liệu (EAGAIN): chờ bằng poll
            pollfd p{fd, POLLIN, 0};
            polls++;
            if (poll(&p, 1, dgrams ? idle : 10000) == 0) break;   // im lặng đủ lâu: in kết quả
            continue;
        }
        auto now = Clock::now();
        if (!dgrams) t_first = now;
        t_last = now;
        for (int i = 0; i < got; i++) {
            dgrams++;
            unsigned len = msgs[i].msg_len;
            if (len % kFrame) trunc++;
            for (unsigned off = 0; off + kFrame <= len; off += kFrame) {
                Frame f;
                if (!get_frame(&store[i * BUF + off], f)) { bad++; continue; }
                if (plain) pdevs[f.dev].see(f.seq); else devs[f.dev].see(f.seq);
                if (work_us) { auto end = Clock::now() + std::chrono::microseconds(work_us); while (Clock::now() < end) {} }
            }
        }
    }
    double s = std::chrono::duration<double>(t_last - t_first).count();
    long drops = udp_counter("RcvbufErrors") - drops0;
    printf("gw1: %ld datagram, %ld lan goi nhan + %ld poll (%.1f datagram/lan goi), %.3f s, %.0f datagram/s\n",
           dgrams, calls, polls, calls ? (double)dgrams / calls : 0.0, s, s > 0 ? dgrams / s : 0.0);
    rusage ru; getrusage(RUSAGE_SELF, &ru);
    double us = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6, sy = ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
    long frames = 0; for (auto& [id, d] : devs) frames += d.frames; for (auto& [id, d] : pdevs) frames += d.frames;
    printf("     CPU cua gw1: user %.3f s, sys %.3f s; %.0f ns/datagram, %.0f ns/frame\n", us, sy,
           dgrams ? (us + sy) * 1e9 / dgrams : 0.0, frames ? (us + sy) * 1e9 / frames : 0.0);
    printf("     frame sai magic %ld, datagram le byte %ld, kernel bo vi rcvbuf day %ld\n", bad, trunc, drops);
    for (auto& [id, d] : devs)
        printf("     dev %u: nhan %ld, mat %ld, trung %ld, dao thu tu %ld, qua tre %ld, khoi dong lai %ld\n",
               id, d.frames, d.lost, d.dup, d.reorder, d.late, d.restart);
    for (auto& [id, d] : pdevs) printf("     dev %u (plain): nhan %ld, mat %ld\n", id, d.frames, d.lost);
}
