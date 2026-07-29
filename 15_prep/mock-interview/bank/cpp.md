# CPP — C/C++ & Modern C++

> Domain `CPP`. Gộp từ [11/cpp.md](../../../11-interview-questions/cpp.md) + [technical_round/02 PHẦN 1](../../technical_round/02_question_bank.md). Metadata: xem [README.md](README.md). Track dùng: `cpp-system`, `modern-cpp`, `emc`, `cpp-concurrency`.

---

#### CPP-001 · 🟢 · concept · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Khác nhau giữa con trỏ và tham chiếu?**
<details><summary>Đáp án</summary>

Con trỏ là biến chứa địa chỉ: có thể null, đổi target, làm pointer arithmetic, có thể chưa khởi tạo. Tham chiếu là alias của một object đã tồn tại: không null, phải bind ngay, không reseat. Dùng reference cho tham số bắt buộc tồn tại; pointer khi có thể null/cần reseat/làm việc với C API.
</details>

#### CPP-002 · 🟢 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**`struct` và `class` khác nhau gì trong C++?**
<details><summary>Đáp án</summary>

Chỉ khác default access: `struct` mặc định public, `class` mặc định private (và default inheritance tương tự). Mọi tính năng khác giống nhau.
</details>

#### CPP-003 · 🟢 · concept · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**`const` đặt ở các vị trí khác nhau của con trỏ nghĩa là gì?**
<details><summary>Đáp án</summary>

`const int* p`: con trỏ tới int hằng (không sửa `*p`, đổi `p` được). `int* const p`: con trỏ hằng (sửa `*p`, không đổi `p`). `const int* const p`: cả hai hằng. Đọc từ phải sang trái.
</details>

#### CPP-004 · 🟢 · concept · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Stack và heap khác nhau thế nào?**
<details><summary>Đáp án</summary>

Stack: tự động theo scope, rất nhanh, nhỏ, vòng đời theo `{}`. Heap: cấp phát thủ công (`new`/`malloc`), linh hoạt vòng đời/kích thước, chậm hơn, có thể fragmentation. Mặc định dùng stack; heap khi cần sống lâu hơn scope hoặc kích thước runtime.
</details>

#### CPP-005 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**RAII là gì? Vì sao là nền tảng của C++?**
<details><summary>Đáp án</summary>

Ràng buộc vòng đời tài nguyên (heap, file, lock) vào vòng đời object: constructor giành, destructor trả. Destructor luôn chạy khi ra scope kể cả khi có exception → giải phóng tự động, exception-safe, không cần `finally`. Nền cho smart pointer, lock_guard, container.
</details>

#### CPP-006 · 🟡 · concept · ⭐ · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Đa hình runtime hoạt động thế nào (vtable/vptr)?**
<details><summary>Đáp án</summary>

Class có hàm virtual sẽ có một vtable (mảng con trỏ tới phiên bản hàm đúng); mỗi object có vptr ẩn trỏ tới vtable của class nó. Gọi hàm virtual qua con trỏ/ref base: lấy vptr → tra vtable → gọi đúng hàm lớp thực (dynamic dispatch). Chi phí: +1 con trỏ/object, +1 lần gián tiếp/lời gọi, không inline được.
</details>

#### CPP-007 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**`unique_ptr`, `shared_ptr`, `weak_ptr` khác nhau và dùng khi nào?**
<details><summary>Đáp án</summary>

`unique_ptr`: sở hữu độc quyền, chỉ move, zero overhead — mặc định cho mọi owning pointer. `shared_ptr`: sở hữu chia sẻ qua ref count atomic, tốn control block — chỉ khi nhiều owner thật sự. `weak_ptr`: không sở hữu, quan sát một `shared_ptr` và phá circular reference, dùng qua `.lock()`. Nguyên tắc: ưu tiên `unique_ptr`.
</details>

#### CPP-008 · 🟡 · concept · ⭐ · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**`std::move` thực sự làm gì? Move semantics tăng hiệu năng ra sao?**
<details><summary>Đáp án</summary>

`std::move` chỉ là `static_cast` một lvalue sang rvalue reference, **không di chuyển gì** — nó cho phép overload move ctor/assignment được chọn. Việc di chuyển thật (cướp con trỏ nội bộ, để nguồn rỗng) nằm trong move ctor: sao chép vài con trỏ thay vì deep copy → O(1) thay vì O(n).
</details>

#### CPP-009 · 🟡 · concept · [→ templates](../../../01-cpp-fundamentals/templates.md)
**Template hoạt động lúc nào? Vì sao định nghĩa phải ở header?**
<details><summary>Đáp án</summary>

Template được compiler instantiate thành code chuyên biệt cho từng kiểu lúc **biên dịch** (không chi phí runtime). Định nghĩa phải ở header vì compiler cần thấy toàn bộ định nghĩa tại điểm sử dụng để sinh code; tách vào .cpp → các TU khác chỉ có khai báo → lỗi linker.
</details>

#### CPP-010 · 🟡 · concept · ⭐ · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Vì sao destructor của base class nên là virtual?**
<details><summary>Đáp án</summary>

