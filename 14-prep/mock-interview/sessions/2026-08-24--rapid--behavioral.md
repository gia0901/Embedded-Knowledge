# Phiên mock — 2026-08-24 · `rapid` · track `behavioral` **(lần 2 — đo lại)**

- **Level:** mid-level · **Số câu:** 12 · **Trần:** T2 *(`rapid` chỉ chạm T1)*
- **Điểm trung bình:** **41/48 = 3.42 / 4** — ⬆️ **+1.00** so với [phiên 1 ngày 23/08](2026-08-23--rapid--behavioral.md) (2.42)
- **Bối cảnh:** phiên **đo lại sau bài tập 5 khung STAR**. Plan ghi việc trước mắt là *tự luyện, không chạy mock* — ứng viên gọi phiên nên phiên chạy ([config §⚖️](../config.md): người dùng nói trong phiên luôn thắng).

> **⚠️ TOÀN BỘ 12 GÓC HỎI ĐỀU MỚI.** [§6 luật ①](../config.md) cấm lặp nguyên văn góc cũ (*"hỏi y hệt là đo trí nhớ về cuộc hội thoại, không đo kiến thức"*). 7 góc lấy từ cột *"Lần sau hỏi"* của [weak-register](../weak-register.md) đã ghi sẵn ở phiên 1; 4 góc lấy từ mục *"Lần sau sẽ hỏi"* trong log phiên 1; 1 góc mới hoàn toàn (counter-offer) ⇒ đã thêm vào bank thành **[BEH-013](../bank/behavioral.md)**.

## Kết quả từng câu (nhìn nhanh)

| # | ID | Góc hỏi (mới) | 23/08 | **24/08** |
|---|----|---------------|-------|-----------|
| 1 | BEH-001 | *"Trong S-Box, phần nào khó nhất — vì sao khó?"* | 2 | **3** |
| 2 | BEH-002 | *"Bug mà lúc đầu em chẩn đoán SAI hướng — nhận ra bằng cách nào?"* | 2 | **4** 🔼 |
| 3 | BEH-006 | *"Một quyết định kỹ thuật của em bị người khác phản đối."* | **1** | **4** 🔼🔼 |
| 4 | BEH-005 | *"Ba tháng chỉ để nâng một kỹ năng — chọn gì, học thế nào?"* | 2 | **3** |
| 5 | BEH-011 | *"Bên anh dùng Yocto nhiều. Em có làm Yocto bao giờ chưa?"* | 2 | **3** |
| 6 | BEH-007 | *"Ba thứ cụ thể em mang lại — và thứ nào đội anh thiếu nhất?"* | 2 | **3** |
| 7 | BEH-003 | *"Hai project cùng deadline một ngày — em CẮT cái gì?"* | 3 | **3** |
| 8 | BEH-004 | *"Tool porting có bao giờ tạo lỗi mà làm tay không mắc không?"* | 3 | **3** |
| 9 | BEH-010 | *"Edge case đó có lọt ra QA/nhà máy/khách hàng không?"* | 3 | **3** |
| 10 | BEH-012 | *"Chỉ duyệt được 30tr, có thêm thưởng dự án + ngân sách đào tạo."* | 3 | **4** 🔼 |
| 11 | BEH-013 🆕 | *"Samsung giữ lại, tăng đúng 35tr, đổi mảng — em ở lại không?"* | — | **4** |
| 12 | BEH-009 | *"Cho tôi BA câu hỏi."* | 2 | **4** 🔼 |

---

## ✅ BA CHỈ SỐ ĐO TRỰC TIẾP — CẢ BA ĐẢO CHIỀU

| Chỉ số | 23/08 | **24/08** |
|---|---|---|
| **Số câu phải probe** | **6/12** | **0/12** |
| **Câu STAR có R tự nguyện** | **0/12** | **5/5** |
| **Con số tự nguyện nói ra** | **0** | **≥6** — *2 tháng · 500ms→3-4s · 7/10 · 100 lần boot · <500ms · 1-2 ngày→1-2 giờ* |

Interviewer **không hỏi một câu probe nào** trong cả phiên. Phiên trước 4/6 probe chỉ để hỏi *"kết quả là gì"*.

**Ba lỗ hổng của phiên 1 đã đóng, không tái diễn lần nào trong 12 câu:**
① thiếu R · ② kể **chính sách** thay vì **câu chuyện** · ③ chọn sai chất liệu (Windows App thay vì Yocto).

> 📌 Đây là xác nhận cho kết luận đã ghi ở [log 23/08](2026-08-23--rapid--behavioral.md):
> *"Có khung sẵn thì đóng gói rất giỏi — 11 câu kia hỏng vì **chưa dựng khung**, không phải vì không biết nói."*
> **Bài tập 5 khung STAR có tác dụng, và tác dụng đo được bằng số.**

