# TLPI — Cụm 06: IPC — Pipe, System V, POSIX, File Locking (ch. 43–48, 51–55) 🎯

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 43–48 (tr. 877–1016) và 51–55 (tr. 1057–1148).
> **Vì sao cụm này 🎯:** câu *"hai process cần trao đổi dữ liệu — bạn chọn cơ chế nào và vì sao?"* là câu **thiết kế hệ thống** rất hay gặp, và nó đo được bạn có nắm **đánh đổi** hay chỉ thuộc tên API. Cụm này cũng là nơi `SD-*` trong bank rút câu.

---

## 1. Bản đồ toàn cảnh — có bao nhiêu cơ chế và vì sao nhiều thế (ch. 43, tr. 877–888)

Kerrisk chia làm **ba nhóm chức năng** (§43.1, tr. 878):

```
                        ┌──────────────────────────────────────────┐
                        │        UNIX IPC facilities               │
                        └──────────────────────────────────────────┘
                                          │
        ┌─────────────────────────────────┼─────────────────────────────────┐
        ▼                                 ▼                                 ▼
 ┌─────────────┐                 ┌─────────────────┐               ┌──────────────┐
 │COMMUNICATION│                 │ SYNCHRONIZATION │               │   SIGNAL     │
 │(trao đổi dữ │                 │ (phối hợp thứ   │               │(chủ yếu cho  │
 │  liệu)      │                 │  tự hành động)  │               │ việc khác,   │
 └──────┬──────┘                 └────────┬────────┘               │ nhưng dùng   │
        │                                 │                        │ đồng bộ được)│
   ┌────┴─────┐                     ┌─────┴──────┐                 └──────────────┘
   ▼          ▼                     ▼            ▼
DATA        SHARED               semaphore    file lock
TRANSFER    MEMORY               (SysV,       (flock,
   │           │                  POSIX)       fcntl)
   │           ├─ System V shm
   │           ├─ POSIX shm
   │           └─ mmap (anonymous / file-backed)
   │
   ├─ BYTE STREAM ─── pipe · FIFO · UNIX domain stream socket · pseudoterminal
   └─ MESSAGE ─────── System V mq · POSIX mq · UNIX domain datagram socket
```

*(vẽ lại theo Figure 43-1, tr. 879)*

> **Vì sao nhiều cơ chế trùng chức năng đến vậy?** Không phải thiết kế — mà là **lịch sử**: System V IPC đến từ System V UNIX, POSIX IPC là bản thiết kế lại sau này để sửa khuyết điểm của System V, socket đến từ BSD. Cả ba tồn tại song song vì **tương thích ngược**. Hiểu điều này là hiểu vì sao API trông lộn xộn.

### 1.1. Byte stream vs message — khác biệt bản chất

```
   BYTE STREAM (pipe, FIFO, TCP socket)     MESSAGE (mq, datagram socket)
   ghi:  "ABC"  "DE"  "FGHI"                ghi:  [ABC] [DE] [FGHI]
         ↓                                        ↓
   đọc:  "ABCDEFGHI"  ← RANH GIỚI MẤT      đọc:  [ABC] rồi [DE] rồi [FGHI]
                                                  ← ranh giới ĐƯỢC GIỮ

   ⇒ byte stream: người nhận phải TỰ TÁCH GÓI      ⇒ message: kernel lo giúp
     (framing: header có length, hoặc delimiter)
```

🆕 Đây là lý do mọi giao thức trên TCP đều phải tự làm **framing** — và là gốc của bug "gói tin dính nhau" mà người mới hay gặp. Xem [system-design bank](../../14-prep/mock-interview/bank/system-design.md), phần thiết kế giao thức UART/TCP.

### 1.2. Vì sao shared memory nhanh nhất

```
   ❌ QUA KERNEL (pipe, message queue, socket)
      Process A                  KERNEL                  Process B
      ┌────────┐   write()   ┌────────────┐   read()   ┌────────┐
      │ buffer │ ──────────► │ buffer     │ ──────────►│ buffer │
      └────────┘   CHÉP 1    └────────────┘   CHÉP 2   └────────┘
                   user→kernel              kernel→user
                            ⇒ HAI lần chép + HAI syscall mỗi lần truyền

   ✅ SHARED MEMORY
      Process A                                          Process B
      ┌────────────┐                          ┌────────────┐
      │ page table │──┐                    ┌──│ page table │
      └────────────┘  ▼                    ▼  └────────────┘
                   ┌──────────────────────────┐
                   │  CÙNG MỘT trang vật lý   │  ⇒ KHÔNG chép, KHÔNG syscall
                   └──────────────────────────┘     ghi là bên kia thấy ngay
```

