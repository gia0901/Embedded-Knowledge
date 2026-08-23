# BEH — Behavioral / HR (STAR)

> Domain `BEH`. Dùng phương pháp **STAR** (Situation → Task → Action → Result), luôn kết bằng **con số**. Chất liệu cá nhân đã ráp sẵn theo CV. Track dùng: `behavioral` (vòng HR/behavioral mọi vị trí).
> Khi mock: interviewer hỏi câu behavioral, ứng viên kể theo STAR; review chấm **cấu trúc STAR + vai trò cá nhân rõ + có con số + không chê công ty cũ**.

---

#### BEH-001 · 🟡 · design · ⭐
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

#### BEH-003 · 🟡 · design · ⭐
**"Làm việc dưới áp lực / với team nước ngoài."**
<details><summary>Gợi ý</summary>

- **S:** Công tác R&D HQ (Hàn Quốc) 2 lần/năm bring-up sản phẩm hướng mass-production.
- **T:** Hỗ trợ tích hợp và fix lỗi gấp trước deadline dây chuyền.
- **A:** Phối hợp trực tiếp kỹ sư HQ, debug cross-layer, ưu tiên lỗi chặn dây chuyền, giao tiếp rõ dù khác ngôn ngữ/múi giờ.
- **R:** Bring-up đúng tiến độ, kịp mass-production.
</details>

#### BEH-004 · 🟡 · design
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

#### BEH-006 · 🟡 · design
**"Một lần bạn bất đồng với đồng nghiệp/quản lý."**
<details><summary>Gợi ý</summary>

Khung: nêu bất đồng **về kỹ thuật, không cá nhân** → lắng nghe lý do của họ → đưa dữ liệu/đo đạc → cùng đi đến quyết định → tôn trọng kết quả chung. Kết: học được gì.
</details>

#### BEH-007 · 🟡 · design
**"Vì sao chúng tôi nên tuyển bạn?"**
<details><summary>Gợi ý</summary>

Khớp 3 điểm mạnh với JD: (1) nền **system/C++ + HAL/driver** thực chiến trên sản phẩm thật; (2) kinh nghiệm **multi-chipset, kernel migration, Device Tree** đúng chất BSP; (3) tư duy **tự động hoá & cải tiến** (giảm 70%/50%). Nhấn: học nhanh, làm được việc cross-layer.
</details>

#### BEH-008 · 🟠 · design · ⭐
**"Vì sao bạn rời công ty cũ?" (câu nhạy cảm)**
<details><summary>Gợi ý</summary>

**Nguyên tắc vàng:** hướng về phía trước (điều *muốn tới*), **không chê** công ty cũ. Bản mẫu: *"[Công ty cũ] là nơi tôi học nhiều về system programming, HAL/driver và quy trình sản phẩm quy mô lớn. Sau gần 3 năm, tôi muốn **mở rộng và đào sâu** ở [lĩnh vực trong JD], đảm nhận **vai trò phạm vi trách nhiệm lớn hơn** và bài toán **thử thách hơn**. Vị trí này phù hợp hướng đó."* Thu nhập: đừng đặt làm lý do đầu; nếu hỏi thẳng lương kỳ vọng thì trao đổi dựa trên thị trường + năng lực. Nhấn **phát triển nghề nghiệp & thử thách**.
</details>

#### BEH-009 · 🟡 · design · ⭐
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

#### BEH-010 · 🟡 · design · ⭐ · 🎤 2026-08-23
**"Kể về một lần bạn làm hỏng việc — một quyết định sai, một lỗi bạn gây ra, hoặc một việc không đạt."**
<details><summary>Gợi ý (khung + chất liệu)</summary>

**Interviewer đang dò gì:** không phải bạn có sai không (ai cũng sai) — mà là **bạn có nhận phần của mình không**, và **có đổi hành vi sau đó không**. Câu này lọc người đổ lỗi.

**Khung 4 bước — thứ tự rất quan trọng:**
1. **Nhận phần của MÌNH trước, một câu, không rào đón.** *"Em nhận feature mà chưa liệt kê hết edge case."*
2. **Bối cảnh sau** (document sai, spec chưa update, PO mặc định mình đã hiểu) — nêu để giải thích, **không** để đỡ đòn.
3. **Đổi hành vi cụ thể** — đây là phần được chấm. *"Thay vì im lặng mò mẫm, em liên hệ thẳng Project Owner làm rõ document."*
4. **Kết quả đo được**, kể cả kết quả ngoài bản thân: *"PO phát hiện spec sai và sửa lại — lỗi đó không lặp ở model sau."*

**Chất liệu S-Box (đã dùng thật):** loạt edge case của adaptive brightness — chuyển chế độ giữa chừng, độ sáng sai lúc boot; sửa dần từng ca thay vì lường trước.

**⚠️ Ba bẫy:**
1. **Kể "hai nhóm nguyên nhân" thay vì MỘT câu chuyện.** Đề hỏi *"một lần"*; trả lời *"có nhiều loại nguyên nhân"* là trả lời **chính sách**, không chấm được.
2. **Nguyên nhân khách quan đứng trước** ⇒ nghe như đỡ đòn trước khi nhận lỗi. Đảo lại.
3. Chọn "thất bại giả" (*"em quá cầu toàn"*) — mất điểm nặng hơn kể một lỗi thật.

