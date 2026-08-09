# TLPI — Cụm 02: Process & Program Execution (ch. 6, 7, 24–28) 🎯

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 6–7 (tr. 113–152) và 24–28 (tr. 513–616).
> **Vì sao cụm này 🎯:** `fork`/`exec`/`wait` là bộ ba nền tảng của mọi thứ chạy trên Unix — shell, daemon, container, `system()`. Và câu *"cái gì được kế thừa qua `fork`, cái gì sống sót qua `exec`?"* là câu phân loại rất tốt vì nó buộc bạn hiểu **process gồm những gì**.

---

## 1. Một process gồm những gì (ch. 6, tr. 113–138)

### 1.1. Bản đồ bộ nhớ — hình phải vẽ được

```
   Địa chỉ ảo (x86-32)
   0xFFFFFFFF ┌────────────────────────────────┐
              │  KERNEL                        │  ánh xạ vào không gian ảo của
              │  (process KHÔNG truy cập được) │  process nhưng bị chặn truy cập
   0xC0000000 ├────────────────────────────────┤
              │  argv, environ                 │  ← tham số dòng lệnh + biến môi trường
              ├────────────────────────────────┤
              │  STACK    (mọc XUỐNG ▼)        │  ← frame mỗi lần gọi hàm:
              │                                │    biến cục bộ, tham số, địa chỉ trả về
              ├╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┤
              │      (vùng chưa cấp phát)      │
              ├╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┤  ← program break (&end)
              │  HEAP     (mọc LÊN ▲)          │    malloc() cấp từ đây
              ├────────────────────────────────┤  &end
              │  BSS — dữ liệu CHƯA khởi tạo   │  ← zero khi nạp; KHÔNG tốn dung lượng file
              ├────────────────────────────────┤  &edata
              │  DATA — dữ liệu ĐÃ khởi tạo    │  ← đọc từ file thực thi
              ├────────────────────────────────┤  &etext
              │  TEXT — mã lệnh (chỉ đọc)      │  ← chia sẻ được giữa nhiều process
   0x08048000 └────────────────────────────────┘
   0x00000000
```

*(vẽ lại theo Figure 6-1, tr. 119)*

**Ba điều đọc ra ngay từ hình:**

1. **`.bss` không tốn dung lượng file thực thi** — chỉ ghi *kích thước cần zero*. `int buf[1000000];` làm file to thêm 0 byte, nhưng chiếm 4 MB RAM lúc chạy. So sánh: `int buf[1000000] = {1};` vào `.data` → file to thêm 4 MB.
2. **Text là chỉ đọc và chia sẻ được** — chạy 100 instance của cùng một chương trình chỉ tốn **một** bản mã lệnh trong RAM.
3. **Stack và heap mọc ngược chiều nhau** vào cùng khoảng trống ở giữa — thiết kế để hai bên linh hoạt chia nhau vùng chưa dùng.

> 🆕 Hình này **giống hệt** bản đồ bộ nhớ MCU ở [08/memory-and-startup.md](../../08-embedded-systems/memory-and-startup.md), trừ một điều quyết định: ở đây có **MMU** nên mỗi process có không gian ảo **riêng**, và tràn stack thì đụng vùng chưa map → **SIGSEGV ngay tại chỗ**. Trên MCU không MMU thì stack đè thẳng lên `.bss` — hỏng âm thầm. Cùng bản đồ, khác hẳn hệ quả khi sai.

### 1.2. Virtual memory — vì sao `fork()` rẻ

Kerrisk giải thích cơ sở của virtual memory là **locality of reference** (§6.4, tr. 118): *spatial locality* (truy cập gần chỗ vừa truy cập) và *temporal locality* (truy cập lại chỗ vừa truy cập). Hệ quả: **chỉ cần giữ một phần address space trong RAM** là chương trình vẫn chạy được.

Cơ chế: chia address space thành **page** cố định, kernel giữ **page table** ánh xạ page ảo → frame vật lý. Đây là nền để hiểu copy-on-write ở §2.2.

### 1.3. Cấp phát bộ nhớ (ch. 7, tr. 139–152)

`malloc()` **không** phải syscall. Nó là hàm thư viện, lấy bộ nhớ theo hai đường:

```c
brk() / sbrk()   // dịch "program break" — mở rộng heap. Dùng cho khối NHỎ
mmap()           // ánh xạ vùng mới. glibc dùng cho khối LỚN (mặc định ≥128 KB)
```

