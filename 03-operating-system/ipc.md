# IPC — Inter-Process Communication

> **TL;DR**
> - Process bị cô lập (address space riêng) nên cần cơ chế OS để giao tiếp: **pipe/FIFO, shared memory, message queue, socket, signal**.
> - **Shared memory**: nhanh nhất (không copy qua kernel) nhưng phải tự đồng bộ (semaphore/mutex).
> - **Pipe/FIFO**: luồng byte một chiều; pipe cho tiến trình họ hàng, FIFO (named pipe) cho tiến trình bất kỳ.
> - **Message queue**: gửi/nhận thông điệp có ranh giới, có thể ưu tiên.
> - **Socket**: linh hoạt nhất, dùng được cả cùng máy (Unix domain) lẫn qua mạng (TCP/UDP).
> - **Signal**: thông báo bất đồng bộ đơn giản (không mang nhiều dữ liệu).
> - Chọn theo: lượng dữ liệu, tốc độ, một/nhiều chiều, cùng máy hay qua mạng, độ phức tạp đồng bộ.

---

## 1. Vì sao cần IPC?

Mỗi process có không gian địa chỉ riêng (cô lập) — đây là điểm mạnh nhưng cũng nghĩa là chúng *không thể* đọc bộ nhớ của nhau trực tiếp. IPC là tập cơ chế do kernel cung cấp để các process trao đổi dữ liệu/tín hiệu một cách có kiểm soát. (Thread cùng process thì chia sẻ bộ nhớ sẵn, chỉ cần đồng bộ — xem [sync-primitives.md](sync-primitives.md).)

---

## 2. Pipe & FIFO (named pipe)

**Pipe**: kênh byte **một chiều**, dữ liệu ghi đầu này đọc đầu kia (FIFO order).

```cpp
int fd[2];
pipe(fd);            // fd[0] đọc, fd[1] ghi
// sau fork(): cha ghi fd[1], con đọc fd[0] → truyền dữ liệu cha→con
```

- Pipe ẩn danh: chỉ dùng giữa tiến trình **họ hàng** (qua fork, kế thừa fd). Đây là cơ chế sau `ls | grep` của shell.
- **FIFO / named pipe** (`mkfifo`): có tên trên filesystem → hai tiến trình **bất kỳ** mở cùng tên để giao tiếp.
- Đơn giản, nhưng là luồng byte không có ranh giới message (phải tự phân định), và một chiều.

---

## 3. Shared memory — nhanh nhất

Hai process **map cùng một vùng nhớ vật lý** vào address space của mình → đọc/ghi trực tiếp, **không copy qua kernel**.

```cpp
// POSIX shared memory
int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
ftruncate(fd, SIZE);
void* p = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// cả hai process thấy cùng dữ liệu tại p
```

- **Nhanh nhất** vì bỏ qua việc copy dữ liệu giữa user↔kernel (các IPC khác thường copy 2 lần).
- **Đánh đổi:** kernel không đồng bộ giúp → bạn **tự** dùng semaphore/mutex để tránh race condition. Đây là phần dễ sai nhất.
- Lý tưởng cho **dữ liệu lớn, tần suất cao** (vd buffer ảnh/video, sensor data).

---

## 4. Message queue

Hàng đợi thông điệp do kernel quản lý: process gửi/nhận các **message có ranh giới rõ ràng** (không phải luồng byte liên tục).

```cpp
// POSIX message queue
mqd_t mq = mq_open("/myq", O_CREAT | O_RDWR, 0644, &attr);
mq_send(mq, buf, len, priority);
mq_receive(mq, buf, maxlen, &priority);
```

- Giữ **ranh giới message** (nhận đúng từng gói), hỗ trợ **ưu tiên**.
- Khử ghép (decoupling): bên gửi/nhận không cần chạy đồng thời (queue đệm).
- Phù hợp giao tiếp dạng lệnh/sự kiện giữa các thành phần.

