# Phiên mock — 2026-08-07 · weak-review · toàn bộ sổ yếu

- **Level:** mid-level · **Số câu:** 8 (toàn bộ weak-register) · **Thời lượng:** ~25 phút
- **Điểm trung bình:** **3.88 / 4** — cao nhất từ đầu plan
- **Bối cảnh:** chèn trước Tuần 1 Buổi 5 theo cờ ⚠️ của [datalogic-plan](../../study-plans/datalogic-plan.md). Có ôn trước theo danh sách phân nhóm A/B.
- **Kết quả tổng: sổ yếu co từ 8 → 4 câu.** Toàn bộ Nhóm A (4 câu đang chờ lần ≥3 thứ hai) đều tốt nghiệp.

## Kết quả từng câu
| ID | Câu (tóm tắt) | Điểm | Kết quả |
|----|---------------|------|---------|
| CPP-009 | template ở header | 4 | ✅ **GỠ** (2→3→4) |
| CPP-045 | `=delete` vs private cũ | 4 | 🔼 2→4 (1/2 lần) |
| CPP-032 | `explicit` | 4 | 🔼 2→4 (1/2 lần) |
| DP-002 | Singleton Meyers | 4 | ✅ **GỠ** (2→3→4) |
| OS-003 | deadlock Coffman | 4 | ✅ **GỠ** (0→3→4) |
| OS-007 | mutex vs semaphore | 4 | ✅ **GỠ** (2→4→4) |
| CPP-024 | shared_ptr 3 tầng | 4 | 🔼 2→4 (1/2 lần) |
| CPP-019 | memory order (mức khó) | 3 | 🔼 2→3 (1/2 lần) |

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-019 — memory order (điểm 3, hỏi ở mức khó)
- **Hỏi gì:** không hỏi lại mức cơ bản (đã qua ngoài phiên 2026-08-06). Hỏi thẳng: (1) B `load` bằng **`relaxed`** trên đúng biến `ready` và thấy `true` — `data` có chắc = 42 không? (2) vì sao lớp bug này chạy đúng x86 nhưng chết ARM?
- **Trả lời:** ✅ đúng cả hai. `relaxed` chỉ đảm bảo atomic trên `ready`, không ràng buộc thứ tự với `data`; x86 memory model mạnh, ARM yếu.
- **Chỗ chỉnh — khung tư duy, không phải kết luận:** nói *"`data = 42` thậm chí có khi chưa được chạy"*. Trong thread A nó **đã chạy** (đúng program order). Vấn đề là **B chưa NHÌN THẤY** — ghi còn nằm trong **store buffer** của core A, hoặc chính B đảo thứ tự hai lệnh đọc. Phát biểu theo **visibility** (nhìn thấy) thay vì **execution** (đã chạy) mới chính xác, dù kết luận y hệt.
- **Chốt:** *"Không phải A chưa làm — mà là B chưa thấy."*
- **Lần sau hỏi mức thiết kế:** *"khi nào bạn CHỌN acquire/release thay vì mặc định seq_cst?"*

### Ghi chú các câu điểm 4 (để giữ nhịp)
- **OS-003:** thiếu duy nhất **`std::scoped_lock`** — cách C++17 idiomatic nhất, phá **hold-and-wait** (lấy tất cả lock một lần). `try_lock`+timeout mà bạn nêu là phá **no preemption** — đúng, nhưng nên có cả hai.
- **DP-002:** đủ ý; thiếu **private constructor** (chỉ nhắc copy `=delete`).
- **CPP-045:** ví dụ tự phát rất tốt — `void print(string) = delete;` chặn kiểu cụ thể. *(Lưu ý kỹ thuật: đây là **non-template overload** bị delete; bản đặc tả template là `template<> void print<string>(string) = delete;` — cả hai đều dùng được, ngữ nghĩa hơi khác.)*
- **CPP-024:** tự nêu được cơ chế tầng 2 (giảm count cũ → đổi 2 con trỏ → tăng count mới) → không phải học thuộc bảng. Lần sau hỏi *cách sửa*.

## 💬 Calibration — ứng viên phản hồi 3 lần "câu này quá sâu"

Ghi lại vì nó đổi cách chấm về sau. Phân định **không đồng đều**:

| Ý | Kết luận |
|---|---|
| **"guard variable"** (DP-002) | ✅ **Ứng viên đúng.** Tên gọi cơ chế ABI là quá sâu cho mid-level. Sổ yếu trước đây ghi "thiếu tên guard variable" **như một lỗ hổng** — sai calibration, đã sửa thành "biết thì tốt" |
| **explicit instantiation** (CPP-009) | 🟡 **Nửa đúng.** Không hỏi ở vòng sàng lọc; nhưng người viết **C++ shared library** dùng nó để giấu implementation template trong `.cpp` — hay xuất hiện ở vòng sâu bàn ABI/build time. Giữ mức "biết để nói khi bị hỏi tiếp" |
| **x86 vs ARM memory model** (CPP-019) | ❌ **Không đồng ý.** JD là **Embedded Linux trên ARM**. *"Vì sao chạy trên máy dev x86 mà chết trên board ARM"* là **lý do lớp bug này tồn tại trong công việc**, không phải trivia. Đây là câu phân biệt người từng debug trên target thật. Giữ nguyên |

> **Tiêu chí chốt để giữ một câu trong sổ yếu:** cơ chế nào **giải thích được một lớp bug sẽ gặp trong công việc** thì đáng học; **tên gọi nội bộ của compiler** thì không.

## Tổng kết
- **Điểm mạnh:** 7/8 câu đạt 4. Việc ôn trước theo phân nhóm A/B hiệu quả rõ rệt — mọi câu từng sai đều được sửa đúng chỗ từng sai (CPP-009 bỏ được "runtime/inline", CPP-032 bỏ được nhầm copy↔conversion, CPP-045 bỏ được "sinh code tối ưu"). Đây là bằng chứng cơ chế weak-register + ghi chú lỗ hổng cụ thể đang hoạt động.
- **Còn lại trong sổ (4 câu, đều ở 1/2 lần ≥3):** CPP-019, CPP-024, CPP-045, CPP-032 — chỉ cần **một** lần ≥3 nữa mỗi câu là sổ sạch.
- **Câu mới thêm vào bank:** không có.
- **Cập nhật weak-register:** GỠ CPP-009, DP-002, OS-003, OS-007 → mục "Đã khắc phục"; 4 câu còn lại cập nhật điểm + ghi chú "lần sau hỏi mức nào".

## Phiên kế đề xuất
- **Tuần 1 Buổi 5** (đúng lịch plan): `/mock coding track dsa` — reverse list, ring buffer, two-sum; viết vào [coding-arena/](../coding-arena/).
- **CN:** `/mock comprehensive track cpp-system` (ôn dồn tích Tuần 1) — chèn 4 câu còn lại của sổ yếu để dọn sạch.
