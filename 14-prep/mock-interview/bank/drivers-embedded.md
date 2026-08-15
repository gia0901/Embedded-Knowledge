# DRV / BUS — Drivers, Device Tree, Embedded & Bus Protocols

> Domain `DRV` (driver/embedded tổng quát) + `BUS` (I2C/SPI/UART). Câu BSP chuyên sâu ở [bsp.md](bsp.md). Track dùng: `drivers-dt`, `bsp`, `lkd`.

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
**User space gọi `read()` trên `/dev/mydev` — kể lại toàn bộ đường đi tới driver và ngược lại.**
<details><summary>Đáp án</summary>

```
  user:  read(fd, buf, n)
    │  ① libc đặt số syscall + tham số, thực thi lệnh trap
    ▼
  CPU chuyển sang KERNEL MODE  ──►  bộ điều phối syscall
    │  ② VFS: từ fd → struct file → tìm bảng file_operations của thiết bị
    ▼
  ③ gọi mydev_read(file, user_buf, count, offset)   ← HÀM CỦA BẠN
    │  ④ lấy dữ liệu (từ thanh ghi / bộ đệm nội bộ)
    │  ⑤ copy_to_user(user_buf, kbuf, n)   ← BẮT BUỘC, không được gán thẳng
    ▼
  ⑥ trả về SỐ BYTE đã đọc  ──►  VFS  ──►  syscall trả về  ──►  user
```

**Bốn điểm quyết định trong `mydev_read`:**

1. **`copy_to_user`, không bao giờ `memcpy`.** Con trỏ user có thể **không hợp lệ**, hoặc trang chưa được nạp, hoặc là mưu toan lừa kernel ghi vào vùng của chính kernel. `copy_to_user` kiểm tra quyền và xử lý được page fault ⇒ **bỏ qua là lỗ hổng bảo mật, không chỉ là bug** ([DRV-006](drivers-embedded.md)).
2. **Trả về số byte thực sự đọc**, có thể **ít hơn** `count` — đây chính là *short read* mà phía user phải xử lý bằng vòng lặp ([LNX-005](linux-sysprog.md)). Driver và ứng dụng là hai nửa của cùng một hợp đồng.
3. **Chưa có dữ liệu thì làm gì?** Đây là chỗ dễ sai nhất:
   - fd mở **blocking** ⇒ cho tiến trình **ngủ** trên một hàng đợi chờ, và đánh thức khi có dữ liệu (thường từ ISR/bottom half).
   - fd mở **`O_NONBLOCK`** ⇒ trả ngay `-EAGAIN`.
   - Đang ngủ mà có signal ⇒ trả **`-EINTR`** để user xử lý ([LNX-027](linux-sysprog.md)).
4. **Cập nhật `offset`** nếu thiết bị có ngữ nghĩa vị trí (nhiều thiết bị dòng thì không).

**Bẫy:** (1) **ngủ trong ngữ cảnh không được ngủ** (ISR, khi đang giữ spinlock) ⇒ treo hệ; (2) quên hỗ trợ `O_NONBLOCK` ⇒ ứng dụng dùng `epoll` bị chặn cả event loop; (3) trả `count` trong khi chỉ copy được một phần ⇒ ứng dụng đọc rác; (4) không hiện thực `poll` ⇒ thiết bị **không dùng được với `epoll`**.

**Chốt:** *"read() đi qua trap → VFS → `file_operations` của driver. Trong đó bắt buộc `copy_to_user`, trả về số byte thật (có thể ít hơn), và xử lý đúng ba ca: ngủ chờ, `O_NONBLOCK`, và bị signal cắt."*
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
**Device tree là gì và giải quyết vấn đề gì? Trước khi có nó thì làm thế nào?**
<details><summary>Đáp án</summary>

**Device tree là một cấu trúc DỮ LIỆU dạng cây mô tả phần cứng của bo**: có những thiết bị gì, ở địa chỉ nào, dùng ngắt nào, clock/nguồn nào cấp cho nó. Bootloader nạp nó (dạng nhị phân `.dtb`) và trao cho kernel lúc boot ([DRV-028](drivers-embedded.md)).

**Vấn đề nó giải quyết — bối cảnh trước đó:**

Trên ARM, thông tin *"bo này có gì ở đâu"* từng được **viết cứng trong mã kernel** — mỗi bo một "board file" bằng C. Hệ quả:
- Cây kernel ARM phình ra **hàng nghìn file gần giống nhau**, phần lớn chỉ khác vài địa chỉ.
- **Mỗi bo mới = sửa mã kernel + biên dịch lại kernel.**
- **Không thể** có một kernel image chạy được nhiều bo ⇒ nhà phân phối không phát hành nổi một bản ARM dùng chung (trong khi x86 làm được nhờ firmware tự mô tả).

**Device tree tách *dữ liệu mô tả* khỏi *mã điều khiển*:**

| | Đổi theo | Nằm ở |
|---|---|---|
| **Device tree** | Từng **bo** | File `.dtb` riêng |
| **Driver** | Từng **loại chip** | Kernel / module |

⇒ **Một kernel image chạy nhiều bo, chỉ đổi DTB.** Thêm bo mới thường **không phải sửa dòng mã kernel nào**.

**Cách nó ghép nối:** kernel duyệt cây, mỗi node lấy chuỗi **`compatible`** rồi tìm driver khai báo hỗ trợ chuỗi đó ⇒ gọi `probe()` ([DRV-029](drivers-embedded.md), [DRV-010](drivers-embedded.md)).

**Bẫy:**
1. ⭐ **Sai `compatible` hoặc thiếu node ⇒ thiết bị KHÔNG XUẤT HIỆN, không có thông báo lỗi.** Kernel boot bình thường, có shell, mà `/dev` thiếu — rất hay bị chẩn đoán nhầm thành "driver lỗi".
2. Tưởng DT **chứa driver** — không, nó chỉ là dữ liệu ([DRV-031](drivers-embedded.md)).
3. Nhét **logic** vào DT — nó là mô tả tĩnh, không phải ngôn ngữ lập trình.
4. Quên rằng DT mô tả cả **quan hệ phụ thuộc** (clock, regulator) ⇒ thứ tự probe không như mong đợi, phải xử lý `-EPROBE_DEFER`.

**Chốt:** *"Device tree tách mô tả phần cứng (theo bo) khỏi mã điều khiển (theo chip), thay cho board file viết cứng trong kernel — nhờ đó một kernel image chạy được nhiều bo."*
</details>

#### DRV-008 · 🟡 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**Memory-mapped I/O là gì? Vì sao truy cập thanh ghi phần cứng bắt buộc phải `volatile`?**
<details><summary>Đáp án</summary>

**MMIO:** thanh ghi của thiết bị được ánh xạ vào **không gian địa chỉ** như bộ nhớ thường ⇒ điều khiển phần cứng bằng chính lệnh đọc/ghi bộ nhớ, không cần lệnh I/O riêng.

**⚠️ Nhưng nó KHÔNG phải bộ nhớ — ba tính chất phá vỡ giả định của compiler:**

| Bộ nhớ thường | Thanh ghi thiết bị |
|---|---|
| Đọc hai lần liên tiếp ⇒ **cùng giá trị** | Có thể **khác nhau** (thanh ghi trạng thái, FIFO nhận) |
| Ghi rồi không đọc ⇒ **thao tác thừa** | Ghi **chính là hành động** (phát lệnh, xoá cờ ngắt) |
| Thứ tự đọc/ghi không quan trọng | **Thứ tự là bắt buộc** (ghi địa chỉ rồi mới ghi dữ liệu) |

Compiler được phép — và sẽ — **tối ưu dựa trên các giả định của cột trái**:
```c
uint32_t *STATUS = (uint32_t*)0x40000000;   // ❌ thiếu volatile
while (*STATUS & BUSY) { }                  // compiler: "không ai đổi *STATUS"
                                            // ⇒ đọc MỘT LẦN rồi lặp mãi
*DATA = 0x01; *DATA = 0x02;                 // ⇒ có thể BỎ lần ghi đầu ("bị đè")
```

**`volatile` nói với compiler: "mỗi lần truy cập đều có thể có tác dụng phụ"** ⇒ cấm bỏ, cấm gộp, cấm nhớ đệm, và **giữ nguyên thứ tự giữa các truy cập volatile**.

```c
#define STATUS (*(volatile uint32_t*)0x40000000)   // ✅
```

**⚠️ Ba giới hạn của `volatile` phải nói được:**
1. **Không phải công cụ đồng bộ đa luồng** — nó không cho nguyên tử, không cho hàng rào bộ nhớ giữa các core ([CPP-022](cpp.md), [CPP-056](cpp.md)).
2. **Không đảm bảo thứ tự với truy cập KHÔNG volatile**, và **không** ngăn CPU/bus sắp xếp lại ⇒ trên hệ có bộ đệm ghi, vẫn cần **rào chắn bộ nhớ** để chắc lệnh đã thực sự tới thiết bị.
3. Trong kernel Linux thì **không tự ép kiểu địa chỉ vật lý** — phải ánh xạ đúng cách và dùng hàm truy cập chuyên dụng ([BSP-012](bsp.md)); chúng đã bao gồm cả rào chắn cần thiết.

