// litmus: phép thử "store buffering" (SB). Với mỗi i, hai thread làm:
//   T0: X[i] = 1; r0[i] = Y[i];        T1: Y[i] = 1; r1[i] = X[i];
// Theo thứ tự trong code, ít nhất một thread phải đọc được 1. Kết quả r0 == 0 && r1 == 0 "không thể có",
// nhưng x86 (TSO) cho phép: lệnh ghi còn nằm trong store buffer khi lệnh đọc phía sau đã chạy xong.
// Hai thread chạy tự do trên N cặp biến khác nhau; ở những i mà hai thread chạy trùng lúc, hiện tượng lộ ra.
// Lời giải Lab 4.3. Build: g++ -O2 -std=c++20 -pthread -o litmus litmus.cpp
// Chạy: ./litmus [relaxed|seq_cst] [N=1000000] [số vòng=20]
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

int main(int argc, char** argv) {
    const bool sc = argc > 1 && std::strcmp(argv[1], "seq_cst") == 0;
    const size_t N = argc > 2 ? std::atol(argv[2]) : 1'000'000;
    const int rounds = argc > 3 ? std::atoi(argv[3]) : 20;
    std::vector<int> X(N), Y(N), R0(N), R1(N);
    std::atomic<int> ready{0};
    long both_zero = 0;
    auto side = [&](std::vector<int>& mine, std::vector<int>& other, std::vector<int>& r) {
        ready.fetch_add(1);
        while (ready.load() < 2) {}                                // xuất phát cùng lúc
        for (size_t i = 0; i < N; ++i) {
            if (sc) {
                std::atomic_ref<int>(mine[i]).store(1, std::memory_order_seq_cst);
                r[i] = std::atomic_ref<int>(other[i]).load(std::memory_order_seq_cst);
            } else {
                std::atomic_ref<int>(mine[i]).store(1, std::memory_order_relaxed);
                r[i] = std::atomic_ref<int>(other[i]).load(std::memory_order_relaxed);
            }
        }
    };
    for (int k = 0; k < rounds; ++k) {
        std::fill(X.begin(), X.end(), 0); std::fill(Y.begin(), Y.end(), 0);
        ready = 0;
        std::thread t0(side, std::ref(X), std::ref(Y), std::ref(R0));
        std::thread t1(side, std::ref(Y), std::ref(X), std::ref(R1));
        t0.join(); t1.join();
        for (size_t i = 0; i < N; ++i) both_zero += (R0[i] == 0 && R1[i] == 0);
    }
    std::printf("%s: %ld / %zu lan thay r0 == 0 && r1 == 0\n", sc ? "seq_cst" : "relaxed", both_zero, N * rounds);
}
