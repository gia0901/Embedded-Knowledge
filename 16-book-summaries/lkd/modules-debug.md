# LKD — Devices & Modules, Debugging, Portability (ch. 17 tr. 337, ch. 18 tr. 363, ch. 19 tr. 379, ch. 20 tr. 395) 🎯(ch.17)

> Thuộc [LKD](README.md). Nguồn: kiến thức Claude, số trang đối chiếu PDF 3rd ed.

---

## Cụm 1 — Modules & Device Model: kobject, sysfs, bus-device-driver (ch. 17, tr. 337–361) 🎯

### Nội dung chính

**Module:** đơn vị code nạp/gỡ runtime — `module_init/module_exit`, `MODULE_LICENSE` (GPL — thiếu là "kernel tainted" + không gọi được symbol GPL-only), `EXPORT_SYMBOL[_GPL]` (module chỉ gọi được symbol đã export), tham số `module_param`, build out-of-tree bằng kbuild Makefile. (Nạp theo sự kiện: uevent → udev → modprobe theo **modalias** — đã ở [lkd/interrupts](interrupts-bottomhalves.md)/[MELP](../melp/bootloader-kernel.md).)

**Device model — trả lời câu "cái gì đứng sau /sys":**
- **kobject**: "object base class" của kernel — refcount + tên + vị trí trong cây; nhúng vào struct lớn hơn (`container_of` lấy ngược — idiom C phải biết); mỗi kobject = **một thư mục trong sysfs**; ktype/kset gom nhóm + định nghĩa attribute.
- **sysfs**: cây kobject phơi ra userspace; **attribute = file, một giá trị/file** (quy ước cứng — [MELP](../melp/drivers-init-power.md)); show/store callback.
- **Tam giác bus – device – driver** (đỉnh của chương): mỗi **bus** (platform, i2c, spi, pci, usb...) giữ hai danh sách device & driver; đăng ký bên nào cũng chạy **match** (DT: compatible; USB/PCI: ID table) → match thành công → **probe**. Chính là cơ chế đã dùng suốt ở [MELP bootloader-kernel](../melp/bootloader-kernel.md) — LKD cho phần móng: vì sao mọi subsystem trông giống nhau (đều là kobject + bus pattern).
- Uevent: kobject thêm/bớt → thông điệp lên userspace (udev) → tạo /dev node, nạp module — chuỗi hotplug trọn vẹn.

### Insight đáng nhớ

- **"/sys không phải feature — nó là cấu trúc dữ liệu nội bộ của kernel tự phơi ra"**: mỗi thư mục một kobject thật đang sống trong RAM. Hiểu vậy thì `ls /sys/bus/i2c/devices` không phải tra cứu mà là *đọc trực tiếp* danh sách device của i2c bus — kỹ năng debug bind/probe hằng ngày (`/sys/bus/*/drivers/*/bind|unbind` gỡ/gắn driver không cần reboot).
- `container_of` + ops-struct + kobject = ba viên gạch OOP-trong-C của toàn kernel; đọc được một subsystem là đọc được mọi subsystem.

### Góc interview

**Câu 1 (🎯):** Trình bày cơ chế bus–device–driver. Driver `.probe` được gọi khi nào — kể đủ các con đường.

<details><summary>Đáp án</summary>

- Mỗi bus giữ **hai danh sách** (devices, drivers) + hàm **match**. probe chạy khi một cặp device–driver match **bất kể bên nào đến sau**:
  1. **Driver đăng ký** (`platform_driver_register`, boot hoặc insmod) → duyệt danh sách device sẵn có của bus → match (DT compatible / ID table) → probe từng device khớp.
  2. **Device xuất hiện** (parse DT lúc boot, hotplug USB, i2c controller probe tạo device con, `new_device` qua sysfs) → duyệt danh sách driver → match → probe.
  3. **Probe lại sau EPROBE_DEFER**: driver khác probe xong → kernel thử lại hàng đợi deferred.
  4. Thủ công: `echo <dev> > /sys/bus/.../drivers/<drv>/bind` (unbind/bind — debug không cần reboot).
- probe nhận device, xin tài nguyên (devm_*), đăng ký vào subsystem chức năng; fail thì trả lỗi — device ở lại danh sách chờ driver khác/lần sau.
- Điểm cộng: nói được "vì hai chiều nên thứ tự nạp module không quan trọng" — chính là điều làm hệ module + DT hoạt động; và udev/modalias là cách driver *được nạp* trước khi có mà match.

</details>

---

## Cụm 2 — Debugging kernel (ch. 18, tr. 363–378)

### Nội dung chính

- **printk**: gọi được từ *hầu hết* mọi context (kể cả ISR — có ring buffer riêng); **log level** (`KERN_ERR`...`KERN_DEBUG`, `/proc/sys/kernel/printk` ngưỡng ra console); rate limit (`printk_ratelimited`) tránh bão log. ⚠️ Hiện đại: dùng `dev_err/dev_dbg` (kèm tên device) + **dynamic debug** (`/sys/kernel/debug/dynamic_debug/control` — bật dòng debug theo file/hàm lúc runtime, không rebuild).
- **Oops (tr. ~367):** kernel gặp lỗi (deref NULL...) nhưng cứu được context → in report (PC, call trace, registers) rồi giết task đang chạy — hệ *có thể* sống tiếp nhưng **đã không còn đáng tin**; trong ISR/quá nặng → **panic** (dừng hẳn). Đọc oops: dòng `PC is at hàm+0xoffset/size` → `addr2line`/`decode_stacktrace.sh` với vmlinux ra file:line — kỹ năng lõi BSP ([09/kernel-debugging.md](../../09-debugging/kernel-debugging.md) có chi tiết).
- **Tainted kernel**: cờ đánh dấu (module ngoài cây, proprietary, đã oops...) — maintainer nhìn taint trước khi đọc bug report.
- Công cụ khác: `BUG_ON/WARN_ON` (assert của kernel — WARN in trace không chết), **Magic SysRq** (`echo t > /proc/sysrq-trigger` — dump task, sync, reboot khi hệ gần chết), git bisect tìm commit hỏng (tr. ~376), kernel debugger (kgdb).
- ⚠️ Bổ sung hiện đại nên biết tên: **KASAN** (ASan cho kernel), **lockdep** (bắt sai lock ordering), **ftrace**, **kdump/crash** (thu core kernel sau panic — chuẩn hậu kiểm sản phẩm).

