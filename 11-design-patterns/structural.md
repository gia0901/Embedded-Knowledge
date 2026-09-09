# Structural — Ghép object lại mà không dính chặt vào nhau

> **TL;DR**
> - Structural pattern trả lời: *"hai mảnh này phải làm việc với nhau — làm sao để đổi một bên không kéo theo bên kia?"*
> - **Bridge** là cái quan trọng nhất cho việc của bạn: khi có **hai chiều biến thiên độc lập**, kế thừa cả hai cho **N×M** lớp; Bridge kéo về **N+M**. **Pimpl** là một ứng dụng khác của cùng ý tưởng, phục vụ mục tiêu khác: **giấu triển khai & giữ ABI**.
> - **Adapter** đổi interface cho khớp; **Facade** giữ nguyên các interface bên dưới nhưng che một trình tự phức tạp. HAL thường là **cả hai cộng lại**.
> - **Proxy** và **Decorator** ở mức **nhận diện tên** — cùng hình dạng "bọc một object", khác mục đích.
> - Ranh giới `.so` làm mọi thứ ở đây đắt hơn: mỗi interface là một **hợp đồng nhị phân** ([in-practice/02 §3.3](in-practice/02-interface-impl-plugin.md)).

---

## 0. Bốn cách "bọc một object" — phân biệt trước khi đi sâu

Bốn pattern dưới đây có **hình dạng code gần như giống hệt nhau** (giữ một con trỏ tới interface rồi ủy nhiệm). Ai nhớ được **mục đích** thì phân biệt được; ai chỉ nhớ hình dạng thì lẫn.

| Pattern | Interface thấy được | Mục đích | Xếp chồng nhiều lớp? |
|---|---|---|---|
| **Adapter** | **ĐỔI** (A → B) | Ghép cái không khớp | Không |
| **Bridge** | Giữ nguyên | Tách **hai chiều biến thiên** | Không |
| **Facade** | **MỚI, đơn giản hơn** | Che một **trình tự** phức tạp | Không |
| Proxy | Giữ nguyên | **Kiểm soát truy cập** tới object thật | Hiếm |
| Decorator | Giữ nguyên | **Thêm chức năng** | **Có** — điểm nhận dạng chính |

> **Phép thử một câu:** *Interface có đổi không?* → đổi ⟹ Adapter. *Có thêm việc mới không?* → có, xếp chồng được ⟹ Decorator. *Chỉ chắn đường vào object thật?* ⟹ Proxy. *Che nhiều bước gọi?* ⟹ Facade. *Cắt một trục biến thiên ra?* ⟹ Bridge.

---

## 1. Bridge — pattern quan trọng nhất của file này

| | |
|---|---|
| **Bản chất** | Tách **abstraction** (cái client thấy) khỏi **implementation** (cách làm), nối bằng một con trỏ, để hai bên tiến hoá độc lập |
| **Vấn đề nó giải** | **Bùng nổ lớp con** khi có ≥ 2 chiều biến thiên |
| **Dấu hiệu cần dùng** | Tên class bắt đầu ghép hai chiều: `LocalDimmingSocA`, `OledDimmingSocB`… |
| **Cái giá** | Một interface phụ + một lần gián tiếp; object phải được **ghép** lúc dựng |
| **Đừng dùng khi** | Chỉ có một chiều biến thiên ⟹ kế thừa thường (hoặc Strategy) là đủ |

**Bài toán:** N thuật toán dimming × M SoC (mỗi SoC ghi duty xuống HW một kiểu).

```
Ke thua ca hai truc:
  GlobalDimmingSocA   LocalDimmingSocA   OledDimmingSocA
  GlobalDimmingSocB   LocalDimmingSocB   OledDimmingSocB     => N x M lop
```

```cpp
// IMPLEMENTOR — bien thien theo SoC
class IDimmingBackend {
public:
    virtual ~IDimmingBackend() = default;
    virtual void writeDuty(int zone, int duty) = 0;   // SocA: ioctl · SocB: sysfs/mailbox
    virtual int  zoneCount() const             = 0;
};

// ABSTRACTION — bien thien theo thuat toan
class DimmingBase : public IDimming {
protected:
    std::unique_ptr<IDimmingBackend> hw_;             // "cay cau" — KHONG ke thua
public:
    explicit DimmingBase(std::unique_ptr<IDimmingBackend> hw) : hw_(std::move(hw)) {}
};

// Ghep luc chay: N + M lop, phu duoc N x M to hop
auto d = std::make_unique<LocalDimming>(std::make_unique<SocABackend>());
```

| | Kế thừa hai trục | **Bridge** |
|---|---|---|
| Số lớp | **N × M** | **N + M** |
| Thêm 1 SoC | +N lớp | **+1 lớp** |
| Thêm 1 thuật toán | +M lớp | **+1 lớp** |
| Tổ hợp mới | Phải viết lớp mới | **Ghép lúc runtime** |

