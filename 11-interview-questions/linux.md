# Câu hỏi phỏng vấn — Linux System Programming

> Topic gốc: [04 Linux System Programming](../04-linux-system-programming/). Tự trả lời trước khi mở đáp án.

---

## 🟢 Cơ bản

<details><summary>1) File descriptor là gì?</summary>

Một số nguyên không âm, là chỉ mục vào bảng fd riêng của process; mỗi entry trỏ tới open file description trong kernel (offset, cờ) và tới đối tượng thực (inode, socket...). fd 0/1/2 = stdin/stdout/stderr. "Everything is a file" — file, pipe, socket, device đều thao tác qua fd với cùng syscall. → [file-io](../04-linux-system-programming/file-io.md)
</details>

<details><summary>2) Syscall là gì và vì sao có chi phí?</summary>

Là cách user yêu cầu kernel làm việc đặc quyền (I/O, cấp bộ nhớ): wrapper libc đặt số syscall + tham số, thực thi lệnh trap, CPU chuyển sang kernel mode, kernel thực hiện rồi trả về. Chi phí từ chuyển ngữ cảnh user↔kernel (đổi mức đặc quyền, lưu/khôi phục trạng thái) và đôi khi context switch. → [file-io](../04-linux-system-programming/file-io.md), [kernel-userspace](../05-drivers-device-tree/kernel-userspace.md)
</details>

<details><summary>3) Blocking và non-blocking I/O khác nhau gì?</summary>

Blocking (mặc định): `read` trên fd chưa có dữ liệu làm thread ngủ chờ. Non-blocking (`O_NONBLOCK`): trả về ngay, nếu chưa có dữ liệu thì `errno == EAGAIN`. Non-blocking cho phép một thread phục vụ nhiều fd (nền cho event loop với epoll). → [file-io](../04-linux-system-programming/file-io.md)
</details>

---

## 🟡 Trung bình

<details><summary>4) fork và exec khác nhau? Vì sao dùng chung?</summary>

`fork` tạo process con (bản sao COW). `exec` thay thế image hiện tại bằng chương trình khác, giữ PID, không trả về nếu thành công. Dùng chung để chạy chương trình mới mà vẫn giữ process cha; khoảng giữa cho con tùy biến môi trường (redirect fd, đổi uid) — chính là cách shell làm `cmd > file`. → [processes-signals](../04-linux-system-programming/processes-signals.md)
</details>

<details><summary>5) read() trả về ít byte hơn yêu cầu — lỗi không? Xử lý sao?</summary>

Không phải lỗi (short read). `read`/`write` trả số byte thực sự xử lý, có thể ít hơn (pipe/socket mới có một phần, bị signal ngắt). Xử lý bằng vòng lặp tiếp tục cho phần còn lại tới khi đủ/EOF/lỗi, và xử lý `EINTR` bằng thử lại. → [file-io](../04-linux-system-programming/file-io.md)
</details>

<details><summary>6) Vì sao nên dùng sigaction thay vì signal?</summary>

`signal()` có ngữ nghĩa không thống nhất giữa nền tảng (có hệ reset handler về mặc định sau lần đầu) và không kiểm soát rõ mask/restart. `sigaction` xác định, di động: kiểm soát signal mask khi handler chạy, cờ `SA_RESTART` (tự thử lại syscall bị ngắt), `SA_SIGINFO`. → [processes-signals](../04-linux-system-programming/processes-signals.md)
</details>

<details><summary>7) Phân biệt stdio buffer, page cache, và fsync.</summary>

stdio buffer ở user space (libc), gom dữ liệu giảm số syscall; `fflush` đẩy xuống kernel. Page cache ở kernel, cache nội dung file trong RAM; `write` thành công chỉ đảm bảo tới page cache. `fsync` ép kernel ghi page cache xuống disk vật lý — quan trọng cho độ bền (embedded mất điện). → [file-io](../04-linux-system-programming/file-io.md)
</details>

<details><summary>8) Cơ chế IPC nào nhanh nhất, đánh đổi gì?</summary>

Shared memory — hai process map cùng vùng nhớ vật lý, đọc/ghi trực tiếp không copy qua kernel (các IPC khác copy 2 lần). Đánh đổi: kernel không tự đồng bộ, phải tự dùng semaphore/mutex để tránh race. → [ipc-linux](../04-linux-system-programming/ipc-linux.md)
</details>

---

## 🟠 Khó

<details><summary>9) epoll khác select/poll thế nào? Vì sao scale tốt hơn?</summary>

