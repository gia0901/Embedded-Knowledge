# Câu hỏi phỏng vấn — Tư duy & System Design

> Topic gốc: [10 Thinking](../10-thinking/), [07 Shared Libraries](../07-shared-libraries/). Phần này nhiều câu **mở** — đáp án là *cách tiếp cận*, không phải con số duy nhất. Tự trình bày trước, rồi đối chiếu.

---

## 🟡 Tư duy (Trung bình)

<details><summary>1) Khi gặp vấn đề kỹ thuật chưa từng thấy, bạn tiếp cận thế nào?</summary>

Quy trình: làm rõ vấn đề thực sự + ràng buộc + tiêu chí thành công (tránh giải sai bài) → chia nhỏ thành phần giải được → phác vài hướng, so sánh đánh đổi, chọn (bắt đầu từ giải pháp đơn giản nhất chạy được rồi tối ưu) → triển khai từng phần, kiểm chứng liên tục → đánh giá đúng/đủ tốt cho ràng buộc chưa. Xuyên suốt suy luận từ first principles và nêu rõ đánh đổi. → [problem-solving](../10-thinking/problem-solving.md)
</details>

<details><summary>2) Vì sao "không có giải pháp tốt nhất, chỉ có đánh đổi"?</summary>

Mọi quyết định kỹ thuật hy sinh mặt này để được mặt kia; "tốt nhất" chỉ có nghĩa trong một ngữ cảnh/ràng buộc — giải pháp tối ưu trên server có thể tệ trên MCU 64KB RAM. Các trục đánh đổi: thời gian↔bộ nhớ, hiệu năng↔đơn giản, tốc độ phát triển↔tối ưu, linh hoạt↔đơn giản, throughput↔latency, tổng quát↔chuyên biệt. Luôn hỏi: tối ưu cho cái gì, trong ràng buộc nào? → [problem-solving](../10-thinking/problem-solving.md)
</details>

<details><summary>3) Suy luận từ first principles nghĩa là gì? Ví dụ.</summary>

Quay về nguyên lý cơ bản nhất và suy luận lên thay vì sao chép khuôn mẫu ("cargo cult"). Hỏi "tại sao" liên tục tới khi chạm nguyên lý không chia nhỏ hơn. Ví dụ: thay vì mặc định "cần mutex", hỏi tại sao → vì có dữ liệu chia sẻ bị ghi đồng thời → nếu thiết kế mỗi thread giữ bản riêng (không chia sẻ mutable) thì loại bỏ được khóa, vừa đúng vừa nhanh hơn. → [problem-solving](../10-thinking/problem-solving.md)
</details>

---

## 🟠 System Design (Khó)

<details><summary>4) Khi được giao bài system design mơ hồ, bước đầu tiên?</summary>

Làm rõ yêu cầu trước khi vẽ: yêu cầu chức năng (làm gì, input/output, use case) và đặc biệt **phi chức năng** (hiệu năng, RAM/flash, điện, độ tin cậy, realtime, boot time — với embedded chính ràng buộc này định hình kiến trúc), phạm vi (trong/ngoài scope), nêu giả định. Giải đúng một bài sai là vô nghĩa. → [system-design](../10-thinking/system-design.md)
</details>

<details><summary>5) System design cho embedded khác web/server scale thế nào?</summary>

Ràng buộc khác: web tối ưu throughput/latency/scale ngang (thêm máy, cache phân tán) cho triệu request, lỗi xử lý bằng retry/redundancy, deploy liên tục. Embedded giới hạn RAM/flash/CPU/điện/realtime, "nhiều" là vài KB/mW, **thường không scale ngang** (vừa phần cứng cố định), lỗi xử lý bằng watchdog/fail-safe (không người can thiệp), cập nhật firmware khó nên phải an toàn (A/B+rollback). Đừng bê tư duy "thêm server" vào embedded. → [system-design](../10-thinking/system-design.md)
</details>

<details><summary>6) Nguyên tắc thiết kế nào giúp hệ thống dễ bảo trì/mở rộng?</summary>

Tách concern/module hóa (mỗi module một trách nhiệm rõ — single responsibility), interface ổn định + coupling thấp + information hiding (đổi nội bộ không lan ra), tránh chia sẻ trạng thái mutable, trạng thái xác định + xử lý lỗi đầy đủ (fail to safe state), tách logic khỏi I/O phần cứng (HAL) để test trên host, YAGNI (không over-engineer). → [system-design](../10-thinking/system-design.md)
</details>

<details><summary>7) Thiết kế dễ test khi phụ thuộc phần cứng?</summary>

Tách logic khỏi truy cập phần cứng bằng Hardware Abstraction Layer/interface: logic gọi qua interface thay vì đụng thanh ghi/driver trực tiếp, nên test có thể thay phần cứng bằng mock/stub và chạy logic trên host (nhanh, lặp lại được). Module coupling thấp + inject phụ thuộc (thời gian/IO) để kiểm soát trong test. Build cùng code trên host để chạy ASan/TSan (thường không chạy được trên target). → [system-design](../10-thinking/system-design.md)
</details>