Nếu `delete` object con qua con trỏ base mà destructor base không virtual, chỉ `~Base()` chạy, `~Derived()` bị bỏ → leak tài nguyên lớp con và là UB. Base class đa hình phải có virtual destructor.
</details>

#### CPP-011 · 🟠 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Vì sao move constructor nên `noexcept`? Hậu quả nếu quên?**
<details><summary>Đáp án</summary>

`std::vector` (và container khác) chỉ dùng move khi reallocate **nếu** move ctor là `noexcept`; nếu không, nó copy để giữ strong exception guarantee (move ném giữa chừng sẽ mất dữ liệu không khôi phục). Quên `noexcept` → container âm thầm copy thay vì move → mất hiệu năng mà không báo lỗi.
</details>

#### CPP-012 · 🟠 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**RVO là gì? Vì sao `return std::move(local)` là phản tác dụng?**
<details><summary>Đáp án</summary>

RVO/copy elision: compiler xây object trả về thẳng vào vị trí caller, bỏ qua cả copy lẫn move (C++17 bắt buộc một số ca). `return v;` cho phép RVO. `return std::move(v)` biến biểu thức thành rvalue reference chứ không phải tên biến → chặn RVO và ép một lần move thừa, thường chậm hơn.
</details>

#### CPP-013 · 🟠 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Perfect forwarding là gì? `std::move` khác `std::forward` thế nào?**
<details><summary>Đáp án</summary>

Perfect forwarding truyền tham số qua wrapper template (factory, `emplace`…) mà **giữ nguyên** tính lvalue/rvalue của đối số gốc, dùng forwarding reference `T&&` + `std::forward<T>`. `std::move` luôn ép rvalue (dùng cho rvalue ref thường); `std::forward` chỉ ép rvalue nếu gốc là rvalue (dùng trong template). `Args&&... + std::forward<Args>(args)...` là mẫu chuẩn.
</details>

#### CPP-014 · 🟠 · concept · ⭐ · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Phân biệt rvalue reference và universal (forwarding) reference?**
<details><summary>Đáp án</summary>

`Widget&&` với kiểu cụ thể (không suy luận) = rvalue ref thật → dùng `std::move`. `template<class T> f(T&& x)` với `T` *được suy luận* = universal ref, nhận cả lvalue lẫn rvalue (reference collapsing) → dùng `std::forward<T>(x)` để giữ nguyên value category. Đây là nền của perfect forwarding.
</details>

#### CPP-015 · 🟠 · concept · [→ lambdas-functional](../../../02-modern-cpp/lambdas-functional.md)
**Bẫy nguy hiểm nhất của lambda capture là gì?**
<details><summary>Đáp án</summary>

Capture by reference (`[&]`) một biến local rồi để lambda sống lâu hơn biến đó (lưu lại, chạy async) → dangling reference, UB. Lambda sống ngắn (truyền vào algorithm ngay) thì `[&]` ổn; lambda lưu lại/async nên capture by copy hoặc by move.
</details>

#### CPP-016 · 🟠 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Object slicing là gì và khi nào xảy ra?**
<details><summary>Đáp án</summary>

Khi gán/copy object lớp con vào object **trị giá** kiểu lớp cha, phần dữ liệu riêng lớp con bị cắt và đa hình mất (gọi hàm base). Xảy ra vì object base kích thước cố định. Tránh bằng dùng con trỏ/reference base.
</details>

#### CPP-017 · 🟠 · concept · [→ templates](../../../01-cpp-fundamentals/templates.md)
**Khi nào dùng template, khi nào dùng đa hình virtual?**
<details><summary>Đáp án</summary>

Template: tập kiểu biết lúc compile, cần hiệu năng (STL, container) — quyết định lúc compile, inline được, không vtable, nhưng code bloat. Virtual: cần mở rộng/đa hình runtime (plugin, danh sách object khác kiểu qua interface) — linh hoạt nhưng có chi phí gián tiếp.
</details>

#### CPP-018 · 🔴 · concept · [→ concurrency](../../../02-modern-cpp/concurrency.md)
**`std::atomic` đủ để đồng bộ, hay cần mutex? Khi nào dùng cái nào?**
<details><summary>Đáp án</summary>

`atomic` cho thao tác trên một biến đơn (counter, flag, con trỏ) — không khóa, hiệu năng cao. Mutex cho critical section phức tạp: cập nhật nhiều biến liên quan phải nhất quán, hoặc thao tác phức hợp không biểu diễn được bằng một atomic op. Tăng counter → atomic; cập nhật cấu trúc dữ liệu (thêm node + cập nhật size) → mutex.
</details>

#### CPP-019 · 🔴 · concept · [→ concurrency](../../../02-modern-cpp/concurrency.md)
**Memory order là gì? Mặc định nên dùng cái nào và vì sao thận trọng?**
<details><summary>Đáp án</summary>

