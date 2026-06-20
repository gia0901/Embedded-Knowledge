# Driver Basics — Phân loại, Module, file_operations

> **TL;DR**
> - Driver là code trong kernel làm cầu nối giữa OS và phần cứng (hoặc thiết bị ảo). 3 loại chính: **character** (luồng byte: tty, sensor), **block** (truy cập khối, có cache: disk), **network** (gói tin: NIC).
> - Driver thường đóng gói thành **kernel module** (`.ko`) — nạp/gỡ động bằng `insmod`/`rmmod`/`modprobe`, không cần build lại kernel.
> - **Character driver** phơi bày qua **device node** (`/dev/xxx`) và một struct **`file_operations`** ánh xạ `open/read/write/ioctl` của user xuống hàm trong driver.
> - Mô hình hiện đại: tách **driver** và **device**, kernel **match** chúng (qua device tree/ID) rồi gọi `probe()`. Quản lý tài nguyên đúng vòng đời (request/free) là tối quan trọng — bug trong kernel = sập cả hệ thống.
> - Xử lý phần cứng: **interrupt** (thiết bị báo khi xong) tốt hơn **polling** (CPU hỏi liên tục); việc nặng đẩy xuống **bottom half** (workqueue/tasklet).

---

## 1. Vì sao cần driver & vì sao nó ở trong kernel?

Phần cứng đa dạng (mỗi chip thanh ghi/giao thức khác nhau), nhưng ứng dụng muốn một API thống nhất (`read`/`write`). Driver là lớp dịch: phía trên phơi bày interface chuẩn cho kernel/user, phía dưới điều khiển phần cứng cụ thể (đọc/ghi thanh ghi, xử lý ngắt, DMA).

Driver chạy trong **kernel space** vì cần: truy cập trực tiếp phần cứng/thanh ghi, xử lý interrupt, và làm việc với các tài nguyên đặc quyền mà user space bị cấm. Hệ quả: **một bug trong driver có thể sập cả hệ thống** (kernel panic) hoặc gây lỗ hổng bảo mật → đòi hỏi cẩn trọng cao.

---

## 2. Ba loại driver

| Loại | Mô hình truy cập | Đặc điểm | Ví dụ |
|------|------------------|----------|-------|
| **Character** | Luồng byte tuần tự | Đọc/ghi byte, thường không buffer phức tạp | tty, serial, sensor, GPIO, `/dev/null` |
| **Block** | Khối cố định, truy cập ngẫu nhiên | Qua block layer + page cache, có scheduler I/O | ổ cứng, SSD, eMMC, SD |
| **Network** | Gói tin (packet) | Không qua `/dev`; dùng socket API + net stack | Ethernet, WiFi NIC |

Character driver là loại hay gặp/được hỏi nhất trong phỏng vấn embedded.

---

## 3. Kernel module

```c
#include <linux/module.h>
#include <linux/init.h>

static int __init my_init(void) {     // chạy khi nạp module
    pr_info("hello driver\n");
    return 0;                         // != 0 → nạp thất bại
}
static void __exit my_exit(void) {    // chạy khi gỡ module
    pr_info("bye\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");                 // bắt buộc; non-GPL "làm bẩn" kernel
```

- Build ra `.ko`, nạp bằng `insmod my.ko` / `modprobe my` (modprobe tự xử lý dependency), gỡ bằng `rmmod`.
- Ưu điểm: phát triển/cập nhật driver mà không build lại & reboot toàn kernel; tiết kiệm bộ nhớ (chỉ nạp khi cần).
- Module chạy trong kernel space → không có libc (dùng API kernel: `kmalloc` thay `malloc`, `pr_info`/`printk` thay `printf`).

---

## 4. Character driver & `file_operations`

Khi user mở `/dev/mydev` và gọi `read`, kernel định tuyến tới hàm trong driver qua struct **`file_operations`**:

```c
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off);
static long    my_ioctl(struct file *f, unsigned int cmd, unsigned long arg);

static const struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .read    = my_read,        // ứng với read() của user
    .write   = my_write,
    .unlocked_ioctl = my_ioctl,
    .release = my_release,     // ứng với close()
};
```

Đăng ký (cấp số major/minor + tạo device node):
```c
// cách hiện đại (cdev + class → udev tự tạo /dev/mydev)
alloc_chrdev_region(&dev, 0, 1, "mydev");   // xin major/minor động
cdev_init(&my_cdev, &fops);
cdev_add(&my_cdev, dev, 1);
device_create(my_class, NULL, dev, NULL, "mydev");  // udev tạo /dev/mydev
```

