# 📋 Study Plans — Kế hoạch ôn tập theo vị trí

> Lớp **điều phối**: biến toàn bộ tài liệu của repo (topic 01–14 + book summary + question bank) thành **lịch ôn hành động được**, nhắm từng vị trí cụ thể. Đây không phải nội dung kiến thức mới — mọi mục đều **link tới tài liệu sẵn có**; chỗ nào chưa có thì ghi vào [gap-register.md](gap-register.md) để bổ sung sau.

---

## Cách dùng

Mỗi kế hoạch có **hai tầng** (đọc song song, không loại trừ nhau):

- **🏃 Tầng SPRINT** — dày đặc, cho đợt phỏng vấn sắp tới. Chỉ đi các mục **🎯 xác suất gặp cao**, tổ chức theo tuần, kèm vòng tự kiểm tra bằng question bank. Mục tiêu: *"nói được bản chất"* các câu ruột, không học vẹt.
- **📚 Tầng NỀN TẢNG** — phủ toàn diện, dài hơi, cho mục tiêu Middle+ → Senior. Không gắn ngày cụ thể; đánh dấu tiến độ khi hoàn thành.

**Quy trình mỗi buổi ôn (áp cho cả hai tầng):**
1. Đọc mục 🎯 trong kế hoạch → mở tài liệu được link (ưu tiên **book summary** cho chiều sâu, **topic** cho bản cô đọng).
2. **Tự trả lời trước** các câu hỏi liên quan (che đáp án) → mở `<details>` đối chiếu.
3. Chỗ trả lời sai/lúng túng → ghi lại; nếu là **lỗ hổng tài liệu** (repo chưa có) → thêm vào [gap-register.md](gap-register.md).

Ký hiệu trong checklist: `- [ ]` chưa ôn · `- [x]` đã ôn vững · **🎯** thuộc tầng sprint (ưu tiên cao) · **🕳️** tài liệu còn thiếu (xem gap register).

---

## Các kế hoạch

| Vị trí | Kế hoạch | Trọng tâm |
|--------|----------|-----------|
| **Embedded SW Engineer (BSP)** | [bsp-plan.md](bsp-plan.md) | boot/U-Boot, device tree/probe, kernel internals, interrupt/DMA, storage/OTA, real-time |
| **System Software / C++ Engineer** | [cpp-systemsw-plan.md](cpp-systemsw-plan.md) | modern C++, ABI/shared library, concurrency, design pattern, OS fundamentals |

> Hai vị trí **dùng chung nhiều phần nền** (OS, concurrency, debug, DSA). Kế hoạch BSP nghiêng về kernel/phần cứng; kế hoạch C++ nghiêng về ngôn ngữ/ABI. Nếu ứng tuyển cả hai, đi tầng sprint của vị trí sắp phỏng vấn trước, phần chung tự khớp.

## Tài liệu dùng chung cho cả hai hướng

- **Tự kiểm tra:** [technical_round/02_question_bank.md](../technical_round/02_question_bank.md) (nền chung C++/system/bus/debug), [04_bsp_question_bank.md](../technical_round/04_bsp_question_bank.md) (chuyên sâu BSP), [11-interview-questions/](../../11-interview-questions/) (đáp án ẩn, phân độ khó).
- **Lý thuyết cốt lõi:** [technical_round/01_theory.md](../technical_round/01_theory.md).
- **Behavioral (STAR) + câu hỏi ngược + "vì sao rời công ty cũ":** [technical_round/03_behavior.md](../technical_round/03_behavior.md) — dùng cho vòng HR/behavioral của **mọi** vị trí (company-agnostic).

## Backlog

[gap-register.md](gap-register.md) — danh sách **"chủ đề đáng có mà repo chưa có"**, xếp theo ưu tiên. Cập nhật mỗi khi phát hiện lỗ hổng trong lúc ôn; là backlog để repo lớn lên có chủ đích (gồm cả các cuốn sách trong hàng đợi: TLPI, LDD3/Bootlin).

---

## Nhịp ôn gợi ý (template tuần)

Không cứng nhắc — điều chỉnh theo quỹ thời gian. Một tuần sprint điển hình:

| Buổi | Việc |
|------|------|
| 3–4 buổi đọc/hiểu | Đi các mục 🎯 của tuần đó (đọc summary + tự kiểm tra question bank) |
| 1 buổi cuối tuần | **Mock interview**: tự nói to đáp án các câu ⭐/🎯, hoặc nhờ người hỏi; đối chiếu [11-interview-questions](../../11-interview-questions/) |
| Xuyên suốt | Luyện **DSA** song song (vài bài LeetCode/tuần theo pattern — [13-dsa](../../13-dsa/)) |
| Khi phát hiện lỗ hổng | Ghi vào [gap-register.md](gap-register.md), không dừng mạch ôn để lấp ngay |
