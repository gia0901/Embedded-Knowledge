// canbits: dựng chuỗi bit của một khung dữ liệu CAN 2.0A (ID 11 bit) theo đặc tả, đếm bit nhồi,
// và tính tải bus cho một bảng tin. Kernel máy lab không có CAN nên bài này tính, không đo trên bus.
// Build: g++ -O2 -std=c++20 -Wall -o canbits canbits.cpp
// Chạy:  ./canbits ID DATAHEX            ví dụ ./canbits 0x123 1122334455667788
//        ./canbits --load BITRATE FILE   FILE: mỗi dòng "ID số_byte chu_kỳ_ms"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct Bits { int total, stuffed; std::vector<int> wire; };

static Bits build(unsigned id, const std::vector<unsigned char>& d) {
    std::vector<int> b;                                   // phần chịu nhồi bit: SOF tới hết CRC
    auto put = [&](unsigned v, int n) { for (int i = n - 1; i >= 0; i--) b.push_back((v >> i) & 1); };
    put(0, 1);                                            // SOF (dominant = 0)
    put(id, 11);                                          // ID
    put(0, 1); put(0, 1); put(0, 1);                      // RTR, IDE, r0
    put(d.size(), 4);                                     // DLC
    for (unsigned char x : d) put(x, 8);                  // dữ liệu
    unsigned crc = 0;                                     // CRC-15, đa thức 0x4599, trên SOF..dữ liệu
    for (int bit : b) {
        int nxt = bit ^ ((crc >> 14) & 1);
        crc = (crc << 1) & 0x7FFF;
        if (nxt) crc ^= 0x4599;
    }
    put(crc, 15);
    Bits r{0, 0, {}};
    int run = 0, last = -1;                               // nhồi: sau 5 bit giống nhau chèn 1 bit ngược
    for (int bit : b) {
        r.wire.push_back(bit);
        run = (bit == last) ? run + 1 : 1; last = bit;
        if (run == 5) { r.wire.push_back(!bit); r.stuffed++; last = !bit; run = 1; }
    }
    int tail = 1 + 2 + 7 + 3;                             // CRC delim, ACK slot + delim, EOF, khoảng nghỉ (IFS)
    r.total = r.wire.size() + tail;
    return r;
}

static int worst(int n) { return 8 * n + 47 + (34 + 8 * n - 1) / 4; }   // trường hợp nhồi xấu nhất

int main(int argc, char** argv) {
    if (argc >= 4 && std::string(argv[1]) == "--load") {
        double rate = atof(argv[2]); std::ifstream in(argv[3]); std::string line;
        double load = 0, wload = 0;
        printf("  ID    byte  chu ky  bit(du lieu 0x55)  bit(xau nhat)  tai(xau nhat)\n");
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream s(line); std::string ids; int n; double ms;
            s >> ids >> n >> ms;
            unsigned id = strtoul(ids.c_str(), nullptr, 0);
            Bits b = build(id, std::vector<unsigned char>(n, 0x55));
            double l = b.total / rate / (ms / 1000), wl = worst(n) / rate / (ms / 1000);
            load += l; wload += wl;
            printf("  0x%03X  %d    %4.0f ms  %3d               %3d            %5.2f%%\n", id, n, ms, b.total, worst(n), 100 * wl);
        }
        printf("tong tai o %.0f kbit/s: %.1f%% (du lieu 0x55), %.1f%% (xau nhat)\n", rate / 1000, 100 * load, 100 * wload);
        return 0;
    }
    if (argc < 3) { fprintf(stderr, "dung: %s ID DATAHEX | --load BITRATE FILE\n", argv[0]); return 1; }
    unsigned id = strtoul(argv[1], nullptr, 0);
    std::vector<unsigned char> d;
    for (size_t i = 0; i + 1 < strlen(argv[2]); i += 2) d.push_back(strtoul(std::string(argv[2] + i, 2).c_str(), nullptr, 16));
    Bits b = build(id, d);
    printf("ID 0x%03X, %zu byte: %d bit tren day (%d bit nhoi), xau nhat %d bit\n", id, d.size(), b.total, b.stuffed, worst(d.size()));
    printf("  o 500 kbit/s: %.1f us; o 125 kbit/s: %.1f us\n", b.total / 0.5, b.total / 0.125);
    printf("  bit truoc ACK: ");
    for (int x : b.wire) putchar('0' + x);
    putchar('\n');
}
