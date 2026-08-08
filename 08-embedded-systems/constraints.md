# Embedded Constraints — Bộ nhớ, Năng lượng, Realtime & Kỹ thuật tối ưu

> **TL;DR**
> - Embedded sống với **ràng buộc khắc nghiệt**: ít RAM/flash, nguồn pin hạn chế, deadline realtime, không người can thiệp, môi trường khắc nghiệt.
> - **Bộ nhớ**: tránh/giới hạn heap (fragmentation, non-deterministic) → ưu tiên cấp phát tĩnh/stack/memory pool; biết rõ footprint.
> - **Năng lượng**: dùng sleep mode, tắt peripheral không dùng, dùng interrupt thay polling, hạ clock — phần mềm ảnh hưởng lớn tới tuổi thọ pin.
> - **Độ tin cậy**: **watchdog** reset khi treo; xử lý lỗi không được crash; cập nhật firmware an toàn (A/B + rollback).
> - **Tối ưu đúng chỗ**: đo trước (đừng đoán); cân bằng giữa tốc độ, RAM, flash, và điện năng tùy bài toán.

---

## 1. Tư duy chung: lập trình dưới ràng buộc

PC có tài nguyên gần như vô hạn so với embedded. Trên embedded, mọi quyết định đánh đổi giữa **RAM, flash, CPU cycle, năng lượng, độ trễ, độ tin cậy**. Kỹ sư embedded giỏi luôn hỏi: *cái này tốn bao nhiêu RAM? worst-case bao lâu? có cấp phát động không? ngốn điện thế nào?* — chứ không chỉ "chạy đúng".

---

## 2. Ràng buộc bộ nhớ

RAM/flash thường tính bằng KB–MB. Vấn đề & kỹ thuật:

**Heap fragmentation** — vì sao `malloc` thất bại dù *tổng* bộ nhớ trống vẫn còn nhiều:

```
Heap 1KB, sau một thời gian cấp phát/giải phóng xen kẽ:

 ┌────┬──────┬────┬────────┬────┬──────┬────┬──────────┐
 │ 96 │ FREE │128 │  FREE  │ 64 │ FREE │192 │   FREE   │
 │dùng│ 120B │dùng│  100B  │dùng│  80B │dùng│   130B   │
 └────┴──────┴────┴────────┴────┴──────┴────┴──────────┘

 Tổng FREE = 120+100+80+130 = 430 byte
 malloc(200)  →  ❌ THẤT BẠI — không khối LIỀN NHAU nào đủ 200

 Trên PC: hiếm gặp (heap lớn, có MMU dồn trang).
 Trên MCU 64KB RAM, chạy 6 tháng không reboot: gần như CHẮC CHẮN xảy ra.
```

Cộng thêm: thời gian `malloc` **không tất định** (phải duyệt free list, độ dài thay đổi) → phá vỡ phân tích WCET của hệ realtime.

**Ba lựa chọn thay thế — chọn theo vòng đời dữ liệu:**

| Cách | Khi nào dùng | Ưu | Nhược |
|---|---|---|---|
| **Cấp phát tĩnh** (`static`/global, buffer cố định) | Dữ liệu sống suốt vòng đời chương trình | Biết footprint **lúc biên dịch**; linker báo ngay nếu không đủ RAM | Chiếm chỗ kể cả lúc không dùng; không co giãn |
| **Stack** | Dữ liệu vòng đời ngắn, trong một hàm | Miễn phí, tự dọn, tất định | Stack MCU rất nhỏ (vài KB) — mảng lớn/đệ quy → **overflow âm thầm** (§ không có MMU) |
| **Memory pool** (fixed-block) | Nhiều object **cùng kích thước**, cấp/thả liên tục (packet, message, sample) | **Tất định O(1)**, không phân mảnh | Phải chọn trước kích thước khối + số lượng; lãng phí nếu object nhỏ hơn khối |

```c
// Memory pool tối giản — cấp/thả O(1), KHÔNG BAO GIỜ phân mảnh
#define POOL_N     16
#define BLOCK_SIZE 64

static uint8_t  pool[POOL_N][BLOCK_SIZE];
static uint16_t free_list[POOL_N];      // stack các chỉ số khối rảnh
static int      free_top = POOL_N;

void pool_init(void) {
    for (int i = 0; i < POOL_N; i++) free_list[i] = i;
    free_top = POOL_N;
}

void *pool_alloc(void) {                 // O(1), thời gian TẤT ĐỊNH
    if (free_top == 0) return NULL;      // hết khối — báo lỗi, không "cố tìm"
    return pool[free_list[--free_top]];
}

void pool_free(void *p) {                // O(1)
    int idx = ((uint8_t*)p - &pool[0][0]) / BLOCK_SIZE;
    free_list[free_top++] = (uint16_t)idx;
}
```

