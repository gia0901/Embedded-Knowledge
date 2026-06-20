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

- **Heap fragmentation**: cấp phát/giải phóng động lâu dài làm bộ nhớ phân mảnh → `malloc` thất bại dù tổng còn trống, và thời gian cấp phát **không tất định**. Nguy hiểm cho hệ chạy liên tục/realtime.
- **Giải pháp ưu tiên:**
  - **Cấp phát tĩnh** (biến global/`static`, buffer kích thước cố định) — biết footprint lúc biên dịch.
  - **Stack** cho dữ liệu vòng đời ngắn (nhưng stack nhỏ — cẩn thận overflow, đệ quy, mảng lớn).
  - **Memory pool / fixed-block allocator**: cấp phát từ pool các khối cùng kích thước → tất định, không phân mảnh.
  - Tránh `malloc/new` trong vòng lặp nóng / sau giai đoạn init; nhiều coding standard embedded (MISRA) hạn chế cấp phát động.
- **Tiết kiệm flash**: `-Os` (tối ưu kích thước), loại bỏ code/feature không dùng (LTO, `--gc-sections`), tránh template/STL phình code nếu eo hẹp, dùng `const` để dữ liệu nằm ở flash (read-only) thay vì RAM.
- Đo footprint: `size`, map file của linker, theo dõi RAM/stack high-water mark.

---

## 3. Ràng buộc năng lượng

Nhiều thiết bị chạy pin → phần mềm quyết định tuổi thọ:
- **Sleep modes**: đưa CPU/peripheral vào trạng thái ngủ sâu khi rảnh, thức dậy bằng interrupt (vd RTC, GPIO). "Race to sleep": làm xong nhanh rồi ngủ.
- **Interrupt thay polling**: polling giữ CPU bận → tốn điện; interrupt cho CPU ngủ giữa các sự kiện.
- **Tắt peripheral & clock gating**: tắt module (ADC, UART...) và clock khi không dùng.
- **Hạ tần số/điện áp** (DVFS) khi tải thấp.
- Đánh đổi: hiệu năng/độ trễ vs tiết kiệm điện — thiết kế theo profile sử dụng thực tế.

---

## 4. Độ tin cậy & an toàn

Thiết bị thường chạy không người giám sát, lâu dài, môi trường khắc nghiệt:
- **Watchdog timer**: phần cứng đếm ngược; phần mềm phải "kick" định kỳ. Nếu hệ treo (không kick), watchdog **reset** thiết bị → tự phục hồi. Phải kick đúng chỗ (không kick mù trong khi tác vụ đã chết).
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

- **Fixed-point arithmetic**: khi không có FPU, dùng số nguyên/Qm.n thay float → nhanh & tất định.
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
