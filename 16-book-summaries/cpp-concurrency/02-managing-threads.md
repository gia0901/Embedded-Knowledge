# Ch. 2 — Managing threads (tr. 16–35) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 1 — Hello, world of concurrency](01-hello-concurrency.md)** · **[Ch. 3 → Sharing data between threads](03-sharing-data.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Đây là chương API nền.** Chương 1 nói *vì sao*; chương này nói *làm thế nào*: khởi chạy thread, quyết định chờ hay thả, truyền tham số an toàn, chuyển quyền sở hữu, chọn số thread, nhận diện thread. Gần như **mọi bug vòng đời thread trong C++ đều nằm trong chương này**, nên nó có mật độ câu hỏi phỏng vấn rất cao dù chỉ 20 trang.

**Sách tự nêu 3 mục tiêu của chương (tr. 16):** khởi chạy thread và các cách chỉ định code chạy trên đó · **chờ thread xong (join) so với để nó tự chạy (detach)** · nhận diện thread một cách duy nhất.

---

## Cụm 1 — Vòng đời thread: khởi chạy, join, detach (tr. 17–24)

### 1.1 Nền tảng: mọi chương trình C++ đều đã có sẵn một thread (tr. 17)

Câu mở đầu mục 2.1 rất đáng nhớ vì nó định khung mọi thứ sau đó:

> *"Every C++ program has at least one thread, which is started by the C++ runtime: the thread running `main()`."* (tr. 17)

Từ đó, chương trình có thể **khởi chạy thêm thread** với **một hàm khác làm entry point**. Các thread này chạy **đồng thời với nhau và với thread khởi đầu**. Và có một đối xứng đẹp: *"In the same way that the program exits when it returns from `main()`, when the specified entry point function returns, the thread exits."* (tr. 17) — tức **hàm entry point return = thread kết thúc**.

Ta gọi thread chạy `main()` là **initial thread** (thread khởi đầu). Toàn bộ chương 2 xoay quanh một object duy nhất: **`std::thread`** — và điều quan trọng nhất phải nắm ngay từ đầu:

> 🆕 **`std::thread` KHÔNG PHẢI là thread.** Nó là một **object quản lý (handle) sở hữu** một thread of execution của OS. Thread thật do OS tạo; `std::thread` chỉ nắm quyền sở hữu nó. Phân biệt này giải thích *toàn bộ* các hành vi lạ trong chương: vì sao destructor gọi `std::terminate`, vì sao object movable-not-copyable, vì sao `join()` chỉ gọi được một lần. Sách nói ý này ở tr. 27 (*"they do own a resource: each instance is responsible for managing a thread of execution"*), nhưng nắm trước sẽ đọc dễ hơn nhiều.

### 1.2 Khởi chạy thread: `std::thread` nhận **bất kỳ callable nào** (tr. 17–18)

Khởi chạy thread **luôn quy về một việc**: construct một object `std::thread` chỉ định task cần chạy (tr. 17).

**Dạng đơn giản nhất — hàm thường, không tham số, trả `void`:**

```cpp
#include <thread>              // BẮT BUỘC: định nghĩa lớp std::thread

void do_some_work();
std::thread my_thread(do_some_work);   // thread mới chạy do_some_work() rồi dừng
```

Sách nhấn: task chạy trên thread riêng **cho tới khi hàm return, rồi thread dừng** (tr. 17).

**Dạng thứ hai — function object (callable type).** `std::thread` làm việc với **bất kỳ callable type nào**, giống phần lớn Standard Library (tr. 17):

```cpp
class background_task {
public:
    void operator()() const {      // function call operator → object này "gọi được"
        do_something();
        do_something_else();
    }
};

background_task f;
std::thread my_thread(f);          // object f được COPY vào bộ nhớ của thread mới
```

**⚠️ Bẫy ở đây (tr. 17):** *"the supplied function object is **copied into the storage belonging to the newly created thread** of execution and invoked from there. It's therefore essential that the copy behaves equivalently to the original, or the result may not be what's expected."* → Nếu function object của bạn có state, **thread chạy trên BẢN SAO**, không phải object gốc. Sửa state bên trong thread **không phản ánh ra object gốc**. (Đây cũng là gợi ý sớm cho Listing 2.1 phía dưới: copy object là an toàn, nhưng copy một object *chứa reference* thì reference vẫn trỏ ra ngoài.)

### 1.3 ⚠️ "C++'s most vexing parse" — cái bẫy cú pháp kinh điển (tr. 18)

Nếu truyền một **temporary** thay vì biến có tên, cú pháp trùng với **khai báo hàm**, và compiler sẽ hiểu là khai báo hàm:

```cpp
std::thread my_thread(background_task());   // ⚠️ KHÔNG tạo thread nào cả!
```

Sách giải nghĩa chính xác dòng này khai báo cái gì (tr. 18): một **hàm** tên `my_thread`, nhận **một tham số** kiểu *pointer-to-a-function-taking-no-parameters-and-returning-a-`background_task`-object*, và **trả về** `std::thread`.

**Ba cách tránh (tr. 18):**

```cpp
background_task f;
std::thread t0(f);                             // (a) đặt tên cho function object

std::thread t1((background_task()));           // (b) thêm một cặp ngoặc ngoài
std::thread t2{background_task()};             // (c) uniform initialization (ngoặc nhọn)
```

**Cách thứ tư — lambda, và là cách sách khuyên (tr. 18):** lambda *"avoids this problem"*, đồng thời cho phép **capture biến local**, tránh phải truyền thêm tham số:

```cpp
std::thread my_thread([]{          // lambda không capture: viết ngay tại chỗ
    do_something();
    do_something_else();
});
```

> 🆕 Trong code hiện đại, **lambda là mặc định** để khởi chạy thread: rõ ràng, không dính most vexing parse, và đọc thấy ngay thread làm gì mà không phải nhảy đi tìm định nghĩa hàm. Nhưng phải cẩn thận capture: `[&]` capture-by-reference dẫn thẳng tới bug dangling reference ở mục 1.5 dưới đây.

### 1.4 Luật vàng: **phải quyết định join hay detach trước khi `std::thread` bị hủy** (tr. 18)

Đây là **câu quan trọng nhất của cả chương**:

> *"Once you've started your thread, you need to explicitly decide whether to wait for it to finish (by joining with it...) or leave it to run on its own (by detaching it...). If you don't decide before the `std::thread` object is destroyed, **then your program is terminated** (the `std::thread` destructor calls `std::terminate()`)."* (tr. 18)

Ba hệ quả sách nêu ngay sau đó (tr. 18), đều hay bị hiểu sai:

1. **Phải đúng cả khi có exception.** *"It's therefore imperative that you ensure that the thread is correctly joined or detached, **even in the presence of exceptions**."* → dẫn thẳng tới mục 2.1.3 (RAII).
2. **Chỉ cần quyết định trước khi OBJECT bị hủy** — bản thân thread có thể đã xong từ lâu trước khi bạn join/detach. Không có yêu cầu về thời điểm nào khác.
3. **Detach xong thread vẫn sống tiếp:** *"if you detach it, then if the thread is still running, it will continue to do so, and may continue running long after the `std::thread` object is destroyed; it will only stop running when it finally returns from the thread function."*

> 🆕 **Vì sao standard chọn `terminate()` chứ không tự động join?** Vì cả hai lựa chọn ngầm đều sai trong một nửa số trường hợp: tự động join có thể **treo chương trình vô hạn** ở một chỗ không ai ngờ; tự động detach thì **im lặng để lại dangling reference** như Listing 2.1. Standard chọn cách ồn ào nhất — bắt lập trình viên nói rõ ý định. (Sách nói cùng logic ở tr. 28 khi giải thích move-assignment cũng `terminate` *"for consistency with the `std::thread` destructor"*.)

### 1.5 Bug kinh điển #1: thread giữ reference tới biến local đã chết (tr. 18–20)

Sách nêu nguyên tắc trước: *"If you don't wait for your thread to finish, you need to **ensure that the data accessed by the thread is valid until the thread has finished with it**"* (tr. 18–19). Đây **không phải vấn đề mới** — truy cập object đã hủy là UB kể cả trong code single-thread — nhưng *"the use of threads provides an additional opportunity to encounter such lifetime issues"* (tr. 19).

**Listing 2.1 (tr. 19) — hàm return trong khi thread vẫn còn truy cập biến local:**

```cpp
struct func {
    int& i;                                // ⚠️ giữ REFERENCE, không phải bản sao
    func(int& i_) : i(i_) {}

    void operator()() {
        for (unsigned j = 0; j < 1000000; ++j) {
            do_something(i);               // ⚠️ có thể truy cập DANGLING REFERENCE
        }
    }
};

void oops() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    my_thread.detach();                    // KHÔNG chờ thread xong
}                                          // → some_local_state bị HỦY tại đây,
                                           //   nhưng thread mới có thể vẫn đang chạy
```

Chú ý mấu chốt: object `my_func` **được copy** vào thread (như mục 1.2 đã nói), **nhưng bản sao vẫn chứa `int& i` trỏ về `some_local_state` của `oops()`**. Copy object không cứu được gì khi object chứa reference.

**Table 2.1 (tr. 19–20)** — sách vẽ dòng thời gian hai cột, đây là bản dựng lại đầy đủ:

```
      Main thread                                 New thread
      ─────────────────────────────────────       ──────────────────────────────────────────
 (1)  Construct my_func với reference tới
      some_local_state
 (2)  Khởi chạy thread my_thread
                                          ──────► Started
                                                  Gọi func::operator()
 (3)  Detach my_thread                            Đang chạy func::operator();
                                                  có thể gọi do_something(some_local_state)
 (4)  HỦY some_local_state          ✗             Vẫn đang chạy
 (5)  Thoát khỏi oops()                           Vẫn chạy func::operator(); vẫn gọi
                                                  do_something với reference tới
                                                  some_local_state  ═══► UNDEFINED BEHAVIOR
```

