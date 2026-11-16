// wakelat: task tuần hoàn 1 ms (như vòng điều khiển hay thread gửi heartbeat), đo độ trễ đánh thức:
// thời điểm thực sự chạy trừ thời điểm hẹn. Giống cyclictest thu nhỏ. Lời giải Lab 3.2.
// Build: g++ -O2 -std=c++20 -o wakelat wakelat.cpp
// Chạy:  ./wakelat <số chu kỳ> other|other1|fifo <số hog cùng core>   (other1 = other + timer slack 1 ns)
#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

static constexpr int kCore = 1;
static constexpr long kPeriodNs = 1'000'000;

static void pin(int core) { cpu_set_t s; CPU_ZERO(&s); CPU_SET(core, &s); sched_setaffinity(0, sizeof s, &s); }
static long ns(const timespec& t) { return t.tv_sec * 1'000'000'000L + t.tv_nsec; }

int main(int argc, char** argv) {
    int loops = atoi(argv[1]);
    bool fifo = std::strcmp(argv[2], "fifo") == 0;
    int hogs = atoi(argv[3]);
    std::vector<pid_t> hp;
    for (int i = 0; i < hogs; ++i) {
        pid_t p = fork();
        if (p == 0) { pin(kCore); for (;;) {} }                     // hog: tranh CPU với task đo, policy thường
        hp.push_back(p);
    }
    pin(kCore);
    if (std::strcmp(argv[2], "other1") == 0) prctl(PR_SET_TIMERSLACK, 1);   // mặc định 50 us cho task thường
    if (fifo) {
        sched_param sp{.sched_priority = 80};
        if (sched_setscheduler(0, SCHED_FIFO, &sp)) {
            std::perror("sched_setscheduler");
            for (pid_t p : hp) kill(p, SIGKILL);                   // đừng bỏ lại hog quay vòng mãi
            while (wait(nullptr) > 0) {}
            return 1;
        }
    }
    std::vector<long> lat(loops);
    timespec next; clock_gettime(CLOCK_MONOTONIC, &next);
    for (int i = 0; i < loops; ++i) {
        next.tv_nsec += kPeriodNs;
        if (next.tv_nsec >= 1'000'000'000) { next.tv_nsec -= 1'000'000'000; ++next.tv_sec; }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);   // hẹn giờ tuyệt đối: không trôi
        timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
        lat[i] = ns(now) - ns(next);
    }
    for (pid_t p : hp) kill(p, SIGKILL);
    while (wait(nullptr) > 0) {}
    std::sort(lat.begin(), lat.end());
    long sum = 0; for (long v : lat) sum += v;
    auto pct = [&](double q) { return lat[static_cast<size_t>(q * (loops - 1))] / 1000.0; };
    std::printf("%-6s hog=%d  min %6.1f  avg %7.1f  p50 %6.1f  p99 %7.1f  max %7.1f us\n",
                argv[2], hogs, lat[0] / 1000.0, sum / 1000.0 / loops, pct(.5), pct(.99), lat.back() / 1000.0);
}
