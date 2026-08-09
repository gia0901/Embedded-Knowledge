# Ch. 6 — Designing lock-based concurrent data structures (tr. 173–204) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 5 — Memory model & atomics](05-memory-model.md)** · **[Ch. 7 → Lock-free data structures](07-lock-free-structures.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Đây là chương "thực hành thiết kế".** Sau ch. 5 (low-level nặng lý thuyết), chương này **nghỉ khỏi low-level** và áp dụng mọi thứ đã học (ch. 3 mutex, ch. 4 condition variable) để **thiết kế container thread-safe thật**. Mạch xuyên suốt: đi từ **coarse-grained** (một mutex cho cả cấu trúc) sang **fine-grained** (nhiều mutex, nhiều concurrency hơn) — và mỗi bước phải soi lại 4 tiêu chí an toàn. Giá trị interview: **thiết kế thread-safe queue/stack** là câu hỏi coding-design rất hay gặp cho vị trí System SW/C++.

**Sách nêu mục tiêu chương (tr. 173):** thiết kế cấu trúc dữ liệu cho concurrency nghĩa là gì · guideline · các ví dụ cài đặt (stack, queue, hash map, linked list).

---

## Cụm 1 — "Thiết kế cho concurrency" nghĩa là gì + guideline (tr. 174–176)

### 1.1 Thread-safe vs "tạo cơ hội cho concurrency thật" (tr. 174)

**Thread-safe (tr. 174):** nhiều thread truy cập đồng thời, mỗi thread thấy **view nhất quán**, không mất/hỏng dữ liệu, mọi invariant được giữ, không race. Nhưng thường chỉ an toàn cho **một số kiểu truy cập nhất định** (vd nhiều reader ok, writer cần độc quyền).

**⚠️ Nhưng thiết kế thật sự cho concurrency là NHIỀU HƠN thế (tr. 174):** phải **tạo cơ hội cho concurrency** cho các thread. Bản chất mutex là **serialization** — thread thay phiên nhau, truy cập **tuần tự chứ không đồng thời**. Câu chốt:

> *"the smaller the protected region, the fewer operations are serialized, and the greater the potential for concurrency."* (tr. 174)

→ 🆕 Đây là **căng thẳng trung tâm của cả chương**: an toàn đòi khóa; hiệu năng đòi khóa **ít và nhỏ**. Mọi thiết kế fine-grained là một điểm cân bằng giữa hai thứ đó.

### 1.2 Bốn tiêu chí AN TOÀN (nhắc lại ch. 3) (tr. 175)

1. **Không thread nào thấy được invariant đang vỡ** do thread khác gây ra.
2. **Tránh race-trong-interface** — cung cấp hàm cho **thao tác hoàn chỉnh**, không phải từng bước (như gộp `top()`+`pop()` ch. 3).
3. **Chú ý exception** — đảm bảo invariant không vỡ khi có exception.
4. **Tối thiểu hóa cơ hội deadlock** — thu hẹp phạm vi lock, tránh nested lock.

Trước tất cả: phải quyết **ràng buộc lên người dùng** — hàm nào an toàn gọi từ thread khác khi một thread đang dùng? (tr. 175). Thường **constructor/destructor cần độc quyền** — người dùng phải bảo đảm không truy cập trước khi construct xong / sau khi destruct bắt đầu. `swap()`/copy/assign thì tùy thiết kế.

### 1.3 Bốn câu hỏi để TĂNG CONCURRENCY (tr. 175–176)

- Có thu hẹp phạm vi lock để làm **một phần thao tác NGOÀI lock** được không?
- Có bảo vệ **các phần khác nhau** bằng **mutex khác nhau** được không?
- Mọi thao tác có cần **cùng mức bảo vệ** không?
- Một thay đổi nhỏ ở cấu trúc dữ liệu có tăng concurrency mà **không đổi ngữ nghĩa** không?

Tất cả quy về một ý (tr. 175): **tối thiểu hóa serialization, tối đa hóa concurrency thật.** Mẫu phổ biến: nhiều reader đồng thời + writer độc quyền → `std::shared_mutex`.

### Insight đáng nhớ (Cụm 1)
- **Thread-safe là điều kiện cần, không đủ.** Một mutex bọc mọi hàm → thread-safe nhưng **serialize hết** → mất lợi ích multithread. Mục tiêu là an toàn **VÀ** đồng thời.
- **Vùng bảo vệ càng nhỏ, concurrency càng nhiều.** Câu này định hướng mọi quyết định fine-grained trong chương.

---

## Cụm 2 — thread-safe stack & queue với MỘT mutex (coarse-grained) (tr. 176–182)

### 2.1 thread-safe stack — phân tích 4 tiêu chí (Listing 6.1, tr. 176–179)

Đây chính là `threadsafe_stack` từ ch. 3. Chương này **phân tích kỹ từng tiêu chí** — mẫu tư duy đáng học:

```cpp
template<typename T>
class threadsafe_stack {
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack() {}
    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.m);
        data = other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value) {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));          // (B) có thể ném — nhưng std::stack đảm bảo an toàn
    }
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();    // (c) ném khi rỗng — CHƯA sửa gì → an toàn
        std::shared_ptr<T> const res(
            std::make_shared<T>(std::move(data.top())));  // (d) make_shared có thể ném — CHƯA pop → an toàn
        data.pop();                               // (e) không ném
        return res;
    }
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = std::move(data.top());            // (f) copy/move-assign có thể ném — CHƯA pop → an toàn
        data.pop();                               // (g)
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
```

