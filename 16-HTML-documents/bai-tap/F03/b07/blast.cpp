// blast: client thử server echo. Mở C kết nối, mỗi kết nối gửi M tin S byte, rồi đọc lại và kiểm.
// Build: g++ -O2 -std=c++20 -Wall -o blast blast.cpp
// Chạy:  ./blast IP PORT [tuỳ chọn]
//   --conns C     số kết nối (mặc định 1)        --msgs M   số tin mỗi kết nối (mặc định 1000)
//   --size S      byte payload mỗi tin (mặc định 100)
//   --chunk K     cắt dòng byte thành các write() K byte (0 = một write cho tất cả, mặc định)
//   --gap-us U    ngủ U µs giữa hai write()
//   --noread      không đọc trả lời (thử backpressure); chỉ ghi
//   --hold S      với --noread: giữ kết nối thêm S giây rồi mới đóng
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "wire.h"

static double now_ms() { timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "dung: %s ip port [--conns C --msgs M --size S --chunk K --gap-us U --noread --hold S]\n", argv[0]); return 1; }
    long conns = 1, msgs = 1000, size = 100, chunk = 0, gap = 0, hold = 0; bool noread = false;
    for (int i = 3; i < argc; i++) {
        std::string a = argv[i];
        auto val = [&] { return i + 1 < argc ? atol(argv[++i]) : 0L; };
        if (a == "--conns") conns = val(); else if (a == "--msgs") msgs = val(); else if (a == "--size") size = val();
        else if (a == "--chunk") chunk = val(); else if (a == "--gap-us") gap = val(); else if (a == "--noread") noread = true;
        else if (a == "--hold") hold = val();
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    sockaddr_in dst{}; dst.sin_family = AF_INET; dst.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &dst.sin_addr);

    // Dòng byte cần gửi trên mỗi kết nối: M tin, tin thứ i có payload toàn byte ('a' + i % 26).
    std::string stream;
    for (long i = 0; i < msgs; i++) { std::string p(size, char('a' + i % 26)); put_msg(stream, p.data(), p.size()); }

    double t0 = now_ms();
    std::vector<int> fds;
    for (long c = 0; c < conns; c++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0), one = 1;
        if (connect(fd, (sockaddr*)&dst, sizeof dst) < 0) { printf("connect thu %ld: %s\n", c + 1, strerror(errno)); return 1; }
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);   // mỗi write() thành gói riêng
        fds.push_back(fd);
    }
    long writes = 0;
    for (int fd : fds) {
        size_t off = 0;
        while (off < stream.size()) {
            size_t n = chunk ? std::min<size_t>(chunk, stream.size() - off) : stream.size() - off;
            ssize_t w = write(fd, stream.data() + off, n);
            if (w < 0) { perror("write"); return 1; }
            off += w; writes++;
            if (gap) usleep(gap);
        }
    }
    printf("da gui %ld tin x %ld ket noi = %zu byte moi ket noi, %ld write()\n", msgs, conns, stream.size(), writes);
    fflush(stdout);
    if (noread) { sleep(hold); for (int fd : fds) close(fd); return 0; }

    long ok = 0, bad = 0;
    for (int fd : fds) {
        Decoder d; std::string m; long got = 0; char buf[65536];
        while (got < msgs) {
            ssize_t r = read(fd, buf, sizeof buf);
            if (r <= 0) { printf("ket noi dong som sau %ld tin\n", got); break; }
            d.feed(buf, r);
            while (got < msgs && d.next(m) == 1) {
                if (m.size() == size_t(size) && m.find_first_not_of(char('a' + got % 26)) == std::string::npos) ok++; else bad++;
                got++;
            }
        }
        close(fd);
    }
    printf("nhan lai %ld/%ld tin dung, %ld sai, %.1f ms\n", ok, msgs * conns, bad, now_ms() - t0);
    return bad || ok != msgs * conns;
}
