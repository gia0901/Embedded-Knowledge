# TLPI — Cụm 03: Signals & Timers (ch. 20–23) 🎯

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 20–23, tr. 387–512.
> **Vì sao cụm này ưu tiên cao:** signal là chỗ **nhiều bug thật nhất** trong lập trình Linux userspace, và là chỗ phỏng vấn phân loại rõ nhất giữa "biết gọi API" và "hiểu cơ chế". Kerrisk dành **ba chương liền** cho nó — nhiều hơn bất kỳ chủ đề đơn lẻ nào khác trong sách.

---

## 1. Signal là gì — mô hình tinh thần đúng (ch. 20, tr. 387–420)

**Signal là thông báo bất đồng bộ gửi tới process rằng "có sự kiện xảy ra".** Kerrisk gọi nó là *"software interrupts"* (§20.1, tr. 388) — và phép so sánh đó là cách hiểu đúng nhất: nó **cắt ngang luồng thực thi ở một điểm không đoán trước được**, đúng như ngắt phần cứng cắt ngang CPU.

### 1.1. Vòng đời một signal — ba trạng thái phải phân biệt

Đây là chỗ nhầm nhiều nhất. Ba động từ **khác nhau**, không đồng nghĩa:

```
   ①  GENERATED (sinh ra)          ②  PENDING (chờ)              ③  DELIVERED (giao)
       sự kiện xảy ra                  đã sinh nhưng CHƯA giao        thực sự tác động lên process
   ┌──────────────────┐            ┌────────────────────┐        ┌─────────────────────┐
   │ kill(), lỗi phần │            │ signal bị CHẶN     │        │ chạy handler        │
   │ cứng (SIGSEGV),  │───────────►│ (blocked bởi       │───────►│ HOẶC hành vi mặc    │
   │ phần mềm (SIGPIPE│            │  signal mask)      │        │ định (term/core/ign)│
   │ ), user gõ ^C    │            │  → nằm chờ         │        │                     │
   └──────────────────┘            └────────────────────┘        └─────────────────────┘
                                            │
                                    bỏ chặn (sigprocmask)
                                    → được giao NGAY
```

**Hệ quả quan trọng:** nếu signal bị chặn *rất lâu*, nó vẫn **pending** — không mất. Nhưng...

### 1.2. Signal chuẩn KHÔNG xếp hàng — hệ quả lớn nhất của chương 20

Kerrisk nhấn mạnh (§20.10, tr. 405): tập signal pending được biểu diễn bằng **một bit cho mỗi loại signal**. Nghĩa là:

```
   Signal mask / pending set = MỘT BIT mỗi signal

   SIGINT  SIGCHLD  SIGTERM  ...
     [1]     [1]      [0]

   ⇒ 100 lần SIGCHLD tới trong lúc bị chặn  →  bit vẫn chỉ là 1
   ⇒ Khi bỏ chặn: handler chạy ĐÚNG MỘT LẦN
   ⇒ 99 signal kia BIẾN MẤT, không có cách nào biết
```

> ⚠️ **Đây là nguồn của bug kinh điển: process zombie tích tụ.** Bạn cài handler `SIGCHLD` gọi `wait()` một lần. Mười process con chết gần như đồng thời → chỉ một `SIGCHLD` được giao → chỉ một con được thu hoạch → **9 zombie ở lại vĩnh viễn**. Cách chữa đúng là **vòng lặp `waitpid()` với `WNOHANG`** cho tới khi hết (§26.3.1) — xem Góc interview câu 4.

**Realtime signal** (`SIGRTMIN`…`SIGRTMAX`, §22.8, tr. 460) sửa đúng ba hạn chế này: **có xếp hàng**, **có thứ tự ưu tiên**, và **mang được dữ liệu kèm** (`sigval` qua `sigqueue()`). Nhưng hàng đợi có trần (`RLIMIT_SIGPENDING`) — tràn là quay về mất tin.

### 1.3. Bảng signal cần thuộc

| Signal | Mặc định | Chặn được? | Khi nào |
|---|---|---|---|
| `SIGINT` | term | ✓ | Ctrl-C |
| `SIGTERM` | term | ✓ | **`kill` mặc định** — tín hiệu "xin hãy tự dọn dẹp rồi thoát" |
| `SIGKILL` | term | ❌ **không** | `kill -9` — không chặn, không bắt, không bỏ qua |
| `SIGSTOP` | stop | ❌ **không** | dừng process |
| `SIGSEGV` | core | ✓ | truy cập bộ nhớ sai |
| `SIGPIPE` | term | ✓ | ghi vào pipe/socket **không còn reader** |
| `SIGCHLD` | **ignore** | ✓ | con chết/dừng/tiếp tục |
| `SIGHUP` | term | ✓ | terminal đóng; **quy ước: daemon dùng nó để nạp lại config** |
| `SIGALRM` | term | ✓ | `alarm()`/`setitimer()` hết hạn |
| `SIGUSR1/2` | term | ✓ | tự định nghĩa |

