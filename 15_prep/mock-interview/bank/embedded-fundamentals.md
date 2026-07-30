# EMB — Embedded Software nền tảng (bare-metal / MCU / RTOS / firmware)

> Domain `EMB`. Kiến thức nền của Embedded Software **nói chung** — không giả định có Linux/MMU/kernel; bổ trợ cho `DRV`/`BUS`/`BSP` (tầng Linux) và là nền để hiểu sâu chính BSP. Track dùng: `embedded`, `bsp`.
> Nguồn: [08-embedded-systems](../../../08-embedded-systems/). Một số câu là kiến thức nền chung (🆕 so với repo) — link 08 để đọc thêm.
> 🏗️ = câu thiết kế/tình huống.

---

## A — C cho embedded & thanh ghi

#### EMB-001 · 🟢 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**Cách set / clear / toggle / test một bit trong thanh ghi?**
<details><summary>Đáp án</summary>

- **Set** bit n: `REG |= (1u << n);`
- **Clear** bit n: `REG &= ~(1u << n);`
- **Toggle** bit n: `REG ^= (1u << n);`
- **Test** bit n: `if (REG & (1u << n))`
- Nhiều bit cùng lúc dùng mask: `REG = (REG & ~MASK) | (val << SHIFT);` (read-modify-write). Lưu ý: RMW trên thanh ghi phần cứng **không atomic** — nếu ISR cũng đụng thanh ghi đó cần critical section; nhiều SoC có thanh ghi SET/CLR riêng (ghi 1 để set/clear, tránh RMW). Dùng `1u` (unsigned) để tránh UB dịch bit dấu.
</details>

#### EMB-002 · 🟡 · concept · ⭐ · [→ constraints](../../../08-embedded-systems/constraints.md)
**Vì sao embedded ưu tiên fixed-width types (`stdint.h`)? `int` có rủi ro gì?**
<details><summary>Đáp án</summary>

`int`/`long` có **kích thước phụ thuộc nền tảng** (int 16-bit trên vài MCU, 32-bit chỗ khác) → code giả định kích thước sẽ sai khi port, tràn số ngoài dự kiến, layout struct trao đổi qua wire lệch. Dùng `uint8_t/int16_t/uint32_t…` (và `uintptr_t`, `size_t`) để **kích thước xác định**, đúng ý khi thao tác thanh ghi/giao thức/serialize. Kèm: dùng unsigned cho thao tác bit; cẩn thận **integer promotion** (uint8_t được nâng lên int khi tính toán) gây bất ngờ.
</details>

#### EMB-003 · 🟠 · concept · ⭐ · [→ architecture](../../../08-embedded-systems/architecture.md)
**Cách truy cập thanh ghi phần cứng trong C? Vì sao `volatile`? Dùng `union`/bitfield map thanh ghi có an toàn không?**
<details><summary>Đáp án</summary>

Map bằng con trỏ tới `volatile`: `#define REG (*(volatile uint32_t*)0x40021000)` hoặc struct `volatile` đặt tại địa chỉ base. **`volatile`** vì giá trị đổi ngoài luồng compiler thấy (phần cứng) và ghi có side effect → cấm cache/loại bỏ/gộp/reorder truy cập (đọc status 2 lần phải là 2 lần đọc thật). **Bitfield/union map thanh ghi**: tiện đọc nhưng **không portable** — chuẩn C không quy định thứ tự bit trong bitfield, padding, endianness → dễ sai khi đổi compiler/kiến trúc; nhiều coding standard cấm bitfield cho thanh ghi, ưu tiên mask + shift tường minh. Ghi thanh ghi còn cần đúng **thứ tự** (memory barrier) khi có DMA/ngắt.
</details>

#### EMB-004 · 🟡 · concept · [→ memory-model](../../../01-cpp-fundamentals/memory-model.md)
**Vai trò của `static`, `const`, `volatile`, `extern` trong embedded C?**
<details><summary>Đáp án</summary>

