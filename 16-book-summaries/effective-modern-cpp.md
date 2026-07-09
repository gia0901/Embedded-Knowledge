# Effective Modern C++ — Scott Meyers (1st edition, 2014)

> **Nguồn summary:** kiến thức Claude, chưa đối chiếu PDF
> **Vì sao đọc cuốn này:** 42 items giải thích *tại sao* nên viết C++11/14 theo cách nào — đúng tầng "insight lý do thiết kế" mà topic 02 (Modern C++) không đủ chỗ trình bày. Rất nhiều câu hỏi interview C++ hiện đại lấy thẳng từ các item của sách này.

---

## TL;DR — cuốn sách này dạy gì

C++11/14 không chỉ là "C++ cũ thêm tính năng" mà là một ngôn ngữ với **idiom mới**: type deduction ở khắp nơi (template, `auto`, `decltype`), move semantics thay đổi cách nghĩ về hiệu năng, smart pointer thay thế `new/delete` thủ công, lambda thay thế functor, và concurrency API chuẩn hóa. Sách đi qua 42 "item" — mỗi item một lời khuyên cụ thể kèm lý do sâu (deduction rules, exception safety, ABI, hiệu năng) và các bẫy (proxy types, universal reference vs rvalue reference, capture dangling...).

## 🕐 Đọc gì nếu chỉ có N giờ

- **~2 giờ:** Cụm 4 (Smart Pointers) + Cụm 5 (Move & Forwarding) — hai cụm giá trị nhất.
- **~4 giờ:** thêm Cụm 3 (Moving to Modern C++ — các item 7, 9–12, 14, 15, 17).
- **~6 giờ:** thêm Cụm 7 (Concurrency) + Cụm 6 (Lambdas).

## 🎯 Lộ trình ôn nhanh trước interview

1. **Cụm 4 — Smart Pointers** (hay hỏi nhất: unique vs shared, control block, weak_ptr, make_shared)
2. **Cụm 5 — Move & Perfect Forwarding** (std::move làm gì, universal reference, khi nào move không xảy ra)
3. **Cụm 3 — Moving to Modern C++** (rule of five/zero, override, noexcept, constexpr, scoped enum)
4. **Cụm 7 — Concurrency API** (async vs thread, future, atomic vs volatile)
5. **Cụm 6 — Lambdas** (capture by ref dangling, init capture)
6. **Cụm 1+2 — Type Deduction & auto** (nền cho mọi thứ trên, ít bị hỏi trực diện)
7. **Cụm 8 — Tweaks** (pass-by-value, emplace — điểm cộng khi bàn hiệu năng)

## 🗺️ Bản đồ: chương ↔ cụm chủ đề ↔ topic liên quan

| Cụm | Chương sách | Items | 🎯 | Topic liên quan |
|-----|-------------|-------|----|-----------------|
| 1. Type Deduction | Ch.1 Deducing Types | 1–4 | | [01/templates](../01-cpp-fundamentals/templates.md) |
| 2. auto | Ch.2 auto | 5–6 | | — |
| 3. Moving to Modern C++ | Ch.3 | 7–17 | 🎯 | [02/raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md), [01/oop](../01-cpp-fundamentals/oop.md) |
| 4. Smart Pointers | Ch.4 | 18–22 | 🎯 | [02/raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md), [07/api-design](../07-shared-libraries/api-design.md) (Pimpl) |
| 5. Move & Perfect Forwarding | Ch.5 Rvalue References... | 23–30 | 🎯 | [02/move-semantics](../02-modern-cpp/move-semantics.md) |
| 6. Lambdas | Ch.6 Lambda Expressions | 31–34 | | [02/lambdas-functional](../02-modern-cpp/lambdas-functional.md) |
| 7. Concurrency API | Ch.7 The Concurrency API | 35–40 | 🎯 | [02/concurrency](../02-modern-cpp/concurrency.md) |
| 8. Tweaks | Ch.8 | 41–42 | | — |

---

## Cụm 1 — Type Deduction (ch. 1, Items 1–4)

### Nội dung chính

**Item 1 — Template type deduction: 3 trường hợp.** Mọi thứ xoay quanh dạng tổng quát:

```cpp
template<typename T>
void f(ParamType param);   // ParamType có thể là T, T&, const T&, T&&, T*...

f(expr);                   // compiler suy ra T VÀ ParamType từ expr
```

**Case 1 — `ParamType` là reference/pointer (không phải universal reference):** bỏ phần reference của `expr`, rồi pattern-match phần còn lại:

```cpp
template<typename T> void f(T& param);

int x = 27;  const int cx = x;  const int& rx = x;
f(x);   // T = int         → param: int&
f(cx);  // T = const int   → param: const int&   (const được GIỮ — an toàn)
f(rx);  // T = const int   → param: const int&   (reference-ness của rx bị bỏ)
```

Nếu `ParamType` là `const T&` thì `T = int` cho cả ba (const đã nằm sẵn trong ParamType).

**Case 2 — `ParamType` là universal reference (`T&&`):** hành xử khác biệt duy nhất trong C++:
- `expr` là **lvalue** → `T` và ParamType đều là **lvalue reference** (`int&`) — nhờ reference collapsing (chi tiết ở Cụm 5).
- `expr` là **rvalue** → như Case 1 (`T = int`, param `int&&`).

**Case 3 — pass-by-value (`T param`):** param là **bản copy độc lập** → bỏ reference, bỏ **top-level** `const`/`volatile`:

```cpp
template<typename T> void f(T param);
f(cx);  // T = int — bản copy không bị ràng buộc const của bản gốc

const char* const ptr = "hello";
f(ptr); // T = const char*  — const CỦA POINTER bị bỏ (top-level),
        //                    const của POINTEE được giữ (không phải top-level)
```

**Array & function decay.** Truyền array vào param by-value → decay thành pointer (`const char*`); nhưng param **by-reference** giữ nguyên kiểu array → suy ra được cả kích thước:

```cpp
template<typename T, std::size_t N>                 // suy ra N từ kiểu array
constexpr std::size_t arraySize(T (&)[N]) noexcept { return N; }

int vals[] = {1, 3, 7, 9, 11};
std::array<int, arraySize(vals)> mapped;            // dùng được ở compile time
```

Function types decay thành function pointer tương tự.

**Item 2 — `auto` type deduction = template deduction, trừ MỘT ngoại lệ.** Khi khai báo `auto x = expr;`, `auto` đóng vai `T`, toàn bộ type specifier (`const auto&`...) đóng vai `ParamType` — 3 case y hệt Item 1. Ngoại lệ duy nhất: **braced initializer**:

```cpp
auto x1 = 27;      // int
auto x2(27);       // int
auto x3 = {27};    // std::initializer_list<int>  ← ngoại lệ!
auto x4{27};       // sách (2014): initializer_list<int>
                   // ⚠️ sau proposal N3922 (mọi compiler hiện đại): int
```

⚠️ Sách viết trước N3922; quy tắc hiện hành: `auto x{v}` (direct-init, 1 phần tử) → kiểu của `v`; `auto x = {v}` (copy-init) → `initializer_list`. Template deduction **thất bại** với braced init (không suy ra được `T`), còn `auto` thì suy ra `initializer_list` — đây là khác biệt duy nhất.

Từ C++14: `auto` làm return type của hàm và `auto` trong lambda parameter dùng **template deduction** (không phải auto deduction) → `return {1, 2, 3};` trong hàm trả về `auto` là **compile error**.

**Item 3 — `decltype` trả về đúng declared type, không bỏ gì cả.** `decltype(name)` → đúng kiểu khai báo của `name` (giữ nguyên `const`, `&`). Quy tắc đặc biệt: với **expression phức tạp hơn một cái tên** mà là lvalue kiểu `T` → `decltype` trả về `T&`:

```cpp
int x = 0;
decltype(x)    // int      — x là name
decltype((x))  // int&     — (x) là lvalue expression, không phải name!
```

Ứng dụng chính: viết hàm trả về đúng kiểu mà `operator[]` của container trả về (thường là `T&`, nhưng `vector<bool>` thì không — Item 6):

```cpp
template<typename Container, typename Index>
decltype(auto) authAndAccess(Container&& c, Index i) {  // C++14
    authenticateUser();
    return std::forward<Container>(c)[i];   // giữ nguyên reference-ness của c[i]
}
```

Nếu dùng `auto` thường làm return type → template deduction **bỏ mất reference** → trả về bản copy → `authAndAccess(d, 5) = 10;` không compile. `decltype(auto)` = "deduce theo quy tắc decltype".

⚠️ Bẫy kinh điển: trong hàm trả về `decltype(auto)`, viết `return x;` trả về `int`, nhưng `return (x);` trả về `int&` — **reference tới biến local** → UB.

### Insight đáng nhớ

- Type deduction là "ngữ pháp nền" của Modern C++: `auto`, lambda, `decltype(auto)`, universal reference, structured bindings (C++17) đều xây trên 3 case của Item 1. Hiểu Case 3 (bỏ top-level const) và Case 2 (lvalue → `T&`) là giải thích được phần lớn hành vi "khó hiểu" của template.
- Array decay ở Case 3 chính là lý do `sizeof(arr)` trong hàm nhận array by-value cho kết quả sai — bug C kinh điển, giờ giải thích được bằng ngôn ngữ deduction.

### Ít quan trọng

- **Item 4 — Cách xem deduced type:** IDE hover, cố ý gây compile error với template chưa định nghĩa (`template<typename T> class TD;` rồi `TD<decltype(x)> xType;` — đọc error message), hoặc Boost.TypeIndex; `typeid`/`std::type_info::name` **không đáng tin** vì nó decay kiểu như pass-by-value (Item 4).

### Góc interview

**Câu 1:** `auto x = {1, 2, 3};` — `x` có kiểu gì? Truyền `{1, 2, 3}` vào `template<typename T> void f(T param)` thì sao?

<details><summary>Đáp án</summary>

- `auto x = {1, 2, 3};` → `x` là `std::initializer_list<int>` — ngoại lệ duy nhất của auto deduction so với template deduction.
- `f({1, 2, 3})` → **compile error**: template type deduction không suy ra được `T` từ braced initializer (braced init không có kiểu). Muốn nhận được phải khai báo tường minh `void f(std::initializer_list<int> il)`.
- Bẫy mở rộng: hàm C++14 trả về `auto` dùng *template* deduction → `return {1, 2, 3};` cũng lỗi.

</details>

**Câu 2:** Cùng gọi `f(cx)` với `const int cx`, vì sao `template<void f(T&)>` giữ `const` mà `template<void f(T)>` lại bỏ `const`?

<details><summary>Đáp án</summary>

Vì ngữ nghĩa khác nhau:
- `T& param` — param **trỏ về chính đối tượng gốc** → phải giữ `const` để không cho sửa đối tượng gốc qua reference: `T = const int`, param `const int&`.
- `T param` — param là **bản copy độc lập**. Việc bản gốc không sửa được không liên quan gì đến việc bản copy có sửa được hay không → top-level `const` bị bỏ: `T = int`.
- Lưu ý phân biệt: với `const char* const ptr`, chỉ const **của pointer** (top-level) bị bỏ; const **của pointee** giữ nguyên vì nó là một phần của "giá trị được copy" (`T = const char*`).

</details>

### Đọc thêm (tùy chọn)

- [01/templates.md](../01-cpp-fundamentals/templates.md) — nền template căn bản.

## Cụm 2 — auto (ch. 2, Items 5–6)

### Nội dung chính

**Item 5 — Ưu tiên `auto` hơn khai báo kiểu tường minh.** Không chỉ là gõ ít hơn — `auto` **loại bỏ nhiều lớp bug**:

1. **Buộc phải khởi tạo:** `auto x;` không compile → hết bug biến chưa init.
2. **Tránh "type shortcut" sai lệch âm thầm:**

```cpp
std::vector<int> v;
unsigned sz = v.size();   // ❌ v.size() là std::vector<int>::size_type (64-bit trên Linux x64)
                          //    unsigned là 32-bit → truncate âm thầm với vector lớn
auto sz2 = v.size();      // ✅ đúng kiểu, mọi platform
```

3. **Tránh copy ẩn do sai kiểu trong range-for** — bug rất phổ biến:

```cpp
std::unordered_map<std::string, int> m;
for (const std::pair<std::string, int>& p : m) { ... }
// ❌ value_type thật là pair<CONST string, int> → kiểu không khớp
//    → compiler TẠO TEMPORARY mỗi vòng lặp, p bind vào temporary
//    → vừa tốn copy, vừa &p không trỏ vào phần tử thật của map

for (const auto& p : m) { ... }   // ✅ đúng kiểu, không copy
```

