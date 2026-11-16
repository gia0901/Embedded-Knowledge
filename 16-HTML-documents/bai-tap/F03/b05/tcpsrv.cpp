// tcpsrv: server TCP tối giản để quan sát bind, TIME_WAIT và RST.
// Build: g++ -O2 -std=c++20 -Wall -o tcpsrv tcpsrv.cpp
// Chạy:  ./tcpsrv PORT [--reuseaddr] [--linger0] [--once]
//   nhận một dòng, trả "ok <dòng>", rồi server đóng trước (active close).
//   --reuseaddr  đặt SO_REUSEADDR trước bind
//   --linger0    SO_LINGER {1, 0}: close() gửi RST thay vì FIN, không có TIME_WAIT
//   --once       phục vụ một kết nối rồi thoát
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s port [--reuseaddr] [--linger0] [--once]\n", argv[0]); return 1; }
    bool reuse = false, linger0 = false, once = false;
    for (int i = 2; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--reuseaddr") reuse = true; else if (a == "--linger0") linger0 = true; else if (a == "--once") once = true;
    }
    int ls = socket(AF_INET, SOCK_STREAM, 0), one = 1;
    if (reuse) setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(atoi(argv[1])); a.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(ls, (sockaddr*)&a, sizeof a) < 0) { printf("tcpsrv: bind loi: %s\n", strerror(errno)); return 1; }
    listen(ls, 16);
    printf("tcpsrv: nghe :%s%s%s\n", argv[1], reuse ? " (SO_REUSEADDR)" : "", linger0 ? " (linger0: dong bang RST)" : "");
    fflush(stdout);
    do {
        sockaddr_in p{}; socklen_t pl = sizeof p;
        int c = accept(ls, (sockaddr*)&p, &pl);
        if (c < 0) { perror("accept"); continue; }
        char buf[256]; ssize_t n = read(c, buf, sizeof buf - 1);
        if (n < 0) n = 0;
        buf[n] = 0; if (n && buf[n - 1] == '\n') buf[n - 1] = 0;
        std::string reply = "ok " + std::string(buf) + "\n";
        if (write(c, reply.data(), reply.size()) < 0) perror("write");
        if (linger0) { linger l{1, 0}; setsockopt(c, SOL_SOCKET, SO_LINGER, &l, sizeof l); }
        close(c);
        printf("tcpsrv: phuc vu %s:%d, nhan \"%s\", da dong\n", inet_ntoa(p.sin_addr), ntohs(p.sin_port), buf);
        fflush(stdout);
    } while (!once);
    close(ls);
}
