# SD — Tư duy & System Design

> Domain `SD`. Nhiều câu **mở** — chấm theo *cách tiếp cận*, không đáp án duy nhất. Track dùng: `system-design`, `bsp`, `cpp-system`.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Tư duy giải quyết vấn đề | 5 |
| **B** | Phương pháp system design | 4 |
| **C** | Bài thiết kế hệ thống | 7 |
| **D** | Thiết kế API & thư viện | 5 |
| **E** | ABI & versioning | 5 |
| **F** | Linking & loading | 7 |

---

## A — Tư duy giải quyết vấn đề

#### SD-033 · 🟢 · concept · ⭐ · 📦 2026-08-13 · [→ problem-solving](../../../10-thinking/problem-solving.md)
**Trong phỏng vấn, vì sao nên "think aloud"?**
<details><summary>Đáp án</summary>

**Vì thứ được chấm là QUÁ TRÌNH SUY NGHĨ, không phải đáp án.** Interviewer đã biết đáp án; cái họ không biết là **bạn suy nghĩ thế nào** — và đó mới là thứ dự đoán được bạn làm việc ra sao.

**Bốn lợi ích cụ thể:**
1. **Ngồi im 5 phút = 0 thông tin.** Dù bạn đang nghĩ rất hay, interviewer chỉ thấy im lặng và ghi *"không tiếp cận được bài"*.
2. **Được điểm cho hướng đi đúng** kể cả khi chưa ra đáp án cuối.
3. **Được sửa hướng sớm.** Nói ra giả định sai ⇒ interviewer thường gợi ý ngay, thay vì để bạn đi lạc 20 phút.
4. **Cho thấy bạn nêu được đánh đổi** — thứ phân biệt mid với senior mạnh hơn cả việc giải đúng.

**Nói gì cho có ích (không phải kể lể):**
- *"Tôi đang giả định X — có đúng không?"* ⇒ làm rõ đề bài.
- *"Có hai hướng: A đơn giản nhưng O(n²), B phức tạp hơn nhưng O(n log n). Với n nhỏ tôi chọn A."* ⇒ **nêu đánh đổi**.
- *"Tôi chưa chắc chỗ này, để tôi thử một ví dụ nhỏ."* ⇒ trung thực + có phương pháp.
- *"Ca biên cần xử lý: rỗng, một phần tử, tràn số."* ⇒ tư duy kỹ lưỡng.

⚠️ **Không phải nói liên tục.** Xin **một phút im lặng để nghĩ** là hoàn toàn ổn — miễn là **nói ra rằng bạn đang làm vậy**: *"cho tôi một phút sắp xếp ý nhé"*.

⚠️ **Cạm bẫy ngược:** nói nhiều mà không có cấu trúc cũng trừ điểm. Khung an toàn: **làm rõ đề → nêu hướng và đánh đổi → chọn một → cài đặt → kiểm ca biên**.

**Chốt:** *"Interviewer chấm quá trình chứ không chấm đáp án — im lặng là không có thông tin. Nói giả định, nói đánh đổi, và xin một phút để nghĩ khi cần."*
</details>

---
⬅️ [Bank index](README.md)

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

#### SD-032 · 🟡 · concept · 📦 2026-08-13 · [→ problem-solving](../../../10-thinking/problem-solving.md)
**Khi bế tắc với một bài toán, bạn làm gì?**
<details><summary>Đáp án</summary>

**Bế tắc gần như luôn có nghĩa: một GIẢ ĐỊNH nào đó của bạn đang sai.** Vì nếu mọi giả định đều đúng thì đường đi đã hiện ra. Nên việc cần làm không phải "cố nghĩ mạnh hơn" mà là **đi tìm giả định sai**.

**Sáu việc, xếp theo thứ tự nên thử:**

| # | Việc | Vì sao hiệu quả |
|---|---|---|
| **1** | **Liệt kê những gì đã LOẠI TRỪ, và dựa trên căn cứ nào** | Bug hầu như luôn nấp trong một giả định **chưa từng được kiểm** ([DBG-008](debugging.md)) |
| **2** | **Giải thích cho người khác** (rubber duck) | Việc phải nói thành lời buộc bạn phát biểu rõ giả định — và bạn thường tự thấy chỗ sai giữa câu |
| **3** | **Thu nhỏ bài toán** | Bỏ bớt cho tới khi còn ca nhỏ nhất vẫn lỗi; thứ còn lại chính là nguyên nhân |
| **4** | **Đảo ngược câu hỏi** | Thay vì *"vì sao nó sai?"* hỏi *"vì sao nó lại từng ĐÚNG?"* — thường lộ ra cơ chế thật |
| **5** | **Đổi mức trừu tượng** | Đang soi từng dòng thì lùi ra nhìn kiến trúc; đang bàn kiến trúc thì soi vào dữ liệu thật |
| **6** | **Nghỉ / ngủ một đêm** | Không phải lười — nó phá vỡ vết mòn tư duy đang giữ bạn ở một hướng sai |

**Ba câu tự hỏi khi thật sự tắc:**
- *"Nếu bắt buộc phải sai ở đâu đó, tôi tin chắc chỗ nào nhất?"* → **đi kiểm đúng chỗ đó**.
- *"Tôi đang giải đúng bài toán không?"* — nhiều khi bế tắc vì bài toán bị phát biểu sai từ đầu.
- *"Ai đã gặp chuyện này rồi?"* — hỏi sớm không phải là yếu; **tốn hai ngày rồi mới hỏi** mới là lãng phí.

⚠️ **Chống chỉ định:** thử ngẫu nhiên nhiều thứ cùng lúc. Nó phá luôn khả năng suy luận vì bạn không còn biết cái gì gây ra cái gì ([DBG-008](debugging.md) — mỗi lần đổi một biến).

**Chốt:** *"Bế tắc nghĩa là có một giả định sai — nên việc cần làm là liệt kê những gì đã loại trừ và kiểm lại căn cứ, chứ không phải cố nghĩ mạnh hơn. Và đừng thử ngẫu nhiên nhiều thứ cùng lúc."*
</details>

---

## B — Phương pháp system design

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

---

## C — Bài thiết kế hệ thống

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

#### SD-011 · 🔴 · design · 🏗️ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md), [rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Thiết kế cơ chế giao tiếp giữa một process realtime và một process xử lý/giao diện trên cùng SoC.**
<details><summary>Đáp án</summary>

Tách vai trò (realtime đọc/điều khiển, process kia xử lý/UI/mạng). Giao tiếp cùng máy hiệu năng cao → **shared memory** (zero-copy) tổ chức ring buffer cho luồng dữ liệu lớn, đồng bộ bằng semaphore/mutex process-shared đặt trong vùng shm; dùng **eventfd** hoặc Unix domain socket để báo hiệu/điều khiển có ranh giới. Nếu hệ heterogeneous (Cortex-A Linux + Cortex-M RTOS) thì shared memory + mailbox/RPMsg (AMP). Đánh đổi: shared memory nhanh nhất nhưng tự đồng bộ (dễ sai); message queue/socket an toàn hơn nhưng có copy. Lường lỗi: bên realtime không được block chờ bên kia (buffer + non-blocking), watchdog cho cả hai.
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

#### SD-016 · 🟠 · design · 🏗️ · [→ system-design §5](../../../10-thinking/system-design.md), [creational §Object Pool](../../../11-design-patterns/creational.md), [constraints](../../../08-embedded-systems/constraints.md)
**Thiết kế quản lý bộ nhớ tất định cho thiết bị chạy lâu dài (không cấp phát động sau init).**
<details><summary>Đáp án (khung)</summary>

