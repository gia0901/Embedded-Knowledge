# 2026-09-04 · `retention` · 10 câu · trần T2

> **30/40 = 3.00** · **2 regression** (`CPP-045`, `CPP-020`) · **8 câu dời lịch +2 tuần**
> Trả xong **hai đợt chồng nhau**: 24–29/08 (quá hạn) + 02–07/09 + 04–09/09. Mọi góc lấy từ cột *"Góc mới đề xuất"* đã ghi sẵn trong [bảng 🔁](../weak-register.md) — **không lặp góc cũ**.
> Câu mới sinh ra: [LNX-043](../bank/linux-sysprog.md) · [CPP-065](../bank/cpp.md).

## Bảng điểm

| # | ID | Góc mới đã dùng | Điểm | Trước | KQ |
|---|----|-----------------|------|-------|-----|
| 1 | CPP-019 | đọc code — producer/consumer thiếu nhãn một đầu | **3** | 3 | ✅ dời +2 tuần |
| 2 | CPP-024 | backtrace double-free → chẩn đoán ngược về tầng 2 | **4** | 4 | ✅ |
| 3 | CPP-045 | code review — phản biện `private` không định nghĩa | **2** | 3 | 🔻 **regression** |
| 4 | CPP-020 | class tự viết move → copy bị gì | **2** | 4 | 🔻 **regression** |
| 5 | CPP-029 | reallocate + move ctor không `noexcept` | **3** | 3 | ✅ |
| 6 | LNX-023 | code đọc `/proc/self/status` — `st_size`=0, `mmap` fail | **3** | — | ✅ |
| 7 | CPP-032 | `explicit operator bool` — contextual conversion | **3** | 3 | ✅ |
| 8 | OS-003 | mặt trái `std::lock` — livelock | **4** | 3 | ✅ **lỗi cũ đã vá** |
| 9 | OS-007 | PI bật rồi mà H vẫn trễ — còn nghi gì | **3** | 4 | ✅ |
| 10 | CPP-009 | `Stack<float>` mà không được build lại `.so` | **3** | 4 | ✅ |

---

## 🔴 PHÁT HIỆN CHÍNH — hai regression hỏng CÙNG MỘT KIỂU

Câu 3 và câu 4 **không phải hai lỗi rời**. Cả hai hỏi *"compiler tự động làm gì với **special member function**"*, và cả hai lần:

| | Câu 3 (`CPP-045`) | Câu 4 (`CPP-020`) |
|---|---|---|
| **(a) phát biểu LUẬT** | ✅ đúng — 2 lý do `= delete` tốt hơn | ✅ đúng — khai báo move ⇒ copy bị xoá |
| **(b) suy ra HỆ QUẢ** | ❌ sai — nói lỗi hiện ở **link**, thực tế **compile** | ❌ sai — nói `b = a` chạy, thực tế **vẫn lỗi** |

> ### Tên gọi khoảng trống: **thuộc luật, nhưng không "chạy compiler trong đầu" được**
>
> Mẫu **MỚI**, khác hai mẫu đã ghi: *"biết mà không nói được"* (18/08) và *"thuộc bài ≠ hiểu bài"* (19/08). Ở đây **nói được, hiểu luật** — chỉ hỏng khi phải **suy ra kết quả cụ thể**.
>
> ⚠️ **`rapid` không bao giờ bắt được mẫu này** — nó chỉ hỏi *"X là gì"*, mà phần đó ứng viên luôn đúng. Chỉ câu **đọc code + dự đoán output** mới lộ ra.
>
> **Cách chữa rẻ nhất và duy nhất: GÕ THỬ.** Ba dòng, 10 giây, `g++` trả lời thay vì phải đoán.

---

## Câu 3 — CPP-045 · **2 điểm** 🔻 regression (3 → 2)

**Đề:** Đồng nghiệp gửi code review, kèm comment *"cách này chạy ổn từ thời C++98, không cần `= delete`"*:
```cpp
class FileHandle {
public:
    explicit FileHandle(const char* path);
    ~FileHandle();
private:
    FileHandle(const FileHandle&);              // khai báo, KHÔNG định nghĩa
    FileHandle& operator=(const FileHandle&);
};
```
**(a)** Phản biện — ít nhất 2 lý do kỹ thuật. **(b)** Có tình huống nào cách cũ ném lỗi ra chỗ **khó tìm hơn hẳn**?

