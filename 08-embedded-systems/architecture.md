# Embedded Architecture — SoC, Bus, Memory-Mapped I/O

> **TL;DR**
> - **SoC** (System on Chip) tích hợp CPU + bộ nhớ + peripheral (bus, timer, ADC, GPIO...) trên một chip → nhỏ, tiết kiệm điện, đặc thù embedded.
> - **MCU vs MPU**: MCU (microcontroller) nhỏ, có RAM/flash on-chip, thường chạy bare-metal/RTOS, không MMU. MPU (microprocessor) mạnh hơn, RAM/storage ngoài, có MMU → chạy Linux.
> - **Memory-mapped I/O**: thanh ghi của peripheral được ánh xạ vào không gian địa chỉ bộ nhớ → đọc/ghi địa chỉ = điều khiển phần cứng. Phải dùng `volatile` (và barrier) vì giá trị thay đổi ngoài tầm CPU.
> - **Bus giao tiếp**: UART (đơn giản, 2 dây, async), I2C (2 dây, nhiều slave, địa chỉ), SPI (nhanh, full-duplex, nhiều dây) — chọn theo tốc độ/số chân/số thiết bị.
> - **Interrupt + DMA**: cơ chế để CPU không phải bận chờ/copy — nền tảng hiệu năng & tiết kiệm điện.

---

## 1. SoC — System on Chip

Khác PC (CPU, RAM, chipset, GPU rời nhau), embedded thường dùng **SoC**: tích hợp trên một chip:
- CPU core (thường ARM Cortex-A/-M, RISC-V).
- Bộ nhớ (cache; có thể cả RAM/flash on-chip với MCU).
- **Peripheral controller**: UART, I2C, SPI, USB, Ethernet MAC, GPIO, timer, ADC/DAC, PWM...
- Đôi khi GPU, DSP, NPU, crypto engine.

Lợi ích: nhỏ gọn, ít linh kiện, tiết kiệm điện, rẻ khi sản xuất số lượng lớn. Đây là lý do device tree quan trọng — mô tả các khối tích hợp này cho kernel ([05/device-tree](../05-drivers-device-tree/device-tree.md)).

---

## 2. MCU vs MPU

| | MCU (Microcontroller) | MPU (Microprocessor / Application Processor) |
|--|----------------------|----------------------------------------------|
| Ví dụ | STM32, ESP32, AVR, Cortex-M | i.MX, Snapdragon, Cortex-A, Raspberry Pi |
| Bộ nhớ | RAM/flash **on-chip** (KB–MB) | RAM/storage **ngoài** (MB–GB) |
| MMU | Thường **không** (có MPU bảo vệ) | **Có** → chạy OS đầy đủ |
| Phần mềm | Bare-metal hoặc **RTOS** | **Linux**/Android |
| Điện năng | Rất thấp | Cao hơn |
| Dùng cho | Điều khiển realtime, sensor node | Giao diện, mạng, xử lý nặng |

→ Quyết định kiến trúc phần mềm: MCU thường no-MMU + RTOS/bare-metal; MPU có MMU + Linux. (Xem [rtos-vs-linux.md](rtos-vs-linux.md).)

---

## 3. Memory-Mapped I/O (MMIO) — cách CPU điều khiển phần cứng

Peripheral phơi bày các **thanh ghi (register)** (control, status, data). Hai cách CPU truy cập:
- **Port-mapped I/O** (x86 cũ): không gian I/O riêng, lệnh `in`/`out`.
- **Memory-mapped I/O** (ARM/embedded, phổ biến): thanh ghi peripheral được gán vào **không gian địa chỉ bộ nhớ** → đọc/ghi địa chỉ đó như đọc/ghi RAM, nhưng thực ra điều khiển phần cứng.

**Bản đồ không gian địa chỉ** — mấu chốt của MMIO là *cùng một* không gian địa chỉ chứa cả bộ nhớ thật lẫn thanh ghi phần cứng:

```
0xFFFF_FFFF ┌────────────────────────┐
            │  Cortex-M system regs  │  SCB, NVIC, SysTick…
0xE000_0000 ├────────────────────────┤
            │                        │
            │      (không dùng)      │
            │                        │
0x4002_0000 ├────────────────────────┤ ◄── GPIOA_BASE
            │  GPIOA  ┌────────────┐ │       +0x00 MODER   (chế độ chân)
            │         │ thanh ghi  │ │       +0x14 ODR     (output data)
            │         └────────────┘ │       +0x10 IDR     (input data)
0x4000_0000 ├────────────────────────┤ ◄── vùng PERIPHERAL bắt đầu
            │                        │
0x2000_0000 ├────────────────────────┤ ◄── SRAM (bộ nhớ THẬT)
            │   .data .bss  heap ↓   │
            │            stack ↑     │
0x0800_0000 ├────────────────────────┤ ◄── FLASH (code + hằng số)
            │  vector table, .text   │
0x0000_0000 └────────────────────────┘

  Ghi vào 0x2000_1234  →  đổi một byte trong RAM.        (vô hại)
  Ghi vào 0x4002_0014  →  ĐIỆN ÁP TRÊN CHÂN CHIP ĐỔI.    (có side effect vật lý!)
  Cú pháp C giống hệt nhau — chỉ địa chỉ khác.
```

