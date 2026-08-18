# 🎤 Phiên mock · 2026-08-17 · `rapid` · track `drivers-dt`

- **Tham số:** `rapid` · 12 câu · trần T2 (thực tế chỉ chạm **T1**) · Ngày **D1** của [lịch 14 ngày](../../study-plans/datalogic-plan.md)
- **Điểm tổng: 3.92/4** — **cao nhất từ trước tới nay**. 11/12 câu đạt 4.
- **Bối cảnh:** phiên **đầu tiên** chạm domain `DRV` (42 câu, phủ **0%** trước phiên).

## Kết quả từng câu

| # | ID | Câu | Điểm |
|---|----|-----|------|
| 1 | DRV-001 | char / block / network driver | **4** |
| 2 | DRV-003 | major & minor number | **4** |
| 3 | DRV-028 | DTS / DTB / `dtc` | **4** |
| 4 | DRV-007 | device tree giải quyết vấn đề gì | **4** |
| 5 | DRV-029 | `compatible`, vì sao khai nhiều giá trị | **4** |
| 6 | DRV-031 | DT có chứa driver không | **3** |
| 7 | DRV-006 | vì sao không deref con trỏ user | **4** |
| 8 | DRV-008 | MMIO + `volatile` | **4** |
| 9 | DRV-009 | `ioctl` vs `sysfs` | **4** |
| 10 | BUS-002 | khi nào I2C, khi nào SPI | **4** |
| 11 | BUS-005 | vì sao I2C cần pull-up | **4** |
| 12 | DRV-019 | vì sao PCI/USB không cần DT | **4** |

---

## 🔴 Phát hiện quan trọng hơn điểm: **phủ 0% ≠ yếu**

Trước phiên, `DRV` bị xếp **ưu tiên 🔴 số 1** vì phủ 0%. Kết quả: **3.92** — cao hơn hẳn domain đã ôn kỹ (`CPP` phủ 71% ≈ 3.2 · `LNX` phủ 59% = 2.1–2.67).

**Nguyên nhân rõ khi đối chiếu [RESUME.tex](../../../RESUME.tex):** đây là việc ứng viên **làm hằng ngày** (driver HAL, device tree, kernel migration 5.10→6.12). Độ phủ đo *"đã hỏi chưa"*, **không** đo *"có biết không"* — hai thứ đã bị lẫn khi lập bảng ưu tiên 17/08.

⇒ **Sửa plan:** hạ `DRV` khỏi 🔴. D2–D4 rút còn **một `rapid`/ngày**, bỏ `daily` kèm theo. Giờ tiết kiệm dồn sang `BSP` (6%), `SD` (12%), `BEH`/`RES` (0%) — nơi **chưa có bằng chứng nào** về độ vững.

---

## 🔎 Câu 6 · DRV-031 · **3/4** (câu duy nhất không đạt 4)

**Device tree có chứa driver không? Nó khác driver ở chỗ nào?**

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ✅ *"Không, nó là bản mô tả phần cứng, không phải code vận hành."* ✅ nêu đúng lý do tách (mô tả phần cứng vô vàn, gộp vào kernel thì phình). ⚠️ Phần đuôi lòng vòng về driver "có thể viết riêng / include build cùng kernel / bỏ đi khi build", rồi **tự nhận chưa rõ**: *"tại sao device tree luôn luôn rời còn driver thì tuỳ tình huống?"*

**❌ Vì sao 3:** bản chất đúng và bật ra ngay, nhưng đuôi câu trả lời **tự mâu thuẫn** (vừa nói phải tách vì phình, vừa nói driver thì gộp cũng được) mà không giải quyết được mâu thuẫn đó. Thang `rapid` trừ điểm **lan man**.

**Đáp án đầy đủ — đã bổ sung vào bank [DRV-031](../bank/drivers-embedded.md):**

| | Driver | Device tree |
|---|---|---|
| Bản chất | **Code cho một dòng chip** | **Sự thật về một tấm board cụ thể** |
| Tái dùng | ✅ chip nào board nào cũng chung driver | ❌ board A/B **không bao giờ** chung |
| Thừa thì sao | **Vô hại** — không match `compatible` thì không probe | **Boot hỏng** — tìm thiết bị ở địa chỉ trống |

⇒ **Thứ tái dùng được thì gói chung; thứ riêng của từng board thì tách ra.** Kernel chứa driver của 100 chip vẫn chạy mọi board (chỉ cái khớp mới probe); nhưng nhét mô tả board vào kernel thì image **chỉ còn chạy đúng một board** — mất sạch lợi ích DT sinh ra để có.

📌 DT **có thể** nhúng vào image (`CONFIG_ARM_APPENDED_DTB`) — người ta **cố tình tránh**. Không phải *"không thể"* mà là *"không nên"*.

**Lần sau sẽ hỏi (T2):** *"cùng một SoC, hai board khác nhau chỉ khác chân GPIO reset — bạn tách cái gì ra DT, cái gì để trong driver?"*

