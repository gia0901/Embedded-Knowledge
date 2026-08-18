# DP — Design Patterns & SOLID

> Domain `DP`. Track dùng: `design-patterns`, `cpp-system`.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Nền tảng & SOLID | 5 |
| **B** | Creational | 6 |
| **C** | Structural | 5 |
| **D** | Behavioral | 3 |

---

## A — Nền tảng & SOLID

#### DP-003 · 🟢 · concept · [→ README](../../../11-design-patterns/README.md)
**Design pattern là gì và phân thành mấy nhóm?**
<details><summary>Đáp án</summary>

Giải pháp tái sử dụng đã được kiểm chứng cho các vấn đề thiết kế lặp lại; cũng là từ vựng chung để trao đổi thiết kế. Ba nhóm: creational (tạo object — Singleton, Factory, Builder), structural (tổ chức object — Adapter, Bridge/Pimpl, Facade, Proxy), behavioral (tương tác/hành vi — Strategy, Observer, State, Command).
</details>

#### DP-001 · 🟢 · concept · [→ solid-principles](../../../11-design-patterns/solid-principles.md)
**SOLID là gì?**
<details><summary>Đáp án</summary>

5 nguyên lý thiết kế OOP: Single Responsibility (một class một trách nhiệm), Open/Closed (mở để mở rộng, đóng để sửa đổi), Liskov Substitution (lớp con thay được lớp cha về hành vi), Interface Segregation (interface nhỏ chuyên biệt), Dependency Inversion (phụ thuộc abstraction). Mục tiêu: giảm coupling, tăng cohesion → code dễ bảo trì/mở rộng/test.
</details>

#### DP-008 · 🟠 · concept · [→ solid-principles](../../../11-design-patterns/solid-principles.md)
**Cho ví dụ vi phạm Liskov Substitution Principle.**
<details><summary>Đáp án</summary>

Square kế thừa Rectangle: Square override `setWidth` để đổi cả height (giữ vuông), phá vỡ kỳ vọng "đổi width không đổi height" của code viết cho Rectangle → đa hình cho kết quả sai. Square "is-a" Rectangle về dữ liệu nhưng không về hành vi. Bài học: kế thừa phải là is-a thực sự về hành vi; nghi ngờ thì dùng composition.
</details>

#### DP-011 · 🔴 · concept · ⭐ · [→ solid-principles](../../../11-design-patterns/solid-principles.md), [system-design](../../../10-thinking/system-design.md)
**Logic đọc mã vạch của bạn gọi thẳng driver I2C. Sếp yêu cầu: phải test được trên máy dev, không có phần cứng. Dependency Inversion giải quyết thế nào — và cụ thể "đảo ngược" cái gì?**

```cpp
// Trạng thái hiện tại
#include "i2c_driver.h"                      // ← logic PHỤ THUỘC driver

class ScannerLogic {
public:
    int readBarcode() {
        i2c_write(ADDR, CMD_TRIGGER);        // ← gọi thẳng hàm C của driver
        return i2c_read(ADDR);
    }
};
```
<details><summary>Đáp án</summary>

**Cơ chế — "đảo ngược" là đảo CHIỀU PHỤ THUỘC, không phải "dùng interface".**

Đây là chỗ hầu hết câu trả lời dừng lại quá sớm. Thêm một interface **chưa chắc** là DIP; điều quyết định là **ai SỞ HỮU interface đó**.

```
❌ TRƯỚC:   ScannerLogic ──────────────►  i2c_driver
            (cấp cao)                     (cấp thấp)
            Muốn build/test logic thì BẮT BUỘC phải có driver

✅ SAU:     ScannerLogic ──►  IBarcodePort  ◄────── I2cBarcodeAdapter
            (cấp cao)        (cấp cao SỞ HỮU)      (cấp thấp)
            Mũi tên của tầng dưới ĐẢO CHIỀU, chĩa LÊN
```

**Interface phải nằm ở tầng logic và mô tả *nhu cầu của logic*** (`readBarcode()`), **không** mô tả *khả năng của driver* (`i2cWrite`, `i2cRead`). Nếu bạn đặt `II2cDriver` cạnh driver rồi cho logic gọi vào — **không có gì bị đảo cả**, chỉ thêm một tầng gián tiếp.

