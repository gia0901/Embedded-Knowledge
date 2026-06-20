# Concurrency — Thread, Mutex, Atomic, Memory Order

> **TL;DR**
> - **`std::thread`**: tạo luồng; phải `join()` hoặc `detach()` trước khi hủy, nếu không → `std::terminate`.
> - **Data race** (2 thread truy cập cùng dữ liệu, ≥1 ghi, không đồng bộ) = **Undefined Behavior**. Phải bảo vệ bằng mutex hoặc atomic.
> - **`std::mutex` + `std::lock_guard`/`std::unique_lock`** (RAII) để khóa vùng tới hạn. Khóa nhiều mutex → dùng `std::scoped_lock` (C++17) tránh deadlock.
> - **`std::atomic`** cho thao tác lock-free trên biến đơn; **memory order** quy định mức độ sắp xếp lại lệnh được phép.
> - **`std::async`/`std::future`** cho mô hình tác vụ trả kết quả. **`condition_variable`** để thread chờ điều kiện.
> - Tư duy: ưu tiên **không chia sẻ trạng thái mutable**; nếu phải chia sẻ thì đồng bộ nghiêm ngặt và giữ critical section nhỏ.

---

## 1. `std::thread` cơ bản

```cpp
#include <thread>

void work(int id) { /* ... */ }

std::thread t(work, 42);   // bắt đầu chạy ngay
// ... làm việc khác song song ...
t.join();                  // chờ t kết thúc

// Hoặc detach (chạy nền, không chờ) — hiếm dùng, dễ rò:
// t.detach();
```

- Một `std::thread` bị hủy khi vẫn **joinable** (chưa join/detach) → gọi `std::terminate()` (crash). Luôn join/detach.
- Modern: dùng **`std::jthread`** (C++20) — tự join khi hủy + hỗ trợ stop token.
- Truyền tham chiếu vào thread phải dùng `std::ref`; nếu không sẽ bị copy.

---

## 2. Data race — gốc rễ của bug concurrency

```cpp
int counter = 0;
void inc() { for (int i = 0; i < 100000; ++i) ++counter; }   // ❌ data race

std::thread a(inc), b(inc);
a.join(); b.join();
// counter KHÔNG chắc = 200000 — vì ++counter không atomic (read-modify-write)
```

**Data race**: ≥2 thread truy cập cùng một vùng nhớ, ít nhất một là **ghi**, và không có đồng bộ → **Undefined Behavior** (không chỉ sai số, mà UB thực sự). Hai cách sửa: **mutex** hoặc **atomic**.

---

## 3. Mutex & RAII lock

```cpp
#include <mutex>
std::mutex mtx;
int counter = 0;

void inc() {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);   // RAII: lock khi tạo, unlock khi ra scope
        ++counter;                               // critical section
    }
}
```

| Loại | Đặc điểm |
|------|----------|
| `std::lock_guard` | Đơn giản nhất, lock toàn scope, không unlock sớm được |
| `std::unique_lock` | Linh hoạt: unlock/lock lại, defer, dùng với condition_variable |
| `std::scoped_lock` (C++17) | Khóa **nhiều** mutex cùng lúc, an toàn deadlock |
| `std::shared_mutex` (C++17) | Reader-writer: nhiều reader hoặc một writer |

**Luôn dùng RAII lock**, không `lock()`/`unlock()` thủ công (dễ quên unlock khi exception/early return).

---

## 4. Deadlock & cách tránh

**Deadlock**: 2 thread chờ lẫn nhau giải phóng mutex → kẹt vĩnh viễn.

```cpp
// Thread 1: lock(A) rồi lock(B)
// Thread 2: lock(B) rồi lock(A)   → deadlock kinh điển
```

4 điều kiện Coffman (cần đủ cả 4 để deadlock): mutual exclusion, hold-and-wait, no preemption, circular wait. Phá một điều kiện là tránh được.

Cách tránh:
- **Luôn khóa mutex theo cùng một thứ tự** (phá circular wait).
- Dùng **`std::scoped_lock(m1, m2)`** — khóa nhiều mutex bằng thuật toán tránh deadlock.
- Giữ critical section **nhỏ và ngắn**; tránh gọi code không kiểm soát khi đang giữ lock.
- Tránh nested lock khi có thể.

