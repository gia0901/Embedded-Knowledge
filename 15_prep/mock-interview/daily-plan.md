# 📅 Daily Plan — Mock interview 4 tuần sprint (~5 buổi/tuần)

> ⚠️ **ĐANG CHẠY PLAN JD-CỤ THỂ:** [datalogic-plan.md](../study-plans/datalogic-plan.md) (xem §📍 Tiến độ ở đó để resume). File này là **lịch mock tổng quát** (BSP-first) — dùng làm *dự phòng / tham khảo nhịp*, **không** phải plan đang thực thi. Nếu đang ôn cho Datalogic, theo plan kia.

> Lịch **mock interview** cho đợt phỏng vấn 1–2 tháng tới. Cường độ: **sprint 4 tuần, ~5 buổi/tuần**. Ưu tiên **BSP (track ưu tiên 1)**, xen C++ system, nền chung (OS/DSA/debug), và behavioral. Bổ trợ cho lịch đọc [../study-plans/](../study-plans/) — plan kia cho *đọc/hiểu*, plan này cho *kiểm tra dạng phỏng vấn*.
> Mỗi dòng = một phiên: gõ `/mock` rồi chọn (hoặc nói thẳng `mock <type> track <track>`). Cuối mỗi tuần có buổi **`weak-review`** để đóng lỗ hổng tích lũy.

## Nguyên tắc
- **Warm-up bằng `rapid`** đầu buổi khó; **`comprehensive` cuối tuần** để giả lập vòng thật.
- **Câu yếu luôn được hỏi lại** — không bỏ câu đã đúng; [weak-register](weak-register.md) điều hướng.
- Nếu lịch phỏng vấn tới gần, dồn vào track sắp phỏng vấn (thường `bsp`).
- Điều chỉnh tự do theo quỹ thời gian; giữ **nhịp đều** quan trọng hơn hoàn thành đủ.

---

## Tuần 1 — Nền BSP + C++ (phủ rộng, xây phản xạ)

| Buổi | Phiên | Mục tiêu |
|------|-------|----------|
| T2 | `daily` track `bsp` | khởi động, đo mặt bằng BSP |
| T3 | `by-level 🟢🟡 track bsp` (10 câu) | chắc nền boot/DT/driver |
| T4 | `rapid` track `cpp-system` | phản xạ Modern C++ (smart ptr, move, RAII) |
| T5 | `coding` track `dsa` (3 bài) | reverse list, two-sum, ring buffer → viết vào [coding-arena/](coding-arena/) |
| T6 | `comprehensive` track `bsp` (16 câu) | giả lập vòng BSP đầu tiên |
| CN | `weak-review` (mọi track) | đóng lỗ hổng tuần 1 |

## Tuần 2 — Đào sâu BSP + OS/concurrency

| Buổi | Phiên | Mục tiêu |
|------|-------|----------|
| T2 | `daily` track `all` | ôn rải, giữ nhịp |
| T3 | `deep-dive track bsp` (5 câu 🏗️) | bring-up, OTA no-brick, DMA, RT |
| T4 | `by-level 🟠🔴 track cpp-system` | atomic/memory-order, ABI, noexcept |
| T5 | `comprehensive track cpp-system` (16 câu) | giả lập vòng C++ system |
| T6 | `coding track bsp` (3 bài) | memcpy/strlen, ring buffer lock-free, endianness |
| CN | `weak-review track bsp` | củng cố BSP |

## Tuần 3 — Tình huống & cross-topic (mức senior)

| Buổi | Phiên | Mục tiêu |
|------|-------|----------|
| T2 | `rapid track os` + `linux-sysprog` | phản xạ OS/syscall/IPC |
| T3 | `deep-dive track bsp` (bộ 🏗️ H1–H4: bring-up/crash/latency/OOM) | tư duy debug hệ thống |
| T4 | `deep-dive track system-design` (5 câu) | HAL, ABI, sensor pipeline, RT↔UI IPC |
| T5 | `comprehensive track all` (16 câu) | vòng thật đa dạng, chèn câu xuyên-topic |
| T6 | `by-level 🔴 track bsp` | công phá câu senior còn yếu |
| CN | `weak-review` (mọi track) | đóng lỗ hổng tuần 3 |

## Tuần 4 — Chốt & mô phỏng ngày phỏng vấn

| Buổi | Phiên | Mục tiêu |
|------|-------|----------|
| T2 | `behavioral` (track `behavioral`) | STAR, "vì sao rời công ty", câu hỏi ngược |
| T3 | `comprehensive track bsp` (16 câu) | full dress rehearsal BSP |
| T4 | `weak-review` (mọi track) | quét sạch câu yếu còn lại |
| T5 | `comprehensive track cpp-system` | dress rehearsal C++ system |
| T6 | `rapid track bsp` + `behavioral` nhẹ | giữ phong độ, không nhồi |
| CN | Nghỉ / đọc lại [sessions/](sessions/) tổng kết | vào phỏng vấn với đầu óc tỉnh |

---

## Sau 4 tuần
- Nếu còn thời gian: lặp lại tuần 3–4 với trọng tâm track sắp phỏng vấn, xen `weak-review` dày hơn.
- Nếu chuyển hướng dài hơi (Middle+ → Senior): chuyển sang tầng nền của [../study-plans/](../study-plans/) + `deep-dive` định kỳ.
- Backlog chủ đề chưa có câu (DMA sâu hơn, Yocto thực chiến): xem [../study-plans/gap-register.md](../study-plans/gap-register.md); câu mới phát sinh trong mock tự động vào [bank/](bank/).
