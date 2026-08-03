# 🔴 Weak Register — Sổ câu còn yếu (ưu tiên hỏi lại)

> Danh sách câu đã trả lời **sai / chưa chuẩn / lúng túng** (điểm ≤ 2 theo [config.md §4](config.md)) — interviewer **ưu tiên chèn lại** vào các phiên. Không có luật "đúng rồi thôi": câu ở đây được hỏi lại tới khi vững (≥ 3 điểm **hai lần liên tiếp** → gỡ khỏi sổ).
> Interviewer cập nhật file này ở Bước 4 mỗi phiên. Git-track để thấy điểm yếu co lại dần.

## Cách đọc
- **Lần gặp** = số phiên đã hỏi câu này. **Điểm gần nhất** = điểm 2 lần mới nhất (vd `2, 3`).
- Sắp xếp: câu điểm thấp + gặp nhiều lần mà chưa lên → **ưu tiên cao nhất**.

## Bảng câu yếu

| ID | Câu (tóm tắt) | Track | Lần gặp | Điểm gần nhất | Ghi chú lỗ hổng |
|----|---------------|-------|---------|----------------|-----------------|
| CPP-009 | template ở header — compile-time | cpp-system, modern-cpp | 1 | 2 | **nói "runtime" trước khi sửa**; phản xạ đúng = *instantiation lúc compile*; def ở header vì mỗi TU cần thấy đủ định nghĩa (không thì linker undefined reference) |
| OS-007 | mutex vs semaphore | cpp-system, bsp | 1 | 2 | thiếu **ownership** (ai lock nấy unlock; semaphore ai signal cũng được) + **priority inheritance** (binary semaphore thay mutex mất nó → priority inversion) |
| OS-003 | deadlock — 4 điều kiện Coffman + cách phá | cpp-system, bsp | 2 | 0, 3 | 🔼 lần 2 đạt 3 (đủ 4 điều kiện + lock ordering) — **cần thêm 1 lần ≥3 nữa** để gỡ |
| DP-002 | Singleton hiện đại (Meyers static-local) | cpp-system, design-patterns | 2 | 2, 3 | 🔼 lần 2 đạt 3 — **cần thêm 1 lần ≥3 nữa**. Bổ sung: tên cơ chế = **guard variable**; nhớ private ctor + copy `=delete` |

## Đã khắc phục (lưu vết — gỡ khỏi bảng trên khi ≥3 điểm hai lần)

| ID | Câu | Ngày gỡ |
|----|-----|---------|
| CPP-020 | Rule of 0/3/5 | 2026-07-29 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
| CPP-029 | emplace_back vs push_back | 2026-08-03 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
