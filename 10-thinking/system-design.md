# System Design — Thiết kế hệ thống (hướng Embedded Linux)

> **TL;DR**
> - System design phỏng vấn đo: khả năng **làm rõ yêu cầu mơ hồ**, **chia kiến trúc thành phần**, **đánh đổi có lý do**, và **giao tiếp** — không phải vẽ sơ đồ đẹp.
> - Quy trình: **Requirement (chức năng + phi chức năng) → Ước lượng/ràng buộc → Kiến trúc tổng (component + data flow) → Đi sâu thành phần chính → Đánh đổi & rủi ro → Mở rộng/lỗi**.
> - Embedded khác web scale: ràng buộc là **RAM/flash/CPU/điện/realtime/độ tin cậy**, không phải "triệu request/giây". Tối ưu cho tài nguyên hữu hạn và chạy lâu dài không người can thiệp.
> - Nguyên tắc thiết kế tốt: **tách concern (module hóa)**, interface rõ ràng, **single responsibility**, xử lý lỗi & trạng thái xác định, tránh chia sẻ trạng thái mutable.
> - Không có kiến trúc "đúng" — chỉ có kiến trúc phù hợp ràng buộc. Luôn nêu giả định.

---

## 1. System design phỏng vấn thực sự đo gì?

Không phải bạn nhớ kiến trúc mẫu, mà là: bạn có **hỏi đúng câu** trước khi thiết kế không? có chia hệ thống thành phần hợp lý không? có **nhận ra và cân nhắc đánh đổi** không? có lường lỗi/mở rộng không? và có **trình bày mạch lạc** không? Đây là kỹ năng của người dẫn dắt thiết kế — thứ phân biệt Senior.

---

## 2. Quy trình thiết kế

```
① REQUIREMENTS   → chức năng (làm gì) + phi chức năng (nhanh/ít RAM/tin cậy?)
② CONSTRAINTS    → ước lượng & ràng buộc: tài nguyên, tải, môi trường
③ HIGH-LEVEL     → component chính + luồng dữ liệu giữa chúng (sơ đồ khối)
④ DEEP DIVE      → đi sâu 1–2 thành phần cốt lõi / chỗ khó nhất
⑤ TRADE-OFFS     → vì sao chọn vậy, các phương án khác, rủi ro
⑥ FAILURE/SCALE  → lỗi xảy ra thì sao? mở rộng/cập nhật thế nào?
```

### ① Requirements — làm rõ trước khi vẽ
- **Functional**: hệ thống *phải làm gì*? input/output? use case chính?
- **Non-functional**: yêu cầu về hiệu năng, độ trễ, bộ nhớ, điện năng, độ tin cậy, bảo mật, thời gian boot? — **đây là phần định hình kiến trúc embedded**.
- Phạm vi: cái gì trong, cái gì ngoài scope (đừng thiết kế thừa).

### ② Constraints — ước lượng & ràng buộc
Embedded: bao nhiêu RAM/flash? CPU loại gì (có FPU? đa nhân?)? nguồn pin hay cắm điện? tần suất dữ liệu (sensor 1kHz?)? deadline realtime? môi trường (nhiệt độ, mất điện)? Những con số này **quyết định** lựa chọn (heap hay tĩnh, RTOS hay Linux, polling hay interrupt+DMA).

### ③ High-level — kiến trúc tổng
Vẽ các **component** và **luồng dữ liệu** giữa chúng. Mỗi component một trách nhiệm rõ. Xác định interface giữa chúng (đây là lúc dùng tư duy [API design](../07-shared-libraries/api-design.md)).

### ④ Deep dive — đào sâu chỗ cốt lõi
Chọn 1–2 thành phần khó/quan trọng nhất và thiết kế chi tiết: cấu trúc dữ liệu, thuật toán, đồng bộ, xử lý lỗi. Đừng dàn đều — tập trung chỗ rủi ro.

### ⑤ Trade-offs — nêu rõ lựa chọn
Với mỗi quyết định lớn: phương án nào khác? vì sao chọn cái này? đánh đổi gì? (xem [problem-solving.md](problem-solving.md)).

### ⑥ Failure & evolution — lường trước
Thành phần X chết thì sao? mất điện giữa chừng? dữ liệu hỏng? cập nhật firmware thế nào (A/B, rollback)? hệ thống lớn hơn 10x thì kiến trúc còn hợp không?