**Chốt:** *"MMIO khiến thanh ghi trông như bộ nhớ, nhưng đọc có thể đổi giá trị và ghi chính là hành động — `volatile` cấm compiler tối ưu dựa trên giả định bộ nhớ thường. Nó không phải công cụ đồng bộ và không thay được rào chắn bộ nhớ."*
</details>

#### DRV-009 · 🟡 · concept · [→ kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**`ioctl` và `sysfs` khác nhau? Khi nào dùng cái nào — và vì sao `ioctl` bị mang tiếng xấu?**
<details><summary>Đáp án</summary>

| | **`ioctl`** | **`sysfs`** |
|---|---|---|
| Hình thức | Một syscall, mã lệnh + con trỏ tới struct | **File văn bản**, một giá trị / một file |
| Kiểu dữ liệu | **Nhị phân tuỳ ý**, struct phức tạp | Chuỗi văn bản đơn giản |
| Nguyên tử | **Có** — một lệnh, nhiều tham số cùng lúc | Không — mỗi file ghi riêng |
| Dùng từ shell | Không (phải viết chương trình) | **Có** — `cat`/`echo` là đủ |
| Khám phá được? | Không — phải đọc header | **Có** — duyệt thư mục là thấy |

**Chọn thế nào:**

| Tình huống | Dùng |
|---|---|
| Đọc/đặt **một tham số đơn giản** (độ sáng, tần số, bật/tắt) | **sysfs** — script hoá được, không cần viết code |
| Thao tác **phức tạp, nhiều tham số phải đi cùng nhau** (nạp firmware, cấu hình một transaction, truyền struct) | **ioctl** — vì cần **nguyên tử** và cần kiểu nhị phân |
| Luồng dữ liệu | Không phải hai cái này ⇒ `read`/`write` hoặc `mmap` ([DRV-034](drivers-embedded.md)) |

**⚠️ Vì sao `ioctl` mang tiếng xấu — cần nói được để trả lời tròn:**
1. **Không có kiểu.** Mã lệnh là một số nguyên, tham số là `void*` ⇒ compiler **không kiểm được gì**; truyền nhầm struct là hỏng lúc chạy.
2. **Là hố đen API.** Mỗi driver tự bịa tập lệnh riêng ⇒ không nhất quán, khó học, khó bảo trì.
3. ⭐ **Bẫy tương thích 32/64-bit.** Chương trình userspace 32-bit chạy trên kernel 64-bit truyền struct có **kích thước và căn lề khác** ⇒ kernel đọc sai trường. Phải viết thêm đường xử lý riêng — lỗi rất hay gặp và rất khó lần.
4. Là **ABI vĩnh viễn**: đã phát hành thì phải giữ mãi; cần thêm trường thì phải **thiết kế sẵn phần dự phòng + trường version** ngay từ đầu.

⇒ **Nguyên tắc thực dụng:** ưu tiên **sysfs** cho mọi thứ đơn giản; chỉ dùng `ioctl` khi thật sự cần nguyên tử hoặc dữ liệu nhị phân — và khi dùng thì **đặt version vào struct ngay từ phiên bản đầu**.

**Chốt:** *"sysfs cho tham số đơn giản (script hoá được, tự khám phá được), ioctl cho thao tác nhiều tham số cần nguyên tử. ioctl không có kiểu, là ABI vĩnh viễn, và dính bẫy 32/64-bit — nên chỉ dùng khi buộc phải."*
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
**DMA là gì? Lợi ích, và vì sao nó sinh ra vấn đề *cache coherency*?**
<details><summary>Đáp án</summary>

**DMA (Direct Memory Access):** một khối phần cứng chuyển dữ liệu **thẳng giữa thiết bị và RAM**, **không cần CPU** sao chép từng byte. CPU chỉ cấu hình (địa chỉ nguồn/đích, độ dài) rồi làm việc khác; xong thì DMA **báo bằng ngắt**.

**Lợi ích:** giải phóng CPU (không còn vòng lặp copy), thông lượng cao hơn, và **độ trễ tất định hơn** cho luồng dữ liệu liên tục (audio, ADC, mạng).

**⚠️ Vấn đề cache coherency — đây là phần được hỏi:**

CPU không đọc thẳng RAM mà qua **cache**. DMA thì ghi/đọc **thẳng RAM**, thường **không đi qua cache của CPU** ⇒ hai bên nhìn thấy **hai phiên bản khác nhau** của cùng vùng nhớ:

| Hướng | Chuyện gì hỏng |
|---|---|
| **Thiết bị → RAM** (nhận dữ liệu) | DMA ghi dữ liệu mới vào RAM, nhưng CPU còn **bản cũ trong cache** ⇒ CPU đọc ra **dữ liệu cũ** |
| **RAM → thiết bị** (gửi dữ liệu) | CPU ghi vào **cache**, chưa xuống RAM ⇒ DMA đọc RAM và gửi đi **dữ liệu cũ/rác** |

**Cách xử lý — hai chiến lược:**

| | **Coherent (consistent) DMA** | **Streaming DMA** |
|---|---|---|
| Cách làm | Cấp vùng nhớ **không cache** (hoặc phần cứng tự bảo đảm nhất quán) | Vùng nhớ thường, nhưng **đồng bộ cache thủ công** trước/sau mỗi lần truyền |
| Hợp với | Vùng nhỏ, dùng **liên tục** (mô tả, vòng lặp điều khiển) | Khối dữ liệu **lớn, một lần** |
| Chi phí | CPU truy cập **chậm** (không có cache) | Phải gọi đúng hàm đồng bộ, đúng hướng, đúng thời điểm |

⭐ **Luật vàng của streaming DMA: từ lúc trao bộ đệm cho DMA tới lúc lấy lại, CPU TUYỆT ĐỐI KHÔNG được chạm vào nó.** Vi phạm luật này là nguồn của lớp bug *"dữ liệu thỉnh thoảng sai vài byte"* — phụ thuộc timing, không tái hiện được, và **không xuất hiện trên máy dev x86** (x86 có cache nhất quán với DMA, ARM thì thường không) ([BSP-011](bsp.md)).

**Bẫy khác:** (1) bộ đệm DMA phải **liên tục về mặt vật lý** và đúng **căn lề** — cấp bằng cách thông thường có thể không thoả; (2) bộ đệm nằm trên **stack** là sai (có thể không phù hợp cho DMA và biến mất khi hàm return); (3) quên rằng thiết bị nhìn thấy **địa chỉ bus**, không phải địa chỉ ảo của CPU.

**Chốt:** *"DMA chuyển dữ liệu không cần CPU, nhưng nó đi thẳng RAM còn CPU đọc qua cache ⇒ hai bên thấy hai phiên bản. Chọn coherent cho vùng nhỏ dùng liên tục, streaming cho khối lớn — và tuyệt đối không chạm bộ đệm khi đã trao cho DMA."*
</details>

#### DRV-013 · 🟠 · concept · ⭐ · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Mô tả quá trình boot của một hệ embedded Linux — và mỗi giai đoạn hỏng thì triệu chứng ra sao?**
<details><summary>Đáp án</summary>

```
 cấp nguồn → BootROM → SPL → U-Boot → kernel + DTB → init (PID 1) → ứng dụng
              (trong chip)  (SRAM)   (DRAM)
```

| Giai đoạn | Chạy ở đâu | Việc chính |
|---|---|---|
| **BootROM** | ROM trong chip, **không sửa được** | Đọc chân chọn chế độ boot, nạp **SPL** vào SRAM nội |
| **SPL** | **SRAM nội** (chỉ vài chục–vài trăm KB) | ⭐ **Khởi tạo DRAM**, rồi nạp U-Boot vào đó |
| **U-Boot** | DRAM | Khởi tạo lưu trữ/mạng, nạp **kernel + device tree**, truyền `bootargs` |
| **Kernel** | DRAM | Khởi tạo subsystem, nạp driver, **mount rootfs** |
| **init (PID 1)** | userspace | Mount, cấu hình mạng, chạy service ([BSP-030](bsp.md)) |

**⭐ Vì sao phải nhiều giai đoạn — trả lời cho đúng:** không phải "cho gọn", mà vì **SRAM nội quá nhỏ**. BootROM chỉ nạp nổi một chương trình bé xíu; chương trình đó (**SPL**) có đúng một việc quan trọng là **bật DRAM lên**, để từ đó mới đủ chỗ cho bootloader đầy đủ rồi tới kernel. **Ràng buộc kích thước SRAM chính là lý do tồn tại của SPL.**

**Chẩn đoán theo chỗ chết** — triệu chứng chỉ thẳng ra giai đoạn:

| Triệu chứng | Chết ở | Nghi gì |
|---|---|---|
| **Im lặng hoàn toàn** trên UART | BootROM/SPL | Nguồn, chân chọn chế độ boot, thiết bị boot trống/hỏng |
| Ra **ký tự rác** rồi dừng | SPL | UART chạy nhưng **sai baud** ⇒ cấu hình clock sai |
| SPL in ra rồi **treo** | Khởi tạo **DRAM** | Sai tham số DDR — bug khó nhất của bring-up |
| U-Boot chạy, kernel **im lặng** | Trao tay kernel | Sai địa chỉ nạp, thiếu/sai **DTB**, sai `bootargs` ([BSP-003](bsp.md)) |
| Panic *"unable to mount root fs"* | rootfs | Sai `root=`, **thiếu driver lưu trữ** trong kernel/initramfs ([BSP-029](bsp.md)) |
| Boot xong rồi **reboot vòng lặp** | userspace/watchdog | Init chết, hoặc watchdog không được nuôi kịp ([DRV-017](drivers-embedded.md)) |

