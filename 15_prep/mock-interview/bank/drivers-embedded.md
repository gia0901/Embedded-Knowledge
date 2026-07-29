# DRV / BUS — Drivers, Device Tree, Embedded & Bus Protocols

> Domain `DRV` (driver/embedded tổng quát) + `BUS` (I2C/SPI/UART). Gộp từ [11/drivers.md](../../../11-interview-questions/drivers.md) + [technical_round/02 PHẦN 3,5](../../technical_round/02_question_bank.md). Câu BSP chuyên sâu ở [bsp.md](bsp.md). Track dùng: `drivers-dt`, `bsp`, `lkd`.

---

#### DRV-001 · 🟢 · concept · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Phân biệt character, block, network driver.**
<details><summary>Đáp án</summary>

Character: luồng byte tuần tự qua `/dev/...` (tty, sensor, GPIO). Block: truy cập theo khối, ngẫu nhiên, qua block layer + page cache (disk, eMMC). Network: xử lý gói tin, không dưới `/dev`, gắn net stack, truy cập qua socket. Đa số driver sensor/điều khiển là character device.
</details>

#### DRV-002 · 🟢 · concept · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Kernel module là gì? Ưu điểm?**
<details><summary>Đáp án</summary>

Đoạn code kernel nạp/gỡ động (`.ko`) bằng insmod/modprobe/rmmod, không cần build lại/khởi động lại kernel. Ưu: phát triển/cập nhật driver nhanh, tiết kiệm bộ nhớ (nạp khi cần), giữ kernel gọn. Chạy trong kernel space nên dùng API kernel (`kmalloc`, `printk`), không có libc.
</details>

#### DRV-003 · 🟢 · concept · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Major và minor number để làm gì?**
<details><summary>Đáp án</summary>

Major xác định driver nào phụ trách thiết bị (kernel định tuyến thao tác trên device node tới đúng driver). Minor do driver dùng để phân biệt các instance/thiết bị cụ thể nó quản lý.
</details>

#### DRV-004 · 🟢 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**MCU và MPU khác nhau gì?**
<details><summary>Đáp án</summary>

MCU: RAM/flash on-chip (KB–MB), thường không MMU, chạy bare-metal/RTOS, điện thấp — điều khiển realtime. MPU: RAM/storage ngoài (MB–GB), có MMU nên chạy Linux, mạnh hơn, điện cao hơn — giao diện/mạng/xử lý nặng. Có/không MMU quyết định chạy được Linux hay không.
</details>

#### DRV-005 · 🟡 · concept · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Khi user gọi read() trên /dev/mydev, điều gì xảy ra trong driver?**
<details><summary>Đáp án</summary>

`read` là syscall; kernel tra device node để biết major/minor → xác định driver → gọi hàm `.read` trong `file_operations` của driver. Hàm nhận `char __user *buf`, lấy dữ liệu từ phần cứng/đệm, dùng `copy_to_user` chép sang buffer user an toàn, trả số byte đọc.
</details>

#### DRV-006 · 🟡 · concept · ⭐ · [→ kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Vì sao driver không được dereference con trỏ user trực tiếp? (`copy_from_user`/`copy_to_user`)**
<details><summary>Đáp án</summary>

Con trỏ user thuộc address space user, có thể không hợp lệ, trỏ vào kernel (tấn công), hoặc bị swap (fault). Dereference trực tiếp gây oops hoặc lỗ hổng bảo mật. Phải dùng `copy_from_user`/`copy_to_user` (xác thực vùng, xử lý fault an toàn khi chép qua ranh giới user↔kernel, trả `-EFAULT` nếu sai).
</details>

#### DRV-007 · 🟡 · concept · ⭐ · [→ device-tree](../../../05-drivers-device-tree/device-tree.md)
**Device tree là gì và giải quyết vấn đề gì?**
<details><summary>Đáp án</summary>

Cấu trúc dữ liệu mô tả phần cứng (CPU, bus, thiết bị, địa chỉ thanh ghi, IRQ, clock) tách rời mã kernel, bootloader nạp cho kernel (.dts→.dtb). Giải quyết vấn đề ARM/embedded: phần cứng không tự khai báo như PCI, trước phải hard-code "board file" → bùng nổ code. Với DT, một kernel image chạy nhiều board chỉ bằng DTB khác nhau; driver bind qua `compatible` string.
</details>

#### DRV-008 · 🟡 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**Memory-mapped I/O là gì? Vì sao cần volatile?**
<details><summary>Đáp án</summary>