---

## 🔎 Chi tiết ôn

### Câu 3 · BEH-006 · **1 → 4** ⭐ bước nhảy lớn nhất phiên

**Đề:** *"Kể về **một quyết định kỹ thuật của bạn bị người khác phản đối**. Chuyện đó diễn ra thế nào?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** lỗi performance khi `insmod` FRC/TCON driver, ngày càng nhiều ở model mới. Thời gian `insmod` từ **500ms → 3-4s**. Nguyên nhân: driver nền nạp 3 driver thật (FRC, TCON, sensor) qua `call_usermodehelper()`, spawn process ở **priority mặc định (120)** nên bị process khác preempt.
- **Đề xuất của bạn:** driver nền nạp **tuần tự** là mấu chốt ⇒ dùng **multithread** nạp đồng loạt cả 3.
- **Đồng nghiệp phản đối:** cho rằng nguyên nhân chính là **priority mặc định** ⇒ nâng lên **`SCHED_FIFO`**.
- **Cách chốt:** cùng ngồi lại, **test từng phương án**. Multithread: **7/10 case đúng giờ**, 3 case chậm do preempt. Phương án kia tương tự.
- **Kết cục:** áp **cả hai** — multithread + `SCHED_FIFO` cho các thread đó — rồi **request team Performance kiểm tra ảnh hưởng toàn hệ thống**. Sau **100 lần boot test**, **không còn delay trên 500ms**.

**✅ Vì sao 4 — bốn thứ vượt mức mid:**

1. ⭐ **Có đúng mắt xích bank gọi là "phần được chấm": ĐƯA DỮ LIỆU.**
   > **Bank [BEH-006](../bank/behavioral.md):** *"…lắng nghe lý do của họ → **đưa dữ liệu/đo đạc** → cùng đi đến quyết định…"*

   Phiên 1 đây là phần trắng hoàn toàn (điểm 1). Nay có 10 lần test, 7/10, và 100 lần boot nghiệm thu.
2. **Hai giả thuyết cạnh tranh trình bày công bằng** — không dựng đồng nghiệp thành người sai.
3. **Kết cục hợp nhất, không thắng–thua** — hoá ra cả hai đều đúng một phần.
4. ⭐ **Nghĩ tới tác động hệ thống trước khi áp:** *"request team Performance kiểm tra xem có ảnh hưởng toàn hệ thống hay không."* Đây là tín hiệu senior rõ nhất phiên — biết `SCHED_FIFO` cho driver chạy sớm lúc boot **có thể bỏ đói thứ khác**, nên không tự ý áp rồi thôi.

**⚠️ Hai chỗ nên siết khi nói thật:**
- *"Chuyện tương tự cho giải pháp của đồng nghiệp"* — **bỏ trống số liệu phương án kia**, trong khi người nghe đang chờ so sánh. Một câu là đủ: *"phương án priority đơn thuần được 6/10, cũng không đủ."*
- **Dài ~150 giây**, vượt trần `rapid`. Nhưng dài **vì có nội dung** (hai giả thuyết, hai kết quả đo, giải pháp hợp nhất, nghiệm thu) — khác hẳn kiểu dài của câu 1. Ở phỏng vấn thật, câu này đáng để dài.

**Chốt:** *"Bất đồng kỹ thuật thì đừng tranh luận — đo. Và khi áp giải pháp động tới scheduler, hỏi người quản lý hiệu năng toàn hệ thống trước."*
**Lần sau sẽ hỏi:** *"nếu team Performance nói `SCHED_FIFO` gây rủi ro toàn hệ thống thì em còn phương án nào?"* — ép nghĩ tiếp khi giải pháp tốt nhất bị chặn.
**Ôn:** [bank BEH-006](../bank/behavioral.md) · [bank OS-007](../bank/os.md) *(priority/scheduling)*
</details>

---

### Câu 2 · BEH-002 · **2 → 4**

**Đề:** *"Kể tôi nghe một bug mà **lúc đầu bạn chẩn đoán sai hướng**. Bạn nhận ra mình sai bằng cách nào?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** bug bring-up TV — tắt rồi bật lại thì màn đen, backlight vẫn sáng. *"Chẩn đoán ban đầu: **video source có vấn đề** → không có nội dung → đen."* Reproduce, capture log, phân tích userspace: lời gọi bật backlight inverter đã xong, bình thường; **kiểm tra video source: có, 1920x1080@60Hz, hoàn toàn bình thường → chẩn đoán sai**. Xoay sang kiểm tra tổng thể quá trình tắt/bật: khi tắt gồm 2 bước (tắt inverter + bật black pattern của FRC); khi bật **không có bước unmute FRC** ⇒ khớp triệu chứng. Nguyên nhân gốc: **thiếu sót trong quá trình porting FRC driver từ base driver**. Bổ sung code unmute, test, verify, submit.