---

## 🔴 Bài thiết kế tổng hợp (Senior)

<details><summary>8) Thiết kế phần mềm thu thập dữ liệu sensor trên thiết bị nhúng Linux.</summary>

Cách tiếp cận (không phải đáp án duy nhất):
- **Requirements**: số sensor, bus (I2C/SPI), tần số lấy mẫu, lưu local hay gửi server, mất dữ liệu chấp nhận được không, realtime cứng không.
- **Constraints**: RAM/flash, có mạng/pin không, tần số → quyết định buffer & lưu trữ.
- **High-level**: Sensor HAL → Acquisition (interrupt+DMA theo lịch) → Ring buffer → Processing → Storage (vòng, chống mòn flash) / Uploader (mạng, retry); systemd quản service, watchdog giám sát.
- **Deep dive**: interrupt+DMA thay polling (tiết kiệm CPU/điện); ring buffer producer–consumer giữa thread đọc & xử lý, đồng bộ bằng mutex+cv hoặc lock-free.
- **Trade-offs**: poll vs interrupt+DMA; ghi thẳng flash (mòn) vs gom batch; gửi realtime (tốn điện/mạng) vs gom rồi gửi.
- **Failure**: mất điện khi ghi (atomic/journaling), mất mạng (buffer+retry), sensor lỗi (timeout, không treo hệ), treo (watchdog).

Cách trình bày (làm rõ → chia phần → đào sâu chỗ khó → đánh đổi → lường lỗi) quan trọng hơn "đáp án". → [system-design](../10-thinking/system-design.md)
</details>

<details><summary>9) Thiết kế một C++ shared library/API cho lớp trên dùng. Bạn quan tâm gì?</summary>

- **API tốt**: tối thiểu (mọi thứ public là cam kết maintain), che giấu chi tiết, nhất quán, dễ dùng đúng/khó dùng sai (kiểu mạnh, RAII, `[[nodiscard]]`), tài liệu hóa hợp đồng (tiền/hậu điều kiện, ownership, thread-safety).
- **Ownership rõ ràng** qua kiểu: trả `unique_ptr` (sở hữu), nhận `const&`/`span` (mượn) — người đọc chữ ký hiểu ngay ai sở hữu.
- **Error handling nhất quán**: exception (C++ thuần) hoặc mã lỗi/`expected` (biên giới C/embedded).
- **Ổn định ABI**: pimpl để giấu data member (thêm field không phá ABI); biên giới C (`extern "C"`, POD, opaque handle) nếu cần đa compiler/ngôn ngữ; versioning kỷ luật (soname theo major, không đổi chữ ký/layout đã phát hành — chỉ thêm).
- Cẩn thận virtual function trong interface public (thêm virtual phá vtable). → [api-design](../07-shared-libraries/api-design.md), [abi-versioning](../07-shared-libraries/abi-versioning.md)
</details>

<details><summary>10) Tình huống: thư viện của bạn cập nhật phiên bản mới khiến app khách hàng (không build lại) crash. Nguyên nhân khả dĩ và cách tránh?</summary>

Nguyên nhân khả dĩ: **ABI break** dù API không đổi (app không build lại nên dùng binary cũ với `.so` mới). Vd: thêm data member vào struct/class public (đổi sizeof/offset), thêm/đổi thứ tự virtual function (đổi vtable), đổi chữ ký, đổi kích thước/alignment kiểu. Cách tránh: giữ ABI tương thích — dùng pimpl để giấu data member, chỉ thêm hàm mới thay vì sửa cái cũ, không đổi layout đã phát hành, thận trọng virtual; nếu buộc phá ABI thì tăng major + đổi soname (`libfoo.so.2`) để hai bản cùng tồn tại; dùng symbol versioning; kiểm tra bằng `abidiff`/abi-compliance-checker trước khi phát hành. → [abi-versioning](../07-shared-libraries/abi-versioning.md)
</details>

<details><summary>11) Thiết kế cơ chế giao tiếp giữa một process realtime và một process xử lý/giao diện trên cùng SoC.</summary>

Cách tiếp cận: tách vai trò (realtime đọc/điều khiển, process kia xử lý/UI/mạng). Giao tiếp cùng máy hiệu năng cao → **shared memory** (zero-copy) tổ chức ring buffer cho luồng dữ liệu lớn, đồng bộ bằng semaphore/mutex process-shared đặt trong vùng shm; dùng **eventfd** hoặc Unix domain socket để báo hiệu/điều khiển có ranh giới. Nếu là hệ heterogeneous (Cortex-A chạy Linux + Cortex-M chạy RTOS) thì dùng shared memory + mailbox/RPMsg (AMP). Đánh đổi: shared memory nhanh nhất nhưng tự đồng bộ (dễ sai); message queue/socket an toàn hơn nhưng có copy. Lường lỗi: bên realtime không được block chờ bên kia (dùng buffer + non-blocking), watchdog cho cả hai. → [ipc-linux](../04-linux-system-programming/ipc-linux.md), [rtos-vs-linux](../08-embedded-systems/rtos-vs-linux.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
