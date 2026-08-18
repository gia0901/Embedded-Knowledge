# OS — Operating System

> Domain `OS`. Track dùng: `os`, `bsp`, `cpp-system`, `ostep`, `cpp-concurrency`.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Process & thread | 10 |
| **B** | Scheduling | 5 |
| **C** | Memory management | 6 |
| **D** | Đồng bộ hoá & deadlock | 7 |

---

## A — Process & thread

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

#### OS-019 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**`fork()` trả về gì?**
<details><summary>Đáp án</summary>

Trả **0 cho process con**, **PID của con cho process cha**, **-1 nếu lỗi**. Không gian địa chỉ được copy-on-write.
</details>

#### OS-022 · 🟢 · concept · 📦 2026-08-13 · [→ ipc](../../../03-operating-system/ipc.md)
**Hai thread trong cùng process trao đổi dữ liệu chỉ bằng một biến toàn cục. Hai process thì không làm vậy được. Vì sao — và cái "không được" đó nằm ở đâu?**
<details><summary>Đáp án</summary>

**Cơ chế: mỗi process có một bảng trang riêng.** Địa chỉ ảo `0x7fff1234` của process A và của process B ánh xạ tới **hai khung trang vật lý khác nhau**. Con trỏ của A đưa sang B là **vô nghĩa** — không phải bị cấm, mà là *chỉ sang chỗ khác*.

Thread thì ngược lại: chúng **dùng chung một bảng trang** (chung không gian địa chỉ, chung heap, chung biến toàn cục), chỉ riêng **stack** và thanh ghi. Nên "chia sẻ" với thread là **mặc định**, không cần cơ chế gì.

| | Thread cùng process | Hai process |
|---|---|---|
| Không gian địa chỉ | **Chung** | Riêng |
| Chia sẻ dữ liệu | Mặc định — chỉ cần biến chung | Phải có **cơ chế IPC** |
| Vấn đề phải lo | **Đồng bộ** (race) | **Vận chuyển** + đồng bộ |
| Một bên hỏng | Kéo cả process chết | Bên kia sống, có thể phát hiện |

⇒ **Đảo ngược đáng nhớ:** thread có sẵn cái khó của IPC (chia sẻ) nhưng mất cái lợi (cách ly lỗi). Chọn nhiều process **chính là** chọn trả giá bằng IPC để mua lấy sự cách ly ([OS-017](os.md)).

**Bẫy:** nói *"process không chia sẻ được bộ nhớ"* — **sai**, `mmap(MAP_SHARED)`/shared memory chính là việc **cố ý map hai bảng trang về cùng khung vật lý**. Đúng phải là: *không chia sẻ **mặc định**, phải yêu cầu tường minh*.

**Chốt:** *"Thread chung bảng trang nên chia sẻ là mặc định; process riêng bảng trang nên con trỏ mất nghĩa — IPC là cơ chế bắc cầu qua ranh giới đó."*
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

#### OS-009 · 🟡 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Zombie và orphan process là gì? Daemon chạy 3 tuần rồi `fork()` trả `-1` trong khi RAM còn nhiều — chuyện gì xảy ra?**
<details><summary>Đáp án</summary>

| | **Zombie** | **Orphan** |
|---|---|---|
| Là gì | Con **đã chết**, cha **chưa `wait()`** đọc exit status | Cha **chết trước**, con vẫn sống |
| Vì sao tồn tại | Kernel phải giữ exit status cho tới khi có người đọc | — |
| Nguy hiểm? | **Có, nếu tích tụ** | **Không** — PID 1 (init/systemd) nhận nuôi và reap tự động |
| Dọn bằng | `wait()`/`waitpid()` ở cha | Tự động |

**Trả lời tình huống:** zombie tích tụ ⇒ cạn **PID**, không phải cạn RAM. Mỗi zombie chỉ giữ một entry nhỏ trong bảng process (PID + exit status), nên `free`/`df` **không thấy gì bất thường** — đó chính là dấu hiệu nhận diện. Trần: `/proc/sys/kernel/pid_max`.

**Ba cách dọn — chọn theo *có cần exit status không*:**

| Cách | Khi nào | Đánh đổi |
|---|---|---|
| `waitpid()` đồng bộ | Cần biết kết quả ngay | **Chặn** cha |
| Handler `SIGCHLD` | Cần exit status, không muốn chặn | Phải **vét cạn bằng vòng lặp** — xem bẫy ① |
| `signal(SIGCHLD, SIG_IGN)` / `SA_NOCLDWAIT` | **Không cần** exit status | **Mất sạch** exit status |

**Bẫy:**
1. ⭐ **`SIGCHLD` KHÔNG xếp hàng.** Năm con thoát gần cùng lúc có thể chỉ sinh **một** lần gọi handler. Gọi `waitpid` một lần là **rò zombie** ⇒ bắt buộc:
   ```c
   void on_sigchld(int) {
       int saved = errno;                            // handler phải bảo toàn errno
       while (waitpid(-1, NULL, WNOHANG) > 0) { }    // ✅ vét cạn
       errno = saved;
   }
   ```