**✅ Vì sao 4 — đúng khung câu hỏi:** **giả thuyết sai → cách bác bỏ nó bằng bằng chứng → xoay hướng → nguyên nhân thật.** Nêu ra giả thuyết sai *rồi nói đã bác bỏ nó bằng gì* chính là thứ câu này đo, và phiên 1 không có.

Và bạn đi xa hơn [phiên 18/08](2026-08-18--rapid--resume.md) **một tầng**: nguyên nhân gốc không dừng ở *"thiếu unmute"* mà truy tới **thiếu sót lúc porting từ base driver** — trả lời được *"vì sao lỗi này TỒN TẠI"*, không chỉ *"lỗi này LÀ GÌ"*. R có mặt, không cần probe.

**Còn thiếu một câu** — thứ log 18/08 đánh dấu 🎯 *"đáng nhớ nhất"*:
> *"Em tìm ra bằng cách **đặt đường tắt và đường bật cạnh nhau** chứ không đọc lần lượt."*

Bạn **đã làm** đúng vậy (*"cần kiểm tra lại tổng thể quá trình tắt/bật"*) nhưng **không gọi tên nó thành một kỹ thuật**. Khác biệt: kể sự cố ⇒ người nghe biết bạn sửa được **một** bug; gọi tên kỹ thuật ⇒ họ tin bạn sửa được **lớp bug đó**.

**Chốt:** *"Triệu chứng ở màn hình, gốc ở chỗ đường bật không đối xứng với đường tắt — em tìm ra bằng cách đặt hai đường cạnh nhau."*
**Lần sau sẽ hỏi:** *"em có bao giờ sửa một bug mà sau đó phát hiện mình sửa nhầm chỗ không?"*
**Ôn:** [bank BEH-002](../bank/behavioral.md) · [bank DBG-016](../bank/debugging.md)
</details>

---

### Câu 1 · BEH-001 · **2 → 3** — nội dung đủ, **đóng gói thừa**

**Đề:** *"Bạn làm S-Box Signage. Trong project đó, **phần nào khó nhất** — và vì sao nó khó?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** adaptive brightness — chưa tồn tại trên model này. Yêu cầu: chạy được trên **1 thiết bị độc lập và nhiều thiết bị cùng lúc**, và *"dù setup theo cách nào đi nữa, **chỉ có 1 binary duy nhất**"*. Khó vì **nguyên nhân chủ quan** (chưa nắm S-Box model, chưa rõ source) và **khách quan** (spec chưa update, thiếu behavior cho edge case). Cách giải: vừa viết code, test edge case, vừa trao đổi với Project Owner về behavior lúc boot / lúc bật-tắt adaptive. *"Khoảng **2 tháng**, hoàn thành, **vượt qua bài test bên phía QA, và software được xuất xưởng**."*

**✅ Được — hai thứ quan trọng, cả hai đều KHÔNG cần probe:**
1. **Chi tiết bị bỏ quên hai phiên liền nay đã xuất hiện:** *"chỉ có 1 binary duy nhất"*. Đây đúng là thứ [log 18/08](2026-08-18--rapid--resume.md) kê đơn cho `RES-001`.
2. **R thật:** *"pass QA và xuất xưởng"* — kết quả mạnh nhất một kỹ sư có thể nói. Kèm mốc **2 tháng**.

**❌ Vì sao chưa 4 — dài (~110 giây) và có đoạn thừa.** Chỗ thừa là khung *"nguyên nhân chủ quan / khách quan"* — hợp cho câu *"một lần làm hỏng việc"* ([BEH-010](../bank/behavioral.md)), **lạc chỗ** ở câu *"phần nào khó nhất"*. Người nghe chờ *"khó vì đâu"*, nhận được *"khó vì hai nhóm nguyên nhân"* — đúng nhưng đọc như báo cáo.

**Bản rút gọn — cùng dữ kiện, ~50 giây:**
> *"Phần khó nhất là adaptive brightness, vì model này chưa từng có. Yêu cầu là nó phải chạy được cả khi thiết bị đứng một mình lẫn khi nhiều thiết bị ghép thành một màn — mà **chỉ được một binary duy nhất**. Cái khó không nằm ở thuật toán mà ở chỗ **spec chưa mô tả hành vi cho các edge case**: bật tính năng lúc boot thì thế nào, đang chạy mà tắt adaptive thì thế nào. Em vừa viết vừa test edge case vừa chốt hành vi với Project Owner. **Hai tháng, pass QA, và software xuất xưởng.**"*

