# Essential Training — Design Pattern trong việc THẬT của bạn

> **TL;DR**
> - Bản training **chỉ gồm những gì bạn đã làm và đã thấy** trong hai tài liệu chuẩn: [A1 — `libdisplay`](in-practice/A1-baseline-libdisplay.md) và [A2 — C++ interface/HAL](in-practice/A2-cpp-interface-hal.md). Không học dàn đều 23 pattern GoF; không dùng bản cải tiến B1/B2.
> - **9 pattern + 1 nền**, xếp theo thứ tự học: §0 **interface & đa hình** (nền của mọi thứ, kể cả bảng con trỏ hàm trong kernel) → §1 Strategy → §2 Bridge → §3 Factory Method → §4 Abstract Factory → §5 Singleton → §6 Null Object → §7 Facade (+ Adapter) → §8 Self-registration → §9 Service Locator (mức nhận diện).
> - Mỗi pattern đi theo đúng một khuôn: **một câu** → **vấn đề nó giải** → **ví dụ cơ bản** (code chạy được) → **trong công việc của bạn** → **cái giá & bẫy**.
> - §10 là thứ đáng giá không kém: **chỗ bạn cố ý KHÔNG dùng pattern** (Panel Control). §11 là bản đồ một trang để ôn trước giờ phỏng vấn.
> - Mọi code ở phần *ví dụ cơ bản* đã compile và chạy thật (`g++ -std=c++17 -Wall -Wextra`, gcc 11.4); output in ngay dưới code.

> 🔒 **Tên trong tài liệu này là tên đã khử nhạy cảm** (`libdisplay`, `lib_api_*`, `IDimmingAlgo`, `panel_ops`…), giống A1/A2. Khi kể ở phỏng vấn cũng dùng đúng các tên này, hoặc nói chung chung *"một shared library điều khiển màn hình"*.

---

## Cách dùng tài liệu này

| Bạn muốn | Đọc |
|---|---|
| Học từ đầu | Đi theo thứ tự §0 → §11. Mỗi mục khoảng 10 phút |
| Ôn nhanh trước phỏng vấn | §11 (bản đồ một trang) + phần *"Trong công việc"* của từng mục |
| Đào sâu một chỗ | Theo link sang A1/A2 (bối cảnh đầy đủ) hoặc sang [creational](creational.md) / [structural](structural.md) / [behavioral](behavioral.md) (lý thuyết chung) |
| Tự kiểm tra | Bảng *"Câu hỏi phỏng vấn liên quan"* cuối file — đáp án ở bank, tự trả lời trước khi mở |

**Một câu hỏi xuyên suốt cả tài liệu:** *"cái gì ở đây sẽ thay đổi, và tôi cô lập nó ở đâu?"* Mỗi pattern bên dưới là một câu trả lời cho câu hỏi đó, ở một chỗ khác nhau.

---

## §0. Nền: interface và đa hình — thứ mọi pattern dựng lên

**Một câu:** code phía trên chỉ biết một **interface**; loại cụ thể được **cắm vào sau**, và lời gọi tự chạy tới đúng hàm của loại đó.

**Vì sao là nền:** 8 trong 9 pattern bên dưới đều là *"giữ một con trỏ tới interface rồi gọi qua nó"*. Khác nhau chỉ ở **ý định**: ai tạo object, cái gì được cắm vào, cắm lúc nào. Hiểu chắc §0 thì phần còn lại là phân biệt ý định.

**Ví dụ cơ bản — C++:**
```cpp
struct ISensor { virtual ~ISensor() = default; virtual int read() = 0; };
struct TempSensor  : ISensor { int read() override { return 25; } };
struct LightSensor : ISensor { int read() override { return 300; } };

void report(ISensor& s) { std::printf("read = %d\n", s.read()); }   // không biết loại cụ thể
```
**Cùng ý tưởng — C thuần, không có `virtual`:** tự dựng bảng con trỏ hàm.
```c
struct sensor_ops { int (*read)(void); };            /* "interface" viết tay */
static const struct sensor_ops light_ops = { light_read };
ops->read();                                          /* "gọi ảo" */
```
```
read = 25
read = 300
C ops->read() = 300
```

**Bên dưới `virtual` là gì:** mỗi class có đa hình có một **vtable** — bảng con trỏ hàm do compiler dựng. Mỗi object mang một con trỏ ẩn (vptr) trỏ tới vtable của class nó. `s.read()` dịch thành *"lấy vptr, đọc ô số k, nhảy tới đó"*. Bảng `sensor_ops` ở trên chính là vtable **viết tay**.

**Trong công việc của bạn — xuất hiện ở cả hai tầng:**

| Tầng | Cơ chế | Ở đâu |
|---|---|---|
| User-space C++ | `virtual` — vtable do compiler dựng | `IDimmingAlgo`, `IDimmingBackend` ([A1 §5.3](in-practice/A1-baseline-libdisplay.md)), `IDisplay` ([A2 §2](in-practice/A2-cpp-interface-hal.md)) |
| Kernel C | `panel_ops` — vtable viết tay | Driver nền gọi `ops->set_freq()`, driver chip điền bảng ([A1 §6](in-practice/A1-baseline-libdisplay.md)) |

**Cái giá & bẫy — điều quan trọng nhất của cả tài liệu:**
- 🔴 **Vtable là ABI.** Binary đã compile chỉ nhớ **số thứ tự ô**, không nhớ tên hàm. Chèn một hàm vào **giữa** interface (hay giữa `panel_ops`) thì mọi ô phía sau lệch ⟹ gọi hàm này mà hàm khác chạy, **không crash, không log**. Đã đo thật: [A2 §3.3](in-practice/A2-cpp-interface-hal.md) (gọi `setPower` mà destructor chạy) và bank [DP-043](../14-prep/mock-interview/bank/design-patterns.md) (gọi `set_freq` mà `set_brightness` chạy).
- Hình ảnh dễ nhớ: ***API là hợp đồng bằng TÊN, ABI là hợp đồng bằng VỊ TRÍ*** — như tủ có ngăn đánh số; chèn ngăn mới vào giữa thì "ngăn số 2" chứa thứ khác.
- Luật sống: thêm hàm **chỉ ở cuối**, và phải có cách hỏi bên kia *"biết tới phiên bản nào"* ([A2 §2.1](in-practice/A2-cpp-interface-hal.md)).

