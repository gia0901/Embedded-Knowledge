# Creational Patterns — Singleton, Factory, Builder

> **TL;DR**
> - Creational pattern giải quyết **cách tạo object** sao cho linh hoạt và tách biệt khỏi nơi dùng.
> - **Singleton**: đảm bảo một class chỉ có một instance toàn cục. Tiện nhưng dễ lạm dụng (global state ẩn, khó test) — dùng dè dặt.
> - **Factory Method / Factory**: tách *quyết định tạo loại object nào* khỏi code dùng → thêm loại mới không sửa code client (OCP).
> - **Builder**: xây object phức tạp từng bước, tránh constructor "telescoping" (quá nhiều tham số).
> - C++ hiện đại: nhiều trường hợp dùng `make_unique`, factory function trả `unique_ptr`, hoặc designated initializers thay cho pattern nặng.

---

## 1. Singleton — một instance duy nhất

Đảm bảo chỉ có một instance và cung cấp điểm truy cập toàn cục (vd logger, config, hardware manager).

```cpp
class Logger {
public:
    static Logger& instance() {       // Meyers' Singleton (C++11+): thread-safe
        static Logger inst;            // khởi tạo lazy, an toàn đa luồng theo chuẩn
        return inst;
    }
    void log(const std::string&);
    Logger(const Logger&) = delete;   // cấm copy
    Logger& operator=(const Logger&) = delete;
private:
    Logger() = default;               // constructor private
};
// dùng: Logger::instance().log("hi");
```

- **Meyers' Singleton** (static local): C++11 đảm bảo khởi tạo static local là thread-safe → cách hiện đại, gọn nhất.
- **Cảnh báo (rất hay được hỏi):** Singleton là **global state trá hình** → tạo coupling ẩn, khó test (không thay được bằng mock), khó kiểm soát thứ tự khởi tạo/hủy (static init order fiasco). Lạm dụng là anti-pattern.
- **Khi nào dùng:** thật sự chỉ được phép có một (vd quản lý một tài nguyên phần cứng duy nhất). Cân nhắc thay bằng **dependency injection** (truyền instance vào) để dễ test.

### Vì sao Meyers' Singleton thread-safe? (câu hỏi kinh điển)

Mấu chốt nằm ở dòng `static Logger inst;` — một **local static có khởi tạo động**. Chuẩn C++11 (§[stmt.dcl]) **bắt buộc**: *nếu nhiều luồng cùng vào khai báo này lần đầu, chỉ một luồng chạy khởi tạo, các luồng còn lại phải **chờ** cho tới khi khởi tạo xong.* Đây là bảo đảm của **ngôn ngữ**, không phải may mắn.

Compiler hiện thực bằng một **guard variable** ẩn (Itanium ABI: `__cxa_guard_acquire/release`) — thực chất một cờ atomic gắn với biến static:

```cpp
// Compiler sinh code ĐẠI Ý cho `static Logger inst;`:
if ((guard.load(acquire) & 1) == 0) {     // fast path: đã init chưa? (chỉ 1 atomic load)
    if (__cxa_guard_acquire(&guard)) {     // luồng đầu tiên giành quyền (khóa)
        new (&inst) Logger();              //   chạy constructor đúng MỘT lần
        __cxa_guard_release(&guard);       //   set cờ + release → luồng khác thấy object đã dựng xong
    }                                      // luồng khác tới cùng lúc: BLOCK ở đây tới khi release
}
return inst;
```

- **Sau lần đầu**, mọi lời gọi chỉ là **một atomic load** (`guard & 1`) rồi return — gần như free, không khóa. Nên gọi `instance()` triệu lần vẫn rẻ.
- Thuật ngữ hay gặp: cơ chế này gọi là **"magic statics"**.

**Đối chiếu — vì sao *double-checked locking* tự viết trước C++11 lại SAI:**

```cpp
// ❌ Kiểu cũ, HỎNG trước C++11:
if (!inst) {                      // check 1 (không khóa)
    lock(mtx);
    if (!inst) inst = new Logger();   // check 2 (đã khóa)
    unlock(mtx);
}
```
Lỗi: `inst = new Logger()` gồm 3 việc — *(1) cấp phát, (2) chạy constructor, (3) gán con trỏ vào `inst`*. Trước C++11 **không có memory model**, compiler/CPU được phép **sắp xếp lại** thành 1→3→2: `inst` thành non-null **trước khi** constructor chạy xong. Luồng khác thấy `inst != null` ở check 1 (không khóa) → dùng ngay một object **dựng dở** → UB.

C++11 sửa gốc rễ bằng cách cấp **memory model + `std::atomic`**: giờ *có thể* viết DCLP đúng (dùng `atomic` với acquire/release), **nhưng** đơn giản hơn nhiều là để magic statics lo — nên trong C++11+ **đừng tự viết DCLP nữa**.