Thanh ghi peripheral được ánh xạ vào không gian địa chỉ bộ nhớ; đọc/ghi địa chỉ = điều khiển phần cứng. Cần `volatile` vì giá trị thanh ghi đổi do phần cứng (không cache được) và ghi có side effect (không loại bỏ/gộp được). Cần thêm barrier cho thứ tự. Trên Linux kernel dùng `ioremap` + `readl/writel`.
</details>

#### DRV-009 · 🟡 · concept · [→ kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**ioctl và sysfs khác nhau? Khi nào dùng cái nào?**
<details><summary>Đáp án</summary>

ioctl: gửi lệnh điều khiển tùy biến + struct tham số qua mã lệnh — linh hoạt cho thao tác không hợp read/write (set chế độ, query trạng thái), nhưng dễ thành API thiếu chuẩn. sysfs: mỗi thuộc tính là file văn bản dưới `/sys`, một giá trị/file, dễ dùng từ shell, tự tài liệu hóa. Dùng sysfs cho thuộc tính cấu hình/trạng thái đơn giản; ioctl cho lệnh phức tạp/truyền struct.
</details>

#### DRV-010 · 🟠 · concept · ⭐ · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Probe() làm gì? Device và driver match thế nào?**
<details><summary>Đáp án</summary>

Linux Device Model tách device (đến từ device tree/ACPI/bus enumeration) khỏi driver (code điều khiển). Driver khai báo match table (`of_match_table` khớp `compatible`, hoặc ID table). Khi device khớp driver, kernel gọi `probe()` — khởi tạo: ánh xạ thanh ghi (`ioremap`), xin IRQ, cấp tài nguyên, đăng ký subsystem. Tháo thì gọi `remove()`. *(Đường đi chi tiết + EPROBE_DEFER: xem [BSP-006](bsp.md).)*
</details>

#### DRV-011 · 🟠 · concept · ⭐ · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Top half / bottom half trong xử lý interrupt là gì?**
<details><summary>Đáp án</summary>

Interrupt handler (top half) chạy với ngắt bị tắt nên phải cực nhanh, không được ngủ. Việc nặng/có thể ngủ hoãn sang bottom half: tasklet/softirq (ngữ cảnh atomic) hoặc workqueue/threaded IRQ (ngữ cảnh process, được phép ngủ). Giữ hệ thống đáp ứng và không bỏ lỡ ngắt khác. *(Vì sao ISR không được ngủ: [BSP-010](bsp.md).)*
</details>

#### DRV-012 · 🟠 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**DMA là gì? Vấn đề cache coherency?**
<details><summary>Đáp án</summary>

DMA controller chuyển dữ liệu giữa peripheral và RAM không cần CPU copy từng byte (CPU chỉ cấu hình rồi nhận interrupt khi xong) → giải phóng CPU, throughput cao, tiết kiệm điện. Vấn đề: DMA ghi thẳng RAM không qua cache CPU → phải flush/invalidate cache đúng lúc để CPU và DMA thấy dữ liệu nhất quán; buffer cần căn lề và vùng nhớ phù hợp. *(Coherent vs streaming DMA: [BSP-011](bsp.md).)*
</details>

#### DRV-013 · 🟠 · concept · ⭐ · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Mô tả quá trình boot của embedded Linux.**
<details><summary>Đáp án</summary>

Power-on → CPU chạy **Boot ROM** (cứng trong SoC), nạp **SPL** vào SRAM → SPL khởi tạo DRAM, nạp **U-Boot** vào DRAM → U-Boot nạp kernel+DTB+initramfs, nhảy vào **kernel** → kernel khởi tạo, parse DTB, nạp driver, mount **rootfs**, chạy **init (PID 1)** → **userspace**. Mỗi giai đoạn khởi tạo đủ phần cứng để nạp giai đoạn sau (bootstrapping). *(Chi tiết + vì sao nhiều giai đoạn: [BSP-002](bsp.md).)*
</details>

#### DRV-014 · 🟠 · concept · ⭐ · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Vì sao quản lý tài nguyên trong driver quan trọng? devm_* giúp gì?**
<details><summary>Đáp án</summary>

Rò rỉ trong kernel không cô lập như crash process — tích lũy làm cạn hệ thống, dùng tài nguyên đã free gây oops/panic. Mọi thứ xin phải trả đúng thứ tự ngược kể cả nhánh lỗi — dễ sót. `devm_*` (devm_kmalloc, devm_request_irq, devm_ioremap...) gắn tài nguyên vào vòng đời device và tự giải phóng khi remove/probe lỗi — như RAII cho driver.
</details>