⚠️ **Việc đầu tiên khi bring-up bo mới: làm UART chạy.** Không có console thì mọi bước sau chỉ là đoán ([BSP-022](bsp.md)).

**Chốt:** *"BootROM → SPL (khởi tạo DRAM) → U-Boot → kernel+DTB → init. SPL tồn tại vì SRAM nội quá nhỏ. Và mỗi giai đoạn chết một kiểu — triệu chứng nói cho bạn biết đang ở đâu."*
</details>

#### DRV-014 · 🟠 · concept · ⭐ · [→ driver-basics](../../../05-drivers-device-tree/driver-basics.md)
**Vì sao quản lý tài nguyên trong driver lại đặc biệt quan trọng? `devm_*` giúp gì?**
<details><summary>Đáp án</summary>

**Vì sao đặc biệt:** driver chạy **trong kernel** — không có ngoại lệ C++, không có RAII, không có tiến trình để hệ điều hành dọn hộ khi chết. **Rò một IRQ, một vùng ánh xạ, hay một clock là rò VĨNH VIỄN cho tới khi reboot.** Và tệ hơn: gỡ module rồi mà quên huỷ đăng ký handler ⇒ ngắt tới, kernel nhảy vào **mã đã bị giải phóng** ⇒ sập máy.

**⭐ Chỗ hỏng nhiều nhất là ĐƯỜNG LỖI trong `probe()`** — không phải đường thành công:

```c
/* ❌ Kiểu thủ công — mỗi bước lỗi phải nhả đúng những gì đã lấy, theo đúng thứ tự ngược */
if (!(base = ioremap(...)))          goto err;
if (!(clk = clk_get(...)))           goto err_unmap;
if (request_irq(...))                goto err_clk;
if (register_device(...))            goto err_irq;
return 0;
err_irq:   free_irq(...);
err_clk:   clk_put(clk);
err_unmap: iounmap(base);
err:       return ret;
```
Thêm một tài nguyên ở giữa là phải sửa **mọi nhãn phía dưới**. Đây là nơi bug sinh ra: đường lỗi **hiếm khi được test**, nên thiếu một `iounmap` có thể sống nhiều năm rồi mới lộ.

**`devm_*` (managed device resources) đảo ngược trách nhiệm:** tài nguyên lấy bằng `devm_ioremap`, `devm_clk_get`, `devm_request_irq`… được **gắn vào vòng đời của `struct device`**. Kernel **tự giải phóng** — theo **thứ tự ngược** — khi driver gỡ **hoặc khi `probe` thất bại giữa chừng.**

```c
/* ✅ Kiểu devm — không có nhãn lỗi, không thể quên nhả */
base = devm_ioremap_resource(dev, res);   if (IS_ERR(base)) return PTR_ERR(base);
clk  = devm_clk_get(dev, NULL);           if (IS_ERR(clk))  return PTR_ERR(clk);
ret  = devm_request_irq(dev, irq, ...);   if (ret)          return ret;
return 0;
```

⇒ Đây chính là **RAII của kernel** — cùng ý tưởng với `unique_ptr` trong C++ ([DBG-025](debugging.md)): gắn việc giải phóng vào một vòng đời có sẵn thay vì trông chờ lập trình viên nhớ.

**⚠️ Giới hạn — biết để không dùng sai:**
1. Chỉ hoạt động cho tài nguyên gắn với **một `struct device`**. Thứ có vòng đời khác (workqueue toàn cục, thứ tạo ngoài `probe`) vẫn phải nhả tay.
2. **Thứ tự giải phóng là ngược lại thứ tự lấy** — nếu phần cứng đòi thứ tự tắt khác (vd phải dừng DMA *trước* khi nhả bộ đệm) thì phải làm tay.
3. **Không dùng nửa vời:** trộn `devm_*` với thủ công trong cùng driver là công thức để nhả sai thứ tự.

**Chốt:** *"Trong kernel không có ai dọn hộ, và chỗ hỏng nhất là đường lỗi của `probe` — thứ hiếm khi được test. `devm_*` gắn tài nguyên vào vòng đời của device để kernel tự nhả, đúng tinh thần RAII."*
</details>

#### DRV-015 · 🔴 · design · ⭐ · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**"Realtime" nghĩa là gì? Khi nào chọn RTOS, khi nào chọn Linux?**
<details><summary>Đáp án</summary>

⭐ **Realtime = ĐÚNG HẠN CÓ ĐẢM BẢO, không phải nhanh.** Đo bằng **thời gian xấu nhất**, không phải trung bình: hệ luôn mất đúng 5 ms **tốt hơn** hệ trung bình 1 ms nhưng thỉnh thoảng 50 ms ([EMB-034](embedded-fundamentals.md)).

| | **RTOS** (FreeRTOS, Zephyr…) | **Linux** |
|---|---|---|
| Độ trễ ngắt | **~1 µs**, tất định | ~10–100 µs, **không đảm bảo** |
| Bộ nhớ tối thiểu | **KB** | **MB** (cần MMU) |
| Lập lịch | Ưu tiên cứng, preempt ngay | CFS công bằng; RT có nhưng vẫn không đảm bảo cứng |
| Có sẵn | Rất ít — bạn tự viết gần hết | **Khổng lồ**: TCP/IP, filesystem, USB, GUI, OTA |
| Bảo vệ bộ nhớ | Thường không (có MPU) | **Có** (MMU) — một tiến trình chết không kéo cả hệ |
| Thời gian phát triển | Chậm hơn cho tính năng phức tạp | **Nhanh** |

**Chọn theo câu hỏi chi phối:**

| Nếu… | Chọn |
|---|---|
| Có **hạn chót cứng cỡ µs–ms**, trễ là hỏng sản phẩm | **RTOS/bare-metal** |
| RAM tính bằng **KB**, chip không có MMU | **RTOS** |
| Cần mạng, filesystem, giao diện, OTA, thư viện lớn | **Linux** |
| Cần **cả hai** | **Kết hợp** — xem [BSP-028](bsp.md) |

**⚠️ Vì sao Linux thường không đủ cho hard realtime:** nhiều nguồn bất định cộng dồn — vùng **cấm ngắt/cấm preempt** trong kernel, **page fault**, tranh chấp khoá, **priority inversion**, quản lý tần số/idle state của CPU. `PREEMPT_RT` giảm mạnh nhưng vẫn là *soft/firm*, không phải đảm bảo toán học ([BSP-021](bsp.md)).

**Ba câu hỏi phải hỏi trước khi chọn** — thiếu chúng thì mọi tranh luận là vô nghĩa:
1. *"Hạn chót là bao nhiêu, đo từ sự kiện nào tới hành động nào?"* — không có con số thì chưa phải yêu cầu.
2. *"Trễ một lần thì hậu quả gì?"* — mất một mẫu hay gãy một cánh tay robot ⇒ hard hay soft.
3. *"Tần suất bao nhiêu?"* — 10 kHz và 10 Hz dẫn tới hai kiến trúc hoàn toàn khác.

**Chốt:** *"Realtime là đúng hạn có đảm bảo, đo bằng worst-case. RTOS khi có hạn chót cứng và tài nguyên tính bằng KB; Linux khi cần hệ sinh thái. Cần cả hai thì tách hai nhân — và luôn hỏi con số hạn chót trước."*
</details>

#### DRV-016 · 🔴 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Vì sao nên hạn chế cấp phát động (heap) trong embedded? Thay bằng gì?**
<details><summary>Đáp án</summary>

**Bốn vấn đề — hai cái đầu là cốt lõi:**

| Vấn đề | Vì sao chết người trên nhúng |
|---|---|
| ⭐ **Thời gian bất định** | `malloc` mất bao lâu **phụ thuộc trạng thái phân mảnh** — không có chặn trên ⇒ phá hỏng mọi đảm bảo realtime ([EMB-035](embedded-fundamentals.md)) |
| ⭐ **Phân mảnh không thể phục hồi** | Không có MMU thì **không gom lại được**: còn 100 KB trống nhưng rời rạc ⇒ không cấp nổi khối 40 KB. Thiết bị chạy vài tuần rồi `malloc` trả `NULL` dù "còn bộ nhớ" |
| **Xử lý lỗi khó** | Hết bộ nhớ ở giữa một thao tác thì làm gì? Trên máy chủ có thể thoát; thiết bị field **không được phép** |
| **Rò rỉ tích tụ** | Chạy nhiều tháng không restart ⇒ rò một chút mỗi giờ cũng đủ chết |

⇒ **Điểm mấu chốt:** vấn đề **không phải là hết bộ nhớ**, mà là **không dự đoán được** — cả về thời gian lẫn về thành công/thất bại. Hệ nhúng cần tính tất định hơn cần sự linh hoạt.

**Thay bằng gì — theo thứ tự ưu tiên:**

