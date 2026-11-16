// frag: cấp phát 1 triệu khối nhỏ, giải phóng gần hết nhưng giữ lại 1/64, xem RSS có giảm không.
// Sau đó gọi malloc_trim(0) và xem lại. Lời giải Lab 6.4.
// Build: g++ -O2 -std=c++20 -o frag frag.cpp
// Chạy:  ./frag <kích thước khối, byte> keep|freeall [số khối, mặc định 1000000]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <malloc.h>
#include <string>
#include <vector>

static long rss_kb() {
    std::ifstream f("/proc/self/status");
    for (std::string l; std::getline(f, l);)
        if (l.rfind("VmRSS:", 0) == 0) return std::atol(l.c_str() + 6);
    return -1;
}

int main(int argc, char** argv) {
    size_t sz = std::atol(argv[1]);
    bool keep = std::strcmp(argv[2], "keep") == 0;
    const int n = argc > 3 ? std::atoi(argv[3]) : 1'000'000;
    std::vector<char*> p(n);
    long r0 = rss_kb();
    for (int i = 0; i < n; ++i) { p[i] = static_cast<char*>(std::malloc(sz)); std::memset(p[i], 1, sz); }
    long r1 = rss_kb();
    for (int i = 0; i < n; ++i)
        if (!keep || i % 64 != 0) { std::free(p[i]); p[i] = nullptr; }   // keep: còn lại 1/64 rải đều trong heap
    long r2 = rss_kb();
    malloc_trim(0);                                                      // xin glibc trả page rảnh về kernel
    long r3 = rss_kb();
    std::printf("khoi %4zu B, %-7s: dau %6ld, sau malloc %7ld, sau free %7ld, sau malloc_trim %7ld kB\n",
                sz, argv[2], r0, r1, r2, r3);
}
