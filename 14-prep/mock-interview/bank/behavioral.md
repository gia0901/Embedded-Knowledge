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

#### BEH-013 · 🟠 · design · ⭐ · 🎤 2026-08-24 · [→ BEH-008](#beh-008--design--)
**"Giả sử bạn nộp đơn nghỉ và công ty cũ giữ lại — tăng lương đúng bằng mức bạn đòi, cho đổi sang mảng bạn muốn. Bạn có ở lại không?"** *(counter-offer)*
<details><summary>Gợi ý (khung + vì sao câu này nguy hiểm)</summary>

**Interviewer đang dò gì — đây là PHÉP THỬ, không phải câu hỏi thật.** Họ vừa nghe bạn nói lý do rời đi là *"muốn thử thách / vai trò lớn hơn"* ([BEH-008](#beh-008--design--)). Câu này kiểm tra lý do đó **có thật không**, hay chỉ là vỏ bọc lịch sự của *"lương thấp"*.

**⚠️ Vì sao nó nguy hiểm:** nhà tuyển dụng sợ nhất là tốn công phỏng vấn, chốt offer, rồi ứng viên **dùng offer đó để mặc cả với công ty cũ**. Do dự ở câu này là tự khai mình có thể làm thế.

**Khung 3 phần — tổng ~20 giây, không dài hơn:**
1. **Trả lời NGAY, không do dự.** *"Em không."* Một mệnh đề. Ngập ngừng ở đây đắt hơn mọi câu chữ sau đó.
2. **Nhắc lại đúng động cơ đã nêu ở BEH-008** — phải **khớp**, vì họ đang đối chiếu hai câu trả lời.
3. ⭐ **Nêu MỘT thứ cụ thể nơi mới có mà chỗ cũ không có** — đây là phần nâng từ 3 lên 4. Nói chung chung (*"thử thách mới"*) thì đúng nhưng không chứng minh được; nói cụ thể thì họ tin.

| Trả lời | Interviewer nghe thấy |
|---|---|
| ❌ *"Cũng còn tuỳ…"* / *"Để em cân nhắc"* | *"Người này sẽ dùng offer của mình đi mặc cả."* |
| ❌ *"Không, vì lương bên anh tốt hơn"* | Mâu thuẫn với lý do đã nêu ở BEH-008 ⇒ mất tín nhiệm cả hai câu |
| ✅ *"Không. Vì cái em muốn không phải con số — ở chỗ cũ em làm một mảng hẹp trong một sản phẩm; ở đây em muốn chạm cả chuỗi từ BSP tới ứng dụng."* | Lý do nhất quán, có nội dung, không mặc cả |

**Bẫy:** ① do dự dù chỉ một nhịp · ② lấy tiền làm lý do (phá luôn [BEH-008](#beh-008--design--)) · ③ chê công ty cũ để cho câu trả lời "mạnh hơn" — không cần, và luôn phản tác dụng.

**Chốt:** *"Trả lời ngay, khớp với lý do đã nêu, và nêu một thứ cụ thể nơi mới có mà chỗ cũ không có."*
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

## 🎤 Từ phiên B1 (2026-09-12) — bám JD mới (process & communication)

#### BEH-014 · 🟡 · concept · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ JD: "Participating in peer-reviews of solution designs and related code"]
**"Một MR sửa driver: code chạy đúng, test pass, nhưng nhét nhánh `if` chip-specific vào driver nền thay vì đi qua bảng function pointer. Bạn đó nói 'đang gấp deadline, để sau refactor'. Em viết gì vào comment, và có approve không?"**
<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn **chặn người khác** hay bạn **làm nợ trở nên nhìn thấy được**. Người review chặn MR vì lý do kiến trúc trong lúc gấp là người khó làm việc cùng; người approve rồi im lặng là người để nợ mục ruỗng. Họ muốn đường thứ ba.

**Thứ tự review (nói được thứ tự là đã ăn nửa điểm):**
> ① **mục đích** — code này định làm gì → ② **hợp đồng** — có phá ABI / tương thích ngược / thêm phụ thuộc mới giữa module không → ③ **tính đúng đắn & lỗi ngầm** (null deref, quên free, đường thoát sớm) → ④ **đọc được** (lồng sâu, logic dư, SOLID) → ⑤ **test có phủ đúng phần vừa đổi không**

⚠️ Tầng ② hay bị xếp nhầm chung với "style". Trong ngữ cảnh **shared library + kernel driver**, nó **nặng hơn** lỗi ngầm — lỗi ngầm hỏng một lần, phá ABI hỏng mọi người dùng.

**Trả lời mẫu — bốn việc trong một comment:**
> *"Approve để kịp release. Nhưng nhánh `if` này bỏ qua bảng function pointer, nên chip tiếp theo sẽ phải thêm nhánh nữa — sau 3 chip là không ai đọc nổi. Mình đã mở [JIRA-1234], hạn <ngày sau release>, gán cho bạn, có ghi rõ đường đi đúng. Và bổ sung giúp mình một test phủ nhánh mới trước khi merge nhé — chỗ đó hiện chưa có test nào chạm tới."*

| Việc | Vì sao |
|---|---|
| **Approve** | Không chặn người ta trong lúc gấp |
| Nói cái giá **bằng số** (*"sau 3 chip"*) | Cái giá trừu tượng không thuyết phục ai |
| Nợ có **chủ + hạn + chỗ có người nhắc** | Comment trong code sẽ mục; Jira có deadline thì không |
| Xin **một** thứ rẻ mà có giá trị ngay (test) | Đổi được thiện chí lấy phòng vệ thật |

**Bẫy:** ① chỉ *"comment cảnh báo trong code"* — 6 tháng sau không ai đọc · ② chặn MR ⇒ mang tiếng cản đường · ③ approve rồi tự đi sửa ⇒ mất cơ hội dạy, và người kia lặp lại lần sau.

**Follow-up hay bị hỏi:** *"Ticket tới hạn, không ai làm, bạn kia đã đổi team. Em làm gì?"*
</details>

#### BEH-015 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ JD: "drive the relation with Validation and Verification team in a fruitful manner"]
**"V&V báo một lỗi. Em chạy lại không tái hiện được, ba vòng qua lại vẫn không. Bên kia bắt đầu cho rằng em đá bóng về phía họ. Em làm gì?"**
<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** JD dùng đúng chữ *"in a fruitful manner"* — họ đã gặp kỹ sư biến quan hệ với V&V thành đấu tố. Câu này đo **ai nhận phần việc về mình**.

**Vòng 1 — hỏi đúng thứ tự (hai nguyên nhân phổ biến nhất trước):**
1. **Hardware revision** — bản pre-development hay bản đã chốt?
2. **Image version** chính xác.
3. Bối cảnh: nhiệt độ, thời lượng chạy, mạng, kịch bản thao tác.
4. Có `dmesg` / log app / coredump tại thời điểm lỗi không.

⭐ **Vòng 2 — điểm xoay của cả câu: thôi XIN dữ liệu, bắt đầu LÀM cho dữ liệu tự sinh ra.**

| Việc | Vì sao nó gỡ được bế tắc |
|---|---|
| ⭐ **Gửi build có đo sẵn** (log ở đúng đường nghi ngờ, tracepoint, vòng đệm trạng thái) | Họ **không phải làm gì khác** — vẫn chạy kịch bản cũ, nhưng mỗi lần chạy giờ sinh dữ liệu cho bạn. Bạn **thôi phụ thuộc vào việc tái hiện được** |
| **Script thu thập một lệnh** (`collect.sh` gom dmesg + log + `/proc` + version + coredump) | Đừng bắt người khác nhớ phải lấy gì — họ sẽ quên, và bạn mất thêm một vòng |
| **Chốt chung định nghĩa "tái hiện"** (*"chạy 20 lần, bị ≥1 lần"*) | Không có định nghĩa chung thì hai bên cãi về **sự tồn tại** của bug thay vì về **nguyên nhân**. Nối [RES-029](resume.md) — quy tắc số ba |

**Câu nói gỡ quan hệ, dùng được nguyên văn:**
> *"Em tin là có bug — em chỉ chưa dựng lại được ở đây. Em gửi anh/chị một build có log ở đúng chỗ nghi ngờ, cùng một script chạy một lệnh là gom hết. Anh/chị cứ chạy đúng kịch bản cũ; lần bị tiếp theo em sẽ có đủ dữ liệu mà không cần phiền thêm."*

Ba việc trong một câu: **công nhận bug là thật** (gỡ ngòi) · **nhận phần việc về mình** · **họ không phải làm gì thêm**.

**Bẫy:** ① *"bên em test không thấy"* — câu này chấm dứt hợp tác · ② chỉ xin thêm log qua từng vòng mail · ③ đòi mượn unit như phương án **duy nhất** (thường không mượn được: khác site, thiết bị không rời phòng lab).

**Follow-up:** *"Build có log gửi đi, chạy 2 tuần không bị lần nào. Em kết luận gì?"* → *"không tái hiện"* **cũng là dữ liệu**: thêm log làm đổi timing ⇒ nghi **race condition** (Heisenbug).
</details>

#### BEH-016 · 🟡 · concept · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ JD; đi kèm [BEH-008](behavioral.md) "vì sao rời công ty cũ"]
**"Vì sao em ứng tuyển vị trí này?"** — *(khác hẳn "vì sao nên tuyển em" — xem bẫy)*
<details><summary>Khung trả lời</summary>

🔴 **Bẫy số một, và là bẫy đã mắc thật (12/09): trả lời nhầm sang câu khác.**

| Câu hỏi | Họ dò gì |
|---|---|
| *"Vì sao nên nhận em?"* → [BEH-007](behavioral.md) | **Năng lực** — em làm được gì |
| *"Vì sao em chọn chỗ này?"* → câu này | **Động lực** — em có hiểu chỗ này làm gì không, lý do em tới có thật không |

Câu này tồn tại vì **người đến vì lương sẽ đi vì lương**. Trả lời bằng năng lực là **né mất câu hỏi**, và người phỏng vấn có kinh nghiệm nhận ra ngay.

**Cấu trúc 3 nhịp — cái *kéo*, không phải cái *đẩy*:**

> **① Cái em đã có — một câu, đặt bối cảnh.**
> *"Ba năm qua em làm xuyên tầng — C++ interface, shared library, kernel driver — nhưng luôn trong **một** dòng sản phẩm và **một** hệ build nội bộ."*
>
> **② Cái JD có mà chỗ em không có — nói THẲNG tên.**
> *"Hai thứ ở đây em chưa có: **Yocto** — em hiểu bài toán nó giải và các thành phần, nhưng chưa vận hành trong sản phẩm thật; và **driver cho I2C/SPI/Ethernet** — em đã làm việc với thiết bị I2C qua subsystem, nhưng phần driver là do library sẵn lo. Em muốn tới chỗ mà hai thứ đó là việc hằng ngày."*
>
> **③ Cái em mang tới đổi lại — trích chữ của JD.**
> *"Đổi lại, JD có 'design complex subsystems' và 'take design choice autonomously' — đó đúng là thứ em đã làm: một interface chung cho nhiều chipset, và một quyết định tối ưu boot time em tự đo hai phương án rồi chọn."*

**Ba luật:**
1. **Nói tên gap ra trước khi họ hỏi.** Tự nêu ⇒ **tự nhận thức**. Bị moi ⇒ **chỗ hổng**.
2. **Cấm "môi trường tốt", "học hỏi nhiều".** Không phân biệt được với 200 ứng viên khác.
3. **Phải trích ≥1 cụm chữ từ JD.** Nó chứng minh đã đọc.

**Kèm — nâng [BEH-008](behavioral.md) *"vì sao rời"* từ chung chung lên cụ thể:**
> *"Samsung cho em nền rất chắc về system software — ba năm qua em đi hết **chiều dọc** của một dòng sản phẩm. Cái em thiếu bây giờ là **chiều ngang**: dòng sản phẩm khác, hệ build khác, phần cứng khác. Ở chỗ cũ điều đó khó xảy ra vì kiến trúc đã ổn định và em sẽ lặp lại chu kỳ port chip hằng năm."*

⚠️ **Không chê công ty cũ.** Câu trên nói về **cái mình muốn**, không nói về **cái họ thiếu** — đó là ranh giới.
</details>

---
⬅️ [Bank index](README.md)
