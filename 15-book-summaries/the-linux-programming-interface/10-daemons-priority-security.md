# TLPI — Cụm 10: Daemon, Priority, Security & Capabilities (ch. 9–12, 34–40)

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 9–12 (tr. 167–232) và 34–40 (tr. 699–832).
> **Định vị:** phần tra cứu — nhưng **§2 (viết daemon)** và **§3 (realtime scheduling)** là hai mục thực chiến thật sự cho công việc Embedded Linux.

---

## 1. Process credentials (ch. 9, tr. 167–184)

### 1.1. Ba loại UID — và vì sao cần ba

| UID | Là gì | Dùng để |
|---|---|---|
| **Real UID** (`ruid`) | Ai **thật sự** chạy chương trình | Xác định *chủ sở hữu* process |
| **Effective UID** (`euid`) | UID kernel dùng để **kiểm tra quyền** | **Quyết định bạn làm được gì** |
| **Saved set-UID** (`suid`) | Bản lưu để **quay lại** đặc quyền sau khi bỏ tạm | Hạ rồi nâng đặc quyền |

```
   Chạy /usr/bin/passwd (set-UID root) bởi user `gia` (uid 1000):

        ruid = 1000 (gia)        ← ai chạy
        euid = 0    (root)       ← quyền thực tế đang có  ⇒ ghi được /etc/shadow
        suid = 0    (root)       ← bản lưu để quay lại
```

**Mẫu an toàn — hạ đặc quyền khi không cần:**

```c
uid_t orig = geteuid();
seteuid(getuid());          // ① hạ euid xuống real uid — tạm bỏ đặc quyền
    /* ... làm việc KHÔNG cần đặc quyền (đọc file của user, chạy code lạ) ... */
seteuid(orig);              // ② lấy lại — được, nhờ saved set-UID

/* Khi KHÔNG bao giờ cần đặc quyền nữa — BỎ VĨNH VIỄN */
setresuid(getuid(), getuid(), getuid());   // xoá cả saved set-UID
```

> ⚠️ **`seteuid()` bỏ đặc quyền TẠM THỜI — kẻ tấn công chiếm được luồng thực thi vẫn `seteuid(0)` lại được** vì saved set-UID còn nguyên. Muốn bỏ **vĩnh viễn** phải `setresuid()` (hoặc `setuid()` khi đang là root) để **xoá cả ba**. Đây là lỗi bảo mật rất hay gặp.

---

## 2. Daemon (ch. 37, tr. 767–782) 🎯

### 2.1. Daemon là gì

Kerrisk định nghĩa bằng hai tính chất (§37.1, tr. 768): **sống lâu** (thường tạo lúc boot, chạy tới khi tắt máy) và **không có controlling terminal** — nên không nhận được signal từ terminal (`SIGINT`, `SIGTSTP`, `SIGHUP`).

### 2.2. Sáu bước biến process thành daemon

Đây là danh sách cần thuộc — TLPI trình bày ở §37.2 (tr. 768–769):

```c
/* ① fork() rồi CHA THOÁT */
if (fork() != 0) _exit(EXIT_SUCCESS);
/*   Hai lý do (theo sách):
     - shell thấy cha kết thúc → in prompt tiếp, để con chạy nền
     - con CHẮC CHẮN không phải process group leader → điều kiện bắt buộc cho ② */

/* ② setsid() — tạo SESSION mới, cắt đứt controlling terminal */
if (setsid() == -1) return -1;

/* ③ fork() LẦN HAI rồi cha thoát */
if (fork() != 0) _exit(EXIT_SUCCESS);
/*   Để con-cháu KHÔNG phải session leader → theo quy ước System V (Linux theo),
     process không phải session leader thì KHÔNG BAO GIỜ giành lại được
     controlling terminal, kể cả khi mở một thiết bị terminal sau này.
     (Cách thay thế: dùng O_NOCTTY cho mọi open() có thể trỏ tới terminal.) */

/* ④ umask(0) — không để umask thừa hưởng ảnh hưởng quyền file daemon tạo */
umask(0);

/* ⑤ chdir("/") — không giữ thư mục làm việc trên một filesystem
      có thể cần unmount */
chdir("/");

/* ⑥ Đóng mọi fd thừa kế, rồi trỏ 0/1/2 vào /dev/null */
for (int fd = 0; fd < sysconf(_SC_OPEN_MAX); fd++) close(fd);
int fd = open("/dev/null", O_RDWR);      /* thành fd 0 */
dup2(fd, STDOUT_FILENO);                 /* fd 1 */
dup2(fd, STDERR_FILENO);                 /* fd 2 */
```

