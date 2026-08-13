// =====================================================================
//  OS-020 · signal-safety trong daemon — BẢN ĐÃ REVIEW
//  Phiên: 2026-08-13 · rapid · track linux-sysprog (Datalogic Tuần 2 · B1)
//  Điểm: 3/4  (lần trước 2 — nền đã vá xong, mất điểm ở tầng follow-up)
//
//  ĐỀ: daemon dưới đây treo 3–4 lần / 500 lần `systemctl stop`, phải SIGKILL.
//      Chỉ ra hàm cấm + cơ chế, rồi viết lại handler cho đúng.
//
//      static char* g_last_msg = NULL;
//      void on_sigterm(int sig) {
//          printf("[shutdown] signal %d, last msg: %s\n", sig, g_last_msg);
//          free(g_last_msg);
//          g_last_msg = NULL;
//          _exit(0);
//      }
//      int main(void) {
//          struct sigaction sa = { .sa_handler = on_sigterm };
//          sigaction(SIGTERM, &sa, NULL);
//          for (;;) { g_last_msg = strdup(read_next_event()); process(g_last_msg); }
//      }
//
//  ✅ ĐƯỢC
//   [N1] Gọi tên đúng hai hàm cấm: printf + free (lần trước sai — tưởng write() cấm)
//   [N2] Cơ chế chuẩn: strdup giữ khoá heap -> SIGTERM chen -> free xin lại cùng khoá
//        -> self-deadlock. KHÔNG còn nhầm "_r là signal-safe" như phiên trước
//   [N3] Tự giải thích được vì sao 3–4/500 chứ không phải 500/500 (cửa sổ race hẹp)
//   [N4] Bản sửa dùng đúng mẫu: volatile sig_atomic_t + handler chỉ set cờ
//   [N5] EINTR: nhận diện đúng giá trị trả về của read() khi bị signal cắt
//
//  ❌ MẤT ĐIỂM
//   [E1] SA_RESTART là QUYẾT ĐỊNH NGƯỢC — đề xuất nó ở đúng daemon này thì
//        SIGTERM trở nên vô hiệu (chứng minh bằng thí nghiệm, xem cuối file)
//   [E2] printf(...) rồi _exit(0): _exit KHÔNG flush stdio -> mất dòng log
//        khi stdout là pipe (journald). Chỉ nêu được sau khi bị thu hẹp đề
//   [E3] Không biết systemd chờ TimeoutStopSec (mặc định 90s) rồi mới SIGKILL
//   [E4] Kiểm tra cờ đặt SAU process(): nếu read_next_event() chặn thì
//        không bao giờ chạy tới -> daemon vẫn treo, đúng bug ban đầu
//   [E5] Diễn đạt lệch: "read async-signal-safe nên không phải điểm gây lỗi".
//        Async-signal-safety = "gọi được TỪ TRONG handler không". read() ở
//        main loop thì tính chất đó không liên quan — vấn đề của nó là blocking
//
//  Build:  g++ -std=c++17 -Wall -Wextra -o sigterm 2026-08-13--OS-020--sigterm-handler.cpp
//  Run:    ./sigterm          (tự chạy 3 thí nghiệm, không cần systemd)
// =====================================================================

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

// ---------------------------------------------------------------------
// Stub cho hai hàm của đề (để file này compile + chạy độc lập).
// read_next_event() được cố ý làm CHẶN — đó là điều kiện lộ bug [E4].
// ---------------------------------------------------------------------
static int g_event_pipe[2];

static char* read_next_event() {
    char c;
    ssize_t n = ::read(g_event_pipe[0], &c, 1);   // chặn: không ai ghi vào pipe
    if (n <= 0) return nullptr;                    // n < 0 && errno == EINTR
    return strdup("scan:4006381333931");
}
static void process(const char*) { /* no-op */ }


