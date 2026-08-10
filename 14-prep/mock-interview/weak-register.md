# 🔴 Weak Register — Sổ câu còn yếu (ưu tiên hỏi lại)

> Danh sách câu đã trả lời **sai / chưa chuẩn / lúng túng** (điểm ≤ 2 theo [config.md §4](config.md)) — interviewer **ưu tiên chèn lại** vào các phiên. Không có luật "đúng rồi thôi": câu ở đây được hỏi lại tới khi vững (≥ 3 điểm **hai lần liên tiếp** → gỡ khỏi sổ).
> Interviewer cập nhật file này ở Bước 4 mỗi phiên. Git-track để thấy điểm yếu co lại dần.

## Cách đọc
- **Lần gặp** = số phiên đã hỏi câu này. **Điểm gần nhất** = điểm 2 lần mới nhất (vd `2, 3`).
- Sắp xếp: câu điểm thấp + gặp nhiều lần mà chưa lên → **ưu tiên cao nhất**.

## Bảng câu yếu

| ID | Câu (tóm tắt) | Track | Lần gặp | Điểm gần nhất | Ghi chú lỗ hổng |
|----|---------------|-------|---------|----------------|-----------------|
| CPP-032 | explicit / implicit conversion | cpp-system, modern-cpp | 3 | 4, **2** | 🔻 **RESET bộ đếm** (2026-08-10). Nửa **constructor** đã vững (`send(1024)` / `send("hello")` — chỉ đúng cả hai ca + nêu đúng vì sao nguy hiểm). Hổng **nửa còn lại**: `explicit` áp cho **cả conversion operator** — không biết **safe-bool problem** (`operator bool` không explicit ⟹ `T`→`bool`→`int` ⟹ `cout << obj`, `a == b`, `a + 1` đều compile sạch với `-Wall`), không biết **contextual conversion to bool** (vì sao `explicit operator bool` mà `if (obj)` vẫn chạy). Tiêu chí bỏ `explicit` cũng chưa chặt (nói "vì dùng tự nhiên" → đúng phải là *lossless, không làm việc gì*: cấp phát/mở tài nguyên/đổi ngữ nghĩa ⟹ explicit). Ôn: [CPP-053](bank/cpp.md) (câu mới) |
| CPP-054 | move ctor vs move assign (RAII fd, move-only) | cpp-system, modern-cpp | 1 | **2** | 🆕 Thêm 2026-08-10 (phát sinh từ follow-up CPP-045). Viết move ctor **đọc `fd_` chưa khởi tạo** (`if (fd_ != -1) close(fd_)` trong ctor) → UB, có thể `close()` fd của module khác; hỏi 2 lần vẫn trả lời *"fd_ mang giá trị từ constructor"* + *"hai chỗ cùng logic"*. Chưa nắm: **move ctor xây từ số 0 nên KHÔNG dọn**, move assign **phải dọn**. Thiếu `noexcept` (⟹ move-only không dùng được trong `vector`). Phụ: `<bits/stdc++.h>` trong header thư viện |

## 🔁 Lịch kiểm tra lại (retention) — hàng đợi, KHÔNG phải nghĩa địa

> **Vấn đề đã có:** câu gỡ khỏi sổ rơi vào bảng "Đã khắc phục" rồi **không cơ chế nào kéo về**. Type `retention` lẽ ra làm việc đó nhưng phải *"ưu tiên câu đã lâu chưa hỏi lại"* mà không nơi nào ghi ngày → thực tế bị bỏ qua. Bảng này là nguồn để `retention` rút, thay cho việc quét `sessions/`.
>
> **Luật:** gỡ ở tuần N → kiểm tra lại **tuần N+2** (~2 tuần, đúng khoảng quên). Rải vào **slot khởi động 🔁 5–10′ đầu mỗi buổi** ([datalogic-plan](../study-plans/datalogic-plan.md) nguyên tắc ②), không dồn hết vào CN.
> **Hỏi theo config §6 luật ①:** nén phần nền còn 1 checkpoint, trọng số ở follow-up, **không lặp lại góc cũ**. Điểm **< 3 → kéo thẳng về bảng sổ yếu bên trên** (regression). Điểm ≥ 3 → dời lịch thêm 2 tuần hoặc coi như vững nếu hết plan.

