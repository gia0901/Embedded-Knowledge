# Ch. 9–11 — Kernel Synchronization & Timers (tr. 161–230)

> Thuộc [LKD](README.md) · **[⏮ 02 Interrupts & Bottom Halves](02-interrupts-bottomhalves.md)** · **[04 Memory → ⏭](04-memory.md)**
> Nguồn: **đọc trực tiếp PDF** (Linux Kernel Development, Robert Love, 3rd ed. 2010), **trang sách = trang PDF − 27**.
> Ký hiệu: không đánh dấu = nội dung sách · **🆕 = bổ sung/liên hệ ngoài sách** · **⚠️ = cần cẩn trọng / sách lỗi thời** · **🎯 = câu hỏi phỏng vấn kinh điển** · trích dẫn kèm `(tr. X)`.
> Ba chương này trả lời câu hỏi **"làm sao nhiều luồng chạy đồng thời mà không giẫm lên dữ liệu của nhau"** — chương 9 dựng *lý thuyết* (race condition, deadlock, khóa cái gì), chương 10 là *bộ đồ nghề* (atomic, spinlock, mutex, seqlock, barrier), chương 11 là *thời gian* (HZ, jiffies, timer, delay). Ví dụ xương sống của cả cụm khóa: **"đoạn giữ lock có ngủ được không, và context nào cùng đụng dữ liệu?"** — trả lời hai vế là chọn đúng primitive.

---

## Cụm 1 — Vì sao kernel phải lock (ch. 9, tr. 161–173)

### 1.1 Critical region & race condition (tr. 162)

**Định nghĩa nền:** đoạn code truy cập/sửa dữ liệu chung gọi là **critical region** (hay critical section). Để an toàn, code phải chạy **atomically** — *"operations complete without interruption as if the entire critical region were one indivisible instruction"* (tr. 162). Nếu **hai luồng cùng lúc** ở trong cùng critical region → **race condition** (đặt tên vì các luồng "đua" xem ai tới trước). Việc đảm bảo không có race gọi là **synchronization**. Câu cảnh báo (tr. 162): race *"hard to track down and debug—getting it right at the start is important"* — bug race hiếm khi lộ, khó tái hiện.

**Ví dụ xương sống — máy ATM (tr. 162–163):** rút tiền từ tài khoản. Code kiểm tra đủ tiền rồi trừ:
```c
int total = get_total_from_account();       /* số dư */
int withdrawal = get_withdrawal_amount();   /* số muốn rút */
if (total < withdrawal) { error("Không đủ tiền!"); return -1; }
total -= withdrawal;
update_total_funds(total);                  /* ghi số dư mới */
spit_out_money(withdrawal);                 /* nhả tiền */
```
Giờ giả sử **cùng lúc** có giao dịch thứ hai (vợ rút ở ATM khác, ngân hàng trừ phí). Số dư $105, rút $100 và trừ phí $10. Nếu chạy tuần tự thì đúng (phí trước → $95 → rút $100 fail). Nhưng race (tr. 163): **cả hai cùng kiểm tra** ($105 ≥ cả $100 lẫn $10 → OK), rồi rút trừ ra $5, phí trừ ra $95, rồi **cả hai ghi đè** → kết quả $95, *"Free money!"*. → Ngân hàng phải **khóa tài khoản** để mỗi giao dịch atomic với giao dịch khác.

**Ví dụ tối giản — `i++` (tr. 163–164):** một biến global, một thao tác tăng. `i++` dịch ra **ba lệnh máy**: `đọc i vào register` → `+1` → `ghi lại`. Hai luồng, `i` khởi đầu 7:

| Thread 1 | Thread 2 |
|---|---|
| đọc i (7) | đọc i (7) |
| tăng (7→8) | — |
| — | tăng (7→8) |
| ghi i (8) | — |
| — | ghi i (8) |

→ `i` thành **8** thay vì 9. Giải pháp: lệnh **atomic** đọc-tăng-ghi trong một bước không chia cắt được (chương 10, atomic).

### 1.2 Locking: khóa cửa & tính "advisory" (tr. 165–166)

`i++` chỉ cần một lệnh atomic. Nhưng critical region thật thường **trải nhiều hàm** (lấy dữ liệu khỏi struct này, parse, thêm vào struct kia) — atomic không đủ. Cần **lock** (tr. 165): ẩn dụ **cửa có khóa** — critical region là căn phòng, chỉ một luồng vào một lúc; vào thì khóa cửa sau lưng, xong thì mở, kẻ khác chờ ngoài. *"Threads hold locks; locks protect data."*

⚠️ Điểm quan trọng (tr. 166): lock là **advisory và voluntary** — *"Nothing prevents you from writing code that manipulates the... queue without the appropriate lock."* Lock chỉ có tác dụng nếu **mọi** người truy cập dữ liệu đều tôn trọng nó. Khác biệt lớn nhất giữa các loại lock: **khi lock bận thì làm gì** — có loại **busy wait** (spin), có loại **ngủ** (sleep). Và lock không đẩy race đi đâu cả mà *"shrinks the critical region down to just the lock and unlock code"* — bản thân lock được hiện thực bằng **atomic test-and-set** (x86: `compare and exchange`) nên "kiểm tra & chiếm khóa" là một lệnh không chia cắt.

### 1.3 Nguồn concurrency trong kernel (tr. 167)

Userspace đơn luồng chỉ lo signal. Kernel thì **mặc định concurrent** — sách liệt kê 5 nguồn:
- **Interrupts** — chen bất cứ lúc nào ([02](02-interrupts-bottomhalves.md)).
- **Softirqs & tasklets** — kernel raise/schedule bất cứ lúc nào ([02](02-interrupts-bottomhalves.md)).
- **Kernel preemption** — task trong kernel bị task khác chen ([01](01-process-sched-syscalls.md) mục 2.7).
- **Sleeping** — task ngủ → gọi scheduler → task khác đi vào cùng đường code.
- **SMP** — hai CPU chạy code kernel **cùng lúc thật sự**.

🆕 Phân biệt (tr. 167): **pseudo-concurrency** (một CPU, các luồng đan xen) vs **true concurrency** (SMP, chạy song song thật). Nguyên nhân khác nhau nhưng **cùng gây race, cùng cần bảo vệ**. Code an toàn trước ISR = **interrupt-safe**; trước SMP = **SMP-safe**; trước preemption = **preempt-safe** (tr. 168; và *"being SMP-safe implies being preempt-safe"*).

