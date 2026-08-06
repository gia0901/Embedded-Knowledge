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

**Resource Acquisition Is Initialization** — ràng buộc vòng đời **tài nguyên** (heap, file descriptor, mutex, socket, handle…) vào vòng đời một **object**: constructor **giành**, destructor **trả**.

**Vì sao nó là *nền tảng*, không chỉ là một tiện ích** — hai tầng lý do:
1. *(nông)* Không phải nhớ giải phóng thủ công → hết `delete` sót, hết quên `close()`/`unlock()`.
2. *(sâu — mới là điểm chính)* Destructor được ngôn ngữ **bảo đảm** chạy khi object ra scope, **kể cả khi thoát bằng exception** (stack unwinding). Nhờ đó C++ **không cần `finally`**: mọi đường thoát khỏi hàm — `return` giữa chừng, `break`, exception — đều dọn dẹp đúng, mà không viết thêm dòng nào. Xem [CPP-027](cpp.md).

```cpp
// ❌ Thủ công: mỗi đường thoát là một chỗ để quên
void f() {
    m.lock();
    if (early) return;      // 💥 quên unlock -> deadlock
    risky();                // 💥 exception -> quên unlock
    m.unlock();
}

// ✅ RAII: một dòng, đúng ở MỌI đường thoát
void f() {
    std::lock_guard<std::mutex> lk(m);   // ctor giành
    if (early) return;                   // ✅ dtor nhả
    risky();                             // ✅ exception -> dtor vẫn nhả
}                                        // ✅ dtor nhả
```

**Toàn bộ thư viện chuẩn xây trên nó:** `unique_ptr`/`shared_ptr` (memory), `lock_guard`/`scoped_lock` (mutex), `fstream` (file), `vector`/`string` (buffer). Và [Rule of 0](cpp.md) là hệ quả trực tiếp: dùng member RAII thì **không phải tự viết** destructor/copy/move nào cả.

**Chốt:** *"Tài nguyên có chủ là một object; hết scope là trả. Đó là lý do C++ không cần `finally` và không cần GC."*
</details>

#### CPP-006 · 🟡 · concept · ⭐ · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Đa hình runtime hoạt động thế nào (vtable/vptr)?**
<details><summary>Đáp án</summary>

**Cơ chế — hai thứ, đừng lẫn:**
- **vtable**: **một bảng cho mỗi *class*** (nằm ở vùng read-only của binary), là mảng con trỏ tới phiên bản hàm virtual đúng của class đó.
- **vptr**: **một con trỏ ẩn trong mỗi *object***, do constructor gán, trỏ tới vtable của class thật của object.

```
Shape* p = new Circle();      p ──► [ vptr ] ──► vtable của Circle
p->area();                                       ├ [0] Circle::area
// compiler sinh: (*p->vptr[0])(p)               └ [1] Circle::~Circle
```

Lời gọi `p->area()` **không** biết class thật lúc compile — nó **tra bảng lúc chạy** (dynamic dispatch): lấy vptr từ object → nhảy tới ô tương ứng → gọi. Chỉ số ô là cố định lúc compile, nên chi phí là **hằng số**, không phải tìm kiếm.

**Chi phí — hay bị hỏi tiếp, đặc biệt ở vị trí embedded:**

| Loại | Chi phí |
|---|---|
| Bộ nhớ | +1 con trỏ **mỗi object** (8 byte trên 64-bit) — đáng kể với object nhỏ, số lượng lớn |
| Mỗi lời gọi | +1 lần gián tiếp (load vptr → load ô → call) |
| Tối ưu | **Không inline được** (compiler không biết đích) → mất luôn các tối ưu phía sau. Đây thường là chi phí lớn nhất, không phải bản thân lần gián tiếp |
| Layout | Class không còn là POD → không `memcpy`/serialize thô được |

**Bẫy:** (1) tưởng "mỗi object có một vtable" — không, **vtable dùng chung cho cả class**, object chỉ giữ *con trỏ*; (2) gọi hàm virtual trong **constructor/destructor** → vptr lúc đó còn trỏ vtable của **class đang xây**, nên gọi phiên bản base chứ không phải override ([CPP-035](cpp.md)); (3) đánh dấu `final` giúp compiler **devirtualize** (gọi thẳng, inline được) — mẹo tối ưu thực tế.

**Chốt:** *"vtable là bảng của class, vptr là con trỏ trong object. Gọi virtual = tra bảng lúc chạy — chi phí thật nằm ở chỗ không inline được, không phải một lần gián tiếp."*
</details>

#### CPP-007 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**`unique_ptr`, `shared_ptr`, `weak_ptr` khác nhau và dùng khi nào?**
<details><summary>Đáp án</summary>

**Trục phân biệt là *quyền sở hữu*** (ai quyết định lúc nào object chết), không phải "cái nào tiện hơn".

| | `unique_ptr` | `shared_ptr` | `weak_ptr` |
|---|---|---|---|
| Sở hữu | **Độc quyền** — đúng 1 chủ | **Chia sẻ** — N chủ, chết khi count = 0 | **Không sở hữu** — chỉ quan sát |
| Copy | ❌ chỉ `move` | ✅ (tăng count) | ✅ |
| Chi phí | **Zero** — bằng con trỏ thô | 2 con trỏ + control block + **atomic** mỗi copy/huỷ | như `shared_ptr`, không đụng strong count |
| Truy cập | `*` `->` | `*` `->` | phải `.lock()` → trả `shared_ptr` (rỗng nếu object đã chết) |
| Dùng khi | **Mặc định** cho mọi owning pointer | Vòng đời **thật sự** do nhiều bên độc lập quyết định | Phá **circular reference** ([CPP-025](cpp.md)); cache/observer "dùng nếu còn sống" |

```cpp
auto u = std::make_unique<Widget>();      // ✅ mặc định
auto s = std::make_shared<Widget>();      // chỉ khi thật sự nhiều owner
std::weak_ptr<Widget> w = s;              // quan sát, không giữ sống

if (auto sp = w.lock()) sp->use();        // ✅ cách DUY NHẤT dùng weak_ptr:
                                          //    lock() rồi kiểm tra, không hỏi expired() rồi mới lock
```

**Bẫy:** (1) mặc định chọn `shared_ptr` cho "an toàn" — thực ra nó **giấu** kiến trúc sở hữu mù mờ, tốn atomic mỗi copy, và mở đường cho circular leak; (2) `w.expired()` rồi mới `w.lock()` là **race** — object có thể chết ở giữa; `lock()` một lần rồi kiểm tra kết quả; (3) `unique_ptr` **không** phải "yếu hơn" — nó chuyển quyền sở hữu bằng `std::move` và diễn đạt ý định rõ hơn.

**Chốt:** *"Mặc định `unique_ptr`. Chỉ lên `shared_ptr` khi trả lời được: **ai** cùng sở hữu, và **vì sao** không xác định được một chủ duy nhất."*
</details>

#### CPP-008 · 🟡 · concept · ⭐ · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**`std::move` thực sự làm gì? Move semantics tăng hiệu năng ra sao?**
<details><summary>Đáp án</summary>

**`std::move` KHÔNG move gì cả** — tên gọi là một trong những cái tên tệ nhất của chuẩn. Nó chỉ là một **`static_cast`** sang rvalue reference:

```cpp
template<class T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(t);   // ← toàn bộ "phép màu"
}
```

**Ba bước thực sự xảy ra:**
1. `std::move(x)` đổi **value category** của `x` từ lvalue → rvalue. **Không sinh một lệnh máy nào** khi chạy.
2. Nhờ đó overload resolution chọn **move ctor / move assignment** thay vì bản copy.
3. **Move ctor mới là nơi di chuyển thật**: cướp con trỏ nội bộ của nguồn, rồi **để nguồn ở trạng thái rỗng nhưng hợp lệ** (valid but unspecified).

```cpp
std::string a = "…1MB…";
std::string b = std::move(a);   // copy 3 con trỏ (O(1)) thay vì memcpy 1MB (O(n))
// a giờ hợp lệ nhưng nội dung KHÔNG xác định — chỉ được gán lại hoặc huỷ
```

**Vì sao nhanh:** copy phải **cấp phát + sao chép** toàn bộ dữ liệu (O(n) + malloc); move chỉ **chuyển quyền sở hữu buffer** — vài lần gán con trỏ, O(1), không cấp phát.

**Bẫy:** (1) tưởng `std::move` tự làm gì đó lúc runtime — không, nó thuần compile-time; (2) **dùng lại biến sau khi move** (`a` ở trên) — không phải UB nhưng giá trị **không xác định**, đừng đọc; (3) `std::move` một object `const` **im lặng rơi về copy** (`const T&&` không bind được move ctor) → mất tối ưu mà không có cảnh báo; (4) `return std::move(local)` — **thừa và có hại**, nó **chặn NRVO** (copy elision), cứ `return local;`.

**Chốt:** *"`std::move` là một cast, không phải một hành động. Nó chỉ *cho phép* move xảy ra; move ctor mới làm việc."*
</details>

#### CPP-009 · 🟡 · concept · [→ templates](../../../01-cpp-fundamentals/templates.md)
**Template hoạt động lúc nào? Vì sao định nghĩa phải ở header?**
<details><summary>Đáp án</summary>

Template được compiler instantiate thành code chuyên biệt cho từng kiểu lúc **biên dịch** (không chi phí runtime). Định nghĩa phải ở header vì compiler cần thấy toàn bộ định nghĩa tại điểm sử dụng để sinh code; tách vào .cpp → các TU khác chỉ có khai báo → lỗi linker.
</details>

