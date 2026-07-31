# 🔴 Weak Register — Sổ câu còn yếu (ưu tiên hỏi lại)

> Danh sách câu đã trả lời **sai / chưa chuẩn / lúng túng** (điểm ≤ 2 theo [config.md §4](config.md)) — interviewer **ưu tiên chèn lại** vào các phiên. Không có luật "đúng rồi thôi": câu ở đây được hỏi lại tới khi vững (≥ 3 điểm **hai lần liên tiếp** → gỡ khỏi sổ).
> Interviewer cập nhật file này ở Bước 4 mỗi phiên. Git-track để thấy điểm yếu co lại dần.

## Cách đọc
- **Lần gặp** = số phiên đã hỏi câu này. **Điểm gần nhất** = điểm 2 lần mới nhất (vd `2, 3`).
- Sắp xếp: câu điểm thấp + gặp nhiều lần mà chưa lên → **ưu tiên cao nhất**.

## Bảng câu yếu

| ID | Câu (tóm tắt) | Track | Lần gặp | Điểm gần nhất | Ghi chú lỗ hổng |
|----|---------------|-------|---------|----------------|-----------------|
| OS-003 | deadlock — 4 điều kiện Coffman + cách phá | cpp-system, bsp | 1 | 0 | không trả lời được; nhớ: mutual exclusion / hold-and-wait / no-preemption / circular wait; phá bằng lock ordering |
| DP-002 | Singleton hiện đại (Meyers static-local) | cpp-system, design-patterns | 1 | 2 | dùng `static` local trong `instance()` — thread-safe theo chuẩn, KHÔNG mutex mỗi get |
| CPP-029 | emplace_back vs push_back | cpp-system, modern-cpp | 2 | 2, 4 | ⏳ lần 2 đạt (4) — **cần thêm 1 lần ≥3 nữa** để gỡ. Bản chất = dựng tại chỗ từ ĐỐI SỐ ctor |

## Đã khắc phục (lưu vết — gỡ khỏi bảng trên khi ≥3 điểm hai lần)

| ID | Câu | Ngày gỡ |
|----|-----|---------|
| CPP-020 | Rule of 0/3/5 | 2026-07-29 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
