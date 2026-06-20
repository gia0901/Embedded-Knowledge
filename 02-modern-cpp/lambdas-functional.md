# Lambdas & Functional Features — Lambda, auto, và tiện ích hiện đại

> **TL;DR**
> - **Lambda** là một object hàm ẩn danh (closure) tạo ngay tại chỗ; capture biến từ scope xung quanh.
> - Capture: `[=]` copy, `[&]` reference, `[x]` copy x, `[&x]` ref x, `[this]` con trỏ this. **Cẩn thận `[&]` với lambda sống lâu hơn biến** → dangling.
> - **`std::function`**: type-erased wrapper gói mọi callable (lambda, function pointer, functor) — linh hoạt nhưng có overhead; lambda trực tiếp/template nhanh hơn.
> - **`auto`**: suy luận kiểu, giảm rườm rà; dùng đúng chỗ tăng đọc hiểu, lạm dụng thì che mất kiểu.
> - **Structured bindings** (C++17): `auto [a, b] = pair;` giải nén gọn gàng.

---

## 1. Lambda — bản chất

Lambda là cú pháp ngắn để tạo một **closure**: một object có `operator()`. Compiler sinh ra một class ẩn danh.

```cpp
auto add = [](int a, int b) { return a + b; };   // closure object
add(2, 3);   // 5

// Tương đương khái niệm:
struct __lambda { int operator()(int a, int b) const { return a + b; } };
```

Cú pháp đầy đủ:
```cpp
[capture](params) mutable -> ReturnType { body }
//  ▲        ▲       ▲          ▲
//  |        |       |          └ kiểu trả về (thường suy luận, bỏ được)
//  |        |       └ cho phép sửa biến capture-by-copy
//  |        └ tham số
//  └ danh sách capture
```

---

## 2. Capture — phần dễ sai nhất

```cpp
int x = 1, y = 2;

auto byCopy = [x]()      { return x; };       // chụp bản sao x tại thời điểm tạo
auto byRef  = [&y]()     { y++; };            // tham chiếu tới y thật
auto all_c  = [=]()      { return x + y; };   // copy mọi biến dùng tới
auto all_r  = [&]()      { x++; y++; };       // reference mọi biến dùng tới
auto mixed  = [=, &y]()  { return x + y; };   // copy mặc định, riêng y theo ref
auto init   = [z = x+1](){ return z; };       // init capture (C++14): tạo biến mới
auto mv     = [p = std::move(ptr)]() {...};   // capture-by-move
```

> ⚠️ **Bẫy dangling:** `[&]` capture theo reference. Nếu lambda được lưu lại và gọi **sau khi** biến gốc đã hết scope → đọc rác (UB).
> ```cpp
> std::function<int()> makeCounter() {
>     int count = 0;
>     return [&]() { return ++count; };   // ❌ count chết khi hàm return → dangling
>     // sửa: return [count]() mutable { return ++count; };  (capture by copy)
> }
> ```
> Quy tắc: lambda sống ngắn (truyền vào algorithm ngay) → `[&]` tiện và rẻ. Lambda lưu lại/async → ưu tiên capture by copy hoặc by move.

- `mutable`: cho phép sửa biến capture-by-copy bên trong (bản sao riêng, không ảnh hưởng gốc).
- `[this]` capture con trỏ this (truy cập member) — cẩn thận lifetime của object.

---

## 3. Lambda với STL algorithms — ứng dụng phổ biến nhất

```cpp
std::vector<int> v{5, 2, 8, 1, 9};

std::sort(v.begin(), v.end(), [](int a, int b){ return a > b; });   // giảm dần
auto it = std::find_if(v.begin(), v.end(), [](int n){ return n > 4; });
int cnt = std::count_if(v.begin(), v.end(), [](int n){ return n % 2 == 0; });
std::for_each(v.begin(), v.end(), [](int& n){ n *= 2; });
```

Đây là lý do lambda ra đời: truyền hành vi cho thuật toán ngay tại chỗ, không cần định nghĩa functor riêng.

---

## 4. `std::function` vs lambda trực tiếp vs template

```cpp
// 1) Template — biết kiểu callable lúc compile, inline được, nhanh nhất
template <typename F>
void apply(F f) { f(); }

// 2) std::function — type erasure, lưu được callable bất kỳ cùng chữ ký
std::function<void()> cb = [](){ std::cout << "hi"; };
cb = someFunctionPtr;   // gán lại bằng callable khác cùng signature

// 3) Function pointer — chỉ nhận hàm tự do / lambda KHÔNG capture
void (*fp)() = [](){};   // OK vì lambda không capture
```

| | Function pointer | `std::function` | Template `F` |
|--|-----------------|-----------------|--------------|
| Nhận lambda có capture? | Không | Có | Có |
| Overhead | Thấp | Có (heap alloc tiềm năng, gọi gián tiếp) | Zero (inline) |
| Lưu trong member/container? | Có | Có | Khó (kiểu khác nhau) |
| Khi dùng | C API/callback đơn giản | Cần lưu callable đa dạng | Hot path, generic |

