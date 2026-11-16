// reorder: thí nghiệm store-load (kiểu Dekker). Thread A: x = 1; r1 = y.  Thread B: y = 1; r2 = x.
// Nếu mọi lệnh chạy đúng thứ tự viết thì không thể có r1 == 0 và r2 == 0. Đếm số lần điều đó vẫn xảy ra.
// Lời giải Lab 4.4. Build: g++ -O2 -std=c++20 -pthread -o reorder reorder.cpp
// Chạy:  ./reorder <số vòng> relaxed|acqrel|seqcst
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <thread>

static std::atomic<int> x, y, r1, r2;
static std::atomic<long> round_no{0};        // main tăng để mở một vòng mới
static std::atomic<int> done{0};              // mỗi thread báo xong vòng

static void pin(int core) { cpu_set_t s; CPU_ZERO(&s); CPU_SET(core, &s); pthread_setaffinity_np(pthread_self(), sizeof s, &s); }

template <std::memory_order St, std::memory_order Ld>
static void side(std::atomic<int>& mine, std::atomic<int>& other, std::atomic<int>& r, int core, long rounds) {
    pin(core);
    for (long i = 1; i <= rounds; ++i) {
        while (round_no.load(std::memory_order_acquire) != i) {}    // chờ main mở vòng i
        mine.store(1, St);
        r.store(other.load(Ld), std::memory_order_relaxed);
        done.fetch_add(1, std::memory_order_acq_rel);
    }
}

template <std::memory_order St, std::memory_order Ld> static long run(long rounds) {
    std::thread a(side<St, Ld>, std::ref(x), std::ref(y), std::ref(r1), 1, rounds);
    std::thread b(side<St, Ld>, std::ref(y), std::ref(x), std::ref(r2), 2, rounds);
    long both_zero = 0;
    for (long i = 1; i <= rounds; ++i) {
        x = 0; y = 0; done = 0;
        round_no.store(i, std::memory_order_release);
        while (done.load(std::memory_order_acquire) != 2) {}
        if (r1 == 0 && r2 == 0) ++both_zero;
    }
    a.join(); b.join();
    return both_zero;
}

int main(int argc, char** argv) {
    long n = std::atol(argv[1]);
    const char* m = argv[2];
    long z = !std::strcmp(m, "relaxed") ? run<std::memory_order_relaxed, std::memory_order_relaxed>(n)
           : !std::strcmp(m, "acqrel")  ? run<std::memory_order_release, std::memory_order_acquire>(n)
                                        : run<std::memory_order_seq_cst, std::memory_order_seq_cst>(n);
    std::printf("%-8s %ld vong: r1 == r2 == 0 xay ra %ld lan (%.3f%%)\n", m, n, z, 100.0 * z / n);
}
