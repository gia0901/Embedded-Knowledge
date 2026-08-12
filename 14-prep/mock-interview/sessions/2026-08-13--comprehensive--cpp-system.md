# Phiên mock — 2026-08-13 · comprehensive · track cpp-system

- **Level:** mid-level · **Trần độ sâu:** **T2** · **Số câu:** 15/16 (dừng trước bài coding theo yêu cầu) · ~70 phút
- **Điểm trung bình:** **3.0 / 4**
- **Bối cảnh:** Buổi CN Tuần 1 phần 2 — chốt Tuần 1, theo [datalogic-plan](../../study-plans/datalogic-plan.md). Phiên chạy lại sau lần [tạm dừng 10/08](2026-08-10--comprehensive--cpp-system.md), **bộ câu hoàn toàn mới**, các câu sổ yếu/retention đều đổi góc.
- **Câu 16 (coding `SocketHandle`) hoãn sang buổi sau** — chưa làm, chưa chấm.

## Kết quả từng câu

| # | ID | Câu (tóm tắt) | Điểm | Kết quả |
|---|----|---------------|------|---------|
| 1 | OS-001 | process vs thread (daemon USB + log) | **4** | Nêu đúng đánh đổi độ tin cậy 24/7; COW đúng |
| 2 | CPP-030 | size/capacity + invalidation | **3** | (c) trắng, phải gợi ý mới ra |
| 3 | CPP-031 | `const` member + `mutable` | **3** | Thiếu khái niệm *logical constness* |
| 4 | CPP-011 | move ctor `noexcept` | **3** | Không gọi tên *strong exception guarantee* |
| 5 | CPP-014 | rvalue vs universal reference | **4** | Chính xác cả điều kiện lẫn ca (X)(Y) |
| 6 | CPP-016 | object slicing | **2** | 🔴 **VÀO SỔ** — tầng thiết kế phòng ngừa trắng |
| 7 | CPP-049 | move không phải luôn rẻ | **3** | Tự mâu thuẫn (a)↔(b), gỡ được khi bị chỉ ra |
| 8 | OS-020 | thread-safe vs reentrant | **2** | 🔴 **VÀO SỔ** — sai về async-signal-safety |
| 9 | CPP-017 | template vs virtual | **3** | "19 bản không dùng" sai; thin-template chưa biết |
| 10 | SD-016 | bộ nhớ tất định | **2** | 🔴 **VÀO SỔ** — (b) cần gợi ý mạnh; phần STL trắng |
| 11 | DP-011 | DIP / testability | **3** | Chưa trả lời "ai sở hữu interface" |
| 12 | **CPP-032** | explicit — góc *hai chiều ngầm* | **3** | 🔼 2→3 (1/2 lần), **vẫn trong sổ** |
| 13 | **CPP-020** | Rule of 0 — góc *dtor vô hại* | **4** | ✅ retention đạt → dời Tuần 4 |
| 14 | **CPP-029** | emplace — góc *rò rỉ + khi nào vô ích* | **3** | ✅ retention đạt → dời Tuần 4 |
| 15 | CPP-052 | chẩn đoán race `shared_ptr` | **3** | Lập luận đúng hướng |

**Sổ yếu: 2 → 5 câu** (thêm CPP-016, OS-020, SD-016; CPP-032 lên 1/2; CPP-054 chưa hỏi).

---

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-016 — object slicing (điểm **2**) 🔴 ưu tiên cao

**Đề:** `struct Scanner : Device`; `void handle(Device d)` nhận **theo giá trị**; `devices.push_back(s)`.

**Làm được:** gọi đúng tên *object slicing*; biết `d.reset()` chạy bản `Device`.

**Chỉnh cơ chế (b):** ứng viên nói *"s bị slicing nên vptr trỏ vtable Base"*. Chính xác hơn: **vptr không bao giờ được copy**. Khi truyền theo giá trị, cái chạy là `Device::Device(const Device&)` — nó **dựng một object `Device`**, và constructor của `Device` tự đặt vptr trỏ vtable của `Device`. Phần `Scanner::cfg_` không được copy vì tham số chỉ có kích thước `sizeof(Device)`.

