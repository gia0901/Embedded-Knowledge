#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

// ---- TN1 (Cau 10): quen close(p[1]) o cha -> read khong bao gio thay EOF ----
static void tn_pipe(bool close_write_end) {
    int p[2]; if (pipe(p)) return;
    pid_t worker = fork();
    if (worker == 0) {                       // "cha" trong thi nghiem
        pid_t c = fork();
        if (c == 0) {                        // con: giong scan_tool
            dup2(p[1], STDOUT_FILENO); close(p[0]); close(p[1]);
            fputs("ket qua quet", stdout); fflush(stdout);
            _exit(0);
        }
        if (close_write_end) close(p[1]);    // <-- cach sua
        char buf[256]; ssize_t n; size_t tot = 0;
        while ((n = read(p[0], buf, sizeof buf)) > 0) tot += (size_t)n;
        _exit(tot > 0 ? 0 : 1);              // ve duoc day = thay EOF
    }
    close(p[0]); close(p[1]);
    usleep(400000);
    int st; pid_t r = waitpid(worker, &st, WNOHANG);
    bool hung = (r == 0);
    if (hung) { kill(worker, SIGKILL); waitpid(worker, &st, 0); }
    printf("  cha %-18s -> %s\n", close_write_end ? "CO close(p[1])" : "QUEN close(p[1])",
           hung ? "TREO vinh vien trong read()   <-- bug" : "doc xong, thay EOF, thoat sach");
}

// ---- TN2 (Cau 11): SIGCHLD khong xep hang -> 1 waitqpid/handler bo sot zombie ----
static int g_reaped = 0;
static void h_once(int)  { if (waitpid(-1, nullptr, WNOHANG) > 0) g_reaped++; }
static void h_loop(int)  { while (waitpid(-1, nullptr, WNOHANG) > 0) g_reaped++; }

static void tn_sigchld(bool loop) {
    struct sigaction sa = { };
    sa.sa_handler = loop ? h_loop : h_once;
    sigemptyset(&sa.sa_mask); sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, nullptr);
    g_reaped = 0;

    const int N = 5;
    for (int i = 0; i < N; i++) if (fork() == 0) _exit(0);   // 5 con thoat cung luc
    usleep(300000);

    int zombies = 0;
    for (int i = 0; i < N; i++) if (waitpid(-1, nullptr, WNOHANG) > 0) zombies++;
    printf("  handler %-22s -> thu hoi trong handler: %d/%d, con lai ZOMBIE: %d %s\n",
           loop ? "waitpid trong VONG LAP" : "waitpid GOI 1 LAN",
           g_reaped, N, zombies, zombies ? "<-- ro PID" : "");
    signal(SIGCHLD, SIG_DFL);
}

// ---- TN3 (Cau 5): fopen("w") cat file NGAY, truoc khi ghi byte nao ----
static void tn_truncate() {
    const char* path = "cfg.json";
    FILE* f0 = fopen(path, "w");
    fputs("{\"brightness\":80,\"lang\":\"vi\"}", f0); fclose(f0);
    struct stat st{}; stat(path, &st);
    printf("  ban cu tren dia                    -> %ld byte\n", (long)st.st_size);

    FILE* f = fopen(path, "w");                 // <-- chi MO, chua ghi gi
    stat(path, &st);
    printf("  ngay sau fopen(\"w\"), CHUA ghi gi   -> %ld byte  <-- mat dien o day = mat sach\n",
           (long)st.st_size);
    fputs("{\"brightness\":30,\"lang\":\"vi\"}", f);
    stat(path, &st);
    printf("  da fputs nhung CHUA fclose         -> %ld byte  (con nam trong buffer stdio)\n",
           (long)st.st_size);
    fclose(f);
    stat(path, &st);
    printf("  sau fclose                         -> %ld byte  (moi xuong page cache)\n",
           (long)st.st_size);
    unlink(path);
}

int main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);
    printf("=== TN1 (Cau 10): pipe khong thay EOF ===\n");
    tn_pipe(false); tn_pipe(true);
    printf("\n=== TN2 (Cau 11): SIGCHLD khong xep hang ===\n");
    tn_sigchld(false); tn_sigchld(true);
    printf("\n=== TN3 (Cau 5): fopen(\"w\") cat file NGAY LAP TUC ===\n");
    tn_truncate();
    return 0;
}