2. `kill -9` **không** diệt được zombie — nó đã chết rồi. Muốn dọn phải giết **cha** (khi đó PID 1 nhận nuôi và reap) hoặc sửa cha.
3. Nhầm zombie (`Z`) với `D` state — `D` là **đang sống và kẹt I/O**, triệu chứng và hướng điều tra hoàn toàn khác ([LNX-022](linux-sysprog.md)).

**Chốt:** *"Zombie ăn PID chứ không ăn RAM — nên triệu chứng là `fork()` lỗi trong khi `free` vẫn đẹp. Và `SIGCHLD` không xếp hàng nên handler phải vét cạn bằng vòng lặp."*
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

**Nguyên nhân điển hình khiến hàm không thread-safe:** dùng **biến toàn cục/static**. Đó là lý do có họ `_r` — `strtok_r`, `localtime_r`, `getpwnam_r`: thay vì trả con trỏ tới **bộ nhớ tĩnh dùng chung**, chúng nhận **buffer do caller cấp** → hết trạng thái chia sẻ.

---

### Ví dụ cụ thể — cùng một hàm, hai câu trả lời khác nhau

```cpp
std::mutex m;
int counter = 0;

void logEvent(const char* msg) {
    std::lock_guard<std::mutex> lk(m);     // ← khoá
    counter++;
    write(fd, msg, strlen(msg));
}
```
| | Trả lời | Vì sao |
|---|---|---|
| Thread-safe? | ✅ **Có** | mutex bảo vệ `counter` và thứ tự ghi |
| Reentrant / async-signal-safe? | ❌ **Không** | Luồng chính đang giữ `m` → signal chen ngang → handler gọi lại `logEvent` → `lock()` lần hai trên mutex **không đệ quy** → **tự deadlock**. Chương trình treo, không crash, không log gì |

**Bản dùng được trong signal handler:**
```cpp
volatile sig_atomic_t counter = 0;          // kiểu chuẩn cho dữ liệu chia sẻ với handler

void logEventSafe(const char* msg, size_t len) {
    counter++;                              // không khoá
    write(STDERR_FILENO, msg, len);         // ✅ write LÀ async-signal-safe
}
```
Bỏ mutex, bỏ mọi thứ đụng trạng thái toàn cục. Truyền sẵn `len` để khỏi phụ thuộc `strlen`.

### ⚠️ Hai hiểu lầm phải tránh

**① `write()` KHÔNG phải hàm cấm.** Nó nằm trong danh sách async-signal-safe của POSIX (`man 7 signal-safety`) vì là **syscall trần** — không khoá, không buffer trong userspace.

| ✅ Async-signal-safe | ❌ KHÔNG (dù thread-safe) | Vì sao cấm |
|---|---|---|
| `write`, `read`, `open`, `close` | `printf`, `fprintf`, `puts` | buffer stdio + khoá stdio |
| `_exit`, `kill`, `signal`, `sigaction` | `malloc`, `free`, `new` | khoá heap — handler chen giữa lúc heap dở dang |
| `time`, `sem_post` | `localtime`, `strerror` | trả con trỏ tới **static buffer** |

**② Họ `_r` giải quyết THREAD-SAFETY, không phải SIGNAL-SAFETY.** Hai tính chất giao nhau nhưng khác nhau — `localtime_r` thread-safe nhưng vẫn **không** nằm trong danh sách async-signal-safe. Đừng suy ra "cứ `_r` là gọi được trong handler".

**Nguyên tắc tự suy khi không nhớ danh sách:** hàm nào **lấy khoá** hoặc **đụng trạng thái toàn cục có thể đang dở dang** thì cấm. Handler chen vào **giữa** một thao tác của luồng chính — mọi bất biến đang xây dở đều có thể bị nhìn thấy.

**Mẫu thực chiến:** handler chỉ làm việc tối thiểu — đặt một cờ `volatile sig_atomic_t` hoặc `write()` một byte vào **self-pipe** — rồi để vòng lặp chính (event loop) xử lý phần nặng. Đây là *self-pipe trick*, xem [LNX-011](linux-sysprog.md).

**Chốt:** *"Thread-safe được phép dùng khoá; reentrant thì không — vì kẻ chen ngang chính là mình. Vì thế danh sách signal-safe hẹp hơn hẳn, và `_r` không phải là vé vào cửa."*
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

#### OS-017 · 🔴 · design · [→ process-thread](../../../03-operating-system/process-thread.md)
**Khi nào dùng nhiều process thay vì nhiều thread cho một ứng dụng?**
<details><summary>Đáp án</summary>

Dùng process khi cần cô lập/độ tin cậy cao (một thành phần crash không kéo sập phần khác — vd browser tách tab), thành phần độc lập hoặc cần quyền hạn/bảo mật khác nhau, fault containment mạnh. Dùng thread khi cần chia sẻ dữ liệu lớn thường xuyên, song song trong cùng ứng dụng, giảm overhead. Đánh đổi: cô lập & an toàn vs nhẹ & chia sẻ nhanh.
</details>

---

## B — Scheduling

#### OS-025 · 🟡 · concept · 📦 2026-08-13 · [→ scheduling](../../../03-operating-system/scheduling.md)
**CPU-bound và I/O-bound khác nhau ra sao? Scheduler nên đối xử với chúng thế nào, và vì sao?**
<details><summary>Đáp án</summary>

