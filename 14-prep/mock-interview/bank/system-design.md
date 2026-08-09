# SD — Tư duy & System Design

> Domain `SD`. Nhiều câu **mở** — chấm theo *cách tiếp cận*, không đáp án duy nhất. Track dùng: `system-design`, `bsp`, `cpp-system`.

---

#### SD-001 · 🟡 · concept · [→ problem-solving](../../../10-thinking/problem-solving.md)
**Khi gặp vấn đề kỹ thuật chưa từng thấy, bạn tiếp cận thế nào?**
<details><summary>Đáp án</summary>

Quy trình: làm rõ vấn đề thực sự + ràng buộc + tiêu chí thành công (tránh giải sai bài) → chia nhỏ thành phần giải được → phác vài hướng, so sánh đánh đổi, chọn (bắt đầu từ giải pháp đơn giản nhất chạy được rồi tối ưu) → triển khai từng phần, kiểm chứng liên tục → đánh giá đúng/đủ tốt cho ràng buộc chưa. Xuyên suốt suy luận từ first principles và nêu rõ đánh đổi.
</details>

#### SD-002 · 🟡 · concept · [→ problem-solving](../../../10-thinking/problem-solving.md)
**Vì sao "không có giải pháp tốt nhất, chỉ có đánh đổi"?**
<details><summary>Đáp án</summary>

Mọi quyết định kỹ thuật hy sinh mặt này để được mặt kia; "tốt nhất" chỉ có nghĩa trong một ngữ cảnh/ràng buộc — giải pháp tối ưu trên server có thể tệ trên MCU 64KB RAM. Các trục: thời gian↔bộ nhớ, hiệu năng↔đơn giản, tốc độ phát triển↔tối ưu, linh hoạt↔đơn giản, throughput↔latency, tổng quát↔chuyên biệt. Luôn hỏi: tối ưu cho cái gì, trong ràng buộc nào?
</details>

#### SD-003 · 🟡 · concept · [→ problem-solving](../../../10-thinking/problem-solving.md)
**Suy luận từ first principles nghĩa là gì? Ví dụ.**
<details><summary>Đáp án</summary>

Quay về nguyên lý cơ bản nhất và suy luận lên thay vì sao chép khuôn mẫu ("cargo cult"). Hỏi "tại sao" liên tục tới khi chạm nguyên lý không chia nhỏ hơn. Ví dụ: thay vì mặc định "cần mutex", hỏi tại sao → vì có dữ liệu chia sẻ bị ghi đồng thời → nếu thiết kế mỗi thread giữ bản riêng (không chia sẻ mutable) thì loại bỏ được khóa, vừa đúng vừa nhanh hơn.
</details>

#### SD-004 · 🟠 · design · [→ system-design](../../../10-thinking/system-design.md)
**Khi được giao bài system design mơ hồ, bước đầu tiên?**
<details><summary>Đáp án</summary>

Làm rõ yêu cầu trước khi vẽ: yêu cầu chức năng (làm gì, input/output, use case) và đặc biệt **phi chức năng** (hiệu năng, RAM/flash, điện, độ tin cậy, realtime, boot time — với embedded chính ràng buộc này định hình kiến trúc), phạm vi (trong/ngoài scope), nêu giả định. Giải đúng một bài sai là vô nghĩa.
</details>

#### SD-005 · 🟠 · design · [→ system-design](../../../10-thinking/system-design.md)
**System design cho embedded khác web/server scale thế nào?**
<details><summary>Đáp án</summary>

Ràng buộc khác: web tối ưu throughput/latency/scale ngang (thêm máy, cache phân tán) cho triệu request, lỗi xử lý bằng retry/redundancy, deploy liên tục. Embedded giới hạn RAM/flash/CPU/điện/realtime, "nhiều" là vài KB/mW, **thường không scale ngang** (phần cứng cố định), lỗi xử lý bằng watchdog/fail-safe (không người can thiệp), cập nhật firmware khó nên phải an toàn (A/B+rollback). Đừng bê tư duy "thêm server" vào embedded.
</details>

#### SD-006 · 🟠 · design · [→ system-design](../../../10-thinking/system-design.md)
**Nguyên tắc thiết kế nào giúp hệ thống dễ bảo trì/mở rộng?**
<details><summary>Đáp án</summary>

Tách concern/module hóa (mỗi module một trách nhiệm rõ), interface ổn định + coupling thấp + information hiding (đổi nội bộ không lan ra), tránh chia sẻ trạng thái mutable, trạng thái xác định + xử lý lỗi đầy đủ (fail to safe state), tách logic khỏi I/O phần cứng (HAL) để test trên host, YAGNI (không over-engineer).
</details>

