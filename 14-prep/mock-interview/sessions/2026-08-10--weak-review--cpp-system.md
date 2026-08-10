# Phiên mock — 2026-08-10 · weak-review · track cpp-system

- **Level:** mid-level · **Số câu:** 4 (toàn bộ weak-register) · **Thời lượng:** ~30 phút
- **Điểm trung bình:** **3.0 / 4**
- **Bối cảnh:** Buổi CN Tuần 1 theo [datalogic-plan](../../study-plans/datalogic-plan.md) (trễ 2 ngày). Cả 4 câu đều đang ở **1/2 lần ≥3** → phiên này để dọn sổ trước khi sang Tuần 2.
- **Cách hỏi:** không hỏi lại mức cơ bản (đã đạt 4 ở phiên 07/08). Hỏi thẳng **tầng sâu hơn** đúng theo ghi chú "lần sau hỏi mức nào" trong sổ yếu.
- **Kết quả tổng: sổ yếu 4 → 2 câu** — gỡ 3 (CPP-019/024/045), giữ 1 (CPP-032, tụt 4→2), **thêm 1 câu mới phát sinh** (CPP-054).

## Kết quả từng câu

| ID | Câu (tóm tắt) | Điểm | Diễn biến | Kết quả |
|----|---------------|------|-----------|---------|
| CPP-019 | memory order — hỏi mức **thiết kế** | 3 | 2→3→**3** | ✅ **GỠ** |
| CPP-024 | `shared_ptr` — hỏi **cách sửa** | 4 | 2→4→**4** | ✅ **GỠ** |
| CPP-045 | `= delete` — hỏi qua **đọc code** | 3 | 2→4→**3** | ✅ **GỠ** |
| CPP-032 | `explicit` — hỏi mức **thiết kế API** | 2 | 2→4→**2** | ❌ **GIỮ**, reset bộ đếm |

**Câu mới thêm vào bank:** [CPP-053](../bank/cpp.md) (safe-bool / `explicit operator bool`) · [CPP-054](../bank/cpp.md) (move ctor vs move assign, RAII fd move-only). CPP-054 vào luôn weak-register (điểm 2).

---

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-032 — `explicit` (điểm **2**) 🔴 ưu tiên cao nhất

**Đề đã hỏi:** class `Buffer` có `Buffer(size_t)`, `Buffer(const char*)`, `operator bool()`; API `void send(const Buffer&)`.
(a) vì sao `send(1024)` / `send("hello")` compile được? (b) `operator bool` không explicit cho lọt biểu thức vô nghĩa nào? (c) sửa thế nào, ctor nào cố ý KHÔNG explicit?

**Làm được — ý (a), tốt:** chỉ đúng cả hai ca là implicit conversion qua ctor 1 đối số, và nêu đúng *vì sao nguy hiểm*: `send("hello")` **trông như** gửi chuỗi nhưng thực chất dựng `Buffer` tạm; `send(1024)` cấp phát 1KB rỗng rồi gửi đi.

**Hổng — ý (b) trả lời "không rõ".** Đây là **nửa còn lại của chính câu bank**:

> **Trích bank CPP-032:** *"`explicit` chặn chuyển đổi ngầm qua constructor một-đối-số **(và conversion operator)**."*

Tên vấn đề: **safe-bool problem**. Cơ chế: `bool` là **integral type** → compiler đi tiếp một nấc `T` → `bool` → `int` (integral promotion) → object tham gia mọi phép toán số học/so sánh.

**Đã biên dịch + chạy thật, không phải phỏng đoán:**

```
cout << a   -> 1       // ❗ in "1" dù Buffer không có operator<<
a == c      -> 1       // ❗ "hello" == "world" ra TRUE — so sánh 2 cái bool, không phải nội dung
a + 1       -> 2       // ❗ cộng Buffer với số
a > 3       -> 0       // ❗ so sánh Buffer với số
int n = a;  -> 1       // ❗ gán Buffer vào int
```

| Compiler | Bắt được | Ghi chú |
|---|---|---|
| `g++ 11.4 -Wall -Wextra` | **0 / 5** | im lặng tuyệt đối |
| `clang -Wall -Wextra` | **1 / 5** | chỉ `a > 3`, qua `-Wtautological-constant-out-of-range-compare` |

⟹ **4/5 ca lọt qua mọi compiler không một tiếng động.** Không công cụ nào cứu — chỉ `explicit` mới chặn được. Đó là lý do câu này đáng nằm trong sổ yếu.

