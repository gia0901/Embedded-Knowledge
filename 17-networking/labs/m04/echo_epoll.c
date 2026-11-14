// echo_epoll.c — TCP echo server một thread, nhiều client, dùng epoll (Linux)
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 9200
#define MAX_EVENTS 64

static void die(const char *msg) { perror(msg); exit(1); }

int main(void) {
    signal(SIGPIPE, SIG_IGN);                     // ghi vào socket đã đóng: nhận EPIPE thay vì chết

    int lfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (lfd < 0) die("socket");
    int one = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(PORT),
                                .sin_addr.s_addr = htonl(INADDR_ANY) };
    if (bind(lfd, (struct sockaddr *)&addr, sizeof addr) < 0) die("bind");
    if (listen(lfd, SOMAXCONN) < 0) die("listen");

    int ep = epoll_create1(0);
    if (ep < 0) die("epoll_create1");
    struct epoll_event ev = { .events = EPOLLIN, .data.fd = lfd };
    if (epoll_ctl(ep, EPOLL_CTL_ADD, lfd, &ev) < 0) die("epoll_ctl");
    printf("echo server on :%d (1 thread, epoll)\n", PORT);

    struct epoll_event events[MAX_EVENTS];
    int clients = 0;
    for (;;) {
        int n = epoll_wait(ep, events, MAX_EVENTS, -1);   // ngủ tới khi có fd sẵn sàng
        if (n < 0) { if (errno == EINTR) continue; die("epoll_wait"); }
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == lfd) {                       // listening socket: có connection mới
                for (;;) {
                    struct sockaddr_in peer;
                    socklen_t len = sizeof peer;
                    int cfd = accept4(lfd, (struct sockaddr *)&peer, &len, SOCK_NONBLOCK);
                    if (cfd < 0) break;            // EAGAIN: đã lấy hết connection đang chờ
                    struct epoll_event cev = { .events = EPOLLIN, .data.fd = cfd };
                    epoll_ctl(ep, EPOLL_CTL_ADD, cfd, &cev);
                    clients++;
                    if (clients % 100 == 1) {      // log thưa để không làm chậm load test
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof ip);
                        printf("+ %s:%d fd=%d (%d clients)\n", ip, ntohs(peer.sin_port), cfd, clients);
                    }
                }
            } else {                               // connection socket: có dữ liệu hoặc peer đã đóng
                char buf[4096];
                ssize_t r = read(fd, buf, sizeof buf);
                if (r > 0) {
                    // Demo: giả định write() ghi hết. Code thật phải giữ phần chưa ghi
                    // và đăng ký EPOLLOUT để ghi tiếp khi socket sẵn sàng.
                    if (write(fd, buf, (size_t)r) < 0 && errno != EAGAIN) perror("write");
                } else if (r == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                    epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);                     // quên dòng này → CLOSE-WAIT tích tụ
                    clients--;
                    if (clients == 0) printf("- all clients gone\n");
                }
            }
        }
    }
}
