# Phiên mock — 2026-07-29 · daily · track cpp-system (#2)

- **Level:** mid-level · **Số câu:** 6 · **Thời lượng:** ~15 phút
- **Điểm trung bình:** 3.33 / 4

## Kết quả từng câu
| ID | Câu (tóm tắt) | Tự đánh giá | Điểm | Ghi chú |
|----|---------------|-------------|------|---------|
| CPP-002 | struct vs class | ok | 3 | thiếu: khác cả default inheritance access |
| DP-002 | Singleton hiện đại | lúng túng | 2 | nói "mutex mỗi get" — thiếu Meyers static-local (thread-safe, không mutex) |
| CPP-026 | kỹ thuật C++17 | ok | 3 | 2 tính năng đúng (structured bindings, optional); kể thêm variant/string_view/if constexpr |
| CPP-046 | override/final | ok | 4 | xuất sắc, nêu compiler kiểm tra chữ ký |
| CPP-029 | emplace vs push_back (revisit) | ok | 4 | ✅ 2→4: forward đối số ctor + ca initializer_list. Đã ôn kỹ |
| CPP-020 | Rule of 0/3/5 (revisit) | ok | 4 | ✅ nắm bẫy dtor chặn sinh move. Đạt ≥3 lần 2 liên tiếp |

## Tổng kết
- **Điểm mạnh:** override/final, emplace, Rule of 0/3/5 — 2 câu weak-register đều lên 4 sau khi ôn tài liệu.
- **Lỗ hổng mới:** DP-002 Singleton (Meyers static-local, không mutex mỗi get).
- **Câu mới thêm vào bank:** không có.
- **Cập nhật weak-register:** GỠ CPP-020 (2,4,4 — ≥3 hai lần liên tiếp) → mục đã khắc phục; CPP-029 lên 4 (2,4 — 1/2, chưa gỡ); +DP-002 (mới, điểm 2); OS-003 giữ nguyên.

## Phiên kế đề xuất
- `daily`/`weak-review` chèn OS-003 (deadlock — chưa hỏi lại lần nào) + CPP-029 (lần tốt thứ 2 để gỡ) + DP-002.
- Hoặc đổi gió: `daily track embedded` để kiểm nền bare-metal.
