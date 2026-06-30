# 📕 FILE 3 — BEHAVIORAL & CÂU HỎI NGƯỢC INTERVIEWER
### Dùng phương pháp **STAR** · gắn với chính dự án trong CV của bạn

> **STAR = Situation → Task → Action → Result.** Luôn kết bằng **con số/kết quả**. Bạn có sẵn các con số mạnh: giảm **70%** thời gian (AI skill Tizen), giảm **50%** công sức (chip porting tool), **Best Employee of Quarter Q1-2026**, kernel **5.10→6.12**, công tác R&D HQ Hàn Quốc 2 lần/năm.

---

## A. CÁC CÂU BEHAVIORAL CỐT LÕI (đã ráp sẵn chất liệu của bạn)

### 1. ⭐ "Kể về thành tựu bạn tự hào nhất"
> **S:** Đội tốn nhiều thời gian viết unit test và implement thủ công trên Tizen (OS in-house của Samsung).
> **T:** Tôi nhận trách nhiệm tìm cách tăng năng suất cho quy trình này.
> **A:** Tôi tuỳ biến internal AI assistant skill (SKILL.md) để tự sinh unit test và code theo đúng coding convention nội bộ — thiết kế prompt, định nghĩa skill, kiểm thử đầu ra.
> **R:** Giảm khoảng **70%** thời gian cho phần việc đó; là một yếu tố giúp tôi đạt **Best Employee of Quarter**.

### 2. ⭐ "Kể về một bug khó nhất bạn từng xử lý"
> Dùng câu chuyện **cross-layer**: brightness không cập nhật → lần theo user-space (log/GDB) → ranh giới ioctl → kernel (`dmesg`/`printk`) → phát hiện sai offset thanh ghi / `copy_from_user` → fix → verify lại toàn tuyến.
> **Chốt bài học:** lỗi xuyên tầng phải lần theo dữ liệu qua từng tầng, không đoán. *(Thay bằng case thật của bạn nếu có, giữ cấu trúc.)*

### 3. ⭐ "Làm việc dưới áp lực / với team nước ngoài"
> **S:** Công tác tại R&D HQ (Hàn Quốc) 2 lần/năm để bring-up sản phẩm hướng mass-production.
> **T:** Hỗ trợ tích hợp và fix lỗi gấp trước deadline dây chuyền sản xuất.
> **A:** Phối hợp trực tiếp với kỹ sư HQ, debug cross-layer, ưu tiên lỗi chặn dây chuyền, giao tiếp rõ ràng dù khác biệt ngôn ngữ/múi giờ.
> **R:** Sản phẩm bring-up đúng tiến độ, kịp mass-production.

### 4. "Một lần bạn cải tiến quy trình / tự động hoá"
> **S:** Việc porting driver/library sang chip mới làm thủ công, lặp lại, dễ sai.
> **T:** Giảm công sức và lỗi cho workflow porting.
> **A:** Viết tool Python tự động cập nhật build system (CMake/Makefile) và adapt source qua nhiều nền tảng phần cứng.
> **R:** Giảm **>50%** công sức thủ công, ít lỗi hơn, nhanh hơn.

### 5. "Điểm yếu của bạn là gì?"
> Chọn điểm yếu **thật + đang cải thiện**, tránh sáo rỗng. Gợi ý hợp với bạn:
> *"Công cụ debug như GDB tôi dùng chưa thật sâu — trước đây tôi chủ yếu debug qua log userspace, `dmesg` và phân tích core dump trên server nội bộ. Tôi đang chủ động học GDB bài bản (breakpoint, watch, backtrace, remote debug với gdbserver) để chủ động hơn khi cần."*
> → Vừa thật, vừa cho thấy bạn đã có giải pháp thay thế và đang tiến bộ.

### 6. "Một lần bạn bất đồng với đồng nghiệp/quản lý"
> Khung trả lời: nêu bất đồng **về kỹ thuật, không cá nhân** → bạn lắng nghe lý do của họ → đưa dữ liệu/đo đạc → cùng đi đến quyết định → tôn trọng kết quả chung. Kết: học được gì.

### 7. "Vì sao chúng tôi nên tuyển bạn?"
> Khớp 3 điểm mạnh của bạn với JD: (1) nền tảng **system/C++ + HAL/driver** thực chiến trên sản phẩm thật của Samsung; (2) kinh nghiệm **multi-chipset, kernel migration, Device Tree** đúng chất BSP; (3) tư duy **tự động hoá & cải tiến** (giảm 70%/50%). Nhấn: học nhanh, làm được việc cross-layer.

---

## B. ⭐ "VÌ SAO BẠN RỜI SAMSUNG?" — câu nhạy cảm, trả lời khéo mà vẫn thật