**Ôn:** [device-tree.md](../../../05-drivers-device-tree/device-tree.md) · bank [DRV-031](../bank/drivers-embedded.md)
</details>

---

## ⚖️ Phán định 6 phàn nàn về chất lượng bank — kiểm bằng `grep`

Ứng viên nêu **6 điểm** trong lúc trả lời. Kết quả: **4 đúng · 2 nửa đúng · 0 sai** — tỉ lệ cao, chứng tỏ đang đọc kỹ và đúng chỗ.

| Câu | Phàn nàn | Phán định | Bằng chứng |
|---|---|---|---|
| DRV-008 | *"dùng ví dụ MCU, chưa rõ kernel viết thế nào"* | ✅ **Đúng** | `grep "ioremap\|readl\|writel"` trong DRV-008 → **0**. Giới hạn ③ có nói bóng gió *"phải ánh xạ đúng cách"* nhưng **không chỉ ra cách** |
| BUS-005 | *"không có hình vẽ minh hoạ"* | ✅ **Đúng** | Chủ đề mà sơ đồ là bắt buộc |
| BUS-005 | *"toàn bằng lời, không giải thích tại sao"* | ❌ **Không đúng** | Bank **có** mục *"Vì sao thiết kế như vậy — hai lý do"* + bảng đánh đổi + 4 bẫy (3.217 ký tự). Thiếu là **con số dòng điện**, không phải thiếu lý do |
| DRV-006 | *"3 việc không rõ bản chất tại sao"* | 🟡 **Nửa đúng** | Bank **liệt kê** đúng 3 việc nhưng không nói **hậu quả nếu thiếu**. Phần bẫy thì tốt (4 bẫy, có atomic-sleep + TOCTOU) |
| BUS-002 | *"chưa thấy nói full/half-duplex"* | 🟡 **Nửa đúng** | Bank chỉ ghi từ khoá trống không — nhưng [bus-protocols.md](../../../05-drivers-device-tree/bus-protocols.md) **có bảng + giải thích MOSI/MISO đồng thời** |
| DRV-019 | *"descriptor là gì? cơ chế?"* | ✅ **Đúng** | Bank dùng chữ "descriptor" như thể đã biết |

### ✅ Đã sửa ngay trong phiên (6 chỗ trong `bank/drivers-embedded.md`)

1. **DRV-008** — thêm hẳn mục *"🐧 Trong KERNEL thì viết thế nào"*: đối chiếu MCU (địa chỉ vật lý dùng thẳng) vs kernel (**có MMU ⇒ phải `ioremap`**), code `devm_platform_ioremap_resource` + `readl/writel`, bảng 4 khác biệt, và điểm đảo ngược thói quen: **trong kernel KHÔNG dùng `volatile` cho thanh ghi** — accessor đã mang theo **memory barrier** mà `volatile` không có. *(Ghi rõ tên hàm là **T3 không chấm**; thứ được chấm là hiểu **vì sao phải map**.)*
2. **DRV-006** — bảng *"thiếu việc này thì chuyện gì xảy ra"*: xác thực địa chỉ ⇒ chặn **leo thang đặc quyền**; page fault an toàn ⇒ chặn **oops** (bảng ngoại lệ); trả số byte chưa chép ⇒ đường thoát `-EFAULT`.
3. **BUS-005** — **sơ đồ ASCII** open-drain + luật **một cặp pull-up cho CẢ BUS** + phép tính `I = 3,3 V / 4,7 kΩ ≈ 0,7 mA`, đối chiếu push-pull chập (hàng trăm mA–vài A).
4. **BUS-002** — bảng full/half-duplex 3 bus + hệ quả *"SPI là **trao đổi**, không phải gửi-rồi-nhận"*.
5. **DRV-019** — định nghĩa descriptor = **căn cước thiết bị tự mang trong ROM** + bảng đối chiếu *"bus nào hỏi được 'anh là ai'"*.
6. **DRV-031** — mục ⭐ vì sao DT luôn rời còn driver thì tuỳ *(xem phần Câu 6 ở trên)*.

---

## Tổng kết

- **Điểm mạnh:** `DRV`/`BUS` là vùng **vững thật** — 11/12 câu bật ra ngay, gọn, đúng, có ví dụ. Đặc biệt DRV-007 (nêu đủ 3 vấn đề thời tiền-DT) và DRV-029 (giải thích đúng thứ tự cụ thể→tổng quát + lý do tương thích ngược).
- **Điểm cần giữ:** phản xạ **chất vấn tài liệu** — 6 phàn nàn thì 4 đúng hoàn toàn. Đây là cách bank dày lên đúng chỗ.
- **Không có câu ≤ 2** ⇒ **không thêm gì vào weak-register**. Lần đầu tiên có phiên như vậy.
- **Câu mới thêm vào bank:** không. **Câu được viết dày thêm:** 6 (xem trên).

## Phiên kế đề xuất

`/mock rapid track bsp` — `BSP` đang **6%** và là trụ JD thứ hai (kernel/Yocto/boot/DMA). Sau kết quả hôm nay, đây mới là chỗ chưa biết độ vững.
