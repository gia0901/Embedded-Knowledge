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

**Special member functions:** destructor, copy ctor, copy assignment, move ctor, move assignment.

- **Rule of 3** (C++98): nếu cần tự viết 1 trong {destructor, copy ctor, copy assignment} thì thường cần viết cả 3 (vì class quản lý tài nguyên thô).
- **Rule of 5** (C++11): thêm move ctor + move assignment để hỗ trợ move semantics.
- **Rule of 0** (khuyến nghị): **thiết kế để không phải viết cái nào** — dùng member là RAII type (`vector`, `string`, smart pointer) thì compiler tự sinh đúng. Đây là mục tiêu nên hướng tới.

```cpp
// Rule of 0: không cần viết destructor/copy/move — các member tự lo
class Config {
    std::string name_;
    std::vector<int> values_;
    std::unique_ptr<Impl> impl_;   // move-only → Config tự thành move-only
};
```

---

## 7. Lỗi & lưu ý hay được hỏi

- **Không** tạo 2 smart pointer từ cùng một con trỏ thô (`raw`): mỗi cái nghĩ mình sở hữu → double free.
- `unique_ptr` cho mảng: `std::unique_ptr<int[]>` (gọi `delete[]`).
- `shared_ptr` thread-safe về **đếm**, không thread-safe về **dữ liệu** trỏ tới (cần mutex riêng).
- `enable_shared_from_this` khi cần lấy `shared_ptr` tới `this` từ trong method.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) RAII là gì? Vì sao quan trọng trong C++?</summary>

RAII = Resource Acquisition Is Initialization: ràng buộc vòng đời của một tài nguyên (heap, file, mutex, socket...) vào vòng đời một object. Constructor giành tài nguyên, destructor trả lại. Vì destructor luôn được gọi khi object ra scope — kể cả khi có exception — nên tài nguyên được giải phóng tự động, đúng đắn và exception-safe, không cần `finally`. Đây là nền tảng cho smart pointer, `lock_guard`, container... và là cách C++ tránh leak.
</details>

<details><summary>2) unique_ptr, shared_ptr, weak_ptr khác nhau và dùng khi nào?</summary>

`unique_ptr`: sở hữu độc quyền, không copy (chỉ move), zero overhead — dùng mặc định. `shared_ptr`: sở hữu chia sẻ qua reference count atomic, tốn bộ nhớ và chi phí đếm — chỉ dùng khi nhiều owner độc lập thật sự cùng quyết định vòng đời. `weak_ptr`: tham chiếu không sở hữu, không tăng ref count — dùng để phá circular reference giữa các `shared_ptr` hoặc quan sát object "nếu còn sống" (cache/observer), truy cập qua `lock()`.
</details>

<details><summary>3) shared_ptr hoạt động bên trong thế nào? Có thread-safe không?</summary>

`shared_ptr` quản lý một control block chứa strong count và weak count. Mỗi copy tăng strong count, mỗi hủy giảm; khi strong count về 0, object bị hủy; khi cả hai về 0, control block bị giải phóng. Các thao tác đếm là **atomic** nên việc copy/destroy `shared_ptr` từ nhiều thread là an toàn. Tuy nhiên đây chỉ là thread-safe cho **bộ đếm**, KHÔNG đảm bảo an toàn cho dữ liệu mà nó trỏ tới — truy cập dữ liệu đó đồng thời vẫn cần đồng bộ (mutex).
</details>

<details><summary>4) Circular reference với shared_ptr là gì, khắc phục ra sao?</summary>

Khi hai object giữ `shared_ptr` trỏ tới nhau, strong count của mỗi cái không bao giờ về 0 dù không còn ai bên ngoài tham chiếu → cả hai bị leak. Khắc phục: biến một chiều (thường là chiều "ngược"/parent) thành `weak_ptr` để không tăng strong count, phá vòng. Khi cần dùng, gọi `lock()` để lấy `shared_ptr` tạm.
</details>

<details><summary>5) Rule of 0/3/5 là gì?</summary>

Liên quan 5 special member functions (destructor, copy ctor, copy assign, move ctor, move assign). Rule of 3: nếu phải tự viết 1 trong {destructor, copy ctor, copy assign} thì thường phải viết cả 3 (class quản lý tài nguyên thô). Rule of 5: thêm move ctor và move assign cho move semantics. Rule of 0 (khuyến nghị): thiết kế để **không phải viết cái nào** bằng cách dùng member RAII (smart pointer, container); khi đó compiler tự sinh đúng và an toàn.
</details>

<details><summary>6) Vì sao nên dùng make_unique/make_shared thay vì new?</summary>

`make_shared` cấp phát object và control block trong **một** lần cấp phát (thay vì hai khi dùng `shared_ptr<T>(new T)`), nhanh hơn và locality tốt hơn. Cả `make_unique`/`make_shared` đều **exception-safe**: tránh rò rỉ trong trường hợp đánh giá tham số xen kẽ giữa `new` và constructor smart pointer. Ngoài ra code gọn, không lặp tên kiểu, và không để lộ con trỏ thô.
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [move-semantics.md](move-semantics.md)
