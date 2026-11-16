// backend: giả lập server backend nhận lô frame từ gw2 qua TCP (khung theo wire.h, payload = N x 16 byte).
// Build: g++ -O2 -std=c++20 -Wall -o backend backend.cpp
// Chạy:  ./backend PORT [--slow-us U] [--rcvbuf B]
//   --slow-us U   ngủ U µs sau mỗi lô, giả lập ghi DB chậm
//   --rcvbuf B    đặt SO_RCVBUF B byte (tắt autotuning) để bộ đệm kernel không che hàng đợi của gw2
// Mỗi giây in số frame nhận; khi gw2 ngắt kết nối thì in tổng và chờ kết nối mới.
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "../b04/frame.h"
#include "wire.h"

static volatile sig_atomic_t stop = 0;
static double now_s() { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s port [--slow-us U] [--rcvbuf B]\n", argv[0]); return 1; }
    long slow = 0, rcvbuf = 0;
    for (int i = 2; i + 1 < argc; i += 2) {
        std::string a = argv[i];
        if (a == "--slow-us") slow = atol(argv[i + 1]); else if (a == "--rcvbuf") rcvbuf = atol(argv[i + 1]);
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    struct sigaction sa{}; sa.sa_handler = [](int) { stop = 1; };   // không SA_RESTART: poll() trả EINTR
    sigaction(SIGTERM, &sa, nullptr); sigaction(SIGINT, &sa, nullptr);
    int ls = socket(AF_INET, SOCK_STREAM, 0), one = 1;
    if (rcvbuf) setsockopt(ls, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf);   // socket accept() kế thừa
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(atoi(argv[1]));
    if (bind(ls, (sockaddr*)&a, sizeof a) < 0 || listen(ls, 16) < 0) { perror("bind/listen"); return 1; }
    printf("backend nghe cong %s, slow %ld us/lo, rcvbuf %s\n", argv[1], slow, rcvbuf ? std::to_string(rcvbuf).c_str() : "auto"); fflush(stdout);
    long total = 0;
    for (;;) {
        int fd = accept(ls, nullptr, nullptr);
        if (fd < 0) { if (stop) return 0; continue; }
        printf("gw ket noi\n"); fflush(stdout);
        Decoder d; std::string m; char buf[65536];
        long frames = 0, batches = 0, bad = 0, sec_frames = 0;
        double t0 = now_s(), tick = t0 + 1;
        auto report = [&] {
            for (double t = now_s(); t >= tick; tick += 1) {
                printf("  t=%2.0f  %6ld frame/s\n", tick - t0, sec_frames); fflush(stdout);
                sec_frames = 0;
            }
        };
        for (;;) {
            pollfd p{fd, POLLIN, 0};
            int pr = poll(&p, 1, 200);
            report();
            if (stop) break;
            if (pr <= 0) continue;
            ssize_t r = read(fd, buf, sizeof buf);
            if (r <= 0) break;
            d.feed(buf, r);
            int k;
            while ((k = d.next(m)) == 1) {
                batches++;
                for (size_t o = 0; o + kFrame <= m.size(); o += kFrame) {
                    Frame f;
                    if (get_frame((const uint8_t*)m.data() + o, f)) { frames++; sec_frames++; } else bad++;
                }
                if (slow) usleep(slow);
                report();
            }
            if (k < 0) { printf("khung hong, dong\n"); break; }
        }
        total += frames;
        printf("%s: phien nay %ld frame trong %ld lo, sai %ld, du %zu byte chua thanh lo | tong %ld frame\n",
               stop ? "backend dung" : "gw ngat", frames, batches, bad, d.pending(), total);
        fflush(stdout);
        close(fd);
        if (stop) return 0;
    }
}
