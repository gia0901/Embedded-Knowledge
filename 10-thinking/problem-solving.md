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

<details><summary>1) Khi gặp một vấn đề kỹ thuật chưa từng thấy, bạn tiếp cận thế nào?</summary>

Tôi theo một quy trình: trước hết **làm rõ** vấn đề thực sự là gì (phân biệt triệu chứng với gốc rễ), xác định ràng buộc (tài nguyên, thời gian, scale, tương thích) và tiêu chí thành công (tối ưu cho tốc độ, bộ nhớ, đơn giản hay độ tin cậy) — tránh lao vào giải sai bài. Sau đó **chia nhỏ** vấn đề lớn thành các phần giải được, **phác thảo vài hướng** và so sánh đánh đổi để chọn (thường bắt đầu từ giải pháp đơn giản nhất chạy được rồi tối ưu), **triển khai từng phần** với kiểm chứng liên tục, và cuối cùng **đánh giá** xem đã đúng và đủ tốt cho ràng buộc chưa, còn cải thiện gì. Xuyên suốt tôi suy luận từ bản chất (first principles) thay vì áp khuôn máy móc, và nêu rõ các đánh đổi đang chấp nhận.
</details>

<details><summary>2) "Suy luận từ first principles" nghĩa là gì? Cho ví dụ.</summary>

Là quay về các nguyên lý cơ bản nhất và suy luận lên, thay vì sao chép giải pháp/khuôn mẫu một cách máy móc ("cargo cult"). Cách làm là liên tục hỏi "tại sao" cho tới khi chạm nguyên lý không thể chia nhỏ hơn, rồi xây giải pháp từ đó. Ví dụ: thay vì mặc định "cần mutex để bảo vệ biến này", hỏi tại sao cần mutex — vì có dữ liệu chia sẻ bị nhiều thread ghi đồng thời; từ đó nhận ra nếu thiết kế để mỗi thread giữ bản dữ liệu riêng (không chia sẻ trạng thái mutable) thì loại bỏ được nhu cầu khóa hoàn toàn, vừa đúng vừa nhanh hơn. Hoặc khi tối ưu: thay vì đoán thuật toán chậm, đo và phát hiện thời gian tốn ở số lần truy cập I/O — vấn đề gốc khác hẳn giả định ban đầu.
</details>

<details><summary>3) Vì sao nói "không có giải pháp tốt nhất, chỉ có đánh đổi"? Cho vài trục đánh đổi.</summary>

Vì mọi quyết định kỹ thuật đều hy sinh mặt này để được mặt kia, và "tốt nhất" chỉ có nghĩa trong một ngữ cảnh/ràng buộc cụ thể — một giải pháp tối ưu trên server có thể tệ trên MCU 64KB RAM. Các trục đánh đổi kinh điển: thời gian chạy ↔ bộ nhớ (lookup table nhanh nhưng tốn RAM vs tính lại); hiệu năng ↔ đơn giản/dễ bảo trì; tốc độ phát triển ↔ mức tối ưu (dùng thư viện sẵn vs tự viết); linh hoạt ↔ đơn giản (cấu hình động vs hard-code); throughput ↔ latency (batch lớn vs đáp ứng nhanh từng item); tổng quát ↔ chuyên biệt. Kỹ sư giỏi nêu rõ đang tối ưu cho cái gì trong ràng buộc nào, thay vì giả vờ có lựa chọn hoàn hảo.
</details>

<details><summary>4) Khi bế tắc với một bài toán, bạn làm gì?</summary>

Một số kỹ thuật tôi dùng: **đơn giản hóa** — giải một phiên bản dễ hơn (ít ràng buộc, input nhỏ) trước rồi tổng quát hóa; **đổi góc nhìn** — nghĩ ngược từ kết quả mong muốn lùi lại, hoặc đổi cách biểu diễn vấn đề (đồ thị, máy trạng thái); tìm **tương tự** với bài đã biết; xét **trường hợp biên** (rỗng, nhỏ nhất, lớn nhất) để lộ cấu trúc; **rubber duck** — giải thích thành lời cho người/vật khác để tự phát hiện lỗ hổng trong giả định; và **tách concern** để cô lập một khía cạnh thay vì giải mọi thứ cùng lúc. Quan trọng là không ngồi im đoán mò mà chủ động đổi cách tiếp cận và kiểm chứng.
</details>

<details><summary>5) Trong phỏng vấn, vì sao nên "think aloud"?</summary>

Vì người phỏng vấn đánh giá **cách bạn tư duy**, không chỉ đáp án cuối — và họ không đọc được suy nghĩ trong đầu bạn. Nói ra giả định, hướng tiếp cận, lý do chọn/loại từng phương án cho thấy bạn suy luận có hệ thống và biết cân nhắc đánh đổi. Nên bắt đầu bằng làm rõ yêu cầu, trình bày giải pháp đơn giản trước rồi cải thiện dần, và khi gặp sai/bí thì nói rõ "tôi thử hướng khác vì..." để thể hiện khả năng tự sửa. Ngồi im giải trong đầu rồi đưa đáp án — kể cả đúng — thường mất điểm so với người trình bày rõ ràng quá trình, vì giao tiếp kỹ thuật là một phần năng lực của kỹ sư, đặc biệt ở cấp Senior.
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [system-design.md](system-design.md)
