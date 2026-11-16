// layout: một tiến trình framestat nằm ở đâu trong không gian địa chỉ ảo.
// In địa chỉ của code, biến toàn cục, heap nhỏ, buffer lớn, stack; rồi in /proc/self/maps để đối chiếu.
// Lời giải Lab 5.1. Build: g++ -O2 -std=c++20 -I.. -o layout layout.cpp     Chạy: ./layout
#include "frame.h"
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

int g_counter = 0;                       // .bss
const char g_name[] = "framestat";       // .rodata

int main() {
    auto small = std::make_unique<Frame[]>(4);              // 48 B: heap (brk)
    auto buf = make_frames(4'000'000, 64);                   // 64 MB: mmap riêng
    int local = 0;
    std::printf("%-26s %p\n", "code   main", (void*)&main);
    std::printf("%-26s %p\n", "rodata g_name", (void*)g_name);
    std::printf("%-26s %p\n", "bss    g_counter", (void*)&g_counter);
    std::printf("%-26s %p\n", "heap   4 Frame (48 B)", (void*)small.get());
    std::printf("%-26s %p\n", "mmap   buffer 64 MB", (void*)buf.data());
    std::printf("%-26s %p\n", "stack  local", (void*)&local);
    std::printf("\n/proc/self/maps:\n");
    std::ifstream m("/proc/self/maps");
    for (std::string line; std::getline(m, line);) std::printf("%s\n", line.c_str());
    std::ifstream st("/proc/self/status");                   // tổng vùng ảo và phần đang có RAM
    for (std::string line; std::getline(st, line);)
        if (line.rfind("VmSize", 0) == 0 || line.rfind("VmRSS", 0) == 0) std::printf("%s\n", line.c_str());
}
