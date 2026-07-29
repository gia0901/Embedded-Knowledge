# BEH — Behavioral / HR (STAR)

> Domain `BEH`. Từ [technical_round/03_behavior.md](../../technical_round/03_behavior.md). Dùng phương pháp **STAR** (Situation → Task → Action → Result), luôn kết bằng **con số**. Chất liệu cá nhân đã ráp sẵn theo CV. Track dùng: `behavioral` (vòng HR/behavioral mọi vị trí).
> Khi mock: interviewer hỏi câu behavioral, ứng viên kể theo STAR; review chấm **cấu trúc STAR + vai trò cá nhân rõ + có con số + không chê công ty cũ**.

---

#### BEH-001 · 🟡 · design · ⭐ · [→ 03_behavior](../../technical_round/03_behavior.md)
**"Kể về thành tựu bạn tự hào nhất."**
<details><summary>Gợi ý (chất liệu đã ráp)</summary>

- **S:** Đội tốn nhiều thời gian viết unit test và implement thủ công trên Tizen (OS in-house).
- **T:** Nhận trách nhiệm tăng năng suất cho quy trình này.
- **A:** Tuỳ biến internal AI assistant skill (SKILL.md) để tự sinh unit test + code theo coding convention nội bộ — thiết kế prompt, định nghĩa skill, kiểm thử đầu ra.
- **R:** Giảm ~**70%** thời gian phần việc đó; yếu tố giúp đạt **Best Employee of Quarter**.
</details>

#### BEH-002 · 🟡 · design · ⭐ · [→ DBG-016](debugging.md)
**"Kể về một bug khó nhất bạn từng xử lý."**
<details><summary>Gợi ý</summary>

Dùng câu chuyện **cross-layer**: brightness không cập nhật → lần theo user-space (log/GDB) → ranh giới ioctl → kernel (`dmesg`/`printk`) → phát hiện sai offset thanh ghi / `copy_from_user` → fix → verify lại toàn tuyến. **Bài học:** lỗi xuyên tầng phải lần theo dữ liệu qua từng tầng, không đoán. *(Thay bằng case thật nếu có, giữ cấu trúc.)*
</details>

#### BEH-003 · 🟡 · design · ⭐ · [→ 03_behavior](../../technical_round/03_behavior.md)
**"Làm việc dưới áp lực / với team nước ngoài."**
<details><summary>Gợi ý</summary>

- **S:** Công tác R&D HQ (Hàn Quốc) 2 lần/năm bring-up sản phẩm hướng mass-production.
- **T:** Hỗ trợ tích hợp và fix lỗi gấp trước deadline dây chuyền.
- **A:** Phối hợp trực tiếp kỹ sư HQ, debug cross-layer, ưu tiên lỗi chặn dây chuyền, giao tiếp rõ dù khác ngôn ngữ/múi giờ.
- **R:** Bring-up đúng tiến độ, kịp mass-production.
</details>

#### BEH-004 · 🟡 · design · [→ 03_behavior](../../technical_round/03_behavior.md)
**"Một lần bạn cải tiến quy trình / tự động hoá."**
<details><summary>Gợi ý</summary>

- **S:** Porting driver/library sang chip mới làm thủ công, lặp lại, dễ sai.
- **T:** Giảm công sức và lỗi cho workflow porting.
- **A:** Viết tool Python tự động cập nhật build system (CMake/Makefile) + adapt source qua nhiều nền tảng.
- **R:** Giảm **>50%** công sức thủ công, ít lỗi, nhanh hơn.
</details>

#### BEH-005 · 🟡 · design · [→ DBG-017](debugging.md)
**"Điểm yếu của bạn là gì?"**
<details><summary>Gợi ý</summary>

Chọn điểm yếu **thật + đang cải thiện**: *"Công cụ debug như GDB tôi dùng chưa thật sâu — trước chủ yếu debug qua log userspace, `dmesg`, core dump trên server nội bộ. Tôi đang chủ động học GDB bài bản (breakpoint, watch, backtrace, remote debug với gdbserver) để chủ động hơn."* → vừa thật, vừa cho thấy đã có giải pháp thay thế và đang tiến bộ.
</details>

