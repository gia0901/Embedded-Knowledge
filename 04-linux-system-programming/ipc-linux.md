# IPC trên Linux — API thực tế

> **TL;DR**
> - Đây là góc *thực hành* của [03-operating-system/ipc.md](../03-operating-system/ipc.md): API cụ thể trên Linux và lựa chọn.
> - **Pipe** `pipe()`, **FIFO** `mkfifo()`; **shared memory** dùng `shm_open` + `mmap` (POSIX, nên dùng) hoặc System V `shmget`; **message queue** `mq_open` (POSIX); **Unix domain socket** `socket(AF_UNIX)`.
> - **POSIX vs System V**: ưu tiên **POSIX** (API gọn, dựa trên fd nên hợp với epoll, dọn dẹp dễ). System V (`shmget/msgget/semget`) cũ, dùng key/ID riêng, còn gặp ở code legacy.
> - Linux-specific tiện: **`eventfd`** (đếm/báo hiệu), **`memfd`**, **`signalfd`**, **`timerfd`** — biến cơ chế thành fd để dùng chung trong epoll event loop.

---

## 1. Bản đồ lựa chọn nhanh

| Nhu cầu | Cơ chế nên dùng | API chính |
|---------|-----------------|-----------|
| Luồng dữ liệu cha→con đơn giản | pipe | `pipe()` |
| Hai tiến trình bất kỳ, luồng byte | FIFO | `mkfifo()` + `open` |
| Dữ liệu lớn, tốc độ tối đa, cùng máy | shared memory | `shm_open` + `mmap` |
| Thông điệp có ranh giới/ưu tiên | message queue | `mq_open`/`mq_send` |
| Hai chiều, linh hoạt, có thể qua mạng | socket | `socket()` |
| Báo hiệu/đếm trong event loop | eventfd | `eventfd()` |

---

## 2. Pipe & FIFO

```c
// Pipe (anonymous) — giữa tiến trình họ hàng
int fd[2];
pipe(fd);                 // fd[0] đọc, fd[1] ghi
// fork(): cha đóng fd[0], ghi fd[1]; con đóng fd[1], đọc fd[0]

// FIFO (named pipe) — hai tiến trình bất kỳ
mkfifo("/tmp/myfifo", 0666);
int wfd = open("/tmp/myfifo", O_WRONLY);   // bên ghi
int rfd = open("/tmp/myfifo", O_RDONLY);   // bên đọc (tiến trình khác)
```

- Là luồng byte một chiều, không ranh giới message → tự phân định gói nếu cần.
- `write` vào pipe không còn đầu đọc → nhận `SIGPIPE` (thường nên ignore và xử lý `EPIPE`).
- Pipe có thể dùng trực tiếp trong epoll (là fd).

---

## 3. Shared memory (POSIX) + đồng bộ

```c
// Tạo & map
int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
ftruncate(fd, SIZE);
void* p = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// ... đọc/ghi *p ... (cả hai process thấy chung)
munmap(p, SIZE);
shm_unlink("/myshm");     // xóa tên khi xong
```

- **Nhanh nhất** (không copy qua kernel) nhưng **phải tự đồng bộ**: đặt một **POSIX semaphore** (`sem_t`, có thể đặt *trong* shared memory với `sem_init(..., pshared=1, ...)`) hoặc process-shared mutex (`PTHREAD_PROCESS_SHARED`) để tránh race.
- Pattern thực tế: vùng shm chứa cả dữ liệu + một header có semaphore/flags điều phối (vd ring buffer cho producer–consumer giữa 2 process).

---

## 4. Message queue (POSIX)

```c
struct mq_attr attr = { .mq_maxmsg = 10, .mq_msgsize = 256 };
mqd_t mq = mq_open("/myq", O_CREAT | O_RDWR, 0644, &attr);
mq_send(mq, msg, len, /*priority*/ 0);
char buf[256]; unsigned prio;
mq_receive(mq, buf, sizeof buf, &prio);
mq_close(mq); mq_unlink("/myq");
```

- Giữ ranh giới message, hỗ trợ ưu tiên, kernel đệm → gửi/nhận không cần đồng thời.
- `mqd_t` trên Linux là fd → dùng được với epoll (chờ message như chờ I/O).

---

## 5. Unix domain socket

```c
int s = socket(AF_UNIX, SOCK_STREAM, 0);
struct sockaddr_un addr = { .sun_family = AF_UNIX };
strcpy(addr.sun_path, "/tmp/my.sock");
bind(s, (struct sockaddr*)&addr, sizeof addr);
listen(s, 5);
int c = accept(s, NULL, NULL);   // server nhận kết nối
// client: socket + connect tới cùng path
```

- Cùng API như network socket → dễ chuyển sang TCP sau này.
- Nhanh hơn TCP loopback (không qua stack mạng), và có thể **truyền file descriptor** giữa process qua `SCM_RIGHTS` (ancillary data) — rất mạnh (vd truyền socket đã accept cho worker).
- Hỗ trợ stream (`SOCK_STREAM`) và datagram (`SOCK_DGRAM`).

---

## 6. POSIX vs System V IPC

