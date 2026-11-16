// uart_rx: gateway đọc frame từ cổng serial, tách khung kiểu --len hoặc --cobs, kiểm CRC và seq.
// Build: g++ -O2 -std=c++20 -Wall -o uart_rx uart_rx.cpp
// Chạy:  ./uart_rx TTY --len|--cobs      (thoát khi im lặng 1 s sau byte cuối, in thống kê)
#include <poll.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include "../b04/frame.h"
#include "uart.h"

static long ok = 0, bad_crc = 0, bad_len = 0, bad_cobs = 0, max_seq = 0;

static void handle(const uint8_t* b, size_t n) {          // b = [frame 16][crc 2]
    if (n != kFrame + 2) { bad_len++; return; }
    uint16_t c = crc16(b, kFrame);
    Frame f;
    if (c != (uint16_t(b[kFrame]) << 8 | b[kFrame + 1]) || !get_frame(b, f)) { bad_crc++; return; }
    ok++;
    if (f.seq > (uint32_t)max_seq) max_seq = f.seq;
}

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s tty --len|--cobs\n", argv[0]); return 1; }
    bool cobs = std::string(argv[2]) == "--cobs";
    int fd = open_raw(argv[1]);
    if (fd < 0) return 1;
    std::vector<uint8_t> acc, dec;
    uint8_t buf[4096]; long bytes = 0; bool started = false;
    for (;;) {
        pollfd p{fd, POLLIN, 0};
        if (poll(&p, 1, started ? 1000 : -1) <= 0) break;
        ssize_t r = read(fd, buf, sizeof buf);
        if (r <= 0) break;
        started = true; bytes += r;
        for (ssize_t i = 0; i < r; i++) {
            uint8_t x = buf[i];
            if (cobs) {
                if (x != 0) { acc.push_back(x); continue; }
                if (acc.empty()) continue;                // 0x00 liền nhau: khung rỗng, bỏ qua
                if (cobs_decode(acc, dec)) handle(dec.data(), dec.size()); else bad_cobs++;
                acc.clear();
            } else {
                acc.push_back(x);
                if (acc.size() == 1u + acc[0]) {           // đủ [len][len byte]
                    handle(acc.data() + 1, acc[0]);
                    acc.clear();
                }
            }
        }
    }
    printf("rx: %ld byte, frame dung %ld, sai crc %ld, sai do dai %ld, cobs hong %ld, seq lon nhat %ld\n",
           bytes, ok, bad_crc, bad_len, bad_cobs, max_seq);
}