// =====================================================================
//  PHẦN 1 — BẢN ỨNG VIÊN NỘP (giữ nguyên từng dòng, chỉ chèn comment)
//  Nguồn: coding-arena/test.cpp
// =====================================================================
namespace submitted {

static char* g_last_msg = NULL;

volatile sig_atomic_t isTerminated = 0;          // ✅ [N4] đúng kiểu, đúng volatile

void on_sigterm(int sig) {
    (void)sig;                                    // (thêm để -Wextra sạch)
    isTerminated = 1;                             // ✅ [N4] handler chỉ set cờ — CHUẨN
}                                                 // ✅ bỏ hẳn printf/free khỏi handler

int main_(void) {
    struct sigaction sa = { };
    sa.sa_handler = on_sigterm;
    sigaction(SIGTERM, &sa, NULL);
    // ⚠️ [E1] Ở follow-up (e) bạn đề xuất thêm sa.sa_flags = SA_RESTART.
    //         ĐỪNG. Với SA_RESTART, read() bên dưới tự chạy lại sau signal
    //         => vòng lặp không bao giờ tới dòng kiểm tra cờ => SIGTERM vô hiệu.
    //         Thí nghiệm 3 ở cuối file chạy thật cả hai nhánh.

    for (;;) {
        g_last_msg = strdup(read_next_event());   // ❌ [E4] read() CHẶN ở đây.
                                                  //    SIGTERM tới -> handler set cờ ->
                                                  //    read trả EINTR -> strdup(NULL) = UB.
                                                  //    Không kiểm tra giá trị trả về.
        process(g_last_msg);

        if (isTerminated == 1) {                  // ❌ [E4] kiểm tra cờ ĐẶT SAI CHỖ:
                                                  //    nằm sau chỗ chặn, nên lúc thiết bị
                                                  //    rảnh (không có event) thì không bao
                                                  //    giờ chạy tới. systemd chờ
                                                  //    TimeoutStopSec=90s rồi SIGKILL [E3]
            printf("[shutdown] sigterm, last msg: %s\n", g_last_msg);
            _exit(0);                             // ❌ [E2] _exit KHÔNG flush stdio.
                                                  //    stdout -> pipe (journald) = full
                                                  //    buffered => DÒNG LOG BIẾN MẤT.
                                                  //    Dùng fflush+_exit, hoặc exit(),
                                                  //    hoặc write(2,...) thẳng.
        }
    }
}

} // namespace submitted


// =====================================================================
//  PHẦN 2 — BẢN SỬA
//  Giữ nguyên mọi quyết định thiết kế hợp lý của ứng viên:
//    · handler chỉ set cờ  · volatile sig_atomic_t  · in last_msg lúc shutdown
//  Chỉ vá 4 lỗi đã đánh nhãn [E1]–[E4].
// =====================================================================
namespace fixed {

static char* g_last_msg = nullptr;
volatile sig_atomic_t isTerminated = 0;

void on_sigterm(int sig) {
    (void)sig;
    isTerminated = 1;            // ✅ giữ nguyên: handler làm ít nhất có thể
    // ⚠️ Handler KHÔNG được đụng errno của main. Ở đây không gọi syscall nên
    //    không sửa errno; nếu có (vd write) thì phải lưu/khôi phục errno.
}

int main_(void) {
    struct sigaction sa = { };
    sa.sa_handler = on_sigterm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;             // ✅ [E1] KHÔNG SA_RESTART — ta CẦN read() bị EINTR
                                 //    đánh thức, đó chính là cơ chế thoát vòng lặp
    sigaction(SIGTERM, &sa, nullptr);

    for (;;) {
        char* msg = read_next_event();

        if (msg == nullptr) {                    // ✅ [E4] kiểm tra trả về TRƯỚC khi dùng
            if (errno == EINTR) {
                if (isTerminated) break;         // ✅ signal shutdown -> thoát vòng lặp
                continue;                        // ✅ signal khác     -> thử lại
            }
            break;                               // lỗi thật
        }

        free(g_last_msg);
        g_last_msg = msg;
        process(g_last_msg);

        if (isTerminated) break;                 // ✅ vẫn kiểm tra ở cuối vòng, cho ca
                                                 //    signal tới lúc đang process()
    }

    // ✅ [E2] Dọn dẹp ở main, KHÔNG ở handler — nơi này gọi gì cũng được
    printf("[shutdown] sigterm, last msg: %s\n", g_last_msg ? g_last_msg : "(none)");
    fflush(stdout);                              // ✅ bắt buộc: _exit không flush stdio
    free(g_last_msg);
    g_last_msg = nullptr;
    return 0;                                    // ✅ return từ main = exit() = có flush
}

} // namespace fixed


// =====================================================================
//  PHẦN 3 — BA THÍ NGHIỆM (chạy thật, không phỏng đoán · config §6 luật ⑥)
// =====================================================================

// TN1: printf + _exit qua pipe -> mất log.  [E2]
static void tn1_stdio_flush() {
    printf("\n=== TN1: printf roi _exit(0), stdout la PIPE (giong journald) ===\n");
    fflush(stdout);
    for (int use_exit = 0; use_exit <= 1; use_exit++) {
        int p[2];
        if (pipe(p) != 0) return;
        pid_t c = fork();
        if (c == 0) {
            dup2(p[1], STDOUT_FILENO); close(p[0]); close(p[1]);
            // Khôi phục MẶC ĐỊNH THẬT của glibc khi stdout là pipe: full buffering.
            // (main() của file test này đặt line-buffered để output không mất khi
            //  fork — con kế thừa cài đặt đó, nên phải đặt lại cho đúng đời thật.)
            setvbuf(stdout, nullptr, _IOFBF, 4096);
            printf("[shutdown] last msg: hello\n");
            if (use_exit) exit(0);
            _exit(0);
        }
        close(p[1]);
        char buf[128] = {0};
        ssize_t n = ::read(p[0], buf, sizeof buf - 1);
        close(p[0]); waitpid(c, nullptr, 0);
        printf("  %-10s -> journald nhan %zd byte: %s\n",
               use_exit ? "exit(0)" : "_exit(0)", n < 0 ? 0 : n,
               n > 0 ? buf : "(RONG — MAT LOG)\n");
    }
}

