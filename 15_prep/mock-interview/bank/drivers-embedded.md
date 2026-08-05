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

**Nguyên tắc nền: con trỏ từ user là *dữ liệu không tin được*, không phải địa chỉ.** Bốn lý do, hai về đúng đắn và hai về bảo mật:

| | Vấn đề | Hậu quả nếu deref thẳng |
|---|---|---|
| 1 | Địa chỉ **không hợp lệ** (chưa map, đã `munmap`) | **Kernel oops** — sập cả hệ thống, không phải chỉ crash 1 process |
| 2 | Page **bị swap ra / chưa fault-in** | Page fault trong ngữ cảnh không xử lý được |
| 3 | 🔒 User cố tình đưa **địa chỉ kernel** | Kernel tự đọc/ghi vùng của chính nó theo lệnh user → **leo thang đặc quyền** |
| 4 | 🔒 Process khác / thời điểm khác | Con trỏ chỉ có nghĩa trong **address space** của process đó |

```c
// ❌ TUYỆT ĐỐI KHÔNG
static ssize_t my_write(struct file *f, const char __user *ubuf, size_t len, loff_t *off) {
    memcpy(kbuf, ubuf, len);          // deref thẳng con trỏ user -> oops / lỗ hổng
}

// ✅ Đúng
    if (len > sizeof(kbuf)) return -EINVAL;        // ✅ luôn kiểm tra len TRƯỚC
    if (copy_from_user(kbuf, ubuf, len))           // trả về SỐ BYTE CHƯA chép được
        return -EFAULT;                            // ✅ khác 0 = lỗi
```

`copy_from_user`/`copy_to_user` (và `get_user`/`put_user` cho biến đơn) làm ba việc: **xác thực** vùng địa chỉ thuộc user space, **xử lý page fault an toàn** (fault-in hoặc bỏ cuộc sạch sẽ), và trả **số byte chưa chép được** thay vì nổ.

**Chú thích `__user`** trên tham số không phải trang trí — nó cho **sparse** (`make C=1`) kiểm tra tĩnh và báo lỗi nếu bạn deref nhầm. Luôn giữ.

**Bẫy:** (1) tưởng giá trị trả về là số byte **đã** chép — ngược lại, **0 = thành công**; (2) quên kiểm tra `len` trước khi copy → user truyền `len` khổng lồ gây tràn buffer kernel; (3) gọi `copy_*_user` trong **ngữ cảnh atomic** (spinlock/ISR) — nó **có thể ngủ** (page fault) → deadlock; (4) **TOCTOU**: copy vào kernel rồi validate, đừng validate trên bộ nhớ user rồi mới copy — user có thể đổi giữa hai bước.

**Chốt:** *"Con trỏ user chỉ là một con số user đưa cho bạn. Luôn qua `copy_*_user`, luôn kiểm tra độ dài, và nhớ 0 mới là thành công."*
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

**Ý tưởng nền — Linux Device Model tách đôi:** **device** (thiết bị *tồn tại*: đến từ device tree, ACPI, hoặc bus tự liệt kê như PCI/USB) và **driver** (code *biết điều khiển* một loại thiết bị). Hai bên đăng ký độc lập với **bus**; bus lo việc **ghép đôi**.

**Match theo bus — mỗi bus một cách:**

| Bus | Khớp bằng | Nguồn device |
|---|---|---|
| Platform (I2C/SPI/MMIO on-chip) | `of_match_table` ↔ thuộc tính **`compatible`** trong DTS | **Device tree** — phải khai tay |
| PCI | `id_table` ↔ **Vendor ID / Device ID** đọc từ config space | **Tự liệt kê** (enumeration) |
| USB | `id_table` ↔ VID/PID / class | **Tự liệt kê** |

Khớp được → bus gọi **`probe()`** của driver.