**Vì sao — hai tầng:**
- **Tầng nông (ai cũng nói được):** *"để thay bằng mock lúc test"*.
- **Tầng thật:** nó cắt **phụ thuộc lúc BUILD/LINK**, không chỉ lúc chạy. Sau khi đảo, `ScannerLogic` **compile được mà không cần file driver tồn tại** ⇒ (1) test chạy trên **x86 host**, nhanh gấp trăm lần nạp firmware; (2) chạy được **ASan/TSan/fuzzer** — thứ không chạy nổi trên MCU; (3) **team logic và team driver làm song song**, chỉ cần chốt interface trước; (4) đổi I2C sang SPI/USB **không đụng một dòng logic nào**.

```cpp
// ✅ SAU — interface do TẦNG LOGIC định nghĩa, theo nhu cầu của nó
struct IBarcodePort {                        // đặt cạnh ScannerLogic
    virtual ~IBarcodePort() = default;
    virtual std::optional<std::string> read() = 0;
};

class ScannerLogic {
    IBarcodePort& port_;                     // ← inject từ ngoài
public:
    explicit ScannerLogic(IBarcodePort& p) : port_(p) {}
    int readBarcode() { /* dùng port_.read() */ }
};

// Tầng driver hiện thực interface của tầng trên — mũi tên chĩa LÊN
class I2cBarcodeAdapter : public IBarcodePort { /* gọi i2c_write/i2c_read */ };

// Test trên host, không cần phần cứng
class FakeBarcodePort : public IBarcodePort {
    std::optional<std::string> read() override { return "4006381333931"; }
};
```

**Ba cách hiện thực seam — chọn theo ràng buộc, đây là phần đánh đổi:**

| Cách | Chi phí runtime | Đổi được lúc nào | Hợp khi |
|---|---|---|---|
| **Virtual interface** (trên) | 1 lần gọi gián tiếp qua vtable | **Runtime** | Mặc định. Không đủ nhanh mới tính tiếp |
| **Template / CRTP** | **0** — nội tuyến hết | Compile-time | Hot path, ISR, MCU không đủ RAM cho vtable |
| **Link-time seam** (cùng tên hàm, 2 file `.c`) | **0** | Lúc link | Codebase **C** thuần, không đổi được sang C++ |

⚠️ Chỉ bỏ virtual khi **đã đo**. Một lần gọi gián tiếp trên đường đọc mã vạch 30 lần/giây là vô nghĩa; trong ISR chạy 100 kHz thì có thể đáng.

**Bẫy:**
1. **Đặt interface ở tầng driver** — lỗi phổ biến nhất, không đảo gì cả (xem sơ đồ).
2. **Interface rò rỉ chi tiết tầng dưới** — `IBarcodePort` mà có hàm `setI2cClockSpeed()` thì đổi sang SPI là vỡ. Interface phải mô tả **cái logic cần**, không phải **cái driver có**.
3. **Mock hoá mọi thứ** → test xanh nhưng không bắt được bug tích hợp thật. DIP cho phép test logic **nhanh**, **không thay thế** test trên phần cứng thật.
4. **Nhầm DIP với DI.** *Dependency **Injection*** chỉ là cách truyền phụ thuộc vào (qua ctor/setter) — một kỹ thuật. *Dependency **Inversion*** là nguyên tắc về **chiều phụ thuộc và quyền sở hữu interface**. Dùng DI mà interface vẫn thuộc tầng dưới ⇒ vẫn chưa đảo.

**Chốt:** *"DIP không phải 'thêm interface' — mà là tầng cao **sở hữu** interface theo nhu cầu của nó, để tầng thấp phải chĩa lên. Bằng chứng đã làm đúng: logic compile và test được khi driver chưa tồn tại."*

> 🎤 Viết lại 2026-08-13 (nợ từ Tuần 1). Bản cũ (465 ký tự) chỉ nêu kết luận *"thay implementation bằng mock"* — không nói được **đảo cái gì**, không có seam nào ngoài virtual, không có đánh đổi.
</details>

#### DP-012 · 🔴 · concept · [→ behavioral](../../../11-design-patterns/behavioral.md)
**Khi nào KHÔNG nên dùng design pattern / áp SOLID?**
<details><summary>Đáp án</summary>

