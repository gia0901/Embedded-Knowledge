# Câu hỏi phỏng vấn — C/C++ & Modern C++

> Topic gốc: [01 C++ Fundamentals](../01-cpp-fundamentals/), [02 Modern C++](../02-modern-cpp/). Tự trả lời trước khi mở đáp án.

---

## 🟢 Cơ bản

<details><summary>1) Khác nhau giữa con trỏ và tham chiếu?</summary>

Con trỏ là biến chứa địa chỉ: có thể null, đổi target, làm pointer arithmetic, có thể chưa khởi tạo. Tham chiếu là alias của một object đã tồn tại: không null, phải bind ngay, không reseat. Dùng reference cho tham số bắt buộc tồn tại; pointer khi có thể null/cần reseat/làm việc với C API. → [memory-model](../01-cpp-fundamentals/memory-model.md)
</details>

<details><summary>2) `struct` và `class` khác nhau gì trong C++?</summary>

Chỉ khác default access: `struct` mặc định public, `class` mặc định private (và default inheritance tương tự). Mọi tính năng khác giống nhau. → [oop](../01-cpp-fundamentals/oop.md)
</details>

<details><summary>3) `const` đặt ở các vị trí khác nhau của con trỏ nghĩa là gì?</summary>

`const int* p`: con trỏ tới int hằng (không sửa `*p`, đổi `p` được). `int* const p`: con trỏ hằng (sửa `*p`, không đổi `p`). `const int* const p`: cả hai hằng. Đọc từ phải sang trái. → [memory-model](../01-cpp-fundamentals/memory-model.md)
</details>

<details><summary>4) Stack và heap khác nhau thế nào?</summary>

Stack: tự động theo scope, rất nhanh, nhỏ, vòng đời theo `{}`. Heap: cấp phát thủ công (`new`/`malloc`), linh hoạt vòng đời/kích thước, chậm hơn, có thể fragmentation. Mặc định dùng stack; heap khi cần sống lâu hơn scope hoặc kích thước runtime. → [memory-model](../01-cpp-fundamentals/memory-model.md)
</details>

---

## 🟡 Trung bình

<details><summary>5) RAII là gì? Vì sao là nền tảng của C++?</summary>

Ràng buộc vòng đời tài nguyên (heap, file, lock) vào vòng đời object: constructor giành, destructor trả. Destructor luôn chạy khi ra scope kể cả khi có exception → giải phóng tự động, exception-safe, không cần `finally`. Nền cho smart pointer, lock_guard, container. → [raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md)
</details>

<details><summary>6) Đa hình runtime hoạt động thế nào (vtable/vptr)?</summary>

Class có hàm virtual sẽ có một vtable (mảng con trỏ tới phiên bản hàm đúng); mỗi object có vptr ẩn trỏ tới vtable của class nó. Gọi hàm virtual qua con trỏ/ref base: lấy vptr → tra vtable → gọi đúng hàm lớp thực (dynamic dispatch). Chi phí: 1 con trỏ/object + 1 lần gián tiếp. → [oop](../01-cpp-fundamentals/oop.md)
</details>

<details><summary>7) `unique_ptr`, `shared_ptr`, `weak_ptr` khác nhau và dùng khi nào?</summary>

`unique_ptr`: sở hữu độc quyền, chỉ move, zero overhead — mặc định. `shared_ptr`: sở hữu chia sẻ qua ref count atomic, tốn hơn — chỉ khi nhiều owner thật sự. `weak_ptr`: không sở hữu, phá circular reference / quan sát, dùng qua `lock()`. → [raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md)
</details>

<details><summary>8) `std::move` thực sự làm gì? Move semantics tăng hiệu năng ra sao?</summary>

`std::move` chỉ là cast sang rvalue reference, không di chuyển gì — nó cho phép chọn move ctor/assignment. Move "ăn cắp" tài nguyên (sao chép vài con trỏ, set nguồn về null) — O(1) thay vì deep copy O(n). → [move-semantics](../02-modern-cpp/move-semantics.md)
</details>

