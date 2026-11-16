// telemd v2.2: telemd v2.1 (Bài 5, bỏ chế độ bug) thêm snapshot bảng đếm vào state.json.
//  - Khởi động: đọc state.json nếu có và nguyên vẹn, khôi phục bảng đếm; hỏng thì bỏ qua và báo.
//  - Thread "snap": mỗi giây chép bảng đếm (trong lock) rồi ghi ra file (ngoài lock) theo cách atomic:
//    state.json.tmp → fsync → rename → fsync thư mục. Khi nhận SIGTERM ghi thêm một lần cuối.
// Lời giải Lab 7.4. Build: g++ -O2 -g -std=c++20 -pthread -o telemd telemd.cpp
// Chạy:  ./telemd [port=9000] [số worker=2] [file trạng thái=state.json]
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
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
static std::string g_state = "state.json";
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
            { std::lock_guard g(g_stats_mu); ++g_per_dev[dev]; ++g_total; }
            if (temp >= 24.9) raise_alarm(dev);          // nhả lock trước khi gọi hàm khác (Bài 5)
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

int main(int argc, char** argv) {
    int port = argc > 1 ? std::atoi(argv[1]) : 9000;
    int nw = argc > 2 ? std::atoi(argv[2]) : 2;
    if (argc > 3) g_state = argv[3];
    load_state();

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
    std::thread snap(snapshot_loop);
    std::printf("telemd v2.2 pid=%d port=%d worker=%d\n", getpid(), port, nw);
    std::fflush(stdout);

    int sig;
    sigwait(&sigs, &sig);
    g_stop = true;
    rx.join();
    for (auto& w : ws) w.join();
    wd.join();
    snap.join();
    save_state();                                       // lần ghi cuối khi dừng sạch
    std::printf("dung sach (signal %d): %ld ban tin, %ld canh bao\n", sig, g_total, g_alarms);
    close(fd);
}
