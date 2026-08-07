# 🔴 Weak Register — Sổ câu còn yếu (ưu tiên hỏi lại)

> Danh sách câu đã trả lời **sai / chưa chuẩn / lúng túng** (điểm ≤ 2 theo [config.md §4](config.md)) — interviewer **ưu tiên chèn lại** vào các phiên. Không có luật "đúng rồi thôi": câu ở đây được hỏi lại tới khi vững (≥ 3 điểm **hai lần liên tiếp** → gỡ khỏi sổ).
> Interviewer cập nhật file này ở Bước 4 mỗi phiên. Git-track để thấy điểm yếu co lại dần.

## Cách đọc
- **Lần gặp** = số phiên đã hỏi câu này. **Điểm gần nhất** = điểm 2 lần mới nhất (vd `2, 3`).
- Sắp xếp: câu điểm thấp + gặp nhiều lần mà chưa lên → **ưu tiên cao nhất**.

## Bảng câu yếu

| ID | Câu (tóm tắt) | Track | Lần gặp | Điểm gần nhất | Ghi chú lỗ hổng |
|----|---------------|-------|---------|----------------|-----------------|
| CPP-019 | memory order — 3 mức + vì sao thận trọng | cpp-system, modern-cpp | 3 | 2, 3 | 🔼 **1/2 lần ≥3.** Đã vững: 3 mức, mặc định, release/acquire (3 ca), ca `relaxed` (nhãn phải có ở **cả hai đầu**), x86 vs ARM. Còn **một chỗ chỉnh khung tư duy**: nói *"`data=42` chưa được chạy"* → đúng phải là **B chưa NHÌN THẤY** (store buffer / B tự đảo 2 lệnh đọc) — *visibility*, không phải *execution*. Lần sau hỏi mức thiết kế: *"khi nào bạn CHỌN acquire/release thay vì seq_cst?"* |
| CPP-024 | shared_ptr thread-safe (3 tầng) | cpp-system, modern-cpp | 3 | 2, 4 | 🔼 **1/2 lần ≥3** — phục hồi sau cú tụt, tự nêu được cơ chế tầng 2 (giảm count cũ → đổi 2 con trỏ → tăng count mới). Lần sau hỏi: *cách sửa* (`atomic<shared_ptr>` C++20 / `atomic_load-store` / mỗi thread giữ copy riêng) |
| CPP-045 | `=delete` vs private cũ | cpp-system, modern-cpp | 2 | 2, 4 | 🔼 **1/2 lần ≥3** — đủ 3 tầng (compile vs link · message rõ · áp cho hàm bất kỳ + ví dụ template). Chỉ cần 1 lần ≥3 nữa |
| CPP-032 | explicit / implicit conversion | cpp-system, modern-cpp | 2 | 2, 4 | 🔼 **1/2 lần ≥3** — ví dụ `vector<int> a = 10` chuẩn, nêu đúng hậu quả. Chỉ cần 1 lần ≥3 nữa |

## Đã khắc phục (lưu vết — gỡ khỏi bảng trên khi ≥3 điểm hai lần)

| ID | Câu | Ngày gỡ |
|----|-----|---------|
| CPP-020 | Rule of 0/3/5 | 2026-07-29 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
| CPP-029 | emplace_back vs push_back | 2026-08-03 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
| CPP-009 | template ở header — compile-time | 2026-08-07 (2 → 3 → **4**; nói thẳng *instantiation lúc compile* + linker error, trả lời được cả explicit instantiation) |
| DP-002 | Singleton hiện đại (Meyers static-local) | 2026-08-07 (2 → 3 → **4**; lazy + thread-safe **chỉ ở khởi tạo** + copy `=delete`) |
| OS-003 | deadlock — 4 điều kiện Coffman | 2026-08-07 (0 → 3 → **4**; đủ 4 điều kiện, nêu đúng "phải thoả ĐỒNG THỜI", lock ordering + try_lock timeout) |
| OS-007 | mutex vs semaphore | 2026-08-07 (2 → 4 → **4**; mục đích + ownership + priority inheritance kèm cơ chế) |

> **Ghi chú calibration (2026-08-07):** tên gọi nội bộ của compiler (vd *guard variable* ở DP-002) **không** tính là lỗ hổng — chỉ là "biết thì tốt". Tiêu chí giữ một câu trong sổ: **cơ chế giải thích được một lớp bug sẽ gặp trong công việc**, không phải thuật ngữ thuộc lòng.