*(rút gọn từ Table 20-1, tr. 396)*

> **Vì sao `SIGKILL`/`SIGSTOP` không chặn được:** để **luôn** còn một cách kết thúc/dừng process bất trị. Đây là quyết định thiết kế, không phải thiếu sót — và là lý do `kill -9` không cho process cơ hội dọn dẹp (không xoá file tạm, không flush buffer, không nhả khoá). 🆕 Đó cũng là lý do `docker stop` gửi `SIGTERM` trước, chờ, rồi mới `SIGKILL`.

---

## 2. Signal handler — nơi mọi thứ dễ hỏng (ch. 21, tr. 421–446) 🎯

### 2.1. `signal()` hay `sigaction()`?

**Luôn dùng `sigaction()`.** Kerrisk nói thẳng (§21.2, tr. 427): `signal()` có **ngữ nghĩa khác nhau giữa các hệ thống** (System V vs BSD) — điều khác nhau gồm cả những thứ chí tử: handler có tự reset về mặc định sau lần đầu không, signal có bị chặn trong lúc handler chạy không, syscall có được restart không.

```c
struct sigaction sa;
sa.sa_handler = handler;            // hàm xử lý
sigemptyset(&sa.sa_mask);           // signal nào bị CHẶN THÊM khi handler chạy
                                    // (bản thân signal đang xử lý LUÔN bị chặn)
sa.sa_flags = SA_RESTART;           // xem §3
if (sigaction(SIGINT, &sa, NULL) == -1)
    errExit("sigaction");
```

Ba trường, và trường giữa hay bị bỏ qua: **`sa_mask` chống việc handler bị chính signal khác cắt ngang** giữa chừng — quan trọng khi handler đụng dữ liệu chia sẻ.

### 2.2. Async-signal-safe — quy tắc quan trọng nhất cụm này 🎯

**Định nghĩa của sách** (§21.1.2, tr. 425): *"An async-signal-safe function is one that the implementation guarantees to be safe when called from a signal handler. A function is async-signal-safe either because it is reentrant or because it is not interruptible by a signal handler."*

**Vì sao cần khái niệm này** — Kerrisk minh hoạ bằng chương trình `nonreentrant.c` (Listing 21-3, tr. 424) rất thuyết phục:

```c
/* Ý tưởng chương trình chứng minh của sách (rút gọn):
   crypt() trả về con trỏ tới BỘ NHỚ TĨNH dùng chung. */

char *cr1 = strdup(crypt(argv[1], "xx"));   // chép ra buffer riêng để so sánh

static void handler(int sig) {
    crypt(str2, "xx");        // ← handler cũng gọi crypt() → ĐÈ LÊN bộ nhớ tĩnh đó
}

for (;;) {
    if (strcmp(crypt(argv[1], "xx"), cr1) != 0)   // main gọi crypt() liên tục
        printf("Mismatch!\n");                     // ← IN RA khi handler chen vào
}
```

Chạy chương trình rồi bấm Ctrl-C liên tục → in ra "Mismatch". Kết quả *sai* xuất hiện **không phải do lỗi logic**, mà vì handler chen vào giữa lúc `crypt()` đang dùng bộ nhớ tĩnh của nó. Đó là **non-reentrant** ở dạng thuần khiết nhất.

**Hai lý do một hàm không an toàn:**
1. **Dùng biến/buffer tĩnh dùng chung** — `crypt()`, `strtok()`, `getpwnam()`, `asctime()`, và **mọi hàm stdio** (`printf` dùng buffer chung).
2. **Cập nhật cấu trúc dữ liệu toàn cục theo nhiều bước** — kinh điển nhất là **`malloc`/`free`**: chúng thao tác trên free-list của heap. Handler gọi `malloc()` trong lúc main đang ở giữa `malloc()` → **heap hỏng**.

**Danh sách an toàn** (Table 21-1, tr. 425–426) — thuộc nhóm chính là đủ:

| Nhóm | Ví dụ |
|---|---|
| I/O thô | `read()`, `write()`, `open()`, `close()` |
| Process | `_exit()`, `fork()`, `execve()`, `waitpid()`, `kill()` |
| Signal | `sigaction()`, `sigprocmask()`, `signal()`, `raise()` |
| Thời gian | `alarm()`, `time()`, `sleep()` |
| ⚠️ **KHÔNG** an toàn | **`printf` và mọi stdio**, `malloc`/`free`, `strtok`, `crypt`, `getpwnam`, `syslog` |

