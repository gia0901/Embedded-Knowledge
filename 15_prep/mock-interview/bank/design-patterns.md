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

Đảm bảo một class chỉ có một instance + điểm truy cập toàn cục. C++11+ dùng Meyers' Singleton: `static` local trong hàm `instance()` — khởi tạo **lazy** (chỉ dựng lần gọi đầu) và **thread-safe theo chuẩn** (compiler sinh guard variable, xem [DP-014](#dp-014--concept---creational)). Cấm copy (`= delete`), constructor private.
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

#### DP-014 · 🔴 · concept · ⭐ · [→ creational §Vì sao Meyers thread-safe](../../../12-design-patterns/creational.md)
**Vì sao Meyers' Singleton thread-safe? Double-checked locking tự viết trước C++11 sai ở đâu?**
<details><summary>Đáp án</summary>

**Vì sao thread-safe:** dòng `static T inst;` là local static khởi tạo động; chuẩn C++11 (§[stmt.dcl]) **bắt buộc** — nếu nhiều luồng cùng vào lần đầu, chỉ một luồng chạy khởi tạo, các luồng khác **chờ** tới khi xong. Compiler hiện thực bằng **guard variable** ẩn (Itanium ABI `__cxa_guard_acquire/release`): luồng đầu giành quyền chạy constructor một lần rồi release (set cờ atomic); luồng khác tới cùng lúc bị block tới khi release. Sau lần đầu, mỗi lần gọi chỉ là **một atomic load** (fast path, không khóa) → gần như free. Tên gọi: "magic statics".

**Vì sao DCLP cũ sai:** `inst = new T()` gồm 3 bước — cấp phát, chạy constructor, gán con trỏ. Trước C++11 **không có memory model**, compiler/CPU được sắp xếp lại thành gán-con-trỏ **trước khi** constructor xong → luồng khác thấy `inst != null` ở check-không-khóa và dùng ngay object **dựng dở** → UB. C++11 sửa gốc bằng memory model + `std::atomic` (giờ *có thể* viết DCLP đúng với acquire/release), nhưng đơn giản hơn là để magic statics lo → **đừng tự viết DCLP nữa**.

**Bẫy:** chỉ thread-safe phần *khởi tạo*, không phải phần *dùng* — method sửa trạng thái chung vẫn cần mutex riêng. (Embedded: `-fno-threadsafe-statics` tắt guard để bỏ chi phí atomic, nhưng mất luôn bảo đảm này.)
</details>

#### DP-015 · 🟠 · concept · 🏗️ · [→ creational §Object Pool](../../../12-design-patterns/creational.md), [constraints](../../../08-embedded-systems/constraints.md)
**Object Pool là gì? Vì sao hợp embedded? Điểm tinh tế khi hiện thực?**
<details><summary>Đáp án</summary>

Cấp phát sẵn một tập object cố định + danh sách slot rảnh; `acquire()` mượn một slot, `release()` trả — cả hai **O(1), không chạm heap**. Hợp embedded vì: footprint biết trước lúc biên dịch (mảng tĩnh), thời gian mượn/trả **tất định** (không đi qua allocator → tránh fragmentation và độ trễ bất định của malloc), khi cạn trả `nullptr` thay vì `bad_alloc`. Điểm tinh tế: thường dùng **placement new** để dựng object trên vùng nhớ có sẵn → phải **gọi destructor tường minh** (`p->~T()`) trong `release`, khác `delete` thường; bản production bọc con trỏ trong RAII handle (custom deleter tự gọi `release`) để khỏi quên trả, thêm mutex nếu đa luồng.
</details>

#### DP-016 · 🟡 · concept · [→ structural §Decorator](../../../12-design-patterns/structural.md)
**Decorator pattern là gì? Khác Proxy thế nào?**
<details><summary>Đáp án</summary>

Decorator thêm hành vi cho object **động, từng lớp** bằng cách bọc nó trong các decorator **cùng interface**, có thể **xếp chồng** nhiều lớp (vd file ← nén ← mã hóa) — tránh bùng nổ lớp con cho mọi tổ hợp tính năng (`EncryptedCompressedStream`...). Mỗi decorator vừa *là* interface đó vừa *giữ* một con trỏ tới object được bọc để ủy nhiệm. Khác Proxy: cả hai đều bọc và cùng interface, nhưng **Proxy kiểm soát *truy cập*** tới một object (lazy load, quyền, cache) — thường một lớp; **Decorator thêm *chức năng*** và thiết kế để **chồng nhiều lớp**. Trong C++ có thể thay bằng template/composition khi tập tính năng biết lúc compile.
</details>

---
⬅️ [Bank index](README.md)
