# OS — Operating System

> Domain `OS`. Track dùng: `os`, `bsp`, `cpp-system`, `ostep`, `cpp-concurrency`.

---

#### OS-001 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Process và thread khác nhau thế nào?**
<details><summary>Đáp án</summary>

Process có không gian địa chỉ riêng và tài nguyên riêng, được cô lập. Thread là luồng thực thi trong process, chia sẻ code/data/heap/fd với thread cùng process nhưng có stack/register riêng. Thread tạo và giao tiếp rẻ (qua bộ nhớ chung) nhưng dễ data race và một thread lỗi sập cả process; process cô lập tốt nhưng tạo/giao tiếp đắt hơn.
</details>

#### OS-002 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Thread chia sẻ gì, có riêng gì?**
<details><summary>Đáp án</summary>

Chia sẻ: code, data/bss, heap, file descriptor, signal handler. Riêng: stack, register (gồm PC, SP), errno, thread-local storage.
</details>

#### OS-003 · 🟡 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Deadlock là gì? Bốn điều kiện Coffman? Cách tránh?**
<details><summary>Đáp án</summary>

**Cơ chế** — các thread chờ vòng tròn tài nguyên do nhau giữ nên kẹt **vĩnh viễn** (không tự thoát, khác livelock/starvation). Ca kinh điển 2 lock:

```
T1: lock(A) ──── chờ B ────► T2 đang giữ B
T2: lock(B) ──── chờ A ────► T1 đang giữ A      → kẹt
```

**Bốn điều kiện Coffman — phải thoả ĐỒNG THỜI cả 4**, nên chỉ cần phá **một** là hết deadlock:

| Điều kiện | Nghĩa | Phá bằng cách |
|---|---|---|
| **Mutual exclusion** | Tài nguyên không chia sẻ được | Hiếm phá được (bản chất của lock); dùng lock-free/bản sao dữ liệu |
| **Hold-and-wait** | Giữ lock này rồi mới đi xin lock kia | Lấy **tất cả** lock một lần: `std::scoped_lock(m1, m2)` |
| **No preemption** | Không ai giật lock khỏi tay thread khác | `try_lock` + timeout → thất bại thì **nhả hết** rồi thử lại |
| **Circular wait** | Đồ thị chờ tạo thành vòng | ⭐ **Lock ordering** — mọi nơi trong code luôn khoá theo **cùng một thứ tự toàn cục** |

```cpp
// ❌ Deadlock: hai hàm khoá ngược thứ tự nhau
void f() { lock_guard a(m1); lock_guard b(m2); }
void g() { lock_guard b(m2); lock_guard a(m1); }   // ngược -> circular wait

// ✅ C1: lock ordering — quy ước m1 luôn trước m2 ở MỌI nơi
// ✅ C2: để thư viện lo, lấy cả hai nguyên tử (thuật toán tránh deadlock)
void f() { std::scoped_lock lk(m1, m2); }   // C++17
void g() { std::scoped_lock lk(m2, m1); }   // thứ tự tham số KHÔNG còn quan trọng
```

**Bẫy hay gặp:** (1) nghĩ deadlock chỉ xảy ra với ≥2 mutex — **một** mutex non-recursive tự lock hai lần cũng đủ chết; (2) quên rằng gọi **callback / hàm người khác viết** trong lúc giữ lock có thể lại đi lock ngược thứ tự → đừng giữ lock khi gọi ra ngoài; (3) tưởng `try_lock` là xong — thử lại mãi mà không backoff sẽ thành **livelock**.

**Chốt:** thuộc 4 điều kiện, nhưng thực chiến trả lời **2 thứ**: `scoped_lock` (phá hold-and-wait) và **lock ordering** (phá circular wait). Phát hiện: `gdb` + `bt` mọi thread xem ai chờ ai, hoặc TSan.
</details>

#### OS-004 · 🟢 · concept · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Race condition là gì?**
<details><summary>Đáp án</summary>

Kết quả phụ thuộc thứ tự/timing không kiểm soát giữa nhiều luồng truy cập dữ liệu chung (ít nhất một ghi). Vùng cần loại trừ lẫn nhau là critical section; bảo vệ bằng mutex hoặc atomic.
</details>

#### OS-005 · 🟡 · concept · ⭐ · [→ process-thread](../../../03-operating-system/process-thread.md)
**Context switch là gì? Vì sao switch process tốn hơn switch thread?**
<details><summary>Đáp án</summary>

