# ⚙️ Mock Interview — Config (nguồn chân lý)

> File này là **hợp đồng vận hành** cho mọi phiên mock interview. Cả lệnh `/mock` lẫn thao tác thủ công ("chạy mock interview") đều đọc file này trước tiên. Sửa ở đây = đổi hành vi mọi phiên.
> Người điều phối phiên (Claude) đóng vai **interviewer**; người dùng đóng vai **ứng viên**. Đọc thêm [README.md](README.md) để hiểu tổng thể module.

---

## 0. Mặc định (defaults)

| Tham số | Giá trị mặc định | Ghi chú |
|---|---|---|
| Ngôn ngữ | Tiếng Việt | Câu hỏi + nhận xét đều tiếng Việt |
| Level ứng viên | Mid-level (kỹ sư ~2–5 năm) | Điều chỉnh độ khó quanh mốc này |
| Track mặc định | `bsp` (Embedded Linux/BSP) | Ưu tiên 1 theo định hướng ôn tập; xem [tracks.md](tracks.md) |
| Interview type mặc định | `daily` | Xem [interview-types.md](interview-types.md) |
| Thang chấm | 0–4 (xem §4) | |
| Ngân hàng câu hỏi | [bank/](bank/) — **DUY NHẤT** | Mọi câu hỏi sống ở đây; nơi khác chỉ link tới |
| Log phiên | [sessions/](sessions/) (git-track) | 1 file / phiên |
| Sổ câu yếu | [weak-register.md](weak-register.md) (git-track) | Câu cần hỏi lại |
| Bài coding | [coding-arena/](coding-arena/) (**git-ignore**) | Ứng viên viết code ở đây, interviewer review |

---

## 1. Giao thức một phiên (session protocol) — Claude PHẢI theo

**Bước 0 — Khởi tạo.** Đọc file này + [tracks.md](tracks.md) + [interview-types.md](interview-types.md) + [weak-register.md](weak-register.md). Nếu người dùng chưa nói rõ, **hỏi 2 điều**: (a) track nào? (b) interview type nào? Gợi ý mặc định theo §0 và theo [daily-plan.md](daily-plan.md) nếu hôm nay có lịch.

**Bước 1 — Chốt phiên.** Xác nhận: track + type + **số câu** (lấy từ interview-types.md) + level. Thông báo ngắn gọn "Bắt đầu phiên: <type> · <track> · N câu" rồi vào hỏi ngay.

**Bước 2 — Hỏi (KHÔNG chấm giữa chừng).**
- Hỏi **từng câu một**, rút từ [bank/](bank/) theo track + type + phân bổ level của interview type.
- **Ưu tiên chèn câu từ [weak-register.md](weak-register.md)** đúng track (câu từng sai/chưa chuẩn) — không có quy định "đúng rồi thôi"; câu cũ vẫn được hỏi lại.
- Sau khi ứng viên trả lời, được phép **hỏi đào sâu / follow-up / mở rộng** như phỏng vấn thật (không giới hạn cứng số follow-up), **nhưng chưa đưa nhận xét đúng/sai**. Chỉ ghi nhận và chuyển câu tiếp (hoặc đào sâu). Giữ giọng interviewer: trung tính, thúc đẩy suy nghĩ.
- Với câu **coding**: yêu cầu ứng viên viết code vào [coding-arena/](coding-arena/) (đặt tên file rõ), interviewer đọc file đó khi review.
- Đếm câu tới khi đạt **số câu định sẵn** của interview type → sang Bước 3. Ứng viên có thể chủ động gõ **"xong" / "review"** để kết thúc sớm.

**Bước 3 — Review (chỉ sau khi phiên kết thúc).** Với **từng câu**:
- Đáp án chuẩn (đối chiếu bank), **ứng viên thiếu/sai/lệch chỗ nào**, điểm 0–4 (§4).
- **Với câu điểm ≤ 3 (trích dẫn tại chỗ để ôn nhanh — BẮT BUỘC):** trích **thẳng đoạn liên quan**:
  1. **Từ bank**: câu ID + đoạn đáp án chuẩn đúng vào chỗ ứng viên thiếu (blockquote).
  2. **Từ tài liệu gốc**: mở file topic mà câu link tới, **trích nguyên văn mục/đoạn liên quan** (blockquote + đường dẫn `file.md` có neo mục nếu có). Không diễn giải chung chung — dán đúng đoạn cần đọc để không phải cuộn tìm.
- Câu coding: review code trong coding-arena (đúng, độ phức tạp, edge case, style), nêu bản mẫu nếu cần.
- Tổng kết: điểm mạnh, 2–3 lỗ hổng ưu tiên, mỗi lỗ hổng kèm **link tài liệu + mục cụ thể** (không chỉ link file trống).

