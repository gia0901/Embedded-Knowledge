# Driver Basics — Phân loại, Module, file_operations

> **TL;DR**
> - Driver là code trong kernel làm cầu nối giữa OS và phần cứng (hoặc thiết bị ảo). 3 loại chính: **character** (luồng byte: tty, sensor), **block** (truy cập khối, có cache: disk), **network** (gói tin: NIC).
> - Driver thường đóng gói thành **kernel module** (`.ko`) — nạp/gỡ động bằng `insmod`/`rmmod`/`modprobe`, không cần build lại kernel.
> - **Character driver** phơi bày qua **device node** (`/dev/xxx`) và một struct **`file_operations`** ánh xạ `open/read/write/ioctl` của user xuống hàm trong driver.
> - Mô hình hiện đại: tách **driver** và **device**, kernel **match** chúng (qua device tree/ID) rồi gọi `probe()`. Quản lý tài nguyên đúng vòng đời (request/free) là tối quan trọng — bug trong kernel = sập cả hệ thống.
> - Xử lý phần cứng: **interrupt** tốt hơn **polling** (nhưng có mẫu **lai** kiểu NAPI chống bão ngắt); **top half** ACK rồi thoát, việc nặng đẩy xuống **bottom half** — mặc định nên dùng **threaded IRQ** (tasklet đã deprecated). Dữ liệu chung với ISR phải khoá bằng **`spin_lock_irqsave`**, không phải mutex.

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

### 6.1 Polling hay interrupt?

Thiết bị báo *"tôi xong / tôi có dữ liệu"* theo hai cách:

| | **Polling** | **Interrupt** |
|---|---|---|
| Cơ chế | CPU **hỏi** thanh ghi trạng thái theo chu kỳ | Thiết bị **kéo chân IRQ**, CPU đang làm gì cũng bị ngắt |
| Chi phí khi **im lặng** | ❌ Vẫn đốt CPU + **chặn CPU vào idle state** ⇒ tốn pin | ✅ Gần như bằng 0 |
| Chi phí khi **dồn dập** | ✅ Rẻ — một vòng quét xử lý nhiều sự kiện | ❌ Mỗi sự kiện một lần vào/ra ngắt (lưu/khôi phục ngữ cảnh) |
| Độ trễ | Tệ nhất = **một chu kỳ hỏi** | Vài µs |
| Tính tất định | Dự đoán được | Phụ thuộc tải hệ thống |

**Nguyên tắc chọn:** mặc định dùng **interrupt**. Chỉ polling khi (a) thiết bị **không có** đường IRQ, (b) sự kiện đến **liên tục và rất dày** đến mức chi phí vào/ra ngắt lớn hơn chi phí quét, hoặc (c) cần **độ trễ tất định** cho một vòng điều khiển chặt.

> 🔀 **Mẫu thứ ba — lai (điều mà nhiều người bỏ sót):** bật interrupt lúc rảnh, khi ngắt đầu tiên đến thì **tắt ngắt và chuyển sang polling** cho tới khi hết dữ liệu, rồi bật lại. Đây chính là **NAPI** của network stack Linux, và là cách chống **interrupt storm** — thiết bị bắn ngắt nhanh hơn khả năng xử lý, hệ thống dành 100% thời gian vào/ra ngắt và **treo mềm** (livelock).

### 6.2 Đường đi của một ngắt

```
 thiết bị          bộ điều khiển ngắt        CPU                    kernel
┌────────┐  IRQ   ┌──────────────┐        ┌─────┐         ┌──────────────────────┐
│ sensor │───────►│  GIC / APIC  │───────►│ core│────────►│ generic IRQ layer    │
└────────┘  line  │ (map, ưu tiên│  vector└─────┘ lưu     │  → handler CỦA BẠN   │
                  │  , masking)  │                ngữ cảnh└──────────┬───────────┘
                  └──────────────┘                                   │
    device tree khai `interrupts = <...>`                            │ trả về
    → kernel dịch thành số IRQ ảo (virq)                             ▼
                                                        IRQ_HANDLED / IRQ_NONE
                                                        / IRQ_WAKE_THREAD
```