---

## 3. Embedded design khác web/server design

| | Web/Server scale | Embedded |
|--|------------------|----------|
| Ràng buộc chính | Throughput, latency, scale ngang | RAM/flash/CPU/điện/realtime |
| "Nhiều" nghĩa là | Triệu user/request | KB bộ nhớ, mW điện |
| Mở rộng | Thêm máy (horizontal) | Thường **không** thêm được — phải vừa phần cứng cố định |
| Lỗi | Retry, redundancy, restart service | Watchdog, fail-safe, không người can thiệp |
| Triển khai | Deploy liên tục | Firmware update khó/hiếm, phải an toàn (rollback) |
| Tối ưu cho | Chi phí/scale | Tài nguyên hữu hạn, độ tin cậy, tuổi thọ pin |

→ Đừng bê tư duy "thêm server, dùng cache phân tán" vào bài embedded. Ở đây "tối ưu" nghĩa là vừa RAM, ngủ tiết kiệm điện, tất định về thời gian, chạy năm trời không reboot.

---

## 4. Nguyên tắc thiết kế tốt (áp dụng được mọi nơi)

- **Separation of concerns / modularity**: chia hệ thành module độc lập, mỗi module một trách nhiệm rõ → dễ hiểu, test, thay thế.
- **Single Responsibility**: mỗi thành phần làm tốt *một* việc.
- **Interface rõ ràng, coupling thấp**: module giao tiếp qua interface ổn định, giấu chi tiết bên trong → đổi nội bộ không lan ra (information hiding).
- **Tránh chia sẻ trạng thái mutable**: giảm bug đồng bộ; ưu tiên message passing/ownership rõ.
- **Trạng thái xác định & xử lý lỗi**: máy trạng thái rõ ràng, mọi lỗi có đường xử lý (fail to safe state), không "lỡ" trạng thái.
- **Design for testability**: tách logic khỏi I/O phần cứng (vd hardware abstraction layer) → test logic trên host.
- **YAGNI / không over-engineer**: thiết kế cho yêu cầu hiện tại + chừa chỗ mở rộng hợp lý, không vẽ vời tính năng tưởng tượng.

---

## 5. Hộp đồ nghề thiết kế embedded (khối tái dùng)

Hầu hết bài design embedded ráp lại từ **một số khối lặp đi lặp lại**. Nhớ sẵn "hộp đồ nghề" này để không thiết kế lại từ số 0 mỗi lần — gặp bài chỉ việc chọn khối phù hợp và giải thích *vì sao*.

| Vấn đề gặp phải | Khối giải quyết (chọn theo ràng buộc) |
|---|---|
| **Nối producer nhanh ↔ consumer chậm** (ISR/DMA → xử lý) | **Ring buffer** (SPSC lock-free nếu 1–1), double-buffering; có *backpressure*/drop-policy khi đầy |
| **Mô hình đồng thời** | 1 **event loop** (đơn giản, tất định) · **thread theo vai trò** · **RTOS task** + priority. Ưu tiên *message passing* hơn shared mutable ([sync-primitives](../03-operating-system/sync-primitives.md)) |
| **Lưu trữ trên flash** | Chống mòn (**wear leveling**), ghi **atomic/journaling** chống mất điện, gom **batch** giảm số lần ghi, log-structured |
| **Giao tiếp/nhận gói** | **Framing** (độ dài hoặc delimiter) + **CRC/checksum** + sequence/ack + **state machine** reassembly ([networking](../13-networking/)) |
| **Cập nhật firmware** | **A/B partition** + rollback + **chữ ký/hash** verify + xác nhận health sau boot ([boot-process](../08-embedded-systems/boot-process.md)) |
| **Độ tin cậy / không người trực** | **Watchdog**, **fail to safe state**, timeout ở *mọi* chỗ chờ, xử lý brownout/mất điện |
| **Bộ nhớ tất định** | Cấp phát **tĩnh/pool** ([object pool](../11-design-patterns/creational.md)), tính *budget* RAM trước, **không cấp phát động sau init** |
| **Test được** | **HAL** tách logic khỏi phần cứng, inject phụ thuộc, build trên host chạy ASan/TSan |