**Context switch** = OS lưu trạng thái CPU của tác vụ đang chạy (register, **PC**, **SP**, cờ) và khôi phục trạng thái của tác vụ kế.

**Chi phí gồm hai phần — phần thứ hai lớn hơn nhiều và hay bị bỏ sót:**

| | Thread ↔ thread (cùng process) | Process ↔ process |
|---|---|---|
| **Trực tiếp** — lưu/khôi phục register, chạy scheduler | ✅ có | ✅ có |
| Đổi **address space** (nạp page table base — `CR3`/`TTBR`) | ❌ **không cần** (chung address space) | ✅ **phải** |
| **Flush TLB** | ❌ | ✅ thường phải (trừ khi CPU có **ASID/PCID** để gắn tag) |
| **Gián tiếp** — cache/TLB "lạnh" sau khi đổi | Nhẹ | **Nặng** — vài trăm ns→µs kế tiếp chạy chậm vì miss liên tục |

**Điểm mấu chốt:** chi phí *trực tiếp* (lưu ~16 thanh ghi) chỉ vài trăm nanosecond và gần như nhau ở cả hai ca. Cái làm process switch **đắt hơn hẳn** là chi phí **gián tiếp**: sau khi flush TLB, mọi truy cập bộ nhớ đầu tiên đều **TLB miss** → phải **page table walk** (nhiều lần đọc RAM), cộng với cache bị tác vụ mới đẩy ra. Bạn không "trả tiền" lúc switch mà trả **rải rác sau đó**.

**Vì sao đáng quan tâm trong embedded/real-time:** context switch là nguồn **jitter**. Hệ real-time đo **worst-case**, và một switch kèm cache/TLB lạnh có thể đội latency lên hàng chục lần so với trung bình. Đó là lý do người ta **ghim thread vào core** (`sched_setaffinity`), giảm số thread, và tránh thiết kế bắt switch liên tục.

**Bẫy:** (1) nói "flush **cache**" — sai thuật ngữ; đúng là **đổi page table + flush TLB** (cache dữ liệu phần lớn có thể giữ nhờ đánh địa chỉ vật lý); (2) quên **ASID/PCID** — CPU hiện đại gắn tag để **không phải** flush toàn bộ, nên "luôn flush TLB" là quá tuyệt đối; (3) không phân biệt switch **tự nguyện** (block chờ I/O) và **bị ép** (hết time slice, preempt) — `vmstat`/`pidstat -w` cho hai số này, và tỉ lệ ép cao là dấu hiệu quá nhiều thread tranh CPU.

**Chốt:** *"Lưu/khôi phục register là phần rẻ. Đổi process còn phải đổi page table + flush TLB, và cái đắt nhất là cache/TLB lạnh mà bạn trả dần sau đó."*
</details>

#### OS-006 · 🟡 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Mutex và spinlock khác nhau? Khi nào dùng spinlock?**
<details><summary>Đáp án</summary>

**Khác nhau đúng một chỗ: làm gì khi *không lấy được* lock.**
- **Mutex** → **ngủ** (block): thread bị gỡ khỏi runqueue, nhường CPU cho việc khác; khi lock rảnh mới được đánh thức.
- **Spinlock** → **bận xoay** (busy-wait): `while (!try_acquire()) {}` — giữ nguyên CPU, đốt chu kỳ để kiểm tra liên tục.

| | Mutex | Spinlock |
|---|---|---|
| Khi chờ | Ngủ (block) | Bận xoay |
| Chờ **lâu** | Rẻ (nhường CPU) | Đắt (đốt CPU vô ích) |
| Chờ **rất ngắn** | Đắt (2 lần context switch ~ µs) | Rẻ (vài chục ns) |
| Trên **1 core** | Hoạt động bình thường | **Vô nghĩa/chết**: thread giữ lock không có CPU để chạy tiếp mà nhả |
| Phù hợp | Critical section dài, user space | Critical section **vài lệnh**, kernel/SMP |

**Vì sao kernel lại cần spinlock, không dùng mutex cho gọn?** Không phải chỉ vì tốc độ — mà vì có **ngữ cảnh không được phép ngủ**: trong **interrupt handler** (không có process context để đánh thức lại) hay trong atomic context, gọi thứ có thể ngủ là bug nghiêm trọng. Spinlock là lựa chọn *duy nhất* ở đó. Kernel Linux còn có biến thể `spin_lock_irqsave()` — vừa lấy lock vừa tắt ngắt local, tránh ISR trên **cùng core** nhảy vào đòi chính lock đó và tự khoá mình.