---

## 5. Socket

Endpoint giao tiếp hai chiều, mô hình tổng quát nhất:

- **Unix domain socket** (`AF_UNIX`): giữa các process **cùng máy** — nhanh hơn TCP loopback, có thể truyền cả file descriptor.
- **Network socket** (`AF_INET`, TCP/UDP): giao tiếp **qua mạng**, hoặc giữa các máy.
- **TCP**: tin cậy, có thứ tự, hướng kết nối (stream). **UDP**: không kết nối, không đảm bảo, nhẹ (datagram).

Ưu điểm: cùng một API cho local lẫn remote → dễ mở rộng từ một máy ra phân tán.

---

## 6. Signal

Thông báo **bất đồng bộ** gửi tới process (vd `SIGINT` từ Ctrl+C, `SIGTERM` yêu cầu dừng, `SIGSEGV` segfault, `SIGKILL` giết ngay).

```cpp
signal(SIGINT, handler);     // (đơn giản; thực tế nên dùng sigaction)
```

- Mang **ít thông tin** (chỉ số hiệu signal; có `sigqueue`/`SA_SIGINFO` để kèm dữ liệu nhỏ).
- Handler chạy bất đồng bộ → chỉ được gọi hàm **async-signal-safe** trong đó (không `printf`, `malloc`...). Pattern an toàn: set một `volatile sig_atomic_t` flag rồi xử lý ở main loop.
- Dùng để báo hiệu/điều khiển, không phải để truyền dữ liệu lớn. (Chi tiết: [04-linux-system-programming/processes-signals.md](../04-linux-system-programming/processes-signals.md).)

---

## 7. So sánh & lựa chọn

| Cơ chế | Hướng | Dữ liệu | Tốc độ | Đồng bộ | Qua mạng? |
|--------|-------|---------|--------|---------|-----------|
| Pipe/FIFO | 1 chiều | Luồng byte | Trung bình | Kernel lo blocking | Không |
| Shared memory | 2 chiều | Bất kỳ (vùng nhớ) | **Nhanh nhất** | **Tự lo** (semaphore) | Không |
| Message queue | 2 chiều | Message có ranh giới + ưu tiên | Trung bình | Kernel đệm | Không |
| Socket (Unix) | 2 chiều | Stream/datagram | Tốt | Kernel lo | Không |
| Socket (TCP/UDP) | 2 chiều | Stream/datagram | Phụ thuộc mạng | Kernel lo | **Có** |
| Signal | 1 chiều | Rất ít (số hiệu) | Nhanh | — | Không |

**Hướng dẫn chọn:**
- Dữ liệu lớn, hiệu năng tối đa, cùng máy → **shared memory** (+ semaphore).
- Trao đổi lệnh/sự kiện có cấu trúc → **message queue**.
- Cần qua mạng hoặc thiết kế hướng mở rộng → **socket**.
- Luồng dữ liệu đơn giản, pipeline kiểu shell → **pipe/FIFO**.
- Báo hiệu/điều khiển nhẹ → **signal**.

---

## 8. 💰 Chi phí thật & ⚠️ bẫy

> Đây là góc **lý thuyết**. Góc **API + thực chiến trên Linux** (kèm code, sơ đồ chọn, `SIGBUS`, robust mutex) nằm ở [04-linux-system-programming/ipc-linux.md](../04-linux-system-programming/ipc-linux.md).

**Chỉ có HAI cách chuyển byte** — mọi khác biệt còn lại đẻ ra từ đây:

| | ① Kernel copy hộ (pipe · socket · mq) | ② Map chung khung trang (shared memory) |
|---|---|---|
| Copy / message | **2** (user→kernel→user) | **0** |
| Syscall / message | **2** | **0** |
| Đồng bộ | **Kernel lo** (đệm, chặn khi đầy) | **Bạn lo** |
| **Một bên chết** | **Kernel lo** ⇒ bên kia nhận **EOF/EPIPE** và biết đường xử lý | **Bạn lo** ⇒ khoá kẹt vĩnh viễn, bên kia **treo mãi** |
| Báo hiệu "có dữ liệu mới" | Có sẵn | **Không có** — thường phải kèm `eventfd`/semaphore |

