# 🎤 Phiên mock · 2026-08-16 · `rapid` · track `debugging`

- **Tham số:** `rapid` · 12 câu · trần T2 *(thực tế phiên `rapid` chỉ chạm **T1**)* · warm-up cho Tuần 2 Buổi 3
- **Điểm tổng: 3.67/4** — **cao nhất từ đầu plan** (ngang B5 3.67). **8/12 câu đạt 4.**
- **Lưu ý lịch:** §📍 ghi bài test của B3 là `/mock daily track debugging`. Phiên này là `rapid` — loại khác, đo thứ khác ⇒ **B3 chưa được tick** bằng phiên này.

> ⚠️ **Chấm theo [thang riêng của `rapid`](../config.md) (§4):** đo *đúng + gọn + bật ra ngay*. **Không trừ điểm vì thiếu chiều sâu** — T2 không được hỏi thì không được chấm. Ngược lại, **lan man cũng bị trừ**.
> ⇒ **3.67 hôm nay và 2.1 hôm qua KHÔNG mâu thuẫn** — xem mục "Chẩn đoán hai phiên" cuối file.

## Kết quả từng câu

| # | Câu | Điểm | Ghi chú |
|---|---|---|---|
| 1 | DBG-001 `-g` / `-O0` | **4** | Đủ cả hai vế, gọn |
| 2 | DBG-002 `step` vs `next` | **4** | Chuẩn xác |
| 3 | DBG-028 `strace` vs `ltrace` | **4** | Nêu đúng **ranh giới** user↔kernel vs app↔`.so` |
| 4 | DBG-003 core dump | **4** | Tự nêu góc *"phù hợp thiết bị ngoài field"* |
| 5 | DBG-005 strace cho *"không khởi động được"* | **3** | Có **loại lỗi**, thiếu **phương pháp** |
| 6 | DBG-007 phát hiện memory leak | **4** | ASan/Valgrind + RSS — khớp bank |
| 7 | DBG-024 loại lỗi bộ nhớ *(+probe)* | **4** | Liệt kê đủ; probe ra đúng lý do lõi |
| 8 | DBG-025 phòng ngừa bằng C++ hiện đại | **3** | RAII đúng; thiếu nửa bộ công cụ |
| 9 | DBG-018 debug trên target không gdb | **4** | 3 hướng + đánh đổi, không cần gợi |
| 10 | DBG-019 oops vs panic | **4** | Chuẩn, kèm *"cả hai in stack trace ra dmesg"* |
| 11 | DBG-027 `git bisect` *(+probe)* | **3** | 2/3 điều kiện chỉ ra **sau khi probe** |
| 12 | LNX-024 namespaces/cgroups 🔴*weak* | **3** 🔼 | **0 → 3.** Đã đọc bản bank viết lại |

---

## 🔎 Chi tiết ôn — bốn câu 3 điểm

### Câu 5 · DBG-005 · **3/4**

Chương trình báo lỗi mơ hồ *"không khởi động được"*, log không nói gì thêm. `strace` giúp bạn cái gì trong ca này?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ✅ liệt kê đúng các **loại**: file config không tồn tại · không có quyền access · không có quyền read/write · sai đường dẫn.

**❌ Vì sao chưa 4:** thiếu **phương pháp** — thứ interviewer thật muốn nghe.

**Đáp án đầy đủ:**
> Chạy `strace ./prog`, **nhìn syscall CUỐI CÙNG trước khi process thoát**. Gần như luôn là `openat` trả **`ENOENT`** (thiếu file) hoặc **`EACCES`** (thiếu quyền), hoặc `connect` trả `ECONNREFUSED`.

**Khác biệt cần thấy:** *"strace tìm được lỗi loại X"* = mô tả công cụ. *"Nhìn syscall cuối cùng trước khi chết"* = **cách dùng nó trong 10 giây**. Câu sau mới cho thấy đã thật sự dùng.

**Chốt:** *"strace biến 'không khởi động được' thành một dòng `ENOENT` chỉ đúng tên file đang thiếu."*

**Lần sau sẽ hỏi:** *"strace ra hàng nghìn dòng — bạn lọc thế nào để chỉ còn phần đáng nhìn?"*

**Ôn:** [09-debugging/tools.md](../../../09-debugging/tools.md) · bank [DBG-005](../bank/debugging.md)
</details>

### Câu 8 · DBG-025 · **3/4**

