# Ch. 7–8 — Interrupts & Bottom Halves (tr. 113–159)

> Thuộc [LKD](README.md) · **[⏮ 01 Process/Sched/Syscalls](01-process-sched-syscalls.md)** · **[03 Sync & Timers → ⏭](03-sync-timers.md)**
> Nguồn: **đọc trực tiếp PDF** (Linux Kernel Development, Robert Love, 3rd ed. 2010), **trang sách = trang PDF − 27**.
> Ký hiệu: không đánh dấu = nội dung sách · **🆕 = bổ sung/liên hệ ngoài sách** · **⚠️ = cần cẩn trọng / sách lỗi thời** · **🎯 = câu hỏi phỏng vấn kinh điển** · trích dẫn kèm `(tr. X)`.
> Hai chương này là **trái tim của giao tiếp phần cứng ↔ kernel**. Chương 7: phần cứng "gọi" CPU bằng interrupt, và **top half** (ISR) xử lý ngay. Chương 8: phần việc còn lại được **hoãn** sang **bottom half** (softirq/tasklet/workqueue) để ISR chạy thật nhanh. Ví dụ xương sống xuyên suốt: **card mạng nhận gói** — đọc một lần sẽ thấy tại sao phải chẻ đôi.

---

## Cụm 1 — Interrupt Handlers: top half (ch. 7, tr. 113–131)

### 1.1 Vì sao có interrupt, và interrupt đi qua đâu (tr. 113–114)

**Bức tranh trước:** một nhiệm vụ cốt lõi của kernel là quản lý phần cứng (đĩa, bàn phím, radio...). Nhưng **CPU nhanh hơn phần cứng hàng bậc độ lớn** → nếu kernel ra lệnh rồi *đứng chờ* thiết bị chậm trả lời thì phí CPU khủng khiếp. Có hai cách để CPU không phải chờ:

- **Polling:** kernel **định kỳ hỏi** trạng thái thiết bị. Nhược điểm (tr. 113): tốn overhead vì **hỏi lặp đi lặp lại bất kể thiết bị có sẵn sàng hay không**.
- **Interrupt (giải pháp tốt hơn):** cho phần cứng một cơ chế **tự báo cho kernel khi cần chú ý**. *"This mechanism is called an interrupt."*

**Interrupt đi qua đâu (tr. 113–114):** thiết bị phát **tín hiệu điện** vào chân của một **interrupt controller** — con chip *"multiplexes multiple interrupt lines into a single line to the processor"* (gom nhiều đường ngắt vào một đường tới CPU). Controller báo CPU; CPU **ngắt việc đang làm** để xử lý. Điểm cốt lõi (tr. 113): interrupt sinh ra **bất đồng bộ với clock của CPU** — *"they can occur at any time"* → kernel có thể bị chen **bất cứ lúc nào**.

**IRQ line (tr. 114):** mỗi interrupt gắn một **giá trị số** — gọi là **interrupt request (IRQ) line** — để kernel biết thiết bị nào ngắt. Trên PC cổ điển: IRQ 0 = timer, IRQ 1 = keyboard (cố định); còn thiết bị PCI thì IRQ **cấp động**. Ý bất biến: *"a specific interrupt is associated with a specific device, and the kernel knows this."*

> **Interrupt vs Exception (sidebar tr. 114) — phải phân biệt:** exception xảy ra **đồng bộ** với clock CPU, do **chính CPU sinh ra khi thực thi lệnh** (chia cho 0, page fault) → còn gọi là *synchronous interrupt*. Interrupt thì **bất đồng bộ**, do **phần cứng** phát. Nhưng **cùng hạ tầng xử lý** — và bạn đã gặp một exception ở [01-process-sched-syscalls.md](01-process-sched-syscalls.md): **system call = software interrupt** trap vào kernel. Interrupt hoạt động tương tự, chỉ khác là *phần cứng* (không phải phần mềm) phát ra.

### 1.2 Interrupt handler (ISR) & interrupt context (tr. 114–115, 122)

**Interrupt handler** (hay **interrupt service routine — ISR**) là hàm kernel chạy để đáp một interrupt cụ thể. Nó **thuộc về driver** của thiết bị. Trong Linux, ISR là **hàm C bình thường** khớp một prototype nhất định. Cái khác biệt (tr. 115):
- kernel gọi nó **để đáp interrupt** (không ai gọi trực tiếp);
- nó chạy trong một **context đặc biệt gọi là interrupt context** — *"occasionally called atomic context because... code executing in this context is unable to block."*

**Interrupt context — luật sắt (tr. 122):** đây là khái niệm sinh tử, đối lập với **process context** ([01](01-process-sched-syscalls.md) mục 1.4). Nguyên văn:

> *"Interrupt context... is not associated with a process. The current macro is not relevant (although it points to the interrupted process). Without a backing process, interrupt context cannot sleep—how would it ever reschedule?"* (tr. 122)

→ **Không có task đứng sau → không có gì để xếp hàng ngủ và đánh thức → CẤM ngủ.** Câu *"how would it ever reschedule?"* là cách nhớ gốc rễ nhất: ngủ = gọi scheduler nhường CPU và chờ được đánh thức; interrupt context không có "ai" để đánh thức. Suy ra cấm mọi thứ **có thể ngủ**: mutex/semaphore, `kmalloc(GFP_KERNEL)`, `copy_*_user`, `msleep`.

Và ISR phải **nhanh (tr. 115, 122):** vì nó *"has interrupted other code (possibly even another interrupt handler on a different line!)"*. Ít nhất, việc của ISR là **ack** với phần cứng: *"Hey, hardware, I hear ya; now get back to work!"*