**Vì sao bước ⑥ quan trọng:** nếu để `stdout`/`stderr` đóng hẳn, hàm thư viện nào đó `open()` một file sẽ **nhận được fd 1** — rồi một `printf` vô tình sẽ **ghi đè lên file đó**. Trỏ vào `/dev/null` là cách chặn triệt để.

> 🆕 **Trên hệ thống dùng systemd, đừng làm hết sáu bước này.** systemd muốn service chạy ở **foreground** (`Type=simple`) và tự lo hết: nó đã tách terminal, quản lý PID, và **thu `stdout`/`stderr` vào journal** — đó là cách log tốt hơn `syslog`. Tự daemon hoá lại khiến systemd mất dấu process. Sáu bước trên vẫn cần khi chạy trên init đơn giản (SysV init, BusyBox init) — tình huống rất phổ biến trên thiết bị nhúng.

### 2.3. Ba việc daemon nào cũng phải làm

```c
/* ① Chỉ MỘT instance — file lock, kernel tự nhả khi process chết */
int fd = open("/var/run/d.pid", O_RDWR|O_CREAT, 0644);
struct flock fl = {.l_type=F_WRLCK, .l_whence=SEEK_SET, .l_start=0, .l_len=0};
if (fcntl(fd, F_SETLK, &fl) == -1) fatal("đã có instance chạy");
/* → xem cụm 06 §4.3: hơn hẳn "tạo file .lock" vì file cờ không tự dọn khi kill -9 */

/* ② SIGHUP = nạp lại cấu hình (quy ước lâu đời — daemon không có terminal
      nên SIGHUP mất nghĩa gốc và được tái sử dụng) */
static volatile sig_atomic_t reload = 0;
static void sighupHandler(int sig) { reload = 1; }   /* chỉ đặt cờ! */

/* ③ SIGTERM = tắt sạch: đóng file, fsync, nhả khoá, ghi trạng thái */
static void sigtermHandler(int sig) { shuttingDown = 1; }
```

**Ghi log:** `openlog()`/`syslog()`/`closelog()` (§37.5, tr. 773) — gửi tới `syslogd`/`journald`, có **facility** (loại dịch vụ) và **level** (`LOG_ERR`, `LOG_WARNING`, `LOG_INFO`, `LOG_DEBUG`).
⚠️ **`syslog()` KHÔNG async-signal-safe** — đừng gọi trong signal handler ([cụm 03 §2.2](03-signals-and-timers.md)).

---

## 3. Lập lịch & ưu tiên (ch. 35, tr. 733–752) 🎯

### 3.1. Nice — chỉ ảnh hưởng process thường

```c
nice(5);                                  // giảm ưu tiên đi 5
setpriority(PRIO_PROCESS, 0, 10);         // đặt giá trị nice tuyệt đối
```

Khoảng **−20 (ưu tiên cao nhất) … +19 (thấp nhất)**, mặc định 0. ⚠️ **Chỉ process đặc quyền mới giảm được nice** (tức tăng ưu tiên) — user thường chỉ tự hạ mình xuống được.

> Nice là **gợi ý tương đối**, không phải đảm bảo. Nó **không** cho bạn realtime.

### 3.2. Chính sách realtime — phần đáng giá cho embedded

| Chính sách | Kiểu | Hành vi |
|---|---|---|
| `SCHED_OTHER` | Thường (CFS) | Mặc định; nice có tác dụng ở đây |
| `SCHED_BATCH`, `SCHED_IDLE` | Thường | Cho việc nền, ưu tiên rất thấp |
| **`SCHED_FIFO`** | **Realtime** | Chạy tới khi **tự nhường / bị chặn / bị preempt bởi ưu tiên CAO hơn**. **Không có time slice** |
| **`SCHED_RR`** | **Realtime** | Như FIFO nhưng **có time slice** giữa các task **cùng mức ưu tiên** |