---

## §1. Strategy — thay thuật toán mà không sửa nơi dùng

**Một câu:** đóng gói mỗi thuật toán sau một interface chung; **context** giữ một "khe cắm" và uỷ nhiệm vào đó, nên đổi thuật toán không phải sửa context.

**Vấn đề nó giải:** một chỗ trong code có **nhiều cách làm cùng một việc**, và cách nào được dùng tuỳ cấu hình. Không có Strategy thì context đầy `if/switch`; thêm cách thứ tư là sửa vào file đang chạy tốt.

**Ví dụ cơ bản — bộ lọc mẫu cảm biến:**
```cpp
struct IFilter { virtual ~IFilter() = default; virtual int apply(int raw) = 0; };
struct NoFilter : IFilter { int apply(int raw) override { return raw; } };
struct Ema : IFilter {                                // làm mượt: mỗi lần tiến 1/4 khoảng cách
    int last = 0;
    int apply(int raw) override { last = last + (raw - last) / 4; return last; }
};

class SensorReader {                                  // CONTEXT — không đổi khi thêm bộ lọc
    std::unique_ptr<IFilter> filter_;                 // khe cắm
public:
    explicit SensorReader(std::unique_ptr<IFilter> f) : filter_(std::move(f)) {}
    void setFilter(std::unique_ptr<IFilter> f) { filter_ = std::move(f); }
    int sample(int raw) { return filter_->apply(raw); }
};

SensorReader r(std::make_unique<NoFilter>());
r.sample(400);                                        // 400
r.setFilter(std::make_unique<Ema>());                 // đổi thuật toán lúc chạy
int a = r.sample(400);                                // 100
int b = r.sample(400);                                // 175
```
> ⚠️ Bản đầu của ví dụ này viết `printf("%d %d", r.sample(400), r.sample(400))` và in ra **`175 100`**: thứ tự đánh giá đối số là *unspecified*, gcc chọn phải sang trái. Gọi hàm có tác dụng phụ thì tách câu lệnh.

**Trong công việc của bạn — `IDimmingAlgo`** ([A1 §5.5](in-practice/A1-baseline-libdisplay.md)):

| Vai Strategy | Trong `libdisplay` |
|---|---|
| Context | `lib_dimming` — chỉ uỷ nhiệm, không biết thuật toán nào |
| Interface | `IDimmingAlgo` |
| Các thuật toán | `GlobalDimming` · `LocalDimming` · `OLEDDimming` |
| Chọn khi nào | Theo **model** (loại đèn nền của panel), lúc khởi tạo |

```cpp
uint32_t lib_dimming::SetAmbientMode(int32_t mode) {
    return m_pDimmingPanel->SetAmbientMode(mode);    // chỉ uỷ nhiệm — A1 §5.7
}
```
Mua được gì: thêm một thuật toán dimming **không sửa** `lib_dimming` hay mặt tiền `lib_api_*`.

**Cái giá & bẫy:**
- Một tầng gián tiếp + một object phụ. Với lời gọi cấu hình thì miễn phí.
- **Đừng dùng khi chỉ có một thuật toán.** `lib_dimming_interface → lib_dimming` ở tầng trên chỉ có **một** implementation, nên đó **chưa phải** Strategy mà là DIP + uỷ nhiệm ([A1 §5.7](in-practice/A1-baseline-libdisplay.md)).
- C++ hiện đại: thuật toán **thuần hàm** (không state) thì `std::function` + lambda gọn hơn cây class; thuật toán **có state** như dimming (bộ lọc, lịch sử vùng) thì class là đúng. Lý thuyết đầy đủ: [behavioral §1](behavioral.md).

---

## §2. Bridge — tách HAI chiều biến thiên khỏi nhau

**Một câu:** khi một thứ thay đổi theo **hai trục độc lập**, giữ một trục ở cây kế thừa (**abstraction**), đẩy trục kia ra một interface riêng (**implementor**), và abstraction **giữ con trỏ** tới implementor.

**Vấn đề nó giải:** kế thừa chỉ mô hình được **một** trục. Ép cả hai vào kế thừa thì ra N × M lớp, và phần logic bị **chép lại** trong từng lớp.

**Ví dụ cơ bản — quạt: logic điều khiển × chip PWM:**
```cpp
struct IPwm {                                         // IMPLEMENTOR — nửa phần cứng, theo chip
    virtual ~IPwm() = default;
    virtual void setDuty(int percent) = 0;
};
struct PwmChipA : IPwm { void setDuty(int p) override { printf("chipA reg <- %d\n", p); } };
struct PwmChipB : IPwm { void setDuty(int p) override { printf("chipB reg <- %d\n", p * 255 / 100); } };

class Fan {                                           // ABSTRACTION — nửa logic
protected:
    IPwm& pwm_;                                       // ⬅️ cây cầu sang phần cứng
public:
    explicit Fan(IPwm& p) : pwm_(p) {}
    virtual ~Fan() = default;
    virtual void onTemp(int c) = 0;
};
struct StepFan   : Fan { using Fan::Fan; void onTemp(int c) override { pwm_.setDuty(c > 60 ? 100 : 30); } };
struct SmoothFan : Fan { using Fan::Fan;
    void onTemp(int c) override { pwm_.setDuty(c < 30 ? 0 : (c > 80 ? 100 : (c - 30) * 2)); } };

PwmChipA a; PwmChipB b;
StepFan(a).onTemp(70);                                // logic bậc thang trên chip A
SmoothFan(b).onTemp(55);                              // logic tuyến tính trên chip B
```
```
chipA reg <- 100
chipB reg <- 127
```
2 logic + 2 chip = 4 lớp, ghép được 4 tổ hợp. Thêm chip C chỉ cần **một** lớp `PwmChipC`; không lớp logic nào bị sửa.

