# 🎯 Datalogic — Embedded Linux Engineer (C++17, Linux) · Plan 4 tuần

> Kế hoạch ôn **nhắm đúng JD** [Embedded Linux Engineer @ Datalogic Việt Nam](https://itviec.com/it-jobs/embedded-linux-engineer-c-linux-datalogic-viet-nam-2835). Dùng chung cả **4 tuần** (nền vững) lẫn **2 tuần** (nước rút — **gộp đôi khối lượng mỗi ngày**: làm 2 buổi/ngày thay vì 1).
> Không phải kiến thức mới — mọi mục **link tới tài liệu + bank câu hỏi** sẵn có. Ký hiệu: `- [ ]` chưa xong · `- [x]` xong · 🎯 xác suất gặp cao · 🎤 phiên mock.

---

## JD → tài liệu (bản đồ)

| Trụ JD | Đọc | Test (bank/mock) |
|---|---|---|
| **C++17** | [02-modern-cpp](../../02-modern-cpp/), [EMC summary](../../16-book-summaries/effective-modern-cpp.md) | `CPP-*` (nhất là 026, 037, 040–051 emc) |
| **Linux userspace + debug** 🎯 | [04-linux-system-programming](../../04-linux-system-programming/), [09-debugging](../../09-debugging/) | `LNX-*`, `DBG-*` |
| **Kernel space** | [05-drivers-device-tree](../../05-drivers-device-tree/), [lkd](../../16-book-summaries/lkd/) | `DRV-*`, `BSP-B` |
| **Drivers I2C/SPI/PCI/USB** 🎯 | [driver-basics](../../05-drivers-device-tree/driver-basics.md), [pci-usb-drivers](../../05-drivers-device-tree/pci-usb-drivers.md) | `BUS-*`, `DRV-019…027` |
| **Yocto + CMake/CI** | [yocto](../../06-build-systems/yocto.md), [cmake](../../06-build-systems/cmake.md), [cross-compilation](../../06-build-systems/cross-compilation.md) | `BLD-*`, `BSP-017…019` |
| **Design subsystem + lead + code review** | [10/system-design](../../10-thinking/system-design.md), [07-shared-libraries](../../07-shared-libraries/) | `SD-*`, `BEH-*` |

**Track mock riêng cho JD:** `/mock ... track datalogic` (trộn đúng các domain trên).

---

## Tuần 1 — C++17 & Modern C++ (+ CMake)

*Trọng tâm JD "C++17". Vá nền ngôn ngữ trước.*

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] [raii-smart-pointers](../../02-modern-cpp/raii-smart-pointers.md) §6 (Rule 0/3/5), [move-semantics](../../02-modern-cpp/move-semantics.md) | `/mock rapid track cpp-system` |
| 2 | - [ ] 🎯 EMC items nền: [effective-modern-cpp](../../16-book-summaries/effective-modern-cpp.md) (auto, nullptr, override, make_unique, special members) | `/mock by-level 🟡 track modern-cpp` |
| 3 | - [ ] [concurrency](../../02-modern-cpp/concurrency.md) (atomic, memory order, mutex) + [OS sync](../../03-operating-system/sync-primitives.md) | `/mock daily track cpp-system` |
| 4 | - [ ] [cmake](../../06-build-systems/cmake.md) (target-based, PUBLIC/PRIVATE, toolchain file) | `/mock ... track build-systems` (BLD-001…003) |
| 5 | Coding: viết vào [coding-arena/](../mock-interview/coding-arena/) | `/mock coding track dsa` (reverse list, ring buffer, two-sum) |
| CN | Ôn lại | `/mock weak-review` |

## Tuần 2 — Linux system programming (userspace + debug) 🎯

*Đúng chỗ bạn muốn tập trung ("Familiar with Linux userspace + debugging").*

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] [file-io](../../04-linux-system-programming/file-io.md), [processes-signals](../../04-linux-system-programming/processes-signals.md) | `/mock rapid track linux-sysprog` |
| 2 | - [ ] 🎯 [io-multiplexing](../../04-linux-system-programming/io-multiplexing.md) (epoll, LT/ET, event loop), [ipc-linux](../../04-linux-system-programming/ipc-linux.md) | `/mock by-level 🟠 track linux-sysprog` |
| 3 | - [ ] 🎯 [09-debugging](../../09-debugging/): gdb, strace, perf, ASan/TSan, core dump | `/mock daily track debugging` |
| 4 | - [ ] [gdb](../../09-debugging/gdb.md) sâu (breakpoint/watch/bt/remote gdbserver) — điểm yếu cần vá | `/mock deep-dive track debugging` |
| 5 | Coding: bài low-level C | `/mock coding track cpp-system` (memcpy/strlen, endianness) |
| CN | Ôn lại | `/mock weak-review` |

## Tuần 3 — Kernel & drivers (I2C/SPI + PCI/USB) 🎯