*(sơ đồ dựa trên Figure 43-2, tr. 880)*

**Cái giá:** kernel **không** đồng bộ hộ. Bạn phải tự thêm semaphore/mutex, và tự lo memory ordering. Đó là toàn bộ đánh đổi của shared memory: **nhanh nhất, nguy hiểm nhất**.

---

## 2. Pipe và FIFO (ch. 44, tr. 889–920) 🎯

### 2.1. Pipe

```c
int fd[2];
pipe(fd);                    // fd[0] = đầu ĐỌC, fd[1] = đầu GHI

if (fork() == 0) {           /* ===== CON: đọc ===== */
    close(fd[1]);            // ⚠️ BẮT BUỘC đóng đầu GHI không dùng
    char buf[100];
    ssize_t n;
    while ((n = read(fd[0], buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, n);
    close(fd[0]);
    _exit(EXIT_SUCCESS);
}
/* ===== CHA: ghi ===== */
close(fd[0]);                // ⚠️ BẮT BUỘC đóng đầu ĐỌC không dùng
write(fd[1], "xin chào", 8);
close(fd[1]);                // đóng → con thấy EOF
wait(NULL);
```

> ⚠️ **Vì sao phải đóng đầu không dùng — hai bug đối xứng:**
> - **Không đóng đầu GHI ở bên đọc** → `read()` **không bao giờ trả 0** (EOF), vì vẫn còn một fd mở cho đầu ghi (chính là của bản thân nó). Người đọc **treo vĩnh viễn**.
> - **Không đóng đầu ĐỌC ở bên ghi** → khi người đọc thật đã chết, `write()` vẫn **không sinh `SIGPIPE`**, nên bên ghi không biết mà dừng.

**Đặc điểm:**
- **Một chiều** trên Linux. Cần hai chiều → hai pipe, hoặc **socketpair()**.
- **Chỉ dùng được giữa process có quan hệ họ hàng** (fd kế thừa qua `fork`).
- Dung lượng có hạn (Linux hiện đại: 64 KB, chỉnh bằng `fcntl(F_SETPIPE_SZ)`). **Đầy thì `write()` chặn** → đây chính là **backpressure** tự nhiên của shell pipeline.
- Người đọc chết → bên ghi nhận **`SIGPIPE`** (mặc định giết process); nếu chặn/bỏ qua thì `write()` trả **`EPIPE`**.

### 2.2. `PIPE_BUF` — tính nguyên tử của `write()` 🎯

Kerrisk nêu rất rõ (§44.1, tr. 895–896): *"Writes of up to `PIPE_BUF` bytes are guaranteed to be atomic"*. **Trên Linux `PIPE_BUF` = 4096** (khác nhau giữa các hệ: 512 tối thiểu theo SUSv3, 5120 trên Solaris 8).

```
   Nhiều process cùng ghi vào MỘT pipe:

   ≤ PIPE_BUF byte mỗi lần:   [AAAA][BBBB][AAAA]      ✅ không xen kẽ
   >  PIPE_BUF byte mỗi lần:  [AAAA][BB][AAA][BBB]    ❌ dữ liệu BỊ TRỘN
```

Chi tiết tinh: khi ghi **≤ `PIPE_BUF`**, `write()` sẽ **chặn nếu cần** cho tới khi đủ chỗ để hoàn tất **nguyên tử**. Khi ghi **> `PIPE_BUF`**, nó chuyển được bao nhiêu thì chuyển và **cho phép bị xen kẽ** bởi process khác.

> 🆕 **Hệ quả thiết kế:** nếu nhiều worker cùng ghi log vào một pipe, hãy giữ mỗi bản ghi **≤ 4096 byte** — nếu không, các dòng log sẽ trộn vào nhau, và bug này chỉ lộ ra khi tải cao.

### 2.3. FIFO (named pipe)

```sh
mkfifo /tmp/myfifo          # hoặc mkfifo("/tmp/myfifo", 0666) trong C
```