4. **Lưu closure hiệu quả:** `auto cl = [](...){...};` giữ đúng kiểu closure — còn `std::function` là type erasure: object to hơn, có thể cấp phát heap, gọi chậm hơn (gián tiếp). Chỉ dùng `std::function` khi thật sự cần "một biến chứa được nhiều loại callable".

Đánh đổi: code kém "đọc ngay ra kiểu" — sách lập luận rằng tên hàm/ngữ cảnh thường đã đủ thông tin, và các IDE hiển thị kiểu khi cần; sai lệch kiểu tường minh gây hại nhiều hơn lợi ích đọc hiểu.

**Item 6 — Bẫy "invisible proxy type": dùng explicitly typed initializer idiom.** Một số `operator[]`/expression trả về **proxy object** thay vì giá trị thật:

```cpp
std::vector<bool> features(const Widget& w);   // trả về by value

bool highPriority = features(w)[5];        // ✅ proxy → convert sang bool ngay
auto highPriority = features(w)[5];        // ❌ highPriority là vector<bool>::reference
                                           //    — proxy giữ pointer vào vector TEMPORARY
processWidget(w, highPriority);            //    temporary đã hủy → dangling → UB!
```

`std::vector<bool>` không lưu `bool` thật mà đóng gói bit → `operator[]` không thể trả `bool&`, phải trả proxy `std::vector<bool>::reference`. Với `bool` tường minh, proxy convert ngầm sang `bool` ngay khi temporary còn sống; với `auto`, proxy được giữ lại → dangling.

Proxy "vô hình" còn xuất hiện ở **expression templates** (thư viện đại số tuyến tính: `Matrix sum = m1 + m2 + m3;` — vế phải là kiểu `Sum<Sum<Matrix,Matrix>,Matrix>` chứ không phải `Matrix`).

Cách xử lý — **không bỏ `auto`**, mà ép kiểu tường minh mong muốn:

```cpp
auto highPriority = static_cast<bool>(features(w)[5]);   // ✅ rõ ý định, vẫn auto
auto sum = static_cast<Matrix>(m1 + m2 + m3);
```

Nhận diện proxy: đọc documentation/header của thư viện; proxy thường lộ ra ở kiểu trả về của `operator[]`, `operator+`...

### Insight đáng nhớ

- Triết lý của Meyers: `auto` không phải "lười gõ" mà là **chuyển việc giữ kiểu nhất quán từ người sang compiler** — người sai âm thầm (truncate, temporary), compiler thì không.
- Quy tắc thực dụng: mặc định `auto`; khi vế phải là API lạ có thể trả proxy → `static_cast` làm rõ. Đây cũng là style nhất quán với `make_unique`, structured bindings sau này.

### Góc interview

**Câu 1:** Đoạn code sau có bug gì? Sửa thế nào?

```cpp
std::vector<bool> flags = getFlags();
auto ok = flags[0];
flags.push_back(true);
if (ok) { ... }
```

<details><summary>Đáp án</summary>

- `ok` là `std::vector<bool>::reference` (proxy chứa pointer vào block bit của `flags`), **không phải** `bool`.
- `push_back` có thể gây **reallocation** → proxy trỏ vào bộ nhớ đã giải phóng → `if (ok)` đọc dangling pointer → UB. (Cùng bản chất với iterator invalidation.)
- Sửa: `auto ok = static_cast<bool>(flags[0]);` hoặc `bool ok = flags[0];` — chốt giá trị ngay khi đọc.
- Ý mở rộng ăn điểm: `vector<bool>` là specialization đóng gói bit, `operator[]` không trả được `bool&`; vì thế nhiều codebase cấm `vector<bool>`, dùng `std::vector<char>`/`std::bitset` thay thế.

</details>

### Đọc thêm (tùy chọn)

- [02/lambdas-functional.md](../02-modern-cpp/lambdas-functional.md) — closure vs `std::function`.

## Cụm 3 — Moving to Modern C++ (ch. 3, Items 7–17) 🎯

### Nội dung chính

**Item 7 — `{}` vs `()` khi khởi tạo.** Braced initialization (`{}`) là "uniform initialization": dùng được ở mọi ngữ cảnh (default member init, container phần tử, non-copyable object), và có 2 ưu điểm an toàn:

```cpp
double d = 3.7;
int x{d};      // ❌ compile error — {} CẤM narrowing conversion
int y(d);      // ✅ compile, truncate âm thầm = 3

Widget w1();   // ⚠️ most vexing parse — khai báo HÀM w1 trả về Widget!
Widget w2{};   // ✅ gọi default constructor
```

Nhược điểm lớn: nếu class có constructor nhận `std::initializer_list`, `{}` **bị hút về** overload đó một cách rất "tham lam":

```cpp
std::vector<int> v1(10, 20);   // 10 phần tử, giá trị 20
std::vector<int> v2{10, 20};   // 2 phần tử: 10 và 20  — khác hẳn!
```

Hệ quả thiết kế: thêm `initializer_list` ctor vào class đã có ctor khác là **thay đổi ý nghĩa code client** đang dùng `{}`. Trong template generic (như `make_unique` dùng `(...)` bên trong), người viết phải *chọn hộ* người dùng một trong hai — không có đáp án chung, chỉ cần chọn nhất quán và document.

**Item 8 — Dùng `nullptr`, không dùng `0`/`NULL`.** `0` là `int`, `NULL` là macro kiểu integral — cả hai không phải kiểu pointer:

```cpp
void f(int);
void f(void*);
f(0);        // gọi f(int) — dù ý định là null pointer!
f(NULL);     // gọi f(int) hoặc ambiguous tùy platform
f(nullptr);  // ✅ gọi f(void*) — nullptr có kiểu std::nullptr_t, convert sang MỌI pointer
```

Trong template, truyền `0`/`NULL` deduction ra kiểu `int` → không match parameter kiểu pointer → lỗi khó hiểu. `nullptr` còn làm code tự-document: thấy là biết đang nói về pointer.

**Item 9 — `using` (alias declaration) thay cho `typedef`.** Lý do quyết định: **alias template** — `typedef` không template hoá được:

```cpp
template<typename T>
using MyAllocList = std::list<T, MyAlloc<T>>;     // ✅ gọn

MyAllocList<Widget> lw;

// typedef phải bọc trong struct:
template<typename T>
struct MyAllocListT { typedef std::list<T, MyAlloc<T>> type; };
MyAllocListT<Widget>::type lw2;                    // dài dòng
// và trong template khác phải viết: typename MyAllocListT<T>::type  (dependent type)
// còn alias template: MyAllocList<T> — không cần typename, không cần ::type
```

Đây chính là lý do C++14 thêm `std::remove_const_t<T>` (alias) bên cạnh `std::remove_const<T>::type` (C++11).

**Item 10 — Scoped enum (`enum class`) thay unscoped `enum`.** Ba lợi ích:

| | `enum Color {...}` | `enum class Color {...}` |
|---|---|---|
| Namespace | Enumerator **rò ra scope ngoài** | Phải viết `Color::black` |
| Chuyển kiểu | Ngầm sang integral/double → so sánh vô nghĩa vẫn compile | Không convert ngầm; muốn thì `static_cast` |
| Forward declare | C++11 phải chỉ định underlying type | Mặc định được (underlying mặc định `int`) |

```cpp
enum class Status: std::uint8_t;   // forward declare + chọn size — hữu ích cho embedded/ABI
```

Trường hợp unscoped enum còn hữu ích: đặt tên index cho `std::get<>` của tuple (tận dụng implicit conversion sang `size_t`).

**Item 11 — `= delete` thay cho "private + không định nghĩa".** Cách C++98 (khai báo private, không implement) chỉ chặn ở **link time** và chỉ với code ngoài class. `= delete`:
- Lỗi ngay **compile time**, kể cả khi member/friend gọi.
- Dùng được cho **hàm tự do** và **template specialization** — chặn overload/instantiation không mong muốn:

```cpp
bool isLucky(int number);
bool isLucky(char) = delete;      // chặn convert ngầm char → int
bool isLucky(double) = delete;    // chặn double VÀ float (float thăng hạng lên double)

template<typename T> void processPointer(T* ptr);
template<> void processPointer<void>(void*) = delete;   // cấm void*
```

Quy ước: deleted function nên để `public` — compiler check accessibility trước deleted status, để private sẽ ra error message gây hiểu lầm.

**Item 12 — Viết `override` cho mọi hàm virtual override.** Override đòi hỏi khớp **chính xác**: tên, tham số, const-ness, reference qualifier, exception spec tương thích; return type covariant. Sai một điểm → hàm mới **che (hide)** chứ không override — compile vẫn qua, bug chỉ lộ lúc runtime:

```cpp
class Base {
public:
    virtual void doWork() const;
};
class Derived : public Base {
public:
    void doWork();              // ⚠️ thiếu const → KHÔNG override, compile vẫn OK
    void doWork() override;     // ❌ compile error → lộ bug ngay — đây là mục đích!
};
```

`override` biến lỗi thiết kế thành compile error và làm rõ ý định khi đọc code. Đi kèm: `final` (cấm override tiếp / cấm kế thừa class).

Phần phụ của item: **member function reference qualifiers** — overload theo việc `*this` là lvalue hay rvalue:

```cpp
class Widget {
public:
    std::vector<int>& data() &  { return values; }             // gọi trên lvalue
    std::vector<int>  data() && { return std::move(values); }  // gọi trên rvalue → move ra
};
auto vals = makeWidget().data();   // dùng bản && → move, không copy
```

**Item 13 — Ưu tiên `const_iterator`:** C++11 làm nó thực dụng: `cbegin()/cend()`, các hàm `insert/erase` nhận `const_iterator`. Trong code generic dùng non-member `std::begin/std::end` (C++14 thêm `std::cbegin/std::cend`). Quy tắc cũ "dùng const bất cứ khi nào có thể" áp dụng cho cả iterator.

**Item 14 — `noexcept` là một phần của interface, và là đòn bẩy tối ưu.**
- Với hàm `noexcept`, compiler **không cần giữ stack ở trạng thái unwindable** → code gọn hơn. Nếu exception thoát ra khỏi hàm `noexcept` → `std::terminate` (không unwind).
- Quan trọng nhất với move: `std::vector::push_back` khi grow muốn move phần tử sang buffer mới, nhưng phải giữ **strong exception guarantee** → chỉ move nếu move ctor `noexcept`, ngược lại **copy** (cơ chế `std::move_if_noexcept`). → **Move ctor/assignment không `noexcept` = mất gần hết lợi ích hiệu năng khi ở trong container.**
- `swap` cũng conditionally noexcept dựa trên noexcept của phần tử.
- Mặc định ngầm: **destructor và deallocation ngầm là `noexcept`**.
- Chỉ khai `noexcept` khi cam kết **lâu dài** — gỡ `noexcept` sau này là phá interface. Phù hợp tự nhiên với hàm "wide contract" (không tiền điều kiện).

**Item 15 — `constexpr` = "có thể tính lúc compile time".**
- `constexpr` **object**: hằng compile-time thực thụ, dùng được ở nơi cần constant expression (kích thước array, non-type template argument, enumerator...). Mạnh hơn `const`: `const` chỉ hứa "không đổi", không hứa "biết lúc compile".
- `constexpr` **function**: gọi với toàn argument compile-time → kết quả compile-time; gọi với runtime value → chạy như hàm thường. **Một hàm phục vụ cả hai thế giới** → mở rộng vùng code có thể "dời" sang compile time (embedded: dời tính toán từ runtime sang lúc build, tiết kiệm CPU/RAM).
- C++11 giới hạn body = 1 câu `return`; **C++14 nới**: cho phép loop, biến local, nhiều statement; member function `constexpr` C++14 không còn ngầm `const`.
- Literal type (kể cả class có `constexpr` constructor) dùng được với `constexpr`.
- Cũng là cam kết interface: bỏ `constexpr` đi là phá client đang dùng ở compile-time context.

**Item 16 — Hàm member `const` phải an toàn cho concurrent read.** Quy ước hậu C++11: `const` = "đọc" → nhiều thread gọi hàm `const` đồng thời **không cần lock** là kỳ vọng hợp lý. Nhưng `mutable` cache phá vỡ điều đó:

```cpp
class Polynomial {
public:
    RootsType roots() const {              // "đọc" — nhưng ghi cache!
        if (!rootsAreValid) {              // ⚠️ data race nếu 2 thread cùng vào
            /* tính toán */ 
            rootsAreValid = true;
        }
        return rootVals;
    }
private:
    mutable bool rootsAreValid{false};     // mutable = ghi được trong hàm const
    mutable RootsType rootVals{};
};
```