Driver **không** tự chọn số IRQ — nó lấy từ device tree qua `platform_get_irq()`; kernel đã dịch mô tả phần cứng thành **virq** (số IRQ ảo) trước đó.

### 6.3 Xin IRQ đúng cách — mã hoàn chỉnh

```c
struct my_priv {
    void __iomem *base;      // thanh ghi đã ioremap
    int           irq;
    spinlock_t    lock;      // bảo vệ dữ liệu dùng chung với ISR
    u32           events;
};

/* ---- top half: chạy trong ngữ cảnh NGẮT, phải NHANH và KHÔNG được ngủ ---- */
static irqreturn_t my_isr(int irq, void *dev_id)
{
    struct my_priv *priv = dev_id;         // chính con trỏ đã truyền lúc request
    u32 status = readl(priv->base + REG_STATUS);

    /* IRQ dùng chung: phải xác định "có phải của tôi không".
       Không phải của mình mà trả IRQ_HANDLED là NUỐT ngắt của driver khác. */
    if (!(status & STATUS_MY_EVENT))
        return IRQ_NONE;

    writel(status, priv->base + REG_STATUS);   // ACK — xoá cờ, nếu không nó bắn lại mãi

    spin_lock(&priv->lock);                    // đã ở trong ngắt ⇒ không cần _irqsave
    priv->events++;
    spin_unlock(&priv->lock);

    return IRQ_HANDLED;
}

/* ---- trong probe() ---- */
static int my_probe(struct platform_device *pdev)
{
    struct my_priv *priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
    int ret;

    if (!priv)
        return -ENOMEM;
    spin_lock_init(&priv->lock);

    priv->base = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(priv->base))
        return PTR_ERR(priv->base);

    priv->irq = platform_get_irq(pdev, 0);     // đọc từ device tree; tự in lỗi nếu thiếu
    if (priv->irq < 0)
        return priv->irq;

    /* devm_ ⇒ kernel TỰ free_irq khi device bị gỡ. dev_id = priv:
       vừa là ngữ cảnh cho handler, vừa là "chữ ký" để free đúng handler khi IRQ dùng chung. */
    ret = devm_request_irq(&pdev->dev, priv->irq, my_isr,
                           IRQF_SHARED, dev_name(&pdev->dev), priv);
    if (ret)
        return ret;

    platform_set_drvdata(pdev, priv);
    return 0;
}
```

> ⚠️ **Thứ tự trong `probe` là bắt buộc, không phải sở thích:** ánh xạ thanh ghi và khởi tạo mọi thứ mà ISR sẽ chạm **TRƯỚC** khi `request_irq`. Ngay khi hàm đó trả về, ngắt có thể bắn — kể cả từ ngắt tồn đọng của thiết bị khác trên cùng đường IRQ dùng chung.

> 🔥 **`IRQ_NONE` không phải hình thức.** Nếu **mọi** handler trên một đường IRQ đều trả `IRQ_NONE` liên tục, kernel kết luận không ai xử lý và **tắt hẳn đường ngắt đó**, kèm log kinh điển:
> ```
> irq 42: nobody cared (try booting with the "irqpoll" option)
> ```
> Thấy dòng này trong `dmesg` ⇒ hoặc ai đó quên ACK, hoặc handler trả sai giá trị.

### 6.4 Vì sao top half **không được ngủ**

Ngữ cảnh ngắt **không phải một process**: nó mượn stack của process đang chạy, `current` không có ý nghĩa gì, và **không có gì để scheduler quay lại**. "Ngủ" nghĩa là nhờ scheduler đánh thức mình sau — mà ở đây không có thực thể nào để đánh thức.