### 1.4 Khóa cái gì: "Lock data, not code" (tr. 168–169)

Nguyên tắc thiết kế cốt lõi (lặp lại xuyên sách, tr. 168): **khóa DỮ LIỆU, đừng khóa CODE.** Cách dễ nhất là hỏi *cái gì KHÔNG cần khóa*: biến local trên stack, dữ liệu chỉ một task đụng → khỏi khóa. Còn lại: *"if another thread of execution can access the data... if anyone else can see it, lock it."*

Câu quan trọng nhất về **quy trình** (tr. 169): việc *cài lock* thì dễ; việc *khó* là **nhận diện đâu là dữ liệu chung và critical section** — nên phải **thiết kế lock từ đầu**, thêm sau là ác mộng. Bộ câu hỏi tự vấn khi viết code kernel (tr. 169):
- Dữ liệu có global không? Luồng khác truy cập được không?
- Chia sẻ giữa process context và interrupt context? Giữa hai ISR?
- Bị preempt giữa chừng thì task mới có đụng cùng dữ liệu?
- Task có thể ngủ không? Ngủ thì dữ liệu ở trạng thái nào?
- Cái gì ngăn dữ liệu bị free khỏi tay tôi?
- Hàm này chạy lại trên CPU khác thì sao?

> **CONFIG_SMP vs UP (sidebar tr. 168):** kernel cấu hình lúc compile — `CONFIG_SMP` tắt thì code spinlock **compile away** (máy một CPU khỏi tốn); `CONFIG_PREEMPT` tương tự. Lời khuyên: *"provide appropriate protection for the most pessimistic case, SMP with kernel preemption"* — viết cho trường hợp xấu nhất thì mọi cấu hình đều an toàn.

### 1.5 Deadlock: ABBA & lock ordering (tr. 169–171)

**Deadlock (tr. 169):** các luồng chờ nhau nhả tài nguyên, không ai nhả → treo. Ẩn dụ: ngã tư 4 chiều, ai cũng chờ người khác đi trước. Hai dạng:
- **Self-deadlock:** một luồng lấy lại chính lock nó đang giữ → chờ chính mình nhả → treo. ⚠️ *"Linux, thankfully, does not provide recursive locks. This is widely considered a good thing"* (tr. 169) — spinlock/mutex Linux **không đệ quy** (recursive lock dẫn tới semantic khóa cẩu thả).
- **ABBA / deadly embrace (tr. 170):** hai luồng, hai lock; mỗi luồng giữ một cái và chờ cái kia:

| Thread 1 | Thread 2 |
|---|---|
| lấy lock A | lấy lock B |
| chờ lock B | chờ lock A |

**Quy tắc chống deadlock (tr. 170):** (1) **lock ordering** — lock lồng nhau **luôn lấy theo cùng một thứ tự** (và ghi chú thứ tự đó); (2) chống starvation; (3) không lấy đôi cùng một lock; (4) thiết kế đơn giản. Ví dụ cat/dog/fox: nếu một hàm lấy theo cat→dog→fox thì **mọi** hàm phải theo thứ tự đó; lấy fox rồi dog là bug tiềm tàng. Thứ tự **unlock** không ảnh hưởng deadlock (nhưng quen dùng ngược thứ tự lock). 🆕 Kernel có **lockdep** (`CONFIG_PROVE_LOCKING`) kiểm chứng lock ordering lúc runtime — bắt ABBA trước khi nó thành treo thật.

### 1.6 Contention & scalability: granularity (tr. 171–172)

- **Lock contention** = lock đang bị giữ mà luồng khác đang cố lấy. Contention cao (lock hay lấy, hoặc giữ lâu) → **bottleneck**, giới hạn hiệu năng.
- **Granularity** = lượng dữ liệu một lock bảo vệ. **Coarse** (một lock cả subsystem) dễ đúng nhưng nghẽn khi nhiều CPU; **fine-grained** (một lock mỗi phần tử) scale tốt nhưng phức tạp + overhead.

Ví dụ tiến hóa (tr. 172): **runqueue của scheduler** — 2.4 có **một** runqueue global (lock nghẽn: cả hệ chỉ một CPU vào scheduler một lúc); 2.6 O(1) tách **per-CPU runqueue mỗi cái một lock**. ⚠️ Nhưng đừng "fine-grained hóa vô độ": trên máy 2 CPU ít contention thì overhead của cả rừng lock là phí. Kết luận vàng (tr. 172): *"Start simple and grow in complexity only as needed. Simplicity is key."* — bắt đầu coarse, tách khi **đo** thấy contention.

### Insight đáng nhớ (cụm 1)

- Ví dụ ATM (tr. 163) là cách nhớ race condition tốt nhất: **check-rồi-act không atomic = ai cũng thấy đủ tiền rồi cùng trừ**. Cùng khuôn với `i++` (đọc-sửa-ghi bị xen). Trả lời interview nên kể được một trong hai.
- **"Lock data, not code" (tr. 168)** là câu chốt thiết kế: mỗi dữ liệu chung phải có **một chủ lock ghi rõ** ("struct foo được foo_lock bảo vệ"). Lock bọc *code* dẫn tới không biết đang bảo vệ gì.
- **Việc khó không phải cài lock mà là nhận diện dữ liệu chung (tr. 169)** → phải thiết kế lock từ đầu. Bộ 6 câu tự vấn là "checklist" đáng thuộc.

### Góc interview (cụm 1)

**Câu 1:** Vì sao code kernel phải lo đồng bộ nhiều hơn code userspace thường?

<details><summary>Đáp án</summary>

- Userspace đơn luồng chỉ lo signal handler. Kernel **mặc định concurrent** — 5 nguồn (tr. 167): **SMP** (cùng hàm kernel chạy đồng thời N CPU — syscall từ nhiều process), **kernel preemption** (chen giữa hai dòng lệnh — [01](01-process-sched-syscalls.md)), **interrupt/softirq/tasklet** (chen bất cứ đâu — [02](02-interrupts-bottomhalves.md)), **sleeping** (mở cửa cho task khác vào cùng dữ liệu).
- Phân biệt **pseudo-concurrency** (đan xen trên một CPU) vs **true concurrency** (SMP thật) — cùng gây race, cùng cần bảo vệ (tr. 167).
- Hệ quả thiết kế: mọi dữ liệu chung phải có **chủ lock ghi chép** ("lock data, not code"); chọn *loại* lock theo **context nào cùng đụng dữ liệu** (bảng phối ở cụm 2 — câu ăn tiền của kernel dev). Công cụ: **lockdep** (kiểm ordering), `CONFIG_DEBUG_ATOMIC_SLEEP` (bắt "ngủ khi atomic").