Bỏ khung phân loại, giữ nguyên mọi dữ kiện, **ngắn hơn một nửa**.

**Chốt:** *"Nói cái khó nằm ở ĐÂU, đừng phân loại nguyên nhân thành nhóm."*
**Lần sau sẽ hỏi:** *"nếu làm lại từ đầu, em sẽ làm khác chỗ nào?"*
**Ôn:** [bank BEH-001](../bank/behavioral.md)
</details>

---

### Câu 5 · BEH-011 · **2 → 3** — chọn đúng chất liệu, **vẫn chưa bắc cầu**

**Đề:** *"Bên anh dùng Yocto khá nhiều. **Bạn có làm Yocto bao giờ chưa?**"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"Em không làm Yocto trong công việc tại Samsung. Tuy nhiên, nhận thấy Yocto là một phần quan trọng của lĩnh vực Embedded Linux, nên em quyết định tự học và rèn luyện, áp dụng lên board Linux thông dụng là BeagleBone Black, nắm được các thành phần cơ bản: **bitbake, OpenEmbedded-core, poky**, và đã **thành công build ra một distribution tối giản, boot lên và shell hoạt động được**."*

**✅ Cải thiện rõ so với phiên 1** (khi đó chọn Windows App — chất liệu ít liên quan JD nhất). Cấu trúc chuẩn: thừa nhận **một mệnh đề rồi đi tiếp ngay** → lý do học → bằng chứng cụ thể → **kết quả kiểm chứng được** (*"boot lên và shell hoạt động"*). ~35 giây, gọn.

**❌ Vì sao chưa 4 — thiếu đúng phần được chấm:**
> **Bank [RES-012](../bank/resume.md):**
> **2. Bắc cầu sang thứ gần nhất bạn CÓ.** *Đây là phần được chấm:*
> - **Yocto** ← bạn có **cross-compilation**, **CMake/Makefile**, **build system cho nhiều nền tảng**, **port driver qua nhiều chipset**.

Bạn trả lời như thể Yocto là **một sở thích rời** với công việc. Thực tế nó **nối thẳng** vào việc bạn làm hằng ngày. Thêm **một câu** vào giữa là thành 4:

> *"…Em không dùng Yocto ở Samsung, nhưng nền thì có sẵn: em cross-compile và duy trì build system CMake/Makefile cho nhiều nền tảng chipset, nên vấn đề Yocto giải quyết — **dựng một image tái lập được cho nhiều board** — em hiểu bài toán. Nên em tự dựng thử trên BeagleBone Black…"*

Khác biệt: *"em có học Yocto"* ↔ ***"em đã làm 80% việc mà Yocto tự động hoá, chỉ chưa dùng chính công cụ đó."*** Vế sau là điều interviewer cần nghe.

**Chốt:** *"Thừa nhận một câu, bắc cầu từ nền đã có, rồi mới đưa bằng chứng."*
**Lần sau sẽ hỏi:** ép sang mảng còn trống hẳn — *"còn PCI/USB thì sao?"* — xem có giữ được cấu trúc thừa nhận→bắc cầu khi **không có bằng chứng** để đưa.
**Ôn:** [bank BEH-011](../bank/behavioral.md) · [bank RES-012](../bank/resume.md) · [06-build-systems/yocto.md](../../../06-build-systems/yocto.md)
</details>

---

### Câu 4 · BEH-005 · **2 → 3** — và **một câu nên bỏ**

**Đề:** *"Nếu bạn được **ba tháng chỉ để nâng một kỹ năng** — không phải làm dự án, chỉ học — bạn chọn kỹ năng nào, và học thế nào?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** chọn **System Design**. Lý do: thời đại AI-assisted, tư duy ngày càng quan trọng; *"các kỹ năng chuyên môn thuần khác (viết code, ngôn ngữ lập trình, testing…) **hoàn toàn có thể làm tốt bởi AI**, tuy nhiên bài toán thực tế thì không."* Cách học: coding challenge, design pattern, đề tài thực tế (*thiết kế hệ thống đo cảm biến ngoài trời*), phân tích chọn SoC/MCU, yêu cầu thực tế, đánh đổi. Đã và đang làm: phân tích requirement → constraint → block diagram → sequence diagram → code → test.

**✅ Được:** mạch lạc, có lý do, có **phương pháp cụ thể** với ví dụ embedded, và **đã bắt đầu** chứ không phải dự định.

**❌ Hai điểm:**

**① Không chọn lỗ hổng chính mình đã khai.** Phiên 1 bạn nói điểm yếu là **debug tooling**; [CLAUDE.md §2](../../../CLAUDE.md) ghi vậy; và đang có **7 bài 🧪 lab `DBG-030…036`** chưa làm nhắm đúng đó. Cho tự do chọn thì chọn System Design. Không sai — nhưng interviewer vừa nghe *"debug là điểm yếu"* rồi nghe bạn chọn học thứ khác sẽ ghi: *"biết mình thiếu gì nhưng chưa ưu tiên vá."*