Khi vấn đề đơn giản và code ổn định, không có nhu cầu thay đổi/mở rộng thực sự — nhồi pattern/abstraction là over-engineering (vi phạm YAGNI). Trong embedded còn phải tính chi phí runtime: mỗi interface/virtual thêm vtable + gọi gián tiếp, đáng kể trên hot path. Pattern là công cụ phục vụ "dễ thay đổi an toàn"; đoạn code không cần thay đổi thì đừng tốn công trừu tượng. C++ hiện đại cũng thay nhiều pattern bằng tính năng ngôn ngữ (lambda/`std::function` cho Strategy/Command, `std::variant`+`std::visit` cho Visitor).
</details>

---

## B — Creational

#### DP-002 · 🟢 · concept · [→ creational](../../../11-design-patterns/creational.md)
**Singleton là gì? Cách hiện đại trong C++?**
<details><summary>Đáp án</summary>

Đảm bảo một class chỉ có một instance + điểm truy cập toàn cục. C++11+ dùng Meyers' Singleton: `static` local trong hàm `instance()` — khởi tạo **lazy** (chỉ dựng lần gọi đầu) và **thread-safe theo chuẩn** (compiler sinh guard variable, xem [DP-014](#dp-014--concept---creational)). Cấm copy (`= delete`), constructor private.
</details>

#### DP-004 · 🟡 · concept · [→ creational](../../../11-design-patterns/creational.md)
**Factory pattern giải quyết vấn đề gì? Liên hệ SOLID?**
<details><summary>Đáp án</summary>

Tách quyết định tạo loại object nào khỏi code dùng: client làm việc qua interface chung, gọi factory để nhận object thay vì tự `new` class cụ thể. Hiện thực Dependency Inversion + Open/Closed (thêm loại mới chỉ sửa factory, không sửa client). Hay dùng cho plugin.
</details>

#### DP-009 · 🟠 · concept · [→ creational](../../../11-design-patterns/creational.md)
**Vì sao Singleton bị coi là anti-pattern khi lạm dụng?**
<details><summary>Đáp án</summary>

Vì thực chất là global state trá hình: tạo coupling ẩn (mọi nơi truy cập được, khó lần dependency), khó test (không thay được bằng mock vì truy cập trực tiếp), và gây vấn đề thứ tự khởi tạo/hủy của static (static init order fiasco). Chỉ dùng khi thật sự bắt buộc một instance; cân nhắc dependency injection để dễ test.
</details>

#### DP-014 · 🔴 · concept · ⭐ · [→ creational §Vì sao Meyers thread-safe](../../../11-design-patterns/creational.md)
**Vì sao Meyers' Singleton thread-safe? Double-checked locking tự viết trước C++11 sai ở đâu?**
<details><summary>Đáp án</summary>

**Vì sao thread-safe:** dòng `static T inst;` là local static khởi tạo động; chuẩn C++11 (§[stmt.dcl]) **bắt buộc** — nếu nhiều luồng cùng vào lần đầu, chỉ một luồng chạy khởi tạo, các luồng khác **chờ** tới khi xong. Compiler hiện thực bằng **guard variable** ẩn (Itanium ABI `__cxa_guard_acquire/release`): luồng đầu giành quyền chạy constructor một lần rồi release (set cờ atomic); luồng khác tới cùng lúc bị block tới khi release. Sau lần đầu, mỗi lần gọi chỉ là **một atomic load** (fast path, không khóa) → gần như free. Tên gọi: "magic statics".

**Vì sao DCLP cũ sai:** `inst = new T()` gồm 3 bước — cấp phát, chạy constructor, gán con trỏ. Trước C++11 **không có memory model**, compiler/CPU được sắp xếp lại thành gán-con-trỏ **trước khi** constructor xong → luồng khác thấy `inst != null` ở check-không-khóa và dùng ngay object **dựng dở** → UB. C++11 sửa gốc bằng memory model + `std::atomic` (giờ *có thể* viết DCLP đúng với acquire/release), nhưng đơn giản hơn là để magic statics lo → **đừng tự viết DCLP nữa**.

**Bẫy:** chỉ thread-safe phần *khởi tạo*, không phải phần *dùng* — method sửa trạng thái chung vẫn cần mutex riêng. (Embedded: `-fno-threadsafe-statics` tắt guard để bỏ chi phí atomic, nhưng mất luôn bảo đảm này.)
</details>

#### DP-015 · 🟠 · concept · 🏗️ · [→ creational §Object Pool](../../../11-design-patterns/creational.md), [constraints](../../../08-embedded-systems/constraints.md)
**Object Pool là gì? Vì sao hợp embedded? Điểm tinh tế khi hiện thực?**
<details><summary>Đáp án</summary>

