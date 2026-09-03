# 2026-08-29 · `rapid` · track `resume` · 12 câu

> **40/48 = 3.33** · Bước 4 hoàn tất **2026-09-04**.
> Phiên `resume` **lần 2** (lần 1: [18/08 — 3.67](2026-08-18--rapid--resume.md)). **Toàn bộ 12 góc hỏi đều MỚI**; **4 câu tự phát** (`RES-013…016`) đã thêm vào [bank](../bank/resume.md).

## ⚠️ Đọc con số 3.33 cho đúng — ĐỪNG so thẳng với 3.67 của 18/08

| | 18/08 | 29/08 |
|---|---|---|
| Nguồn câu | **đúng 12 câu bank** | **toàn góc mới** + 4 câu chưa từng tồn tại |
| Chất liệu | resume bản cũ, đã ôn | mục **Kernel Driver Load-Time Optimization** chưa ôn lần nào |
| Điểm | 3.67 | **3.33** |
| **Bỏ riêng câu 0 điểm** | — | **40/44 = 3.64** trên 11 câu |

⇒ **Toàn bộ mức sụt nằm ở MỘT câu** (`RES-016`, 0đ). Trên chất liệu chưa ôn, phần còn lại giữ nguyên phong độ.

## Bảng điểm

| # | ID | Góc hỏi | Điểm |
|---|----|---------|------|
| 1 | RES-001 | Chọn 1 trong 4 contribution cho 30 giây đầu | **3** 🔁 lỗi lặp lần 3 |
| 2 | 🆕 RES-013 | Đo 3–4s → 0,5s bằng gì, mốc nào | 4 |
| 3 | 🆕 RES-014 | Phát hiện "bị chen ngang" bằng cách nào | **4** ⭐ câu tốt nhất phiên |
| 4 | 🆕 RES-015 | Hai solution — chọn gì, rủi ro cái bị loại | 4 |
| 5 | RES-012 | "Kể em dùng Yocto thế nào" | **3** ⚠️ xem 🔴 số 1 |
| 6 | RES-011 | Vì sao để app Windows trong resume | 4 |
| 7 | 🆕 RES-016 | Dòng AI — interviewer nghĩ gì bất lợi | **0** 🔴 |
| 8 | RES-002 | Board config sai thì hệ thống làm gì | 4 |
| 9 | RES-004 | "Giữ tương thích ngược 5.10" là gì | 4 |
| 10 | RES-005 | Binary biết mình ở chế độ nào bằng gì | **3** |
| 11 | RES-008 | Bước ĐẦU TIÊN cắt đôi bài toán | **3** |
| 12 | RES-006 | Cơ chế chống nhấp nháy + cái giá | 4 |

---

## 🔴 PHÁT HIỆN SỐ 1 — không nằm ở điểm số: một khẳng định KHÔNG ĐỐI CHIẾU ĐƯỢC

Ở câu 5, sau probe *"board thật hay QEMU?"*, ứng viên trả lời:

> *"board thật, shell điều khiển được thông qua đường Serial cắm vào PC"*

**Đối chiếu [plan §📍](../../study-plans/datalogic-plan.md) — 8/8 bài lab BSP đều ⬜ chưa làm**, và plan ghi rõ hai chặn:

> ⚠️ *"**không có cáp USB–TTL 3.3V thì toàn bộ Thẻ A vô nghĩa**"*
> ⭐ *"**Chưa tick đủ checklist thì đừng bắt đầu bài 031**"*

**Hai khả năng, cả hai đều phải xử lý:**

- **Đã làm thật** ⇒ tracking sai, phải cập nhật. Khi đó phải kể được **chi tiết chỉ người làm mới biết**: giữ nút **S2** lúc cấp nguồn (eMMC tranh boot với thẻ SD), `MACHINE = "beaglebone-yocto"`, dòng `U-Boot SPL` đầu tiên trên console.
- **Chưa làm** ⇒ đây là **thứ nguy hiểm nhất trong cả phiên**, rơi đúng bẫy ① của chính bank `RES-012`:

> **Bẫy:** ① nói *"em có biết Yocto"* khi chỉ đọc qua — **hỏi hai câu là lộ**, và **mất niềm tin cho cả buổi**