```c
struct sched_param sp = { .sched_priority = 50 };    // realtime: 1…99, CAO = ưu tiên cao
if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
    errExit("sched_setscheduler");                    // cần CAP_SYS_NICE
```

> ⚠️ **Ưu tiên realtime đảo chiều so với nice:** với nice, **số nhỏ = ưu tiên cao**; với realtime, **số LỚN = ưu tiên cao**. Đây là bẫy hay bị hỏi.
>
> ⚠️ **Process `SCHED_FIFO` bị vòng lặp vô hạn sẽ TREO CẢ MÁY** — nó không bao giờ nhường, và không có time slice để cắt. Linux có van an toàn `sched_rt_runtime_us` (mặc định cho realtime dùng tối đa 950 ms mỗi giây) nhưng đừng dựa vào nó. **Luôn test trên máy có đường vào từ xa.**

**Ba việc đi kèm khi làm realtime trên Linux** 🆕:
1. **`mlockall(MCL_CURRENT | MCL_FUTURE)`** — khoá bộ nhớ, tránh page fault giữa vòng điều khiển ([cụm 08 §1.4](08-memory-mappings-and-shared-libraries.md)).
2. **CPU affinity** — `sched_setaffinity()` ghim task vào một lõi, tránh mất cache khi bị chuyển lõi.
3. **Cấp phát trước mọi thứ** — không `malloc` trong vòng nóng ([cụm 02 §1.3](02-processes-and-exec.md)).

Đo bằng **`cyclictest`**, đọc cột **Max**, không phải Avg — xem [08/rtos-vs-linux.md](../../08-embedded-systems/rtos-vs-linux.md).

---

## 4. Capabilities — thay thế cho "root hoặc không gì cả" (ch. 39, tr. 797–816)

Mô hình cổ điển chỉ có hai mức: **UID 0 làm được mọi thứ**, UID khác thì không. Quá thô — một web server chỉ cần bind port 80 lại phải chạy root với **toàn bộ** quyền.

**Capabilities chia quyền của root thành ~40 mảnh độc lập:**

| Capability | Cho phép |
|---|---|
| `CAP_NET_BIND_SERVICE` | Bind port < 1024 |
| `CAP_NET_ADMIN` | Cấu hình mạng, interface |
| `CAP_NET_RAW` | Raw socket (`ping`) |
| `CAP_SYS_NICE` | Đặt ưu tiên realtime (§3.2) |
| `CAP_SYS_TIME` | Chỉnh đồng hồ hệ thống |
| `CAP_CHOWN` | Đổi chủ sở hữu file |
| `CAP_KILL` | Gửi signal cho process bất kỳ |
| `CAP_SYS_ADMIN` | ⚠️ "thùng rác" — quá nhiều thứ, gần như tương đương root |

```sh
setcap cap_net_bind_service=+ep /usr/local/bin/myserver   # gán vào file
getcap /usr/local/bin/myserver
```
⇒ chương trình bind được port 80 mà **không cần chạy root**, và **không có** bất kỳ quyền root nào khác.

🆕 Đây là nền của **container security** (Docker bỏ bớt capability cho container) và là cách đúng để thay thế set-UID root trong hệ thống hiện đại.

---

## 5. Viết chương trình đặc quyền an toàn (ch. 38, tr. 783–796)

Checklist Kerrisk đưa ra, rút gọn thành những điểm áp dụng được ngay:

| Nguyên tắc | Cụ thể |
|---|---|
| **Đặc quyền tối thiểu** | Dùng **capability** thay set-UID root; hạ đặc quyền sớm nhất có thể; bỏ **vĩnh viễn** (`setresuid`) khi không cần nữa |
| **Không tin đầu vào** | Kiểm tra **mọi** đầu vào: tham số, biến môi trường, đầu vào từ mạng, tên file |
| **Không chạy shell** | ⚠️ **Không dùng `system()`, `popen()`** trong chương trình đặc quyền — chúng gọi shell nên dính chèn lệnh và phụ thuộc `PATH`/`IFS`. Dùng `fork()`+`execve()` với **đường dẫn tuyệt đối** |
| **Xử lý đúng `errno`** | Kiểm tra giá trị trả về của **mọi** syscall — đặc biệt `setuid()`, vì bỏ qua lỗi ở đó nghĩa là **vẫn còn đặc quyền mà tưởng đã bỏ** |
| **Đóng fd thừa** | `O_CLOEXEC` cho mọi fd; nếu không, `exec` chương trình lạ là **rò fd** ([cụm 02 §4.4](02-processes-and-exec.md)) |
| **Chống TOCTOU** | Đừng `access()` rồi `open()`; dùng họ **`*at()`** hoặc mở trước rồi `fstat()` trên fd |
| **Xoá môi trường** | `LD_PRELOAD`, `LD_LIBRARY_PATH`, `IFS`, `PATH` — kernel bỏ qua vài cái với set-UID, nhưng an toàn nhất là **tự dựng lại `environ` sạch** |
| **Không tạo core dump** | Core dump của process đặc quyền có thể chứa dữ liệu nhạy cảm (Linux mặc định đã tắt cho set-UID) |

