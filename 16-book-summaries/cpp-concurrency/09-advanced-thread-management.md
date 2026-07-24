# Ch. 9 — Advanced thread management (tr. 300–326) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 8 — Designing concurrent code](08-designing-concurrent-code.md)** · **[Ch. 10 → Parallel algorithms](10-parallel-algorithms.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Chương thực hành: xây thread pool + cơ chế interrupt thread.** Hai chủ đề: (1) **thread pool** tiến hóa từ đơn giản → work-stealing (câu hỏi thiết kế hay gặp cho System SW); (2) **interrupting threads** — sách tự xây (2019 chưa có chuẩn), nhưng **C++20 đã chuẩn hóa** thành `std::jthread` + `std::stop_token` (⚠️ ghi chú xuyên suốt).

**Sách nêu (tr. 300):** kỹ thuật quản lý thread nâng cao — thread pool và interrupting threads.

---

## Cụm 1 — Thread pool: tiến hóa 5 bước (tr. 300–315)

**Ẩn dụ (tr. 300):** công ty có "car pool" — số xe hữu hạn dùng chung thay vì mỗi nhân viên một xe. Thread pool tương tự: **không thực tế tạo một thread cho mỗi task**; thay vào đó có **nhóm worker thread cố định** lấy task từ **queue** để chạy.

### 1.1 Thread pool đơn giản nhất (Listing 9.1, tr. 301–303)

Số worker cố định = `hardware_concurrency()`; task đẩy vào `threadsafe_queue<std::function<void()>>`; worker lặp: lấy task → chạy → lấy tiếp; rỗng thì `std::this_thread::yield()`.

```cpp
class thread_pool {
    std::atomic_bool done;
    threadsafe_queue<std::function<void()>> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner;                      // RAII join mọi thread (ch. 8)
    void worker_thread() {
        while (!done) {
            std::function<void()> task;
            if (work_queue.try_pop(task)) task();
            else std::this_thread::yield();
        }
    }
public:
    thread_pool() : done(false), joiner(threads) {
        unsigned const n = std::thread::hardware_concurrency();
        try { for (unsigned i = 0; i < n; ++i)
                  threads.push_back(std::thread(&thread_pool::worker_thread, this)); }
        catch (...) { done = true; throw; }   // spawn fail → dừng sạch
    }
    ~thread_pool() { done = true; }
    template<typename F> void submit(F f) { work_queue.push(std::function<void()>(f)); }
};
```
**⚠️ Thứ tự khai báo member quan trọng (tr. 303):** `done` + `work_queue` **trước** `threads` **trước** `joiner` → hủy đúng thứ tự (không hủy queue khi thread còn chạy). **Hạn chế:** không chờ task được, không trả giá trị, không xử lý blocking task, có thể deadlock. Việc đơn giản thì `std::async` (ch. 8) còn tốt hơn.

### 1.2 Task chờ được + trả giá trị (Listing 9.2, tr. 303–307)

`submit` trả **`std::future`**; task bọc `std::packaged_task` (truyền cả kết quả lẫn exception). **⚠️ `packaged_task` chỉ movable, không copyable → `std::function` không chứa được** → tự viết **`function_wrapper`** (type-erasure, move-only). Dùng `std::result_of<F()>::type` (🆕 C++17 dùng `std::invoke_result_t`) suy ra kiểu trả về.

**`parallel_accumulate` với pool này (Listing 9.3, tr. 306):** làm việc theo **số BLOCK** (không phải số thread) — chia thành khối nhỏ nhất đáng làm concurrent, pool tự scale. **⚠️ Khối quá nhỏ → overhead submit/future lấn át → chậm hơn một thread.** Thread pool tự lo exception safety (exception qua future; destructor bỏ task chưa xong, chờ thread xong).

### 1.3 Task chờ task khác — nguy cơ DEADLOCK (Listing 9.4/9.5, tr. 307–309)

