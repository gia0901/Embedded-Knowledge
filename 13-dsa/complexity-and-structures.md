# Complexity & Data Structures

> **TL;DR**
> - **Big-O** mô tả độ tăng của thời gian/bộ nhớ theo kích thước input ở **worst case** — để so sánh thuật toán độc lập phần cứng.
> - Thứ tự thường gặp (tốt→xấu): O(1) < O(log n) < O(n) < O(n log n) < O(n²) < O(2ⁿ) < O(n!).
> - Chọn cấu trúc dữ liệu theo **thao tác chủ đạo**: tra cứu nhanh → hash map; giữ thứ tự + tra cứu → balanced tree (map); LIFO/FIFO → stack/queue; truy cập ngẫu nhiên → array/vector.
> - **Cache locality** quan trọng thực tế: `vector` (liền mạch) thường nhanh hơn `list` (rải rác) dù cùng O(n) — đặc biệt trên embedded.
> - Luôn nêu **cả time lẫn space complexity**.

---

## 1. Big-O — phân tích độ phức tạp

Big-O mô tả thời gian/bộ nhớ tăng thế nào khi input lớn dần, bỏ qua hằng số và số hạng bậc thấp → so sánh bản chất thuật toán, không phụ thuộc máy.

| Big-O | Tên | Ví dụ |
|-------|-----|-------|
| O(1) | Hằng số | Truy cập mảng theo index, hash lookup (trung bình) |
| O(log n) | Logarit | Binary search, thao tác trên balanced BST |
| O(n) | Tuyến tính | Duyệt mảng một lần |
| O(n log n) | Linearithmic | Sort hiệu quả (merge/heap/intro sort) |
| O(n²) | Bậc hai | Hai vòng lặp lồng (so từng cặp) |
| O(2ⁿ), O(n!) | Mũ, giai thừa | Brute force tổ hợp/hoán vị (cần tối ưu) |

- Phân tích **worst case** là mặc định; đôi khi nói thêm average/amortized.
- **Amortized**: chi phí trung bình qua nhiều thao tác — vd `vector::push_back` là O(1) amortized (thỉnh thoảng reallocate O(n) nhưng phân bổ ra thì O(1)).
- **Space complexity**: bộ nhớ phụ dùng thêm (kể cả stack đệ quy) — đừng quên nêu.

---

## 2. Cấu trúc dữ liệu cốt lõi & độ phức tạp

| Cấu trúc | Truy cập | Tìm | Chèn | Xóa | Ghi chú |
|----------|----------|-----|------|-----|---------|
| **Array/`vector`** | O(1) | O(n) | O(n)* | O(n) | Liền mạch, cache tốt; `push_back` O(1) amortized |
| **Linked list** | O(n) | O(n) | O(1)** | O(1)** | Chèn/xóa rẻ nếu đã có vị trí; cache kém |
| **Hash map** (`unordered_map`) | — | O(1) avg | O(1) avg | O(1) avg | Worst O(n) khi nhiều collision; không thứ tự |
| **Balanced BST** (`map`/`set`) | — | O(log n) | O(log n) | O(log n) | Giữ thứ tự sắp xếp, duyệt theo thứ tự |
| **Stack** | — | — | O(1) | O(1) | LIFO |
| **Queue / Deque** | — | — | O(1) | O(1) | FIFO / hai đầu |
| **Heap** (`priority_queue`) | O(1) xem top | — | O(log n) | O(log n) pop | Lấy min/max nhanh |

\* chèn cuối vector là O(1) amortized; chèn giữa O(n). \** với linked list, cần con trỏ tới vị trí; tìm vị trí vẫn O(n).

---

## 3. Chọn cấu trúc theo thao tác chủ đạo

Hỏi: **thao tác nào diễn ra nhiều nhất?** rồi chọn cấu trúc tối ưu cho nó.

- Tra cứu theo key nhanh, không cần thứ tự → **hash map** (`unordered_map`).
- Cần giữ **thứ tự sắp xếp** + tra cứu/range query → **balanced tree** (`map`/`set`).
- Truy cập ngẫu nhiên theo index, duyệt tuần tự, cache-friendly → **array/`vector`**.
- Chèn/xóa nhiều ở đầu/cuối → **deque**; LIFO → **stack**; FIFO → **queue**.
- Luôn lấy phần tử lớn/nhỏ nhất → **heap** (`priority_queue`).
- Cần "đã thấy chưa" / loại trùng → **hash set**.

---

## 4. STL container — bản đồ nhanh (C++)

| Nhu cầu | Container |
|---------|-----------|
| Mảng động | `std::vector` (mặc định cho hầu hết) |
| Mảng cố định compile-time | `std::array` (embedded: không heap) |
| Tra cứu key→value nhanh | `std::unordered_map` |
| Key→value có thứ tự | `std::map` |
| Tập phần tử duy nhất | `std::unordered_set` / `std::set` |
| Hai đầu | `std::deque` |
| Stack/Queue | `std::stack` / `std::queue` (adapter) |
| Hàng đợi ưu tiên | `std::priority_queue` |

