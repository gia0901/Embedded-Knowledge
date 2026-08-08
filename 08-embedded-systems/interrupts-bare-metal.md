# Ngắt bare-metal (ISR, vector table, chia sẻ dữ liệu, hard fault)

> Ngắt trên MCU **không có OS**: quy tắc viết ISR, vector table/NVIC, latency, **chia sẻ dữ liệu ISR ↔ main an toàn**, reentrancy, và điều tra hard fault. Bổ trợ cho [architecture.md §5](architecture.md) (interrupt & DMA tổng quan).
> Ôn dạng phỏng vấn: bank [EMB-009…013](../15_prep/mock-interview/bank/embedded-fundamentals.md), [EMB-032](../15_prep/mock-interview/bank/embedded-fundamentals.md).

---

## 1. ISR là gì & quy tắc viết

ISR (Interrupt Service Routine) = hàm chạy khi ngắt xảy ra, ngoài luồng chính. Quy tắc:

- **Ngắn và nhanh** — giữ latency cho ngắt khác.
- **Không blocking / không ngủ / không chờ.**
- **Không `malloc` / `printf` / hàm không reentrant** (giữ khóa nội bộ → deadlock/corruption).
- **Không việc nặng** — chỉ ack thiết bị, đọc/ghi thanh ghi tối thiểu, đặt cờ / đẩy dữ liệu vào buffer, rồi **báo main loop / task xử lý** (deferred).
- **Xóa cờ ngắt đúng chỗ** để không bị gọi lại vô hạn.
- Biến chia sẻ với main phải `volatile` (§3).

```c
volatile uint16_t adc_sample;
volatile bool      adc_ready;

void ADC_IRQHandler(void) {
    adc_sample = ADC->DR;     // đọc thanh ghi (cũng xóa cờ)
    adc_ready  = true;        // báo main loop
}                              // xử lý nặng để main làm
```

## 2. Vector table & NVIC

**Vector table** = mảng **địa chỉ handler**, đặt ở đầu flash (địa chỉ 0x0000_0000 sau reset trên Cortex-M). CPU không "tìm" ISR — nó lấy **chỉ số ngắt × 4** làm offset, đọc địa chỉ tại đó, rồi nhảy thẳng vào. Vì thế phân phối ngắt là **O(1) và tất định**:

```
Địa chỉ      Nội dung (4 byte mỗi entry)
0x0000_0000  ┌──────────────────────────┐
             │ Initial SP  (giá trị nạp │  ← KHÔNG phải handler!
             │ vào MSP khi reset)       │     Entry 0 là stack pointer.
0x0000_0004  ├──────────────────────────┤
             │ Reset_Handler            │  ← lệnh đầu tiên chạy sau reset
0x0000_0008  ├──────────────────────────┤
             │ NMI_Handler              │
0x0000_000C  ├──────────────────────────┤
             │ HardFault_Handler        │  ← §6
             │  … các fault khác …      │
0x0000_003C  ├──────────────────────────┤
             │ SysTick_Handler          │
0x0000_0040  ├──────────────────────────┤
             │ IRQ0  (vd WWDG)          │  ← từ đây là ngắt PERIPHERAL
0x0000_0044  │ IRQ1  (vd EXTI0)         │
             │  …                       │
             │ IRQn  (vd ADC_IRQHandler)│  ← CPU nhảy vào ĐÂY khi ADC bắn ngắt
             └──────────────────────────┘
```

> Hệ quả thực tế: viết sai **tên** handler (vd `ADC_IRQHandler` thành `ADC_Handler`) thì **không có lỗi build** — linker lặng lẽ giữ handler mặc định (thường là `while(1)`), và chương trình "treo bí ẩn" khi ngắt bắn. Đây là bug bare-metal kinh điển.

**NVIC** (Nested Vectored Interrupt Controller): enable/disable từng ngắt, đặt **priority**, và cho phép **nesting** — ngắt ưu tiên cao **preempt** ISR đang chạy:

```
priority  (số NHỎ = ưu tiên CAO trên Cortex-M)

  main   ████░                                      ░████████
              │                                     ▲
  IRQ_B  (pri 2)  ███░           ░███
                     │           ▲
  IRQ_A  (pri 0)         ████████            ← A preempt B vì pri nhỏ hơn
                     ▲          
                  A bắn khi B đang chạy → B bị TẠM DỪNG, A chạy trước,
                  xong A mới trả về B, xong B mới trả về main.

  Ngược lại: nếu B (pri 2) bắn khi A (pri 0) đang chạy → B PHẢI CHỜ.
```

- **Số priority nhỏ = ưu tiên cao** (ngược trực giác — hay bị hỏi bẫy).
- Priority chia hai phần: **preemption priority** (quyết định có preempt được không) và **sub-priority** (chỉ quyết định thứ tự khi *cùng* lúc chờ, **không** preempt nhau).
- **NMI** (non-maskable): không thể tắt, dùng cho sự kiện chí tử (mất điện, lỗi clock).
- Đặt priority sai → **jitter/latency**: ngắt cần tất định lại bị ngắt vặt chặn.

## 3. Chia sẻ dữ liệu ISR ↔ main loop (mấu chốt)

Ba vấn đề khi ISR và main cùng đụng một dữ liệu:

1. **Compiler tối ưu** → biến chia sẻ phải **`volatile`** (ISR sửa "ngoài luồng" main thấy).
2. **Atomicity** — đọc/ghi biến nhiều byte (uint32 trên MCU 8/16-bit, hoặc struct) có thể bị ISR **chen giữa chừng** → dữ liệu rách.
3. **Nhất quán nhiều biến** — cập nhật 2 biến liên quan mà ISR chen giữa → main thấy trạng thái nửa vời.

**Giải pháp theo tình huống:**

- Biến đơn ≤ kích thước word + cờ đơn giản → `volatile` + kiểu atomic của phần cứng đủ.
- Dữ liệu nhiều byte / nhiều biến → **critical section**: tắt ngắt quanh đoạn đọc/ghi ở phía **main**, giữ **cực ngắn**:

```c
uint32_t snapshot;
__disable_irq();               // hoặc lưu/khôi phục PRIMASK để lồng an toàn
snapshot = shared_counter;     // đọc nhất quán
__enable_irq();
```

> ⚠️ `volatile` **không** thay được critical section: nó cấm tối ưu nhưng **không** làm RMW/đa-biến atomic.

### 3.1. Ring buffer SPSC — mẫu tốt nhất khi truyền *luồng dữ liệu*

**Vì sao tốt hơn critical section?** Cách tắt ngắt ở trên có một chi phí ẩn: **mỗi lần main đọc dữ liệu là một lần cả hệ thống bị điếc**. Ngắt khẩn cấp đến đúng lúc đó phải chờ → đẩy **worst-case latency** lên (§4). Ring buffer SPSC bỏ hẳn chi phí đó.

**Ý tưởng cốt lõi — tách quyền sở hữu, không chia sẻ quyền ghi:**

```
        ISR (producer)                      main loop (consumer)
        chỉ GHI head ────┐                 ┌──── chỉ GHI tail
        chỉ ĐỌC tail ──┐ │                 │ ┌── chỉ ĐỌC head
                       │ │                 │ │
                       ▼ ▼                 ▼ ▼
      buf[0] buf[1] buf[2] buf[3] buf[4] buf[5] buf[6] buf[7]
        │      │      ███    ███    ███     │      │      │
        └──────┴──────┴──────┴──────┴───────┴──────┴──────┘
                      ▲                     ▲
                     tail                  head
                  (đọc từ đây)          (ghi vào đây)

  KHÔNG có biến nào bị HAI bên cùng ghi  →  không có race  →  không cần khoá.
```

Đây là toàn bộ lý do nó hoạt động: **mỗi chỉ số có đúng một người ghi**. Không phải "may mà chạy được" — nó đúng theo thiết kế.