**Hai cách xử lý sách đưa ra (tr. 20):**

1. **Làm thread function tự chứa (self-contained): COPY dữ liệu vào thread thay vì chia sẻ.** Vì callable object được copy vào thread nên object gốc hủy ngay cũng không sao — **nhưng vẫn phải cảnh giác với object chứa pointer/reference như Listing 2.1**.
   → Quy tắc sách chốt: *"it's a bad idea to create a thread within a function that has access to the local variables in that function, **unless the thread is guaranteed to finish before the function exits**."*
2. **Bảo đảm thread xong trước khi hàm thoát — bằng `join()`.**

> 🆕 Phiên bản lambda của đúng bug này, hay gặp hơn trong code thật:
> ```cpp
> void oops2() {
>     int state = 0;
>     std::thread t([&]{ heavy(state); });  // ⚠️ capture BY REFERENCE
>     t.detach();
> }                                          // state chết, lambda vẫn cầm reference
> ```
> Sửa: capture **by value** `[state]`, hoặc `join()` trước khi thoát scope.

### 1.6 `join()` — chờ thread xong (tr. 20)

Thay `my_thread.detach()` bằng `my_thread.join()` ở Listing 2.1 là đủ để thread chắc chắn xong **trước khi hàm thoát**, tức trước khi biến local bị hủy (tr. 20).

Sách rất thẳng thắn về giới hạn của `join()`:

> *"`join()` is a simple and **brute-force** technique—either you wait for a thread to finish or you don't. If you need more fine-grained control over waiting for a thread, such as **to check whether a thread is finished, or to wait only a certain period of time**, then you have to use alternative mechanisms such as **condition variables and futures**, which we'll look at in chapter 4."* (tr. 20)

**Hai tính chất phải nhớ của `join()` (tr. 20):**

- `join()` **dọn luôn storage** gắn với thread → sau đó object `std::thread` **không còn gắn với thread nào**.
- Vì thế **chỉ gọi `join()` được ĐÚNG MỘT LẦN**; sau khi gọi, object không còn `joinable()` nữa (`joinable()` trả `false`).

```cpp
std::thread t(work);
assert(t.joinable());   // true: đang sở hữu một thread
t.join();
assert(!t.joinable());  // false: đã "nhả" thread — gọi join() lần nữa là UB/exception
```

> 🆕 **`joinable()` nghĩa là gì cho chính xác:** *"object này có đang sở hữu một thread cần được join hoặc detach không"*. Nó **không** trả lời "thread đã chạy xong chưa". Một thread đã chạy xong nhưng chưa được join **vẫn** `joinable() == true`. Đây là chỗ hay nhầm khi đọc code `if (t.joinable()) t.join();`.

### 1.7 Bug kinh điển #2: exception làm nhảy qua `join()` → RAII (tr. 20–22)

**Vấn đề (tr. 20–21):** nếu định `join()`, phải chọn **chỗ đặt lệnh join rất cẩn thận**, vì *"the call to `join()` is liable to be skipped if an exception is thrown after the thread has been started but before the call to `join()`"* — mà bỏ qua join thì destructor `std::thread` gọi `terminate()` → **cả chương trình chết**.

**Listing 2.2 (tr. 21) — cách thủ công, dùng try/catch:**

```cpp
struct func;                              // xem định nghĩa ở Listing 2.1

void f() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    try {
        do_something_in_current_thread(); // ← chỗ này có thể ném exception
    }
    catch (...) {
        t.join();                         // (1) đường thoát BẤT THƯỜNG: vẫn phải join
        throw;                            //     rồi ném tiếp cho tầng trên
    }
    t.join();                             // (2) đường thoát BÌNH THƯỜNG
}
```

Sách chê ngay chính giải pháp này (tr. 21): *"The use of try/catch blocks is **verbose**, and it's **easy to get the scope slightly wrong**, so this isn't an ideal scenario."* Yêu cầu thật sự là: đúng trên **mọi đường thoát** (bình thường lẫn exception), và **cơ chế phải đơn giản, ngắn gọn**.

**Listing 2.3 (tr. 21–22) — dùng RAII (Resource Acquisition Is Initialization): một class join trong destructor:**

```cpp
class thread_guard {
    std::thread& t;                       // giữ REFERENCE tới thread bên ngoài
public:
    explicit thread_guard(std::thread& t_) : t(t_) {}

    ~thread_guard() {
        if (t.joinable()) {               // (1) BẮT BUỘC kiểm tra: join 2 lần là sai
            t.join();
        }
    }

    thread_guard(thread_guard const&) = delete;              // (2) cấm copy
    thread_guard& operator=(thread_guard const&) = delete;   //     và copy-assign
};

struct func;                              // xem Listing 2.1

void f() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);                    // ← từ đây trở đi mọi đường thoát đều an toàn

    do_something_in_current_thread();     // ném exception cũng không sao
}                                         // g bị hủy TRƯỚC t → join xảy ra đúng lúc
```

Sách giải thích ba chi tiết, cả ba đều là câu hỏi phỏng vấn tiềm năng (tr. 22):

1. **Thứ tự hủy:** *"the local objects are destroyed in reverse order of construction"* → `g` được construct **sau** `t` nên bị hủy **trước** `t` ⇒ join xong rồi `t` mới hủy ⇒ không bao giờ chạm vào destructor "chưa quyết định" của `std::thread`. *"This even happens if the function exits because `do_something_in_current_thread` throws an exception."*
2. **Vì sao phải `if (t.joinable())`:** vì `join()` chỉ gọi được một lần cho một thread — nếu thread đã được join ở chỗ khác rồi thì gọi nữa là sai.
3. **Vì sao `=delete` copy:** *"Copying or assigning such an object would be dangerous, because it **might then outlive the scope of the thread it was joining**."* Khai báo deleted để mọi mưu toan copy **fail ngay lúc biên dịch**.

> ⚠️ **Điểm yếu còn lại của `thread_guard`:** nó giữ `std::thread&` — tức object thread **nằm ngoài** guard và vẫn có thể bị người khác join/detach, hoặc guard có thể sống lâu hơn thread nó tham chiếu. Sách chính là nêu điểm này ở tr. 28 để dẫn tới `scoped_thread` (Cụm 3, Listing 2.6) — bản **sở hữu hẳn** thread thay vì tham chiếu.

### 1.8 `detach()` — thả thread chạy nền (tr. 22–24)

Gọi `detach()` để thread chạy nền, **không còn cách nào liên lạc trực tiếp** (tr. 22–23):

- **Không thể chờ nó xong nữa**; một khi đã detach thì *"it isn't possible to obtain a `std::thread` object that references it, so it can no longer be joined."*
- **Quyền sở hữu và điều khiển chuyển sang C++ Runtime Library**, thư viện này bảo đảm tài nguyên của thread được thu hồi đúng khi thread kết thúc.

```cpp
std::thread t(do_background_work);
t.detach();
assert(!t.joinable());       // sau detach, object không còn gắn với thread nào (tr. 23)
```

**Điều kiện gọi được `detach()`** giống hệt `join()`: phải **có** một thread để detach, tức chỉ gọi `t.detach()` khi `t.joinable()` trả `true` (tr. 23).

**Detached thread = daemon thread (tr. 23).** Sách gọi tên theo khái niệm **daemon process** của UNIX — process chạy nền không có giao diện. Đặc điểm: thường **sống rất lâu**, gần trọn vòng đời ứng dụng, làm các việc nền như **theo dõi filesystem, dọn cache cũ, tối ưu cấu trúc dữ liệu**. Ở thái cực ngược lại, detach cũng hợp lý cho **task fire-and-forget**, hoặc khi đã có cơ chế khác để biết thread xong.

**Listing 2.4 (tr. 23–24) — ví dụ thực tế: trình soạn thảo văn bản mở nhiều tài liệu.** Bối cảnh: mỗi cửa sổ tài liệu chạy trên một thread riêng, cùng code nhưng khác dữ liệu; thread xử lý yêu cầu **không quan tâm** thread kia bao giờ xong vì đang làm tài liệu chẳng liên quan → *"a prime candidate for running a detached thread"* (tr. 23).

```cpp
void edit_document(std::string const& filename) {
    open_document_and_display_gui(filename);

    while (!done_editing()) {
        user_command cmd = get_user_input();

        if (cmd.type == open_new_document) {
            std::string const new_name = get_filename_from_user();
            std::thread t(edit_document, new_name);  // (1) truyền THAM SỐ cho thread
            t.detach();                              // (2) không chờ: tài liệu độc lập
        } else {
            process_user_input(cmd);
        }
    }
}
```

Listing này còn phục vụ mục đích thứ hai (tr. 24): cho thấy **truyền tham số vào thread function** — thay vì chỉ đưa tên hàm cho constructor, ta đưa thêm `new_name`. Đó chính là chủ đề Cụm 2.

### Insight đáng nhớ (Cụm 1)

- **Một `std::thread` chỉ có hai kết cục hợp lệ: `join()` hoặc `detach()`.** Bỏ quên = `std::terminate()`. Vì thế trong code sản xuất, **đừng bao giờ để `std::thread` trần trong scope có thể ném exception** — bọc RAII (hoặc dùng `std::jthread` của C++20, xem Cụm 3).
- **Copy callable vào thread không cứu được lifetime của dữ liệu mà callable trỏ tới.** Reference/pointer/lambda capture-by-ref đều xuyên qua ranh giới thread. Quy tắc thực dụng: *thread nào detach thì dữ liệu nó dùng phải sống ít nhất bằng nó* — an toàn nhất là **copy toàn bộ dữ liệu vào thread**.
- **`join()` là công cụ thô** (chờ hoặc không). Muốn "chờ có timeout", "hỏi xem xong chưa", "lấy kết quả trả về" → phải sang `future`/condition variable ở ch. 4. Nhận ra giới hạn này sớm giúp không cố nhồi `join()` vào chỗ không hợp.

---

## Cụm 2 — Truyền tham số vào thread function (tr. 24–27)

