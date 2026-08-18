# Phiên mock — 2026-08-18 · `rapid` · track `resume`

> **Kết quả: 44/48 = 3.67/4** · 12 câu · **9 câu điểm 4**, 2 câu điểm 3, **1 câu điểm 2**
> **Ý nghĩa riêng của phiên này:** `RES` đi từ **0% → 100%** phủ trong một phiên. Đây là domain **chắc chắn 100% bị hỏi** (phỏng vấn thật dành 30–50% thời gian) mà trước hôm nay **chưa hỏi câu nào**.
> Thang chấm: [config §4 → thang riêng cho `rapid`](../config.md) — đo **độ trôi chảy T1**, không đo T2. Lan man bị trừ ngang với thiếu ý.

---

## Bảng điểm

| # | ID | Chủ đề | Điểm |
|---|---|---|---|
| 1 | RES-001 | Giới thiệu bản thân + project tâm đắc | **3** |
| 2 | RES-007 | Dimming / FRC / TCON cho người ngoài ngành | 4 |
| 3 | RES-003 | OOP trong kernel viết bằng C | 4 |
| 4 | RES-002 | Một C++ interface, nhiều cấu hình phần cứng | 4 |
| 5 | RES-010 | Device tree cho phần cứng mới | 4 |
| 6 | RES-004 | Kernel migration 5.10 → 6.12 | 4 |
| 7 | RES-006 | Luồng ALS → độ sáng, chống nhấp nháy | 4 |
| 8 | RES-005 | Vì sao POSIX mq (không socket/shm) | 4 |
| 9 | RES-008 | Debug xuyên tầng user–kernel | 4 |
| 10 | RES-009 | Con số "giảm 70%" đo bằng gì | 4 |
| 11 | RES-011 | Windows app MVVM | **3** |
| 12 | RES-012 | Resume thiếu Yocto / I2C-SPI / PCI-USB | **2** |

---

## Câu 1 — RES-001 · **3 điểm**

**Đề:** *"Em giới thiệu qua về công việc hiện tại và một project em tâm đắc nhất."*

<details><summary>Đã trả lời gì · nhận xét · phần còn thiếu</summary>

**Đã trả lời:** system software engineer, tầng middleware C++ interface / shared library xuống kernel API, component Dimming/FRC/TCON cho smart TV và enterprise. Project tâm đắc: S-Box Signage — adaptive brightness, *"có cơ hội vận dụng C++, thread, condition variable, POSIX message queue"*. **Sau khi bị probe** mới nêu được kết quả: chạy trên thiết bị độc lập có cảm biến, đồng bộ nhiều thiết bị bằng **một binary duy nhất**, hữu ích cho ngoài trời / phòng họp.

**Được:** câu định vị đầu tiên **chuẩn** — nêu đúng tầng mình đứng và biên trên/biên dưới. Chọn đúng project (S-Box, không phải Windows app) — đây là bẫy ③ của bank và ứng viên **không mắc**.

**Vì sao chưa 4:** phần *"tâm đắc"* kết thúc bằng **danh sách công nghệ đã dùng**, không phải **kết quả đạt được**. Phải có probe mới ra được cái hay nhất (một binary chạy cả hai chế độ). Ở màn screen thật, interviewer thường **không** probe — họ ghi "kể được task, chưa thấy impact" rồi đi tiếp.

**Bank yêu cầu 4 phần, ứng viên làm được 1, 2 và một nửa 4:**
> **2. Chọn ĐÚNG project.** Với JD Datalogic (Embedded Linux), chọn **S-Box** … **không** chọn Windows Display Manager. ✅
> **3. Vấn đề → giải pháp → con số.** Ví dụ S-Box: *"nhiều màn ghép thành một, mỗi máy tự chỉnh sáng theo cảm biến thì chúng lệch nhau, nhìn thành từng ô"* → mq đồng bộ → *"ghép lại nhìn như một màn duy nhất"*. ❌ **thiếu — đây là phần mất điểm**

**Sửa thế nào:** đảo thứ tự. Nói **vấn đề trước, công nghệ sau**:
> *"Nhiều S-Box ghép thành một màn lớn. Mỗi máy tự chỉnh sáng theo cảm biến của nó thì chúng lệch nhau, nhìn thành từng ô. Em làm phần đồng bộ độ sáng qua POSIX message queue — kết quả là ghép lại nhìn như một màn duy nhất, và cùng một binary chạy được cả chế độ đơn lẻ lẫn đồng bộ."*

