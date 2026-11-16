// fsyncbench: ghi N lần, mỗi lần 4 KB nối tiếp vào một file, theo năm cách bền vững khác nhau.
// In thời gian trung bình, p50, p99 của mỗi lần ghi (tính cả fsync nếu có). Lời giải Lab 7.2.
// Build: g++ -O2 -std=c++20 -o fsyncbench fsyncbench.cpp
// Chạy:  ./fsyncbench <file> <N> none|fdatasync|fsync|osync|odirect
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

int main(int argc, char** argv) {
    const char* path = argv[1];
    int n = std::atoi(argv[2]);
    const char* mode = argv[3];
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
    if (!std::strcmp(mode, "osync")) flags |= O_SYNC;            // mỗi write chỉ trả về khi dữ liệu đã xuống disk
    if (!std::strcmp(mode, "odirect")) flags |= O_DIRECT;        // bỏ qua page cache; buffer phải căn 4096
    int fd = open(path, flags, 0644);
    if (fd < 0) { std::perror("open"); return 1; }
    void* buf;
    if (posix_memalign(&buf, 4096, 4096)) return 1;
    std::memset(buf, 'x', 4096);
    std::vector<double> us(n);
    for (int i = 0; i < n; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        if (pwrite(fd, buf, 4096, static_cast<off_t>(i) * 4096) != 4096) { std::perror("pwrite"); return 1; }
        if (!std::strcmp(mode, "fdatasync")) fdatasync(fd);      // dữ liệu + metadata cần để đọc lại (kích thước)
        if (!std::strcmp(mode, "fsync")) fsync(fd);              // dữ liệu + mọi metadata (mtime...)
        us[i] = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count();
    }
    close(fd);
    double sum = 0; for (double v : us) sum += v;
    std::sort(us.begin(), us.end());
    std::printf("%-9s %d x 4 KB: tb %8.1f us  p50 %8.1f  p99 %8.1f  tong %7.1f ms\n",
                mode, n, sum / n, us[n / 2], us[n * 99 / 100], sum / 1000);
}