| | CPU-bound | I/O-bound |
|---|---|---|
| Dùng hết time slice? | **Có** — bị cắt ngang | **Không** — tự nhường sớm để chờ I/O |
| Thời gian chạy mỗi lượt | Dài | Rất ngắn (µs) rồi ngủ |
| Cái nó cần | **Thông lượng** | **Độ trễ thấp** |
| Ví dụ | giải mã ảnh, nén | bàn phím, mạng, đọc cảm biến |

**Vì sao scheduler nên ƯU TIÊN tiến trình I/O-bound** — nghịch lý ban đầu, nhưng đúng:

Tiến trình I/O-bound chạy **rất ngắn** rồi lại đi ngủ. Cho nó chạy trước thì nó **tốn gần như không CPU** mà lại **kịp phát lệnh I/O sớm** ⇒ thiết bị (đĩa, mạng) được giữ bận, tức là **song song hoá CPU với I/O**. Bắt nó xếp hàng sau một tác vụ CPU-bound thì thiết bị nằm không, và người dùng thấy giật.

⇒ Đó là lý do scheduler cổ điển **thưởng cho tiến trình hay ngủ**: Linux O(1) trước kia cộng bonus theo thời gian ngủ; **CFS** ngày nay đạt cùng hiệu ứng một cách tự nhiên — tiến trình ngủ nhiều thì `vruntime` **tăng chậm**, nên khi thức dậy nó ở đầu hàng ([OS-016](os.md)).

**Bẫy:** (1) tưởng ưu tiên I/O-bound là "thiên vị" — thực ra nó **tăng tổng thông lượng hệ thống**, không chỉ tăng tính đáp ứng; (2) một tiến trình **không cố định** một loại — nó đổi pha (đọc file rồi xử lý rồi lại đọc), nên scheduler phải đo **hành vi gần đây** chứ không dán nhãn một lần; (3) nhầm với **priority**: đây là cơ chế **tự động theo hành vi**, không phải `nice` do người đặt.

**Chốt:** *"I/O-bound chạy ngắn rồi ngủ, nên cho nó chạy trước gần như miễn phí mà lại giữ được thiết bị luôn bận — ưu tiên nó là tăng thông lượng, không phải thiên vị."*
</details>

#### OS-027 · 🟡 · concept · 📦 2026-08-13 · [→ scheduling](../../../03-operating-system/scheduling.md)
**Starvation là gì? Aging giải quyết ra sao — và nó khác deadlock chỗ nào?**
<details><summary>Đáp án</summary>

**Starvation:** một task **không bao giờ** (hoặc rất lâu mới) được tài nguyên, dù hệ thống vẫn đang tiến triển bình thường. Nguyên nhân điển hình: ưu tiên cố định và luôn có task ưu tiên cao hơn sẵn sàng; hoặc reader–writer lock mà reader tới liên tục ⇒ **writer starvation**.

**Phân biệt với deadlock — hay bị hỏi kèm:**

| | Deadlock | Starvation | Livelock |
|---|---|---|---|
| Hệ thống có tiến triển? | ❌ **Đứng hẳn** | ✅ Có — chỉ *một số* bị bỏ đói | ❌ Không, dù **đang chạy** |
| Tự khỏi được? | Không bao giờ | **Có thể** (khi tải giảm) | Có thể |
| Chữa bằng | Phá điều kiện Coffman ([OS-003](os.md)) | **Aging / fairness** | Ngẫu nhiên hoá lùi (backoff) |

**Aging:** ưu tiên **hiệu dụng** của task tăng dần theo thời gian nó phải chờ. Chờ đủ lâu thì nó vượt lên trên và được phục vụ ⇒ **chặn trên thời gian chờ**, biến "có thể không bao giờ" thành "chậm nhất là X".

**Trong thực tế Linux:** CFS gần như miễn nhiễm starvation ở lớp `SCHED_OTHER` — nó lập lịch theo `vruntime`, task chờ lâu có `vruntime` thấp nên **tự động** được chọn. `nice` chỉ đổi **tốc độ tăng** `vruntime`, không tạo ưu tiên tuyệt đối. ⇒ **Starvation thật sự nguy hiểm nằm ở lớp realtime**, nơi ưu tiên là **tuyệt đối** và không có aging.

**Bẫy:** (1) gọi mọi cái chậm là starvation — phải phân biệt *chậm vì tải* với *không bao giờ tới lượt*; (2) quên rằng starvation cũng xảy ra ở **khoá**, không chỉ ở CPU (writer starvation); (3) tưởng tăng ưu tiên là chữa được — thường chỉ **dời** nạn đói sang task khác.

**Chốt:** *"Deadlock là cả hệ đứng; starvation là hệ vẫn chạy nhưng có kẻ không bao giờ tới lượt. Aging đặt trần cho thời gian chờ — và đó chính là thứ lớp realtime KHÔNG có."*
</details>

#### OS-026 · 🟠 · concept · ⭐ · 📦 2026-08-13 · [→ scheduling](../../../03-operating-system/scheduling.md)
**Linux có những scheduling class nào? Task đọc cảm biến của bạn thỉnh thoảng trễ 40 ms — có nên chuyển nó sang realtime không?**
<details><summary>Đáp án</summary>

**Các lớp, theo thứ tự ưu tiên tuyệt đối** (lớp trên còn task chạy được thì lớp dưới **không bao giờ** được CPU):

