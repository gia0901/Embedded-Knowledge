# Phiên mock — 2026-08-10 · comprehensive · track cpp-system

> ## ⏸️ TRẠNG THÁI: TẠM DỪNG ở câu 10/16 — **CHƯA CHẤM ĐIỂM**
>
> Phiên dừng giữa chừng theo yêu cầu (phiên quá dài). **Không có điểm số nào được ghi** và **weak-register không được cập nhật** — vì Bước 3 (review) chưa chạy, chấm lúc này là chấm nửa vời.
>
> **File này là đề bài để tự ôn**, không phải log kết quả. Đáp án ẩn trong `<details>` — **tự trả lời trước rồi mới mở**.
>
> **Kế hoạch:** tự ôn đủ 16 câu (kể cả follow-up) → **mock lại phiên này** sau. Khi mock lại, interviewer đọc file này để **đổi góc hỏi**, không lặp nguyên văn (config §6).

- **Level:** mid-level · **Đã hỏi:** 9 câu + follow-up · **Còn lại:** 7 câu (câu 10–16)
- **Bối cảnh:** Buổi CN Tuần 1 phần 2 — chốt Tuần 1 (C++17 & Modern C++), theo [datalogic-plan](../../study-plans/datalogic-plan.md)
- **Cơ cấu đã thiết kế:** 3 câu 🟢 khởi động · 5 câu 🟡🟠 cơ chế · 3 câu 🔴 design 🏗️ · 2 bài coding · 3 câu xuyên-topic. Chèn sẵn 2 câu sổ yếu (CPP-032, CPP-054) + 2 câu retention đến hạn (CPP-020, CPP-029).

---

# PHẦN A — 9 câu ĐÃ HỎI

> Có kèm câu trả lời thực tế của bạn để đối chiếu. **Nhận xét ẩn trong `<details>`.**

---

## Câu 1 · 🟢 · CPP-029 (retention — đến hạn Tuần 2)

```cpp
struct Widget {
    explicit Widget(int id);      // ⚠️ explicit
    Widget(const char* name);     // không explicit
};
std::vector<Widget> v;

v.emplace_back(42);        // (1)
v.push_back(42);           // (2)
v.emplace_back("sensor");  // (3)
v.push_back("sensor");     // (4)
```

**a)** Dòng nào compile được, dòng nào không?
**b)** Giải thích **cơ chế** khiến `emplace_back` và `push_back` đối xử khác nhau với `explicit`.
**c)** Đây thường được kể là *ưu điểm* của `emplace_back`. Có góc nào coi nó là **nhược điểm** không?

**🔁 Follow-up đã hỏi:** `explicit` tồn tại để chặn tạo object ngoài ý muốn. Khi `emplace_back(42)` compile được, bảo vệ đó còn tác dụng gì với `vector<Widget>`? Bạn có khó chịu không, và làm gì được?

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) được 1,3,4; không được 2. (b) `emplace_back` gọi thẳng constructor và construct tại chỗ trong vector → như `Widget(42)`, hợp lệ. `push_back` cần một object **hoàn chỉnh** → `42` phải chuyển ngầm thành `Widget`, mà ctor `explicit` cấm → lỗi compile. (c) nhược điểm: `emplace_back` hoạt động qua template deduction nên **không deduce được braced-init-list**, còn `push_back` thì được.

**✅ Đúng hết.** (a)(b) chuẩn. (c) nêu đúng một hạn chế thật của `emplace`.

**Góc bạn chưa nêu ở (c) — mới là góc chính:** `emplace_back` **đi vòng qua `explicit`**. Người viết `Widget` cố ý đánh `explicit` để chặn `Widget w = 42;`, nhưng `v.emplace_back(42)` vẫn lọt. Một cú gõ nhầm (định truyền tên, gõ nhầm số) sẽ **compile im lặng**. Đây là mặt trái của "construct tại chỗ": nó gọi **direct-initialization** (`Widget(42)`) chứ không phải **copy-initialization** (`Widget w = 42`), mà `explicit` chỉ chặn cái sau.

**Ở follow-up bạn bảo vệ thiết kế:** *"push_back dùng cho object hoàn chỉnh nên cấm implicit là hợp lý; emplace_back thì người dùng đã ngầm hiểu là truyền argument để construct"*. — **Lập luận này đúng và trùng với lý do chuẩn hoá.** Giữ nguyên quan điểm được, miễn là **biết** rằng `explicit` không bảo vệ ở đường `emplace`.

**Chốt:** *"`push_back` = copy-init (bị `explicit` chặn) · `emplace_back` = direct-init (không bị chặn). Tiện hơn, nhưng mất một lớp lưới an toàn."*
</details>

---

## Câu 2 · 🟢 · CPP-043

```cpp
void log(int level);
void log(const char* msg);

log(NULL);      // (1)
log(0);         // (2)
log(nullptr);   // (3)
```

**a)** Mỗi dòng gọi vào overload nào? Dòng nào gây bất ngờ?
**b)** `nullptr` có **kiểu** gì? Khác `NULL` và `0` ở chỗ nào — về **kiểu**, không phải giá trị.
**c)** Ngoài overload, `nullptr` còn giúp gì trong code **template**?

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) dòng 3 → `const char*`; dòng 1, 2 → `int`. "Dòng 2 gây bất ngờ nhất, vì người dùng nghĩ NULL phải gọi vào dạng pointer." (b) kiểu `std::nullptr_t`, là kiểu con trỏ chứ không phải kiểu số. (c) chưa gặp, nhưng suy luận: tránh việc `NULL` vô tình instantiate ra bản nhận kiểu integral.

**✅ Đúng bản chất cả ba.** (c) tự suy ra được đúng nội dung **EMC Item 8** dù chưa đọc — tốt.

**Một slip nhỏ:** ở (a) bạn ghi *"dòng 2"* nhưng lý do bạn nêu (*"người dùng nghĩ NULL phải gọi vào pointer"*) là của **dòng 1 (`NULL`)**. Dòng 2 (`log(0)`) không bất ngờ — ai cũng biết `0` là số. Bất ngờ nằm ở `NULL`, vì cái tên gợi ý "con trỏ" nhưng nó chỉ là macro `0` (hoặc `0L`).

**Bổ sung cho (b) — chính xác hơn:** `nullptr` **không phải kiểu con trỏ**. Nó có kiểu riêng `std::nullptr_t`, kiểu này **chuyển ngầm được sang mọi kiểu con trỏ** nhưng **không** chuyển sang kiểu số nguyên. Khác biệt tinh tế nhưng đúng chỗ đó mới giải thích được vì sao overload resolution chọn `const char*`.

**Ý (c) đầy đủ (EMC Item 8):**
```cpp
template<typename FuncType, typename PtrType>
auto call(FuncType f, PtrType ptr) { return f(ptr); }

call(logAndOpen, NULL);      // ❌ PtrType deduce ra kiểu SỐ (long) → lỗi hoặc gọi nhầm
call(logAndOpen, nullptr);   // ✅ PtrType = std::nullptr_t → chuyển đúng sang con trỏ
```
Ngoài hàm template, `NULL` còn không dùng được để so sánh trong `constexpr`/SFINAE theo kiểu.
</details>

---

## Câu 3 · 🟢 · OS-002

**a)** Thread cùng process **chia sẻ** gì, **có riêng** gì?
**b)** Trong một hàm có `static int counter;` và `int tmp;`. Hai thread cùng chạy — biến nào race, biến nào không? Vì sao?
**c)** Muốn mỗi thread có **bản riêng** của một biến toàn cục thì dùng gì? Trong **shared library** có gì phải lưu ý?

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) chung memory map → chia sẻ global (biến, hàm); riêng: biến local, thanh ghi, stack. (b) `static` race vì sống suốt vòng đời chương trình và dùng chung; `tmp` không race vì mỗi thread có bản riêng trên stack. (c) *"định nghĩa biến static riêng cho mỗi thread"* — **chưa nêu được từ khoá**; phần shared library: **chưa rõ**.

**✅ (a)(b) đúng.** ❌ **(c) là lỗ hổng.**

**Đáp án (a) đầy đủ — chia theo vùng nhớ:**

| Chia sẻ (chung address space) | Riêng từng thread |
|---|---|
| code (`.text`), `.data`/`.bss` (global/static) | **stack** riêng |
| **heap** | **thanh ghi** + program counter |
| file descriptor table, signal handler, cwd, PID | **TLS** (thread-local storage) |
| memory mapping | `errno` (thực chất là TLS) |

