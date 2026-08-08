# Lập trình RTOS & kiến trúc firmware

> Từ *chọn* RTOS ([rtos-vs-linux.md](rtos-vs-linux.md)) sang **lập trình** RTOS: scheduler, primitive đồng bộ, ISR→task, sizing stack, schedulability; và **kiến trúc firmware** (superloop / event-driven / RTOS) + bootloader bare-metal.
> Ôn dạng phỏng vấn: bank [EMB-014…023](../15_prep/mock-interview/bank/embedded-fundamentals.md).

---

## 1. Scheduler: preemptive vs cooperative

- **Cooperative**: task chạy tới khi **tự nhường** (yield/block) → đơn giản, ít cần bảo vệ dữ liệu (không bị chen bất ngờ), nhưng **một task tham lam treo cả hệ**, latency phụ thuộc task khác → khó đảm bảo real-time.
- **Preemptive**: scheduler **cướp CPU** theo priority/tick → task ưu tiên cao chạy ngay, tất định hơn, hợp real-time; đổi lại phải **bảo vệ dữ liệu chia sẻ** (bị chen bất cứ lúc nào) + chi phí context switch.

Cùng một tải, hai scheduler cho kết quả khác hẳn về **thời điểm** task quan trọng được chạy:

```
  Task_HIGH cần chạy ngay khi sự kiện tới (deadline 5ms)
  Task_LOW  đang chạy một phép tính dài 40ms

 ❌ COOPERATIVE
   Task_LOW  ████████████████████████████████████████│Task_HIGH ███
                                                     ▲
   sự kiện ──┘ (t=5ms)                          t=40ms: LOW mới tự nhường
                                                 → HIGH trễ 35ms → TRƯỢT DEADLINE

 ✅ PREEMPTIVE
   Task_LOW  █████│                       ┌─────────────────────────
   Task_HIGH      │███████████████████████│
                  ▲
   sự kiện ───────┘ (t=5ms) → scheduler CƯỚP CPU ngay → HIGH chạy đúng hạn ✅
                             LOW chạy tiếp phần còn lại sau
```

Cái giá của preemptive: `Task_LOW` bị chen **bất cứ lúc nào**, kể cả giữa lúc đang sửa dở dữ liệu chia sẻ → **bắt buộc** phải bảo vệ (§2). Cooperative không cần vì bạn biết chính xác điểm nào có thể bị nhường.

RTOS phổ biến (FreeRTOS, Zephyr) mặc định **preemptive theo priority**. **Time slicing** (round-robin) chỉ áp cho task **cùng priority**; task priority cao hơn luôn preempt.

## 2. Primitive đồng bộ — dùng cái nào khi nào

| Primitive | Dùng cho | Ghi chú |
|---|---|---|
| **Mutex** | loại trừ lẫn nhau bảo vệ tài nguyên chia sẻ | có **ownership** + **priority inheritance** (chống priority inversion); chỉ task giữ mới trả |
| **Binary/counting semaphore** | **báo hiệu** (ISR "give" → task "take"); đếm tài nguyên | **không ownership** → không dùng thay mutex (mất PI) |
| **Queue** | truyền **dữ liệu có ranh giới** giữa task/ISR | thread-safe sẵn — cách chính để chuyển dữ liệu |
| **Event flag/group** | chờ **tổ hợp nhiều sự kiện** (AND/OR nhiều bit) | vd chờ cả "data ready" và "buffer free" |

Quy tắc: bảo vệ vùng tới hạn → **mutex**; báo hiệu ISR→task → **semaphore**; chuyển dữ liệu → **queue**; chờ nhiều điều kiện → **event flag**. (Lý do binary semaphore ≠ mutex: xem [priority inversion / OS-015](../15_prep/mock-interview/bank/os.md).)

## 3. ISR → task (deferred interrupt processing)

