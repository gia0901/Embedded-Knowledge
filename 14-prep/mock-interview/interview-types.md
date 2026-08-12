# 🎛️ Interview Types — Loại phiên phỏng vấn

> Mỗi loại có **số câu định sẵn** (điều kiện kết thúc phiên → chuyển review) và **cơ cấu level/type**. Interviewer đếm đủ số câu thì chốt; ứng viên có thể gõ **"xong"/"review"** để dừng sớm.
> Cú pháp: `mock <type> track <track>` — vd `mock daily`, `mock comprehensive track cpp-system`.
>
> ⚠️ **Trần độ sâu:** mọi loại dưới đây chạy ở **T2 (mức phỏng vấn thật)**, trừ `deep-dive` = **T3 (nâng cao)**. T3 = tên lệnh/flag/internals/lock-free — **hỏi được nhưng không tính điểm** ở phiên mặc định. Xem [config.md §6 → Trần độ sâu](config.md).

## 🔺 Cách BẬT chế độ chuyên sâu (T3)

Interviewer **không bao giờ tự bật**. Ba cách, chọn cách hợp ngữ cảnh:

| Cách | Lệnh | Được gì |
|---|---|---|
| **1. Loại phiên chuyên sâu** | `/mock deep-dive track cpp-system` | 5 câu 🟠🔴 design/tình huống, ~40′, **toàn bộ phiên ở T3** |
| **2. Cờ `--deep` trên loại bất kỳ** | `/mock comprehensive track cpp-system --deep`<br>`/mock daily --deep` | Giữ nguyên **số câu + cơ cấu** của loại đó, chỉ **nâng trần lên T3** (tính điểm T3, bài coding được phép cỡ lớn) |
| **3. Nói bằng lời** | *"hỏi khó vào"* · *"cho tôi mức senior"* · *"chấm cả phần chuyên sâu"* | Như cách 2 |

**Khác nhau giữa cách 1 và 2:** `deep-dive` đổi **cả nội dung** (chỉ còn câu design/tình huống, bỏ phần khởi động). `--deep` giữ nguyên hình dạng phiên, chỉ **đổi thang chấm và trần câu hỏi** — hợp khi muốn *"vẫn giả lập vòng thật nhưng chấm khắt khe hơn"*.

**Tắt lại:** bỏ cờ đi. Không có trạng thái dính — mỗi phiên độc lập, mặc định luôn về **T2**.

> 💡 Khi nào nên dùng: còn xa ngày phỏng vấn và muốn **nâng trần**. Sát ngày thì ngược lại — chạy T2 để **phủ rộng**, vì phỏng vấn thật hỏi ở T2.

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
Hỏi lại **mọi câu** trong [weak-register.md](weak-register.md) (lọc theo track nếu nêu) tới khi trả lời vững. Trả lời đạt ≥ 3 điểm hai lần → interviewer gỡ khỏi sổ. Chạy định kỳ (buổi CN theo plan).

## `retention` — Kiểm tra độ nhớ · câu đã trả lời tốt · ~8 câu
Hỏi lại các câu **đã từng trả lời TỐT** (≥3) — kiểm tra kiến thức có còn vững theo thời gian (spaced review). "Đúng rồi" không loại câu khỏi vùng hỏi.
- **Nguồn rút: bảng [🔁 Lịch kiểm tra lại](weak-register.md)** (có ngày đến hạn + góc đã dùng + góc mới đề xuất). Ưu tiên câu **quá hạn lâu nhất**.
- Hỏi theo [config §6 luật ①](config.md): **nén phần nền còn 1 checkpoint**, dồn trọng số vào follow-up, **không lặp lại góc cũ**.
- Câu nào tụt **< 3** → **kéo về weak-register** (regression). ≥ 3 → dời lịch +2 tuần.
- Không nhất thiết chạy thành phiên riêng: câu đến hạn có thể rải vào **slot khởi động 🔁 5–10′** đầu mỗi buổi.

## `full-review` — Kiểm tra toàn diện · mọi câu đã hỏi
Trộn **toàn bộ** câu đã từng hỏi (yếu + tốt, mọi track/level) từ [sessions/](sessions/) — quét rộng để tìm chỗ hổng ẩn. Số câu linh hoạt (mặc định ~12); ứng viên gõ "xong" khi muốn dừng. Hợp giai đoạn nước rút.

---

## Bảng tra nhanh

| Type | Số câu | Trần | Nghiêng về | Khi nào dùng |
|---|---|---|---|---|
| `daily` | 6 | T2 | mix nhẹ (code = snippet 5–10′) | mỗi ngày |
| `rapid` | 12 | T2 | 🟢🟡 concept | warm-up / ít thời gian |
| `comprehensive` | 16 | T2 | đủ 🟢→🔴 + **1 bài vừa / 2 bài nhỏ** | **giả lập vòng thật**, cuối tuần |
| `by-level` | 10 | T2 | 1 level | vá đúng mức yếu |
| `coding` | 3 | T2 | 3 bài **cỡ nhỏ** 10–15′ | luyện live-coding |
| `deep-dive` | 5 | 🔺 **T3** | 🟠🔴 design, bài lớn 40′+ | **nâng trần** (opt-in) |
| `weak-review` | biến thiên | T2 | câu đã sai | củng cố |
| `retention` | ~8 | T2 | câu đã trả lời tốt | kiểm tra độ nhớ theo thời gian |
| `full-review` | ~12 | T2 | mọi câu đã hỏi | quét rộng, nước rút |