```c
static int my_probe(struct platform_device *pdev) {
    void __iomem *base = devm_platform_ioremap_resource(pdev, 0);  // ánh xạ thanh ghi
    if (IS_ERR(base)) return PTR_ERR(base);
    int irq = platform_get_irq(pdev, 0);
    ret = devm_request_irq(&pdev->dev, irq, my_isr, 0, "mydrv", priv);   // xin IRQ
    // … cấp tài nguyên, đăng ký với subsystem (input/net/tty/…)
    return 0;
}
static struct platform_driver my_drv = {
    .probe = my_probe, .remove = my_remove,
    .driver = { .name = "mydrv", .of_match_table = my_of_match },
};
```

**`probe()` làm gì:** ánh xạ thanh ghi (`ioremap`), xin IRQ, cấp bộ nhớ/DMA, lấy clock/regulator/GPIO/pinctrl, rồi **đăng ký với subsystem** phù hợp để thiết bị lộ ra userspace. `remove()` làm ngược lại — hoặc dùng **`devm_*`** để kernel tự dọn (RAII cho driver, [DRV-014](drivers-embedded.md)).

**Bẫy:** (1) ⭐ **`-EPROBE_DEFER`** — probe chạy khi thiết bị *phụ thuộc* (regulator, clock, GPIO controller) **chưa sẵn sàng**; đừng coi là lỗi thật, phải **trả `-EPROBE_DEFER`** để kernel thử lại sau. Đây là câu hỏi rất hay gặp ([BSP-006](bsp.md)); (2) probe **thất bại giữa chừng** phải nhả đúng thứ tự ngược — dùng `devm_*` để khỏi sót; (3) tưởng "cắm module là driver chạy" — không có device khớp thì `probe()` **không bao giờ** được gọi; kiểm bằng `/sys/bus/*/devices`, `/sys/bus/*/drivers`; (4) `compatible` trong DTS phải khớp **chuỗi**, sai một ký tự là im lặng không probe.

**Chốt:** *"Device và driver đăng ký riêng, bus ghép chúng qua match table (`compatible` hoặc VID/DID). Khớp thì `probe()` chạy — và nếu phụ thuộc chưa sẵn sàng thì trả `-EPROBE_DEFER` chứ đừng báo lỗi."*
</details>

#### DRV-011 · 🟠 · concept · ⭐ · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Top half / bottom half trong xử lý interrupt là gì?**
<details><summary>Đáp án</summary>

**Vấn đề cần giải:** interrupt handler chạy trong **ngữ cảnh ngắt** — ngắt (ít nhất là cùng loại) đang bị chặn, **không có process context** nên **không được ngủ**. Giữ nó lâu = tăng latency cho mọi ngắt khác, có thể **mất** ngắt. Nhưng việc cần làm thì thường nặng (xử lý gói tin, cấp bộ nhớ, đọc I2C chậm).

**Giải pháp: chia đôi.**
- **Top half** (hard IRQ) — chạy **ngay**, làm **tối thiểu**: ack thiết bị (xoá cờ ngắt để nó không bắn lại), đọc dữ liệu khẩn khỏi thanh ghi, rồi **lên lịch** bottom half. Vài chục lệnh.
- **Bottom half** — chạy **sau**, làm phần nặng, ở ngữ cảnh dễ thở hơn.

**Chọn loại bottom half — tiêu chí quyết định là *có cần ngủ không*:**

| Cơ chế | Ngữ cảnh | Ngủ được? | Dùng khi |
|---|---|---|---|
| **softirq** | atomic | ❌ | Hạ tầng kernel (mạng, block) — driver thường không tự viết |
| **tasklet** | atomic (trên softirq) | ❌ | Việc ngắn, nhanh. *(đang bị deprecate dần)* |
| **workqueue** | **process** | ✅ | Mặc định nên chọn: cấp bộ nhớ `GFP_KERNEL`, đọc I2C/SPI, chờ I/O |
| **threaded IRQ** (`request_threaded_irq`) | **process** (kthread riêng) | ✅ | ⭐ Cách hiện đại: gọn, có thể chỉnh priority — hợp **real-time** |