**Vì sao `free()` thường KHÔNG trả RAM về hệ điều hành** (§7.1.2, tr. 145): `free()` chỉ đưa khối vào **free list** của allocator để tái sử dụng. Program break chỉ hạ xuống nếu khối được giải phóng nằm **sát đỉnh heap**. Khối cấp bằng `mmap()` thì `free()` **có** trả lại ngay (`munmap`).

> ⚠️ **Đây là lý do `top`/`ps` báo RSS không giảm sau khi bạn `free()` hàng loạt** — và là lý do người ta hay tưởng nhầm là memory leak. 🆕 Cùng cơ chế này gây **heap fragmentation** — vấn đề nghiêm trọng trên thiết bị nhúng chạy nhiều tháng, xem [08/constraints.md](../../08-embedded-systems/constraints.md).

---

## 2. `fork()` — nhân đôi process (ch. 24, tr. 513–530) 🎯

### 2.1. Một lời gọi, hai lần trả về

```c
pid_t pid = fork();
switch (pid) {
case -1:
    errExit("fork");                 // thất bại
case 0:
    /* ===== CON ===== */            // fork() trả về 0 trong con
    printf("con: pid=%ld, cha=%ld\n", (long) getpid(), (long) getppid());
    _exit(EXIT_SUCCESS);             // ⚠️ _exit() chứ không phải exit() — xem §3.2
default:
    /* ===== CHA ===== */            // fork() trả về PID CỦA CON trong cha
    printf("cha: con là %ld\n", (long) pid);
    wait(NULL);
    break;
}
```

**Vì sao trả về khác nhau:** con luôn biết cha mình qua `getppid()`, nên không cần trả PID cha; ngược lại cha **cần** biết PID con để `wait()` đúng đứa. Trả `0` cho con vì `0` không bao giờ là PID hợp lệ.

> ⚠️ **Thứ tự chạy sau `fork()` là KHÔNG XÁC ĐỊNH.** Cha hay con chạy trước phụ thuộc scheduler. Code phụ thuộc thứ tự là bug — cần thứ tự thì phải đồng bộ tường minh (signal, pipe, semaphore).

### 2.2. Copy-on-write — vì sao nhân đôi cả process lại nhanh

Nếu `fork()` thực sự chép toàn bộ bộ nhớ thì nó vô dụng — nhất là khi con `exec()` ngay sau đó, mọi công chép đều **vứt đi**. Kernel dùng **copy-on-write** (§24.2.2, tr. 522):

```
   NGAY SAU fork() — không chép gì cả
   ┌──────────┐                        ┌──────────┐
   │  CHA     │──┐                  ┌──│  CON     │
   │ page tbl │  │                  │  │ page tbl │
   └──────────┘  ▼                  ▼  └──────────┘
              ┌────────────────────────┐
              │  TRANG VẬT LÝ dùng CHUNG│  ← đánh dấu CHỈ ĐỌC + cờ COW
              │       "AAAA"           │
              └────────────────────────┘

   KHI MỘT BÊN GHI — lúc đó mới chép, và chỉ chép ĐÚNG TRANG BỊ GHI
   ┌──────────┐                        ┌──────────┐
   │  CHA     │──┐                  ┌──│  CON     │
   └──────────┘  ▼                  ▼  └──────────┘
        ┌────────────────┐   ┌────────────────┐
        │  "AAAA" (gốc)  │   │  "BBBB" (bản   │  ← page fault → kernel chép trang
        └────────────────┘   │   chép riêng)  │     rồi cho ghi
                             └────────────────┘
```

*(vẽ lại theo Figure 24-3, tr. 525)*

**Cơ chế:** cả hai page table trỏ vào cùng trang vật lý, đánh dấu **chỉ đọc**. Bên nào ghi thì sinh **page fault**; kernel bắt được, chép trang đó ra bản riêng, đổi quyền thành ghi được, rồi cho lệnh ghi chạy tiếp. **Chỉ những trang thực sự bị ghi mới bị chép.**

### 2.3. `fork()` vs `vfork()` vs `clone()` — số đo thật của sách 🎯

Table 28-3 (tr. 609–610) — tạo **100 000 process**, đo bằng chương trình thật:

| Cách tạo | Process 1,70 MB | Process 2,70 MB | Process **11,70 MB** |
|---|---|---|---|
| `fork()` | 22,27 s | 26,38 s | **126,93 s** |
| `vfork()` | 3,52 s | 3,55 s | **3,53 s** |
| `clone()` | 2,97 s | 2,98 s | **2,93 s** |
| `fork()` + `exec()` | 135,72 s | 146,15 s | **260,34 s** |
| `vfork()` + `exec()` | 107,36 s | 107,81 s | **107,97 s** |

