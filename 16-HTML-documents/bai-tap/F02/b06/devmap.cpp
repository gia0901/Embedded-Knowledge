// devmap: chèn N device id khác nhau vào ba kiểu bảng đếm, in RSS tăng thêm và số byte mỗi device.
// Đo cái giá thật của "bảng đếm theo device" trong telemd khi số device không bị giới hạn. Lời giải Lab 6.3.
// Build: g++ -O2 -std=c++20 -o devmap devmap.cpp
// Chạy:  ./devmap map|unordered|vector <N>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static long rss_kb() {
    std::ifstream f("/proc/self/status");
    for (std::string l; std::getline(f, l);)
        if (l.rfind("VmRSS:", 0) == 0) return std::atol(l.c_str() + 6);
    return -1;
}

int main(int argc, char** argv) {
    const char* kind = argv[1];
    long n = std::atol(argv[2]);
    long before = rss_kb();
    std::map<int, long> m;
    std::unordered_map<int, long> u;
    std::vector<std::pair<int, long>> v;
    for (long i = 0; i < n; ++i) {
        int dev = static_cast<int>((i * 2654435761L) & 0x7fffffff);   // id rải rác như id thiết bị thật
        if (!std::strcmp(kind, "map")) ++m[dev];
        else if (!std::strcmp(kind, "unordered")) ++u[dev];
        else v.emplace_back(dev, 1);
    }
    if (!std::strcmp(kind, "vector")) std::sort(v.begin(), v.end());   // vector đã sắp: tra bằng binary search
    long after = rss_kb();
    std::printf("%-9s %8ld device: RSS +%7ld kB = %5.1f byte/device\n", kind, n, after - before, (after - before) * 1024.0 / n);
}
