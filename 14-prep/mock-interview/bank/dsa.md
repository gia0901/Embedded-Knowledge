# DSA — Data Structures & Algorithms

> Domain `DSA`. Nên luyện code thực tế trên Leetcode song song ([COD-*](coding.md)). Track dùng: `dsa`, `cpp-mindset`, `cpp-system`.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Độ phức tạp & container | 6 |
| **B** | Mẫu giải thuật | 6 |
| **C** | Ring buffer | 4 |

---

## A — Độ phức tạp & container

#### DSA-001 · 🟢 · concept · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Big-O là gì? Phân biệt time và space complexity.**
<details><summary>Đáp án</summary>

Big-O mô tả tốc độ tăng của tài nguyên theo kích thước input khi input lớn, bỏ qua hằng số/số hạng bậc thấp. Time complexity đo số thao tác; space complexity đo bộ nhớ phụ dùng thêm (gồm cả stack đệ quy). Mặc định phân tích worst case. Nên nêu cả hai vì có thể đánh đổi cho nhau.
</details>

#### DSA-002 · 🟢 · concept · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Khi nào dùng hash map, khi nào dùng map (balanced tree)?**
<details><summary>Đáp án</summary>

Hash map (`unordered_map`): tra cứu/chèn/xóa O(1) trung bình, không thứ tự. Tree (`map`/`set`): O(log n), giữ **thứ tự sắp xếp** — duyệt theo thứ tự, tìm min/max, range query. Cần tra cứu thuần → hash map; cần thứ tự/range → tree.
</details>

#### DSA-003 · 🟢 · concept · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Khác biệt giữa array/vector và linked list?**
<details><summary>Đáp án</summary>

Vector lưu liền mạch: truy cập O(1) theo index, cache tốt, nhưng chèn/xóa giữa O(n). Linked list: chèn/xóa O(1) nếu có vị trí, nhưng truy cập O(n) và cache kém (node rải rác). Mặc định dùng vector trừ khi cần chèn/xóa nhiều ở vị trí đã biết.
</details>

#### DSA-004 · 🟡 · concept · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Amortized O(1) của vector::push_back nghĩa là gì?**
<details><summary>Đáp án</summary>

Push_back là O(1) khi còn capacity; khi đầy phải cấp vùng mới (gấp đôi) và copy/move toàn bộ — O(n) lần đó. Nhưng vì capacity tăng cấp số nhân, reallocate ngày càng thưa, nên trung bình qua n lần push_back tổng chi phí O(n), chia đều O(1) mỗi lần — đó là amortized O(1).
</details>

#### DSA-009 · 🟠 · concept · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md)
**Vì sao vector thường nhanh hơn list dù cùng O(n)?**
<details><summary>Đáp án</summary>

Big-O bỏ qua chi phí truy cập bộ nhớ thực tế. Vector liền mạch → prefetch hiệu quả, phần lớn cache hit. List node rải rác trên heap → mỗi bước duyệt thường cache miss (đắt hơn hit hàng chục–trăm lần). Nên vector nhanh hơn nhiều lần trong thực tế — lý do thực dụng ưu tiên cấu trúc liền mạch, đặc biệt embedded/hot path.
</details>

#### DSA-012 · 🔴 · concept · [→ complexity-and-structures](../../../12-dsa/complexity-and-structures.md), [constraints](../../../08-embedded-systems/constraints.md)
**Khi nào tối ưu space quan trọng hơn time? Liên hệ embedded.**
<details><summary>Đáp án</summary>

Khi chạy trên hệ RAM hạn chế (embedded, MCU vài KB–MB) hoặc xử lý dữ liệu cực lớn không vừa bộ nhớ. Một thuật toán O(n) time nhưng O(n) space phụ có thể bất khả thi trên thiết bị ít RAM, trong khi bản O(n log n) time nhưng O(1) space lại chạy được. Trên embedded còn tránh cấp phát động (fragmentation, không tất định) — ưu tiên in-place hoặc bộ nhớ tĩnh có giới hạn. Phải hỏi rõ ràng buộc tài nguyên trước khi chọn thuật toán; "tốt nhất" phụ thuộc ngữ cảnh.
</details>

---

## B — Mẫu giải thuật

#### DSA-005 · 🟡 · concept · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Two-sum: giải O(n) thế nào?**
<details><summary>Đáp án</summary>

Mảng chưa sắp xếp: dùng hash map, duyệt một lần, mỗi phần tử tính phần bù `target - a[i]` và kiểm tra đã có trong map chưa; có thì trả cặp index, chưa thì lưu `a[i]→i`. O(n) time, O(n) space. Mảng đã sắp xếp: two pointers (l đầu, r cuối, dịch theo tổng) O(n) time O(1) space.
</details>

