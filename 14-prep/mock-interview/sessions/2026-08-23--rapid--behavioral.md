# Phiên mock — 2026-08-23 · `rapid` · track `behavioral`

- **Level:** mid-level · **Số câu:** 12 · **Trần:** T2 *(thực tế `rapid` chỉ chạm T1)*
- **Điểm trung bình:** **29/48 = 2.42 / 4** — 🔴 **thấp nhất trong mọi phiên đã chạy**
- **Bối cảnh:** `BEH` là **domain trắng cuối cùng** (0% phủ tới hôm nay), và là domain **100% bị hỏi**, thường ngay 10 phút đầu.

> **⚠️ Hai điều chỉnh của phiên — ghi lại để lần sau không tưởng là lỗi:**
> **①** Bank `BEH` lúc bắt đầu chỉ có **9 câu**, `rapid` cần **12** ⇒ interviewer tự phát **3 câu mới**, đã thêm vào bank theo [config §1 Bước 4](../config.md): **`BEH-010`** (làm hỏng việc) · **`BEH-011`** (học nhanh) · **`BEH-012`** (lương kỳ vọng). Cả ba đều là câu chắc chắn bị hỏi mà bank trống hẳn.
> **②** `rapid` áp lên behavioral **không** nghĩa là hỏi nông — nghĩa là **bấm giờ 60–90 giây/câu**. Theo [thang riêng cho `rapid`](../config.md#4), *"lan man cũng bị trừ"*. Đây là **thuốc đúng bệnh** cho lỗi đóng gói đã đo ngày 18/08.

## Kết quả từng câu (nhìn nhanh)

| # | ID | Câu | Điểm | Phải probe? |
|---|----|-----|------|------------|
| 1 | BEH-001 | Thành tựu tự hào nhất | **2** | ✅ (hỏi kết quả) |
| 2 | BEH-002 | Bug khó nhất | **2** | ✅ (hỏi nguyên nhân) |
| 3 | BEH-005 | Điểm yếu | **2** | ✅ (câu đầu là phi-điểm-yếu) |
| 4 | BEH-004 | Cải tiến quy trình | **3** | ✅ (hỏi con số) |
| 5 | BEH-006 | Bất đồng quan điểm | **1** 🔴 | ✅ → *"chưa rõ"* |
| 6 | BEH-003 | Làm việc dưới áp lực | **3** | — |
| 7 | BEH-010 🆕 | Một lần làm hỏng việc | **3** | — |
| 8 | BEH-011 🆕 | Học thứ mới trong thời gian ngắn | **2** | — |
| 9 | BEH-008 | Vì sao rời công ty cũ | **4** ⭐ | — |
| 10 | BEH-012 🆕 | Lương kỳ vọng | **3** | ✅ (đẩy giá) |
| 11 | BEH-007 | Vì sao nên tuyển bạn | **2** | — |
| 12 | BEH-009 | Câu hỏi ngược | **2** | — |

**Phủ `BEH`: 0% → 100%** (12/12).

---

## 🔴 NGUYÊN NHÂN GỐC — một lỗi lặp 12 lần (khối phân tích CHUNG)

> 📌 **Sổ yếu trỏ về đây.** Bảy câu ≤2 điểm của phiên này **chung đúng một nguyên nhân**; chép phân tích vào từng dòng sổ yếu là lỗi *"một sự thật, nhiều chỗ"* mà repo đã dính 4 lần. Mỗi dòng sổ yếu chỉ ghi **phần riêng** của câu đó và trỏ về khối này.

### Mọi câu trả lời đều dừng ở chữ **A** của STAR

| STAR | Kết quả đo |
|---|---|
| **S** — Situation | ✅ gần như luôn có |
| **T** — Task | ✅ có |
| **A** — Action | ✅ **rất tốt** — chi tiết kỹ thuật đúng, thứ tự mạch lạc, không bịa |
| **R** — **Result** | ❌ **0/12 câu tự nói ra** |

**Con số tự nguyện nói ra trong cả phiên: 0.** Dòng đầu [bank/behavioral.md](../bank/behavioral.md) ghi: *"luôn kết bằng **con số**"*.

**Ứng viên CÓ con số.** Q4 khi bị hỏi *"tiết kiệm bao nhiêu"* → ra ngay **1 ngày → 10 phút + 1 giờ verify**, sắc và đáng tin (tính cả chi phí review). Resume có sẵn **70%** và **>50%**. Không cái nào được nói ra.

**6/12 câu phải probe — và 4/6 probe đó chỉ để hỏi *"kết quả là gì"*.**
Theo [thang `rapid`](../config.md#4): *"phải gợi mới ra"* = **2 điểm**. Đó là toàn bộ lý do con số 2.42.

### Ba biểu hiện của cùng một lỗi

| Biểu hiện | Câu dính | Mô tả |
|---|---|---|
| **① Dừng ở A, không tới R** | 1, 2, 3, 4, 6, 8 | Kể xong *"em đã làm gì"* rồi im — thiếu *"và kết quả là"* |
| **② Kể CHÍNH SÁCH thay vì CÂU CHUYỆN** | **5**, 7 | Hỏi *"một lần"*, trả lời *"hầu hết các lần"*. Interviewer không chấm được chính sách |
| **③ Không bắc cầu sang thứ mình ĐÃ CÓ** | 8, 11 | Q8 chọn Windows App thay vì Yocto/BBB; Q11 tự khai *"trải dài nhiều tầng"* mà không dẫn một bằng chứng nào |

### ⚠️ Bài tập 18/08 chưa làm — bằng chứng đanh nhất

Log [18/08](2026-08-18--rapid--resume.md) kê đơn **nguyên văn** cho `RES-001`, cùng chất liệu S-Box, cùng lỗi:

> **Sửa thế nào:** đảo thứ tự. Nói **vấn đề trước, công nghệ sau**:
> *"Nhiều S-Box ghép thành một màn lớn. Mỗi máy tự chỉnh sáng theo cảm biến của nó thì chúng lệch nhau, nhìn thành từng ô. Em làm phần đồng bộ độ sáng qua POSIX message queue — kết quả là **ghép lại nhìn như một màn duy nhất**, và **cùng một binary chạy được cả chế độ đơn lẻ lẫn đồng bộ**."*

Câu 1 hôm nay: *"…em phát triển tính năng Adaptive Brightness, tự động điều khiển độ sáng theo điều kiện ánh sáng môi trường."* → probe → *"người xem cảm thấy dễ chịu, ví dụ ngoài trời, phòng họp."*

**Phần hay nhất — đồng bộ nhiều màn, một binary hai chế độ — không xuất hiện.** [§📍 plan](../../study-plans/datalogic-plan.md) đã ghi đây là *"🗣️ bài tự luyện còn nợ"* suốt **5 ngày**.

### ⭐ Bằng chứng ngược — vấn đề KHÔNG phải năng lực diễn đạt

**BEH-008 = 4 điểm, ~20 giây, không thừa một chữ.** Đó là câu **duy nhất ứng viên đã chuẩn bị trước**.

⇒ **Khi có khung sẵn thì đóng gói rất giỏi.** 11 câu kia hỏng vì **chưa dựng khung**, không phải vì không biết nói. Đây là lý do lỗ hổng này **sửa được nhanh** — và tại sao đọc thêm tài liệu **không** chữa được nó.

---

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### Câu 1 · BEH-001 · **2/4**

**Đề:** *"Chào bạn. Bạn kể tôi nghe về **thành tựu bạn tự hào nhất** trong công việc đi."*
**🔁 Probe:** *"Được. Và **kết quả** thì sao — nó mang lại gì?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"…tham gia phát triển S-Box Signage 2026, trong đó em phát triển tính năng Adaptive Brightness, tự động điều khiển độ sáng màn hình theo điều kiện ánh sáng môi trường."* → probe → *"thiết bị hoạt động với độ sáng tối ưu tuỳ môi trường, người xem cảm thấy dễ chịu; ví dụ ngoài trời, phòng họp, phòng hội nghị."*

**✅ Được:** chọn **đúng project** (S-Box, không phải Windows app) — bank gọi đây là bẫy và ứng viên **không mắc**.

**❌ Vì sao 2:**
1. Dừng ở *"em phát triển tính năng X"* = mô tả **nhiệm vụ**, không phải **thành tựu**.
2. Probe ra được lợi ích nhưng **định tính** (*"dễ chịu"*), **không con số**.
3. **Bỏ mất phần mạnh nhất** — đồng bộ nhiều thiết bị, một binary hai chế độ.

> **Bank [BEH-001](../bank/behavioral.md) — chú ý phần R:**
> **R:** Giảm ~**70%** thời gian phần việc đó; yếu tố giúp đạt **Best Employee of Quarter**.

⚠️ Bank ráp BEH-001 quanh **AI/Tizen**, ứng viên chọn **S-Box** — cả hai đều hợp lệ, nhưng chọn S-Box thì **phải tự dựng R** vì bank chưa có. **Bản dựng sẵn (28 giây):**

> *"Nhiều S-Box ghép thành một màn lớn — mỗi máy tự chỉnh sáng theo cảm biến của nó thì chúng lệch nhau, nhìn ra từng ô. Em làm adaptive brightness và phần đồng bộ qua POSIX message queue. **Kết quả: ghép lại nhìn như một màn duy nhất, và cùng một binary chạy được cả chế độ đơn lẻ lẫn đồng bộ** — không phải build hai bản."*

Có vấn đề → giải pháp → kết quả → một chi tiết kỹ thuật đáng nhớ.

**Chốt:** *"Thành tựu = vấn đề tôi gỡ được, không phải tính năng tôi viết ra."*
**Lần sau sẽ hỏi:** vẫn T1, **góc khác** — *"trong project đó, phần nào KHÓ nhất và vì sao?"* — xem có tự đi tới R không khi đề không gợi chữ "thành tựu".
**Ôn:** [bank BEH-001](../bank/behavioral.md) · [log 18/08 RES-001](2026-08-18--rapid--resume.md)
</details>

---

### Câu 2 · BEH-002 · **2/4**

**Đề:** *"Kể tôi nghe về **bug khó nhất** bạn từng xử lý."*
**🔁 Probe:** *"Rồi cuối cùng **nguyên nhân là gì**?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"…lỗi màn hình đen nhưng backlight vẫn sáng. Quá trình debug phải thực hiện đầy đủ các bước gồm reproduce, capture log, phân tích từ khi người dùng bấm điều khiển TV bật → userspace → kernel space, xuyên suốt từng tầng để tìm ra đúng nguyên nhân gốc."* → probe → *"sau khi bật inverter, FRC không unmute và giữ nguyên black pattern, do đó TV vẫn đen trong khi backlight vẫn sáng."*

**✅ Được:** quy trình debug kể **đúng thứ tự, mạch lạc**. Nguyên nhân (khi probe) **rất chính xác**.

**❌ Vì sao 2:** câu hỏi là *"bug khó nhất"* — **thứ interviewer muốn nghe chính là CÁI BUG**, mà nó chỉ ra sau khi bị hỏi. Thiếu luôn **cách tìm ra** và **kết quả**.

📌 **So sánh trực tiếp: cùng câu chuyện này đạt 4 điểm ngày 18/08** — vì có thêm một câu mà hôm nay không có:

> **Câu chốt** *(log 18/08, RES-008)*: *"Triệu chứng nằm ở màn hình, nhưng gốc nằm ở chỗ **đường bật không đối xứng với đường tắt**. Em tìm ra bằng cách **đặt hai đường cạnh nhau** chứ không phải đọc lần lượt."*

Câu đó nêu một **kỹ thuật chuyển được sang bug khác**, không chỉ một sự cố. Log 18/08 đánh dấu 🎯 *"ĐÂY là câu đáng nhớ nhất"* vì nó **vá đúng chỗ resume bị nghi**: resume liệt kê Debugging là kỹ năng trong khi [CLAUDE.md §2](../../../CLAUDE.md) ghi đó là điểm yếu tự nhận.

**Chốt:** *"Kể bug = kể cái bug, không phải kể quy trình đi tìm nó. Quy trình chỉ đáng nói khi nó dạy được một kỹ thuật."*
**Lần sau sẽ hỏi:** *"kể một bug mà em SAI hướng chẩn đoán lúc đầu — em nhận ra bằng cách nào?"*
**Ôn:** [bank BEH-002](../bank/behavioral.md) · [bank DBG-016](../bank/debugging.md) · [log 18/08 RES-008](2026-08-18--rapid--resume.md)
</details>

---

### Câu 3 · BEH-005 · **2/4**

**Đề:** *"**Điểm yếu của bạn** là gì?"*
**🔁 Probe:** *"Đó là hạn chế của môi trường. Còn về **bản thân bạn** — kỹ năng nào bạn thấy mình còn yếu?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời (lượt 1):** *"vì làm ở tầng system software, nơi đòi hỏi tính stable rất cao và phải tuân thủ chuẩn chỉ từng dòng code submit, nên em không có nhiều cơ hội phát huy sự sáng tạo và chủ động khi viết code; bù lại em cải thiện qua project cá nhân và luyện DSA."*

**Bạn trả lời (sau probe):** *"kỹ năng liên quan đến debug — làm việc trên TV full-set, hàng nghìn package, debug sâu bằng công cụ là bất khả thi, chủ yếu qua reproduce và capture log. Em thiếu kinh nghiệm dùng tool debug, đang tự cải thiện bằng cách debug qua các chương trình nhỏ tự viết."*

**❌ Câu đầu là "phi điểm yếu"** — quy về **hạn chế của môi trường**, không phải thiếu sót của bản thân. ⚠️ **Rủi ro kèm theo:** nó đọc ra thành *"tôi thấy gò bó vì quy trình"* — tín hiệu xấu cho vai trò embedded, nơi kỷ luật là **yêu cầu công việc**, không phải trở ngại.

**✅ Sau probe thì rất tốt:** nêu đúng **debug tooling**, có **lý do khách quan hợp lý**, có **hành động khắc phục**.

> **Bank [BEH-005](../bank/behavioral.md):**
> Chọn điểm yếu **thật + đang cải thiện**: *"Công cụ debug như GDB tôi dùng chưa thật sâu — trước chủ yếu debug qua log userspace, `dmesg`, core dump trên server nội bộ. Tôi đang chủ động học GDB bài bản (breakpoint, watch, backtrace, remote debug với gdbserver) để chủ động hơn."* → vừa thật, vừa cho thấy **đã có giải pháp thay thế** và **đang tiến bộ**.

**Đáp án đúng nằm sẵn trong bank.** Đi vòng qua một câu vòng vo rồi mới tới ⇒ mất điểm ở chỗ lẽ ra được cộng.

🔗 **Nối tới việc đang nợ:** làm xong **7 bài 🧪 lab `DBG-030…036`** thì câu này nâng cấp thành: *"…em đang luyện bằng cách dựng lại từng lớp lỗi trên chương trình nhỏ — `strace` cho lỗi chết lúc khởi động, ASan cho corruption, core dump cho crash không có mặt lúc đó."* Cụ thể hơn hẳn, và **đo được**.

**Chốt:** *"Nói thẳng một điểm yếu kỹ thuật thật + việc đang làm để vá. Đổ cho môi trường thì mất cả điểm trung thực lẫn điểm cầu thị."*
**Lần sau sẽ hỏi:** *"nếu được thêm 3 tháng chỉ để nâng một kỹ năng, em chọn gì và làm thế nào?"* — cùng tầng, khác vỏ.
**Ôn:** [bank BEH-005](../bank/behavioral.md) · [bank DBG-017](../bank/debugging.md) · [09-debugging/gdb.md](../../../09-debugging/gdb.md)
</details>

---

### Câu 4 · BEH-004 · **3/4**

**Đề:** *"Kể về một lần bạn **cải tiến quy trình** hoặc tự động hoá một việc gì đó cho team."*
**🔁 Probe:** *"Tiết kiệm được **bao nhiêu**?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** tool porting Python. *"Mỗi năm có SoC mới, FRC-TCON mới trên các mẫu TV, nên porting và bringup (**chưa phân biệt được 2 cái này**) là công việc bắt buộc hàng năm. Thủ công thì lặp lại: tầng lib và kernel API, copy từ chip chuẩn làm base rồi sửa tay — đổi tên file, sửa từng tên hàm, sửa config, update Makefile. Tự động hoá bằng tool Python: copy, paste, đổi tên… dành thời gian cho phần porting quan trọng hơn là update device tree."* → probe → *"không có con số chính xác, nhưng ước lượng: porting mất 1 ngày, dùng tool còn 10 phút + 1 tiếng review/build/verify."*

**✅ Rất tốt:** tình huống cụ thể, mô tả **đúng bản chất công việc lặp**, và **thành thật là ước lượng** khi đưa con số — cùng phẩm chất đã cho `RES-009` **4 điểm** ngày 18/08. Biết **tách việc máy làm được khỏi việc người phải làm** (device tree) là tư duy tự động hoá đúng.

**❌ Vì sao chưa 4:** con số **chỉ ra khi bị hỏi**. Resume ghi sẵn **>50%** — không nói ra thì người đọc resume và người nghe bạn nói đang nhận **hai thông điệp khác nhau**.

**⚠️ Lỗ hổng thuật ngữ ứng viên TỰ NÊU — cần vá gấp, JD Datalogic dùng cả hai từ:**

| | **Porting** | **Bring-up** |
|---|---|---|
| Xuất phát | Code **đã chạy** trên nền A | Board **mới, chưa chạy gì** |
| Việc chính | Sửa cho hợp nền B (API, config, build) | Làm nó **sống lần đầu**: clock, DDR, bootloader, console, storage |
| Xong khi | Chức năng cũ chạy lại trên nền mới | **Boot tới shell**, các bus cơ bản lên |
| Hỏng thì thấy gì | Build lỗi / chức năng sai | **Không thấy gì cả** — màn đen, không log |

Việc ứng viên mô tả (copy chip base, đổi tên hàm, sửa Makefile) là **porting**. Bring-up diễn ra **trước đó, ở tầng board**.

**Chốt:** *"Porting là làm code cũ chạy trên nền mới; bring-up là làm board mới sống lần đầu. Tool của em giải quyết porting."*
**Lần sau sẽ hỏi:** *"tool đó có bao giờ tạo ra lỗi mà làm tay không mắc không?"* — dò xem có nghĩ về rủi ro của tự động hoá không.
**Ôn:** [bank BEH-004](../bank/behavioral.md) · [08-embedded-systems/boot-process.md](../../../08-embedded-systems/boot-process.md)
</details>

---

### Câu 5 · BEH-006 · **1/4** 🔴 thấp nhất phiên

**Đề:** *"Kể về một lần bạn **bất đồng quan điểm** với đồng nghiệp hoặc quản lý. Chuyện đó kết thúc thế nào?"*
**🔁 Probe:** *"Cho tôi **một lần cụ thể** — bất đồng về cái gì, bạn đề xuất gì, cuối cùng chốt theo hướng nào?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"hầu hết các bất đồng liên quan đến việc nên implement một feature như thế nào… em và mọi người chọn hướng cùng ngồi lại, liệt kê hết tất cả giải pháp và cùng phân tích mặt lợi hại, sau đó chốt phương án tốt nhất trong bối cảnh, hoặc tạo phiên bản kết hợp."*
→ probe → **"chưa rõ. Hãy gợi ý dựa trên resume của tôi."**

**❌ Đề hỏi "kể về MỘT lần", trả lời "hầu hết các lần".** Đó là **chính sách**, không phải **câu chuyện**. Interviewer không chấm được chính sách — ai cũng nói được *"ngồi lại phân tích lợi hại"*.

> **Bank [BEH-006](../bank/behavioral.md):**
> *"Khung: nêu bất đồng **về kỹ thuật, không cá nhân** → lắng nghe lý do của họ → **đưa dữ liệu/đo đạc** → cùng đi đến quyết định → tôn trọng kết quả chung. Kết: học được gì."*

Mắt xích thiếu là **"đưa dữ liệu/đo đạc"** — nó biến câu chuyện từ *"hai người cãi nhau rồi hoà"* thành ***"tôi giải quyết bất đồng bằng bằng chứng"***.

**⭐ Ứng viên KHÔNG thiếu chất liệu — có sẵn ít nhất ba, đều đã đạt 4 điểm ở phiên 18/08:**

| Chất liệu | Bất đồng ẩn trong đó | Dữ liệu để chốt |
|---|---|---|
| **Chọn POSIX mq** thay socket/shm (RES-005) | *"Sao không dùng shm cho nhanh?"* | Chỉ `uint16_t` — không cần chia sẻ vùng nhớ lớn; shm tự tạo race + tight coupling; mq có sẵn ranh giới message |
| **4 giây settling window** (RES-006) | Chậm thích nghi đổi lấy hết nhấp nháy — **chắc chắn có người muốn ngắn hơn** | Đo số lần đổi độ sáng/phút ở từng cửa sổ |
| **Đường power-on bất đối xứng** (RES-008) | Sửa ở FRC hay ở tầng lib? | Đường nào là nguồn gốc bất đối xứng |

**Chốt:** *"Behavioral hỏi MỘT lần thì phải có MỘT thời điểm. Mở bằng 'Hồi làm S-Box, có lần…' — không bao giờ mở bằng 'thường thì…'."*
**Lần sau sẽ hỏi:** ⚠️ **vẫn T1, cùng câu** (chưa trả lời được thì chưa đo được gì) — nhưng **thu hẹp đề** để ép ra câu chuyện: *"kể về một quyết định kỹ thuật của em bị người khác phản đối"*.
**Ôn:** [bank BEH-006](../bank/behavioral.md) · [log 18/08](2026-08-18--rapid--resume.md) — chất liệu RES-005/006/008
</details>

---

### Câu 6 · BEH-003 · **3/4**

**Đề:** *"Bạn có đi công tác Hàn Quốc hỗ trợ bring-up. Kể về một lần **làm việc dưới áp lực** ở đó."*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"áp lực phải thực hiện nhiều việc một lúc: bringup TV model 2026, bringup S-Box signage 2026, analyze và fix defect các mẫu hiện hành, các mẫu cũ hỗ trợ OS-Upgrade lên thế hệ mới. Hướng giải quyết: dựa trên deadline hiện tại ở mỗi project — S-Box signage 2026 có deadline sớm nhất nên ưu tiên, sau đó là các mẫu TV và defect."*

**✅ Được:** tình huống **thật và cụ thể** (bốn luồng việc song song, kể tên được), và **tiêu chí ưu tiên rõ ràng bằng deadline**. Không nói *"em cố gắng hết sức"* — nói *"cái nào deadline sớm nhất làm trước"*. Đó là câu trả lời của người **biết quản lý việc**, không phải người chịu đựng áp lực.

**❌ Vì sao chưa 4:** thiếu **R**. Bank có sẵn:

> **Bank [BEH-003](../bank/behavioral.md):**
> **R:** Bring-up đúng tiến độ, kịp mass-production.

Một câu, và nó biến *"em bận"* thành ***"em giao hàng đúng hẹn"***. Thiếu thêm vế **team nước ngoài** mà đề gợi ý — phối hợp trực tiếp kỹ sư HQ, khác ngôn ngữ/múi giờ (đây cũng là bằng chứng tiếng Anh, thứ JD yêu cầu).

**Chốt:** *"Ưu tiên theo deadline là cách trả lời đúng — chỉ cần thêm một câu nói rằng cuối cùng mọi thứ kịp."*
**Lần sau sẽ hỏi:** *"nếu hai project cùng deadline một ngày thì em cắt cái gì?"* — ép chọn, dò tiêu chí sâu hơn deadline.
**Ôn:** [bank BEH-003](../bank/behavioral.md)
</details>

---

### Câu 7 · BEH-010 🆕 · **3/4**

**Đề:** *"Kể về một lần bạn **làm hỏng việc** — một quyết định sai, một lỗi bạn gây ra, hoặc một việc không đạt. Chuyện gì xảy ra và bạn làm gì sau đó?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** hai nhóm nguyên nhân. *"Khách quan: document sai (chưa update), giao tiếp với Project Owner chưa hiểu ý nhau (họ hiểu nhiều về sản phẩm và mặc định mình cũng hiểu). Chủ quan: chưa kiểm tra toàn bộ edge case, phải sửa dần từng cái (chạy bình thường thì ổn, chuyển qua adaptive brightness thì lỗi, lỗi độ sáng lúc boot…). Giải pháp: thay vì im lặng và mò mẫm, em liên hệ trực tiếp Project Owner hỏi thật rõ document → họ phát hiện và sửa specification; trao đổi kỹ các tình huống edge case rồi cùng quyết định."*

**✅ Được — hai thứ đáng giá:**
1. **Tự nhận phần chủ quan** (chưa phủ hết edge case). Nhiều người chỉ kể phần khách quan rồi dừng.
2. **Đổi hành vi cụ thể và có kết quả ra ngoài bản thân**: *"thay vì im lặng mò mẫm → liên hệ thẳng PO"* → **PO phát hiện và sửa specification**. Đó là kết quả thật, ảnh hưởng tới cả team, không chỉ tới mình.

**❌ Vì sao chưa 4 — hai chỗ:**
1. **Vẫn là "hai nhóm nguyên nhân", không phải MỘT câu chuyện.** Cùng lỗi ② với câu 5, nhẹ hơn.
2. ⭐ **Nguyên nhân khách quan đứng TRƯỚC** ⇒ nghe như đỡ đòn trước khi nhận lỗi. **Đảo lại**: nhận phần mình trước (một câu, không rào đón), bối cảnh sau.

**Đáp án đầy đủ — khung 4 bước** (đã ghi vào [bank BEH-010](../bank/behavioral.md)): ① nhận phần của mình trước → ② bối cảnh sau → ③ **đổi hành vi cụ thể** *(phần được chấm)* → ④ kết quả đo được.

**Chốt:** *"Nhận phần của mình trước rồi mới nói bối cảnh — và thứ đáng nói nhất là em đã đổi cách làm việc, không phải em đã sửa xong bug."*
**Lần sau sẽ hỏi:** *"lỗi đó có lọt tới khách hàng không? Nếu có thì xử lý thế nào?"* — dò mức độ nghiêm trọng thật và khả năng xử lý khủng hoảng.
**Ôn:** [bank BEH-010](../bank/behavioral.md) 🆕
</details>

---

### Câu 8 · BEH-011 🆕 · **2/4**

**Đề:** *"Vị trí này có mảng bạn chưa làm qua. Kể tôi nghe một lần bạn phải **học một thứ hoàn toàn mới trong thời gian ngắn** — bạn học thế nào, và mất bao lâu để làm được việc?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** Windows App (Samsung Display Manager). Ba trục học: *"① Windows API hoạt động thế nào — lấy realtime, file handling, cú pháp; tận dụng nền Linux OS giúp học nhanh hơn. ② Tư duy thiết kế app: học MVVM, không strict như system software, thử nghiệm và verify nhanh. ③ Học và tăng cường kỹ năng debug: build bằng Visual Studio, debug mạnh, đặt breakpoint, in log trực tiếp khi app vận hành."*

**✅ Được:** ba trục học rõ ràng, và **tự nối sang điểm yếu debug** — ý hay, cho thấy biết mình thiếu gì.

**❌ Vì sao 2 — CHỌN SAI CHẤT LIỆU (lỗi nặng nhất, không phải lỗi nội dung):**

Đề mở **đúng cánh cửa**: *"vị trí này có mảng bạn chưa làm qua"*. Ứng viên chọn **Windows App** — dòng **ít liên quan JD nhất** trên resume.

Thứ đáng nói là **Yocto trên BeagleBone Black**: có thật, tự dựng image, nắm bitbake/OE-core/poky. **Trùng đúng trụ JD** mà resume đang để trống.

⚠️ **Đây là RES-012 lặp lại** (18/08, **2 điểm**): *"không bắc cầu sang thứ mình ĐÃ CÓ"*. Cùng cơ chế hỏng, đổi vỏ câu hỏi. Hai lần trong 5 ngày.

**Thiếu thêm:** **mốc thời gian** (ứng viên có: **1 tháng** — đã nói ở phiên 18/08) và **kết quả**.

> **Bank [BEH-011](../bank/behavioral.md) 🆕 — luật chọn chất liệu:**
> Chọn thứ **gần JD nhất** mà bạn thật sự có, **không** chọn thứ dễ kể nhất. Phần được chấm là **bước ② bắc cầu từ thứ đã có**, không phải câu chuyện học tập.

**Chốt:** *"Câu này không hỏi khả năng học — nó hỏi chỗ trống trong resume có lấp được không. Chất liệu chọn quan trọng hơn câu chuyện kể."*
**Lần sau sẽ hỏi:** hỏi thẳng, tách riêng: *"em có làm Yocto bao giờ chưa?"* — xem có tự nêu BeagleBone Black không (cùng góc đã ghi cho `RES-012`).
**Ôn:** [bank BEH-011](../bank/behavioral.md) 🆕 · [bank RES-012](../bank/resume.md) · [06-build-systems/yocto.md](../../../06-build-systems/yocto.md)
</details>

---

### Câu 10 · BEH-012 🆕 · **3/4**

**Đề:** *"**Mức lương kỳ vọng** của bạn là bao nhiêu?"*
**🔁 Probe (đẩy giá):** *"Con số đó hơi cao so với ngân sách bọn anh dự kiến cho vị trí này. Bạn nghĩ sao?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"35 triệu VNĐ gross."* → đẩy giá → *"đối với em đây là con số tốt để bản thân có thể toàn tâm toàn ý làm việc và cống hiến hết mình, không còn nghĩ nhiều về việc lương bổng trong tương lai. Nên em hy vọng mình có thể xem xét được mức lương này."*

**✅ Được — hai thứ, và thứ hai quan trọng hơn:**
1. **Ra số dứt khoát, không vòng vo.** Rất nhiều ứng viên hỏng ngay đây (*"em để công ty quyết"*) và mất quyền neo giá.
2. **Giữ được bình tĩnh khi bị đẩy** — không rút lui, không phòng thủ gay gắt. Đây mới là thứ câu này thật sự đo.

**❌ Vì sao chưa 4 — neo sai chỗ:**

| Cách neo | Ví dụ | Người trả lương nghe thấy |
|---|---|---|
| ❌ **Nhu cầu bản thân** *(bạn đã dùng)* | *"để em toàn tâm toàn ý làm việc"* | *"Đây là vấn đề của bạn, không phải giá trị của bạn."* |
| ✅ **Thị trường + năng lực cụ thể** | *"Mức này hợp mặt bằng kỹ sư 3 năm có kernel driver, multi-chipset và kernel migration 5.10→6.12."* | *"Có căn cứ — và đang nhắc tôi nhớ vì sao bạn đáng giá."* |

**Thiếu thêm:** không **hỏi ngược band của họ**, và không mở đường **tổng gói đãi ngộ**.

**Bản mẫu khi bị đẩy:**
> *"Em hiểu ạ. Con số đó em đưa dựa trên mặt bằng thị trường cho kỹ sư có nền driver và kernel — nhưng em quan tâm tổng thể gói đãi ngộ và lộ trình chứ không riêng lương cứng. **Anh chia sẻ giúp em band của vị trí này thì em dễ trao đổi hơn.**"*

Câu cuối **chuyển quả bóng về phía họ** mà **không hạ giá**.

> **Bank [BEH-008](../bank/behavioral.md)** cũng nhắc: *"nếu hỏi thẳng lương kỳ vọng thì trao đổi dựa trên **thị trường + năng lực**"*.

**Chốt:** *"Ra số dứt khoát, neo vào thị trường và năng lực chứ không vào nhu cầu của mình, rồi hỏi ngược band của họ."*
**Lần sau sẽ hỏi:** *"nếu bọn anh chỉ duyệt được 30 nhưng có thêm cổ phần/thưởng theo dự án thì sao?"* — dò khả năng đàm phán tổng gói.
**Ôn:** [bank BEH-012](../bank/behavioral.md) 🆕
</details>

---

### Câu 11 · BEH-007 · **2/4**

**Đề:** *"**Vì sao chúng tôi nên tuyển bạn** cho vị trí này?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"Em có kiến thức và mang lại bộ kỹ năng trải dài nhiều tầng trong system software (userspace, kernel space), khá phù hợp với công việc làm sản phẩm máy quét của công ty mình và đặc biệt là vị trí Embedded Linux Engineer. Em nghĩ mình sẽ nhanh chóng hòa nhập và tạo ra được giá trị sớm."*

**✅ Được:** gọi tên **sản phẩm của họ** (máy quét) và **đúng vị trí** — cụ thể hơn hẳn câu 9.

**❌ Vì sao 2:** *"kiến thức trải dài nhiều tầng"* là **lời tự khai không bằng chứng**. **Không một dòng nào từ resume được dẫn ra.** Câu này là chỗ duy nhất trong buổi phỏng vấn bạn được **chủ động** chọn ba thứ mạnh nhất của mình — bỏ trống nó là lãng phí lớn nhất.

> **Bank [BEH-007](../bank/behavioral.md):**
> Khớp **3 điểm mạnh** với JD: (1) nền **system/C++ + HAL/driver** thực chiến trên sản phẩm thật; (2) kinh nghiệm **multi-chipset, kernel migration, Device Tree** đúng chất BSP; (3) tư duy **tự động hoá & cải tiến** (giảm 70%/50%). Nhấn: học nhanh, làm được việc cross-layer.

**Ba trụ, mỗi trụ một bằng chứng — 45 giây.** Ứng viên có đủ cả ba và không dùng cái nào.

**Chốt:** *"Ba trụ, mỗi trụ một bằng chứng cụ thể. Không có bằng chứng thì đó chỉ là lời tự khen."*
**Lần sau sẽ hỏi:** *"trong ba thứ em vừa nêu, thứ nào em nghĩ đội anh thiếu nhất?"* — ép xếp hạng, dò hiểu biết về JD.
**Ôn:** [bank BEH-007](../bank/behavioral.md) · [RESUME.tex](../../../RESUME.tex)
</details>

---

### Câu 12 · BEH-009 · **2/4**

**Đề:** *"Câu cuối. **Bạn có câu hỏi gì cho tôi không?**"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"Công ty mình có rất nhiều loại hình sản phẩm phức tạp và quy mô software cũng lớn. Em muốn hỏi thêm về domain mà team Việt Nam đang thực hiện, chúng ta có vai trò gì, đến mức độ nào trong phần được giao từ công ty mẹ?"*

**✅ Được:** **câu hỏi tốt** — phạm vi và quyền tự quyết của team VN so với công ty mẹ. Cho thấy nghĩ về **vai trò và ownership**, không chỉ về task. Đây là câu người có kinh nghiệm mới hỏi.

**❌ Vì sao 2 — chỉ có MỘT câu.** Bank: *"chuẩn bị **3–5 câu**"*, và *"không hỏi là điểm trừ"*. Một câu rồi dừng nằm gần "không hỏi" hơn là gần "có chuẩn bị".

**Thiếu hẳn nhóm kỹ thuật** — mà nhóm đó **vừa hỏi vừa khoe**:

> **Bank [BEH-009](../bank/behavioral.md):**
> - Team dùng **SoC/nền tảng** nào; công việc nghiêng **kernel/driver (BSP)** hay **userspace (C++)**?
> - Quy trình từ code đến sản phẩm — **build system, CI, cách test trên phần cứng thật**?
> - Cách team debug lỗi khó/cross-layer — có **hạ tầng core dump/tracing** nội bộ?
> - Bài toán kỹ thuật khó nhất team đối mặt 6–12 tháng tới?

Hỏi câu về **cross-layer debug infrastructure** là phát tín hiệu *"tôi biết đó là một hạng mục hạ tầng phải đầu tư"* — mà đó **đúng là việc bạn làm thật**.

**Chốt:** *"Chuẩn bị 3–5 câu, trong đó ít nhất 2 câu kỹ thuật. Câu hỏi ngược là lượt nói cuối cùng của mình — đừng bỏ."*
**Lần sau sẽ hỏi:** yêu cầu **3 câu** và xem có câu nào chạm tới SoC/CI/hạ tầng debug không.
**Ôn:** [bank BEH-009](../bank/behavioral.md)
</details>

---

## Tổng kết

### Điểm mạnh

1. **Phần A của STAR rất chắc.** Chi tiết kỹ thuật đúng, thứ tự mạch lạc, không bịa. Nguyên liệu tốt — thiếu đúng bước đóng gói cuối.
2. **Trung thực có kiểm soát.** Tự khai *"chưa phân biệt porting/bring-up"*, tự nhận phần chủ quan khi hỏng việc, không thổi phồng con số (*"không có con số chính xác, nhưng ước lượng…"*). Interviewer tin được người như vậy.
3. **BEH-008 = 4 điểm chứng minh năng lực đóng gói CÓ SẴN.** Câu duy nhất đã chuẩn bị trước → gọn ghẽ trong 20 giây.

### Lỗ hổng ưu tiên

| # | Lỗ hổng | Loại | Việc phải làm |
|---|---|---|---|
| **1** 🔴 | **0/12 câu tự đi tới R. 0 con số tự nguyện.** 4/6 probe chỉ để hỏi *"kết quả là gì"* | **Đóng gói** | Viết **5 khung STAR** ra giấy, **mỗi khung kết bằng một con số**. Nói to bấm giờ **60–90 giây/khung** |
| **2** 🔴 | **Kể CHÍNH SÁCH thay vì CÂU CHUYỆN** (Q5 = 1đ, Q7 một phần) | **Đóng gói** | Mọi câu behavioral mở bằng **một thời điểm cụ thể**: *"Hồi làm S-Box, có lần…"*. **Cấm** mở bằng *"thường thì…"* |
| **3** 🟠 | **Không bắc cầu sang thứ mình ĐÃ CÓ** (Q8 chọn Windows App thay vì Yocto/BBB; Q11 không dẫn bằng chứng nào) | **Đóng gói** — lặp lại `RES-012` (2đ, 18/08) | Trước mỗi câu tự hỏi: *"chi tiết nào trên resume trả lời được câu này?"* |

> **Cả ba đều là lỗi ĐÓNG GÓI. Không có một lỗ hổng kiến thức nào trong cả phiên.**
> Kết luận **y hệt 18/08** — và việc nó lặp lại nguyên vẹn sau 5 ngày là bằng chứng bài tập luyện nói chưa được làm.

### Đọc đúng con số 2.42

| Phiên | Điểm | Bản chất domain |
|---|---|---|
| 17/08 `rapid` drivers-dt | **3.92** | việc làm hằng ngày |
| 18/08 `rapid` resume | 3.67 | kể về mình — **có chuẩn bị một phần** |
| 19/08 `rapid` bsp | 3.33 | đọc tài liệu, chưa thực hành |
| 21/08 `daily` cpp-system | 3.0 | T2, 4/6 câu retention/weak |
| **23/08 `rapid` behavioral** | **2.42** | **chưa chuẩn bị gì** |

Đây là **thang đo mức độ chuẩn bị**, không phải thang đo năng lực. `BEH` là domain **duy nhất chưa từng đầu tư một giờ nào** — và là domain **100% bị hỏi**, thường **ngay 10 phút đầu**, đúng lúc interviewer hình thành ấn tượng.

**Tin tốt: đây là domain RẺ NHẤT để sửa.** Không cần học gì mới — chất liệu đủ trong [RESUME.tex](../../../RESUME.tex) và trong 4 log phiên trước. Chỉ cần **viết ra và nói to**.

---

## Cập nhật đã thực hiện (Bước 4)

- ✅ Log này
- ✅ `bank/behavioral.md`: **9 → 12 câu** — thêm `BEH-010` (làm hỏng việc) · `BEH-011` (học nhanh, ⭐ luật chọn chất liệu) · `BEH-012` (lương kỳ vọng, ⭐ cách giữ giá)
- ✅ `weak-register.md`: thêm **7 câu ≤2đ**, mỗi dòng chỉ ghi **phần riêng** + trỏ về khối *"NGUYÊN NHÂN GỐC"* ở trên
- ❌ `coding-arena/reviewed/` — không áp dụng (phiên behavioral, không có code)
- ✅ `study-plans/datalogic-plan.md` §📍 — **BEH 0% → 100%**, đổi ▶️ LÀM TIẾP
