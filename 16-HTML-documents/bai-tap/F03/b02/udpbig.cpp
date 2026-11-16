// udpbig: gửi một datagram UDP lớn, có hoặc không cho phép phân mảnh, rồi in path MTU kernel đang nhớ.
// Build: g++ -O2 -std=c++20 -o udpbig udpbig.cpp
// Chạy:  ./udpbig 10.0.2.2 5000 3000 [df]
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s ip port bytes [df]\n", argv[0]); return 1; }
    bool df = argc > 4 && !strcmp(argv[4], "df");
    sockaddr_in dst{}; dst.sin_family = AF_INET; dst.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &dst.sin_addr);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // IP_PMTUDISC_DO: đặt cờ DF, không bao giờ phân mảnh. IP_PMTUDISC_DONT: cho kernel phân mảnh.
    int v = df ? IP_PMTUDISC_DO : IP_PMTUDISC_DONT;
    setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &v, sizeof v);
    connect(fd, (sockaddr*)&dst, sizeof dst);        // connect để đọc được IP_MTU và nhận lỗi ICMP
    std::vector<char> buf(atoi(argv[3]), 'x');
    for (int i = 0; i < 2; i++) {                      // gửi hai lần: lần đầu có thể chưa biết path MTU
        ssize_t n = send(fd, buf.data(), buf.size(), 0);
        int mtu = 0; socklen_t l = sizeof mtu;
        getsockopt(fd, IPPROTO_IP, IP_MTU, &mtu, &l);
        if (n < 0) printf("lan %d: send loi: %s, IP_MTU=%d\n", i + 1, strerror(errno), mtu);
        else       printf("lan %d: gui %zd byte, df=%d, IP_MTU=%d\n", i + 1, n, df, mtu);
        usleep(200000);
    }
}
