# Memory Management — *C++ High Performance* ch. 7 (tr. 191–236) 🎯

> **Nguồn summary:** đã đối chiếu PDF *C++ High Performance, 2nd Edition* (Andrist & Sehr, Packt 12/2020). **Offset: trang PDF = trang sách + 27.**
> **Vì sao cụm này:** đây là chương **trúng embedded nhất của cả cuốn**. Repo đã nói *"đừng dùng heap trong đường nóng"* ([08/constraints.md](../../08-embedded-systems/constraints.md)) nhưng chưa nói **dùng gì thay thế và viết ra sao**. Chương này trả lời: arena cấp phát trên **stack**, allocator có state, và `std::pmr` — thứ C++17 chuẩn hoá đúng cho bài toán này.

---

## TL;DR cụm này

1. **Stack nhanh không phải vì phép màu** — nó là allocator đơn giản nhất có thể: một con trỏ chạy tới/lui, không đồng bộ đa luồng, không phân mảnh. Mọi allocator tự viết đều đang cố bắt chước tính chất đó trong một phạm vi hẹp hơn.
2. **Alignment và padding đổi được `sizeof` mà không đổi một byte dữ liệu nào** — chỉ cần sắp lại thứ tự khai báo member. Sách đo được **24 → 16 byte**.
3. **`new` làm HAI việc** (cấp phát + dựng object). Tách được bằng **placement new** — và đây là lý do `operator new` riêng của class **không được gọi** khi dùng `make_shared` hay `vector::reserve`.
4. **Allocator là một phần của KIỂU.** `std::vector<int>` và `std::vector<int, MyAlloc>` là hai kiểu khác nhau, không truyền cho nhau được. C++17 chữa bằng `std::pmr` — thêm một tầng gián tiếp để mọi container `pmr` dùng chung một allocator.
5. **PMR cho sẵn thứ bạn định tự viết:** `monotonic_buffer_resource` chính là cái Arena trong chương này, đã chuẩn hoá.

---

## Cụm 1 — Bộ nhớ máy tính & bộ nhớ tiến trình (tr. 192–199)

### Nội dung chính

**Virtual address space.** Mỗi process thấy một không gian địa chỉ riêng; **MMU** dịch địa chỉ ảo → vật lý ở **mỗi lần truy cập bộ nhớ**. Tầng gián tiếp này cho phép OS chỉ giữ trong RAM phần đang dùng, phần còn lại đẩy ra **swap space**. Nói cách khác: *"có thể coi RAM vật lý là một cache của không gian bộ nhớ ảo nằm trên ổ đĩa"* (tr. 192).

**Memory page & page fault.** Không gian địa chỉ chia thành **page** kích thước cố định. Truy cập một page chưa được map vào **page frame** vật lý ⇒ **hardware exception** gọi là **page fault**. Sách nhấn mạnh: *page fault không phải lỗi* — nó là ngắt cần thiết để nạp dữ liệu từ đĩa (tr. 192).

Khi hết page frame trống, phải **evict** một frame. Frame **dirty** (đã sửa từ lúc nạp) phải ghi xuống đĩa trước; frame **clean** thì vứt thẳng. Cơ chế này gọi là **paging**.

> 🎯 **Chi tiết đắt cho embedded (tr. 193):** *không phải OS hỗ trợ virtual memory nào cũng hỗ trợ paging.* **iOS có virtual memory nhưng KHÔNG BAO GIỜ ghi dirty page xuống đĩa** — chỉ evict clean page; hết RAM thì **giết process** cho tới khi đủ. Android làm tương tự. Lý do: ghi ngược xuống flash **tốn pin** và **giảm tuổi thọ flash**.
>
> 🆕 Đây chính là mô hình của **thiết bị nhúng Linux dùng eMMC/NAND**: hầu hết cấu hình **tắt swap**, nên hết RAM là OOM-killer chứ không phải chậm dần. Liên hệ [03/memory-management](../../03-operating-system/memory-management.md).

**Thrashing** (tr. 194): hệ thống cạn RAM ⇒ paging liên tục ⇒ mỗi lần process được lên CPU nó lại chạm vào page vừa bị đẩy ra. Máy gần như đứng vì dành hết thời gian để paging. **Cách phát hiện: nhìn tần suất page fault.**

**Stack vs heap** (tr. 194–199). Sách nói rõ: chuẩn C++ không dùng chữ "stack"/"heap" mà dùng *free store*, *storage duration* — nhưng mọi hiện thực đều dùng stack cho lời gọi hàm, nên cứ gọi vậy cho dễ.

```
địa chỉ thấp                                                    địa chỉ cao
┌──────────┬──────────┬─────────────────┬──────────────────────────────┐
│   code   │  static  │  heap  ──────►  │       ◄────── stack          │
└──────────┴──────────┴─────────────────┴──────────────────────────────┘
                       lớn lên xuống dưới      lớn lên ngược lên trên
                                    (hình theo Figure 7.2)
```

**Năm tính chất riêng của stack** (tr. 195):

| Tính chất | Hệ quả |
|---|---|
| Là **khối liền mạch** | Cache locality tốt sẵn |
| **Kích thước tối đa cố định** | Vượt ⇒ **stack overflow** ⇒ crash |
| **Không bao giờ phân mảnh** | Cấp/thu luôn ở đỉnh, tuần tự |
| Cấp phát **gần như luôn nhanh** | Chỉ là dịch một con trỏ |
| **Mỗi thread một stack riêng** | Bộ nhớ stack coi như **thread-safe** |

**Đo stack thật bằng code** (tr. 195–197) — sách in ra địa chỉ biến local ở ba mức hàm lồng nhau:

```cpp
void func1() { auto i = 0; std::cout << "func1(): " << std::addressof(i) << '\n'; }
void func2() { auto i = 0; std::cout << "func2(): " << std::addressof(i) << '\n'; func1(); }
int main()   { auto i = 0; std::cout << "main():  " << std::addressof(i) << '\n'; func2(); func1(); }
```

Output thật của sách:
```
main():  0x7ea075ac
func2(): 0x7ea07594      ← thấp hơn 24 byte
func1(): 0x7ea0757c      ← thấp hơn 24 byte nữa
func1(): 0x7ea07594      ← func1 gọi từ main: cùng mức với func2 trước đó
```

⇒ Stack **lớn xuống dưới**, mỗi frame **24 byte** dù biến `i` chỉ 4 byte. **20 byte còn lại** là return address + padding cho alignment.

**Đo kích thước stack tối đa** (tr. 197): đệ quy vô hạn, mỗi tầng cấp 1 KB trên stack, in ra khoảng cách tới đáy:

```cpp
void func(std::byte* stack_bottom_addr) {
    std::byte data[1024];                              // 1 KB mỗi tầng
    std::cout << stack_bottom_addr - data << '\n';
    func(stack_bottom_addr);                           // đệ quy vô hạn — CỐ Ý
}
int main() { std::byte b; func(&b); }
```

Ba dòng cuối trước khi crash: `8378667 / 8379755 / 8380843` ⇒ **stack ≈ 8 MB**, khớp với `ulimit -s` trả về `8192` (KB).

> ⚠️ **Bẫy cross-platform (tr. 198):** **Windows mặc định 1 MB**. Chương trình chạy ngon trên macOS/Linux có thể **stack overflow trên Windows** nếu không chỉnh.
>
> 🆕 Trên **thread** Linux, mặc định `pthread` cũng là 8 MB *virtual* nhưng chỉ commit theo trang; trên **MCU/RTOS** thì stack mỗi task thường chỉ **vài KB** — cùng đoạn code, hai thế giới khác nhau. Liên hệ [08/memory-and-startup](../../08-embedded-systems/memory-and-startup.md).

**Phân mảnh heap** (tr. 198–199) — ví dụ số cụ thể của sách: khối 16 KB, cấp xen kẽ object loại **A (1 KB)** và **B (2 KB)** cho tới đầy, rồi giải phóng **toàn bộ A**.

