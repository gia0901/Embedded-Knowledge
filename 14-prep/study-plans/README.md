# 📋 Study Plans — Kế hoạch ôn tập theo vị trí

> Lớp **điều phối**: biến toàn bộ tài liệu của repo (topic 01–13 + book summary + question bank) thành **lịch ôn hành động được**, nhắm từng vị trí cụ thể. Đây không phải nội dung kiến thức mới — mọi mục đều **link tới tài liệu sẵn có**; chỗ nào chưa có thì ghi vào [gap-register.md](gap-register.md) để bổ sung sau.

---

## Cách dùng

**Vào việc ngay:** mở [datalogic-plan.md](datalogic-plan.md) → **§📍 Tiến độ hiện tại** (ngay đầu file). Đó là **nguồn tracking DUY NHẤT** — cho biết đang ở buổi nào, lệnh `/mock` làm tiếp, sổ yếu hiện tại. Không suy đoán tiến độ từ nơi khác.

Kế hoạch tổ chức theo **tuần × buổi**, mỗi buổi hai cột: **Đọc/Hiểu** (chuẩn bị trước — doc cụ thể + pool câu hỏi bank) và **Test 🎤** (phiên mock). Ba nguyên tắc xuyên suốt nằm ở đầu plan: ① ưu tiên *tư duy* hơn cú pháp · ② **xen kẽ + lặp lại ngắt quãng** (khởi động trộn ~5–10′ mỗi buổi + ôn dồn tích cuối tuần) · ③ đọc thì liệt kê đầy đủ, mock thì để ngẫu nhiên.

**Quy trình mỗi buổi ôn:**
1. **🔁 Khởi động ~5–10′** — câu yếu ([weak-register](../mock-interview/weak-register.md)) hoặc câu đến hạn trong bảng **🔁 Lịch kiểm tra lại** của tuần hiện tại.
2. Đọc mục của buổi → mở tài liệu được link (ưu tiên **book summary** cho chiều sâu, **topic** cho bản cô đọng) + pool câu bank đã liệt kê.
3. **Tự trả lời trước** (che đáp án) → mở `<details>` đối chiếu.
4. Chạy phiên mock của buổi. Sai/lúng túng → vào weak-register; nếu là **lỗ hổng tài liệu** (repo chưa có) → [gap-register.md](gap-register.md).

Ký hiệu trong checklist: `- [ ]` chưa ôn · `- [x]` đã ôn vững · **🎯** xác suất gặp cao theo JD · **🔁** ôn lặp lại · **🎤** phiên mock · **🕳️** tài liệu còn thiếu (xem gap register).

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
| B1–B4 (đọc/hiểu) | Đi mục của buổi đó + phiên mock tương ứng ghi ở cột Test 🎤. Mở đầu bằng **🔁 khởi động ~5–10′** |
| B5 | **Coding** — viết vào [coding-arena/](../mock-interview/coding-arena/), interviewer review rồi lưu `reviewed/` |
| CN | **Ôn dồn tích** — quét *tất cả* các tuần đã học (không chỉ tuần vừa rồi): `/mock weak-review` + `/mock comprehensive` |
| Xuyên suốt | Luyện **DSA** song song (vài bài LeetCode/tuần theo pattern — [12-dsa](../../12-dsa/)) |
| Khi phát hiện lỗ hổng | Ghi vào [gap-register.md](gap-register.md), không dừng mạch ôn để lấp ngay |