| Cách | Khi nào |
|---|---|
| **Cấp phát tĩnh** (mảng toàn cục, cỡ cố định) | Mặc định. Biết số lượng tối đa lúc thiết kế |
| **Memory pool** cỡ cố định | Cần cấp/thu động nhưng **cùng kích thước** — cấp/thu O(1), **không phân mảnh** |
| **Ring buffer** cấp trước | Luồng dữ liệu producer–consumer |
| **Cấp phát HẾT lúc khởi động, không bao giờ giải phóng** | Cần linh hoạt lúc cấu hình nhưng tất định lúc chạy |
| **`std::pmr`** (C++17) | Muốn **giữ nguyên** `std::vector`/`std::string` mà đổi nguồn cấp phát sang pool ⇒ không phải viết lại logic ([SD-016](system-design.md)) |

**Nguyên tắc thực dụng thường dùng:** *"cấp phát chỉ được xảy ra trong giai đoạn khởi tạo; sau khi hệ thống vào trạng thái chạy thì **không một byte nào** được cấp phát nữa."* Dễ kiểm tra (đếm lời gọi sau khi khởi động), dễ giải thích, và đủ mạnh.

**Bẫy:** (1) tưởng tránh `malloc` là đủ — `std::string`, `std::vector`, `std::function`, và cả **exception** đều có thể cấp phát ngầm ([CPP-059](cpp.md)); (2) đo bộ nhớ đỉnh **chỉ ở đường chạy bình thường**, quên đường lỗi; (3) dùng pool nhưng vẫn để nhiều cỡ khác nhau ⇒ phân mảnh quay lại.

**Chốt:** *"Vấn đề của heap trên nhúng không phải hết bộ nhớ mà là bất định — thời gian không chặn trên và phân mảnh không gom lại được. Thay bằng cấp phát tĩnh/pool, và đặt luật: sau khởi tạo thì không cấp phát nữa."*
</details>

#### DRV-017 · 🔴 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Watchdog là gì? Dùng đúng cách thế nào — và kiểu dùng nào biến nó thành đồ trang trí?**
<details><summary>Đáp án</summary>

**Watchdog là một bộ đếm ngược trong phần cứng.** Phần mềm phải **nạp lại (nuôi)** nó định kỳ; không nuôi kịp ⇒ nó **reset hệ thống**. Mục đích: cứu thiết bị khỏi trạng thái treo mà **không cần người tới nơi** — thứ bắt buộc với thiết bị ngoài hiện trường.

**⚠️ Ba cách dùng SAI — cách 1 phổ biến nhất và tệ nhất:**

| Cách dùng sai | Vì sao hỏng |
|---|---|
| ⭐ **Nuôi trong một timer/thread riêng chỉ biết nuôi** | Nó **vẫn nuôi** dù toàn bộ phần còn lại đã chết ⇒ watchdog **không bao giờ kích hoạt** ⇒ hoàn toàn vô dụng, mà lại tạo cảm giác an toàn giả |
| **Nuôi trong ISR** | ISR vẫn chạy khi main loop đã treo ⇒ cùng bệnh trên |
| **Timeout quá dài** | Thiết bị treo hàng phút mới reset — người dùng đã thấy hỏng từ lâu |

**✅ Cách dùng đúng — nuôi phải CHỨNG MINH hệ thống đang làm việc:**
- Mỗi task quan trọng **check-in** một cờ trong chu kỳ của nó; chỉ khi **tất cả** đã check-in thì mới nuôi watchdog, rồi xoá cờ.
- Nuôi ở **một chỗ duy nhất**, trong vòng lặp chính — không rải khắp code.
- Timeout đặt theo **chu kỳ chậm nhất** trong các task đó, cộng biên an toàn.

**⭐ Và quan trọng không kém: GIỮ BẰNG CHỨNG trước khi reset.** Reset sạch sẽ nghĩa là mất sạch thông tin ⇒ mỗi lần watchdog kích hoạt bạn học được **con số không**. Phải ghi lại lý do reset + trạng thái vào vùng nhớ **sống sót qua reboot**, rồi đọc ở lần boot sau ([08/hardware-debug.md](../../../08-embedded-systems/hardware-debug.md), [DBG-014](debugging.md)).

⚠️ **Bẫy lúc boot:** bật watchdog từ bootloader thì được bảo vệ toàn tuyến — nhưng nếu userspace khởi động **lâu hơn** timeout thì thiết bị **reboot vòng lặp**, trông y hệt hỏng phần cứng ([08/boot-process.md](../../../08-embedded-systems/boot-process.md)).

**Bẫy khác:** thiết bị reboot vòng lặp ngoài hiện trường mà không ai biết vì sao — nên phải có **bộ đếm số lần reset** lưu bền, và cơ chế **rơi về chế độ an toàn** sau N lần liên tiếp thay vì lặp mãi.

**Chốt:** *"Watchdog chỉ có giá trị khi việc nuôi nó CHỨNG MINH hệ thống đang làm việc — nuôi bằng một thread riêng là biến nó thành đồ trang trí. Và phải giữ bằng chứng trước khi reset, nếu không mỗi lần kích hoạt bạn không học được gì."*
</details>

#### DRV-018 · 🔴 · concept · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Vì sao thiết bị nhúng dùng A/B partition và rootfs read-only?**
<details><summary>Đáp án</summary>

**Hai kỹ thuật giải hai vấn đề khác nhau — hay bị gộp làm một.**

**① A/B partition — giải bài toán *cập nhật không được brick*.**

Giữ **hai bản** hệ thống hoàn chỉnh. Đang chạy bản A thì ghi bản mới vào **B** (A vẫn nguyên vẹn), rồi đổi cờ boot sang B và khởi động lại.

| Tình huống | Kết quả |
|---|---|
| Mất điện **giữa lúc ghi** B | A còn nguyên ⇒ boot lại vào A, **không sao** |
| B ghi xong nhưng **boot lỗi** | Bootloader đếm số lần thử; quá ngưỡng ⇒ **tự quay về A** |
| B boot và chạy tốt | Ứng dụng báo *"đã ổn"* ⇒ chốt B làm bản chính |

⇒ **Điểm mấu chốt: phải có cờ "đã xác nhận khởi động thành công"**, do **ứng dụng** đặt sau khi tự kiểm tra — không phải do bootloader đặt ngay khi kernel chạy. Thiếu bước này thì một hệ boot được nhưng ứng dụng chết vẫn bị coi là thành công ([BSP-015](bsp.md)).

**② rootfs read-only — giải bài toán *hỏng do mất điện* và *trôi cấu hình*.**

| Được gì | Cụ thể |
|---|---|
| **Chống hỏng filesystem** | Mất điện khi đang ghi là nguyên nhân hàng đầu làm hỏng rootfs. Không ghi ⇒ không hỏng |
| **Trạng thái biết trước** | Mọi thiết bị chạy **đúng một** nội dung ⇒ tái hiện lỗi được, khách không tự sửa lung tung |
| **Đỡ mòn flash** | Không ghi vào vùng hệ thống ([BSP-016](bsp.md)) |
| **Xác minh được** | Tính được checksum/chữ ký cho cả phân vùng ⇒ nền của secure boot ([BSP-026](bsp.md)) |

**Đi kèm bắt buộc:** phần cần ghi (cấu hình, log, dữ liệu) tách sang **phân vùng riêng ghi được**, dùng filesystem chịu được mất điện; và các thư mục tạm dùng RAM. Ghi cấu hình vẫn phải theo mẫu **ghi tạm → `fsync` → `rename`** ([LNX-007](linux-sysprog.md)).

**Đánh đổi:** A/B **tốn gấp đôi** dung lượng lưu trữ — trên thiết bị flash nhỏ đây là quyết định thật, và lựa chọn thay thế (cập nhật tại chỗ + phân vùng phục hồi) rẻ hơn nhưng **rủi ro cao hơn**.

**Chốt:** *"A/B cho phép cập nhật mà luôn còn một bản chạy được, với điều kiện có cờ xác nhận do ứng dụng đặt. rootfs read-only chặn hỏng do mất điện và giữ mọi thiết bị ở đúng một trạng thái — trả bằng gấp đôi dung lượng."*
</details>

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
**So sánh UART, I2C, SPI — chọn cái nào cho tình huống nào?**
<details><summary>Đáp án</summary>

| | **UART** | **I2C** | **SPI** |
|---|---|---|---|
| Số dây | 2 (TX, RX) | **2** (SDA, SCL) | **4** (MOSI, MISO, SCLK, CS) + 1 CS mỗi slave |
| Đồng bộ? | **Không** — hai bên tự giữ nhịp | Có (SCL) | Có (SCLK) |
| Số thiết bị | **2** (điểm–điểm) | **Nhiều** (địa chỉ 7-bit) | Nhiều, nhưng **mỗi cái một chân CS** |
| Tốc độ điển hình | 9,6 k – 1 Mbps | 100 k / 400 k / 1 Mbps | **10 – 100 Mbps** |
| Song công | Có | Không (nửa song công) | **Có** (MOSI + MISO cùng lúc) |
| Xác nhận nhận được | Không | **Có** (ACK/NAK mỗi byte) | Không |
| Độ phức tạp chân | Thấp | **Thấp nhất khi nhiều thiết bị** | Cao (tốn chân theo số slave) |

**Chọn thế nào — theo ràng buộc chi phối:**