**⚠️ Vấn đề (tr. 307):** quicksort đệ quy submit task vào pool rồi **chờ** nó. Với số thread hữu hạn, **mọi thread có thể đang chờ task chưa được schedule vì không còn thread rảnh** → deadlock. Giải: **thread chờ thì tự chạy task khác trong queue** — thêm `run_pending_task()`:
```cpp
void thread_pool::run_pending_task() {
    function_wrapper task;
    if (work_queue.try_pop(task)) task();
    else std::this_thread::yield();
}
```
Quicksort: `while (new_lower.wait_for(0s) == timeout) pool.run_pending_task();` — chờ nửa dưới bằng cách **chạy task đang chờ** thay vì block. Giải đúng vấn đề deadlock (như "helping" ch. 7).

### 1.4 Tránh contention trên queue: local queue (Listing 9.6, tr. 310–311)

**⚠️ Vấn đề (tr. 310):** mọi `submit`/`run_pending_task` đụng **một queue chung** → càng nhiều processor càng contention; kể cả lock-free queue vẫn **cache ping-pong** (ch. 8). Giải: **mỗi thread một local queue** (`thread_local std::queue`) — worker đẩy task mới vào **queue riêng**, chỉ lấy từ pool queue khi local rỗng. Local queue là `std::queue` thường (chỉ một thread đụng → khỏi khóa).
```cpp
void run_pending_task() {
    function_wrapper task;
    if (local_work_queue && !local_work_queue->empty()) { /* lấy từ LOCAL */ }
    else if (pool_work_queue.try_pop(task)) { /* lấy từ POOL chung */ }
    else std::this_thread::yield();
}
```
**⚠️ Nhược:** phân bố việc không đều → một thread đầy queue, thread khác đói (quicksort: chỉ chunk trên cùng vào pool queue, còn lại kẹt local queue của một thread) → **triệt tiêu lợi ích pool**.

### 1.5 Work stealing (Listing 9.7/9.8, tr. 311–315)

Giải bài đói: **thread rảnh "trộm" task từ queue thread khác.** Cần `work_stealing_queue` (owner push/pop một đầu, kẻ trộm lấy đầu kia). Sách dùng **mutex bảo vệ** (giả định trộm hiếm → ít contention), lock-free deque nằm ngoài phạm vi sách.
```cpp
void run_pending_task() {
    task_type task;
    if (pop_task_from_local_queue(task) ||       // 1. queue riêng
        pop_task_from_pool_queue(task) ||        // 2. pool chung
        pop_task_from_other_thread_queue(task))  // 3. TRỘM từ thread khác
        task();
    else std::this_thread::yield();
}
```
**⚠️ Chống mọi thread cùng trộm từ thread đầu (tr. 315):** mỗi thread bắt đầu quét từ **thread KẾ tiếp** (offset index theo index của mình). 🆕 Đây là kiến trúc của các thread pool thật (Intel TBB, .NET ThreadPool, Go runtime, Rust rayon).

### Insight đáng nhớ (Cụm 1)
- **Thread pool tiến hóa: đơn giản → waitable (future) → run_pending_task (chống deadlock) → local queue (giảm contention) → work stealing (cân bằng tải).** Mỗi bước sửa đúng một khiếm khuyết.
- **🎯 Task chờ task khác trong pool = nguy cơ deadlock** (mọi thread chờ task chưa schedule). Giải: **thread chờ tự chạy task khác** (`run_pending_task`) — cùng ý "helping" ch. 7.
- **Local queue + work stealing** = mẫu chuẩn của thread pool hiện đại: local queue giảm cache ping-pong, work stealing cân bằng tải khi phân bố không đều.
- **⚠️ Khối quá nhỏ → overhead pool lấn át.** Chọn granularity task đủ lớn để đáng submit.

---

## Cụm 2 — Interrupting threads (tr. 315–326)