</details>

---

## Cụm 2 — Bộ công cụ đồng bộ (ch. 10, tr. 175–206)

### 2.1 Atomic operations (tr. 175–182)

**Nền tảng** của mọi thứ khác — lệnh chạy **không chia cắt được**. Hai họ: **integer** và **bit**.

**Atomic integer (tr. 176–178):** thao tác trên kiểu riêng `atomic_t`:
```c
typedef struct { volatile int counter; } atomic_t;   // KHÔNG dùng int trần
```
Vì sao kiểu riêng (tr. 176)? Để (1) đảm bảo atomic ops chỉ dùng với type này; (2) chặn compiler tối ưu bậy (đọc alias); (3) giấu khác biệt arch. Dùng:
```c
atomic_t v = ATOMIC_INIT(0);
atomic_set(&v, 4);        // v = 4 (atomic)
atomic_add(2, &v);        // v = 6
atomic_inc(&v);           // v = 7
printk("%d\n", atomic_read(&v));         // đọc ra int → "7"
int atomic_dec_and_test(atomic_t *v);    // giảm 1, trả true nếu ==0 (bài toán refcount kinh điển)
```
Dùng nhiều nhất: **counter** (khóa cả scheme cho một counter là overkill — `atomic_inc/dec` nhẹ hơn nhiều, ít cache-line thrash — tr. 179). Có `atomic64_t` (long) cho 64-bit.

> **Atomicity ≠ Ordering (sidebar tr. 179):** đây là phân biệt phải thuộc. **Atomicity** = lệnh chạy trọn vẹn hoặc không, đọc word luôn thấy giá trị *trước* hoặc *sau* một write, không bao giờ *giữa chừng* (int 42→365 đọc ra 42 hoặc 365, không lẫn). **Ordering** = giữ đúng **thứ tự tương đối** của nhiều lệnh (kể cả ở CPU khác). Atomic ops **chỉ đảm bảo atomicity**; ordering do **barrier** lo (mục 2.9).

**Atomic bitwise (tr. 181–182):** khác integer — thao tác trên **địa chỉ nhớ bất kỳ** (con trỏ + số bit), không cần kiểu riêng:
```c
unsigned long word = 0;
set_bit(0, &word);                 // bit 0 = 1 (atomic)
set_bit(1, &word);                 // word = 3
clear_bit(1, &word);
if (test_and_set_bit(0, &word)) { ... }   // set bit 0, trả giá trị CŨ (đọc-sửa-ghi atomic)
```
Có bản **nonatomic** (tiền tố `__`, vd `__test_bit`) — nhanh hơn khi dữ liệu đã được lock cách khác. ⚠️ Với bit, thường **không có lựa chọn khác** — bitops là cách portable duy nhất set một bit cụ thể (tr. 182).

### 2.2 Spin lock — lock phổ biến nhất (tr. 183–188)

**Bản chất (tr. 183):** lock **một chủ**; nếu bị **contended** (đã có người giữ), luồng **busy loop (spin)** chờ. Ẩn dụ (tr. 184): ngồi ngoài cửa **liên tục kiểm tra** chìa khóa. Vì spin **đốt CPU**, spinlock là *"lightweight single-holder lock that should be held for short durations"* — sách nêu con số: **giữ lock ngắn hơn hai lần context switch** (nếu lâu hơn thì đáng lẽ nên ngủ). Dùng:
```c
DEFINE_SPINLOCK(mr_lock);
spin_lock(&mr_lock);
/* critical region ... */
spin_unlock(&mr_lock);
```
Hai điểm phải thuộc:
- ⚠️ **Spinlock KHÔNG đệ quy (tr. 185):** lấy lại lock mình đang giữ → spin chờ chính mình → **deadlock**.
- Trên **UP** (một CPU) spinlock **compile away** — chỉ còn tác dụng **bật/tắt kernel preemption** (đánh dấu vùng nonpreemptible).

**Spinlock với ISR — biến thể `_irqsave` (tr. 185):** spinlock **dùng được trong ISR** (mutex thì không, vì mutex ngủ). Nhưng nếu lock chia sẻ với ISR, phía process **phải tắt local interrupt** trước khi lấy lock. Vì sao? (tr. 185)

> Nếu không tắt: process đang **giữ lock** trên CPU0 → đúng lúc đó ISR nổ **trên CPU0** → ISR cố lấy cùng lock → spin chờ → nhưng chủ lock là process **đứng im ngay dưới chân** ISR (ISR không nhường CPU) → **deadlock một CPU tự khóa mình** (chính là double-acquire deadlock của chương 9).

```c
DEFINE_SPINLOCK(mr_lock);
unsigned long flags;
spin_lock_irqsave(&mr_lock, flags);      // lưu trạng thái IRQ + tắt local IRQ + lấy lock
/* critical region ... */
spin_unlock_irqrestore(&mr_lock, flags); // nhả lock + KHÔI PHỤC trạng thái IRQ cũ
```
`_irqsave`/`_irqrestore` **lưu rồi khôi phục** trạng thái interrupt → an toàn khi lồng (nếu interrupt vốn đã tắt thì không bị bật nhầm). Bản `spin_lock_irq`/`spin_unlock_irq` (bật vô điều kiện) **không khuyến nghị** (tr. 186 — khó chắc interrupt vốn bật). ⚠️ Chỉ cần tắt interrupt **trên CPU hiện tại** — ISR trên CPU khác spin chờ bình thường rồi lấy được lock, không sao (tr. 185).

**Spinlock với bottom half (tr. 187):** `spin_lock_bh()` lấy lock **+ tắt mọi bottom half**. Dùng khi process context chia sẻ dữ liệu với softirq/tasklet. (Chi tiết phối: tasklet cùng loại không tự chạy song song → dữ liệu riêng tasklet khỏi lock; softirq/tasklet-khác-loại → phải lock — [02](02-interrupts-bottomhalves.md) mục 2.7.)

**Các bản khác (tr. 186–187):** `spin_lock_init()` (khởi tạo động), `spin_trylock()` (thử lấy, contended thì trả 0 ngay, **không spin**), `spin_is_locked()`.

