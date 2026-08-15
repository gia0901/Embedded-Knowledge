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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DSA-001](../14-prep/mock-interview/bank/dsa.md) | Big-O là gì? Phân biệt time và space complexity. |
| [DSA-004](../14-prep/mock-interview/bank/dsa.md) | Amortized O(1) của vector::push_back nghĩa là gì? |
| [DSA-002](../14-prep/mock-interview/bank/dsa.md) | Khi nào dùng hash map, khi nào dùng balanced tree (map)? |
| [DSA-009](../14-prep/mock-interview/bank/dsa.md) | Vì sao vector thường nhanh hơn list dù cùng O(n)? |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [algorithm-patterns.md](algorithm-patterns.md)