**Bẫy chí mạng:** ⚠️ **tuyệt đối không ngủ/block khi đang giữ spinlock** (không `kmalloc(GFP_KERNEL)`, không `copy_to_user`, không mutex) — thread khác sẽ spin **mãi mãi** trên các core còn lại, đốt 100% CPU. Đây là câu hỏi kiểm tra rất phổ biến ở vị trí kernel/BSP.

**Chốt:** *"Chờ ngắn hơn một lần context switch → spin; dài hơn → ngủ. Trong ngữ cảnh không được ngủ (ISR) thì không có lựa chọn nào khác ngoài spin."*
</details>

#### OS-007 · 🟡 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md#L74), [scheduling §6](../../../03-operating-system/scheduling.md)
**Mutex và semaphore khác nhau?**
<details><summary>Đáp án</summary>

**Khác biệt gốc là *mục đích*, không phải cơ chế** — nhiều người trả lời "semaphore đếm được, mutex thì không" và dừng ở đó; đó mới là phần vỏ.

| | **Mutex** = *khoá có chủ* | **Semaphore** = *bộ đếm báo hiệu* |
|---|---|---|
| Dùng để | **Bảo vệ** critical section (mutual exclusion) | **Báo hiệu** giữa luồng, hoặc quản **N** tài nguyên |
| **Ownership** | ✅ thread nào lock thì **chính nó** phải unlock | ❌ không có chủ — **bất kỳ** thread nào cũng `post/signal` được |
| Hình dạng dùng | **Đối xứng**: lock…unlock trong cùng một thread, cùng phạm vi | **Bất đối xứng**: bên A `wait`, bên B `signal` (điển hình **ISR → task**) |
| Priority inheritance | ✅ có (RTOS/pthread `PTHREAD_PRIO_INHERIT`) | ❌ không |

**Vì sao ownership quan trọng đến vậy** — nó là điều kiện để có **priority inheritance**, thứ chặn **priority inversion**:

```
L (ưu tiên THẤP) đang giữ mutex.
H (ưu tiên CAO)  cần mutex đó → phải chờ L.
M (ưu tiên TRUNG BÌNH, không liên quan) preempt L.
→ H bị chặn vô thời hạn bởi M — kẻ có ưu tiên thấp hơn nó.   ❌
```

Với **mutex**, OS biết *ai đang giữ* (nhờ ownership) nên **tạm nâng ưu tiên của L lên bằng H** để L chạy xong và nhả lock nhanh → H đi tiếp. Với **semaphore**, kernel **không biết ai đang giữ** (ai signal cũng được) → **không thể** nâng ai → priority inversion không được xử lý. Đây chính là sự cố **Mars Pathfinder (1997)**.

**Bẫy:** dùng **binary semaphore (0/1) thay mutex** để bảo vệ critical section — chạy đúng trong test, chết trong hệ real-time. Mất priority inheritance, và mất luôn cả lưới an toàn "unlock nhầm thread" (`sem_post` từ thread khác vẫn hợp lệ → mở khoá sau lưng chủ sở hữu).

**Chốt:** *"Mutex bảo vệ, semaphore báo hiệu."* Cùng bảo vệ được về mặt hình thức, nhưng chỉ mutex có **ownership** → chỉ mutex có **priority inheritance**.
</details>

#### OS-008 · 🟡 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Virtual memory giải quyết vấn đề gì?**
<details><summary>Đáp án</summary>

Cho mỗi process không gian địa chỉ ảo riêng được MMU ánh xạ tới vật lý: (1) bảo vệ/cô lập giữa các process; (2) đơn giản hóa — mỗi process thấy không gian liền mạch, tránh fragmentation ngoài; (3) overcommit — bộ nhớ ảo vượt RAM nhờ swap + demand paging.
</details>

#### OS-009 · 🟡 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Zombie và orphan process là gì?**
<details><summary>Đáp án</summary>

Zombie: con đã kết thúc nhưng cha chưa `wait()` đọc exit status → entry còn trong bảng process; dọn bằng `wait()/waitpid()`. Orphan: cha chết trước con → con được init/systemd (PID 1) nhận nuôi và reap.
</details>