> Vì sao pool không phân mảnh: **mọi khối cùng kích thước** nên khối vừa trả luôn dùng lại được cho yêu cầu tiếp theo. Phân mảnh sinh ra từ việc trộn lẫn nhiều kích thước — bỏ điều kiện đó là bỏ luôn vấn đề.

- Tránh `malloc/new` trong vòng lặp nóng / sau giai đoạn init; nhiều coding standard embedded (MISRA) hạn chế cấp phát động.
- **Tiết kiệm flash**: `-Os` (tối ưu kích thước), loại bỏ code/feature không dùng (LTO, `--gc-sections`), tránh template/STL phình code nếu eo hẹp, dùng `const` để dữ liệu nằm ở flash (read-only) thay vì RAM.
- Đo footprint: `size`, map file của linker, theo dõi RAM/stack high-water mark.

---

## 3. Ràng buộc năng lượng

Nhiều thiết bị chạy pin → phần mềm quyết định tuổi thọ. Vẽ dòng điện theo thời gian là thấy ngay vì sao:

```
 dòng điện
    ▲
10mA│ ███████████████████████████████████████  ❌ POLLING: CPU thức liên tục
    │                                              quay vòng hỏi "có dữ liệu chưa?"
    └──────────────────────────────────────────► t     → ~10mA TRUNG BÌNH

10mA│ ██          ██          ██               ✅ INTERRUPT + SLEEP
    │ ██          ██          ██                  thức đúng lúc có việc
 5µA│ ░░░░░░░░░░░░██░░░░░░░░░░██░░░░░░░░░░░░░     ngủ sâu phần còn lại
    └──┬──────────┬──────────┬─────────────────► t     → ~50µA TRUNG BÌNH
       ngắt       ngắt       ngắt
                                              chênh lệch ~200 LẦN tuổi thọ pin
```

- **Sleep modes**: đưa CPU/peripheral vào trạng thái ngủ sâu khi rảnh, thức dậy bằng interrupt (vd RTC, GPIO). **"Race to sleep"**: chạy ở tần số cao để làm xong **thật nhanh** rồi ngủ sâu — thường tiết kiệm hơn chạy chậm rề rề ở tần số thấp, vì năng lượng ≈ (công suất × thời gian) và thời gian giảm mạnh hơn công suất tăng.
- **Interrupt thay polling** — khác biệt nằm ở chỗ CPU làm gì khi *không* có việc:

```c
// ❌ Polling: CPU quay vòng, đốt điện cả khi không có gì xảy ra
while (1) {
    if (UART->SR & RXNE) process(UART->DR);   // 99.9% số vòng là VÔ ÍCH
}

// ✅ Interrupt + sleep: CPU ngủ, phần cứng đánh thức khi có việc thật
while (1) {
    __WFI();                    // Wait For Interrupt — CPU dừng clock, tụt xuống µA
    if (flag_rx) {              // tỉnh dậy vì có ngắt
        flag_rx = false;
        process(rx_byte);
    }
}
```
- **Tắt peripheral & clock gating**: tắt module (ADC, UART...) và clock khi không dùng.
- **Hạ tần số/điện áp** (DVFS) khi tải thấp.
- Đánh đổi: hiệu năng/độ trễ vs tiết kiệm điện — thiết kế theo profile sử dụng thực tế.

---

## 4. Độ tin cậy & an toàn

Thiết bị thường chạy không người giám sát, lâu dài, môi trường khắc nghiệt:
- **Watchdog timer**: phần cứng đếm ngược; phần mềm phải "kick" định kỳ. Nếu hệ treo (không kick), watchdog **reset** thiết bị → tự phục hồi. **Kick sai chỗ là lỗi thiết kế phổ biến nhất** — watchdog vẫn được vỗ về trong khi hệ đã chết:

```c
// ❌ SAI: kick trong một timer ISR độc lập
void TIM2_IRQHandler(void) {
    IWDG->KR = 0xAAAA;          // kick "mù"
}
// -> main loop treo cứng, mọi task chết, nhưng timer ISR vẫn chạy đều
//    -> watchdog KHÔNG BAO GIỜ reset. Thiết bị đơ vĩnh viễn ngoài field.

// ✅ ĐÚNG: chỉ kick khi MỌI task quan trọng đều xác nhận còn sống
static volatile uint32_t alive_mask = 0;
#define TASK_SENSOR  (1u << 0)
#define TASK_COMM    (1u << 1)
#define TASK_CTRL    (1u << 2)
#define ALL_TASKS    (TASK_SENSOR | TASK_COMM | TASK_CTRL)

void task_sensor(void) { /* … */ alive_mask |= TASK_SENSOR; }   // mỗi task tự báo

void main_loop(void) {
    while (1) {
        run_all_tasks();
        if ((alive_mask & ALL_TASKS) == ALL_TASKS) {   // ✅ đủ mặt mới kick
            IWDG->KR = 0xAAAA;
            alive_mask = 0;                            // reset để chu kỳ sau phải báo lại
        }
    }
}
```