**Phân tích của sách (tr. 177–179):**
- **An toàn cơ bản:** mỗi hàm khóa `m` → chỉ một thread chạm data → không ai thấy invariant vỡ.
- **Race-trong-interface:** `empty()`/`pop()` có nguy cơ, nhưng `pop()` **kiểm `empty` bên trong lock** và trả dữ liệu **trực tiếp** (gộp top+pop) → hết race.
- **Exception (tr. 178):** phân tích từng điểm ném — mấu chốt là **thao tác có thể ném luôn nằm TRƯỚC `data.pop()`** (thao tác sửa cấu trúc). `make_shared` ném (d) thì stack **chưa bị đụng**. → exception-safe.
- **⚠️ Deadlock (tr. 178):** vẫn có nguy cơ vì **gọi user code khi giữ lock** — copy/move constructor của `T` (B, d), copy-assign (f), user-defined `operator new`. Nếu chúng lại gọi member của stack hoặc lấy lock khác → deadlock. Nhưng **hợp lý bắt người dùng chịu trách nhiệm** (không thể push/pop mà không copy/cấp phát).

**⚠️ Hai hạn chế lớn của thiết kế một-mutex (tr. 179):**
1. **Serialize hoàn toàn:** dù nhiều thread gọi được, **chỉ một thread làm việc tại một thời điểm** → contention cao thì mất hiệu năng.
2. **Không có cách CHỜ item được thêm:** thread muốn chờ phải **liên tục gọi `empty()`** hoặc `pop()` bắt exception → phí tài nguyên. → dùng condition variable trong queue (Cụm sau).

### 2.2 thread-safe queue với condition variable (Listing 6.2, tr. 179–181)

Cùng cấu trúc stack, thêm `data_cond.notify_one()` trong `push()`, và `wait_and_pop()` giải bài toán chờ:
```cpp
void push(T new_value) {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(std::move(new_value));
    data_cond.notify_one();                    // đánh thức thread đang wait_and_pop
}
void wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this]{ return !data_queue.empty(); });  // chờ không đốt CPU
    value = std::move(data_queue.front());
    data_queue.pop();
}
```
`try_pop()` giống `pop()` của stack nhưng **không ném khi rỗng** (trả `bool`/con trỏ NULL). `wait_and_pop()` **không thêm race/deadlock mới** vì `wait()` không trả về tới khi queue có phần tử, và dữ liệu vẫn được lock bảo vệ.

**⚠️ Vấn đề exception tinh vi (tr. 181):** nếu nhiều thread chờ, `notify_one()` đánh thức **một** thread; nếu thread đó **ném trong `wait_and_pop()`** (khi dựng `shared_ptr`), **không thread nào khác được đánh thức** → item kẹt. Ba cách sửa: (a) `notify_all()` (nhưng phần lớn thread thức rồi ngủ lại); (b) `wait_and_pop()` gọi `notify_one()` khi ném; (c) **chuyển khởi tạo `shared_ptr` sang `push()`** — lưu `shared_ptr` thay vì giá trị trực tiếp.

### 2.3 Lưu `shared_ptr` — Listing 6.3 (tr. 181–182)

Cách (c) — queue chứa `std::queue<std::shared_ptr<T>>`:
```cpp
void push(T new_value) {
    std::shared_ptr<T> data(
        std::make_shared<T>(std::move(new_value)));   // (f) cấp phát NGOÀI lock!
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(data);
    data_cond.notify_one();
}
```
**Hai lợi ích (tr. 182):** (1) copy `shared_ptr` ra khỏi queue **không ném** → `wait_and_pop` an toàn lại; (2) **cấp phát bộ nhớ làm NGOÀI lock** (f) → giảm thời gian giữ mutex → tăng concurrency. Đây là ý tưởng dẫn đường: **đẩy việc tốn giờ (cấp phát) ra ngoài vùng khóa.**

**⚠️ Nhưng vẫn serialize (tr. 182):** vì dùng `std::queue<>` — **một item được bảo vệ hoặc không**, một mutex cho cả cái. Muốn fine-grained phải **tự cài cấu trúc** để đặt mutex cho từng phần.

### Insight đáng nhớ (Cụm 2)
- **Mẫu phân tích 4 tiêu chí** (an toàn cơ bản → race-interface → exception → deadlock) là **quy trình đáng học** khi được yêu cầu "thiết kế thread-safe X".
- **Exception-safety qua thứ tự:** đặt mọi thao tác có-thể-ném **TRƯỚC** thao tác sửa cấu trúc (`data.pop()`) → nếu ném thì cấu trúc chưa bị đụng.
- **Đẩy cấp phát ra ngoài lock** (lưu `shared_ptr`, `make_shared` trong `push`) → giảm thời gian giữ mutex. Nguyên tắc chung: **việc tốn giờ đừng làm dưới lock.**

---

## Cụm 3 — 🎯 thread-safe queue với FINE-GRAINED locking (tr. 183–194)

Đây là cụm kỹ thuật nhất chương, và là **câu hỏi thiết kế hay gặp**.

### 3.1 Vì sao một singly-linked-list ngây thơ KHÔNG fine-grained được (Listing 6.4, tr. 183–185)

