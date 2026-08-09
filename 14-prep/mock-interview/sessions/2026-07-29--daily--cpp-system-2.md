# Phiên mock — 2026-07-29 · daily · track cpp-system (#2)

- **Level:** mid-level · **Số câu:** 6 · **Thời lượng:** ~15 phút
- **Điểm trung bình:** 3.33 / 4

## Kết quả từng câu (nhìn nhanh)
| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| CPP-002 | struct vs class | 3 | thiếu: khác cả default inheritance access |
| DP-002 | Singleton hiện đại | 2 | nói "mutex mỗi get" — thiếu Meyers static-local |
| CPP-026 | kỹ thuật C++17 | 3 | 2 tính năng đúng; kể thêm variant/string_view/if constexpr |
| CPP-046 | override/final | 4 | xuất sắc, nêu compiler kiểm tra chữ ký |
| CPP-029 | emplace vs push_back (revisit) | 4 | ✅ 2→4: forward đối số ctor + ca initializer_list |
| CPP-020 | Rule of 0/3/5 (revisit) | 4 | ✅ nắm bẫy dtor chặn sinh move — đạt ≥3 lần 2 liên tiếp |

## 🔎 Chi tiết ôn — câu điểm ≤ 3
> *(Retrofit 2026-08-03 theo format mới; log gốc chỉ có bảng.)*

### DP-002 — Singleton hiện đại (điểm 2)
- **Thiếu/sai:** nói "mutex mỗi lần get" — không cần; bỏ sót Meyers static-local (chuẩn đã lo thread-safe).
- **Bank:** > "Đảm bảo một class chỉ có một instance + điểm truy cập toàn cục. C++11+ dùng Meyers' Singleton: `static` local trong hàm `instance()` — khởi tạo **lazy** và **thread-safe theo chuẩn** (compiler sinh **guard variable**). Cấm copy (`= delete`), constructor private."
- **Tài liệu:** > "Chuẩn C++11 **bắt buộc**: nếu nhiều luồng cùng vào lần đầu, chỉ một luồng chạy khởi tạo, các luồng còn lại **chờ**... guard variable ẩn... Sau lần đầu chỉ là một atomic load." ([creational.md §Vì sao Meyers thread-safe](../../../11-design-patterns/creational.md))
- **Chốt:** `static` local lo thread-safe qua guard variable — **KHÔNG** mutex mỗi get. Nhớ private ctor + copy `=delete`. *(Chỉ thread-safe phần khởi tạo, không phải phần dùng.)*

### Câu điểm 3 (gọn)
- **CPP-002:** struct/class khác **cả default inheritance access** (struct→public, class→private); ngoài default thì giống hệt.
- **CPP-026:** kể thêm C++17 — `std::variant`, `string_view`, `if constexpr`, structured bindings, `std::optional`, fold expression.

## Tổng kết
- **Điểm mạnh:** override/final, emplace, Rule of 0/3/5 — 2 câu weak-register đều lên 4 sau khi ôn tài liệu.
- **Lỗ hổng mới:** DP-002 Singleton (Meyers static-local, không mutex mỗi get).
- **Câu mới thêm vào bank:** không có.
- **Cập nhật weak-register:** GỠ CPP-020 (2,4,4 — ≥3 hai lần liên tiếp) → mục đã khắc phục; CPP-029 lên 4 (2,4 — 1/2, chưa gỡ); +DP-002 (mới, điểm 2); OS-003 giữ nguyên.

## Phiên kế đề xuất
- `daily`/`weak-review` chèn OS-003 (deadlock — chưa hỏi lại lần nào) + CPP-029 (lần tốt thứ 2 để gỡ) + DP-002.
- Hoặc đổi gió: `daily track embedded` để kiểm nền bare-metal.
