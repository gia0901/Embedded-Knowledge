# Bare-metal C — thanh ghi, số học & độ tin cậy

> **TL;DR**
> - Nền C khi **không có OS**: không ai dọn hộ, không có MMU chặn hộ — một dòng sai làm treo cả hệ.
> - **`int` không cố định độ rộng** giữa các trình biên dịch/kiến trúc ⇒ dùng `stdint.h` (`uint8_t`, `uint32_t`). ⚠️ Bẫy **integer promotion**: phép toán trên kiểu nhỏ hơn `int` bị **nâng lên `int`** trước khi tính.
> - **Thanh ghi phần cứng bắt buộc `volatile`** — giá trị đổi ngoài luồng compiler nhìn thấy, và mỗi lần ghi có **side effect**. Map thanh ghi bằng **bitfield/union là không portable** (chuẩn C không quy định thứ tự bit, padding, endianness).
> - **`const` đặt ở flash (`.rodata`)** thay vì ăn RAM — mẹo tiết kiệm RAM rẻ nhất trên MCU.
> - Không có FPU ⇒ **fixed-point**; và mọi so sánh thời gian phải viết theo kiểu **chịu được wrap** của counter.
> - Độ tin cậy: chống integer overflow, **CRC/checksum**, và **MISRA C** khi dự án cần chứng nhận.
>
> Bổ trợ [architecture.md](architecture.md) (MMIO), [constraints.md](constraints.md), [memory-and-startup.md](memory-and-startup.md).
---

## 1. Kiểu số xác định (`stdint.h`) & integer promotion

`int`/`long` có kích thước **phụ thuộc nền tảng** (int 16-bit trên vài MCU, 32-bit chỗ khác). Code giả định kích thước → tràn ngoài dự kiến, struct trao đổi qua wire lệch. Dùng kiểu cố định:

```c
#include <stdint.h>
uint8_t  reg8;      // luôn 8-bit
uint32_t addr;      // luôn 32-bit
int16_t  temp;
uintptr_t p;        // đủ chứa một con trỏ
```

**Bẫy integer promotion:** phép toán trên kiểu nhỏ hơn `int` được **nâng lên `int`** trước khi tính:

```c
uint8_t a = 0xFF, b = 0x01;
uint8_t c = a + b;              // a+b tính ở int (=256), rồi cắt về uint8_t -> 0
if ((a + b) > 255) { /* ... */ } // đúng: so sánh ở int, không bị cắt
uint8_t x = 200; x = x << 2;    // 200<<2 = 800 ở int, cắt còn 8-bit -> mất bit cao
```

→ Dùng unsigned cho thao tác bit; ép kiểu tường minh khi cần giữ/cắt; cẩn thận trộn signed/unsigned.

## 2. Bit manipulation

```c
#define BIT(n)   (1u << (n))
REG |=  BIT(3);                 // set bit 3
REG &= ~BIT(3);                 // clear bit 3
REG ^=  BIT(3);                 // toggle bit 3
if (REG & BIT(3)) { /* ... */ } // test bit 3

// nhiều bit qua mask (read-modify-write)
REG = (REG & ~(0x3u << 4)) | (val << 4);   // đặt trường 2-bit ở vị trí 4
```

Lưu ý: dùng `1u` (unsigned) — `1 << 31` trên int là **UB** (tràn dấu). **RMW không atomic**: nếu ISR cũng đụng thanh ghi đó cần [critical section](interrupts-bare-metal.md); nhiều SoC có thanh ghi **SET/CLR riêng** (ghi 1 để set/clear từng bit) tránh RMW.

### 2.1 Bài viết tay hay gặp

> Output thật (gcc 11.4, `-std=c11 -Wall -Wextra`). Đếm số bit 1 và kiểm endianness đã có ở [COD-009](../14-prep/mock-interview/bank/coding.md), [COD-010](../14-prep/mock-interview/bank/coding.md); tạo mask `n` bit ở [COD-021](../14-prep/mock-interview/bank/coding.md).

```c
bool     is_pow2(uint32_t x)        { return x && !(x & (x - 1)); }   /* x & (x-1) xoa bit 1 thap nhat */
uint32_t lowest_set(uint32_t x)     { return x & (~x + 1); }          /* = x & -x voi unsigned */
uint8_t  swap_nibbles(uint8_t b)    { return (uint8_t)((b << 4) | (b >> 4)); }
uint8_t  reverse8(uint8_t b) {
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) { r = (uint8_t)((r << 1) | (b & 1u)); b >>= 1; }   /* lay bit thap cua b, day vao r */
    return r;
}
uint32_t bswap32(uint32_t x) {      /* dao thu tu BYTE — doi endianness */
    return  (x >> 24) | ((x >> 8) & 0x0000FF00u) | ((x << 8) & 0x00FF0000u) | (x << 24);
}
```

```
is_pow2: 0=0 1=1 64=1 96=0
lowest_set(0x68)=0x8
swap_nibbles(0xA5)=0x5A
reverse8(0x01)=0x80 reverse8(0xB0)=0x0D
bswap32(0x11223344)=0x44332211  __builtin_bswap32=0x44332211
```