Ngủ trong ngữ cảnh atomic ⇒ kernel báo:
```
BUG: scheduling while atomic
```
Bật `CONFIG_DEBUG_ATOMIC_SLEEP` để bắt sớm — nó chèn kiểm tra vào mọi hàm có `might_sleep()`.

**Cấm tuyệt đối trong top half:**

| Cấm | Vì sao |
|---|---|
| `kmalloc(..., GFP_KERNEL)` | có thể ngủ chờ giải phóng bộ nhớ → dùng **`GFP_ATOMIC`** |
| `mutex_lock`, `down()` | semaphore/mutex là **sleeping lock** → dùng **spinlock** |
| `copy_to_user` / `copy_from_user` | có thể page fault → ngủ |
| `msleep`, `wait_event`, `schedule()` | ngủ theo đúng nghĩa đen |
| Đọc/ghi **I2C, SPI, MMC** | các bus này ngủ chờ truyền xong |
| `disable_irq()` **chính đường của mình** | chờ handler hiện tại kết thúc → **tự khoá chính mình**. Dùng `disable_irq_nosync()` |

**Và một luật ngầm quan trọng hơn cả danh sách trên:** top half chạy khi **đường ngắt đó đang bị mask**. Nó càng dài thì độ trễ ngắt của **toàn hệ thống** càng tệ. Mục tiêu là **vài µs**: ACK, đọc dữ liệu khẩn khỏi FIFO (kẻo tràn), đánh thức phần còn lại, thoát.

### 6.5 Bốn cơ chế bottom half

| Cơ chế | Ngữ cảnh | Ngủ được? | Ghi chú |
|---|---|---|---|
| **softirq** | atomic | ❌ | Cố định lúc biên dịch; hạ tầng kernel (net, block) dùng. Driver thường **không** tự viết |
| **tasklet** | atomic (trên nền softirq) | ❌ | ⚠️ **Đang bị gỡ khỏi kernel** — xem cảnh báo dưới |
| **workqueue** | **process** | ✅ | Chạy trên pool worker dùng chung (CMWQ). Linh hoạt, gom được nhiều việc |
| **threaded IRQ** | **process** (kthread riêng) | ✅ | ⭐ Mẫu **mặc định nên dùng** cho driver mới |

> ⚠️ **Kernel hiện đại (quan trọng nếu bạn đang migrate 5.10 → 6.12):** **tasklet đã deprecated**, đang được gỡ dần từ ~5.19; 6.9 thêm **BH workqueue** làm bản thay thế. Từ 5.9, `tasklet_init()` cũ nhường chỗ cho `tasklet_setup()` với chữ ký callback khác. **Code mới không nên viết tasklet** — dùng threaded IRQ hoặc workqueue. Chi tiết ở [lkd/02-interrupts-bottomhalves](../15-book-summaries/lkd/02-interrupts-bottomhalves.md).

### 6.6 Threaded IRQ — mẫu mặc định cho driver mới

Kernel tự lo việc chia đôi: hàm thứ nhất chạy trong ngữ cảnh ngắt, hàm thứ hai chạy trong một **kernel thread riêng** và **được phép ngủ**.

```c
/* top half — vẫn trong ngữ cảnh ngắt */
static irqreturn_t my_hardirq(int irq, void *dev_id)
{
    struct my_priv *priv = dev_id;
    u32 status = readl(priv->base + REG_STATUS);

    if (!(status & STATUS_MY_EVENT))
        return IRQ_NONE;

    writel(status, priv->base + REG_STATUS);   // ACK ngay
    priv->last_status = status;                // cất lại cho thread đọc

    return IRQ_WAKE_THREAD;                    // ⇒ đánh thức my_threadfn
}

/* bottom half — chạy trong kthread, ĐƯỢC PHÉP NGỦ */
static irqreturn_t my_threadfn(int irq, void *dev_id)
{
    struct my_priv *priv = dev_id;

    /* Ở đây thoải mái: đọc I2C, kmalloc(GFP_KERNEL), mutex_lock, chờ I/O... */
    i2c_smbus_read_byte_data(priv->client, REG_DATA);
    process_and_push_to_userspace(priv);

    return IRQ_HANDLED;
}

/* trong probe */
ret = devm_request_threaded_irq(&pdev->dev, priv->irq,
                                my_hardirq,      // NULL nếu không cần lọc nhanh
                                my_threadfn,
                                IRQF_ONESHOT,    // xem giải thích bên dưới
                                dev_name(&pdev->dev), priv);
```