**(c) Từ khoá: `thread_local`** (C++11 — storage duration thứ tư, cạnh `static`/`automatic`/`dynamic`).
```cpp
thread_local int counter = 0;   // mỗi thread một bản, tự khởi tạo/huỷ theo vòng đời thread
```

**Phần shared library (chỗ bạn chưa rõ) — vì sao đáng biết với JD này:** truy cập biến `thread_local` **không** rẻ như đọc một global thường. Có nhiều **TLS access model**:
- Trong executable link tĩnh: model nhanh (`local-exec`) — chỉ là một offset từ thanh ghi thread pointer (`%fs` trên x86-64, `TPIDR_EL0` trên ARM64).
- Trong `.so` được **`dlopen` lúc runtime**: phải dùng **`global-dynamic`** → mỗi lần truy cập gọi hàm runtime **`__tls_get_addr()`** để tra bảng. Chậm hơn đáng kể, và cấp phát block TLS có thể xảy ra **lazy** lần truy cập đầu.

Hệ quả thực tế: **đừng đặt `thread_local` trong hot path của một shared library**. Nếu cần, đọc một lần vào biến local rồi dùng lại. Có thể ép model bằng `-ftls-model=initial-exec`, nhưng khi đó `.so` **không `dlopen` được nữa** (phải link lúc load) — đúng kiểu đánh đổi phỏng vấn muốn nghe.

**Chốt:** *"`thread_local` là TLS. Trong executable nó gần như free; trong `.so` dlopen nó là một lời gọi hàm mỗi lần truy cập."*
</details>

---

## Câu 4 · 🟡 · CPP-037 · ⭐ C++17

```cpp
// Trước:  void parse(const std::string& cfg);
// Sau:    void parse(std::string_view cfg);
```

**a)** Được lợi gì? Nêu một call site cụ thể mà bản cũ tốn chi phí còn bản mới thì không.
**b)** Đoạn này sai ở đâu, sai **vào lúc nào**:
```cpp
std::string_view name = get_config()["device_name"];   // get_config() trả về map THEO GIÁ TRỊ
std::cout << name;
```
**c)** Với API **public của shared library**, có nên đổi hết `const std::string&` → `string_view`? Nêu lý do **không** đổi.

**🔁 Follow-up đã hỏi:** thư viện đã ra 1.0, khách hàng link vào và **không build lại**. Bạn phát hành 1.1 đổi chữ ký sang `string_view`. Chuyện gì xảy ra với app khách hàng, và ở **giai đoạn nào**?

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) `parse("hello")` — bản cũ phải cấp phát `std::string` tạm, bản mới không. (b) `get_config()` trả về map **theo giá trị** → là temporary; `operator[]` trả reference vào trong temporary đó; hết câu lệnh temporary bị huỷ → `string_view` **dangling** → `cout` là UB. (c) không nên đổi hết, vì không kiểm soát được vòng đời chuỗi mà `string_view` trỏ tới; chỉ dùng khi đọc và xử lý ngay, không lưu lại.

**✅ (a)(b)(c) đúng và sắc.** (b) truy đúng tới "temporary bị huỷ cuối full-expression" — đây là bẫy số 1 của `string_view`.

**❌ Follow-up: chưa trả lời được — đây là lỗ hổng ABI, sẽ gặp lại ở câu 13 và 16.**

**Đáp án follow-up:** đổi chữ ký hàm trong C++ là **phá vỡ ABI**, không chỉ API.
- Tên hàm trong `.so` là **mangled name** mã hoá cả kiểu tham số: `parse(const std::string&)` → `_Z5parseRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE`; `parse(std::string_view)` → `_Z5parseSt17basic_string_viewIcSt11char_traitsIcEE`. **Hai symbol khác nhau hoàn toàn.**
- App khách hàng đã compile, trong bảng relocation của nó ghi symbol **cũ**. Thư viện 1.1 không còn export symbol đó nữa.
- **Giai đoạn xảy ra: lúc LOAD (dynamic linking), không phải compile, không phải chạy nghiệp vụ.** Thông báo kiểu:
  ```
  ./app: symbol lookup error: ./libcfg.so.1: undefined symbol: _Z5parseRKNSt7...
  ```
  App **không khởi động được**. Với `dlopen` thì `dlsym()` trả `NULL`.

**Nguyên tắc:** thay đổi chữ ký hàm public = **bump SONAME** (`libcfg.so.1` → `libcfg.so.2`), hoặc **giữ overload cũ** và thêm bản mới. Không bao giờ đổi lặng lẽ trong bản minor.

> Đây chính là lý do **không** liên quan lifetime mà câu hỏi (c) nhắm tới: dù bạn kiểm soát được vòng đời hoàn hảo, bạn vẫn không được phép đổi.
</details>

---

## Câu 5 · 🟡 · CPP-027

```cpp
void process() {
    Resource* r = acquire();
    Mutex.lock();
    doWork();          // ← có thể ném exception
    Mutex.unlock();
    release(r);
}
```

**a)** `doWork()` ném thì hậu quả gì? Kể đủ.
**b)** Viết lại bằng RAII. Cơ chế nào của C++ đảm bảo dọn dẹp **vẫn chạy** khi có exception — gọi tên.
**c)** Nhiều dự án embedded build `-fno-exceptions`. Khi đó RAII còn giá trị gì?

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) leak `r`, mutex kẹt vĩnh viễn, mọi thread chờ mutex treo theo. (b) bọc `Resource` bằng RAII class tự `release` trong dtor; mutex dùng `lock_guard`; cơ chế = **stack unwinding**. (c) RAII vẫn giá trị: tự dọn không cần nhớ thủ công, cú pháp gọn, chi phí wrapper không đáng kể.

**✅ Đúng cả ba, gọi đúng tên stack unwinding.**

**Bổ sung (a):** còn một hậu quả nữa hay bị quên — nếu `doWork()` ném và **không ai bắt**, `std::terminate()` được gọi; nhưng nguy hiểm hơn là **mutex kẹt làm hệ thống chết dần**: thread khác không crash, chúng **treo im lặng**, log không có gì. Trên embedded chạy 24/7 thì đây là kiểu lỗi "thiết bị đơ sau vài ngày".

**Bổ sung (c) — hai ý mạnh hơn mà bạn chưa nêu:**
1. **Early return.** RAII không chỉ cứu exception, nó cứu **mọi đường thoát**: `return` sớm, `break`, `goto`, `continue`. Một hàm có 5 chỗ `return` mà dọn thủ công thì phải viết 5 lần — và lần sửa sau sẽ quên một chỗ. Đây là giá trị **lớn nhất** khi `-fno-exceptions`.
2. **Đúng thứ tự huỷ, tự động.** Nhiều tài nguyên phụ thuộc nhau, dtor chạy **ngược thứ tự khởi tạo** — miễn phí. Viết tay thì phải tự nhớ thứ tự.

**Chốt:** *"RAII không phải cơ chế cho exception. Nó là cơ chế cho **mọi đường ra khỏi scope** — exception chỉ là đường ra khó thấy nhất."*
</details>

---

## Câu 6 · 🟠 · CPP-048 + CPP-020 (retention) — ⭐ câu lộ nhiều nhất

```cpp
class Session {
public:
    Session(const char* name) : buf_(new char[256]) { strncpy(buf_, name, 255); }
    ~Session() { delete[] buf_; }        // ← có dtor
private:
    std::unique_ptr<Connection> conn_;   // RAII
    char*  buf_;                         // thô
    std::string tag_;
};

std::vector<Session> sessions;
sessions.push_back(Session("dev0"));     // (X)
```

**a)** Compiler **tự sinh** special member nào, **không sinh** cái nào? Cái gì đã chặn?
**b)** Dòng (X) — chuyện gì xảy ra? Truy tới hậu quả cụ thể.
**c)** Sửa theo **Rule of 0**. Sau khi sửa, `Session` còn copy được không?

<details><summary>Bạn đã trả lời gì + Nhận xét — ⚠️ đọc kỹ phần này</summary>

**Bạn trả lời ban đầu:** (a) tự sinh copy (shallow), không sinh default ctor, không sinh move (vì có dtor). (b) **"lỗi runtime: leak + dangling"** — shallow copy con trỏ, temporary huỷ và `delete[]`, con trỏ trong vector thành dangling. (c) bản sửa dùng `std::string buf_`, bỏ dtor → *"vẫn copy bình thường, deep copy đúng"*.

