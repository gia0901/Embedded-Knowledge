# Ch. 7 — Designing lock-free concurrent data structures (tr. 205–250) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 6 — Lock-based structures](06-lock-based-structures.md)** · **[Ch. 8 → Designing concurrent code](08-designing-concurrent-code.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **🎯 Chương lock-free — repo chưa có nguồn nào khác phủ.** Đây là chủ đề **phân loại ứng viên senior C++** cùng với ch. 5. Xây trực tiếp trên memory model (ch. 5) — sách nói thẳng (tr. 206): *"It is vital for the understanding of this chapter that you have read and understood all of chapter 5."* Chứa ba khái niệm không tài liệu nào khác trong repo có: **hazard pointers, split reference counting, ABA problem**.

**Sách nêu mục tiêu chương (tr. 205):** cài cấu trúc dữ liệu concurrent **không lock** · kỹ thuật **quản lý bộ nhớ** trong lock-free · guideline viết code lock-free.

**Cảnh báo của sách (tr. 206):** *"You need to take extreme care when designing these data structures, because they're hard to get right, and the conditions that cause the design to fail may occur very rarely."*

---

## Cụm 1 — Định nghĩa: blocking, nonblocking, lock-free, wait-free + pros/cons (tr. 206–210)

### 1.1 Blocking vs nonblocking (tr. 206)

- **Blocking (tr. 206):** dùng mutex/condition variable/future → thread bị **suspend** tới khi thread khác hành động (unlock, notify, làm future ready). OS gỡ thread bị block khỏi CPU.
- **Nonblocking (tr. 206):** không dùng hàm blocking. **⚠️ Nhưng nonblocking ≠ lock-free!**

**Spinlock là nonblocking NHƯNG KHÔNG lock-free (tr. 207):** `spinlock_mutex` (Listing 7.1, từ ch. 5) không gọi hàm blocking nào — nó *"spin"* trong vòng lặp `test_and_set`. Code dùng nó là nonblocking, nhưng **vẫn là mutex, vẫn chỉ một thread khóa được** → không lock-free.

### 1.2 Ba mức nonblocking (tr. 207)

> - **Obstruction-Free:** nếu **mọi thread khác dừng**, thì một thread bất kỳ hoàn thành thao tác trong **số bước hữu hạn**.
> - **Lock-Free:** nếu nhiều thread thao tác, thì sau **số bước hữu hạn**, **một trong số chúng** hoàn thành.
> - **Wait-Free:** **mọi thread** hoàn thành thao tác trong **số bước hữu hạn**, bất kể thread khác làm gì.

**Obstruction-free ít dùng (tr. 207):** hiếm khi mọi thread khác dừng → chủ yếu là dấu hiệu của một cài đặt lock-free **thất bại**.

### 1.3 Lock-free & wait-free chi tiết (tr. 207–208)

**Lock-free (tr. 207):** ≥2 thread truy cập đồng thời (không nhất thiết cùng thao tác — lock-free queue có thể cho 1 push + 1 pop nhưng hỏng nếu 2 push). **Nếu một thread bị scheduler suspend giữa chừng, các thread khác VẪN hoàn thành được** mà không phải chờ nó.

Thuật toán CAS thường **có vòng lặp** (thread khác sửa data → phải làm lại rồi CAS lại). Vẫn lock-free **nếu CAS cuối cùng sẽ thành công khi các thread khác bị suspend**. Nếu không → là spinlock (nonblocking nhưng không lock-free).

**⚠️ Lock-free có thể gây STARVATION (tr. 207):** nếu thread khác chạy với timing "xấu", một thread có thể **liên tục phải retry**. Cấu trúc tránh được điều này là **wait-free**.

**Wait-free = lock-free + mọi thread hoàn thành trong số bước hữu hạn (tr. 208).** **⚠️ Cực khó viết đúng:** mỗi thao tác phải làm **một lần (single pass)** và bước của thread này **không được làm thao tác thread khác fail**. Hầu hết ví dụ trong chương **KHÔNG wait-free** — chúng có `while` trên `compare_exchange` không giới hạn số lần lặp.

### 1.4 Pros & cons (tr. 208–210)

**Ưu điểm:**
1. **Concurrency tối đa (tr. 208):** lock-free — *"some thread makes progress with every step"*; wait-free — mọi thread tiến bất kể thread khác.
2. **Robustness (tr. 208):** **thread chết khi giữ lock → cấu trúc hỏng vĩnh viễn**; thread chết giữa chừng lock-free → **chỉ mất data của thread đó**, các thread khác tiếp tục bình thường.

**Nhược điểm:**
1. **⚠️ Không deadlock nhưng CÓ LIVE LOCK (tr. 209):** hai thread mỗi cái làm thay đổi buộc thread kia phải restart → cả hai lặp mãi. Ẩn dụ: hai người cùng chui qua khe hẹp, kẹt, lùi ra, thử lại. Thường ngắn (phụ thuộc timing) → **giảm hiệu năng** chứ không kẹt lâu dài. Wait-free không bị (có giới hạn số bước).
2. **⚠️ CÓ THỂ CHẬM HƠN (tr. 209–210):** atomic operations **chậm hơn nhiều** non-atomic (thường ~100× trên desktop), và lock-free có **nhiều atomic op hơn**. Phần cứng phải đồng bộ data giữa thread → **cache ping-pong** (ch. 8) là gánh nặng lớn. **→ Phải đo (profile) cả hai bản (lock-based và lock-free) trước khi quyết.**