**Template rút gọn (đọc lướt trước khi vào phòng):**
```
① Hỏi:   chức năng? + phi chức năng (RAM/flash/điện/latency/realtime/tin cậy?) + scope
② Số:    RAM? flash? CPU/FPU? nguồn? tần suất dữ liệu? deadline? → chốt ràng buộc
③ Vẽ:    3–6 component + luồng dữ liệu; mỗi khối 1 trách nhiệm; interface giữa chúng
④ Đào:   1–2 chỗ khó nhất (đồng bộ? realtime? mất điện?) — thiết kế chi tiết
⑤ Đổi:   mỗi quyết định lớn: phương án khác? vì sao chọn? hy sinh gì?
⑥ Hỏng:  component chết? mất điện? mất mạng? dữ liệu hỏng? update? → fail-safe
```

---

## 6. Ví dụ giải mẫu

> Ba bài dưới đây là **mẫu tư duy** — đọc để thấy cách ráp "hộp đồ nghề" §5 theo quy trình §2. Thêm nhiều **prompt để tự-test** (đáp án ẩn) ở [bank SD](../14-prep/mock-interview/bank/system-design.md); luyện bằng `/mock type design track system-design`.

### 6.1 "Thiết kế phần mềm thu thập dữ liệu sensor trên thiết bị nhúng Linux"

Phác cách tiếp cận (không phải đáp án duy nhất):
- **Requirements**: đọc N sensor qua I2C/SPI ở tần số bao nhiêu? lưu local? gửi server? mất dữ liệu chấp nhận được không? realtime cứng không?
- **Constraints**: RAM/flash, có mạng không, pin hay cắm, tần số lấy mẫu → quyết định buffer, lưu trữ.
- **High-level**: *Sensor driver/HAL* → *Acquisition (đọc theo lịch, interrupt/DMA)* → *Ring buffer* → *Processing* → *Storage (vòng, chống mòn flash)* / *Uploader (mạng, retry)*. init/systemd quản service; watchdog giám sát.
- **Deep dive**: cơ chế đọc (interrupt+DMA thay vì polling để tiết kiệm CPU/điện), ring buffer giữa producer (ISR/thread đọc) và consumer (xử lý) — đồng bộ thế nào ([sync-primitives](../03-operating-system/sync-primitives.md)).
- **Trade-offs**: poll vs interrupt+DMA; lưu thẳng flash (mòn) vs gom batch; gửi realtime (tốn điện/mạng) vs gom rồi gửi.
- **Failure**: mất điện giữa lúc ghi (atomic write/journaling), mất mạng (buffer + retry), sensor lỗi (timeout, đánh dấu, không treo hệ), treo (watchdog reset).

> Cách trình bày này quan trọng hơn "đáp án": cho thấy bạn làm rõ → chia phần → đào sâu chỗ khó → nêu đánh đổi → lường lỗi.

### 6.2 "Thiết kế hệ thống cập nhật firmware (OTA) an toàn"

Bài "whole system" rất hay gặp cho Embedded Linux (Datalogic JD ghi rõ "design entire systems"). Chìa khoá: **không được brick thiết bị dù mất điện giữa chừng**.

- **Requirements**: nguồn ảnh update (mạng OTA / USB / thẻ nhớ)? kích thước ảnh? cho phép downtime bao lâu? cần ký số (chống ảnh giả)? tần suất cập nhật? có phải cập nhật cả bootloader/kernel không?
- **Constraints**: flash có đủ **2 slot** không? RAM đủ để verify hash/chữ ký? nguồn có ổn định (nguy cơ mất điện khi ghi)? bootloader có kiểm soát được việc chọn slot không?
- **High-level**: *Downloader* (kéo ảnh, resume được) → *Verifier* (kiểm **hash + chữ ký** trước khi tin) → *Writer* (ghi vào **slot B** trong khi **slot A** đang chạy) → *Bootloader* (cờ "thử slot B") → *Health-check sau boot* (OK → **commit**; lỗi → **rollback** về A).
- **Deep dive**: **A/B partition** + cờ trong bootloader (`boot_next`, `try_count`); chuyển slot là thao tác **atomic** (đổi một cờ, không ghi đè đang chạy); chỉ mark slot bootable **sau khi** verify chữ ký; **anti-rollback** bằng version counter (chặn hạ cấp về bản có lỗ hổng). Xem [boot-process](../08-embedded-systems/boot-process.md), [melp/bootloader-kernel](../15-book-summaries/melp/bootloader-kernel.md).
- **Trade-offs**: **A/B** (tốn gấp đôi flash, nhưng an toàn tuyệt đối) vs **in-place + recovery partition** (tiết kiệm flash, rủi ro hơn); **ký số** (an toàn, cần quản lý khóa) vs chỉ **CRC** (chống hỏng nhưng không chống giả mạo).
- **Failure**: mất điện khi ghi slot B → B hỏng nhưng **A vẫn boot** (chưa chuyển cờ); ảnh mới boot lỗi → watchdog + `try_count` hết → bootloader **tự rollback** về A; ảnh giả mạo → **chặn ở Verifier**.