**Hụt (c) — câu hỏi là "thiết kế để KHÔNG THỂ xảy ra", ứng viên chỉ sửa call site.** Cách chặn từ phía class:

```cpp
struct Device {
    virtual void reset() = 0;            // ① abstract ⟹ không tạo Device theo giá trị được
    virtual ~Device() = default;
protected:
    Device(const Device&)            = default;   // ② copy chỉ dành cho lớp con
    Device& operator=(const Device&) = default;
};
```
Sau đó `void handle(Device d);` **lỗi ngay tại khai báo hàm** (`abstract type cannot be used as parameter`). Đồng nghiệp tuần sau không viết được nữa — đó mới là *"không thể xảy ra"*.

> **Trích bank CPP-016:** *"Slicing = copy một object lớp con vào biến lớp cha ⟹ mất phần dẫn xuất + mất tính đa hình."*

**Chốt:** *"Sửa call site là dán băng; sửa class là bịt lỗ. Base đa hình nên abstract, và copy nên `protected`."*

**Ôn:** [01-cpp-fundamentals/oop.md](../../../01-cpp-fundamentals/oop.md) · bank CPP-016, CPP-010

---

### OS-020 — thread-safe vs reentrant (điểm **2**) 🔴 ưu tiên cao

**Đề:** `logEvent` có `lock_guard` + `counter++` + `write`. Thread-safe? Reentrant? Gọi từ signal handler thì sao? Viết bản dùng được trong handler.

**Làm được:** phân biệt đúng hai khái niệm (*"thread-safe = nhiều thread gọi đồng thời; reentrant = bị cắt ngang rồi gọi lại"*); nhận đúng **self-deadlock**.

**❌ Sai ở phần vận dụng — hai chỗ:**

1. **Nói `write()` không dùng được trong signal handler.** Sai — `write()` **LÀ** async-signal-safe, nằm trong danh sách POSIX (`man 7 signal-safety`). Nó là **syscall trần**: không khoá, không buffer userspace.
2. **Đề xuất họ `_r` để đạt signal-safety.** `_r` sinh ra cho **thread-safety** (bỏ static buffer): `strtok_r`, `localtime_r`. Hai tính chất **giao nhau nhưng khác nhau** — `localtime_r` thread-safe nhưng **vẫn không** async-signal-safe.

| ✅ Async-signal-safe | ❌ KHÔNG (dù thread-safe) | Vì sao cấm |
|---|---|---|
| `write`, `read`, `open`, `close` | `printf`, `fprintf` | buffer + khoá stdio |
| `_exit`, `kill`, `sigaction` | `malloc`, `free`, `new` | khoá heap |
| `time`, `sem_post` | `localtime`, `strerror` | trả con trỏ tới static buffer |

**Nguyên tắc tự suy (thay cho học thuộc):** hàm nào **lấy khoá** hoặc **đụng trạng thái toàn cục có thể đang dở dang** thì cấm — vì handler chen vào **giữa** thao tác của luồng chính.

**Bản đúng:**
```cpp
volatile sig_atomic_t counter = 0;
void logEventSafe(const char* msg, size_t len) {
    counter++;                              // không khoá
    write(STDERR_FILENO, msg, len);         // ✅ hợp lệ
}
```
**Mẫu thực chiến:** handler chỉ đặt cờ hoặc `write()` một byte vào **self-pipe**, việc nặng để event loop chính làm.

> Bank [OS-020](../bank/os.md) **đã được viết lại** trong phiên này (theo góp ý của ứng viên: đáp án cũ sơ sài, thiếu code) — nay có ví dụ đầy đủ + bảng tra + cảnh báo `_r` ≠ signal-safe.

**Ôn:** [04-linux-system-programming/processes-signals.md](../../../04-linux-system-programming/processes-signals.md) · bank OS-020, LNX-011

