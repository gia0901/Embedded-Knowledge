# Move Semantics — Di chuyển thay vì sao chép

> **TL;DR**
> - **lvalue**: có tên, có địa chỉ, tồn tại lâu. **rvalue**: tạm thời, sắp hết hạn (vd kết quả biểu thức, literal).
> - **Move** = "ăn cắp" tài nguyên (con trỏ heap, buffer) từ object nguồn thay vì copy sâu → nhanh. Nguồn còn lại ở trạng thái hợp lệ nhưng không xác định (thường rỗng).
> - `std::move` **không di chuyển gì cả** — nó chỉ là một cast sang rvalue reference (`T&&`), cho phép chọn move ctor/assignment.
> - **Perfect forwarding** (`std::forward` + `T&&` universal reference) giữ nguyên tính lvalue/rvalue khi truyền tham số tiếp.
> - Trình biên dịch thường tự loại bỏ copy/move (**copy elision / RVO**) — đừng `return std::move(local)`.

---

## 1. Vấn đề: copy đắt đỏ

```cpp
std::vector<int> makeBig() {
    std::vector<int> v(1'000'000);
    return v;             // trước C++11: có thể copy cả triệu phần tử
}
std::string s = a + b;    // a+b tạo string tạm → trước đây copy vào s
```

Trước C++11, gán/trả về object lớn có thể **deep copy** toàn bộ — lãng phí khi nguồn là tạm thời và sắp bị hủy. Move semantics cho phép **chuyển quyền sở hữu** tài nguyên nội bộ thay vì sao chép.

---

## 2. lvalue vs rvalue

```cpp
int x = 10;       // x là lvalue (có tên, có địa chỉ)
                  // 10 là rvalue (literal tạm thời)
int y = x + 1;    // (x+1) là rvalue; y là lvalue
&x;               // OK — lvalue có địa chỉ
// &(x+1);        // ❌ rvalue không có địa chỉ bền vững
```

- **lvalue**: định danh được, có thể lấy địa chỉ, sống qua biểu thức hiện tại.
- **rvalue**: giá trị tạm, không tên (hoặc sắp hết hạn) — ứng viên lý tưởng để "rút ruột".

**rvalue reference** `T&&`: bind được tới rvalue, là cơ chế cho phép move.

```cpp
void f(const std::string& s);   // nhận lvalue (và cả rvalue) — chỉ đọc
void f(std::string&& s);        // nhận rvalue — được phép "ăn cắp" tài nguyên của s
```

---

## 3. Move constructor & move assignment

```cpp
class Buffer {
    std::size_t size_ = 0;
    int* data_ = nullptr;
public:
    Buffer(std::size_t n) : size_(n), data_(new int[n]) {}
    ~Buffer() { delete[] data_; }

    // Copy: sao chép sâu (đắt)
    Buffer(const Buffer& o) : size_(o.size_), data_(new int[o.size_]) {
        std::copy(o.data_, o.data_ + size_, data_);
    }

    // Move: ăn cắp con trỏ (rẻ) — đánh dấu noexcept!
    Buffer(Buffer&& o) noexcept : size_(o.size_), data_(o.data_) {
        o.data_ = nullptr;        // để destructor của o không delete nhầm
        o.size_ = 0;
    }

    Buffer& operator=(Buffer&& o) noexcept {
        if (this != &o) {
            delete[] data_;       // dọn tài nguyên cũ
            data_ = o.data_; size_ = o.size_;
            o.data_ = nullptr; o.size_ = 0;
        }
        return *this;
    }
};
```

Move chỉ chép vài con trỏ/số → O(1), thay vì O(n) như copy.

> **`noexcept` rất quan trọng:** `std::vector` chỉ dùng move khi reallocate nếu move ctor là `noexcept` (nếu không, nó copy để giữ strong exception guarantee). Quên `noexcept` → mất hiệu năng âm thầm.

---

## 4. `std::move` — chỉ là một cast

```cpp
std::string a = "hello";
std::string b = std::move(a);   // chọn move ctor; a giờ ở trạng thái hợp lệ-nhưng-rỗng
// a vẫn dùng được nhưng KHÔNG nên giả định nội dung
```

`std::move(x)` tương đương `static_cast<T&&>(x)`. Nó **không** di chuyển gì — chỉ *cho phép* trình biên dịch chọn overload nhận `T&&` (move). Việc "di chuyển thật" nằm trong move ctor/assignment.

Sau khi bị move, object nguồn ở trạng thái **valid but unspecified** — có thể gán lại hoặc hủy, không nên đọc giá trị.

---

## 5. Universal reference & perfect forwarding

Trong template, `T&&` (khi `T` được suy ra) là **universal/forwarding reference** — bind được cả lvalue lẫn rvalue (reference collapsing).

```cpp
template <typename T>
void wrapper(T&& arg) {              // universal reference
    target(std::forward<T>(arg));    // giữ nguyên lvalue/rvalue gốc
}
```

- `std::forward<T>(arg)`: chuyển tiếp đúng "tính chất" của arg — nếu gốc là rvalue thì forward thành rvalue (cho move), lvalue thì giữ lvalue.
- Dùng `std::move` cho rvalue reference *thường*; `std::forward` cho *forwarding reference* trong template.
- Ứng dụng: `emplace_back`, `make_unique` — chuyển tham số tới constructor mà không copy thừa.

