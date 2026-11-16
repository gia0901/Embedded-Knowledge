// tcpcli: client TCP giống thiết bị giữ kết nối lâu tới server, để đo khi nào phát hiện server mất.
// Build: g++ -O2 -std=c++20 -Wall -o tcpcli tcpcli.cpp
// Chạy:  ./tcpcli IP PORT [--every MS] [--ka IDLE,INTVL,CNT] [--uto MS] [--for S]
//   --every MS          gửi một dòng mỗi MS mili giây (0 = không gửi gì, chỉ chờ đọc)
//   --ka IDLE,INTVL,CNT bật TCP keepalive: sau IDLE s im lặng, dò mỗi INTVL s, CNT lần không trả lời thì bỏ
//   --uto MS            TCP_USER_TIMEOUT: dữ liệu đã gửi chưa được ACK quá MS thì bỏ kết nối
//   --for S             tự thoát sau S giây nếu chưa có lỗi (mặc định 60)
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using Clock = std::chrono::steady_clock;
static Clock::time_point t0;
static double now_s() { return std::chrono::duration<double>(Clock::now() - t0).count(); }

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s ip port [--every MS] [--ka I,V,C] [--uto MS] [--for S]\n", argv[0]); return 1; }
    int every = 0, ka_i = 0, ka_v = 0, ka_c = 0, uto = 0, dur = 60;
    for (int i = 3; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--every" && i + 1 < argc) every = atoi(argv[++i]);
        else if (a == "--ka" && i + 1 < argc) sscanf(argv[++i], "%d,%d,%d", &ka_i, &ka_v, &ka_c);
        else if (a == "--uto" && i + 1 < argc) uto = atoi(argv[++i]);
        else if (a == "--for" && i + 1 < argc) dur = atoi(argv[++i]);
    }
    t0 = Clock::now();
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ka_i) {
        int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);
        setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &ka_i, sizeof ka_i);
        setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &ka_v, sizeof ka_v);
        setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &ka_c, sizeof ka_c);
    }
    if (uto) setsockopt(fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &uto, sizeof uto);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &a.sin_addr);
    if (connect(fd, (sockaddr*)&a, sizeof a) < 0) { printf("%7.3f s  connect loi: %s\n", now_s(), strerror(errno)); return 1; }
    sockaddr_in me{}; socklen_t ml = sizeof me; getsockname(fd, (sockaddr*)&me, &ml);
    printf("%7.3f s  da ket noi tu cong %d\n", now_s(), ntohs(me.sin_port)); fflush(stdout);
    long sent = 0, got = 0; double next = every ? every / 1000.0 : 1e9;
    char buf[4096];
    while (now_s() < dur) {
        int wait_ms = (int)std::max(0.0, std::min(next - now_s(), 1.0) * 1000);
        pollfd p{fd, POLLIN, 0};
        int r = poll(&p, 1, wait_ms);
        if (r > 0) {
            ssize_t n = read(fd, buf, sizeof buf);
            if (n > 0) { got += n; continue; }
            int e = errno;
            printf("%7.3f s  read: %s\n", now_s(), n == 0 ? "EOF (phia kia dong)" : strerror(e));
            printf("          da gui %ld dong, nhan %ld byte\n", sent, got);
            return 0;
        }
        if (every && now_s() >= next) {
            char line[64]; int len = snprintf(line, sizeof line, "dong %ld\n", sent);
            if (send(fd, line, len, MSG_NOSIGNAL) < 0) {
                printf("%7.3f s  send: %s\n", now_s(), strerror(errno));
                printf("          da gui %ld dong, nhan %ld byte\n", sent, got);
                return 0;
            }
            sent++; next += every / 1000.0;
        }
    }
    printf("%7.3f s  het thoi gian, khong phat hien loi; da gui %ld dong, nhan %ld byte\n", now_s(), sent, got);
}
