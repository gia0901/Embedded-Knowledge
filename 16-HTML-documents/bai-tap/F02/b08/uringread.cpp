// uringread: đọc ngẫu nhiên 4 KB với O_DIRECT (không qua page cache), so pread từng lần với io_uring
// giữ nhiều yêu cầu cùng lúc (queue depth). Dùng cho §8.6.
// Build: g++ -O2 -std=c++20 -o uringread uringread.cpp -luring
// Chạy:  ./uringread <file> pread|uring [số lần đọc=20000] [queue depth=32]
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <liburing.h>
#include <random>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "dung: %s file pread|uring [n] [qd]\n", argv[0]); return 2; }
    std::string mode = argv[2];
    long n = argc > 3 ? std::atol(argv[3]) : 20000;
    unsigned qd = argc > 4 ? std::atoi(argv[4]) : 32;
    int fd = open(argv[1], O_RDONLY | O_DIRECT | O_CLOEXEC);
    if (fd < 0) { std::perror("open"); return 1; }
    struct stat st; fstat(fd, &st);
    long blocks = st.st_size / 4096;
    std::mt19937_64 rng(42);
    auto off = [&] { return (off_t)(rng() % blocks) * 4096; };
    void* buf;
    if (posix_memalign(&buf, 4096, 4096 * qd)) return 1;      // O_DIRECT cần buffer căn 4096

    long enters = 0;
    auto t0 = std::chrono::steady_clock::now();
    if (mode == "pread") {
        for (long i = 0; i < n; ++i)
            if (pread(fd, buf, 4096, off()) != 4096) { std::perror("pread"); return 1; }
        enters = n;
    } else {
        io_uring ring;
        if (int e = io_uring_queue_init(qd, &ring, 0); e < 0) { std::fprintf(stderr, "io_uring_queue_init: %d\n", e); return 1; }
        long submitted = 0, done = 0;
        while (done < n) {
            while (submitted < n && submitted - done < qd) {   // giữ đầy qd yêu cầu đang chạy
                io_uring_sqe* sqe = io_uring_get_sqe(&ring);
                long slot = submitted % qd;
                io_uring_prep_read(sqe, fd, (char*)buf + slot * 4096, 4096, off());
                ++submitted;
            }
            io_uring_submit_and_wait(&ring, 1);                 // một system call: nộp lô mới, chờ ít nhất 1 kết quả
            ++enters;
            io_uring_cqe* cqe;
            unsigned head, seen = 0;
            io_uring_for_each_cqe(&ring, head, cqe) {           // lấy mọi kết quả đã có, không cần system call
                if (cqe->res != 4096) { std::fprintf(stderr, "read loi: %d\n", cqe->res); return 1; }
                ++seen;
            }
            io_uring_cq_advance(&ring, seen);
            done += seen;
        }
        io_uring_queue_exit(&ring);
    }
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("%-5s qd=%-3u %ld lan doc 4 KB: %6.0f IOPS, %5.1f us/lan, %ld system call\n",
                mode.c_str(), mode == "pread" ? 1u : qd, n, n / s, s * 1e6 / n, enters);
}