| Lớp | Policy | Dùng cho | Ghi chú |
|---|---|---|---|
| **Deadline** | `SCHED_DEADLINE` | Task chu kỳ có hạn chót rõ | Khai báo *runtime/period/deadline*, kernel **từ chối** nếu quá tải |
| **Realtime** | `SCHED_FIFO`, `SCHED_RR` | Điều khiển, audio, motion | Ưu tiên **1–99**. FIFO chạy tới khi tự nhường; RR có time slice |
| **Normal** ⭐ | `SCHED_OTHER` (CFS) | Gần như mọi thứ | Chia sẻ công bằng, chỉnh bằng `nice` |
| **Idle** | `SCHED_IDLE`, `SCHED_BATCH` | Việc nền, không cần đáp ứng | Chỉ chạy lúc rảnh |

**Trả lời câu hỏi thật — "có nên đổi sang realtime không?": KHÔNG, chưa phải bây giờ.** Trước hết phải biết **40 ms đó đến từ đâu**, vì realtime chỉ chữa **một** trong các nguyên nhân:

| Nguyên nhân thật | `SCHED_FIFO` có chữa được? |
|---|---|
| Bị task khác giành CPU | ✅ Có |
| Kẹt chờ I/O / khoá (`D` state) | ❌ **Không** — ưu tiên cao vẫn phải chờ |
| Page fault, hoán đổi trang | ❌ Không (chữa bằng `mlockall`) |
| Bị **priority inversion** | ❌ **Tệ hơn** — xem dưới |
| Tần số CPU/idle state chuyển chậm | ❌ Không (chữa bằng chính sách nguồn) |

⚠️ **Vì sao realtime nguy hiểm:** `SCHED_FIFO` **chạy tới khi nó tự nhường**. Một vòng lặp bận ở ưu tiên 50 sẽ **treo cứng cả máy** — kể cả shell của bạn. Và nó khuếch đại **priority inversion**: task RT chờ khoá do task thường giữ, mà task thường không bao giờ được lịch để nhả khoá ⇒ kẹt vĩnh viễn ([OS-015](os.md)). Dùng RT thì **bắt buộc** kèm mutex có **priority inheritance**.

**Trình tự đúng:** đo xem trễ đến từ đâu → nếu đúng là bị giành CPU thì mới `SCHED_FIFO` **ưu tiên thấp nhất đủ dùng**, kèm `mlockall()` (khoá trang, tránh page fault), tách CPU riêng cho task đó nếu cần, và **luôn để một lối thoát** (`RLIMIT_RTTIME` hoặc watchdog).

**Chốt:** *"Realtime không làm code nhanh hơn — nó chỉ thay đổi ai được CPU trước. Trễ do chờ I/O, page fault hay inversion thì đổi lớp lịch không cứu được, mà còn dễ treo máy."*
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
**CFS lập lịch theo nguyên lý nào? Vì sao nó không cần "ưu tiên" hay "bonus cho tiến trình I/O" như scheduler cổ điển?**
<details><summary>Đáp án</summary>

**Ý tưởng gốc:** mô phỏng một **CPU đa nhiệm lý tưởng** — nơi N tác vụ mỗi cái chạy đồng thời ở tốc độ 1/N. Không thể làm thật, nên CFS **đo độ lệch** so với lý tưởng đó và luôn ưu tiên kẻ đang **thiệt nhất**.

**Cơ chế — chỉ một con số: `vruntime`.**
- Mỗi tác vụ có `vruntime` = thời gian CPU đã dùng, **chia cho trọng số** (trọng số suy từ `nice`).
- Scheduler **luôn chọn tác vụ có `vruntime` nhỏ nhất** (giữ trong cây đỏ–đen, lấy phần tử trái nhất — O(log n)).
- Chạy xong một lượt thì `vruntime` tăng, tự động tụt xuống dưới trong hàng.

⇒ **`nice` không tạo ưu tiên tuyệt đối** — nó chỉ đổi **tốc độ tăng** `vruntime`. Tác vụ `nice` thấp có `vruntime` tăng chậm nên được chọn thường xuyên hơn, nhưng tác vụ `nice` cao **vẫn luôn tới lượt**. Đây là lý do lớp `SCHED_OTHER` gần như **miễn nhiễm starvation** ([OS-027](os.md)).

**Vì sao không cần "bonus cho tiến trình I/O"** — chỗ hay được hỏi và là điểm đẹp nhất của thiết kế:

Scheduler cổ điển (Linux O(1)) phải **đoán** tiến trình nào là I/O-bound rồi cộng thưởng — heuristic phức tạp và hay đoán sai. CFS **không cần đoán**: tiến trình ngủ chờ I/O thì **không tiêu CPU**, nên `vruntime` của nó **đứng yên** trong lúc mọi tiến trình khác tăng. Khi nó thức dậy, `vruntime` của nó nhỏ nhất ⇒ **được chạy ngay**. Tính đáp ứng tốt là **hệ quả tự nhiên** của phép đo, không phải luật đặc biệt ([OS-025](os.md)).

