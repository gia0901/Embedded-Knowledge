# DP — Design Patterns & SOLID

> Domain `DP`. Từ [11/design-patterns.md](../../../11-interview-questions/design-patterns.md). Track dùng: `design-patterns`, `cpp-system`.

---

#### DP-001 · 🟢 · concept · [→ solid-principles](../../../12-design-patterns/solid-principles.md)
**SOLID là gì?**
<details><summary>Đáp án</summary>

5 nguyên lý thiết kế OOP: Single Responsibility (một class một trách nhiệm), Open/Closed (mở để mở rộng, đóng để sửa đổi), Liskov Substitution (lớp con thay được lớp cha về hành vi), Interface Segregation (interface nhỏ chuyên biệt), Dependency Inversion (phụ thuộc abstraction). Mục tiêu: giảm coupling, tăng cohesion → code dễ bảo trì/mở rộng/test.
</details>

#### DP-002 · 🟢 · concept · [→ creational](../../../12-design-patterns/creational.md)
**Singleton là gì? Cách hiện đại trong C++?**
<details><summary>Đáp án</summary>

Đảm bảo một class chỉ có một instance + điểm truy cập toàn cục. C++11+ dùng Meyers' Singleton: `static` local trong hàm `instance()` — khởi tạo lazy, thread-safe theo chuẩn.
</details>

#### DP-003 · 🟢 · concept · [→ README](../../../12-design-patterns/README.md)
**Design pattern là gì và phân thành mấy nhóm?**
<details><summary>Đáp án</summary>

Giải pháp tái sử dụng đã được kiểm chứng cho các vấn đề thiết kế lặp lại; cũng là từ vựng chung để trao đổi thiết kế. Ba nhóm: creational (tạo object — Singleton, Factory, Builder), structural (tổ chức object — Adapter, Bridge/Pimpl, Facade, Proxy), behavioral (tương tác/hành vi — Strategy, Observer, State, Command).
</details>

#### DP-004 · 🟡 · concept · [→ creational](../../../12-design-patterns/creational.md)
**Factory pattern giải quyết vấn đề gì? Liên hệ SOLID?**
<details><summary>Đáp án</summary>

Tách quyết định tạo loại object nào khỏi code dùng: client làm việc qua interface chung, gọi factory để nhận object thay vì tự `new` class cụ thể. Hiện thực Dependency Inversion + Open/Closed (thêm loại mới chỉ sửa factory, không sửa client). Hay dùng cho plugin.
</details>

#### DP-005 · 🟡 · concept · [→ behavioral](../../../12-design-patterns/behavioral.md)
**Strategy pattern là gì? C++ hiện đại hiện thực gọn thế nào?**
<details><summary>Đáp án</summary>

Đóng gói các thuật toán/hành vi hoán đổi được sau interface chung, chọn/đổi lúc runtime mà không sửa code dùng (OCP + DIP). C++ hiện đại với strategy đơn giản thường dùng `std::function` + lambda gọn hơn cả cây class; cần hiệu năng compile-time thì dùng template parameter.
</details>

#### DP-006 · 🟡 · concept · [→ behavioral](../../../12-design-patterns/behavioral.md)
**Observer pattern dùng khi nào? Rủi ro?**
<details><summary>Đáp án</summary>

Khi một subject đổi trạng thái cần tự động thông báo nhiều observer quan tâm mà không gắn chặt — nền của event-driven/callback (phổ biến embedded: sự kiện sensor, GPIO, nút bấm). Rủi ro chính: lifetime/dangling — observer bị hủy mà chưa unsubscribe → subject gọi vào con trỏ chết (UB); dùng weak_ptr hoặc unsubscribe an toàn.
</details>

#### DP-007 · 🟡 · concept · [→ structural](../../../12-design-patterns/structural.md)
**Pimpl liên quan Bridge thế nào? Lợi ích?**
<details><summary>Đáp án</summary>

Pimpl là ứng dụng cụ thể của Bridge: tách abstraction (class public) khỏi implementation (struct Impl trong .cpp) qua `unique_ptr<Impl>`. Lợi ích: bảo vệ ABI (sizeof không đổi, thêm field không phá layout public) và giảm compile time (header không lộ dependency).
</details>

#### DP-008 · 🟠 · concept · [→ solid-principles](../../../12-design-patterns/solid-principles.md)
**Cho ví dụ vi phạm Liskov Substitution Principle.**
<details><summary>Đáp án</summary>