> ⚠️ **`printf()` trong signal handler là bug**, dù 99% lần chạy trông vẫn ổn. Nó không an toàn vì stdio dùng **buffer và khoá dùng chung**: nếu signal tới đúng lúc main đang ở giữa `printf`, handler gọi `printf` sẽ làm hỏng trạng thái buffer — kết quả là output lẫn lộn, hoặc **deadlock** (main đang giữ khoá stdio, handler chờ chính khoá đó, mà main không chạy tiếp được vì đang bị handler cắt ngang).
>
> **Thay bằng:** `write(STDERR_FILENO, "msg\n", 4)` — `write()` là syscall, nằm trong danh sách an toàn.

### 2.3. Ba quy tắc viết handler đúng

```c
volatile sig_atomic_t gotSignal = 0;   // ① kiểu + volatile đúng chuẩn

static void handler(int sig) {
    int savedErrno = errno;            // ② LƯU errno
    gotSignal = 1;                     //    (handler chỉ ĐẶT CỜ, không làm gì nặng)
    errno = savedErrno;                //    khôi phục errno
}
```

1. **Handler càng ngắn càng tốt** — lý tưởng là chỉ **đặt một cờ**, để main loop làm việc thật. Đây chính là mô hình *top-half/bottom-half* của ngắt phần cứng, áp cho userspace ([08/interrupts-bare-metal](../../08-embedded-systems/interrupts-bare-metal.md)).
2. **Biến chia sẻ với handler phải là `volatile sig_atomic_t`** (§21.1.3, tr. 428). `sig_atomic_t` đảm bảo đọc/ghi **nguyên tử** (không bị cắt giữa chừng); `volatile` cấm compiler tối ưu bỏ lần đọc lại. 🆕 Đây đúng là câu chuyện `volatile` của MCU — cùng vấn đề, khác ngữ cảnh ([EMB-010](../../14-prep/mock-interview/bank/embedded-fundamentals.md)).
3. **Lưu và khôi phục `errno`** (§21.1.2, tr. 426). Handler có thể gọi syscall (vd `write`) làm hỏng `errno`; main quay lại đọc `errno` thì được giá trị sai. Đây là bug cực khó lần vì nó chỉ xuất hiện khi signal tới đúng khe hở.

---

## 3. `EINTR` và `SA_RESTART` — cạm bẫy thực chiến số 1 (§21.5, tr. 442–444) 🎯

### 3.1. Vấn đề

```
   ① Cài handler cho một signal
   ② Gọi syscall CHẶN, vd read() từ terminal
   ③ Signal tới trong lúc read() đang chặn → handler chạy
   ④ Handler trả về... rồi sao?

   → MẶC ĐỊNH: read() THẤT BẠI với errno = EINTR ("Interrupted function")
```

Đây **không phải lỗi thật** — chỉ là "bị cắt ngang". Nếu code của bạn coi `-1` là lỗi và thoát, chương trình sẽ **chết ngẫu nhiên** mỗi khi có signal tới đúng lúc.

### 3.2. Hai cách xử lý

```c
/* Cách 1 — tự khởi động lại bằng vòng lặp (sách, tr. 442) */
while ((cnt = read(fd, buf, BUF_SIZE)) == -1 && errno == EINTR)
    continue;                               /* thân rỗng — chỉ lặp lại */
if (cnt == -1)
    errExit("read");                        /* lỗi THẬT (khác EINTR) */

/* glibc có sẵn macro cho việc này (cần _GNU_SOURCE): */
TEMP_FAILURE_RETRY(cnt = read(fd, buf, BUF_SIZE));
```

```c
/* Cách 2 — bảo kernel tự restart hộ */
sa.sa_flags = SA_RESTART;      // ← cờ theo TỪNG SIGNAL, không phải toàn cục
sigaction(SIGINT, &sa, NULL);
```

### 3.3. Bẫy lớn: `SA_RESTART` KHÔNG cứu được mọi syscall 🎯

Đây là chỗ Kerrisk liệt kê rất chi tiết (tr. 443–444), và là **chi tiết phân loại ứng viên** — vì rất nhiều người tin `SA_RESTART` là xong chuyện.

| Có restart với `SA_RESTART` | **KHÔNG BAO GIỜ** restart (dù có `SA_RESTART`) |
|---|---|
| `wait()`, `waitpid()`, `waitid()` | **`select()`, `pselect()`, `poll()`, `ppoll()`** |
| `read()`, `write()`, `readv()`, `writev()`, `ioctl()` — **chỉ trên thiết bị "chậm"** (terminal, pipe, FIFO, socket) | **`epoll_wait()`, `epoll_pwait()`** |
| `open()` khi nó có thể chặn (vd mở FIFO) | `semop()`, `msgrcv()`, `msgsnd()` (System V IPC) |
| `accept()`, `connect()`, `send()`, `recv()`… | `read()` từ fd của **inotify** |
| `mq_receive()`, `mq_send()` | **`sleep()`, `nanosleep()`, `clock_nanosleep()`** |
| `flock()`, `fcntl()` (đặt khoá), `futex(FUTEX_WAIT)` | **`pause()`, `sigsuspend()`, `sigwaitinfo()`** |
| `sem_wait()`, `pthread_mutex_lock()`, `pthread_cond_wait()` | `io_getevents()` |

