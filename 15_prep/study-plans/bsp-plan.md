# 🔧 Kế hoạch ôn — Embedded SW Engineer (BSP)

> Vị trị bạn đang phỏng vấn. Xem [cách dùng chung](README.md). Hai tầng: 🏃 **Sprint** (sát phỏng vấn) + 📚 **Nền tảng** (dài hơi).
> Câu mở màn gần như chắc chắn: *"Kể từ lúc cấp nguồn đến khi có shell trên board"* → luyện đến mức nói trôi chảy là ưu tiên số một.

---

## Bản đồ nhanh — vị trí BSP hỏi gì

```
Boot chain ──► Device Tree/probe ──► Kernel internals ──► Phần cứng
(U-Boot,       (compatible,          (interrupt, lock,     (DMA, cache,
 SPL, TF-A)     EPROBE_DEFER,         memory, bottom half)   MMIO, bus)
                pinctrl)                    │
                                            ▼
                          Storage/OTA ── Real-time ── Debug board-level
                          (MTD/UBI,      (PREEMPT_RT,  (JTAG, oops,
                           eMMC, A/B)     cyclictest)   pstore, earlycon)
                                            │
                          + Modern C++ (driver vẫn viết C/C++: volatile vs atomic, RAII)
```

Nguồn tự kiểm tra chính: **[04_bsp_question_bank.md](../technical_round/04_bsp_question_bank.md)** (A→H) + [02_question_bank.md](../technical_round/02_question_bank.md) (phần bus/debug) + [11-interview-questions/drivers](../../11-interview-questions/drivers.md).

---

## 🏃 Tầng SPRINT (≈4 tuần — chỉ mục 🎯)

### Tuần 1 — Boot chain + Device Tree (lõi tuyệt đối)

- [ ] 🎯 **Boot flow ROM→SPL→U-Boot→kernel + vì sao nhiều pha** — [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) cụm 1 · nền: [08/boot-process](../../08-embedded-systems/boot-process.md)
- [ ] 🎯 **U-Boot: env, bootargs, bootcmd, FIT, nạp ảnh** — [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) cụm 3
- [ ] 🎯 **Device Tree: compatible → probe → EPROBE_DEFER, pinctrl** — [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) cụm 2 · nền: [05/device-tree](../../05-drivers-device-tree/device-tree.md)
- [ ] 🎯 **Kernel build/Kconfig, y-vs-m (gà-trứng), vmlinux/Image/zImage/uImage** — [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) cụm 4–5
- [ ] Tự kiểm tra: [04_bsp](../technical_round/04_bsp_question_bank.md) **A1–A4, B1–B4**

### Tuần 2 — Interrupt / DMA / MMIO + kernel locking

- [ ] 🎯 **ISR không được ngủ, top/bottom half, threaded IRQ** — [lkd/interrupts-bottomhalves](../../16-book-summaries/lkd/interrupts-bottomhalves.md) · [melp/drivers-init-power](../../16-book-summaries/melp/drivers-init-power.md) cụm 1
- [ ] 🎯 **Kernel locking: spinlock vs mutex, spin_lock_irqsave, per-CPU, barrier** — [lkd/sync-timers](../../16-book-summaries/lkd/sync-timers.md) cụm 2
- [ ] 🎯 **DMA: coherent vs streaming, cache maintenance, ioremap, /dev/mem** — [04_bsp C2–C3](../technical_round/04_bsp_question_bank.md) · nền cache: [ostep/virtualization-memory](../../16-book-summaries/ostep/virtualization-memory.md) 🕳️ *(chưa có topic doc riêng — xem gap register)*
- [ ] 🎯 **Cấp phát bộ nhớ kernel: kmalloc vs vmalloc, GFP_KERNEL vs GFP_ATOMIC** — [lkd/memory](../../16-book-summaries/lkd/memory.md) cụm 1
- [ ] Tự kiểm tra: [04_bsp](../technical_round/04_bsp_question_bank.md) **C1–C4**

### Tuần 3 — Storage/OTA + Real-time + Power

- [ ] 🎯 **NAND thô vs eMMC, MTD/UBI/UBIFS vs ext4/f2fs (FTL ở đâu?)** — [melp/storage-update](../../16-book-summaries/melp/storage-update.md) cụm 1 · nền: [ostep/persistence](../../16-book-summaries/ostep/persistence.md)
- [ ] 🎯 **OTA A/B không brick: bootcount, watchdog, health check, anti-rollback** — [melp/storage-update](../../16-book-summaries/melp/storage-update.md) cụm 2
- [ ] 🎯 **PREEMPT_RT, cyclictest, priority inversion + PI, checklist app RT** — [melp/debug-realtime](../../16-book-summaries/melp/debug-realtime.md) cụm 4
- [ ] **Power management: 4 tầng (cpufreq/cpuidle/runtime PM/suspend), wakeup source** — [melp/drivers-init-power](../../16-book-summaries/melp/drivers-init-power.md) cụm 3 🕳️ *(chưa có topic doc)*
- [ ] Tự kiểm tra: [04_bsp](../technical_round/04_bsp_question_bank.md) **D1–D3, F1, G1–G2**