Square kế thừa Rectangle: Square override `setWidth` để đổi cả height (giữ vuông), phá vỡ kỳ vọng "đổi width không đổi height" của code viết cho Rectangle → đa hình cho kết quả sai. Square "is-a" Rectangle về dữ liệu nhưng không về hành vi. Bài học: kế thừa phải là is-a thực sự về hành vi; nghi ngờ thì dùng composition.
</details>

#### DP-009 · 🟠 · concept · [→ creational](../../../12-design-patterns/creational.md)
**Vì sao Singleton bị coi là anti-pattern khi lạm dụng?**
<details><summary>Đáp án</summary>

Vì thực chất là global state trá hình: tạo coupling ẩn (mọi nơi truy cập được, khó lần dependency), khó test (không thay được bằng mock vì truy cập trực tiếp), và gây vấn đề thứ tự khởi tạo/hủy của static (static init order fiasco). Chỉ dùng khi thật sự bắt buộc một instance; cân nhắc dependency injection để dễ test.
</details>

#### DP-010 · 🟠 · concept · [→ behavioral](../../../12-design-patterns/behavioral.md)
**State pattern là gì? Vì sao embedded hay dùng enum + switch thay vì bản OOP?**
<details><summary>Đáp án</summary>

State pattern cho object đổi hành vi khi trạng thái nội bộ đổi — hiện thực state machine (cốt lõi firmware/protocol). Embedded thường dùng enum + switch/bảng chuyển trạng thái vì tất định, không cấp phát động, không chi phí virtual, footprint biết trước, dễ review — hợp ràng buộc tài nguyên và tin cậy. Bản OOP (mỗi state một class) chỉ đáng dùng khi logic mỗi state đủ phức tạp.
</details>

#### DP-011 · 🔴 · concept · [→ solid-principles](../../../12-design-patterns/solid-principles.md), [system-design](../../../10-thinking/system-design.md)
**Dependency Inversion giúp testability thế nào? Liên hệ embedded HAL.**
<details><summary>Đáp án</summary>

DIP yêu cầu module cấp cao phụ thuộc abstraction (interface) và inject implementation từ ngoài. Khi test, thay implementation thật (database, driver phần cứng) bằng mock/stub cùng interface → test logic độc lập, nhanh, không cần tài nguyên thật. Đây chính là nền của Hardware Abstraction Layer trong embedded: logic gọi qua interface nên test được trên host với phần cứng giả lập, còn build cùng code chạy ASan/TSan.
</details>

#### DP-012 · 🔴 · concept · [→ behavioral](../../../12-design-patterns/behavioral.md)
**Khi nào KHÔNG nên dùng design pattern / áp SOLID?**
<details><summary>Đáp án</summary>

Khi vấn đề đơn giản và code ổn định, không có nhu cầu thay đổi/mở rộng thực sự — nhồi pattern/abstraction là over-engineering (vi phạm YAGNI). Trong embedded còn phải tính chi phí runtime: mỗi interface/virtual thêm vtable + gọi gián tiếp, đáng kể trên hot path. Pattern là công cụ phục vụ "dễ thay đổi an toàn"; đoạn code không cần thay đổi thì đừng tốn công trừu tượng. C++ hiện đại cũng thay nhiều pattern bằng tính năng ngôn ngữ (lambda/`std::function` cho Strategy/Command, `std::variant`+`std::visit` cho Visitor).
</details>

#### DP-013 · 🔴 · design · [→ creational](../../../12-design-patterns/creational.md), [linking-loading](../../../07-shared-libraries/linking-loading.md)
**Thiết kế hệ thống plugin trong C++ dùng pattern nào?**
<details><summary>Đáp án</summary>

Kết hợp Factory + (thường) Strategy/Bridge với cơ chế nạp động: định nghĩa interface abstract cho plugin (`IPlugin` với pure virtual); mỗi plugin là shared library export hàm factory `extern "C"` (tránh name mangling) trả về con trỏ tới interface. Chương trình chính dùng `dlopen`/`dlsym` nạp `.so` lúc runtime, lấy factory, gọi qua interface — không cần biết class cụ thể lúc build (Open/Closed). Chú ý: ownership rõ ràng (plugin tạo thì plugin hủy, hoặc trả unique_ptr), không để exception/kiểu C++ vượt biên nếu cần ổn định ABI, và quản lý vòng đời `.so` (không dlclose khi còn object sống).
</details>

---
⬅️ [Bank index](README.md)