Làm sao **phòng ngừa** lỗi bộ nhớ ngay từ đầu bằng C++ hiện đại, thay vì đi bắt chúng về sau?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ✅ *"Tốt nhất: RAII (smart pointer, container, class wrapper viết đủ chuẩn) — cấp phát/giải phóng tự động, kể cả khi có exception."* ⚠️ vế hai: *"khác: quy ước viết code luôn để ý free, luôn check null…"*

**✅ Được:** RAII là câu trả lời đúng và bạn ra ngay, kèm ý **exception-safety** — phần nhiều người quên.

**❌ Vì sao chưa 4:** vế hai đi ngược tinh thần câu hỏi — *"quy ước luôn để ý free"* chính là thứ C++ hiện đại **thay thế**, không phải bổ sung. Và bộ công cụ mới nêu được một phần:

| Loại lỗi | Công cụ ngôn ngữ chặn nó |
|---|---|
| Leak / double-free / UAF | `unique_ptr`, `shared_ptr`, **Rule of Zero** |
| Buffer overflow | `std::array`, `.at()`, `std::span`, `string_view` — thay con trỏ trần + độ dài |
| Uninitialized read | khởi tạo tại chỗ khai báo, `{}` init |
| Dangling iterator | range-`for`, thuật toán `<algorithm>` thay index thủ công |

Cộng **hàng rào tự động**: `-Wall -Wextra -Werror` + ASan/UBSan **chạy trong CI**, không chỉ chạy tay.

**Chốt:** *"Không phải nhớ kỹ hơn — mà là làm cho lỗi đó **không viết ra được**."*

**Lần sau sẽ hỏi:** *"code cũ đầy `new`/`delete`, không được viết lại — bạn dựng hàng rào gì để bug mới không thêm vào?"*

**Ôn:** [09-debugging/memory-bugs.md](../../../09-debugging/memory-bugs.md) · [02-modern-cpp/raii-smart-pointers.md](../../../02-modern-cpp/raii-smart-pointers.md) · bank [DBG-025](../bank/debugging.md)
</details>

### Câu 11 · DBG-027 · **3/4**

`git bisect` hoạt động thế nào, và điều kiện gì phải có thì mới dùng được?

**🔁 Probe:** còn điều kiện nào nữa không — ngoài hai mốc good/bad?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ✅ cơ chế **binary search trên lịch sử commit** + cần một mốc OK và một mốc lỗi — ra ngay. Sau probe: ✅ *"phải reproduce được để khẳng định good/bad"* + *"mỗi commit build và chạy được"*.

**❌ Vì sao chưa 4:** hai trong ba điều kiện chỉ ra **sau khi bị probe**. Ở màn screen thật, câu hỏi thường dừng ở lần hỏi đầu — thứ không nói ra coi như không có.

> **Ba điều kiện bắt buộc:** ① **tái hiện được ổn định** ② **mỗi commit build & chạy được** ③ **biết một mốc TỐT**. — bank [DBG-027](../bank/debugging.md)

**Ý đắt chưa chạm:**
> Nó chỉ ra commit **làm bug lộ ra**, không nhất thiết là commit **có lỗi**. Rất thường gặp: một commit vô hại đổi thứ tự khởi tạo, làm lộ một race đã tồn tại từ lâu.

Cộng: **tự động hoá** (`git bisect run <script>` trả 0/khác 0) — với bug hiếm thì script chạy 200 lần rồi báo xấu nếu lỗi xuất hiện ≥1 lần.

**⚖️ Ghi chú của ứng viên về chất lượng bank — ĐÚNG, đã vá 16/08.** Bảng cũ viết *"commit hỏng giữa chừng làm **gãy chuỗi**"* rồi bỏ lửng, không nói gãy thế nào. Đã viết lại thành: không build ⇒ **không trả lời good/bad được** ⇒ bước đó không chia đôi được khoảng ⇒ `skip` thử commit lân cận; nếu **cả một vùng** không build được thì bisect báo *"There are only 'skip'ped commits left to test"* và trả về **một DANH SÁCH commit khả nghi** thay vì một commit.

**Chốt:** *"bisect là binary search trên lịch sử — 1000 commit chỉ cần 10 lần thử. Nhưng nó đòi bug tái hiện ổn định và mọi commit build được, nên nó thưởng cho thói quen commit nhỏ."*