**Hai điều rút ra:**

1. **Vòng lặp `EINTR` quanh `epoll_wait`/`select`/`poll` là BẮT BUỘC** — không có cờ nào thay được. Nối thẳng với [cụm 05 §7 câu 5](05-alternative-io-models.md).
2. **"Thiết bị chậm" là khái niệm có thật.** `read()` từ **file trên đĩa** không thuộc nhóm này (kernel coi I/O đĩa là luôn thoả mãn được qua buffer cache), nên nó **không** bị `EINTR` — đó là lý do bug này *không xuất hiện* khi test với file, chỉ lộ ra khi chạy với socket/pipe/terminal thật.

> 🆕 Ghi chú tinh: nếu `read()`/`write()` **đã chuyển được một phần dữ liệu** rồi mới bị signal cắt, nó **không** restart mà **trả về thành công với số byte đã chuyển** (tr. 443). Tức là partial read/write ở [cụm 01 §2.2](01-concepts-and-file-io.md) có thêm một nguyên nhân nữa: signal.

---

## 4. Chặn signal và bài toán race (ch. 22, tr. 447–478)

### 4.1. Signal mask

```c
sigset_t blockSet, prevMask;
sigemptyset(&blockSet);
sigaddset(&blockSet, SIGINT);

/* Chặn SIGINT trước vùng tới hạn, lưu mask cũ */
sigprocmask(SIG_BLOCK, &blockSet, &prevMask);

/* ... vùng tới hạn: sửa dữ liệu mà handler cũng đụng ... */

sigprocmask(SIG_SETMASK, &prevMask, NULL);   /* khôi phục */
```

Ba lệnh: `SIG_BLOCK` (thêm), `SIG_UNBLOCK` (bớt), `SIG_SETMASK` (đặt hẳn). Trong chương trình đa luồng dùng **`pthread_sigmask()`** — mask là thuộc tính **của từng thread** (§33.2).

### 4.2. Race giữa chương trình và chính handler của nó 🎯

Kerrisk trình bày một race rất đẹp (§22.9, tr. 463–464) — *"the main program is racing against its own signal handler"*:

```c
/* ❌ CÓ BUG — Listing 22-4 của sách, tr. 463 */
sigprocmask(SIG_BLOCK, &intMask, &prevMask);   // chặn SIGINT
    /* ... vùng tới hạn ... */
sigprocmask(SIG_SETMASK, &prevMask, NULL);     // bỏ chặn
                                               // ← ⚠️ SIGINT tới ĐÚNG ĐÂY:
                                               //    handler chạy XONG rồi mới tới pause()
pause();                                       // → chờ MÃI MÃI signal thứ HAI
```

```
   thời gian ──────────────────────────────────────────────►
   main:  [chặn][vùng tới hạn][BỎ CHẶN]···khe hở···[pause()]
                                          ▲              ▲
                              SIGINT tới ĐÂY,      pause() bắt đầu chờ,
                              handler chạy & xong  nhưng signal ĐÃ TIÊU THỤ RỒI
                                                   → treo vĩnh viễn
```

**Lời giải: `sigsuspend()`** — làm **nguyên tử** hai việc "đặt lại mask" và "đi ngủ":

```c
/* ✅ ĐÚNG */
sigprocmask(SIG_BLOCK, &intMask, &prevMask);
    /* ... vùng tới hạn ... */
sigsuspend(&prevMask);      // đặt mask = prevMask VÀ ngủ, KHÔNG có khe hở
                            // luôn trả -1 với errno = EINTR
```

> **Câu chốt:** đây **cùng một mẫu tư duy** với `O_APPEND`/`O_EXCL` ở [cụm 01 §2.4](01-concepts-and-file-io.md): *hai thao tác không được để ai chen vào giữa thì phải tìm syscall làm gộp*. Mẫu này còn xuất hiện ở `pselect`/`ppoll`/`epoll_pwait` (nhận `sigmask` nguyên tử) và `accept4(SOCK_CLOEXEC)`.

### 4.3. Ba cách hiện đại hơn để "chờ signal"

| Cách | Ý tưởng | Ưu |
|---|---|---|
| **`sigwaitinfo()` / `sigtimedwait()`** (§22.10) | **Đồng bộ** nhận signal: chặn signal rồi *chờ lấy* nó, **không cần handler** | Thoát hẳn ràng buộc async-signal-safe |
| **`signalfd()`** (§22.11, tr. 471) | Biến signal thành **file descriptor đọc được** | **Đưa signal vào `epoll`** cùng mọi fd khác — mảnh ghép cuối của event loop |
| **self-pipe trick** | Handler chỉ `write()` 1 byte vào pipe; main loop `select` trên pipe đó | Cách cổ điển, khả chuyển, làm được điều tương tự `signalfd` |