<details><summary>9) Template hoạt động lúc nào? Vì sao định nghĩa phải ở header?</summary>

Template được compiler instantiate thành code chuyên biệt cho từng kiểu lúc **biên dịch** (không chi phí runtime). Định nghĩa phải ở header vì compiler cần thấy toàn bộ định nghĩa tại điểm sử dụng để sinh code; tách vào .cpp → các TU khác chỉ có khai báo → lỗi linker. → [templates](../01-cpp-fundamentals/templates.md)
</details>

<details><summary>10) Vì sao destructor của base class nên là virtual?</summary>

Nếu xóa object con qua con trỏ base mà destructor base không virtual, chỉ `~Base()` chạy, `~Derived()` bị bỏ → leak tài nguyên lớp con và là UB. Base class đa hình phải có virtual destructor. → [oop](../01-cpp-fundamentals/oop.md)
</details>

---

## 🟠 Khó

<details><summary>11) Vì sao move constructor nên `noexcept`? Hậu quả nếu quên?</summary>

`std::vector` (và container khác) chỉ dùng move khi reallocate **nếu** move ctor là `noexcept`; nếu không, nó copy để giữ strong exception guarantee (move ném giữa chừng sẽ mất dữ liệu không khôi phục). Quên `noexcept` → container âm thầm copy thay vì move → mất hiệu năng mà không báo lỗi. → [move-semantics](../02-modern-cpp/move-semantics.md)
</details>

<details><summary>12) RVO là gì? Vì sao `return std::move(local)` là phản tác dụng?</summary>

RVO/copy elision: compiler xây object trả về thẳng vào vị trí caller, bỏ qua cả copy lẫn move (C++17 bắt buộc một số ca). `return v;` cho phép RVO. `return std::move(v)` biến biểu thức thành rvalue reference chứ không phải tên biến → chặn RVO và ép một lần move thừa, thường chậm hơn. → [move-semantics](../02-modern-cpp/move-semantics.md)
</details>

<details><summary>13) Perfect forwarding là gì? `std::move` khác `std::forward` thế nào?</summary>

Perfect forwarding truyền tham số qua wrapper template mà giữ nguyên tính lvalue/rvalue của đối số gốc, dùng forwarding reference `T&&` + `std::forward<T>`. `std::move` luôn ép rvalue (dùng cho rvalue ref thường); `std::forward` chỉ ép rvalue nếu gốc là rvalue (dùng trong template). → [move-semantics](../02-modern-cpp/move-semantics.md)
</details>

<details><summary>14) Bẫy nguy hiểm nhất của lambda capture là gì?</summary>

Capture by reference (`[&]`) một biến local rồi để lambda sống lâu hơn biến đó (lưu lại, chạy async) → dangling reference, UB. Lambda sống ngắn (truyền vào algorithm ngay) thì `[&]` ổn; lambda lưu lại/async nên capture by copy hoặc by move. → [lambdas-functional](../02-modern-cpp/lambdas-functional.md)
</details>

<details><summary>15) Object slicing là gì và khi nào xảy ra?</summary>

Khi gán/copy object lớp con vào object **trị giá** kiểu lớp cha, phần dữ liệu riêng lớp con bị cắt và đa hình mất (gọi hàm base). Xảy ra vì object base kích thước cố định. Tránh bằng dùng con trỏ/reference base. → [oop](../01-cpp-fundamentals/oop.md)
</details>

<details><summary>16) Khi nào dùng template, khi nào dùng đa hình virtual?</summary>

Template: tập kiểu biết lúc compile, cần hiệu năng (STL, container) — quyết định lúc compile, inline được, không vtable, nhưng code bloat. Virtual: cần mở rộng/đa hình runtime (plugin, danh sách object khác kiểu qua interface) — linh hoạt nhưng có chi phí gián tiếp. → [templates](../01-cpp-fundamentals/templates.md)
</details>

---

## 🔴 Senior

