# Templates — Lập trình generic trong C++

> **TL;DR**
> - Template là **khuôn** để compiler sinh ra code cho từng kiểu cụ thể lúc biên dịch (**compile-time**), không có chi phí runtime.
> - `template <typename T>` cho hàm và class. Compiler **instantiate** (sinh code) khi bạn dùng với một kiểu cụ thể.
> - **Định nghĩa template phải nằm trong header** (vì instantiation cần thấy toàn bộ định nghĩa).
> - Lỗi template thường dài và khó đọc; C++20 **concepts** giúp ràng buộc kiểu và báo lỗi rõ ràng hơn.
> - Đây là nền tảng của STL (`vector<T>`, `map<K,V>`...) và generic programming.

---

## 1. Bản chất: vì sao cần template?

Bạn muốn viết một logic dùng được cho **nhiều kiểu** mà không lặp code, không mất an toàn kiểu (type safety), và không tốn chi phí runtime (khác với đa hình virtual). Giải pháp của C++ là template: bạn viết một lần, compiler sinh ra phiên bản chuyên biệt cho mỗi kiểu khi cần.

So sánh 3 cách "tổng quát hóa":

| Cách | Thời điểm | Type-safe | Chi phí runtime |
|------|-----------|-----------|-----------------|
| Macro `#define` | preprocessor | ❌ không | không (nhưng nguy hiểm, không hiểu kiểu) |
| `void*` (kiểu C) | runtime | ❌ không | có (cast, mất kiểm tra) |
| **Template** | **compile-time** | ✅ có | **không** |
| Virtual/đa hình | runtime | ✅ có | có (vtable) |

---

## 2. Function template

```cpp
template <typename T>
T maxOf(T a, T b) {
    return (a > b) ? a : b;
}

int    i = maxOf(3, 7);          // T = int  (suy ra tự động — template argument deduction)
double d = maxOf(2.5, 1.0);      // T = double
auto   s = maxOf<std::string>("a", "b");  // chỉ định tường minh
```

- Compiler **suy ra** `T` từ tham số (deduction); có thể chỉ định tường minh `maxOf<int>(...)`.
- Với mỗi kiểu khác nhau, compiler sinh một bản hàm riêng (**instantiation**).

---

## 3. Class template

```cpp
template <typename T>
class Stack {
    std::vector<T> data_;
public:
    void push(const T& v) { data_.push_back(v); }
    T pop() { T v = data_.back(); data_.pop_back(); return v; }
    bool empty() const { return data_.empty(); }
};

Stack<int> si;          // phải chỉ định kiểu (trước C++17)
Stack<std::string> ss;
```

> Từ C++17 có **CTAD** (class template argument deduction): `std::vector v{1,2,3};` tự suy ra `vector<int>`.

---

## 4. Non-type template parameter

Tham số template không nhất thiết là kiểu — có thể là **hằng số** (rất hữu ích cho embedded: mảng kích thước cố định, không cấp phát động).

```cpp
template <typename T, std::size_t N>
class Array {
    T data_[N];
public:
    constexpr std::size_t size() const { return N; }
    T& operator[](std::size_t i) { return data_[i]; }
};

Array<int, 8> a;   // mảng 8 phần tử, kích thước cố định lúc compile-time
```

Đây chính là ý tưởng của `std::array<T, N>`.

---

## 5. Template specialization (chuyên biệt hóa)

Cung cấp một bản cài đặt **riêng** cho một kiểu cụ thể khi logic tổng quát không phù hợp/cần tối ưu.

```cpp
template <typename T>
struct TypeName { static const char* get() { return "unknown"; } };

template <>                                   // full specialization cho bool
struct TypeName<bool> { static const char* get() { return "bool"; } };
```

- **Full specialization** (`template <>`): chỉ định toàn bộ tham số.
- **Partial specialization** (chỉ class template): chuyên biệt một phần, vd cho mọi con trỏ `T*`:
  ```cpp
  template <typename T>
  struct TypeName<T*> { static const char* get() { return "pointer"; } };
  ```

> `std::vector<bool>` là một specialization nổi tiếng (đóng gói bit) — và cũng là ví dụ kinh điển về specialization gây bất ngờ.

---

## 6. Vì sao template phải ở trong header?

Compiler chỉ sinh code cho một instantiation khi nó **thấy đầy đủ định nghĩa** template tại điểm sử dụng. Nếu tách định nghĩa vào `.cpp`, các translation unit khác chỉ thấy khai báo → lỗi **linker** (undefined reference).

→ **Quy ước:** đặt cả khai báo lẫn định nghĩa template trong header (`.h`/`.hpp`). (Có kỹ thuật *explicit instantiation* để tách, nhưng hiếm dùng và phải biết trước danh sách kiểu.)

Hệ quả: template làm tăng thời gian biên dịch và kích thước binary (code bloat — mỗi kiểu một bản code).

---

## 7. Variadic template (C++11) — số lượng tham số tùy ý

