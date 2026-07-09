# LKD — Kernel Synchronization & Timers (ch. 9 tr. 161, ch. 10 tr. 175, ch. 11 tr. 207) 🎯🎯

> Thuộc [LKD](README.md). Nguồn: kiến thức Claude, số trang đối chiếu PDF 3rd ed.

---

## Cụm 1 — Vì sao kernel phải lock (ch. 9, tr. 161–172)

### Nội dung chính

**Nguồn concurrency trong kernel (tr. 167)** — nhiều hơn userspace: (1) SMP thật — nhiều CPU chạy code kernel đồng thời; (2) **kernel preemption** — task đang trong kernel bị task khác chen; (3) **interrupt/softirq** chen bất kỳ lúc nào; (4) ngủ trong kernel → task khác chạy vào cùng đường code. Code an toàn trước interrupt = *interrupt-safe*, trước SMP = *SMP-safe*, trước preemption = *preempt-safe*.

**Khóa cái gì?** — **"Lock DỮ LIỆU, đừng lock code"** (tr. 168): mọi biến chia sẻ được phải trả lời "ai bảo vệ mày?"; quy tắc thiết kế từ đầu, thêm sau là ác mộng. Deadlock, lock ordering, contention/scalability (tr. 169–171): 4 điều kiện + lock theo thứ tự toàn cục — trùng khung [OSTEP concurrency](../ostep/concurrency.md); riêng kernel: **lockdep** kiểm chứng ordering lúc runtime.

**Granularity:** lock to (một lock cả subsystem) dễ đúng nhưng nghẽn khi nhiều core; tách nhỏ (per-bucket, per-CPU) scale nhưng phức tạp — "bắt đầu to, tách khi đo thấy contention" (tr. 171).

### Góc interview

**Câu 1:** Vì sao code kernel phải lo đồng bộ nhiều hơn code userspace bình thường?

<details><summary>Đáp án</summary>

- Userspace single-thread: chỉ lo signal handler. Kernel thì **mặc định concurrent**: cùng một hàm kernel chạy đồng thời trên N CPU (SMP) — syscall từ nhiều process; kernel preemption chen giữa hai dòng lệnh; ISR/softirq chen bất cứ đâu (trừ khi disable); và ngủ (blocking) mở cửa cho task khác đi vào cùng dữ liệu.
- Hệ quả thiết kế: mọi dữ liệu chia sẻ phải có chủ lock được ghi chép; chọn *loại* lock theo **context nào cùng đụng dữ liệu** (bảng phối ở cụm 2 — đây là câu hỏi ăn tiền của kernel dev); công cụ: lockdep, `CONFIG_DEBUG_ATOMIC_SLEEP` bắt "ngủ khi cầm spinlock".

</details>

---

## Cụm 2 — Bộ công cụ đồng bộ của kernel (ch. 10, tr. 175–206) 🎯🎯

### Nội dung chính

**Atomic (tr. 175–182):** `atomic_t` / `atomic64_t` — `atomic_inc/dec/add_return/dec_and_test`; atomic bitops (`set_bit/test_and_set_bit`) trên vùng nhớ bất kỳ. Rẻ nhất — đủ cho counter/flag đơn.

**Spinlock (tr. 183–188)** — lock chờ-bằng-quay, dùng được trong **mọi context kể cả interrupt**; đoạn giữ lock **không được ngủ** (kèm: giữ spinlock là preemption bị tắt trên CPU đó). Bộ biến thể — **đây là bảng phải thuộc**:

```c
spin_lock(&l);                    // chỉ process↔process (không ISR/BH nào đụng dữ liệu)
spin_lock_irqsave(&l, flags);     // dữ liệu chung với ISR: tắt local IRQ + lock
                                  // (không tắt → ISR chen ĐÚNG LÚC đang giữ lock trên
                                  //  cùng CPU → ISR spin chờ lock mà chủ lock đứng im
                                  //  ngay dưới chân → DEADLOCK một CPU)
spin_lock_bh(&l);                 // dữ liệu chung với softirq/tasklet: tắt BH, khỏi tắt IRQ
```

