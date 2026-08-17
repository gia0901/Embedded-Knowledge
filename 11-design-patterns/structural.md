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

## 5. Decorator — thêm hành vi từng lớp

Thêm hành vi cho object **động, từng lớp**, mà không sửa class gốc — bằng cách bọc object trong các "decorator" **cùng interface**. Điểm khác Proxy: Proxy *kiểm soát truy cập* tới một object, Decorator *thêm chức năng* và có thể **xếp chồng** nhiều lớp.

```cpp
struct IStream { virtual void write(const std::string&) = 0; virtual ~IStream() = default; };

class FileStream : public IStream {                    // object gốc
    void write(const std::string& s) override { /* ghi ra file */ }
};

// Decorator cơ sở: vừa LÀ IStream, vừa GIỮ một IStream để ủy nhiệm
class StreamDecorator : public IStream {
protected:
    std::unique_ptr<IStream> wrapped_;
public:
    explicit StreamDecorator(std::unique_ptr<IStream> s) : wrapped_(std::move(s)) {}
};

class CompressStream : public StreamDecorator {
    using StreamDecorator::StreamDecorator;
    void write(const std::string& s) override { wrapped_->write(compress(s)); }  // thêm 1 lớp
};
class EncryptStream : public StreamDecorator {
    using StreamDecorator::StreamDecorator;
    void write(const std::string& s) override { wrapped_->write(encrypt(s)); }   // thêm lớp nữa
};

// Xếp chồng động: file ← nén ← mã hóa
std::unique_ptr<IStream> s =
    std::make_unique<EncryptStream>(
        std::make_unique<CompressStream>(
            std::make_unique<FileStream>()));
s->write("data");   // encrypt → compress → ghi file
```

Tránh **bùng nổ lớp con** cho mọi tổ hợp tính năng (`EncryptedCompressedFileStream`, `CompressedFileStream`...) — chỉ cần ghép các decorator. Trong C++ đôi khi thay bằng template/composition khi tập tính năng biết lúc compile.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-019](../14-prep/mock-interview/bank/design-patterns.md) | Adapter pattern dùng khi nào? |
| [DP-007](../14-prep/mock-interview/bank/design-patterns.md) | Pimpl liên quan thế nào tới Bridge pattern? Lợi ích của nó? |
| [DP-017](../14-prep/mock-interview/bank/design-patterns.md) | Facade khác Adapter thế nào? |
| [DP-018](../14-prep/mock-interview/bank/design-patterns.md) | Proxy pattern có những biến thể nào? Cho ví dụ ứng dụng. |

---
⬅️ [creational.md](creational.md) · ➡️ Tiếp theo: [behavioral.md](behavioral.md)
