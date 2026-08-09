# Bus giao tiếp ngoại vi — UART · I2C · SPI

> **TL;DR**
> - Ba bus phổ biến nhất nối SoC với ngoại vi. Câu phỏng vấn thực chất luôn là **"chọn cái nào và vì sao"**, không phải đọc thuộc thông số.
> - **UART**: 2 dây, **bất đồng bộ** (không clock chung) → hai bên phải thoả thuận **baud rate**. Điểm-điểm. Console/log.
> - **I2C**: 2 dây **dùng chung**, đa thiết bị theo **địa chỉ 7-bit**, open-drain + pull-up. Ít dây, chậm.
> - **SPI**: 4 dây, đồng bộ, full-duplex, **nhanh nhất**, mỗi slave thêm một chân **CS**.
> - Trục đánh đổi duy nhất cần nhớ: **số dây ↔ số thiết bị ↔ tốc độ**.
> - Ôn dạng phỏng vấn: bank domain `BUS` trong [drivers-embedded.md](../14-prep/mock-interview/bank/drivers-embedded.md).

---

## 1. Bảng so sánh — thuộc bảng này là trả lời được phần lớn câu hỏi

| | **UART** | **I2C** | **SPI** |
|---|---|---|---|
| Số dây | 2 (TX, RX) | 2 (SDA, SCL) | 4 (MOSI, MISO, SCLK, CS) |
| Đồng bộ? | **Bất đồng bộ** (không clock) | Đồng bộ (có clock) | Đồng bộ (có clock) |
| Kiểu | Point-to-point | **Bus đa thiết bị** (địa chỉ) | Master + nhiều slave (chọn bằng CS) |
| Tốc độ | Thấp (~115200 bps phổ biến) | TB (100k / 400k / 1M+ Hz) | **Cao nhất** (chục MHz) |
| Song công | Full-duplex | Half-duplex | Full-duplex |
| Chọn thiết bị | — | Theo **địa chỉ** trên bus | Theo chân **CS** riêng |
| Báo nhận | không (có thể thêm parity) | **ACK/NACK** từng byte | **không có** cơ chế chuẩn |
| Thêm 1 thiết bị tốn | (không mở rộng được) | **0 dây** | **1 dây CS** |
| Dùng điển hình | Console/log, GPS, modem | Sensor, EEPROM, RTC, PMIC | Flash, LCD, ADC tốc độ cao |

---

## 2. UART

**Bản chất:** truyền nối tiếp **bất đồng bộ** — không có dây clock, hai bên phải thoả thuận **baud rate** trước. Khung mỗi ký tự: start bit → data bits → (parity) → stop bit. Hai dây TX/RX **nối chéo** nhau.

```
   Khung UART 8N1 (8 data, No parity, 1 stop), nghỉ = mức CAO

    nghỉ  start │ D0 D1 D2 D3 D4 D5 D6 D7 │ stop   nghỉ
   ─────┐      ┌──┬──┬──┬──┬──┬──┬──┬──┐        ┌──────
        └──────┘  │  │  │  │  │  │  │  └────────┘
         ▲                                  ▲
      cạnh xuống = mốc đồng bộ           trở về CAO
      (receiver tự lấy nhịp từ đây)
```

**Vì sao baud rate sai lại ra ký tự rác:** receiver không có clock từ bên gửi — nó chỉ bắt **cạnh xuống của start bit**, rồi *tự đếm giờ* theo baud rate đã cấu hình để lấy mẫu từng bit. Lệch baud → lấy mẫu trượt dần sang bit kế → giải ra byte khác hẳn. Sai ~2% trở lên là hỏng. Đây là lý do UART cần **thạch anh đủ chính xác**, và là lỗi số một khi cắm console mà chỉ thấy `���`.

**Khi nào dùng:** debug console/log (gần như mọi board embedded đều có), giao tiếp điểm-điểm đơn giản, GPS/modem.

**Hạn chế:** không có địa chỉ → **không mở rộng** nhiều thiết bị trên cùng cặp dây; không có ACK.

**Flow control** (hay bị hỏi tiếp): khi bên nhận xử lý không kịp, dữ liệu **rơi âm thầm** vì không có ACK. Hai cách chặn: **RTS/CTS** (hardware, thêm 2 dây — tin cậy) hoặc **XON/XOFF** (software, chèn ký tự điều khiển vào luồng — không dùng được cho dữ liệu nhị phân).

---

## 3. I2C

**Bản chất:** bus **2 dây dùng chung** (SDA data, SCL clock), **đa thiết bị**: mỗi slave có **địa chỉ 7-bit**. Master phát clock, gọi địa chỉ, rồi đọc/ghi. Cả hai dây là **open-drain** → bắt buộc có **điện trở pull-up**.

