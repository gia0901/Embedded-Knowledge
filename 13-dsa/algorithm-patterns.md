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

<details><summary>1) Khi nào dùng sliding window? Nó cải thiện complexity thế nào?</summary>

Dùng sliding window cho các bài về **đoạn con liên tục** (substring/subarray) — ví dụ "đoạn con dài nhất/ngắn nhất thỏa một điều kiện", tổng cửa sổ, đếm trong cửa sổ. Ý tưởng là duy trì một cửa sổ `[left, right]` và trượt: mở rộng `right` để thêm phần tử, thu hẹp `left` khi cửa sổ vi phạm điều kiện. Vì mỗi phần tử chỉ vào và ra khỏi cửa sổ đúng một lần, tổng công việc là O(n) — cải thiện từ O(n²) của cách xét mọi đoạn con bằng hai vòng lồng. Đây là biến thể của two pointers áp dụng cho đoạn liên tục.
</details>

<details><summary>2) Two-sum: giải O(n) thế nào và đánh đổi gì?</summary>

Với mảng chưa sắp xếp, dùng hash map: duyệt một lần, với mỗi phần tử `a[i]` tính phần bù `need = target - a[i]` và kiểm tra `need` đã có trong map chưa; nếu có thì trả về cặp index, nếu chưa thì lưu `a[i] → i` vào map. Độ phức tạp O(n) thời gian, O(n) bộ nhớ — đánh đổi thêm không gian (map) để giảm thời gian từ O(n²) của brute force hai vòng lặp. Nếu mảng **đã sắp xếp**, có thể dùng two pointers (l ở đầu, r ở cuối, dịch theo tổng so với target) đạt O(n) thời gian mà chỉ O(1) bộ nhớ phụ — tốt hơn về không gian nhưng yêu cầu mảng sắp xếp.
</details>

<details><summary>3) BFS và DFS khác nhau thế nào? Khi nào dùng BFS?</summary>

BFS (Breadth-First Search) duyệt theo tầng dùng một queue — thăm tất cả node ở khoảng cách k trước khi sang k+1. DFS (Depth-First Search) đi sâu hết một nhánh rồi quay lui, dùng stack hoặc đệ quy. Dùng **BFS khi cần đường đi ngắn nhất trên đồ thị không trọng số** (vì nó thăm node theo thứ tự khoảng cách tăng dần, lần đầu chạm đích là ngắn nhất) hoặc duyệt theo tầng. Dùng DFS cho tìm đường bất kỳ, đếm/khám phá thành phần liên thông, phát hiện chu trình, và backtracking (sinh tổ hợp/hoán vị). Cả hai đều cần đánh dấu visited để tránh lặp vô tận; DFS đệ quy trên đồ thị sâu cần coi chừng stack overflow (có thể chuyển sang stack tường minh).
</details>

<details><summary>4) Dynamic programming áp dụng khi nào? Hai cách hiện thực?</summary>

DP áp dụng cho bài tối ưu hoặc đếm có hai đặc điểm: **cấu trúc con tối ưu** (lời giải tối ưu của bài lớn dựa trên lời giải tối ưu của bài con) và **bài toán con chồng lấp** (cùng một bài con được tính lại nhiều lần) — dấu hiệu đề thường là "số cách", "min/max chi phí". Ý tưởng cốt lõi là lưu kết quả bài con để không tính lại, giảm độ phức tạp mũ xuống đa thức. Hai cách hiện thực: **top-down (memoization)** — viết đệ quy tự nhiên rồi cache kết quả mỗi state; **bottom-up (tabulation)** — lặp xây kết quả từ bài con nhỏ nhất lên. Phần khó nhất là định nghĩa state và công thức truy hồi (recurrence); cách thực tế là bắt đầu từ lời giải brute-force đệ quy, nhận ra các bài con lặp lại, rồi thêm cache hoặc chuyển thành bảng.
</details>

<details><summary>5) Trong phỏng vấn coding, quy trình tiếp cận một bài nên thế nào?</summary>

Làm rõ trước: hỏi về ràng buộc (kích thước input, miền giá trị, có trùng/âm không, yêu cầu thời gian/không gian), xác nhận input/output bằng một ví dụ nhỏ. Sau đó **nhận diện pattern** (mảng sắp xếp → two pointers/binary search; đoạn con → sliding window; đồ thị → BFS/DFS; "số cách/min-max" → DP...). Trình bày **brute force** trước để có baseline đúng và thể hiện hiểu bài, rồi tối ưu dần, nêu rõ đánh đổi. Phân tích complexity (time + space) cho giải pháp chọn. Cuối cùng kiểm tra **edge case** (rỗng, một phần tử, trùng, biên). Quan trọng là **think aloud** — nói ra suy nghĩ và lý do, vì người phỏng vấn chấm cách tiếp cận chứ không chỉ đáp án.
</details>

---
⬅️ [complexity-and-structures.md](complexity-and-structures.md) · ➡️ Tiếp theo: [14-networking/](../14-networking/)