- **`static`** (biến file/hàm): giới hạn linkage nội bộ (đóng gói, tránh đụng tên); **`static` local**: giữ giá trị qua các lần gọi, cấp ở `.data`/`.bss` không phải stack.
- **`const`**: dữ liệu chỉ đọc — trên MCU thường được đặt ở **flash (`.rodata`)** thay vì tốn RAM (quan trọng khi RAM ít); bảng tra cứu, chuỗi hằng nên `const`.
- **`volatile`**: cấm compiler tối ưu truy cập — thanh ghi phần cứng, biến bị ISR sửa.
- **`extern`**: khai báo biến/hàm định nghĩa ở TU khác (chia sẻ giữa file).
- Kết hợp hay gặp: `volatile` cho biến ISR; `const volatile` cho thanh ghi chỉ-đọc (status) — vừa cấm ghi vừa cấm tối ưu đọc.
</details>

---

## B — Bộ nhớ bare-metal & khởi động

#### EMB-005 · 🟡 · concept · ⭐ · [→ constraints](../../../08-embedded-systems/constraints.md)
**Các section `.text` / `.data` / `.bss` / `.rodata` chứa gì, nằm ở đâu (flash/RAM)?**
<details><summary>Đáp án</summary>

- **`.text`**: mã lệnh (+ thường `.rodata`) — nằm ở **flash**, thực thi tại chỗ (XIP) hoặc copy vào RAM.
- **`.rodata`**: hằng số/chuỗi chỉ đọc — **flash**.
- **`.data`**: biến toàn cục/static **có giá trị khởi tạo ≠ 0** — *giá trị* lưu ở flash, *biến* sống ở **RAM**; startup copy từ flash → RAM.
- **`.bss`**: biến toàn cục/static **khởi tạo 0 / không khởi tạo** — chỉ chiếm **RAM**, không tốn flash; startup **zero** vùng này.
- Ngoài ra: **stack** và **heap** ở RAM. Hiểu bản đồ này để đọc map file, tính footprint flash/RAM, và biết vì sao biến global chưa khởi tạo vẫn là 0.
</details>

#### EMB-006 · 🟠 · concept · ⭐ · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Chuyện gì xảy ra TRƯỚC `main()` trên MCU (startup / C runtime)?**
<details><summary>Đáp án</summary>

Sau reset, CPU nạp **stack pointer** và **reset vector** từ đầu vector table (flash). Reset handler (startup code, thường asm/crt0) làm: (1) set clock/PLL tối thiểu (đôi khi trong SystemInit); (2) **copy `.data`** từ flash sang RAM; (3) **zero `.bss`**; (4) khởi tạo C runtime (constructor C++ toàn cục nếu có — `__libc_init_array`); (5) gọi `main()`. Nếu bỏ bước copy .data/zero .bss → biến global có giá trị rác → bug "chạy được ở bản này, hỏng ở bản kia". Đây là lý do biến chưa khởi tạo là 0 (nhờ .bss zero), và vì sao main() không bao giờ return trên bare-metal (thường là while(1)).
</details>

#### EMB-007 · 🟠 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Linker script để làm gì trong bare-metal?**
<details><summary>Đáp án</summary>

Linker script mô tả **bản đồ bộ nhớ** của thiết bị (vùng FLASH và RAM: địa chỉ + kích thước) và **đặt section vào đâu**: `.text/.rodata` → FLASH, `.data`/`.bss`/stack/heap → RAM, định nghĩa symbol (`_sdata`, `_edata`, `_sbss`, `_ebss`, `_estack`) mà startup code dùng để copy/zero. Nó cũng đặt **vector table** đúng địa chỉ reset. Sai linker script → link tràn vùng nhớ, đặt sai địa chỉ → không boot. Khi cần vùng đặc biệt (DMA buffer không cache, bootloader/app tách bank, giữ biến qua reset) → khai section riêng trong linker script + attribute.
</details>

#### EMB-008 · 🟠 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Stack và heap trên MCU: rủi ro gì? Phát hiện stack overflow thế nào (không MMU)?**
<details><summary>Đáp án</summary>

RAM ít + **không MMU** → không có guard page: stack lớn quá tràn **âm thầm** đè lên `.bss`/`.data`/heap → corruption khó lần (biến "tự đổi"). Heap: fragmentation + `malloc` không tất định → nhiều hệ **cấm heap sau init** (tĩnh/pool). Phát hiện stack overflow: (1) **stack painting** — điền pattern (0xAA) lúc init, đo mức cao nhất bị đè; (2) đặt **canary**/word chặn cuối stack, kiểm tra định kỳ; (3) **MPU** đặt vùng cấm ngay dưới stack → fault khi tràn; (4) RTOS có hook `stack overflow check`. Sizing stack: cộng worst-case call depth + local lớn + **stack ISR** (ngắt lồng).
</details>

