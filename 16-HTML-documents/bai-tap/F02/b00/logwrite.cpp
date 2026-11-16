// logwrite: ghi 100.000 dòng log theo hai cách, đếm thời gian. Dùng cho Lab 0.3.
// Build: g++ -O2 -std=c++20 -o logwrite logwrite.cpp
// Chạy:  ./logwrite direct  log.txt      # mỗi dòng một write()
//        ./logwrite buffered log.txt      # gom vào buffer 64 KB rồi mới write()
//        ./logwrite buffered log.txt crash  # abort() trước khi xả buffer cuối
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "dùng: %s direct|buffered file [crash]\n", argv[0]); return 2; }
    bool buffered = std::strcmp(argv[1], "buffered") == 0;
    bool crash = argc > 3;
    int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0) { std::perror("open"); return 1; }
    const int N = 100'000;
    std::string buf;
    buf.reserve(1 << 16);
    auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i) {
        char line[64];
        int len = std::snprintf(line, sizeof line, "dev=%d temp=21.5 seq=%d\n", i % 4, i);
        if (!buffered) { if (write(fd, line, len) != len) return 1; continue; }
        buf.append(line, len);
        if (buf.size() > (1 << 16) - 64) {                 // buffer gần đầy: một write cho ~2000 dòng
            if (write(fd, buf.data(), buf.size()) != (ssize_t)buf.size()) return 1;
            buf.clear();
        }
    }
    if (crash) std::abort();                               // mô phỏng crash: phần còn trong buf bị mất
    if (!buf.empty() && write(fd, buf.data(), buf.size()) != (ssize_t)buf.size()) return 1;
    auto t1 = std::chrono::steady_clock::now();
    std::printf("%s: %d dong trong %.1f ms\n", argv[1], N, std::chrono::duration<double, std::milli>(t1 - t0).count());
    close(fd);
}