#### CPP-010 · 🟡 · concept · ⭐ · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Vì sao destructor của base class nên là virtual?**
<details><summary>Đáp án</summary>

**Cơ chế:** `delete p` với `p` kiểu `Base*` phải quyết định gọi destructor nào. Nếu `~Base()` **không** virtual, lời gọi được **bind tĩnh** theo *kiểu con trỏ* → chỉ `~Base()` chạy, `~Derived()` **không bao giờ** chạy. Nếu **virtual**, lời gọi đi qua vtable → chọn đúng `~Derived()` theo *kiểu thật*, rồi tự động gọi ngược lên `~Base()`.

```cpp
class Base    { public: ~Base() {} };                    // ❌ KHÔNG virtual
class Derived : public Base {
    int* data_ = new int[100];
public:
    ~Derived() { delete[] data_; }
};

Base* p = new Derived();
delete p;   // ❌ chỉ ~Base() chạy → ~Derived() bị bỏ → leak data_
            //    Chuẩn nói đây là UNDEFINED BEHAVIOR, không chỉ "leak"

// ✅ Sửa:
class Base { public: virtual ~Base() = default; };
```

**Nói "UB" chứ đừng nói "leak"** — đó là chi tiết phân biệt người hiểu bản chất. Leak chỉ là *triệu chứng thường thấy*; chuẩn quy định hành vi là **không xác định**, nên nó có thể corrupt heap chứ không chỉ rò rỉ. Kể cả `Derived` không có tài nguyên gì thì `delete` qua base non-virtual **vẫn** là UB.

**Quy tắc áp dụng:**

| Class dùng làm gì | Destructor nên là |
|---|---|
| Base **đa hình** (xoá object con qua con trỏ base) | `virtual ~Base() = default;` |
| Base chỉ để tái sử dụng code, **không** xoá qua con trỏ base | `protected: ~Base() = default;` — cấm luôn việc xoá qua base, khỏi trả phí vtable |
| Class **không** làm base (`final`, value type) | Không cần gì — thêm `virtual` là tự trả phí vptr vô ích |

**Bẫy:** (1) khai báo `~Base()` (dù `= default`) làm compiler **ngừng tự sinh move ctor/assign** → class âm thầm mất move, mọi thao tác rơi về copy → nếu khai báo dtor thì cân nhắc khai báo đủ 5 ([Rule of 5](cpp.md)); (2) thêm `virtual` cho mọi class "cho chắc" — mỗi object phải mang thêm vptr, hỏng cả layout POD và khả năng dùng `memcpy`.

**Chốt:** *"Xoá qua con trỏ base mà dtor không virtual = UB. Base đa hình → virtual dtor; không định cho xoá qua base → dtor protected."*
</details>

#### CPP-011 · 🟠 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Vì sao move constructor nên `noexcept`? Hậu quả nếu quên?**
<details><summary>Đáp án</summary>

**Vì `std::vector` phải chọn giữa *nhanh* và *an toàn*, và nó luôn chọn an toàn.**

Khi `vector` hết chỗ, nó cấp vùng nhớ mới rồi chuyển từng phần tử sang. Nó muốn **move** (rẻ), nhưng phải giữ **strong exception guarantee**: nếu có exception, `vector` phải như chưa hề reallocate.

- **Copy** ném giữa chừng → vùng cũ **vẫn nguyên vẹn** → huỷ vùng mới, coi như chưa xảy ra gì. ✅ khôi phục được.
- **Move** ném giữa chừng → các phần tử đã move sang là *rỗng*, phần tử ở vùng cũ **đã bị cướp ruột** → **không thể** quay lui. ❌

Nên `vector` chỉ dám move khi bạn **cam kết move không ném** — bằng `noexcept`. Không cam kết → nó âm thầm **copy**.

```cpp
class Buffer {
    char* data_; size_t n_;
public:
    Buffer(Buffer&& o) noexcept          // ✅ noexcept -> vector sẽ MOVE
        : data_(o.data_), n_(o.n_) { o.data_ = nullptr; o.n_ = 0; }
    Buffer& operator=(Buffer&& o) noexcept { /* … */ return *this; }
};

static_assert(std::is_nothrow_move_constructible_v<Buffer>);   // ✅ chốt bằng compile-time
```

**Vì sao đây là bẫy khó chịu:** mất `noexcept` **không có warning, không có lỗi** — code vẫn đúng, chỉ **chậm đi âm thầm**, và chỉ lộ ra khi profile thấy `vector` push nhiều lại đi gọi copy ctor. Với `Buffer` 1MB, đó là khác biệt giữa O(1) và O(n) mỗi lần realloc.

**Bẫy:** (1) move ctor **cấp phát** thứ gì đó (vd cấp buffer mới thay vì cướp con trỏ) thì **không được** đánh `noexcept` — sửa thiết kế, đừng nói dối compiler; nói dối mà ném thật → `std::terminate` ngay; (2) `std::move_if_noexcept` là thứ `vector` dùng bên trong, nên biết tên; (3) move ctor do **compiler tự sinh** thường đã là `noexcept` — vấn đề chỉ phát sinh khi bạn tự viết ([Rule of 5](cpp.md)).

**Chốt:** *"`vector` chỉ dám move khi move không thể ném — không `noexcept` thì nó copy để giữ strong guarantee. Quên = mất hiệu năng, không có tiếng động nào."*
</details>

#### CPP-012 · 🟠 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**RVO là gì? Vì sao `return std::move(local)` là phản tác dụng?**
<details><summary>Đáp án</summary>

**RVO / copy elision:** compiler **dựng thẳng** object trả về vào ô nhớ của caller — bỏ qua **cả copy lẫn move**, tức **không** phép sao chép nào cả. Caller truyền ngầm địa chỉ đích, hàm construct tại chỗ.

| | Điều kiện | Từ C++17 |
|---|---|---|
| **RVO** (trả về **tạm**, prvalue) | `return Widget(1,2);` | **Bắt buộc** — chuẩn *đảm bảo* không có copy/move, kể cả khi ctor bị `delete` |
| **NRVO** (trả về **biến local có tên**) | `Widget w; …; return w;` | Vẫn là *tối ưu tuỳ chọn* — compiler được phép nhưng không bắt buộc |

**Vì sao `return std::move(w)` phản tác dụng:** NRVO chỉ áp dụng khi biểu thức `return` là **tên của một biến local**. `std::move(w)` là một **biểu thức cast**, không còn là tên biến → compiler **mất quyền** elide → bắt buộc gọi move ctor. Bạn tự tay đổi *0 phép sao chép* lấy *1 lần move*.

```cpp
Widget make() {
    Widget w;
    // …
    return w;              // ✅ NRVO: 0 lần copy/move (và nếu không NRVO thì vẫn tự move)
    // return std::move(w);   ❌ chặn NRVO -> luôn tốn 1 move
}
```

Kể cả khi NRVO không xảy ra, `return w;` **vẫn tự động move** (chuẩn quy định coi `w` là rvalue trong ngữ cảnh return). Nên `std::move` ở đây **không bao giờ có lợi**, chỉ có hại.

**Bẫy:** (1) `return std::move(member_)` hay `return std::move(param)` — với **tham số hàm** thì NRVO vốn không áp dụng, nên `std::move` ở đó **hợp lệ**; chỉ cấm với **local**; (2) trả về **kiểu khác** kiểu biến (vd biến `Derived`, trả `Base`) thì elision không áp dụng — lúc này `std::move` mới có tác dụng; (3) `-fno-elide-constructors` chỉ để học/quan sát, đừng dùng thật.

**Chốt:** *"`return w;` — chấm hết. Compiler đã lo elide, và nếu không elide được thì nó tự move. Thêm `std::move` chỉ tước mất RVO."*
</details>

#### CPP-013 · 🟠 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Perfect forwarding là gì? `std::move` khác `std::forward` thế nào?**
<details><summary>Đáp án</summary>

**Vấn đề cần giải:** một wrapper template (factory, `emplace_back`, `make_unique`) nhận tham số rồi chuyển tiếp xuống constructor bên dưới. Nếu chuyển tiếp "thường", **thông tin lvalue/rvalue bị mất** — caller truyền rvalue (đáng lẽ được move) nhưng bên trong wrapper nó đã thành một biến có tên, tức lvalue → hàm đích **copy** thay vì move.

**Perfect forwarding = giữ nguyên value category** của đối số gốc suốt đường đi. Công thức: **forwarding reference `T&&` + `std::forward<T>`**.

```cpp
template<class T, class... Args>
std::unique_ptr<T> my_make_unique(Args&&... args) {          // ✅ Args&& = forwarding ref
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}                                                             // ✅ lvalue -> copy, rvalue -> move

std::string s = "hi";
my_make_unique<Widget>(s);              // s là lvalue -> Widget copy từ s (s còn nguyên)
my_make_unique<Widget>(std::move(s));   // rvalue    -> Widget move (s bị cướp) ✅ đúng ý caller
```

| | `std::move` | `std::forward<T>` |
|---|---|---|
| Làm gì | **Luôn** ép sang rvalue, vô điều kiện | Ép sang rvalue **chỉ khi** `T` suy ra từ một rvalue |
| Dùng ở đâu | Rvalue reference **thường** (`Widget&&`), khi bạn **biết chắc** muốn cướp | Bên trong template, trên **forwarding reference** (`T&&` suy luận) |
| Cách nhớ | "Tôi quyết định: move đi" | "Giữ nguyên như caller đã đưa" |