---

### SD-016 — bộ nhớ tất định (điểm **2**) 🔴

**Làm được — phần khó đã có:** nêu **đúng cả hai** vấn đề: **phân mảnh** (chết dù tổng RAM còn đủ) và **thời gian cấp phát bất định**. Biết hướng **pool/arena cấp một lần**.

**Hụt (b): vì sao "delete cẩn thận" không chữa được phân mảnh.** Cần gợi ý mạnh mới ra.

```
 [100][1000][100][1000][100][1000][100]
   ↓ free HẾT khối 1000 (không rò rỉ byte nào)
 [100][ 900][100][ 900][100][ 900][100]     ← tổng trống 2700
 Xin 2000 byte → ❌ THẤT BẠI
```
**Allocator không gom được** vì muốn gom phải **dời** các khối 100 đang dùng — mà C/C++ phơi bày **địa chỉ thật**, chương trình đang giữ con trỏ trỏ thẳng vào đó; dời là mọi con trỏ thành rác, allocator không có cách tìm chúng để sửa. Ngôn ngữ GC nén được vì dùng **handle** chứ không phải địa chỉ trần.

⇒ Phân mảnh là hệ quả của **mẫu cấp phát**, không phải **kỷ luật giải phóng**.

**Hụt (c): "code đã lỡ dùng STL khắp nơi" → trắng.** Câu trả lời là **`std::pmr`** (C++17) — giữ nguyên container, chỉ đổi nguồn cấp phát (đã biên dịch + chạy thật):

```cpp
static std::array<std::byte, 64*1024> pool;
std::pmr::monotonic_buffer_resource res{pool.data(), pool.size()};

std::pmr::vector<Sample> samples{&res};     // vẫn là vector
std::pmr::string         name{&res};
res.release();                              // reset cả pool trong O(1)
```
**Đánh đổi:** thêm một lần gián tiếp qua vtable mỗi lần cấp phát; đổi lại tất định + không phải sửa logic.

> Bank [SD-016](../bank/system-design.md) **đã viết lại** kèm sơ đồ phân mảnh, giải thích cơ chế và code `pmr` đã chạy.

**Ôn:** [08-embedded-systems/constraints.md](../../../08-embedded-systems/constraints.md) · [11-design-patterns/creational.md](../../../11-design-patterns/creational.md) (Object Pool)

---

### CPP-032 — explicit, góc *hai chiều ngầm* (điểm **3**, sổ yếu → 1/2 lần ≥3)

**Đề:** lớp `Timeout` có **cả** `Timeout(int)` **và** `operator int()`, đều không `explicit`.

**Làm được:** `a + b` → `int` 300; `wait(a+b)` → 300 quay ngược thành `Timeout`; `retry(a)` chạy **100 lần** trong khi `a` nghĩa là **100 ms** — và nêu đúng **vì sao tệ hơn** ca `Status` hôm trước: giá trị *"đúng con số người viết nghĩ tới, chỉ sai đơn vị"* nên log trông bình thường, bug sống lâu.

**Ý (c) — nhập nhằng: trả lời ĐÚNG** sau khi được đưa bằng chứng: `a == 100` có **hai đường cùng hạng** (`100→Timeout` dùng `operator==` của bạn · `a→int` dùng `==` dựng sẵn) → compiler không chọn được.

**❌ Ca ứng viên tự nêu thì SAI:** cho rằng `Timeout c(a)` gọi `Timeout(int)` qua đường `a→int`. Chạy thật:

```
File của ứng viên:            Cùng file + log ở copy ctor:
  ctor 1 arg  ← a               ctor 1 arg  ← a
  ctor 1 arg  ← b               ctor 1 arg  ← b
  ctor 1 arg  ← "c(a)"?         COPY ctor   ← Timeout c(a)   ✅
  300                           ctor 1 arg  ← wait(a+b): 300 → Timeout
                                300
```
**Copy ctor là exact match, luôn thắng** chuỗi chuyển đổi do người dùng định nghĩa. Dòng log thứ ba thực ra của `wait(a+b)`.

