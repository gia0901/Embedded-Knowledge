// telemd v2.1: telemd v2 (Bài 2) thêm hai thứ.
//  1. Cảnh báo nhiệt độ: bản tin có temp >= 24.9 thì gọi raise_alarm(), in số bản tin của device đó.
//     Chế độ "bug" gọi raise_alarm() khi đang giữ g_stats_mu, mà raise_alarm() lại khóa g_stats_mu: tự deadlock.
//  2. Watchdog: mỗi worker tăng heartbeat sau mỗi bản tin; thread "wdog" thấy một worker có việc mà
//     heartbeat đứng yên quá 2 giây thì in cảnh báo và abort() để để lại core dump.
// Lời giải Lab 5.2 và 5.4. Build: g++ -O2 -g -std=c++20 -pthread -o telemd telemd.cpp
// Chạy:  ./telemd [port=9000] [số worker=2] [ok|bug]
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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

struct Queue {                                   // bounded queue như Bài 2, thêm size() cho watchdog
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
        if (q.empty()) return false;
        out = std::move(q.front());
        q.pop_front();
        lk.unlock();
        not_full.notify_one();
        return true;
    }
    size_t size() { std::lock_guard g(m); return q.size(); }
    void close() { { std::lock_guard g(m); closed = true; } not_empty.notify_all(); not_full.notify_all(); }
};

static std::atomic<bool> g_stop{false};
static Queue g_q;
static std::mutex g_stats_mu;
static std::map<int, long> g_per_dev;
static long g_total = 0;
static long g_alarms = 0;
static bool g_bug = false;
static std::vector<std::atomic<long>> g_beat(16);   // heartbeat của từng worker

static void raise_alarm(int dev) {
    long seen;
    { std::lock_guard g(g_stats_mu); seen = g_per_dev[dev]; ++g_alarms; }   // cần lock để đọc bảng đếm
    if (seen % 500 == 0) std::fprintf(stderr, "alarm dev %d (da nhan %ld ban tin)\n", dev, seen);
}

static void rx_loop(int fd) {
    pthread_setname_np(pthread_self(), "rx");
    char buf[512];
    while (!g_stop) {
        ssize_t n = recv(fd, buf, sizeof buf - 1, 0);
        if (n > 0) g_q.push(std::string(buf, n));
    }
    g_q.close();
}

static void worker(int id) {
    pthread_setname_np(pthread_self(), ("wk-" + std::to_string(id)).c_str());
    std::string msg;
    while (g_q.pop(msg)) {
        int dev; double temp;
        if (std::sscanf(msg.c_str(), "dev=%d temp=%lf", &dev, &temp) == 2) {
            if (g_bug) {
                std::lock_guard g(g_stats_mu);
                ++g_per_dev[dev];
                ++g_total;
                if (temp >= 24.9) raise_alarm(dev);      // BUG: đang giữ g_stats_mu, raise_alarm khóa lại lần nữa
            } else {
                { std::lock_guard g(g_stats_mu); ++g_per_dev[dev]; ++g_total; }
                if (temp >= 24.9) raise_alarm(dev);      // đúng: nhả lock trước khi gọi hàm khác
            }
        }
        g_beat[id].fetch_add(1, std::memory_order_relaxed);
    }
}

static void watchdog(int nw) {
    pthread_setname_np(pthread_self(), "wdog");
    std::vector<long> last(nw, -1);
    std::vector<int> still(nw, 0);
    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        bool backlog = g_q.size() > 0;                  // queue rỗng thì worker đứng yên là bình thường
        for (int i = 0; i < nw; ++i) {
            long b = g_beat[i].load(std::memory_order_relaxed);
            still[i] = (b == last[i] && backlog) ? still[i] + 1 : 0;
            last[i] = b;
            if (still[i] >= 4) {                        // 4 x 500 ms
                std::fprintf(stderr, "wdog: wk-%d dung yen 2 s (heartbeat %ld, queue %zu), abort de lay core dump\n", i, b, g_q.size());
                std::abort();
            }
        }
    }
}

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 9000;
    int nw = argc > 2 ? std::atoi(argv[2]) : 2;
    g_bug = argc > 3 && std::strcmp(argv[3], "bug") == 0;

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
    int rcvbuf = 4 << 20;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf);

    std::thread rx(rx_loop, fd);
    std::vector<std::thread> ws;
    for (int i = 0; i < nw; ++i) ws.emplace_back(worker, i);
    std::thread wd(watchdog, nw);
    std::printf("telemd v2.1 pid=%d port=%d worker=%d %s\n", getpid(), port, nw, g_bug ? "(bug)" : "");
    std::fflush(stdout);

    int sig;
    sigwait(&sigs, &sig);
    g_stop = true;
    rx.join();
    for (auto& w : ws) w.join();
    wd.join();
    std::printf("dung sach (signal %d): %ld ban tin, %ld canh bao\n", sig, g_total, g_alarms);
    close(fd);
}