#### SD-007 · 🟠 · design · [→ system-design](../../../10-thinking/system-design.md)
**Thiết kế dễ test khi phụ thuộc phần cứng?**
<details><summary>Đáp án</summary>

Tách logic khỏi truy cập phần cứng bằng Hardware Abstraction Layer/interface: logic gọi qua interface thay vì đụng thanh ghi/driver trực tiếp, nên test có thể thay phần cứng bằng mock/stub và chạy logic trên host (nhanh, lặp lại được). Module coupling thấp + inject phụ thuộc (thời gian/IO) để kiểm soát trong test. Build cùng code trên host để chạy ASan/TSan.
</details>

#### SD-008 · 🔴 · design · 🏗️ · [→ system-design](../../../10-thinking/system-design.md)
**Thiết kế phần mềm thu thập dữ liệu sensor trên thiết bị nhúng Linux.**
<details><summary>Đáp án (khung)</summary>

- **Requirements**: số sensor, bus (I2C/SPI), tần số lấy mẫu, lưu local hay gửi server, mất dữ liệu chấp nhận được không, realtime cứng không.
- **Constraints**: RAM/flash, có mạng/pin không, tần số → quyết định buffer & lưu trữ.
- **High-level**: Sensor HAL → Acquisition (interrupt+DMA theo lịch) → Ring buffer → Processing → Storage (vòng, chống mòn flash) / Uploader (mạng, retry); systemd quản service, watchdog giám sát.
- **Deep dive**: interrupt+DMA thay polling; ring buffer producer–consumer, đồng bộ mutex+cv hoặc lock-free.
- **Trade-offs**: poll vs interrupt+DMA; ghi thẳng flash (mòn) vs gom batch; gửi realtime vs gom rồi gửi.
- **Failure**: mất điện khi ghi (atomic/journaling), mất mạng (buffer+retry), sensor lỗi (timeout, không treo hệ), treo (watchdog).
- Cách trình bày (làm rõ → chia phần → đào sâu chỗ khó → đánh đổi → lường lỗi) quan trọng hơn "đáp án".
</details>

#### SD-009 · 🔴 · design · 🏗️ · ⭐ · [→ api-design](../../../07-shared-libraries/api-design.md), [abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Thiết kế một C++ shared library/API cho lớp trên dùng. Bạn quan tâm gì?**
<details><summary>Đáp án</summary>

- **API tốt**: tối thiểu (mọi thứ public là cam kết maintain), che giấu chi tiết, nhất quán, dễ dùng đúng/khó dùng sai (kiểu mạnh, RAII, `[[nodiscard]]`), tài liệu hóa hợp đồng (tiền/hậu điều kiện, ownership, thread-safety).
- **Ownership rõ ràng** qua kiểu: trả `unique_ptr` (sở hữu), nhận `const&`/`span` (mượn).
- **Error handling nhất quán**: exception (C++ thuần) hoặc mã lỗi/`expected` (biên giới C/embedded).
- **Ổn định ABI**: pimpl để giấu data member; biên giới C (`extern "C"`, POD, opaque handle) nếu cần đa compiler/ngôn ngữ; versioning kỷ luật (soname theo major, không đổi chữ ký/layout đã phát hành — chỉ thêm).
- Cẩn thận virtual function trong interface public (thêm virtual phá vtable).
</details>

#### SD-010 · 🔴 · design · 🏗️ · ⭐ · [→ abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Thư viện cập nhật phiên bản mới khiến app khách hàng (không build lại) crash. Nguyên nhân khả dĩ và cách tránh?**
<details><summary>Đáp án</summary>

Nguyên nhân khả dĩ: **ABI break** dù API không đổi (app không build lại nên dùng binary cũ với `.so` mới). Vd: thêm data member vào struct/class public (đổi sizeof/offset), thêm/đổi thứ tự virtual function (đổi vtable), đổi chữ ký, đổi kích thước/alignment kiểu. Cách tránh: giữ ABI tương thích — pimpl giấu data member, chỉ thêm hàm mới thay vì sửa cái cũ, không đổi layout đã phát hành, thận trọng virtual; nếu buộc phá ABI thì tăng major + đổi soname (`libfoo.so.2`) để hai bản cùng tồn tại; dùng symbol versioning; kiểm tra bằng `abidiff`/abi-compliance-checker trước khi phát hành.
</details>

#### SD-011 · 🔴 · design · 🏗️ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md), [rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Thiết kế cơ chế giao tiếp giữa một process realtime và một process xử lý/giao diện trên cùng SoC.**
<details><summary>Đáp án</summary>

