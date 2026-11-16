// dl: task SCHED_DEADLINE. Mỗi chu kỳ (period) làm work_us micro giây việc rồi sched_yield() để báo xong job.
// In thời gian từ đầu chu kỳ tới lúc job xong. Lời giải Lab 3.4.
// Build: g++ -O2 -std=c++20 -o dl dl.cpp
// Chạy:  sudo ./dl <runtime_us> <period_us> <work_us> <số job>
#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

struct sched_attr {                                     // glibc chưa bọc sched_setattr: tự khai báo theo man 2 sched_setattr
    uint32_t size, sched_policy;
    uint64_t sched_flags;
    int32_t sched_nice;
    uint32_t sched_priority;
    uint64_t sched_runtime, sched_deadline, sched_period;
};

static long now_us() { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1'000'000L + t.tv_nsec / 1000; }
static void work_us(long us) {                          // đốt đúng us micro giây CPU của thread này
    auto cpu = [] { timespec t; clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t); return t.tv_sec * 1'000'000L + t.tv_nsec / 1000; };
    long s = cpu();
    while (cpu() - s < us) {}
}

int main(int argc, char** argv) {
    long runtime = atol(argv[1]), period = atol(argv[2]), work = atol(argv[3]);
    int jobs = atoi(argv[4]);
    sched_attr a{};
    a.size = sizeof a;
    a.sched_policy = SCHED_DEADLINE;
    a.sched_runtime = runtime * 1000;                   // đơn vị ns
    a.sched_deadline = period * 1000;                   // deadline = period: job phải xong trong chu kỳ của nó
    a.sched_period = period * 1000;
    if (syscall(SYS_sched_setattr, 0, &a, 0)) {
        std::printf("sched_setattr(runtime %ld us, period %ld us): %s\n", runtime, period, std::strerror(errno));
        return 1;
    }
    std::vector<long> done(jobs);
    sched_yield();                                      // bắt đầu đúng đầu một chu kỳ
    for (int i = 0; i < jobs; ++i) {
        long start = now_us();
        work_us(work);
        done[i] = now_us() - start;
        sched_yield();                                  // với SCHED_DEADLINE: "job này xong", ngủ tới chu kỳ sau
    }
    std::sort(done.begin(), done.end());
    int late = std::count_if(done.begin(), done.end(), [&](long v) { return v > period; });
    std::printf("runtime %ld / period %ld us, work %ld us: job xong sau p50 %ld us, max %ld us, %d/%d job lỡ deadline\n",
                runtime, period, work, done[jobs / 2], done.back(), late, jobs);
}