⚠️ Nhưng phải chặn: tiến trình ngủ **rất lâu** (hàng giờ) mà thức dậy với `vruntime` cũ thì sẽ **độc chiếm CPU** để "đòi nợ". CFS chặn bằng cách kéo `vruntime` của tác vụ vừa thức lên gần `min_vruntime` hiện tại của hàng — chỉ cho nó lợi thế **vừa đủ**.

**Bẫy:** (1) nói *"CFS chia CPU đều"* — đúng phải là **công bằng theo trọng số**; (2) tưởng `nice` là ưu tiên như realtime — không, nó chỉ đổi tỉ lệ, và **mọi** tác vụ `SCHED_OTHER` đều thua **mọi** tác vụ realtime bất kể `nice` ([OS-026](os.md)); (3) tưởng CFS đảm bảo độ trễ — **không**; cần đảm bảo thì phải sang `SCHED_FIFO`/`SCHED_DEADLINE`.

**Chốt:** *"CFS chỉ theo dõi một con số — vruntime — và luôn chọn kẻ thiệt nhất. Tính đáp ứng cho tiến trình I/O là hệ quả tự nhiên (ngủ thì vruntime đứng yên), không phải một luật riêng."*
</details>

---

## C — Memory management

#### OS-008 · 🟡 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Virtual memory giải quyết vấn đề gì? Nêu ba vấn đề nó chữa, và cái giá phải trả.**
<details><summary>Đáp án</summary>

**Cơ chế:** mỗi process có không gian địa chỉ **ảo** riêng, MMU dịch sang vật lý qua page table ([OS-023](os.md)). Chương trình **không bao giờ** thấy địa chỉ vật lý.

**Ba vấn đề nó chữa — và nếu không có thì hỏng thế nào:**

| Vấn đề | Không có VM thì | VM chữa bằng |
|---|---|---|
| **Cô lập & bảo vệ** | Process A ghi bậy vào vùng của B, hỏng **im lặng** | Địa chỉ của A **không tồn tại** trong bảng của B ⇒ ghi bậy thành SIGSEGV, chết đúng chỗ |
| **Phân mảnh ngoài** | Còn 100 MB trống nhưng rời rạc ⇒ không cấp nổi khối 50 MB liền | Vật lý rời rạc, **ảo vẫn liền mạch** — chỉ cần đủ số khung |
| **Bộ nhớ > RAM** | Chương trình phải vừa RAM thật | Demand paging + swap: chỉ nạp trang **thực sự chạm tới** |

**Ba thứ "miễn phí" mọc ra từ đây** — hay bị quên nhưng rất hay được hỏi: **`fork()` rẻ** nhờ copy-on-write ([OS-013](os.md)) · **`mmap` file** biến đọc file thành truy cập bộ nhớ · **shared library** map cùng một bản `.so` vào nhiều process, chỉ tốn RAM một lần.

**Cái giá phải trả:**
- **Độ trễ bất định** — một lệnh gán bình thường có thể sinh **major fault** và chờ I/O hàng ms. Đây là lý do hệ **realtime** phải `mlockall()` khoá trang, và lý do MCU không MMU lại **tất định hơn** ([OS-024](os.md)).
- Chi phí tra bảng (đỡ bởi TLB) và **TLB flush khi context switch process** — chính là lý do switch process đắt hơn switch thread ([OS-005](os.md)).
- **Overcommit**: Linux hứa nhiều hơn nó có; tới lúc chạm thật mà hết RAM thì **OOM killer** giết process — `malloc` trả về non-NULL không có nghĩa bộ nhớ đó chắc chắn dùng được.

**Bẫy:** (1) nói VM = swap — **sai**, swap chỉ là *một* ứng dụng; máy tắt swap vẫn có đầy đủ virtual memory; (2) tưởng mỗi process tốn RAM bằng kích thước không gian ảo — chỉ trang **đã chạm** mới tốn khung thật.

**Chốt:** *"Virtual memory mua cô lập + chống phân mảnh + chạy quá RAM, trả bằng độ trễ bất định — đó là lý do realtime phải khoá trang và MCU không MMU lại dễ tất định hơn."*
</details>

#### OS-023 · 🟡 · concept · 📦 2026-08-13 · [→ memory-management](../../../03-operating-system/memory-management.md)
**Paging hoạt động thế nào? Page table là gì, và vì sao tra nó lại không giết chết hiệu năng?**
<details><summary>Đáp án</summary>

**Cơ chế:** bộ nhớ được chia thành **trang** (page) cỡ cố định — thường **4 KB**. Bộ nhớ vật lý chia thành **khung** (frame) cùng cỡ. **Page table** là bảng ánh xạ *số trang ảo → số khung vật lý*, mỗi process một bảng, do **MMU** tra tự động ở mỗi lần truy cập bộ nhớ.

Địa chỉ ảo tách làm hai phần:
```
   địa chỉ ảo 48-bit          ┌── số trang ảo ──┐┌─ offset ─┐
                              │    36 bit       ││  12 bit  │   (4 KB = 2^12)
   page table:  số trang ảo ──► số khung vật lý
   địa chỉ vật lý           =  số khung  ×  4096  +  offset
```
Offset **không đổi** khi dịch — chỉ phần số trang được tra bảng.

**Vấn đề và cách giải — đây là phần ăn điểm:**

