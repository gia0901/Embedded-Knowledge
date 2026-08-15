# Problem Solving — Tư duy giải quyết vấn đề

> **TL;DR**
> - Senior khác Middle không phải ở chỗ "biết nhiều" mà ở **cách tiếp cận vấn đề**: làm rõ trước khi giải, chia nhỏ, suy luận từ bản chất (first principles), và luôn cân nhắc **đánh đổi (trade-off)**.
> - Quy trình: **Hiểu (clarify) → Chia nhỏ (decompose) → Lập kế hoạch (plan) → Thực thi (execute) → Đánh giá (evaluate)**.
> - "Không có giải pháp đúng tuyệt đối, chỉ có đánh đổi phù hợp với ngữ cảnh." Luôn hỏi: *ràng buộc là gì? tối ưu cho cái gì?*
> - Khi bí: quay về **bản chất** (vấn đề thật sự là gì?), đơn giản hóa, hoặc đổi góc nhìn.
> - Trong phỏng vấn: **nói ra suy nghĩ** (think aloud) — người phỏng vấn chấm *cách bạn nghĩ*, không chỉ đáp án.

---

## 1. Vì sao "cách nghĩ" quan trọng hơn "biết đáp án"?

Kiến thức tra được; tư duy thì không. Một vấn đề thực tế hiếm khi giống hệt cái đã học — điều phân biệt kỹ sư giỏi là khả năng **tiếp cận cái chưa biết** một cách có hệ thống: đặt đúng câu hỏi, phân rã, suy luận, và biết mình đang đánh đổi gì. Đây cũng là điều phỏng vấn Senior thật sự đo, đặc biệt qua câu hỏi mở.

---

## 2. Quy trình 5 bước

```
① CLARIFY    → Hiểu đúng vấn đề. Đừng giải sai bài.
② DECOMPOSE  → Chia thành phần nhỏ giải được.
③ PLAN       → Phác thảo hướng giải + đánh đổi, chọn hướng.
④ EXECUTE    → Triển khai từng phần, kiểm chứng dần.
⑤ EVALUATE   → Đúng chưa? đủ tốt chưa? còn cải thiện gì?
```

### ① Clarify — làm rõ trước khi giải
Sai lầm lớn nhất là **lao vào giải sai bài**. Trước hết:
- Vấn đề **thực sự** là gì? (triệu chứng vs gốc rễ)
- **Ràng buộc**: tài nguyên (RAM/CPU/điện), thời gian, tương thích, scale?
- **Tiêu chí thành công**: tối ưu cho tốc độ? bộ nhớ? đơn giản? độ tin cậy?
- **Giả định**: cái gì cho trước, cái gì phải hỏi?

> Trong phỏng vấn, hỏi làm rõ là **điểm cộng**, không phải dấu hiệu yếu. "Input lớn cỡ nào? cần realtime không? chạy trên thiết bị nào?" cho thấy tư duy kỹ sư.

### ② Decompose — chia để trị
Vấn đề lớn = tập hợp vấn đề nhỏ giải được. Tách theo chức năng, theo tầng, theo luồng dữ liệu. Một bài "không biết bắt đầu từ đâu" thường tan biến khi chia đúng thành các mảnh quen thuộc.

### ③ Plan — phác thảo & chọn hướng
Nghĩ ≥2 cách tiếp cận, so sánh đánh đổi, **chọn có lý do**. Bắt đầu từ giải pháp đơn giản nhất chạy được (brute force/naive) rồi tối ưu — đừng nhảy ngay vào tối ưu phức tạp.

### ④ Execute — triển khai & kiểm chứng dần
Làm từng phần, kiểm tra mỗi bước thay vì viết hết rồi mới chạy. Giữ vòng phản hồi ngắn.

### ⑤ Evaluate — nhìn lại
Giải pháp đúng chưa (test edge case)? đủ tốt cho ràng buộc chưa? Đánh đổi đã chấp nhận có hợp lý? Còn đơn giản hóa được không?

---

## 3. First principles — suy luận từ bản chất

Thay vì áp dụng công thức/khuôn mẫu một cách máy móc, quay về **nguyên lý cơ bản** và suy luận lên:

- "Vì sao cần mutex ở đây?" → vì có dữ liệu chia sẻ bị ghi đồng thời → vậy nếu *không chia sẻ* (mỗi thread bản riêng) thì không cần mutex.
- "Vì sao chậm?" → đo thấy tốn ở I/O → vậy vấn đề không phải thuật toán mà là số lần truy cập đĩa.

First-principles giúp tránh "cargo cult" (làm theo vì người ta làm vậy) và tìm ra giải pháp gốc. Hỏi liên tục **"tại sao"** cho tới khi chạm nguyên lý không thể chia nhỏ hơn.

---

## 4. Trade-off — không có giải pháp hoàn hảo

Mọi quyết định kỹ thuật là đánh đổi. Kỹ sư giỏi *nêu rõ* đánh đổi thay vì giả vờ có lựa chọn hoàn hảo. Các trục đánh đổi kinh điển:

| Trục | Ví dụ |
|------|-------|
| Thời gian ↔ Bộ nhớ | Lookup table (nhanh, tốn RAM) vs tính lại (chậm, ít RAM) |
| Hiệu năng ↔ Đơn giản/Bảo trì | Tối ưu vi mô vs code dễ đọc |
| Tốc độ phát triển ↔ Tối ưu | Dùng thư viện sẵn vs tự viết |
| Linh hoạt ↔ Đơn giản | Cấu hình động vs hard-code |
| Throughput ↔ Latency | Batch lớn vs đáp ứng nhanh từng item |
| Tổng quát ↔ Chuyên biệt | Template/generic vs giải pháp riêng tối ưu |

> Câu hỏi luôn đặt: **"tối ưu cho cái gì, trong ràng buộc nào?"** Một giải pháp tốt trên server có thể tệ trên MCU 64KB RAM.

---

## 5. Kỹ thuật khi bế tắc

- **Đơn giản hóa**: giải phiên bản dễ hơn trước (ít ràng buộc, input nhỏ), rồi tổng quát hóa.
- **Đổi góc nhìn**: nghĩ ngược (từ kết quả mong muốn lùi lại), hoặc đổi đại diện (không gian, đồ thị, trạng thái).
- **Tương tự (analogy)**: bài này giống bài nào đã biết?
- **Trường hợp biên**: xét input nhỏ nhất/lớn nhất/rỗng — thường lộ cấu trúc.
- **Rubber duck**: giải thích to thành lời cho người/vật khác — lỗ hổng tự lộ.
- **Tách concern**: đừng giải mọi thứ cùng lúc; cô lập một khía cạnh.

---

## 6. Trong phỏng vấn: think aloud

Người phỏng vấn không đọc được suy nghĩ — họ chấm **quá trình**:
- **Nói ra** giả định, hướng tiếp cận, lý do chọn/loại.
- Bắt đầu bằng làm rõ yêu cầu (mục ①).
- Nêu giải pháp naive trước, rồi cải thiện — cho thấy bạn biết đánh đổi.
- Khi sai/bí, nói "thử lại theo hướng khác vì..." — thể hiện khả năng tự sửa.
- Im lặng giải trong đầu = mất điểm dù ra đáp án. Giao tiếp là một phần của kỹ năng.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [SD-001](../14-prep/mock-interview/bank/system-design.md) | Khi gặp một vấn đề kỹ thuật chưa từng thấy, bạn tiếp cận thế nào? |
| [SD-003](../14-prep/mock-interview/bank/system-design.md) | "Suy luận từ first principles" nghĩa là gì? Cho ví dụ. |
| [SD-002](../14-prep/mock-interview/bank/system-design.md) | Vì sao nói "không có giải pháp tốt nhất, chỉ có đánh đổi"? Cho vài trục đánh đổi. |
| [SD-032](../14-prep/mock-interview/bank/system-design.md) | Khi bế tắc với một bài toán, bạn làm gì? |
| [SD-033](../14-prep/mock-interview/bank/system-design.md) | Trong phỏng vấn, vì sao nên "think aloud"? |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [system-design.md](system-design.md)