```
sau khi cấp xen kẽ:   [A][ B  ][A][ B  ][A][ B  ][A][ B  ]   = 16 KB đầy
sau khi free hết A:   [ ][ B  ][ ][ B  ][ ][ B  ][ ][ B  ]   = còn TRỐNG 6 KB
                       ↑ mỗi lỗ chỉ 1 KB
xin cấp một B (2 KB): ✗ THẤT BẠI — 6 KB trống nhưng không chỗ nào liền 2 KB
                                    (hình theo Figure 7.4–7.5)
```

### Insight đáng nhớ

- **Stack chính là một memory allocator** (tr. 198): *"stack chỉ là một loại allocator khác, hiện thực được rất hiệu quả vì mẫu sử dụng của nó LUÔN tuần tự"*. Cấp và thu đều ở đỉnh ⇒ không bao giờ phân mảnh, và chỉ cần dịch một con trỏ. Đây là câu neo cho toàn bộ phần Arena ở cuối chương.
- **Heap phức tạp hơn stack vì hai lý do, không phải một**: (1) chia sẻ giữa các thread ⇒ phải đồng bộ; (2) vòng đời tuỳ ý ⇒ nguy cơ phân mảnh.

### Góc interview

**Q1. Vì sao cấp phát trên stack nhanh hơn heap? Kể ít nhất hai lý do khác nhau về bản chất.**
<details><summary>Đáp án</summary>

Hai lý do **độc lập** — trả lời được cả hai mới là đủ:

**① Thuật toán cấp phát tầm thường.** Stack cấp/thu **luôn ở đỉnh**, tuần tự, thu theo đúng thứ tự ngược với cấp. Nên "cấp phát" chỉ là **cộng/trừ một con trỏ**. Heap phải quản lý danh sách khối trống, tìm khối vừa, tách/gộp khối — chi phí **không tất định**.

**② Không cần đồng bộ.** Mỗi thread có **stack riêng** ⇒ không cần lock/atomic. Heap **dùng chung giữa mọi thread** trong process ⇒ `malloc` phải tự bảo vệ (glibc dùng nhiều arena + lock để giảm tranh chấp, nhưng vẫn có chi phí).

**Hệ quả thứ ba, thường bị quên:** stack **không bao giờ phân mảnh**, còn heap thì có. Với thiết bị chạy liên tục hàng tháng, phân mảnh là dạng "rò rỉ" mà `valgrind` không báo — RSS phình dần dù không leak.

**Bẫy:** trả lời *"vì stack nằm trong cache"* là **sai nhân quả**. Stack có cache locality tốt vì nó **liền mạch và được dùng đi dùng lại**, chứ nó không nằm ở một vùng phần cứng đặc biệt nào — vẫn là RAM như heap.

Liên hệ bank [CPP-004](../../14-prep/mock-interview/bank/cpp.md).
</details>

**Q2. Thiết bị nhúng của bạn chạy 3 tuần thì `malloc` bắt đầu trả `nullptr`, dù `free` được gọi đủ cặp và valgrind báo 0 leak. Chuyện gì xảy ra?**
<details><summary>Đáp án</summary>

**Phân mảnh heap (heap fragmentation).** Không phải leak: tổng bộ nhớ đã free vẫn đủ, nhưng **không còn khối LIỀN MẠCH** đủ lớn cho yêu cầu hiện tại.

Cơ chế, theo ví dụ tr. 198–199: cấp xen kẽ hai kích thước khác nhau, rồi giải phóng loại nhỏ ⇒ heap thành hình răng lược, mỗi lỗ 1 KB, xin 2 KB thì thất bại dù còn 6 KB trống.

**Vì sao valgrind im lặng:** valgrind memcheck đo **leak** (bộ nhớ mất tham chiếu), không đo phân mảnh. Mọi `malloc` đều có `free` tương ứng ⇒ sạch theo tiêu chí của nó.

**Cách khoanh vùng:** theo dõi RSS + `mallinfo()`/`malloc_stats()` theo thời gian; nếu **`arena` tăng đều mà `uordblks` (đang dùng) phẳng** thì gần như chắc chắn là phân mảnh.

**Cách chữa — theo đúng tinh thần chương này:**
1. **Loại cấp phát động khỏi đường chạy dài hạn**: cấp trước lúc khởi tạo, dùng lại (object pool).
2. **Arena/pool cấp phát kích thước CỐ ĐỊNH** ⇒ mọi lỗ đều bằng nhau ⇒ không thể phân mảnh (sách nêu đây là một trong ba chiến lược arena, tr. 221).
3. **Arena có vòng đời giới hạn**: gom mọi cấp phát của một "phiên" rồi thu một lượt (`monotonic_buffer_resource`).

**Bẫy:** nói *"gọi `free` là bộ nhớ trả về OS"* — không. glibc giữ lại trong arena để tái sử dụng, **RSS không giảm**; xem [03/memory-management](../../03-operating-system/memory-management.md).
</details>

---

## Cụm 2 — Object trong bộ nhớ: `new`/`delete` và placement new (tr. 199–203)

### Nội dung chính

**`new` làm hai việc, `delete` cũng vậy** (tr. 199):

| Biểu thức | Việc 1 | Việc 2 |
|---|---|---|
| `new User{"John"}` | **cấp phát** bộ nhớ đủ chứa `User` | **dựng** object (gọi constructor) |
| `delete user` | **huỷ** object (gọi destructor) | **giải phóng** bộ nhớ |

**Placement new — tách hai việc đó** (tr. 200):

```cpp
auto* memory = std::malloc(sizeof(User));   // (1) chỉ CẤP PHÁT
auto* user = ::new (memory) User("john");   // (2) chỉ DỰNG object tại địa chỉ đã có
```

Cú pháp `::new (memory)` là **placement new** — dạng `new` **không cấp phát**. Dấu `::` ép phân giải từ global namespace để tránh trúng một `operator new` bị nạp chồng.

**Không có "placement delete"** ⇒ phải làm tay hai bước:
```cpp
user->~User();      // gọi destructor TƯỜNG MINH
std::free(memory);  // rồi mới trả bộ nhớ
```

> ⚠️ Sách in hẳn một hộp cảnh báo (tr. 200): *"Đây là **lần duy nhất** bạn được gọi destructor tường minh. Đừng bao giờ gọi kiểu này trừ khi object được tạo bằng placement new."*

**Phiên bản hiện đại hơn** (tr. 201): C++17 thêm nhóm hàm `std::uninitialized_*` trong `<memory>` để dựng/copy/move vào vùng nhớ chưa khởi tạo, và `std::destroy_at()` để huỷ mà không giải phóng:

```cpp
auto* memory = std::malloc(sizeof(User));
auto* user_ptr = reinterpret_cast<User*>(memory);
std::uninitialized_fill_n(user_ptr, 1, User{"john"});
std::destroy_at(user_ptr);
std::free(memory);
// C++20: std::construct_at(user_ptr, User{"john"});  thay cho uninitialized_fill_n
```

**Nạp chồng `operator new`/`operator delete`** (tr. 201–203). Đây là kỹ thuật sách dùng lại **nhiều lần trong chương** để *đo* số lần cấp phát:

```cpp
auto operator new(size_t size) -> void* {
    void* p = std::malloc(size);
    std::cout << "allocated " << size << " byte(s)\n";
    return p;
}
auto operator delete(void* p) noexcept -> void {
    std::cout << "deleted memory\n";
    return std::free(p);
}
```

Ba mức nạp chồng, từ rộng tới hẹp:
- **Global** `operator new` / `operator delete` — ảnh hưởng cả chương trình.
- **`operator new[]` / `operator delete[]`** — cặp riêng cho mảng, **phải nạp chồng riêng**.
- **Theo class** — `static` member của class đó; hữu ích hơn global vì thường ta chỉ cần allocator riêng cho **một** class.

```cpp
class Document {
public:
    auto operator new(size_t size) -> void*  { return ::operator new(size); }
    auto operator delete(void* p) -> void    { ::operator delete(p); }
};
auto* p = new Document{};    // dùng bản của CLASS
auto* q = ::new Document{};  // ép dùng bản GLOBAL
```

> ⚠️ **Luật cặp đôi (tr. 202):** nạp chồng `operator new` thì **phải** nạp chồng `operator delete`. Bộ nhớ phải được giải phóng bởi **đúng allocator đã cấp nó** — `std::malloc` ↔ `std::free`, `operator new[]` ↔ `operator delete[]`.

