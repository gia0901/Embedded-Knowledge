# 🎛️ Interview Types — Loại phiên phỏng vấn

> Mỗi loại có **số câu định sẵn** (điều kiện kết thúc phiên → chuyển review) và **cơ cấu level/type**. Interviewer đếm đủ số câu thì chốt; ứng viên có thể gõ **"xong"/"review"** để dừng sớm.
> Cú pháp: `mock <type> track <track>` — vd `mock daily`, `mock comprehensive track cpp-system`.
>
> ⚠️ **Trần độ sâu:** mọi loại chạy ở **T2**, trừ `deep-dive` = **T3**. Luật đầy đủ (gồm cách bật T3) sống ở [config.md §6](config.md) — không chép lại ở đây.

## `daily` — Ôn hằng ngày (mặc định) · 6 câu · 15–20′
Đa dạng, nhẹ, giữ nhịp mỗi ngày. Cơ cấu:
- 2 câu **rapid** 🟢 (phản xạ nhanh, đáp án 1–2 câu).
- 3 câu **concept** 🟡 (giải thích cơ chế, so sánh).
- 1 câu **revisit** — từ [weak-register.md](weak-register.md) (ưu tiên), hoặc nếu trống thì **câu retention** (câu đã trả lời tốt, đã lâu chưa hỏi lại — đọc [sessions/](sessions/)).
- Track: mặc định `bsp`; hoặc `all` để trộn. Theo §📍 của [datalogic-plan](../study-plans/datalogic-plan.md) nếu plan đang chạy.

## `rapid` — Phản xạ nhanh · 12 câu · ~15′
Bắn liên tục câu 🟢🟡 **concept**, đáp án ngắn gọn, đo độ *trôi chảy* — thứ interviewer test ở màn khởi động. Ít follow-up. Hợp warm-up trước phiên lớn hoặc lúc ít thời gian.

> ⚠️ **`rapid` là NGOẠI LỆ DUY NHẤT của [config §6](config.md) luật ①②③④⑤** — xem mục *"Ngoại lệ duy nhất của luật ① và ④"*. Ở đây **được phép hỏi thẳng *"X là gì"***, tối đa **1 probe ngắn**, **~1 phút/câu**. Đây là loại phiên duy nhất luyện năng lực *nói gọn một khái niệm* — thứ vòng screen thật hỏi trước tiên. Chạy đúng ngân sách 15′; muốn sâu thì đổi type, **không** trộn.

## `comprehensive` — Giả lập vòng technical thật · 16 câu · ~60′
Mô phỏng một vòng phỏng vấn kỹ thuật đầy đủ trên **1 track chính**. Cơ cấu tăng dần độ khó — **cộng đúng 16**:

| Phần | Số câu | Ghi chú |
|---|---|---|
| 🟢 Khởi động, nền tảng | **3** | hỏi nhanh, vẫn có ≥1 follow-up |
| 🟡 Cơ chế, so sánh | **5** | có follow-up |
| 🟠🔴 Design/tình huống 🏗️ | **3** | chấm theo khung, đào sâu nhiều |
| 💻 **Coding** | **1** | **1 bài cỡ vừa (20–30′)** — hoặc **2 bài cỡ nhỏ**, khi đó rút bớt 1 câu concept |
| 🔴 Revisit từ [weak-register](weak-register.md) | **2** | ưu tiên cao nhất |
| 🔁 Retention (đến hạn) + câu **xuyên-topic** | **2** | xuyên-topic = nối nhiều mảng, thước đo senior |
| **Tổng** | **16** | |

⚠️ **Không nhét 2 bài coding cỡ vừa** — ngân sách 60′ không đủ (xem [config §6 → ba cỡ bài](config.md)). Đây là lỗi đã mắc ngày 2026-08-10.

- Kết: review chi tiết + chấm tổng + 3 lỗ hổng ưu tiên.

## `by-level` — Luyện đúng 1 mức khó · 10 câu
Chọn **một** level: `mock by-level 🟠 track bsp` (hoặc gõ `easy/medium/hard/senior`). Rút 10 câu cùng mức trong track. Dùng để: củng cố nền (🟢🟡) hoặc công phá câu senior (🟠🔴).

## `coding` — Bài code tại chỗ · 3 bài **cỡ nhỏ** · 30–45′
Interviewer ra **3 bài cỡ nhỏ (10–15′/bài)** — một hàm, không phải một hệ thống: reverse list, two-sum, `memcpy`/`strlen`, endianness, driver skeleton. Ứng viên **viết code vào [coding-arena/](coding-arena/)** (mỗi bài 1 file). Review: đúng/chạy, độ phức tạp, edge case, style, bản mẫu.
- **Muốn bài cỡ vừa** (class có state — RAII wrapper, ring buffer mutex, 20–30′): rút còn **2 bài**, nói rõ khi chốt phiên.
- **Lock-free / SPSC / đa luồng (40′+) là T3** → chỉ ở `deep-dive`, không ra ở đây.
- **Tính điểm cả việc HỎI SPEC trước khi code** ("mấy luồng?", "đầy thì đè hay chặn?") — ngang với chất lượng code.

## `deep-dive` — Đào sâu tình huống · 5 câu · ~40′ · 🔺 **loại DUY NHẤT chạy ở T3**
5 câu 🟠🔴 **design/tình huống** trong 1 mảng (vd `deep-dive track bsp` → bring-up/OTA/RT). Mỗi câu đào nhiều tầng follow-up. Luyện năng lực senior: đánh đổi, thiết kế, phân tích lỗi.
- **T3 được tính điểm ở đây**: tên công cụ/flag, internals, kỹ thuật lock-free, tối ưu cache.
- Bài coding (nếu có) được phép **cỡ lớn 40′+**.
- Dùng khi muốn **nâng trần**, không dùng để giả lập vòng phỏng vấn thật — cái đó là `comprehensive`.

## `weak-review` — Ôn lại câu yếu · toàn bộ weak-register
Hỏi lại **mọi câu** trong [weak-register.md](weak-register.md) (lọc theo track nếu nêu) tới khi trả lời vững. Trả lời đạt ≥ 3 điểm hai lần → interviewer gỡ khỏi sổ.

---

> 🗑️ **Đã bỏ 2026-08-18 — `retention` và `full-review`.** Cả hai **chưa từng chạy một lần nào** trong 17 phiên, và cơ chế của chúng đã sống ở chỗ khác:
> - *Retention* = **slot khởi động 🔁 5–10′ đầu mỗi buổi**, rút từ bảng [🔁 Lịch kiểm tra lại](weak-register.md). Chính `interview-types` bản cũ cũng ghi *"không nhất thiết chạy thành phiên riêng"* — tức type này tự khai là thừa.
> - *Full-review* = `comprehensive` (giả lập vòng thật) + `weak-review` (quét câu đã sai). Không có gì nó làm được mà hai type kia không làm.