> ⚠️ **Chỉ thread-safe phần *khởi tạo*, không phải phần *dùng*.** Sau khi có `inst`, nếu các method của nó (vd `log()`) sửa trạng thái chung, bạn **vẫn phải tự** đồng bộ (mutex) bên trong — magic statics không bảo vệ điều đó.
>
> 💡 *Embedded:* một số compiler cho tắt guard bằng `-fno-threadsafe-statics` để bỏ chi phí atomic — nhưng khi đó init **không còn** thread-safe, chỉ dùng nếu chắc chắn single-thread lúc khởi tạo.

---

## 2. Factory — tách việc tạo object

Vấn đề: code client phải biết và `new` các class cụ thể → gắn chặt, thêm loại mới phải sửa client.

```cpp
struct Sensor { virtual double read() = 0; virtual ~Sensor() = default; };
struct TempSensor  : Sensor { double read() override; };
struct HumiSensor  : Sensor { double read() override; };

// Factory: client chỉ biết enum/chuỗi, không biết class cụ thể
std::unique_ptr<Sensor> createSensor(SensorType t) {
    switch (t) {
        case SensorType::Temp: return std::make_unique<TempSensor>();
        case SensorType::Humi: return std::make_unique<HumiSensor>();
    }
    return nullptr;
}
```

- Tách *quyết định loại nào* khỏi *nơi dùng* → client làm việc qua interface `Sensor`, không phụ thuộc class cụ thể (DIP).
- Thêm loại sensor mới → sửa factory (một chỗ), không sửa mọi client (OCP).
- **Factory Method** (biến thể OOP): để lớp con quyết định tạo object gì qua một virtual method `createX()`.
- Hay dùng cho **plugin** (kết hợp `dlopen`, factory `extern "C"` — xem [linking-loading](../07-shared-libraries/linking-loading.md)).

---

## 3. Builder — xây object phức tạp từng bước

Vấn đề: object có nhiều tham số (nhiều optional) → constructor "telescoping" khó đọc, dễ nhầm thứ tự.

```cpp
// ❌ Telescoping constructor: HttpRequest(url, method, headers, body, timeout, retries...)
//    HttpRequest("...", GET, {}, "", 30, 3);  // 30 là gì? 3 là gì?

// ✅ Builder: đặt tên từng bước, chỉ set cái cần
auto req = HttpRequest::Builder("http://api")
               .method(GET)
               .timeout(30)
               .retries(3)
               .build();
```

Bộ khung Builder — mỗi setter trả `*this` để **nối chuỗi (fluent)**, `build()` chốt lại (và có thể validate):

```cpp
class HttpRequest {
    std::string url_, method_ = "GET";
    int timeout_ = 60, retries_ = 0;
    HttpRequest() = default;                 // chỉ Builder được tạo
public:
    class Builder {
        HttpRequest r_;
    public:
        explicit Builder(std::string url) { r_.url_ = std::move(url); }
        Builder& method(std::string m)  { r_.method_  = std::move(m); return *this; }
        Builder& timeout(int s)         { r_.timeout_ = s;            return *this; }
        Builder& retries(int n)         { r_.retries_ = n;            return *this; }
        HttpRequest build() {
            if (r_.url_.empty()) throw std::invalid_argument("url required"); // validate tập trung
            return std::move(r_);
        }
    };
};
```

- Code dễ đọc (mỗi tham số có tên), bỏ qua được tham số optional, có thể validate trong `build()`.
- C++ hiện đại đôi khi thay bằng **struct tham số + designated initializers** (C++20): `HttpRequest{.url="...", .timeout=30}` — gọn hơn cho trường hợp đơn giản. Builder vẫn hơn khi cần **validate tập trung**, tạo **immutable object**, hoặc quá trình dựng có logic nhiều bước.

---

## 4. Object Pool (đáng nhắc cho embedded)

Tái sử dụng một tập object cấp phát sẵn thay vì tạo/hủy liên tục → tránh fragmentation và chi phí cấp phát động. Rất phù hợp embedded/realtime ([constraints](../08-embedded-systems/constraints.md)): cấp phát tĩnh một pool, "mượn"/"trả" object thay vì `new`/`delete`.

**Ý tưởng cốt lõi:** giữ sẵn một mảng slot + danh sách "slot rảnh". `acquire()` lấy một slot rảnh, `release()` trả về — cả hai **O(1), không chạm heap**:

```cpp
template <typename T, std::size_t N>
class ObjectPool {
    alignas(T) unsigned char storage_[N * sizeof(T)];  // vùng nhớ tĩnh, KHÔNG heap
    std::array<T*, N>  free_;      // stack các slot đang rảnh
    std::size_t        freeCount_ = 0;
public:
    ObjectPool() {
        for (std::size_t i = 0; i < N; ++i)            // ban đầu mọi slot đều rảnh
            free_[freeCount_++] = reinterpret_cast<T*>(storage_) + i;
    }

    template <typename... Args>
    T* acquire(Args&&... args) {                       // "mượn" + dựng tại chỗ
        if (freeCount_ == 0) return nullptr;           // pool cạn → tất định, không throw bad_alloc
        T* slot = free_[--freeCount_];
        return new (slot) T(std::forward<Args>(args)...);  // placement new: dựng ngay trên slot
    }

    void release(T* p) {                               // "trả" + hủy
        if (!p) return;
        p->~T();                                       // gọi destructor tường minh (placement new ⇒ tự hủy)
        free_[freeCount_++] = p;                       // slot lại rảnh
    }
};
// ObjectPool<Packet, 32> pool;  auto* pkt = pool.acquire(...);  ... ;  pool.release(pkt);
```