Queue = singly linked list, `head` (unique_ptr) + `tail` (raw pointer). Ý định: hai mutex, một cho `head`, một cho `tail`. **⚠️ Nhưng hai vấn đề (tr. 184–185):**
1. `push()` sửa **cả `tail` và `head`** khi queue rỗng (Listing 6.4: `head = std::move(p)`) → phải khóa cả hai.
2. **Vấn đề nghiêm trọng:** cả `push()` và `try_pop()` đụng **`next` pointer của một node**: `push` sửa `tail->next`, `try_pop` đọc `head->next`. Nếu queue **chỉ một item** thì `head == tail` → `head->next` và `tail->next` **là cùng một object** → phải khóa cùng mutex ở cả hai → **không hơn gì một mutex**.

### 3.2 Lời giải: **dummy node** tách head khỏi tail (Listing 6.5, tr. 185–186)

**Ý tưởng then chốt (tr. 185):** preallocate một **dummy node không dữ liệu** để **luôn có ít nhất một node** — tách node ở head khỏi node ở tail. Queue rỗng → cả `head` và `tail` trỏ dummy node (không phải NULL). `try_pop()` không đụng `head->next` khi rỗng. Có ≥1 item thật → `head` và `tail` trỏ **node KHÁC nhau** → hết race trên `head->next`/`tail->next`. Giá phải trả: thêm một tầng gián tiếp (data lưu bằng `shared_ptr`).

```cpp
template<typename T>
class queue {
    struct node {
        std::shared_ptr<T> data;           // lưu bằng pointer để cho phép dummy node
        std::unique_ptr<node> next;
    };
    std::unique_ptr<node> head;
    node* tail;
public:
    queue() : head(new node), tail(head.get()) {}   // dummy node ban đầu
    std::shared_ptr<T> try_pop() {
        if (head.get() == tail) return std::shared_ptr<T>();  // so head==tail thay vì NULL
        std::shared_ptr<T> const res(head->data);
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return res;
    }
    void push(T new_value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);       // node MỚI làm dummy node mới
        tail->data = new_data;                   // gán data vào dummy node CŨ
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
};
```
**Cái được (tr. 186):** `push()` giờ chỉ đụng `tail`, `try_pop()` đụng `head` (và `tail` chỉ để so sánh ban đầu). **`push` và `try_pop` không bao giờ thao tác cùng một node** → dùng **hai mutex riêng** được.

### 3.3 Đặt lock đúng chỗ + bẫy `get_tail` (Listing 6.6, tr. 186–190)

```cpp
template<typename T>
class threadsafe_queue {
    struct node { std::shared_ptr<T> data; std::unique_ptr<node> next; };
    std::mutex head_mutex;
    std::unique_ptr<node> head;
    std::mutex tail_mutex;
    node* tail;

    node* get_tail() {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);   // khóa tail_mutex chỉ để đọc tail
        return tail;
    }
    std::unique_ptr<node> pop_head() {
        std::lock_guard<std::mutex> head_lock(head_mutex);
        if (head.get() == get_tail()) {          // ⚠️ get_tail() PHẢI ở TRONG head_mutex
            return nullptr;
        }
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }
public:
    threadsafe_queue() : head(new node), tail(head.get()) {}
    std::shared_ptr<T> try_pop() {
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }
    void push(T new_value) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        std::unique_ptr<node> p(new node);
        node* const new_tail = p.get();
        std::lock_guard<std::mutex> tail_lock(tail_mutex);   // khóa tail_mutex quanh mọi truy cập tail
        tail->data = new_data;
        tail->next = std::move(p);
        tail = new_tail;
    }
};
```

**⚠️ Vì sao `tail_mutex` cần thiết cả cho việc đọc data từ head (tr. 188):** không phải chỉ bảo vệ `tail` — nó tạo **defined ordering** giữa `push` và `pop`. `get_tail()` khóa **cùng mutex** như `push` → hoặc `get_tail` chạy trước `push` (thấy tail cũ), hoặc sau (thấy tail mới + data mới). Không có nó → data race (như ch. 5).

**⚠️ Bẫy kinh điển — `get_tail()` PHẢI ở TRONG lock của `head_mutex` (tr. 188–189):** bản SAI đặt `get_tail()` ngoài lock:
```cpp
// ❌ BẢN HỎNG:
std::unique_ptr<node> pop_head() {
    node* const old_tail = get_tail();                       // lấy tail NGOÀI head_mutex
    std::lock_guard<std::mutex> head_lock(head_mutex);       // rồi mới khóa head
    if (head.get() == old_tail) { ... }                      // ⚠️ head & tail có thể ĐÃ đổi
    // → head có thể bị đẩy VƯỢT tail, off khỏi list → hỏng cấu trúc
}
```
Vấn đề: giữa `get_tail()` và khóa `head_mutex`, thread khác có thể `pop_head()` trước → khi thread này lấy được lock, `old_tail` **không còn là tail, thậm chí không còn trong list**. Bản đúng giữ `get_tail()` **trong** `head_mutex` → không thread nào đổi được `head`, `tail` chỉ đi xa hơn (an toàn).

**Phân tích (tr. 189–190):**
- **Deadlock:** chỗ duy nhất khóa hai lock là `pop_head()` — luôn `head_mutex` **rồi** `tail_mutex`, thứ tự cố định → không deadlock.
- **Concurrency (tr. 189–190):** cấp phát node/data trong `push` **không giữ lock** → nhiều thread cấp phát song song; thêm node vào list chỉ vài phép gán con trỏ → giữ lock rất ngắn. `try_pop` giữ `tail_mutex` chỉ để đọc → **gần như toàn bộ `try_pop` chạy song song với `push`**. `delete` node (tốn) nằm **ngoài** `head_mutex`.