### 2.3 Reader-writer spin lock (tr. 188–190)

Khi truy cập chia rõ **đọc** vs **ghi**: nhiều reader **đồng thời** OK (miễn không có writer); writer thì **độc quyền**. (Task list — [01](01-process-sched-syscalls.md) — dùng rwlock.)
```c
DEFINE_RWLOCK(mr_rwlock);
read_lock(&mr_rwlock);   /* chỉ đọc */   read_unlock(&mr_rwlock);
write_lock(&mr_rwlock);  /* đọc+ghi */   write_unlock(&mr_rwlock);
```
⚠️ **Không "nâng cấp" read→write (tr. 189):** `read_lock` rồi `write_lock` → deadlock (writer chờ mọi reader nhả, **kể cả chính mình**). ⚠️ **rwlock thiên vị reader (tr. 190):** reader mới vẫn lấy được lock trong khi writer đang chờ → **đủ reader thì bỏ đói writer** (writer starvation). 🆕 Đây là lý do kernel hiện đại **chuộng RCU** hơn rwlock cho đọc-nhiều (xem ⚠️ cuối cụm).

### 2.4 Semaphore — sleeping lock (tr. 190–194)

**Bản chất (tr. 190):** khi contended, semaphore **cho task ngủ** (đưa vào wait queue) thay vì spin → CPU đi làm việc khác. Ẩn dụ: tới cửa thấy khóa bận thì **ghi tên vào danh sách, lấy số**, người trong phòng ra đánh thức. Đổi lại: overhead cao hơn spinlock (hai lần context switch). Kết luận đáng nhớ (tr. 191):
- ngủ khi chờ → hợp với lock **giữ lâu**; không hợp lock giữ ngắn (overhead > thời gian giữ);
- **chỉ dùng ở process context** (interrupt context không schedulable);
- **được ngủ khi đang giữ semaphore** (không deadlock — kẻ khác chỉ ngủ chờ);
- **không được giữ spinlock rồi đi lấy semaphore** (vì có thể ngủ, mà giữ spinlock thì cấm ngủ);
- semaphore **không tắt kernel preemption** → không hại scheduling latency (khác spinlock).

**Counting vs binary (tr. 191):** count = số người giữ đồng thời tối đa. count=1 = **binary semaphore = mutex** (mutual exclusion). count>1 = counting semaphore (giới hạn số lượng, ít dùng trong kernel). Dijkstra 1968: `P()/V()` → Linux gọi **`down()`/`up()`**. Dùng:
```c
static DECLARE_MUTEX(mr_sem);            // binary semaphore (count=1)
if (down_interruptible(&mr_sem)) {       // ngủ TASK_INTERRUPTIBLE; signal đánh thức → -EINTR
        /* nhận signal, chưa lấy được */
}
/* critical region */
up(&mr_sem);
```
⚠️ Dùng `down_interruptible()` (ngủ nhận signal — đúng) thay vì `down()` (ngủ lì `TASK_UNINTERRUPTIBLE` — thường sai). Có `down_trylock()`. Bản reader-writer: `struct rw_semaphore`, `down_read`/`down_write`, và **`downgrade_write()`** (biến write lock thành read lock — thứ rwlock spin không có, tr. 195).

### 2.5 Mutex — sleeping lock đơn giản, có kỷ luật (tr. 195–197)

Semaphore quá tổng quát → khó debug. Kernel thêm **`struct mutex`** — *"a mutex is a mutex"* (tr. 195): sleeping lock **mutual exclusion**, interface đơn giản hơn + hiệu quả hơn + **ràng buộc chặt hơn**:
```c
DEFINE_MUTEX(name);
mutex_lock(&mutex);
/* critical region */
mutex_unlock(&mutex);
```
**Ràng buộc (tr. 196) — chính là điểm mạnh:**
- chỉ **một** chủ (count luôn = 1);
- **ai khóa phải người đó mở** (không khóa ở context này mở ở context khác);
- **không đệ quy** (không lock lại, không unlock cái đang mở);
- **không được exit khi đang giữ** mutex;
- **không dùng được trong ISR/bottom half** (kể cả `mutex_trylock`);
- chỉ quản qua **API chính thức** (không copy/tay-khởi-tạo).

🆕 Nhờ ràng buộc chặt, `CONFIG_DEBUG_MUTEXES` **tự bắt vi phạm** — đây là lý do mutex là **mặc định**. Công thức chọn (tr. 197): *"prefer the new mutex type to semaphores"* — bắt đầu bằng mutex, chỉ chuyển semaphore khi vướng một ràng buộc và hết cách.

**Spin lock vs mutex — bảng quyết định (Bảng 10.8, tr. 197):**

| Yêu cầu | Chọn |
|---|---|
| Overhead thấp / giữ lock ngắn | **Spin lock** |
| Giữ lock lâu | **Mutex** |
| Cần lock **trong interrupt context** | **Spin lock** (bắt buộc — mutex ngủ) |
| Cần **ngủ** khi đang giữ lock | **Mutex** (bắt buộc — spinlock cấm ngủ) |

### 2.6 Completion variable (tr. 197)

"Chờ việc kia xong": một task `wait_for_completion()`, task kia làm xong gọi `complete()` đánh thức. Là **semaphore-0 kiểu kernel**, dùng cho init/teardown giữa hai luồng.
```c
DECLARE_COMPLETION(mr_comp);
wait_for_completion(&mr_comp);   // bên chờ
complete(&mr_comp);              // bên báo xong
```
🆕 `vfork()` dùng completion để đánh thức cha khi con `exec`/`exit` ([01](01-process-sched-syscalls.md) mục 1.5). Bài chuẩn: "chờ kernel thread khởi động xong rồi mới tiếp".

### 2.7 Seqlock (tr. 200)

Cho **đọc rất nhiều, ghi hiếm, writer không được chờ reader**. Cơ chế: một **sequence counter** — writer lấy lock **tăng sequence** (đang ghi → số **lẻ**); reader đọc sequence trước/sau, **lệch hoặc lẻ thì đọc lại**:
```c
seqlock_t mr_seq_lock = DEFINE_SEQLOCK(mr_seq_lock);
// writer:
write_seqlock(&mr_seq_lock);  /* ghi */  write_sequnlock(&mr_seq_lock);
// reader:
unsigned long seq;
do {
        seq = read_seqbegin(&mr_seq_lock);
        /* đọc dữ liệu */
} while (read_seqretry(&mr_seq_lock, seq));   // có write xen giữa → lặp lại
```
⚠️ Seqlock **thiên vị writer** (writer luôn lấy được, reader lặp lại đến khi hết writer). 🆕 User nổi tiếng: **`jiffies`** (cụm 3) — `get_jiffies_64()` đọc bằng seqlock, timer interrupt cập nhật bằng `write_seqlock(&xtime_lock)` (tr. 201).