| Ràng buộc chi phối | Chọn | Vì sao |
|---|---|---|
| **Tốc độ cao** (màn hình, ADC nhanh, flash, thẻ SD) | **SPI** | Nhanh hơn cả bậc độ lớn, song công |
| **Nhiều cảm biến chậm, ít chân** (nhiệt độ, EEPROM, RTC, PMIC) | **I2C** | Thêm thiết bị **không tốn thêm chân** — chỉ tốn một địa chỉ |
| **Nối ra ngoài bo / khoảng cách xa / console** | **UART** | Đơn giản, không cần clock chung; nền của serial console và module GPS/GSM |

**Ba điểm ăn điểm khi so sánh:**
1. ⭐ **I2C tiết kiệm chân, SPI tiết kiệm thời gian.** Đây là đánh đổi cốt lõi — 20 cảm biến I2C vẫn 2 dây, còn 20 slave SPI cần **24 chân**.
2. **Chỉ I2C có xác nhận.** Mỗi byte có ACK/NAK ⇒ master **biết** thiết bị có ở đó và có nhận được không. SPI ghi vào chỗ trống cũng "thành công" — im lặng. Đây là lý do bring-up I2C dễ chẩn đoán hơn ([BUS-007](drivers-embedded.md)).
3. **UART không có clock chung** ⇒ hai bên phải **thoả thuận trước baud rate**; lệch quá ~2% là ra ký tự rác. Đó là chữ ký của lỗi cấu hình clock lúc bring-up ([08/boot-process.md](../../../08-embedded-systems/boot-process.md)).

**Bẫy:** (1) chọn I2C cho luồng dữ liệu lớn (màn hình) rồi ngạc nhiên vì chậm; (2) quên I2C cần **pull-up** ([BUS-005](drivers-embedded.md)); (3) quên SPI cần thống nhất **CPOL/CPHA** — sai là đọc ra rác mà không báo lỗi gì ([BUS-004](drivers-embedded.md)).

**Chốt:** *"I2C tiết kiệm chân và có ACK nên hợp nhiều cảm biến chậm; SPI nhanh và song công nhưng tốn một chân CS mỗi thiết bị; UART không cần clock chung nên hợp nối ra ngoài bo."*
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
**Chân CS trong SPI để làm gì? CPOL/CPHA là gì — và sai thì hiện tượng ra sao?**
<details><summary>Đáp án</summary>

**CS (Chip Select):** SPI **không có địa chỉ** như I2C, nên master chọn slave bằng cách **kéo chân CS của riêng nó xuống**. Slave nào không được chọn thì thả MISO ở trạng thái trở kháng cao (không tham gia). ⇒ Thêm slave = **thêm một chân CS**, đó là cái giá của SPI.

CS còn đánh dấu **ranh giới một giao dịch**: nhiều chip yêu cầu CS giữ thấp **suốt** cả lệnh (địa chỉ + dữ liệu); nhả CS giữa chừng là chip reset trạng thái nội bộ ⇒ lỗi *"đọc được byte đầu, các byte sau là rác"*.

**CPOL/CPHA — hai bit quyết định *lấy mẫu dữ liệu ở đâu*:**

| | Nghĩa |
|---|---|
| **CPOL** (polarity) | Mức **nghỉ** của clock: 0 = nghỉ ở thấp, 1 = nghỉ ở cao |
| **CPHA** (phase) | Lấy mẫu ở sườn **thứ nhất** (0) hay sườn **thứ hai** (1) của mỗi chu kỳ |

4 tổ hợp = **mode 0–3**. Master và slave **phải khớp**; datasheet của chip nói nó cần mode nào.

**⚠️ Sai thì hiện tượng thế nào — đây là phần được hỏi:**
- **Không có báo lỗi nào cả.** SPI không có ACK ⇒ giao dịch vẫn "thành công", chỉ là dữ liệu sai.
- Triệu chứng điển hình: đọc ra giá trị **lệch một bit** (dịch trái/phải), hoặc luôn `0x00`/`0xFF`, hoặc giá trị **chập chờn đổi theo tốc độ clock**.
- ⭐ Dấu hiệu nhận diện: **hạ tốc độ SPI xuống rất thấp mà vẫn sai** ⇒ nghi mode; còn *chỉ sai khi tốc độ cao* ⇒ nghi tín hiệu/đi dây/thời gian setup-hold.

**Cách xử lý khi bring-up:** đọc datasheet lấy mode đúng; nếu tài liệu mơ hồ thì **thử cả 4 mode** (rẻ hơn suy luận); và **đo bằng logic analyzer** — nhìn sườn nào dữ liệu ổn định là biết ngay, không cần đoán.

**Chốt:** *"CS chọn slave và đánh dấu ranh giới giao dịch; CPOL/CPHA quyết định lấy mẫu ở sườn nào. Sai mode thì SPI không báo lỗi — chỉ ra dữ liệu sai, nên hạ tốc mà vẫn sai là dấu hiệu nghi mode."*
</details>

#### BUS-005 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ bus-protocols](../../../05-drivers-device-tree/bus-protocols.md)
**Vì sao I2C bắt buộc phải có điện trở pull-up? Chọn giá trị thế nào — và quên thì hiện tượng ra sao?**
<details><summary>Đáp án</summary>

**Cơ chế: I2C dùng ngõ ra *open-drain*.** Thiết bị chỉ có khả năng **kéo đường xuống 0**, hoàn toàn **không có khả năng đẩy lên 1**. Không có pull-up thì mức cao không bao giờ tồn tại — đường dây **thả nổi**.

**Vì sao thiết kế như vậy — hai lý do, không phải tuỳ tiện:**
1. **Nhiều thiết bị dùng chung một dây.** Nếu mỗi bên đều đẩy được lên 1, hai bên đẩy ngược nhau là **chập** (một bên kéo 0, một bên đẩy 1 ⇒ dòng lớn, hỏng chip). Open-drain khiến việc dùng chung an toàn: ai cũng chỉ được kéo xuống.
2. **Cho phép AND có dây (wired-AND)** — nền của hai cơ chế cốt lõi của I2C: **arbitration** (nhiều master, ai kéo xuống trước thì thắng, bên thua tự phát hiện và rút) và **clock stretching** (slave giữ SCL xuống để bắt master chờ — [BUS-006](drivers-embedded.md)).

**Chọn giá trị — đánh đổi hai chiều:**

| | Pull-up **nhỏ** (vd 1 kΩ) | Pull-up **lớn** (vd 10 kΩ) |
|---|---|---|
| Sườn lên | **Nhanh** — kéo lên dứt khoát | **Chậm** — bo tròn, có thể không kịp đạt mức cao |
| Dòng tiêu thụ khi kéo 0 | **Lớn** | Nhỏ (tốt cho thiết bị pin) |
| Hợp với | Bus dài, nhiều thiết bị, tốc độ cao (400 kHz+) | Bus ngắn, ít thiết bị, ưu tiên tiết kiệm điện |

Ràng buộc vật lý: thời gian sườn lên ≈ **R × C**, với C là **điện dung tổng của cả bus** (dây + mọi chân thiết bị). Càng nhiều thiết bị và dây càng dài thì C càng lớn ⇒ phải **giảm R**. Giá trị điển hình **2,2–4,7 kΩ**.

**⚠️ Bẫy thực chiến:**
1. **Quên hẳn pull-up** ⇒ SDA/SCL thả nổi, đọc ra rác hoặc luôn 0 ⇒ mọi giao dịch `NAK`. Triệu chứng: *"driver đúng mà thiết bị không bao giờ trả lời"*.
2. ⭐ **Lắp pull-up ở NHIỀU module cùng lúc** — mỗi bo con mang sẵn 4,7 kΩ, cắm 4 cái là thành **~1,2 kΩ song song** ⇒ dòng quá lớn, thiết bị không kéo nổi mức thấp hợp lệ ⇒ lỗi **ngẫu nhiên theo số thiết bị đang cắm**. Cực khó lần nếu không nghĩ tới.
3. **Pull-up lên sai điện áp** — bus 1,8 V mà kéo lên 3,3 V ⇒ hỏng chip hoặc mức logic sai; cần level shifter.
4. Đây là lớp lỗi **phần cứng** mà triệu chứng trông y hệt lỗi phần mềm ⇒ khi bring-up, **đo bằng scope trước khi truy code**.

**Chốt:** *"I2C open-drain nên chỉ kéo được xuống — pull-up tạo mức cao, và chính nó cho phép wired-AND để có arbitration và clock stretching. Trị số là đánh đổi giữa tốc độ sườn lên và dòng tiêu thụ; nhiều module cắm chung dễ thành pull-up song song quá nhỏ."*
</details>

#### BUS-006 · 🟠 · concept · ⭐ · 📦 2026-08-13 · [→ bus-protocols](../../../05-drivers-device-tree/bus-protocols.md)
**Clock stretching trong I2C là gì? Nó gây ra lớp lỗi kiểu nào?**
<details><summary>Đáp án</summary>

**Cơ chế:** bình thường **master** phát xung SCL. Nhưng vì SCL là **open-drain + pull-up** ([BUS-005](drivers-embedded.md)), **slave** có thể **giữ SCL xuống mức 0** — master thả SCL lên mà thấy nó vẫn ở 0 thì phải **đợi**. Đó là cách slave nói *"tôi chưa sẵn sàng, khoan đã"*.