### 3.4 Thêm `wait_and_pop` với fine-grained locking (Listing 6.7–6.10, tr. 190–194)

`wait_and_pop` phức tạp hơn: điều kiện chờ là "queue không rỗng" = `head != tail`. Nhưng ta đã biết chỉ cần `tail_mutex` để **đọc** `tail` → predicate `head != get_tail()` chỉ cần giữ `head_mutex`. Điểm hay của cài đặt: `wait_for_data()` **trả cả lock** cho caller để giữ nguyên lock khi sửa list:

```cpp
std::unique_lock<std::mutex> wait_for_data() {
    std::unique_lock<std::mutex> head_lock(head_mutex);
    data_cond.wait(head_lock, [&]{ return head.get() != get_tail(); });
    return std::move(head_lock);              // TRẢ lock cho caller giữ tiếp
}
std::unique_ptr<node> wait_pop_head(T& value) {
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    value = std::move(*head->data);           // lấy value TRONG lock (trước khi bỏ node)
    return pop_head();
}
```
**⚠️ Exception-safety với overload nhận `T& value` (tr. 190–191):** nếu bỏ node ra rồi mới copy-assign vào `value` mà copy ném → **mất dữ liệu** (không trả về queue được). Sửa: **di chuyển thao tác có-thể-ném vào TRONG vùng khóa, TRƯỚC khi bỏ node** (đó là lý do `wait_pop_head(T&)` lấy `value` trước `pop_head()`).

**⚠️ Cải tiến nhỏ với `notify_one` (tr. 190):** với fine-grained, nên **unlock trước khi `notify_one()`** — nếu để lock khi notify, thread được đánh thức có thể phải chờ lock. `push` (Listing 6.8) đóng scope `tail_mutex` **trước** `data_cond.notify_one()`.

Sách nhắc queue này là **unbounded** (tr. 194); bounded queue (giới hạn độ dài, `push` chờ khi đầy) hữu ích để cân bằng việc giữa thread — mở rộng dễ bằng cách wait trong `push`.

### Insight đáng nhớ (Cụm 3)
- **Dummy node là thủ pháp then chốt** để tách head khỏi tail → cho phép hai mutex. Ý tưởng "thêm một tầng gián tiếp để tách vùng tranh chấp" tái xuất nhiều nơi trong lock-free (ch. 7).
- **⚠️ Bẫy `get_tail` trong/ngoài lock:** thứ tự lấy lock quyết định đúng/sai. Lấy tail ngoài `head_mutex` → head có thể vượt tail → hỏng list. Đây là loại bug fine-grained locking điển hình.
- **Fine-grained thắng ở đâu:** cấp phát/delete node **ngoài lock**, lock chỉ giữ quanh vài phép gán con trỏ → `push` và `try_pop` gần như hoàn toàn song song.

---

## Cụm 4 — thread-safe lookup table: hash + shared_mutex per bucket (tr. 194–199)

### 4.1 Vì sao chọn hash table, không phải tree/array (tr. 195–196)

Lookup table (map/dictionary) có **pattern khác stack/queue**: **ít khi sửa, chủ yếu đọc** (như DNS cache ch. 3). Bỏ **iterator** (khó thread-safe). Bốn thao tác cơ bản: add, change, remove, get (kèm "nếu có"). Gộp add+change thành `add_or_update_mapping`. `get_value` trả **default value** hoặc `pair<value,bool>` hoặc smart pointer NULL để biểu diễn "không có".

**Ba cách cài associative container (tr. 196):**
| | Concurrency | Lý do |
|---|---|---|
| **Binary tree** (red-black) | Kém | Mọi lookup/sửa bắt đầu từ **root** → phải khóa root → gần như một lock cho cả cây |
| **Sorted array** | Tệ nhất | Không biết trước vị trí → một lock cho cả mảng |
| **Hash table** | **Tốt** | Với số bucket cố định, bucket của key là thuộc tính của key+hash → **mỗi bucket một lock riêng** |

→ Hash table + `shared_mutex` mỗi bucket = tăng concurrency **N lần** (N = số bucket).

### 4.2 Cài đặt (Listing 6.11, tr. 196–198)

