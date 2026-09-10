# Creational — Ai tạo object, và tạo ra loại nào?

> **TL;DR**
> - Creational pattern trả lời **một** câu hỏi: *"đoạn code cần dùng object có được phép biết class cụ thể của nó không?"* Nếu **không** — vì loại object phụ thuộc board / config / runtime — thì việc *tạo* phải tách khỏi việc *dùng*.
> - **Factory Method**: tạo **một** loại sản phẩm; client chỉ thấy interface. **Abstract Factory**: tạo **cả một họ** phải khớp nhau — điểm bán hàng của nó là **nhất quán họ được bảo đảm bằng KIỂU, không bằng kỷ luật**.
> - **Singleton**: bảo đảm một instance. Tiện, và là pattern **bị lạm dụng nhiều nhất** — nó là global state trá hình. Phần đáng học không phải "viết thế nào" mà là *vì sao Meyers thread-safe* và *vì sao "một instance" sai qua ranh giới `.so`*.
> - **Object Pool**: tái sử dụng object cấp phát sẵn — cấp phát **tất định**, không chạm heap. Đây là creational pattern hợp embedded nhất.
> - Áp dụng thật: [in-practice/B1 §4](in-practice/B1-redesign-architecture.md) (Abstract Factory theo SoC) · [in-practice/02](in-practice/A2-cpp-interface-hal.md) (Factory Method + Singleton trong HAL của bạn).

---

## 0. Vấn đề gốc: `new` là một phụ thuộc

```cpp
void configure() {
    LocalDimming d;        // ⬅️ dong nay GAN CHAT ham nay voi mot class cu the
    d.setTarget(300);
}
```

Dòng `new`/khai báo object cụ thể tạo ra **ba** ràng buộc cùng lúc: (1) phải `#include` header của class đó, (2) phải build lại khi class đó đổi, (3) **không thể** đổi loại lúc runtime. Với một app chạy trên nhiều board, ràng buộc (3) là chặn cứng.

⟹ **Creational pattern = dời điểm quyết định "loại nào" ra khỏi điểm sử dụng**, và dồn nó về **một chỗ duy nhất**.

| Điểm quyết định nằm ở đâu | Cơ chế | Đổi được lúc nào |
|---|---|---|
| Tại chỗ dùng | `new Concrete` | ❌ không |
| Một hàm/lớp tạo tập trung | **Factory Method** | ✅ runtime |
| Một object đại diện cho cả nền tảng | **Abstract Factory** | ✅ runtime, **theo cả họ** |
| Trong `.so` nạp lúc chạy | Factory + `dlopen` + self-registration | ✅ theo board, không rebuild app |

---

## 1. Factory Method

| | |
|---|---|
| **Bản chất** | Đưa việc *chọn class cụ thể* vào sau một hàm ảo, để client chỉ làm việc với interface |
| **Cái biến thiên** | Loại sản phẩm được tạo |
| **Cô lập ở đâu** | Trong lớp con của factory |
| **Nguyên lý** | OCP (thêm loại không sửa client) + DIP (client phụ thuộc interface) |
| **Đừng dùng khi** | Chỉ có một loại và sẽ mãi chỉ một loại |

```cpp
struct Sensor { virtual double read() = 0; virtual ~Sensor() = default; };

// Client chi biet enum, khong biet class cu the
std::unique_ptr<Sensor> createSensor(SensorType t) {
    switch (t) {
        case SensorType::Temp: return std::make_unique<TempSensor>();
        case SensorType::Humi: return std::make_unique<HumiSensor>();
    }
    return nullptr;
}
```

**Ba biến thể hay gặp, phân biệt nhanh:**

| Tên | Hình dạng | Ghi chú |
|---|---|---|
| *Simple factory* | Một hàm free + `switch` | Không phải pattern GoF, nhưng **thường là đủ** |
| **Factory Method** (GoF) | Một **hàm ảo** trong lớp cha, lớp con quyết định | Dùng khi factory tự nó cũng đa hình |
| Factory qua plugin | Hàm ảo nằm trong `.so`, đăng ký ngược lên app | [in-practice/02](in-practice/A2-cpp-interface-hal.md) |

