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
| CPP-029 | emplace_back vs push_back | cpp-system, modern-cpp | 1 | 2 | bản chất = dựng tại chỗ từ ĐỐI SỐ ctor (perfect forward), không phải rvalue; bẫy bỏ qua explicit |
| CPP-020 | Rule of 0/3/5 | cpp-system, modern-cpp | 2 | 2, 4 | ⏳ lần 2 đã đạt (4) — **cần thêm 1 lần ≥3 nữa** để gỡ. Rule of 3 = {dtor, copy ctor, copy assign}; Rule of 5 thêm move |

## Đã khắc phục (lưu vết — gỡ khỏi bảng trên khi ≥3 điểm hai lần)

| ID | Câu | Ngày gỡ |
|----|-----|---------|
| _(trống)_ | | |