**Lần sau sẽ hỏi:** *"bug xuất hiện 1/50 lần chạy — bisect còn dùng được không, làm thế nào?"* (định nghĩa "xấu" theo tỉ lệ + `bisect run`).

**Ôn:** [09-debugging/mindset.md](../../../09-debugging/mindset.md) · bank [DBG-027](../bank/debugging.md)
</details>

### Câu 12 · LNX-024 · **3/4** 🔼 *(0 → 3)*

**namespaces** và **cgroups** — mỗi cái giải quyết vấn đề gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ✅ *"namespaces: cô lập tầm nhìn, mỗi process có góc nhìn độc bản (pid, ipc, mount…), process ngoài không thấy, không đụng được."* ✅ *"cgroups: cô lập tài nguyên — process có vấn đề memory và crash thì không lan sang giết process khác."*

**✅ Được:** hôm qua (15/08) **trắng hoàn toàn**; hôm nay ra ngay cả hai vế. Rõ ràng đã đọc bản bank viết lại — đúng việc §📍 giao làm khởi động.

**❌ Vì sao chưa 4:** cgroups chỉ được mô tả qua **một** biểu hiện (OOM cục bộ). Nó rộng hơn — **giới hạn + hạch toán**:
- `MemoryMax=` (RAM) · `CPUQuota=` (CPU) · io limit · `TasksMax=` (chống fork bomb)
- Phần **hạch toán** (đo ai đang dùng bao nhiêu) quan trọng ngang phần giới hạn — nó là cách bạn *biết* app nào đang ăn tài nguyên trước khi phải giới hạn.

⚠️ **Đây mới là T1.** Tầng T2 vẫn còn nợ ⇒ **chưa gỡ khỏi sổ yếu**.

**Chốt:** *"Namespace trả lời 'thấy được gì', cgroup trả lời 'dùng được bao nhiêu'."*

**Lần sau sẽ hỏi (T2):** *"đặt `MemoryMax=64M` cho app đối tác rồi mà daemon của bạn vẫn chết — nghi gì?"*

**Ôn:** bank [LNX-024](../bank/linux-sysprog.md) *(viết lại 15/08)*. ⚠️ Repo vẫn **chưa có tài liệu topic** — nợ ⑦.1 ở plan.
</details>

---

## 📊 Chẩn đoán hai phiên liên tiếp — phần quan trọng nhất của log này

| Phiên | Loại | Đo cái gì | Điểm |
|---|---|---|---|
| 15/08 | `by-level` 🟠 | **T2** — vận dụng, sửa, đánh đổi | **2.1** |
| 16/08 | `rapid` | **T1** — biết là gì, nói gọn | **3.67** |

> **Nền kiến thức chắc; chỗ hụt nằm ở tầng VẬN DỤNG, không phải tầng BIẾT.**

Chẩn đoán này cụ thể hơn "điểm cao/thấp" và nó đổi cách ôn: **không cần đọc thêm tài liệu mới**, cần chuyển thứ đã biết thành **quyết định** và **dòng code sửa**. Trùng khớp lỗ hổng ① của phiên 15/08 (*"chẩn đoán được, sửa không được"*).

**Hệ quả cho phỏng vấn thật:** vòng **screen / khởi động** sẽ tạo ấn tượng tốt. Áp lực dồn vào **phần giữa**, khi interviewer chuyển sang *"giờ bạn làm gì"*.

**Quan sát về cách trả lời:** phiên này **không** còn phản xạ mở đầu bằng *"chưa rõ"* như hôm qua — nói thẳng, gọn, đúng nhịp rapid. Giữ cách này.

---

## 📌 Nợ phát sinh

| # | Nợ | Trạng thái |
|---|---|---|
| 1 | Bank `DBG-027` — cụm *"gãy chuỗi"* nói lửng, không giải thích chuyện gì xảy ra | ✅ **Đã vá 16/08** |
| 2 | **RSS chỉ được nhắc 1 dòng trong `09-debugging/`**, phần *"vì sao"* nằm ở topic khác mà **không có link nối**: `memory-bugs.md:85` ghi *"theo dõi RSS qua thời gian"*; giải thích thật (`free()` không trả RAM về OS, glibc giữ arena ⇒ **RSS không giảm là BÌNH THƯỜNG**, phải xem xu hướng) nằm ở [03-operating-system/memory-management.md](../../../03-operating-system/memory-management.md) | ⬜ Thêm link chéo — **ứng viên tự phát hiện** |
