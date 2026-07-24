# Ch. 3 — Sharing data between threads (tr. 36–71) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 2 — Managing threads](02-managing-threads.md)** · **[Ch. 4 → Synchronizing concurrent operations](04-synchronization.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Đây là chương "đúng đắn" đầu tiên — và là chương dùng hằng ngày nhất.** Ch. 2 cho công cụ tạo thread; chương này trả lời câu hỏi cốt lõi: *khi nhiều thread cùng đụng một dữ liệu thì hỏng thế nào, và chặn ra sao.* Nội dung dày gấp đôi ch. 2, chứa **hai câu hỏi phỏng vấn kinh điển bậc nhất** (race trong interface `stack`, và deadlock + lock ordering) mà gần như buổi phỏng vấn concurrency nào cũng hỏi một trong hai.

**Sách nêu 3 mục tiêu chương (tr. 36):** các vấn đề khi chia sẻ dữ liệu · **bảo vệ dữ liệu bằng mutex** · các cơ chế bảo vệ thay thế.

**Ẩn dụ mở đầu (tr. 36):** ở chung căn hộ — một bếp, một phòng tắm. Không thể hai người cùng dùng phòng tắm; nướng xúc xích cùng lúc người kia nướng bánh trên cùng lò thì hỏng cả hai; và nỗi bực khi đang làm dở việc thì phát hiện ai đó đã mượn mất/đổi mất thứ mình cần. **Với thread cũng vậy:** phải có **luật** — thread nào được đụng dữ liệu nào, khi nào, và cập nhật được **truyền đạt** ra sao. Sách chốt lạnh lùng: hậu quả *"can be far worse than sausage-flavored cakes."*

---

## Cụm 1 — Vấn đề gốc: invariant vỡ, race condition, data race (tr. 37–40)

### 1.1 Chỉ có **ghi** mới gây rắc rối; công cụ tư duy là **invariant** (tr. 37)

Nguyên tắc gốc, đáng thuộc từng chữ (tr. 37):

> *"the problems with sharing data between threads are all due to the consequences of **modifying** data. If all shared data is **read-only**, there's no problem, because the data read by one thread is unaffected by whether or not another thread is reading the same data."*

→ 🆕 Đây là nền của rất nhiều thiết kế: **immutable data = tự do chia sẻ, khỏi khóa**. Mọi cơ chế trong chương chỉ cần thiết vì có **ghi**.

**Invariant** (bất biến) — *"statements that are always true about a particular data structure"* (tr. 37), ví dụ *"biến này chứa số phần tử của list"*. Vấn đề: invariant **thường bị phá tạm thời trong lúc cập nhật**, nhất là khi thao tác phải sửa **nhiều hơn một giá trị**.

**Ví dụ xương sống — xóa node khỏi doubly linked list (Figure 3.1, tr. 37–38).** Invariant: đi theo con trỏ `next` từ A sang B thì con trỏ `prev` của B phải trỏ về A. Bốn bước xóa node N:

```
Trạng thái đầu:     ... ⇄ [P] ⇄ [N] ⇄ [Q] ⇄ ...      (⇄ = cặp next/prev nhất quán)

(a) Xác định node cần xóa: N
(b) Sửa next của P  →  trỏ tới Q:
                    ... [P] ──next──→ [Q]        NHƯNG Q.prev vẫn trỏ về N
                        [P] ←─prev─── [N] ──→ [Q]
                    ⚠️ INVARIANT VỠ: đi xuôi P→Q, nhưng đi ngược Q→N→... không khớp
(c) Sửa prev của Q  →  trỏ về P:      ... [P] ⇄ [Q] ...   (invariant lại đúng)
(d) delete N
```

Câu chốt (tr. 38): *"between steps b and c, the links going in one direction are inconsistent with the links going in the opposite direction, and the invariant is broken."* Nếu một thread **đọc** list trong lúc thread khác đang ở giữa bước (b)-(c):
- Đọc xuôi (trái→phải) → **nhảy qua** node đang bị xóa.
- Nếu thread thứ hai lại đang xóa **node ngoài cùng bên phải** → có thể **hỏng vĩnh viễn** cấu trúc và cuối cùng crash.

Đây chính là **race condition** — nguyên nhân bug phổ biến nhất của code concurrent.

### 1.2 Race condition & data race — phân biệt cho chuẩn (tr. 38–39)

**Ẩn dụ (tr. 38–39):** mua vé xem phim ở rạp lớn, nhiều quầy bán song song. Ghế bạn được chọn **phụ thuộc người kia đặt trước hay bạn đặt trước** — nếu còn vài ghế cuối thì đúng nghĩa "chạy đua".

**Định nghĩa của sách (tr. 39):**

> *"a race condition is **anything where the outcome depends on the relative ordering of execution of operations on two or more threads**; the threads race to perform their respective operations."*

Điểm tinh tế — **không phải race nào cũng xấu (tr. 39):**
- **Benign race (lành tính):** mọi kết cục đều chấp nhận được. Ví dụ hai thread cùng thêm phần tử vào queue — thứ tự ai vào trước thường **không quan trọng**, miễn invariant được giữ.
- **Problematic race (có vấn đề):** race dẫn tới **invariant vỡ** (như xóa node dở dang). *"When talking about concurrency, the term race condition is usually used to mean a **problematic** race condition."*

**⚠️ `data race` là một khái niệm HẸP HƠN và nghiêm trọng hơn (tr. 39):** C++ Standard định nghĩa **data race** = *"the specific type of race condition that arises because of **concurrent modification to a single object**"* → **data race gây undefined behavior**. (Chi tiết ở §5.1.2, ch. 5.)

> 🆕 Ba tầng cần phân biệt rõ khi phỏng vấn:
> | Khái niệm | Là gì | Hậu quả |
> |---|---|---|
> | **Race condition** | Kết cục phụ thuộc thứ tự thread | Có thể lành tính hoặc có hại |
> | **Data race** (nghĩa hẹp C++) | ≥2 thread truy cập **cùng object**, ≥1 là **ghi**, không đồng bộ | **Undefined behavior** — luôn phải diệt |
> | **Race trong interface** (§3.2.3) | Từng thao tác an toàn nhưng **chuỗi thao tác** thì không | Logic sai dù có mutex |

**Vì sao race khó bắt (tr. 39):** cửa sổ lỗi rất **hẹp** — nếu hai lệnh sửa là hai lệnh CPU liên tiếp thì xác suất lỗi mỗi lần chạy rất nhỏ; nhưng **tải càng cao, số lần thao tác càng nhiều → xác suất trúng chuỗi lỗi càng tăng**. Và cay đắng nhất (tr. 39): race **thường biến mất dưới debugger** vì debugger làm đổi timing — *"they can often disappear entirely when the application is run under the debugger."*

### 1.3 Bốn cách xử lý problematic race (tr. 39–40)

Sách liệt kê, đặt nền cho cả cuốn:
1. **Bọc dữ liệu bằng cơ chế bảo vệ (mutex)** — thread khác chỉ thấy trạng thái *"chưa bắt đầu"* hoặc *"đã xong"*, không thấy trạng thái trung gian. ← **chương này.**
2. **Lock-free programming** — thiết kế lại cấu trúc sao cho mỗi thay đổi là **bất khả phân (indivisible)** và luôn giữ invariant. *"difficult to get right"* — dính sâu memory model (ch. 5) và lock-free (ch. 7).
3. **Software Transactional Memory (STM)** — xử lý cập nhật như **giao dịch database**: ghi log, commit một phát; nếu bị thread khác sửa thì restart. **C++ chưa hỗ trợ trực tiếp** (chỉ có TS), sách không phủ — nhưng ý tưởng *"làm riêng tư rồi commit một bước"* sẽ quay lại.

→ Cơ chế **cơ bản nhất** mà Standard cung cấp là **mutex**.

### Insight đáng nhớ (Cụm 1)
- **Read-only = miễn khóa.** Nếu tách được phần bất biến, phần đó chia sẻ tự do. Mọi chi phí đồng bộ sinh ra từ **ghi**.
- **Invariant là ngôn ngữ để suy luận về đúng đắn.** Bug concurrent = "một thread thấy invariant đang vỡ". Bảo vệ dữ liệu = "không ai thấy được trạng thái trung gian".
- **Đừng tin phép thử dưới debugger.** Race không tái hiện dưới gdb không có nghĩa là hết bug — nó nghĩa là bạn vừa đổi timing.

---

## Cụm 2 — Mutex cơ bản: `std::mutex`, `lock_guard`, và cạm bẫy rò rỉ pointer (tr. 40–43)

### 2.1 Mutex là gì, và `std::lock_guard` (RAII) (tr. 40–41)

