# COD — Bài coding tại chỗ

> Domain `COD`. Track dùng: `dsa`, `cpp-system`, `bsp`, `cpp-mindset`. **Hai mục, hai dạng bài khác nhau:**
>
> | Mục | Dạng | Được chấm cái gì |
> |---|---|---|
> | **[A — Implement từ đầu](#a--implement-từ-đầu)** (`COD-001…013`) | viết code từ trang trắng | ý tưởng → code → **độ phức tạp** |
> | **[B — Đọc code tìm lỗi](#b----đọc-code-tìm-lỗi-bug-hunt)** (`COD-014…026`) | cho sẵn đoạn code có bug | **dòng nào · vì sao · triệu chứng · sửa + công cụ** |
>
> **Khi mock (`coding` type):** interviewer ra đề, ứng viên **viết code vào [../coding-arena/](../coding-arena/)** (mỗi bài 1 file). Review: đúng/chạy, độ phức tạp, edge case, style. Đáp án dưới là *bản mẫu tham chiếu* — che lại khi tự luyện.

---

## A — Implement từ đầu

#### COD-001 · 🟡 · coding · ⭐ · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Đảo Linked List.**
<details><summary>Bản mẫu</summary>

Ý tưởng: đi qua list, đổi hướng con trỏ `next` từng node. Giữ 3 con trỏ prev/cur/next.
```cpp
struct Node { int val; Node* next; };

Node* reverse(Node* head) {
    Node* prev = nullptr;
    while (head) {
        Node* nxt = head->next;  // lưu node kế
        head->next = prev;       // đảo hướng
        prev = head;             // tiến prev
        head = nxt;              // tiến head
    }
    return prev;                 // prev là head mới
}
```
**O(n) time, O(1) space.** — nói **cả hai**: `O(1) space` chính là điểm bản iterative thắng bản đệ quy (đệ quy tốn `O(n)` stack, và **tràn stack** với list dài — nguy hiểm thật trên embedded).

**Follow-up hay bị hỏi:**
- *Vì sao `while (cur)` chứ không phải `while (cur->next)`?* → `cur->next` bỏ sót node cuối (chưa kịp đảo) **và segfault ngay với list rỗng**.
- *Doubly linked list thì sao?* → không cần biến `nxt` nữa: với mỗi node **swap `next` và `prev`**, rồi tiến bằng `cur = cur->prev` (vì `prev` giờ đang giữ node kế cũ). Vẫn trả về node cuối cùng thăm được.
- *Bản đệ quy?* → `O(n)` stack, thường **không** dùng trên hệ nhúng.
</details>

#### COD-002 · 🟡 · coding · ⭐ · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Phát hiện vòng trong Linked List (Floyd).**
<details><summary>Bản mẫu</summary>

Ý tưởng: hai con trỏ nhanh (2 bước) / chậm (1 bước); nếu có vòng chúng sẽ gặp nhau.
```cpp
bool hasCycle(Node* head) {
    Node *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}
```
**O(n) time, O(1) space.**
</details>

#### COD-003 · 🟡 · coding · ⭐ · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Two Sum (hash map).**
<details><summary>Bản mẫu</summary>

```cpp
std::vector<int> twoSum(std::vector<int>& a, int target) {
    std::unordered_map<int,int> seen;       // value -> index
    for (int i = 0; i < (int)a.size(); ++i) {
        int need = target - a[i];
        if (seen.count(need)) return {seen[need], i};
        seen[a[i]] = i;
    }
    return {};
}
```
**O(n) time, O(n) space.**
</details>

#### COD-004 · 🟡 · coding · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Kiểm tra ngoặc cân bằng (stack).**
<details><summary>Bản mẫu</summary>

```cpp
bool isValid(const std::string& s) {
    std::stack<char> st;
    std::unordered_map<char,char> match{{')','('},{']','['},{'}','{'}};
    for (char c : s) {
        if (c=='('||c=='['||c=='{') st.push(c);
        else {
            if (st.empty() || st.top()!=match[c]) return false;
            st.pop();
        }
    }
    return st.empty();
}
```
**O(n) time, O(n) space.**
</details>

#### COD-005 · 🟡 · coding · ⭐ · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Tự cài `memcpy` & `strlen` (rất hay hỏi C/embedded).**
<details><summary>Bản mẫu</summary>

```c
void* my_memcpy(void* dst, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) *d++ = *s++;      // không xử lý vùng chồng lấn (đó là memmove)
    return dst;
}
size_t my_strlen(const char* s) {
    const char* p = s;
    while (*p) ++p;              // đếm tới '\0'
    return (size_t)(p - s);
}
```
**Follow-up:** *khác biệt `memcpy` vs `memmove`?* → `memmove` xử lý đúng khi src/dst chồng lấn (chép lùi nếu cần); `memcpy` không đảm bảo.
</details>

#### COD-006 · 🟡 · coding · ⭐ · [→ ring-buffer.md](../../../12-dsa/ring-buffer.md)
**Ring Buffer.**
<details><summary>Bản mẫu</summary>

```cpp
template <typename T, size_t N>
class RingBuffer {
    T buf_[N]; size_t head_=0, tail_=0;
public:
    bool empty() const { return head_==tail_; }
    bool full()  const { return (head_+1)%N == tail_; }
    bool push(const T& v){ if(full()) return false; buf_[head_]=v; head_=(head_+1)%N; return true; }
    bool pop(T& out){ if(empty()) return false; out=buf_[tail_]; tail_=(tail_+1)%N; return true; }
};
```
**Hỏi ngược trước khi code** (ăn điểm nhất): *một luồng hay nhiều luồng? đầy thì từ chối hay đè cái cũ? byte stream hay phần tử?* Rồi nêu ngay cạm bẫy: `head == tail` vừa là rỗng vừa là đầy → bản trên **hy sinh 1 ô** để phân biệt (chứa được N−1).

**Follow-up theo tầng** — [ring-buffer.md](../../../12-dsa/ring-buffer.md): ① chính sách khi đầy (§4) → ② luồng byte, `memcpy` hai khối khi vắt biên (§5) → ③ mutex + condvar có `close()` (§6) → ④ lock-free SPSC: chỉ số chạy tự do + mask, **hai** cặp release/acquire, `alignas(64)` chống false sharing (§7) → ⑤ nó nằm ở đâu trong Linux thật: `dmesg`, pipe, `kfifo`, ALSA, `io_uring` (§8).
</details>

#### COD-007 · 🟢 · coding · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Reverse string in-place / Palindrome.**
<details><summary>Bản mẫu</summary>

```cpp
void reverseStr(std::string& s) {
    int i=0, j=(int)s.size()-1;
    while (i<j) std::swap(s[i++], s[j--]);   // two pointers
}
bool isPalindrome(const std::string& s) {
    int i=0, j=(int)s.size()-1;
    while (i<j) if (s[i++]!=s[j--]) return false;
    return true;
}
```
</details>

#### COD-008 · 🟡 · coding · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**BFS & DFS trên đồ thị.**
<details><summary>Bản mẫu</summary>

```cpp
// graph: adjacency list  vector<vector<int>>
void bfs(const std::vector<std::vector<int>>& g, int start) {
    std::vector<bool> vis(g.size(), false);
    std::queue<int> q; q.push(start); vis[start]=true;
    while (!q.empty()) {
        int u=q.front(); q.pop();
        for (int v : g[u]) if (!vis[v]) { vis[v]=true; q.push(v); }
    }
}
void dfs(const std::vector<std::vector<int>>& g, int u, std::vector<bool>& vis) {
    vis[u]=true;
    for (int v : g[u]) if (!vis[v]) dfs(g, v, vis);
}
```
</details>

#### COD-009 · 🟢 · coding · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Đếm số bit 1 (Brian Kernighan).**
<details><summary>Bản mẫu</summary>

```cpp
int popcount(unsigned x) {
    int c=0;
    while (x) { x &= (x-1); ++c; }   // mỗi lần xoá bit 1 thấp nhất
    return c;
}
```
</details>

#### COD-010 · 🟢 · coding · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Kiểm tra endianness.**
<details><summary>Bản mẫu</summary>

```c
int is_little_endian() {
    unsigned int x = 1;
    return *(char*)&x == 1;   // byte thấp ở địa chỉ thấp -> little-endian
}
```
</details>

#### COD-011 · 🟡 · coding · ⭐ · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Tự cài `vector<int>` thô: `push_back`, `operator[]`, `at`, `size`, `capacity`.** Nêu chiến lược tăng dung lượng và vì sao.
<details><summary>Bản mẫu</summary>

**Hỏi spec trước:** cần copy được không? cần move? `at` ném exception hay trả mã lỗi (embedded thường tắt exception)?

```cpp
class IntVec {
    int*   d_   = nullptr;
    size_t n_   = 0;
    size_t cap_ = 0;
    void grow(size_t want) {
        size_t c = cap_ ? cap_ * 2 : 4;      // NHÂN ĐÔI, không cộng thêm
        while (c < want) c *= 2;
        int* nd = new int[c];
        for (size_t i = 0; i < n_; ++i) nd[i] = d_[i];
        delete[] d_;
        d_ = nd; cap_ = c;
    }
public:
    IntVec() = default;
    ~IntVec() { delete[] d_; }
    IntVec(const IntVec& o) : d_(new int[o.cap_]), n_(o.n_), cap_(o.cap_) {
        for (size_t i = 0; i < n_; ++i) d_[i] = o.d_[i];
    }
    IntVec& operator=(IntVec o) { swap(o); return *this; }   // copy-and-swap
    IntVec(IntVec&& o) noexcept { swap(o); }
    void swap(IntVec& o) noexcept {
        std::swap(d_, o.d_); std::swap(n_, o.n_); std::swap(cap_, o.cap_);
    }
    void push_back(int v) { if (n_ == cap_) grow(n_ + 1); d_[n_++] = v; }
    int& operator[](size_t i) { return d_[i]; }              // KHÔNG kiểm tra — như STL
    int  at(size_t i) const { if (i >= n_) throw std::out_of_range("at"); return d_[i]; }
    size_t size() const { return n_; }
    size_t capacity() const { return cap_; }
};
```

**Ba điểm interviewer thật sự chấm:**
1. ⭐ **Vì sao nhân đôi chứ không `cap_+1`?** Nhân đôi cho `push_back` chi phí **amortized O(1)** (n lần push tốn tổng O(n) phép copy). Cộng thêm 1 mỗi lần → mỗi push copy lại toàn bộ → **O(n²)**. Đây là câu hỏi hay bị hỏi ngược lại nhất.
2. **Rule of 3/5** — có con trỏ thô sở hữu bộ nhớ ⇒ **bắt buộc** định nghĩa destructor + copy + move, nếu không thì shallow copy → **double free** (xem [COD-023](coding.md)).
3. **`operator[]` không kiểm tra, `at()` có** — giống hợp đồng của STL. Nói ra sự phân biệt này là tín hiệu tốt.

**Output đã chạy** (`g++ -std=c++17 -Wall -Wextra -O2`, không warning):
```
n=1 cap=4
n=2 cap=4
n=3 cap=4
n=4 cap=4
n=5 cap=8
...
n=9 cap=16
n=10 cap=16
v[0]=0 w[0]=99 (copy doc lap)
at(100) nem: at
```

**Follow-up:** *push_back có invalidate iterator/con trỏ không?* → **có**, khi realloc. Đây đúng là lớp bug hay gặp thật. · *Embedded thì sao?* → cấp phát động trong hot path là rủi ro; ưu tiên `reserve()` một lần lúc init, hoặc mảng tĩnh có sức chứa cố định.
</details>

#### COD-012 · 🟠 · coding · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Tự cài hash map `string → int` bằng separate chaining:** `insert` (trùng key thì ghi đè), `find`, tự rehash.
<details><summary>Bản mẫu</summary>

**Hỏi spec trước:** key trùng thì ghi đè hay báo lỗi? cần xoá không? ước lượng số phần tử (để chọn số bucket ban đầu)?

```cpp
class StrIntMap {
    struct Node { std::string k; int v; Node* next; };
    std::vector<Node*> buckets_;
    size_t n_ = 0;
    static size_t hash(const std::string& s) {              // FNV-1a
        size_t h = 1469598103934665603ULL;
        for (char c : s) { h ^= (unsigned char)c; h *= 1099511628211ULL; }
        return h;
    }
    void rehash() {
        std::vector<Node*> old = buckets_;
        buckets_.assign(old.size() * 2, nullptr);
        for (Node* p : old)
            while (p) { Node* nx = p->next;                 // GỠ node cũ, không cấp phát mới
                        size_t b = hash(p->k) % buckets_.size();
                        p->next = buckets_[b]; buckets_[b] = p; p = nx; }
    }
public:
    explicit StrIntMap(size_t nb = 8) : buckets_(nb, nullptr) {}
    ~StrIntMap() { for (Node* p : buckets_) while (p) { Node* nx = p->next; delete p; p = nx; } }
    StrIntMap(const StrIntMap&) = delete;                   // chưa cần copy -> CẤM, đừng để mặc định sai
    StrIntMap& operator=(const StrIntMap&) = delete;
    void insert(const std::string& k, int v) {
        size_t b = hash(k) % buckets_.size();
        for (Node* p = buckets_[b]; p; p = p->next)
            if (p->k == k) { p->v = v; return; }            // trùng key -> ghi đè
        buckets_[b] = new Node{k, v, buckets_[b]};          // chèn đầu list: O(1)
        if (++n_ > buckets_.size()) rehash();               // load factor 1.0
    }
    bool find(const std::string& k, int& out) const {
        size_t b = hash(k) % buckets_.size();
        for (Node* p = buckets_[b]; p; p = p->next)
            if (p->k == k) { out = p->v; return true; }
        return false;
    }
};
```

**Bốn điểm được chấm:**
1. **Vì sao cần rehash?** Load factor tăng ⇒ chain dài ⇒ `find` tiến về **O(n)**. Rehash giữ chain ngắn để `find` **trung bình O(1)**.
2. ⭐ **`find` là O(1) trung bình, KHÔNG phải O(1) tệ nhất** — tệ nhất O(n) khi mọi key đụng cùng bucket. Nói được điều này quan trọng hơn viết đúng hàm hash.
3. **Rehash gỡ node cũ chứ không cấp phát lại** — tránh copy `std::string` lần nữa.
4. **`= delete` copy** khi chưa cần: có con trỏ thô mà để copy mặc định là double free. Cấm còn hơn cài sai.

**Output đã chạy** (10 key + 1 lần ghi đè, không warning):
```
size=10 buckets=16
find(frc)   = 1, v=777
find(khong) = 0
```

**Follow-up:** *chaining vs open addressing?* → chaining đơn giản, chịu được load factor cao, nhưng mỗi node là **một lần cấp phát + một lần nhảy con trỏ** (xấu cho cache). Open addressing bám cache tốt hơn, hợp embedded, nhưng xoá phức tạp (tombstone) và tệ khi load factor cao. · *Vì sao embedded hay tránh `std::unordered_map`?* → cấp phát động từng node, footprint khó đoán.
</details>

#### COD-013 · 🟢 · coding · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Cài queue (FIFO) chỉ dùng hai stack.** Nêu độ phức tạp của `pop`.
<details><summary>Bản mẫu</summary>

Ý tưởng: `in_` nhận phần tử mới; `out_` trả phần tử ra. `out_` rỗng thì **dốc toàn bộ** `in_` sang — thứ tự bị đảo hai lần nên thành đúng FIFO.

```cpp
class Queue2Stack {
    std::stack<int> in_, out_;
    void shift() { while (!in_.empty()) { out_.push(in_.top()); in_.pop(); } }
public:
    void push(int v) { in_.push(v); }
    int pop() {
        if (out_.empty()) shift();                       // CHỈ dốc khi out_ rỗng
        if (out_.empty()) throw std::runtime_error("queue rong");
        int v = out_.top(); out_.pop(); return v;
    }
    bool empty() const { return in_.empty() && out_.empty(); }
    size_t size() const { return in_.size() + out_.size(); }
};
```

⭐ **Điểm được chấm nằm ở độ phức tạp, không ở code:** `pop` **tệ nhất O(n)** (lần phải dốc), nhưng **amortized O(1)** — mỗi phần tử chỉ bị chuyển từ `in_` sang `out_` **đúng một lần** trong cả đời nó. Người trả lời "O(n)" thì đúng một nửa; người nói được "amortized O(1) vì mỗi phần tử chuyển đúng một lần" mới là đủ.

⚠️ **Bẫy hay mắc:** dốc `in_` sang `out_` **mỗi lần pop** (kể cả khi `out_` còn hàng) → thành **O(n) thật** mỗi lần, mất hết lợi ích. Điều kiện `if (out_.empty())` chính là toàn bộ thuật toán.

**Output đã chạy:** push 1,2,3 → pop 2 lần → push 4 → dốc hết:
```
1 2 3 4
pop rong nem: queue rong
```

**Follow-up:** *`empty()` viết sao cho đúng?* → phải kiểm **cả hai** stack. Chỉ kiểm `out_` là sai khi vừa push xong.
</details>

---

## B — 🔎 Đọc code tìm lỗi (bug hunt)

> **Dạng bài trên giấy: đưa một đoạn code, hỏi *"có bug gì, biểu hiện thế nào, sửa ra sao"*.** Khác với mục A ở chỗ không cần viết từ đầu — nhưng khó hơn ở chỗ phải **đọc ra thứ compiler không nói**.
>
> ⚠️ **Mọi output trong mục này là OUTPUT THẬT đã chạy** (`gcc/g++ 11.4.0`, Ubuntu 22.04, x86-64), dán nguyên văn theo [config §6 luật ⑥](../config.md). Chưa chạy thì đừng khẳng định.
>
> 📌 **Số liệu đo thật khi dựng bộ này — đáng nhớ hơn cả từng snippet.** Đếm warning của `gcc/g++ -Wall -Wextra` trên cả 13 bài:
>
> | Mức build | Số bài có warning | Bài nào |
> |---|---|---|
> | **`-O0`** (debug build) | **3 / 13** | `COD-017`, `020`, `022` |
> | **`-O2`** (release build) | **5 / 13** | thêm `COD-016`, `025` |
>
> ⭐ **Hai kết luận:** ① **8 bài compile sạch ở mọi mức** — gồm double free (`023`), deadlock (`024`), đọc ngoài mảng (`014`), treo vì thiếu `volatile` (`018`). ② **Chính cảnh báo cũng phụ thuộc `-O`** — `-Warray-bounds` và `-Wrestrict` cần optimizer chạy phân tích mới thấy, nên **build debug cảnh báo ÍT hơn build release**. Ai chỉ build `-O0` rồi tin `-Wall` sạch là đang tin vào một phép đo yếu hơn mình tưởng.
>
> Đó là lý do câu hỏi *"bạn phát hiện lớp bug này bằng cách nào"* quan trọng ngang việc chỉ đúng dòng sai.
>
> **Khung trả lời được chấm (4 phần, thiếu phần nào trừ phần đó):** ① **dòng nào** sai · ② **vì sao** sai (cơ chế, không phải "nó lỗi") · ③ **triệu chứng ngoài đời** (crash? sai ngầm? treo? chỉ sai ở `-O2`?) · ④ **sửa thế nào** + **phát hiện bằng công cụ gì**.

#### COD-014 · 🟢 · coding · ⭐ · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Đoạn này tính tổng `n` phần tử đầu. Sai ở đâu?**
```c
int sum_first(const int *a, int n) {
    int s = 0;
    for (int i = 0; i <= n; i++) s += a[i];
    return s;
}
```
<details><summary>Đáp án</summary>

① **`i <= n`** — off-by-one kinh điển. Mảng `n` phần tử có chỉ số hợp lệ `0..n-1`; vòng lặp chạm `a[n]`, **đọc ngoài mảng**.

② Đọc một `int` ngay sau vùng cấp phát. Không phải "lỗi" theo nghĩa chương trình dừng — đó là **UB**, muốn làm gì thì làm.

③ ⭐ **Triệu chứng nguy hiểm nhất: KHÔNG có triệu chứng.** Chạy thật với `a = {1,2,3,4,5}`:
```
sum = 15
```
Đúng bằng kết quả mong đợi — vì `a[5]` tình cờ là 0. Bug ngồi im cho tới ngày layout bộ nhớ đổi (thêm một biến, đổi compiler, đổi phiên bản library) rồi trả về số rác. **`gcc -Wall -Wextra` không báo gì.**

④ Sửa `i < n`. Phát hiện bằng **ASan** — cùng chương trình đó:
```
==5583==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x503000000054
READ of size 4 at 0x503000000054 thread T0
    #0 ... in sum_first b14.c:5
    #1 ... in main b14.c:11
```

**Follow-up:** *nếu `a` là mảng trên stack thay vì heap thì ASan còn bắt được không?* → có, báo `stack-buffer-overflow`. · *Vì sao `-Wall` không bắt?* → `n` là tham số runtime, compiler không biết kích thước; bắt được chỉ khi mảng có kích thước hằng nhìn thấy tại chỗ.
</details>

#### COD-015 · 🟢 · coding · ⭐ · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Hàm log này chạy sập. Vì sao?**
```c
void log_line(const char *tag, int code) {
    char buf[16];
    sprintf(buf, "[%s] code=%d", tag, code);
    puts(buf);
}
log_line("DISPLAY_DIMMING", 12345);   // goi nhu the nay
```
<details><summary>Đáp án</summary>

① **`sprintf` không có giới hạn kích thước.** Chuỗi kết quả `[DISPLAY_DIMMING] code=12345` dài **29 ký tự + NUL = 30 byte**, viết vào `buf[16]`.

② Ghi đè 14 byte ngoài mảng — trên stack, tức đè lên **stack canary** và có thể cả **địa chỉ trả về**. Đây chính là lớp lỗi dùng để khai thác (stack smashing).

③ Chạy thật:
```
*** stack smashing detected ***: terminated
```
Chương trình bị `abort` bởi cơ chế `-fstack-protector` (bật mặc định trên Ubuntu). ⚠️ **Đừng kết luận "may quá có canary bắt"** — canary chỉ bắt lúc hàm *return*; nếu dữ liệu bị đè là biến local đang dùng thì hỏng lặng lẽ trước đó.

④ Sửa: `snprintf(buf, sizeof buf, ...)` và **kiểm giá trị trả về** (`snprintf` trả về độ dài *cần thiết*, lớn hơn `sizeof buf` nghĩa là đã bị cắt). Phát hiện: ASan, `-fstack-protector-strong`, hoặc `-Wformat-overflow` khi tag là hằng nhìn thấy được.

**Follow-up:** *`snprintf` có luôn NUL-terminate không?* → **có** (khác `strncpy`, xem [COD-020](coding.md)), trừ khi kích thước truyền vào là 0.
</details>

#### COD-016 · 🟠 · coding · ⭐ · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Muốn dịch nội dung buffer sang phải để chèn header. Đoạn này sai chỗ nào — và vì sao test lại pass?**
```c
memcpy(buf + 1, buf, 256);   /* dich phai 1 byte */
```
<details><summary>Đáp án</summary>

① **`memcpy` cấm hai vùng chồng lấn.** Chuẩn C quy định tham số của `memcpy` là `restrict` — trình biên dịch và libc được phép giả định không chồng lấn. Ở đây `[buf, buf+256)` và `[buf+1, buf+257)` chồng nhau 255 byte.

② Nếu libc copy **xuôi** từ đầu, byte vừa ghi sẽ bị đọc lại làm nguồn → nội dung bị "bôi" (smear). Nếu copy **ngược** hoặc copy theo khối lớn, kết quả lại đúng. **Kết quả phụ thuộc implementation** — đó là định nghĩa của UB.

③ ⭐ **Đây là bug nguy hiểm nhất trong cả bộ, vì nó phụ thuộc mức tối ưu.** Cùng một file, so `memcpy` với `memmove`:
```
### -O0 ###
memcpy == memmove (bug KHONG lo ra o lan chay nay)
### -O2 ###
LECH tu byte 17: memcpy=0x0F  memmove=0x10
```
Debug build **sạch**, release build **sai dữ liệu** — và sai ngầm, không crash. Đây đúng là kịch bản *"chạy ở lab thì đúng, ra hiện trường thì sai"*.

④ Sửa: **`memmove`** — nó xử lý chồng lấn đúng (chọn chiều copy theo vị trí). Chi phí gần như bằng nhau; đừng "tối ưu" bằng `memcpy`. Phát hiện: `gcc -Wall -Wextra` **có** bắt được, nhưng ⚠️ **chỉ từ `-O1` trở lên** (đo thật: `-O0` → 0 warning, `-O1`/`-O2` → 1) và chỉ khi nhìn thấy cả hai con trỏ:
```
warning: '__builtin_memcpy' accessing 8 bytes at offsets 1 and 0 overlaps 7 bytes at offset 1 [-Wrestrict]
```
nhưng **không** bắt được khi hai con trỏ đến từ tham số hàm. Valgrind `memcheck` báo *"Source and destination overlap"*; **ASan thì không** — nó kiểm biên vùng nhớ, không kiểm chồng lấn.

**Follow-up:** *khi nào `memcpy` vẫn đúng dù hai vùng gần nhau?* → khi **không** giao nhau, dù liền kề. · *`std::copy` thì sao?* → cũng không đảm bảo với vùng chồng lấn; muốn dịch về sau dùng `std::copy_backward`.
</details>

#### COD-017 · 🟡 · coding · ⭐ · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Hàm này biên dịch được. Nó có chạy được không?**
```cpp
const std::string& name_of(int id) {
    std::string s = "panel-" + std::to_string(id);
    return s;
}
```
<details><summary>Đáp án</summary>

① **Trả về reference tới biến local.** `s` bị huỷ khi hàm return; reference trả về trỏ vào stack frame đã chết — **dangling reference**.

② Người gọi cầm một reference hợp lệ về mặt kiểu nhưng trỏ vào bộ nhớ đã thu hồi. Đọc nó là UB.

③ Chạy thật — segfault ngay:
```
AddressSanitizer: SEGV on unknown address 0x000000000008
The signal is caused by a READ memory access.
    #0 ... in std::__cxx11::basic_string<...>::size() const
```
Ở đây may là sập ngay. Nguy hiểm hơn là ca **sập muộn** — stack frame chưa bị đè, đọc ra đúng giá trị trong vài lần chạy đầu.

④ Sửa: **trả về theo giá trị** `std::string name_of(int)`. Không sợ tốn — C++17 có **guaranteed copy elision**, giá trị trả về được dựng thẳng tại chỗ người gọi, không có copy nào. ⭐ Đây là chỗ ghi điểm C++17 tự nhiên nhất.

Đây là **một trong 4 snippet compiler có bắt**:
```
warning: reference to local variable 's' returned [-Wreturn-local-addr]
```

**Follow-up (rất hay hỏi):** *cùng lớp lỗi này còn xuất hiện ở đâu?* → ⓐ `std::string_view` trỏ vào temporary (`sv = std::string("x") + "y"`) · ⓑ lambda **bắt tham chiếu** rồi chạy bất đồng bộ · ⓒ giữ iterator/con trỏ sau khi `vector` realloc ([COD-011](coding.md)) · ⓓ trả reference tới member của object đã chết. · *Nếu `s` là `static` thì sao?* → hết dangling nhưng **không reentrant / không thread-safe**, đổi bug này lấy bug khác.
</details>

#### COD-018 · 🟠 · coding · ⭐ · [→ interrupts-bare-metal](../../../08-embedded-systems/interrupts-bare-metal.md)
**Vòng chờ cờ do interrupt handler bật. Chạy debug build thì đúng, release build thì treo. Vì sao?**
```c
static int stop = 0;
static void on_alarm(int sig) { stop = 1; }     /* dong vai ISR */

int main(void) {
    signal(SIGALRM, on_alarm);
    alarm(1);
    while (!stop) { }
    puts("thoat vong lap");
}
```
<details><summary>Đáp án</summary>

① **`stop` thiếu `volatile`.** (Và trong ngữ cảnh signal, đúng chuẩn phải là `volatile sig_atomic_t`.)

② Trong `while (!stop) {}` compiler không thấy đoạn nào trong luồng điều khiển ghi vào `stop`, nên nó được phép **nạp `stop` vào thanh ghi một lần** rồi lặp trên thanh ghi đó — thành vòng lặp vô hạn. Đây **không phải lỗi compiler**: theo mô hình bộ nhớ của C, việc handler ghi vào một biến không `volatile` không nằm trong tầm nhìn của nó.

③ ⭐ **Cùng một file, khác nhau đúng ở mức tối ưu** — chạy thật với `timeout 4`:
```
### -O0 ###   thoat vong lap        [exit=0]
### -O2 ###   (khong in gi)         [exit=124  <- bi timeout giet, tuc TREO]
```
`-Wall -Wextra`: **không một warning**. Đây là nguyên mẫu của lớp bug *"chỉ có ở release"* — và trên embedded nó xuất hiện với **cờ do ISR bật** hoặc **thanh ghi memory-mapped**.

④ Sửa: `static volatile sig_atomic_t stop = 0;`. `volatile` = *"mỗi lần đọc phải đọc lại từ bộ nhớ thật"*, đúng thứ cần cho MMIO và cờ ISR.

**Follow-up (ranh giới hay bị hỏi ngược):** *`volatile` có làm biến trở nên thread-safe không?* → **KHÔNG.** Nó chặn tối ưu hoá đọc/ghi, **không** cung cấp atomicity, cũng **không** cung cấp thứ tự (memory ordering) giữa các luồng. Đồng bộ giữa thread phải dùng `std::atomic` / mutex. Dùng `volatile` để đồng bộ đa luồng là quan niệm sai phổ biến nhất về từ khoá này. · *Vậy `volatile` dùng ở đâu là đúng?* → MMIO register, cờ do ISR bật, biến chia sẻ với `setjmp/longjmp`.
</details>

#### COD-019 · 🟡 · coding · ⭐ · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Hàm in các cặp liền kề. Nó sập với một loại input rất bình thường — loại nào?**
```cpp
void print_pairs(const std::vector<int>& v) {
    for (size_t i = 0; i < v.size() - 1; ++i)
        printf("%d,%d\n", v[i], v[i+1]);
}
```
<details><summary>Đáp án</summary>

① **`v.size() - 1` khi `v` rỗng.** `size()` trả về `size_t` — **kiểu không dấu**. `0 - 1` không thành `-1` mà **wrap** thành `SIZE_MAX` = 18.446.744.073.709.551.615.

② Vòng lặp tưởng là "chạy 0 lần" thực ra chạy gần như vô hạn, và `v[0]` trên vector rỗng đọc vào vùng không hợp lệ ngay vòng đầu.

③ Chạy thật với `std::vector<int> empty;`:
```
Segmentation fault (core dumped)      [exit=139]
```
Với ASan:
```
AddressSanitizer: SEGV on unknown address 0x000000000004
    #0 ... in print_pairs(std::vector<int...> const&) b19.cpp:5
```
`-Wall -Wextra`: **không warning** — hai toán hạng cùng là unsigned nên không có so sánh signed/unsigned để mà cảnh báo.

④ Ba cách sửa, nêu được cả ba là tốt nhất:
- `for (size_t i = 0; i + 1 < v.size(); ++i)` — ⭐ **an toàn nhất, không có phép trừ nào**;
- `if (v.empty()) return;` rồi giữ nguyên;
- `if (v.size() >= 2)` bọc ngoài.

**Follow-up:** *vì sao `i + 1 < v.size()` tốt hơn `i < v.size() - 1`?* → nó **không bao giờ trừ trên kiểu không dấu**, nên không có chỗ để wrap. Quy tắc chung: với unsigned, **chuyển phép trừ thành phép cộng ở vế kia**. · *`-Wsign-compare` bắt được ca nào?* → ca so sánh `int i` với `v.size()`, không phải ca này.
</details>

#### COD-020 · 🟡 · coding · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Đoạn này copy tên cấu hình vào buffer. Có gì sai?**
```c
char dst[8];
strncpy(dst, "DISPLAY_1", sizeof dst);
printf("dst = [%s]\n", dst);
```
<details><summary>Đáp án</summary>

① **`strncpy` KHÔNG đảm bảo NUL-terminate.** Nguồn dài 9 ký tự, `sizeof dst` = 8 ⇒ nó chép đúng 8 ký tự và **dừng, không thêm `'\0'`**.

② `dst` chứa 8 ký tự đặc, không có kết thúc chuỗi. Mọi hàm `str*` sau đó (`printf %s`, `strlen`, `strcmp`) sẽ **đọc tràn sang bộ nhớ kế tiếp** cho tới khi tình cờ gặp byte 0.

③ Chạy thật:
```
strlen(dst) = 8  (buffer chi 8 byte)
hex: 44 49 53 50 4C 41 59 5F 00 FD 02 10
```
`strlen` bằng đúng `sizeof` là **dấu hiệu chắc chắn** không còn chỗ cho NUL. Lần chạy này byte thứ 9 tình cờ là `00` nên `printf` trông vẫn "bình thường" — đó là kiểu bug đi qua review dễ nhất.

④ Sửa — chọn một:
- `snprintf(dst, sizeof dst, "%s", src)` — ⭐ **luôn NUL-terminate**, cách gọn nhất;
- `strncpy(dst, src, sizeof dst - 1); dst[sizeof dst - 1] = '\0';`
- `strlcpy` nếu nền tảng có (BSD, không có trong glibc chuẩn).

Đây là **một trong 4 snippet compiler bắt được**:
```
warning: 'strncpy' output truncated copying 8 bytes from a string of length 9 [-Wstringop-truncation]
```
nhưng chỉ khi nguồn là chuỗi hằng nhìn thấy tại chỗ.

**Follow-up:** *`strncpy` sinh ra để làm gì mà lại có hành vi kỳ vậy?* → nó **không** được thiết kế cho chuỗi C an toàn, mà cho **trường độ dài cố định** kiểu bản ghi cũ (điền phần dư bằng `'\0'`, không cần terminator). Dùng nó như "strcpy an toàn" là dùng sai mục đích.
</details>

#### COD-021 · 🟡 · coding · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Hàm tạo mask `bits` bit thấp. Nó sai với một giá trị — giá trị nào?**
```c
static uint32_t mask_upto(int bits) { return (1u << bits) - 1; }
```
<details><summary>Đáp án</summary>

① **`bits == 32`.** Dịch một giá trị `unsigned int` (32 bit) đi **đủ hoặc quá 32 bit** là **UB** theo chuẩn C.

② Trên x86, lệnh `shl` chỉ lấy **5 bit thấp** của số đếm ⇒ `1u << 32` thực thi thành `1u << 0` = 1 ⇒ hàm trả về **0** thay vì `0xFFFFFFFF`. Trên ARM kết quả có thể là 0. Hai kiến trúc, hai kết quả khác nhau — đặc trưng UB.

③ Chạy thật:
```
bits=30 -> mask=0x3FFFFFFF
bits=31 -> mask=0x7FFFFFFF
bits=32 -> mask=0x00000000     <-- ky vong 0xFFFFFFFF
```
`-Wall -Wextra`: **không warning** (vì `bits` là biến runtime; nếu viết thẳng `1u << 32` thì gcc mới báo).

④ Sửa:
- ép rộng hơn rồi hạ: `return (uint32_t)(((uint64_t)1 << bits) - 1);` — đúng tới `bits == 32`;
- hoặc chặn ca biên: `return bits >= 32 ? 0xFFFFFFFFu : (1u << bits) - 1;`

**Follow-up:** *`1 << 31` với `int` thì sao?* → cũng UB (tràn signed). Với bit mask **luôn dùng unsigned**: `1u`, `1UL`, `UINT32_C(1)`. · *Phát hiện bằng gì?* → **UBSan** (`-fsanitize=undefined`) báo `shift exponent 32 is too large`.
</details>

#### COD-022 · 🟢 · coding · [→ bare-metal-c](../../../08-embedded-systems/bare-metal-c.md)
**Chạy chế độ standalone mà message queue vẫn được mở. Vì sao?**
```c
switch (m) {
case MODE_STANDALONE:
    init_local();
case MODE_SYNC:
    init_mq();
    break;
}
```
<details><summary>Đáp án</summary>

① **Thiếu `break` sau `init_local()`** — `case MODE_STANDALONE` rơi xuyên (fall through) vào `case MODE_SYNC`.

② `switch` trong C/C++ là **nhảy tới nhãn rồi chạy tiếp**, không phải "chọn một nhánh". Không có `break` thì chạy thẳng qua nhãn kế.

③ Chạy thật với `start(MODE_STANDALONE)`:
```
mode=0
  init_local()
  init_mq()        <-- khong duoc goi o che do standalone
```

④ Sửa: thêm `break;`. Nếu **cố ý** muốn rơi xuyên thì đánh dấu rõ ràng — C++17 có sẵn thuộc tính cho việc này: `[[fallthrough]];` (C thì dùng `__attribute__((fallthrough))` hoặc comment `/* fallthrough */`).

Đây là **một trong 4 snippet compiler bắt được** — và nó nằm trong `-Wextra`:
```
warning: this statement may fall through [-Wimplicit-fallthrough=]
    9 |         init_local();
      |         ^~~~~~~~~~~~
   10 |     case MODE_SYNC:
```
⭐ **Nói được rằng `[[fallthrough]]` làm warning này im lặng** là ghi điểm C++17 đúng chỗ: nó biến ý định của người viết thành thứ compiler kiểm tra được.

**Follow-up:** *có lúc nào fall through là đúng không?* → có, khi nhiều nhãn dùng chung một khối (`case 'a': case 'b':` — không có câu lệnh xen giữa thì không cảnh báo), hoặc chuỗi xử lý tăng dần cố ý.
</details>

#### COD-023 · 🟠 · coding · ⭐ · [→ raii-smart-pointers](../../../02-modern-cpp/raii-smart-pointers.md)
**Class này trông đầy đủ RAII. Chạy thì sập. Vì sao?**
```cpp
class Buffer {
    int*   data_;
    size_t n_;
public:
    explicit Buffer(size_t n) : data_(new int[n]), n_(n) {}
    ~Buffer() { delete[] data_; }
    int& operator[](size_t i) { return data_[i]; }
    size_t size() const { return n_; }
};

static void consume(Buffer b) { printf("consume: size=%zu\n", b.size()); }
int main() { Buffer a(4); a[0] = 42; consume(a); printf("a[0]=%d\n", a[0]); }
```
<details><summary>Đáp án</summary>

① **Vi phạm Rule of Three.** Class có destructor tự giải phóng tài nguyên nhưng **không định nghĩa copy constructor / copy assignment** ⇒ compiler sinh bản mặc định, và bản mặc định copy **giá trị con trỏ** (shallow copy).

② `consume(a)` truyền **theo giá trị** ⇒ `b` là bản copy nông, `b.data_ == a.data_`. Khi `consume` return, `~Buffer()` của `b` chạy `delete[]` trên vùng nhớ **mà `a` vẫn đang sở hữu**. Từ đó `a` là dangling; cuối `main`, `~Buffer()` của `a` `delete[]` lần thứ hai.

③ Chạy thật — thấy được **cả hai** hậu quả, theo thứ tự:
```
consume: size=4
sau consume: a[0]=1624025173      <-- use-after-free: 42 da bien mat
free(): double free detected in tcache 2
Aborted (core dumped)             [exit=134]
```
Với ASan:
```
AddressSanitizer: heap-use-after-free on address 0x502000000010
READ of size 4 at 0x502000000010 thread T0
    #0 ... in main b23.cpp:17
```
`-Wall -Wextra`: **không một warning.** ⭐ Đây là điểm đáng nhớ nhất của snippet — lỗi bộ nhớ nặng nhất trong bộ lại là lỗi compiler im lặng hoàn toàn.

④ Ba cách sửa, xếp theo thứ tự nên ưu tiên:
1. ⭐ **Rule of Zero** — bỏ con trỏ thô, dùng `std::vector<int>` hoặc `std::unique_ptr<int[]>`. Khi đó **không cần viết destructor, copy hay move nào cả**. Đây là câu trả lời tốt nhất cho C++ hiện đại.
2. **Rule of Five** — nếu buộc phải quản lý tay: định nghĩa đủ destructor + copy ctor + copy assign + move ctor + move assign (deep copy).
3. **`= delete` copy** nếu ngữ nghĩa không nên copy — như [COD-012](coding.md) đã làm.

**Follow-up:** *vì sao gọi là Rule of THREE mà giờ lại nói FIVE?* → C++11 thêm move ⇒ 5 hàm. Và **Rule of Zero** mới là mặc định nên nhắm tới: định nghĩa 0 hàm bằng cách để member tự quản lý tài nguyên. · *`consume(const Buffer&)` có chữa được không?* → chữa được **ca này** (không còn copy), nhưng **không chữa được bug** — class vẫn copy sai ở mọi chỗ khác.
</details>

#### COD-024 · 🟠 · coding · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Hàm này thỉnh thoảng làm cả chương trình đứng im. Chỉ ra kịch bản.**
```cpp
static std::mutex m;
static int regs[4];

static int read_reg(int idx) {
    m.lock();
    if (idx < 0 || idx >= 4) return -1;    // duong thoat som
    int v = regs[idx];
    m.unlock();
    return v;
}
```
<details><summary>Đáp án</summary>

① **Đường thoát sớm `return -1` bỏ qua `m.unlock()`.** Mutex bị giữ vĩnh viễn.

② Lần gọi tiếp theo — **kể cả từ chính luồng đó** — chặn ở `m.lock()` mãi mãi. `std::mutex` **không** reentrant: cùng một luồng khoá hai lần là UB, thực tế trên glibc là treo.

③ Chạy thật, 3 lời gọi liên tiếp `read_reg(2)`, `read_reg(9)`, `read_reg(1)`:
```
regs[2] = 30
regs[9] = -1
                        <-- dung o day, khong in gi them
[exit=124]  (bi timeout 4s giet -> TREO)
```
`-Wall -Wextra`: **không warning.** Triệu chứng ngoài đời là *"app treo, không crash, không log"* — đúng lớp bug khó nhất, phải soi bằng `gdb thread apply all bt` hoặc `/proc/<pid>/task/*/stack`.

④ Sửa — **dùng RAII, đừng khoá tay**:
```cpp
static int read_reg(int idx) {
    std::lock_guard<std::mutex> lk(m);       // hoac std::scoped_lock lk(m); (C++17)
    if (idx < 0 || idx >= 4) return -1;      // unlock tu dong o MOI duong ra
    return regs[idx];
}
```
⭐ Lý do thật sự khiến RAII thắng ở đây: nó đúng ở **mọi** đường thoát — kể cả đường bạn chưa nghĩ tới, và kể cả khi có **exception** ném ra giữa vùng khoá. Người sửa bằng cách "thêm `m.unlock()` trước `return -1`" là vá đúng ca này nhưng **để nguyên lớp bug**.

**Follow-up:** *`std::scoped_lock` khác `lock_guard` chỗ nào (C++17)?* → `scoped_lock` khoá được **nhiều mutex cùng lúc** theo thuật toán tránh deadlock; với một mutex thì tương đương. C++17 khuyên dùng `scoped_lock` làm mặc định. · *Kiểm tra tham số nên đặt trước hay sau khi khoá?* → **trước** — `idx` là tham số local, không cần bảo vệ. Khoá muộn nhất có thể, mở sớm nhất có thể.
</details>

#### COD-025 · 🟠 · coding · ⭐ · [→ architecture](../../../08-embedded-systems/architecture.md)
**Parse gói tin 7 byte từ đường truyền bằng cách ép kiểu con trỏ. Sai ở đâu?**
```c
struct Header { uint8_t type; uint32_t len; uint16_t crc; };

uint8_t wire[7] = { 0x01, 0x00,0x00,0x01,0x00, 0xAB,0xCD };
const struct Header *h = (const struct Header *)(wire + 1);
printf("type=%u len=%u crc=%u\n", h->type, h->len, h->crc);
```
<details><summary>Đáp án</summary>

**Hai bug độc lập, phải chỉ ra cả hai:**

① **(a) Padding.** Compiler chèn byte đệm để mỗi thành viên nằm đúng biên alignment của nó: `uint32_t len` phải ở offset chia hết cho 4. Struct **không hề khớp** với 7 byte trên đường truyền.
① **(b) Misaligned access.** `wire + 1` là địa chỉ lẻ; đọc `uint32_t` từ đó là truy cập không thẳng hàng. Trên x86 chỉ chậm; trên ARM/MIPS cấu hình chặt thì **bus fault**. Ngoài ra ép `uint8_t*` sang `struct*` rồi đọc còn vi phạm **strict aliasing**.

② Kết quả là đọc sai hoàn toàn field, và có thể đọc **ngoài mảng** (struct 12 byte đọc từ offset 1 của mảng 7 byte).

③ Chạy thật:
```
sizeof(struct Header) = 12 (wire that su = 7)
offsetof len = 4, crc = 8
type=0 len=3959475627 crc=49992
```
Struct **12 byte** cho 7 byte dữ liệu; `len` đáng lẽ là 1 thì ra 3.959.475.627.

⚠️ **Cảnh báo ở đây phụ thuộc mức tối ưu** (đo thật): `-O0` và `-O1` → **0 warning**; từ `-O2` mới có:
```
warning: array subscript 'const struct Header[0]' is partly outside array bounds of 'uint8_t[7]' [-Warray-bounds]
```
Và nó chỉ xuất hiện vì `wire` là mảng local có kích thước nhìn thấy được — **đổi thành con trỏ tham số thì im hoàn toàn**, tức đúng ca thật khi parse buffer nhận từ driver.

④ Sửa — **đừng ép kiểu, hãy giải mã từng field**:
```c
uint8_t  type =  wire[1];
uint32_t len; memcpy(&len, wire + 2, 4);        /* memcpy: khong misaligned, khong aliasing */
len = le32toh(len);                              /* va PHAI xu ly endianness */
uint16_t crc; memcpy(&crc, wire + 6, 2); crc = le16toh(crc);
```
⭐ `memcpy` ở đây **không tốn gì** — compiler thường sinh đúng một lệnh load. Đây là cách chuẩn để đọc dữ liệu wire-format trong C.

**Follow-up:** *`__attribute__((packed))` có giải quyết được không?* → giải quyết **padding** nhưng **không** giải quyết misaligned: nó chỉ khiến compiler tự sinh code đọc từng byte (chậm hơn), và lấy địa chỉ của member trong struct packed là cái bẫy mới. Nó cũng **không** giải quyết endianness. · *Vì sao `offsetof(crc)` là 8 chứ không phải 6?* → sau `len` (offset 4..7) thì 8 đã thẳng hàng 2 byte, nên `crc` ở 8, rồi struct đệm thêm tới bội số của 4 ⇒ 12. · *Làm sao biết layout thật?* → in `sizeof` và `offsetof`, hoặc `pahole`.
</details>

#### COD-026 · 🟠 · coding · ⭐ · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Hai lời gọi trong một `printf`. In ra gì?**
```cpp
static std::stack<int> q;
static int take() { int v = q.top(); q.pop(); return v; }

int main() {
    q.push(3); q.push(2); q.push(1);      // top = 1
    printf("%d %d\n", take(), take());    // ky vong "1 2"
}
```
<details><summary>Đáp án</summary>

① **Thứ tự đánh giá các đối số của một lời gọi hàm là *unspecified*** — kể cả trong C++17. Hai lần `take()` đều có **side effect** (`q.pop()`), nên kết quả phụ thuộc compiler.

② Không phải UB (C++17 đảm bảo các đối số **không xen kẽ** nhau — *indeterminately sequenced*), nhưng **thứ tự** thì không ai hứa. gcc thường đánh giá **phải sang trái**.

③ Chạy thật với gcc 11.4.0:
```
2 1
```
Kỳ vọng `1 2`, thực tế `2 1`. `-Wall -Wextra`: **không warning.** *(Chỉ xác minh trên gcc — máy này không có clang; điểm mấu chốt đúng là kết quả **được phép** khác nhau giữa các compiler.)*

⭐ **Bug này có thật, không phải câu đố:** hai bản mẫu `COD-012` và `COD-013` trong chính file bank này lần đầu chạy đều **sai output** vì đúng lỗi này trong hàm test — không phải vì cấu trúc dữ liệu sai.

④ Sửa: **tách ra biến có tên**, mỗi side effect một câu lệnh:
```cpp
int a = take();
int b = take();
printf("%d %d\n", a, b);
```

**Follow-up — đây là chỗ C++17 *thật sự* đổi luật, rất đáng biết:** C++17 **đã** quy định thứ tự cho một số toán tử trước đây không có:
- `a << b << c` (stream) — trái sang phải, nên `std::cout << f() << g()` giờ **có** thứ tự xác định;
- `a.b`, `a->b`, `a[b]`, và **assignment** `a = b` (vế **phải** đánh giá trước vế trái);
- `new T(expr)`.

Nhưng **đối số hàm thì KHÔNG** — vẫn unspecified trong C++17 và C++20. Biết được ranh giới "cái nào C++17 đã sửa, cái nào chưa" là tín hiệu bạn đọc chuẩn thật chứ không chỉ nghe tên feature.
</details>

---

## Cách trả lời bài coding (rất quan trọng — review chấm cả điều này)

### Mục A — viết code từ đầu · **5 nhịp, dùng cả khi làm TRÊN GIẤY**

1. **Hỏi spec trước khi đặt bút:** input/output, ràng buộc, edge case (mảng rỗng? null? trùng? in-place hay được cấp thêm? kích thước tối đa?).
2. **Nói approach + O(time)/O(space)** *trước khi* viết dòng đầu tiên.
3. **Viết** — chữ đọc được, tên biến tử tế.
4. ⭐ **Tự dry-run bằng bảng giá trị:** 1 ca thường + 1 ca biên. **Đây là nhịp thay thế compiler khi làm trên giấy** — bỏ nhịp này thì không có gì bắt lỗi giùm bạn.
5. **Tự nêu cái mình biết là chưa xử lý** — thà mình nói ra còn hơn để interviewer tìm thấy.

> 🖊️ **Trên giấy khác gõ máy ở đâu:** không compiler, không autocomplete, không chạy test. Nội dung bài **không** dễ hơn, nhưng **nhịp ① và ⑤ được cân nặng hơn hẳn** — vì đó là bằng chứng duy nhất cho thấy bạn kiểm soát được bài toán khi không có máy xác nhận giùm.
> **Bí thì nói to suy nghĩ**, nêu brute-force trước rồi tối ưu — im lặng là điểm trừ.

### Mục B — đọc code tìm lỗi · **khung 4 phần**

| Phần | Nội dung | Hay bị bỏ sót |
|---|---|---|
| ① | **Dòng nào** sai | — |
| ② | **Vì sao** sai — nêu *cơ chế*, không phải "nó lỗi" | |
| ③ | **Triệu chứng ngoài đời**: crash? sai ngầm? treo? chỉ sai ở `-O2`? | ⚠️ **bỏ sót nhiều nhất** |
| ④ | **Sửa thế nào** + **phát hiện bằng công cụ gì** | ⚠️ vế "công cụ" hay bị quên |

> ⭐ **Trả lời được ③ là chỗ tách người có kinh nghiệm hiện trường với người chỉ đọc sách.** Đo thật trên 13 snippet mục B: `-Wall -Wextra` bắt **3/13 ở `-O0`** và **5/13 ở `-O2`** — **8 bài compile sạch ở mọi mức**, gồm double free, deadlock và đọc ngoài mảng. Vì vậy câu *"lớp bug này phát hiện bằng cách nào"* quan trọng ngang việc chỉ đúng dòng.
>
> **Ba snippet đáng thuộc vì cùng một bài học** — `COD-016` (memcpy chồng lấn), `COD-018` (thiếu `volatile`), `COD-023` (Rule of Three): cả ba **chạy đúng ở `-O0`, hỏng ở `-O2`** hoặc hỏng ngầm. Đó là nguyên mẫu của *"lab thì đúng, hiện trường thì sai"*.

---
⬅️ [Bank index](README.md)