**⚠️ Cập nhật quan trọng nhất chương:** sách viết 2019, **C++ chưa có cơ chế interrupt** nên tự xây từ đầu. **C++20 đã chuẩn hóa** (chính đề xuất P0660 của các tác giả gồm Anthony Williams) thành **`std::jthread` + `std::stop_token`/`std::stop_source`/`std::stop_callback`**. 🆕 **Trong code mới dùng bản chuẩn, không tự viết** — nhưng đọc để hiểu cơ chế bên dưới.

### 2.1 Interface + interrupt flag (Listing 9.9, tr. 316–318)

`interruptible_thread` = `std::thread` + hàm `interrupt()`. Cơ chế: **`thread_local interrupt_flag`** cho mỗi thread; `interrupt()` set flag của thread đó. Constructor dùng `std::promise`/`future` để lấy địa chỉ `this_thread_interrupt_flag` của thread mới:
```cpp
thread_local interrupt_flag this_thread_interrupt_flag;
class interruptible_thread {
    std::thread internal_thread;
    interrupt_flag* flag;
public:
    template<typename F> interruptible_thread(F f) {
        std::promise<interrupt_flag*> p;
        internal_thread = std::thread([f, &p]{
            p.set_value(&this_thread_interrupt_flag);   // gửi địa chỉ flag về constructor
            f();
        });
        flag = p.get_future().get();                    // constructor chờ tới khi có flag
    }
    void interrupt() { if (flag) flag->set(); }
};
```

### 2.2 Phát hiện interrupt: `interruption_point()` (tr. 318)

Thread phải **tự kiểm** flag; `interruption_point()` ném `thread_interrupted` nếu flag set:
```cpp
void interruption_point() {
    if (this_thread_interrupt_flag.is_set()) throw thread_interrupted();
}
void foo() {
    while (!done) { interruption_point(); process_next_item(); }
}
```
**⚠️ Chưa lý tưởng (tr. 318):** chỗ tốt nhất để interrupt là khi thread **đang block chờ** — mà lúc đó nó **không chạy để gọi `interruption_point()`**. Cần `interruptible_wait()`.

### 2.3 Interrupt một `condition_variable` wait (Listing 9.10–9.11, tr. 318–320)

**⚠️ Bản ngây thơ HỎNG (Listing 9.10):** hai lỗi — (1) `cv.wait()` có thể ném → không clear association (sửa bằng RAII destructor); (2) **race condition:** nếu interrupt xảy ra **giữa `interruption_point()` và `wait()`**, thread chưa wait nên notify không đánh thức được → miss interrupt. Không sửa được an toàn (truyền reference mutex sang thread interrupt → nguy cơ deadlock/dùng mutex đã hủy).

**Giải thực dụng (Listing 9.11, tr. 320):** dùng **`wait_for()` với timeout nhỏ (1ms)** thay `wait()` → đặt giới hạn trên cho độ trễ thấy interrupt. Đổi lại **nhiều spurious wake hơn** (do timeout) — chấp nhận được. `condition_variable_any` (Listing 9.12) làm được sạch hơn vì custom mutex lock được cả interrupt flag.

**Interrupt các blocking khác (tr. 322):** `future` — loop với `wait_for` timeout nhỏ trong `interruptible_wait`.

### 2.4 Xử lý interrupt + ví dụ (Listing 9.13, tr. 323–326)

`thread_interrupted` là exception thường → **catch được**:
```cpp
try { do_something(); }
catch (thread_interrupted&) { handle_interruption(); }
```
**⚠️ Nếu để exception thoát khỏi hàm thread → `std::terminate`.** Giải: đặt `catch(thread_interrupted)` trong **wrapper** của constructor (Listing tr. 324–325) → an toàn cho exception propagate mà chỉ kết thúc thread đó.