Fix: thêm `mutable std::mutex m;` và lock trong `roots()`, hoặc `std::atomic` **nếu chỉ một biến đếm/cờ đơn lẻ**. ⚠️ Hai `std::atomic` liên quan nhau (cached value + valid flag) vẫn race về logic — từ hai biến liên quan trở lên phải dùng mutex. Lưu ý: `std::mutex`/`std::atomic` là move-only... thực ra là **không copy không move** → class chứa chúng mất copyability.

**Item 17 — Quy tắc sinh special member functions (nền của Rule of Five/Zero).** C++11 có 6 hàm đặc biệt: default ctor, dtor, copy ctor, copy assign, move ctor, move assign. Quy tắc sinh tự động:

- **Move operations** chỉ được sinh khi class **không khai báo**: copy operation nào, move operation nào, và destructor. (Khai báo 1 trong 5 thứ đó → không sinh move.)
- **Khai báo move** → copy ctor & copy assign bị `delete` ngầm.
- Copy operations: vẫn được sinh khi có destructor/copy khác (di sản C++98) nhưng hành vi này **deprecated** — đừng dựa vào.
- Move được sinh = **memberwise move**: member nào move được thì move, không thì copy.

Hệ quả thực dụng — **Rule of Five**: đã tự viết destructor (hay copy) nghĩa là class đang tự quản tài nguyên → phải cân nhắc viết/`= default` cả 5. **Rule of Zero**: thiết kế sao cho không phải viết cái nào (dùng RAII member như `unique_ptr`, `vector`). Bẫy hay gặp:

```cpp
class StringTable {
public:
    ~StringTable() { makeLogEntry("destroying"); }   // chỉ thêm log dtor...
    // ⚠️ ... là move ctor/assign KHÔNG được sinh nữa
    //    → mọi "move" âm thầm rơi về COPY map khổng lồ — chậm đi hàng trăm lần, không có warning
private:
    std::map<int, std::string> values;
};
// Fix: khai báo lại ý định
// StringTable(StringTable&&) = default;
// StringTable& operator=(StringTable&&) = default;
```

Member function template không bao giờ chặn việc sinh special members (dù signature trùng).

### Insight đáng nhớ

- Sợi chỉ đỏ của chương: C++11 biến nhiều **quy ước ngầm** thành **cam kết có compiler kiểm tra** — `override`, `= delete`, `noexcept`, `constexpr`, `enum class` đều là "nói rõ ý định để compiler bắt lỗi hộ". Đây là câu trả lời đẹp cho câu hỏi interview "Modern C++ hiện đại hơn ở chỗ nào?".
- Item 14 + 17 kết nối thành chuỗi nhân-quả đắt giá: *tự viết destructor → mất move sinh tự động → container copy thay vì move → chậm âm thầm*; và *move không `noexcept` → vector không dám move khi grow*. Hai điều này giải thích phần lớn "code C++11 mà không nhanh hơn".
- `const` sau C++11 mang thêm nghĩa **thread-safety contract** (Item 16) — thay đổi ngữ nghĩa ít người để ý.

### Ít quan trọng

- Chi tiết dùng unscoped enum làm index cho `std::get<>` của tuple và template `toUType` chuyển enum về underlying type (Item 10, phần cuối).
- Lịch sử `NULL` trên các platform và interaction với template deduction chi tiết (Item 8, phần giữa).
- `std::cbegin` tự viết cho C++11 (Item 13, phần cuối).

### Góc interview

**Câu 1 (Rule of Five/Zero):** Class sau có vấn đề gì về hiệu năng? Giải thích cơ chế và cách sửa.

```cpp
class Buffer {
public:
    ~Buffer() { log("Buffer destroyed"); }
private:
    std::vector<uint8_t> data_;   // thường chứa hàng MB
};
```

<details><summary>Đáp án</summary>

- Khai báo destructor (dù chỉ để log) → theo quy tắc Item 17, compiler **không sinh move ctor/move assign** nữa.
- Copy ctor/copy assign vẫn được sinh (hành vi legacy) → mọi chỗ tưởng là move (`return buf;` không elide được, `std::move(buf)`, đưa vào `vector<Buffer>` khi grow...) âm thầm rơi về **copy toàn bộ vector hàng MB**. Không có warning nào.
- Sửa: khai báo lại ý định rõ ràng:

```cpp
class Buffer {
public:
    ~Buffer() { log("Buffer destroyed"); }
    Buffer(Buffer&&) noexcept = default;              // khôi phục move
    Buffer& operator=(Buffer&&) noexcept = default;
    Buffer(const Buffer&) = default;                  // giữ copy nếu muốn
    Buffer& operator=(const Buffer&) = default;
};
```

- Ý ăn điểm: (1) nêu **Rule of Zero** — nếu không cần log thì đừng viết dtor, để cả 6 hàm tự sinh; (2) nêu `noexcept` trên move — thiếu nó `vector<Buffer>` khi grow vẫn copy (Item 14, `move_if_noexcept`).
- **Bẫy thường gặp khi trả lời:** nói "có destructor thì mất cả copy" — sai, copy vẫn được sinh (chỉ deprecated); và quên mất rằng bug này **không hề báo lỗi**, chỉ chậm.

</details>

**Câu 2 (`noexcept`):** `noexcept` ảnh hưởng gì đến hiệu năng của `std::vector<T>::push_back`?

<details><summary>Đáp án</summary>

- Khi vector hết capacity, nó cấp buffer mới và chuyển phần tử cũ sang. Để giữ **strong exception guarantee** (nếu ném exception giữa chừng, vector phải như chưa hề đụng đến), nó chỉ dám **move** khi chắc chắn move không ném — tức move ctor của `T` là `noexcept`. Nếu không, nó **copy** (bản gốc còn nguyên, ném lúc nào cũng rollback được).
- Cơ chế chuẩn: `std::move_if_noexcept` — trả về rvalue ref nếu move `noexcept` (hoặc T không copy được), ngược lại trả lvalue ref → chọn copy ctor.
- Sơ đồ quyết định:

```
push_back cần grow
        │
   move ctor của T noexcept?
      ┌──┴──┐
     Có     Không
      │       │
   MOVE     COPY (an toàn rollback,
  (nhanh)    nhưng chậm + tốn RAM gấp đôi lúc grow)
```

- Kết luận thực dụng: **luôn đánh `noexcept` cho move ctor/move assign** (thường qua `= default` với member đều noexcept-movable). Đây là lý do hàng đầu khiến "đã viết move rồi mà vẫn chậm".
- **Bẫy khi trả lời:** tưởng `noexcept` chỉ là documentation — nó thay đổi **lựa chọn thuật toán** của library lúc runtime (qua `noexcept` operator ở compile time) và cho phép compiler bỏ mã unwind.

</details>

**Câu 3 (`enum class`):** Vì sao embedded codebase thường ưu tiên `enum class X : uint8_t`? Nêu đủ các lý do.

<details><summary>Đáp án</summary>

1. **Kiểm soát size/ABI:** chỉ định underlying type `uint8_t` → biết chính xác 1 byte, ổn định khi serialize, đặt vào register map, struct packed, giao tiếp qua mạng/IPC — unscoped enum không chỉ định để compiler tự chọn, có thể khác nhau giữa toolchain.
2. **Forward declaration:** khai trước `enum class Status : uint8_t;` trong header → giảm dependency biên dịch; sửa danh sách enumerator không buộc rebuild toàn bộ client (quan trọng với build embedded lâu).
3. **Không convert ngầm sang int:** chặn các phép so sánh/tính toán vô nghĩa giữa các enum khác nhau (vd `if (color == status)`) — bug logic compile qua được với unscoped enum.
4. **Không rò tên ra scope ngoài:** `Status::Ok` và `Result::Ok` chung sống được; unscoped enum thì đụng tên.
- Muốn lấy giá trị số (ghi thanh ghi, log): `static_cast<uint8_t>(Status::Ok)` — tường minh, đúng chỗ.

</details>

**Câu 4 (`override`):** Không viết `override` thì chuyện tồi tệ nhất có thể xảy ra là gì? Cho ví dụ cụ thể.

<details><summary>Đáp án</summary>

- Tồi tệ nhất: hàm derived **không override mà che** hàm base — chương trình compile sạch, chạy "bình thường", nhưng gọi qua pointer/reference base sẽ chạy **bản base**, không phải bản derived. Bug logic thầm lặng, thường chỉ lộ ở tình huống hiếm.
- Ví dụ:

```cpp
class Sensor {
public:
    virtual void onData(const Packet& p);        // base
};
class TempSensor : public Sensor {
public:
    void onData(Packet& p);   // ⚠️ thiếu const trên tham số → hàm MỚI, không override
};
// driver gọi qua Sensor* → luôn chạy Sensor::onData — TempSensor::onData không bao giờ chạy
```

- Các điều kiện phải khớp **chính xác** để override: tên, danh sách tham số, const-ness của hàm, reference qualifier, return type covariant, virtual ở base. Sai bất kỳ điểm nào → hide.
- `override` đảo tình thế: mismatch → **compile error ngay**. Chi phí bằng 0, nên quy ước là bắt buộc trong mọi codebase nghiêm túc (kèm `final` khi muốn khóa).

</details>

### Đọc thêm (tùy chọn)

- [01/oop.md](../01-cpp-fundamentals/oop.md) — virtual, vtable (nền của Item 12).
- [02/raii-smart-pointers.md](../02-modern-cpp/raii-smart-pointers.md) — RAII, nền của Rule of Zero.
- [02/concurrency.md](../02-modern-cpp/concurrency.md) — mutex/atomic (nền của Item 16).

## Cụm 4 — Smart Pointers (ch. 4, Items 18–22) 🎯

### Nội dung chính

**Item 18 — `std::unique_ptr`: ownership độc quyền, chi phí bằng raw pointer.**
- **Move-only** (copy bị delete) — chuyển ownership là move, ngữ nghĩa rõ ràng: tại mọi thời điểm đúng một chủ.
- Kích thước **bằng raw pointer**, thao tác gần như không overhead → "mặc định đầu tiên" khi cần smart pointer, kể cả trên embedded.
- **Custom deleter là một phần của kiểu**: `std::unique_ptr<Widget, decltype(del)>`. Deleter là lambda **không capture** → không tăng size; deleter có state/function pointer → size tăng theo.

```cpp
auto delInvmt = [](Investment* p) {   // lambda không capture → zero overhead
    makeLogEntry(p);
    delete p;
};

template<typename... Ts>
std::unique_ptr<Investment, decltype(delInvmt)>   // factory: dạng dùng phổ biến nhất
makeInvestment(Ts&&... args);
```

- Hai dạng: `unique_ptr<T>` và `unique_ptr<T[]>` (dạng array hiếm dùng — `std::array`/`vector` tốt hơn).
- **Chuyển ngầm sang `shared_ptr`** được → factory trả `unique_ptr` là thiết kế đúng: client tự quyết ownership model.

**Item 19 — `std::shared_ptr`: shared ownership qua control block.** Cấu trúc:

```
   shared_ptr<T> (2 con trỏ — to gấp đôi raw ptr)
   ┌──────────────┐
   │ ptr → T ─────┼──────────→ ┌──────────┐
   │ ptr → CB ────┼──┐         │ T object │
   └──────────────┘  │         └──────────┘
                     ▼
          Control Block (trên heap)
          ┌───────────────────────────┐
          │ strong ref count (atomic) │ ← số shared_ptr đang trỏ
          │ weak ref count  (atomic)  │ ← số weak_ptr (giữ CB sống, không giữ T)
          │ custom deleter (nếu có)   │
          │ allocator, ...            │
          └───────────────────────────┘
```

- Ref count **atomic** → tăng/giảm là RMW atomic, có chi phí thật (đặc biệt khi copy shared_ptr trong vòng lặp/hot path — truyền `const shared_ptr&` hoặc raw pointer/reference cho hàm chỉ "dùng" không "sở hữu").
- Move shared_ptr **không** đụng ref count → rẻ hơn copy.
- Khác `unique_ptr`: **deleter không thuộc kiểu** → hai `shared_ptr<Widget>` với deleter khác nhau vẫn cùng kiểu, bỏ chung container được; size shared_ptr không đổi theo deleter (deleter nằm trong control block).
- **Quy tắc tạo control block** — nguồn bug số 1: control block được tạo khi (1) `make_shared`, (2) tạo từ `unique_ptr`, (3) tạo từ **raw pointer**. Suy ra:

```cpp
auto pw = new Widget;
std::shared_ptr<Widget> spw1(pw);   // control block #1
std::shared_ptr<Widget> spw2(pw);   // control block #2 — cùng 1 Widget!
// → double delete → UB. KHÔNG BAO GIỜ tạo 2 shared_ptr từ cùng raw pointer.
// Quy tắc: truyền thẳng kết quả new, hoặc dùng make_shared.
```

