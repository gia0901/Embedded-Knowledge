# 🔴 Weak Register — Sổ câu còn yếu (ưu tiên hỏi lại)

> Danh sách câu đã trả lời **sai / chưa chuẩn / lúng túng** (điểm ≤ 2 theo [config.md §4](config.md)) — interviewer **ưu tiên chèn lại** vào các phiên. Không có luật "đúng rồi thôi": câu ở đây được hỏi lại tới khi vững (≥ 3 điểm **hai lần liên tiếp** → gỡ khỏi sổ).
> Interviewer cập nhật file này ở Bước 4 mỗi phiên. Git-track để thấy điểm yếu co lại dần.

## Cách đọc
- **Lần gặp** = số phiên đã hỏi câu này. **Điểm gần nhất** = điểm 2 lần mới nhất (vd `2, 3`).
- Sắp xếp: câu điểm thấp + gặp nhiều lần mà chưa lên → **ưu tiên cao nhất**.

## Bảng câu yếu

| ID | Câu (tóm tắt) | Track | Lần gặp | Điểm gần nhất | Ghi chú lỗ hổng |
|----|---------------|-------|---------|----------------|-----------------|
| CPP-019 | memory order — 3 mức + vì sao thận trọng | cpp-system, modern-cpp | 1 | 1 | không nhớ tên mức nào; thiếu **CPU cũng reorder** (không chỉ compiler). Học: `seq_cst` (mặc định) / `acquire-release` (cặp publish–subscribe) / `relaxed` (chỉ atomicity); chỉ hạ khi đã đo |
| CPP-024 | shared_ptr thread-safe (3 tầng) | cpp-system, modern-cpp | 2 | 4, 2 | 🔽 **tụt từ 4** — tưởng "ghi đồng thời cùng 1 instance `p = other` là an toàn vì count atomic". Đúng = control block ✅ / **instance ❌** / payload ❌. Xem [CPP-052](bank/cpp.md) |
| CPP-045 | `=delete` vs private cũ | cpp-system, modern-cpp | 1 | 2 | hiểu sai "sinh code tối ưu"; đúng = hàm **bị xóa**, lỗi lúc **compile** (private cũ = link), áp cho **hàm bất kỳ** không chỉ special members |
| CPP-032 | explicit / implicit conversion | cpp-system, modern-cpp | 1 | 2 | ví dụ đầu sai (copy ≠ conversion); đúng = chặn convert ngầm qua ctor 1-đối-số; ctor 1-đối-số mặc định nên `explicit` |
| CPP-009 | template ở header — compile-time | cpp-system, modern-cpp | 2 | 2, 3 | 🔼 lần 2 đạt 3 — **cần thêm 1 lần ≥3 nữa**. Đúng = *instantiation lúc compile*; def ở header vì mỗi TU cần thấy đủ định nghĩa |
| OS-007 | mutex vs semaphore | cpp-system, bsp | 2 | 2, 4 | 🔼 lần 2 đạt **4** (ownership + priority inheritance + cơ chế nâng priority + recursive) — **cần thêm 1 lần ≥3 nữa** để gỡ. Còn thiếu: phân vai *bảo vệ* (mutex) vs *báo hiệu bất đối xứng* (semaphore, ISR give → task take) |
| OS-003 | deadlock — 4 điều kiện Coffman + cách phá | cpp-system, bsp | 2 | 0, 3 | 🔼 lần 2 đạt 3 (đủ 4 điều kiện + lock ordering) — **cần thêm 1 lần ≥3 nữa** để gỡ |
| DP-002 | Singleton hiện đại (Meyers static-local) | cpp-system, design-patterns | 2 | 2, 3 | 🔼 lần 2 đạt 3 — **cần thêm 1 lần ≥3 nữa**. Bổ sung: tên cơ chế = **guard variable**; nhớ private ctor + copy `=delete` |

## Đã khắc phục (lưu vết — gỡ khỏi bảng trên khi ≥3 điểm hai lần)

| ID | Câu | Ngày gỡ |
|----|-----|---------|
| CPP-020 | Rule of 0/3/5 | 2026-07-29 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
| CPP-029 | emplace_back vs push_back | 2026-08-03 (điểm 2 → 4 → 4, đạt ≥3 hai lần liên tiếp) |