---

## C — Ngắt (bare-metal)

#### EMB-009 · 🟡 · concept · ⭐ · [→ architecture](../../../08-embedded-systems/architecture.md)
**ISR là gì? Quy tắc viết ISR đúng?**
<details><summary>Đáp án</summary>

ISR (Interrupt Service Routine) = hàm chạy khi ngắt xảy ra, ngoài luồng chính. Quy tắc: **ngắn và nhanh** (giữ latency cho ngắt khác); **không blocking/không ngủ**, không chờ; **không `malloc`/`printf`/hàm không reentrant** (giữ khóa nội bộ → deadlock/corruption); **không thao tác nặng** — chỉ ack thiết bị, đọc/ghi thanh ghi tối thiểu, đặt cờ/đẩy dữ liệu vào buffer, rồi báo main loop/task xử lý (deferred). Biến chia sẻ với main phải `volatile`; RMW phần cứng cần cẩn thận. Xóa cờ ngắt đúng chỗ để không bị gọi lại vô hạn.
</details>

#### EMB-010 · 🟠 · concept · ⭐ · [→ constraints](../../../08-embedded-systems/constraints.md)
**Chia sẻ dữ liệu giữa ISR và main loop an toàn thế nào?**
<details><summary>Đáp án</summary>

Ba vấn đề: (1) compiler tối ưu → biến chia sẻ phải **`volatile`** (ISR sửa "ngoài luồng" main thấy); (2) **atomicity** — đọc/ghi biến nhiều byte (uint32 trên MCU 8/16-bit, hoặc struct) có thể bị ISR chen giữa chừng → dữ liệu rách; (3) nhất quán nhiều biến. Giải: với biến đơn ≤ kích thước word + cờ đơn giản → `volatile` + kiểu atomic của phần cứng đủ. Với dữ liệu nhiều byte/nhiều biến → **critical section**: tắt ngắt quanh đoạn đọc/ghi ở phía main (`__disable_irq()/__enable_irq()` hoặc lưu/khôi phục PRIMASK), giữ **cực ngắn**. Mẫu tốt: ISR ghi vào **ring buffer** (SPSC) — main đọc, chỉ cần index atomic, tránh tắt ngắt. `volatile` **không** thay được critical section cho RMW.
</details>

#### EMB-011 · 🟠 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**Vector table và NVIC là gì? Interrupt priority / nesting hoạt động ra sao?**
<details><summary>Đáp án</summary>

**Vector table**: mảng địa chỉ handler (ở đầu flash) — mỗi nguồn ngắt một entry; CPU tra bảng để nhảy vào ISR đúng. **NVIC** (Nested Vectored Interrupt Controller, Cortex-M): enable/disable từng ngắt, đặt **priority**, hỗ trợ **nesting** — ngắt priority cao hơn **preempt** ISR đang chạy (số priority nhỏ = ưu tiên cao trên Cortex-M). Cấu hình: nhóm priority (preemption vs sub-priority). Lưu ý: ngắt cùng/thấp hơn priority phải chờ; đặt priority sai gây jitter/latency. Có ngắt không maskable (NMI). Tránh giữ critical section (tắt ngắt) lâu vì chặn cả ngắt ưu tiên cao.
</details>

#### EMB-012 · 🟡 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**Interrupt latency là gì? Yếu tố nào ảnh hưởng?**
<details><summary>Đáp án</summary>

Latency = thời gian từ lúc sự kiện ngắt xảy ra tới lúc ISR bắt đầu chạy (đôi khi tính tới khi xử lý xong). Yếu tố: (1) **critical section** đang tắt ngắt (đoạn code giữ ngắt tắt càng lâu, latency worst-case càng lớn); (2) ISR **priority thấp hơn** đang chạy chặn (không preempt được); (3) thời gian lưu context/vào handler (một phần cứng định); (4) wait states flash/clock thấp. Muốn latency thấp & tất định: giữ critical section cực ngắn, đặt priority hợp lý, ISR gọn. Đây là số phải **đo** cho hệ real-time (worst-case, không phải trung bình).
</details>

#### EMB-013 · 🟠 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Hàm reentrant là gì? Vì sao quan trọng với ISR?**
<details><summary>Đáp án</summary>

