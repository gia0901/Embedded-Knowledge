// readbatch: đọc 4 triệu frame (64 MB) từ file bằng read(), mỗi lần 16 byte (một frame) hoặc cả lô.
// File nằm sẵn trong page cache sau lần đọc đầu, nên đây là giá của syscall và copy, không phải giá của đĩa.
// Lời giải Lab 6.1. Build: g++ -O2 -std=c++20 -I.. -o readbatch readbatch.cpp   Chạy: ./readbatch
#include "bench.h"
#include "frame.h"
#include <fcntl.h>
#include <unistd.h>

int main() {
    const size_t n = 4'000'000;
    {   auto buf = make_frames(n, 64);                      // tạo file frames.bin 64 MB
        int fd = open("frames.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (write(fd, buf.data(), buf.size()) != (ssize_t)buf.size()) { perror("write"); return 1; }
        close(fd); }
    long sums[4] = {};
    int k = 0;
    for (size_t chunk : {16ul, 256ul, 4096ul, 65536ul}) {
        std::vector<uint8_t> b(chunk);
        char name[40]; std::snprintf(name, sizeof name, "read %6zu B/lan", chunk);
        bench_ns(name, n, 3, [&] {
            int fd = open("frames.bin", O_RDONLY);
            long sum = 0; Frame f; ssize_t got;
            while ((got = read(fd, b.data(), chunk)) > 0)
                for (ssize_t i = 0; i + (ssize_t)kWire <= got; i += kWire)
                    if (parse(&b[i], f) && f.type == TEMP) sum += f.value;
            close(fd);
            sums[k] = sum;
        });
        ++k;
    }
    std::printf("tong nhiet do: %ld %ld %ld %ld\n", sums[0], sums[1], sums[2], sums[3]);
}