**Interrupt stack (tr. 122–123):** trước kia ISR **dùng chung kernel stack** của process bị ngắt (2 trang = 8KB trên 32-bit, 16KB trên 64-bit) → phải cực dè sẻn stack. Từ 2.6, có tùy chọn giảm stack xuống **1 trang (4KB)** và cho ISR **stack riêng, một stack mỗi CPU, một trang** — gọi là **interrupt stack**. Lời khuyên (tr. 123): *"Always use an absolute minimum amount of stack space."* → 🆕 không mảng lớn cục bộ, không đệ quy sâu trong ISR.

### 1.3 Top half vs bottom half — vì sao phải chẻ đôi (tr. 115)

Đây là **ý trung tâm** nối hai chương. Hai mục tiêu của ISR **mâu thuẫn nhau**: *chạy nhanh* và *làm nhiều việc*. Giải pháp: chẻ xử lý interrupt làm **hai nửa**:
- **Top half** = chính ISR — chạy **ngay** khi nhận interrupt, chỉ làm việc **time-critical** (ack interrupt, reset phần cứng).
- **Bottom half** = phần **hoãn lại**, chạy **sau, với mọi interrupt bật lại** (chương 8).

**Ví dụ xương sống — card mạng (tr. 115):** card nhận gói từ mạng → phát interrupt ngay ("Hey, kernel, tôi có gói mới!"). ISR:
1. ack phần cứng;
2. **copy gói từ buffer card vào main memory** — phải nhanh vì *"the network data buffer on the networking card is fixed and miniscule in size"*; chậm là **buffer overrun → mất gói**;
3. chuẩn bị card nhận gói tiếp.

Xong ba việc time-critical đó, ISR trả CPU về code bị ngắt. **Phần xử lý gói (đẩy lên protocol stack) làm sau, ở bottom half.** 🆕 Nhớ hình ảnh này là nhớ trọn nguyên tắc: *ISR = thư ký nhận điện, chép tối thiểu, hẹn xử lý sau*.

### 1.4 Đăng ký handler: `request_irq` (tr. 116–118)

Driver đăng ký ISR + bật một IRQ line bằng `request_irq()` (khai trong `<linux/interrupt.h>`):
```c
/* request_irq: cấp phát một interrupt line */
int request_irq(unsigned int irq,            // số IRQ (thời DT: probe/lấy động, không hard-code)
                irq_handler_t handler,       // con trỏ hàm — ISR thật sự
                unsigned long flags,         // bitmask IRQF_*
                const char *name,            // tên hiện trong /proc/interrupts (vd "keyboard")
                void *dev);                  // cookie: phân biệt device khi SHARED; truyền lại cho handler
```

**Flags đáng nhớ (tr. 116–117):**
- `IRQF_SHARED` — **line dùng chung nhiều handler**. Mọi handler trên line phải cùng đặt cờ này; nếu không, chỉ một handler tồn tại trên line.
- `IRQF_SAMPLE_RANDOM` — timing của interrupt góp vào **entropy pool** (nguồn số ngẫu nhiên). ⚠️ Đừng đặt cho thiết bị ngắt đều đặn (timer) hay bị attacker chi phối (mạng).
- `IRQF_TIMER` — handler cho system timer.
- ⚠️ `IRQF_DISABLED` — chạy handler với **mọi interrupt tắt**. Sách nói *"disabling all interrupts is bad form"*; đã **bị xóa** ở kernel hiện đại (handler nay luôn chạy với local interrupt disabled).

**`dev` — cookie cho line dùng chung (tr. 116):** khi free handler, `dev` là *"unique cookie to enable the removal of only the desired interrupt handler"*. Line không chung thì truyền `NULL`; line chung thì **bắt buộc cookie duy nhất** (thường truyền con trỏ `device` struct — vừa unique vừa hữu ích trong handler).

**Trả về (tr. 117):** 0 = thành công; khác 0 = lỗi (handler **không** được đăng ký). `-EBUSY` = line đang bận (bạn hoặc người kia quên `IRQF_SHARED`).

**⚠️ Điểm hay quên nhất — `request_irq()` CÓ THỂ NGỦ (tr. 117):**
> *"Note that request_irq() can sleep and therefore cannot be called from interrupt context."*

Vì sao? Sách truy đến tận gốc: đăng ký → tạo entry trong `/proc/irq` → `proc_mkdir()` → `proc_create()` → **`kmalloc()`** → *"kmalloc() can sleep. So there you go!"* 🆕 Đây là mẫu suy luận điển hình của kernel: *"hàm này ngủ không?"* → phải lần theo chuỗi gọi tới tận cấp phát bộ nhớ.

### 1.5 Viết handler: prototype & giá trị trả về (tr. 118–119)

```c
static irqreturn_t intr_handler(int irq, void *dev);
```
- **`irq`** — số line; ít dùng, chủ yếu để in log (trước kernel 2.0, `irq` dùng để phân biệt nhiều device chung một handler — nay đã có `dev`).
- **`dev`** — chính cookie đã truyền vào `request_irq`; với line chung nó **phân biệt device nào**.
- **Trả `irqreturn_t`** (thực chất là `int`): **`IRQ_NONE`** = "interrupt này **không phải** của device tôi"; **`IRQ_HANDLED`** = "đúng device tôi, đã xử lý". (Hoặc `IRQ_RETVAL(val)`.)

Vì sao cần `IRQ_NONE`/`IRQ_HANDLED` (tr. 119)? Để kernel **phát hiện spurious interrupt** (line kêu mà không ai nhận): *"If all the interrupt handlers on a given interrupt line return IRQ_NONE, then the kernel can detect the problem"* → 🆕 quá nhiều lần → kernel tắt line, log `"nobody cared"`.

> **Reentrancy (sidebar tr. 119) — điểm phân loại:** *"Interrupt handlers in Linux need not be reentrant. When a given interrupt handler is executing, the corresponding interrupt line is masked out on all processors."* → Cùng một handler **không bao giờ tự chạy chồng** (nested) cho cùng line. Các line **khác** vẫn được phục vụ; chỉ line hiện tại bị mask. *"This greatly simplifies writing your interrupt handler."*