### 6.3 "Thiết kế kiến trúc phần mềm cho một máy quét mã vạch" (whole product)

Đúng chất Datalogic (máy quét công nghiệp). Đây là bài "cả sản phẩm" — thể hiện khả năng chia hệ lớn thành pipeline.

- **Requirements**: nguồn ảnh (camera/CCD)? loại mã (1D/2D)? **latency mục tiêu** từ lúc bấm trigger tới khi ra kết quả (vd < 100ms)? output đi đâu (USB-HID như bàn phím / serial / mạng)? điều kiện ánh sáng? hoạt động offline?
- **Constraints**: CPU có **NEON/DSP** để tăng tốc decode? RAM cho frame buffer? cầm tay chạy pin (điện) hay cắm? độ phân giải × fps → băng thông dữ liệu.
- **High-level**: *Trigger/Illumination* → *Image capture* (V4L2/camera driver, **DMA**) → *Preprocess* (binarize, khử nhiễu) → *Decode* (định vị + giải mã theo symbology) → *Output formatter* (HID/serial) → *Config/Management* (bật/tắt symbology, cập nhật firmware).
- **Deep dive**: **pipeline đồng thời** — capture (ISR/DMA) đẩy frame vào **ring buffer**, một **decode thread** nặng CPU/NEON lấy ra xử lý → dùng khối "ring buffer producer/consumer" §5; **zero-copy** frame (chia sẻ buffer DMA, không copy) để giữ latency; phân bổ **budget latency** trigger→result cho từng tầng.
- **Trade-offs**: decode **1 frame** (nhanh, có thể trượt) vs **gộp nhiều frame** (chính xác hơn, tăng latency); xử lý **on-device** vs offload; **polling** camera vs **interrupt+DMA** (tiết kiệm CPU/điện — chọn cái sau).
- **Failure**: decode fail → thử frame kế, hết timeout → **beep báo lỗi**; buffer overrun → **drop frame cũ nhất** (giữ realtime); ánh sáng kém → auto-exposure + bật illumination. Liên hệ [architecture](../08-embedded-systems/architecture.md) (MMIO/DMA), [drivers](../05-drivers-device-tree/).

> Chung cho cả ba: **cùng một quy trình 6 bước + cùng hộp đồ nghề** §5 — chỉ đổi bài toán. Người phỏng vấn đánh giá *cách bạn đi*, không phải bạn thuộc kiến trúc mẫu.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Khi được giao một bài system design mơ hồ, bước đầu tiên bạn làm gì?</summary>

Làm rõ yêu cầu trước khi thiết kế bất cứ gì. Tôi hỏi về **yêu cầu chức năng** (hệ thống phải làm gì, input/output, use case chính) và đặc biệt **yêu cầu phi chức năng** (hiệu năng, độ trễ, giới hạn RAM/flash, điện năng, độ tin cậy, realtime, thời gian boot) — vì với embedded chính các ràng buộc này định hình kiến trúc. Tôi cũng xác định phạm vi (cái gì trong/ngoài scope) để không thiết kế thừa, và nêu rõ các giả định. Chỉ sau khi hiểu đúng bài toán và ràng buộc tôi mới phác kiến trúc — vì giải đúng một bài sai là vô nghĩa, và một giải pháp tốt cho ràng buộc này có thể tệ cho ràng buộc khác.
</details>

<details><summary>2) System design cho embedded khác gì cho web/server scale?</summary>