🆕 `signalfd()` là câu trả lời hiện đại cho toàn bộ chương này: thay vì viết handler bị trói bởi async-signal-safe, bạn **đọc signal như đọc dữ liệu** trong cùng vòng lặp `epoll`. Đây là kiến trúc mà systemd và mọi daemon hiện đại dùng.

---

## 5. Timer & sleep (ch. 23, tr. 479–512)

Ba thế hệ API, biết thế hệ nào dùng cho việc gì:

| Thế hệ | API | Độ phân giải | Ghi chú |
|---|---|---|---|
| Cổ | `alarm()` | **giây** | Gửi `SIGALRM`. Chỉ **một** timer/process — gọi lần hai huỷ lần một |
| BSD | `setitimer()` / `getitimer()` | micro-giây | Ba loại: `ITIMER_REAL` (thời gian thực → `SIGALRM`), `ITIMER_VIRTUAL` (CPU user), `ITIMER_PROF` (CPU user+sys) |
| **POSIX** | `timer_create()` + `timer_settime()` | nano-giây | **Nhiều timer**, chọn được clock, và chọn được **cách báo**: signal, hoặc **tạo thread mới** (`SIGEV_THREAD`) |
| Linux | **`timerfd_create()`** (§23.8, tr. 505) | nano-giây | Timer thành **fd đọc được** → **vào thẳng `epoll`** |

**Các clock chính** (Table 23-1, tr. 493):

| Clock | Đặc điểm | Dùng khi |
|---|---|---|
| `CLOCK_REALTIME` | Giờ thực; **NHẢY được** khi NTP/admin chỉnh giờ | Cần giờ theo lịch |
| **`CLOCK_MONOTONIC`** | Đơn điệu tăng từ một mốc bất kỳ; **không bao giờ nhảy lùi** | **Đo khoảng thời gian, timeout** |
| `CLOCK_PROCESS_CPUTIME_ID` | Thời gian CPU của process | Profiling |
| `CLOCK_THREAD_CPUTIME_ID` | Thời gian CPU của thread | Profiling |

> ⚠️ **Bẫy hay hỏi:** đo elapsed time bằng `CLOCK_REALTIME` là sai — NTP chỉnh giờ lùi 1 giây thì phép đo ra **số âm**, và timeout có thể chờ thêm cả tiếng. **Luôn dùng `CLOCK_MONOTONIC`** cho khoảng thời gian. 🆕 Đây đúng là lý do `std::chrono::steady_clock` tồn tại song song với `system_clock` trong C++ — và là câu hỏi C++ hay được hỏi kèm.

**`sleep()` và signal:** `sleep()`/`nanosleep()` nằm trong nhóm **không bao giờ restart** (§3.3). Bị signal cắt thì `nanosleep()` trả `EINTR` **và ghi thời gian còn lại** vào tham số `remain` — muốn ngủ đủ phải tự lặp:

```c
struct timespec req = { .tv_sec = 5, .tv_nsec = 0 }, rem;
while (nanosleep(&req, &rem) == -1 && errno == EINTR)
    req = rem;                  // ngủ tiếp phần CÒN LẠI
```

🆕 `clock_nanosleep()` với cờ `TIMER_ABSTIME` sạch hơn cho vòng lặp định kỳ: ngủ tới một **thời điểm tuyệt đối**, nên sai số không tích luỹ qua các chu kỳ — đúng thứ cần cho vòng điều khiển realtime ([08/rtos-programming](../../08-embedded-systems/rtos-programming.md)).

---

## 6. Ít quan trọng — 1 dòng + tham chiếu

- **§20.3 — Thay đổi hành vi: `signal()`** (tr. 399): dùng khi đọc code cũ; viết mới thì `sigaction()`.
- **§20.5 — `kill()` và quyền gửi signal** (tr. 402, Figure 20-2): ai được gửi signal cho ai (cùng UID hoặc privileged). `kill(pid, 0)` = **kiểm tra process tồn tại mà không gửi gì**.
- **§20.8 — `raise()`, `killpg()`** (tr. 404): gửi cho chính mình / cho cả process group.
- **§20.13–20.15 — `abort()`, signal trong lỗi phần cứng** (tr. 415): `abort()` sinh `SIGABRT` → core dump.
- **§21.4 — Handler kết thúc bằng cách khác `return`**: `longjmp()` từ handler (nguy hiểm — dùng `sigsetjmp`/`siglongjmp` để khôi phục cả mask), hoặc `_exit()`.
- **§22.1–22.4 — Signal lõi & thứ tự giao** (tr. 448, Figure 22-1): thứ tự khi nhiều signal cùng bỏ chặn là **không xác định** (Linux: theo số hiệu tăng dần, nhưng đừng phụ thuộc).
- **§22.5 — `SA_SIGINFO` và `siginfo_t`** (tr. 455): handler nhận thêm ngữ cảnh — ai gửi (`si_pid`), vì sao (`si_code`), địa chỉ lỗi (`si_addr` cho `SIGSEGV`). Cần khi viết crash handler.
- **§23.4–23.6 — `timer_create()` chi tiết, `SIGEV_THREAD`** (tr. 491–504): tra khi thật sự cần nhiều timer POSIX.
- **§23.9 — `RLIMIT_CPU`, `getrusage()`** (tr. 510).