- Biến thể của bug trên: tạo `shared_ptr(this)` trong khi bên ngoài cũng đang có shared_ptr quản lý object → **`std::enable_shared_from_this<T>`** (CRTP): kế thừa nó và gọi `shared_from_this()` — dùng lại control block có sẵn. Tiền đề: phải **đã có** một shared_ptr bên ngoài (thường ép bằng factory + private constructor); nếu chưa có → UB (C++17: ném `std::bad_weak_ptr`).
- Không có `shared_ptr<T[]>` (đến C++17; C++20 mới thêm) — sách khuyên dùng container chuẩn.

**Item 20 — `std::weak_ptr`: shared_ptr không sở hữu, phát hiện dangling.** weak_ptr trỏ vào object do shared_ptr quản lý nhưng **không tăng strong count** → object hủy được dù weak_ptr còn đó; weak_ptr biết mình đã "expired":

```cpp
auto spw = std::make_shared<Widget>();
std::weak_ptr<Widget> wpw(spw);
spw = nullptr;                      // Widget hủy — wpw expired

if (auto sp = wpw.lock()) {         // lock(): atomic "check + tạo shared_ptr"
    // dùng sp an toàn
} else {
    // object đã chết
}
// KHÔNG dùng: if (!wpw.expired()) { wpw.lock()->... }  — race giữa 2 bước!
```

Ba use case chính: **cache** (object có thể bị hủy bởi nơi khác), **observer list** (subject không sở hữu observer nhưng cần biết observer còn sống), **phá vòng shared_ptr** (A↔B trỏ nhau bằng shared_ptr → strong count không bao giờ về 0 → leak; một chiều đổi thành weak_ptr). Lưu ý: quan hệ cha-con cây thông thường không cần weak_ptr — con trỏ ngược lên cha bằng raw pointer là đủ vì lifetime cha bao trùm con.

**Item 21 — Ưu tiên `make_shared`/`make_unique` hơn `new` trực tiếp.** (`make_unique` từ C++14.) Ba lý do:

1. **Không lặp type name** — `auto p = std::make_unique<Widget>();` vs `std::unique_ptr<Widget> p(new Widget);`
2. **Exception safety:**

```cpp
processWidget(std::shared_ptr<Widget>(new Widget), computePriority());
// C++11/14: compiler được phép xen kẽ: new Widget → computePriority() → shared_ptr ctor
// computePriority() ném → Widget vừa new bị LEAK
processWidget(std::make_shared<Widget>(), computePriority());   // ✅ kín kẽ
```
(C++17 siết luật đánh giá argument nên bẫy này giảm, nhưng make vẫn là thói quen đúng.)

3. **`make_shared` cấp phát MỘT lần** cho cả object + control block (new riêng → 2 lần cấp phát) → nhanh hơn, ít fragmentation — điểm cộng lớn cho embedded.

Khi **không** dùng make được: (1) cần **custom deleter**; (2) muốn truyền **braced initializer** (make perfect-forward bằng `()` — muốn `{}` phải tạo `initializer_list` trước); (3) với `make_shared`: class có `operator new/delete` riêng, hoặc hệ thống có **weak_ptr sống rất lâu + object rất lớn** — vì object và control block chung một khối, khối nhớ chỉ được giải phóng khi **cả weak count về 0** → object memory bị "giam" theo weak_ptr.

**Item 22 — Pimpl với `unique_ptr`: destructor phải nằm trong .cpp.** Pimpl (pointer to implementation) cắt dependency biên dịch — header chỉ còn forward declaration:

```cpp
// widget.h — client không thấy Gadget, vector...
class Widget {
public:
    Widget();
    ~Widget();                          // ⚠️ chỉ KHAI BÁO
    Widget(Widget&&) noexcept;          // move cũng chỉ khai báo
    Widget& operator=(Widget&&) noexcept;
private:
    struct Impl;                        // incomplete type
    std::unique_ptr<Impl> pImpl;
};

// widget.cpp — nơi Impl là complete type
struct Widget::Impl { std::string name; std::vector<double> data; Gadget g; };
Widget::Widget() : pImpl(std::make_unique<Impl>()) {}
Widget::~Widget() = default;                        // ✅ default TẠI ĐÂY
Widget::Widget(Widget&&) noexcept = default;
Widget& Widget::operator=(Widget&&) noexcept = default;
```

Vì sao: default deleter của `unique_ptr` gọi `static_assert(sizeof(T) > 0)` trước khi `delete` — destructor sinh tự động (inline trong header) sẽ instantiate chỗ đó khi `Impl` còn incomplete → **compile error khó hiểu**. Đẩy định nghĩa dtor (và move, copy nếu cần — copy phải tự viết deep copy) xuống .cpp nơi `Impl` đã complete. Với `shared_ptr` thì không gặp (deleter không thuộc kiểu) — nhưng Pimpl đúng ngữ nghĩa là exclusive ownership → `unique_ptr` là lựa chọn đúng.

### Insight đáng nhớ

- Bảng chi phí — chọn công cụ theo giá:

| | Size | Cấp phát | Copy | Chi phí deref |
|---|---|---|---|---|
| raw ptr / `unique_ptr` | 1 ptr | — / theo object | move-only (unique) | 0 |
| `shared_ptr` | 2 ptr | +control block | atomic RMW count | 0 |
| `weak_ptr` | 2 ptr | dùng CB có sẵn | atomic (weak count) | phải `lock()` |

- Triết lý ownership: `unique_ptr` là **mặc định**; `shared_ptr` chỉ khi thật sự nhiều chủ ngang hàng không rõ ai chết sau cùng; hàm chỉ "mượn" nhận raw pointer/reference — **ownership là chuyện của interface, không phải của mọi biến**.
- Control block là khái niệm mở khóa cả cụm: hiểu nó là tự trả lời được double-free từ raw pointer, `enable_shared_from_this`, vì sao make_shared nhanh, vì sao weak_ptr giam bộ nhớ.

### Ít quan trọng

- Chi tiết bố cục bộ nhớ khi deleter có state lớn với `unique_ptr` (Item 18, phần cuối).
- `std::allocate_shared` và tương tác với custom allocator (Item 21).
- So sánh hiệu năng Pimpl trước/sau và bàn luận copy semantics cho Pimpl class (Item 22, phần cuối).

### Góc interview

**Câu 1:** `unique_ptr` khác `shared_ptr` những gì? Khi nào chọn cái nào?

<details><summary>Đáp án</summary>

| Tiêu chí | `unique_ptr` | `shared_ptr` |
|---|---|---|
| Ownership | Độc quyền, move-only | Chia sẻ, đếm tham chiếu |
| Size | = raw pointer | = 2 raw pointer |
| Cấp phát phụ | Không | Control block (ref counts atomic, deleter) |
| Chi phí copy | Không copy được | Atomic increment/decrement |
| Custom deleter | Một phần của **kiểu** (đổi size nếu có state) | Nằm trong control block — không đổi kiểu/size |
| Chuyển đổi | → shared_ptr được (một chiều) | Không quay lại unique được |

- Chọn: **mặc định `unique_ptr`** — đủ cho đại đa số (factory, Pimpl, thành viên sở hữu tài nguyên). `shared_ptr` chỉ khi lifetime thật sự do nhiều bên quyết định (cache + client, callback giữ object sống, đa luồng không rõ ai kết thúc sau). Hàm chỉ sử dụng object: nhận `T&`/`T*`, đừng nhận smart pointer.
- **Bẫy khi trả lời:** quên chữ "atomic" của ref count (điểm mấu chốt về chi phí, và là lý do shared_ptr copy nhiều trong hot path gây chậm); nói "shared_ptr thread-safe" chung chung — chỉ **ref count** thread-safe, còn object trỏ tới thì không.

</details>

**Câu 2:** Code sau sai ở đâu, hậu quả và cách sửa?

```cpp
Widget* raw = new Widget;
std::shared_ptr<Widget> p1(raw);
std::shared_ptr<Widget> p2(raw);
```

<details><summary>Đáp án</summary>

- Mỗi lần construct `shared_ptr` từ **raw pointer**, nó tạo **control block MỚI** — nó không có cách nào biết đã có control block khác cho cùng object. Ở đây Widget có 2 control block, mỗi cái strong count = 1.
- Hậu quả: `p1` hết đời → count #1 về 0 → `delete raw`. `p2` hết đời → count #2 về 0 → **delete lần 2** → heap corruption/UB.
- Sửa (theo thứ tự ưu tiên): (1) `auto p1 = std::make_shared<Widget>();` rồi `auto p2 = p1;` — copy shared_ptr chia sẻ control block; (2) nếu buộc phải từ raw: chỉ tạo **một** shared_ptr từ nó, các bản sau copy từ bản đầu.
- Biến thể hay bị hỏi kèm: trong member function trả `shared_ptr(this)` — cùng bug. Fix bằng `std::enable_shared_from_this` + `shared_from_this()`, với điều kiện object đã được một shared_ptr bên ngoài quản lý (thường ép bằng factory trả shared_ptr + constructor private).

</details>

**Câu 3:** `weak_ptr` giải quyết vấn đề gì mà raw pointer không làm được? Vẽ tình huống vòng tham chiếu.

<details><summary>Đáp án</summary>

- Raw pointer không sở hữu nhưng **không thể biết object còn sống hay đã hủy** → dùng sau khi hủy là UB. `weak_ptr` = "không sở hữu nhưng kiểm tra được": `lock()` trả shared_ptr hợp lệ hoặc null — thao tác **atomic**, không race giữa kiểm tra và sử dụng.
- Vòng tham chiếu:

```
   ┌────────── shared_ptr ──────────┐
   ▼                                │
┌──────┐                        ┌───┴───┐
│  A   │──── shared_ptr ───────►│   B   │
└──────┘                        └───────┘
strong(A) = 1 (từ B)         strong(B) = 1 (từ A)
→ dù mọi shared_ptr bên ngoài đã hủy, count không về 0 → cả A, B leak vĩnh viễn

Fix: B → A đổi thành weak_ptr:
┌──────┐                        ┌───────┐
│  A   │──── shared_ptr ───────►│   B   │
└──────┘◄─── weak_ptr ──────────┘
→ bên ngoài thả A → strong(A) = 0 → A hủy → thả shared_ptr B → B hủy. Sạch.
```

- Use case khác: cache (trả shared_ptr từ weak_ptr nếu còn), observer (subject giữ `vector<weak_ptr<Observer>>`).
- **Bẫy khi trả lời:** dùng `expired()` rồi mới `lock()` — hai bước riêng có race trong đa luồng; đúng bài là `if (auto sp = wp.lock())`. Và nhớ: weak_ptr giữ **control block** sống (weak count) chứ không giữ object — liên quan điểm trừ của `make_shared` với object lớn.

</details>

**Câu 4:** Vì sao `make_shared` thường nhanh hơn `shared_ptr<T>(new T)`? Khi nào *không* nên dùng `make_shared`?

<details><summary>Đáp án</summary>

- `shared_ptr<T>(new T)`: **2 lần cấp phát** — một cho T, một cho control block. `make_shared<T>`: **1 lần** cho khối gộp [T | control block] → nhanh hơn, locality tốt hơn, ít fragmentation (đáng kể trên embedded heap nhỏ).

```
new + shared_ptr:   [ T ]        [ control block ]     ← 2 malloc, 2 vùng rời
make_shared:        [ T | control block ]              ← 1 malloc
```

- Kèm lợi ích exception safety: `f(shared_ptr<T>(new T), g())` — C++11/14 cho phép xen kẽ đánh giá, `g()` ném giữa `new` và constructor shared_ptr → leak; `make_shared` gói kín.
- Không dùng `make_shared` khi: (1) cần **custom deleter** (make không có chỗ truyền); (2) class có `operator new/delete` riêng (khối gộp không đi qua chúng); (3) object **lớn** + có `weak_ptr` **sống lâu**: khối gộp chỉ được free khi weak count về 0 → bộ nhớ object bị giam dù object đã destruct; (4) cần khởi tạo bằng braced initializer trực tiếp.

</details>

**Câu 5 (bẫy 🎯):** Pimpl bằng `unique_ptr`, code compile lỗi "invalid application of 'sizeof' to an incomplete type" tại nơi dùng `Widget w;` trong file client. Vì sao và sửa thế nào?

<details><summary>Đáp án</summary>

- Widget không khai báo destructor → compiler sinh dtor **inline** ngay tại chỗ `w` bị hủy trong file client. Dtor đó gọi dtor của `unique_ptr<Impl>`, mà default deleter `delete pImpl` có `static_assert(sizeof(Impl) > 0)` — tại file client, `Impl` là **incomplete type** → lỗi.
- Sửa: khai báo `~Widget();` trong header, và **định nghĩa trong widget.cpp** (sau định nghĩa `struct Widget::Impl`): `Widget::~Widget() = default;`. Move/copy operations tương tự (khai trong header, `= default`/tự viết trong .cpp).
- Ý ăn điểm: giải thích vì sao `shared_ptr<Impl>` không dính lỗi này — deleter của shared_ptr không phải một phần của kiểu, được lưu runtime trong control block, không cần complete type tại điểm sinh dtor; nhưng Pimpl vẫn nên dùng `unique_ptr` vì ownership là độc quyền.