Cùng nội dung, nhưng interviewer nghe được **impact ở giây thứ 10** thay vì giây thứ 90.

**Ôn lại:** [bank RES-001](../bank/resume.md) — mục *"Câu trả lời tốt gồm"* phần 3.
</details>

---

## Câu 2 — RES-007 · **4 điểm**

**Đề:** Dimming, FRC, TCON — giải thích ngắn gọn cho người ngoài ngành.

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** Dimming = điều khiển độ sáng backlight lúc vận hành. FRC = Frame Rate Controller, IC nội suy frame, cải thiện FPS, giảm rung lắc. TCON = Timing Controller, đảm bảo nội dung xuất ra panel đúng thời gian, điều chỉnh theo độ phân giải / picture mode / tần số quét, chạy test pattern. **Tự bổ sung ranh giới công việc**: phần mình làm chủ yếu là dịch logic chung chung (điều khiển độ sáng, tắt màn) thành các bước thật (bật black pattern → tắt GPIO backlight → tắt inverter); FRC/TCON chỉ là kernel API gửi command xuống SoC.

**Vì sao 4:** ba định nghĩa **đúng, gọn, không thuật ngữ thừa**. Và vế cuối là thứ ăn điểm mạnh nhất — **tự vạch ranh giới mình làm gì / không làm gì**. Nó phát tín hiệu trung thực và hiểu hệ thống, thay vì nhận vơ cả mảng.

**Giữ nguyên cách trả lời này ở phỏng vấn thật.**
</details>

---

## Câu 3 — RES-003 · **4 điểm**

**Đề:** Resume ghi dùng "cơ chế OOP" trong kernel driver. Kernel viết bằng C — làm sao có OOP?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** driver nhiều chủng loại theo chipset và đổi theo năm ⇒ cần một driver làm interface cho lớp trên. OOP triển khai bằng **struct làm instance, con trỏ hàm trong struct làm method**. Runtime: TV đọc config board/file → `insmod` đúng driver cho FRC/TCON của chipset đang chạy → con trỏ hàm ở driver interface link tới implement thật ⇒ cơ chế như **polymorphism**.

**Vì sao 4:** đúng **cả cơ chế lẫn thời điểm bind**. Nhiều người chỉ nói "dùng con trỏ hàm" mà không nói **ai gán và lúc nào** — ứng viên nói được cả chuỗi: config board → `insmod` → gán con trỏ.

**Nếu muốn thêm một câu ăn tiền** (không bắt buộc): *"đây chính là mẫu mà `file_operations` và `struct platform_driver` của kernel dùng — bảng con trỏ hàm, tức vtable viết tay."* Nối được vào cơ chế có sẵn của kernel cho thấy không phải tự nghĩ ra một mình.
</details>

---

## Câu 4 — RES-002 · **4 điểm**

**Đề:** Làm thế nào để **một** C++ interface phục vụ nhiều model phần cứng?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** interface chỉ mang ý nghĩa **tính năng**, áp cho mọi model, **app không biết** khác biệt. Lúc boot, TV đọc config board và tạo **Dimming instance** theo model (global/local dimming, OLED…). FRC/TCON đi qua **kernel ioctl** nên không có instance ở C++. Bộ interface điều khiển display liên kết ba thành phần: dimming instance → polymorphism ở C++; FRC/TCON → ioctl + polymorphism ở **phía kernel**.

**Vì sao 4:** nêu đúng **hai tầng polymorphism khác nhau** (C++ và kernel) và **vì sao chúng khác nhau** — cái đi qua ioctl thì đa hình nằm dưới kernel, không nằm ở C++. Đây là chi tiết chỉ người làm thật mới phân biệt. Cộng thêm câu *"app không biết điều này"* — đúng tinh thần DIP.
</details>

---

## Câu 5 — RES-010 · **4 điểm**