- **Vấn đề**: `malloc`/`new` ở runtime gây **fragmentation** (thất bại dù còn RAM) + thời gian **bất định** → nguy hiểm cho hệ chạy năm trời/realtime.
- **Chiến lược**: cấp phát mọi thứ **lúc init** rồi khóa; sau đó chỉ dùng **pool/arena** cố định. Các khối: **object pool** (slot cố định, mượn/trả O(1)), **fixed-block allocator** theo size-class, **ring/arena allocator** cho dữ liệu tạm (reset theo khung), **stack allocator** cho vòng đời LIFO.
- **Deep dive**: tính **budget RAM** trước (mỗi module bao nhiêu, worst-case đồng thời); pool trả `nullptr` khi cạn (tất định) thay vì `bad_alloc`; placement new + destructor tường minh; cân nhắc `-fno-exceptions`/no-RTTI nếu môi trường yêu cầu.

---

### ⭐ Vì sao "cẩn thận `delete` đầy đủ" KHÔNG chữa được phân mảnh

Đây là chỗ hay bị nói lướt. **Phân mảnh không phải là leak** — nó xảy ra ngay cả khi bạn giải phóng hoàn hảo:

```
Cấp phát xen kẽ rồi giải phóng hết các khối 1000:

 [100][1000][100][1000][100][1000][100]
   ↓ free hết khối 1000
 [100][    ][100][    ][100][    ][100]
        900        900        900         ← tổng trống 2700 byte

 Xin 1 khối 2000 byte  →  ❌ THẤT BẠI (không mảnh nào đủ 2000)
```

**Vì sao allocator không gom ba mảnh 900 lại?** Vì muốn gom thì phải **dời** các khối 100 đang dùng — mà C/C++ phơi bày **địa chỉ thật**: chương trình đang giữ con trỏ trỏ thẳng vào đó. Dời khối = mọi con trỏ ấy thành rác, và allocator **không có cách nào tìm ra** chúng để cập nhật.

> Ngôn ngữ có GC (Java, Go) **nén được** heap chính vì chúng không cho bạn giữ địa chỉ trần — chúng dùng handle mà runtime có thể sửa. C++ đánh đổi khả năng đó lấy hiệu năng và khả năng kiểm soát.

⇒ Phân mảnh là hệ quả của **mẫu cấp phát**, không phải của **kỷ luật giải phóng**. Sửa bằng cách **đổi mẫu cấp phát**, không phải bằng cách "cẩn thận hơn".

### Nếu code ĐÃ LỠ dùng STL khắp nơi

Không phải viết lại — C++17 có **`std::pmr`** (polymorphic memory resource): giữ nguyên container, chỉ đổi **nguồn cấp phát**.

```cpp
#include <memory_resource>

static std::array<std::byte, 64 * 1024> pool;            // bộ nhớ TĨNH, cấp 1 lần
std::pmr::monotonic_buffer_resource res{pool.data(), pool.size()};

std::pmr::vector<Sample>          samples{&res};          // vẫn là vector
std::pmr::string                  name{&res};
std::pmr::unordered_map<int, Log> logs{&res};

// ... dùng bình thường ...
res.release();      // "giải phóng" cả pool trong O(1) — reset con trỏ, không free từng khối
```

| Memory resource | Hành vi | Hợp với |
|---|---|---|
| `monotonic_buffer_resource` | Chỉ cấp, **không** trả lẻ; reset cả khối | Dữ liệu theo **khung/vòng lặp** — reset mỗi chu kỳ |
| `unsynchronized_pool_resource` | Pool theo size-class, có trả lẻ | Object vòng đời lẫn lộn, **một luồng** |
| `synchronized_pool_resource` | Như trên, có khoá | Nhiều luồng dùng chung pool |

**Đánh đổi phải nói ra:** `pmr` thêm **một lần gián tiếp qua vtable** ở mỗi lần cấp phát (memory resource là interface ảo). Đổi lại: tất định, không phân mảnh heap toàn cục, và **không phải sửa logic**. Với hệ chạy dài ngày, đây gần như luôn là giao dịch có lợi.

**Chốt:** *"Phân mảnh sinh ra từ mẫu cấp phát chứ không từ việc quên free — nên phải đổi mẫu, không phải đổi kỷ luật. Cấp một lần lúc init, sau đó chỉ mượn-trả trong pool; code cũ dùng STL thì bọc bằng `std::pmr` thay vì viết lại."*
- **Trade-offs**: pool (nhanh, tất định, nhưng phí RAM giữ chỗ + phải ước lượng đúng) vs heap (linh hoạt, rủi ro); nhiều size-class (ít phí) vs một cỡ (đơn giản).
- **Failure**: pool cạn → chính sách rõ (drop/chờ/degrade), không tràn âm thầm; phát hiện leak-logic (mượn không trả) bằng đếm slot; guard chống double-release.
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

---

## D — Thiết kế API & thư viện

#### SD-009 · 🔴 · design · 🏗️ · ⭐ · [→ api-design](../../../07-shared-libraries/api-design.md), [abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Thiết kế một C++ shared library/API cho lớp trên dùng. Bạn quan tâm gì?**
<details><summary>Đáp án</summary>

- **API tốt**: tối thiểu (mọi thứ public là cam kết maintain), che giấu chi tiết, nhất quán, dễ dùng đúng/khó dùng sai (kiểu mạnh, RAII, `[[nodiscard]]`), tài liệu hóa hợp đồng (tiền/hậu điều kiện, ownership, thread-safety).
- **Ownership rõ ràng** qua kiểu: trả `unique_ptr` (sở hữu), nhận `const&`/`span` (mượn).
- **Error handling nhất quán**: exception (C++ thuần) hoặc mã lỗi/`expected` (biên giới C/embedded).
- **Ổn định ABI**: pimpl để giấu data member; biên giới C (`extern "C"`, POD, opaque handle) nếu cần đa compiler/ngôn ngữ; versioning kỷ luật (soname theo major, không đổi chữ ký/layout đã phát hành — chỉ thêm).
- Cẩn thận virtual function trong interface public (thêm virtual phá vtable).
</details>

#### SD-021 · 🟠 · concept · ⭐ · 📦 2026-08-13 · [→ api-design](../../../07-shared-libraries/api-design.md)
**Pimpl idiom là gì? Nó giải quyết vấn đề gì, và cái giá là gì?**
<details><summary>Đáp án</summary>

**Pimpl = "Pointer to IMPLementation"**: mọi thành viên dữ liệu chuyển vào một struct giấu trong `.cpp`; header chỉ còn **một con trỏ**.

```cpp
// widget.h — client CHỈ thấy chừng này
class Widget {
public:
    Widget(); ~Widget();                 // ⚠️ phải khai và định nghĩa trong .cpp
    Widget(Widget&&) noexcept;
    Widget& operator=(Widget&&) noexcept;
    void draw();
private:
    struct Impl;                          // khai báo trước, KHÔNG định nghĩa
    std::unique_ptr<Impl> pImpl;          // sizeof(Widget) = sizeof(pointer), MÃI MÃI
};
```