Khác pipe ở đúng một điểm quyết định: **có tên trên filesystem** → **hai process bất kỳ, không cần họ hàng**, mở cùng đường dẫn là giao tiếp được.

> ⚠️ **Bẫy khi mở FIFO:** `open()` FIFO để **đọc** sẽ **chặn** cho tới khi có process khác mở nó để **ghi**, và ngược lại. Đây là cơ chế đồng bộ có chủ đích (rendezvous), nhưng dễ gây "treo không hiểu vì sao" khi test một mình. Muốn tránh → `O_NONBLOCK`.

---

## 3. System V IPC vs POSIX IPC (ch. 45–48, 51–54)

### 3.1. Ba cặp tương ứng

| Chức năng | System V (cũ) | POSIX (mới) |
|---|---|---|
| Message queue | `msgget`/`msgsnd`/`msgrcv` | `mq_open`/`mq_send`/`mq_receive` |
| Semaphore | `semget`/`semop` | `sem_open`/`sem_wait`/`sem_post` |
| Shared memory | `shmget`/`shmat`/`shmdt` | `shm_open` + **`mmap`** |

### 3.2. Vì sao POSIX IPC tốt hơn — nêu đúng ba điểm của sách 🎯

Kerrisk liệt kê (§51.2, tr. 1060–1061):

> 1. *"The POSIX IPC interface is **simpler** than the System V IPC interface."*
> 2. *"The POSIX IPC model — the use of **names instead of keys**, and the open, close, and unlink functions — is **more consistent with the traditional UNIX file model**."*
> 3. *"POSIX IPC objects are **reference counted**. This simplifies object deletion, because we can unlink a POSIX IPC object, knowing that it will be destroyed only when all processes have closed it."*

Điểm ③ là điểm thực dụng nhất. So sánh:

```
   System V:  đối tượng có KERNEL PERSISTENCE, không đếm tham chiếu
              → process tạo nó chết mà quên IPC_RMID → đối tượng "mồ côi"
                NẰM LẠI cho tới khi reboot hoặc gõ `ipcrm`
              → phải dùng `ipcs` để xem, `ipcrm` để dọn tay

   POSIX:     shm_unlink()/mq_unlink() → đánh dấu xoá; đối tượng thật sự biến mất
              khi process CUỐI CÙNG đóng nó — giống hệt unlink() cho file thường
              → xem bằng `ls /dev/shm`, `ls /dev/mqueue`
```

**Nhưng System V thắng một điểm — khả chuyển:** *"System V IPC is specified in SUSv3 and supported on nearly every UNIX implementation. By contrast, each of the POSIX IPC mechanisms is an **optional** component in SUSv3."* Trên chính Linux: POSIX shm chỉ có từ kernel 2.4, POSIX semaphore đầy đủ từ 2.6, POSIX mq từ 2.6.6.

> **Kết luận thực dụng:** code mới trên Linux → **dùng POSIX IPC** (hoặc socket). Gặp System V IPC → thường là code cũ; biết đủ để đọc và để dọn rác bằng `ipcs`/`ipcrm`.

### 3.3. Shared memory POSIX — mẫu dùng thật

```c
/* ===== Bên tạo ===== */
int fd = shm_open("/mysh", O_CREAT | O_RDWR, 0600);   // tạo /dev/shm/mysh
ftruncate(fd, sizeof(struct SharedData));              // ⚠️ BẮT BUỘC đặt kích thước
struct SharedData *p = mmap(NULL, sizeof(*p),
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);                       // ✅ đóng fd được — ánh xạ VẪN CÒN

/* ===== Dùng ===== */
sem_wait(&p->sem);               // ⚠️ TỰ đồng bộ — kernel KHÔNG lo hộ
p->counter++;
sem_post(&p->sem);

/* ===== Dọn ===== */
munmap(p, sizeof(*p));
shm_unlink("/mysh");             // chỉ bên nào chịu trách nhiệm mới gọi
```

Ba chi tiết hay sai:
1. **Quên `ftruncate()`** → vùng có kích thước 0 → truy cập là `SIGBUS`.
2. **Đóng `fd` sau `mmap` là hợp lệ** — ánh xạ giữ tham chiếu riêng.
3. **Con trỏ không dùng chung được** giữa các process (địa chỉ ánh xạ có thể khác nhau) → trong vùng shm phải dùng **offset**, không dùng con trỏ tuyệt đối.