**Đề:** Sửa gì trong device tree cho phần cứng mới, và driver làm sao biết chạy cho node nào?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** DTS cho display chứa các **GPIO tự chủ điều khiển được**: bật/tắt panel, kiểm tra TCON PMIC, TCON Vdd, và các chân đặc thù chỉ có trên model riêng — **các chân này còn dùng để định danh thiết bị**, giúp driver/library chọn đúng instance, không chỉ dựa board config. Quy trình mỗi model mới: kiểm GPIO Table mới → xác định đúng vendor và dts → thêm/sửa/xoá GPIO định danh. Cơ chế match: node có property **`compatible`** → kernel tìm đúng driver → match → `probe`.

**Vì sao 4:** trả lời được **cả hai vế** của câu hỏi (sửa gì / match thế nào), và vế đầu là kinh nghiệm thật không thể bịa. Điểm thú vị: dùng **sự hiện diện của GPIO làm dấu hiệu định danh model** — một mẹo thực chiến ngoài sách vở.

**Lưu ý nhỏ khi nói ở phỏng vấn:** phần mô tả GPIO hơi dài trước khi tới `compatible`. Interviewer hỏi *"driver làm sao biết"* thì nên **trả lời `compatible` trước**, rồi mới kể chi tiết GPIO nếu còn thời gian.
</details>

---

## Câu 6 — RES-004 · **4 điểm**

**Đề:** Kernel migration 5.10 → 6.12: cái gì vỡ?

<details><summary>Đã trả lời gì · nhận xét · một chỗ cần chỉnh</summary>

**Đã trả lời:** ① **đổi chữ ký hàm** (`probe`, `device_create`, `class_create`…) — build error lộ hết, sửa dần. ② **hàm bị xoá** — tìm thay thế hoặc bỏ hẳn (`set_fs`). ③ **convention khắt khe hơn**: luôn phải có prototype, biến phải declare ở đầu hàm, cấm biến unused. ④ **giữ tương thích ngược với 5.10** bằng macro `KERNEL_VERSION` để chia case.

**Vì sao 4:** bốn nhóm **phân loại đúng**, có ví dụ cụ thể (`set_fs` — bị gỡ thật ở 5.18 cho hầu hết kiến trúc), và ý ④ là thứ phân biệt người **đã làm** với người **đã đọc**: trong sản phẩm thật luôn phải chạy song song hai đời kernel.

**⚠️ Một chỗ nên chỉnh khi nói thật:** *"biến phải declare ở đầu hàm"* — đây là ràng buộc của **`-Wdeclaration-after-statement`**, kernel đã bật **từ lâu**, **không phải** thứ thay đổi giữa 5.10 và 6.12. Gộp nó vào danh sách "cái vỡ khi migrate" có thể bị hỏi ngược *"cái đó đâu phải mới?"*. Nói an toàn hơn: *"cờ cảnh báo bị siết chặt hơn, `-Werror` bắt nhiều thứ trước đây chỉ warning"* — đúng và không sai chi tiết.
</details>

---

## Câu 7 — RES-006 · **4 điểm** ⭐ câu tốt nhất phiên

**Đề:** Luồng dữ liệu ALS → độ sáng. Làm sao màn không nhấp nháy khi ánh sáng dao động?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** hai cơ chế —
1. **Chu kỳ cố định 60 Hz** bằng thread + condition variable ⇒ dữ liệu đến dồn dập cũng không điều khiển dồn dập, nhịp luôn ổn định.
2. **Không dùng thẳng giá trị đo** mà tính ra **target**; độ sáng hiện tại lưu ở biến static và **cộng dồn từng step** tới khi chạm target ⇒ target đổi đột ngột thì màn vẫn đổi từ từ. Thêm nữa, mỗi chu kỳ **chưa vội tính** — đợi đủ **4 giây** cho sensor ổn định mới đọc và tính target ⇒ loại nhiễu. **Đánh đổi: chậm thích nghi hơn.**

**Vì sao 4 — và vì sao đây là câu mạnh nhất phiên:** ứng viên nêu **ba lớp phòng vệ độc lập** (tách nhịp điều khiển khỏi nhịp dữ liệu · giới hạn tốc độ đổi · cửa sổ ổn định), rồi **tự nêu đánh đổi mà không cần ai hỏi**. Trong `rapid` thì đánh đổi không bị bắt buộc — tự nói ra là điểm cộng thật.

Ba lớp đó có tên chuẩn trong ngành, dùng được nếu muốn nghe "đúng ngôn ngữ": **fixed-cadence control loop** · **slew-rate limiting / ramping** · **debounce (settling window)**. Không cần thiết, nhưng biết thì nói nhanh hơn.
</details>