| | POSIX (nên dùng) | System V (legacy) |
|--|------------------|-------------------|
| Shared memory | `shm_open` + `mmap` | `shmget`/`shmat` |
| Message queue | `mq_open` | `msgget`/`msgsnd` |
| Semaphore | `sem_open`/`sem_init` | `semget`/`semop` |
| Định danh | Tên dạng `/name` | key (`ftok`) + ID số |
| Hợp với fd/epoll? | Phần lớn có (fd) | Không (ID riêng) |
| Dọn dẹp | `*_unlink`, theo namespace | `ipcrm`, dễ "rò" nếu quên |

→ Code mới ưu tiên **POSIX**: API nhất quán, dựa trên fd (tích hợp epoll), quản lý vòng đời rõ ràng. Biết System V để đọc code cũ.

---

## 7. fd-based primitives của Linux (rất hữu ích cho event loop)

Linux biến nhiều cơ chế thành **file descriptor** để gộp chung vào một epoll loop, tránh xử lý đặc biệt:

- **`eventfd`**: bộ đếm 64-bit dùng để báo hiệu giữa thread/process (vd worker báo "có việc"). Nhẹ hơn pipe cho mục đích notify.
- **`signalfd`**: nhận signal qua fd → đọc signal trong event loop thay vì viết handler async-signal-safe.
- **`timerfd`**: timer phát sự kiện qua fd → quản lý timeout trong cùng epoll loop.
- **`memfd_create`**: vùng nhớ ẩn danh có fd, dùng làm shared memory truyền qua socket.

Đây là lý do event loop trên Linux có thể xử lý đồng nhất: socket, timer, signal, notify — tất cả là fd.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Trên Linux bạn triển khai shared memory giữa hai process thế nào? Đồng bộ ra sao?</summary>

Dùng POSIX shared memory: `shm_open("/name", ...)` tạo/mở một đối tượng shared memory, `ftruncate` đặt kích thước, rồi `mmap(..., MAP_SHARED, fd, 0)` để map vào address space — cả hai process map cùng tên sẽ thấy chung vùng nhớ. Vì kernel không tự đồng bộ, phải tự bảo vệ truy cập: đặt một POSIX semaphore (`sem_t` với pshared, có thể nằm ngay trong vùng shm) hoặc một mutex process-shared (`PTHREAD_PROCESS_SHARED`) để loại trừ lẫn nhau. Khi xong gọi `munmap` và `shm_unlink` để xóa tên.
</details>

<details><summary>2) POSIX IPC và System V IPC khác nhau? Nên dùng cái nào?</summary>

System V IPC (`shmget`/`msgget`/`semget`) là API cũ, định danh bằng key (`ftok`) và ID số, không dựa trên file descriptor nên khó tích hợp epoll, và đối tượng dễ bị "rò" nếu quên dọn (`ipcrm`). POSIX IPC (`shm_open`+`mmap`, `mq_open`, `sem_open`) mới hơn, định danh bằng tên dạng `/name`, phần lớn dựa trên fd (tích hợp được event loop), API nhất quán và vòng đời rõ ràng (`*_unlink`). Code mới nên dùng POSIX; biết System V để đọc/ bảo trì code legacy.
</details>

<details><summary>3) Unix domain socket có gì hơn TCP loopback và pipe?</summary>

So với TCP loopback: Unix domain socket không đi qua stack mạng (không TCP/IP, không checksum/định tuyến) nên nhanh hơn và độ trễ thấp hơn cho giao tiếp cùng máy, nhưng vẫn dùng **cùng API socket** nên dễ chuyển sang TCP khi cần qua mạng. So với pipe: nó hai chiều, hỗ trợ cả stream lẫn datagram, mô hình client–server (nhiều client), và đặc biệt có thể **truyền file descriptor** giữa các process qua `SCM_RIGHTS` — ví dụ một process accept kết nối rồi chuyển socket đó cho process worker xử lý.
</details>

<details><summary>4) eventfd/signalfd/timerfd dùng để làm gì?</summary>

Chúng là các cơ chế Linux biến notify/signal/timer thành **file descriptor** để gộp vào chung một epoll event loop. `eventfd` là bộ đếm 64-bit để báo hiệu nhẹ giữa thread/process (đánh thức event loop khi có việc). `signalfd` cho phép nhận signal bằng cách `read` một fd, tránh phải viết handler async-signal-safe. `timerfd` phát sự kiện hết hạn qua fd để xử lý timeout trong cùng loop. Nhờ vậy event loop xử lý đồng nhất mọi nguồn sự kiện (socket, timer, signal, notify) chỉ qua epoll.
</details>

<details><summary>5) Khi nào chọn message queue thay vì shared memory?</summary>

Chọn message queue khi cần trao đổi các **thông điệp rời rạc có ranh giới rõ** (vd lệnh, sự kiện), muốn kernel đệm để bên gửi/nhận không phải chạy đồng thời, hoặc cần ưu tiên thông điệp — và đặc biệt khi muốn tránh tự xử lý đồng bộ. Chọn shared memory khi cần thông lượng/độ trễ tối ưu cho **dữ liệu lớn, tần suất cao** trên cùng máy, chấp nhận tự đồng bộ bằng semaphore/mutex. Nói gọn: message queue thiên về đơn giản & an toàn cho giao tiếp dạng message; shared memory thiên về hiệu năng tối đa cho dữ liệu khối lớn.
</details>

---
⬅️ [io-multiplexing.md](io-multiplexing.md) · ➡️ Tiếp theo: [05-drivers-device-tree/](../05-drivers-device-tree/)