**Ba vấn đề nó giải quyết:**
1. ⭐ **Ổn định ABI** — `sizeof(Widget)` không bao giờ đổi dù bạn thêm bao nhiêu thành viên ⇒ khách thay `.so` mà không build lại vẫn an toàn ([SD-017](system-design.md)).
2. **Giảm phụ thuộc biên dịch** — header không cần `#include` các kiểu nội bộ ⇒ sửa chi tiết cài đặt **không bắt client biên dịch lại**; với dự án lớn đây là khác biệt hàng chục phút mỗi lần build.
3. **Giấu chi tiết thật sự** — client không đọc được cấu trúc nội bộ từ header.

**⚠️ Ba cái bẫy kỹ thuật (hay bị hỏi):**
1. **Destructor phải khai trong header và ĐỊNH NGHĨA trong `.cpp`.** Để compiler sinh destructor ở header thì tại đó `Impl` còn **incomplete** ⇒ `unique_ptr` xoá một kiểu chưa đầy đủ ⇒ **lỗi biên dịch khó hiểu**. Cùng lý do với move ctor/assign.
2. **Khai destructor ⇒ mất move tự sinh** ⇒ phải `= default` move trong `.cpp` ([CPP-020](cpp.md)).
3. `const` method **không** tự lan sang `pImpl` (con trỏ const trỏ tới non-const) — cần `propagate_const` hoặc kỷ luật thủ công.

**Cái giá:** một lần **cấp phát động** cho mỗi đối tượng + một lần **gián tiếp** mỗi lần chạm thành viên + **mất khả năng inline**. ⇒ **Không dùng** cho lớp nhỏ, tạo/huỷ nhiều, nằm trong vòng lặp nóng. Dùng cho lớp ở **biên giới thư viện**, số lượng ít, vòng đời dài.

**Chốt:** *"Pimpl giấu toàn bộ dữ liệu sau một con trỏ nên `sizeof` cố định vĩnh viễn — ổn định ABI và cắt phụ thuộc biên dịch. Trả bằng một cấp phát + một gián tiếp mỗi đối tượng, nên chỉ dùng ở biên giới thư viện."*
</details>

#### SD-022 · 🟠 · design · ⭐ · 📦 2026-08-13 · [→ api-design](../../../07-shared-libraries/api-design.md)
**Vì sao thư viện hệ thống thường phơi **C API** dù bên trong viết bằng C++? Ở biên giới C phải tuân thủ quy tắc gì?**
<details><summary>Đáp án</summary>

**Bốn lý do phơi C API:**
1. ⭐ **ABI của C ổn định, của C++ thì không.** C không có name mangling phức tạp, không vtable, không exception đi qua biên ⇒ cùng một `.so` dùng được với **compiler khác, phiên bản khác** ([SD-018](system-design.md)).
2. **Mọi ngôn ngữ đều gọi được C** — Python, Rust, Go, Java, C# đều có cơ chế gọi C. Không ngôn ngữ nào gọi thẳng C++ được.
3. **Không kéo theo runtime C++** — không phụ thuộc phiên bản libstdc++ trên máy khách.
4. **Bề mặt nhỏ, dễ giữ ổn định** qua nhiều năm.

**⚠️ Quy tắc bắt buộc ở biên giới C:**

| Quy tắc | Vì sao |
|---|---|
| **`extern "C"`** cho mọi hàm phơi ra | Tắt name mangling ⇒ symbol có tên ổn định ([SD-028](system-design.md)) |
| **KHÔNG để exception thoát ra** | Ném xuyên qua biên C là **UB**. Bọc `try/catch(...)` ở **mọi** hàm phơi ra, đổi thành mã lỗi |
| **Chỉ dùng kiểu POD & con trỏ mờ** | Không `std::string`, không `std::vector`, không lớp có vtable |
| **Sở hữu đối xứng** | Thư viện cấp thì **thư viện giải phóng** (`foo_create`/`foo_destroy`). Client `free()` bộ nhớ do thư viện `new` là hỏng khi khác allocator |
| **Không truyền `bool`/`enum` C++ trần** | Kích thước có thể khác; dùng kiểu số có độ rộng cố định |
| **Struct công khai có trường `size`** | Để mở rộng sau này mà không phá ABI ([SD-020](system-design.md)) |

```c
typedef struct Scanner Scanner;                 /* con trỏ mờ — client không thấy bố cục */
Scanner* scanner_create(const ScannerCfg* cfg);
int      scanner_read(Scanner* s, uint8_t* buf, size_t len, size_t* out_len);
void     scanner_destroy(Scanner* s);           /* đối xứng với create */
```

**Đánh đổi:** API C **kém tiện** hơn nhiều (không RAII, không template, phải kiểm mã lỗi thủ công). ⇒ Mẫu thực tế: **lõi C++ → biên giới C → lớp bọc C++ (header-only) cho client C++**. Client C++ vẫn có RAII và exception, mà **ranh giới nhị phân vẫn là C**.

**Chốt:** *"C API ổn định về ABI, gọi được từ mọi ngôn ngữ, không kéo runtime C++. Đổi lại phải chặn exception ở biên, chỉ dùng POD và con trỏ mờ, và giữ sở hữu đối xứng — rồi bọc lại bằng header C++ cho tiện."*
</details>

#### SD-023 · 🟠 · design · 📦 2026-08-13 · [→ api-design](../../../07-shared-libraries/api-design.md)
**Làm sao thể hiện **quyền sở hữu** rõ ràng qua kiểu dữ liệu trong một API C++?**
<details><summary>Đáp án</summary>

**Vấn đề:** `Widget* getWidget();` **không nói gì** — người gọi phải `delete` hay không? Tài liệu có thể sai hoặc không ai đọc. Hệ quả là leak (không ai xoá) hoặc double-free (cả hai cùng xoá).

⇒ **Nguyên tắc: sở hữu phải nằm trong KIỂU, không nằm trong tài liệu.** Compiler và người đọc cùng biết.

| Ý định | Kiểu nên dùng | Người gọi hiểu ngay |
|---|---|---|
| **Trao quyền sở hữu** | `std::unique_ptr<T>` | *"Của tôi rồi, tôi lo huỷ"* |
| **Chia sẻ sở hữu** | `std::shared_ptr<T>` | *"Cùng sở hữu, hết tham chiếu mới huỷ"* |
| **Chỉ cho mượn, chắc chắn tồn tại** | `T&` | *"Dùng đi, đừng giữ lại, đừng xoá"* |
| **Cho mượn, có thể không có** | `T*` (thô, **không sở hữu**) hoặc `std::optional<T&>` | *"Có thể null; không xoá"* |
| **Cho xem chuỗi/mảng, không giữ** | `std::string_view`, `std::span` | *"Chỉ đọc, và **không được giữ quá lâu**"* |

**Ba quy ước làm rõ thêm:**
1. ⭐ **Con trỏ thô = KHÔNG sở hữu.** Đặt luật này cho toàn dự án ⇒ thấy `T*` là biết không phải xoá. Sở hữu **luôn** đi qua smart pointer.
2. **Tham số nhận theo giá trị + `std::move`** khi hàm sẽ giữ lại — nó nói *"tôi lấy một bản của bạn"* và cho người gọi chọn copy hay move ([CPP-050](cpp.md)).
3. **Factory trả `unique_ptr`**, không trả con trỏ thô — vừa rõ sở hữu vừa an toàn với exception.

⚠️ **Ở biên giới nhị phân của thư viện thì KHÔNG dùng được các kiểu này** (smart pointer là kiểu C++, phá ABI). Ở đó dùng cặp **`create`/`destroy` đối xứng** trên con trỏ mờ, và ghi rõ trong tên hàm ([SD-022](system-design.md)).

