// telemd v2: nhiều thread. Thread "rx" nhận UDP và đẩy vào queue; K thread "wk-i" lấy ra, parse,
// cập nhật bảng đếm theo device; thread chính chờ SIGTERM/SIGINT bằng sigwait rồi dừng mọi thread.
// Queue và lock giải thích ở Bài 4; ở Bài 2 chỉ cần biết mỗi thread làm gì và nhìn chúng từ bên ngoài.
// Build: g++ -O2 -std=c++20 -pthread -o telemd telemd.cpp
// Chạy:  ./telemd [port=9000] [số worker=2]
#include <arpa/inet.h>
#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

struct Queue {                                   // bounded queue: mutex + 2 condition variable (Bài 4)
    std::mutex m;
    std::condition_variable not_empty, not_full;
    std::deque<std::string> q;
    size_t cap = 1024;
    bool closed = false;
    void push(std::string s) {
        std::unique_lock lk(m);
        not_full.wait(lk, [&] { return q.size() < cap || closed; });
        if (closed) return;
        q.push_back(std::move(s));
        lk.unlock();
        not_empty.notify_one();
    }
    bool pop(std::string& out) {
        std::unique_lock lk(m);
        not_empty.wait(lk, [&] { return !q.empty() || closed; });
        if (q.empty()) return false;               // đã đóng và hết việc
        out = std::move(q.front());
        q.pop_front();
        lk.unlock();
        not_full.notify_one();
        return true;
    }
    void close() { { std::lock_guard g(m); closed = true; } not_empty.notify_all(); not_full.notify_all(); }
};

static std::atomic<bool> g_stop{false};
static Queue g_q;
static std::mutex g_stats_mu;
static std::map<int, long> g_per_dev;
static long g_total = 0;

static void rx_loop(int fd) {
    pthread_setname_np(pthread_self(), "rx");
    char buf[512];
    while (!g_stop) {
        ssize_t n = recv(fd, buf, sizeof buf - 1, 0);      // có SO_RCVTIMEO 100 ms: định kỳ kiểm tra g_stop
        if (n > 0) g_q.push(std::string(buf, n));
    }
    g_q.close();
}

static void worker(int id) {
    pthread_setname_np(pthread_self(), ("wk-" + std::to_string(id)).c_str());   // tối đa 15 ký tự
    std::string msg;
    while (g_q.pop(msg)) {
        int dev; double temp;
        if (std::sscanf(msg.c_str(), "dev=%d temp=%lf", &dev, &temp) != 2) continue;
        std::lock_guard g(g_stats_mu);                     // bảng đếm dùng chung giữa các worker
        ++g_per_dev[dev];
        ++g_total;
    }
}

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 9000;
    int nw = argc > 2 ? std::atoi(argv[2]) : 2;

    // chặn SIGTERM/SIGINT TRƯỚC khi tạo thread: thread con thừa hưởng mask, chỉ main nhận qua sigwait
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigs, nullptr);

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) { std::perror("bind"); return 1; }
    timeval tv{0, 100'000};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    int rcvbuf = 4 << 20;                                  // socket buffer lớn hơn: chịu được đợt packet dồn
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf);

    std::thread rx(rx_loop, fd);
    std::vector<std::thread> ws;
    for (int i = 0; i < nw; ++i) ws.emplace_back(worker, i);
    std::printf("telemd v2 pid=%d port=%d worker=%d\n", getpid(), port, nw);
    std::fflush(stdout);

    int sig;
    sigwait(&sigs, &sig);                                  // main ngủ ở đây tới khi có SIGTERM/SIGINT
    g_stop = true;
    rx.join();
    for (auto& w : ws) w.join();
    for (auto [dev, cnt] : g_per_dev) std::printf("dev %d: %ld\n", dev, cnt);
    std::printf("dung sach (signal %d): %ld ban tin\n", sig, g_total);
    close(fd);
}