**Lựa chọn:** hot path/generic → template. Cần lưu trữ/đồng nhất kiểu (callback list, command) → `std::function`. Liên thông C → function pointer.

---

## 5. `auto` — suy luận kiểu

```cpp
auto i = 42;                       // int
auto& r = someVector;              // reference, tránh copy
const auto& cr = getBigObject();   // const ref, không copy
auto it = m.begin();               // khỏi gõ std::map<...>::iterator dài dòng
auto lambda = [](int x){ return x; };  // kiểu lambda không gõ tay được
```

- **Nên dùng** khi: kiểu dài/rườm rà (iterator), kiểu không gõ được (lambda), trong template generic, hoặc khi tên kiểu không thêm thông tin.
- **Cẩn thận:** `auto` bỏ reference và const mặc định (`auto x = ref;` là copy). Muốn giữ thì `auto&`, `const auto&`.
- Đừng lạm dụng tới mức người đọc không biết kiểu gì — cân bằng giữa gọn và rõ.

---

## 6. Structured bindings (C++17)

```cpp
std::map<std::string, int> ages{{"an", 30}};

for (const auto& [name, age] : ages)      // giải nén cặp key/value
    std::cout << name << ": " << age;

auto [it, inserted] = ages.insert({"bình", 25});   // tách kết quả

std::tuple<int, std::string, double> t{1, "x", 2.5};
auto [id, label, value] = t;              // tách tuple
```

Gọn hơn nhiều so với `.first/.second` hay `std::get<>`.

---

## 7. Một số tiện ích modern khác (điểm danh nhanh)

- **`constexpr`**: tính toán lúc biên dịch — quan trọng cho embedded (giảm runtime, lookup table).
- **`nullptr`**: thay `NULL`/`0`, an toàn kiểu.
- **`enum class`**: enum có scope, không ngầm convert sang int → an toàn hơn `enum` cũ.
- **`std::optional`** (C++17): biểu diễn "có thể không có giá trị" thay cho con trỏ/sentinel.
- **`std::string_view`** (C++17): tham chiếu chuỗi không sở hữu, tránh copy — cẩn thận lifetime.
- **Range-based for**: `for (auto& x : container)`.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Lambda là gì? Compiler biến nó thành cái gì?</summary>

Lambda là cú pháp tạo một object hàm ẩn danh (closure) ngay tại chỗ. Compiler sinh ra một class ẩn danh có `operator()` (và các member tương ứng với biến được capture). Vì vậy lambda là một object có trạng thái, không phải con trỏ hàm — đó là lý do lambda có capture không gán được vào function pointer.
</details>

<details><summary>2) Các kiểu capture của lambda? Bẫy nguy hiểm nhất là gì?</summary>

`[=]` copy mọi biến dùng tới, `[&]` reference mọi biến, `[x]` copy x, `[&x]` ref x, `[this]` con trỏ this, `[z = expr]` init capture (C++14), capture-by-move `[p = std::move(x)]`. Bẫy nguy hiểm nhất là capture by reference (`[&]`) một biến local rồi để lambda sống lâu hơn biến đó (lưu lại, chạy async) → dangling reference, UB. Với lambda sống lâu nên capture by copy/move.
</details>

<details><summary>3) std::function khác lambda trực tiếp và function pointer thế nào?</summary>

Function pointer chỉ trỏ được tới hàm tự do hoặc lambda **không capture**, overhead thấp. `std::function` là wrapper type-erased lưu được **bất kỳ** callable cùng chữ ký (lambda có capture, functor, function pointer), linh hoạt nhưng có overhead (gọi gián tiếp, có thể cấp phát heap). Truyền lambda qua tham số template (`template<typename F>`) cho phép inline, zero overhead nhưng mỗi kiểu callable là một kiểu khác nhau (khó lưu đồng nhất). Chọn template cho hot path, `std::function` khi cần lưu trữ/đồng nhất kiểu.
</details>

<details><summary>4) auto có copy hay giữ reference? Khi nào nên/không nên dùng?</summary>

`auto` mặc định **suy ra kiểu giá trị, bỏ reference và top-level const** → `auto x = ref;` tạo bản copy. Muốn giữ tham chiếu phải viết `auto&` hoặc `const auto&`. Nên dùng `auto` khi kiểu dài/rườm rà (iterator), kiểu không gõ tay được (lambda), generic code, hoặc tên kiểu không thêm thông tin. Không nên khi nó che mất kiểu quan trọng khiến code khó hiểu, hoặc vô tình tạo copy đắt.
</details>

<details><summary>5) Khác nhau giữa enum và enum class?</summary>

`enum` cũ (unscoped): tên hằng đổ vào scope bao quanh (dễ đụng tên), ngầm chuyển đổi sang int. `enum class` (scoped, C++11): tên nằm trong scope của enum (`Color::Red`), không tự convert sang int (an toàn kiểu hơn, tránh so sánh nhầm giữa hai enum khác nhau), có thể chỉ định kiểu nền (`enum class E : uint8_t`). Nên ưu tiên `enum class`.
</details>

---
⬅️ [move-semantics.md](move-semantics.md) · ➡️ Tiếp theo: [concurrency.md](concurrency.md)
