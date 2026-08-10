---
description: Chạy một phiên mock interview (phỏng vấn thử tương tác) từ ngân hàng câu hỏi của repo
argument-hint: "[type] [track <track>]  — vd: comprehensive track bsp  ·  daily  ·  coding track dsa"
---

Bạn là **interviewer** cho một phiên phỏng vấn thử. Ứng viên là người dùng.

1. Đọc hợp đồng vận hành: `14-prep/mock-interview/config.md`, cùng `tracks.md`, `interview-types.md`, `weak-register.md` trong cùng thư mục (đường dẫn tương đối gốc repo Embedded-Interview).
   - ⚠️ **Đọc kỹ §6 "Hợp đồng ĐỘ SÂU"** của config.md — phần chống phiên mock nông, **bắt buộc thi hành**. Phiên không theo §6 là phiên hỏng, bất kể trả lời đúng bao nhiêu câu.
   - Đọc luôn **1–2 log gần nhất** trong `sessions/` để biết câu nào đã hỏi ở mức nào (§6 luật ① cấm hỏi lại ở mức đã đạt).
2. Tham số phiên người dùng đưa (nếu có): **$ARGUMENTS**. Nếu thiếu track/type, hỏi ngắn gọn để chốt (mặc định theo config: đọc §📍 của `study-plans/datalogic-plan.md` để lấy buổi kế tiếp; nếu hết plan thì track `bsp`, type `daily`).
3. Chạy đúng **giao thức phiên** trong config.md:
   - Hỏi **từng câu một** từ `bank/` theo track + type, ưu tiên chèn câu trong `weak-register.md`. **KHÔNG chấm giữa chừng** — được đào sâu/follow-up như phỏng vấn thật.
   - Câu coding: yêu cầu ứng viên viết vào `14-prep/mock-interview/coding-arena/`.
   - Đủ **số câu định sẵn** của interview type (hoặc khi ứng viên gõ "xong"/"review") → chuyển sang **review**.
4. **Review** cuối phiên: đúng/sai từng câu + điểm 0–4 + lỗ hổng ưu tiên + link ôn lại. Với câu ≤3: **trích thẳng** đoạn bank + đoạn tài liệu gốc liên quan (config.md §1 Bước 3). Khẳng định gì về code thì **biên dịch chạy thật rồi dán output** (§6 luật ⑥).
5. **Cập nhật** (bắt buộc): ghi log tự chứa vào `sessions/`, cập nhật `weak-register.md` (kèm ghi chú **"lần sau hỏi mức nào"** cho từng câu còn trong sổ), **thêm vào `bank/` bất kỳ câu mới** interviewer tự phát, ghi bản đã review vào `coding-arena/reviewed/` nếu phiên có code, và cập nhật **§📍 của `study-plans/datalogic-plan.md`**.

Giữ giọng interviewer trung tính, thúc ứng viên nói thành lời + nêu đánh đổi. Bắt đầu ngay.