**Ý tưởng (tr. 40):** đánh dấu mọi đoạn code truy cập cấu trúc dữ liệu là **mutually exclusive** (loại trừ lẫn nhau) — nếu một thread đang chạy một đoạn thì thread khác muốn đụng phải **chờ**. **mutex** (MUTual EXclusion): trước khi truy cập thì **lock**, xong thì **unlock**; Thread Library bảo đảm khi một thread đã lock thì mọi thread khác lock cùng mutex đó phải **chờ tới khi nó unlock**.

**⚠️ Đừng gọi `lock()`/`unlock()` trực tiếp (tr. 41):** vì phải **nhớ unlock trên MỌI đường thoát**, kể cả đường exception. Dùng **`std::lock_guard`** — RAII: lock khi construct, unlock khi destruct, bảo đảm *"a locked mutex is always correctly unlocked."* Cả hai khai trong `<mutex>`.

**Listing 3.1 (tr. 41) — bảo vệ một list:**

```cpp
#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;              // (1) dữ liệu chung
std::mutex     some_mutex;             // (2) mutex bảo vệ CHÍNH dữ liệu đó

void add_to_list(int new_value) {
    std::lock_guard<std::mutex> guard(some_mutex);   // (3) lock khi vào, unlock khi ra
    some_list.push_back(new_value);
}

bool list_contains(int value_to_find) {
    std::lock_guard<std::mutex> guard(some_mutex);   // (4) cùng mutex → loại trừ (3)
    return std::find(some_list.begin(), some_list.end(), value_to_find)
        != some_list.end();
}
```

Nhờ dùng **cùng một `some_mutex`**, `list_contains()` *"will never see the list partway through a modification by `add_to_list()`"* (tr. 41).

**🆕 Cập nhật cú pháp C++17 (sách nêu ở tr. 41–42):**
```cpp
std::lock_guard guard(some_mutex);      // class template argument deduction — bỏ <std::mutex>
std::scoped_lock guard(some_mutex);     // scoped_lock: bản C++17 "xịn hơn" của lock_guard (§3.2.4)
```
Sách nói sẽ vẫn viết `std::lock_guard<std::mutex>` đầy đủ cho rõ ràng và tương thích compiler cũ (tr. 42).

### 2.2 Đóng gói vào class — và cái bẫy "rò rỉ tham chiếu" (tr. 42–43)

Thay vì biến global, **gom mutex + dữ liệu vào một class** (tr. 42): đánh dấu rõ chúng liên quan, đóng gói, ép buộc bảo vệ. Data + mutex là **private member**, các hàm thao tác thành **member function** đều lock trước khi đụng data.

**⚠️ NHƯNG "khóa đủ mọi member function" vẫn CHƯA đủ (tr. 42):**

> *"if one of the member functions **returns a pointer or reference** to the protected data, then it doesn't matter that the member functions all lock the mutex... **you've blown a big hole in the protection**. Any code that has access to that pointer or reference can now access (and potentially modify) the protected data **without locking the mutex**."*

**Listing 3.2 (tr. 43) — vô tình đưa reference ra ngoài, nguy hiểm hơn vì qua hàm do người dùng cấp:**

```cpp
class some_data {
    int a;
    std::string b;
public:
    void do_something();
};

class data_wrapper {
private:
    some_data data;
    std::mutex m;
public:
    template<typename Function>
    void process_data(Function func) {
        std::lock_guard<std::mutex> l(m);
        func(data);                    // (1) trao dữ liệu "được bảo vệ" cho hàm NGOÀI
    }
};

some_data* unprotected;

void malicious_function(some_data& protected_data) {
    unprotected = &protected_data;     // (2) lén cất địa chỉ ra biến toàn cục
}

data_wrapper x;
void foo() {
    x.process_data(malicious_function);   // (3) truyền hàm ác vào → bypass bảo vệ
    unprotected->do_something();          // (4) ⚠️ truy cập KHÔNG khóa mutex
}
```

Vấn đề gốc (tr. 43): code **không làm được điều đặt ra** — đánh dấu *mọi* đoạn truy cập là mutually exclusive; nó bỏ sót đoạn `unprotected->do_something()` ở `foo()`. Thread Library **không cứu được** — trách nhiệm của lập trình viên.

**→ Guideline vàng của mục này (tr. 43), sách in nghiêng:**

> *"**Don't pass pointers and references to protected data outside the scope of the lock, whether by returning them from a function, storing them in externally visible memory, or passing them as arguments to user-supplied functions.**"*

🆕 Ba con đường rò rỉ cần soi: **(a) return value / out-parameter**, **(b) cất vào biến ngoài**, **(c) truyền vào callback/hàm do người dùng cấp**. (c) là xảo quyệt nhất vì trông vô hại.

### Insight đáng nhớ (Cụm 2)
- **Mutex bảo vệ *code path*, không bảo vệ *dữ liệu*.** Chỉ cần một đường (return, out-param, callback) chạm dữ liệu mà không qua lock là thủng. Bảo vệ bằng mutex = **thiết kế interface cẩn thận**, không phải rắc `lock_guard` khắp nơi.
- **Luôn đặt mutex ngay cạnh dữ liệu nó bảo vệ** (cùng class, private). Nhìn là biết "cái gì khóa cái gì". Mutex global rời rạc → mất dấu quan hệ, dễ khóa nhầm/thiếu.
- **`std::lock_guard` / `std::scoped_lock` là mặc định.** Đừng gọi `lock()`/`unlock()` tay — quên unlock trên đường exception là kinh điển.

---

## Cụm 3 — 🎯🎯 Race **vốn có trong interface**: bài toán `stack` kinh điển (tr. 44–50)

Đây là **mục hay được hỏi nhất chương**, và là chỗ tách người "biết dùng mutex" khỏi người "hiểu concurrency".

### 3.1 Vấn đề: từng thao tác an toàn ≠ chuỗi thao tác an toàn (tr. 44–46)

Sách bắt đầu bằng cảnh báo (tr. 44): *"Just because individual operations on the list are safe, you're not out of the woods yet."* Xét `std::stack` (Listing 3.3, tr. 44) — 5 thao tác: `push()`, `pop()`, `top()`, `empty()`, `size()`. Kể cả khi ta:
- đổi `top()` trả **copy** thay vì reference (theo guideline §3.2.2), và
- bọc toàn bộ nội bộ bằng mutex,

thì interface **vẫn dính race**. Và mấu chốt (tr. 44): *"This problem is not unique to a mutex-based implementation; it's an **interface problem**, so the race conditions would still occur with a lock-free implementation."*

**Race #1 — `empty()`/`size()` không đáng tin (tr. 44–45):** kết quả `empty()` hay `size()` có thể đúng **lúc gọi**, nhưng ngay khi return, thread khác đã kịp `push()`/`pop()` → thông tin lỗi thời.

```cpp
stack<int> s;
if (!s.empty()) {              // (1) kiểm tra không rỗng
    int const value = s.top(); // (2) ⚠️ giữa (1) và (2), thread khác có thể pop() sạch
    s.pop();                   // (3)
    do_something(value);
}
```

Trong code **đơn luồng** đoạn này không chỉ an toàn mà là **bắt buộc** (gọi `top()` trên stack rỗng là UB). Nhưng với **stack chia sẻ**, có thể có `pop()` từ thread khác **chen vào giữa `empty()` (1) và `top()` (2)** → classic race, và **mutex nội bộ KHÔNG chặn được** vì đây là hệ quả của **interface** (tr. 45).

**Race #2 — giữa `top()` và `pop()`, tinh vi hơn nhiều (tr. 45–46).** Hai thread cùng chạy đoạn trên trên cùng stack `s` có 2 phần tử. Table 3.1 (tr. 46) dựng lại:

```
      Thread A                        Thread B
      ──────────────────────          ──────────────────────
      if(!s.empty())
                                      if(!s.empty())
      int value = s.top();   ← đọc phần tử X
                                      int value = s.top();   ← CŨNG đọc X (chưa ai pop)
      s.pop();               ← xóa X
                                      s.pop();               ← xóa Y (phần tử dưới!)
      do_something(value=X)
                                      do_something(value=X)
```

Hậu quả (tr. 46): *"one of the two values on the stack is **discarded without ever having been read**, whereas the other is **processed twice**."* — **X bị xử lý hai lần, Y bị mất trắng, không đọc lần nào.** Sách gọi đây là race *"far more insidious"* vì **không có gì trông sai rõ ràng**, và hậu quả xảy ra xa nơi gây lỗi.

→ Lời giải: **gộp `top()` và `pop()` thành MỘT thao tác dưới cùng một lock.**

### 3.2 Vì sao `std::stack` lại tách `top()`/`pop()` — nghịch lý Cargill (tr. 46–47)

Câu hỏi tự nhiên: sao không làm `pop()` vừa xóa vừa trả giá trị? Đây là chỗ sách dạy một bài học exception-safety sâu (Tom Cargill 1994, Herb Sutter):

