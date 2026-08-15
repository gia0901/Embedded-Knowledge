# Algorithm Patterns — Nhận diện & áp dụng

> **TL;DR**
> - Đa số bài Leetcode medium thuộc một số **pattern quen thuộc**; nhận ra pattern → biết khung giải, không phải nghĩ từ đầu.
> - **Two pointers / Sliding window**: bài về mảng/chuỗi con liên tục, cặp phần tử.
> - **Hash map**: đếm/tra nhanh, "đã thấy chưa", bù trừ (two-sum).
> - **Binary search**: dữ liệu sắp xếp, hoặc "tìm giá trị nhỏ nhất thỏa điều kiện".
> - **BFS/DFS**: cây, đồ thị, lưới (grid), tìm đường/thành phần liên thông.
> - **Dynamic programming**: bài tối ưu/đếm có cấu trúc con chồng lấp ("số cách", "min/max").
> - Quy trình: làm rõ → ví dụ nhỏ → nhận diện pattern → brute force → tối ưu → phân tích complexity → test edge case.

---

## 1. Vì sao học theo pattern?

Bài medium hiếm khi hoàn toàn mới — chúng là biến thể của vài khuôn. Người giải tốt không "thông minh hơn" mà **nhận ra bài thuộc dạng nào** rồi áp khung tương ứng. Học pattern biến "không biết bắt đầu từ đâu" thành "à, đây là sliding window".

---

## 2. Two Pointers — hai con trỏ

Dùng hai chỉ số duyệt mảng/chuỗi, thường để giảm O(n²) xuống O(n).

**Dấu hiệu**: mảng đã sắp xếp, tìm cặp/bộ ba thỏa điều kiện, đảo ngược, loại trùng tại chỗ.
```cpp
// Two-sum trên mảng ĐÃ sắp xếp: tìm cặp có tổng = target
int l = 0, r = n - 1;
while (l < r) {
    int sum = a[l] + a[r];
    if (sum == target) return {l, r};
    else if (sum < target) ++l;     // cần lớn hơn → dịch trái phải
    else --r;                        // cần nhỏ hơn → dịch phải trái
}
```

---

## 3. Sliding Window — cửa sổ trượt

Two-pointer cho bài về **đoạn con liên tục** (substring/subarray): mở rộng/thu hẹp cửa sổ `[left, right]`.

**Dấu hiệu**: "đoạn con dài nhất/ngắn nhất thỏa...", "tổng/độ dài cửa sổ".
```cpp
// Đoạn con dài nhất không lặp ký tự
int left = 0, best = 0;
std::unordered_set<char> seen;
for (int right = 0; right < n; ++right) {
    while (seen.count(s[right])) { seen.erase(s[left]); ++left; }  // thu hẹp tới khi hợp lệ
    seen.insert(s[right]);
    best = std::max(best, right - left + 1);
}
```
Biến O(n²) (xét mọi đoạn) thành O(n) (mỗi phần tử vào/ra cửa sổ một lần).

---

## 4. Hash Map — đếm & tra cứu

**Dấu hiệu**: "đã xuất hiện chưa", đếm tần suất, tìm phần bù, nhóm theo key.
```cpp
// Two-sum (mảng chưa sắp xếp): O(n) nhờ map
std::unordered_map<int,int> seen;          // value → index
for (int i = 0; i < n; ++i) {
    int need = target - a[i];
    if (seen.count(need)) return {seen[need], i};   // tìm phần bù đã thấy
    seen[a[i]] = i;
}
```
Đổi không gian lấy thời gian: O(n) time, O(n) space thay vì O(n²) brute force.

---

## 5. Binary Search — tìm nhị phân