> ⚠️ **Đừng gọi nhầm tên với Builder.** Builder dựng object **nhiều bước** (setter nối chuỗi rồi `build()`); Factory tạo xong trong **một** lời gọi. Chính codebase của bạn có `IDisplayBuilder` mà thực chất là Factory Method — nói sai tên là mời interviewer hỏi vào đúng chỗ đó ([DP-025](../14-prep/mock-interview/bank/design-patterns.md)).

---

## 2. Abstract Factory

| | |
|---|---|
| **Bản chất** | Một object đại diện cho **một biến thể nền tảng**, tạo ra **cả họ** thành phần của nền tảng đó |
| **Cái biến thiên** | Nền tảng (SoC, OS, backend), kéo theo **nhiều** sản phẩm đổi cùng lúc |
| **Cô lập ở đâu** | Trong concrete factory — mỗi nền tảng một class |
| **Nguyên lý** | OCP + DIP, **cộng thêm** một bảo đảm mà Factory Method không có: nhất quán họ |
| **Đừng dùng khi** | Chỉ có **một** loại sản phẩm cần tạo ⟹ Factory Method là đủ |

```cpp
// Hop dong: tao CA HO thanh phan cua mot nen tang
class ISocFactory {
public:
    virtual ~ISocFactory() = default;
    virtual std::unique_ptr<IVideoEnhancer> createEnhancer() = 0;
    virtual std::unique_ptr<IDimming>       createDimming()  = 0;
    virtual std::unique_ptr<IFrc>           createFrc()      = 0;
};

// Chon MOT LAN luc boot -> switch duy nhat cua ca library
std::unique_ptr<ISocFactory> pickFactory(const BoardConfig& cfg);
```

⭐ **Điểm bán hàng thật, và là chỗ ăn điểm ở phỏng vấn.** Nếu chỉ cần "tạo object theo SoC" thì vài hàm rời `makeEnhancer(soc)`, `makeDimming(soc)` cũng làm được. Cái mà chúng **không** làm được: ngăn việc trộn enhancer của SoC-A với dimming của SoC-B. Lỗi đó **compile sạch, chạy được**, chỉ sai trên đúng một board — lớp bug đắt nhất.

> **Abstract Factory chuyển ràng buộc "cả họ phải cùng một nền tảng" từ *kỷ luật lập trình viên* sang *hệ thống kiểu*.** Chỉ có một object factory, và nó **là** nền tảng đó — không còn tham số nào để truyền sai.

| | Vài hàm `makeX(Type)` | **Abstract Factory** |
|---|---|---|
| Nhất quán họ | Do người viết nhớ | **Do kiểu bảo đảm** |
| Vị trí `switch` | Lặp trong mỗi hàm | **Một chỗ duy nhất** |
| Thêm nền tảng mới | Sửa N hàm | Thêm 1 class + 1 `case` |
| Thay bằng test double | Hook từng hàm | Thay **một** factory |

Đầy đủ với ví dụ hệ display: [in-practice/B1 §4](in-practice/B1-redesign-architecture.md) · câu [DP-022](../14-prep/mock-interview/bank/design-patterns.md).

---

## 3. Singleton

| | |
|---|---|
| **Bản chất** | Bảo đảm **đúng một** instance và cho một điểm truy cập toàn cục |
| **Vấn đề nó thật sự giải** | Tài nguyên **vật lý** chỉ có một (một thiết bị, một bus, một vùng nhớ ánh xạ) |
| **Vấn đề nó KHÔNG giải** | "Cho tiện truy cập ở mọi nơi" — đó là global state, không phải Singleton |
| **Cái giá** | Coupling **ẩn** (không nhìn thấy trong chữ ký hàm), khó test, thứ tự khởi tạo/huỷ khó kiểm soát |

```cpp
class Logger {
public:
    static Logger& instance() {   // Meyers' Singleton — C++11 bao dam thread-safe
        static Logger inst;
        return inst;
    }
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
private:
    Logger() = default;
};
```

### 3.1 Vì sao Meyers thread-safe

Mấu chốt ở `static Logger inst;` — một **local static có khởi tạo động**. Chuẩn C++11 (§[stmt.dcl]) **bắt buộc**: nhiều luồng cùng vào lần đầu thì **chỉ một** luồng chạy khởi tạo, các luồng còn lại **chờ**. Đây là bảo đảm của **ngôn ngữ**, không phải may mắn.

Compiler hiện thực bằng **guard variable** ẩn (Itanium ABI: `__cxa_guard_acquire/release`):