```cpp
template <typename... Args>            // parameter pack
void print(const Args&... args) {
    (std::cout << ... << args) << '\n';   // fold expression (C++17)
}
print(1, " hello ", 3.14);   // sinh code nhận đúng 3 tham số
```

Nền tảng cho `std::make_unique`, `emplace_back`, `printf` an toàn kiểu...

---

## 8. Concepts (C++20) — ràng buộc kiểu

Trước C++20, nếu dùng template với kiểu không hợp lệ, lỗi compile rất dài và khó hiểu. **Concepts** cho phép nêu rõ yêu cầu lên kiểu:

```cpp
#include <concepts>

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>          // chỉ chấp nhận kiểu số
T square(T x) { return x * x; }

square(5);        // OK
// square("hi"); // lỗi rõ ràng: "hi" không thỏa Numeric
```

Lợi ích: thông báo lỗi dễ đọc, interface tự tài liệu hóa, hỗ trợ overload theo ràng buộc.

---

## 9. Template vs Đa hình virtual — chọn cái nào?

| | Template (compile-time) | Virtual (runtime) |
|--|------------------------|-------------------|
| Thời điểm quyết định kiểu | Biên dịch | Chạy |
| Hiệu năng | Cao (inline được, không vtable) | Có chi phí gián tiếp |
| Kích thước binary | Có thể phình (code bloat) | Gọn hơn |
| Kiểu phải biết trước? | Có (lúc compile) | Không (mở rộng runtime, plugin) |
| Lỗi lộ ra khi | Biên dịch | Chạy |

**Chọn template** khi tập kiểu biết lúc biên dịch và cần hiệu năng (STL, container, thuật toán). **Chọn virtual** khi cần mở rộng/đa hình lúc runtime (plugin, xử lý danh sách object khác kiểu qua interface chung).

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Template là gì và hoạt động lúc nào? Có chi phí runtime không?</summary>

Template là khuôn để compiler sinh ra code chuyên biệt cho từng kiểu cụ thể tại **thời điểm biên dịch** (instantiation). Không có chi phí runtime như đa hình virtual: code sinh ra như thể bạn viết tay cho kiểu đó, có thể inline. Đánh đổi là tăng thời gian biên dịch và có thể phình kích thước binary (code bloat).
</details>

<details><summary>2) Vì sao định nghĩa template phải nằm trong header?</summary>

Vì compiler cần thấy toàn bộ định nghĩa template tại điểm sử dụng để instantiate code cho kiểu cụ thể. Nếu tách định nghĩa vào file `.cpp`, các translation unit khác chỉ có khai báo → không sinh được code → lỗi linker (undefined reference). Cách khác là explicit instantiation nhưng phải biết trước danh sách kiểu và ít dùng.
</details>

<details><summary>3) Phân biệt full specialization và partial specialization.</summary>

Full specialization (`template <>`) cung cấp bản cài đặt riêng cho một bộ tham số template **cụ thể hoàn toàn** (vd `Foo<int>`); áp dụng cho cả function và class template. Partial specialization chuyên biệt cho một **nhóm** kiểu theo mẫu (vd mọi con trỏ `Foo<T*>`, hoặc `Foo<T, int>`); **chỉ class template** mới hỗ trợ partial specialization, function template thì không (phải dùng overload).
</details>

<details><summary>4) Khi nào dùng template, khi nào dùng đa hình virtual?</summary>

Template khi tập kiểu biết lúc biên dịch và cần hiệu năng cao (STL, container, thuật toán generic) — quyết định lúc compile, inline được, không vtable. Virtual khi cần đa hình/mở rộng lúc runtime (plugin, danh sách object khác kiểu qua interface chung) — linh hoạt nhưng có chi phí gián tiếp. Hai cơ chế đôi khi kết hợp (type erasure, vd `std::function`).
</details>

<details><summary>5) Non-type template parameter là gì? Ví dụ ứng dụng embedded.</summary>

Là tham số template mang **giá trị hằng** thay vì kiểu, vd `template <typename T, std::size_t N>`. Hữu ích cho embedded vì cho phép định nghĩa cấu trúc kích thước cố định lúc biên dịch mà không cấp phát động — chính là `std::array<T, N>`, buffer kích thước cố định, ma trận cố định... Tránh được heap, tốt cho hệ thống hạn chế tài nguyên/realtime.
</details>

<details><summary>6) Concepts (C++20) giải quyết vấn đề gì?</summary>

Concepts cho phép đặt **ràng buộc** lên tham số template (vd "T phải là kiểu số", "T phải so sánh được"). Lợi ích: thông báo lỗi biên dịch rõ ràng, dễ đọc hơn (thay vì lỗi template lồng nhiều tầng), interface tự tài liệu hóa, và cho phép chọn overload theo ràng buộc thỏa mãn. Trước C++20 người ta phải dùng SFINAE/`enable_if` rườm rà để đạt mục tiêu tương tự.
</details>

---
⬅️ [oop.md](oop.md) · ➡️ Tiếp theo: [02-modern-cpp/](../02-modern-cpp/)
