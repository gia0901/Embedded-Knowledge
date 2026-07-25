# Ch. 4 — Synchronizing concurrent operations (tr. 72–123) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 3 — Sharing data](03-sharing-data.md)** · **[Ch. 5 → The C++ memory model and atomics](05-memory-model.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Đây là chương dài nhất sách (~50 trang) và trả lời trực tiếp câu hỏi mà ch. 3 để ngỏ:** làm sao một thread **chờ một sự kiện** từ thread khác **mà không đốt CPU**. Ch. 3 bảo vệ *dữ liệu*; chương này đồng bộ *hành động*. Chứa hai chủ đề phỏng vấn dùng hằng ngày: **condition variable** (+ thread-safe queue, spurious wakeup, while-không-if) và **future/promise/`async`/`packaged_task`** (+ cách lấy giá trị/exception từ thread — thứ mà ch. 2 để ngỏ).

**Sách nêu 4 mục tiêu chương (tr. 72):** chờ một sự kiện · chờ **sự kiện một-lần** bằng future · chờ **có giới hạn thời gian** · dùng đồng bộ để **đơn giản hóa code**.

**⚠️ Lưu ý xuyên suốt về Concurrency TS:** phần cuối chương (4.4.3–4.4.10) mô tả các tính năng trong `std::experimental` (continuations `.then()`, `when_all`/`when_any`, latches, barriers). **Latch & barrier NAY đã chuẩn hóa trong C++20** thành `std::latch`/`std::barrier` (không còn `experimental`); **continuations (`.then`) vẫn CHƯA vào standard** tính đến C++23. Mình đọc để nắm ý tưởng, nhưng phần chắc chắn được hỏi là **condition variable + future/promise/async** (mục 4.1–4.3).

---

## Cụm 1 — Chờ sự kiện bằng **condition variable** (tr. 73–76)

### 1.1 Ba cách chờ, hai cách đầu đều tệ (tr. 73–74)

**Ẩn dụ (tr. 73):** đi tàu đêm, muốn xuống đúng ga. (a) **Thức cả đêm** canh — không lỡ ga nhưng kiệt sức. (b) **Đặt báo thức** theo giờ dự kiến — tàu trễ thì dậy sớm, hết pin thì ngủ quên. (c) **Lý tưởng:** có ai đó đánh thức bạn **đúng lúc** tàu tới ga.

Ánh xạ sang thread — một thread chờ thread khác xong việc:

**Cách 1 — busy-wait (spin) kiểm cờ liên tục (tr. 73): ⚠️ phí kép.**
```cpp
bool flag; std::mutex m;
void wait_for_flag() {
    std::unique_lock<std::mutex> lk(m);
    while (!flag) { /* quay vòng liên tục, giữ CPU */ }
}
```
Phí (a) **đốt CPU** kiểm cờ vô ích, và (b) **giữ mutex** khiến thread kia **không lock được để set cờ**. Ẩn dụ: thức cả đêm nói chuyện với lái tàu → tàu chạy chậm hơn.

**Cách 2 — sleep-and-check (tr. 73–74):** ngủ ngắn giữa các lần kiểm.
```cpp
void wait_for_flag() {
    std::unique_lock<std::mutex> lk(m);
    while (!flag) {
        lk.unlock();                                          // (1) nhả mutex TRƯỚC khi ngủ
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // (2) ngủ 100ms
        lk.lock();                                            // (3) khóa lại rồi kiểm tiếp
    }
}
```
Đỡ hơn (không đốt CPU khi ngủ), nhưng **khó chọn thời gian ngủ (tr. 74):** ngủ ngắn → vẫn phí kiểm; ngủ dài → **trễ** khi việc đã xong (rớt frame trong game, lố time slice trong realtime).

**Cách 3 — condition variable, cách nên dùng (tr. 74):** *"a condition variable is associated with an event or other condition, and one or more threads can wait for that condition to be satisfied."* Thread xác định điều kiện thỏa → **notify** thread đang chờ để đánh thức.

### 1.2 `std::condition_variable` — Listing 4.1 (tr. 74–75)

Hai loại (tr. 74): **`std::condition_variable`** (chỉ làm việc với `std::mutex` — **ưu tiên dùng**) và **`std::condition_variable_any`** (làm việc với bất kỳ thứ gì "mutex-like" — linh hoạt hơn nhưng tốn size/hiệu năng/tài nguyên OS). Cả hai ở `<condition_variable>`.

```cpp
std::mutex mut;
std::queue<data_chunk> data_queue;         // (1) hàng đợi truyền dữ liệu giữa 2 thread
std::condition_variable data_cond;

void data_preparation_thread() {
    while (more_data_to_prepare()) {
        data_chunk const data = prepare_data();
        {
            std::lock_guard<std::mutex> lk(mut);
            data_queue.push(data);          // (2) đẩy dữ liệu (trong lock)
        }                                   //     ← đóng scope: NHẢ mutex TRƯỚC khi notify
        data_cond.notify_one();             // (3) đánh thức MỘT thread đang chờ
    }
}

void data_processing_thread() {
    while (true) {
        std::unique_lock<std::mutex> lk(mut);              // (4) unique_lock, KHÔNG lock_guard
        data_cond.wait(
            lk, []{ return !data_queue.empty(); });        // (5) chờ tới khi queue có dữ liệu
        data_chunk data = data_queue.front();
        data_queue.pop();
        lk.unlock();                                       // (6) NHẢ lock trước khi xử lý (tốn giờ)
        process(data);
        if (is_last_chunk(data)) break;
    }
}
```

**Cơ chế `wait()` — phần quan trọng nhất (tr. 75):** `wait(lk, pred)`:
1. Gọi predicate (lambda). **Thỏa (true)** → return ngay, **mutex vẫn đang khóa**.
2. **Không thỏa (false)** → `wait()` **tự unlock mutex** và đưa thread vào trạng thái blocked/waiting.
3. Khi có `notify_one()` → thread thức dậy, **tái chiếm lock**, **kiểm điều kiện lại**. Thỏa → return (mutex vẫn khóa); không thỏa → nhả lock, ngủ tiếp.

→ **Đây là lý do phải dùng `std::unique_lock` chứ không `std::lock_guard` (tr. 75–76):** `wait()` cần **unlock rồi lock lại** mutex nhiều lần — `lock_guard` không cho phép điều đó. Nếu mutex cứ khóa suốt lúc ngủ thì thread chuẩn bị dữ liệu không lock được để push → điều kiện **không bao giờ** thỏa → treo.

**Vì sao notify sau khi nhả mutex (tr. 75):** đóng scope `{ }` ở (2) nhả mutex **trước** `notify_one()` (3) → nếu thread chờ thức dậy ngay, nó **không phải block lại** để chờ mình unlock.

### 1.3 ⚠️ Spurious wakeup & vì sao dùng predicate/`while` (tr. 76)

**Spurious wake (đánh thức giả) (tr. 76):** khi thread chờ tái chiếm mutex và kiểm điều kiện **mà KHÔNG phải do notify** từ thread khác → gọi là spurious wake. Số lần/tần suất là **không xác định**.

Hệ quả thực chiến: *"it isn't advisable to use a function with side effects for the condition check"* — predicate **không nên có side effect**, vì có thể bị gọi **nhiều lần bất định**.