```cpp
template<typename Key, typename Value, typename Hash = std::hash<Key>>
class threadsafe_lookup_table {
    class bucket_type {
        typedef std::pair<Key, Value> bucket_value;
        typedef std::list<bucket_value> bucket_data;
        typedef typename bucket_data::iterator bucket_iterator;
        bucket_data data;
        mutable std::shared_mutex mutex;         // mỗi bucket một shared_mutex

        bucket_iterator find_entry_for(Key const& key) const {
            return std::find_if(data.begin(), data.end(),
                [&](bucket_value const& item){ return item.first == key; });
        }
    public:
        Value value_for(Key const& key, Value const& default_value) const {
            std::shared_lock<std::shared_mutex> lock(mutex);       // ĐỌC: shared → nhiều reader
            auto const found = find_entry_for(key);
            return (found == data.end()) ? default_value : found->second;
        }
        void add_or_update_mapping(Key const& key, Value const& value) {
            std::unique_lock<std::shared_mutex> lock(mutex);       // GHI: unique → độc quyền
            auto const found = find_entry_for(key);
            if (found == data.end()) data.push_back(bucket_value(key, value));
            else found->second = value;
        }
        void remove_mapping(Key const& key) {
            std::unique_lock<std::shared_mutex> lock(mutex);
            auto const found = find_entry_for(key);
            if (found != data.end()) data.erase(found);
        }
    };

    std::vector<std::unique_ptr<bucket_type>> buckets;
    Hash hasher;
    bucket_type& get_bucket(Key const& key) const {
        std::size_t const bucket_index = hasher(key) % buckets.size();
        return *buckets[bucket_index];                             // KHÔNG cần lock: số bucket cố định
    }
public:
    threadsafe_lookup_table(unsigned num_buckets = 19, Hash const& h = Hash())
        : buckets(num_buckets), hasher(h) {
        for (unsigned i = 0; i < num_buckets; ++i) buckets[i].reset(new bucket_type);
    }
    threadsafe_lookup_table(threadsafe_lookup_table const&) = delete;
    Value value_for(Key const& key, Value const& default_value = Value()) const {
        return get_bucket(key).value_for(key, default_value);
    }
    void add_or_update_mapping(Key const& key, Value const& value) {
        get_bucket(key).add_or_update_mapping(key, value);
    }
    void remove_mapping(Key const& key) { get_bucket(key).remove_mapping(key); }
};
```
Điểm thiết kế (tr. 198): **số bucket cố định (mặc định 19 — số nguyên tố)** → `get_bucket()` gọi **không cần lock**; rồi bucket mutex khóa **shared** (đọc) hoặc **unique** (ghi). Exception: `value_for` không sửa gì; `remove_mapping` dùng `erase` (không ném); `add_or_update` — `push_back` exception-safe, còn nhánh gán để user chịu.

### 4.3 Snapshot toàn bộ — khóa mọi bucket theo thứ tự (Listing 6.12, tr. 199)

`get_map()` cần view nhất quán → khóa **tất cả** bucket. **⚠️ Chống deadlock: luôn khóa theo CÙNG thứ tự** (tăng dần index) → không deadlock. Đây là thao tác duy nhất khóa mọi bucket.
```cpp
std::map<Key, Value> get_map() const {
    std::vector<std::unique_lock<std::shared_mutex>> locks;
    for (unsigned i = 0; i < buckets.size(); ++i)                 // khóa theo thứ tự tăng dần index
        locks.push_back(std::unique_lock<std::shared_mutex>(buckets[i].mutex));
    std::map<Key, Value> res;
    for (unsigned i = 0; i < buckets.size(); ++i)
        for (auto it = buckets[i].data.begin(); it != buckets[i].data.end(); ++it)
            res.insert(*it);
    return res;
}
```

### Insight đáng nhớ (Cụm 4)
- **Hash table là lựa chọn tự nhiên cho fine-grained lookup:** bucket độc lập → **một lock mỗi bucket**, concurrency tăng N lần. Tree/array không tách được vì mọi truy cập qua một điểm chung.
- **Số bucket cố định → `get_bucket` khỏi khóa.** Đọc-nhiều-ghi-hiếm → `shared_mutex` mỗi bucket (nhiều reader + writer độc quyền trên từng bucket).
- **Snapshot toàn cục = khóa mọi bucket theo thứ tự cố định** → chống deadlock. Đây là ngoại lệ duy nhất phải giữ nhiều lock.

---

## Cụm 5 — thread-safe list: mutex per node + hand-over-hand locking (tr. 199–203)

### 5.1 Bài toán iterator & giải pháp `for_each` nội bộ (tr. 199–200)

**⚠️ Iterator kiểu STL rất khó thread-safe (tr. 199–200):** iterator giữ reference vào cấu trúc nội bộ; nếu thread khác sửa/xóa element iterator đang trỏ → hỏng. Vòng đời iterator **ngoài kiểm soát** của container → ý tồi. **Giải pháp:** cung cấp **hàm lặp `for_each` là một phần của container** — container tự lo lock. Giá: `for_each` phải **gọi user code khi giữ lock nội bộ** (vi phạm nhẹ guideline ch. 3) → để user tự chịu trách nhiệm không gây deadlock/không lưu reference ra ngoài.

### 5.2 Hand-over-hand locking: mutex mỗi node (Listing 6.13, tr. 200–203)

**Ý tưởng (tr. 200):** **một mutex mỗi node**. Thao tác trên các phần khác nhau của list **thật sự song song** — mỗi thao tác chỉ giữ lock trên node nó quan tâm, **nhả node cũ khi chuyển sang node kế** (hand-over-hand = "chuyền tay").