```
                        VDD
                         │
                    ┌────┴────┐  ← pull-up (thường 2.2k–10k)
                    R         R
   Master ──┬── SDA ─┴──┬─────────┬─────────┐
            │           │         │         │
            └── SCL ────┴─────────┴─────────┤
                    Slave 0x50  0x68      0x1A
                                (mỗi slave một địa chỉ)
```

**Vì sao phải open-drain + pull-up:** mọi thiết bị chỉ được phép **kéo dây xuống 0**, không ai được đẩy lên 1 (điện trở lo việc đó). Nhờ vậy hai thiết bị nói cùng lúc **không đốt nhau** — và chính đặc tính này cho phép ACK, clock stretching, arbitration hoạt động. Chọn R sai: quá lớn → cạnh lên chậm, sai ở tốc độ cao; quá nhỏ → tốn dòng, slave yếu không kéo nổi xuống mức thấp.

**Bốn khái niệm gần như chắc chắn bị hỏi:**

| Khái niệm | Là gì | Vì sao có |
|---|---|---|
| **START / STOP** | SDA đổi mức **trong khi SCL đang CAO** (bình thường SDA chỉ đổi khi SCL THẤP) | Cần một tín hiệu "khác mọi dữ liệu" để đánh dấu đầu/cuối giao dịch |
| **ACK / NACK** | Sau mỗi byte, bên nhận kéo SDA xuống 0 = ACK; để nổi = NACK | Không ACK khi gọi địa chỉ ⇒ **không có thiết bị đó trên bus** — đây chính là cách `i2cdetect` quét |
| **Clock stretching** | **Slave** giữ SCL ở mức THẤP để bắt master chờ | Slave chậm (ADC đang chuyển đổi, EEPROM đang ghi) cần thêm thời gian. Master phải biết **nhả SCL rồi kiểm tra nó có lên thật không**; master làm bit-bang ẩu thường bỏ qua bước này → treo/hỏng dữ liệu |
| **Arbitration** | Nhiều master cùng phát: ai ghi 1 mà thấy dây là 0 thì **tự rút lui** | Nhờ open-drain, mất quyền được phát hiện mà **không mất dữ liệu** của bên thắng |

**Khi nào dùng:** nối nhiều IC tốc độ thấp với ít dây — sensor nhiệt/ánh sáng, EEPROM, RTC, PMIC.

**Bẫy thực chiến:** **trùng địa chỉ** hai IC cùng loại trên một bus. Cách xử lý: chân cấu hình địa chỉ trên IC (nếu có), hoặc **I2C mux/switch** (PCA954x), hoặc tách sang bus I2C thứ hai.

---

## 4. SPI

**Bản chất:** đồng bộ, **4 dây**, full-duplex, **nhanh nhất**. Master chọn slave bằng chân **CS** riêng; MOSI (master→slave) và MISO (slave→master) chạy **đồng thời** theo SCLK.

```
   Master ── SCLK ──────┬──────────┬─────
          ── MOSI ──────┼──────────┼─────
          ── MISO ──────┼──────────┼─────
          ── CS0 ───────┘          │
          ── CS1 ──────────────────┘
                     Slave0      Slave1     (mỗi slave một dây CS)
```

**CPOL / CPHA — 4 mode:** hai bit cấu hình quyết định **lấy mẫu dữ liệu ở cạnh nào của clock**. Master và slave **phải cùng mode**, lệch là ra dữ liệu rác (thường lệch 1 bit hoặc đọc toàn `0x00`/`0xFF`).

- **CPOL** = mức nghỉ của clock: 0 = nghỉ THẤP, 1 = nghỉ CAO.
- **CPHA** = lấy mẫu ở cạnh **thứ nhất** (0) hay **thứ hai** (1) sau khi CS tích cực.

```
   Mode 0 (CPOL=0, CPHA=0) — phổ biến nhất
   SCLK  ──┐  ┌──┐  ┌──┐  ┌──      nghỉ THẤP
           └──┘  └──┘  └──┘
   MOSI  ══╳═════╳═════╳═════      dữ liệu đổi ở cạnh XUỐNG
           ▲     ▲     ▲
        lấy mẫu ở cạnh LÊN (cạnh thứ nhất)

   Mode 1 (CPOL=0, CPHA=1)
           ┌──┐  ┌──┐  ┌──┐
   SCLK  ──┘  └──┘  └──┘  └──
              ▲     ▲     ▲
        lấy mẫu ở cạnh XUỐNG (cạnh thứ hai)
```

Tra datasheet slave để biết mode; sai mode là lỗi bring-up SPI kinh điển, và **soi bằng logic analyzer** là cách xác định nhanh nhất ([08/hardware-debug](../08-embedded-systems/hardware-debug.md)).