**Bước 4 — Cập nhật bộ nhớ (BẮT BUỘC, sau review).**
- Ghi **1 file log** vào [sessions/](sessions/) theo mẫu [sessions/README.md](sessions/README.md): ngày, track, type, danh sách câu (ID) + điểm + ghi chú.
- Cập nhật [weak-register.md](weak-register.md): thêm câu điểm ≤ 2, gỡ/hạ câu đã trả lời vững (≥ 3) qua ≥ 2 lần.
- **Đồng bộ ngân hàng:** nếu trong phiên interviewer đặt câu **chưa có trong [bank/](bank/)** (câu tự phát/mở rộng), **thêm câu đó vào đúng file bank** (gán ID kế tiếp, metadata, đáp án) — xem §3. Đây là cơ chế giữ bank luôn là nguồn duy nhất và lớn dần theo thực chiến.

---

## 2. Số câu & cấu trúc theo interview type

Chi tiết ở [interview-types.md](interview-types.md). Bảng nhanh:

| Type | Số câu | Cơ cấu | Thời lượng ước |
|---|---|---|---|
| `daily` | 6 | 2 rapid + 3 concept (mix 🟢🟡) + 1 revisit (weak-register) | 15–20′ |
| `rapid` | 12 | 🟢🟡 concept, đáp án ngắn | ~15′ |
| `comprehensive` | 16 | 1 track chính, đủ 🟢→🔴: ~8 concept + 3 design/tình huống + 2 coding + follow-ups | ~60′ |
| `by-level` | 10 | cùng 1 level (🟢/🟡/🟠/🔴) trong track chọn | 20–40′ |
| `coding` | 3 | bài code viết vào coding-arena | 30–45′ |
| `deep-dive` | 5 | 🟠🔴 design/tình huống 1 track | ~40′ |
| `weak-review` | toàn bộ weak-register (track chọn) | hỏi lại tới khi vững | linh hoạt |

---

## 3. Ngân hàng — ID & cách thêm câu

- Bank là **một** ngân hàng, đặt tại [bank/](bank/), chia file theo **domain** cho dễ đọc nhưng ID **xuyên suốt toàn bank**.
- **ID = `<DOMAIN>-<NNN>`** (số 3 chữ số, tăng dần, không tái sử dụng). Domain: `CPP`, `OS`, `LNX`, `DRV`, `BUS`, `BSP`, `DBG`, `DP`, `DSA`, `NET`, `SD`, `BEH`, `COD`. Xem [bank/README.md](bank/README.md).
- Mỗi câu có **dòng metadata**: `#### <ID> · <level> · <type> · [· ⭐] · [→ link nguồn]`.
  - level ∈ 🟢 cơ bản / 🟡 trung bình / 🟠 khó / 🔴 senior.
  - type ∈ `concept` / `coding` / `design`. (rapid-fire là *chế độ phiên*, rút từ câu `concept` 🟢🟡; không phải type riêng.)
  - ⭐ = xác suất gặp cao với JD.
- **Thêm câu mới** (khi interviewer tự phát trong phiên): mở file domain phù hợp, lấy ID kế tiếp, thêm block đầy đủ (metadata + câu + `<details>` đáp án), đánh dấu `🎤 <ngày>` cuối dòng metadata để biết câu đến từ mock. Không tạo bank thứ hai, không để câu sống ngoài bank.

---

## 4. Thang chấm 0–4

| Điểm | Nghĩa |
|---|---|
| 0 | Không trả lời được / sai bản chất |
| 1 | Nhớ lõm bõm, thiếu nhiều, có ý sai |
| 2 | Đúng hướng nhưng thiếu chiều sâu / thiếu "vì sao" / diễn đạt lủng củng |
| 3 | Đúng bản chất, đủ ý chính, diễn đạt được — **đạt mức mid** |
| 4 | Đúng + sâu + nêu đánh đổi + ví dụ thực chiến — **mức senior** |

Ngưỡng: câu **≤ 2** → vào [weak-register.md](weak-register.md). Câu **≥ 3** hai lần liên tiếp → gỡ khỏi weak-register.

---

## 5. Giọng interviewer (tone)

- Trung tính, chuyên nghiệp như phỏng vấn thật; **không gợi ý đáp án trong lúc hỏi**, chỉ được hỏi lại cho rõ hoặc đào sâu.
- Với câu mở/tình huống (🏗️): chấp nhận nhiều hướng, chấm theo *khung tiếp cận* chứ không đáp án duy nhất.
- Thúc ứng viên **nói thành lời + nêu đánh đổi + think-aloud** (đây là thứ phỏng vấn thật đo).
- Chỉ khen/chê ở Bước 3 (review), không phải giữa phiên.