**Phần đúng:** ✅ *"có dtor ⟹ move ctor và move assign KHÔNG được sinh"* — đúng, đây là lõi Rule of 5.

**❌ Sai ở (a)(b)(c) vì bỏ sót một member.** Đã **biên dịch thật** để kiểm chứng:

```
### BẢN GỐC:
error: use of deleted function 'Session::Session(const Session&)'
note: 'Session::Session(const Session&)' is implicitly deleted because
      the default definition would be ill-formed:
error: use of deleted function 'std::unique_ptr<...>::unique_ptr(const unique_ptr&)'

### BẢN SỬA CỦA BẠN (thêm `Session b = a;`):
error: use of deleted function 'Session::Session(const Session&)'
error: use of deleted function 'std::unique_ptr<...>::unique_ptr(const unique_ptr&)'
```

**`std::unique_ptr<Connection> conn_` mới là member quyết định.** `unique_ptr` có copy ctor `= delete`. Một class chứa member không copy được thì copy ctor của nó **được khai báo ngầm nhưng định nghĩa là deleted**.

**Chuỗi suy luận đúng cho (a)(b):**
1. Có dtor tự viết ⟹ **move ctor / move assign không sinh**.
2. `unique_ptr` member ⟹ **copy ctor / copy assign bị deleted**.
3. `push_back(Session(...))` cần move (ưu tiên) hoặc copy → move không có, copy deleted → **LỖI COMPILE**, không phải lỗi runtime.

> **Bạn đã tự sửa được sau khi tôi chỉ ra bạn đã comment mất dòng `unique_ptr` trong file test.** Và bạn tổng quát hoá đúng: class gốc cũng lỗi compile. Nhưng **bài học phương pháp quan trọng hơn nội dung**: bạn đã **loại bỏ chính biến đang được đo** rồi kết luận "không có vấn đề gì". Phép thử bỏ đi biến cần đo thì không nói lên điều gì. Đây là kỹ năng debug — mảng bạn tự nhận là điểm yếu.

**(c) Bản sửa đúng — `Session` trở thành MOVE-ONLY, không copy được:**
```cpp
class Session {
public:
    Session(std::string name) : buf_(std::move(name)) {}
    // KHÔNG viết dtor, KHÔNG viết copy/move → Rule of 0 thật sự
private:
    std::unique_ptr<Connection> conn_;
    std::string buf_;
    std::string tag_;
};
// Compiler sinh: move ctor ✅, move assign ✅ (vì không còn dtor tự viết)
//                copy ctor ❌ deleted, copy assign ❌ deleted (vì unique_ptr)
static_assert(!std::is_copy_constructible_v<Session>);
static_assert( std::is_move_constructible_v<Session>);
```
`sessions.push_back(Session("dev0"))` giờ **chạy được** — vì dùng **move**, không cần copy.

**"Mất gì không?"** — mất khả năng copy. Muốn copy thì phải quyết định `conn_` nghĩa là gì: nếu chia sẻ được thì `shared_ptr<Connection>`; nếu mỗi Session phải có connection riêng thì viết copy ctor tự tạo connection mới (bỏ Rule of 0, có lý do chính đáng).

**Cách kiểm chứng không cần viết copy ctor** (câu bạn hỏi giữa phiên):
```cpp
static_assert(std::is_copy_constructible_v<Session>, "copy được");
static_assert(std::is_move_constructible_v<Session>, "move được");
```
Không đụng vào class, không vi phạm Rule of 0.

**Bảng cần thuộc — điều gì chặn việc sinh special member:**

| Bạn viết | Compiler NGỪNG sinh |
|---|---|
| dtor | move ctor, move assign |
| copy ctor **hoặc** copy assign | move ctor, move assign |
| move ctor **hoặc** move assign | copy ctor, copy assign (**bị delete**) |
| bất kỳ ctor nào | default ctor |
| — | *Member không copy được ⟹ copy của class bị **deleted*** |

**Chốt:** *"`unique_ptr` member = class move-only, dù bạn không viết dòng nào. Có dtor = mất move. Hai luật này gặp nhau thì class không copy cũng không move được."*
</details>

---

## Câu 7 · 🟠 · CPP-012

```cpp
std::vector<int> makeData() {
    std::vector<int> v(1000);
    return v;                 // (A)
    // return std::move(v);   // (B)
}
```

**a)** (A) hay (B) nhanh hơn? Giải thích **cơ chế**.
**b)** Vì sao `return std::move(v)` **phản tác dụng**? Nó chặn mất cái gì?
**c)** Từ C++17 có một dạng elision được chuẩn **bắt buộc**. Dạng nào — RVO hay NRVO? Ví dụ trên rơi vào dạng nào?

**🔁 Follow-up đã hỏi:**
```cpp
std::vector<int> f() { std::vector<int> v(1000); return v; }   // (1) biến CÓ TÊN
std::vector<int> g() { return std::vector<int>(1000); }        // (2) temporary KHÔNG tên
```
C++17 bắt buộc elision cho cái nào? Cái còn lại: *bắt buộc* hay *cho phép*?

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) A nhanh hơn, compiler construct thẳng `v` ở nơi gọi. (b) `std::move` chặn RVO → thành 2 bước construct + move. (c) *"dạng RVO, compiler chắc chắn dùng"*. **Follow-up: chưa rõ.**

**✅ (a)(b) đúng bản chất.** ❌ **(c) sai — và đây đúng là chỗ follow-up nhắm tới.**

**Slip thuật ngữ:** bạn gọi RVO là *"Right Value Optimization"*. Đúng là **Return Value Optimization**.

**Phân biệt hai thứ — đây là nội dung chính:**

| | Tên | Ví dụ | C++17 |
|---|---|---|---|
| Trả về **prvalue không tên** | **RVO** (copy elision) | `return std::vector<int>(1000);` | ✅ **BẮT BUỘC** (guaranteed copy elision) |
| Trả về **biến local có tên** | **NRVO** (*Named* RVO) | `return v;` | ⚠️ **CHO PHÉP, không bắt buộc** |

**Ví dụ trong đề là NRVO** — tức C++17 **không** bắt buộc. Mọi compiler thực tế đều làm, nhưng chuẩn không ép; tắt bằng `-fno-elide-constructors` là thấy copy/move quay lại. Nói *"chắc chắn"* ở đây là **sai về mặt chuẩn**.

**Vì sao C++17 chỉ bắt buộc cho prvalue:** C++17 định nghĩa lại prvalue — nó **không còn là một object**, mà là "công thức để khởi tạo một object". `std::vector<int>(1000)` không tạo temporary rồi copy đi; nó **khởi tạo thẳng** vào chỗ đích. Không có gì để elide, nên không cần copy/move ctor tồn tại:
```cpp
struct NoMove { NoMove()=default; NoMove(NoMove&&)=delete; };
NoMove g() { return NoMove{}; }   // ✅ C++17 OK — C++14 thì LỖI
```

**Vì sao (b) phản tác dụng — nói cho đủ:** `return std::move(v)` biến biểu thức trả về từ **lvalue có tên** (đủ điều kiện NRVO) thành **xrvalue** → NRVO **không áp dụng được nữa** → bắt buộc gọi move ctor. Với `vector` thì move rẻ, nhưng vẫn thừa một lần construct + huỷ. Tệ hơn: nếu kiểu trả về là loại **không move được** thì `std::move` biến code từ chạy tốt thành **copy đắt** (hoặc lỗi).

**Ngoại lệ đáng biết (kẻo áp dụng máy móc):** khi trả về **member của local** hoặc **tham số hàm**, `std::move` là **cần**:
```cpp
std::string f(std::string s) { return std::move(s); }   // ✅ ĐÚNG — tham số không đủ điều kiện NRVO
```

**Chốt:** *"prvalue không tên → C++17 bắt buộc elide. Biến có tên → NRVO, chỉ được phép. Và đừng bao giờ `return std::move(local)` — nó biến 'không copy gì cả' thành 'chắc chắn move'."*
</details>

---

## Câu 8 · 🟠 · CPP-015

```cpp
class DeviceMonitor {
    int threshold_ = 50;
    std::vector<int> samples_;
public:
    void start(ThreadPool& pool) {
        pool.submit([=]() {                       // (A)
            for (int s : samples_)
                if (s > threshold_) alert();
        });
    }
};
```

**a)** `[=]` thực sự capture cái gì?
**b)** Vì sao nguy hiểm? Kịch bản crash cụ thể.
**c)** Sửa thế nào? ≥2 hướng + đánh đổi.