### 1.6 Shared handler & ví dụ ISR thật (tr. 119–122)

**Handler dùng chung (tr. 120)** phải: (1) đặt `IRQF_SHARED`; (2) `dev` duy nhất; (3) **tự biết device mình có ngắt hay không** — cần **status register** phần cứng để kiểm tra. Kernel gọi **lần lượt** từng handler đăng ký trên line; handler nào không phải device mình thì phải **thoát ngay**.

**ISR thật từ driver RTC (tr. 120–121)** — đọc để thấy một ISR "đời thực" trông ra sao:
```c
static irqreturn_t rtc_interrupt(int irq, void *dev)
{
        spin_lock(&rtc_lock);                      // (a) lock bảo vệ rtc_irq_data chống CPU khác

        rtc_irq_data += 0x100;
        rtc_irq_data &= ~0xff;
        rtc_irq_data |= (CMOS_READ(RTC_INTR_FLAGS) & 0xF0);   // đọc thanh ghi trạng thái

        if (rtc_status & RTC_TIMER_ON)
            mod_timer(&rtc_irq_timer, jiffies + HZ/rtc_freq + 2*HZ/100);   // hẹn timer (ch.11)

        spin_unlock(&rtc_lock);

        spin_lock(&rtc_task_lock);                 // (b) lock thứ hai bảo vệ callback
        if (rtc_callback)
                rtc_callback->func(rtc_callback->private_data);
        spin_unlock(&rtc_task_lock);

        wake_up_interruptible(&rtc_wait);          // đánh thức process đang chờ đọc RTC
        kill_fasync(&rtc_async_queue, SIGIO, POLL_IN);

        return IRQ_HANDLED;
}
```
🆕 Quan sát: dùng **`spin_lock`** (không `_irqsave`) — vì đây *đã ở trong* ISR, local interrupt vốn đã tắt; nhưng phía process đụng cùng `rtc_lock` sẽ phải dùng `spin_lock_irqsave` (đào ở [03-sync-timers.md](03-sync-timers.md)). Đọc thanh ghi, hẹn timer, đánh thức process chờ — rồi trả `IRQ_HANDLED`. Ngắn gọn, không ngủ.

### 1.7 Interrupt đi trong kernel: `do_IRQ` → `handle_IRQ_event` → `ret_from_intr` (tr. 122–126)

Đường đi (Figure 7.1, tr. 123): device → controller → CPU **dừng việc, tắt interrupt system, nhảy tới điểm vào định trước** → assembly `entry.S` lưu register + số IRQ trên stack → gọi **`do_IRQ()`** (từ đây là C):
```c
unsigned int do_IRQ(struct pt_regs regs);   // pt_regs = register đã lưu ở assembly
```
`do_IRQ()` (tr. 124): tính số line, **ack + mask line** (trên PC: `mask_and_ack_8259A()`), rồi gọi **`handle_IRQ_event()`** chạy chuỗi handler của line:
```c
irqreturn_t handle_IRQ_event(unsigned int irq, struct irqaction *action)
{
        irqreturn_t ret, retval = IRQ_NONE;
        ...
        if (!(action->flags & IRQF_DISABLED))
                local_irq_enable_in_hardirq();     // bật lại interrupt (trừ khi IRQF_DISABLED)
        do {
                ret = action->handler(irq, action->dev_id);   // gọi từng handler trên line
                switch (ret) {
                case IRQ_WAKE_THREAD:                  // 🆕 threaded IRQ — đánh thức kernel thread
                        ...
                        wake_up_process(action->thread);
                case IRQ_HANDLED:
                        status |= action->flags;
                        break;
                }
                retval |= ret;
                action = action->next;                 // sang handler kế (line chung)
        } while (action);
        ...
        local_irq_disable();                           // do_IRQ() mong interrupt vẫn tắt khi trả về
        return retval;
}
```
Xong, quay về assembly **`ret_from_intr()`** (tr. 126) — kiểm tra reschedule (nhắc lại từ [01](01-process-sched-syscalls.md) mục 2.7):
- interrupt ngắt **user process** → nếu `need_resched` set → gọi `schedule()`;
- interrupt ngắt **kernel** → gọi `schedule()` **chỉ khi `preempt_count == 0`** (không giữ lock). Ngược lại *"it is not safe to preempt the kernel."*

🆕 Đây chính là chỗ `preempt_count` từ chương 4 phát huy tác dụng: interrupt là một trong những **điểm kiểm tra** để kernel preemption xảy ra.

### 1.8 `/proc/interrupts` — công cụ chẩn đoán số một (tr. 126–127)

```
        CPU0
 0:     3602371      XT-PIC     timer
 1:     3048         XT-PIC     i8042
 4:     2689466      XT-PIC     uhci-hcd, eth0      ← line 4 DÙNG CHUNG: 2 device
 12:    85077        XT-PIC     uhci-hcd
```
Bốn cột (tr. 127): **line** | **số đếm interrupt (mỗi CPU một cột)** | **controller** (XT-PIC = PIC chuẩn PC; IO-APIC trên máy mới) | **tên device** (từ tham số `name` của `request_irq`). Line chung liệt kê **mọi** device (như line 4). 🆕 Đọc **delta theo thời gian** (`watch -d cat /proc/interrupts`), không đọc số tuyệt đối — xem interview Câu 2.

### 1.9 Điều khiển interrupt (tr. 127–130)

Lý do điều khiển interrupt = **đồng bộ** (tr. 127). Tắt interrupt đảm bảo handler **không chen** code hiện tại — và **cũng tắt kernel preemption**. Nhưng câu cảnh báo phải thuộc:

> *"Neither disabling interrupt delivery nor disabling kernel preemption provides any protection from concurrent access from another processor."* (tr. 127)

→ **Tắt local interrupt KHÔNG bảo vệ khỏi CPU khác** — đó là việc của **spinlock** ([03](03-sync-timers.md)). Thường phải dùng **cả hai**: lock (chống CPU khác) + tắt local interrupt (chống ISR trên CPU mình).

**Các interface (tr. 127–130):**
```c
local_irq_disable();   /* tắt interrupt trên CPU hiện tại */   // x86: lệnh cli
local_irq_enable();    /* bật lại */                            // x86: lệnh sti
```
⚠️ `local_irq_disable/enable` **nguy hiểm nếu interrupt vốn đã tắt trước đó** — `enable` bật vô điều kiện. Dùng cặp save/restore an toàn hơn (tr. 128):
```c
unsigned long flags;
local_irq_save(flags);      /* lưu trạng thái hiện tại RỒI tắt */
/* ... */
local_irq_restore(flags);   /* khôi phục về ĐÚNG trạng thái trước */
```
⚠️ `flags` phải ở **cùng stack frame** (không truyền sang hàm khác — vài arch như SPARC nhét thông tin stack vào đó) → **save và restore phải trong cùng một hàm**.

> **"No More Global cli()" (sidebar tr. 128):** xưa có `cli()` tắt interrupt trên **mọi** CPU (và CPU khác vào vùng `cli()` phải đợi). **Deprecated ở 2.5.** Nay mọi đồng bộ interrupt = **local interrupt control + spinlock**. Lợi ích: buộc driver dùng lock hạt mịn thật sự (nhanh hơn "một lock to toàn cục" là bản chất của `cli()`).

**Mask một line cụ thể cho cả hệ (tr. 129):**
```c
void disable_irq(unsigned int irq);         // mask line + CHỜ handler đang chạy xong
void disable_irq_nosync(unsigned int irq);  // mask line, KHÔNG chờ
void enable_irq(unsigned int irq);
void synchronize_irq(unsigned int irq);     // chỉ chờ handler xong (không mask)
```
Các call **nest** (gọi `disable_irq` 2 lần thì phải `enable_irq` 2 lần). ⚠️ **Bất lịch sự khi mask line dùng chung** (tắt luôn interrupt của mọi device trên line) → driver PCI hiện đại **không dùng**; chỉ còn ở driver legacy (parallel port).

**Kiểm tra trạng thái (tr. 130):** `irqs_disabled()` (local interrupt có đang tắt?), `in_interrupt()` (đang xử lý **bất kỳ** interrupt/bottom half nào?), `in_irq()` (đang trong **ISR** cụ thể?). Dùng `in_interrupt() == 0` để biết "đang ở process context → được ngủ".

### Insight đáng nhớ (cụm 1)

- **Câu *"how would it ever reschedule?"* (tr. 122)** là cách nhớ **gốc rễ** vì sao ISR không ngủ được — không có task đứng sau thì không có gì để đánh thức. Trả lời interview nên xuất phát từ đây, không học vẹt danh sách cấm.
- **Top/bottom half (tr. 115) = một trade-off**: line đang mask trong lúc ISR chạy → ISR càng lâu, sự kiện càng dễ mất và latency toàn hệ càng tăng → đẩy tối đa việc xuống bottom half. Ví dụ card mạng (copy nhanh vào RAM ở top half, xử lý gói ở bottom half) nén trọn nguyên tắc.
- **"Tắt interrupt KHÔNG chống được CPU khác" (tr. 127)** là ranh giới giữa interrupt control và locking — nhầm chỗ này là nguồn của cả một lớp bug SMP. Cầu nối thẳng sang [03](03-sync-timers.md): lock chống CPU khác, tắt local interrupt chống ISR cùng CPU.

### ⚠️ Đã thay đổi so với sách

- **Threaded IRQ (`request_threaded_irq`)** — sách chỉ chớm nhắc (`IRQ_WAKE_THREAD` trong `handle_IRQ_event`), nay là **khuyến nghị mặc định**: phần nặng chạy trong **kernel thread ngủ được, chỉnh priority được**; `PREEMPT_RT` ép hầu hết IRQ thành threaded. Đây là bổ sung quan trọng nhất phải tự đọc thêm — chi tiết ở [melp/drivers-init-power.md](../melp/drivers-init-power.md).
- **`devm_request_irq`** — bản "managed", **tự `free_irq` theo vòng đời device** (đỡ quên free lúc gỡ driver).
- `IRQF_DISABLED` **bị xóa** (2.6.35+); số IRQ hard-code thay bằng **Device Tree** (`interrupts = <...>` trong node) + generic IRQ chip framework; MSI/MSI-X (PCIe) và GIC per-device khiến **shared line** hiếm dần.

### Góc interview (cụm 1)

**Câu 1 (🎯):** Trong ISR bạn được làm gì và **không** được làm gì? Giải thích *từ gốc*, đừng liệt kê.

<details><summary>Đáp án</summary>

- **Gốc (tr. 122):** ISR chạy trong **interrupt context** — *"not associated with a process... cannot sleep—how would it ever reschedule?"* Không có task đứng sau → không thể ngủ (ngủ = nhường CPU và chờ được đánh thức, nhưng không có "ai" để xếp hàng/đánh thức → lỗi `scheduling while atomic`). Thêm nữa: line đang **mask** → kéo dài = mất sự kiện + tăng latency cả hệ.
- **Suy ra CẤM** (mọi thứ có thể ngủ): mutex/semaphore, `kmalloc(GFP_KERNEL)`, `copy_from/to_user` (page fault → ngủ — [01](01-process-sched-syscalls.md) mục 3.5), `msleep`, `request_irq` (→ `kmalloc`, tr. 117), mọi I/O đồng bộ.
- **ĐƯỢC:** `spin_lock` (và phía process phải `_irqsave` nếu chung lock — [03](03-sync-timers.md)), `GFP_ATOMIC`, đọc/ghi thanh ghi, `wake_up`, `mod_timer`, schedule bottom half. Stack nhỏ (tr. 123) → không mảng lớn cục bộ/đệ quy.
- **Chốt bằng thiết kế:** ISR = **ack + gom tối thiểu + defer** (tasklet/workqueue/threaded IRQ). Trả lời theo cấu trúc "gốc → hệ quả → thiết kế" ăn điểm hơn danh sách thuộc lòng.