### Insight đáng nhớ

- Placement new **không phải kiến thức học thuật** — nó là lý do kỹ thuật khiến `std::make_shared` và `std::vector::reserve` **bỏ qua** `operator new` riêng của class bạn (chi tiết ở Cụm 6). Không hiểu placement new thì cái bug đó không thể giải thích được.
- Sách tự nhắc (tr. 201): *"những tiện ích bộ nhớ mức thấp này được trình bày để HIỂU, việc dùng `reinterpret_cast` và chúng trong codebase thật nên giữ ở mức tối thiểu tuyệt đối."*

### Góc interview

**Q3. `new`/`delete` khác `malloc`/`free` chỗ nào? Trộn lẫn chúng thì sao?**
<details><summary>Đáp án</summary>

**Khác biệt cốt lõi: `new` gọi constructor, `malloc` thì không.**

| | `malloc`/`free` | `new`/`delete` |
|---|---|---|
| Cấp phát bộ nhớ | ✅ | ✅ |
| **Gọi constructor / destructor** | ❌ | ✅ |
| Kiểu trả về | `void*` — phải cast | **đúng kiểu**, type-safe |
| Kích thước | phải tự tính `sizeof` | compiler tự tính |
| Thất bại | trả `NULL` | **ném `std::bad_alloc`** |
| Tuỳ biến được không | thay `malloc` toàn cục | **nạp chồng theo class** được |

**Trộn lẫn = undefined behavior.** `free()` một con trỏ từ `new` ⇒ destructor **không chạy** (rò rỉ tài nguyên bên trong object: fd, mutex, buffer con) và allocator có thể khác nhau ⇒ hỏng heap. Ngược lại `delete` một con trỏ từ `malloc` ⇒ gọi destructor trên vùng nhớ **chưa từng được dựng object**.

Luật của sách (tr. 202): *"bộ nhớ phải được giải phóng bởi đúng allocator đã cấp nó"* — kể cả cặp `new[]`/`delete[]`.

**Nâng lên T2 — khi nào vẫn phải dùng `malloc`:** khi cần **tách cấp phát khỏi dựng object** (viết allocator, container tự chế). Khi đó cấp bằng `malloc`, dựng bằng **placement new**, huỷ bằng gọi destructor tường minh, trả bằng `free`.

Liên hệ bank [CPP-033](../../14-prep/mock-interview/bank/cpp.md).
</details>

---

## Cụm 3 — Alignment & padding (tr. 203–210) 🎯

### Nội dung chính

**Vì sao có alignment.** CPU đọc bộ nhớ vào thanh ghi **mỗi lần một word** (64 bit trên kiến trúc 64-bit). Để làm việc hiệu quả với các kiểu dữ liệu khác nhau, nó **ràng buộc địa chỉ** mà object mỗi kiểu được đặt. Trích nguyên văn chuẩn C++ mà sách dẫn (tr. 204):

> *"An alignment is an implementation-defined integer value representing the number of bytes between successive addresses at which a given object can be allocated."*
> (Alignment là một số nguyên do hiện thực định nghĩa, thể hiện **số byte giữa các địa chỉ liên tiếp** mà một object kiểu đó được phép đặt vào.)

```
alignof(int) == 4  →  hợp lệ tại 0x00, 0x04, 0x08, 0x0C, ...

word 64-bit:  │◄──── 8 byte ────►│◄──── 8 byte ────►│
   ĐÚNG:      │ int │ int │ int  │                  │   1 word → lấy được cả 3
   SAI:       │  │ int  │  int ──┼──►                │   int thứ 2 VẮT QUA 2 word
                              ↑ tốt nhất là chậm, tệ nhất là CRASH
                                    (hình theo Figure 7.6)
```

**Kiểm tra alignment cho đúng chuẩn** (tr. 205) — không dùng `%` mà dùng `std::align`:

```cpp
bool is_aligned(void* ptr, std::size_t alignment) {
    assert(ptr != nullptr);
    assert(std::has_single_bit(alignment));           // C++20 <bit>: phải là luỹ thừa của 2
    auto s = std::numeric_limits<std::size_t>::max();
    auto aligned_ptr = ptr;
    std::align(alignment, 1, aligned_ptr, s);         // chỉnh con trỏ nếu chưa aligned
    return ptr == aligned_ptr;                        // không đổi ⇒ vốn đã đúng
}
```

> 🆕 Vì sao không dùng `ptr % alignment == 0`? Chuẩn **không quy định** địa chỉ bắt đầu đếm từ 0 — chỉ trên thực tế mọi nền tảng đều vậy. Muốn portable tuyệt đối thì dùng `std::align`.

**`std::max_align_t` — cái `new` và `malloc` bảo đảm** (tr. 205): bộ nhớ trả về từ `new`/`malloc` luôn **aligned đủ cho MỌI kiểu scalar**, tức theo `alignof(std::max_align_t)`. Hệ quả đo được của sách: cấp hai `char` liên tiếp bằng `new` thì **cách nhau 16 byte**, dù `alignof(char) == 1`.

```cpp
auto* p1 = new char{'a'};
auto* p2 = new char{'b'};
// khoảng cách p1→p2 = 16 byte trên máy tác giả — 15 byte bị BỎ PHÍ (Figure 7.7)
```

**`alignas` — yêu cầu chặt hơn mặc định** (tr. 206):

```cpp
alignas(64) int x{};                      // ép x, y nằm trên HAI cache line khác nhau
alignas(64) int y{};

struct alignas(64) CacheLine { std::byte data[64]; };   // chiếm đúng 1 cache line

constexpr auto ps = std::size_t{4096};
struct alignas(ps) Page { std::byte data_[ps]; };       // đúng 1 memory page
auto* page = new Page{};  assert(is_aligned(page, ps));
```

C++17 thêm bản nạp chồng `operator new`/`delete` nhận `std::align_val_t`, nên `new` với kiểu over-aligned cũng đúng. Có thêm `aligned_alloc()` trong `<cstdlib>`.

> ⚠️ **Page size không thuộc C++ abstract machine** (tr. 207) — không có cách portable để lấy. Dùng `getpagesize()`/`sysconf(_SC_PAGESIZE)` trên Unix.

**Padding — đổi thứ tự khai báo, đổi `sizeof`** (tr. 208–209). Compiler **bắt buộc giữ nguyên thứ tự member** như ta khai báo, nên nó phải **chèn padding** để mỗi member đúng alignment:

```cpp
class Document {           // ❌ sizeof == 24
    bool   is_cached_{};   // 1 byte
    // std::byte padding1[7];   ← compiler chèn: double cần align 8
    double rank_{};        // 8 byte
    int    id_{};          // 4 byte
    // std::byte padding2[4];   ← chèn: cả class phải là bội của 8
};

class Document {           // ✅ sizeof == 16 — CÙNG dữ liệu
    double rank_{};        // 8
    int    id_{};          // 4
    bool   is_cached_{};   // 1
    // std::byte padding[3];    ← chỉ còn 3 byte padding ở cuối
};
```

```
v1 (24B): [bool][░░░░░░░ 7 ][    double 8    ][ int 4 ][░░░░ 4 ]
v2 (16B): [    double 8    ][ int 4 ][bool][░░░ 3 ]
              (hình theo Figure 7.8)
```

**Luật chung của sách:** *đặt member LỚN NHẤT lên đầu, nhỏ nhất xuống cuối.* Alignment của cả struct = alignment của member khắt khe nhất, và `sizeof` phải là bội của nó.

### Insight đáng nhớ

- **"Kích thước một object có thể thay đổi chỉ bằng cách đổi thứ tự khai báo member"** (tr. 209) — 24 → 16 byte, giảm **33%** mà không bỏ một trường nào. Với mảng 1 triệu phần tử, đó là 8 MB.
- Alignment **không chỉ là hiệu năng**: trên một số nền tảng, truy cập không aligned làm **chương trình crash** (tr. 204). 🆕 Đây là lớp bug kinh điển khi port code x86 sang ARM cũ / khi ép kiểu con trỏ vào buffer đọc từ mạng.

### Góc interview