Cấp phát sẵn một tập object cố định + danh sách slot rảnh; `acquire()` mượn một slot, `release()` trả — cả hai **O(1), không chạm heap**. Hợp embedded vì: footprint biết trước lúc biên dịch (mảng tĩnh), thời gian mượn/trả **tất định** (không đi qua allocator → tránh fragmentation và độ trễ bất định của malloc), khi cạn trả `nullptr` thay vì `bad_alloc`. Điểm tinh tế: thường dùng **placement new** để dựng object trên vùng nhớ có sẵn → phải **gọi destructor tường minh** (`p->~T()`) trong `release`, khác `delete` thường; bản production bọc con trỏ trong RAII handle (custom deleter tự gọi `release`) để khỏi quên trả, thêm mutex nếu đa luồng.
</details>

#### DP-013 · 🔴 · design · [→ creational](../../../11-design-patterns/creational.md), [linking-loading](../../../07-shared-libraries/linking-loading.md)
**Thiết kế hệ thống plugin trong C++ dùng pattern nào?**
<details><summary>Đáp án</summary>

Kết hợp Factory + (thường) Strategy/Bridge với cơ chế nạp động: định nghĩa interface abstract cho plugin (`IPlugin` với pure virtual); mỗi plugin là shared library export hàm factory `extern "C"` (tránh name mangling) trả về con trỏ tới interface. Chương trình chính dùng `dlopen`/`dlsym` nạp `.so` lúc runtime, lấy factory, gọi qua interface — không cần biết class cụ thể lúc build (Open/Closed). Chú ý: ownership rõ ràng (plugin tạo thì plugin hủy, hoặc trả unique_ptr), không để exception/kiểu C++ vượt biên nếu cần ổn định ABI, và quản lý vòng đời `.so` (không dlclose khi còn object sống).
</details>

---

## C — Structural

#### DP-007 · 🟡 · concept · [→ structural](../../../11-design-patterns/structural.md)
**Pimpl liên quan Bridge thế nào? Lợi ích?**
<details><summary>Đáp án</summary>

Pimpl là ứng dụng cụ thể của Bridge: tách abstraction (class public) khỏi implementation (struct Impl trong .cpp) qua `unique_ptr<Impl>`. Lợi ích: bảo vệ ABI (sizeof không đổi, thêm field không phá layout public) và giảm compile time (header không lộ dependency).
</details>

#### DP-016 · 🟡 · concept · [→ structural §Decorator](../../../11-design-patterns/structural.md)
**Decorator pattern là gì? Khác Proxy thế nào?**
<details><summary>Đáp án</summary>

Decorator thêm hành vi cho object **động, từng lớp** bằng cách bọc nó trong các decorator **cùng interface**, có thể **xếp chồng** nhiều lớp (vd file ← nén ← mã hóa) — tránh bùng nổ lớp con cho mọi tổ hợp tính năng (`EncryptedCompressedStream`...). Mỗi decorator vừa *là* interface đó vừa *giữ* một con trỏ tới object được bọc để ủy nhiệm. Khác Proxy: cả hai đều bọc và cùng interface, nhưng **Proxy kiểm soát *truy cập*** tới một object (lazy load, quyền, cache) — thường một lớp; **Decorator thêm *chức năng*** và thiết kế để **chồng nhiều lớp**. Trong C++ có thể thay bằng template/composition khi tập tính năng biết lúc compile.
</details>

#### DP-019 · 🟡 · concept · 📦 2026-08-13 · [→ structural](../../../11-design-patterns/structural.md)
**Adapter pattern dùng khi nào? Cho ví dụ trong embedded.**
<details><summary>Đáp án</summary>

**Adapter chuyển đổi interface của một lớp có sẵn sang interface mà code của bạn mong đợi** — để hai thứ **không được thiết kế để làm việc cùng nhau** vẫn ghép được.

**Khi nào dùng — ba dấu hiệu:**
1. Có một **thư viện/driver bên thứ ba** không sửa được, nhưng interface của nó không khớp với hệ thống của bạn.
2. Muốn **đổi nhà cung cấp** (đổi chip cảm biến, đổi thư viện) mà **không sửa logic**.
3. Cần **cách ly** code của mình khỏi một API hay thay đổi.