**🔁 Follow-up đã hỏi:** cách "copy ra biến rời rồi capture" cần biến trung gian. C++14 có cách làm ngay trong capture-list — viết thử.

<details><summary>Bạn đã trả lời gì + Nhận xét</summary>

**Bạn trả lời:** (a) capture `this`, không phải data member; thực chất là `this->threshold_`, `this->samples_`. (b) nếu task chạy sau khi object `DeviceMonitor` đã bị huỷ → truy cập member đã giải phóng → UB. (c) copy ra biến rời rồi capture theo tên; hoặc C++17 `[*this]` copy cả object (nặng hơn nhưng đa dụng). **Follow-up:** `[threshold = threshold_, samples = samples_]` — init capture.

**✅ Câu tốt nhất phiên. Đúng và đủ cả ba tầng, follow-up trả lời ngay.**

**Bổ sung nhỏ cho đầy đủ:**
- `[=]` capture `this` **theo giá trị** — nhưng "giá trị" ở đây là **con trỏ**, nên các member vẫn bị truy cập qua con trỏ đó ⟹ hiệu ứng y như capture by reference. Đây là điều gây hiểu nhầm nhất: dấu `=` khiến người ta tưởng "copy hết cho an toàn".
- **C++20 deprecate** việc `[=]` capture ngầm `this` — chính vì bẫy này. Compiler mới sẽ cảnh báo.
- Hướng thứ 3 chưa nêu: **`shared_ptr` + `weak_ptr`** — lambda giữ `weak_ptr`, khi chạy thì `lock()`; object chết thì `lock()` trả `nullptr` và task tự bỏ qua. Đây là cách chuẩn khi **không kiểm soát được thứ tự huỷ** (đúng ca thread pool):
```cpp
void start(ThreadPool& pool) {
    std::weak_ptr<DeviceMonitor> weak = shared_from_this();
    pool.submit([weak] {
        if (auto self = weak.lock()) { /* an toàn */ }
    });
}
```
Đánh đổi: phải quản object bằng `shared_ptr` (dùng `enable_shared_from_this`), thêm chi phí atomic — nhưng là hướng **duy nhất đúng** khi vòng đời thật sự không xác định.

**Bảng chốt:**

| Cách | Khi nào dùng | Đánh đổi |
|---|---|---|
| `[threshold = threshold_, samples = samples_]` (C++14) | Chỉ cần vài member | Copy dữ liệu; ý định rõ ràng nhất |
| `[*this]` (C++17) | Cần nhiều member | Copy **cả object** — đắt nếu object lớn |
| `weak_ptr` + `lock()` | Vòng đời không kiểm soát được | Phải dùng `shared_ptr`, chi phí atomic |
</details>

---

## Câu 9 · 🟠 · CPP-032 (SỔ YẾU — lần trước 2 điểm)

```cpp
class Status {
public:
    static Status ok();
    static Status error(int code);
    operator bool() const { return code_ == 0; }   // "if (st)" cho tiện
    int  code() const { return code_; }
private:
    int code_;
};
Status readSensor();
Status writeConfig();
```
```cpp
if (readSensor())                              // (1)
    process();
if (readSensor() == writeConfig())             // (2)
    log("cùng kết quả");
int failures = !readSensor() + !writeConfig(); // (3)
std::cout << "status = " << readSensor();      // (4)
if (readSensor() > 0)                          // (5)
    retry();
```

**a)** Dòng nào compile được? Dòng nào là bug dù compile được? (2) và (4) thực sự so sánh/in ra cái gì?
**b)** Một thay đổi duy nhất giết các dòng sai mà **vẫn giữ (1) chạy**. Là gì, và **vì sao (1) không chết theo**?
**c)** `Status` có nên có `operator bool` không? Thiết kế lại thì bạn chọn hướng nào?

**🔁 Follow-up đã hỏi:** vì sao **(3) vẫn sống sót** sau khi thêm `explicit`? Đó là lỗ hổng hay đúng ý muốn?

<details><summary>Bạn đã trả lời gì + Nhận xét — ✅ đã phục hồi</summary>

**Bạn trả lời:** (a) tất cả compile được; 2, 4, 5 là bug; giá trị thật là `int` đã chuyển ngầm từ `bool`. (b) thêm `explicit` cho `operator bool`; dòng (1) không chết vì trong `if(...)` thì `operator bool` vẫn được gọi hợp lệ. (c) nên có, để `if`/`while` dùng gọn — nhưng **phải `explicit`**. **Follow-up:** đúng ý muốn; `!` với `operator bool` là hợp lệ, `bool + bool` → `int` là đúng dự đoán.

**✅ Đúng hết. So với lần trước (07/08 hỏi lý thuyết → 2 điểm) thì đây là phục hồi rõ rệt** — bạn đã tự nhận diện safe-bool problem trong code review mà không cần gợi ý.

**Số liệu compiler thật (đã chạy `g++ -std=c++17 -Wall -Wextra`):**

| Dòng | Không `explicit` | Có `explicit` |
|---|---|---|
| (1) `if (readSensor())` | ✅ | ✅ |
| (2) `readSensor() == writeConfig()` | ✅ | ❌ lỗi compile |
| (3) `!readSensor() + !writeConfig()` | ✅ | **✅ vẫn sống** |
| (4) `cout << readSensor()` | ✅ | ❌ lỗi compile |
| (5) `readSensor() > 0` | ✅ | ❌ lỗi compile |

**Giá trị thật trước khi sửa:**
- (2) so sánh **hai `bool`**: `error(5)` và `error(99)` đều cho `false` ⟹ `false == false` ⟹ **`true`** ⟹ log *"cùng kết quả"* cho hai lỗi **hoàn toàn khác nhau**. Đây là bug tệ nhất trong 5 dòng vì nó *im lặng và sai về ngữ nghĩa*.
- (4) in ra **`1`** hoặc **`0`**, không phải mã lỗi. Log production trở nên vô dụng.

**Tên chính xác của cơ chế cứu (1):** **contextual conversion to bool** (C++11). Ở các vị trí ngôn ngữ *đằng nào cũng cần bool*, conversion operator `explicit` **vẫn được gọi tự động**: `if` · `while` · `for(;;)` · `!x` · `x && y` · `x || y` · `x ? a : b` · `static_assert` · `do-while`.

**Vì sao (3) sống:** `!x` nằm trong danh sách trên ⟹ `!readSensor()` cho ra `bool` hợp lệ ⟹ `bool + bool` là integral promotion bình thường. **Đúng ý muốn** — đếm số lần thất bại là cách dùng chính đáng.

**(c) — hướng thiết kế đầy đủ hơn câu trả lời của bạn:**

| Hướng | Ưu | Nhược |
|---|---|---|
| `explicit operator bool` (bạn chọn) | Gọn, quen thuộc (giống `optional`, `ifstream`) | Vẫn ẩn mã lỗi; người dùng dễ quên kiểm `code()` |
| Đặt tên rõ: `bool isOk() const` | Không magic, không bẫy nào | Dài hơn ở call site |
| `[[nodiscard]] Status` | **Ép** người dùng không được bỏ qua giá trị trả về | Không ngăn được việc dùng sai kiểu |
| `std::expected<T,E>` (C++23) / `outcome` | Ép xử lý lỗi bằng kiểu | Chưa dùng được ở C++17 |

**Câu trả lời senior:** *"`explicit operator bool` **cộng** `[[nodiscard]]` trên `Status`. Cái đầu chặn dùng sai kiểu, cái sau chặn quên kiểm tra — hai lớp lỗi khác nhau."*
</details>

---

# PHẦN B — 7 câu CHƯA HỎI (câu 10–16)

> Tự trả lời trước khi mở `<details>`. Khi mock lại, những câu này sẽ được hỏi ở **góc khác**.

---

## Câu 10 · 💻 CODING 1 · COD-006 (làm lại — bài plan yêu cầu)

> **⛔ KHÔNG mở `reviewed/2026-08-09--COD-006--ring-buffer.cpp` trước khi làm xong.**
> Lần trước **3/4**. Ba lỗi đã ghi: **cấp phát STL trong hot path** · **không hỏi spec "mấy luồng?"** · **`empty()/full()` không an toàn đa luồng**.

**Đề:** viết `RingBuffer<T>` cho daemon thu dữ liệu sensor trên embedded Linux.

