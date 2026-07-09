# Linux Kernel Development — Robert Love (3rd edition, 2010)

> **Nguồn summary:** kiến thức Claude, **đã đối chiếu mục lục + số trang theo PDF** `Linux.Kernel.Development.3rd.Edition.pdf` (ISBN 978-0-672-32946-3); nội dung chi tiết viết từ kiến thức, đối chiếu chọn lọc. Số trang `(tr. X)` theo bản PDF này.
> **Vì sao đọc cuốn này:** kernel internals dễ đọc nhất từng được viết — đúng tầng bên dưới công việc BSP: interrupt, bottom halves, kernel locking, kmalloc/slab, VFS/block. Là "phần chìm" giải thích cho mọi API driver bạn gọi hằng ngày.
> ⚠️ **Sách viết thời kernel 2.6.34 (2010)** — khung khái niệm vẫn đúng, nhưng nhiều chi tiết đã đổi; mỗi cụm có mục **"⚠️ Đã thay đổi so với sách"** cập nhật đến kernel hiện đại (~6.x). Đọc sách này *kèm* các ghi chú đó, đừng đọc chay.

---

## TL;DR — cuốn sách này dạy gì

Kernel không phải hộp đen: nó là một chương trình C với các ràng buộc đặc biệt (không libc, stack nhỏ cố định, không được ngủ tùy tiện, chạy đồng thời trên mọi CPU). Sách đi qua các hệ cơ quan: process & scheduler (task_struct, CFS), syscall, **interrupt & bottom halves**, **locking đủ loại và khi nào dùng gì**, timer/jiffies, **cấp phát bộ nhớ trong kernel** (page/zone/kmalloc/slab/vmalloc/per-CPU), address space process (mm_struct/VMA), VFS bốn object, block layer & I/O scheduler, page cache/writeback, device model & module, debug kernel. Giọng văn: giải thích *vì sao thiết kế như vậy* — đúng chất interview.

## 🕐 Đọc gì nếu chỉ có N giờ

- **~3 giờ:** [interrupts-bottomhalves.md](interrupts-bottomhalves.md) + [sync-timers.md](sync-timers.md) — trái tim của nghề driver/BSP.
- **~6 giờ:** thêm [memory.md](memory.md) (kmalloc/GFP/slab — dùng hằng ngày khi viết driver).
- **~9 giờ:** thêm [process-sched-syscalls.md](process-sched-syscalls.md) và phần device model trong [modules-debug.md](modules-debug.md).

## 🎯 Lộ trình ôn nhanh trước interview (BSP)

1. **Interrupt & bottom halves** ([interrupts-bottomhalves.md](interrupts-bottomhalves.md)) — request_irq, interrupt context, softirq/tasklet/workqueue chọn cái nào.
2. **Kernel locking** ([sync-timers.md](sync-timers.md)) — spinlock vs mutex trong kernel, spin_lock_irqsave, lock với bottom half, barriers.
3. **Cấp phát bộ nhớ kernel** ([memory.md](memory.md)) — kmalloc vs vmalloc, GFP_KERNEL vs GFP_ATOMIC, slab, kernel stack nhỏ.
4. **Syscall & process kernel-side** ([process-sched-syscalls.md](process-sched-syscalls.md)) — task_struct, current, copy_*_user, CFS nội tạng.
5. **Device model & sysfs** ([modules-debug.md](modules-debug.md)) — kobject, bus/device/driver — nền của mọi thứ trong /sys.
6. **VFS & block** ([vfs-block-pagecache.md](vfs-block-pagecache.md)) — bốn object VFS, bio, page cache (đọc sau cùng, đã có nền OSTEP).

## 🗺️ Bản đồ: chương ↔ file ↔ topic liên quan

| File | Chương (trang) | Nội dung | 🎯 BSP |
|------|----------------|----------|--------|
| [process-sched-syscalls.md](process-sched-syscalls.md) | 3 (tr. 23), 4 (tr. 41), 5 (tr. 69) | task_struct, fork/CoW, kernel threads; CFS implementation; syscall từ trap đến sys_* | 🎯 |
| [interrupts-bottomhalves.md](interrupts-bottomhalves.md) | 7 (tr. 113), 8 (tr. 133) | request_irq, interrupt context; softirq/tasklet/workqueue | 🎯🎯 |
| [sync-timers.md](sync-timers.md) | 9 (tr. 161), 10 (tr. 175), 11 (tr. 207) | atomic, spinlock, mutex, completion, seqlock, barriers; jiffies/HZ, timers, delay | 🎯🎯 |
| [memory.md](memory.md) | 12 (tr. 231), 15 (tr. 305) | page/zone, kmalloc/GFP, vmalloc, slab, kernel stack, per-CPU; mm_struct, VMA, page tables | 🎯 |
| [vfs-block-pagecache.md](vfs-block-pagecache.md) | 13 (tr. 261), 14 (tr. 289), 16 (tr. 323) | superblock/inode/dentry/file; bio, I/O scheduler; page cache & writeback | |
| [modules-debug.md](modules-debug.md) | 17 (tr. 337), 18 (tr. 363), 19 (tr. 379), 20 (tr. 395) | kobject/sysfs, device model, module; printk/oops/SysRq; portability | 🎯 (ch.17) |

Chương 1–2 (giới thiệu, build kernel — tr. 1–21) và chương 6 (kernel data structures — list_head, kfifo, rbtree, tr. 85–111) không có file riêng: ý đáng giá của ch. 6 được nhúng vào các cụm dùng chúng (list_head ở process, rbtree ở CFS/VMA); ch. 2 đã có [melp/bootloader-kernel.md](../melp/bootloader-kernel.md) phủ hiện đại hơn.

Liên kết repo: [03-operating-system/](../../03-operating-system/), [05-drivers-device-tree/](../../05-drivers-device-tree/), [09-debugging/](../../09-debugging/), nền lý thuyết: [ostep/](../ostep/README.md).