**Q4. `struct { bool a; double b; int c; }` — `sizeof` bằng bao nhiêu và vì sao? Làm nó nhỏ lại mà không bỏ trường nào.**
<details><summary>Đáp án</summary>

**24 byte.** Cách tính:

| Offset | Nội dung | Vì sao |
|---|---|---|
| 0 | `bool a` (1B) | |
| 1–7 | **padding 7B** | `double` có `alignof == 8`, phải bắt đầu ở bội số của 8 |
| 8–15 | `double b` (8B) | |
| 16–19 | `int c` (4B) | |
| 20–23 | **padding 4B** | alignment của cả struct = 8 (member khắt khe nhất) ⇒ `sizeof` phải là **bội của 8** |

**Làm nhỏ lại: sắp lại theo thứ tự giảm dần alignment.**

```cpp
struct S { double b; int c; bool a; };   // sizeof == 16
// [double 8][int 4][bool 1][pad 3]
```

**Luật thực hành:** member lớn nhất lên đầu, nhỏ nhất xuống cuối.

**Nâng lên T2 — vì sao compiler không tự sắp lại giúp?** Vì chuẩn C++ **bắt buộc** giữ nguyên thứ tự khai báo của các member cùng access specifier — để layout dự đoán được, phục vụ tương tác C, serialize, `memcpy`, và ABI ổn định.

**Ba hệ quả thực chiến:**
1. **Trên mảng lớn**, tiết kiệm 8 byte/phần tử là tiết kiệm **cache line**, không chỉ RAM — xem [cụm cache](data-structures-cache.md).
2. **Struct dùng để map lên thanh ghi phần cứng hoặc gói tin mạng** thì padding là **bug**, không phải tối ưu — phải `#pragma pack`/`__attribute__((packed))` và chấp nhận truy cập không aligned.
3. Ngược chiều: đôi khi ta **cố ý** padding tới 64 byte (`alignas(64)`) để **chống false sharing** giữa hai thread.

Liên hệ bank [CPP-038](../../14-prep/mock-interview/bank/cpp.md), [12-dsa/ring-buffer](../../12-dsa/ring-buffer.md).
</details>

**Q5. `alignas(64)` dùng để làm gì trong code đa luồng?**
<details><summary>Đáp án</summary>

Để **chống false sharing**. Cache hoạt động theo đơn vị **cache line (thường 64 byte)**, không theo byte. Hai biến khác nhau nhưng **nằm chung một cache line** thì khi thread A ghi biến của nó, **toàn bộ line** bị invalidate trong cache của core B — dù B chỉ đọc biến khác. Kết quả: hai thread không hề chia sẻ dữ liệu logic nào vẫn **ping-pong cache line** cho nhau.

```cpp
struct Counters {          // ❌ hai biến chung 1 cache line
    std::atomic<int> a;
    std::atomic<int> b;
};
struct Counters {          // ✅ mỗi biến một line riêng
    alignas(64) std::atomic<int> a;
    alignas(64) std::atomic<int> b;
};
```

**Chi phí:** struct phình từ 8 byte lên 128 byte. Nên chỉ làm ở chỗ **đã đo được** tranh chấp — không rải `alignas(64)` khắp nơi.

⚠️ **Đây là T3 theo [config §6](../../14-prep/mock-interview/config.md)** — biết tên `alignas(64)` là điểm cộng, nhưng thứ **tính điểm** là giải thích được *vì sao hai biến không chia sẻ gì vẫn làm chậm nhau*.

Liên hệ [cpp-concurrency/08-designing-concurrent-code](../cpp-concurrency/08-designing-concurrent-code.md).
</details>

---

## Cụm 4 — Quyền sở hữu bộ nhớ (tr. 210–217)

### Nội dung chính

**Mọi biến đều đã có chủ, trừ một loại** (tr. 210):

| Loại biến | Ai sở hữu | Huỷ khi nào |
|---|---|---|
| Biến local | **scope hiện tại** | thoát scope |
| Biến static/global | **chương trình** | kết thúc chương trình |
| Data member | **instance của class chứa nó** | object đó bị huỷ |
| **Biến động (`new`)** | ❌ **KHÔNG có chủ mặc định** | ...tuỳ lập trình viên |

⇒ *"chỉ biến động là không có chủ mặc định"*. Raw pointer **không diễn đạt quyền sở hữu**, nên chỉ dùng raw pointer thì không lần ra được ai chịu trách nhiệm giải phóng.

**RAII** (tr. 211–213) — sách minh hoạ bằng tài nguyên **không phải bộ nhớ** (connection), đúng trọng tâm:

```cpp
class RAIIConnection {
public:
    explicit RAIIConnection(const std::string& url) : connection_{open_connection(url)} {}
    ~RAIIConnection() {
        try { close(connection_); }
        catch (const std::exception&) { /* xử lý, nhưng KHÔNG BAO GIỜ ném từ destructor */ }
    }
    auto& get() { return connection_; }
private:
    Connection connection_;
};

auto send_request(const std::string& request) {
    auto connection = RAIIConnection("http://www.example.com/");
    send_request(connection.get(), request);
    // không cần close — destructor lo, KỂ CẢ khi có exception ném ra giữa chừng
}
```

**Ba loại quyền sở hữu qua smart pointer** (tr. 213), diễn đạt theo lời sách:

| Smart pointer | Nghĩa |
|---|---|
| `unique_ptr` | *"Tôi, và chỉ tôi, sở hữu object này. Dùng xong tôi xoá."* |
| `shared_ptr` | *"Tôi sở hữu cùng người khác. Khi không ai cần nữa, nó bị xoá."* |
| `weak_ptr` | *"Tôi sẽ dùng nếu nó còn tồn tại, nhưng không giữ nó sống chỉ vì tôi."* |

**Chi phí `unique_ptr`** (tr. 214) — chi tiết ít người biết: nó *"gần như không có overhead"* nhưng **không phải bằng 0**. Vì `unique_ptr` có **destructor không tầm thường**, nó **không thể truyền qua thanh ghi CPU** khi truyền vào hàm, khác với raw pointer. ⇒ chậm hơn raw pointer một chút.

**`make_shared` — đo bằng số lần cấp phát** (tr. 214–215). Nạp chồng `operator new` để đếm:

```cpp
auto i = std::make_shared<double>(42.0);
// output:  allocated 32 bytes / deleted memory              ← MỘT lần cấp phát

auto i = std::shared_ptr<double>{new double{42.0}};
// output:  allocated 4 bytes / allocated 32 bytes           ← HAI lần cấp phát
//          deleted memory / deleted memory
```

⇒ `make_shared` gộp **object + control block (đếm tham chiếu)** vào **một** lần cấp phát. Lợi hai mặt: ít cấp phát hơn, và **cache-friendly hơn nhờ spatial locality** (hai thứ luôn dùng cùng nhau thì nằm cạnh nhau).

**`weak_ptr` khác raw pointer thế nào** (tr. 216) — câu hỏi hay bị hỏi ngược:

```cpp
auto i = std::make_shared<int>(10);
auto weak_i = std::weak_ptr<int>{i};
// ... có thể i.reset() ở đây, object bị xoá ...
if (auto shared_i = weak_i.lock()) {   // lock() = thử nâng lên shared_ptr
    std::cout << *shared_i << '\n';    // vào được đây ⇒ object CHẮC CHẮN còn sống
} else {
    std::cout << "weak_i has expired\n";
}
```

Raw pointer dangling thì **không có cách nào hỏi "còn sống không"**; `weak_ptr` thì có, và `lock()` giữ object sống trong suốt thời gian ta dùng.

### Insight đáng nhớ

- **`shared_ptr` thread-safe ở BỘ ĐẾM, không ở object.** Sách nói rõ: *"`std::shared_ptr` an toàn luồng ở bên trong, nên bộ đếm phải được cập nhật nguyên tử để tránh race"* (tr. 214). Đó cũng là **chi phí**: mỗi lần copy là một atomic increment.
- Ownership không phải chuyện riêng của C++ (tr. 216): *"một khi đã nắm vững, nó sẽ cải thiện kỹ năng lập trình của bạn ở cả ngôn ngữ khác... Suy nghĩ về quyền sở hữu object sẽ tác động tích cực lên thiết kế và kiến trúc chương trình bạn viết."*

