# Phiên mock — 2026-08-09 · coding · track dsa

- **Level:** mid-level · **Số câu:** 3 bài code · **Thời lượng:** ~40 phút
- **Điểm trung bình:** **3.67 / 4**
- **Bối cảnh:** 🎯 Datalogic plan — **Tuần 1 Buổi 5**. Bài COD-006 (ring buffer) được chấm theo thoả thuận riêng: **không tính điểm phần tái tạo code** (ứng viên vừa đọc [13-dsa/ring-buffer.md](../../../13-dsa/ring-buffer.md) hôm trước), chỉ chấm **câu hỏi ngược, lựa chọn thiết kế, và giải thích**.

## Kết quả từng câu (nhìn nhanh)

| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| COD-001 | Đảo linked list (iterative) | **4** | Code chuẩn; giải thích đúng vai trò 3 con trỏ, `prev=nullptr`, `while(cur)` vs `while(cur->next)`. Follow-up doubly list trả lời đúng. Chỉ nêu Time, **quên Space** |
| COD-006 | Ring Buffer | **3** | Hỏi ngược tốt (3 câu, có câu `sizeof(T)` nặng); chọn đúng chính sách đè-cũ; sửa đúng race khi được chỉ. **Nhưng:** không hỏi "mấy luồng?", ship race TOCTOU ở bản 1, và bản cuối **cấp phát `unordered_map` trong hot path** |
| COD-003 | Two Sum (hash map) | **4** | One-pass chuẩn, nêu đúng đánh đổi time↔space, giải thích cơ chế "đến lượt phần bù sẽ tìm ra" rất gọn. Gán nhầm nhãn bộ test khi nêu ca hỏng |

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### COD-006 — Ring Buffer (điểm 3)

**(a) Thiếu/sai — 4 điểm, xếp theo mức nghiêm trọng:**

1. **Không hỏi "mấy luồng?"** — đây là câu spec **số một** của ring buffer, vì nó quyết định toàn bộ thiết kế (có `count_` được không, có cần atomic không, mutex hay lock-free). Ứng viên **đặt thẳng `mutex` vào** mà chưa từng hỏi — tức mutex là *mặc định không xét*, không phải *lựa chọn*. Cũng không hỏi: byte-stream hay phần tử? (interviewer phải tự nêu capacity compile-time).
2. **Race TOCTOU ở bản 1**: `pop()` gọi `empty()` **trước** khi lấy khoá → hai consumer cùng thấy "còn 1 phần tử", cùng vào, `count_` underflow về `SIZE_MAX` và thread thứ hai đọc rác. Sửa đúng khi được vẽ interleaving, nhưng **không tự tổng quát hoá**: `empty()`/`full()` vẫn `public` + không khoá, nên `while (!rb.empty())` ở `main()` vẫn là đúng loại race đó.
3. **Bản cuối cấp phát động trong hot path** — `unordered_map<int, pair<T,T>> overriden;` khai **bên trong `push()`**, tức mỗi lần push (1000 lần/giây) là một lần dựng/huỷ hash map trên heap, chỉ để chứa **tối đa một** phần tử. Đây là thứ chính ring buffer sinh ra để tránh. Đúng phải là hai biến stack: `T old; bool dropped = false;`
4. **Tín hiệu drop vẫn chưa dùng được**: `push` cuối cùng **luôn `return true`** (đã từng sửa thành `false` rồi đổi lại) → caller không phát hiện được mất mẫu; và điều kiện in log là `if (full())` chứ không phải "vừa drop" — nếu có `pop` chen vào giữa thì `full()` false và **drop bị nuốt luôn**. Ở 1000 mẫu/giây, in `cout` mỗi lần drop cũng sai hướng: đúng là **`dropped_count_`** rồi xuất định kỳ.

**(b) Trích bank — [COD-006](../bank/coding.md):**

> **Hỏi ngược trước khi code** (ăn điểm nhất): *một luồng hay nhiều luồng? đầy thì từ chối hay đè cái cũ? byte stream hay phần tử?* Rồi nêu ngay cạm bẫy: `head == tail` vừa là rỗng vừa là đầy → bản trên **hy sinh 1 ô** để phân biệt (chứa được N−1).

**(c) Trích tài liệu gốc — [13-dsa/ring-buffer.md §4](../../../13-dsa/ring-buffer.md):**

> ⚠️ Bẫy phổ biến nhất trong review code: chọn "đè cũ nhất" **mà không đếm số lần đè**. Hệ trông vẫn chạy êm, và bạn không bao giờ biết mình đang mất dữ liệu. **Luôn có một `dropped_count`** và đưa nó ra ngoài (log/metric/sysfs). Cả `dmesg` lẫn `perf` đều làm điều này — `perf` in thẳng "*n events lost*".

Và [§1](../../../13-dsa/ring-buffer.md), về đúng cái bản cuối vi phạm:

> | Cấp phát khi chạy | **không** | có (theo lô) | có (mỗi phần tử) |
> | Realtime / ISR / kernel | **dùng được** | ❌ (`new` không tất định, có thể ngủ) | ❌ |

