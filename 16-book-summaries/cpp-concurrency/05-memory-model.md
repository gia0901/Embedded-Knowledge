# Ch. 5 — The C++ memory model and operations on atomic types (tr. 124–172) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 4 — Synchronization](04-synchronization.md)** · **[Ch. 6 → Lock-based concurrent data structures](06-lock-based-structures.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **🎯🎯 ĐÂY LÀ CHƯƠNG GIÁ TRỊ INTERVIEW CAO NHẤT SÁCH — và là lý do chính đọc cuốn này.** Nó giải thích **vì sao** mọi thứ ở ch. 2–4 (mutex, future, condition variable) hoạt động, và là chủ đề **phân loại ứng viên senior C++** mà không tài liệu nào khác trong repo phủ. Chương khó nhất → mình đầu tư nhiều code + sơ đồ happens-before hơn mọi chương khác, giải thích từng bước.

**Sách nêu mục tiêu chương (tr. 124):** chi tiết C++ memory model · các atomic type của Standard Library · các thao tác trên chúng · cách dùng chúng để **đồng bộ giữa thread**.

**Câu định vị cả chương (tr. 124):** *"if you use mutexes to protect your data and condition variables, futures, latches, or barriers to signal events, the details of why they work aren't important. It's only when you start trying to get 'close to the machine' that the precise details of the memory model matter."* → Nếu chỉ dùng mutex thì **không cần chương này**; nó cần khi viết **lock-free** (ch. 7) hoặc muốn hiểu bản chất. Atomic operations cho phép đồng bộ **low-level thường rút về 1–2 lệnh CPU** (tr. 125).

---

## Cụm 1 — Nền tảng memory model: object, memory location, data race, modification order (tr. 125–128)

### 1.1 Object & memory location (tr. 125–126)

**Mọi dữ liệu trong C++ là object (tr. 125)** — không theo nghĩa Smalltalk/Ruby, mà theo nghĩa: object = *"a region of storage"* có type và lifetime. Object được lưu trong **một hoặc nhiều memory location**; mỗi memory location hoặc là:
- một object/sub-object kiểu **scalar** (`unsigned short`, `my_class*`...), hoặc
- một **dãy bit field liền kề**.

**⚠️ Bit field liền kề = CÙNG một memory location (Figure 5.1, tr. 125–126):**
```cpp
struct my_data {
    int i;              // memory location riêng
    double d;           // memory location riêng
    unsigned bf1:10;    // ┐ bf1 & bf2 CHUNG một memory location
    int bf2:25;         // ┘
    int bf3:0;          // bit field độ dài 0 → TÁCH bf4 sang location riêng, tự nó KHÔNG có location
    int bf4:9;          // memory location riêng (nhờ bf3 tách)
    int i2;             // memory location riêng
    char c1, c2;        // mỗi char một memory location riêng
    std::string s;      // gồm nhiều memory location bên trong
};
```

**4 điều phải nhớ (tr. 126):**
1. Mọi biến là object, kể cả member của object khác.
2. Mọi object chiếm **ít nhất một** memory location.
3. Biến kiểu cơ bản (`int`, `char`) chiếm **đúng một** memory location, bất kể kích thước, kể cả khi liền kề/trong mảng.
4. Bit field liền kề chung **một** memory location.

### 1.2 Memory location & concurrency — định nghĩa data race (tr. 126–127)

**Mọi thứ xoay quanh memory location (tr. 126):**
- Hai thread truy cập **memory location KHÁC nhau** → không vấn đề.
- Hai thread truy cập **CÙNG memory location** → phải cẩn thận: nếu **không ai ghi** thì ok (read-only miễn đồng bộ); nếu **có ai ghi** → nguy cơ race condition.

**Để tránh race, phải có ORDERING được ép buộc (tr. 126–127)** giữa các truy cập. Hai cách: **(a) mutex** (cùng mutex khóa trước cả hai truy cập → một cái phải xảy ra trước cái kia), **(b) synchronization của atomic operations** (§5.2–5.3).

**⚠️ Định nghĩa data race — câu quan trọng nhất Cụm 1 (tr. 127):**

> *"If there's no enforced ordering between two accesses to a single memory location from separate threads, one or both of those accesses is not atomic, and if one or both is a write, then **this is a data race and causes undefined behavior**."*

Sách nhấn UB nghiêm trọng thế nào (tr. 127): *"once an application contains any undefined behavior, all bets are off... it may do anything at all. I know of one case where a particular instance of undefined behavior caused someone's monitor to catch fire."* Nhưng có lối thoát: **dùng atomic operations trên memory location đang race** → không ngăn được race (cái nào chạm trước vẫn không xác định) nhưng **đưa chương trình về vùng defined behavior** (tr. 127).

### 1.3 Modification order (tr. 127–128)

**Mọi object có một modification order (tr. 127):** = tất cả các lần ghi vào object đó từ mọi thread, bắt đầu từ khởi tạo. Trong một lần chạy, **mọi thread phải đồng ý** về thứ tự này. Nếu không phải atomic type → **bạn** chịu trách nhiệm đồng bộ đủ để các thread đồng ý; nếu thread thấy **dãy giá trị khác nhau** cho một biến → data race + UB. Nếu dùng atomic → compiler lo.

Hệ quả (tr. 127–128): cấm một số **speculative execution** — một khi thread đã thấy một mục trong modification order, đọc sau của thread đó phải trả **giá trị sau đó** (không lùi). **⚠️ Nhưng: mọi thread đồng ý modification order của TỪNG object riêng lẻ, KHÔNG nhất thiết đồng ý thứ tự tương đối giữa các object khác nhau** (tr. 128) — đây là hạt nhân của toàn bộ sự khó hiểu về sau.

### Insight đáng nhớ (Cụm 1)
- **Data race (nghĩa C++) = UB, luôn phải diệt.** Điều kiện: ≥2 thread chạm **cùng memory location**, ≥1 **ghi**, **không đồng bộ**, ≥1 **không atomic**. Tránh bằng mutex hoặc atomic.
- **Bit field liền kề chung memory location** → ghi hai bit field cạnh nhau từ hai thread = data race dù "logic" là hai biến khác nhau. Bẫy tinh vi.
- **Mỗi object có modification order riêng, nhưng thread không buộc đồng ý thứ tự GIỮA các object.** Ghi nhớ câu này — nó giải thích mọi kết quả "phản trực giác" ở Cụm 4.

---

## Cụm 2 — Atomic type & thao tác: `atomic_flag`, `atomic<bool>`, `atomic<T*>`, integral, primary template (tr. 128–142)

### 2.1 Atomic operation là gì, `is_lock_free` (tr. 128–129)

