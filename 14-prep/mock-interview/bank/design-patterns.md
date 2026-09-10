# DP — Design Patterns & SOLID

> Domain `DP`. Track dùng: `design-patterns`, `cpp-system`.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Nền tảng & SOLID | 5 |
| **B** | Creational | 7 |
| **C** | Structural | 6 |
| **D** | Behavioral | 7 |
| **E** | 🎯 **Case study hệ display** — pattern rút từ chính code & resume của bạn | 13 |

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
</details>

#### DP-012 · 🔴 · concept · [→ solid-principles §7](../../../11-design-patterns/solid-principles.md)
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

> ⚠️ **"Một instance" chỉ đúng trong MỘT chương trình đã link xong.** Khi singleton nằm ở header mà nhiều `.so` cùng include, số instance do **dynamic linker** quyết định chứ không do C++ — xem [DP-020](#dp-020--concept---creational-linking-loading-symbol-interposition).
</details>

#### DP-020 · 🟠 · concept · ⭐ · 🎤 2026-08-21 · [→ creational](../../../11-design-patterns/creational.md), [linking-loading §symbol interposition](../../../07-shared-libraries/linking-loading.md#L100)
**Hai shared library cùng `#include` một header chứa Meyers' Singleton, app link cả hai. Trong tiến trình có bao nhiêu instance? Điều gì lật ngược được câu trả lời?**
<details><summary>Đáp án</summary>

**Cơ chế — bốn bước, hiểu bước 2 là hiểu cả câu:**

1. `static Logger inst;` trong hàm inline ⇒ compiler sinh **một symbol global (weak)** cho biến đó **trong mỗi** `.so` include header, cùng tên mangled.
2. ⭐ Dynamic linker nạp các `.so` vào **một** tiến trình, gặp symbol trùng tên ⇒ **symbol interposition**: hợp nhất về **một** định nghĩa duy nhất, cái xuất hiện trước trong thứ tự tìm kiếm thắng (cùng cơ chế `LD_PRELOAD` dùng).
3. Mọi lời gọi `Logger::instance()` đi qua **PLT/GOT** về cùng một địa chỉ.
4. ⇒ Mặc định trên Linux/ELF: **ĐÚNG MỘT instance.**

```
[0x7b465ec6c03c] count=1  (display)
[0x7b465ec6c03c] count=2  (sensor)     ← cùng địa chỉ, count đi tiếp
```

**"Vì sao" hai tầng:**
- *Tầng nông* (ai cũng nói được): *"Meyers' singleton đảm bảo một instance"* — **sai chỗ**: chuẩn C++ chỉ nói về **một chương trình**, còn "chương trình" gồm mấy `.so` là chuyện của **linker**, không phải của chuẩn.
- *Tầng sâu*: tính duy nhất ở đây **không do C++ bảo đảm mà do ELF/dynamic linker bảo đảm** ⇒ hễ đổi cách export symbol là kết quả đổi theo.

**Ba thứ lật ngược kết quả:**

| Thứ | Vì sao | Kết quả |
|---|---|---|
| **`-fvisibility=hidden`** khi build `.so` | Symbol không vào bảng động ⇒ không có gì để hợp nhất | **Mỗi `.so` một instance** |
| **`dlopen(..., RTLD_LOCAL)`** + symbol đã ẩn | Vùng symbol riêng, không nhập vào global scope | Mỗi lần nạp một instance |
| **Link tĩnh** logger vào từng `.so` | Mỗi `.so` mang sẵn một bản copy | Nhiều instance |

Chạy thật với `-fvisibility=hidden` (giấu `Logger`, chỉ export entry point):
```
[0x7498fbfc602c] count=1  (display)
[0x7498fbfc102c] count=1  (sensor)     ← địa chỉ KHÁC, count riêng
```

**Muốn CHẮC CHẮN một instance — đừng phó mặc may rủi:**
```cpp
// ✅ export có chủ đích, không phụ thuộc cờ build của người khác
class __attribute__((visibility("default"))) Logger { ... };
```
Chắc hơn nữa: **đặt logger vào MỘT `.so` riêng**, các lib khác cùng link vào — khi đó chỉ có đúng một định nghĩa tồn tại, không cần trông vào interposition.

**⚠️ Bẫy — hai cái, cái sau nặng hơn:**
1. Tưởng `-fvisibility=hidden` chỉ là tối ưu kích thước/tốc độ load. Nó **đổi ngữ nghĩa chương trình** với mọi trạng thái global: singleton, biến `static` trong hàm inline, và cả `typeinfo` (⇒ `dynamic_cast`/`catch` qua ranh giới `.so` có thể trượt).
2. 🔴 Tưởng "hai `.so`" nghĩa là "hai bên tách biệt" rồi nhảy sang **shared memory**. Sai tầng: hai `.so` nằm **cùng một tiến trình, cùng một không gian địa chỉ** — chúng đã dùng chung mọi địa chỉ. Shared memory giải bài toán **liên tiến trình**; ở đây nó không giải quyết gì mà còn thêm việc tự đồng bộ.

**Chốt:** *"Mặc định là MỘT — nhưng không phải nhờ C++, mà nhờ symbol interposition của dynamic linker. Hễ `-fvisibility=hidden`, `RTLD_LOCAL` hay link tĩnh xen vào là thành nhiều. Muốn chắc thì export có chủ đích, hoặc đẩy singleton vào một `.so` riêng."*
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

#### DP-037 · 🟡 · concept · 🎤 2026-09-09 · [→ creational §Abstract Factory](../../../11-design-patterns/creational.md)
**Factory Method và Abstract Factory khác nhau ở đâu? Dấu hiệu nào cho biết bạn cần cái thứ hai?**
<details><summary>Đáp án</summary>

Factory Method tạo **một** loại sản phẩm — một hàm ảo trả về object, lớp con quyết định class cụ thể. Abstract Factory tạo **cả một họ** sản phẩm phải khớp nhau — interface có **nhiều** `createX()`, mỗi concrete factory là một "biến thể nền tảng" trọn vẹn.

| Dấu hiệu | Cần |
|---|---|
| Chỉ một thứ cần tạo, client không nên biết class cụ thể | Factory Method |
| ≥ 2 thứ tạo ra **phải cùng một biến thể**, trộn nhầm là bug | **Abstract Factory** |

Giá trị thật của Abstract Factory không phải "gom cho gọn" mà là **biến ràng buộc nhất quán họ thành ràng buộc kiểu** — chỉ có một object factory, nên không còn tham số nào để truyền sai.
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

#### DP-038 · 🟡 · concept · 🎤 2026-09-09 · [→ structural §Bridge](../../../11-design-patterns/structural.md)
**Bridge pattern giải quyết vấn đề gì? Nó khác Strategy chỗ nào khi code trông giống hệt nhau?**
<details><summary>Đáp án</summary>

Bridge chống **bùng nổ lớp con** khi có **hai chiều biến thiên độc lập**: thay vì kế thừa cả hai trục (N × M lớp), giữ một trục ở cây kế thừa (abstraction) và đẩy trục kia ra một interface riêng (implementor) mà abstraction **giữ** — còn N + M lớp, ghép được N × M tổ hợp lúc runtime.

| | Strategy | Bridge |
|---|---|---|
| Số trục biến thiên | Một | **Hai** |
| Thứ được cắm vào | Thuật toán **thay thế được** | **Nửa còn lại** của cùng một thứ |
| Đổi lúc runtime | Có — mục đích chính | Hiếm; thường gắn lúc dựng |

Code gần như giống nhau (giữ con trỏ tới interface rồi ủy nhiệm); khác nhau ở **ý định**: Strategy tách *hành vi khỏi context*, Bridge tách *hai chiều biến thiên khỏi nhau*. Pimpl là một ứng dụng khác của cùng ý tưởng, phục vụ mục tiêu khác (giấu implementation, giữ ABI).
</details>

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

#### DP-034 · 🟡 · concept · 🎤 2026-09-09 · [→ behavioral §Command](../../../11-design-patterns/behavioral.md)
**Command pattern là gì? Nó mở ra những khả năng nào mà gọi hàm trực tiếp không có?**
<details><summary>Đáp án</summary>

Đóng gói một yêu cầu (**hành động + tham số**) thành một **object**, tách *người phát lệnh* khỏi *người thực thi*. Gọi hàm trực tiếp thì lệnh biến mất ngay khi chạy xong; biến nó thành dữ liệu thì lệnh **tồn tại độc lập với thời điểm thi hành**.

| Khả năng mở ra | Vì sao cần object |
|---|---|
| Xếp hàng / gửi qua IPC | Phải **tuần tự hoá được** |
| Hoãn thi hành ("áp dụng lúc T") | Tách thời điểm phát khỏi thời điểm chạy |
| Undo / redo | Lưu lịch sử + biết cách đảo ngược |
| Ghi log, replay, macro | Lệnh là dữ liệu, đọc lại được |

Trong embedded: hàng đợi lệnh giữa thread/process/core. Qua ranh giới process thì command phải là **POD phẳng có `version`** — struct đó là hợp đồng nhị phân y như vtable.
</details>

#### DP-035 · 🟡 · concept · 🎤 2026-09-09 · [→ behavioral §Template Method](../../../11-design-patterns/behavioral.md)
**Template Method và Strategy đều cho phép "thay đổi một phần hành vi". Chọn cái nào, dựa trên tiêu chí gì?**
<details><summary>Đáp án</summary>

| | Template Method | Strategy |
|---|---|---|
| Quan hệ | **Kế thừa** — lớp con *là* một biến thể | **Composition** — context *giữ* một strategy |
| Cái cố định | **Khung thuật toán** ở lớp cha | Chỉ interface |
| Cái thay đổi | Vài **bước hook** | Toàn bộ cách làm |
| Đổi lúc runtime | ❌ cố định lúc tạo object | ✅ gán strategy khác |
| Chi phí | Rẻ hơn (không object phụ) | Thêm một object + một lần gián tiếp |

**Tiêu chí chọn, một câu:** *"Có bao giờ cần đổi hành vi trên một object ĐANG SỐNG không?"* — Có ⟹ Strategy. Không, và các biến thể **chung khung, chỉ khác vài bước** ⟹ Template Method. Rủi ro của Template Method: lớp cha điều khiển luồng nên dễ vi phạm LSP nếu lớp con phá kỳ vọng của khung.
</details>

#### DP-036 · 🟡 · concept · ⭐ · 🎤 2026-09-09 · [→ behavioral §Null Object](../../../11-design-patterns/behavioral.md)
**Null Object pattern là gì? Nó đổi lấy điều gì, và khi nào KHÔNG nên dùng?**
<details><summary>Đáp án</summary>

Trả về một object **hợp lệ nhưng không làm gì** (báo trạng thái bằng mã lỗi) thay vì `nullptr`, để caller không phải kiểm ở mọi lời gọi.

| | Trả `nullptr` | Null Object |
|---|---|---|
| Quên kiểm | **Segfault** | Trả mã lỗi, log được |
| Phân biệt "không hỗ trợ" vs "lỗi thật" | Không | Có |

**Đổi lấy:** an toàn lấy *độ hiện* — lỗi không còn nổ ngay tại chỗ mà **im lặng trôi đi**. Vì vậy **không dùng** khi việc bỏ qua âm thầm là nguy hiểm (ghi cấu hình an toàn, lệnh điều khiển phần cứng phải chắc chắn tới nơi): ở đó *fail loud* mới đúng. Điều kiện dùng được: mã trả về **phải được kiểm và log**, nếu không Null Object chỉ đang giấu bug.
</details>

#### DP-039 · 🟡 · concept · 🎤 2026-09-09 · [→ behavioral §Memento](../../../11-design-patterns/behavioral.md)
**Memento pattern là gì? Điểm cốt lõi nhiều người bỏ sót khi mô tả nó?**
<details><summary>Đáp án</summary>

Chụp lại trạng thái nội bộ của một object để khôi phục sau — ba vai: **Originator** (biết tự chụp/tự nạp), **Memento** (bản chụp), **Caretaker** (giữ, nhưng **không đọc bên trong**).

**Điểm hay bị bỏ sót:** giá trị nằm ở chữ *"không phơi bên trong"*. Nếu caretaker tự đọc từng field rồi tự set lại thì mỗi setting mới phải sửa caretaker — encapsulation vỡ. Với Memento, thêm state mới chỉ sửa originator.

Memento cổ điển sống **trong một process, một phiên** (undo/redo). Muốn nó qua restart hoặc sang máy khác thì phải thêm ba thứ pattern không nói tới: **versioning**, **capability của thiết bị đích**, và **chính sách lỗi khi áp dụng dở dang**.
</details>

---

## E — 🎯 Case study hệ display (`libdisplay` + HAL bạn viết)

> Nguồn: [11-design-patterns/in-practice/](../../../11-design-patterns/in-practice/) — chia **🅰️ TIÊU CHUẨN** (`A1` hệ thật · `A2` ranh giới C++ interface) và **🅱️ CẢI TIẾN** (`B1` kiến trúc · `B2` sự kiện).
> ⚠️ **Định danh trong mục này là tên tài liệu, không phải tên thật** — bộ từ vựng chuẩn ở [in-practice/README](../../../11-design-patterns/in-practice/README.md).
> ⭐ Cả mục này bám resume, nên **xác suất bị hỏi rất cao**: interviewer đọc resume xong sẽ đi thẳng vào đây.
> 🧪 **5 bài lab NGỒI MÁY** cho domain này (Null Object · data race · `-rdynamic` · hai bài vtable ABI) sống ở [A2 §7](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md) cùng pack code hoàn chỉnh — **không chép lại ở đây**, vì chúng dính liền với mã nguồn.

#### DP-021 · 🟠 · concept · ⭐ · 🎤 2026-09-09 · [→ A1 §5](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md), [B1 §1](../../../11-design-patterns/in-practice/B1-redesign-architecture.md)
**Trong library display của bạn: dimming chia thành global / local / oled (cùng xuất phát từ một class gốc), còn video enhancement chia theo loại chip (cũng kế thừa từ một class gốc). Nhìn qua thì cả hai đều là "kế thừa + đa hình". Vì sao đây lại là HAI pattern khác nhau — và là hai cái nào?**
<details><summary>Đáp án</summary>

**Cơ chế — hỏi "trục biến thiên" trước khi gọi tên pattern.** Cùng là kế thừa, nhưng *cái gì thay đổi* khác nhau:

| | dimming | video enhancement |
|---|---|---|
| Cái thay đổi | **Thuật toán** trên cùng một nền (global: 1 giá trị · local: N zone theo histogram · oled: ABL + chống burn-in) | **Toàn bộ cách điều khiển** vì đổi chip: register map, command set, ràng buộc thời điểm |
| Số thứ đổi cùng lúc | **Một** — chỉ dimming | **Cả họ** — enhancer + dimming backend + frc của chip đó phải khớp nhau |
| Điểm quyết định | Có thể đổi **lúc runtime** (picture mode, nội dung) | **Một lần lúc boot**, theo board config |
| ⇒ Pattern | **Strategy** (behavioral) | **Abstract Factory** (creational) |

**"Vì sao" — tầng nông và tầng sâu:**
- *Nông (ai cũng nói được):* "một cái là hành vi, một cái là tạo object".
- *Sâu (phân biệt ứng viên):* **Abstract Factory tồn tại để bảo đảm tính NHẤT QUÁN CỦA HỌ.** Nếu chỉ cần "tạo object theo chip" thì vài hàm `makeX(ChipType)` rời là đủ. Cái mà factory rời **không** cho được: ngăn việc trộn enhancer của chip-A với dimming backend của chip-B — lỗi đó **compile sạch, chạy được**, chỉ sai trên đúng một board (xem DP-022). Abstract Factory biến ràng buộc đó thành ràng buộc **kiểu**, không còn phụ thuộc kỷ luật lập trình viên.

**Chúng KHÔNG loại trừ nhau — thường đi cùng nhau:**
```cpp
// Factory (tạo) đẻ ra Strategy (hành vi) rồi cắm vào context
std::unique_ptr<IDimming> ChipAFactory::createDimming() {
    return std::make_unique<LocalDimming>(std::make_unique<DimmingBackendChipA>());
}
```

**Bẫy:** trả lời "cả hai đều là Factory" hoặc "cả hai đều là Strategy". **Chốt:** *Factory trả lời **ai được tạo ra**, Strategy trả lời **nó cư xử thế nào** — hai câu hỏi khác nhau nên hai pattern khác nhau, và chúng xuất hiện cùng nhau chứ không thay nhau.*
</details>

#### DP-022 · 🔴 · design · ⭐ · 🏗️ · 🎤 2026-09-09 · [→ B1 §4](../../../11-design-patterns/in-practice/B1-redesign-architecture.md)
**Đồng nghiệp đề xuất: thay vì một `IPlatformFactory`, chỉ cần vài hàm rời — `makeEnhancer(ChipType)`, `makeDimming(ChipType)`, `makeFrc(ChipType)`. Ngắn hơn, không thêm class nào. Code compile sạch và chạy được trên bàn. Bạn phản đối bằng lý do gì? Nêu CỤ THỂ lỗi mà thiết kế đó cho phép xảy ra.**
<details><summary>Đáp án</summary>

**Cơ chế của lỗi — theo từng bước:**
1. Mỗi hàm nhận `ChipType` **độc lập** ⟹ không gì buộc ba lời gọi phải dùng **cùng một** giá trị.
2. Một chỗ khởi tạo (đường init khác, code nhánh product, một bản merge) truyền thiếu/sai `soc` cho **một** trong ba.
3. Kết quả: enhancer của chip-A chạy cùng dimming backend của chip-B.
4. Trình biên dịch **không thấy gì sai** — kiểu đều đúng. Linker cũng vậy. Test trên board đang dùng có thể vẫn đúng.

**Triệu chứng ngoài hiện trường:** sai màu / nhấp nháy / một feature im lặng không hoạt động, **trên đúng một model board**, không crash, không log. Đây là lớp bug đắt nhất: không phát hiện được ở CI, chỉ lộ ở giai đoạn tích hợp hoặc tệ hơn là ở khách.

**"Vì sao" tách tầng:**
- *Nông:* "Abstract Factory gom lại cho gọn / đỡ lặp `switch`".
- *Sâu:* nó **chuyển một ràng buộc từ kỷ luật sang hệ thống kiểu**. Với `IPlatformFactory`, "cả họ phải cùng một chip" **không thể vi phạm** — vì chỉ có một object factory, và nó *là* chip đó. Không còn tham số nào để truyền sai.

| | Vài hàm `makeX(ChipType)` | **`IPlatformFactory`** |
|---|---|---|
| Nhất quán họ | Do lập trình viên nhớ | **Do kiểu bảo đảm** |
| `switch (soc)` nằm ở đâu | Lặp trong **mỗi** hàm | **Một chỗ duy nhất** (`pickFactory`) |
| Thêm chip mới | Sửa **N** hàm | Thêm 1 class + 1 `case` |
| Truyền test double | Phải hook từng hàm | Thay **một** factory |

**Nhượng bộ công bằng (nói ra sẽ được cộng điểm, đừng bảo vệ cực đoan):** nếu chỉ có **một** loại sản phẩm cần tạo, Abstract Factory là thừa — Factory Method đủ. Nó chỉ trả công khi có **≥ 2 thứ phải khớp nhau**.

**Chốt:** *Vấn đề không phải dài dòng, mà là thiết kế đó **cho phép trộn nhầm họ** và lỗi ấy compile sạch. Abstract Factory làm cho lỗi đó không viết ra được.*
</details>

#### DP-023 · 🔴 · design · ⭐ · 🏗️ · 🎤 2026-09-09 · [→ A1 §5.3](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md), [structural §1](../../../11-design-patterns/structural.md)
**Bạn có 3 thuật toán dimming (global/local/oled) và 3 chip, mỗi chip ghi duty xuống phần cứng một kiểu khác nhau. Cách làm ngây thơ cho ra 9 class (`LocalDimmingChipA`, `OledDimmingChipB`…). Thiết kế lại thế nào? Và: pattern bạn dùng khác Strategy chỗ nào, khi code của chúng trông giống hệt nhau?**
<details><summary>Đáp án</summary>

**Cơ chế — Bridge: tách hai trục, nối bằng con trỏ.**
1. Nhận ra có **hai trục biến thiên độc lập**: *thuật toán* (N) và *đường xuống phần cứng* (M).
2. Kế thừa chỉ mô hình hoá được **một** trục; ép cả hai vào kế thừa ⟹ **N × M** lớp.
3. Giữ trục *thuật toán* ở cây kế thừa (**abstraction**), đẩy trục *phần cứng* ra một interface riêng (**implementor**), abstraction **giữ** một implementor.

```cpp
class IDimmingBackend {                       // IMPLEMENTOR — biến thiên theo chip
public:
    virtual ~IDimmingBackend() = default;
    virtual void writeDuty(int zone, int duty) = 0;
    virtual int  zoneCount() const             = 0;
};
class DimmingBase : public IDimming {         // ABSTRACTION — biến thiên theo thuật toán
protected:
    std::unique_ptr<IDimmingBackend> hw_;     // "cây cầu"
public:
    explicit DimmingBase(std::unique_ptr<IDimmingBackend> hw) : hw_(std::move(hw)) {}
};
// ghép lúc chạy: 3 + 3 = 6 lớp, phủ được 9 tổ hợp
auto d = std::make_unique<LocalDimming>(std::make_unique<DimmingBackendChipA>());
```

| | Kế thừa hai trục | **Bridge** |
|---|---|---|
| Số lớp | N × M = 9 | **N + M = 6** |
| Thêm 1 chip | +N lớp | **+1** |
| Thêm 1 thuật toán | +M lớp | **+1** |
| Tổ hợp mới | Viết lớp mới | **Ghép lúc runtime** |

**Bridge vs Strategy — code giống hệt, Ý ĐỊNH khác** *(đây là phần tính điểm)*:

| | Strategy | Bridge |
|---|---|---|
| Số trục | **Một** | **Hai** |
| Thứ được cắm vào là | Một **thuật toán thay thế được** | **Nửa còn lại** của cùng một thứ (dimming *cần* backend mới chạy được) |
| Đổi lúc runtime | Có — mục đích chính | Hiếm; thường gắn lúc dựng |
| Câu hỏi nó trả lời | *"Cư xử thế nào?"* | *"Làm sao tránh bùng nổ lớp con?"* |

**Bẫy:** trả lời "Bridge với Strategy giống nhau" rồi dừng ⟹ 2 điểm. **Chốt:** *Cả hai đều là giữ con trỏ tới interface rồi ủy nhiệm; khác nhau ở chỗ Strategy tách **hành vi khỏi context**, Bridge tách **hai chiều biến thiên khỏi nhau**.*
</details>

#### DP-024 · 🟡 · concept · ⭐ · 🎤 2026-09-09 · [→ B1 §7.1](../../../11-design-patterns/in-practice/B1-redesign-architecture.md)
**Trong library có 4 feature. Dimming và video enhancement đi qua interface + factory. Còn `frc` và `tcon` chỉ là một command cố định xuống chip, và bạn để nguyên hàm gọi thẳng. Vì sao không bọc chúng cho đồng bộ? Nêu cái giá cụ thể.**
<details><summary>Đáp án</summary>

Vì trừu tượng hoá phải **trả giá cho một biến thể đã tồn tại**, không phải cho một biến thể tưởng tượng. `frc`/`tcon` không có thuật toán, không state, không biến thể — bọc lại thì mua được **số không**.

Cái giá cụ thể, không nói chung chung:

| Chi phí | Cụ thể |
|---|---|
| Đọc hiểu | 3 file + 1 lần gián tiếp để phát hiện nó chỉ gọi **một** `ioctl` |
| Runtime | 1 object heap + 1 vtable + 1 virtual call cho mỗi lệnh |
| **ABI** | Thêm một virtual interface **qua ranh giới `.so`** = thêm một hợp đồng nhị phân phải giữ **vĩnh viễn** |
| Nhận thức | Người đọc sau đi tìm biến thể không tồn tại — abstraction **nói dối** về hệ thống |

**Phép thử trước khi trừu tượng hoá** — "không" ở bất kỳ câu nào thì đừng làm: ① đã có **≥ 2 biến thể thật** chưa? ② chúng khác nhau về **hành vi** hay chỉ khác **tham số**? ③ có ai cần **hoán đổi** chúng không? — `frc`/`tcon` trượt cả ba; `dimming` đạt cả ba.

**Chốt:** *Nói được chỗ mình **cố tình không** dùng pattern là tín hiệu senior mạnh hơn kể tên năm pattern — vì nó cho thấy bạn tính được cả cái giá, không chỉ cái lợi.*
</details>

#### DP-025 · 🟡 · concept · 🎤 2026-09-09 · [→ A2 §3.1](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md)
**Trong HAL của bạn, `IDisplayBuilder` chỉ có đúng một hàm `buildNewDisplayHandle()` trả về `IDisplay*`. Nó tên là "Builder" — nhưng theo GoF thì đó là pattern nào?**
<details><summary>Đáp án</summary>

Là **Factory Method**, không phải Builder. GoF Builder giải bài toán *object có nhiều tham số / dựng nhiều bước* — hình dạng của nó là nhiều setter nối chuỗi rồi `build()`. Ở đây chỉ **một lời gọi ra object hoàn chỉnh**, và **lớp con của factory** (nằm trong `.so`) quyết định tạo class nào — đúng định nghĩa Factory Method.

Rủi ro thật: nói *"em dùng Builder"* ⟹ interviewer hỏi ngay *"Builder khác Factory thế nào?"* ⟹ mô tả không khớp code vừa kể. Cách nói an toàn: *"trong codebase nó tên `IDisplayBuilder`, nhưng đúng tên GoF là Factory Method — nó tạo trong một lời gọi chứ không dựng từng bước; `Builder` là quy ước nội bộ có sẵn."*
</details>

#### DP-026 · 🟠 · concept · ⭐ · 🎤 2026-09-09 · [→ A2 §3.2](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md)
**Đọc đoạn này (trích từ HAL bạn viết). Nó có vấn đề gì khi hai thread cùng gọi lần đầu? Và vì sao chữ `static` ở dòng đầu không cứu được?**
```cpp
IDisplay* IDisplay::getInstance() {
    static IDisplay* p_instance = nullptr;
    if (p_instance == nullptr) {
        loadlib(LIB_PATH);
        if (builder) p_instance = builder->buildNewDisplayHandle();
        else         p_instance = new (std::nothrow) IDisplay();
    }
    return p_instance;
}
```
<details><summary>Đáp án</summary>

**Cơ chế — vì sao `static` không cứu:** bảo đảm thread-safe của C++11 ("magic statics") chỉ áp cho **việc khởi tạo** biến static. Ở đây khởi tạo là `= nullptr` — một **constant initialization**, compiler **không sinh guard variable** nào cả. Toàn bộ `if` + phép gán bên dưới là **code thường**, không được bảo vệ. Đây là mẫu **check-then-act** kinh điển: thread A đọc `p_instance` (còn null) trong lúc thread B đang ghi.

**Bằng chứng đo được** (TSan, `-fsanitize=thread`, 4 thread):
```
WARNING: ThreadSanitizer: data race
  Read of size 8 ... IDisplay::getInstance()      (dòng `if`)
  Previous write of size 8 ... IDisplay::getInstance()  (dòng gán)
  Location is global 'IDisplay::getInstance()::p_instance'
```

**Hậu quả cụ thể:** `buildNewDisplayHandle()` chạy **hai lần** ⟹ hai `DisplayImpl`, mỗi cái `open("/dev/display_dev")` riêng ⟹ **rò một fd** + hai object cùng điều khiển một thiết bị. (`dlopen` gọi hai lần thì vô hại — nó idempotent, chỉ tăng refcount.)

**Sửa — để magic statics làm việc, đừng tự viết khoá:**
```cpp
// ✅ static local có KHỞI TẠO ĐỘNG ⟹ compiler sinh guard variable
static IDisplay* p_instance = [] () -> IDisplay* {
    loadlib(LIB_PATH);
    if (builder) return builder->buildNewDisplayHandle();
    return new (std::nothrow) IDisplay();
}();
return p_instance;
```
Kiểm chứng: TSan im lặng, và `g++ -S | grep cxa_guard` cho thấy `__cxa_guard_acquire/release/abort` — guard có thật. Sau lần đầu chỉ còn **một atomic load**, gọi triệu lần vẫn rẻ.

**Bẫy:** nghĩ "có `static` là an toàn". **Chốt:** *magic statics bảo vệ **khởi tạo**, không bảo vệ **logic bạn viết quanh nó** — và cũng không bảo vệ các method của object sau đó.*
</details>

#### DP-027 · 🔴 · concept · ⭐ · 🎤 2026-09-09 · [→ A2 §3.3](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md), [abi-versioning](../../../07-shared-libraries/abi-versioning.md)
**Bạn thêm API mới `virtual int setBrightness(int)` vào `IDisplay`, đặt TRƯỚC `setPower` cho gọn nhóm. Rebuild app, giữ nguyên `libdisplay.so` cũ. Chạy: dòng log của `setPower` biến mất, thay vào đó destructor chạy, exit code 0, không crash. Giải thích chuyện gì đã xảy ra và rút ra luật.**
<details><summary>Đáp án</summary>

**Cơ chế — vtable là một mảng, lời gọi virtual là một CHỈ MỤC vào mảng đó.** Compiler dịch `p->setPower(x)` thành *"nhảy tới slot số k"*, với `k` **đóng băng vào binary lúc biên dịch**.

Vtable thật do gcc sinh (`-fdump-lang-class`) cho `DisplayImpl` **cũ**:
```
16    DisplayImpl::setPower        ⬅️ slot hàm 0
24    DisplayImpl::~DisplayImpl    ⬅️ slot hàm 1 (complete dtor)
32    DisplayImpl::~DisplayImpl    ⬅️ slot hàm 2 (deleting dtor)
```

| slot hàm | `.so` **cũ** (không đổi) | App **mới** (header đã chèn `setBrightness`) |
|---|---|---|
| 0 | `setPower` | `setBrightness` |
| 1 | `~DisplayImpl` | **`setPower`** |
| 2 | `~DisplayImpl` | `~IDisplay` |

App gọi `setPower` ⟹ nhảy slot 1 ⟹ ở object đến từ `.so` cũ, slot 1 là **destructor** ⟹ destructor chạy. Hai bên đang dùng **hai bản đồ vtable khác nhau**.

**"Vì sao" tách tầng:**
- *Nông:* "đổi interface thì phải build lại `.so`".
- *Sâu:* **không có gì phát hiện được lỗi này.** Không symbol nào thiếu ⟹ link sạch, `dlopen` sạch, không cảnh báo. Nó chỉ là một **số nguyên** sai. Triệu chứng ngoài hiện trường: *"board này bật panel không lên"* — không ai nghĩ tới vtable.

**Luật sống với virtual interface qua `.so`:**

| Việc | Được? | Vì sao |
|---|---|---|
| **Append** virtual mới ở **cuối** | ✅ | Chỉ mục cũ giữ nguyên; `.so` cũ chưa override thì rơi về bản base |
| Chèn virtual vào **giữa** | ❌ | Dịch chỉ mục mọi slot sau nó |
| **Xoá** / đổi thứ tự virtual | ❌ | Như trên |
| Thêm **data member** vào interface | ❌ | Đổi `sizeof`/layout — `.so` cũ dựng object sai kích thước |
| Đổi chữ ký (kể cả thêm tham số mặc định) | ❌ | Đổi mangled name / cách truyền tham số |

**Mở rộng đáng giá:** **bảng con trỏ hàm** mà kernel driver dùng để dispatch là *cùng một ý tưởng, viết tay bằng C* — và dính **đúng luật này**: chèn một entry vào giữa struct ops thì mọi driver biên dịch trước đó gọi nhầm hàm.

**Chốt:** *Qua ranh giới `.so`, **vtable là ABI**. API mới chỉ được **thêm vào cuối**. Đây cũng là lý do tồn tại của Pimpl và của biên giới `extern "C"`.*
</details>

#### DP-028 · 🟠 · concept · 🎤 2026-09-09 · [→ A2 §2.1](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md)
**Trong HAL của bạn, các API của `IDisplay` là `virtual` thường chứ không phải pure virtual, và bản base trả `-ENOTSUP`; khi không nạp được `.so` thì `getInstance()` trả về một object `IDisplay` dummy thay vì `nullptr`. Đó là pattern gì, và nó mua được HAI thứ nào?**
<details><summary>Đáp án</summary>

**Null Object pattern** — trả về một object **hợp lệ nhưng không làm gì**, báo trạng thái bằng mã lỗi, thay vì trả `nullptr`.

**Thứ nhất — an toàn ở phía caller:**

| | Trả `nullptr` | **Null Object** |
|---|---|---|
| Caller | Phải kiểm ở **mọi** lời gọi | Gọi thẳng |
| Quên kiểm | **Segfault** | Trả `-ENOTSUP`, log được |
| Phân biệt "không hỗ trợ" vs "lỗi thật" | Không | Có (mã lỗi riêng) |

**Thứ hai — tương thích tiến, NHƯNG phải nói chính xác ở mức nào.** Vì API là virtual **thường**, implementation **không phải sửa code** khi interface thêm API: `DisplayImpl` không override `setBrightness()` thì rơi về bản base trả `-ENOTSUP`. Với `= 0` thì mỗi API mới **bắt buộc mọi impl cập nhật cùng lúc** — bất khả thi khi các board release lệch nhau.

⚠️ **Đó là tương thích ở mức MÃ NGUỒN, KHÔNG phải mức NHỊ PHÂN** — đây là chỗ phân biệt câu trả lời đúng với câu trả lời nghe có vẻ đúng:

| | Tương thích **nguồn** | Tương thích **nhị phân** |
|---|---|---|
| Nghĩa | `.so` **biên dịch lại** với header mới mà không sửa dòng nào | `.so` **đã biên dịch từ trước** dùng luôn được |
| virtual-không-pure cho không? | ✅ Có | ❌ **Không** |
| Vì sao | Base có sẵn cài đặt mặc định | Vtable `.so` cũ **ngắn hơn** — không có slot cho API mới |

**Đo thật:** app build header v2 gọi `setBrightness()` trên object từ `.so` **v1** ⟹ nhảy vào slot 3 của một vtable chỉ có 3 slot hàm ⟹ **segfault**, *không* phải `-ENOTSUP`.

**Bẫy (rất phổ biến):** tin rằng "virtual không-pure ⟹ `.so` cũ tự rơi về base". Sai — vtable của `DisplayImpl` do **`.so`** phát ra, biên dịch từ header cũ.

**Chốt:** *Null Object đổi lỗi `nullptr` lấy lỗi có-mã-trả-về, và cho tương thích **nguồn**. Muốn gọi API mới an toàn trên `.so` cũ thì phải có **cơ chế hỏi khả năng** (version/capability) tồn tại **từ v1** — thêm virtual luôn là ABI break.*
</details>

#### DP-029 · 🟠 · design · 🏗️ · 🎤 2026-09-09 · [→ A2 §4](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md), [linking-loading](../../../07-shared-libraries/linking-loading.md)
**HAL của bạn: `.so` có một hàm `__attribute__((constructor))`, khi được `dlopen` thì tự gọi ngược lên app để đăng ký factory của mình. Vì sao làm vòng vèo vậy thay vì link thẳng `.so` vào app? Đánh đổi là gì — và điều kiện kỹ thuật nào khiến `.so` gọi được symbol nằm trong app?**
<details><summary>Đáp án</summary>

**Cơ chế (self-registration / Inversion of Control), theo bước:** `dlopen` ⟹ loader chạy hàm constructor của `.so` ⟹ hàm đó gọi `IDisplay::injectBuilder(new DisplayBuilderImpl)` — một symbol nằm trong **app** ⟹ con trỏ builder static trong app khác NULL ⟹ app gọi builder để lấy object thật. Interface **không hề biết** có bao nhiêu implementation tồn tại; thêm impl mới = thêm một `.so`, **không sửa một dòng nào** của interface.

**Điều kiện kỹ thuật** (câu hỏi đuổi hay gặp): executable phải **xuất symbol** của nó ra cho `.so` phân giải — `-rdynamic`, trong CMake là `ENABLE_EXPORTS ON`. Thiếu nó thì `dlopen` vẫn thành công nhưng `.so` không resolve được `injectBuilder` ⟹ hỏng **lúc runtime**, không phải lúc link.

**Đánh đổi:**

| | Link thẳng | **`dlopen` + self-registration** |
|---|---|---|
| Chọn implementation | Lúc **link**, cố định trong binary | Lúc **boot**, theo board config |
| Một binary nhiều board | ❌ mỗi board một bản | ✅ **một bản** |
| Thiếu `.so` | Chết ngay lúc nạp | ✅ chạy tiếp ở chế độ Null Object |
| Thiếu/sai symbol | Bắt lúc **link** | 🔴 chỉ lộ lúc **runtime** |
| ABI | Thường build cùng lúc | 🔴 **phải giữ thủ công** (DP-027) |
| Debug | Thẳng | Khó hơn — breakpoint trong `.so` chưa nạp là *pending* |

**Chốt:** *Đổi **an toàn lúc link** lấy **linh hoạt lúc boot**. Đáng đổi khi một binary phải phục vụ nhiều board; không đáng khi chỉ có một cấu hình phần cứng.*
</details>

#### DP-030 · 🟠 · design · ⭐ · 🏗️ · 🎤 2026-09-09 · [→ B2 §1](../../../11-design-patterns/in-practice/B2-redesign-events.md)
**Bạn làm adaptive brightness: cảm biến ánh sáng phát sự kiện theo Observer, module brightness đăng ký nhận `onLux(lux)` rồi gọi `setBrightness`. Pattern dùng đúng sách. Nhưng khi ánh sáng thay đổi nhanh, màn hình nhấp nháy thấy rõ. Pattern sai chỗ nào — và bạn thêm gì?**
<details><summary>Đáp án</summary>

**Pattern không sai — nó chỉ không giải bài toán đó.** Observer giải vấn đề **cấu trúc**: ai gọi ai, làm sao sensor không gắn cứng với module brightness. Nhấp nháy là vấn đề **miền**: giá trị lux dao động liên tục (người đi qua, mây, đèn chớp), nối thẳng `onLux → setBrightness` là đuổi theo từng mẫu.

**Đường ống cần đặt SAU Observer — bốn tầng, mỗi tầng chống một thứ khác nhau:**

| Tầng | Chống điều gì | Cơ chế |
|---|---|---|
| ① Lọc | Nhiễu mẫu đơn lẻ | Trung bình trượt / EMA trên N mẫu |
| ② **Hysteresis** | Dao động quanh **ngưỡng** | Hai biên khác nhau: lên ở 120 lux, xuống ở 80 lux |
| ③ Giới hạn tốc độ | Nhảy bậc thấy được | Ramp: ≤ X nits/giây |
| ④ Chống lặp | Ghi thừa xuống HW | Bỏ qua nếu đích không đổi |

⭐ **Tầng ② là tầng hay bị quên nhất và là tầng quan trọng nhất.** Chỉ lọc trung bình thì vẫn nhấp nháy khi lux đứng **ngay tại ngưỡng chuyển** — mỗi mẫu lại lật qua lật lại. Phải để **ngưỡng lên ≠ ngưỡng xuống**.

Hysteresis là một **máy trạng thái nhỏ**, và ở đây **enum + ngưỡng** là đủ — không cần State pattern OOP: nó nằm trên đường sự kiện tần suất cao, cần tất định, không cấp phát.

**Rủi ro riêng của Observer, hỏi kèm:** observer bị huỷ mà quên `unsubscribe` ⟹ subject gọi vào con trỏ chết. Giữ `weak_ptr` trong danh sách và tự dọn khi `lock()` fail.

**Chốt:** *Observer mua **cấu trúc**, không mua **tính đúng đắn của miền**. Câu hỏi T2 luôn nằm ở nửa sau: "pattern **không** giải quyết được gì, và bạn xử lý phần đó thế nào".*
</details>

#### DP-031 · 🟠 · design · 🏗️ · 🎤 2026-09-09 · [→ B2 §2](../../../11-design-patterns/in-practice/B2-redesign-events.md)
**Bạn đồng bộ độ sáng giữa nhiều S-Box qua POSIX message queue — mỗi message là một Command đã tuần tự hoá. Nhưng người xem vẫn thấy các panel đổi lệch nhau thành lưới. Thiếu gì trong message? Và vì sao struct đó bắt buộc phải có trường `version`?**
<details><summary>Đáp án</summary>

**Cơ chế của lỗi:** mỗi unit nhận message ở **thời điểm khác nhau** (lịch scheduler, tải, thứ tự đọc queue). Nếu ngữ nghĩa là *"đổi ngay khi nhận"* thì độ lệch giữa các unit chính là độ lệch nhận message ⟹ mắt thấy lưới.

**Sửa: đổi ngữ nghĩa từ "ngay" thành "tại mốc T".**
```cpp
struct BrightnessCmd {
    uint16_t version;      // hợp đồng nhị phân — xem dưới
    uint16_t opcode;       // SET_NITS / RAMP_TO
    uint32_t seq;          // chống lặp + phát hiện mất gói
    int32_t  value;
    uint64_t apply_at_ms;  // ⬅️ MẤU CHỐT: áp dụng tại mốc này, không phải khi nhận
};
```
Đây đúng là một lý do gốc Command pattern tồn tại: **hoãn thực thi** — tách *thời điểm phát lệnh* khỏi *thời điểm thi hành*. (Kèm điều kiện: đồng hồ các unit phải cùng gốc — dùng clock đơn điệu chung / đồng bộ thời gian; và `apply_at_ms` phải đủ xa để unit chậm nhất kịp nhận.)

**Vì sao bắt buộc có `version`:** struct đi qua ranh giới process là **hợp đồng nhị phân**, y hệt vtable qua `.so` (DP-027). Các unit **không update cùng lúc** — firmware lệch phiên bản là chuyện bình thường trên video wall.

| Việc | Được? |
|---|---|
| Thêm field vào **cuối** + tăng `version`, bên nhận đọc theo `version` | ✅ |
| Chèn field vào **giữa** | ❌ lệch offset, unit cũ đọc rác |
| Đổi kiểu field (`int32`→`int64`) | ❌ |
| Giả định `sizeof` giống nhau hai bên | ❌ padding/alignment có thể khác |

**Chốt:** *Cứ chỗ nào hai binary build riêng phải hiểu nhau — vtable qua `.so`, struct qua message queue, bảng con trỏ hàm giữa HAL và driver — thì layout là ABI, và luật luôn giống nhau: **chỉ được thêm vào cuối**.*
</details>

#### DP-032 · 🟡 · concept · 🎤 2026-09-09 · [→ B2 §3](../../../11-design-patterns/in-practice/B2-redesign-events.md)
**Feature "Preset" (lưu cấu hình màn hình, khôi phục sau restart, export sang màn hình khác) về bản chất là Memento. Memento sách vở thiếu ba thứ gì cho yêu cầu "export sang màn hình khác"?**
<details><summary>Đáp án</summary>

Memento cổ điển sống **trong một process, một phiên chạy** (undo/redo): originator tự chụp/tự nạp, caretaker giữ mà không đọc bên trong. Export sang máy khác đẻ thêm ba vấn đề pattern không nói tới:

1. **Versioning** — preset lưu bởi app 1.0, mở bằng 1.2: field mới thiếu thì lấy mặc định; đánh version và chỉ thêm field vào cuối (cùng luật ABI của DP-027/DP-031).
2. **Capability của thiết bị đích** — màn hình kia không hỗ trợ một setting, hoặc có dải giá trị khác: áp dụng từng phần + báo cáo cái bỏ qua (đúng tinh thần `-ENOTSUP` của Null Object, DP-028).
3. **Tính nguyên tử** — áp 10 setting, cái thứ 7 hỏng: dừng và rollback, hay đi tiếp và báo cáo? Không có đáp án đúng duy nhất; **có một quyết định rõ ràng và nhất quán** mới là câu trả lời.

**Chốt:** *Memento mua được **encapsulation** (caretaker không cần biết bên trong); nó không mua được **tính khả chuyển** — phần đó là versioning + capability + chính sách lỗi.*
</details>

#### DP-033 · 🔴 · concept · ⭐ · 🎤 2026-09-09 · [→ A2 §3.3 + §7 Lab 3a](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md)
**Bạn đã thêm `IDisplayBuilder::abiVersion()` để app hỏi `.so` xem nó build với header phiên bản nào, và app chỉ gọi API mới khi version đủ. Một hôm app in ra `impl ABI version = 2 (interface = 2)` — khớp hoàn toàn — nhưng gọi `setBrightness()` thì destructor chạy và hàm trả về số rác. Vì sao cơ chế version không cứu được?**
<details><summary>Đáp án</summary>

**Cơ chế:** version chỉ nói *"tôi biết bao nhiêu API"*, nó **không mô tả BỐ CỤC** của những API đó. Ở đây `.so` được build từ header có `setBrightness` khai báo **sau** destructor, còn app build từ header có nó **trước** destructor. Cùng gọi là "v2", cùng số lượng API — nhưng **thứ tự slot vtable khác nhau**.

```
slot ham   |  .so (v2, dung thu tu)   |  app (v2, chen TRUOC dtor)
   0       |  setPower                |  setPower
   1       |  ~DisplayImpl  (complete)|  setBrightness   <-- app nhay vao day
   2       |  ~DisplayImpl  (deleting)|  ~IDisplay (complete)
   3       |  setBrightness           |  ~IDisplay (deleting)
```
App dịch `setBrightness` thành *"nhảy slot 1"*; slot 1 của `.so` là **destructor (bản complete)** ⟹ destructor chạy, giá trị trả về là rác. *(Destructor chiếm **hai** slot liền nhau — complete và deleting — tại đúng vị trí nó được khai báo; đó là lý do chèn virtual trước nó làm lệch cả hai.)*

**"Vì sao" tách tầng:**
- *Nông:* "phải build lại `.so`".
- *Sâu:* **version bảo vệ được API THIẾU, không bảo vệ được slot BỊ ĐẢO.** Hai lớp lỗi khác nhau:

| Lớp lỗi | Cơ chế nào chặn được |
|---|---|
| `.so` cũ **thiếu** API mới | ✅ Version / capability check |
| `.so` có đủ API nhưng **thứ tự slot khác** | ❌ Không cơ chế runtime nào — chỉ **kỷ luật review** |
| Đổi chữ ký / layout data member | ❌ Như trên |

**Triệu chứng đặc trưng, đáng nhớ:** hỏng **có chọn lọc** — `setPower` (slot 0) vẫn chạy đúng, chỉ hàm nằm sau chỗ chèn mới lệch; **không crash, exit 0**. Ngoài hiện trường đọc như *"hầu hết mọi thứ vẫn chạy, riêng một feature im lặng"*.

**Cách phòng thật sự:** ① luật review *"virtual mới chỉ được thêm sau destructor, ở cuối cùng"* ② kiểm tự động trong CI bằng `abidiff`/`abi-compliance-checker` so vtable giữa hai bản ③ hoặc tránh hẳn bằng biên `extern "C"` + struct ops có version.

**Chốt:** *Version check là **hợp đồng về NỘI DUNG**; vtable là **hợp đồng về BỐ CỤC**. Cái thứ hai không tự kiểm tra được lúc chạy — nên nó phải được bảo vệ ở lúc review và lúc build.*
</details>

---

⬅️ [Bank index](README.md)
