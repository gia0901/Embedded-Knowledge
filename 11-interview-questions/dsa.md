# Câu hỏi phỏng vấn — Data Structures & Algorithms

> Topic gốc: [13 DSA](../13-dsa/). Phần này nên **luyện code thực tế** trên Leetcode song song. Tự trả lời lý thuyết trước khi mở.

---

## 🟢 Cơ bản

<details><summary>1) Big-O là gì? Phân biệt time và space complexity.</summary>

Big-O mô tả tốc độ tăng của tài nguyên theo kích thước input khi input lớn, bỏ qua hằng số/số hạng bậc thấp. Time complexity đo số thao tác; space complexity đo bộ nhớ phụ dùng thêm (gồm cả stack đệ quy). Mặc định phân tích worst case. Nên nêu cả hai vì có thể đánh đổi cho nhau. → [complexity-and-structures](../13-dsa/complexity-and-structures.md)
</details>

<details><summary>2) Khi nào dùng hash map, khi nào dùng map (balanced tree)?</summary>

Hash map (`unordered_map`): tra cứu/chèn/xóa O(1) trung bình, không thứ tự. Tree (`map`/`set`): O(log n), giữ **thứ tự sắp xếp** — duyệt theo thứ tự, tìm min/max, range query. Cần tra cứu thuần → hash map; cần thứ tự/range → tree. → [complexity-and-structures](../13-dsa/complexity-and-structures.md)
</details>

<details><summary>3) Khác biệt giữa array/vector và linked list?</summary>

Vector lưu liền mạch: truy cập O(1) theo index, cache tốt, nhưng chèn/xóa giữa O(n). Linked list: chèn/xóa O(1) nếu có vị trí, nhưng truy cập O(n) và cache kém (node rải rác). Mặc định dùng vector trừ khi cần chèn/xóa nhiều ở vị trí đã biết. → [complexity-and-structures](../13-dsa/complexity-and-structures.md)
</details>

---

## 🟡 Trung bình

<details><summary>4) Amortized O(1) của vector::push_back nghĩa là gì?</summary>

Push_back là O(1) khi còn capacity; khi đầy phải cấp vùng mới (gấp đôi) và copy/move toàn bộ — O(n) lần đó. Nhưng vì capacity tăng cấp số nhân, reallocate ngày càng thưa, nên trung bình qua n lần push_back tổng chi phí O(n), chia đều O(1) mỗi lần — đó là amortized O(1). → [complexity-and-structures](../13-dsa/complexity-and-structures.md)
</details>

<details><summary>5) Two-sum: giải O(n) thế nào?</summary>

Mảng chưa sắp xếp: dùng hash map, duyệt một lần, mỗi phần tử tính phần bù `target - a[i]` và kiểm tra đã có trong map chưa; có thì trả cặp index, chưa thì lưu `a[i]→i`. O(n) time, O(n) space. Mảng đã sắp xếp: two pointers (l đầu, r cuối, dịch theo tổng) O(n) time O(1) space. → [algorithm-patterns](../13-dsa/algorithm-patterns.md)
</details>

<details><summary>6) Sliding window dùng khi nào? Cải thiện complexity ra sao?</summary>

Cho bài về đoạn con liên tục (substring/subarray) — "đoạn dài/ngắn nhất thỏa điều kiện". Duy trì cửa sổ [left, right], mở rộng right và thu hẹp left khi vi phạm điều kiện. Mỗi phần tử vào/ra cửa sổ một lần → O(n), cải thiện từ O(n²) của cách xét mọi đoạn. → [algorithm-patterns](../13-dsa/algorithm-patterns.md)
</details>

<details><summary>7) BFS và DFS khác nhau? Khi nào dùng BFS?</summary>

BFS duyệt theo tầng (queue) — thăm node gần trước. DFS đi sâu rồi quay lui (stack/đệ quy). Dùng BFS khi cần **đường ngắn nhất trên đồ thị không trọng số** (thăm theo khoảng cách tăng dần) hoặc duyệt theo tầng; DFS cho tìm đường bất kỳ, thành phần liên thông, phát hiện chu trình, backtracking. Cả hai cần đánh dấu visited. → [algorithm-patterns](../13-dsa/algorithm-patterns.md)
</details>

