# OSTEP — Phần II: Concurrency (ch. 26–33)

> Thuộc [OSTEP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Crux của phần này: *nhiều luồng thực thi đan xen trên dữ liệu chung — làm sao xây các nguyên thủy đồng bộ (lock, condition variable, semaphore) đúng VÀ hiệu quả, từ hỗ trợ tối thiểu của phần cứng?*

---

## Cụm 1 — Threads & Thread API (ch. 26–27) 🎯

### Nội dung chính

**Thread = luồng thực thi trong cùng address space.** Mỗi thread có riêng: **PC, registers, stack** (mỗi thread một stack trong cùng address space — "thread-local" theo nghĩa vị trí, không phải bảo vệ). Chia sẻ: **code, data/globals, heap, file descriptors**. Context switch giữa thread cùng process: đổi registers/stack, **không đổi page table** → rẻ hơn process switch.

Vì sao dùng thread: (1) **parallelism** — tận dụng nhiều core cho một việc; (2) **tránh block** — thread này chờ I/O, thread khác chạy tiếp (overlap trong một chương trình).

**Vấn đề trung tâm — race condition:** ví dụ xương sống của cả phần: 2 thread cùng `counter++` 1 triệu lần, kết quả < 2 triệu và **mỗi lần chạy mỗi khác**. Vì `counter++` là **3 lệnh máy**:

```
load  reg ← counter      Thread A: load (đọc 50)
reg = reg + 1               ── interrupt, switch sang B ──
store counter ← reg      Thread B: load 50, +1, store 51
                         Thread A (tiếp): +1 (từ 50), store 51  ← mất 1 lần đếm!
```

Từ vựng chuẩn phải dùng đúng: **critical section** (đoạn code đụng dữ liệu chung), **race condition** (kết quả phụ thuộc timing), **mutual exclusion** (chỉ một thread trong critical section tại một thời điểm), **indeterminate** (chạy mỗi lần mỗi khác). Kẻ thù thật sự: **scheduler có thể cắt ở BẤT KỲ lệnh nào** — code đúng là code đúng với mọi cách đan xen.

**Thread API (ch. 27)** — pthread tối thiểu:

```c
pthread_create(&t, NULL, worker, arg);   // chạy worker(arg) trên thread mới
pthread_join(t, &ret);                   // chờ + lấy giá trị trả về
// ⚠️ đừng trả về con trỏ tới biến TRÊN STACK của thread con — stack đó biến mất sau khi thread kết thúc
pthread_mutex_lock(&m); ... pthread_mutex_unlock(&m);   // luôn check return code
pthread_cond_wait(&cv, &m); pthread_cond_signal(&cv);   // cụm 3
```

### Insight đáng nhớ

- Câu đóng đinh mental model: **"race không phải vì hai thread chạy 'cùng lúc' — mà vì thao tác tưởng một bước hóa ra nhiều bước, và bị cắt giữa chừng."** Trên 1 core vẫn race như thường (timer interrupt cắt); nhiều core chỉ làm nó dễ xảy ra và thêm tầng memory ordering.
- "Stack của thread" nằm trong **cùng address space** — thread khác *có thể* ghi đè lên nó (bug hoang dã nhất: con trỏ lạc sang stack thread khác). Chia sẻ theo mặc định là con dao hai lưỡi của thread so với process.

### Ít quan trọng

- Chi tiết từng flag của pthread attr, trylock/timedlock (ch. 27) — tra man page khi cần; guideline cuối chương (giữ code đồng bộ đơn giản, init lock đúng chỗ...).

### Góc interview

**Câu 1 (🎯):** Thread khác process những gì? Khi nào chọn multi-process thay vì multi-thread?

<details><summary>Đáp án</summary>

| | Thread (cùng process) | Process |
|---|---|---|
| Address space | **Chung** (code/heap/globals); riêng stack + registers | Riêng hoàn toàn |
| Tạo/switch | Nhẹ; switch không đổi page table (không flush TLB) | Nặng hơn; switch đổi address space |
| Trao đổi dữ liệu | Trực tiếp qua bộ nhớ chung (nhanh, nhưng phải tự đồng bộ) | Phải qua IPC (pipe, shm, socket) |
| Cô lập lỗi | **Không** — một thread segfault/hỏng heap → chết/hỏng cả process | Có — chết một process, hệ còn lại sống |

- Chọn **multi-process** khi cần: **cô lập lỗi/bảo mật** (mỗi kết nối/plugin một process — kiến trúc browser, sshd), quyền khác nhau (drop privilege), sống sót từng phần (watchdog restart process con), hoặc tận dụng nhiều máy sau này. Chọn **thread** khi: chia sẻ dữ liệu lớn thường xuyên, cần switch/spawn rẻ, latency thấp.
- Điểm cộng: nói được "trên Linux cả hai đều là `task` tạo bằng `clone()` — khác nhau ở cờ chia sẻ gì (CLONE_VM, CLONE_FILES...)" — thread/process là **phổ liên tục**, không phải hai loài.

</details>

**Câu 2:** Vì sao `counter++` từ 2 thread cho kết quả sai dù chỉ chạy trên 1 core? `volatile` có cứu không?

<details><summary>Đáp án</summary>

- `counter++` = load → add → store (3 lệnh; kể cả khi là 1 lệnh `inc` trên x86, không có prefix `lock` thì trên SMP vẫn không atomic). Timer interrupt có thể cắt **giữa load và store**: hai thread cùng đọc giá trị cũ → hai lần tăng thành một. Một core hay nhiều core chỉ khác *xác suất*, không khác *bản chất*.
- `volatile` **không cứu**: nó chỉ cấm compiler tối ưu/gộp truy cập — load-add-store vẫn là 3 bước cắt được, không có atomicity, không có ordering giữa các thread (xem [EMC++ Item 40](../effective-modern-cpp.md#cụm-7--concurrency-api-ch-7-items-3540-)). Dùng volatile làm đồng bộ là data race = UB.
- Cứu đúng cách: `std::atomic<int>`/`atomic_fetch_add` (lệnh RMW atomic của phần cứng — `lock xadd`, `ldxr/stxr`), hoặc mutex quanh đoạn tăng. Chọn atomic cho một biến đơn; mutex khi bất biến phủ nhiều biến.

</details>

---

## Cụm 2 — Locks: từ phần cứng đến mutex (ch. 28–29) 🎯

### Nội dung chính

**Tiêu chí đánh giá một lock:** (1) **mutual exclusion** — đúng đã; (2) **fairness** — có thread đói không; (3) **performance** — chi phí khi không tranh chấp / tranh chấp trên 1 CPU / nhiều CPU.

**Chuỗi xây dựng — vì sao cần phần cứng:**
- **Tắt interrupt**: chỉ chạy được trên 1 CPU, trao cho user quyền quá lớn, miss interrupt — chỉ hợp trong kernel cho đoạn cực ngắn.
- **Load/store thường (cờ `flag`)**: sai — hai thread cùng thấy flag=0 rồi cùng set 1 (kiểm tra và đặt là 2 bước) → cần lệnh **atomic RMW** từ phần cứng.
- **Test-and-set** (đọc giá trị cũ + ghi 1, atomic) → **spinlock**:

```c
void lock(lock_t *l)   { while (TestAndSet(&l->flag, 1) == 1) ; }  // quay chờ
void unlock(lock_t *l) { l->flag = 0; }
```

Đúng, nhưng: **không fair** (ai may nấy được, có thể đói), và **spin đốt CPU** — trên 1 CPU là thảm họa (spin nguyên time slice trong khi kẻ giữ lock không được chạy).

- **Compare-and-swap (CAS)**: mạnh hơn (nếu `*p == expected` thì ghi new, atomic) — nền của mọi cấu trúc lock-free; **LL/SC** (load-linked/store-conditional — ARM `ldxr/stxr`) tương đương.
- **Ticket lock** (dùng fetch-and-add): lấy số thứ tự, chờ đến lượt — **FIFO công bằng**, mọi thread đều tiến; vẫn spin.
- **Chuyển spin thành ngủ**: yield (nhường CPU — đỡ nhưng vẫn tốn switch, vẫn không fair) → **hàng đợi + park/unpark** (Solaris) hoặc **futex** (Linux): chờ thì **ngủ trong kernel**, unlock đánh thức đúng một thread; kèm xử lý race "chuẩn bị ngủ thì lock được nhả" (setpark/futex so sánh giá trị trước khi ngủ).
- **Two-phase lock** — cái mutex thật của glibc: **spin một chút** (đoán lock sắp nhả — tiết kiệm 2 lần context switch) rồi mới **ngủ bằng futex**. Fast path không tranh chấp: **một lệnh CAS trong user space, không syscall**.

**Locked data structures (ch. 29):** thêm lock vào cấu trúc dữ liệu:
- Counter chính xác (1 lock) không scale → **approximate/sloppy counter**: counter cục bộ per-CPU + dồn về global theo ngưỡng S — đổi độ chính xác lấy scalability (tư tưởng per-CPU data thấy khắp kernel).
- List/queue/hash: **hand-over-hand locking** (lock từng node) nghe hay nhưng thường **chậm hơn 1 lock to** (chi phí lock/unlock mỗi node); hash table mỗi **bucket một lock** thì scale đẹp — bài học: **concurrency thêm vào phải trả phí, đo rồi hãy tin**.

### Insight đáng nhớ

- Bậc thang đáng thuộc: **atomic instruction → spinlock → +fairness (ticket) → +sleep (futex) → two-phase mutex**. Trả lời "mutex hoạt động thế nào" bằng bậc thang này là trọn vẹn từ phần cứng lên glibc.
- Quy tắc chọn nhanh: **spinlock** cho critical section cực ngắn + biết chắc kẻ giữ lock đang chạy trên CPU khác (trong kernel, interrupt context — không được ngủ); **mutex** cho user space mặc định. Spin trên 1 CPU = đốt time slice vô ích.
- "Đúng trước, nhanh sau, và ĐO": sloppy counter/bucket lock thắng, hand-over-hand thua — trực giác concurrency thường sai, benchmark quyết định.

### Ít quan trọng

- Cài đặt đầy đủ queue lock với guard/park từng dòng và bảng so đo counter (ch. 28–29 giữa chương); fetch-and-add semantics chi tiết.

### Góc interview

**Câu 1 (🎯):** Spinlock vs mutex — khác nhau thế nào, chọn khi nào? Vì sao trong interrupt handler không được dùng mutex?

<details><summary>Đáp án</summary>

- **Spinlock**: chờ bằng **vòng lặp CAS/test-and-set** — không nhả CPU. Được: không context switch, latency lấy lock cực thấp khi chờ ngắn. Mất: đốt CPU suốt lúc chờ; thảm họa nếu kẻ giữ lock không đang chạy (bị preempt/1 CPU) — spin cả time slice vô ích.
- **Mutex** (futex-based): tranh chấp thì **ngủ** — nhả CPU cho việc khác, được đánh thức khi unlock. Được: không đốt CPU, chờ lâu vẫn ổn. Mất: 2 context switch + syscall khi tranh chấp (fast path không tranh chấp vẫn chỉ là 1 CAS user space); giữ mutex là có thể **ngủ**.
- Chọn: critical section **ngắn** (vài chục–trăm ns), chạy đa core, không thể ngủ → spinlock (chủ yếu trong kernel). Mặc định user space → mutex (glibc đã two-phase: spin ngắn rồi ngủ — bạn được cả hai).
- **Interrupt context không được dùng mutex** vì mutex có thể **ngủ (schedule)** — interrupt handler không phải thread có ngữ cảnh schedule được (không có task để đưa vào hàng chờ rồi đánh thức; ngủ trong handler = treo CPU/deadlock hệ thống). Kernel dùng spinlock, và phía process phải dùng bản `spin_lock_irqsave` — **tắt interrupt cục bộ** khi lấy lock mà handler cũng lấy, nếu không: handler ngắt đúng lúc process đang giữ lock, handler spin chờ lock mà kẻ giữ nó đứng im ngay dưới chân mình → deadlock một CPU.
- **Bẫy khi trả lời:** "spinlock nhanh hơn mutex" vô điều kiện — chỉ đúng khi chờ ngắn hơn chi phí 2 lần switch; và quên fast path của mutex không có syscall.

</details>

**Câu 2:** Futex là gì? Vì sao mutex hiện đại "rẻ khi không tranh chấp"?

<details><summary>Đáp án</summary>

- **Futex (fast userspace mutex)** = cơ chế kernel Linux gồm hai nửa: trạng thái lock là **một biến int trong user space**; kernel chỉ cung cấp hai thao tác trên địa chỉ đó khi cần chờ: `FUTEX_WAIT(addr, expected)` — *ngủ nếu `*addr` vẫn bằng expected* (so sánh + ngủ là atomic trong kernel → không mất wakeup), và `FUTEX_WAKE(addr, n)` — đánh thức n kẻ chờ.
- Đường đi của mutex glibc:
  1. **Lock, không tranh chấp**: một lệnh **CAS** 0→1 trong user space. Thành công → xong, **zero syscall**. Đây là lý do "rẻ khi không tranh chấp".
  2. **Lock, tranh chấp**: CAS fail → đặt trạng thái "có kẻ chờ" (2) → `FUTEX_WAIT(addr, 2)` ngủ trong kernel.
  3. **Unlock**: đặt 0; nếu trạng thái cho biết có kẻ chờ → `FUTEX_WAKE(addr, 1)`.
- Kèm spin ngắn trước khi WAIT (two-phase) để né syscall khi lock sắp nhả.
- Điểm cộng: futex là nền của gần như mọi nguyên thủy đồng bộ user space trên Linux (mutex, condvar, semaphore, `std::mutex`, park/unpark của runtime các ngôn ngữ); kể được "so sánh expected để chống lost wakeup" là hiểu thật.

</details>

---

## Cụm 3 — Condition Variables (ch. 30) 🎯

### Nội dung chính

**Bài toán:** thread cần **chờ một điều kiện** (con đã xong, buffer có hàng...). Spin-check cờ thì đốt CPU → cần cơ chế **ngủ cho tới khi được báo**: condition variable = **hàng đợi các thread đang chờ**, với `wait(cv, mutex)` và `signal(cv)`/`broadcast(cv)`.

**Hai quy tắc sống còn:**

1. **`wait` phải gọi khi đang giữ mutex** — nó *atomically*: nhả mutex + đưa thread vào hàng chờ + ngủ; thức dậy thì **lấy lại mutex** rồi mới trả về. Không có atomicity này → race giữa "kiểm tra điều kiện" và "đi ngủ": điều kiện thành true + signal bắn **ngay trước khi kịp ngủ** → ngủ vĩnh viễn (**lost wakeup**).
2. **Luôn kiểm tra điều kiện bằng `while`, không phải `if`:**

```c
pthread_mutex_lock(&m);
while (queue_empty())            // KHÔNG BAO GIỜ dùng if!
    pthread_cond_wait(&cv, &m);  // nhả m + ngủ; dậy thì đã cầm lại m
item = dequeue();
pthread_mutex_unlock(&m);
```

Vì: (a) **Mesa semantics** (mọi hệ thật): signal chỉ là *gợi ý* "điều kiện có thể đã đổi" — giữa lúc được đánh thức và lúc thực sự chạy, thread khác có thể **nẫng mất** điều kiện (consumer khác lấy mất hàng); (b) **spurious wakeup** — được phép dậy không lý do. `while` = dậy thì kiểm tra lại, sai thì ngủ tiếp.

**Producer/consumer (bounded buffer)** — bài mẫu phải viết được tay:

```c
cond_t empty, fill;  mutex_t m;   // HAI cv: producer chờ 'empty', consumer chờ 'fill'

producer:                          consumer:
lock(m);                           lock(m);
while (count == MAX)               while (count == 0)
    wait(&empty, &m);                  wait(&fill, &m);
put(i);                            tmp = get();
signal(&fill);                     signal(&empty);
unlock(m);                         unlock(m);
```

Vì sao **hai** cv: dùng một cv, consumer có thể đánh thức **consumer khác** (thay vì producer) → cả đám ngủ hết (deadlock kiểu "signal nhầm đối tượng"). Sách dựng từng phiên bản sai → chỉ chỗ chết → sửa; đáng đọc lại nguyên văn khi có PDF.

Mẹo an toàn: khi không chắc signal hay broadcast — **broadcast + while** luôn đúng (chỉ tốn hiệu năng: thundering herd); ví dụ covering condition: `allocate/free memory` — free bao nhiêu không biết ai đủ dùng → broadcast.

### Insight đáng nhớ

- **"Signal không chuyển giao trạng thái — nó chỉ là tiếng gõ cửa."** Trạng thái nằm trong biến được mutex bảo vệ; cv chỉ là chỗ ngủ. Suy ra cả hai quy tắc: giữ mutex khi wait (đọc trạng thái nhất quán) và while (trạng thái có thể đổi trước khi bạn chạy).
- Cặp (mutex + cv + biến điều kiện) là **một bộ ba không tách rời** — thiếu một trong ba là sai. Đây cũng chính là `std::condition_variable` + `unique_lock` + predicate của C++ ([02/concurrency.md](../../02-modern-cpp/concurrency.md)).

### Ít quan trọng

- Lịch sử Hoare semantics (signal chuyển quyền ngay — đẹp lý thuyết, không hệ nào dùng) (ch. 30, aside).
- Các phiên bản sai trung gian của producer/consumer (đọc nguyên văn thì hay, tóm lại thì mất giá trị).

### Góc interview

**Câu 1 (🎯):** Vì sao chờ condition variable phải dùng `while` chứ không phải `if`? Nêu đủ hai lý do.

<details><summary>Đáp án</summary>

1. **Mesa semantics:** `signal` chỉ chuyển thread chờ sang **ready** — không chạy ngay, không cầm mutex ngay. Từ lúc được đánh thức tới lúc thật sự cầm lại mutex và chạy, **thread thứ ba có thể chen vào lấy mất điều kiện** (consumer khác dequeue mất phần tử vừa được produce). `if` → dậy là tin điều kiện đúng → thao tác trên queue rỗng → crash/UB. `while` → dậy kiểm tra lại, mất thì ngủ tiếp.
2. **Spurious wakeup:** POSIX/`std::condition_variable` cho phép `wait` trả về **không có signal nào** (hệ quả cài đặt: futex bị ngắt, multiprocessor race). Chuẩn nói thẳng: predicate phải được kiểm tra lại.
- Chốt: `while` biến "được đánh thức" từ *cam kết* thành *gợi ý kiểm tra lại* — đúng bản chất Mesa. C++ gói sẵn: `cv.wait(lk, pred)` chính là vòng `while (!pred()) wait;`.
- **Bẫy khi trả lời:** chỉ nêu spurious wakeup mà quên Mesa/kẻ chen ngang — lý do thứ nhất mới là lý do thiết kế chính; spurious chỉ là "giọt nước tràn ly" khiến while thành bắt buộc tuyệt đối.

</details>

**Câu 2 (🎯):** Viết producer/consumer với bounded buffer bằng mutex + condition variable. Vì sao cần hai condition variable?

<details><summary>Đáp án</summary>

```cpp
std::mutex m;
std::condition_variable not_full, not_empty;
std::queue<Item> q;
const size_t MAX = 64;

void produce(Item x) {
    std::unique_lock<std::mutex> lk(m);
    not_full.wait(lk, [&]{ return q.size() < MAX; });  // = while(!pred) wait
    q.push(std::move(x));
    not_empty.notify_one();          // đánh thức CONSUMER
}
Item consume() {
    std::unique_lock<std::mutex> lk(m);
    not_empty.wait(lk, [&]{ return !q.empty(); });
    Item x = std::move(q.front()); q.pop();
    not_full.notify_one();           // đánh thức PRODUCER
    return x;
}
```

- **Hai cv** để signal **đúng đối tượng**: consumer xong việc phải đánh thức *producer* (chỗ trống mới), producer xong phải đánh thức *consumer* (hàng mới). Dùng một cv + `notify_one`: consumer lấy hàng xong có thể đánh thức **consumer khác** (vào thấy queue rỗng, ngủ lại) trong khi producer đầy hàng chờ mãi → hệ đứng im dù việc còn. (Một cv + `notify_all` thì đúng nhưng thundering herd.)
- Các điểm chấm điểm: predicate trong `wait` (tự động while), `notify` **sau khi** đổi trạng thái, `unique_lock` chứ không `lock_guard` (wait cần nhả/lấy lại), move item ra ngoài trước khi (tùy chọn) unlock sớm.

</details>

---

## Cụm 4 — Semaphores (ch. 31)

### Nội dung chính

**Semaphore = một số nguyên + hai thao tác atomic** (Dijkstra): `sem_wait` (P) — giảm 1, **âm thì ngủ**; `sem_post` (V) — tăng 1, có kẻ ngủ thì đánh thức một. Giá trị khởi tạo là toàn bộ "tính cách": 

| Khởi tạo | Vai trò |
|---|---|
| 1 | **Binary semaphore** = lock |
| 0 | **Ordering/signaling** = chờ sự kiện (join: con post, cha wait; giống cv một phát) |
| N | **Đếm tài nguyên** = giới hạn N kẻ vào (connection pool, bounded buffer slots) |

Producer/consumer bằng semaphore: `empty=MAX, full=0` đếm chỗ/hàng + **mutex quanh thao tác buffer** — thứ tự lấy quan trọng: **wait(empty/full) TRƯỚC, lock(mutex) SAU** — ôm mutex rồi mới ngủ chờ chỗ là deadlock kinh điển.

Ứng dụng khác: **reader-writer lock** (readers đếm nhau, reader đầu khóa writers — kèm cảnh báo: writer đói, và RW lock thường **không nhanh hơn** mutex thường — đo đi!); **throttling** (chặn N thread vào vùng tốn RAM); Zemaphore — tự cài semaphore bằng mutex+cv (bài tập chuẩn).

### Insight đáng nhớ

- Semaphore = **cv + counter nhớ lịch sử**: khác cv (signal vào hư không là mất), `sem_post` **được đếm lại** — post trước wait sau vẫn nhận. Đây là điểm phân biệt bản chất và là câu trả lời cho "cv vs semaphore".
- Thực chiến embedded: semaphore là nguyên thủy **ISR-friendly** kinh điển (RTOS: `xSemaphoreGiveFromISR` báo task từ interrupt) — nơi mutex (có ownership, có thể ngủ) không dùng được; POSIX: `sem_post` là **async-signal-safe**, gọi được trong signal handler, pthread_cond_signal thì không.

### Ít quan trọng

- Bàn luận lịch sử Dijkstra/tên P-V; cài reader-writer đầy đủ và biến thể no-starve (ch. 31 giữa) — đọc khi cần viết RW lock tay.

### Góc interview

**Câu 1:** Mutex vs binary semaphore — có phải một thứ không?

<details><summary>Đáp án</summary>

Giống nhau ở mức "cho 1 kẻ qua", khác nhau ở **hợp đồng**:
- **Ownership:** mutex có chủ — ai lock nấy unlock (unlock hộ là UB/EPERM); semaphore **không có chủ** — thread A wait, thread B post là hợp lệ (chính vì thế nó làm được signaling giữa ISR→task, thread→thread).
- **Hệ quả của ownership:** mutex hỗ trợ được **priority inheritance** (biết chủ để nâng priority — chống priority inversion, quan trọng cho realtime), **recursive lock**, phát hiện lỗi (unlock kẻ khác, relock chính mình). Semaphore không thể có các tính năng này — không biết "chủ" là ai.
- **Ngữ nghĩa đếm:** post semaphore được tích lũy; unlock mutex đang mở là lỗi.
- Chốt: dùng mutex cho **mutual exclusion**, semaphore cho **signaling/đếm tài nguyên** — dùng binary semaphore làm lock là bỏ priority inheritance và error checking, trên hệ realtime đây là khác biệt nghiêm trọng (vụ Mars Pathfinder là case priority inversion + mutex có PI là thuốc chữa).

</details>

---

## Cụm 5 — Concurrency Bugs (ch. 32) 🎯

### Nội dung chính

Nghiên cứu thực nghiệm (Lu et al. — khảo sát bug 4 app lớn: MySQL, Apache, Mozilla, OpenOffice): **đa số bug concurrency KHÔNG phải deadlock** (~2/3 là non-deadlock), và non-deadlock quy về 2 mẫu:

1. **Atomicity violation** — hai thao tác *tưởng* liền khối bị cắt giữa:

```c
Thread1: if (thd->proc_info) fputs(thd->proc_info, ...);   // check ...
Thread2: thd->proc_info = NULL;                             // ... bị cắt ở đây
// fix: lock quanh CẢ cụm check-rồi-dùng (bất biến phủ nhiều thao tác)
```

2. **Order violation** — code *giả định* A chạy trước B nhưng không có gì ép:

```c
Thread1: mThread = PR_CreateThread(...);      // tưởng chạy trước
Thread2: mThread->State = ...;                // NULL nếu chạy trước Thread1!
// fix: ép thứ tự bằng cv/semaphore (init xong mới post), đừng "chắc nó kịp"
```

**Deadlock — 4 điều kiện cần (đủ cả 4 mới deadlock được):** (1) **mutual exclusion** — tài nguyên độc quyền; (2) **hold-and-wait** — ôm cái này chờ cái kia; (3) **no preemption** — không tước được; (4) **circular wait** — vòng chờ khép kín. Phá **bất kỳ một** điều kiện là hết deadlock:

| Phá điều kiện | Kỹ thuật thực tế |
|---|---|
| Circular wait | **Lock ordering toàn cục** (giải pháp thực dụng số 1 — thứ tự theo địa chỉ lock khi phải lấy cặp: `std::lock(m1, m2)`, `scoped_lock`) |
| Hold-and-wait | Lấy tất cả lock trong một "mega lock" — giết concurrency, ít dùng |
| No preemption | `trylock` + backoff (thả hết, thử lại — coi chừng **livelock**: thêm random delay) |
| Mutual exclusion | Lock-free bằng CAS (không lock thì không deadlock) |
| Né hẳn (avoidance) | Banker/schedule tĩnh — cần biết trước, phi thực tế app thường |
| Detect & recover | DB làm (phát hiện chu trình wait-for graph, giết một transaction) |

### Insight đáng nhớ

- Con số "2/3 bug không phải deadlock" chỉnh lại trọng tâm ôn tập: **atomicity violation** (check-then-act, read-modify-write) mới là bug bạn sẽ viết ra tuần sau — pattern nhận diện: *"khoảng hở giữa kiểm tra và hành động"* (giống hệt TOCTOU của file system).
- Bốn điều kiện deadlock là **bộ khung trả lời interview chuẩn**: nêu 4 điều kiện → chỉ ra phá cái nào bằng gì → chốt "thực dụng nhất là lock ordering". C++: `std::scoped_lock(m1, m2)` lấy nhiều lock không deadlock.
- Nối sang tooling của repo: TSan (ThreadSanitizer) bắt data race lúc chạy, helgrind; lockdep trong kernel bắt sai lock ordering — [09/tools.md](../../09-debugging/tools.md).

### Ít quan trọng

- Chi tiết Dining Philosophers (đổi thứ tự lấy đũa của một triết gia = phá circular wait) — biết kể trong 3 câu là đủ; wait-free/lock-free sâu hơn nằm ngoài phạm vi sách.

### Góc interview

**Câu 1 (🎯):** Nêu 4 điều kiện deadlock. Với mỗi điều kiện, một kỹ thuật thực tế để phá. Trong code C++ bạn phòng deadlock thế nào?

<details><summary>Đáp án</summary>

- Bốn điều kiện (thiếu một là không thể deadlock): **mutual exclusion** (tài nguyên độc quyền), **hold-and-wait** (ôm một chờ hai), **no preemption** (không tước được lock), **circular wait** (vòng T1→L2, T2→L1 khép kín).
- Phá từng cái: circular wait → **áp thứ tự lấy lock toàn cục** (theo thiết kế hoặc theo địa chỉ mutex); hold-and-wait → gom việc lấy lock vào một điểm/atomic; no preemption → `trylock` + nhả hết + retry với **random backoff** (tránh livelock — hai thread nhả rồi va nhau mãi); mutual exclusion → cấu trúc lock-free bằng CAS cho hot path.
- C++ thực dụng:
  1. `std::scoped_lock lk(m1, m2);` — lấy nhiều mutex bằng thuật toán tránh deadlock (thử-và-nhả), thay cho hai `lock_guard` lồng nhau.
  2. Quy ước tầng: lock của tầng thấp không bao giờ gọi ra code lấy lock tầng cao (đặc biệt: **không gọi callback của user khi đang giữ lock** — deadlock kinh điển của thư viện).
  3. Thu nhỏ critical section — copy dữ liệu ra rồi xử lý ngoài lock.
  4. Kiểm chứng: TSan + stress test; kernel: lockdep.
- **Bẫy khi trả lời:** liệt kê 4 điều kiện xong dừng — người phỏng vấn chờ vế "phá thế nào"; và nhầm livelock với deadlock (livelock: vẫn chạy hùng hục mà không tiến — thiếu backoff).

</details>

**Câu 2:** Atomicity violation là gì? Vì sao nó phổ biến hơn deadlock, và tool nào bắt được?

<details><summary>Đáp án</summary>

- Là bug "**khoảng hở check–act**": trình tự tưởng nguyên khối (kiểm tra rồi dùng, đọc-sửa-ghi, iterate trong khi kẻ khác sửa) bị thread khác chen giữa. Ví: `if (p) use(*p)` đua với `p = NULL`; `map.count(k)` rồi `map[k]` đua với `erase`. Mutex có sẵn cũng không cứu nếu **lock từng thao tác riêng lẻ** thay vì lock **trọn bất biến** — sai phổ biến: "container này thread-safe từng hàm" nhưng chuỗi hai hàm thì không.
- Phổ biến hơn deadlock vì: nó là **mặc định của tư duy tuần tự** — ai cũng viết check-then-act theo thói quen; deadlock cần cấu hình nhiều lock chéo nhau mới xuất hiện, atomicity violation chỉ cần *một* biến chung + *hai* dòng code. Và nó im lặng: không treo, chỉ thỉnh thoảng sai — khó lộ khi test.
- Tool: **ThreadSanitizer** (TSan — bắt data race lúc chạy, báo cả hai stack trace đụng nhau), helgrind; race không có data race thuần (đua logic trên atomic) thì TSan bó tay — phải review bất biến. Phòng từ thiết kế: thu hẹp dữ liệu chung, API giao dịch trọn gói (`find_or_insert` một lệnh thay vì check + insert), immutable + message passing.

</details>

---

## Cụm 6 — Event-based Concurrency (ch. 33)

### Nội dung chính

Con đường né thread: **một luồng + event loop** — chờ sự kiện (`select/poll/epoll` trên tập fd), sự kiện đến thì chạy **handler ngắn, chạy trọn** (run-to-completion):

```c
while (1) {
    n = epoll_wait(...);            // ngủ tới khi có fd sẵn sàng
    for (mỗi sự kiện) handler(fd);  // KHÔNG bị cắt ngang → không race, không lock
}
```

- Được: **không lock, không race nội bộ** (chỉ một luồng — scheduler không cắt được handler), kiểm soát lịch chạy tường minh, rẻ hơn nghìn thread.
- Giá phải trả:
  1. **Không được block** trong handler — một lệnh `read` chậm/disk I/O là **đứng cả server** → mọi I/O phải non-blocking/async (AIO cho disk), kéo theo:
  2. **Stack bị xé** (*manual stack management*): trạng thái "đang làm dở" phải tự đóng gói vào **continuation** (struct + callback) thay vì nằm trên stack như code thread — code khó viết/đọc hơn (chính là "callback hell"; async/await của các ngôn ngữ hiện đại là đường ngọt hóa cho đúng vấn đề này).
  3. **Không tự tận dụng đa core** — một loop = một core; muốn nhiều core: nhiều loop/process (nginx worker) → lại có chia sẻ dữ liệu → lock quay lại.

### Insight đáng nhớ

- Event-based không "giải quyết" concurrency — nó **đổi trục vấn đề**: từ *race + lock* (thread) sang *không được block + tự quản trạng thái* (event). Chọn phe = chọn loại bug bạn muốn sống cùng.
- Đây là mô hình của nginx/Node.js/libuv và của **rất nhiều firmware/middleware nhúng** (main loop + state machine) — bạn có thể đã viết event-based mà không gọi tên nó. Chi tiết epoll/LT-ET: [04/io-multiplexing.md](../../04-linux-system-programming/io-multiplexing.md).

### Ít quan trọng

- API `select()` từng tham số và ví dụ code đầy đủ (repo 04 đã kỹ hơn); bàn luận AIO/signal trộn event (ch. 33 cuối).

### Góc interview

**Câu 1:** Server 10k kết nối: thread-per-connection vs event loop — trade-off? Vì sao nginx chọn event còn code của nó khó viết hơn Apache cổ điển?

<details><summary>Đáp án</summary>

- **Thread-per-connection:** mỗi kết nối một thread ngủ trên `read()`. 10k thread = 10k stack (mặc định 8MB virtual/thread — hàng chục GB address space, RAM commit theo dùng thật nhưng vẫn nặng), scheduler quản 10k thực thể, context switch + cache nguội theo tải; nhưng **code tuần tự dễ viết**, block thoải mái, tận dụng đa core tự nhiên, một kết nối treo không lây (che được lỗi block).
- **Event loop (epoll):** một thread quản cả 10k fd — bộ nhớ ~O(kết nối) chứ không O(thread), không switch, không lock. Đổi lại: handler **tuyệt đối không được block** (một disk read đồng bộ đứng cả 10k kết nối), trạng thái mỗi kết nối phải tự quản (state machine/continuation), core phải scale bằng nhiều worker process.
- nginx chọn event vì bài toán của proxy/static server là **I/O-bound với hàng vạn kết nối nhàn rỗi** — chi phí mỗi kết nối phải gần 0; cái giá "code khó" trả một lần bởi đội nginx, người dùng không phải viết handler. Apache prefork/worker chọn dễ-viết-dễ-mở-rộng-module — mỗi mô hình đúng với bài toán của nó.
- Điểm cộng: mô hình lai hiện đại — event loop cho network I/O + **thread pool cho việc nặng CPU/disk** (libuv, io_uring, "N loop × M worker"); và async/await = event-based với stack do compiler quản hộ.

</details>

### Đọc thêm (tùy chọn)

- [03/sync-primitives.md](../../03-operating-system/sync-primitives.md), [02/concurrency.md](../../02-modern-cpp/concurrency.md) — nguyên thủy đồng bộ hai tầng OS/C++.
- [04/io-multiplexing.md](../../04-linux-system-programming/io-multiplexing.md) — select/poll/epoll chi tiết.
- [EMC++ Cụm 7](../effective-modern-cpp.md) — concurrency API mức C++.
