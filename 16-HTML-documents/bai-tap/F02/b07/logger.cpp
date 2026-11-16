// logger: daemon giả lập ghi log liên tục vào một file (mở bằng O_APPEND), tốc độ cho trước.
// SIGHUP: đóng và mở lại file theo đường dẫn, như các daemon hỗ trợ logrotate. Lời giải Lab 7.1.
// Build: g++ -O2 -std=c++20 -o logger logger.cpp
// Chạy:  ./logger <file log> <MB mỗi giây>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>

static volatile sig_atomic_t g_reopen = 0;
static void on_hup(int) { g_reopen = 1; }

int main(int argc, char** argv) {
    const char* path = argv[1];
    long mbps = std::atol(argv[2]);
    struct sigaction sa{};
    sa.sa_handler = on_hup;
    sigaction(SIGHUP, &sa, nullptr);
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    char line[1024];
    std::memset(line, 'x', sizeof line);
    line[sizeof line - 1] = '\n';
    std::printf("logger pid=%d fd=%d\n", getpid(), fd);
    std::fflush(stdout);
    for (;;) {
        for (long i = 0; i < mbps * 10; ++i)              // mbps MB mỗi giây = mbps*1024 dòng 1 KB, chia 100 lần
            if (write(fd, line, sizeof line) < 0) { std::perror("write"); return 1; }
        timespec ts{0, 10'000'000};
        nanosleep(&ts, nullptr);
        if (g_reopen) {                                   // logrotate đã đổi tên file: mở file mới theo đường dẫn
            g_reopen = 0;
            int nfd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
            dup2(nfd, fd);                                // giữ nguyên số fd, đóng file cũ
            close(nfd);
        }
    }
}
