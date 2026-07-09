# MELP — Driver Interface, Init & Power Management (ch. 11–15) 🎯

> Thuộc [MELP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Ch. 12 (prototyping với breakout board) chỉ lấy vài ý — phần "tay to" của nó nằm luôn trong cụm 1.

---

## Cụm 1 — Giao tiếp driver từ userspace & viết module (ch. 11–12) 🎯

### Nội dung chính

**Bốn cửa từ userspace vào kernel/phần cứng** — chọn đúng cửa là câu hỏi thiết kế:

| Cửa | Dùng cho | Ví dụ |
|---|---|---|
| **/dev** (char/block) | Dòng dữ liệu, ioctl điều khiển | `/dev/ttyS0`, `/dev/i2c-1`, `/dev/spidev0.0`, `/dev/video0` |
| **sysfs** `/sys` | Thuộc tính đơn giản đọc/ghi (một giá trị một file) | trigger LED, cat gpio, trạng thái driver |
| **debugfs** | Nội soi debug (không phải ABI ổn định) | `/sys/kernel/debug/...` |
| **netlink/proc** | Sự kiện bất đồng bộ, thống kê legacy | udev event, `/proc/interrupts` |

- Nguyên tắc kernel: **sysfs là ABI** (`Documentation/ABI/`) — một giá trị/file; nhồi format phức tạp vào sysfs là red flag review.
- **Không phải cái gì cũng cần kernel driver**: bus có sẵn driver tổng quát cho userspace — `i2c-dev` (`ioctl(I2C_RDWR)`), `spidev`, **libgpiod** (`/dev/gpiochipN` — thay sysfs GPIO cũ đã deprecated), UART termios, `/dev/mem` + `mmap` (map thanh ghi trực tiếp — vũ khí bring-up, tội đồ sản phẩm). Prototype bằng userspace trước, viết driver thật khi cần ngắt/hiệu năng/framework (iio, input, v4l2...).
- **Khung một platform driver tối thiểu** (đủ khoe trong phỏng vấn):

```c
static int mydev_probe(struct platform_device *pdev) {
    struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    base = devm_ioremap_resource(&pdev->dev, res);       // map thanh ghi (theo reg trong DT)
    irq  = platform_get_irq(pdev, 0);
    devm_request_threaded_irq(&pdev->dev, irq, NULL,     // threaded IRQ: handler chạy
                              mydev_thread_fn,           // trong thread — được phép ngủ
                              IRQF_ONESHOT, "mydev", priv);
    clk = devm_clk_get_enabled(&pdev->dev, NULL);        // devm_*: tự giải phóng khi remove/lỗi
    return 0;                                            // hoặc -EPROBE_DEFER nếu thiếu tài nguyên
}
static const struct of_device_id mydev_of[] = {
    { .compatible = "myco,mydev" }, { }                  // ← khớp node DT
};
MODULE_DEVICE_TABLE(of, mydev_of);                       // để udev tự modprobe (modalias)
static struct platform_driver mydev_driver = {
    .probe = mydev_probe,
    .driver = { .name = "mydev", .of_match_table = mydev_of },
};
module_platform_driver(mydev_driver);
```

- Từ khóa ăn điểm trong khung trên: **devm_** (managed resource — hết leak trong error path), **threaded IRQ** (phần nặng chạy được trong context ngủ được), **MODULE_DEVICE_TABLE** (tự động nạp module theo uevent), `-EPROBE_DEFER`.
- Module: `insmod` (nạp thẳng file) vs `modprobe` (giải quyết dependency qua `modules.dep` + tìm theo tên/alias); `lsmod`, `rmmod`, tham số module qua `/sys/module/*/parameters`.

### Insight đáng nhớ

- Thang quyết định đáng nhớ: **"userspace được thì userspace"** (i2c-dev/spidev/libgpiod/uio) — nhanh viết, chết không kéo kernel; xuống kernel khi cần: **ngắt thật sự, DMA, latency, hoặc gia nhập framework chuẩn** (để ứng dụng chuẩn dùng được thiết bị). Trình bày được *thang* này giá trị hơn viết thuộc lòng một char driver.
- `/dev/mem` là con dao hai lưỡi định danh nghề BSP: bring-up ai cũng dùng (devmem đọc thanh ghi kiểm tra clock/pinmux trước khi đổ lỗi driver), nhưng có mặt trong sản phẩm = lỗ hổng bảo mật + vượt mặt driver đang chạy.

### Ít quan trọng

- Chi tiết breakout board/SparkFun cụ thể của ch. 12 (bài lab); danh sách đầy đủ subsystem framework (iio/input/leds/rtc...) — gặp đâu học đó theo subsystem.

### Góc interview

**Câu 1 (🎯):** Vì sao interrupt handler không được ngủ? Threaded IRQ giải quyết gì? Top half / bottom half là gì?

<details><summary>Đáp án</summary>

- **Không được ngủ vì không có ai để đánh thức đúng:** handler chạy trong **interrupt context** — mượn ngữ cảnh CPU hiện tại, **không phải một process/thread có task_struct schedule được**; "ngủ" nghĩa là gọi scheduler nhường CPU và chờ được wake — interrupt context không có chỗ đứng trong hàng đợi scheduler, ngủ = treo CPU/panic (`scheduling while atomic`). Ngoài ra handler thường chạy với IRQ (dòng đó) bị mask — ngủ lâu là mất ngắt, deadlock với chính lock mà process bên dưới đang giữ.
- Hệ quả: trong handler cấm mutex (ngủ được), cấm kmalloc(GFP_KERNEL) (có thể ngủ), cấm copy_to_user... — chỉ spinlock, GFP_ATOMIC, việc thật ngắn.
- **Top half / bottom half:** chia đôi công việc — top (hardclick): ack thiết bị, gom dữ liệu tối thiểu, lên lịch phần còn lại; bottom (chạy sau, ưu tiên thấp hơn, ngắt đã mở): xử lý nặng — cơ chế: softirq/tasklet (vẫn atomic context) hoặc **workqueue** (process context — ngủ thoải mái).
- **Threaded IRQ** (`request_threaded_irq`) = chuẩn hóa mô hình đó: handler cứng tối thiểu (hoặc NULL + IRQF_ONESHOT), phần chính chạy trong **kernel thread riêng cho IRQ đó** — được ngủ, mutex được, ưu tiên chỉnh được (RT!); PREEMPT_RT ép hầu hết IRQ thành threaded — đường thẳng tới câu hỏi realtime ([debug-realtime.md](debug-realtime.md)).

</details>

**Câu 2:** Bạn cần đọc một sensor I2C mỗi 100ms và một encoder cần đếm xung latency thấp. Kernel driver hay userspace cho từng cái? Vì sao?

<details><summary>Đáp án</summary>

- **Sensor I2C 100ms → userspace** (`i2c-dev` + timer): chu kỳ chậm, không ngắt gấp, sai một chuyến đọc không chết ai; code userspace dễ viết/debug/update, crash không kéo kernel. Chỉ xuống kernel (iio driver) khi cần: nhiều consumer chuẩn hóa, trigger/buffer phần cứng, hoặc sensor thuộc chuỗi thời gian chính xác.
- **Encoder đếm xung latency thấp → kernel** (hoặc thấp hơn nữa): xung có thể hàng kHz — mỗi xung một round-trip userspace (wakeup + syscall) là vừa trễ vừa đốt CPU và **mất xung** khi hệ bận. Đúng bài: ngắt trong kernel driver đếm (top half tăng counter, đọc theo lô), tốt hơn nữa là **offload phần cứng**: timer/counter peripheral của SoC (eQEP, counter subsystem) đếm hộ — CPU chỉ đọc thanh ghi định kỳ, đó là câu trả lời senior (đẩy việc realtime xuống silicon khi có thể).
- Khung chấm điểm nằm ở **tiêu chí**: tần suất sự kiện × yêu cầu latency × hậu quả khi trễ — không phải "kernel xịn hơn userspace".

</details>

---

## Cụm 2 — Init: PID 1 và các dòng họ (ch. 13–14)

### Nội dung chính

Ba lựa chọn init và trade-off (câu hỏi chọn-và-biện-luận):

| | BusyBox init | System V | systemd |
|---|---|---|---|
| Cơ chế | `/etc/inittab` + script `rcS` | runlevel + script `/etc/init.d/` + symlink S/K | unit file khai báo, dependency graph, song song |
| Size/phụ thuộc | Vài chục KB | Nhỏ | Lớn (nhiều MB, kéo dbus...) |
| Giám sát service | respawn thô sơ | Không (script chạy rồi thôi) | **Có**: restart, watchdog, cgroup từng service |
| Boot song song/on-demand | Không | Không | Có (socket activation) |
| Hợp với | Thiết bị tối giản | Legacy | Thiết bị "giàu" (RAM ≥ 128MB), cần quản lý service nghiêm túc |

- systemd trên embedded — lý do *thật* để chọn: **giám sát + tự phục hồi service** (Restart=, WatchdogSec= nối với watchdog cứng), **phụ thuộc tường minh** (After/Requires — hết script race), cgroup cô lập tài nguyên từng service, journal. Giá: size, độ phức tạp, thời gian boot phải tune (`systemd-analyze blame/critical-chain`).
- Boot nhanh (nhu cầu thật: camera/automotive): đo từng chặng (bootloader → kernel → init → service), chiêu: falcon mode/SPL boot thẳng, kernel bỏ driver thừa + initcall_debug, rootfs RO, service khởi động lười/on-demand.

### Insight đáng nhớ

- PID 1 có hai đặc quyền/kèm nghĩa vụ: **nhận nuôi orphan + phải reap zombie** ([OSTEP](../ostep/virtualization-cpu.md)) và **không được chết** (panic). Mọi init đều chỉ là vòng lặp "đẻ service, chờ SIGCHLD, respawn theo chính sách" — hiểu vậy thì đọc inittab hay unit file đều ra một khung.

### Ít quan trọng

- runit (ch. 14) — đáng biết tồn tại như điểm giữa BusyBox/systemd (supervise per-service, nhẹ); chi tiết cú pháp unit systemd đầy đủ — tra `man systemd.service` khi viết.

### Góc interview

**Câu 1:** Chọn init system cho: (a) camera IP 64MB RAM, boot 2s; (b) gateway công nghiệp 512MB RAM chạy 15 service. Biện luận.

<details><summary>Đáp án</summary>

- **(a) BusyBox init** (hoặc thậm chí app-as-PID1): 64MB RAM + mục tiêu 2s không có chỗ cho systemd (bản thân nó chiếm đáng kể RAM/flash và thời gian khởi tạo); inittab + một script rcS tuyến tính, service ít, respawn thô sơ là đủ; boot tối ưu từng ms (SPL falcon, kernel tối giản). Trade-off chấp nhận: không có dependency manager — tự kỷ luật trong script.
- **(b) systemd**: 15 service là đủ đông để script tay thành nợ — cần dependency tường minh, **tự restart có chính sách + watchdog per-service**, cgroup chặn một service ăn cả RAM, journal tập trung để hậu kiểm ngoài hiện trường. 512MB thừa sức. Trade-off: kích thước + học phí unit; tune boot bằng `systemd-analyze`.
- Format ăn điểm: nêu **tiêu chí trước** (RAM/flash, số service, yêu cầu tự phục hồi, thời gian boot) rồi map — không tuyên bố "systemd tốt hơn/tệ hơn" vô điều kiện.

</details>

---

## Cụm 3 — Power Management (ch. 15)

### Nội dung chính

Bốn tầng công cụ PM của Linux — từ "chạy tiết kiệm" đến "ngủ":

1. **cpufreq** — chỉnh **tần số/điện áp khi đang chạy** (DVFS; OPP table trong DT): governor `ondemand/schedutil` (theo tải — schedutil hiện đại nhất, lấy tín hiệu thẳng từ scheduler), `performance`, `powersave`, `userspace`.
2. **cpuidle** — CPU **rảnh thì ngủ nông/sâu** (C-states): idle sâu tiết kiệm hơn nhưng **exit latency** cao hơn — trade-off với realtime (RT thường giới hạn độ sâu idle).
3. **Runtime PM** — **từng thiết bị** tự tắt khi không dùng (driver khai `runtime_suspend/resume`, đếm tham chiếu usage): hệ "thức" nhưng các khối im lặng — nguồn tiết kiệm lớn nhất của thiết bị always-on.
4. **System suspend** — cả hệ ngủ: freeze process → suspend device (theo thứ tự cây thiết bị) → CPU ngủ sâu; RAM tự refresh (**suspend-to-RAM**, `echo mem > /sys/power/state`). Dậy bằng **wakeup source** (khai trong DT/driver `wakeup-source`, RTC alarm, GPIO, mạng WoL).
- Đo mới quản được: `powertop`, đo dòng thật trên board (PM là mảng "số liệu hay nói dối cảm giác").

### Insight đáng nhớ

- Khung nhớ 4 tầng: **"chạy chậm lại (cpufreq) → rảnh thì chợp mắt (cpuidle) → bộ phận không dùng thì tắt (runtime PM) → cả hệ đi ngủ (suspend)"** — mọi câu hỏi PM rơi vào một trong bốn ngăn, và bài toán thiết kế là *phối* cả bốn theo power budget.
- PM là nơi **bug DT/driver lộ muộn nhất**: suspend/resume hỏng vì một driver không có resume handler, wakeup source quên khai, clock bị tắt chung cây — kỹ năng debug: `/sys/power/`, `pm_test` mode, so sánh trạng thái thanh ghi trước/sau resume.

### Ít quan trọng

- Hibernate (suspend-to-disk) — hiếm trên embedded; chi tiết OPP bindings và cpufreq stats (ch. 15 giữa) — tra khi tune thật.

### Góc interview

**Câu 1:** Thiết bị chạy pin luôn-bật, đo dòng thấy cao hơn thiết kế 3×. Trình tự truy tìm của bạn?

<details><summary>Đáp án</summary>

Đi từ tổng thể xuống từng khối, mỗi bước có số liệu:
1. **CPU có được nghỉ không**: `powertop`/`turbostat` tương đương — C-state residency (CPU có xuống idle sâu không hay bị **wakeup bão** — timer/IRQ nào réo liên tục: `powertop` tab wakeups, `/proc/interrupts` delta); tần số có xuống không (governor gì, `scaling_cur_freq`).
2. **Process nào không cho ngủ**: poll/timer dày (strace nhanh nghi phạm, powertop chỉ mặt), busy-loop lỗi, log spam ra flash.
3. **Thiết bị ngoại vi**: runtime PM có hoạt động (`/sys/devices/.../power/runtime_status` — khối nào `active` mãi), driver nào giữ clock/regulator (clk_summary trong debugfs, regulator_summary), PHY/USB/Wi-Fi không vào power save.
4. **Phần cứng/DT**: rail nào không tắt theo thiết kế (đo từng rail nếu board cho phép), pinmux chân nổi (floating gây rò), pull sai chiều.
5. Chốt bằng **ma trận đo**: dòng ở từng trạng thái (idle desktop, tắt từng subsystem một) — khoanh vùng bằng phép trừ.
- Trả lời có cấu trúc "CPU idle? → ai đánh thức? → khối nào còn thức? → phần cứng?" + gọi tên công cụ từng bước là mức kỹ sư PM thực chiến.

</details>

### Đọc thêm (tùy chọn)

- [05-drivers-device-tree/](../../05-drivers-device-tree/) — kernel/userspace boundary, driver model.
- [OSTEP concurrency](../ostep/concurrency.md) — lock trong kernel context (nền của câu ISR).