---

## Câu 8 — RES-005 · **4 điểm**

**Đề:** Vì sao POSIX message queue mà không phải socket hay shared memory?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** độ sáng chỉ là một `uint16_t`. ① **Không phải tự framing từng byte** như socket/pipe/FIFO. ② Dữ liệu **tuần tự FIFO**, từ library (nơi tính độ sáng) đẩy ra app (app lo phần multi-device qua LAN — không thuộc phạm vi mình) ⇒ mq là vừa đủ và đơn giản. ③ **Không có nhu cầu chia sẻ vùng nhớ lớn** để tiết kiệm copy; dùng shm chỉ **tự tạo áp lực race**, **tight coupling** với phía app, phải tự lo đồng bộ, **không có kernel bảo đảm**.

**Vì sao 4:** đây là câu trả lời **đúng bản chất**, không phải học thuộc. Ý ① chạm đúng điểm khác biệt cốt lõi giữa mq và socket — **ranh giới message có sẵn** (liên hệ [LNX-005](../bank/linux-sysprog.md): TCP là luồng byte, không có ranh giới). Ý ③ nêu được **cái giá của shm** thay vì chỉ khen mq.

Một ý có thể thêm nếu bị đào sâu ở phiên khác: mq còn có **ưu tiên message** và **bounded queue** (đầy thì block hoặc `EAGAIN`) — tức backpressure có sẵn, shm không có.
</details>

---

## Câu 9 — RES-008 · **4 điểm**

**Đề:** Kể một ca debug xuyên tầng user–kernel cụ thể.

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** triệu chứng — tắt rồi bật TV thì **màn đen nhưng backlight vẫn sáng**. Các bước: ① reproduce và lấy log trọn khoảng tắt → bật. ② trace từ lúc user bấm tắt → tìm hàm power off ở user space, tới đó là hết (user không cần biết phần cứng) → **trace xuống kernel**, thấy hai thao tác: bật **FRC black pattern** trước, rồi mới tắt **inverter GPIO** của backlight. ③ trace tiếp đường power-on: user space power-on, kernel vẫn hai bước, **inverter GPIO đã ON nhưng KHÔNG tắt FRC black pattern** ⇒ khớp đúng triệu chứng. ④ đọc code: nhánh tắt black pattern **chưa được implement** ở đường power-on cho model 2027. Bổ sung, test, verify.

**Vì sao 4:** cấu trúc chuẩn của một ca debug thật — reproduce → khoanh vùng theo tầng → **so sánh đường đi ngược nhau** → gốc rễ. Điểm mạnh nhất: **so đường off với đường on** để tìm thao tác không đối xứng. Đó là một kỹ thuật tổng quát, không chỉ đúng cho ca này.

**Câu chốt nên dùng ở phỏng vấn:** *"Triệu chứng nằm ở màn hình, nhưng gốc nằm ở chỗ đường bật không đối xứng với đường tắt. Em tìm ra bằng cách đặt hai đường cạnh nhau chứ không phải đọc lần lượt."* — 🎯 **ĐÂY là câu đáng nhớ nhất**, vì [CLAUDE.md §2](../../../CLAUDE.md) ghi debug là **điểm yếu** của ứng viên trong khi resume liệt kê nó là **kỹ năng**. Ca này chính là bằng chứng để bịt khoảng cách đó.
</details>

---

## Câu 10 — RES-009 · **4 điểm**

**Đề:** Con số "giảm ~70% thời gian làm việc" đo bằng cách nào?

<details><summary>Đã trả lời gì · nhận xét</summary>

**Đã trả lời:** dựa trên ước tính thực tế. Mỗi lần submit code cần **5–10 testcase** để đủ bao phủ; trước khi dùng AI mất **~1 ngày**. Sau khi apply AI và cung cấp đủ bối cảnh: **10–15 phút sinh + ~1 giờ review**. Tổng còn **1–2 giờ / 8 giờ**.

**Vì sao 4:** có **đơn vị đo cụ thể** (số testcase / lần submit), **mốc trước–sau**, và **thành thật là ước tính** thay vì bịa ra phương pháp đo. Quan trọng: **vẫn tính giờ review** vào chi phí — người thổi phồng thường quên phần này.

