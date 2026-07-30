# Bộ nhớ bare-metal & khởi động (startup / linker)

> Bản đồ bộ nhớ của một chương trình bare-metal, chuyện gì xảy ra **trước `main()`**, linker script, và rủi ro stack/heap khi **không có MMU**. Bổ trợ cho [boot-process.md](boot-process.md) (boot *Linux*) — file này là startup **MCU**.
> Ôn dạng phỏng vấn: bank [EMB-005…008](../15_prep/mock-interview/bank/embedded-fundamentals.md).

---

## 1. Các section: `.text` / `.rodata` / `.data` / `.bss`

Chương trình được chia thành các section, mỗi cái chứa loại dữ liệu khác nhau và **nằm ở đâu (flash/RAM)** khác nhau:

| Section | Chứa gì | Lưu ở | Chiếm RAM? |
|---|---|---|---|
| `.text` | mã lệnh | **flash** | không (chạy XIP từ flash, hoặc copy vào RAM) |
| `.rodata` | hằng số / chuỗi chỉ đọc (`const`) | **flash** | không |
| `.data` | biến global/static **khởi tạo ≠ 0** | *giá trị* ở flash, *biến* ở **RAM** | có |
| `.bss` | biến global/static **= 0 / chưa khởi tạo** | không lưu giá trị | có (chỉ RAM) |

```c
const char msg[] = "hi";   // .rodata (flash)
int counter = 5;           // .data  (giá trị 5 ở flash, biến ở RAM)
int buffer[256];           // .bss   (256*4 byte RAM, không tốn flash)
static int state;          // .bss
```

Hiểu bản đồ này để: đọc **map file**, tính footprint flash/RAM riêng biệt, và biết **vì sao biến global chưa khởi tạo vẫn là 0** (nhờ startup zero `.bss`). Stack và heap cũng ở RAM.

## 2. Chuyện gì xảy ra TRƯỚC `main()` (startup / crt0)

Sau reset, CPU (Cortex-M) nạp **stack pointer** và **reset vector** từ đầu vector table (flash), rồi chạy **reset handler** (startup code, asm/crt0):

1. Set clock/PLL tối thiểu (thường trong `SystemInit`).
2. **Copy `.data`** từ flash → RAM (biến khởi tạo có giá trị đúng).
3. **Zero `.bss`** (biến chưa khởi tạo = 0).
4. Khởi tạo C runtime (gọi **constructor C++ toàn cục** nếu có — `__libc_init_array`).
5. Gọi `main()`.

```c
// khung reset handler (rút gọn) — dùng symbol do linker script định nghĩa
void Reset_Handler(void) {
    // 2) copy .data: từ _sidata (flash) sang [_sdata, _edata) (RAM)
    for (uint32_t *s=&_sidata, *d=&_sdata; d<&_edata; ) *d++ = *s++;
    // 3) zero .bss: [_sbss, _ebss)
    for (uint32_t *d=&_sbss; d<&_ebss; ) *d++ = 0;
    SystemInit();
    __libc_init_array();     // 4) global ctors
    main();                  // 5)
    while (1) {}              // main không được return trên bare-metal
}
```

Nếu bỏ bước copy `.data`/zero `.bss` → biến global có **giá trị rác** → bug "chạy được bản này, hỏng bản kia". Đây cũng là lý do `main()` không bao giờ return (thường `while(1)`).

## 3. Linker script

Linker script mô tả **bản đồ bộ nhớ vật lý** và **đặt section vào đâu**:

```ld
MEMORY {
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}
SECTIONS {
  .text   : { *(.isr_vector) *(.text*) *(.rodata*) } > FLASH
  _sidata = LOADADDR(.data);
  .data   : { _sdata = .; *(.data*) _edata = .; } > RAM AT> FLASH
  .bss    : { _sbss = .; *(.bss*) _ebss = .; } > RAM
  /* stack đặt ở đỉnh RAM; symbol cho startup dùng */
}
```

Vai trò: đặt `.text/.rodata` → FLASH, `.data`/`.bss`/stack → RAM; đặt **vector table** đúng địa chỉ reset; định nghĩa symbol (`_sdata`, `_edata`, `_sbss`, `_ebss`, `_sidata`) mà startup dùng để copy/zero. Sai linker script → link tràn vùng nhớ / đặt sai địa chỉ → không boot. Vùng đặc biệt (DMA buffer không cache, bootloader/app tách bank, biến giữ qua reset) → khai section riêng + attribute.

## 4. Stack & heap trên MCU — không MMU

RAM ít + **không MMU** → **không có guard page**:

- **Stack overflow âm thầm**: stack lớn quá đè lên `.bss`/`.data`/heap → corruption khó lần (biến "tự đổi"). Sizing = worst-case call depth + local lớn + **stack ISR** (ngắt lồng).
- **Heap**: fragmentation + `malloc` không tất định → nhiều hệ **cấm heap sau init** (dùng tĩnh/pool).

**Phát hiện stack overflow:**
1. **Stack painting** — điền pattern (0xAA…) lúc init, đo mức cao nhất bị đè (high-water-mark).
2. **Canary** — đặt word chặn cuối stack, kiểm tra định kỳ.
3. **MPU** — đặt vùng cấm ngay dưới stack → fault khi tràn (xem §5).
4. RTOS có hook `stack overflow check`.

## 5. MPU (Memory Protection Unit)

MPU (có trên nhiều Cortex-M) **không phải MMU** — không dịch địa chỉ ảo, nhưng đặt được **quyền truy cập theo vùng** (read/write/execute, privileged/user). Dùng để: bắt **stack overflow** (vùng cấm dưới stack → fault), chặn thực thi từ RAM (chống một số khai thác), cô lập task (task chỉ truy cập vùng của nó → lỗi một task không phá task khác), bảo vệ vùng code/vector. Vi phạm → **MemManage fault**. Nhẹ hơn MMU nhưng đủ tăng độ tin cậy/an toàn cho hệ nhúng.

---

## Ôn tập (bank)

[EMB-005](../15_prep/mock-interview/bank/embedded-fundamentals.md) (sections), [EMB-006](../15_prep/mock-interview/bank/embedded-fundamentals.md) (startup/crt0), [EMB-007](../15_prep/mock-interview/bank/embedded-fundamentals.md) (linker script), [EMB-008](../15_prep/mock-interview/bank/embedded-fundamentals.md) (stack/heap MCU). Liên quan: [constraints.md](constraints.md) (ràng buộc bộ nhớ), [DRV-016 hạn chế heap](../15_prep/mock-interview/bank/drivers-embedded.md).