**Spec:**
- Sức chứa **cố định**, cấp phát **một lần** lúc khởi tạo. **Không cấp phát gì trong `push()`/`pop()`.**
- **Một** producer thread — **một** consumer thread. Đúng 2 luồng (SPSC).
- Đầy thì **đè cái cũ nhất**, và **đếm số sample bị mất** — `dropped_count()`.
- Có `empty()`, `full()`, `size()` dùng được từ cả hai luồng.

**Yêu cầu:**
1. **Trước khi viết**, nói bạn chọn cơ chế đồng bộ nào và **vì sao** (mutex? atomic? lock-free?). Nêu đánh đổi.
2. Viết vào **file mới** `coding-arena/ring_buffer_v2.cpp`, có `main()` test.
3. Nêu độ phức tạp `push`/`pop`.

<details><summary>Khung đáp án — CHỈ mở sau khi đã viết xong code của bạn</summary>

**Bước 1 — điều interviewer chờ nghe TRƯỚC khi bạn gõ phím:**

> *"Spec nói đúng 1 producer + 1 consumer ⟹ đây là **SPSC**. Ca này **không cần mutex**: mỗi chỉ số chỉ có **một** thread ghi, thread kia chỉ đọc. Dùng hai `std::atomic<size_t>` với **acquire/release** là đủ và không có lock. Nếu spec là nhiều producer thì tôi sẽ đổi sang mutex hoặc CAS — lock-free MPMC phức tạp hơn nhiều và tôi sẽ không tự viết nếu không có lý do đo đạc."*

Chính đoạn nói ra đó là thứ vá lỗi *"không hỏi spec mấy luồng"* của lần trước.

**Bước 2 — những quyết định thiết kế phải nêu:**

| Quyết định | Vì sao |
|---|---|
| `std::vector<T> buf_` cấp phát **trong constructor** | Không bao giờ cấp phát trong hot path |
| Sức chứa **luỹ thừa của 2** + `idx & mask_` | Thay `%` (phép chia, chậm trên MCU) bằng AND |
| Chỉ số **chạy tự do** (không wrap), chỉ mask khi truy cập | Phân biệt được đầy/rỗng mà không cần tốn 1 ô hoặc thêm cờ |
| `head_` chỉ producer ghi · `tail_` chỉ consumer ghi | Điều kiện để SPSC không cần lock |
| `alignas(64)` cho hai chỉ số | Chống **false sharing** — hai thread ghi hai biến cùng cache line làm cache ping-pong |

**Bộ khung:**
```cpp
#include <atomic>
#include <vector>
#include <cstddef>

template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity_pow2)
        : buf_(capacity_pow2), mask_(capacity_pow2 - 1) {
        // yêu cầu luỹ thừa của 2
    }

    // ---- Producer thread GỌI ----
    void push(const T& item) {
        const size_t h = head_.load(std::memory_order_relaxed); // chỉ producer ghi head_
        const size_t t = tail_.load(std::memory_order_acquire); // đọc tiến độ consumer
        if (h - t == buf_.size()) {                             // ĐẦY → đè cái cũ nhất
            tail_.store(t + 1, std::memory_order_release);      // bỏ 1 phần tử cũ
            dropped_.fetch_add(1, std::memory_order_relaxed);   // counter độc lập → relaxed đủ
        }
        buf_[h & mask_] = item;                                 // KHÔNG cấp phát
        head_.store(h + 1, std::memory_order_release);          // công bố sau khi ghi xong
    }

    // ---- Consumer thread GỌI ----
    bool pop(T& out) {
        const size_t t = tail_.load(std::memory_order_relaxed);
        const size_t h = head_.load(std::memory_order_acquire); // thấy h ⟹ thấy dữ liệu đã ghi
        if (h == t) return false;                               // RỖNG
        out = buf_[t & mask_];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    // ---- Gọi được từ CẢ HAI thread (snapshot, không phải giá trị "đúng mãi") ----
    bool   empty() const { return size() == 0; }
    bool   full()  const { return size() == buf_.size(); }
    size_t size()  const {
        return head_.load(std::memory_order_acquire) -
               tail_.load(std::memory_order_acquire);
    }
    size_t dropped_count() const { return dropped_.load(std::memory_order_relaxed); }

private:
    std::vector<T> buf_;
    size_t mask_;
    alignas(64) std::atomic<size_t> head_{0};   // chỉ producer ghi
    alignas(64) std::atomic<size_t> tail_{0};   // chỉ consumer ghi
    alignas(64) std::atomic<size_t> dropped_{0};
};
```

**Độ phức tạp:** `push` và `pop` đều **O(1)**, không cấp phát, không lock.

**⚠️ Điểm mà lần trước bạn bị trừ — `empty()`/`full()` "an toàn đa luồng" nghĩa là gì:**
Chúng an toàn ở nghĩa **không UB** (đọc atomic, không rách dữ liệu), **nhưng giá trị trả về là ảnh chụp tức thời** — vừa đọc xong `empty()==true` thì producer có thể đã push. Vì vậy **không được** viết:
```cpp
if (!rb.empty()) { rb.pop(x); }   // ❌ TOCTOU — sai tư duy
```
mà phải để `pop()` **tự** báo:
```cpp
if (rb.pop(x)) { ... }            // ✅ kiểm tra và lấy trong MỘT thao tác
```
Nói được ý này trong phỏng vấn quan trọng hơn viết đúng code.

**Đè cái cũ nhất — chỗ tinh tế:** ở SPSC, `tail_` lẽ ra chỉ consumer được ghi. Khi producer đè, nó phải ghi `tail_` ⟹ **phá vỡ giả định SPSC**. Bản trên chấp nhận đánh đổi này (đơn giản, chấp nhận đua hiếm gặp làm lệch một phần tử). **Nói ra đánh đổi này** — hoặc nêu phương án đúng hơn: consumer tự phát hiện bị bỏ xa (`h - t > capacity`) rồi tự nhảy `tail_`, để producer **không bao giờ** đụng `tail_`.

**Ôn:** [12-dsa/ring-buffer.md](../../../12-dsa/ring-buffer.md) §4 (chính sách khi đầy) → §7 (lock-free SPSC) · bank [COD-006](../bank/coding.md), [DSA-013](../bank/dsa.md), [DSA-014](../bank/dsa.md)
</details>

---

## Câu 11 · 🔴 · CPP-018

**a)** `std::atomic` có đủ để đồng bộ không, hay vẫn cần mutex? Khi nào dùng cái nào?
**b)** Cho đoạn này — nó **sai**, chỉ ra vì sao:
```cpp
std::atomic<int> count{0};
std::vector<int> data;

// Thread A
data.push_back(42);
count++;

// Thread B
if (count > 0)
    process(data.back());
```
**c)** Đổi `count` thành `std::atomic` có làm `data` an toàn không? Ranh giới giữa "atomic là đủ" và "phải có mutex" nằm ở đâu?

<details><summary>Đáp án</summary>

**(a) Hai thứ khác nhau:**

| | `std::atomic` | `mutex` |
|---|---|---|
| Bảo vệ | **một** biến, **một** thao tác | **một vùng code** / nhiều biến |
| Cơ chế | lệnh CPU atomic, không ngủ | có thể block, OS đánh thức |
| Chi phí | ~chục ns | ~trăm ns khi tranh chấp (syscall futex) |
| Dùng khi | counter, cờ, con trỏ đơn lẻ | **bất biến trải trên nhiều biến** |

**Ranh giới thật:** atomic đủ khi **bất biến (invariant) chỉ nằm trong một biến**. Cần mutex khi bất biến **trải trên nhiều biến** hoặc thao tác gồm **nhiều bước phải không tách rời**.

```cpp
std::atomic<int> a, b;
if (a > 0) { a--; b++; }   // ❌ mỗi lệnh atomic, nhưng CẢ KHỐI thì không
                           //    Thread khác chen giữa → a và b lệch nhau
```

**(b) Đoạn code sai ở hai tầng:**
1. **`data` không được bảo vệ.** `push_back` có thể **realloc** toàn bộ vector; thread B đọc `data.back()` giữa lúc đó → truy cập bộ nhớ đã giải phóng. `count` là atomic **không bảo vệ `data`**.
2. **Ngay cả khi `data` không realloc**: `count++` mặc định là `seq_cst` nên ở đây may mắn có ràng buộc thứ tự — nhưng nếu ai đó "tối ưu" thành `count.fetch_add(1, relaxed)` thì B thấy `count > 0` mà **chưa thấy** phần tử đã push (xem CPP-019). Đúng nghĩa: atomic đảm bảo **tính nguyên tử của count**, không đảm bảo **trạng thái của data**.