**Bridge khác Strategy chỗ nào — code trông giống hệt:** áp **phép thử "bỏ phần được cắm vào"**:

| | Strategy (`IFilter` trong `SensorReader`) | Bridge (`IPwm` trong `Fan`) |
|---|---|---|
| Bỏ phần cắm vào thì phần kia còn hoàn chỉnh không? | **Còn**: `SensorReader` vẫn là context hợp lệ | **Không**: `Fan` tính xong mà không có `IPwm` thì không quay được quạt |
| Số trục biến thiên | **Một** (thuật toán) | **Hai** (logic × chip), độc lập nhau |
| Ý định | Tách **hành vi** khỏi context | Tách **hai nửa của một thứ** khỏi nhau |

**Trong công việc của bạn — dimming: thuật toán × backend** ([A1 §5.3, §5.4](in-practice/A1-baseline-libdisplay.md)):

| Vai Bridge | Trong `libdisplay` | Biến thiên theo |
|---|---|---|
| Abstraction | `IDimmingAlgo` → `GlobalDimming`, `LocalDimming`, `OLEDDimming` | **Model** — chọn lúc chạy |
| Implementor | `IDimmingBackend` → `DimmingBackendChipA_Global`… | **Chip** — chốt lúc **build** (CMake chỉ build thư mục chip đích) |
| Cây cầu | `IDimmingBackend* m_pDimmingBackend` trong lớp thuật toán | — |

```cpp
GlobalDimming::GlobalDimming(IDimmingBackend* pDimmingBackend, DimmingType_k eDetectedDimming)
    : Shm(get_lib_shm()->shmGlobalDimming) {
    m_pDimmingBackend = pDimmingBackend;             // GHÉP lúc runtime — A1 §5.3
}
```

⭐ **Một lớp, hai vai:** `IDimmingAlgo` là **Strategy** khi nhìn từ `lib_dimming`, và là **abstraction của Bridge** khi nhìn sang `IDimmingBackend` ([A1 §5.5](in-practice/A1-baseline-libdisplay.md)).

⚠️ **Hệ thật không phẳng như sách — nói thẳng ra:** backend nhân theo **chip × thuật toán**, vì phần cứng Local khác nhau thật giữa các chip. Và backend **không mỏng**: backend Local dày ngang thuật toán. Cái Bridge giữ được là **thuật toán chỉ viết một lần cho mọi chip**; không có Bridge thì cả thuật toán bị chép theo từng chip. 🚫 Đừng kể *"N + M lớp"* hay *"backend chỉ vài chục dòng"* — sai với hệ thật ([A1 §5.4](in-practice/A1-baseline-libdisplay.md)).

**Bridge xuất hiện lần hai — ở kernel, viết bằng C** ([A1 §6.3](in-practice/A1-baseline-libdisplay.md)):

| | User-space — dimming | Kernel — panel driver |
|---|---|---|
| Phần chung, không biết chip | `IDimmingAlgo` | `drv_panel_core` |
| Phần theo chip | `DimmingBackendChipX_AlgoY` | `drv_panel_chipX` |
| Cơ chế | `virtual` | `panel_ops` — vtable viết tay |

Và lần ba — ở ranh giới `.so` của HAL: `IDisplay` (header app nhìn thấy) ↔ `DisplayImpl` (nằm trong `.so`), dạng *handle–body* ([A2 §2](in-practice/A2-cpp-interface-hal.md)).

**Cái giá & bẫy:**
- Thêm một interface và một tầng gián tiếp. Chỉ đáng khi **hai trục thật sự độc lập** và cả hai đều có ≥ 2 biến thể đang tồn tại.
- Gọi tên "Bridge" mà không phân biệt được với Strategy thì đừng gọi tên; nói *"tách thuật toán khỏi phần ghi phần cứng"* an toàn hơn ([resume-plan §10](../14-prep/study-plans/resume-plan.md) điểm 5).
- Lý thuyết + Pimpl (một ứng dụng khác của cùng ý tưởng): [structural §1](structural.md).

---

## §3. Factory Method — cho lớp con quyết định tạo class nào

**Một câu:** một **hàm ảo trả về object**; lớp con override hàm đó để quyết định class cụ thể. Bên gọi chỉ thấy interface, không biết tên class.

**Vấn đề nó giải:** code cần **tạo** một object nhưng không nên biết (hoặc không thể biết lúc compile) class cụ thể nào.

**Ví dụ cơ bản:**
```cpp
struct ILogger { virtual ~ILogger() = default; virtual void log(const char*) = 0; };
struct UartLogger : ILogger { void log(const char* m) override { printf("UART: %s\n", m); } };
struct FileLogger : ILogger { void log(const char* m) override { printf("FILE: %s\n", m); } };

class App {
public:
    virtual ~App() = default;
    void run() { auto lg = makeLogger(); lg->log("boot ok"); }   // App không biết class cụ thể
protected:
    virtual std::unique_ptr<ILogger> makeLogger() = 0;          // ⬅️ FACTORY METHOD
};
struct DevBoardApp : App { std::unique_ptr<ILogger> makeLogger() override { return std::make_unique<UartLogger>(); } };
struct ProductApp  : App { std::unique_ptr<ILogger> makeLogger() override { return std::make_unique<FileLogger>(); } };
```
```
UART: boot ok
FILE: boot ok
```