</details>

### Đọc thêm (tùy chọn)

- [02/raii-smart-pointers.md](../02-modern-cpp/raii-smart-pointers.md) — nền RAII + smart pointer của repo.
- [07/api-design.md](../07-shared-libraries/api-design.md) — Pimpl trong thiết kế API/ABI của shared library.

## Cụm 5 — Move Semantics & Perfect Forwarding (ch. 5, Items 23–30) 🎯

### Nội dung chính

**Item 23 — `std::move` không move, `std::forward` không forward.** Cả hai chỉ là **cast**:
- `std::move(x)` = cast **vô điều kiện** sang rvalue (`static_cast<remove_reference_t<T>&&>(x)`). Nó chỉ làm `x` **đủ điều kiện** để bị move — move thật xảy ra (hay không) ở constructor/assignment được chọn bởi overload resolution.
- `std::forward<T>(x)` = cast **có điều kiện**: chỉ cast sang rvalue nếu argument gốc là rvalue (thông tin nằm trong `T` — xem Item 28).

Bẫy `const` kinh điển:

```cpp
class Annotation {
public:
    explicit Annotation(const std::string text)
        : value(std::move(text)) {}   // ⚠️ compile OK, tưởng move...
    // std::move(text) có kiểu const std::string&&
    // → không match move ctor (nhận string&&, non-const)
    // → match COPY ctor (const string&) → copy âm thầm!
private:
    std::string value;
};
```

→ Quy tắc: **muốn move thì đừng khai `const`**; và đừng tin `std::move` "chắc chắn nhanh" — nó chỉ là lời đề nghị.

**Item 24 — Phân biệt universal reference với rvalue reference.** `T&&` có **hai nghĩa** tùy ngữ cảnh:

```cpp
void f(Widget&& param);          // rvalue reference — kiểu cụ thể, không deduction
Widget&& var1 = Widget();        // rvalue reference

template<typename T>
void f(T&& param);               // UNIVERSAL reference — có type deduction
auto&& var2 = var1;              // universal reference
```

Điều kiện để là universal reference: (1) có **type deduction cho T**, (2) dạng phải **chính xác `T&&`** — thêm bất kỳ gì là mất:

```cpp
template<typename T> void f(std::vector<T>&& v);  // rvalue ref (không phải T&&)
template<typename T> void f(const T&& v);         // rvalue ref (có const)
// và: trong class template vector<T>, push_back(T&&) là rvalue ref
//     (T đã cố định theo instance, không deduction tại call site);
//     còn emplace_back(Args&&... args) là universal ref (Args deduce mỗi lần gọi)
```

Universal reference = "tham chiếu nhận được mọi thứ": lvalue → thành lvalue ref, rvalue → thành rvalue ref.

**Item 25 — `std::move` cho rvalue reference, `std::forward` cho universal reference — ở lần dùng cuối.**

```cpp
class Widget {
public:
    Widget(Widget&& rhs)                       // rvalue ref → chắc chắn được move
        : name(std::move(rhs.name)) {}

    template<typename T>
    void setName(T&& newName)                  // universal ref → giữ nguyên "tính chất" gốc
        { name = std::forward<T>(newName); }   // lvalue vào → copy; rvalue vào → move
};
```

Dùng `std::move` trên universal reference = **cướp tài nguyên của lvalue** của caller — bug nghiêm trọng. Dùng ở **lần dùng cuối cùng** trong hàm (trước đó object còn cần nguyên vẹn).

**Return value:** hàm trả **by value** một rvalue reference param → `return std::move(rhs);` (tận dụng move). Nhưng **KHÔNG BAO GIỜ** `return std::move(local);` với biến local:

```cpp
Widget makeWidget() {
    Widget w;
    ...
    return w;                // ✅ RVO (copy elision): construct thẳng vào chỗ trả về — 0 copy 0 move
    // return std::move(w);  // ❌ phá điều kiện RVO (phải là tên biến local, không cast)
                             //    → ép move — TỆ HƠN không làm gì
}
```

Chuẩn quy định: chỗ nào RVO được phép mà compiler không làm, `return w;` cũng tự được xử như rvalue — nghĩa là viết tay `std::move` không bao giờ thắng.

**Items 26–27 — Tránh overload trên universal reference; các lối thoát.** Hàm universal reference là **overload tham lam nhất**: nó instantiate ra exact match cho hầu hết mọi kiểu, nuốt luôn các overload bạn tưởng sẽ được chọn:

```cpp
template<typename T> void logAndAdd(T&& name);   // universal ref
void logAndAdd(int idx);                          // overload cho index

short s = 22;
logAndAdd(s);   // ⚠️ gọi bản template (T = short& — exact match)
                //    chứ KHÔNG phải bản int (cần promotion)!
```

Tệ nhất là **perfect-forwarding constructor** — nó nuốt cả copy constructor:

```cpp
class Person {
public:
    template<typename T> explicit Person(T&& n);   // forwarding ctor
    Person(const Person&);                          // copy ctor (compiler sinh)
};
Person p("Nancy");
Person cloneOfP(p);   // ⚠️ p là Person& (non-const) → forwarding ctor exact match,
                      //    copy ctor cần thêm const → THUA. Gọi nhầm bản template!
// và class Derived : Person { Derived(const Derived& r) : Person(r) {} }
// → Person(r) với r kiểu Derived → lại rơi vào forwarding ctor!
```

Các giải pháp (Item 27):
1. **Đừng overload** — đặt tên khác (`logAndAddName` / `logAndAddNameIdx`).
2. **Pass by `const T&`** — từ bỏ chút hiệu năng, đổi lấy đơn giản.
3. **Pass by value** — hợp lý khi kiểu chắc chắn copy được và sẽ được copy (Item 41).
4. **Tag dispatch** — thêm tham số tag `std::is_integral` phân luồng ở compile time:

```cpp
template<typename T>
void logAndAdd(T&& name) {
    logAndAddImpl(std::forward<T>(name),
                  std::is_integral<std::remove_reference_t<T>>());
}
void logAndAddImpl(int idx, std::true_type);                       // nhánh integral
template<typename T> void logAndAddImpl(T&& n, std::false_type);   // nhánh còn lại
```

5. **Ràng buộc template bằng `std::enable_if`** — cách duy nhất cứu forwarding constructor: loại bản template khỏi overload set khi argument là `Person` (hoặc derived):

```cpp
template<typename T,
         typename = std::enable_if_t<
             !std::is_base_of<Person, std::decay_t<T>>::value &&
             !std::is_integral<std::remove_reference_t<T>>::value>>
explicit Person(T&& n) : name(std::forward<T>(n)) {}
```

(C++20 concepts thay thế toàn bộ kỹ thuật này bằng `requires` — dễ đọc hơn nhiều.) Nhược điểm chung của perfect forwarding interface: error message dài khủng khiếp khi kiểu sai lọt sâu vào trong.

**Item 28 — Reference collapsing: cơ chế bên dưới universal reference.** Người dùng không được khai "reference to reference", nhưng compiler tạo ra trong 4 ngữ cảnh (template instantiation, `auto`, `typedef`/alias, `decltype`) và **collapse** theo quy tắc:

```
  Tham chiếu ngoài
       &      &&
  &  [ & ] [ &  ]     ← lvalue reference "thắng":
  && [ & ] [ && ]       chỉ && của && mới ra &&
```

Universal reference hoạt động nhờ 2 quy tắc phối hợp: (1) deduction đặc biệt — lvalue argument → `T = X&`, rvalue → `T = X`; (2) collapsing — `X& &&` → `X&`, `X&&` → `X&&`. `std::forward<T>` chỉ đơn giản khai thác thông tin encode trong `T` đó:

```cpp
template<typename T>
T&& forward(remove_reference_t<T>& param) {
    return static_cast<T&&>(param);
    // T = X&  (lvalue vào)  → X& && → X&   : trả lvalue ref — không move
    // T = X   (rvalue vào)  → X&&          : trả rvalue ref — cho phép move
}
```

**Item 29 — Đừng mặc định move là rẻ.** Ba tình huống move không giúp gì:
- **Kiểu không có move** (legacy C++98, hoặc bị chặn sinh theo Item 17) → `std::move` âm thầm thành copy.
- **Move không rẻ hơn copy**: `std::string` với **SSO** (small string optimization — chuỗi ngắn nằm ngay trong object, không có pointer để "steal") → move = copy từng byte như copy. `std::array` — dữ liệu nằm **trong object**, move là move **từng phần tử** O(n), khác hẳn `vector` (chỉ chuyển 3 con trỏ, O(1)):

```
vector:  [ptr|size|cap] ──→ heap data       move = steal 3 con trỏ  (O(1))
array:   [e0|e1|...|eN] (tại chỗ)           move = move N phần tử   (O(n))
```

- **Move không dùng được**: context đòi noexcept mà move không khai `noexcept` (Item 14).

→ Viết generic code: đừng giả định mọi kiểu move rẻ; đo trước khi tin.

**Item 30 — Các trường hợp perfect forwarding thất bại.** "Thất bại" = deduction fail hoặc deduce ra kiểu sai. Danh sách cần thuộc:
1. **Braced initializer**: `fwd({1, 2, 3})` — template không deduce được braced init (Item 2). Workaround: `auto il = {1,2,3}; fwd(il);`
2. **`0` hoặc `NULL` làm null pointer** — deduce ra `int`, không phải pointer. Dùng `nullptr`.
3. **Declaration-only `static const` integral member** — không có definition → không có địa chỉ → reference (universal ref là reference!) không bind được → link error. (C++17 `inline` variable đã xóa vấn đề này.)
4. **Tên hàm overload / function template** — không biết chọn bản nào để deduce. Workaround: cast tường minh hoặc khai pointer đúng kiểu trước.
5. **Bitfield** — non-const reference không bind được vào bitfield. Workaround: copy ra biến (`static_cast` về kiểu giá trị) rồi forward bản copy.

### Insight đáng nhớ

- Câu thần chú đáng nhớ nhất sách: **"`std::move` doesn't move anything. `std::forward` doesn't forward anything."** Cả hai là cast — move thật do overload resolution quyết định. Nắm câu này là gỡ được 80% hiểu lầm về move.
- Move semantics **không phải phép màu miễn phí**: `const` giết move (Item 23), thiếu `noexcept` giết move trong container (Item 14), SSO/`std::array` khiến move chẳng rẻ hơn (Item 29), tự viết dtor giết luôn việc sinh move (Item 17). "C++11 tự nhanh hơn" chỉ đúng khi cả chuỗi điều kiện này được tôn trọng.
- RVO > move > copy. Đừng "tối ưu" bằng `std::move` ở return — compiler làm tốt hơn bạn.

### Ít quan trọng

- Bàn luận chi tiết lịch sử tên gọi "universal reference" vs "forwarding reference" (chuẩn dùng từ sau) (Item 24).
- Chi tiết cài đặt `std::forward` với remove_reference đầy đủ và biến thể forward sai (Item 28, phần cuối).
- Ví dụ Matrix + expression về pass-by-value cho move-heavy type (Item 25, phần giữa).

### Góc interview

**Câu 1:** `std::move` làm gì? Sau `auto b = std::move(a);` thì `a` ở trạng thái nào?

<details><summary>Đáp án</summary>

- `std::move` **chỉ cast** `a` sang rvalue reference (`static_cast<T&&>`), không di chuyển byte nào. Cast đó khiến overload resolution **chọn move constructor** của kiểu (nếu có) khi khởi tạo `b` — chính move ctor mới "steal" tài nguyên (chép con trỏ nội bộ, gán nguồn về null).
- Trạng thái `a` sau move: chuẩn quy định "**valid but unspecified**" — vẫn là object hợp lệ, destructor chạy an toàn, gán giá trị mới được; nhưng **không được giả định nội dung** (đọc giá trị là bug logic). Với STL container thường là rỗng, nhưng đó là chi tiết cài đặt, không phải cam kết.
- Bẫy nên chủ động nêu: (1) move trên object `const` → âm thầm rơi về **copy** (move ctor nhận `T&&` non-const, không bind vào `const T&&`; copy ctor `const T&` thì nhận được); (2) nếu kiểu không có move ctor → cũng rơi về copy — `std::move` không bao giờ gây lỗi compile vì "không move được", nó chỉ im lặng kém hiệu quả.

</details>

**Câu 2:** Phân biệt `void f(Widget&&)` và `template<typename T> void f(T&&)`. Vì sao cần `std::forward` ở bản template?