**Vì sao slave cần:** nó đang bận làm việc thật — cảm biến đang **chuyển đổi ADC**, EEPROM đang **ghi vào ô nhớ** (hàng ms), MCU-làm-slave đang bận trong ISR khác. Không có cơ chế này thì master cứ bắn xung và dữ liệu ra sẽ là rác.

**⚠️ Lớp lỗi nó gây ra — và vì sao đau:**

| Lỗi | Biểu hiện |
|---|---|
| **Master không hỗ trợ stretching** | Nhiều bộ điều khiển I2C (nhất là loại **bit-bang bằng GPIO**, và một số IP trong SoC) **không kiểm** SCL có thực sự lên hay chưa ⇒ chúng cứ chạy tiếp ⇒ **dữ liệu lệch bit**, đọc ra giá trị vô nghĩa |
| **Master có timeout quá ngắn** | Slave giữ lâu hơn timeout ⇒ master bỏ cuộc, báo lỗi bus |
| ⭐ **Treo bus vĩnh viễn** | Slave bị reset/mất điện **đúng lúc đang giữ SCL hoặc SDA xuống** ⇒ đường dây kẹt ở 0 ⇒ **toàn bộ bus chết**, kể cả các thiết bị khác |

**Vì sao khó chẩn đoán:** lỗi **phụ thuộc thời điểm** — chỉ xảy ra khi slave tình cờ bận (vừa được yêu cầu đo, vừa ghi EEPROM). Nên nó biểu hiện thành *"thỉnh thoảng đọc sai một giá trị"*, và **không tái hiện được** khi bạn chạy chậm lại để quan sát.

**Cách xử lý:**
- **Đọc datasheet của cả hai bên**: slave có stretch không, master có hỗ trợ không, timeout bao nhiêu. Đây là ca kinh điển *"chip nào cũng đúng, ghép lại thì sai"*.
- **Hạ tốc độ bus** — thường làm vấn đề biến mất, và đó là **chẩn đoán** chứ không chỉ là cách chữa.
- **Khôi phục bus bị kẹt:** phát tay tối đa 9 xung SCL để slave đẩy nốt bit còn lại rồi nhả SDA, sau đó phát điều kiện STOP. Nhiều driver có sẵn thủ tục này; thiết bị field nên có.
- **Đo bằng scope/logic analyzer** — nhìn thấy SCL bị giữ thấp là xong, không cần đoán ([08/hardware-debug.md](../../../08-embedded-systems/hardware-debug.md)).

**Chốt:** *"Slave giữ SCL xuống để bắt master chờ. Nguy hiểm ở chỗ nhiều master không kiểm SCL đã lên chưa nên chạy tiếp và lệch bit — lỗi phụ thuộc thời điểm, không tái hiện được khi chạy chậm."*
</details>

#### BUS-007 · 🟠 · design · ⭐ · 🏗️ · 📦 2026-08-13 · [→ bus-protocols](../../../05-drivers-device-tree/bus-protocols.md)
**Cảm biến I2C mới gắn không phản hồi (`NAK` mọi giao dịch). Bạn debug theo trình tự nào?**
<details><summary>Đáp án</summary>

**Nguyên tắc: đi từ tầng VẬT LÝ lên tầng phần mềm** — vì lỗi phần cứng biểu hiện y hệt lỗi phần mềm, mà kiểm phần cứng lại **nhanh hơn nhiều**. Sai thứ tự là mất hàng ngày đọc code trong khi vấn đề là một chân chưa hàn.

| # | Kiểm gì | Cách kiểm | Loại trừ được |
|---|---|---|---|
| **1** | **Nguồn & chân** | Đo VCC/GND của cảm biến; kiểm SDA/SCL có nối đúng (không đảo) | Chưa cấp nguồn, hàn thiếu, đảo dây |
| **2** | **Pull-up** | Đo mức nghỉ của SDA/SCL — phải là **mức cao**. Ở 0 hoặc lơ lửng ⇒ thiếu pull-up hoặc có kẻ đang kéo xuống | [BUS-005](drivers-embedded.md) |
| **3** | **Có xung không** | Scope/logic analyzer trên SCL khi phần mềm phát lệnh. **Không có xung ⇒ vấn đề ở phía master/driver, chưa tới cảm biến** | Chia đôi bài toán tại đây |
| **4** | **Địa chỉ** | Quét bus xem có ai trả lời. Datasheet cho địa chỉ **7-bit**, nhiều tài liệu/code viết dạng **8-bit đã dịch trái** ⇒ lệch 1 bit là NAK vĩnh viễn. Kiểm cả chân **A0/A1** chọn địa chỉ | ⭐ Lỗi phổ biến **số một** |
| **5** | **Mức điện áp** | Cảm biến 1,8 V mà bus 3,3 V (hoặc ngược lại) ⇒ cần level shifter | Không tương thích mức |
| **6** | **Tốc độ & stretching** | Hạ xuống 100 kHz thử; hết lỗi ⇒ nghi timing/stretching | [BUS-006](drivers-embedded.md) |
| **7** | **Phần mềm** | Lúc này mới đọc driver: đúng bus number? device tree khai đúng? thứ tự thanh ghi? | — |

**⭐ Bước 3 là bước chia đôi quan trọng nhất** — nó trả lời *"vấn đề nằm ở bên tôi hay bên nó?"*, đúng tinh thần *mỗi thí nghiệm phải loại một nửa* ([DBG-008](debugging.md)).

**Hai câu hỏi nên hỏi sớm, tiết kiệm hàng giờ:**
- *"Có bo nào giống hệt mà chạy được không?"* — có ⇒ so sánh phần cứng trước khi so sánh code.
- *"Trước đây nó từng chạy chưa?"* — từng chạy ⇒ tìm cái vừa đổi (bisect), chưa bao giờ ⇒ nghi khai báo/phần cứng.

**Bẫy:** (1) tin driver hơn tin scope — chân không ra tín hiệu thì mọi lập luận về code là vô nghĩa; (2) nhiều thiết bị **trùng địa chỉ** trên cùng bus ⇒ trả lời chồng nhau; (3) quên rằng một slave đang **kẹt kéo SDA xuống** làm **cả bus chết**, nên "cảm biến mới" có thể chỉ là nạn nhân.

**Chốt:** *"Đi từ vật lý lên: nguồn → pull-up → có xung không → địa chỉ → mức áp → tốc độ → mới tới code. Bước 'có xung không' chia đôi bài toán, và sai địa chỉ 7-bit/8-bit là lỗi phổ biến nhất."*
</details>

#### DRV-028 · 🟢 · concept · 📦 2026-08-13 · [→ device-tree](../../../05-drivers-device-tree/device-tree.md)
**DTS, DTB, `dtc` là gì?**
<details><summary>Đáp án</summary>

**DTS** (`.dts`/`.dtsi`) là **mã nguồn** device tree — văn bản người đọc được, mô tả phần cứng theo cây. `.dtsi` là phần dùng chung để `#include` (vd mô tả SoC), `.dts` là bo cụ thể.

**`dtc`** (device tree compiler) biên dịch DTS → **DTB** (`.dtb`, còn gọi *flattened device tree*) — dạng **nhị phân** mà bootloader nạp vào RAM và trao cho kernel.

```
   board.dts  +  soc.dtsi   ──dtc──►   board.dtb   ──bootloader──►   kernel
   (văn bản, người sửa)                (nhị phân)                    (đọc lúc boot)
```

**Điểm đáng nhớ:** DTB là **dữ liệu**, không phải mã — nó chỉ *mô tả* phần cứng, không chứa logic điều khiển ([DRV-031](drivers-embedded.md)). Vì tách rời khỏi kernel image nên **đổi mô tả phần cứng không cần biên dịch lại kernel** — chỉ thay DTB.

**Chốt:** *"DTS là nguồn, `dtc` biên dịch, DTB là nhị phân bootloader trao cho kernel — và nó là dữ liệu mô tả, không phải mã."*
</details>

#### DRV-029 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ device-tree](../../../05-drivers-device-tree/device-tree.md)
**Property `compatible` dùng để làm gì? Vì sao thường khai nhiều giá trị?**
<details><summary>Đáp án</summary>

**`compatible` là KHOÁ GHÉP NỐI giữa một node phần cứng trong device tree và một driver trong kernel.** Kernel duyệt cây, với mỗi node lấy chuỗi `compatible` rồi tìm driver nào khai báo hỗ trợ chuỗi đó — khớp thì gọi `probe()` của driver ([DRV-010](drivers-embedded.md)).

```dts
sensor@48 {
    compatible = "ti,tmp102", "ti,tmp101";   /* cụ thể trước, tương thích ngược sau */
    reg = <0x48>;
};
```

**Vì sao nhiều giá trị — đây là phần ăn điểm:** danh sách xếp **từ cụ thể nhất tới tổng quát nhất**. Kernel thử lần lượt.
⇒ Lợi ích: chip mới `tmp102` chạy được **ngay** với kernel cũ chỉ biết `tmp101` (vì tương thích thanh ghi), **không cần sửa gì**. Sau này khi có driver riêng cho `tmp102` biết khai thác tính năng mới, nó tự động thắng vì được thử trước.