### 1.1 ⭐ Bridge vs Strategy — code giống hệt, Ý ĐỊNH khác

Đây là câu hỏi phân loại ứng viên, và trả lời *"hai cái giống nhau"* rồi dừng là 2 điểm.

| | **Strategy** (behavioral) | **Bridge** (structural) |
|---|---|---|
| Số trục biến thiên | **Một** | **Hai** |
| Thứ được cắm vào là | Một **thuật toán thay thế được** | **Nửa còn lại** của cùng một thứ |
| Object có chạy được nếu thiếu nó? | Thường có (có default) | **Không** — dimming *cần* backend |
| Đổi lúc runtime | ✅ mục đích chính | Hiếm; gắn lúc dựng |
| Câu hỏi nó trả lời | *"Cư xử thế nào?"* | *"Tránh bùng nổ lớp con kiểu gì?"* |

> **Chốt:** *Strategy tách **hành vi khỏi context**; Bridge tách **hai chiều biến thiên khỏi nhau**.*

### 1.2 Pimpl — cùng ý tưởng, mục tiêu khác

```cpp
// widget.h — interface on dinh, KHONG lo data member
class Widget {
public:
    Widget(); ~Widget();
    void doSomething();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;   // "cay cau" toi implementation
};
```

| | Bridge cổ điển | **Pimpl** |
|---|---|---|
| Mua được gì | Chống bùng nổ lớp con | **Giấu triển khai · giữ ABI · giảm compile time** |
| Số implementation | Nhiều, thay được | Thường **đúng một** |
| Implementor có đa hình không | Có | Thường không |

**Vì sao Pimpl giữ được ABI:** thêm field vào `Impl` **không đổi `sizeof(Widget)`** (vẫn là một con trỏ) ⟹ code đã biên dịch trước đó vẫn dựng object đúng kích thước. Đối chiếu: thêm data member thẳng vào class **là ABI break**, và thêm virtual cũng vậy ([in-practice/02 §3.3](in-practice/02-interface-impl-plugin.md) · [07/abi-versioning](../07-shared-libraries/abi-versioning.md)).

**Cái giá của Pimpl, phải nói ra:** một lần cấp phát heap cho mỗi object + một lần gián tiếp mỗi lời gọi + không inline được. Trên đường nóng của embedded thì cân nhắc; trên API cấu hình thì gần như miễn phí. Chi tiết: [07/api-design §2](../07-shared-libraries/api-design.md).

---

## 2. Adapter — đầu chuyển interface

| | |
|---|---|
| **Bản chất** | Bọc một class hữu ích nhưng interface **không khớp**, phơi ra interface mà client mong đợi |
| **Vấn đề nó giải** | Tích hợp code **không sửa được**: thư viện bên thứ ba, driver legacy, API vendor |
| **Cái nó KHÔNG làm** | Không đơn giản hoá, không thêm chức năng — chỉ **dịch** |
| **Đừng dùng khi** | Bạn **sửa được** phía kia ⟹ sửa thẳng, đừng thêm một tầng |

```cpp
struct ILogger { virtual void log(const std::string&) = 0; };   // client mong doi

class ThirdPartyLog { public: void writeMessage(const char*, int level); };  // thu vien cu

class LogAdapter : public ILogger {                             // dau chuyen
    ThirdPartyLog& impl_;
public:
    explicit LogAdapter(ThirdPartyLog& l) : impl_(l) {}
    void log(const std::string& s) override { impl_.writeMessage(s.c_str(), 0); }
};
```

⭐ **Giá trị thật của Adapter là ngăn interface lạ LAN RA.** Không có nó, chữ ký của vendor (kiểu dữ liệu, quy ước mã lỗi, `const char*` thay vì `std::string`) sẽ rò rỉ vào khắp codebase — và khi đổi vendor thì phải sửa mọi nơi. Adapter dồn toàn bộ thiệt hại về **một file**.

**Ví dụ embedded:** SoC vendor cấp API dạng `int vendor_set_bl(uint8_t*, size_t)`; adapter phơi ra `IDimmingBackend::writeDuty(int zone, int duty)` mà phần còn lại của library dùng. Đổi vendor = viết adapter mới.

---

## 3. Facade — mặt tiền cho một trình tự

| | |
|---|---|
| **Bản chất** | Một interface **cấp cao mới** che một **trình tự nhiều bước** qua nhiều hệ thống con |
| **Vấn đề nó giải** | Client phải biết thứ tự gọi đúng, và thứ tự đó dễ làm sai |
| **Cái nó KHÔNG làm** | Không giấu các lớp bên dưới (vẫn dùng trực tiếp được), không thêm chức năng |
| **Đừng dùng khi** | Bên dưới chỉ có một lời gọi ⟹ không có "trình tự" nào để che |

