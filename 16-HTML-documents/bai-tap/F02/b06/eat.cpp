// eat: xin bộ nhớ từng bước bằng malloc, có hoặc không chạm vào, in tiến độ.
// Dùng để thấy overcommit (xin được nhiều hơn RAM nếu không chạm) và OOM killer (khi chạm thật).
// Lời giải Lab 6.2. Build: g++ -O2 -std=c++20 -o eat eat.cpp
// Chạy:  ./eat <MB mỗi bước> <số bước> touch|notouch [số giây giữ bộ nhớ ở cuối, mặc định 1]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

int main(int argc, char** argv) {
    size_t mb = std::atol(argv[1]);
    int steps = std::atoi(argv[2]);
    bool touch = std::strcmp(argv[3], "touch") == 0;
    size_t total = 0;
    for (int i = 0; i < steps; ++i) {
        char* p = static_cast<char*>(std::malloc(mb << 20));
        if (!p) { std::printf("buoc %d: malloc(%zu MB) tra NULL sau khi da xin %zu MB\n", i, mb, total); return 1; }
        if (touch) std::memset(p, 1, mb << 20);           // chạm thật: kernel phải cấp page RAM
        total += mb;
        std::printf("da xin %zu MB%s\n", total, touch ? " (da cham)" : "");
        std::fflush(stdout);                              // để thấy dòng cuối trước khi bị kill
    }
    int hold = argc > 4 ? std::atoi(argv[4]) : 1;
    std::printf("xong, giu %d giay\n", hold);
    std::fflush(stdout);
    sleep(hold);
}
