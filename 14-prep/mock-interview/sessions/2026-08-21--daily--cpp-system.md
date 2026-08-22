# Phiên mock — 2026-08-21 · `daily` · track `cpp-system`

- **Level:** mid-level · **Số câu:** 6 · **Trần độ sâu:** T2
- **Điểm trung bình:** **18/24 = 3.0 / 4**
- **Bối cảnh:** phiên **trả nợ retention** — 4 câu `CPP-009`/`DP-002`/`OS-003`/`OS-007` đến hạn **17–23/08**, đã bỏ qua ở 16, 17, 18, 19, 20/08. Chạy trước hạn chót 2 ngày.
- **⚠️ Cơ cấu lệch chuẩn CÓ CHỦ ĐÍCH:** `daily` mặc định có **1** slot revisit ([interview-types.md](../interview-types.md)); phiên này chạy **4 retention + 2 câu weak**, theo chỉ thị §📍 của [datalogic-plan](../../study-plans/datalogic-plan.md). Hợp lệ theo [config §⚖️](../config.md): plan quyết định *hỏi cái gì*, config quyết định *hỏi thế nào* — trần vẫn giữ **T2**.

## Kết quả từng câu (nhìn nhanh)

| # | ID | Nguồn | Góc hỏi (mới — góc cũ cấm lặp) | Điểm |
|---|----|-------|-------------------------------|------|
| 1 | CPP-009 | 🔁 retention | link error khi tách template ra `.cpp` — đọc message, chẩn đoán | **4** |
| 2 | DP-002 | 🔁 retention | Singleton trong `.so` — còn *một* instance không? | **1** 🔴 |
| 3 | OS-003 | 🔁 retention | 2 hàm lock 2 mutex ngược thứ tự → chỉ ra + **viết code sửa** | **3** |
| 4 | OS-007 | 🔁 retention | tình huống RT: task cao kẹt sau task thấp → gọi tên + cơ chế cứu | **4** |
| 5 | CPP-032 | 🔴 weak (góc 4) | `explicit` cho ctor **nhiều đối số** + braced-init | **3** |
| 6 | CPP-016 | 🔴 weak | chặn slicing **từ phía class**, không sửa call site | **3** |

**Chuyển động sổ yếu:** `CPP-032` **gỡ** (2 lần liên tiếp ≥3) · `CPP-016` 2→3 (1/2 lần) · `DP-002` **kéo về sổ yếu** (regression 4→1).

---

## 🔴 PHÁT HIỆN CHÍNH — rơi điểm vì REPO HỤT, không phải vì ứng viên quên

Ba câu retention giữ nguyên độ vững sau 2 tuần (**4 · 3 · 4**) ⇒ cơ chế spaced review **đang chạy đúng**. Toàn bộ mức sụt của phiên nằm ở **một** câu — và câu đó **bank không hề dạy**:

- Bank [DP-002](../bank/design-patterns.md) là 🟢, đáp án **3 dòng**, không có một chữ nào về `.so`.
- [11-design-patterns/creational.md §1](../../../11-design-patterns/creational.md) cũng **không nhắc shared library**.
- Cơ chế **có** trong repo nhưng nằm ở topic khác, đúng **một dòng**: [linking-loading.md:100](../../../07-shared-libraries/linking-loading.md#L100).

⇒ Hai mảnh **không nối với nhau**; người ôn Singleton không bao giờ đi tới `linking-loading.md`. Đây là mẫu **lần thứ 4** (sau `LNX-023`, `LNX-026`, `LNX-029`).

**Đã xử lý ngay trong phiên:** thêm câu bank mới **[DP-020](../bank/design-patterns.md)** (khung 5 phần, có output chạy thật), cross-link từ `DP-002`, và thêm cảnh báo + link vào `creational.md §1`.

> ✅ **KHÔNG tái diễn lỗi 19/08** (*"thuộc bài ≠ hiểu bài"*): phiên này không có chỗ nào ứng viên đọc thuộc rồi tự khai không hiểu.
> ✅ **KHÔNG tái diễn lỗi "repo không có tài liệu về X"**: phiên này không lần nào tuyên bố như vậy.

---

## 🔎 Chi tiết ôn

### Câu 1 · CPP-009 · 🟡 · **4/4** 🔁 retention

Header có template, **định nghĩa nằm ở `.cpp` riêng**:

```cpp
// stack.h
template <typename T>
class Stack {
public:
    void   push(const T& v);
    T      pop();
    bool   empty() const;
private:
    T      data_[32];
    size_t n_ = 0;
};

// stack.cpp
#include "stack.h"
template <typename T> void Stack<T>::push(const T& v) { data_[n_++] = v; }
template <typename T> T    Stack<T>::pop()            { return data_[--n_]; }
template <typename T> bool Stack<T>::empty() const    { return n_ == 0; }

// main.cpp
int main() { Stack<int> s; s.push(42); std::printf("%d\n", s.pop()); }
```

**Output thật** (`g++ -std=c++17 -Wall -Wextra`):
```
$ g++ -c stack.cpp -o stack.o     # ✅ 0 warning
$ g++ -c main.cpp  -o main.o      # ✅ 0 warning
$ g++ stack.o main.o -o app
/usr/bin/ld: main.o: in function `main':
main.cpp:(.text+0x4c): undefined reference to `Stack<int>::push(int const&)'
main.cpp:(.text+0x5b): undefined reference to `Stack<int>::pop()'
collect2: error: ld returned 1 exit status
```

**(a)** Chuyện gì đã xảy ra, theo đúng thứ tự các bước build? **(b)** `stack.cpp` compile sạch không một warning — vì sao compiler không kêu, và `stack.o` chứa gì? **(c)** Vì sao báo `push`/`pop` mà **không** báo `empty()`?

**🔁 Follow-up đã hỏi:** (d) các cách sửa + đánh đổi từng cách · (e) ràng buộc *"không phát tán mã nguồn, chỉ giao header + `.so`"* thì cách nào chết · (f) 6 tháng sau team cần `Stack<float>` — ai phải sửa · (g) *"giấu cài đặt"* và *"T tuỳ ý"* có cùng thoả được không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ compiler thấy prototype, để ngỏ, hẹn linker tìm sau.
- (b) ✅ ý đúng — `stack.o` không có định nghĩa nên link hỏng.
- (c) ✅ `main.cpp` không dùng `empty()` nên linker không đi tìm.
- (d) ✅ **cả hai cách**: định nghĩa vào header (giá: code bloat khi nhiều `T`) · **explicit instantiation** trong `.cpp` (giá: phải biết trước tập `T`).
- (e) ✅ header chết vì lộ cài đặt ⇒ chỉ còn explicit instantiation.
- (f) ✅ *"Tôi phải sửa"* — tự nhận trách nhiệm bảo trì.
- (g) ✅ kết luận đúng là hai yêu cầu mâu thuẫn; chọn explicit instantiation. ❌ không gọi tên hướng còn lại.

**✅ Được:** đi hết bốn tầng follow-up. Theo [config §6 luật ①](../config.md), câu retention **không chấm phần nền** — trọng số ở (d)–(g), và phần đó trọn vẹn. Ý (f) là chỗ phân biệt người **đã làm** với người **đã đọc**: cái giá thật của explicit instantiation là **nợ bảo trì đổ về người viết thư viện**, không phải một dòng cú pháp.

**❌ Một chỗ diễn đạt (không trừ điểm):** ở (b) nói *"không một ai cần đến định nghĩa nên `stack.o` không có"*. Chính xác hơn: compiler **không thể** sinh mã khi chưa biết `T`; nó chỉ sinh khi có **instantiation**, mà `stack.cpp` **không hề** instantiate `Stack<int>`. "Không ai cần" nghe như tối ưu bỏ bớt; thực tế là **không có gì để bỏ**.

**Đáp án đầy đủ — phần bổ sung cho (g):** hướng còn lại tên là **type erasure** — đặt một interface **không template** ở ranh giới thư viện (virtual base, hoặc lõi `void*` + hàm thao tác), rồi bọc một lớp template **mỏng trong header** cho tiện dùng. Cách này giấu được cài đặt **và** nhận `T` tuỳ ý; trả giá bằng **lời gọi gián tiếp** (mất inline). `std::function` chính là mẫu đó. Đây là T2 nâng — không biết vẫn 4 điểm, nhưng trúng trụ *"C++ shared library"* của JD.

**Chốt:** *"Template chỉ sinh mã khi được instantiate, nên đơn vị dịch dùng nó phải thấy toàn bộ định nghĩa. Muốn giấu cài đặt thì hoặc explicit instantiation (nhận nợ bảo trì tập T), hoặc bỏ template ở ranh giới và dùng type erasure."*

**Lần sau sẽ hỏi (dời +2 tuần → 04–09/09):** góc 3 — *"thư viện đang dùng explicit instantiation; giờ phải thêm `Stack<float>` mà không được build lại `.so` đã giao cho khách. Còn đường nào không?"*

**Ôn:** [bank CPP-009](../bank/cpp.md) · [01-cpp-fundamentals/templates.md](../../../01-cpp-fundamentals/templates.md) · [bank SD-026](../bank/system-design.md)
</details>

---

### Câu 2 · DP-002 · 🟢→🟠 · **1/4** 🔴 REGRESSION (4 → 1)

Logger dùng Meyers' Singleton, **định nghĩa trong header**:

```cpp
// logger.h
class Logger {
public:
    static Logger& instance() {
        static Logger inst;      // Meyers' singleton
        return inst;
    }
    void log(const char* who) {
        std::printf("[%p] count=%d  (%s)\n", (void*)this, ++count_, who);
    }
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
private:
    Logger() = default;
    int count_ = 0;
};
```

Hai shared library **đều `#include "logger.h"`**, app link cả hai:

```
app  ──┬──► libdisplay.so  ──► Logger::instance().log("display")
       └──► libsensor.so   ──► Logger::instance().log("sensor")
```

**(a)** Có **bao nhiêu** đối tượng `Logger` trong tiến trình? **(b)** Câu trả lời đó luôn đúng, hay phụ thuộc thứ gì khác? **(c)** Muốn **chắc chắn** một instance thì làm thế nào?

**🔁 Follow-up đã hỏi:** (d) hai symbol trùng tên ở hai object — dynamic linker xử lý ra sao? · (e) hai `.so` **cùng một tiến trình, cùng không gian địa chỉ** — shared memory đang giải quyết gì? · (f) dự đoán hai dòng output.

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ❌ **"2 đối tượng"** — *"2 file `.so` là 2 phiên bản độc lập"*. **Sai.**
- (b) ❌ lệch — *"nếu dùng chung 1 logger lưu dưới dạng shared memory thì chỉ có 1"*.
- (c) ❌ **shared memory**, map vào từng `.so`. (Đánh đổi tự nêu — race — thì đúng, nhưng cho một giải pháp sai tầng.)
- (d) 🟡 *"Chưa rõ. Theo tôi nó sẽ chọn một và duy trì một cái."* ← **đây chính là đáp án**, nhưng nói dưới dạng phỏng đoán và **mâu thuẫn với (a)** mà không nhận ra.
- (e) ❌ vẫn bảo lưu shared memory.
- (f) ❌ *"chưa rõ"*.

**❌ Vì sao mất điểm — ba tầng, tầng (c) nặng nhất:**
1. Kết luận T1 **đảo ngược** (2 thay vì 1).
2. Có cơ chế trong đầu ở (d) nhưng **không dám chốt**, và không thấy nó đá (a).
3. 🔴 **Chẩn đoán sai TẦNG vấn đề**: nghe *"hai `.so`"* → nhảy sang khung *"hai bên tách biệt"* → đề xuất cơ chế **liên tiến trình** cho hai lib **cùng một address space**.

**Đáp án đầy đủ — Output THẬT (đã chạy):**

```
===== MẶC ĐỊNH (default visibility) =====
[0x7b465ec6c03c] count=1  (display)
[0x7b465ec6c03c] count=2  (sensor)      ← CÙNG địa chỉ, count đi tiếp
[0x7b465ec6c03c] count=3  (display)
```
⇒ **MỘT** instance.

**Cơ chế:** `static Logger inst;` trong hàm inline ⇒ mỗi `.so` sinh **một symbol global (weak)** cùng tên mangled. Dynamic linker nạp vào **một** tiến trình, gặp symbol trùng tên ⇒ **symbol interposition**: hợp nhất về **một** định nghĩa, cái xuất hiện trước trong thứ tự tìm kiếm thắng (cùng cơ chế `LD_PRELOAD`).

⭐ **Điểm mấu chốt:** tính duy nhất ở đây **không do C++ bảo đảm** — chuẩn chỉ nói về *một chương trình*, còn "chương trình gồm mấy `.so`" là chuyện của **linker**.

**Ba thứ lật ngược kết quả** (không cái nào là shared memory):

| Thứ | Vì sao | Kết quả |
|---|---|---|
| **`-fvisibility=hidden`** | Symbol không vào bảng động ⇒ không có gì để hợp nhất | **mỗi `.so` một instance** |
| **`dlopen(..., RTLD_LOCAL)`** + symbol đã ẩn | Vùng symbol riêng | mỗi lần nạp một instance |
| **Link tĩnh** logger vào từng `.so` | Mỗi `.so` mang sẵn một bản | nhiều instance |

Chạy thật với `-fvisibility=hidden` (giấu `Logger`, chỉ export entry point):
```
[0x7498fbfc602c] count=1  (display)
[0x7498fbfc102c] count=1  (sensor)      ← địa chỉ KHÁC, count riêng
[0x7498fbfc602c] count=2  (display)
```
⇒ **2 instance — nhưng vì visibility, không phải vì "hai `.so` là hai thứ độc lập".**

**Cách đúng cho (c):** export **có chủ đích** (`__attribute__((visibility("default")))` cho `Logger`), hoặc chắc hơn — **đặt logger vào MỘT `.so` riêng** để cả hai lib cùng link vào, khi đó chỉ tồn tại đúng một định nghĩa, không phải trông vào interposition.

**Chốt:** *"Mặc định là MỘT — nhưng nhờ symbol interposition của dynamic linker, không phải nhờ C++. `-fvisibility=hidden`, `RTLD_LOCAL` hay link tĩnh xen vào là thành nhiều."*

**⚖️ Repo có lỗi thật (lần thứ 4):** bank `DP-002` 🟢 3 dòng + `creational.md` đều không nhắc `.so`; cơ chế nằm ở [linking-loading.md:100](../../../07-shared-libraries/linking-loading.md#L100) đúng **một dòng**, không nối sang. **Đã vá trong phiên:** thêm [DP-020](../bank/design-patterns.md), cross-link `DP-002`, thêm cảnh báo vào `creational.md §1`.

**Lần sau sẽ hỏi (sổ yếu, T1 trước — [config §6](../config.md) cấm đưa câu ≤2 lên T2):** *"cùng một binary, cùng một tiến trình — vì sao `-fvisibility=hidden` lại đổi được SỐ LƯỢNG object trong chương trình?"* Kiểm đúng chỗ đã hụt: **ai** quyết định tính duy nhất.

**Ôn:** [bank DP-020](../bank/design-patterns.md) ⭐ (mới) · [linking-loading.md §symbol interposition](../../../07-shared-libraries/linking-loading.md#L100) · [creational.md §1](../../../11-design-patterns/creational.md)
</details>

---

### Câu 3 · OS-003 · 🟡 ⭐ · **3/4** 🔁 retention

```cpp
struct Panel {
    std::mutex m;
    int  brightness = 50;
    std::string name;
};

void balance(Panel& a, Panel& b) {
    std::lock_guard<std::mutex> la(a.m);
    std::lock_guard<std::mutex> lb(b.m);
    int avg = (a.brightness + b.brightness) / 2;
    a.brightness = avg;
    b.brightness = avg;
}

// thread 1: balance(left, right);
// thread 2: balance(right, left);
```
Chạy 8 tiếng ở lab, service **treo cứng đúng một lần** — không crash, không log.

**(a)** Dựng lại kịch bản theo trình tự thời gian. **(b)** ✍️ **Viết code sửa** vào `coding-arena/balance_fix.cpp`. **(c)** Cách sửa phá **điều kiện Coffman nào**? Nếu mai có `balance3(...)` ba panel thì còn đỡ được không?

**🔁 Follow-up đã hỏi:** (d) 16 panel / 8 thread cặp rời nhau — bản sửa của bạn ảnh hưởng hiệu năng ra sao? · (e) bạn **xoá `std::mutex m` khỏi `Panel`** — `read_brightness(Panel&)` khoá `p.m` ở nơi khác thì sao? · (f) có cách **giữ mutex riêng từng panel** mà vẫn không deadlock không? · (g) `scoped_lock` làm gì **bên trong**, phá điều kiện nào?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ chuẩn: T1 lock `a` chờ `b`; T2 lock `b` **trước khi T1 kịp**, chờ `a`.
- (b) 🟡 **bản 1** — thay toàn bộ bằng **một `std::mutex g_mtx` global**, xoá `Panel::m`.
- (c) ✅ *"phá circular wait bằng 1 khoá global duy nhất"* · `balance3` vẫn đỡ được.
- (d) ✅ tự nêu: *"một thread ghi thì các thread khác phải chờ → overhead cao"*.
- (e) ✅ tự nêu: *"phải sửa lại toàn bộ; hàng trăm hàm thì tốn thời gian"*.
- (f) ✅ **`std::scoped_lock`** — và **viết lại code** đúng.
- (g) ❌ *"có thuật toán đảm bảo thứ tự lock phù hợp"* + *"phá circular wait"* — **cả hai đều lệch**.

**✅ Được:** ý (a) chuẩn. Đặc biệt (d)+(e): **tự soi ra cái giá của chính bản sửa mình vừa viết**, không cần tôi chỉ. Rất ít người mid-level làm việc này.

**Bản nộp cuối — compile sạch, đã chạy thật** (`g++ -std=c++17 -Wall -Wextra` → exit 0, 0 warning):
```cpp
void balance(Panel& a, Panel& b) {
    std::scoped_lock lck(a.m, b.m);   // ✅
    int avg = (a.brightness + b.brightness) / 2;
    a.brightness = avg;
    b.brightness = avg;
}
```

**❌ Vì sao chưa 4 — ý (g), cơ chế bên trong:**

`std::scoped_lock` (qua `std::lock`) **không sắp thứ tự gì cả**. Nó chạy **try-and-backoff**: khoá một cái, `try_lock` phần còn lại; hễ một cái trượt thì **nhả sạch tất cả** rồi thử lại. Vì nó **không bao giờ giữ một lock trong lúc chờ lock khác**, thứ nó phá là **hold-and-wait**, **không phải** circular wait.

**Vì sao phân biệt này có giá trị thật:**

| Cách | Phá điều kiện | Phạm vi hiệu lực |
|---|---|---|
| **Lock ordering** thủ công | **Circular wait** | Toàn cục — đòi **mọi người trong codebase tuân thủ mãi mãi** |
| **`std::scoped_lock`** | **Hold-and-wait** | **Cục bộ** — đúng ngay tại chỗ dùng, không cần ai hợp tác |
| **Một khoá global** (bản 1 của bạn) | Mutual exclusion còn 1 tài nguyên ⇒ không thể có vòng | Đúng, nhưng **giết song song** |

⇒ Đó chính là lý do `scoped_lock` là câu trả lời tốt hơn cho `balance3`.

> **Bank [OS-003](../bank/os.md):**
> | **Hold-and-wait** | Giữ lock này rồi mới đi xin lock kia | Lấy **tất cả** lock một lần: `std::scoped_lock(m1, m2)` |
> | **Circular wait** | Đồ thị chờ tạo thành vòng | ⭐ **Lock ordering** — mọi nơi luôn khoá theo **cùng một thứ tự toàn cục** |

> **Tài liệu gốc** [sync-primitives.md:176](../../../03-operating-system/sync-primitives.md#L176):
> *"Cần khoá nhiều mutex cùng lúc: **`std::scoped_lock(a, b)`** — nó chống deadlock bằng **thuật toán tránh**, an toàn hơn tự lock theo thứ tự."*
>
> ⚠️ **Ghi nhận công bằng cho ứng viên:** doc chỉ nói *"thuật toán tránh"*, **không** giải thích try-and-backoff, **không** gọi tên hold-and-wait. Bank có, doc không.

**Chốt:** *"`scoped_lock` không sắp thứ tự — nó lấy tất-cả-hoặc-không-gì, nên phá hold-and-wait. Lock ordering mới là cái phá circular wait, và nó đòi cả codebase tuân thủ."*

**Lần sau sẽ hỏi (dời +2 tuần → 04–09/09):** góc 3 — *"`std::lock` phải thử lại nhiều lần. Nếu hai thread cứ nhả–thử–nhả–thử mãi thì gọi là hiện tượng gì, và có xảy ra thật không?"* (livelock, và vì sao thực tế nó hội tụ).

**Ôn:** [bank OS-003](../bank/os.md) · [sync-primitives.md](../../../03-operating-system/sync-primitives.md)
</details>

---

### Câu 4 · OS-007 · 🟡 ⭐ · **4/4** ⭐ câu tốt nhất phiên · 🔁 retention

Linux `SCHED_FIFO`, ba luồng:

| Luồng | Ưu tiên | Việc |
|---|---|---|
| **H** | 80 | Đọc cảm biến, hạn **10 ms** cứng |
| **M** | 50 | Xử lý ảnh, tốn CPU, **không đụng gì tới H hay L** |
| **L** | 10 | Ghi log xuống flash |

`H` và `L` dùng chung buffer, bảo vệ bằng **binary semaphore** (`sem_init(&sem,0,1)`) — người viết trước chọn vì *"nó cũng khoá được mà linh hoạt hơn mutex"*. Ngoài hiện trường **thỉnh thoảng** `H` trễ hạn tới **hàng trăm ms**; log sạch; tải cao thì hay gặp.

**(a)** Dựng kịch bản theo trình tự thời gian, **gọi tên hiện tượng**. **(b)** `M` không đụng semaphore — sao lại là thủ phạm? **(c)** Sửa thế nào, và **vì sao** cơ chế đó làm được trong khi cái đang dùng thì không?

**🔁 Follow-up đã hỏi:** (d) viết `std::mutex` trong C++ — **đã đủ chưa**? · (e) còn cơ chế nào khác chống được lớp lỗi này, khác PI ở chỗ nào?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ **priority inversion**, đủ ba mốc: L giữ semaphore → H chờ → M preempt L → L cầm khoá lâu bất thường → H trễ hạn.
- (b) ✅ *"ưu tiên M cao hơn L nên nó preempt được L"*.
- (c) ✅ mutex thay semaphore; lý do **priority inheritance**; giải thích đúng **ownership** — semaphore không biết ai đang giữ nên không nâng được ai.
- (d) ✅ *"Chưa. `std::mutex` mặc định không đảm bảo — phải dùng pthread mutex với `PTHREAD_PRIO_INHERIT`."*
- (e) ✅ **priority ceiling** + nêu đúng điều kiện áp dụng (biết trước phổ ưu tiên của hệ).

**✅ Vì sao 4 — trọn vẹn, không cần bổ sung:** không dừng ở *"đổi sang mutex"* mà đi tới **vì sao mutex làm được**: ownership ⇒ kernel biết ai đang giữ ⇒ nâng được ưu tiên; semaphore ai `post` cũng được ⇒ **không có ai để nâng**.

Hai probe cuối là chỗ tách người **đã làm** khỏi người **đã đọc**: rất nhiều người dừng ở *"dùng mutex là xong"*, trong khi mặc định pthread là `PTHREAD_PRIO_NONE` — tức **mặc định SAI cho hệ RT**.

**Bổ sung một ý cho tròn (không bắt buộc):** PI phản ứng **sau khi** có tranh chấp (nâng lúc bị chặn); ceiling nâng **ngay khi lấy khoá** kể cả không ai tranh ⇒ **ceiling tất định hơn nhưng "phí" hơn**. Bạn đã ngụ ý, chỉ chưa nói thành cặp đối lập.

**Chốt:** *"Mutex bảo vệ, semaphore báo hiệu. Chỉ mutex có ownership nên chỉ mutex có priority inheritance — và trong C++ thì `std::mutex` không mở được nó, phải xuống pthread."*

**Lần sau sẽ hỏi (dời +2 tuần → 04–09/09):** góc 3 — *"bật `PTHREAD_PRIO_INHERIT` rồi mà H vẫn thỉnh thoảng trễ. Còn nghi gì?"* (page fault trong critical section · L bị block trên I/O flash · priority inversion **kép** qua hai khoá · kernel không phải PREEMPT_RT — nối sang [BSP-021](../bank/bsp.md)).

**Ôn:** [bank OS-007](../bank/os.md) · [sync-primitives.md](../../../03-operating-system/sync-primitives.md#L74) · [scheduling.md §6](../../../03-operating-system/scheduling.md)
</details>

---

### Câu 5 · CPP-032 · **3/4** 🎉 **GỠ KHỎI SỔ YẾU** (2 lần liên tiếp ≥3: 13/08 = 3 → 21/08 = 3)

```cpp
struct Region {
    int x, y, w, h;
    Region(int x_, int y_, int w_, int h_) : x(x_), y(y_), w(w_), h(h_) {}
};
void blit(Region r);
```
PR của đồng nghiệp — **compile sạch, `-Wall -Wextra` 0 warning**:
```cpp
blit({0, 0, 1920, 1080});          // ①
Region a = {1, 2, 3, 4};           // ②
Region b{5, 6, 7, 8};              // ③
Region full_screen() {
    return {0, 0, 1920, 1080};     // ④
}
```

**(a)** Cơ chế nào cho phép ① compile? **(b)** Có gì đáng lo — mô tả **lớp bug**. **(c)** Thêm `explicit` thì **dòng nào gãy, dòng nào sống**? **(d)** Cái giá là gì — bạn có thực sự chọn `explicit` không?

**🔁 Follow-up đã hỏi:** (e) bạn bỏ trống ④ — gãy hay sống? · (f) codebase có 200 chỗ `return {…}` — thêm `explicit` là bạn vừa làm gì với chúng, và tiêu chí *"kiểu nào explicit, kiểu nào không"* là gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ implicit conversion `{0,0,1920,1080}` → `Region(...)`.
- (b) 🟡 *"trông như `initializer_list` nhưng không phải; người đọc dễ hiểu nhầm"* — lo hợp lệ nhưng **khác** lớp bug chính.
- (c) ✅ ① gãy · ② gãy · ③ **sống** (*"braced init tiêu chuẩn"* — đúng). ❌ **bỏ trống ④**.
- (d) 🟡 *"nên chọn"* — chưa nêu cái giá.
- (e) ✅ (sau probe) ④ **gãy**, suy đúng theo cơ chế của ①②.
- (f) 🟡 giữ quyết định + **nêu đánh đổi thành lời** (*"đổi sự tiện dụng lấy sự rõ ràng"*). ❌ chưa có **tiêu chí tổng quát**.

**Output thật — xác nhận ranh giới:**
```
=== KHÔNG explicit ===  → cả 4 dòng chạy
=== CÓ explicit ===
t.cpp:7:48: error: converting to ‘Region’ from initializer list would use explicit constructor   ← ④ return {…}
t.cpp:9:9:  error: converting to ‘Region’ from initializer list would use explicit constructor   ← ② Region a = {…}
t.cpp:10:27: error: converting to ‘Region’ from initializer list would use explicit constructor  ← ① blit({…})
```
**③ KHÔNG có trong danh sách lỗi** — đúng như bạn nói.

**❌ Vì sao chưa 4 — hai chỗ:**

**① Bỏ trống ④ ở lượt đầu.** `return {0,0,1920,1080};` là **copy-list-initialization**, cùng họ với ② ⇒ `explicit` giết nó. Và đây lại chính là **cái giá đắt nhất** của quyết định — bỏ sót nó làm (d) thành *"nên chọn"* mà chưa cân gì.

| Dòng | Dạng khởi tạo | `explicit` chặn? |
|---|---|---|
| ① `blit({…})` | copy-list-init (đối số hàm) | ❌ **gãy** |
| ② `Region a = {…}` | copy-list-init | ❌ **gãy** |
| ③ `Region b{…}` | **direct**-list-init | ✅ **sống** |
| ④ `return {…}` | copy-list-init | ❌ **gãy** |

⇒ Luật gọn: **có dấu `=` hoặc đi qua chỗ cần *chuyển đổi* thì `explicit` chặn; viết thẳng `T x{...}` thì không.**

**② Thiếu tiêu chí tổng quát:**
> **Để ngầm** khi phép chuyển đổi **lossless** và **không làm việc gì**.
> **Bắt `explicit`** khi nó **cấp phát / mở tài nguyên / đổi ngữ nghĩa**, hoặc khi **nhầm đơn vị / nhầm thứ tự đối số** là có thể. `Region(x,y,w,h)` rơi vào vế sau — bốn `int` không phân biệt được, hoán vị nhầm vẫn compile sạch.

**Về ý (b):** lớp bug chính **không phải** nhầm với `initializer_list`, mà là **chuyển đổi diễn ra im lặng tại call site** ⇒ nhầm thứ tự/đơn vị không ai thấy, `-Wall -Wextra` cũng câm.

**Chốt:** *"C++11 làm `explicit` có nghĩa cả với ctor nhiều đối số, vì braced-init mở đường chuyển đổi ngầm. Cái giá là mất `return {…}` và `T x = {…}` — đáng trả khi các đối số cùng kiểu và hoán vị được."*

**🔁 Đã xếp lịch kiểm tra lại (gỡ 21/08 → hạn 04–09/09).** Góc mới đề xuất: `explicit` cho **conversion operator** ở ca `explicit operator bool` — vì sao `if (obj)` vẫn chạy (contextual conversion) trong khi `cout << obj` thì không.

**Ôn:** [bank CPP-032](../bank/cpp.md) · [bank CPP-055](../bank/cpp.md)
</details>

---

### Câu 6 · CPP-016 · 🟠 · **3/4** 🔼 từ **2** (1/2 lần ≥3)

```cpp
struct Device {
    std::string name = "generic";
    virtual void power_off() { /* chỉ tắt nguồn */ }
    virtual ~Device() = default;
};
struct Panel : Device {
    Panel() { name = "panel"; }
    void power_off() override { /* black pattern -> backlight -> inverter */ }
};

void shutdown(Device d) { d.power_off(); }   // ⚠️ nhận theo GIÁ TRỊ
```
Output thật (`-Wall -Wextra`, **0 warning**):
```
gọi qua tham trị:
  shutdown(panel): Device::power_off (chỉ tắt nguồn)
gọi qua tham chiếu:
  shutdown_ref(panel): Panel::power_off (black pattern -> backlight -> inverter)
```

**(a)** Gọi tên hiện tượng: cái gì mất, cái gì còn? **(b)** `name` **vẫn in ra `"panel"`** nhưng `power_off()` gọi bản `Device` — vì sao mất cái này giữ cái kia? **(c)** 🎯 Sửa **từ phía class `Device`** để **lần sau không ai viết được `void shutdown(Device d)` nữa** — lỗi phải nổ ngay khi họ khai báo hàm.

**🔁 Follow-up đã hỏi:** (d) có cách nào **giữ copy `Panel`→`Panel`** mà vẫn cấm cắt qua `Device` không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ **object slicing** — phần dư của `Panel` bị cắt, còn lại phần `Device`.
- (b) ✅ **rất chính xác**: `name` sống vì `Panel()` đã ghi vào field của base; `power_off()` gọi bản `Device` vì **vptr không được copy sang** — object mới mang vtable của `Device`.
- (c) ✅ **hai** cơ chế kèm đánh đổi: ① `= delete` copy ở `Device` (giá: `Panel` **cũng** mất copy) · ② `Device` **abstract** bằng pure virtual (giá: mất hành vi mặc định).
- (d) 🟡 *"cấm copy ở `Device`, tự viết copy ở `Panel`"* — chạy được nhưng có bẫy. ❌ không nêu **`protected` copy ctor**.

**✅ Được — đây là bước tiến lớn nhất của phiên.** Lần trước ([13/08](2026-08-13--rapid--linux-sysprog.md)) tầng **thiết kế phòng ngừa TRẮNG**, chỉ sửa được call site. Hôm nay nêu **hai** cơ chế từ phía class, mỗi cái kèm đánh đổi.

Cả hai được xác nhận bằng compiler:
```
########## abstract ##########
error: cannot declare parameter ‘d’ to be of abstract type ‘Device’   ← nổ ngay tại KHAI BÁO
########## delete ##########
error: use of deleted function ‘Device::Device(const Device&)’
error: use of deleted function ‘Panel::Panel(const Panel&)’           ← Panel chết theo (đúng như bạn nêu)
```

**❌ Vì sao chưa 4 — ý (d) có bẫy.** *"Cấm copy ở `Device`, tự viết copy ở `Panel`"* **compile được** (đã thử, exit 0) — nhưng copy ctor của `Panel` **không gọi được** copy ctor của `Device` (đã `delete`), nên buộc phải dựng phần base bằng **default ctor** ⇒ **dữ liệu của `Device` im lặng không được copy**. Bạn vừa đổi *slicing lộ liễu* lấy **mất dữ liệu âm thầm** — khó phát hiện hơn.

**Cách chuẩn là `protected`, không phải `delete`:**
```cpp
struct Device {
    // ... virtual, dtor ...
protected:
    Device(const Device&)            = default;   // ✅ lớp con dùng được, người ngoài thì không
    Device& operator=(const Device&) = default;
};
```
Chạy thật — đúng **cả hai vế cùng lúc**:
```
=== chỉ copy Panel -> Panel ===
q.name=panel -> Panel::power_off                             ← ✅ copy đầy đủ, đa hình còn nguyên

=== khi gọi shutdown(p) ===
error: ‘Device::Device(const Device&)’ is protected within this context
note:   initializing argument 1 of ‘void shutdown(Device)’    ← ✅ chặn đúng chỗ cắt
```
`protected` chặn đúng ca cần chặn vì **`Panel` là lớp con nên truy cập được**, còn `shutdown` là code ngoài nên không.

⚠️ **Khác biệt tinh tế — liên quan trực tiếp tới yêu cầu ở (c)** (*"lỗi phải nổ ngay khi khai báo"*):

| Cách | `void shutdown(Device d)` — **khai báo** | Khi **gọi** | Giữ được copy `Panel`→`Panel`? |
|---|---|---|---|
| `protected` copy ctor | ✅ **vẫn compile** | ❌ lỗi | ✅ **có** |
| `= delete` copy ctor | ✅ vẫn compile | ❌ lỗi | ❌ mất |
| Base **abstract** | ❌ **lỗi ngay tại khai báo** | — | ✅ có |

⇒ Nếu tiêu chí là *"không ai viết nổi hàm đó"* thì **abstract base là cách DUY NHẤT đạt được**. Bạn nêu cả hai cách nhưng chưa phân biệt chúng bắt lỗi ở **hai thời điểm khác nhau**.

> **Bank [CPP-016](../bank/cpp.md) — bẫy (3):** *"muốn chặn hẳn: cho base class **`protected` copy ctor/assign**, hoặc `= delete` chúng — biến slicing thành **lỗi compile**."*

**Chốt:** *"Đa hình chỉ sống qua con trỏ/reference. Chặn từ phía class: `protected` copy ctor giữ được copy cho lớp con và chặn ở lúc gọi; abstract base chặn sớm hơn — ngay tại khai báo hàm."*

**Lần sau sẽ hỏi (còn trong sổ yếu, 1/2 lần ≥3):** góc 3 — *"`std::vector<Device> v; v.push_back(panel);` — compile sạch, chạy không crash, nhưng sai. Chuyện gì xảy ra, và `protected` copy ctor có cứu được ca này không?"* (ca container, ca nguy hiểm nhất trong bank).

**Ôn:** [bank CPP-016](../bank/cpp.md) · [01-cpp-fundamentals/oop.md](../../../01-cpp-fundamentals/oop.md)
</details>

---

## Tổng kết

### Điểm mạnh — ba thứ giữ nguyên

1. **Tầng RT / đồng bộ rất chắc** (Q4 = 4, Q3 = 3). Priority inversion dựng lại được **không cần gợi ý**, kể cả `PTHREAD_PRIO_INHERIT` và priority ceiling. Đúng vùng JD Datalogic hỏi thật.
2. **Tự nêu cái giá của chính giải pháp mình vừa viết** (Q3 d/e, Q1 f). Rất ít người mid-level tự soi ngược bản sửa của mình.
3. **Tầng thiết kế phòng ngừa đã mở** (Q6) — 13/08 trắng hoàn toàn ở đúng chỗ đó.

### Lỗ hổng ưu tiên

| # | Lỗ hổng | Loại | Việc phải làm |
|---|---|---|---|
| **1** 🔴 | **Chẩn đoán sai TẦNG vấn đề** (Q2): nghe *"hai `.so`"* → nhảy sang khung *liên tiến trình*, đề xuất shared memory cho hai lib **cùng một address space** | **Kiến thức + phản xạ khung** | Đọc [DP-020](../bank/design-patterns.md) (mới) + [linking-loading §symbol interposition](../../../07-shared-libraries/linking-loading.md#L100). Phản xạ cần tập: *"chúng có chung không gian địa chỉ không?"* trước khi chọn cơ chế |
| **2** 🟠 | **Nói cơ chế đúng dưới dạng phỏng đoán rồi bỏ** (Q2 d) — *"chưa rõ, theo tôi nó chọn một cái"* **chính là** symbol interposition, và nó **mâu thuẫn với (a)** mà không nhận ra | **Diễn đạt / tự kiểm** | Mẫu lặp **lần 3** (LNX-026 15/08 · BSP-029 19/08 · DP-002 nay). Khi câu sau đá câu trước, **dừng lại và chọn một** — interviewer luôn thấy mâu thuẫn đó |
| **3** 🟡 | **Biết công cụ, chưa biết công cụ làm gì bên trong** (Q3 g) — dùng đúng `scoped_lock` nhưng sai cơ chế và sai điều kiện Coffman | **Kiến thức** | [OS-003](../bank/os.md) bảng 4 điều kiện: **`scoped_lock` → hold-and-wait**, **lock ordering → circular wait** |

### So với các phiên gần nhất

| Phiên | Loại | Điểm | Ghi chú |
|---|---|---|---|
| 16/08 | `rapid` debugging | 3.67 | |
| 17/08 | `rapid` drivers-dt | **3.92** | 11/12 đạt 4 |
| 18/08 | `rapid` resume | 3.67 | lỗi **đóng gói** |
| 19/08 | `rapid` bsp | 3.33 | **thuộc bài ≠ hiểu bài** |
| **21/08** | **`daily` cpp-system** | **3.0** | **rơi vì repo hụt** — 1 câu, và câu đó bank không dạy |

> ⚠️ **Không so ngang được với 4 phiên trên.** Bốn phiên kia là `rapid` (đo **T1**, thang riêng); phiên này là `daily` chấm ở **T2** với 4/6 câu là retention/weak — tức **không có câu dễ nào**. Theo [config §6 luật ①](../config.md), câu retention/weak **không tính điểm phần nền**, nên cùng một mức hiểu biết sẽ ra điểm thấp hơn `rapid`.

---

## Cập nhật đã thực hiện (Bước 4)

- ✅ Log này (`sessions/2026-08-21--daily--cpp-system.md`)
- ✅ `weak-register.md`: **gỡ `CPP-032`** (+ xếp lịch retention 04–09/09) · **thêm `DP-002`** (regression) · `CPP-016` 2→3 · cập nhật cột KQ + "góc đã dùng" cho 4 câu retention
- ✅ `bank/design-patterns.md`: **thêm [DP-020](../bank/design-patterns.md)** (🟠 ⭐, khung 5 phần, output chạy thật) + cross-link từ `DP-002`
- ✅ `11-design-patterns/creational.md`: thêm cảnh báo `.so` vào §1 + dòng `DP-020` vào bảng câu hỏi
- ✅ `coding-arena/reviewed/2026-08-21--OS-003--balance-fix.cpp`
- ✅ `study-plans/datalogic-plan.md` §📍
