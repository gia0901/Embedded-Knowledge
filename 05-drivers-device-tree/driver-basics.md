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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DRV-001](../14-prep/mock-interview/bank/drivers-embedded.md) | Phân biệt character, block và network driver. |
| [DRV-002](../14-prep/mock-interview/bank/drivers-embedded.md) | Kernel module là gì? Ưu điểm so với biên dịch tĩnh vào kernel? |
| [DRV-005](../14-prep/mock-interview/bank/drivers-embedded.md) | Khi user space gọi read() trên /dev/mydev, điều gì xảy ra trong driver? |
| [DRV-003](../14-prep/mock-interview/bank/drivers-embedded.md) | Major và minor number để làm gì? |
| [DRV-010](../14-prep/mock-interview/bank/drivers-embedded.md) | Probe() trong mô hình driver hiện đại làm gì? Device và driver match thế nào? |
| [DRV-011](../14-prep/mock-interview/bank/drivers-embedded.md) | Vì sao interrupt tốt hơn polling? Top half và bottom half là gì? |
| [DRV-014](../14-prep/mock-interview/bank/drivers-embedded.md) | Quản lý tài nguyên trong driver vì sao quan trọng? devm_* giúp gì? |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [kernel-userspace.md](kernel-userspace.md)