**Ví dụ (Listing 9.13, tr. 325–326):** desktop search — background thread monitor filesystem, chạy trọn đời app. Khi thoát: **interrupt TẤT CẢ thread trước, rồi mới join từng cái.**
```cpp
for (auto& t : background_threads) t.interrupt();   // interrupt HẾT trước
for (auto& t : background_threads) t.join();        // RỒI mới join
```
**⚠️ Vì sao interrupt hết rồi mới join (tr. 326):** thread không dừng ngay khi interrupt (phải tới interruption point kế + chạy destructor/handler). Nếu join ngay từng cái → thread interrupt phải chờ, dù còn việc (interrupt thread khác). Interrupt hết trước → **các thread xử lý interrupt SONG SONG** → xong sớm hơn.

### Insight đáng nhớ (Cụm 2)
- **🆕 C++20 `std::jthread` + `std::stop_token` thay toàn bộ cơ chế tự-viết này.** Trong code mới: `jthread` nhận callable có tham số đầu `std::stop_token st`, kiểm `st.stop_requested()`; destructor tự `request_stop()` + join. Nêu được điều này = điểm cộng.
- **Cooperative interruption:** thread bị interrupt **tự quyết** làm gì (catch để tiếp, hoặc để propagate kết thúc). Không "giật thảm" đột ngột như kill — an toàn hơn.
- **Interrupt hết trước, join sau** → xử lý interrupt song song, dừng nhanh hơn.
- **⚠️ Interrupt condition variable wait khó làm sạch** — giải thực dụng là `wait_for` timeout nhỏ (đánh đổi spurious wake).

---

## Tóm tắt chương (theo sách, tr. 326)

Kỹ thuật quản lý thread nâng cao: **thread pool** (local work queue + work stealing giảm overhead đồng bộ; `run_pending_task` khi chờ subtask loại bỏ deadlock) và **interrupting threads** (interruption point + interruptible wait).

**🆕 Bảng chốt — thread pool & interrupt:**
| Chủ đề | Kỹ thuật | C++20 thay thế |
|---|---|---|
| Thread pool cơ bản | worker + shared queue + `hardware_concurrency` | — (tự xây) |
| Chờ task + kết quả | `packaged_task` + `future` + `function_wrapper` | — |
| Task chờ task khác | `run_pending_task` (chống deadlock) | — |
| Giảm contention | local queue (thread_local) | — |
| Cân bằng tải | work stealing (trộm từ thread khác) | — |
| Interrupt thread | `thread_local interrupt_flag` + `interruption_point` | **`std::jthread` + `std::stop_token`** |

---

## Góc interview

**Câu 1 (🎯 câu thiết kế hay gặp):** Thiết kế một **thread pool**. Các vấn đề chính phải giải quyết là gì?

<details><summary>Đáp án</summary>

**Cấu trúc cơ bản (Listing 9.1):** N worker cố định (= `hardware_concurrency()`), một `threadsafe_queue` chứa task; worker lặp lấy-task-chạy-yield; RAII (`join_threads`) join khi hủy. **⚠️ Thứ tự khai báo member:** `done` + `queue` trước `threads` trước `joiner` (hủy đúng thứ tự).

**Năm vấn đề phải giải (tiến hóa của chương):**
1. **Chờ task + lấy kết quả:** `submit` trả `std::future`; task bọc `packaged_task` (truyền cả kết quả lẫn exception). ⚠️ `packaged_task` move-only → `std::function` không chứa được → tự viết `function_wrapper` type-erasure.
2. **🎯 Task chờ task khác → DEADLOCK:** quicksort đệ quy submit rồi chờ; số thread hữu hạn → mọi thread có thể chờ task chưa schedule. Giải: **`run_pending_task()`** — thread chờ thì **tự chạy task khác** thay vì block.
3. **Contention trên queue chung:** mọi submit/pop đụng một queue → cache ping-pong (ch. 8). Giải: **local queue mỗi thread** (`thread_local`), chỉ lấy pool queue khi local rỗng.
4. **Phân bố tải không đều:** một thread đầy queue, thread khác đói. Giải: **work stealing** — thread rảnh trộm task từ queue thread khác (bắt đầu quét từ thread kế để tránh đụng độ).
5. **Exception safety:** exception task qua future; destructor bỏ task chưa xong, chờ thread xong.

