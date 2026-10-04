# 01 — C/C++ Fundamentals

Nền tảng ngôn ngữ C/C++ — phần được hỏi nhiều nhất trong phỏng vấn Embedded/System. Hiểu chắc topic này là điều kiện để học tốt Modern C++, OS và Debugging.

## 🗺️ Bức tranh tổng thể

> **Sợi chỉ đỏ:** Hiểu C++ nền tảng = trả lời ba câu hỏi về object: *sống ở đâu, tổ chức thế nào, tổng quát ra sao.*

```mermaid
flowchart LR
    A["<b>object SỐNG Ở ĐÂU</b><br/>memory-model<br/><i>stack/heap, con trỏ, lifetime</i>"]
    B["<b>object TỔ CHỨC THẾ NÀO</b><br/>oop<br/><i>class, kế thừa, virtual, vtable</i>"]
    C["<b>object TỔNG QUÁT RA SAO</b><br/>templates<br/><i>sinh code theo kiểu, compile-time</i>"]
    A -->|"nền của"| B -->|"tổng quát hoá"| C
    A -->|"C thuần: vẽ bộ nhớ"| P["<b>c-pointers-arrays</b><br/><i>decay, T**, mảng 2 chiều, chuỗi</i>"]
    P -->|"thứ chỉ C có"| I["<b>c-language-idioms</b><br/><i>macro, linkage, byte, goto cleanup</i>"]
```

- **Nhánh C thuần (`c-pointers-arrays` → `c-language-idioms`)** rẽ ra từ `memory-model`: cùng nền bộ nhớ, nhưng trả lời câu hỏi phỏng vấn **C trên giấy** chứ không đi tiếp lên OOP. Phần C riêng cho phần cứng (thanh ghi, bit, fixed-point) nằm ở [08/bare-metal-c](../08-embedded-systems/bare-metal-c.md).

- **`memory-model` là nền của tất cả:** `vtable`/`vptr` trong `oop` chỉ hiểu được khi đã nắm layout bộ nhớ; lỗi con trỏ/lifetime là gốc của hầu hết bug C++.
- **`oop` → `templates`:** template tổng quát hoá class/hàm theo kiểu; hiểu class trước thì hiểu class template dễ hơn.
- **Liên kết lên tầng trên:** memory model dẫn thẳng tới RAII & smart pointer ([02](../02-modern-cpp/)); vtable giải thích chi phí đa hình và vì sao C++ ABI nhạy cảm ([07](../07-shared-libraries/abi-versioning.md)).
- **Câu hỏi tổng hợp:** *"Khi gọi một hàm virtual qua con trỏ base, điều gì xảy ra ở mức bộ nhớ?"* — buộc nối `memory-model` + `oop`.

## Tài liệu trong topic

| # | File | Nội dung | Trạng thái |
|---|------|----------|-----------|
| 1 | [memory-model.md](memory-model.md) | Bộ nhớ chương trình: stack/heap/data/bss/text, con trỏ vs tham chiếu, lifetime, undefined behavior | ✅ |
| 2 | [c-pointers-arrays.md](c-pointers-arrays.md) | **C thuần:** số học con trỏ, array decay, đọc khai báo, `T **`, mảng 2 chiều, chuỗi, con trỏ hàm, `void *` — mọi output chạy thật | ✅ |
| 3 | [c-language-idioms.md](c-language-idioms.md) | **C thuần (tiếp):** preprocessor & macro, linkage, `union`/tuần tự hoá byte, `goto cleanup`, tự viết `atoi`/`itoa` | ✅ |
| 4 | [oop.md](oop.md) | class/struct, kế thừa, `virtual`, vtable/vptr, abstract class, đa hình | ✅ |
| 5 | [templates.md](templates.md) | function/class template, instantiation, specialization, generic programming | ✅ |

## Thứ tự đọc gợi ý
`memory-model` → `c-pointers-arrays` → `c-language-idioms` → `oop` → `templates`. Memory model là nền cho mọi thứ; hai file `c-*` là phần **C thuần** hay bị hỏi trên giấy (con trỏ 1–2 chiều, macro, linkage) — đọc ngay sau memory model, bỏ qua được nếu chỉ ôn C++; vtable trong OOP cần hiểu layout bộ nhớ; template hiểu rõ hơn khi đã nắm class.

## Liên kết
- Câu hỏi phỏng vấn: domain `C` trong [bank/c-programming.md](../14-prep/mock-interview/bank/c-programming.md) · domain `CPP` trong [bank/cpp.md](../14-prep/mock-interview/bank/cpp.md)
- Nối tiếp: [02-modern-cpp/](../02-modern-cpp/)