**⚠️ Lỗi phương pháp (lần thứ 2):** copy ctor **ngầm** không in gì, nên phép đo chỉ-log-một-ctor **không phân biệt được hai giả thuyết**; ba dòng log khớp ba object chỉ là trùng hợp. Cách chặn: `static_assert(std::is_copy_constructible_v<Timeout>)`, hoặc log ở **cả hai** constructor.

**Câu mới thêm bank: [CPP-055](../bank/cpp.md)** — chuyển ngầm hai chiều + nhập nhằng.

**Lần sau hỏi (góc thứ tư):** `explicit` cho ctor **nhiều đối số** (C++11 cho phép) + braced-init.

---

### Ghi chú gọn các câu điểm 3 còn lại

- **CPP-030** — (a)(b) chuẩn, đặc biệt nhận định *"đổi 200 thành 50 thì đúng nhưng không yên tâm"* là **đúng chất kỹ sư**. (c) ban đầu trắng; sau gợi ý nêu được 3 hướng (mảng tĩnh · sức chứa cố định/ring buffer · container không realloc). Bổ sung: câu chuẩn là **`std::deque`/`std::list`** (tham chiếu bền), hoặc `vector<unique_ptr<T>>`, hoặc **lưu chỉ số thay vì con trỏ**.
- **CPP-031** — thiếu tên khái niệm **logical constness** (vs *bitwise* constness). Tiêu chí chặt cho `mutable`: *"gọi hàm `const` hai lần với cùng đầu vào, **người gọi** có quan sát thấy khác biệt không?"* — không ⟹ `mutable` chính đáng. Về `m_`: lý do bắt buộc `mutable` là **`lock()` sửa chính object mutex**.
- **CPP-011** — thiếu tên **strong exception guarantee**. Trạng thái vùng cũ khi move ném: ứng viên nói *"5000 phần tử bị leak"* — sai; chúng **vẫn ở vùng cũ nhưng đã bị moved-from (rỗng ruột)**, nên vector không thể trả về nguyên trạng. Với **copy** thì bản gốc còn nguyên ⟹ rollback được. Đó là lý do vector chọn copy khi move không `noexcept`.
- **CPP-049** — (a) và (b) tự mâu thuẫn (nói `array` move O(N) nhưng lại xếp vào "thực sự move"), gỡ đúng khi bị chỉ ra. `const std::string` không move được vì `std::move` cho ra `const string&&`, không bind được vào `string&&` ⟹ rơi về `const string&` ⟹ copy.
- **CPP-017** — (c) nói *"19 bản không được sử dụng"* là sai: có 20 loại sensor thì cả 20 bản **đều dùng**; bloat vì mỗi bản là một bản sao đầy đủ của thuật toán. Kỹ thuật giảm bloat chưa biết: **thin template / fat function** — đẩy phần **không phụ thuộc kiểu** vào một hàm thường, template chỉ còn vỏ mỏng.
- **DP-011** — nêu đúng "cả hai tầng phụ thuộc abstraction" nhưng chưa trả lời **ai sở hữu interface**: **module cấp cao định nghĩa** `ISensorPort.h` (nó khai báo *cái nó cần*), driver **implement**. Đó mới là "inversion" — thường thì tầng thấp định nghĩa cái nó *cung cấp* rồi tầng cao phải theo. Phần DIP không tốn runtime: **template injection** — `BarcodeReader<MockPort>` cho test, `BarcodeReader<RealPort>` cho production (nối với câu 9).
- **CPP-052** — lập luận chẩn đoán đúng hướng (writer đang đổi con trỏ, reader copy nửa chừng → double free). Chưa nêu được **các bước** của `g_cfg = ...`: giảm count cũ → ghi **hai** con trỏ (payload + control block) → tăng count mới; khoảng giữa ba bước đó là chỗ thread kia chen vào.