⚠️ **Bẫy `string_view`/`span`:** chúng **không sở hữu** ⇒ nếu API lưu chúng lại thì đó là con trỏ treo chờ nổ. Chỉ dùng cho tham số **đọc rồi bỏ** ([CPP-037](cpp.md)).

**Chốt:** *"Đặt sở hữu vào kiểu: `unique_ptr` là trao, `shared_ptr` là chia, tham chiếu và con trỏ thô là mượn. Con trỏ thô không bao giờ sở hữu — đó là quy ước phải thống nhất toàn dự án."*
</details>

#### SD-024 · 🟠 · design · 📦 2026-08-13 · [→ api-design](../../../07-shared-libraries/api-design.md)
**Thư viện của bạn nên báo lỗi bằng exception hay mã lỗi? Vì sao phải chọn NHẤT QUÁN?**
<details><summary>Đáp án</summary>

| | **Exception** | **Mã lỗi / `expected`** |
|---|---|---|
| Không thể bỏ sót | ✅ Không bắt thì chương trình dừng | ❌ **Bỏ qua giá trị trả về là im lặng** |
| Đường code chính | **Sạch** — không lẫn kiểm tra lỗi | Rối vì `if (rc != OK)` khắp nơi |
| Chi phí khi **không** lỗi | ~0 | Một phép so sánh |
| Chi phí khi **có** lỗi | **Đắt** (unwind), khó dự đoán | Rẻ, tất định |
| Qua biên giới C/ABI | ❌ **UB** | ✅ Đi qua được |
| Hệ nhúng / realtime | Thường **bị cấm** (kích thước, tính tất định) | ✅ Hợp |

**Chọn theo ngữ cảnh:**
- **Thư viện C++ nội bộ, ứng dụng thường** ⇒ **exception** cho lỗi *thật sự bất thường*, và `std::optional`/`expected` cho **lỗi được mong đợi** (không tìm thấy, hết dữ liệu). Đây là ranh giới quan trọng: *hết file* không phải chuyện bất thường ⇒ đừng ném.
- **Thư viện có biên giới C**, hoặc firmware/realtime, hoặc dự án cấm exception ⇒ **mã lỗi**.

**⭐ Vì sao NHẤT QUÁN quan trọng hơn chọn cái nào:** trộn hai kiểu buộc người dùng **phải kiểm cả hai đường** ở mọi lời gọi. Thực tế họ sẽ quên một trong hai ⇒ lỗi lọt qua im lặng. Một thư viện *"đa số trả mã lỗi nhưng vài hàm ném"* nguy hiểm hơn hẳn thư viện chỉ dùng một kiểu — kể cả kiểu kém hơn.

**Ba quy tắc dù chọn cách nào:**
1. **Constructor không có mã lỗi** ⇒ hoặc ném, hoặc dùng **factory** trả `expected`/`optional` và để constructor luôn thành công.
2. **`[[nodiscard]]`** cho mọi hàm trả mã lỗi ⇒ compiler cảnh báo khi bị bỏ qua, vá đúng điểm yếu lớn nhất của mã lỗi.
3. **Đảm bảo an toàn ngoại lệ** phải nêu rõ trong tài liệu (mạnh / cơ bản / không ném) — và **destructor không bao giờ được ném** ([CPP-027](cpp.md)).

**Chốt:** *"Exception cho lỗi bất thường trong C++ thuần; mã lỗi khi có biên giới C hoặc ràng buộc nhúng; `optional`/`expected` cho lỗi được mong đợi. Quan trọng nhất là nhất quán — trộn hai kiểu là cách chắc chắn để lỗi lọt qua."*
</details>

---

## E — ABI & versioning

#### SD-017 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Phân biệt API và ABI. Cho một thay đổi **giữ nguyên API** nhưng **phá ABI**.**
<details><summary>Đáp án</summary>

| | **API** (Application Programming Interface) | **ABI** (Application Binary Interface) |
|---|---|---|
| Hợp đồng ở mức | **Mã nguồn** | **Nhị phân** |
| Gồm những gì | Tên hàm, kiểu tham số, ngữ nghĩa | **Kích thước & bố cục struct**, offset thành viên, thứ tự vtable, quy ước gọi hàm, tên symbol đã mangle |
| Phá vỡ thì | **Biên dịch lại là hỏng** — thấy ngay | **Vẫn biên dịch, vẫn chạy** — hỏng lúc chạy, im lặng |
| Ai quan tâm | Người viết code dùng thư viện | Người **chỉ thay file `.so`** mà không build lại |

**⭐ Ví dụ kinh điển — thêm một thành viên vào struct:**
```cpp
// v1.0 (app đã biên dịch với bản này)     // v1.1 — API y hệt, ABI ĐÃ VỠ
struct Config { int timeout; };            struct Config { int timeout; int retries; };
```
`sizeof(Config)` đổi từ 4 → 8. App cũ vẫn cấp phát **4 byte**, còn thư viện mới ghi vào **8 byte** ⇒ **ghi đè ra ngoài** ⇒ hỏng bộ nhớ ngẫu nhiên, crash ở chỗ chẳng liên quan. Không có thông báo lỗi nào, không có cảnh báo lúc liên kết.

**Vì sao ABI đau hơn API:** lỗi API xuất hiện **lúc biên dịch**, có thông báo rõ ràng, sửa xong là hết. Lỗi ABI xuất hiện **ở máy khách**, biểu hiện thành crash ngẫu nhiên hoặc dữ liệu sai, và triệu chứng **cách xa nguyên nhân** ⇒ đúng lớp bug tốn hàng tuần.

**Dấu hiệu nhận biết trong thực tế:** *"khách chép `.so` mới vào rồi app crash, chép lại bản cũ thì hết"* — nghe câu này là nghi ABI break ngay ([SD-010](system-design.md)).

**Chốt:** *"API là hợp đồng mức mã nguồn, ABI là hợp đồng mức nhị phân — bố cục struct, vtable, tên symbol. Thêm một field vào struct giữ nguyên API nhưng phá ABI, và nó hỏng im lặng ở máy khách."*
</details>

#### SD-018 · 🟠 · concept · ⭐ · 📦 2026-08-13 · [→ abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Những thay đổi nào trong C++ thường phá ABI? Vì sao C++ dễ vỡ ABI hơn C nhiều?**
<details><summary>Đáp án</summary>

**Danh sách phá ABI — nhóm theo cơ chế:**

| Nhóm | Thay đổi | Vì sao vỡ |
|---|---|---|
| **Bố cục dữ liệu** | Thêm/bớt/đổi thứ tự **thành viên**; đổi kiểu thành viên; đổi alignment/packing | Offset của mọi thành viên sau đó **dịch đi** |
| **vtable** | Thêm/bớt/đổi **thứ tự** hàm virtual; thêm base class; đổi một hàm thường thành virtual | Lời gọi virtual là *"nhảy tới ô thứ N của vtable"* — đổi thứ tự là gọi **nhầm hàm** |
| **Chữ ký hàm** | Đổi kiểu tham số/trả về, thêm tham số (kể cả có giá trị mặc định), đổi `const` | Tên đã **mangle** thay đổi ⇒ symbol cũ biến mất |
| **Ngữ nghĩa ngầm** | Đổi giá trị enum, đổi kích thước mảng thành viên, đổi quy ước sở hữu (ai `delete`) | Vẫn liên kết được nhưng **hành vi sai** |
| **Môi trường** | Đổi compiler/phiên bản/cờ ảnh hưởng bố cục, đổi phiên bản libstdc++ | Bố cục kiểu chuẩn có thể khác |