**Ba kết luận đắt giá đọc ra từ bảng:**

1. **`fork()` vẫn tăng theo kích thước process** dù có copy-on-write: 1,7 MB → 11,7 MB làm nó chậm **5,7 lần** (22 → 127 giây). Vì COW không miễn phí — kernel vẫn phải **chép page table** và đánh dấu mọi trang là chỉ đọc. Càng nhiều trang, càng tốn.
2. **`vfork()` và `clone()` gần như PHẲNG** theo kích thước (3,52 → 3,53 giây) — chúng không chép page table.
3. **`exec()` mới là phần đắt nhất:** `fork+exec` là 135 giây so với `fork` đơn 22 giây — tức `exec()` chiếm ~83% chi phí. Kerrisk còn ghi chú bảng này **chưa lộ hết** chi phí `exec()` (chưa tính công của dynamic linker).

> 🆕 Đây là căn cứ định lượng cho **kiến trúc pre-fork**: nginx/Apache tạo sẵn worker lúc khởi động thay vì `fork` mỗi request. Và là lý do process nặng (đã `malloc` nhiều) **không nên `fork`** — nếu buộc phải, hãy `fork` sớm khi còn nhẹ, hoặc dùng `posix_spawn()`.

**`vfork()` — nhanh nhưng nguy hiểm** (§24.3, tr. 526): con **dùng chung không gian nhớ với cha** và **cha bị treo** cho tới khi con `exec()` hoặc `_exit()`. Ràng buộc ngặt: giữa `vfork()` và `exec()`, con **không được** gán biến nào ngoài `pid_t` nhận kết quả, **không được return** khỏi hàm gọi `vfork()`, **không được** gọi hàm khác. Vi phạm → hỏng bộ nhớ của **cha**. Kerrisk khuyên: chỉ dùng khi thật sự cần tốc độ tối đa và con `exec()` ngay.

### 2.4. File descriptor qua `fork()`

```
   TRƯỚC fork()                     SAU fork()
   Cha ─── fd 3 ──┐                 Cha ─── fd 3 ──┐
                  ▼                                 ▼
          ┌──────────────┐                  ┌──────────────┐
          │ file descr.  │                  │ file descr.  │  ← CHUNG MỘT cái
          │ offset = 100 │                  │ offset = 100 │
          └──────────────┘                  └──────────────┘
                                                    ▲
                                 Con ─── fd 3 ──────┘
```

*(theo Figure 24-2, tr. 521)*

**Con nhận bản sao của bảng fd, nhưng các bản sao trỏ vào CÙNG file description** → **chung offset và chung status flags**. Cha đọc 50 byte thì offset của con cũng nhảy sang 50.

🆕 Đây vừa là tính năng (shell pipeline dựa vào nó) vừa là bẫy — hai process cùng ghi log mà không có `O_APPEND` sẽ đè nhau. Nối thẳng với ba bảng ở [cụm 01 §2.3](01-concepts-and-file-io.md).

---

## 3. Kết thúc process & thu hoạch con (ch. 25–26, tr. 531–562)

### 3.1. `exit()` vs `_exit()` — khác biệt gây bug thật

| | `_exit()` (syscall) | `exit()` (hàm thư viện) |
|---|---|---|
| Gọi handler `atexit()`/`on_exit()` | ❌ | ✅ |
| **Xả buffer stdio** | ❌ | ✅ |
| Đóng stream stdio | ❌ | ✅ |
| Kết thúc process | ✅ | ✅ (gọi `_exit()` ở cuối) |

> ⚠️ **Vì sao con nên dùng `_exit()`:** sau `fork()`, con có **bản sao buffer stdio của cha**. Nếu con gọi `exit()`, nó **xả bản sao đó** → dữ liệu cha đã in mà chưa flush bị **in RA HAI LẦN** (một lần bởi con, một lần bởi cha). Bug kinh điển:

```c
printf("xin chào\n");        // vào buffer stdio; nếu stdout KHÔNG phải tty
                             // (vd `./prog > f.txt`) thì fully-buffered, chưa xả
if (fork() == 0) {
    exit(0);                 // ❌ con XẢ bản sao buffer  → "xin chào" lần 1
}
wait(NULL);
exit(0);                     //    cha xả buffer của mình → "xin chào" lần 2
```

Chạy ra terminal thì **không thấy bug** (line-buffered, đã xả trước `fork`); ghi ra file thì thấy. **Hai cách chữa:** `fflush(NULL)` **trước** `fork()`, hoặc con dùng `_exit()`.

