// tlb: bảng lớn truy cập ngẫu nhiên, cấp phát bằng trang 4 KB và bằng hugepage 2 MB (THP qua madvise).
//   chase : đọc phụ thuộc theo hoán vị ngẫu nhiên (độ trễ thuần, như latency ở Bài 3)
//   update: mỗi frame cộng vào một ô ngẫu nhiên của bảng (kiểu bảng flow/conntrack), các lần cập nhật độc lập
// Lời giải Lab 5.2. Build: g++ -O2 -std=c++20 -I.. -o tlb tlb.cpp      Chạy: ./tlb [MB...]
#include "bench.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <numeric>
#include <random>
#include <string>
#include <sys/mman.h>

// Cấp phát n byte căn theo 2 MB; huge = true: xin THP, false: cấm THP.
static void* alloc(size_t n, bool huge) {
    const size_t A = 2 << 20;
    char* p = (char*)mmap(nullptr, n + A, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("mmap"); std::exit(1); }
    char* q = (char*)(((uintptr_t)p + A - 1) & ~(A - 1));
    madvise(q, n, huge ? MADV_HUGEPAGE : MADV_NOHUGEPAGE);
    return q;                                   // phần thừa đầu/cuối bỏ qua cho gọn
}

static long anon_huge_kb() {                    // AnonHugePages của chính tiến trình này
    std::ifstream f("/proc/self/smaps_rollup");
    for (std::string line; std::getline(f, line);)
        if (line.rfind("AnonHugePages:", 0) == 0) return std::atol(line.c_str() + 14);
    return -1;
}

int main(int argc, char** argv) {
    std::vector<size_t> mbs;
    for (int i = 1; i < argc; ++i) mbs.push_back(std::atol(argv[i]));
    if (mbs.empty()) mbs = {1, 8, 64, 512, 2048};
    std::printf("%8s %6s %10s %10s %14s\n", "bang", "trang", "chase ns", "update ns", "AnonHuge (MB)");
    for (size_t mb : mbs) {
        const size_t bytes = mb << 20, n = bytes / 8;
        for (bool huge : {false, true}) {
            long before = anon_huge_kb();
            auto* t = (uint64_t*)alloc(bytes, huge);
            // chase: mỗi ô 64 B một nút, nối theo hoán vị ngẫu nhiên
            const size_t nodes = bytes / 64;
            std::vector<uint32_t> perm(nodes);
            std::iota(perm.begin(), perm.end(), 0);
            std::shuffle(perm.begin() + 1, perm.end(), std::mt19937(1));
            for (size_t i = 0; i < nodes; ++i) t[perm[i] * 8] = perm[(i + 1) % nodes] * 8;
            long thp = (anon_huge_kb() - before) / 1024;
            const size_t steps = 4'000'000;
            char name[64];
            std::snprintf(name, sizeof name, "chase %zu MB %s", mb, huge ? "2M" : "4K");
            uint64_t p = 0;
            double c = bench_ns(name, steps, 5, [&] { for (size_t i = 0; i < steps; ++i) p = t[p]; keep(p); });
            // update: chỉ số ngẫu nhiên tính từ số thứ tự frame (hash), không phụ thuộc lần trước
            std::snprintf(name, sizeof name, "update %zu MB %s", mb, huge ? "2M" : "4K");
            double u = bench_ns(name, steps, 5, [&] {
                for (uint64_t i = 0; i < steps; ++i) t[((i * 0x9E3779B97F4A7C15ull) >> 20) % n] += 1;
            });
            std::printf("%6zu MB %6s %10.2f %10.2f %14ld\n", mb, huge ? "2 MB" : "4 KB", c, u, thp);
            munmap(t, bytes);       // chỉ trả phần đã căn; đủ cho bài lab
        }
    }
}