**Quy ước tên: `"nhàsảnxuất,tênchip"`** — tiền tố nhà sản xuất để tránh đụng tên giữa các hãng.

**Bẫy:**
1. ⭐ **Sai một ký tự trong `compatible` ⇒ không driver nào khớp ⇒ thiết bị đơn giản là KHÔNG XUẤT HIỆN** — không có thông báo lỗi nào cả. Đây là ca *"kernel boot bình thường, có shell, mà `/dev` thiếu thiết bị"*, rất hay bị nhầm thành lỗi driver ([08/boot-process.md](../../../08-embedded-systems/boot-process.md)).
2. Khai chuỗi tổng quát **trước** chuỗi cụ thể ⇒ mãi mãi dùng driver kém hơn.
3. Tự bịa chuỗi không có trong tài liệu binding ⇒ chạy được trên cây của bạn nhưng không ai upstream chấp nhận, và bản kernel sau đổi là gãy.

**Chốt:** *"`compatible` là khoá ghép node phần cứng với driver, xếp từ cụ thể tới tổng quát để chip mới chạy được với driver cũ. Sai một ký tự thì thiết bị biến mất im lặng."*
</details>

#### DRV-030 · 🟡 · concept · 📦 2026-08-13 · [→ device-tree](../../../05-drivers-device-tree/device-tree.md)
**Driver lấy thông tin từ device tree bằng cách nào? Kể các loại tài nguyên nó lấy ra.**
<details><summary>Đáp án</summary>

Trong `probe()`, driver nhận con trỏ tới **device node** của chính nó rồi **hỏi** kernel từng tài nguyên. Điểm cốt lõi: driver **không tự đọc file DTB** — nó dùng API của kernel, và kernel đã phân tích cây sẵn.

| Cần gì | DT khai gì | Driver lấy ra thành |
|---|---|---|
| **Địa chỉ thanh ghi** | `reg = <0x40000000 0x1000>` | Vùng địa chỉ đã **ánh xạ** sẵn, dùng ngay |
| **Số ngắt** | `interrupts = <...>` | Số IRQ để đăng ký handler |
| **Xung nhịp** | `clocks = <&clk 5>` | Đối tượng clock để bật/tắt, đặt tần số |
| **Nguồn** | `vdd-supply = <&reg_3v3>` | Regulator để bật nguồn cho chip |
| **Chân GPIO** | `reset-gpios = <&gpio1 7 ...>` | Đối tượng GPIO để nhấp chân reset |
| **Tham số riêng** | `sample-rate = <100>` | Giá trị số/chuỗi tuỳ ý |

**⭐ Hai điểm thiết kế đáng nói:**
1. **Driver không hard-code địa chỉ.** Cùng một driver chạy trên nhiều bo với địa chỉ khác nhau — vì địa chỉ đến **từ dữ liệu**, không từ mã. Đây chính là lý do device tree tồn tại.
2. **Dùng biến thể `devm_*`** để kernel **tự giải phóng** khi driver gỡ hoặc khi `probe` lỗi giữa chừng ⇒ xoá bỏ cả lớp bug rò tài nguyên trên đường lỗi ([DRV-014](drivers-embedded.md)).

**Bẫy:** (1) tài nguyên **có thể chưa sẵn sàng** — clock/regulator do driver khác cung cấp mà driver đó chưa nạp ⇒ trả `-EPROBE_DEFER`, kernel sẽ **thử lại sau**; coi nó là lỗi thật rồi bỏ cuộc là bug hay gặp; (2) không kiểm giá trị trả về của từng lệnh lấy tài nguyên ⇒ crash sâu bên trong với con trỏ `NULL`; (3) quên rằng thứ tự bật **nguồn → clock → nhả reset** là ràng buộc **phần cứng** ghi trong datasheet, DT không tự lo hộ.

**Chốt:** *"Driver hỏi kernel từng tài nguyên (reg, interrupts, clocks, gpios) trong `probe` thay vì hard-code — nên cùng driver chạy được nhiều bo. Và phải xử lý `-EPROBE_DEFER` vì tài nguyên có thể chưa sẵn sàng."*
</details>

#### DRV-031 · 🟡 · concept · 📦 2026-08-13 · [→ device-tree](../../../05-drivers-device-tree/device-tree.md)
**Device tree có chứa driver không? Nó khác driver ở chỗ nào?**
<details><summary>Đáp án</summary>

**Không. Đây là hai thứ khác loại hoàn toàn:**

| | **Device tree** | **Driver** |
|---|---|---|
| Là gì | **Dữ liệu** — mô tả *có cái gì, ở đâu* | **Mã** — biết *làm thế nào để điều khiển* |
| Trả lời câu | *"Bo này có chip TMP102 ở bus I2C-1, địa chỉ 0x48, IRQ 27"* | *"Muốn đọc nhiệt độ thì ghi thanh ghi 0x01 rồi đọc 2 byte ở 0x00"* |
| Đổi theo | **Từng bo** | **Từng loại chip** |
| Nằm ở | File `.dtb` riêng, bootloader nạp | Trong kernel hoặc module `.ko` |

**Phép loại suy:** device tree là **danh sách linh kiện + sơ đồ nối dây**; driver là **hướng dẫn sử dụng** của một loại linh kiện. Có sơ đồ mà không có hướng dẫn thì không dùng được; có hướng dẫn mà không biết linh kiện nối ở đâu cũng vậy.

**Vì sao tách ra — vấn đề nó giải quyết:** trước device tree, thông tin *"bo này có gì ở đâu"* được **hard-code trong mã kernel** (mỗi bo một file board), khiến kernel ARM phình ra hàng nghìn file gần giống nhau và **mỗi bo mới phải sửa và biên dịch lại kernel**. Tách dữ liệu ra ⇒ **một kernel image chạy nhiều bo**, chỉ đổi DTB.

⇒ Đây là hiện thân của nguyên tắc **tách dữ liệu khỏi mã** — cùng ý tưởng với việc tách cấu hình khỏi chương trình.

**Bẫy:** cố nhét **logic** vào device tree (thứ tự khởi tạo phức tạp, điều kiện) — DT là **mô tả tĩnh**, không phải ngôn ngữ lập trình. Logic thuộc về driver.

**Chốt:** *"DT là dữ liệu mô tả phần cứng theo từng bo; driver là mã điều khiển theo từng loại chip. Tách ra để một kernel image chạy được nhiều bo."*
</details>

#### DRV-032 · 🟠 · concept · 📦 2026-08-13 · [→ device-tree](../../../05-drivers-device-tree/device-tree.md)
**Device tree khác ACPI thế nào? Vì sao thế giới ARM nhúng dùng DT còn PC dùng ACPI?**
<details><summary>Đáp án</summary>

Cả hai giải **cùng một bài toán**: *"kernel làm sao biết máy này có phần cứng gì, ở đâu?"* — nhưng theo hai triết lý ngược nhau.

| | **Device tree** | **ACPI** |
|---|---|---|
| Bản chất | **Dữ liệu tĩnh** — bảng mô tả | Dữ liệu **+ mã thực thi** (bytecode do firmware cung cấp, kernel chạy trong máy ảo thông dịch) |
| Ai giữ | Thường **trong cây kernel**, do nhà phát triển bo duy trì | **Trong firmware/BIOS**, do nhà sản xuất máy cung cấp |
| Đổi mô tả | Sửa DTS, biên dịch lại DTB | Phải **cập nhật firmware** |
| Quản lý nguồn | Kernel tự điều khiển clock/regulator | Gọi **phương thức do firmware cung cấp** — kernel không cần biết chi tiết |

**Vì sao mỗi bên chọn khác nhau — lý do là MÔ HÌNH KINH DOANH, không phải kỹ thuật thuần:**

- **PC/server:** một bản Windows/Linux phải chạy trên **hàng nghìn** máy của hàng trăm hãng, và **hệ điều hành không được biết trước** phần cứng. ⇒ Đẩy phần đặc thù vào **firmware**: máy tự mô tả và tự cung cấp mã điều khiển nguồn. Hệ điều hành chỉ gọi. Đổi lại: **firmware lỗi thì hệ điều hành chịu**, và ACPI phức tạp hơn nhiều.
- **ARM nhúng:** nhà phát triển hệ thống **kiểm soát cả phần cứng lẫn kernel**, và SoC có hàng trăm biến thể clock/regulator rất đặc thù. ⇒ Mô tả **tĩnh** là đủ, mà lại **đơn giản, kiểm tra được, sửa được** — không phụ thuộc firmware của bên thứ ba.

**Xu hướng đáng biết:** **ARM server** dùng **ACPI**, không dùng DT — vì mô hình kinh doanh giống PC (một OS chạy nhiều máy của nhiều hãng). ⇒ Chứng minh đúng luận điểm: **lựa chọn theo mô hình phân phối, không theo kiến trúc CPU**.

**Chốt:** *"DT là mô tả tĩnh do người làm bo giữ; ACPI là mô tả kèm mã do firmware giữ. Chọn cái nào phụ thuộc ai kiểm soát phần cứng — đó là lý do ARM server dùng ACPI dù cùng kiến trúc với ARM nhúng."*
</details>

