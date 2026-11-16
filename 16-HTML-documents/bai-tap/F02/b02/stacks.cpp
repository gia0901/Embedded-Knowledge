// stacks: tạo N thread đang ngủ, in VmSize/VmRSS của process. So stack mặc định với stack 64 KB.
// Lời giải một phần của Lab 2.4. Build: g++ -O2 -std=c++20 -pthread -o stacks stacks.cpp
// Chạy:  ./stacks <số thread> [kích thước stack, KB; 0 = mặc định]
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>

static void* sleeper(void*) { pause(); return nullptr; }   // giữ thread sống, không tốn CPU

static void print_mem() {
    std::ifstream f("/proc/self/status");
    for (std::string l; std::getline(f, l);)
        if (l.rfind("VmSize", 0) == 0 || l.rfind("VmRSS", 0) == 0 || l.rfind("Threads", 0) == 0) std::printf("  %s\n", l.c_str());
}

int main(int argc, char** argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 1000;
    long kb = argc > 2 ? std::atol(argv[2]) : 0;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (kb) {
        int rc = pthread_attr_setstacksize(&attr, kb * 1024);
        if (rc) { std::printf("setstacksize(%ld KB): %s\n", kb, std::strerror(rc)); return 1; }   // trả mã lỗi, không đặt errno
    }
    std::vector<pthread_t> ts(n);
    for (int i = 0; i < n; ++i) {
        int rc = pthread_create(&ts[i], &attr, sleeper, nullptr);
        if (rc) { std::printf("pthread_create thread %d: %s\n", i, std::strerror(rc)); return 1; }
    }
    std::printf("%d thread, stack %s:\n", n, kb ? (std::to_string(kb) + " KB").c_str() : "mac dinh");
    print_mem();
    return 0;   // exit() kết thúc mọi thread
}