> **Mặc định dùng `std::vector`** trừ khi có lý do rõ — nhờ cache locality nó thường thắng `list`/`map` trong thực tế dù lý thuyết tương đương. Trên embedded, `std::array`/vùng tĩnh để tránh heap ([constraints](../08-embedded-systems/constraints.md)).

---

## 5. Cache locality — yếu tố thực tế Big-O bỏ qua

Big-O coi mọi truy cập bộ nhớ là bằng nhau, nhưng thực tế **cache miss** đắt hơn cache hit hàng chục–trăm lần. `vector` lưu liền mạch → duyệt rất nhanh (prefetch, ít miss); `list`/cây trỏ rải rác → mỗi bước có thể cache miss. Vì vậy `vector` thường nhanh hơn `list` cho duyệt/chèn-cuối dù cùng O(n). Đây là lý do thực dụng để ưu tiên cấu trúc liền mạch — đặc biệt quan trọng trên embedded/hot path. Liên hệ [03/memory-management](../03-operating-system/memory-management.md).

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Big-O là gì? Phân biệt time và space complexity.</summary>

Big-O mô tả tốc độ tăng của tài nguyên (thời gian hoặc bộ nhớ) theo kích thước input khi input lớn dần, bỏ qua hằng số và số hạng bậc thấp — cho phép so sánh bản chất thuật toán độc lập với phần cứng. Time complexity đo số thao tác cơ bản theo input; space complexity đo bộ nhớ **phụ** dùng thêm (không tính input), bao gồm cả stack của đệ quy. Hai cái có thể đánh đổi cho nhau (vd dùng thêm bộ nhớ để tăng tốc, như memoization). Mặc định phân tích worst case; đôi khi nêu thêm average hoặc amortized. Khi trả lời nên nêu cả hai, vì một thuật toán nhanh nhưng tốn O(n) bộ nhớ phụ có thể không phù hợp cho hệ hạn chế RAM.
</details>

<details><summary>2) Amortized O(1) của vector::push_back nghĩa là gì?</summary>

`vector` lưu phần tử trong một vùng nhớ liền mạch với dung lượng (capacity) nhất định. Khi push_back mà còn chỗ thì chỉ là gán — O(1). Khi đầy, nó cấp vùng mới lớn hơn (thường gấp đôi) và copy/move toàn bộ phần tử cũ sang — O(n) cho lần đó. Nhưng vì capacity tăng theo cấp số nhân, các lần reallocate ngày càng thưa, nên **tính trung bình qua nhiều lần push_back**, chi phí mỗi lần vẫn là hằng số — đó là "amortized O(1)". Nói cách khác, tổng chi phí của n lần push_back là O(n), chia đều ra O(1) mỗi lần, dù một số lần lẻ tẻ tốn O(n).
</details>

<details><summary>3) Khi nào dùng hash map, khi nào dùng balanced tree (map)?</summary>

Dùng hash map (`unordered_map`) khi cần tra cứu/chèn/xóa theo key cực nhanh — O(1) trung bình — và **không** cần thứ tự. Dùng balanced tree (`map`/`set`) khi cần **giữ thứ tự sắp xếp** của key: duyệt theo thứ tự, tìm phần tử nhỏ nhất/lớn nhất, hoặc range query (tìm các key trong khoảng) — các thao tác này là O(log n) và hash map không hỗ trợ hiệu quả. Đánh đổi: hash map nhanh hơn về trung bình nhưng worst case O(n) khi nhiều collision và không có thứ tự; tree ổn định O(log n) và có thứ tự nhưng chậm hơn về hằng số và locality kém hơn. Nếu chỉ cần tra cứu thuần thì hash map; nếu cần thứ tự/range thì tree.
</details>

<details><summary>4) Vì sao vector thường nhanh hơn list dù cùng O(n)?</summary>

Vì Big-O bỏ qua chi phí thực tế của truy cập bộ nhớ, trong khi cache locality tạo khác biệt lớn. `vector` lưu phần tử liền mạch trong bộ nhớ, nên khi duyệt, CPU prefetch hiệu quả và phần lớn truy cập là cache hit. `list` lưu mỗi node rải rác trên heap, nối bằng con trỏ, nên mỗi bước duyệt thường gây cache miss — đắt hơn cache hit hàng chục đến hàng trăm lần. Kết quả là duyệt hay chèn-cuối trên `vector` thường nhanh hơn `list` nhiều lần dù lý thuyết cùng O(n). Đây là lý do thực dụng nên mặc định dùng `vector` (hoặc cấu trúc liền mạch) trừ khi cần chèn/xóa giữa nhiều ở vị trí đã biết — đặc biệt quan trọng trên embedded và hot path.
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [algorithm-patterns.md](algorithm-patterns.md)