<details><summary>17) `std::atomic` đủ để đồng bộ, hay cần mutex? Khi nào dùng cái nào?</summary>

`atomic` cho thao tác trên một biến đơn (counter, flag, con trỏ) — không khóa, hiệu năng cao. Mutex cho critical section phức tạp: cập nhật nhiều biến liên quan phải nhất quán, hoặc thao tác phức hợp không biểu diễn được bằng một atomic op. Tăng counter → atomic; cập nhật cấu trúc dữ liệu (thêm node + cập nhật size) → mutex. → [concurrency](../02-modern-cpp/concurrency.md)
</details>

<details><summary>18) Memory order là gì? Mặc định nên dùng cái nào và vì sao thận trọng?</summary>

CPU/compiler reorder lệnh để tối ưu; trong đa luồng điều này khiến thread khác thấy thứ tự ghi khác kỳ vọng. Memory order ràng buộc thứ tự quanh atomic op: `seq_cst` (nhất quán toàn cục, dễ suy luận, chi phí cao), `acquire/release` (đồng bộ cặp đôi), `relaxed` (chỉ atomicity). Mặc định `seq_cst`; chỉ hạ xuống khi đã đo và thật sự hiểu — đây là vùng cực dễ sai. → [concurrency](../02-modern-cpp/concurrency.md)
</details>

<details><summary>19) Rule of 0/3/5 là gì? Vì sao Rule of 0 được khuyến nghị?</summary>

Liên quan 5 special member function. Rule of 3: cần tự viết 1 trong {destructor, copy ctor, copy assign} thì thường cần cả 3. Rule of 5: thêm move ctor + move assign. Rule of 0: thiết kế để không phải viết cái nào, bằng cách dùng member RAII (smart pointer, container) → compiler tự sinh đúng và an toàn, ít lỗi nhất. → [raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md)
</details>

<details><summary>20) Tình huống: một class có `std::unique_ptr` member nhưng compile báo lỗi khi đặt trong `std::vector`. Vì sao?</summary>

`unique_ptr` là move-only (không copy) → class chứa nó cũng thành move-only. `std::vector` khi reallocate cần move (hoặc copy). Nếu class không có move ctor `noexcept`, vector có thể không dùng được move an toàn (cần copy mà copy bị xóa) → lỗi. Sửa: định nghĩa move ctor/assignment `noexcept` (Rule of 5) hoặc `= default` chúng. Đây là giao điểm của move semantics + Rule of 5 + noexcept. → [move-semantics](../02-modern-cpp/move-semantics.md), [raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md)
</details>

<details><summary>21) `volatile` có dùng để đồng bộ biến giữa các thread được không?</summary>

Không. `volatile` chỉ cấm compiler tối ưu bỏ/cache/reorder truy cập — cần khi biến đổi bởi phần cứng/ISR. Nhưng nó **không** đảm bảo atomicity cho read-modify-write, cũng không đảm bảo memory ordering giữa các core. Đồng bộ đa luồng cần `std::atomic` hoặc mutex. Nhầm lẫn này rất phổ biến và là bug nghiêm trọng. → [concurrency](../02-modern-cpp/concurrency.md), [constraints](../08-embedded-systems/constraints.md)
</details>

<details><summary>22) Vì sao C++ ABI không ổn định giữa các compiler? Hệ quả khi thiết kế thư viện?</summary>

Vì C++ phơi bày nhiều chi tiết triển khai ra ABI: name mangling không chuẩn hóa, layout object/vtable, cách xử lý exception, cài đặt thư viện chuẩn — khác nhau giữa compiler/phiên bản. Hệ quả: thư viện cần ổn định/đa ngôn ngữ nên phơi bày C API (`extern "C"`, POD, opaque handle) ở biên giới và cài đặt C++ bên trong; tránh để class C++/exception vượt biên; dùng pimpl để bảo vệ ABI. → [api-design](../07-shared-libraries/api-design.md), [abi-versioning](../07-shared-libraries/abi-versioning.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