---

## 6. Ít quan trọng — 1 dòng + tham chiếu

- **Ch. 10 — Time** (tr. 185): `time()`, `gettimeofday()`, `clock_gettime()`; ⚠️ **dùng `CLOCK_MONOTONIC` để đo khoảng thời gian** ([cụm 03 §5](03-signals-and-timers.md)). Chuyển đổi calendar time ↔ broken-down time (`localtime`, `strftime`); ⚠️ `localtime()` không thread-safe → `localtime_r()`.
- **Ch. 11 — System limits** (tr. 211): `sysconf()`, `pathconf()`; hằng số như `_SC_OPEN_MAX`, `_PC_PIPE_BUF`. Quy tắc: **đừng hardcode giới hạn**, hỏi lúc chạy.
- **Ch. 12 — `/proc`** (tr. 223): `/proc/PID/status`, `/proc/PID/fd/`, `/proc/PID/maps`, `/proc/PID/cmdline`. 🆕 **Đây là công cụ debug hạng nhất**: `/proc/PID/maps` cho bản đồ bộ nhớ (đối chiếu [cụm 02 §1.1](02-processes-and-exec.md)), `/proc/PID/fd/` cho biết process đang mở gì (nền của `lsof`).
- **Ch. 34 — Process group, session, job control** (tr. 699): nền để hiểu bước ②③ của daemon; controlling terminal, foreground/background job, `SIGHUP` khi terminal đóng.
- **Ch. 36 — Process resources** (tr. 753): `getrusage()`, `setrlimit()`; giới hạn hay dùng: `RLIMIT_NOFILE` (số fd — **hay phải tăng cho server**), `RLIMIT_CORE`, `RLIMIT_AS`, `RLIMIT_STACK`.
- **Ch. 40 — Login accounting** (tr. 817): `utmp`/`wtmp`/`lastlog`; dữ liệu cho `who`, `last`. Ít dùng.

---

## 7. Góc interview

<details><summary>1) Viết một daemon cần làm những bước nào? Vì sao cần <code>fork()</code> hai lần?</summary>

**Sáu bước:**
1. **`fork()`, cha thoát** — shell thấy tiến trình kết thúc nên in prompt tiếp; và quan trọng hơn, con **chắc chắn không phải process group leader**, điều kiện bắt buộc cho bước 2.
2. **`setsid()`** — tạo **session mới**, tách khỏi **controlling terminal**. Từ đây daemon không nhận `SIGINT`/`SIGHUP` từ terminal nữa. *(Gọi được vì bước 1 đảm bảo ta không phải group leader — `setsid()` thất bại nếu đang là group leader.)*
3. **`fork()` lần hai, cha thoát** — để tiến trình cuối **không phải session leader**. Theo quy ước System V (Linux theo), process không phải session leader thì **không bao giờ giành lại được controlling terminal**, kể cả khi vô tình mở một thiết bị terminal về sau. *(Cách thay thế: dùng `O_NOCTTY` trên mọi `open()` có thể trỏ tới terminal.)*
4. **`umask(0)`** — không để umask thừa hưởng làm sai quyền file daemon tạo.
5. **`chdir("/")`** — không giữ thư mục làm việc trên một filesystem có thể cần unmount.
6. **Đóng mọi fd thừa kế**, rồi trỏ fd 0/1/2 vào `/dev/null`. Nếu để chúng **đóng hẳn**, một hàm thư viện `open()` file sẽ **nhận được fd 1**, và một `printf` vô tình sẽ **ghi đè file đó**.

