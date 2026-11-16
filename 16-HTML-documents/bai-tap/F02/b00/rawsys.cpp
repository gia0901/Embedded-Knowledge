// rawsys: gọi write() bằng lệnh syscall trực tiếp (không qua glibc) để thấy quy ước của kernel:
// kernel trả -errno trong rax; glibc đổi thành -1 và đặt errno. Lời giải Lab 0.4. Chỉ cho x86-64.
// Build: g++ -O2 -std=c++20 -o rawsys rawsys.cpp
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/syscall.h>
#include <unistd.h>

static long raw_write(long fd, const void* buf, long len) {
    long ret;
    // ABI x86-64: rax = số hiệu syscall, rdi/rsi/rdx = tham số 1..3; lệnh syscall ghi đè rcx và r11
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(SYS_write), "D"(fd), "S"(buf), "d"(len)
                 : "rcx", "r11", "memory");
    return ret;
}

int main() {
    const char msg[] = "xin chao tu lenh syscall\n";
    long r1 = raw_write(1, msg, sizeof msg - 1);
    std::printf("raw_write(fd=1)  -> %ld\n", r1);

    long r2 = raw_write(42, msg, sizeof msg - 1);       // fd 42 chưa mở
    std::printf("raw_write(fd=42) -> %ld  (= -EBADF, EBADF=%d)\n", r2, EBADF);

    errno = 0;
    long r3 = write(42, msg, sizeof msg - 1);           // qua glibc
    std::printf("write(fd=42)     -> %ld, errno=%d (%s)\n", r3, errno, std::strerror(errno));
}