```c
// Mẫu hiện đại: kernel tự lo top/bottom half
request_threaded_irq(irq,
                     my_hardirq,      // top half: ack + đọc nhanh, trả IRQ_WAKE_THREAD
                     my_threadfn,     // bottom half: chạy trong kthread, ĐƯỢC PHÉP NGỦ
                     IRQF_ONESHOT, "mydrv", priv);
```

**Bẫy:** (1) ⚠️ gọi hàm **có thể ngủ** trong top half — `kmalloc(GFP_KERNEL)`, `mutex_lock`, `copy_to_user`, `msleep`, đọc I2C — là bug nghiêm trọng, kernel báo *"scheduling while atomic"*; trong top half phải `GFP_ATOMIC` và **spinlock** ([OS-006](os.md)); (2) quên **ack/xoá cờ ngắt** trong top half → ngắt bắn lại vô hạn, hệ thống treo; (3) dùng tasklet cho việc cần ngủ — nó vẫn là ngữ cảnh atomic, không phải "nhẹ hơn nên an toàn hơn"; (4) chia sẻ dữ liệu giữa top half và phần còn lại phải dùng `spin_lock_irqsave` (chặn ISR trên **cùng core** tự khoá chính mình).

**Chốt:** *"Top half: ack rồi biến, không được ngủ. Bottom half: làm phần nặng. Cần ngủ → workqueue hoặc threaded IRQ; không cần → tasklet/softirq."*
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
## PCI / USB drivers

#### DRV-019 · 🟡 · concept · ⭐ · [→ pci-usb-drivers §1](../../../05-drivers-device-tree/pci-usb-drivers.md)
**Vì sao PCI/USB không cần khai trong device tree như I2C/SPI?**
<details><summary>Đáp án</summary>

PCI/USB **tự liệt kê** (self-enumerating): PCI có **configuration space** (Vendor/Device ID, class, BAR) kernel quét lúc boot rồi gán địa chỉ; USB có **descriptor** host đọc khi cắm. Kernel biết thiết bị *là gì* và *ở đâu* mà không cần khai. I2C/SPI **không discoverable** — bus không có cơ chế hỏi "ai đang cắm ở địa chỉ này" → phải khai trong **device tree** (compatible + reg). Đây là lý do gốc DT tồn tại cho embedded bus.
</details>

#### DRV-020 · 🟠 · concept · [→ pci-usb-drivers §1.2](../../../05-drivers-device-tree/pci-usb-drivers.md)
**Một PCI driver probe làm những gì? BAR là gì?**
<details><summary>Đáp án</summary>

Driver khai `pci_driver` với `id_table` (Vendor/Device ID) + `MODULE_DEVICE_TABLE(pci,…)`; kernel match ID quét được → gọi `probe(pdev)`. Trong probe: `pci_enable_device` → `pci_request_regions` (xin quyền BAR) → `pci_iomap(pdev, bar, 0)` map **BAR** thành MMIO → `pci_set_master` (cho phép DMA) → `dma_set_mask` → xin IRQ → đăng ký subsystem. **BAR** (Base Address Register, trong config space) khai thiết bị cần vùng địa chỉ (MMIO/IO) kích thước bao nhiêu; kernel/firmware gán địa chỉ, driver `ioremap` để chạm thanh ghi. Ưu tiên `pcim_*`/`devm_*` để tự dọn.
</details>

#### DRV-021 · 🟠 · concept · ⭐ · [→ pci-usb-drivers §1.3](../../../05-drivers-device-tree/pci-usb-drivers.md)
**INTx vs MSI/MSI-X trên PCI khác nhau thế nào?**
<details><summary>Đáp án</summary>

**Khác biệt gốc: báo ngắt bằng *dây* hay bằng *một lần ghi bộ nhớ*.**
- **INTx** (legacy): thiết bị **kéo một đường dây vật lý** (INTA–INTD). Chỉ có 4 đường cho cả bus → **nhiều thiết bị dùng chung**, **level-triggered**.
- **MSI/MSI-X**: thiết bị **ghi một giá trị vào một địa chỉ** đã định (thực chất là một transaction PCIe ghi bộ nhớ). Interrupt controller thấy lần ghi đó và sinh ngắt. Không có dây riêng nào cả.

