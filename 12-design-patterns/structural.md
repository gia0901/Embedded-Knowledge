# Structural Patterns — Adapter, Bridge/Pimpl, Facade, Proxy

> **TL;DR**
> - Structural pattern giải quyết **cách tổ chức/kết hợp object & class** thành cấu trúc lớn hơn mà vẫn linh hoạt.
> - **Adapter**: bọc một interface không tương thích thành interface client mong đợi ("đầu chuyển").
> - **Bridge / Pimpl**: tách abstraction khỏi implementation để hai bên thay đổi độc lập; Pimpl là ứng dụng quen thuộc trong C++ (bảo vệ ABI, giảm compile time).
> - **Facade**: cung cấp một interface đơn giản che giấu một hệ thống con phức tạp.
> - **Proxy**: object thay thế kiểm soát truy cập tới object thật (lazy load, caching, access control).
> - Nhiều cái đã gặp rải rác: Pimpl ([api-design](../07-shared-libraries/api-design.md)), HAL ~ Adapter/Facade.

---

## 1. Adapter — đầu chuyển interface

Khi có một class hữu ích nhưng interface không khớp với cái client cần (vd thư viện bên thứ ba, code legacy, driver cũ), Adapter bọc nó lại.

```cpp
// Client mong đợi interface này
struct ILogger { virtual void log(const std::string&) = 0; };

// Thư viện cũ có interface khác
class ThirdPartyLog { public: void writeMessage(const char*, int level); };

// Adapter: chuyển ILogger → ThirdPartyLog
class LogAdapter : public ILogger {
    ThirdPartyLog& impl_;
public:
    LogAdapter(ThirdPartyLog& l) : impl_(l) {}
    void log(const std::string& s) override { impl_.writeMessage(s.c_str(), 0); }
};
```

Dùng để tích hợp code không sửa được (thư viện, hardware API) vào kiến trúc của mình mà không lan tỏa interface lạ ra khắp nơi.

---

## 2. Bridge & Pimpl — tách abstraction khỏi implementation

**Bridge**: tách phần "abstraction" (cái client thấy) khỏi "implementation" (cách làm), nối qua một con trỏ → hai bên tiến hóa độc lập, tránh bùng nổ lớp con khi có nhiều chiều biến thể.

**Pimpl** (Pointer to Implementation) là ứng dụng phổ biến nhất của ý tưởng này trong C++:
```cpp
// widget.h — interface ổn định, KHÔNG lộ data member
class Widget {
public:
    Widget(); ~Widget();
    void doSomething();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;   // "cây cầu" tới implementation
};
```
Lợi: **bảo vệ ABI** (thêm field vào Impl không đổi layout public), **giảm compile time** (header không lộ dependency). Chi tiết: [api-design](../07-shared-libraries/api-design.md).

---

## 3. Facade — mặt tiền đơn giản

Cung cấp **một interface cấp cao, đơn giản** che giấu sự phức tạp của nhiều hệ thống con bên dưới.

```cpp
// Bên dưới: nhiều bước phức tạp
class FuelInjector; class Ignition; class Starter;

// Facade: client chỉ cần một lời gọi
class Engine {
    FuelInjector fuel; Ignition ign; Starter starter;
public:
    void start() {            // che giấu trình tự phức tạp
        fuel.prime();
        ign.on();
        starter.crank();
    }
};
// client: engine.start();  — không cần biết 3 bước bên trong
```

Giảm coupling giữa client và hệ thống con; client không cần hiểu chi tiết. **Hardware Abstraction Layer (HAL)** trong embedded thường mang tính Facade + Adapter: che giấu chi tiết thanh ghi/driver sau một API gọn.

---

## 4. Proxy — đại diện kiểm soát truy cập

Một object đứng thay cho object thật, kiểm soát việc truy cập tới nó. Các biến thể:
- **Virtual proxy**: trì hoãn tạo object đắt tới khi thật sự cần (lazy loading).
- **Protection proxy**: kiểm tra quyền trước khi cho truy cập.
- **Caching proxy**: lưu kết quả để tránh tính lại/gọi mạng.
- **Remote proxy**: đại diện cục bộ cho object ở xa (RPC stub).

