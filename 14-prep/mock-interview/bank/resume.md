# 📄 RES — Câu hỏi bám RESUME

> **Domain `RES`.** Mọi câu ở đây bám vào **một dòng cụ thể trong [RESUME_current.tex](../../../RESUME_current.tex)**. Đây là phần **chắc chắn 100% bị hỏi** — khác với câu kỹ thuật thuần chỉ *có thể* bị hỏi. Phỏng vấn thật dành **30–50%** thời gian ở đây.
>
> ⚠️ **Đáp án ở file này KHÔNG phải "lời giải".** Chỉ bạn mới biết bạn đã làm gì. Đáp án là **KHUNG**, gồm 4 phần:
> 1. **Interviewer đang dò gì** — câu hỏi thật nằm sau câu hỏi bề mặt.
> 2. **Câu trả lời tốt gồm** — bộ khung để bạn tự điền chi tiết của mình vào.
> 3. **Nền kỹ thuật phải nắm** — thứ bạn buộc phải giải thích được, kèm link ôn.
> 4. **Bẫy** — chỗ ứng viên hay tự đào hố.
>
> **Cách luyện:** nói thành lời, bấm giờ **90 giây/câu**. Viết ra giấy không tính — phỏng vấn là nói.
> **Chạy phiên:** `/mock rapid track resume` (quét rộng) hoặc `/mock daily track resume` (đào T2).

---

#### RES-001 · 🟡 · concept · ⭐ · 🏗️ · [→ RESUME §Work Experience]
**"Em giới thiệu qua về công việc hiện tại và một project em tâm đắc nhất."**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Em là kỹ sư phần mềm hệ thống, ba năm viết C/C++ trên Linux cho TV và màn hình doanh nghiệp, chạy trên SoC ARM. Việc của em đi xuyên tầng: từ **C++ interface** mà shared library đưa cho ứng dụng, xuống **library**, xuống **driver trong kernel** nói chuyện với phần cứng.*
>
> *Project em tâm đắc nhất là độ sáng tự động trên S-Box — hộp điều khiển màn hình LED quảng cáo; nhiều box ghép lại để chiếu một bức tường lớn. **Bài toán là nếu mỗi box tự chỉnh sáng theo cảm biến riêng thì mỗi ô sáng một kiểu, đổi sáng ở một lúc khác nhau — đứng xa nhìn ra rõ từng ô vuông.** Chính tài liệu lắp đặt của sản phẩm cũng ghi: các box không liên kết với nhau thì độ sáng mỗi box có thể khác nhau.*
>
> *Em làm thuật toán đọc cảm biến ánh sáng và phần đồng bộ: một box làm master, tính ra **một** mức sáng chung rồi phát qua mạng cho các box còn lại; mức sáng đổi dần từng bước nên mắt không thấy giật. Phần em tự quyết định là các lúc **chuyển trạng thái** — bật, tắt tính năng, và khởi động lại khi tính năng đang bật — những chỗ spec ban đầu bỏ trống.*
>
> *Kết quả là cả bức tường đổi sáng cùng lúc, cùng mức, nhìn như **một màn duy nhất**, và chỉ **một binary** chạy được cả chế độ một box lẫn chế độ ghép. Anh muốn em kể kỹ phần nào — thuật toán, phần đồng bộ, hay kiến trúc library bên dưới?"*

**Nếu bị hỏi tiếp**
- *"Không đồng bộ thì người xem thấy gì?"* — Mỗi ô đổi sáng ở một thời điểm và một mức khác nhau, bức tường loang lổ như bàn cờ.
- *"Em biết nó chạy đúng bằng cách nào?"* — Hai cách: **nhìn** — đổi ánh sáng phòng, các ô đổi cùng lúc và cùng mức; và **so log** mức sáng ghi xuống ở từng box — giá trị trùng nhau, thời điểm chỉ lệch trong một nhịp cập nhật của màn hình (1/60 giây), mắt không thấy được. Đó là phần em kiểm được ở tầng của mình; phía ứng dụng có thể còn cách kiểm khác.
- *"Phần nào là em tự quyết?"* — Chuyển trạng thái: bật thì chuyển dần chứ không nhảy; tắt thì giữ hay trả về mức cũ; khởi động lại khi đang bật thì khôi phục thế nào. Em kẻ bảng trạng thái × sự kiện để không sót ô nào ([RES-031](resume.md)).

