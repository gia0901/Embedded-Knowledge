# Linux Kernel Development — Robert Love (3rd edition, 2010)

> **Nguồn summary:** kiến thức Claude, **đã đối chiếu mục lục + số trang theo PDF** `Linux.Kernel.Development.3rd.Edition.pdf` (ISBN 978-0-672-32946-3); nội dung chi tiết viết từ kiến thức, đối chiếu chọn lọc. Số trang `(tr. X)` theo bản PDF này.
> **Vì sao đọc cuốn này:** kernel internals dễ đọc nhất từng được viết — đúng tầng bên dưới công việc BSP: interrupt, bottom halves, kernel locking, kmalloc/slab, VFS/block. Là "phần chìm" giải thích cho mọi API driver bạn gọi hằng ngày.
> ⚠️ **Sách viết thời kernel 2.6.34 (2010)** — khung khái niệm vẫn đúng, nhưng nhiều chi tiết đã đổi; mỗi cụm có mục **"⚠️ Đã thay đổi so với sách"** cập nhật đến kernel hiện đại (~6.x). Đọc sách này *kèm* các ghi chú đó, đừng đọc chay.

---

## TL;DR — cuốn sách này dạy gì

Kernel không phải hộp đen: nó là một chương trình C với các ràng buộc đặc biệt (không libc, stack nhỏ cố định, không được ngủ tùy tiện, chạy đồng thời trên mọi CPU). Sách đi qua các hệ cơ quan: process & scheduler (task_struct, CFS), syscall, **interrupt & bottom halves**, **locking đủ loại và khi nào dùng gì**, timer/jiffies, **cấp phát bộ nhớ trong kernel** (page/zone/kmalloc/slab/vmalloc/per-CPU), address space process (mm_struct/VMA), VFS bốn object, block layer & I/O scheduler, page cache/writeback, device model & module, debug kernel. Giọng văn: giải thích *vì sao thiết kế như vậy* — đúng chất interview.

## 🕐 Đọc gì nếu chỉ có N giờ

- **~3 giờ:** [02-interrupts-bottomhalves.md](02-interrupts-bottomhalves.md) + [03-sync-timers.md](03-sync-timers.md) — trái tim của nghề driver/BSP.
- **~6 giờ:** thêm [04-memory.md](04-memory.md) (kmalloc/GFP/slab — dùng hằng ngày khi viết driver).
- **~9 giờ:** thêm [01-process-sched-syscalls.md](01-process-sched-syscalls.md) và phần device model trong [06-modules-debug.md](06-modules-debug.md).

## 🎯 Lộ trình ôn nhanh trước interview (BSP)

1. **Interrupt & bottom halves** ([02-interrupts-bottomhalves.md](02-interrupts-bottomhalves.md)) — request_irq, interrupt context, softirq/tasklet/workqueue chọn cái nào.
2. **Kernel locking** ([03-sync-timers.md](03-sync-timers.md)) — spinlock vs mutex trong kernel, spin_lock_irqsave, lock với bottom half, barriers.
3. **Cấp phát bộ nhớ kernel** ([04-memory.md](04-memory.md)) — kmalloc vs vmalloc, GFP_KERNEL vs GFP_ATOMIC, slab, kernel stack nhỏ.
4. **Syscall & process kernel-side** ([01-process-sched-syscalls.md](01-process-sched-syscalls.md)) — task_struct, current, copy_*_user, CFS nội tạng.
5. **Device model & sysfs** ([06-modules-debug.md](06-modules-debug.md)) — kobject, bus/device/driver — nền của mọi thứ trong /sys.
6. **VFS & block** ([05-vfs-block-pagecache.md](05-vfs-block-pagecache.md)) — bốn object VFS, bio, page cache (đọc sau cùng, đã có nền OSTEP).

## 🗺️ Bản đồ: chương ↔ file ↔ topic liên quan

