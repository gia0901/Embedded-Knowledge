// spawn: chạy một lệnh ngoài với stdout chuyển hướng vào file, chờ và báo exit code. Lời giải Lab 1.2.
// Giống những gì shell làm cho "cmd > out": fork, trong con mở file + dup2 + exec, cha waitpid.
// Build: g++ -O2 -std=c++20 -o spawn spawn.cpp
// Chạy:  ./spawn <file output> <lệnh> [tham số...]
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

int run(const char* out, char* const argv[]) {
    std::fflush(stdout);                                   // tránh con in lại buffer của cha
    pid_t pid = fork();
    if (pid < 0) { std::perror("fork"); return -1; }
    if (pid == 0) {
        int fd = open(out, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
        if (fd < 0) { std::perror("open"); _exit(126); }
        if (dup2(fd, STDOUT_FILENO) < 0) _exit(126);       // dup2 bỏ cờ CLOEXEC trên fd 1, fd gốc tự đóng khi exec
        execvp(argv[0], argv);
        // chỉ tới đây khi exec thất bại; dùng _exit để không chạy destructor/atexit của cha
        std::fprintf(stderr, "exec %s: %s\n", argv[0], std::strerror(errno));
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR) { std::perror("waitpid"); return -1; }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);  // cùng quy ước với shell
    return -1;
}

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "dùng: %s out cmd [args]\n", argv[0]); return 2; }
    int rc = run(argv[1], argv + 2);
    std::printf("lenh '%s' ket thuc, exit code %d\n", argv[2], rc);
    return rc == 0 ? 0 : 1;
}