> **Nguyên tắc vàng:** hướng về phía trước (điều bạn *muốn tới*), **không chê** công ty cũ. Lý do thật của bạn (thu nhập, mở rộng, vai trò cao hơn, thử thách hơn) đều **chính đáng** — chỉ cần đóng gói tích cực, chuyên nghiệp.

**Bản trả lời mẫu (ghép các động cơ thật của bạn, lịch sự):**
> "Samsung là nơi tôi học được rất nhiều về system programming, HAL/driver và quy trình sản phẩm quy mô lớn. Sau gần 3 năm, tôi muốn **mở rộng và đào sâu** kiến thức ở [lĩnh vực trong JD], đảm nhận **vai trò với phạm vi trách nhiệm lớn hơn** và những bài toán **thử thách hơn**. Vị trí này phù hợp với hướng phát triển đó, nên tôi rất hứng thú."

**Về thu nhập — nói thế nào?**
> Đừng đặt tiền làm lý do *đầu tiên* trong phỏng vấn kỹ thuật. Nếu được hỏi thẳng về lương kỳ vọng thì trao đổi thẳng thắn, dựa trên thị trường + năng lực. Còn câu "vì sao rời đi" thì nhấn **phát triển nghề nghiệp & thử thách** — đó cũng là sự thật của bạn, và là điều nhà tuyển dụng muốn nghe. (Thu nhập tăng thường là **hệ quả tự nhiên** của vai trò cao hơn, không cần nói toạc.)

**Biến thể ngắn gọn (nếu muốn súc tích):**
> "Tôi tìm cơ hội đảm nhận vai trò rộng hơn và những bài toán kỹ thuật thử thách hơn để phát triển nhanh hơn trong mảng [embedded/BSP/C++]."

---

## C. ⭐ CÂU HỎI NGƯỢC INTERVIEWER (luôn chuẩn bị 3-5 câu — KHÔNG hỏi là điểm trừ)

> Hỏi câu thông minh thể hiện bạn nghiêm túc và đánh giá *họ* ngược lại. Chọn 3-5 câu hợp ngữ cảnh.

### Về công việc & kỹ thuật (nên hỏi)
- Team đang dùng SoC/nền tảng nào, và phần lớn công việc nghiêng về **kernel/driver (BSP)** hay **userspace/application (C++)**?
- Quy trình từ code đến sản phẩm ra sao — build system, CI, cách test trên phần cứng thật?
- Codebase chủ yếu là legacy cần maintain hay đang xây mới? Tech debt hiện ở mức nào?
- Cách team debug các lỗi khó/cross-layer — có hạ tầng core dump/tracing nội bộ không?
- Bài toán kỹ thuật khó nhất team đang đối mặt trong 6-12 tháng tới là gì?

### Về vai trò & phát triển (nên hỏi)
- Vai trò này thành công sau 6 tháng / 1 năm trông như thế nào?
- Lộ trình phát triển — cơ hội đi sâu kỹ thuật vs hướng lead/architect?
- Team học công nghệ mới / chia sẻ kiến thức ra sao?

### Về team & văn hoá
- Cấu trúc team thế nào, tôi sẽ làm việc gần với những vai trò nào?
- Phong cách review code và ra quyết định kỹ thuật của team?

### ⚠️ Tránh hỏi (trong vòng kỹ thuật đầu)
- Lương/thưởng/nghỉ phép chi tiết (để dành vòng HR/offer).
- "Công ty làm gì?" (phải tự tìm hiểu trước).
- Câu chỉ trả lời được bằng có/không, không mở ra trao đổi.

---

## D. CHECKLIST NGÀY PHỎNG VẤN
- [ ] Giải thích trôi **mọi dòng** trong CV — đặc biệt S-Box, multi-chipset, AI skill, chip porting tool.
- [ ] Thuộc 3 câu chuyện STAR: 1 thành tựu, 1 bug khó, 1 áp lực/teamwork.
- [ ] Luyện nói **con số**: 70%, 50%, 5.10→6.12, Best Employee Q1-2026.
- [ ] Chuẩn bị sẵn câu "vì sao rời Samsung" (bản tích cực) + câu lương kỳ vọng.
- [ ] Chuẩn bị **3-5 câu hỏi ngược**.
- [ ] Ôn nhanh File 1 (lý thuyết) + tự test File 2 (Q&A).
- [ ] Ngủ đủ; tới sớm; mang theo bản CV in.

---

## E. KHUNG STAR TRỐNG (điền câu chuyện của riêng bạn)
```
Tình huống (S): ......................................................
Nhiệm vụ   (T): ......................................................
Hành động  (A): ...................................................... (nhấn VAI TRÒ của BẠN)
Kết quả    (R): ...................................................... (kèm CON SỐ)
Bài học      : ......................................................
```
Chuẩn bị sẵn 4-5 câu chuyện theo khung này, mỗi câu xoay được nhiều câu hỏi khác nhau (thành tựu / thất bại / xung đột / lãnh đạo / học nhanh).