#### OS-010 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Page fault có phải luôn là lỗi không?**
<details><summary>Đáp án</summary>

Không. Minor fault (page đã trong RAM nhưng chưa map, hoặc COW) chỉ cập nhật page table — nhanh; major fault (page trên disk/swap) phải đọc I/O — chậm; chỉ invalid fault (địa chỉ không hợp lệ) mới sinh SIGSEGV. Cơ chế này cho phép demand paging.
</details>

#### OS-011 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**TLB là gì và vì sao quan trọng cho hiệu năng?**
<details><summary>Đáp án</summary>

**Vấn đề TLB giải:** mỗi lần CPU chạm bộ nhớ, MMU phải dịch **địa chỉ ảo → vật lý** bằng cách tra **page table**. Page table là **multi-level** (x86-64: 4–5 tầng) nằm **trong RAM** → một lần dịch tốn **4–5 lần đọc RAM**. Nếu làm thật như vậy, mỗi lệnh `mov` sẽ chậm gấp năm.

**TLB (Translation Lookaside Buffer)** = **cache của các ánh xạ page→frame** nằm ngay trong MMU:

| | Chi phí |
|---|---|
| **TLB hit** | Dịch **tức thì** (~1 chu kỳ, song song với truy cập cache) |
| **TLB miss** | **Page table walk** — 4–5 lần đọc RAM (một số có thể tự hit cache), rồi nạp vào TLB |

TLB rất nhỏ (vài chục đến vài nghìn entry) nhưng nhờ **locality** vẫn đạt hit rate rất cao — nên hệ thống chạy được.

**Vì sao nó là mối bận tâm thật, không chỉ lý thuyết:**
- **Context switch process** → đổi page table → **flush TLB** (trừ khi có **ASID/PCID** gắn tag theo process) → giai đoạn sau switch toàn TLB miss. Đây là phần đắt của [OS-005](os.md).
- **Huge pages** (2MB thay vì 4KB) là kỹ thuật tối ưu trực diện: **một entry TLB phủ 512 lần nhiều bộ nhớ hơn** → giảm mạnh miss cho workload dữ liệu lớn (database, JVM heap). Nêu được ý này là điểm cộng rõ.
- Duyệt dữ liệu **rải rác trên nhiều page** có thể chậm không phải vì cache miss mà vì **TLB miss** — hai thứ khác nhau, đo bằng `perf stat -e dTLB-load-misses`.

**Bẫy:** (1) lẫn **TLB** với **cache dữ liệu** — TLB cache *ánh xạ địa chỉ*, cache CPU cache *nội dung*; một truy cập có thể TLB hit + cache miss hoặc ngược lại; (2) nói "switch process luôn flush TLB" — CPU hiện đại có ASID/PCID nên **không phải** flush toàn bộ; (3) quên rằng kernel cũng bị ảnh hưởng — mitigation Meltdown (**KPTI**) tách page table kernel/user, làm tăng TLB pressure và đó là lý do nó tốn hiệu năng.

**Chốt:** *"TLB là cache của bảng dịch địa chỉ. Miss không chỉ chậm một chút — nó biến một lần truy cập thành 4–5 lần đọc RAM. Vì thế flush TLB khi switch process là đắt, và huge pages là cách giảm áp lực TLB."*
</details>

#### OS-012 · 🟠 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md), [concurrency §7](../../../02-modern-cpp/concurrency.md)
**Condition variable: vì sao phải đi kèm mutex và predicate (dùng while)?**
<details><summary>Đáp án</summary>

**Cơ chế `wait()`** — ba bước, hiểu bước 1 là hiểu vì sao cần mutex:
1. **Nhả mutex** rồi đưa thread đi ngủ (blocked, **0% CPU** — không phải busy-wait). Nhả là bắt buộc: nếu ôm khoá mà ngủ thì không ai vào sửa được điều kiện → **deadlock**.
2. Ngủ tới khi bị đánh thức.
3. **Giành lại mutex** rồi mới trả về → khi `wait()` trả về, bạn đang giữ khoá và đọc predicate an toàn.