> **Windowed watchdog**: phải kick trong một *cửa sổ* thời gian — không quá muộn **và không quá sớm**. Bắt được cả lỗi "chạy loạn quá nhanh" (vd code nhảy lạc vào vòng lặp kick liên tục), thứ mà watchdog thường bỏ sót.
- **Xử lý lỗi không crash**: kiểm tra giá trị trả về, fail an toàn (safe state), không để một lỗi nhỏ làm sập toàn hệ. Trong hard realtime/safety, exception thường bị cấm — dùng mã lỗi.
- **Brown-out / mất điện**: ghi dữ liệu quan trọng cẩn thận (atomic, journaling), dùng rootfs read-only + A/B partition để cập nhật an toàn ([boot-process.md](boot-process.md)).
- **Đảm bảo realtime**: phân tích worst-case execution time (WCET), tránh thao tác không tất định (heap, page fault) trong vùng realtime, dùng `mlockall` để khóa bộ nhớ (Linux).

---

## 5. Ràng buộc thời gian (realtime) — kỹ thuật

- Giữ **interrupt handler ngắn** (top half), đẩy việc nặng xuống bottom half/task ([05/driver-basics](../05-drivers-device-tree/driver-basics.md)).
- Tránh trong vùng realtime: cấp phát động, I/O blocking, page fault (lock memory), khóa giữ lâu.
- Dùng **priority hợp lý** + priority inheritance để tránh inversion.
- Đo **latency & jitter** thực tế (vd cyclictest trên Linux PREEMPT_RT), không tin lý thuyết suông.

---

## 6. Một số kỹ thuật & lưu ý thực dụng

- **Fixed-point arithmetic**: khi không có FPU, dùng số nguyên/Qm.n thay float → nhanh & tất định. Ý tưởng: đặt một "dấu phẩy ảo" ở vị trí cố định trong số nguyên.

```c
// Q16.16: 32 bit = 16 bit phần nguyên | 16 bit phần thập phân
typedef int32_t q16_t;
#define Q16_SHIFT 16
#define TO_Q16(x)   ((q16_t)((x) * (1 << Q16_SHIFT)))   // 1.5  -> 0x0001_8000
#define FROM_Q16(x) ((float)(x) / (1 << Q16_SHIFT))

q16_t q16_add(q16_t a, q16_t b) { return a + b; }        // ✅ cộng/trừ: y như số nguyên

q16_t q16_mul(q16_t a, q16_t b) {
    // ⚠️ BẪY: a*b tràn 32 bit ngay với số vừa phải -> PHẢI qua trung gian 64 bit
    return (q16_t)(((int64_t)a * b) >> Q16_SHIFT);
}
```

> Đánh đổi: nhanh và **tất định** (không có FPU emulation hàng trăm chu kỳ), nhưng **tự quản scale** và phải canh **overflow khi nhân**. Có FPU (Cortex-M4F/M7) hoặc tính toán ít → cứ dùng `float`. Không FPU + tính nhiều trong vòng lặp nóng → fixed-point.
- **Bit manipulation & lookup table**: tiết kiệm CPU/RAM cho phép tính lặp lại.
- **`volatile`** cho biến chia sẻ với ISR/phần cứng; nhưng `volatile` **không** thay thế đồng bộ đa luồng (không atomic) — đừng nhầm.
- **Alignment & packing**: `struct` packing ảnh hưởng kích thước & tốc độ truy cập; lệch alignment có thể fault trên một số kiến trúc.
- **Đo trước khi tối ưu**: dùng profiler/đếm cycle/đo dòng điện. Tối ưu sai chỗ tốn công mà vô ích; "premature optimization" vẫn đúng, nhưng trên embedded ràng buộc rõ nên cần đo sớm hơn PC.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Vì sao nên hạn chế cấp phát động (heap) trong embedded? Thay bằng gì?</summary>

Cấp phát động lâu dài gây **fragmentation**: bộ nhớ bị phân mảnh khiến `malloc` có thể thất bại dù tổng dung lượng còn trống, và thời gian cấp phát/giải phóng **không tất định** — cả hai đều nguy hiểm cho hệ chạy liên tục và hệ realtime. Trên thiết bị RAM rất ít, một lần `malloc` thất bại có thể làm hỏng chức năng. Thay thế: cấp phát **tĩnh** (biến static/global, buffer cố định — biết footprint lúc biên dịch), dùng **stack** cho dữ liệu vòng đời ngắn (cẩn thận overflow vì stack nhỏ), hoặc **memory pool/fixed-block allocator** (cấp các khối cùng kích thước từ pool — tất định và không phân mảnh). Nhiều coding standard embedded (như MISRA) hạn chế hoặc cấm cấp phát động sau giai đoạn khởi tạo.
</details>