**Atomic operation = thao tác bất khả phân (indivisible) (tr. 128):** không thread nào quan sát được nó "nửa chừng" — hoặc xong hoặc chưa. Ngược lại, thao tác **non-atomic** có thể bị thấy nửa chừng.

Atomic type ở `<atomic>`. **`is_lock_free()` (tr. 128):** trả `true` nếu thao tác dùng **lệnh atomic thật của phần cứng**; `false` nếu dùng **lock nội bộ** của compiler/thư viện. **⚠️ Quan trọng:** dùng atomic để **thay mutex** cho hiệu năng — nếu bản thân atomic dùng mutex nội bộ thì **mất luôn lợi ích**, thà dùng mutex cho dễ đúng.

**`is_always_lock_free` (C++17, tr. 129):** `static constexpr` — `true` nếu lock-free trên **mọi** phần cứng đích có thể chạy. `std::atomic<int>::is_always_lock_free` thường `true`; `std::atomic<uintmax_t>` có thể là runtime property. Kèm macro `ATOMIC_*_LOCK_FREE` (0 = không bao giờ, 2 = luôn, 1 = runtime).

### 2.2 `std::atomic_flag` — building block + spinlock (tr. 132–134)

`std::atomic_flag` là atomic type **đơn giản nhất** và là **cái duy nhất bảo đảm lock-free** (tr. 132). Cực hạn chế: chỉ **set/clear**, không load/store thường, không copy/assign. Khởi tạo **bắt buộc** `ATOMIC_FLAG_INIT` (về clear):
```cpp
std::atomic_flag f = ATOMIC_FLAG_INIT;   // luôn bắt đầu ở trạng thái clear
```
Ba thao tác: destructor, `clear()` (store op → không dùng acquire/acq_rel), `test_and_set()` (RMW → dùng tag nào cũng được).

**Vì sao mọi atomic KHÔNG copy/assign được (tr. 133):** *"assignment and copy-construction involve two objects. A single operation on two distinct objects can't be atomic."* Copy = đọc object này + ghi object kia = **hai thao tác trên hai object** → không thể atomic.

**Listing 5.1 — spinlock mutex từ `atomic_flag` (tr. 133–134):**
```cpp
class spinlock_mutex {
    std::atomic_flag flag;
public:
    spinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire));   // quay tới khi set được (old==false)
    }
    void unlock() {
        flag.clear(std::memory_order_release);                  // clear = nhả khóa
    }
};
```
Dùng được với `std::lock_guard`. **⚠️ Busy-wait trong `lock()`** → tệ nếu có contention cao, nhưng đủ đảm bảo mutual exclusion. (Ngữ nghĩa acquire/release ở §5.3.6 giải thích vì sao đúng.)

### 2.3 `std::atomic<bool>` & compare-exchange — hòn đá tảng (tr. 134–136)

`std::atomic<bool>` đầy đủ hơn `atomic_flag`: construct từ `bool` thường, `store()`, `load()`, `exchange()`. **⚠️ Assignment trả về GIÁ TRỊ (không phải reference) (tr. 134):** để tránh phải load lại (mà load lại = mở cửa cho race).

**Compare-exchange — "hòn đá tảng của lập trình atomic" (tr. 135):** so giá trị atomic với **expected**; nếu bằng thì ghi **desired** (trả `true`); nếu khác thì **cập nhật expected = giá trị hiện tại** (trả `false`).

**⚠️ `weak` vs `strong` (tr. 135–136):**
| | `compare_exchange_weak` | `compare_exchange_strong` |
|---|---|---|
| Có thể **fail giả (spurious)** dù giá trị == expected? | **CÓ** — khi máy thiếu lệnh CAS đơn, thread bị switch giữa chừng | **KHÔNG** — chỉ fail khi thật sự != expected |
| Phải dùng trong **vòng lặp**? | **Có** (vì spurious fail) | Không nhất thiết |
| Khi nào chọn | Tính giá trị store **rẻ** (tránh double-loop) | Tính giá trị store **tốn** (tránh tính lại) |

```cpp
// weak PHẢI trong loop vì spurious failure:
bool expected = false;
extern std::atomic<bool> b;
while (!b.compare_exchange_weak(expected, true) && !expected);
//  ↑ lặp khi expected VẪN false = fail giả; nếu expected thành true nghĩa là thread khác ghi trước
```

**Hai memory order cho compare-exchange (tr. 136):** một cho **success**, một cho **failure**. **⚠️ failure không được là release/acq_rel** (fail không store), và **không được strict hơn success**. Nếu không ghi failure → mặc định = success nhưng **bỏ phần release** (release→relaxed, acq_rel→acquire).

`std::atomic<bool>` **có thể không lock-free** (khác `atomic_flag`) → kiểm bằng `is_lock_free()`.

### 2.4 `std::atomic<T*>` — pointer arithmetic (tr. 137–138)

Giao diện như `atomic<bool>` nhưng thao tác trên `T*`. Thêm **số học con trỏ**: `fetch_add()`/`fetch_sub()` (RMW, trả **giá trị CŨ**), `+=`/`-=`/`++`/`--` (trả giá trị **MỚI**).
```cpp
Foo some_array[5];
std::atomic<Foo*> p(some_array);
Foo* x = p.fetch_add(2);          // p → &some_array[2], nhưng x = some_array (giá trị CŨ)
assert(x == some_array);
assert(p.load() == &some_array[2]);
x = (p -= 1);                     // p → &some_array[1], x = giá trị MỚI = &some_array[1]
```
`fetch_add`/`fetch_sub` = **exchange-and-add**, là RMW → nhận mọi memory tag và tham gia **release sequence** (§5.3.4). Dạng toán tử (`+=`) luôn `seq_cst`.

### 2.5 Atomic integral & primary template (tr. 138–140)

**Integral (`atomic<int>`...) (tr. 138):** thêm `fetch_add/sub/and/or/xor`, `+=/-=/&=/|=/^=`, `++/--`. Thiếu `*`, `/`, shift (làm bằng `compare_exchange_weak` trong loop nếu cần). Named function trả giá trị **cũ**; compound-assign trả **mới**.

**`std::atomic<UDT>` cho user-defined type (tr. 138–140) — ⚠️ điều kiện ngặt:**
- Kiểu phải có **trivial copy-assignment** (không virtual function/base, dùng copy-assign do compiler sinh, mọi base & member cũng trivial) → cho phép dùng `memcpy`.
- **⚠️ Compare-exchange so sánh BITWISE (như `memcmp`)**, không dùng `operator==` của UDT. → Nếu kiểu có **padding bit** hoặc `==` khác `memcmp`, compare-exchange có thể **fail dù giá trị "bằng"**. Áp cả cho `atomic<float>`/`atomic<double>` (cùng giá trị nhưng representation khác → `compare_exchange_strong` fail). **Không có atomic arithmetic cho float.**
- Lý do (tr. 139): về nguồn gốc là guideline ch. 3 — không cho user code chạy khi giữ lock nội bộ (deadlock/block). Và tăng cơ hội compiler dùng lệnh atomic trực tiếp (coi UDT như dãy byte thô).
- Không tạo được `atomic<vector<int>>` (copy ctor non-trivial); tạo được với class chứa counter/flag/pointer/mảng đơn giản. Phức tạp hơn → dùng `std::mutex`.