```cpp
struct ITempSensor {                       // interface hệ thống bạn cần
    virtual ~ITempSensor() = default;
    virtual float celsius() = 0;
};

class Tmp102Adapter : public ITempSensor { // bọc driver của hãng
    tmp102_handle_t h_;                    // API C của hãng: đọc ra raw 12-bit
public:
    float celsius() override { return tmp102_read_raw(&h_) * 0.0625f; }
};
```

⇒ **Đây chính là bản chất của HAL trong embedded** ([BSP-001](bsp.md)): mỗi chip một adapter, logic phía trên chỉ biết `ITempSensor`. Đổi chip = viết adapter mới, **không đụng logic**.

**Liên hệ với DIP:** adapter là cách **hiện thực** Dependency Inversion — interface do tầng logic sở hữu, adapter (tầng thấp) hiện thực nó ⇒ mũi tên phụ thuộc chĩa lên ([DP-011](design-patterns.md)).

⚠️ **Đánh đổi:** thêm một tầng gián tiếp (lời gọi ảo + đôi khi cả chuyển đổi dữ liệu). Ở đường nóng hoặc MCU RAM ít, cân nhắc adapter **compile-time** bằng template thay vì virtual ([CPP-017](cpp.md)).

⚠️ **Bẫy:** adapter **rò rỉ** interface gốc — vẫn phơi ra khái niệm riêng của chip (thanh ghi, mã lỗi của hãng) ⇒ đổi chip vẫn phải sửa logic, tức là adapter đã thất bại ở đúng mục đích của nó.

**Chốt:** *"Adapter dịch interface của thứ có sẵn sang thứ bạn cần — nền của HAL. Nhưng nếu nó vẫn để lộ khái niệm của chip ra ngoài thì đổi chip vẫn phải sửa logic, và adapter đó vô dụng."*
</details>

#### DP-017 · 🟡 · concept · 📦 2026-08-13 · [→ structural](../../../11-design-patterns/structural.md)
**Facade khác Adapter thế nào?**
<details><summary>Đáp án</summary>

Cả hai đều "bọc" thứ khác lại, nhưng **mục đích ngược nhau**:

| | **Adapter** | **Facade** |
|---|---|---|
| Vấn đề giải quyết | Interface **không khớp** | Interface **quá phức tạp** |
| Bọc cái gì | Thường **một** lớp/thư viện | **Nhiều** lớp / cả một hệ con |
| Interface kết quả | **Do bên ngoài quy định** (bạn phải khớp) | **Do bạn tự thiết kế** cho gọn |
| Câu hỏi nó trả lời | *"Làm sao ghép được?"* | *"Làm sao dùng cho đỡ mệt?"* |

**Ví dụ trong embedded:**
- **Adapter:** driver TMP102 của hãng → interface `ITempSensor` của bạn ([DP-019](design-patterns.md)).
- **Facade:** `ScannerApi::scan()` — bên trong nó bật nguồn cảm biến, chờ ổn định, cấu hình phơi sáng, chụp, giải mã, tắt nguồn. Người dùng chỉ gọi **một hàm** thay vì biết cả bảy bước và thứ tự của chúng.

**⭐ Facade thường là thứ bạn phơi ra ở BIÊN GIỚI THƯ VIỆN**: giấu hệ con phức tạp sau một interface nhỏ ⇒ vừa dễ dùng, vừa **giảm bề mặt ABI** phải giữ ổn định ([SD-020](system-design.md)).

⚠️ **Bẫy của Facade:** giấu quá tay ⇒ người dùng cần một biến thể mà facade không cho ⇒ họ **đi vòng qua** nó, và bạn có hai đường vào hệ con. Cách xử lý: facade cho ca thường gặp, **vẫn cho phép truy cập tầng dưới** khi cần — đừng chặn cứng.

**Chốt:** *"Adapter sửa interface không khớp, Facade làm gọn interface quá phức tạp. Adapter phải theo hình dạng người ta quy định; Facade thì bạn tự thiết kế."*
</details>

#### DP-018 · 🟠 · concept · 📦 2026-08-13 · [→ structural](../../../11-design-patterns/structural.md)
**Proxy pattern có những biến thể nào? Cho ví dụ ứng dụng.**
<details><summary>Đáp án</summary>

