// uart_tx: thiết bị gửi frame cảm biến 16 byte (Bài 4) qua cổng serial, đóng khung kiểu --len hoặc --cobs.
// Build: g++ -O2 -std=c++20 -Wall -o uart_tx uart_tx.cpp
// Chạy:  ./uart_tx TTY SO_FRAME --len|--cobs [--cut N] [--lead0]
//   --len     [độ dài 1 byte][frame 16][crc 2]
//   --cobs    COBS([frame 16][crc 2]) rồi 0x00
//   --cut N   mỗi frame thứ N chỉ gửi 7 byte đầu (thiết bị reset giữa chừng)
//   --lead0   (cobs) gửi thêm 0x00 trước mỗi khung
#include <cstdlib>
#include <cstring>
#include <string>
#include "../b04/frame.h"
#include "uart.h"

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s tty so_frame --len|--cobs [--cut N] [--lead0]\n", argv[0]); return 1; }
    long n = atol(argv[2]), cut = 0; bool cobs = false, lead0 = false;
    for (int i = 3; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--cobs") cobs = true; else if (a == "--len") cobs = false; else if (a == "--lead0") lead0 = true;
        else if (a == "--cut" && i + 1 < argc) cut = atol(argv[++i]);
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    int fd = open_raw(argv[1]);
    if (fd < 0) return 1;
    long cuts = 0, bytes = 0;
    for (long i = 1; i <= n; i++) {
        uint8_t body[kFrame + 2];
        Frame f{7, uint32_t(i), 1, 0, int16_t(2500 + i % 100), uint32_t(i * 3)};
        put_frame(body, f);
        uint16_t c = crc16(body, kFrame);
        body[kFrame] = c >> 8; body[kFrame + 1] = c & 0xFF;
        std::vector<uint8_t> wire;
        if (cobs) {
            if (lead0) wire.push_back(0);
            auto e = cobs_encode(body, sizeof body);
            wire.insert(wire.end(), e.begin(), e.end());
            wire.push_back(0);
        } else {
            wire.push_back(sizeof body);
            wire.insert(wire.end(), body, body + sizeof body);
        }
        size_t len = wire.size();
        if (cut && i % cut == 0) { len = 7; cuts++; }
        for (size_t off = 0; off < len;) {
            ssize_t w = write(fd, wire.data() + off, len - off);
            if (w < 0) { perror("write"); return 1; }
            off += w;
        }
        bytes += len;
    }
    tcdrain(fd);
    printf("tx: %ld frame, %ld bi cat, %ld byte\n", n, cuts, bytes);
}
