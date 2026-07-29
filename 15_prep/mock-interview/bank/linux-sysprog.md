# LNX — Linux System Programming

> Domain `LNX`. Gộp từ [11/linux.md](../../../11-interview-questions/linux.md) + [technical_round/02 PHẦN 2](../../technical_round/02_question_bank.md). Track dùng: `linux-sysprog`, `bsp`, `cpp-system`, `ostep`.

---

#### LNX-001 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**File descriptor là gì?**
<details><summary>Đáp án</summary>

Một số nguyên không âm, là chỉ mục vào bảng fd riêng của process; mỗi entry trỏ tới open file description trong kernel (offset, cờ) và tới đối tượng thực (inode, socket...). fd 0/1/2 = stdin/stdout/stderr. "Everything is a file" — file, pipe, socket, device đều thao tác qua fd với cùng syscall.
</details>

#### LNX-002 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md), [kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Syscall là gì và vì sao có chi phí?**
<details><summary>Đáp án</summary>

Là cách user yêu cầu kernel làm việc đặc quyền (I/O, cấp bộ nhớ): wrapper libc đặt số syscall + tham số, thực thi lệnh trap, CPU chuyển sang kernel mode, kernel thực hiện rồi trả về. Chi phí từ chuyển ngữ cảnh user↔kernel (đổi mức đặc quyền, lưu/khôi phục trạng thái) và đôi khi context switch.
</details>

#### LNX-003 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Blocking và non-blocking I/O khác nhau gì?**
<details><summary>Đáp án</summary>

Blocking (mặc định): `read` trên fd chưa có dữ liệu làm thread ngủ chờ. Non-blocking (`O_NONBLOCK`): trả về ngay, nếu chưa có dữ liệu thì `errno == EAGAIN`. Non-blocking cho phép một thread phục vụ nhiều fd (nền cho event loop với epoll).
</details>

#### LNX-004 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**fork và exec khác nhau? Vì sao dùng chung?**
<details><summary>Đáp án</summary>

`fork` tạo process con (bản sao COW). `exec` thay thế image hiện tại bằng chương trình khác, giữ PID, không trả về nếu thành công. Dùng chung để chạy chương trình mới mà vẫn giữ process cha; khoảng giữa cho con tùy biến môi trường (redirect fd, đổi uid) — chính là cách shell làm `cmd > file`.
</details>

#### LNX-005 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**read() trả về ít byte hơn yêu cầu — lỗi không? Xử lý sao?**
<details><summary>Đáp án</summary>

Không phải lỗi (short read). `read`/`write` trả số byte thực sự xử lý, có thể ít hơn (pipe/socket mới có một phần, bị signal ngắt). Xử lý bằng vòng lặp tiếp tục cho phần còn lại tới khi đủ/EOF/lỗi, và xử lý `EINTR` bằng thử lại.
</details>

#### LNX-006 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Vì sao nên dùng sigaction thay vì signal?**
<details><summary>Đáp án</summary>

`signal()` có ngữ nghĩa không thống nhất giữa nền tảng (có hệ reset handler về mặc định sau lần đầu) và không kiểm soát rõ mask/restart. `sigaction` xác định, di động: kiểm soát signal mask khi handler chạy, cờ `SA_RESTART` (tự thử lại syscall bị ngắt), `SA_SIGINFO`.
</details>

#### LNX-007 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Phân biệt stdio buffer, page cache, và fsync.**
<details><summary>Đáp án</summary>

stdio buffer ở user space (libc), gom dữ liệu giảm số syscall; `fflush` đẩy xuống kernel. Page cache ở kernel, cache nội dung file trong RAM; `write` thành công chỉ đảm bảo tới page cache. `fsync` ép kernel ghi page cache xuống disk vật lý — quan trọng cho độ bền (embedded mất điện).
</details>

#### LNX-008 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Cơ chế IPC nào nhanh nhất, đánh đổi gì?**
<details><summary>Đáp án</summary>

Shared memory — hai process map cùng vùng nhớ vật lý, đọc/ghi trực tiếp không copy qua kernel (các IPC khác copy 2 lần). Đánh đổi: kernel không tự đồng bộ, phải tự dùng semaphore/mutex để tránh race.
</details>

#### LNX-009 · 🟠 · concept · ⭐ · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**epoll khác select/poll thế nào? Vì sao scale tốt hơn?**
<details><summary>Đáp án</summary>

select/poll truyền toàn bộ tập fd mỗi lần gọi và kernel quét tuyến tính O(n); select còn giới hạn ~1024. epoll cho đăng ký fd một lần, kernel duy trì tập và chỉ trả về fd **đã sẵn sàng** → chi phí tỉ lệ số fd sẵn sàng (k), không phải tổng (n). Khi nhiều kết nối idle, epoll hiệu quả hơn hẳn (nền của Nginx/Redis). Chỉ có trên Linux.
</details>

#### LNX-010 · 🟠 · concept · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Level-triggered và edge-triggered khác nhau? ET cần lưu ý gì?**
<details><summary>Đáp án</summary>

