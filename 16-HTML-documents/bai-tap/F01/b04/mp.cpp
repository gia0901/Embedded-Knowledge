// mp.cpp: mẫu "message passing" giữa hai nhân, chỉ để đọc asm (không có main).
// Build: g++ -O2 -std=c++20 -c mp.cpp && objdump -d --no-show-raw-insn -C mp.o
#include <atomic>
#include <cstdint>

struct Frame16 { uint8_t b[16]; };
Frame16 slot;                         // dữ liệu
std::atomic<int> ready{0};            // cờ

void publish_relaxed(const Frame16& f) { slot = f; ready.store(1, std::memory_order_relaxed); }
void publish_release(const Frame16& f) { slot = f; ready.store(1, std::memory_order_release); }
bool consume_acquire(Frame16& out) {
    if (ready.load(std::memory_order_acquire) == 0) return false;
    out = slot; return true;
}