**Vì sao cần mutex** — hai lý do, lý do (2) mới là lý do sâu:
1. Predicate đọc **dữ liệu chung** (`queue`, cờ…) → phải bảo vệ như mọi critical section.
2. Cặp thao tác *"kiểm tra điều kiện → đi ngủ"* phải **nguyên tử**. Nếu không có mutex, `notify` của producer có thể lọt đúng vào khe giữa hai bước — consumer thấy điều kiện sai, rồi mới ngủ, trong khi tín hiệu đã bay qua → **lost wakeup**, ngủ vĩnh viễn. Mutex + `wait` nhả khoá *sau khi* đã vào hàng đợi chờ mới đóng được khe này.

**Vì sao `while` chứ không `if`** — `wait()` trả về **không** có nghĩa "điều kiện của bạn đã đúng", chỉ có nghĩa "có thể có gì đó đã đổi". Ba tình huống bắt nó trả về khi predicate vẫn sai:

| Tình huống | Bản chất |
|---|---|
| **Spurious wakeup** | OS/futex đánh thức mà **không ai** `notify` — chuẩn cho phép, không phải bug |
| **Stolen wakeup** *(thực tế gặp nhiều nhất)* | Có `notify` **thật**, nhưng thread khác giành lock trước và "ăn" mất dữ liệu → mình thức dậy thì queue rỗng lại. Điển hình: `notify_all()` với 3 consumer nhưng chỉ 1 item |
| **Lost wakeup** | Điều kiện đã đúng **trước khi** gọi `wait` → `while` kiểm tra trước nên không ngủ oan |

Dùng `if` thì khi thức dậy code **rơi thẳng xuống critical section với predicate sai** → `q.front()` trên queue rỗng = **UB** (thường không crash tại chỗ, chỉ đọc rác rồi hỏng dữ liệu về sau).

```cpp
// ✅ Dạng chuẩn — predicate overload CHÍNH LÀ vòng while, chuẩn định nghĩa vậy
std::unique_lock<std::mutex> lk(m);
cv.wait(lk, []{ return !q.empty(); });   // ⇔ while (q.empty()) cv.wait(lk);
int v = q.front(); q.pop();              // chắc chắn: pred đúng + đang giữ lock

// ❌ cv.wait(lk);              — overload 1 tham số KHÔNG lặp, không kiểm tra gì
// ❌ if (q.empty()) cv.wait(lk);
```

**Chốt:** mặc định dùng `cv.wait(lk, pred)` — nó vừa lặp, vừa kiểm tra predicate **trước khi ngủ lần đầu** (chống lost wakeup), và khiến bạn không thể viết nhầm thành `if`.
</details>

#### OS-013 · 🟠 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Copy-on-write trong fork hoạt động thế nào?**
<details><summary>Đáp án</summary>

**Ý tưởng: hoãn công việc tới lúc *thật sự* cần — và thường thì không bao giờ cần.**

**Cơ chế, từng bước:**
1. `fork()`: kernel **không** copy bộ nhớ. Nó copy **page table** và cho cha/con trỏ về **cùng các physical page**, nhưng đánh dấu **tất cả là read-only** (kể cả page vốn writable), kèm ref count.
2. Một bên **đọc** → bình thường, không có gì xảy ra. Đây là ca phổ biến nhất.
3. Một bên **ghi** → MMU thấy page read-only → sinh **page fault** (loại **minor**, không đụng disk).
4. Kernel xử lý fault: **nhân bản đúng page đó**, cho bên ghi bản riêng writable, giảm ref count. Các page còn lại vẫn chia sẻ.

**Vì sao đây là thiết kế quan trọng:** mẫu dùng phổ biến nhất của `fork` là **`fork` + `exec` ngay** (shell chạy lệnh). `exec` **thay toàn bộ** address space — nên nếu `fork` copy thật, 100% công sức đó bị vứt đi. Với COW, chi phí `fork` gần như chỉ là copy page table → nhanh hơn hàng trăm lần và tiết kiệm cả RAM.

**Bẫy:** (1) tưởng COW là "không bao giờ copy" — nó chỉ **hoãn**; process ghi nhiều thì cuối cùng vẫn copy, chỉ là **theo từng page và chỉ page cần**; (2) hệ quả đo lường: RSS của con nhìn "nhỏ" nhưng có thể phình bất ngờ khi bắt đầu ghi → **overcommit** rồi bị **OOM killer** dù `fork` đã thành công; đây là lý do `fork` một process chiếm 10GB rồi ghi dần vẫn có thể chết; (3) trong **hệ real-time / embedded**, COW đưa vào **latency không tất định** — lần ghi đầu vào mỗi page tốn một page fault; hệ RT dùng `mlockall()` hoặc `posix_spawn`/`vfork` để tránh; (4) `vfork` khác COW: nó **chia sẻ** address space và treo cha lại, chỉ an toàn khi `exec` ngay.