#### DRV-015 · 🔴 · design · ⭐ · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**"Realtime" nghĩa là gì? Khi nào chọn RTOS, khi nào Linux?**
<details><summary>Đáp án</summary>

Realtime = **tất định** về thời gian (đảm bảo deadline trong mọi trường hợp, quan tâm worst-case latency & jitter), không phải nhanh. Chọn RTOS khi cần tất định cao (hard realtime), worst-case nhỏ và bounded, MCU tài nguyên ít. Chọn Linux khi cần hệ sinh thái phong phú (mạng, FS, đa process, driver) và realtime ở mức soft/firm (hoặc PREEMPT_RT). Vừa cần cả hai → kiến trúc heterogeneous (AMP). *(PREEMPT_RT chi tiết: [BSP-021](bsp.md).)*
</details>

#### DRV-016 · 🔴 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Vì sao nên hạn chế heap trong embedded? Thay bằng gì?**
<details><summary>Đáp án</summary>

Heap lâu dài gây fragmentation (malloc thất bại dù còn trống) và thời gian cấp phát không tất định — nguy hiểm cho hệ chạy liên tục/realtime, RAM ít. Thay bằng: cấp phát tĩnh (biết footprint lúc compile), stack (cẩn thận overflow), memory pool/fixed-block allocator (tất định, không phân mảnh). MISRA và nhiều chuẩn embedded hạn chế cấp phát động sau init.
</details>

#### DRV-017 · 🔴 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Watchdog là gì? Dùng đúng cách thế nào?**
<details><summary>Đáp án</summary>

Timer phần cứng đếm ngược; phần mềm phải "kick" định kỳ. Hệ treo (không kick) → watchdog reset thiết bị → tự phục hồi (quan trọng khi không người giám sát). Dùng đúng: kick ở vị trí phản ánh hệ thực sự hoạt động bình thường (cuối chu kỳ chính, hoặc khi mọi task quan trọng báo còn sống), không kick mù trong interrupt độc lập. Windowed watchdog bắt cả lỗi chạy quá nhanh.
</details>

#### DRV-018 · 🔴 · concept · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Vì sao embedded dùng A/B partition và rootfs read-only?**
<details><summary>Đáp án</summary>

A/B: ghi bản mới vào slot không dùng, boot thử; nếu hỏng thì bootloader rollback về slot cũ → cập nhật firmware an toàn, tránh bricking thiết bị ngoài field. rootfs read-only: bảo vệ hệ thống khỏi hư hỏng do ghi (mất điện đột ngột) và mòn flash; dữ liệu thay đổi để ở phân vùng data riêng. Kết hợp cho cập nhật được mà vẫn tin cậy/chịu lỗi. *(Thiết kế OTA đầy đủ: [BSP-015](bsp.md).)*
</details>

---
<a id="bus"></a>
## BUS — Bus Protocols (I2C / SPI / UART)

#### BUS-001 · 🟡 · concept · ⭐ · [→ architecture](../../../08-embedded-systems/architecture.md)
**So sánh I2C, SPI, UART.**
<details><summary>Đáp án</summary>

UART: bất đồng bộ, 2 dây, point-to-point, cần thoả thuận baud rate (console/log). I2C: đồng bộ, 2 dây dùng chung (SDA/SCL), đa thiết bị theo **địa chỉ**, tốc độ vừa, cần pull-up (sensor/EEPROM/RTC). SPI: đồng bộ, 4 dây, full-duplex, **nhanh nhất**, chọn slave bằng **CS** (flash/màn hình). Đánh đổi: số dây vs số thiết bị vs tốc độ.
</details>

#### BUS-002 · 🟡 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**Khi nào chọn I2C, khi nào SPI?**
<details><summary>Đáp án</summary>

I2C khi cần nối nhiều thiết bị tốc độ thấp với ít dây (tiết kiệm chân). SPI khi cần băng thông cao, full-duplex và chấp nhận tốn dây (mỗi slave thêm 1 CS).
</details>

#### BUS-003 · 🟢 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**UART cần hai bên thống nhất gì? Sai thì sao?**
<details><summary>Đáp án</summary>

Thống nhất **baud rate** (và data bits/parity/stop bit) vì không có dây clock. Sai baud → nhận ký tự rác.
</details>

#### BUS-004 · 🟡 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**CS trong SPI để làm gì? CPOL/CPHA là gì?**
<details><summary>Đáp án</summary>

CS (Chip Select) chọn slave nào đang giao tiếp — mỗi slave một dây CS. CPOL/CPHA quy định cực tính và pha của clock (SPI mode 0–3); master và slave phải cùng mode mới hiểu nhau.
</details>

---
⬅️ [Bank index](README.md)
