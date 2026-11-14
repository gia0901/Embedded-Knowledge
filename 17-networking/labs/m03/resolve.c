// resolve.c — gọi getaddrinfo() giống mọi client C/C++ trước khi connect()
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e3 + ts.tv_nsec / 1e6;
}

int main(int argc, char **argv) {
    const char *host = argc > 1 ? argv[1] : "example.com";
    const char *port = argc > 2 ? argv[2] : "443";
    struct addrinfo hints = {0}, *res, *p;
    hints.ai_family = AF_UNSPEC;       /* cả IPv4 và IPv6 */
    hints.ai_socktype = SOCK_STREAM;   /* address dùng cho TCP */

    double t0 = now_ms();
    int rc = getaddrinfo(host, port, &hints, &res);
    double t1 = now_ms();
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo(%s): %s (%.2f ms)\n", host, gai_strerror(rc), t1 - t0);
        return 1;
    }
    printf("%s resolved in %.2f ms\n", host, t1 - t0);
    for (p = res; p != NULL; p = p->ai_next) {
        char ip[INET6_ADDRSTRLEN];
        void *addr = (p->ai_family == AF_INET)
            ? (void *)&((struct sockaddr_in *)p->ai_addr)->sin_addr
            : (void *)&((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
        inet_ntop(p->ai_family, addr, ip, sizeof ip);
        printf("  %-4s %s port %s\n", p->ai_family == AF_INET ? "IPv4" : "IPv6", ip, port);
    }
    freeaddrinfo(res);
    return 0;
}
