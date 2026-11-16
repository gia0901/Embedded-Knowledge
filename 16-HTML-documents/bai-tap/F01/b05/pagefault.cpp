// pagefault: giá của lần chạm đầu. Cấp 512 MB rồi ghi 1 byte mỗi 4 KB, đếm page fault và thời gian:
//   4K      : mmap thường, cấm THP     -> mỗi trang 4 KB một fault
//   2M      : mmap + MADV_HUGEPAGE     -> mỗi 2 MB một fault
//   populate: mmap MAP_POPULATE, cấm THP -> kernel cấp sẵn mọi trang ngay trong mmap
// Lời giải Lab 5.3. Build: g++ -O2 -std=c++20 -o pagefault pagefault.cpp     Chạy: ./pagefault [MB=512]
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <sys/resource.h>

static long minflt() { rusage r; getrusage(RUSAGE_SELF, &r); return r.ru_minflt; }
static double ms_since(std::chrono::steady_clock::time_point a) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - a).count();
}

int main(int argc, char** argv) {
    const size_t mb = argc > 1 ? std::atol(argv[1]) : 512, n = mb << 20, A = 2 << 20;
    std::printf("%-9s %10s %10s %12s %12s\n", "cach", "mmap ms", "cham ms", "page fault", "ns/4KB");
    for (int mode = 0; mode < 3; ++mode) {
        const char* name[] = {"4K", "2M", "populate"};
        long f0 = minflt();
        auto a = std::chrono::steady_clock::now();
        int flags = MAP_PRIVATE | MAP_ANONYMOUS | (mode == 2 ? MAP_POPULATE : 0);
        // populate: kernel cấp mọi trang ngay trong mmap. THP của máy lab ở chế độ [madvise], vùng này không
        // xin THP nên được cấp bằng trang 4 KB.
        char* p = (char*)mmap(nullptr, n + A, PROT_READ | PROT_WRITE, flags, -1, 0);
        if (p == MAP_FAILED) { perror("mmap"); return 1; }
        char* q = (char*)(((uintptr_t)p + A - 1) & ~(A - 1));
        if (mode == 0) madvise(q, n, MADV_NOHUGEPAGE);
        if (mode == 1) madvise(q, n, MADV_HUGEPAGE);
        double t_map = ms_since(a);
        auto b = std::chrono::steady_clock::now();
        for (size_t i = 0; i < n; i += 4096) q[i] = 1;          // chạm mỗi trang 4 KB một lần
        double t_touch = ms_since(b);
        long f = minflt() - f0;
        std::printf("%-9s %10.1f %10.1f %12ld %12.1f\n", name[mode], t_map, t_touch, f, (t_map + t_touch) * 1e6 / (n / 4096));
        munmap(p, n + A);
    }
}