**Bẫy:** (1) dùng `std::move` trên forwarding reference → cướp mất object của caller dù họ truyền lvalue — **lỗi kinh điển**; (2) `std::forward` trên rvalue ref thường thì thừa nhưng vô hại, ngược lại thì nguy hiểm; (3) **forward hai lần** cùng một tham số (dùng `args` ở hai chỗ) → lần thứ hai đọc object đã bị cướp; (4) quên tham số kiểu: phải là `std::forward<T>(x)`, `std::forward(x)` không biên dịch. Xem thêm [CPP-014](cpp.md).

**Chốt:** *"`move` là mệnh lệnh, `forward` là chuyển tiếp trung thực. Trong template với `T&&` suy luận, luôn dùng `forward`."*
</details>

#### CPP-014 · 🟠 · concept · ⭐ · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**Phân biệt rvalue reference và universal (forwarding) reference?**
<details><summary>Đáp án</summary>

**Cùng cú pháp `&&`, khác nhau ở một điều kiện duy nhất: kiểu có được *suy luận* hay không.**

| | **Rvalue reference** | **Universal / forwarding reference** |
|---|---|---|
| Hình dạng | `Widget&&` — kiểu **cụ thể**, không suy luận | `T&&` với `T` **được suy luận**, hoặc `auto&&` |
| Nhận được | **Chỉ** rvalue | **Cả** lvalue lẫn rvalue (nhờ *reference collapsing*) |
| Chuyển tiếp bằng | `std::move` | `std::forward<T>` |

```cpp
void f(Widget&& w);                       // rvalue ref THẬT — chỉ nhận rvalue
template<class T> void g(T&& x);          // ✅ universal ref — T được suy luận
auto&& r = anything;                       // ✅ universal ref

template<class T> void h(std::vector<T>&& v);   // ❌ KHÔNG phải universal ref!
                                                //    T suy luận, nhưng tham số là vector<T>&&
                                                //    -> không đúng dạng "T&&" trần
```

**Vì sao `T&&` lại nhận được lvalue** — *reference collapsing*: truyền lvalue thì `T` suy ra là `Widget&`, thay vào thành `Widget& &&` → sập lại thành `Widget&`. Truyền rvalue thì `T` = `Widget`, thành `Widget&&`. Một khuôn, hai vai.

```cpp
template<class T>
void wrapper(T&& x) {
    inner(std::forward<T>(x));   // ✅ lvalue vào -> lvalue ra; rvalue vào -> rvalue ra
    // inner(std::move(x));      // ❌ ép rvalue LUÔN -> "cướp" cả object của caller
}
```

**Bẫy:** (1) dùng `std::move` trên universal ref → object của caller bị move mất dù họ truyền lvalue; (2) `const T&&` **không** phải universal ref (`const` chặn collapsing); (3) universal ref là **overload tham lam** — nó khớp *mọi thứ*, thường thắng cả copy ctor → dùng `std::enable_if`/`requires` để ghìm lại; (4) `std::forward` viết thiếu tham số kiểu (`std::forward(x)`) là lỗi — luôn `std::forward<T>(x)`.

**Chốt:** *"`&&` + kiểu được suy luận = universal ref → `std::forward`. `&&` + kiểu cụ thể = rvalue ref → `std::move`."* Đây là nền của **perfect forwarding** (`make_unique`, `emplace_back` đều dựa vào nó).
</details>

#### CPP-015 · 🟠 · concept · [→ lambdas-functional](../../../02-modern-cpp/lambdas-functional.md)
**Bẫy nguy hiểm nhất của lambda capture là gì?**
<details><summary>Đáp án</summary>

**Dangling capture: `[&]` một biến local rồi để lambda sống lâu hơn biến đó.** Lambda giữ *reference*, không giữ object — biến chết, reference thành rác, đọc là **UB**. Nguy hiểm vì code **chạy đúng trong test** (bộ nhớ chưa bị ghi đè) rồi hỏng ở production.

```cpp
std::function<int()> makeCounter() {
    int count = 0;
    return [&]() { return ++count; };   // ❌ count chết khi hàm return -> dangling
}
// ✅ sửa: return [count]() mutable { return ++count; };   // capture by copy
```

**Quy tắc quyết định — theo *tuổi thọ* của lambda, không theo sở thích:**

| Lambda sống | Capture | Vì sao |
|---|---|---|
| **Ngắn** — truyền thẳng vào `std::sort`, `for_each`, gọi xong là xong | `[&]` | Rẻ, biến chắc chắn còn sống |
| **Lưu lại / async** — vào `std::function`, thread, callback, queue | `[=]` / `[x]` / `[p = std::move(ptr)]` | Lambda phải **tự sở hữu** dữ liệu của nó |

**Cái bẫy con, còn khó thấy hơn: `[this]` và `[=]` trong class.**

```cpp
class Worker {
    int state_;
    void start() {
        pool.post([this]{ use(state_); });   // ❌ capture con TRỎ this
    }                                        //    object Worker chết trước khi task chạy -> UB
};
// ✅ C++17: [*this] copy cả object   |   ✅ hoặc giữ sống bằng shared_from_this()
```

⚠️ **`[=]` trong hàm thành viên KHÔNG copy các member** — nó capture `this` rồi truy cập member qua đó. Nhìn như "copy hết cho an toàn" nhưng thực chất vẫn là reference tới object. C++20 đã deprecate `[=]` ngầm capture `this` chính vì hiểu lầm này.

**Bẫy:** (1) tưởng `[=]` luôn an toàn — sai với member (xem trên); (2) `mutable` chỉ cho phép **sửa bản sao riêng** trong lambda, không đụng biến gốc; (3) capture biến chỉ để đọc mà object lớn → `[=]` copy đắt, dùng `[&x]` hoặc `[x = std::cref(big)]`.

**Chốt:** *"Lambda sống lâu hơn scope → phải tự sở hữu dữ liệu. `[&]` chỉ dành cho lambda dùng-rồi-vứt, và `[=]` trong class không cứu bạn — nó vẫn là `this`."*
</details>

#### CPP-016 · 🟠 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Object slicing là gì và khi nào xảy ra?**
<details><summary>Đáp án</summary>

**Cơ chế:** một object kiểu `Base` có **kích thước cố định** lúc compile (`sizeof(Base)`). Gán một `Derived` vào đó, chỉ phần `Base` được copy — **phần dữ liệu riêng của `Derived` bị cắt bỏ**, và vptr cũng thành vptr của `Base` → **mất luôn đa hình**.

```cpp
Circle c(2.0);
Shape s = c;      // ❌ slicing: chỉ phần Shape được copy
s.area();         // gọi Shape::area — KHÔNG phải Circle::area

Shape& r = c;     // ✅ reference: không copy, đa hình còn nguyên
Shape* p = &c;    // ✅ con trỏ: như trên
```

**Ba chỗ nó lẻn vào mà không ai để ý:**

| Chỗ | Ví dụ | Sửa |
|---|---|---|
| **Tham số by-value** | `void draw(Shape s)` | `void draw(const Shape& s)` |
| **Container of value** | `std::vector<Shape> v; v.push_back(circle);` | `std::vector<std::unique_ptr<Shape>>` |
| **Gán / trả về by-value** | `Shape s = getCircle();` | trả `unique_ptr<Shape>` hoặc dùng reference |

Nguy hiểm nhất là ca **container**: `vector<Shape>` biên dịch sạch sẽ, chạy không crash, chỉ **im lặng gọi sai hàm** — không có warning nào theo mặc định.

**Bẫy:** (1) tưởng slicing là lỗi bộ nhớ — không, nó **không** phải UB, chỉ là copy đúng luật nhưng **sai ý định**, nên không công cụ nào bắt được; (2) quên rằng nó xảy ra cả khi **gán** (`s = c`, qua copy assignment của base) chứ không chỉ khi khởi tạo; (3) muốn chặn hẳn: cho base class `protected` copy ctor/assign, hoặc `= delete` chúng — biến slicing thành **lỗi compile**.

**Chốt:** *"Đa hình chỉ sống qua con trỏ/reference. Hễ để object đa hình nằm ở dạng trị giá là đã cắt mất nó."*
</details>

#### CPP-017 · 🟠 · concept · [→ templates](../../../01-cpp-fundamentals/templates.md)
**Khi nào dùng template, khi nào dùng đa hình virtual?**
<details><summary>Đáp án</summary>

**Cùng giải một bài toán — "một logic, nhiều kiểu" — nhưng quyết định ở hai thời điểm khác nhau.** Câu hỏi phân định: **tập kiểu có biết được lúc biên dịch không?**

| | **Template** (compile-time) | **Virtual** (runtime) |
|---|---|---|
| Chọn phiên bản | Lúc **biên dịch** (instantiation) | Lúc **chạy** (tra vtable) |
| Chi phí runtime | **Không** — như viết tay cho kiểu đó, **inline được** | +vptr/object, +1 gián tiếp, **không inline** |
| Kích thước binary | Có thể **phình** (code bloat — mỗi kiểu một bản) | Gọn hơn — một bản code dùng chung |
| Thời gian biên dịch | Lâu; lỗi khó đọc | Nhanh |
| Ràng buộc kiểu | *Duck typing* — chỉ cần có đúng hàm/toán tử | Phải **kế thừa** đúng interface |
| Mở rộng sau khi ship | ❌ phải build lại | ✅ thêm class mới, kể cả **plugin/`dlopen`** |