| | **INTx** | **MSI** | **MSI-X** |
|---|---|---|---|
| Cơ chế | Dây vật lý | Ghi bộ nhớ | Ghi bộ nhớ |
| Chia sẻ | ✅ **có** — phải hỏi "của mình không?" | ❌ riêng | ❌ riêng |
| Số vector / device | 1 (dùng chung) | tới **32**, phải liên tiếp | tới **2048**, rời rạc |
| Kiểu | Level-triggered | Edge-like | Edge-like |
| Định tuyến tới core | Khó | ✅ mỗi vector một CPU | ✅ |

**Vì sao MSI-X quan trọng thực tế — ba lợi ích, không chỉ "nhanh hơn":**
1. **Không chia sẻ** → handler **không phải** đọc thanh ghi status để xác nhận "ngắt này của tôi"; bỏ được một lần đọc MMIO (đắt) mỗi ngắt, và bỏ luôn lớp bug "trả `IRQ_NONE` sai".
2. **Nhiều vector riêng** → NIC gán **mỗi queue một vector**, mỗi vector ghim vào **một core** → xử lý song song thật, không có điểm nghẽn chung.
3. **Không còn race của level-triggered** — INTx phải ack đúng cách nếu không ngắt bắn lại vô hạn.

```c
int nvec = pci_alloc_irq_vectors(pdev, 1, want,
                                 PCI_IRQ_MSIX | PCI_IRQ_MSI | PCI_IRQ_INTX);  // ưu tiên giảm dần
if (nvec < 0) return nvec;
int irq = pci_irq_vector(pdev, 0);
request_irq(irq, my_isr, 0, "mydrv", priv);      // ✅ MSI/MSI-X: KHÔNG cần IRQF_SHARED
```

**Bẫy:** (1) vẫn truyền `IRQF_SHARED` khi dùng MSI-X — thừa, và cho thấy chưa hiểu là nó không chia sẻ; (2) code chỉ chạy đúng khi có đủ vector — `pci_alloc_irq_vectors` có thể trả **ít hơn** yêu cầu, phải xử lý; (3) quên rằng MSI-X cần **BAR** chứa bảng MSI-X được map; (4) hỏi ngược: *"vì sao thiết bị cũ vẫn dùng INTx?"* — tương thích, và một số nền tảng/ảo hoá không hỗ trợ đủ.

**Chốt:** *"INTx = kéo dây, ít vector, phải chia sẻ. MSI/MSI-X = ghi một message vào bộ nhớ → mỗi nguồn ngắt một vector riêng, ghim được vào core. Hiện đại thì MSI-X."*
</details>

#### DRV-022 · 🟠 · concept · [→ pci-usb-drivers §1.4](../../../05-drivers-device-tree/pci-usb-drivers.md)
**DMA trên PCI hoạt động thế nào? Vai trò `pci_set_master`?**
<details><summary>Đáp án</summary>

PCI device là **bus master** — tự đọc/ghi RAM không cần CPU; `pci_set_master()` bật khả năng đó. Driver dùng cùng DMA API: `dma_alloc_coherent` (descriptor ring, uncached) + `dma_map_single/sg` (payload streaming, kernel flush/invalidate cache); `dma_set_mask_and_coherent` khai độ rộng địa chỉ device chịu được (32/64-bit) để kernel cấp buffer trong tầm. PCIe thường qua **IOMMU** (dịch + bảo vệ địa chỉ). Vẫn phải lo cache maintenance nếu hệ không coherent. *(Nền DMA: [BSP-011](bsp.md).)*
</details>

#### DRV-023 · 🟡 · concept · ⭐ · [→ pci-usb-drivers §2.1](../../../05-drivers-device-tree/pci-usb-drivers.md)
**Kiến trúc USB: descriptor và transfer types?**
<details><summary>Đáp án</summary>