**② ⚠️ Một câu có rủi ro thật.** *"Viết code, ngôn ngữ lập trình, testing hoàn toàn có thể làm tốt bởi AI"* — nói với **người đang tuyển kỹ sư C++ nhúng**, và người phỏng vấn bạn có thể chính là người viết code đó hằng ngày. Nó có thể nghe thành *"phần việc các anh đang tuyển là thứ AI làm được"*.

| | Cách nói |
|---|---|
| ❌ | *"Viết code thì AI làm tốt rồi."* |
| ✅ | *"AI giúp em đi nhanh ở phần lặp lại, nên phần em muốn đầu tư là chỗ nó không làm thay được — chọn kiến trúc, xác định ràng buộc, cân đánh đổi."* |

Cùng ý — nhưng vế sau **nói về mình**, vế trước **nói về giá trị công việc của người nghe**.

**Chốt:** *"Nói mình muốn đầu tư vào đâu, đừng nói phần việc nào đã bị AI thay thế."*
**Lần sau sẽ hỏi:** *"trong 3 tháng đó em đo tiến bộ bằng gì?"* — ép ra tiêu chí nghiệm thu.
**Ôn:** [bank BEH-005](../bank/behavioral.md) · [10-thinking/system-design.md](../../../10-thinking/system-design.md)
</details>

---

### Câu 6 · BEH-007 · **2 → 3**

**Đề:** *"Nêu cho tôi **ba thứ cụ thể** bạn mang lại cho vị trí này. Rồi nói luôn: trong ba thứ đó, thứ nào bạn nghĩ **đội tôi đang thiếu nhất**?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ① nền system software — C++, shared library, HAL driver xuyên tầng user–kernel trên sản phẩm Embedded Linux thật ② bring-up & porting — multi-chipset, porting base driver/library, bring-up hardware qua device tree, đọc board info ở U-Boot, truyền DTS arguments cho kernel ③ tư duy học liên tục & tự động hoá — Samsung Display Manager (ngoài thế mạnh), Chipset Porting Tool **giảm 1-2 ngày → 1-2 giờ**. Xếp hạng: **nền system software** thiếu nhất, *"vì dựa trên JD, modern C++ và Linux/Driver đang được yêu cầu cao nhất"*.

**✅ Cải thiện lớn.** Phiên 1 là *"kiến thức trải dài nhiều tầng"* — tự khai không bằng chứng. Nay **ba trụ, mỗi trụ có bằng chứng**, đúng khung bank, và **con số tự nguyện**.

**❌ Vì sao chưa 4 — lập luận xếp hạng vòng tròn.** *"JD nhấn mạnh X"* ⇒ *"đội thiếu X"* không chắc: JD nhấn thứ **quan trọng**, không nhất thiết thứ **khó tuyển**. Bản sắc hơn tách hai điều đó:

> *"JD nhấn Yocto/BSP nên chắc bên anh đã có người mạnh mảng đó. Thứ em nghĩ **khó tuyển** hơn là người **đi được xuyên tầng** — cầm được cả C++ API phía trên lẫn driver phía dưới, và debug được lỗi nằm ở ranh giới. Đó là chỗ em nghĩ mình bù được."*

**⚠️ Chi tiết cần thống nhất — con số trôi qua ba phiên bản:**

| Nguồn | Con số |
|---|---|
| [RESUME.tex](../../../RESUME.tex) | *"giảm >50%"* |
| Phiên 23/08 | *"1 ngày → 10 phút + 1 tiếng review"* |
| Phiên 24/08 | *"1-2 ngày → 1-2 giờ"* |

Cùng bậc nên không ai bắt bẻ được — nhưng **chốt một phiên bản và dùng mãi**, vì interviewer có ghi chép.

**Chốt:** *"Tách 'JD nhấn mạnh gì' khỏi 'đội khó tuyển gì' — chúng không giống nhau."*
**Lần sau sẽ hỏi:** *"nếu đội đã có người mạnh cả ba thứ đó thì em còn mang lại gì?"*
**Ôn:** [bank BEH-007](../bank/behavioral.md) · [RESUME.tex](../../../RESUME.tex)
</details>

---

### Câu 7 · BEH-003 · **3** — trả lời "làm cái nào trước", đề hỏi "**cắt** cái gì"

