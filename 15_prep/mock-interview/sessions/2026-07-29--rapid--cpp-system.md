# Phiên mock — 2026-07-29 · rapid · track cpp-system

- **Level:** mid-level · **Số câu:** 12 · **Thời lượng:** ~15 phút (rapid)
- **Điểm trung bình:** 3.17 / 4

## Kết quả từng câu
| ID | Câu (tóm tắt) | Tự đánh giá | Điểm | Ghi chú (thiếu gì / lỗ hổng) |
|----|---------------|-------------|------|------------------------------|
| CPP-001 | con trỏ vs tham chiếu | ok | 3 | thiếu: reference không null + phải bind ngay lúc khai báo |
| CPP-004 | stack vs heap | ok | 4 | thiếu 1 ý: heap khi cần sống lâu hơn scope / size runtime |
| CPP-007 | unique/shared/weak_ptr | ok | 4 | đủ, nêu cyclic + thread-safe |
| CPP-008 | std::move thực sự làm gì | ok | 4 | gọn, đúng bản chất |
| CPP-005 | RAII | ok | 3 | thiếu "vì sao nền tảng": stack unwinding khi exception → exception-safe |
| CPP-010 | virtual destructor | ok | 3 | chính xác hơn: là UB, không chỉ leak |
| CPP-006 | vtable/vptr | ok | 3 | thiếu: lookup runtime, chi phí, chỉ class có virtual mới có vtable |
| CPP-020 | Rule of 0/3/5 | lúng túng | 2 | **nhầm copy↔move**: Rule of 3 = {dtor, copy ctor, copy assign}; Rule of 5 thêm move |
| CPP-024 | shared_ptr thread-safe | ok | 4 | đúng: control block atomic, payload tự bảo vệ |
| CPP-025 | circular reference | ok | 4 | diễn giải ref_cnt kẹt rất rõ |
| OS-003 | deadlock 4 điều kiện Coffman | không nhớ | 0 | không trả lời được — câu ruột, phải thuộc |
| OS-006 | mutex vs spinlock | ok | 4 | vững |

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
