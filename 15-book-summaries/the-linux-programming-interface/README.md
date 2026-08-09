# The Linux Programming Interface — Michael Kerrisk (No Starch Press, 2010)

> **Nguồn summary:** **đã đối chiếu PDF** — bản 1556 trang (`The Linux Programming Interface.pdf`, ModDate 2025-03-29). **Quy đổi trang: `trang PDF = trang sách + 44`** (xác minh chéo 2 điểm: ch. 47 sách tr. 965 → PDF 1009; ch. 63 sách tr. 1325 → PDF 1369). Mọi số trang trong các file cụm là **trang SÁCH**.
> **Vì sao đọc cuốn này:** đây là **sách tham chiếu API userspace Linux đầy đủ nhất** — 64 chương, 1400+ trang, viết bởi người bảo trì `man-pages`. Với JD [Datalogic](../../14-prep/study-plans/datalogic-plan.md), trụ số 1 là *"Familiar with Linux userspace + debugging"* — đó chính là nội dung cuốn này. Nó trả lời cấp độ *"vì sao API được thiết kế thế"*, không chỉ *"gọi hàm nào"*.

---

## TL;DR — cuốn sách này dạy gì

Toàn bộ **giao diện lập trình giữa chương trình userspace và kernel Linux**: file I/O, process, signal, thread, IPC, socket, và các mô hình I/O. Ba luận điểm xuyên suốt:

1. **"Everything is a file descriptor."** Cùng bốn syscall `open/read/write/close` chạy được trên file thường, pipe, terminal, socket, thiết bị. Sự thống nhất này (ch. 4) là thứ khiến shell pipeline, `select`/`epoll`, và cả `dup2` redirect hoạt động — một trừu tượng, dùng lại khắp nơi.
2. **API là kết quả của lịch sử, không phải thiết kế sạch.** Rất nhiều điều khó chịu (signal không đáng tin ở bản cũ, System V IPC lạc lõng, `errno` toàn cục) là **di sản tương thích ngược**. Kerrisk luôn giải thích *vì sao nó thành ra như vậy* — đó là phần khiến cuốn sách hơn hẳn `man`.
3. **Sai lầm phổ biến đều đến từ việc không đọc kỹ ngữ nghĩa biên.** `read()` trả về ít hơn yêu cầu, syscall bị `EINTR`, `write()` vào pipe đầy, buffer stdio chưa flush lúc `fork()` — cuốn sách dành nhiều trang cho đúng những ca này, và đó cũng là chỗ phỏng vấn hỏi.

> **Quan hệ với các sách khác trong repo:** [OSTEP](../ostep/) dạy *kernel làm gì bên trong* (lý thuyết OS); [LKD](../lkd/) dạy *kernel cài đặt ra sao* (code kernel); **TLPI dạy *userspace gọi vào kernel thế nào*** (API). Ba góc của cùng một ranh giới. Với công việc hiện tại của bạn (C++ shared library gọi kernel API), TLPI là góc **sát việc nhất**.

---

## 🕐 Đọc gì nếu chỉ có N giờ

| Có | Đọc |
|---|---|
| **2 giờ** | [01 §File I/O](01-concepts-and-file-io.md) (ch. 4–5, 13) + [05 Alternative I/O models](05-alternative-io-models.md) (ch. 63). Đây là hai cụm trả lời được nhiều câu phỏng vấn Linux nhất trên mỗi giờ bỏ ra. |
| **6 giờ** | Thêm [03 Signals](03-signals-and-timers.md) (ch. 20–22) + [02 Process & exec](02-processes-and-exec.md) (ch. 24–28). Bốn cụm này phủ ~80% câu `LNX-*` và `OS-*` trong bank. |
| **15 giờ** | Thêm [04 Threads](04-threads.md) (ch. 29–33) + [06 IPC](06-ipc.md) (ch. 43–48, 51–55). |
| **Đọc hết** | Theo thứ tự file 01 → 10. Cụm 07–10 là tham chiếu tra cứu, đọc lướt cũng được. |

---

## 🎯 Lộ trình ôn nhanh trước interview

Xếp theo **giá trị/phút** cho JD Embedded Linux, ôn ngược từ trên xuống nếu sát ngày:

1. **[05 — Alternative I/O models](05-alternative-io-models.md)** 🎯 — `select`/`poll`/`epoll`, **level vs edge-triggered**, vì sao epoll thắng khi N lớn. Trụ JD "userspace", và là câu hỏi thiết kế server kinh điển.
2. **[01 — File I/O & buffering](01-concepts-and-file-io.md)** 🎯 — fd là gì, quan hệ 3 bảng của kernel, `dup2`, **atomicity & race**, `O_APPEND`, và **hai tầng buffer** (stdio vs kernel) — chỗ này giải thích `fflush` vs `fsync`, một câu hay hỏi mà ít người trả lời sạch.
3. **[03 — Signals & timers](03-signals-and-timers.md)** 🎯 — **async-signal-safe**, `EINTR` và `SA_RESTART`, `SIGCHLD`/zombie, vì sao `printf` trong handler là bug.
4. [02 — Process & program execution](02-processes-and-exec.md) 🎯 — `fork`/`exec`/`wait`, cái gì kế thừa qua `fork` và qua `exec`, `O_CLOEXEC`, layout bộ nhớ process.
5. [04 — Threads](04-threads.md) 🎯 — thread vs process, `pthread_mutex`/`cond`, thread-safety và reentrancy, cancellation.
6. [06 — IPC](06-ipc.md) 🎯 — bảng chọn cơ chế IPC, pipe/FIFO, shared memory + semaphore, **file locking**.
7. Còn thời gian: [07 Sockets](07-sockets.md) → [08 mmap & shared libraries](08-memory-mappings-and-shared-libraries.md).

---

## 🗺️ Bản đồ: chương ↔ cụm ↔ topic liên quan

| Cụm (file) | Chương sách | Trang sách | 🎯 | Topic repo liên quan |
|---|---|---|---|---|
| [01 — Khái niệm nền & File I/O](01-concepts-and-file-io.md) | 1–5, 13 | 1–112, 233–250 | 🎯 | [04/file-io](../../04-linux-system-programming/file-io.md) |
| [02 — Process & program execution](02-processes-and-exec.md) | 6, 7, 24–28 | 113–152, 513–616 | 🎯 | [03/process-thread](../../03-operating-system/process-thread.md), [04/processes-signals](../../04-linux-system-programming/processes-signals.md) |
| [03 — Signals & timers](03-signals-and-timers.md) | 20–23 | 387–512 | 🎯 | [04/processes-signals](../../04-linux-system-programming/processes-signals.md) |
| [04 — Threads](04-threads.md) | 29–33 | 617–698 | 🎯 | [03/process-thread](../../03-operating-system/process-thread.md), [02/concurrency](../../02-modern-cpp/concurrency.md) |
| [05 — Alternative I/O models](05-alternative-io-models.md) | 63 | 1325–1374 | 🎯 | [04/io-multiplexing](../../04-linux-system-programming/io-multiplexing.md) |
| [06 — IPC](06-ipc.md) | 43–48, 51–55 | 877–1016, 1057–1148 | 🎯 | [03/ipc](../../03-operating-system/ipc.md), [04/ipc-linux](../../04-linux-system-programming/ipc-linux.md) |
| [07 — Sockets](07-sockets.md) | 56–62, 64 | 1149–1324, 1375–1400 | | [13-networking](../../13-networking/) |
| [08 — Memory mappings & shared libraries](08-memory-mappings-and-shared-libraries.md) | 41, 42, 49, 50 | 833–876, 1017–1056 | | [07-shared-libraries](../../07-shared-libraries/) |
| [09 — File systems & attributes](09-filesystems-and-file-attributes.md) | 8, 14–19 | 153–166, 251–386 | | [03/memory-management](../../03-operating-system/memory-management.md) |
| [10 — Daemon, priority, security](10-daemons-priority-security.md) | 9–12, 34–40 | 167–232, 699–832 | | [08/constraints](../../08-embedded-systems/constraints.md) |

**Phụ lục** (không làm cụm riêng — tra khi cần): A `strace` (tr. 1401) · B `getopt` (1405) · C ép kiểu NULL (1413) · D cấu hình kernel (1417) · E nguồn đọc thêm (1419) · F lời giải bài tập (1425).

---

## 📌 Tiến độ viết cụm

| Cụm | Trạng thái |
|---|---|
| 01 — Khái niệm nền & File I/O 🎯 | ✅ **Xong** (đối chiếu PDF, neo trang thật) |
| 02 — Process & program execution 🎯 | ✅ **Xong** (đối chiếu PDF, neo trang thật) |
| 03 — Signals & timers 🎯 | ✅ **Xong** (đối chiếu PDF, neo trang thật) |
| 05 — Alternative I/O models 🎯 | ✅ **Xong** (đối chiếu PDF, neo trang thật) |
| 04 — Threads 🎯 | ✅ **Xong** (đối chiếu PDF, neo trang thật) |
| 06 — IPC 🎯 | ✅ **Xong** (đối chiếu PDF, neo trang thật) |
| 07 — Sockets & terminals | ✅ **Xong** |
| 08 — Memory mappings & shared libraries | ✅ **Xong** |
| 09 — File systems & attributes | ✅ **Xong** |
| 10 — Daemon, priority, security | ✅ **Xong** |

> Viết theo lối **cụm 🎯 trước** đúng như [quy ước §7 bước 3](../README.md) cho phép với sách rất dày. **✅ XONG CẢ 10/10 CỤM** — 6 cụm 🎯 (file I/O · process/exec · signals · threads · epoll · IPC) đầu tư sâu, 4 cụm tra cứu (sockets · mmap/shared-lib · filesystem · daemon/security) viết đủ theo quy ước.

---
⬅️ [Về index book summaries](../README.md)