Hàm **reentrant** = có thể bị ngắt giữa chừng và gọi lại (từ ISR hoặc thread khác) mà vẫn đúng — không dùng **biến static/global có trạng thái** không được bảo vệ, không trả con trỏ tới buffer tĩnh dùng chung. Quan trọng vì ISR có thể chen vào giữa một hàm main đang chạy: nếu cả hai gọi cùng một hàm **non-reentrant** (vd `strtok`, một số `malloc`, hàm dùng buffer tĩnh) → corruption. Cách viết reentrant: dùng biến local (stack)/tham số thay vì static, tránh trạng thái chia sẻ, hoặc bảo vệ bằng critical section. Liên quan nhưng khác thread-safe (reentrant nghiêm ngặt hơn: an toàn cả khi tự gọi lại chính mình qua ngắt).
</details>

---

## D — RTOS concepts

#### EMB-014 · 🟡 · concept · ⭐ · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Preemptive vs cooperative scheduling khác nhau thế nào?**
<details><summary>Đáp án</summary>

**Cooperative**: task chạy tới khi **tự nhường** (gọi yield/block) → đơn giản, không cần bảo vệ nhiều (không bị chen bất ngờ), nhưng **một task tham lam treo cả hệ**, latency phụ thuộc task khác → khó đảm bảo real-time. **Preemptive**: scheduler **cướp CPU** theo priority/tick → task ưu tiên cao chạy ngay, tất định hơn, hợp real-time; đổi lại phải **bảo vệ dữ liệu chia sẻ** (mutex/critical section) vì bị chen bất cứ lúc nào, và có chi phí context switch. RTOS phổ biến (FreeRTOS, Zephyr) mặc định preemptive theo priority.
</details>

#### EMB-015 · 🟡 · concept · ⭐ · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**RTOS primitives: semaphore / mutex / queue / event flag — dùng cái nào khi nào?**
<details><summary>Đáp án</summary>

- **Mutex**: loại trừ lẫn nhau bảo vệ tài nguyên chia sẻ; có **ownership** + **priority inheritance** (chống priority inversion). Chỉ task giữ mới trả.
- **Binary/counting semaphore**: **báo hiệu** (không ownership) — ISR "give" báo task "take"; counting cho đếm tài nguyên/sự kiện. Không dùng semaphore thay mutex (mất PI).
- **Queue**: truyền **dữ liệu có ranh giới** giữa task/ISR (thread-safe sẵn), thường là cách chính để chuyển dữ liệu.
- **Event flag/group**: chờ **tổ hợp nhiều sự kiện** (AND/OR nhiều bit) — vd chờ cả "data ready" và "buffer free".
- Quy tắc: bảo vệ vùng tới hạn → mutex; báo hiệu ISR→task → semaphore; chuyển dữ liệu → queue; chờ nhiều điều kiện → event flag.
</details>

#### EMB-016 · 🟠 · concept · ⭐ · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**ISR giao tiếp với task trong RTOS thế nào? (deferred interrupt processing)**
<details><summary>Đáp án</summary>

ISR phải ngắn → chỉ làm tối thiểu rồi **đẩy việc xuống task**: "give" semaphore / gửi vào queue / set event flag để đánh thức một task xử lý (mô hình top/bottom-half). Bắt buộc dùng **API bản `…FromISR`** (vd `xSemaphoreGiveFromISR`, `xQueueSendFromISR` của FreeRTOS) vì API thường không an toàn trong ngữ cảnh ngắt. Nếu việc đánh thức làm một task ưu tiên cao hơn sẵn sàng, ISR set cờ `xHigherPriorityTaskWoken` và gọi **`portYIELD_FROM_ISR`** để context switch ngay khi ra khỏi ISR (giảm latency). Không được block/chờ trong ISR. Đây là bản RTOS của "top half báo, bottom half xử lý".
</details>

#### EMB-017 · 🟠 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Mỗi task có stack riêng — sizing stack và phát hiện overflow trong RTOS?**
<details><summary>Đáp án</summary>

Mỗi task có **stack riêng** (cấp lúc tạo task); tổng RAM = Σ stack task + heap RTOS + .data/.bss → dễ cạn. **Sizing**: ước worst-case call depth + local lớn + dùng công cụ high-water-mark (`uxTaskGetStackHighWaterMark`) để đo mức thực rồi chừa biên; nhớ cộng chi phí **ngắt** chạy trên stack task (hoặc stack riêng tùy port). **Phát hiện overflow**: RTOS có hook (`configCHECK_FOR_STACK_OVERFLOW` — kiểm pattern/pointer khi switch), stack painting, hoặc **MPU** đặt vùng cấm cuối stack task. Overflow không MMU = đè task/biến khác → corruption khó lần, nên bật check khi phát triển.
</details>