Đó là lý do compiler **không được phép** đối xử với hai chỗ này như nhau, và cũng là lý do `volatile` tồn tại.

```c
// Cách 1: macro cho từng thanh ghi (đơn giản, hay gặp trong code cũ)
#define GPIO_BASE   0x40020000u
#define GPIO_ODR    (*(volatile uint32_t*)(GPIO_BASE + 0x14))

GPIO_ODR |=  (1u << 5);   // set chân 5 lên cao
GPIO_ODR &= ~(1u << 5);   // kéo chân 5 xuống thấp

// Cách 2: struct đặt tại base — rõ ràng hơn, chuẩn của vendor HAL
typedef struct {
    volatile uint32_t MODER;    // +0x00
    volatile uint32_t OTYPER;   // +0x04
    volatile uint32_t OSPEEDR;  // +0x08
    volatile uint32_t PUPDR;    // +0x0C
    volatile const uint32_t IDR;// +0x10  ⭐ const volatile: thanh ghi CHỈ ĐỌC
    volatile uint32_t ODR;      // +0x14
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef*)0x40020000u)
GPIOA->ODR |= (1u << 5);        // trình biên dịch tự tính offset — khó sai hơn
```

> ⚠️ **`GPIO_ODR |= x` là read-modify-write** — ba thao tác, **không nguyên tử**. Nếu một ISR cũng đụng thanh ghi đó → mất cập nhật. Nhiều SoC vì thế có thanh ghi **BSRR** (set/reset riêng, ghi 1 để tác động) cho phép đổi bit **không cần đọc trước** — dùng nó thay vì RMW khi có thể.

**`volatile` bắt buộc**: giá trị thanh ghi có thể đổi do phần cứng (vd status register), và việc *ghi* có **side effect** (kích hoạt hành động). `volatile` cấm compiler tối ưu bỏ/đổi thứ tự/đọc-cache các truy cập đó. Với thứ tự giữa nhiều thanh ghi và bộ nhớ, cần thêm **memory barrier** (`__DMB`/`dmb`) vì CPU có thể reorder.

> Trên Linux kernel **không** dereference địa chỉ vật lý trực tiếp — phải `ioremap()` để map vào không gian ảo kernel rồi dùng `readl()/writel()` (đã lo barrier & volatile). Xem [05/driver-basics](../05-drivers-device-tree/driver-basics.md).

---

## 4. Các bus giao tiếp phổ biến

| Bus | Số dây | Tốc độ | Đặc điểm | Dùng cho |
|-----|--------|--------|----------|----------|
| **UART** | 2 (TX, RX) | Thấp–TB | Bất đồng bộ, point-to-point, không clock chung | Console/debug, GPS, module |
| **I2C** | 2 (SDA, SCL) | TB | Đồng bộ, **nhiều slave** qua địa chỉ, half-duplex, có ACK | Sensor, EEPROM, RTC |
| **SPI** | 4+ (MOSI, MISO, SCLK, CS) | Cao | Đồng bộ, **full-duplex**, master chọn slave bằng CS | Flash, ADC nhanh, display |
| **CAN** | 2 | TB | Bền vững, đa master, ưu tiên thông điệp | Ô tô, công nghiệp |
| **USB/Ethernet** | — | Cao | Phức tạp, có stack | Kết nối ngoại vi/mạng |

**Lựa chọn nhanh:** ít chân + nhiều thiết bị địa chỉ hóa → **I2C**; cần tốc độ cao, full-duplex → **SPI**; giao tiếp đơn giản point-to-point/console → **UART**.

---

## 5. Interrupt & DMA — không bắt CPU chờ/copy

- **Interrupt** (đã bàn ở [05/driver-basics](../05-drivers-device-tree/driver-basics.md)): peripheral báo sự kiện thay vì CPU polling → tiết kiệm CPU, đáp ứng nhanh, cho phép CPU ngủ tiết kiệm điện giữa các sự kiện.
- **DMA** (Direct Memory Access): một controller chuyên dụng **chuyển dữ liệu giữa peripheral và RAM mà không cần CPU** can thiệp từng byte:

```
 ❌ KHÔNG DMA — CPU khuân từng byte
    ┌─────┐  đọc   ┌────────┐   ghi   ┌─────┐
    │ CPU │◄───────│ UART   │────────►│ RAM │      CPU bận 100% suốt quá trình,
    └─────┘        └────────┘         └─────┘      1000 byte = 1000 vòng lặp

 ✅ CÓ DMA — CPU chỉ ra lệnh rồi đi làm việc khác
    ┌─────┐ ①cấu hình  ┌─────────┐
    │ CPU │───────────►│   DMA   │                 ① CPU: nguồn, đích, số byte, GO
    └─────┘            │controller│                ② DMA tự chuyển, CPU RẢNH
       ▲               └────┬────┘                 ③ xong → DMA bắn ngắt báo CPU
       │ ③ ngắt "xong"      │ ② chuyển thẳng
       │                    ▼
       │              ┌────────┐   ┌─────┐
       └──────────────│ UART   │──►│ RAM │
                      └────────┘   └─────┘
```

  - Quan trọng cho throughput cao (audio, video, network, ADC tốc độ cao).
  - ⚠️ **Cache coherency — cái bẫy lớn nhất của DMA.** DMA đọc/ghi **thẳng RAM**, không đi qua cache của CPU:

```
 Ca 1: DMA GHI vào RAM (peripheral → RAM), CPU đọc sau
   RAM  : [ dữ liệu MỚI do DMA ghi ]
   cache: [ dữ liệu CŨ từ lần đọc trước ]   ← CPU đọc trúng cache → thấy dữ liệu CŨ ❌
   ⟹ trước khi đọc phải INVALIDATE cache (vứt bản cũ, buộc nạp lại từ RAM)

 Ca 2: CPU GHI vào buffer, rồi bảo DMA gửi đi (RAM → peripheral)
   cache: [ dữ liệu MỚI CPU vừa ghi ]  ← còn nằm trong cache, chưa xuống RAM
   RAM  : [ dữ liệu CŨ ]                 ← DMA đọc chỗ này → gửi đi dữ liệu CŨ ❌
   ⟹ trước khi khởi động DMA phải FLUSH (clean) cache xuống RAM
```

  > Quy tắc nhớ: **CPU sắp đọc dữ liệu DMA vừa ghi → invalidate. CPU vừa ghi, DMA sắp đọc → flush.** Trên Linux, API `dma_map_single()/dma_unmap_single()` với đúng **hướng** (`DMA_FROM_DEVICE`/`DMA_TO_DEVICE`) đã lo việc này — và đó là lý do **không được đụng buffer trong lúc đang map** ([BSP-011](../14-prep/mock-interview/bank/bsp.md)).

  - **Alignment & vùng nhớ**: buffer DMA thường phải căn lề theo **cache line** (nếu không, hai biến khác nhau nằm chung một line → flush/invalidate cái này giẫm lên cái kia), và nằm ở vùng nhớ DMA truy cập được (một số SoC có vùng RAM mà DMA controller không với tới).

---

## 6. Đặc thù cần nhớ khi lập trình embedded phần cứng

- **Endianness**: thứ tự byte (little/big) ảnh hưởng khi đọc thanh ghi/giao thức nhị phân nhiều byte.

```c
uint32_t x = 0x12345678;
// little-endian (ARM, x86): byte thấp ở địa chỉ thấp
//   addr+0: 78 | addr+1: 56 | addr+2: 34 | addr+3: 12
// big-endian (mạng, một số MCU):
//   addr+0: 12 | addr+1: 34 | addr+2: 56 | addr+3: 78

bool is_little_endian(void) {
    uint32_t v = 1;
    return *(uint8_t*)&v == 1;      // byte đầu là 01 -> little
}
// ⚠️ Quan trọng khi: nhận gói tin mạng (network byte order = BIG endian -> ntohl),
//    đọc file nhị phân, hoặc truyền struct giữa hai kiến trúc khác endianness.
```

- **Bit manipulation**: thao tác bit là ngôn ngữ hằng ngày khi làm việc với thanh ghi:

```c
#define BIT(n)  (1u << (n))

REG |=  BIT(5);                       // SET bit 5
REG &= ~BIT(5);                       // CLEAR bit 5
REG ^=  BIT(5);                       // TOGGLE bit 5
if (REG & BIT(5))     { /* … */ }     // TEST bit 5

// Ghi một TRƯỜNG nhiều bit (vd 3 bit ở vị trí 4..6) — phải mask rồi mới or:
#define FIELD_MASK   (0x7u << 4)
REG = (REG & ~FIELD_MASK) | ((value & 0x7u) << 4);   // ⚠️ read-modify-write, xem §3

// ⚠️ Dùng 1u (unsigned): 1 << 31 trên int có dấu là UB.
```
- **Datasheet / reference manual**: nguồn chân lý về địa chỉ thanh ghi, ý nghĩa từng bit, timing — kỹ năng đọc datasheet là bắt buộc.
- **Fixed-point vs floating-point**: nhiều MCU không có FPU → dùng số nguyên/fixed-point để tránh phép float chậm.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) SoC là gì và khác gì kiến trúc PC truyền thống?</summary>