### 2.8 Preemption disabling & per-CPU data (tr. 201–202)

Kernel preemptive → task có thể bị chen vào **cùng critical region**. Spinlock đã đánh dấu vùng nonpreemptible (giữ spinlock ⇒ không preempt). Nhưng có lúc **không cần lock mà vẫn cần tắt preemption** — điển hình **per-CPU data**: dữ liệu riêng mỗi CPU thì không cần lock (chỉ CPU đó đụng)... **trừ khi** bị preempt và **dời sang CPU khác** giữa chừng:
```
task A sửa biến per-CPU foo (không lock)
task A bị preempt → task B chạy → task B sửa foo → task A chạy lại, tiếp tục sửa foo  // pseudo-race!
```
Giải: `preempt_disable()`/`preempt_enable()` (nest được — dựa `preempt_count`, tr. 202). Gọn hơn: `get_cpu()` (lấy số CPU **+ tắt preemption**) / `put_cpu()`:
```c
int cpu = get_cpu();      // tắt preemption, cpu = CPU hiện tại
/* thao tác per-CPU data */
put_cpu();                // bật lại; sau đây 'cpu' không còn hợp lệ
```
🆕 `preempt_count` chính là bộ đếm ta gặp ở [01](01-process-sched-syscalls.md) (kernel preemption) và [02](02-interrupts-bottomhalves.md) (`local_bh_disable`) — **một số nguyên = "task có đang atomic không"**, nền của sleep-debugging.

### 2.9 Ordering & barriers (tr. 203–206)

**Vấn đề (tr. 203):** cả **compiler** lẫn **CPU** đều **reorder** load/store để tối ưu. `a=1; b=2;` — CPU có thể ghi `b` trước `a` (không thấy quan hệ). Bình thường vô hại, nhưng khi **giao tiếp với phần cứng** hoặc **giữa CPU trên SMP**, thứ tự **phải** giữ. Barrier ép thứ tự đó:
- **`rmb()`** — read barrier: không load nào bị reorder qua nó.
- **`wmb()`** — write barrier: không store nào bị reorder qua nó.
- **`mb()`** — cả hai.
- **`read_barrier_depends()`** — read barrier chỉ cho load **phụ thuộc dữ liệu** (nhanh hơn `rmb()` trên vài arch).
- **`barrier()`** — chỉ chặn **compiler** reorder (nhẹ như không).
- **`smp_rmb/wmb/mb`** — trên SMP = barrier thật, trên UP = chỉ `barrier()` (compiler).

Ví dụ (tr. 204): Thread 1 `a=3; mb(); b=4;` và Thread 2 `c=b; rmb(); d=a;` — không có barrier, `c` có thể =4 (mới) mà `d`=1 (cũ). `mb()` ép a,b ghi đúng thứ tự; `rmb()` ép c,d đọc đúng thứ tự. ⚠️ Hiệu ứng **tùy arch**: x86 **không reorder store** nên `wmb()` là no-op; **ARM ordering yếu** → thiếu barrier là **bug thật**, không lý thuyết. Dùng barrier cho **arch yếu nhất** thì code compile tối ưu cho arch của bạn.

> **⚠️ BKL — Big Kernel Lock (tr. 198):** đọc như bia mộ. Lock global đệ quy, ngủ-được, chỉ process context — sinh ra để chuyển từ SMP thô (2.0: một task trong kernel một lúc) sang fine-grained. *"New users of the BKL are forbidden"* — nay **đã xóa hẳn** (2.6.39). Bài học: một lock to cho cả kernel **không scale**.

### Insight đáng nhớ (cụm 2)

- **Toàn bộ chương gói trong một câu hỏi: "đoạn giữ lock có ngủ được không, và context nào cùng đụng dữ liệu?"** Vế 1 chọn **spinlock (không ngủ) vs mutex/semaphore (ngủ)**; vế 2 chọn **biến thể** (`_irqsave` nếu ISR đụng, `_bh` nếu bottom half đụng, bản trần nếu chỉ process). Đây là format câu hỏi phỏng vấn kernel phổ biến nhất.
- **Atomicity ≠ Ordering (tr. 179)** — atomic ops chỉ đảm bảo cái trước; thứ tự cần **barrier**. Nhầm hai cái này là nguồn bug lock-free trên ARM.
- **Ràng buộc của mutex (tr. 196)** *là* điểm mạnh, không phải phiền phức: chính nó cho phép `CONFIG_DEBUG_MUTEXES` bắt lỗi tự động → nên mutex là mặc định, semaphore chỉ khi buộc.
- **`preempt_count`** xuyên suốt ba chương: đếm lock + preempt-disable + BH-disable = "atomicity của task". Nắm nó là nắm cầu nối mọi context.

### ⚠️ Đã thay đổi so với sách

- **RCU (Read-Copy-Update)** — sách gần như không cover, nay là cơ chế **đọc-nhiều thống trị** kernel: reader đọc **không lock** (chỉ `rcu_read_lock`, gần như free); writer **copy → sửa → đổi con trỏ** nguyên tử; **grace period** trước khi free bản cũ (đợi mọi reader cũ rời đi). Thay dần rwlock (không còn writer starvation). Phỏng vấn kernel senior hay chạm — trình bày được ý tưởng 3 câu là điểm cộng lớn.
- **BKL xóa hẳn** (2.6.39). **Semaphore** gần như chỉ còn legacy — `struct mutex` + completion phủ hầu hết. **`DECLARE_MUTEX`/`init_MUTEX`** (semaphore-as-mutex) đã bỏ; dùng `DEFINE_MUTEX`.
- **PREEMPT_RT** (nay đã vào mainline) biến **spinlock thường thành sleeping lock** (rt_mutex) để giảm latency → "spinlock không ngủ" đúng với kernel thường, **có dấu sao với RT** ([melp/debug-realtime.md](../melp/debug-realtime.md)). Barrier API mở rộng: `smp_load_acquire`/`smp_store_release` (acquire/release semantics kiểu C11).