**Sửa:**
```cpp
std::mutex m;
// Thread A
{ std::lock_guard<std::mutex> lk(m); data.push_back(42); }
// Thread B
{ std::lock_guard<std::mutex> lk(m); if (!data.empty()) process(data.back()); }
```
Không cần `count` nữa — `data.size()` đã dưới sự bảo vệ của mutex.

**(c) Không.** `atomic<int> count` chỉ khiến **bản thân count** không bị đọc/ghi rách. Nó **không** dựng hàng rào quanh `data`. Đây là hiểu lầm phổ biến nhất về atomic: *"tôi đã atomic hoá cái cờ, nên dữ liệu đi kèm cũng an toàn"* — sai; muốn vậy phải dùng **release/acquire trên đúng cờ đó** và **không ai được sửa `data` sau khi công bố** (xem CPP-019, CPP-024).

**Chốt:** *"Atomic bảo vệ **một ô nhớ**. Mutex bảo vệ **một bất biến**. Hỏi 'bất biến của tôi gồm mấy biến?' — nhiều hơn một thì mutex."*

**Ôn:** [02-modern-cpp/concurrency.md](../../../02-modern-cpp/concurrency.md) · [03-operating-system/sync-primitives.md](../../../03-operating-system/sync-primitives.md)
</details>

---

## Câu 12 · 🔴 🏗️ · SD-009 — đúng việc bạn đang làm

**Đề:** Bạn thiết kế một **C++ shared library** cho lớp trên (app team) dùng. Ngoài chuyện code chạy đúng, bạn quan tâm những gì? Trình bày như đang thuyết phục tech lead.

**Đào sâu sẽ hỏi:** (1) API nên là C++ thuần hay bọc `extern "C"`? (2) Làm sao **giấu** implementation? (3) Khách hàng báo *"nâng thư viện lên là app crash"* — bạn hỏi lại họ điều gì đầu tiên?

<details><summary>Khung đáp án (câu 🏗️ — chấm theo khung, không đáp án duy nhất)</summary>

**Khung 5 trục để trả lời có tổ chức:**

**① Ranh giới API (cái khó nhất)**
- API càng **hẹp** càng dễ sống lâu. Mỗi hàm public là một lời hứa giữ mãi.
- **Không** để kiểu STL trong chữ ký nếu muốn ABI bền — `std::string`, `std::vector` có layout **khác nhau giữa các compiler/phiên bản** (libstdc++ vs libc++; `_GLIBCXX_USE_CXX11_ABI=0/1` đã từng gây đúng lớp bug này).
- Cân nhắc **`extern "C"` ở lớp ngoài** + C++ bên trong: C ABI ổn định, không mangling, gọi được từ Python/Rust/Java. Đánh đổi: mất kiểu mạnh, phải tự quản lý handle (`void*` hoặc struct opaque).

**② Giấu implementation**
- **Pimpl** — header chỉ khai báo `class Impl; std::unique_ptr<Impl> pimpl_;`. Thêm member vào `Impl` **không đổi kích thước** class public ⟹ không phá ABI. Chi phí: một lần cấp phát + một lần gián tiếp.
- **Visibility**: `-fvisibility=hidden` + đánh dấu `__attribute__((visibility("default")))` cho đúng những symbol muốn xuất. Giảm kích thước bảng symbol, tăng tốc load, và **ngăn khách hàng lỡ phụ thuộc vào internal**.
- **Version script** (`.map`) để khoá danh sách symbol xuất.

**③ Vòng đời & sở hữu**
- Ai cấp phát thì người đó giải phóng — **không** để khách `delete` con trỏ do thư viện `new` (khác heap/allocator ⟹ crash). Cung cấp `create()`/`destroy()` theo cặp.
- Không ném exception qua ranh giới `.so` nếu khách có thể build bằng compiler khác.

**④ Versioning**
- **SONAME** (`libfoo.so.1`) đổi khi **phá ABI**; minor (`libfoo.so.1.2`) khi tương thích ngược.
- Semantic versioning + tài liệu ghi rõ **cái gì được đảm bảo**, cái gì là internal.

**⑤ Thread-safety & lỗi**
- **Ghi rõ trong tài liệu** hàm nào thread-safe, hàm nào không — không để khách đoán.
- Chiến lược lỗi nhất quán: mã lỗi hay exception, chọn **một**.

**Ba câu đào sâu:**
1. *C++ thuần hay `extern "C"`?* — nội bộ cùng toolchain thì C++ thuần (kiểu mạnh, RAII). Phát hành ra ngoài / đa ngôn ngữ / cần ABI bền thì **C ở biên giới**.
2. *Giấu implementation?* — Pimpl + `-fvisibility=hidden` + version script (3 lớp).
3. *"Nâng thư viện là crash"* — hỏi đầu tiên: **"Anh có build lại app không, hay chỉ thay file `.so`?"** Nếu chỉ thay file thì gần như chắc chắn là **ABI break** (xem câu 13, 16), không phải bug logic.

**Ôn:** [07-shared-libraries/api-design.md](../../../07-shared-libraries/api-design.md), [abi-versioning.md](../../../07-shared-libraries/abi-versioning.md) · bank SD-009
</details>

---

## Câu 13 · 🔴 🏗️ · CPP-023 — ⚠️ chỗ bạn hụt ở câu 4

**a)** Vì sao **ABI của C++ không ổn định** giữa các compiler (thậm chí giữa các phiên bản của cùng một compiler)? Kể ít nhất 3 nguồn gây bất ổn.
**b)** Những thay đổi nào sau đây **phá ABI**, cái nào không?
1. Thêm một hàm public mới vào class
2. Thêm một **data member private** vào class
3. Thêm tham số có giá trị mặc định vào hàm public
4. Đổi `const std::string&` → `std::string_view`
5. Đổi thân hàm (không đổi chữ ký)
6. Thêm một hàm **virtual** mới vào cuối class

**c)** Thiết kế thư viện thế nào để **giảm** rủi ro này?

<details><summary>Đáp án</summary>

**(a) Nguồn gây bất ổn:**
1. **Name mangling** — không có chuẩn. `_Z5parseRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE` (Itanium ABI, GCC/Clang) hoàn toàn khác MSVC.
2. **Layout của class** — thứ tự member, padding, vị trí vptr, cách bố trí đa kế thừa/kế thừa ảo đều là quyết định của compiler.
3. **Layout của thư viện chuẩn** — `std::string` đổi layout giữa GCC 4 và GCC 5 (`_GLIBCXX_USE_CXX11_ABI`); libstdc++ và libc++ khác nhau hoàn toàn.
4. **Exception handling & RTTI** — cơ chế unwind, cách so sánh `type_info` khác nhau.
5. **Quy ước gọi hàm** — cái gì đi thanh ghi, cái gì đi stack; ai dọn stack.

**(b) Bảng — câu này phân loại người từng phát hành thư viện thật:**

| # | Thay đổi | Phá ABI? | Vì sao |
|---|---|---|---|
| 1 | Thêm hàm public **không virtual** | ✅ **An toàn** | Chỉ thêm symbol mới; code cũ không tham chiếu tới |
| 2 | Thêm **data member private** | ❌ **PHÁ** | Đổi `sizeof(class)`. Code cũ cấp phát theo kích thước cũ → tràn/hỏng bộ nhớ. **Đây là lý do Pimpl tồn tại** |
| 3 | Thêm tham số có **giá trị mặc định** | ❌ **PHÁ** | Giá trị mặc định điền tại **call site**, và **chữ ký đổi ⟹ mangled name đổi** |
| 4 | `const std::string&` → `string_view` | ❌ **PHÁ** | Mangled name đổi → `undefined symbol` lúc load |
| 5 | Đổi **thân hàm** | ✅ **An toàn** | Symbol y nguyên. Đây là lý do bản vá bảo mật phát hành được mà không cần build lại app |
| 6 | Thêm **virtual** mới vào cuối class | ❌ **PHÁ** | Đổi bố cục **vtable**. Class dẫn xuất do khách viết có vtable dựng theo bản cũ → gọi nhầm hàm |

