// pollbench: chi phí một lần chờ khi có N fd mở nhưng chỉ 1 fd có dữ liệu. So poll với epoll.
// Lời giải Lab 8.1. Build: g++ -O2 -std=c++20 -o pollbench pollbench.cpp
// Chạy:  ./pollbench <N fd> [số lần chờ=20000]
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <unistd.h>
#include <vector>

int main(int argc, char** argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 1000;
    int iters = argc > 2 ? std::atoi(argv[2]) : 20000;
    rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    rl.rlim_cur = rl.rlim_max;                          // mỗi pipe tốn 2 fd; soft limit mặc định chỉ 1024
    setrlimit(RLIMIT_NOFILE, &rl);
    std::vector<int> rd(n), wr(n);
    for (int i = 0; i < n; ++i) {
        int p[2];
        if (pipe2(p, O_NONBLOCK | O_CLOEXEC)) { std::perror("pipe2"); return 1; }
        rd[i] = p[0]; wr[i] = p[1];
    }
    char c = 'x';
    if (write(wr[n - 1], &c, 1) != 1) return 1;        // chỉ fd cuối có dữ liệu, và không ai đọc nó

    std::vector<pollfd> pf(n);
    for (int i = 0; i < n; ++i) pf[i] = {rd[i], POLLIN, 0};
    auto t0 = std::chrono::steady_clock::now();
    long ready = 0;
    for (int k = 0; k < iters; ++k) ready += poll(pf.data(), n, 0);   // kernel duyệt cả n fd mỗi lần
    auto t1 = std::chrono::steady_clock::now();

    int ep = epoll_create1(EPOLL_CLOEXEC);
    for (int i = 0; i < n; ++i) {
        epoll_event ev{};
        ev.events = EPOLLIN;                            // level-triggered: còn dữ liệu là còn báo
        ev.data.u32 = i;
        epoll_ctl(ep, EPOLL_CTL_ADD, rd[i], &ev);       // đăng ký một lần
    }
    epoll_event evs[64];
    auto t2 = std::chrono::steady_clock::now();
    for (int k = 0; k < iters; ++k) ready += epoll_wait(ep, evs, 64, 0);   // chỉ trả fd sẵn sàng
    auto t3 = std::chrono::steady_clock::now();

    auto us = [&](auto a, auto b) { return std::chrono::duration<double, std::micro>(b - a).count() / iters; };
    std::printf("N=%-6d poll %8.2f us/lan   epoll_wait %6.2f us/lan   (san sang: %ld/%d)\n",
                n, us(t0, t1), us(t2, t3), ready, 2 * iters);
}