### Góc interview (cụm 2)

**Câu 1 (🎯):** Dữ liệu được cả **syscall path** lẫn **ISR** truy cập — dùng lock gì, và giải thích deadlock nếu chọn sai.

<details><summary>Đáp án</summary>

- Chọn **`spin_lock_irqsave`** ở phía process context (syscall path — [01](01-process-sched-syscalls.md)); trong chính ISR dùng `spin_lock` thường (vào ISR thì local IRQ vốn đã tắt). Không dùng mutex/semaphore vì **ISR không ngủ được** ([02](02-interrupts-bottomhalves.md)) → sleeping lock cấm trong ISR.
- **Deadlock nếu process path dùng `spin_lock` trần (tr. 185):** process **giữ lock** trên CPU0 → ISR nổ **trên CPU0** → ISR `spin_lock` chờ lock → nhưng chủ lock là process **đứng im ngay dưới chân** ISR trên cùng CPU (ISR không nhường CPU bao giờ) → **spin vĩnh viễn = một CPU tự khóa mình** (double-acquire deadlock, chương 9). `_irqsave` **tắt local interrupt** trong đoạn giữ lock → ISR không chen được trên CPU này; CPU khác thì ISR vẫn chạy nhưng spin chờ rồi lấy được lock — đúng nghĩa.
- **Điểm cộng:** `_irqsave` vs `_irq` (save giữ nguyên trạng thái IRQ trước — an toàn khi lồng); chỉ chung với tasklet/softirq thì `spin_lock_bh` đủ (rẻ hơn tắt IRQ); giữ critical section ngắn (đo bằng thời gian tắt IRQ — nguồn latency số một của RT).

</details>

**Câu 2 (🎯):** Vì sao "không được ngủ khi đang giữ spinlock"? Điều gì **thật sự** xảy ra nếu ngủ?

<details><summary>Đáp án</summary>

- Spinlock chờ bằng **quay** (tr. 183) — kẻ chờ **đốt CPU** đến khi chủ nhả. Chủ **ngủ** (schedule đi mất) thì: (1) mọi CPU khác đụng lock quay **rất lâu** (đến khi chủ được schedule lại — có thể ms/vô hạn nếu chủ chờ chính CPU đang quay); (2) tệ hơn — giữ spinlock là **preemption disabled** (tr. 185); gọi schedule trong trạng thái đó là lỗi thẳng **`BUG: scheduling while atomic`**; (3) khóa chéo: chủ ngủ chờ sự kiện mà sự kiện cần CPU đang bận quay → treo hệ.
- **"Ngủ" gồm cả gián tiếp:** `kmalloc(GFP_KERNEL)` (reclaim có thể ngủ — [04](04-memory.md)), `copy_*_user` (page fault — [01](01-process-sched-syscalls.md) mục 3.5), `mutex_lock`, `msleep` — nguồn bug thật vì trông vô hại. Bắt bằng `CONFIG_DEBUG_ATOMIC_SLEEP`.
- **Đảo lại cũng đúng:** đoạn cần ngủ → mutex (bảng tr. 197); và độ dài critical section quyết định spin có rẻ hơn hai lần context switch không (tr. 183) — cùng trade-off của [ostep/concurrency.md](../ostep/concurrency.md), kernel chỉ thêm ràng buộc context.

</details>

**Câu 3:** Per-CPU data giải quyết gì, và vì sao **vẫn** cần `get_cpu()/put_cpu()`?

<details><summary>Đáp án</summary>

- Mỗi CPU một bản dữ liệu (`DEFINE_PER_CPU`, ch. 12 — [04](04-memory.md)) → không chia sẻ → **không cần lock**, không **cache-line bouncing** (một counter global bị N CPU ghi là thảm họa cache coherence — mỗi ghi invalidate line CPU khác). Kỹ thuật scale số 1 của kernel.
- **Vẫn cần `get_cpu()/put_cpu()` (tr. 201–202):** đọc "CPU của tôi" rồi bị **preempt và dời sang CPU khác** giữa chừng → đang sửa dữ liệu CPU cũ *từ* CPU mới → race với chính chủ CPU cũ. `get_cpu()` **tắt preemption** khép lỗ này (interrupt cũng đụng thì thêm `local_irq`).
- **Trade-off phải nêu:** tổng hợp giá trị toàn hệ phải gom N bản (chậm/xấp xỉ) — đổi write-scalability lấy read-aggregate; đúng cho counter thống kê, sai cho giá trị cần chính xác tức thời.

</details>

---

## Cụm 3 — Timers & Time (ch. 11, tr. 207–230)

### 3.1 HZ & tick (tr. 208–212)

**Bức tranh:** nhiều việc kernel là **time-driven** (định kỳ: cân bằng runqueue, refresh màn hình) hoặc **hẹn giờ** (delayed I/O). Việc định kỳ do **system timer** lo — phần cứng phát **interrupt ở tần số cố định**; handler của nó (**timer interrupt**) cập nhật thời gian + làm việc định kỳ.

**HZ (tr. 208–209):** tần số tick, define lúc compile trong `<asm/param.h>`. **x86 mặc định 100** (cũng có 250/1000); **ARM 100** (Bảng 11.1). Chu kỳ = 1/HZ giây. ⚠️ *"never assume that HZ has any given value"* — đừng hard-code 100.

**Chọn HZ = trade-off (tr. 210–211):** HZ cao (1000) → **độ phân giải mịn hơn** (granularity 1ms thay vì 10ms), **timer/`poll`/`select` chính xác hơn**, **preemption đúng lúc hơn** (giảm scheduling latency — timer interrupt là chỗ giảm timeslice + set `need_resched`). Nhưng HZ cao → **timer interrupt chạy thường hơn** → **nhiều overhead, cache thrash, tốn điện** hơn.

> **Tickless OS (sidebar tr. 212):** `CONFIG_NO_HZ` — thay vì bắn timer đều mỗi 1ms, kernel **lập lịch timer động theo timer đang chờ**; hệ idle thì không bị interrupt vô ích → **tiết kiệm điện** (quan trọng cho embedded/laptop). Mô hình "tick đều đặn HZ" của sách nay chỉ còn là một chế độ.

### 3.2 Jiffies & wraparound (tr. 212–216)