Quy tắc chọn: **ai cùng đụng dữ liệu quyết định biến thể** — ISR đụng → `_irqsave`; chỉ bottom half đụng → `_bh`; không ai chen → bản trần. Reader-writer spinlock (tr. 188) cho đọc-nhiều-ghi-ít (⚠️ writer starvation; kernel hiện đại chuộng RCU hơn — xem ⚠️).

**Ngủ được — semaphore & mutex (tr. 190–197):** tranh chấp thì **ngủ** thay vì quay → chỉ dùng ở process context; giữ được qua đoạn ngủ. **`struct mutex`** là mặc định (ownership, debug tốt, tối ưu hơn); semaphore chỉ khi cần count/không-ownership. Bảng chọn của sách (tr. 197):

| Tình huống | Chọn |
|---|---|
| Critical section ngắn, có thể trong interrupt | spinlock |
| Cần ngủ trong lúc giữ lock (alloc GFP_KERNEL, copy_*_user, I/O) | **mutex** (bắt buộc) |
| Chờ dự kiến lâu | mutex (đỡ đốt CPU) |
| Trong ISR/tasklet | spinlock (mutex cấm) |

**Completion (tr. 197):** "chờ việc kia xong" — `wait_for_completion` / `complete` — semaphore-0 kiểu kernel, dùng cho init/teardown giữa hai luồng (đúng bài "chờ kernel thread khởi động xong").

**Seqlock (tr. 200):** writer tăng sequence khi ghi; reader đọc không lock, kiểm tra sequence trước/sau — lệch/lẻ thì đọc lại. Tuyệt cho **đọc rất nhiều, ghi hiếm, writer không được chờ** — chính là jiffies/gettimeofday.

**Preemption control (tr. 201):** `preempt_disable/enable` — chống bị chen trên CPU hiện tại (per-CPU data không cần lock nhưng cần chống dời CPU/preempt giữa chừng — `get_cpu/put_cpu`).

**Barriers (tr. 203–206):** compiler + CPU đều **reorder**; `barrier()` (chặn compiler), `smp_rmb/smp_wmb/smp_mb` (chặn CPU trên SMP) — cần khi giao tiếp lock-free giữa CPU (ring buffer tự chế: ghi data **rồi** wmb **rồi** ghi index; đọc index **rồi** rmb **rồi** đọc data). Trên ARM (weak ordering) thiếu barrier là bug thật chứ không lý thuyết — x86 che giấu vì ordering mạnh.

### Insight đáng nhớ

- **Toàn bộ chương gói trong một câu hỏi: "đoạn giữ lock có thể ngủ không, và context nào cùng đụng dữ liệu?"** — trả lời hai vế là ra đúng primitive + đúng biến thể. Đây cũng là format câu hỏi phỏng vấn kernel phổ biến nhất.
- BKL (Big Kernel Lock, tr. 198): đọc như bia mộ — bài học "một lock to cho cả kernel không scale"; **đã bị xóa hẳn (2.6.39)**.

### ⚠️ Đã thay đổi so với sách

- **RCU (read-copy-update)** — sách gần như không cover, nay là cơ chế đọc-nhiều thống trị kernel (đọc không lock + writer copy rồi đổi con trỏ + grace period trước khi free); phỏng vấn kernel senior hay chạm — biết trình bày ý tưởng 3 câu là điểm cộng lớn.
- BKL xóa (2.6.39); rwlock lép vế trước RCU; PREEMPT_RT biến spinlock thường thành sleeping lock ([melp/debug-realtime.md](../melp/debug-realtime.md)) — "spinlock không ngủ" đúng với kernel thường, có dấu sao với RT.
- Semaphore gần như chỉ còn legacy; `struct mutex` + completion phủ hầu hết nhu cầu.

### Góc interview

**Câu 1 (🎯):** Dữ liệu được cả syscall path lẫn ISR truy cập — dùng lock gì, và giải thích deadlock xảy ra nếu chọn sai.

<details><summary>Đáp án</summary>