**Chọn template khi:** tập kiểu **đóng và biết trước**, cần hiệu năng — STL, container, thuật toán generic, code trong hot path, hoặc kiểu không sửa được (`int`, `float`, struct C không kế thừa được).

**Chọn virtual khi:** tập kiểu **mở / quyết định lúc chạy** — plugin, backend chọn theo config, giữ nhiều object khác kiểu trong **một** container (`vector<unique_ptr<Shape>>`), hoặc cần **mock** để test.

**Góc embedded (hay hỏi):** template không tốn RAM runtime nhưng **ăn flash** (code bloat) — trên MCU vài trăm KB đó là ràng buộc thật. Virtual tốn vptr mỗi object + chặn inline. Không có câu trả lời "luôn đúng", phải nói theo **ràng buộc tài nguyên cụ thể**.

**Bẫy:** (1) trả lời "template nhanh hơn nên luôn dùng" — bỏ qua code bloat và compile time; (2) quên rằng hai cái **kết hợp được**: *type erasure* (`std::function`, `std::any`) dùng template ở ngoài + virtual ở trong để có API generic mà vẫn linh hoạt runtime; (3) CRTP là lựa chọn thứ ba khi muốn "đa hình" mà không trả phí vtable.

**Chốt:** *"Biết kiểu lúc compile → template (trả phí bằng binary). Chỉ biết lúc chạy → virtual (trả phí bằng gián tiếp)."*
</details>

#### CPP-018 · 🔴 · concept · [→ concurrency](../../../02-modern-cpp/concurrency.md)
**`std::atomic` đủ để đồng bộ, hay cần mutex? Khi nào dùng cái nào?**
<details><summary>Đáp án</summary>

**Ranh giới: `atomic` bảo vệ được đúng *một* biến, trong đúng *một* thao tác.** Hễ tính đúng đắn phụ thuộc vào **≥2 biến** hoặc **≥2 bước**, atomic không đủ — cần mutex.

| Dùng `atomic` | Dùng `mutex` |
|---|---|
| Counter, flag, con trỏ đơn | Cập nhật **nhiều biến** phải nhất quán với nhau |
| Một thao tác đọc-sửa-ghi có sẵn: `fetch_add`, `exchange`, `compare_exchange` | Chuỗi thao tác phải **không bị chen** (kiểm tra rồi hành động) |
| Không khoá → không deadlock, dùng được ở nơi không được ngủ | Critical section dài, gọi hàm khác, cấp phát bộ nhớ |

```cpp
std::atomic<int> hits{0};
hits.fetch_add(1);                    // ✅ một biến, một thao tác

// ❌ Sai kinh điển: mỗi biến đều atomic nhưng CẶP thì không
std::atomic<int> n{0};
std::atomic<int> sum{0};
n++; sum += v;      // thread khác có thể quan sát n đã tăng mà sum chưa -> bất biến "sum là tổng của n phần tử" bị vỡ

// ❌ Sai thứ hai: check-then-act, hai bước không nguyên tử
if (head.load() == nullptr) head.store(newNode);   // thread khác chen vào giữa

// ✅ Đúng: một thao tác nguyên tử duy nhất
Node* expected = nullptr;
head.compare_exchange_strong(expected, newNode);   // CAS
```

**Bẫy:** (1) tưởng "biến nào cũng atomic là thread-safe" — *bất biến giữa các biến* mới là thứ cần bảo vệ, không phải từng biến; (2) atomic **không** miễn phí — trên nhiều core, cùng ghi một biến gây **cache line ping-pong**, có khi chậm hơn mutex; (3) `atomic<T>` với `T` lớn (struct) sẽ **âm thầm dùng mutex bên trong** — kiểm bằng `is_lock_free()`; (4) atomic đi kèm **memory order** ([CPP-019](cpp.md)) — mặc định `seq_cst` mới là cái đang giữ cho nó đúng.

**Chốt:** *"Một biến, một thao tác → atomic. Nhiều biến hoặc nhiều bước phải nhất quán → mutex. Lock-free chỉ đáng làm khi đã đo và thật sự cần."*
</details>

#### CPP-019 · 🔴 · concept · ⭐ · [→ concurrency §6](../../../02-modern-cpp/concurrency.md#L114)
**Memory order là gì? Mặc định nên dùng cái nào và vì sao thận trọng?**
<details><summary>Đáp án</summary>

**Vấn đề gốc: `atomic` đảm bảo *tính nguyên tử*, không đảm bảo *thứ tự nhìn thấy*.** Hai chuyện khác nhau — memory order là phần lo chuyện thứ hai.

**Ai reorder — hai tầng, đừng chỉ nói compiler:**
1. **Compiler** — sắp xếp lại lệnh khi tối ưu.
2. **CPU** — out-of-order execution, **store buffer** (ghi nằm đệm trong core một lúc mới lộ ra cho core khác), cache coherence có độ trễ.

Cả hai được phép, vì chúng chỉ cần bảo toàn ngữ nghĩa **trong một thread**. Đa thread mới lộ: thread khác quan sát thấy thứ tự ghi **khác** thứ tự bạn viết.

| Mức | Đảm bảo | Chi phí | Dùng khi |
|---|---|---|---|
| `seq_cst` | **Mặc định.** Tồn tại **một** thứ tự tuần tự toàn cục mà **mọi** thread đều thấy giống nhau | Cao nhất (thường full barrier / `mfence`) | Mặc định, tới khi đo thấy nghẽn |
| `acquire` / `release` | Đồng bộ **theo cặp** — xem mục dưới | Trung bình (thường free trên x86, có phí trên ARM) | Producer/consumer, flag công bố dữ liệu |
| `relaxed` | **Chỉ** atomicity — không ràng buộc thứ tự gì | Thấp nhất | Counter độc lập (thống kê), không ai suy luận theo nó |

---

### Release / acquire — mẫu "publish → subscribe"

```cpp
std::atomic<bool> ready{false};
int data = 0;                 // ⚠️ biến THƯỜNG, không atomic

// ───── Thread A (publisher) ─────        // ───── Thread B (subscriber) ─────
data = 42;                                 if (ready.load(memory_order_acquire)) {
ready.store(true, memory_order_release);       use(data);      // ✅ chắc chắn thấy 42
                                           }
//  ▲ mọi ghi TRƯỚC release được công bố    //  ▲ thấy true ⟹ thấy mọi thứ A làm trước
```

Điều được đảm bảo, phát biểu dạng **NẾU… THÌ…**:

> **NẾU** B `acquire` trên **đúng biến** `ready` **VÀ đọc được đúng giá trị** A vừa `release` ghi
> **THÌ** B thấy **mọi** ghi A làm trước `release` — **kể cả `data`, một biến không atomic**.

**Đọc theo 3 ca để nhớ chắc:**

| Ca | Có đồng bộ? | Kết quả |
|---|---|---|
| B `acquire` `ready`, **thấy `true`** | ✅ có | Thấy `data == 42`. Đúng như mong đợi |
| B `acquire` `ready`, **thấy `false`** | ❌ không | Nhưng `if` **không chạy** → không chạm `data` → **vẫn an toàn**. Sự đúng đắn nằm ở chỗ **cờ canh cửa** |
| B `acquire` **một biến khác** (`flag2`) | ❌ không | Không lập được cặp → `use(data)` có thể đọc rác, dù A đã `release` xong |

⚠️ **`release` KHÔNG phải lệnh "flush"** đẩy dữ liệu ra cho cả thế giới. Cặp đôi lập qua **dữ liệu**, không qua **thời gian**: không phải *"A release xong rồi nên B sau đó chắc chắn thấy"*, mà là *"B **nhìn thấy giá trị** của A ⟹ B thấy luôn mọi thứ A làm trước đó"*. Quan hệ này tên là **synchronizes-with**.

**Mỗi bên chỉ dựng NỬA hàng rào — phải đủ cả hai:**

```
Thread A                              Thread B
  data = 42                             ┌── acquire ───────────  ⛔ đọc phía sau
  ⛔ ghi phía trên ───┐ không trượt      │   if (ready)              không trượt LÊN
  ready = true (rel)  │ xuống dưới       │       use(data)   ✅
                      └──── nhìn thấy ───┘
```

Vì thế **đổi `acquire` thành `relaxed` là hỏng, dù đọc đúng biến và thấy `true`**:

```cpp
if (ready.load(memory_order_relaxed))   // thấy true, nhưng KHÔNG lập cặp đồng bộ
    use(data);                          // ❌ data vẫn có thể là rác
```
`relaxed` chỉ hứa `ready` không bị đọc rách. Không có `acquire`, CPU của B được tự do đọc `data` **trước** khi đọc `ready` → lấy về giá trị cũ.

**Ba điều kiện, thiếu một là mất đồng bộ:** ① nhãn phải có ở **cả hai đầu** · ② phải **cùng một biến atomic** · ③ phải **đọc thấy giá trị đã release**.

---

**Bẫy:** (1) nghĩ "đã là `atomic` thì thread khác thấy ngay và đúng thứ tự" — sai, đó là việc của memory order; (2) hạ `relaxed` cho cờ *đồng bộ* vì "đo thấy nhanh hơn" — bug loại này **không reproduce được**; (3) ⭐ code sai kiểu trên **vẫn chạy đúng trên x86** (memory model mạnh, không reorder load–load) rồi **chết trên ARM** (weak memory model) — đúng lớp bug nguy hiểm nhất cho embedded; (4) tưởng `volatile` thay được — không, xem [CPP-022](cpp.md).

