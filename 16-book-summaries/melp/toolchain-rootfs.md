# MELP — Toolchain & Root Filesystem (ch. 1–2, 5) 🎯

> Thuộc [MELP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.

---

## Cụm 1 — Toolchain & Cross-compilation (ch. 1–2) 🎯

### Nội dung chính

**Bốn thành phần của mọi hệ Embedded Linux** (khung của cả cuốn sách): toolchain → bootloader → kernel → root filesystem. Toolchain đứng đầu vì mọi thứ khác được build bằng nó.

**Toolchain gồm:** compiler (gcc/clang) + binutils (as, ld, objcopy, objdump...) + **C library** + headers kernel + debugger. **Tuple** định danh mục tiêu: `<arch>-<vendor>-<kernel>-<os/abi>`:

```
aarch64-linux-gnu          → ARM64, glibc
arm-linux-gnueabihf        → ARM32, glibc, hard-float ABI (hf!)
arm-none-eabi              → ARM bare-metal (không Linux — cho MCU/bootloader giai đoạn sớm)
```

- **Cross vs native**: build trên host x86 cho target ARM — vì target quá yếu/không tự build được; hệ quả là mọi rắc rối "build tìm thư viện của host thay vì target".
- **sysroot** — khái niệm trung tâm: cây thư mục chứa **headers + thư viện CỦA TARGET** (`/usr/include`, `/lib`, `/usr/lib`); compiler cross tìm mọi thứ trong sysroot (`--sysroot=`, `-print-sysroot`). Lỗi kinh điển: link nhầm `.so` của host (x86) vào binary ARM → "file in wrong format"; hoặc build system tự ý lấy `/usr/include` của host → compile được, chạy sai.
- **Chọn C library** — câu trade-off hay gặp:

| | glibc | musl | uclibc-ng |
|---|---|---|---|
| Size | Lớn (MB) | Nhỏ (trăm KB) | Nhỏ |
| Tương thích | Chuẩn de-facto, mọi phần mềm | Rất tốt, đôi khi vá | Kha khá |
| Đặc điểm | NSS, locale đầy đủ, hiệu năng tốt | Sạch, static linking tốt, license MIT | Cấu hình được từng tính năng |
| Chọn khi | RAM/flash thoải mái (mặc định) | Hệ nhỏ, container, static | Hệ rất nhỏ kiểu cũ |

- Nguồn toolchain: vendor SDK, distro (`gcc-aarch64-linux-gnu`), **crosstool-NG** (tự build, kiểm soát trọn), hoặc **để Yocto/Buildroot tự build** (chuẩn nhất cho sản phẩm — toolchain khớp chính xác libc/kernel headers của image).
- Build lib/app có dependency: `pkg-config` với `PKG_CONFIG_SYSROOT_DIR`, autotools `./configure --host=aarch64-linux-gnu`, CMake **toolchain file** (`CMAKE_TOOLCHAIN_FILE` — set compiler, sysroot, `CMAKE_FIND_ROOT_PATH_MODE_*` để "chỉ tìm trong sysroot") — nối thẳng [06/cmake.md](../../06-build-systems/cmake.md), [06/cross-compilation.md](../../06-build-systems/cross-compilation.md).

### Insight đáng nhớ

- **"Cross-compile khó không phải ở compiler mà ở dependency"**: compiler đổi một biến môi trường là xong; cái vỡ trận là build system *lén* dùng headers/libs/tool của host. Kỹ năng thật = khoanh vùng "cái này đến từ host hay sysroot?" (`file` một binary là biết kiến trúc — công cụ chẩn đoán số 1).
- ABI trong tuple là chuyện thật: `gnueabihf` vs `gnueabi` (hard vs soft float) **không link lẫn được** — board dùng ABI nào là quyết định toàn hệ, đổi giữa chừng = rebuild thế giới.

### Ít quan trọng

- Quy trình build crosstool-NG từng bước, cấu trúc thư mục toolchain nội bộ (ch. 2 giữa) — làm theo tài liệu khi cần; lịch sử OABI/EABI.

### Góc interview

**Câu 1:** Binary bạn cross-compile chạy trên board báo lỗi `not found` dù file tồn tại và có quyền thực thi. Nguyên nhân và cách chẩn đoán?

<details><summary>Đáp án</summary>

- Thông báo `not found` đánh lừa: không phải "không thấy binary" mà là **không thấy trình thông dịch/loader hoặc thư viện** mà binary yêu cầu:
  1. **Dynamic loader sai**: binary đòi `/lib/ld-linux-aarch64.so.1` (ghi trong ELF header) nhưng rootfs dùng musl (`/lib/ld-musl-...`) hoặc thiếu loader — xảy ra khi build bằng toolchain glibc mà rootfs musl (hoặc ngược lại).
  2. **Thiếu shared library**: lib có trên host (trong sysroot lúc link) nhưng chưa cài lên rootfs.
  3. **Sai kiến trúc/ABI**: binary ARM32 hf trên hệ soft-float, hay x86 trên ARM (không có binfmt) — cũng hiện `not found`/`Exec format error`.
- Chẩn đoán theo thứ tự: `file ./binary` (kiến trúc + "interpreter ..." đúng không) → so interpreter với thực tế rootfs (`ls /lib/ld-*`) → `readelf -d ./binary | grep NEEDED` liệt kê lib cần, đối chiếu rootfs (trên board không có ldd thì dùng loader trực tiếp: `/lib/ld-linux-*.so.1 --list ./binary`) → giải pháp: build đúng libc của rootfs, cài đủ lib, hoặc **static link** (musl static là bài gọn cho tool nhỏ).
- Câu này lộ ngay người từng cross-compile thật — trả lời "chmod +x" là trượt.

</details>

**Câu 2:** sysroot là gì? CMake toolchain file cần khai những gì để cross-compile đúng?

<details><summary>Đáp án</summary>

- **sysroot** = "ảnh chụp gốc filesystem của target" dành cho compiler: headers (`usr/include`) + libraries (`lib`, `usr/lib`) đúng kiến trúc/libc target. Compiler cross tiền tố mọi đường dẫn tìm kiếm bằng sysroot → không bao giờ chạm headers/libs của host. Của ai cung cấp: toolchain đóng gói sẵn, hoặc Yocto SDK/Buildroot `staging/` sinh ra (giàu hơn — chứa cả lib bên thứ ba đã build).
- Toolchain file CMake tối thiểu:

```cmake
set(CMAKE_SYSTEM_NAME Linux)                 # kích hoạt chế độ cross
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_SYSROOT      /path/to/sysroot)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # tool build (protoc...) → tìm ở HOST
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # lib → CHỈ tìm trong sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # headers → CHỈ trong sysroot
```

- Ba dòng `FIND_ROOT_PATH_MODE` là linh hồn: chúng mã hóa đúng bài học "chương trình chạy lúc build thuộc host, thứ được link thuộc target". Điểm cộng: biết `CMAKE_CROSSCOMPILING_EMULATOR` (qemu-user) cho phép chạy test target ngay trên host.

</details>

---

## Cụm 2 — Root Filesystem từ con số 0 (ch. 5) 🎯

### Nội dung chính

**Rootfs tối thiểu để boot đến shell** — hiểu từng mảnh để không "cargo cult":

```
/
├── bin, sbin, usr/...   ← chương trình: thường là BUSYBOX (một binary,
│                           N symlink: ls→busybox, sh→busybox... ~1-2MB đủ cả hệ)
├── lib                  ← libc (.so) + DYNAMIC LOADER ld-*.so — thiếu loader
│                           là "init not found" dù init nằm đó!
├── dev                  ← device nodes: kernel tự cấp bằng DEVTMPFS (hiện đại);
│                           quản lý động: mdev (BusyBox) / udev (systemd) xử lý
│                           hotplug + modalias nạp module
├── proc, sys            ← mount procfs/sysfs ngay đầu init — không có /proc
│                           là ps/top/mount câm
├── etc/init.d, inittab  ← init script (BusyBox init đọc /etc/inittab)
└── tmp, run, var        ← tmpfs
```

- **Ai là PID 1**: kernel chạy `init=` (mặc định thử `/sbin/init` → `/bin/sh`...) — chương trình đầu tiên và duy nhất kernel đích thân khởi động; mọi thứ còn lại do init đẻ ra. Init chết = kernel panic.
- **Quyền sở hữu file khi build**: tạo rootfs trên host bằng user thường → file thuộc uid 1000 — sai; giải pháp **fakeroot** (giả syscall chown/chmod trong user space) hoặc để Buildroot/Yocto lo (họ dùng fakeroot/pseudo bên trong).
- **Boot phục vụ phát triển — hai vũ khí năng suất:**
  - **NFS root**: `root=/dev/nfs nfsroot=<ip>:/path ip=dhcp` — rootfs nằm trên host, sửa file **thấy ngay trên board không cần flash lại** (chu kỳ dev nhanh gấp bội);
  - **TFTP**: U-Boot tải kernel/DTB qua mạng (`tftpboot`) — không flash kernel mỗi lần thử.
- **initramfs (initrd)**: rootfs nhỏ nén **trong RAM**, unpack trước rootfs thật — vai trò: chứa driver/logic mount rootfs phức tạp (LUKS, RAID, module storage), hoặc **là cả hệ thống** với thiết bị siêu nhỏ/recovery mode. Hình thức: cpio nén, nhúng vào kernel hoặc file riêng bootloader nạp.
- Kích cỡ tham chiếu đáng nhớ: rootfs BusyBox tĩnh ~vài MB; thêm glibc ~+3–5MB — "Linux tối thiểu" nhỏ hơn nhiều người nghĩ.

### Insight đáng nhớ

- Trò chơi tư duy đáng làm một lần: *"kernel panic: no init found"* → duyệt checklist: `root=` đúng partition? FS driver có trong kernel? `/sbin/init` tồn tại? **là binary đúng kiến trúc? loader + libc của nó có trong /lib?** — 5 câu này phủ 95% lỗi rootfs đầu đời, và interviewer thích hỏi chính kịch bản này.
- NFS root/TFTP không phải "mẹo cũ" — nó là **chu kỳ dev chuẩn của nghề BSP** đến tận nay; kể ra được workflow này trong interview = tín hiệu người làm thật.

### Ít quan trọng

- Bảng device node major/minor tạo tay bằng `mknod` (thời tiền devtmpfs — hiểu khái niệm là đủ); chi tiết cấu hình từng applet BusyBox (ch. 5 giữa).

### Góc interview

**Câu 1:** Kernel boot xong báo `Kernel panic - not syncing: No working init found`. Liệt kê nguyên nhân theo thứ tự bạn kiểm tra.

<details><summary>Đáp án</summary>

Trước đó kernel đã **mount root thành công** (nếu không sẽ là panic "VFS: Unable to mount root fs" — phân biệt hai panic này là điểm cộng đầu tiên), vậy vấn đề nằm trong rootfs:
1. `/sbin/init` (hoặc đường trong `init=`) **không tồn tại** — ảnh rootfs thiếu, symlink busybox gãy (link tuyệt đối trỏ đường host).
2. Tồn tại nhưng **không chạy được**: sai kiến trúc/ABI (`file` kiểm tra), **thiếu dynamic loader hoặc libc trong /lib** (binary dynamic mà rootfs chỉ chép binary quên lib — nguyên nhân số 1 với người mới; init tĩnh không dính), thiếu quyền +x.
3. Init chạy nhưng **chết ngay**: script `#!/bin/sh` mà `/bin/sh` không có; inittab/unit lỗi khiến PID 1 exit.
4. Rootfs mount **read-only mà init cần ghi sớm** (hiếm — thường chỉ gây lỗi muộn hơn).
- Cách khoanh nhanh: `init=/bin/sh` vào bootargs — shell lên được nghĩa là hệ sống, vấn đề ở init/script (đi tiếp từ shell: mount /proc, chạy tay init xem lỗi); shell cũng không lên → nhóm 1–2 (kiến trúc/loader/thiếu file).

</details>

**Câu 2:** initramfs là gì, khác rootfs thật thế nào, khi nào bắt buộc phải có?

<details><summary>Đáp án</summary>

- **initramfs** = archive cpio (nén) được unpack vào **tmpfs trong RAM** rất sớm; kernel chạy `/init` của nó **trước khi** biết đến rootfs thật. Khác rootfs thật: sống trong RAM (mất khi tắt), gắn với kernel (nhúng trong Image hoặc file đi kèm bootloader nạp), nhỏ và chuyên dụng.
- Vai trò chuẩn: **giai đoạn chuẩn bị mount rootfs thật** khi việc đó cần logic/driver không tiện built-in — storage driver dạng module, mở LUKS (hỏi passphrase/key), ráp RAID/LVM, tìm root theo UUID, rồi `switch_root` sang rootfs thật.
- **Bắt buộc** khi: driver của thiết bị chứa rootfs là module (gà-trứng — xem [bootloader-kernel.md](bootloader-kernel.md)); rootfs mã hóa/cần unlock; root trên thiết bị cần userspace helper (network phức tạp, fw loading). **Không cần** khi: driver storage + FS đều built-in và root= chỉ thẳng được — đa số thiết bị nhúng cấu hình tĩnh bỏ qua initramfs cho boot nhanh, đơn giản.
- Vai phụ đáng nêu: initramfs **làm luôn recovery/factory mode** (hệ tối giản độc lập rootfs chính — rootfs hỏng vẫn còn đường vào), và là toàn bộ hệ thống cho thiết bị chỉ-RAM.

</details>

### Đọc thêm (tùy chọn)

- [06/cross-compilation.md](../../06-build-systems/cross-compilation.md), [06/cmake.md](../../06-build-systems/cmake.md) — nền cross-compile của repo.
- [07/linking-loading.md](../../07-shared-libraries/linking-loading.md) — dynamic loader, NEEDED, rpath: giải thích sâu lỗi "not found".