**Free functions (tr. 140–142):** bản non-member (prefix `atomic_`, nhận **pointer** làm tham số đầu — tương thích C), có bản `_explicit` cho memory order. Có **atomic free functions cho `std::shared_ptr`** (`std::atomic_load`/`store`) — ⚠️ `shared_ptr` **không phải** atomic type, truy cập một `shared_ptr` từ nhiều thread mà không dùng các hàm này = data race. (Concurrency TS có `atomic_shared_ptr<T>` là atomic type thật.)

### Insight đáng nhớ (Cụm 2)
- **`atomic_flag` = lock-free bảo đảm, cực hạn chế → chỉ để xây spinlock.** Mọi atomic khác có thể không lock-free → `is_lock_free()`/`is_always_lock_free`.
- **compare-exchange là hòn đá tảng của lock-free.** `weak` (spurious fail, dùng trong loop, tính rẻ) vs `strong` (không fail giả, tính tốn). So sánh **bitwise** — cẩn thận float/padding.
- **atomic assignment trả GIÁ TRỊ không REFERENCE**, named function trả **giá trị cũ**, compound-assign trả **mới** — để tránh load-lại-mở-cửa-race.

---

## Cụm 3 — 🎯🎯 synchronizes-with & happens-before: hai quan hệ nền tảng (tr. 142–146)

Đây là **trái tim lý thuyết của chương**. Mọi memory order ở Cụm 4 chỉ là cách **tạo ra hay không tạo ra** hai quan hệ này.

### 3.1 Ví dụ mở đầu — Listing 5.2 (tr. 142–143)

```cpp
std::vector<int> data;                       // dữ liệu THƯỜNG (non-atomic)
std::atomic<bool> data_ready(false);

void writer_thread() {
    data.push_back(42);                      // (d) ghi dữ liệu non-atomic
    data_ready = true;                       // (e) set cờ atomic (seq_cst mặc định)
}
void reader_thread() {
    while (!data_ready.load()) {             // (b) chờ cờ
        std::this_thread::sleep(std::chrono::milliseconds(1));
    }
    std::cout << "The answer=" << data[0] << "\n";   // (c) đọc dữ liệu non-atomic
}
```

**Vì sao KHÔNG data race dù `data` là non-atomic (tr. 143)?** Nhờ chuỗi quan hệ:
```
(d) ghi data   ──happens-before──►  (e) set data_ready       [cùng thread, sequenced-before]
(e) set true   ──synchronizes-with─►  (b) load đọc true       [store/load atomic cùng biến]
(b) load true  ──happens-before──►  (c) đọc data              [cùng thread, sequenced-before]
────────────────────────────────────────────────────────────  [happens-before có tính BẮC CẦU]
⟹ (d) ghi data  happens-before  (c) đọc data   →  KHÔNG data race, đọc được 42
```
Sơ đồ (Figure 5.2 dựng lại):
```
   Writer thread                    Reader thread
   ─────────────                    ─────────────
   data.push_back(42)               data_ready.load() → false
        │ (sequenced-before)              │
        ▼                           data_ready.load() → false
   data_ready = true  ──────sync──►  data_ready.load() → true
                          -with            │ (sequenced-before)
                                           ▼
                                     data[0] → 42
```

### 3.2 synchronizes-with (tr. 143–144)

**Chỉ có được GIỮA thao tác trên ATOMIC type (tr. 143).** Ý tưởng cơ bản (tr. 144):

> Một **atomic write W** (tag thích hợp) trên biến `x` **synchronizes-with** một **atomic read** (tag thích hợp) trên `x` mà đọc: (a) giá trị do W ghi, HOẶC (b) giá trị do write sau đó của **cùng thread** đã ghi W, HOẶC (c) một chuỗi **read-modify-write** trên `x` bởi **bất kỳ thread nào** mà giá trị đầu chuỗi là do W ghi (→ release sequence, §5.3.4).

Đơn giản: **thread A store, thread B load đúng giá trị đó → store synchronizes-with load.** Phần "tag thích hợp" là chỗ memory order can thiệp (Cụm 4). Mặc định (`seq_cst`) mọi thứ đều "tag thích hợp".

### 3.3 happens-before & inter-thread happens-before (tr. 145–146)

**happens-before = xương sống của ordering (tr. 145):** xác định thao tác nào **thấy** hiệu ứng của thao tác nào.
- **Trong một thread:** nếu A **sequenced-before** B (A ở câu lệnh trước B trong source) → A happens-before B.
- **⚠️ Trong cùng một câu lệnh: thường KHÔNG có happens-before** (Listing 5.3): `foo(get_num(), get_num())` in ra "1,2" hoặc "2,1" — **không xác định** thứ tự hai lời gọi.

**inter-thread happens-before (tr. 145–146):** nếu A (thread 1) **synchronizes-with** B (thread 2) → A inter-thread happens-before B. **Bắc cầu** (transitive): A→B, B→C ⟹ A→C. Kết hợp với sequenced-before: A sequenced-before B và B inter-thread-happens-before C ⟹ A inter-thread-happens-before C → *"you need only ONE synchronizes-with relationship for the data to be visible"* (tr. 146).

**⚠️ strongly-happens-before (tr. 146):** hầu như giống happens-before, chỉ khác: thao tác tag **`memory_order_consume`** tham gia inter-thread-happens-before **nhưng KHÔNG** strongly-happens-before. Vì hầu hết code không dùng `consume`, khác biệt này không ảnh hưởng thực tế — sách dùng "happens-before" cho gọn.

### Insight đáng nhớ (Cụm 3)
- **happens-before = "A thấy được hiệu ứng của mọi thứ trước A".** Data race = "một thread thấy được trạng thái mà không có happens-before bảo vệ".
- **synchronizes-with là cây cầu DUY NHẤT nối hai thread** (chỉ atomic tạo được). Một cặp store-release/load-acquire đúng giá trị = một nhịp cầu; happens-before bắc cầu nối phần còn lại (sequenced-before trong mỗi thread).
- **Công thức tiêu chuẩn:** `ghi data (sequenced) → store cờ atomic (sync-with) load cờ → (sequenced) đọc data`. Đây là mẫu "publish dữ liệu qua một cờ atomic" — nền của mọi lock-free publish.