SoC (System on Chip) tích hợp CPU, bộ nhớ (cache, đôi khi cả RAM/flash) và nhiều peripheral controller (UART, I2C, SPI, GPIO, timer, USB, Ethernet MAC, đôi khi GPU/DSP/NPU) lên **một chip duy nhất**. Khác với PC truyền thống nơi CPU, RAM, chipset, GPU là các linh kiện rời trên bo mạch. Lợi ích của SoC: nhỏ gọn, ít linh kiện, tiêu thụ điện thấp, chi phí thấp khi sản xuất lớn — rất phù hợp embedded. Hệ quả là phần cứng tích hợp không tự khai báo nên cần device tree để mô tả cho kernel.
</details>

<details><summary>2) Phân biệt MCU và MPU.</summary>

MCU (microcontroller, vd STM32, ESP32, Cortex-M) tích hợp RAM và flash ngay trên chip với dung lượng nhỏ (KB–MB), thường không có MMU (chỉ có MPU bảo vệ vùng nhớ), tiêu thụ điện rất thấp, và chạy bare-metal hoặc RTOS — phù hợp điều khiển realtime, sensor node. MPU (microprocessor/application processor, vd i.MX, Cortex-A, Raspberry Pi) mạnh hơn, dùng RAM và storage ngoài (MB–GB), **có MMU** nên chạy được OS đầy đủ như Linux/Android, tiêu thụ điện cao hơn — phù hợp giao diện, mạng, xử lý nặng. Sự có/không MMU là yếu tố quyết định chạy được Linux hay không.
</details>

<details><summary>3) Memory-mapped I/O là gì? Vì sao phải dùng volatile khi truy cập thanh ghi?</summary>

Memory-mapped I/O là cơ chế trong đó các thanh ghi của peripheral được ánh xạ vào không gian địa chỉ bộ nhớ của CPU; đọc/ghi tại các địa chỉ đó thực chất là đọc trạng thái hoặc điều khiển phần cứng, dù cú pháp giống truy cập bộ nhớ thường. Phải dùng `volatile` vì: (1) giá trị thanh ghi có thể thay đổi do phần cứng ngoài tầm kiểm soát của CPU (vd status register), nên compiler không được cache giá trị đã đọc; (2) hành động ghi có side effect (kích hoạt phần cứng), nên compiler không được loại bỏ hay gộp các lần ghi tưởng như "thừa". `volatile` buộc mỗi truy cập diễn ra đúng như viết. Với thứ tự giữa nhiều truy cập còn cần thêm memory barrier vì CPU có thể reorder.
</details>

<details><summary>4) So sánh UART, I2C, SPI. Khi nào chọn cái nào?</summary>

UART: 2 dây (TX/RX), bất đồng bộ không cần clock chung, point-to-point, đơn giản, tốc độ thấp-trung bình — dùng cho console/debug, GPS, module. I2C: 2 dây (SDA/SCL), đồng bộ, hỗ trợ **nhiều slave** trên cùng bus nhờ địa chỉ, half-duplex, có cơ chế ACK, tốc độ trung bình — dùng cho sensor, EEPROM, RTC. SPI: 4+ dây (MOSI/MISO/SCLK/CS), đồng bộ, **full-duplex**, tốc độ cao, master chọn slave bằng chân CS riêng — dùng cho flash, ADC nhanh, màn hình. Chọn: cần ít chân và nhiều thiết bị địa chỉ hóa → I2C; cần tốc độ cao và full-duplex → SPI; giao tiếp point-to-point đơn giản hoặc console → UART.
</details>

<details><summary>5) DMA là gì? Lợi ích và vấn đề cần lưu ý?</summary>

DMA (Direct Memory Access) là cơ chế dùng một controller chuyên dụng để chuyển dữ liệu giữa peripheral và RAM (hoặc RAM–RAM) **mà không cần CPU sao chép từng byte**: CPU chỉ cấu hình nguồn, đích, kích thước rồi làm việc khác, DMA phát interrupt khi hoàn tất. Lợi ích: giải phóng CPU, tăng throughput lớn (audio, video, mạng, ADC tốc độ cao) và tiết kiệm điện. Vấn đề cần lưu ý: **cache coherency** — DMA ghi thẳng RAM không qua cache CPU, nên phải flush/invalidate cache đúng lúc để CPU và DMA thấy dữ liệu nhất quán; ngoài ra buffer DMA thường yêu cầu căn lề và đặt ở vùng nhớ phù hợp (non-cacheable hoặc được quản lý cache cẩn thận).
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [boot-process.md](boot-process.md)