</details>

**Câu 2:** `/proc/interrupts` cho bạn biết gì khi debug? Nêu 3 kịch bản đọc-ra-bệnh.

<details><summary>Đáp án</summary>

Bám cấu trúc bảng (tr. 126–127: line | số đếm/CPU | controller | tên device):
1. **Số đếm đứng im** dù device hoạt động → ngắt **không tới CPU**: sai số IRQ / trigger type trong DT, pinmux chân ngắt, device chưa bật phát ngắt, hoặc **line bị kernel tắt vì spurious** (log `"nobody cared"` trong dmesg — có handler trả `IRQ_NONE` mãi, cơ chế ở tr. 119).
2. **Số đếm tăng điên (IRQ storm)** → trigger sai (level mà quên ack → ngắt lại ngay), chân nhiễu/floating, device lỗi; hệ quả CPU 100% trong ISR → cũng là nguồn watchdog/RCU stall.
3. **Dồn hết một CPU** → affinity mặc định; hệ throughput cao cần rải ra (`/proc/irq/N/smp_affinity`, `irqbalance`), hệ RT cần dồn *ra khỏi* core realtime.
- **Điểm cộng:** đọc **delta** (`watch -d`) chứ không số tuyệt đối; đối chiếu cột tên driver để biết ai là ai; nghi bottom half thì xem thêm `/proc/softirqs` ([cụm 2](#cụm-2--bottom-halves-softirq-tasklet-workqueue-ch-8-tr-133159)).

</details>

---

## Cụm 2 — Bottom Halves: softirq, tasklet, workqueue (ch. 8, tr. 133–159)

### 2.1 Vì sao có bottom half & chia việc thế nào (tr. 133–135)

**Giới hạn của ISR (tr. 133)** — lý do bottom half tồn tại: ISR (1) chạy async, chen code khác kể cả ISR khác → phải nhanh; (2) chạy với **interrupt (một phần hoặc toàn bộ) bị tắt** → càng phải nhanh; (3) timing-critical; (4) **không ở process context → không block được**. Nên: việc gấp/liên quan phần cứng làm ở **top half**, phần còn lại **hoãn** sang **bottom half** — *"runs at a more convenient time, with all interrupts enabled."*

**Chia việc top/bottom (tips tr. 134):**
- việc **time-sensitive** → top half;
- việc **liên quan phần cứng** → top half;
- việc **không được để interrupt (nhất là cùng interrupt) chen** → top half;
- **mọi thứ còn lại** → bottom half.

Không có luật cứng; *"the decision is left entirely up to the device-driver author."*

**"Later is often simply not now" (tr. 135)** — câu cô đọng bản chất bottom half: điểm mấu chốt **không phải** chạy vào một thời điểm tương lai *cụ thể*, mà *"defer work until any point in the future when the system is less busy and interrupts are again enabled."* Thường bottom half chạy **ngay sau khi interrupt trả về**, nhưng **với mọi interrupt bật lại** — đó mới là điều quan trọng.

### 2.2 Ba cơ chế bottom half & lịch sử (tr. 135–137)

Qua lịch sử Linux có nhiều cơ chế (đặt tên rối kinh khủng, sách tự trào). Bảng 8.1 (tr. 137):

| Cơ chế | Trạng thái |
|---|---|
| **BH** (Bottom Half nguyên thủy) | Bỏ ở 2.5 — 32 cái tĩnh, **đồng bộ toàn cục** (không hai cái chạy song song, kể cả khác loại) → không scale |
| **Task queues** | Bỏ ở 2.5 |
| **softirq** | Có từ 2.3 |
| **tasklet** | Có từ 2.3 |
| **work queue** | Có từ 2.5 |

→ Kernel 2.6 còn **ba** cơ chế: **softirq, tasklet, work queue**. Tasklet xây trên softirq; work queue là subsystem riêng (xây trên kernel thread). 🆕 (Kernel timer — [03](03-sync-timers.md) — cũng là "hoãn việc", nhưng hoãn **đến sau một khoảng thời gian**, khác với "hoãn đến bất cứ lúc nào tiện".)

### 2.3 Softirq (tr. 137–141)

**Đặc tính (tr. 138):** softirq **cấp phát tĩnh lúc compile** (không đăng ký/hủy động), tối đa 32, hiện chỉ ~9. Đại diện bởi:
```c
struct softirq_action {
        void (*action)(struct softirq_action *);
};
static struct softirq_action softirq_vec[NR_SOFTIRQS];   // mảng cố định
```
**Concurrency — điểm phân loại quan trọng nhất (tr. 138):** *"A softirq never preempts another softirq. The only event that can preempt a softirq is an interrupt handler. Another softirq—even the same one—can run on another processor, however."* → **Cùng một softirq CÓ THỂ chạy song song trên nhiều CPU** → handler phải tự lo an toàn dữ liệu (per-CPU data hoặc lock).

**Thực thi (tr. 138–139):** softirq phải được **"raise"** (đánh dấu pending) — thường ISR raise trước khi thoát; rồi softirq chạy tại: (1) đường return từ hardware interrupt; (2) kernel thread **ksoftirqd**; (3) code kiểm tra tường minh (networking). Lõi xử lý trong `__do_softirq()`:
```c
u32 pending = local_softirq_pending();   // bitmask 32-bit: bit n set = softirq n pending
if (pending) {
    struct softirq_action *h = softirq_vec;
    set_softirq_pending(0);              // lưu mask xong thì XÓA mask thật (với interrupt đã tắt)
    do {
        if (pending & 1)
             h->action(h);               // bit thấp set → chạy handler tương ứng
        h++;
        pending >>= 1;                    // dịch phải, xét bit kế
    } while (pending);
}
```

**Priority (Bảng 8.2, tr. 140):** index enum = priority (số nhỏ chạy trước): `HI_SOFTIRQ`(0), `TIMER`(1), `NET_TX`(2), `NET_RX`(3), `BLOCK`(4), `TASKLET`(5), `SCHED`(6), `HRTIMER`(7), `RCU`(8).

**Dùng softirq (tr. 141):** đăng ký `open_softirq(NET_TX_SOFTIRQ, net_tx_action)`, raise bằng `raise_softirq(NET_TX_SOFTIRQ)`. Handler **chạy với interrupt bật, không ngủ được**. Câu quyết định (tr. 141): *"The raison d'être to softirqs is scalability. If you do not need to scale to infinitely many processors, then use a tasklet."* → **Chỉ networking + block device** dùng softirq trực tiếp; nếu softirq phải lấy lock để chống chính nó chạy song song thì *"there would be no reason to use a softirq"* — nên chúng dùng **per-CPU data** để tránh lock.

### 2.4 Tasklet (tr. 142–146)

**Bản chất (tr. 142):** tasklet **xây trên softirq** (`HI_SOFTIRQ`, `TASKLET_SOFTIRQ`), interface đơn giản hơn + luật lock nhẹ hơn. Sách khuyên thẳng: *"As a device driver author, the decision whether to use softirqs versus tasklets is simple: You almost always want to use tasklets."* (⚠️ Tên "tasklet" **chẳng liên quan gì task/process** — tr. 142.)

```c
struct tasklet_struct {
        struct tasklet_struct *next;
        unsigned long state;          // 0 | TASKLET_STATE_SCHED | TASKLET_STATE_RUN
        atomic_t count;               // count != 0 → tasklet bị disable
        void (*func)(unsigned long);  // handler
        unsigned long data;           // tham số truyền vào handler
};
```

**Concurrency — khác softirq ở đây (tr. 142):** *"two of the same type of tasklet cannot run simultaneously"* — **cùng một tasklet KHÔNG bao giờ tự chạy song song**; hai tasklet **khác** loại thì có thể song song trên hai CPU. → Đây là "softirq mà không chạy chồng chính nó" → dữ liệu chỉ tasklet đó đụng thì **khỏi lock**.

**Dùng (tr. 144–145):**
```c
DECLARE_TASKLET(my_tasklet, my_tasklet_handler, dev);   // tĩnh, count=0 (enabled)
DECLARE_TASKLET_DISABLED(name, func, data);             // tĩnh, count=1 (disabled)
tasklet_init(t, tasklet_handler, dev);                  // động
void tasklet_handler(unsigned long data);               // prototype — KHÔNG ngủ, interrupt bật
tasklet_schedule(&my_tasklet);                          // đánh dấu pending → chạy một lần sớm
```
Chi tiết đáng nhớ (tr. 145): tasklet **luôn chạy trên CPU đã schedule nó** (tận dụng cache); `tasklet_disable()` **chờ** tasklet đang chạy xong; `tasklet_kill()` gỡ khỏi hàng đợi (**ngủ** → cấm gọi từ interrupt context). Nếu tasklet chia sẻ dữ liệu với ISR → vẫn phải `spin_lock_irqsave`; với tasklet/softirq khác → phải lock.

### 2.5 ksoftirqd — van xả khi bị dội softirq (tr. 146–147)

**Vấn đề (tr. 146):** softirq có thể bị raise **tần suất cao** (bão network) và **tự raise lại chính mình** (softirq network tự kích hoạt lại). Hai giải pháp ngây thơ đều tệ:
- **Xử lý ngay + xử lý luôn cả cái vừa raise lại** → tải cao thì kernel *"continually service softirqs without accomplishing much else. User-space is neglected"* → **userspace đói**.
- **Không xử lý cái vừa raise lại** (đợi interrupt sau) → **softirq đói**, và trên hệ rảnh thì phí (đáng lẽ xử lý ngay được).

**Thỏa hiệp (tr. 147):** không xử lý ngay cái tự-raise-lại; nếu số softirq **quá nhiều** thì **đánh thức một họ kernel thread `ksoftirqd/n`** (mỗi CPU một cái, **nice 19** — priority thấp nhất) để gánh. Nhờ đó: bão softirq **không bỏ đói userspace** (vì ksoftirqd nice thấp, nhường process quan trọng), mà softirq "thừa" **vẫn được chạy** eventually; hệ rảnh thì ksoftirqd schedule ngay nên softirq vẫn nhanh. Vòng lặp của nó:
```c
for (;;) {
        if (!softirq_pending(cpu))
                schedule();                       // không có việc → nhường CPU
        set_current_state(TASK_RUNNING);
        while (softirq_pending(cpu)) {
                do_softirq();                     // xử lý, kể cả cái tự raise lại
                if (need_resched())
                    schedule();                   // nhường cho process quan trọng hơn
        }
        set_current_state(TASK_INTERRUPTIBLE);
}
```
🆕 **ksoftirqd là bài học thiết kế tổng quát**: *hệ bị dội việc phải có van xả xuống nền có kiểm soát* — cùng khuôn với NAPI (interrupt→poll khi bão gói), kswapd (reclaim theo watermark), writeback thread ([05](05-vfs-block-pagecache.md)).

### 2.6 Work queue — bottom half chạy được ở process context (tr. 149–155)

**Khác biệt cốt lõi (tr. 149):** work queue hoãn việc vào **kernel thread** → **luôn chạy ở process context** → *"work queues are schedulable and can therefore sleep."* Đây là **cơ chế bottom half duy nhất ngủ được** → dùng khi cần cấp nhiều bộ nhớ, lấy semaphore, làm block I/O. Quy tắc chọn (tr. 149): *"If the deferred work needs to sleep, work queues are used. If the deferred work need not sleep, softirqs or tasklets are used."*

**worker thread (tr. 149–150):** kernel dựng sẵn worker thread mặc định **`events/n`** (mỗi CPU một cái) chạy hàm `worker_thread()`; hầu hết driver đẩy việc cho `events`. Cấu trúc: `workqueue_struct` (một per *loại* worker) → `cpu_workqueue_struct` (một per CPU) → `work_struct` (một per *việc*). Lõi `worker_thread()`:
```c
for (;;) {
        prepare_to_wait(&cwq->more_work, &wait, TASK_INTERRUPTIBLE);
        if (list_empty(&cwq->worklist))
                schedule();                       // không có việc → ngủ
        finish_wait(&cwq->more_work, &wait);
        run_workqueue(cwq);                       // có việc → chạy hết
}
```
🆕 Chú ý mẫu **`prepare_to_wait`/`schedule`/`finish_wait`** — chính là mẫu wait-queue chống lost-wakeup ở [01](01-process-sched-syscalls.md) mục 2.6.

**Dùng (tr. 153–154):**
```c
DECLARE_WORK(name, func, data);                   // tĩnh
INIT_WORK(work, func, data);                       // động
void work_handler(void *data);                     // chạy process context, interrupt bật, ngủ được
schedule_work(&work);                              // đẩy vào events queue, chạy sớm
schedule_delayed_work(&work, delay);               // chạy sau 'delay' tick
flush_scheduled_work();                            // CHỜ mọi việc trong queue xong (ngủ → process ctx)
```
**⚠️ Cảnh báo tinh tế (tr. 153):** dù chạy ở process context, work handler **KHÔNG truy cập được bộ nhớ user** — *"there is no associated user-space memory map for kernel threads"* (nhớ kernel thread `mm == NULL`, [01](01-process-sched-syscalls.md) mục 1.5). Kernel chỉ chạm bộ nhớ user khi chạy **thay mặt process** (trong syscall). Cần queue riêng thì `create_workqueue("name")` + `queue_work(wq, work)`.

### 2.7 Chọn cơ chế nào & locking giữa các bottom half (tr. 156–158)

**Bảng so sánh (Bảng 8.3, tr. 156):**

| Bottom half | Context | Serialization sẵn có |
|---|---|---|
| **softirq** | Interrupt (atomic) | **Không** — cùng loại chạy song song nhiều CPU |
| **tasklet** | Interrupt (atomic) | **Với chính nó** — cùng tasklet không tự chạy song song |
| **work queue** | **Process** | Không (nhưng ở process context nên lock như code thường) |

**Cây quyết định (tr. 157):** *"do you need to sleep for any reason? Then work queues are your only option. Otherwise, tasklets are preferred. Only if scalability becomes a concern do you investigate softirqs."* Work queue overhead cao nhất (kernel thread + context switch) nhưng đủ cho hầu hết; softirq nhanh nhất nhưng phải tự lo lock → dành cho hạ tầng threaded sẵn (network/block).

**Locking giữa bottom half (tr. 157):**
- **tasklet** serialized với chính nó → khỏi lo *intra*-tasklet; nhưng *inter*-tasklet (hai tasklet chung dữ liệu) → phải lock.
- **softirq** không serialization → **mọi** dữ liệu chung cần lock.
- **process context + bottom half** chung dữ liệu → **disable bottom half + lock** (`spin_lock_bh`).
- **interrupt context + bottom half** chung dữ liệu → **disable interrupt + lock** (`spin_lock_irqsave`).
- **work queue** → lock như code process context bình thường. (Bảng phối đầy đủ ở [03-sync-timers.md](03-sync-timers.md).)

**Disable bottom half (tr. 157–158):** `local_bh_disable()` / `local_bh_enable()` (tắt/bật softirq + tasklet trên CPU hiện tại; nest được). Chi tiết đáng nhớ:
```c
void local_bh_disable(void) {
        current_thread_info()->preempt_count += SOFTIRQ_OFFSET;   // ← DÙNG CHÍNH preempt_count!
}
```
> *"a single per-task counter represents the atomicity of a task. This has proven useful for work such as debugging sleeping-while-atomic bugs."* (chú thích tr. 158)

🆕 Đây là mảnh ghép đẹp nối cả ba chương: **`preempt_count`** — cùng bộ đếm kernel preemption ([01](01-process-sched-syscalls.md) mục 2.7) — cũng đếm "đang trong bottom half". Một số nguyên duy nhất biểu diễn **"task này có đang atomic không?"** → là nền để `CONFIG_DEBUG_ATOMIC_SLEEP` bắt lỗi "ngủ khi atomic".

### Insight đáng nhớ (cụm 2)

- **Ba cơ chế = hai câu hỏi:** *"cần ngủ không?"* (có → work queue) và *"cần scale cực cao + tự lo lock?"* (có → softirq; còn lại → tasklet). Trả lời interview đúng bằng đúng cây này (tr. 157).
- **Concurrency quyết định lock:** softirq (cùng loại song song → luôn lock/per-CPU) vs tasklet (không tự chạy chồng → khỏi lock intra) vs work queue (process context → lock thường). Đây là "linh hồn" của việc chọn cơ chế, không phải cú pháp.
- **ksoftirqd (tr. 147)** dạy nguyên tắc *van xả có kiểm soát khi bị dội việc* — nhạ priority (nice 19) để không bỏ đói userspace nhưng vẫn xử lý hết. Nhận ra cùng khuôn ở NAPI/kswapd/writeback là dấu hiệu hiểu kiến trúc.
- **`preempt_count` (tr. 158)** là sợi chỉ xuyên suốt: nó đếm lock + kernel-preempt-disable + bottom-half-disable → **một số nguyên = "atomicity của task"**. Nắm được nó là nắm cầu nối process/interrupt/bottom-half context.

### ⚠️ Đã thay đổi so với sách

- **Tasklet đã deprecated** (gỡ dần từ ~5.19, thay bằng **threaded IRQ** hoặc **BH workqueue** từ 6.9) → **code mới không viết tasklet nữa**. Cây quyết định hiện đại: cần ngủ → workqueue/threaded IRQ; không ngủ → giữ trong hardirq nếu ngắn, hoặc BH workqueue.
- **Work queue đã thành CMWQ** (Concurrency Managed WorkQueue, 2.6.36+): **pool worker chung per-CPU** thay vì "một thread riêng mỗi queue" như sách mô tả; API `schedule_work`/`INIT_WORK` giữ nguyên nhưng mô hình thread bên dưới khác. Có `alloc_workqueue()` với cờ (`WQ_UNBOUND`, `WQ_HIGHPRI`, ordered...) thay `create_workqueue`.
- **BH + task queue** (tr. 148, 155) chỉ còn giá trị khảo cổ. Số softirq hiện đại nhiều hơn 9 một chút (thêm `IRQ_POLL`, `TASKLET_HI`...).

### Góc interview (cụm 2)

**Câu 1 (🎯):** Driver nhận gói qua ngắt, cần **parse** (nhanh, không ngủ) rồi **ghi log ra file** (chậm, ngủ). Bố trí các tầng thế nào?

<details><summary>Đáp án</summary>

Ba tầng theo trục "được ngủ hay không + độ gấp" (tr. 134, 149, 157):
- **ISR (top half):** ack thiết bị, kéo dữ liệu khỏi FIFO/buffer card vào ring buffer (hoặc kick DMA), raise bottom half. Vài µs — đúng ví dụ card mạng (tr. 115).
- **Parse — không ngủ, cần sớm:** **threaded IRQ handler** (khuyến nghị hiện đại) hoặc **tasklet** (code cũ): parse từ ring buffer, cập nhật state (lock với ISR bằng `spin_lock_irqsave` nếu ISR cũng đụng — tr. 157), đẩy kết quả cần log vào hàng đợi thứ hai. Không dùng softirq trừ khi cần scale cực cao và tự lo per-CPU/lock (tr. 141).
- **Ghi log — ngủ được:** **`schedule_work()`** → worker thread mở file/ghi/`fsync` thoải mái ở process context (tr. 149). **Tuyệt đối không ghi file từ tasklet/softirq** (VFS có thể ngủ).
- **Điểm chấm:** đúng ba tầng; nêu **backpressure** (ring đầy → drop có đếm, không block ISR); và nếu parse cũng nhẹ thì gộp vào threaded handler, đừng chế nhiều tầng vô ích. ⚠️ Nêu tasklet đã deprecated là điểm cộng thời sự.

</details>

**Câu 2 (🎯):** softirq, tasklet, work queue khác nhau về **concurrency** thế nào — và hệ quả **locking** của từng loại?

<details><summary>Đáp án</summary>

Bám tr. 138, 142, 156–157:
- **softirq:** *"even two of the same type can run concurrently"* → **cùng loại chạy song song trên nhiều CPU** → handler phải reentrant: **per-CPU data hoặc spinlock tự quản** cho mọi dữ liệu chung. Lý do chỉ network/block đủ đô dùng trực tiếp (đổi lấy scale tối đa). softirq chỉ bị **interrupt handler** preempt, không bị softirq khác preempt.
- **tasklet:** kernel bảo đảm **cùng một tasklet không tự chạy song song** (serialized với chính nó); tasklet **khác** vẫn song song. → dữ liệu chỉ tasklet đó đụng thì **khỏi lock**; vẫn phải lock với ISR (`_irqsave`) và với process context.
- **work queue:** chạy **process context**, work item có thể song song trên worker khác nhau (CMWQ) trừ **ordered workqueue** → lock như code thường (**mutex được** vì ngủ được).
- **Phối context (tr. 157):** process ↔ bottom half chung dữ liệu → `spin_lock_bh` (disable BH + lock); interrupt ↔ bottom half → `spin_lock_irqsave`. Bảng đầy đủ ở [03-sync-timers.md](03-sync-timers.md). Nêu ⚠️ tasklet deprecated + CMWQ là điểm cộng.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [01-process-sched-syscalls.md](01-process-sched-syscalls.md) — process/interrupt context, `preempt_count`, `need_resched`, mẫu wait-queue (nền của mọi luật ở đây).
- [03-sync-timers.md](03-sync-timers.md) — bảng phối lock đầy đủ (`spin_lock_irqsave`/`_bh`), và kernel timer (một dạng "hoãn việc theo thời gian").
- [melp/drivers-init-power.md](../melp/drivers-init-power.md) — threaded IRQ góc dùng-hằng-ngày; [05-drivers-device-tree/driver-basics.md](../../05-drivers-device-tree/driver-basics.md) — vị trí ISR trong khung driver.
- [ostep/concurrency.md](../ostep/concurrency.md) — nền lý thuyết đồng bộ (nếu muốn đào sâu phần lock ở cụm sau).

**Chương tiếp theo:** [03 — Kernel Synchronization & Timers →](03-sync-timers.md) (atomic, spinlock, mutex, seqlock, barriers, jiffies/timer — cách bảo vệ dữ liệu chung giữa mọi context vừa gặp).
