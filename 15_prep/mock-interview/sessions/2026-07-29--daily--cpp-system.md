# Phiên mock — 2026-07-29 · daily · track cpp-system

- **Level:** mid-level · **Số câu:** 6 · **Thời lượng:** ~15 phút
- **Điểm trung bình:** 3.0 / 4

## Kết quả từng câu (nhìn nhanh)
| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| CPP-028 | enum vs enum class | 3 | thiếu: enum class chỉ định kiểu nền (uint8_t) — góc embedded |
| CPP-003 | const int* / int* const / … | 3 | `int* const` nói ngược: KHÔNG đổi nơi trỏ, đổi được `*p` |
| CPP-029 | emplace_back vs push_back | 2 | dựng tại chỗ từ **đối số ctor**; bẫy bỏ qua explicit |
| CPP-044 | using alias vs typedef | 3 | trúng alias template; "typedef không alias kiểu" hơi sai |
| OS-007 | mutex vs semaphore | 3 | thiếu priority inheritance; khung mutex/sem chưa chính xác |
| CPP-020 | Rule of 0/3/5 (revisit) | 4 | ✅ lần này chuẩn (trước 2) — tiến bộ |

## 🔎 Chi tiết ôn — câu điểm ≤ 3
> *(Retrofit 2026-08-03 theo format mới; log gốc chỉ có bảng.)*

### CPP-029 — emplace_back vs push_back (điểm 2)
- **Thiếu/sai:** hiểu là "nhận rvalue"; đúng là **nhận đối số của constructor** rồi forward dựng thẳng. Chưa nêu bẫy bỏ qua `explicit`.
- **Bank:** > "`push_back` nhận một **object đã dựng** rồi copy/move vào container. `emplace_back` nhận **đối số của constructor**, forward vào để **dựng object thẳng trong bộ nhớ container** — bỏ qua bước tạo object tạm." Bẫy: > "`emplace_back` dựng qua **direct-initialization** nên **bỏ qua `explicit`**... `vv.emplace_back(10)` ✅ gọi vector(size_t)=10 phần tử — thường NGOÀI Ý MUỐN; `vv.push_back(10)` ❌ bị chặn — an toàn hơn."
- **Tài liệu:** [move-semantics.md §5.1](../../../02-modern-cpp/move-semantics.md) (emplace vs push_back, 4 tình huống).
- **Chốt:** mặc định `push_back` (rõ + an toàn kiểu); `emplace_back` chỉ khi thật sự truyền **đối số ctor** của object đắt. *(Sau đó lên 4 → gỡ khỏi weak 2026-08-03.)*

### Câu điểm 3 (gọn)
- **CPP-003:** `int* const` = **con trỏ hằng** (không đổi *nơi trỏ*, đổi được `*p`); `const int*` = trỏ tới hằng. Đọc từ phải sang.
- **CPP-028:** enum class thêm — **chỉ định kiểu nền** (`: uint8_t`) để kiểm soát kích thước, hợp embedded.
- **CPP-044:** alias template đúng; nhưng typedef **vẫn** alias kiểu được cho ca thường — nó chỉ *không* template hoá được.
- **OS-007:** thiếu **priority inheritance** (lý do RTOS chọn mutex, không phải binary semaphore); ownership = ai lock nấy unlock. *(Xem chi tiết đầy đủ ở log 2026-08-03 rapid.)*

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