---

## 🟠 Khó

<details><summary>8) Dynamic programming áp dụng khi nào? Hai cách hiện thực?</summary>

Áp dụng cho bài tối ưu/đếm có cấu trúc con tối ưu + bài toán con chồng lấp (tính lại nhiều lần) — dấu hiệu "số cách", "min/max". Lưu kết quả bài con để không tính lại, giảm mũ xuống đa thức. Top-down (memoization): đệ quy + cache. Bottom-up (tabulation): lặp xây từ nhỏ. Khó nhất là định nghĩa state + recurrence; bắt đầu từ brute-force đệ quy rồi thêm cache. → [algorithm-patterns](../13-dsa/algorithm-patterns.md)
</details>

<details><summary>9) Vì sao vector thường nhanh hơn list dù cùng O(n)?</summary>

Big-O bỏ qua chi phí truy cập bộ nhớ thực tế. Vector liền mạch → prefetch hiệu quả, phần lớn cache hit. List node rải rác trên heap → mỗi bước duyệt thường cache miss (đắt hơn hit hàng chục–trăm lần). Nên vector nhanh hơn nhiều lần trong thực tế. Lý do thực dụng ưu tiên cấu trúc liền mạch, đặc biệt embedded/hot path. → [complexity-and-structures](../13-dsa/complexity-and-structures.md)
</details>

<details><summary>10) Cho một mảng, làm sao nhận biết nên dùng pattern nào?</summary>

Dựa vào tín hiệu trong đề: mảng đã sắp xếp + tìm cặp/bộ → two pointers; "min X thỏa điều kiện đơn điệu" hoặc sắp xếp + tìm → binary search; đoạn con liên tục dài/ngắn nhất → sliding window; "đã thấy chưa"/đếm/phần bù → hash map; cây/đồ thị/grid/đường đi → BFS/DFS; "số cách"/"min-max" với con chồng lấp → DP; sinh mọi tổ hợp/hoán vị → backtracking. Nhận diện pattern biến "không biết bắt đầu từ đâu" thành áp khung quen thuộc. → [algorithm-patterns](../13-dsa/algorithm-patterns.md)
</details>

---

## 🔴 Senior

<details><summary>11) Quy trình tiếp cận một bài coding interview nên thế nào?</summary>

Làm rõ trước: ràng buộc (kích thước input, miền giá trị, trùng/âm, yêu cầu time/space), xác nhận I/O bằng ví dụ nhỏ. Nhận diện pattern. Trình bày brute force trước (baseline đúng, thể hiện hiểu bài) rồi tối ưu, nêu đánh đổi. Phân tích complexity (time + space). Test edge case (rỗng, một phần tử, trùng, biên). Quan trọng: **think aloud** — nói ra suy nghĩ và lý do, vì người phỏng vấn chấm cách tiếp cận chứ không chỉ đáp án. → [algorithm-patterns](../13-dsa/algorithm-patterns.md), [problem-solving](../10-thinking/problem-solving.md)
</details>

<details><summary>12) Khi nào tối ưu space quan trọng hơn time? Liên hệ embedded.</summary>

Khi chạy trên hệ RAM hạn chế (embedded, MCU vài KB–MB) hoặc xử lý dữ liệu cực lớn không vừa bộ nhớ. Một thuật toán O(n) time nhưng O(n) space phụ có thể bất khả thi trên thiết bị ít RAM, trong khi bản O(n log n) time nhưng O(1) space lại chạy được. Trên embedded còn tránh cấp phát động (fragmentation, không tất định) — ưu tiên giải pháp in-place hoặc dùng bộ nhớ tĩnh có giới hạn. Vì vậy phải hỏi rõ ràng buộc tài nguyên trước khi chọn thuật toán; "tốt nhất" phụ thuộc ngữ cảnh. → [complexity-and-structures](../13-dsa/complexity-and-structures.md), [constraints](../08-embedded-systems/constraints.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