**`jiffies` (tr. 212):** biến global đếm **số tick từ khi boot**; tăng 1 mỗi timer interrupt → có HZ jiffies/giây → **uptime = jiffies/HZ**. Chuyển đổi:
```c
unsigned long later = jiffies + 5*HZ;        // 5 giây nữa
unsigned long fraction = jiffies + HZ / 10;  // 1/10 giây nữa
```
**Nội tại (tr. 213–214):** `jiffies` là `unsigned long` (32-bit trên máy 32-bit → **wrap sau ~497 ngày ở HZ=100, chỉ 49.7 ngày ở HZ=1000**!). Kernel giữ `jiffies` overlay lên **32 bit thấp** của `jiffies_64` (linker magic) — code cũ đọc `jiffies` như thường, code quản thời gian dùng `get_jiffies_64()` (đọc bằng seqlock — cụm 2).

**Wraparound — phải dùng macro (tr. 214–215):** so `jiffies` trực tiếp là bug khi nó wrap về 0. Kernel cho 4 macro so hiệu **có dấu**:
```c
#define time_after(unknown, known)  ((long)(known) - (long)(unknown) < 0)
#define time_before(unknown, known) ((long)(unknown) - (long)(known) < 0)
// ...time_after_eq, time_before_eq
```
Bản an toàn wraparound của "kiểm timeout":
```c
unsigned long timeout = jiffies + HZ/2;      // hết hạn sau 0.5s
/* ... làm việc ... */
if (time_before(jiffies, timeout)) { /* chưa hết giờ */ }
else { /* hết giờ — lỗi */ }
```

### 3.3 Timer interrupt handler (tr. 216–221)

Hai phần cứng (tr. 216): **RTC** (real-time clock — pin nuôi, giữ giờ khi tắt máy; boot đọc RTC khởi tạo **wall time** `xtime`) và **system timer** (x86: **PIT**, hoặc local APIC timer/TSC — phát interrupt ở HZ).

Handler chia phần arch-riêng (lấy `xtime_lock`, ack timer, gọi tiếp) và **`tick_periodic()`** arch-chung (tr. 218):
```c
static void tick_periodic(int cpu)
{
        if (tick_do_timer_cpu == cpu) {
                write_seqlock(&xtime_lock);        // xtime_lock LÀ seqlock, không phải spinlock
                tick_next_period = ktime_add(tick_next_period, tick_period);
                do_timer(1);
                write_sequnlock(&xtime_lock);
        }
        update_process_times(user_mode(get_irq_regs()));
        profile_tick(CPU_PROFILING);
}
void do_timer(unsigned long ticks) {
        jiffies_64 += ticks;      // TĂNG jiffies (an toàn vì đang giữ xtime_lock)
        update_wall_time();       // cập nhật giờ tường
        calc_global_load();       // tính load average
}
```
`update_process_times()` → gọi `run_local_timers()` (raise `TIMER_SOFTIRQ` chạy timer hết hạn), `scheduler_tick()` (giảm timeslice + set `need_resched` — [01](01-process-sched-syscalls.md)). 🆕 Chú ý: accounting **tính cả tick cho process đang chạy lúc interrupt** dù nó có thể đã vào/ra kernel nhiều lần trong tick — accounting thô kiểu Unix, cũng là lý do muốn HZ cao (tr. 219).

**Wall time (tr. 220):** `xtime` (`struct timespec`: giây từ **epoch 1/1/1970 UTC** + nano-giây), bảo vệ bằng **seqlock** `xtime_lock`. `gettimeofday()` → `sys_gettimeofday()` đọc `xtime` bằng vòng seqlock rồi `copy_to_user` ([01](01-process-sched-syscalls.md) mục 3.5).

### 3.4 Dynamic timers (tr. 222–224)

Khác bottom half ("hoãn đến lúc nào tiện"), **kernel timer hoãn đến *sau một khoảng thời gian*** — *"certainly no less, and with hope, not much longer"*:
```c
struct timer_list my_timer;
init_timer(&my_timer);
my_timer.expires  = jiffies + delay;   // hết hạn sau 'delay' tick (giá trị tuyệt đối)
my_timer.data     = 0;                 // tham số truyền vào handler
my_timer.function = my_function;       // void my_function(unsigned long data)
add_timer(&my_timer);                  // kích hoạt
mod_timer(&my_timer, jiffies + new_delay);  // đổi hạn (kích hoạt luôn nếu đang tắt)
```
⚠️ Hai điểm sinh tử:
- Timer chạy trong **softirq context (atomic — KHÔNG ngủ!)** (`TIMER_SOFTIRQ`, tr. 224) — handler timer không được ngủ. Và **không chính xác tuyệt đối**: *"there may be a delay... timers cannot be used to implement any sort of hard real-time processing"* — chỉ đảm bảo "không sớm hơn hạn".
- **Race khi hủy (tr. 223):** `del_timer()` chỉ đảm bảo timer **sẽ không chạy trong tương lai** — nhưng handler **có thể đang chạy trên CPU khác** (SMP). Muốn chờ handler xong (bắt buộc trước khi free dữ liệu timer dùng): **`del_timer_sync()`** (⚠️ không dùng được từ interrupt context vì nó chờ/ngủ). Đừng "del rồi set expires rồi add" thay `mod_timer` (không an toàn SMP).

🆕 Hiện thực (tr. 224): timer lưu trong **5 nhóm theo thời điểm hết hạn** (timer wheel), dời dần xuống khi tới gần → mỗi lần chạy softirq chỉ tốn ít công tìm timer hết hạn.

### 3.5 Delay: chờ ngắn không dùng timer (tr. 225–229)

Driver hay cần chờ **rất ngắn** (µs) cho phần cứng — chọn theo ngữ cảnh:

| Cần chờ | Cách | Ghi chú |
|---|---|---|
| Bội số tick / không cần chính xác | **busy loop** `while (time_before(jiffies, timeout));` | ⚠️ đốt CPU; `jiffies` là `volatile` để reload mỗi vòng (tr. 226) |
| Bội số tick, cho phép nhường | `while (...) cond_resched();` | Chỉ schedule nếu `need_resched` — **process context** |
| µs–ms, **atomic context** | `udelay(usecs)` / `ndelay` / `mdelay(msecs)` | **Busy-wait** dựa `loops_per_jiffy`/BogoMIPS (tr. 227); `mdelay` dài là **tội ác** |
| ms+, **process context** | `schedule_timeout(s * HZ)` sau `set_current_state(TASK_INTERRUPTIBLE)` | **Ngủ thật** — CPU làm việc khác (tr. 227) |
| Chờ **sự kiện HOẶC hết giờ** | wait queue + `schedule_timeout` | Dậy vì event / hết giờ / signal — phải kiểm tra lý do (tr. 229) |