```cpp
template<typename T>
class threadsafe_list {
    struct node {
        std::mutex m;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
        node() : next() {}                                    // head node (dummy)
        node(T const& value) : data(std::make_shared<T>(value)) {}
    };
    node head;
public:
    void push_front(T const& value) {
        std::unique_ptr<node> new_node(new node(value));      // cấp phát NGOÀI lock
        std::lock_guard<std::mutex> lk(head.m);               // chỉ khóa head → không deadlock
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template<typename Function>
    void for_each(Function f) {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);              // (i) khóa head
        while (node* const next = current->next.get()) {
            std::unique_lock<std::mutex> next_lk(next->m);    // (1) khóa node KẾ...
            lk.unlock();                                      // (2) ...RỒI nhả node hiện tại
            f(*next->data);                                   // (3) gọi user code khi giữ lock node kế
            current = next;
            lk = std::move(next_lk);                          // (4) chuyền lock sang node kế
        }
    }

    template<typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p) {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while (node* const next = current->next.get()) {
            std::unique_lock<std::mutex> next_lk(next->m);
            lk.unlock();
            if (p(*next->data)) return next->data;            // khớp → trả ngay (không lặp tiếp)
            current = next;
            lk = std::move(next_lk);
        }
        return std::shared_ptr<T>();
    }

    template<typename Predicate>
    void remove_if(Predicate p) {
        node* current = &head;
        std::unique_lock<std::mutex> lk(head.m);
        while (node* const next = current->next.get()) {
            std::unique_lock<std::mutex> next_lk(next->m);
            if (p(*next->data)) {
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);        // gỡ node khỏi list
                next_lk.unlock();                             // nhả TRƯỚC khi old_next hủy (bên dưới)
            } else {                                          // → node bị delete khi old_next hết scope
                lk.unlock();
                current = next;
                lk = std::move(next_lk);
            }
        }
    }
};
```

**Cơ chế hand-over-hand trong `for_each` (tr. 202):** khóa head (i) → lấy con trỏ next → khóa node kế (1) → **nhả node trước** (2) → gọi user function (3, khi vẫn giữ lock node đang xử lý) → chuyền lock (4). Vì **luôn khóa next TRƯỚC khi nhả current**, reference/node không bị hỏng giữa chừng.

**Phân tích deadlock/race (tr. 203):** *"quite definitely no"* nếu predicate/function ngoan. Lý do: **iteration luôn một chiều, luôn bắt đầu từ head, luôn khóa next trước khi nhả current** → **không có thứ tự lock khác nhau giữa các thread** → không deadlock. Điểm race duy nhất là delete node trong `remove_if` (làm sau khi unlock `next_lk`) — nhưng an toàn vì **vẫn giữ lock node `current` trước đó** → không thread nào lấy được lock node đang xóa.

**⚠️ Hạn chế (tr. 203):** vì mỗi node phải khóa lần lượt, **thread không vượt được nhau** — một thread xử lý lâu ở một node thì thread khác **kẹt** khi tới node đó.

### Insight đáng nhớ (Cụm 5)
- **Hand-over-hand locking = "luôn khóa node kế trước khi nhả node hiện tại".** Cho phép nhiều thread làm việc trên các node khác nhau đồng thời, nhưng **cùng một chiều duyệt** → không deadlock (thứ tự lock nhất quán).
- **Iterator STL rất khó thread-safe → thay bằng `for_each`/`find_first_if`/`remove_if` nội bộ.** Container tự lo lock; user code chạy khi giữ lock node → user chịu trách nhiệm không deadlock.
- **⚠️ Nhược cố hữu:** thread không vượt nhau — node "chậm" chặn mọi thread sau. Fine-grained per-node đổi contention toàn cục lấy contention cục bộ theo node.

---

## Tóm tắt chương (theo sách, tr. 203–204)

Chương này bắt đầu bằng ý nghĩa của "thiết kế cấu trúc dữ liệu cho concurrency" + guideline, rồi lần lượt qua **stack, queue, hash map, linked list** — áp dụng guideline để cài đặt cho truy cập concurrent, dùng lock bảo vệ và tránh data race. Giờ bạn có thể nhìn thiết kế của mình để tìm **cơ hội concurrency** và **nguy cơ race**.

**Câu chuyển sang ch. 7 (tr. 204):** tiếp theo là cách **tránh lock hoàn toàn** — dùng atomic (ch. 5) tạo ràng buộc ordering cần thiết, vẫn theo cùng bộ guideline.

**🆕 Bảng chốt — tiến trình coarse → fine-grained trong chương:**
| Cấu trúc | Khóa | Concurrency | Kỹ thuật then chốt |
|---|---|---|---|
| stack (6.1) | 1 mutex | Serialize hết | gộp top+pop, exception qua thứ tự |
| queue (6.2–6.3) | 1 mutex + condvar | Serialize hết | `wait_and_pop`, lưu `shared_ptr`, cấp phát ngoài lock |
| queue fine (6.6–6.10) | 2 mutex (head/tail) | push ∥ pop | **dummy node**, `get_tail` trong `head_mutex` |
| lookup table (6.11) | shared_mutex/bucket | N bucket ∥ | **hash → bucket độc lập**, số bucket cố định |
| list (6.13) | mutex/node | node ∥ node | **hand-over-hand locking** |

---

## Góc interview

**Câu 1 (🎯 câu thiết kế hay gặp):** Thiết kế một **thread-safe queue** cho phép thread **chờ** khi rỗng. Phân tích an toàn (race, exception, deadlock).

<details><summary>Đáp án</summary>

**Cài đặt coarse-grained + condition variable (Listing 6.2/6.3):**
```cpp
template<typename T>
class threadsafe_queue {
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T>> data_queue;   // lưu shared_ptr: copy-out không ném + cấp phát ngoài lock
    std::condition_variable data_cond;
public:
    void push(T new_value) {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));  // cấp phát NGOÀI lock
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(data);
        data_cond.notify_one();
    }
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this]{ return !data_queue.empty(); });   // chờ không đốt CPU
        value = std::move(*data_queue.front());
        data_queue.pop();
    }
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty()) return false;
        value = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
};
```