<details><summary>Đã trả lời gì · nhận xét · đáp án đầy đủ</summary>

**Đã trả lời (a):** ① không rõ ý định khi đọc — nằm trong `private` phải mất công suy nghĩ; `= delete` biết chắc hàm đã bị xoá. ② thiếu linh hoạt trong template programming khi muốn xoá **một overload** nhất định.
**Đã trả lời (b):** *"lỗi ở bước linker với log thiếu rõ ràng: private member… Nếu dùng delete, lỗi phát hiện ngay khi compile"*. Follow-up *"gọi từ ĐÂU thì mới ra lỗi linker?"* → *"khi cố copy ở **bên ngoài** class, ở thể ở process/thread bất kỳ. Lỗi hiện ở lúc link."*

**Được:** hai lý do ở (a) đều đúng và đáng giá — nhất là ý **xoá một overload cụ thể**, đó là công dụng mà cách cũ không có.

**Sai ở (b) — và (b) chính là trọng tâm câu hỏi.** Copy từ **NGOÀI** class ⇒ **compile error**, hiện ngay:
```
t45.cpp: In function ‘void outside(const FileHandle& h)’:
t45.cpp:9:55: error: ‘FileHandle::FileHandle(const FileHandle&)’ is private within this context
t45.cpp:6:5: note: declared private here
```
Lỗi **link** chỉ xảy ra khi lời gọi **qua được access check** — tức từ **trong class hoặc `friend`**:
```
--- compile: 0 (qua duoc) ---
t45b.cpp:(.text._ZNK10FileHandle5cloneEv...): undefined reference to `FileHandle::FileHandle(FileHandle const&)'
collect2: error: ld returned 1 exit status
```
⇒ Ứng viên **đảo ngược đúng chiều** của câu hỏi.

⚠️ Thêm một nhầm tầng: *"ở process/thread bất kỳ"* — **access control là luật ngôn ngữ lúc BIÊN DỊCH**, không liên quan gì tới process/thread lúc chạy.

**Ý mạnh nhất chưa nêu:** khai báo copy ctor — **dù `private`** — vẫn **chặn compiler sinh move operations**. Class **im lặng** mất khả năng move; mọi chỗ tưởng đang move hoá ra **copy**. `= delete` không có tác dụng phụ đó. Đây là mối nối thẳng sang [CPP-020](../bank/cpp.md) — và chính là lý do hai câu này phải ôn **cùng nhau**.

**Ôn lại:** [bank CPP-045](../bank/cpp.md) · [oop.md](../../../01-cpp-fundamentals/oop.md) · [raii-smart-pointers.md](../../../02-modern-cpp/raii-smart-pointers.md).
</details>

---

## Câu 4 — CPP-020 · **2 điểm** 🔻 regression (4 → 2)

**Đề:**
```cpp
class Buffer {
    std::unique_ptr<uint8_t[]> data_;
    size_t                     size_;
public:
    explicit Buffer(size_t n) : data_(new uint8_t[n]), size_(n) {}
    Buffer(Buffer&& o) noexcept : data_(std::move(o.data_)), size_(o.size_) { o.size_ = 0; }
    Buffer& operator=(Buffer&& o) noexcept { data_ = std::move(o.data_); size_ = o.size_; o.size_ = 0; return *this; }
};
Buffer a(1024);
Buffer b = a;      // (X)
```
**(a)** Dòng (X) xảy ra chuyện gì? **(b)** Nếu **xoá cả hai hàm move** thì (X) đổi thành gì?

<details><summary>Đã trả lời gì · nhận xét · đáp án đầy đủ</summary>

**Đã trả lời (a):** *"Lỗi compile vì không có copy operator. Sửa move ⇒ compiler xác nhận tự quản lý tài nguyên, không generate copy."* ✅ **Đúng, và lý do cũng đúng.**

**Đã trả lời (b):** *"Thành rule of 0, `b = a` hoạt động với copy mặc định do compiler generate. `unique_ptr` là container chuẩn, compiler có thể xử lý move/copy cho nó."* ❌ **Sai.**

**Chạy thật:**
```
t20.cpp:10:41: error: use of deleted function ‘Buffer::Buffer(const Buffer&)’
note: ‘Buffer::Buffer(const Buffer&)’ is implicitly deleted because
      the default definition would be ill-formed:
error: use of deleted function ‘std::unique_ptr<...>::unique_ptr(const std::unique_ptr<...>&)’
```

**Vì sao:** copy ctor **được khai báo ngầm**, nhưng **định nghĩa ngầm là ill-formed** vì member `unique_ptr` **không copy được** — đó là bản chất của `unique_ptr` (sở hữu **độc quyền**). Nói *"`unique_ptr` … compiler có thể xử lý copy cho nó"* là **ngược hẳn** với mục đích tồn tại của nó.

📌 **Công bằng — lời khuyên thì ĐÚNG:** bỏ cả hai hàm move ⇒ **Rule of 0**, và class **tự động move được, tự động không copy được** — đúng thứ ta muốn. Sai nằm ở **dự đoán về dòng (X)**, không ở khuyến nghị thiết kế.

**Ôn lại:** [bank CPP-020](../bank/cpp.md) · [raii-smart-pointers.md](../../../02-modern-cpp/raii-smart-pointers.md).
</details>

---

## ✅ BẰNG CHỨNG NGƯỢC — retention CÓ tác dụng, đo được

### Câu 8 — OS-003 · **4 điểm** (3 → 4), lỗi ghi trong sổ 21/08 **đã vá**

**Đề:** Thread 1 chạy `std::scoped_lock lk(a.m, b.m)`, thread 2 chạy `std::scoped_lock lk(b.m, a.m)`. **(a)** Có deadlock không, `scoped_lock` **phá điều kiện Coffman nào**? **(b)** Nếu hai thread cứ nhả–thử mãi thì hiện tượng đó **tên gì**, khác deadlock chỗ nào?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** (a) *"Không. Phá điều kiện **hold-and-wait**, vì nó không giữ luôn khoá nếu khoá kia không lock được."* (b) *"**livelock**, khác ở chỗ lock/unlock liên tục trong lúc runtime, tuy nhiên vẫn không thể vào được."* Follow-up → tự nêu **lock ordering ⇒ phá circular wait**.

**Sổ yếu ghi nguyên văn ngày 21/08:**
> ⚠️ hụt **cơ chế bên trong**: nói *"đảm bảo thứ tự lock"* (**sai**, là try-and-backoff) và *"phá circular wait"* (**sai**, là **hold-and-wait**)

⇒ Hôm nay gọi **đúng cả hai**, và ở follow-up còn **tự phân biệt hai cơ chế**: `lock ordering` phá *circular wait* · `try-and-backoff` phá *hold-and-wait*. **Đây là ca có đủ hai mốc trước–sau cho một lỗi cụ thể** ⇒ bằng chứng cơ chế retention hoạt động.

**Còn thiếu (nhỏ):** vì sao livelock **hầu như không xảy ra thật** — `std::lock` thử lại **bắt đầu từ mutex vừa thất bại** (không phải luôn từ cái đầu), cộng jitter lập lịch ⇒ thế đối xứng vỡ gần như tức thì. Ứng viên trả lời *"có khả năng xảy ra"* rồi chuyển sang đề xuất lock ordering — đúng về **thiết kế**, nhưng né mất câu hỏi về **cơ chế**.
</details>

### Câu 6 — LNX-023 · **3 điểm** · "không rõ" rồi tự gỡ được

<details><summary>Đã trả lời gì · output chạy thật</summary>

**Đã trả lời:** (a) *"không phải file trên disk, chỉ là entry, kernel chạy hàm tương ứng cho ra kết quả"* ✅ · (b) *"không có memory, không có dữ liệu để map"* ✅ · (c) **"không rõ"**, chỉ nói không dùng `mmap`.

**Follow-up** (`buf = malloc(st.st_size + 1); n = read(fd, buf, st.st_size);`) → trả lời **chuẩn xác**: *"n = 0 … dễ nhầm lẫn là đã đọc đến EOF nên mới trả về 0"*.

**Output chạy thật** (`gcc -Wall -Wextra`):
```
mmap: No such device
st_size = 0
read() tra ve = 0
read() voi buffer 8KB = 1547 byte
```
File có **1547 byte** mà `read()` báo **0** — không một thông báo lỗi nào.

**Đã thành câu bank riêng: [LNX-043](../bank/linux-sysprog.md).**
</details>

### Câu 7 — CPP-032 · **3 điểm** · tự sửa giữa chừng

<details><summary>Đã trả lời gì · output chạy thật</summary>

**Đã trả lời (a):** chỉ nêu dòng (3) hỏng — **bỏ sót dòng (2)** `bool ok = f;`. **(b)** *"vì `if (f)` là đúng tình huống được dùng `operator bool()`"* — đúng kết luận nhưng **không gọi tên cơ chế**. **(c)** tiêu chí: explicit khi object cấp phát tài nguyên / nhiều tham số dễ nhầm; ngầm khi ngữ nghĩa đơn giản.

**Follow-up** → **tự sửa đúng**: `bool ok = f;` hỏng (copy-initialization cần chuyển đổi **ngầm**), `bool ok(f);` chạy (direct-initialization **cho phép** explicit).

**Chạy thật:**
```
t32.cpp:10:15: error: cannot convert ‘FileHandle’ to ‘bool’ in initialization      ← (2)
t32.cpp:11:15: error: no match for ‘operator<<’ ...                                ← (3)
```
Chỉ (2) và (3) hỏng; (1), (4) và `bool ok2(f)` đều qua.

**Tên cơ chế cần nhớ:** **contextual conversion to `bool`** — `if` / `while` / `for` / `!` / `&&` / `||` / `?:` **được phép** dùng `explicit operator bool`. Đó là lý do idiom này an toàn: dùng được ở chỗ cần, chặn được chỗ không nên.
</details>

---

## Ba câu 3 điểm — thiếu đúng cái gì

### Câu 1 — CPP-019 · **3 điểm** · mô hình bên dưới bị NGƯỢC

<details><summary>Đề · đã trả lời · phần còn thiếu</summary>

**Đề:**
```cpp
int data = 0;  std::atomic<bool> ready{false};
void producer() { data = 42;                                    // (1)
                  ready.store(true, std::memory_order_release); }  // (2)