**⭐ Vì sao C++ dễ vỡ hơn C nhiều — ba lý do:**
1. **C++ phơi bày bố cục lớp ra header.** Client biên dịch theo `sizeof` và offset **tại thời điểm build của họ**; chúng bị "nướng" vào mã máy của họ. C cũng có vấn đề này với struct, nhưng C thường dùng con trỏ mờ (opaque) nhiều hơn.
2. **vtable là một mảng có thứ tự.** Trong C không có khái niệm tương đương ⇒ C++ có thêm cả một chiều để vỡ.
3. **Name mangling mã hoá cả chữ ký** ⇒ mọi thay đổi nhỏ đều đổi tên symbol; và **quy tắc mangle khác nhau giữa compiler** ([CPP-023](cpp.md)).

⇒ **Hệ quả thiết kế:** đây chính là lý do các thư viện C++ nghiêm túc hoặc **phơi C API** ([SD-022](system-design.md)), hoặc dùng **Pimpl** để giấu toàn bộ bố cục ([SD-021](system-design.md)), hoặc chỉ phơi **interface thuần ảo** với cam kết không bao giờ đổi thứ tự.

**Bẫy:** (1) tưởng thêm hàm virtual **ở cuối** là an toàn — không, lớp dẫn xuất của client đã có vtable riêng; (2) tưởng thêm tham số **có giá trị mặc định** là an toàn về ABI — không, giá trị mặc định được điền **ở phía client** và tên symbol vẫn đổi; (3) đổi từ `std::string` này sang kiểu khác trong struct công khai.

**Chốt:** *"C++ phơi bố cục lớp và thứ tự vtable ra header, mà cả hai đều bị nướng vào mã của client — nên gần như mọi thay đổi cấu trúc đều phá ABI. Đó là lý do thư viện nghiêm túc giấu bố cục sau Pimpl hoặc phơi C API."*
</details>

#### SD-019 · 🟠 · concept · 📦 2026-08-13 · [→ abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**`soname` là gì và liên quan thế nào tới tương thích ABI? Symbol versioning khác gì với việc tăng soname?**
<details><summary>Đáp án</summary>

**Ba cái tên của một thư viện — phải phân biệt:**

| Tên | Ví dụ | Ai dùng |
|---|---|---|
| **Tên thật** (real name) | `libfoo.so.1.4.2` | File thật trên đĩa |
| **soname** | `libfoo.so.1` | **Được nướng vào chương trình lúc liên kết**; loader tìm đúng tên này lúc chạy |
| **Linker name** | `libfoo.so` | Symlink, chỉ dùng lúc **build** |

**Luật:** **soname = lời hứa về ABI.** Cùng soname ⇒ hứa **tương thích ABI ngược** (chương trình build với `1.4.2` chạy được với `1.5.0`). **Phá ABI ⇒ BẮT BUỘC tăng soname** (`.so.1` → `.so.2`).

⇒ Nhờ vậy hai phiên bản **cùng tồn tại** trên một máy: app cũ nạp `libfoo.so.1`, app mới nạp `libfoo.so.2`, không giẫm lên nhau. Đây là cách Linux tránh "DLL hell".

**Symbol versioning — công cụ tinh vi hơn:** gắn **phiên bản cho từng symbol** bên trong *cùng một* thư viện, giữ đồng thời **nhiều bản của một hàm**:

| | **Tăng soname** | **Symbol versioning** |
|---|---|---|
| Mức chi tiết | **Cả thư viện** | **Từng hàm** |
| App cũ | Phải giữ file `.so.1` riêng | Vẫn dùng `.so.1`, gọi vào bản cũ của hàm — **cùng một file** |
| Chi phí bảo trì | Thấp | **Cao** — phải giữ mã của mọi bản cũ mãi mãi |
| Dùng khi | **Mặc định** cho hầu hết dự án | Thư viện nền tảng cực rộng (glibc) — nơi bắt cả hệ thống nâng cấp là bất khả thi |

**Thực dụng:** dự án bình thường **cứ tăng soname**. Symbol versioning chỉ đáng khi bạn là glibc — tức là không thể yêu cầu toàn thế giới build lại.

**Bẫy:** (1) **quên tăng soname sau khi phá ABI** — đây là nguyên nhân gốc của phần lớn ca *"chép `.so` mới vào là app crash"*; (2) tăng phiên bản **file** nhưng giữ nguyên soname (đổi `1.4.2`→`1.5.0` mà vẫn `.so.1`) ⇒ **đúng** nếu tương thích, **thảm hoạ** nếu không; (3) đóng gói thiếu symlink ⇒ build được ở máy dev, khách không chạy được.

**Chốt:** *"soname là lời hứa ABI được nướng vào chương trình — phá ABI thì phải tăng soname để hai phiên bản cùng sống. Symbol versioning làm ở mức từng hàm, mạnh hơn nhưng phải nuôi mã cũ mãi, nên chỉ hợp thư viện nền tảng."*
</details>

#### SD-010 · 🔴 · design · 🏗️ · ⭐ · [→ abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Thư viện cập nhật phiên bản mới khiến app khách hàng (không build lại) crash. Nguyên nhân khả dĩ và cách tránh?**
<details><summary>Đáp án</summary>

Nguyên nhân khả dĩ: **ABI break** dù API không đổi (app không build lại nên dùng binary cũ với `.so` mới). Vd: thêm data member vào struct/class public (đổi sizeof/offset), thêm/đổi thứ tự virtual function (đổi vtable), đổi chữ ký, đổi kích thước/alignment kiểu. Cách tránh: giữ ABI tương thích — pimpl giấu data member, chỉ thêm hàm mới thay vì sửa cái cũ, không đổi layout đã phát hành, thận trọng virtual; nếu buộc phá ABI thì tăng major + đổi soname (`libfoo.so.2`) để hai bản cùng tồn tại; dùng symbol versioning; kiểm tra bằng `abidiff`/abi-compliance-checker trước khi phát hành.
</details>

#### SD-020 · 🔴 · design · ⭐ · 🏗️ · 📦 2026-08-13 · [→ abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Bạn duy trì một shared library C++ cho khách hàng chỉ thay file `.so`, không build lại. Thiết kế thế nào để phát triển tiếp mà không phá ABI?**
<details><summary>Đáp án</summary>

**Nguyên tắc gốc: cái gì KHÔNG phơi ra thì không thể phá.** Mọi kỹ thuật dưới đây đều là biến thể của việc **giảm bề mặt nhị phân**.

**Bốn kỹ thuật, theo thứ tự hiệu quả:**

| # | Kỹ thuật | Giải quyết gì |
|---|---|---|
| **1** | **Pimpl** — mọi thành viên dữ liệu giấu sau một con trỏ | `sizeof` lớp **không bao giờ đổi** ⇒ thêm/bớt/đổi thành viên thoải mái ([SD-021](system-design.md)) |
| **2** | **Interface thuần ảo + factory** — client chỉ thấy lớp abstract, đối tượng do thư viện tạo | Client không biết bố cục lớp thật. ⚠️ **Chỉ được THÊM hàm virtual ở CUỐI**, không bao giờ đổi thứ tự |
| **3** | **Phơi C API** ở biên giới, C++ chỉ ở bên trong | Xoá bỏ cả vtable lẫn name mangling ⇒ ổn định nhất ([SD-022](system-design.md)) |
| **4** | **Struct có version + trường dự phòng** | Struct công khai đặt `uint32_t size` ở đầu (client điền `sizeof`) ⇒ thư viện biết client dùng bản nào và xử lý được cả hai |