### Góc interview

**Q6. Vì sao nên dùng `make_shared` thay vì `shared_ptr<T>{new T{...}}`?**
<details><summary>Đáp án</summary>

Hai lý do, sách chứng minh lý do (2) **bằng số đo**:

**① Exception safety.** Với `f(std::shared_ptr<T>{new T}, g())`, compiler được phép sắp xếp: `new T` → `g()` → dựng `shared_ptr`. Nếu `g()` ném, object vừa `new` **rò rỉ** vì chưa kịp có ai sở hữu. `make_shared` là một lời gọi hàm duy nhất ⇒ không có khe hở đó.

**② Một lần cấp phát thay vì hai.** `shared_ptr` cần **control block** (bộ đếm tham chiếu) tách rời object. Đo bằng cách nạp chồng `operator new` (tr. 215):

```
make_shared<double>(42.0)              → allocated 32 bytes            (1 lần)
shared_ptr<double>{new double{42.0}}   → allocated 4 + allocated 32    (2 lần)
```

**Lợi phụ:** object và control block nằm **liền nhau** ⇒ cache-friendly hơn, vì mọi thao tác copy/destroy shared_ptr đều chạm cả hai.

**Nâng lên T2 — khi nào KHÔNG dùng `make_shared`:**
- Cần **custom deleter** (`shared_ptr<FILE>{fp, &fclose}`) — `make_shared` không nhận deleter.
- Object **rất lớn** và có `weak_ptr` sống lâu: control block gộp chung nghĩa là **bộ nhớ của object không được trả** cho tới khi weak_ptr cuối cùng chết (chỉ destructor chạy). Với object vài MB thì đó là vấn đề thật.

Liên hệ bank [CPP-007](../../14-prep/mock-interview/bank/cpp.md), [CPP-020](../../14-prep/mock-interview/bank/cpp.md).
</details>

---

## Cụm 5 — Small object optimization (tr. 217–220)

### Nội dung chính

**Vấn đề:** container như `std::vector`/`std::string` tự cấp phát động khi cần — tiện, nhưng với vài phần tử nhỏ thì **cấp phát heap đắt hơn cả dữ liệu**. Đa số chuỗi trong chương trình thật là chuỗi ngắn.

**Cách đo của sách** (tr. 217) — lại là nạp chồng `operator new`:

```cpp
auto allocated = size_t{0};
void* operator new(size_t size) { void* p = std::malloc(size); allocated += size; return p; }
void  operator delete(void* p) noexcept { return std::free(p); }

int main() {
    allocated = 0;
    auto s = std::string{""};      // thay bằng chuỗi dài dần để dò ngưỡng
    std::cout << "stack space = " << sizeof(s)
              << ", heap space = " << allocated
              << ", capacity = "   << s.capacity() << '\n';
}
```

Kết quả thật trên libc++ 64-bit, **release mode**:

| Chuỗi | Output |
|---|---|
| `""` (rỗng) | `stack space = 24, heap space = 0, capacity = 22` |
| **22 ký tự** | y hệt — **vẫn 0 byte heap** |
| **23 ký tự** | `stack space = 24, heap space = 32, capacity = 31` ← **đã phải dùng heap** |

⇒ `std::string` chỉ **24 byte** mà chứa được **22 ký tự không cấp phát gì**.

**Làm thế nào?** Dùng **`union` hai layout** (tr. 218–219):

```cpp
struct Long {              // 8 + 8 + 8 = 24 byte
    size_t capacity_{};
    size_t size_{};
    char*  data_{};        // trỏ tới heap
};
struct Short {             // 1 + 23 = 24 byte
    unsigned char size_{}; // chuỗi ngắn ⇒ size chỉ 0..22, 1 byte là đủ
    char data_[23]{};      // buffer NẰM NGAY TRONG object
};
union u_ { Short short_layout_; Long long_layout_; };
```

**Cờ phân biệt short/long cất ở đâu?** Đây là chỗ khéo nhất: libc++ dùng **bit thấp nhất (LSB)** của `capacity_` ở chế độ long, và **LSB của `size_`** ở chế độ short.
- Ở chế độ **long**: bit đó **dư thừa sẵn** vì string luôn cấp phát kích thước là bội của 2.
- Ở chế độ **short**: chỉ cần 7 bit để lưu size (0–22), thừa ra 1 bit.

```
      ┌─ 24 byte ─────────────────────────────────────────┐
LONG  │ capacity_ (8B, LSB=cờ) │ size_ (8B) │ data_ ptr(8B)│ ──► heap
SHORT │ size_(1B, LSB=cờ) │        data_[23]                │  (không heap)
      └───────────────────────────────────────────────────┘
                          (hình theo Figure 7.9)
```

> Sách lưu ý: xử lý **big-endian** còn phức tạp hơn nữa, vì bit cờ phải nằm **cùng một vị trí vật lý** trong cả hai layout.

### Insight đáng nhớ

Kết luận của sách (tr. 220), đáng nhớ nguyên văn tinh thần: *"Những mẹo khéo như thế này chính là lý do bạn nên dùng các class hiệu quả và đã được kiểm thử kỹ của thư viện chuẩn TRƯỚC khi tự viết. Tuy vậy, biết những tối ưu này tồn tại và hoạt động ra sao vẫn quan trọng và hữu ích, kể cả khi bạn không bao giờ phải tự viết một cái."*

🆕 **Liên hệ embedded:** SSO là lý do `std::string` **có thể dùng được** trong nhiều đường nóng — miễn chuỗi ngắn. Nhưng ngưỡng 22 ký tự **là chi tiết hiện thực** (libstdc++ khác libc++), nên **không được** thiết kế dựa vào nó. Cần bảo đảm không cấp phát ⇒ dùng `std::array<char, N>` hoặc `std::string_view`.

### Góc interview

**Q7. `std::string` có luôn cấp phát heap không? Chứng minh.**
<details><summary>Đáp án</summary>

**Không** — nhờ **small string optimization (SSO)**. Chuỗi đủ ngắn được lưu **ngay bên trong** object string, không chạm heap.

**Cách chứng minh** (đúng cách sách làm): nạp chồng global `operator new` để đếm byte đã cấp, rồi in `sizeof(s)`, số byte heap, và `s.capacity()`. Trên libc++ 64-bit: string **24 byte**, chứa được **22 ký tự** với **0 byte heap**; sang ký tự thứ **23** thì cấp 32 byte heap (capacity 31 — thừa 1 byte cho `'\0'` vì libc++ luôn giữ chuỗi null-terminated bên trong).

**Cơ chế:** `union` hai layout cùng 24 byte — `Short{unsigned char size_; char data_[23]}` và `Long{size_t capacity_; size_t size_; char* data_}` — phân biệt bằng **1 bit thừa** (LSB của `capacity_` ở long, LSB của `size_` ở short).

**Bẫy 1:** trả lời *"22 ký tự"* như một hằng số phổ quát. Đó là **chi tiết của libc++**; libstdc++ và MSVC có ngưỡng khác. Không được viết code phụ thuộc con số này.

**Bẫy 2:** quên rằng phải đo ở **release mode** — debug build có thể khác.

**Nâng lên T2 — vì sao thiết kế bằng `union` chứ không thêm buffer phụ?** Vì thêm buffer phụ làm string **to lên cả khi không dùng buffer đó**. `union` cho phép tái dùng đúng 24 byte cho hai mục đích loại trừ nhau.
</details>

---

## Cụm 6 — Custom memory management: arena, allocator, PMR (tr. 220–236) 🎯

### Nội dung chính

**Ba lý do chính đáng để tự quản lý bộ nhớ** (tr. 220–221):
1. **Debug & chẩn đoán** — chính là thứ sách dùng suốt chương: nạp chồng `operator new` để in ra.
2. **Sandboxing** — giới hạn bộ nhớ cho một vùng code, và thu hồi hết khi vùng đó xong.
3. **Hiệu năng** — khi buộc phải cấp phát động và cần nhanh hơn `malloc` cho **mẫu sử dụng cụ thể của mình**.