**Phân tích 4 tiêu chí (mẫu tư duy của chương):**
1. **Race-interface:** gộp `front`+`pop`, kiểm `empty` trong lock. `wait_and_pop` không trả về tới khi có phần tử → không thấy queue rỗng.
2. **Exception:** lưu `std::shared_ptr` → copy ra không ném; `make_shared` cấp phát **ngoài lock** trong `push`. (Nếu lưu `T` trực tiếp: nếu `wait_and_pop` ném khi dựng `shared_ptr` mà đã `notify_one`, thread khác không được đánh thức → item kẹt; sửa bằng lưu `shared_ptr` như trên.)
3. **Deadlock:** một mutex, không nested → không deadlock nội bộ. `push` unlock trước `notify_one` (giảm chờ lock cho thread thức dậy).
4. **`mutable mutex`** để `empty() const` lock được.

**Điểm cộng:** nêu `notify_one` vs `notify_all` (one khi một-consumer-đủ, all khi mọi-thread-cần), và giải thích `wait` cần `unique_lock` + predicate (chống spurious/stolen wakeup). Nếu hỏi sâu hơn về hiệu năng → chuyển sang **fine-grained (head/tail mutex + dummy node)** ở Câu 2.

</details>

**Câu 2 (🎯 nâng cao):** Một `threadsafe_queue` với một mutex serialize hết. Làm sao cho **`push` và `pop` chạy song song**? Cạm bẫy chính?

<details><summary>Đáp án</summary>

**Vấn đề gốc (tr. 184–185):** singly-linked-list ngây thơ — `push` sửa `tail->next`, `pop` đọc `head->next`; khi queue **một item** thì `head == tail` → **cùng một node** → phải khóa cùng mutex → vô nghĩa.

**Lời giải: dummy node (Listing 6.5–6.6):** preallocate một **dummy node không dữ liệu** để luôn có ≥1 node → tách node ở head khỏi node ở tail. Có ≥1 item thật → head và tail trỏ **node khác nhau** → dùng **hai mutex**: `head_mutex`, `tail_mutex`.
```cpp
struct node { std::shared_ptr<T> data; std::unique_ptr<node> next; };
std::mutex head_mutex; std::unique_ptr<node> head;
std::mutex tail_mutex; node* tail;

threadsafe_queue() : head(new node), tail(head.get()) {}  // dummy node

void push(T new_value) {
    auto new_data = std::make_shared<T>(std::move(new_value));  // cấp phát NGOÀI lock
    std::unique_ptr<node> p(new node);
    node* const new_tail = p.get();
    std::lock_guard<std::mutex> tail_lock(tail_mutex);   // chỉ khóa tail
    tail->data = new_data;
    tail->next = std::move(p);
    tail = new_tail;
}
node* get_tail() {
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}
std::unique_ptr<node> pop_head() {
    std::lock_guard<std::mutex> head_lock(head_mutex);
    if (head.get() == get_tail()) return nullptr;        // get_tail() TRONG head_mutex!
    auto old_head = std::move(head);
    head = std::move(old_head->next);
    return old_head;
}
```

**Ba cạm bẫy phải nêu:**
1. **⚠️ `get_tail()` phải ở TRONG `head_mutex` (tr. 188–189):** nếu lấy tail ngoài lock rồi mới khóa head, giữa hai bước thread khác pop → `old_tail` không còn trong list → so sánh sai → **head vượt tail, off khỏi list, hỏng cấu trúc**.
2. **⚠️ `tail_mutex` không chỉ bảo vệ `tail`, mà tạo defined ordering giữa `push` và `pop` (tr. 188):** `get_tail` khóa cùng mutex như `push` → hoặc thấy tail cũ hoặc thấy tail mới + data mới. Không có nó = data race (ch. 5).
3. **Thứ tự lock cố định:** `pop_head` luôn `head_mutex` **rồi** `tail_mutex` → không deadlock.

**Cái được (tr. 189–190):** cấp phát/delete node **ngoài lock**; lock chỉ giữ quanh vài phép gán con trỏ → `push` và `try_pop` **gần như hoàn toàn song song**. Thêm `wait_and_pop`: predicate `head != get_tail()` chỉ cần `head_mutex`; `wait_for_data()` trả cả lock để giữ nguyên khi sửa list.

</details>

**Câu 3 (🎯):** Thiết kế một **thread-safe hash map (lookup table)** với concurrency cao. Vì sao hash table tốt hơn tree/array?

<details><summary>Đáp án</summary>

**Vì sao hash table (tr. 196):**
| | Concurrency | Lý do |
|---|---|---|
| Binary tree | Kém | Mọi truy cập qua **root** → phải khóa root |
| Sorted array | Tệ nhất | Không biết trước vị trí → một lock cả mảng |
| **Hash table** | **Tốt** | Bucket của key là thuộc tính của key+hash → **một lock mỗi bucket** → concurrency ×N |