Sách nói thẳng bản chất (tr. 76): `wait` về cơ bản là **tối ưu của busy-wait**, và một cài đặt hợp lệ (dù không lý tưởng) chỉ là vòng lặp:
```cpp
template<typename Predicate>
void minimal_wait(std::unique_lock<std::mutex>& lk, Predicate pred) {
    while (!pred()) { lk.unlock(); lk.lock(); }   // KHÔNG hề ngủ, KHÔNG hề chờ notify!
}
```
→ **Code của bạn phải chạy đúng kể cả với cài đặt tối thiểu này**, lẫn cài đặt chỉ thức khi có `notify`. Đây chính là lý do **phải kiểm điều kiện trong vòng lặp** (predicate của `wait` làm hộ bạn) — không được giả định "thức dậy nghĩa là điều kiện đã thỏa".

> 🆕 **Nếu tự viết `wait` không predicate, PHẢI dùng `while` không phải `if`:**
> ```cpp
> // ❌ SAI: while(!ready) if → spurious wake làm chạy tiếp khi chưa sẵn sàng
> // if (!ready) cv.wait(lk);
> // ✅ ĐÚNG:
> while (!ready) cv.wait(lk);        // tương đương cv.wait(lk, []{ return ready; });
> ```
> Đây là **bẫy phỏng vấn kinh điển "while vs if với condition variable"**. Lý do có hai: (1) **spurious wakeup**, (2) **stolen wakeup** — thread khác chen vào lấy mất điều kiện giữa lúc bạn thức và tái chiếm lock. Predicate-form `wait(lk, pred)` **đã bọc sẵn vòng lặp** này.

### Insight đáng nhớ (Cụm 1)
- **Condition variable = "đánh thức tôi khi điều kiện thỏa", thay cho busy-wait/sleep.** Không đốt CPU, không trễ do ngủ quá.
- **Luôn dùng `unique_lock` + predicate với `wait`.** Predicate bọc vòng lặp chống spurious/stolen wakeup; `unique_lock` cho `wait` unlock-relock. Predicate **không side effect**.
- **Ba việc phải đúng thứ tự:** giữ lock → sửa dữ liệu chung → **nhả lock** → `notify`. Và nhả lock trước khi làm việc tốn giờ (như `process`).

---

## Cụm 2 — Xây **thread-safe queue** bằng condition variable (tr. 76–81)

### 2.1 Interface: cùng bài học race-trong-interface như `stack` ch. 3 (tr. 76–78)

`std::queue` (Listing 4.2) cũng có 3 nhóm thao tác — query trạng thái (`empty`/`size`), query phần tử (`front`/`back`), sửa đổi (`push`/`pop`) — nên **dính cùng race-trong-interface** như `stack` (§3.2.3). Phải **gộp `front()` + `pop()`** thành một hàm.

Điểm mới của queue: thread nhận thường **cần chờ** dữ liệu. Nên có **hai biến thể `pop`** (Listing 4.3, tr. 77–78):
- **`try_pop()`** — thử pop, **trả về ngay** kể cả khi không có gì (kèm chỉ báo thất bại).
- **`wait_and_pop()`** — **chờ** tới khi có giá trị.

Mỗi biến thể lại có 2 overload (trả qua reference / trả `shared_ptr`) như `stack`.

### 2.2 `threadsafe_queue` hoàn chỉnh — Listing 4.5 (tr. 79–80)

```cpp
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>
class threadsafe_queue {
private:
    mutable std::mutex mut;              // ⚠️ mutable: lock được trong empty() const
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() {}
    threadsafe_queue(threadsafe_queue const& other) {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();                     // báo cho một thread đang wait_and_pop
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);       // unique_lock cho wait()
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });
        value = data_queue.front();
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);        // lock_guard đủ: KHÔNG chờ
        if (data_queue.empty()) return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty()) return std::shared_ptr<T>();   // rỗng → con trỏ NULL
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
```

Điểm sách nhấn:
- **`mutable std::mutex` (tr. 80):** dù `empty()` là `const` và copy-ctor nhận `const&`, các thread khác vẫn có thể có **non-const reference** và đang gọi hàm sửa đổi → vẫn phải lock; mà lock là thao tác *sửa* mutex → phải `mutable`.
- **`try_pop` dùng `lock_guard`, `wait_and_pop` dùng `unique_lock`:** chỉ hàm nào **gọi `wait()`** mới cần `unique_lock`.
- **`notify_one` trong `push`:** mỗi lần đẩy dữ liệu, đánh thức **một** thread đang `wait_and_pop`.

### 2.3 `notify_one` vs `notify_all` (tr. 80–81)

- **`notify_one` (tr. 80–81):** khi nhiều thread chia việc và **chỉ một** thread nên phản hồi mỗi notify (như nhiều processing thread rút từ một queue). **Không đảm bảo** thread nào được đánh thức, thậm chí có thread nào đang chờ hay không.
- **`notify_all` (tr. 81):** khi **nhiều thread cùng chờ một sự kiện và tất cả cần phản hồi** — ví dụ dữ liệu chung vừa được khởi tạo, mọi thread dùng chung nó; hoặc chờ một lần cập nhật định kỳ. Đánh thức **tất cả** thread đang `wait()` để mỗi thread tự kiểm điều kiện.

**Câu chuyển sang future (tr. 81):** nếu thread chỉ chờ **một lần** (điều kiện thỏa xong không bao giờ chờ lại), nhất là khi chờ **một dữ liệu cụ thể**, thì condition variable có thể **không phải lựa chọn tốt nhất** → dùng **future**.

### Insight đáng nhớ (Cụm 2)
- **Thread-safe queue là "kênh" chuẩn giữa producer và consumer.** Đồng bộ gói gọn trong queue → giảm mạnh số điểm race. `wait_and_pop` cho consumer chờ không đốt CPU; `try_pop` cho consumer không muốn block.
- **`notify_one` khi một-thread-đủ; `notify_all` khi mọi-thread-cần.** Chọn sai `notify_all` khi chỉ cần một → đánh thức thừa (thundering herd), phí CPU; chọn sai `notify_one` khi cần tất cả → thread bị bỏ ngủ.

---

## Cụm 3 — 🎯 Future: lấy giá trị (và exception) từ thread khác (tr. 81–90)

### 3.1 Future là gì (tr. 81–82)

**Ẩn dụ (tr. 81):** ra sân bay, làm thủ tục xong, **chờ thông báo lên máy bay** — có thể vài giờ. Bạn làm việc khác (đọc sách, ăn ở quán) nhưng thực chất chờ **một tín hiệu**. Và chuyến bay đó chỉ đi **một lần**.

**Future mô hình hóa "sự kiện một-lần" (tr. 81):** thread cần chờ một sự kiện thì **lấy một future đại diện** cho nó, rồi hoặc **thăm dò định kỳ** (poll), hoặc **chờ hẳn** tới khi future sẵn sàng. Future có thể **mang dữ liệu** (kết quả) hoặc không. **Sự kiện xảy ra rồi thì future không reset được.**

Hai loại (tr. 81, mô phỏng `unique_ptr`/`shared_ptr`):
| | `std::future<>` | `std::shared_future<>` |
|---|---|---|
| Số instance trỏ tới sự kiện | **Duy nhất một** | **Nhiều** |
| Copy | Không (chỉ move) | **Có** |
| Truy cập từ nhiều thread | Không an toàn (data race) | An toàn **nếu mỗi thread có bản copy riêng** |