### Góc interview

**Câu 1:** Board ngoài hiện trường thỉnh thoảng reboot không rõ lý do. Trình tự truy vết phía kernel?

<details><summary>Đáp án</summary>

1. **Phân loại reboot trước**: panic? watchdog cắn? mất nguồn? — xem uptime pattern + **nguyên nhân reset của SoC** (thanh ghi reset reason: POR/WDT/SW — BSP đọc được lúc boot, log lại mỗi lần lên) — bước rẻ nhất, khoanh nửa không gian.
2. **Giữ được log qua reboot**: `pstore/ramoops` (vùng RAM sống sót warm reboot — kernel tự ghi oops/panic vào đó, đọc lại ở `/sys/fs/pstore` sau boot) — cấu hình DT reserved-memory; đây là câu trả lời "làm sao thấy panic khi không có UART cắm".
3. Panic có log → decode (vmlinux + `decode_stacktrace.sh`), xem taint, module nào trong trace; nghi ngờ memory corruption → bật **KASAN** bản staging; nghi deadlock → lockdep, hung task detector (`hung_task_timeout_secs` — bắt task D kẹt).
4. Watchdog cắn không panic → RCU stall/IRQ storm/vòng lặp tắt preemption: bật `softlockup/hardlockup detector`, sysrq từ console nếu còn vào được.
5. Kịch bản nặng: **kdump** (kernel dự phòng thu vmcore) — nặng cho embedded ít RAM nhưng là chuẩn khi bug đắt tiền.
- Điểm chấm: pstore/ramoops + reset reason — hai thứ đặc trưng embedded mà người chỉ làm server không quen.

</details>

---

## Cụm 3 — Portability & Community (ch. 19–20, tr. 379–406) — lướt

### Nội dung chính (rút gọn)

- **Portability (ch. 19)** — checklist viết code kernel chạy mọi arch, với BSP đây là chuyện hằng ngày:
  - **Word size**: `long` = word máy (32/64), pointer vừa long — không nhét pointer vào int; kiểu cỡ cố định `u8/u16/u32/u64` cho phần cứng/on-disk/protocol.
  - **Endianness**: dữ liệu từ thiết bị/mạng phải qua `le32_to_cpu/be32_to_cpu`... — không đọc trần (ARM little vs network big — bug bring-up có thật; `readl` đã lo little-endian MMIO).
  - **Alignment**: truy cập không thẳng hàng trên nhiều arch = fault/chậm — `get_unaligned()`; struct chia sẻ với phần cứng: `__packed` + cẩn trọng.
  - Page size không phải hằng (4K/16K/64K — ARM64 có cả ba!), HZ không phải hằng (dùng jiffies+HZ macro), per-arch: cache line size (`____cacheline_aligned`), DMA coherency khác nhau.
- **Community (ch. 20)**: coding style (tabs 8, K&R braces — `scripts/checkpatch.pl`), patch gửi qua mail dạng `git format-patch`, chuỗi review maintainer → subsystem tree → mainline. Giá trị với bạn: **đọc hiểu văn hóa upstream** để trích dẫn/backport patch tử tế trong công việc BSP; chi tiết quy trình gửi patch đọc khi cần (tr. 395–406).

### Góc interview

**Câu 1:** Struct mô tả gói dữ liệu từ thiết bị SPI (chip big-endian) — các bẫy portability khi parse trên ARM64?

<details><summary>Đáp án</summary>

1. **Endianness**: field đa byte của chip big-endian đọc trên ARM little-endian phải `be16_to_cpu/be32_to_cpu` từng field — không memcpy rồi dùng trần (bug "giá trị × 256").
2. **Alignment & padding**: compiler chèn padding theo ABI — struct on-wire phải `__packed`; nhưng packed → field có thể **misaligned** → deref trực tiếp trên vài arch là fault → `get_unaligned_be32()` cho field nghi ngờ (ARM64 chịu unaligned với normal memory nhưng *không* với device memory, và packed + volatile là vùng xám).
3. **Kích thước kiểu**: dùng `u8/u16/u32` (`__be16/__be32` càng tốt — sparse bắt lỗi quên convert), không `int/long` (đổi size theo arch).
4. Bitfield C cho layout on-wire: **tránh** — thứ tự bit là implementation-defined; dùng mask/shift.
- Chốt: khai struct với `__be32` + `__packed`, parse qua helper `get_unaligned_be32` — ba lớp bảo hộ đủ mọi arch; và viết unit test parse với mẫu byte cố định.

</details>

### Đọc thêm (tùy chọn)

- [05/driver-basics.md](../../05-drivers-device-tree/driver-basics.md), [09/kernel-debugging.md](../../09-debugging/kernel-debugging.md) — bản thực hành của repo.
- [melp/bootloader-kernel.md](../melp/bootloader-kernel.md) — DT match/probe phần nổi; file này là phần móng.