- Chọn **`spin_lock_irqsave`** ở phía process context (syscall path), `spin_lock` thường trong chính ISR (vào ISR thì local IRQ vốn đã disabled).
- Deadlock nếu process path dùng `spin_lock` trần: process đang **giữ lock** trên CPU0 → đúng lúc đó ISR nổ **trên CPU0** → ISR `spin_lock` chờ lock → nhưng chủ lock là process đang đứng im *ngay bên dưới* ISR trên cùng CPU (ISR không nhường CPU bao giờ) → quay vĩnh viễn = **deadlock một CPU tự khóa mình**. `_irqsave` tắt local interrupt trong đoạn giữ lock → ISR không chen được trên CPU này; CPU khác thì ISR vẫn chạy nhưng spin chờ bình thường rồi lấy được lock — đúng nghĩa.
- Vì sao không mutex: ISR không ngủ được → mutex cấm trong ISR, hết lựa chọn.
- Điểm cộng: `_irqsave` vs `_irq` (save giữ nguyên trạng thái IRQ trước đó — an toàn khi lồng); dữ liệu chỉ chung với tasklet/softirq thì `spin_lock_bh` đủ (rẻ hơn tắt IRQ); và giữ critical section ngắn — đo bằng thời gian tắt IRQ (latency nguồn số một của RT).

</details>

**Câu 2 (🎯):** Vì sao "không được ngủ khi đang giữ spinlock"? Điều gì thật sự xảy ra nếu ngủ?

<details><summary>Đáp án</summary>

- Spinlock chờ bằng **quay** — kẻ chờ đốt CPU đến khi chủ nhả. Chủ **ngủ** (schedule đi mất) thì: (1) mọi CPU khác đụng lock quay **rất lâu** (đến khi chủ được schedule lại — có thể ms/vô hạn nếu chủ chờ chính CPU đang quay); (2) tệ hơn — giữ spinlock là **preemption disabled**: gọi schedule trong trạng thái đó là lỗi thẳng (`BUG: scheduling while atomic`); (3) kịch bản khóa chéo: chủ ngủ chờ sự kiện mà sự kiện đó cần CPU đang bận quay → treo hệ.
- "Ngủ" bao gồm cả gián tiếp: `kmalloc(GFP_KERNEL)` (reclaim có thể ngủ), `copy_*_user` (page fault), mutex_lock, msleep — nguồn bug thật vì trông vô hại. Bắt bằng `CONFIG_DEBUG_ATOMIC_SLEEP`.
- Đảo lại cũng đúng: đoạn cần ngủ → mutex; và độ dài critical section quyết định spin có rẻ hơn 2 lần context switch không ([OSTEP](../ostep/concurrency.md) — cùng trade-off, kernel chỉ thêm ràng buộc context).

</details>

**Câu 3:** Per-CPU data giải quyết gì, và vì sao vẫn cần `get_cpu()/put_cpu()`?

<details><summary>Đáp án</summary>

- Mỗi CPU một bản dữ liệu (`DEFINE_PER_CPU`, tr. 255–258 ch. 12) → không chia sẻ → **không cần lock**, không cache-line bouncing (biến counter toàn cục bị N CPU ghi là thảm họa cache coherence — mỗi lần ghi invalidate line của CPU khác). Đây là kỹ thuật scale số 1 của kernel (sloppy counter của [OSTEP](../ostep/concurrency.md) chính là nó).
- Vẫn cần `get_cpu()/put_cpu()` (disable preemption) vì: đọc "CPU của tôi" rồi bị **preempt và dời sang CPU khác** giữa chừng → đang sửa dữ liệu của CPU cũ từ CPU mới — race với chính chủ CPU cũ. Chặn preemption trong đoạn thao tác per-CPU data khép lỗ này (interrupt cũng đụng thì thêm local_irq).
- Trade-off phải nêu: tổng hợp giá trị toàn hệ phải đi gom N bản (chậm/xấp xỉ) — đổi write-scalability lấy read-aggregate; đúng cho counter thống kê, sai cho giá trị cần chính xác tức thời.

</details>

---

## Cụm 3 — Timers & Time (ch. 11, tr. 207–230)