**Vẽ:** [whiteboard D5](../../whiteboard.md#d5--s-box-nhiều-box--một-bức-tường-một-mức-sáng) — ba box, một master có cảm biến, một đường mạng, một bức tường.

**Móc nền tảng:** thread chạy theo chu kỳ + condition variable ([CPP-069](cpp.md), [CPP-070](cpp.md)) · POSIX message queue ([RES-005](resume.md)) · kiến trúc shared library ([RES-035](resume.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn có **sở hữu** một mảng nào thật không, hay chỉ làm task được giao · ② khả năng kể một hệ thống **từ trên xuống** (bối cảnh → vấn đề → việc bạn làm → kết quả) · ③ chọn project nào — nó lộ ra bạn tự hào về cái gì.

**Câu trả lời tốt gồm (90 giây, không dài hơn):**
1. **Một câu định vị:** *"Em làm system software cho Display Enhancement trên TV Samsung — tầng giữa, từ C++ interface xuống kernel driver nói chuyện với SoC."*
2. **Chọn ĐÚNG project.** Với JD Datalogic (Embedded Linux), chọn **S-Box** (multi-device, POSIX mq, sensor) hoặc **HAL multi-chipset** — **không** chọn Windows Display Manager.
3. **Vấn đề → giải pháp → con số.** Ví dụ S-Box: *"nhiều màn ghép thành một, mỗi máy tự chỉnh sáng theo cảm biến thì chúng lệch nhau, nhìn thành từng ô"* → mq đồng bộ → *"ghép lại nhìn như một màn duy nhất"*.
4. **Chốt bằng phần bạn tự quyết định**, không phải phần được giao.

**Nền kỹ thuật phải nắm:** kiến trúc tầng của chính bạn — vẽ được lên whiteboard trong 30 giây:
```
App / Middleware
      | C++ API (interface thống nhất)
System SW  (shared library)   <-- bạn ở đây
      | ioctl / sysfs
Kernel driver (HAL)
      | register / I2C
SoC / panel hardware
```

---

### 🎙️ BẢN NÓI — 45 giây, PHẢI đọc to bấm giờ

> ⚠️ **Vì sao câu này có bản nói mà các câu khác không:** `RES-001` hỏng **ba lần liên tiếp** cùng một kiểu (18/08 → 23/08 qua `BEH-001` → 29/08), và cả ba lần **nội dung đều có sẵn trong đầu** — hỏng ở khâu **dựng khung và nói ra**. Đó là loại lỗi **chỉ chữa được bằng nói to bấm giờ**, không chữa được bằng đọc thêm. *(Bằng chứng: `BEH` 2.42 → 3.42 sau đúng một ngày viết 5 khung STAR.)*
>
> 🔴 **Đây là bản để BẠN sửa lại theo giọng mình rồi thuộc** — không phải để đọc thuộc lòng nguyên văn. Giữ nguyên **thứ tự bốn nhịp**; đổi chữ thoải mái.

**Bản A — "project tâm đắc nhất" (mặc định):**

> *"Em làm system software cho display enhancement trên TV và màn hình doanh nghiệp — từ C++ interface xuống kernel driver nói chuyện với SoC.*
>
> *Thứ em tâm đắc nhất là S-Box. **Bài toán là nhiều màn ghép lại thành một màn lớn, mà mỗi máy tự chỉnh sáng theo cảm biến riêng của nó thì chúng lệch nhau — nhìn ra từng ô rõ rệt.***
>
> *Em làm phần đồng bộ độ sáng qua POSIX message queue, một máy làm master phân phối giá trị cho các máy còn lại. Chống nhấp nháy bằng cách chỉ tính lại target **4 giây một lần** rồi step dần tới đó.*
>
> ***Kết quả là ghép lại nhìn như một màn duy nhất, và cùng một binary chạy được cả chế độ đơn lẻ lẫn đồng bộ.**"*

**Bốn nhịp — và vì sao từng nhịp có mặt:**

| Nhịp | Nội dung | Chữa lỗi nào |
|---|---|---|
| 1 | **Định vị** — em đứng ở tầng nào, biên trên/biên dưới là gì | (đã làm tốt cả 3 lần, giữ) |
| 2 | 🔴 **VẤN ĐỀ trước, bằng hình ảnh** — *"nhìn ra từng ô"* | **Lỗi lặp #1:** ba lần đều nhảy thẳng vào công nghệ |
| 3 | Giải pháp — **gọi tên cơ chế, không liệt kê công nghệ** | Ba lần đều thành *"vsync 60Hz, message queue…"* |
| 4 | 🔴 **Kết quả có thể HÌNH DUNG hoặc ĐO** | **Lỗi lặp #2:** ba lần đóng câu **không có gì cụ thể** |

⏱️ **Cách luyện (30 phút, một lần là đủ):** bấm giờ, nói to, ghi âm. Nghe lại và chỉ kiểm **hai** thứ — ① câu thứ hai đã là **vấn đề** chưa ② câu cuối có **thứ đo được / hình dung được** chưa. Đạt cả hai hai lần liên tiếp thì gỡ khỏi sổ yếu.

**Bản B — dùng khi họ hỏi *"kể một vấn đề khó em đã giải quyết"* (KHÔNG phải "tâm đắc"):**

> Chuyển sang **tối ưu thời gian nạp driver** ([RES-013](resume.md)…[RES-015](resume.md)) — vì nó có **chuỗi chẩn đoán bằng bằng chứng** và **con số cứng**: `probe` 3–5 giây → `dmesg` xác nhận đoạn chậm chỉ là gán biến → **CPU 100%, tiến trình RT priority 94 từ giây thứ 4** → driver nền nạp tuần tự bằng `call_usermodehelper` ở **priority 120** ⇒ bị preempt. Kết: **3–4 giây xuống dưới 0,5 giây**, nghiệm thu **100 chu kỳ boot**.
>
> 📌 **Phân vai rõ:** S-Box chứng minh **sở hữu một tính năng end-to-end**; insmod chứng minh **chẩn đoán và đo đạc**. JD Datalogic nhấn debugging ⇒ nếu chỉ được kể **một** thứ trong cả buổi, kể cái thứ hai.

---

---

### 🎙️ BẢN C — MỞ MÀN 3 MÓC

> ⚠️ **`RES-001` đã hỏng LẦN THỨ TƯ** (18/08 → 23/08 qua `BEH-001` → 29/08 → **12/09**). Bản A/B ở trên trả lời câu *"project tâm đắc nhất"*. Bản C trả lời câu **mở màn thuần** — *"em giới thiệu qua về bản thân"* — là câu **thật sự** được hỏi ngày 12/09, và là câu bị hỏng.
>
> **Khác biệt cốt lõi:** bản A/B kể **một** thứ cho sâu. Bản C thả **ba móc** rồi **dừng** — mục tiêu không phải kể hay, mà là **chọn trước ba chủ đề interviewer sẽ đào**.

**🇻🇳 Bản Việt — 80 giây:**
> *"Em là Software Engineer tại Samsung, ba năm làm system software cho TV và dòng enterprise display trên SoC ARM.*
> *Công việc của em **xuyên tầng**: từ C++ interface mà shared library phơi ra, xuống HAL, xuống kernel driver nói chuyện với SoC.*
> *Có ba thứ em làm mà em nghĩ đáng nói:*
> *— Một, bọn em có **một interface C++ chung cho nhiều chipset**: library chọn implementation lúc chạy theo board config, còn kernel dispatch qua function-pointer table — app ở trên không phải build lại.*
> *— Hai, em **tối ưu thời gian nạp driver từ 3–4 giây xuống dưới 0.5 giây**, sau khi đo hai phương án và verify qua 100 lần boot.*
> *— Ba, em **port driver qua các thế hệ chip** hằng năm, và migrate kernel lên 6.12 mà vẫn giữ tương thích ngược với nhánh cũ.*
> *Em đang muốn đi sâu hơn về kernel/driver và làm với quy trình sản phẩm bài bản hơn — Yocto, CI, V&V. Anh muốn nghe kỹ phần nào ạ?"*

**🇬🇧 Bản English — 80 giây:** xem [RES-032](resume.md).

**Ba móc ↔ JD:**

| Móc | Cắn vào dòng JD |
|---|---|
| 🅐 interface đa chipset + fn-ptr table | *"design and implement complex software subsystems"* |
| 🅑 3–4s → 0.5s, đo 2 phương án, 100 boot | *"take design choice autonomously"* · *"drive the relation with V&V"* |
| 🅒 port driver + kernel migration | *"Linux kernel space and driver programming"* |

**Bốn lỗi của bản đã nói ngày 12/09 — kiểm lại mỗi lần luyện:**

| ❌ Đã mắc | ✅ Phải là |
|---|---|
| *"hoàn thành **kịp tiến độ** adaptive brightness"* | *"3–4 giây xuống dưới 0.5 giây, verify 100 lần boot"* — "kịp tiến độ" là phát biểu về **lịch**, không mở ra câu hỏi nào |
| **Không nhắc** HAL đa chipset | Là **móc số một** — đúng câu đạt **4 điểm** 5 phút sau đó |
| *"Em có thể trình bày lại nếu anh muốn?"* | *"Anh muốn nghe kỹ phần nào?"* — câu **có/không** đá bóng về chân mình |
| Không có lý do ứng tuyển | Có, nối vào JD — dọn sẵn đường cho [BEH-016](behavioral.md) |

⚠️ **Kỷ luật: thả móc rồi IM.** Mỗi móc đúng **một câu**. Interviewer cắn móc nào thì mở STAR 2 phút cho móc đó.

⏱️ **Cách luyện:** nói to, bấm giờ, tới khi ổn định **≤ 90s** — cả bản Việt lẫn bản Anh. Rồi tự hỏi *"nghe xong, người ta sẽ hỏi mình cái gì?"* — nếu câu trả lời không rơi vào một trong ba móc thì móc chưa hoạt động.

---

**Bẫy:** ① kể theo trình tự thời gian *"đầu tiên em được giao…"* — chán và không lộ năng lực · ② nhảy thẳng vào chi tiết kỹ thuật mà không dựng bối cảnh, interviewer không theo kịp · ③ chọn project Windows cho một JD Embedded Linux · ④ nói *"em làm theo yêu cầu team"* — triệt tiêu mọi cơ hội ghi điểm.
</details>

#### RES-002 · 🟠 · design · ⭐ · 🏗️ · [→ RESUME: "one interface across chipsets… selects its implementation at boot from board configuration… applications above stay unchanged"]
**Resume ghi *"một interface dùng chung cho nhiều chipset, thư viện chọn implementation lúc boot theo board configuration"*. Kể cơ chế đó — và điều gì xảy ra khi board configuration đọc ra sai?**

> 📌 *Resume **đã tự nói ra cơ chế** (chọn lúc boot + bảng con trỏ hàm), nên câu hỏi dịch trọng tâm sang **hệ quả và chế độ hỏng** — đó mới là chỗ interviewer khoan.*

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Bài toán: cùng một chức năng nhưng mỗi dòng chip ghi xuống phần cứng một kiểu. Nếu để khác biệt đó lộ lên trên thì mỗi lần thêm chip là ứng dụng phải sửa.*
>
> *Bọn em giải ở **hai tầng**, cùng một ý tưởng. Ở user-space, ứng dụng chỉ thấy **một C++ interface**. Lúc khởi động, library đọc **cấu hình của board** để biết đây là model nào, rồi dựng đúng bộ cài đặt cho model đó — ứng dụng không biết và không cần biết. Ở kernel, driver chung không biết tên chip; nó gọi xuống driver của chip qua một **bảng con trỏ hàm** mà driver chip tự điền vào khi được nạp — giống bảng hàm ảo của C++ nhưng viết tay bằng C.*
>
> *Thêm một chip mới thì thêm một bộ cài đặt và một driver, **không sửa code đang chạy tốt**, và ứng dụng không phải build lại.*
>
> *Còn chế độ hỏng thì em nói thẳng: cấu hình **thiếu** thì library rơi về mặc định an toàn — thuật toán phổ biến nhất. Nhưng cấu hình **sai giá trị** thì library **nạp nhầm** cài đặt và **vẫn chạy tiếp**, không dừng lại. Lỗi khi đó có hai nguồn: phía phần cứng đưa spec sai, hoặc phía mình đọc cấu hình sai. Triệu chứng là hiển thị sai trên đúng một model — không crash, không log."*

**Nếu bị hỏi tiếp**
- *"Vậy em cải thiện thế nào?"* — Ba việc rẻ: **log rõ** cấu hình và cài đặt đã chọn ngay lúc khởi động, để lỗi lộ ở lần đọc log đầu tiên; **kiểm chéo** khi có thể — thông tin chip mà driver báo lên có khớp với cấu hình không, lệch thì cảnh báo; và đưa việc kiểm cấu hình vào **test nhà máy**. Dữ liệu cấu hình còn **sửa được** bằng dữ liệu giả, nên bring-up và test board mới không phải chờ phần cứng chuẩn.
- *"Sao không `if (chip == …)`?"* — Mỗi lần thêm chip là sửa vào file đang chạy tốt ở mọi chỗ có `if`; quên một chỗ là lỗi chỉ trên đúng model đó.
- *"Thêm hàm ảo vào interface đã phát hành được không?"* — Chỉ thêm ở **cuối**; chèn giữa làm lệch bảng hàm ảo, app cũ gọi nhầm hàm ([DP-027](design-patterns.md)).

**Móc nền tảng:** đa hình runtime / vtable ([CPP-006](cpp.md)) · Abstract Factory ([DP-022](design-patterns.md)) · bảng con trỏ hàm trong kernel ([DP-043](design-patterns.md)) · ABI ([SD-017](system-design.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **câu quan trọng nhất trong resume của bạn** với JD này — nó chính là *"thiết kế C++ shared library"*. Họ dò: bạn hiểu **abstraction** hay chỉ viết `if (chipset == A)`?

**Câu trả lời tốt gồm:**
1. **Nêu vấn đề trước:** nhiều chipset, cùng một chức năng, chi tiết thanh ghi/luồng khác nhau. Nếu để lộ lên trên thì mỗi lần thêm chipset là sửa khắp nơi.
2. **Nói cơ chế bạn thật sự dùng** — trung thực. Nếu là `if/switch` theo model thì **nói thẳng**, rồi nói bạn thấy hạn chế gì và sẽ cải thiện ra sao. Interviewer quý sự trung thực + nhận thức hơn là mô tả đẹp mà hỏi sâu là vỡ.
3. **Gọi tên nguyên tắc:** interface do **tầng trên** định nghĩa, implementation nằm ở dưới — đảo chiều phụ thuộc (**DIP**). Ba cách tạo "khe cắm": **virtual** (chọn lúc chạy) · **template/CRTP** (chọn lúc biên dịch, không tốn vtable) · **link-time** (mỗi sản phẩm link một `.so` khác).
4. **Đánh đổi:** virtual tốn một lần gọi gián tiếp nhưng thêm chipset **không phải sửa code cũ**; `if/switch` nhanh hơn nhưng mỗi lần thêm là sửa vào file đang chạy tốt (rủi ro hồi quy).

**Nền kỹ thuật phải nắm:** [DP-011](design-patterns.md) DIP/HAL · [solid-principles](../../../11-design-patterns/solid-principles.md) (OCP: *mở để mở rộng, đóng để sửa đổi*) · [api-design](../../../07-shared-libraries/api-design.md) · nếu là `.so` giao cho bên khác thì thêm **ABI** ([SD-017…031](system-design.md)).

**⭐ Chế độ hỏng — phần resume KHÔNG nói, và là chỗ ăn điểm** *(đã hỏi 2026-08-29, đạt 4đ)*:
- **Dimming:** không có config khớp ⇒ rơi về **default an toàn** là *Global Dimming*, vốn chiếm hầu hết tỉ trọng sản phẩm. Nêu được *"có default an toàn"* quan trọng hơn nêu tên default.
- **Panel driver:** driver thật xác định **lúc chạy** từ **factory data / board config**, vì một board ghép được với nhiều loại panel. ⭐ Và data đó **thay đổi được** để bring-up/test bằng data giả — chi tiết này cho thấy bạn hiểu nhu cầu vận hành thật, không chỉ đường happy path.
- **Config sai giá trị** (không phải thiếu): hệ thật **nạp nhầm và chạy tiếp**, không có bước từ chối. Nguồn lỗi là spec phần cứng sai hoặc phía mình đọc cấu hình sai. Nói thẳng điều đó rồi nêu cách cải thiện (log cấu hình đã chọn, kiểm chéo với thông tin chip, test nhà máy) — xem trả lời mẫu.

**Bẫy:** ① nói *"em dùng OOP"* rồi hết — phải nêu **ai chọn implementation, chọn lúc nào** · ② thổi phồng thành kiến trúc plugin động nếu thực tế chỉ là `#ifdef` — hỏi hai tầng là lộ · ③ quên rằng đây là **shared library**: thêm virtual function vào class đang phát hành là **phá ABI** ⇒ nêu được ý này là ghi điểm senior.
</details>

#### RES-003 · 🟠 · concept · ⭐ · [→ RESUME: "the kernel driver dispatches through a function-pointer table"]
**Resume ghi kernel driver *"điều phối qua một bảng con trỏ hàm"*. Kernel viết bằng C thuần — cơ chế đó hoạt động thế nào, và nó tương đương cái gì trong C++?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** câu này **rất dễ bị hỏi** vì nó nghe mâu thuẫn. Họ dò xem bạn **thật sự đọc code kernel** hay chỉ mượn chữ.

**Câu trả lời tốt gồm — vẽ được cái này ra là đạt:**
```c
struct panel_ops {                      // "lớp trừu tượng"
    int  (*init)(struct panel *p);
    int  (*set_brightness)(struct panel *p, int level);
    void (*release)(struct panel *p);
};

static const struct panel_ops chipA_ops = { .init = a_init, .set_brightness = a_set, ... };
static const struct panel_ops chipB_ops = { .init = b_init, .set_brightness = b_set, ... };

struct panel {                          // "đối tượng"
    const struct panel_ops *ops;        // <-- vtable thủ công
    void *priv;                         // <-- dữ liệu riêng của từng chipset
};

p->ops->set_brightness(p, 50);          // <-- gọi ảo
```
- **Kế thừa** = struct lồng struct (struct con chứa struct cha ở đầu) + `container_of()` để đi ngược từ con về cha.
- **Đa hình** = bảng con trỏ hàm — đúng thứ C++ sinh tự động dưới tên **vtable**.
- **Đóng gói** = `void *priv`, tầng trên không cần biết bên trong.

**Nêu thêm để ghi điểm:** đây **không phải sáng tạo riêng**, chính kernel làm vậy khắp nơi — `file_operations`, `i2c_driver`, `platform_driver` đều là ops struct. Nói được điều này chứng minh bạn đọc kernel thật.

**Nền kỹ thuật phải nắm:** [oop.md](../../../01-cpp-fundamentals/oop.md) (vtable) · [driver-basics](../../../05-drivers-device-tree/driver-basics.md) (`file_operations`) · `container_of`.

**Bẫy:** ① nói *"em viết C++ trong kernel"* — kernel Linux **không dùng C++**, nói vậy là sai nghiêm trọng · ② không giải thích được **`void *priv` để làm gì** · ③ bị hỏi *"khác gì vtable của C++"* mà không trả lời được (khác: bạn **tự quản lý**, không có RTTI, không exception, và mỗi instance chỉ tốn một con trỏ).
</details>

#### RES-004 · 🟠 · concept · ⭐ · 🏗️ · [→ RESUME: "kernel migration (Linux 5.10 → 6.12)"]
**Em có làm kernel migration 5.10 lên 6.12. Cái gì vỡ? Em phát hiện và xử lý thế nào?**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Em làm migration từ 5.10 lên 6.12 cho mảng driver hiển thị của em, với yêu cầu **vẫn chạy trên nhánh 5.10** cho sản phẩm đang bán.*
>
> *Cái vỡ em chia bốn nhóm. **Đổi chữ ký hàm**: một số hàm đăng ký và tạo thiết bị đổi tham số — compiler báo hết, sửa dần. **Hàm bị xoá**: có API cũ bị gỡ hẳn, phải tìm cách thay. **Cảnh báo bị siết**: nhiều thứ trước chỉ là warning giờ thành lỗi build. Và nhóm khó nhất là **đổi hành vi ngầm** — thứ tự khởi tạo, thời điểm, mặc định của subsystem — compiler không báo gì, chỉ lộ lúc chạy, nên phải so log khởi động hai kernel và đọc lịch sử thay đổi của subsystem đó.*
>
> *Tương thích ngược em làm bằng **một source duy nhất** build được cho cả hai kernel: kiểm phiên bản bằng `LINUX_VERSION_CODE` so với `KERNEL_VERSION(...)`, tách nhánh cài đặt ở đúng chỗ khác nhau, phần chung giữ nguyên. Cái giá là nhánh nào cũng phải test riêng, và `#if` càng nhiều code càng khó đọc — nên chỉ hợp khi khác biệt ít và có hạn."*

**Nếu bị hỏi tiếp**
- *"Vì sao kernel không giữ ABI cho driver như glibc giữ cho app?"* — Cố ý: để kernel tự do sửa nội bộ, và khuyến khích driver vào mainline; module ngoài cây phải theo từng phiên bản.
- *"Khi nào nên tách hai nhánh thay vì `#if`?"* — Khi khác biệt lan ra kiến trúc chứ không còn là vài API.

**Móc nền tảng:** kernel module · device tree · kernel không có ABI ổn định cho module.

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** 🎯 **Món ngon nhất trong resume của bạn cho JD Datalogic** (họ làm kernel/BSP). Câu này lộ ra bạn **có thật sự chạm kernel** hay chỉ sửa vài dòng. Kể được một ca cụ thể là ăn điểm rất mạnh.

**Câu trả lời tốt gồm:**
1. **Quy mô:** bao nhiêu driver, bao lâu, một mình hay team.
2. **Phân loại cái vỡ** — kể theo nhóm, không kể lan man:

   | Nhóm | Ví dụ điển hình |
   |---|---|
   | **API đổi chữ ký** | callback `remove` đổi kiểu trả về; `probe` bỏ tham số `id` |
   | **API bị xoá** | hàm cũ bị thay bằng API mới hẳn |
   | **Đổi cách khai báo** | macro đăng ký driver, thuộc tính device tree |
   | **Đổi hành vi ngầm** | thứ tự probe, timing, mặc định của subsystem — **loại khó nhất** |

3. **Cách phát hiện** — nhóm 1–3 thì **compiler báo**, dễ. Nhóm 4 **không báo gì**, chỉ lộ lúc chạy ⇒ phải **so sánh hành vi hai kernel**, đọc log boot hai bên, và đọc changelog/commit của subsystem liên quan.
4. **Con số cuối:** đưa sản phẩm boot được / bao nhiêu driver chạy lại.

**Nếu bạn chỉ tham gia một phần:** nói rõ phần của mình. *"Em phụ trách N driver trong mảng display"* — trung thực và vẫn ghi điểm.

**Nền kỹ thuật phải nắm:** kernel **không có ABI ổn định cho module ngoài cây** (đây là lý do việc này tồn tại — nói được là điểm cộng lớn) · [driver-basics](../../../05-drivers-device-tree/driver-basics.md) · [device-tree](../../../05-drivers-device-tree/device-tree.md) · [kernel-debugging](../../../09-debugging/kernel-debugging.md).


**⭐ Tương thích ngược với nhánh 5.10** ⇒ chắc chắn bị hỏi *"tương thích ngược nghĩa là gì — một source hay hai nhánh?"*

**Bản trả lời đã đạt 4đ (29/08):** ***một source duy nhất*** build hợp lệ cho **cả 5.10 lẫn 6.12** — vừa không ảnh hưởng sản phẩm đang bán, vừa sẵn sàng cho đời sau. Cách làm: kiểm **kernel version** rồi tách nhánh cài đặt bằng macro cho từng phiên bản, phần chung giữ đúng convention (khai báo trước, logic sau, luôn có function prototype).

**Nêu thêm được thì lên hẳn một bậc:** ① macro chuẩn là `LINUX_VERSION_CODE` so với `KERNEL_VERSION(a,b,c)` ② **cái giá** của một-source: `#ifdef` càng nhiều thì **nhánh nào cũng phải test riêng**, và code đọc khó dần ⇒ chỉ hợp khi số nhánh **ít và có hạn** ③ khi nào thì **nên tách hai nhánh** thay vì `#ifdef`: khi khác biệt lan ra kiến trúc chứ không còn là vài API.

**Bẫy:** ① trả lời chung chung *"em sửa lỗi build"* — không lộ được gì · ② **không** chuẩn bị được **một ca cụ thể có tên và có cách chẩn đoán** · ③ bị hỏi *"vì sao kernel không giữ ABI như glibc?"* mà tắc — chuẩn bị sẵn: **cố ý**, để kernel tự do refactor nội bộ, và để ép driver vào mainline.
</details>

#### RES-005 · 🟠 · design · ⭐ · 🏗️ · [→ RESUME: "POSIX message queue… synchronize brightness across multiple S-Box devices"]
**Em dùng POSIX message queue để đồng bộ độ sáng giữa nhiều thiết bị. Vì sao chọn message queue mà không phải socket hay shared memory?**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Em làm rõ phạm vi trước: message queue của POSIX chỉ chạy **trong một máy**. Ở S-Box có hai chặng. Chặng một **trong một box**: library của em đưa mức sáng chuẩn vào message queue, process ứng dụng trên cùng box nhận về. Chặng hai **giữa các box**: các box nối với nhau qua mạng LAN, mỗi box một IP; đường mạng đó mang cả nội dung hiển thị lẫn lệnh điều khiển, và mức sáng tự động là một trong các lệnh điều khiển. Phần mạng do tầng ứng dụng đảm nhận.*
>
> *Cho chặng library → ứng dụng, em chọn message queue vì bản tin **nhỏ và thưa** — một mức sáng, chỉ khi thay đổi. Message queue giữ **ranh giới bản tin** — không phải tự cắt gói như socket luồng byte; có **độ ưu tiên** cho từng bản tin; và có **hàng đợi sẵn** — bên nhận bận thì bản tin vẫn nằm đó chờ.*
>
> *Shared memory nhanh hơn nhưng **không có cơ chế báo hiệu** — phải tự thêm semaphore hay eventfd, tự đồng bộ, và một bên chết khi giữ khoá là bên kia kẹt; quá đắt cho vài byte. Socket trong máy thì dùng được, nhưng phải tự định khung bản tin và thừa cho giao tiếp một chiều trong một box."*

**Nếu bị hỏi tiếp**
- *"Hàng đợi đầy thì sao?"* — Mặc định chỉ khoảng 10 bản tin; đầy thì `mq_send` chặn, hoặc trả `EAGAIN` nếu mở non-blocking. Với mức sáng — một trạng thái — em ưu tiên bản tin mới nhất, bỏ bản cũ.
- *"Sao không dùng luôn socket cho cả hai chặng?"* — Mỗi chặng chọn theo bản chất của nó: trong máy thì message queue đơn giản và có sẵn ranh giới bản tin; giữa các máy thì bắt buộc là mạng.
- *"Máy con biết mình là con thế nào; master chết thì sao?"* — Xem [RES-027](resume.md).

**Vẽ:** [whiteboard D5](../../whiteboard.md#d5--s-box-nhiều-box--một-bức-tường-một-mức-sáng) — ba box, một master có cảm biến, một đường mạng, một bức tường.

**Móc nền tảng:** chọn IPC ([LNX-017](linux-sysprog.md)) · dừng thread đang chặn trong `mq_receive` ([LNX-044](linux-sysprog.md)) · shared memory + khoá ([LNX-045](linux-sysprog.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn **chọn có lý do** hay dùng cái mình quen. Đây cũng là chỗ nối thẳng sang IPC — mảng bạn đã ôn kỹ.

⚠️ **Làm rõ trước khi trả lời** (và việc *hỏi lại* này chính là điểm cộng): nhiều S-Box là **nhiều process trên cùng một máy** hay **nhiều máy nối mạng**? POSIX mq **chỉ chạy trong một máy** — nếu là nhiều máy vật lý thì phải có socket ở đâu đó. Trả lời đúng bối cảnh thật của bạn.

**Câu trả lời tốt gồm:**
1. **Yêu cầu thật:** bản tin nhỏ (một mức sáng), thưa (khi cảm biến đổi), cần **ranh giới bản tin rõ ràng**, không cần băng thông.
2. **Vì sao mq hợp:** giữ **ranh giới bản tin** (không phải luồng byte như TCP ⇒ không phải tự framing — xem [LNX-005](linux-sysprog.md)) · có **độ ưu tiên bản tin** · có **hàng đợi sẵn**, bên nhận bận thì bản tin vẫn nằm đó · cắm được vào `select`/`epoll` qua fd (Linux).
3. **Vì sao không cái kia:** **shm** nhanh nhưng *không có cơ chế báo hiệu* và phải tự đồng bộ, một bên chết là kẹt khoá ([LNX-015](linux-sysprog.md)) — quá đắt cho vài byte mỗi lần · **TCP socket** phải tự framing, thừa cho giao tiếp nội máy.
4. **Ca hỏng:** một thiết bị chết / hàng đợi đầy thì sao? (mq đầy ⇒ `mq_send` chặn hoặc `EAGAIN` — bạn chọn cái nào và vì sao?)

**Nền kỹ thuật phải nắm:** [LNX-016](linux-sysprog.md) mq · [LNX-035](linux-sysprog.md) POSIX vs SysV · [LNX-017](linux-sysprog.md) chọn IPC · [ipc-linux.md §4 trục quyết định](../../../04-linux-system-programming/ipc-linux.md).


**⭐ Nửa câu trả lời hay bị thiếu:**

Resume ghi *"a **single binary** serving both standalone and synchronized modes"* ⇒ interviewer sẽ hỏi: **binary đó biết mình đang ở chế độ nào bằng cách nào?**

Phần **đã trả lời được**: cờ **Multi S-Box** do **application** gửi xuống lúc **runtime** (chính là mục bật tính năng trên thiết bị); bật rồi thì máy đó thành **master** phân phối giá trị thanh ghi cho các máy con.

🔴 **Phần còn thiếu — và là nửa quan trọng hơn:** *máy CON biết mình là con bằng cách nào?* Một binary đóng **hai vai** thì phải kể được **cả hai vai**, nếu không thì chưa chứng minh được "một binary hai chế độ" thực sự vận hành ra sao — mà đó lại chính là **điểm hay nhất** của câu chuyện S-Box.

Chốt sẵn ba câu này trước khi đi phỏng vấn:
1. Máy con nhận vai **thụ động** (cứ nghe hàng đợi, có lệnh thì áp) hay được **cấu hình** thành con?
2. Ai quyết định **ai là master** — người dùng chọn, hay máy tự thoả thuận?
3. **Master chết thì sao** — các máy con giữ giá trị cuối, hay quay về tự chỉnh theo cảm biến của mình?

> 📌 Câu 3 là câu interviewer thích nhất, vì nó hỏi về **chế độ hỏng** — và ở hệ nhiều thiết bị thì chế độ hỏng mới là phần thiết kế thật.

**Bẫy:** ① nói *"vì mq dễ dùng"* — không phải lý do kỹ thuật · ② không biết mq **có giới hạn** (`/proc/sys/fs/mqueue/msg_max`, mặc định 10 bản tin) · ③ không nghĩ tới **thiết bị chết giữa chừng** — với sản phẩm ghép nhiều màn thì đây là ca thật · ④ nhầm mq là liên máy được.
</details>

#### RES-006 · 🟡 · concept · ⭐ · [→ RESUME: "adaptive brightness control based on Ambient Light Sensor"]
**Kể luồng dữ liệu từ cảm biến ánh sáng tới lúc màn hình đổi độ sáng. Làm sao để màn không nhấp nháy liên tục?**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Chuỗi dữ liệu có bốn khúc. Một, **cảm biến ánh sáng nối qua I2C**; một library cảm biến riêng đọc nó theo nhịp của họ, 60 lần mỗi giây. Hai, phía library của em **lấy giá trị mới nhất mỗi 400 ms**. Ba, **lọc nhiễu**. Bốn, tính mức sáng mục tiêu rồi **đi dần** tới đó và ghi xuống phần cứng.*
>
> *Cách lọc nhiễu: em chia độ sáng môi trường thành **các khoảng** bằng một mảng ngưỡng. Mỗi lần đọc, giá trị rơi vào khoảng nào thì đếm cho khoảng đó. Nếu **10 lần đọc liên tiếp — tức 4 giây — cùng rơi vào một khoảng** thì coi là ổn định, lấy mức sáng của khoảng đó để tính mục tiêu mới; chỉ cần một lần rơi sang khoảng khác là đặt lại bộ đếm và bắt đầu một cửa sổ 4 giây mới. Một bàn tay lướt qua hay đèn flash không đủ 4 giây nên không làm đổi mục tiêu, và việc chia thành khoảng giúp giá trị dao động nhẹ quanh một mức không làm đổi sáng.*
>
> *Chống nhấp nháy thì có hai lớp: mục tiêu chỉ đổi khi ánh sáng đã ổn định 4 giây, và khi đổi thì một thread chạy theo chu kỳ đưa độ sáng **tiến từng bước nhỏ**, nên mắt thấy màn sáng lên từ từ chứ không nhảy bậc.*
>
> *Đánh đổi là **mượt hay nhạy**: bật đèn phòng thì màn phải mất ít nhất 4 giây mới bắt đầu đổi theo. Với biển quảng cáo, nơi ánh sáng thay đổi chậm, em chọn mượt."*

**Nếu bị hỏi tiếp**
- *"Ánh sáng dao động ngay ranh giới hai khoảng thì sao?"* — Cửa sổ cứ bị đặt lại nên mục tiêu giữ nguyên — đó là hành vi mong muốn, màn không nhảy qua lại. Cải tiến được nữa là dùng **hysteresis**: ngưỡng đi lên khác ngưỡng đi xuống.
- *"Mất liên lạc với cảm biến?"* — Độ sáng được giữ ở **mức mặc định** — tài liệu lắp đặt của sản phẩm cũng ghi đúng hành vi này; kiểm lại cáp và khởi động lại box thì chức năng chạy lại.
- *"Sao library của em không tự đọc I2C luôn?"* — Đọc cảm biến là việc của library cảm biến, chạy theo nhịp riêng; library của em chỉ cần **giá trị mới nhất** — đúng mô hình hộp thư một ô ([CPP-071](cpp.md)).

**Móc nền tảng:** thread chu kỳ + condvar ([CPP-069](cpp.md), [CPP-070](cpp.md)) · latest-value ([CPP-071](cpp.md)) · I2C · polling vs interrupt.

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn nắm **cả chuỗi** hay chỉ một khúc · ② có nghĩ tới **chất lượng trải nghiệm** không, hay chỉ "đọc được giá trị là xong". Câu *"chống nhấp nháy"* là câu phân loại.

**Câu trả lời tốt gồm — vẽ chuỗi:**
```
ALS (I2C) -> driver đọc định kỳ / interrupt -> lọc & làm mượt
          -> ánh xạ lux -> mức sáng -> ghi xuống panel
```

**Phần chống nhấp nháy — nêu được ít nhất hai cơ chế:**
| Cơ chế | Chống được gì |
|---|---|
| **Hysteresis** (ngưỡng lên khác ngưỡng xuống) | Dao động quanh đúng một ngưỡng ⇒ bật/tắt liên tục |
| **Lọc trung bình trượt / EMA** | Nhiễu tức thời (bàn tay lướt qua, đèn flash) |
| **Chuyển dần theo thời gian (ramp)** | Nhảy bậc thấy rõ ⇒ đổi mượt trong vài trăm ms |
| **Chu kỳ lấy mẫu hợp lý** | Lấy quá dày tốn điện + nhiễu; quá thưa thì phản ứng chậm |

**Đánh đổi phải nêu:** lọc mạnh ⇒ ổn định nhưng **phản ứng chậm** (bật đèn phòng mà 5 giây sau màn mới sáng); lọc nhẹ ⇒ nhạy nhưng nhấp nháy. Đây là chỗ cho thấy bạn từng **chỉnh số thật**.

**Nền kỹ thuật phải nắm:** I2C ([bus-protocols](../../../05-drivers-device-tree/bus-protocols.md), [BUS-001…007](drivers-embedded.md)) · polling vs interrupt ([BSP-010…013](bsp.md)) · sysfs phơi tham số ra userspace ([DRV-035](drivers-embedded.md)).

**Bẫy:** ① chỉ kể *"đọc sensor rồi set brightness"* — bỏ mất phần khó nhất · ② không nói được **đọc bằng polling hay interrupt** và vì sao · ③ không biết cảm biến của mình nối bằng bus gì (chuẩn bị: I2C) · ④ quên phần **người dùng chỉnh tay** thì hệ tự động phải nhường ra sao.
</details>

#### RES-007 · 🟢 · concept · ⭐ · [→ RESUME: "picture quality (dimming) and panel control (resolution, frame rate, backlight)"]
**Picture quality (dimming) và panel control — em giải thích ngắn gọn cho người ngoài ngành hiểu được không?**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Màn hình có hai câu hỏi.*
>
> *Câu một: **sáng bao nhiêu?** Đằng sau tấm màn là dàn đèn. Phần **picture quality** — chỗ em làm là dimming — tính xem đèn nên sáng bao nhiêu theo nội dung đang chiếu: cảnh tối thì hạ đèn ở vùng tối, nên màu đen sâu hơn và tốn ít điện hơn. Phần này là **tính toán**, mỗi dòng sản phẩm một thuật toán.*
>
> *Câu hai: **hiển thị thế nào?** Bật tắt màn, đổi chế độ hiển thị, và ghi con số độ sáng vừa tính xuống phần cứng. Đó là **panel control** — chỉ là **ra lệnh**, xuống driver của tấm màn trong kernel.*
>
> *Vì một bên là tính toán, một bên là ra lệnh, nên trong library em tách làm hai phần — đó cũng là hai nhánh của kiến trúc. Phần em viết là thuật toán dimming và đường đi của nó từ C++ interface xuống tới driver."*

**Nếu bị hỏi tiếp**
- *"Sao không để mỗi tính năng tự ghi thẳng xuống phần cứng?"* — Có **một cổng** xuống phần cứng thì dễ kiểm soát, dễ log, dễ khoá. Lệnh đơn đi thẳng panel control; tính năng nặng thuật toán thì tính xong mới đưa xuống cổng đó.

**Móc nền tảng:** kiến trúc library ([RES-035](resume.md)) · `ioctl` ([DRV-006](drivers-embedded.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn **hiểu** hay chỉ chép từ khoá vào resume · ② **năng lực diễn đạt** — giải thích được cho người không làm TV là dấu hiệu senior rõ nhất.

**Câu trả lời tốt — hai câu hỏi về một tấm panel, ngôn ngữ đời thường trước, thuật ngữ sau:**
- **Picture quality — "sáng bao nhiêu?"**: *"đằng sau tấm panel là dàn đèn LED; dimming hạ đèn ở vùng ảnh tối — đen sâu hơn, tốn ít điện hơn. Phần này phải **tính toán** theo nội dung từng khung hình."*
- **Panel control — "hiển thị thế nào?"**: *"bật tắt panel, đổi độ phân giải, đổi số hình mỗi giây, ghi độ sáng đã tính xuống phần cứng. Phần này chỉ là **ra lệnh**, xuống **panel driver** trong kernel."*
- **Câu nối sang kiến trúc:** *"vì một bên là tính toán, một bên là ra lệnh, nên trong library em tách làm hai component — đó cũng là hai nhánh của kiến trúc."* Câu này biến câu 🟢 thành móc dẫn sang câu kiến trúc ([A1 §4](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)).

**Ghi điểm thêm:** nói **bạn động vào phần nào** — interface C++, library, hay driver.

**Bẫy:** ① thuật ngữ chồng thuật ngữ (PWM, judder, resolution trần) — nghe là biết chưa quen giải thích · ② nói dài 3 phút cho một câu 🟢 · ③ không nói mình làm phần nào.
</details>

#### RES-008 · 🟠 · concept · ⭐ · 🏗️ · [→ RESUME: "cross-layer debugging (user-space & kernel-space)"]
**Resume ghi em debug xuyên tầng user–kernel. Kể một ca cụ thể: triệu chứng ban đầu là gì, em khoanh vùng ra sao, gốc rễ nằm ở đâu?**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Ca em hay kể là **TV khởi động lên hình chậm** trên model mới. Triệu chứng ở tầng người dùng, nhưng nguyên nhân có thể ở bất kỳ tầng nào: ứng dụng ra lệnh bật màn muộn, library chậm, hay driver trong kernel chưa sẵn sàng.*
>
> *Việc đầu tiên em làm là **một phép đo chia đôi**: đặt mốc thời gian ở hai phía ranh giới — lúc user-space phát lệnh bật màn, và lúc driver của màn được nạp xong trong log kernel. Kết quả: lệnh tới đúng giờ, nhưng **driver chưa nạp xong** — tức là lỗi nằm ở phía kernel, khỏi phải soi ứng dụng.*
>
> *Chia tiếp ở phía kernel: phần code khởi tạo của driver chạy nhanh, nhưng **thời điểm bắt đầu** của nó bị trễ. Log và tải CPU cho thấy từ giây thứ tư có một tiến trình **ưu tiên thời gian thực** chiếm CPU, trong khi các driver nền được nạp **lần lượt từng cái** với ưu tiên thường — nên bị chen ngang. Thời gian nạp từ dưới nửa giây thành 3–4 giây.*
>
> *Em sửa bằng cách nạp song song kết hợp nâng ưu tiên, nhờ team hiệu năng đánh giá tác động lên phần khởi động khác, và nghiệm thu qua **100 lần khởi động**. Để lần sau lộ sớm, mốc thời gian nạp driver được giữ trong log khởi động để so giữa các bản build."*

**Ca thứ hai (ngắn), nếu được hỏi thêm:** *màn bật, đèn nền sáng, nhưng hình **đen hoàn toàn**.* Đo ở ranh giới: lệnh bật đèn đã xuống tới phần cứng và đèn sáng thật — nên không phải lỗi đường bật màn. Gốc rễ là một khối xử lý hình phía trước vẫn đang bật **lớp che màu đen** — lớp dùng để che nhiễu lúc khởi động — vì thiếu lệnh gỡ lớp che trong trình tự bật. Sửa: thêm bước đó vào trình tự bật màn.

**Nếu bị hỏi tiếp**
- *"Ca treo, không crash, không log thì sao?"* — Vẫn là chia đôi: treo ở user hay kernel? `/proc/<pid>/wchan` và stack của thread cho biết nó đang chờ gì trong kernel; `gdb -p` + backtrace mọi thread cho phía user ([DBG-010](debugging.md)).
- *"Công cụ ở từng tầng?"* — User: log, gdb, core dump, strace. Ranh giới: giá trị trả về của `ioctl`, `errno`, sysfs. Kernel: `dmesg`/`printk` có mốc thời gian, ftrace.

**Móc nền tảng:** chia đôi không gian nghi ngờ ([09/mindset](../../../09-debugging/mindset.md)) · debug cross-layer ([DBG-016](debugging.md)) · scheduling: ưu tiên thường vs thời gian thực ([RES-015](resume.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ **Câu rủi ro cao nhất trong resume của bạn.** Bạn **tự liệt kê debug là kỹ năng**, nên họ sẽ khoan — trong khi [CLAUDE.md §2](../../../CLAUDE.md) ghi đây là điểm yếu tự nhận. Phải chuẩn bị **một ca thật, kể trôi chảy**.

**Câu trả lời tốt gồm — theo đúng thứ tự này:**
1. **Triệu chứng như người dùng thấy** (*"màn chớp đen ~1 giây khi chuyển nguồn vào"*), không phải như log thấy.
2. **Chia đôi bài toán:** lỗi ở user hay kernel? Bằng chứng nào chia được? (log hai bên, `dmesg` timestamp, thử gọi thẳng ioctl bỏ qua tầng trên…)
3. **Thu hẹp dần** — mỗi bước loại được một nửa. Đây là phần được chấm.
4. **Gốc rễ + cách sửa + cách chặn tái diễn** (thêm log? thêm test? thêm kiểm tra tham số?).

**Nền kỹ thuật phải nắm — chuẩn bị gọi tên được công cụ ở từng tầng:**

| Tầng | Công cụ |
|---|---|
| Userspace | `strace`, `gdb`, core dump, ASan, log |
| Ranh giới | `ioctl` trả lỗi gì, `errno`, `/proc`, `/sys` |
| Kernel | `dmesg`/`printk`, `ftrace`, `/proc/<pid>/wchan` |

🧪 **Chuẩn bị bằng tay:** làm [DBG-030…036](debugging.md) — 7 bài lab đúng cho câu này, đặc biệt [DBG-033](debugging.md) (core dump), [DBG-034](debugging.md) (treo).


> 🔴 **BẰNG CHỨNG 2026-09-12 — đọc kỹ trước khi ôn tiếp.** Phần "cắt đôi" dưới đây **đã nằm sẵn trong bank từ trước phiên B1**. Trong phiên đó, ứng viên vẫn trả lời bằng **quét tuần tự từ trên xuống**, và **giữ nguyên câu trả lời sau HAI lần được làm rõ rằng câu hỏi muốn một phép cắt đôi** ⇒ **2 điểm**.
>
> ⇒ Đây **không phải lỗ hổng nội dung, mà là lỗi TRUY XUẤT.** Đọc lại mục này lần nữa **sẽ không chữa được** — cùng một cách đã thất bại. Cách chữa: **luyện phản xạ** — mỗi khi nghe một triệu chứng, câu đầu tiên bật ra phải là *"phép đo nào chia đôi được?"*, trước cả khi nghĩ tới log.
> 🧪 Bộ lab `DBG-030…041` tồn tại đúng cho việc này — làm `DBG-032` ASan + `DBG-036` valgrind trước.

**⭐ Câu này hỏi CẮT ĐÔI, không hỏi quy trình:**

Tình huống điển hình: *userspace gọi API set độ sáng, **hàm trả về thành công**, nhưng **màn hình không đổi**.*

❌ **Cách trả lời được 3 điểm** — đưa một quy trình **quét tuần tự từ trên xuống**: khoanh vùng thời gian → đọc log userspace → nếu log sạch thì nghi kernel → điều tra kernel. Đúng, nhưng đó là **dò tuần tự**, chi phí O(số tầng).

✅ **Cách trả lời được 4 điểm — MỘT phép đo chia đôi không gian nghi ngờ:**

> *"Việc đầu tiên em làm là **đọc ngược giá trị ngay tại biên** — `printk` đúng chỗ driver ghi thanh ghi, hoặc đọc lại register/sysfs. Một phép đo đó trả lời dứt khoát:*
> - *giá trị **có** xuống tới kernel ⇒ lỗi nằm ở **kernel/SoC** (ghi sai thanh ghi, sai thời điểm, SoC không nhận)*
> - *giá trị **chưa** xuống ⇒ lỗi ở **userspace / đường truyền** (sai ioctl, bị nuốt ở tầng thư viện, gọi nhầm instance)"*

**Vì sao khác biệt này quan trọng chứ không phải tiểu tiết:** với stack nhiều tầng (app → C++ interface → shared lib → ioctl → driver → SoC), quét tuần tự tốn **O(n)** lần điều tra, chia đôi tốn **O(log n)**. Và mỗi lần điều tra ở kernel là một lần build + flash + reboot.

📌 **Đây trùng đúng điểm yếu tự nhận ở [CLAUDE.md §2](../../../CLAUDE.md)** (*"debug chủ yếu đọc log + so code + suy luận"*). Phân biệt cần nhớ: **đọc log là THU THẬP bằng chứng; chia đôi là THIẾT KẾ phép đo.** Interviewer hỏi *"bước đầu tiên"* là đang dò vế thứ hai.

**Ôn:** [09-debugging/mindset.md](../../../09-debugging/mindset.md) — chia đôi không gian nghi ngờ.

**Bẫy:** ① *"em đọc log rồi so code"* — đúng là cách bạn đang làm, nhưng nói trần trụi vậy nghe như **không có phương pháp**. Vẫn kể sự thật, nhưng kể **có cấu trúc**: *"em bắt đầu từ log để dựng mốc thời gian, rồi khoanh vùng bằng cách…"* · ② kể một ca mà **gốc rễ hoá ra người khác tìm ra** · ③ không nêu được **cách chặn tái diễn** — interviewer senior rất để ý phần này.
</details>

#### RES-009 · 🟡 · concept · 🏗️ · [→ RESUME: "Automated driver/library porting with Python tooling… reducing manual effort by more than 50%"]
**Em ghi tool porting giảm hơn 50% công sức thủ công. Con số đó đo bằng cách nào?**

> 📌 *Neo lại 2026-09-04: con số cũ **"giảm 70% thời gian làm việc" đã bị gỡ khỏi resume**. Câu hỏi giữ nguyên giá trị — chỉ đổi sang con số còn sống. **Không xoá ID**: kỹ năng *bảo vệ một con số mình tự viết ra* vẫn là câu lọc.*

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Bài toán: mỗi năm có chip mới, phải port driver và library sang — phần lớn là việc **lặp lại**: chép source gốc, đổi tên theo chip mới, thêm nhánh cho chip mới vào các chỗ rẽ nhánh, sửa CMake, Makefile và cấu hình build. Nhiều file, hàng chục nghìn dòng, rất dễ sót.*
>
> *Tool Python của em làm phần lặp lại đó. Và em **cố ý để lại cho con người** những gì phụ thuộc phần cứng: địa chỉ thanh ghi, bus, device tree, phần porting kernel, và việc tạo tổ hợp chip với tấm màn.*
>
> *Con số **1–2 ngày xuống 2–3 giờ** là cho **phần thủ công của một lần port**, không phải cả dự án, và em nói thẳng đó là **ước lượng** của em qua các lần làm cả hai cách, không phải số đo chính thức.*
>
> *Kiểm tra bản port: review và build; ra đúng file driver cho tổ hợp mới là bước đầu; sau đó test trên model mới — khởi động được và nạp đúng cài đặt. Nếu làm tiếp, em cho tool **tự kiểm** ngay sau khi sinh: tên chip cũ phải biến mất hết, mọi tổ hợp khai báo đều có phần tương ứng; và chạy tool trên một chip đã port tay rồi so khác biệt."*

**Nếu bị hỏi tiếp**
- *"Sao không để AI làm?"* — Việc này phải chạy **y hệt** cho hàng chục tổ hợp và phải **review được từng dòng**; cần phép biến đổi xác định nên là script. AI hợp khi cần phán đoán.
- *"Ước lượng thì sao dám ghi vào resume?"* — Vì phạm vi đã ghi hẹp (*"manual effort"*) và em nói rõ cách ước lượng khi được hỏi; em không trình bày nó như một phép đo.

**Móc nền tảng:** CMake/Makefile · build nhiều tổ hợp trong CI ([BLD-021](build-systems.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ **Đây là câu kiểm tra độ tin cậy, không phải câu kỹ thuật.** Mọi con số trong resume đều mời gọi câu hỏi này. Trả lời được ⇒ toàn bộ resume tăng độ tin. Ú ớ ⇒ interviewer bắt đầu nghi ngờ **mọi** con số khác.

**Câu trả lời tốt gồm:**
1. **Phạm vi hẹp lại cho chính xác:** 50% của **việc gì** — không phải toàn bộ công việc, mà là *"phần thủ công khi port một driver sang chipset mới: sửa CMake/Makefile + chỉnh source cho khớp nền tảng"*. ⚠️ Chữ *"manual effort"* trong resume đã hẹp sẵn — **bám vào nó**, đừng để interviewer hiểu thành *"nhanh gấp đôi cả dự án"*.
2. **Mốc so sánh:** trước đây làm tay mất bao lâu cho **một lần port**, sau khi có tool còn bao lâu — và **phần nào tool KHÔNG làm được** (vẫn phải review, vẫn phải sửa chỗ đặc thù chipset).
3. **Đo trên bao nhiêu mẫu:** *"em đo trên N module đã làm cả hai cách"* — hoặc **thừa nhận là ước lượng** nếu đúng là ước lượng.
4. **Giới hạn:** chỗ nào tool **không** giúp được (logic phức tạp vẫn phải viết tay; phải review lại output).

> **Trung thực là chiến lược tốt nhất ở đây.** *"Đây là ước lượng của em dựa trên N lần làm, không phải số đo chính thức"* — câu này **an toàn hơn** một con số cứng mà không có cơ sở.

**Nếu không bảo vệ được con số:** cân nhắc sửa resume thành phát biểu định tính (*"significantly reduced"*) hoặc thu hẹp phạm vi rõ ràng.

🔗 **Cùng loại câu, khác con số:** [RES-013](resume.md) (*3–4s → 0,5s* — con số **đo được**, dễ bảo vệ nhất) · [RES-016](resume.md) (*1 ngày → 1–2 giờ* — con số **rủi ro nhất**, vì nó nói về AI chứ không về bạn). Chuẩn bị cả ba, vì cả ba đều nằm trong resume.

**Bẫy:** ① bịa thêm chi tiết cho khớp — mâu thuẫn sẽ lộ ở câu sau · ② trả lời *"sếp em bảo vậy"* · ③ để lộ rằng 70% là **phỏng đoán một lần** nhưng lại viết như một phép đo.
</details>

#### RES-010 · 🟡 · concept · ⭐ · [→ RESUME: "device tree updates, new hardware support"]
**Em có làm device tree cho phần cứng mới. Cụ thể em sửa gì, và làm sao driver biết nó phải chạy cho node nào?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **kiến thức nền BSP** mà JD Datalogic cần. Câu này rất dễ đào sâu và bạn **nên** để nó được đào — đây là vùng mạnh tiềm năng của bạn.

**Câu trả lời tốt gồm:**
1. **DT dùng để làm gì:** mô tả **phần cứng không tự khai báo được** (khác PCI/USB tự liệt kê) ⇒ kernel biết có thiết bị gì, ở địa chỉ nào, dùng IRQ/GPIO/clock nào — **không phải biên dịch lại kernel** cho mỗi board.
2. **Bạn sửa gì cụ thể:** thêm node cho chip mới, đặt `compatible`, khai địa chỉ thanh ghi (`reg`), chân GPIO, IRQ, clock, và các tham số riêng.
3. **Cơ chế ghép:** chuỗi **`compatible`** trong node khớp với bảng `of_match_table` trong driver ⇒ kernel gọi **`probe()`** của driver đó. Nói được vòng đời `probe`/`remove` là đạt.
4. **Nếu có:** kể ca `EPROBE_DEFER` — driver cần một tài nguyên chưa sẵn sàng nên xin hoãn, kernel gọi lại sau. Nêu được là điểm cộng thật.

**Nền kỹ thuật phải nắm:** [device-tree.md](../../../05-drivers-device-tree/device-tree.md) · [DRV-028…034](drivers-embedded.md) (DTS/DTB/dtc, `compatible`, driver đọc DT) · [BSP-006…009](bsp.md) (probe, `EPROBE_DEFER`, pinctrl).

**Bẫy:** ① nhầm DT là **code chạy** — nó là **dữ liệu mô tả**, bootloader nạp `.dtb` rồi truyền cho kernel · ② không phân biệt được `.dts` (nguồn) / `.dtb` (đã biên dịch) / `dtc` (trình biên dịch) · ③ không giải thích được vì sao x86 **không** dùng DT (dùng ACPI).
</details>

#### RES-011 · 🟡 · concept · 🏗️ · [→ RESUME: "Samsung Display Manager (Windows Application)", MVVM]
**Em có làm ứng dụng Windows với MVVM — kể về nó.** *(và cách lái về đúng thế mạnh)*

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** thường chỉ là **hỏi cho đủ resume**. Nhưng với JD Embedded Linux, đây là mục **ít liên quan nhất** — mục tiêu của bạn là trả lời tử tế rồi **lái về** mảng mạnh, không sa đà.

**Câu trả lời tốt gồm:**
1. **Mở bằng BỐI CẢNH + KẾT QUẢ, không mở bằng tính năng** — *"Lúc em vào, app điều khiển được màn hình và đã lưu được setting của chính app bằng JSON, nhưng chưa có khái niệm preset. Em làm Preset và import/export preset giữa các màn hình, xong trong một tháng, kịp release 1.0 của team."* Một câu này trả lời luôn ba thứ interviewer định hỏi sau: **phạm vi**, **thời gian**, **kết quả**. Chi tiết & bẫy: [RES-022](resume.md).
2. **Ngắn** — 30–45 giây tổng. Preset, import/export preset sang màn hình khác, kiến trúc MVVM.
3. **Rút ra thứ CHUYỂN ĐƯỢC sang embedded**, đây mới là phần đáng nói:
   - **MVVM = tách trạng thái khỏi hiển thị** — cùng tư duy với tách *business logic* khỏi *tầng phần cứng* trong HAL.
   - **Cấu hình bền vững qua khởi động lại** — bài toán y hệt trên thiết bị (lưu setting vào flash, lo mất điện giữa chừng).
   - **Modern C++** — RAII, smart pointer, `std::optional`. Nói **tên thứ mình dùng được**, đừng nói tên chuẩn: xem cảnh báo ⚠️ ngay dưới.
4. **Tự lái:** *"phần này giúp em rõ hơn về tách tầng, nhưng mảng em đầu tư sâu là system software và driver ở phía Linux."*

> ⚠️ **Đừng gắn nhãn "C++17" cho riêng SDM.** App build được tới C++20, và thứ duy nhất thuộc C++17 dùng trong đó là `std::optional` — nói "C++17" là tự mời một câu hỏi mình không có gì để trả lời. Resume **đã bỏ nhãn này** khỏi mục SDM (07/09), chỉ còn "Modern C++".
> Nhãn **17** vẫn còn ở dòng **TECHNICAL SKILLS** (`Modern C++ (11/14/17)`) và nó **phải bảo vệ được bằng cả 3 năm**, không phải bằng SDM. Chuẩn bị sẵn ít nhất **hai** thứ C++17 dùng thật ngoài `optional` (structured binding, `if constexpr`, `string_view`, `[[nodiscard]]`…) — nếu không có, hạ dòng Skills xuống "(11/14)" còn an toàn hơn.

🔗 **Đào sâu ở đâu:** [RES-017](resume.md) (bản nháp vs sửa trực tiếp) · [RES-018](resume.md) (DDC/CI ≈ I²C) · [RES-019](resume.md) (định danh màn hình) · [RES-020](resume.md) (ghi file & mất điện) · [RES-021](resume.md) (đồng bộ Save/Load).

---

### 🧭 KHUNG KỂ CHUNG cho mọi câu SDM — *"bản đầu tiên + hướng cải tiến"*

> ⚠️ **Đặt ở đây MỘT CHỖ**, các câu `RES-017…021` trỏ về. Đừng chép lại.

SDM là **thế hệ đầu**, làm để **kịp tiến độ dự án**. Nhiều phần **cố ý chưa tối ưu**. Đó là sự thật, và nói ra là **đúng** — nhưng cách nói quyết định bạn nghe như *kỹ sư biết đánh đổi* hay *người bào chữa*.

**Ba mức trả lời, xếp từ mạnh xuống yếu:**

| Mức | Dạng câu | Interviewer nghe thấy |
|---|---|---|
| ⭐⭐ **Senior** | *"Em **biết** rủi ro X. Chấp nhận vì ràng buộc Y **cụ thể**, và **bán kính ảnh hưởng** chỉ là Z. Hướng đúng là W."* | đánh đổi **có ý thức** |
| ⭐ **Mid, trung thực** | *"Lúc đó em **không nghĩ tới**. Sau này nhìn lại thì đúng ra nên W."* | thành thật, học được |
| 🔴 **Yếu** | *"Phần đó chưa tối ưu vì lúc đó gấp."* | **bào chữa** — và nếu lặp ở mọi câu thì thành mẫu xấu |

🔴 **Chỗ hỏng phải tránh:** nói *"v1 nên chưa tối ưu"* ở **mọi** câu. Một lần là bối cảnh; ba lần là **cái cớ mặc định**. Mỗi câu phải có **ràng buộc riêng** và **bán kính ảnh hưởng riêng**.

✅ **Và luôn có sẵn vế thứ hai:** *"đồng nghiệp cầm dự án sau này đã cải tiến — họ làm bằng W"* (như state machine ở [RES-017](resume.md)). Nó cho thấy bạn **theo dõi tiếp** sản phẩm sau khi rời tay, và bạn **đồng ý** với hướng đúng.


### ✅ Bản trả lời đã đạt 4đ (2026-08-29) — chốt lại, đừng dựng lại từ đầu

Góc hỏi: *"Bạn ứng tuyển Embedded Linux. Vì sao trong resume lại để một app Windows? Nó chứng minh điều gì mà bốn mục kia không chứng minh được?"*

**Ba lý do, theo đúng thứ tự này:**

1. **Khả năng thích ứng — có con số** — nhận một dự án ngoài chuyên môn, đang thiếu người, và **một tháng sau team có release 1.0**. Đừng nói suông "em thích ứng nhanh"; nói mốc thời gian ([RES-022](resume.md)).
2. **Modern C++ vận dụng thật** — move semantics, smart pointer, RAII, OOP design; và nó **quay lại phục vụ** chất lượng code cho C++ interface / shared library ở phía embedded.
3. ⭐⭐ **MVVM → HAL, qua Dependency Inversion** — đây là câu ăn điểm, vì nó biến một mục "lạc đề" thành bằng chứng cho đúng thứ JD cần:

> *"MVVM giúp em hiểu thêm cách HAL hoạt động trên embedded: **lớp trên không biết hardware, hardware không biết lớp trên** — hai bên chỉ gặp nhau ở một interface trừu tượng. Kết quả là coupling thấp, và **mock test được** khi chưa có phần cứng."*

📌 **Vì sao (3) mạnh:** nó không phải phép so sánh cho vui. Trong chính project đó bạn **đã dùng** `MockAppViewModel` để dựng UI khi chưa có màn hình Samsung cắm vào — **đúng cùng một lý do** người ta tách HAL để test driver không cần board. Nêu được ví dụ `Mock` này là biến lý thuyết thành **bằng chứng đã làm**.

**Bẫy:** ① kể dài về UI/Windows trong buổi phỏng vấn Embedded Linux · ② để lộ rằng đây là project bạn thích nhất (interviewer sẽ nghi bạn không hợp vị trí embedded) · ③ ngược lại — **coi thường project của chính mình** (*"cái đó không liên quan đâu ạ"*) — luôn rút ra được điều gì đó.
</details>

#### RES-012 · 🟠 · concept · ⭐ · 🏗️ · [→ RESUME vs JD: thiếu Yocto, I2C/SPI, PCI/USB]
**"Anh thấy resume em không nhắc Yocto, cũng không thấy I2C/SPI hay PCI/USB. Bên anh dùng những thứ đó khá nhiều."**

> 📌 *Resume không ghi Yocto/BitBake* ⟹ câu này đến **từ phía JD**, không từ resume. Rủi ro nói quá bên dưới vẫn nguyên: đừng tự đưa ra khẳng định mà resume không có.

> 🔴 **CẢNH BÁO NÓI QUÁ — sự cố lặp lần thứ hai (14/09).** Trả lời câu Yocto, ứng viên nói *"…và **đã thành công build một minimal image**"*, rồi khi bị đào chỉ đưa được *"image cho BeagleBone, PC x86, ~30 phút, đặt MACHINE là beaglebone"* — **không** tên image, **không** giá trị `MACHINE` đúng (`beaglebone-yocto`), **không** gì trong `local.conf`. Tracking repo ([plan §10](../../study-plans/archive/datalogic-plan.md)) ghi lab BSP **1/8**, `BSP-036` (Yocto) **⬜ chưa làm**. Cùng khẳng định này **đã bị phân xử là sai ngày 06/09** ([bài học #8](../../study-plans/archive/datalogic-plan.md)).
>
> ⇒ **Chọn một trước buổi phỏng vấn:** ⓐ **làm thật** `BSP-036` (một buổi tối) rồi mọi thứ nói ra đều thật; hoặc ⓑ **hạ khẳng định**: *"Em chưa vận hành Yocto trong sản phẩm. Phần em thật sự chạm là **sstate** — em đã đo ba phép đo về cache build."* — cái này **có thật** (`BSP-038` ✅) và **vẫn mạnh**, vì `sstate` là chi tiết chỉ người đã chạm mới nói.
>
> **Câu chữ đã chốt khi bị hỏi về board (06/09 — dùng nguyên văn):** *"Em chưa dùng Yocto trong sản phẩm thật. Em **có board BBB**, đã dựng xong console serial qua CP2102 và boot được, **đang bắt đầu bộ lab**."* Kèm một chi tiết chỉ người cầm board mới biết: **giữ S2 lúc cấp nguồn để SD thắng eMMC**. Phân biệt rạch ròi *"dựng được môi trường"* với *"build được image"* — bị khoan thì hai câu đó đọc như nhau.
>
> **Phép thử trước khi khẳng định bất cứ điều gì:** *"nếu họ đào ba câu nữa, mình còn trả lời được không?"* Không ⇒ đừng khẳng định. Khung xử lý khoảng trống đầy đủ: [BEH-017](behavioral.md).


<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ Đây **không** phải câu để loại bạn — đây là câu đo **cách bạn xử lý khoảng trống**. Phản ứng của bạn quan trọng hơn nội dung.

**Công thức ba bước — đừng chối, đừng xin lỗi:**

1. **Thừa nhận thẳng, không vòng vo.** *"Đúng ạ, em chưa làm Yocto trong sản phẩm thật."* — một câu, không kèm biện hộ.
2. **Bắc cầu sang thứ gần nhất bạn CÓ.** Đây là phần được chấm:
   - **Yocto** ← bạn có **cross-compilation**, **CMake/Makefile**, **build system cho nhiều nền tảng**, **port driver qua nhiều chipset**. Bạn hiểu *vấn đề* Yocto giải quyết (dựng cả bản phân phối tái lập được cho thiết bị), dù chưa gõ recipe.
   - **I2C/SPI** ← bạn làm **cảm biến ánh sáng** (gần như chắc chắn qua I2C) và **driver nói chuyện với SoC** qua thanh ghi.
   - **PCI/USB** ← trung thực là chưa; nêu bạn nắm nguyên lý chung (device enumeration, driver model) và học nhanh vì nền driver đã có.
3. **Nói cụ thể mình đang làm gì để lấp** — *"em đang đọc về layer/recipe/sstate của Yocto"* — cụ thể, không hứa suông.

**Nền kỹ thuật nên có trước khi đi phỏng vấn:** [yocto.md](../../../06-build-systems/yocto.md) ở mức **tư duy** (layer/bbappend để làm gì · sstate giải quyết gì · DEPENDS vs RDEPENDS) — [plan nguyên tắc ①](../../study-plans/archive/datalogic-plan.md) ghi rõ **không cần thuộc cú pháp recipe** · [bus-protocols](../../../05-drivers-device-tree/bus-protocols.md) · [pci-usb-drivers](../../../05-drivers-device-tree/pci-usb-drivers.md).

**Bẫy:** ① nói *"em có biết Yocto"* khi chỉ đọc qua — hỏi hai câu là lộ, và **mất niềm tin cho cả buổi** · ② xin lỗi dài dòng, hạ thấp bản thân · ③ chỉ nói *"em học nhanh lắm"* mà không có bằng chứng — trong khi bạn **có** bằng chứng thật: đã port driver qua nhiều chipset và migrate kernel hai đời.
</details>

---

#### RES-017 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-04 · [→ RESUME: "Preset feature … users can save their preferred display settings"]
**"Khi người dùng đang sửa một Preset rồi bấm Cancel, em quay lại trạng thái cũ bằng cách nào? Vì sao không cho sửa trên một bản nháp trong bộ nhớ cho an toàn?"**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Phương án đầu tiên em làm là **bản nháp**: sửa vào một bản sao trong bộ nhớ, bấm Save mới ghi xuống màn hình. Nghe an toàn, nhưng em bỏ vì hai lý do khác loại nhau. Về **trải nghiệm**: người dùng kéo thanh độ sáng mà màn **không sáng lên**, tức là đang chỉnh mù — với app mà mục đích là nhìn thấy kết quả thì không chấp nhận được. Về **code**: mọi hàm đọc ghi setting phải rẽ nhánh *"đang sửa hay không"*, rải khắp nơi, rất dễ sót một chỗ.*
>
> *Phương án em chốt: **sửa thẳng lên màn hình thật**, nhưng trước khi vào chế độ sửa thì **chụp lại** cấu hình đang dùng; bấm Cancel thì ghi bản chụp đó trở lại.*
>
> *Cái giá em nhận: trạng thái cần khôi phục giờ nằm **trên phần cứng**, nên **mọi đường thoát** khỏi chế độ sửa phải được kiểm soát — Cancel, đóng app, rút cáp, chuyển sang màn khác. Em xử lý bằng cờ trạng thái; đúng nhưng dễ quên ở chỗ thêm mới. Sau này team chuyển sang **state machine**, và em thấy đó là lời giải tốt hơn: thêm một đường thoát mới thì bảng trạng thái buộc mình trả lời nó phải làm gì."*

**Nếu bị hỏi tiếp**
- *"Vì sao cờ không đủ?"* — Cờ đúng ở lúc viết, hỏng ở lúc người sau thêm đường thoát mới mà không biết phải xử lý cờ.

**Móc nền tảng:** snapshot/rollback (giống bản A/B khi cập nhật firmware) · State pattern · Memento ([DP-032](design-patterns.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⭐ Đây là câu **đáng giá nhất** của cả project — vì nó là một quyết định thiết kế mà ứng viên **đã tự đảo ngược**. Họ dò: ① bạn có tự đánh giá lại thiết kế của mình không · ② bạn có nêu được **cái giá** của phương án mình chọn không · ③ bạn có biết **lời giải tốt hơn** mà mình đã không dùng không. Rất ít ứng viên mid-level trả lời được cả ba.

**Câu trả lời tốt gồm — kể theo đúng trình tự đã xảy ra:**

1. **Phương án đầu (bản nháp):** sửa vào một bản sao trong bộ nhớ, bấm **Save** mới ghi xuống. Nghe an toàn.
2. **Hai lý do bỏ nó** — nêu **cả hai**, vì chúng thuộc hai loại khác nhau:
   - 🔴 **Sai về trải nghiệm:** người dùng kéo thanh **độ sáng** mà **màn hình không sáng lên** ⇒ đang **chỉnh mù**. Với một app mà toàn bộ mục đích là *nhìn thấy kết quả*, đó là lỗi chí mạng.
   - 🔴 **Sai về code:** để có bản nháp thì **mọi getter/setter phải rẽ nhánh** *"đang edit hay không"* — rải khắp nơi, **dễ sót một chỗ** và UI hiển thị sai.
3. **Phương án chốt:** **sửa thẳng lên màn hình thật**; trước khi vào chế độ edit thì **chụp lại trạng thái đang dùng** vào `currentPresetSettings` để **rollback nếu Cancel**.
4. ⭐ **Nêu cái giá mình đã nhận** — phần phân biệt ứng viên: *"đổi lại, trạng thái cần khôi phục giờ nằm **trên phần cứng**, nên **mọi đường thoát** khỏi chế độ edit đều phải được kiểm soát"* — Cancel, đóng app, rút cáp, chuyển màn hình khác…
5. ⭐⭐ **Nói thẳng lời giải tốt hơn:** *"em xử lý bằng **cờ trong mode** — không sai, nhưng **dễ quên ở chỗ gọi mới**. Team sau này làm bằng **state machine**, và đó là lời giải đúng hơn."*

**Vì sao (5) là câu ăn điểm chứ không phải câu tự hạ mình:** nó chứng minh bạn phân biệt được *"giải pháp chạy được"* và *"giải pháp không thể dùng sai"*. Đó đúng là khoảng cách **mid → senior** mà [plan §📍](../../study-plans/archive/datalogic-plan.md) đã ghi (*"dừng ở sửa xong, chưa tới ngăn tái diễn"*) — ở câu này bạn **đi tới vế thứ hai**.

**Nền kỹ thuật nên nắm:** state machine vs cờ boolean rải rác ([behavioral.md — State](../../../11-design-patterns/behavioral.md)) · vì sao *"số trạng thái tăng thì số tổ hợp cờ tăng theo cấp số nhân"* · rollback/snapshot là mẫu chung: `currentPresetSettings` ở đây ≡ **bản cũ trong OTA A/B** ([BSP-015](bsp.md)) ≡ **transaction rollback**.

**Bẫy:** ① kể thành *"em làm sai rồi sửa"* với giọng xin lỗi — phải kể là *"em đo lại rồi đổi quyết định"* · ② nêu phương án chốt mà **quên cái giá** ⇒ nghe như chưa hiểu hết hệ quả · ③ khen state machine mà **không nói vì sao cờ không đủ** (câu trả lời: cờ đúng ở *thời điểm viết*, hỏng ở *thời điểm người sau thêm đường thoát mới*).
</details>

---

#### RES-018 · 🟡 · concept · ⭐ · 🎤 2026-09-04 · [→ RESUME: "Samsung Display Manager"; nối [bus-protocols](../../../05-drivers-device-tree/bus-protocols.md)]
**"App của em nói chuyện với màn hình bằng gì? Kênh đó có gì đáng lưu ý, và nó liên quan gì tới công việc embedded?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **cầu nối mạnh nhất** từ một project Windows sang JD Embedded Linux. Nếu chỉ nói *"em gọi API Windows"* thì mục này vô giá trị với họ; nói được tầng bus thì nó **thành bằng chứng kinh nghiệm bus**.

**Câu trả lời tốt gồm:**

1. **Kênh là DDC/CI** — chạy ghép trên chính dây **HDMI/DP**, và **về bản chất là một bus I²C** nằm sẵn trong đầu nối. Mỗi thiết lập (độ sáng, tương phản, response time…) là một **mã VCP** — đọc/ghi giống **thanh ghi**.
2. ⭐ **Bốn tính chất khiến nó *giống hệt* bài toán driver nhúng:**

| Tính chất DDC/CI | Đối ứng ở embedded |
|---|---|
| **Chậm** (I²C ~100 kHz) — một lần đọc/ghi tốn hàng chục ms | Không được làm I/O chậm khi đang **giữ khoá** / trong **ISR** |
| **Hay lỗi**, không bảo đảm trả lời | Phải **retry**, và retry phải **có giới hạn + backoff** |
| **Nhiều bên cùng dùng một bus** (driver GPU cũng đọc EDID trên đó) | Tranh chấp bus — phải **nhường**, không độc chiếm |
| Không có thời gian đáp ứng bảo đảm | Không được **block** luồng giao diện / luồng realtime |

3. **Việc bạn thật sự phải xử lý:** không được block UI, **retry từng bước**, và **nhường bus** cho bên khác.
4. **Câu chốt để lái về JD:** *"DDC/CI ở đây và I²C/SPI trên thiết bị nhúng là **cùng một lớp bài toán** — bus chậm, không tin cậy, dùng chung. Chỉ khác cái tên và chỗ em ngồi trong stack."*

**Nền kỹ thuật phải nắm trước khi nói câu này:** [bus-protocols.md](../../../05-drivers-device-tree/bus-protocols.md) — I²C: open-drain + pull-up · START/STOP · ACK/NACK · **clock stretching** · arbitration khi nhiều master. Nói *"giống I²C"* mà không giải thích được **clock stretching** hay **vì sao I²C cần pull-up** thì follow-up sẽ lộ.

**Bẫy:** ① nói *"DDC/CI là I²C"* rồi dừng — phải nêu **hệ quả thiết kế**, đó mới là thứ được chấm · ② thổi phồng thành *"em viết driver I²C"* — bạn ở **phía userspace gọi xuống**, nói đúng vị trí của mình · ③ quên mất rằng đây là chỗ **duy nhất** trong project Windows nối thẳng được sang JD, rồi kể toàn UI ([RES-011](resume.md)).
</details>

---

#### RES-019 · 🟡 · concept · ⭐ · 🎤 2026-09-04 · [→ RESUME: "Samsung Display Manager"; khung kể chung ở [RES-011](resume.md)]
**"App đang mở, người dùng rút dây một màn hình rồi cắm lại — hoặc đổi sang cổng khác. Preset còn gắn đúng màn đó không?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có hiểu **định danh thiết bị** là một bài toán riêng không — hay mặc định *"thiết bị thứ hai luôn là thiết bị thứ hai"*. Đây là câu hỏi **rất hay gặp ở embedded**, chỉ đổi vỏ.

**Sự thật của bản v1 — nói thẳng:** hỗ trợ tối đa **4 màn**, mỗi màn có bộ preset riêng, và khoá bằng **chỉ số** (`AtMonitorIndex`) — tức **vị trí trong danh sách liệt kê**, không phải định danh của chính cái màn hình. Mặc định là màn đang hiển thị app.

⇒ **Giới hạn đã biết:** nếu **thứ tự liệt kê đổi** (rút/cắm, đổi cổng, bật màn theo thứ tự khác), bộ preset có thể **gắn sang màn khác**. Người dùng thấy *"preset của tôi nhảy lung tung"*.

**Vì sao chấp nhận được ở v1 — nêu ràng buộc CỤ THỂ, đừng nói chung chung:** kịch bản mục tiêu là **bàn làm việc cố định**, cắm một lần rồi thôi; và **bán kính ảnh hưởng** chỉ là *preset gắn nhầm*, người dùng chọn lại được — **không mất dữ liệu, không hỏng thiết bị**.

**⭐ Hướng đúng — nói được cái này mới ăn điểm:** khoá theo **định danh ổn định lấy từ EDID** (mã nhà sản xuất + mã sản phẩm + **serial**), rớt về *model + cổng* khi màn không có serial. Mỗi lần danh sách thay đổi thì **so khớp lại theo ID**, không theo vị trí. Chỉ số chỉ còn là *cách hiển thị*, không phải *khoá lưu trữ*.

**⭐⭐ Cầu nối sang embedded — đây là lý do câu này đáng có trong bank:** **cùng một bài toán, ba chỗ khác nhau**

| Chỗ | Biểu hiện |
|---|---|
| `/dev/sda` vs `/dev/sdb` đảo nhau sau reboot | ⇒ dùng `/dev/disk/**by-id**` thay vì `by-path` |
| Thiết bị USB cắm lại ra số khác | ⇒ udev rule khớp theo **serial/VID:PID** |
| Nhiều cảm biến cùng loại trên một bus I²C | ⇒ phân biệt bằng **địa chỉ + node device tree**, không phải thứ tự probe |

**Câu chốt:** *"Bài học em rút ra: **vị trí không phải danh tính**. Cái gì cần sống qua lần cắm lại thì phải khoá bằng thứ thuộc về chính thiết bị."*

**Bẫy:** ① nói *"em xử lý rồi"* trong khi v1 khoá theo chỉ số — [hỏi hai câu là lộ](resume.md) · ② nói giới hạn mà **không nêu hướng đúng** ⇒ nghe như chưa nghĩ tới · ③ quên nêu **bán kính ảnh hưởng** — nó là thứ biến "bug" thành "đánh đổi có ý thức".
</details>

---

#### RES-020 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-04 · [→ RESUME: "restore them across restarts"; khung kể chung ở [RES-011](resume.md)]
**"Preset lưu xuống JSON. (a) Mất điện đúng lúc đang ghi file thì sao? (b) Sau này em đổi định dạng file — file cũ của người dùng thế nào?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⭐ Đây là câu **chuyển thẳng sang embedded** — cấu hình trên flash, OTA, mất điện giữa chừng. Ở desktop mất điện là hiếm; **ở thiết bị nhúng nó là chuyện thường ngày**. Trả lời tốt câu này chứng minh bạn nghĩ được như người làm firmware.

**Sự thật bản v1 — nói thẳng:** **refresh và ghi đè toàn bộ file**, **không có trường `version`**.

### (a) Mất điện lúc đang ghi

**Cơ chế hỏng:** ghi đè tại chỗ có một **cửa sổ** mà file đã bị cắt cụt nhưng chưa ghi xong ⇒ lần mở sau **parse JSON thất bại** ⇒ **mất toàn bộ preset**. Không phải hỏng một dòng — hỏng **cả file**.

**Ràng buộc biện minh cho v1 (cụ thể, không nói chung chung):** app **desktop**, mất điện đột ngột hiếm; và **bán kính ảnh hưởng** là *thiết lập hiển thị của người dùng*, **không phải chức năng thiết bị** — cùng lắm là chỉnh lại.

**⭐ Hướng đúng — ghi nguyên tử bằng đổi tên:**
```
1. ghi toàn bộ nội dung mới vào  presets.json.tmp
2. fsync()  ← ép dữ liệu xuống đĩa THẬT, không nằm trong page cache
3. rename("presets.json.tmp", "presets.json")   ← ATOMIC trên cùng filesystem
```
⇒ Ở mọi thời điểm, `presets.json` **hoặc là bản cũ nguyên vẹn, hoặc là bản mới nguyên vẹn** — không bao giờ là một nửa. *(Bước 2 hay bị bỏ: `rename` nguyên tử về **metadata**, nhưng nếu nội dung file mới còn trong page cache thì mất điện vẫn ra file rỗng.)*

### (b) Đổi định dạng file

**Không có `version` ⇒ hai kiểu hỏng, kiểu thứ hai tệ hơn:** ① parse thất bại ⇒ mất preset · 🔴 ② parse **thành công** nhưng thiếu trường mới ⇒ nhận giá trị mặc định **im lặng**, người dùng tưởng preset còn nguyên mà thực ra đã đổi.

**⭐ Hướng đúng:** đặt `"version": N` **ngay từ bản đầu** *(rẻ nhất lúc chưa có người dùng nào, đắt nhất khi đã có)*. Lúc load: `version` cũ hơn ⇒ chạy **migration** rồi ghi lại; `version` mới hơn ⇒ **từ chối và giữ nguyên file**, đừng cố đọc.

### ⭐⭐ Cầu nối sang embedded

| SDM | Thiết bị nhúng |
|---|---|
| ghi đè `presets.json` | ghi đè partition cấu hình trên flash |
| tmp + `fsync` + `rename` | **A/B partition** — ghi vào bank không chạy rồi mới chuyển ([BSP-015](bsp.md)) |
| `version` + migration | schema cấu hình qua nhiều đời firmware |
| mất điện là **hiếm** | mất điện là **bình thường** — thiết kế phải giả định nó xảy ra |

**Câu chốt:** *"Ở desktop em coi mất điện là ngoại lệ nên ghi đè thẳng. Ở thiết bị nhúng thì nó là **ca mặc định** — và lời giải cùng một hình dạng: **ghi ra chỗ khác, xong xuôi mới chuyển sang**."*

**Bẫy:** ① nói *"em ghi nguyên tử bằng rename"* — **v1 KHÔNG làm thế**, nói vậy là khẳng định sai · ② nêu `rename` mà **quên `fsync`** · ③ chỉ lo (a) mà bỏ (b) — schema evolution mới là thứ cắn về lâu dài · ④ nói *"chưa tối ưu vì gấp"* mà không nêu **ràng buộc + bán kính ảnh hưởng** ([khung kể chung](resume.md)).
</details>

---

#### RES-021 · 🟡 · concept · ⭐ · 🎤 2026-09-04 · [→ RESUME: "Samsung Display Manager"; khung kể chung ở [RES-011](resume.md)]
**"Save và Load có chạy khác thread không? Nếu sau này có, em bảo vệ thế nào — đặt một `std::mutex` làm member của class quản lý file là đủ chưa?"**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Ở bản 1.0, Save và Load **không** chạy khác thread, và em không đặt khoá. Lý do không phải *"chưa gặp lỗi"* mà là **thiết kế**: mọi thao tác preset đi qua luồng giao diện, không có đường nào chạy song song. Đó là giả định em đang dựa vào, và em ghi nó ra.*
>
> *Nếu sau này giả định vỡ — có auto-save chạy nền, hay hai cửa sổ — thì một `std::mutex` làm member **chưa đủ**. Mutex member chỉ tuần tự hoá những lời gọi **đi qua đúng object đó**. Thứ cần bảo vệ là **cái file**, nằm ngoài object: hai object khác nhau có hai mutex khác nhau, và hai process — chạy app hai lần — thì mutex trong bộ nhớ không thấy nhau.*
>
> *Em chọn khoá theo **phạm vi của tài nguyên**: trong một object thì mutex member; toàn process thì một chủ sở hữu duy nhất giữ khoá; liên process thì khoá của hệ điều hành trên file. Và rẻ nhất là làm cho việc ghi **nguyên tử**: ghi ra file tạm rồi `rename` đè lên — người đọc không bao giờ thấy file dở dang, nên nhu cầu khoá giảm hẳn."*

**Nếu bị hỏi tiếp**
- *"Liên hệ với library Linux của em?"* — Cùng bài học: khoá phải cùng phạm vi với tài nguyên. State library nằm trong shared memory dùng chung mọi process, nên khoá là semaphore dùng chung, không phải mutex trong một object ([LNX-046](linux-sysprog.md)).

**Móc nền tảng:** phạm vi của khoá · `flock`/`fcntl` · ghi nguyên tử bằng `rename`.

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn có phân biệt *"chưa gặp lỗi"* với *"đã an toàn"* không · ② ⭐ bạn có hiểu **phạm vi của một mutex** không — đây là chỗ phân biệt thật.

**Sự thật bản v1 — nói thẳng và nói cho ĐÚNG cách:** **chưa có mutex**, và **chưa gặp lỗi**. Nhưng lý do phải nói là **cấu trúc**, không phải may mắn:

> *"Truy cập được **tuần tự hoá bởi thiết kế** — mọi thao tác preset đi qua luồng UI, không có đường nào chạy song song. Nên v1 không cần khoá."*

⚠️ **Khác biệt quan trọng:** *"chưa gặp lỗi nên chắc ổn"* = **yếu**. *"tuần tự hoá bởi thiết kế, và đây là giả định em đang dựa vào"* = **mạnh** — vì bạn nêu được **điều kiện** để nó còn đúng.

**Rủi ro khi giả định đó vỡ** (thêm auto-save nền, đồng bộ cloud, mở hai cửa sổ): reader đọc **file đang ghi dở**, hoặc hai writer **xen kẽ** nhau.

### ⭐ Câu chính: mutex làm member — đủ chưa? **Chưa.**

**Cơ chế:** một `std::mutex` member chỉ tuần tự hoá các truy cập **đi qua đúng object đó**. Nhưng thứ cần bảo vệ **không phải object** — mà là **cái file**, một tài nguyên nằm **ngoài** object và có phạm vi **toàn hệ thống**.

| Ai đụng vào file | Mutex member có chặn được? |
|---|---|
| 2 thread cùng dùng **một** instance | ✅ có |
| 2 **instance** khác nhau của cùng class | ❌ **không** — hai mutex khác nhau |
| 2 **tiến trình** (mở 2 lần app, app + tool cài đặt) | ❌ **không** — mutex nằm trong không gian địa chỉ riêng |

**Ba mức khoá, chọn theo phạm vi của tài nguyên:**

| Phạm vi cần bảo vệ | Công cụ |
|---|---|
| Trong một object | `std::mutex` member |
| Toàn tiến trình (một file, nhiều nơi gọi) | **một chủ sở hữu duy nhất** (singleton) giữ khoá — hoặc khoá tĩnh gắn với *đường dẫn file* |
| **Liên tiến trình** | khoá của HĐH: `flock`/`fcntl` (Linux) · `LockFileEx` (Windows) |

📌 **Và cách rẻ nhất vẫn là (a) của [RES-020](resume.md):** ghi **tmp + `rename`** làm reader **không bao giờ** nhìn thấy file dở dang — **giảm hẳn nhu cầu khoá** thay vì thêm khoá.

**Cầu nối sang embedded:** đúng bài toán *"nhiều bên cùng ghi một tài nguyên"* trên thiết bị — hai tiến trình cùng ghi file cấu hình, hoặc userspace tool và driver cùng đụng một device node. Và [LNX-042](linux-sysprog.md): đặt `pthread_mutex_t` vào shared memory **không tự động** dùng được liên tiến trình — phải bật `PTHREAD_PROCESS_SHARED`. Cùng một bài học: **khoá phải cùng phạm vi với tài nguyên**.

**Bẫy:** ① nói *"em dùng mutex"* trong khi v1 không có · ② *"chưa gặp lỗi nên ổn"* — không nêu được **giả định** đang dựa vào · ③ trả lời *"đủ rồi"* cho câu mutex member — đây chính là chỗ câu hỏi nhắm vào · ④ thêm khoá mà quên rằng **ghi nguyên tử** giải quyết phần lớn vấn đề rẻ hơn.
</details>

---

#### RES-022 · 🟡 · concept · ⭐ · 🏗️ · [→ RESUME: "Joined a short-staffed team … delivered the ``Preset'' feature with settings persistence in one month, in time for the team's 1.0 release"]
**"Em nói một tháng ra được tính năng Preset, và team kịp release 1.0. Lúc em vào thì app đã có gì, em làm chính xác phần nào, và 'kịp 1.0' là nhờ em hay nhờ cả team?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ba thứ cùng lúc — ① mốc thời gian có **thật và nhớ được** không · ② bạn **nhận công đúng phần mình** hay vơ cả release · ③ dưới sức ép một tháng thì bạn **cắt cái gì**, và có **biết mình đang cắt** không. Câu ③ mới là câu ăn điểm; hai câu đầu chỉ để loại.

**Câu trả lời tốt gồm bốn phần, theo thứ tự này:**

1. **Trạng thái lúc vào — mô tả bằng NĂNG LỰC CÒN THIẾU, không bằng lời chê.**
   *"App đã chạy được, điều khiển được màn hình qua DDC/CI, và đã lưu được setting của chính app bằng JSON. Thứ chưa có là **preset** — người dùng chưa lưu lại được một bộ cấu hình để gọi lại, và chưa mang được cấu hình từ màn hình này sang màn hình khác."* — một câu, khách quan, không nhắc "team yếu".
   ⚠️ **Đừng nhận phần lưu JSON là của mình.** Nó **có trước** khi bạn vào. Bạn **dùng lại** nó làm chỗ lưu cho preset — và nói đúng như vậy còn mạnh hơn: *tái sử dụng cơ chế sẵn có thay vì dựng cơ chế thứ hai* là một quyết định thiết kế, không phải thiếu sót.
2. **Phạm vi bạn làm — cụ thể tới mức đếm được**, vì "một tháng" nghe *nhỏ* nếu không có phạm vi kèm theo: Preset cho **tối đa 4 màn hình** (index chọn màn) · **import/export preset** — setup một màn rồi bê nguyên cấu hình sang màn khác · **đồng bộ luồng Save/Load** với UI.
   ⭐ **Import/export là chi tiết đáng nêu nhất**, vì nó là chỗ duy nhất trong project chạm vào một câu hỏi kỹ thuật thật: *cấu hình rời khỏi máy nó sinh ra thì phải mang theo gì để dùng lại được ở nơi khác* — xem follow-up ngay dưới.
3. **Kết quả + RANH GIỚI CÔNG LAO.**
   ✅ *"Phần em nhận xong đúng hạn nên 1.0 không phải cắt tính năng."*
   🔴 *"Nhờ em mà team release được 1.0."* — cùng một sự thật, nhưng vế sau interviewer nghe thấy **người vơ công**.
4. ⭐ **Đánh đổi đã chấp nhận CÓ Ý THỨC** — đây là chỗ biến "làm nhanh" thành "làm nhanh mà biết mình đang đánh đổi gì": ghi đè toàn bộ file, **không có trường version** ([RES-020](resume.md)) · **chưa có khoá** cho luồng Save/Load ([RES-021](resume.md)) · sửa trực tiếp thay vì bản nháp ([RES-017](resume.md)). Mỗi cái nêu **ràng buộc riêng** và **bán kính ảnh hưởng riêng** — xem [KHUNG KỂ CHUNG](resume.md) ở [RES-011](resume.md).

🎯 **Follow-up gần như chắc chắn:** *"Export ở màn hình này, import sang màn hình khác **model khác** thì sao?"* — đây là câu hỏi hay nhất mà mục SDM tạo ra được, vì nó đúng bài toán **portability của cấu hình**: file preset phải **tự mô tả** (nó thuộc model nào, gồm những thuộc tính nào), và bên nhận phải **bỏ qua thuộc tính không hỗ trợ** thay vì áp bừa. Nối thẳng sang [RES-019](resume.md) (định danh màn hình) và [RES-020](resume.md) (chưa có trường version ⇒ file cũ gặp app mới thì xử lý ra sao). Trả lời trung thực theo [KHUNG KỂ CHUNG](resume.md): bản đầu làm tới đâu, và hướng đúng là gì.

📌 **Vì sao mục này đáng để trong resume dù là app Windows:** nó là bằng chứng **duy nhất** trong resume cho *"nhận việc ngoài chuyên môn, trong ràng buộc thời gian, và giao đúng hạn"*. Ba mục kia chứng minh chiều sâu kỹ thuật; mục này chứng minh **độ tin cậy khi giao việc**. Lái tiếp: [RES-011](resume.md).

**Bẫy:** ① biến *"team đang thiếu người"* thành lời chê đồng nghiệp — nêu **một câu như ràng buộc** rồi đi tiếp · ② nhận công cả release (phần 3) · ③ nói "một tháng" mà **không nói phạm vi** ⇒ nghe như một tháng làm được rất ít · ④ để "gấp" thành **cái cớ mặc định** ở mọi câu SDM sau đó · ⑤ **không nhớ mình vào tháng nào, 1.0 ra khi nào** — con số mơ hồ tự huỷ đúng như [RES-012](resume.md) đã dạy: một chi tiết không đứng vững kéo theo nghi ngờ mọi con số còn lại.
</details>

---

#### RES-013 · 🟡 · concept · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "Cut driver startup time from 3–4 seconds back to under 0.5 second"]
**"Con số 3–4 giây xuống dưới 0,5 giây — em đo bằng gì? Đo từ mốc nào tới mốc nào?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** con số trong resume là **đo được** hay **nghe nói**. Đây là câu **lọc** — không bảo vệ được một con số mình tự viết ra thì mọi con số còn lại trong resume đều mất giá trị.

**Câu trả lời tốt gồm:**
1. **Công cụ đo + nó nằm ở đâu** — đo **trong kernel** bằng `jiffies` (hoặc `ktime_get()` cho độ phân giải ns), không phải bấm giờ bằng mắt.
2. **Hai mốc rõ ràng** — từ lúc vào `probe()` của driver nền → hết vòng nạp các driver con → thoát `probe()`. Nói được **vì sao chọn hai mốc đó**: `probe` bao trọn phần khởi tạo + nạp driver con, nên nó là đơn vị đo có nghĩa.
3. **Cùng một phép đo cho trước và sau** — nếu baseline đo kiểu khác thì con số cải thiện vô nghĩa.
4. ⭐ **Biết giới hạn phép đo của mình:** `jiffies` có độ phân giải `1/HZ` (thường 1–10 ms) — thừa cho thang **giây**, nhưng nói ra được điều đó cho thấy bạn hiểu công cụ chứ không chỉ dùng nó.

**Nền kỹ thuật phải nắm:** `jiffies` / `HZ` / `time_before()` · `ktime_get()` vs `jiffies` (độ phân giải) · `probe()` chạy ở đâu trong tiến trình boot ([BSP-006](bsp.md)) · `initcall_debug` + `systemd-analyze` như cách đo **độc lập** để đối chiếu ([BSP-034](bsp.md)).

**Bẫy:** ① nói *"khoảng 3–4 giây"* mà không nêu mốc — interviewer sẽ hỏi *"tính từ đâu?"* và bạn phải nghĩ tại chỗ · ② quên rằng baseline phải đo **cùng cách** · ③ nói con số mà **không có đơn vị đo lặp lại được** (đo một lần vs trung bình N lần boot).
</details>

---

#### RES-014 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "interrupted by other startup work"]
**"'Driver bị chen ngang bởi việc khởi động khác' — em phát hiện ra điều đó bằng cách nào?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là câu **phân biệt mạnh nhất** trong nhóm resume — nó đo bạn **chẩn đoán bằng bằng chứng** hay **đoán rồi thử**. Ứng viên mid-level thường kể *"em thử cái này không được thì thử cái kia"*; ứng viên tốt kể **chuỗi suy luận có số liệu**.

**Câu trả lời tốt gồm — bốn mắt xích, phải đủ cả bốn:**
1. **Triệu chứng có số:** `probe` kéo dài 3–5 s.
2. **Loại trừ nguyên nhân hiển nhiên trước:** đọc `dmesg`, xác nhận đoạn chậm chỉ là **gán biến**, không gọi SoC, không chờ I/O ⇒ **không phải code chậm**.
3. ⭐ **Bằng chứng về tranh chấp CPU:** CPU **100%**, và **hàng loạt tiến trình `SCHED_FIFO` priority RT xuất hiện từ giây thứ 4** — đúng cửa sổ thời gian bị chậm.
4. ⭐ **Nối được hai đầu:** driver nền nạp driver con **tuần tự** bằng `call_usermodehelper()`, và tiến trình đó chạy ở **priority thường (120)** ⇒ **bị preempt** bởi các tiến trình RT. Kết luận rút ra từ số liệu, không phải phỏng đoán.

**Nền kỹ thuật phải nắm:** thang priority trong kernel (**số nhỏ = ưu tiên cao**; RT 0–99, `SCHED_OTHER` map vào 100–139, mặc định **120**) · `SCHED_FIFO`/`SCHED_RR` preempt mọi task `SCHED_OTHER` ([OS-026](os.md)) · `call_usermodehelper()` chạy ở **userspace** với priority thường · `ps -eo pid,cls,rtprio,comm` để nhìn class + rtprio.

**Bẫy:** ① dừng ở *"CPU cao nên chậm"* — chưa nói được **ai** chiếm và **vì sao mình thua** · ② nhầm chiều thang priority · ③ kể giải pháp trước khi kể bằng chứng ⇒ nghe như đoán trúng.
</details>

---

#### RES-015 · 🟠 · design · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "measuring two solutions — loading in parallel and raising scheduling priority"]
**"Em thử hai giải pháp: nạp song song và nâng scheduling priority. Cuối cùng chọn cái nào, vì sao, và rủi ro của phương án bị loại?"**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Trước hết là chẩn đoán: thời gian nạp driver tăng từ dưới nửa giây lên 3–4 giây trên model mới. Log kernel cho thấy đoạn chậm không phải code của driver; cùng lúc đó CPU ở 100% vì một tiến trình **ưu tiên thời gian thực** của hệ thống chạy từ giây thứ tư, trong khi driver nền được nạp **lần lượt từng cái** với ưu tiên thường — nên bị chen ngang.*
>
> *Hai phương án sửa **hai nửa khác nhau**: nạp **song song** bỏ được phần tuần tự nhưng vẫn ưu tiên thường nên vẫn bị chen; **nâng ưu tiên** thì thắng được tranh chấp nhưng vẫn nạp lần lượt. Em đo từng cái: riêng lẻ chỉ đạt khoảng **7 trên 10** lần khởi động. Nên em **kết hợp cả hai**.*
>
> *Trước khi áp, em nhờ team hiệu năng đánh giá, vì song song cộng ưu tiên thời gian thực có thể thành điểm nghẽn cho phần khởi động khác. Nghiệm thu bằng **100 lần khởi động**, tất cả dưới nửa giây.*
>
> *Rủi ro của từng phương án: song song thì thứ tự phụ thuộc giữa driver không còn được bảo đảm — driver cần cái chưa sẵn sàng sẽ phải hoãn; ưu tiên thời gian thực thì task đó không bị chen, nếu nó bận lâu hoặc kẹt sẽ bỏ đói phần còn lại."*

**Nếu bị hỏi tiếp**
- *"Phát hiện bị chen ngang bằng cách nào?"* — Đặt mốc thời gian trong log kernel, rồi xem tải CPU và các tiến trình chạy ở đúng khoảng đó ([RES-014](resume.md)).
- *"Vì sao 100 lần là đủ?"* — Lỗi xuất hiện khoảng 3/10 lần; 100 lần sạch thì xác suất lỗi còn sót là rất nhỏ ([RES-029](resume.md)).

**Móc nền tảng:** scheduling: ưu tiên thường vs thời gian thực ([scheduling](../../../03-operating-system/scheduling.md)) · priority inversion ([OS-007](os.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① có **đo** từng phương án hay chọn theo cảm tính · ② có nhìn ra **tác động lên phần còn lại của hệ thống** không — đây là ranh giới mid → senior · ③ có dám nói *"cả hai đều chưa đủ"* không.

**Câu trả lời tốt gồm:**
1. **Mỗi phương án chữa một nửa khác nhau:** nạp song song bỏ được phần **tuần tự** nhưng vẫn ở priority 120 ⇒ vẫn bị preempt. Nâng priority thắng được tranh chấp nhưng **vẫn nạp tuần tự** ⇒ vẫn dài.
2. ⭐ **Có ngưỡng chấp nhận và dám nói phương án đơn lẻ trượt:** riêng lẻ chỉ đạt ~**7/10** lần boot ⇒ **kết hợp cả hai**.
3. ⭐⭐ **Tự kiểm tác động hệ thống:** *"song song + RT priority có thể trở thành điểm nghẽn cho phần khởi động khác, nên em nhờ team Performance đánh giá trước khi áp"*. Rất ít ứng viên mid-level nghĩ tới bước này.
4. **Nghiệm thu bằng số lần lặp:** 100 chu kỳ boot, không phải "thử vài lần thấy ổn".

**Rủi ro của mỗi phương án — phải nêu được:**

| Phương án | Rủi ro |
|---|---|
| **Nạp song song** | Thứ tự phụ thuộc giữa driver không còn được bảo đảm ⇒ `-EPROBE_DEFER` hoặc race lúc init; tăng đỉnh CPU/RAM cùng lúc |
| **Nâng priority (RT)** | Task RT **không bị preempt** ⇒ nếu nó bận rộn hoặc kẹt sẽ **bỏ đói** phần còn lại; đẩy jitter sang subsystem khác |
| **Cả hai** | Cộng dồn cả hai rủi ro ⇒ **bắt buộc** phải đo lại toàn hệ, không chỉ đo thời gian nạp |

**Bẫy:** ① nói *"chọn cái nhanh hơn"* mà không nêu giá phải trả · ② quên rằng RT priority là **đánh đổi toàn cục**, không phải tối ưu cục bộ · ③ viết resume là *"chose"* trong khi thực tế **dùng cả hai** — sai sự thật và **tự mở một câu hỏi khó**.
</details>

---

#### RES-016 · 🟡 · concept · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME Skills: "AI-assisted Development: Claude Code, internal AI — unit-test generation, code implementation"]
**"Dòng về AI trong resume — nếu interviewer đọc nó theo hướng bất lợi cho em thì họ nghĩ gì? Em nói thêm câu nào để chặn?"**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Họ có thể nghĩ: *phần nào là em làm, bỏ AI đi thì còn lại gì?* Em trả lời bằng cách **phân vai rõ**.*
>
> *AI giúp em ở phần **lặp lại**: sinh unit test từ scenario em viết, theo bộ khung có sẵn của dự án; đổi tên, thêm khai báo hàm, soát cú pháp khi port code. Phần **của em** là chỗ AI không làm thay được: đọc tài liệu chip, trao đổi với người sở hữu module, device tree, bootloader, quyết định thiết kế, và chẩn đoán — AI không đọc hộ em `dmesg` để thấy một tiến trình ưu tiên cao chen vào ở giây thứ tư khi em tối ưu thời gian nạp driver.*
>
> *Với test do AI sinh, em kiểm ba thứ: test có **kiểm kết quả** không hay chỉ gọi hàm cho chạy qua; **làm hỏng code** thì test có đỏ không; và scenario có **ca lỗi** không. Và một thay đổi chạy trên rất nhiều model nên test xanh vẫn phải qua test trên thiết bị.*
>
> *Còn việc nào cần **đúng y như lần trước** cho hàng chục tổ hợp — như porting — em viết **script**, không dùng AI, vì em cần một phép biến đổi xác định và review được từng dòng."*

**Nếu bị hỏi tiếp**
- *"AI sinh một hàm kernel chạy đúng nhưng em không hiểu hết — merge hay chặn?"* — **Chặn**. Code em không giải thích được thì em không bảo trì được và không debug được lúc nó hỏng ngoài hiện trường; em viết lại hoặc đọc tới khi hiểu rồi mới merge.

**Móc nền tảng:** chất lượng test AI sinh ([BLD-043](build-systems.md)) · test double ([BLD-040](build-systems.md)) · porting tool ([RES-009](resume.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có **tự đọc được resume của mình bằng mắt người tuyển** không. Đây là câu đo **nhận thức rủi ro**, không đo kiến thức.

**Suy nghĩ bất lợi họ có — nói thẳng ra được là đã ăn nửa điểm:**
> *"Vậy phần nào là **em** làm? Bỏ AI đi thì còn lại gì?"*

Với công ty đang tuyển **kỹ sư C++ nhúng**, dòng đó có thể đọc thành: bạn là **người điều phối công cụ**, không phải người **giải được vấn đề**. Rủi ro càng lớn khi con số (1 ngày → 1–2 giờ) **ấn tượng hơn** mọi con số khác trong resume.

> 📌 *Dòng AI nằm ở mục **Skills** và không kèm con số* ⟹ dạng hỏi hay gặp nhất là *"code AI sinh ra thì em kiểm soát chất lượng thế nào?"* ([BLD-043](build-systems.md)). Câu chặn và cách nối sang bằng chứng bên dưới áp cho cả dạng đó.

**Câu chặn — một câu, không thanh minh dài:**
> *"AI giúp em đi nhanh ở phần **lặp lại**, nên phần em muốn đầu tư là chỗ **nó không làm thay được**."*

⭐ **Rồi nối ngay vào bằng chứng của chính mình** — đây là phần biến câu phòng thủ thành câu ghi điểm:
> *"— như bài tối ưu thời gian nạp driver: AI không đọc hộ em `dmesg` để thấy tiến trình RT chen vào giây thứ 4."*

**Nguyên tắc chung áp cho mọi dòng resume:** mỗi dòng nên tự hỏi *"câu này mở ra câu hỏi nào?"*. Dòng nói về **công cụ** luôn mở ra câu hỏi *"còn năng lực của bạn ở đâu?"*; dòng nói về **chẩn đoán** thì không.

**Bẫy:** ① nói *"code/test hoàn toàn có thể do AI làm tốt"* — nói với người đang tuyển kỹ sư viết code là **rủi ro thật** · ② thanh minh dài dòng ⇒ nghe như đang chột dạ · ③ chối bỏ dùng AI ⇒ mâu thuẫn với chính resume.
</details>

---

---

## 🎤 Từ phiên B1 (2026-09-12) — bám `RESUME_current.tex`

#### RES-023 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ RESUME: "the kernel driver dispatches through a function-pointer table"]
**"CV nói kernel driver dispatch qua function-pointer table. Vì sao cần tới HAI tầng bảng đó? Ai điền bảng, điền lúc nào?"**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Hai tầng vì **hai lý do khác nhau**.*
>
> *Tầng một là **ranh giới license**: driver nền của nền tảng là GPL, còn driver của từng chip là phần proprietary. Phần proprietary không được dùng trực tiếp các hàm kernel chỉ xuất cho GPL, nên giữa hai bên cần một lớp gọi qua bảng hàm thay vì link thẳng.*
>
> *Tầng hai là **đa hình theo chip**: driver nền không biết tên chip nào. Lúc khởi động, hệ thống xác định board dùng chip gì rồi nạp driver của chip đó; driver chip **tự điền hàm của nó vào bảng** — gọi là tự đăng ký. Từ đó mọi lời gọi từ driver nền đi qua bảng và tự tới đúng chip. Giống bảng hàm ảo của C++, chỉ là viết tay bằng C.*
>
> *Ai điền: chính driver chip, lúc nó được nạp. Driver nền chỉ cung cấp hàm đăng ký và gọi qua bảng."*

**Nếu bị hỏi tiếp**
- *"Bảng do người viết tay thì rủi ro gì?"* — Hai rủi ro của bảng hàm ảo: ô chưa điền (`NULL`) và chèn hàm vào giữa làm lệch ô ([DP-043](design-patterns.md)).
- *"Vì sao kernel không giữ ABI cho module?"* — Cố ý, để kernel được tự do sửa nội bộ; module phải build lại theo từng phiên bản.

**Móc nền tảng:** `EXPORT_SYMBOL` vs `EXPORT_SYMBOL_GPL` · self-registration ([DP-029](design-patterns.md)) · vtable ([CPP-006](cpp.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có phân biệt được **cơ chế** với **lý do tồn tại của cơ chế** không. Ai cũng mô tả được function pointer; ít người nói được vì sao có *hai* tầng.

**Câu trả lời tốt — hai tầng, hai lý do KHÁC nhau:**

| Tầng | Nằm giữa | Tồn tại vì |
|---|---|---|
| 1 | driver nền (**GPL**) ⟷ bridge driver (**private**) | **Ranh giới license.** Symbol `EXPORT_SYMBOL_GPL` không dùng được từ module proprietary ⇒ cần một đường không phải link trực tiếp |
| 2 | bridge driver ⟷ chip driver (**proprietary**) | **Đa hình theo chip** — đúng vai trò của vtable trong C++ |

**Cơ chế đăng ký:** driver nền xác định board dùng chip A → `insmod` chip A → **chip driver tự map hàm của nó vào bảng trong bridge** → từ đó mọi lời gọi tự tới đúng chip. Đây là **self-registration**, không phải bridge đi tìm chip.

⭐ **Câu chốt phải nói ra sớm, đừng chôn ở cuối:** *"Hai tầng tồn tại vì hai lý do khác nhau — một là **ranh giới license**, một là **đa hình**."*

**Nền kỹ thuật phải nắm:** `EXPORT_SYMBOL` vs `EXPORT_SYMBOL_GPL` · lan truyền `TAINT_PROPRIETARY_MODULE` (module GPL **phụ thuộc** module proprietary thì **mất** quyền dùng symbol GPL — nên trò "module đệm GPL re-export" đã bị chặn) · vì sao ngành hay đẩy phần proprietary lên **userspace** (char device/ioctl, UIO/VFIO) thay vì vật lộn trong kernel.

**Bẫy:** ① mô tả cơ chế mà không nêu lý do ⇒ nghe như over-engineering · ② nói "cho nó OOP" rồi dừng — bỏ mất phần hay nhất · ③ khẳng định nhân quả về Nvidia mà không chắc; kể **cơ chế** thì an toàn, kể **nhân quả** thì đừng khẳng định.

**Trình bày:** xem khung *"một nguyên tắc, hai lần áp dụng"* — [log B1 §B](../sessions/2026-09-12--B1--resume.md).
</details>

#### RES-024 · 🟡 · concept · ⭐ · 🎤 2026-09-12 · [→ RESUME: "ambient light sensor"; nối [bus-protocols](../../../05-drivers-device-tree/bus-protocols.md)]
**"Cảm biến ánh sáng của S-Box nối vào bằng bus gì? Ai viết driver cho nó — em chạm tới tầng nào?"**
<details><summary>Khung trả lời</summary>

**Vì sao câu này quan trọng:** JD ghi thẳng *"device drivers for **I2C**, SPI, Ethernet"* mà CV **không nêu tên bus nào**. Đây là **cầu nối duy nhất** giữa việc thật và một trụ JD — bỏ lỡ nó là bỏ lỡ điểm miễn phí.

**Trả lời trung thực có cấu trúc — ba tầng, nói rõ mình ở tầng nào:**
1. **Bus:** I2C.
2. **Ai lo:** library sẵn có (monitor sensor theo tần số riêng, xử lý số liệu, cache).
3. **Mình chạm gì:** request giá trị đã xử lý; phần thuật toán và chính sách độ sáng là của mình.

⚠️ **Đừng gộp nhầm thành "em làm driver I2C".** [Bài học #8](../../study-plans/archive/datalogic-plan.md): interviewer không phân biệt được "gộp nhầm" với "bịa".

⭐ **Cách nâng câu này thành điểm cộng** — nối sang cái mình **thật sự** quyết định: *"Phần I2C thì library lo. Cái em quyết là **chính sách phía trên**: chu kỳ tính target, bước chuyển, và đánh đổi giữa độ trễ với khả năng lọc nhiễu."* ([RES-025](resume.md))
</details>

#### RES-025 · 🟠 · design · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ RESUME: "without flickering when lighting changes quickly"]
**"Chống nháy khi ánh sáng đổi nhanh — em chặn ở đâu, tham số nào quyết định nhạy hay ì? Có cách nào lọc nhiễu mà KHÔNG trả giá bằng độ trễ không?"**
<details><summary>Khung trả lời</summary>

**Cơ chế:** tính **target** → **step** dần tới target (nấc ~200ms). Hai tham số điều khiển hành vi:

| Tham số | Tăng lên thì | Giảm xuống thì |
|---|---|---|
| **Chu kỳ tính target** (vd 4s) | Lọc nhiễu tốt, ì hơn | Nhạy, nhưng dễ dao động khi sensor không ổn định |
| **Bước step** (vd 200ms/nấc) | Chuyển nhanh, dễ thấy giật | Mượt, nhưng lâu tới target |

**Phần T2 — các cách lọc nhiễu KHÔNG phải trả bằng độ trễ:**

| Cách | Ý tưởng | Cái giá |
|---|---|---|
| **Trung bình trượt / low-pass** | Lấy mẫu **nhanh**, làm mượt trên cửa sổ ⇒ vẫn phản ứng nhanh với thay đổi **bền**, bỏ qua xung | Cần tầng đọc+cache riêng; **đa hình theo loại sensor** nếu mỗi loại lấy mẫu khác nhau |
| **Median filter** | Bỏ hẳn xung đơn lẻ (đèn chớp) | Cửa sổ nhỏ vẫn rẻ; hơi tốn RAM |
| **Hysteresis / dead band** | Chỉ đổi khi lệch quá ngưỡng ⇒ không dao động quanh biên | Phải chọn ngưỡng; đổi nhỏ sẽ bị bỏ qua |

⭐ **Điểm được chấm không phải "chọn đúng", mà là NÊU ĐƯỢC LÝ DO BỎ.** Ví dụ mẫu: *"Trung bình trượt tốt hơn về lý thuyết, nhưng nó kéo theo một tầng đọc-và-cache phải đa hình theo từng loại sensor, trong khi lợi ích chỉ để phòng ca hiếm — và lúc đó còn 2 tuần tới release. Em chọn giãn chu kỳ vì thiết bị đặt cố định trong phòng họp, độ sáng vốn ổn định."*

**Bẫy:** nêu con số (4 giây) mà không nêu **vì sao chấp nhận được trong bối cảnh này**. Con số không có bối cảnh nghe như tuỳ tiện.

**Nối:** [B2-redesign-events](../../../11-design-patterns/in-practice/B2-redesign-events.md) — Observer + hysteresis.
</details>

#### RES-026 · 🟠 · concept · ⭐ · 🎤 2026-09-12 · [→ RESUME: "Synchronized brightness across multiple S-Box units over POSIX message queues"]
**"POSIX message queue là IPC trong MỘT máy. Video wall là NHIỀU máy. Dòng CV này khớp thế nào?"**
<details><summary>Khung trả lời</summary>

⚠️ **Đây là câu CV tự mời.** Câu chữ hiện tại đọc tự nhiên thành *"các unit đồng bộ với nhau **bằng** POSIX mq"* — sai, vì mq không qua được ranh giới máy. Người đọc kỹ sẽ nghĩ một trong hai: ứng viên **nhầm khái niệm IPC**, hoặc đang **phóng đại**. Cả hai đều xấu.

**Sự thật (kiến trúc đúng):**
```
[unit MASTER]  lib --mq--> app  --Ethernet-->  app [unit SLAVE] --> lib
                    ^                                ^
              IPC trong 1 máy                  giữa các máy
```
mq là **đường ra một chiều từ library lên app trong cùng máy**; giữa các unit là **Ethernet**.

⭐ **Phải nói CHỦ ĐỘNG, trước khi bị hỏi:**
> *"Trong dòng đó, message queue là đường từ library lên app **trong cùng một máy**; giữa các unit thì đi **Ethernet**. Em viết gọn quá nên dễ gây hiểu nhầm."*

Tự nêu ⇒ **chính xác hoá**. Bị moi ra ⇒ **bắt lỗi**. Cùng một sự thật, hai kết quả.

**Nền phải nắm:** vì sao mq/shm/pipe **không** qua được ranh giới máy (chúng là đối tượng của kernel một máy) · khi nào chọn mq thay vì socket **trong cùng máy** (có priority, có ranh giới thông điệp, không cần xử lý luồng byte) — nối [LNX-005](linux-sysprog.md).
</details>

#### RES-027 · 🟡 · design · 🏗️ · 🎤 2026-09-12 · [→ RESUME: "a single binary serving both standalone and synchronized modes"]
**"Một binary hai mode: nó biết mình ở mode nào bằng cách nào, ai là master? Hai unit cùng bật adaptive thì sao? Master mất điện thì sao?"**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Binary biết mình ở chế độ nào nhờ một **cờ cấu hình** do ứng dụng gửi xuống lúc chạy — chính là mục bật tính năng đồng bộ trên thiết bị. Máy nào bật độ sáng tự động thì thành **master**: nó đo, tính ra mức chuẩn và **đẩy** xuống các máy còn lại. Máy con không tự tính, chỉ áp giá trị nhận được.*
>
> *Vì luồng điều khiển là **đẩy từ master** chứ không phải mỗi máy tự kéo, nên *"hai master"* không phải là một chế độ hỏng thật: ứng dụng ở máy con không đọc hàng đợi của chính nó.*
>
> *Master mất điện: bọn em **chủ động chấp nhận không có failover**. Bức tường là thiết bị cố định, có người vận hành; mất master thì cả bức tường đứng ở mức sáng cuối cùng — sai lệch nhìn thấy được nhưng không nguy hiểm. Làm bầu master tự động thì phải thêm heartbeat và xử lý trường hợp hai bên cùng tưởng mình là master — chi phí không tương xứng với lợi ích ở sản phẩm này."*

**Nếu bị hỏi tiếp**
- *"Muốn có failover thì làm gì?"* — Heartbeat định kỳ từ master; mất heartbeat quá N giây thì máy con quay về tự chỉnh theo cảm biến của mình, hoặc một máy được chỉ định trước lên thay.

**Vẽ:** [whiteboard D5](../../whiteboard.md#d5--s-box-nhiều-box--một-bức-tường-một-mức-sáng) — ba box, một master có cảm biến, một đường mạng, một bức tường.

**Móc nền tảng:** POSIX message queue ([RES-005](resume.md)) · thiết kế chế độ hỏng · split-brain.

</details>

<details><summary>Khung trả lời</summary>

**Phần cơ chế:** cờ cấu hình (vd `multi_sbox`) rẽ nhánh luồng logic; bật adaptive ⇒ unit đó thành master, tính giá trị chuẩn rồi **đẩy lệnh xuống slave**.

⭐ **Chìa khoá để hai câu ca biên không thành lỗ hổng: luồng điều khiển là PUSH từ master, không phải mỗi unit tự kéo.** Vì mq một chiều (lib → app) và app slave không đọc mq của chính nó, "hai master" **không** phải failure mode.

**Ca "master mất điện" — điểm nằm ở CÁCH KỂ, không ở nội dung:**

| ❌ Mô tả | ✅ Quyết định đã cân nhắc |
|---|---|
| *"Slave không bật adaptive nên giữ nguyên độ sáng."* | *"Bọn em **chấp nhận** không có failover. Wall là thiết bị cố định có người vận hành; mất master thì wall đứng ở độ sáng cuối — sai lệch **nhìn thấy được nhưng không nguy hiểm**. Làm bầu master tự động thì phải thêm heartbeat + xử lý split-brain, chi phí không tương xứng."* |

Cùng một sự thật: một bên nghe như **chưa nghĩ tới**, một bên nghe như **đã cân nhắc rồi bỏ**.

**Luật chung:** mọi ca biên chưa xử lý phải kể ở dạng *"đã cân nhắc và chấp nhận"*, **kèm cái giá của phương án bị loại**.
</details>

#### RES-028 · 🟠 · design · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ RESUME: "raising scheduling priority"; nối [scheduling](../../../03-operating-system/scheduling.md)]
**"Team Performance từ chối `SCHED_FIFO` vì sợ đẩy trễ audio/network lúc boot. Giải pháp tốt nhất của em vừa bị chặn — còn phương án nào?"**
<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có **một** khung tư duy hay **nhiều** khung. Đây là câu phân loại.

⭐ **Luật phải thuộc: bài "cái gì đó chậm" LUÔN có hai vế — *làm nhanh hơn* và *làm ít hơn*.** Trả lời chỉ một vế là lộ ra chỉ có một khung.

| Vế | Phương án | Đánh đổi |
|---|---|---|
| Giành CPU | `nice` âm | Yếu — `nice` chỉ đổi **trọng số** CFS, **không** cho quyền ưu tiên. Nhưng an toàn: không starve được ai |
| Giành CPU | `SCHED_RR` prio **thấp**, **chỉ trong cửa sổ insmod** rồi hạ về | Gần trọn lợi ích RT, rủi ro bị **giới hạn theo thời gian** |
| Giành CPU | Ghim CPU riêng (`taskset`/cpuset) | Không đụng priority ⇒ Performance team dễ chấp nhận |
| **Giảm việc** | Rút ngắn `probe()`: tách phần chờ phần cứng ra **thread**, **lazy init** cho resolve symbol | Phức tạp hơn; phải xử lý đường đua lần gọi đầu |
| **Giảm việc** | Build thẳng vào kernel thay vì `insmod` | Mất tính module hoá |
| **Giảm việc** | Gộp nhiều sub-driver thành một | Phá tính private của source; boot team phải biết combination |
| **Đổi thời điểm** | Nạp **sau** đường tới hạn, chỉ cần xong trước lúc hiển thị | Không tranh giành với ai cả — thường là lời giải rẻ nhất |

**Câu ghi điểm về `nice` (hay bị nói mơ hồ):** *"Hạ `nice` không cho em **quyền ưu tiên**, nó chỉ cho em **phần bánh to hơn** — task khác vẫn chen vào. Còn bất kỳ task `SCHED_FIFO` nào cũng preempt **sạch mọi** task `SCHED_OTHER`, bất kể nice. Đó chính là lý do team Performance lo."*

**Và cách gỡ lo ngại đó:** `sched_rt_runtime_us=950000 / sched_rt_period_us=1000000` — kernel **chừa 5% CPU** cho non-RT, nên một task FIFO quay vòng không treo cả máy. Biết con số này là biết **giới hạn rủi ro**, không chỉ biết rủi ro tồn tại.

**Nối:** [OS-029](os.md) · [log B1 §D](../sessions/2026-09-12--B1--resume.md)
</details>

#### RES-029 · 🟡 · concept · ⭐ · 🎤 2026-09-12 · [→ RESUME: "verified it over 100 boot cycles"]
**"Vì sao 100 lần boot là đủ? Con số đó ở đâu ra?"**
<details><summary>Khung trả lời</summary>

**Có câu trả lời định lượng — đây là *quy tắc số ba* (rule of three):**

> Chạy **n** lần độc lập, **0** lần lỗi ⟹ với độ tin cậy **95%**, tỉ lệ lỗi thật **< 3/n**.

| n, 0 lỗi | Kết luận (95%) |
|---|---|
| 30 | < 10% |
| **100** | **< 3%** |
| 300 | < 1% |
| 3000 | < 0,1% |

⭐ **Cách nói — biến "100" từ con số quy ước thành quyết định kỹ thuật:**
> *"100 lần không tuỳ tiện. Với 0 lỗi trên 100 lần, em kết luận được tỉ lệ lỗi dưới 3% ở tin cậy 95%. Nếu team cần dưới 1% thì phải 300 lần — đó là con số em đưa ra để họ quyết định đánh đổi thời gian test."*

Đây đúng thứ JD gọi là *"drive the relation with V&V in a fruitful manner"*: **không xin họ số, mà đưa họ công thức để chọn số.**

⚠️ **Điều kiện áp dụng — nói được là điểm cộng:** các lần chạy phải **độc lập** và **cùng điều kiện**. Bug phụ thuộc nhiệt thì 100 lần boot nguội không nói gì về boot nóng.

**Dùng lại ở:** [BEH-015](behavioral.md) — chốt chung với V&V định nghĩa *"chạy bao nhiêu lần thì tính là tái hiện / là đã fix"*.
</details>

#### RES-030 · 🟡 · concept · 🏗️ · 🎤 2026-09-12 · [→ RESUME: "Samsung Display Manager (Windows Application)"]
**"Đây là dòng duy nhất trong CV không phải Linux/embedded. Nó liên quan gì tới vị trí này?"**
<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có tự đọc được CV bằng mắt người tuyển không — và bạn xử lý một dòng "lạc quẻ" bằng cách thanh minh hay bằng cách bắc cầu.

**Bước 1 — thừa nhận thẳng, một mệnh đề rồi đi tiếp ngay.** *"Không liên quan trực tiếp."* Đừng vòng vo.

**Bước 2 — bắc cầu. Có hai loại cầu, và loại thứ hai MẠNH HƠN:**

| Loại cầu | Ví dụ | Sức nặng |
|---|---|---|
| **Kỹ thuật** | MVVM ↔ HAL + implementation theo phần cứng · Modern C++ dùng chung · cơ chế lưu cấu hình / xử lý file | 🟡 Có lý nhưng nghe hơi gượng — ai cũng bắc được |
| ⭐ **Hành vi** | *"Đội thiếu người, cần release. Em nhận một feature chưa ai làm và giao đúng hạn."* | 🟢 **Mạnh hơn** — JD viết thẳng *"shows initiative, ownership, and commitment"* |

**Bẫy hay mắc:** để cầu hành vi ở vai trò **lời giải thích bối cảnh** thay vì **luận điểm chính**. Nói *"dự án thiếu người nên em hỗ trợ"* rồi chuyển sang nói MVVM là **đặt cái mạnh xuống dưới cái yếu**.

**Bước 3 — chốt ranh giới công lao trước khi bị hỏi** ([RES-022](resume.md)): phần nào đã có sẵn, phần nào là mình.
</details>

#### RES-031 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-12 · [→ JD: "Solicit, generate and manage requirements"]
**"Kể một lần requirement đến tay em ở dạng chưa dùng được. Nó hổng ở đâu, em tìm ra bằng phương pháp gì?"**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Ở tính năng độ sáng tự động của S-Box, requirement ban đầu chỉ có **đường thành công**: bật tính năng, đọc cảm biến, tính giá trị, gửi đi — hết.*
>
> *Em không đợi lúc code mới gặp, mà **kẻ bảng trạng thái nhân sự kiện**: hàng là trạng thái — tắt, đang chuyển, đã ổn định; cột là sự kiện — bật, tắt, người dùng chỉnh tay, khởi động lại, mất cảm biến. Mỗi ô buộc phải có câu trả lời; **ô trống là một câu hỏi gửi lại bên yêu cầu**.*
>
> *Bảng tự sinh ra ba câu hỏi requirement bỏ trống: đang tắt mà bật thì nhảy ngay hay chuyển dần; tắt đi thì giữ mức hiện tại hay quay về mức trước khi bật; và bật rồi khởi động lại thì khôi phục thế nào. Nó còn chỉ ra hai ô mà cách *"gặp đâu xử lý đó"* không tìm ra: đang chuyển dở thì người dùng chỉnh tay, và cảm biến hỏng giữa chừng — ô này sản phẩm chốt là **giữ độ sáng ở mức mặc định**.*
>
> *Em mang bảng đó đi chốt lại với người viết spec, rồi mới code."*

**Nếu bị hỏi tiếp**
- *"Sao không hỏi lúc gặp?"* — Gặp lúc code thì đã muộn: quyết định bị đẩy cho người code đưa ra ngầm, và QA tìm thấy ở cuối.

**Móc nền tảng:** state machine · thiết kế theo chế độ hỏng ([RES-027](resume.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** JD có *"anticipate needs and potential technical issues **in the development phase**"*. Họ muốn biết bạn phát hiện lỗ hổng **trước khi code** hay **sau khi QA báo**.

**Ví dụ mẫu (S-Box):** requirement chỉ có *bật Adaptive → lấy sensor → tính thanh ghi → gửi mq → hết*. Ba lỗ hổng, cả ba đều là **cạnh chuyển trạng thái**:
1. Đang tắt, bật adaptive → nhảy ngay hay **chuyển dần**?
2. Tắt adaptive → **giữ nguyên** hay **quay về** giá trị trước khi bật?
3. Bật adaptive rồi **reboot** → hành vi ra sao?

⭐ **Phần được chấm nặng nhất là PHƯƠNG PHÁP, không phải danh sách.** *"Em gặp lúc code thì thấy"* và *"em rà theo bảng chuyển trạng thái"* là hai tín hiệu hoàn toàn khác nhau dù kết quả giống hệt.

**Phương pháp có tên: bảng chuyển trạng thái (state transition table).** Liệt kê mọi **trạng thái × sự kiện**, buộc mỗi ô có câu trả lời:

| Trạng thái ↓ / Sự kiện → | Bật adaptive | Tắt adaptive | User set tay | Reboot | Mất sensor |
|---|---|---|---|---|---|
| **OFF** | ? ① | – | đặt giá trị | ? ③ | – |
| **ON, đang chuyển** | – | ? ② | ? | ? | ? |
| **ON, đã ổn định** | – | ? ② | ? | ? | ? |

Bảng **tự sinh ra** cả ba lỗ hổng trên, **cộng thêm** hai ô mà cách gặp-thì-thấy không tìm ra: *"đang chuyển dở thì user set tay"* và *"sensor hỏng giữa chừng"*.

**Cách nói:** *"Requirement chỉ mô tả **đường thành công**. Em kẻ bảng trạng thái × sự kiện rồi buộc mỗi ô phải có câu trả lời — ô trống là một câu hỏi gửi lại bên yêu cầu."*

**Nối:** [10/problem-solving.md](../../../10-thinking/problem-solving.md)
</details>

#### RES-032 · 🟡 · concept · ⭐ · 🎤 2026-09-12 · [→ 🇬🇧 phiên 50% tiếng Anh]
🇬🇧 **"Could you introduce yourself and walk me through what you're working on?"** — trả lời bằng tiếng Anh, ≤ 90 giây.
<details><summary>🎙️ Trả lời mẫu</summary>

> *"I'm a software engineer at Samsung — three years building system software for TVs and enterprise display products on ARM SoCs.*
>
> *My work spans the full stack: from the **C++ interface** our shared library exposes, down to the **kernel drivers** talking to the hardware.*
>
> *Three things I'd highlight. First, **one C++ interface across many chipsets**: the library picks its implementation at boot from the board configuration, and the kernel driver dispatches through a function-pointer table, so applications above never change. Second, I **cut driver load time from three-to-four seconds to under half a second** — I measured two options and verified the fix over a hundred boot cycles. Third, I **port drivers to new chip generations** every year, including a kernel migration to 6.12 while keeping the older branch working.*
>
> *I'd like to go deeper on the system side and work in a more structured product process. **Which of those would you like me to go into?**"*

**If they follow up in English**
- *"What exactly is your part in the library?"* — *"The brightness algorithm and its path from the C++ interface down to the driver, plus porting and bring-up on new chips."*
- Tránh: *"I developed"* cho việc đang làm (dùng *"I develop / I've been developing"*); tránh kết bằng câu có/không.

**Móc nền tảng:** cùng ba móc với [RES-001](resume.md) Bản C.

</details>

<details><summary>Khung trả lời</summary>

⚠️ **Hiện tượng đã đo (12/09):** bản Việt 120s có đủ bring-up/porting + ba thành tựu; bản Anh **không có móc nào**. Sang tiếng Anh, ứng viên **thu mình về phần an toàn nhất**. Đây là chuyện bình thường và **cách chữa là kịch bản viết sẵn**, không phải "cải thiện tiếng Anh".

**Bản mẫu 80 giây:**
> *"I'm a software engineer at Samsung, three years building system software for TVs and enterprise display products on ARM SoCs.*
> *My work spans the full stack — from the C++ interface our shared library exposes, down through the HAL, to the kernel driver talking to the SoC.*
> *Three things I'd highlight:*
> *— First, we have **one C++ interface across several chipsets**. The library picks its implementation at runtime from board configuration, and the kernel driver dispatches through a function-pointer table, so applications above never change.*
> *— Second, I **cut driver load time from three-to-four seconds down to under half a second** — I measured two options and verified the fix over a hundred boot cycles.*
> *— Third, I **port drivers across chip generations** every year, including a kernel migration to 6.12 while keeping backward compatibility with the older branch.*
> *What I'm looking for is to go deeper on the kernel side and work with a more formal product process — Yocto, CI, validation. **Which of those would you like me to go into?**"*

**Ba lỗi ngôn ngữ hay mắc:**

| Sai | Đúng | Vì sao |
|---|---|---|
| *"I **developed** System SW"* | *"I **develop**"* / *"I've **been developing**"* | Việc **đang** làm |
| *"the full **system layer**"* | *"the full **stack**"* / *"**all system layers**"* | Số ít nghe như chỉ một tầng |
| *"**Do you want me to** walk through them?"* | *"**Which of those** would you like me to go into?"* | Câu **có/không** đá bóng về chân mình; câu **chọn-một-trong-ba** giới hạn sân |

**Bẫy lớn nhất:** kết bằng câu hỏi có/không. Họ nói *"sure"* rồi bạn **lại phải tự chọn** — đúng thứ vừa thất bại.
</details>

#### RES-033 · 🟠 · concept · ⭐ · 🎤 2026-09-12 · [→ 🇬🇧 giải thích kỹ thuật bằng tiếng Anh]
🇬🇧 **"You mentioned the kernel driver dispatches through a function-pointer table. Why do you need two levels of that? Walk me through it."**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"There are two levels because they solve **two different problems** — one is a **license boundary**, the other is **polymorphism**.*
>
> *The first level sits between the platform's core driver, which is GPL, and the proprietary part. The proprietary side can't use GPL-only kernel symbols directly, so the two talk through a table of function pointers instead of linking to each other.*
>
> *The second level is about chips. The core driver doesn't know which chip is on the board. At boot the system figures out the chip and loads its driver, and that chip-specific driver **registers its functions into the table**. From then on every call goes through the table and lands in the right chip. It's the same idea as a C++ vtable — just written by hand in C.*
>
> *So: one level for the license boundary, one level for polymorphism."*

**If they follow up**
- *"What can go wrong with a hand-written table?"* — *"Two things a compiler would protect you from: an empty slot — so we check for NULL and return 'not supported' — and inserting a function in the middle, which shifts every slot after it. We only append at the end and check a size field at registration."*

**Móc nền tảng:** [RES-023](resume.md) · [DP-043](design-patterns.md).

</details>

<details><summary>Khung trả lời</summary>

**Nội dung: giống [RES-023](resume.md).** Câu này đo thêm một thứ khác — **nội dung có sống sót qua đổi ngôn ngữ không**.

⭐ **Luật cho mọi câu kỹ thuật bằng tiếng Anh: mở bằng MỘT CÂU TÓM rồi mới đi vào chi tiết.**
> *"There are two levels because they solve two different problems — one is a **license boundary**, the other is **polymorphism**."*

Người nghe có khung rồi thì ngữ pháp lệch vài chỗ cũng không cản trở.

**Từ vựng nghiệp vụ hay chệch:**

| Hay nói | Nên nói |
|---|---|
| *"the chip-**depend** driver"* | *"the chip-**specific** driver"* |
| *"they **match** their implementation to the table"* | *"it **registers** its implementation **into** the table"* |
| *"the polymorphism is complete"* | *"…which gives us polymorphism in kernel space"* |
| *"**achieve** the GPL license rule"* | *"…to stay on the right side of the **GPL boundary**"* |

**Với câu "kể thành tựu 30 giây" bằng tiếng Anh:** phải đóng bằng **kết quả**, không phải bằng nỗ lực. *"…in about two weeks"* là nỗ lực; *"…and it shipped with the product"* là kết quả.
</details>

#### RES-034 · 🟠 · concept · ⭐ · 🏗️ · [→ A1 §10](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)
🇬🇧 **"Walk me through the architecture of the library you worked on."** — trả lời bằng tiếng Anh, ≤ 90 giây, vẽ sơ đồ song song.
<details><summary>🎙️ Trả lời mẫu</summary>

**Bản 30 giây — nói trước:**
> *"It's a shared library that controls the display, used across product lines and more than ten chip families. Three boundaries: apps call a **C++ interface**; that interface calls into the library through a **plain C API**; the library talks to the **panel driver in the kernel** through `ioctl`. Inside, **Picture Quality computes** the brightness from the content, and **Panel Control issues the commands**. That's the 30-second version — which part would you like me to open up?"*

**Bản dài — khi được bảo *"tell me more"*, mỗi ranh giới thêm một câu "why":**
> *"First, apps call a C++ interface. That boundary is closed — same team, same toolchain — so C++ is fine there. Below it is a **plain C API**, because the library is loaded by many processes built at different times, and only C gives a **stable binary contract**. Each C entry point is also **the single place where we take the lock**, because the shared state lives in shared memory.*
>
> *Second, behind that C facade there are two parts: Picture Quality is algorithm-heavy and keeps state; Panel Control is simple commands that go straight down through `ioctl`. Picture Quality also writes its final brightness through Panel Control, so there is one path to the hardware.*
>
> *Third, in the kernel, a core driver that knows nothing about the chip **dispatches through a function-pointer table**, and each chip driver **registers itself** into that table when it loads. So the same idea appears twice: polymorphism with virtual functions in user space, and a hand-written vtable in the kernel."*

**If they ask** *"Why not expose the C++ classes directly?"* — *"Two independent reasons: the **ABI** — a C++ class leaks layout and vtable order across the boundary; and **locking** — the C functions are the only door into the library, so no caller can skip the lock."*

**Móc nền tảng:** cùng móc với [RES-035](resume.md) và [DP-040](design-patterns.md).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn nhìn hệ thống theo **ranh giới và quyết định**, hay theo danh sách class · ② bạn có tự nói ra **vì sao** ở mỗi ranh giới không · ③ tiếng Anh kỹ thuật có giữ được mạch khi không còn đọc thuộc.

**Câu trả lời tốt gồm — ba ranh giới, một ý đắt nhất:**
1. App → C++ interface: ranh giới **khép**, C++ được phép.
2. Interface → library: **C API**, vì library được nạp vào nhiều process build lệch thời gian. Đồng thời là **điểm khoá** duy nhất.
3. Sau mặt tiền: Picture Quality (nặng logic, có state) và Panel Control (lệnh đơn → `ioctl`).
4. Ý đắt nhất: **cùng một ý tưởng ở hai tầng** — Bridge bằng `virtual` ở user-space, bảng con trỏ hàm ở kernel.

**Bản 30 giây — nói trước, cùng khung với [RES-035](resume.md):**
> *"It's a shared library that controls the display, used across product lines and more than ten chip families. Three boundaries: apps call a **C++ interface**; that interface calls into the library through a **plain C API**; the library talks to the **panel driver in the kernel** through `ioctl`. Inside, **Picture Quality computes** the brightness from the content, and **Panel Control issues the commands**. That's the 30-second version — which part would you like me to open up?"*

Sơ đồ vẽ song song — bắt đầu từ **app**, không phải từ API C:
```
App ──► C++ interface ──► C API ──┬──► Picture Quality ──┐
                         (lock)   │                      ▼
                                  └───────────────► Panel Control ──ioctl──► panel driver ──fn-ptr table──► chip driver
```

**Bản mẫu ~75 giây** (khi họ nói *"tell me more"*):
> *"It's a display-control shared library used across several product lines and more than ten chipsets. I'd describe it through three boundaries.*
>
> *First, applications don't call the library directly. They call a C++ interface. That boundary is **closed** — same team, same toolchain — so C++ is fine there. The implementation of that interface calls down into the library through a **plain C API**, because the library is loaded by many processes built at different times, and only C gives a stable binary contract. Each C function is also the **single place where we take the inter-process lock**.*
>
> *Second, behind that C facade there are two components, split by how much logic they carry. **Picture Quality** — mainly dimming — is algorithm-heavy, stateful, and runs every frame. **Panel Control** is simple commands like resolution or refresh rate; it just packs arguments and issues an ioctl. Picture Quality also writes its final brightness through Panel Control, so that's the only path to the hardware.*
>
> *Third, in the kernel, a core driver that knows nothing about the chip calls through a **function-pointer table**, and each chip driver registers itself into that table at probe time.*
>
> *What I find most interesting is that **the same idea appears twice**: in user space we separate algorithms from hardware access with a Bridge using virtual functions; in the kernel we do the same with a hand-written vtable. In both places, the chip is fixed at build time and the combination for a model is chosen at runtime."*

**Cụm từ nên dùng thay cho cách nói dịch từng chữ:**

| Tránh | Dùng |
|---|---|
| *"the app calls to the library"* | *"the app calls **into** the library"* / *"calls down into"* |
| *"C is more stable"* | *"C gives a **stable binary contract** / a stable ABI"* |
| *"we lock at the API"* | *"each C entry point is **the single place where we take the lock**"* |
| *"the kernel uses pointer functions"* | *"the kernel driver **dispatches through a function-pointer table**"* |
| *"it is decided when building"* | *"the chip is **fixed at build time**; the combination is **chosen at runtime**"* |

**Nền kỹ thuật phải nắm:** [A1 §3](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) (narrow waist), §4 (PQ/Panel Control), §6.3 (bảng hai tầng). Câu đuổi chắc chắn tới: *"why not expose the C++ interface directly?"* — trả lời bằng **hai lý do độc lập** ([DP-040](design-patterns.md)).

**Bẫy:**
1. Kể **danh sách class** thay vì ranh giới. Người nghe tiếng Anh mất mạch nhanh hơn người nghe tiếng Việt.
2. Sang tiếng Anh thì **co nội dung** lại còn một nửa (lỗi đã đo ở [RES-032](resume.md)). Ba ranh giới phải còn đủ ba.
3. Nói tên sản phẩm, tên symbol nội bộ. Nói *"a display-control shared library"* là đủ, và nói rõ mình đang giữ ranh giới bảo mật là **điểm cộng** ([A1 §🔒](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)).
</details>

#### RES-035 · 🟡 · concept · ⭐ · 🏗️ · [→ A1 §10](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)
**"Kể anh nghe kiến trúc phần library em làm."** — bắt đầu bằng bản **30 giây**; interviewer nói *"kể thêm"* thì mở sang bản **90 giây**.

> *Nguyên tắc: người không làm TV vẫn hiểu · kể ngắn trước, mở dần · mỗi câu thả một móc vào kiến thức nền. Bản tiếng Anh: [RES-034](resume.md).*

<details><summary>🎙️ Trả lời mẫu</summary>

**Bản 30 giây — thuộc nguyên văn:**
> *"Em làm một shared library điều khiển màn hình, dùng chung cho nhiều dòng sản phẩm và hơn mười dòng chip. Em kể theo **ba ranh giới**: app gọi một **C++ interface**; interface đó gọi xuống library qua **một API C**; library ra lệnh xuống **panel driver trong kernel** bằng `ioctl`. Trong library có hai phần: **Picture Quality tính độ sáng** theo nội dung hình, **Panel Control ra lệnh** cho tấm màn. Đó là bản 30 giây — phần nào anh muốn em mở ra thì em mở."*

**Bản 90 giây — khi được bảo *"kể thêm"*, mỗi ranh giới thêm một câu "vì sao":**
> *"Thứ nhất, app không gọi thẳng library mà gọi một C++ interface. Ranh giới đó **khép** — cùng team, cùng build — nên dùng C++ được. Nhưng library được nạp vào **nhiều process build lệch thời gian**, nên giữa interface và library em giữ một **API C thuần**: C là hợp đồng nhị phân ổn định. Mỗi hàm C đó còn là **chỗ duy nhất lấy khoá**, vì state dùng chung giữa các process nằm trong **shared memory**.*
>
> *Thứ hai, trong library: Picture Quality nặng thuật toán và có state; Panel Control chỉ là lệnh đơn đi thẳng xuống `ioctl`. Picture Quality ghi kết quả cuối cũng qua Panel Control, nên chỉ có **một đường** xuống phần cứng. Thuật toán tách khỏi phần ghi phần cứng theo chip, nên thêm chip không phải viết lại thuật toán.*
>
> *Thứ ba, trong kernel: driver chung không biết chip nào; nó gọi qua một **bảng con trỏ hàm** mà driver của chip **tự điền** khi được nạp. Cùng một ý tưởng xuất hiện hai lần: hàm ảo ở user-space, bảng hàm viết tay ở kernel."*

**Móc đã thả — chuẩn bị ở mức cơ bản:** `extern "C"`/ABI ([DP-040](design-patterns.md)) · shared memory + khoá ([LNX-045](linux-sysprog.md)) · library tự khởi tạo khi được nạp ([DP-041](design-patterns.md)) · `ioctl`/`copy_from_user` ([DRV-006](drivers-embedded.md)) · bảng con trỏ hàm ([DP-043](design-patterns.md)) · `virtual` ([CPP-006](cpp.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn nhìn hệ thống theo **ranh giới và lý do**, hay theo danh sách class · ② bạn có **điều khiển được độ dài** không — kể gọn rồi để họ chọn chỗ đào · ③ những chỗ bạn thả móc có **đỡ được** khi họ đào vào không.

**Bản 30 giây — ba ranh giới, hai phần, một câu trao quyền:**
> *"Em làm một shared library điều khiển màn hình, dùng chung cho nhiều dòng sản phẩm và hơn mười dòng chip. Ba ranh giới: app gọi một **C++ interface**; interface gọi xuống library qua **một API C**; library ra lệnh xuống **panel driver trong kernel** bằng `ioctl`. Trong library có hai phần: **Picture Quality tính độ sáng** theo nội dung hình, **Panel Control ra lệnh** cho panel. Đó là bản 30 giây — phần nào anh muốn em mở ra thì em mở."*

**Bản 90 giây — mỗi ranh giới thêm một câu "vì sao":** xem [A1 §10.2](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md). Ba "vì sao" phải có: API C vì library nạp vào **nhiều process build lệch thời gian** · mỗi hàm C là **chỗ duy nhất lấy khoá** vì state nằm trong shared memory · kernel gọi driver chip qua **bảng con trỏ hàm**. Dimming chỉ kể ở mức *"thuật toán + backend điều khiển phần cứng"*.

**Nền kỹ thuật — các móc vừa thả:** bảng [A1 §10.3](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) — `extern "C"`/ABI · shm + semaphore · `__attribute__((constructor))` · `ioctl`/`copy_from_user` · bảng con trỏ hàm · `virtual`. Ôn mỗi chỗ ở **mức hiểu cơ bản**.

**Bẫy:**
1. Mở đầu bằng tên class hoặc tên pattern — người không làm TV mất mạch ngay.
2. Kể luôn bản 90 giây khi chưa ai hỏi — lấy mất quyền chọn của interviewer và đốt thời gian.
3. Tự mở chi tiết backend nhân theo chip × thuật toán — đó là **câu dự phòng**, chỉ nói khi bị hỏi đúng chỗ ([A1 §10.4](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)).
4. Nhắc FRC/TCON hoặc thuật ngữ ngành TV không giải thích.

**Chốt:** bản 30 giây là thứ phải **thuộc nguyên văn**; bản 90 giây là thứ phải **hiểu đủ để nói bằng lời của mình**.
</details>

---

## Cách dùng file này

| Việc | Lệnh / cách làm |
|---|---|
| Quét rộng, luyện nói gọn | `/mock rapid track resume` |
| Đào sâu T2 vào 3–4 câu | `/mock daily track resume` |
| Tự luyện | Bấm giờ **90 giây/câu**, **nói thành tiếng** |

> **Ưu tiên nếu ít thời gian:** RES-002 (interface đa chipset) · RES-004 (kernel migration) · RES-008 (cross-layer debug) · RES-012 (khoảng trống Yocto/I2C). Bốn câu này gần như chắc chắn xuất hiện và có sức nặng lớn nhất.

⬅️ [Bank index](README.md)
