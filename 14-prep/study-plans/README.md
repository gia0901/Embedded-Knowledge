# 📋 Study Plans — Kế hoạch ôn tập theo vị trí

> Lớp **điều phối**: biến toàn bộ tài liệu của repo (topic 01–13 + book summary + question bank) thành **lịch ôn hành động được**, nhắm từng vị trí cụ thể. Đây không phải nội dung kiến thức mới — mọi mục đều **link tới tài liệu sẵn có**; chỗ nào chưa có thì ghi vào [gap-register.md](gap-register.md) để bổ sung sau.

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
| 🎯 **Embedded Linux Engineer @ Datalogic** (JD cụ thể) **← ĐANG CHẠY, DUY NHẤT** | [datalogic-plan.md](datalogic-plan.md) — **resume ở §📍 Tiến độ (đầu file)** | 4 tuần (dùng được cho 2 tuần nước rút): C++17, Linux sysprog+debug, kernel/driver I2C/SPI + **PCI/USB**, **Yocto**/CMake/CI, design + lead/review |

> **Chỉ còn một kế hoạch, cố ý.** Hai plan nghề chung chung trước đây (`bsp-plan.md`, `cpp-systemsw-plan.md`) đã **xoá 2026-08-09**: chúng trỏ vào bộ câu hỏi cũ nay không còn, không ai tick checkbox, và nội dung đã bị `datalogic-plan` phủ kín ở dạng cụ thể hơn (bám JD thật, có §📍 tracking sống). Ôn theo JD đang nhắm — khi có JD mới thì viết plan mới cho JD đó, đừng dựng lại plan generic.

## Tài liệu dùng chung cho cả hai hướng

- **Tự kiểm tra + phỏng vấn thử:** [mock-interview/](../mock-interview/) — **ngân hàng câu hỏi duy nhất** [bank/](../mock-interview/bank/) (mọi câu C++/OS/Linux/BSP/debug/DSA/design/network/behavioral, đáp án ẩn, phân độ khó, ID ổn định) + phiên phỏng vấn thử tương tác (`/mock`).
- **Lý thuyết cốt lõi:** chính là các topic **01–13** ở gốc repo — xem [OVERVIEW.md](../../OVERVIEW.md) để biết chúng nối với nhau thế nào.
- **Behavioral (STAR) + câu hỏi ngược + "vì sao rời công ty cũ":** [bank/behavioral.md](../mock-interview/bank/behavioral.md) — dùng cho vòng HR/behavioral của **mọi** vị trí (company-agnostic).

## Backlog

[gap-register.md](gap-register.md) — danh sách **"chủ đề đáng có mà repo chưa có"**, xếp theo ưu tiên. Cập nhật mỗi khi phát hiện lỗ hổng trong lúc ôn; là backlog để repo lớn lên có chủ đích (gồm cả các cuốn sách trong hàng đợi: TLPI, LDD3/Bootlin).

---

## Nhịp ôn gợi ý (template tuần)

Không cứng nhắc — điều chỉnh theo quỹ thời gian. Một tuần sprint điển hình:

| Buổi | Việc |
|------|------|
| 3–4 buổi đọc/hiểu | Đi các mục 🎯 của tuần đó (đọc summary + tự kiểm tra qua [bank/](../mock-interview/bank/)) |
| 1 buổi cuối tuần | **Mock interview tương tác**: gõ `/mock` (type `comprehensive` giả lập vòng thật) — theo §📍 của [datalogic-plan](datalogic-plan.md) |
| Xuyên suốt | Luyện **DSA** song song (vài bài LeetCode/tuần theo pattern — [12-dsa](../../12-dsa/)) |
| Khi phát hiện lỗ hổng | Ghi vào [gap-register.md](gap-register.md), không dừng mạch ôn để lấp ngay |