Interviewer thật sẽ hỏi câu thứ hai (*"dòng log đầu tiên trên console là gì?"*). Sập ở đó thì **mọi câu 4 điểm phía trên mất giá trị** — họ sẽ tự hỏi còn chỗ nào phóng đại nữa.

📌 **Công bằng: về CẤU TRÚC câu này tiến bộ rõ** so với 18/08 (2đ) — thừa nhận → bắc cầu (cross-compilation, CMake, port driver) → nêu thành phần cụ thể (bitbake/OE-Core/poky). Đó **đúng công thức 3 bước** của bank. Giữ **3 chứ không 4** vì **một khẳng định không kiểm chứng được đáng giá thấp hơn không khẳng định gì**.

**Phiên bản an toàn — bỏ đúng một mệnh đề, giữ nguyên sức mạnh:**
> *"Em chưa dùng Yocto trong sản phẩm thật. Nền em có là cross-compilation và build system đa nền tảng — em đã port driver qua nhiều chipset và migrate kernel 5.10 lên 6.12. Em đang tự học Yocto ở mức tư duy: layer/bbappend để làm gì, sstate giải quyết gì, DEPENDS khác RDEPENDS. **Em có board BeagleBone Black và đang dựng lab để làm thật.**"*

---

## Câu 1 — RES-001 · **3 điểm** · 🔁 **LỖI LẶP LẦN THỨ BA**

**Đề:** *"Resume có 4 mục Selected Contributions. Đầu buổi phỏng vấn, interviewer nói 'kể tôi nghe một thứ bạn tâm đắc nhất'. Bạn chỉ có 30 giây. Chọn mục nào, và câu mở đầu của bạn là gì?"*

<details><summary>Đã trả lời gì · nhận xét · phần còn thiếu</summary>

**Đã trả lời (nguyên văn):** *"Thành tựu mà em tâm đắc nhất là phát triển Adaptive Brightness cho S-Box, nhằm phục vụ một nhu cầu thiết thực là điều chỉnh độ sáng panel dựa trên ánh sáng môi trường, bằng cách áp dụng cơ chế vsync tiêu chuẩn 60Hz để tính toán độ sáng và message queue để luân chuyển giá trị tính toán ra các thiết bị khác, nhờ đó em đã hoàn thành được tính năng này hoạt động trên 1 thiết bị độc lập và nhiều thiết bị cùng lúc, qua đó hoàn thành được tiến độ quan trọng để dự án kịp hoàn thành."*

**Được:** chọn **đúng project** (S-Box, không phải Windows app — bẫy ③ của bank, **không mắc**). Và **tiến bộ thật so với 18/08**: nêu được kết quả *"chạy trên 1 thiết bị độc lập và nhiều thiết bị cùng lúc"* **tự nguyện, không cần probe** — 18/08 phải probe mới ra.

**Vì sao chưa 4 — hai thứ y hệt 18/08:**

| | 18/08 | 29/08 |
|---|---|---|
| Chọn đúng project | ✅ | ✅ |
| **Nêu VẤN ĐỀ trước công nghệ** | ❌ | ❌ **vẫn thiếu** |
| Kết quả tự nguyện (không probe) | ❌ | ✅ **đã sửa** |
| **Có con số** | ❌ | ❌ **vẫn thiếu** |

Giữa câu vẫn là **danh sách công nghệ** (*"vsync 60Hz… message queue…"*), và đóng bằng *"hoàn thành được tiến độ quan trọng"* — **không con số nào**. Cả câu là **một câu văn duy nhất** cho một slot 30 giây.

**Log 18/08 đã kê đơn NGUYÊN VĂN:**
> **Sửa thế nào:** đảo thứ tự. Nói **vấn đề trước, công nghệ sau**:
> *"Nhiều S-Box ghép thành một màn lớn. Mỗi máy tự chỉnh sáng theo cảm biến của nó thì chúng lệch nhau, **nhìn thành từng ô**. Em làm phần đồng bộ độ sáng qua POSIX message queue — kết quả là ghép lại nhìn như một màn duy nhất, và **cùng một binary** chạy được cả chế độ đơn lẻ lẫn đồng bộ."*
> Cùng nội dung, nhưng interviewer nghe được **impact ở giây thứ 10** thay vì giây thứ 90.

