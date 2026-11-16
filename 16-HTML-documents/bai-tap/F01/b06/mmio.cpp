// mmio: thanh ghi thiết bị là địa chỉ bộ nhớ (memory-mapped I/O). Chỉ để đọc asm (không có main, không chạy).
// So sánh truy cập qua con trỏ thường và qua con trỏ volatile tới cùng một "thanh ghi" UART giả định.
// Build: aarch64-linux-gnu-g++ -O2 -std=c++20 -c mmio.cpp && aarch64-linux-gnu-objdump -d --no-show-raw-insn -C mmio.o
//        (hoặc g++ cho x86-64 để so sánh). Lời giải Lab 6.4.
#include <cstdint>

constexpr uintptr_t UART = 0x09000000;          // địa chỉ ví dụ (UART PL011 trên board ảo QEMU "virt")
#define REG(off)  (*(uint32_t*)(UART + (off)))
#define VREG(off) (*(volatile uint32_t*)(UART + (off)))
constexpr uintptr_t DR = 0x00, FR = 0x18;      // data register, flag register
constexpr uint32_t TXFF = 1u << 5;              // bit "FIFO phát đầy"

void send2_plain(uint8_t a, uint8_t b) {
    while (REG(FR) & TXFF) {}                   // chờ FIFO còn chỗ
    REG(DR) = a;
    while (REG(FR) & TXFF) {}
    REG(DR) = b;
}

void send2_volatile(uint8_t a, uint8_t b) {
    while (VREG(FR) & TXFF) {}
    VREG(DR) = a;
    while (VREG(FR) & TXFF) {}
    VREG(DR) = b;
}

// DMA: CPU ghi mô tả (descriptor) vào RAM, rồi gõ "chuông" (doorbell) là một thanh ghi để thiết bị đọc descriptor.
struct Desc { uint64_t addr; uint32_t len; uint32_t flags; };
Desc ring[256];                                  // nằm trong RAM, thiết bị đọc bằng DMA
constexpr uintptr_t DOORBELL = 0x40;

void post_nofence(uint32_t i, uint64_t buf, uint32_t len) {
    ring[i] = {buf, len, 1};                     // 1 = thiết bị được phép dùng
    VREG(DOORBELL) = i;
}

void post_fence(uint32_t i, uint64_t buf, uint32_t len) {
    ring[i] = {buf, len, 1};
    __atomic_thread_fence(__ATOMIC_RELEASE);     // C++: std::atomic_thread_fence(std::memory_order_release)
    VREG(DOORBELL) = i;
}

void post_dsb(uint32_t i, uint64_t buf, uint32_t len) {
    ring[i] = {buf, len, 1};
#if defined(__aarch64__)
    asm volatile("dsb st" ::: "memory");        // như wmb() của Linux trên arm64: chờ mọi lệnh ghi hoàn tất
#else
    asm volatile("sfence" ::: "memory");        // wmb() của Linux trên x86-64
#endif
    VREG(DOORBELL) = i;
}
