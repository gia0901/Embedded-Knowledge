# Câu hỏi phỏng vấn — Drivers, Device Tree & Embedded

> Topic gốc: [05 Drivers & Device Tree](../05-drivers-device-tree/), [08 Embedded Systems](../08-embedded-systems/). Tự trả lời trước khi mở đáp án.

---

## 🟢 Cơ bản

<details><summary>1) Phân biệt character, block, network driver.</summary>

Character: luồng byte tuần tự qua `/dev/...` (tty, sensor, GPIO). Block: truy cập theo khối, ngẫu nhiên, qua block layer + page cache (disk, eMMC). Network: xử lý gói tin, không dưới `/dev`, gắn net stack, truy cập qua socket. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

<details><summary>2) Kernel module là gì? Ưu điểm?</summary>

Đoạn code kernel nạp/gỡ động (`.ko`) bằng insmod/modprobe/rmmod, không cần build lại/khởi động lại kernel. Ưu: phát triển/cập nhật driver nhanh, tiết kiệm bộ nhớ (nạp khi cần), giữ kernel gọn. Chạy trong kernel space nên dùng API kernel (`kmalloc`, `printk`), không có libc. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

<details><summary>3) Major và minor number để làm gì?</summary>

Major xác định driver nào phụ trách thiết bị (kernel định tuyến thao tác trên device node tới đúng driver). Minor do driver dùng để phân biệt các instance/thiết bị cụ thể nó quản lý. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

<details><summary>4) MCU và MPU khác nhau gì?</summary>

MCU: RAM/flash on-chip (KB–MB), thường không MMU, chạy bare-metal/RTOS, điện thấp — điều khiển realtime. MPU: RAM/storage ngoài (MB–GB), có MMU nên chạy Linux, mạnh hơn, điện cao hơn — giao diện/mạng/xử lý nặng. Có/không MMU quyết định chạy được Linux hay không. → [architecture](../08-embedded-systems/architecture.md)
</details>

---

## 🟡 Trung bình

<details><summary>5) Khi user gọi read() trên /dev/mydev, điều gì xảy ra trong driver?</summary>

`read` là syscall; kernel tra device node để biết major/minor → xác định driver → gọi hàm `.read` trong `file_operations` của driver. Hàm nhận `char __user *buf`, lấy dữ liệu từ phần cứng/đệm, dùng `copy_to_user` chép sang buffer user an toàn, trả số byte đọc. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

<details><summary>6) Vì sao driver không được dereference con trỏ user trực tiếp?</summary>

Con trỏ user thuộc address space user, có thể không hợp lệ, trỏ vào kernel (tấn công), hoặc bị swap (fault). Dereference trực tiếp gây oops hoặc lỗ hổng bảo mật. Phải dùng `copy_from_user`/`copy_to_user` (xác thực vùng, xử lý fault, trả `-EFAULT` nếu sai). → [kernel-userspace](../05-drivers-device-tree/kernel-userspace.md)
</details>

<details><summary>7) Device tree là gì và giải quyết vấn đề gì?</summary>

Cấu trúc dữ liệu mô tả phần cứng (CPU, bus, thiết bị, địa chỉ thanh ghi, IRQ) tách rời mã kernel, bootloader nạp cho kernel. Giải quyết vấn đề ARM/embedded: phần cứng không tự khai báo như PCI, trước phải hard-code "board file" → bùng nổ code. Với DT, một kernel chạy nhiều board chỉ bằng DTB khác nhau. → [device-tree](../05-drivers-device-tree/device-tree.md)
</details>

<details><summary>8) Memory-mapped I/O là gì? Vì sao cần volatile?</summary>

Thanh ghi peripheral được ánh xạ vào không gian địa chỉ bộ nhớ; đọc/ghi địa chỉ = điều khiển phần cứng. Cần `volatile` vì giá trị thanh ghi đổi do phần cứng (không cache được) và ghi có side effect (không loại bỏ/gộp được). Cần thêm barrier cho thứ tự. Trên Linux kernel dùng `ioremap` + `readl/writel`. → [architecture](../08-embedded-systems/architecture.md)
</details>

<details><summary>9) ioctl và sysfs khác nhau? Khi nào dùng cái nào?</summary>

ioctl: gửi lệnh điều khiển tùy biến + struct tham số qua mã lệnh — linh hoạt cho thao tác không hợp read/write, nhưng dễ thành API thiếu chuẩn. sysfs: mỗi thuộc tính là file văn bản dưới `/sys`, một giá trị/file, dễ dùng từ shell, tự tài liệu hóa. Dùng sysfs cho thuộc tính cấu hình/trạng thái đơn giản; ioctl cho lệnh phức tạp/truyền struct. → [kernel-userspace](../05-drivers-device-tree/kernel-userspace.md)
</details>

---

## 🟠 Khó

<details><summary>10) Probe() làm gì? Device và driver match thế nào?</summary>

Linux Device Model tách device (đến từ device tree/ACPI/bus enumeration) khỏi driver (code điều khiển). Driver khai báo match table (`of_match_table` khớp `compatible`, hoặc ID table). Khi device khớp driver, kernel gọi `probe()` — khởi tạo: ánh xạ thanh ghi (`ioremap`), xin IRQ, cấp tài nguyên, đăng ký subsystem. Tháo thì gọi `remove()`. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

<details><summary>11) Top half / bottom half trong xử lý interrupt là gì?</summary>