Đây là mục ngắn nhưng **dày bẫy nhất** chương. Nguyên tắc gốc chỉ một câu, nhưng phải đọc từng chữ (tr. 24):

> *"by default, **the arguments are copied into internal storage**, where they can be accessed by the newly created thread of execution, and then **passed to the callable object or function as rvalues** as if they were temporaries. **This is done even if the corresponding parameter in the function is expecting a reference.**"*

Tách thành ba mệnh đề để nhớ:

| # | Quy tắc (tr. 24) | Hệ quả |
|---|---|---|
| (a) | Tham số được **copy vào storage nội bộ** của thread | Thread làm việc trên **bản sao**, không phải biến gốc |
| (b) | Rồi được truyền cho hàm **dưới dạng rvalue** (như temporary) | Hàm nhận `T&` (non-const ref) sẽ **không biên dịch được** |
| (c) | (a)+(b) xảy ra **kể cả khi** hàm khai báo tham số là reference | Ảo tưởng "tôi truyền reference" là sai mặc định |

### 2.1 Bẫy #1: **chuyển kiểu xảy ra MUỘN — trong thread mới** (tr. 24–25)

```cpp
void f(int i, std::string const& s);
std::thread t(f, 3, "hello");
```

Tạo thread chạy `f(3, "hello")` — nhưng chú ý (tr. 24): dù `f` nhận `std::string`, **string literal được copy nguyên dạng `char const*`**, và *"converted to a `std::string` **only in the context of the new thread**"*.

Vô hại với literal (sống suốt chương trình). **Chết người với biến automatic:**

```cpp
void f(int i, std::string const& s);

void oops(int some_param) {
    char buffer[1024];                       // ⚠️ mảng LOCAL
    sprintf(buffer, "%i", some_param);
    std::thread t(f, 3, buffer);             // ⚠️ chỉ POINTER tới buffer được copy
    t.detach();
}                                            // buffer bị hủy — trong khi việc chuyển
                                             // char* → std::string có thể CHƯA xảy ra
```

Sách mô tả chính xác cơ chế hỏng (tr. 25): *"there's a significant chance that the `oops` function will **exit before the buffer has been converted to a `std::string` on the new thread**, thus leading to undefined behavior."*

**Cách sửa — ép chuyển kiểu ở thread GỌI, trước khi vào constructor (tr. 25):**

```cpp
void not_oops(int some_param) {
    char buffer[1024];
    sprintf(buffer, "%i", some_param);
    std::thread t(f, 3, std::string(buffer));  // ✅ chuyển sang std::string NGAY tại đây
    t.detach();                                //    → thread nhận bản sao tự chứa
}
```

Sách chốt nguyên nhân gốc (tr. 25): bạn đã **trông cậy vào implicit conversion** từ `char*` sang `std::string`, nhưng *"this conversion happens too late because the `std::thread` constructor **copies the supplied values as is**, without converting to the expected argument type."*

> 🆕 Quy tắc rút gọn để nhớ: **`std::thread` copy đúng cái bạn viết, không copy cái hàm cần.** Mọi conversion mà bạn ngầm dựa vào — `char*`→`string`, `T*`→`shared_ptr`, iterator→container — đều bị đẩy sang thread mới. Cứ **tự tay tạo object đầy đủ trước khi truyền**.

### 2.2 Bẫy #2: muốn truyền reference thật → phải dùng `std::ref` (tr. 25–26)

Chiều ngược lại **không xảy ra âm thầm** — nó **không biên dịch được**, và đó là điều may:

```cpp
void update_data_for_widget(widget_id w, widget_data& data);   // nhận NON-CONST ref

void oops_again(widget_id w) {
    widget_data data;
    std::thread t(update_data_for_widget, w, data);  // ⚠️ KHÔNG BIÊN DỊCH ĐƯỢC
    display_status();
    t.join();
    process_widget_data(data);                       // (nếu compile được thì data vẫn rỗng)
}
```

Lý do đầy đủ (tr. 25): constructor `std::thread` *"is oblivious to the types of the arguments expected by the function and **blindly copies** the supplied values. But the internal code **passes copied arguments as rvalues** in order to work with move-only types, and will thus try to call `update_data_for_widget` with an rvalue. **This will fail to compile** because you can't pass an rvalue to a function that expects a non-const reference."*

Chú ý mệnh đề *"in order to work with move-only types"* — đây là **lý do thiết kế**: truyền dưới dạng rvalue là điều kiện để hỗ trợ `std::unique_ptr` (mục 2.4 dưới).

**Sửa bằng `std::ref` (tr. 25–26):**

```cpp
std::thread t(update_data_for_widget, w, std::ref(data));   // ✅ truyền reference thật
```

Khi đó `update_data_for_widget` nhận **reference tới `data`**, không phải bản sao tạm, và code biên dịch được.

Sách nêu luôn "họ hàng" của cơ chế này (tr. 26): *"both the operation of the `std::thread` constructor and the operation of `std::bind` are defined in terms of **the same mechanism**"* — ai quen `std::bind` thì semantics này quen thuộc ngay.

> ⚠️ **`std::ref` không phải phép màu — nó chỉ tắt cơ chế an toàn.** Dùng `std::ref` nghĩa là bạn **tự cam kết** rằng object đó **sống lâu hơn thread**, và mọi truy cập đồng thời vào nó **đã được đồng bộ** (ch. 3). Trong ví dụ trên điều đó đúng vì có `t.join()` trước `process_widget_data`. Nếu đổi thành `t.detach()` thì `std::ref(data)` biến ngay thành Listing 2.1.

### 2.3 Truyền member function pointer (tr. 26)

Vì dùng chung cơ chế với `std::bind`, ta truyền được **con trỏ hàm thành viên**, với điều kiện đưa **object pointer làm đối số đầu tiên**:

```cpp
class X {
public:
    void do_lengthy_work();
};

X my_x;
std::thread t(&X::do_lengthy_work, &my_x);   // gọi my_x.do_lengthy_work() trên thread mới
```

Và có thể truyền tiếp đối số: *"the third argument to the `std::thread` constructor will be the first argument to the member function, and so forth"* (tr. 26):

```cpp
class X { public: void do_work(int level, std::string tag); };
X my_x;
std::thread t(&X::do_work, &my_x, 3, std::string("io"));   // → my_x.do_work(3, "io")
```

> ⚠️ Truyền `&my_x` là **truyền pointer trần** — lại đúng vấn đề lifetime: `my_x` phải sống lâu hơn thread. Với object trên heap dùng chung, cân nhắc capture `std::shared_ptr` bằng lambda thay vì pointer trần.

### 2.4 Truyền object **move-only** (`std::unique_ptr`) (tr. 26–27)

Tình huống thú vị còn lại: đối số **không copy được, chỉ move được** — dữ liệu bên trong một object *chuyển* sang object khác, để lại object gốc rỗng (tr. 26).

Sách nhắc lại `std::unique_ptr` cho trọn ý (tr. 26): quản lý bộ nhớ tự động cho object cấp phát động; **chỉ một instance trỏ tới một object tại một thời điểm**; khi instance đó bị hủy thì object được delete. Move constructor / move assignment cho phép **chuyển quyền sở hữu** giữa các `unique_ptr`, để lại nguồn là **NULL pointer**. Quy tắc: nguồn là **temporary** thì move **tự động**; nguồn là **named value** thì phải **gọi `std::move()` tường minh**.

```cpp
void process_big_object(std::unique_ptr<big_object>);

std::unique_ptr<big_object> p(new big_object);
p->prepare_data(42);
std::thread t(process_big_object, std::move(p));   // ✅ chuyển QUYỀN SỞ HỮU vào thread
                                                   //    (sau dòng này p == nullptr)
```

Sách mô tả đúng hai chặng của quyền sở hữu (tr. 26): *"the ownership of `big_object` is transferred **first into internal storage for the newly created thread and then into `process_big_object`**."*

> 🆕 **Đây là kỹ thuật đáng nhớ nhất của Cụm 2 cho code thật:** `std::move` một `unique_ptr` vào thread là cách **sạch nhất** để trao dữ liệu lớn cho thread mà **không copy và không chia sẻ**. Không copy → không tốn; không chia sẻ → **không cần mutex nào cả**. Ngược hẳn với `std::ref` (chia sẻ, phải đồng bộ, phải lo lifetime). Khi thiết kế, ưu tiên **chuyển sở hữu** hơn **chia sẻ**.

### Insight đáng nhớ (Cụm 2)

- **Mặc định của `std::thread` là "copy rồi truyền như rvalue".** Ba trường hợp còn lại đều phải **nói tường minh**: muốn reference → `std::ref`; muốn chuyển sở hữu → `std::move`; muốn kiểu đã chuyển đổi → **tự convert trước khi truyền**.
- Ba lỗi trong mục này nằm trên **một trục duy nhất: dữ liệu chết trước thread**. `char buffer[]`, `&my_x`, `std::ref(data)` đều là cùng một bug mặc ba bộ quần áo.
- 🆕 **Checklist 10 giây trước mỗi `std::thread(...)`:** với từng đối số, hỏi *"cái này sống tới khi thread xong chứ?"* Nếu không chắc → **copy nó, hoặc move nó vào thread**, đừng tham chiếu ra ngoài.

---

## Cụm 3 — Chuyển quyền sở hữu thread (tr. 27–31)

### 3.1 `std::thread` là **movable nhưng không copyable** (tr. 27)

Cụm 2 kết bằng một quan sát bắc cầu (tr. 27): nhiều lớp trong Standard Library có **cùng ownership semantics với `std::unique_ptr`**, và `std::thread` là một trong số đó. Nó không sở hữu object động, **nhưng sở hữu một tài nguyên**:

> *"each instance is responsible for managing a thread of execution. This ownership can be transferred between instances, because instances of `std::thread` are **movable, even though they aren't copyable**. This ensures that **only one object is associated with a particular thread of execution at any one time** while allowing programmers the option of transferring that ownership between objects."* (tr. 27)

