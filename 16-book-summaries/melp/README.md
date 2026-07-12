# Mastering Embedded Linux Programming — Chris Simmonds (1st edition, 2015)

> **Nguồn summary:** ⚠️ **PDF của người học là 1st edition (2015, 14 chương)** — không phải 3rd edition (2021) như giả định ban đầu. Trạng thái đối chiếu từng file ghi trong bảng dưới; số trang `(tr. X)` trong các file **đã đối chiếu** là theo đúng bản PDF này (`/home/gia/Desktop/Books/Mastering Embedded Linux Programming.pdf`).
> **Vì sao đọc cuốn này:** "giáo trình BSP" sát nhất — đi trọn chuỗi toolchain → bootloader → kernel → rootfs → build system → storage → driver → init → debug/RT, đúng phạm vi công việc và phỏng vấn của vị trí **Embedded Software Engineer (BSP)**.
> **Quy ước trong các file:** nội dung không đánh dấu = có trong sách; **🆕 = bổ sung ngoài sách** (kiến thức hiện đại 2015 chưa có — TF-A/secure boot, FIT chi tiết, EPROBE_DEFER, earlycon, OTA framework, Yocto hiện đại...); **⚠️ = sách lỗi thời ở điểm đó.**

---

## TL;DR — cuốn sách này dạy gì

Xây một hệ Embedded Linux từ con số 0 quanh **bốn thành phần** (tr. 5): toolchain → bootloader → kernel → root filesystem; rồi vận hành: build tự động (Buildroot/Yocto), chiến lược lưu trữ flash, giao tiếp driver, init, và debug/profile/realtime. Triết lý xuyên suốt: mọi lựa chọn (libc, FS, init, cách update) là **trade-off biện luận được** — đúng chất câu hỏi phỏng vấn BSP.

## 🕐 Đọc gì nếu chỉ có N giờ

- **~3 giờ:** [bootloader-kernel.md](bootloader-kernel.md) — boot flow + device tree + build/boot kernel là lõi của mọi buổi phỏng vấn BSP.
- **~6 giờ:** thêm [storage-update.md](storage-update.md) (flash, UBI, update) và cụm Yocto trong [build-systems.md](build-systems.md).
- **~9 giờ:** thêm [toolchain-rootfs.md](toolchain-rootfs.md) và cụm Real-time trong [debug-realtime.md](debug-realtime.md).

## 🎯 Lộ trình ôn nhanh trước interview (BSP)

1. **Boot flow ARM + U-Boot + Device Tree** ([bootloader-kernel.md](bootloader-kernel.md)) — câu mở màn "kể từ lúc cấp nguồn".
2. **Kernel build/Kconfig + porting board** ([bootloader-kernel.md](bootloader-kernel.md)).
3. **Storage flash + update** ([storage-update.md](storage-update.md)) — MTD/UBI vs eMMC, A/B update.
4. **Yocto/Buildroot** ([build-systems.md](build-systems.md)).
5. **Real-time** ([debug-realtime.md](debug-realtime.md)) — PREEMPT_RT, cyclictest, nguồn latency.
6. **Toolchain & rootfs tối giản** ([toolchain-rootfs.md](toolchain-rootfs.md)) — sysroot, hai loại kernel panic, NFS boot.
7. **Driver interface + init + power** ([drivers-init-power.md](drivers-init-power.md)).

## 🗺️ Bản đồ: chương (1st ed) ↔ file ↔ trạng thái đối chiếu

| File | Chương 1st ed (trang) | 🎯 BSP | Trạng thái đối chiếu |
|------|----------------------|--------|----------------------|
| [toolchain-rootfs.md](toolchain-rootfs.md) | 1 (tr. 1), 2 Toolchains (tr. 13), 5 Root Filesystem (tr. 95) | 🎯 | Kiến thức + mục lục |
| [bootloader-kernel.md](bootloader-kernel.md) | **3 Bootloaders + Device Tree (tr. 41), 4 Kernel (tr. 69)** | 🎯🎯 | ✅ **ĐÃ ĐỐI CHIẾU SÂU toàn văn** |
| [build-systems.md](build-systems.md) | 6 Build System: Buildroot + Yocto (tr. 129) | 🎯 | Kiến thức + mục lục (Yocto trong file theo bản hiện đại — sách 2015 đã cũ nhiều ở chương này) |
| [storage-update.md](storage-update.md) | 7 Storage Strategy (tr. 159; "Updating in the field" chỉ có tr. 192–195) | 🎯🎯 | Kiến thức + mục lục; phần OTA/A-B framework chủ yếu là 🆕 ngoài sách (1st ed nói rất ngắn) |
| [drivers-init-power.md](drivers-init-power.md) | 8 Device Drivers (tr. 197), 9 Init (tr. 229); **power: 1st ed KHÔNG có chương này** → phần power là 🆕 | 🎯 | Kiến thức + mục lục |
| [debug-realtime.md](debug-realtime.md) | 10 Processes/Threads (tr. 247), 11 Memory (tr. 273), 12 GDB (tr. 295), 13 Profiling/Tracing, 14 Real-Time | RT 🎯 | Kiến thức + mục lục |

> Ghi chú edition: 3rd edition (2021) thêm hẳn các chương Yocto sâu, "Updating Software in the Field" (OTA đầy đủ), "Managing Power" — nội dung tương ứng trong các file trên vẫn giữ (giá trị phỏng vấn cao) nhưng được hiểu là 🆕 so với PDF 1st ed đang có. Nếu sau này có PDF 3rd ed, chỉ cần đối chiếu bổ sung các file đó.

Liên kết repo: [05-drivers-device-tree/](../../05-drivers-device-tree/), [06-build-systems/](../../06-build-systems/), [08-embedded-systems/](../../08-embedded-systems/), [09-debugging/](../../09-debugging/).