**`IRQF_ONESHOT` làm gì:** giữ đường ngắt **bị mask cho tới khi hàm thread chạy xong**. **Bắt buộc** khi top half là `NULL` — vì không có ai ACK, ngắt mức (level-triggered) sẽ bắn lại ngay lập tức và treo máy. Ngay cả khi có top half, cờ này vẫn hữu ích với thiết bị chỉ được đọc tuần tự.

**Vì sao ưu tiên threaded IRQ hơn workqueue:**
- Có **kthread riêng** ⇒ chỉnh được **priority** (`chrt`) và CPU affinity ⇒ hợp **real-time**.
- Không tranh pool worker chung với phần còn lại của hệ thống.
- Vòng đời gắn với IRQ ⇒ không phải tự quản `cancel_work_sync` lúc remove.

> Trên kernel bật **`PREEMPT_RT`**, gần như **mọi** IRQ handler bị **ép thành threaded** để giảm độ trễ ngắt. Viết sẵn theo mẫu này là code của bạn đã tương thích RT.

### 6.7 Workqueue — khi cần gom việc hoặc hoãn có kỳ hạn

```c
struct my_priv {
    void __iomem      *base;
    struct work_struct work;      // nhúng THẲNG vào priv
    u32                pending;
};

static void my_work_fn(struct work_struct *work)
{
    /* lấy ngược ra priv từ địa chỉ của member — mẫu container_of kinh điển */
    struct my_priv *priv = container_of(work, struct my_priv, work);

    mutex_lock(&priv->io_lock);   // ✅ process context: ngủ thoải mái
    handle_pending(priv);
    mutex_unlock(&priv->io_lock);
}

static irqreturn_t my_isr(int irq, void *dev_id)
{
    struct my_priv *priv = dev_id;

    writel(IRQ_ACK, priv->base + REG_STATUS);
    priv->pending++;
    schedule_work(&priv->work);   // đẩy việc xuống, KHÔNG chờ
    return IRQ_HANDLED;
}

/* probe:  INIT_WORK(&priv->work, my_work_fn);
   remove: cancel_work_sync(&priv->work);   ← BẮT BUỘC, xem bẫy (4) */
```

Dùng workqueue thay threaded IRQ khi: việc **không gắn với một IRQ cụ thể**, cần **hoãn có kỳ hạn** (`schedule_delayed_work`), hoặc muốn **gộp nhiều lần kích hoạt thành một** (`schedule_work` khi work đang chờ thì **không xếp thêm** — đúng ý đồ chống dồn).

### 6.8 Chia sẻ dữ liệu với ISR — spinlock, **không phải** mutex

Đây là chỗ sai nhiều nhất, vì code vẫn chạy đúng cho tới khi trúng đúng thời điểm.

```c
/* PROCESS context (vd hàm read của file_operations) */
static ssize_t my_read(struct file *f, char __user *buf, size_t n, loff_t *off)
{
    struct my_priv *priv = f->private_data;
    unsigned long flags;
    u32 snapshot;

    /* _irqsave: LƯU trạng thái ngắt rồi TẮT ngắt trên CPU này.
       Thiếu bước tắt ⇒ ISR bắn ngay giữa vùng găng TRÊN CÙNG CPU
       ⇒ nó cũng đòi spinlock đó ⇒ DEADLOCK cứng. */
    spin_lock_irqsave(&priv->lock, flags);
    snapshot = priv->events;
    spin_unlock_irqrestore(&priv->lock, flags);

    return copy_to_user(buf, &snapshot, sizeof(snapshot)) ? -EFAULT : sizeof(snapshot);
}
```