Template param = **kiểu dữ liệu kèm theo**; dùng `std::future<void>` khi không có dữ liệu. **⚠️ Bản thân future object không tự đồng bộ (tr. 82):** truy cập **một** future từ nhiều thread không có đồng bộ thêm = **data race**.

**Ba cách tạo future (tr. 82–87):** (1) `std::async` — chạy task nền; (2) `std::packaged_task` — bọc callable; (3) `std::promise` — set giá trị tường minh. Đi từ cao xuống thấp.

### 3.2 `std::async` — chạy task nền, lấy giá trị bằng `get()` (tr. 82–84)

Đây là câu trả lời cho điều **ch. 2 để ngỏ** (`std::thread` không trả giá trị trực tiếp được).

**Listing 4.6 (tr. 83):**
```cpp
#include <future>
#include <iostream>
int find_the_answer_to_ltuae();
void do_other_stuff();

int main() {
    std::future<int> the_answer = std::async(find_the_answer_to_ltuae);  // chạy nền
    do_other_stuff();                                                     // làm việc khác
    std::cout << "The answer is " << the_answer.get() << std::endl;       // get() BLOCK tới khi có
}
```
`std::async` trả về **`std::future`** (không phải `std::thread`); `get()` **block tới khi future sẵn sàng** rồi trả giá trị.

**Truyền tham số giống `std::thread` (Listing 4.7, tr. 83–84):** member fn thì đối số 2 là object (trực tiếp/pointer/`std::ref`); rvalue được **move**; hỗ trợ **move-only type**.
```cpp
X x;
auto f1 = std::async(&X::foo, &x, 42, "hello");   // (&x)->foo(42,"hello")
auto f2 = std::async(&X::bar, x, "goodbye");      // copy_of_x.bar("goodbye")
auto f3 = std::async(Y(), 3.141);                 // move_of_Y()(3.141)
auto f4 = std::async(std::ref(y), 2.718);         // y(2.718)
auto f5 = std::async(move_only());                // tmp() với tmp từ std::move(...)
```

**⚠️ Launch policy — điểm hay bị bỏ sót (tr. 84):** mặc định **implementation tự chọn** chạy thread mới hay chạy đồng bộ lúc `get()`. Chỉ định bằng `std::launch`:
```cpp
auto f6 = std::async(std::launch::async, Y(), 1.2);        // BẮT BUỘC thread mới
auto f7 = std::async(std::launch::deferred, baz, std::ref(x));  // hoãn tới wait()/get()
auto f8 = std::async(std::launch::deferred | std::launch::async, baz, ...);  // impl chọn (mặc định)
auto f9 = std::async(baz, std::ref(x));                    // = mặc định
f7.wait();                                                 // giờ mới chạy hàm deferred
```
**⚠️ Nếu deferred, hàm có thể KHÔNG BAO GIỜ chạy** (nếu không ai gọi `wait`/`get`). 🆕 Đây là bẫy thực chiến: `std::async` mặc định (không policy) là **không xác định** — có thể deferred → task không chạy song song như bạn tưởng, hoặc destructor của future **block** chờ task. Nếu cần chạy nền thật, **luôn ghi `std::launch::async` tường minh**.

### 3.3 `std::packaged_task` — bọc callable, nối future vào task (tr. 84–87)

`std::packaged_task<>` **buộc một future vào một callable** (tr. 84): khi package task được **gọi**, nó chạy callable và làm future sẵn sàng (giá trị trả về lưu làm associated data). Là **khối xây thread pool** (ch. 9) hay scheduler — chia việc lớn thành sub-task, mỗi cái bọc `packaged_task`, đưa vào scheduler; scheduler chỉ làm việc với `packaged_task` thay vì hàm rời.

Template param là **chữ ký hàm** (`void()`, `int(std::string&, double*)`...). Kiểu không cần khớp tuyệt đối — chuyển đổi ngầm được (tr. 85). `packaged_task` là **callable**, gọi trực tiếp / bọc `std::function` / đưa vào `std::thread` đều được.

**Listing 4.9 (tr. 85–86) — chạy code trên GUI thread** (nhiều framework GUI đòi update GUI từ thread riêng):
```cpp
std::mutex m;
std::deque<std::packaged_task<void()>> tasks;
bool gui_shutdown_message_received();
void get_and_process_gui_message();

void gui_thread() {                                    // thread GUI
    while (!gui_shutdown_message_received()) {
        get_and_process_gui_message();                 // xử lý message GUI (click...)
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m);
            if (tasks.empty()) continue;               // không có task → lặp tiếp
            task = std::move(tasks.front());           // lấy task ra
            tasks.pop_front();
        }
        task();                                        // CHẠY task trên GUI thread
    }
}
std::thread gui_bg_thread(gui_thread);

template<typename Func>
std::future<void> post_task_for_gui_thread(Func f) {   // gửi task cho GUI thread
    std::packaged_task<void()> task(f);
    std::future<void> res = task.get_future();          // lấy future TRƯỚC khi đẩy task đi
    std::lock_guard<std::mutex> lk(m);
    tasks.push_back(std::move(task));
    return res;                                         // caller có thể chờ future (hoặc bỏ qua)
}
```
Ý chính: **tách "cái gì chạy" khỏi "chạy ở đâu"**. Lấy `get_future()` **trước** khi chuyển `packaged_task` đi nơi khác để gọi sau.

### 3.4 `std::promise` — set giá trị tường minh (tr. 87–88)

Dùng khi task **không diễn đạt được thành một lời gọi hàm đơn**, hoặc kết quả đến từ **nhiều nơi**. Bối cảnh (tr. 87): xử lý **nhiều network connection trên ít thread** (một thread lo nhiều connection) — vì mỗi connection một thread thì cạn tài nguyên OS/quá nhiều context switch.

`std::promise<T>` **set giá trị** (`set_value()`), đọc qua `std::future<T>` gắn với nó (lấy bằng `get_future()`). Waiting thread block trên future; thread cấp dữ liệu set giá trị qua promise → future sẵn sàng.

**Listing 4.10 (tr. 88):**
```cpp
void process_connections(connection_set& connections) {
    while (!done(connections)) {
        for (auto connection = connections.begin(), end = connections.end();
             connection != end; ++connection) {
            if (connection->has_incoming_data()) {
                data_packet data = connection->incoming();
                std::promise<payload_type>& p = connection->get_promise(data.id);
                p.set_value(data.payload);            // dữ liệu đến → set promise
            }
            if (connection->has_outgoing_data()) {
                outgoing_packet data = connection->top_of_outgoing_queue();
                connection->send(data.payload);
                data.promise.set_value(true);         // gửi xong → set cờ thành công
            }
        }
    }
}
```

### 3.5 🎯 Lưu **exception** vào future (tr. 88–90)

Đây là tính năng quan trọng và hay hỏi: **exception được truyền qua thread** một cách sạch sẽ.

**Với `std::async` (tr. 89):** nếu hàm chạy trong `async` **ném exception**, exception đó được **lưu vào future** thay cho giá trị, future sẵn sàng, và **`get()` ném lại** exception đó.
```cpp
double square_root(double x) {
    if (x < 0) throw std::out_of_range("x<0");
    return sqrt(x);
}
std::future<double> f = std::async(square_root, -1);
double y = f.get();     // ⚠️ ném std::out_of_range Y HỆT như gọi trực tiếp square_root(-1)
```
*(Standard không nói rõ ném object gốc hay bản copy — tùy compiler/thư viện.)* `packaged_task` cũng vậy.

