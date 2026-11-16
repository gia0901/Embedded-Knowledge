// twice: printf trước fork, không fflush. Chạy ra terminal và ra file để so sánh.
// Build: g++ -O2 -std=c++20 -o twice twice.cpp
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    std::printf("telemd: bat dau\n");     // nằm trong buffer stdio nếu stdout không phải terminal
    pid_t pid = fork();
    if (pid == 0) return 0;               // con thoát bằng return: exit() xả buffer đã chép từ cha
    waitpid(pid, nullptr, 0);
}