- **Major number** xác định driver nào phụ trách; **minor** phân biệt thiết bị/instance trong driver.
- `udev` (user space) lắng nghe uevent và **tự tạo node** trong `/dev` — không phải `mknod` thủ công.

---

## 5. Mô hình driver hiện đại: device ↔ driver matching

Linux Device Model tách rời:
- **Device**: mô tả một thiết bị tồn tại (từ device tree, ACPI, bus enumeration như PCI/USB).
- **Driver**: code biết điều khiển một loại thiết bị, khai báo nó hỗ trợ những thiết bị nào (`of_match_table`, ID table).

Kernel **match** device với driver phù hợp rồi gọi **`probe()`** của driver (nơi khởi tạo: ánh xạ thanh ghi, xin IRQ, cấp tài nguyên); khi tháo gọi **`remove()`**.

```c
static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my-device",
        .of_match_table = my_of_match,   // khớp với compatible trong device tree
    },
};
module_platform_driver(my_driver);
```

→ Đây là lý do device tree quan trọng: nó cung cấp danh sách "device" để match (xem [device-tree.md](device-tree.md)).

---

## 6. Interrupt vs Polling & top/bottom half

Thiết bị báo "tôi xong/có dữ liệu" theo 2 cách:
- **Polling**: CPU hỏi thanh ghi trạng thái liên tục → đốt CPU, độ trễ tùy chu kỳ hỏi. Chỉ hợp khi sự kiện rất thường xuyên hoặc thiết bị không có IRQ.
- **Interrupt** (ưu tiên): thiết bị phát IRQ, CPU đang làm việc khác sẽ bị ngắt để chạy **interrupt handler** → hiệu quả, độ trễ thấp.

```c
request_irq(irq, my_isr, IRQF_SHARED, "mydev", dev);
```

**Top half / bottom half:** interrupt handler (top half) phải chạy **cực nhanh** (ngắt đang bị tắt, không được ngủ). Việc nặng/chậm hoãn lại **bottom half**:
- **Tasklet / softirq**: chạy trong ngữ cảnh atomic, nhanh.
- **Workqueue**: chạy trong ngữ cảnh process → **được phép ngủ** (vd cấp bộ nhớ, chờ I/O), linh hoạt hơn — thường ưu tiên.
- **Threaded IRQ** (`request_threaded_irq`): phần xử lý chạy trong một kernel thread riêng.

---

## 7. Quản lý tài nguyên trong driver (rất hay sai)

- Mọi thứ xin phải **trả lại** đúng thứ tự ngược khi lỗi/remove: `kmalloc`↔`kfree`, `request_irq`↔`free_irq`, `ioremap`↔`iounmap`, `request_mem_region`↔`release_mem_region`.
- Dùng **managed API `devm_*`** (`devm_kmalloc`, `devm_request_irq`, `devm_ioremap`...) — kernel **tự giải phóng** khi device bị remove → giảm leak/handle sót (giống RAII cho driver).
- Lỗi trong kernel không có "process crash" cô lập: leak = cạn kiệt dần, dùng tài nguyên đã free = **kernel oops/panic**.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Phân biệt character, block và network driver.</summary>

Character driver phơi bày thiết bị như một luồng byte tuần tự, đọc/ghi từng byte qua device node (`/dev/...`) — vd tty, serial, sensor, GPIO. Block driver quản lý thiết bị truy cập theo khối cố định và ngẫu nhiên (disk, SSD, eMMC), đi qua block layer với page cache và I/O scheduler để tối ưu/gộp request. Network driver xử lý gói tin, không xuất hiện dưới `/dev` mà gắn vào network stack và được truy cập qua socket API. Character driver là loại cơ bản và hay được hỏi nhất.
</details>

<details><summary>2) Kernel module là gì? Ưu điểm so với biên dịch tĩnh vào kernel?</summary>