#### DSA-006 · 🟡 · concept · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Sliding window dùng khi nào? Cải thiện complexity ra sao?**
<details><summary>Đáp án</summary>

Cho bài về đoạn con liên tục (substring/subarray) — "đoạn dài/ngắn nhất thỏa điều kiện". Duy trì cửa sổ [left, right], mở rộng right và thu hẹp left khi vi phạm điều kiện. Mỗi phần tử vào/ra cửa sổ một lần → O(n), cải thiện từ O(n²) của cách xét mọi đoạn.
</details>

#### DSA-007 · 🟡 · concept · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**BFS và DFS khác nhau? Khi nào dùng BFS?**
<details><summary>Đáp án</summary>

BFS duyệt theo tầng (queue) — thăm node gần trước. DFS đi sâu rồi quay lui (stack/đệ quy). Dùng BFS khi cần **đường ngắn nhất trên đồ thị không trọng số** hoặc duyệt theo tầng; DFS cho tìm đường bất kỳ, thành phần liên thông, phát hiện chu trình, backtracking. Cả hai cần đánh dấu visited.
</details>

#### DSA-008 · 🟠 · concept · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Dynamic programming áp dụng khi nào? Hai cách hiện thực?**
<details><summary>Đáp án</summary>

Áp dụng cho bài tối ưu/đếm có cấu trúc con tối ưu + bài toán con chồng lấp (tính lại nhiều lần) — dấu hiệu "số cách", "min/max". Lưu kết quả bài con để không tính lại, giảm mũ xuống đa thức. Top-down (memoization): đệ quy + cache. Bottom-up (tabulation): lặp xây từ nhỏ. Khó nhất là định nghĩa state + recurrence; bắt đầu từ brute-force đệ quy rồi thêm cache.
</details>

#### DSA-010 · 🟠 · concept · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md)
**Cho một mảng, làm sao nhận biết nên dùng pattern nào?**
<details><summary>Đáp án</summary>

Dựa tín hiệu trong đề: mảng đã sắp xếp + tìm cặp/bộ → two pointers; "min X thỏa điều kiện đơn điệu" hoặc sắp xếp + tìm → binary search; đoạn con liên tục dài/ngắn nhất → sliding window; "đã thấy chưa"/đếm/phần bù → hash map; cây/đồ thị/grid/đường đi → BFS/DFS; "số cách"/"min-max" con chồng lấp → DP; sinh mọi tổ hợp/hoán vị → backtracking. Nhận diện pattern biến "không biết bắt đầu từ đâu" thành áp khung quen thuộc.
</details>

#### DSA-011 · 🔴 · concept · [→ algorithm-patterns](../../../12-dsa/algorithm-patterns.md), [problem-solving](../../../10-thinking/problem-solving.md)
**Quy trình tiếp cận một bài coding interview nên thế nào?**
<details><summary>Đáp án</summary>

Làm rõ trước: ràng buộc (kích thước input, miền giá trị, trùng/âm, yêu cầu time/space), xác nhận I/O bằng ví dụ nhỏ. Nhận diện pattern. Trình bày brute force trước (baseline đúng) rồi tối ưu, nêu đánh đổi. Phân tích complexity (time + space). Test edge case (rỗng, một phần tử, trùng, biên). Quan trọng: **think aloud** — nói ra suy nghĩ và lý do, vì người phỏng vấn chấm cách tiếp cận chứ không chỉ đáp án.
</details>

---

## C — Ring buffer

#### DSA-015 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ ring-buffer](../../../12-dsa/ring-buffer.md)
**Ring buffer: làm sao phân biệt "đầy" với "rỗng" khi cả hai đều cho `head == tail`?**
<details><summary>Đáp án</summary>

**Vấn đề:** với hai chỉ số `head`/`tail` chạy vòng, trạng thái **rỗng** và **đầy** đều dẫn tới `head == tail` ⇒ không phân biệt được.

**Ba cách giải — mỗi cách một đánh đổi:**

| Cách | Làm thế nào | Đánh đổi |
|---|---|---|
| **① Bỏ trống một ô** | Coi là đầy khi `(head+1) % N == tail` | Đơn giản nhất, **không cần biến thêm** ⇒ hợp lock-free. Mất **một ô** sức chứa |
| **② Giữ biến `count`** | Đếm số phần tử đang có | Rõ ràng, dùng hết N ô. Nhưng `count` là **biến chung cả hai bên cùng sửa** ⇒ **phá vỡ SPSC lock-free** ([DSA-016](dsa.md)) |
| **③ Chỉ số chạy tự do** | `head`/`tail` **không** lấy dư, chỉ tăng mãi; số phần tử = `head - tail`; lấy dư **chỉ khi truy cập mảng** | Dùng hết N ô **và** giữ được lock-free. Phải để ý **tràn số** — an toàn nếu N là luỹ thừa của 2 và dùng số nguyên không dấu |