### Nội dung chính

- **HZ** (tr. 208): tần số tick — timer interrupt mỗi 1/HZ giây (x86 1000, nhiều embedded 100/250); HZ cao = độ phân giải mịn + overhead nhiều (tr. 210–211). **jiffies** (tr. 212): đếm tick từ boot; `jiffies_64`; wraparound xử lý bằng macro `time_after/time_before` (tr. 214 — so hiệu có dấu, không so trực tiếp).
- Timer interrupt handler làm gì (tr. 217): cập nhật jiffies/thời gian, accounting cho scheduler, chạy timer hết hạn, load balancing.
- **Kernel timers** (tr. 222): `timer_list` — hẹn chạy hàm tại `jiffies + delay`, chạy trong **softirq context (atomic — không ngủ!)**, không chính xác tuyệt đối (chỉ "không sớm hơn"); race khi hủy: `del_timer_sync` (chờ handler xong — bắt buộc trên SMP/trước khi free dữ liệu).
- **Delay** (tr. 225–229) — chọn theo ngữ cảnh:

| Cần chờ | Cách |
|---|---|
| µs–ms, trong atomic context | `udelay/mdelay` — busy-wait (mdelay dài là tội ác) |
| ms+, process context | `msleep` / `usleep_range` — ngủ thật |
| Chờ đến điều kiện/sự kiện | wait queue / completion — đừng poll |
| Hẹn việc tương lai | timer (atomic) / delayed_work (ngủ được) |

### ⚠️ Đã thay đổi so với sách

- **hrtimers** (độ phân giải ns, không lệ thuộc tick) sách chỉ chớm — nay là chuẩn cho mọi thứ cần chính xác (`hrtimer`, POSIX timers, nanosleep đi qua nó).
- **Tickless/NO_HZ**: `CONFIG_NO_HZ_IDLE` (tắt tick khi idle — tiết kiệm điện, quan trọng embedded), `NO_HZ_FULL` (tắt tick cả khi chạy — core RT/DPDK). Mô hình "tick đều đặn HZ" của sách chỉ còn là một chế độ.
- `timer_list` API đổi (`timer_setup` thay `init_timer`); timer wheel được viết lại (4.8) — ngữ nghĩa "không sớm hơn, có thể trễ" càng rõ.

### Góc interview

**Câu 1:** Driver cần chờ 5ms giữa hai lần ghi thanh ghi. `mdelay(5)` có ổn không? Còn 5µs thì sao?

<details><summary>Đáp án</summary>

- **5ms bằng `mdelay` là tệ**: busy-wait đốt CPU 5ms (hàng triệu chu kỳ), giữ nguyên context — nếu đang atomic thì còn tắt preemption/IRQ suốt 5ms → latency cả hệ tăng vọt (RT chết đứng). Đúng bài: đang ở **process context** (probe, ioctl...) → `msleep(5)` hoặc `usleep_range(5000, 6000)` — ngủ thật, CPU làm việc khác. Chỉ khi kẹt trong atomic context *và* không thể dời việc ra ngoài mới cân nhắc mdelay — và câu trả lời đúng thường là "tái cấu trúc để chờ ở process context / dùng delayed_work".
- **5µs**: ngủ không xuống được cỡ đó (overhead schedule > 5µs) → `udelay(5)` là hợp lệ kể cả trong atomic — busy-wait 5µs rẻ hơn一 lần context switch; `usleep_range` cho khoảng 10µs–ms ở process context.
- Chốt bằng quy tắc: **thời gian chờ so với chi phí context switch + context hiện tại** quyết định busy-wait hay sleep — cùng trade-off spinlock/mutex một lần nữa.

</details>

### Đọc thêm (tùy chọn)

- [ostep/concurrency.md](../ostep/concurrency.md) — nền lock/CV; [melp/debug-realtime.md](../melp/debug-realtime.md) — PREEMPT_RT đổi luật spinlock.
- [02/concurrency.md](../../02-modern-cpp/concurrency.md) — tầng C++ userspace của cùng các khái niệm.