**Chốt (câu trả lời an toàn):** *"CPU và compiler đều reorder; single-thread không thấy, đa thread thì lộ. Ba mức: seq_cst / acquire-release / relaxed. Em mặc định `seq_cst` và chỉ hạ khi đã profile — sai ở đây tạo bug không tái hiện được."* Thái độ thận trọng ở câu cuối là thứ interviewer chấm, không phải việc thuộc tên.
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

**`volatile` nói với compiler đúng một điều: "mỗi lần đọc/ghi biến này phải diễn ra thật, đừng tối ưu".** Cấm cache vào thanh ghi, cấm loại bỏ lần đọc/ghi tưởng như thừa, cấm đảo thứ tự **giữa các truy cập volatile với nhau**. Hết. Nó là chỉ thị cho **compiler**, không phải lệnh cho **phần cứng**.

**Dùng đúng — hai ca, cả hai đều là *một* bên ngoài luồng compiler thấy:**
1. **MMIO** — thanh ghi phần cứng: giá trị đổi do thiết bị (status register), và **ghi có side effect** (kích hoạt hành động) nên không được gộp/bỏ.
2. **Biến chia sẻ với ISR** — ISR sửa "ngoài luồng" của main loop.

```cpp
#define GPIO_ODR (*(volatile uint32_t*)(GPIO_BASE + 0x14))   // ✅ MMIO
volatile bool flag;              // ✅ ISR đặt, main loop đọc — 1 biến ≤ word
```

**KHÔNG dùng để đồng bộ đa luồng.** Ba thứ nó **không** cho:

| Không có | Hậu quả |
|---|---|
| **Atomicity** | `v++` vẫn là read-modify-write 3 bước → mất cập nhật |
| **Memory ordering giữa các core** | Core khác vẫn có thể thấy thứ tự ghi khác — `volatile` không sinh barrier |
| **Cache coherence / visibility** | Không ép dữ liệu lộ ra cho core khác |

Đồng bộ đa luồng → `std::atomic` (kèm memory order, [CPP-019](cpp.md)) hoặc mutex.

**Bẫy:** (1) ⚠️ **`volatile` nhiều byte không nguyên tử** — `volatile uint64_t` trên MCU/CPU 32-bit đọc bằng **2 lệnh load**; ISR chen vào giữa → **torn read**, ghép nửa cũ nửa mới thành giá trị chưa từng tồn tại. Biến ISR **≤ 1 word** thì `volatile` đủ; lớn hơn → **critical section** (tắt ngắt cực ngắn) hoặc ring buffer SPSC ([EMB-010](embedded-fundamentals.md)); (2) MMIO cần thêm **memory barrier** (`__DMB`/`dmb`) để ràng buộc thứ tự giữa thanh ghi và bộ nhớ thường — `volatile` không lo việc đó; trong kernel Linux dùng `readl()/writel()` (đã bao gồm barrier); (3) `volatile` **không** thay được `std::atomic` kể cả cho `bool` trên đa lõi.

**Chốt:** *"`volatile` = 'đọc/ghi thật, đừng tối ưu'. Nó nói chuyện với compiler, không nói chuyện với các core khác — nên không cho atomicity, không cho ordering, không thread-safe."*
</details>

#### CPP-023 · 🔴 · concept · [→ api-design](../../../07-shared-libraries/api-design.md), [abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Vì sao C++ ABI không ổn định giữa các compiler? Hệ quả khi thiết kế thư viện?**
<details><summary>Đáp án</summary>

Vì C++ phơi bày nhiều chi tiết triển khai ra ABI: name mangling không chuẩn hóa, layout object/vtable, cách xử lý exception, cài đặt thư viện chuẩn — khác nhau giữa compiler/phiên bản. Hệ quả: thư viện cần ổn định/đa ngôn ngữ nên phơi bày C API (`extern "C"`, POD, opaque handle) ở biên giới và cài đặt C++ bên trong; tránh để class C++/exception vượt biên; dùng pimpl bảo vệ ABI.
</details>

#### CPP-024 · 🟡 · concept · ⭐ · [→ concurrency](../../../02-modern-cpp/concurrency.md)
**`shared_ptr` có thread-safe không?**
<details><summary>Đáp án</summary>

**Câu hỏi thiếu chủ ngữ — phải hỏi lại "thread-safe *cái gì*?".** Có **ba tầng**, chỉ tầng đầu được chuẩn bảo đảm:

| Tầng | Thread-safe? | Nghĩa là |
|---|---|---|
| **Control block** (strong/weak count) | ✅ | Nhiều thread cùng **copy/huỷ các instance khác nhau** trỏ về một object → an toàn. Đếm bằng atomic |
| **Bản thân object `shared_ptr`** | ❌ | Hai thread cùng ghi vào **cùng một biến** `shared_ptr` = data race → xem [CPP-052](cpp.md) |
| **Object được trỏ tới** (payload) | ❌ | Không ai bảo vệ dữ liệu của bạn — cần mutex riêng |

```cpp
std::shared_ptr<Widget> g = std::make_shared<Widget>();

// ✅ An toàn: mỗi thread có instance RIÊNG (tham số by value = copy)
void worker(std::shared_ptr<Widget> p) { /* copy/huỷ p thoải mái */ }

// ❌ Race tầng 2: hai thread cùng GHI vào cùng một biến g
g = std::make_shared<Widget>();     // chạy song song ở 2 thread -> hỏng

// ❌ Race tầng 3: count atomic không bảo vệ nội dung Widget
p->counter++;                        // vẫn cần mutex/atomic bên trong Widget
```

**Bẫy — chính là hiểu lầm phổ biến nhất:** *"count là atomic nên `shared_ptr` an toàn"*. Atomic chỉ áp cho **bộ đếm**; phép gán `p = other` phải giảm count cũ, ghi **hai** con trỏ trong `p`, tăng count mới — **không** nguyên tử.

**Chốt:** *"Đếm thì atomic — nhưng cái vỏ `shared_ptr` và cái ruột object thì không."* Chi phí kèm theo: mỗi copy/huỷ là một thao tác atomic (bus lock) → đừng truyền `shared_ptr` by value trong hot path, dùng `const&`.
</details>

#### CPP-025 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**Circular reference là gì, gây hậu quả gì, fix ra sao?**
<details><summary>Đáp án</summary>

**Cơ chế:** hai object cùng giữ `shared_ptr` trỏ về nhau. Object chỉ bị huỷ khi **strong count về 0**, nhưng ở đây mỗi bên đang được bên kia giữ:

```
A.strong = 1 (do B giữ)      B.strong = 1 (do A giữ)
Thả hết shared_ptr ngoài  →  count mỗi bên vẫn = 1 → không ai chết → LEAK vĩnh viễn
```

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;   // ❌ chiều ngược cũng "sở hữu" -> vòng
};
a->next = b;  b->prev = a;        // thả a, b -> cả hai đều KHÔNG được huỷ

struct Node {
    std::shared_ptr<Node> next;   // ✅ chiều "xuôi" sở hữu
    std::weak_ptr<Node>   prev;   // ✅ chiều "ngược" chỉ quan sát -> phá vòng
};
if (auto p = node->prev.lock()) p->use();   // truy cập qua lock()
```

**Cách quyết định chiều nào dùng `weak_ptr`** — hỏi *"ai giữ ai sống?"*: parent sở hữu child → `parent.children` là `shared_ptr`, `child.parent` là `weak_ptr`. Observer không giữ subject sống → observer list nên là `weak_ptr`. Cache không nên giữ object sống → `weak_ptr`.

**Bẫy:** (1) tưởng chỉ vòng 2 object mới lỗi — vòng **A→B→C→A** cũng vậy, và khó thấy hơn nhiều; (2) tự trỏ chính mình (`this` lưu vào member `shared_ptr`) cũng là vòng độ dài 1; (3) **`shared_ptr` không phải GC** — GC dò được cycle, ref counting thì **không**; (4) leak này Valgrind/ASan báo là "still reachable"/leak nhưng không chỉ ra *vì sao*, phải tự soi đồ thị sở hữu.

**Chốt:** *"Ref counting không phá được chu trình. Vẽ đồ thị sở hữu, và mọi cạnh 'ngược' phải là `weak_ptr`."*
</details>

#### CPP-026 · 🟡 · concept · [→ 02-modern-cpp](../../../02-modern-cpp/)
**Nêu vài kỹ thuật C++17 bạn hay dùng.**
<details><summary>Đáp án</summary>

`std::optional` (giá trị có thể vắng, thay sentinel), `std::variant` + `std::visit` (union an toàn kiểu), `std::string_view` (cửa sổ chỉ-đọc, không copy — hợp embedded), structured bindings (`auto [a,b]=...`), `if constexpr` (rẽ nhánh compile-time trong template).
</details>

#### CPP-027 · 🟡 · concept · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**RAII giúp exception safety thế nào?**
<details><summary>Đáp án</summary>

**Cơ chế — stack unwinding.** Khi một exception được ném, runtime "tháo" ngăn xếp từng frame một cho tới khi tìm được `catch` phù hợp. Trên đường tháo, nó gọi **destructor của mọi object đã khởi tạo đầy đủ** trong các scope bị thoát. Vậy nên tài nguyên nằm trong object RAII **được trả tự động** trên cả đường thoát bất thường — thứ mà `delete`/`unlock()` viết tay luôn bỏ sót.

```cpp
// ❌ Không exception-safe: risky() ném -> rò cả bộ nhớ lẫn lock
void f() {
    Widget* w = new Widget;
    m.lock();
    risky();
    m.unlock(); delete w;
}