**Đồng bộ trong shm:** đặt semaphore/mutex **ngay trong vùng shm** với thuộc tính process-shared:

```c
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);   // ← mấu chốt
pthread_mutex_init(&p->mtx, &attr);       // mtx nằm TRONG vùng shm
/* hoặc: sem_init(&p->sem, 1 /* pshared */, 1); */
```

> ⚠️ **Rủi ro lớn nhất của shm:** process **chết khi đang giữ khoá** → mọi process khác treo vĩnh viễn. Lời giải: `PTHREAD_MUTEX_ROBUST` (thread kế tiếp nhận `EOWNERDEAD` và phải `pthread_mutex_consistent()`), hoặc dùng **file lock** (kernel tự nhả khi process chết — xem §4).

---

## 4. File locking (ch. 55, tr. 1117–1148) 🎯

### 4.1. Advisory, không phải mandatory

Kerrisk nói rõ (§55.1, tr. 1119): *"By default, file locks are **advisory**. This means that a process can simply **ignore** a lock placed by another process."*

```
   ADVISORY (mặc định)                  MANDATORY (Linux có nhưng KHÔNG nên dùng)
   Khoá chỉ có tác dụng nếu             Kernel ép mọi I/O tôn trọng khoá
   MỌI process đều CHỦ ĐỘNG kiểm tra    → nhưng trên Linux cài đặt có RACE,
   trước khi I/O                          cần mount -o mand, và đã bị coi là hỏng
```

⇒ **Khoá file là một quy ước hợp tác**, không phải hàng rào. Một process cố tình bỏ qua vẫn ghi được.

### 4.2. `flock()` vs `fcntl()` — bảng phân biệt

| | `flock()` | `fcntl()` (POSIX record lock) |
|---|---|---|
| Phạm vi | **Cả file** | **Từng đoạn byte** (byte range) |
| Chuẩn | BSD, không có trong SUSv3 | **POSIX** — khả chuyển |
| Gắn với | **File description** (tầng ② ở [cụm 01 §2.3](01-concepts-and-file-io.md)) | **(process, i-node)** |
| Qua `fork()` | Con **chia sẻ** cùng khoá | Con **KHÔNG** kế thừa khoá |
| Qua `dup()` | Chia sẻ khoá | — |
| Nâng/hạ cấp khoá | Không nguyên tử | Nguyên tử |
| Trên NFS | ⚠️ không đáng tin ở bản cũ | Có hỗ trợ (qua lockd) |

> ⚠️ **Bẫy chết người của `fcntl()` lock** — hành vi phản trực giác nhất trong toàn bộ POSIX: khoá gắn với cặp **(process, i-node)**, nên **đóng BẤT KỲ fd nào** trỏ tới file đó sẽ **nhả HẾT khoá** của process trên file đó — kể cả fd hoàn toàn khác, kể cả khoá do phần code khác đặt.
>
> ```c
> fd1 = open("data", O_RDWR);  fcntl(fd1, F_SETLK, &lock);  // đặt khoá
> fd2 = open("data", O_RDONLY);                              // mở lại để đọc
> close(fd2);                  // 💥 KHOÁ TRÊN fd1 BIẾN MẤT
> ```
> 🆕 Đây là lý do Linux thêm **OFD locks** (`F_OFD_SETLK`, từ kernel 3.15) — gắn khoá với **file description** thay vì process, sửa cả bẫy này lẫn vấn đề với thread. **Code mới nên dùng OFD locks.**

### 4.3. Dùng để làm gì trong thực tế

```c
/* Mẫu file khoá cho daemon — đảm bảo chỉ MỘT instance chạy */
int fd = open("/var/run/mydaemon.pid", O_RDWR | O_CREAT, 0644);
struct flock fl = { .l_type = F_WRLCK, .l_whence = SEEK_SET,
                    .l_start = 0, .l_len = 0 };      // l_len=0 → tới CUỐI file
if (fcntl(fd, F_SETLK, &fl) == -1) {                 // F_SETLK: không chặn
    if (errno == EACCES || errno == EAGAIN)
        fatal("daemon đã chạy rồi");
}
/* ✅ Kernel TỰ NHẢ khoá khi process chết — kể cả bị SIGKILL hay crash.
   Đây là ưu điểm quyết định so với dùng file cờ tự chế. */
```