---

## Cụm 4 — 🎯🎯 Sáu memory ordering: seq_cst, relaxed, acquire-release, consume (tr. 146–164)

Sáu option, **ba mô hình (tr. 146):**
| Mô hình | Các tag | Ý |
|---|---|---|
| **Sequentially consistent** | `seq_cst` (mặc định) | Một total order toàn cục, dễ hiểu nhất, đắt nhất |
| **Acquire-release** | `acquire`, `release`, `acq_rel`, (`consume`) | Đồng bộ **cặp đôi**, không total order, rẻ hơn |
| **Relaxed** | `relaxed` | Không synchronizes-with, chỉ giữ modification order từng biến |

Chi phí khác nhau theo kiến trúc (tr. 146–147): **x86/x86-64 gần như miễn phí acquire-release** (và seq_cst chỉ tốn nhẹ ở store); kiến trúc weakly-ordered (ARM, POWER) thì seq_cst đắt hơn acquire-release đắt hơn relaxed.

### 4.1 Sequentially consistent — total order toàn cục (tr. 147–149)

**Định nghĩa (tr. 147):** mọi thao tác `seq_cst` hành xử *"as if all these operations were performed in some particular sequence by a single thread"* — **mọi thread thấy CÙNG một thứ tự**. Không reorder được. seq_cst store synchronizes-with seq_cst load đọc giá trị đó, **và mạnh hơn:** mọi thao tác seq_cst sau load đó cũng phải **xuất hiện sau** store với mọi thread khác.

**Listing 5.4 — total ordering (tr. 148–149):**
```cpp
std::atomic<bool> x, y;
std::atomic<int> z;
void write_x()        { x.store(true, std::memory_order_seq_cst); }
void write_y()        { y.store(true, std::memory_order_seq_cst); }
void read_x_then_y()  { while (!x.load(std::memory_order_seq_cst)); if (y.load(std::memory_order_seq_cst)) ++z; }
void read_y_then_x()  { while (!y.load(std::memory_order_seq_cst)); if (x.load(std::memory_order_seq_cst)) ++z; }
// 4 thread chạy 4 hàm trên, sau join:
assert(z.load() != 0);   // ⟹ KHÔNG BAO GIỜ fire
```
**Vì sao `z != 0` (tr. 148–149):** phải có total order → store-x hoặc store-y xảy ra trước. Nếu `read_x_then_y` thấy `y==false` thì store-x < store-y trong total order → `read_y_then_x` thấy `x==true` (vì đã qua `while(!y)`). Đối xứng. → ít nhất một hàm `++z`. **Cả hai có thể true → z=2; nhưng z=0 là bất khả.**

### 4.2 Relaxed — gần như không ràng buộc (tr. 149–155)

**Định nghĩa (tr. 150):** relaxed **không** tham gia synchronizes-with. Trong **cùng thread**, thao tác trên **cùng biến** vẫn giữ happens-before (không đọc được giá trị cũ hơn giá trị đã thấy). Nhưng **không ràng buộc gì với thread khác** ngoài modification order từng biến.

**⚠️ Câu phải khắc cốt (tr. 150):** *"**threads don't have to agree on the order of events**"* — không còn total order toàn cục, các thread thấy **view khác nhau** của cùng các thao tác. Phải vứt bỏ mô hình "interleaving" lẫn mô hình "compiler/CPU reorder lệnh".

**Listing 5.5 — assert CÓ THỂ fire (tr. 150–152):**
```cpp
std::atomic<bool> x, y;
std::atomic<int> z;
void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);    // (b)
    y.store(true, std::memory_order_relaxed);    // (c)
}
void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed));  // (d) chờ y==true
    if (x.load(std::memory_order_relaxed))       // (e) ⚠️ có thể đọc x==false!
        ++z;
}
// assert(z != 0);   ← CÓ THỂ FIRE
```
**Vì sao (e) có thể thấy `x==false` dù (b) happens-before (c) trong thread ghi:** `x` và `y` là **biến khác nhau**, relaxed không tạo ordering **giữa** biến khác nhau. (d) thấy `y==true` **không kéo theo** (e) thấy `x==true`.

**🆕 Ẩn dụ "người trong cubicle" của sách (tr. 153–155) — rất đáng nhớ:** mỗi biến atomic là **một người trong phòng kín với cuốn sổ** ghi danh sách giá trị. Bạn gọi điện bảo ghi (store: viết xuống cuối) hoặc hỏi giá trị (load: đọc một số). Ông ta cho bạn **bất kỳ giá trị nào từ chỗ bạn đang đứng trở xuống** (không lùi lên). Với relaxed: **mỗi người một cuốn sổ riêng, KHÔNG liên hệ gì nhau**, và ông ta có thể nói với bạn số khác với số nói cho người khác (mỗi người một sticky note). → Nhiều biến = **cả một dãy phòng độc lập**.

**Khuyến nghị mạnh của sách (tr. 155):** *"I strongly recommend **avoiding relaxed atomic operations** unless they're absolutely necessary, and even then using them only with extreme caution."*

### 4.3 Acquire-release — đồng bộ cặp đôi (tr. 155–161)

**Định nghĩa (tr. 155–156):** không total order, nhưng có đồng bộ. Load = **acquire** (`memory_order_acquire`); store = **release** (`memory_order_release`); RMW = acquire/release/cả hai (`acq_rel`). **Đồng bộ theo cặp** giữa thread release và thread acquire:

> *"A **release operation synchronizes-with an acquire operation that reads the value written**."*

**Listing 5.7 — acquire-release KHÔNG cho total order (tr. 156–157):** đổi Listing 5.4 sang acquire/release → **assert `z!=0` CÓ THỂ fire**, vì `x` và `y` do hai thread khác nhau ghi, release→acquire trên biến này không ảnh hưởng thao tác trên biến kia. Hai thread đọc có thể thấy **view khác nhau**.

