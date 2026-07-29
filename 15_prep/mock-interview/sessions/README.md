# 📋 Sessions — Log phiên mock interview

> Mỗi phiên mock interview ghi **một file** vào thư mục này (git-track để theo dõi tiến bộ qua thời gian). Interviewer (Claude) tạo file ở **Bước 4** của [../config.md](../config.md) sau khi review xong.

## Quy ước tên file
`YYYY-MM-DD--<type>--<track>.md` — vd `2026-07-30--comprehensive--bsp.md`. Nếu nhiều phiên cùng ngày/type/track, thêm hậu tố `-2`.

## Mẫu file log

```markdown
# Phiên mock — <ngày> · <type> · track <track>

- **Level:** mid-level · **Số câu:** N · **Thời lượng:** ~X phút
- **Điểm trung bình:** M.M / 4

## Kết quả từng câu
| ID | Câu (tóm tắt) | Tự đánh giá | Điểm | Ghi chú (thiếu gì / lỗ hổng) |
|----|---------------|-------------|------|------------------------------|
| BSP-002 | boot chain nhiều giai đoạn | ok | 3 | quên nêu TF-A BL1→BL33 |
| BSP-011 | coherent vs streaming DMA | lúng túng | 2 | không nêu dma_addr_t / cấm đụng buffer giữa map-unmap |
| COD-001 | reverse linked list | ok | 4 | code sạch, nêu O(1) space |
| … |

## Tổng kết
- **Điểm mạnh:** …
- **Lỗ hổng ưu tiên (top 3):** 1) … 2) … 3) …
- **Câu mới thêm vào bank:** BSP-0xx (nếu có)
- **Cập nhật weak-register:** +BSP-011, +… / gỡ …

## Phiên kế đề xuất
- vd: `deep-dive track bsp` tập trung DMA/interrupt; hoặc `weak-review` cuối tuần.
```

## Xem tiến bộ
- Điểm trung bình theo thời gian, câu lặp lại nhiều lần mà vẫn thấp → xem [../weak-register.md](../weak-register.md).
- Lịch chạy: [../daily-plan.md](../daily-plan.md).