**Bank [RES-001](../bank/resume.md), phần vẫn chưa làm được:**
> **3. Vấn đề → giải pháp → con số.**

⚠️ **Đây là lần thứ ba cùng một lỗi:** kê đơn 18/08 → `BEH-001` hỏng y hệt 23/08 (log ghi *"BÀI TẬP 18/08 CHƯA LÀM"*) → nay 29/08 vẫn thiếu vế *vấn đề* và vế *con số*. **Không phải lỗ hổng kiến thức** — nội dung ứng viên nắm thừa. Là **chưa dựng khung và chưa nói to bấm giờ**. Đọc thêm tài liệu **không chữa được**.

**Ôn lại:** [bank RES-001](../bank/resume.md) mục *"Câu trả lời tốt gồm"* phần 3 · [log 18/08 câu 1](2026-08-18--rapid--resume.md).
</details>

---

## Câu 5 — RES-012 · **3 điểm** *(18/08: 2 → nay 3)*

**Đề:** *"Resume ghi `Yocto/BitBake` trong Technical Skills. Interviewer hỏi: 'Kể tôi nghe bạn đã dùng Yocto thế nào.'"*

<details><summary>Đã trả lời gì · nhận xét · phần còn thiếu</summary>

**Đã trả lời:** chưa dùng Yocto trong công việc chính → tự học, áp dụng lên BeagleBone Black → nắm bitbake / openembedded-core / poky → vận dụng nền cross-compilation, CMake/Makefile → **"đã build được minimal image chạy trên BBB, shell đã hoạt động"**. Sau probe: *"board thật, shell qua serial cắm vào PC"*.

**Được — cấu trúc đúng công thức bank, tiến bộ rõ so với 18/08:**
> 1. **Thừa nhận thẳng** ✅ · 2. **Bắc cầu sang thứ gần nhất mình CÓ** ✅ · 3. **Nói cụ thể đang làm gì để lấp** ✅

18/08 hỏng vì *"mở đầu bằng phủ định, không bắc cầu, nói sai hotplug"*. Nay **bắc cầu được** và **nêu đúng ba thành phần** — phần diễn đạt đã chữa.

**Vì sao chưa 4:** khẳng định về BBB **không đối chiếu được với tracking** — xem 🔴 **PHÁT HIỆN SỐ 1** ở đầu log. Một khẳng định không kiểm chứng được **hạ giá trị cả câu trả lời**, vì nó biến một câu an toàn thành một câu có thể sập ở follow-up thứ hai.

**Ôn lại:** [bank RES-012](../bank/resume.md) · [yocto.md](../../../06-build-systems/yocto.md) mức tư duy · 🧪 [BSP-036…038](../bank/bsp.md).
</details>

---

## Câu 7 — RES-016 · **0 điểm** 🔴 · rủi ro plan ĐÃ CẢNH BÁO mà vẫn lọt

**Đề:** *"Resume: 'Applied AI (Claude Code, Internal AI) … cutting authoring time for a code submission from about 1 day to 1–2 hours.' Interviewer đọc dòng này có thể nghĩ ra một điều tiêu cực về bạn. Điều đó là gì, và bạn nói thêm câu nào để chặn nó?"*

<details><summary>Đã trả lời gì · nhận xét · đáp án</summary>

**Đã trả lời:** *"không rõ"*.

**Điều tiêu cực họ nghĩ:**
> *"Vậy phần nào là **em** làm? Bỏ AI đi thì còn lại gì?"*

Với công ty tuyển **kỹ sư C++ nhúng**, dòng đó đọc thành: bạn là **người điều phối công cụ**, không phải người **giải được vấn đề**. Rủi ro càng lớn vì con số (1 ngày → 1–2 giờ) **ấn tượng hơn mọi con số khác** trong resume — nó là thứ mắt interviewer dừng lại.

**Câu chặn (một câu, không thanh minh):**
> *"AI giúp em đi nhanh ở phần **lặp lại**, nên phần em muốn đầu tư là chỗ **nó không làm thay được**."*

