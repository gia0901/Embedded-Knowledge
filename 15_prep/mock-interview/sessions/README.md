# 📋 Sessions — Log phiên mock interview

> Mỗi phiên mock interview ghi **một file** vào thư mục này (git-track để theo dõi tiến bộ qua thời gian). Interviewer (Claude) tạo file ở **Bước 4** của [../config.md](../config.md) sau khi review xong.

## Quy ước tên file
`YYYY-MM-DD--<type>--<track>.md` — vd `2026-07-30--comprehensive--bsp.md`. Nếu nhiều phiên cùng ngày/type/track, thêm hậu tố `-2`.

## Mẫu file log

> **Nguyên tắc:** log phải **tự chứa** — mở lại là ôn được ngay, không cần lục chat. Bảng cho *nhìn nhanh*; mục 🔎 **persist nguyên văn phần review chi tiết** (trích bank + tài liệu gốc) mà Bước 3 config bắt buộc cho câu ≤3.

```markdown
# Phiên mock — <ngày> · <type> · track <track>

- **Level:** mid-level · **Số câu:** N · **Thời lượng:** ~X phút
- **Điểm trung bình:** M.M / 4
- **Bối cảnh:** (buổi nào / plan nào, nếu thuộc study-plan)

## Kết quả từng câu (nhìn nhanh)
| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| BSP-002 | boot chain nhiều giai đoạn | 3 | quên nêu TF-A BL1→BL33 |
| BSP-011 | coherent vs streaming DMA | 2 | thiếu dma_addr_t / cấm đụng buffer giữa map-unmap |
| COD-001 | reverse linked list | 4 | code sạch, O(1) space |

## 🔎 Chi tiết ôn — câu điểm ≤ 3 (BẮT BUỘC — persist review Bước 3)
> Với MỖI câu ưu tiên (nhất là ≤2): (a) thiếu/sai gì, (b) trích **đáp án bank** đúng chỗ thiếu, (c) trích **tài liệu gốc** (blockquote + link có neo dòng), (d) câu chốt để nhớ. Câu điểm 3 nhẹ có thể gộp 1–2 dòng.

### BSP-011 — coherent vs streaming DMA (điểm 2)
- **Thiếu/sai:** không phân biệt hướng DMA, quên `dma_addr_t`, không nói cấm CPU đụng buffer giữa map–unmap.
- **Bank:** > "Coherent: cấp buffer nhất quán cache, hợp mô tả/ring bền. Streaming: map/unmap từng lần, phải tôn trọng hướng DMA_TO/FROM_DEVICE, **CPU không đụng buffer khi đã map**."
- **Tài liệu:** > "…đoạn nguyên văn…" ([lkd/04-memory.md#Lxx](../../../16-book-summaries/lkd/04-memory.md))
- **Chốt:** streaming = "mượn tạm, trả nhanh"; sai hướng/đụng sớm → dữ liệu hỏng.

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