ISR phải ngắn → chỉ làm tối thiểu rồi **đẩy việc xuống task**: "give" semaphore / gửi queue / set event flag để đánh thức task xử lý (mô hình top/bottom-half của RTOS). Bắt buộc dùng **API bản `…FromISR`**:

```c
void UART_IRQHandler(void) {
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(rx_queue, &byte, &woken);   // API an toàn trong ISR
    portYIELD_FROM_ISR(woken);   // context switch ngay nếu task vừa dậy có priority cao hơn
}
```

`xHigherPriorityTaskWoken` + `portYIELD_FROM_ISR` → nếu việc đánh thức làm một task ưu tiên cao hơn sẵn sàng, switch **ngay khi ra khỏi ISR** (giảm latency). Không được block/chờ trong ISR.

## 4. Stack mỗi task & phát hiện overflow

Mỗi task có **stack riêng** (cấp khi tạo task) → tổng RAM = Σ stack task + heap RTOS + `.data`/`.bss` → dễ cạn. **Sizing**: ước worst-case call depth + local lớn + dùng **high-water-mark** (`uxTaskGetStackHighWaterMark`) đo mức thực rồi chừa biên; nhớ cộng chi phí **ngắt** chạy trên stack task (hoặc stack riêng tùy port). **Phát hiện overflow**: hook RTOS (`configCHECK_FOR_STACK_OVERFLOW` — kiểm pattern/pointer khi switch), stack painting, hoặc **MPU** đặt vùng cấm cuối stack task. Không MMU → overflow đè task/biến khác = corruption khó lần → bật check khi phát triển.

## 5. Schedulability & Rate Monotonic Scheduling

**RMS**: gán **priority tĩnh theo tần suất** — task chu kỳ ngắn (tần số cao) → priority cao hơn; tối ưu cho task chu kỳ, deadline = chu kỳ, preemptive.

**Hệ có kịp deadline không?** Không chỉ nhìn CPU trung bình. Ví dụ có số cụ thể:

| Task | C (thời gian chạy) | T (chu kỳ) | U = C/T | Priority theo RMS |
|---|---|---|---|---|
| τ₁ đọc sensor | 1 ms | 5 ms | 0.20 | **cao nhất** (T ngắn nhất) |
| τ₂ điều khiển | 2 ms | 10 ms | 0.20 | trung bình |
| τ₃ gửi log | 8 ms | 40 ms | 0.20 | thấp nhất |
| | | **ΣU** | **0.60** | |

- **Ngưỡng Liu & Layland** với n=3: `3 × (2^(1/3) − 1)` = **0.779**.
- ΣU = 0.60 ≤ 0.779 → ✅ **chắc chắn kịp deadline**, không cần phân tích thêm.

Nhưng nếu τ₃ tăng lên 16 ms (U₃ = 0.40, ΣU = 0.80 > 0.779) → **chưa kết luận được là trượt**; ngưỡng chỉ là *điều kiện đủ*, không phải điều kiện cần. Lúc này phải phân tích **response-time** từng task:

```
  R = C + Σ (preemption từ mọi task ưu tiên CAO hơn) + B (blocking)
              ▲                                          ▲
    số lần task ưu tiên cao chen vào trong           thời gian bị task ưu tiên
    khoảng R, mỗi lần tốn C của nó                   THẤP giữ mutex (§2)

  Ví dụ τ₂: R₂ = C₂ + ⌈R₂/T₁⌉ × C₁ + B₂
              = 2  + ⌈R₂/5⌉ × 1   + B₂     (giải lặp tới khi hội tụ)
  Kịp deadline khi  R₂ ≤ T₂ = 10 ms.
```

⚠️ **Hai khoản người ta hay quên** và cũng là hai khoản làm hệ trượt deadline trong thực tế: **blocking** (B — task ưu tiên cao chờ mutex do task thấp giữ, xem priority inversion) và **jitter** (thời điểm sự kiện tới không đều).