---

## 5. `std::atomic`

```cpp
#include <atomic>
std::atomic<int> counter{0};

void inc() {
    for (int i = 0; i < 100000; ++i)
        ++counter;          // atomic read-modify-write — không cần mutex
}
// counter == 200000 đảm bảo
```

- Thao tác trên `atomic` là **không thể chia cắt** (indivisible), không bị data race.
- Phù hợp cho biến đơn (counter, flag, con trỏ). Phức tạp hơn (cập nhật nhiều biến liên quan) → vẫn cần mutex.
- `is_lock_free()`: kiểm tra có thực sự lock-free (không khóa ngầm) — thường true cho kiểu nhỏ.
- `compare_exchange_weak/strong` (CAS): nền tảng cho cấu trúc lock-free.

---

## 6. Memory order (nâng cao nhưng hay được hỏi ở mức ý niệm)

CPU và compiler có thể **sắp xếp lại** (reorder) lệnh để tối ưu. Trong đa luồng, điều này có thể khiến thread khác thấy thứ tự ghi khác kỳ vọng. Memory order quy định ràng buộc:

| Order | Ý nghĩa | Chi phí |
|-------|---------|---------|
| `memory_order_seq_cst` | Mặc định. Thứ tự tuần tự nhất quán toàn cục — dễ suy luận nhất | Cao nhất |
| `memory_order_acquire`/`release` | Đồng bộ cặp đôi: release "công bố", acquire "nhìn thấy" — đủ cho hầu hết pattern producer/consumer | Trung bình |
| `memory_order_relaxed` | Chỉ đảm bảo atomicity, không đảm bảo thứ tự — dùng cho counter độc lập | Thấp nhất |

> Lời khuyên thực tế: **mặc định `seq_cst`** (an toàn, dễ đúng). Chỉ hạ xuống acquire/release/relaxed khi đã đo được và *thật sự hiểu* — đây là vùng rất dễ sai.

---

## 7. `condition_variable` — chờ điều kiện

```cpp
std::mutex m;
std::condition_variable cv;
std::queue<int> q;

// Consumer
void consume() {
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{ return !q.empty(); });   // nhả lock & ngủ tới khi predicate đúng
    int x = q.front(); q.pop();
}                                            // (predicate chống spurious wakeup)

// Producer
void produce(int x) {
    { std::lock_guard<std::mutex> lk(m); q.push(x); }
    cv.notify_one();                          // đánh thức 1 consumer
}
```

- Luôn dùng `cv.wait(lock, predicate)` — predicate chống **spurious wakeup** (thức dậy không lý do).
- `notify_one()` / `notify_all()`.

---

## 8. `std::async` & `std::future` — mô hình tác vụ

```cpp
#include <future>

std::future<int> fut = std::async(std::launch::async, []{
    return expensiveComputation();
});
// ... làm việc khác ...
int result = fut.get();   // chờ & lấy kết quả (block tới khi xong)
```

- Trả kết quả từ thread một cách an toàn, kể cả exception (được chuyển qua `get()`).
- Mức trừu tượng cao hơn `std::thread` — nghĩ theo "tác vụ trả giá trị" thay vì "luồng".
- `std::promise`/`std::future`: kênh truyền giá trị một lần giữa thread.

---

## 9. Tư duy concurrency đúng

1. **Tránh chia sẻ trạng thái mutable** nếu được (immutable, message passing, thread-local).
2. Nếu phải chia sẻ → **một mutex bảo vệ một nhóm dữ liệu** rõ ràng; giữ critical section nhỏ.
3. **Đừng tối ưu memory order sớm** — `seq_cst`/mutex trước, đo rồi mới hạ.
4. Cẩn thận lifetime: object bị thread tham chiếu phải sống đủ lâu.
5. Test concurrency khó → dùng **ThreadSanitizer (TSan)** để phát hiện data race.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Data race là gì? Hậu quả?</summary>