**Listing 5.8 — acquire-release ÁP ordering lên relaxed (tr. 157–158), ĐÂY là chỗ thấy giá trị:**
```cpp
std::atomic<bool> x, y;
std::atomic<int> z;
void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);    // (B) relaxed
    y.store(true, std::memory_order_release);    // (c) RELEASE
}
void read_y_then_x() {
    while (!y.load(std::memory_order_acquire));  // (d) ACQUIRE — chờ y==true
    if (x.load(std::memory_order_relaxed))       // (e) relaxed
        ++z;
}
// assert(z != 0);   ← KHÔNG BAO GIỜ fire
```
**Chuỗi lập luận (tr. 158) — mẫu vàng cần thuộc:**
```
(B) store x  ──happens-before──►  (c) store y release      [cùng thread, sequenced-before]
(c) release  ──synchronizes-with─►  (d) acquire đọc true    [release/acquire, đúng giá trị]
(d) acquire  ──happens-before──►  (e) load x                [cùng thread, sequenced-before]
────────────────────────────────────────────────────────────  [bắc cầu]
⟹ (B) store x  happens-before  (e) load x  →  (e) đọc true, assert không fire
```
**⚠️ Điều kiện then chốt (tr. 158):** acquire và release **phải thành cặp trên CÙNG biến**, và acquire phải **thấy giá trị** mà release ghi. Nếu (d) không trong `while` (đọc `y==false`) thì không có ràng buộc gì lên `x`. Nếu (c) hoặc (d) đổi thành relaxed → mất ordering, assert có thể fire.

**🆕 Ẩn dụ mở rộng (tr. 158–159):** mỗi store thuộc một **"batch"**; store-release là "số cuối của batch N". Load thường (relaxed) chỉ hỏi số; **load-acquire** hỏi "số này có phải cuối batch nào không". Khi bạn báo cho ông ta **các batch bạn biết**, ông ta phải tìm giá trị cuối của batch đó **trở xuống** → ép ông ta cho giá trị mới. Đây chính là cơ chế release→acquire truyền hiệu ứng.

**Liên hệ mutex (tr. 161):** *"locking a mutex is an **acquire** operation, and unlocking the mutex is a **release** operation."* Acquire/release phải trên **cùng biến** — y như "cùng mutex khi đọc và khi ghi". Nếu xây lock từ acquire/release atomic thì code dùng lock **thấy như seq_cst** dù nội bộ không phải.

### 4.4 Transitive synchronization — Listing 5.9 (tr. 159–161)

Acquire-release **bắc cầu qua thread trung gian không chạm dữ liệu:**
```cpp
std::atomic<int> data[5];
std::atomic<bool> sync1(false), sync2(false);

void thread_1() {
    data[0].store(42, std::memory_order_relaxed);  // ... 5 giá trị relaxed
    data[4].store(2003, std::memory_order_relaxed);
    sync1.store(true, std::memory_order_release);  // (B) release
}
void thread_2() {
    while (!sync1.load(std::memory_order_acquire));  // (c) acquire ← sync với (B)
    sync2.store(true, std::memory_order_release);    // (d) release
}
void thread_3() {
    while (!sync2.load(std::memory_order_acquire));  // (e) acquire ← sync với (d)
    assert(data[0].load(std::memory_order_relaxed) == 42);  // KHÔNG fire
    // ... mọi assert data[i] KHÔNG fire
}
```
Chuỗi (tr. 160): `stores data → sync1(B) → load sync1(c) → store sync2(d) → load sync2(e) → loads data`. thread_2 **chỉ chạm sync1, sync2** nhưng đủ để nối thread_1 với thread_3. Có thể gộp `sync1`+`sync2` thành một biến bằng `compare_exchange_strong` với `acq_rel` ở thread_2.

**⚠️ Chọn đúng semantics cho RMW (tr. 161):** `fetch_sub` với **acquire** *không synchronize với gì* (không phải release op dù có store); store *không synchronize với* `fetch_or` **release** (phần đọc của fetch_or không phải acquire). **`acq_rel` = vừa acquire vừa release.**

### 4.5 ⚠️ `memory_order_consume` — ĐỪNG DÙNG (tr. 161–164)

`consume` liên quan **data dependency**. Nhưng sách nói thẳng (tr. 161): *"the C++17 standard explicitly recommends that you do not use it... **you should not use `memory_order_consume` in your code!**"*

Ý tưởng: `consume` là acquire **giới hạn ở dữ liệu phụ thuộc trực tiếp** — dùng khi atomic load **một con trỏ tới dữ liệu** (Listing 5.10): con trỏ store-release + load-consume → dữ liệu **trỏ tới** được đồng bộ, nhưng dữ liệu **không phụ thuộc** thì không. `std::kill_dependency()` phá dependency chain. **Chốt (tr. 163):** *"you should always use `memory_order_acquire` where you might be tempted to use `memory_order_consume`."*

### Insight đáng nhớ (Cụm 4)
- **seq_cst: mọi thread thấy một total order → dễ suy luận, mặc định, đắt.** relaxed: **thread không đồng ý thứ tự** → cực khó, chỉ giữ modification order từng biến. acquire-release: **đồng bộ cặp đôi** trên cùng biến, không total order, rẻ hơn seq_cst.
- **Mẫu vàng acquire-release (Listing 5.8):** `ghi data (relaxed/thường) → store-release cờ` ⟷ `load-acquire cờ → đọc data`. release/acquire **cùng biến**, acquire **thấy giá trị** release ghi. Đây là cách publish dữ liệu rẻ hơn seq_cst.
- **mutex lock = acquire, unlock = release.** Đây là câu trả lời khi được hỏi "acquire/release liên quan gì tới mutex". Xây lock từ acquire/release → code dùng lock thấy như seq_cst.
- **Đừng dùng relaxed trừ khi bắt buộc; đừng bao giờ dùng consume** (standard khuyến nghị dùng acquire thay thế).

---

## Cụm 5 — Release sequence, fences, và ordering non-atomic (tr. 164–172)

### 5.1 Release sequence — nhiều consumer, một producer (tr. 164–166)

**Vấn đề (tr. 165):** producer `count.store(N, release)`; nhiều consumer `count.fetch_sub(1, acquire)` để "nhận" một item rồi đọc buffer. Với **một** consumer thì store-release synchronizes-with fetch_sub-acquire — ok. Với **hai** consumer, fetch_sub thứ hai thấy giá trị do fetch_sub **thứ nhất** ghi (không phải store gốc) → **nếu không có luật release sequence**, consumer thứ hai không có happens-before với producer → data race.

**Luật release sequence (tr. 164):** nếu store tag `release`/`acq_rel`/`seq_cst`, load tag `consume`/`acquire`/`seq_cst`, và **mỗi thao tác trong chuỗi đọc giá trị do thao tác trước ghi**, thì chuỗi là **release sequence** và **store gốc synchronizes-with load cuối**. **Các RMW trong chuỗi có thể mang BẤT KỲ ordering nào (kể cả relaxed).**

