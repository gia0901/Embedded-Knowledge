# B2 — Sự kiện & trạng thái, thiết kế lại: Observer · Command · Memento

> 🅱️ **PHẦN B — CẢI TIẾN.** Cùng bối cảnh sản phẩm, nhưng đây là **thiết kế nên có**, không phải mô tả hệ đang chạy. Kiến trúc thật ở [🅰️ A1](A1-baseline-libdisplay.md); phần vá 5 điểm yếu ở [🅱️ B1](B1-redesign-architecture.md).

> **TL;DR**
> - Ba pattern **tầng 2** ([bản đồ](README.md)), mỗi cái neo vào một dòng resume: **Observer** ↔ adaptive brightness · **Command** ↔ đồng bộ brightness qua POSIX mq · **Memento** ↔ feature Preset.
> - ⭐ **Bài học xuyên suốt cả ba: pattern giải quyết vấn đề *cấu trúc*, không giải quyết vấn đề *miền*.** Observer đưa sự kiện tới đúng chỗ — nhưng **không** chống nhấp nháy; Command đóng gói yêu cầu — nhưng **không** làm nó tới nơi; Memento chụp trạng thái — nhưng **không** làm nó đọc được ở máy khác.
> - Chỗ khác biệt giữa mid và senior nằm đúng ở nửa sau của mỗi câu trên.
> - Mục tiêu tầng 2: **nói được một đoạn 3–4 câu nêu đúng vấn đề pattern giải**, không cần thuộc code.

---

## 1. Observer — ambient light sensor → độ sáng

> Neo resume: *"Built adaptive brightness control driven by an ambient light sensor, allowing screen brightness to adjust automatically... **without flickering when lighting changes quickly**"*

### 1.1 Pattern làm được gì

Sensor (**subject**) không nên biết ai quan tâm tới ánh sáng môi trường. Hôm nay là module brightness; mai thêm module log, thêm module tiết kiệm điện. Observer cắt đúng phụ thuộc đó:

```cpp
class ILightObserver {
public:
    virtual ~ILightObserver() = default;
    virtual void onLux(int lux) = 0;
};

class LightSensor {                              // subject — không biết ai nghe
    std::vector<std::weak_ptr<ILightObserver>> obs_;   // weak: không giữ vòng đời observer
public:
    void subscribe(std::shared_ptr<ILightObserver> o) { obs_.push_back(std::move(o)); }
    void publish(int lux) {
        for (auto it = obs_.begin(); it != obs_.end(); ) {
            if (auto p = it->lock()) { p->onLux(lux); ++it; }
            else                     { it = obs_.erase(it); }   // tự dọn observer đã chết
        }
    }
};
```

**Vì sao `weak_ptr` chứ không phải con trỏ thô** — đây là rủi ro số một của Observer và là câu hỏi mặc định: observer bị huỷ mà quên `unsubscribe` ⟹ subject gọi vào con trỏ chết ⟹ UB. `weak_ptr` biến "quên gỡ đăng ký" từ **crash** thành **tự dọn**. ([behavioral §2](../behavioral.md), câu [DP-006](../../14-prep/mock-interview/bank/design-patterns.md))

### 1.2 ⭐ Chỗ Observer **không** giúp được: nhấp nháy

Nối thẳng `onLux → setBrightness` là **đúng pattern nhưng sai sản phẩm**. Ánh sáng môi trường dao động liên tục (người đi qua, mây, đèn chớp) ⟹ độ sáng đuổi theo từng mẫu ⟹ **nhấp nháy** — đúng thứ dòng resume nói là đã xử lý.

Nhấp nháy là **vấn đề miền**, phải giải bằng cơ chế miền, xếp thành một **đường ống** sau Observer:

| Tầng | Chống điều gì | Cơ chế |
|---|---|---|
| ① **Lọc** | Nhiễu mẫu đơn lẻ | Trung bình trượt / EMA trên N mẫu gần nhất |
| ② **Hysteresis** | Dao động quanh **ngưỡng** | Đổi mức chỉ khi vượt ngưỡng **một biên** (lên ở 120 lux, xuống ở 80 lux — không cùng một điểm) |
| ③ **Giới hạn tốc độ** | Nhảy bậc thấy được | Ép mỗi bước đổi ≤ X nits/giây (ramp), không nhảy thẳng |
| ④ **Chống lặp** | Ghi thừa xuống HW | Bỏ qua nếu giá trị đích không đổi |

> **Hysteresis là một máy trạng thái nhỏ** — và ở đây **enum + ngưỡng** là đủ, không cần State pattern OOP ([behavioral §3](../behavioral.md)). Nó nằm trên đường sự kiện tần suất cao ⟹ giữ tất định, không cấp phát.

✅ **Cách kể ở phỏng vấn** *(đoạn này ăn điểm vì nó cho thấy bạn phân biệt được cấu trúc với miền)*:

> *"Sensor phát sự kiện theo kiểu Observer nên tầng brightness không gắn cứng với driver sensor. Nhưng riêng Observer thì gây nhấp nháy — nối thẳng sự kiện vào setBrightness là đuổi theo từng mẫu. Nên sau Observer tôi đặt một đường ống: lọc trung bình trượt, rồi hysteresis hai ngưỡng để không dao động quanh điểm chuyển, rồi ramp giới hạn tốc độ đổi. Pattern lo phần ai-gọi-ai; chống nhấp nháy là phần thuật toán, pattern không làm hộ."*

---

## 2. Command — đồng bộ brightness giữa nhiều S-Box qua POSIX mq

> Neo resume: *"Synchronized brightness across multiple S-Box units over POSIX message queues... with a single binary serving both standalone and synchronized modes"*

### 2.1 Vì sao message **chính là** Command

Command pattern = **đóng gói một yêu cầu (hành động + tham số) thành một object** để lưu, xếp hàng, gửi đi, hoãn, hoặc undo. Một message trong queue là hiện thân tự nhiên nhất của nó — chỉ khác là nó vượt qua **ranh giới process**, nên phải ở dạng **byte phẳng** chứ không phải object có vtable.

```cpp
// Command "đã tuần tự hoá" — cái đi trên mq. POD, không con trỏ, không vtable.
struct BrightnessCmd {
    uint16_t version;     // ⬅️ BẮT BUỘC — xem §2.2
    uint16_t opcode;      // SET_NITS / RAMP_TO / QUERY
    uint32_t seq;         // chống lặp & phát hiện mất gói
    int32_t  value;
    uint64_t apply_at_ms; // thời điểm áp dụng — không phải "ngay khi nhận"
};
```

⭐ **`apply_at_ms` là chi tiết đắt giá nhất ở đây.** Với video wall, mỗi unit nhận message ở thời điểm khác nhau; nếu ai nhận được thì áp dụng ngay, người xem thấy **các panel đổi lệch nhau** — đúng cái "visible grid" mà resume nói đã loại bỏ. Ra lệnh *"đổi tại mốc thời gian T"* thay vì *"đổi ngay"* mới cho ra một mặt phẳng đồng nhất. Đây là **Command hoãn thực thi** — một trong những lý do gốc pattern tồn tại.

### 2.2 ⚠️ Trường `version` — cùng bài học ABI của [A2 §3.3](A2-cpp-interface-hal.md), lặp lại ở tầng khác

`struct` đi qua ranh giới process là **một hợp đồng nhị phân**, hệt như vtable:

| Việc | Được? |
|---|---|
| Thêm field vào **cuối** + tăng `version`, bên nhận đọc theo `version` | ✅ |
| Chèn field vào **giữa** | ❌ Lệch offset — unit chưa update đọc rác |
| Đổi kiểu field (`int32` → `int64`) | ❌ |
| Dựa vào `sizeof(struct)` giống nhau hai bên | ❌ Padding/alignment có thể khác giữa build |

> **Mẫu lặp đáng nhớ — nói được là dấu hiệu hiểu hệ thống:** *"Cứ chỗ nào hai binary build riêng phải hiểu nhau — vtable qua `.so`, struct qua message queue, bảng con trỏ hàm giữa driver và HAL — thì layout là ABI, và luật luôn giống nhau: **chỉ được thêm vào cuối**."*

### 2.3 "Một binary, hai chế độ"

Dòng resume nói *một binary phục vụ cả standalone lẫn synchronized*. Đó **không** phải chỗ cho pattern nặng — là một nhánh cấu hình:

```cpp
// Chế độ standalone: "gửi" chỉ là thực thi tại chỗ.
// Chế độ synced:    gửi lên mq, mọi unit (kể cả mình) áp dụng tại apply_at_ms.
```

Đúng tinh thần [B1 §7.1](B1-redesign-architecture.md): **hai biến thể của một hành vi ⟹ đủ điều kiện cho một abstraction mỏng** (một `ICommandSink` với hai impl), **không** đủ điều kiện cho một tầng framework.

---

## 3. Memento — feature Preset (Samsung Display Manager)

> Neo resume: *"Built the 'Preset' feature — users can save a display configuration, restore it across restarts, and **export it to apply on another monitor**"*
> ⚠️ Kèm ghi chú thực tế: phần **lưu JSON đã có sẵn** trước khi bạn vào, **không nhận công** ([plan §📍, ghi chú 07/09](../../14-prep/study-plans/datalogic-plan.md) · câu [RES-022](../../14-prep/mock-interview/bank/resume.md)).

### 3.1 Pattern

**Memento** = chụp lại trạng thái nội bộ của một object để khôi phục sau, **mà không phơi cấu trúc bên trong ra ngoài**. Ba vai:

| Vai | Trong Preset |
|---|---|
| **Originator** | Đối tượng cấu hình màn hình — biết cách tự chụp và tự nạp lại |
| **Memento** | Bản chụp (đối tượng preset / JSON) — với bên ngoài là **khối mờ** |
| **Caretaker** | UI danh sách preset — lưu, đặt tên, xoá; **không đọc bên trong** |