**Đọc / ghi một trường bit** (`width` bit bắt đầu từ `pos`) — dạng thao tác thanh ghi thật hay gặp nhất:

```c
#define FIELD_MASK(pos, width)        (((1u << (width)) - 1u) << (pos))
#define FIELD_GET(reg, pos, width)    (((reg) & FIELD_MASK(pos, width)) >> (pos))
#define FIELD_SET(reg, pos, width, v) \
    ((reg) = ((reg) & ~FIELD_MASK(pos, width)) | (((uint32_t)(v) << (pos)) & FIELD_MASK(pos, width)))
```

```
FIELD_GET(0xFFFF00FF, 4, 4) = 0xF
sau FIELD_SET(reg, 8, 4, 0xA):  0xFFFF0AFF
sau FIELD_SET(reg, 8, 4, 0x1F): 0xFFFF0FFF     <- gia tri qua rong bi CAT, khong lan sang truong ben canh
```

Ba chi tiết phân biệt người làm thật: ① **xoá trường cũ trước** rồi mới OR giá trị mới (chỉ OR thì bit 1 cũ còn nguyên) · ② **AND giá trị với mask** để giá trị quá rộng không phá trường bên cạnh · ③ `width = 32` làm `1u << 32` — **UB** ([COD-021](../14-prep/mock-interview/bank/coding.md)). Kernel có sẵn `GENMASK`, `FIELD_GET`/`FIELD_PREP` cho đúng việc này. Macro này đánh giá `reg` hai lần — đừng truyền biểu thức có tác dụng phụ ([C-022](../14-prep/mock-interview/bank/c-programming.md)).

| Bài | Ý chính một câu |
|---|---|
| Lũy thừa của 2 | `x & (x - 1)` xoá bit 1 thấp nhất; còn 0 nghĩa là chỉ có một bit 1. Nhớ loại `x = 0` |
| Bit 1 thấp nhất | `x & -x` (unsigned) — dùng để duyệt từng bit đang bật |
| Đảo bit | Lấy bit thấp của nguồn, đẩy vào bên phải của đích, lặp `width` lần; nhanh hơn thì bảng tra 16 hoặc 256 phần tử |
| Đổi chỗ nibble / byte | Dịch rồi OR; đổi byte = đổi endianness (`__builtin_bswap32`, `htonl`) |
| Ghi trường bit | Xoá trường → AND giá trị với mask → OR vào; nếu ISR cũng đụng thì cần critical section |

## 3. Truy cập thanh ghi phần cứng

Map bằng con trỏ tới `volatile`:

```c
#define GPIOA_ODR (*(volatile uint32_t*)0x48000014u)   // Output Data Register
GPIOA_ODR |= BIT(5);   // bật chân PA5

// hoặc struct volatile đặt tại base (rõ ràng hơn):
typedef struct { volatile uint32_t MODER, OTYPER, /*...*/ ODR; } GPIO_TypeDef;
#define GPIOA ((GPIO_TypeDef*)0x48000000u)
GPIOA->ODR |= BIT(5);
```

**Vì sao `volatile`:** giá trị thanh ghi đổi ngoài luồng compiler thấy (phần cứng), và ghi có **side effect** → cấm cache/loại bỏ/gộp/reorder (đọc status 2 lần phải là 2 lần đọc thật). `const volatile` cho thanh ghi chỉ-đọc (status).

**Bitfield/union map thanh ghi — không portable:** chuẩn C không quy định thứ tự bit trong bitfield, padding, endianness → dễ sai khi đổi compiler/kiến trúc. Nhiều coding standard **cấm bitfield cho thanh ghi**, ưu tiên mask + shift tường minh. Ghi thanh ghi khi có DMA/ngắt còn cần đúng **thứ tự** (memory barrier).

## 4. `static` / `const` / `volatile` / `extern`

- **`static`** (file/hàm): linkage nội bộ (đóng gói); **`static` local**: giữ giá trị qua các lần gọi, ở `.data`/`.bss` không phải stack.
- **`const`**: dữ liệu chỉ đọc — trên MCU thường đặt ở **flash (`.rodata`)** thay vì tốn RAM (bảng tra cứu, chuỗi hằng nên `const`).
- **`volatile`**: cấm tối ưu truy cập — thanh ghi, biến bị ISR sửa.
- **`extern`**: khai báo biến/hàm định nghĩa ở TU khác.
- Kết hợp: `const volatile` cho thanh ghi status (cấm ghi + cấm tối ưu đọc).

> Hệ quả khi build **nhiều file** (khai báo vs định nghĩa, `-fno-common` từ gcc 10, `static` trong header, `const` ở phạm vi file khác nhau giữa C và C++): [01/c-language-idioms §2](../01-cpp-fundamentals/c-language-idioms.md).

## 5. Fixed-point vs floating-point (khi không có FPU)