Kernel module (`.ko`) là một đoạn code kernel có thể **nạp/gỡ động** lúc runtime bằng `insmod`/`modprobe`/`rmmod`, không cần biên dịch lại hay khởi động lại toàn bộ kernel. Ưu điểm: phát triển và cập nhật driver nhanh (chỉ nạp lại module), tiết kiệm bộ nhớ vì chỉ nạp khi cần thiết, và giữ kernel cốt lõi gọn. Module chạy trong kernel space nên không dùng được libc — phải dùng API kernel (`kmalloc`, `printk`...). Biên dịch tĩnh phù hợp cho driver thiết yếu lúc boot (vd driver ổ chứa rootfs).
</details>

<details><summary>3) Khi user space gọi read() trên /dev/mydev, điều gì xảy ra trong driver?</summary>

`read()` của user là một syscall; kernel tra device node `/dev/mydev` để biết major/minor → xác định driver phụ trách, rồi gọi hàm `.read` đã đăng ký trong struct `file_operations` của driver đó. Hàm read của driver nhận con trỏ `char __user *buf` (buffer của user, không được truy cập trực tiếp), thực hiện lấy dữ liệu từ phần cứng/bộ đệm, dùng `copy_to_user` để chép sang buffer user một cách an toàn, và trả về số byte đã đọc. Tương tự, `open/write/close/ioctl` ánh xạ tới `.open/.write/.release/.unlocked_ioctl`.
</details>

<details><summary>4) Major và minor number để làm gì?</summary>

Major number xác định **driver** nào phụ trách thiết bị — kernel dùng nó để định tuyến thao tác trên device node tới đúng driver. Minor number do driver dùng để phân biệt các **thiết bị/instance** cụ thể mà nó quản lý (vd nhiều cổng serial cùng một driver có cùng major, khác minor). Device node trong `/dev` mang cặp (major, minor); ngày nay thường xin major động (`alloc_chrdev_region`) và để `udev` tự tạo node thay vì `mknod` thủ công.
</details>

<details><summary>5) Probe() trong mô hình driver hiện đại làm gì? Device và driver match thế nào?</summary>

Linux Device Model tách "device" (thiết bị tồn tại, đến từ device tree, ACPI, hay enumeration của bus như PCI/USB) khỏi "driver" (code điều khiển một loại thiết bị). Mỗi driver khai báo nó hỗ trợ thiết bị nào qua một match table (vd `of_match_table` khớp thuộc tính `compatible` trong device tree, hoặc ID table cho PCI/USB). Khi kernel tìm thấy device khớp với một driver, nó gọi hàm `probe()` của driver — nơi khởi tạo: ánh xạ vùng thanh ghi (`ioremap`), xin IRQ, cấp phát tài nguyên, đăng ký với subsystem. Khi device bị tháo hoặc module gỡ, `remove()` được gọi để dọn dẹp.
</details>

<details><summary>6) Vì sao interrupt tốt hơn polling? Top half và bottom half là gì?</summary>

Polling buộc CPU liên tục hỏi thanh ghi trạng thái → lãng phí CPU và độ trễ phụ thuộc chu kỳ hỏi. Interrupt cho thiết bị chủ động báo khi có sự kiện, CPU chỉ phản ứng khi cần → hiệu quả và độ trễ thấp (polling chỉ hợp khi sự kiện cực dày hoặc thiết bị không hỗ trợ IRQ). Interrupt handler (top half) chạy với ngắt bị tắt nên phải **cực nhanh và không được ngủ**; phần xử lý nặng/có thể ngủ được hoãn sang bottom half — tasklet/softirq (ngữ cảnh atomic) hoặc workqueue/threaded IRQ (ngữ cảnh process, được phép ngủ). Điều này giữ hệ thống đáp ứng và không bỏ lỡ ngắt khác.
</details>

<details><summary>7) Quản lý tài nguyên trong driver vì sao quan trọng? devm_* giúp gì?</summary>

Vì driver chạy trong kernel: rò rỉ tài nguyên không bị cô lập như crash một process — nó tích lũy làm cạn kiệt hệ thống, còn dùng tài nguyên đã giải phóng gây kernel oops/panic. Mọi thứ xin phải được trả đúng và theo thứ tự ngược (kfree, free_irq, iounmap, release_mem_region...), kể cả trên các nhánh lỗi — rất dễ sót. Các API managed `devm_*` (devm_kmalloc, devm_request_irq, devm_ioremap...) gắn tài nguyên vào vòng đời của device và **tự giải phóng** khi device bị remove hoặc probe lỗi, đóng vai trò như RAII cho driver, giảm mạnh leak và lỗi dọn dẹp.
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [kernel-userspace.md](kernel-userspace.md)
