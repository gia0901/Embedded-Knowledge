# COD — Bài coding tại chỗ

> Domain `COD`. Từ [technical_round/02 PHẦN 7](../../technical_round/02_question_bank.md). Mỗi bài: *ý tưởng → code → độ phức tạp*. Track dùng: `dsa`, `cpp-system`, `bsp`, `cpp-mindset`.
> **Khi mock (`coding` type):** interviewer ra đề, ứng viên **viết code vào [../coding-arena/](../coding-arena/)** (mỗi bài 1 file). Review: đúng/chạy, độ phức tạp, edge case, style. Đáp án dưới là *bản mẫu tham chiếu* — che lại khi tự luyện.

---

#### COD-001 · 🟡 · coding · ⭐ · [→ algorithm-patterns](../../../13-dsa/algorithm-patterns.md)
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
**O(n) time, O(1) space.** *(Follow-up: bản đệ quy.)*
</details>

#### COD-002 · 🟡 · coding · ⭐ · [→ algorithm-patterns](../../../13-dsa/algorithm-patterns.md)
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

#### COD-003 · 🟡 · coding · ⭐ · [→ algorithm-patterns](../../../13-dsa/algorithm-patterns.md)
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

#### COD-004 · 🟡 · coding · [→ algorithm-patterns](../../../13-dsa/algorithm-patterns.md)
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

#### COD-006 · 🟡 · coding · ⭐ · [→ ring-buffer.md](../../../13-dsa/ring-buffer.md)
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

**Follow-up theo tầng** — [ring-buffer.md](../../../13-dsa/ring-buffer.md): ① chính sách khi đầy (§4) → ② luồng byte, `memcpy` hai khối khi vắt biên (§5) → ③ mutex + condvar có `close()` (§6) → ④ lock-free SPSC: chỉ số chạy tự do + mask, **hai** cặp release/acquire, `alignas(64)` chống false sharing (§7) → ⑤ nó nằm ở đâu trong Linux thật: `dmesg`, pipe, `kfifo`, ALSA, `io_uring` (§8).
</details>

#### COD-007 · 🟢 · coding · [→ algorithm-patterns](../../../13-dsa/algorithm-patterns.md)
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

#### COD-008 · 🟡 · coding · [→ algorithm-patterns](../../../13-dsa/algorithm-patterns.md)
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

#### COD-009 · 🟢 · coding · [→ complexity-and-structures](../../../13-dsa/complexity-and-structures.md)
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

---

## Cách trả lời bài coding (rất quan trọng — review chấm cả điều này)
1. **Làm rõ đề trước khi code:** input/output, ràng buộc, edge case (mảng rỗng? null? trùng?).
2. **Nói ý tưởng trước, code sau:** interviewer chấm cách tư duy, không chỉ code chạy.
3. **Nêu độ phức tạp** time & space sau khi xong.
4. **Tự test edge case:** rỗng, 1 phần tử, biên.
5. **Bí thì nói to suy nghĩ** + nêu brute-force trước rồi tối ưu — im lặng là điểm trừ.

---
⬅️ [Bank index](README.md)