Tách vai trò (realtime đọc/điều khiển, process kia xử lý/UI/mạng). Giao tiếp cùng máy hiệu năng cao → **shared memory** (zero-copy) tổ chức ring buffer cho luồng dữ liệu lớn, đồng bộ bằng semaphore/mutex process-shared đặt trong vùng shm; dùng **eventfd** hoặc Unix domain socket để báo hiệu/điều khiển có ranh giới. Nếu hệ heterogeneous (Cortex-A Linux + Cortex-M RTOS) thì shared memory + mailbox/RPMsg (AMP). Đánh đổi: shared memory nhanh nhất nhưng tự đồng bộ (dễ sai); message queue/socket an toàn hơn nhưng có copy. Lường lỗi: bên realtime không được block chờ bên kia (buffer + non-blocking), watchdog cho cả hai.
</details>

#### SD-012 · 🔴 · design · 🏗️ · ⭐ · [→ system-design §6.2](../../../10-thinking/system-design.md), [boot-process](../../../08-embedded-systems/boot-process.md)
**Thiết kế hệ thống cập nhật firmware (OTA) an toàn — không brick dù mất điện giữa chừng.**
<details><summary>Đáp án (khung)</summary>

- **Requirements**: nguồn ảnh (OTA/USB/thẻ), kích thước, downtime cho phép, cần ký số không, có cập nhật bootloader/kernel không.
- **Constraints**: flash đủ 2 slot? RAM verify hash/chữ ký? nguồn ổn định? bootloader chọn được slot?
- **High-level**: Downloader (resume) → Verifier (**hash + chữ ký** trước khi tin) → Writer (ghi **slot B** khi **A** đang chạy) → Bootloader (cờ "thử B") → Health-check sau boot (OK→**commit**, lỗi→**rollback** A).
- **Deep dive**: **A/B partition** + cờ `boot_next`/`try_count`; chuyển slot **atomic** (đổi cờ, không ghi đè đang chạy); chỉ mark bootable **sau** verify; **anti-rollback** bằng version counter.
- **Trade-offs**: A/B (tốn gấp đôi flash, an toàn) vs in-place+recovery (ít flash, rủi ro); ký số (an toàn, cần key mgmt) vs chỉ CRC (chống hỏng, không chống giả).
- **Failure**: mất điện khi ghi B → A vẫn boot (chưa chuyển cờ); bản mới boot lỗi → watchdog + hết `try_count` → tự rollback A; ảnh giả → chặn ở Verifier.
</details>

#### SD-013 · 🔴 · design · 🏗️ · ⭐ · [→ system-design §6.3](../../../10-thinking/system-design.md), [architecture](../../../08-embedded-systems/architecture.md)
**Thiết kế kiến trúc phần mềm cho một máy quét mã vạch (whole product).**
<details><summary>Đáp án (khung)</summary>

- **Requirements**: nguồn ảnh (camera/CCD), loại mã (1D/2D), **latency** trigger→result (vd <100ms), output (USB-HID/serial/mạng), ánh sáng, offline?
- **Constraints**: CPU có NEON/DSP? RAM frame buffer? pin hay cắm? độ phân giải×fps → băng thông.
- **High-level**: Trigger/Illumination → Image capture (V4L2, **DMA**) → Preprocess (binarize, khử nhiễu) → Decode (định vị + symbology) → Output formatter (HID/serial) → Config/Management.
- **Deep dive**: pipeline đồng thời — capture (ISR/DMA) → **ring buffer** frame → **decode thread** nặng CPU/NEON; **zero-copy** buffer DMA; chia **budget latency** từng tầng.
- **Trade-offs**: decode 1 frame (nhanh, dễ trượt) vs gộp nhiều frame (chính xác, tăng latency); on-device vs offload; polling vs interrupt+DMA (chọn DMA).
- **Failure**: decode fail → frame kế, hết timeout → beep lỗi; overrun → drop frame cũ nhất; ánh sáng kém → auto-exposure + illumination.
</details>

#### SD-014 · 🔴 · design · 🏗️ · [→ system-design §5](../../../10-thinking/system-design.md), [io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Thiết kế module nhận gói dữ liệu qua UART/USB (framing + toàn vẹn + tái hợp).**
<details><summary>Đáp án (khung)</summary>

