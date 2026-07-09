# LKD — Interrupts & Bottom Halves (ch. 7 tr. 113, ch. 8 tr. 133) 🎯🎯

> Thuộc [LKD](README.md). Nguồn: kiến thức Claude, số trang đối chiếu PDF 3rd ed.

---

## Cụm 1 — Interrupt Handlers (ch. 7, tr. 113–131)

### Nội dung chính

**Vì sao có interrupt:** phần cứng chậm và bất định — polling phí CPU; interrupt cho phần cứng "gọi" CPU khi có việc (tr. 113). Interrupt ≠ exception: interrupt **bất đồng bộ** từ phần cứng; exception **đồng bộ** do CPU sinh khi thực thi lệnh (page fault, syscall trap) — cùng hạ tầng xử lý, khác nguồn.

**Đăng ký handler (tr. 116):**

```c
int request_irq(unsigned int irq,           // số IRQ (thời DT: platform_get_irq lấy từ node)
                irq_handler_t handler,
                unsigned long flags,        // IRQF_*
                const char *name,           // hiện trong /proc/interrupts
                void *dev);                 // cookie — phân biệt device khi SHARED, truyền lại cho handler
```

- Flags đáng nhớ (tr. 116): `IRQF_SHARED` — nhiều handler chung một line (bắt buộc mỗi handler tự hỏi thiết bị mình "có phải mày ngắt không?" rồi trả `IRQ_NONE`/`IRQ_HANDLED`); `IRQF_ONESHOT` — giữ mask line đến khi threaded handler xong. ⚠️ `IRQF_DISABLED` trong sách đã **bị xóa** (kernel hiện đại: handler luôn chạy với local interrupt disabled).
- Handler trả `irqreturn_t`: `IRQ_HANDLED` / `IRQ_NONE` (giúp kernel phát hiện **spurious/stuck IRQ** — line kêu mà không ai nhận, quá nhiều lần → kernel tắt line, log "nobody cared").
- `free_irq()` khi gỡ; bản hiện đại dùng `devm_request_irq` — tự free theo vòng đời device.

**Interrupt context (tr. 122)** — luật sắt (đã học ở [MELP](../melp/drivers-init-power.md)/[OSTEP](../ostep/concurrency.md), đây là nguồn gốc):
- Không có task_struct đứng sau (`current` vô nghĩa về ngữ nghĩa) → **không ngủ, không schedule** → cấm mutex/`GFP_KERNEL`/`copy_*_user`.
- Stack riêng và **nhỏ** (interrupt stack 1 page/CPU — tr. 123) → không đệ quy sâu, không mảng to trên stack.
- Phải **nhanh**: line đang mask (ít nhất line của mình), làm lâu = mất sự kiện, tăng latency toàn hệ.

**`/proc/interrupts` (tr. 126):** bảng đếm ngắt theo CPU/line/tên — công cụ chẩn đoán số một: IRQ storm (số nhảy điên), ngắt không đến (số đứng im — nghi DT/pinmux/mask), affinity lệch (dồn một CPU — chỉnh `/proc/irq/N/smp_affinity`).

**Điều khiển ngắt (tr. 127–130):** `local_irq_save/restore` (tắt trên **CPU hiện tại** — bảo vệ chống chính ISR trên CPU mình, dùng theo cặp lồng nhau an toàn), `disable_irq[_nosync]/enable_irq` (tắt **một line** mọi CPU — chờ handler đang chạy xong), phân biệt với masking trong controller. Tắt local interrupt **không** bảo vệ dữ liệu khỏi CPU khác — đó là việc của spinlock ([sync-timers.md](sync-timers.md)).

### Insight đáng nhớ

- Handler nên là "thư ký nhận điện": **ack thiết bị, chép tối thiểu, hẹn xử lý sau** — mọi thiết kế ISR tốt đều quy về câu này; phần "hẹn sau" chính là bottom half (cụm 2).
- Shared IRQ là di sản PCI line-based; thời MSI/MSI-X và GIC per-device thì hiếm dần — nhưng câu "handler shared phải kiểm tra thiết bị mình trước" vẫn là câu hỏi phân loại kiến thức nền.

### ⚠️ Đã thay đổi so với sách

- **Threaded IRQ (`request_threaded_irq`)** — xuất hiện đúng thời sách in nên sách gần như không nói; nay là **mặc định khuyến nghị** cho driver (phần nặng chạy trong kernel thread, ngủ được, priority chỉnh được; PREEMPT_RT ép hầu hết IRQ thành threaded). Đây là bổ sung quan trọng nhất phải tự đọc thêm ngoài sách — đã có chi tiết ở [melp/drivers-init-power.md](../melp/drivers-init-power.md).
- `IRQF_DISABLED` bị xóa (2.6.35+); generic IRQ chip framework + DT (`interrupts =` trong node) thay cho số IRQ hard-code.