**Vì sao cần chuyển sở hữu (tr. 27):** viết một hàm **tạo thread chạy nền rồi trả quyền sở hữu về cho caller** (thay vì tự chờ); hoặc chiều ngược lại — tạo thread rồi **giao vào một hàm khác** để hàm đó chờ.

### 3.2 Ví dụ 6 dòng, dòng cuối **giết chương trình** (tr. 27–28)

Ví dụ này gói toàn bộ quy tắc move của `std::thread`, sách dùng 3 object `t1`, `t2`, `t3` cho 2 thread:

```cpp
void some_function();
void some_other_function();

std::thread t1(some_function);            // (1) thread A khởi chạy, thuộc t1
std::thread t2 = std::move(t1);           // (2) A chuyển sang t2 — t1 rỗng
t1 = std::thread(some_other_function);    // (3) thread B khởi chạy, gán vào t1
                                          //     KHÔNG cần std::move: nguồn là temporary
std::thread t3;                           // (4) default-construct: KHÔNG gắn thread nào
t3 = std::move(t2);                       // (5) A chuyển từ t2 sang t3 — t2 rỗng
t1 = std::move(t3);                       // (6) ⚠️ std::terminate() — CHƯƠNG TRÌNH CHẾT
```

Diễn giải từng bước của sách (tr. 27–28), rất đáng bám sát:

```
        t1                    t2                    t3
(1)   [thread A]              —                     —
(2)      —                 [thread A]               —          ← move tường minh
(3)   [thread B]           [thread A]               —          ← move từ temporary (ngầm)
(4)   [thread B]           [thread A]           (rỗng)
(5)   [thread B]              —                [thread A]      ← move tường minh
(6)   ✗ t1 ĐANG giữ thread B mà bị gán đè  ⇒  std::terminate()
```

**Vì sao (6) chết:** *"in this case `t1` already had an associated thread (which was running `some_other_function`), so `std::terminate()` is called to terminate the program"* — và lý do thiết kế là **nhất quán với destructor** (tr. 28):

> *"This is done for consistency with the `std::thread` destructor... you must explicitly wait for a thread to complete or detach it before destruction, and **the same applies to assignment: you can't just drop a thread by assigning a new value** to the `std::thread` object that manages it."*

Hai chi tiết cú pháp phụ, cũng hay bị hỏi (tr. 27):

- **Bước (3) không cần `std::move`** vì nguồn là **temporary** — *"moving from temporaries is automatic and implicit"*.
- **Bước (2), (5), (6) cần `std::move`** vì nguồn là **named object**.

> 🆕 Nếu thật sự muốn thay thread trong `t1`, phải **giải quyết thread cũ trước**:
> ```cpp
> if (t1.joinable()) t1.join();   // hoặc t1.detach() — tùy ý định
> t1 = std::move(t3);             // giờ mới an toàn
> ```

### 3.3 Chuyển thread **ra khỏi** và **vào trong** hàm (tr. 28)

**Listing 2.5 (tr. 28) — trả `std::thread` từ hàm** (chuyển sở hữu ra ngoài):

```cpp
std::thread f() {
    void some_function();
    return std::thread(some_function);   // trả temporary → move ngầm
}

std::thread g() {
    void some_other_function(int);
    std::thread t(some_other_function, 42);
    return t;                            // trả named object → vẫn ổn (move khi return)
}
```

**Chuyển sở hữu VÀO hàm** — nhận `std::thread` **theo giá trị** (tr. 28):

```cpp
void f(std::thread t);                   // nhận BY VALUE = nhận quyền sở hữu

void g() {
    void some_function();
    f(std::thread(some_function));       // (a) temporary → move ngầm
    std::thread t(some_function);
    f(std::move(t));                     // (b) named → phải std::move tường minh
}
```

### 3.4 `scoped_thread` — RAII **sở hữu hẳn** thread (Listing 2.6, tr. 28–29)

Lợi ích trực tiếp của move support: nâng cấp `thread_guard` (Listing 2.3) thành lớp **nhận quyền sở hữu** thay vì giữ reference. Sách nêu đúng hai điều nó khắc phục (tr. 28):

1. *"avoids any unpleasant consequences should the `thread_guard` object **outlive the thread it was referencing**"*;
2. *"**no one else can join or detach the thread** once ownership has been transferred into the object."*

```cpp
class scoped_thread {
    std::thread t;                                    // SỞ HỮU (không phải reference)
public:
    explicit scoped_thread(std::thread t_)
        : t(std::move(t_))                            // nhận by value rồi move vào
    {
        if (!t.joinable())
            throw std::logic_error("No thread");      // (1) kiểm tra NGAY trong ctor
    }

    ~scoped_thread() {
        t.join();                                     // (2) không cần if — ctor đã bảo đảm
    }

    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

struct func;                                          // xem Listing 2.1

void f() {
    int some_local_state;
    scoped_thread t{std::thread(func(some_local_state))};   // truyền thread THẲNG vào
    do_something_in_current_thread();
}                                                     // ~scoped_thread → join
```

**So sánh mà sách nhấn (tr. 29):** với `thread_guard` (Listing 2.3), destructor **phải** kiểm tra `joinable()`; với `scoped_thread`, ta **dời việc kiểm tra lên constructor và ném exception nếu sai** — destructor nhờ đó sạch sẽ. Đây là một pattern thiết kế đáng học: **đẩy kiểm tra bất biến (invariant) về constructor để destructor không phải phòng thủ**.

> ⚠️ **Lỗi nhỏ trong Listing 2.6 (tr. 29):** sách viết `int some_local_state;` — **không khởi tạo**, trong khi Listing 2.3 viết `int some_local_state = 0;`. Vì `func::operator()` gọi `do_something(i)` trên chính biến này, đọc giá trị chưa khởi tạo là **UB**. Khi gõ lại code, viết `int some_local_state = 0;`.

### 3.5 `joining_thread` — và **⚠️ cập nhật: nay đã là `std::jthread` (C++20)** (Listing 2.7, tr. 29–31)

Bối cảnh sách viết (tr. 29): *"One of the proposals for C++17 was for a `joining_thread` class... This didn't get consensus in the committee, so it wasn't accepted into the standard (though **it's still on track for C++20 as `std::jthread`**), but it's relatively easy to write."*

**Listing 2.7 (tr. 29–30) — bản tự viết đầy đủ:**

```cpp
class joining_thread {
    std::thread t;
public:
    joining_thread() noexcept = default;

    // (1) forward MỌI đối số sang constructor của std::thread — dùng y như std::thread
    template<typename Callable, typename... Args>
    explicit joining_thread(Callable&& func, Args&&... args)
        : t(std::forward<Callable>(func), std::forward<Args>(args)...) {}

    explicit joining_thread(std::thread t_) noexcept : t(std::move(t_)) {}

    joining_thread(joining_thread&& other) noexcept : t(std::move(other.t)) {}

    // (2) move-assign: JOIN thread cũ trước rồi mới nhận thread mới
    //     → KHÔNG terminate như std::thread (xem mục 3.2)
    joining_thread& operator=(joining_thread&& other) noexcept {
        if (joinable()) join();
        t = std::move(other.t);
        return *this;
    }
    joining_thread& operator=(std::thread other) noexcept {
        if (joinable()) join();
        t = std::move(other);
        return *this;
    }

    ~joining_thread() noexcept {
        if (joinable()) join();          // (3) điểm mấu chốt: tự join khi hủy
    }

    void swap(joining_thread& other) noexcept { t.swap(other.t); }

    std::thread::id get_id() const noexcept { return t.get_id(); }
    bool joinable() const noexcept        { return t.joinable(); }
    void join()                           { t.join(); }
    void detach()                         { t.detach(); }

    // (4) "cửa thoát hiểm": lấy std::thread bên trong khi cần API gốc
    std::thread&       as_thread() noexcept       { return t; }
    const std::thread& as_thread() const noexcept { return t; }
};
```

Hai điểm thiết kế đáng học từ listing này:

- **(2) move-assignment join thread cũ** thay vì `terminate` — chính là chỗ nó "hiền" hơn `std::thread`, sửa đúng cái bẫy ở bước (6) mục 3.2.
- **(4) `as_thread()`** — wrapper cung cấp lối truy cập object gốc, để không **mất tính năng** khi bọc. Pattern hữu ích cho mọi lớp wrapper.

> ⚠️ **Cập nhật so với sách (viết 2019):** `std::jthread` **đã chính thức vào C++20** — không còn "on track" nữa. Dùng bản chuẩn thay vì tự viết:
> ```cpp
> #include <thread>
> void f() {
>     std::jthread t(work, 42);     // dùng y hệt std::thread...
> }                                 // ...nhưng destructor TỰ request_stop() rồi join()
> ```
> `std::jthread` **hơn** `joining_thread` của sách ở một điểm lớn: nó tích hợp **cooperative cancellation** qua `std::stop_token`/`std::stop_source`. Callable có thể nhận `std::stop_token` làm tham số đầu tiên và tự kiểm tra:
> ```cpp
> std::jthread t([](std::stop_token st) {
>     while (!st.stop_requested()) {      // hợp tác dừng, không cần cờ atomic tự chế
>         do_one_chunk_of_work();
>     }
> });
> // t.request_stop();  ← hoặc để destructor tự gọi
> ```
> Đây chính là bài toán sách phải tự giải thủ công ở **ch. 9 (interrupting threads)** — 🆕 khi đọc tới đó, nhớ rằng C++20 đã chuẩn hóa cơ chế này.

### 3.6 Chứa thread trong container — nhóm thread (Listing 2.8, tr. 30–31)

Move support còn cho phép **đặt `std::thread` vào container move-aware** như `std::vector` (tr. 30):

```cpp
void do_work(unsigned id);

void f() {
    std::vector<std::thread> threads;

    for (unsigned i = 0; i < 20; ++i) {
        threads.emplace_back(do_work, i);   // (1) construct THẲNG trong vector
    }

    for (auto& entry : threads)             // (2) join lần lượt từng thread
        entry.join();
}
```