> **Quy ước tên file: đánh số theo thứ tự đọc (thứ tự chương sách)** — `01-…` → `06-…`, nhìn tên là biết đang ở phần nào. Đây là **cụm chủ đề gộp nhiều chương** (không 1:1 như cuốn C++ Concurrency), số file = thứ tự trình bày của sách. Mỗi file có link **⏮ file trước / file sau ⏭** ở đầu để đọc liền mạch. (Lộ trình BSP ở trên là **thứ tự ôn ưu tiên**, khác thứ tự file — cứ theo số file để đọc tuần tự, hoặc theo lộ trình để ôn nhanh.)

| File | Chương (trang) | Nội dung | 🎯 BSP |
|------|----------------|----------|--------|
| [01-process-sched-syscalls.md](01-process-sched-syscalls.md) | 3 (tr. 23), 4 (tr. 41), 5 (tr. 69) | task_struct, fork/CoW, kernel threads; CFS implementation; syscall từ trap đến sys_* | 🎯 |
| [02-interrupts-bottomhalves.md](02-interrupts-bottomhalves.md) | 7 (tr. 113), 8 (tr. 133) | request_irq, interrupt context; softirq/tasklet/workqueue | 🎯🎯 |
| [03-sync-timers.md](03-sync-timers.md) | 9 (tr. 161), 10 (tr. 175), 11 (tr. 207) | atomic, spinlock, mutex, completion, seqlock, barriers; jiffies/HZ, timers, delay | 🎯🎯 |
| [04-memory.md](04-memory.md) | 12 (tr. 231), 15 (tr. 305) | page/zone, kmalloc/GFP, vmalloc, slab, kernel stack, per-CPU; mm_struct, VMA, page tables | 🎯 |
| [05-vfs-block-pagecache.md](05-vfs-block-pagecache.md) | 13 (tr. 261), 14 (tr. 289), 16 (tr. 323) | superblock/inode/dentry/file; bio, I/O scheduler; page cache & writeback | |
| [06-modules-debug.md](06-modules-debug.md) | 17 (tr. 337), 18 (tr. 363), 19 (tr. 379), 20 (tr. 395) | kobject/sysfs, device model, module; printk/oops/SysRq; portability | 🎯 (ch.17) |

Chương 1–2 (giới thiệu, build kernel — tr. 1–21) và chương 6 (kernel data structures — list_head, kfifo, rbtree, tr. 85–111) không có file riêng: ý đáng giá của ch. 6 được nhúng vào các cụm dùng chúng (list_head ở process, rbtree ở CFS/VMA); ch. 2 đã có [melp/bootloader-kernel.md](../melp/bootloader-kernel.md) phủ hiện đại hơn.

Liên kết repo: [03-operating-system/](../../03-operating-system/), [05-drivers-device-tree/](../../05-drivers-device-tree/), [09-debugging/](../../09-debugging/), nền lý thuyết: [ostep/](../ostep/README.md).

---

## Ghi chú khi đọc/viết (quy ước)

- **✅ TỰ CHỨA — CHO PHÉP LẶP** (theo [quy ước chung](../README.md) Mục 1): mỗi file viết **đầy đủ, mạch lạc tại chỗ**, kể cả khi trùng với [ostep/](../ostep/README.md) (lý thuyết OS), [melp/](../melp/README.md) (BSP) hay các topic 03/05/09. Người đọc **không phải nhảy sang tài liệu khác giữa chừng**; link chéo (như dòng "Liên kết repo" trên, hay các note đầu file) chỉ đặt dạng **"đọc thêm/nền lý thuyết (tùy chọn)"** để tự tra khi muốn đào sâu. Cùng một khái niệm (vd spinlock, interrupt context) có thể xuất hiện ở nhiều file — đó là **có chủ đích** để mỗi cụm đọc trọn vẹn một mình.
- **Đọc tuần tự theo số file** (`01-…` → `06-…`) hoặc theo **lộ trình BSP** ở trên; mỗi file có điều hướng ⏮/⏭ ở đầu.
- **⚠️ Sách 2010 (kernel 2.6.34)** — mỗi cụm có mục **"⚠️ Đã thay đổi so với sách"** cập nhật đến kernel ~6.x. Không đọc chay bản gốc.