// TN2 + TN3: SIGTERM tới lúc read() đang chặn — sa_flags = 0 vs SA_RESTART.  [E1][E4]
//
// Đo đúng cách systemd nhìn: chạy "daemon" trong process con, gửi SIGTERM,
// chờ 1 giây rồi xem NÓ CÒN SỐNG KHÔNG. Còn sống = `systemctl stop` sẽ treo
// tới TimeoutStopSec rồi phải SIGKILL. (Không dùng alarm() — SIGALRM mặc định
// giết process, làm hỏng chính phép đo.)
static volatile sig_atomic_t tn_stop = 0;
static void tn_handler(int) { tn_stop = 1; }

static void tn23_eintr(bool use_restart) {
    int p[2];
    if (pipe(p) != 0) return;

    pid_t daemon = fork();
    if (daemon == 0) {                          // ---- "daemon" ----
        close(p[0]);
        struct sigaction sa = { };
        sa.sa_handler = tn_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = use_restart ? SA_RESTART : 0;
        sigaction(SIGTERM, &sa, nullptr);

        int ev[2];
        if (pipe(ev) != 0) _exit(2);
        for (;;) {                              // vòng lặp y như bản ứng viên
            char b;
            ssize_t n = ::read(ev[0], &b, 1);   // CHẶN: không ai ghi
            if (n < 0 && errno == EINTR) {      // chỉ tới được đây khi sa_flags = 0
                if (tn_stop) _exit(0);          // ✅ thoát sạch
                continue;
            }
        }
    }

    close(p[1]);
    usleep(150000);
    kill(daemon, SIGTERM);                      // = `systemctl stop`
    usleep(1000000);                            // systemd chờ... (thật ra 90s)

    int st = 0;
    pid_t r = waitpid(daemon, &st, WNOHANG);
    bool alive = (r == 0);
    if (alive) { kill(daemon, SIGKILL); waitpid(daemon, &st, 0); }

    printf("  sa_flags=%-11s -> sau SIGTERM 1s: %-28s %s\n",
           use_restart ? "SA_RESTART" : "0",
           alive ? "VAN CON SONG (treo)" : "da thoat, rc=0",
           alive ? "=> phai SIGKILL  ❌" : "=> stop sach  ✅");
    fflush(stdout);
    close(p[0]);
}

int main() {
    setvbuf(stdout, nullptr, _IOLBF, 0);        // line-buffered: output không mất khi fork
    tn1_stdio_flush();

    printf("\n=== TN2/TN3: SIGTERM toi khi read() dang chan ===\n");
    tn23_eintr(false);
    tn23_eintr(true);
    printf("  (SA_RESTART: read tu chay lai => vong lap khong bao gio kiem tra duoc co\n"
           "   => `systemctl stop` treo den TimeoutStopSec=90s roi bi SIGKILL)\n");

    printf("\n=== Ban SUA chay that ===\n");
    if (pipe(g_event_pipe) != 0) return 1;
    pid_t c = fork();
    if (c == 0) { usleep(150000); kill(getppid(), SIGTERM); _exit(0); }
    int rc = fixed::main_();
    waitpid(c, nullptr, 0);
    printf("  -> thoat sach voi rc=%d (khong can SIGKILL)\n", rc);
    return 0;
}

// =====================================================================
//  OUTPUT THẬT (g++ -std=c++17 -Wall -Wextra, không warning) — 2026-08-13
// =====================================================================
//
//  === TN1: printf roi _exit(0), stdout la PIPE (giong journald) ===
//    _exit(0)   -> journald nhan 0 byte: (RONG — MAT LOG)          <-- [E2]
//    exit(0)    -> journald nhan 27 byte: [shutdown] last msg: hello
//
//  === TN2/TN3: SIGTERM toi khi read() dang chan ===
//    sa_flags=0           -> sau SIGTERM 1s: da thoat, rc=0         => stop sach  OK
//    sa_flags=SA_RESTART  -> sau SIGTERM 1s: VAN CON SONG (treo)    => phai SIGKILL  <-- [E1]
//
//  === Ban SUA chay that ===
//    [shutdown] sigterm, last msg: (none)
//    -> thoat sach voi rc=0 (khong can SIGKILL)
//
//  CHỐT: "Handler chỉ set cờ. Chỗ đang chặn phải để EINTR đánh thức — đừng
//         dập bằng SA_RESTART. Và _exit không flush stdio."
// =====================================================================