USB **host-centric** (thiết bị chỉ nói khi host hỏi), host controller = xHCI (USB3)/EHCI. Descriptor phân cấp: **Device** (VID/PID, class) → **Configuration** → **Interface** (một chức năng — driver bind ở mức *interface*) → **Endpoint** (kênh dữ liệu một chiều). **Transfer types** theo endpoint: **Control** (setup/điều khiển), **Bulk** (dữ liệu lớn, tin cậy — storage/máy in), **Interrupt** (nhỏ, định kỳ, độ trễ giới hạn — chuột/bàn phím), **Isochronous** (đúng nhịp, không đảm bảo — audio/video).
</details>

#### DRV-024 · 🟡 · concept · [→ pci-usb-drivers §2.2](../../../05-drivers-device-tree/pci-usb-drivers.md)
**USB enumeration diễn ra thế nào? Driver match theo gì?**
<details><summary>Đáp án</summary>

Cắm vào → host phát hiện (pull-up), **reset**, gán **address**, đọc descriptor, chọn configuration, rồi **match driver** theo **VID/PID** (driver riêng) hoặc theo **class** (HID, Mass Storage, CDC → driver class dùng chung, cắm là chạy không cần driver riêng). Sau đó driver dùng endpoint trao đổi. Đây là lý do chuột/USB stick "cắm là nhận" — dùng class driver có sẵn.
</details>

#### DRV-025 · 🟠 · concept · [→ pci-usb-drivers §2.3](../../../05-drivers-device-tree/pci-usb-drivers.md)
**USB host driver viết thế nào? URB là gì?**
<details><summary>Đáp án</summary>

Khai `usb_driver` với `id_table`; bind ở mức **interface** (`probe(intf, id)`). Giao tiếp qua **URB** (USB Request Block) = mô tả một lần truyền tới một endpoint: `usb_submit_urb()` **bất đồng bộ** → xong thì callback chạy (hợp streaming, phải re-submit cho interrupt-in). Bản **đồng bộ** tiện hơn cho trao đổi đơn giản: `usb_control_msg()`, `usb_bulk_msg()` (block tới khi xong/timeout). `disconnect()` khi rút.
</details>

#### DRV-026 · 🟠 · concept · ⭐ · [→ pci-usb-drivers §2.4](../../../05-drivers-device-tree/pci-usb-drivers.md)
**USB gadget là gì? Khi nào dùng?**
<details><summary>Đáp án</summary>

Khi board embedded của bạn đóng vai **USB device** (cắm vào PC), không phải host. Dùng **USB gadget framework** + driver **UDC** (USB Device Controller). Chọn **function**: `g_serial`/CDC-ACM (cổng COM ảo — hay dùng cho console/debug), `g_mass_storage` (ổ USB), `g_ether` (mạng qua USB), HID; ghép nhiều function qua **configfs** (composite gadget). OTG/dual-role: board vừa host vừa device tùy chiều cắm. Phân biệt rõ **host driver** (điều khiển thiết bị cắm vào) vs **gadget** (làm cho mình thành thiết bị) là điểm hay bị hỏi.
</details>

#### DRV-027 · 🟡 · concept · [→ pci-usb-drivers §2.5](../../../05-drivers-device-tree/pci-usb-drivers.md)
**Debug một vấn đề USB thế nào?**
<details><summary>Đáp án</summary>

`lsusb` / `lsusb -t` (cây thiết bị + driver nào bind), `dmesg` (enumerate/reset/disconnect, lỗi cấp nguồn), **`usbmon`** + Wireshark (bắt gói USB thật — xem transfer nào fail), `/sys/kernel/debug/usb`. Lỗi hay gặp: sai endpoint/direction, quên re-submit URB interrupt-in, thiếu quyền (cần **udev rule**), UDC/gadget không match, thiếu dòng điện (hub). Nguyên tắc: xác nhận enumerate được trước (lsusb), rồi mới soi transfer (usbmon).
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