**Ba việc daemon nào cũng phải làm thêm:** đảm bảo **một instance** bằng `fcntl` file lock (kernel tự nhả khi process chết, hơn hẳn file `.lock` tự chế); **`SIGHUP` = nạp lại cấu hình** (quy ước lâu đời, vì daemon không có terminal nên `SIGHUP` mất nghĩa gốc); **`SIGTERM` = tắt sạch** (đóng file, `fsync`, nhả khoá).

🆕 **Trên systemd thì đừng làm hết sáu bước này** — dùng `Type=simple`, chạy foreground, để systemd lo tách terminal và thu `stdout`/`stderr` vào journal. Tự daemon hoá khiến systemd mất dấu process. Sáu bước trên vẫn cần với SysV init hoặc BusyBox init — rất phổ biến trên thiết bị nhúng.
</details>

<details><summary>2) Real UID, effective UID và saved set-UID khác nhau thế nào?</summary>

- **Real UID** — ai **thật sự** chạy chương trình; xác định *chủ sở hữu* của process.
- **Effective UID** — UID mà **kernel dùng để kiểm tra quyền**; đây là cái **quyết định bạn làm được gì**.
- **Saved set-UID** — bản lưu để **quay lại** đặc quyền sau khi bỏ tạm.

Ví dụ `passwd` (set-UID root) chạy bởi user `gia` (uid 1000): `ruid=1000`, `euid=0`, `suid=0`. Nhờ `euid=0` nó ghi được `/etc/shadow`; nhờ `ruid=1000` hệ thống vẫn biết ai đang chạy nó.

**Mẫu đúng cho chương trình đặc quyền:**
```c
seteuid(getuid());                       // hạ đặc quyền TẠM khi không cần
    /* ... làm việc thường ... */
seteuid(0);                              // lấy lại — được, nhờ saved set-UID

setresuid(getuid(), getuid(), getuid()); // BỎ VĨNH VIỄN: xoá cả ba
```

⚠️ **Lỗi bảo mật hay gặp:** tưởng `seteuid(getuid())` là đã an toàn. Không — **saved set-UID vẫn còn**, nên kẻ tấn công chiếm được luồng thực thi chỉ cần gọi `seteuid(0)` là **lấy lại root**. Muốn bỏ hẳn phải `setresuid()` (hoặc `setuid()` khi đang là root) để xoá cả ba.

⚠️ **Luôn kiểm tra giá trị trả về của `setuid()`/`seteuid()`** — nó **có thể thất bại** (vd chạm `RLIMIT_NPROC`). Bỏ qua lỗi nghĩa là chương trình **vẫn còn đặc quyền trong khi bạn tưởng đã bỏ**.

🆕 **Cách hiện đại hơn hẳn set-UID root: capabilities.** Thay vì cho toàn bộ quyền root rồi cố bỏ bớt, chỉ cấp đúng mảnh cần: `setcap cap_net_bind_service=+ep ./myserver` cho phép bind port 80 mà **không có** bất kỳ quyền root nào khác.
</details>

<details><summary>3) <code>SCHED_FIFO</code>, <code>SCHED_RR</code> và nice khác nhau ra sao? Rủi ro khi dùng realtime?</summary>

**`nice`** chỉ áp cho `SCHED_OTHER` (CFS — chính sách thường). Khoảng **−20 … +19**, **số nhỏ = ưu tiên cao**. Chỉ là **gợi ý tương đối** cho scheduler, **không** đảm bảo gì về thời gian. Chỉ process đặc quyền mới giảm được nice (tăng ưu tiên).

**Chính sách realtime** — process realtime **luôn** được ưu tiên hơn **mọi** process thường:
- **`SCHED_FIFO`** — chạy tới khi **tự nhường, bị chặn, hoặc bị preempt bởi task ưu tiên CAO hơn**. **Không có time slice.**
- **`SCHED_RR`** — như FIFO nhưng **có time slice** giữa các task **cùng mức ưu tiên** (round-robin).

Ưu tiên realtime **1…99**, và ⚠️ **số LỚN = ưu tiên cao** — **ngược với nice**. Đây là bẫy hay bị hỏi.

