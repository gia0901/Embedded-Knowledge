# API Design — Thiết kế C++ Library Interface

> **TL;DR**
> - API tốt: **dễ dùng đúng, khó dùng sai**; tối thiểu, nhất quán, che giấu chi tiết triển khai, ổn định theo thời gian.
> - **Pimpl idiom** (pointer to implementation): giấu data member sau một con trỏ → giảm thời gian biên dịch và **bảo vệ ABI** (thêm field không phá layout public).
> - **Biên giới C** (`extern "C"`, kiểu POD, opaque handle): cho ABI ổn định, gọi được từ ngôn ngữ khác, tương thích chéo compiler — đánh đổi sự tiện lợi của C++.
> - **Ownership rõ ràng**: ai cấp phát/giải phóng? Trả `unique_ptr`, nhận tham số bằng `const&`/`span`, tránh trả con trỏ thô mơ hồ.
> - **Error handling** nhất quán: exception (trong C++) hoặc mã lỗi/`expected` (ở biên giới C). Không trộn lẫn tùy tiện.

---

## 1. Nguyên tắc nền tảng

> "Make interfaces easy to use correctly and hard to use incorrectly." — Scott Meyers

- **Tối thiểu (minimal)**: chỉ phơi bày những gì người dùng *cần*. Mọi thứ public là cam kết phải maintain mãi.
- **Che giấu (information hiding)**: ẩn chi tiết triển khai → tự do thay đổi bên trong mà không ảnh hưởng người dùng. `private`, pimpl, opaque handle.
- **Nhất quán (consistent)**: quy ước đặt tên, thứ tự tham số, cách báo lỗi giống nhau xuyên suốt → người dùng đoán được.
- **Khó dùng sai**: dùng kiểu mạnh thay vì `bool`/`int` mơ hồ (vd `enum class Mode` thay `bool`), RAII để không quên dọn dẹp, `[[nodiscard]]` cho giá trị không nên bỏ.
- **Ổn định**: thiết kế để tiến hóa được mà không phá API/ABI (xem [abi-versioning.md](abi-versioning.md)).

---

## 2. Pimpl idiom — giấu triển khai

```cpp
// widget.h (public)  — KHÔNG lộ data member
class Widget {
public:
    Widget();
    ~Widget();                       // phải khai báo (định nghĩa ở .cpp vì Impl incomplete)
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;

    void doSomething();
private:
    struct Impl;                     // forward declaration
    std::unique_ptr<Impl> pImpl;     // con trỏ tới triển khai
};
```
```cpp
// widget.cpp (private)
struct Widget::Impl {
    int internalState;               // thêm/bớt field ở ĐÂY không phá ABI/header
    std::vector<int> cache;
};
Widget::Widget() : pImpl(std::make_unique<Impl>()) {}
Widget::~Widget() = default;          // ở đây Impl đã complete
void Widget::doSomething() { pImpl->internalState++; }
```

Lợi ích:
- **Bảo vệ ABI**: `sizeof(Widget)` luôn = một con trỏ; thêm field vào `Impl` không đổi layout public → không phá ABI.
- **Giảm compile time**: header không cần include chi tiết/dependency của Impl → ít rebuild lan truyền khi đổi internal.
- Đánh đổi: thêm một lần gấp gián tiếp (con trỏ) + cấp phát heap → cân nhắc cho hot path.

---

## 3. Biên giới C (`extern "C"`) — khi cần ABI bền & liên thông

C++ ABI không ổn định giữa compiler/phiên bản (mangling, layout, exception). Để có thư viện **gọi được từ C/Python/Rust** và **không vỡ khi đổi compiler**, phơi bày một **C API** mỏng:

```c
// public C header
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Engine Engine;            // opaque handle — giấu hoàn toàn nội dung

Engine*   engine_create(void);
int       engine_process(Engine*, const uint8_t* data, size_t len);  // trả mã lỗi
void      engine_destroy(Engine*);

#ifdef __cplusplus
}
#endif
```
```cpp
// implementation .cpp — bên trong vẫn là C++ đầy đủ
struct Engine { /* C++ members, dùng class/vector... thoải mái */ };
extern "C" Engine* engine_create() { return new Engine(); }
extern "C" void    engine_destroy(Engine* e) { delete e; }
```

Quy tắc biên giới C:
- Chỉ truyền **kiểu POD / con trỏ opaque**; không để class C++, `std::string`, exception **vượt biên** (UB chéo compiler).
- **Không cho exception thoát** qua biên giới C → bắt hết, chuyển thành mã lỗi.
- Cấp phát và giải phóng phải **cùng phía** thư viện (caller gọi `engine_destroy`, không tự `free`).