**Một chỉnh nhỏ về cách trình bày:** 1 ngày (8 h) → 1,5 h là **giảm ~81%**, trong khi resume ghi **70%**. Chênh này **có lợi cho bạn** (ghi thận trọng) nhưng nên **nói chủ động** để khỏi bị bắt bẻ:
> *"Đo thô thì ra khoảng 80%, em ghi 70% cho chắc vì không phải task nào cũng hợp để tự động hoá."*

Câu đó biến một chỗ có thể bị soi thành một tín hiệu **trung thực + thận trọng**.
</details>

---

## Câu 11 — RES-011 · **3 điểm**

**Đề:** Kể về Samsung Display Manager — ứng dụng Windows, MVVM.

<details><summary>Đã trả lời gì · nhận xét · phần còn thiếu</summary>

**Đã trả lời:** mở đầu định vị đúng — monitor không OS, chạy firmware, **không liên quan Linux**; là cơ hội học Modern C++ và MVVM trong **1 tháng**, **không phải chuyên môn chính**. Sau đó kể chi tiết hai tính năng: ① lưu app settings vào **JSON** để restore sau restart; ② **Preset** — user lưu/đổi nhanh bộ setting trên monitor qua **VCP protocol**, kèm preview (chỉnh thoải mái, cancel thì rollback), save vào JSON, và tổ chức OOP cho các nhóm preset + preset backup.

**Được:** câu mở đầu **đúng sách** — tự định vị phạm vi, không thổi phồng, không tự hạ thấp (bẫy ③ của bank). Nội dung cụ thể, có tên giao thức thật (VCP).

**Vì sao chưa 4 — không phải vì sai, mà vì PHÂN BỔ THỜI GIAN:** đây là câu trả lời **dài nhất phiên** dành cho **dòng resume ít liên quan nhất** với JD Embedded Linux. Bank ghi thẳng:
> **1. Ngắn** — 30–45 giây. Tính năng Preset, lưu cấu hình bằng JSON, kiến trúc MVVM.

Và phần đáng giá nhất thì **thiếu hẳn** — bank gọi nó là *"phần đáng nói"*:
> **2. Rút ra thứ CHUYỂN ĐƯỢC sang embedded**:
> - **MVVM = tách trạng thái khỏi hiển thị** — cùng tư duy với tách *business logic* khỏi *tầng phần cứng* trong HAL.
> - **Cấu hình bền vững qua khởi động lại** — bài toán y hệt trên thiết bị (lưu setting vào flash, lo mất điện giữa chừng).
> - **Modern C++ (11/14/17)** — trùng đúng yêu cầu "C++17" của JD.

Ứng viên **có sẵn cả ba** trong câu chuyện của mình (MVVM, JSON persist + rollback, C++11/14/17) nhưng **không nối sang embedded**. Đặc biệt tiếc: cơ chế **preview → cancel → rollback** chính là bài toán **atomic config update** — cùng họ với mẫu *ghi tạm → `fsync` → `rename`* ([LNX-007](../bank/linux-sysprog.md)) và với **A/B partition + rollback** trên thiết bị nhúng ([DRV-018](../bank/drivers-embedded.md)).

**Sửa thế nào — rút còn 45 giây, kết bằng một câu bắc cầu:**
> *"Ứng dụng Windows cho monitor, em làm tầng ViewModel: lưu cấu hình bằng JSON để khôi phục sau restart, và tính năng Preset cho phép user thử rồi cancel để quay lại trạng thái cũ. Cái em mang sang được là tư duy tách trạng thái khỏi hiển thị — giống tách business logic khỏi tầng phần cứng trong HAL — và mẫu 'thử rồi rollback' thì chính là bài toán cập nhật cấu hình an toàn trên thiết bị. Mảng em đầu tư sâu vẫn là system software và driver phía Linux."*

**Ôn lại:** [bank RES-011](../bank/resume.md).
</details>

---

## Câu 12 — RES-012 · **2 điểm** 🔴 câu yếu nhất phiên

**Đề:** *"Anh thấy resume em không nhắc Yocto, cũng không thấy I2C/SPI hay PCI/USB. Bên anh dùng những thứ đó khá nhiều."*

<details><summary>Đã trả lời gì · nhận xét đầy đủ · phần còn thiếu</summary>