#### DRV-033 · 🟡 · concept · 📦 2026-08-13 · [→ kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Vì sao phải tách kernel space và user space? Cơ chế nào thực thi ranh giới đó?**
<details><summary>Đáp án</summary>

**Vấn đề nếu không tách:** mọi chương trình đều chạm được vào phần cứng và vào bộ nhớ của nhau ⇒ một chương trình lỗi (hoặc độc hại) làm sập cả máy, và **không có gì phân xử** khi hai chương trình cùng muốn dùng một thiết bị.

**Ranh giới được thực thi bởi PHẦN CỨNG, không phải bằng quy ước — đây là điểm mấu chốt:**

| Cơ chế | Thực thi cái gì |
|---|---|
| **Mức đặc quyền của CPU** (user mode / kernel mode) | Lệnh đặc quyền (đổi bảng trang, cấm ngắt, truy cập cổng I/O) **chạy ở user mode là sinh exception** — không phải "không nên", mà là *không thể* |
| **MMU + bit quyền trong page table** | Trang của kernel đánh dấu "chỉ kernel"; user chạm vào ⇒ **fault**. Mỗi process cũng không thấy trang của process khác ([OS-008](os.md)) |
| **Cổng vào duy nhất: syscall** | User **không gọi thẳng** hàm kernel; nó thực thi lệnh trap ⇒ CPU chuyển mức đặc quyền và nhảy tới **một điểm vào do kernel định trước**. Không tự chọn được nhảy đi đâu |

⇒ **Ba tính chất có được:** *bảo vệ* (lỗi bị chặn lại), *phân xử* (kernel là trọng tài duy nhất cho tài nguyên chung), *trừu tượng hoá* (ứng dụng thấy "file", không thấy sector đĩa).

**Cái giá:** mỗi lần vượt ranh giới tốn **chuyển mức đặc quyền + lưu/khôi phục trạng thái** — lý do syscall đắt hơn hàm thường cả bậc độ lớn, và là động lực của mọi kỹ thuật **gom syscall** (buffer stdio, `epoll`, `io_uring`, `mmap`) ([LNX-002](linux-sysprog.md)).

**Bẫy:** (1) tưởng ranh giới do kernel "tự giữ kỷ luật" — không, **CPU cưỡng chế**; (2) tưởng hệ không MMU thì không có tách biệt — vẫn có **mức đặc quyền**, chỉ mất phần bảo vệ bộ nhớ ([OS-024](os.md)); (3) quên rằng **driver chạy trong kernel** nên nó **không được bảo vệ** — con trỏ hỏng trong driver là hỏng cả hệ ([DBG-020](debugging.md)).

**Chốt:** *"Ranh giới do phần cứng cưỡng chế: mức đặc quyền CPU + bit quyền trong page table, và syscall là cổng vào duy nhất. Nhờ vậy có bảo vệ, phân xử và trừu tượng hoá — trả giá bằng chi phí mỗi lần vượt ranh giới."*
</details>

#### DRV-034 · 🟠 · concept · 📦 2026-08-13 · [→ kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Khi nào nên cho userspace giao tiếp với driver bằng `mmap` thay vì `read`/`write`/`ioctl`?**
<details><summary>Đáp án</summary>

**Cơ chế:** driver hiện thực `mmap` để **ánh xạ thẳng** một vùng — bộ đệm DMA, hoặc thanh ghi thiết bị — vào không gian địa chỉ của tiến trình. Sau đó userspace truy cập như bộ nhớ thường: **không syscall, không copy**.

**Chọn `mmap` khi cả hai điều sau đúng:**
1. **Dữ liệu lớn và tần suất cao** — ví dụ khung hình video, bộ đệm âm thanh, mẫu ADC liên tục. `read()` mỗi khung là **một lần copy + một syscall**; ở 30–60 fps với vài MB/khung thì chi phí đó là thật.
2. **Truy cập ngẫu nhiên hoặc dùng lại nhiều lần** trên cùng vùng nhớ.

| | `read`/`write` | `ioctl` | **`mmap`** |
|---|---|---|---|
| Hợp với | Luồng dữ liệu tuần tự | **Lệnh điều khiển**, cấu hình | **Khối dữ liệu lớn, lặp lại** |
| Copy | 1 lần mỗi lần gọi | Nhỏ | **0** |
| Syscall mỗi lần truyền | Có | Có | **Không** |
| Báo "có dữ liệu mới" | Có sẵn (fd sẵn sàng) | — | **Không có** ⇒ vẫn cần `poll`/`ioctl`/`eventfd` |

⇒ **Mẫu thực tế: `mmap` cho dữ liệu + `poll`/`ioctl` cho điều khiển và báo hiệu.** Đây đúng là cách các framework video/âm thanh của Linux làm.

**⚠️ Cái giá — vì sao không dùng mmap cho mọi thứ:**
1. **Mất điểm kiểm soát.** Với `read`/`ioctl`, driver biết chính xác **khi nào** userspace chạm dữ liệu nên kiểm tra hợp lệ được. Với `mmap`, userspace ghi bất cứ lúc nào — driver **không chen vào được**. Nếu vùng đó là thanh ghi thiết bị thì userspace **ghi bậy là treo phần cứng**.
2. **Đồng bộ trở thành việc của bạn** — hệt như shared memory giữa hai process ([LNX-015](linux-sysprog.md)).
3. **Cache coherency.** Bộ đệm DMA phải xử lý đúng chuyện cache của CPU và thiết bị nhìn thấy dữ liệu khác nhau — sai là dữ liệu cũ/rác, và bug **phụ thuộc kiến trúc** ([DRV-012](drivers-embedded.md)).
4. **An ninh:** map nhầm vùng ⇒ lộ bộ nhớ kernel hoặc cho userspace quyền chọc phần cứng.

**Chốt:** *"`mmap` xoá được copy và syscall nên đáng dùng cho khối dữ liệu lớn tần suất cao — nhưng nó lấy đi điểm kiểm soát của driver và đẩy đồng bộ, cache coherency sang cho bạn. Lệnh điều khiển thì vẫn `ioctl`."*
</details>

#### DRV-035 · 🔴 · design · ⭐ · 🏗️ · 📦 2026-08-13 · [→ kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Bạn quyết định đặt một chức năng ở kernel space hay user space dựa trên nguyên tắc nào?**
<details><summary>Đáp án</summary>

**Nguyên tắc gốc: mặc định là USER SPACE. Chỉ đưa vào kernel phần BẮT BUỘC phải ở đó.**

**Bốn lý do chính đáng để vào kernel** — không có cái nào trong bốn thì đừng vào:

| Lý do | Ví dụ |
|---|---|
| **Cần đặc quyền phần cứng** | Chạm thanh ghi thiết bị, nhận ngắt, cấu hình DMA |
| **Cần độ trễ mà userspace không đạt được** | Xử lý trong ISR, đáp ứng vài µs |
| **Cần phân xử tài nguyên dùng chung** | Nhiều tiến trình cùng dùng một thiết bị |
| **Chi phí vượt ranh giới quá lớn** | Đường dữ liệu tần suất rất cao mà mỗi gói phải qua lại user↔kernel |

**Vì sao mặc định là userspace — cái giá của kernel:**

| | Kernel | User |
|---|---|---|
| Lỗi con trỏ | **Sập cả máy** | Chết một process, hệ vẫn sống |
| Debug | Khó hơn hẳn — mất khả năng thử nhanh ([DBG-020](debugging.md)) | gdb, ASan, Valgrind thoải mái |
| Thư viện | Rất hạn chế, không C++ chuẩn, **stack chỉ 8–16 KB** | Đầy đủ |
| Cập nhật | Thường phải reboot | Restart tiến trình |
| Giấy phép & upstream | Ràng buộc GPL, phải theo API kernel luôn đổi | Tự do |

⇒ **Chi phí bảo trì của code kernel cao hơn nhiều lần** — đó mới là lý do thật của nguyên tắc này, không phải hiệu năng.

**Mẫu chuẩn hay dùng — chia đôi:** để trong kernel **phần mỏng nhất có thể** (chạm phần cứng, gom dữ liệu, phát sự kiện), đẩy **toàn bộ chính sách và logic** ra userspace. Kernel lo *"làm thế nào"*, userspace lo *"làm gì và khi nào"*.

**Ví dụ áp dụng:** thuật toán xử lý ảnh từ cảm biến ⇒ **userspace** (dùng `mmap` để tránh copy — [DRV-034](drivers-embedded.md)). Đọc mẫu ADC ở 10 kHz theo ngắt và gom vào ring buffer ⇒ **kernel**, vì userspace không đảm bảo được độ trễ đó.

⚠️ **Lối thoát đáng cân nhắc:** một số lớp thiết bị có **framework driver ở userspace** (vd USB, một số bus) — đủ nhanh cho nhiều ca và giữ được mọi lợi thế của userspace. Hỏi *"có làm được ở userspace không?"* trước khi viết module kernel.

**Chốt:** *"Mặc định userspace; vào kernel chỉ khi cần đặc quyền phần cứng, độ trễ, phân xử, hoặc chi phí vượt ranh giới. Và mẫu đúng là kernel mỏng nhất có thể — kernel lo 'làm thế nào', userspace lo 'làm gì'."*
</details>

---
⬅️ [Bank index](README.md)