```cpp
// Code compiler sinh ra, DAI Y:
if ((guard.load(acquire) & 1) == 0) {      // fast path: 1 atomic load
    if (__cxa_guard_acquire(&guard)) {      // luong dau tien gianh quyen
        new (&inst) Logger();               //   ctor chay dung MOT lan
        __cxa_guard_release(&guard);        //   set co + release
    }                                       // luong khac: BLOCK toi khi release
}
return inst;
```

- **Sau lần đầu**, mỗi lời gọi chỉ là **một atomic load** rồi return — gọi triệu lần vẫn rẻ. Thuật ngữ: **"magic statics"**.
- 🔴 **Chỉ khởi tạo hằng thì KHÔNG có guard.** `static Logger* p = nullptr;` là *constant initialization* — compiler không sinh guard nào, và mọi `if (p == nullptr) { p = ... }` bên dưới là **check-then-act có data race**. Đã đo thật: **178/200** lần chạy 8 luồng dựng object **hai lần** ([in-practice/A2 §3.2](in-practice/A2-cpp-interface-hal.md)).
- ⚠️ **Chỉ thread-safe phần *khởi tạo*, không phải phần *dùng*.** Method sửa state chung thì vẫn phải tự đồng bộ.

**Vì sao double-checked locking tự viết trước C++11 SAI:**

```cpp
// ❌ Hong truoc C++11
if (!inst) { lock(mtx); if (!inst) inst = new Logger(); unlock(mtx); }
```
`inst = new Logger()` gồm 3 việc — *cấp phát · chạy ctor · gán con trỏ*. Trước C++11 **không có memory model**, compiler/CPU được phép sắp xếp lại thành 1→3→2: `inst` thành non-null **trước khi** ctor xong ⟹ luồng khác thấy `inst != null` ở check đầu (không khoá) và dùng ngay một object **dựng dở** ⟹ UB. C++11 sửa gốc rễ bằng memory model + `std::atomic`; nhưng đơn giản hơn nhiều là để magic statics lo — **đừng tự viết DCLP nữa**.

> 💡 *Embedded:* `-fno-threadsafe-statics` bỏ được chi phí guard — nhưng khi đó init **không còn** thread-safe, chỉ dùng nếu chắc chắn single-thread lúc khởi tạo.

### 3.2 "Một instance" — chỉ đúng trong MỘT chương trình đã link xong

Nếu Singleton nằm ở header mà nhiều `.so` cùng include, số instance thực tế do **dynamic linker** quyết định, **không** do chuẩn C++: mặc định các symbol trùng tên được hợp nhất (**symbol interposition**) ⟹ **một** instance; nhưng `-fvisibility=hidden`, `dlopen(RTLD_LOCAL)` hoặc link tĩnh sẽ cho **mỗi `.so` một bản riêng**. Cơ chế: [linking-loading.md](../07-shared-libraries/linking-loading.md) · câu [DP-020](../14-prep/mock-interview/bank/design-patterns.md).

### 3.3 Khi nào KHÔNG dùng — và dùng gì thay

| Lý do định dùng Singleton | Có đúng không | Thay bằng |
|---|---|---|
| Tài nguyên vật lý chỉ có một | ✅ Đúng chỗ | — |
| "Truy cập ở đâu cũng được cho tiện" | ❌ Global state | Truyền tham số / DI |
| "Chỉ cần một, tiết kiệm bộ nhớ" | ❌ Không phải vấn đề của pattern này | Object thường |
| "Cần khởi tạo lazy" | ❌ | `std::optional` / khởi tạo tường minh lúc boot |

> **Chốt để nói ra được:** *"Singleton hợp lệ khi việc có hai instance là **sai về mặt vật lý**, không phải khi nó tiện. Cái giá là coupling ẩn: một hàm dùng Singleton không khai báo điều đó trong chữ ký, nên không test được bằng cách thay thế."*

---

## 4. Object Pool

| | |
|---|---|
| **Bản chất** | Cấp phát trước một tập object cố định, **"mượn/trả"** thay vì `new`/`delete` |
| **Vấn đề nó giải** | Cấp phát động **không tất định** về thời gian và **gây fragmentation** trên hệ chạy lâu dài |
| **Cô lập ở đâu** | Toàn bộ vòng đời bộ nhớ nằm trong pool, biết trước lúc biên dịch |
| **Đừng dùng khi** | Tần suất tạo/huỷ thấp, hoặc kích thước không đoán trước được |