<details><summary>Đáp án</summary>

- `Widget&&` là **rvalue reference**: chỉ nhận rvalue (Widget tạm, kết quả `std::move`). `T&&` với T được deduce là **universal/forwarding reference**: nhận cả lvalue lẫn rvalue — lvalue vào thì `T = Widget&` (param collapse thành `Widget&`), rvalue vào thì `T = Widget` (param là `Widget&&`).
- Bên trong hàm, **param luôn là lvalue** (nó có tên!) — truyền tiếp `param` trần sẽ luôn được đối xử như lvalue → mất khả năng move ở tầng dưới. `std::forward<T>(param)` khôi phục "tính chất" gốc dựa vào T: `T = Widget&` → trả lvalue ref (giữ copy), `T = Widget` → cast rvalue (cho move). Đó là "perfect" trong perfect forwarding.
- Điều kiện nhận diện universal ref: có **deduction** + dạng **đúng `T&&`** — `const T&&`, `vector<T>&&`, hay `T&&` trong class template đã fix T đều là rvalue reference thường.
- **Bẫy khi trả lời:** nói "dùng `std::move` trong hàm forwarding" — sai, `std::move` ở đó cướp tài nguyên lvalue của caller; move cho rvalue ref, forward cho universal ref.

</details>

**Câu 3:** `return std::move(localVar);` có nhanh hơn `return localVar;` không? Giải thích RVO.

<details><summary>Đáp án</summary>

- **Không — thường chậm hơn.** Với `return localVar;`, compiler áp dụng **RVO/NRVO (copy elision)**: object local được **construct thẳng vào vị trí giá trị trả về** của caller — 0 copy, 0 move. Điều kiện: (1) kiểu biến local trùng kiểu trả về, (2) biến local là **cái được return trực tiếp** (một cái tên, không phải expression).
- `std::move(localVar)` là expression cast → **phá điều kiện (2)** → compiler mất quyền elide, buộc phải gọi move ctor thật. Kết quả: từ "0 thao tác" thành "1 move".
- Chốt hạ: chuẩn còn quy định — chỗ nào đủ điều kiện RVO mà compiler chọn không elide, `return localVar;` **tự động được xử lý như rvalue** (move). Tức viết `std::move` không bao giờ thắng, chỉ hòa hoặc thua.
- Ngoại lệ hợp lệ: return một **tham số** by-value/rvalue-reference (tham số không bao giờ được RVO) → `return std::move(param);` là đúng bài.
- **Bẫy khi trả lời:** khoe "luôn move ở return cho chắc" — đây là anti-pattern có tên riêng trong sách ("pessimization"); và từ C++17, copy elision với **prvalue** (return Widget{...}) là **bắt buộc**, không còn là tối ưu tùy chọn.

</details>

**Câu 4 (bẫy 🎯):** Vì sao constructor dạng `template<typename T> Person(T&&)` có thể "nuốt" copy constructor? Hậu quả và cách chặn?

<details><summary>Đáp án</summary>

- Gọi `Person cloneOfP(p);` với `p` là `Person` **non-const**: overload resolution so sánh (1) forwarding ctor instantiate thành `Person(Person&)` — **exact match**; (2) copy ctor `Person(const Person&)` — cần thêm `const`. Exact match thắng → chạy bản template, không phải copy ctor. Với class kế thừa còn tệ hơn: `Derived(const Derived& r): Person(r)` — `r` kiểu `Derived` không bao giờ khớp chính xác copy ctor của Person → luôn rơi vào forwarding ctor.
- Hậu quả: bản template thường làm việc khác hẳn copy (vd forward vào `std::string` → compile error dài hàng trang, hoặc hành vi sai lệch âm thầm).
- Cách chặn: loại bản template khỏi overload set khi T là Person/derived — `std::enable_if_t<!std::is_base_of<Person, std::decay_t<T>>::value>` (C++11/14), hoặc `requires (!std::derived_from<std::remove_cvref_t<T>, Person>)` (C++20). `std::decay_t` cần thiết để xử lý cả `Person&`, `const Person&`.
- Ý ăn điểm: quy tắc chung của sách — **tránh overload trên universal reference**; nếu API cần cả hai hành vi, cân nhắc tag dispatch hoặc đặt tên hàm khác trước khi với tới enable_if.

</details>

**Câu 5:** Move của `std::vector` và `std::array` khác nhau thế nào về độ phức tạp? `std::string` thì sao?

<details><summary>Đáp án</summary>

- `std::vector<T>`: dữ liệu nằm trên **heap**, object chỉ giữ [ptr | size | capacity] → move = chép 3 con trỏ + null hóa nguồn = **O(1)**, không đụng phần tử.
- `std::array<T, N>`: dữ liệu nằm **ngay trong object** (stack/inline) — không có con trỏ nào để steal → move = gọi move ctor **từng phần tử** = **O(n)**; nếu T không move được thì là copy từng phần tử. Move array không rẻ hơn copy array bao nhiêu (trừ khi phần tử move rẻ hơn copy).
- `std::string`: thường có **SSO** — chuỗi ngắn (~15 ký tự tùy implementation) lưu ngay trong object, không heap → move chuỗi ngắn = **chép byte như copy**, không nhanh hơn. Chuỗi dài (trên heap) → move O(1) như vector.
- Bài học generic code: "move luôn rẻ" là giả định sai; hiệu năng phụ thuộc **dữ liệu nằm trong object hay ngoài heap**.

</details>

### Đọc thêm (tùy chọn)

- [02/move-semantics.md](../02-modern-cpp/move-semantics.md) — nền move/rvalue của repo.
- [01/templates.md](../01-cpp-fundamentals/templates.md) — deduction (Cụm 1) là tiền đề của universal reference.

## Cụm 6 — Lambdas (ch. 6, Items 31–34)

### Nội dung chính

Từ vựng chuẩn trước đã: **lambda expression** = cú pháp trong source; nó sinh ra một **closure class** (compiler tự đặt tên) và một **closure object** ở runtime. Capture list quyết định closure object chứa gì.

**Item 31 — Tránh default capture (`[&]`, `[=]`) — cả hai đều có bẫy dangling.**

**`[&]` — dangling reference lộ liễu:**

```cpp
std::vector<std::function<bool(int)>> filters;

void addDivisorFilter() {
    int divisor = computeDivisor();
    filters.emplace_back([&](int value) { return value % divisor == 0; });
    // ⚠️ closure giữ REFERENCE tới biến local divisor
}   // ← divisor chết ở đây → mọi lần gọi filter sau này là UB
```

Viết rõ `[&divisor]` không sửa được bug nhưng ít nhất **lộ tên biến** để reviewer soi lifetime.

**`[=]` — cảm giác an toàn giả tạo, 2 bẫy:**

1. **Capture `this`, không capture member:**

```cpp
class Widget {
    int divisor;
public:
    void addFilter() const {
        filters.emplace_back([=](int value) { return value % divisor == 0; });
        // ⚠️ member không capture được! Thực chất compiler capture con trỏ this,
        //    divisor nghĩa là this->divisor
    }
};
// Widget hủy trước khi filter được gọi → this dangling → UB, dù đã "capture by value"!
// Fix C++11: int divisorCopy = divisor; capture [divisorCopy]
// Fix C++14: [divisor = divisor] (init capture) — copy thật sự
```

2. **Biến static không được capture** nhưng vẫn dùng được trong lambda → `[=]` gợi ý "mọi thứ là bản copy độc lập" nhưng biến static thay đổi sau đó **ảnh hưởng closure** — hiểu lầm ngữ nghĩa.

**Item 32 — Init capture (C++14): move object vào closure.** C++11 chỉ có capture by value/reference — không move được (vấn đề với `unique_ptr`, object đắt copy). C++14 thêm **init capture** (generalized lambda capture): tự đặt tên member của closure + biểu thức khởi tạo nó:

```cpp
auto pw = std::make_unique<Widget>();
auto func = [pw = std::move(pw)] {           // member 'pw' của closure ← move từ pw ngoài
    return pw->isValidated();
};
// hoặc tạo thẳng: [pw = std::make_unique<Widget>()] {...}
```

Workaround C++11 (để hiểu bản chất): tự viết functor class, hoặc `std::bind` object move vào bind rồi lambda nhận reference — C++14 làm những kỹ thuật này lỗi thời.

**Item 33 — Generic lambda + perfect forwarding: `auto&&` và `decltype`.** Lambda C++14 nhận `auto` param; muốn forward tiếp phải lấy được "T" — nhưng lambda không có T tường minh → dùng `decltype`:

```cpp
auto f = [](auto&& param) {
    return doWork(std::forward<decltype(param)>(param));
    // param là lvalue  → decltype(param) = X&  → forward giữ lvalue
    // param là rvalue  → decltype(param) = X&& → forward cast rvalue (collapse X&& && → X&&)
};
auto fVariadic = [](auto&&... params) {
    return doWork(std::forward<decltype(params)>(params)...);
};
```

**Item 34 — Lambda thay cho `std::bind`.** Lambda thắng ở mọi mặt đáng kể: dễ đọc hơn, inline được (bind gọi qua function pointer → khó inline), và bind có các bẫy ngữ nghĩa: argument được **đánh giá tại thời điểm gọi bind** hay khi gọi callable? (ví dụ `steady_clock::now() + 1h` bị chốt sớm); tên hàm overload phải cast tay; mọi argument truyền by-reference ngầm khó thấy. C++14 trở đi: gần như không còn lý do dùng bind (use case cuối cùng của C++11 — move capture và polymorphic call — đều được C++14 giải quyết).

### Insight đáng nhớ

- Lambda chỉ là **đường tắt cú pháp cho functor class** — nhớ mô hình này là mọi câu hỏi về capture tự sáng: capture by value = data member copy, by reference = member reference, `[=]` trong member function = **member con trỏ `this`**. Không có phép màu.
- Quy tắc thực chiến: **luôn liệt kê capture tường minh**; với callback sống lâu hơn scope tạo ra nó (hàng đợi, timer, thread — rất phổ biến trong embedded middleware), mặc định nghi ngờ mọi capture reference và mọi `this`.
- C++17 thêm `[*this]` (copy cả object) cho đúng vấn đề của Item 31; C++20 deprecate capture `this` ngầm qua `[=]` — chứng tỏ bẫy này thật đến mức chuẩn phải sửa.

### Ít quan trọng

- Chi tiết so sánh từng dòng lambda vs bind và bản cài `IsValAndArch` bằng class tay (Item 34, Item 32 phần C++11 workaround).

### Góc interview

**Câu 1 (bẫy):** Đoạn code sau có bug lifetime nào? Sửa cho cả C++11 và C++14.

```cpp
class Controller {
    int threshold_;
public:
    void registerCallback(EventBus& bus) {
        bus.subscribe([=](const Event& e) {
            if (e.level > threshold_) alert(e);
        });
    }
};
```

<details><summary>Đáp án</summary>

- `[=]` **không copy `threshold_`** — member không capture được. Compiler capture **con trỏ `this`**; `threshold_` trong lambda nghĩa là `this->threshold_`. Nếu `Controller` bị hủy trong khi callback còn nằm trong EventBus (rất điển hình: bus sống lâu hơn controller) → mọi event đến sau đó deref `this` dangling → UB.
- Fix C++14 (init capture — copy giá trị thật):

```cpp
bus.subscribe([threshold = threshold_](const Event& e) {
    if (e.level > threshold) alert(e);
});
```

- Fix C++11: `int t = threshold_;` rồi capture `[t]`.
- Nếu callback *cần* gọi member function (cần object sống): quản lý Controller bằng `shared_ptr`, capture `[self = shared_from_this()]` (giữ sống) hoặc `weak_ptr` + `lock()` trong lambda (không giữ sống, kiểm tra trước khi dùng) — pattern chuẩn của async code.
- **Bẫy khi trả lời:** khẳng định "`[=]` copy hết nên an toàn" — chính là hiểu lầm mà item này tồn tại để sửa.

</details>

**Câu 2:** Closure của lambda `[x, &y](int v){ return x + y + v; }` thực chất là gì? Nó nằm ở stack hay heap?

<details><summary>Đáp án</summary>

- Compiler sinh một **class** tương đương:

```cpp
class __Closure {
    int  x;    // capture by value → data member copy
    int& y;    // capture by reference → member reference
public:
    __Closure(int x_, int& y_) : x(x_), y(y_) {}
    auto operator()(int v) const { return x + y + v; }   // mặc định const
};
```

