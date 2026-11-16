// guard: dùng quyền trang (mprotect) để bắt lỗi truy cập bộ nhớ ngay tại lệnh sai, giống MPU trên Cortex-M.
// Đặt buffer frame sát cuối một trang, trang ngay sau là "trang gác" PROT_NONE. Đọc lố một byte là SIGSEGV.
// Handler in địa chỉ lỗi (si_addr) và loại lỗi (si_code), rồi nhảy về để chạy tiếp ca sau.
// Lời giải Lab 5.4. Build: g++ -O2 -std=c++20 -I.. -o guard guard.cpp      Chạy: ./guard  hoặc  ./guard noguard
#include "frame.h"
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>

static sigjmp_buf env;
static void on_segv(int, siginfo_t* si, void*) {
    const char* code = si->si_code == SEGV_MAPERR ? "SEGV_MAPERR (khong co anh xa)" :
                       si->si_code == SEGV_ACCERR ? "SEGV_ACCERR (sai quyen)" : "khac";
    std::printf("    -> SIGSEGV tai %p, %s\n", si->si_addr, code);   // printf trong handler: chỉ để học
    siglongjmp(env, 1);
}

template <class F> static void try_case(const char* name, F f) {
    std::printf("%s\n", name);
    if (sigsetjmp(env, 1) == 0) { f(); std::printf("    -> chay xong, khong loi\n"); }
}

int main(int argc, char** argv) {
    const bool noguard = argc > 1;                               // ./guard noguard: bỏ trang gác (bản đối chứng)
    struct sigaction sa{};
    sa.sa_sigaction = on_segv; sa.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, nullptr);

    const long pg = sysconf(_SC_PAGESIZE);
    auto* base = (uint8_t*)mmap(nullptr, 3 * pg, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    uint8_t* guard = base + 2 * pg;
    if (!noguard) mprotect(guard, pg, PROT_NONE);                // trang gác
    // 10 frame (160 B) đặt sát trang gác; frame cuối bị cắt còn 8 byte (gói tin nhận thiếu)
    auto src = make_frames(10, 4);
    uint8_t* buf = guard - (9 * kWire + 8);
    std::memcpy(buf, src.data(), 9 * kWire + 8);
    std::printf("trang %ld B, buffer %p .. %p, trang gac bat dau %p\n", pg, (void*)buf, (void*)(guard - 1), (void*)guard);

    volatile long sink = 0;
    try_case("1. parse 9 frame day du", [&] { Frame f; for (int i = 0; i < 9; ++i) { sink += parse(buf + i * kWire, f); sink += f.value; } });
    try_case("2. parse ca frame thu 10 (bi cat)", [&] {
        Frame f; for (int i = 0; i < 10; ++i) { sink += parse(buf + i * kWire, f); sink += f.value; }
        std::printf("    frame 10: parse tra ve %d, value = %d\n", parse(buf + 9 * kWire, f), f.value);
    });
    try_case("3. doc con tro null + 8", [&] { sink += *(volatile long*)8; });
    mprotect(base, pg, PROT_READ);                               // trang đầu: chỉ đọc
    try_case("4. doc trang chi-doc", [&] { sink += *(volatile uint8_t*)base; });
    try_case("5. ghi trang chi-doc", [&] { *(volatile uint8_t*)base = 1; });
}