**⭐ Chọn thế nào:**
- Một luồng, hoặc đã có mutex ⇒ **② `count`** vì dễ đọc nhất.
- **SPSC lock-free** ⇒ **① hoặc ③**, vì mỗi biến chỉ có **một** bên ghi.
- ③ là cách các cài đặt nghiêm túc hay dùng (kể cả trong kernel), vì được cả sức chứa lẫn tính lock-free.

⚠️ **Vì sao ② phá lock-free:** `count` bị **cả producer lẫn consumer cùng ghi** ⇒ phải nguyên tử hoá và đồng bộ ⇒ mất đúng tính chất *"mỗi biến một người ghi"* vốn là nền tảng của SPSC.

⚠️ **Bẫy tràn số ở ③:** nếu `head`/`tail` là số nguyên **không dấu** và N là luỹ thừa của 2 thì phép trừ `head - tail` **vẫn đúng** khi tràn (số học modulo) — nhưng dùng số **có dấu** thì tràn là **UB**. Đây là chi tiết dễ sai.

**Chốt:** *"Ba cách: bỏ trống một ô, giữ biến đếm, hoặc chỉ số chạy tự do. Biến đếm dễ đọc nhất nhưng bị cả hai bên ghi nên phá lock-free — vì thế cài đặt SPSC dùng cách bỏ ô hoặc chỉ số tự do."*
</details>

#### DSA-013 · 🟡 · concept · ⭐ · 🎤 2026-08-09 · [→ ring-buffer](../../../12-dsa/ring-buffer.md)
**Ring buffer sức chứa N — bạn chọn N bằng cách nào? Cho một phép tính cụ thể.**
<details><summary>Đáp án</summary>

N **không** phải số tròn chọn cảm tính. Công thức xuất phát: **N ≥ tốc độ producer × thời gian consumer vắng mặt lâu nhất**, rồi nhân hệ số an toàn.

Ví dụ: cảm biến 1000 mẫu/giây, consumer thỉnh thoảng bận 5 ms → tối thiểu `1000 × 0.005 = 5` mẫu. Chọn N = 64 hay 128 là đã dư nhiều lần; chọn 1000 (= 1 giây buffer) chỉ hợp lý nếu có **ràng buộc khác** biện minh — ví dụ consumer xử lý **theo lô 1000 mẫu**, lúc đó buffer phải chứa nổi trọn một lô.

Ba thứ phải nói kèm:
1. **Cái giá bằng RAM**: `N × sizeof(T)`. Với `T` = 128 byte và N = 1000 → **128 000 byte ≈ 125 KB** (không phải MB — sai đơn vị ở đây là lỗi chết người trên thiết bị nhúng).
2. **Làm tròn lên luỹ thừa của 2** nếu định dùng `& (N-1)` thay `% N`, hoặc dùng chỉ số chạy tự do.
3. **Ghi lại lý do** cạnh hằng số. "N = 1024 cho chắc" là chỗ để bug ngồi chờ: người sau không biết được tăng hay giảm.

⚠️ Buffer to hơn **không** sửa được tình trạng consumer chậm hơn producer **về trung bình** — nó chỉ mua thêm thời gian cho **burst**. Nếu tốc độ trung bình không kịp, mọi N đều đầy, chỉ là sớm hay muộn.
</details>

#### DSA-014 · 🟠 · concept · ⭐ · 🎤 2026-08-09 · [→ ring-buffer](../../../12-dsa/ring-buffer.md)
**Ring buffer chọn chính sách "đè cái cũ nhất". Consumer làm sao biết mình vừa mất dữ liệu?**
<details><summary>Đáp án</summary>

Tự nó **không biết** — và đó chính là mối nguy: hệ chạy êm trong khi dữ liệu bốc hơi. Chính sách đè cũ **bắt buộc** đi kèm cơ chế báo:

1. **Một bộ đếm `dropped_count`** tăng mỗi lần đè, **đưa ra ngoài** (log định kỳ / metric / `sysfs` / trường trong struct trả về). Đây là bắt buộc, không phải tuỳ chọn. `dmesg` và `perf` đều làm — `perf` in thẳng "*n events lost*".
2. **Không** in log ngay tại chỗ đè: ở 1000 mẫu/giây với consumer treo, đó là 1000 dòng log/giây, và nếu in trong vùng giữ khoá thì I/O chậm sẽ **chặn luôn producer**. Đếm trong critical section, xuất ngoài critical section, theo chu kỳ.
3. **Giá trị của bộ đếm là để consumer HÀNH ĐỘNG**, không chỉ để điều tra sau: consumer đọc `dropped_count`, thấy tăng thì biết **lô đang gom dở đã có lỗ** → vứt lô đó thay vì tính toán trên dữ liệu đứt quãng.

Chi tiết đáng nói thêm: nội dung **còn lại** trong buffer luôn liên tục (vì luôn vứt từ đầu cũ) — cái đứt quãng là **lô mà consumer gom vắt qua giai đoạn drop**. Nếu `push` là hàm public, cho nó **trả về được trạng thái drop** để caller biết ngay, đừng luôn trả `true`.

Liên hệ: DSA-013 (chọn N), [COD-006](coding.md) (cài đặt).
</details>

#### DSA-016 · 🔴 · concept · 🔺T3 · 📦 2026-08-13 · [→ ring-buffer](../../../12-dsa/ring-buffer.md)
**SPSC lock-free ring buffer hoạt động thế nào? Vì sao điều kiện "một producer, một consumer" lại quan trọng đến vậy?**

> 🔺 **Câu T3** — chỉ tính điểm ở phiên `deep-dive` ([config §6](../config.md)). Ở phiên thường, không biết vẫn đạt 4 nếu nắm chắc bản mutex ([COD-006](coding.md)).

<details><summary>Đáp án</summary>

**Ý tưởng cốt lõi: MỖI BIẾN CHỈ CÓ MỘT NGƯỜI GHI.**

| Biến | Ai **ghi** | Ai **đọc** |
|---|---|---|
| `head` (vị trí ghi) | **Chỉ producer** | Cả hai |
| `tail` (vị trí đọc) | **Chỉ consumer** | Cả hai |

⇒ Không bao giờ có hai bên cùng ghi một ô ⇒ **không cần khoá loại trừ**. Đây là toàn bộ lý do nó lock-free được — và cũng là lý do **thêm một producer thứ hai là hỏng ngay**: hai producer cùng ghi `head` ⇒ cần thao tác nguyên tử kiểu so-sánh-và-đổi, tức là đã sang bài toán khác hẳn (MPMC), khó hơn nhiều bậc.

**Nhưng "không cần khoá" chưa đủ — vẫn cần ĐỒNG BỘ BỘ NHỚ.** Vấn đề: consumer thấy `head` đã tăng nhưng **chưa chắc thấy dữ liệu** vừa ghi vào ô đó, vì compiler và CPU được phép sắp xếp lại.

⇒ Cần cặp **release/acquire**: producer ghi dữ liệu **rồi mới** publish `head` bằng *release*; consumer đọc `head` bằng *acquire* **rồi mới** đọc dữ liệu. Cặp này đảm bảo: *thấy `head` mới ⇒ chắc chắn thấy cả dữ liệu ghi trước đó* ([CPP-019](cpp.md)).

⚠️ **Đây là lý do "chạy đúng trên x86, chết trên ARM":** x86 có mô hình bộ nhớ mạnh nên thường tha cho code thiếu nhãn; ARM lỏng hơn nhiều và sẽ lộ bug ngay.

**Hai tối ưu chuyên sâu (T3):**
- **Kích thước luỹ thừa của 2** ⇒ thay `% N` bằng phép AND — phép chia dư đắt hơn nhiều trên MCU không có lệnh chia.
- **`alignas(64)` tách `head` và `tail` sang hai cache line khác nhau** ⇒ tránh **false sharing**: hai biến ở chung một cache line làm hai core liên tục giành nhau đúng dòng cache đó, mất phần lớn hiệu năng vừa giành được.

**⚠️ Khi nào KHÔNG nên dùng lock-free:** khi bạn **chưa đo** thấy mutex là nút thắt. Bản dùng mutex **đúng dễ hơn nhiều**, dễ đọc, dễ bảo trì; bản lock-free sai thì hỏng **hiếm và không tái hiện được**. Mặc định dùng mutex, chỉ đổi khi có số liệu ([COD-006](coding.md)).

**Chốt:** *"SPSC lock-free được vì mỗi chỉ số chỉ một bên ghi — thêm producer thứ hai là mất tính chất đó. Nhưng vẫn cần cặp release/acquire, nếu không thì đúng trên x86 và chết trên ARM."*
</details>

---
⬅️ [Bank index](README.md)

---

⬅️ [Bank index](README.md)