**Proxy giữ NGUYÊN interface của đối tượng thật**, nhưng chen vào giữa để làm thêm việc gì đó. Đây là điểm phân biệt: adapter **đổi** interface, proxy **giữ nguyên**.

| Biến thể | Thêm việc gì | Ví dụ |
|---|---|---|
| **Virtual proxy** | **Hoãn khởi tạo** tới lần dùng đầu | Ảnh/tài nguyên nặng chỉ nạp khi thực sự cần |
| **Remote proxy** | Che giấu việc đối tượng nằm ở **process/máy khác** | Client stub của RPC/IPC — gọi như hàm cục bộ |
| **Protection proxy** | Kiểm tra **quyền** trước khi cho qua | Chỉ tiến trình có quyền mới ghi được thanh ghi cấu hình |
| **Caching proxy** | Nhớ kết quả, tránh gọi lại | Đọc cảm biến chậm: trả giá trị cache nếu chưa quá N ms |
| **Logging/counting proxy** | Ghi lại mọi lời gọi | Gỡ lỗi, đo hiệu năng mà **không sửa** đối tượng thật |

**Ví dụ embedded đáng nhớ — caching proxy cho cảm biến:**
```cpp
class CachedSensor : public ITempSensor {
    ITempSensor& real_;  float last_;  steady_clock::time_point t_;
public:
    float celsius() override {
        auto now = steady_clock::now();                 // ⚠️ steady_clock, không system_clock
        if (now - t_ > 100ms) { last_ = real_.celsius(); t_ = now; }
        return last_;                                   // đọc I2C chậm ⇒ tránh gọi lại
    }
};
```
Logic phía trên **không biết** có cache — đó chính là giá trị của việc giữ nguyên interface.

⚠️ **Đánh đổi:** proxy làm hành vi **khó đoán hơn** — người gọi tưởng đang đọc giá trị mới nhất nhưng nhận giá trị cũ 100 ms. Với dữ liệu điều khiển thì đây có thể là bug nghiêm trọng ⇒ **phải ghi rõ trong tài liệu**, và cân nhắc cho phép ép đọc mới.

**Chốt:** *"Proxy giữ nguyên interface và chen thêm việc: hoãn khởi tạo, che giấu khoảng cách, kiểm quyền, cache, ghi log. Khác adapter ở chỗ adapter ĐỔI interface còn proxy thì không."*
</details>

---
⬅️ [Bank index](README.md)

---

## D — Behavioral

#### DP-005 · 🟡 · concept · [→ behavioral](../../../11-design-patterns/behavioral.md)
**Strategy pattern là gì? C++ hiện đại hiện thực gọn thế nào?**
<details><summary>Đáp án</summary>

Đóng gói các thuật toán/hành vi hoán đổi được sau interface chung, chọn/đổi lúc runtime mà không sửa code dùng (OCP + DIP). C++ hiện đại với strategy đơn giản thường dùng `std::function` + lambda gọn hơn cả cây class; cần hiệu năng compile-time thì dùng template parameter.
</details>

#### DP-006 · 🟡 · concept · [→ behavioral](../../../11-design-patterns/behavioral.md)
**Observer pattern dùng khi nào? Rủi ro?**
<details><summary>Đáp án</summary>

Khi một subject đổi trạng thái cần tự động thông báo nhiều observer quan tâm mà không gắn chặt — nền của event-driven/callback (phổ biến embedded: sự kiện sensor, GPIO, nút bấm). Rủi ro chính: lifetime/dangling — observer bị hủy mà chưa unsubscribe → subject gọi vào con trỏ chết (UB); dùng weak_ptr hoặc unsubscribe an toàn.
</details>

#### DP-010 · 🟠 · concept · [→ behavioral](../../../11-design-patterns/behavioral.md)
**State pattern là gì? Vì sao embedded hay dùng enum + switch thay vì bản OOP?**
<details><summary>Đáp án</summary>

State pattern cho object đổi hành vi khi trạng thái nội bộ đổi — hiện thực state machine (cốt lõi firmware/protocol). Embedded thường dùng enum + switch/bảng chuyển trạng thái vì tất định, không cấp phát động, không chi phí virtual, footprint biết trước, dễ review — hợp ràng buộc tài nguyên và tin cậy. Bản OOP (mỗi state một class) chỉ đáng dùng khi logic mỗi state đủ phức tạp.
</details>

---

⬅️ [Bank index](README.md)