⭐ **Ứng viên tự viết trúng một ca ngay trong file của mình** — [explicit_issue.cpp:28](../coding-arena/explicit_issue.cpp#L28): `std::cout << "sent: " << buf << std::endl;` — dòng này lẽ ra phải lỗi compile, nó chạy được vì `buf` ngầm hoá thành `bool` → in `1`. Viết ra hoàn toàn tự nhiên mà không nhận ra → đúng cách bug này lọt vào codebase thật.

**Cách sửa — `explicit operator bool() const`, và `if (buf)` VẪN chạy.**
Vì C++11 định nghĩa **contextual conversion to bool**: ở vị trí ngôn ngữ đằng nào cũng cần bool, conversion operator `explicit` vẫn được gọi tự động. Danh sách: `if` · `while` · `for(;;)` · `!x` · `x && y` · `x || y` · `x ? a : b` · `static_assert` · điều kiện `do-while`.

```cpp
explicit operator bool() const { return !buffer_.empty(); }
if (buf) {}       // ✅ vẫn chạy
if (!buf) {}      // ✅
std::cout << buf; // ❌ lỗi compile — ĐÚNG như mong muốn
int n = buf;      // ❌
a == b;           // ❌
```
STL đều làm vậy: `unique_ptr`, `shared_ptr`, `optional`, `function`, `ifstream`.

**Hổng — ý (c), tiêu chí chưa chặt.** Ứng viên nói bỏ `explicit` cho `const char*` "vì dùng tự nhiên, đúng mục đích". Tiêu chí chuẩn:

> **Để implicit khi conversion là "cùng một thứ, đổi cách biểu diễn" (lossless, không làm gì cả). Đánh `explicit` khi conversion *làm một việc gì đó* — cấp phát, mở tài nguyên, đổi ngữ nghĩa.**

| Ctor | Quyết định | Lý do |
|---|---|---|
| `Buffer(size_t)` | 🔴 explicit bắt buộc | `1024` **không phải** một buffer; nó *cấp phát* 1024 byte |
| `Buffer(const char*)` | 🟡 cũng nên explicit | Nghe tự nhiên nhưng **copy toàn bộ dữ liệu** — giấu một lần cấp phát + copy trong hot path |

Đối chiếu STL: `std::string` để implicit cho `const char*`; `std::vector` **explicit** cho `size_t`; `unique_ptr` explicit cho con trỏ thô (nhận con trỏ = **nhận quyền sở hữu**, không được ngầm).

> **Trích tài liệu gốc** — [01-cpp-fundamentals/oop.md](../../../01-cpp-fundamentals/oop.md) dùng đúng idiom này ở dòng 66: `explicit Circle(double r) : r_(r) {}`.

**Chốt:** *"`explicit` không chỉ cho constructor. Ctor 1 đối số mặc định `explicit`; bỏ explicit là quyết định phải **biện minh được**. Conversion operator thiếu `explicit` biến object thành số nguyên — mà `if` vẫn chạy nhờ contextual conversion, nên không mất gì khi thêm."*

**Ôn:** [CPP-053](../bank/cpp.md) (câu mới, đầy đủ) · [CPP-032](../bank/cpp.md) · [oop.md](../../../01-cpp-fundamentals/oop.md).

---

### CPP-045 — `= delete` (điểm **3**) — lý thuyết đạt, code có UB

**Đề đã hỏi:** cho class `DeviceHandle` kiểu C++98 (copy ctor/assign khai báo `private`, không định nghĩa) + một member function `dump()` gọi `DeviceHandle copy = *this;`.
(a) hỏng ở giai đoạn nào, gọi từ ngoài class thì khác gì? (b) viết lại bằng `= delete`, đặt public hay private? (c) theo Rule of 0/3/5 còn thiếu gì; cho move cấm copy ra sao?

**Làm được — (a) và (b) đạt:**
- Lỗi ở **link**, lý do đúng: khai báo có → compiler chấp nhận lời gọi, để trống symbol; **linker** mới phát hiện không có định nghĩa.
- Gọi **ngoài class** → lỗi **compile-time** vì vi phạm access control (private). Đúng.
- `= delete` để `public`, lý do "thể hiện rõ ý định". Bổ sung lý do kỹ thuật: để `private` thì compiler báo lỗi **access** trước, **che mất** thông điệp *"use of deleted function"*.

> **Trích bank CPP-045:** *"Cách cũ (C++98): khai báo copy ctor/assign **private + không định nghĩa** → dùng nhầm chỉ lỗi lúc **link**, thông báo mơ hồ. `= delete`: hàm **tồn tại nhưng bị xóa** → mọi lời gọi lỗi ngay lúc **compile**."*

**Hổng — phần code (c): move constructor đọc `fd_` chưa khởi tạo (UB).** Chi tiết đầy đủ ở [CPP-054](../bank/cpp.md) + [bản đã review](../coding-arena/reviewed/2026-08-10--CPP-045--device-handle.cpp).

---

### CPP-019 — memory order, mức thiết kế (điểm **3**) → GỠ

**Đề đã hỏi:** producer/consumer queue dùng `std::atomic` trong shared library trên board ARM.
(a) khi nào hạ `seq_cst` → `acquire/release`, căn cứ gì? (b) trên ARM tiết kiệm gì ở tầng phần cứng, x86 có khác? (c) khi nào yêu cầu đổi ngược `relaxed` → `seq_cst`?

**Làm được:**
- (a) nêu đúng **3 điều kiện lập cặp acq/rel**: có sequence dữ liệu cần công bố · **cùng một biến atomic** · nhãn ở **cả hai đầu**. Khớp bank.
- (c) sau follow-up, ví dụ `relaxed` **chuẩn**: biến đếm retry (tăng 1 mỗi lần, quá 10 thì cancel) — *"không ảnh hưởng mạch logic khác"*. Khớp bank (*"counter độc lập, không ai suy luận theo nó"*).
- Ranh giới tự nêu: **"biến atomic đó có được dùng để suy luận về một dữ liệu khác không"** — đúng ranh giới.
- Thái độ *"ưu tiên seq_cst để không phải suy nghĩ"* — chính là thứ interviewer chấm.

**Thiếu — một chỗ, khung quyết định:** ý (a) nói *"hạ khi yêu cầu performance mà seq_cst nặng không đáp ứng"*. Căn cứ đúng phải là **đã đo và xác định chính chỗ này là nghẽn**, không phải cảm giác.

> **Trích tài liệu gốc** — [02-modern-cpp/concurrency.md:124](../../../02-modern-cpp/concurrency.md#L124):
> *"Mặc định `seq_cst` (an toàn, dễ đúng). Chỉ hạ xuống acquire/release/relaxed khi **đã đo được** và thật sự hiểu — đây là vùng rất dễ sai."*
> Và [dòng 176](../../../02-modern-cpp/concurrency.md#L176): *"Đừng tối ưu memory order sớm — `seq_cst`/mutex trước, **đo rồi mới hạ**."*

**Lưu ý:** ý (c) ban đầu ứng viên mô tả **ca dùng acq/rel** chứ không phải ca *relaxed → seq_cst*; follow-up gỡ được ngay.

**Chốt:** *"`relaxed` đủ khi không ai đọc biến đó rồi kết luận về một dữ liệu khác. Hễ nó là **cửa canh** cho dữ liệu khác → phải acquire/release hoặc seq_cst. Và chỉ hạ mức sau khi **đã đo**."*

---

## 📗 Ghi chú câu điểm 4 (giữ nhịp) — CPP-024

**Đề đã hỏi:** `shared_ptr<Config> g_config` toàn cục, reader trong hot path copy rồi dùng, writer `reload()` ~1 lần/giờ gán object mới. (a) race tầng mấy, hậu quả gì trên board? (b) các cách sửa + đánh đổi? (c) kẹt C++17 thì chọn gì, vì sao không bọc `std::mutex`?

**Rất tốt — ý (c) tự dựng được cơ chế vòng đời** (mô hình **publish/RCU**), không cần gợi ý:
> *"`atomic_load` tự tạo con trỏ copy tạm và đọc giá trị… khi `atomic_store` xảy ra đồng thời, object cũ vẫn an toàn nhờ `atomic_load` cùng thời điểm. Sau khi read kết thúc, count về 0, giải phóng object cũ."*

Đây đúng là cách hot-reload config trong shared library thật.

**Ba chỉnh nhỏ (không trừ điểm):**

1. **Gán nhầm tầng 3.** Ứng viên nói `use(cfg->timeout)` là race tầng 3. Trong snippet này **không** — writer tạo `Config` **hoàn toàn mới** rồi publish, **không sửa** object cũ; reader cầm object cũ đọc dữ liệu bất biến → an toàn. Tầng 3 chỉ thành race nếu writer làm `g_config->timeout = 5;` (sửa tại chỗ). **Bài học thiết kế:** chính vì *publish object mới thay vì sửa tại chỗ* mà tầng 3 biến mất — đó là lý do chọn pattern này.
2. **Gọi tên hậu quả tầng 2 chính xác hơn** ("dữ liệu hỏng hoặc crash" còn mơ hồ):
   > **Trích bank CPP-052:** *"`p = other` không atomic — giảm count cũ, ghi **hai** con trỏ, tăng count mới. Hai thread xen kẽ có thể để `p` trỏ **payload của A nhưng control block của B** → **double-free / leak**."*

   Triệu chứng trên board: crash trong destructor hoặc `free()`, backtrace trỏ vào chỗ **không liên quan** — refcount đã hỏng từ rất lâu trước đó.
3. **⚠️ Cảnh báo phiên bản (nói ra sẽ ghi điểm):** `std::atomic_load`/`atomic_store` trên `shared_ptr` **deprecated từ C++20**, **loại bỏ ở C++26**. Câu trả lời senior: *"C++17 thì em dùng `atomic_load/store` nhưng ghi chú lại vì nó deprecated; lên C++20 đổi sang `std::atomic<std::shared_ptr<T>>`."*
4. **Về việc bỏ mutex:** lập luận "overhead lớn" đúng nhưng chưa sắc — vấn đề chính là **serialize toàn bộ reader** (mọi thread xếp hàng dù chỉ đọc). Phương án trung gian đáng nhắc: `std::shared_mutex` (C++17) cho nhiều reader song song — thua atomic nhưng dễ đúng hơn.

---

## 💬 Calibration — ứng viên phản hồi "câu 1 ý (b) quá sâu cho mid-level"

| Ý | Kết luận |
|---|---|
| **Tên lệnh barrier ARM** (`stlr`/`ldar` vs `dmb ish`) | ✅ **Ứng viên đúng.** Kiến thức compiler-backend, **không chấm**, chỉ "biết thì tốt" |
| **Ý niệm "acq/rel gần như free trên x86, có phí thật trên ARM"** | 🟡 **Giữ.** Không cần tên lệnh, nhưng đây là lý do **tối ưu memory order đo trên máy dev x86 là vô nghĩa** — đúng lớp bug của JD ARM |
| **"Mọi người mặc định seq_cst không suy nghĩ"** | ✅ **Và đó là đúng** — tài liệu của chính repo khuyên vậy. Câu (a)/(c) đo *có biết vì sao mặc định đó tồn tại*, không đòi phải hạ mức |

**Thang chấm CPP-019 từ nay:** ý (a) căn cứ hạ mức + ý (c) ranh giới `relaxed` = **đủ 3 điểm**; tầng phần cứng chỉ nâng 3 → 4. (Đã ghi vào [weak-register](../weak-register.md).)

---

## Tổng kết

**Điểm mạnh:**
- **CPP-024 xuất sắc** — tự dựng mô hình publish/RCU cho hot-reload config, bằng chứng hiểu cơ chế chứ không thuộc bảng 3 tầng.
- Phản xạ *"bí thì về phương án an toàn"* (seq_cst) — đúng thái độ engineer.
- Phản hồi calibration hợp lý, đã điều chỉnh thang chấm.

**3 lỗ hổng ưu tiên:**
1. 🔴 **`explicit` cho conversion operator (safe-bool)** — [CPP-053](../bank/cpp.md), [oop.md](../../../01-cpp-fundamentals/oop.md). Liên quan trực tiếp việc đang làm (viết C++ API interface cho lớp trên).
2. 🟠 **Move ctor vs move assign + `noexcept` bắt buộc** — [CPP-054](../bank/cpp.md), [move-semantics.md](../../../02-modern-cpp/move-semantics.md), [raii-smart-pointers.md §6](../../../02-modern-cpp/raii-smart-pointers.md).
3. 🟡 **Nói "đo rồi mới hạ" khi bàn tối ưu** (không chỉ memory order) — [concurrency.md:124,176](../../../02-modern-cpp/concurrency.md#L124).

**Cập nhật đã làm:** weak-register (gỡ CPP-019/024/045; CPP-032 reset; thêm CPP-054) · bank +2 câu (CPP-053, CPP-054, đã verify compile `-Wall -Wextra`) · reviewed/ +2 file.

## Phiên kế đề xuất
- **Ngay sau đây (chốt Tuần 1):** bài tay ~15′ viết lại `ring_buffer.cpp` (bỏ `unordered_map` khỏi `push()`, thêm `dropped_count_`, `empty()/full()` an toàn đa luồng) → rồi `/mock comprehensive track cpp-system` (16 câu). Chèn CPP-032 + CPP-054 vào phiên đó để dọn sổ.
- **Tuần 2 Buổi 1:** `/mock rapid track linux-sysprog`.