### 3.2. Zombie và orphan

```
   ZOMBIE                                    ORPHAN
   Con CHẾT, cha CHƯA wait()                 Cha chết TRƯỚC con
   ┌────────┐        ┌────────┐              ┌────────┐   ┌────────┐
   │  Cha   │        │  Con   │              │  Cha   │   │  Con   │
   │(chưa   │        │ ĐÃ CHẾT│              │ ĐÃ CHẾT│   │ còn    │
   │ wait)  │        │ nhưng  │              └────────┘   │ sống   │
   └────────┘        │ entry  │                           └───┬────┘
                     │ CÒN    │                               │ nhận cha mới
                     └────────┘                          ┌────▼─────┐
   → chiếm slot bảng process                             │ init (1) │ (hoặc
     cho tới khi cha wait() hoặc chết                     │          │  subreaper)
                                                          └──────────┘
                                                     → init tự wait() → KHÔNG zombie
```

**Zombie nguy hiểm ở chỗ:** không giết được (`kill -9` vô nghĩa — nó đã chết rồi), chỉ biến mất khi cha `wait()` hoặc cha chết. Daemon chạy lâu mà tạo con liên tục không `wait()` sẽ **cạn bảng process của toàn hệ thống**.

**Thu hoạch đúng cách:**

```c
/* Cách 1 — chặn chờ */
pid_t pid = wait(&status);          // chờ BẤT KỲ con nào chết

/* Cách 2 — không chặn, dùng trong handler SIGCHLD */
static void sigchldHandler(int sig) {
    int saved = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)   // ⚠️ VÒNG LẶP — signal không xếp hàng!
        continue;                             //    (xem cụm 03 §1.2)
    errno = saved;
}

/* Cách 3 — bảo kernel tự thu hoạch (không lấy được exit status) */
signal(SIGCHLD, SIG_IGN);
```

**Đọc `status`** bằng macro, không tự bóc bit:

```c
if (WIFEXITED(status))        printf("thoát bình thường, mã %d\n", WEXITSTATUS(status));
else if (WIFSIGNALED(status)) printf("bị giết bởi signal %d\n",   WTERMSIG(status));
else if (WIFSTOPPED(status))  printf("bị dừng bởi signal %d\n",   WSTOPSIG(status));
```

---

## 4. `exec()` — thay ruột process (ch. 27, tr. 563–590) 🎯

### 4.1. `exec()` KHÔNG tạo process mới

Điểm dễ hiểu sai nhất: `exec()` **thay toàn bộ nội dung** của process hiện tại bằng chương trình mới — **PID không đổi**, process không mới. Text, data, bss, heap, stack đều bị **vứt và dựng lại**.

**Thành công thì `exec()` KHÔNG BAO GIỜ trả về** — vì mã lệnh gọi nó đã bị ghi đè. Có return nghĩa là **đã lỗi**:

```c
execl("/bin/ls", "ls", "-l", (char *) NULL);
errExit("execl");        // ← chỉ chạy tới đây khi execl() THẤT BẠI
```

### 4.2. Sáu biến thể — giải mã hậu tố

| Hậu tố | Nghĩa |
|---|---|
| **`l`** — *list* | Tham số truyền **rời**, kết bằng `(char *) NULL`: `execl(path, "ls", "-l", NULL)` |
| **`v`** — *vector* | Tham số truyền bằng **mảng** `char *argv[]` |
| **`p`** — *path* | Tìm chương trình theo biến môi trường **`PATH`** (nên nhận *tên* thay vì đường dẫn đầy đủ) |
| **`e`** — *environment* | Truyền **môi trường riêng** thay vì kế thừa `environ` |

⇒ `execl`, `execlp`, `execle`, `execv`, `execvp`, `execve`. **Chỉ `execve()` là syscall thật**, năm cái kia là wrapper thư viện gọi vào nó *(Table 27-1, tr. 581)*.

> ⚠️ **Bảo mật:** biến thể có `p` tìm theo `PATH` — nếu `PATH` chứa `.` hoặc thư mục ghi được bởi người khác, kẻ tấn công đặt file trùng tên vào đó là bạn chạy code của họ. **Chương trình đặc quyền phải dùng đường dẫn tuyệt đối** và biến thể **không** có `p`.

### 4.3. Cái gì sống sót qua `exec()` — bảng phải thuộc 🎯