**Khi nào dùng:** cần băng thông cao — flash NOR/NAND, màn hình LCD, ADC nhanh, thẻ SD.

**Hạn chế:** tốn dây (mỗi slave thêm một CS → nhiều slave thì cạn chân GPIO); **không có ACK** → không biết slave có nghe không, phải tự thêm CRC/kiểm tra ở tầng giao thức; không có chuẩn thống nhất về khung lệnh (mỗi chip một kiểu).

---

## 5. Chọn bus thế nào — câu tổng hợp hay hỏi

> **UART** khi cần kênh nối tiếp đơn giản điểm-điểm như console/log. **I2C** khi cần nối *nhiều* thiết bị tốc độ thấp với *ít dây*, chấp nhận tốc độ vừa phải (sensor, EEPROM, RTC). **SPI** khi cần *tốc độ cao*, full-duplex và chấp nhận tốn dây (flash, màn hình). Tóm lại là đánh đổi giữa **số dây, số thiết bị và tốc độ**.

Ba câu hỏi ngược nên đặt trước khi chốt: **bao nhiêu thiết bị?** (>1 mà ít chân → I2C) · **cần bao nhiêu băng thông?** (MB/s → SPI) · **có ràng buộc chân GPIO không?** (cạn chân → I2C hoặc SPI + mux).

---

## 6. Góc Linux — bus này lộ ra ở đâu

| | Trong device tree | Trong driver | Công cụ soi từ userspace |
|---|---|---|---|
| I2C | node con của `&i2c1`, `reg = <0x50>` = địa chỉ slave | `i2c_driver`, `i2c_transfer()` | `i2cdetect -y 1`, `i2cget`, `i2cdump` |
| SPI | node con của `&spi0`, `reg` = **chỉ số CS**, `spi-max-frequency`, `spi-cpol`/`spi-cpha` | `spi_driver`, `spi_sync()` | `spidev` + `spi-tools` |
| UART | `&uart0` + `stdout-path` cho console | tty/serial core | `stty`, `minicom`, `picocom` |

Điểm nối quan trọng: **`reg` trong device tree mang nghĩa khác nhau tuỳ bus** — với I2C là **địa chỉ 7-bit trên bus**, với SPI là **chỉ số chân CS**. Đây là chỗ hay nhầm khi viết DT lần đầu ([device-tree.md](device-tree.md)).

Khi thiết bị "không lên": thứ tự kiểm tra là **DT có node chưa → driver có probe không (`dmesg`) → đo chân bằng logic analyzer**. Bước cuối tách bạch được *lỗi phần mềm* với *lỗi phần cứng/nối dây*, và là thứ phân biệt người từng bring-up board thật ([08/hardware-debug](../08-embedded-systems/hardware-debug.md)).

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) So sánh UART, I2C, SPI — khi nào chọn cái nào?</summary>

**UART** bất đồng bộ, 2 dây, điểm-điểm: không có clock chung nên hai bên phải thoả thuận baud rate; dùng cho console/log, GPS, modem. Không mở rộng nhiều thiết bị vì không có địa chỉ.

**I2C** đồng bộ, 2 dây **dùng chung**, đa thiết bị theo địa chỉ 7-bit, open-drain + pull-up, có ACK/NACK từng byte. Tốc độ trung bình (100k–1M Hz), half-duplex. Dùng khi cần nối *nhiều* IC tốc độ thấp mà *ít dây*: sensor, EEPROM, RTC, PMIC. Thêm thiết bị **không tốn thêm dây**.

**SPI** đồng bộ, 4 dây, full-duplex, nhanh nhất (chục MHz), mỗi slave một chân CS. Dùng khi cần băng thông cao: flash, LCD, ADC nhanh. Không có ACK, và mỗi slave thêm một chân.

Trục đánh đổi: **số dây ↔ số thiết bị ↔ tốc độ**. Câu chốt: I2C tiết kiệm chân, SPI tiết kiệm thời gian, UART tiết kiệm công sức.
</details>

<details><summary>2) Vì sao I2C bắt buộc có điện trở pull-up?</summary>

Vì SDA/SCL là **open-drain**: thiết bị chỉ có transistor kéo dây **xuống 0**, không ai đẩy lên 1 — điện trở pull-up làm việc đó. Hệ quả trực tiếp: hai thiết bị phát cùng lúc **không gây xung đột điện** (một bên kéo xuống, bên kia thả nổi, dây ra mức thấp), thay vì đấu ngắn nguồn với đất như push-pull.