- **Requirements**: tốc độ (baud/throughput), kích thước gói (cố định/biến), độ trễ, mất gói chấp nhận được không, song công?
- **Constraints**: RAM buffer, CPU (parse trong ISR hay thread?), có DMA UART không.
- **High-level**: RX (interrupt/DMA) → **ring buffer byte** → **Framer** (state machine tách gói) → **Validator** (CRC/checksum) → Dispatcher (theo type) → ACK/retry nếu có.
- **Deep dive**: **framing** — độ dài (header có length) hoặc **delimiter** + byte-stuffing (escape khi delimiter xuất hiện trong data); **state machine** reassembly (WAIT_SYNC → LEN → PAYLOAD → CRC); không parse nặng trong ISR (chỉ đẩy byte vào ring buffer, thread lo phần còn lại).
- **Trade-offs**: fixed-length (đơn giản, phí băng thông) vs length-prefixed (gọn, phải chống length giả) vs delimiter (stream-friendly, cần escaping); CRC (mạnh) vs checksum (rẻ).
- **Failure**: byte lỗi/mất sync → CRC fail → **loại gói, resync** tới delimiter kế; buffer đầy → drop + đếm lỗi; gói dở khi timeout → hủy, về WAIT_SYNC.
</details>

#### SD-015 · 🔴 · design · 🏗️ · [→ system-design §4](../../../10-thinking/system-design.md), [driver-basics](../../../05-drivers-device-tree/driver-basics.md), [kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Thiết kế một subsystem driver end-to-end: từ API userspace tới chạm phần cứng.**
<details><summary>Đáp án (khung)</summary>

- **Requirements**: thao tác userspace cần (đọc/ghi/cấu hình/stream?), đồng bộ hay bất đồng bộ, nhiều tiến trình cùng mở?, tần suất/độ trễ.
- **Constraints**: bus (I2C/SPI/MMIO?), cần DMA?, interrupt hay polling, ràng buộc realtime.
- **High-level**: *Userspace lib/API* (che ioctl sau hàm C++ sạch) → *Char device* (`/dev/x`, `file_operations`) → *Kernel driver core* (probe qua device tree, quản state) → *HW access* (MMIO/regmap, IRQ handler top/bottom-half) → phần cứng.
- **Deep dive**: chọn kênh userspace↔kernel — **ioctl** (lệnh cấu hình), **read/write** (luồng dữ liệu), **sysfs** (thuộc tính đơn giản), **mmap** (zero-copy dữ liệu lớn); `copy_to/from_user` ở biên; đồng bộ trong kernel (mutex vs spinlock nếu đụng IRQ); top-half nhanh + bottom-half (workqueue/threaded IRQ) cho việc nặng.
- **Trade-offs**: ioctl (linh hoạt, kém khám phá) vs sysfs (dễ script, chỉ hợp giá trị đơn); polling (đơn giản, tốn CPU) vs interrupt+DMA; mmap (nhanh, phức tạp) vs read/write.
- **Failure**: userspace truyền con trỏ/kích thước xấu → validate + `copy_*` an toàn; thiết bị treo → timeout + reset; probe lỗi → `EPROBE_DEFER`/nhả tài nguyên sạch; tránh sleak khi rmmod.
</details>

#### SD-016 · 🟠 · design · 🏗️ · [→ system-design §5](../../../10-thinking/system-design.md), [creational §Object Pool](../../../11-design-patterns/creational.md), [constraints](../../../08-embedded-systems/constraints.md)
**Thiết kế quản lý bộ nhớ tất định cho thiết bị chạy lâu dài (không cấp phát động sau init).**
<details><summary>Đáp án (khung)</summary>

- **Vấn đề**: `malloc`/`new` ở runtime gây **fragmentation** (thất bại dù còn RAM) + thời gian **bất định** → nguy hiểm cho hệ chạy năm trời/realtime.
- **Chiến lược**: cấp phát mọi thứ **lúc init** rồi khóa; sau đó chỉ dùng **pool/arena** cố định. Các khối: **object pool** (slot cố định, mượn/trả O(1)), **fixed-block allocator** theo size-class, **ring/arena allocator** cho dữ liệu tạm (reset theo khung), **stack allocator** cho vòng đời LIFO.
- **Deep dive**: tính **budget RAM** trước (mỗi module bao nhiêu, worst-case đồng thời); pool trả `nullptr` khi cạn (tất định) thay vì `bad_alloc`; placement new + destructor tường minh; cân nhắc `-fno-exceptions`/no-RTTI nếu môi trường yêu cầu.
- **Trade-offs**: pool (nhanh, tất định, nhưng phí RAM giữ chỗ + phải ước lượng đúng) vs heap (linh hoạt, rủi ro); nhiều size-class (ít phí) vs một cỡ (đơn giản).
- **Failure**: pool cạn → chính sách rõ (drop/chờ/degrade), không tràn âm thầm; phát hiện leak-logic (mượn không trả) bằng đếm slot; guard chống double-release.
</details>

---
⬅️ [Bank index](README.md)