### Tuần 4 — Modern C++ (cho driver) + Debug + Mock

- [ ] 🎯 **volatile (MMIO) vs std::atomic (threads)** — [EMC++ Item 40](../../16-book-summaries/effective-modern-cpp.md) · [02_question_bank Q10](../technical_round/02_question_bank.md)
- [ ] 🎯 **Smart pointer, RAII, rule of 5/0** (driver userspace/tooling vẫn dùng) — [EMC++ cụm 4](../../16-book-summaries/effective-modern-cpp.md) · [02-modern-cpp](../../02-modern-cpp/)
- [ ] 🎯 **Debug board-level: earlycon, oops decode, pstore/ramoops, JTAG, reset reason** — [lkd/modules-debug](../../16-book-summaries/lkd/modules-debug.md) cụm 2 · [melp/debug-realtime](../../16-book-summaries/melp/debug-realtime.md) cụm 2 · [09/kernel-debugging](../../09-debugging/kernel-debugging.md)
- [ ] 🎯 **4 tình huống bring-up/debug** (board câm, crash hiện trường, trễ chu kỳ, chậm dần→bị kill) — [04_bsp H1–H4](../technical_round/04_bsp_question_bank.md)
- [ ] **Mock**: nói to đáp án A1 (boot flow) + B1 (probe) + C1 (ISR) + D2 (OTA) không nhìn tài liệu
- [ ] **Behavioral**: ôn STAR + "vì sao rời công ty cũ" + câu hỏi ngược — [technical_round/03_behavior.md](../technical_round/03_behavior.md)
- [ ] **Bus protocol**: I2C/SPI/UART so sánh, CPOL/CPHA, khi nào chọn cái nào — [02_question_bank Q31–34](../technical_round/02_question_bank.md) 🕳️ *(chiều sâu timing/điện chưa có topic)*

---

## 📚 Tầng NỀN TẢNG (dài hơi — phủ toàn diện)

Ngoài các mục sprint ở trên, phủ nốt để chắc gốc:

**Kernel & driver (đọc trọn thay vì chỉ 🎯):**
- [ ] [lkd/](../../16-book-summaries/lkd/README.md) — cả 6 file (process/sched/syscall, VFS/block/pagecache, device model đầy đủ)
- [ ] [05-drivers-device-tree/](../../05-drivers-device-tree/) — driver-basics, kernel-userspace đầy đủ
- [ ] [melp/](../../16-book-summaries/melp/README.md) — toolchain-rootfs, build-systems (Yocto) 🕳️

**OS nền tảng (lý thuyết dưới BSP):**
- [ ] [ostep/](../../16-book-summaries/ostep/README.md) — cả 4 file (CPU/memory/concurrency/persistence)
- [ ] [03-operating-system/](../../03-operating-system/) — đầy đủ 5 file
- [ ] [04-linux-system-programming/](../../04-linux-system-programming/) — file-io, signals, epoll, IPC

**Phần cứng & tối ưu:**
- [ ] [cpp-mindset/understanding-the-machine](../../16-book-summaries/cpp-mindset/understanding-the-machine.md) — cache, SIMD, TLB, affinity
- [ ] [08-embedded-systems/](../../08-embedded-systems/) — architecture, rtos-vs-linux, constraints

**Build & shared library (BSP cũng dựng build system):**
- [ ] [06-build-systems/](../../06-build-systems/) — makefile, cmake, cross-compilation 🕳️ *(thiếu Yocto/Buildroot)*
- [ ] [cpp-mindset/modularity](../../16-book-summaries/cpp-mindset/modularity.md) — -fPIC, export symbol, tách build

**Debug toàn diện:**
- [ ] [09-debugging/](../../09-debugging/) — mindset, gdb, tools, memory-bugs, kernel-debugging

---

## ✅ Vòng tự kiểm tra

Nguồn câu hỏi theo độ ưu tiên:
1. **[04_bsp_question_bank.md](../technical_round/04_bsp_question_bank.md)** — 24 câu chuyên sâu BSP (A→H), có bảng map câu↔tài liệu cuối file. **Đây là bộ chính.**
2. [02_question_bank.md](../technical_round/02_question_bank.md) — nền chung (C++/IPC/bus/debug), phần 3–6 trúng BSP.
3. [11-interview-questions/](../../11-interview-questions/) — drivers, operating-system, linux, debugging (đáp án ẩn, phân theo độ khó).
4. [01_theory.md](../technical_round/01_theory.md) — lý thuyết cốt lõi (mục C–F trúng BSP).

**Chuẩn "đạt":** nói to được đáp án + nêu được **bẫy thường gặp** (các câu 🎯 đều có mục này) mà không nhìn tài liệu.

---

## 🕳️ Lỗ hổng tài liệu cho vị trí BSP

Các mục đánh 🕳️ ở trên đang thiếu topic doc riêng (mới nằm rải trong book summary/question bank). Xem [gap-register.md](gap-register.md) mục "BSP" — ưu tiên cao nhất: **DMA & cache coherency**, **Yocto/Buildroot**. Nếu còn thời gian trước phỏng vấn và các mục 🎯 đã vững, lấp hai lỗ hổng này trước.
