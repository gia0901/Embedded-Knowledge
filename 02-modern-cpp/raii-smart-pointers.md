# RAII & Smart Pointers — Quản lý tài nguyên an toàn

> **TL;DR**
> - **RAII** (Resource Acquisition Is Initialization): gắn vòng đời tài nguyên (memory, file, lock, socket...) vào vòng đời một object. Constructor giành tài nguyên, destructor trả lại → tự động, an toàn ngoại lệ (exception-safe).
> - **`unique_ptr`**: sở hữu độc quyền, không copy được (chỉ move), zero overhead. Dùng mặc định.
> - **`shared_ptr`**: sở hữu chia sẻ qua reference count (atomic). Tốn bộ nhớ + chi phí đếm. Chỉ dùng khi *thật sự* cần nhiều owner.
> - **`weak_ptr`**: tham chiếu *không sở hữu*, phá vỡ chu trình tham chiếu (circular reference) của `shared_ptr`.
> - **Rule of 0**: thiết kế để không cần tự viết destructor/copy/move (nhờ RAII member). Nếu phải viết một trong số đó → cân nhắc viết đủ (Rule of 5).

---

## 1. RAII — ý tưởng cốt lõi của C++

Vấn đề: cấp phát tài nguyên thủ công dễ rò rỉ khi có nhiều đường return hoặc exception.

```cpp
void bad() {
    int* p = new int[100];
    if (something()) return;   // ❌ leak: quên delete
    risky();                   // ❌ nếu ném exception → leak
    delete[] p;
}
```

RAII gói tài nguyên vào object trên **stack**; khi object ra scope (kể cả do exception), destructor **luôn** chạy → trả tài nguyên.

```cpp
void good() {
    std::vector<int> v(100);   // RAII: tự cấp & tự giải phóng
    if (something()) return;   // ✅ vector tự dọn
    risky();                   // ✅ exception → destructor vẫn chạy
}                              // ✅ giải phóng tại đây
```

RAII áp dụng cho **mọi** tài nguyên cặp đôi acquire/release: `std::lock_guard` (mutex), `std::fstream` (file), smart pointer (heap)... Đây là lý do C++ hiếm khi cần `finally` như Java.

---

## 2. `std::unique_ptr` — sở hữu độc quyền

```cpp
#include <memory>

auto p = std::make_unique<Widget>(args);  // ưu tiên make_unique (C++14)
p->doSomething();
// tự delete khi p ra scope — không cần delete thủ công

// Không copy được:
// auto q = p;            // ❌ lỗi biên dịch
auto q = std::move(p);    // ✅ chuyển quyền sở hữu; p giờ == nullptr
```

- **Zero overhead**: kích thước bằng một con trỏ thô, không chi phí runtime.
- Truyền quyền sở hữu bằng `std::move`. Trả về từ factory function rất tự nhiên.
- Hỗ trợ custom deleter (vd đóng `FILE*`, `fclose`).
- **Mặc định nên dùng `unique_ptr`**; chỉ nâng lên `shared_ptr` khi cần.

---

## 3. `std::shared_ptr` — sở hữu chia sẻ

```cpp
auto a = std::make_shared<Widget>();  // ref count = 1
{
    auto b = a;                       // ref count = 2 (copy → tăng đếm)
}                                     // b ra scope → ref count = 1
// khi ref count == 0 → object bị hủy
```

- Quản lý **control block** chứa **reference count** (và weak count). Đếm tăng/giảm **atomic** → thread-safe cho việc đếm (không tự động an toàn cho dữ liệu bên trong!).
- **Chi phí:** object to gấp đôi con trỏ thường (2 con trỏ), thêm cấp phát control block, thao tác atomic mỗi lần copy/destroy.
- **`make_shared` tốt hơn** `shared_ptr<T>(new T)`: 1 lần cấp phát thay vì 2, exception-safe.

> ⚠️ Đừng lạm dụng `shared_ptr` như "con trỏ cho tiện". Sở hữu chia sẻ nghĩa là *không ai biết chắc ai dọn dẹp* → khó suy luận. Chỉ dùng khi vòng đời thật sự chia sẻ giữa nhiều owner độc lập.

---

## 4. `std::weak_ptr` — quan sát không sở hữu

Vấn đề **circular reference**: hai `shared_ptr` trỏ lẫn nhau → ref count không bao giờ về 0 → **leak**.

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev;   // ❌ chu trình → leak
};
```

Giải: một chiều dùng `weak_ptr` (không tăng ref count):

```cpp
struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev;   // ✅ không sở hữu → không tạo chu trình
};