**Cùng cơ chế xuất hiện ở nơi khác:** `mmap(MAP_PRIVATE)`, dedupe RAM của KSM, snapshot của filesystem/container image — COW là một **mẫu chung**, không chỉ chuyện của `fork`.

**Chốt:** *"`fork` chỉ copy page table và khoá mọi page read-only; page nào bị ghi thì mới nhân bản page đó qua một minor fault. Nhanh vì mẫu dùng thật là fork rồi exec — copy sớm là copy thừa."*
</details>

#### OS-014 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Thrashing là gì?**
<details><summary>Đáp án</summary>

Khi tổng working set của các process vượt RAM, hệ thống liên tục swap in/out và dành phần lớn thời gian cho I/O thay vì tính toán → hiệu năng sụp đổ. Khắc phục: giảm tải, thêm RAM, hoặc OOM killer kết thúc process ngốn bộ nhớ.
</details>

#### OS-015 · 🔴 · concept · ⭐ · [→ scheduling](../../../03-operating-system/scheduling.md)
**Priority inversion là gì? Priority inheritance chữa thế nào, và vì sao semaphore không chữa được?**
<details><summary>Đáp án</summary>

- **Inversion:** L (thấp) giữ mutex; H (cao) chờ mutex đó; M (trung, không liên quan) chiếm CPU của L → L không chạy để nhả → H chờ vô chừng — ưu tiên lộn ngược. Sự cố nổi tiếng **Mars Pathfinder** reset liên tục vì kịch bản này.
- **Priority inheritance:** khi H block, chủ mutex (L) **tạm nhận priority của H** → vượt M, chạy xong nhả, tụt về cũ → thời gian H chờ chặn trên = critical section của L. (Biến thể: priority ceiling.)
- **Semaphore không PI được** vì PI cần biết **chủ để nâng**; semaphore không có ownership (A wait, B post đều hợp lệ). Đây là lý do kỹ thuật của quy tắc "mutex cho mutual exclusion, semaphore cho signaling" trong hệ RT.
- Thực chiến: pthread mutex phải khai `PTHREAD_PRIO_INHERIT` tường minh (mặc định NONE).
</details>

#### OS-016 · 🔴 · concept · [→ scheduling](../../../03-operating-system/scheduling.md)
**CFS lập lịch theo nguyên lý nào?**
<details><summary>Đáp án</summary>

CFS mô phỏng CPU đa nhiệm lý tưởng: theo dõi vruntime (thời gian CPU ảo có trọng số theo nice) của mỗi tác vụ và luôn chọn chạy tác vụ có vruntime nhỏ nhất (dùng cây đỏ-đen). Tác vụ nice thấp (ưu tiên cao) có vruntime tăng chậm nên được nhiều CPU hơn → chia sẻ công bằng theo trọng số.
</details>

#### OS-017 · 🔴 · design · [→ process-thread](../../../03-operating-system/process-thread.md)
**Khi nào dùng nhiều process thay vì nhiều thread cho một ứng dụng?**
<details><summary>Đáp án</summary>

Dùng process khi cần cô lập/độ tin cậy cao (một thành phần crash không kéo sập phần khác — vd browser tách tab), thành phần độc lập hoặc cần quyền hạn/bảo mật khác nhau, fault containment mạnh. Dùng thread khi cần chia sẻ dữ liệu lớn thường xuyên, song song trong cùng ứng dụng, giảm overhead. Đánh đổi: cô lập & an toàn vs nhẹ & chia sẻ nhanh.
</details>

#### OS-018 · 🔴 · design · [→ sync-primitives](../../../03-operating-system/sync-primitives.md), [concurrency](../../../02-modern-cpp/concurrency.md)
**Thiết kế producer–consumer an toàn giữa các thread thế nào?**
<details><summary>Đáp án</summary>

Dùng một hàng đợi chia sẻ được bảo vệ bằng mutex, cộng condition variable: consumer `cv.wait(lock, []{return !queue.empty();})` (nhả lock khi ngủ, predicate chống spurious wakeup); producer khóa mutex, push, rồi `notify_one`. Cân nhắc bounded queue (thêm cv cho "không đầy") để chống tràn bộ nhớ, và `notify_all` khi nhiều consumer. Trên hệ đơn giản có thể dùng lock-free queue + atomic nhưng khó đúng.
</details>

