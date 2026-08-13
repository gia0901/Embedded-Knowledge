# 📋 Sessions — Log phiên mock interview

> Mỗi phiên mock interview ghi **một file** vào thư mục này (git-track để theo dõi tiến bộ qua thời gian). Interviewer (Claude) tạo file ở **Bước 4** của [../config.md](../config.md) sau khi review xong.

## Quy ước tên file
`YYYY-MM-DD--<type>--<track>.md` — vd `2026-07-30--comprehensive--bsp.md`. Nếu nhiều phiên cùng ngày/type/track, thêm hậu tố `-2`.

## Mẫu file log

> **Nguyên tắc:** log phải **tự chứa** — mở lại là ôn được ngay, không cần lục chat.

### ⚠️ Ba luật BẮT BUỘC về hình thức (rút từ góp ý 2026-08-13)

**① CHÉP NGUYÊN ĐỀ BÀI — kể cả code.**
Ghi *"CPP-030 size/capacity"* rồi bên dưới phân tích ba trang là **log hỏng**: đọc lại không biết đang nói về cái gì. Mỗi câu phải có **nguyên văn đề bài + snippet code y như lúc hỏi**, rồi mới tới nhận xét. Đây là điều kiện của "tự chứa", không phải tuỳ chọn.

**② TRÌNH BÀY THEO KIỂU BANK — đề mở, feedback + đáp án ẩn trong `<details>`.**
```markdown
### Câu N · <ID> · <level> · **<điểm>/4**

<nguyên văn đề bài, đủ code>

**🔁 Follow-up:** <nguyên văn follow-up đã hỏi>

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** <tóm tắt trung thực, đánh dấu ✅/❌ từng ý>
**✅ Được:** … **❌ Vì sao mất điểm:** …
**Đáp án đầy đủ:** <cơ chế + bảng + code>
**Chốt:** <một câu>
**Lần sau sẽ hỏi:** <góc mới — chép sang weak-register>
**Ôn:** <link tài liệu + bank>
</details>
```
Để đề **mở** thì lần sau còn tự trả lời lại được; đáp án **ẩn** nên không lộ. **Không ngại lặp nội dung bank** — giá trị của log nằm ở chỗ *lỗi của chính mình đặt cạnh đáp án chuẩn*, thứ bank không có.

**③ CÂU ĐIỂM 3 PHẢI GIẢI THÍCH ĐẦY ĐỦ NHƯ CÂU ĐIỂM 2.**
Điểm 3 là *"đạt mức mid"*, **không** phải "xong rồi". Gộp các câu 3 vào một danh sách gạch đầu dòng ngắn là **sai** — đó chính là khoảng cách giữa 3 và 4, tức chỗ cần đọc nhất. Mỗi câu 3 phải nói rõ: **được gì · vì sao chưa 4 · đáp án đầy đủ của phần còn thiếu**.

> Áp cho **mọi** câu ≤ 3. Câu 4 thì gọn hơn được, nhưng vẫn giữ nguyên đề bài và ghi phần bổ sung.

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
- **Tài liệu:** > "…đoạn nguyên văn…" ([lkd/04-memory.md#Lxx](../../../15-book-summaries/lkd/04-memory.md))
- **Chốt:** streaming = "mượn tạm, trả nhanh"; sai hướng/đụng sớm → dữ liệu hỏng.

## 💬 Calibration (chỉ khi ứng viên phản hồi "câu này quá sâu / lệch trọng tâm")
> Ghi lại vì nó **đổi cách chấm về sau**. Phân định TỪNG Ý: ✅ đồng ý (hạ khỏi tiêu chí chấm) · 🟡 nửa đúng (giữ mức "biết thì tốt") · ❌ không đồng ý (nêu lý do gắn với JD). Kết luận chép sang mục calibration của [../weak-register.md](../weak-register.md).

## Tổng kết
- **Điểm mạnh:** …
- **Lỗ hổng ưu tiên (top 3):** 1) … 2) … 3) …
- **Câu mới thêm vào bank:** BSP-0xx (nếu có)
- **Cập nhật weak-register:** +BSP-011 / gỡ BSP-002 → **đã xếp lịch kiểm tra lại: Tuần N+2**
- **Bản code đã review:** `../coding-arena/reviewed/YYYY-MM-DD--<ID>--<slug>.cpp` (nếu phiên có code)
- **Cập nhật §📍 plan:** đã tick buổi … + sửa 4 dòng trạng thái

## Phiên kế đề xuất
- vd: `deep-dive track bsp` tập trung DMA/interrupt; hoặc `weak-review` cuối tuần.
```

> **4 đầu ra BẮT BUỘC của một phiên** (config Bước 4) — log này chỉ là 1 trong 4: ① `sessions/` · ② `weak-register.md` (gỡ câu ⟹ **phải** xếp vào bảng 🔁 Lịch kiểm tra lại) · ③ `bank/` (câu tự phát) · ④ `coding-arena/reviewed/` (nếu có code). Cộng thêm **§📍 của plan** nếu phiên thuộc study-plan.

## Xem tiến bộ
- Điểm trung bình theo thời gian, câu lặp lại nhiều lần mà vẫn thấp → xem [../weak-register.md](../weak-register.md).
- Lịch chạy: §📍 của [datalogic-plan](../../study-plans/datalogic-plan.md).