Ý phải nêu khi phỏng vấn: đo bằng công cụ, **chừa biên**, tính cả blocking & jitter. Deadline > chu kỳ hoặc task rời rạc → cân nhắc EDF (deadline-based).

## 6. Tick vs tickless

- **Tick**: timer ngắt định kỳ (vd 1ms) làm nhịp — đếm timeout, đánh thức task ngủ, time slicing. Nhược: đánh thức CPU liên tục → tốn điện khi rảnh + jitter.
- **Tickless (low-power)**: khi rảnh, RTOS **tắt tick**, đặt timer đúng bằng thời điểm task kế cần dậy → CPU ngủ sâu lâu hơn → tiết kiệm điện (quan trọng cho thiết bị pin).

## 7. Kiến trúc firmware — chọn thế nào

- **Superloop** (`while(1){ taskA(); taskB(); }` + ngắt): đơn giản nhất, footprint bé — hợp firmware nhỏ, ít task, timing lỏng. Nhược: task chậm làm trễ task khác; thường phải thêm state machine + timer để không block.
- **Event-driven** (superloop + hàng đợi sự kiện, ISR đẩy event): một luồng nhưng phản ứng theo sự kiện, không polling bận → tiết kiệm điện, mở rộng tốt hơn.
- **RTOS**: nhiều task ưu tiên, preemptive → tách concern rõ, đảm bảo task quan trọng đúng hạn; đổi lại tốn RAM (stack mỗi task) + phải bảo vệ dữ liệu chia sẻ + phức tạp hơn.

Chọn: bắt đầu superloop/event-driven nếu đủ; lên RTOS khi có **nhiều task với ràng buộc timing khác nhau** hoặc cần blocking API sạch. "Đơn giản nhất chạy được" trước. (State machine — xương sống firmware — xem [DP-010](../15_prep/mock-interview/bank/design-patterns.md).)

## 8. Bootloader bare-metal & jump-to-app

Bootloader = chương trình nhỏ chạy đầu: kiểm tra có yêu cầu update không, nếu không thì **nhảy vào application**:

```c
// jump to app (Cortex-M): đặt lại vector table + SP + PC của app
void jump_to_app(uint32_t app_base) {
    uint32_t sp = *(uint32_t*)app_base;         // entry 0 = stack pointer của app
    uint32_t pc = *(uint32_t*)(app_base + 4);   // entry 1 = reset vector của app
    SCB->VTOR = app_base;                        // trỏ vector table về app
    __set_MSP(sp);
    ((void(*)(void))pc)();                        // nhảy
}
```

**Update**: bố cục flash **dual-bank** (bootloader + app A/B) — nhận firmware qua UART/CAN/USB/OTA, ghi vào bank không chạy, **verify CRC/chữ ký trước khi kích hoạt**, đổi con trỏ boot; fallback nếu app mới không set cờ "healthy" trong N lần boot; watchdog phủ treo. Nguyên tắc như OTA Linux thu nhỏ: **không được brick**, verify trước khi commit, có đường lùi. Bootloader phải cực ổn định (ghi dở = brick). (So với BSP Linux: [BSP-015 OTA](../15_prep/mock-interview/bank/bsp.md).)

---

## Ôn tập (bank)

[EMB-014](../15_prep/mock-interview/bank/embedded-fundamentals.md) (preemptive/cooperative), [EMB-015](../15_prep/mock-interview/bank/embedded-fundamentals.md) (primitives), [EMB-016](../15_prep/mock-interview/bank/embedded-fundamentals.md) (ISR→task), [EMB-017](../15_prep/mock-interview/bank/embedded-fundamentals.md) (stack sizing), [EMB-018](../15_prep/mock-interview/bank/embedded-fundamentals.md) (RMS), [EMB-019](../15_prep/mock-interview/bank/embedded-fundamentals.md) (tickless), [EMB-020…023](../15_prep/mock-interview/bank/embedded-fundamentals.md) (kiến trúc firmware, bootloader).