**Đề:** *"Giả sử **hai project cùng deadline đúng một ngày**, làm không kịp cả hai. Bạn **cắt cái gì**?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** hai tiêu chí có thứ tự — ① **sản phẩm nào quan trọng hơn** (*"chiếm tỉ trọng cao trong mục tiêu bán hàng thì buộc phải hoàn thành trước"*) ② nếu ngang nhau thì **ưu tiên cái có tỉ lệ hoàn thành cao hơn**, *"tránh tình huống làm cái khó hơn rồi rủi ro trễ cả 2"*. Sau đó **trình bày với project leader**, để leader xem xét; nếu không đồng ý thì trao đổi thêm về *"các tiêu chí khác mà em chưa nắm được"*.

**✅ Được — ba thứ:** hai tiêu chí **có thứ tự** và **có lý do đúng** cho tiêu chí hai; **đưa lên leader** thay vì tự quyết (ưu tiên xuyên project là quyết định cấp quản lý — nhận ra đúng ranh giới); và **tự nêu giới hạn thông tin của mình**. Trung thực đúng chỗ.

**❌ Vì sao chưa 4:** bạn trả lời **sequencing** (làm cái nào trước), không phải **cắt**. Còn đòn bẩy thứ ba mà kỹ sư có kinh nghiệm luôn nêu — **cắt PHẠM VI thay vì cắt PROJECT**:

> *"Trước khi bỏ hẳn một cái, em xem trong mỗi project phần nào là **blocking dây chuyền** và phần nào **ship sau được**. Thường không phải chọn 1 trong 2 project, mà là ship cả hai ở mức **tối thiểu chạy được** rồi bù phần còn lại ở bản sau."*

Đây đúng là cách hàng thật ra khỏi nhà máy — và bạn **đã sống trong quy trình đó**: bring-up hướng mass-production luôn có khái niệm *"đủ để chạy dây chuyền"*.

**Chốt:** *"Trước khi cắt project, cắt phạm vi. Hỏi phần nào chặn dây chuyền, phần nào ship sau được."*
**Lần sau sẽ hỏi:** *"leader bảo làm cả hai, không cắt gì — em làm sao?"*
**Ôn:** [bank BEH-003](../bank/behavioral.md)
</details>

---

### Câu 8 · BEH-004 · **3** · và Câu 9 · BEH-010 · **3** — cùng một khoảng cách

> ⭐ **Hai câu này lộ ra TRẦN MỚI của phiên, và nó là khoảng cách mid → senior.** Chỉ nhìn thấy được sau khi ba lỗi cơ bản của phiên 1 đã đóng.

**Câu 8 — Đề:** *"Về cái tool porting bằng Python. **Nó có bao giờ tạo ra lỗi mà làm tay không mắc phải không?**"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"Có. Các lỗi liên quan đến trình bày: if/else thụt dòng không đúng, giãn dòng sai, **ghi chồng đè lên các source code cũ**… do phương pháp xử lý regex, cách xác định block code chưa hoàn chỉnh. Việc test, build và review liên tục trên nhiều chipset khác nhau là cần thiết để hoàn thiện app."*

**✅ Được:** trung thực và cụ thể. Cái thứ ba — **ghi đè source cũ** — là **lớp lỗi mà làm tay không bao giờ gây ra**, đúng trọng tâm câu hỏi. Nguyên nhân kỹ thuật nêu đúng: regex không xác định được block code.

**❌ Vì sao chưa 4:** cách xử lý bạn nêu là **PHÁT HIỆN**, không phải **PHÒNG NGỪA**. Với một tool ghi đè source, câu trả lời của người đã bị bỏng là:

| Cơ chế | Ngăn được gì |
|---|---|
| **Dry-run — in diff trước khi ghi** | Người xem trước khi hỏng |
| **Bắt buộc cây git sạch; sửa xong xem `git diff`** | Luôn hoàn tác được |
| **Idempotent — chạy hai lần cho cùng kết quả** | Chạy lặp không nhân đôi thay đổi |
| **Từ chối chạy nếu file đích đã bị sửa tay** | Đúng ca "ghi đè source cũ" |

**Chốt:** *"Tool ghi đè source thì phải dry-run + diff, không phải test kỹ hơn."*
**Lần sau sẽ hỏi:** *"nếu tool đó được cả team dùng chứ không riêng em, em đổi gì trong thiết kế?"*
**Ôn:** [bank BEH-004](../bank/behavioral.md)
</details>

**Câu 9 — Đề:** *"Loạt edge case của adaptive brightness mà bạn không lường trước — **có cái nào lọt ra ngoài không**? Nếu có thì xử lý ra sao?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"Chưa từng. Ở Samsung quy trình chặt: **development → SW QA → release → Factory test → Ship**. Gần như tuyệt đối các lỗi nằm ở SW QA và quay lại development nhanh chóng."* Nếu có lọt: ① xem xét bằng chứng từ nhà máy/khách hàng + tự reproduce để có debug log ② xác nhận lỗi phía mình, rơi vào edge case nào, **sửa gấp, report lên cấp trên** ③ test pass rồi **request hotfix release**.

