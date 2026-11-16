// busy: K thread quay vòng tính toán trong 2 giây, ghim cùng một core hoặc để tự do.
// In số context switch tự nguyện / bị ép của từng thread (đọc /proc/self/task/<tid>/status). Lời giải Lab 2.3.
// Build: g++ -O2 -std=c++20 -pthread -o busy busy.cpp
// Chạy:  ./busy <K> pin|free
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <pthread.h>
#include <sched.h>
#include <string>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>
#include <vector>

static std::string ctx(pid_t tid) {
    std::ifstream f("/proc/self/task/" + std::to_string(tid) + "/status");
    std::string l, out;
    while (std::getline(f, l))
        if (l.find("ctxt_switches") != std::string::npos) out += l.substr(0, l.find(':')).substr(0, 3) + "=" + l.substr(l.find_last_of("\t ") + 1) + " ";
    return out;   // "vol=… non=…"
}

int main(int argc, char** argv) {
    int k = argc > 1 ? std::atoi(argv[1]) : 2;
    bool pin = argc > 2 && std::strcmp(argv[2], "pin") == 0;
    std::atomic<bool> go{true};
    std::vector<std::thread> ts;
    std::vector<std::string> res(k);
    for (int i = 0; i < k; ++i)
        ts.emplace_back([&, i] {
            if (pin) { cpu_set_t s; CPU_ZERO(&s); CPU_SET(0, &s); pthread_setaffinity_np(pthread_self(), sizeof s, &s); }
            volatile unsigned long x = 0;
            while (go.load(std::memory_order_relaxed)) x = x + 1;          // không bao giờ tự nhường CPU
            res[i] = ctx(static_cast<pid_t>(syscall(SYS_gettid)));
        });
    std::this_thread::sleep_for(std::chrono::seconds(2));
    go = false;
    for (auto& t : ts) t.join();
    for (int i = 0; i < k; ++i) std::printf("thread %d (%s): %s\n", i, pin ? "pin core 0" : "tu do", res[i].c_str());
}