---

## 7. Góc interview 🎯

<details><summary>1) Async-signal-safe là gì? Vì sao <code>printf()</code> trong signal handler là bug?</summary>

**Async-signal-safe** = hàm được đảm bảo an toàn khi gọi từ signal handler. Một hàm an toàn **hoặc** vì nó **reentrant** (không dùng trạng thái chia sẻ), **hoặc** vì nó không thể bị signal handler cắt ngang.

**Vì sao cần khái niệm này:** signal handler cắt ngang luồng chính **ở một điểm bất kỳ** — kể cả *giữa chừng* một hàm thư viện. Nếu handler gọi lại chính hàm đó, hai lần gọi sẽ giẫm lên trạng thái chung của nhau.

**Hai nguồn gây không an toàn:**
1. **Trạng thái tĩnh dùng chung** — `crypt()`, `strtok()`, `getpwnam()`, `asctime()`. TLPI có chương trình chứng minh (Listing 21-3): main gọi `crypt()` liên tục, handler cũng gọi `crypt()`; kết quả `strcmp` **sai** vì handler ghi đè lên buffer tĩnh của `crypt()`.
2. **Cấu trúc dữ liệu toàn cục cập nhật nhiều bước** — điển hình là **`malloc`/`free`** thao tác trên free-list của heap. Handler gọi `malloc` khi main đang giữa `malloc` → **hỏng heap**.

**`printf()` phạm cả hai:** stdio dùng **buffer chung** và (trong bản đa luồng) **khoá chung**. Hậu quả có thể là output lẫn lộn, buffer hỏng, hoặc **deadlock** — main đang giữ khoá stdio thì bị cắt ngang, handler đòi đúng khoá đó, mà main không thể chạy tiếp để nhả khoá.

Nó "chạy được" 99% lần chỉ vì khe hở rất hẹp — đó là bug tệ nhất: **hiếm, không tái hiện được, và nổ lúc tải cao**.

**Cách viết đúng:**
```c
volatile sig_atomic_t gotSignal = 0;
static void handler(int sig) {
    int saved = errno;                       // ③ lưu errno
    write(STDERR_FILENO, "caught\n", 7);     // ✅ write() an toàn, printf() thì không
    gotSignal = 1;                            // ① chỉ đặt cờ
    errno = saved;                            // ③ khôi phục
}
```
Ba quy tắc: **handler chỉ đặt cờ**, biến chia sẻ là **`volatile sig_atomic_t`**, và **lưu/khôi phục `errno`** (vì syscall trong handler làm hỏng `errno` mà main đang định đọc).
</details>

<details><summary>2) <code>EINTR</code> là gì? <code>SA_RESTART</code> có giải quyết hết không?</summary>

**`EINTR`** = syscall đang **chặn** thì bị signal handler cắt ngang, nên trả về `-1` với `errno == EINTR`. **Không phải lỗi thật** — chỉ là "bị ngắt, chưa làm xong". Code coi mọi `-1` là lỗi sẽ **chết ngẫu nhiên** khi có signal tới đúng lúc.

Hai cách xử lý:
```c
/* Cách 1 — tự lặp */
while ((n = read(fd, buf, len)) == -1 && errno == EINTR)
    continue;
/* glibc: TEMP_FAILURE_RETRY(n = read(fd, buf, len)); */

/* Cách 2 — cờ theo từng signal */
sa.sa_flags = SA_RESTART;
```

**`SA_RESTART` KHÔNG giải quyết hết** — đây là ý phân loại. Danh sách **không bao giờ** được restart, dù đặt cờ:
- **`select()`, `pselect()`, `poll()`, `ppoll()`**
- **`epoll_wait()`, `epoll_pwait()`**
- **`sleep()`, `nanosleep()`, `clock_nanosleep()`**
- **`pause()`, `sigsuspend()`, `sigwaitinfo()`**
- `semop()`, `msgrcv()`, `msgsnd()` (System V IPC), `read()` từ fd inotify

⇒ **Vòng lặp `EINTR` quanh `epoll_wait`/`select`/`poll` là bắt buộc**, không có cờ thay thế.