**✅ Được:** trung thực, và nêu đúng **chuỗi cổng chặn thật**. Biết lỗi bị bắt ở đâu trong một tổ chức mass-production là kiến thức thật, không đọc sách ra.

**❌ Vì sao chưa 4:** quy trình bạn mô tả **bắt đầu bằng debug**. Với lỗi đã ra khỏi nhà máy, việc đầu tiên **không phải debug** mà là **chặn máu**:

1. **Bao nhiêu thiết bị dính?** (lô nào, firmware version nào) — xác định bán kính ảnh hưởng
2. **Dừng shipment / giữ lô chưa xuất**
3. **Báo ngay lên trên** — *trước* khi tìm ra nguyên nhân, không phải sau
4. Rồi mới reproduce & fix
5. Hotfix release
6. ⭐ **Post-mortem: vì sao QA không bắt được, thêm regression test cho ca đó**

Bạn dừng ở **⑤**. Bước ⑥ mới là bước ngăn tái diễn.

📌 **Công bằng mà nói — bạn CÓ tư duy này, chỉ chưa dùng đều.** Ngay câu 3 bạn nói *"phải giải quyết dứt điểm để **không tái diễn ở các model năm sau**"* và chủ động nhờ team Performance đánh giá tác động hệ thống — đó chính xác là tư duy phòng ngừa. **Nó có mặt khi bạn kể chuyện ĐÃ xảy ra, và vắng mặt khi bạn phải thiết kế quy trình cho tình huống GIẢ ĐỊNH.**

**Chốt:** *"Lỗi ra tới hiện trường thì thứ tự là chặn máu → báo → sửa → ngăn tái diễn. Debug không phải bước đầu tiên."*
**Lần sau sẽ hỏi:** *"khách hàng báo lỗi mà em reproduce mãi không được — làm gì?"*
**Ôn:** [bank BEH-010](../bank/behavioral.md) · [bank DBG-014](../bank/debugging.md) *(crash ngoài field)*
</details>

---

### Câu 10 · BEH-012 · **3 → 4** · Câu 11 · BEH-013 🆕 · **4** · Câu 12 · BEH-009 · **2 → 4**

<details><summary>Ba câu đạt 4 — bạn trả lời gì và vì sao đủ điểm</summary>

**Câu 10 — Đề:** *"Bọn anh chỉ duyệt được **30 triệu**, nhưng có thêm **thưởng theo dự án** và **ngân sách đào tạo**. Bạn nghĩ sao?"*
**Bạn trả lời:** *"Thưởng theo dự án là con số cố định hay linh động? Nếu tổng thu nhập chia ra hằng tháng cân bằng ở mức 35 triệu thì em chấp nhận."*

**✅ Ba nước đi đúng thứ tự trong ~15 giây:** ① **hỏi lại trước khi trả lời** (thứ thiếu hẳn phiên 1) → ② **quy về đơn vị so sánh được** (tổng thu nhập / tháng) → ③ **giữ số mà không cứng nhắc**. Đây là cách đàm phán của người biết việc.
*Sót nhỏ:* bỏ qua **ngân sách đào tạo** — nó cũng là tiền, và với mảng bạn đang thiếu (Yocto, debug tooling) thì đáng giá thật.

---

**Câu 11 — Đề:** *"Giả sử bạn nộp đơn nghỉ, và Samsung giữ lại — tăng lương lên đúng **35 triệu**, cho bạn đổi sang mảng bạn muốn. **Bạn có ở lại không?**"*
**Bạn trả lời:** *"Em không. Quyết định không chỉ nằm ở con số lương, mà là mong muốn tiếp tục mở rộng kiến thức trong lĩnh vực của mình, thử sức những thử thách mới với vai trò cao hơn."*

**✅ Đây là PHÉP THỬ, và bạn qua.** Interviewer hỏi để xem lý do *"muốn thử thách"* ở [BEH-008](../bank/behavioral.md) có thật không hay chỉ là vỏ bọc của chuyện lương. Trả lời **ngay, không do dự** (do dự ở đây đắt hơn mọi câu chữ sau đó), và **khớp** với động cơ đã nêu.
*Nếu muốn sắc hơn:* nêu **một thứ cụ thể** nơi mới có mà chỗ cũ không có — *"ở chỗ cũ em làm một mảng hẹp trong một sản phẩm; ở đây em muốn chạm cả chuỗi từ BSP tới ứng dụng."*
⇒ Câu này chưa có trong bank ⇒ **đã thêm thành [BEH-013](../bank/behavioral.md)**.