> **Vì sao mẫu này thắng "tạo file `.lock` rồi kiểm tra tồn tại":** file cờ tự chế **không tự dọn** khi process bị `kill -9` hoặc mất điện → lần khởi động sau daemon tưởng đã có instance chạy và từ chối chạy. Khoá của kernel **luôn được nhả** khi process kết thúc, bằng bất cứ cách nào.

---

## 5. Chọn cơ chế nào — bảng quyết định 🎯

| Nhu cầu | Chọn | Vì sao |
|---|---|---|
| Process **họ hàng**, luồng byte một chiều | **pipe** | Đơn giản nhất; backpressure sẵn có |
| Process **bất kỳ** cùng máy, luồng byte | **FIFO** hoặc **UNIX domain socket** | FIFO đơn giản; socket **hai chiều** và truyền được fd |
| Cần **ranh giới message** | **UNIX domain datagram socket** hoặc **POSIX mq** | mq còn có **ưu tiên** và **thông báo bất đồng bộ** |
| **Dữ liệu lớn, tần suất cao**, cùng máy | **shared memory** (`shm_open` + `mmap`) | **Zero-copy** — không chép, không syscall mỗi lần |
| Truyền **file descriptor** giữa process | **UNIX domain socket** + `SCM_RIGHTS` | Cơ chế **duy nhất** làm được |
| **Qua mạng** | **Internet domain socket** | Cái duy nhất vượt máy |
| Đảm bảo **một instance** của daemon | **`fcntl()`/OFD file lock** | Kernel tự nhả khi process chết |
| Đồng bộ N tài nguyên hữu hạn | **POSIX semaphore** | Đếm được; process-shared nếu đặt trong shm |
| Báo hiệu đơn giản, tích hợp `epoll` | **eventfd** / **pipe** | eventfd nhẹ hơn pipe (một bộ đếm 64-bit) |

**Ba câu hỏi ngược nên đặt trước khi chọn** (🆕):
1. **Cùng máy hay qua mạng?** — qua mạng thì chỉ còn socket.
2. **Bao nhiêu dữ liệu, tần suất nào?** — lớn + nóng → shm; nhỏ + thưa → mq/socket cho an toàn.
3. **Có cần ranh giới message không?** — có → mq/datagram; không → pipe/stream, nhưng phải tự framing.

> **Câu chốt khi phỏng vấn:** *"Mặc định tôi chọn **UNIX domain socket** cho IPC cùng máy — hai chiều, có ranh giới message nếu dùng datagram, truyền được fd, và tích hợp thẳng vào `epoll`. Chỉ chuyển sang **shared memory** khi đã đo và thấy chi phí chép/syscall là nút cổ chai — vì lúc đó tôi phải tự lo đồng bộ và tự lo ca process chết khi đang giữ khoá."*

---

## 6. Ít quan trọng — 1 dòng + tham chiếu

- **§43.2–43.3 — Phân loại chi tiết, persistence** (tr. 880): ba mức tồn tại — *process* (mmap ẩn danh), *kernel* (SysV/POSIX IPC — sống qua process nhưng chết khi reboot), *filesystem* (file-mapped — sống qua reboot).
- **§44.8–44.9 — `popen()`/`pclose()`** (tr. 913): tiện cho việc đơn giản; ⚠️ dùng shell nên **không dùng trong chương trình đặc quyền**.
- **§45.1–45.2 — Khoá IPC System V, `ftok()`** (tr. 922): sinh key từ đường dẫn + số; ⚠️ `ftok()` có thể **trùng key**, đây là một trong các khuyết điểm của System V.
- **§46 — System V message queue** (tr. 937): `msgsnd`/`msgrcv` có chọn theo **type** (đọc được message loại cụ thể). Đọc lướt.
- **§47 — System V semaphore** (tr. 965): `semop()` với mảng `sembuf`, cờ **`SEM_UNDO`** (tự hoàn tác khi process chết — ý tưởng hay nhưng cài đặt phức tạp). Đọc lướt.
- **§48 — System V shared memory, `shmget`/`shmat`** (tr. 997).
- **§49–50 — `mmap()` chi tiết, `mprotect`, `mlock`, `madvise`** (tr. 1017–1056): **thuộc cụm 08**; `MAP_SHARED` vs `MAP_PRIVATE`, file-backed vs anonymous.
- **§52 — POSIX message queue** (tr. 1063): ưu điểm riêng so với System V — có **`mq_notify()`** báo bất đồng bộ, và trên Linux **`mqd_t` là fd** nên đưa được vào `epoll`.
- **§53 — POSIX semaphore** (tr. 1089): named (`sem_open`) vs unnamed (`sem_init`).
- **§55.3–55.6 — Chi tiết `fcntl` lock, giới hạn, `/proc/locks`** (tr. 1130).

