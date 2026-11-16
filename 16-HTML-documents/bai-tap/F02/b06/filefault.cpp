// filefault: mmap một file chỉ đọc, đọc 1 byte mỗi page 4 KB, in số major/minor fault và thời gian.
// Chạy hai lần: lần đầu sau khi xóa page cache (major fault: phải đọc disk), lần sau page đã ở cache (minor).
// Build: g++ -O2 -std=c++20 -o filefault filefault.cpp
// Chạy:  ./filefault <file>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { std::perror("open"); return 1; }
    struct stat st; fstat(fd, &st);
    auto* p = static_cast<const unsigned char*>(mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    rusage a; getrusage(RUSAGE_SELF, &a);
    auto t0 = std::chrono::steady_clock::now();
    unsigned long sum = 0;
    for (off_t off = 0; off < st.st_size; off += 4096) sum += p[off];
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    rusage b; getrusage(RUSAGE_SELF, &b);
    std::printf("%ld MB: major fault %ld, minor fault %ld, %.1f ms (sum %lu)\n", static_cast<long>(st.st_size >> 20),
                b.ru_majflt - a.ru_majflt, b.ru_minflt - a.ru_minflt, ms, sum);
}