| Vấn đề | Giải pháp | Vì sao hiệu quả |
|---|---|---|
| Bảng phẳng cho 48-bit quá to (hàng trăm GB/process) | **Page table nhiều tầng** (x86-64: 4–5 tầng) | Chỉ cấp phát nhánh **thực sự dùng**; vùng địa chỉ trống không tốn gì |
| Tra bảng nhiều tầng = **4 lần đọc RAM** cho *mỗi* truy cập | **TLB** — cache các ánh xạ vừa dùng | Hit rate thực tế **>99%** ⇒ chi phí trung bình gần bằng 0 ([OS-011](os.md)) |
| Trang chưa nạp / chưa cấp | **Page fault** → kernel nạp rồi cho chạy lại lệnh | Cho phép demand paging, COW, mmap ([OS-010](os.md)) |

**Vì sao 4 KB:** trang nhỏ ⇒ ít lãng phí trong trang cuối (internal fragmentation) nhưng bảng to và TLB phủ ít; trang lớn thì ngược lại. Đó là lý do có **huge page** (2 MB) cho tải làm việc trên vùng nhớ lớn — một mục TLB phủ được 2 MB thay vì 4 KB.

**Bẫy:** (1) tưởng page table nằm trong MMU — **không**, nó nằm trong RAM; MMU chỉ có thanh ghi trỏ tới gốc bảng (đổi bảng này = một phần của context switch process); (2) tưởng mỗi truy cập đều tra bảng — thực tế **TLB đỡ gần hết**; (3) quên rằng đổi process ⇒ **TLB có thể phải xoá**, đây chính là lý do context switch process đắt hơn thread ([OS-005](os.md)).

**Chốt:** *"Paging ánh xạ trang ảo → khung vật lý qua bảng nhiều tầng trong RAM; TLB làm cho việc tra đó gần như miễn phí, và page fault là chỗ kernel chen vào để làm demand paging."*
</details>

#### OS-010 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Page fault có phải luôn là lỗi không? Ứng dụng của bạn chạy mượt trên máy dev nhưng giật từng cơn trên thiết bị — page fault liên quan thế nào?**
<details><summary>Đáp án</summary>

**Không — phần lớn page fault là hoạt động BÌNH THƯỜNG.** "Fault" ở đây nghĩa là *"MMU không dịch được, mời kernel vào xử lý"*, không phải *"chương trình sai"*.

| Loại | Chuyện gì xảy ra | Chi phí | Có phải lỗi? |
|---|---|---|---|
| **Minor** | Trang **đã ở RAM** nhưng chưa map vào process này (thư viện dùng chung, COW, `mmap` vừa chạm lần đầu) — kernel chỉ **sửa page table** | **~µs** | ❌ Bình thường |
| **Major** | Trang **không ở RAM** — phải đọc từ đĩa/swap | **~ms** (chậm hơn **1.000×**) | ❌ Bình thường, nhưng **đắt** |
| **Invalid** | Địa chỉ không hợp lệ / sai quyền | — | ✅ **SIGSEGV** |

**Trả lời tình huống — vì sao dev mượt mà thiết bị giật:** máy dev có nhiều RAM và page cache ấm ⇒ gần như chỉ **minor** fault. Thiết bị RAM ít, lưu trữ chậm (eMMC/SD) ⇒ **major** fault thật, mỗi cái ~ms. Giật *"từng cơn"* là chữ ký của major fault theo cụm — nhất là ngay sau khi khởi động hoặc sau khi một tiến trình khác vừa đẩy trang của bạn ra.

⇒ **Đây là lớp bug không thể tái hiện trên máy dev.** Muốn đo phải nhìn **tỉ lệ major/minor**, không nhìn tổng số fault.

**Cách chữa, theo thứ tự:**
1. **Giảm working set** — vấn đề thật thường là dùng quá nhiều bộ nhớ, không phải paging.
2. **`mlockall(MCL_CURRENT|MCL_FUTURE)`** — khoá trang trong RAM cho tiến trình cần tất định (realtime, điều khiển). Đổi lại: chiếm RAM cứng, và **phải cấp phát trước** (touch hết) vì `mlockall` không cứu được trang chưa từng chạm.
3. **Chạm trước (pre-fault)** vùng nhớ + stack lúc khởi động, thay vì để fault xảy ra giữa vòng điều khiển.
4. Cân nhắc **tắt swap** trên thiết bị nhúng — thà OOM sớm và rõ còn hơn giật ngẫu nhiên.

**Bẫy:** (1) thấy số page fault lớn rồi hoảng — **minor fault nhiều là bình thường**, mỗi lần chạm trang mới đều tính; (2) tưởng `malloc` xong là có RAM — Linux chỉ cấp **địa chỉ ảo**, RAM thật đến ở lần **chạm đầu tiên** (một minor fault), nên thời điểm tốn RAM khác thời điểm gọi `malloc`; (3) quên rằng **stack cũng fault** — hàm đệ quy sâu lần đầu chạm stack mới cũng sinh fault.

**Chốt:** *"Page fault là cơ chế, không phải lỗi — chỉ invalid mới là lỗi. Cái cần đo là tỉ lệ MAJOR, vì nó chậm hơn minor cả nghìn lần và chỉ lộ ra trên thiết bị RAM ít."*
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

#### OS-014 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Swap là gì? Thrashing xảy ra khi nào — và vì sao nó là "vách đá" chứ không phải "dốc thoải"?**
<details><summary>Đáp án</summary>

