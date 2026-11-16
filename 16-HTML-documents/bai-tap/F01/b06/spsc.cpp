// spsc: hàng đợi một người ghi, một người đọc, giữa thread "NIC" (ghi frame vào ring) và thread "worker"
// (parse + thống kê). Đây là mô hình của RX ring giữa card mạng và driver, hay giữa hai thread trong DPDK.
//   mutex   : std::mutex + std::deque (cách ngây thơ)
//   ring    : vòng tròn, head và tail atomic nằm sát nhau
//   pad     : head và tail mỗi cái một cache line
//   cache   : thêm bản sao cục bộ của chỉ số bên kia, chỉ đọc lại khi tưởng ring đầy/rỗng
//   batch   : thêm gom lô: công bố chỉ số sau mỗi 32 frame thay vì mỗi frame
// Lời giải Lab 6.2. Build: g++ -O2 -std=c++20 -I.. -pthread -o spsc spsc.cpp     Chạy: ./spsc
#include "bench.h"
#include "frame.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <pthread.h>
#include <thread>

constexpr size_t kCap = 4096;                         // số slot, lũy thừa 2
struct Slot { uint8_t b[kWire]; };

static void pin(int cpu) { cpu_set_t s; CPU_ZERO(&s); CPU_SET(cpu, &s); pthread_setaffinity_np(pthread_self(), sizeof s, &s); }

struct RingNaive {                                    // head, tail sát nhau
    std::atomic<size_t> head{0}, tail{0};
    Slot slot[kCap];
};
struct RingPad {
    alignas(64) std::atomic<size_t> head{0};         // worker ghi
    alignas(64) std::atomic<size_t> tail{0};         // NIC ghi
    alignas(64) Slot slot[kCap];
};

template <int Mode, class R> long run(R& r, const std::vector<uint8_t>& src, size_t n) {
    r.head = 0; r.tail = 0;
    long sum = 0;
    const size_t m = src.size() / kWire;
    std::thread nic([&] {
        pin(0);
        size_t t = 0, head_cache = 0;
        for (size_t i = 0; i < n; ++i) {
            if constexpr (Mode == 0) { while (t - r.head.load(std::memory_order_acquire) == kCap) {} }
            else { while (t - head_cache == kCap) head_cache = r.head.load(std::memory_order_acquire); }
            std::memcpy(r.slot[t & (kCap - 1)].b, &src[(i % m) * kWire], kWire);
            ++t;
            if (Mode < 2 || (t & 31) == 0 || i + 1 == n) r.tail.store(t, std::memory_order_release);
        }
    });
    pin(2);
    Frame f;
    size_t h = 0, tail_cache = 0;
    while (h < n) {
        if constexpr (Mode == 0) { while (h == r.tail.load(std::memory_order_acquire)) {} tail_cache = h + 1; }
        else { while (h == tail_cache) tail_cache = r.tail.load(std::memory_order_acquire); }
        const size_t end = Mode == 2 ? tail_cache : h + 1;    // batch: xử lý hết phần đã thấy
        for (; h < end; ++h) if (parse(r.slot[h & (kCap - 1)].b, f) && f.type == TEMP) sum += f.value;
        r.head.store(h, std::memory_order_release);           // trả slot cho NIC sau mỗi lần xử lý
    }
    nic.join();
    return sum;
}

int main() {
    const size_t n = 32'000'000;
    auto src = make_frames(1 << 16, 64);                      // 1 MB frame mẫu, NIC lặp lại
    long s[6] = {};
    bench_ns("1 thread, khong ring", n, 5, [&] {               // mốc: cùng việc, không chuyển dữ liệu giữa hai nhân
        long sum = 0; Frame f;
        for (size_t i = 0; i < n; ++i) if (parse(&src[(i % (1 << 16)) * kWire], f) && f.type == TEMP) sum += f.value;
        s[5] = sum; });
    {   std::deque<Slot> q; std::mutex mu;
        bench_ns("mutex + deque", n, 5, [&] {
            long sum = 0;
            std::thread nic([&] { pin(0); for (size_t i = 0; i < n; ++i) { Slot x; std::memcpy(x.b, &src[(i % (1 << 16)) * kWire], kWire);
                                    std::lock_guard g(mu); q.push_back(x); } });
            pin(2); Frame f; size_t got = 0;
            while (got < n) { Slot x; { std::lock_guard g(mu); if (q.empty()) continue; x = q.front(); q.pop_front(); }
                              ++got; if (parse(x.b, f) && f.type == TEMP) sum += f.value; }
            nic.join(); s[0] = sum; }); }
    auto naive = std::make_unique<RingNaive>();
    auto pad = std::make_unique<RingPad>();
    bench_ns("ring, head/tail sat",   n, 5, [&] { s[1] = run<0>(*naive, src, n); });
    bench_ns("ring, pad",             n, 5, [&] { s[2] = run<0>(*pad, src, n); });
    bench_ns("ring, pad + cache",     n, 5, [&] { s[3] = run<1>(*pad, src, n); });
    bench_ns("ring, pad+cache+batch", n, 5, [&] { s[4] = run<2>(*pad, src, n); });
    std::printf("tong nhiet do: %ld %ld %ld %ld %ld %ld\n", s[5], s[0], s[1], s[2], s[3], s[4]);
}
