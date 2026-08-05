# Phiên mock — 2026-08-05 · daily · track cpp-system

- **Level:** mid-level · **Số câu:** 6 · **Thời lượng:** ~20 phút
- **Điểm trung bình:** 2.83 / 4
- **Bối cảnh:** Tuần 1 **Buổi 3** của [datalogic-plan](../../study-plans/datalogic-plan.md) — concurrency (02-modern-cpp) + OS sync-primitives.

## Kết quả từng câu (nhìn nhanh)
| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| OS-004 | race condition | 4 | định nghĩa chuẩn + ví dụ counter + bóc **RMW** ở follow-up; thiếu mỗi từ "critical section" |
| CPP-024 | shared_ptr thread-safe | 2 | 🔽 từ 4 — tưởng ghi đồng thời **cùng 1 instance** là an toàn vì count atomic → sinh câu mới **CPP-052** |
| CPP-022 | volatile ⭐ | 3 | phần lõi rất tốt; sai ca `volatile uint64_t` trên MCU 32-bit: nói "tràn số", đúng là **torn read** |
| OS-012 | condition variable ⭐ | 3 | trả lời đầu chưa trúng cơ chế; follow-up gỡ tốt (nhả mutex khi ngủ, deadlock, spurious wakeup) |
| CPP-019 | memory order | 1 | không nhớ tên 3 mức; chỉ nói **compiler** reorder, thiếu **CPU** |
| OS-007 | mutex vs semaphore (revisit) | 4 | 🔼 từ 2 — ownership + priority inheritance + cơ chế nâng priority + recursive mutex |

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-019 — memory order (điểm 1) 🔴 *lỗ hổng lớn nhất phiên*
- **Thiếu/sai:** không nhớ tên mức nào ("mức 1 là mặc định, an toàn nhưng chậm"); follow-up chỉ nêu **compiler** reorder — **thiếu mất CPU cũng reorder** (out-of-order execution, store buffer), vốn là lý do memory order tồn tại ở tầng phần cứng.
- **Bank:** > "CPU/compiler reorder lệnh để tối ưu; trong đa luồng điều này khiến thread khác thấy thứ tự ghi khác kỳ vọng. Memory order ràng buộc thứ tự quanh atomic op: `seq_cst` (nhất quán toàn cục, dễ suy luận, chi phí cao), `acquire/release` (đồng bộ cặp đôi), `relaxed` (chỉ atomicity). Mặc định `seq_cst`; chỉ hạ xuống khi đã đo và thật sự hiểu — vùng cực dễ sai."
- **Tài liệu:** > "**CPU và compiler** có thể **sắp xếp lại** (reorder) lệnh để tối ưu. Trong đa luồng, điều này có thể khiến thread khác thấy thứ tự ghi khác kỳ vọng." ([concurrency.md §6](../../../02-modern-cpp/concurrency.md#L116))
  > "Lời khuyên thực tế: **mặc định `seq_cst`** (an toàn, dễ đúng). Chỉ hạ xuống acquire/release/relaxed khi đã đo được và *thật sự hiểu* — đây là vùng rất dễ sai." ([:124](../../../02-modern-cpp/concurrency.md#L124))

  | Mức | Đảm bảo gì | Chi phí |
  |---|---|---|
  | `memory_order_seq_cst` | **Mặc định**. Thứ tự tuần tự nhất quán **toàn cục** | Cao nhất |
  | `memory_order_acquire`/`release` | Đồng bộ **cặp đôi**: release "công bố", acquire "nhìn thấy" — producer/consumer | Trung bình |
  | `memory_order_relaxed` | **Chỉ** atomicity, không ràng buộc thứ tự — counter độc lập | Thấp nhất |
- **Chốt (câu trả lời an toàn khi bị hỏi):** *"CPU và compiler đều reorder; single-thread không thấy vì semantic được bảo toàn, đa thread thì lộ. Ba mức: seq_cst / acquire-release / relaxed. Em mặc định seq_cst, chỉ hạ khi đã profile — sai ở đây là loại bug không reproduce được."* Câu cuối (thái độ thận trọng) là thứ interviewer muốn nghe nhất.

### CPP-024 — shared_ptr thread-safe (điểm 2) 🔽 *tụt từ 4*
- **Thiếu/sai:** vế đầu đúng (control block atomic). Nhưng follow-up trả lời **sai**: "hai thread cùng `p = otherPtr` trên cùng object `p` là an toàn vì strong_cnt atomic, object trỏ tới cũng không bị ảnh hưởng". Gộp 3 tầng làm 1.
- **Ba tầng phải tách:**

  | Tầng | Thread-safe? |
  |---|---|
  | Control block (strong/weak count) | ✅ atomic |
  | **Bản thân object `shared_ptr`** (payload ptr + control block ptr) | ❌ ghi đồng thời cùng 1 instance = data race |
  | Object được trỏ tới (payload) | ❌ cần mutex riêng |

  `p = other` **không** atomic: giảm count cũ → ghi **hai** con trỏ → tăng count mới. Xen kẽ → `p` trỏ payload A + control block B → double-free/leak. Muốn an toàn: `std::atomic<std::shared_ptr<T>>` (C++20), `atomic_load/store` (C++11, deprecated C++20), hoặc mutex — tốt hơn nữa là **mỗi thread giữ copy riêng**.
- **Bank:** > "Bộ đếm tham chiếu là atomic nên copy/destroy `shared_ptr` từ nhiều thread an toàn. Nhưng **object được trỏ tới không được bảo vệ** — ghi đồng thời vào object vẫn cần mutex." (CPP-024); chi tiết 3 tầng ở **[CPP-052](../bank/cpp.md)** (câu mới thêm từ phiên này).
- **Tài liệu:** > "Đếm tăng/giảm **atomic** → thread-safe cho việc đếm (**không tự động an toàn cho dữ liệu bên trong!**)" ([raii-smart-pointers.md:70](../../../02-modern-cpp/raii-smart-pointers.md#L70)); > "`shared_ptr` thread-safe về **đếm**, không thread-safe về **dữ liệu** trỏ tới (cần mutex riêng)." ([:224](../../../02-modern-cpp/raii-smart-pointers.md#L224))
- **Chốt:** *"Đếm thì atomic — nhưng cái vỏ `shared_ptr` và cái ruột object thì không."*

### CPP-022 — volatile (điểm 3)
- **Thiếu/sai:** phần chính **rất tốt** (không liên quan thread-safe; cấm compiler tối ưu/reorder/cache; biến bị tác nhân ngoài sửa — GPIO). Ca ISR ghi flag đơn: đúng. **Sai ở ca `volatile uint64_t` trên MCU 32-bit** — trả lời "bị tràn"; đúng là **atomicity / torn read**: CPU 32-bit đọc 64-bit bằng **2 lệnh load**, ISR chen vào giữa → main ghép nửa cũ + nửa mới → giá trị **chưa từng tồn tại**.
- **Bank (EMB-010):** > "(2) **atomicity** — đọc/ghi biến nhiều byte (uint32 trên MCU 8/16-bit, hoặc struct) có thể bị ISR chen giữa chừng → **dữ liệu rách**… Với dữ liệu nhiều byte/nhiều biến → **critical section**: tắt ngắt quanh đoạn đọc/ghi ở phía main, giữ **cực ngắn**. Mẫu tốt: ISR ghi vào **ring buffer** (SPSC). `volatile` **không** thay được critical section cho RMW."
- **Tài liệu:** > "⚠️ `volatile` **không** thay được critical section: nó cấm tối ưu nhưng **không** làm RMW/đa-biến atomic." ([interrupts-bare-metal.md:57](../../../08-embedded-systems/interrupts-bare-metal.md#L57))
  > "Biến đơn ≤ kích thước word + cờ đơn giản → `volatile` + kiểu atomic của phần cứng đủ. Dữ liệu nhiều byte / nhiều biến → **critical section**." ([:45-46](../../../08-embedded-systems/interrupts-bare-metal.md#L45))
- **Chốt:** `volatile` = *"đọc/ghi thật, đừng tối ưu"*. **Không** atomicity, **không** ordering giữa core. ISR-shared ≤ 1 word → volatile đủ; > 1 word → tắt ngắt / ring buffer SPSC.

### OS-012 — condition variable (điểm 3)
- **Thiếu/sai:** câu trả lời đầu chưa trúng cơ chế — mutex chỉ được giải thích là "để xử lý critical section an toàn", `while` hiểu thành "kiểm tra liên tục cho tới khi thỏa" (nghiêng về polling). Follow-up **gỡ lại tốt**: nêu đúng `wait()` nhả mutex khi ngủ, giữ lock mà ngủ → deadlock, gọi đúng tên **spurious wakeup**. Điểm 3 vì phải gợi mới ra lõi.
- **Bank:** > "Đi kèm mutex vì điều kiện dựa trên dữ liệu chung cần bảo vệ; `wait` **nhả mutex khi ngủ** (để thread khác sửa điều kiện) và **giành lại khi thức**. Predicate chống **spurious wakeup** và race giữa notify/wait."
- **Tài liệu:** > "(1) **spurious wakeup** — condition variable có thể đánh thức thread mà không có notify… (2) **lost wakeup / kiểm tra điều kiện trước khi chờ** — nếu điều kiện đã đúng trước khi gọi wait thì không chờ. Dạng `cv.wait(lock, pred)` lặp kiểm tra predicate, **nhả lock khi ngủ và giành lại khi thức**." ([concurrency.md §7](../../../02-modern-cpp/concurrency.md#L211))
- **Bổ sung sau phiên (người học tự chốt đúng):** `while` **không phải polling** — không thỏa thì **rơi lại vào `wait()` và ngủ tiếp** (blocked trong futex, 0% CPU), chỉ chạy vài lệnh mỗi lần *thức dậy*. Dùng `if` thì khi thức dậy sẽ **rơi thẳng xuống critical section với predicate sai** → pop queue rỗng / deref null → corruption, chứ không phải "chờ hụt".
- **Tầng thứ 3 hay bị bỏ sót — "stolen wakeup"** (thực chiến gặp nhiều hơn spurious): 3 consumer cùng chờ, producer đẩy **1** item rồi `notify_all()`. Cả 3 thức. A giành lock trước, lấy item, nhả lock. B/C thức **hợp lệ** (có notify thật) nhưng lúc giành được lock thì queue **đã rỗng lại**. → Ba lý do cần `while`: **spurious wakeup** (không ai notify) · **stolen wakeup** (bị thread khác ăn mất điều kiện) · **lost wakeup** (điều kiện đã đúng trước khi `wait`).
- **Chốt:** *"`wait()` trả về chỉ có nghĩa là 'có thể có gì đó đã đổi', không có nghĩa 'điều kiện của bạn đúng'."* Nên phải kiểm tra lại — và kiểm tra lại thì phải trong **vòng lặp**, vì có thể vẫn sai. Mutex bắt buộc vì "check điều kiện rồi đi ngủ" phải nguyên tử; nếu không, notify lọt vào khe giữa hai bước.

### Ghi chú câu điểm 4 (để giữ nhịp)
- **OS-004:** đủ ý, chỉ nên đóng gói bằng thuật ngữ **critical section**.
- **OS-007:** đã lên 4. Bổ sung để "senior": phân **vai trò** — mutex = *bảo vệ* critical section (đối xứng, cùng thread lock/unlock); semaphore = *báo hiệu bất đối xứng* (ISR `give` → task `take`) hoặc quản N tài nguyên. Dùng sai vai trò mới là gốc lỗi; mất priority inheritance chỉ là hệ quả.

## Tổng kết
- **Điểm mạnh:** race condition + RMW rất chắc; `volatile` phần lõi (câu ⭐ hay bị nhầm) trả lời gọn đúng; OS-007 lên hẳn 4 sau khi ôn — chứng tỏ cơ chế weak-register đang hiệu quả.
- **Lỗ hổng ưu tiên (top 3):**
  1. **Memory order (CPP-019)** — thuộc 3 mức + "CPU **và** compiler cùng reorder" → [concurrency.md §6](../../../02-modern-cpp/concurrency.md#L114).
  2. **shared_ptr 3 tầng thread-safety (CPP-024/052)** → [raii-smart-pointers.md §shared_ptr](../../../02-modern-cpp/raii-smart-pointers.md#L70).
  3. **volatile ≠ atomic với ISR (CPP-022)** — "torn read", không phải "tràn số" → [interrupts-bare-metal.md §3](../../../08-embedded-systems/interrupts-bare-metal.md#L39).
- **Câu mới thêm vào bank:** **CPP-052** (🟠 ⭐) — "Hai thread cùng gán vào cùng một object `shared_ptr` — an toàn không? Ba tầng thread-safety."
- **Cập nhật weak-register:** +CPP-019 (mới, điểm 1); +CPP-024 (mới, tụt 4→2); OS-007 lên 4 (2,4 — 1/2 lần tốt, **chưa gỡ**); CPP-045/CPP-032/CPP-009/OS-003/DP-002 giữ nguyên (không hỏi phiên này).

## Phiên kế đề xuất
- **Tuần 1 Buổi 4:** `/mock ... track build-systems` (CMake — tư duy target-based, BLD-001…003) theo plan.
- Chèn `weak-review` sớm cho **CPP-019** (memory order) — điểm 1 là mức thấp nhất từ đầu plan tới giờ; nên đọc lại §6 concurrency trước khi bị hỏi lại.