**Rồi nối ngay vào bằng chứng của chính mình** — biến câu phòng thủ thành câu ghi điểm:
> *"— như bài tối ưu thời gian nạp driver: AI không đọc hộ em `dmesg` để thấy tiến trình RT chen vào giây thứ 4."*

⚠️ **Rủi ro này KHÔNG mới.** [plan §📍](../../study-plans/datalogic-plan.md) đã ghi từ **24/08**:
> ⚠️ **Một câu nên bỏ khỏi vốn từ phỏng vấn:** *"viết code, ngôn ngữ lập trình, testing hoàn toàn có thể làm tốt bởi AI"* (BEH-005). Nói với người đang tuyển kỹ sư C++ nhúng là **rủi ro thật**.

Cùng một rủi ro, **đổi vỏ** từ *câu nói* sang *dòng resume* — và không nhận ra. ⇒ Bài học: cảnh báo được ghi dưới dạng **một câu cụ thể** thì chỉ chặn được **đúng câu đó**. Phải nâng lên thành **nguyên tắc**: *mỗi dòng resume tự hỏi "câu này mở ra câu hỏi nào?"* — dòng nói về **công cụ** luôn mở ra *"còn năng lực của bạn ở đâu?"*; dòng nói về **chẩn đoán** thì không.

**Ôn lại:** [bank RES-016](../bank/resume.md) · [BEH-005](../bank/behavioral.md) · [log 24/08](2026-08-24--rapid--behavioral.md).
</details>

---

## Câu 10 — RES-005 · **3 điểm**

**Đề:** *"Resume: 'a single binary serving both standalone and synchronized modes.' Lúc chạy, binary đó biết mình đang ở chế độ nào bằng cách nào? Ai quyết định — và quyết định lúc nào?"*

<details><summary>Đã trả lời gì · nhận xét · phần còn thiếu</summary>

**Đã trả lời:** cờ **Multi SBOX** gửi từ application (chính là mục enable tính năng trên thiết bị); app gửi request set cờ ⇒ thiết bị hoạt động như **master** phân phối register value tới các device con.

**Được:** trả lời đủ **ai quyết định** (application) và **lúc nào** (runtime, khi người dùng bật) — hai vế chính của câu hỏi. Nêu được vai master và việc phân phối register value.

**Vì sao chưa 4 — thiếu nửa sau của cơ chế:** chỉ mô tả phía **master**. Một binary chạy **hai vai** thì phải nói được **cả hai**: *thiết bị con biết mình là con bằng cách nào?* Cùng một cờ nhưng giá trị khác? Cấu hình riêng? Nghe địa chỉ mq nào? Không trả lời được vế này thì chưa chứng minh được *"một binary hai chế độ"* thực sự vận hành ra sao — mà đó chính là **điểm hay nhất** của câu chuyện S-Box.

Diễn đạt cũng vòng (*"cờ chính là mục enable tính năng này ở trên thiết bị"*) — ở `rapid`, lòng vòng bị trừ ngang với thiếu ý ([config §4](../config.md)).

**Ôn lại:** [bank RES-005](../bank/resume.md) · [ipc-linux.md](../../../04-linux-system-programming/ipc-linux.md) (POSIX mq).
</details>

---

## Câu 11 — RES-008 · **3 điểm**

**Đề:** *"userspace gọi API set độ sáng, hàm trả về thành công, nhưng màn hình không đổi. Bước ĐẦU TIÊN bạn làm để cắt đôi bài toán — biết lỗi nằm phía user hay phía kernel?"*

<details><summary>Đã trả lời gì · nhận xét · phần còn thiếu</summary>

**Đã trả lời:** ① khoanh vùng thời gian gọi hàm → ② kiểm log userspace (giá trị out of range, sai ioctl command) → ③ nếu log userspace bình thường thì nhiều khả năng lỗi ở kernel → ④ điều tra kernel (giá trị truyền xuống đúng chưa, ioctl, set tới SoC). Kết: lỗi user thì 2 bước, lỗi kernel thì đủ 4 bước.

**Được:** có **quy trình**, không mò; biết dùng log để loại trừ; kết luận có điều kiện rõ ràng.

