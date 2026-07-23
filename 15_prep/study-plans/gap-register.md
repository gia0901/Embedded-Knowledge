# 🕳️ Gap Register — Lỗ hổng tài liệu & Backlog

> Danh sách **"chủ đề đáng có mà repo chưa có (hoặc còn mỏng)"**, xếp theo ưu tiên. Mục đích: để repo lớn lên **có chủ đích**, không ngẫu hứng. Cập nhật mỗi khi phát hiện lỗ hổng lúc ôn (theo [quy trình](README.md): ghi vào đây, không dừng mạch ôn để lấp ngay).
>
> **Cột trạng thái:** ⬜ chưa có · 🟡 có rải rác trong book summary/question bank nhưng **chưa có topic doc riêng** · ✅ đã lấp.
> **Ưu tiên:** 🔴 cao (hay hỏi + đang thiếu) · 🟠 vừa · 🟢 thấp/dài hạn.

---

## Nhóm BSP (Embedded / kernel / phần cứng)

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **DMA & cache coherency** | 🔴 | 🟡 | [04_bsp C2–C3](../technical_round/04_bsp_question_bank.md), rải trong [lkd/memory](../../16-book-summaries/lkd/memory.md) | Cần topic doc: coherent vs streaming, `dma_map_*`, cache flush/invalidate, IOMMU/SMMU, scatter-gather, dma_addr_t. Hay hỏi ở BSP. |
| **Yocto / Buildroot** | 🔴 | 🟡 | [melp/build-systems](../../16-book-summaries/melp/build-systems.md) | Topic 06 **thiếu hẳn** — chỉ có Make/CMake/cross-compile. Đa số shop BSP dùng Yocto; cần bổ sung vào 06 (recipe, layer, bbappend, machine/distro, sstate, devtool). |
| **Power management** | 🟠 | 🟡 | [melp/drivers-init-power](../../16-book-summaries/melp/drivers-init-power.md) cụm 3 | Chưa có topic doc. 4 tầng (cpufreq/cpuidle/runtime PM/system suspend), wakeup source, OPP, DVFS. |
| **Secure boot / TF-A chiều sâu** | 🟠 | 🟡 | [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) (🆕 phần bổ sung) | Mới ở mức khái niệm. Cần: chain of trust chi tiết, BL1–BL33, eFuse/OTP, verified boot của U-Boot (FIT + khóa), anti-rollback. |
| **Bus protocol chiều sâu (I2C/SPI/UART)** | 🟠 | 🟡 | [02_question_bank Q31–34](../technical_round/02_question_bank.md), [11/drivers](../../11-interview-questions/drivers.md) | Có so sánh mức khái niệm; thiếu timing/điện: CPOL/CPHA waveform, clock stretching, START/STOP, arbitration, flow control, tốc độ. |
| **LDD3 / Bootlin summary** | 🟠 | ⬜ | — (trong hàng đợi sách) | Char driver, ioctl, interrupt, DMA từ góc viết driver thật. Đã lên kế hoạch (xem [16/README](../../16-book-summaries/README.md)). |
| **pinctrl / clock / regulator framework** | 🟢 | 🟡 | [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) (nhắc pinctrl) | Chi tiết framework: consumer/provider, clk_get/prepare/enable, regulator_get. |
| **Device model kernel sâu (kobject/sysfs/uevent)** | 🟢 | 🟡 | [lkd/modules-debug](../../16-book-summaries/lkd/modules-debug.md) cụm 1 | Có khung; có thể đào sâu thêm cho vị trí driver-heavy. |

---

## Nhóm C++ / System Software

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **Memory model & lock-free** (atomic ordering, happens-before, ABA, hazard pointer) | 🟠 | 🟡 **đang lấp** | Sách [cpp-concurrency](../../16-book-summaries/cpp-concurrency/README.md) đã dựng khung (ch. 5 + ch. 7) | Trước đây **không tài liệu nào trong repo phủ**; đang được lấp bằng summary cuốn C++ Concurrency in Action. Đổi thành ✅ khi 2 chương đó viết xong. |
| **C++20/23 depth** (ranges, concepts, coroutines) | 🟠 | 🟡 | [cpp-mindset](../../16-book-summaries/cpp-mindset/README.md) (concepts/ranges chạm nhẹ), [EMC++](../../16-book-summaries/effective-modern-cpp.md) (dừng C++14) | EMC++ chỉ tới C++14. Cần bổ sung: ranges/views sâu, concepts thay SFINAE, `std::expected`, `std::span`; **coroutines chưa có ở đâu**. |
| **Template metaprogramming sâu** (SFINAE, CRTP, type traits) | 🟠 | 🟡 | [01/templates](../../01-cpp-fundamentals/templates.md) (cơ bản), [cpp-mindset/abstraction-in-detail](../../16-book-summaries/cpp-mindset/abstraction-in-detail.md) (traits) | Còn mỏng. CRTP (static polymorphism), tag dispatch, `enable_if`, variadic template, fold expression. |
| **Coroutines (C++20)** | 🟢 | ⬜ | — | Chưa có. co_await/co_yield, generator, task; ít hỏi ở embedded nhưng xuất hiện ở vị trí C++ hiện đại. |
| **TLPI summary** (userspace API sâu) | 🟢 | ⬜ | — (trong hàng đợi sách) | The Linux Programming Interface — bổ trợ topic 03–04. Đã lên kế hoạch, lùi ưu tiên sau BSP. |
| **Networking chiều sâu** | 🟢 | 🟡 | [14-networking](../../14-networking/) | Topic 14 còn nhẹ. TLS handshake, HTTP/2, MQTT chi tiết nếu JD liên quan mạng. |

---

## Nhóm chung (kỹ năng phỏng vấn)

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú |
|--------|---------|-----------|---------------|---------|
| **Behavioral / STAR stories** | 🟠 | 🟡 | [03_behavior](../technical_round/03_behavior.md) | Kiểm tra đã đủ câu chuyện STAR chưa (conflict, failure, leadership, khó khăn kỹ thuật). |
| **Mock interview scaffolding** | 🟢 | ⬜ | — | Có thể thêm bộ "đề mock" gồm 5–6 câu trộn cho một buổi tập nói. |
| **System design practice (embedded)** | 🟠 | 🟡 | [10/system-design](../../10-thinking/system-design.md), [11/system-design](../../11-interview-questions/system-design.md) | Có khung; nên thêm 2–3 bài tập system design nhúng đã giải mẫu (vd: thiết kế firmware update, data pipeline sensor). |

---

## Cách lấp một lỗ hổng

Khi quyết định lấp một mục:
1. Nếu là **kiến thức nền** → viết vào **topic** tương ứng (vd DMA → thêm file vào [05-drivers-device-tree/](../../05-drivers-device-tree/) hoặc topic mới; Yocto → bổ sung [06-build-systems/](../../06-build-systems/)).
2. Nếu là **chiều sâu từ một cuốn sách** → làm book summary (theo [quy ước 16/README](../../16-book-summaries/README.md)).
3. Cập nhật lại checklist trong [bsp-plan](bsp-plan.md)/[cpp-systemsw-plan](cpp-systemsw-plan.md): bỏ dấu 🕳️, trỏ link tới tài liệu mới.
4. Đổi trạng thái ở bảng trên thành ✅.