Sách bình luận hai điều (tr. 31):

- Đây thường là thứ ta cần khi **chia nhỏ công việc của một thuật toán**: trước khi return cho caller, **mọi thread phải xong**.
- Cấu trúc đơn giản này ngụ ý công việc mỗi thread là **tự chứa**, và kết quả chỉ là **side effect lên dữ liệu chung**. Nếu `f()` phải trả về giá trị phụ thuộc kết quả các thread thì phải **xem dữ liệu chung sau khi thread kết thúc** — cách khác để chuyển kết quả giữa thread là ở **ch. 4** (future).
- Ý nghĩa lớn hơn: bỏ thread vào `vector` là **bước đầu tự động hóa việc quản lý** — thay vì từng biến riêng lẻ, ta xử lý chúng **như một nhóm**. Bước tiếp theo là **số thread quyết định lúc chạy** (Cụm 4).

> ⚠️ **Bản thân Listing 2.8 chưa exception-safe:** nếu một `entry.join()` ném exception giữa chừng, các thread còn lại trong vector sẽ bị hủy khi chưa join → `std::terminate`. Sách để dành exception safety cho **ch. 8**; trong code thật, dùng `std::vector<std::jthread>` (C++20) là xong.

### Insight đáng nhớ (Cụm 3)

- **`std::thread` cư xử y hệt `std::unique_ptr` vì cùng một lý do:** nó sở hữu tài nguyên **duy nhất**, nên copy là vô nghĩa (hai object cùng quản một thread?), còn move là hợp lý. Trả lời được câu "vì sao `std::thread` không copyable" bằng lập luận **unique ownership** là điểm cộng lớn.
- **Gán đè lên một `std::thread` đang giữ thread = `terminate`, y như quên join.** Cả hai là **cùng một luật** dưới hai hình dạng: *không được vứt bỏ một thread mà chưa quyết định số phận nó*.
- **`thread_guard` → `scoped_thread` → `joining_thread` → `std::jthread`** là một tiến trình cải tiến đẹp, mỗi bước sửa đúng một khiếm khuyết: giữ-reference → sở-hữu → có-đủ-API-và-move → **có cả cooperative cancellation, và đã chuẩn hóa**.

---

## Cụm 4 — Chọn số thread lúc chạy: `hardware_concurrency` & `parallel_accumulate` (tr. 31–33)

### 4.1 `std::thread::hardware_concurrency()` (tr. 31)

> *"This function returns **an indication of the number of threads that can truly run concurrently** for a given execution of a program. On a multicore system it might be the number of CPU cores, for example. **This is only a hint, and the function might return 0** if this information isn't available, but it can be a useful guide for splitting a task among threads."* (tr. 31)

Ba điều phải nhớ, cả ba đều là bẫy phỏng vấn:

1. Nó là **gợi ý (hint)**, không phải bảo đảm.
2. Nó **có thể trả về 0** → code **bắt buộc** phải có nhánh dự phòng.
3. Nó nói về **phần cứng**, không biết gì về **tải hiện tại của máy** hay **các thread khác trong chính process của bạn**.

### 4.2 Listing 2.9 — `parallel_accumulate` (tr. 31–33)

Sách nói trước về vị thế của ví dụ này (tr. 31): trong code thật nên dùng **`std::reduce` bản song song** ở ch. 10 thay vì tự viết, nhưng listing này minh họa **ý tưởng cơ bản**: chia việc cho các thread, **có số phần tử tối thiểu mỗi thread** để tránh overhead của quá nhiều thread. Và một cảnh báo thẳng: *"this implementation **assumes that none of the operations will throw an exception**, even though exceptions are possible; the `std::thread` constructor will throw if it can't start a new thread"* — xử lý exception để dành ch. 8.

```cpp
template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);   // kết quả GHI RA qua reference
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);

    if (!length)                                   // (1) dải rỗng → trả init luôn
        return init;

    unsigned long const min_per_thread = 25;       // (2) mỗi thread ít nhất 25 phần tử
    unsigned long const max_threads =
        (length + min_per_thread - 1) / min_per_thread;   // làm tròn LÊN

    unsigned long const hardware_threads =
        std::thread::hardware_concurrency();

    unsigned long const num_threads =              // (3) chốt số thread
        std::min(hardware_threads != 0 ? hardware_threads : 2,   // 0 → dùng 2
                 max_threads);

    unsigned long const block_size = length / num_threads;      // (4) kích thước khối

    std::vector<T>           results(num_threads);              // (5) chỗ chứa kết quả
    std::vector<std::thread> threads(num_threads - 1);          //     BỚT 1: đã có main

    Iterator block_start = first;
    for (unsigned long i = 0; i < (num_threads - 1); ++i) {     // (6) khởi chạy
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(                  // move-assign vào phần tử vector
            accumulate_block<Iterator, T>(),
            block_start, block_end,
            std::ref(results[i]));                 // ⚠️ std::ref: cần GHI ra ngoài
        block_start = block_end;
    }

    accumulate_block<Iterator, T>()(               // (7) thread HIỆN TẠI làm khối cuối
        block_start, last, results[num_threads - 1]);   //     → gánh phần dư

    for (auto& entry : threads)                    // (8) chờ tất cả
        entry.join();

    return std::accumulate(results.begin(), results.end(), init);  // (9) gộp kết quả
}
```

**Giải thích từng bước theo sách (tr. 32–33):**

- **(2) Vì sao cần `min_per_thread`:** *"This is to avoid creating 32 threads on a 32-core machine when you have only five values in the range."* Tức **giới hạn trên của số thread do KHỐI LƯỢNG VIỆC quyết định**, không phải do phần cứng.
- **(3) Vì sao lấy `min` với `hardware_threads`:** *"You don't want to run more threads than the hardware can support (which is called **oversubscription**), because the context switching will mean that more threads will decrease the performance."* Và nếu `hardware_concurrency()` trả 0 thì thay bằng **con số bạn tự chọn — sách chọn 2**. Sách cân đối hai phía: *"You don't want to run too many threads because that would slow things down on a single-core machine, but likewise you don't want to run too few because you'd be passing up the available concurrency."*
- **(4) Chia không hết thì sao:** *"If you're worrying about cases where the number doesn't divide evenly, don't—you'll handle that later."* — và "later" chính là bước (7).
- **(5) Vì sao `num_threads - 1` thread:** *"you need to launch one fewer thread than `num_threads`, because **you already have one**"* — thread gọi hàm cũng làm việc, không ngồi không.
- **(7) Khối cuối do thread hiện tại xử lý:** *"This is where you take account of any uneven division: you know the end of the final block **must be `last`**, and it doesn't matter how many elements are in that block."* Đây là cách xử lý phần dư **rất gọn**, đáng học.
- **(9)** Cuối cùng cộng dồn các kết quả từng khối bằng `std::accumulate` tuần tự.

**Ba cảnh báo sách nêu ở cuối (tr. 33)** — hay được hỏi vì chúng cho thấy *"song song hóa không phải là biến đổi trung tính"*:

1. **⚠️ Kết quả có thể KHÁC bản tuần tự** khi phép cộng của `T` **không kết hợp (not associative)** — ví dụ `float`, `double` — vì việc chia dải thành khối làm **đổi thứ tự gộp**, mà số dấu phẩy động thì `(a+b)+c ≠ a+(b+c)`.
2. **Yêu cầu iterator chặt hơn:** phải ít nhất là **forward iterator**, trong khi `std::accumulate` chạy được với **input iterator một-lượt**.
3. **`T` phải default-constructible** để tạo được `std::vector<T> results(num_threads)`.

Sách khái quát: *"These sorts of requirement changes are common with parallel algorithms; by their nature they're different in order to make them parallel, and this has consequences for the results and requirements."* (tr. 33)

Và ghi chú thiết kế quan trọng (tr. 33): *"because **you can't return a value directly from a thread**, you must pass in a reference to the relevant entry in the `results` vector"* — hạn chế này chính là động lực của **`std::future` ở ch. 4**.

> ⚠️ **Không khớp giữa lời văn và code trong sách (tr. 33):** đoạn giải thích viết *"you can wait for all the threads you spawned with `std::for_each`, as in listing 2.8"*, nhưng cả Listing 2.8 lẫn 2.9 (2nd ed) đều dùng **range-based for** `for(auto& entry: threads) entry.join();`. Đây là **tàn dư từ 1st edition** (bản cũ dùng `std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));`). Code mới đúng và dễ đọc hơn; đừng bối rối khi thấy chữ `for_each`.

> 🆕 **Vì sao `std::ref(results[i])` ở đây là AN TOÀN**, dù Cụm 2 vừa cảnh báo về nó? Ba điều kiện đều thỏa: (a) `results` sống trong `parallel_accumulate` và **có `join()` trước khi hàm return** → lifetime bảo đảm; (b) **mỗi thread ghi vào một phần tử riêng biệt**, không có hai thread đụng cùng ô → không data race; (c) `results` **không bị reallocate** (kích thước cố định từ đầu) → reference không bị vô hiệu. **Thiếu bất kỳ điều nào là bug.** Đặc biệt (c): nếu dùng `push_back` trong vòng lặp thì mọi reference lấy trước đó **dangling** ngay.

> ⚠️ 🆕 **Còn một cạm bẫy hiệu năng mà chương 2 chưa nói tới: false sharing.** Các phần tử `results[i]` nằm **sát nhau trong bộ nhớ**, nhiều phần tử có thể chung một **cache line 64 byte**. Mỗi lần một thread ghi kết quả, nó **làm mất hiệu lực cache line của các thread khác** → "cache ping-pong". Với listing này tác hại nhỏ (mỗi thread chỉ ghi **một lần cuối cùng**), nhưng nếu vòng lặp cập nhật `result` liên tục thì hiệu năng có thể **tệ hơn bản tuần tự**. Sách xử lý đầy đủ ở **ch. 8** (tr. 251–299).

