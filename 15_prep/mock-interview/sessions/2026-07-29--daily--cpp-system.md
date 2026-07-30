# Phiên mock — 2026-07-29 · daily · track cpp-system

- **Level:** mid-level · **Số câu:** 6 · **Thời lượng:** ~15 phút
- **Điểm trung bình:** 3.0 / 4

## Kết quả từng câu
| ID | Câu (tóm tắt) | Tự đánh giá | Điểm | Ghi chú (thiếu gì / lỗ hổng) |
|----|---------------|-------------|------|------------------------------|
| CPP-028 | enum vs enum class | ok | 3 | thiếu: enum class chỉ định kiểu nền (uint8_t) — góc embedded |
| CPP-003 | const int* / int* const / const int* const | lúng túng câu giữa | 3 | `int* const` nói ngược: KHÔNG đổi nơi trỏ, đổi được *p |
| CPP-029 | emplace_back vs push_back | chưa chắc | 2 | bản chất = dựng tại chỗ từ ĐỐI SỐ ctor, không phải rvalue; bẫy bỏ qua explicit |
| CPP-044 | using alias vs typedef | ok | 3 | trúng alias template; diễn đạt "typedef không alias kiểu" hơi sai |
| OS-007 | mutex vs semaphore | ok | 3 | thiếu priority inheritance; khung mutex-thread/sem-process chưa chính xác |
| CPP-020 | Rule of 0/3/5 (revisit) | ok | 4 | ✅ lần này chuẩn (trước 2) — tiến bộ |

## Tổng kết
- **Điểm mạnh:** const/alias/enum/Rule-of-0-3-5 nắm được; diễn đạt gọn.
- **Lỗ hổng ưu tiên:**
  1. emplace_back vs push_back — hiểu lại "dựng tại chỗ từ đối số ctor" (CPP-029).
  2. int* const — cách đọc const/pointer (khoá giá trị vs khoá con trỏ).
  3. mutex vs semaphore — priority inheritance là lý do RTOS chọn mutex.
- **Câu mới thêm vào bank:** không có.
- **Cập nhật weak-register:** +CPP-029 (mới, điểm 2); CPP-020 lần 2 điểm 4 (1/2 lần tốt — chưa gỡ); OS-003 giữ nguyên (không hỏi phiên này).

## Phiên kế đề xuất
- `weak-review track cpp-system` để đóng CPP-020 (lần tốt thứ 2) + CPP-029, hoặc
- `daily track embedded` để bắt đầu vào domain EMB vừa dựng.