| ID | Gỡ ngày | 🔁 Kiểm tra lại | Góc đã dùng (**cấm lặp**) | Góc mới đề xuất | KQ |
|----|---------|-----------------|---------------------------|-----------------|-----|
| CPP-020 | 29/07 | **Tuần 2** (10–16/08) | Rule 0/3/5 là gì, vì sao Rule 0 | Cho class có `unique_ptr` + `char*` thô lẫn lộn → hỏi compiler sinh gì, cái nào hỏng | ⬜ |
| CPP-029 | 03/08 | **Tuần 2** (10–16/08) | emplace_back vs push_back khác gì | Cho đoạn `v.emplace_back(...)` với ctor `explicit` → vì sao compile được mà `push_back` thì không | ⬜ |
| CPP-009 | 07/08 | **Tuần 3** (17–23/08) | template ở header, vì sao | Link error thật khi tách template ra `.cpp` → đọc message, chẩn đoán | ⬜ |
| DP-002 | 07/08 | **Tuần 3** (17–23/08) | Singleton Meyers, lazy + thread-safe | Singleton trong shared library `.so` — có còn *một* instance không? (mỗi `.so` một copy) | ⬜ |
| OS-003 | 07/08 | **Tuần 3** (17–23/08) | 4 điều kiện Coffman | Cho 2 hàm lock 2 mutex ngược thứ tự → chỉ ra deadlock + sửa bằng `scoped_lock` | ⬜ |
| OS-007 | 07/08 | **Tuần 3** (17–23/08) | mutex vs semaphore, ownership, PI | Cho tình huống RT: task ưu tiên cao kẹt sau task thấp → gọi tên hiện tượng + cơ chế cứu | ⬜ |
| CPP-019 | 10/08 | **Tuần 4** (24–29/08) | 3 mức · release/acquire 3 ca · khi nào chọn acq/rel | **Đọc code**: đoạn producer/consumer thiếu nhãn ở một đầu → chỉ ra chỗ thiếu | ⬜ |
| CPP-024 | 10/08 | **Tuần 4** (24–29/08) | 3 tầng · cách sửa hot-reload config | Cho **backtrace crash trong `free()`** → chẩn đoán ngược về race tầng 2 | ⬜ |
| CPP-045 | 10/08 | **Tuần 4** (24–29/08) | link vs compile · viết lại class | **Code review**: đồng nghiệp đề xuất `private` không định nghĩa → phản biện | ⬜ |

Ký hiệu KQ: ⬜ chưa hỏi · ✅ ≥3 (vững, dời +2 tuần) · 🔻 <3 (**kéo về sổ yếu**)

---

## Đã khắc phục (lưu vết — gỡ khỏi bảng trên khi ≥3 điểm hai lần)

| ID | Câu | Ngày gỡ |
|----|-----|---------|
| CPP-020 | Rule of 0/3/5 | 2026-07-29 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
| CPP-029 | emplace_back vs push_back | 2026-08-03 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
| CPP-009 | template ở header — compile-time | 2026-08-07 (2 → 3 → **4**; nói thẳng *instantiation lúc compile* + linker error, trả lời được cả explicit instantiation) |
| DP-002 | Singleton hiện đại (Meyers static-local) | 2026-08-07 (2 → 3 → **4**; lazy + thread-safe **chỉ ở khởi tạo** + copy `=delete`) |
| OS-003 | deadlock — 4 điều kiện Coffman | 2026-08-07 (0 → 3 → **4**; đủ 4 điều kiện, nêu đúng "phải thoả ĐỒNG THỜI", lock ordering + try_lock timeout) |
| OS-007 | mutex vs semaphore | 2026-08-07 (2 → 4 → **4**; mục đích + ownership + priority inheritance kèm cơ chế) |
| CPP-019 | memory order — 3 mức + vì sao thận trọng | 2026-08-10 (2 → 3 → **3**; hỏi ở mức **thiết kế**: nêu đúng 3 điều kiện lập cặp acq/rel + ví dụ `relaxed` đủ dùng chuẩn (retry counter) + ranh giới *"biến atomic có dùng để suy luận về dữ liệu khác không"*. Còn thiếu: nói **"đo/profile rồi mới hạ"** như căn cứ quyết định) |
| CPP-024 | shared_ptr thread-safe (3 tầng) | 2026-08-10 (2 → 4 → **4**; tự dựng được mô hình **publish/RCU** cho hot-reload config: `atomic_load` giữ object cũ sống tới khi reader xong. Nhắc thêm: `atomic_load/store` **deprecated C++20, bỏ ở C++26**) |
| CPP-045 | `=delete` vs private cũ | 2026-08-10 (2 → 4 → **3**; lý thuyết đạt — link error vs compile error khi gọi ngoài class, `= delete` để `public` cho message rõ. Phần code sinh ra câu mới [CPP-054](bank/cpp.md)) |

> **Ghi chú calibration (2026-08-07):** tên gọi nội bộ của compiler (vd *guard variable* ở DP-002) **không** tính là lỗ hổng — chỉ là "biết thì tốt". Tiêu chí giữ một câu trong sổ: **cơ chế giải thích được một lớp bug sẽ gặp trong công việc**, không phải thuật ngữ thuộc lòng.

> **Ghi chú calibration (2026-08-10) — CPP-019, tầng barrier ARM:** ứng viên phản hồi câu *"hạ seq_cst → acq/rel tiết kiệm gì ở tầng phần cứng ARM"* là quá sâu cho mid-level. ✅ **Đồng ý một phần** — tên lệnh (`stlr`/`ldar`/`dmb ish`) là kiến thức compiler-backend, **không chấm**, chỉ tính "biết thì tốt". ❌ **Nhưng giữ ý niệm**: *"acq/rel gần như free trên x86, có phí thật trên ARM"* vẫn phải biết, vì nó là lý do **tối ưu memory order đo trên máy dev x86 là vô nghĩa** — đúng lớp bug của JD ARM. Thang chấm CPP-019 từ nay: ý (a) căn cứ hạ mức + ý (c) ranh giới `relaxed` = đủ 3 điểm; tầng phần cứng chỉ nâng 3 → 4.
