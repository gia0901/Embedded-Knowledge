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

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Vì sao process cần IPC còn thread thì không (theo cách đó)?</summary>

Mỗi process có không gian địa chỉ riêng và bị OS cô lập, nên không đọc/ghi trực tiếp bộ nhớ của process khác; muốn trao đổi dữ liệu phải qua cơ chế IPC do kernel cung cấp (pipe, shared memory, message queue, socket, signal). Thread trong cùng process đã chia sẻ sẵn code/data/heap, nên trao đổi dữ liệu chỉ là đọc/ghi biến chung — không cần IPC mà chỉ cần đồng bộ (mutex, condition variable) để tránh race.
</details>

<details><summary>2) Cơ chế IPC nào nhanh nhất và vì sao? Đánh đổi là gì?</summary>

Shared memory nhanh nhất vì hai process map cùng một vùng nhớ vật lý và đọc/ghi trực tiếp, **không phải copy dữ liệu qua kernel** (các IPC khác thường copy từ user sang kernel rồi sang user — hai lần). Đánh đổi: kernel không tự đồng bộ giúp, nên lập trình viên phải tự dùng semaphore/mutex để tránh race condition — đây là phần dễ sai. Phù hợp cho dữ liệu lớn, tần suất cao trên cùng máy.
</details>

<details><summary>3) Pipe và FIFO khác nhau thế nào?</summary>

Cả hai là kênh byte một chiều theo thứ tự FIFO. Pipe ẩn danh không có tên trên filesystem nên chỉ dùng được giữa các tiến trình **họ hàng** (kế thừa file descriptor qua fork), ví dụ `ls | grep`. FIFO (named pipe, tạo bằng `mkfifo`) có một tên trên filesystem, nên hai tiến trình **bất kỳ, không họ hàng** có thể mở cùng tên đó để giao tiếp.
</details>

<details><summary>4) Message queue khác pipe ở điểm nào?</summary>

Pipe là luồng byte liên tục không có ranh giới — bên nhận phải tự phân định đâu là hết một thông điệp. Message queue giữ **ranh giới message**: mỗi lần gửi là một thông điệp rời rạc, bên nhận lấy đúng từng thông điệp; ngoài ra hỗ trợ **độ ưu tiên** và khử ghép (bên gửi/nhận không cần chạy đồng thời vì queue đệm). Pipe phù hợp luồng dữ liệu kiểu stream, message queue phù hợp trao đổi lệnh/sự kiện có cấu trúc.
</details>

<details><summary>5) Khi nào dùng socket thay vì shared memory?</summary>

Dùng socket khi cần giao tiếp **qua mạng/giữa các máy** hoặc muốn thiết kế dễ mở rộng từ một máy ra phân tán (cùng API cho local Unix domain lẫn remote TCP/UDP). Shared memory chỉ hoạt động trên cùng một máy và yêu cầu tự đồng bộ; nó tối ưu cho thông lượng/độ trễ cực thấp với dữ liệu lớn cùng máy. Nói ngắn gọn: hiệu năng cực đại cùng máy → shared memory; linh hoạt/qua mạng/mở rộng → socket.
</details>

<details><summary>6) Signal có hạn chế gì? Vì sao handler phải cẩn thận?</summary>

Signal chỉ mang rất ít thông tin (chủ yếu là số hiệu signal) nên không dùng để truyền dữ liệu lớn — nó là cơ chế thông báo bất đồng bộ. Handler được gọi bất đồng bộ, có thể chen vào giữa bất kỳ chỗ nào của chương trình, kể cả khi đang trong `malloc`/`printf`; vì vậy trong handler chỉ được gọi các hàm **async-signal-safe**, nếu không sẽ gây deadlock/UB. Pattern an toàn là chỉ đặt một cờ `volatile sig_atomic_t` trong handler rồi xử lý thực sự ở vòng lặp chính.
</details>

---
⬅️ [sync-primitives.md](sync-primitives.md) · ➡️ Tiếp theo: [04-linux-system-programming/](../04-linux-system-programming/)