*Trụ kernel + drivers của JD. Vùng bạn mạnh (kernel/DT) + lấp PCI/USB.*

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] [driver-basics](../../05-drivers-device-tree/driver-basics.md), [kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md) · *bổ đệm* [bare-metal-c](../../08-embedded-systems/bare-metal-c.md) phần `volatile`/register/bit-manip ([EMB-A](../mock-interview/bank/embedded-fundamentals.md)) — nền cho MMIO `readl/writel` | `/mock daily track drivers-dt` |
| 2 | - [ ] 🎯 [device-tree](../../05-drivers-device-tree/device-tree.md) + [BSP-006…009](../mock-interview/bank/bsp.md) (probe, EPROBE_DEFER, pinctrl) | `/mock by-level 🟠 track bsp` |
| 3 | - [ ] 🎯 **[pci-usb-drivers](../../05-drivers-device-tree/pci-usb-drivers.md)** (PCI config/BAR/MSI, USB descriptor/URB/gadget) | `/mock ... track drivers-dt` (DRV-019…027) |
| 4 | - [ ] [interrupt/DMA](../mock-interview/bank/bsp.md) (BSP-010…013: threaded IRQ, coherent vs streaming DMA) · *bổ đệm* [interrupts-bare-metal](../../08-embedded-systems/interrupts-bare-metal.md) khái niệm ISR ([EMB-C](../mock-interview/bank/embedded-fundamentals.md)): không sleep trong ISR, critical section, reentrancy — chuyển thẳng sang top-half/bottom-half kernel | `/mock deep-dive track bsp` |
| 5 | - [ ] [lkd](../../16-book-summaries/lkd/) cụm driver/interrupt (đọc thêm) | `/mock comprehensive track drivers-dt` |
| CN | Ôn lại | `/mock weak-review` |

> **Phạm vi `embedded-fundamentals` (EMB) cho JD này:** chỉ ôn **A** (C/thanh ghi/`volatile`/bit-manip) + **C** (khái niệm ISR) như *đệm* cho buổi 1 & 4 ở trên — vì chúng chuyển thẳng sang MMIO driver + kernel IRQ. **Hoãn** B (startup/linker chi tiết), **D (RTOS)**, F/G/H (CRC/MISRA, low-power, JTAG/SWD): JD là Embedded **Linux** (có MMU, không RTOS/bare-metal firmware) nên các phần này là "nice-to-have" ghi điểm nếu bị hỏi lan, không phải câu loại. Full EMB để dành cho hồ sơ BSP tổng quát ([bsp-plan](bsp-plan.md)).

## Tuần 4 — Yocto/CI + Design + Behavioral + tổng duyệt

*Yocto (JD) + kỹ năng "lead/design/review" + dress rehearsal.*

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] 🎯 **[yocto](../../06-build-systems/yocto.md)** (recipe, layer/bbappend, sstate, devtool, SDK) + [BSP-017…019](../mock-interview/bank/bsp.md) | `/mock ... track build-systems` (BLD-004…010) |
| 2 | - [ ] 🎯 [system-design](../../10-thinking/system-design.md) §5 hộp đồ nghề + §6 ví dụ giải mẫu (**OTA**, **máy quét mã vạch** — đúng chất Datalogic) + [07-shared-libraries](../../07-shared-libraries/) (ABI, API design) | `/mock deep-dive track system-design` (SD-012…016) |
| 3 | - [ ] 🎯 **Behavioral cho JD**: [BEH](../mock-interview/bank/behavioral.md) + phần "lead junior / code review" (§ dưới) | `/mock ... track behavioral` |
| 4 | Tổng duyệt vòng thật | `/mock comprehensive track datalogic` (16 câu) |
| 5 | Kiểm tra toàn diện | `/mock full-review` (quét mọi câu đã hỏi) |
| CN | Nghỉ + đọc lại [sessions/](../mock-interview/sessions/) | — |

---

## Behavioral riêng cho JD (JD nhấn "lead junior + code review")

Chuẩn bị STAR ([BEH](../mock-interview/bank/behavioral.md)) cho các câu **rất khả năng gặp** với JD này:
- **"Kể lần bạn mentor/hướng dẫn một junior"** → dùng chất liệu Samsung (hướng dẫn quy trình, AI skill, chip porting tool).
- **"Cách bạn review code / xử lý bất đồng kỹ thuật khi review"** → nêu tiêu chí review (đúng đắn, an toàn, đọc được), góp ý xây dựng, dựa dữ liệu.
- **"Thiết kế một subsystem phức tạp bạn từng làm"** → chọn case cross-layer (S-Box/brightness) hoặc multi-chipset HAL, trình bày theo khung [SD](../mock-interview/bank/system-design.md).
- **"Vì sao rời công ty cũ"** ([BEH-008](../mock-interview/bank/behavioral.md)) + **câu hỏi ngược** ([BEH-009](../mock-interview/bank/behavioral.md)) — hỏi về SoC/nền tảng, tỉ lệ kernel vs userspace, quy trình Yocto/CI, đội ngũ.
- Chuẩn bị nói **tiếng Anh** cho 2–3 câu ruột (JD yêu cầu English).

## Lịch nước rút 2 tuần

Gộp đôi mỗi ngày theo thứ tự ưu tiên: **Tuần A** = (Tuần 1 buổi 1–2–4) + (Tuần 2 buổi 2–3–4); **Tuần B** = (Tuần 3 buổi 2–3–4) + (Tuần 4 buổi 1–3–4). Bỏ các buổi coding/đọc-thêm nếu thiếu giờ, giữ **mọi buổi 🎯 + `comprehensive` + `full-review`**.

## Vá điểm yếu đã biết
Từ [weak-register](../mock-interview/weak-register.md): OS-003 (deadlock), CPP-029 (emplace), DP-002 (Singleton) — chèn vào các buổi `weak-review`. GDB (bạn tự nhận yếu) → ưu tiên Tuần 2 buổi 4.