#### EMB-018 · 🔴 · concept · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Rate Monotonic Scheduling là gì? Cách nghĩ về "hệ có kịp deadline không"?**
<details><summary>Đáp án</summary>

RMS: gán **priority tĩnh theo tần suất** — task chu kỳ ngắn (tần số cao) → priority cao hơn; đây là gán priority tối ưu cho task chu kỳ, deadline = chu kỳ, preemptive. **Schedulability**: một điều kiện đủ (Liu & Layland) là tổng utilization ΣCᵢ/Tᵢ ≤ n(2^{1/n}−1) (→ ~0.69 khi n lớn) thì chắc chắn kịp; vượt ngưỡng thì phải phân tích **response-time** từng task (tính worst-case gồm cả preemption từ task ưu tiên cao + blocking). Ý niệm phải nêu khi phỏng vấn: deadline có kịp không **không chỉ nhìn CPU trung bình** mà nhìn **worst-case + blocking (priority inversion) + jitter**; đo bằng công cụ, chừa biên. Deadline > chu kỳ hoặc task rời rạc → EDF/deadline-based.
</details>

#### EMB-019 · 🟡 · concept · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Tick vs tickless; time slicing là gì?**
<details><summary>Đáp án</summary>

**Tick**: RTOS dùng một timer ngắt định kỳ (vd 1ms) làm nhịp — đếm timeout, đánh thức task ngủ, và **time slicing** (round-robin giữa task cùng priority mỗi tick). Nhược: tick đều đặn **đánh thức CPU liên tục** → tốn điện khi rảnh, và jitter do tick. **Tickless (low-power)**: khi hệ rảnh, RTOS **tắt tick**, đặt timer đúng bằng thời điểm task kế cần dậy → CPU ngủ sâu lâu hơn, tiết kiệm điện (quan trọng cho thiết bị pin). Time slicing chỉ áp cho task cùng priority; task priority cao hơn luôn preempt.
</details>

---

## E — Kiến trúc firmware

#### EMB-020 · 🟡 · design · ⭐ · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Superloop vs RTOS vs event-driven — chọn kiến trúc firmware thế nào?**
<details><summary>Đáp án</summary>

- **Superloop** (`while(1){ taskA(); taskB(); }` + ngắt): đơn giản nhất, footprint bé, dễ suy luận — hợp firmware nhỏ, ít task, timing lỏng. Nhược: khó đảm bảo timing khi task nhiều/không đều (task chậm làm trễ task khác); thường phải thêm state machine + timer để không block.
- **Event-driven** (superloop + hàng đợi sự kiện, ISR đẩy event): vẫn một luồng nhưng phản ứng theo sự kiện, không polling bận — tiết kiệm điện, mở rộng tốt hơn.
- **RTOS**: nhiều task ưu tiên, preemptive → tách concern rõ, đảm bảo task quan trọng chạy đúng hạn; đổi lại tốn RAM (stack mỗi task) + phải bảo vệ dữ liệu chia sẻ + độ phức tạp.
- Chọn: bắt đầu superloop/event-driven nếu đủ; lên RTOS khi có **nhiều task với ràng buộc timing khác nhau** hoặc cần blocking API sạch. "Đơn giản nhất chạy được" trước.
</details>

#### EMB-021 · 🟠 · design · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Hiện thực state machine trong firmware — cách nào, đánh đổi?**
<details><summary>Đáp án</summary>

State machine là xương sống firmware/protocol. Cách: (1) **enum + switch** (switch theo state, xử lý event) — đơn giản, tất định, không cấp phát động, dễ review, footprint biết trước → mặc định embedded; (2) **table-driven** (bảng [state][event] → hành động/state kế) — gọn khi nhiều state/event, dễ mở rộng, tách logic khỏi dữ liệu; (3) **OOP State pattern** (mỗi state một class) — chỉ đáng khi logic mỗi state phức tạp, chấp nhận vtable/gián tiếp. Nguyên tắc: định nghĩa state/event/transition rõ ràng, xử lý event lạ ở mọi state (không "rơi"), tránh cấu trúc động. *(Xem thêm [DP-010](design-patterns.md).)*
</details>