**Với `std::promise` (tr. 89–90):** dùng **`set_exception()`** thay `set_value()`:
```cpp
try {
    some_promise.set_value(calculate_value());
} catch (...) {
    some_promise.set_exception(std::current_exception());   // lấy exception đang bay
}
// Hoặc dựng thẳng không cần throw:
some_promise.set_exception(std::make_exception_ptr(std::logic_error("foo")));
```
`std::make_exception_ptr` **sạch hơn** try/catch khi biết trước kiểu exception, và cho compiler tối ưu tốt hơn (tr. 90).

**⚠️ broken_promise (tr. 90):** nếu **hủy `std::promise`/`std::packaged_task` mà không set value/không gọi task**, destructor lưu exception **`std::future_error`** với mã **`std::future_errc::broken_promise`** vào future. Logic: *"by creating a future you make a promise to provide a value or exception, and by destroying the source without providing one, you break that promise."* — nếu không, thread chờ sẽ **chờ mãi mãi**.

### Insight đáng nhớ (Cụm 3)
- **`async` → `packaged_task` → `promise`: cao xuống thấp.** `async` khi chỉ cần "chạy nền lấy kết quả"; `packaged_task` khi cần **tách task khỏi nơi chạy** (thread pool, GUI thread); `promise` khi kết quả đến **không phải từ một lời gọi hàm** (network, sự kiện rời rạc).
- **Future truyền cả giá trị LẪN exception qua ranh giới thread.** `get()` ném lại exception y như gọi hàm trực tiếp — đây là điểm đắt giá so với tự chuyền cờ lỗi.
- **⚠️ `std::async` mặc định có thể deferred.** Muốn chạy nền thật → **`std::launch::async` tường minh**. Và cẩn thận: future của `async` mà không giữ tên thì **destructor có thể block** chờ task xong.

---

## Cụm 4 — `std::shared_future`: nhiều thread cùng chờ một sự kiện (tr. 90–92)

**Vì sao cần (tr. 90):** `std::future` chỉ cho **một thread** chờ kết quả (`get()` là **one-shot** — gọi xong hết giá trị). Nhiều thread cùng chờ một sự kiện → **`std::shared_future`** (copyable).

**⚠️ Cách dùng đúng — mỗi thread một bản copy riêng (Figure 4.1, tr. 91):**
- Member function trên **một** `shared_future` object **vẫn không đồng bộ** → chia sẻ một object giữa nhiều thread mà không lock = **data race**.
- **Cách đúng:** truyền **bản copy** của `shared_future` cho từng thread → mỗi thread truy cập **object local riêng**, phần shared state bên trong đã được thư viện đồng bộ.

**Tạo `shared_future` từ `future` bằng move (tr. 92):**
```cpp
std::promise<int> p;
std::future<int> f(p.get_future());
assert(f.valid());                             // f hợp lệ
std::shared_future<int> sf(std::move(f));       // CHUYỂN quyền sở hữu (f rỗng sau đó)
assert(!f.valid());
assert(sf.valid());

// Hoặc gọn hơn với share():
std::promise<std::string> p2;
std::shared_future<std::string> sf2(p2.get_future());   // move ngầm từ rvalue
auto sf3 = p2.get_future().share();                     // share() tự tạo shared_future
```
`share()` + `auto` tránh gõ kiểu dài dòng, và tự cập nhật khi đổi kiểu promise (tr. 92).

**Ứng dụng (tr. 91):** bảng tính song song — mỗi ô có một giá trị cuối, được **nhiều công thức ô khác** dùng. Các ô phụ thuộc dùng `shared_future` tham chiếu ô nguồn; chạy song song, ô nào đủ điều kiện thì tính, ô phụ thuộc block tới khi dependency sẵn sàng.

### Insight đáng nhớ (Cụm 4)
- **`future` = unique ownership (một người lấy), `shared_future` = shared (nhiều người lấy).** Đúng cặp `unique_ptr`/`shared_ptr`.
- **Chống data race trên `shared_future`: mỗi thread một bản copy.** Bản thân object không tự đồng bộ; chỉ shared state bên trong mới đồng bộ.

---

## Cụm 5 — Chờ **có giới hạn thời gian**: clock, duration, time_point (tr. 92–99)

### 5.1 Hai loại timeout & hậu tố `_for`/`_until` (tr. 92–93)

- **Duration-based** — chờ một **khoảng** (vd 30ms) → hàm hậu tố **`_for`**.
- **Absolute** — chờ tới một **thời điểm** (vd 17:30:15 UTC 30/11/2011) → hàm hậu tố **`_until`**.

Ví dụ `std::condition_variable` có `wait_for()` và `wait_until()`, mỗi cái 2 overload (có/không predicate).

### 5.2 Clocks (tr. 93–94)

Một **clock** cung cấp 4 thứ: **now()** (thời gian hiện tại), **kiểu time_point**, **tick period**, và **có steady không**.

| Clock | Đặc điểm |
|---|---|
| `std::chrono::system_clock` | Đồng hồ "thực" của hệ; **⚠️ KHÔNG steady** (có thể bị chỉnh → `now()` sau nhỏ hơn `now()` trước); chuyển đổi được với `time_t` |
| `std::chrono::steady_clock` | **Steady** — tick đều, không chỉnh được → **dùng cho timeout** |
| `std::chrono::high_resolution_clock` | Tick period nhỏ nhất (độ phân giải cao nhất); có thể là typedef của clock khác |

`is_steady` = true nếu clock steady. **Steady clock quan trọng cho tính timeout** (tr. 94).

### 5.3 Durations (tr. 94–96)

`std::chrono::duration<Rep, Period>` — `Rep` = kiểu số (`int`/`double`), `Period` = phân số giây (`std::ratio`).

```cpp
std::chrono::duration<short, std::ratio<60,1>>    // phút, lưu trong short
std::chrono::duration<double, std::ratio<1,1000>> // ms, lưu trong double

// Typedef sẵn: nanoseconds, microseconds, milliseconds, seconds, minutes, hours

// Literal C++14 (namespace std::chrono_literals):
using namespace std::chrono_literals;
auto one_day = 24h;
auto half_an_hour = 30min;
auto gap = 30ms;
// 15ns == std::chrono::nanoseconds(15). Với floating literal: 2.5min là duration<float, ratio<60,1>>

// Chuyển đổi: ngầm nếu không mất mát (h→s OK), tường minh khi có thể cắt:
std::chrono::milliseconds ms(54802);
std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(ms);  // = 54 (CẮT, không làm tròn)

// Số học + count():
std::chrono::milliseconds(1234).count();  // 1234
```

**Chờ theo duration (tr. 95):**
```cpp
std::future<int> f = std::async(some_task);
if (f.wait_for(std::chrono::milliseconds(35)) == std::future_status::ready)
    do_something_with(f.get());
```
`wait_for`/`wait_until` trả **status**: `future_status::ready` / `timeout` / `deferred`. **⚠️ Đo bằng steady clock nội bộ** → 35ms nghĩa là 35ms trôi qua thật, kể cả khi system clock bị chỉnh (tr. 95–96). (Thực tế thời gian trả về có thể dài hơn do scheduling.)

### 5.4 Time points (tr. 96–97)