**Listing 5.11 (tr. 164–165):**
```cpp
std::vector<int> queue_data;
std::atomic<int> count;
void populate_queue() {
    unsigned const number_of_items = 20;
    queue_data.clear();
    for (unsigned i = 0; i < number_of_items; ++i) queue_data.push_back(i);
    count.store(number_of_items, std::memory_order_release);   // (B) store gốc
}
void consume_queue_items() {
    while (true) {
        int item_index;
        if ((item_index = count.fetch_sub(1, std::memory_order_acquire)) <= 0) {  // (c) RMW
            wait_for_more_items();
            continue;
        }
        process(queue_data[item_index - 1]);   // (e) đọc buffer AN TOÀN
    }
}
```
**Nhờ release sequence (tr. 165):** fetch_sub thứ nhất **tham gia release sequence** → `store()` synchronizes-with **cả** fetch_sub thứ hai (dù giữa hai consumer **không** có synchronizes-with). Không cần cho fetch_sub mang release (tránh đồng bộ thừa giữa hai consumer).

### 5.2 Fences (memory barriers) (tr. 166–168)

**Fence = thao tác ép ordering mà KHÔNG sửa dữ liệu (tr. 166)**, thường kết với thao tác `relaxed`. Fence là **global**, đặt "một lằn ranh trong code" mà thao tác không vượt qua.

**Listing 5.12 — fence sắp thứ tự các thao tác relaxed (tr. 166–167):**
```cpp
std::atomic<bool> x, y;
std::atomic<int> z;
void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);            // (B)
    std::atomic_thread_fence(std::memory_order_release); // (c) RELEASE FENCE
    y.store(true, std::memory_order_relaxed);            // (d)
}
void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed));          // (e)
    std::atomic_thread_fence(std::memory_order_acquire); // (f) ACQUIRE FENCE
    if (x.load(std::memory_order_relaxed))               // (g)
        ++z;
}
// assert(z != 0);   ← KHÔNG fire
```
**Cơ chế (tr. 167):** release fence (c) synchronizes-with acquire fence (f) vì (e) đọc giá trị (d) ghi → (B) store x happens-before (g) load x → đọc `true`. **⚠️ Cần CẢ HAI fence** (release ở thread này, acquire ở thread kia).

**⚠️ Điểm đồng bộ là CHÍNH FENCE, không phải store (tr. 167–168):** nếu chuyển `x.store` **sau** release fence thì (B) và (d) không còn bị fence tách → mất ordering, assert có thể fire. Fence chỉ áp ordering khi nó **nằm GIỮA** hai thao tác.

### 5.3 Ordering NON-ATOMIC bằng atomic — mục đích cuối cùng (tr. 168–169)

**Listing 5.13:** thay `x` bằng **`bool` thường** (non-atomic) — hành vi **vẫn y hệt**:
```cpp
bool x = false;                              // ⚠️ NON-ATOMIC
std::atomic<bool> y;
std::atomic<int> z;
void write_x_then_y() {
    x = true;                                // (B) ghi non-atomic
    std::atomic_thread_fence(std::memory_order_release);
    y.store(true, std::memory_order_relaxed);
}
void read_y_then_x() {
    while (!y.load(std::memory_order_relaxed));
    std::atomic_thread_fence(std::memory_order_acquire);
    if (x) ++z;                              // (e) đọc non-atomic — KHÔNG data race
}
// assert(z != 0);   ← KHÔNG fire
```
Fence ép ordering lên `x` (non-atomic) một khi thread đọc đã thấy `y`. `y` **vẫn phải atomic** (nếu không thì data race trên `y`), nhưng `x` thì fence bảo vệ. **Đây là mục đích thật của atomic: ép ordering lên non-atomic để tránh data race** (tr. 168).

### 5.4 Mọi cơ chế đồng bộ đều quy về synchronizes-with (tr. 169–172)

Spinlock (Listing 5.1): `lock()` = acquire op, `unlock()` = release op trên cùng memory location → `unlock()` synchronizes-with `lock()` sau đó. Sách liệt kê **quan hệ synchronizes-with của mọi facility** (tr. 170–172):
- **`std::thread`:** ctor xong synchronizes-with hàm chạy trên thread mới; thread xong synchronizes-with `join()` trả về.
- **mutex:** `unlock` synchronizes-with `lock` sau đó (trong lock order). `try_lock` **fail không** tham gia đồng bộ.
- **promise/future:** `set_value`/`set_exception` synchronizes-with `wait`/`get` trả về.
- **`async` (launch::async):** thread task xong synchronizes-with `wait`/`get`.
- **latch:** `count_down` synchronizes-with `wait`.
- **⚠️ condition_variable:** *"do NOT provide any synchronization relationships. They are optimizations over busy-wait loops; all the synchronization is provided by the operations on the associated **mutex**."*

### Insight đáng nhớ (Cụm 5)
- **Release sequence** cho phép **một** store-release đồng bộ với **cả chuỗi RMW** (dù RMW mang relaxed) → nhiều consumer rút từ một counter mà không cần đồng bộ thừa giữa chúng.
- **Fence = lằn ranh, đồng bộ tại CHÍNH nó, cần cả release-fence lẫn acquire-fence.** Dùng để sắp thứ tự các thao tác relaxed và **non-atomic**.
- **Đích cuối của chương: dùng atomic ép ordering lên NON-ATOMIC** (Listing 5.13) → đây là lý do memory model tồn tại. mutex/future/latch đều xây trên synchronizes-with; **condition_variable thì KHÔNG** — nó dựa vào mutex đi kèm.

---

## Tóm tắt chương (theo sách, tr. 172)

Chương này phủ chi tiết low-level của C++ memory model và atomic operations — nền của đồng bộ giữa thread: các atomic type (specialization + primary template `std::atomic<>`), thao tác trên chúng, và chi tiết phức tạp của các memory-ordering option; **fences** và cách ghép với atomic để ép ordering; và cuối cùng quay về **dùng atomic ép ordering giữa non-atomic operations**, cùng bảng quan hệ đồng bộ của các facility cấp cao.

**Câu chuyển sang ch. 6 (tr. 172):** giờ dùng các facility cấp cao **cùng** atomic để thiết kế **container hiệu quả cho truy cập concurrent** và **thuật toán xử lý song song**.

**🆕 Bảng chốt — 6 memory order:**
| Tag | Loại op | synchronizes-with? | Ghi chú |
|---|---|---|---|
| `seq_cst` (mặc định) | mọi | Có + **total order toàn cục** | Dễ nhất, đắt nhất |
| `acquire` | load, RMW | Có (nếu đọc giá trị của release) | Ngăn reorder sau nó lùi lên trước |
| `release` | store, RMW | Có (với acquire đọc giá trị) | Ngăn reorder trước nó xuống sau |
| `acq_rel` | RMW | Cả acquire lẫn release | Cho RMW cần cả hai chiều |
| `relaxed` | mọi | **KHÔNG** | Chỉ giữ modification order từng biến |
| `consume` | load, RMW | data-dependency (⚠️ **đừng dùng**) | Standard khuyên dùng `acquire` |

---

## Góc interview