Ràng buộc và mục tiêu tối ưu khác hẳn. Web/server tối ưu cho throughput, latency và khả năng scale ngang (thêm máy, cache phân tán, load balancer) trước hàng triệu request; lỗi xử lý bằng retry/redundancy/restart, và deploy liên tục. Embedded bị giới hạn bởi RAM/flash (KB–MB), CPU yếu, điện năng (pin), yêu cầu realtime và độ tin cậy cao; "nhiều" nghĩa là vài KB bộ nhớ hay vài mW điện chứ không phải triệu user. Quan trọng là **thường không scale ngang được** — phải vừa phần cứng cố định; lỗi xử lý bằng watchdog và fail-safe vì không người can thiệp; và cập nhật firmware khó nên phải an toàn (A/B, rollback). Do đó không nên bê tư duy "thêm server/cache phân tán" vào bài embedded; ở đây tối ưu nghĩa là vừa tài nguyên, tiết kiệm điện, tất định thời gian và chạy bền bỉ lâu dài.
</details>

<details><summary>3) Những nguyên tắc thiết kế nào giúp một hệ thống dễ bảo trì và mở rộng?</summary>

Tách concern và module hóa: chia hệ thành các module độc lập, mỗi module một trách nhiệm rõ (single responsibility), giao tiếp qua interface ổn định và giấu chi tiết nội bộ (information hiding) để coupling thấp — nhờ đó thay đổi bên trong một module không lan ra toàn hệ. Tránh chia sẻ trạng thái mutable để giảm bug đồng bộ. Thiết kế trạng thái xác định với xử lý lỗi đầy đủ (mọi lỗi có đường dẫn về safe state). Tách logic khỏi I/O phần cứng (hardware abstraction layer) để test được logic trên host. Và tuân theo YAGNI — thiết kế cho yêu cầu hiện tại cộng chỗ mở rộng hợp lý, không over-engineer cho tính năng tưởng tượng. Những nguyên tắc này làm hệ thống dễ hiểu, dễ test, dễ thay thế từng phần.
</details>

<details><summary>4) Trong thiết kế, bạn xử lý lỗi và trường hợp hệ thống chạy không người giám sát thế nào?</summary>

Tôi thiết kế để hệ thống luôn về được trạng thái an toàn (fail-safe) khi có lỗi, thay vì treo hay sập âm thầm. Cụ thể với embedded: dùng watchdog timer để tự reset nếu hệ treo (và kick watchdog ở vị trí phản ánh hệ thực sự hoạt động); kiểm tra mọi giá trị trả về và có đường xử lý cho từng lỗi (sensor timeout thì đánh dấu lỗi và tiếp tục, không để một thiết bị lỗi làm treo cả hệ); bảo vệ dữ liệu quan trọng trước mất điện bằng ghi atomic/journaling; xử lý mất kết nối bằng buffer cục bộ và retry; và cập nhật firmware an toàn bằng A/B partition với rollback để không "brick" thiết bị. Vì thiết bị thường không có người can thiệp, khả năng tự phục hồi và chịu lỗi là yêu cầu cốt lõi chứ không phải tính năng thêm.
</details>

<details><summary>5) Làm sao thiết kế để hệ thống dễ test, đặc biệt khi phụ thuộc phần cứng?</summary>

Chìa khóa là tách logic nghiệp vụ khỏi truy cập phần cứng bằng một lớp trừu tượng (Hardware Abstraction Layer/interface): code logic gọi qua interface thay vì đụng trực tiếp thanh ghi/driver, nhờ đó khi test có thể thay phần cứng thật bằng một implementation giả lập (mock/stub) và chạy logic trên host (PC) — nhanh, lặp lại được, không cần board thật. Thiết kế module với coupling thấp và interface rõ ràng cũng cho phép test từng phần độc lập. Với phần phụ thuộc thời gian/IO, tiêm (inject) các phụ thuộc đó để kiểm soát trong test. Ngoài ra build cùng code trên host để chạy các công cụ như AddressSanitizer/ThreadSanitizer (thường không chạy được trên target hạn chế). Tư duy "design for testability" ngay từ kiến trúc giúp phát hiện bug sớm và rẻ hơn nhiều so với chỉ test trên thiết bị.
</details>

---
⬅️ [problem-solving.md](problem-solving.md) · ➡️ Tiếp theo: [bank/system-design.md](../14-prep/mock-interview/bank/system-design.md)