LT (mặc định): báo liên tục chừng nào fd còn dữ liệu chưa đọc — dễ đúng. ET (`EPOLLET`): chỉ báo một lần khi trạng thái chuyển sang sẵn sàng — phải dùng fd non-blocking và đọc/ghi tới khi `EAGAIN` để vét cạn, nếu không "treo" sự kiện. ET hiệu năng cao hơn nhưng dễ sai.
</details>

#### LNX-011 · 🟠 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Vì sao trong signal handler chỉ được gọi hàm async-signal-safe?**
<details><summary>Đáp án</summary>

Handler chạy bất đồng bộ, có thể chen vào giữa bất kỳ hàm không reentrant nào (vd `malloc`/`printf` đang giữ khóa nội bộ). Gọi lại hàm đó trong handler có thể deadlock/corruption (UB). Chỉ gọi hàm async-signal-safe (`write`, `_exit`). Pattern an toàn: handler chỉ set `volatile sig_atomic_t` flag, xử lý ở main loop; hoặc dùng `signalfd`.
</details>

#### LNX-012 · 🟠 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Điều gì xảy ra với fd qua fork và exec?**
<details><summary>Đáp án</summary>

Qua fork, con kế thừa bản sao bảng fd, các fd cha/con trỏ tới cùng open file description (chung offset). Qua exec, fd giữ nguyên, trừ khi đánh dấu close-on-exec (`O_CLOEXEC`) thì tự đóng. Đặt CLOEXEC là thực hành tốt để tránh rò fd vào tiến trình con.
</details>

#### LNX-013 · 🔴 · design · ⭐ · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Mô tả kiến trúc event loop. Nguyên tắc cốt lõi là gì?**
<details><summary>Đáp án</summary>

Vòng lặp gọi `epoll_wait` ngủ tới khi có fd sẵn sàng, rồi với mỗi fd: accept kết nối mới (đăng ký vào epoll) hoặc đọc/ghi non-blocking và xử lý, quay lại chờ. Một thread phục vụ rất nhiều kết nối → ít RAM, ít context switch (Nginx/Redis/Node). Nguyên tắc: **không bao giờ block** trong event loop; mọi I/O non-blocking, tác vụ CPU nặng đẩy sang thread riêng. Mở rộng: nhiều loop trên nhiều core + thread pool.
</details>

#### LNX-014 · 🔴 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Bạn dùng những cơ chế Linux nào để tích hợp signal/timer vào event loop?**
<details><summary>Đáp án</summary>

Các fd-based primitives: `signalfd` (nhận signal qua fd, tránh handler async-signal-safe), `timerfd` (timer phát sự kiện qua fd cho timeout), `eventfd` (đếm/báo hiệu nhẹ giữa thread/process), `memfd` (vùng nhớ ẩn danh có fd). Nhờ biến mọi nguồn sự kiện thành fd, event loop xử lý đồng nhất socket + timer + signal + notify chỉ qua epoll.
</details>

#### LNX-015 · 🔴 · design · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Thiết kế giao tiếp hiệu năng cao giữa hai process trên cùng máy?**
<details><summary>Đáp án</summary>

Dùng shared memory cho dữ liệu lớn/tần suất cao (zero-copy): `shm_open` + `mmap`, tổ chức một ring buffer trong vùng shm cho producer–consumer, đồng bộ bằng POSIX semaphore process-shared hoặc mutex `PTHREAD_PROCESS_SHARED` đặt trong header của vùng shm. Nếu cần truyền message có ranh giới/điều khiển, kết hợp Unix domain socket (truyền được fd qua SCM_RIGHTS) hoặc eventfd để báo hiệu. Cân nhắc cache line alignment để tránh false sharing.
</details>

#### LNX-016 · 🟡 · concept · ⭐ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Khi nào dùng POSIX message queue thay vì shared memory?**
<details><summary>Đáp án</summary>

Message queue cho **ranh giới message rõ ràng**, có priority, và decouple producer–consumer — hợp khi dữ liệu là chuỗi giá trị/sự kiện theo thời gian và throughput không phải nút thắt. Shared memory nhanh hơn nhưng phải tự lo mutex/semaphore và dễ race. Chọn message queue khi an toàn/dễ bảo trì quan trọng hơn tốc độ tối đa.
</details>

#### LNX-017 · 🟡 · design · ⭐ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Message queue đầy thì xử lý sao?**
<details><summary>Đáp án</summary>

Mặc định `mq_send` block; `O_NONBLOCK` trả `EAGAIN`. Nhưng nếu chỉ **giá trị đích mới nhất** quan trọng (vd ramp độ sáng), giải pháp đúng là **coalescing/latest-value-wins** (giữ ô giá trị đích, consumer nội suy) hoặc **drop-oldest** (ghi đè giá trị cũ vì giá trị mới phản ánh trạng thái chính xác hơn), kèm **rate limiting** ở producer. Tránh để block làm trễ phản ứng.
</details>

#### LNX-018 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**So sánh các cơ chế IPC.**
<details><summary>Đáp án</summary>

Pipe/FIFO: byte stream đơn giản, 1 chiều. Message queue: có ranh giới message + priority. Shared memory: nhanh nhất, tự đồng bộ. Socket: liên máy/mạng. Signal: báo sự kiện, ít dữ liệu. Chọn theo: tốc độ vs ranh giới message vs phạm vi (cùng máy/khác máy).
</details>

---
⬅️ [Bank index](README.md)