→ Đây là pattern phổ biến cho shared library hệ thống cần ổn định lâu dài và đa ngôn ngữ.

---

## 4. Ownership & lifetime — nói rõ ai sở hữu

Mơ hồ về sở hữu là nguồn leak/double-free và là điểm hay bị soi khi phỏng vấn.

```cpp
std::unique_ptr<Widget> createWidget();        // trả về → caller SỞ HỮU (rõ ràng)
void process(const Data& in);                  // chỉ đọc, KHÔNG sở hữu (const ref)
void consume(std::unique_ptr<Resource> r);     // nhận → hàm SỞ HỮU (move vào)
void observe(const Widget& w);                 // mượn, không giữ quá lời gọi
std::span<const int> view() const;             // tham chiếu vùng dữ liệu, không sở hữu (C++20)
```

Quy ước thực tế:
- Trả về object mới → **`unique_ptr`** (sở hữu rõ) hoặc theo trị giá (RVO).
- Tham số chỉ đọc lớn → **`const T&`**; chuỗi/buffer → `std::string_view`/`std::span` (không copy, nhưng *không giữ* quá lời gọi vì không sở hữu).
- Tránh trả **con trỏ thô** mà không nói rõ vòng đời; nếu phải, tài liệu hóa "ai free".
- Smart pointer ở **interface** chỉ khi thật cần truyền ownership; nếu chỉ dùng nội bộ thì không ép lộ ra API.

---

## 5. Error handling — chọn một cách và nhất quán

| Cách | Khi nào | Lưu ý |
|------|---------|-------|
| **Exception** | API thuần C++, lỗi hiếm/ngoại lệ | Dễ đọc happy-path; **không cho thoát qua biên giới C**; cân nhắc cấm trong hot path/embedded no-exceptions |
| **Mã lỗi (int/enum)** | Biên giới C, embedded | Tường minh, không overhead; dễ quên kiểm tra (`[[nodiscard]]` giúp) |
| **`std::expected<T,E>`** (C++23) / `optional` | Lỗi là kết quả bình thường | Bắt buộc xử lý, không overhead exception |

Nguyên tắc: **một phong cách nhất quán** trong cùng một API. Tài liệu hóa hàm nào ném gì / trả mã gì. Ở biên giới C luôn dùng mã lỗi và bắt mọi exception bên trong.

---

## 6. Một số guideline thực dụng khác

- **Đặt tên** rõ ràng, theo động từ cho hành động (`parse`, `connect`), nhất quán số nhiều/đơn.
- **Tham số**: ít, đúng thứ tự logic; nhóm tham số liên quan thành struct (`struct Options`) thay vì 6 tham số rời (tránh nhầm thứ tự).
- **`const`-correctness**: hàm không sửa trạng thái → `const`; con trỏ/ref chỉ đọc → `const T&`.
- **Tránh phơi bày kiểu của thư viện thứ ba** trong header công khai (kéo theo phụ thuộc + ràng buộc version).
- **Versioning & deprecation**: đánh dấu `[[deprecated]]` trước khi xóa; cung cấp đường nâng cấp.
- **Tài liệu hóa hợp đồng**: tiền điều kiện, hậu điều kiện, thread-safety, ownership — ngay tại header.
- **Thread-safety**: nói rõ object nào an toàn dùng đa luồng, hàm nào reentrant.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Nguyên tắc cốt lõi của một API tốt là gì?</summary>

API tốt nên "dễ dùng đúng và khó dùng sai": tối thiểu (chỉ phơi bày những gì cần, vì mọi thứ public là cam kết phải duy trì), che giấu chi tiết triển khai (để tự do thay đổi bên trong mà không ảnh hưởng người dùng), nhất quán (quy ước đặt tên, thứ tự tham số, cách báo lỗi đồng nhất để người dùng đoán được), dùng kiểu mạnh và RAII để ngăn lỗi (vd `enum class` thay `bool` mơ hồ, smart pointer để không quên dọn dẹp, `[[nodiscard]]`), và thiết kế để tiến hóa được mà không phá vỡ API/ABI. Tài liệu hóa rõ hợp đồng: tiền/hậu điều kiện, ownership, thread-safety.
</details>

<details><summary>2) Pimpl idiom là gì? Giải quyết vấn đề gì?</summary>