**Swap:** vùng trên đĩa dùng làm chỗ chứa tạm cho trang bị đẩy khỏi RAM. Nó **mở rộng** bộ nhớ khả dụng, đổi lại tốc độ: RAM ~**100 ns**, đĩa ~**ms** — chênh khoảng **10.000×**.

**Working set:** tập trang mà process **thực sự** chạm tới trong một khoảng thời gian gần. Đây mới là con số quyết định, **không phải** tổng bộ nhớ đã cấp.

**Thrashing:** khi tổng working set của các process **vượt RAM**, mỗi lần nạp một trang lại phải đẩy một trang **đang cần** ra — và trang vừa bị đẩy lập tức được đòi lại. Hệ dành gần hết thời gian cho I/O paging thay vì tính toán.

**Vì sao là VÁCH ĐÁ, không phải dốc thoải** — đây là phần ăn điểm:

```
hiệu năng
   │████████████████████▓▓▒▒
   │                        ░
   │                         ░░░░░░░░░  ← sụp đổ
   └──────────────────────┬──────────► tải bộ nhớ
                    working set ≈ RAM
```
Vì nó là **vòng phản hồi dương**: thiếu RAM → paging nhiều hơn → tiến trình chạy chậm hơn → **giữ trang lâu hơn** → càng thiếu RAM. Qua điểm gãy thì hệ **không tự hồi phục** dù tải không tăng thêm. Triệu chứng kinh điển: **CPU idle cao** nhưng máy như chết đứng, load average vọt lên (đúng lớp dấu hiệu của [LNX-022](linux-sysprog.md) — nhiều tiến trình ở `D` state chờ I/O).

**Cách chữa, theo thứ tự:**
1. **Giảm working set** — đây mới là gốc; thêm RAM chỉ dời điểm gãy.
2. Đặt **giới hạn bộ nhớ** (cgroup) để một tiến trình xấu không kéo cả máy xuống — thà nó bị OOM sớm.
3. **OOM killer** là cơ chế cứu hộ, không phải giải pháp: nó giết theo `oom_score` và có thể **giết nhầm** tiến trình quan trọng.
4. Trên **thiết bị nhúng: cân nhắc tắt swap hẳn.** Thà OOM sớm và rõ ràng còn hơn thiết bị "sống mà không phản hồi" — với lưu trữ flash thì thrashing còn **mài mòn** thiết bị.

**Bẫy:** (1) tưởng thêm swap là chữa được — swap nhiều chỉ khiến hệ **thrash lâu hơn trước khi chết**, nhiều khi tệ hơn OOM ngay; (2) nhìn "RAM còn trống ít" rồi hoảng — Linux **cố ý** dùng RAM trống làm page cache, con số cần nhìn là *available*, không phải *free*; (3) nhầm swap-out bình thường (trang lạnh bị đẩy ra, vô hại) với thrashing (trang **nóng** bị đẩy ra rồi đòi lại ngay).

**Chốt:** *"Thrashing là vòng phản hồi dương chứ không phải suy giảm tuyến tính — nên nó sụp đổ đột ngột và không tự hồi. Dấu hiệu: CPU rảnh mà máy đứng."*
</details>

#### OS-024 · 🟠 · concept · ⭐ · 📦 2026-08-13 · [→ memory-management](../../../03-operating-system/memory-management.md)
**Hệ thống nhúng KHÔNG có MMU (MCU chạy bare-metal/RTOS) khác gì? Điều đó đổi cách bạn viết code ra sao?**
<details><summary>Đáp án</summary>

**Không MMU nghĩa là: địa chỉ chương trình thấy CHÍNH LÀ địa chỉ vật lý.** Mất luôn toàn bộ tầng dịch — và mất theo cả những thứ ta quen coi là miễn phí:

| Có MMU (Linux) | Không MMU (MCU/RTOS) |
|---|---|
| Mỗi process một không gian riêng | **Một không gian phẳng duy nhất**, mọi task nhìn thấy nhau |
| Con trỏ hỏng → **SIGSEGV**, chết một process | Con trỏ hỏng → **ghi đè lung tung**, hỏng im lặng ở chỗ khác |
| Bộ nhớ ảo liên tục dù vật lý rời rạc | **Phân mảnh là vĩnh viễn** — không gom lại được |
| Demand paging, swap, COW | Không có |
| `fork()` | Không có (µClinux dùng `vfork`) |

**Ba hệ quả đổi cách viết code — đây là phần được chấm:**

1. **Không có lưới an toàn cho con trỏ.** Ghi tràn mảng không crash ngay; nó **sửa biến của module khác**, và triệu chứng xuất hiện cách chỗ lỗi rất xa. ⇒ Phải bù bằng: **MPU** (nếu có — bảo vệ vùng, không dịch địa chỉ), canary quanh buffer, kiểm biên chủ động, review kỹ thay vì dựa vào crash.
2. **Cấp phát động trở nên nguy hiểm.** Không nén được phân mảnh ⇒ chạy vài tuần rồi `malloc` trả `NULL` dù tổng bộ nhớ trống còn nhiều. ⇒ **Cấp phát tĩnh hoặc pool cỡ cố định**, cấp hết lúc khởi động, **cấm cấp phát trong đường nóng** ([SD-016](system-design.md)).
3. **Tràn stack không được báo.** Có MMU thì guard page bắt được; không có thì stack task này **đâm thẳng vào** dữ liệu task khác. ⇒ Tô mẫu (pattern fill) lên stack lúc khởi động rồi **đo mức dùng cao nhất**, đặt watermark, bật MPU cho vùng stack nếu có.