| Sống sót ✅ | Bị vứt ❌ |
|---|---|
| **PID, PPID**, process group, session | Toàn bộ **text/data/bss/heap/stack** |
| **File descriptor đang mở** (trừ `FD_CLOEXEC`) | **Signal handler** (vì code handler không còn) |
| **Offset và status flags** của file | Vùng nhớ `mmap` |
| **Signal mask** và tập signal pending | Khoá do `pthread` giữ; mọi thread khác |
| UID/GID hiệu lực (trừ khi set-UID) | Bộ đếm timer `alarm()`… (tuỳ loại) |
| Thư mục làm việc, umask, giới hạn tài nguyên | |

**Điểm tinh dễ mất điểm:** **signal *handler* bị vứt nhưng signal *mask* thì không**, và **signal đang bị `SIG_IGN` vẫn tiếp tục bị bỏ qua**. Nghĩa là chương trình mới thừa hưởng một signal mask mà nó không hề biết → có thể "điếc" với signal mà nó tưởng mình đang nhận. Chương trình đặc quyền nên **reset signal mask** sau `exec`.

### 4.4. `FD_CLOEXEC` — vì sao nó tồn tại

Kerrisk nêu chính xác hai lý do (§27.4, tr. 575–576):

> 1. *"The file descriptor may have been opened by a library function."* Thư viện mở file mà bạn **không biết** — bạn không có cách nào `close()` nó trước `exec()`. Vì vậy **nguyên tắc chung: hàm thư viện phải luôn đặt close-on-exec cho file nó mở.**
> 2. *"If the `exec()` call fails... we may want to keep the file descriptors open."* Nếu tự `close()` trước rồi `exec()` lỗi, bạn **không mở lại được đúng file đó** nữa. `FD_CLOEXEC` chỉ đóng khi `exec()` **thành công**.

```c
/* Cách cũ — hai bước, có race với thread khác đang fork() */
int flags = fcntl(fd, F_GETFD);
flags |= FD_CLOEXEC;
fcntl(fd, F_SETFD, flags);

/* Cách hiện đại — nguyên tử ngay lúc tạo */
fd  = open(path, O_RDONLY | O_CLOEXEC);
fd2 = accept4(sfd, NULL, NULL, SOCK_CLOEXEC);
epfd = epoll_create1(EPOLL_CLOEXEC);
```

> 🆕 **Vì sao phải nguyên tử:** giữa `open()` và `fcntl(F_SETFD)` có một khe hở; nếu thread khác `fork()`+`exec()` đúng lúc đó, fd **rò rỉ** sang chương trình mới. Cùng mẫu race với `sigsuspend` ([cụm 03 §4.2](03-signals-and-timers.md)) và `O_EXCL` ([cụm 01 §2.4](01-concepts-and-file-io.md)) — *hai thao tác phải gộp làm một*.

### 4.5. Ghép lại: shell chạy một lệnh thế nào

```
   shell (PID 100)
      │
      │  fork()
      ├──────────────────────► con (PID 101) — bản sao của shell
      │                             │
      │ wait(&status)               │  ① dọn dẹp: đóng fd không cần,
      │  ⏸ chặn, chờ                │     dup2() để redirect (> < |),
      │                             │     reset signal handler về mặc định
      │                             │
      │                             │  ② execve("/bin/ls", …)
      │                             ├───► ruột process bị THAY, PID VẪN LÀ 101
      │                             │     ls chạy, ghi vào fd 1 (đã bị redirect)
      │                             │
      │                             │  ③ _exit(status)
      │  ◄──────────────────────────┘
      │  wait() trả về → shell đọc exit status, in prompt tiếp
```

*(theo Figure 24-1, tr. 514)*

**Toàn bộ Unix nằm ở hình này.** Việc tách `fork` khỏi `exec` — thay vì một lời gọi "spawn" duy nhất — chính là thứ tạo ra **cửa sổ ở bước ①** để con tự cấu hình mình (redirect, đổi UID, đổi thư mục, đóng fd) **trước khi** chương trình mới chạy. Đó là lý do shell pipeline, `sudo`, container đều làm được mà không cần API đặc biệt nào.

---

## 5. Ít quan trọng — 1 dòng + tham chiếu