**Đã trả lời:** *"Công việc chính của em không sử dụng các kiến thức đó. Tuy nhiên em sẵn sàng học hỏi…"* → Yocto: đã học build distribution chạy trên **BeagleBone Black**, hiểu **bitbake, openembedded-core, poky**. → I2C/SPI: nắm ưu nhược điểm về **tốc độ, số dây, mô hình nhiều thiết bị**. → PCI/USB: *"khá đặc thù em chưa có cơ hội trải nghiệm, biết rằng 2 loại giao thức này là hotplug tự động."*

**Được:** phần **Yocto rất tốt và cụ thể** — có phần cứng thật (BBB), có tên đúng ba thành phần. Đây là bằng chứng, không phải lời hứa. Và ứng viên **không nói dối** về PCI/USB.

**Vì sao chỉ 2 — ba lỗi, xếp theo mức độ nghiêm trọng:**

**① Mở đầu bằng câu phủ định trần trụi.** *"Công việc chính của em không sử dụng các kiến thức đó"* — đúng sự thật nhưng đây là **câu tệ nhất để mở đầu**. Bank ghi công thức ba bước, và bước 1 phải là thừa nhận **gọn rồi đi tiếp ngay**, không đứng lại ở chỗ trống:
> **Công thức ba bước — đừng chối, đừng xin lỗi:**
> **1. Thừa nhận thẳng, không vòng vo.** *"Đúng ạ, em chưa làm Yocto trong sản phẩm thật."* — một câu, không kèm biện hộ.

Câu của ứng viên gom **cả ba** thứ vào một lời phủ định chung ⇒ interviewer nghe thành *"em không có gì trong ba mảng anh vừa hỏi"*, trước khi kịp nghe phần Yocto tốt ở sau.

**② KHÔNG BẮC CẦU — đây là phần bị chấm và bị bỏ trắng.** Bank ghi rõ đây mới là chỗ ăn điểm:
> **2. Bắc cầu sang thứ gần nhất bạn CÓ.** Đây là phần được chấm:
> - **Yocto** ← bạn có **cross-compilation**, **CMake/Makefile**, **build system cho nhiều nền tảng**, **port driver qua nhiều chipset**.
> - **I2C/SPI** ← bạn làm **cảm biến ánh sáng** (gần như chắc chắn qua I2C) và **driver nói chuyện với SoC** qua thanh ghi.
> - **PCI/USB** ← trung thực là chưa; nêu bạn nắm nguyên lý chung (device enumeration, driver model) và học nhanh vì nền driver đã có.

Đau nhất là **I2C**: ứng viên trả lời bằng kiến thức sách vở (*"tốc độ, số dây"*) trong khi **chính họ vừa kể ở câu 5** rằng làm việc với **TCON PMIC, GPIO, thanh ghi SoC**, và **câu 7** là một **cảm biến ánh sáng** — thứ gần như chắc chắn nối bằng I2C. Bằng chứng nằm ngay trong câu chuyện của mình mà không dùng.

**③ Một chỗ nói chưa chính xác:** *"PCI/USB là hotplug tự động."* Tính chất quyết định **không phải hotplug** mà là **tự liệt kê (self-enumeration)** — PCI có **configuration space**, USB có **descriptor**, nên kernel hỏi được *"anh là ai"* và **không cần khai trong device tree**. PCI trên nhúng thường **không** hotplug theo nghĩa cắm rút.

🔴 **Và đây là điểm đáng lo nhất của cả phiên: ứng viên BIẾT điều này.** [DRV-019](../bank/drivers-embedded.md) — *"Vì sao PCI/USB không cần khai trong device tree"* — đã được hỏi ở phiên **17/08** và đạt **4 điểm**, trả lời đúng cả configuration space lẫn descriptor. Một ngày sau, cùng chủ đề nhưng **đóng khung là câu hỏi về resume** thì không truy xuất được.

⇒ **Đây là lỗ hổng DIỄN ĐẠT/TRUY XUẤT, không phải lỗ hổng KIẾN THỨC.** Phân biệt này quan trọng vì hai thứ ôn bằng hai cách khác nhau ([config §4](../config.md)): lỗ hổng kiến thức thì đọc lại tài liệu; lỗ hổng truy xuất thì phải **luyện nói dưới đúng khung câu hỏi đó**.