Xét `stack<vector<int>>`. `vector` cấp phát động → **copy vector phải xin thêm heap** → nếu hệ quá tải/thiếu tài nguyên, copy constructor có thể **ném `std::bad_alloc`**. Nếu `pop()` *"return giá trị vừa xóa"*: nó **xóa khỏi stack trước**, rồi **copy để trả về** — nếu copy ném exception thì (tr. 46–47): *"the data popped is **lost**; it has been removed from the stack, but the copy was unsuccessful!"*

→ Người thiết kế `std::stack` **cố tình tách đôi**: `top()` lấy (không đổi stack) rồi `pop()` xóa — nếu không copy an toàn được thì **dữ liệu vẫn còn trên stack**. **⚠️ Nhưng chính cái tách đôi đó lại là nguồn race #2!** Ta đang muốn *gộp* đúng cái mà exception-safety *tách*. Cần lời giải dung hòa.

### 3.3 Bốn phương án gộp `top()`+`pop()` an toàn (tr. 47–48)

| Option | Cách làm | Ưu | Nhược |
|---|---|---|---|
| **1. Truyền reference** `pop(T& result)` | Caller cấp sẵn chỗ chứa, `pop` gán vào | Đơn giản, tránh copy khi return | Caller phải **dựng sẵn instance** trước (tốn/không khả thi với vài kiểu); đòi kiểu **assignable** |
| **2. Yêu cầu copy/move ctor không ném** | Chỉ nhận kiểu mà return-by-value không thể throw | An toàn tuyệt đối | **Hạn chế:** nhiều kiểu có copy ctor ném và không có move ctor. Kiểm tại compile time bằng `std::is_nothrow_copy_constructible` / `std::is_nothrow_move_constructible` |
| **3. Trả `shared_ptr`** tới phần tử | `pop()` trả `std::shared_ptr<T>` | **pointer copy không bao giờ ném** → tránh Cargill; `shared_ptr` tự quản bộ nhớ, không leak; thư viện kiểm soát cấp phát | Với kiểu đơn giản (int), overhead quản lý bộ nhớ **vượt** chi phí copy trực tiếp |
| **4. Kết hợp 1 + (2 hoặc 3)** | Cho người dùng chọn | Linh hoạt, *"never rule out flexibility, especially in generic code"* | — |

### 3.4 `threadsafe_stack` hoàn chỉnh (Listing 3.4 + 3.5, tr. 48–50)

Sách cài **option 1 + option 3** (hai overload `pop()`), rút interface từ 5 xuống 3 thao tác:

```cpp
#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack : std::exception {
    const char* what() const throw();
};

template<typename T>
class threadsafe_stack {
private:
    std::stack<T>      data;
    mutable std::mutex m;              // ⚠️ 'mutable': để lock được trong hàm const (empty())
public:
    threadsafe_stack() {}

    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);   // (1) khóa NGUỒN khi copy
        data = other.data;                           //     copy trong THÂN ctor, không phải
    }                                                //     member-init list → giữ lock suốt copy

    threadsafe_stack& operator=(const threadsafe_stack&) = delete;   // (2) cấm gán

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }

    std::shared_ptr<T> pop() {                        // option 3
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();        // (3) kiểm rỗng TRONG lock → hết race #1
        std::shared_ptr<T> const res(               // (4) cấp phát kết quả TRƯỚC khi sửa stack
            std::make_shared<T>(data.top()));         //     → nếu make_shared ném, stack còn nguyên
        data.pop();
        return res;
    }

    void pop(T& value) {                              // option 1
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m);          // dùng được nhờ 'mutable'
        return data.empty();
    }
};
```

