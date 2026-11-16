// share: chạy một process quay vòng cho mỗi giá trị nice, tất cả ghim core 0 (hoặc tự do với "free").
// In CPU time mỗi process nhận được trong T giây. Lời giải Lab 3.1.
// Build: g++ -O2 -std=c++20 -o share share.cpp
// Chạy:  ./share <giây> <nice1> [nice2 ...] [free]   (giá trị "rt" = SCHED_FIFO priority 50 thay cho nice)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sched.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

static double now(clockid_t c) { timespec t; clock_gettime(c, &t); return t.tv_sec + t.tv_nsec / 1e9; }

int main(int argc, char** argv) {
    double secs = atof(argv[1]);
    bool pin = std::strcmp(argv[argc - 1], "free") != 0;
    int n = argc - 2 - (pin ? 0 : 1);
    std::vector<int> fds(n);
    double end = now(CLOCK_MONOTONIC) + secs;                          // chung một mốc kết thúc cho mọi con
    for (int i = 0; i < n; ++i) {
        int p[2]; if (pipe(p)) return 1; fds[i] = p[0];
        if (fork() == 0) {
            if (pin) { cpu_set_t s; CPU_ZERO(&s); CPU_SET(0, &s); sched_setaffinity(0, sizeof s, &s); }
            if (std::strcmp(argv[2 + i], "rt") == 0) {
                sched_param sp{.sched_priority = 50};
                sched_setscheduler(0, SCHED_FIFO, &sp);                // realtime: chạy trước mọi task thường
            } else {
                setpriority(PRIO_PROCESS, 0, atoi(argv[2 + i]));      // nice của chính process này
            }
            while (now(CLOCK_MONOTONIC) < end) {}                      // quay vòng, không bao giờ tự ngủ
            double cpu = now(CLOCK_PROCESS_CPUTIME_ID);               // CPU time thực sự nhận được
            if (write(p[1], &cpu, sizeof cpu) < 0) _exit(1);
            _exit(0);
        }
        close(p[1]);
    }
    std::vector<double> cpu(n); double sum = 0;
    for (int i = 0; i < n; ++i) { if (read(fds[i], &cpu[i], sizeof(double)) <= 0) cpu[i] = 0; sum += cpu[i]; }
    while (wait(nullptr) > 0) {}
    for (int i = 0; i < n; ++i)
        std::printf("nice %3s  cpu %5.2f s  %5.1f%%\n", argv[2 + i], cpu[i], 100 * cpu[i] / sum);
}
