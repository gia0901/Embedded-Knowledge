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

- **Vector table**: mảng địa chỉ handler (đầu flash) — mỗi nguồn ngắt một entry; CPU tra bảng để nhảy vào đúng ISR. Entry đầu = giá trị stack pointer ban đầu, thứ hai = reset handler.
- **NVIC** (Nested Vectored Interrupt Controller, Cortex-M): enable/disable từng ngắt, đặt **priority**, hỗ trợ **nesting** — ngắt priority cao hơn **preempt** ISR đang chạy. **Số priority nhỏ = ưu tiên cao** trên Cortex-M. Nhóm priority chia thành preemption vs sub-priority.
- Ngắt cùng/thấp hơn priority phải chờ. Có **NMI** (non-maskable). Đặt priority sai gây jitter/latency.

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

- Mẫu tốt nhất khi có thể: ISR ghi vào **ring buffer (SPSC)** — main đọc, chỉ cần index kiểu atomic, **tránh tắt ngắt** hoàn toàn.

> ⚠️ `volatile` **không** thay được critical section: nó cấm tối ưu nhưng **không** làm RMW/đa-biến atomic.

## 4. Interrupt latency

Latency = thời gian từ lúc sự kiện ngắt xảy ra tới khi ISR bắt đầu (đôi khi tính tới khi xử lý xong). Yếu tố:

1. **Critical section đang tắt ngắt** — đoạn giữ ngắt tắt càng lâu, latency worst-case càng lớn (lý do critical section phải cực ngắn).
2. ISR **priority thấp hơn** đang chạy chặn (không preempt được).
3. Thời gian lưu context / vào handler (phần cứng định).
4. Wait states flash / clock thấp.

Muốn latency thấp & tất định: critical section ngắn, priority hợp lý, ISR gọn. Đây là số phải **đo worst-case** cho hệ real-time, không phải trung bình.

## 5. Reentrancy

Hàm **reentrant** = có thể bị ngắt giữa chừng và gọi lại (từ ISR/thread khác) mà vẫn đúng — không dùng **biến static/global có trạng thái** không bảo vệ, không trả con trỏ tới buffer tĩnh dùng chung. Quan trọng vì ISR có thể chen vào giữa hàm main đang chạy: nếu cả hai gọi cùng một hàm **non-reentrant** (`strtok`, một số `malloc`, hàm dùng buffer tĩnh) → corruption. Viết reentrant: dùng biến local/tham số thay static, tránh trạng thái chia sẻ, hoặc bảo vệ bằng critical section. (Nghiêm ngặt hơn thread-safe: an toàn cả khi tự gọi lại qua ngắt.)

## 6. Hard fault (Cortex-M) — điều tra

Hard fault = CPU gặp lỗi nghiêm trọng (truy cập bộ nhớ sai, lệnh không hợp lệ, unaligned access, escalate từ fault khác). Điều tra:

1. Đọc **fault status registers** (`CFSR`, `HFSR`, `MMFAR`, `BFAR`) — cho biết *loại* lỗi + địa chỉ.
2. Lấy **stacked frame** CPU đẩy khi vào fault (R0–R3, R12, **LR, PC, xPSR**) — **PC** = lệnh gây lỗi, LR = đường về; đọc từ stack trong fault handler.
3. Map PC → dòng source bằng map file / `addr2line` / debugger.

```c
// HardFault handler in ra thanh ghi để phân tích ngoài field
void HardFault_Handler(void) {
    uint32_t *sp = (uint32_t*)__get_MSP();   // hoặc PSP tùy EXC_RETURN
    uint32_t pc = sp[6], lr = sp[5];         // vị trí đẩy của PC, LR
    printf("HardFault PC=%08lx LR=%08lx CFSR=%08lx\n", pc, lr, SCB->CFSR);
    while (1) {}
}
```

Nguyên nhân hay gặp: dereference null/dangling, **stack overflow** đè vùng khác, gọi qua con trỏ hàm rác, unaligned access, truy cập peripheral chưa bật clock. Viết một `HardFault_Handler` in các thanh ghi này ra UART là **công cụ vàng** ngoài hiện trường.

---

## Ôn tập (bank)

[EMB-009](../15_prep/mock-interview/bank/embedded-fundamentals.md) (ISR rules), [EMB-010](../15_prep/mock-interview/bank/embedded-fundamentals.md) (chia sẻ dữ liệu ISR↔main), [EMB-011](../15_prep/mock-interview/bank/embedded-fundamentals.md) (vector table/NVIC), [EMB-012](../15_prep/mock-interview/bank/embedded-fundamentals.md) (latency), [EMB-013](../15_prep/mock-interview/bank/embedded-fundamentals.md) (reentrancy), [EMB-032](../15_prep/mock-interview/bank/embedded-fundamentals.md) (hard fault). Đối chiếu góc Linux: [DRV-011 top/bottom half](../15_prep/mock-interview/bank/drivers-embedded.md).