**Chọn biến thể khoá theo đúng cặp ngữ cảnh đang tranh nhau:**

| Ai tranh với ai | Dùng |
|---|---|
| process ↔ process | `mutex` (ngủ được, rẻ khi giữ lâu) |
| process ↔ **hard IRQ** | **`spin_lock_irqsave` / `spin_unlock_irqrestore`** |
| process ↔ softirq/tasklet | `spin_lock_bh` |
| **trong** hard IRQ handler | `spin_lock` thường — ngắt đã bị tắt sẵn |
| process ↔ **threaded IRQ** | ✅ **`mutex` được** — cả hai đều là process context |

**Hai luật kèm theo:**
- Vùng găng giữ spinlock phải **cực ngắn** — đang tắt ngắt, và **cấm ngủ** bên trong.
- **Không bao giờ** `mutex_lock` trong hard IRQ handler. Dòng cuối bảng chính là một lý do nữa để dùng threaded IRQ: nó **trả lại cho bạn quyền dùng mutex**.

> 🆕 Trên **`PREEMPT_RT`**, `spinlock_t` biến thành **sleeping lock** (rt_mutex) — nên code cẩu thả kiểu "giữ spinlock rồi gọi hàm có thể ngủ" sẽ **nổ ra trên RT** dù chạy im trên kernel thường.

### 6.9 Bẫy thường gặp

1. **Quên ACK trong top half** ⇒ với ngắt mức, thiết bị giữ chân IRQ ⇒ **bão ngắt**, hệ thống treo mềm ngay khi nạp module.
2. **Trả `IRQ_HANDLED` cho ngắt không phải của mình** trên đường dùng chung ⇒ **nuốt ngắt** của driver khác; thiết bị kia "thỉnh thoảng mất dữ liệu" — bug cực khó truy.
3. **`request_irq` trước khi khởi tạo xong** ⇒ ISR bắn vào struct chưa sẵn sàng ⇒ oops ngay lúc nạp.
4. **Quên `cancel_work_sync()` lúc remove** ⇒ work chạy sau khi `priv` đã được giải phóng ⇒ **use-after-free trong kernel** = panic. (`devm_request_threaded_irq` không có lỗi này vì kernel tự gỡ IRQ.)
5. **`spin_lock` thường thay vì `spin_lock_irqsave`** ở process context ⇒ deadlock, nhưng **chỉ khi ISR bắn đúng lúc trên đúng CPU đó** — chạy hàng tuần mới gặp một lần.
6. **Làm việc nặng trong top half** (parse gói, `printk` nhiều dòng) ⇒ đẩy độ trễ ngắt toàn hệ thống lên, thường lộ ra ở nơi khác: audio rè, mất byte UART, RT task trượt deadline.

> **Chốt phần này:** *top half ACK rồi biến; mọi thứ còn lại đẩy xuống threaded IRQ. Câu hỏi quyết định luôn là "đoạn code này có cần ngủ không" — cần thì nó không được ở trong ngữ cảnh ngắt.*

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
| [DRV-036](../14-prep/mock-interview/bank/drivers-embedded.md) | IRQ dùng chung: handler phải làm gì khác? Trả sai `IRQ_HANDLED`/`IRQ_NONE` thì sao? |
| [DRV-037](../14-prep/mock-interview/bank/drivers-embedded.md) | Biến dùng chung giữa `read()` và ISR — khoá bằng gì? Vì sao `spin_lock` thường chưa đủ? |
| [DRV-014](../14-prep/mock-interview/bank/drivers-embedded.md) | Quản lý tài nguyên trong driver vì sao quan trọng? devm_* giúp gì? |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [kernel-userspace.md](kernel-userspace.md)
