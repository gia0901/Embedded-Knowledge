# Câu hỏi phỏng vấn — Design Patterns & SOLID

> Topic gốc: [12 Design Patterns](../12-design-patterns/). Tự trả lời trước khi mở đáp án.

---

## 🟢 Cơ bản

<details><summary>1) SOLID là gì?</summary>

5 nguyên lý thiết kế OOP: Single Responsibility (một class một trách nhiệm), Open/Closed (mở để mở rộng, đóng để sửa đổi), Liskov Substitution (lớp con thay được lớp cha về hành vi), Interface Segregation (interface nhỏ chuyên biệt), Dependency Inversion (phụ thuộc abstraction). Mục tiêu: giảm coupling, tăng cohesion → code dễ bảo trì/mở rộng/test. → [solid-principles](../12-design-patterns/solid-principles.md)
</details>

<details><summary>2) Singleton là gì? Cách hiện đại trong C++?</summary>

Đảm bảo một class chỉ có một instance + điểm truy cập toàn cục. C++11+ dùng Meyers' Singleton: `static` local trong hàm `instance()` — khởi tạo lazy, thread-safe theo chuẩn. → [creational](../12-design-patterns/creational.md)
</details>

<details><summary>3) Design pattern là gì và phân thành mấy nhóm?</summary>

Giải pháp tái sử dụng đã được kiểm chứng cho các vấn đề thiết kế lặp lại; cũng là từ vựng chung để trao đổi thiết kế. Ba nhóm: creational (tạo object — Singleton, Factory, Builder), structural (tổ chức object — Adapter, Bridge/Pimpl, Facade, Proxy), behavioral (tương tác/hành vi — Strategy, Observer, State, Command). → [README](../12-design-patterns/README.md)
</details>

---

## 🟡 Trung bình

<details><summary>4) Factory pattern giải quyết vấn đề gì? Liên hệ SOLID?</summary>

Tách quyết định tạo loại object nào khỏi code dùng: client làm việc qua interface chung, gọi factory để nhận object thay vì tự `new` class cụ thể. Hiện thực Dependency Inversion (phụ thuộc abstraction) + Open/Closed (thêm loại mới chỉ sửa factory, không sửa client). Hay dùng cho plugin. → [creational](../12-design-patterns/creational.md)
</details>

<details><summary>5) Strategy pattern là gì? C++ hiện đại hiện thực gọn thế nào?</summary>

Đóng gói các thuật toán/hành vi hoán đổi được sau interface chung, chọn/đổi lúc runtime mà không sửa code dùng (hiện thực OCP + DIP). C++ hiện đại với strategy đơn giản thường dùng `std::function` + lambda gọn hơn cả cây class; cần hiệu năng compile-time thì dùng template parameter. → [behavioral](../12-design-patterns/behavioral.md)
</details>

<details><summary>6) Observer pattern dùng khi nào? Rủi ro?</summary>

Khi một subject đổi trạng thái cần tự động thông báo nhiều observer quan tâm mà không gắn chặt — nền của event-driven/callback (phổ biến trong embedded: sự kiện sensor, GPIO, nút bấm). Rủi ro chính: lifetime/dangling — observer bị hủy mà chưa unsubscribe → subject gọi vào con trỏ chết (UB); dùng weak_ptr hoặc unsubscribe an toàn. → [behavioral](../12-design-patterns/behavioral.md)
</details>

<details><summary>7) Pimpl liên quan Bridge thế nào? Lợi ích?</summary>

Pimpl là ứng dụng cụ thể của Bridge: tách abstraction (class public) khỏi implementation (struct Impl trong .cpp) qua `unique_ptr<Impl>`. Lợi ích: bảo vệ ABI (sizeof không đổi, thêm field không phá layout public) và giảm compile time (header không lộ dependency). → [structural](../12-design-patterns/structural.md)
</details>

---

## 🟠 Khó

<details><summary>8) Cho ví dụ vi phạm Liskov Substitution Principle.</summary>