### Góc interview

**Câu 1 (🎯):** Trong ISR bạn được làm gì và không được làm gì? Giải thích *từ gốc* thay vì liệt kê.

<details><summary>Đáp án</summary>

- Gốc: ISR chạy trong **interrupt context** — mượn CPU, không có task đứng sau → không thể schedule (ngủ = gọi scheduler nhường CPU và chờ wake; không có task để xếp hàng/đánh thức → `scheduling while atomic`); và line ngắt đang mask → kéo dài là mất sự kiện/tăng latency cả hệ.
- Suy ra cấm: mutex/semaphore (có thể ngủ), `kmalloc(GFP_KERNEL)` (có thể ngủ đợi reclaim), `copy_from/to_user` (có thể page fault → ngủ), msleep, mọi I/O đồng bộ. Được: spinlock (`spin_lock` — và phía process phải dùng `_irqsave` nếu chung lock, xem [sync-timers.md](sync-timers.md)), `GFP_ATOMIC`, đọc/ghi thanh ghi, wake_up, schedule bottom half.
- Stack nhỏ riêng → không mảng lớn cục bộ, không đệ quy.
- Chốt bằng thiết kế: ISR = ack + gom tối thiểu + defer (tasklet/workqueue/threaded IRQ) — trả lời có cấu trúc "gốc → hệ quả → thiết kế" thay vì học vẹt danh sách cấm.

</details>

**Câu 2:** `/proc/interrupts` cho bạn biết gì khi debug? Nêu 3 kịch bản đọc-ra-bệnh.

<details><summary>Đáp án</summary>

1. **Số đếm đứng im** dù thiết bị hoạt động → ngắt không tới CPU: sai số IRQ/trigger type trong DT, pinmux chân ngắt, thiết bị không được cấu hình phát ngắt, hoặc line bị kernel tắt vì spurious ("nobody cared" trong dmesg — handler nào đó trả IRQ_NONE mãi).
2. **Số đếm tăng điên** (IRQ storm) → mức trigger sai (level vs edge — level mà quên ack là ngắt lại ngay), chân nhiễu/floating, thiết bị lỗi; hệ quả CPU 100% trong ISR — cũng là lý do watchdog/RCU stall.
3. **Dồn hết một CPU** → affinity mặc định; hệ throughput cao cần rải (`smp_affinity`, irqbalance) hoặc hệ RT cần dồn *ra khỏi* core realtime — đọc kèm cột tên driver để biết ai là ai.
- Cộng thêm: so **delta theo thời gian** (`watch -d`) chứ không đọc số tuyệt đối; đối chiếu `/proc/softirqs` khi nghi tầng bottom half.

</details>

---

## Cụm 2 — Bottom Halves: softirq, tasklet, workqueue (ch. 8, tr. 133–159) 🎯

### Nội dung chính

**Vì sao chia đôi (tr. 134):** ISR phải nhanh nhưng việc thì nhiều → chẻ: **top half** chạy ngay (ack, gom), **bottom half** chạy "lát nữa, với ngắt mở". Lịch sử nhiều cơ chế (BH, task queues — tr. 135, đã chết), còn lại ba đường:

| (tr. 156 "Which Bottom Half Should I Use?") | **softirq** (tr. 137) | **tasklet** (tr. 142) | **workqueue** (tr. 149) |
|---|---|---|---|
| Context | Interrupt (atomic) | Interrupt (atomic) | **Process** (kernel thread) |
| Được ngủ? | ❌ | ❌ | ✅ — mutex, alloc, I/O thoải mái |
| Concurrency | **Cùng loại chạy song song trên nhiều CPU** → handler phải tự lock/per-CPU | Cùng một tasklet **không bao giờ chạy song song với chính nó** (khác tasklet thì có) | Như thread thường |
| Đăng ký | Tĩnh, compile-time, số lượng đếm trên đầu ngón tay (net RX/TX, block, timer, RCU, sched) | Động, dễ dùng — xây trên softirq | Động — `schedule_work`, queue riêng được |
| Dành cho | Hạ tầng hiệu năng cực cao (network, block) | Driver thường cần defer nhanh, không ngủ | Việc cần ngủ / lâu / không gấp |