---

## 💬 Góp ý của ứng viên về chất lượng bank (3 lần trong phiên)

> *"câu trả lời trong bank còn sơ sài, chỉ khẳng định, thiếu code, đọc khó hiểu"* — nêu ở OS-020, SD-016, DP-011.

✅ **Đồng ý.** Đã viết lại **OS-020** và **SD-016** ngay trong phiên (thêm code chạy được, sơ đồ, bảng tra, giải thích *vì sao* thay vì khẳng định). **DP-011 còn nợ** — ghi vào việc cần làm.

**Tiêu chuẩn cho đáp án bank từ nay:** mỗi câu 🟠🔴 phải có (1) **cơ chế** giải thích *vì sao*, không chỉ kết luận; (2) **code minh hoạ** đã biên dịch được; (3) **bảng đối chiếu** khi có nhiều phương án; (4) câu **chốt** một dòng.

## ⚠️ Lỗ hổng phương pháp — lặp lại lần thứ 2

| Phiên | Sự việc |
|---|---|
| 10/08 câu 6 | Comment mất `unique_ptr` (chính biến đang đo) rồi kết luận *"không có vấn đề gì"* |
| 13/08 câu 12 | Log chỉ ở một ctor, không phân biệt được copy ctor ngầm (im lặng) với `ctor(int)` |

**Cùng một dạng:** kết luận từ phép đo **không đủ sức phân biệt hai giả thuyết**.
**Cách chặn — hỏi trước khi kết luận:** *"nếu giả thuyết kia đúng, kết quả có KHÁC đi không?"* Không khác ⟹ phép đo vô nghĩa.
Đây là kỹ năng debug — mảng ứng viên tự nhận là điểm yếu; xuất hiện 2/2 phiên nên tách thành mục riêng để theo dõi.

## Tổng kết

**Điểm mạnh:**
- **CPP-014 (4đ)** — universal reference: đúng cả điều kiện lẫn hai ca bẫy (X)(Y), không cần gợi ý.
- **OS-001 (4đ)** — tự đưa ra đánh đổi *"2 process cho thiết bị 24/7, đổi tài nguyên lấy độ tin cậy"*, đúng chất T2.
- **CPP-020 (4đ)** retention — nhận ngay dtor "vô hại" giết move.
- Khi bị chỉ ra sai thì **gỡ nhanh và tổng quát hoá được** (câu 7, câu 12).

**3 lỗ hổng ưu tiên:**
1. 🔴 **async-signal-safety** — [OS-020](../bank/os.md), [processes-signals.md](../../../04-linux-system-programming/processes-signals.md)
2. 🔴 **Thiết kế phòng ngừa (chặn ở class, không vá ở call site)** — [CPP-016](../bank/cpp.md), [oop.md](../../../01-cpp-fundamentals/oop.md)
3. 🟠 **Bộ nhớ tất định + `std::pmr`** — [SD-016](../bank/system-design.md), [constraints.md](../../../08-embedded-systems/constraints.md)

**Cập nhật đã làm:** sổ yếu **2 → 5** (+CPP-016, +OS-020, +SD-016; CPP-032 lên 1/2) · retention CPP-020 (4) và CPP-029 (3) **đạt → dời Tuần 4** · bank **+CPP-055**, **viết lại OS-020 + SD-016** · plan §📍.

**Nợ:** viết lại đáp án **DP-011** theo tiêu chuẩn mới · **câu 16 coding `SocketHandle`** chưa làm.

## Phiên kế đề xuất
- **Buổi sau:** `/mock coding track cpp-system` — làm `SocketHandle` (CPP-054, sổ yếu) + `ring_buffer_v2` bản mutex (COD-006). Cả hai **làm từ file trống**, không mở `reviewed/`.
- **Tuần 2 Buổi 1:** `/mock rapid track linux-sysprog` — và OS-020 sẽ được chèn lại ở đó (đúng domain).