<details><summary>2) Phần mềm ảnh hưởng tới tiêu thụ điện như thế nào?</summary>

Rất lớn, đặc biệt với thiết bị chạy pin. Các kỹ thuật phần mềm: đưa CPU và peripheral vào **sleep mode** khi rảnh và thức dậy bằng interrupt (chiến lược "race to sleep" — làm xong nhanh rồi ngủ sâu); dùng **interrupt thay vì polling** để CPU không bận chờ; **tắt peripheral và clock** (clock gating) khi không dùng; **hạ tần số/điện áp** (DVFS) khi tải thấp. Polling, busy-wait, hoặc giữ CPU thức không cần thiết đốt điện vô ích. Vì vậy thiết kế luồng điều khiển hướng sự kiện (event-driven) và quản lý trạng thái nguồn là phần quan trọng của lập trình embedded tiết kiệm năng lượng.
</details>

<details><summary>3) Watchdog timer là gì và dùng thế nào cho đúng?</summary>

Watchdog là một timer phần cứng đếm ngược; phần mềm phải định kỳ "kick" (reset bộ đếm) để báo hệ thống còn sống. Nếu hệ treo hoặc kẹt (không kick kịp), watchdog hết giờ và **reset thiết bị**, cho phép tự phục hồi — quan trọng với thiết bị chạy không người giám sát. Dùng đúng: kick ở vị trí phản ánh hệ thống thực sự hoạt động bình thường (vd cuối một chu kỳ xử lý chính, hoặc khi mọi task quan trọng đều báo còn sống), **không** kick mù trong một interrupt độc lập vì như vậy watchdog vẫn được kick dù logic chính đã chết. Một số thiết kế dùng windowed watchdog (phải kick trong một cửa sổ thời gian, không quá sớm cũng không quá muộn) để bắt cả lỗi chạy quá nhanh.
</details>

<details><summary>4) Làm sao đảm bảo tính realtime ở mức lập trình?</summary>

Một số nguyên tắc: giữ interrupt handler thật ngắn (top half) và đẩy việc nặng xuống bottom half/task; tránh trong vùng realtime các thao tác không tất định như cấp phát động, I/O blocking, page fault (khóa bộ nhớ bằng `mlockall` trên Linux để tránh swap/fault), và giữ khóa quá lâu; gán độ ưu tiên hợp lý và dùng priority inheritance để tránh priority inversion; phân tích worst-case execution time (WCET) thay vì chỉ trung bình; và **đo latency/jitter thực tế** (vd cyclictest trên Linux PREEMPT_RT) thay vì tin lý thuyết. Trên Linux còn dùng `SCHED_FIFO`/`SCHED_RR` và cô lập CPU (`isolcpus`) cho tác vụ realtime.
</details>

<details><summary>5) volatile có đủ để đồng bộ biến chia sẻ giữa các thread không?</summary>

Không. `volatile` chỉ đảm bảo compiler không tối ưu bỏ/cache/đổi thứ tự các truy cập tới biến đó — cần thiết khi biến thay đổi bởi phần cứng hoặc ISR (để mỗi lần đọc/ghi diễn ra thật). Nhưng `volatile` **không** cung cấp tính nguyên tử (atomicity) cho thao tác đọc-sửa-ghi, cũng không cung cấp memory ordering/đồng bộ giữa các CPU core. Do đó dùng `volatile` để bảo vệ dữ liệu chia sẻ giữa các thread trên hệ đa lõi là sai và gây race condition. Đồng bộ đa luồng cần `std::atomic` (với memory order phù hợp) hoặc mutex; `volatile` chỉ đúng cho giao tiếp với phần cứng/ISR và ngay cả khi đó vẫn có thể cần barrier.
</details>

<details><summary>6) Làm sao giảm dung lượng flash/RAM của firmware?</summary>

Về flash (code): biên dịch với `-Os` (tối ưu kích thước), bật LTO và `--gc-sections` để loại bỏ code/dữ liệu không dùng, cẩn thận với template/STL vì có thể phình code, dùng `const` để đặt dữ liệu chỉ-đọc ở flash thay vì RAM, và loại bỏ tính năng/log không cần trong bản release. Về RAM: ưu tiên cấp phát tĩnh có kiểm soát thay vì heap, giảm kích thước buffer xuống mức đủ dùng, đặt dữ liệu hằng ở flash, theo dõi stack high-water mark để cắt dư. Luôn **đo** bằng lệnh `size`, map file của linker và công cụ phân tích footprint để biết phần nào chiếm chỗ thay vì cắt mò.
</details>

---
⬅️ [rtos-vs-linux.md](rtos-vs-linux.md) · ➡️ Tiếp theo: [09-debugging/](../09-debugging/)