```c
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#define RB_SIZE 64                     // ⚠️ PHẢI là lũy thừa 2 (xem ghi chú dưới)
#define RB_MASK (RB_SIZE - 1)

static uint8_t             rb_buf[RB_SIZE];
static _Atomic uint32_t    rb_head = 0;   // ISR ghi, main đọc
static _Atomic uint32_t    rb_tail = 0;   // main ghi, ISR đọc

// ---------- phía ISR (producer) — KHÔNG chờ, KHÔNG tắt ngắt ----------
void UART_IRQHandler(void) {
    uint8_t byte = UART->DR;                                  // đọc + xoá cờ ngắt

    uint32_t h = atomic_load_explicit(&rb_head, memory_order_relaxed); // mình sở hữu -> relaxed đủ
    uint32_t t = atomic_load_explicit(&rb_tail, memory_order_acquire); // đọc của bên kia -> acquire

    if (((h + 1) & RB_MASK) == (t & RB_MASK))                 // đầy?
        return;                                               // DROP (xem đánh đổi bên dưới)

    rb_buf[h & RB_MASK] = byte;                               // ① ghi DỮ LIỆU trước
    atomic_store_explicit(&rb_head, h + 1, memory_order_release); // ② rồi mới công bố index
}                                                             //    release: ① không trượt xuống sau ②

// ---------- phía main (consumer) ----------
bool rb_pop(uint8_t *out) {
    uint32_t t = atomic_load_explicit(&rb_tail, memory_order_relaxed); // mình sở hữu
    uint32_t h = atomic_load_explicit(&rb_head, memory_order_acquire); // ③ acquire ghép với ②

    if (t == h) return false;                                 // rỗng

    *out = rb_buf[t & RB_MASK];                               // ④ thấy h mới ⟹ chắc chắn thấy dữ liệu ①
    atomic_store_explicit(&rb_tail, t + 1, memory_order_release);
    return true;
}
```

**Ba chi tiết quyết định tính đúng đắn:**

1. **Thứ tự ①→② và cặp `release`/`acquire`.** Nếu index `head` được công bố **trước** khi byte kịp ghi vào buffer, consumer đọc phải rác. `release` ở ② đảm bảo mọi ghi phía trên (①) đã nhìn thấy được với ai `acquire` thành công ở ③. Đây **đúng mẫu publish → subscribe**, xem [CPP-019](../15_prep/mock-interview/bank/cpp.md).
   > Trên MCU **đơn lõi** (Cortex-M), compiler barrier là đủ và `volatile` + thứ tự lệnh thường "chạy được". Nhưng viết đúng memory order thì code **portable** sang đa lõi (Cortex-A, ARM64) mà không phải sửa — và đây chính là chỗ interviewer đào.

2. **Kích thước lũy thừa 2 + mask.** `idx & (N-1)` thay cho `idx % N`: phép chia/modulo trên MCU không có divider tốn hàng chục chu kỳ, mask chỉ tốn 1. Ngoài ra để index **chạy tự do** (không reset về 0) rồi mới mask giúp phân biệt đầy/rỗng mà không phải hy sinh slot — với điều kiện kiểu unsigned (tràn số unsigned có định nghĩa, wrap đúng).

3. **Phân biệt ĐẦY và RỖNG.** Cả hai đều cho `head == tail` nếu làm ngây thơ. Ba cách xử lý:

| Cách | Ưu | Nhược |
|---|---|---|
| **Hy sinh 1 slot** (đầy khi `head+1 == tail`) | Đơn giản nhất, không thêm biến chia sẻ | Mất 1 phần tử; đây là cách dùng ở code trên |
| Giữ thêm biến `count` | Dùng hết dung lượng | ⚠️ `count` bị **cả hai bên ghi** → **phá vỡ SPSC**, phải khoá lại |
| Index chạy tự do, so `head - tail` | Dùng hết dung lượng, vẫn SPSC | Phải cẩn thận wrap-around và kiểu unsigned |

**Đánh đổi & khi nào KHÔNG dùng:**

| | |
|---|---|
| ✅ **Hợp khi** | Truyền **luồng dữ liệu** ISR → main (UART RX, ADC sample, sự kiện). Đúng **1 producer + 1 consumer** |
| ✅ **Được gì** | Không tắt ngắt → **latency worst-case không bị ảnh hưởng**; ISR không bao giờ chờ (bounded time — hợp real-time) |
| ❌ **Không dùng khi** | **Nhiều** producer hoặc **nhiều** consumer → SPSC sụp đổ, cần khoá hoặc MPMC queue thật |
| ❌ | Cần **truyền dữ liệu lớn/biến thiên** — ring buffer byte không có ranh giới message, phải tự đóng khung |
| ⚠️ **Phải quyết định** | **Buffer đầy thì làm gì?** Code trên **drop** dữ liệu mới. Lựa chọn khác: ghi đè cái cũ nhất (hợp log/trace), hoặc đếm số lần drop để báo lỗi. **ISR tuyệt đối không được chờ** — nên "block đến khi có chỗ" không phải lựa chọn |