```cpp
std::vector<std::string> v;
v.emplace_back("hello");   // construct tại chỗ, không tạo temporary rồi move
```

---

## 6. Copy elision & RVO — đừng phá nó

```cpp
std::vector<int> make() {
    std::vector<int> v(100);
    return v;               // ✅ RVO: construct thẳng vào nơi nhận, không copy/move
}

std::vector<int> make_bad() {
    std::vector<int> v(100);
    return std::move(v);    // ❌ PHẢN tác dụng — chặn RVO, ép move (chậm hơn)
}
```

- **RVO** (Return Value Optimization) / **copy elision**: compiler xây object thẳng tại vị trí của caller → bỏ qua cả copy lẫn move. Từ C++17, một số trường hợp **bắt buộc** elision.
- **Quy tắc:** trả về biến local thì `return v;` — đừng bọc `std::move`. Bọc move lại ngăn RVO và thường chậm hơn.

---

## 7. Khi nào quan tâm move?

- Viết class quản lý tài nguyên thô (buffer, handle): nên cung cấp move ctor/assignment `noexcept` (Rule of 5) — hoặc tốt hơn, dùng member RAII để đạt Rule of 0.
- Truyền/trả về object lớn: tin tưởng RVO + move của thư viện chuẩn.
- Tham số nhận "rồi giữ lại": cân nhắc **pass-by-value rồi move** (`set(std::string s){ name_ = std::move(s); }`) — vừa nhận copy vừa nhận move hiệu quả.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) lvalue và rvalue khác nhau thế nào? rvalue reference là gì?</summary>

lvalue là biểu thức định danh được, có địa chỉ bền, sống qua biểu thức hiện tại (vd biến có tên). rvalue là giá trị tạm thời, không tên hoặc sắp hết hạn (vd literal, kết quả `a+b`, giá trị trả về theo trị). rvalue reference `T&&` là tham chiếu chỉ bind tới rvalue, cho phép một hàm/constructor "ăn cắp" tài nguyên nội bộ của object tạm đó (move) thay vì copy.
</details>

<details><summary>2) std::move thực sự làm gì?</summary>

Không di chuyển gì cả. `std::move(x)` chỉ là một `static_cast` biến `x` thành rvalue reference (`T&&`), nhờ đó trình biên dịch chọn được overload move ctor/assignment. Việc "di chuyển" thật (ăn cắp con trỏ/buffer) nằm trong định nghĩa move ctor/assignment. Sau khi bị move, object nguồn ở trạng thái valid-but-unspecified.
</details>

<details><summary>3) Move semantics tăng hiệu năng bằng cách nào?</summary>

Thay vì deep copy tài nguyên (vd cấp phát buffer mới rồi copy n phần tử — O(n)), move ctor chỉ chuyển quyền sở hữu bằng cách sao chép vài con trỏ/kích thước rồi đặt nguồn về null — O(1). Điều này đặc biệt hiệu quả khi nguồn là object tạm sắp bị hủy (rvalue), vì không cần giữ lại bản sao.
</details>

<details><summary>4) Vì sao move constructor nên là noexcept?</summary>

Các container chuẩn như `std::vector` chỉ dùng move khi reallocate **nếu** move ctor được đánh dấu `noexcept`; nếu không, chúng buộc phải dùng copy để bảo toàn strong exception guarantee (nếu move ném giữa chừng sẽ mất dữ liệu không khôi phục được). Vì vậy quên `noexcept` khiến `vector` âm thầm copy thay vì move → mất hiệu năng.
</details>

<details><summary>5) Perfect forwarding là gì? std::move khác std::forward ra sao?</summary>

Perfect forwarding là truyền tham số qua một lớp wrapper template mà **giữ nguyên** tính lvalue/rvalue (và const) của đối số gốc, để lớp đích chọn đúng copy/move. Đạt được bằng forwarding reference `T&&` (T suy ra) + `std::forward<T>(arg)`. `std::move` luôn ép thành rvalue (dùng cho rvalue reference thường, khi chắc chắn muốn move); `std::forward` chỉ ép thành rvalue *nếu* đối số gốc là rvalue (dùng trong template để forward).
</details>

<details><summary>6) RVO là gì? Vì sao không nên return std::move(local)?</summary>

RVO (Return Value Optimization)/copy elision là tối ưu trong đó compiler xây object trả về thẳng vào vị trí của caller, bỏ qua hoàn toàn copy lẫn move (C++17 bắt buộc trong một số trường hợp). Khi `return v;` (v là local), RVO áp dụng được. Nếu viết `return std::move(v)`, biểu thức trả về thành rvalue reference chứ không phải tên biến → chặn RVO và ép một lần move không cần thiết, thường chậm hơn. Quy tắc: trả về biến local thì cứ `return v;`.
</details>

---
⬅️ [raii-smart-pointers.md](raii-smart-pointers.md) · ➡️ Tiếp theo: [lambdas-functional.md](lambdas-functional.md)