### Insight đáng nhớ (Cụm 4)

- **Công thức chọn số thread của sách rất đáng thuộc lòng**, vì nó gói cả ba ràng buộc:
  `num_threads = min( hardware_concurrency() (hoặc fallback nếu 0), ceil(length / min_per_thread) )`
  → tôn trọng **phần cứng** (tránh oversubscription), tôn trọng **khối lượng việc** (tránh thread thừa cho việc bé), và **chịu được** khi hệ thống không cho biết số core.
- **Thread gọi hàm cũng phải làm việc.** Tạo `n` thread rồi ngồi chờ là lãng phí đúng một core. Mẫu `num_threads - 1` + "khối cuối do mình làm" vừa tiết kiệm vừa **xử lý phần dư miễn phí**.
- **Song song hóa làm đổi hợp đồng của hàm**, không chỉ đổi tốc độ: kết quả floating-point khác đi, yêu cầu iterator mạnh hơn, ràng buộc kiểu chặt hơn. Nêu được điều này khi phỏng vấn cho thấy tư duy vượt mức "chia mảng ra cho nhiều thread".

---

## Cụm 5 — Nhận diện thread: `std::thread::id` (tr. 34–35)

### 5.1 Hai cách lấy id (tr. 34)

```cpp
std::thread t(work);

std::thread::id id1 = t.get_id();                   // (1) từ object std::thread
std::thread::id id2 = std::this_thread::get_id();   // (2) id của THREAD HIỆN TẠI
                                                    //     (cũng khai trong <thread>)
```

**⚠️ Nếu object `std::thread` không gắn thread nào**, `get_id()` trả về một `std::thread::id` **default-constructed**, mang nghĩa **"not any thread"** (tr. 34).

### 5.2 Tính chất của `std::thread::id` (tr. 34)

- **Copy và so sánh thoải mái** — *"they wouldn't be of much use as identifiers otherwise"*.
- **Bằng nhau** ⇒ **cùng một thread**, *hoặc* cả hai đều mang giá trị "not any thread". **Khác nhau** ⇒ hai thread khác nhau, *hoặc* một cái là thread còn cái kia là "not any thread".
- Không chỉ có `==`/`!=`: `std::thread::id` cung cấp **đầy đủ bộ toán tử so sánh**, tạo **total ordering** trên mọi giá trị phân biệt. Nhờ vậy dùng được làm **key trong associative container**, hoặc **sắp xếp**. Thứ tự này cư xử trực giác: *"if `a<b` and `b<c`, then `a<c`"*.
- Standard Library cũng cung cấp **`std::hash<std::thread::id>`** → dùng được làm key trong **unordered container** (`std::unordered_map`).

> ⚠️ **Total ordering đó là tùy ý (arbitrary), không mang ý nghĩa gì.** `id_a < id_b` **không** nói thread nào tạo trước, chạy trên core nào, hay ưu tiên cao hơn. Nó tồn tại **chỉ để** `std::map` dùng được. Đừng suy diễn ngữ nghĩa từ thứ tự.

### 5.3 Dùng để làm gì (tr. 34–35)

**Cách dùng #1 — kiểm tra "tôi có phải master thread không" (tr. 34):** khi các thread chia việc như Listing 2.9, **thread khởi đầu** có thể cần làm thêm phần việc riêng ở giữa thuật toán:

```cpp
std::thread::id master_thread;                  // lưu TRƯỚC khi khởi chạy các thread khác

void some_core_part_of_algorithm() {
    if (std::this_thread::get_id() == master_thread) {
        do_master_thread_work();                // chỉ master làm phần này
    }
    do_common_work();                           // mọi thread đều làm phần này
}
```

Sách nêu rõ động lực dùng id thay vì truyền tham số (tr. 33): có thể truyền một số định danh (như `i` ở Listing 2.8), **nhưng** *"if the function that needs the identifier is **several levels deep in the call stack** and could be called from any thread, it's **inconvenient** to have to do it that way."* → Đây chính là lý do standard đưa id vào: *"When we were designing the C++ Standard Library we foresaw this need, so each thread has a unique identifier."*

**Cách dùng #2 — gắn id vào cấu trúc dữ liệu (tr. 34):** lưu `std::this_thread::get_id()` vào cấu trúc như một phần của thao tác; các thao tác sau **so id đã lưu với id của thread đang thao tác** để quyết định **cái gì được phép / bắt buộc**.

> 🆕 Đây chính là cách hiện thực **recursive mutex thủ công** hoặc kiểm tra *"chỉ thread đã tạo object mới được hủy nó"* — ví dụ luật "GUI object chỉ được đụng từ UI thread" trong Qt/GTK.

**Cách dùng #3 — làm key trong associative container (tr. 35):** khi cần gắn dữ liệu riêng cho từng thread và **thread-local storage không phù hợp**. Ví dụ sách nêu: một **controlling thread** lưu thông tin về từng thread nó quản lý, hoặc dùng để **truyền thông tin giữa các thread**.

**Nguyên tắc chốt của sách (tr. 35):** *"`std::thread::id` will suffice as a generic identifier for a thread in most circumstances; **it's only if the identifier has semantic meaning associated with it** (such as being an index into an array) **that alternatives should be necessary**."* → Cần **chỉ số 0..n-1** để đánh index mảng thì vẫn phải tự truyền số; `thread::id` không cho bạn điều đó.

### 5.4 In ra id (tr. 35)

```cpp
std::cout << std::this_thread::get_id();       // in được, nhưng...
```

**⚠️ Output là implementation-dependent hoàn toàn** (tr. 35). Standard **chỉ bảo đảm**: id bằng nhau thì in ra giống nhau, id khác nhau thì in ra khác nhau. *"This is therefore primarily useful for **debugging and logging**, but the values have no semantic meaning."*

> 🆕 Đừng bao giờ **parse** chuỗi id, đừng ép sang số để làm index, và đừng giả định nó là **TID của OS** (`gettid()` trên Linux). Muốn lấy TID thật để đối chiếu với `top`/`ps`/`ftrace`, phải dùng `native_handle()` + API nền tảng (`pthread_self()` / `syscall(SYS_gettid)`) — cơ chế "cửa thoát hiểm" mà ch. 1 (tr. 13) đã nhắc.

### Insight đáng nhớ (Cụm 5)

- **`std::thread::id` giải một bài toán rất cụ thể:** nhận diện thread **ở sâu trong call stack** mà không phải luồn tham số qua từng tầng. Nếu bạn kiểm soát được chỗ gọi thì truyền index còn tốt hơn.
- **Nó là định danh mờ (opaque):** so sánh được, hash được, in được để debug — **hết**. Mọi ngữ nghĩa khác đều là bạn tự bịa.

---

## Tóm tắt chương (theo sách, tr. 35)

Sách tự tổng kết: chương này phủ **những điều cơ bản của quản lý thread** với Standard Library — **khởi chạy** thread, **chờ chúng xong**, và **không chờ** vì muốn chúng chạy nền; **truyền tham số** vào thread function; **chuyển trách nhiệm quản lý thread** từ chỗ này sang chỗ khác; dùng **nhóm thread để chia việc**; và **nhận diện thread** để gắn dữ liệu/hành vi cho thread cụ thể.

Câu chuyển tiếp sang ch. 3 (tr. 35): *"Although you can do quite a lot with purely independent threads that each operate on separate data, sometimes it's desirable to **share data among threads** while they're running."*

**🆕 Bảng chốt — 6 bẫy của chương 2 và cách chặn:**

| # | Bẫy | Triệu chứng | Cách chặn |
|---|-----|-------------|-----------|
| 1 | Quên join/detach | `std::terminate()`, chương trình chết đột ngột | RAII (`scoped_thread`) hoặc **`std::jthread`** (C++20) |
| 2 | Exception nhảy qua `join()` | Như trên, nhưng chỉ xảy ra trên đường lỗi hiếm | RAII — **đừng dùng try/catch thủ công** (tr. 21) |
| 3 | `detach()` + reference tới biến local | UB, crash ngẫu nhiên, khó tái hiện | **Copy/move dữ liệu vào thread**; tránh capture `[&]` khi detach |
| 4 | Truyền `char*`/pointer, dựa vào implicit conversion | UB do convert xảy ra **muộn**, trong thread mới | **Tự convert trước khi truyền**: `std::string(buffer)` |
| 5 | Tưởng đã truyền reference | Không compile (may) hoặc thread sửa bản sao (xui) | `std::ref(x)` — và tự cam kết lifetime + đồng bộ |
| 6 | Gán đè lên `std::thread` đang chạy | `std::terminate()` | `join()`/`detach()` thread cũ trước khi gán |

---

## Góc interview

**Câu 1 (🎯 kinh điển, gần như chắc chắn được hỏi):** `join()` và `detach()` khác nhau thế nào? Điều gì xảy ra nếu **không gọi cái nào**? Viết cách bảo đảm luôn đúng kể cả khi có exception.

<details><summary>Đáp án</summary>

**Khác biệt:**

| | `join()` | `detach()` |
|---|---|---|
| Ý nghĩa | Thread **gọi** dừng lại **chờ** thread kia xong | **Thả** thread chạy nền, cắt đứt liên hệ |
| Sau khi gọi | Object không còn `joinable()`; storage của thread được **dọn** (tr. 20) | Object không còn `joinable()`; quyền sở hữu chuyển cho **C++ Runtime Library** (tr. 22–23) |
| Lấy được kết quả? | Có (qua biến chung/`std::ref`, xem Listing 2.9) | **Không** — không còn cách liên lạc trực tiếp (tr. 22) |
| Gọi lại lần 2 | **Không được** — chỉ một lần cho mỗi thread (tr. 20) | Không được (đã hết `joinable()`) |
| Tên gọi khác | — | **daemon thread**, theo daemon process của UNIX (tr. 23) |