#### EMB-022 · 🟠 · design · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Bootloader bare-metal + firmware update trên MCU hoạt động thế nào?**
<details><summary>Đáp án</summary>

Bootloader = chương trình nhỏ chạy đầu tiên: kiểm tra có yêu cầu update không, nếu không thì **nhảy vào application** (đặt lại vector table về địa chỉ app: trên Cortex-M set VTOR, nạp SP + reset vector của app rồi jump). Update: (1) bố cục flash **dual-bank A/B** (hoặc bootloader + 1 app + staging) — nhận firmware qua UART/CAN/USB/OTA, ghi vào bank không chạy, **verify CRC/chữ ký trước khi kích hoạt**, đổi con trỏ boot; (2) fallback: nếu app mới không set cờ "healthy" trong N lần boot → bootloader quay về bank cũ; watchdog phủ treo. Nguyên tắc như OTA Linux nhưng thu nhỏ: **không được brick**, verify trước khi commit, có đường lùi. Bootloader phải cực ổn định (ghi dở nó = brick).
</details>

#### EMB-023 · 🟡 · concept · [→ architecture](../../../08-embedded-systems/architecture.md)
**HAL / phân tầng driver trong firmware bare-metal — vì sao?**
<details><summary>Đáp án</summary>

Tách **logic ứng dụng** khỏi **truy cập phần cứng** qua một lớp interface (HAL): app gọi `led_on()` / `sensor_read()` thay vì đụng thanh ghi trực tiếp. Lợi: (1) **port sang chip/board khác** chỉ thay lớp HAL, app không đổi (đúng chất công việc multi-chipset); (2) **test trên host** — thay HAL bằng mock/stub, chạy logic + unit test trên PC (không cần phần cứng thật); (3) đọc code rõ ràng, đóng gói chi tiết register. Đánh đổi: thêm một lớp gián tiếp (chi phí nhỏ; hot path có thể inline/`static inline`). Vendor HAL (STM32 HAL/LL, CMSIS) là ví dụ; nhiều team viết HAL mỏng riêng để kiểm soát. *(Xem [BSP-001](bsp.md), [DP-011](design-patterns.md).)*
</details>

---

## F — Số học & độ tin cậy

#### EMB-024 · 🟠 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Fixed-point vs floating-point khi MCU không có FPU?**
<details><summary>Đáp án</summary>

MCU không FPU → phép float được **emulate bằng phần mềm** (chậm hàng chục–trăm lần, tốn code) → tránh trong hot path/ISR. **Fixed-point**: biểu diễn số thực bằng số nguyên với "điểm thập phân ảo" (vd Q16.16: 32-bit, 16 bit phần nguyên + 16 bit phân) → dùng số học nguyên nhanh + tất định. Đánh đổi: phải tự quản scale, cẩn thận **overflow khi nhân** (Q16.16 × Q16.16 cần 64-bit trung gian rồi dịch), độ chính xác cố định. Khi nào chọn: có FPU (Cortex-M4F/M7) hoặc tính toán ít → float tiện; không FPU + tính nhiều/real-time → fixed-point. Nêu được: float trên MCU không FPU là **bẫy hiệu năng/tất định** kinh điển.
</details>

#### EMB-025 · 🟡 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Integer overflow/underflow — bug embedded kinh điển, phòng thế nào?**
<details><summary>Đáp án</summary>

Số nguyên kích thước cố định (uint8/16/32) tràn khi vượt max → quay vòng (unsigned) hoặc **UB (signed overflow)**. Bug hay gặp: bộ đếm ms tràn (so sánh `time > deadline` sai khi wrap), trừ hai timestamp ra số âm ở unsigned, tích lũy cảm biến tràn, `a+b` tràn trước khi gán vào biến lớn hơn. Phòng: (1) chọn kiểu đủ lớn + fixed-width rõ ràng; (2) so sánh thời gian bằng **hiệu** chịu wrap: `(int32_t)(now - deadline) >= 0` thay vì so trực tiếp; (3) kiểm biên trước phép tính; (4) unsigned cho biến chỉ tăng, cẩn thận integer promotion; (5) `-fsanitize=undefined` khi test trên host, bật cảnh báo. Với signed, tràn là UB → optimizer có thể làm điều bất ngờ.
</details>