void consumer() { while (!ready.load(std::memory_order_relaxed))   // (3)
                      std::this_thread::yield();
                  std::cout << data << "\n"; }                     // (4)
```
*(Ứng viên hỏi rõ `std::this_thread::yield()` là gì trước khi trả lời — **hỏi rõ API không phải mất điểm**, đó là hành vi đúng ở phỏng vấn thật.)*

**Đã trả lời:** sai ở dòng (3); release phải đi cặp với acquire trên cùng biến atomic; (4) có thể in **0**. ✅ **Đúng hết.**

**Follow-up (a)** *"release ở (2) có đảm bảo (1) xảy ra trước (2) trên thread A không?"* → **"Không"** ❌ **SAI**.

**Đúng là:** `memory_order_release` ở (2) **CÓ** đảm bảo mọi đọc/ghi trước nó không bị đẩy xuống sau — đó **chính là định nghĩa** của release. **Thread A hoàn toàn đúng.** Hỏng nằm ở **thread B**: `relaxed` **không tạo cạnh *synchronizes-with***, nên B không có gì bảo đảm **nhìn thấy** thứ tự mà A đã thiết lập.

⇒ Ứng viên kết luận đúng (*"chỗ hỏng ở phía B"*) nhưng **mô hình bên dưới ngược**: tưởng thứ tự của **A** bị phá, thực tế A đúng còn **B mù**.

**Chưa chạm tới (theo ghi chú calibration 10/08, đây là điều kiện lên 4đ):** ý niệm **x86 vs ARM** — acq/rel gần như *free* trên x86 nhưng **có phí thật trên ARM** ⇒ *tối ưu memory order đo trên máy dev x86 là vô nghĩa*. Đúng lớp bug của JD ARM. *(Tên lệnh barrier `stlr`/`ldar`/`dmb ish` là **T3, KHÔNG chấm**.)*

**Ôn lại:** [bank CPP-019](../bank/cpp.md) · [concurrency.md](../../../02-modern-cpp/concurrency.md).
</details>

### Câu 5 — CPP-029 · **3 điểm**

<details><summary>Đề · đã trả lời · phần còn thiếu</summary>

**Đề:** `vector<Panel>` với `reserve(2)`, `Panel` có move ctor **không** `noexcept`; `emplace_back` thứ ba gây reallocate. (a) hai phần tử cũ chuyển bằng **move hay copy**, vì sao? (b) hệ quả trên thiết bị nhúng, sửa **một chữ** ở đâu?

**Đã trả lời:** (a) **copy** — *"object tạo ra không được bảo đảm sẽ không quăng exception, vector chọn con đường an toàn, nếu copy lỗi thì bản gốc vẫn còn tồn tại"* ✅ **hoàn toàn đúng** (chính là `move_if_noexcept` + strong exception guarantee). (b) chi phí copy tăng theo kích thước vector; **sửa: thêm `noexcept` vào move** ✅ **chính xác**.

**Follow-up** *"nếu move KHÔNG thể `noexcept` (phải cấp phát) thì còn cách nào?"* → **"không rõ"**.

**Đáp án:** ① ⭐ **`reserve()` đúng dung lượng ngay từ đầu** — né hẳn reallocate, và trên embedded ta thường **biết trước N** ② `std::deque` — thêm phần tử **không** di chuyển phần tử cũ ③ `std::vector<std::unique_ptr<Panel>>` — di chuyển **con trỏ**, luôn `noexcept` ④ cấp phát sẵn bằng pool/arena.

**Ôn lại:** [bank CPP-029](../bank/cpp.md) · [complexity-and-structures.md](../../../12-dsa/complexity-and-structures.md).
</details>

### Câu 9 — OS-007 · **3 điểm** (4 → 3) · nêu 1/4 nguyên nhân

<details><summary>Đề · đã trả lời · ba nguyên nhân còn thiếu</summary>

**Đề:** H (`SCHED_FIFO`, chu kỳ 10 ms) và L dùng chung mutex đã bật **`PTHREAD_PRIO_INHERIT`**. PI hoạt động đúng, **H vẫn thỉnh thoảng trễ deadline**. Còn nghi những gì?

**Đã trả lời:** khi L chạy, có thể có **thread RT khác priority cao hơn** preempt L ⇒ H vẫn bị kéo dài. Đề xuất: xác định rõ mức priority các thread quan trọng và chọn **mức trần (ceiling)** phù hợp cho mutex. Follow-up *"việc gì L làm trong critical section mà nâng priority không giúp?"* → **I/O / giao tiếp phần cứng** ✅.

**Được:** nguyên nhân nêu ra **đúng**, và **priority ceiling** là hướng xử lý chuẩn cho đúng ca đó. Follow-up ra được I/O.

**Ba nguyên nhân còn thiếu:**
1. ⭐ **Page fault trong critical section** — một phép truy cập bộ nhớ *trông như bình thường* lại **block trên I/O**. Chữa: `mlockall(MCL_CURRENT|MCL_FUTURE)` + pre-fault stack/heap. Đây là cái **khó ngờ nhất**, vì code không hề có lời gọi I/O nào.
2. **Inversion kép qua hai khoá** — L giữ M1, kẹt ở M2 do M (medium) đang giữ; PI trên M1 **không tự bắc cầu** sang M2.
3. **Kernel không phải PREEMPT_RT** — đoạn không preempt được trong kernel (ISR, spinlock, softirq) làm trễ H **bất kể** priority userspace. Nối [BSP-021](../bank/bsp.md).

**Ôn lại:** [bank OS-007](../bank/os.md) · [scheduling.md](../../../03-operating-system/scheduling.md) · [rtos-vs-linux.md](../../../08-embedded-systems/rtos-vs-linux.md).
</details>

### Câu 10 — CPP-009 · **3 điểm** (4 → 3) · thiếu phương án đúng nghề mình nhất

<details><summary>Đề · đã trả lời · phương án còn thiếu</summary>

**Đề:** Khách chỉ có `stack.h` + `libstack.so` (explicit instantiation cho `int`, `double`). Khách cần `Stack<float>`, **không được build lại `.so`**. (a) hỏng ở bước nào? (b) còn đường nào, kèm cái giá?

**Đã trả lời:** (a) **link**, *"không tìm thấy định nghĩa phù hợp"* ✅. (b) hai phương án: ① buộc phải build lại `.so` ② chuyển hết định nghĩa lên `stack.h` — kèm **đúng cái giá**: *"vi phạm policy chia sẻ source code"*.

**Được:** (a) chính xác; phương án ② đúng và **tự nêu được cái giá thật** (lộ source).

**Thiếu — và đây là phương án đúng nghề bạn nhất:** ⭐ **type erasure**: giữ phần cài đặt **không phải template** trong `.so` (thao tác trên **byte**, hoặc qua interface ảo), header chỉ còn **vỏ template mỏng** ép kiểu gọi xuống. Khách dùng `Stack<float>` **không cần build lại `.so`**, **không lộ source**. Trả giá: mất một phần tối ưu theo kiểu, thêm một tầng gián tiếp, tự lo `sizeof`/alignment/lifetime.

Cùng tư duy với [SD-021 Pimpl](../bank/system-design.md) và [SD-022 biên giới C API](../bank/system-design.md): **đẩy phần thay đổi ra khỏi thứ đã ship**.

**Đã thành câu bank riêng: [CPP-065](../bank/cpp.md).**
</details>

### Câu 2 — CPP-024 · **4 điểm**

<details><summary>Đề · đã trả lời · phần bù</summary>

**Đề:** `PanelRegistry` với `shared_ptr<PanelInfo> cur_`, N reader gọi `get()` (`return cur_`), 1 writer gọi `refresh()` (`cur_ = p`). Backtrace: `free(): double free detected in tcache 2` → `_Sp_counted_ptr::_M_destroy` → `~__shared_ptr` → `PanelRegistry::refresh`. (a) refcount là atomic, sao vẫn double free? (b) hỏng tầng nào? (c) sửa sao?

**Đã trả lời:** (a) `cur_ = p` giải phóng `cur_` trong khi một reader đang đọc chính object đó ⇒ giải phóng hai lần. (b) ✅ **tầng 2 — bản thân object `shared_ptr` bị tác động đồng thời**. (c) ① mutex ở cả hai hàm — *"hiệu quả nhưng overhead, nhất là 1 writer N reader"* ② atomic cho `shared_ptr`. Follow-up C++17 → ✅ **`std::atomic_load` / `std::atomic_store`**.

**Chính xác về tầng và về API C++17.** *(`std::atomic<std::shared_ptr<T>>` chỉ có từ C++20; `atomic_load/store` bị **deprecate ở C++20 và bỏ ở C++26** — nhưng JD là **C++17** nên đây đúng là lựa chọn.)*

**Bù phần ứng viên nói thẳng là chưa biết — `std::shared_mutex` (C++17):** reader-writer lock. Reader khoá bằng `std::shared_lock` (**N reader vào song song**), writer khoá bằng `std::unique_lock` (**độc quyền**). So sánh: với 1 writer / N reader thì `atomic_load/store` **nhẹ hơn** (không khoá); `shared_mutex` cần khi phải bảo vệ **nhiều biến cùng lúc** một cách nhất quán. Xem [OS-028](../bank/os.md).

👍 **Nói "tôi không có kiến thức về `shared_mutex`" là hành vi ĐÚNG** — tốt hơn hẳn đoán bừa, và ngược hẳn với vấn đề đã ghi ở [log 29/08](2026-08-29--rapid--resume.md).
</details>

---

## 📌 Ưu tiên

| | Việc | Vì sao |
|---|---|---|
| **1** 🔴 | **Gõ thử 3 dòng thay vì đoán** — gặp câu *"compiler sinh gì / hỏng ở bước nào"* thì mở file, `g++`, đọc thông báo | **Nguyên nhân chung của CẢ HAI regression** |
| **2** 🟠 | Ôn `CPP-045` + `CPP-020` **cùng nhau** — cùng một chủ đề: *khai báo một special member ảnh hưởng gì tới các cái còn lại* | [oop.md](../../../01-cpp-fundamentals/oop.md) · [raii-smart-pointers.md](../../../02-modern-cpp/raii-smart-pointers.md) |
| **3** 🟠 | `CPP-019` — vẽ lại mô hình: **A đúng, B mù**, không phải *"A bị đảo thứ tự"* | [concurrency.md](../../../02-modern-cpp/concurrency.md) |

⬅️ [Về sessions](README.md)