Chính đặc tính này là nền cho ba cơ chế của I2C: **ACK** (bên nhận kéo SDA xuống), **clock stretching** (slave giữ SCL thấp để bắt master chờ), và **arbitration** (master ghi 1 mà đọc lại thấy 0 thì biết mình thua và rút lui, không mất dữ liệu bên thắng).

Chọn giá trị: quá lớn → cạnh lên chậm (RC), sai ở 400 kHz trở lên; quá nhỏ → tốn dòng và slave có thể không kéo nổi xuống mức thấp hợp lệ. Thường 2.2k–10k tuỳ tốc độ và điện dung bus.
</details>

<details><summary>3) Clock stretching là gì? Nó gây ra lỗi kiểu nào?</summary>

Là việc **slave** chủ động giữ SCL ở mức THẤP để buộc master chờ, khi nó chưa sẵn sàng — ví dụ ADC đang chuyển đổi, hoặc EEPROM đang ghi vào ô nhớ. Vì SCL là open-drain nên slave "giữ" được clock dù master mới là bên phát.

Master đúng chuẩn phải: nhả SCL lên, **rồi đọc lại xem SCL có thật sự lên mức cao chưa**; nếu chưa thì đợi. Master bit-bang tự viết hay bỏ bước kiểm tra này và cứ đếm giờ mà chạy tiếp → đọc trúng lúc slave chưa sẵn sàng → **dữ liệu rác hoặc treo bus**. Một số controller I2C phần cứng cũng hỗ trợ thiếu hoặc có erratum ở phần này, nên datasheet SoC hay ghi rõ.

Triệu chứng điển hình: đọc sensor lúc được lúc không, hoặc bus kẹt ở mức thấp. Cách xác nhận nhanh nhất là **soi SCL bằng logic analyzer** — thấy clock bị kéo dài bất thường là đúng nó.
</details>

<details><summary>4) CPOL/CPHA là gì? Sai thì hiện tượng ra sao?</summary>

Hai bit cấu hình quyết định quan hệ giữa clock và dữ liệu trong SPI, tạo thành 4 mode (0–3). **CPOL** = mức nghỉ của clock (0: nghỉ thấp, 1: nghỉ cao). **CPHA** = dữ liệu được lấy mẫu ở cạnh **thứ nhất** (0) hay **thứ hai** (1) sau khi CS tích cực.

Master và slave **bắt buộc cùng mode**. Sai mode nghĩa là master lấy mẫu ở đúng lúc dữ liệu đang chuyển tiếp → đọc ra rác: thường là **lệch một bit** (giá trị bị nhân/chia đôi so với kỳ vọng) hoặc toàn `0x00`/`0xFF`.

Trong Linux, mode khai trong device tree bằng `spi-cpol`/`spi-cpha` (hoặc trong `spi_device->mode`). Cách xác định nhanh khi bring-up: đọc datasheet slave, và nếu vẫn nghi thì **bắt SCLK + MOSI + CS bằng logic analyzer** rồi so cạnh — nhanh hơn thử lần lượt 4 mode.
</details>

<details><summary>5) Thiết bị I2C không phản hồi — bạn debug thế nào?</summary>

Đi từ ngoài vào, mỗi bước loại bỏ một tầng:

1. **`i2cdetect -y <bus>`** — nếu địa chỉ không hiện, nghĩa là gọi địa chỉ không ai ACK. Vấn đề ở tầng điện/địa chỉ, chưa cần đụng driver.
2. **Nguồn và chân**: IC đã được cấp nguồn chưa (regulator/PMIC đã bật?), chân có đúng là bus đó không (pinmux trong device tree), có **pull-up** không.
3. **Địa chỉ**: datasheet ghi địa chỉ 8-bit hay 7-bit? Linux dùng **7-bit** — nhầm chỗ này làm lệch một bit là chuyện rất hay xảy ra. Có IC nào **trùng địa chỉ** trên cùng bus không.
4. **Logic analyzer trên SDA/SCL**: có thấy START + byte địa chỉ không? Có ACK không? SCL có bị **kéo thấp kéo dài** (clock stretching / bus kẹt) không? Cạnh lên có bị bo tròn (pull-up quá lớn) không?
5. **Tầng phần mềm**: node device tree đã có chưa, driver đã `probe` chưa (`dmesg`), có `EPROBE_DEFER` vì regulator/clock chưa sẵn sàng không.

Điểm cần thể hiện: **tách lỗi phần mềm khỏi lỗi phần cứng càng sớm càng tốt** — `i2cdetect` và logic analyzer làm được việc đó trong vài phút, thay vì đọc code driver hàng giờ.
</details>

---
⬅️ [Về index topic](README.md) · Liên quan: [device-tree.md](device-tree.md) · [driver-basics.md](driver-basics.md) · [08/hardware-debug.md](../08-embedded-systems/hardware-debug.md)