**Thiết kế (Listing 6.11):**
```cpp
template<typename Key, typename Value, typename Hash = std::hash<Key>>
class threadsafe_lookup_table {
    class bucket_type {
        std::list<std::pair<Key,Value>> data;
        mutable std::shared_mutex mutex;         // shared_mutex mỗi bucket
    public:
        Value value_for(Key const& k, Value const& def) const {
            std::shared_lock<std::shared_mutex> lock(mutex);   // đọc: nhiều reader
            /* find_if... */
        }
        void add_or_update_mapping(Key const& k, Value const& v) {
            std::unique_lock<std::shared_mutex> lock(mutex);   // ghi: độc quyền
            /* push_back hoặc gán... */
        }
    };
    std::vector<std::unique_ptr<bucket_type>> buckets;   // số bucket CỐ ĐỊNH (mặc định 19, nguyên tố)
    Hash hasher;
    bucket_type& get_bucket(Key const& k) const {
        return *buckets[hasher(k) % buckets.size()];     // KHÔNG cần lock (số bucket cố định)
    }
public:
    Value value_for(Key const& k, Value const& def = Value()) const {
        return get_bucket(k).value_for(k, def);
    }
    void add_or_update_mapping(Key const& k, Value const& v) {
        get_bucket(k).add_or_update_mapping(k, v);
    }
};
```

**Ba điểm ăn điểm:**
1. **Số bucket cố định** → `get_bucket()` **không cần lock** (chỉ đọc `buckets.size()` bất biến).
2. **`shared_mutex` mỗi bucket** — đọc-nhiều-ghi-hiếm (pattern điển hình của map): nhiều reader đồng thời + writer độc quyền, **trên từng bucket** → concurrency ×N.
3. **Bỏ iterator** (khó thread-safe); gộp add+change thành `add_or_update`; `get_value` trả default/`pair<value,bool>`/smart pointer NULL để biểu diễn "không có".

**⚠️ Snapshot toàn bộ (get_map):** khóa **mọi bucket theo thứ tự tăng dần index** → view nhất quán + chống deadlock. Đây là thao tác duy nhất giữ nhiều lock.

</details>

**Câu 4 (🟠):** Thiết kế thread-safe **linked list** cho nhiều thread duyệt/sửa đồng thời. Giải thích **hand-over-hand locking** và vì sao không deadlock.

<details><summary>Đáp án</summary>

**Ý tưởng: một mutex mỗi node + hand-over-hand locking (tr. 200):** thao tác trên các phần khác nhau của list thật sự song song; mỗi thao tác chỉ giữ lock node nó quan tâm, **nhả node cũ khi chuyển sang node kế**.

**Cốt lõi `for_each` (Listing 6.13):**
```cpp
struct node { std::mutex m; std::shared_ptr<T> data; std::unique_ptr<node> next; };
node head;

template<typename Function>
void for_each(Function f) {
    node* current = &head;
    std::unique_lock<std::mutex> lk(head.m);              // khóa head
    while (node* const next = current->next.get()) {
        std::unique_lock<std::mutex> next_lk(next->m);    // (1) khóa node KẾ trước...
        lk.unlock();                                      // (2) ...RỒI mới nhả node hiện tại
        f(*next->data);                                   // (3) gọi user code khi giữ lock node đang xử lý
        current = next;
        lk = std::move(next_lk);                          // (4) chuyền lock sang node kế
    }
}
```

**Vì sao KHÔNG deadlock (tr. 203):** iteration **luôn một chiều, luôn từ head, luôn khóa `next` TRƯỚC khi nhả `current`** → **mọi thread lấy lock theo cùng thứ tự** (từ head về cuối) → không thể có circular wait → không deadlock. (Đây chính là guideline "khóa theo thứ tự cố định" của ch. 3.)

**An toàn khi delete trong `remove_if`:** node bị gỡ được delete **sau khi** unlock lock của nó, nhưng **vẫn giữ lock node `current` trước đó** → không thread nào lấy được lock node đang xóa → an toàn.

**Điểm phải nêu:**
- **⚠️ Iterator STL bỏ đi** — thay bằng `for_each`/`find_first_if`/`remove_if` nội bộ (container tự lo lock). User code chạy khi giữ lock node → **user chịu trách nhiệm** không deadlock/không lưu reference ra ngoài.
- **⚠️ Nhược cố hữu:** thread **không vượt nhau** — một thread xử lý lâu ở một node chặn mọi thread sau tới node đó. Fine-grained per-node đổi contention toàn cục lấy contention cục bộ.
- **`push_front`** chỉ khóa `head.m` (cấp phát node ngoài lock) → không deadlock, giữ lock cực ngắn.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [03-sharing-data.md](03-sharing-data.md) — nền của chương này: `threadsafe_stack` gốc, race-trong-interface, deadlock guideline, `shared_mutex`, hand-over-hand đã nhắc ở §3.2.5.
- [04-synchronization.md](04-synchronization.md) — condition variable + `threadsafe_queue` gốc (Cụm 2 chương này mở rộng từ đó).
- [05-memory-model.md](05-memory-model.md) — vì sao thiếu `tail_mutex` trong `get_tail` = data race (defined ordering giữa push và pop).
- [cpp-mindset/data-structures.md](../cpp-mindset/data-structures.md) — hash table, linked list, cache locality — nền cấu trúc dữ liệu tuần tự.
- [03-operating-system/sync-primitives.md](../../03-operating-system/sync-primitives.md) — reader-writer lock, deadlock ở tầng OS.

**Chương tiếp theo:** [Ch. 7 — Designing lock-free concurrent data structures →](07-lock-free-structures.md) 🎯 (nonblocking vs lock-free vs wait-free; stack/queue không lock; **memory reclamation, hazard pointer, reference counting, ABA problem**) — quay lại atomic của ch. 5 để **bỏ lock hoàn toàn**; đây là chương lock-free mà repo chưa có nguồn nào khác phủ.