**Vì sao chưa 4 — trả lời sai DẠNG câu hỏi.** Đề hỏi **bước ĐẦU TIÊN để cắt đôi**; câu trả lời là một **quy trình quét tuần tự từ trên xuống**. Đó là *dò tuần tự*, không phải *chia đôi không gian nghi ngờ*.

**Phép cắt đôi rẻ nhất:** **đọc ngược giá trị ngay tại biên** — `printk` tại đúng chỗ driver ghi thanh ghi, hoặc đọc lại register/sysfs. **Một** phép đo trả lời dứt khoát:
- giá trị **có** xuống tới kernel ⇒ lỗi ở **kernel/SoC**
- **chưa** xuống ⇒ lỗi ở **userspace/đường truyền**

Khác biệt không phải chi tiết kỹ thuật mà là **tư duy**: quét tuần tự tốn O(số tầng); chia đôi tốn O(log). Với hệ nhiều tầng (app → C++ interface → shared lib → ioctl → driver → SoC) thì khác biệt đó rất lớn.

📌 Trùng đúng điểm yếu tự nhận ở [CLAUDE.md §2](../../../CLAUDE.md): *"debug chủ yếu đọc log + so code + suy luận"*. Đọc log là **thu thập bằng chứng**; chia đôi là **thiết kế phép đo**.

**Ôn lại:** [09-debugging/mindset.md](../../../09-debugging/mindset.md) — chia đôi không gian nghi ngờ · [bank RES-008](../bank/resume.md) · [DBG](../bank/debugging.md).
</details>

---

## Năm câu 4 điểm — ghi lại vì đây là vốn mang đi phỏng vấn

<details><summary>RES-013 · RES-014 ⭐ · RES-015 · RES-002 · RES-004 · RES-006 · RES-011</summary>

**Câu 2 · RES-013 — đo bằng gì:** `jiffies` phía kernel, từ vào `probe()` của driver nền → nạp từng driver con → thoát `probe()`; giải thích được **vì sao chọn `probe` làm đơn vị đo** (bao trọn khởi tạo + nạp driver con). *Còn có thể thêm:* độ phân giải `1/HZ`, và baseline đo cùng cách.

**Câu 3 · RES-014 — ⭐ CÂU TỐT NHẤT PHIÊN:** chuỗi bốn mắt xích đầy đủ — probe 3–5 s → `dmesg` xác nhận đoạn chậm **chỉ là gán biến**, không gọi SoC ⇒ loại trừ "code chậm" → **CPU 100%, hàng loạt tiến trình RT priority 94 xuất hiện từ giây thứ 4** → kiểm chéo: driver nền nạp tuần tự bằng `call_usermodehelper` ở **priority 120** ⇒ **bị preempt**. Đây là **chẩn đoán bằng bằng chứng**, có số, có cơ chế. Rất ít ứng viên mid-level kể được như vậy.

**Câu 4 · RES-015:** dùng **cả hai** giải pháp vì mỗi cái riêng lẻ thiếu điểm mạnh của cái kia (song song nhưng vẫn priority 120 · boost priority nhưng vẫn tuần tự); riêng lẻ chỉ **7/10** ⇒ kết hợp; và ⭐ **tự nhờ team Performance đánh giá** vì multithread + RT có thể thành điểm nghẽn hệ thống. Ý cuối là **mức senior**. ⚠️ Ứng viên **tự phát hiện resume ghi sai** (`chose` ⇒ thực tế dùng cả hai) — xem mục *Sửa resume* bên dưới.

**Câu 6 · RES-011 — Windows app:** ba lý do, trong đó ⭐ **bắc cầu MVVM → HAL** rất mạnh: Dependency Inversion, lớp trên không biết hardware / hardware không biết lớp trên, giảm coupling, dễ mock test. Biến một mục "lạc đề" thành bằng chứng cho đúng thứ JD cần. *(Phần trả lời bị gửi lặp nhiều lần do lỗi paste — tính là một câu, không trừ điểm lan man.)*