- **§6.1–6.3 — PID, PPID, `getpid()`** (tr. 114): PID cấp tuần tự tới `/proc/sys/kernel/pid_max` rồi quay vòng.
- **§6.7 — Biến môi trường, `getenv`/`setenv`/`putenv`** (tr. 125): ⚠️ `putenv()` **không** chép chuỗi — truyền biến cục bộ vào là bug. `setenv()` có chép.
- **§6.8 — `setjmp`/`longjmp`** (tr. 131): nhảy phi cục bộ; nguy hiểm, tránh trừ khi thật cần (dùng `sigsetjmp` nếu liên quan signal).
- **§7.1.3 — `alloca()`** (tr. 150): cấp trên **stack**, tự giải phóng khi hàm return, nhanh hơn `malloc` — nhưng tràn stack thì không phát hiện được.
- **§25.3 — `atexit()`, `on_exit()`** (tr. 536): đăng ký hàm dọn dẹp; chạy **ngược thứ tự đăng ký**; **không** chạy khi `_exit()` hoặc bị signal giết.
- **§26.2 — `waitid()`** (tr. 553): kiểm soát chi tiết hơn `waitpid()`, chờ được cả trạng thái stop/continue.
- **§26.4 — `system()`** (tr. 559): tiện nhưng ⚠️ **không dùng trong chương trình đặc quyền** — nó gọi shell nên dính lỗ hổng chèn lệnh và phụ thuộc `PATH`/`IFS`.
- **§28.1–28.2 — Process accounting** (tr. 592): ghi bản ghi khi process kết thúc; ít dùng.
- **§28.3 — `clone()` chi tiết + Table 28-2 các cờ** (tr. 599): `CLONE_VM`, `CLONE_FILES`, `CLONE_NEWNS`… — đây là **nền của thread (NPTL) và của container**. Đáng đọc nếu quan tâm namespace.

---

## 6. Góc interview 🎯

<details><summary>1) <code>fork()</code> làm gì? Con kế thừa những gì từ cha?</summary>

`fork()` tạo một process con **gần như giống hệt** cha. Đặc biệt ở chỗ **một lời gọi trả về hai lần**: trả `0` trong con, trả **PID của con** trong cha, `-1` nếu lỗi. Bất đối xứng vì con luôn tìm được cha qua `getppid()`, còn cha thì cần PID để `wait()` đúng đứa.

**Con kế thừa:** bản sao của toàn bộ không gian nhớ (text/data/bss/heap/stack), **bảng file descriptor**, thư mục làm việc, umask, signal mask và **disposition** của signal, UID/GID, giới hạn tài nguyên, biến môi trường.

**Con KHÔNG kế thừa:** PID (mới), PPID (= PID cha), **khoá file** của cha, timer đang chạy (`alarm()` bị reset), tập signal **pending** (con bắt đầu với tập rỗng), và — quan trọng — **chỉ thread gọi `fork()` mới tồn tại trong con**, mọi thread khác biến mất (mutex chúng đang giữ thì **kẹt vĩnh viễn**).

**Về file descriptor — đây là chỗ ăn điểm:** con nhận *bản sao bảng fd*, nhưng các bản sao **trỏ vào cùng file description** với cha → **chung offset, chung status flags**. Cha đọc 50 byte thì offset của con cũng nhảy. Đây vừa là tính năng (shell pipeline dựa vào nó) vừa là bẫy (hai process ghi cùng log không có `O_APPEND` sẽ đè nhau).

**Về hiệu năng:** `fork()` không chép bộ nhớ ngay mà dùng **copy-on-write** — cả hai trỏ chung trang vật lý đánh dấu chỉ đọc, bên nào ghi thì page fault và kernel chép riêng đúng trang đó. Nhưng **vẫn phải chép page table**, nên `fork()` **vẫn đắt theo kích thước process**: số đo TLPI cho thấy process 11,7 MB `fork` chậm hơn process 1,7 MB **5,7 lần** (127 s vs 22 s cho 100 000 lần).

⚠️ **Thứ tự chạy của cha và con sau `fork()` là không xác định** — phụ thuộc scheduler. Phụ thuộc thứ tự là bug.
</details>

<details><summary>2) Phân biệt <code>fork()</code>, <code>exec()</code>, <code>wait()</code>. Vì sao Unix tách <code>fork</code> khỏi <code>exec</code>?</summary>

- **`fork()`** — tạo **process mới** (nhân đôi process hiện tại). PID mới.
- **`exec()`** — **thay toàn bộ nội dung** process hiện tại bằng chương trình khác. **PID không đổi, không có process mới.** Thành công thì *không bao giờ trả về* — có return nghĩa là đã lỗi.
- **`wait()`** — cha **chờ** con kết thúc và **thu hoạch** exit status, giải phóng entry của con khỏi bảng process.

Chạy một lệnh trong shell là ghép cả ba: `fork()` → (con cấu hình) → `execve()` → cha `wait()`.