Pimpl (pointer to implementation) là kỹ thuật đưa toàn bộ data member và chi tiết triển khai của một class ra một struct `Impl` định nghĩa trong file `.cpp`, còn header công khai chỉ giữ một `std::unique_ptr<Impl>` và forward declaration. Nó giải quyết hai vấn đề: (1) **bảo vệ ABI** — `sizeof` của class public không đổi (luôn là một con trỏ), nên thêm/bớt field trong `Impl` không thay đổi layout mà client thấy, tránh ABI break; (2) **giảm thời gian biên dịch** — header không cần include các dependency của triển khai, nên thay đổi nội bộ không buộc rebuild lan truyền tới mọi nơi include header. Đánh đổi: một lần truy cập gián tiếp qua con trỏ và một cấp phát heap, cần cân nhắc ở hot path.
</details>

<details><summary>3) Vì sao shared library hệ thống thường phơi bày C API thay vì C++ trực tiếp?</summary>

Vì C++ ABI không ổn định giữa các compiler và thậm chí giữa các phiên bản của cùng compiler (khác biệt về name mangling, layout object/vtable, cách xử lý exception, cài đặt thư viện chuẩn). Một C API `extern "C"` với kiểu POD và opaque handle tạo ra ABI ổn định và xác định: gọi được từ C và nhiều ngôn ngữ khác (Python, Rust...), không vỡ khi client dùng compiler khác, và bề mặt nhỏ dễ giữ tương thích lâu dài. Bên trong thư viện vẫn cài đặt bằng C++ đầy đủ; chỉ lớp biên giới là C. Đổi lại mất đi sự tiện lợi và an toàn kiểu của C++ ở mặt tiếp xúc (phải quản lý lỗi bằng mã lỗi, ownership thủ công).
</details>

<details><summary>4) Những quy tắc nào cần tuân thủ ở biên giới C của một thư viện C++?</summary>

(1) Chỉ truyền kiểu POD hoặc con trỏ opaque qua biên giới — không để class C++, `std::string`, `std::vector` hay tham chiếu vượt biên vì layout/ABI của chúng không đảm bảo chéo compiler. (2) Không cho exception C++ thoát qua hàm `extern "C"` — phải bắt hết bên trong và chuyển thành mã lỗi (exception vượt biên C là UB). (3) Cấp phát và giải phóng tài nguyên phải cùng phía thư viện: cung cấp cặp create/destroy và caller phải gọi destroy của thư viện, không tự `free`/`delete`, vì allocator hai bên có thể khác nhau. (4) Giữ giao diện ổn định và versioning rõ ràng. Bên trong vẫn dùng C++ thoải mái.
</details>

<details><summary>5) Làm sao thể hiện ownership rõ ràng qua kiểu trong API C++?</summary>

Dùng kiểu để mã hóa ý định sở hữu: trả về `std::unique_ptr<T>` (hoặc theo trị giá) khi caller nhận quyền sở hữu object mới; nhận `std::unique_ptr<T>` theo trị (move-in) khi hàm tiếp quản sở hữu; nhận `const T&` khi chỉ đọc và không giữ object quá lời gọi; dùng `std::string_view`/`std::span` cho tham chiếu vùng dữ liệu không sở hữu (lưu ý không lưu lại quá thời gian sống của nguồn). Tránh trả con trỏ thô mơ hồ về vòng đời; nếu buộc phải, tài liệu hóa rõ ai chịu trách nhiệm giải phóng. Nguyên tắc là người đọc chữ ký hàm hiểu được ngay ai sở hữu cái gì, giảm leak và double-free.
</details>

<details><summary>6) Nên chọn exception hay mã lỗi cho thư viện? Vì sao phải nhất quán?</summary>

Tùy ngữ cảnh: exception phù hợp API thuần C++ với lỗi mang tính ngoại lệ/hiếm — giữ happy-path sạch, khó bỏ sót xử lý; nhưng không được để thoát qua biên giới C và có thể bị cấm trong môi trường embedded/no-exceptions hoặc hot path nhạy hiệu năng. Mã lỗi (int/enum) tường minh, không overhead, phù hợp biên giới C và embedded, nhưng dễ bị quên kiểm tra (giảm thiểu bằng `[[nodiscard]]`). `std::expected`/`optional` buộc xử lý kết quả mà không cần exception. Quan trọng là **nhất quán** trong cùng một API: trộn lẫn các phong cách khiến người dùng khó biết phải kiểm tra lỗi thế nào, dễ bỏ sót và gây bug; nên chọn một cách, tài liệu hóa, và áp dụng đồng đều.
</details>

---
⬅️ [abi-versioning.md](abi-versioning.md) · ➡️ Tiếp theo: [08-embedded-systems/](../08-embedded-systems/)