### Insight đáng nhớ (Cụm 1)
- **nonblocking ⊋ lock-free ⊋ wait-free.** Spinlock là nonblocking nhưng không lock-free (vẫn mutual exclusion). Lock-free = "luôn có một thread tiến". Wait-free = "mọi thread tiến trong số bước hữu hạn".
- **Lý do thật để dùng lock-free: robustness + max concurrency, KHÔNG phải luôn nhanh hơn.** Atomic chậm + cache ping-pong → nhiều khi lock-based còn nhanh hơn. **Đo trước.**
- **Không deadlock nhưng có live lock + starvation.** Lock-free giải quyết deadlock, đổi lấy các vấn đề timing khác.

---

## Cụm 2 — Lock-free stack cơ bản: `push`/`pop` bằng CAS (tr. 210–213)

### 2.1 `push` — CAS trên head (Listing 7.2, tr. 210–211)

Stack = linked list, `head` là `std::atomic<node*>`. `push` ba bước: tạo node → set `next = head` → set `head = node`. **⚠️ Race giữa bước 2 và 3:** thread khác có thể đổi `head` sau khi ta đọc. Giải bằng **CAS ở bước 3**:

```cpp
template<typename T>
class lock_free_stack {
    struct node {
        T data;
        node* next;
        node(T const& data_) : data(data_) {}   // chuẩn bị data TRONG constructor
    };
    std::atomic<node*> head;
public:
    void push(T const& data) {
        node* const new_node = new node(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node));  // CAS: chỉ set nếu head chưa đổi
    }
};
```
**Điểm hay của CAS (tr. 211):** khi `compare_exchange_weak` **fail**, nó **tự cập nhật tham số đầu (`new_node->next`) thành giá trị hiện tại của `head`** → **không cần load lại `head`** mỗi vòng lặp. Dùng `weak` được vì đã lặp trực tiếp khi fail. **⚠️ Node phải sẵn sàng HOÀN TOÀN trước khi `head` trỏ tới** (thread khác đọc được ngay sau CAS) → chuẩn bị data trong constructor.

`push` **lock-free, an toàn:** chỗ duy nhất ném là `new node` (tự dọn, list chưa đổi); không race; không lock → không deadlock.

### 2.2 `pop` — bài toán khó (tr. 211–213)

`pop` năm bước: đọc `head` → đọc `head->next` → set `head = head->next` → lấy data → **delete node**. **⚠️ Hai vấn đề với nhiều thread:**
1. **Dangling pointer (bước 5):** hai thread đọc cùng `head`; thread 1 chạy hết tới delete trước khi thread 2 tới bước 2 → thread 2 **dereference con trỏ đã chết**. → **Đây là vấn đề lớn nhất của lock-free** — tạm thời **leak node (bỏ bước 5)**.
2. **Hai thread trả cùng node:** cùng đọc `head` → cùng trả một node. Giải bằng **CAS trên `head`** như `push`.

**Exception-safety (tr. 212–213):** không thể dùng "trả qua reference" như ch. 3 (chỉ copy an toàn được **sau khi** đã lấy node khỏi list). → **Trả `std::shared_ptr<T>`** (dựng khi `push`, copy ra không ném):