**Nếu không gọi cái nào:** khi object `std::thread` bị hủy, **destructor gọi `std::terminate()`** → **chương trình chết** (tr. 18). Không phải leak, không phải cảnh báo — **chết ngay**.

**Vì sao standard chọn vậy?** Vì cả hai mặc định ngầm đều nguy hiểm: tự join có thể treo chương trình vô hạn ở chỗ bất ngờ; tự detach thì âm thầm để lại dangling reference (Listing 2.1). Standard buộc lập trình viên **nói rõ ý định**. Cùng logic đó áp cho **move-assignment**: gán đè lên object đang giữ thread cũng `terminate` — *"for consistency with the `std::thread` destructor"* (tr. 28).

**⚠️ Chú ý:** chỉ cần quyết định **trước khi object bị hủy** — thread có thể đã xong từ lâu (tr. 18). Và điều kiện gọi được cả hai đều là `t.joinable() == true` (tr. 23).

**Bảo đảm đúng khi có exception — RAII.** Cách thủ công (Listing 2.2) là try/catch, nhưng sách chê thẳng: *"verbose, and it's easy to get the scope slightly wrong"* (tr. 21). Cách đúng là `scoped_thread` (Listing 2.6, tr. 28–29):

```cpp
class scoped_thread {
    std::thread t;                                  // SỞ HỮU thread
public:
    explicit scoped_thread(std::thread t_) : t(std::move(t_)) {
        if (!t.joinable())
            throw std::logic_error("No thread");    // kiểm tra invariant trong ctor
    }
    ~scoped_thread() { t.join(); }                  // destructor sạch, không cần if
    scoped_thread(scoped_thread const&) = delete;
    scoped_thread& operator=(scoped_thread const&) = delete;
};

void f() {
    int state = 0;
    scoped_thread t{std::thread(func(state))};
    do_something_in_current_thread();               // ném exception cũng an toàn
}                                                   // ~scoped_thread → join
```

Ba chi tiết nên nói thêm để ghi điểm:
1. **Thứ tự hủy ngược thứ tự construct** (tr. 22) — guard construct sau nên hủy trước, join xảy ra trước khi `std::thread` bị hủy.
2. **Cấm copy bằng `=delete`** vì bản sao *"might then outlive the scope of the thread it was joining"* (tr. 22).
3. So sánh với `thread_guard` (Listing 2.3): bản đó giữ `std::thread&`, phải kiểm `joinable()` trong destructor và có nguy cơ **sống lâu hơn thread nó tham chiếu**; `scoped_thread` **sở hữu hẳn** nên khắc phục cả hai, đồng thời *"no one else can join or detach the thread"* (tr. 28).

**Điểm cộng lớn — nêu cập nhật C++20:** sách (2019) tự viết `joining_thread` (Listing 2.7) và ghi *"still on track for C++20 as `std::jthread`"* (tr. 29). **Nay `std::jthread` đã chuẩn hóa** — destructor tự `request_stop()` rồi `join()`, và có sẵn **cooperative cancellation** qua `std::stop_token`:

```cpp
std::jthread t([](std::stop_token st) {
    while (!st.stop_requested()) do_chunk();
});   // destructor: request_stop() + join()
```

Kết luận thực chiến: **trong code mới, đừng dùng `std::thread` trần** — dùng `std::jthread`, hoặc RAII wrapper nếu buộc phải ở C++17.

</details>

**Câu 2 (🎯 rất hay hỏi, phân loại người đã đọc kỹ):** `std::thread t(f, 3, buffer);` với `char buffer[1024]` là biến local và `void f(int, std::string const&)` — có vấn đề gì? Nếu tôi muốn thread **sửa** một biến của tôi thì viết thế nào?

<details><summary>Đáp án</summary>

**Quy tắc gốc (tr. 24)** — phải thuộc: *"the arguments are **copied into internal storage**... and then passed to the callable object or function **as rvalues** as if they were temporaries. **This is done even if the corresponding parameter in the function is expecting a reference.**"*

**Vấn đề của đoạn code:** `f` nhận `std::string const&`, nhưng `std::thread` **copy nguyên `char const*`** (con trỏ tới `buffer`), **không** chuyển sang `std::string`. Việc chuyển đổi đó xảy ra **muộn — trong thread mới**. Nếu `oops()` thoát trước (nhất là khi có `detach()`), `buffer` đã bị hủy ⇒ **undefined behavior** (tr. 24–25).

**Sửa — convert ở thread gọi, trước khi vào constructor (tr. 25):**
```cpp
std::thread t(f, 3, std::string(buffer));   // ✅ thread nhận một std::string tự chứa
```
Nguyên nhân gốc, nói theo sách: *"the `std::thread` constructor **copies the supplied values as is**, without converting to the expected argument type"* (tr. 25). Khẩu quyết: **thread copy đúng cái bạn viết, không copy cái hàm cần.**

**Muốn thread sửa biến của mình → `std::ref` (tr. 25–26):**
```cpp
void update(widget_id w, widget_data& data);

widget_data data;
std::thread t(update, w, std::ref(data));   // ✅ truyền reference thật
t.join();
process(data);                              // giờ mới đọc — sau join
```
Nếu **không** có `std::ref`, code **không biên dịch được** — và đó là điều tốt. Lý do (tr. 25): thread copy giá trị rồi truyền **dưới dạng rvalue** (*"in order to work with move-only types"*), mà **rvalue không bind được vào non-const reference**.

**⚠️ Ba điều kiện bắt buộc khi dùng `std::ref`** (đây là chỗ ứng viên giỏi tách khỏi ứng viên khá):
1. **Lifetime**: object phải sống lâu hơn thread → hầu như luôn phải `join()` trong cùng scope. Ghép `std::ref` với `detach()` là tái tạo lại Listing 2.1.
2. **Đồng bộ**: nếu có thread khác cùng đụng object đó → **data race**, phải có mutex (ch. 3).
3. **Ổn định địa chỉ**: nếu ref trỏ vào phần tử của `vector` mà vector **reallocate** thì reference **dangling**.
   *(Listing 2.9 dùng `std::ref(results[i])` an toàn vì thỏa cả ba: có join, mỗi thread một ô riêng, vector kích thước cố định.)*

**Ba cách truyền tham số khác nên nêu để hoàn chỉnh:**
- **Move-only type — chuyển sở hữu (tr. 26):**
  ```cpp
  std::unique_ptr<big_object> p(new big_object);
  p->prepare_data(42);
  std::thread t(process_big_object, std::move(p));   // p == nullptr sau dòng này
  ```
  🆕 Đây là **cách sạch nhất**: không copy (rẻ) và không chia sẻ (**khỏi mutex**). Khi thiết kế, ưu tiên **chuyển sở hữu** hơn **chia sẻ**.
- **Member function (tr. 26):** `std::thread t(&X::do_lengthy_work, &my_x);` — đối số thứ 3 trở đi thành đối số 1 trở đi của hàm thành viên. (Vẫn phải lo lifetime của `my_x`.)
- **Lambda:** cách phổ biến nhất trong code hiện đại — nhưng **capture by value** (`[data]`) nếu thread có thể sống lâu hơn scope; `[&]` chỉ dùng khi chắc chắn `join()` trong cùng scope.

**Chốt bằng một câu:** mặc định của `std::thread` là *copy rồi truyền như rvalue*; muốn khác đi phải **nói tường minh** — `std::ref` để chia sẻ, `std::move` để chuyển sở hữu, tự convert để tránh chuyển đổi muộn.

</details>

**Câu 3 (🎯):** Vì sao `std::thread` **movable nhưng không copyable**? Đoạn code sau in ra gì / xảy ra chuyện gì?

```cpp
std::thread t1(some_function);
std::thread t2 = std::move(t1);
t1 = std::thread(some_other_function);
std::thread t3;
t3 = std::move(t2);
t1 = std::move(t3);
```

<details><summary>Đáp án</summary>

**Vì sao không copyable:** `std::thread` **sở hữu một tài nguyên duy nhất** — một thread of execution của OS. Sách nói rõ ở tr. 27: *"each instance is responsible for managing a thread of execution... **only one object is associated with a particular thread of execution at any one time**."* Nếu cho copy thì hai object cùng "quản" một thread → ai chịu trách nhiệm join? Join hai lần? Đây là **cùng một lý lẽ** khiến `std::unique_ptr` và `std::ifstream` không copyable. **Move** thì hợp lý: nó **chuyển** trách nhiệm, không nhân bản nó.

**Diễn biến đoạn code (tr. 27–28):**

```
        t1                 t2                 t3          Ghi chú
(1)  [thread A]            —                  —           A = some_function
(2)     —               [thread A]            —           move TƯỜNG MINH (named object)
(3)  [thread B]         [thread A]            —           B = some_other_function;
                                                          KHÔNG cần std::move: nguồn là TEMPORARY
(4)  [thread B]         [thread A]         (rỗng)         default-ctor: không gắn thread nào
(5)  [thread B]            —             [thread A]       move tường minh
(6)  ✗✗✗  std::terminate()  →  CHƯƠNG TRÌNH CHẾT
```

**Dòng (6) `t1 = std::move(t3);` giết chương trình** vì `t1` **đang giữ** thread B đang chạy. Sách: *"in this case `t1` already had an associated thread (which was running `some_other_function`), so `std::terminate()` is called"* (tr. 27).

**Lý do thiết kế** (đây là phần đắt của câu trả lời) — tr. 28: *"This is done **for consistency with the `std::thread` destructor**... you must explicitly wait for a thread to complete or detach it before destruction, and **the same applies to assignment: you can't just drop a thread by assigning a new value**."* Tức **quên join** và **gán đè** là **cùng một luật** dưới hai hình dạng: *không được vứt bỏ một thread mà chưa quyết định số phận nó.*

**Sửa:**
```cpp
if (t1.joinable()) t1.join();    // (hoặc detach() — tùy ý định)
t1 = std::move(t3);              // giờ mới an toàn
```