---

**Câu 12 — Đề:** *"Bạn có câu hỏi gì cho tôi không? **Cho tôi ba câu.**"*
**Bạn hỏi:** ① SoC nào, platform gì — Yocto, Buildroot hay full distribution? ② công việc nghiêng userspace (C++, HAL) hay kernel space (BSP, driver)? ③ cách team debug lỗi cross-layer — có debug hardware không, có dùng gdb/valgrind trên thiết bị thật không, hay chủ yếu log/tracing/coredump?

**✅ Ba câu, cả ba đều kỹ thuật, trúng cả ba mục bank liệt kê.** Câu ③ là hay nhất — cho thấy bạn hiểu debug trên embedded là **bài toán hạ tầng**, không phải kỹ năng cá nhân, và ngầm nói bạn biết chạy `valgrind` trên target là chuyện **không đương nhiên**. **Vừa hỏi vừa chứng minh.**
*Còn thiếu:* chưa hỏi về **CI / build pipeline / cách test trên phần cứng thật**, và chưa hỏi **kỳ vọng sau 6 tháng**.
</details>

---

## Tổng kết

### Điểm mạnh

1. **0/12 câu cần probe** (phiên 1: 6/12). Chỉ số cứng nhất, và nó nói bài luyện có tác dụng thật.
2. **Câu 3 đạt mức senior** — bất đồng kỹ thuật giải quyết bằng **số liệu**, kết cục hợp nhất, và **tự kiểm tra tác động hệ thống** trước khi áp `SCHED_FIFO`. Rất ít ứng viên mid-level nghĩ tới bước cuối.
3. **Đàm phán lương gọn và đúng bài** (câu 10, 11) — hỏi lại → quy về đơn vị so sánh → giữ số; từ chối counter-offer dứt khoát.

### Lỗ hổng ưu tiên — **TRẦN MỚI, không phải lỗi cũ**

| # | Lỗ hổng | Loại | Việc phải làm |
|---|---|---|---|
| **1** 🟠 | **Dừng ở "sửa xong", chưa tới "ngăn tái diễn"** (câu 8: phát hiện thay vì phòng ngừa · câu 9: hotfix, thiếu containment + post-mortem) | **Tư duy — mid→senior** | Với mọi câu quy trình, thêm bước cuối: *"và để nó không lặp lại, em…"* |
| **2** 🟡 | **Chưa bắc cầu ở BEH-011** — có bằng chứng Yocto nhưng không nối vào cross-compile / CMake / porting đa chipset làm hằng ngày | **Đóng gói** (đã nhẹ hơn nhiều) | Thêm **một câu**: *"nền thì em có sẵn…"* |
| **3** 🟡 | **Kiểm soát độ dài** — câu 1 (~110 s) và câu 3 (~150 s) vượt trần. Câu 3 dài **vì có nội dung**; câu 1 dài **vì có đoạn thừa** | **Đóng gói** | Bấm giờ lại đúng hai câu đó; cắt khung phân loại thừa ở câu 1 |

> ⚠️ **Một câu nên bỏ khỏi vốn từ phỏng vấn:** *"viết code, ngôn ngữ lập trình, testing hoàn toàn có thể làm tốt bởi AI"* (câu 4). Nói với người đang tuyển kỹ sư C++ nhúng là rủi ro thật. Đổi thành *"AI giúp em đi nhanh ở phần lặp lại, nên phần em muốn đầu tư là chỗ nó không làm thay được."*

### Vị trí trong chuỗi phiên

| Phiên | Điểm |
|---|---|
| 17/08 `rapid` drivers-dt | **3.92** |
| 18/08 `rapid` resume | 3.67 |
| **24/08 `rapid` behavioral (lần 2)** | **3.42** ⬆️ *(từ 2.42)* |
| 19/08 `rapid` bsp | 3.33 |
| 21/08 `daily` cpp-system | 3.0 |
| ~~23/08 `rapid` behavioral (lần 1)~~ | ~~2.42~~ |

`BEH` từ **domain yếu nhất** lên **thứ ba trong sáu phiên gần nhất**, trong **một ngày**.

---

## Cập nhật đã thực hiện (Bước 4)

- ✅ Log này
- ✅ `bank/behavioral.md`: **12 → 13 câu** — thêm **`BEH-013`** (counter-offer, 🟠⭐)
- ✅ `weak-register.md`: 7 dòng `BEH` cập nhật điểm lần 2 (đều **1/2 lần ≥3**), ghi **góc đã dùng** + **góc mới** cho lần sau
- ❌ `coding-arena/reviewed/` — không áp dụng (phiên behavioral)
- ✅ `study-plans/datalogic-plan.md` §📍
