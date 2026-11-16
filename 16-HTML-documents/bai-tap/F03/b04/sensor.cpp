// sensor: giả lập thiết bị gửi frame 16 byte qua UDP, có thể chèn lỗi để kiểm gw.
// Build: g++ -O2 -std=c++20 -Wall -o sensor sensor.cpp
// Chạy:  ./sensor IP PORT SO_FRAME [tuỳ chọn]
//   --rate PPS     số datagram mỗi giây (0 = nhanh nhất có thể, mặc định)
//   --batch K      K frame mỗi datagram (mặc định 1)
//   --dev D        dev_id (mặc định 7)
//   --connect      connect() rồi send(): nhận được lỗi ICMP như ECONNREFUSED
//   --drop N       bỏ (không gửi) mỗi frame thứ N      --dup N   gửi đôi mỗi frame thứ N
//   --swap N       đảo thứ tự frame thứ N với frame sau --bad N   hỏng magic mỗi frame thứ N
//   --restart-at I tại frame thứ I đặt lại seq về 0 (giả lập thiết bị khởi động lại)
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "frame.h"

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s ip port so_frame [--rate R --batch K --dev D --connect --drop N --dup N --swap N --bad N --restart-at I]\n", argv[0]); return 1; }
    long n = atol(argv[3]), rate = 0, drop = 0, dup = 0, swp = 0, bad = 0, restart = -1;
    int batch = 1, dev = 7; bool conn = false;
    for (int i = 4; i < argc; i++) {
        std::string a = argv[i];
        auto val = [&] { return i + 1 < argc ? atol(argv[++i]) : 0L; };
        if (a == "--rate") rate = val(); else if (a == "--batch") batch = val(); else if (a == "--dev") dev = val();
        else if (a == "--connect") conn = true; else if (a == "--drop") drop = val(); else if (a == "--dup") dup = val();
        else if (a == "--swap") swp = val(); else if (a == "--bad") bad = val(); else if (a == "--restart-at") restart = val();
        else { fprintf(stderr, "khong hieu %s\n", a.c_str()); return 1; }
    }
    sockaddr_in dst{}; dst.sin_family = AF_INET; dst.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &dst.sin_addr) != 1) { fprintf(stderr, "dia chi sai\n"); return 1; }
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (conn && connect(fd, (sockaddr*)&dst, sizeof dst) < 0) { perror("connect"); return 1; }

    // Dựng danh sách frame theo đúng thứ tự sẽ gửi, kèm số liệu "sự thật" để so với gw.
    std::vector<Frame> q; long n_drop = 0, n_dup = 0, n_swap = 0, n_bad = 0;
    uint32_t seq = 0;
    for (long i = 0; i < n; i++) {
        if (i == restart) seq = 0;
        Frame f{(uint16_t)dev, seq++, 1, 0, (int16_t)(2500 + i % 300), (uint32_t)(i * 7)};
        if (drop && i % drop == drop - 1) { n_drop++; continue; }
        if (bad && i % bad == bad - 1) { f.flags = 0xBD; n_bad++; }
        q.push_back(f);
        if (dup && i % dup == dup - 1) { q.push_back(f); n_dup++; }
    }
    if (swp) for (size_t i = swp - 1; i + 1 < q.size(); i += swp) { std::swap(q[i], q[i + 1]); n_swap++; i++; }

    std::vector<uint8_t> buf(kFrame * batch);
    timespec next; clock_gettime(CLOCK_MONOTONIC, &next);
    long dgrams = 0, errs = 0, first_err = -1; int last_errno = 0;
    timespec t0 = next;
    for (size_t i = 0; i < q.size(); i += batch) {
        size_t k = std::min<size_t>(batch, q.size() - i);
        for (size_t j = 0; j < k; j++) {
            put_frame(&buf[kFrame * j], q[i + j]);
            if (q[i + j].flags == 0xBD) buf[kFrame * j] = 0x00;          // hỏng magic
        }
        if (rate > 0) {                                                   // nhịp đều theo đồng hồ tuyệt đối
            next.tv_nsec += 1000000000L / rate;
            while (next.tv_nsec >= 1000000000L) { next.tv_nsec -= 1000000000L; next.tv_sec++; }
            timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
            long ahead = (next.tv_sec - now.tv_sec) * 1000000000L + (next.tv_nsec - now.tv_nsec);
            if (ahead > 200000) clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
            else while (ahead > 0) {                                      // khoảng ngắn: chờ bận, ngủ không đủ chính xác
                clock_gettime(CLOCK_MONOTONIC, &now);
                ahead = (next.tv_sec - now.tv_sec) * 1000000000L + (next.tv_nsec - now.tv_nsec);
            }
        }
        ssize_t r = conn ? send(fd, buf.data(), kFrame * k, 0)
                         : sendto(fd, buf.data(), kFrame * k, 0, (sockaddr*)&dst, sizeof dst);
        if (r < 0) { errs++; last_errno = errno; if (first_err < 0) first_err = dgrams; }
        dgrams++;
    }
    timespec t1; clock_gettime(CLOCK_MONOTONIC, &t1);
    double s = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
    printf("sensor dev=%d: %zu frame trong %ld datagram, %.3f s (%.0f datagram/s)\n", dev, q.size(), dgrams, s, dgrams / s);
    if (n_drop + n_dup + n_swap + n_bad + (restart >= 0))
        printf("  chen loi: bo %ld, gui doi %ld, dao %ld cap, hong %ld, khoi dong lai %s\n",
               n_drop, n_dup, n_swap, n_bad, restart >= 0 ? "co" : "khong");
    if (errs) printf("  loi gui: %ld lan, lan dau o datagram #%ld: %s\n", errs, first_err, strerror(last_errno));
}