```cpp
template <typename T, std::size_t N>
class ObjectPool {
    alignas(T) unsigned char storage_[N * sizeof(T)];  // vung nho TINH, khong heap
    std::array<T*, N>  free_;
    std::size_t        freeCount_ = 0;
public:
    ObjectPool() {
        for (std::size_t i = 0; i < N; ++i)
            free_[freeCount_++] = reinterpret_cast<T*>(storage_) + i;
    }
    template <typename... Args>
    T* acquire(Args&&... args) {
        if (freeCount_ == 0) return nullptr;            // can pool -> TAT DINH, khong throw
        T* slot = free_[--freeCount_];
        return new (slot) T(std::forward<Args>(args)...);   // placement new
    }
    void release(T* p) {
        if (!p) return;
        p->~T();                                        // PHAI goi dtor tuong minh
        free_[freeCount_++] = p;
    }
};
```

**Ba điều đáng nói:**
1. **Footprint biết trước lúc biên dịch** (mảng tĩnh) và thời gian mượn/trả **O(1) tất định** — hợp realtime ([08/constraints](../08-embedded-systems/constraints.md)).
2. Cạn pool trả `nullptr` thay vì `bad_alloc` bất định ⟹ đường lỗi cũng tất định.
3. **Điểm tinh tế:** dùng `placement new` nên `release` **phải gọi destructor tường minh** — khác `delete` thông thường. Bản production bọc con trỏ trong RAII handle (custom deleter gọi `release`) để không quên trả, và thêm mutex nếu đa luồng.

---

## 5. Builder — nhận diện, không đầu tư

**Vấn đề nó giải:** object có nhiều tham số optional ⟹ constructor "telescoping" khó đọc, dễ nhầm thứ tự. Mỗi setter trả `*this` để nối chuỗi, `build()` chốt lại và validate tập trung.

```cpp
auto req = HttpRequest::Builder("http://api").method(GET).timeout(30).build();
```

**Vì sao chỉ ở mức nhận diện** ([bản đồ chọn lọc](in-practice/README.md)): nó giải bài toán *object cấu hình phức tạp*, hiếm gặp trong tầng HAL/driver; và C++20 có `designated initializers` (`Cfg{.url="...", .timeout=30}`) làm phần lớn công việc gọn hơn. Builder vẫn hơn khi cần **validate tập trung**, tạo **immutable object**, hoặc quá trình dựng có nhiều bước phụ thuộc nhau.

> ⚠️ **Điểm cần nhớ duy nhất về Builder:** phân biệt nó với Factory. *Builder = dựng nhiều bước; Factory = chọn loại, xong trong một lời gọi.*

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-002](../14-prep/mock-interview/bank/design-patterns.md) 🔴 | Singleton là gì? Cách hiện đại trong C++? *(đang trong sổ yếu — regression 4→1)* |
| [DP-014](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Vì sao Meyers' Singleton thread-safe? DCLP tự viết trước C++11 sai ở đâu? |
| [DP-020](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Hai `.so` cùng include header Singleton — có mấy instance? Điều gì lật ngược kết quả? |
| [DP-009](../14-prep/mock-interview/bank/design-patterns.md) | Vì sao Singleton bị coi là anti-pattern khi lạm dụng? |
| [DP-004](../14-prep/mock-interview/bank/design-patterns.md) | Factory pattern giải quyết vấn đề gì? Liên hệ SOLID? |
| [DP-037](../14-prep/mock-interview/bank/design-patterns.md) | Factory Method và Abstract Factory khác nhau ở đâu? Dấu hiệu cần cái thứ hai? |
| [DP-022](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Vài hàm `makeX(SocType)` rời có gì sai so với một `ISocFactory`? |
| [DP-015](../14-prep/mock-interview/bank/design-patterns.md) | Object Pool là gì? Vì sao hợp embedded? Điểm tinh tế khi hiện thực? |
| [DP-013](../14-prep/mock-interview/bank/design-patterns.md) | Thiết kế hệ thống plugin trong C++ dùng pattern nào? |
| [DP-026](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Đọc `getInstance()` trong HAL của bạn — bug gì? Vì sao `static` không cứu được? |

---
⬅️ [solid-principles.md](solid-principles.md) · ➡️ Tiếp theo: [structural.md](structural.md)