---

## 7. Góc interview 🎯

<details><summary>1) Có những cơ chế IPC nào trên Linux? Chọn cái nào khi nào?</summary>

Chia làm ba nhóm chức năng: **truyền dữ liệu**, **đồng bộ**, và **signal** (chủ yếu cho việc khác nhưng dùng đồng bộ được).

| Nhu cầu | Chọn | Lý do |
|---|---|---|
| Process **họ hàng**, luồng byte một chiều | **pipe** | Đơn giản nhất, có backpressure sẵn |
| Process **bất kỳ** cùng máy | **FIFO** / **UNIX domain socket** | FIFO có tên trên fs; socket **hai chiều** và truyền được fd |
| Cần **ranh giới message** | **UNIX datagram socket** / **POSIX mq** | mq có thêm **ưu tiên** và `mq_notify` |
| **Dữ liệu lớn, tần suất cao** | **shared memory** | **Zero-copy** |
| Truyền **file descriptor** | **UNIX domain socket + `SCM_RIGHTS`** | Cơ chế **duy nhất** làm được |
| **Qua mạng** | **Internet socket** | Cái duy nhất vượt máy |
| Một instance daemon | **`fcntl`/OFD file lock** | Kernel tự nhả khi process chết |

**Trục đánh đổi chính là số lần chép dữ liệu:** pipe/mq/socket đi **qua kernel** → **hai lần chép** (user→kernel, kernel→user) và hai syscall mỗi lần truyền. Shared memory ánh xạ **cùng trang vật lý** vào cả hai process → **không chép, không syscall**, nhưng **kernel không đồng bộ hộ** — bạn phải tự thêm semaphore/mutex và tự lo ca process chết khi đang giữ khoá.

**Ba câu hỏi ngược nên đặt trước:** cùng máy hay qua mạng? · bao nhiêu dữ liệu và tần suất nào? · có cần ranh giới message không?

**Câu chốt:** mặc định chọn **UNIX domain socket** cho IPC cùng máy (hai chiều, framing sẵn nếu datagram, truyền được fd, vào thẳng `epoll`); chỉ chuyển sang **shared memory** khi **đã đo** thấy chi phí chép là nút cổ chai.
</details>

<details><summary>2) Pipe và FIFO khác nhau thế nào? <code>PIPE_BUF</code> để làm gì?</summary>

**Chung:** cả hai là **luồng byte một chiều**, FIFO theo đúng thứ tự ghi, không có ranh giới message.

**Khác biệt duy nhất nhưng quyết định:**
- **Pipe** ẩn danh, **không có tên** trên filesystem → chỉ dùng được giữa process **có quan hệ họ hàng** (fd kế thừa qua `fork`). Đây là cách shell nối `a | b`.
- **FIFO** (`mkfifo`) **có tên trên filesystem** → **hai process bất kỳ, không cần họ hàng**, mở cùng đường dẫn là giao tiếp được.

**Hành vi chung phải nhớ:**
- Pipe đầy → `write()` **chặn** (backpressure tự nhiên).
- Người đọc chết → bên ghi nhận **`SIGPIPE`** (mặc định giết process); chặn/bỏ qua thì `write()` trả **`EPIPE`**.
- Phải **đóng đầu không dùng**: không đóng đầu ghi ở bên đọc → `read()` **không bao giờ trả EOF**, treo vĩnh viễn.
- ⚠️ Mở FIFO để đọc **chặn** cho tới khi có ai mở để ghi (và ngược lại) — cơ chế rendezvous có chủ đích, nhưng hay gây "treo không rõ lý do" khi test một mình.

**`PIPE_BUF`** = số byte tối đa mà một `write()` vào pipe được đảm bảo **nguyên tử** — không bị xen kẽ với dữ liệu của process khác. **Trên Linux là 4096** (SUSv3 chỉ yêu cầu tối thiểu 512).