- **Vì sao hợp embedded:** footprint biết trước lúc biên dịch (mảng tĩnh), thời gian mượn/trả **tất định** (O(1), không đi qua allocator), và khi cạn thì trả `nullptr` thay vì `bad_alloc` bất định — hợp hệ chạy lâu dài/realtime.
- **Điểm tinh tế:** dùng **placement new** (dựng object trên vùng nhớ có sẵn) nên phải **gọi destructor tường minh** trong `release` — khác `delete` thông thường. Bản production thường bọc con trỏ trả về trong RAII handle (custom deleter gọi `release`) để tránh quên trả, và thêm mutex nếu đa luồng.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Singleton là gì? Vì sao thường bị coi là anti-pattern khi lạm dụng?</summary>

Singleton đảm bảo một class chỉ có một instance và cung cấp điểm truy cập toàn cục tới nó (vd logger, config). Trong C++11+ cách gọn và an toàn nhất là Meyers' Singleton — một `static` local trong hàm `instance()`, được chuẩn đảm bảo khởi tạo lazy và thread-safe. Nó bị coi là anti-pattern khi lạm dụng vì thực chất là **global state trá hình**: tạo coupling ẩn (mọi nơi đều có thể truy cập, khó lần dependency), khó test (không thay được bằng mock/stub vì truy cập trực tiếp), và gây vấn đề thứ tự khởi tạo/hủy của các static (static init order fiasco). Chỉ nên dùng khi thật sự bắt buộc một instance (vd một tài nguyên phần cứng duy nhất), và cân nhắc dependency injection để dễ test hơn.
</details>

<details><summary>2) Factory pattern giải quyết vấn đề gì? Liên hệ với nguyên lý SOLID nào?</summary>

Factory tách việc **quyết định tạo loại object nào** ra khỏi code sử dụng: client làm việc qua một interface chung và gọi factory (truyền enum/chuỗi/tham số) để nhận về object, thay vì tự `new` các class cụ thể. Nhờ đó client không phụ thuộc vào class cụ thể (Dependency Inversion — phụ thuộc abstraction), và khi thêm một loại mới chỉ cần sửa factory ở một chỗ chứ không phải mọi nơi dùng (Open/Closed — mở rộng không sửa code client). Factory rất hay dùng cho kiến trúc plugin, nơi loại object cụ thể chỉ biết lúc runtime. Trong C++ hiện đại factory thường trả về `std::unique_ptr` để thể hiện ownership rõ ràng.
</details>

<details><summary>3) Builder pattern dùng khi nào? C++ hiện đại có cách thay thế không?</summary>

Builder dùng khi cần xây một object phức tạp có nhiều tham số (đặc biệt nhiều optional), nhằm tránh "telescoping constructor" — constructor với quá nhiều tham số khó đọc và dễ nhầm thứ tự (vd không rõ `30` và `3` nghĩa là gì). Builder cho phép set từng thuộc tính có tên theo kiểu chuỗi gọi (fluent), bỏ qua tham số không cần, và validate khi `build()`. Trong C++ hiện đại, với trường hợp đơn giản có thể thay bằng một struct cấu hình kết hợp **designated initializers** (C++20): `Config{.timeout=30, .retries=3}` — đạt được sự rõ ràng tương tự mà gọn hơn, không cần class Builder riêng. Builder vẫn hữu ích khi quá trình xây cần logic/validate phức tạp hoặc tạo ra immutable object.
</details>

<details><summary>4) Object pool có lợi gì cho embedded?</summary>

Object pool cấp phát sẵn một tập object cố định và cho "mượn"/"trả" thay vì tạo/hủy động liên tục. Trong embedded điều này rất giá trị vì tránh được cấp phát động ở runtime — vốn gây heap fragmentation (malloc có thể thất bại dù còn bộ nhớ) và có thời gian cấp phát không tất định, cả hai đều nguy hiểm cho hệ chạy lâu dài/realtime với RAM hạn chế. Pool thường được cấp phát tĩnh nên footprint biết trước lúc biên dịch, thời gian "mượn/trả" tất định, và phù hợp với các tiêu chuẩn embedded hạn chế cấp phát động sau giai đoạn khởi tạo.
</details>

---
⬅️ [solid-principles.md](solid-principles.md) · ➡️ Tiếp theo: [structural.md](structural.md)