// Dùng weak_ptr: phải lock() để lấy shared_ptr tạm thời, kiểm tra còn sống
if (auto sp = node.prev.lock()) {   // sp là shared_ptr hoặc nullptr
    sp->doSomething();
}
```

`weak_ptr` cũng dùng cho cache/observer: muốn tham chiếu object *nếu nó còn tồn tại*, không kéo dài vòng đời của nó.

---

## 5. So sánh nhanh

| | `unique_ptr` | `shared_ptr` | `weak_ptr` |
|--|-------------|--------------|------------|
| Sở hữu | Độc quyền | Chia sẻ | Không sở hữu |
| Copy được? | Không (chỉ move) | Có | Có |
| Overhead | Zero | Ref count (atomic) + control block | Theo control block |
| Dùng khi | Mặc định | Nhiều owner thật sự | Phá vòng / quan sát |

---

## 6. Rule of 0 / 3 / 5

**5 special member functions:** destructor, copy ctor, copy assignment, move ctor, move assignment.

Tóm tắt định hướng trước khi vào chi tiết:

| Quy tắc | Nói về điều gì | Khi nào áp |
|---|---|---|
| **Rule of 3** | **ĐÚNG/SAI** — tránh leak, double-free, aliasing | class ôm tài nguyên thô, cần copy |
| **Rule of 5** | **NHANH/CHẬM** — thêm đường move O(1) thay vì copy O(n) | như trên + quan tâm hiệu năng |
| **Rule of 0** | **không phải viết gì** — để RAII member lo | mặc định nên hướng tới |

### 6.1. Rule of 3 — vì sao là "bộ ba bất khả phân"

Cả 3 hàm chỉ là **3 mặt của MỘT việc: quản lý sở hữu một tài nguyên thô**. Cần tùy biến bất kỳ cái nào ⟹ class đang sở hữu tài nguyên (con trỏ heap, file handle…) mà bản mặc định của compiler xử lý sai ⟹ **cả 3 đều sai theo**.

```cpp
class Buffer {
    char*  data;
    size_t size;
public:
    Buffer(size_t n) : data(new char[n]), size(n) {}
    ~Buffer() { delete[] data; }               // (1) có tài nguyên riêng -> phải giải phóng
};
```

**Vì sao viết copy → phải có destructor?**
Bạn chỉ viết copy ctor khi bản mặc định *sai*. Copy mặc định là **shallow** — chép `data = other.data` (chép con trỏ, không chép mảng). Viết copy ctor riêng nghĩa là muốn **deep copy** (`new` mảng mới). Nhưng hễ copy có `new` → mỗi object **tự sở hữu** một mảng → **bắt buộc `delete`** ở đâu đó → cần destructor. Nói ngược lại: nếu class **không cần destructor** (không sở hữu gì) thì copy mặc định đã đúng → chẳng có lý do viết copy ctor. Vậy "cần copy riêng" ⟺ "sở hữu tài nguyên" ⟺ "cần destructor".

**Vì sao viết copy ctor → phải viết copy assignment?**
Hai hàm cùng làm "biến object này thành bản sao của object kia": copy ctor dựng object *mới*; copy assign thay nội dung object *đã tồn tại*. Nếu deep-copy ở ctor nhưng để **copy assign mặc định** (shallow):

```cpp
Buffer a(10), b(20);
a = b;   // copy assign MẶC ĐỊNH: a.data = b.data (chép con trỏ!)
```
→ (1) mảng gốc 10 byte của `a` không ai giải phóng = **leak**; (2) `a.data` và `b.data` trỏ **cùng mảng** → khi cả hai hủy = **double free** (UB), sửa `a` cũng đổi `b` (aliasing). Cùng "bệnh sở hữu con trỏ thô" khiến cả hai phải tự viết. Bản đúng của copy assign (copy-and-swap, an toàn self-assignment):

```cpp
    Buffer(const Buffer& o) : data(new char[o.size]), size(o.size) {
        std::copy(o.data, o.data + o.size, data);          // deep copy, O(n)
    }
    Buffer& operator=(const Buffer& o) {                   // copy-and-swap
        if (this != &o) { Buffer tmp(o); std::swap(data, tmp.data); std::swap(size, tmp.size); }
        return *this;   // tmp mang tài nguyên cũ ra scope -> tự dtor
    }