Data race xảy ra khi ≥2 thread truy cập đồng thời cùng một vùng nhớ, ít nhất một thao tác là ghi, và không có đồng bộ hóa nào giữa chúng. Theo chuẩn C++ đây là **Undefined Behavior** — không chỉ là kết quả sai số mà chương trình có thể làm bất cứ gì. Sửa bằng mutex (bảo vệ vùng tới hạn) hoặc dùng `std::atomic` cho biến đơn.
</details>

<details><summary>2) Khi nào dùng mutex, khi nào dùng atomic?</summary>

`std::atomic` cho thao tác trên **một biến đơn** (counter, flag, con trỏ) — không khóa, hiệu năng cao, không thể chia cắt. Mutex cho **vùng tới hạn phức tạp**: cập nhật nhiều biến liên quan phải nhất quán với nhau, hoặc thao tác phức hợp không thể biểu diễn bằng một atomic op. Nếu chỉ tăng một counter thì atomic; nếu cập nhật một cấu trúc dữ liệu (vd thêm node vào list rồi cập nhật size) thì mutex.
</details>

<details><summary>3) Deadlock là gì, 4 điều kiện và cách tránh?</summary>

Deadlock là tình trạng các thread chờ lẫn nhau giải phóng tài nguyên (mutex) nên kẹt vĩnh viễn. Bốn điều kiện Coffman cần đủ cả: mutual exclusion, hold-and-wait, no preemption, circular wait. Cách tránh: luôn khóa nhiều mutex theo **cùng một thứ tự cố định** (phá circular wait); dùng `std::scoped_lock` để khóa nhiều mutex an toàn; giữ critical section nhỏ; tránh gọi hàm không kiểm soát khi đang giữ lock; tránh nested lock.
</details>

<details><summary>4) Vì sao luôn dùng lock_guard/unique_lock thay vì lock()/unlock() thủ công?</summary>

Vì RAII: lock được giải phóng tự động khi object lock ra scope, kể cả khi có exception hoặc nhiều đường return. Gọi `unlock()` thủ công dễ bị bỏ sót ở một nhánh hoặc khi exception ném giữa critical section → mutex bị giữ mãi → deadlock. `lock_guard` đơn giản; `unique_lock` linh hoạt hơn (defer, unlock sớm, dùng với condition_variable).
</details>

<details><summary>5) Memory order để làm gì? Mặc định nên dùng cái nào?</summary>

CPU/compiler có thể reorder lệnh để tối ưu; trong đa luồng điều này khiến thread khác quan sát thứ tự ghi khác kỳ vọng. Memory order quy định ràng buộc sắp xếp quanh các thao tác atomic: `seq_cst` (nhất quán tuần tự toàn cục, dễ suy luận nhất, chi phí cao), `acquire/release` (đồng bộ cặp đôi cho producer/consumer), `relaxed` (chỉ đảm bảo atomicity). Mặc định nên dùng `seq_cst`; chỉ hạ xuống khi đã đo hiệu năng và thật sự hiểu, vì đây là vùng cực dễ sai.
</details>

<details><summary>6) Vì sao cv.wait nên dùng kèm predicate?</summary>

Vì hai lý do: (1) **spurious wakeup** — condition variable có thể đánh thức thread mà không có notify; predicate kiểm tra lại điều kiện thật để bỏ qua lần thức giả. (2) **lost wakeup / kiểm tra điều kiện trước khi chờ** — nếu điều kiện đã đúng trước khi gọi wait thì không chờ. Dạng `cv.wait(lock, pred)` lặp kiểm tra predicate, nhả lock khi ngủ và giành lại khi thức, đảm bảo đúng đắn.
</details>

<details><summary>7) std::thread bị hủy khi còn joinable thì sao? jthread khác gì?</summary>

Nếu một `std::thread` còn joinable (chưa `join()` cũng chưa `detach()`) mà destructor chạy, nó gọi `std::terminate()` làm chương trình chết. Vì vậy phải luôn join hoặc detach trước khi thread object ra scope. `std::jthread` (C++20) khắc phục bằng cách **tự join** trong destructor và hỗ trợ cooperative cancellation qua `stop_token`, an toàn và tiện hơn.
</details>

---
⬅️ [lambdas-functional.md](lambdas-functional.md) · ➡️ Tiếp theo: [03-operating-system/](../03-operating-system/)