**Câu 1 (🎯🎯 phân loại senior):** Giải thích **happens-before** và **synchronizes-with**. Cho đoạn "publish dữ liệu qua một cờ", chứng minh vì sao không data race dù dữ liệu là non-atomic.

<details><summary>Đáp án</summary>

**Hai định nghĩa (tr. 143–146):**
- **synchronizes-with** — chỉ có **giữa thao tác trên atomic type**. Một atomic **store-release** trên `x` **synchronizes-with** một atomic **load-acquire** trên `x` mà **đọc đúng giá trị store đó ghi** (hoặc giá trị của release sequence tiếp theo). Đây là **cây cầu duy nhất nối hai thread**.
- **happens-before** — quan hệ "A thấy hiệu ứng của B". Trong một thread: A **sequenced-before** B (A ở câu lệnh trước) ⟹ A happens-before B. Giữa thread: A **synchronizes-with** B ⟹ A inter-thread-happens-before B. **Bắc cầu.**

**Đoạn code (Listing 5.2):**
```cpp
std::vector<int> data;                  // NON-ATOMIC
std::atomic<bool> data_ready(false);
void writer() {
    data.push_back(42);                 // (d)
    data_ready = true;                  // (e) store atomic
}
void reader() {
    while (!data_ready.load());         // (b) load atomic
    std::cout << data[0];               // (c) đọc non-atomic
}
```
**Chứng minh không data race:**
```
(d) ghi data ──happens-before──► (e) store data_ready   [sequenced-before, cùng thread writer]
(e) store true ──synchronizes-with──► (b) load đọc true  [atomic store-release / load-acquire cùng biến]
(b) load true ──happens-before──► (c) đọc data           [sequenced-before, cùng thread reader]
─────────────────────────────────────────────────────── [happens-before BẮC CẦU]
⟹ (d) ghi data  happens-before  (c) đọc data
```
Vì có happens-before giữa ghi và đọc `data`, **không có data race** dù `data` là non-atomic → reader thấy `42`. Đây là **mẫu publish kinh điển**: `ghi data (sequenced) → store cờ atomic (sync-with) load cờ → (sequenced) đọc data`. Chỉ cần **một** synchronizes-with là đủ để mọi thứ ghi trước đó ở writer hiện ra với reader.

**Điểm cộng:** nêu rằng đây chính là cách mutex hoạt động — `unlock` (release) synchronizes-with `lock` (acquire) sau đó; mọi thứ sửa trong critical section trước `unlock` được thấy bởi thread `lock` tiếp theo (tr. 169).

</details>

**Câu 2 (🎯🎯):** So sánh `memory_order_seq_cst`, `acquire`/`release`, và `relaxed`. Cho ví dụ acquire-release **áp** ordering lên biến relaxed. Khi nào seq_cst khác acquire-release?

<details><summary>Đáp án</summary>

| | `seq_cst` | acquire-release | `relaxed` |
|---|---|---|---|
| Total order toàn cục | **Có** — mọi thread thấy cùng thứ tự | **Không** | Không |
| synchronizes-with | Có | Có (**cặp** release↔acquire cùng biến) | **Không** |
| Chi phí | Cao nhất | Trung bình (miễn phí trên x86) | Thấp nhất |
| Suy luận | Dễ nhất | Khó | Rất khó |

**Ví dụ acquire-release áp ordering lên relaxed (Listing 5.8):**
```cpp
void write_x_then_y() {
    x.store(true, std::memory_order_relaxed);    // (B) relaxed
    y.store(true, std::memory_order_release);    // (c) RELEASE
}
void read_y_then_x() {
    while (!y.load(std::memory_order_acquire));  // (d) ACQUIRE
    if (x.load(std::memory_order_relaxed)) ++z;  // (e) relaxed
}
// assert(z != 0) KHÔNG fire
```
Chuỗi: (B) *sequenced-before* (c); (c) *release* **synchronizes-with** (d) *acquire* (vì (d) đọc `true`); (d) *sequenced-before* (e). Bắc cầu ⟹ (B) store x **happens-before** (e) load x ⟹ đọc `true`. **Dù `x` là relaxed**, nó "đi ké" cặp release-acquire trên `y`. **⚠️ Điều kiện:** (d) phải **thấy giá trị** (c) ghi (nhờ `while`), và release/acquire **cùng biến `y`**.

**Khác biệt seq_cst vs acquire-release (Listing 5.4 vs 5.7):**
- **seq_cst:** với hai biến `x`, `y` ghi bởi hai thread và hai thread đọc chéo, `assert(z != 0)` **không bao giờ fire** — vì total order buộc mọi thread đồng ý store-x hay store-y trước.
- **acquire-release:** cùng code, `assert(z != 0)` **CÓ THỂ fire** — vì không có total order, hai thread đọc có thể thấy **view khác nhau** (cái thấy x trước, cái thấy y trước). Đây chính là chỗ seq_cst mạnh hơn: nó thêm ràng buộc **giữa các biến khác nhau** mà acquire-release không có.

**Chốt:** dùng seq_cst khi cần mọi thread đồng ý thứ tự **giữa nhiều biến**; dùng acquire-release khi chỉ cần **publish qua một biến** (rẻ hơn). Tránh relaxed trừ khi bắt buộc (counter thuần, không đồng bộ dữ liệu khác).

</details>

**Câu 3 (🎯):** Phân biệt `compare_exchange_weak` và `compare_exchange_strong`. Vì sao `weak` phải dùng trong vòng lặp? Viết một atomic counter tăng bằng CAS.

<details><summary>Đáp án</summary>

**Khác biệt (tr. 135–136):**
| | `weak` | `strong` |
|---|---|---|
| Fail **giả (spurious)** dù `== expected`? | **Có** — trên máy thiếu lệnh CAS đơn, thread bị OS switch giữa chừng chuỗi lệnh | **Không** — chỉ fail khi thật sự `!= expected` |
| Dùng trong loop | **Bắt buộc** | Không nhất thiết |
| Chọn khi | Tính giá trị store **rẻ** (tránh double-loop: strong có thể tự chứa loop trên nền tảng weak) | Tính giá trị store **tốn** (khỏi tính lại khi expected chưa đổi) |

**Vì sao weak phải trong loop:** nó có thể trả `false` **dù giá trị đúng bằng expected** (spurious failure — do timing, không do giá trị). Phải lặp để phân biệt "fail giả" với "thread khác ghi trước":
```cpp
bool expected = false;
while (!b.compare_exchange_weak(expected, true) && !expected);
// lặp khi expected VẪN false (fail giả); dừng khi thành công HOẶC expected==true (thread khác ghi trước)
```