#### EMB-026 · 🟡 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**CRC / checksum — vì sao cần, khác nhau thế nào?**
<details><summary>Đáp án</summary>

Khi truyền qua UART/CAN/RF hay lưu vào flash, dữ liệu có thể **hỏng bit** (nhiễu, mòn flash, mất điện khi ghi) → gắn mã kiểm tra để **phát hiện lỗi**. **Checksum** (cộng byte) rẻ nhưng yếu — không bắt được nhiều lỗi (đảo thứ tự, lỗi bù trừ). **CRC** (Cyclic Redundancy Check) dựa trên chia đa thức → bắt lỗi mạnh hơn nhiều (mọi lỗi 1–2 bit, burst error ≤ độ dài CRC), là chuẩn cho frame giao thức + xác thực firmware image; nhiều MCU có **CRC phần cứng**. Lưu ý: CRC chỉ **phát hiện** lỗi, không sửa (muốn sửa cần ECC/FEC) và **không phải bảo mật** (chống sửa cố ý cần chữ ký/HMAC). Firmware update: CRC/hash để kiểm toàn vẹn + chữ ký để chống giả mạo.
</details>

#### EMB-027 · 🟡 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**MISRA C là gì, vì sao dùng trong embedded? Cho vài ví dụ quy tắc.**
<details><summary>Đáp án</summary>

MISRA C = bộ **coding guideline** cho C trong hệ nhúng an toàn/quan trọng (ô tô, y tế, công nghiệp) nhằm né vùng **không xác định/dễ lỗi** của C và tăng tính portable/kiểm chứng được. Ví dụ quy tắc: cấm/hạn chế cấp phát động sau init; cấm `goto` bừa; không dựa vào thứ tự đánh giá; ép kiểu tường minh, không trộn signed/unsigned ngầm; mỗi `switch` có `default`, mỗi `case` có `break`; không dùng `//` (bản cũ); một điểm return… Enforce bằng **static analyzer** (Coverity, PC-lint, cppcheck) trong CI. Đánh đổi: code cứng nhắc hơn, đôi khi phải "deviation" có ghi chú. Nêu được: MISRA là về **giảm rủi ro và audit được**, không phải tối ưu.
</details>

---

## G — Low-power (MCU)

#### EMB-028 · 🟡 · concept · [→ constraints](../../../08-embedded-systems/constraints.md)
**Các low-power mode của MCU (sleep/stop/standby) — đánh đổi và wakeup source?**
<details><summary>Đáp án</summary>

Thang tiết kiệm điện đổi lấy **thời gian/khả năng đánh thức + giữ trạng thái** (tên khác nhau theo hãng, mô hình chung): **Sleep/Idle** — tắt CPU clock, peripheral vẫn chạy, đánh thức nhanh bởi bất kỳ ngắt nào, giữ toàn bộ RAM. **Stop/Deep-sleep** — tắt phần lớn clock/PLL, giữ RAM (self-refresh), dòng thấp hơn nhiều, đánh thức bởi số ít nguồn (RTC alarm, EXTI GPIO), exit latency cao hơn. **Standby/Shutdown** — tắt gần hết, **mất RAM** (chỉ giữ vài thanh ghi backup + RTC), dòng µA, "đánh thức" gần như reset (chạy lại từ đầu), nguồn giới hạn (WKUP pin, RTC). Chọn theo: cần đánh thức nhanh + giữ ngữ cảnh → sleep; ngủ lâu tiết kiệm tối đa + chấp nhận khởi động lại → standby. Wakeup source phải cấu hình trước khi vào mode.
</details>

#### EMB-029 · 🟠 · design · 🏗️ · [→ constraints](../../../08-embedded-systems/constraints.md)
**Thiết kế firmware chạy pin lâu — bạn tối ưu điện thế nào?**
<details><summary>Đáp án (khung)</summary>