**(c) Giảm rủi ro — 5 kỹ thuật:**
1. **Pimpl** — mọi state nằm trong `Impl`; class public chỉ có một con trỏ ⟹ `sizeof` cố định vĩnh viễn ⟹ mục (2) hết nguy hiểm.
2. **Không có hàm virtual** trong class public (hoặc khoá bằng `final`) ⟹ mục (6) hết nguy hiểm.
3. **Biên giới `extern "C"`** ⟹ không mangling, không layout class ⟹ mục (1)(3)(4) hết nguy hiểm.
4. **`-fvisibility=hidden` + version script** ⟹ khách không lỡ phụ thuộc internal.
5. **SONAME kỷ luật** ⟹ phá ABI thì bump major; loader từ chối nạp sai bản, **báo lỗi rõ ràng thay vì crash bí ẩn**.

**Chốt:** *"API là lời hứa với **người viết code**, ABI là lời hứa với **binary đã build**. Đổi thân hàm chỉ phá API-behavior; đổi sizeof hay vtable thì phá cả nhị phân đã phát hành."*

**Ôn:** [07-shared-libraries/abi-versioning.md](../../../07-shared-libraries/abi-versioning.md), [linking-loading.md](../../../07-shared-libraries/linking-loading.md)
</details>

---

## Câu 14 · 🔴 · CPP-021 (xuyên-topic: C++ + Pimpl + build)

Header thư viện:
```cpp
// widget.h
class Impl;                       // khai báo trước, chưa định nghĩa
class Widget {
public:
    Widget();
    void doSomething();
private:
    std::unique_ptr<Impl> pimpl_;
};
```
```cpp
// main.cpp
#include "widget.h"
int main() { Widget w; }          // ❌ LỖI COMPILE
```

**a)** Lỗi gì, và **vì sao** — `unique_ptr` vốn được quảng cáo là "chỉ cần forward declaration"?
**b)** Sửa thế nào? Chính xác thì phải thêm gì và **đặt ở đâu**?
**c)** Nếu đổi sang `std::shared_ptr<Impl>` thì có gặp lỗi này không? Vì sao khác nhau?

<details><summary>Đáp án</summary>

**(a) Lỗi:** `invalid application of 'sizeof' to incomplete type 'Impl'` — phát ra từ bên trong `~unique_ptr()`.

**Cơ chế:** `Widget` không khai báo destructor ⟹ compiler **sinh destructor inline ngay tại chỗ dùng** (`main.cpp`). Destructor đó phải huỷ `pimpl_` ⟹ gọi `~unique_ptr<Impl>()` ⟹ gọi `default_delete<Impl>` ⟹ thực thi `delete ptr` ⟹ **`delete` một incomplete type**. Chuẩn yêu cầu kiểu phải **complete** tại điểm `delete`, vì phải biết destructor nào cần gọi và kích thước để giải phóng. Ở `main.cpp` thì `Impl` mới chỉ được khai báo trước.

> Nghĩa là: `unique_ptr` **khai báo** được với incomplete type; nhưng **huỷ** thì cần complete type. Nơi cần complete là **nơi destructor được sinh ra**, không phải nơi khai báo.

**(b) Sửa — hai phần, thiếu một là vẫn hỏng:**
```cpp
// widget.h
class Widget {
public:
    Widget();
    ~Widget();                    // ① KHAI BÁO, không định nghĩa ở đây
    Widget(Widget&&) noexcept;    // (dtor tự viết ⟹ move không tự sinh — Rule of 5!)
    Widget& operator=(Widget&&) noexcept;
private:
    std::unique_ptr<Impl> pimpl_;
};
```
```cpp
// widget.cpp
#include "widget.h"
class Impl { /* ... định nghĩa đầy đủ ... */ };

Widget::Widget() : pimpl_(std::make_unique<Impl>()) {}
Widget::~Widget() = default;                              // ② ĐỊNH NGHĨA Ở ĐÂY,
Widget::Widget(Widget&&) noexcept = default;              //    nơi Impl đã complete
Widget& Widget::operator=(Widget&&) noexcept = default;
```
**Điểm mấu chốt:** `= default` phải nằm trong **`.cpp`**, sau định nghĩa `Impl`. Viết `~Widget() = default;` trong header thì y hệt như không viết gì — vẫn lỗi.

**Bẫy kép:** khai báo destructor ⟹ **move ctor/assign không được sinh** ⟹ `Widget` mất khả năng move. Phải khai báo lại cả hai (đúng nội dung **câu 6** của phiên này).

**(c) `shared_ptr` KHÔNG gặp lỗi này.** Vì `shared_ptr` **xoá kiểu (type erasure) của deleter**: deleter được ghi vào **control block ngay lúc constructor chạy** — thời điểm đó `Impl` đã complete (trong `.cpp`). `~shared_ptr` chỉ gọi deleter đã lưu, không cần biết kiểu.

Đánh đổi: `shared_ptr` nặng hơn (control block + đếm atomic) và **chia sẻ quyền sở hữu** — sai ngữ nghĩa cho Pimpl (mỗi `Widget` phải sở hữu riêng `Impl`). **Vẫn nên dùng `unique_ptr`** và viết đúng 4 dòng ở trên.

**Chốt:** *"`unique_ptr` gọi `delete` tại nơi destructor được sinh. Pimpl thì phải kéo nơi đó vào `.cpp` — bằng cách khai báo dtor ở header, định nghĩa `= default` ở cpp. Và nhớ khai báo lại move."*

**Ôn:** bank CPP-021, DP-007 (Pimpl/Bridge) · [11-design-patterns/structural.md](../../../11-design-patterns/structural.md)
</details>

---

## Câu 15 · 💻 CODING 2 · CPP-054 (SỔ YẾU — 2 điểm ngày 10/08)

> Lỗi lần trước: **move ctor đọc `fd_` chưa khởi tạo** (UB, có thể `close()` fd của module khác) · thiếu `noexcept` · `<bits/stdc++.h>` trong header thư viện.
> **⛔ Không mở `reviewed/2026-08-10--CPP-045--device-handle.cpp` trước khi làm xong.**

**Đề:** viết class RAII quản lý một **socket fd** cho thư viện của bạn.
- Cấm copy, cho phép move.
- Có `get()`, `release()` (nhả quyền sở hữu **không đóng**), `reset(int newFd)`.
- Viết vào `coding-arena/socket_handle.cpp`, có `main()` test **≥4 ca biên**: move ctor, move assign, **self-move-assign**, dùng trong `std::vector`.

**Câu hỏi kèm (trả lời trước khi viết):** move constructor và move assignment khác nhau **chỗ nào**? Vì sao khác?

<details><summary>Đáp án — CHỈ mở sau khi viết xong</summary>

**Bảng phải thuộc:**

| | Move **CONSTRUCTOR** | Move **ASSIGNMENT** |
|---|---|---|
| Object đích | **CHƯA tồn tại** (đang được xây) | **ĐÃ tồn tại**, đang giữ tài nguyên |
| Member lúc vào hàm | **RÁC** (kiểu built-in không có initializer) | Giá trị hợp lệ |
| Dọn tài nguyên cũ? | ❌ **Không có gì để dọn** | ✅ **BẮT BUỘC**, không thì leak fd |
| Cần `this != &other`? | ❌ Không thể tự-move-construct | ✅ Có |

```cpp
#include <unistd.h>
#include <utility>

class SocketHandle {
public:
    explicit SocketHandle(int fd = -1) noexcept : fd_(fd) {}
    ~SocketHandle() { if (fd_ != -1) ::close(fd_); }

    SocketHandle(const SocketHandle&)            = delete;   // public → message lỗi rõ
    SocketHandle& operator=(const SocketHandle&) = delete;

    // Đích CHƯA tồn tại → KHÔNG dọn. Làm ở member-init-list nên không có
    // khoảnh khắc nào fd_ chưa khởi tạo.
    SocketHandle(SocketHandle&& o) noexcept
        : fd_(std::exchange(o.fd_, -1)) {}

    // Đích ĐÃ tồn tại và đang giữ fd → PHẢI dọn trước.
    SocketHandle& operator=(SocketHandle&& o) noexcept {
        if (this != &o) {
            if (fd_ != -1) ::close(fd_);
            fd_ = std::exchange(o.fd_, -1);
        }
        return *this;
    }

    int  get() const noexcept { return fd_; }
    int  release()   noexcept { return std::exchange(fd_, -1); }
    void reset(int newFd = -1) noexcept {
        if (fd_ != -1 && fd_ != newFd) ::close(fd_);
        fd_ = newFd;
    }
private:
    int fd_ = -1;      // ⭐ default member initializer — diệt tận gốc lớp bug "đọc rác"
};
```