**Sửa thế nào — dựng sẵn và học thuộc khung, đây là câu chắc chắn bị hỏi:**
> *"Yocto thì em chưa dùng trong sản phẩm, nhưng em có nền cross-compilation và build system — CMake/Makefile cho nhiều nền tảng chipset — nên hiểu vấn đề Yocto giải quyết. Em cũng đã tự dựng một image chạy trên BeagleBone Black, nắm bitbake, OE-core, poky.*
> *I2C/SPI thì em có chạm qua công việc: driver của em nói chuyện với TCON PMIC và cảm biến ánh sáng, và em nắm rõ vì sao I2C cần pull-up, clock stretching gây ra lớp lỗi gì.*
> *PCI/USB thì em thành thật là chưa làm. Em nắm nguyên lý — chúng tự liệt kê qua configuration space và descriptor, nên khác I2C/SPI ở chỗ không cần khai trong device tree. Nền driver và device model em đã có nên em tin là học nhanh."*

**Ôn lại:** [bank RES-012](../bank/resume.md) · [bus-protocols.md](../../../05-drivers-device-tree/bus-protocols.md) · [pci-usb-drivers.md](../../../05-drivers-device-tree/pci-usb-drivers.md) · [yocto.md](../../../06-build-systems/yocto.md)
</details>

---

## Tổng kết

### Điểm mạnh — ba thứ nên giữ nguyên

1. **Kể được kiến trúc từ trên xuống.** Câu 3, 4, 5 nối thành một mạch liền: C++ interface → factory theo board config → ioctl → kernel driver → struct + con trỏ hàm → SoC. Rất ít ứng viên mid-level kể được trọn tầng như vậy.
2. **Tự nêu đánh đổi mà không cần hỏi** (câu 7 — 4 giây ổn định đổi lấy tốc độ thích nghi). Đây là tín hiệu senior rõ nhất trong phiên.
3. **Tự vạch ranh giới phạm vi công việc** (câu 2, 11) — nói rõ cái gì mình làm, cái gì chỉ là API gọi xuống. Trung thực có kiểm soát, không nhận vơ.

### Lỗ hổng ưu tiên

| # | Lỗ hổng | Loại | Việc phải làm |
|---|---|---|---|
| **1** 🔴 | **RES-012 — câu "resume thiếu X"**: mở đầu bằng phủ định, không bắc cầu, nói sai "hotplug" | **DIỄN ĐẠT / truy xuất** (kiến thức đã có — DRV-019 đạt 4 hôm 17/08) | Viết sẵn khung 3 bước, **nói to bấm giờ 60 giây**, lặp tới khi bật ra tự nhiên. Không phải đọc lại tài liệu |
| **2** 🟠 | **Kể project thiếu IMPACT** (câu 1) — kết bằng danh sách công nghệ thay vì kết quả | **Cấu trúc kể chuyện** | Viết lại phần S-Box theo mẫu **vấn đề → giải pháp → kết quả**, đặt impact ở **10 giây đầu** |
| **3** 🟡 | **Phân bổ thời gian sai** (câu 11) — câu dài nhất cho dòng resume ít liên quan nhất | **Diễn đạt** | Rút Windows app còn 45 giây + một câu bắc cầu sang HAL/rollback |

> **Mẫu chung của cả ba:** không có lỗ hổng kiến thức nào. **Cả ba đều là lỗi ĐÓNG GÓI** — biết đủ, nhưng sắp xếp sai thứ tự hoặc không nối được sang thứ người nghe cần. Đây là loại lỗi mà đọc thêm tài liệu **không chữa được**; chỉ luyện nói mới chữa.

### So với hai phiên rapid gần nhất

| Phiên | Điểm | Ghi chú |
|---|---|---|
| 16/08 `rapid` debugging | 3.67 | |
| 17/08 `rapid` drivers-dt | **3.92** | 11/12 câu điểm 4 |
| **18/08 `rapid` resume** | **3.67** | 9/12 câu điểm 4; **1 câu điểm 2 đầu tiên sau 2 phiên sạch** |

Điểm số ngang phiên 16/08, nhưng **hình dạng khác hẳn**: không phải yếu đều, mà **9 câu rất chắc + 1 câu hụt hẳn**. Câu hụt lại đúng câu **chắc chắn bị hỏi** và **dễ sửa nhất** — chỉ cần dựng khung và luyện nói.