⇒ **Câu chốt của cả trang:** cách ② nhanh hơn vì nó **trả lại cho bạn** đúng phần việc kernel vẫn làm hộ.

**⚠️ Bẫy:**

**① "Shared memory nhanh nhất" là câu trả lời chưa xong.** Nó chỉ đúng khi message **lớn và dày**. Với message nhỏ (vài trăm byte) chi phí bị **syscall** chi phối ⇒ shm gần như **không nhanh hơn** mà vẫn phải trả đủ ba khoản ở cột ②. **Phép thử: đã ĐO thấy copy là nút thắt chưa?**

**② Trục "một bên chết" hay bị bỏ quên nhất, và là trục hay bị hỏi nhất.** Tách hai process để cách ly lỗi, rồi đặt một mutex chung vào giữa = **nối lại đúng thứ vừa tách**. Chữa bằng robust mutex, bỏ khoá ở đường nóng, hoặc watchdog.

**③ Byte stream không có ranh giới message.** Pipe/FIFO/`SOCK_STREAM` chỉ đảm bảo *dãy byte*, **không** đảm bảo *"một lần ghi = một lần đọc"* ⇒ mọi protocol phải **tự framing**. Đây là lớp bug *"đúng ở lab, sai ở khách"* ([13-networking/tcp-ip.md §6](../13-networking/tcp-ip.md)).

**④ Ghi vào pipe không còn bên đọc ⇒ `SIGPIPE`, mặc định GIẾT process** — im lặng, không log. Daemon phải `signal(SIGPIPE, SIG_IGN)` rồi xử lý `EPIPE`.

**⑤ Hàng đầy là câu hỏi NGHIỆP VỤ, không phải kỹ thuật.** Dữ liệu **trạng thái** (nhiệt độ, độ sáng) ⇒ **đè cái cũ**, vì mẫu cũ đã sai so với hiện tại. Dữ liệu **sự kiện/lệnh** ⇒ **chặn/backpressure**, mất là sai nghiệp vụ. Luôn có **biến đếm số bị bỏ** — mất thì được, không biết mình mất thì không.

**⑥ Signal là cơ chế báo hiệu, không phải kênh truyền dữ liệu.** Không xếp hàng (nhiều lần gửi có thể gộp thành một), không mang được payload đáng kể, và handler chỉ được gọi hàm **async-signal-safe**. Muốn dùng nghiêm túc trong event loop ⇒ **`signalfd`** ([04/ipc-linux.md §7](../04-linux-system-programming/ipc-linux.md)).

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [OS-022](../14-prep/mock-interview/bank/os.md) | Vì sao process cần IPC còn thread thì không (theo cách đó)? |
| [LNX-008](../14-prep/mock-interview/bank/linux-sysprog.md) | Cơ chế IPC nào nhanh nhất và vì sao? Đánh đổi là gì? |
| [LNX-020](../14-prep/mock-interview/bank/linux-sysprog.md) | Pipe và FIFO khác nhau thế nào? |
| [LNX-018](../14-prep/mock-interview/bank/linux-sysprog.md) | Message queue khác pipe ở điểm nào? |
| [LNX-016](../14-prep/mock-interview/bank/linux-sysprog.md) | Khi nào dùng socket thay vì shared memory? |
| [LNX-011](../14-prep/mock-interview/bank/linux-sysprog.md) | Signal có hạn chế gì? Vì sao handler phải cẩn thận? |

---
⬅️ [sync-primitives.md](sync-primitives.md) · ➡️ Tiếp theo: [04-linux-system-programming/](../04-linux-system-programming/)