> **Chốt:** SPSC ring buffer là cách **duy nhất** vừa truyền được luồng dữ liệu ISR→main vừa **không** đụng tới việc tắt ngắt. Cái giá là ràng buộc đúng **một** producer và **một** consumer — vi phạm ràng buộc đó thì mọi đảm bảo biến mất.

## 4. Interrupt latency

Latency = thời gian từ lúc **sự kiện xảy ra** tới khi **ISR bắt đầu chạy** (đôi khi tính tới khi xử lý xong). Vẽ ra trục thời gian mới thấy nó gồm những khoản gì:

```
 sự kiện                                                      ISR chạy
 xảy ra                                                       lệnh đầu
   │                                                              │
   ▼                                                              ▼
   ├──────────┬──────────────────┬──────────────┬─────────────────┤
   │    ①     │        ②         │      ③       │        ④        │
   │ đồng bộ  │  ĐANG TẮT NGẮT   │  ISR pri cao │  lưu context    │
   │ phần cứng│  (critical sect.)│  hơn đang    │  + fetch vector │
   │ (cố định)│  ← BẠN gây ra    │  chạy/chờ    │  (phần cứng     │
   │          │                  │              │   + wait state) │
   └──────────┴──────────────────┴──────────────┴─────────────────┘
                       ▲                  ▲
              phần DUY NHẤT bạn      phần điều chỉnh được
              kiểm soát trực tiếp    bằng THIẾT KẾ priority
```

| # | Khoản | Ai quyết định | Giảm bằng cách |
|---|---|---|---|
| ① | Đồng bộ tín hiệu vào NVIC | Phần cứng | Không đổi được |
| ② | **Critical section đang tắt ngắt** | **Code của bạn** | Giữ đoạn `__disable_irq()` **cực ngắn** — hoặc bỏ hẳn bằng ring buffer (§3.1) |
| ③ | ISR priority **cao hơn** đang chạy, hoặc cùng priority phải chờ xong | Thiết kế priority | Gán priority theo **yêu cầu thời gian thật**, không theo "cảm giác quan trọng"; giữ mọi ISR ngắn |
| ④ | Lưu context (8 thanh ghi), đọc vector, nạp lệnh | Phần cứng + flash | Tăng clock, bật cache/prefetch, để handler nóng trong RAM (một số MCU) |

**Điểm quan trọng nhất:** với hệ real-time bạn phải đo **worst-case**, không phải trung bình. Trung bình 2 µs mà worst-case 200 µs (vì một critical section dài hiếm khi chạy) là hệ **đã hỏng** — deadline bị vi phạm ở đúng cái lần hiếm đó. Và ② là khoản duy nhất bạn *tự tay* tạo ra, nên nó là chỗ đầu tiên phải soi.

## 5. Reentrancy

Hàm **reentrant** = có thể bị ngắt giữa chừng và gọi lại (từ ISR/thread khác) mà vẫn đúng — không dùng **biến static/global có trạng thái** không bảo vệ, không trả con trỏ tới buffer tĩnh dùng chung. Quan trọng vì ISR có thể chen vào giữa hàm main đang chạy: nếu cả hai gọi cùng một hàm **non-reentrant** (`strtok`, một số `malloc`, hàm dùng buffer tĩnh) → corruption. Viết reentrant: dùng biến local/tham số thay static, tránh trạng thái chia sẻ, hoặc bảo vệ bằng critical section. (Nghiêm ngặt hơn thread-safe: an toàn cả khi tự gọi lại qua ngắt.)

## 6. Hard fault (Cortex-M) — điều tra

