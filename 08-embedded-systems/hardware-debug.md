# Debug phần cứng embedded (JTAG/SWD, semihosting, không-debugger)

> Công cụ debug khi làm việc với phần cứng thật: giao diện debug (JTAG/SWD), và cách debug **khi không có debugger đầy đủ** trên target. Bổ trợ cho [09-debugging](../09-debugging/) (debug userspace/kernel) và [interrupts-bare-metal.md](interrupts-bare-metal.md) (hard fault).
> Ôn dạng phỏng vấn: bank [EMB-030…031](../15_prep/mock-interview/bank/embedded-fundamentals.md).

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
- **GPIO / LED toggle = "printf bằng chân"** — bật/tắt chân đánh dấu điểm code, đo bằng **logic analyzer / oscilloscope** → thấy **timing thật** (latency ISR, chu kỳ task) mà **không làm chậm** như printf. Vũ khí số 1 cho bug timing.
- **Logic analyzer / scope** — xem tín hiệu bus thật (I2C/SPI/UART) để tách **lỗi phần mềm vs phần cứng** (câu kinh điển "driver chạy nhưng chân không ra tín hiệu" → đo chân).
- **Trạng thái giữ qua reset** — ghi mã lỗi vào **backup register / vùng RAM không bị zero** để đọc sau khi crash/reset (kết hợp [HardFault_Handler](interrupts-bare-metal.md)).

## 3. Chiến lược theo loại bug

- **Bug logic tái hiện được** → debugger (SWD) + breakpoint/watch.
- **Bug timing / real-time** → **GPIO + scope** (đo, không dừng); tránh printf làm đổi timing.
- **Crash / hard fault** → `HardFault_Handler` in fault registers + stacked PC/LR ra UART (xem [interrupts-bare-metal.md §6](interrupts-bare-metal.md)); map PC bằng `addr2line`/map file.
- **Lỗi giao tiếp thiết bị** → logic analyzer đọc bus, đối chiếu datasheet (đúng địa chỉ? đúng timing? pull-up?).
- **Crash hiếm ngoài field** → log có cấu trúc + mã lỗi giữ qua reset + watchdog restart nhưng giữ bằng chứng trước khi restart.

---

## Ôn tập (bank)

[EMB-030](../15_prep/mock-interview/bank/embedded-fundamentals.md) (JTAG vs SWD), [EMB-031](../15_prep/mock-interview/bank/embedded-fundamentals.md) (debug không-debugger), [EMB-032](../15_prep/mock-interview/bank/embedded-fundamentals.md) (hard fault). Đối chiếu góc Linux/host: [DBG-014 field crash](../15_prep/mock-interview/bank/debugging.md), [DBG-017 GDB](../15_prep/mock-interview/bank/debugging.md).