**Chốt:** *"Em nhận phần của mình trước, rồi mới nói bối cảnh — và thứ đáng nói nhất là em đã đổi cách làm việc sau đó, chứ không phải em đã sửa xong bug."*
</details>

#### BEH-011 · 🟡 · design · ⭐ · 🎤 2026-08-23 · [→ RES-012](resume.md)
**"Vị trí này có mảng bạn chưa làm qua. Kể một lần bạn học một thứ hoàn toàn mới trong thời gian ngắn — học thế nào, bao lâu thì làm được việc?"**
<details><summary>Gợi ý (khung + chất liệu)</summary>

**Interviewer đang dò gì:** đây **không** phải câu hỏi về khả năng học. Nó là **phiên bản mềm của [RES-012](resume.md)** — họ đang hỏi *"chỗ trống trong resume bạn lấp được không?"*. ⇒ Chất liệu bạn chọn **quan trọng hơn** câu chuyện bạn kể.

**⭐ Luật chọn chất liệu:** chọn thứ **gần JD nhất** mà bạn thật sự có, **không** chọn thứ dễ kể nhất.

| Chất liệu | Dùng khi | Vì sao |
|---|---|---|
| **Yocto trên BeagleBone Black** ⭐ | JD có Yocto/BSP | Trùng đúng trụ JD, có phần cứng thật, có tên đúng (bitbake, OE-core, poky) — **bằng chứng, không phải lời hứa** |
| Windows App / MVVM | JD có desktop/UI | Xa JD embedded nhất — **chỉ dùng khi không còn gì khác** |

**Khung 4 phần:** ① điểm xuất phát (*"nền em là Linux system software"*) → ② **bắc cầu từ thứ đã có** (*"cross-compilation và CMake/Makefile nhiều nền tảng nên em hiểu vấn đề Yocto giải quyết"*) → ③ cách học + **mốc thời gian cụ thể** → ④ **bằng chứng làm được việc**.

**⚠️ Bẫy lớn nhất — không bắc cầu.** Kể một câu chuyện học tập hay nhưng **không nối về JD** thì interviewer vẫn ghi *"chưa có kinh nghiệm mảng này"*. Phần ② mới là phần được chấm.

**Chốt:** *"Em chọn chất liệu gần với thứ anh cần nhất mà em thật sự đã làm, và nói rõ em bắc cầu từ nền nào sang."*
</details>

#### BEH-012 · 🟠 · design · ⭐ · 🎤 2026-08-23
**"Mức lương kỳ vọng của bạn là bao nhiêu?" — và khi bị đẩy: "hơi cao so với ngân sách bọn anh."**
<details><summary>Gợi ý (khung + cách giữ giá)</summary>

**Interviewer đang dò gì:** hai thứ, và thứ hai quan trọng hơn — ① con số có hợp lý không · ② **bạn có giữ được bình tĩnh khi bị đẩy không**. Người rút lui ngay khi bị chê đắt sẽ bị ép tiếp; người phòng thủ gay gắt thì khó làm việc cùng.

**Khung 3 bước:**
1. **Ra số dứt khoát, không vòng vo.** Vòng vo (*"em để công ty quyết"*) là mất quyền neo giá.
2. **Neo vào THỊ TRƯỜNG + NĂNG LỰC, không neo vào nhu cầu bản thân.** ⭐ Đây là chỗ phân biệt.
3. **Mở đường tổng thu nhập**, rồi **hỏi ngược band của họ**.

| Cách neo | Ví dụ | Người trả lương nghe thấy |
|---|---|---|
| ❌ **Nhu cầu bản thân** | *"để em toàn tâm toàn ý làm việc"* | *"Đây là vấn đề của bạn, không phải giá trị của bạn."* |
| ✅ **Thị trường + năng lực cụ thể** | *"Mức này hợp mặt bằng kỹ sư 3 năm có kernel driver, multi-chipset và kernel migration 5.10→6.12."* | *"Có căn cứ, và đang nhắc tôi nhớ vì sao bạn đáng giá."* |

**Bản mẫu khi bị đẩy:**
> *"Em hiểu ạ. Con số đó em đưa dựa trên mặt bằng thị trường cho kỹ sư có nền driver và kernel — nhưng em quan tâm tổng thể gói đãi ngộ và lộ trình chứ không riêng lương cứng. **Anh chia sẻ giúp em band của vị trí này thì em dễ trao đổi hơn.**"*

Câu cuối **chuyển quả bóng về phía họ** mà không hạ giá.

**⚠️ Bẫy:** ① tự hạ số ngay khi bị chê đắt ⇒ số ban đầu mất tín nhiệm · ② lấy lương làm **lý do rời công ty cũ** (xem [BEH-008](#beh-008--design--)) · ③ đưa số mà chưa biết gross/net.

**Chốt:** *"Ra số dứt khoát, neo vào thị trường và năng lực chứ không vào nhu cầu của mình, rồi hỏi ngược band của họ."*
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