- Closure object là biến bình thường — khai báo `auto c = [...]` trên stack thì nó nằm **trên stack**, to đúng bằng tổng capture (lambda không capture → size 1 byte, convert được sang function pointer). **Không có heap allocation nào** từ bản thân lambda.
- Heap chỉ xuất hiện khi gói vào `std::function` (closure lớn hơn small-buffer của nó) — đây là điểm phân biệt quan trọng về chi phí (Item 5).
- Chi tiết ăn điểm: `operator()` mặc định `const` → không sửa được capture-by-value trừ khi khai `mutable`; capture by reference thì sửa đối tượng gốc được (reference member không cần mutable).

</details>

### Đọc thêm (tùy chọn)

- [02/lambdas-functional.md](../02-modern-cpp/lambdas-functional.md) — nền lambda của repo.

## Cụm 7 — Concurrency API (ch. 7, Items 35–40) 🎯

### Nội dung chính

**Item 35 — Ưu tiên task-based (`std::async`) hơn thread-based (`std::thread`).** Ba tầng khái niệm "thread": hardware thread (core/hyperthread), software/OS thread (kernel schedule), `std::thread` (object C++ — handle của OS thread). Task-based đẩy các vấn đề tầng thấp cho runtime:

```cpp
int doAsyncWork();
std::thread t(doAsyncWork);           // thread-based: KHÔNG lấy được return value;
                                      // exception trong doAsyncWork → std::terminate
auto fut = std::async(doAsyncWork);   // task-based: fut.get() trả về kết quả
                                      // HOẶC ném lại exception của task — xử lý được
```

`std::thread` bắt bạn tự đối mặt: **oversubscription** (nhiều software thread hơn hardware → context switch, cache thrashing), **load balancing**, và **resource exhaustion** (`std::system_error` khi hết thread — kể cả khi hàm là `noexcept`). `std::async` (default policy) được phép "không tạo thread mới" — chạy deferred trên thread gọi `get()` → tự giảm oversubscription. Vẫn cần `std::thread` trực tiếp khi: cần API của thread bên dưới (`native_handle()` → `pthread_setaffinity_np`, priority — nhu cầu thật của embedded/realtime), cần tự quản lý oversubscription cho app đặc thù, hoặc implement công nghệ threading riêng (thread pool).

**Item 36 — Chỉ định `std::launch::async` nếu cần bất đồng bộ thật.** Default policy = `async | deferred` — hệ thống tự chọn:
- `std::launch::async` — chạy trên **thread khác**, bắt đầu ngay.
- `std::launch::deferred` — **không chạy** cho tới khi `get()/wait()`, chạy **đồng bộ trên thread gọi get**; không bao giờ gọi get → **không bao giờ chạy**.

Hệ quả của default: không đoán được có concurrent hay không → bug với `thread_local` (chạy trên thread nào?), và **treo vô hạn** với vòng lặp polling:

```cpp
auto fut = std::async(f);                        // default policy
while (fut.wait_for(100ms) != std::future_status::ready) {  
    // ⚠️ nếu f bị DEFERRED: wait_for luôn trả future_status::deferred
    //    → không bao giờ 'ready' → loop vĩnh viễn!
}
// Fix: kiểm tra deferred trước
if (fut.wait_for(0s) == std::future_status::deferred) {
    fut.get();                                    // chạy đồng bộ
} else {
    while (fut.wait_for(100ms) != std::future_status::ready) { ... }
}
```

Quy tắc: dùng default khi (task không cần chạy concurrent thật, không đụng `thread_local`, chấp nhận có thể không bao giờ chạy nếu không get); ngược lại viết rõ `std::async(std::launch::async, f)`.

**Item 37 — `std::thread` phải unjoinable trên mọi đường ra khỏi scope.** Destructor của một `std::thread` **joinable** → gọi `std::terminate`. Lý do thiết kế: hai lựa chọn còn lại đều tệ hơn — implicit `join` (chờ ngầm → treo khó hiểu), implicit `detach` (thread tiếp tục ghi vào stack frame đã chết → UB kinh dị). Chuẩn chọn "chết to, chết rõ".

```cpp
void doWork() {
    std::thread t(task);
    ...
    if (conditionFails()) return;   // ⚠️ t joinable khi hủy → std::terminate!
    t.join();
}
```

Fix chuẩn: **RAII guard** — mọi đường ra đều xử lý:

```cpp
class ThreadRAII {
public:
    enum class DtorAction { join, detach };
    ThreadRAII(std::thread&& t, DtorAction a) : action(a), t(std::move(t)) {}
    ~ThreadRAII() {
        if (t.joinable()) {                     // check trước — join/detach trên
            if (action == DtorAction::join) t.join();   // unjoinable là UB
            else t.detach();
        }
    }
    ThreadRAII(ThreadRAII&&) = default;         // dtor tự viết → move phải khai lại (Item 17)
    ThreadRAII& operator=(ThreadRAII&&) = default;
    std::thread& get() { return t; }
private:
    DtorAction action;
    std::thread t;                              // khai CUỐI danh sách member — init sau cùng
};
```

(C++20 thêm `std::jthread` — chính là ý tưởng này vào chuẩn, kèm stop token.)

**Item 38 — Destructor của future: thường không block, TRỪ một trường hợp.** Future là một đầu của **kênh caller ↔ callee**; kết quả callee nằm trong **shared state** (heap):

```
  caller                                    callee
  ┌────────┐      shared state (heap)      ┌─────────┐
  │ future │ ───→ ┌──────────────────┐ ←── │ promise/│
  └────────┘      │ result, flags,   │     │ async   │
                  │ ref counts       │     └─────────┘
                  └──────────────────┘
```

- Quy tắc thường: dtor của future chỉ hủy member + giảm ref count shared state. **Không join, không chờ.**
- **Ngoại lệ:** future là cái **cuối cùng** trỏ tới shared state của task chạy qua **`std::async` với policy `async`** (task chưa xong) → dtor **block đến khi task chạy xong** (implicit join). Hệ quả thực tế: vứt future của std::async đi không "fire-and-forget" được — nó chờ!
- Future từ `std::promise`/`std::packaged_task` không có hành vi đặc biệt này (nhưng chú ý: bạn vẫn phải quản lý thread chạy packaged_task theo Item 37).

**Item 39 — Giao tiếp sự kiện one-shot: void future gọn hơn condvar/flag.** So sánh các cách thread A báo "sự kiện xảy ra" cho thread B:

| Cách | Vấn đề |
|---|---|
| `std::condition_variable` | Cần mutex đi kèm; phải xử lý **spurious wakeup** (wait với predicate); **lost wakeup** — notify trước khi bên kia wait là mất |
| Flag `std::atomic<bool>` + poll | Không mất event, nhưng thread chờ **busy-wait chiếm CPU** |
| condvar + flag kết hợp | Đúng nhưng rườm rà, dễ viết sai |
| **`std::promise<void>` + future** | Gọn, không cần mutex, không spurious/lost wakeup |

```cpp
std::promise<void> p;
// thread chờ:
p.get_future().wait();       // block thật (không poll), dậy đúng một lần
// thread báo:
p.set_value();               // "sự kiện xảy ra"
```

Giới hạn phải nêu: **one-shot** — promise set một lần duy nhất, không tái sử dụng; có cấp phát heap (shared state). Sự kiện lặp lại nhiều lần → quay về condvar + predicate. Use case đẹp trong sách: tạo thread ở trạng thái "treo" chờ lệnh xuất phát (suspend rồi thả), kết hợp `std::shared_future` để thả **nhiều** thread cùng lúc.

**Item 40 — `std::atomic` cho concurrency, `volatile` cho special memory.** Câu hỏi phân biệt kinh điển, đặc biệt đắt với embedded:

- **`std::atomic<T>`**: thao tác **atomic đối với thread khác** — RMW (`++`, `fetch_add`, CAS) là một khối không chia cắt; đồng thời áp đặt **ordering** (mặc định sequentially consistent — code trước store không bị dời xuống sau, các thread thấy thứ tự nhất quán). Nhưng compiler **được phép** tối ưu (gộp, loại) các access "thừa" miễn giữ ngữ nghĩa với thread khác.
- **`volatile`**: bảo compiler "bộ nhớ này **đặc biệt** — mỗi lần đọc/ghi trong code là một lần đọc/ghi THẬT, không được gộp, không được loại, không đảo với volatile khác" — chính xác là hợp đồng cần cho **memory-mapped I/O** (đọc thanh ghi 2 lần có thể ra 2 giá trị; ghi tưởng-như-thừa vẫn là lệnh gửi phần cứng). Nhưng volatile **KHÔNG** atomic, **KHÔNG** tạo ordering/fence giữa các thread → dùng volatile để đồng bộ thread là **data race = UB**.

```cpp
volatile uint32_t* status = reinterpret_cast<uint32_t*>(0x4000'0100);
auto a = *status;   // 2 lần đọc THẬT từ thanh ghi — không được gộp
auto b = *status;   // (không volatile: compiler có thể tối ưu còn 1 lần đọc)

std::atomic<bool> ready{false};   // cờ giữa 2 thread — PHẢI là atomic, không phải volatile
```

Hai thứ trực giao, kết hợp được: `volatile std::atomic<uint32_t>` — thanh ghi đặc biệt *và* truy cập từ nhiều thread.

### Insight đáng nhớ

- Chương này thực chất dạy một nguyên tắc: **API mức cao (task/future) mã hóa sẵn các quyết định khó** (join hay detach? exception đi đâu? bao nhiêu thread?) — dùng `std::thread` trần nghĩa là tự trả lời tất cả, và Item 37/38 cho thấy trả lời sai thì `std::terminate`/block bất ngờ. Với embedded thì ngược lại một phần: cần affinity/priority → buộc xuống `std::thread::native_handle()` — biết *vì sao* phải xuống mới là hiểu.
- Item 40 là ranh giới đúng chuyên môn embedded: `volatile` = giao tiếp với **phần cứng**, `atomic` = giao tiếp giữa **thread**. Ai nói "volatile để thread-safe" là red flag — chính bạn có thể được phỏng vấn câu này theo cả hai chiều.
- Future dtor block (Item 38) là hành vi bất ngờ nhất chương — "fire and forget" bằng `std::async` không tồn tại.

### Ít quan trọng

- Bản ThreadRAII đầy đủ + bàn luận interrupt point (Item 37, phần cuối); chi tiết cài đặt polling loop kết hợp deferred check (Item 36); phiên bản dùng `std::shared_future` thả nhiều thread và detour về copyability của future (Item 39, phần cuối).

### Góc interview

**Câu 1:** `std::async` khác gì tự tạo `std::thread` rồi dùng `std::promise`? Khi nào buộc phải dùng `std::thread`?

<details><summary>Đáp án</summary>

- `std::async(f)` trả `std::future` nhận **return value** của f; **exception** trong f được lưu vào shared state và **ném lại tại `get()`** — với `std::thread`, exception thoát khỏi hàm thread → `std::terminate`, còn return value phải tự chuyển qua promise/biến chung.
- `std::async` để runtime quyết định lịch chạy (default policy có thể **deferred** — chạy lười trên thread gọi get) → tự tránh oversubscription/hết thread; `std::thread` luôn tạo OS thread thật, hết tài nguyên → ném `system_error`.
- Kèm trách nhiệm: thread object phải join/detach trước khi hủy (Item 37); future từ async thì dtor tự "join" trường hợp cuối (Item 38).
- Buộc dùng `std::thread` khi cần **kiểm soát thread thật**: `native_handle()` để set affinity/priority/scheduling policy (SCHED_FIFO cho realtime — nhu cầu embedded điển hình), đặt tên thread, stack size (qua native API), hoặc xây thread pool riêng.
- **Bẫy khi trả lời:** quên default launch policy — nói "`std::async` luôn chạy song song" là sai; nó *được phép* deferred, muốn chắc chắn phải `std::launch::async`.

</details>

**Câu 2 (bẫy 🎯):** Code này có vấn đề gì?

```cpp
void process() {
    auto fut = std::async(std::launch::async, heavyTask);
    if (quickCheck()) return;          // đường ra sớm
    use(fut.get());
}
```

<details><summary>Đáp án</summary>

- Tại `return` sớm, `fut` bị hủy. Đây đúng ngoại lệ của Item 38: future **cuối cùng** trỏ vào shared state của task `std::launch::async` chưa hoàn thành → destructor **block chờ heavyTask chạy xong**. Hàm tưởng return ngay hóa ra treo — "đường ra nhanh" thành "đường ra chậm nhất".
- Không phải bug UB, mà là **bug hiệu năng/latency tiềm ẩn** — loại khó thấy nhất khi review.
- Hướng xử lý tùy ý định: (1) thật sự muốn hủy sớm → thiết kế task có cơ chế dừng (cờ atomic/stop token) và vẫn chấp nhận chờ nó thoát; (2) muốn fire-and-forget → đừng dùng `std::async`; dùng thread pool/executor hoặc `std::thread` + detach (kèm mọi rủi ro lifetime của detach); (3) chấp nhận chờ → ít nhất ghi rõ comment.
- Ý ăn điểm: nêu quy tắc gốc — dtor future bình thường **không** block; chỉ tổ hợp (cuối cùng + std::async + policy async + chưa xong) mới block. Người nhớ mỗi vế "future dtor block" là nhớ sai.

