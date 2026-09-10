# Behavioral — Ai làm gì, khi nào, và ai báo cho ai

> **TL;DR**
> - Behavioral pattern trả lời: *"trách nhiệm nằm ở đâu, và các mảnh nói chuyện với nhau kiểu gì?"*
> - **Strategy** (đổi thuật toán lúc chạy) và **Template Method** (giữ khung, thay vài bước) là **hai lời giải cho cùng một câu hỏi** — chọn theo *"có cần đổi trên object đang sống không?"*.
> - **Observer** đưa sự kiện tới đúng chỗ; **Command** biến một yêu cầu thành **dữ liệu** để xếp hàng/hoãn/undo; **State** cho object đổi hành vi theo trạng thái; **Memento** chụp state mà không phơi bên trong; **Null Object** biến *"không hỗ trợ"* thành một phần của hợp đồng.
> - ⭐ **Sợi chỉ đỏ của cả file: pattern mua CẤU TRÚC, không mua TÍNH ĐÚNG ĐẮN CỦA MIỀN.** Observer không chống nhấp nháy; Command không làm lệnh tới nơi; Memento không làm file đọc được ở máy khác. **Câu hỏi T2 luôn nằm ở nửa sau đó.**
> - Embedded: ưu tiên bản **nhẹ** (function pointer, enum + bảng) khi thứ đang làm nằm trên đường nóng.

---

## 0. Bản đồ: mỗi pattern cô lập một loại "biến thiên"

| Pattern | Cái biến thiên được cô lập | Câu hỏi nó trả lời |
|---|---|---|
| **Strategy** | Thuật toán | *"Cư xử thế nào?"* — đổi được lúc chạy |
| **Template Method** | Vài **bước** trong một khung cố định | *"Khung giống nhau, chi tiết khác nhau"* |
| **Observer** | **Ai** quan tâm tới sự kiện | *"Báo cho ai khi có thay đổi?"* |
| **State** | Hành vi theo **trạng thái nội bộ** | *"Cùng sự kiện, khác phản ứng"* |
| **Command** | **Thời điểm** thi hành | *"Lệnh này chạy khi nào, ở đâu?"* |
| **Memento** | **Lát cắt trạng thái** tại một thời điểm | *"Quay lại được không?"* |
| **Null Object** | Sự **vắng mặt** của implementation | *"Không có thì cư xử ra sao?"* |

---

## 1. Strategy