Hai chi tiết ăn điểm thêm:
- `read()`/`write()` chỉ restart trên **"thiết bị chậm"** (terminal, pipe, FIFO, socket). **File trên đĩa không thuộc nhóm này** → đó là lý do bug không lộ khi test với file, chỉ lộ khi chạy với socket thật.
- Nếu `read()`/`write()` **đã chuyển được một phần** rồi mới bị cắt, nó **không** restart mà **trả về thành công với số byte đã chuyển** — thêm một nguyên nhân của partial read/write.
</details>

<details><summary>3) Vì sao <code>SIGKILL</code> và <code>SIGSTOP</code> không thể bắt hay chặn?</summary>

Đây là **quyết định thiết kế có chủ đích**: hệ điều hành phải **luôn** còn một cách cuối cùng để kết thúc hoặc dừng một process, kể cả khi process đó bị lỗi, treo, hoặc cố tình chống lại. Nếu bắt được `SIGKILL`, một chương trình chỉ cần cài handler rỗng là thành bất tử.

**Hệ quả thực tế** — process bị `SIGKILL` **không có cơ hội dọn dẹp**: không xoá file tạm, không flush buffer (dữ liệu trong buffer stdio và kernel chưa `fsync` có thể mất), không nhả khoá, không đóng kết nối tử tế.

Vì vậy quy ước đúng là **`SIGTERM` trước, `SIGKILL` sau**:
```
kill <pid>          → SIGTERM: "xin hãy tự dọn dẹp rồi thoát"
   ... chờ vài giây ...
kill -9 <pid>       → SIGKILL: chỉ khi nó không chịu chết
```
Đây chính là cách `systemd` (`TimeoutStopSec`) và `docker stop` hoạt động.

Suy ra thiết kế cho daemon: **luôn cài handler `SIGTERM`** để đóng file, `fsync`, nhả khoá, ghi trạng thái — đừng để `SIGKILL` là con đường duy nhất kết thúc chương trình của bạn.

Ghi chú thêm: process trong trạng thái **uninterruptible sleep (D state)** — thường là đang chờ I/O đĩa/NFS — thì `SIGKILL` cũng **không** kết liễu ngay được, phải chờ I/O xong. Đó là lý do đôi khi `kill -9` "không ăn thua".
</details>

<details><summary>4) Signal không xếp hàng gây ra bug gì? Cho một ví dụ thật.</summary>

Tập signal pending là **một bit cho mỗi loại signal**. Nên N lần cùng một signal tới trong lúc nó bị chặn (hoặc trong lúc handler đang chạy) sẽ **gộp thành một** — N−1 cái kia **biến mất không dấu vết**.

**Bug kinh điển: zombie tích tụ.**
```c
/* ❌ SAI — chỉ thu hoạch được MỘT con mỗi lần báo */
static void sigchldHandler(int sig) {
    wait(NULL);
}
```
Mười process con chết gần như đồng thời → chỉ **một** `SIGCHLD` được giao → **9 zombie ở lại vĩnh viễn**, chiếm slot trong bảng process cho tới khi cha chết.

```c
/* ✅ ĐÚNG — vòng lặp cho tới khi hết con đã chết */
static void sigchldHandler(int sig) {
    int saved = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)    // WNOHANG: không chặn khi hết con
        continue;
    errno = saved;                             // waitpid() làm hỏng errno
}
```
`WNOHANG` là mấu chốt: nó khiến `waitpid()` trả `0` ngay khi không còn con nào *đã chết*, thay vì chặn chờ.

**Cách khác:** đặt `SIGCHLD` thành `SIG_IGN`, hoặc dùng `SA_NOCLDWAIT` — kernel tự thu hoạch, không bao giờ có zombie (nhưng cũng không lấy được exit status).

**Giải pháp tổng quát cho vấn đề gộp signal:** dùng **realtime signal** (`SIGRTMIN`+n) — chúng **có xếp hàng**, có thứ tự ưu tiên, và mang được dữ liệu kèm qua `sigqueue()`. Nhưng hàng đợi có trần `RLIMIT_SIGPENDING`; tràn thì lại mất tin.

**Bài học chung:** signal hợp để báo *"có sự kiện loại X"*, **không** hợp để đếm *"có bao nhiêu sự kiện X"*. Cần đếm thì dùng cơ chế khác (pipe, socket, `signalfd` + hàng đợi riêng).
</details>

<details><summary>5) Có race condition giữa chương trình và chính signal handler của nó không? Chữa thế nào?</summary>

**Có.** TLPI gọi đúng tên: *"the main program is racing against its own signal handler"* (§22.9).

Kịch bản: chặn signal để bảo vệ vùng tới hạn, xong thì bỏ chặn rồi ngủ chờ signal:
```c
/* ❌ CÓ BUG */
sigprocmask(SIG_BLOCK, &intMask, &prevMask);
    /* vùng tới hạn */
sigprocmask(SIG_SETMASK, &prevMask, NULL);   // bỏ chặn
                                             // ⚠️ SIGINT tới ĐÚNG KHE HỞ này:
                                             //    handler chạy xong TRƯỚC khi tới pause()
pause();                                     // → chờ signal THỨ HAI, treo vĩnh viễn
```
Signal có thể đã được *sinh ra* bất cứ lúc nào trong vùng tới hạn và chỉ được *giao* khi bỏ chặn — tức là ngay tại khe hở giữa hai lệnh.