`udelay`/`mdelay` là busy-loop biết trước bao nhiêu vòng = bao lâu (từ `loops_per_jiffy` tính lúc boot — sidebar **BogoMIPS**, tr. 227). `schedule_timeout()` thực chất **là một ứng dụng của kernel timer** (đặt timer gọi `process_timeout` đánh thức, rồi `schedule()` — tr. 228). ⚠️ Mọi cách delay: **tránh khi đang giữ lock hoặc interrupt tắt** (tr. 226).

### Insight đáng nhớ (cụm 3)

- **HZ là một trade-off** (tr. 210): mịn hơn ↔ tốn hơn. Nhớ được "granularity = 1/HZ, average error = nửa tick" là trả lời được vì sao `poll/select` timeout và scheduling latency phụ thuộc HZ.
- **`jiffies` wrap (tr. 214)** — 49.7 ngày ở HZ=1000 — là lý do **phải dùng `time_after/time_before`** (so hiệu có dấu), không so `jiffies` trực tiếp. Câu hỏi bẫy kinh điển.
- **Timer chạy trong softirq = atomic = không ngủ** (tr. 224), và **`del_timer_sync` bắt buộc trên SMP** trước khi free — hai điểm hay sai nhất khi viết driver.
- **Busy-wait vs sleep = cùng trade-off spinlock/mutex** (thời gian chờ so với chi phí context switch + context hiện tại): `udelay` trong atomic/ngắn, `msleep`/`schedule_timeout` ở process context/dài.

### ⚠️ Đã thay đổi so với sách

- **hrtimers** (high-resolution timers, phân giải ns, không lệ thuộc tick) — sách chỉ chớm; nay là chuẩn cho mọi thứ cần chính xác (`hrtimer`, POSIX timers, `nanosleep` đi qua nó). `timer_list` API đổi: **`timer_setup()`** thay `init_timer` (handler nhận `struct timer_list *`, không còn `data` trần); timer wheel viết lại (4.8).
- **Tickless / NO_HZ** đã thành thực tế: `NO_HZ_IDLE` (tắt tick khi idle — tiết kiệm điện), **`NO_HZ_FULL`** (tắt tick cả khi chạy — core dành cho RT/DPDK).
- **`usleep_range()`** là bản khuyến nghị cho ngủ khoảng chục µs–ms ở process context (thay busy `udelay` khi có thể); `msleep` cho ms+. `del_timer_sync` nay là `timer_delete_sync`.

### Góc interview (cụm 3)

**Câu 1:** Driver cần chờ **5ms** giữa hai lần ghi thanh ghi. `mdelay(5)` có ổn không? Còn **5µs** thì sao?

<details><summary>Đáp án</summary>

- **5ms bằng `mdelay` là tệ (tr. 227):** busy-wait **đốt CPU 5ms** (hàng triệu chu kỳ), giữ nguyên context — nếu đang atomic thì còn tắt preemption/IRQ suốt 5ms → latency cả hệ tăng vọt (RT chết đứng). Đúng bài: đang ở **process context** (probe/ioctl) → **`msleep(5)`** hoặc `usleep_range(5000, 6000)` — ngủ thật, CPU làm việc khác. Chỉ khi kẹt trong atomic context *và* không dời việc ra được mới cân nhắc `mdelay`; câu trả lời đúng thường là "tái cấu trúc để chờ ở process context / dùng `schedule_delayed_work`".
- **5µs:** ngủ không xuống được cỡ đó (overhead schedule > 5µs) → **`udelay(5)`** hợp lệ kể cả trong atomic — busy-wait 5µs rẻ hơn một lần context switch; `usleep_range` cho khoảng chục µs–ms ở process context.
- **Chốt bằng quy tắc:** **thời gian chờ so với chi phí context switch + context hiện tại** quyết định busy-wait hay sleep — cùng trade-off spinlock/mutex một lần nữa.

</details>

**Câu 2:** Vì sao so `if (jiffies > timeout)` là bug? Sửa thế nào?

<details><summary>Đáp án</summary>

- **`jiffies` là `unsigned long`, sẽ wrap** (tr. 214) — ~49.7 ngày ở HZ=1000, 497 ngày ở HZ=100. Nếu `timeout = jiffies + HZ/2` mà `jiffies` **wrap về 0** giữa chừng, thì `jiffies` (số nhỏ) < `timeout` (số lớn) **dù logic đã trễ hạn** → điều kiện đảo ngược, bug.
- **Sửa:** dùng macro so **hiệu có dấu** — `time_before(jiffies, timeout)` / `time_after(jiffies, timeout)` (tr. 215). Chúng ép về `(long)` và so hiệu, nên đúng cả khi một tham số vừa wrap qua 0.
- **Điểm cộng:** cùng lớp vấn đề với đọc `jiffies_64` không nguyên tử trên máy 32-bit → phải `get_jiffies_64()` (seqlock, cụm 2); và biết `jiffies` khai `volatile` để busy-loop reload mỗi vòng (tr. 226).

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [01-process-sched-syscalls.md](01-process-sched-syscalls.md) — `preempt_count`/kernel preemption, wait-queue, `copy_*_user` (có thể ngủ); [02-interrupts-bottomhalves.md](02-interrupts-bottomhalves.md) — interrupt/bottom-half context, `local_bh_disable`, `spin_lock_bh` (bảng phối lock).
- [04-memory.md](04-memory.md) — `GFP_KERNEL` vs `GFP_ATOMIC` (vì sao alloc "có thể ngủ"), per-CPU allocator.
- [ostep/concurrency.md](../ostep/concurrency.md) — nền lý thuyết lock/CV/semaphore; [melp/debug-realtime.md](../melp/debug-realtime.md) — PREEMPT_RT đổi luật spinlock.
- [02-modern-cpp/concurrency.md](../../02-modern-cpp/concurrency.md) — tầng C++ userspace của cùng các khái niệm (atomic, memory order, barrier).

**Chương tiếp theo:** [04 — Memory Management →](04-memory.md) (page/zone, kmalloc/GFP, slab, vmalloc, per-CPU, và address space của process — nơi `GFP_ATOMIC` và "alloc có thể ngủ" được giải thích tận gốc).