**(d) Chốt để nhớ:**
- Ring buffer tồn tại để **không cấp phát khi chạy**. Đặt `unordered_map` vào `push()` là **tự phá lý do tồn tại của nó**.
- Câu spec số một luôn là **"mấy luồng?"** — hỏi trước khi chọn `count_` hay mutex.
- Kiểm tra trạng thái (`empty`/`full`) **phải nằm trong cùng vùng khoá** với hành động dựa trên nó. Tách ra = TOCTOU.
- Đè cái cũ **luôn đi kèm `dropped_count`**, nếu không thì mất dữ liệu là im lặng.

### Ghi nhận điểm ĐÚNG mà interviewer từng nghi ngờ

Interviewer đặt bẫy ở câu "lô 1000 mẫu có còn liên tục không" với giả định ứng viên sẽ sai. **Ứng viên trả lời đúng:** vì luôn vứt từ đầu cũ, phần còn lại trong buffer luôn là **hậu tố liên tục** = N mẫu mới nhất. Chỗ thật sự nguy hiểm (interviewer bổ sung sau) là **lô mà consumer đang gom dở** vắt qua lúc treo — nó có lỗ và consumer không tự biết, và đó chính là lý do cần `dropped_count`.

### Ghi chú nhỏ — COD-001 & COD-003 (điểm 4, không phải lỗ hổng)

- **COD-001:** hỏi "time **và space**", chỉ trả lời Time O(n). Space O(1) là **điểm bán hàng chính** của bản iterative so với bản đệ quy (O(n) stack) — bỏ qua là mất một câu ăn điểm. Ngoài ra khi nói về `while (cur->next != nullptr)`: hậu quả không chỉ "chưa kịp đảo node cuối" mà còn **segfault ngay với danh sách rỗng**.
- **COD-003:** cơ chế ca hỏng mô tả đúng ("chỉ có một số 3, nó tự cộng với chính nó") nhưng gán nhầm bộ test — đó chính là bộ ① `[3,2,4]`, không phải bộ ②. Code dùng `mp.count(remain)` rồi `mp[remain]` = **hai lần tra hash**; một lần `find()` là đủ.

## Tổng kết

- **Điểm mạnh:**
  - **Chủ động hỏi ngược trước khi code** — làm đúng, và câu về `sizeof(T)` nặng (hy sinh 1 ô tốn 128 byte) là câu của người đã nghĩ tới bộ nhớ thật.
  - **Suy luận đánh đổi rành mạch**: `count_` dùng hết ô nhưng bị cả hai phía ghi → cần khoá. Đây đúng là trục quyết định của toàn bài.
  - **Sửa đúng và nhanh khi được chỉ vào interleaving** — không cãi, không sửa mò.
  - **Two-sum và reverse list ở mức chắc**, giải thích cơ chế gọn hơn mức trung bình mid.
  - Tự bắt lỗi đơn vị (125 MB → 125 KB) khi được yêu cầu kiểm lại.

- **Lỗ hổng ưu tiên (top 3):**
  1. **Thiếu câu hỏi spec về đa luồng** — [ring-buffer.md §11 bước 1](../../../13-dsa/ring-buffer.md) + [§9 cây quyết định](../../../13-dsa/ring-buffer.md). Đọc lại đúng hai mục đó.
  2. **Không tổng quát hoá bug sau khi sửa** — sửa `pop()` nhưng để nguyên `empty()`/`full()` public không khoá. Sau khi vá một race, phải quét cả class xem còn chỗ nào cùng dạng: [ring-buffer.md §10 checklist](../../../13-dsa/ring-buffer.md).
  3. **Phản xạ "cứ dùng STL container"** trong đường dữ liệu nóng embedded — [ring-buffer.md §1 bảng so sánh](../../../13-dsa/ring-buffer.md) và [08/constraints.md](../../../08-embedded-systems/constraints.md) (cấm heap sau init).

- **Câu mới thêm vào bank:** **DSA-013** (chọn N cho ring buffer), **DSA-014** (đè cũ + phát hiện mất dữ liệu). Thêm follow-up doubly-linked-list vào **COD-001**.
- **Cập nhật weak-register:** **không đổi** — không câu nào ≤ 2. Sổ vẫn 4 câu (CPP-019, CPP-024, CPP-045, CPP-032), tất cả ở 1/2 lần ≥3. Phiên này là `coding` nên không chèn được câu concept của sổ.

## Phiên kế đề xuất

- **Tuần 1 Buổi CN** (theo plan): ôn dồn tích Tuần 1 — `/mock weak-review` để dọn nốt 4 câu trong sổ (mỗi câu chỉ cần **một** lần ≥3 nữa là sổ sạch), rồi `/mock retention track cpp-system`.
- Bài tập tự làm trước buổi CN (~15′): **viết lại `ring_buffer.cpp`** bỏ `unordered_map`, thêm `dropped_count_`, cho `push` trả về trạng thái drop dùng được, và làm `empty()/full()` an toàn với đa luồng (hoặc `private`).
