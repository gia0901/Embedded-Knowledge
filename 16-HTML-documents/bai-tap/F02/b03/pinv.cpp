// pinv: priority inversion trên một core, ba thread SCHED_FIFO như Mars Pathfinder 1997.
// L (prio 10) giữ mutex làm 50 ms việc; H (prio 30) cần mutex đó; M (prio 20) không dùng mutex, quay 300 ms.
// In thời gian H phải chờ mutex, với mutex thường và với mutex priority inheritance. Lời giải Lab 3.3.
// Build: g++ -O2 -std=c++20 -pthread -o pinv pinv.cpp
// Chạy:  sudo ./pinv normal|inherit [nopin]   (nopin: không ghim core, để đối chứng)
#include <cstdio>
#include <cstring>
#include <ctime>
#include <pthread.h>
#include <sched.h>

static pthread_mutex_t g_mu;
static timespec g_t0;
static bool g_pin = true;

static double ms_since(const timespec& a) {
    timespec b; clock_gettime(CLOCK_MONOTONIC, &b);
    return (b.tv_sec - a.tv_sec) * 1e3 + (b.tv_nsec - a.tv_nsec) / 1e6;
}
static void sleep_until_ms(int ms) {                       // hẹn giờ theo mốc chung g_t0
    timespec t = g_t0;
    t.tv_nsec += ms * 1'000'000L;
    t.tv_sec += t.tv_nsec / 1'000'000'000; t.tv_nsec %= 1'000'000'000;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, nullptr);
}
static void work_ms(int ms) {                              // đốt đúng ms mili giây CPU của chính thread này
    timespec s; clock_gettime(CLOCK_THREAD_CPUTIME_ID, &s);
    for (;;) {
        timespec n; clock_gettime(CLOCK_THREAD_CPUTIME_ID, &n);
        if ((n.tv_sec - s.tv_sec) * 1e3 + (n.tv_nsec - s.tv_nsec) / 1e6 >= ms) return;
    }
}

static void* low(void*) {
    sleep_until_ms(0);
    pthread_mutex_lock(&g_mu);
    std::printf("%6.1f ms  L lấy mutex\n", ms_since(g_t0));
    work_ms(50);
    pthread_mutex_unlock(&g_mu);
    std::printf("%6.1f ms  L nhả mutex\n", ms_since(g_t0));
    return nullptr;
}
static void* medium(void*) {
    sleep_until_ms(10);
    std::printf("%6.1f ms  M bắt đầu quay 300 ms\n", ms_since(g_t0));
    work_ms(300);
    std::printf("%6.1f ms  M xong\n", ms_since(g_t0));
    return nullptr;
}
static void* high(void*) {
    sleep_until_ms(5);
    timespec a; clock_gettime(CLOCK_MONOTONIC, &a);
    pthread_mutex_lock(&g_mu);
    std::printf("%6.1f ms  H có mutex sau khi chờ %.1f ms\n", ms_since(g_t0), ms_since(a));
    pthread_mutex_unlock(&g_mu);
    return nullptr;
}

static pthread_t spawn(void* (*fn)(void*), int prio) {
    pthread_attr_t a; pthread_attr_init(&a);
    pthread_attr_setinheritsched(&a, PTHREAD_EXPLICIT_SCHED);   // không thì thread con lấy policy của main
    pthread_attr_setschedpolicy(&a, SCHED_FIFO);
    sched_param sp{.sched_priority = prio};
    pthread_attr_setschedparam(&a, &sp);
    cpu_set_t s; CPU_ZERO(&s); CPU_SET(3, &s);
    if (g_pin) pthread_attr_setaffinity_np(&a, sizeof s, &s);   // cả ba trên core 3: tranh nhau một CPU
    pthread_t t;
    if (int rc = pthread_create(&t, &a, fn, nullptr)) { std::printf("pthread_create: %s\n", std::strerror(rc)); }
    return t;
}

int main(int argc, char** argv) {
    bool pi = argc > 1 && std::strcmp(argv[1], "inherit") == 0;
    g_pin = !(argc > 2 && std::strcmp(argv[2], "nopin") == 0);
    pthread_mutexattr_t ma; pthread_mutexattr_init(&ma);
    if (pi) pthread_mutexattr_setprotocol(&ma, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&g_mu, &ma);
    std::printf("mutex %s\n", pi ? "PTHREAD_PRIO_INHERIT" : "thuong");
    clock_gettime(CLOCK_MONOTONIC, &g_t0);
    g_t0.tv_nsec += 20'000'000;                                 // mốc 0 = 20 ms sau: đủ thời gian tạo xong cả ba thread
    if (g_t0.tv_nsec >= 1'000'000'000) { g_t0.tv_nsec -= 1'000'000'000; ++g_t0.tv_sec; }
    pthread_t l = spawn(low, 10), m = spawn(medium, 20), h = spawn(high, 30);
    pthread_join(l, nullptr); pthread_join(m, nullptr); pthread_join(h, nullptr);
}