⇒ **Đổi lại được gì:** tất định. Không page fault ⇒ **không có độ trễ bất ngờ** — đúng thứ hệ realtime cứng cần. Nhanh hơn và rẻ hơn.

**Bẫy:** tưởng "không MMU = không bảo vệ được gì" — nhiều Cortex-M có **MPU**: không dịch địa chỉ nhưng **chặn được truy cập trái vùng**, đủ để biến hỏng-im-lặng thành fault-ngay-tại-chỗ.

**Chốt:** *"Không MMU thì con trỏ hỏng không crash mà sửa dữ liệu của người khác, và phân mảnh là vĩnh viễn — nên đổi sang cấp phát tĩnh/pool và dựa vào MPU thay vì dựa vào crash."*
</details>

---

## D — Đồng bộ hoá & deadlock

#### OS-004 · 🟢 · concept · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Race condition là gì?**
<details><summary>Đáp án</summary>

Kết quả phụ thuộc thứ tự/timing không kiểm soát giữa nhiều luồng truy cập dữ liệu chung (ít nhất một ghi). Vùng cần loại trừ lẫn nhau là critical section; bảo vệ bằng mutex hoặc atomic.
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

#### OS-028 · 🟡 · concept · 📦 2026-08-13 · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Reader–writer lock dùng khi nào? Rủi ro gì — và vì sao nhiều khi mutex thường lại nhanh hơn?**
<details><summary>Đáp án</summary>

**Ý tưởng:** đọc không xung đột với đọc ⇒ cho **nhiều reader song song**, nhưng writer phải **độc quyền**.

```cpp
std::shared_mutex sm;
{ std::shared_lock lk(sm); /* nhiều reader cùng vào */ }
{ std::unique_lock lk(sm); /* writer độc quyền */ }
```

**Ba rủi ro, theo thứ tự hay gặp:**

1. **Writer starvation** — reader tới liên tục thì writer chờ mãi ([OS-027](os.md)). Cài đặt "reader ưu tiên" bị bệnh này; "writer ưu tiên" thì ngược lại làm reader chờ. Không có lựa chọn miễn phí.
2. ⭐ **Thường CHẬM HƠN mutex thường.** Đây là chỗ ít người nói được: rwlock phải **đếm số reader**, mà biến đếm đó là **một ô nhớ ghi chung** ⇒ mọi reader đều **ghi** vào cùng cache line ⇒ cache line **bật qua lại giữa các core**. Reader tưởng là "chỉ đọc" nhưng vẫn tạo tranh chấp ghi. Với critical section **ngắn**, chi phí này **át hết** phần song song thu được.
3. **Không đệ quy và không nâng cấp được.** Đang giữ shared lock mà muốn đổi sang unique (upgrade) là **deadlock** ở hầu hết cài đặt — phải nhả rồi lấy lại, và giữa hai bước đó dữ liệu có thể đã đổi.

**Khi nào rwlock thực sự thắng — cả ba điều kiện phải đúng:**
- Tỉ lệ đọc **áp đảo** (≫ 90%),
- **và** critical section **đủ dài** để phần song song bù được chi phí đếm,
- **và** thực sự có nhiều core cùng đọc.

**Nếu không đủ ba điều kiện, các lựa chọn thường tốt hơn:** `std::mutex` thường (đơn giản, dự đoán được) · **copy-on-write / publish** qua `shared_ptr` atomic — reader **không khoá gì cả** ([CPP-024](cpp.md)) · dữ liệu bất biến, thay nguyên bản mới.

**Bẫy:** dùng rwlock vì *"đọc nhiều nên chắc nhanh hơn"* mà **không đo** — đây là một trong những tối ưu hoá hay phản tác dụng nhất.

**Chốt:** *"rwlock chỉ thắng khi đọc áp đảo VÀ critical section đủ dài — vì bản thân việc đếm reader đã là một phép ghi chung gây tranh chấp cache. Không đo thì đừng đổi."*
</details>

---
⬅️ [Bank index](README.md)

#### OS-018 · 🔴 · design · [→ sync-primitives](../../../03-operating-system/sync-primitives.md), [concurrency](../../../02-modern-cpp/concurrency.md)
**Thiết kế producer–consumer an toàn giữa các thread thế nào?**
<details><summary>Đáp án</summary>

Dùng một hàng đợi chia sẻ được bảo vệ bằng mutex, cộng condition variable: consumer `cv.wait(lock, []{return !queue.empty();})` (nhả lock khi ngủ, predicate chống spurious wakeup); producer khóa mutex, push, rồi `notify_one`. Cân nhắc bounded queue (thêm cv cho "không đầy") để chống tràn bộ nhớ, và `notify_all` khi nhiều consumer. Trên hệ đơn giản có thể dùng lock-free queue + atomic nhưng khó đúng.
</details>

---

⬅️ [Bank index](README.md)