**Kỷ luật vận hành đi kèm — kỹ thuật thôi không đủ:**
- **Kiểm soát symbol xuất ra.** Mặc định mọi symbol đều công khai ⇒ vô tình biến chi tiết nội bộ thành ABI. Ẩn hết, chỉ xuất tường minh những gì có chủ đích. *(Cách làm cụ thể là T3.)*
- **Đặt luật rõ về sở hữu:** ai cấp phát thì **người đó giải phóng** — client `new` rồi thư viện `delete` là hỏng khi hai bên dùng runtime khác ([SD-023](system-design.md)).
- **Không bao giờ để kiểu STL đi qua biên giới** trong ca hỗ trợ nhiều compiler: `std::string`/`std::vector` có bố cục khác nhau giữa các phiên bản libstdc++.
- **Kiểm ABI tự động trong CI** — so bề mặt nhị phân bản mới với bản phát hành trước; phá thì **fail build**, đừng trông chờ vào việc nhớ.
- **Tăng soname khi buộc phải phá** ([SD-019](system-design.md)) — phá ABI có kiểm soát vẫn tốt hơn phá âm thầm.

⚠️ **Đánh đổi phải nói ra:** Pimpl thêm **một lần gián tiếp + một lần cấp phát** mỗi đối tượng, và chặn inline ⇒ không dùng cho lớp nhỏ, tạo nhiều, nằm trong đường nóng. Interface thuần ảo thêm lời gọi ảo. **Ổn định ABI không miễn phí** — đổi lấy hiệu năng và sự tiện lợi.

**Chốt:** *"Giảm bề mặt nhị phân: giấu dữ liệu sau Pimpl, phơi interface thuần ảo hoặc C API, kiểm soát symbol xuất ra — rồi bắt CI kiểm ABI mỗi lần phát hành. Và chấp nhận trả bằng một lần gián tiếp."*
</details>

---

## F — Linking & loading

#### SD-025 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ linking-loading](../../../07-shared-libraries/linking-loading.md)
**Phân biệt linking và loading. Static và dynamic linking khác nhau ở đâu?**
<details><summary>Đáp án</summary>

**Hai giai đoạn khác nhau, khác cả thời điểm lẫn người thực hiện:**

| | **Linking** | **Loading** |
|---|---|---|
| Khi nào | **Lúc build** (hoặc lúc chạy với thư viện động) | **Lúc chạy chương trình** |
| Ai làm | **Linker** (`ld`) | **Kernel + dynamic loader** |
| Làm gì | Gộp object file, **phân giải symbol**, gán địa chỉ | Đọc file thực thi vào bộ nhớ, nạp các `.so` phụ thuộc, **phân giải nốt symbol động** |

**Static vs dynamic:**

| | **Static** (`.a`) | **Dynamic** (`.so`) |
|---|---|---|
| Mã thư viện nằm ở | **Chép thẳng vào** file thực thi | File `.so` riêng, nạp lúc chạy |
| Kích thước file thực thi | Lớn | Nhỏ |
| RAM khi nhiều tiến trình cùng dùng | **Mỗi tiến trình một bản** | **Dùng chung một bản** ([SD-030](system-design.md)) |
| Vá lỗi bảo mật trong thư viện | **Phải build lại toàn bộ** ứng dụng | Thay file `.so` là xong |
| Thời gian khởi động | Nhanh hơn | Chậm hơn chút (phân giải symbol) |
| Rủi ro | Không có | **ABI / thiếu thư viện ở máy khách** |

**Chọn thế nào:**
- **Static:** firmware/thiết bị nhúng đơn chương trình · muốn triển khai một file duy nhất · muốn tất định tuyệt đối về phiên bản.
- **Dynamic:** hệ có nhiều chương trình dùng chung thư viện · cần vá bảo mật độc lập · thư viện là sản phẩm giao cho khách ([SD-031](system-design.md)).

**Bẫy:** (1) tưởng static luôn nhanh hơn — với **một** tiến trình thì gần như bằng nhau; lợi thế của dynamic là **RAM khi có nhiều tiến trình**; (2) static không loại bỏ được vấn đề phiên bản — nó chỉ **đóng băng** nó vào lúc build, và lỗi bảo mật thì bạn phải phát hành lại tất cả; (3) quên rằng thứ tự thư viện trên dòng lệnh linker **có ý nghĩa** với `.a`.

**Chốt:** *"Linking gộp và phân giải symbol lúc build; loading nạp và phân giải nốt lúc chạy. Static nhét mã vào file thực thi (một file, không lo phiên bản); dynamic chia sẻ được RAM và vá được độc lập, đổi lấy rủi ro ABI."*
</details>

#### SD-026 · 🟡 · concept · 📦 2026-08-13 · [→ linking-loading](../../../07-shared-libraries/linking-loading.md)
**Symbol là gì? Khi nào gặp *undefined reference* và khi nào *multiple definition*?**
<details><summary>Đáp án</summary>

**Symbol** là **cái tên** mà linker dùng để nối "chỗ dùng" với "chỗ định nghĩa" — tên hàm hoặc biến toàn cục, kèm thông tin nó là **định nghĩa** hay chỉ là **tham chiếu**.

| Lỗi | Nghĩa | Nguyên nhân thường gặp |
|---|---|---|
| **undefined reference** | Có **chỗ dùng**, **không có** định nghĩa nào | Quên liên kết thư viện · khai báo mà chưa cài đặt · **sai thứ tự thư viện** trên dòng lệnh (`.a` chỉ lấy cái đang thiếu tại thời điểm gặp nó) · **thiếu `extern "C"`** khi trộn C và C++ ([SD-028](system-design.md)) · định nghĩa template nằm trong `.cpp` ([CPP-009](cpp.md)) |
| **multiple definition** | Có **nhiều hơn một** định nghĩa | Định nghĩa biến/hàm **trong header** rồi include nhiều nơi (thiếu `inline` hoặc `extern`) · gộp cùng một `.cpp` hai lần |

**Ba luật giúp tránh cả hai:**
1. **Header chỉ nên KHAI BÁO**, không định nghĩa. Buộc phải định nghĩa trong header ⇒ đánh dấu **`inline`** (hoặc `constexpr`, hoặc C++17 `inline` cho biến).
2. **Biến toàn cục:** `extern` ở header, định nghĩa ở **đúng một** `.cpp`.
3. **Template phải định nghĩa trong header** — compiler cần thấy toàn bộ khi *instantiate*; đây là nguyên nhân *undefined reference* rất hay gặp mà người mới không nghĩ tới.

⚠️ **Bẫy include guard:** guard chỉ chống include **hai lần trong cùng một đơn vị dịch**; nó **không** chống việc hai `.cpp` khác nhau cùng sinh ra một định nghĩa ⇒ vẫn *multiple definition* ở bước link. Đây là hiểu nhầm phổ biến.

**Chốt:** *"undefined = có người dùng mà không ai định nghĩa; multiple = nhiều nơi cùng định nghĩa. Header khai báo, `.cpp` định nghĩa, template thì bắt buộc ở header — và include guard không cứu được multiple definition giữa các file."*
</details>

#### SD-027 · 🟠 · concept · 📦 2026-08-13 · [→ linking-loading](../../../07-shared-libraries/linking-loading.md)
**Dynamic loader làm gì khi chương trình khởi động? PLT/GOT và lazy binding để làm gì?**
<details><summary>Đáp án</summary>

