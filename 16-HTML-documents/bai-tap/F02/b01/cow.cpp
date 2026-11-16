// cow: đo cấp phát lười và copy-on-write bằng số minor page fault mà kernel báo (getrusage).
// Build: g++ -O2 -std=c++20 -o cow cow.cpp
#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static long minflt() { rusage r; getrusage(RUSAGE_SELF, &r); return r.ru_minflt; }

int main() {
    const size_t SZ = 64 << 20;                                    // 64 MB = 16384 page 4 KB
    long f0 = minflt();
    char* p = static_cast<char*>(mmap(nullptr, SZ, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (p == MAP_FAILED) return 1;
    long f1 = minflt();
    std::memset(p, 1, SZ);                                         // chạm mọi page
    long f2 = minflt();
    std::printf("mmap 64 MB          : %ld page fault\n", f1 - f0);
    std::printf("ghi lan dau 64 MB   : %ld page fault\n", f2 - f1);

    std::fflush(stdout);                    // xả buffer TRƯỚC fork, nếu không con sẽ in lại phần của cha
    pid_t pid = fork();
    if (pid == 0) {                                                // con: dùng chung page với cha
        long c0 = minflt(); long s = 0;
        for (size_t i = 0; i < SZ; i += 4096) s += p[i];          // chỉ đọc
        long c1 = minflt();
        for (size_t i = 0; i < SZ; i += 4096) p[i] = 2;            // ghi: mỗi page bị sao chép
        long c2 = minflt();
        std::printf("[con] doc sau fork  : %ld page fault (s=%ld)\n", c1 - c0, s / 4096);
        std::printf("[con] ghi sau fork  : %ld page fault\n", c2 - c1);
        std::fflush(stdout);                                       // _exit không xả buffer của stdio
        _exit(0);
    }
    int st;
    waitpid(pid, &st, 0);
    std::printf("[cha] p[0] van la   : %d\n", p[0]);
    munmap(p, SZ);
}