**⚠️ Chọn granularity:** làm việc theo **số block**, không phải số thread → pool tự scale. Nhưng block quá nhỏ → overhead submit/future lấn át → chậm hơn một thread.

**Điểm cộng:** local queue + work stealing chính là kiến trúc thread pool thật (Intel TBB, Go scheduler, Rust rayon, .NET ThreadPool). Việc đơn giản độc lập thì `std::async` còn tốt hơn (ch. 8).

</details>

**Câu 2 (🟠):** Làm sao **dừng an toàn** một long-running thread (worker/background)? C++20 hỗ trợ gì?

<details><summary>Đáp án</summary>

**Nguyên tắc: cooperative interruption (tr. 315–316)** — báo cho thread nên dừng, để nó **kết thúc gọn gàng** (chạy destructor/cleanup) thay vì "giật thảm" đột ngột (không có `kill thread` an toàn).

**Cơ chế tự xây (sách, 2019):**
- **`thread_local interrupt_flag`** cho mỗi thread; `interrupt()` set flag.
- **`interruption_point()`** — thread tự kiểm flag, ném `thread_interrupted` nếu set; gọi ở chỗ an toàn để dừng.
- **`interruptible_wait()`** — cho blocking wait (chỗ tốt nhất để interrupt là khi thread đang block, nhưng lúc đó nó không chạy để gọi interruption_point). ⚠️ Interrupt `condition_variable` wait khó làm sạch (race giữa check và wait) → giải thực dụng: **`wait_for` timeout nhỏ (1ms)**, đổi lấy nhiều spurious wake.
- Xử lý: `catch(thread_interrupted&)` — tiếp tục hoặc để propagate kết thúc thread. ⚠️ Để exception thoát khỏi hàm thread → `std::terminate` → đặt catch trong wrapper.
- **⚠️ Interrupt HẾT trước rồi join sau** (tr. 326): thread không dừng ngay → interrupt hết để chúng xử lý song song, dừng nhanh hơn.

**🆕 C++20 — dùng bản chuẩn, không tự viết:** đề xuất P0660 (chính các tác giả sách) đã thành **`std::jthread` + `std::stop_token`/`std::stop_source`/`std::stop_callback`**:
```cpp
std::jthread t([](std::stop_token st) {
    while (!st.stop_requested()) {        // hợp tác dừng
        do_next_operation();
    }
});
// t.request_stop();  ← hoặc để destructor tự gọi
```
`jthread` destructor **tự `request_stop()` rồi `join()`**. `stop_callback` cho phép chạy hàm khi có stop request (vd đánh thức condition variable). Đây là cách chuẩn, an toàn hơn hẳn tự xây.

**Điểm cộng — liên hệ:** cooperative cancellation cũng là mô hình của Go (`context.Context`), Java (`Thread.interrupt()`), Rust (channel/`AtomicBool`). Không dùng "force kill" vì để thread cleanup an toàn.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [02-managing-threads.md](02-managing-threads.md) — `std::jthread` + `std::stop_token` (C++20) chi tiết; `joining_thread` (Listing 2.7) là tiền thân.
- [08-designing-concurrent-code.md](08-designing-concurrent-code.md) — `join_threads` RAII, cache ping-pong (lý do dùng local queue), oversubscription.
- [04-synchronization.md](04-synchronization.md) — `packaged_task`, `future`, condition variable wait (nền của interruptible_wait).
- [07-lock-free-structures.md](07-lock-free-structures.md) — "helping" (thread chờ tự làm việc) — cùng ý `run_pending_task`.
- [10-thinking/system-design.md](../../10-thinking/system-design.md) — thiết kế thread pool ở tầng kiến trúc hệ thống.

**Chương tiếp theo:** [Ch. 10 — Parallel algorithms →](10-parallel-algorithms.md) (parallel algorithms C++17: **execution policies** `seq`/`par`/`par_unseq`, cách dùng) — dùng thread pool + parallel design để chạy thuật toán Standard Library song song.