**Trình tự khi chạy một chương trình liên kết động:**
1. Kernel nạp file thực thi, thấy nó cần **interpreter** ⇒ nạp và trao quyền cho **dynamic loader**.
2. Loader đọc danh sách **phụ thuộc** (theo **soname** — [SD-019](system-design.md)), tìm từng `.so` theo thứ tự đường dẫn tìm kiếm, **đệ quy** cho cả phụ thuộc của phụ thuộc.
3. `mmap` từng thư viện vào không gian địa chỉ.
4. **Relocation** — điền các địa chỉ thật vào những chỗ còn trống.
5. Chạy **hàm khởi tạo** của từng thư viện (constructor của object toàn cục), rồi mới nhảy vào `main`.

**Vấn đề cần giải:** thư viện được nạp ở **địa chỉ khác nhau mỗi lần** (ASLR, và nó dùng chung nên không thể cố định) ⇒ **không thể nướng địa chỉ tuyệt đối vào mã**, vì mã là **chia sẻ chỉ-đọc** giữa các tiến trình.

**Lời giải — thêm một tầng gián tiếp:**

| | Là gì | Nằm ở |
|---|---|---|
| **GOT** (Global Offset Table) | **Bảng địa chỉ** — mã tra bảng thay vì chứa địa chỉ | Vùng **dữ liệu** (ghi được, riêng mỗi tiến trình) |
| **PLT** (Procedure Linkage Table) | Các đoạn nhảy trung gian cho **lời gọi hàm** | Vùng mã |

⇒ Mã máy chỉ chứa *"nhảy tới ô số N của GOT"*. Loader điền GOT lúc chạy ⇒ **mã không đổi** ⇒ vẫn chia sẻ được giữa các tiến trình. Đây chính là lý do phải biên dịch thư viện với **mã độc lập vị trí** ([SD-030](system-design.md)).

**Lazy binding:** phân giải một hàm **lần đầu tiên nó được gọi**, thay vì phân giải hết lúc khởi động. Lần gọi đầu đi qua PLT tới loader, loader tìm địa chỉ thật, **ghi vào GOT**; các lần sau nhảy thẳng.
- **Được:** khởi động nhanh hơn nhiều với thư viện có hàng nghìn symbol mà chương trình chỉ dùng vài chục.
- **Mất:** lần gọi đầu tốn thêm; và lỗi *"thiếu symbol"* **lộ ra giữa chừng lúc chạy** thay vì lúc khởi động ⇒ với hệ cần tất định (realtime, an toàn) thì nên **tắt lazy binding** để phân giải hết ngay từ đầu và biết sớm.

**Chốt:** *"Loader nạp `.so` theo soname, mmap, relocation rồi chạy hàm khởi tạo. GOT/PLT là tầng gián tiếp để mã không chứa địa chỉ tuyệt đối nên chia sẻ được; lazy binding hoãn phân giải tới lần gọi đầu — nhanh khởi động nhưng lỗi thiếu symbol lộ ra muộn."*
</details>

#### SD-028 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ linking-loading](../../../07-shared-libraries/linking-loading.md)
**C++ name mangling là gì? `extern "C"` dùng để làm gì?**
<details><summary>Đáp án</summary>

**Name mangling:** compiler C++ mã hoá **chữ ký đầy đủ** (namespace, tên lớp, kiểu tham số, `const`…) vào **tên symbol**.

```cpp
void  foo(int);        →  _Z3fooi
void  foo(double);     →  _Z3food        // khác tên ⇒ overload hoạt động được
extern "C" void bar(int);  →  bar        // giữ nguyên
```

**Vì sao cần:** C++ cho phép **overload**, namespace, hàm thành viên — mà linker chỉ biết **tên**. Không mã hoá kiểu vào tên thì hai `foo` khác nhau sẽ đụng nhau. Mangling là cách C++ nhét thông tin kiểu vào một thế giới chỉ có tên.

**`extern "C"` tắt mangling** ⇒ symbol mang **đúng tên bạn viết**. Dùng khi:
1. **Phơi API từ thư viện C++** để ngôn ngữ khác gọi được ([SD-022](system-design.md)).
2. **Gọi hàm C từ C++** — header C phải bọc trong `extern "C"` (header hệ thống đã tự bọc sẵn bằng `#ifdef __cplusplus`).
3. **Callback truyền cho API C** — nhiều API C yêu cầu con trỏ hàm có liên kết C.

```cpp
#ifdef __cplusplus
extern "C" {
#endif
    int scanner_read(void* h, unsigned char* buf, unsigned len);
#ifdef __cplusplus
}
#endif
```

**⚠️ Ba giới hạn:**
1. **`extern "C"` KHÔNG overload được** — chỉ được một hàm cho mỗi tên (vì tên không còn mã hoá kiểu).
2. Nó **chỉ tắt mangling**, **không** làm hàm an toàn với biên giới ABI: exception vẫn không được thoát ra, kiểu tham số vẫn phải là POD ([SD-022](system-design.md)).
3. **Quy tắc mangle khác nhau giữa compiler** ⇒ trộn `.so` build bằng hai compiler C++ khác nhau là nguồn của *undefined reference* khó hiểu ([CPP-023](cpp.md)) — đây cũng là lý do C API ổn định hơn.

**Chốt:** *"Mangling mã hoá chữ ký vào tên symbol để overload hoạt động; `extern \\"C\\"` tắt nó để có tên ổn định gọi được từ ngôn ngữ khác. Nhưng nó chỉ sửa cái tên — exception và kiểu C++ vẫn không được đi qua biên."*
</details>

#### SD-029 · 🟠 · concept · 📦 2026-08-13 · [→ linking-loading](../../../07-shared-libraries/linking-loading.md)
**`dlopen`/`dlsym` dùng để làm gì? Khác liên kết động thông thường ra sao?**
<details><summary>Đáp án</summary>

| | **Liên kết động thông thường** | **`dlopen`/`dlsym`** |
|---|---|---|
| Khi nào nạp | **Lúc khởi động**, tự động | **Lúc chạy**, do bạn quyết định |
| Biết trước tên thư viện? | **Có** — nướng vào file thực thi lúc build | **Không** — chuỗi tính lúc chạy |
| Thiếu thư viện | Chương trình **không khởi động được** | `dlopen` trả `NULL` ⇒ **xử lý được**, chạy tiếp |
| Lấy hàm | Gọi trực tiếp | `dlsym` trả con trỏ, phải **tự ép kiểu** |

**Ba ứng dụng chính:**
1. ⭐ **Kiến trúc plugin** — chương trình quét một thư mục, nạp mọi `.so` tìm thấy. Đây là cách trình duyệt, editor, và nhiều daemon cho phép mở rộng **mà không cần build lại**.
2. **Tính năng tuỳ chọn** — chỉ nạp thư viện nặng khi người dùng thực sự dùng tính năng đó ⇒ khởi động nhanh, RAM ít.
3. **Chọn cài đặt lúc chạy** — nạp bản tối ưu theo phần cứng phát hiện được, hoặc theo cấu hình.