```cpp
struct node {
    std::shared_ptr<T> data;                          // lưu bằng pointer
    node* next;
    node(T const& data_) : data(std::make_shared<T>(data_)) {}
};
std::shared_ptr<T> pop() {
    node* old_head = head.load();
    while (old_head &&                                 // kiểm null TRƯỚC khi dereference
           !head.compare_exchange_weak(old_head, old_head->next));
    return old_head ? old_head->data : std::shared_ptr<T>();
}
```
**⚠️ Lock-free nhưng KHÔNG wait-free** (vòng `while` có thể lặp mãi nếu CAS liên tục fail), và **vẫn leak node** (chưa delete). Managed language (Java/C#) có GC lo; C++ phải tự dọn → ba kỹ thuật memory reclamation dưới đây.

### Insight đáng nhớ (Cụm 2)
- **CAS-loop là khung xương của lock-free.** Fail → CAS tự nạp giá trị mới → lặp lại. Node phải sẵn sàng hoàn toàn trước khi công bố (CAS `head`).
- **Vấn đề khó nhất KHÔNG phải logic mà là RECLAMATION:** khi nào an toàn `delete` một node mà thread khác có thể còn con trỏ tới? Toàn bộ phần còn lại của chương là ba câu trả lời cho câu hỏi này.

---

## Cụm 3 — Reclamation #1: đếm thread trong `pop()` (tr. 214–218)

### 3.1 Ý tưởng: delete khi không còn thread nào trong `pop` (tr. 214)

**Chỉ thread trong `pop` mới đụng node** (push chỉ chạm node của mình tới khi vào list). Nếu **chỉ một thread `pop`** → nó là thread duy nhất chạm node → delete an toàn. Nhiều thread `pop` → cần **đếm**: node vừa lấy ra đưa vào list **"to_be_deleted"**; đếm `threads_in_pop`; khi về 0 → an toàn xóa cả list.

```cpp
std::atomic<unsigned> threads_in_pop;
std::atomic<node*> to_be_deleted;

std::shared_ptr<T> pop() {
    ++threads_in_pop;                        // đếm TRƯỚC khi làm gì
    node* old_head = head.load();
    while (old_head &&
           !head.compare_exchange_weak(old_head, old_head->next));
    std::shared_ptr<T> res;
    if (old_head) res.swap(old_head->data);  // swap ra (delete node sau) thay vì copy pointer
    try_reclaim(old_head);                   // giảm counter + thử xóa
    return res;
}
void try_reclaim(node* old_head) {
    if (threads_in_pop == 1) {               // ta là thread DUY NHẤT trong pop?
        node* nodes_to_delete = to_be_deleted.exchange(nullptr);  // "giành" list cho mình
        if (!--threads_in_pop)               // giảm counter, vẫn là 0?
            delete_nodes(nodes_to_delete);   // → an toàn xóa cả list
        else if (nodes_to_delete)
            chain_pending_nodes(nodes_to_delete);  // có thread khác vào → xâu lại
        delete old_head;                     // node của ta: an toàn xóa
    } else {                                 // có thread khác trong pop
        chain_pending_node(old_head);        // → hoãn xóa, đưa vào to_be_deleted
        --threads_in_pop;
    }
}
```

### 3.2 ⚠️ Vì sao phải kiểm `threads_in_pop` LẠI sau khi giành list (Figure 7.1, tr. 216–218)

Điểm tinh vi nhất: giữa lần kiểm `threads_in_pop == 1` đầu và lúc `exchange(nullptr)` giành list, **thread khác có thể vào `pop`** và thêm node vào list mà nó **vẫn đang tham chiếu** (như `old_head` của nó). Nếu xóa mù → UB cho thread đó. Vì thế phải **giảm counter rồi kiểm lại về 0** trước khi thật sự xóa.

**⚠️ Hạn chế chí mạng (tr. 218):** hoạt động tốt khi **tải thấp** (có "quiescent point" — lúc không thread nào trong `pop`). **Tải cao → không bao giờ có quiescent point** (thread mới vào `pop` trước khi thread cũ ra hết) → `to_be_deleted` **phình vô hạn = leak lại**. → Cần cơ chế khác: **hazard pointers**.

### Insight đáng nhớ (Cụm 3)
- **"Đếm thread trong vùng nguy hiểm" chỉ an toàn tại quiescent point.** Tải cao không có quiescent point → leak. Đây là bài học: reclamation phải **theo từng node**, không theo "toàn cục rảnh".
- **`swap` data ra khỏi node thay vì copy pointer** → data được giải phóng ngay khi không cần, không bị giữ sống bởi node chưa xóa.

---

## Cụm 4 — 🎯 Reclamation #2: Hazard Pointers (tr. 218–226)

### 4.1 Ý tưởng (Maged Michael, tr. 218–219)

**Hazard pointer = "con trỏ nguy hiểm":** xóa một node mà thread khác có thể còn tham chiếu là **nguy hiểm**. Ý tưởng: trước khi truy cập object mà thread khác có thể muốn xóa, thread **đặt hazard pointer** trỏ tới object → báo "xóa cái này lúc này là nguy hiểm". Xong thì **clear**. Khi muốn xóa: **kiểm hazard pointer của MỌI thread** — nếu không cái nào trỏ tới object → xóa an toàn; nếu có → **hoãn lại**, kiểm định kỳ.

🆕 Ẩn dụ của sách: đua thuyền Oxford/Cambridge — cox giơ tay báo "chưa sẵn sàng"; trọng tài không được xuất phát khi còn tay giơ.

### 4.2 Đặt hazard pointer đúng cách trong `pop` (tr. 219–221)

```cpp
std::shared_ptr<T> pop() {
    std::atomic<void*>& hp = get_hazard_pointer_for_current_thread();
    node* old_head = head.load();
    do {
        node* temp;
        do {                                 // (★) vòng lặp bảo đảm hp đặt ĐÚNG head hiện tại
            temp = old_head;
            hp.store(old_head);              // đặt hazard pointer
            old_head = head.load();          // đọc lại head
        } while (old_head != temp);          // lặp tới khi head KHÔNG đổi giữa 2 lần đọc
    } while (old_head &&
             !head.compare_exchange_strong(old_head, old_head->next));  // strong (đang làm việc trong loop)
    hp.store(nullptr);                       // clear hazard pointer khi xong
    std::shared_ptr<T> res;
    if (old_head) {
        res.swap(old_head->data);
        if (outstanding_hazard_pointers_for(old_head))   // thread khác còn trỏ tới?
            reclaim_later(old_head);         // → hoãn xóa
        else
            delete old_head;                 // → xóa ngay
        delete_nodes_with_no_hazards();      // quét list hoãn, xóa cái nào hết hazard
    }
    return res;
}
```
**⚠️ Vì sao vòng `while(old_head != temp)` (★) (tr. 219):** giữa lúc đọc `head` (bước 1) và đặt hazard pointer (bước 2), node có thể **bị xóa**. Đặt hp xong phải **đọc lại `head`** và kiểm nó vẫn bằng giá trị ta vừa đặt hp — nếu đổi thì node cũ có thể đã bị xóa, phải làm lại. Dùng `compare_exchange_strong` (không `weak`) vì fail giả sẽ reset hp không cần thiết.

### 4.3 Cấp phát hazard pointer per-thread (Listing 7.7, tr. 221–222)

Mảng cố định `hazard_pointers[max_hazard_pointers]` (thread ID + pointer). `get_hazard_pointer_for_current_thread()` dùng biến `thread_local static hp_owner` — lần đầu mỗi thread gọi, constructor `hp_owner` **CAS để giành một slot trống**; destructor (khi thread thoát) trả slot. `outstanding_hazard_pointers_for(p)` **quét mảng** tìm entry trỏ `p`.

### 4.4 Reclaim generic (Listing 7.8, tr. 222–224)

`reclaim_later` là **template** (hazard pointer là tiện ích tổng quát, không buộc vào stack node). `data_to_reclaim` lưu `void* data` + `std::function<void(void*)> deleter` (con trỏ tới `do_delete<T>` cast đúng kiểu rồi delete). `delete_nodes_with_no_hazards` **giành cả list bằng `exchange`**, rồi với mỗi node: hết hazard → xóa; còn hazard → xâu lại.

### 4.5 ⚠️ Cái giá & cải tiến (tr. 224–226)

**Chậm (tr. 224–225):** mỗi `pop` quét `max_hazard_pointers` biến atomic (atomic ~100× chậm hơn non-atomic), và quét cho **mỗi node trong list hoãn** → O(max² ) mỗi lần. **Cải tiến:** (a) chỉ reclaim khi list có **> 2×max_hazard_pointers** node → bảo đảm xóa được ≥ max node; (b) mỗi thread giữ **list reclaim thread-local riêng** (khỏi atomic) — đổi bộ nhớ lấy tốc độ.

**⚠️ Vấn đề pháp lý (tr. 225–226):** hazard pointer **bị IBM đăng ký patent** (nay có lẽ đã hết hạn — kiểm tra luật sư nếu ở nước patent còn hiệu lực). Có đề xuất đưa vào C++ Standard tương lai (P0566). 🆕 **C++26 dự kiến chuẩn hóa `std::hazard_pointer`** — đến lúc đó dùng bản chuẩn.

### Insight đáng nhớ (Cụm 4)
- **Hazard pointer = "tôi đang dùng node này, đừng xóa".** Reader công bố con trỏ đang dùng ra chỗ mọi thread thấy; deleter kiểm mọi hazard pointer trước khi xóa. **Reclaim theo từng node** → giải được bài toán tải cao của Cụm 3.
- **⚠️ Vòng lặp đọc-lại-head sau khi đặt hp** là mấu chốt đúng đắn: node có thể bị xóa giữa lúc đọc head và đặt hp.
- **Đắt (quét mảng atomic mỗi pop) và vướng patent.** Đây là lý do reference counting (Cụm 5) thường được ưa hơn.

---

## Cụm 5 — 🎯 Reclamation #3: Reference Counting + tinh chỉnh memory order (tr. 226–236)

### 5.1 Dùng lock-free `shared_ptr` nếu có (Listing 7.9/7.10, tr. 226–228)

Nếu `std::atomic_is_lock_free(&some_shared_ptr)` trả `true` → **vấn đề reclamation biến mất**: dùng `std::shared_ptr<node>` cho list, dùng `std::atomic_load`/`std::atomic_compare_exchange_weak` free functions. **⚠️ Nhưng hiếm platform có lock-free `shared_ptr`.** Concurrency TS có `std::experimental::atomic_shared_ptr<T>` (đơn giản hơn, khỏi nhớ `atomic_load`/`store`) — 🆕 **C++20 có `std::atomic<std::shared_ptr<T>>`** (bản chuẩn của ý này).

### 5.2 Split reference counting — khi không có lock-free shared_ptr (tr. 228–232)

**Ý tưởng (tr. 228–229): HAI counter mỗi node — external + internal.** Tổng = số reference thật.
- **external_count** đi kèm **con trỏ tới node** (trong `counted_node_ptr`), **+1 mỗi lần đọc con trỏ**.
- Khi reader xong với node → **giảm internal_count**.
- Khi cặp external/pointer không cần nữa → **internal += (external − 1)**, bỏ external. Khi **internal == 0** → không còn reference → xóa an toàn.

```cpp
struct counted_node_ptr {
    int external_count;
    node* ptr;
};
struct node {
    std::shared_ptr<T> data;
    std::atomic<int> internal_count;
    counted_node_ptr next;
    node(T const& data_) : data(std::make_shared<T>(data_)), internal_count(0) {}
};
std::atomic<counted_node_ptr> head;

void push(T const& data) {
    counted_node_ptr new_node;
    new_node.ptr = new node(data);
    new_node.external_count = 1;              // node mới: 1 reference (chính head)
    new_node.ptr->next = head.load();
    while (!head.compare_exchange_weak(new_node.ptr->next, new_node));
}
```
**⚠️ Điều kiện lock-free (tr. 230):** `counted_node_ptr` phải đủ nhỏ để `std::atomic<counted_node_ptr>` lock-free — cần platform hỗ trợ **double-word-compare-and-swap (DWCAS)**. Nếu không → `std::atomic` dùng **mutex nội bộ** → "lock-free" thành lock-based! (Mẹo: nhét counter vào **spare bits của con trỏ** nếu address space < 64-bit.)

### 5.3 `pop` với split ref counting (Listing 7.12, tr. 230–232)

```cpp
void increase_head_count(counted_node_ptr& old_counter) {
    counted_node_ptr new_counter;
    do {
        new_counter = old_counter;
        ++new_counter.external_count;        // +1 external TRƯỚC khi dereference
    } while (!head.compare_exchange_strong(old_counter, new_counter));
    old_counter.external_count = new_counter.external_count;
}
std::shared_ptr<T> pop() {
    counted_node_ptr old_head = head.load();
    for (;;) {
        increase_head_count(old_head);       // (1) tăng external → an toàn dereference
        node* const ptr = old_head.ptr;
        if (!ptr) return std::shared_ptr<T>();          // list rỗng
        if (head.compare_exchange_strong(old_head, ptr->next)) {   // (2) giành node
            std::shared_ptr<T> res;
            res.swap(ptr->data);
            int const count_increase = old_head.external_count - 2;  // (3) −2: bỏ khỏi list −1, ta hết dùng −1
            if (ptr->internal_count.fetch_add(count_increase) == -count_increase)
                delete ptr;                  // internal về 0 → xóa
            return res;
        } else if (ptr->internal_count.fetch_sub(1) == 1) {  // (4) CAS fail → giảm ref; nếu ta là cuối
            delete ptr;                      // → xóa
        }
    }
}
```
**Vì sao tăng external TRƯỚC khi dereference (tr. 231):** nếu dereference con trỏ trước khi tăng ref count, thread khác có thể **free node trước khi ta truy cập** → dangling. Tăng external count đảm bảo con trỏ **hợp lệ suốt thời gian truy cập** — **đây là lý do chính của split reference count**.
**Vì sao `−2` (tr. 231):** `old_head.external_count` − 1 (đã bỏ node khỏi list) − 1 (thread này hết dùng).

### 5.4 Áp memory model để nới lỏng ordering (Listing 7.13, tr. 232–236)

Sau khi logic đúng với `seq_cst`, sách **phân tích từng thao tác** để nới lỏng (tr. 232–234):
- **`push`:** CAS thành công cần **`memory_order_release`** (để tạo happens-before với `pop` đọc `next` non-atomic); fail → **`relaxed`**.
- **`increase_head_count`:** CAS thành công cần **`memory_order_acquire`** (synchronize-with release của push); fail → **`relaxed`**.
- **CAS set `head = ptr->next` trong pop:** **`relaxed`** đủ (đã có happens-before từ acquire ở `increase_head_count`).
- **`fetch_add` counter thành công-branch:** **`memory_order_release`**; **fail-branch `fetch_sub`:** **`relaxed`**, nhưng khi giảm về 0 phải `load(memory_order_acquire)` trước `delete` (bảo đảm `swap` happens-before `delete`).

**Kết quả (tr. 236):** `pop` giờ **37 dòng** (so với 8 dòng lock-based ch. 6, 7 dòng lock-free chưa quản bộ nhớ). Câu chốt: *"lots of the complexity in lock-free code comes from managing memory."*

### Insight đáng nhớ (Cụm 5)
- **Split reference counting: tăng external count TRƯỚC khi dereference** → con trỏ được "ghim" hợp lệ suốt thời gian truy cập. External đi với con trỏ, internal ở trong node; tổng về 0 → xóa.
- **⚠️ Cần DWCAS để lock-free thật.** Không có → `atomic<counted_node_ptr>` dùng mutex nội bộ → "lock-free" giả. Luôn kiểm `is_lock_free()`.
- **Nới lỏng memory order là bước TỐI ƯU cuối cùng, sau khi logic đúng với seq_cst.** Đầu tư nhiều dòng code đổi lấy hiệu năng — cân nhắc có đáng không.

---

## Cụm 6 — Lock-free queue + kỹ thuật "helping" (tr. 236–247)

### 6.1 SPSC queue dễ, MPMC khó (tr. 236–238)

Queue: `head` và `tail` atomic. **Single-producer single-consumer (SPSC) đơn giản** (Listing 7.14) — happens-before qua `tail` store/load là đủ. **⚠️ Nhiều thread `push` đồng thời = data race:** hai thread đọc cùng `tail`, cùng ghi data/next của **cùng node**. Nhiều `pop` = cùng vấn đề như stack `pop` (dùng lại giải pháp ref counting).

### 6.2 Vấn đề `push`: phải set data TRƯỚC khi update tail (tr. 238)

Để có happens-before, `push` phải set data trên dummy node **trước** khi update `tail` → nhưng concurrent `push` đọc cùng `tail` → race trên data. **Hai giải pháp (tr. 238):** (a) thêm dummy node giữa các node thật, chỉ cần `next` atomic; (b) làm **data pointer atomic**, set bằng CAS — thread thắng CAS mới là chủ node.

### 6.3 🎯 "Helping" — biến busy-wait thành wait-free hơn (tr. 240–247)

Vấn đề còn lại: một thread `push` phải **chờ** thread `push` khác hoàn thành → **busy-wait loop = thực chất blocking** (tr. 249). **Giải pháp "helping" (tr. 240–247):** thread đang chờ **tự làm nốt các bước dang dở** của thread kia nếu nó được schedule trước. Cần đổi data member thành atomic + dùng CAS để set. Đây là kỹ thuật làm lock-free queue **thật sự nonblocking** thay vì busy-wait.

### Insight đáng nhớ (Cụm 6)
- **SPSC queue dễ hơn MPMC nhiều.** Nếu bài toán chỉ 1 producer + 1 consumer, đừng phí công viết MPMC lock-free.
- **Busy-wait loop = thực chất blocking → thà dùng mutex.** "Helping" (thread chờ tự làm nốt việc thread kia) biến busy-wait thành nonblocking thật — nhưng cực phức tạp.

---

## Cụm 7 — 🎯 Guideline viết lock-free (tr. 247–250)

### 7.1 Dùng `seq_cst` để prototype (tr. 247–248)
`seq_cst` dễ suy luận nhất (total order). **Bắt đầu với `seq_cst`, chỉ nới lỏng SAU khi logic cơ bản chạy đúng.** Nới lỏng là **tối ưu hóa** → đừng làm sớm. **⚠️ Test chạy đúng KHÔNG đảm bảo đúng** — cần algorithm checker duyệt mọi tổ hợp thread visibility.

### 7.2 Dùng scheme reclaim bộ nhớ lock-free (tr. 248–249)
Ba kỹ thuật đã học: (1) **đợi không thread nào truy cập** rồi xóa hết pending; (2) **hazard pointers**; (3) **reference counting**. Ý chung: **theo dõi bao nhiêu thread đang truy cập một object, chỉ xóa khi không còn reference**. Khác: **garbage collector** (lý tưởng), hoặc **recycle node** (không free tới khi hủy cấu trúc — nhưng làm ABA problem nặng hơn).

### 7.3 ⚠️ Coi chừng ABA PROBLEM (tr. 249) — 🎯 hay hỏi

**ABA problem** — nguy cơ của **mọi thuật toán dựa trên CAS:**
```
1. Thread 1 đọc biến atomic x, thấy giá trị A.
2. Thread 1 làm gì đó dựa trên A (dereference nếu là con trỏ...).
3. Thread 1 bị OS stall.
4. Thread khác đổi x: A → B.
5. Thread khác đổi data gắn với A → giá trị A mà thread 1 giữ không còn hợp lệ
   (vd free vùng nhớ được trỏ tới).
6. Thread khác đổi x: B → A (nếu là con trỏ, có thể là object MỚI trùng địa chỉ cũ).
7. Thread 1 tỉnh dậy, CAS trên x so với A → THÀNH CÔNG (vì đúng là A),
   NHƯNG đây là "A sai". Data đọc ở bước 2 không còn hợp lệ → HỎNG cấu trúc.
```
**Cách tránh phổ biến nhất (tr. 249):** kèm một **ABA counter** cạnh biến `x`; CAS trên **cả cụm (x + counter)** như một đơn vị. Mỗi lần thay giá trị, counter++ → dù `x` trùng giá trị cũ, CAS **fail** vì counter đã đổi. **⚠️ ABA đặc biệt phổ biến ở thuật toán dùng free list / recycle node** (địa chỉ được tái sử dụng).

🆕 Đây chính là lý do các cấu trúc trong chương (dùng split ref count / hazard pointer, không recycle địa chỉ tùy tiện) **không bị ABA**.

### 7.4 Nhận diện busy-wait loop và "help" thread kia (tr. 249–250)
Busy-wait loop = thực chất **blocking** → thà dùng mutex. Sửa: cho thread chờ **tự làm nốt các bước dang dở** của thread kia → bỏ busy-wait, thao tác thành nonblocking. Trong queue: đổi data member thành atomic + CAS.

### Insight đáng nhớ (Cụm 7)
- **ABA problem là bẫy kinh điển của CAS:** giá trị quay về A không có nghĩa "không có gì xảy ra". Tránh bằng **tagged pointer / version counter** (CAS trên cụm value+counter). Đặc biệt nguy khi recycle địa chỉ node.
- **Quy trình: seq_cst prototype → logic đúng → reclamation scheme → nới lỏng memory order → thay busy-wait bằng helping.** Đây là checklist thiết kế lock-free.

---

## Tóm tắt chương (theo sách, tr. 250)

Chương này cài các lock-free structure (stack, queue), cho thấy phải cẩn thận với **memory ordering** để không data race, và **quản lý bộ nhớ khó hơn nhiều** so với lock-based — ba cơ chế reclaim (đếm thread / hazard pointer / reference counting). Cùng kỹ thuật **helping** để bỏ wait loop. Lock-free khó và dễ sai, nhưng có tính **scalability** quan trọng ở một số tình huống.

**Câu chuyển sang ch. 8 (tr. 250):** từ cấu trúc dữ liệu concurrent chuyển sang **code concurrent nói chung** — parallel algorithm dùng nhiều thread để tăng hiệu năng.

**🆕 Bảng chốt — ba kỹ thuật memory reclamation:**
| Kỹ thuật | Ý tưởng | Ưu | Nhược |
|---|---|---|---|
| **Đếm thread trong pop** | Xóa khi `threads_in_pop == 0` | Đơn giản | **Leak khi tải cao** (không quiescent point) |
| **Hazard pointers** | Mỗi thread công bố con trỏ đang dùng | Reclaim theo node, chịu tải cao | Chậm (quét mảng atomic), **vướng patent** |
| **Reference counting (split)** | external + internal count mỗi node | Không vướng patent, gọn về ý niệm | Cần **DWCAS**, code dài, ABA nếu recycle |

---

## Góc interview

**Câu 1 (🎯🎯 phân loại senior):** Phân biệt **lock-free**, **wait-free**, **obstruction-free**, và nonblocking. Spinlock thuộc loại nào? Lock-free có luôn nhanh hơn lock-based không?

<details><summary>Đáp án</summary>

**Bốn khái niệm (tr. 206–208):**
- **Nonblocking:** không dùng hàm blocking (không suspend thread). **Tập lớn nhất.**
- **Obstruction-free:** nếu **mọi thread khác dừng**, một thread hoàn thành trong số bước hữu hạn. (Ít dùng — chủ yếu mô tả lock-free thất bại.)
- **Lock-free:** nhiều thread thao tác → sau số bước hữu hạn, **một trong số chúng** hoàn thành. Thread bị suspend giữa chừng **không chặn** thread khác.
- **Wait-free:** **mọi** thread hoàn thành trong số bước hữu hạn, bất kể thread khác.

Quan hệ: **nonblocking ⊋ obstruction-free ⊋ lock-free ⊋ wait-free.**

**⚠️ Spinlock là NONBLOCKING nhưng KHÔNG lock-free (tr. 207):** không gọi hàm blocking (spin trong loop), nhưng **vẫn là mutex — chỉ một thread khóa được** → không có "một thread luôn tiến". Đây là bẫy hay gặp: "nonblocking" không đủ, phải hỏi lock-free hay không.

**⚠️ Lock-free KHÔNG luôn nhanh hơn (tr. 208–210):**
- **Atomic op chậm** (~100× non-atomic trên desktop) + lock-free có **nhiều atomic op hơn**.
- **Cache ping-pong** khi nhiều thread đụng cùng atomic variable (ch. 8) — gánh nặng lớn.
- **→ Phải profile cả hai bản trước khi quyết.**

**Lý do thật để dùng lock-free (tr. 208):**
1. **Robustness:** thread chết khi giữ lock → cấu trúc hỏng vĩnh viễn; thread chết trong lock-free → chỉ mất data của nó.
2. **Max concurrency:** không thread nào bị chặn chờ thread khác.

**Đánh đổi:** không deadlock, nhưng có **live lock** (hai thread buộc nhau restart mãi) và **starvation** (lock-free không đảm bảo thread nào cũng tiến — chỉ wait-free mới đảm bảo).

</details>

**Câu 2 (🎯🎯 rất hay hỏi):** ABA problem là gì? Cho ví dụ. Cách phòng tránh?

<details><summary>Đáp án</summary>

**ABA problem (tr. 249)** — nguy cơ của **mọi thuật toán CAS**. Kịch bản:
```
1. Thread 1 đọc biến atomic x = A.
2. Thread 1 làm gì đó dựa trên A (vd lưu con trỏ node A).
3. Thread 1 bị OS stall.
4. Thread khác: x đổi A → B (vd pop node A).
5. Thread khác: FREE vùng nhớ của A (hoặc đổi data gắn với A).
6. Thread khác: x đổi B → A — nhưng đây là object MỚI trùng địa chỉ cũ
   (allocator tái dùng địa chỉ của A đã free).
7. Thread 1 tỉnh, CAS x so với A → THÀNH CÔNG (đúng là A về mặt bit),
   nhưng đây là "A sai" — node cũ đã bị free/thay. → HỎNG cấu trúc.
```

**Ví dụ cụ thể — lock-free stack pop:** thread 1 đọc `head = A`, chuẩn bị CAS `head = A->next`. Bị stall. Thread khác pop A, pop A->next, rồi push lại một node MỚI trùng địa chỉ A (allocator tái dùng). Thread 1 tỉnh, CAS `head: A → A->next` **thành công** (head đúng là địa chỉ A), nhưng `A->next` giờ trỏ vào vùng đã free → **head trỏ rác**.

**Cách phòng tránh (tr. 249):**
1. **ABA counter / tagged pointer (phổ biến nhất):** kèm một counter cạnh biến `x`; CAS trên **cả cụm `{x, counter}`** như một đơn vị (cần DWCAS). Mỗi lần thay `x`, **counter++** → dù `x` trùng giá trị cũ, CAS **fail** vì counter đã đổi. 🆕 Đây là "version number" — cùng ý với optimistic locking.
2. **Hazard pointers / reference counting:** node không bị free/tái dùng khi còn thread tham chiếu → không có bước 5-6. **Các cấu trúc trong chương này không bị ABA** chính nhờ vậy.
3. **Không recycle địa chỉ:** ABA **đặc biệt nặng khi dùng free list / tái dùng node** (địa chỉ quay lại). Trả node về allocator (không recycle) giảm nguy cơ.

**Điểm cộng:** liên hệ — đây là lý do `std::atomic` DWCAS (double-word CAS) hữu ích, và vì sao split reference counting (Cụm 5) dùng `counted_node_ptr` gộp counter với pointer.

</details>

**Câu 3 (🎯):** Trong lock-free structure, vấn đề khó nhất là gì? Kể **ba kỹ thuật memory reclamation** và trade-off.

<details><summary>Đáp án</summary>

**Vấn đề khó nhất (tr. 211–214):** **memory reclamation** — khi nào an toàn `delete` một node mà thread khác có thể còn con trỏ tới? Nếu xóa khi thread khác đang dereference → dangling pointer, UB. Logic CAS thì tương đối dễ; *"lots of the complexity in lock-free code comes from managing memory"* (tr. 236).

**Ba kỹ thuật (tr. 248–249):**

| Kỹ thuật | Ý tưởng | Ưu | Nhược |
|---|---|---|---|
| **Đếm thread trong pop** (7.2.2) | `to_be_deleted` list; xóa khi `threads_in_pop == 0` | Đơn giản nhất | **⚠️ Leak khi tải cao** — không có quiescent point (thread mới vào trước khi thread cũ ra hết) → list phình vô hạn |
| **Hazard pointers** (7.2.3) | Mỗi thread công bố con trỏ đang dùng ra chỗ mọi thread thấy; deleter kiểm mọi hazard trước khi xóa | Reclaim **theo từng node**, chịu tải cao | **⚠️ Chậm** (quét mảng atomic mỗi pop, atomic ~100× chậm); **vướng patent IBM** |
| **Reference counting (split)** (7.2.4) | external count (đi với con trỏ) + internal count (trong node); tổng về 0 → xóa | Không vướng patent, gọn ý niệm | **⚠️ Cần DWCAS** (không thì `atomic` dùng mutex → lock-free giả); code rất dài; ABA nếu recycle |

**Ý chung của cả ba (tr. 248):** theo dõi **bao nhiêu thread đang truy cập một object**, chỉ xóa khi **không còn reference nào**.

**Điểm cộng:**
- Nếu platform có **lock-free `std::shared_ptr`** (hoặc C++20 `std::atomic<std::shared_ptr<T>>`) → vấn đề biến mất, dùng thẳng.
- **Garbage collector** là kịch bản lý tưởng (Java/C# lo hộ) — C++ không có sẵn.
- **Split reference counting: tăng external count TRƯỚC khi dereference** → "ghim" con trỏ hợp lệ suốt thời gian truy cập.

</details>

**Câu 4 (🟠):** Viết `push()` cho một lock-free stack. Vì sao dùng `compare_exchange_weak` và điều gì đảm bảo không race?

<details><summary>Đáp án</summary>

```cpp
template<typename T>
class lock_free_stack {
    struct node {
        std::shared_ptr<T> data;
        node* next;
        node(T const& d) : data(std::make_shared<T>(d)) {}  // chuẩn bị data TRONG constructor
    };
    std::atomic<node*> head;
public:
    void push(T const& data) {
        node* const new_node = new node(data);
        new_node->next = head.load();
        while (!head.compare_exchange_weak(new_node->next, new_node));  // CAS-loop
    }
};
```

**Vì sao không race (tr. 210–211):**
- **CAS đảm bảo `head` chưa đổi từ lúc đọc:** `compare_exchange_weak(new_node->next, new_node)` chỉ set `head = new_node` **nếu `head` vẫn bằng `new_node->next`**. Nếu thread khác đã đổi `head` → CAS fail → **tự cập nhật `new_node->next` = giá trị `head` hiện tại** → lặp lại (không cần load lại head thủ công).
- **⚠️ Node phải sẵn sàng HOÀN TOÀN trước khi công bố:** ngay sau CAS thành công, thread khác đọc được node → phải chuẩn bị data **trong constructor**, không sửa sau.

**Vì sao `weak` không `strong` (tr. 211):** vì đã **lặp trực tiếp khi fail** — spurious failure của `weak` chỉ khiến lặp thêm một vòng (vô hại), mà `weak` cho code tối ưu hơn trên vài kiến trúc (không có loop nội bộ như `strong`).

**Phân tích an toàn:** chỗ duy nhất ném là `new node` (tự dọn, list chưa đổi → an toàn); không lock → không deadlock. **⚠️ Lock-free nhưng KHÔNG wait-free** (CAS có thể fail vô hạn lần về lý thuyết).

**Điểm cộng:** `pop` khó hơn nhiều vì **reclamation** (khi nào delete node an toàn) — đó mới là phần khó của lock-free, không phải `push`.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [05-memory-model.md](05-memory-model.md) — **BẮT BUỘC đọc trước:** happens-before, synchronizes-with, 6 memory_order, release sequence, CAS weak/strong. Chương 7 là ứng dụng trực tiếp.
- [06-lock-based-structures.md](06-lock-based-structures.md) — bản lock-based của stack/queue để so sánh (8 dòng vs 37 dòng lock-free).
- [lkd/sync-timers.md](../lkd/sync-timers.md) — **RCU (Read-Copy-Update)** trong kernel — kỹ thuật reclamation lock-free tương tự hazard pointer, dùng cực nhiều trong Linux kernel.
- [cpp-mindset/understanding-the-machine.md](../cpp-mindset/understanding-the-machine.md) — cache coherence (MESI), vì sao atomic chậm + cache ping-pong (ch. 8 sẽ đào sâu).
- Ch. 8 (designing concurrent code) — false sharing, cache ping-pong, Amdahl's law: giải thích **vì sao** lock-free đôi khi chậm hơn lock-based.

**Chương tiếp theo:** [Ch. 8 — Designing concurrent code →](08-designing-concurrent-code.md) 🎯 (chia việc giữa thread; **cache ping-pong, false sharing, oversubscription**; exception safety song song; **Amdahl's law**; parallel `for_each`/`find`/`partial_sum`) — từ cấu trúc dữ liệu chuyển sang **thiết kế code concurrent nói chung**.
