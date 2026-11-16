// telemd v3: telemd v2.2 (Bài 7) đổi thread rx thành một event loop epoll.
//  - Một epoll chờ ba loại fd: socket UDP (non-blocking), timerfd (mỗi giây in thống kê),
//    signalfd (SIGTERM/SIGINT dừng, SIGHUP in thống kê ngay). Không còn SO_RCVTIMEO thức dậy 10 lần/giây.
//  - recvmmsg nhận tối đa 64 bản tin mỗi system call; cả lô được đưa vào queue một lần.
//  - SO_RXQ_OVFL: kernel báo số gói socket này đã đánh rơi vì receive buffer đầy.
// Lời giải Lab 8.4. Build: g++ -O2 -g -std=c++20 -pthread -o telemd telemd.cpp
// Chạy:  ./telemd [port=9000] [số worker=2] [file trạng thái=state.json] [rcvbuf byte=4194304] [lô=64]
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

using Batch = std::vector<std::string>;          // v3: mỗi phần tử của queue là một lô bản tin

struct Queue {                                   // bounded queue như Bài 2, thêm size() cho watchdog
    std::mutex m;
    std::condition_variable not_empty, not_full;
    std::deque<Batch> q;
    size_t cap = 1024;
    bool closed = false;
    void push(Batch s) {
        std::unique_lock lk(m);
        not_full.wait(lk, [&] { return q.size() < cap || closed; });
        if (closed) return;
        q.push_back(std::move(s));
        lk.unlock();
        not_empty.notify_one();
    }
    bool pop(Batch& out) {
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
static std::string g_state = "state.json";
static std::vector<std::atomic<long>> g_beat(16);   // heartbeat của từng worker

static void raise_alarm(int dev) {
    long seen;
    { std::lock_guard g(g_stats_mu); seen = g_per_dev[dev]; ++g_alarms; }   // cần lock để đọc bảng đếm
    if (seen % 500 == 0) std::fprintf(stderr, "alarm dev %d (da nhan %ld ban tin)\n", dev, seen);
}

static void worker(int id) {
    pthread_setname_np(pthread_self(), ("wk-" + std::to_string(id)).c_str());
    Batch batch;
    while (g_q.pop(batch)) {
        for (auto& msg : batch) {
            int dev; double temp;
            if (std::sscanf(msg.c_str(), "dev=%d temp=%lf", &dev, &temp) == 2) {
                { std::lock_guard g(g_stats_mu); ++g_per_dev[dev]; ++g_total; }
                if (temp >= 24.9) raise_alarm(dev);      // nhả lock trước khi gọi hàm khác (Bài 5)
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

static bool save_state() {                          // ghi atomic: người đọc và lần khởi động sau thấy bản cũ hoặc bản mới
    std::map<int, long> copy; long total;
    { std::lock_guard g(g_stats_mu); copy = g_per_dev; total = g_total; }   // chép trong lock, ghi file ngoài lock
    std::string s = "{\"total\":" + std::to_string(total) + ",\"dev\":[";
    for (auto [dev, cnt] : copy) s += "[" + std::to_string(dev) + "," + std::to_string(cnt) + "],";
    if (s.back() == ',') s.pop_back();
    s += "],\"end\":" + std::to_string(total) + "}\n";
    std::string tmp = g_state + ".tmp";
    int fd = open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0) return false;
    bool ok = write(fd, s.data(), s.size()) == static_cast<ssize_t>(s.size()) && fsync(fd) == 0;
    close(fd);
    if (!ok || rename(tmp.c_str(), g_state.c_str()) != 0) return false;
    int dfd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);   // file trạng thái nằm ở thư mục hiện tại
    if (dfd >= 0) { fsync(dfd); close(dfd); }
    return true;
}

static void load_state() {
    FILE* f = std::fopen(g_state.c_str(), "r");
    if (!f) { std::printf("chua co %s, bat dau tu 0\n", g_state.c_str()); return; }
    std::string s; char buf[4096]; size_t n;
    while ((n = std::fread(buf, 1, sizeof buf, f)) > 0) s.append(buf, n);
    std::fclose(f);
    long total = -1, end = -2;
    size_t e = s.rfind("\"end\":");
    if (std::sscanf(s.c_str(), "{\"total\":%ld", &total) != 1 || e == std::string::npos ||
        std::sscanf(s.c_str() + e, "\"end\":%ld", &end) != 1 || total != end) {
        std::printf("%s hong (%zu byte), bo qua va bat dau tu 0\n", g_state.c_str(), s.size());
        return;
    }
    std::map<int, long> m;
    size_t p = s.find("\"dev\":[");
    if (p != std::string::npos) p += 7;                 // bỏ qua `"dev":[`, từ đây là các cặp [dev,count]
    while (p != std::string::npos && (p = s.find('[', p)) != std::string::npos && p < e) {
        int dev; long cnt;
        if (std::sscanf(s.c_str() + p, "[%d,%ld]", &dev, &cnt) == 2) m[dev] = cnt;
        ++p;
    }
    std::lock_guard g(g_stats_mu);
    g_per_dev = std::move(m);
    g_total = total;
    std::printf("khoi phuc tu %s: %ld ban tin, %zu device\n", g_state.c_str(), g_total, g_per_dev.size());
}

static void snapshot_loop() {
    pthread_setname_np(pthread_self(), "snap");
    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!save_state()) std::perror("save_state");
    }
}

static long g_rx = 0, g_drops = 0, g_syscalls = 0;      // chỉ thread event loop ghi

static void on_socket(int fd, int batch_max) {          // đọc tới khi EAGAIN (đúng cho cả LT và ET)
    std::vector<mmsghdr> msgs(batch_max);
    std::vector<iovec> iov(batch_max);
    std::vector<char> bufs(batch_max * 512);
    std::vector<char> ctl(batch_max * CMSG_SPACE(sizeof(uint32_t)));
    for (;;) {
        for (int i = 0; i < batch_max; ++i) {
            iov[i] = {&bufs[i * 512], 511};
            msgs[i].msg_hdr = {};
            msgs[i].msg_hdr.msg_iov = &iov[i];
            msgs[i].msg_hdr.msg_iovlen = 1;
            msgs[i].msg_hdr.msg_control = &ctl[i * CMSG_SPACE(sizeof(uint32_t))];
            msgs[i].msg_hdr.msg_controllen = CMSG_SPACE(sizeof(uint32_t));
        }
        int n = recvmmsg(fd, msgs.data(), batch_max, MSG_DONTWAIT, nullptr);
        ++g_syscalls;
        if (n <= 0) return;                             // EAGAIN: hết dữ liệu, quay về epoll_wait
        Batch b;
        b.reserve(n);
        for (int i = 0; i < n; ++i) {
            b.emplace_back(&bufs[i * 512], msgs[i].msg_len);
            for (cmsghdr* c = CMSG_FIRSTHDR(&msgs[i].msg_hdr); c; c = CMSG_NXTHDR(&msgs[i].msg_hdr, c))
                if (c->cmsg_level == SOL_SOCKET && c->cmsg_type == SO_RXQ_OVFL)
                    g_drops = *reinterpret_cast<uint32_t*>(CMSG_DATA(c));   // tổng số gói đã rơi
        }
        g_rx += n;
        g_q.push(std::move(b));
        if (n < batch_max) return;                      // lô chưa đầy: socket đã cạn
    }
}

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 9000;
    int nw = argc > 2 ? std::atoi(argv[2]) : 2;
    if (argc > 3) g_state = argv[3];
    int rcvbuf = argc > 4 ? std::atoi(argv[4]) : 4 << 20;
    int batch_max = argc > 5 ? std::atoi(argv[5]) : 64;
    load_state();

    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGHUP);
    pthread_sigmask(SIG_BLOCK, &sigs, nullptr);         // chặn trước khi tạo thread: mọi thread thừa hưởng
    int sfd = signalfd(-1, &sigs, SFD_CLOEXEC);          // signal đến dưới dạng dữ liệu đọc được từ fd

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) { std::perror("bind"); return 1; }
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof rcvbuf);
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_RXQ_OVFL, &one, sizeof one);

    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    itimerspec its{{1, 0}, {1, 0}};                     // lần đầu sau 1 s, rồi mỗi 1 s
    timerfd_settime(tfd, 0, &its, nullptr);

    int ep = epoll_create1(EPOLL_CLOEXEC);
    for (int f : {fd, tfd, sfd}) {
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = f;
        epoll_ctl(ep, EPOLL_CTL_ADD, f, &ev);
    }

    std::vector<std::thread> ws;
    for (int i = 0; i < nw; ++i) ws.emplace_back(worker, i);
    std::thread wd(watchdog, nw);
    std::thread snap(snapshot_loop);
    socklen_t sl = sizeof rcvbuf;
    getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &sl);
    std::printf("telemd v3 pid=%d port=%d worker=%d rcvbuf=%d lo=%d\n", getpid(), port, nw, rcvbuf, batch_max);
    std::fflush(stdout);

    long last_rx = 0, wakeups = 0;
    int sig = 0;
    while (!sig) {
        epoll_event evs[8];
        int n = epoll_wait(ep, evs, 8, -1);             // ngủ hẳn tới khi có việc
        ++wakeups;
        for (int i = 0; i < n; ++i) {
            int f = evs[i].data.fd;
            if (f == fd) {
                on_socket(fd, batch_max);
            } else if (f == tfd) {
                uint64_t ticks;
                if (read(tfd, &ticks, 8) != 8) continue;    // phải đọc, nếu không LT báo mãi
                if (g_rx != last_rx)
                    std::printf("1s: nhan %ld ban tin, roi %ld, queue %zu\n", g_rx - last_rx, g_drops, g_q.size());
                last_rx = g_rx;
                std::fflush(stdout);
            } else if (f == sfd) {
                signalfd_siginfo si;
                if (read(sfd, &si, sizeof si) != sizeof si) continue;
                if (si.ssi_signo == SIGHUP)
                    std::printf("SIGHUP: nhan %ld, roi %ld, %ld recvmmsg, %ld lan thuc day\n", g_rx, g_drops, g_syscalls, wakeups);
                else sig = si.ssi_signo;
                std::fflush(stdout);
            }
        }
    }
    g_stop = true;
    g_q.close();
    for (auto& w : ws) w.join();
    wd.join();
    snap.join();
    save_state();                                       // lần ghi cuối khi dừng sạch
    std::printf("dung sach (signal %d): %ld ban tin, %ld canh bao, roi %ld, %ld recvmmsg, %ld lan thuc day\n",
                sig, g_total, g_alarms, g_drops, g_syscalls, wakeups);
    close(fd);
}