// ✅ Exception-safe, và ngắn hơn
void f() {
    auto w = std::make_unique<Widget>();
    std::lock_guard<std::mutex> lk(m);
    risky();          // ném -> unwinding gọi ~lock_guard rồi ~unique_ptr
}
```

**Ba mức bảo đảm (thuật ngữ interviewer hay hỏi tiếp):**

| Mức | Nghĩa |
|---|---|
| **Basic** | Có exception thì không leak, object vẫn ở trạng thái hợp lệ (có thể đã đổi) |
| **Strong** | Hoặc thành công, hoặc **nguyên trạng như chưa gọi** (commit-or-rollback). Kỹ thuật: **copy-and-swap** |
| **`noexcept`** | Cam kết không ném. Bắt buộc thực chất cho **move ctor** — `vector` chỉ dám dùng move khi realloc nếu move ctor là `noexcept`, không thì âm thầm rơi về **copy** |

**Bẫy:** (1) **destructor không được ném** — nếu ném trong lúc unwinding → `std::terminate` ngay; dtor mặc định là `noexcept`; (2) `new` trần trong danh sách tham số hàm vẫn có thể rò (thứ tự đánh giá) → luôn `make_unique`/`make_shared`; (3) "dự án em tắt exception (`-fno-exceptions`), embedded mà" — vẫn nên RAII, vì nó còn lo cả đường `return` sớm.

**Chốt:** *"Stack unwinding gọi destructor trên mọi đường thoát → tài nguyên nằm trong object RAII thì không thể rò. Đó là lý do C++ không có `finally`."* Nền tảng chung xem [CPP-005](cpp.md).
</details>

#### CPP-028 · 🟢 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**`enum` và `enum class` khác nhau? Vì sao ưu tiên `enum class`?**
<details><summary>Đáp án</summary>

`enum` cũ: tên hằng "rò" ra scope bao ngoài (dễ đụng tên), **ngầm chuyển sang int** (dễ so sánh nhầm hai enum khác loại), kiểu nền không kiểm soát. `enum class` (scoped): tên nằm trong scope enum (`Color::Red`), **không ngầm chuyển sang số** (phải `static_cast`), chỉ định được kiểu nền (`enum class E : uint8_t` — hữu ích embedded để cố định kích thước). Ưu tiên `enum class` cho an toàn kiểu + kiểm soát layout.
</details>

#### CPP-029 · 🟡 · concept · [→ move-semantics](../../../02-modern-cpp/move-semantics.md)
**`emplace_back` khác `push_back` thế nào? Khi nào emplace KHÔNG lợi, khi nào push_back tốt hơn?**
<details><summary>Đáp án</summary>

**Khác biệt cốt lõi:** `push_back` nhận một **object đã dựng** rồi copy/move nó vào container. `emplace_back` nhận **đối số của constructor**, forward (perfect forwarding) vào để **dựng object thẳng trong bộ nhớ container** — bỏ qua bước tạo object tạm.

```cpp
struct Foo { Foo(int, const std::string&); };
std::vector<Foo> v;
v.push_back(Foo(1, "a"));   // dựng temporary Foo -> move vào vector
v.emplace_back(1, "a");     // dựng thẳng trong vector, KHÔNG có temporary
```

**Bốn tình huống:**

1. **emplace LỢI** — khi truyền **đối số constructor** của object đắt (như trên): tiết kiệm một lần dựng + move temporary.

2. **HÒA (emplace không nhanh hơn)** — khi bạn đã có sẵn một object: cả hai đều phải copy/move nó, không có temporary nào để tiết kiệm.
   ```cpp
   Foo f(1, "a");
   v.push_back(f);              // copy f
   v.emplace_back(f);           // cũng copy f — y hệt
   v.push_back(std::move(f));   // move
   v.emplace_back(std::move(f));// cũng move — y hệt
   ```
   Trường hợp này chọn `push_back` cho **rõ ý** hơn.

3. **push_back AN TOÀN/ĐÚNG Ý hơn** — vì `emplace_back` dựng qua **direct-initialization** nên **bỏ qua `explicit`** và ép kiểu "ngầm" mà push_back sẽ chặn:
   ```cpp
   std::vector<std::vector<int>> vv;
   vv.emplace_back(10);   // ✅ compile: gọi vector(size_t)=10 phần tử — thường NGOÀI Ý MUỐN!
   vv.push_back(10);      // ❌ compile error: chặn vì vector(size_t) là explicit — an toàn hơn
   ```
   `emplace` "quá dễ dãi" có thể lặng lẽ dựng nhầm object. Với resource-owning: `v.emplace_back(new T)` compile được (ctor `unique_ptr(T*)` là explicit) nhưng dễ **leak nếu reallocate ném** giữa chừng — ưu tiên `push_back(std::make_unique<T>())`.

4. **push_back BẮT BUỘC** — với **braced-init-list** `{...}`: emplace không suy được kiểu từ `{}`.
   ```cpp
   std::vector<std::vector<int>> vv;
   vv.push_back({1, 2, 3});    // ✅ ok: {1,2,3} là initializer_list<int>
   vv.emplace_back({1, 2, 3}); // ❌ compile error: không suy được kiểu đối số
   ```

**Chốt:** mặc định dùng `push_back` cho **rõ ràng + an toàn kiểu**; dùng `emplace_back` khi thật sự truyền **đối số constructor** của object đắt để tránh temporary. Đừng "emplace mọi nơi" — nó không luôn nhanh hơn và có thể che lỗi ép kiểu.
</details>

#### CPP-030 · 🟡 · concept · [→ complexity-and-structures](../../../13-dsa/complexity-and-structures.md)
**`std::vector`: phân biệt `size` và `capacity`; `reserve` để làm gì; iterator invalidation là gì?**
<details><summary>Đáp án</summary>

`size` = số phần tử đang có; `capacity` = số chỗ đã cấp (≥ size). Khi `size` chạm `capacity`, push_back cấp vùng mới (thường gấp đôi) + move toàn bộ. `reserve(n)` cấp trước capacity để tránh reallocate lặp (biết trước số lượng → 1 lần cấp). **Iterator/pointer/reference invalidation:** khi vector reallocate (hoặc chèn/xóa), mọi iterator/con trỏ vào phần tử cũ **trở nên vô hiệu** (trỏ bộ nhớ đã giải phóng) — dùng tiếp là UB. Đây là bug kinh điển khi giữ con trỏ vào phần tử rồi push_back thêm.
</details>

#### CPP-031 · 🟡 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**`const` member function nghĩa là gì? `mutable` dùng khi nào?**
<details><summary>Đáp án</summary>

Hàm thành viên `const` cam kết **không sửa trạng thái quan sát được** của object (`this` là con trỏ tới const) → gọi được trên object const, và là một phần của const-correctness (biên dịch chặn sửa nhầm). `mutable` cho phép một data member **vẫn sửa được trong hàm const** — dùng cho trạng thái *không thuộc giá trị logic* của object: cache/memo, mutex bảo vệ (khóa mutex trong hàm `const` getter thread-safe), bộ đếm thống kê.
</details>

#### CPP-032 · 🟡 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**`explicit` để làm gì? Nêu một bug do implicit conversion.**
<details><summary>Đáp án</summary>

`explicit` chặn **chuyển đổi ngầm** qua constructor một-đối-số (và conversion operator). Không có nó, `Widget w = 42;` hoặc gọi `f(Widget)` bằng `f(42)` sẽ ngầm tạo `Widget(42)` — dễ ngoài ý muốn. Ví dụ bug kinh điển: `std::vector<int> v(10)` (10 phần tử) vs `v = 10` bị chặn nhờ explicit; hoặc một hàm nhận `String` bị gọi nhầm với con số. Quy tắc: ctor một đối số **mặc định để `explicit`** trừ khi thật sự muốn cho chuyển ngầm.
</details>

#### CPP-033 · 🟡 · concept · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**`new`/`delete` khác `malloc`/`free` thế nào?**
<details><summary>Đáp án</summary>

`malloc`/`free` chỉ cấp/nhả **bộ nhớ thô**, không gọi ctor/dtor, trả `void*`, báo lỗi bằng `NULL`. `new` cấp bộ nhớ **rồi gọi constructor**, trả đúng kiểu, ném `std::bad_alloc` khi thất bại; `delete` gọi **destructor** rồi nhả. Không được trộn (`free` một con trỏ `new`, hay `delete` một con trỏ `malloc`) → UB. Còn `new[]`/`delete[]` phải đi cặp. Trong C++ hiện đại: hầu như không gọi `new`/`delete` trực tiếp — dùng smart pointer/container.
</details>

#### CPP-034 · 🟠 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**`dynamic_cast` khác `static_cast` thế nào? RTTI là gì, chi phí ra sao?**
<details><summary>Đáp án</summary>

`static_cast`: ép kiểu **lúc compile**, không kiểm tra runtime — nhanh, nhưng downcast sai kiểu là UB. `dynamic_cast`: downcast trong cây kế thừa **có kiểm tra runtime** qua **RTTI** (Run-Time Type Information, đọc từ vtable) — trả `nullptr` (con trỏ) hoặc ném `std::bad_cast` (reference) nếu kiểu thực không khớp; chỉ dùng được với class **đa hình** (có virtual). Chi phí: tra RTTI lúc runtime (chậm hơn), tăng kích thước binary. Embedded/hot path thường tránh; nhiều dự án build `-fno-rtti`. Cần rẽ theo kiểu con thường xét lại thiết kế (virtual dispatch / `std::variant`+`visit`).
</details>

#### CPP-035 · 🟠 · concept · [→ oop](../../../01-cpp-fundamentals/oop.md)
**Vì sao KHÔNG nên gọi hàm virtual trong constructor/destructor?**
<details><summary>Đáp án</summary>

Trong ctor/dtor của Base, object **chưa/đã không còn là** Derived: vptr trỏ vtable của **Base** tại thời điểm đó → lời gọi virtual chạy phiên bản **Base**, không phải override của Derived, kể cả khi bạn đang tạo/hủy một Derived. Không phải bug cú pháp mà là ngữ nghĩa "object được xây từ base lên, hủy từ derived xuống". Nếu logic cần hành vi Derived lúc khởi tạo → dùng hàm init gọi sau khi construct xong, hoặc factory + two-phase init.
</details>

#### CPP-036 · 🟠 · concept · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Undefined behavior là gì? Kể vài ví dụ và vì sao nó nguy hiểm hơn "chỉ là bug".**
<details><summary>Đáp án</summary>

UB = tình huống chuẩn C++ **không định nghĩa hành vi** → compiler được phép giả định nó *không bao giờ xảy ra* và tối ưu dựa trên giả định đó. Ví dụ: đọc biến chưa khởi tạo, truy cập ngoài mảng, dereference nullptr/dangling, signed overflow, data race, dùng object sau khi hủy, vi phạm strict aliasing. Nguy hiểm vì: (1) có thể "chạy đúng" lúc dev rồi hỏng ở production/optimizer khác (Heisenbug); (2) optimizer có thể **xóa cả nhánh kiểm tra** (vd bỏ `if (p==null)` sau khi đã deref `p`) → lỗi lan xa nguồn. Phòng: bật `-fsanitize=undefined,address`, `-Wall -Wextra`, tránh cấu trúc UB ngay từ thiết kế.
</details>

#### CPP-037 · 🟡 · concept · [→ 02-modern-cpp](../../../02-modern-cpp/)
**`std::string_view` lợi gì? Bẫy nguy hiểm nhất?**
<details><summary>Đáp án</summary>

`string_view` là **cửa sổ chỉ-đọc** (con trỏ + độ dài) vào chuỗi ký tự có sẵn — truyền tham số chuỗi **không copy, không cấp phát**, nhận được cả `std::string`, C-string, buffer (rất hợp embedded/parse). Bẫy chí mạng: nó **không sở hữu** dữ liệu → nếu chuỗi gốc bị hủy/đổi (vd view vào một `std::string` tạm, hoặc trả `string_view` trỏ vào biến local) → **dangling**, UB. Quy tắc: dùng cho tham số sống ngắn; **không lưu lại** string_view vượt vòng đời nguồn; cẩn thận `string_view` không đảm bảo kết thúc `'\0'` (không truyền thẳng vào API C mong C-string).
</details>

#### CPP-038 · 🟠 · concept · ⭐ · [→ constraints](../../../08-embedded-systems/constraints.md)
**Alignment và padding là gì? `alignas`/`alignof` dùng khi nào? (góc embedded)**
<details><summary>Đáp án</summary>

Mỗi kiểu có **alignment** (địa chỉ phải chia hết cho N) do phần cứng yêu cầu/tối ưu truy cập. Compiler chèn **padding** giữa/sau member để mỗi member đúng alignment → `sizeof(struct)` lớn hơn tổng member; **thứ tự khai báo member ảnh hưởng kích thước** (sắp từ lớn→nhỏ giảm padding). `alignof(T)` truy vấn; `alignas(N)` ép alignment mạnh hơn — dùng cho: buffer DMA (thường cần căn cache line 64B), tránh **false sharing** (đệm biến giữa các core ra 2 cache line), MMIO/struct ánh xạ phần cứng, SIMD. Embedded còn quan tâm: đọc dữ liệu chưa căn lề (misaligned) có thể **fault** trên ARM cũ, và struct trao đổi qua wire cần `#pragma pack`/serialize thủ công vì layout không portable.
</details>