Ý nghĩa thực tế: nhiều process cùng ghi vào **một** pipe (vd nhiều worker ghi log chung), nếu mỗi bản ghi **≤ 4096 byte** thì các bản ghi **không trộn vào nhau**; vượt quá thì kernel có thể chia nhỏ và **xen kẽ** với process khác → log lẫn lộn. Bug này chỉ lộ khi tải cao.

Chi tiết tinh: khi ghi ≤ `PIPE_BUF`, `write()` sẽ **chặn nếu cần** để hoàn tất nguyên tử; khi ghi > `PIPE_BUF`, nó chuyển được bao nhiêu thì chuyển và cho phép bị xen kẽ.
</details>

<details><summary>3) Vì sao shared memory nhanh nhất? Cái giá là gì?</summary>

**Nhanh vì bỏ được cả hai thứ đắt nhất:**

Với pipe/message queue/socket, mỗi lần truyền dữ liệu phải qua kernel: **chép lần 1** từ buffer user của A vào kernel, **chép lần 2** từ kernel ra buffer user của B — cộng **hai syscall**. Với shared memory, kernel ánh xạ **cùng một trang vật lý** vào không gian địa chỉ của cả hai process; A ghi thì B **thấy ngay**, **không chép, không syscall**.

**Cái giá — bốn thứ bạn phải tự lo:**

1. **Đồng bộ hoàn toàn thủ công.** Kernel không biết bạn đang ghi dở. Phải đặt semaphore/mutex **ngay trong vùng shm** với `PTHREAD_PROCESS_SHARED` (hoặc `sem_init(..., pshared=1, ...)`).
2. **Process chết khi đang giữ khoá** → mọi process khác **treo vĩnh viễn**. Cần `PTHREAD_MUTEX_ROBUST` (thread kế nhận `EOWNERDEAD`, phải gọi `pthread_mutex_consistent()`), hoặc dùng **file lock** vì kernel tự nhả khi process chết.
3. **Không dùng được con trỏ tuyệt đối** — địa chỉ `mmap` có thể khác nhau giữa các process. Trong vùng shm phải dùng **offset**.
4. **Không có ranh giới message, không có thông báo** — cần báo "có dữ liệu mới" thì phải thêm eventfd/semaphore/pipe.

**Mẫu dùng POSIX:**
```c
int fd = shm_open("/mysh", O_CREAT|O_RDWR, 0600);
ftruncate(fd, size);                       // ⚠️ quên bước này → SIGBUS khi truy cập
void *p = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);                                  // ✅ hợp lệ — ánh xạ vẫn còn
```

🆕 Mẫu thực chiến hay dùng: **ring buffer đặt trong vùng shm** cho producer–consumer giữa hai process, đồng bộ bằng semaphore process-shared — xem [12-dsa/ring-buffer.md §8](../../12-dsa/ring-buffer.md).
</details>

<details><summary>4) System V IPC và POSIX IPC — nên dùng cái nào?</summary>

**Code mới trên Linux: POSIX IPC.** TLPI nêu ba ưu điểm:

1. **Giao diện đơn giản hơn** System V.
2. **Nhất quán với mô hình file truyền thống của UNIX** — dùng **tên** thay vì key, và các hàm `open`/`close`/`unlink` hoạt động y như với file.
3. **Có đếm tham chiếu** — đây là ưu điểm thực dụng nhất: `shm_unlink()` đánh dấu xoá, đối tượng **thật sự biến mất khi process cuối cùng đóng nó**, giống hệt `unlink()` cho file thường.

**Vì sao ③ quan trọng:** System V IPC **không đếm tham chiếu** và có kernel persistence. Process tạo nó chết mà quên `IPC_RMID` → đối tượng **nằm lại trong kernel cho tới khi reboot**, phải dùng `ipcs` để xem và `ipcrm` để dọn tay. Với POSIX, `ls /dev/shm` và `ls /dev/mqueue` là xem được, và vòng đời tự quản.