**Vì sao tách đôi thay vì một lời gọi "spawn"?** Vì việc tách tạo ra một **cửa sổ giữa `fork` và `exec`**, nơi process con — vẫn còn là bản sao của cha, chạy code của cha — có thể **tự cấu hình môi trường thực thi** trước khi chương trình mới nắm quyền:

```c
if (fork() == 0) {
    int fd = open("out.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    dup2(fd, STDOUT_FILENO);   // ← redirect: `ls > out.txt`
    close(fd);
    setuid(nobody_uid);        // ← hạ đặc quyền
    chdir("/srv");             // ← đổi thư mục
    execvp("ls", args);        // ls chạy trong môi trường ĐÃ được dựng sẵn
    _exit(127);                // chỉ tới đây nếu exec lỗi
}
```

Nhờ cửa sổ này mà **redirect, pipeline, `sudo`, container** đều làm được **không cần API đặc biệt nào** — `ls` không hề biết output của nó bị chuyển hướng.

**Cái giá:** `fork()` đắt khi process lớn (phải chép page table), và với chương trình chỉ định chạy lệnh khác thì đó là công vứt đi. Vì vậy có `vfork()` (nhanh nhưng ràng buộc ngặt) và `posix_spawn()` (gộp fork+exec, hợp cho hệ không có MMU).

**Số đo TLPI:** `fork+exec` mất 135 s cho 100 000 lần, trong khi `fork` đơn chỉ 22 s — **`exec()` chiếm ~83% chi phí**, chứ không phải `fork`.
</details>

<details><summary>3) Zombie process là gì? Orphan là gì? Cách phòng?</summary>

**Zombie:** con **đã chết** nhưng cha **chưa `wait()`**. Kernel giữ lại entry trong bảng process (PID, exit status, thống kê tài nguyên) để cha còn lấy được exit status. Zombie **không dùng CPU, không dùng RAM**, nhưng **chiếm một slot** trong bảng process.

Nguy hiểm ở chỗ: **không giết được** — `kill -9` vô nghĩa vì nó đã chết rồi. Nó chỉ biến mất khi cha `wait()` hoặc **cha chết** (lúc đó `init` nhận nuôi và thu hoạch). Daemon chạy lâu, tạo con liên tục mà không `wait()` sẽ **cạn bảng process của toàn hệ thống**.

**Orphan:** cha chết **trước** con. Con được **`init` (PID 1)** nhận nuôi — hoặc "subreaper" gần nhất nếu có đăng ký `PR_SET_CHILD_SUBREAPER`. `init` luôn `wait()` nên orphan **không** thành zombie. Đây cũng chính là mẹo tạo daemon bằng **double fork**.

**Ba cách phòng zombie:**

```c
/* ① Handler SIGCHLD với VÒNG LẶP — cách đúng nhất */
static void sigchldHandler(int sig) {
    int saved = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)   /* ⚠️ PHẢI là vòng lặp */
        continue;
    errno = saved;                            /* waitpid làm hỏng errno */
}

/* ② Bảo kernel tự thu hoạch — đơn giản nhưng MẤT exit status */
signal(SIGCHLD, SIG_IGN);

/* ③ Double fork — con giữa thoát ngay, cháu thành orphan → init nuôi */
if (fork() == 0) {
    if (fork() == 0) { /* cháu: làm việc thật */ _exit(0); }
    _exit(0);                       /* con giữa chết ngay */
}
wait(NULL);                          /* cha thu hoạch con giữa — nhanh gọn */
```

**Vì sao ① bắt buộc phải là vòng lặp:** signal chuẩn **không xếp hàng**. Mười con chết gần như đồng thời chỉ sinh **một** `SIGCHLD` → `wait()` một lần chỉ thu được một đứa → **9 zombie ở lại**. `WNOHANG` khiến `waitpid` trả `0` ngay khi hết con đã chết, thay vì chặn.
</details>

<details><summary>4) Cái gì sống sót qua <code>exec()</code>? Vì sao cần <code>FD_CLOEXEC</code>?</summary>

**Sống sót:** PID/PPID, process group và session, **file descriptor đang mở** (cùng offset và status flags), **signal mask** và tập signal pending, thư mục làm việc, umask, UID/GID hiệu lực, giới hạn tài nguyên.

**Bị vứt:** toàn bộ text/data/bss/heap/stack, các vùng `mmap`, **signal handler** (code handler không còn tồn tại), mọi thread khác.