#### CPP-039 · 🔴 · concept · [→ templates](../../../01-cpp-fundamentals/templates.md)
**SFINAE là gì? C++20 concepts thay thế nó ra sao?**
<details><summary>Đáp án</summary>

**SFINAE** ("Substitution Failure Is Not An Error"): khi thay kiểu vào template mà tạo ra kiểu không hợp lệ ở phần chữ ký, overload đó **bị loại khỏi tập chọn lặng lẽ** thay vì lỗi biên dịch — dùng (qua `std::enable_if`, `void_t`, decltype) để **bật/tắt overload theo tính chất kiểu** (vd chỉ nhận kiểu số học). Nhược: cú pháp khó đọc, thông báo lỗi khủng khiếp. **Concepts (C++20)** làm cùng việc nhưng khai báo *ràng buộc* rõ ràng, đọc được (`template<std::integral T>` hoặc `requires`), thông báo lỗi gọn ("T không thỏa concept X"), và chọn overload theo mức ràng buộc chặt hơn. Concepts là cách hiện đại thay cho phần lớn SFINAE.
</details>

---
## Từ *Effective Modern C++* (Scott Meyers) — track `emc`

> Neo theo **Item** của sách; link nguồn tới bản summary [effective-modern-cpp.md](../../../16-book-summaries/effective-modern-cpp.md) để đào sâu.

#### CPP-040 · 🟠 · concept · ⭐ · [→ EMC Item 1–2](../../../16-book-summaries/effective-modern-cpp.md)
**`auto` suy luận kiểu theo quy tắc nào? `auto&&` là gì?**
<details><summary>Đáp án</summary>

`auto` dùng **cùng quy tắc suy luận với template** (trừ một ngoại lệ): (1) khai báo là `auto` (by value) → **bỏ ref/const/volatile top-level** (`const int& → int`); (2) `auto&`/`const auto&` → giữ, không bỏ const; (3) `auto&&` = **universal reference** → bám value category của biểu thức (lvalue→lvalue ref, rvalue→rvalue ref, reference collapsing). Ngoại lệ duy nhất so với template: `auto x = {1,2,3}` suy ra `std::initializer_list<int>` (template thì fail). Hệ quả thực dụng: `for (auto x : v)` copy; `for (const auto& x : v)` mượn; `for (auto&& x : v)` forward được (đúng cho generic).
</details>

#### CPP-041 · 🟠 · concept · [→ EMC Item 5–6](../../../16-book-summaries/effective-modern-cpp.md)
**`auto` lợi gì? Khi nào nó "phản chủ" với proxy type (vd `vector<bool>`)?**
<details><summary>Đáp án</summary>

Lợi: buộc khởi tạo, tránh chuyển kiểu ngầm/thu hẹp ngoài ý, gõ gọn, tự đúng khi kiểu đổi (vd `size_t` thay vì `int` cho `.size()`). Bẫy **invisible proxy type**: `std::vector<bool>::operator[]` trả một **proxy** (`reference` giả), không phải `bool&`. `auto b = v[i];` → `b` bắt lấy proxy, không phải `bool`; khi vector bị hủy/đổi, proxy **dangling** → dùng `b` là UB. Cách chữa: **explicitly typed initializer idiom** — `auto b = static_cast<bool>(v[i]);` (ép về kiểu thật ngay). Bài học: cẩn thận `auto` với các API trả proxy (expression templates, `bitset::reference`…).
</details>

#### CPP-042 · 🟠 · concept · [→ EMC Item 7](../../../16-book-summaries/effective-modern-cpp.md)
**Khởi tạo bằng `{}` khác `()` thế nào? `std::initializer_list` gây bất ngờ gì?**
<details><summary>Đáp án</summary>

`{}` (braced/uniform init): dùng được ở mọi ngữ cảnh, **cấm narrowing** (`int x{2.5};` lỗi biên dịch — tốt cho an toàn), và tránh **most vexing parse** (`Widget w();` bị hiểu là khai báo hàm; `Widget w{};` thì không). Bất ngờ: nếu class có constructor nhận `std::initializer_list`, **`{}` sẽ ưu tiên gọi nó** kể cả khi constructor khác khớp hơn — `std::vector<int> v{10, 2}` tạo vector **{10, 2}** (2 phần tử) chứ không phải 10 phần tử giá trị 2 (`v(10, 2)`). Quy tắc: biết class bạn dùng có ctor initializer_list không; chọn `{}` vs `()` có chủ đích.
</details>

#### CPP-043 · 🟡 · concept · [→ EMC Item 8](../../../16-book-summaries/effective-modern-cpp.md)
**Vì sao dùng `nullptr` thay cho `0`/`NULL`?**
<details><summary>Đáp án</summary>

`0` là int, `NULL` thường là `0`/`0L` (kiểu integer) → khi overload có cả `f(int)` và `f(void*)`, truyền `NULL` gọi nhầm `f(int)`, không phải bản con trỏ. `nullptr` có kiểu riêng `std::nullptr_t`, chuyển ngầm sang **mọi kiểu con trỏ nhưng không sang integer** → chọn đúng overload con trỏ, và làm code template (suy luận kiểu) đúng ý. Cũng rõ nghĩa hơn khi đọc. Luôn dùng `nullptr`.
</details>

#### CPP-044 · 🟡 · concept · [→ EMC Item 9](../../../16-book-summaries/effective-modern-cpp.md)
**`using` (alias declaration) hơn `typedef` chỗ nào?**
<details><summary>Đáp án</summary>

