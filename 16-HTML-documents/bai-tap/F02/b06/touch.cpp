// touch: xin N MB bằng mmap rồi chạm (ghi 1 byte) vào từng page 4 KB. In VmRSS, số page fault và thời gian.
// Chế độ: none = xin nhưng không chạm; touch = chạm từng page; populate = MAP_POPULATE (kernel cấp sẵn lúc mmap);
//         huge = madvise(MADV_HUGEPAGE) trước khi chạm (page 2 MB). Lời giải Lab 6.1.
// Build: g++ -O2 -std=c++20 -o touch touch.cpp
// Chạy:  ./touch <MB> none|touch|populate|huge
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <sys/resource.h>

static long rss_kb() {
    std::ifstream f("/proc/self/status");
    for (std::string l; std::getline(f, l);)
        if (l.rfind("VmRSS:", 0) == 0) return std::atol(l.c_str() + 6);
    return -1;
}
static long minflt() { rusage r; getrusage(RUSAGE_SELF, &r); return r.ru_minflt; }

int main(int argc, char** argv) {
    size_t mb = std::atol(argv[1]);
    const char* mode = argv[2];
    size_t sz = mb << 20;
    long rss0 = rss_kb(), f0 = minflt();
    auto t0 = std::chrono::steady_clock::now();
    int flags = MAP_PRIVATE | MAP_ANONYMOUS | (std::strcmp(mode, "populate") == 0 ? MAP_POPULATE : 0);
    char* p = static_cast<char*>(mmap(nullptr, sz, PROT_READ | PROT_WRITE, flags, -1, 0));
    if (p == MAP_FAILED) { std::perror("mmap"); return 1; }
    if (std::strcmp(mode, "huge") == 0) madvise(p, sz, MADV_HUGEPAGE);
    long rss1 = rss_kb();
    if (std::strcmp(mode, "none") != 0)
        for (size_t off = 0; off < sz; off += 4096) p[off] = 1;      // lần ghi đầu vào mỗi page: page fault
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    long rss2 = rss_kb(), faults = minflt() - f0;
    std::printf("%-8s %5zu MB: RSS %6ld -> %6ld (sau mmap) -> %7ld kB, minor fault %7ld, %7.1f ms",
                mode, mb, rss0, rss1, rss2, faults, ms);
    if (faults) std::printf(", %.2f us/fault", ms * 1000 / faults);
    if (std::strcmp(mode, "none") != 0) {                         // đọc ngẫu nhiên 20 triệu lần: đo chi phí cache miss + TLB miss
        auto t1 = std::chrono::steady_clock::now();
        unsigned long x = 12345, sum = 0;
        for (int i = 0; i < 20'000'000; ++i) {                      // địa chỉ sau phụ thuộc giá trị vừa đọc: không chồng được các lần miss
            x = x * 6364136223846793005UL + 1442695040888963407UL + p[(x >> 16) % sz];
            sum += x & 1;
        }
        double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t1).count() / 20e6;
        std::printf(", doc ngau nhien %.1f ns/lan (sum %lu)", ns, sum);
    }
    std::printf("\n");
}