`std::chrono::time_point<Clock, Duration>` = khoảng thời gian kể từ **epoch** của clock. Cộng/trừ duration ra time_point mới; trừ hai time_point ra duration (đo code):
```cpp
auto start = std::chrono::high_resolution_clock::now();
do_something();
auto stop = std::chrono::high_resolution_clock::now();
std::cout << std::chrono::duration<double>(stop - start).count() << " seconds";
```

**Listing 4.11 (tr. 97) — chờ CV có timeout, cách ĐÚNG:**
```cpp
std::condition_variable cv;
bool done;
std::mutex m;
bool wait_loop() {
    auto const timeout = std::chrono::steady_clock::now()
                       + std::chrono::milliseconds(500);   // ⚠️ tính MỐC 1 lần, dùng _until
    std::unique_lock<std::mutex> lk(m);
    while (!done) {
        if (cv.wait_until(lk, timeout) == std::cv_status::timeout)
            break;
    }
    return done;
}
```
**⚠️ Vì sao `wait_until` với mốc cố định, KHÔNG `wait_for` trong vòng lặp (tr. 97):** nếu dùng `wait_for()` trong loop, mỗi lần spurious wakeup **reset lại đồng hồ chờ** → tổng thời gian chờ **không bị chặn** (unbounded). Tính **một mốc tuyệt đối** rồi `wait_until` thì tổng vòng lặp **có chặn**.

### 5.5 Các hàm nhận timeout (tr. 98–99)

- **Sleep:** `std::this_thread::sleep_for(duration)` / `sleep_until(time_point)` — báo thức cơ bản.
- **⚠️ Timed mutex:** `std::mutex`/`std::recursive_mutex` **KHÔNG** hỗ trợ timeout khi lock; phải dùng **`std::timed_mutex`** / **`std::recursive_timed_mutex`** với `try_lock_for()`/`try_lock_until()`.
- CV, future cũng nhận timeout (Table 4.1, tr. 98–99). Return: CV → `cv_status::timeout`/`no_timeout`; mutex → `bool`; future → `future_status::ready`/`timeout`/`deferred`.

### Insight đáng nhớ (Cụm 5)
- **Timeout dùng `steady_clock`, không `system_clock`.** `system_clock` chỉnh được → NTP nhảy giờ có thể làm timeout sai/treo. `steady_clock` đơn điệu tăng.
- **Chờ có deadline: tính mốc tuyệt đối 1 lần + `wait_until`.** Dùng `wait_for` trong vòng lặp spurious-wakeup → tổng chờ unbounded.
- **`_for` = khoảng, `_until` = mốc.** Muốn timeout khi lock mutex → `std::timed_mutex`, `std::mutex` thường không có.

---

## Cụm 6 — Dùng đồng bộ để **đơn giản hóa code**: FP, message passing, continuations, latch/barrier (tr. 99–123)

Ý lớn (tr. 99): dùng các công cụ đồng bộ làm **khối xây dựng** để **tập trung vào cái cần đồng bộ thay vì cơ chế**. Ba paradigm: **functional programming**, **message passing (CSP/Actor)**, **continuation style**.

### 6.1 Functional programming với future — parallel quicksort (tr. 99–104)

**FP (tr. 99–100):** kết quả hàm **chỉ phụ thuộc tham số**, không phụ thuộc/không sửa external state (**pure function**). Với concurrency điều này cực mạnh: *"If there are no modifications to shared data, there can be no race conditions and thus no need to protect shared data with mutexes."* Future là **mảnh ghép cuối** làm FP-style concurrency khả thi trong C++ — truyền future giữa các thread để kết quả tính này phụ thuộc kết quả tính kia **mà không truy cập shared data**.

**Listing 4.12 — quicksort tuần tự FP-style** (nhận & trả list **by value**, không sort in-place):
```cpp
template<typename T>
std::list<T> sequential_quick_sort(std::list<T> input) {
    if (input.empty()) return input;
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());   // lấy phần tử đầu làm pivot (splice, không copy)
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t){ return t < pivot; });             // chia: < pivot | >= pivot
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    auto new_lower(sequential_quick_sort(std::move(lower_part)));   // đệ quy nửa dưới
    auto new_higher(sequential_quick_sort(std::move(input)));       // đệ quy nửa trên
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}
```

**Listing 4.13 — parallel quicksort, chỉ đổi MỘT dòng (tr. 102):**
```cpp
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
    if (input.empty()) return input;
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t){ return t < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    std::future<std::list<T>> new_lower(                   // (★) nửa dưới chạy THREAD KHÁC
        std::async(&parallel_quick_sort<T>, std::move(lower_part)));
    auto new_higher(parallel_quick_sort(std::move(input)));  // nửa trên: đệ quy trực tiếp

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());         // get(): chờ + move kết quả nền
    return result;
}
```
**Cách hoạt động (tr. 102–103):** nửa dưới đẩy sang thread khác bằng `std::async`; nửa trên đệ quy tại chỗ. Đệ quy → tận dụng hardware concurrency (đệ quy 3 tầng → 8 task). Nếu quá nhiều task, thư viện có thể **chuyển sang chạy đồng bộ** (trong thread gọi `get()`) để tránh oversubscription. **⚠️ Nhưng standard cho phép `async` tạo thread mới cho MỌI task** trừ khi ghi `deferred` — nên **phải đọc doc của implementation** nếu dựa vào auto-scaling.

**Listing 4.14 — tự viết `spawn_task` bằng `packaged_task` + `thread`** (thay `async`) — nền để sau này chuyển sang thread pool (ch. 9).

### 6.2 Message passing: CSP & Actor model — ví dụ ATM (tr. 104–108)

**CSP (Communicating Sequential Processes) (tr. 104):** thread **không chia sẻ data**, chỉ **truyền message** qua kênh. Mỗi thread là một **state machine**: nhận message → cập nhật state → có thể gửi message khác. **⚠️ C++ không ép được "không shared data"** (thread chung address space) → **kỷ luật lập trình viên**: message queue thì chia sẻ (bọc trong thư viện), còn lại không.

**Ví dụ ATM (tr. 104–108):** 3 thread — máy móc vật lý, logic ATM, giao tiếp ngân hàng — **chỉ truyền message**. Logic ATM mô hình bằng state machine (Figure 4.3): `waiting_for_card` → `getting_pin` → `verifying_pin` → `waiting_for_withdrawal_amount`...

**Listing 4.15/4.16 — mỗi state là một member function, chờ đúng loại message:**
```cpp
void atm::waiting_for_card() {
    interface_hardware.send(display_enter_card());
    incoming.wait()                                    // chờ message
        .handle<card_inserted>([&](card_inserted const& msg) {   // chỉ xử lý card_inserted
            account = msg.account; pin = "";
            interface_hardware.send(display_enter_pin());
            state = &atm::getting_pin;                 // CHUYỂN state
        });
}
void atm::run() {
    state = &atm::waiting_for_card;
    try { for (;;) (this->*state)(); }                 // vòng lặp gọi hàm state hiện tại
    catch (messaging::close_queue const&) {}
}
```
`handle()` **chuỗi** được nhiều loại message (`getting_pin` xử lý 3 loại: `digit_pressed`, `clear_last_pressed`, `cancel_pressed`); message không khớp bị **bỏ qua**. Đây gọi là **Actor model (tr. 107):** các actor rời rạc (mỗi cái một thread) gửi message cho nhau, **không shared state** ngoài cái truyền qua message.

Lợi ích (tr. 108): *"each thread can be treated entirely independently"* → giảm mạnh độ phức tạp, hạ tỉ lệ bug. Không phải nghĩ về đồng bộ, chỉ nghĩ "nhận message gì, gửi message gì".