- **softirq**: raise trong ISR → chạy tại điểm thoát interrupt; tràn việc → đẩy cho **ksoftirqd** (kernel thread per-CPU, nice thấp — tr. 146) để không chết đói userspace khi bão softirq (câu chuyện thiết kế hay: xử lý ngay thì userspace đói, để sau thì latency — ksoftirqd là thỏa hiệp "xử lý ngay *một ít*, quá thì xuống thread cạnh tranh công bằng").
- **tasklet** = softirq đóng gói dễ dùng (TASKLET_SOFTIRQ) — `tasklet_schedule(&t)`; bảo đảm không tự chạy song song → driver đỡ lock.
- **workqueue** = hàng việc do **kworker thread** chạy trong process context — `INIT_WORK`, `schedule_work(&w)` (queue chung), hoặc `alloc_workqueue` riêng; cần ngủ là chọn nó, hết tranh cãi (tr. 153–156).
- Locking giữa các tầng (tr. 157): dữ liệu chung ISR↔bottom half → bottom half phải `spin_lock_irqsave`... — bảng phối đầy đủ ở [sync-timers.md](sync-timers.md); `local_bh_disable/enable` khi process context cần chặn bottom half.

### Insight đáng nhớ

- Cây quyết định 10 giây: **cần ngủ? → workqueue (hoặc threaded IRQ). Không ngủ mà cần cực nhanh + tự lo lock? → softirq (bạn gần như không bao giờ viết mới). Còn lại → tasklet** (nhưng xem ⚠️ dưới). Trả lời phỏng vấn đúng bằng cây này.
- ksoftirqd là bài học thiết kế tổng quát: **hệ bị dội việc phải có van xả xuống nền có kiểm soát** — cùng khuôn NAPI (interrupt→poll), kswapd (watermark), writeback thread.

### ⚠️ Đã thay đổi so với sách

- **Tasklet đã bị deprecated** (dần gỡ từ ~5.19, thay bằng threaded IRQ hoặc **BH workqueue** từ 6.9) — code mới **không viết tasklet nữa**; cây quyết định hiện đại: cần ngủ → workqueue/threaded IRQ; không ngủ → giữ trong hardirq nếu ngắn, hoặc BH workqueue.
- Workqueue đã thành **CMWQ** (concurrency-managed, 2.6.36+): pool worker chung per-CPU thay vì thread riêng mỗi queue như mô tả cuối sách — API `schedule_work` giữ nguyên, mô hình thread bên dưới khác.
- BH (Old Bottom Half) và task queue trong sách (tr. 135, 148, 155): đọc như khảo cổ.

### Góc interview

**Câu 1 (🎯):** Driver của bạn nhận gói dữ liệu qua ngắt, cần parse (nhanh, không ngủ) rồi ghi log ra file (chậm, ngủ). Bố trí các tầng thế nào?

<details><summary>Đáp án</summary>

- **ISR (top half):** ack thiết bị, kéo dữ liệu khỏi FIFO phần cứng vào ring buffer (hoặc kick DMA), schedule tầng dưới. Vài µs.
- **Parse — không ngủ, cần sớm:** threaded IRQ handler (khuyến nghị hiện đại) hoặc tasklet (code cũ): parse từ ring buffer, cập nhật state (lock bằng spinlock với ISR — `spin_lock_irqsave` nếu ISR cũng đụng), đẩy kết quả cần log vào hàng đợi thứ hai.
- **Ghi log — ngủ được:** `schedule_work()` — worker thread mở file/ghi/fsync thoải mái trong process context. Tuyệt đối không ghi file từ tasklet/softirq (VFS có thể ngủ).
- Điểm chấm: đúng ba tầng theo "được ngủ hay không + độ gấp"; nói thêm backpressure (ring đầy thì drop có đếm — không block ISR) và rằng nếu parse cũng nhẹ thì gộp parse vào threaded handler, đừng chế nhiều tầng vô ích.

</details>

**Câu 2:** softirq khác tasklet khác workqueue về concurrency thế nào — và hệ quả locking của từng loại?

<details><summary>Đáp án</summary>

- **softirq**: cùng một softirq **chạy đồng thời trên nhiều CPU** → handler phải thiết kế reentrant: dữ liệu per-CPU hoặc spinlock tự quản — lý do chỉ network/block đủ đô dùng trực tiếp (đổi lấy scale tối đa).
- **tasklet**: kernel bảo đảm **một tasklet không tự chạy song song** (serialized với chính nó; tasklet khác vẫn song song) → dữ liệu chỉ tasklet đó đụng thì khỏi lock; vẫn phải lock với ISR (irqsave) và với process context.
- **workqueue**: process context, work item có thể chạy song song trên worker khác nhau (CMWQ) trừ khi ordered workqueue → lock như thread thường (mutex được).
- Bảng "ai chen được ai" đầy đủ (kèm spin_lock_bh, local_bh_disable) ở [sync-timers.md](sync-timers.md). Nêu ⚠️ tasklet deprecated là điểm cộng thời sự.

</details>

### Đọc thêm (tùy chọn)

- [melp/drivers-init-power.md](../melp/drivers-init-power.md) — threaded IRQ góc dùng-hằng-ngày; [ostep/concurrency.md](../ostep/concurrency.md) — nền lý thuyết.
- [05/driver-basics.md](../../05-drivers-device-tree/driver-basics.md) — vị trí ISR trong khung driver.