**Lỗi kinh điển (đúng lỗi bạn mắc 10/08):**
```cpp
SocketHandle(SocketHandle&& o) {
    if (fd_ != -1) ::close(fd_);   // ❌ fd_ CHƯA khởi tạo — đọc rác, UB
    fd_ = o.fd_; o.fd_ = -1;
}
```
`SocketHandle b = std::move(a);` — ctor `SocketHandle(int)` **không chạy** cho `b`. Nếu rác trên stack tình cờ khác `-1`, bạn `::close()` **fd của module khác** → socket/file ở nơi không liên quan đột nhiên chết, backtrace vô hại.

**`noexcept` không phải trang trí:** `std::vector<SocketHandle>` khi realloc chỉ dùng move nếu move ctor `noexcept` (strong exception guarantee). Không `noexcept` → fallback sang **copy** → copy đã `= delete` → **không compile**. Move-only + quên `noexcept` = class vô dụng trong container.

**`<bits/stdc++.h>`:** header nội bộ libstdc++ — clang/musl không có, kéo cả STL vào mọi TU. Dùng `<unistd.h>` + `<utility>`.

**Ôn:** bank CPP-054, CPP-045 · [02-modern-cpp/move-semantics.md](../../../02-modern-cpp/move-semantics.md)
</details>

---

## Câu 16 · 🔴 🏗️ · SD-010 (xuyên-topic — tình huống thật)

**Tình huống:** Thư viện `libscan.so.1` của bạn đang chạy ở khách hàng. Bạn phát hành bản mới. Khách báo:

> *"Chúng tôi chỉ copy file `.so` mới đè lên bản cũ, không build lại app. Giờ app crash ngay khi khởi động — có lúc lại chạy được vài phút rồi hỏng dữ liệu."*

**a)** Bạn hỏi lại khách những gì để khoanh vùng? Liệt kê theo thứ tự ưu tiên.
**b)** Hai triệu chứng khác nhau (*crash ngay* vs *chạy rồi hỏng dữ liệu*) gợi ý **hai loại thay đổi khác nhau**. Là gì?
**c)** Bạn dùng công cụ nào để **chứng minh** nguyên nhân?
**d)** Sửa ngắn hạn cho khách + sửa dài hạn cho quy trình?

<details><summary>Đáp án</summary>

**(a) Câu hỏi khoanh vùng, theo thứ tự:**
1. **"`ldd app` và `readelf -d app | grep NEEDED` ra gì?"** — app đang cần SONAME nào? Nếu app cần `libscan.so.1` mà bạn phát hành bản đổi ABI **không bump SONAME**, đó là lỗi của bạn.
2. **"Bản cũ và bản mới build bằng compiler/libstdc++ nào?"** — đổi GCC major hoặc `_GLIBCXX_USE_CXX11_ABI` là đủ phá.
3. **"Crash có backtrace/core dump không? Ngay `main` hay sau một lời gọi cụ thể?"**
4. **"App có kế thừa class nào của thư viện không?"** — nếu có, vtable là nghi phạm số 1.
5. **"Có dùng `dlopen` không, hay link lúc build?"**

**(b) Hai triệu chứng → hai lớp nguyên nhân:**

| Triệu chứng | Nguyên nhân điển hình | Cơ chế |
|---|---|---|
| **Crash ngay khi khởi động** | **Symbol biến mất** — đổi/xoá chữ ký hàm public | Dynamic linker không resolve được → `symbol lookup error` ngay lúc **load**, trước cả `main()` |
| **Chạy được rồi hỏng dữ liệu** | **Layout đổi** — thêm data member (đổi `sizeof`), thêm virtual (đổi vtable), đổi thứ tự member | Linker **resolve thành công** (symbol vẫn còn), nhưng app cấp phát theo `sizeof` cũ / gọi vtable slot cũ → **ghi đè bộ nhớ lân cận**, hỏng dần |

Loại thứ hai **nguy hiểm hơn nhiều** vì nó im lặng. Đây là lý do bump SONAME quan trọng: nó biến lỗi âm thầm thành lỗi **báo ngay lúc load**.

**(c) Công cụ chứng minh:**
```bash
# 1. So sánh symbol xuất giữa hai bản
nm -D --defined-only libscan.so.1.old | sort > old.txt
nm -D --defined-only libscan.so.1.new | sort > new.txt
diff old.txt new.txt          # symbol nào biến mất?

# 2. Công cụ chuyên dụng — trả lời thẳng "có phá ABI không"
abidiff libscan.so.1.old libscan.so.1.new       # libabigail
# hoặc: abi-compliance-checker

# 3. Xem app cần symbol nào
readelf -d app | grep NEEDED
ldd -r app                    # liệt kê symbol chưa resolve

# 4. Bắt lỗi lúc chạy
LD_DEBUG=bindings,libs ./app 2>&1 | head -50
gdb ./app core                # backtrace từ core dump
valgrind --tool=memcheck ./app   # bắt ghi đè do sizeof lệch
```
`abidiff` là câu trả lời ghi điểm — nó in thẳng *"function ... has some sub-type changes"*.

**(d) Sửa:**

*Ngắn hạn cho khách:*
- Rollback về `.so` cũ ngay (giảm thiệt hại).
- Nếu bắt buộc dùng bản mới: **build lại app** với header mới. Không có đường tắt.

*Dài hạn cho quy trình:*
1. **Chạy `abidiff` trong CI** — mọi PR chạm public header đều bị kiểm; phá ABI thì fail build.
2. **Kỷ luật SONAME:** phá ABI ⟹ `libscan.so.2`. Hai bản **cùng tồn tại** được, app cũ chạy bản cũ.
3. **Pimpl + `-fvisibility=hidden` + version script** để bề mặt ABI nhỏ nhất có thể (câu 12, 13).
4. **Tài liệu hoá cam kết**: cái gì thuộc API công khai, cái gì internal, chính sách hỗ trợ bao lâu.
5. Không có hàm **virtual** trong class public.

**Chốt:** *"'Chỉ copy .so mới' chỉ hợp lệ khi ABI không đổi. Nhiệm vụ của người phát hành thư viện là **làm cho việc đó hoặc an toàn, hoặc thất bại ồn ào** — điều tệ nhất là nó chạy được một lúc."*

**Ôn:** [07-shared-libraries/abi-versioning.md](../../../07-shared-libraries/abi-versioning.md) · [09-debugging/tools.md](../../../09-debugging/tools.md) · bank SD-010
</details>

---

# 📌 Ghi chú cho lần ôn / mock lại

## Lỗ hổng đã lộ trong 9 câu đầu (chưa chấm điểm — chỉ là quan sát)

| # | Lỗ hổng | Câu liên quan |
|---|---|---|
| 1 | 🔴 **ABI của shared library** — không biết đổi chữ ký hàm phá ABI, không biết lỗi xảy ra lúc **load** | Câu 4 (follow-up) → sẽ gặp lại ở **câu 13, 16** |
| 2 | 🔴 **`unique_ptr` member ⟹ class move-only** — bỏ sót member quyết định, kết luận sai "lỗi runtime" và "vẫn copy được" | Câu 6 |
| 3 | 🟠 **Phương pháp kiểm chứng** — comment mất chính biến đang đo rồi kết luận "không có vấn đề gì" | Câu 6 |
| 4 | 🟠 **RVO vs NRVO** — không biết C++17 chỉ bắt buộc elision cho **prvalue không tên** | Câu 7 |
| 5 | 🟡 **`thread_local`** — không nêu được từ khoá; chưa biết chi phí TLS trong `.so` | Câu 3 |

## Điểm mạnh quan sát được
- **Câu 8 (lambda capture)** và **câu 4a/b (string_view)** trả lời ở mức tốt, truy đúng cơ chế dangling.
- **Câu 9 (CPP-032)** — phục hồi rõ so với lần trước: tự nhận diện safe-bool problem qua code review, không cần gợi ý.
- Khi bị chỉ ra sai (câu 6), **tổng quát hoá được ngay** sang trường hợp gốc — kỹ năng lần trước bị trừ điểm (09/08) nay đã cải thiện.

## Khi mock lại phiên này
- **Không lặp nguyên văn** các đề trên (config §6). Đổi góc: câu 6 → đưa class đã sửa rồi hỏi *"vì sao `vector::push_back` giờ chạy được"*; câu 9 → đổi sang `operator int()` thay vì `operator bool()`.
- **Chưa cập nhật** `weak-register.md` và **chưa tick** buổi CN trong plan — phiên chưa hoàn tất, không chấm nửa vời.
- Hai bài coding (câu 10, 15) **bắt buộc làm từ file trống**, không mở `reviewed/`.
