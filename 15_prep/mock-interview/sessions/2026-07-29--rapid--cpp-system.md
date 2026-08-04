# Phiên mock — 2026-07-29 · rapid · track cpp-system

- **Level:** mid-level · **Số câu:** 12 · **Thời lượng:** ~15 phút (rapid)
- **Điểm trung bình:** 3.17 / 4

## Kết quả từng câu (nhìn nhanh)
| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| CPP-001 | con trỏ vs tham chiếu | 3 | thiếu: reference không null + bind ngay lúc khai báo |
| CPP-004 | stack vs heap | 4 | thiếu 1 ý: heap khi cần sống lâu hơn scope / size runtime |
| CPP-007 | unique/shared/weak_ptr | 4 | đủ, nêu cyclic + thread-safe |
| CPP-008 | std::move thực sự làm gì | 4 | gọn, đúng bản chất |
| CPP-005 | RAII | 3 | thiếu "vì sao nền tảng": stack unwinding → exception-safe |
| CPP-010 | virtual destructor | 3 | chính xác hơn: là UB, không chỉ leak |
| CPP-006 | vtable/vptr | 3 | thiếu: lookup runtime, chi phí, chỉ class có virtual mới có vtable |
| CPP-020 | Rule of 0/3/5 | 2 | **nhầm copy↔move** |
| CPP-024 | shared_ptr thread-safe | 4 | control block atomic, payload tự bảo vệ |
| CPP-025 | circular reference | 4 | diễn giải ref_cnt kẹt rất rõ |
| OS-003 | deadlock 4 điều kiện Coffman | 0 | không trả lời được — câu ruột |
| OS-006 | mutex vs spinlock | 4 | vững |

## 🔎 Chi tiết ôn — câu điểm ≤ 3
> *(Retrofit 2026-08-03 theo format mới; log gốc chỉ có bảng.)*

### OS-003 — deadlock 4 điều kiện Coffman (điểm 0)
- **Thiếu/sai:** không trả lời được.
- **Bank:** > "Các thread chờ vòng tròn tài nguyên do nhau giữ nên kẹt vĩnh viễn. Bốn điều kiện cần đủ: **mutual exclusion, hold-and-wait, no preemption, circular wait**. Phá một điều kiện là tránh được — phổ biến nhất: **lock ordering** (luôn khóa nhiều mutex theo cùng thứ tự) để phá circular wait; hoặc `try_lock`+timeout, `scoped_lock`."
- **Tài liệu:** [sync-primitives.md](../../../03-operating-system/sync-primitives.md) (mục deadlock).
- **Chốt:** thuộc lòng 4 điều kiện; phá **circular wait** bằng **lock ordering** là cách thực chiến nhất.

### CPP-020 — Rule of 0/3/5 (điểm 2)
- **Thiếu/sai:** nhầm thành viên — Rule of 3 = {dtor, copy ctor, copy assign}; Rule of 5 mới thêm move.
- **Bank:** > "Rule of 3: cần tự viết 1 trong {destructor, copy ctor, copy assign} thì thường cần cả 3. Rule of 5: thêm move ctor + move assign. Rule of 0: thiết kế để không phải viết cái nào, bằng cách dùng member RAII (smart pointer, container) → compiler tự sinh đúng và an toàn."
- **Tài liệu:** [raii-smart-pointers.md §6](../../../02-modern-cpp/raii-smart-pointers.md) (Rule of 0/3/5 — trinity copy↔dtor, move steal+null).
- **Chốt:** 3 = trinity copy; 5 = +move; 0 = giao cho RAII type. *(Đã lên 4 các phiên sau → gỡ khỏi weak 2026-07-29.)*

### Câu điểm 3 (gọn)
- **CPP-001:** reference **không null** + phải bind ngay khi khai báo (con trỏ thì được).
- **CPP-005 (RAII):** nhấn "vì sao nền tảng" — exception → **stack unwinding** → dtor chạy → exception-safe.
- **CPP-010 (virtual dtor):** thiếu virtual dtor khi xóa qua con trỏ base = **UB**, không chỉ leak.
- **CPP-006 (vtable):** thêm — lookup **runtime**, chi phí gián tiếp, **chỉ class có virtual** mới có vptr/vtable.

## Tổng kết
- **Điểm mạnh:** smart pointer / move semantics / RAII / stack-heap rất chắc; diễn đạt gọn đúng kiểu rapid.
- **Lỗ hổng ưu tiên (top 3):**
  1. Rule of 3/5 — nhầm thành viên copy↔move (CPP-020).
  2. Deadlock / 4 điều kiện Coffman — chưa trả lời được (OS-003).
  3. Nêu "vì sao" sâu hơn ở câu 🟡 (RAII↔exception safety, virtual dtor↔UB) — nâng từ "đúng" lên "senior".
- **Câu mới thêm vào bank:** không có (tất cả đã có trong bank).
- **Cập nhật weak-register:** +CPP-020, +OS-003.

## Phiên kế đề xuất
- `by-level 🟡 track cpp-system` để vá các câu 🟡 còn nêu "vì sao" chưa sâu, HOẶC
- `daily` với 1 câu revisit tự động là CPP-020/OS-003 (weak-register sẽ tự chèn).