⚠️ **Rủi ro lớn nhất: một process `SCHED_FIFO` bị vòng lặp vô hạn sẽ TREO CẢ MÁY.** Nó không bao giờ tự nhường, không có time slice để cắt, và ưu tiên cao hơn mọi thứ kể cả shell của bạn. Linux có van an toàn `sched_rt_runtime_us` (mặc định giới hạn realtime ở 950 ms mỗi giây) nhưng **đừng dựa vào nó** — luôn test trên máy có đường vào từ xa.

**Ba việc phải làm kèm khi làm realtime trên Linux:**
1. **`mlockall(MCL_CURRENT|MCL_FUTURE)`** — khoá bộ nhớ trong RAM; một page fault giữa vòng điều khiển tốn hàng mili-giây, đủ trượt deadline.
2. **CPU affinity** (`sched_setaffinity`) — ghim vào một lõi, tránh mất cache khi bị chuyển lõi.
3. **Cấp phát trước mọi thứ** — không `malloc` trong vòng nóng (không tất định, có thể gọi `brk`/`mmap`).

Và cần **`CAP_SYS_NICE`** để đặt được chính sách realtime. Đo kết quả bằng **`cyclictest`**, đọc cột **Max** chứ không phải Avg.
</details>

<details><summary>4) Capabilities là gì? Tốt hơn set-UID root ở chỗ nào?</summary>

Mô hình quyền cổ điển của Unix chỉ có **hai mức**: UID 0 làm được **mọi thứ**, UID khác thì không. Quá thô — một web server chỉ cần **bind port 80** lại buộc phải chạy root với **toàn bộ** quyền: đọc mọi file, giết mọi process, nạp module kernel…

**Capabilities chia quyền của root thành ~40 mảnh độc lập**, cấp riêng từng cái:

| Capability | Cho phép |
|---|---|
| `CAP_NET_BIND_SERVICE` | Bind port < 1024 |
| `CAP_NET_ADMIN` | Cấu hình mạng |
| `CAP_SYS_NICE` | Đặt ưu tiên realtime |
| `CAP_SYS_TIME` | Chỉnh đồng hồ hệ thống |
| `CAP_CHOWN` | Đổi chủ sở hữu file |

```sh
setcap cap_net_bind_service=+ep /usr/local/bin/myserver
getcap /usr/local/bin/myserver
```
⇒ server bind được port 80 **mà không có bất kỳ quyền root nào khác**. Nếu bị khai thác, kẻ tấn công **chỉ** có đúng mảnh quyền đó.

**Ba ưu điểm so với set-UID root:**
1. **Đặc quyền tối thiểu thật sự** — thu hẹp bề mặt tấn công từ "toàn bộ hệ thống" xuống một thao tác.
2. **Không cần code hạ đặc quyền** — không còn `seteuid`/`setresuid` để viết sai.
3. **Kiểm tra được** — `getcap` cho thấy chính xác chương trình có quyền gì; với set-UID bạn phải **đọc code** mới biết.

⚠️ **Cảnh báo:** `CAP_SYS_ADMIN` là "thùng rác" — nó gom quá nhiều thứ nên **gần như tương đương root**. Cấp nó là gần như không được lợi ích bảo mật nào.

🆕 Đây là nền của **container security** — Docker mặc định bỏ bớt phần lớn capability cho container, và đó là lý do container "chạy root" vẫn an toàn hơn root thật trên host.
</details>

---

## 8. Đọc thêm (tùy chọn)

- Bank: `LNX-*`, `BSP-*` (systemd, khởi động dịch vụ) trong [linux-sysprog.md](../../14-prep/mock-interview/bank/linux-sysprog.md) · [bsp.md](../../14-prep/mock-interview/bank/bsp.md).
- Realtime trên Linux (PREEMPT_RT, `cyclictest`): [08/rtos-vs-linux.md](../../08-embedded-systems/rtos-vs-linux.md) và [MELP debug & realtime](../melp/debug-realtime.md).
- init/systemd trên thiết bị nhúng: [MELP bootloader & kernel](../melp/bootloader-kernel.md).
- File lock cho single-instance: [cụm 06 §4.3](06-ipc.md).
- `mlockall`, `mprotect`: [cụm 08 §1.4](08-memory-mappings-and-shared-libraries.md).

---
⬅️ [Cụm 09 — File systems](09-filesystems-and-file-attributes.md) · [README cụm TLPI](README.md)
