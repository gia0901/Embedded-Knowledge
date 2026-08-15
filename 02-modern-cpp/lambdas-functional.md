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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [CPP-058](../14-prep/mock-interview/bank/cpp.md) | Lambda là gì? Compiler biến nó thành cái gì? |
| [CPP-015](../14-prep/mock-interview/bank/cpp.md) | Các kiểu capture của lambda? Bẫy nguy hiểm nhất là gì? |
| [CPP-059](../14-prep/mock-interview/bank/cpp.md) | std::function khác lambda trực tiếp và function pointer thế nào? |
| [CPP-040](../14-prep/mock-interview/bank/cpp.md) | auto có copy hay giữ reference? Khi nào nên/không nên dùng? |
| [CPP-028](../14-prep/mock-interview/bank/cpp.md) | Khác nhau giữa enum và enum class? |

---
⬅️ [move-semantics.md](move-semantics.md) · ➡️ Tiếp theo: [concurrency.md](concurrency.md)