**Atomic counter tăng bằng CAS (mẫu read-modify-write kinh điển):**
```cpp
std::atomic<int> counter{0};
void increment() {
    int old = counter.load(std::memory_order_relaxed);
    while (!counter.compare_exchange_weak(
               old, old + 1,
               std::memory_order_relaxed)) {
        // compare_exchange_weak tự cập nhật 'old' = giá trị hiện tại khi fail
        // → vòng sau tính lại old+1; không cần load() lại
    }
}
```
Điểm ăn điểm: khi CAS fail, nó **tự nạp `old` = giá trị hiện tại** → vòng sau tính `old+1` từ giá trị mới nhất. (Với counter đơn giản thì `counter.fetch_add(1)` gọn hơn và cũng atomic; CAS-loop dùng khi giá trị mới **phụ thuộc** giá trị cũ theo cách phức tạp.)

**⚠️ Bẫy nên nêu:** compare-exchange so sánh **bitwise (như `memcmp`)**, không dùng `operator==`. Với `atomic<float>`/`atomic<UDT>` có padding, có thể **fail dù giá trị "bằng"** vì representation khác (tr. 139). Hai memory order: một cho success, một cho failure; **failure không được là release/không được strict hơn success**.

</details>

**Câu 4 (🎯):** Xây một **spinlock** từ `std::atomic_flag`. Giải thích vì sao `lock`/`unlock` cần acquire/release và điều đó bảo đảm gì.

<details><summary>Đáp án</summary>

```cpp
class spinlock_mutex {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;   // bắt buộc, bắt đầu ở clear
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire));   // quay tới khi set được (old==false)
    }
    void unlock() {
        flag.clear(std::memory_order_release);                  // nhả
    }
};
```

**Cơ chế (tr. 133–134, 169):**
- `test_and_set()` là **RMW**: set flag = true, trả **giá trị cũ**. Nếu cũ là `false` → thread này vừa giành được lock (thoát loop). Nếu cũ là `true` → thread khác đang giữ, quay tiếp (busy-wait).
- `clear()` đặt flag về `false` = nhả lock.

**Vì sao acquire/release (tr. 169):**
- `unlock()` (`clear` với **release**) **synchronizes-with** `lock()` (`test_and_set` với **acquire**) tiếp theo của thread khác — vì lock đọc đúng "trạng thái được nhả".
- Hệ quả: mọi sửa đổi dữ liệu trong critical section **sequenced-before `unlock()`** → **happens-before `unlock()`** → **happens-before `lock()`** của thread sau (nhờ synchronizes-with) → **happens-before mọi truy cập dữ liệu** của thread sau. **Đây chính là bảo đảm của một mutex:** thread vào sau thấy được mọi thay đổi thread trước làm trong critical section.

**Điểm cộng:**
- **`atomic_flag` là atomic type DUY NHẤT bảo đảm lock-free** (tr. 132) — nền để xây mọi thứ khác.
- **⚠️ Nhược:** `lock()` **busy-wait** → phí CPU, kém khi contention cao; chỉ hợp critical section **cực ngắn** (spinlock hạt nhân, không dùng cho vùng chờ lâu). Với contention cao dùng `std::mutex` (có thể block/park thread).
- **Nguyên lý tổng quát (tr. 170):** mọi mutex — `lock()` là acquire op trên một memory location nội bộ, `unlock()` là release op trên chính nó.

</details>

**Câu 5 (🟠):** `condition_variable` có tạo quan hệ synchronizes-with không? `std::async` với `launch::async` thì sao? (Câu bẫy về nguồn gốc đồng bộ.)

<details><summary>Đáp án</summary>

**`condition_variable`: KHÔNG (tr. 172).** Sách nói rõ: *"Condition variables **do not provide any synchronization relationships**. They are optimizations over busy-wait loops, and **all the synchronization is provided by the operations on the associated mutex**."* → Điều làm cho code với condition variable đúng đắn là **mutex đi kèm** (mà `wait` lock/unlock), không phải bản thân CV. Đây là lý do CV **luôn phải dùng với mutex** và predicate.

**`std::async` với `launch::async`: CÓ (tr. 171).** *"The completion of the thread running a task launched via `std::async` with `launch::async` **synchronizes with** a successful return from `wait`/`get`..."* → mọi thứ task làm được thấy bởi thread gọi `get()`.

**Các facility KHÁC tạo synchronizes-with (tr. 170–172):**
- **`std::thread`:** ctor xong sync-with hàm chạy trên thread mới; thread xong sync-with `join()`.
- **mutex:** `unlock` sync-with `lock` sau đó (trong lock order); **`try_lock` fail KHÔNG** tham gia đồng bộ.
- **shared_mutex:** tương tự, gồm cả `lock_shared`/`unlock_shared`.
- **promise/future:** `set_value`/`set_exception` sync-with `wait`/`get`; destructor lưu `broken_promise` cũng sync-with.
- **packaged_task:** hoàn thành function call operator sync-with `wait`/`get`.
- **latch (TS):** `count_down` sync-with `wait`.

**Chốt:** khi được hỏi "cái gì cung cấp đồng bộ trong đoạn dùng condition variable", câu đúng là **mutex đi kèm**, không phải CV — CV chỉ là tối ưu của busy-wait. Đây là bẫy hay gặp.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [lkd/sync-timers.md](../lkd/sync-timers.md) — memory barrier trong kernel (`smp_mb`, `smp_rmb`, `smp_wmb`, `READ_ONCE`/`WRITE_ONCE`), `atomic_t`, `cmpxchg` — **cùng bài toán ordering ở tầng kernel**, ánh xạ gần với acquire/release của C++.
- [cpp-mindset/understanding-the-machine.md](../cpp-mindset/understanding-the-machine.md) — cache coherence (MESI), store buffer, vì sao CPU reorder — **nền phần cứng** giải thích vì sao relaxed cho kết quả "phản trực giác".
- [ostep/concurrency.md](../ostep/concurrency.md) — spinlock từ test-and-set/CAS ở tầng OS, cùng ý tưởng Listing 5.1.
- [EMC++ cụm 7](../effective-modern-cpp.md) — Item 40 (`std::atomic` vs `volatile` — atomic cho đồng bộ, volatile cho memory-mapped I/O, đừng nhầm).
- Ch. 7 (lock-free structures) **xây trực tiếp trên chương này** — ABA problem, hazard pointer, khi nào relaxed đủ; ch. 6 dùng lại các bảo đảm synchronizes-with này cho lock-based structures.

**Chương tiếp theo:** [Ch. 6 — Designing lock-based concurrent data structures →](06-lock-based-structures.md) (thiết kế cho concurrency; thread-safe stack/queue coarse→fine-grained; lookup table, list) — nghỉ khỏi low-level, quay lại thiết kế **container** dùng mutex.
