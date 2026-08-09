# 🎛️ Interview Types — Loại phiên phỏng vấn

> Mỗi loại có **số câu định sẵn** (điều kiện kết thúc phiên → chuyển review) và **cơ cấu level/type**. Interviewer đếm đủ số câu thì chốt; ứng viên có thể gõ **"xong"/"review"** để dừng sớm.
> Cú pháp: `mock <type> track <track>` — vd `mock daily`, `mock comprehensive track cpp-system`.

---

## `daily` — Ôn hằng ngày (mặc định) · 6 câu · 15–20′
Đa dạng, nhẹ, giữ nhịp mỗi ngày. Cơ cấu:
- 2 câu **rapid** 🟢 (phản xạ nhanh, đáp án 1–2 câu).
- 3 câu **concept** 🟡 (giải thích cơ chế, so sánh).
- 1 câu **revisit** — từ [weak-register.md](weak-register.md) (ưu tiên), hoặc nếu trống thì **câu retention** (câu đã trả lời tốt, đã lâu chưa hỏi lại — đọc [sessions/](sessions/)).
- Track: mặc định `bsp`; hoặc `all` để trộn. Theo §📍 của [datalogic-plan](../study-plans/datalogic-plan.md) nếu plan đang chạy.

## `rapid` — Phản xạ nhanh · 12 câu · ~15′
Bắn liên tục câu 🟢🟡 **concept**, đáp án ngắn gọn, đo độ *trôi chảy* — thứ interviewer test ở màn khởi động. Ít follow-up. Hợp warm-up trước phiên lớn hoặc lúc ít thời gian.

## `comprehensive` — Giả lập vòng technical thật · 16 câu · ~60′
Mô phỏng một vòng phỏng vấn kỹ thuật đầy đủ trên **1 track chính**. Cơ cấu tăng dần độ khó:
- ~3 câu 🟢 (khởi động, nền tảng).
- ~5 câu 🟡 (cơ chế, so sánh — có follow-up).
- ~3 câu 🟠🔴 **design/tình huống** (🏗️ — chấm theo khung, đào sâu nhiều).
- ~2 câu **coding** (viết vào [coding-arena/](coding-arena/)).
- ~1–2 câu **revisit** từ weak-register + **câu retention** (câu tốt cũ, kiểm tra độ nhớ) + câu xuyên-topic (nối nhiều mảng — thước đo senior).
- Kết: review chi tiết + chấm tổng + 3 lỗ hổng ưu tiên.

## `by-level` — Luyện đúng 1 mức khó · 10 câu
Chọn **một** level: `mock by-level 🟠 track bsp` (hoặc gõ `easy/medium/hard/senior`). Rút 10 câu cùng mức trong track. Dùng để: củng cố nền (🟢🟡) hoặc công phá câu senior (🟠🔴).

## `coding` — Bài code tại chỗ · 3 bài · 30–45′
Interviewer ra 3 bài (theo track — DSA, C++ implement, driver skeleton, C low-level…). Ứng viên **viết code vào [coding-arena/](coding-arena/)** (mỗi bài 1 file, đặt tên gợi ý). Review: đúng/chạy, độ phức tạp, edge case, style, bản mẫu. Yêu cầu think-aloud + nêu độ phức tạp.

## `deep-dive` — Đào sâu tình huống · 5 câu · ~40′
5 câu 🟠🔴 **design/tình huống** trong 1 mảng (vd `deep-dive track bsp` → bring-up/OTA/RT). Mỗi câu đào nhiều tầng follow-up. Luyện năng lực senior: đánh đổi, thiết kế, phân tích lỗi.

## `weak-review` — Ôn lại câu yếu · toàn bộ weak-register
Hỏi lại **mọi câu** trong [weak-register.md](weak-register.md) (lọc theo track nếu nêu) tới khi trả lời vững. Trả lời đạt ≥ 3 điểm hai lần → interviewer gỡ khỏi sổ. Chạy định kỳ (buổi CN theo plan).

## `retention` — Kiểm tra độ nhớ · câu đã trả lời tốt · ~8 câu
Hỏi lại các câu **đã từng trả lời TỐT** (≥3) trong [sessions/](sessions/), ưu tiên câu **đã lâu chưa hỏi lại** — kiểm tra kiến thức có còn vững theo thời gian (spaced review). "Đúng rồi" không loại câu khỏi vùng hỏi. Câu nào tụt điểm → quay lại weak-register.

## `full-review` — Kiểm tra toàn diện · mọi câu đã hỏi
Trộn **toàn bộ** câu đã từng hỏi (yếu + tốt, mọi track/level) từ [sessions/](sessions/) — quét rộng để tìm chỗ hổng ẩn. Số câu linh hoạt (mặc định ~12); ứng viên gõ "xong" khi muốn dừng. Hợp giai đoạn nước rút.

---

## Bảng tra nhanh

| Type | Số câu | Nghiêng về | Khi nào dùng |
|---|---|---|---|
| `daily` | 6 | mix nhẹ | mỗi ngày |
| `rapid` | 12 | 🟢🟡 concept | warm-up / ít thời gian |
| `comprehensive` | 16 | đủ 🟢→🔴 + coding | giả lập vòng thật, cuối tuần |
| `by-level` | 10 | 1 level | vá đúng mức yếu |
| `coding` | 3 | code | luyện live-coding |
| `deep-dive` | 5 | 🟠🔴 design | luyện tư duy senior |
| `weak-review` | biến thiên | câu đã sai | củng cố |
| `retention` | ~8 | câu đã trả lời tốt | kiểm tra độ nhớ theo thời gian |
| `full-review` | ~12 | mọi câu đã hỏi | quét rộng, nước rút |
