# 🕳️ Gap Register — Lỗ hổng tài liệu & Backlog

> Danh sách **"chủ đề đáng có mà repo chưa có (hoặc còn mỏng)"**, xếp theo ưu tiên. Mục đích: để repo lớn lên **có chủ đích**, không ngẫu hứng. Cập nhật mỗi khi phát hiện lỗ hổng lúc ôn (theo [quy trình](README.md): ghi vào đây, không dừng mạch ôn để lấp ngay).
>
> **Cột trạng thái:** ⬜ chưa có · 🟡 có rải rác trong book summary/question bank nhưng **chưa có topic doc riêng** · ✅ đã lấp.
> **Ưu tiên:** 🔴 cao (hay hỏi + đang thiếu) · 🟠 vừa · 🟢 thấp/dài hạn.

---

## Nhóm BSP (Embedded / kernel / phần cứng)

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **DMA & cache coherency** | 🔴 | 🟡 | bank [BSP-011](../mock-interview/bank/bsp.md) (coherent vs streaming), rải trong [lkd/memory](../../15-book-summaries/lkd/04-memory.md) | Cần topic doc: coherent vs streaming, `dma_map_*`, cache flush/invalidate, IOMMU/SMMU, scatter-gather, dma_addr_t. Hay hỏi ở BSP. |
| **Yocto / Buildroot** | 🔴 | ✅ **đã lấp** | [06/yocto.md](../../06-build-systems/yocto.md) + bank [BLD](../mock-interview/bank/build-systems.md) (CMake/Yocto/CI) + [BSP-017…019](../mock-interview/bank/bsp.md) | Đã thêm topic doc Yocto (BitBake, recipe/layer/bbappend, machine/BSP layer, sstate, devtool, SDK, CVE) + domain BLD trong bank. |
| **PCI & USB drivers** | 🔴 | ✅ **đã lấp** | [05/pci-usb-drivers.md](../../05-drivers-device-tree/pci-usb-drivers.md) + bank [DRV-019…027](../mock-interview/bank/drivers-embedded.md) | JD Datalogic yêu cầu (I2C/SPI/**PCI/USB**). Trước repo chỉ có I2C/SPI/UART; nay thêm PCI (config space/BAR/MSI-X/bus master) + USB (descriptor/URB/gadget). |
| **Power management** | 🟠 | 🟡 | [melp/drivers-init-power](../../15-book-summaries/melp/drivers-init-power.md) cụm 3 | Chưa có topic doc. 4 tầng (cpufreq/cpuidle/runtime PM/system suspend), wakeup source, OPP, DVFS. |
| **Secure boot / TF-A chiều sâu** | 🟠 | 🟡 | [melp/bootloader-kernel](../../15-book-summaries/melp/bootloader-kernel.md) (🆕 phần bổ sung) | Mới ở mức khái niệm. Cần: chain of trust chi tiết, BL1–BL33, eFuse/OTP, verified boot của U-Boot (FIT + khóa), anti-rollback. |
| **Bus protocol chiều sâu (I2C/SPI/UART)** | 🟠 | ✅ **đã lấp** | [05/bus-protocols.md](../../05-drivers-device-tree/bus-protocols.md) + bank [BUS](../mock-interview/bank/drivers-embedded.md) | Đã có topic doc riêng (2026-08-09): bảng so sánh, khung UART + vì sao sai baud ra rác, open-drain/pull-up, START/STOP · ACK/NACK · **clock stretching** · arbitration, CPOL/CPHA 4 mode có waveform, flow control, góc device tree/Linux + 5 câu phỏng vấn. |
| **Bare-metal / MCU / RTOS fundamentals** | 🔴 | ✅ **đã lấp** | [08-embedded-systems](../../08-embedded-systems/): [bare-metal-c](../../08-embedded-systems/bare-metal-c.md), [memory-and-startup](../../08-embedded-systems/memory-and-startup.md), [interrupts-bare-metal](../../08-embedded-systems/interrupts-bare-metal.md), [rtos-programming](../../08-embedded-systems/rtos-programming.md), [hardware-debug](../../08-embedded-systems/hardware-debug.md) + bank [EMB](../mock-interview/bank/embedded-fundamentals.md) | Trước đây topic 08 chỉ có góc SoC/Linux; nay đủ nền Embedded SW chung: C/thanh ghi, startup/linker, ISR bare-metal, RTOS programming, debug HW. |
| **LDD3 / Bootlin summary** | 🟠 | ⬜ | — (trong hàng đợi sách) | Char driver, ioctl, interrupt, DMA từ góc viết driver thật. Đã lên kế hoạch (xem [15/README](../../15-book-summaries/README.md)). |
| **pinctrl / clock / regulator framework** | 🟢 | 🟡 | [melp/bootloader-kernel](../../15-book-summaries/melp/bootloader-kernel.md) (nhắc pinctrl) | Chi tiết framework: consumer/provider, clk_get/prepare/enable, regulator_get. |
| **Device model kernel sâu (kobject/sysfs/uevent)** | 🟢 | 🟡 | [lkd/modules-debug](../../15-book-summaries/lkd/06-modules-debug.md) cụm 1 | Có khung; có thể đào sâu thêm cho vị trí driver-heavy. |

---

## Nhóm C++ / System Software

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **Memory model & lock-free** (atomic ordering, happens-before, ABA, hazard pointer) | 🟠 | ✅ **đã lấp** | **Ch. 5** ([05-memory-model](../../15-book-summaries/cpp-concurrency/05-memory-model.md) — happens-before, synchronizes-with, 6 memory_order, CAS, fences) + **Ch. 7** ([07-lock-free-structures](../../15-book-summaries/cpp-concurrency/07-lock-free-structures.md) — lock-free vs wait-free, 3 kỹ thuật reclamation, hazard pointer, **ABA problem**) | Trước đây **không tài liệu nào trong repo phủ**. Đã lấp trọn bằng ch. 5 + ch. 7 của C++ Concurrency in Action. |
| **C++20/23 depth** (ranges, concepts, coroutines) | 🟠 | 🟡 | [cpp-mindset](../../15-book-summaries/cpp-mindset/README.md) (concepts/ranges chạm nhẹ), [EMC++](../../15-book-summaries/effective-modern-cpp.md) (dừng C++14) | EMC++ chỉ tới C++14. Cần bổ sung: ranges/views sâu, concepts thay SFINAE, `std::expected`, `std::span`; **coroutines chưa có ở đâu**. |
| **Template metaprogramming sâu** (SFINAE, CRTP, type traits) | 🟠 | 🟡 | [01/templates](../../01-cpp-fundamentals/templates.md) (cơ bản), [cpp-mindset/abstraction-in-detail](../../15-book-summaries/cpp-mindset/abstraction-in-detail.md) (traits) | Còn mỏng. CRTP (static polymorphism), tag dispatch, `enable_if`, variadic template, fold expression. |
| **Coroutines (C++20)** | 🟢 | ⬜ | — | Chưa có. co_await/co_yield, generator, task; ít hỏi ở embedded nhưng xuất hiện ở vị trí C++ hiện đại. |
| **TLPI summary** (userspace API sâu) | 🟢 | ⬜ | — (trong hàng đợi sách) | The Linux Programming Interface — bổ trợ topic 03–04. Đã lên kế hoạch, lùi ưu tiên sau BSP. |
| **Networking chiều sâu** | 🟢 | 🟡 | [13-networking](../../13-networking/) | Topic 14 còn nhẹ. TLS handshake, HTTP/2, MQTT chi tiết nếu JD liên quan mạng. |

---

## Nhóm chung (kỹ năng phỏng vấn)

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú |
|--------|---------|-----------|---------------|---------|
| **Behavioral / STAR stories** | 🟠 | ✅ **đã lấp** | bank [BEH-001…009](../mock-interview/bank/behavioral.md) + track `behavioral` của `/mock` | Kiểm tra đã đủ câu chuyện STAR chưa (conflict, failure, leadership, khó khăn kỹ thuật). |
| **Mock interview scaffolding** | 🟢 | ✅ **đã lấp** | [14-prep/mock-interview/](../mock-interview/) | Module phỏng vấn thử tương tác (`/mock`) + ngân hàng câu hỏi duy nhất (bank/) + track/type/sessions/weak-register/coding-arena(+reviewed/). |
| **System design practice (embedded)** | 🟠 | 🟡 | [10/system-design](../../10-thinking/system-design.md), bank [SD](../mock-interview/bank/system-design.md) | Có khung; nên thêm 2–3 bài tập system design nhúng đã giải mẫu (vd: thiết kế firmware update, data pipeline sensor). |

---

## Cách lấp một lỗ hổng

Khi quyết định lấp một mục:
1. Nếu là **kiến thức nền** → viết vào **topic** tương ứng (vd DMA → thêm file vào [05-drivers-device-tree/](../../05-drivers-device-tree/) hoặc topic mới; Yocto → bổ sung [06-build-systems/](../../06-build-systems/)).
2. Nếu là **chiều sâu từ một cuốn sách** → làm book summary (theo [quy ước 16/README](../../15-book-summaries/README.md)).
3. Cập nhật lại ô *Đọc/Hiểu* tương ứng trong [datalogic-plan](datalogic-plan.md): bỏ dấu 🕳️, trỏ link tới tài liệu mới.
4. Đổi trạng thái ở bảng trên thành ✅.
