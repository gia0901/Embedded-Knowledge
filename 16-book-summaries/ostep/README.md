# Operating Systems: Three Easy Pieces (OSTEP) — Remzi & Andrea Arpaci-Dusseau (bản online v1.x, ostep.org)

> **Nguồn summary:** kiến thức Claude, chưa đối chiếu PDF. Số chương theo bản online chính thức (v1.00+); ⚠️ số chương có thể lệch ±1 giữa các bản — sẽ chốt khi đối chiếu PDF.
> **Vì sao đọc cuốn này:** giáo trình OS hay nhất để hiểu *bản chất* — mỗi chương xuất phát từ một câu hỏi ("the crux") rồi xây giải pháp từng bước như người thiết kế OS thật. Khớp gần 1:1 với topic 03 và là nền lý thuyết cho topic 04, 05. Sách miễn phí hợp pháp tại ostep.org.

---

## TL;DR — cuốn sách này dạy gì

OS làm đúng **ba việc** ("three easy pieces"):

1. **Virtualization** — biến tài nguyên vật lý ít (CPU, RAM) thành ảo giác "mỗi process có riêng tất cả": CPU ảo hóa bằng **limited direct execution + context switch + scheduler**; memory ảo hóa bằng **address space + paging + TLB + swapping**.
2. **Concurrency** — khi nhiều luồng chạy đồng thời trên dữ liệu chung, cần **locks, condition variables, semaphores** xây trên hỗ trợ phần cứng (atomic instructions), và hiểu các lớp bug (race, deadlock).
3. **Persistence** — dữ liệu phải sống sót qua mất điện: **I/O devices, file system** (inode, journaling), từ đĩa quay đến SSD.

Mỗi mảnh đều theo công thức: *crux (câu hỏi cốt lõi) → giải pháp ngây thơ → vấn đề → cải tiến dần → cơ chế thật trong OS hiện đại*.

## 🕐 Đọc gì nếu chỉ có N giờ

- **~3 giờ:** [concurrency.md](concurrency.md) cụm Locks + Condition Variables + Concurrency Bugs — sát interview nhất.
- **~6 giờ:** thêm [virtualization-memory.md](virtualization-memory.md) cụm Paging + TLB + Swapping, và [virtualization-cpu.md](virtualization-cpu.md) cụm Limited Direct Execution.
- **~9 giờ:** thêm Scheduling (CPU) và Crash Consistency/Journaling (Persistence).

## 🎯 Lộ trình ôn nhanh trước interview

1. **Concurrency: Locks + CV + bugs** ([concurrency.md](concurrency.md)) — deadlock, race, producer/consumer là câu hỏi "quốc dân".
2. **Paging + TLB + multi-level page table + swapping** ([virtualization-memory.md](virtualization-memory.md)) — virtual memory hỏi sâu ở vị trí system/embedded.
3. **Limited Direct Execution + context switch** ([virtualization-cpu.md](virtualization-cpu.md)) — syscall, user/kernel mode, cái giá của context switch.
4. **Scheduling: MLFQ, CFS** ([virtualization-cpu.md](virtualization-cpu.md)) — nối thẳng sang câu hỏi realtime/priority của embedded Linux.
5. **Crash consistency + journaling** ([persistence.md](persistence.md)) — đắt giá khi bàn về eMMC/flash và mất điện đột ngột trên thiết bị nhúng.
6. **File system: inode, đường đi của read/write** ([persistence.md](persistence.md)).
7. Phần còn lại: Process API, free-space, RAID, LFS/SSD, I/O devices.

## 🗺️ Bản đồ: chương ↔ file ↔ topic liên quan

| File | Phần sách | Chương | Cụm 🎯 | Topic liên quan |
|------|-----------|--------|--------|-----------------|
| [virtualization-cpu.md](virtualization-cpu.md) | I. Virtualization (CPU) | 4–10 | LDE, Scheduling | [03/process-thread](../../03-operating-system/process-thread.md), [03/scheduling](../../03-operating-system/scheduling.md), [04/processes-signals](../../04-linux-system-programming/processes-signals.md) |
| [virtualization-memory.md](virtualization-memory.md) | I. Virtualization (Memory) | 13–24 | Paging+TLB, Multi-level PT, Swapping | [03/memory-management](../../03-operating-system/memory-management.md), [01/memory-model](../../01-cpp-fundamentals/memory-model.md) |
| [concurrency.md](concurrency.md) | II. Concurrency | 26–33 | Threads, Locks, CV, Bugs | [03/sync-primitives](../../03-operating-system/sync-primitives.md), [02/concurrency](../../02-modern-cpp/concurrency.md), [04/io-multiplexing](../../04-linux-system-programming/io-multiplexing.md) |
| [persistence.md](persistence.md) | III. Persistence | 36–45 (+48–50) | vsfs, Crash consistency | [04/file-io](../../04-linux-system-programming/file-io.md), [08/constraints](../../08-embedded-systems/constraints.md) |

Các chương dialogue/intro (1–3, 11–12, 25, 34–35...) là chuyển cảnh — không summary riêng, ý nào đáng giá đã gộp vào cụm liên quan.