- **Kiến trúc event-driven, không polling bận**: CPU ngủ (low-power mode) là mặc định, chỉ dậy khi có ngắt/sự kiện; dùng **tickless** RTOS.
- **Ngủ sâu nhất có thể** giữa các sự kiện; chọn mode theo yêu cầu đánh thức (RTC cho chu kỳ dài, GPIO cho nút bấm).
- **Tắt peripheral không dùng** (clock gating, tắt ADC/radio khi rảnh), hạ tần số clock khi tải nhẹ (DVFS nếu có), giảm điện áp.
- **Gom việc theo lô** (đọc cảm biến rồi ngủ, gửi radio theo batch — radio/TX tốn điện nhất) thay vì làm rải rác.
- **Đo thật**: dùng power profiler/ampe kế đo dòng từng trạng thái, tìm "kẻ ngốn điện" (thường là radio TX, LED, peripheral quên tắt, MCU không vào được deep sleep vì một ngắt dựng dậy liên tục).
- Đánh đổi: đánh thức sâu → latency cao; gom batch → độ trễ dữ liệu. Nêu số: duty cycle, dòng trung bình → tính tuổi thọ pin.
</details>

---

## H — Debug phần cứng

#### EMB-030 · 🟡 · concept · [→ debugging](../../../09-debugging/)
**JTAG và SWD khác nhau? Dùng để làm gì?**
<details><summary>Đáp án</summary>

Cả hai là giao diện **debug/lập trình phần cứng** cho MCU: nạp firmware, đặt breakpoint, đọc/ghi thanh ghi & bộ nhớ, single-step, xem trạng thái CPU khi nó đang chạy/dừng (in-circuit debug qua probe như ST-Link, J-Link). **JTAG**: chuẩn cũ, nhiều dây (TCK/TMS/TDI/TDO/…), hỗ trợ boundary scan + chuỗi nhiều chip. **SWD** (Serial Wire Debug, ARM): chỉ **2 dây** (SWCLK + SWDIO) làm gần hết việc JTAG cho debug — tiết kiệm chân (quan trọng trên MCU nhỏ), phổ biến trên Cortex-M. Thường kèm **SWO** (trace output, `printf` qua ITM). Không có debugger đầy đủ như GDB trên target nhưng cho phép debug mức thanh ghi thật.
</details>

#### EMB-031 · 🟡 · concept · ⭐ · [→ tools](../../../09-debugging/tools.md)
**Không có debugger đầy đủ, bạn debug firmware thế nào?**
<details><summary>Đáp án</summary>

- **printf qua UART**: kênh log kinh điển — in trạng thái/biến ra serial console. Cẩn thận: printf chậm + không reentrant → không gọi trong ISR/hot path; dùng buffer + gửi ngoài ISR.
- **Semihosting / SWO(ITM)**: in qua probe debug không cần UART riêng (chậm, chỉ khi có debugger).
- **GPIO/LED toggle** = "printf bằng chân": bật/tắt chân đánh dấu điểm code, đo bằng **logic analyzer/oscilloscope** → thấy timing thật (đo latency ISR, chu kỳ task) mà không làm chậm như printf.
- **Logic analyzer/scope**: xem tín hiệu bus (I2C/SPI/UART) thật để tách lỗi phần mềm vs phần cứng (câu "driver chạy nhưng chân không ra tín hiệu").
- **Trạng thái giữ qua reset**: ghi mã lỗi vào backup register/vùng RAM không bị zero để đọc sau khi crash/reset.
- Nguyên tắc: chọn công cụ **ít làm nhiễu timing** nhất cho bug timing (GPIO+scope > printf).
</details>

#### EMB-032 · 🟠 · concept · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Hard fault trên Cortex-M — bạn điều tra thế nào?**
<details><summary>Đáp án</summary>

Hard fault = CPU gặp lỗi nghiêm trọng (truy cập bộ nhớ sai, lệnh không hợp lệ, chia 0 nếu bật, unaligned access, escalate từ fault khác). Điều tra: (1) đọc **fault status registers** (CFSR/HFSR/MMFAR/BFAR trên Cortex-M) — cho biết *loại* lỗi (bus fault? usage fault? địa chỉ nào); (2) lấy **stacked frame** mà CPU đẩy khi vào fault (R0–R3, R12, **LR, PC, xPSR**) — **PC** cho biết lệnh gây lỗi, LR đường về; đọc từ stack trong fault handler; (3) map PC → dòng source bằng map file/`addr2line`/debugger. Nguyên nhân hay gặp: dereference con trỏ null/dangling, **stack overflow** đè vùng khác, gọi qua con trỏ hàm rác, unaligned access, truy cập peripheral chưa bật clock. Viết một **HardFault_Handler** in các thanh ghi này ra UART là công cụ vàng ngoài field.
</details>

---
⬅️ [Bank index](README.md)
