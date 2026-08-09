# 🎤 Mock Interview — Phỏng vấn thử tương tác

> Lớp **điều phối phỏng vấn**: biến toàn bộ kiến thức của repo thành **phiên phỏng vấn thử chạy được** — Claude đóng vai interviewer, bạn đóng vai ứng viên. Song song là **ngân hàng câu hỏi duy nhất** để tự ôn. Đây không phải nội dung kiến thức mới; mọi câu đều link về tài liệu nguồn.
> Quan hệ với [study-plans/](../study-plans/): study-plans cho bạn *lịch đọc*; mock-interview cho bạn *bài kiểm tra dạng phỏng vấn*. Dùng xen kẽ.

---

## 🚀 Bắt đầu nhanh (2 cách — cùng đọc [config.md](config.md))

**Cách 1 — Slash command (gọn nhất):** trong một hội thoại mới, gõ
```
/mock
```
Claude sẽ đọc config, hỏi bạn chọn **track** + **loại phiên**, rồi bắt đầu phỏng vấn.

**Cách 2 — Thủ công:** trong hội thoại mới, nói
> "Chạy mock interview" (kèm track/type nếu muốn, vd "mock comprehensive track BSP").

Cả hai đều nạp [config.md](config.md) làm hợp đồng vận hành.

---

## 🧭 Chọn gì cho một phiên?

Một phiên = **1 track** (hỏi về mảng nào) × **1 interview type** (hình thức phỏng vấn).

### Track — hỏi về mảng nào (chi tiết: [tracks.md](tracks.md))
- **Theo công việc:** `bsp` (Embedded Linux/BSP) · `cpp-system` (C++ System SW).
- **Theo phần:** `modern-cpp` · `os` · `linux-sysprog` · `embedded` (bare-metal/MCU/RTOS/firmware) · `drivers-dt` · `debugging` · `dsa` · `design-patterns` · `networking` · `system-design` · `behavioral`.
- **Theo sách đã summary:** `emc` · `cpp-concurrency` · `melp` · `ostep` · `lkd` · `cpp-mindset`.

### Interview type — hình thức (chi tiết: [interview-types.md](interview-types.md))
- `daily` — ôn hằng ngày, 6 câu đa dạng (mặc định).
- `rapid` — 12 câu phản xạ nhanh.
- `comprehensive` — giả lập vòng technical thật, 16 câu đủ độ khó + coding + tình huống.
- `by-level` — luyện đúng 1 mức khó.
- `coding` — làm bài code (viết vào [coding-arena/](coding-arena/)).
- `deep-dive` — 5 câu khó/tình huống 1 mảng.
- `weak-review` — hỏi lại các câu bạn còn yếu.
- `retention` — hỏi lại các câu **đã trả lời tốt** để kiểm tra độ nhớ theo thời gian.
- `full-review` — kiểm tra **toàn diện**: trộn mọi câu đã từng hỏi (yếu + tốt).

---

## 🔁 Cách một phiên diễn ra

```mermaid
flowchart TD
    A["/mock hoặc 'chạy mock interview'"] --> B["Claude đọc config + hỏi track/type"]
    B --> C["Hỏi từng câu · follow-up đào sâu<br/><i>KHÔNG chấm giữa chừng</i>"]
    C --> D{"Đủ số câu<br/>hoặc bạn gõ 'xong'?"}
    D -->|chưa| C
    D -->|rồi| E["REVIEW toàn phiên:<br/>đúng/sai từng câu + điểm + lỗ hổng"]
    E --> F["Cập nhật: sessions/ + weak-register.md<br/>+ thêm câu mới vào bank/"]
    F --> G["Gợi ý ôn tiếp + phiên kế"]
```

**Nguyên tắc quan trọng** (từ yêu cầu của bạn):
- **Review chỉ ở cuối phiên** — trong lúc hỏi, interviewer có thể mở rộng/đào sâu và yêu cầu bạn trả lời thêm như thật; chỉ khi *hoàn thành* mới nhận xét.
- **Câu cũ vẫn được hỏi lại** — không có luật "đúng rồi thôi"; câu từng sai được ưu tiên hỏi lại (weak-register).
- **Bank là nguồn duy nhất** — câu interviewer tự phát mà chưa có trong bank sẽ được **thêm vào bank** sau phiên.

---

## 🗂️ Cấu trúc module

| Đường dẫn | Vai trò |
|---|---|
| [config.md](config.md) | **Hợp đồng vận hành** — defaults, giao thức phiên, thang chấm. Nguồn chân lý. |
| [tracks.md](tracks.md) | Định nghĩa track (job / phần / sách) → ánh xạ tới domain trong bank |
| [interview-types.md](interview-types.md) | Định nghĩa loại phiên → số câu + cơ cấu level |
| [bank/](bank/) | **Ngân hàng câu hỏi DUY NHẤT** (ID xuyên suốt). Xem [bank/README.md](bank/README.md) |
| [sessions/](sessions/) | Log từng phiên (git-track) — [mẫu](sessions/README.md) |
| [weak-register.md](weak-register.md) | Sổ câu còn yếu — ưu tiên hỏi lại |
| [coding-arena/](coding-arena/) | Nơi viết code khi làm bài coding (**git-ignore**) |

---

## 🔗 Nguồn câu hỏi

Toàn bộ câu hỏi sống ở **[bank/](bank/)**, không có nơi thứ hai. Các bộ cũ (`11-interview-questions/`, `14-prep/technical_round/02,03,04`) đã gộp hết về đây và **đã xoá** (2026-08-09) sau khi chỉ còn là con trỏ rỗng.

Không tự học từ nhiều nơi — mọi câu ở một chỗ, mọi nơi khác chỉ link tới.