### 6.3 ⚠️ Continuation-style (Concurrency TS — CHƯA vào standard) (tr. 108–113)

**`.then()` — "khi data sẵn sàng THÌ làm việc này" (tr. 109):** `std::experimental::future` có `then()`: `fut.then(continuation)`. Continuation chạy "trên một thread không xác định" khi future sẵn sàng; `fut` thành invalid, `then()` trả **future mới** giữ kết quả continuation → **chuỗi được**.
```cpp
auto fut = find_the_answer();
auto fut2 = fut.then(find_the_question);   // fut invalid, fut2 giữ kết quả
```
**⚠️ Continuation nhận `future` làm tham số** (không phải giá trị) — để nó tự xử lý exception: `std::string find_the_question(std::experimental::future<int> the_answer);`

**Chaining (Listing 4.20/4.21, tr. 111–112) — login bất đồng bộ không block thread:**
```cpp
std::experimental::future<void> process_login(std::string const& u, std::string const& p) {
    return backend.async_authenticate_user(u, p)
        .then([](std::experimental::future<user_id> id) {
            return backend.async_request_current_info(id.get());
        })
        .then([](std::experimental::future<user_data> info) {
            try { update_display(info.get()); }
            catch (std::exception& e) { display_error(e); }
        });
}
```
Điểm hay: **exception truyền suốt chuỗi** (mỗi `.get()` ném nếu mắt trước ném); **future-unwrapping** — nếu continuation trả `future<T>` thì `.then()` trả `future<T>` (không phải `future<future<T>>`). `shared_future` cũng có `.then` (cho phép **nhiều** continuation).

> 🆕 Continuation (`.then`) **vẫn chưa vào C++ standard** tính đến C++23 (bị hoãn nhiều lần). Nếu phỏng vấn hỏi "làm sao chuỗi các async operation không block" — nêu ý tưởng `.then`/future-unwrapping, và biết rằng **hiện phải dùng thư viện ngoài** (folly `Future`, HPX) hoặc **coroutine C++20** (`co_await` trên awaitable) để đạt cùng mục tiêu.

### 6.4 ⚠️ `when_all` / `when_any` (Concurrency TS) (tr. 114–118)

- **`when_all` (tr. 114–115):** trả future sẵn sàng khi **tất cả** future trong tập sẵn sàng → tránh đánh thức lặp + context switch thừa của việc chờ từng cái một (Listing 4.22 vs 4.23).
- **`when_any` (tr. 115–117):** trả future sẵn sàng khi **bất kỳ một** future sẵn sàng, kèm **index** cái nào kích hoạt (`when_any_result`). Dùng khi tìm giá trị đầu tiên thỏa điều kiện trong dataset lớn (Listing 4.24) — thread nào tìm thấy set `done_flag` (dùng `shared_ptr<atomic<bool>>` capture-by-copy để khỏi lo lifetime).
- Cả hai có bản **iterator-range** và **variadic** (trả tuple thay vì vector); **⚠️ luôn MOVE future vào** (nhận by value).

### 6.5 ⚠️ Latch & Barrier (Concurrency TS → **C++20 chuẩn hóa**) (tr. 118–123)

**Định nghĩa (tr. 118):**
| | **Latch** | **Barrier** |
|---|---|---|
| Bản chất | Object sẵn sàng khi counter **giảm về 0** | Điểm hẹn **tái dùng** cho một nhóm thread |
| "Latch" nghĩa là | Chốt lại — sẵn sàng rồi thì **giữ nguyên** tới khi hủy | Thả nhóm thread rồi **reset** cho vòng sau |
| Ai giảm counter | **Bất kỳ ai**, bao nhiêu lần cũng được | Mỗi thread tới barrier **một lần mỗi vòng** |
| Dùng khi | Chờ một loạt sự kiện xảy ra (một lần) | Đồng bộ nhóm thread theo từng pha (lặp) |

**`std::experimental::latch` (Listing 4.25, tr. 118–119):** construct với counter; `count_down()` khi mỗi sự kiện xảy ra; `wait()` chờ về 0; `is_ready()` kiểm; `count_down_and_wait()`.
```cpp
void foo() {
    unsigned const thread_count = ...;
    latch done(thread_count);                        // chờ thread_count sự kiện
    std::vector<std::future<void>> threads;
    for (unsigned i = 0; i < thread_count; ++i)
        threads.push_back(std::async(std::launch::async, [&, i]{   // i by VALUE (khỏi data race)
            data[i] = make_data(i);
            done.count_down();                       // xong phần data → đếm xuống
            do_more_stuff();                         // rồi làm việc khác
        }));
    done.wait();                                     // chờ mọi data sẵn sàng
    process_data(data, thread_count);                // an toàn: count_down "synchronizes-with" wait
}
```
**⚠️ Điểm mấu chốt (tr. 119):** `count_down` **synchronizes-with** `wait` — thay đổi thấy được bởi thread gọi `count_down` **được đảm bảo thấy** bởi thread trở ra từ `wait`. (Khái niệm synchronizes-with là của **ch. 5** — memory model.) Lambda capture `i` **by value** vì nó là biến đếm vòng lặp — capture by reference = **data race**.

**`std::experimental::barrier` (Listing 4.26, tr. 120–121):** construct với số thread; mỗi thread gọi `arrive_and_wait()` → block tới khi cả nhóm tới → thả hết + reset. `arrive_and_drop()` để rời nhóm (vòng sau cần ít hơn 1 thread). Barrier **chỉ đồng bộ trong nhóm** (thread ngoài nhóm không chờ được).

**`std::experimental::flex_barrier` (Listing 4.27, tr. 121–123):** như barrier nhưng có **completion function** chạy trên **đúng một thread** khi cả nhóm tới (trước khi thả) → dùng cho **serial region**, và có thể **đổi số thread** cho vòng sau (return -1 = giữ nguyên).

> 🆕 **Cập nhật C++20 quan trọng:** latch & barrier **đã chuẩn hóa** thành **`std::latch`** (`<latch>`) và **`std::barrier`** (`<barrier>`) — bỏ `experimental`. API hơi khác: `std::latch` có `count_down()`, `wait()`, `arrive_and_wait()`, `try_wait()`; `std::barrier` nhận **completion function** ngay trong constructor (gộp luôn `flex_barrier`), dùng `arrive_and_wait()`/`arrive_and_drop()`. Trong code mới, **dùng bản `std::` không `experimental`**.

### Insight đáng nhớ (Cụm 6)
- **Ba paradigm né shared mutable data:** FP (future truyền kết quả, pure function không race), message passing (Actor/CSP — mỗi thread một state machine, chỉ message), continuation (`.then` — "khi xong thì làm tiếp"). Cả ba **giảm bug bằng cách bỏ shared data**, không phải bằng khóa cẩn thận hơn.
- **`parallel_quick_sort` chỉ khác `sequential` MỘT dòng** (`std::async` cho nửa dưới) — sức mạnh của FP-style + future. Nhưng chưa tối ưu (`std::partition` vẫn tuần tự) → thực tế dùng `std::sort` với execution policy (ch. 10).
- **⚠️ Phân biệt cái gì đã chuẩn hóa:** condition variable, future, promise, packaged_task, async, timed_mutex, **latch/barrier (C++20)** = standard. **Continuation `.then`, when_all/when_any = vẫn `experimental`** (chưa vào standard) — biết ý tưởng, dùng coroutine/thư viện ngoài để thay.
- **Latch = một lần (chốt); Barrier = lặp (điểm hẹn theo pha).** Latch để "chờ N việc xong"; barrier để "đồng bộ nhóm thread qua từng giai đoạn xử lý".