</details>

**Câu 3 (🎯 embedded):** Phân biệt `volatile` và `std::atomic`. Khi nào dùng cái nào, khi nào cả hai?

<details><summary>Đáp án</summary>

| | `volatile` | `std::atomic<T>` |
|---|---|---|
| Ý nghĩa | "Bộ nhớ đặc biệt": mỗi access trong code = một access thật, không gộp/loại/đảo (với volatile khác) | Thao tác không chia cắt giữa các **thread** + ràng buộc ordering (mặc định seq_cst) |
| Atomic RMW (`++`, CAS) | ❌ đọc-sửa-ghi vẫn 3 bước, race được | ✅ |
| Memory ordering giữa threads | ❌ không fence gì | ✅ (acquire/release/seq_cst) |
| Chặn tối ưu hóa access | ✅ đó là mục đích | ❌ compiler vẫn được tối ưu miễn giữ ngữ nghĩa |
| Dùng cho | **Memory-mapped I/O**, thanh ghi phần cứng, bộ nhớ DMA/ISR ghi vào* | Cờ/biến chia sẻ **giữa threads** |

- Dùng volatile đồng bộ 2 thread → vẫn là **data race** theo memory model C++11 = UB (kể cả khi "chạy có vẻ đúng" trên x86 vốn ordering mạnh — sang ARM ordering yếu là lộ).
- Cả hai cùng lúc: `volatile std::atomic<uint32_t> reg;` — thanh ghi/bộ nhớ đặc biệt *và* nhiều thread truy cập.
- (*) Chú thích trung thực: với ISR/DMA, volatile là điều kiện cần cho việc "đọc thật từ bộ nhớ", nhưng đồng bộ đúng nghĩa với ISR còn cần disable interrupt/atomic tùy nền tảng — volatile một mình không đủ.
- **Bẫy khi trả lời:** "volatile là phiên bản nhẹ của atomic" — sai hoàn toàn, hai hợp đồng trực giao; và "atomic thì không bao giờ bị compiler tối ưu" — sai nốt (hai store liên tiếp cùng biến atomic có thể được gộp nếu không thay đổi hành vi quan sát được).

</details>

**Câu 4:** Vì sao destructor của `std::thread` joinable lại gọi `std::terminate` thay vì tự join hoặc tự detach? Pattern xử lý đúng?

<details><summary>Đáp án</summary>

- Ban thiết kế chuẩn cân 3 phương án cho "thread bị hủy khi còn joinable":
  1. **Implicit join** — dtor chờ thread chạy xong: chương trình **treo ngầm** ở cuối scope, cực khó debug (return sớm ở dòng 5, treo ở dòng }).
  2. **Implicit detach** — thread tiếp tục chạy trong khi stack frame chứa dữ liệu nó đang dùng (tham chiếu local, buffer) **đã bị hủy/tái sử dụng** → memory corruption kiểu "ma ám", tệ nhất trong các loại UB.
  3. **`std::terminate`** — chết ngay, to, rõ, có core dump chỉ đúng chỗ quên join.
  Chuẩn chọn (3): lỗi lập trình thì phải lộ, không được âm thầm.
- Pattern đúng: **RAII guard** — object bọc `std::thread`, destructor kiểm tra `joinable()` rồi join (hoặc detach) theo chính sách khai báo lúc tạo → mọi đường ra (return sớm, exception) đều được xử lý. C++20 chuẩn hóa thành `std::jthread` (auto-join + `std::stop_token` để yêu cầu dừng).
- Ý ăn điểm: nhắc case ngược — gọi `join()`/`detach()` trên thread **đã** unjoinable cũng là lỗi (ném `system_error`), nên guard phải check `joinable()` trước; và default-constructed/moved-from thread là unjoinable.

</details>

### Đọc thêm (tùy chọn)

- [02/concurrency.md](../02-modern-cpp/concurrency.md) — thread, mutex, atomic, memory order của repo.
- [03/sync-primitives.md](../03-operating-system/sync-primitives.md) — condvar, semaphore ở tầng OS.

## Cụm 8 — Tweaks (ch. 8, Items 41–42)

### Nội dung chính

**Item 41 — Cân nhắc pass-by-value cho tham số copyable, rẻ-để-move, và luôn-được-copy.** Ba cách viết hàm "nhận rồi giữ lại" một giá trị:

```cpp
class Widget {
    std::string name;
public:
    // 1) Overload đôi: hiệu quả nhất, nhưng 2 hàm phải maintain
    void addName(const std::string& n) { names.push_back(n); }          // lvalue: 1 copy
    void addName(std::string&& n)      { names.push_back(std::move(n)); } // rvalue: 1 move

    // 2) Universal reference: hiệu quả ngang (1), nhưng phải ở header,
    //    error message tệ, các bẫy Item 26-27, 30
    template<typename T> void addName(T&& n) { names.push_back(std::forward<T>(n)); }

    // 3) Pass by value + move: MỘT hàm, dễ đọc
    void addName(std::string n) { names.push_back(std::move(n)); }
    // lvalue vào: 1 copy (tạo param) + 1 move  — đắt hơn (1) đúng 1 move
    // rvalue vào: 1 move (tạo param) + 1 move  — đắt hơn (1) đúng 1 move
};
```

Kết luận của Meyers: phương án (3) đáng cân nhắc khi **đủ cả 3 điều kiện** — (a) kiểu **copyable** (move-only như `unique_ptr` thì overload rvalue là đủ rồi, by-value ép move-only vẫn ổn nhưng câu chuyện khác); (b) **move rẻ** (chi phí "+1 move" mới bỏ qua được — nhớ Item 29: `array`/SSO string không rẻ); (c) tham số **chắc chắn được copy/giữ lại** (hàm có nhánh return sớm không dùng param → by-value trả giá construct/destruct vô ích).

Hai lưu ý thêm: chuỗi hàm chuyển tiếp by-value **cộng dồn** move qua mỗi tầng; và **by-value bị slicing** với hierarchy — không dùng cho kiểu base class. Với gán (assignment) thay vì construct, phân tích chi phí phức tạp hơn (liên quan capacity có sẵn của chuỗi đích) — by-value có thể đắt hơn tưởng.

**Item 42 — Emplacement thay vì insertion — thường nhanh hơn, không phải luôn luôn.**

```cpp
std::vector<std::string> vs;
vs.push_back("xyzzy");
// "xyzzy" là const char[6] — KHÔNG phải string. push_back(const string&/string&&)
// → tạo string TEMPORARY từ literal → move vào vector → hủy temporary (2 ctor + 1 dtor)

vs.emplace_back("xyzzy");
// perfect-forward literal THẲNG vào constructor của string xây tại chỗ trong vector
// → 1 constructor, không temporary
```

Emplace nhận **arguments của constructor** (variadic + perfect forwarding), insert nhận **object của kiểu phần tử**. Emplace gần như chắc thắng khi đủ 3 điều kiện: (1) giá trị được **construct** vào container (không phải assign vào slot có sẵn — vd `v[k] = ...` hay vị trí giữa vector khi chèn); (2) kiểu argument **khác** kiểu phần tử (literal vs string — có temporary để tiết kiệm); (3) container **không từ chối trùng lặp** (set/map phải dựng node tạm để so sánh trước — lợi thế mất).

Hai bẫy phải biết:

1. **Resource-managing type — emplace mở khe leak:**

```cpp
std::list<std::shared_ptr<Widget>> ptrs;
ptrs.push_back({new Widget, killWidget});      // temporary shared_ptr tạo NGAY — an toàn
ptrs.emplace_back(new Widget, killWidget);     // ⚠️ raw pointer forward vào trong;
// nếu cấp phát node của list ném exception TRƯỚC khi shared_ptr được dựng
// → raw pointer mồ côi → LEAK. 
// Bài học gốc (Item 21): đừng để raw pointer "lơ lửng" — tạo smart pointer ở statement riêng.
```

2. **Explicit constructor — emplace gọi được thứ insert từ chối:**

```cpp
std::vector<std::regex> regexes;
regexes.push_back(nullptr);      // ❌ không compile — copy-init không dùng explicit ctor. Tốt!
regexes.emplace_back(nullptr);   // ⚠️ COMPILE — emplace là direct-init, gọi được
                                 //    regex(const char*) explicit → UB runtime!
```

Emplacement dùng **direct initialization** → vượt qua hàng rào `explicit` — thứ lẽ ra chặn bug ở compile time thành bug runtime.

### Insight đáng nhớ

- Cả hai item cùng dạy một kỹ năng meta: **đếm constructor/move/destructor như đếm tiền** — "nhanh hơn" trong C++ luôn quy về được số thao tác cụ thể trên kiểu cụ thể, không phải niềm tin. Trả lời interview theo kiểu đếm này (1 copy + 1 move vs 1 copy) luôn ăn điểm hơn "cách này nhanh hơn".
- `emplace` vs `push_back` là câu hỏi phỏng vấn phổ biến; người trả lời tròn trịa là người nêu được **cả hai bẫy** (leak với raw pointer, vượt rào explicit) chứ không chỉ "emplace đỡ một temporary".

### Góc interview

**Câu 1:** `emplace_back` khác `push_back` thế nào? Có khi nào `emplace_back` là lựa chọn tồi hơn?

<details><summary>Đáp án</summary>

- `push_back(x)`: nhận **object kiểu phần tử** — argument khác kiểu sẽ convert ra **temporary** trước, rồi copy/move temporary vào container. `emplace_back(args...)`: perfect-forward `args` vào **constructor chạy tại chỗ** trong bộ nhớ container — bỏ được temporary (tiết kiệm 1 ctor + 1 dtor khi kiểu argument ≠ kiểu phần tử, vd `const char*` vào `vector<string>`).
- Khi truyền sẵn đúng kiểu phần tử (lvalue/rvalue string vào `vector<string>`) → hai cách tương đương, cùng 1 copy/move.
- `emplace` tồi hơn khi:
  1. **Kiểu quản lý tài nguyên**: `emplace_back(new T, deleter)` — raw pointer bay vào trong, cấp phát node ném exception trước khi smart pointer kịp dựng → **leak**; `push_back` tạo temporary smart pointer ngay tại call site nên kín kẽ (và đúng nhất là tạo smart pointer ở statement riêng rồi move vào).
  2. **Constructor `explicit`**: emplace dùng direct-init nên gọi được explicit ctor mà copy-init (push_back) từ chối — `emplace_back(nullptr)` vào `vector<regex>` compile rồi UB, trong khi `push_back(nullptr)` chặn ngay từ compile. Emplace vô hiệu hóa một lớp bảo vệ kiểu.
  3. Container **kiểm tra trùng lặp** (set/map): thường phải dựng node/giá trị tạm để so sánh → lợi thế biến mất.

</details>

**Câu 2:** Khi thiết kế `void setName(??? name)` cho một class, bạn chọn `const std::string&`, `std::string&&` overload đôi, `template T&&`, hay `std::string` by-value? Trình bày trade-off.

<details><summary>Đáp án</summary>

Đếm chi phí cho trường hợp "nhận rồi giữ" (`name_ = ...`):

| Cách | lvalue vào | rvalue vào | Nhược điểm chính |
|---|---|---|---|
| `const string&` (một hàm) | 1 copy | 1 copy (mất cơ hội move) | Rvalue bị copy oan |
| Overload `const&` + `&&` | 1 copy | 1 move | ×2 hàm phải maintain; n tham số → 2ⁿ overload |
| `template T&&` (forward) | 1 copy | 1 move | Phải ở header; error message tệ; nuốt overload khác (Item 26); fail cases (Item 30) |
| **By-value + `std::move`** | 1 copy + 1 move | 2 move | Đắt hơn tối ưu đúng **1 move**; đòi kiểu copyable, move rẻ, param chắc chắn được giữ |

- Khuyến nghị thực dụng: string/vector (move rẻ) + hàm setter đơn giản luôn-lưu → **by-value** cho gọn một hàm; API hot path hoặc kiểu move đắt → overload đôi; thư viện generic thật sự cần tối đa hiệu năng và chấp nhận độ phức tạp → forwarding reference (kèm constraint).
- **Bẫy khi trả lời:** quên điều kiện "luôn được copy" — hàm có nhánh bỏ qua param thì by-value trả giá vô ích; và quên **slicing** — không by-value với kiểu thuộc hierarchy.

</details>

### Đọc thêm (tùy chọn)

- Cụm 5 (move/forward) — nền của toàn bộ phân tích chi phí ở đây.