| | |
|---|---|
| **Bản chất** | Đóng gói mỗi thuật toán sau một interface chung để **chọn/đổi lúc runtime** mà không sửa code dùng |
| **Cô lập** | Thuật toán, tách khỏi context |
| **Nguyên lý** | OCP + DIP |
| **Cái giá** | Một object phụ + một lần gián tiếp |
| **Đừng dùng khi** | Chỉ có một thuật toán, hoặc loại được quyết định lúc boot và không đổi nữa (⟹ [Template Method](#2--template-method)) |

```cpp
struct IDimming {
    virtual ~IDimming() = default;
    virtual void onFrame(const FrameStats&) = 0;
};
class GlobalDimming : public IDimming { /* 1 gia tri cho ca panel */ };
class LocalDimming  : public IDimming { /* N zone theo histogram  */ };

class DisplayImpl {                       // CONTEXT — khong doi khi them thuat toan
    std::unique_ptr<IDimming> dimming_;
public:
    void onFrame(const FrameStats& s) { dimming_->onFrame(s); }        // uy nhiem
    void setDimming(std::unique_ptr<IDimming> d) { dimming_ = std::move(d); }
};
```

⭐ **Điểm cốt lõi hay bị bỏ sót:** giá trị của Strategy **không** nằm ở chỗ "có nhiều class kế thừa", mà ở chỗ **context không đổi một dòng nào** khi thêm thuật toán thứ tư. Hành vi được **cắm vào** (composition), không **nướng vào** (inheritance).

**C++ hiện đại:** strategy chỉ là *một hàm* ⟹ `std::function` + lambda gọn hơn cả cây class ([lambdas-functional](../02-modern-cpp/lambdas-functional.md)); cần phân giải lúc biên dịch ⟹ template parameter (không tốn virtual call).

> ⚠️ **Nhưng biết chỗ KHÔNG dùng lambda cũng là một điểm:** dimming **có state** (bộ lọc, lịch sử zone, timer chống nhấp nháy) ⟹ class là đúng. `std::function` hợp với strategy **thuần hàm**.

---

## 2. Template Method

| | |
|---|---|
| **Bản chất** | Lớp cha giữ **khung thuật toán**, lớp con chỉ thay vài **bước hook** |
| **Cô lập** | Các bước biến thiên, trong khi **thứ tự** các bước là bất biến |
| **Cái giá** | Ràng buộc kế thừa; hành vi cố định từ lúc tạo object |
| **Đừng dùng khi** | Các biến thể khác nhau về **cả khung**, hoặc cần đổi lúc runtime |

```cpp
class DimmingBase {
public:
    void onFrame(const FrameStats& s) {     // <- KHUNG: khong virtual, khong doi
        auto duty = compute(s);             //    hook: moi loai tu tinh
        clamp(duty);                        //    buoc chung
        writeToHw(duty);                    //    buoc chung
    }
    virtual ~DimmingBase() = default;
protected:
    virtual Duty compute(const FrameStats&) = 0;   // CHO DUY NHAT lop con thay
};
```

### 2.1 Chọn Strategy hay Template Method

| | **Template Method** | **Strategy** |
|---|---|---|
| Quan hệ | Kế thừa — lớp con *là* biến thể | Composition — context *giữ* strategy |
| Cái cố định | **Khung** (thứ tự các bước) | Chỉ interface |
| Đổi lúc runtime | ❌ | ✅ |
| Chi phí | Rẻ hơn | Thêm một object |
| Rủi ro riêng | Dễ vi phạm **LSP** nếu lớp con phá kỳ vọng của khung | Nhiều mảnh phải ghép |

> **Tiêu chí một câu:** *"Có bao giờ cần đổi hành vi trên một object ĐANG SỐNG không?"* — Có ⟹ Strategy. Không, và các biến thể chung khung ⟹ Template Method.

⚠️ **Bẫy của Template Method:** lớp cha gọi xuống lớp con (**inversion of control**), nên lớp con dễ phá giả định của khung mà compiler không phát hiện được. Nếu hook bắt đầu cần *"gọi hoặc không gọi bước tiếp theo"* thì khung đã sai — chuyển sang Strategy.

---

## 3. Observer

| | |
|---|---|
| **Bản chất** | Subject giữ danh sách observer đã đăng ký; đổi trạng thái thì **thông báo tất cả** |
| **Cô lập** | *Ai* quan tâm — subject không cần biết |
| **Cái giá** | **Vòng đời** (dangling) · thứ tự thông báo không xác định · dễ tạo chuỗi sự kiện khó lần |
| **Đừng dùng khi** | Chỉ có đúng một người nghe và sẽ mãi như vậy ⟹ callback thẳng |

```cpp
class LightSensor {                                   // subject — khong biet ai nghe
    std::vector<std::weak_ptr<ILightObserver>> obs_;  // weak: khong giu vong doi
public:
    void subscribe(std::shared_ptr<ILightObserver> o) { obs_.push_back(std::move(o)); }
    void publish(int lux) {
        for (auto it = obs_.begin(); it != obs_.end(); ) {
            if (auto p = it->lock()) { p->onLux(lux); ++it; }
            else                     { it = obs_.erase(it); }   // tu don observer da chet
        }
    }
};
```

🔴 **Rủi ro số một — vòng đời.** Observer bị huỷ mà quên `unsubscribe` ⟹ subject gọi vào con trỏ chết ⟹ UB. `weak_ptr` biến *"quên gỡ đăng ký"* từ **crash** thành **tự dọn**.

⚠️ **Ba bẫy nữa, ít người nêu nhưng rất thật trong embedded:**
1. **Đăng ký/huỷ TRONG lúc đang notify** ⟹ iterator invalidation. Cách chữa: notify trên bản sao danh sách, hoặc hoãn thay đổi tới cuối vòng lặp.
2. **Notify từ ngữ cảnh ISR** ⟹ observer chạy trong ngắt. Đúng cách: ISR chỉ đẩy sự kiện vào queue, luồng khác notify.
3. **Thứ tự thông báo là chi tiết cài đặt** — code không được phụ thuộc vào nó.

### 3.1 ⭐ Chỗ Observer KHÔNG giúp được

Nối thẳng `onLux → setBrightness` là **đúng pattern nhưng sai sản phẩm**: ánh sáng dao động liên tục ⟹ độ sáng đuổi theo từng mẫu ⟹ **nhấp nháy**. Đó là **vấn đề miền**, phải giải bằng đường ống sau Observer: **lọc** (EMA) → **hysteresis hai ngưỡng** → **giới hạn tốc độ** (ramp) → **chống lặp**. Chi tiết + cách kể ở phỏng vấn: [in-practice/B2 §1.2](in-practice/B2-redesign-events.md).

---

## 4. State

| | |
|---|---|
| **Bản chất** | Object đổi **hành vi** khi trạng thái nội bộ đổi — như thể đổi class |
| **Cô lập** | Logic của từng trạng thái + các phép chuyển hợp lệ |
| **Vì sao quan trọng** | Là cách hiện thực **state machine** — cốt lõi firmware, giao thức, luồng bring-up |
| **Đừng dùng bản OOP khi** | Mỗi state chỉ vài dòng ⟹ enum + `switch`/bảng gọn hơn nhiều |

```cpp
// Ban nhe (hop embedded): tat dinh, khong heap, khong virtual
enum class State { Idle, Running, Error };
class Machine {
    State state_ = State::Idle;
public:
    void event(Event e) {
        switch (state_) {
        case State::Idle:
            if (e == Event::Start) state_ = State::Running;
            break;
        case State::Running:
            if      (e == Event::Fault) state_ = State::Error;
            else if (e == Event::Stop)  state_ = State::Idle;
            break;
        case State::Error:
            if (e == Event::Reset) state_ = State::Idle;
            break;
        }
    }
};
```

| | enum + switch/bảng | State pattern OOP |
|---|---|---|
| Chi phí | Không heap, không virtual | Object + virtual mỗi state |
| Đọc toàn cục | ✅ Thấy **cả** bảng chuyển trong một chỗ | ❌ Rải ra nhiều file |
| Khi logic mỗi state phức tạp | Hàm `event()` phình to | ✅ Mỗi state một class gọn |
| Thêm state mới | Sửa `switch` (OCP kém) | Thêm class |

> **Chốt:** *Embedded ưu tiên enum + bảng vì **tất định** và vì **nhìn thấy toàn bộ máy trạng thái trong một màn hình** — thứ quý nhất khi review firmware. Chuyển sang bản OOP khi logic mỗi state đủ lớn để `switch` không đọc nổi.*

⭐ Ví dụ nhỏ nhưng thật: **hysteresis** ở §3.1 chính là một máy trạng thái hai trạng thái — và ở đó enum là đúng, không cần class.

---

## 5. Command

| | |
|---|---|
| **Bản chất** | Đóng gói một yêu cầu (**hành động + tham số**) thành **object/dữ liệu** |
| **Cô lập** | **Thời điểm và nơi chốn** thi hành, tách khỏi nơi phát lệnh |
| **Mở ra** | Xếp hàng · gửi qua IPC · **hoãn** · undo/redo · log & replay · macro |
| **Đừng dùng khi** | Lệnh chạy ngay, tại chỗ, không cần lưu ⟹ gọi hàm là đủ |

```cpp
// Command "da tuan tu hoa" — cai di tren message queue. POD, khong con tro, khong vtable.
struct BrightnessCmd {
    uint16_t version;      // hop dong nhi phan — BAT BUOC
    uint16_t opcode;
    uint32_t seq;          // chong lap + phat hien mat goi
    int32_t  value;
    uint64_t apply_at_ms;  // <- thi hanh tai moc T, khong phai "ngay khi nhan"
};
```

⭐ **`apply_at_ms` là chi tiết đắt giá nhất.** Với video wall nhiều panel, mỗi unit nhận message ở thời điểm khác nhau; "đổi ngay khi nhận" ⟹ các panel đổi lệch nhau thành lưới. Ra lệnh *"đổi tại mốc T"* mới cho mặt phẳng đồng nhất — đây đúng là lý do gốc Command tồn tại: **tách thời điểm phát khỏi thời điểm chạy**.

⚠️ **Command qua ranh giới process là ABI.** Struct đó phải có `version`, và **chỉ được thêm field vào cuối** — cùng luật với vtable qua `.so`. Chi tiết: [in-practice/B2 §2](in-practice/B2-redesign-events.md).

---

## 6. Memento

| | |
|---|---|
| **Bản chất** | Chụp trạng thái nội bộ để khôi phục sau, **mà không phơi cấu trúc bên trong** |
| **Ba vai** | **Originator** (tự chụp/tự nạp) · **Memento** (bản chụp) · **Caretaker** (giữ, **không đọc**) |
| **Cái giá** | Bộ nhớ cho bản chụp; phải quyết định chụp *sâu* hay *nông* |
| **Đừng dùng khi** | State nhỏ và công khai ⟹ copy thẳng struct là đủ |

**Điểm hay bị bỏ sót:** giá trị nằm ở chữ *"không phơi bên trong"*. Nếu UI tự đọc từng field rồi tự set lại thì mỗi setting mới phải sửa UI — encapsulation vỡ. Với Memento, thêm state mới chỉ sửa **originator**.

⚠️ **Memento cổ điển sống trong MỘT process, MỘT phiên chạy** (undo/redo). Muốn nó qua restart hoặc sang máy khác (feature Preset) thì phải thêm ba thứ pattern không nói tới: **versioning** · **capability của thiết bị đích** · **chính sách lỗi khi áp dụng dở dang**. Xem [in-practice/B2 §3](in-practice/B2-redesign-events.md).

---

## 7. Null Object

| | |
|---|---|
| **Bản chất** | Trả về một object **hợp lệ nhưng không làm gì**, báo trạng thái bằng mã lỗi, thay vì `nullptr` |
| **Cô lập** | Sự **vắng mặt** của implementation — biến nó thành một nhánh hợp lệ của hợp đồng |
| **Nguyên lý** | LSP: *"không hỗ trợ"* là hành vi **được khai báo**, không phải im lặng bất ngờ |
| **Đừng dùng khi** | Bỏ qua âm thầm là nguy hiểm ⟹ *fail loud* mới đúng |

```cpp
// Base la Null Object: moi API co cai dat mac dinh co nghia
int IDisplay::setPower(bool)     { return -ENOTSUP; }
int IDisplay::setBrightness(int) { return -ENOTSUP; }
```

| | Trả `nullptr` | **Null Object** |
|---|---|---|
| Caller | Kiểm ở **mọi** lời gọi | Gọi thẳng |
| Quên kiểm | **Segfault** | Mã lỗi, log được |
| Phân biệt "không hỗ trợ" vs "lỗi thật" | Không | Có |

⚠️ **Nó đổi an toàn lấy độ hiện.** Lỗi không nổ ngay tại chỗ mà **im lặng trôi đi** — nên điều kiện dùng được là **mã trả về phải được kiểm và log**; không thì Null Object chỉ đang giấu bug. Và với lệnh phải chắc chắn tới nơi (ghi cấu hình an toàn, điều khiển phần cứng quan trọng), *fail loud* đúng hơn.

> 🔴 **Một hiểu lầm phải sửa cho đúng:** virtual-không-pure + Null Object cho **tương thích MÃ NGUỒN** (impl không phải sửa code khi interface thêm API), **không** cho tương thích **NHỊ PHÂN** — `.so` đã biên dịch từ trước có vtable **ngắn hơn**, gọi API mới vào đó là **segfault**, không phải `-ENOTSUP`. Đã đo thật: [in-practice/A2 §2.1 + Lab 3b](in-practice/A2-cpp-interface-hal.md).

---

## 8. Khi nào KHÔNG dùng pattern

- **Vấn đề đơn giản** ⟹ một hàm/lambda là đủ; nhồi pattern làm code khó hơn.
- **C++ hiện đại thay được nhiều pattern bằng tính năng ngôn ngữ:** `std::function`/lambda (Strategy, Command đơn giản) · template (Strategy compile-time) · `std::variant` + `std::visit` (một số State/Visitor).
- **Embedded:** cân nhắc chi phí virtual/heap — nhưng nhớ đúng ranh giới: chi phí nằm ở **tần suất gọi**. Virtual trên đường cấu hình là miễn phí; **cấp phát động trên đường mỗi khung hình là lỗi thật**.
- **Ba câu hỏi lọc** trước khi trừu tượng hoá: đã có ≥ 2 biến thể **thật** chưa? Chúng khác **hành vi** hay chỉ khác **tham số**? Có ai cần **hoán đổi** không? ([solid-principles §7](solid-principles.md))

> Pattern là **từ vựng chung** để trao đổi thiết kế và là giải pháp đã kiểm chứng — nhưng chỉ áp khi vấn đề thật sự khớp. Và ở phỏng vấn: **nói tên một pattern là mời interviewer hỏi vào đúng nó**.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-005](../14-prep/mock-interview/bank/design-patterns.md) | Strategy pattern là gì? C++ hiện đại hiện thực gọn thế nào? |
| [DP-035](../14-prep/mock-interview/bank/design-patterns.md) | Template Method và Strategy — chọn cái nào, dựa trên tiêu chí gì? |
| [DP-021](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Dimming chia theo thuật toán, video-enhancer chia theo SoC — vì sao hai pattern khác nhau? |
| [DP-006](../14-prep/mock-interview/bank/design-patterns.md) | Observer pattern dùng khi nào? Rủi ro? |
| [DP-030](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Observer đúng sách mà màn hình vẫn nhấp nháy — pattern sai chỗ nào? |
| [DP-010](../14-prep/mock-interview/bank/design-patterns.md) | State pattern là gì? Vì sao embedded hay dùng enum + switch? |
| [DP-034](../14-prep/mock-interview/bank/design-patterns.md) | Command pattern là gì? Mở ra khả năng nào mà gọi hàm trực tiếp không có? |
| [DP-031](../14-prep/mock-interview/bank/design-patterns.md) | Message trên mq là Command — cần gì để nhiều panel đổi *cùng lúc*? |
| [DP-039](../14-prep/mock-interview/bank/design-patterns.md) | Memento là gì? Điểm cốt lõi nhiều người bỏ sót? |
| [DP-036](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Null Object là gì? Đổi lấy điều gì, và khi nào KHÔNG nên dùng? |
| [DP-028](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Vì sao API của `IDisplay` là virtual thường chứ không pure virtual? |

---
⬅️ [structural.md](structural.md) · ➡️ Tiếp theo: 🎯 [in-practice/](in-practice/) — áp vào việc thật