```cpp
struct IImage { virtual void draw() = 0; };
class RealImage : public IImage {                  // nặng: nạp file ảnh
    RealImage(const std::string& path) { /* load từ disk */ }
};
class ImageProxy : public IImage {                 // nhẹ: chỉ nạp khi draw
    std::string path_;
    std::unique_ptr<RealImage> real_;
public:
    ImageProxy(std::string p) : path_(std::move(p)) {}
    void draw() override {
        if (!real_) real_ = std::make_unique<RealImage>(path_);  // lazy
        real_->draw();
    }
};
```

`shared_ptr`/`weak_ptr` bản thân cũng là dạng smart proxy quản lý vòng đời.

---

## 5. Decorator (điểm danh nhanh)

Thêm hành vi cho object **động, từng lớp**, mà không sửa class gốc — bằng cách bọc object trong các "decorator" cùng interface (vd thêm buffering/compression cho một stream). Tránh bùng nổ lớp con cho mọi tổ hợp tính năng. Trong C++ đôi khi thay bằng template/composition.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Adapter pattern dùng khi nào?</summary>

Adapter dùng khi bạn có một class hữu ích nhưng interface của nó **không khớp** với interface mà client mong đợi — điển hình là thư viện bên thứ ba, code legacy, hoặc API phần cứng mà bạn không sửa được. Adapter bọc object đó lại và phơi bày interface mà client cần, chuyển đổi lời gọi giữa hai bên. Lợi ích: tích hợp code không sửa được vào kiến trúc của mình mà không để interface lạ lan tỏa khắp codebase, và có thể thay thế thư viện bên dưới mà chỉ sửa adapter. Trong embedded, một lớp bọc driver/chip cụ thể sau một interface chung chính là dạng adapter.
</details>

<details><summary>2) Pimpl liên quan thế nào tới Bridge pattern? Lợi ích của nó?</summary>

Pimpl (Pointer to Implementation) là một ứng dụng cụ thể của ý tưởng Bridge: tách phần abstraction (class public mà client thấy) khỏi phần implementation (đặt trong struct `Impl` ở file .cpp), nối với nhau qua một `unique_ptr<Impl>`. Lợi ích chính trong C++: **bảo vệ ABI** — `sizeof` class public không đổi (chỉ là một con trỏ) nên thêm/bớt data member trong `Impl` không thay đổi layout mà client phụ thuộc, tránh ABI break; và **giảm thời gian biên dịch** — header không cần include dependency của implementation, nên thay đổi nội bộ không buộc rebuild mọi nơi include header. Đánh đổi là một lần truy cập gián tiếp qua con trỏ và một cấp phát heap.
</details>

<details><summary>3) Facade khác Adapter thế nào?</summary>

Cả hai đều bọc cái khác, nhưng mục đích khác. Adapter chuyển đổi **một** interface không tương thích thành interface mà client mong đợi — mục tiêu là tương thích, thường giữ nguyên mức độ chức năng. Facade cung cấp **một interface đơn giản, cấp cao** che giấu sự phức tạp của **nhiều** hệ thống con bên dưới — mục tiêu là đơn giản hóa, giảm cho client phải hiểu chi tiết. Nói cách khác: Adapter làm cho thứ không khớp trở nên dùng được; Facade làm cho thứ phức tạp trở nên dễ dùng. Trong embedded, một Hardware Abstraction Layer thường kết hợp cả hai: facade hóa trình tự phức tạp và adapter hóa các chip/driver khác nhau sau một API thống nhất.
</details>

<details><summary>4) Proxy pattern có những biến thể nào? Cho ví dụ ứng dụng.</summary>

Proxy là object đại diện kiểm soát truy cập tới object thật, với các biến thể: virtual proxy (lazy loading — trì hoãn tạo object đắt tới khi thật sự cần, vd chỉ nạp file ảnh khi draw); protection proxy (kiểm tra quyền trước khi cho truy cập); caching proxy (lưu kết quả tránh tính lại hoặc gọi mạng lặp); remote proxy (đại diện cục bộ cho object ở xa, như stub RPC). Ví dụ quen thuộc trong C++: `shared_ptr`/`weak_ptr` là dạng smart proxy quản lý vòng đời và truy cập object. Proxy hữu ích khi muốn thêm một lớp kiểm soát (hiệu năng, bảo mật, vòng đời) mà client vẫn dùng object qua cùng interface như bình thường.
</details>

---
⬅️ [creational.md](creational.md) · ➡️ Tiếp theo: [behavioral.md](behavioral.md)