**System V thắng đúng một điểm — khả chuyển:** nó nằm trong SUSv3 và có trên gần như mọi UNIX; còn **mỗi cơ chế POSIX IPC là thành phần TUỲ CHỌN** trong SUSv3. Ngay trên Linux: POSIX shm chỉ có từ kernel 2.4, POSIX semaphore đầy đủ từ 2.6, POSIX mq từ 2.6.6. Ngoài ra SUSv3 không quy định lệnh xem/xoá đối tượng POSIX IPC nên mỗi hệ làm một kiểu.

**Kết luận thực dụng:** Linux hiện đại → POSIX IPC, hoặc tốt hơn nữa là **UNIX domain socket** (vì nó cho fd, vào được `epoll`, và truyền được fd). Gặp System V IPC thì thường là code cũ — biết đủ để đọc và để dọn rác bằng `ipcs`/`ipcrm`.
</details>

<details><summary>5) File lock dùng để làm gì? <code>flock()</code> khác <code>fcntl()</code> ra sao?</summary>

**Quan trọng nhất phải nói trước: khoá file mặc định là ADVISORY** — process khác **có thể bỏ qua hoàn toàn**. Nó chỉ có tác dụng nếu **mọi** process đều chủ động kiểm tra trước khi I/O. Đây là **quy ước hợp tác**, không phải hàng rào. (Linux có mandatory locking nhưng cài đặt có race và cần `mount -o mand` — coi như không dùng được.)

| | `flock()` | `fcntl()` (POSIX record lock) |
|---|---|---|
| Phạm vi | **Cả file** | **Từng đoạn byte** |
| Chuẩn | BSD, không trong SUSv3 | **POSIX**, khả chuyển |
| Gắn với | **File description** | **(process, i-node)** |
| Qua `fork()` | Con **chia sẻ** khoá | Con **không** kế thừa |
| Nâng/hạ cấp | Không nguyên tử | Nguyên tử |

⚠️ **Bẫy chết người của `fcntl()` lock:** vì khoá gắn với cặp (process, i-node), **đóng bất kỳ fd nào** trỏ tới file đó sẽ **nhả hết khoá** của process trên file đó — kể cả một fd hoàn toàn khác do phần code khác mở:
```c
fd1 = open("data", O_RDWR);  fcntl(fd1, F_SETLK, &lock);   // đặt khoá
fd2 = open("data", O_RDONLY);  close(fd2);                  // 💥 khoá trên fd1 MẤT
```
Nó cũng không phân biệt được các thread trong cùng process. Vì vậy Linux thêm **OFD locks** (`F_OFD_SETLK`, kernel 3.15+) gắn khoá với **file description** — **code mới nên dùng OFD locks**.

**Dùng thực tế nhất — đảm bảo chỉ một instance daemon:**
```c
int fd = open("/var/run/d.pid", O_RDWR|O_CREAT, 0644);
struct flock fl = {.l_type=F_WRLCK, .l_whence=SEEK_SET, .l_start=0, .l_len=0};
if (fcntl(fd, F_SETLK, &fl) == -1)      // F_SETLK = không chặn
    fatal("daemon đã chạy rồi");
```
**Vì sao hơn hẳn "tạo file `.lock` rồi kiểm tra tồn tại":** kernel **tự nhả khoá khi process kết thúc bằng bất cứ cách nào** — kể cả `SIGKILL`, crash, hay mất điện. File cờ tự chế thì **nằm lại vĩnh viễn** sau khi bị `kill -9`, và lần khởi động sau daemon sẽ từ chối chạy vì tưởng đã có instance khác.
</details>

---

## 8. Đọc thêm (tùy chọn)

- Topic repo: [03/ipc.md](../../03-operating-system/ipc.md), [04/ipc-linux.md](../../04-linux-system-programming/ipc-linux.md).
- Bank: `LNX-*` (pipe vs FIFO, shm), `SD-*` (thiết kế IPC hai process) trong [linux-sysprog.md](../../14-prep/mock-interview/bank/linux-sysprog.md) · [system-design.md](../../14-prep/mock-interview/bank/system-design.md).
- Ring buffer trong vùng shm: [12-dsa/ring-buffer.md §8](../../12-dsa/ring-buffer.md).
- Đồng bộ giữa **thread** (khác giữa process): [cụm 04 §2](04-threads.md).
- Góc lý thuyết: [OSTEP concurrency](../ostep/concurrency.md) — semaphore, producer-consumer.

---
⬅️ [Cụm 05 — Alternative I/O models](05-alternative-io-models.md) · [README cụm TLPI](README.md)