**Điểm tinh dễ mất điểm:** signal **handler** bị vứt nhưng signal **mask** thì **không**, và signal đang đặt `SIG_IGN` vẫn tiếp tục bị bỏ qua. Chương trình mới thừa hưởng một mask mà nó **không biết** → có thể "điếc" với signal nó tưởng đang nhận. Chương trình đặc quyền nên **reset signal mask** sau `exec`.

**Vì sao cần `FD_CLOEXEC`** — TLPI nêu đúng hai lý do:
1. **fd có thể do hàm thư viện mở** mà bạn không biết, nên không thể tự `close()` trước `exec()`. Vì thế nguyên tắc: **hàm thư viện phải luôn đặt close-on-exec cho file nó mở**.
2. **Nếu `exec()` thất bại**, bạn có thể vẫn cần các fd đó — mà đã `close()` rồi thì không mở lại đúng file được nữa. `FD_CLOEXEC` chỉ đóng khi `exec()` **thành công**.

Ngoài ra là **bảo mật**: `exec` một chương trình lạ từ process đặc quyền mà rò fd (vd fd của file cấu hình chứa mật khẩu, hoặc socket lắng nghe) là lỗ hổng thật.

**Luôn ưu tiên bản nguyên tử** — `O_CLOEXEC`, `SOCK_CLOEXEC`, `accept4()`, `epoll_create1(EPOLL_CLOEXEC)` — thay vì `open()` rồi `fcntl(F_SETFD)`: giữa hai bước có khe hở, thread khác `fork`+`exec` đúng lúc đó là fd rò ra.
</details>

<details><summary>5) Vì sao process con nên gọi <code>_exit()</code> thay vì <code>exit()</code>?</summary>

Vì `exit()` **xả buffer stdio**, còn con lại đang giữ **bản sao buffer của cha**.

```c
printf("xin chào\n");     // vào buffer stdio, CHƯA xả nếu stdout không phải tty
if (fork() == 0) {
    exit(0);              // ❌ con xả BẢN SAO → in "xin chào" lần 1
}
wait(NULL);
exit(0);                  //    cha xả buffer của mình → in "xin chào" lần 2
```

Chạy `./prog` ra terminal thì **không thấy bug** (stdout là tty → line-buffered → đã xả ngay ở `printf`). Chạy `./prog > f.txt` thì thấy — vì lúc đó stdout là **fully-buffered**. Đây đúng là loại bug "chạy máy tôi thì ổn".

**Khác biệt đầy đủ:**

| | `_exit()` | `exit()` |
|---|---|---|
| Gọi handler `atexit()` | ❌ | ✅ |
| Xả buffer stdio | ❌ | ✅ |
| Kết thúc process | ✅ | ✅ |

**Hai cách chữa:** gọi **`fflush(NULL)` trước `fork()`** (xả mọi stream), hoặc **con dùng `_exit()`**. Thực tế nên làm cả hai.

Lý do phụ nhưng quan trọng không kém: `atexit()` handler của cha thường dọn tài nguyên **dùng chung** (xoá file tạm, đóng kết nối DB, nhả khoá). Con chạy chúng là **phá tài nguyên của cha**. `_exit()` bỏ qua toàn bộ.

⚠️ Cùng lý do đó, sau `fork()` trong chương trình **đa luồng**, con chỉ được gọi các hàm **async-signal-safe** cho tới khi `exec()` — vì mutex do các thread khác giữ sẽ **kẹt vĩnh viễn** trong con (các thread đó không tồn tại để nhả khoá). Xem [cụm 03 §2.2](03-signals-and-timers.md).
</details>

---

## 7. Đọc thêm (tùy chọn)

- Topic repo: [04/processes-signals.md](../../04-linux-system-programming/processes-signals.md), [03/process-thread.md](../../03-operating-system/process-thread.md).
- Bank: `OS-*` (process vs thread, context switch), `LNX-*` (fork/exec/fd) trong [os.md](../../14-prep/mock-interview/bank/os.md) · [linux-sysprog.md](../../14-prep/mock-interview/bank/linux-sysprog.md).
- Góc kernel: [LKD cụm process & scheduler](../lkd/01-process-sched-syscalls.md) — `task_struct`, `do_fork()`, và `clone()` nhìn từ trong.
- Góc lý thuyết OS: [OSTEP virtualization-cpu](../ostep/virtualization-cpu.md) (fork/exec/wait từ đầu) và [virtualization-memory](../ostep/virtualization-memory.md) (COW, page table).

---
⬅️ [Cụm 01 — File I/O](01-concepts-and-file-io.md) · [README cụm TLPI](README.md) · ➡️ [Cụm 03 — Signals](03-signals-and-timers.md)