select/poll truyền toàn bộ tập fd mỗi lần gọi và kernel quét tuyến tính O(n); select còn giới hạn ~1024. epoll cho đăng ký fd một lần, kernel duy trì tập và chỉ trả về fd **đã sẵn sàng** → chi phí tỉ lệ số fd sẵn sàng (k), không phải tổng (n). Khi nhiều kết nối idle, epoll hiệu quả hơn hẳn (nền của Nginx/Redis). Chỉ có trên Linux. → [io-multiplexing](../04-linux-system-programming/io-multiplexing.md)
</details>

<details><summary>10) Level-triggered và edge-triggered khác nhau? ET cần lưu ý gì?</summary>

LT (mặc định): báo liên tục chừng nào fd còn dữ liệu chưa đọc — dễ đúng. ET (`EPOLLET`): chỉ báo một lần khi trạng thái chuyển sang sẵn sàng — phải dùng fd non-blocking và đọc/ghi tới khi `EAGAIN` để vét cạn, nếu không "treo" sự kiện. ET hiệu năng cao hơn nhưng dễ sai. → [io-multiplexing](../04-linux-system-programming/io-multiplexing.md)
</details>

<details><summary>11) Vì sao trong signal handler chỉ được gọi hàm async-signal-safe?</summary>

Handler chạy bất đồng bộ, có thể chen vào giữa bất kỳ hàm không reentrant nào (vd `malloc`/`printf` đang giữ khóa nội bộ). Gọi lại hàm đó trong handler có thể deadlock/corruption (UB). Chỉ gọi hàm async-signal-safe (`write`, `_exit`). Pattern an toàn: handler chỉ set `volatile sig_atomic_t` flag, xử lý ở main loop; hoặc dùng `signalfd`. → [processes-signals](../04-linux-system-programming/processes-signals.md)
</details>

<details><summary>12) Điều gì xảy ra với fd qua fork và exec?</summary>

Qua fork, con kế thừa bản sao bảng fd, các fd cha/con trỏ tới cùng open file description (chung offset). Qua exec, fd giữ nguyên, trừ khi đánh dấu close-on-exec (`O_CLOEXEC`) thì tự đóng. Đặt CLOEXEC là thực hành tốt để tránh rò fd vào tiến trình con. → [file-io](../04-linux-system-programming/file-io.md)
</details>

---

## 🔴 Senior

<details><summary>13) Mô tả kiến trúc event loop. Nguyên tắc cốt lõi là gì?</summary>

Vòng lặp gọi `epoll_wait` ngủ tới khi có fd sẵn sàng, rồi với mỗi fd: accept kết nối mới (đăng ký vào epoll) hoặc đọc/ghi non-blocking và xử lý, quay lại chờ. Một thread phục vụ rất nhiều kết nối → ít RAM, ít context switch (Nginx/Redis/Node). Nguyên tắc: **không bao giờ block** trong event loop; mọi I/O non-blocking, tác vụ CPU nặng đẩy sang thread riêng để không chặn loop. Mở rộng: nhiều loop trên nhiều core + thread pool. → [io-multiplexing](../04-linux-system-programming/io-multiplexing.md)
</details>

<details><summary>14) Bạn dùng những cơ chế Linux nào để tích hợp signal/timer vào event loop?</summary>

Các fd-based primitives: `signalfd` (nhận signal qua fd, tránh handler async-signal-safe), `timerfd` (timer phát sự kiện qua fd cho timeout), `eventfd` (đếm/báo hiệu nhẹ giữa thread/process), `memfd` (vùng nhớ ẩn danh có fd). Nhờ biến mọi nguồn sự kiện thành fd, event loop xử lý đồng nhất socket + timer + signal + notify chỉ qua epoll. → [ipc-linux](../04-linux-system-programming/ipc-linux.md)
</details>

<details><summary>15) Thiết kế giao tiếp hiệu năng cao giữa hai process trên cùng máy?</summary>

Dùng shared memory cho dữ liệu lớn/tần suất cao (zero-copy): `shm_open` + `mmap`, tổ chức một ring buffer trong vùng shm cho producer–consumer, đồng bộ bằng POSIX semaphore process-shared hoặc mutex `PTHREAD_PROCESS_SHARED` đặt trong header của vùng shm. Nếu cần truyền message có ranh giới/điều khiển, kết hợp Unix domain socket (còn truyền được fd qua SCM_RIGHTS) hoặc eventfd để báo hiệu. Cân nhắc cache line alignment để tránh false sharing. → [ipc-linux](../04-linux-system-programming/ipc-linux.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