Square kế thừa Rectangle: Square override `setWidth` để đổi cả height (giữ vuông), phá vỡ kỳ vọng "đổi width không đổi height" của code viết cho Rectangle → đa hình cho kết quả sai. Square "is-a" Rectangle về dữ liệu nhưng không về hành vi. Bài học: kế thừa phải là is-a thực sự về hành vi; nghi ngờ thì dùng composition. → [solid-principles](../12-design-patterns/solid-principles.md)
</details>

<details><summary>9) Vì sao Singleton bị coi là anti-pattern khi lạm dụng?</summary>

Vì thực chất là global state trá hình: tạo coupling ẩn (mọi nơi truy cập được, khó lần dependency), khó test (không thay được bằng mock vì truy cập trực tiếp), và gây vấn đề thứ tự khởi tạo/hủy của static (static init order fiasco). Chỉ dùng khi thật sự bắt buộc một instance; cân nhắc dependency injection để dễ test. → [creational](../12-design-patterns/creational.md)
</details>

<details><summary>10) State pattern là gì? Vì sao embedded hay dùng enum + switch thay vì bản OOP?</summary>

State pattern cho object đổi hành vi khi trạng thái nội bộ đổi — hiện thực state machine (cốt lõi firmware/protocol). Embedded thường dùng enum + switch/bảng chuyển trạng thái vì tất định, không cấp phát động, không chi phí virtual, footprint biết trước, dễ review — hợp ràng buộc tài nguyên và yêu cầu tin cậy. Bản OOP (mỗi state một class) chỉ đáng dùng khi logic mỗi state đủ phức tạp. → [behavioral](../12-design-patterns/behavioral.md)
</details>

---

## 🔴 Senior

<details><summary>11) Dependency Inversion giúp testability thế nào? Liên hệ embedded HAL.</summary>

DIP yêu cầu module cấp cao phụ thuộc abstraction (interface) và inject implementation từ ngoài. Khi test, thay implementation thật (database, driver phần cứng) bằng mock/stub cùng interface → test logic độc lập, nhanh, không cần tài nguyên thật. Đây chính là nền của Hardware Abstraction Layer trong embedded: logic gọi qua interface nên test được trên host với phần cứng giả lập, còn build cùng code chạy ASan/TSan (vốn khó trên target). → [solid-principles](../12-design-patterns/solid-principles.md), [system-design](../10-thinking/system-design.md)
</details>

<details><summary>12) Khi nào KHÔNG nên dùng design pattern / áp SOLID?</summary>

Khi vấn đề đơn giản và code ổn định, không có nhu cầu thay đổi/mở rộng thực sự — nhồi pattern/abstraction là over-engineering (vi phạm YAGNI), làm code phức tạp và khó theo dõi hơn. Trong embedded còn phải tính chi phí runtime: mỗi interface/virtual thêm vtable + gọi gián tiếp, đáng kể trên hệ hạn chế/hot path. Pattern là công cụ phục vụ mục tiêu "dễ thay đổi an toàn"; nếu đoạn code không cần thay đổi thì đừng tốn công trừu tượng hóa. C++ hiện đại cũng thay nhiều pattern bằng tính năng ngôn ngữ (lambda/`std::function` cho Strategy/Command đơn giản, `std::variant`+`std::visit` cho Visitor). → [behavioral](../12-design-patterns/behavioral.md)
</details>

<details><summary>13) Thiết kế hệ thống plugin trong C++ dùng pattern nào?</summary>

Kết hợp Factory + (thường) Strategy/Bridge với cơ chế nạp động: định nghĩa một interface abstract cho plugin (vd `IPlugin` với các pure virtual); mỗi plugin là một shared library export một hàm factory `extern "C"` (để tránh name mangling) trả về con trỏ tới interface. Chương trình chính dùng `dlopen`/`dlsym` nạp `.so` lúc runtime, lấy factory, tạo object và gọi qua interface — không cần biết class cụ thể lúc build (Open/Closed: thêm plugin không sửa core). Cần chú ý: ownership rõ ràng (plugin tạo thì plugin hủy, hoặc trả unique_ptr), không để exception/kiểu C++ vượt biên giới nếu cần ổn định ABI, và quản lý vòng đời `.so` (không dlclose khi còn object sống). → [creational](../12-design-patterns/creational.md), [linking-loading](../07-shared-libraries/linking-loading.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