**Trong công việc của bạn — `IDisplayBuilder`** ([A2 §3.1](in-practice/A2-cpp-interface-hal.md)):
```cpp
class IDisplayBuilder {
public:
    virtual IDisplay* buildNewDisplayHandle() = 0;   // MỘT lời gọi → object xong luôn
};
// trong .so:
IDisplay* DisplayBuilderImpl::buildNewDisplayHandle() { return new (std::nothrow) DisplayImpl(); }
```
App nhận `IDisplay*` mà **không hề biết** có class `DisplayImpl`. Đổi implementation = đổi `.so`, không build lại app.

**Cái giá & bẫy:**
- 🔴 **`IDisplayBuilder` KHÔNG phải Builder pattern.** Builder GoF dựng object **nhiều bước** (nhiều setter rồi `build()`); ở đây chỉ **một** hàm tạo xong luôn ⟹ đúng tên là **Factory Method**. Cách nói an toàn: *"trong code nó tên là Builder, nhưng đúng tên GoF là Factory Method"* ([A2 §3.1](in-practice/A2-cpp-interface-hal.md)).
- Chỉ tạo **một** loại sản phẩm. Cần tạo **cả bộ** phải khớp nhau thì sang §4.

---

## §4. Abstract Factory — tạo cả một HỌ object phải khớp nhau

**Một câu:** một interface có **nhiều** hàm `createX()`; mỗi concrete factory tạo ra **một bộ trọn vẹn** của cùng một biến thể.

**Vấn đề nó giải:** có ≥ 2 thứ cần tạo, và chúng **phải cùng một biến thể** — trộn nhầm là bug. Vài hàm `makeX(type)` rời thì không gì buộc chúng dùng cùng `type`.

**Ví dụ cơ bản — bring-up board:**
```cpp
struct IBoardFactory {                                // cả HỌ phải khớp nhau
    virtual ~IBoardFactory() = default;
    virtual std::unique_ptr<IUart> createUart() = 0;
    virtual std::unique_ptr<IGpio> createGpio() = 0;
};
struct BoardAFactory : IBoardFactory {
    std::unique_ptr<IUart> createUart() override { return std::make_unique<UartA>(); }
    std::unique_ptr<IGpio> createGpio() override { return std::make_unique<GpioA>(); }
};
struct BoardBFactory : IBoardFactory { /* UartB + GpioB */ };

void bringUp(IBoardFactory& f) { printf("%s + %s\n", f.createUart()->name(), f.createGpio()->name()); }
```
```
uartA + gpioA
uartB + gpioB
```
Không có cách nào viết ra `uartA + gpioB`: chỉ có **một** object factory, nên không còn tham số nào để truyền sai.

**Trong công việc của bạn — `DimmingFactory`** ([A1 §5.6](in-practice/A1-baseline-libdisplay.md)):
```cpp
DimmingFactory::DimmingFactory() {
    DimmingType_k type = detect_dimming_type();       // theo model — runtime
    switch (type) {
    case DIMMING_GLOBAL:
        m_pDimmingBackend = new DimmingBackendChipA_Global();
        m_pDimmingObject  = new GlobalDimming(m_pDimmingBackend, type);   // CẶP khớp nhau
        break;
    // LOCAL, OLED tương tự
    }
}
```
- Mỗi thư mục chip có **một** `DimmingFactory` riêng; CMake chỉ build bản của chip đích.
- Nó tạo **cặp** thuật toán + backend. Ghép `GlobalDimming` với `DimmingBackendChipA_Local` vẫn **compile sạch** (kiểu đều là `IDimmingBackend*`), nhưng mọi lệnh ghi rơi vào thân mặc định ⟹ **đèn không đổi, không lỗi, không log**. Lỗi đó bị chặn vì **cả cặp chỉ được tạo ở một chỗ**.

**Phân biệt nhanh với §3:**

| Dấu hiệu | Dùng |
|---|---|
| Chỉ một thứ cần tạo, bên gọi không nên biết class | Factory Method |
| ≥ 2 thứ phải **cùng một biến thể**, trộn nhầm là bug | **Abstract Factory** |

**Cái giá & bẫy:**
- Thêm một **loại sản phẩm** mới (vd `createSpi()`) thì phải sửa **mọi** factory.
- Ở hệ thật: logic chọn thuật toán theo model nằm trong **mỗi** bản factory theo chip ⟹ lặp M lần — điểm yếu #3 của A1 ([A1 §9](in-practice/A1-baseline-libdisplay.md)). Nêu ra trước khi bị hỏi là điểm cộng.
- Chỉ có **một** thứ cần tạo thì hàm rời là đủ — đừng dựng factory object cho đẹp.

---

## §5. Singleton — đúng một instance, một điểm truy cập

**Một câu:** class tự bảo đảm chỉ có **một** instance và cho một hàm toàn cục để lấy nó.

**Vấn đề nó giải:** tài nguyên **vật lý** chỉ có một (một thiết bị, một vùng nhớ ánh xạ). Hai instance cùng điều khiển một thiết bị là sai, không phải chỉ là phí.

**Ví dụ cơ bản — Meyers' Singleton:**
```cpp
class Logger {
public:
    static Logger& instance() {
        static Logger inst;                           // khởi tạo LẦN ĐẦU gọi; thread-safe từ C++11
        return inst;
    }
    Logger(const Logger&)            = delete;        // chặn bản sao
    Logger& operator=(const Logger&) = delete;
    void log(const char* m) { printf("#%d %s\n", ++count_, m); }
private:
    Logger() = default;                               // chặn tự dựng
    int count_ = 0;
};
Logger::instance().log("a");
Logger::instance().log("b");
```
```
#1 a
#2 b
```
Ba thứ cùng nhau mới thành Singleton: **constructor `private`** + **copy `= delete`** + **một hàm truy cập**. `static` cục bộ chỉ là *cách tạo*; thứ *ngăn* bản thứ hai là hai dòng `private`/`delete`.

**Trong công việc của bạn — hai chỗ, một đúng một sai:**