**Câu 8 · RES-002 — board config sai:** dimming có **default an toàn** (Global Dimming — chiếm hầu hết tỉ trọng sản phẩm); frc/tcon xác định **runtime** từ factory data / FMS key, và data đó **đổi được để test/bring-up bằng data giả**. Ý cuối cho thấy hiểu nhu cầu vận hành thật. *Còn thiếu:* fallback khi FMS key sai cho frc/tcon.

**Câu 9 · RES-004 — tương thích ngược:** **một source build được cho cả 5.10 và 6.12**, chia macro theo kernel version, giữ convention (define trước, logic sau, luôn có prototype). Đúng và gọn.

**Câu 12 · RES-006 — chống nhấp nháy:** **debounce** — chỉ tính target từ sensor **4 giây/lần** để khử nhiễu, rồi **step dần** tới target. Đánh đổi: thích nghi chậm hơn 4 s. ⭐ Và **biện minh bằng bối cảnh triển khai**: phòng hội nghị ánh sáng ổn định, chỉ cần điều chỉnh lớn lúc đầu. Nêu được *cái giá* **và** *vì sao cái giá đó chấp nhận được* — đúng chuẩn T2.
> 📌 Tên gọi chính xác hơn: phần "4 giây/lần" là **debounce / temporal filtering**, phần "step dần" là **slew-rate limiting**. Thêm chữ **hysteresis** (ngưỡng chống dao động quanh điểm chuyển) sẽ tròn bộ từ vựng.
</details>

---

## ✏️ Sửa resume — dòng nói SAI kết quả (ứng viên tự phát hiện ở câu 4)

**Hiện tại:**
> *"**Chose** the fix by measuring two solutions — loading in parallel and raising scheduling priority — and verified it over 100 boot cycles"*

`Chose` hàm ý **chọn một, bỏ một**. Thực tế **dùng cả hai**, vì mỗi cái riêng lẻ chỉ đạt **7/10**.

**Đề xuất:**
> *"Measured each fix separately — parallel loading and raising scheduling priority — and **combined both after neither alone passed reliably**; validated over 100 boot cycles **and cleared with the performance team** before rollout"*

Thêm được ba thứ mà **không tốn thêm dòng**: (1) đúng sự thật · (2) *"neither alone passed reliably"* ⇒ **đo trước khi kết luận** · (3) *"cleared with the performance team"* ⇒ **tự kiểm tác động hệ thống** — chính là thứ plan đã ghi là **mức senior** ở phiên 24/08.

---

## 📌 Ba việc, xếp theo mức nguy hiểm

| | Việc | Vì sao |
|---|---|---|
| **1** 🔴 | **Chốt sự thật về lab BBB** — làm rồi thì cập nhật [plan §📍](../../study-plans/datalogic-plan.md); chưa thì **đổi câu trả lời Yocto ngay** | Sập ở đây làm **mất cả buổi**, không chỉ một câu |
| **2** 🔴 | **Học thuộc câu chặn dòng AI** (một câu + ví dụ insmod) | 0 điểm, và rủi ro này plan cảnh báo từ 24/08 mà **vẫn lọt** |
| **3** 🟠 | **Viết + bấm giờ 45 giây cho `RES-001`** — bắt buộc mở bằng *"nhìn thành từng ô"*, đóng bằng **một con số** | Lỗi **lần thứ ba**; chỉ chữa được bằng **nói to**, không phải đọc thêm |

## 🧾 Ghi chú đồng bộ

- **Bank đã lệch khỏi resume:** `RES-009` bám con số *"giảm 70%"* — **con số đó không còn trong resume** (nay là *"1 day → 1–2 hours"* và *">50%"*). Cảnh báo tương ứng trong [plan §📍](../../study-plans/datalogic-plan.md) cũng đã cũ.
- **Có 3 bản resume:** `RESUME_embedded_linux.tex` · `RESUME_cpp_linux.tex` · 🆕 `RESUME_bosch.tex`. Bank `RES` neo vào **bản embedded_linux** ([tracks.md](../tracks.md)) — cần chốt bản nào là bản chuẩn cho track `resume`.
- **Nợ retention quá hạn:** đợt **24–29/08** (`CPP-019/024/045/020/029`) **chưa chạy**. Đợt **04–09/09** (`CPP-009`, `CPP-032`) mở từ 04/09.

⬅️ [Về sessions](README.md)
