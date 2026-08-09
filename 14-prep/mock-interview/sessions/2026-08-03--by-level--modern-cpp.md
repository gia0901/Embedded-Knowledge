# Phiên mock — 2026-08-03 · by-level 🟡 · track modern-cpp

- **Điểm trung bình:** 3.3 / 4
- **Bối cảnh:** Tuần 1 Buổi 2 của [datalogic-plan](../../study-plans/datalogic-plan.md) (C++17 — EMC items nền). 10 câu mức 🟡.

| ID | Câu (tóm tắt) | Tự đánh giá | Điểm | Ghi chú (thiếu gì / lỗ hổng) |
|----|---------------|-------------|------|------------------------------|
| CPP-009 | template ở header | ok | 3 | 🔼 nói compile-time ngay (B1=2); còn dùng "inline" hơi lệch (đúng là *instantiation*) |
| CPP-043 | nullptr vs 0/NULL | ok | 4 | bẫy overload + nullptr_t |
| CPP-044 | using vs typedef | ok | 3 | đúng alias template; thiếu ý phụ (_t chuẩn, đọc xuôi) |
| CPP-045 | `=delete` vs private cũ | lúng túng | 2 | **hiểu sai "compiler sinh code tối ưu"**; thiếu compile-vs-link + áp cho hàm bất kỳ |
| CPP-026 | tính năng C++17 | ok | 4 | optional/string_view/structured binding + lý do |
| CPP-037 | string_view | ok | 4 | non-owning + dangling UB |
| CPP-031 | const member / mutable | ok | 4 | logical constness (mutex/counter) |
| CPP-033 | new/delete vs malloc | ok | 3 | đúng ctor/dtor; thiếu bad_alloc vs NULL, không-trộn, new[]/delete[] |
| CPP-020 | Rule 0/3/5 | ok | 4 | retention — nhớ vững, đủ 3/5/0 + vì sao |
| CPP-032 | explicit | lúng túng | 2 | **ví dụ đầu sai (A b=a là copy)**; hiểu ra qua code (A a=4 + explicit → compile error); nhầm tên "default ctor" cho converting ctor |

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-045 — `= delete` vs private cũ (điểm 2)
- **Thiếu/sai:** hiểu sai "compiler sinh code tối ưu"; thiếu compile-vs-link + áp cho hàm bất kỳ.
- **Bank:** > "Cách cũ... private + không định nghĩa → dùng nhầm chỉ lỗi lúc **link**, mơ hồ. `= delete`: hàm **tồn tại nhưng bị xóa** → mọi lời gọi lỗi ngay lúc **compile**... Còn mạnh hơn: `delete` được cho **hàm bất kỳ** (không chỉ special members)."
- **Tài liệu:** > "Cách C++98 (khai báo private, không implement) chỉ chặn ở **link time** và chỉ với code ngoài class." ([EMC Item 11:444](../../../15-book-summaries/effective-modern-cpp.md#L444))
  > ```cpp
  > bool isLucky(char) = delete;   // chặn convert ngầm char → int
  > ```
- **Chốt:** (1) lỗi ở **compile** (không phải link); (2) dùng cho **hàm thường**, không chỉ copy ctor.

### CPP-032 — explicit (điểm 2)
- **Thiếu/sai:** ví dụ đầu sai (`A b = a` là **copy**, không phải conversion); nhầm tên "default ctor" cho converting ctor. Hiểu ra đúng qua code (`A a = 4` + `explicit` → compile error).
- **Bank:** > "`explicit` chặn **chuyển đổi ngầm** qua constructor một-đối-số (và conversion operator)... `Widget w = 42;` hoặc `f(42)` sẽ ngầm tạo `Widget(42)`. Quy tắc: ctor một đối số **mặc định để `explicit`** trừ khi thật sự muốn cho chuyển ngầm."
- **Tài liệu:** > `explicit Circle(double r) : r_(r) {}` — ctor 1-đối-số nên explicit ([oop.md](../../../01-cpp-fundamentals/oop.md))
- **Chốt:** ctor 1-đối-số **mặc định nên `explicit`**; không có nó, `int` tự biến thành object sau lưng bạn.

### Phụ — guaranteed copy elision (C++17, từ câu hỏi move ctor)
- `A b = A(4)` trong C++17 **không** gọi move ctor — prvalue khởi tạo `b` thẳng (elision bắt buộc), chỉ in ctor một lần. Muốn thấy move ctor: nguồn phải là *object đã tồn tại* (`A b = std::move(a)`) hoặc pre-C++17 / `-fno-elide-constructors`. Nối [move-semantics](../../../02-modern-cpp/move-semantics.md).

## Tổng kết
- **Điểm mạnh:** EMC nền (nullptr/C++17/string_view), const/mutable, Rule 0/3/5 rất chắc; template đã tiến bộ.
- **Lỗ hổng ưu tiên:**
  1. `=delete` — lỗi ở *compile* (không phải link) + áp cho hàm bất kỳ (CPP-045).
  2. `explicit` — implicit conversion qua ctor 1-đối-số; ctor 1-đối-số mặc định nên explicit (CPP-032).
  3. Phụ: guaranteed copy elision C++17 (`A b=A(4)` không gọi move ctor) — nối move-semantics.
- **Coding-arena:** `explicit_issue.cpp` (git-ignore) — dùng để minh hoạ implicit conversion; đã sửa đúng thành compile-error case.
- **Cập nhật weak-register:** CPP-009 2→3 (cần 1 lần ≥3 nữa); +CPP-045 (2); +CPP-032 (2).
- **Bank:** không phát sinh câu mới (cả 10 câu có sẵn).
- **Gợi ý buổi kế:** Tuần 1 Buổi 3 (`/mock daily track cpp-system` — concurrency + OS sync); xen `weak-review` để vá CPP-045/032/009.
