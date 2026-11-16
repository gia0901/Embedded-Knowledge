// ctxsw: đo chi phí context switch. Hai thread chuyền một byte qua lại bằng hai pipe.
// Mỗi vòng = 2 write + 2 read + 2 lần một thread ngủ và thread kia thức. Ghim cả hai vào CÙNG một core
// để buộc kernel chuyển ngữ cảnh mỗi lần; rồi so với hai core khác nhau.
// Build: g++ -O2 -std=c++20 -o ctxsw ctxsw.cpp
#include <chrono>
#include <cstdio>
#include <pthread.h>
#include <sched.h>
#include <thread>
#include <unistd.h>

static void pin(int cpu) {
    cpu_set_t s; CPU_ZERO(&s); CPU_SET(cpu, &s);
    pthread_setaffinity_np(pthread_self(), sizeof s, &s);
}

static double pingpong(int cpu_a, int cpu_b) {
    int ab[2], ba[2];
    if (pipe(ab) || pipe(ba)) return -1;
    const int rounds = 100000;
    std::thread peer([&] {
        pin(cpu_b); char c;
        for (int i = 0; i < rounds; ++i) { if (read(ab[0], &c, 1) != 1 || write(ba[1], &c, 1) != 1) return; }
    });
    pin(cpu_a); char c = 'x';
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < rounds; ++i) { if (write(ab[1], &c, 1) != 1 || read(ba[0], &c, 1) != 1) break; }
    auto t1 = std::chrono::steady_clock::now();
    peer.join();
    for (int fd : {ab[0], ab[1], ba[0], ba[1]}) close(fd);
    return std::chrono::duration<double, std::micro>(t1 - t0).count() / rounds;
}

int main() {
    std::printf("cung 1 core : %5.2f us moi vong (2 lan context switch)\n", pingpong(0, 0));
    std::printf("2 core khac : %5.2f us moi vong (danh thuc thread o core khac)\n", pingpong(0, 1));
}