CPU/compiler reorder lệnh để tối ưu; trong đa luồng điều này khiến thread khác thấy thứ tự ghi khác kỳ vọng. Memory order ràng buộc thứ tự quanh atomic op: `seq_cst` (nhất quán toàn cục, dễ suy luận, chi phí cao), `acquire/release` (đồng bộ cặp đôi), `relaxed` (chỉ atomicity). Mặc định `seq_cst`; chỉ hạ xuống khi đã đo và thật sự hiểu — vùng cực dễ sai.
</details>

#### CPP-020 · 🟡 · concept · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**Rule of 0/3/5 là gì? Vì sao Rule of 0 được khuyến nghị?**
<details><summary>Đáp án</summary>

Liên quan 5 special member function. Rule of 3: cần tự viết 1 trong {destructor, copy ctor, copy assign} thì thường cần cả 3. Rule of 5: thêm move ctor + move assign. Rule of 0: thiết kế để không phải viết cái nào, bằng cách dùng member RAII (smart pointer, container) → compiler tự sinh đúng và an toàn, ít lỗi nhất.
</details>

#### CPP-021 · 🔴 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Một class có `std::unique_ptr` member nhưng compile báo lỗi khi đặt trong `std::vector`. Vì sao?**
<details><summary>Đáp án</summary>

`unique_ptr` là move-only (không copy) → class chứa nó cũng thành move-only. `std::vector` khi reallocate cần move (hoặc copy). Nếu class không có move ctor `noexcept`, vector có thể không dùng được move an toàn (cần copy mà copy bị xóa) → lỗi. Sửa: định nghĩa move ctor/assignment `noexcept` (Rule of 5) hoặc `= default` chúng. Đây là giao điểm move semantics + Rule of 5 + noexcept.
</details>

#### CPP-022 · 🟠 · concept · ⭐ · [→ concurrency](../../../02-modern-cpp/concurrency.md), [constraints](../../../08-embedded-systems/constraints.md)
**`volatile` dùng khi nào? Có giúp thread-safe / đồng bộ biến giữa các thread không?**
<details><summary>Đáp án</summary>

Dùng khi giá trị đổi ngoài luồng nhìn thấy của compiler — đọc **thanh ghi phần cứng (MMIO)** hoặc biến bị **ISR** sửa; nó cấm compiler cache/tối ưu/reorder truy cập. **Không** đảm bảo atomicity cho read-modify-write, cũng không đảm bảo memory ordering giữa các core → **không** thread-safe. Đồng bộ đa luồng cần `std::atomic` hoặc mutex. Nhầm lẫn này rất phổ biến và là bug nghiêm trọng.
</details>

#### CPP-023 · 🔴 · concept · [→ api-design](../../../07-shared-libraries/api-design.md), [abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Vì sao C++ ABI không ổn định giữa các compiler? Hệ quả khi thiết kế thư viện?**
<details><summary>Đáp án</summary>

Vì C++ phơi bày nhiều chi tiết triển khai ra ABI: name mangling không chuẩn hóa, layout object/vtable, cách xử lý exception, cài đặt thư viện chuẩn — khác nhau giữa compiler/phiên bản. Hệ quả: thư viện cần ổn định/đa ngôn ngữ nên phơi bày C API (`extern "C"`, POD, opaque handle) ở biên giới và cài đặt C++ bên trong; tránh để class C++/exception vượt biên; dùng pimpl bảo vệ ABI.
</details>

#### CPP-024 · 🟡 · concept · ⭐ · [→ concurrency](../../../02-modern-cpp/concurrency.md)
**`shared_ptr` có thread-safe không?**
<details><summary>Đáp án</summary>

Bộ đếm tham chiếu là atomic nên copy/destroy `shared_ptr` từ nhiều thread an toàn. Nhưng **object được trỏ tới không được bảo vệ** — ghi đồng thời vào object vẫn cần mutex. "Control block thread-safe, payload thì không."
</details>

#### CPP-025 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**Circular reference là gì, gây hậu quả gì, fix ra sao?**
<details><summary>Đáp án</summary>

Hai object giữ `shared_ptr` lẫn nhau → strong_count không bao giờ về 0 → **memory leak**. Fix: chiều "tham chiếu ngược" dùng `weak_ptr`, muốn truy cập thì `.lock()`.
</details>

#### CPP-026 · 🟡 · concept · [→ 02-modern-cpp](../../../02-modern-cpp/)
**Nêu vài kỹ thuật C++17 bạn hay dùng.**
<details><summary>Đáp án</summary>

`std::optional` (giá trị có thể vắng, thay sentinel), `std::variant` + `std::visit` (union an toàn kiểu), `std::string_view` (cửa sổ chỉ-đọc, không copy — hợp embedded), structured bindings (`auto [a,b]=...`), `if constexpr` (rẽ nhánh compile-time trong template).
</details>

#### CPP-027 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**RAII giúp exception safety thế nào?**
<details><summary>Đáp án</summary>

Khi exception ném, **stack unwinding** gọi destructor của mọi object đã khởi tạo trên đường thoát → tài nguyên (mutex, file, memory) tự nhả, không leak dù thoát hàm theo đường bất thường. Đây là lý do C++ không cần `finally`.
</details>

---
⬅️ [Bank index](README.md)