> ⚠️ Sách tự dội nước lạnh (tr. 221): *"nhiều lập trình viên C++ kỳ cựu chưa bao giờ gặp bài toán thật sự cần tuỳ biến memory manager. Đó là chỉ dấu cho thấy các general-purpose memory manager ngày nay tốt đến mức nào."* Lợi thế duy nhất của ta là **biết mẫu sử dụng cụ thể**, thứ `malloc` không thể biết.

**Ba chiến lược làm arena thắng được `malloc`** (tr. 221) — đây là phần đáng giá nhất cho embedded:

| Chiến lược | Vì sao thắng | 🆕 Hợp với |
|---|---|---|
| **Đơn luồng** | Không cần lock/atomic ⇒ **không bao giờ bị thread khác chặn** — *"quan trọng trong ngữ cảnh real-time"* | ISR, luồng audio/control có deadline |
| **Kích thước cố định** | Dùng free list, **không thể phân mảnh** | pool cho message/buffer cùng cỡ |
| **Vòng đời giới hạn** | Không thu lẻ; xong việc **thu một lượt** | xử lý một request/một frame |

#### Arena — cấp phát trên STACK

Dựa trên `short_alloc` của **Howard Hinnant** (https://howardhinnant.github.io/stack_alloc.html). Sách nói rõ đây là **bản giản lược để minh hoạ**, không phải code production.

```cpp
template <size_t N>
class Arena {
    static constexpr size_t alignment = alignof(std::max_align_t);
public:
    Arena() noexcept : ptr_(buffer_) {}
    Arena(const Arena&) = delete;                 // arena không được copy —
    Arena& operator=(const Arena&) = delete;      // nó SỞ HỮU một vùng nhớ cụ thể
    auto reset() noexcept { ptr_ = buffer_; }     // thu HẾT một lượt, O(1)
    static constexpr auto size() noexcept { return N; }
    auto used() const noexcept { return static_cast<size_t>(ptr_ - buffer_); }
    auto allocate(size_t n) -> std::byte*;
    auto deallocate(std::byte* p, size_t n) noexcept -> void;
private:
    // làm tròn LÊN theo alignment bằng phép bit: (n + 15) & ~15 với alignment = 16
    static auto align_up(size_t n) noexcept -> size_t {
        return (n + (alignment - 1)) & ~(alignment - 1);
    }
    // con trỏ này có thuộc buffer của ta không?
    auto pointer_in_buffer(const std::byte* p) const noexcept -> bool {
        return std::uintptr_t(p) >= std::uintptr_t(buffer_) &&
               std::uintptr_t(p) <  std::uintptr_t(buffer_) + N;
    }
    alignas(alignment) std::byte buffer_[N];      // ⭐ buffer NẰM TRONG object
    std::byte* ptr_{};                            // con trỏ chạy — y hệt stack
};
```

**Ba chi tiết dễ bỏ qua, đều quan trọng:**

1. **`alignas(alignment)` trên `buffer_` là bắt buộc.** Nếu arena nằm trên stack thì mảng `std::byte` **không có bảo đảm gì** về alignment ngoài của `char`. Thiếu `alignas`, arena sẽ trả về địa chỉ sai alignment cho `double`/con trỏ.
2. **`align_up` bằng phép bit** thay vì `%`: `(n + (a-1)) & ~(a-1)` — nhanh hơn chia, đúng khi `a` là luỹ thừa của 2.
3. **Ép sang `std::uintptr_t` trước khi so sánh** (tr. 223): *"so sánh quan hệ giữa các raw pointer trỏ tới các object rời rạc là **undefined behavior**; compiler tối ưu có thể lợi dụng điều đó và cho kết quả bất ngờ."* Đây là loại UB gần như không ai biết.

```cpp
template<size_t N>
auto Arena<N>::allocate(size_t n) -> std::byte* {
    const auto aligned_n = align_up(n);
    const auto available_bytes = static_cast<decltype(aligned_n)>(buffer_ + N - ptr_);
    if (available_bytes >= aligned_n) {
        auto* r = ptr_;
        ptr_ += aligned_n;           // chỉ dịch con trỏ — ĐÚNG như stack
        return r;
    }
    return static_cast<std::byte*>(::operator new(n));   // hết chỗ ⇒ rơi về heap
}

template<size_t N>
auto Arena<N>::deallocate(std::byte* p, size_t n) noexcept -> void {
    if (pointer_in_buffer(p)) {
        n = align_up(n);
        if (p + n == ptr_) { ptr_ = p; }   // CHỈ thu được nếu đó là khối cấp CUỐI CÙNG
        // ngược lại: bỏ qua, không thu hồi
    } else {
        ::operator delete(p);              // không phải của ta ⇒ trả về heap
    }
}
```

> 🎯 **Đây là điểm mấu chốt về đánh đổi:** arena **chỉ thu hồi được khối cuối cùng**. Thu ở giữa thì lặng lẽ **bỏ qua** — bộ nhớ đó mất cho tới khi `reset()`. Đổi lại: cấp phát chỉ là một phép cộng, **không có metadata, không có free list, không có lock**.

**Dùng arena cho một class cụ thể** (tr. 224):

```cpp
auto user_arena = Arena<1024>{};        // 1 KB, có thể đặt trên stack/static/thread_local

class User {
public:
    auto operator new(size_t size) -> void*   { return user_arena.allocate(size); }
    auto operator delete(void* p) -> void     { user_arena.deallocate(static_cast<std::byte*>(p), sizeof(User)); }
    auto operator new[](size_t size) -> void* { return user_arena.allocate(size); }
    auto operator delete[](void* p, size_t size) -> void { user_arena.deallocate(static_cast<std::byte*>(p), size); }
private:
    int id_{};
};

int main() {
    auto user1 = new User{};        // ✅ không cấp phát động
    delete user1;
    auto users = new User[10];      // ✅
    delete[] users;
    auto user2 = std::make_unique<User>();   // ✅ (make_unique dùng new)
}
```

#### ⚠️ Cái bẫy: `operator new` của class KHÔNG phải lúc nào cũng được gọi

Hai dòng sau **không dùng** `User::operator new` (tr. 225–226):

```cpp
auto user = std::make_shared<User>();      // ✗
auto users = std::vector<User>{};
users.reserve(10);                          // ✗
```

**Vì sao:**
- `make_shared` phải cấp **object + control block trong MỘT lần** ⇒ không thể viết `new User()`. Nó cấp một khối rồi **dựng bằng placement new**.
- `vector::reserve(10)` **không dựng 10 object** (làm vậy sẽ đòi mọi kiểu phải có default constructor). Nó chỉ **cấp bộ nhớ**, rồi dựng dần bằng **placement new** khi ta `push_back`.

⇒ Placement new **không đi qua** `operator new`. Muốn container/smart pointer dùng arena của mình thì phải cung cấp **allocator**, không phải nạp chồng `operator new`.

#### Allocator: stateless vs stateful

**Allocator tối thiểu từ C++11** (tr. 226) — đã gọn hơn nhiều so với C++98 nhờ `std::allocator_traits` điền các mặc định:

```cpp
template<typename T>
struct Alloc {
    using value_type = T;
    Alloc();
    template<typename U> Alloc(const Alloc<U>&);
    T*   allocate(size_t n);
    auto deallocate(T*, size_t) const noexcept -> void;
};
template<typename T> auto operator==(const Alloc<T>&, const Alloc<T>&) -> bool;
template<typename T> auto operator!=(const Alloc<T>&, const Alloc<T>&) -> bool;
```

**`Mallocator` — stateless** (tr. 227, gốc của Stephan T. Lavavej): không có state riêng, chỉ gọi `malloc`/`free` toàn cục. Mọi instance **so sánh bằng nhau** ⇒ bộ nhớ cấp bởi instance này giải phóng được bởi instance khác.

**`ShortAlloc` — stateful**, giữ **con trỏ tới arena** (tr. 228):

```cpp
template <class T, size_t N>
struct ShortAlloc {
    using value_type = T;
    using arena_type = Arena<N>;

    ShortAlloc(arena_type& arena) noexcept : arena_{&arena} {}
    template <class U> ShortAlloc(const ShortAlloc<U, N>& other) noexcept : arena_{other.arena_} {}
    template <class U> struct rebind { using other = ShortAlloc<U, N>; };   // container cần cấp kiểu KHÁC T (vd node)

    auto allocate(size_t n) -> T* {
        return reinterpret_cast<T*>(arena_->allocate(n * sizeof(T)));
    }
    auto deallocate(T* p, size_t n) noexcept -> void {
        arena_->deallocate(reinterpret_cast<std::byte*>(p), n * sizeof(T));
    }
    // hai allocator bằng nhau ⟺ CÙNG arena
    template <class U, size_t M> auto operator==(const ShortAlloc<U, M>& o) const noexcept {
        return N == M && arena_ == o.arena_;
    }
    template <class U, size_t M> auto operator!=(const ShortAlloc<U, M>& o) const noexcept { return !(*this == o); }
    template <class U, size_t M> friend struct ShortAlloc;
private:
    arena_type* arena_;      // ĐÂY là toàn bộ state
};
```

**Dùng thật với `std::set`** (tr. 229–230):

```cpp
int main() {
    using SmallSet = std::set<int, std::less<int>, ShortAlloc<int, 512>>;
    auto stack_arena = SmallSet::allocator_type::arena_type{};   // 512 byte TRÊN STACK
    auto unique_numbers = SmallSet{stack_arena};

    auto n = int{};
    while (std::cin >> n) unique_numbers.insert(n);              // ít số ⇒ 0 lần cấp phát heap
    for (const auto& number : unique_numbers) std::cout << number << '\n';
}
```

Ít số ⇒ chạy hoàn toàn trên stack. Nhiều số ⇒ arena đầy ⇒ tự rơi về `::operator new`. **Không có cliff, chỉ có degradation.**

#### Vấn đề chí mạng: allocator là một phần của KIỂU

```cpp
void process(std::vector<int>& buffer) { /* ... */ }     // hàm có sẵn trong codebase

using MyAlloc = ShortAlloc<int, 512>;
auto arena = MyAlloc::arena_type();
auto vec = std::vector<int, MyAlloc>(64, arena);
process(vec);   // ❌ KHÔNG BIÊN DỊCH
```

Lỗi GCC nguyên văn (tr. 231):
```
error: invalid initialization of reference of type 'const std::vector<int>&'
from expression of type 'std::vector<int, ShortAlloc<int, 512> >'
```

`std::vector<int>` thực chất là `std::vector<int, std::allocator<int>>` ⇒ **hai kiểu khác nhau**. Và kích thước cũng khác:

```cpp
sizeof(std::vector<int>)            // 24  — std::allocator là class RỖNG
sizeof(std::vector<int, MyAlloc>)   // 32  — allocator có state (con trỏ arena)
```

> libc++ dùng **compressed pair** (dựa trên *empty base-class optimization*) để allocator rỗng không tốn byte nào.

**Cách tránh:** cho `process()` nhận `std::span` / viết nó thành hàm generic trên range thay vì cột vào `std::vector`.

#### `std::pmr` — lời giải của C++17

C++17 thêm **một tầng gián tiếp**: mọi container trong `std::pmr` dùng **cùng một allocator** là `std::pmr::polymorphic_allocator`, và allocator này **chuyển tiếp** yêu cầu tới một **`memory_resource`** mà nó trỏ tới.

```
   std::pmr::vector<int>
        │ (allocator là polymorphic_allocator — LUÔN cùng một kiểu)
        ▼
   polymorphic_allocator ──con trỏ──► memory_resource  (đa hình, runtime)
                                          ├── monotonic_buffer_resource
                                          ├── unsynchronized_pool_resource
                                          └── PrintingResource (tự viết)
                                    (hình theo Figure 7.10)
```

⇒ **`memory_resource` tương đương với `Arena` của ta; `polymorphic_allocator` là tầng gián tiếp giữ con trỏ tới nó.** Đổi chiến lược cấp phát **không đổi kiểu**.

```cpp
auto v1 = std::vector<int>{};        // std::allocator
auto v2 = std::pmr::vector<int>{};   // polymorphic_allocator
```

**Ba memory resource có sẵn** (`<memory_resource>`, tr. 233):

| Resource | Làm gì | Khi nào dùng |
|---|---|---|
| **`monotonic_buffer_resource`** | **Giống hệt `Arena` ở trên**: chỉ cấp, **chỉ giải phóng khi chính nó bị huỷ** | tạo nhiều object **vòng đời ngắn**; cấp phát cực nhanh |
| **`unsynchronized_pool_resource`** | Nhiều **pool** (slab) khối **kích thước cố định** ⇒ **không phân mảnh trong mỗi pool**. **KHÔNG thread-safe** | nhiều object thuộc **vài cỡ khác nhau** |
| **`synchronized_pool_resource`** | Bản thread-safe của cái trên | như trên, dùng từ nhiều thread |

**Chuỗi upstream:** resource nào cũng nhận được một **upstream resource**, dùng khi nó không đáp ứng nổi (giống hệt việc `ShortAlloc` rơi về `::operator new`). Ba hàm tự do trả về resource toàn cục:
- `std::pmr::new_delete_resource()` — dùng `operator new`/`delete` toàn cục.
- `std::pmr::null_memory_resource()` — **luôn ném `std::bad_alloc`**. 🆕 Cực hợp embedded: dùng làm upstream để **ép chương trình chết ngay** nếu vượt ngân sách bộ nhớ tĩnh, thay vì âm thầm rơi về heap.
- `std::pmr::get_default_resource()` / `set_default_resource()`.

**Viết lại ví dụ `std::set` bằng PMR** — không cần `Arena` lẫn `ShortAlloc` tự viết (tr. 233):

```cpp
int main() {
    auto buffer = std::array<std::byte, 512>{};              // 512 byte trên STACK
    auto resource = std::pmr::monotonic_buffer_resource{
        buffer.data(), buffer.size(),
        std::pmr::new_delete_resource()};                    // upstream khi hết buffer
    auto unique_numbers = std::pmr::set<int>{&resource};

    auto n = int{};
    while (std::cin >> n) unique_numbers.insert(n);
    for (const auto& number : unique_numbers) std::cout << number << '\n';
}
```

**Tự viết memory resource** (tr. 234) — chỉ cần override **ba hàm thuần ảo**:

```cpp
class PrintingResource : public std::pmr::memory_resource {
public:
    PrintingResource() : res_{std::pmr::get_default_resource()} {}
private:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        std::cout << "allocate: " << bytes << '\n';
        return res_->allocate(bytes, alignment);
    }
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        std::cout << "deallocate: " << bytes << '\n';
        return res_->deallocate(p, bytes, alignment);
    }
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return (this == &other);
    }
    std::pmr::memory_resource* res_;
};
```

> ⚠️ **Vì sao lưu default resource trong constructor** thay vì gọi `get_default_resource()` trong `do_allocate`? Vì ai đó có thể gọi `set_default_resource()` **giữa lúc cấp phát và lúc giải phóng** ⇒ giải phóng bằng resource khác resource đã cấp.

Chạy thử:
```cpp
auto res = PrintingResource{};
auto vec = std::pmr::vector<int>{&res};
vec.emplace_back(1);
vec.emplace_back(2);
// allocate: 4 / allocate: 8 / deallocate: 4 / deallocate: 8
```
(Thấy rõ vector **cấp lại và nhân đôi**: 4 byte → 8 byte, rồi trả khối cũ.)

**Bẫy cuối, rất dễ mắc** (tr. 235) — PMR truyền **raw pointer không sở hữu**:

```cpp
auto create_vec() -> std::pmr::vector<int> {
    auto resource = PrintingResource{};
    auto vec = std::pmr::vector<int>{&resource};   // giữ RAW POINTER tới resource
    return vec;
}                                                   // ✗ resource CHẾT ở đây
auto vec = create_vec();
vec.emplace_back(1);                                // ✗ UNDEFINED BEHAVIOR
```

**Resource phải sống lâu hơn mọi container dùng nó.** Lỗi này dễ mắc với `pmr` hơn với allocator tự viết, vì các container `pmr` **cùng kiểu** nên compiler không cản.

### Insight đáng nhớ

- **Arena chính là stack, thu hẹp phạm vi.** Cả `Arena` lẫn `monotonic_buffer_resource` đều chỉ làm một việc: dịch một con trỏ đi tới, và thu hồi một lượt. Đó là lý do chúng nhanh — và cũng là lý do chúng **không thu hồi lẻ được**.
- Lời khuyên cuối chương (tr. 236): *"nếu bạn thấy hấp dẫn với ý định dùng custom allocator để tăng hiệu năng, tôi khuyến khích bạn **đo và phân tích cẩn thận mẫu truy cập bộ nhớ** của ứng dụng trước. Thường chỉ có một tập nhỏ class/object thật sự cần chỉnh."* Nhưng vế sau cũng mạnh không kém: *"giảm số lần cấp phát động, hoặc gom object lại gần nhau trong bộ nhớ, có thể có tác động **rất lớn** tới hiệu năng."*

### Góc interview

**Q8. Firmware của bạn không được phép cấp phát heap sau khi khởi tạo, nhưng bạn cần một `std::set`/`std::vector`. Làm thế nào?**
<details><summary>Đáp án</summary>

**Ba phương án, nêu được đánh đổi mới đạt điểm cao.**

**① `std::pmr` + `monotonic_buffer_resource` trên buffer tĩnh — nên chọn mặc định (C++17).**

```cpp
static std::array<std::byte, 4096> buffer;
auto resource = std::pmr::monotonic_buffer_resource{
    buffer.data(), buffer.size(),
    std::pmr::null_memory_resource()};   // ⭐ hết buffer ⇒ NÉM, không âm thầm rơi về heap
auto s = std::pmr::set<int>{&resource};
```

Điểm mấu chốt: **`null_memory_resource()` làm upstream**. Nó biến "vượt ngân sách bộ nhớ" từ một lỗi âm thầm (rơi về heap, phá vỡ tính tất định) thành một **`std::bad_alloc` nổ ngay** — phát hiện được lúc test.

**② Allocator tự viết (`ShortAlloc` + `Arena`)** — dùng khi kẹt ở C++11/14 chưa có `pmr`. Nhược điểm lớn: **allocator trở thành một phần của kiểu**, `std::vector<int, MyAlloc>` không truyền được cho hàm nhận `std::vector<int>`. Phải sửa API sang `std::span`/generic.

**③ Không dùng container động:** `std::array` cỡ cố định, hoặc `etl::vector`, `boost::container::static_vector`. Đơn giản nhất, nhưng mất API quen thuộc.

**Đánh đổi phải nói ra:** `monotonic_buffer_resource` **chỉ giải phóng khi chính nó bị huỷ** — xoá phần tử **không** trả bộ nhớ về buffer. Nên nó hợp với mẫu *"làm một lô việc rồi reset"* (một request, một frame ảnh), **không hợp** với cấu trúc sống suốt đời và liên tục thêm/xoá. Cho mẫu sau thì dùng `unsynchronized_pool_resource` (pool kích thước cố định, không phân mảnh).

**Bẫy chí mạng:** resource phải **sống lâu hơn** container. Trả về `pmr::vector` từ một hàm có resource là biến local ⇒ **UB** (tr. 235).

Liên hệ [08/constraints.md](../../08-embedded-systems/constraints.md).
</details>

**Q9. Bạn viết `operator new` riêng cho class `User` để nó cấp từ arena. Nhưng `std::make_shared<User>()` và `vector<User>::reserve(10)` vẫn cấp từ heap. Vì sao?**
<details><summary>Đáp án</summary>

**Vì cả hai đều dùng placement new, mà placement new KHÔNG đi qua `operator new`.**

- **`make_shared`** phải cấp **object + control block (đếm tham chiếu) trong MỘT lần cấp phát** — đó chính là ưu điểm của nó. Không có cách nào viết `new User()` mà ra được một khối chứa cả hai. Nên nó cấp một khối thô rồi **dựng `User` bằng placement new** trong đó.
- **`vector::reserve(10)`** **không dựng 10 object** — nếu dựng thì mọi kiểu dùng với vector đều phải có default constructor. Nó chỉ **cấp bộ nhớ đủ chỗ**, rồi dựng từng phần tử bằng **placement new** khi `push_back`/`emplace_back`.

**Bài học tổng quát:** `operator new` của class chỉ chặn được **biểu thức `new User{}`**. Muốn kiểm soát bộ nhớ của **container và smart pointer**, phải đi đường **allocator** (`std::vector<User, MyAlloc>`, `std::allocate_shared`), hoặc dùng `std::pmr`.

**Nâng lên T2 — vì sao `std::pmr` tốt hơn allocator tự viết ở đây?** Vì allocator tự viết **đổi kiểu của container**: `std::vector<int, MyAlloc>` ≠ `std::vector<int>`, không truyền cho hàm cũ được (GCC báo `invalid initialization of reference of type 'const std::vector<int>&'`). `std::pmr` thêm một tầng gián tiếp — mọi container `pmr` dùng **cùng một kiểu allocator**, chỉ khác **con trỏ tới memory_resource** ở runtime.

**Chi phí của tầng gián tiếp đó:** mỗi lần cấp phát là một **lời gọi hàm ảo**. Với `monotonic_buffer_resource` thì thân hàm chỉ là dịch con trỏ, nên vẫn rẻ hơn `malloc` nhiều.
</details>

**Q10. Đọc code này — nó sai ở đâu?**
```cpp
template <size_t N> class Arena {
    std::byte buffer_[N];               // (a)
    std::byte* ptr_;
    bool in_buffer(std::byte* p) {
        return p >= buffer_ && p < buffer_ + N;    // (b)
    }
};
```
<details><summary>Đáp án</summary>

**Hai lỗi, cả hai đều là lỗi im lặng.**

**(a) Thiếu `alignas`.** `std::byte buffer_[N]` chỉ được bảo đảm alignment của `std::byte`, tức **1**. Nếu arena này nằm trên stack, nó có thể bắt đầu ở địa chỉ lẻ ⇒ mọi địa chỉ nó trả về đều sai alignment cho `double`, con trỏ, `int64_t`. Hậu quả: chậm trên x86, **crash trên ARM/nền tảng khắt khe**.

```cpp
static constexpr size_t alignment = alignof(std::max_align_t);
alignas(alignment) std::byte buffer_[N];   // ✅
```

**(b) So sánh quan hệ giữa raw pointer thuộc các object rời rạc là UNDEFINED BEHAVIOR** (tr. 223). Chuẩn C++ chỉ định nghĩa `<`, `>=` giữa các con trỏ **trong cùng một mảng/object**. `p` đến từ đâu đó bên ngoài thì so sánh với `buffer_` là UB — và **compiler tối ưu được phép lợi dụng**, cho ra kết quả bất ngờ (vd giả định luôn `false`).

```cpp
return std::uintptr_t(p) >= std::uintptr_t(buffer_) &&
       std::uintptr_t(p) <  std::uintptr_t(buffer_) + N;    // ✅ ép sang số nguyên rồi so
```

**Lỗi thứ ba, về thiết kế** (không có trong snippet nhưng nên nêu): `Arena` **phải cấm copy** (`= delete` copy ctor và copy assign). Nó sở hữu một vùng nhớ cụ thể và phát con trỏ vào vùng đó; copy arena sẽ tạo ra hai chủ sở hữu cho cùng một tập con trỏ đã phát.

**Vì sao câu này đáng hỏi:** cả hai lỗi đều **không sinh warning**, **không crash lúc test trên x86**, và chỉ lộ ra trên phần cứng thật hoặc khi bật `-O2`.
</details>

---

## Đọc thêm (tuỳ chọn — nội dung trên đã tự chứa)

- [03/memory-management](../../03-operating-system/memory-management.md) — virtual memory, paging, RSS ở góc OS
- [08/constraints.md](../../08-embedded-systems/constraints.md) — vì sao tránh heap trên embedded (chương này là phần *"thay bằng gì"*)
- [08/memory-and-startup](../../08-embedded-systems/memory-and-startup.md) — layout bộ nhớ bare-metal, `.bss`/`.data`, stack MCU
- [data-structures-cache.md](data-structures-cache.md) — cụm 2 của cuốn này: vì sao object nhỏ lại nhanh hơn
- [cpp-mindset/data-structures.md](../cpp-mindset/data-structures.md) — giới thiệu PMR ở mức nhập môn
