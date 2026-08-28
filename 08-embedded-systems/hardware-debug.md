# Debug phần cứng embedded (JTAG/SWD, semihosting, không-debugger)

> **TL;DR**
> - **JTAG/SWD** là đường vào debug ở mức phần cứng — dừng CPU, đọc thanh ghi, nạp flash. SWD ít chân hơn, là chuẩn thực tế trên Cortex-M.
> - Câu hỏi thật của phỏng vấn không phải *"biết JTAG không"* mà **"không có debugger thì bạn làm gì"**.
> - **`printf` qua UART** là kênh kinh điển nhưng **chậm và không reentrant** — nó **đổi timing**, nên vô dụng (và có hại) với bug real-time.
> - ⭐ **GPIO + oscilloscope = "printf bằng chân"**: bật/tắt một chân để đánh dấu điểm code. Đây là cách **duy nhất đo được worst-case latency thật** mà không làm nhiễu hệ.
> - **Giữ trạng thái qua reset** (`.noinit` / backup register) là cách lấy bằng chứng cho crash ngoài hiện trường — ⚠️ linker script phải khai vùng đó **ngoài** phần bị zero lúc startup.
> - Chọn công cụ **theo loại bug**: logic → debugger · timing → GPIO+scope · crash → fault register in ra UART · lỗi giao tiếp → logic analyzer đọc bus thật.
>
> Bổ trợ [interrupts-bare-metal.md](interrupts-bare-metal.md) (hard fault), [09-debugging](../09-debugging/).
---

## 1. JTAG vs SWD — giao diện debug phần cứng

Cả hai cho phép **nạp firmware + debug in-circuit** qua probe (ST-Link, J-Link, CMSIS-DAP): đặt breakpoint, đọc/ghi thanh ghi & bộ nhớ, single-step, xem trạng thái CPU khi chạy/dừng — **debug mức thanh ghi thật**, không phải mô phỏng.

| | JTAG | SWD |
|---|---|---|
| Số dây | nhiều (TCK/TMS/TDI/TDO/TRST) | **2** (SWCLK + SWDIO) |
| Chuẩn | cũ, đa dụng (boundary scan, chuỗi nhiều chip) | ARM, tối giản cho debug |
| Dùng | mọi kiến trúc | phổ biến trên **Cortex-M** (tiết kiệm chân) |
| Trace | qua TDO | kèm **SWO** (ITM `printf` trace) |

SWD làm gần hết việc JTAG cho debug với chỉ 2 chân → hợp MCU nhỏ. Boundary scan (test kết nối chân trên PCB) là thứ JTAG có mà SWD không.

## 2. Debug khi KHÔNG có debugger đầy đủ trên target

Nhiều tình huống (sản phẩm ngoài field, không gắn probe được, bug timing) không dùng được debugger — cần công cụ khác, **chọn cái ít làm nhiễu timing nhất**:

- **printf qua UART** — kênh log kinh điển: in trạng thái/biến ra serial console. Cẩn thận: printf **chậm + không reentrant** → không gọi trong ISR/hot path; dùng buffer + gửi ngoài ISR. Với hệ timing chặt, printf có thể *làm đổi hành vi* (che bug).
- **Semihosting / SWO(ITM)** — in qua probe debug không cần UART riêng; chậm, chỉ khi có debugger gắn.
- **GPIO / LED toggle = "printf bằng chân"** — bật/tắt chân đánh dấu điểm code, đo bằng **logic analyzer / oscilloscope** → thấy **timing thật** mà **không làm chậm** như printf. Vũ khí số 1 cho bug timing.

```c
// Chi phí: 1–2 chu kỳ CPU (dùng thanh ghi BSRR, KHÔNG read-modify-write)
#define DBG_PIN_HIGH()  (GPIOB->BSRR = (1u << 3))         // ~4 ns @ 168MHz
#define DBG_PIN_LOW()   (GPIOB->BSRR = (1u << (3 + 16)))
// so với printf("in ISR\n") qua UART 115200: ~700 µs  →  chậm hơn 175.000 LẦN

void ADC_IRQHandler(void) {
    DBG_PIN_HIGH();            // ← chân lên khi ISR BẮT ĐẦU
    process_sample(ADC->DR);
    DBG_PIN_LOW();             // ← chân xuống khi ISR KẾT THÚC
}
```

Trên scope/logic analyzer đọc ra trực tiếp cả hai con số quan trọng nhất:

```
  tín hiệu kích (sensor/timer)
        │           │           │
        ▼           ▼           ▼
  ──────┐     ──────┐     ──────┐
        └───────────┴───────────┴──────────
  DBG_PIN
        ┌──┐        ┌──┐        ┌────────┐
  ──────┘  └────────┘  └────────┘        └──
        │◄►│                    │◄──────►│
      ① latency              ② thời gian ISR chạy
      (sự kiện → ISR bắt đầu)   (đo được cả JITTER: so sánh các lần)
```

> Đây là cách **duy nhất** đo được worst-case latency thật ([interrupts-bare-metal.md §4](interrupts-bare-metal.md)): chạy vài giờ, để scope bắt giá trị **max**, không phải trung bình. Debugger không làm được vì dừng CPU là phá luôn cái đang đo.
- **Logic analyzer / scope** — xem tín hiệu bus thật (I2C/SPI/UART) để tách **lỗi phần mềm vs phần cứng** (câu kinh điển "driver chạy nhưng chân không ra tín hiệu" → đo chân).
- **Trạng thái giữ qua reset** — ghi mã lỗi vào **backup register / vùng RAM không bị zero** để đọc sau khi crash/reset. Đây là cách lấy được bằng chứng từ thiết bị **ngoài field** nơi không ai cắm probe:

```c
// Vùng RAM khai riêng trong linker script, KHÔNG nằm trong .bss
// -> startup không zero nó -> nội dung SỐNG SÓT qua reset mềm
__attribute__((section(".noinit"))) volatile struct {
    uint32_t magic;          // nhận biết dữ liệu hợp lệ (không phải rác lúc mới cấp nguồn)
    uint32_t fault_pc;
    uint32_t fault_lr;
    uint32_t cfsr;
    uint32_t reset_count;
} crash_log;

#define CRASH_MAGIC 0xC0FFEE00u

void HardFault_Handler(void) {
    uint32_t *sp = /* … chọn MSP/PSP, xem interrupts-bare-metal.md §6 … */;
    crash_log.magic     = CRASH_MAGIC;
    crash_log.fault_pc  = sp[6];
    crash_log.fault_lr  = sp[5];
    crash_log.cfsr      = SCB->CFSR;
    crash_log.reset_count++;
    NVIC_SystemReset();                    // ✅ reboot để thiết bị tiếp tục phục vụ
}

void main(void) {
    if (crash_log.magic == CRASH_MAGIC) {  // lần boot SAU khi crash
        report_to_server(crash_log.fault_pc, crash_log.fault_lr, crash_log.cfsr);
        crash_log.magic = 0;               // đánh dấu đã xử lý
    }
    // …
}
```

> ⚠️ Linker script phải khai section `.noinit` **ngoài** vùng bị zero, và startup **không được** đụng vào. Trên STM32 còn có **backup register** (`RTC->BKPxR`) sống sót cả khi mất nguồn chính nếu có pin VBAT.

## 3. Chiến lược theo loại bug

- **Bug logic tái hiện được** → debugger (SWD) + breakpoint/watch.
- **Bug timing / real-time** → **GPIO + scope** (đo, không dừng); tránh printf làm đổi timing.
- **Crash / hard fault** → `HardFault_Handler` in fault registers + stacked PC/LR ra UART (xem [interrupts-bare-metal.md §6](interrupts-bare-metal.md)); map PC bằng `addr2line`/map file.
- **Lỗi giao tiếp thiết bị** → logic analyzer đọc bus, đối chiếu datasheet (đúng địa chỉ? đúng timing? pull-up?).
- **Crash hiếm ngoài field** → log có cấu trúc + mã lỗi giữ qua reset + watchdog restart nhưng giữ bằng chứng trước khi restart.

---

## Câu hỏi phỏng vấn liên quan

| ID | Câu hỏi |
|----|---------|
| [EMB-030](../14-prep/mock-interview/bank/embedded-fundamentals.md) | JTAG và SWD khác nhau, dùng để làm gì |
| [EMB-031](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Không có debugger đầy đủ thì debug firmware thế nào |
| [EMB-032](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Hard fault trên Cortex-M — điều tra thế nào |
| [DBG-014](../14-prep/mock-interview/bank/debugging.md) | Thiết bị ngoài hiện trường tự khởi động lại vài ngày/lần |
| [DBG-017](../14-prep/mock-interview/bank/debugging.md) | Các lệnh GDB cốt lõi |

⬅️ [Về 08-embedded-systems](README.md)