**Điểm cốt lõi hay bị bỏ sót:** giá trị của Memento nằm ở chữ *"không phơi bên trong"*. Nếu UI tự đọc từng field rồi tự set lại, thì mỗi lần thêm một setting mới phải sửa UI — encapsulation vỡ. Với Memento, thêm setting mới chỉ sửa **originator**.

### 3.2 ⭐ Chỗ Preset **vượt ra ngoài** Memento sách vở

Memento kinh điển là **trong một process, trong một phiên chạy** (undo/redo). Preset của bạn phải làm hai việc mà GoF không nói tới:

| Yêu cầu | Memento cổ điển | Preset thật |
|---|---|---|
| Sống qua restart | Không cần | **Phải** — nên memento buộc phải tuần tự hoá được |
| Sang **máy/màn hình khác** | Không cần | **Phải** — và đây là chỗ khó thật |

**Export sang màn hình khác đẻ ra ba câu hỏi mà pattern không trả lời** — trả lời được là nội dung phỏng vấn tốt:

1. **Versioning.** Preset lưu bởi app 1.0, mở bằng app 1.2 — field mới thiếu thì sao? *(Trả lời: đánh version, thiếu thì lấy mặc định — **cùng luật "thêm vào cuối"** của §2.2 và [A2 §3.3](A2-cpp-interface-hal.md).)*
2. **Khả năng của thiết bị.** Màn hình đích **không hỗ trợ** một setting, hoặc có dải giá trị khác. *(Trả lời: áp dụng từng phần + báo cáo cái bỏ qua — chính là ý **`-ENOTSUP`** của [Null Object](A2-cpp-interface-hal.md) ở tầng khác.)*
3. **Tính nguyên tử.** Áp 10 setting, cái thứ 7 hỏng — dừng hay đi tiếp? *(Không có đáp án đúng duy nhất; có **quyết định rõ ràng và nhất quán** mới là câu trả lời.)*

> 💡 **Bài nói bấm giờ:** ba ý trên chính là khung trả lời cho [RES-022](../../14-prep/mock-interview/bank/resume.md) — *phạm vi · ranh giới công lao · đánh đổi đã chấp nhận* — đang nằm ở việc **#4** trong [hàng đợi của plan](../../14-prep/study-plans/datalogic-plan.md). Đọc to, bấm giờ, đừng chỉ đọc thầm.

---

## 4. Ba pattern, một kết luận

| Pattern | Cho bạn | **Không** cho bạn | Phần "không cho" giải bằng |
|---|---|---|---|
| **Observer** | Sự kiện tới đúng nơi, coupling thấp | Ổn định của giá trị | Lọc + hysteresis + ramp (§1.2) |
| **Command** | Yêu cầu đóng gói được, hoãn được, xếp hàng được | Nó tới nơi & đúng lúc | `seq` + `apply_at_ms` + versioning (§2) |
| **Memento** | Chụp/khôi phục không vỡ encapsulation | Đọc được ở máy khác | Version + capability + chính sách lỗi (§3.2) |

> **Chốt:** *pattern mua **cấu trúc**, không mua **tính đúng đắn của miền**.* Interviewer hỏi *"bạn dùng pattern gì"* là câu T1; câu T2 nằm ngay sau đó — *"nó **không** giải quyết được gì, và bạn xử lý phần đó thế nào"*. Nửa sau mới là chỗ tính điểm ([config §6](../../14-prep/mock-interview/config.md)).

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-030](../../14-prep/mock-interview/bank/design-patterns.md) | Nối `onLux` thẳng vào `setBrightness` — đúng Observer nhưng sản phẩm nhấp nháy. Vì sao pattern không cứu được, và bạn thêm gì? |
| [DP-031](../../14-prep/mock-interview/bank/design-patterns.md) | Message trên POSIX mq là Command — cần thêm gì để nhiều panel đổi độ sáng *cùng lúc*, và vì sao struct đó phải có `version`? |
| [DP-032](../../14-prep/mock-interview/bank/design-patterns.md) | Preset là Memento — nhưng export sang màn hình khác thì Memento sách vở thiếu ba thứ gì? |
| [DP-006](../../14-prep/mock-interview/bank/design-patterns.md) | Observer pattern dùng khi nào? Rủi ro cần lưu ý? |
| [DP-034](../../14-prep/mock-interview/bank/design-patterns.md) | Command pattern là gì? Mở ra khả năng nào mà gọi hàm trực tiếp không có? |
| [RES-022](../../14-prep/mock-interview/bank/resume.md) | Feature Preset — phạm vi, ranh giới công lao, đánh đổi đã chấp nhận |

---
⬅️ 🅱️ [B1-redesign-architecture.md](B1-redesign-architecture.md) · [Về bản đồ](README.md)