**⚠️ Bẫy:**
1. **Mất kiểm tra kiểu hoàn toàn.** `dlsym` trả `void*`; ép sai chữ ký thì **compiler không biết gì**, hỏng lúc chạy. Chuẩn thực dụng: chỉ phơi **C API** qua `dlopen`, và tốt nhất là **một hàm duy nhất** trả về một struct con trỏ hàm có **version**.
2. **Phải kiểm lỗi ở cả hai bước** — `dlopen` và `dlsym` đều có thể thất bại; và cơ chế báo lỗi cần đọc ngay sau khi gọi.
3. **Vòng đời:** đóng thư viện trong khi vẫn còn con trỏ trỏ vào mã của nó ⇒ crash. Plugin đã đăng ký callback thì gần như **không bao giờ nên đóng**.
4. **Symbol đụng nhau** giữa các plugin nạp cùng lúc, và biến toàn cục/singleton trong plugin có thể **không dùng chung** như bạn tưởng.
5. **C++ qua `dlopen`** rất phiền: tên đã mangle ⇒ hầu như luôn phải có một hàm `extern "C"` làm điểm vào factory.

**Chốt:** *"`dlopen`/`dlsym` nạp thư viện theo tên tính lúc chạy — nền của kiến trúc plugin. Đổi lại mất toàn bộ kiểm tra kiểu, nên điểm vào phải là một hàm `extern \\"C\\"` trả về bảng con trỏ hàm có version."*
</details>

#### SD-030 · 🟡 · concept · 📦 2026-08-13 · [→ static-vs-shared](../../../07-shared-libraries/static-vs-shared.md)
**Vì sao shared library tiết kiệm bộ nhớ khi nhiều chương trình cùng dùng? `-fPIC` liên quan thế nào?**
<details><summary>Đáp án</summary>

**Cơ chế tiết kiệm: chia sẻ ở mức KHUNG TRANG VẬT LÝ.** Kernel `mmap` phần **mã** (`.text`) của `.so` vào không gian địa chỉ của từng tiến trình, nhưng **cùng trỏ về một khung trang vật lý** ⇒ 50 tiến trình dùng libc chỉ tốn **một** bản mã trong RAM.

```
   tiến trình A ─┐
   tiến trình B ─┼──► CÙNG khung trang vật lý chứa .text của libc  (chỉ 1 bản trong RAM)
   tiến trình C ─┘
```

⚠️ **Chỉ phần chỉ-đọc mới chia sẻ được.** Phần **dữ liệu** (biến toàn cục, GOT) phải **riêng mỗi tiến trình** ⇒ dùng copy-on-write. Nên tiết kiệm chủ yếu đến từ `.text`.

**Điều kiện để chia sẻ được — và đây là chỗ `-fPIC` vào cuộc:**

Mã chỉ dùng chung được nếu nó **không cần sửa** sau khi nạp. Nhưng thư viện được nạp ở **địa chỉ khác nhau** trong mỗi tiến trình (ASLR, và nó không thể độc chiếm một vùng cố định) ⇒ nếu mã chứa **địa chỉ tuyệt đối**, loader phải **vá lại mã** cho từng tiến trình ⇒ trang bị ghi ⇒ **copy-on-write ⇒ hết chia sẻ**.

**`-fPIC` (Position Independent Code)** sinh mã **không chứa địa chỉ tuyệt đối**: mọi truy cập toàn cục đi qua **GOT**, mọi lời gọi ngoài đi qua **PLT** ([SD-027](system-design.md)). Loader chỉ điền **bảng dữ liệu** (riêng mỗi tiến trình), **mã giữ nguyên** ⇒ chia sẻ được.

**Cái giá:** thêm một lần gián tiếp cho truy cập toàn cục và lời gọi ngoài, và tốn một thanh ghi làm con trỏ bảng trên vài kiến trúc. Nhỏ, nhưng không bằng 0 — đây là một lý do firmware nhúng đơn chương trình hay chọn **liên kết tĩnh**.

**Bẫy:** quên `-fPIC` khi build thư viện ⇒ trên x86-64 thường **lỗi ngay lúc link** (`relocation R_X86_64_32 against ... can not be used when making a shared object`) — thông báo khó hiểu nhưng nghĩa đơn giản: *"mã này chứa địa chỉ tuyệt đối, không dùng chung được"*.

**Chốt:** *"Nhiều tiến trình map chung một khung trang vật lý chứa `.text` — nhưng chỉ khi mã không cần vá lại, tức phải `-fPIC`. Thiếu nó thì loader phải sửa mã cho từng tiến trình và lợi ích chia sẻ biến mất."*
</details>

#### SD-031 · 🟡 · design · 📦 2026-08-13 · [→ static-vs-shared](../../../07-shared-libraries/static-vs-shared.md)
**Khi nào nên chọn static, khi nào chọn shared? Đặt vào bối cảnh một thiết bị nhúng.**
<details><summary>Đáp án</summary>

**Chọn theo câu hỏi chi phối, không theo thói quen:**

| Câu hỏi | Trả lời "có" ⇒ chọn |
|---|---|
| Nhiều chương trình trên máy dùng chung thư viện này? | **Shared** — tiết kiệm RAM thật ([SD-030](system-design.md)) |
| Cần vá lỗi bảo mật của thư viện mà **không** phát hành lại ứng dụng? | **Shared** |
| Thư viện là **sản phẩm giao cho khách** dùng chung? | **Shared** (và phải lo ABI — [SD-020](system-design.md)) |
| Cần triển khai **một file duy nhất**, chắc chắn chạy ở mọi máy? | **Static** |
| Cần **tất định tuyệt đối** — biết chính xác mã nào đang chạy? | **Static** |
| Ứng dụng **duy nhất** trên thiết bị? | **Static** — lợi ích chia sẻ bằng 0 |

**Trên thiết bị nhúng — cân nhắc riêng:**
- Hệ chạy **một ứng dụng chính** (rất phổ biến) ⇒ shared **không tiết kiệm gì** mà còn thêm rủi ro thiếu thư viện, thêm thời gian khởi động, thêm phức tạp khi đóng gói ⇒ **static thường hợp hơn**.
- Hệ có **nhiều tiến trình dùng chung** thư viện lớn (Qt, OpenSSL) ⇒ shared tiết kiệm RAM đáng kể — và RAM là tài nguyên khan hiếm nhất.
- **Cập nhật OTA:** static ⇒ mỗi lần vá là **gói cập nhật lớn hơn** (phải gửi cả ứng dụng). Shared ⇒ gói nhỏ nhưng phải quản lý phiên bản chặt. Đây thường là yếu tố quyết định thực tế khi băng thông hạn chế.
- **Giấy phép:** một số giấy phép (LGPL) ràng buộc khác nhau giữa liên kết tĩnh và động — cần kiểm trước, không phải chuyện kỹ thuật thuần.

**Lựa chọn lai hay dùng:** **static** cho thư viện nội bộ của dự án (ổn định, kiểm soát được, tránh hẳn vấn đề ABI nội bộ) + **shared** cho thư viện hệ thống lớn dùng chung (libc, OpenSSL).

**Bẫy:** (1) chọn shared "cho chuyên nghiệp" trên hệ chỉ có một ứng dụng ⇒ nhận toàn nhược điểm, không được ưu điểm nào; (2) quên rằng static **đóng băng** lỗ hổng bảo mật vào ứng dụng ⇒ phải có quy trình rà soát và phát hành lại; (3) trộn hai kiểu cho **cùng một** thư viện trong một tiến trình ⇒ hai bản, hai bộ biến toàn cục, hành vi rất khó hiểu.

**Chốt:** *"Shared khi nhiều tiến trình dùng chung hoặc cần vá độc lập; static khi một ứng dụng duy nhất, cần một file và tính tất định. Trên nhúng một-ứng-dụng thì static thường đúng — nhưng OTA và giấy phép mới hay là yếu tố quyết định thật."*
</details>

---

⬅️ [Bank index](README.md)