#### BEH-006 · 🟡 · design · [→ 03_behavior](../../technical_round/03_behavior.md)
**"Một lần bạn bất đồng với đồng nghiệp/quản lý."**
<details><summary>Gợi ý</summary>

Khung: nêu bất đồng **về kỹ thuật, không cá nhân** → lắng nghe lý do của họ → đưa dữ liệu/đo đạc → cùng đi đến quyết định → tôn trọng kết quả chung. Kết: học được gì.
</details>

#### BEH-007 · 🟡 · design · [→ 03_behavior](../../technical_round/03_behavior.md)
**"Vì sao chúng tôi nên tuyển bạn?"**
<details><summary>Gợi ý</summary>

Khớp 3 điểm mạnh với JD: (1) nền **system/C++ + HAL/driver** thực chiến trên sản phẩm thật; (2) kinh nghiệm **multi-chipset, kernel migration, Device Tree** đúng chất BSP; (3) tư duy **tự động hoá & cải tiến** (giảm 70%/50%). Nhấn: học nhanh, làm được việc cross-layer.
</details>

#### BEH-008 · 🟠 · design · ⭐ · [→ 03_behavior](../../technical_round/03_behavior.md)
**"Vì sao bạn rời công ty cũ?" (câu nhạy cảm)**
<details><summary>Gợi ý</summary>

**Nguyên tắc vàng:** hướng về phía trước (điều *muốn tới*), **không chê** công ty cũ. Bản mẫu: *"[Công ty cũ] là nơi tôi học nhiều về system programming, HAL/driver và quy trình sản phẩm quy mô lớn. Sau gần 3 năm, tôi muốn **mở rộng và đào sâu** ở [lĩnh vực trong JD], đảm nhận **vai trò phạm vi trách nhiệm lớn hơn** và bài toán **thử thách hơn**. Vị trí này phù hợp hướng đó."* Thu nhập: đừng đặt làm lý do đầu; nếu hỏi thẳng lương kỳ vọng thì trao đổi dựa trên thị trường + năng lực. Nhấn **phát triển nghề nghiệp & thử thách**.
</details>

#### BEH-009 · 🟡 · design · ⭐ · [→ 03_behavior](../../technical_round/03_behavior.md)
**Câu hỏi ngược interviewer (không hỏi là điểm trừ — chuẩn bị 3–5 câu).**
<details><summary>Gợi ý (chọn 3–5)</summary>

- Team dùng SoC/nền tảng nào; công việc nghiêng **kernel/driver (BSP)** hay **userspace (C++)**?
- Quy trình từ code đến sản phẩm — build system, CI, cách test trên phần cứng thật?
- Codebase legacy cần maintain hay xây mới? Tech debt mức nào?
- Cách team debug lỗi khó/cross-layer — có hạ tầng core dump/tracing nội bộ?
- Bài toán kỹ thuật khó nhất team đối mặt 6–12 tháng tới?
- Vai trò này thành công sau 6 tháng/1 năm trông thế nào? Lộ trình sâu kỹ thuật vs lead/architect?
- ⚠️ Tránh (vòng kỹ thuật): lương/thưởng/nghỉ phép chi tiết; "công ty làm gì?"; câu chỉ có/không.
</details>

---

## Phụ lục — chuẩn bị

**Khung STAR trống (điền câu chuyện của bạn):**
```
Tình huống (S): ...
Nhiệm vụ   (T): ...
Hành động  (A): ... (nhấn VAI TRÒ của BẠN)
Kết quả    (R): ... (kèm CON SỐ)
Bài học      : ...
```
Chuẩn bị 4–5 câu chuyện, mỗi câu xoay được nhiều câu hỏi (thành tựu / thất bại / xung đột / lãnh đạo / học nhanh).

**Checklist ngày phỏng vấn:**
- [ ] Giải thích trôi **mọi dòng** trong CV.
- [ ] Thuộc 3 câu chuyện STAR: 1 thành tựu, 1 bug khó, 1 áp lực/teamwork.
- [ ] Luyện nói **con số**.
- [ ] Sẵn câu "vì sao rời công ty cũ" (bản tích cực) + lương kỳ vọng.
- [ ] Chuẩn bị **3–5 câu hỏi ngược**.
- [ ] Ngủ đủ; tới sớm; mang CV in.

---
⬅️ [Bank index](README.md)
