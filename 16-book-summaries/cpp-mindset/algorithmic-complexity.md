# C++ Mindset — Ch. 3: Algorithmic Thinking and Complexity (tr. 61–81) ✅ đã đọc sâu toàn văn

> Thuộc [The C++ Programmer's Mindset](README.md). Nguồn: đọc trực tiếp PDF, trang sách = trang PDF − 25.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> Ý đồ chương (tr. 61): hai phần của việc phát triển thuật toán — *"designing a first algorithm"* (thường không tối ưu, nhưng cần thiết) rồi *"chase down performance bottlenecks"*. Chương này lo phần thiết kế + phân tích; phần "hiểu máy để tối ưu" chuyển sang ch. 4.

---

## Cụm 1 — Tính đúng đắn: chứng minh bằng quy nạp (tr. 62–65)

### Nội dung chính

**Việc đầu tiên khi có thuật toán mới là kiểm tra correctness (tr. 62):** *"does the algorithm actually deliver what it says it does?"* Với thuật toán chạy trên dữ liệu **kích thước cố định** thì chứng minh trực tiếp; với dữ liệu **kích thước thay đổi** cần **mathematical induction (quy nạp toán học)**. Direct proof = chuỗi mệnh đề logic, mỗi cái là hệ quả của cái trước hoặc của **theorem** (định lý đã được chứng minh).

**Quy nạp — cơ chế (tr. 63):** có predicate P(n) (đúng/sai cho mỗi số nguyên không âm n). Muốn kết luận P(n) đúng với **mọi** n, chỉ cần hai điều:
1. **P(0) đúng** — gọi là **base case**.
2. Nếu P(k) đúng với k ≥ 0 bất kỳ, thì **P(k+1) cũng đúng** — gọi là **inductive step**; giả thiết "P(k) đúng" là **inductive hypothesis**.

Cơ chế lan truyền: P(0) → P(1) → P(2) → ... đúng cho mọi n. Base case không nhất thiết ở 0 (có thể 1 hoặc 2 — khi đó mệnh đề đúng cho mọi n ≥ base). Ví dụ toán kinh điển sách dùng (tr. 63–64): chứng minh tổng n số nguyên dương đầu tiên = n(n+1)/2.

**Áp vào thuật toán (tr. 64):** predicate thường là *"thuật toán cho output đúng với input độ dài n"*. Ví dụ sách — **sequential search** (tìm giá trị trong list n phần tử):
- **Base case:** list 1 phần tử — chỉ cần kiểm tra phần tử đó có phải target không; đúng dù thế nào.
- **Inductive step:** giả sử đúng với độ dài k. Với độ dài k+1: chạy thuật toán trên k phần tử đầu (theo giả thiết đã đúng — hoặc tìm thấy rồi, hoặc chưa), rồi kiểm phần tử thứ (k+1). Hoặc khớp → trả về k+1, hoặc "not found". Thuật toán hoàn tất đúng.

Biến thể quan trọng (tr. 65): đôi khi cần **strong induction** — giả thiết P đúng với **mọi** k ≤ n (không chỉ n) rồi chứng minh cho n+1; *"This is especially useful for recursive algorithms."*

### Insight đáng nhớ

- Sách nối quy nạp với thiết kế thuật toán theo một mapping cụ thể (tr. 64): **base case của quy nạp ↔ trường hợp nhỏ nhất của thuật toán; inductive step ↔ "giả sử đúng cho phần nhỏ hơn, chứng minh cho phần lớn hơn"**. 🆕 Đây chính là lý do quy nạp và đệ quy "sinh đôi": cấu trúc chứng minh quy nạp khớp 1-1 với cấu trúc thuật toán đệ quy (base case + recursive case) — nắm một cái là viết được cái kia.
- Lời khuyên phương pháp neo vào ví dụ search (tr. 64): *"sometimes the details need to be written down. When learning a new technique, it is best to start by writing down simple examples"* — ngay cả thuật toán "hiển nhiên đúng" như linear search, viết chứng minh ra vẫn là bài tập rèn tư duy đáng làm.

---

## Cụm 2 — Complexity: Big-O, quan hệ, và amortized (tr. 65–68)

### Nội dung chính

**Complexity là gì (tr. 65):** thước đo **thời gian chạy tăng thế nào theo kích thước input** — tính toán lý thuyết, độc lập phần cứng/cài đặt; là phương tiện chính để so sánh hiệu quả các thuật toán cùng mục tiêu. Ghi bằng **big-O**: số hạng *"dominates the theoretical execution time for large enough n"*. Bảng ví dụ của sách (tr. 65):

| Complexity | Ý nghĩa | Ví dụ sách |
|---|---|---|
| **O(1)** hằng | không phụ thuộc kích thước input | index-based lookup vào array (⚠️ sách lưu ý: *"the ideal; it's actually not physically possible to achieve this complexity, at least with current technology"*) |
| **O(log n)** logarit | | binary search vào sequence có thứ tự — tận dụng ordering chia đôi mỗi bước |
| **O(n)** tuyến tính | tỉ lệ kích thước input | linear search (`find`) — worst case thăm mọi phần tử |
| **O(n log n)** | | thuật toán sort tốt nhất (worst case) |
| **O(n²)** bậc hai | | insertion sort |

**Định nghĩa hình thức (tr. 66):** A(n) có complexity O(g(n)) nếu tồn tại hằng c > 0 và số nguyên dương n₀ sao cho T(A(n)) ≤ c·g(n) với mọi n > n₀. Ba điểm sách nhấn:
- Chỉ quan tâm **asymptotic behavior** (n lớn) — hành vi ở n nhỏ bị che bởi các chi phí khác; hằng c có thể lớn nhưng không đổi kết luận ở n lớn.
- Đây là **upper bound / worst case** — nhiều thuật toán có average hoặc best case tốt hơn.
- **Chỉ báo số hạng trội nhất** (tr. 66): O(n² + n) và O(n²) là **cùng một complexity**. Nếu có 2 tham số kích thước: tính trung bình n vector mỗi vector k thành phần → O(nk); nhưng nếu k không quan trọng trong ngữ cảnh thì *"the dependence on k is consumed by the constant"*.

**Quan hệ giữa các lớp (tr. 66–67):** thứ tự tăng dần: **O(1) < O(log n) < O(n) < O(nʳ) đa thức bậc r < O(2ⁿ) mũ**. Sách đặt câu hỏi định vị hay: log nằm đâu? — *"logarithmic complexity comes next [after constant], as it sits between constant and linear complexity O(n)"*. Về mũ: *"these algorithms have running times that grow so rapidly that they tend to be impractical"* (tr. 66). Figure 3.1 (tr. 67) vẽ 5 đường O(1), O(log N), O(N), O(N log N), O(N²) trên trục log — trực quan hóa khoảng cách khủng khiếp ở N lớn.

**Amortized complexity (tr. 67–68)** — phần đắt nhất cụm, ví dụ chuẩn `std::vector::push_back`:
- Đa số lần push_back là O(1) (tăng con trỏ end + construct). Nhưng khi hết chỗ đã cấp → **realloc + copy/move toàn bộ** sang buffer mới (thường gấp đôi) = O(n) cho lần đó.
- Vì buffer **nhân đôi mỗi lần**, các lần realloc thưa dần khi số phần tử tăng → gọi là **amortized constant**.
- Sách chứng minh bằng số (tr. 68): chi phí lần chèn thứ i là `i` nếu i là lũy thừa của 2, ngược lại là 1. Tổng n lần chèn:

```
Σᵢ cᵢ = n + Σⱼ 2ʲ   (j từ 0 đến ⌊log₂ n⌋)
      = n + (2^(⌊log₂n⌋+1) − 1)/(2−1)
      < 3n
```

→ tổng chi phí n lần chèn < 3n → chi phí amortized = 3n/n = **3, hằng số!** *(⚠️ Lưu ý đọc kỹ: chi phí realloc lần thứ j là 2ʲ, và realloc xảy ra khi i = 2ʲ — sách gộp phần "n" (mỗi lần chèn tốn ≥1) với tổng các lần realloc.)*

### Insight đáng nhớ

- Chứng minh amortized của push_back (tr. 68) neo vào một con số nhớ được: **tổng < 3n → mỗi phần tử tốn trung bình < 3 thao tác**, dù có những lần đơn lẻ tốn O(n). 🆕 Đây là câu trả lời chuẩn cho câu phỏng vấn "vector.push_back là O(1) hay O(n)?": *cả hai* — worst case một lần là O(n), nhưng amortized qua N lần là O(1), và lý do nằm ở **chiến lược nhân đôi** (nếu tăng tuyến tính +k mỗi lần thì amortized thành O(n) — realloc quá thường).
- Nguyên tắc "chỉ giữ số hạng trội" (tr. 66) kèm ví dụ O(n²+n)=O(n²) là điểm hay bị hiểu sai: 🆕 nó **không** có nghĩa "số hạng nhỏ vô hại trong thực tế" — chỉ có nghĩa "ở n đủ lớn thì số hạng lớn thắng". Với n nhỏ (rất thường gặp ở embedded — vài chục phần tử), hằng số và số hạng phụ có thể quyết định — lý do sách dành hẳn ch. 4 cho "hiểu máy" thay vì chỉ đếm big-O.

---

## Cụm 3 — Thiết kế thuật toán: iterative, recursive, divide & conquer (tr. 68–74)

### Nội dung chính

**Triết lý mở đầu (tr. 68–69):** ưu tiên ghép từ thuật toán chuẩn — *"don't reinvent the wheel"*; nhưng không phải bài nào cũng là ca của bài đã biết → phải biết tự thiết kế. Ghi chú thực tế quan trọng (tr. 69): nếu thiết kế thuật toán thấy *"obvious"* thì tốt; nếu **không** obvious *"that is perhaps an indication that you have not found a good abstraction yet — and the algorithm almost writes itself."* Và cảnh báo về việc giảm complexity: nhân ma trận n×n từng được tin là O(n³) mãi đến **Strassen** (divide & conquer, ~O(n^2.8)) — *"the product of many years of study, and resulted in a relatively modest improvement. The real gains come from ensuring one works with the hardware"* (tr. 69) → dẫn thẳng sang ch. 4.

**Iterative (tr. 69–70):** xử lý từng item tuần tự. `std::min_element`/`max_element` là ví dụ. Ưu: rất đơn giản. Nhược: *"Even assuming each processing step only has constant complexity, an iterative algorithm will have at least linear complexity"* (tr. 69). Khái niệm cốt lõi: **invariant (bất biến)** — điều kiện được duy trì qua mỗi vòng lặp, là "nguyên liệu" để chứng minh tính đúng bằng quy nạp. Ví dụ max_element của ch. 1: invariant = *"m ≥ mọi phần tử đã thấy tới giờ"*; thao tác swap m với current lớn hơn giữ đúng invariant → kết thúc thì m = max (tr. 70). Câu đắt: *"Thinking about invariants is like abstraction. Once you find the 'right' invariant for your problem, it makes writing the algorithm substantially easier."*

**Recursive (tr. 70–72):** hai phần — **base case** (không chia nhỏ nữa) + **chia bài toán và đệ quy vào các phần**. Ví dụ tuyệt: tính lũy thừa xⁿ.

Bản iterative — O(n):

```cpp
template <typename T>
T power(T x, unsigned n) {
    if (n == 0) { return T(1); }
    T result = x;
    while (--n > 0) {
        result *= x;
    }
    return result;
}
```

Bản recursive divide & conquer — **O(log n)**:

```cpp
template <typename T>
T power(T x, unsigned n) {
    if (n == 0) { return T(1); }
    if (n == 1) { return x; }
    const auto pow2 = power(x, n / 2);          // tính MỘT LẦN, dùng LẠI
    T result = (n % 2) == 0 ? T(1) : x;         // n lẻ thì nhân thêm x
    return result * pow2 * pow2;                // xⁿ = (x^(n/2))² [× x nếu lẻ]
}
```

Sách giải thích độ phức tạp (tr. 71): mỗi lần chia đôi n → cần log₂(n) lời gọi, mỗi lời gọi ≤ 3 phép nhân (hằng) → **O(log n)**. Chìa khóa: tính `power(x, n/2)` **một lần** rồi bình phương, thay vì tính hai lần.

Góc C++ (tr. 71–72): đệ quy dùng được để **tính lúc compile time** qua `constexpr` function (C++11 chỉ cho đệ quy trong constexpr; chuẩn mới cho cả loop) — nhưng *"inefficient compile-time computations increase the build time of your code substantially."* Và cái giá runtime (tr. 72): **tail-call optimization** đôi khi cứu (compiler cuốn call stack thành assembly hiệu quả) *"but this can only be applied in certain situations"*; không thì mỗi lời gọi thêm một stack frame — *"in extreme cases... can lead to... stack overflows."*

**Divide and conquer (tr. 72–74):** ba bước — **divide** (chia thành các ca nhỏ của cùng bài), **process/conquer** (giải từng ca bằng đệ quy), **combine** (ghép). Finiteness được bảo đảm vì *"decreasing sequences of positive integers terminate after a finite number of steps"* (tr. 72). Ví dụ std: `std::upper_bound`, `std::lower_bound` (binary search).

Phân tích qua **master method / master theorem (tr. 72–73):** thời gian chạy biểu diễn bằng recurrence:

```
T(n) = a·T(n/b) + f(n)      với a > 0, b > 1
```

Sách không phát biểu đầy đủ định lý (tránh rườm rà) nhưng nêu: complexity thường do f(n) quyết định; trường hợp đặc biệt f(n) so được với n^(log_b a) thì T(n) ~ n^(log_b a)·log₂ n. Với ví dụ power: a=1, b=2, f(n) hằng → O(log₂ n).

Ví dụ kinh điển — **quicksort (tr. 73–74)**, có code đầy đủ:

```cpp
template <typename T>
std::ptrdiff_t partition(std::span<T> data) {
    auto& pivot_value = data.back();            // chọn phần tử cuối làm pivot
    std::ptrdiff_t pivot_pos = 0;
    for (auto j = 0; j < data.size() - 1; ++j) {
        if (data[j] <= pivot_value) {
            std::swap(data[pivot_value], data[j]);   // ⚠️ LỖI IN: phải là data[pivot_pos]
            ++pivot_pos;
        }
    }
    std::swap(data[pivot_pos], pivot_value);
    return pivot_pos;
}

template <typename T>
void quicksort(std::span<T> data) {
    if (data.empty()) { return; }
    auto pivot = partition(data);
    quicksort(data.subspan(0, pivot));          // đệ quy nửa trái pivot
    quicksort(data.subspan(pivot + 1));         // đệ quy nửa phải pivot
}
```

⚠️ **Code sách có lỗi in (tr. 73) — phát hiện khi đối chiếu:** trong vòng lặp, `std::swap(data[pivot_value], data[j])` sai — `pivot_value` là *giá trị* (reference tới `data.back()`), dùng nó làm **chỉ số** là sai; phải là `std::swap(data[pivot_pos], data[j])`. Ý định đúng của Lomuto partition: dồn các phần tử ≤ pivot về đầu tại vị trí `pivot_pos`. Đặc điểm sách nêu đúng (tr. 74): quicksort **không có bước combine** — *"Once both of the conquer steps have been completed, there is no further work to be done"*; công việc chính nằm ở divide (partition vừa tìm chỗ chia vừa sơ bộ sắp xếp). Chọn pivot là phần tử cuối chỉ là một lựa chọn — *"One could select any other element (or even select one at random) without changing the characteristics of the algorithm."*

### Insight đáng nhớ

- Cặp `power` iterative O(n) vs recursive O(log n) (tr. 71) là minh chứng gọn nhất sách cho *"thinking in terms of recursion opens up alternative means of splitting the data that can lead to more efficient algorithms"* (tr. 70): cùng bài toán, đổi cách **chia** (chia đôi mũ + tái dùng kết quả bình phương) đổi hẳn lớp complexity. 🆕 Đây là "exponentiation by squaring" — pattern lặp lại ở modular exponentiation (crypto), ma trận lũy thừa (tính Fibonacci O(log n)).
- Khái niệm **invariant** (tr. 70) là sợi dây nối cả chương: nó vừa là công cụ *chứng minh* (cụm 1 — quy nạp cần invariant), vừa là công cụ *thiết kế* (tìm được invariant đúng thì thuật toán "gần như tự viết"). Câu *"thinking about invariants is like abstraction"* móc ngược về ch. 1–2.
- ⚠️ Hai lỗi in trong code chương này (partition swap sai chỉ số ở tr. 73; và ở cụm 2 các chi tiết nhỏ) — tái khẳng định thói quen đọc code sách phản biện. Bản quicksort dùng `std::span` + `subspan` là cách viết hiện đại đáng học (không truyền cặp iterator/index thủ công).

---

## Cụm 4 — Các họ thuật toán: graph, optimization, DP, randomized (tr. 74–78)

### Nội dung chính

**Graph (tr. 74–75):** graph G = (V vertices, E edges nối các đỉnh). Thuật ngữ: **path** (chuỗi cạnh nối đỉnh này tới đỉnh khác), **cycle** (path bắt đầu và kết thúc cùng đỉnh), **acyclic** (không có cycle), **tree** (đồ thị acyclic mà mọi đỉnh nối nhau bằng path). Cạnh mang **direction** và/hoặc **weight** (khoảng cách, capacity, cost...). Bài kinh điển: **shortest path** giữa hai đỉnh (kể cả có weight) → **Dijkstra's algorithm**.

Chiến lược tổng quát sách nhấn (tr. 74):
- Nhiều thuật toán graph là **greedy** — chọn đỉnh/cạnh theo một đại lượng số (vd chọn đỉnh có degree lớn nhất; degree = số cạnh vào/ra đỉnh đó). Dijkstra thuộc loại này.
- Duyệt đồ thị hai cách: **breadth-first** (BFS — duyệt hết các đỉnh kề rồi mới đi sâu) vs **depth-first** (DFS — theo một path đến tận cùng rồi backtrack). *"The different strategies have different uses, depending on the needs of the problem."*
- Graph còn dùng để **lý luận về thuật toán** (tr. 75): nhiều tiến trình mô hình hóa được thành **computation tree** — ví dụ `power` chính là dựng cây tính toán rồi "rebalance để mỗi nhánh mang nhiều foliage hơn". Kết hợp **heuristic** → cắt cả nhánh khỏi cây công việc nếu biết nó không dẫn tới lời giải.

**Optimization (tr. 75–77):** maximize/minimize một **objective function** (đại diện cho cost, rate of return...). Hai lớp:
- **Linear** — objective + ràng buộc đều tuyến tính; giải bằng **simplex method**: các bất đẳng thức tuyến tính định nghĩa một **feasible region** hình simplex (giao các nửa-mặt-phẳng), và vì objective tuyến tính nên nghiệm tối ưu nằm ở **một đỉnh** của region.
- **Non-linear** — objective phi tuyến, phức tạp hơn. Biết đạo hàm → **gradient descent**; không biết/không tồn tại đạo hàm → **Nelder-Mead** (derivative-free, đổi lại kém hiệu quả hơn).
- Tối ưu trên cây → **branch and bound**: dùng heuristic đoán nhánh nào chứa nghiệm tối ưu, **prune** (cắt) không gian tìm kiếm còn lại → giảm complexity bằng cách thu nhỏ search space.

**Gradient descent chi tiết (tr. 76):** đi "xuống dốc" theo hướng dốc nhất = **âm của gradient**; kích thước bước = **learning rate**. Nhược điểm lớn: kẹt ở **local minima** (đáy cục bộ, không phải toàn cục). Ví dụ số của sách: f(x,y)=x²+y², min ở (0,0). Gradient ∇f=(2x,2y), bắt đầu (1,2), learning rate 1/4 → bước theo (−2x,−2y):

```
(1,2) → (1 − ½, 2 − 1) = (½, 1) → (¼, ½) → ... → hội tụ về (0,0) nhưng KHÔNG BAO GIỜ chạm đúng
```

Sách nhấn (tr. 76): *"This process never actually reaches (0,0), but the sequence of iterations does converge to it... it's not usual that a gradient descent eventually lands on the minimum value."* Khi không biết gradient tường minh: ước lượng đạo hàm — một cách phổ biến trong ML là *"tagging data and tracking the basic operations"* (🆕 = automatic differentiation / autograd).

**Dynamic programming (tr. 77):** giống computational thinking — chẻ bài thành phần nhỏ; **khác biệt: các subproblem CHỒNG LẤN nhau** (và thường cùng dạng với bài gốc). Lợi ích đến từ **memoize** — nhớ lời giải subproblem nhỏ để tái dùng. Hai điều kiện để DP hiệu quả:
1. **Optimal substructure** — lời giải tối ưu của bài = tổng hợp lời giải tối ưu của các subproblem.
2. **Overlapping subproblems** — subproblem càng chồng lấn, memoize càng lời.

Sách cảnh báo tinh tế (tr. 77): *"finding the shortest path... can be solved using dynamic programming, but finding the longest path might not"* — không phải bài decompose được nào cũng có optimal substructure; và ví dụ shortest path: đường tối ưu tổng thể **không** nhất thiết chứa subpath tối ưu vì thêm cạnh đầu có thể đổi lựa chọn tốt nhất.

**Randomized (tr. 77–78):** hai cách dùng ngẫu nhiên —
1. **Phân tích average complexity:** nếu biết phân bố dữ liệu, tính được complexity trung bình thay vì chỉ worst case. Ví dụ sort: worst case là dãy đảo ngược, nhưng đó chỉ là 1 trong n! cấu hình → dãy ngẫu nhiên gần như luôn tốt hơn worst case (tr. 77).
2. **Chủ động inject randomness** để cải thiện hiệu năng/tránh worst case — ví dụ **stochastic gradient descent** (SGD) rất phổ biến trong ML: dùng ngẫu nhiên (nội tại hoặc inject) để cải thiện hội tụ (tr. 78).

### Insight đáng nhớ

- Ba ánh xạ "bài thực tế → thuật toán chuẩn" của ch. 1 (dependency=digraph, scheduling=coloring, routing=shortest path) giờ được cụm này cung cấp *bộ công cụ* để giải: greedy (Dijkstra), BFS/DFS, branch-and-bound. 🆕 Đáng nhớ nhất là **greedy vs DP**: cả hai chẻ bài, nhưng greedy chọn cục bộ tối ưu và không nhìn lại, DP nhớ và tái dùng — và không phải bài nào cũng chịu greedy (longest path) hay DP (thiếu optimal substructure).
- Cảnh báo gradient descent *"never actually reaches the minimum... converges to it"* (tr. 76) neo vào ví dụ số (½,1)→(¼,½) là bài học thực hành: 🆕 tối ưu bằng phương pháp lặp cần **tiêu chí dừng** (tol — như `find_root_bisect` ch. 2 tr. 49 đã dùng `std::abs(fm) < tol`), không có "chạm đúng đáy". Đây là lý do numerical code luôn có epsilon.

---

## Cụm 5 — Hiểu máy để tối ưu: cache, SIMD, branch prediction (tr. 78–80)

> Cụm này là "trailer" cho ch. 4 — sách nêu ba đòn bẩy hiệu năng ở mức *thiết kế thuật toán*, chi tiết phần cứng để dành ch. 4.

### Nội dung chính

**Nguyên tắc số 1 (tr. 78):** dùng công cụ chuẩn đã tối ưu (kể cả ngoài stdlib) — *"[they] will almost always perform better than a hand-crafted solution."* Và nhắc lại về đệ quy: C++ *"is not a functional-first programming language"* — recursive call tốn overhead (dựng stack); trông cậy tail-call optimization hoặc inline, hoặc chấp nhận vì trong divide & conquer số lời gọi nhỏ so với tổng công việc (amortized).

**Cache-aware algorithms (tr. 78–79):** CPU rất nhanh → phần lớn thời gian là **chờ dữ liệu từ memory**. Cache nằm giữa CPU và main memory, gần CPU, truy cập nhanh hơn nhiều. CPU/OS/chương trình **prefetch** dữ liệu dự đoán sẽ cần → *"Making algorithms that access data in a predictable way can help facilitate this pre-fetching."* Ví dụ chuẩn — **nhân ma trận (tr. 79):** ma trận lưu **row-major** (hàng này sau hàng kia trong khối liền), nên truy cập **cột** = nhảy xa trong bộ nhớ → thảm họa cache. Giải pháp tối ưu: *"ensure that data is loaded from main memory as little as possible, so we make use of each element as much as possible before it is evicted from the cache"* — lý do **BLAS** (Basic Linear Algebra Subsystem) dùng thuật toán **"blocked"** thân thiện cache.

**SIMD — vectorized operations (tr. 79–80):** phần cứng chuyên biệt thao tác **nhiều số cùng lúc** — Single Instruction Multiple Data. Mọi CPU x86-64 hiện đại có ít nhất **SSE2** (vector 128-bit = 2 double hoặc 4 float); nhiều CPU có **AVX2** (256-bit) hoặc **AVX512** (512-bit — gấp đôi/bốn throughput). Compiler tự auto-vectorize vòng lặp phù hợp. Cách thiết kế để vectorize được (tr. 79):
- Dữ liệu **liền kề trong bộ nhớ, truy cập theo thứ tự từ index 0**.
- ⚠️ Chi tiết thực hành đắt giá: *"Even using the iterators of std::vector can be enough to prevent vectorization. Using std::ptrdiff_t integers as indices and accessing data directly (using something such as std::span) is generally the best way to ensure that the compiler recognizes the opportunity."*
- **Aliasing (tr. 80):** hai reference/pointer trỏ vùng nhớ chồng lấn → compiler **không dám** tối ưu mạnh (không đảm bảo được thứ tự thao tác). C có `restrict` để hứa "không alias"; **C++ không có `restrict`**. Cách xoay: dữ liệu trong hai `std::vector` khác nhau không thể alias (mỗi vector tự quản storage riêng) → span lấy từ hai vector đó cũng không alias — nhưng phải giữ chuỗi lý luận đủ đơn giản để compiler suy ra được.

**Branch prediction & speculative execution (tr. 80):** CPU đoán nhánh nào của conditional sẽ chạy, prefetch instruction/data của nhánh đó (**branch prediction**); **speculative execution** đi xa hơn — chạy **cả hai nhánh** song song rồi bỏ kết quả sai. Điểm cần nhớ:
- Conditional trong code thường (không hot path) → vô hại về hiệu năng.
- Chỗ quan trọng là **tight loop** — *"a small piece of code that is executed on large blocks of data... every nanosecond counts, so the accumulated cost of many branching instructions will have a large cumulative effect."* Nếu không tránh được conditional, speculative execution giúp *"but only if each branch is sufficiently simple."*
- ⚠️ Cảnh báo bảo mật (tr. 80): speculative execution là nguồn *"severe vulnerabilities a few years ago"* (🆕 = Spectre/Meltdown) — với code nhạy bảo mật phải cân nhắc rủi ro này.

### Insight đáng nhớ

- Thông điệp trung tâm chương neo vào ví dụ Strassen (tr. 69) + matrix/BLAS (tr. 79): **"real gains come from working with the hardware"** — sau nhiều năm nghiên cứu, cải tiến *complexity* nhân ma trận chỉ từ O(n³) xuống O(n^2.8), nhưng cùng một O(n³) mà viết **cache-aware + SIMD** thì nhanh hơn hàng chục lần. 🆕 Đây là lý do sách xếp ch. 4 ("Understanding the Machine") ngay sau chương complexity: big-O là điều kiện cần, machine sympathy là điều kiện đủ.
- Mẹo thực hành *"span + ptrdiff_t index thay vì vector iterator để không chặn vectorization"* (tr. 79) là loại chi tiết chỉ người viết HPC thật mới biết — 🆕 rất trúng nghề embedded/HPC: cùng một vòng lặp, đổi cách truy cập (iterator → index trực tiếp trên span) có thể bật/tắt auto-vectorization, đổi throughput 4-8× mà big-O không đổi. Đáng đo bằng `-fopt-info-vec` (gcc) / `-Rpass=loop-vectorize` (clang).

---

## Góc interview

**Câu 1 (🎯):** `std::vector::push_back` là O(1) hay O(n)? Giải thích amortized và vì sao chiến lược nhân đôi lại quan trọng.

<details><summary>Đáp án</summary>

- **Cả hai, tùy góc nhìn** — một lần push_back đơn lẻ có thể là O(n) (khi hết capacity phải realloc + copy/move toàn bộ n phần tử sang buffer mới), nhưng **amortized qua N lần push_back là O(1)** (tr. 67–68).
- Cơ chế amortized: vì buffer **nhân đôi** mỗi lần realloc, các lần realloc thưa dần khi vector lớn lên. Chứng minh của sách (tr. 68): chi phí lần chèn thứ i là i nếu i là lũy thừa của 2 (có realloc), ngược lại là 1. Tổng N lần: Σ = n + Σ 2ʲ (j đến ⌊log₂n⌋) < **3n** → trung bình < 3 thao tác/phần tử → hằng số.
- **Vì sao nhân đôi (chứ không +k tuyến tính):** nếu mỗi lần đầy chỉ tăng thêm hằng số k phần tử, thì realloc xảy ra ~n/k lần, mỗi lần copy O(n) → tổng O(n²) → amortized O(n) (tệ). Nhân đôi làm số lần realloc chỉ log₂(n) → tổng O(n) → amortized O(1). 🆕 (Hệ số nhân không nhất thiết là 2 — nhiều implementation dùng 1.5 để tái dùng bộ nhớ đã free tốt hơn; miễn là hằng số > 1 thì amortized vẫn O(1).)
- Điểm cộng: nêu hệ quả thực hành — nếu biết trước số phần tử, gọi `reserve()` để tránh mọi realloc (như ví dụ grid signal ch. 2 tr. 44 dùng `signals.reserve(dim_x*dim_y)`); và cảnh báo iterator invalidation khi realloc (con trỏ/iterator vào vector cũ thành dangling — nối [OSTEP](../ostep/virtualization-memory.md) về realloc).

</details>

**Câu 2:** Cùng độ phức tạp O(n³), vì sao một cài đặt nhân ma trận có thể nhanh hơn cài đặt khác hàng chục lần? Nêu các đòn bẩy.

<details><summary>Đáp án</summary>

- Big-O chỉ đo **số thao tác asymptotic**, không đo **chi phí mỗi thao tác** — mà trên CPU hiện đại, chi phí thật bị chi phối bởi việc **chờ dữ liệu từ memory** (tr. 78). Cùng O(n³), thứ khác nhau là *"machine sympathy"*:
  1. **Cache locality (tr. 79):** ma trận lưu row-major → truy cập cột = nhảy xa trong bộ nhớ → cache miss liên tục. Thuật toán **blocked/tiled** (như BLAS dùng) load từng khối nhỏ vừa cache, tái dùng tối đa trước khi bị evict → giảm số lần load main memory.
  2. **SIMD (tr. 79):** tổ chức dữ liệu liền kề + truy cập tuần tự để compiler auto-vectorize (SSE2/AVX2/AVX512 xử lý 2-8 double/lệnh) → throughput gấp nhiều lần; dùng span + index thay vì iterator để không chặn vectorization.
  3. **Branch prediction (tr. 80):** giảm conditional trong tight loop.
- Bằng chứng lịch sử của sách (tr. 69): sau nhiều năm nghiên cứu, Strassen chỉ giảm complexity O(n³)→O(n^2.8) ("modest improvement"), trong khi *"the real gains come from ensuring one works with the hardware"* — tối ưu phần cứng cho lãi lớn hơn cải tiến big-O.
- 🆕 Nghề embedded/HPC: đây là lý do đo bằng profiler (perf — ch. 15) thay vì chỉ đếm big-O; cache miss và vectorization ratio là metric thật. Big-O là điều kiện cần (chọn đúng lớp thuật toán), machine sympathy là điều kiện đủ (hằng số nhỏ).

</details>

**Câu 3:** Phân biệt divide-and-conquer, dynamic programming, và greedy. Khi nào dùng cái nào?

<details><summary>Đáp án</summary>

- **Divide & conquer (tr. 72):** chia bài thành các ca **độc lập** của cùng bài, giải riêng, combine. Điều kiện: các phần *"independent of one another"*. Ví dụ: quicksort, mergesort, binary search, exponentiation by squaring. Complexity phân tích bằng master theorem T(n)=a·T(n/b)+f(n).
- **Dynamic programming (tr. 77):** cũng chia nhỏ, nhưng subproblem **CHỒNG LẤN** → **memoize** để tái dùng. Hai điều kiện: **optimal substructure** (nghiệm tối ưu = tổng hợp nghiệm tối ưu subproblem) + **overlapping subproblems** (càng chồng càng lời). Ví dụ: shortest path, chuỗi con chung dài nhất. Cảnh báo sách (tr. 77): không phải bài decompose được nào cũng có optimal substructure — *"finding the shortest path can... but finding the longest path might not."*
- **Greedy (tr. 74):** chọn cục bộ tối ưu tại mỗi bước theo một đại lượng số, **không nhìn lại**. Ví dụ: Dijkstra (chọn đỉnh gần nhất chưa thăm). Nhanh nhưng chỉ đúng khi bài có tính chất greedy-choice (tối ưu cục bộ dẫn tới tối ưu toàn cục).
- **Chọn thế nào:** subproblem độc lập → D&C; subproblem chồng lấn + optimal substructure → DP (memoize); chọn cục bộ chứng minh được là tối ưu toàn cục → greedy (nhanh nhất). 🆕 Quan hệ: D&C là "chia không nhớ", DP là "chia có nhớ" (thêm memoization vào D&C khi subproblem lặp lại), greedy là "không chia, chọn tham lam từng bước". Nếu greedy sai thì lùi về DP; nếu subproblem không lặp thì DP thừa, dùng D&C.
- Điểm cộng: đề cập branch-and-bound (tr. 75) — DP/D&C trên cây + heuristic prune nhánh vô vọng; và invariant (tr. 70) là công cụ chứng minh tính đúng của cả ba.

</details>

### Đọc thêm (tùy chọn)

- [abstraction-in-detail.md](abstraction-in-detail.md) — ch. 2, cụm functions (find_root_bisect với tol) và standard algorithms là nền của chương này.
- [OSTEP virtualization-memory.md](../ostep/virtualization-memory.md) — cache/TLB/paging: nền phần cứng của cụm 5, và ch. 4 sắp tới sẽ đào sâu.
- [13-dsa/](../../13-dsa/) — Big-O, cấu trúc dữ liệu, pattern giải thuật của repo.
- Sách gốc dẫn tham khảo: Cormen et al. *Introduction to Algorithms* (4th ed) và Knuth *The Art of Computer Programming* (tr. 62, 81).