Interrupt handler (top half) chạy với ngắt bị tắt nên phải cực nhanh, không được ngủ. Việc nặng/có thể ngủ hoãn sang bottom half: tasklet/softirq (ngữ cảnh atomic) hoặc workqueue/threaded IRQ (ngữ cảnh process, được phép ngủ). Giữ hệ thống đáp ứng và không bỏ lỡ ngắt khác. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

<details><summary>12) DMA là gì? Vấn đề cache coherency?</summary>

DMA controller chuyển dữ liệu giữa peripheral và RAM không cần CPU copy từng byte (CPU chỉ cấu hình rồi nhận interrupt khi xong) → giải phóng CPU, throughput cao, tiết kiệm điện. Vấn đề: DMA ghi thẳng RAM không qua cache CPU → phải flush/invalidate cache đúng lúc để CPU và DMA thấy dữ liệu nhất quán; buffer cần căn lề và vùng nhớ phù hợp. → [architecture](../08-embedded-systems/architecture.md)
</details>

<details><summary>13) Mô tả quá trình boot của embedded Linux.</summary>

Power-on → CPU chạy **Boot ROM** (cứng trong SoC), nạp **SPL** vào SRAM → SPL khởi tạo DRAM, nạp **U-Boot** vào DRAM → U-Boot nạp kernel+DTB+initramfs, nhảy vào **kernel** → kernel khởi tạo, parse DTB, nạp driver, mount **rootfs**, chạy **init (PID 1)** → **userspace**. Mỗi giai đoạn khởi tạo đủ phần cứng để nạp giai đoạn sau (bootstrapping). → [boot-process](../08-embedded-systems/boot-process.md)
</details>

<details><summary>14) Vì sao quản lý tài nguyên trong driver quan trọng? devm_* giúp gì?</summary>

Rò rỉ trong kernel không cô lập như crash process — tích lũy làm cạn hệ thống, dùng tài nguyên đã free gây oops/panic. Mọi thứ xin phải trả đúng thứ tự ngược kể cả nhánh lỗi — dễ sót. `devm_*` (devm_kmalloc, devm_request_irq, devm_ioremap...) gắn tài nguyên vào vòng đời device và tự giải phóng khi remove/probe lỗi — như RAII cho driver. → [driver-basics](../05-drivers-device-tree/driver-basics.md)
</details>

---

## 🔴 Senior

<details><summary>15) "Realtime" nghĩa là gì? Khi nào chọn RTOS, khi nào Linux?</summary>

Realtime = **tất định** về thời gian (đảm bảo deadline trong mọi trường hợp, quan tâm worst-case latency & jitter), không phải nhanh. Chọn RTOS khi cần tất định cao (hard realtime), worst-case nhỏ và bounded, MCU tài nguyên ít, tính năng tối thiểu. Chọn Linux khi cần hệ sinh thái phong phú (mạng, FS, đa process, driver) và realtime ở mức soft/firm (hoặc PREEMPT_RT). Vừa cần cả hai → kiến trúc heterogeneous (AMP). → [rtos-vs-linux](../08-embedded-systems/rtos-vs-linux.md)
</details>

<details><summary>16) Vì sao Linux mặc định không phù hợp hard realtime? PREEMPT_RT giúp gì?</summary>

Linux tối ưu throughput, không cho worst-case latency: vùng không preempt được trong kernel, xử lý interrupt/softirq, page fault, cân bằng tải scheduler, lock contention gây jitter lớn ở worst case. PREEMPT_RT (phần lớn đã mainline) biến gần như toàn kernel thành preemptible, chuyển interrupt handler thành thread, dùng mutex priority inheritance → worst-case latency xuống hàng chục–trăm µs, đủ nhiều soft/firm realtime. → [rtos-vs-linux](../08-embedded-systems/rtos-vs-linux.md)
</details>

<details><summary>17) Vì sao nên hạn chế heap trong embedded? Thay bằng gì?</summary>

Heap lâu dài gây fragmentation (malloc thất bại dù còn trống) và thời gian cấp phát không tất định — nguy hiểm cho hệ chạy liên tục/realtime, RAM ít. Thay bằng: cấp phát tĩnh (biết footprint lúc compile), stack (cẩn thận overflow), memory pool/fixed-block allocator (tất định, không phân mảnh). MISRA và nhiều chuẩn embedded hạn chế cấp phát động sau init. → [constraints](../08-embedded-systems/constraints.md)
</details>

<details><summary>18) Watchdog là gì? Dùng đúng cách thế nào?</summary>

Timer phần cứng đếm ngược; phần mềm phải "kick" định kỳ. Hệ treo (không kick) → watchdog reset thiết bị → tự phục hồi (quan trọng khi không người giám sát). Dùng đúng: kick ở vị trí phản ánh hệ thực sự hoạt động bình thường (cuối chu kỳ chính, hoặc khi mọi task quan trọng báo còn sống), không kick mù trong interrupt độc lập. Windowed watchdog bắt cả lỗi chạy quá nhanh. → [constraints](../08-embedded-systems/constraints.md)
</details>

<details><summary>19) Vì sao embedded dùng A/B partition và rootfs read-only?</summary>

A/B: ghi bản mới vào slot không dùng, boot thử; nếu hỏng thì bootloader rollback về slot cũ → cập nhật firmware an toàn, tránh bricking thiết bị ngoài field. rootfs read-only: bảo vệ hệ thống khỏi hư hỏng do ghi (mất điện đột ngột) và mòn flash; dữ liệu thay đổi để ở phân vùng data riêng. Kết hợp cho cập nhật được mà vẫn tin cậy/chịu lỗi. → [boot-process](../08-embedded-systems/boot-process.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