Các điểm thiết kế sách nhấn (tr. 48–50):
- **Cắt gọn interface = tăng an toàn tối đa (tr. 48–49):** bỏ `top()`, `size()`, `swap()`, cấm `operator=`. 5 thao tác → 3 (`push`, `pop`, `empty`), *"even `empty()` is superfluous."* Ít bề mặt → **dễ giữ mutex khóa suốt trọn một thao tác**.
- **`pop()` giải cả hai race:** kiểm `empty()` **bên trong lock** (diệt race #1), và **gộp `top`+`pop`** (diệt race #2). Ném `empty_stack` nếu rỗng → an toàn kể cả khi state đổi sau khi ai đó gọi `empty()`.
- **Thứ tự chống-Cargill (4):** `make_shared` (có thể ném) chạy **trước** `data.pop()` (sửa stack). Nếu ném → stack **chưa bị đụng**, dữ liệu không mất.
- **Copy constructor khóa nguồn, copy trong thân hàm (1)** *"in order to ensure that the mutex is held across the copy"* — không dùng member-init list vì cần lock trước rồi mới copy.
- **`mutable std::mutex`:** để `empty() const` vẫn `lock()` được (lock là *"thay đổi"* mutex nhưng không phải logical state của object).

### 3.5 Cái giá của gộp: granularity (tr. 50)

Sách dùng bài toán stack để mở ra khái niệm **lock granularity** (tr. 50):
- Race trong interface sinh ra vì **khóa quá NHỎ (too fine)** — lock không phủ trọn thao tác mong muốn.
- Nhưng **khóa quá LỚN (too coarse)** cũng hại — cực đoan là **một mutex global cho tất cả**. Ví dụ lịch sử đắt giá: *"The first versions of the Linux kernel that were designed to handle multi-processor systems used a **single global kernel lock**"* — kết quả máy 2 CPU chạy tệ hơn 2 máy 1 CPU riêng vì **tranh chấp (contention)** quá lớn. Kernel sau này chuyển sang **fine-grained locking**.

→ Câu chuyển sang deadlock (tr. 50): fine-grained locking dẫn tới đôi khi cần **khóa nhiều mutex một lúc** → nguy cơ **deadlock** — *"almost the opposite of a race condition: rather than two threads racing to be first, each one is waiting for the other."*

### Insight đáng nhớ (Cụm 3)
- **Race trong interface là bug thiết kế, không phải bug thiếu khóa.** Thêm mutex nội bộ **không** cứu được — kể cả bản lock-free cũng dính. Sửa = **đổi interface** (gộp thao tác về nguyên tử logic).
- **`top()`/`pop()` tách đôi trong `std::stack` là feature exception-safety, không phải thiếu sót.** Hiểu nghịch lý Cargill (gộp để hết race ⇄ tách để an toàn exception) rồi giải bằng `shared_ptr`/reference là câu trả lời "senior".
- **Granularity là dao hai lưỡi:** quá nhỏ → race/thiếu bảo vệ; quá lớn → contention giết hiệu năng (bài học global kernel lock). Đích: **một lock phủ đúng một thao tác logic, giữ ngắn nhất có thể.**

---

## Cụm 4 — 🎯 Deadlock: bản chất, `std::lock`/`scoped_lock`, và các guideline (tr. 50–59)

### 4.1 Deadlock là gì (tr. 51)

**Ẩn dụ (tr. 51):** hai đứa trẻ, một cái trống + một dùi trống, cần **cả hai** mới chơi được. Chúng lục thùng đồ chơi cùng lúc — một đứa vớ được trống, đứa kia vớ được dùi. Giờ **kẹt cứng**: mỗi đứa giữ chặt phần mình và đòi phần kia, không ai chơi được.

**Định nghĩa (tr. 51):** mỗi thread trong một cặp cần lock **cả hai** mutex; mỗi thread đang giữ một cái và **chờ** cái kia → *"Neither thread can proceed, because each is waiting for the other to release its mutex."* Đây là **vấn đề lớn nhất khi phải khóa từ hai mutex trở lên**.

### 4.2 Giải pháp 1: khóa **cùng thứ tự** — nhưng có bẫy (tr. 51)

Lời khuyên phổ biến: *"always lock the two mutexes in the same order"* — luôn lock A trước B thì không bao giờ deadlock. **⚠️ Nhưng bẫy khi hai mutex bảo vệ hai instance CÙNG một class** (ví dụ `swap(a, b)`): nếu quy ước "lock mutex của tham số 1 trước tham số 2", thì hai thread gọi `swap(x, y)` và `swap(y, x)` **đồng thời** → mỗi thread lock một cái, chờ cái kia → **deadlock!**

### 4.3 Giải pháp đúng: `std::lock` — khóa nhiều mutex **all-or-nothing** (Listing 3.6, tr. 51–52)

```cpp
class some_big_object;
void swap(some_big_object& lhs, some_big_object& rhs);

class X {
private:
    some_big_object some_detail;
    std::mutex m;
public:
    X(some_big_object const& sd) : some_detail(sd) {}

    friend void swap(X& lhs, X& rhs) {
        if (&lhs == &rhs) return;               // (1) BẮT BUỘC: cùng instance → khỏi khóa
                                                //     (lock mutex mình đã giữ = UB)
        std::lock(lhs.m, rhs.m);                // (2) khóa CẢ HAI, không lo thứ tự/deadlock
        std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);  // (3) "nhận" lock có sẵn
        std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);  // (4)
        swap(lhs.some_detail, rhs.some_detail);
    }
};
```

Giải thích (tr. 52):
- **(1)** Kiểm tra khác instance vì *"attempting to acquire a lock on `std::mutex` when you already hold it is undefined behavior"* (muốn khóa lại được thì dùng `std::recursive_mutex`, §3.3.3).
- **(2) `std::lock`** khóa cả hai mutex bằng **thuật toán tránh deadlock nội bộ**.
- **(3)(4) `std::adopt_lock`** báo cho `lock_guard` rằng mutex **đã được khóa rồi**, chỉ cần **nhận quyền sở hữu** để unlock đúng lúc destruct (thay vì khóa lại).
- **All-or-nothing (tr. 52):** nếu `std::lock` khóa được cái thứ nhất nhưng cái thứ hai ném exception, thì **cái thứ nhất tự động được nhả** — *"`std::lock` provides all-or-nothing semantics."*

**🆕 Cách C++17 gọn hơn — `std::scoped_lock` (tr. 52–53):**

```cpp
void swap(X& lhs, X& rhs) {
    if (&lhs == &rhs) return;
    std::scoped_lock guard(lhs.m, rhs.m);       // khóa cả hai bằng thuật toán như std::lock,
    swap(lhs.some_detail, rhs.some_detail);      // rồi nhả cả hai khi destruct — 1 dòng thay 3
}
```

`std::scoped_lock` là **variadic template**, nhận danh sách mutex; khóa bằng đúng thuật toán `std::lock`. Nhờ class template argument deduction (C++17), `std::scoped_lock guard(lhs.m, rhs.m)` = `std::scoped_lock<std::mutex, std::mutex> guard(...)`. Sách chốt (tr. 53): *"most of the cases where you would have used `std::lock` prior to C++17 can now be written using `std::scoped_lock`, with less potential for mistakes."*

**⚠️ Giới hạn (tr. 53):** `std::lock`/`scoped_lock` chỉ cứu khi bạn khóa các mutex **cùng một lúc**. Nếu phải khóa **rời rạc** (acquire riêng lẻ), chúng không giúp gì → cần các guideline dưới.

### 4.4 Các guideline tránh deadlock (tr. 53–55)

Nguyên tắc gốc, một câu (tr. 53): *"**don't wait for another thread if there's a chance it's waiting for you**."*

1. **AVOID NESTED LOCKS (đừng lồng lock) (tr. 53):** đừng acquire lock nếu đã giữ một lock. Mỗi thread chỉ giữ **một** lock tại một thời điểm → không thể deadlock vì lock. Nếu cần nhiều lock → dùng `std::lock` để lấy **cùng lúc**.
2. **AVOID CALLING USER-SUPPLIED CODE WHILE HOLDING A LOCK (tr. 53–54):** code người dùng có thể làm bất cứ gì, kể cả acquire lock → vi phạm quy tắc 1. (Với generic code như `threadsafe_stack`, mọi thao tác trên kiểu `T` đều là user code → cần quy tắc 3.)
3. **ACQUIRE LOCKS IN A FIXED ORDER (khóa theo thứ tự cố định) (tr. 54):** nếu buộc phải giữ ≥2 lock và không thể dùng `std::lock`, thì **luôn khóa theo cùng thứ tự ở mọi thread**. Ví dụ hand-over-hand locking trên linked list: để duyệt, giữ lock node hiện tại **rồi mới** lock node kế; nhưng nếu hai thread duyệt **ngược chiều** thì deadlock giữa list (Figure 3.2, tr. 55) → phải **quy định chiều duyệt** (luôn A trước B trước C).
4. **USE A LOCK HIERARCHY (phân tầng lock) (tr. 55–59):** trường hợp đặc biệt của "thứ tự cố định" nhưng **kiểm được lúc runtime**.

### 4.5 Hierarchical mutex — kiểm thứ tự lock tại runtime (Listing 3.7 + 3.8, tr. 55–59)

**Ý tưởng (tr. 55):** chia ứng dụng thành **tầng**; gán mỗi mutex một **số tầng**. Luật: *"When code tries to lock a mutex, it isn't permitted to lock that mutex if it already holds a lock from a lower layer."* → chỉ được khóa mutex có số tầng **thấp hơn** mutex đang giữ.

**Listing 3.7 (tr. 56) — cách dùng:**

```cpp
hierarchical_mutex high_level_mutex(10000);     // tầng cao
hierarchical_mutex low_level_mutex(5000);       // tầng thấp
hierarchical_mutex other_mutex(6000);           // tầng giữa

int  do_low_level_stuff();
int  low_level_func() {
    std::lock_guard<hierarchical_mutex> lk(low_level_mutex);
    return do_low_level_stuff();
}

void high_level_stuff(int some_param);
void high_level_func() {
    std::lock_guard<hierarchical_mutex> lk(high_level_mutex);   // giữ tầng 10000
    high_level_stuff(low_level_func());                         // rồi khóa 5000 < 10000 → OK
}

void thread_a() {           // ✅ tuân luật: 10000 → 5000 (giảm dần)
    high_level_func();
}

void do_other_stuff();
void other_stuff() {
    high_level_func();      // ⚠️ đang giữ 6000, high_level_func muốn khóa 10000 > 6000 → LỖI
    do_other_stuff();
}
void thread_b() {           // ✗ vi phạm luật → ném exception lúc runtime
    std::lock_guard<hierarchical_mutex> lk(other_mutex);   // giữ 6000
    other_stuff();
}
```

`thread_b` sai vì: giữ `other_mutex` (6000) rồi gọi `high_level_func()` đòi khóa `high_level_mutex` (10000 > 6000) — **leo ngược tầng** → `hierarchical_mutex` báo lỗi (ném exception / abort). Sách chốt (tr. 57): *"**Deadlocks between hierarchical mutexes are impossible, because the mutexes themselves enforce the lock ordering.**"* Hệ quả: **không thể giữ hai lock cùng tầng** (hand-over-hand đòi mỗi mắt xích một tầng thấp dần — đôi khi bất khả thi).

**Listing 3.8 (tr. 57–59) — cài đặt, điểm hay nằm ở `thread_local`:**

```cpp
class hierarchical_mutex {
    std::mutex internal_mutex;
    unsigned long const hierarchy_value;          // tầng của mutex này
    unsigned long previous_hierarchy_value;       // lưu tầng trước để khôi phục khi unlock
    static thread_local unsigned long this_thread_hierarchy_value;  // (1) TẦNG HIỆN TẠI của thread

    void check_for_hierarchy_violation() {
        if (this_thread_hierarchy_value <= hierarchy_value)   // (2) không được leo lên/ngang
            throw std::logic_error("mutex hierarchy violated");
    }
    void update_hierarchy_value() {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;        // hạ "trần" xuống tầng này
    }
public:
    explicit hierarchical_mutex(unsigned long value)
        : hierarchy_value(value), previous_hierarchy_value(0) {}

    void lock() {
        check_for_hierarchy_violation();          // (3) kiểm TRƯỚC
        internal_mutex.lock();
        update_hierarchy_value();                 // (4) khóa xong mới cập nhật tầng
    }
    void unlock() {
        if (this_thread_hierarchy_value != hierarchy_value)   // (5) chống unlock sai thứ tự
            throw std::logic_error("mutex hierarchy violated");
        this_thread_hierarchy_value = previous_hierarchy_value;  // khôi phục tầng cũ
        internal_mutex.unlock();
    }
    bool try_lock() {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock()) return false;   // (6) không khóa được → không đổi tầng
        update_hierarchy_value();
        return true;
    }
};

thread_local unsigned long
    hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);   // (7) khởi tạo = MAX
```

Điểm mấu chốt (tr. 58–59):
- **(1)(7) `thread_local`:** mỗi thread có bản `this_thread_hierarchy_value` **riêng**, khởi tạo `ULONG_MAX` (ban đầu khóa mutex nào cũng được). Trạng thái ở thread này **độc lập** thread khác → kiểm được hành vi từng thread riêng.
- **`try_lock()` (tr. 59):** giống `lock()` nhưng nếu mutex đang bị thread khác giữ thì **trả `false` ngay** thay vì chờ. Đây là **member function thứ ba** mà một kiểu phải có (`lock`/`unlock`/`try_lock`) để thỏa **mutex concept** → dùng được với `std::lock_guard` dù là kiểu tự định nghĩa.
- **Ưu điểm lớn (tr. 59):** detection là **runtime check nhưng KHÔNG phụ thuộc timing** — *"you don't have to wait around for the rare conditions that cause deadlock to show up."* Chỉ cần một lần chạy vi phạm là lộ, không như deadlock thật (hên xui timing). Ngoài ra **chính quá trình thiết kế phân tầng** đã loại nhiều nguy cơ deadlock trước khi viết code.

**Mở rộng ngoài lock (tr. 59):** deadlock xảy ra với **mọi construct đồng bộ tạo wait-cycle**, không riêng lock. Ví dụ hai thread cùng `join()` nhau → deadlock không cần lock nào. Guideline: đừng chờ thread khác khi đang giữ lock; nếu chờ thread thì cũng nên có **thread hierarchy** (chỉ chờ thread ở tầng thấp hơn); cách đơn giản là **join thread ở đúng hàm đã tạo nó**.

### Insight đáng nhớ (Cụm 4)
- **Deadlock = 4 điều kiện Coffman** (🆕 tên chuẩn sách không nêu nhưng hay được hỏi): mutual exclusion, hold-and-wait, no-preemption, **circular wait**. Mọi guideline của sách đều nhằm **phá circular wait** (thứ tự cố định / hierarchy) hoặc **phá hold-and-wait** (`std::lock` lấy tất cả một lần / tránh nested lock).
- **`std::scoped_lock` (C++17) là câu trả lời mặc định cho "khóa nhiều mutex".** Nêu được all-or-nothing semantics + vì sao thứ tự cố định thủ công dễ sai (bẫy `swap(x,y)`/`swap(y,x)`) là điểm cộng.
- **Hierarchical mutex biến deadrace-timing thành lỗi tất định.** Ý tưởng "kiểm invariant lúc runtime, không phụ thuộc may rủi" đáng mang sang mọi loại bug concurrency.

---

## Cụm 5 — `std::unique_lock`, chuyển quyền sở hữu lock, granularity (tr. 59–64)

### 5.1 `std::unique_lock` — linh hoạt hơn, trả giá nhẹ (tr. 59–61)

`std::unique_lock` nới lỏng invariant của `lock_guard`: **không nhất thiết luôn sở hữu mutex** (tr. 59). Ngoài `std::adopt_lock`, nó nhận **`std::defer_lock`** — construct mà **không khóa**; khóa sau bằng `lk.lock()` hoặc đưa `lk` vào `std::lock()`.

**Listing 3.9 (tr. 60) — swap viết lại bằng `unique_lock` + `defer_lock`:**

```cpp
friend void swap(X& lhs, X& rhs) {
    if (&lhs == &rhs) return;
    std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);  // (1) construct, CHƯA khóa
    std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
    std::lock(lock_a, lock_b);                                    // (2) giờ mới khóa cả hai
    swap(lhs.some_detail, rhs.some_detail);
}
```

`unique_lock` cung cấp `lock()`, `try_lock()`, `unlock()` — chuyển tiếp xuống mutex và **cập nhật một cờ nội bộ** cho biết instance **có đang sở hữu** mutex không. Cờ này cần để destructor gọi `unlock()` **đúng**: có sở hữu thì unlock, không thì thôi. Truy vấn bằng `owns_lock()`.

**⚠️ Cái giá (tr. 60–61):** cờ đó phải **lưu và cập nhật** → `unique_lock` **to hơn và chậm hơn nhẹ** so với `lock_guard`. Khuyến nghị (tr. 61): *"If `std::lock_guard` is sufficient for your needs, I'd recommend using it in preference."* Và nếu chỉ cần khóa nhiều mutex cùng lúc thì **`std::scoped_lock` (C++17) vẫn hơn** `unique_lock`+`std::lock`. Chỉ dùng `unique_lock` khi cần **tính linh hoạt**: deferred locking, hoặc **chuyển quyền sở hữu lock giữa các scope**.

### 5.2 Chuyển quyền sở hữu lock ra khỏi hàm (tr. 61–62)

Vì `unique_lock` **movable-not-copyable**, quyền sở hữu mutex chuyển được bằng move (tự động nếu nguồn là rvalue; phải `std::move` nếu là lvalue):

```cpp
std::unique_lock<std::mutex> get_lock() {
    extern std::mutex some_mutex;
    std::unique_lock<std::mutex> lk(some_mutex);
    prepare_data();
    return lk;                          // (1) trả trực tiếp — compiler tự move
}
void process_data() {
    std::unique_lock<std::mutex> lk(get_lock());   // (2) nhận quyền sở hữu lock
    do_something();                                //     do_something chạy DƯỚI lock đó
}
```

Ứng dụng (tr. 61–62): hàm khóa mutex + chuẩn bị dữ liệu **rồi trao lock cho caller** để caller làm tiếp dưới **cùng một lock** — không thread khác chen vào giữa. Mẫu **gateway class**: object cổng giữ lock làm data member, mọi truy cập qua member function của cổng; hủy cổng = nhả lock.

**Nhả lock sớm (tr. 62):** `unique_lock` cho phép `unlock()` **trước khi** destruct → nếu một nhánh code không cần lock nữa thì nhả ngay, tránh giữ lock lâu hơn cần thiết (giữ lock lâu = thread khác chờ lâu = mất hiệu năng).

### 5.3 Granularity: khóa đúng lượng, giữ đúng lúc (tr. 62–64)

**Định nghĩa (tr. 62):** *lock granularity* = lượng dữ liệu một lock bảo vệ. Fine-grained = ít dữ liệu; coarse-grained = nhiều. Không chỉ phải **đủ coarse để bảo vệ đúng dữ liệu**, mà còn phải **giữ lock chỉ trong đúng thao tác cần nó**.

**Ẩn dụ (tr. 62):** xếp hàng siêu thị mà người trước quên nước sốt cranberry rồi bỏ đi lấy, hoặc tới lượt trả tiền mới lục ví — cả hàng chờ. → **Đừng làm việc tốn thời gian (nhất là file I/O) khi đang giữ lock.** File I/O chậm hơn đọc/ghi bộ nhớ hàng trăm–hàng nghìn lần.

**Mẫu nhả-rồi-khóa-lại bằng `unique_lock` (tr. 62–63):**

```cpp
void get_and_process_data() {
    std::unique_lock<std::mutex> my_lock(the_mutex);
    some_class data = get_next_data_chunk();
    my_lock.unlock();                     // (1) KHÔNG cần lock khi process()
    result_type result = process(data);   //     → thread khác dùng mutex trong lúc này
    my_lock.lock();                       // (2) khóa lại chỉ để ghi kết quả
    write_result(data, result);
}
```

Nguyên tắc chốt (tr. 63): *"**a lock should be held for only the minimum possible time needed to perform the required operations**."*

**⚠️ Nhưng thu nhỏ vùng khóa có thể ĐỔI NGỮ NGHĨA — Listing 3.10 (tr. 63–64):**

```cpp
class Y {
private:
    int some_detail;
    mutable std::mutex m;
    int get_detail() const {
        std::lock_guard<std::mutex> lock_a(m);
        return some_detail;                     // khóa chỉ để đọc 1 int rồi nhả
    }
public:
    Y(int sd) : some_detail(sd) {}
    friend bool operator==(Y const& lhs, Y const& rhs) {
        if (&lhs == &rhs) return true;
        int const lhs_value = lhs.get_detail();  // (1) khóa lhs, đọc, nhả
        int const rhs_value = rhs.get_detail();  // (2) khóa rhs, đọc, nhả  — KHÔNG giữ cùng lúc
        return lhs_value == rhs_value;
    }
};
```

Ưu: mỗi lúc chỉ giữ một lock (không deadlock), giữ ngắn nhất. **Nhược tinh vi (tr. 64):** vì **không giữ cả hai lock cùng lúc**, giữa (1) và (2) giá trị có thể bị đổi (thậm chí bị swap). Nếu `==` trả `true`, nó chỉ nói *"`lhs` tại thời điểm t1 bằng `rhs` tại thời điểm t2"* — **có thể chưa từng có khoảnh khắc nào hai giá trị thật sự bằng nhau**. Câu cảnh báo vàng (tr. 64):

> *"if you don't hold the required locks for the entire duration of an operation, you're exposing yourself to race conditions."*

→ Thu nhỏ granularity phải **không đổi ngữ nghĩa một cách tai hại**. Đôi khi không có mức granularity phù hợp vì không phải truy cập nào cũng cần mức bảo vệ như nhau → cần cơ chế khác `std::mutex` (Cụm 6).

### Insight đáng nhớ (Cụm 5)
- **`lock_guard` là mặc định; `unique_lock` khi cần linh hoạt** (defer, transfer, unlock sớm) — và trả giá bằng cờ owns-lock (to hơn, chậm hơn nhẹ). Khóa-nhiều-mutex thì `scoped_lock` (C++17) hơn cả hai.
- **Giữ lock ngắn nhất có thể; đừng bao giờ làm I/O dưới lock.** Nhưng **đừng thu nhỏ tới mức đổi ngữ nghĩa** (Listing 3.10): một thao tác cần **nhất quán** thì phải giữ đủ lock suốt thao tác, dù đánh đổi song song.
- **Chuyển quyền sở hữu lock** (movable `unique_lock`) mở ra mẫu gateway/return-lock — hàm chuẩn bị dữ liệu dưới lock rồi trao nguyên lock cho caller.

---

## Cụm 6 — Cơ chế thay thế `std::mutex` cho tình huống đặc thù (tr. 64–71)

### 6.1 Bảo vệ **lúc khởi tạo**: `std::once_flag` / `std::call_once` (tr. 64–68)

**Bối cảnh (tr. 64–65):** tài nguyên **đắt để tạo** (mở DB, cấp nhiều bộ nhớ) → muốn **lazy init**: chỉ tạo khi cần. Đơn luồng thì đơn giản:

```cpp
std::shared_ptr<some_resource> resource_ptr;
void foo() {
    if (!resource_ptr)
        resource_ptr.reset(new some_resource);   // ← CHỈ chỗ này cần bảo vệ khi đa luồng
    resource_ptr->do_something();
}
```

**Cách ngây thơ — khóa hết (Listing 3.11, tr. 65): ⚠️ tuần tự hóa không cần thiết.** Mọi thread phải chờ mutex chỉ để **kiểm** xem đã init chưa — kể cả khi đã init xong từ lâu.

**Double-checked locking — ⚠️ SAI, đừng dùng (tr. 65–66):**

```cpp
void undefined_behaviour_with_double_checked_locking() {
    if (!resource_ptr) {                            // (1) đọc NGOÀI lock
        std::lock_guard<std::mutex> lk(resource_mutex);
        if (!resource_ptr) {                        // (2) kiểm lại TRONG lock (double-check)
            resource_ptr.reset(new some_resource);  // (3) ghi TRONG lock
        }
    }
    resource_ptr->do_something();                   // (4)
}
```

Vì sao **UB (tr. 66):** đọc ngoài lock (1) **không đồng bộ** với ghi trong lock (3) ở thread khác → **data race**. Tệ hơn: kể cả khi thread thấy con trỏ đã được ghi, nó có thể **chưa thấy object `some_resource` đã dựng xong** (ghi con trỏ và dựng object có thể bị sắp xếp lại) → `do_something()` (4) chạy trên **object nửa vời**. Đây đúng là **data race** theo Standard = **undefined behavior** (giải thích đầy đủ ở ch. 5).

**Cách đúng — `std::call_once` + `std::once_flag` (tr. 66–67):**

```cpp
std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;                       // dữ liệu đồng bộ cho MỘT init

void init_resource() {
    resource_ptr.reset(new some_resource);
}
void foo() {
    std::call_once(resource_flag, init_resource);   // init CHÍNH XÁC một lần, đã đồng bộ
    resource_ptr->do_something();                   // sau đây chắc chắn init xong
}
```

Ưu điểm (tr. 66): mọi thread yên tâm rằng khi `call_once` return thì con trỏ **đã được init bởi đúng một thread** (theo cách đồng bộ chuẩn). Dữ liệu đồng bộ nằm trong `once_flag`; **mỗi `once_flag` ứng với một init**. *"typically have a lower overhead than using a mutex explicitly, especially when the initialization has already been done."*

**Listing 3.12 (tr. 67) — lazy init cho class member:**

```cpp
class X {
private:
    connection_info connection_details;
    connection_handle connection;
    std::once_flag connection_init_flag;
    void open_connection() {
        connection = connection_manager.open(connection_details);
    }
public:
    X(connection_info const& details) : connection_details(details) {}

    void send_data(data_packet const& data) {
        std::call_once(connection_init_flag, &X::open_connection, this);  // truyền this
        connection.send_data(data);
    }
    data_packet receive_data() {
        std::call_once(connection_init_flag, &X::open_connection, this);
        return connection.receive_data();
    }
};
```

Init xảy ra ở **lần gọi `send_data()` HOẶC `receive_data()` đầu tiên** — cái nào chạy trước. Truyền `this` như đối số phụ (giống constructor `std::thread`/`std::bind`). **⚠️ Lưu ý (tr. 67):** `std::once_flag` **không copy/move được** (như `std::mutex`) → nếu là member thì phải tự định nghĩa special member functions nếu cần.

**Static local — cách C++11 đơn giản hơn cho single global instance (tr. 67–68):**

```cpp
class my_class;
my_class& get_my_class_instance() {
    static my_class instance;      // C++11: init BẢO ĐẢM thread-safe, đúng một thread
    return instance;
}
```

Sách nêu rõ (tr. 67–68): khởi tạo biến `static` local xảy ra **lần đầu control đi qua khai báo**. Trước C++11 đây là race thật (nhiều thread tưởng mình là đầu tiên). **C++11 giải quyết:** init xảy ra trên **đúng một thread**, các thread khác **chờ tới khi init xong**. → thay thế `call_once` khi cần **một global instance duy nhất**. 🆕 Đây chính là nền của **Meyers' Singleton** — pattern singleton an toàn nhất, không cần khóa thủ công.

### 6.2 Bảo vệ dữ liệu **hiếm khi cập nhật**: `std::shared_mutex` (reader-writer) (tr. 68–70)

**Bối cảnh (tr. 68):** cache DNS — một entry gần như bất biến hàng năm, thi thoảng mới thêm/sửa. **Đa số là đọc**; hiếm khi ghi. Dùng `std::mutex` là **quá bi quan** — nó **loại bỏ khả năng đọc song song** ngay cả khi không ai ghi. Cần **reader-writer mutex**: hoặc **một writer độc quyền**, hoặc **nhiều reader chia sẻ đồng thời**.

**Các kiểu Standard cung cấp (tr. 68–69):**
- **`std::shared_mutex`** (C++17) — nhanh hơn nếu không cần thao tác timed.
- **`std::shared_timed_mutex`** (C++14) — thêm thao tác timed (§4.3).
- C++11 **không có** cả hai (dùng Boost).

**Cách dùng ba loại lock trên `shared_mutex` (tr. 69):**
| Mục đích | Lock dùng | Ngữ nghĩa |
|---|---|---|
| **Ghi** (writer) | `std::lock_guard<std::shared_mutex>` hoặc `std::unique_lock<std::shared_mutex>` | **Độc quyền** — như `std::mutex` |
| **Đọc** (reader) | `std::shared_lock<std::shared_mutex>` (C++14) | **Chia sẻ** — nhiều reader cùng lúc |

Ràng buộc (tr. 69): nếu có reader đang giữ shared lock, writer xin exclusive lock **phải chờ** tới khi mọi reader nhả; ngược lại nếu writer đang giữ exclusive lock, không reader/writer nào vào được.

**Listing 3.13 (tr. 69–70) — DNS cache:**

```cpp
#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>

class dns_entry;
class dns_cache {
    std::map<std::string, dns_entry> entries;
    mutable std::shared_mutex entry_mutex;
public:
    dns_entry find_entry(std::string const& domain) const {
        std::shared_lock<std::shared_mutex> lk(entry_mutex);   // (1) SHARED: nhiều reader OK
        auto const it = entries.find(domain);
        return (it == entries.end()) ? dns_entry() : it->second;
    }
    void update_or_add_entry(std::string const& domain,
                             dns_entry const& dns_details) {
        std::lock_guard<std::shared_mutex> lk(entry_mutex);    // (2) EXCLUSIVE: writer độc quyền
        entries[domain] = dns_details;
    }
};
```

`find_entry()` dùng `shared_lock` → nhiều thread gọi đồng thời không sao; `update_or_add_entry()` dùng `lock_guard` (exclusive) → khi ghi thì **chặn cả reader lẫn writer khác**.

**⚠️ Không phải thuốc tiên (tr. 69):** hiệu năng phụ thuộc **số CPU** và **tỉ lệ reader/writer** — *"it's important to profile the performance of the code on the target system"* (chi tiết ch. 8). Nếu writer nhiều, chi phí quản lý shared lock có thể **tệ hơn** `std::mutex` thường.

### 6.3 `std::recursive_mutex` — và vì sao thường là dấu hiệu thiết kế xấu (tr. 70–71)

Với `std::mutex`, thread khóa lại mutex nó **đã giữ** = **UB**. `std::recursive_mutex` cho phép **cùng một thread khóa nhiều lần** — nhưng phải `unlock()` đúng số lần đã `lock()` (khóa 3 lần → mở 3 lần). Dùng với `lock_guard`/`unique_lock` sẽ tự lo.

**⚠️ Nhưng sách khuyên tránh (tr. 70):** *"Most of the time, if you think you want a recursive mutex, you probably need to change your design instead."* Tình huống điển hình: class có mutex, mỗi public method lock–làm–unlock; rồi một public method **gọi public method khác** → khóa lại → UB. "Sửa nhanh bẩn" là đổi sang recursive mutex → cho khóa lại được.

**Vì sao đó là thiết kế xấu (tr. 70):** khi một public method đang giữ lock, **invariant của class thường đang bị phá** — nên public method thứ hai (được gọi lồng) phải **chạy đúng ngay cả khi invariant vỡ**, điều thường sai. **Cách đúng:** tách một **private method không khóa** (giả định lock đã giữ), cho cả hai public method gọi; rồi suy nghĩ kỹ về trạng thái dữ liệu khi gọi nó.

### Insight đáng nhớ (Cụm 6)
- **Chọn cơ chế theo pattern truy cập, đừng mặc định `std::mutex`:** chỉ bảo vệ init → `call_once`/static local; đọc-nhiều-ghi-hiếm → `shared_mutex`; đệ quy → xem lại thiết kế trước khi dùng `recursive_mutex`.
- **Double-checked locking là UB kinh điển — biết để KHÔNG viết.** Thay bằng `call_once` hoặc static local. Đây là bẫy phỏng vấn rất hay gặp.
- **Static local (C++11) = Meyers' Singleton:** init thread-safe không cần khóa tay — cách hiện thực singleton sạch nhất.
- **Recursive mutex thường che giấu invariant vỡ.** Trả lời "tôi sẽ tách private helper không khóa" cho thấy tư duy thiết kế, hơn là "đổi sang recursive_mutex".

---

## Tóm tắt chương (theo sách, tr. 71)

Chương này bàn: race condition có vấn đề tai hại thế nào, và dùng `std::mutex` + **thiết kế interface cẩn thận** để tránh; mutex không phải thuốc tiên — sinh **deadlock**, mà `std::lock()` giúp tránh; các kỹ thuật khác tránh deadlock; chuyển quyền sở hữu lock; chọn **granularity** phù hợp; và các cơ chế bảo vệ đặc thù (`call_once`, `shared_mutex`).

**Câu chuyển sang ch. 4 (tr. 71):** còn một thứ chưa bàn — **chờ input từ thread khác**. `threadsafe_stack` ném exception khi rỗng; muốn một thread **chờ** thread khác `push` thì phải lặp thử `pop` liên tục — **phí CPU vô ích**. Cần cách để thread **chờ một sự kiện mà không đốt CPU** → **condition variable & future** (ch. 4).

**🆕 Bảng chốt — các cơ chế của chương 3 và khi nào dùng:**

| Cơ chế | Dùng khi | Chuẩn |
|---|---|---|
| `std::mutex` + `std::lock_guard` | Bảo vệ dữ liệu chung mặc định | C++11 |
| `std::scoped_lock` | Khóa **nhiều mutex** cùng lúc (chống deadlock) | C++17 |
| `std::lock` + `adopt_lock`/`defer_lock` | Như trên, compiler cũ | C++11 |
| `std::unique_lock` | Cần **defer / transfer / unlock sớm** | C++11 |
| `std::shared_mutex` + `shared_lock` | **Đọc nhiều, ghi hiếm** (reader-writer) | C++17 |
| `std::call_once` + `once_flag` | Bảo vệ **khởi tạo** (lazy init) | C++11 |
| `static` local | **Một global instance** duy nhất (Meyers' Singleton) | C++11 |
| `std::recursive_mutex` | Cùng thread khóa lại — **thường nên tránh** | C++11 |
| Hierarchical mutex (tự viết) | Kiểm **thứ tự lock** tại runtime | — |

---

## Góc interview

**Câu 1 (🎯🎯 gần như chắc chắn hỏi):** Cho một `stack` có `top()`, `pop()`, `empty()`. Ngay cả khi bọc mutex bên trong, vì sao vẫn có race? Thiết kế lại một `threadsafe_stack` đúng.

<details><summary>Đáp án</summary>

**Đây là "race vốn có trong interface" (§3.2.3, tr. 44–46)** — bug **thiết kế**, không phải thiếu khóa. Mutex nội bộ **không cứu được**, và bản lock-free cũng dính đúng vấn đề (tr. 44).

**Hai race:**
1. **`empty()`/`top()` (tr. 44–45):** kết quả `empty()` đúng lúc gọi nhưng ngay sau khi return đã lỗi thời. Giữa `if(!s.empty())` và `s.top()`, thread khác có thể `pop()` sạch → `top()` trên stack rỗng = UB.
2. **`top()`/`pop()` (tr. 45–46, Table 3.1):** hai thread cùng chạy `value=s.top(); s.pop();`. Cả hai `top()` **đọc cùng một phần tử X** (chưa ai pop), rồi **hai lần `pop()`** xóa cả X lẫn Y. Kết quả: **X xử lý hai lần, Y mất trắng.** Đây là race *"far more insidious"* vì không có gì trông sai rõ ràng.

**Vì sao `std::stack` tách `top()`/`pop()` (nghịch lý Cargill, tr. 46–47):** để **exception-safety**. Nếu `pop()` vừa xóa vừa trả về, mà copy giá trị trả về ném exception (vd `vector` copy phải xin heap → `bad_alloc`), thì **dữ liệu đã xóa khỏi stack nhưng mất luôn**. Tách đôi → copy hỏng thì dữ liệu **còn trên stack**. Nghịch lý: ta muốn *gộp* (hết race) đúng cái mà exception-safety *tách*.

**Bốn phương án gộp an toàn (tr. 47–48):** (1) truyền reference `pop(T&)`; (2) yêu cầu copy/move ctor `noexcept`; (3) trả `shared_ptr` (pointer copy không ném → tránh Cargill, tự quản bộ nhớ); (4) kết hợp.

**Thiết kế (Listing 3.5, cài option 1+3):**

```cpp
template<typename T>
class threadsafe_stack {
    std::stack<T> data;
    mutable std::mutex m;                          // mutable: lock được trong empty() const
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m); // khóa nguồn, copy trong thân ctor
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();     // kiểm rỗng TRONG lock → diệt race #1
        auto const res = std::make_shared<T>(data.top());  // cấp phát TRƯỚC khi sửa → chống Cargill
        data.pop();                                // gộp top+pop → diệt race #2
        return res;
    }
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = data.top();
        data.pop();
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
```

Điểm ăn điểm: **kiểm `empty` bên trong lock**, **gộp `top`+`pop`**, **`make_shared` trước `data.pop()`** (chống Cargill), **`mutable mutex`**, **copy ctor khóa nguồn**. Nêu được cả nghịch lý Cargill là mức "senior".

**⚠️ Hạn chế còn lại:** thiết kế này vẫn **ném exception khi rỗng** — muốn thread *chờ* tới khi có phần tử thì cần **condition variable** (ch. 4), không lặp thử `pop` đốt CPU.

</details>

**Câu 2 (🎯🎯 kinh điển):** Deadlock là gì? Viết hàm `swap(X&, X&)` cần khóa hai mutex mà **không** deadlock. Kể các cách tránh deadlock nói chung.

<details><summary>Đáp án</summary>

**Deadlock (tr. 51):** mỗi thread cần khóa **cả hai** mutex, mỗi thread đang giữ một và **chờ** cái kia → không ai tiến. 🆕 **4 điều kiện Coffman**: mutual exclusion, hold-and-wait, no-preemption, **circular wait** — phá bất kỳ điều nào là hết deadlock.

**Bẫy của "khóa cùng thứ tự" (tr. 51):** với `swap` hai instance cùng class, nếu quy ước "khóa tham số 1 trước tham số 2" thì `swap(x,y)` và `swap(y,x)` chạy đồng thời vẫn **deadlock**.

**Giải đúng — `std::scoped_lock` (C++17, tr. 52–53):**
```cpp
friend void swap(X& lhs, X& rhs) {
    if (&lhs == &rhs) return;              // cùng instance → khỏi khóa (khóa lại = UB)
    std::scoped_lock guard(lhs.m, rhs.m);  // khóa CẢ HAI bằng thuật toán tránh deadlock
    swap(lhs.some_detail, rhs.some_detail);
}
```
Hoặc C++11:
```cpp
std::lock(lhs.m, rhs.m);
std::lock_guard<std::mutex> a(lhs.m, std::adopt_lock);  // adopt: nhận lock đã có
std::lock_guard<std::mutex> b(rhs.m, std::adopt_lock);
```
Điểm cộng — **all-or-nothing (tr. 52):** nếu `std::lock` khóa được cái đầu mà cái sau ném exception, cái đầu **tự động nhả**. `scoped_lock` phá **hold-and-wait** vì lấy tất cả trong một thao tác.

**Các guideline tránh deadlock (tr. 53–55), quy về một câu:** *đừng chờ thread khác nếu nó có thể đang chờ bạn*:
1. **Tránh nested lock** — đừng lấy lock khi đang giữ lock; cần nhiều thì `std::lock` lấy một lần.
2. **Đừng gọi user code khi giữ lock** — nó có thể lock thứ khác.
3. **Khóa theo thứ tự cố định** ở mọi thread (khi không dùng `std::lock` được) — vd hand-over-hand trên list phải quy định chiều duyệt, nếu không hai thread duyệt ngược chiều deadlock (Figure 3.2).
4. **Lock hierarchy** — gán số tầng, chỉ được khóa mutex tầng **thấp hơn** cái đang giữ; kiểm được **runtime, không phụ thuộc timing** (tr. 59). Deadlock giữa hierarchical mutex là **bất khả thi** vì mutex tự ép thứ tự.

**Mở rộng (tr. 59):** deadlock xảy ra với **mọi wait-cycle**, không riêng lock — vd hai thread `join()` lẫn nhau. Nên: đừng chờ thread khi giữ lock; join thread ở đúng hàm đã tạo nó.

</details>

**Câu 3 (🎯):** Double-checked locking để lazy-init một `shared_ptr` — có đúng không? Cách đúng là gì?

<details><summary>Đáp án</summary>

**SAI — undefined behavior (tr. 65–66).**
```cpp
if (!resource_ptr) {                              // (1) đọc NGOÀI lock
    std::lock_guard<std::mutex> lk(m);
    if (!resource_ptr)                            // (2) double-check trong lock
        resource_ptr.reset(new some_resource);    // (3) ghi trong lock
}
resource_ptr->do_something();                     // (4)
```
Đọc (1) **không đồng bộ** với ghi (3) ở thread khác → **data race**. Tệ hơn: thread có thể **thấy con trỏ đã ghi nhưng chưa thấy object dựng xong** (ghi con trỏ và khởi tạo object bị sắp xếp lại) → `do_something()` chạy trên **object nửa vời**. Data race theo C++ Standard = **UB** (ch. 5 giải thích qua memory model).

**Cách đúng #1 — `std::call_once` (tr. 66):**
```cpp
std::once_flag flag;
void foo() {
    std::call_once(flag, init_resource);   // init đúng MỘT lần, đã đồng bộ chuẩn
    resource_ptr->do_something();
}
```
Overhead thấp hơn khóa mutex thủ công, nhất là khi đã init xong. `once_flag` không copy/move được.

**Cách đúng #2 — static local (C++11, tr. 67–68), khi cần một global instance:**
```cpp
my_class& instance() {
    static my_class inst;   // C++11 bảo đảm init thread-safe, đúng một thread
    return inst;
}
```
Đây là **Meyers' Singleton** — cách hiện thực singleton an toàn nhất, không cần khóa tay. C++11 định nghĩa: init xảy ra trên đúng một thread, các thread khác chờ tới khi xong.

**Chốt:** đừng bao giờ tự viết double-checked locking; dùng `call_once` hoặc static local — cả hai đều đã giải quyết đúng phần đồng bộ mà bản thủ công làm sai.

</details>

**Câu 4 (🟠):** Dữ liệu đọc rất nhiều, ghi rất hiếm (vd cache DNS). Dùng `std::mutex` có gì phí? Dùng gì thay thế?

<details><summary>Đáp án</summary>

**`std::mutex` quá bi quan (tr. 68):** nó cho **đúng một thread** vào tại một thời điểm, kể cả các thread chỉ **đọc**. Với dữ liệu đọc-nhiều-ghi-hiếm, điều này **triệt tiêu song song đọc** một cách vô ích — nhiều reader lẽ ra chạy đồng thời được.

**Giải: reader-writer mutex — `std::shared_mutex` (C++17, tr. 68–69):**
- **Reader** dùng `std::shared_lock<std::shared_mutex>` → **nhiều reader cùng lúc**.
- **Writer** dùng `std::lock_guard`/`std::unique_lock<std::shared_mutex>` → **độc quyền**.
- Ràng buộc: có reader thì writer chờ hết reader; có writer thì không ai vào.

```cpp
class dns_cache {
    std::map<std::string, dns_entry> entries;
    mutable std::shared_mutex m;
public:
    dns_entry find_entry(std::string const& domain) const {
        std::shared_lock<std::shared_mutex> lk(m);   // đọc: chia sẻ
        auto it = entries.find(domain);
        return it == entries.end() ? dns_entry() : it->second;
    }
    void update_or_add_entry(std::string const& domain, dns_entry const& d) {
        std::lock_guard<std::shared_mutex> lk(m);    // ghi: độc quyền
        entries[domain] = d;
    }
};
```

**Phiên bản (tr. 68–69):** `std::shared_mutex` (C++17), `std::shared_timed_mutex` (C++14, thêm thao tác timed), C++11 không có (dùng Boost).

**⚠️ Cảnh báo (tr. 69):** không phải luôn nhanh hơn — phụ thuộc **số CPU** và **tỉ lệ reader/writer**; nếu ghi nhiều, chi phí quản lý shared lock có thể **tệ hơn** `std::mutex` thường. **Phải profile trên hệ đích** (ch. 8).

</details>

**Câu 5 (🟠):** `std::lock_guard`, `std::unique_lock`, `std::scoped_lock` khác nhau thế nào? Khi nào dùng cái nào?

<details><summary>Đáp án</summary>

| | `std::lock_guard` (C++11) | `std::unique_lock` (C++11) | `std::scoped_lock` (C++17) |
|---|---|---|---|
| Bản chất | RAII tối giản: lock lúc construct, unlock lúc destruct | RAII **linh hoạt**: có thể không sở hữu mutex | RAII cho **nhiều mutex** cùng lúc |
| `defer_lock` / `adopt_lock` | Chỉ `adopt_lock` | **Cả hai** + lock/unlock/try_lock lại | Khóa tất cả bằng thuật toán `std::lock` |
| Unlock sớm | Không | **Có** (`unlock()`) | Không |
| Move (chuyển scope) | Không | **Có** (movable) | Không |
| Chi phí | Nhỏ nhất | **To hơn + chậm hơn nhẹ** (cờ owns-lock) | Như `lock_guard`, cho nhiều mutex |

**Chọn (tr. 60–61):**
- **Mặc định: `std::lock_guard`** — *"If `std::lock_guard` is sufficient, use it in preference"* (tr. 61).
- **Khóa nhiều mutex cùng lúc: `std::scoped_lock`** — gọn nhất, chống deadlock, ít lỗi hơn `std::lock`+`adopt_lock` (tr. 53).
- **`std::unique_lock` chỉ khi cần linh hoạt (tr. 61):**
  1. **Deferred locking** (`defer_lock`) — construct trước, khóa sau (vd đưa vào `std::lock`).
  2. **Chuyển quyền sở hữu lock** giữa các scope (movable) — mẫu return-lock/gateway (tr. 61–62).
  3. **Unlock sớm** — nhả lock giữa chừng để không giữ lâu hơn cần (vd không giữ lock qua lời gọi `process()` hay file I/O — tr. 62–63).

**Điểm cộng — cái giá của `unique_lock`:** nó lưu **cờ owns-lock** (để destructor biết có nên unlock không, truy vấn bằng `owns_lock()`), nên **to hơn và chậm hơn** `lock_guard`. Đừng dùng nếu không cần tính linh hoạt.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [ostep/concurrency.md](../ostep/concurrency.md) — lock/mutex nhìn từ tầng OS: spinlock, futex, cách lock được xây từ phần cứng (test-and-set, CAS); deadlock 4 điều kiện Coffman.
- [03-operating-system/sync-primitives.md](../../03-operating-system/sync-primitives.md) — bản cô đọng mutex/semaphore/deadlock của repo.
- [EMC++ cụm 7](../effective-modern-cpp.md) — Item 40 (`std::atomic` vs `volatile`), bổ trợ cho double-checked locking.
- [lkd/sync-timers.md](../lkd/sync-timers.md) — spinlock vs mutex, `spin_lock_irqsave` trong kernel — cùng bài toán loại trừ lẫn nhau ở tầng kernel.
- Ch. 5 (memory model) giải thích **vì sao** double-checked locking là data race và `call_once` thì không; ch. 6–7 xây các cấu trúc dữ liệu concurrent (lock-based rồi lock-free) trên nền chương này.

**Chương tiếp theo:** [Ch. 4 — Synchronizing concurrent operations →](04-synchronization.md) (condition variable & thread-safe queue, **future/promise/packaged_task/async**, `std::this_thread::sleep_for`, timeout & clock, latches/barriers) — trả lời câu hỏi chương này để ngỏ: làm sao **chờ một sự kiện** mà không đốt CPU.
