// cpi: đo số chu kỳ mỗi lệnh (CPI) của vài loại lệnh, bằng vòng lặp assembly biết trước số lệnh.
// Máy lab là VM nên perf không đọc được bộ đếm phần cứng. Mẹo: chuỗi `add reg,reg` phụ thuộc nhau chạy
// 1 chu kỳ mỗi lệnh, nên đo nó cho ra tần số thật của nhân; kiểm chéo bằng imul (3 chu kỳ).
// Chuỗi `add $1` được in riêng: một số CPU mới gộp nó ngay ở khâu rename nên nhanh hơn 1 chu kỳ.
// Lời giải Lab 0.2. Build: g++ -O2 -std=c++20 -o cpi cpi.cpp      Chạy: ./cpi
#include <chrono>
#include <cstdint>
#include <cstdio>

constexpr long ITERS = 100'000'000;                 // mỗi vòng có 8 lệnh "việc" + 2 lệnh vòng lặp

template <class F> double ns_per_op(F f, double ops = 8.0) {
    f(ITERS / 10);                                  // chạy nháp
    auto a = std::chrono::steady_clock::now();
    f(ITERS);
    auto b = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(b - a).count() / (ITERS * ops);
}

int main() {
    // 1. 8 lệnh add nối tiếp nhau trên cùng một thanh ghi: lệnh sau chờ kết quả lệnh trước
    uint64_t one = 1;
    double add_dep = ns_per_op([one](long n) {
        uint64_t x = 0;
        asm volatile("1:\n\t"
                     "add %2,%0\n\tadd %2,%0\n\tadd %2,%0\n\tadd %2,%0\n\t"
                     "add %2,%0\n\tadd %2,%0\n\tadd %2,%0\n\tadd %2,%0\n\t"
                     "dec %1\n\tjnz 1b" : "+r"(x), "+r"(n) : "r"(one));
    });
    // 1b. cùng chuỗi đó nhưng cộng hằng số $1
    double add_imm = ns_per_op([](long n) {
        uint64_t x = 0;
        asm volatile("1:\n\t"
                     "add $1,%0\n\tadd $1,%0\n\tadd $1,%0\n\tadd $1,%0\n\t"
                     "add $1,%0\n\tadd $1,%0\n\tadd $1,%0\n\tadd $1,%0\n\t"
                     "dec %1\n\tjnz 1b" : "+r"(x), "+r"(n));
    });
    // 2. 8 lệnh add chia cho 4 thanh ghi độc lập: CPU chạy song song được
    double add_ind = ns_per_op([one](long n) {
        uint64_t a = 0, b = 0, c = 0, d = 0;
        asm volatile("1:\n\t"
                     "add %5,%0\n\tadd %5,%1\n\tadd %5,%2\n\tadd %5,%3\n\t"
                     "add %5,%0\n\tadd %5,%1\n\tadd %5,%2\n\tadd %5,%3\n\t"
                     "dec %4\n\tjnz 1b" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(n) : "r"(one));
    });
    // 3. 8 lệnh nhân phụ thuộc nhau
    double mul_dep = ns_per_op([](long n) {
        uint64_t x = 3;
        asm volatile("1:\n\t"
                     "imul %0,%0\n\timul %0,%0\n\timul %0,%0\n\timul %0,%0\n\t"
                     "imul %0,%0\n\timul %0,%0\n\timul %0,%0\n\timul %0,%0\n\t"
                     "dec %1\n\tjnz 1b" : "+r"(x), "+r"(n));
    });
    // 4. 8 lệnh nạp phụ thuộc: p = *p, p trỏ vào chính nó (luôn trúng L1)
    double load_dep = ns_per_op([](long n) {
        void* self[1]; self[0] = self;
        void* p = self;
        asm volatile("1:\n\t"
                     "mov (%0),%0\n\tmov (%0),%0\n\tmov (%0),%0\n\tmov (%0),%0\n\t"
                     "mov (%0),%0\n\tmov (%0),%0\n\tmov (%0),%0\n\tmov (%0),%0\n\t"
                     "dec %1\n\tjnz 1b" : "+r"(p), "+r"(n) : : "memory");
    });
    // 5. 3 lệnh chia 64 bit phụ thuộc nhau mỗi vòng (kèm xor/or phụ, tính theo số lệnh div)
    double div_dep = ns_per_op([](long n) {
        uint64_t x = ~0ull, d = 3;
        asm volatile("1:\n\t"
                     "xor %%edx,%%edx\n\tdiv %2\n\tor $1,%%rax\n\t"
                     "xor %%edx,%%edx\n\tdiv %2\n\tor $1,%%rax\n\t"
                     "xor %%edx,%%edx\n\tdiv %2\n\t"
                     "dec %1\n\tjnz 1b" : "+a"(x), "+r"(n) : "r"(d) : "rdx");
    }, 3.0);
    double ghz = 1.0 / add_dep;                     // add reg,reg phụ thuộc = 1 chu kỳ
    std::printf("tan so nhan uoc tinh: %.2f GHz (add reg,reg)  kiem cheo: %.2f GHz (imul = 3 chu ky)\n",
                ghz, 3.0 / mul_dep);
    auto row = [&](const char* name, double ns) {
        std::printf("%-26s %6.3f ns/lenh  CPI %5.2f  IPC %5.2f\n", name, ns, ns * ghz, 1 / (ns * ghz));
    };
    row("add phu thuoc", add_dep);
    row("add $1 phu thuoc", add_imm);
    row("add doc lap (4 chuoi)", add_ind);
    row("imul phu thuoc", mul_dep);
    row("load phu thuoc (L1)", load_dep);
    row("div 64 bit phu thuoc", div_dep);
}