---

## Tóm tắt chương (theo sách, tr. 123)

Đồng bộ hành động giữa thread là phần quan trọng của viết ứng dụng concurrent. Chương này phủ nhiều cách: từ **condition variable** cơ bản, qua **future/promise/packaged_task**, tới **latch & barrier**. Cùng ba cách tiếp cận đồng bộ: **functional-style** (mỗi task cho kết quả chỉ phụ thuộc input), **message passing** (giao tiếp qua message bất đồng bộ), và **continuation style** (chỉ định task nối tiếp, hệ thống lo scheduling).

**Câu chuyển sang ch. 5 (tr. 123):** đã bàn các công cụ **high-level**; giờ tới lúc nhìn công cụ **low-level** làm mọi thứ hoạt động — **C++ memory model và atomic operations**.

**🆕 Bảng chốt — chọn công cụ đồng bộ nào:**

| Cần | Dùng | Chuẩn |
|---|---|---|
| Chờ điều kiện lặp lại (queue có dữ liệu chưa) | `std::condition_variable` + `wait(lk, pred)` | C++11 |
| Chờ **sự kiện một-lần** + lấy giá trị | `std::future` + `std::async`/`packaged_task`/`promise` | C++11 |
| Chạy task nền lấy kết quả, đơn giản nhất | `std::async` (⚠️ ghi `std::launch::async`) | C++11 |
| Tách task khỏi nơi chạy (thread pool, GUI) | `std::packaged_task` | C++11 |
| Set kết quả từ nơi không phải lời gọi hàm | `std::promise` (`set_value`/`set_exception`) | C++11 |
| Nhiều thread cùng chờ một kết quả | `std::shared_future` (mỗi thread 1 copy) | C++11 |
| Chờ có deadline | `wait_until` + `steady_clock` | C++11 |
| Chờ N việc xong (một lần) | `std::latch` | **C++20** |
| Đồng bộ nhóm thread theo pha (lặp) | `std::barrier` | **C++20** |
| Chuỗi async không block | `.then` (⚠️ chưa chuẩn) / coroutine `co_await` | TS / C++20 |

---

## Góc interview

**Câu 1 (🎯🎯 kinh điển "while vs if"):** Vì sao khi dùng condition variable phải kiểm điều kiện trong **vòng lặp** (hoặc dùng predicate), không phải `if`? Vì sao `wait` cần `unique_lock` chứ không `lock_guard`?

<details><summary>Đáp án</summary>

**Phải dùng `while`/predicate vì hai lý do (tr. 76):**
1. **Spurious wakeup (đánh thức giả):** thread có thể thức dậy và kiểm điều kiện **mà không do ai `notify`**. Số lần/tần suất **bất định**. Sách chỉ rõ: một cài đặt `wait` hợp lệ chỉ là `while(!pred()){ lk.unlock(); lk.lock(); }` — không hề ngủ, không hề chờ notify → code phải chịu được điều đó.
2. 🆕 **Stolen wakeup:** giữa lúc thread A được notify và tái chiếm lock, thread B chen vào lấy mất điều kiện (vd pop hết queue) → A thức dậy nhưng điều kiện lại **không còn thỏa**.

→ Nếu dùng `if`, thread chạy tiếp khi điều kiện **chưa/không còn** thỏa → bug. `while` (hoặc predicate `wait(lk, pred)` đã bọc sẵn vòng lặp) buộc **kiểm lại** sau mỗi lần thức.
```cpp
// ✅ while, không if
while (!ready) cv.wait(lk);
// hoặc predicate-form (tương đương, khuyến nghị):
cv.wait(lk, []{ return ready; });
```
Lưu ý thêm: **predicate không được có side effect** vì có thể bị gọi nhiều lần bất định (tr. 76).

**Vì sao `unique_lock` không `lock_guard` (tr. 75–76):** `wait()` phải **unlock mutex khi ngủ** (để thread khác lock được mà thay đổi điều kiện + notify) rồi **lock lại khi thức**. `std::lock_guard` **không cho unlock/relock giữa chừng** — nó chỉ lock lúc construct, unlock lúc destruct. `unique_lock` mới có `lock()`/`unlock()` linh hoạt. Nếu mutex khóa suốt lúc ngủ → thread chuẩn bị dữ liệu không lock được để push → điều kiện **không bao giờ** thỏa → treo.

**Điểm cộng — thứ tự đúng của producer:** giữ lock → sửa dữ liệu chung → **nhả lock** → `notify_one()`. Nhả lock **trước** notify để thread thức dậy không phải block lại chờ mình unlock (tr. 75).

</details>

**Câu 2 (🎯 kinh điển):** So sánh `std::async`, `std::packaged_task`, `std::promise`. Khi nào dùng cái nào? Làm sao lấy **giá trị** và **exception** từ một thread khác?

<details><summary>Đáp án</summary>

**Ba cách tạo future — cao xuống thấp (tr. 82):**

| | `std::async` | `std::packaged_task` | `std::promise` |
|---|---|---|---|
| Là gì | Chạy một callable **nền**, trả `future` | **Bọc** callable, nối future vào; **gọi sau** | Set giá trị **tường minh** qua `set_value` |
| Lấy future | Giá trị trả về | `get_future()` | `get_future()` |
| Dùng khi | Chỉ cần "chạy nền lấy kết quả" | **Tách task khỏi nơi chạy** (thread pool, GUI thread) | Kết quả đến **không từ một lời gọi hàm** (network, sự kiện rời) |

```cpp
// async:
std::future<int> f = std::async(std::launch::async, compute, arg);
int r = f.get();                          // block tới khi xong

// packaged_task (chạy ở nơi khác):
std::packaged_task<int()> task(work);
std::future<int> f = task.get_future();   // lấy future TRƯỚC
some_queue.push(std::move(task));         // đưa đi nơi khác gọi

// promise (set tay):
std::promise<int> p;
std::future<int> f = p.get_future();
// ... thread khác:
p.set_value(42);                          // → f.get() trả 42
```

**Lấy giá trị:** `future::get()` — block tới khi sẵn sàng rồi trả (one-shot, gọi một lần).

**Lấy exception (tr. 89–90) — tính năng đắt giá:**
- **async/packaged_task:** nếu hàm ném exception, nó **lưu vào future**, và **`get()` ném lại** y như gọi trực tiếp:
  ```cpp
  std::future<double> f = std::async(square_root, -1);
  double y = f.get();     // ném std::out_of_range như square_root(-1) trực tiếp
  ```
- **promise:** dùng `set_exception()`:
  ```cpp
  try { p.set_value(calculate()); }
  catch (...) { p.set_exception(std::current_exception()); }
  // hoặc: p.set_exception(std::make_exception_ptr(std::logic_error("foo")));
  ```

**⚠️ Ba bẫy nên nêu:**
1. **`std::async` mặc định có thể deferred (tr. 84):** không ghi policy → implementation tự chọn; nếu deferred, task **không chạy** cho tới `wait()`/`get()` (có thể không bao giờ). Muốn chạy nền thật → **`std::launch::async` tường minh**.
2. **Future của `async` không giữ tên → destructor BLOCK** chờ task (bẫy `std::async(...)` không gán biến).
3. **broken_promise (tr. 90):** hủy `promise`/`packaged_task` mà không set value → future nhận `std::future_error` mã `broken_promise`, tránh cho thread chờ mãi.