`using Vec = std::vector<int>;` tương đương `typedef` cho ca thường, nhưng đọc xuôi hơn và — quan trọng — hỗ trợ **alias template**: `template<class T> using MyVec = std::vector<T, MyAlloc<T>>;` (typedef không làm template được). Alias template dùng thẳng không cần `::type`; còn nếu dùng traits kiểu cũ phải `typename SomeTrait<T>::type`. C++11 alias template là nền của các `_t` trong chuẩn (`std::enable_if_t`, `std::remove_reference_t`). Ưu tiên `using`.
</details>

#### CPP-045 · 🟡 · concept · [→ EMC Item 11](../../../16-book-summaries/effective-modern-cpp.md)
**`= delete` khác cách cũ (khai báo private không định nghĩa) thế nào?**
<details><summary>Đáp án</summary>

Cách cũ (C++98) cấm copy: khai báo copy ctor/assign **private + không định nghĩa** → dùng nhầm chỉ lỗi lúc **link** (hoặc runtime với friend/member), thông báo mơ hồ. `= delete` (C++11): hàm **tồn tại nhưng bị xóa** → mọi lời gọi lỗi ngay lúc **compile**, rõ ràng; đặt `public` để thông báo lỗi đẹp hơn. Còn mạnh hơn: `delete` được cho **hàm bất kỳ** (không chỉ special members) và **template instantiation cụ thể** — vd cấm gọi một overload với kiểu nhất định (`void f(char) = delete;` chặn ép ngầm). Luôn dùng `= delete`.
</details>

#### CPP-046 · 🟡 · concept · [→ EMC Item 12](../../../16-book-summaries/effective-modern-cpp.md)
**`override` (và `final`) để làm gì? Vì sao nên viết?**
<details><summary>Đáp án</summary>

Không viết `override`, một hàm định "override" mà **lệch chữ ký** (sai const, sai kiểu tham số, quên là hàm base không virtual) sẽ âm thầm tạo **hàm mới** thay vì override → đa hình không chạy như mong, không báo lỗi. `override` bảo compiler **kiểm tra**: nếu không thực sự override một virtual của base → lỗi biên dịch. `final` chặn override tiếp (trên hàm) hoặc kế thừa tiếp (trên class), và cho compiler tối ưu devirtualize. Luôn đánh `override` cho hàm override — rẻ, bắt bug sớm.
</details>

#### CPP-047 · 🟠 · concept · ⭐ · [→ EMC Item 21](../../../16-book-summaries/effective-modern-cpp.md)
**Vì sao ưu tiên `make_unique`/`make_shared` thay vì `new` trực tiếp?**
<details><summary>Đáp án</summary>

(1) **Exception safety:** `f(std::shared_ptr<T>(new T), g())` — nếu `new T` xong, `g()` ném *trước khi* shared_ptr được dựng → leak. `make_shared` gói lại thành một bước, không hở. (2) **Ít lặp code + không viết `new`** (đúng tinh thần không raw new). (3) `make_shared` cấp **một lần** cho cả object + control block (thay vì 2 lần với `new`) → nhanh hơn, ít phân mảnh. Ngoại lệ **không** dùng make: cần custom deleter; cần truyền braced-init-list; với `make_shared`, object + control block chung một khối nên `weak_ptr` còn sống thì **bộ nhớ object chưa được nhả** (cân nhắc khi object lớn + weak_ptr sống lâu); và không dùng được khi ctor cần là private.
</details>

#### CPP-048 · 🟠 · concept · [→ EMC Item 17](../../../16-book-summaries/effective-modern-cpp.md)
**Compiler tự sinh special member function khi nào? Điều gì chặn sinh move?**
<details><summary>Đáp án</summary>

Mặc định compiler sinh: default ctor (nếu không khai ctor nào), copy ctor, copy assign, destructor, và (C++11) move ctor + move assign. Nhưng có ràng buộc chéo: **khai báo bất kỳ move nào → copy bị xóa**; **khai báo copy hoặc destructor → move KHÔNG được sinh** (fall back sang copy!) — đây là lý do một class có destructor thủ công tự nhiên "mất" move, âm thầm copy thay vì move (mất hiệu năng). Rule of Five/Rule of Zero sinh ra chính vì mạng lưới quy tắc này. Muốn giữ đủ: `= default` tường minh cả 5 (hoặc dùng Rule of Zero — không viết cái nào). *(Liên quan [CPP-020](cpp.md).)*
</details>

#### CPP-049 · 🟠 · concept · [→ EMC Item 29](../../../16-book-summaries/effective-modern-cpp.md)
**Move có phải luôn rẻ hơn copy? Kể ca move không giúp gì.**
<details><summary>Đáp án</summary>

Không. Move chỉ rẻ khi object **sở hữu tài nguyên trỏ gián tiếp** (heap buffer) để "cướp" con trỏ. Ca move ≈ copy: (1) kiểu chứa dữ liệu **tại chỗ** như `std::array<T,N>` — move vẫn phải move từng phần tử, O(n); (2) `std::string` với **SSO** (small string optimization) — chuỗi ngắn nằm trong object, move = copy; (3) kiểu **không có move** (chỉ copy) hoặc move không `noexcept` (container sẽ copy). Bài học: đừng giả định `std::move` luôn cho tốc độ; với kiểu không rõ hoặc generic, move có thể không nhanh hơn — đo khi quan trọng.
</details>

#### CPP-050 · 🔴 · concept · [→ EMC Item 41](../../../16-book-summaries/effective-modern-cpp.md)
**Khi nào nên nhận tham số **by value** thay vì overload lvalue/rvalue hay universal ref?**
<details><summary>Đáp án</summary>

Với tham số **copyable, rẻ để move, và luôn được copy/lưu lại** (vd setter `void setName(std::string n) { name_ = std::move(n); }`): nhận **by value** rồi `std::move` vào đích. Lý do: một hàm phục vụ cả lvalue (copy vào tham số) lẫn rvalue (move vào tham số) mà chỉ viết **một** overload, gọn hơn cặp `const&`/`&&` và tránh phình template của universal ref. Đánh đổi: tốn thêm *một* move so với overload tối ưu; **không** áp cho kiểu move đắt (như `std::array`), kiểu chỉ-copy, hay khi không thực sự lưu lại tham số (chỉ đọc → dùng `const&`). Cân nhắc theo "copyable + cheap-to-move + sink parameter".
</details>

#### CPP-051 · 🟠 · concept · [→ EMC Item 35](../../../16-book-summaries/effective-modern-cpp.md)
**`std::async` (task-based) hơn tự tạo `std::thread` (thread-based) ở điểm nào? Lưu ý `std::launch`?**
<details><summary>Đáp án</summary>

`std::async` trả `std::future` và để runtime lo: (1) **lấy giá trị trả về / lan truyền exception** qua future (thread thô không có kênh này — exception thoát thread = `std::terminate`); (2) **quản lý số thread / oversubscription** (có thể chạy trên thread pool, hoặc chạy đồng bộ khi tài nguyên cạn) thay vì bạn tự cân; (3) không phải lo `join`/`detach`. Lưu ý `std::launch`: mặc định là `async | deferred` → có thể **chạy hoãn** (chỉ chạy khi `.get()`), khiến code phụ thuộc "chạy song song ngay" sai. Muốn chắc song song: `std::async(std::launch::async, f)`. Task-based hợp khi cần *kết quả tính toán*; thread-based chỉ khi cần điều khiển thread mức thấp (affinity, priority, RT).
</details>

#### CPP-052 · 🟠 · concept · ⭐ · 🎤 2026-08-05 · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md), [concurrency](../../../02-modern-cpp/concurrency.md)
**Hai thread cùng gán vào *cùng một object* `shared_ptr` (`p = other;`) — có an toàn không? Phân biệt các tầng thread-safety của `shared_ptr`.**
<details><summary>Đáp án</summary>

**Không an toàn.** Phải tách **ba tầng**, chỉ tầng đầu mới được chuẩn bảo đảm:

| Tầng | Thread-safe? |
|---|---|
| **Control block** (strong/weak count) | ✅ đếm bằng atomic → nhiều thread **copy/destroy các instance khác nhau** cùng trỏ 1 object là an toàn |
| **Bản thân object `shared_ptr`** (2 con trỏ: payload ptr + control block ptr) | ❌ ghi đồng thời vào **cùng một instance** = data race |
| **Object được trỏ tới** (payload) | ❌ không được bảo vệ — cần mutex riêng |

Vì sao tầng 2 hỏng: `p = other` **không** phải một thao tác atomic — nó giảm count cũ, ghi **hai** con trỏ trong `p`, tăng count mới. Hai thread chạy xen kẽ có thể để `p` trỏ payload của A nhưng control block của B → double-free / leak. Bản thân các *counter* vẫn atomic, nên "count là atomic" **không** cứu được.

Cách đúng khi thật sự cần chia sẻ một biến `shared_ptr` giữa các thread: `std::atomic<std::shared_ptr<T>>` (C++20), `std::atomic_load/atomic_store` trên `shared_ptr` (C++11, deprecated từ C++20), hoặc bọc mutex. Thiết kế tốt hơn: mỗi thread giữ **bản copy riêng** của `shared_ptr` (copy là an toàn) thay vì cùng ghi vào một instance.

**Chốt:** *"Đếm thì atomic — nhưng cái vỏ `shared_ptr` và cái ruột object thì không."*
</details>

---
⬅️ [Bank index](README.md)
