// rotator: mô phỏng telemd nén log định kỳ bằng một process con. Dùng cho Lab 1.3.
// Bản "leak" không wait con → zombie tích tụ. Bản "reap" dọn bằng waitpid(-1, WNOHANG) trong vòng chính.
// Build: g++ -O2 -std=c++20 -o rotator rotator.cpp
// Chạy:  ./rotator leak  &   (rồi xem ps)      ./rotator reap &
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv) {
    bool reap = argc > 1 && std::strcmp(argv[1], "reap") == 0;
    for (int round = 0; round < 20; ++round) {
        pid_t pid = fork();
        if (pid == 0) {                       // con: "nén log" rồi thoát ngay
            execlp("true", "true", nullptr);
            _exit(127);
        }
        if (reap) {
            // dọn mọi con đã thoát, không chặn nếu chưa có con nào xong
            while (waitpid(-1, nullptr, WNOHANG) > 0) {}
        }
        usleep(100'000);
    }
    if (reap) while (waitpid(-1, nullptr, WNOHANG) > 0) {}   // dọn nốt con của vòng cuối
    std::printf("rotator(%s) pid=%d xong 20 vong, ngu 30 s de ban xem ps\n", reap ? "reap" : "leak", getpid());
    std::fflush(stdout);
    sleep(30);
}