**Dấu hiệu**: dữ liệu **sắp xếp**, hoặc bài "tìm giá trị nhỏ nhất/lớn nhất thỏa một điều kiện đơn điệu" (binary search on answer).
```cpp
int lo = 0, hi = n - 1;
while (lo <= hi) {
    int mid = lo + (hi - lo) / 2;     // tránh overflow (không (lo+hi)/2)
    if (a[mid] == target) return mid;
    else if (a[mid] < target) lo = mid + 1;
    else hi = mid - 1;
}
return -1;   // không thấy
```
O(log n). Mẹo nâng cao: nhiều bài "tối thiểu hóa X sao cho điều kiện(X) đúng" giải bằng binary search trên miền đáp án.

---

## 6. BFS / DFS — duyệt cây & đồ thị

**Dấu hiệu**: cây, đồ thị, lưới 2D, "tìm đường", "thành phần liên thông", "khoảng cách ngắn nhất (không trọng số)".

- **BFS** (queue): duyệt theo tầng → tìm **đường ngắn nhất** trên đồ thị không trọng số.
- **DFS** (stack/đệ quy): đi sâu → phù hợp tìm đường bất kỳ, đếm thành phần, backtracking.
```cpp
// BFS trên grid (số đảo, đường ngắn nhất...)
std::queue<Cell> q; q.push(start); visited[start] = true;
while (!q.empty()) {
    auto cur = q.front(); q.pop();
    for (auto next : neighbors(cur))
        if (!visited[next]) { visited[next] = true; q.push(next); }
}
```
Lưu ý đánh dấu `visited` để tránh lặp vô tận; DFS đệ quy sâu coi chừng stack overflow.

---

## 7. Dynamic Programming — quy hoạch động

**Dấu hiệu**: "số cách", "min/max chi phí", bài có **cấu trúc con tối ưu** + **bài toán con chồng lấp** (tính lại nhiều lần).

Ý tưởng: lưu kết quả bài con để không tính lại.
```cpp
// Fibonacci: từ O(2ⁿ) đệ quy ngây thơ → O(n) với memoization/bottom-up
int fib(int n) {
    std::vector<int> dp(n + 1);
    dp[0] = 0; dp[1] = 1;
    for (int i = 2; i <= n; ++i) dp[i] = dp[i-1] + dp[i-2];
    return dp[n];
}
```
- **Top-down** (memoization): đệ quy + cache. **Bottom-up** (tabulation): lặp xây từ nhỏ.
- Khó nhất là **định nghĩa state** và **công thức truy hồi** (recurrence). Bắt đầu bằng brute-force đệ quy rồi thêm cache.

---

## 8. Bảng nhận diện nhanh

| Tín hiệu trong đề | Pattern |
|-------------------|---------|
| Mảng sắp xếp, tìm cặp/bộ | Two pointers |
| Đoạn con liên tục dài/ngắn nhất | Sliding window |
| "Đã thấy chưa", đếm, phần bù | Hash map |
| Sắp xếp + tìm, "min X thỏa điều kiện" | Binary search |
| Cây/đồ thị/grid, đường đi, liên thông | BFS/DFS |
| Đường ngắn nhất không trọng số | BFS |
| "Số cách", "min/max", con chồng lấp | Dynamic programming |
| Sinh mọi tổ hợp/hoán vị | Backtracking (DFS) |

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DSA-006](../14-prep/mock-interview/bank/dsa.md) | Khi nào dùng sliding window? Nó cải thiện complexity thế nào? |
| [DSA-005](../14-prep/mock-interview/bank/dsa.md) | Two-sum: giải O(n) thế nào và đánh đổi gì? |
| [DSA-007](../14-prep/mock-interview/bank/dsa.md) | BFS và DFS khác nhau thế nào? Khi nào dùng BFS? |
| [DSA-008](../14-prep/mock-interview/bank/dsa.md) | Dynamic programming áp dụng khi nào? Hai cách hiện thực? |
| [DSA-011](../14-prep/mock-interview/bank/dsa.md) | Trong phỏng vấn coding, quy trình tiếp cận một bài nên thế nào? |

---
⬅️ [complexity-and-structures.md](complexity-and-structures.md) · ➡️ Tiếp theo: [13-networking/](../13-networking/)