Hard fault = CPU gặp lỗi nghiêm trọng (truy cập bộ nhớ sai, lệnh không hợp lệ, unaligned access, escalate từ fault khác). Điều tra:

1. Đọc **fault status registers** (`CFSR`, `HFSR`, `MMFAR`, `BFAR`) — cho biết *loại* lỗi + địa chỉ.
2. Lấy **stacked frame** CPU tự đẩy khi vào fault — **PC** = lệnh gây lỗi, LR = đường về.
3. Map PC → dòng source bằng map file / `addr2line` / debugger.

**Stacked frame — vì sao là `sp[6]` và `sp[5]`:** khi vào exception, Cortex-M **tự động** đẩy 8 thanh ghi lên stack theo thứ tự cố định. Biết layout này là đọc được ngay PC/LR mà không cần debugger:

```
      địa chỉ tăng ↑
  sp[7]  ┌─────────────┐
         │    xPSR     │
  sp[6]  ├─────────────┤
         │  🎯  PC     │  ← LỆNH GÂY LỖI — cái bạn cần nhất
  sp[5]  ├─────────────┤
         │  🎯  LR     │  ← ai đã gọi tới đó
  sp[4]  ├─────────────┤
         │    R12      │
  sp[3]  ├─────────────┤
         │    R3       │
  sp[2]  ├─────────────┤
         │    R2       │
  sp[1]  ├─────────────┤
         │    R1       │
  sp[0]  ├─────────────┤ ← SP trỏ vào đây khi handler bắt đầu
         │    R0       │
         └─────────────┘
```

```c
// HardFault handler in ra thanh ghi để phân tích ngoài field
void HardFault_Handler(void) {
    // ⚠️ Chọn ĐÚNG stack: bit 2 của EXC_RETURN (trong LR) cho biết
    //    frame nằm ở MSP (handler/main) hay PSP (thread mode, hay gặp khi có RTOS)
    uint32_t *sp;
    __asm volatile ("tst lr, #4        \n"
                    "ite eq            \n"
                    "mrseq %0, msp     \n"
                    "mrsne %0, psp     \n" : "=r" (sp));

    uint32_t pc = sp[6], lr = sp[5];         // theo layout ở trên
    printf("HardFault PC=%08lx LR=%08lx CFSR=%08lx BFAR=%08lx\n",
           pc, lr, SCB->CFSR, SCB->BFAR);
    while (1) {}
}
```

> ⚠️ Lấy nhầm MSP khi frame thực ra ở PSP (hoặc ngược lại) → `sp[6]` trỏ vào rác, và bạn đi truy một địa chỉ **không liên quan gì** tới lỗi. Đây là lý do bản dùng `__get_MSP()` trần chỉ đúng khi chắc chắn không có RTOS.
> ⚠️ `printf` trong fault handler là **vi phạm quy tắc §1** (không reentrant) — chấp nhận được vì hệ đã chết, nhưng nếu `printf` cũng chết theo thì mất luôn thông tin. Bản production nên ghi thanh ghi vào **vùng RAM không bị xoá khi reset** rồi reboot, đọc ra ở lần boot sau.

Nguyên nhân hay gặp: dereference null/dangling, **stack overflow** đè vùng khác, gọi qua con trỏ hàm rác, unaligned access, truy cập peripheral chưa bật clock. Viết một `HardFault_Handler` in các thanh ghi này ra UART là **công cụ vàng** ngoài hiện trường.

---

## Ôn tập (bank)

[EMB-009](../15_prep/mock-interview/bank/embedded-fundamentals.md) (ISR rules), [EMB-010](../15_prep/mock-interview/bank/embedded-fundamentals.md) (chia sẻ dữ liệu ISR↔main), [EMB-011](../15_prep/mock-interview/bank/embedded-fundamentals.md) (vector table/NVIC), [EMB-012](../15_prep/mock-interview/bank/embedded-fundamentals.md) (latency), [EMB-013](../15_prep/mock-interview/bank/embedded-fundamentals.md) (reentrancy), [EMB-032](../15_prep/mock-interview/bank/embedded-fundamentals.md) (hard fault). Đối chiếu góc Linux: [DRV-011 top/bottom half](../15_prep/mock-interview/bank/drivers-embedded.md).