**Quy tắc `std::move` tường minh hay ngầm (tr. 27):** nguồn là **temporary** → move **tự động, ngầm** (dòng 3); nguồn là **named object** → **phải** `std::move` (dòng 2, 5, 6).

**Ứng dụng của move support — nêu để trọn ý:**
- **Trả thread ra khỏi hàm** (Listing 2.5, tr. 28): `std::thread f() { return std::thread(some_function); }`
- **Nhận thread vào hàm**: `void f(std::thread t);` — nhận **by value** nghĩa là **nhận quyền sở hữu**.
- **Xây RAII `scoped_thread`** sở hữu hẳn thread (Listing 2.6) — chính nhờ move mới làm được.
- **Chứa thread trong `std::vector`** (Listing 2.8, tr. 30–31): `threads.emplace_back(do_work, i);` rồi `for (auto& e : threads) e.join();`

**🆕 Bẫy phụ hay bị hỏi thêm:** sau `std::move(t1)`, `t1` ở trạng thái nào? → Trạng thái hợp lệ nhưng **không gắn thread nào**: `t1.joinable() == false`, `t1.get_id()` trả về id **default-constructed** mang nghĩa *"not any thread"* (tr. 34). Gọi `t1.join()` lúc này là lỗi.

</details>

**Câu 4 (🎯):** Cho một dải phần tử lớn, hãy viết `parallel_accumulate`. Bạn chọn số thread thế nào, và vì sao?

<details><summary>Đáp án</summary>

**Công thức chọn số thread (Listing 2.9, tr. 32) — nên nói ra thành lời trước khi viết code:**

```
num_threads = min( hardware_concurrency()  (hoặc fallback nếu trả 0),
                   ceil(length / min_per_thread) )
```

Hai ràng buộc, mỗi cái chặn một sai lầm ngược nhau:

1. **Trần theo phần cứng** — `std::thread::hardware_concurrency()`: *"You don't want to run more threads than the hardware can support (which is called **oversubscription**), because the **context switching** will mean that more threads will decrease the performance."* (tr. 32). ⚠️ Hàm này chỉ là **hint** và **có thể trả 0** → bắt buộc có fallback (sách chọn **2**, tr. 31–32).
2. **Trần theo khối lượng việc** — `min_per_thread` (sách dùng **25**): *"to avoid creating 32 threads on a 32-core machine when you have only five values in the range"* (tr. 32). Nếu không có ràng buộc này, chi phí tạo thread lấn át việc thật (ch. 1, tr. 9).

**Code (rút gọn từ Listing 2.9, tr. 31–33):**

```cpp
template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    if (!length) return init;                              // dải rỗng

    unsigned long const min_per_thread = 25;
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = std::thread::hardware_concurrency();
    unsigned long const num_threads =
        std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
    unsigned long const block_size = length / num_threads;

    std::vector<T>           results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);     // ← BỚT 1: main cũng làm việc

    Iterator block_start = first;
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(accumulate_block<Iterator, T>(),
                                 block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    accumulate_block<Iterator, T>()(block_start, last, results[num_threads - 1]);
                                                           // ← khối cuối: gánh phần dư

    for (auto& entry : threads) entry.join();
    return std::accumulate(results.begin(), results.end(), init);
}
```

**Hai thủ pháp thiết kế nên chỉ ra:**
- **`num_threads - 1` thread + thread hiện tại làm khối cuối** (tr. 32–33): vừa không lãng phí một core ngồi chờ, vừa **xử lý phần dư miễn phí** — *"you know the end of the final block must be `last`, and it doesn't matter how many elements are in that block."*
- **Kết quả trả về qua `std::ref(results[i])`** vì *"**you can't return a value directly from a thread**"* (tr. 33). An toàn ở đây nhờ ba điều: có `join()` trước khi return, **mỗi thread ghi một ô riêng**, và vector **không reallocate**. → Cách đẹp hơn là `std::future` (ch. 4).

**Ba giới hạn phải chủ động nêu (tr. 33) — đây là phần ăn điểm:**
1. **⚠️ Kết quả có thể khác bản tuần tự** khi phép cộng **không associative** (`float`/`double`), vì chia khối làm **đổi thứ tự gộp**.
2. **Yêu cầu iterator mạnh hơn**: cần ít nhất **forward iterator** (`std::accumulate` chỉ cần input iterator một-lượt); `T` phải **default-constructible**.
3. **Chưa exception-safe**: sách nói thẳng *"this implementation assumes that none of the operations will throw... the `std::thread` constructor will throw if it can't start a new thread"* (tr. 31). Nếu một thread đã chạy mà chỗ khác ném exception, các `std::thread` trong vector bị hủy khi chưa join ⇒ `std::terminate`. Xử lý đầy đủ ở **ch. 8**; cách nhanh là `std::vector<std::jthread>` (C++20).

**⚠️ 🆕 Cạm bẫy hiệu năng nên nhắc (ch. 8):** các `results[i]` nằm sát nhau nên dễ **chung một cache line 64B** → **false sharing / cache ping-pong**. Với listing này tác hại nhỏ (mỗi thread ghi một lần cuối), nhưng nếu cập nhật liên tục thì có thể **chậm hơn bản tuần tự**. Cách chặn: mỗi thread tích lũy vào **biến local trên stack** rồi ghi ra **một lần duy nhất**, hoặc **padding/align** phần tử theo cache line.

**Chốt (tr. 31):** trong code thật, dùng **`std::reduce` với execution policy** (C++17, ch. 10) thay vì tự viết — trừ khi cần kiểm soát chi tiết cách chia việc.

</details>

**Câu 5 (🟠):** `std::thread::id` dùng để làm gì? Có thể dùng nó làm **index vào mảng** không?

<details><summary>Đáp án</summary>

**Lấy ở đâu (tr. 34):** `t.get_id()` (từ object `std::thread`) hoặc `std::this_thread::get_id()` (id của thread hiện tại, cũng khai trong `<thread>`). Nếu object không gắn thread nào, `get_id()` trả `std::thread::id` **default-constructed** = **"not any thread"**.

**Tính chất (tr. 34):**
- Copy và so sánh thoải mái; **bằng nhau ⇒ cùng thread** (hoặc cả hai đều "not any thread").
- Có **đầy đủ bộ toán tử so sánh**, tạo **total ordering** → dùng làm key trong `std::map`, hoặc sắp xếp.
- Có **`std::hash<std::thread::id>`** → dùng làm key trong `std::unordered_map`.

**Ba cách dùng chính:**
1. **Phân biệt master thread khi ở sâu trong call stack (tr. 34):**
   ```cpp
   std::thread::id master_thread;                    // lưu trước khi spawn
   void core_part() {
       if (std::this_thread::get_id() == master_thread) do_master_work();
       do_common_work();
   }
   ```
   Động lực (tr. 33): có thể truyền số định danh, **nhưng** nếu hàm cần định danh nằm *"several levels deep in the call stack"* và có thể bị gọi từ bất kỳ thread nào thì truyền tay **rất bất tiện** — *"When we were designing the C++ Standard Library we foresaw this need."*
2. **Lưu id vào cấu trúc dữ liệu** rồi so với id của thread đang thao tác để quyết định **cái gì được phép** (tr. 34). 🆕 Chính là cách hiện thực "chỉ owner thread mới được đụng object này" (luật GUI thread của Qt/GTK) hay recursive-mutex thủ công.
3. **Làm key trong associative container** khi thread-local storage không phù hợp — ví dụ controlling thread lưu thông tin về từng thread nó quản (tr. 35).

**Câu hỏi chính — dùng làm index mảng được không? → KHÔNG.** Sách trả lời trực tiếp (tr. 35): *"`std::thread::id` will suffice as a generic identifier... **it's only if the identifier has semantic meaning associated with it (such as being an index into an array) that alternatives should be necessary**."* Muốn index `0..n-1` thì **tự truyền số** vào thread (như `i` ở Listing 2.8).

**⚠️ Ba điều KHÔNG được làm với `thread::id`:**
1. **Đừng suy diễn từ thứ tự.** Total ordering tồn tại **chỉ để** `std::map` dùng được; `id_a < id_b` **không** nói thread nào tạo trước, chạy core nào, ưu tiên bao nhiêu.
2. **Đừng parse chuỗi in ra.** `std::cout << id` được, nhưng output **hoàn toàn implementation-dependent**; standard chỉ bảo đảm id bằng nhau in giống nhau, khác nhau in khác nhau — *"primarily useful for debugging and logging"* (tr. 35).
3. **Đừng coi nó là TID của OS.** Muốn số khớp với `top`/`ps`/`ftrace` để debug trên Linux, phải qua `native_handle()` + `pthread_self()`/`syscall(SYS_gettid)` — "cửa thoát hiểm" mà ch. 1 (tr. 13) đã nói.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [EMC++ cụm 7](../effective-modern-cpp.md) — Item 37 (*"làm `std::thread` unjoinable trên mọi đường thoát"*) là chính bài toán RAII của mục 2.1.3, nhìn từ góc Scott Meyers; Item 35 (`std::async` vs `std::thread`).
- [02-modern-cpp/concurrency.md](../../02-modern-cpp/concurrency.md) — bản cô đọng API concurrency của repo.
- [ostep/concurrency.md](../ostep/concurrency.md) — thread từ tầng OS: stack riêng, TCB, vì sao tạo thread tốn tài nguyên.
- [cpp-mindset/understanding-the-machine.md](../cpp-mindset/understanding-the-machine.md) — core vs hardware thread, cache line, nền của false sharing (ch. 8).
- [EMC++ cụm 5](../effective-modern-cpp.md) — move semantics & `std::move`/`std::forward`, nền lý thuyết cho Cụm 2–3 chương này.

**Chương tiếp theo:** [Ch. 3 — Sharing data between threads →](03-sharing-data.md) (race condition, mutex & `lock_guard`, **race vốn có trong interface**, deadlock + lock ordering, `std::lock`/`scoped_lock`, granularity, `once_flag`, `shared_mutex`).
