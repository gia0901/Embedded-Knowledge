# Mastering Embedded Linux Programming — Frank Vasquez & Chris Simmonds (3rd edition, 2021)

> **Nguồn summary:** kiến thức Claude, chưa đối chiếu PDF. ⚠️ Số chương theo 3rd edition (21 chương) — có thể lệch so với bản in cụ thể, sẽ chốt khi đối chiếu PDF.
> **Vì sao đọc cuốn này:** "giáo trình BSP" sát nhất trên thị trường — đi trọn chuỗi toolchain → bootloader → kernel → rootfs → build system → storage/update → driver → init/power → debug/RT, đúng phạm vi công việc và phạm vi phỏng vấn của vị trí **Embedded Software Engineer (BSP)**.
> **Trọng tâm bản summary này:** các phần liên quan BSP được viết sâu và đánh 🎯; các chương ứng dụng userspace (đã có topic 03–04 và OSTEP phủ) viết lướt có chủ đích.

---

## TL;DR — cuốn sách này dạy gì

Xây một hệ Embedded Linux từ con số 0 gồm **bốn thành phần** — toolchain, bootloader, kernel, root filesystem — rồi vận hành vòng đời của nó: build tự động (Buildroot/Yocto), chọn chiến lược lưu trữ trên flash, **cập nhật ngoài hiện trường không được phép chết** (A/B, atomic), giao tiếp phần cứng qua driver và device tree, khởi động (init), quản lý điện, và cuối cùng là debug/profile/realtime. Triết lý xuyên suốt: mọi lựa chọn (thư viện C, FS, init, cơ chế update) là **trade-off có thể biện luận** — đúng chất câu hỏi phỏng vấn BSP.

## 🕐 Đọc gì nếu chỉ có N giờ

- **~3 giờ:** [bootloader-kernel.md](bootloader-kernel.md) — boot flow + kernel + device tree là lõi của mọi buổi phỏng vấn BSP.
- **~6 giờ:** thêm [storage-update.md](storage-update.md) (flash, UBI, OTA update) và cụm Yocto trong [build-systems.md](build-systems.md).
- **~9 giờ:** thêm [toolchain-rootfs.md](toolchain-rootfs.md) và cụm Real-time trong [debug-realtime.md](debug-realtime.md).

## 🎯 Lộ trình ôn nhanh trước interview (BSP)

1. **Boot flow ARM đầy đủ + U-Boot** ([bootloader-kernel.md](bootloader-kernel.md)) — câu mở màn kinh điển "kể từ lúc cấp nguồn đến shell".
2. **Kernel build + Device Tree** ([bootloader-kernel.md](bootloader-kernel.md)) — compatible/probe/deferred probe.
3. **Storage flash + cập nhật OTA** ([storage-update.md](storage-update.md)) — MTD/UBI vs eMMC, A/B update, bootcount/watchdog.
4. **Yocto** ([build-systems.md](build-systems.md)) — layer/recipe/bbappend; hầu hết công ty BSP dùng Yocto.
5. **Real-time: PREEMPT_RT, cyclictest, nguồn latency** ([debug-realtime.md](debug-realtime.md)).
6. **Toolchain & rootfs tối giản** ([toolchain-rootfs.md](toolchain-rootfs.md)) — sysroot, glibc vs musl, NFS boot.
7. **Driver interface + init + power** ([drivers-init-power.md](drivers-init-power.md)).
8. Phần ứng dụng/debug userspace ([debug-realtime.md](debug-realtime.md) nửa đầu) — đã trùng nhiều với topic 04/09, ôn bằng repo là chính.

## 🗺️ Bản đồ: chương ↔ file ↔ topic liên quan

| File | Chương (3rd ed ⚠️) | Nội dung | 🎯 BSP |
|------|--------------------|----------|--------|
| [toolchain-rootfs.md](toolchain-rootfs.md) | 1–2, 5 | Toolchain, cross-compile, sysroot; root filesystem tối giản | 🎯 |
| [bootloader-kernel.md](bootloader-kernel.md) | 3–4 | ROM→SPL→U-Boot→kernel; cấu hình/build kernel; device tree | 🎯🎯 |
| [build-systems.md](build-systems.md) | 6–8 | Buildroot vs Yocto; bitbake, recipe, layer, SDK | 🎯 |
| [storage-update.md](storage-update.md) | 9–10 | NOR/NAND, MTD, UBI/UBIFS, eMMC; update OTA A/B, Mender/RAUC/SWUpdate | 🎯🎯 |
| [drivers-init-power.md](drivers-init-power.md) | 11–15 | Giao tiếp driver từ userspace, viết module; init (BusyBox/SysV/systemd); power management | 🎯 |
| [debug-realtime.md](debug-realtime.md) | 16–21 | Process/thread/memory (lướt); GDB remote; perf/ftrace; **PREEMPT_RT** | RT 🎯 |

Liên kết repo: [05-drivers-device-tree/](../../05-drivers-device-tree/), [06-build-systems/](../../06-build-systems/), [08-embedded-systems/](../../08-embedded-systems/), [09-debugging/](../../09-debugging/).