</details>

**Câu 3 (🎯):** Viết một `threadsafe_queue` mà consumer **chờ** khi rỗng thay vì đốt CPU. Phân biệt `notify_one` và `notify_all`.

<details><summary>Đáp án</summary>

**Cốt lõi: condition variable + `wait_and_pop`** (Listing 4.5):
```cpp
template<typename T>
class threadsafe_queue {
    mutable std::mutex mut;                     // mutable: lock trong empty() const
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();                 // đánh thức MỘT consumer
    }
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);   // unique_lock cho wait()
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });  // chờ, không đốt CPU
        value = data_queue.front();
        data_queue.pop();
    }
    bool try_pop(T& value) {                     // biến thể KHÔNG chờ
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty()) return false;
        value = data_queue.front();
        data_queue.pop();
        return true;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
```
Điểm ăn điểm: **gộp `front`+`pop`** (race-trong-interface như `stack` ch. 3), **`mutable mutex`**, **`unique_lock` cho `wait`, `lock_guard` cho hàm không chờ**, **predicate chống spurious wakeup**.

**`notify_one` vs `notify_all` (tr. 80–81):**
- **`notify_one`** — nhiều consumer chia việc, **chỉ một** nên phản hồi mỗi item. Rẻ hơn. Không đảm bảo thread nào được đánh thức.
- **`notify_all`** — **mọi** thread chờ cần phản hồi (vd shared data vừa init xong, tất cả dùng chung; hoặc cập nhật định kỳ). Đánh thức tất cả để mỗi thread tự kiểm điều kiện.
- **Bẫy:** dùng `notify_all` khi chỉ cần một → **thundering herd** (đánh thức thừa, tranh lock, phí CPU). Dùng `notify_one` khi cần tất cả → thread bị bỏ ngủ. Với queue một-item-một-consumer thì **`notify_one`** là đúng.

</details>

**Câu 4 (🟠):** Chờ một sự kiện **có timeout 500ms** với condition variable. Vì sao dùng `steady_clock` và `wait_until` (mốc tuyệt đối) thay vì `wait_for` trong vòng lặp?

<details><summary>Đáp án</summary>

**Cách đúng (Listing 4.11, tr. 97):**
```cpp
bool wait_loop() {
    auto const timeout = std::chrono::steady_clock::now()
                       + std::chrono::milliseconds(500);   // tính MỐC 1 lần
    std::unique_lock<std::mutex> lk(m);
    while (!done) {
        if (cv.wait_until(lk, timeout) == std::cv_status::timeout)
            break;                                          // hết giờ thật sự
    }
    return done;
}
```

**Vì sao `wait_until` (mốc tuyệt đối) không `wait_for` trong loop (tr. 97):** phải loop để chống spurious wakeup. Nếu dùng **`wait_for(500ms)` trong loop**, mỗi lần spurious wakeup **reset lại 500ms** → tổng thời gian chờ **không bị chặn** (unbounded). Tính **một mốc tuyệt đối** rồi `wait_until` → dù thức dậy bao nhiêu lần, vẫn chờ tới đúng mốc đó → tổng **có chặn**.

**Vì sao `steady_clock` không `system_clock` (tr. 93–94):**
- `system_clock` **chỉnh được** (NTP, người dùng đổi giờ, DST) → `now()` có thể **nhảy lùi** → timeout tính sai, thậm chí chờ rất lâu hoặc rất ngắn.
- `steady_clock` **đơn điệu tăng, không chỉnh được** → 500ms là 500ms thời gian trôi thật. Chuẩn C++ đo duration-based wait bằng steady clock nội bộ chính vì lý do này (tr. 95–96).

**Điểm cộng:** muốn timeout khi **lock mutex** thì `std::mutex` không hỗ trợ — phải dùng `std::timed_mutex` với `try_lock_for()`/`try_lock_until()` (tr. 98).

</details>

**Câu 5 (🟠):** `std::future` và `std::shared_future` khác nhau thế nào? Nếu nhiều thread cần chờ cùng một kết quả thì làm sao cho an toàn?

<details><summary>Đáp án</summary>

**Khác biệt (tr. 81, 90):**
| | `std::future` | `std::shared_future` |
|---|---|---|
| Ownership | **Unique** (một instance/sự kiện) | **Shared** (nhiều instance) |
| Copy | Không (chỉ **move**) | **Có** |
| `get()` | **One-shot** — gọi xong hết giá trị | Nhiều lần, nhiều thread |
| Tương tự | `std::unique_ptr` | `std::shared_ptr` |

`std::future` chỉ cho **một** thread lấy kết quả — vì `get()` one-shot, chia sẻ cũng vô nghĩa (tr. 90).

**⚠️ Nhiều thread cùng chờ → `shared_future`, nhưng mỗi thread một COPY riêng (Figure 4.1, tr. 91):**
- Member function trên **một** `shared_future` object **vẫn không tự đồng bộ** → nhiều thread đụng **cùng object** mà không lock = **data race, UB**.
- **Cách đúng:** truyền **bản copy** cho từng thread. Mỗi thread `get()`/`wait()` trên **object local của nó**; phần shared state bên trong đã được thư viện đồng bộ.
```cpp
std::promise<int> p;
std::shared_future<int> sf = p.get_future().share();   // tạo shared_future
// mỗi thread:
std::thread t([sf]{ /* copy riêng */ int v = sf.get(); ... });
```

**Tạo từ `future`:** `std::shared_future<int> sf(std::move(f));` hoặc `auto sf = f.share();` (chuyển ownership, `f` rỗng sau đó).

**Ứng dụng (tr. 91):** bảng tính song song — một ô giá trị cuối được nhiều công thức ô khác dùng; ô phụ thuộc block trên `shared_future` tới khi ô nguồn sẵn sàng.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [ostep/concurrency.md](../ostep/concurrency.md) — condition variable & semaphore từ tầng OS: producer-consumer, vì sao dùng `while` không `if`, Mesa vs Hoare semantics.
- [03-operating-system/sync-primitives.md](../../03-operating-system/sync-primitives.md) — bản cô đọng condition variable/semaphore của repo.
- [EMC++ cụm 7](../effective-modern-cpp.md) — Item 35 (`std::async` vs `std::thread`), Item 36 (`std::launch::async` khi cần async thật), Item 38–39 (`std::future` destructor block, chờ one-shot event bằng `void` future).
- [lkd/03-sync-timers.md](../lkd/03-sync-timers.md) — completion & wait queue trong kernel — cùng bài toán "chờ sự kiện không đốt CPU" ở tầng kernel.
- Ch. 5 (memory model) giải thích khái niệm **synchronizes-with**/**happens-before** mà latch dùng (tr. 119); ch. 9 xây thread pool trên nền `packaged_task`.

**Chương tiếp theo:** [Ch. 5 — The C++ memory model and operations on atomic types →](05-memory-model.md) 🎯🎯 (memory location & modification order, atomic types, **synchronizes-with / happens-before**, 6 `memory_order`, release sequence, fences) — công cụ **low-level** làm mọi thứ ở trên hoạt động, và là **chương giá trị interview cao nhất sách**.