| | `DimmingFactory::GetDimmingInstance()` ([A1 §5.6](in-practice/A1-baseline-libdisplay.md)) | `IDisplay::getInstance()` bản gốc ([A2 §3.2](in-practice/A2-cpp-interface-hal.md)) |
|---|---|---|
| Viết thế nào | `static DimmingFactory instance;` — **khởi tạo động** | `static IDisplay* p = nullptr; if (p == nullptr) {...}` |
| Thread-safe? | ✅ compiler sinh guard variable | 🔴 **Không** — `= nullptr` là khởi tạo hằng, không sinh guard; phần `if` là check-then-act |
| Bằng chứng | — | TSan bắt được data race; **178/200** lần chạy dựng hai object |

Cách sửa A2: đưa toàn bộ logic vào **khởi tạo động** của `static`, để compiler lo phần khoá:
```cpp
IDisplay* IDisplay::getInstance() {
    static IDisplay* p_instance = [] () -> IDisplay* {   // lambda chạy ĐÚNG MỘT LẦN
        loadlib(LIB_PATH);
        if (builder) return builder->buildNewDisplayHandle();
        return new (std::nothrow) IDisplay();
    }();
    return p_instance;
}
```

**Cái giá & bẫy:**
- Magic static chỉ bảo vệ **việc khởi tạo**. Gọi method của singleton từ nhiều thread **vẫn phải tự khoá**.
- "Một instance" chỉ đúng **trong một process**. Library nạp vào 5 process thì có 5 instance ([§9](#9-service-locator--nhận-diện-đừng-nhầm-với-singleton)).
- Singleton là **global state trá hình**: coupling ẩn, khó test. Dùng khi hai instance là **sai về vật lý**, không phải khi nó tiện ([creational §3](creational.md)).

---

## §6. Null Object — object "không làm gì" thay cho `nullptr`

**Một câu:** thay vì trả `nullptr` rồi bắt **mọi** bên gọi phải kiểm, trả về một object **hợp lệ nhưng không làm gì**, báo trạng thái bằng mã trả về.

**Vấn đề nó giải:** tính năng có thể **không tồn tại** trên một số cấu hình (không có panel, thiếu `.so`). Kiểm `nullptr` ở hàng trăm chỗ gọi thì sớm muộn có chỗ quên ⟹ segfault.

**Ví dụ cơ bản:**
```cpp
struct IBacklight    { virtual ~IBacklight() = default; virtual int set(int level) = 0; };
struct RealBacklight : IBacklight { int set(int l) override { printf("backlight = %d\n", l); return 0; } };
struct NullBacklight : IBacklight { int set(int) override { return -ENOTSUP; } };   // hợp lệ, không làm gì

std::unique_ptr<IBacklight> make(bool hasPanel) {
    if (hasPanel) return std::make_unique<RealBacklight>();
    return std::make_unique<NullBacklight>();         // thay vì nullptr
}
make(false)->set(80);                                 // gọi thẳng, không cần if
```
```
no panel: set(80) -> -95        (-95 = -ENOTSUP)
backlight = 80
```

**Trong công việc của bạn — hai chỗ, hai lựa chọn mã trả về:**

| | `libdisplay` ([A1 §5.7](in-practice/A1-baseline-libdisplay.md)) | HAL `IDisplay` ([A2 §2.1](in-practice/A2-cpp-interface-hal.md)) |
|---|---|---|
| Khi nào dùng | Platform **không có panel** | **Không nạp được `.so`** |
| Null Object là gì | Chính **base class** `IDimmingAlgo` (mọi method có thân) — `new IDimmingAlgo()` | Chính **base class** `IDisplay` — `new (std::nothrow) IDisplay()` |
| Trả về | `LIB_OK` | `-ENOTSUP` |

```cpp
if (Shm.m_iPlatformType == PLATFORM_NO_PANEL)
    m_pDimmingPanel = new IDimmingAlgo();             // ⬅️ NULL OBJECT — cả ~150 API thành no-op
else
    m_pDimmingPanel = DimmingFactory::GetDimmingInstance();
```

**Cái giá & bẫy:**
- **Đổi an toàn lấy độ hiện:** lỗi không nổ tại chỗ mà **im lặng trôi đi**. Không dùng khi bỏ qua âm thầm là nguy hiểm (lệnh phải chắc chắn tới nơi).
- `LIB_OK` cho việc **không làm** khiến bên gọi không phân biệt *"đã đặt"* với *"không hỗ trợ"* ⟹ `-ENOTSUP` rõ hơn. Ở `libdisplay` vẫn giữ `LIB_OK` vì hàng trăm điểm gọi cũ sẽ bắt đầu báo lỗi nếu đổi — điểm yếu #4 ([A1 §9](in-practice/A1-baseline-libdisplay.md)). Nói được **cả hai lựa chọn và lý do** là điểm cộng.
- Dùng chính base class làm Null Object thì rẻ, nhưng base vừa là **hợp đồng** vừa là **cài đặt mặc định** — hai trách nhiệm một chỗ.
- 🔴 Null Object **không** cứu được ABI: `.so` cũ thiếu ô vtable của API mới thì gọi vào là **segfault**, không rơi về `-ENOTSUP` ([A2 §2.1](in-practice/A2-cpp-interface-hal.md), Lab 3b).

---

## §7. Facade (+ Adapter) — một cửa đơn giản che hệ con phức tạp

**Một câu — Facade:** một interface **đơn giản** che một trình tự **nhiều bước** bên trong; bên ngoài gọi một hàm, đúng thứ tự nằm ở một chỗ.
**Một câu — Adapter:** bọc một thứ **đã có sẵn** để nó khớp với interface mà phía trên **mong đợi**.

**Ví dụ cơ bản — Facade:**
```cpp
struct Power { void on()          { puts("power on"); } };
struct Clock { void set(int mhz)  { printf("clock %d MHz\n", mhz); } };
struct Panel { void init(int w, int h) { printf("panel %dx%d\n", w, h); } };

class DisplayFacade {
    Power p_; Clock c_; Panel pn_;
public:
    void start() { p_.on(); c_.set(148); pn_.init(1920, 1080); }   // thứ tự đúng nằm ở MỘT chỗ
};
DisplayFacade().start();
```
```
power on
clock 148 MHz
panel 1920x1080
```

**Ví dụ cơ bản — Adapter:**
```cpp
extern "C" int legacy_uart_send(const char* buf, int len);           // API C có sẵn

struct ISerial { virtual ~ISerial() = default; virtual bool write(const std::string& s) = 0; };
struct LegacyUartAdapter : ISerial {                                  // khớp interface mới với API cũ
    bool write(const std::string& s) override {
        return legacy_uart_send(s.data(), (int)s.size()) == (int)s.size();
    }
};
```

| | Facade | Adapter |
|---|---|---|
| Mục đích | **Đơn giản hoá** một hệ con | **Chuyển đổi** interface cho khớp |
| Interface phía ngoài | Do Facade tự định nghĩa, gọn hơn | Do phía trên **đã định sẵn** |

**Trong công việc của bạn:**

**① `lib_api_*` — Facade** ⭐ ([A1 §3.3, §4.3](in-practice/A1-baseline-libdisplay.md)). Mỗi hàm gói trọn *kiểm shared memory → khoá → điều phối → mở khoá*:
```cpp
int32_t lib_api_set_backlight(int backlight) {
    if (get_check_shm() == -1) return -1;             // shm sẵn sàng?
    int sem_ret = lib_sem_lock(__FUNCTION__);          // KHOÁ liên process
    if (get_dimming_instance() != NULL)
        ret = get_dimming_instance()->SetBacklight(backlight);
    if (sem_ret == 0) lib_sem_unlock(__FUNCTION__);    // MỞ KHOÁ
    return ret;
}
```
Facade này mua được **ba** thứ:
- Che trình tự khoá — bên gọi không thể quên.
- Che C++ bên trong sau **API C** ⟹ ABI ổn định, bên trong refactor tự do ([A1 §3.1–3.2](in-practice/A1-baseline-libdisplay.md)).
- Là **điểm khoá duy nhất** cho caller: object C++ không ai bên ngoài với tới, nên không có đường nào đi vòng qua khoá.

⚠️ **Ngoại lệ phải tự nói ra:** vòng vsync chạy **bên trong** library, không đi qua `lib_api_*` ⟹ không được khoá ở mặt tiền bảo vệ. Đã kiểm ở source thật: đường vsync đọc/ghi state **không** lấy semaphore ([A1 §5.8](in-practice/A1-baseline-libdisplay.md)).

**② `IDisplay` / `DisplayImpl` — Adapter từ API C sang interface C++** ([A1 §8](in-practice/A1-baseline-libdisplay.md)). App mong đợi một C++ interface; `DisplayImpl` hiện thực interface đó bằng cách gọi xuống `lib_api_*`:
```cpp
int DisplayImpl::setPower(bool onoff) {
    return lib_api_set_power(onoff);                  // C++ interface → C API (pack ở A2 §6.4 ghi đúng dòng này dưới dạng comment "thực tế")
}
```

**Cái giá & bẫy:**
- Facade dễ phình thành "god object" nếu dồn cả logic vào. `lib_api_*` giữ được gọn vì nó **chỉ điều phối**, logic nằm ở PQ/Panel Control phía sau.
- Nói tên *"Facade"* là mời câu *"Facade khác Adapter thế nào?"* — chuẩn bị bảng ở trên ([structural §0](structural.md)).

---

## §8. Self-registration — implementation tự ghi danh, không ai sửa danh sách trung tâm

**Một câu:** mỗi implementation **tự đăng ký** mình vào một bảng/registry lúc được nạp; phần chung chỉ tra bảng, không cần biết có bao nhiêu implementation.

**Vấn đề nó giải:** thêm implementation mới mà **không sửa một dòng** nào của phần chung — kể cả khi implementation nằm trong một `.so` hay `.ko` được nạp lúc chạy.

**Ví dụ cơ bản — registry codec:**
```cpp
using Maker = std::function<std::unique_ptr<ICodec>()>;
std::map<std::string, Maker>& registry() {           // static cục bộ: tránh lỗi thứ tự khởi tạo giữa các .cpp
    static std::map<std::string, Maker> r;
    return r;
}
struct Registrar { Registrar(const char* key, Maker m) { registry()[key] = std::move(m); } };

// --- codec_h264.cpp: tự đăng ký, không ai phải sửa danh sách trung tâm ---
static Registrar reg_h264("h264", [] { return std::make_unique<H264>(); });
// --- codec_vp9.cpp ---
static Registrar reg_vp9("vp9", [] { return std::make_unique<Vp9>(); });
```
```
registry: h264 -> h264
registry: vp9 -> vp9
```

**Trong công việc của bạn — hai tầng, cùng một ý tưởng:**

**① HAL `.so` — `__attribute__((constructor))`** ([A2 §1](in-practice/A2-cpp-interface-hal.md)):
```cpp
extern "C" void __attribute__((constructor)) DisplayImpl_Inject() {   // chạy NGAY khi .so được dlopen
    DisplayBuilderImpl::injectThisBuilder();          // gọi NGƯỢC lên app: IDisplay::injectBuilder(...)
}
```
Luồng: app `dlopen` → loader chạy constructor của `.so` → `.so` tự đăng ký factory (§3) ngược lên interface → interface gọi factory để lấy object thật.

**② Kernel — driver chip tự điền `panel_ops`** ([A1 §6.2](in-practice/A1-baseline-libdisplay.md)): driver nền probe, nạp đúng tổ hợp `.ko` cho model; mỗi `.ko` chip lúc init **tự đăng ký** hàm của mình vào bảng.

**Cái giá & bẫy:**
- **Đổi an toàn lúc link lấy linh hoạt lúc boot.** Sai kiểu, thiếu symbol chỉ lộ **lúc chạy** ([A2 §4](in-practice/A2-cpp-interface-hal.md)).
- Mũi tên gọi **ngược** (`.so` gọi vào symbol nằm trong app) chỉ chạy được khi executable xuất symbol: `ENABLE_EXPORTS ON` (= `-rdynamic`). Thiếu thì `dlopen` thất bại lúc chạy, build vẫn sạch ([A2 Lab 2](in-practice/A2-cpp-interface-hal.md)).
- Ô bảng chưa ai điền thì là `NULL` ⟹ gọi vào là crash (trong kernel là oops). Chữa: điền sẵn cả bảng bằng stub trả `-ENOTSUP` — chính là Null Object (§6) ở dạng C.
- Registry dựa trên biến `static` toàn cục giữa nhiều `.cpp` dễ dính lỗi **thứ tự khởi tạo** ⟹ để registry là `static` **cục bộ trong hàm**, như ví dụ.

---

## §9. Service Locator — nhận diện, đừng nhầm với Singleton

**Một câu:** một **con trỏ toàn cục** được gán một lần ở chỗ khởi tạo, và một hàm `get_*()` trả nó về cho ai cần.

**Vì sao chỉ ở mức nhận diện:** bạn **không chủ động chọn** pattern này; nó là thứ đang có trong hệ. Giá trị là **gọi đúng tên** và giải thích vì sao nó không race.

**Ví dụ cơ bản:**
```cpp
static IClock* g_clock = nullptr;                    // con trỏ toàn cục — không gì ép tính duy nhất
IClock* get_clock() { return g_clock; }              // "locator"
void init_services() { static SysClock c; g_clock = &c; }   // gán MỘT lần ở chỗ khởi tạo
```

**Trong công việc của bạn — `get_dimming_instance()`** ([A1 §7.1](in-practice/A1-baseline-libdisplay.md)):
```cpp
static lib_dimming_interface *dimming = NULL;
lib_dimming_interface* get_dimming_instance() { return dimming; }

void lib_init_modules() {                            // chạy một lần, trong constructor của .so
    dimming = new lib_dimming();
}
```

| | Singleton (§5) | Service Locator |
|---|---|---|
| Ai ép tính duy nhất | **Class tự ép** (ctor `private`, copy `delete`) | **Không ai** — chỉ có quy ước *"chỉ hàm init gán"* |
| Ví dụ trong hệ bạn | `DimmingFactory` | `get_dimming_instance()` |

**Hai điều phải nói được:**
1. **Không race** — nhưng không nhờ khoá hay magic static. Việc gán chạy trong `__attribute__((constructor))`, khi `.so` vừa được nạp, **trước khi process tạo thread nào** ⟹ không có ai để tranh. Đây là cách chữa race khác magic static: **dời khởi tạo ra khỏi vùng có cạnh tranh** — đối chiếu với `getInstance()` của A2 (§5) race thật.
2. **"Một instance" có hai nghĩa.** Library nạp vào 5 process ⟹ **5 object** (mỗi process một bản) nhưng **một bản state thật** (trong shared memory, bảo vệ bằng named semaphore). Câu gói cả kiến trúc: ***"object là per-process, state là per-system"*** ([A1 §7.2](in-practice/A1-baseline-libdisplay.md)).

---

## §10. Chỗ bạn CỐ Ý không dùng pattern — Panel Control

**Một câu:** các hàm `panel_ctl_*` (đổi tần số, resolution, nguồn panel) là **hàm tự do** gọi thẳng `ioctl` — không interface, không factory, không đa hình. Đó là quyết định đúng.

```cpp
int32_t panel_ctl_set_frequency(int hz) {
    panel_freq_t arg;
    arg.hz = hz;
    return ioctl(g_panel_fd, PANEL_IOC_SET_FREQ, &arg);  // A1 §4.3
}
```

**Vì sao không bọc cho đồng bộ với dimming** ([A1 §4.4](in-practice/A1-baseline-libdisplay.md)):
- Trừu tượng hoá phải **trả giá cho một biến thể đang tồn tại**.
- Panel Control **có** biến thể theo chip, nhưng biến thể đó đã được **kernel driver hấp thụ**: một bộ ioctl chung, mỗi chip một driver (§2, Bridge ở kernel).
- ⟹ Ở user-space không còn gì để trừu tượng. Bọc thêm interface là trả giá (gián tiếp, thêm file, thêm test double, thêm một thứ phải giữ ABI) để mua **số không**.

**Ba câu hỏi lọc trước khi dùng bất kỳ pattern nào** (từ [README](README.md)): đã có **≥ 2 biến thể thật** chưa? · chúng khác **hành vi** hay chỉ khác **tham số**? · có ai cần **hoán đổi** không?

> 🗣️ *"Ở đây tôi cố ý không dùng pattern, vì biến thể đã nằm ở driver"* là tín hiệu senior rõ hơn kể tên năm pattern.

---

## §11. 🗺️ Bản đồ một trang — ôn trước giờ phỏng vấn

| # | Pattern | Một câu | Chỗ trong hệ của bạn | Bẫy phải nhớ |
|---|---|---|---|---|
| 0 | Interface & đa hình | Phía trên chỉ biết interface | `virtual` ở user-space · `panel_ops` ở kernel | **Vtable là ABI** — chỉ thêm ở cuối |
| 1 | Strategy | Thay thuật toán, context không đổi | `IDimmingAlgo` trong `lib_dimming` | Một biến thể thì chưa phải Strategy |
| 2 | Bridge | Tách hai trục biến thiên | Thuật toán × backend · core × chip driver · `IDisplay` ↔ `DisplayImpl` | Backend **không mỏng**; đừng nói *"N + M"* |
| 3 | Factory Method | Lớp con quyết định tạo class nào | `IDisplayBuilder::buildNewDisplayHandle()` | Tên "Builder" nhưng **không phải** Builder |
| 4 | Abstract Factory | Tạo cả họ phải khớp nhau | `DimmingFactory` tạo cặp thuật toán + backend | Cặp sai vẫn compile sạch nếu không có factory |
| 5 | Singleton | Một instance, ctor `private` + copy `delete` | `DimmingFactory` (đúng) · `getInstance()` A2 (race) | Magic static chỉ bảo vệ **khởi tạo** |
| 6 | Null Object | Object không làm gì thay `nullptr` | `new IDimmingAlgo()` (`LIB_OK`) · `new IDisplay()` (`-ENOTSUP`) | Không cứu được ô vtable thiếu |
| 7 | Facade · Adapter | Một cửa che trình tự · khớp interface | `lib_api_*` (shm → khoá → điều phối) · `DisplayImpl` gọi C API | Vòng vsync **không** qua mặt tiền |
| 8 | Self-registration | Implementation tự ghi danh khi được nạp | `__attribute__((constructor))` trong `.so` · `.ko` chip điền `panel_ops` | Cần `-rdynamic`; ô `NULL` ⟹ crash |
| 9 | Service Locator | Con trỏ toàn cục + `get_*()` | `get_dimming_instance()` | Không phải Singleton; không race vì init trước khi có thread |
| 10 | *Cố ý không dùng* | Biến thể đã ở driver | `panel_ctl_*` → `ioctl` | Trừu tượng hoá phải trả giá cho biến thể **đang tồn tại** |

**Câu kể toàn cảnh — 3 ý, đúng thứ tự:**
1. *Ba ranh giới*: app → C++ interface (khép) → **C API** (mở, điểm khoá duy nhất) → `ioctl` xuống kernel.
2. *Sau mặt tiền*: Picture Quality nặng logic (Strategy + Bridge + Abstract Factory + Null Object); Panel Control **cố ý** không pattern.
3. *Cùng một ý tưởng hai lần*: Bridge bằng `virtual` ở dimming, bằng bảng con trỏ hàm ở kernel — chip chốt lúc build, tổ hợp theo model chốt lúc chạy.

Bản nói đầy đủ ~75″: [A1 §10](in-practice/A1-baseline-libdisplay.md) · bản tiếng Anh: bank [RES-034](../14-prep/mock-interview/bank/resume.md).

**Không có trong tài liệu này — cố ý:** Observer, Command, Memento, State (chỉ xuất hiện ở bản cải tiến B2 hoặc ở project khác), Builder thật, Decorator, Proxy… Tra nhanh khi cần ở [behavioral](behavioral.md) / [structural](structural.md); viết thêm khi có thời gian.

**Tự luyện bằng tay (không bắt buộc, ~60′):** 5 bài lab ở [A2 §7](in-practice/A2-cpp-interface-hal.md) — tận mắt thấy race trong Singleton (§5), Null Object khi mất `.so` (§6), self-registration hỏng khi thiếu `-rdynamic` (§8), và vtable lệch ô (§0).

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi | Mục |
|----|---------|-----|
| [DP-043](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | `panel_ops` trong kernel so với `virtual` — giống, khác, thừa hưởng hai rủi ro nào? | §0 |
| [DP-027](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Chèn virtual vào giữa `IDisplay`, giữ `.so` cũ — chuyện gì xảy ra? | §0 |
| [DP-033](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Version hai bên khớp mà vẫn gọi nhầm hàm — vì sao? | §0 |
| [DP-005](../14-prep/mock-interview/bank/design-patterns.md) | Strategy là gì? C++ hiện đại hiện thực gọn thế nào? | §1 |
| [DP-038](../14-prep/mock-interview/bank/design-patterns.md) | Bridge giải gì? Khác Strategy chỗ nào khi code giống hệt? | §1–2 |
| [DP-023](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | 3 thuật toán × 3 chip — thiết kế để không thành 9 class | §2 |
| [DP-042](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Backend nhân chip × thuật toán — còn là Bridge không? | §2 |
| [DP-025](../14-prep/mock-interview/bank/design-patterns.md) | `IDisplayBuilder` — Builder hay Factory Method? | §3 |
| [DP-037](../14-prep/mock-interview/bank/design-patterns.md) | Factory Method vs Abstract Factory — dấu hiệu cần cái thứ hai | §3–4 |
| [DP-022](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Vài hàm `makeX(ChipType)` rời có gì sai so với một factory object? | §4 |
| [DP-002](../14-prep/mock-interview/bank/design-patterns.md) | Singleton là gì? Cách hiện đại trong C++? | §5 |
| [DP-014](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Vì sao Meyers' Singleton thread-safe? | §5 |
| [DP-026](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Đọc `getInstance()` — race ở đâu, vì sao `static` không cứu? | §5 |
| [DP-036](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Null Object là gì? Đổi lấy gì, khi nào không nên dùng? | §6 |
| [DP-028](../14-prep/mock-interview/bank/design-patterns.md) | API `IDisplay` là virtual thường, base trả `-ENOTSUP` — pattern gì? | §6 |
| [DP-017](../14-prep/mock-interview/bank/design-patterns.md) | Facade khác Adapter thế nào? | §7 |
| [DP-040](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | C API giữa hai vùng C++ — hai lý do độc lập và một ngoại lệ | §7 |
| [DP-029](../14-prep/mock-interview/bank/design-patterns.md) | `__attribute__((constructor))` tự đăng ký — đánh đổi so với link thẳng | §8 |
| [RES-023](../14-prep/mock-interview/bank/resume.md) ⭐ | Vì sao cần hai tầng bảng con trỏ hàm? Ai điền, lúc nào? | §8 |
| [DP-041](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Library nạp vào 5 process — mấy object, mấy state, có phải Singleton? | §9 |
| [DP-024](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Lệnh đơn để nguyên hàm — vì sao không bọc cho đồng bộ? | §10 |

---
⬅️ [Về README topic](README.md) · Bối cảnh đầy đủ: [A1](in-practice/A1-baseline-libdisplay.md) · [A2](in-practice/A2-cpp-interface-hal.md)