#### OS-019 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**`fork()` trả về gì?**
<details><summary>Đáp án</summary>

Trả **0 cho process con**, **PID của con cho process cha**, **-1 nếu lỗi**. Không gian địa chỉ được copy-on-write.
</details>

#### OS-020 · 🟡 · concept · ⭐ · [→ concurrency](../../../02-modern-cpp/concurrency.md)
**Thread-safe và reentrant khác nhau thế nào?**
<details><summary>Đáp án</summary>

Hai **câu hỏi khác nhau**, và chỗ hầu hết người trả lời gộp làm một:
- **Thread-safe:** nhiều **thread** gọi **đồng thời** có an toàn không?
- **Reentrant:** hàm bị **cắt ngang giữa chừng** rồi được gọi lại (từ signal handler, hoặc đệ quy) có an toàn không?

**Khác biệt then chốt nằm ở CÁCH đạt được:**
- Thread-safe **được dùng khoá** — bọc mutex là xong.
- Reentrant **KHÔNG được dùng khoá** — vì hàm bị cắt ngang khi đang giữ khoá rồi gọi lại chính nó sẽ **tự deadlock**. Reentrant chỉ đạt được bằng cách **không có trạng thái chia sẻ**.

⇒ **Reentrant ⇒ thread-safe, nhưng thread-safe ⇏ reentrant.**

Đây chính là lý do danh sách **async-signal-safe hẹp hơn nhiều** so với thread-safe: `printf` **là** thread-safe (glibc khoá stdio) nhưng **không** async-signal-safe — gọi từ handler khi luồng chính đang giữ khoá stdio là deadlock ([LNX-011](linux-sysprog.md)).

**Nguyên nhân điển hình khiến hàm không thread-safe:** dùng **biến toàn cục/static**. Đó là lý do có họ `_r` — `strtok_r`, `localtime_r`, `getpwnam_r`: thay vì trả con trỏ tới **bộ nhớ tĩnh dùng chung**, chúng nhận **buffer do caller cấp** → hết trạng thái chia sẻ → reentrant luôn.
</details>

#### OS-021 · 🟠 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Gọi `fork()` trong chương trình đa luồng thì chuyện gì xảy ra?**
<details><summary>Đáp án</summary>

**Chỉ thread gọi `fork()` tồn tại trong process con.** Mọi thread khác **biến mất** — nhưng **trạng thái chúng để lại thì vẫn nguyên**, và đó mới là vấn đề:

```
   TRƯỚC fork:  Thread A đang giữ mutex M, sửa dở một cấu trúc dữ liệu
                Thread B gọi fork()
        ▼
   CON:         chỉ còn Thread B
                M vẫn ĐANG BỊ KHOÁ — bởi một thread KHÔNG CÒN TỒN TẠI
                → không ai nhả được → ai khoá M sẽ treo VĨNH VIỄN
```

**Hệ quả nghiêm trọng nhất:** trong con, giữa `fork()` và `exec()`, chỉ được gọi hàm **async-signal-safe**. Đặc biệt **`malloc()` có thể deadlock** — nó dùng mutex nội bộ mà một thread khác có thể đang giữ đúng lúc `fork()`. Điều này bất ngờ với nhiều người vì `malloc` trông vô hại.

**Ba cách xử lý, theo thứ tự nên ưu tiên:**
1. **`exec()` ngay sau `fork()`** — `exec` thay toàn bộ không gian nhớ nên mọi trạng thái hỏng biến mất. Đơn giản và đáng tin nhất.
2. **`fork()` trước khi tạo thread nào** — process khởi động, `fork` hết worker cần thiết, *rồi* mỗi worker mới tạo thread của mình.
3. `pthread_atfork()` đăng ký handler khoá/nhả mutex quanh `fork` — đúng lý thuyết nhưng **rất khó làm đủ**, nhất là với mutex nằm trong thư viện bên thứ ba.

Liên quan: cũng vì vậy con nên gọi **`_exit()`** thay `exit()` — `exit()` chạy `atexit` và xả **bản sao buffer stdio của cha** → in trùng và phá tài nguyên chung.

**Chốt:** *"`fork()` trong chương trình đa luồng chỉ an toàn khi con `exec()` ngay."*
</details>

---
⬅️ [Bank index](README.md)