MCU không FPU → float bị **emulate bằng phần mềm** (chậm chục–trăm lần, tốn code) → tránh trong hot path/ISR. **Fixed-point**: biểu diễn số thực bằng số nguyên với "điểm thập phân ảo".

```c
// Q16.16: 32-bit, 16 bit nguyên + 16 bit phân
typedef int32_t q16_16;
#define TO_Q(x)   ((q16_16)((x) * 65536.0))     // đổi (compile-time hằng)
#define Q_ADD(a,b) ((a) + (b))                   // cộng/trừ: nguyên thường
#define Q_MUL(a,b) ((q16_16)(((int64_t)(a) * (b)) >> 16))  // nhân: cần 64-bit trung gian!
```

Đánh đổi: tự quản scale, cẩn thận **overflow khi nhân** (phải mở rộng 64-bit rồi dịch), độ chính xác cố định. Có FPU (Cortex-M4F/M7) hoặc tính ít → float tiện; không FPU + tính nhiều/real-time → fixed-point. Float trên MCU không FPU là **bẫy hiệu năng/tất định** kinh điển.

## 6. Integer overflow / wrap

Số nguyên cố định tràn khi vượt max → quay vòng (unsigned) hoặc **UB (signed)**. Bug hay gặp: bộ đếm ms tràn, trừ hai timestamp ra âm ở unsigned.

**Mẫu so sánh thời gian chịu wrap** (rất quan trọng cho timer):

```c
volatile uint32_t tick_ms;   // tăng trong SysTick ISR, sẽ wrap sau ~49.7 ngày
// SAI: if (tick_ms >= deadline)         -> hỏng quanh thời điểm wrap
// ĐÚNG: so bằng HIỆU, ép signed:
if ((int32_t)(tick_ms - deadline) >= 0) { /* đã tới hạn */ }
```

Phòng: chọn kiểu đủ lớn, so sánh bằng hiệu, kiểm biên trước phép tính, `-fsanitize=undefined` khi test trên host, bật cảnh báo. Signed overflow là **UB** → optimizer có thể làm điều bất ngờ.

## 7. CRC / checksum

Dữ liệu truyền (UART/CAN/RF) hoặc lưu (flash) có thể **hỏng bit** → gắn mã kiểm tra để **phát hiện lỗi**. **Checksum** (cộng byte) rẻ nhưng yếu (không bắt đảo thứ tự, lỗi bù trừ). **CRC** (chia đa thức) bắt lỗi mạnh hơn nhiều (mọi lỗi 1–2 bit, burst ≤ độ dài CRC) — chuẩn cho frame giao thức + xác thực firmware; nhiều MCU có **CRC phần cứng**. Lưu ý: CRC chỉ **phát hiện**, không sửa (muốn sửa cần ECC/FEC), và **không phải bảo mật** (chống sửa cố ý cần chữ ký/HMAC).

## 8. MISRA C

Bộ **coding guideline** cho C trong hệ an toàn/quan trọng (ô tô, y tế) — né vùng không xác định/dễ lỗi của C, tăng portable + audit được. Ví dụ quy tắc: hạn chế cấp phát động sau init; ép kiểu tường minh, không trộn signed/unsigned ngầm; mỗi `switch` có `default`, mỗi `case` có `break`; không dựa vào thứ tự đánh giá. Enforce bằng **static analyzer** (Coverity, PC-lint, cppcheck) trong CI. Đánh đổi: code cứng nhắc hơn, đôi khi phải "deviation" có ghi chú. MISRA là về **giảm rủi ro & audit được**, không phải tối ưu.

---

## Câu hỏi phỏng vấn liên quan

| ID | Câu hỏi |
|----|---------|
| [EMB-001](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Set / clear / toggle / test một bit trong thanh ghi |
| [C-026…029](../14-prep/mock-interview/bank/c-programming.md) | Bài bit viết tay: lũy thừa của 2 · đảo bit · đảo byte · đọc/ghi trường bit |
| [EMB-002](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Vì sao ưu tiên fixed-width types — `int` có rủi ro gì |
| [EMB-003](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Truy cập thanh ghi trong C, vì sao `volatile`, `union`/bitfield |
| [EMB-004](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Vai trò `static` / `const` / `volatile` / `extern` |
| [EMB-024](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Fixed-point vs floating-point khi không có FPU |
| [EMB-025](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Integer overflow — bug embedded kinh điển, phòng thế nào |
| [EMB-026](../14-prep/mock-interview/bank/embedded-fundamentals.md) | CRC / checksum — vì sao cần, khác nhau thế nào |
| [EMB-027](../14-prep/mock-interview/bank/embedded-fundamentals.md) | MISRA C là gì, vì sao dùng |
| [CPP-022](../14-prep/mock-interview/bank/cpp.md) | `volatile` có giúp thread-safe không |
| [CPP-038](../14-prep/mock-interview/bank/cpp.md) | Alignment & padding (góc embedded) |

⬅️ [Về 08-embedded-systems](README.md)
