# 🎯 Design Patterns *trong việc của bạn* — bản đồ chọn lọc

> **Bốn file gốc của topic 11 trả lời "pattern X là gì".** Thư mục này trả lời câu khác, khó hơn và đúng thứ phỏng vấn đo:
> **"Trong hệ display bạn đang làm, quyết định thiết kế nào đẻ ra pattern nào — và chỗ nào bạn cố tình KHÔNG dùng pattern?"**
>
> Không thêm pattern mới cho nhiều. Mục tiêu ngược lại: **cắt bớt**, giữ lại số ít nhưng dựng lại được vấn đề gốc.

---

## Vì sao có thư mục này

Bạn tự đặt vấn đề đúng: *"không tham lam học hết, một vài cái nhưng hiểu rõ bản chất quan trọng hơn số lượng"*. Đây là cách thi hành điều đó.

Hai lý do cụ thể, không phải khẩu hiệu:

1. **Pattern học rời khỏi bối cảnh thì không truy xuất được lúc bị hỏi.** Repo đã đo đúng hiện tượng này hai lần: *T1 3.67 / T2 2.1* — biết định nghĩa nhưng không chuyển thành quyết định ([datalogic-plan §📍](../../14-prep/study-plans/datalogic-plan.md)). Pattern là mảng kiến thức **dễ mắc bệnh này nhất**, vì sách dạy toàn ví dụ `Shape`/`Animal` không dính gì tới công việc.
2. **Nói ra một tên pattern là mời interviewer hỏi vào đúng nó** (bài học #4 của plan, phiên 19/08). Nói *"em dùng Builder"* trong khi cái đó là Factory ⟹ mất điểm nhiều hơn là không nói gì. Thư mục này gọi đúng tên từng thứ trong code bạn đã viết.

---

## 🗺️ Bức tranh — ba tầng đầu tư

```mermaid
flowchart LR
    A["<b>TẦNG 1 — SỞ HỮU</b><br/>5 pattern<br/><i>có mặt trong code/resume<br/>⇒ interviewer CHẮC CHẮN đào</i>"]
    B["<b>TẦNG 2 — NÓI ĐƯỢC MỘT ĐOẠN</b><br/>4 pattern<br/><i>có mặt trong việc<br/>nhưng không phải trục chính</i>"]
    C["<b>TẦNG 3 — CẮT</b><br/>phần còn lại<br/><i>nhận ra tên là đủ,<br/>không đầu tư</i>"]
    A --> B --> C
```

### Tầng 1 — SỞ HỮU (5 pattern)

Tiêu chí vào tầng này: **đã nằm trong code bạn viết hoặc trong một dòng resume**. Interviewer đọc resume xong sẽ đi thẳng vào đây.

| Pattern | Ở đâu trong việc của bạn | Neo trên resume / code | Tài liệu |
|---|---|---|---|
| **Abstract Factory** *(+ Factory Method)* | Chọn cả **họ** implementation theo chip / dòng sản phẩm lúc boot | *"the library selects its implementation at boot from board configuration"* | [B1 §4](B1-redesign-architecture.md) |
| **Strategy** | Dimming **Global / Local / OLED** — cùng hợp đồng, khác thuật toán | *"display enhancement (dimming...)"* | [A1 §5.4](A1-baseline-libdisplay.md) · [behavioral §1](../behavioral.md) |
| **Bridge** | Hai trục biến thiên độc lập: *thuật toán dimming* × *chip* — chống bùng nổ lớp con | *"a single C++ interface across chipsets"* | [A1 §5.3](A1-baseline-libdisplay.md) · [structural §1](../structural.md) |
| **Singleton** | `IDisplay::getInstance()` trong HAL bạn viết | `interface/IDisplay.cpp` | [A2 §3.2](A2-cpp-interface-hal.md) |
| **Observer** | Ambient light sensor → đổi độ sáng | *"adaptive brightness control driven by an ambient light sensor"* | [B2 §1](B2-redesign-events.md) |

> 🔴 **Singleton đang là câu YẾU, không phải câu mạnh.** `DP-002` trong [weak-register](../../14-prep/mock-interview/weak-register.md) là **regression 4 → 1** (ngày 21/08, góc *"Singleton trong `.so` — còn một instance không?"*). Nó nằm ở tầng 1 vì bạn **dùng nó hằng ngày**, chứ không vì bạn đã vững.

### Tầng 2 — NÓI ĐƯỢC MỘT ĐOẠN (7 pattern)

Có mặt trong việc, nhưng không phải trục chính. Mục tiêu: **một đoạn 3–4 câu nêu đúng vấn đề nó giải**, không cần code thuộc lòng.

| Pattern | Ở đâu | Tài liệu |
|---|---|---|
| **Null Object** | `new IDisplay()` dummy trả `-ENOTSUP` khi `.so` không nạp được — **chính code bạn viết** | [A2 §2](A2-cpp-interface-hal.md) · [behavioral §7](../behavioral.md) |
| **Template Method** | Khung dimming chung, mỗi loại chỉ thay vài bước — *lựa chọn thay thế cho Strategy* | [behavioral §2.1](../behavioral.md) |
| **Command** | Message qua POSIX mq đồng bộ brightness giữa các S-Box | [B2 §2](B2-redesign-events.md) · [behavioral §5](../behavioral.md) |
| **State** | Hysteresis chống nhấp nháy · luồng bring-up · giao thức — bản **enum + bảng**, không phải OOP | [B2 §1.2](B2-redesign-events.md) · [behavioral §4](../behavioral.md) |
| **Adapter** | Dịch API vendor/SoC sang interface thống nhất của library — ngăn chữ ký lạ lan ra | [structural §2](../structural.md) |
| **Facade** | HAL gói *"mở clock → cấu hình pin → ghi thanh ghi → chờ ready"* thành một `enable()` | [structural §3](../structural.md) |
| **Memento** | Feature **Preset** của Samsung Display Manager (lưu / khôi phục / export cấu hình) | [B2 §3](B2-redesign-events.md) · [behavioral §6](../behavioral.md) |

> 💡 **Adapter + Facade đáng nói cùng nhau:** HAL thường **là cả hai** — Adapter ở chỗ *dịch interface không khớp*, Facade ở chỗ *che một trình tự nhiều bước*. Nêu được cả hai vai là một câu trả lời mạnh cho JD embedded.

### Tầng 3 — CẮT (nhận ra tên là đủ)

**Builder (GoF thật) · Decorator · Proxy · Visitor · Prototype · Flyweight · Chain of Responsibility · Mediator · Interpreter.**

Lý do cắt — nói thẳng để lần sau không ai lặng lẽ kéo ngược lại:

- **Không có trong việc của bạn.** JD Embedded Linux / BSP hỏi *bring-up, driver, Yocto, debug*; phần C++ hỏi *RAII, ABI, concurrency*. Không có JD embedded nào hỏi Visitor.
- **Đầu tư vào chúng làm hại nhiều hơn lợi**: học nửa vời rồi lỡ miệng nhắc tên ⟹ mở đúng cánh cửa mình không đi qua được.
- Bốn file gốc đã giữ đúng mức *nhận diện* cho nhóm này: [creational §5](../creational.md) (Builder) và [structural §4–5](../structural.md) (Proxy, Decorator) — mỗi cái vài dòng + **một bảng phân biệt hình dạng vs mục đích**. **Đọc một lần, không luyện.**

> ⚠️ **Ngoại lệ có chủ đích — `Object Pool`** ([creational §4](../creational.md)): không nằm trong việc hằng ngày của bạn, nhưng là **câu hỏi embedded kinh điển** (cấp phát tất định, chống fragmentation). Giữ ở tầng 2 về mặt ôn tập dù không có trong code.

---

## Ba quy tắc rút ra từ chính hệ display

Đây là phần đáng mang vào phỏng vấn hơn cả danh sách pattern.

1. **Không phải chỗ nào cũng cần pattern.** `frc` và `tcon` chỉ là *một command cố định xuống chip* — trừu tượng hoá chúng là chi phí thuần tuý. Nói được **chỗ mình cố tình không dùng** là tín hiệu senior rõ hơn kể tên năm pattern. Chi tiết: [B1 §7.1](B1-redesign-architecture.md).
2. **Pattern trả lời "biến thiên ở đâu".** Trước khi chọn pattern, viết ra **trục biến thiên**: *cái gì thay đổi, cái gì đứng yên*. Dimming biến thiên theo **thuật toán**; video-enhancer biến thiên theo **SoC**. Hai trục khác nhau ⟹ hai pattern khác nhau, dù nhìn qua đều là "kế thừa từ một class gốc".
3. **Qua ranh giới `.so`, pattern nào cũng phải trả thêm giá ABI.** Virtual interface là **hợp đồng nhị phân**, không chỉ hợp đồng biên dịch. Đã kiểm chứng thật: chèn một virtual vào giữa `IDisplay` ⟹ app gọi hàm này nhưng **destructor chạy**, không crash, exit 0 — **kể cả khi version check của hai bên khớp nhau**. Xem [A2 §3.3](A2-cpp-interface-hal.md).

> 🧪 **Bộ lab của thư mục này:** [A2 §7](A2-cpp-interface-hal.md) có **5 bài ngồi máy** (Null Object · data race trong Singleton · `-rdynamic` · hai bài vtable ABI) chạy trên **pack code hoàn chỉnh ở [A2 §6](A2-cpp-interface-hal.md)** — userspace thuần, không cần root, không cần phần cứng. Mọi output trong đó là **output thật đã chạy**, dán nguyên văn để đối chiếu.

---

## 🅰️🅱️ Hai phần — đọc đúng phần

> **Vì sao tách:** phỏng vấn hỏi **hai** câu khác nhau — *"kể kiến trúc bạn làm"* và *"nếu làm lại thì đổi gì"*. Trộn hai thứ đó vào một tài liệu thì cả hai đều yếu: mô tả hệ thật bị pha lý tưởng hoá, còn đề xuất cải tiến thì mất chỗ đứng.

| | 🅰️ **PHẦN A — TIÊU CHUẨN** | 🅱️ **PHẦN B — CẢI TIẾN** |
|---|---|---|
| Trả lời câu | *"Kiến trúc bạn làm là gì?"* | *"Làm lại thì đổi gì?"* |
| Nội dung | Hệ **thật đang chạy**, đã khử nhạy cảm — kèm **cả điểm yếu** | Bản vá **đúng 5 điểm yếu** đó |
| Rủi ro nếu kể sai | Bịa ra thứ không có ⟹ bị vặn là lộ | Đổi hết mọi thứ ⟹ tín hiệu thiếu phán đoán |

| # | File | Nội dung |
|---|------|----------|
| 🅰️ | [A1-baseline-libdisplay.md](A1-baseline-libdisplay.md) | **Bắt đầu ở đây.** Kiến trúc `libdisplay`: narrow waist hai tầng · 7 pattern có thật · case study **dimming = Bridge** · **5 điểm yếu tự nhận** |
| 🅰️ | [A2-cpp-interface-hal.md](A2-cpp-interface-hal.md) | **Tầng 0** của A1 — ranh giới C++ interface/impl: mổ từng mảnh, 3 rủi ro đã đo bằng máy, **pack code chạy được** + **5 bài lab 🧪** |
| 🅱️ | [B1-redesign-architecture.md](B1-redesign-architecture.md) | Vá 5 điểm yếu: ISP · trục thứ ba · Abstract Factory thật · hợp đồng lỗi · khoá đi cùng state. Kèm **thứ tự ưu tiên theo rủi ro** |
| 🅱️ | [B2-redesign-events.md](B2-redesign-events.md) | Observer + hysteresis · Command + `apply_at` · Memento/Preset |

**Một trục kiến trúc duy nhất nối cả bốn file:**

```
App
 └─ IDisplay / DisplayImpl      ← 🅰️ A2 · ranh giới KHÉP · C++
      └─ lib_api_*              ← chỗ hẹp · ranh giới MỞ · extern "C"
           └─ libdisplay nội bộ ← 🅰️ A1 (thật) · 🅱️ B1 (làm lại)
```

## 🔤 Bộ từ vựng chuẩn — dùng thống nhất trong cả mục

> ⚠️ Mọi định danh dưới đây là **tên tài liệu**, không phải tên thật của hệ. **Kiến trúc giữ nguyên 100%.** Tư liệu nội bộ còn tên thật bị `.gitignore` và **không được link tới từ tài liệu commit** — mọi file phần A/B phải **tự chứa**.

| Khái niệm | Tên chuẩn |
|---|---|
| Shared library | **`libdisplay`** |
| Mặt tiền C của library | **`lib_api_*`** (`lib_api.h`) |
| C++ interface Tầng 0 | **`IDisplay`** / **`DisplayImpl`** |
| Hợp đồng module trong library | `lib_dimming_interface` · `lib_sensor_interface` · `lib_video_enhancement_interface` · `lib_ambient_interface` |
| Implementation module | `lib_dimming` · `lib_sensor` · `lib_video_enhancement` · `lib_ambient` |
| Biến thể dòng sản phẩm | hậu tố `_signage` |
| Abstraction thuật toán dimming | **`IDimmingAlgo`** *(A)* → **`IDimmingCore` + nhóm khả năng** *(B)* |
| Implementor theo chip | **`IDimmingBackend`** → `DimmingBackendChipA` · `ChipB` |
| Factory | `DimmingFactory` *(A)* → **`IPlatformFactory`** *(B)* |
| Trạng thái đa process | `display_shm_info` · `*ForShm` · `lib_sem_lock` |
| Mã lỗi | `LIB_OK` *(A)* → **`-ENOTSUP`** *(B)* |

**Khử ở mức nội dung:** bỏ hẳn tên/phiên bản OS nền · số lượng chip làm tròn (*"hơn 10"*) · giữ `~150 virtual` (luận cứ ISP) · giữ loại sản phẩm chung (TV/Signage/Monitor).

## Liên kết ngoài topic

- ABI của virtual interface qua `.so`: [07/abi-versioning.md](../../07-shared-libraries/abi-versioning.md) · [07/linking-loading.md](../../07-shared-libraries/linking-loading.md)
- Thiết kế API thư viện, Pimpl: [07/api-design.md](../../07-shared-libraries/api-design.md)
- Chi phí virtual/heap trên hệ hạn chế: [08/constraints.md](../../08-embedded-systems/constraints.md)
- Câu hỏi: mục **E** trong [bank/design-patterns.md](../../14-prep/mock-interview/bank/design-patterns.md) (`DP-021` … `DP-030`)

---
⬅️ [Về index topic 11](../README.md) · ➡️ Bắt đầu: [B1-redesign-architecture.md](B1-redesign-architecture.md)