```cpp
class Engine {                      // Facade
    FuelInjector fuel; Ignition ign; Starter starter;
public:
    void start() {                  // che TRINH TU, khong che tung class
        fuel.prime();
        ign.on();
        starter.crank();
    }
};
```

### 3.1 Adapter vs Facade — câu hỏi hay bị hỏi ([DP-017](../14-prep/mock-interview/bank/design-patterns.md))

| | **Adapter** | **Facade** |
|---|---|---|
| Interface đích | **Có sẵn**, client đã yêu cầu | **Bạn tự đặt ra**, cho gọn |
| Số object bên dưới | Thường **một** | Thường **nhiều** |
| Động cơ | *"Không khớp"* | *"Quá phức tạp / dễ gọi sai thứ tự"* |
| Mức trừu tượng | **Ngang bằng** | **Cao hơn** |

**HAL thường là cả hai:** *Adapter* ở chỗ dịch API/vendor/register sang một interface thống nhất; *Facade* ở chỗ gói *"mở clock → cấu hình pin → ghi thanh ghi → chờ ready"* thành một `enable()`. Nói được cả hai vai này là một câu trả lời mạnh cho JD embedded.

---

## 4. Proxy — nhận diện

Một object **cùng interface** đứng thay object thật để **kiểm soát truy cập**. Bốn biến thể: **virtual** (hoãn khởi tạo object đắt), **protection** (kiểm quyền), **caching** (giữ kết quả), **remote** (đại diện cho object ở xa — RPC stub).

```cpp
class ImageProxy : public IImage {
    std::string path_;
    std::unique_ptr<RealImage> real_;
public:
    void draw() override {
        if (!real_) real_ = std::make_unique<RealImage>(path_);   // lazy
        real_->draw();
    }
};
```

⚠️ **Đánh đổi phải nói ra:** proxy làm hành vi **khó đoán hơn** — caller tưởng đọc giá trị mới nhất nhưng nhận bản cache 100 ms. Với dữ liệu điều khiển thì đó có thể là bug nghiêm trọng ⟹ phải ghi rõ trong tài liệu và cân nhắc cho phép ép đọc mới. `shared_ptr`/`weak_ptr` bản thân cũng là dạng smart proxy quản lý vòng đời.

---

## 5. Decorator — nhận diện

Thêm hành vi **động, từng lớp**, bằng cách bọc object trong các decorator **cùng interface**. Điểm nhận dạng: **xếp chồng được nhiều lớp**.

```cpp
// file <- nen <- ma hoa
auto s = std::make_unique<EncryptStream>(
             std::make_unique<CompressStream>(
                 std::make_unique<FileStream>()));
```

Tránh **bùng nổ lớp con** cho mọi tổ hợp tính năng (`EncryptedCompressedFileStream`…). **Khác Proxy:** Proxy *kiểm soát truy cập* tới một object cụ thể; Decorator *thêm chức năng* và **xếp chồng được**.

> **Vì sao Proxy/Decorator chỉ ở mức nhận diện** ([bản đồ chọn lọc](in-practice/README.md)): chúng giải bài toán *tầng ứng dụng/luồng dữ liệu*, hiếm xuất hiện ở tầng HAL/driver; và trong C++ hiện đại phần lớn ca đơn giản được thay bằng template/composition khi tập tính năng biết lúc compile. Cần nhớ đúng **một** thứ: **hình dạng giống nhau, mục đích khác nhau** (bảng §0).

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-038](../14-prep/mock-interview/bank/design-patterns.md) | Bridge giải quyết vấn đề gì? Khác Strategy chỗ nào khi code giống hệt nhau? |
| [DP-023](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | N thuật toán dimming × M SoC — thiết kế sao để không thành N×M lớp? |
| [DP-007](../14-prep/mock-interview/bank/design-patterns.md) | Pimpl liên quan Bridge thế nào? Lợi ích? |
| [SD-021](../14-prep/mock-interview/bank/system-design.md) | Pimpl idiom là gì? Nó giải quyết vấn đề gì, và cái giá là gì? |
| [DP-019](../14-prep/mock-interview/bank/design-patterns.md) | Adapter pattern dùng khi nào? Cho ví dụ trong embedded. |
| [DP-017](../14-prep/mock-interview/bank/design-patterns.md) | Facade khác Adapter thế nào? |
| [DP-018](../14-prep/mock-interview/bank/design-patterns.md) | Proxy pattern có những biến thể nào? Cho ví dụ ứng dụng. |
| [DP-016](../14-prep/mock-interview/bank/design-patterns.md) | Decorator pattern là gì? Khác Proxy thế nào? |
| [DP-027](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Thêm virtual vào giữa interface, giữ `.so` cũ — chuyện gì xảy ra? |

---
⬅️ [creational.md](creational.md) · ➡️ Tiếp theo: [behavioral.md](behavioral.md)