```

### 6.2. Rule of 5 — thêm đường "cướp tài nguyên" O(1)

Khác biệt tư duy: **Rule of 3 đã đủ ĐÚNG.** Rule of 5 chỉ thêm hiệu năng — thay vì deep-copy mỗi lần đáng ra move được, ta **cướp con trỏ + để nguồn rỗng**, O(1).

```cpp
    Buffer(Buffer&& o) noexcept : data(o.data), size(o.size) {
        o.data = nullptr;   // nguồn thôi sở hữu -> ~Buffer của nó: delete[] nullptr = an toàn no-op
        o.size = 0;         // nguồn ở trạng thái HỢP LỆ nhưng rỗng (dùng lại được)
    }
    Buffer& operator=(Buffer&& o) noexcept {
        if (this != &o) {
            delete[] data;                  // (1) giải phóng tài nguyên CŨ của mình trước
            data = o.data; size = o.size;   // (2) cướp con trỏ của nguồn
            o.data = nullptr; o.size = 0;   // (3) để nguồn rỗng
        }
        return *this;
    }
```

Tinh thần move ctor gói trong 3 dòng: **chép con trỏ → null nguồn**. Quên null nguồn → cả `this` lẫn `o` trỏ cùng mảng → **double free** khi cả hai hủy.

**Cái bẫy hay bị hỏi** — vì sao "viết Rule of 3 thì nên viết luôn Rule of 5":
> Chỉ cần bạn **khai báo destructor (hoặc copy ops)**, compiler **NGỪNG tự sinh move** ctor/assign. Class "mất" move một cách âm thầm → mọi thao tác đáng ra move **rơi về copy**.

Nên một `Buffer` chỉ có Rule of 3, khi bỏ vào `std::vector` và vector reallocate, sẽ **deep-copy từng phần tử** thay vì move — đúng nhưng chậm, không cảnh báo. Viết thêm move (Rule of 5) để khôi phục đường nhanh.

Hai điểm cộng điểm:
- **`noexcept` cho move gần như bắt buộc:** `std::vector` chỉ dùng move khi reallocate *nếu* move ctor `noexcept` (giữ strong exception guarantee); quên `noexcept` → nó lại âm thầm copy.
- **move assign có 3 việc** copy ctor không có: giải phóng tài nguyên cũ, cướp, null nguồn — cộng an toàn **self-move**.

### 6.3. Rule of 0 — đích đến

Nếu member là **RAII type**, chúng tự lo copy/move/hủy đúng → class **không sở hữu tài nguyên thô nào** → compiler sinh **cả 5** vừa đúng vừa hiệu quả → bạn **không viết dòng nào**:

```cpp
// Rule of 0: các member tự lo -> copy/move/dtor đều đúng + nhanh, miễn phí
class Config {
    std::string name_;
    std::vector<int> values_;
    std::unique_ptr<Impl> impl_;   // move-only → Config tự thành move-only
};

// Buffer ở trên viết lại theo Rule of 0:
class Buffer {
    std::vector<char> data;        // vector tự copy (deep) / move (steal) / hủy
public:
    explicit Buffer(size_t n) : data(n) {}
};
```

**Thứ tự tư duy chuẩn:** ưu tiên **Rule of 0**; chỉ khi buộc phải ôm tài nguyên thô (wrap C API, OS handle, MMIO…) mới rơi vào **Rule of 5** — và khi đó viết **đủ cả 5 + `noexcept`** cho move. Rule of 3 "trần" chỉ còn hợp lý khi thật sự không muốn/không cần move.

---

## 7. Lỗi & lưu ý hay được hỏi

- **Không** tạo 2 smart pointer từ cùng một con trỏ thô (`raw`): mỗi cái nghĩ mình sở hữu → double free.
- `unique_ptr` cho mảng: `std::unique_ptr<int[]>` (gọi `delete[]`).
- `shared_ptr` thread-safe về **đếm**, không thread-safe về **dữ liệu** trỏ tới (cần mutex riêng).
- `enable_shared_from_this` khi cần lấy `shared_ptr` tới `this` từ trong method.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [CPP-005](../14-prep/mock-interview/bank/cpp.md) | RAII là gì? Vì sao quan trọng trong C++? |
| [CPP-007](../14-prep/mock-interview/bank/cpp.md) | unique_ptr, shared_ptr, weak_ptr khác nhau và dùng khi nào? |
| [CPP-024](../14-prep/mock-interview/bank/cpp.md) | shared_ptr hoạt động bên trong thế nào? Có thread-safe không? |
| [CPP-025](../14-prep/mock-interview/bank/cpp.md) | Circular reference với shared_ptr là gì, khắc phục ra sao? |
| [CPP-020](../14-prep/mock-interview/bank/cpp.md) | Rule of 0/3/5 là gì? |
| [CPP-047](../14-prep/mock-interview/bank/cpp.md) | Vì sao nên dùng make_unique/make_shared thay vì new? |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [move-semantics.md](move-semantics.md)