**Chữa: `sigsuspend()`** — làm **nguyên tử** hai việc "đặt lại signal mask" và "đình chỉ process":
```c
/* ✅ ĐÚNG */
sigprocmask(SIG_BLOCK, &intMask, &prevMask);
    /* vùng tới hạn */
sigsuspend(&prevMask);        // đặt mask VÀ ngủ, không có khe hở giữa hai việc
```

Đây **cùng một mẫu tư duy** với `O_APPEND` và `O_CREAT|O_EXCL`: *hai thao tác không được để ai chen vào giữa thì phải tìm một syscall làm gộp cả hai*. Cùng mẫu đó sinh ra `pselect()`, `ppoll()`, `epoll_pwait()` (nhận `sigmask` nguyên tử) và `accept4(SOCK_CLOEXEC)`.

**Cách hiện đại hơn, tránh hẳn handler:**
- **`sigwaitinfo()`/`sigtimedwait()`** — chặn signal rồi *chờ lấy nó một cách đồng bộ*, không cần handler → thoát hẳn ràng buộc async-signal-safe.
- **`signalfd()`** — biến signal thành **fd đọc được**, đưa thẳng vào `epoll` cùng mọi fd khác. Đây là kiến trúc của daemon hiện đại: một event loop duy nhất xử lý cả I/O lẫn signal.
- **self-pipe trick** — cách cổ điển tương đương: handler chỉ `write()` một byte vào pipe, main loop `select` trên pipe đó.
</details>

<details><summary>6) Đo khoảng thời gian nên dùng clock nào? Vì sao?</summary>

**`CLOCK_MONOTONIC`**, không phải `CLOCK_REALTIME`.

| Clock | Đặc điểm |
|---|---|
| `CLOCK_REALTIME` | Giờ thực theo lịch. **NHẢY được** — NTP chỉnh, admin đổi giờ, đổi múi giờ |
| **`CLOCK_MONOTONIC`** | Đếm đơn điệu từ một mốc bất kỳ (thường là lúc boot). **Không bao giờ nhảy lùi** |

**Hỏng thế nào nếu dùng `CLOCK_REALTIME`:**
- NTP chỉnh giờ **lùi** 1 giây giữa hai lần đo → khoảng thời gian tính ra **âm**, hoặc timeout chờ thêm rất lâu.
- Đổi múi giờ / DST → phép đo lệch hàng giờ.
- Trên thiết bị nhúng không có RTC pin, đồng hồ **nhảy vọt** khi NTP đồng bộ lần đầu sau boot — đúng lúc hệ thống đang khởi động các service có timeout.

**Quy tắc:** dùng `CLOCK_REALTIME` khi cần *"lúc mấy giờ"* (ghi log, timestamp file); dùng `CLOCK_MONOTONIC` khi cần *"bao lâu"* (timeout, đo hiệu năng, lập lịch định kỳ).

Cùng nguyên tắc đó trong C++: **`std::chrono::steady_clock`** (≈ monotonic) cho đo khoảng, **`system_clock`** cho thời điểm theo lịch — và `steady_clock` tồn tại chính vì lý do trên.

🆕 Cho vòng lặp định kỳ, `clock_nanosleep()` với cờ **`TIMER_ABSTIME`** tốt hơn `nanosleep()`: ngủ tới một **thời điểm tuyệt đối** nên sai số **không tích luỹ** qua các chu kỳ. Với `nanosleep()` tương đối, mỗi vòng trôi thêm một chút, chạy lâu là lệch hẳn.
</details>

---

## 8. Đọc thêm (tùy chọn)

- Topic repo: [04/processes-signals.md](../../04-linux-system-programming/processes-signals.md).
- Bank: `LNX-*` (signal, async-signal-safe, `EINTR`) trong [linux-sysprog.md](../../14-prep/mock-interview/bank/linux-sysprog.md).
- Nối với `epoll`: [cụm 05 §7 câu 5](05-alternative-io-models.md) — vì sao `epoll_wait` luôn cần vòng lặp `EINTR`.
- Đối chiếu góc bare-metal: [08/interrupts-bare-metal.md](../../08-embedded-systems/interrupts-bare-metal.md) — signal handler với ISR là **cùng một bài toán** (ngắt bất đồng bộ, dữ liệu chia sẻ, `volatile`, giữ handler ngắn).

---
⬅️ [Cụm 01 — File I/O](01-concepts-and-file-io.md) · [README cụm TLPI](README.md) · ➡️ [Cụm 05 — Alternative I/O models](05-alternative-io-models.md)
