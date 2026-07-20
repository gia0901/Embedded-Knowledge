# C++ Mindset — Ch. 5: Data Structures (tr. 119–143) ✅ đã đọc sâu toàn văn

> Thuộc [The C++ Programmer's Mindset](README.md). Nguồn: đọc trực tiếp PDF, trang sách = trang PDF − 25.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> Ý đồ chương (tr. 119): chọn container phù hợp + hiểu **hệ quả hiệu năng** của lựa chọn. Câu thần chú lặp lại nhiều lần: ***"just use a vector"*** (tr. 122). Nửa sau chương đào sâu map/set và **cách đo benchmark** để lộ đặc tính hiệu năng thật. **Giá trị embedded cao**: static vector (không cần heap), small vector, allocator căn lề, SoA/AoS, flat map.

---

## Cụm 1 — Stack, heap, con trỏ & RAII: "just use a vector" (tr. 120–123)

### Nội dung chính

**Con trỏ = một tầng gián tiếp (indirection) (tr. 120):** mọi dữ liệu ở mức thấp nhất là dãy bit; kiểu cơ bản (int, float) là building block. **Address/pointer** cho phép tham chiếu tới dãy bit khác → thêm một tầng gián tiếp → *"a compact block of memory to hold the representation of a much larger piece of data."*

**Stack vs Heap (tr. 120–121):**

| | Stack | Heap (free store) |
|---|---|---|
| Quản lý | OS quản; mỗi lời gọi hàm thêm một **stack frame** | Lập trình viên quản (trực tiếp/gián tiếp) |
| Kích thước | Nhỏ, giới hạn; ít cách tác động (`alloca` — không khuyến khích) | Lớn, chứa khối lượng lớn |
| Vòng đời | Theo scope hàm | **Sống lâu hơn một lời gọi hàm** |
| Tốc độ | Đỉnh stack thường nằm trong **L1 cache** → gần như không phạt | Chậm hơn |

Câu chốt (tr. 121): heap là *"essential, or all data could only be defined in the outermost function... too restrictive to be of any use."* Modern C++ giấu độ khó quản heap sau **RAII** (resource acquisition is initialization): object giữ con trỏ tới bộ nhớ ngoài, destructor dọn dẹp — compiler chèn destructor khi ra scope *"even if an error occurs. This is almost always preferable to managing freeing memory by hand."*

**Ví dụ minh họa sức mạnh con trỏ — số nguyên vô hạn (tr. 121–122):** kiểu int cơ bản bị giới hạn số bit (int32 tối đa 2.147.483.647); crypto cần số cực lớn (RSA ~4096 bit). Dùng **một tầng gián tiếp** tạo được số nguyên "lớn tùy ý":

```cpp
class InfinitePrecisionInt {
    uint64_t *limbs_ = nullptr;    // con trỏ tới mảng "limb" (khối 64-bit) — cấp trên heap
    int32_t alloc_ = 0;            // số limb đã cấp phát
    int32_t size_and_sign_ = 0;    // |giá trị| = số limb đang dùng; dấu = dấu của số
};
```
Về cơ bản là một `std::vector` thu nhỏ (tr. 121). Mỗi phép tính phải quyết số bit cần rồi realloc `limbs_` — phức tạp hơn số cố định nhưng đó là cái giá cho tính linh hoạt; *"this kind of implementation can be made very fast. For instance, see the GNU Multiple Precision Arithmetic Library (GMP)"* (tr. 122).

**Arrays & "just use a vector" (tr. 122):** sách dùng thuật ngữ **linear memory** cho "dãy object liền kề". Ba dạng:
- `std::array` — wrapper quanh C-array, **kích thước cố định lúc compile**, sống inline (stack). Dùng làm **buffer nhỏ** thao tác dồn dập (chính là tile trong matmul ch. 4).
- `std::vector` — **linear memory tăng trưởng được**, dạng dùng phổ biến nhất.
- `std::unique_ptr<T[]>` — *"benefits are limited compared to a vector."*
→ Mantra: ***"just use a vector"*** (tr. 122).

**Structured memory — chọn compact hay không (tr. 122–123):** class/struct gom nhiều object. Quyết định thiết kế then chốt:
- Struct **xuất hiện nhiều lần** trong workflow điển hình + không thường xuyên đụng dữ liệu sau con trỏ → làm **compact** (như InfinitePrecisionInt: chỉ đụng limbs khi tính, và cần đụng nhiều cùng lúc → mảng struct vừa cache).
- Ngược lại, một global config object lớn xuất hiện đúng một lần trên heap → **ít tầng gián tiếp tốt hơn** (nhiều indirection thì hại hiệu năng). Câu chốt (tr. 123): *"At all times, you should consider the intent of an object and where it is likely to be used, which determines what characteristics to prioritize."*

### Insight đáng nhớ

- Bộ ba "con trỏ = indirection → linh hoạt" (tr. 120), ví dụ InfinitePrecisionInt (tr. 121), và "just use a vector" (tr. 122) gói một luận điểm: **tính linh hoạt luôn mua bằng một tầng gián tiếp, và tầng gián tiếp luôn có giá (cache miss, phân bổ heap)** — nên mặc định chọn thứ *ít gián tiếp nhất mà vẫn đủ dùng*, tức vector/linear memory. 🆕 Đây là phản đề của thói quen "mọi thứ là object trỏ tới object" (Java-style); C++ thắng hiệu năng nhờ *value semantics + linear memory*.
- Câu "consider the intent and where it is likely to be used" (tr. 123) là kim chỉ nam thiết kế struct: 🆕 layout không có "đúng tuyệt đối", chỉ có "đúng cho pattern truy cập" — nối thẳng SoA/AoS (cụm 3) và cache line (ch. 4 tr. 93).

---

## Cụm 2 — Allocators & smart pointers (tr. 123–127)

### Nội dung chính

**Allocation (tr. 123):** tạo object trên heap cần **allocation** (thường `operator new`), dọn bằng `operator delete`. Modern C++ ưu tiên smart pointer + factory (`make_unique`/`make_shared`).

**Allocators (tr. 123):** container như `std::vector` dùng **allocator** để tùy biến cách tương tác bộ nhớ. Default allocator dùng `new`/`delete`; đổi được qua template argument. Std còn cung cấp **polymorphic allocator** (dùng chuỗi `memory_resource` — gồm pool/arena allocator). Trường hợp dùng thật (tr. 123): làm việc với vector float + SIMD → cần **aligned allocator** (căn lề bộ nhớ — dùng biến thể `operator new` nhận `std::align_val_t`, chính là ch. 4). ⚠️ Lưu ý (tr. 123): `std::vector` với allocator khác nhau là **kiểu khác nhau** → không truyền lẫn nhau qua reference.

**AlignedAllocator (tr. 123–125)** — cài đặt tối giản (rút gọn):

```cpp
template <typename T, std::size_t Alignment=32>
class AlignedAllocator {
    static constexpr std::align_val_t alignment { Alignment };
public:
    using value_type = T;
    constexpr AlignedAllocator() noexcept = default;
    // ... constructor copy từ AlignedAllocator<U>, operator= ...

    [[nodiscard]] T* allocate(std::size_t size) {
        auto* ptr = ::operator new[](size * sizeof(T), alignment);   // new có tham số căn lề
        return static_cast<T*>(ptr);
    }
    void deallocate(T* ptr, std::size_t size) noexcept {
        ::operator delete[](ptr, size, alignment);
    }
    template <typename U> struct rebind { using other = AlignedAllocator<U, Alignment>; };
    // operator== / != so theo Alignment
};
```
Chi tiết quan trọng (tr. 125): `allocator_traits` lo phần method/type còn thiếu; **`rebind`** là cơ chế container đổi kiểu cấp phát (vd linked list cần cấp phát *node*, không phải *element* — rebind từ element type sang node type). Std cung cấp 2 allocator: `std::allocator` (default) và **polymorphic allocator**. Pool allocator đơn giản bằng std (tr. 125):

```cpp
#include <memory_resource>
std::pmr::unsynchronized_memory_resource resource {{512, 16}};
std::pmr::polymorphic_allocator pool_allocator(&resource);
// hoặc std::pmr::set_default_resource(&resource);
```
Cách robust hơn: giữ resource trong static, gọi `init` set default resource cho mọi `std::pmr::polymorphic_allocator` — đặc biệt mạnh khi dùng container re-export qua `std::pmr` namespace (chúng đổi default `std::allocator` thành `polymorphic_allocator`) (tr. 125).

**Smart pointers (tr. 126–127):**
- **`unique_ptr`** — sở hữu độc quyền, delete object khi ra scope.
- **`shared_ptr`** — đếm tham chiếu; ref count = 0 thì delete. Cấu trúc: **2 con trỏ** (data + control block chứa ref count + deleter + weak count). ⚠️ (tr. 127) *"twice as large as a basic pointer"* — vấn đề khi bộ nhớ hiếm.
- **`weak_ptr`** — tham chiếu **không sở hữu**; giữ con trỏ tới control block, biết object còn sống không (qua ref count); dùng cho **cache** và **phá vòng tham chiếu**. Ví dụ cây (tr. 126):

```cpp
struct TreeNode {
    std::weak_ptr<TreeNode> parent;                    // con → cha: WEAK (không giữ sống → không vòng)
    std::vector<std::shared_ptr<TreeNode>> children;   // cha → con: SHARED (sở hữu)
};
```
- `make_unique`/`make_shared` giấu `new` tường minh (mọi `new` phải cặp với `delete`, factory giấu điều đó) (tr. 126). `make_shared<T[]>` từ C++20, các bản khác từ C++14.
- shared_ptr **thread-safe** (ref count) nhưng đối tượng trỏ tới thì không (tr. 127). Thay thế khi cần tiết kiệm: Boost **`intrusive_ptr`** — ref count nằm *trong* object (dùng hàm ngoài để tăng/giảm/delete), **kích thước bằng một con trỏ** (tr. 127); hữu ích khi object đã tự quản ref count (COM objects của Windows).

### Insight đáng nhớ

- Cây `TreeNode` với `weak_ptr parent` + `shared_ptr children` (tr. 126) là bài mẫu **phá reference cycle** gọn nhất: 🆕 quy tắc tổng quát — trong quan hệ sở hữu có chu trình, chọn **một chiều làm weak** (chiều "ngược lên"/"quan sát"); nếu cả hai chiều đều shared thì ref count không bao giờ về 0 → leak. Trùng khớp [OSTEP concurrency](../ostep/concurrency.md) (vòng shared_ptr) và [EMC++ Item 20](../effective-modern-cpp.md).
- `intrusive_ptr` bằng kích thước một con trỏ (tr. 127) là chi tiết đắt cho embedded: 🆕 khi RAM hiếm và cần shared ownership, shared_ptr (2 con trỏ + control block cấp riêng) đắt gấp đôi; intrusive_ptr nhúng ref count vào object → 1 con trỏ, 1 lần cấp phát. Cùng động cơ với `make_shared` gộp control block (một cấp phát thay hai).
- Aligned allocator + `rebind` (tr. 123–125) minh họa allocator không phải "chuyện học thuật": 🆕 với SIMD (căn lề), pool/arena (giảm phân mảnh + latency xác định cho realtime — không gọi malloc trong hot path), PMR là công cụ thật để kiểm soát bộ nhớ mà không đổi code container.

---

## Cụm 3 — Linear memory: vector, SoA/AoS, stack & queue (tr. 127–130)

### Nội dung chính

**Vector — nhắc lại tăng trưởng (tr. 127):** grow bằng realloc + move/copy khi hết chỗ; mỗi lần grow nhân **bội số** kích thước cũ (thực tế thường ×2) → push_back **amortized O(1)** (đã chứng minh ở ch. 3 tr. 68). Nhưng **chèn giữa vector rất đắt** — phải dời mọi phần tử phía sau tiến một bước.

**SoA vs AoS (tr. 127–129)** — ví dụ game:

```cpp
struct Physics { float x, y; float delta_x, delta_y; };
struct Player  { std::string username; Physics physics; /* nhiều field khác */ };

// AoS — array of structs:
void update_physics(std::span<Player> players, float delta_t) {
    for (auto& player : players) {
        player.physics.x += delta_t * player.physics.delta_x;   // chỉ đụng physics,
        player.physics.y += delta_t * player.physics.delta_y;   // nhưng username + field khác
    }                                                           // vẫn bị kéo vào cache!
}
```
Vấn đề (tr. 128): *"a large amount of data in the struct that is not touched at all during this calculation... spaces out the data and degrades the nice cache locality."* → đảo thành **SoA (struct of arrays)**:

```cpp
class Players {                              // SoA — struct of arrays
    std::vector<std::string> usernames;
    std::vector<Physics> all_physics;        // mọi Physics liền kề → vừa cache, vectorize được
public:
    void update_physics(float delta_t) {
        for (auto& physics : all_physics) {  // chỉ duyệt đúng dữ liệu cần
            physics.x += delta_t * physics.delta_x;
            physics.y += delta_t * physics.delta_y;
        }
    }
};
```
Đẩy tiếp: gom **mọi component của mọi entity** (không chỉ Player) vào các array riêng, update tất cả cùng lúc — vì *"the physics component update is always the same regardless of what entity it belongs to"* → đây là mầm của **entity component system (ECS)**, nền của game engine hiện đại (cần thêm indexing system theo dõi entity/component nào còn sống — ngoài phạm vi sách) (tr. 129).

**Stacks & queues (tr. 129):** **queue** hỗ trợ push + pop. Hai loại: **FIFO** (first in first out — pop đầu, push cuối) và **LIFO** (last in first out = **stack** — push/pop cùng một đầu). Cài đặt:
- Stack (LIFO) → **vector** hiệu quả (push/pop ở cuối đều rẻ).
- FIFO → vector **kém** (`pop_front` phải dời mọi phần tử) → dùng **`std::deque`** (double-ended queue): vector-like nhưng lưu các **khối nhỏ** phần tử + metadata, push/pop hai đầu đều nhanh.
- Adapter: **`std::stack`** (LIFO), **`std::queue`** (FIFO), **`std::priority_queue`** (pop phần tử priority lớn/nhỏ nhất). Default container của stack/queue là `deque`.

### Insight đáng nhớ

- SoA/AoS ở đây bổ sung góc **ECS** cho phần đã học ở [ch. 4 tr. 93](understanding-the-machine.md): 🆕 AoS = "nghĩ theo object" (tự nhiên với OOP), SoA = "nghĩ theo pattern truy cập" (nhanh khi hot loop chỉ đụng vài component qua nhiều entity). Câu quyết định: *cái gì được duyệt cùng nhau trong hot loop thì đặt liền kề nhau* — trùng "consider where it is likely to be used" (cụm 1 tr. 123). (Câu interview SoA/AoS đầy đủ đã có ở ch. 4.)
- Bảng chọn stack/queue (tr. 129) neo vào lý do cấu trúc: 🆕 **vector cho LIFO** (thao tác ở cuối rẻ), **deque cho FIFO** (thao tác hai đầu rẻ vì lưu theo khối, không phải một khối liền) — đừng dùng vector làm FIFO (pop_front là O(n)). Đây là ví dụ "chọn container theo thao tác nóng", không theo thói quen.

---

## Cụm 4 — Static vector, small vector, span: công cụ không-heap (tr. 130–134) 🎯 embedded

### Nội dung chính

**Vấn đề của vector — iterator invalidation (tr. 130):** iterator của vector về cơ bản là con trỏ; **truy cập tuần tự cực nhanh & dễ đoán** (tốt cho cache/CPU) → *"using a vector might have a profound impact on runtime performance, even if it doesn't change the computational complexity."* Nhưng khi vector realloc, **iterator/con trỏ cũ thành dangling** — vector là **non-stable container** (linked list là stable). Cách phòng: truyền argument bằng iterator (ranges) hoặc `std::span`; nếu buộc giữ tham chiếu lâu, dùng **index** thay iterator (an toàn hơn khi vector không bị resize) — *"common in game engines in conjunction with an entity component system."*

**Static vector (tr. 130–132)** — 🎯 công cụ embedded quan trọng: vector mà **capacity cố định lúc compile**; thêm/xóa được nhưng **không grow** khi hết chỗ. *"very useful if the maximum size... is known (and is not too large) or in situations where a heap is not implemented (e.g., embedded applications)"* (tr. 130). Không có trong std (có trong **Boost.Container**; C++26 sẽ thêm). Đặc tính: **iterator không bao giờ invalidate** (không resize), chèn/xóa ở cuối gần như **zero cost** (không cần resize logic).

Điểm tinh tế khi cài (tr. 131–132): chèn có thể **fail** (hết chỗ) → không nên throw (throw cần cấp phát heap — thứ ta đang tránh!); trả `std::optional`/`bool`/`outcome`. Và T có thể **không default-constructible** → không dùng `std::array` làm storage được. Khung StaticVector:

```cpp
template <typename T, size_t Size>
class StaticVector {
    alignas(T) char storage_[Size * sizeof(T)] { 0 };   // bộ nhớ THÔ chưa khởi tạo, căn lề T
    size_t current_size_ = 0;
public:
    [[nodiscard]] T* data() noexcept
    { return std::launder(reinterpret_cast<T*>(storage_)); }   // launder: rào để compiler
                                                               // không "nhớ" metadata kiểu char cũ
    [[nodiscard]] bool emplace_back(T&& value) {               // caller PHẢI xử lý fail (bool)
        if (current_size_ < Size) {
            ::new (data() + current_size_) T(std::move(value));  // PLACEMENT NEW: dựng object
            ++current_size_;                                     //   tại vị trí đã có sẵn
            return true;
        }
        return false;                                           // hết chỗ → báo fail, KHÔNG throw
    }
};
```
Vì sao dùng `char storage_[...]` thay vì `T[Size]` (tr. 132): để **bộ nhớ khởi đầu là chưa-khởi-tạo** — không muốn compiler dựng sẵn các T (có thể không dựng được). Chỉ truy cập qua `data()`. `std::launder` (C++17) không bắt buộc nhưng nên có; C++23 có `begin_lifetime_as` đúng mục đích hơn (chưa compiler nào đủ hỗ trợ). Chèn dùng **placement new**; nên đánh `noexcept` có điều kiện (lược cho dễ đọc).

**Small vector (tr. 132)** — lai static + normal vector: chứa **buffer nhỏ tĩnh** (kích thước tùy template param), không cấp heap khi còn trong buffer; vượt quá thì grow như vector thường. *"very fast storage solution for vectors that are more likely to remain relatively small."* Không có trong std (trừ `std::string` — vốn là small vector cho char, gọi là **SSO**); có ở **Abseil**, **Boost.Container**; C++26 sẽ thêm. ⚠️ (tr. 132): small vector là object **khá lớn** (vì buffer nội bộ); và không truyền được qua reference tới hàm nhận `std::vector` (kiểu khác nhau).

**Views & spans (tr. 133–134):** khi thao tác linear memory thường **không cần biết ai quản lý bộ nhớ** — chỉ cần con trỏ đầu + kích thước. **`string_view`** (C++17) và **`span`** (C++20) là **view không sở hữu, agnostic về cách quản lý** — cặp (pointer, size), dùng ở đâu cần linear memory, thường là **tham số hàm** (luôn truyền **by value** để compiler tối ưu). `string_view` chỉ đọc (sửa chuỗi UTF-8 có thể cần realloc). Span hai dạng: size cố định (template — như `std::array`) hoặc **`std::dynamic_extent`** (mặc định, linh hoạt). ⚠️ Hạn chế (tr. 133–134):
- Nhiều interface std chưa dùng view (vd **Regex library** không dùng string_view → copy nội bộ → phạt hiệu năng).
- map key kiểu `std::string` **không** query được bằng `string_view`:
```cpp
std::unordered_map<std::string, double> map;
map["test"] = 1.0;
std::string_view sv_key("test");
auto& val = map[sv_key];    // ⚠️ KHÔNG compile — key là std::string, không nhận string_view
```
- span **không** dùng được `__restrict` (ch. 4) → không hứa được "không alias".
Nhưng các trở ngại này *"rather niche and should not discourage using std::span and std::string_view"* (tr. 134).

### Insight đáng nhớ

- **Static vector là câu trả lời "container trên hệ không có heap"** (tr. 130) — 🎯 đúng nghề embedded: nhiều hệ nhúng/bare-metal cấm cấp phát động (không xác định thời gian, phân mảnh, không có heap). Static vector cho "vector API" trên storage tĩnh: capacity cố định, insert-ở-cuối O(1), iterator không invalidate, fail trả bool thay vì throw. 🆕 Nối [MELP debug-realtime](../melp/debug-realtime.md): cấm malloc trong đường RT → pre-allocate; static vector là hiện thân của nguyên tắc này ở tầng container.
- Kỹ thuật `alignas(T) char storage_[]` + placement new + `std::launder` (tr. 131–132) là **bài mẫu quản lý object lifetime thủ công** — 🆕 chính là cách std::vector/optional/variant tự cài bên trong: tách **cấp phát bộ nhớ** khỏi **dựng object** (placement new dựng object trên bộ nhớ đã có). Hiểu nó là hiểu vì sao "raw storage + construct on demand" cho phép chứa cả kiểu không-default-constructible.
- SSO của `std::string` = small vector cho char (tr. 132) khép lại vòng với [EMC++ Item 29](../effective-modern-cpp.md): 🆕 đây là lý do **move string ngắn không rẻ hơn copy** (dữ liệu nằm trong buffer nội bộ, không có con trỏ để steal) — và vì sao string là object "khá lớn".

---

## Cụm 5 — Linked list & stable vector (tr. 134–135)

### Nội dung chính

**Linked list (tr. 134, Figure 5.1):** tập **node**, mỗi node chứa data + con trỏ tới node kế (singly) và node trước (doubly). Ưu: data độc lập vị trí; chèn tại điểm đã biết **rất rẻ** (chỉ sửa con trỏ node xung quanh). Nhược **lớn** (tr. 134): *"Traversing a linked list is very expensive, compared to traversing a vector. This is because moving from one node to the next involves following a pointer indirection, which makes predicting which page in memory to find rather tricky for the processor and disrupts the operation of the cache."* — mỗi bước là một con trỏ nhảy → cache/prefetch bó tay.

Dùng linked list khi (tr. 135): dữ liệu **lớn**, chèn/xóa **thường xuyên**, cần thao tác toàn cấu trúc **không invalidate** tham chiếu. Nhưng nếu app cần **duyệt lặp lại** → vector phù hợp hơn. Linked list là nền của nhiều cấu trúc std (map/set) và free list của memory page (ch. 4).

**Stable vector (tr. 135):** lai vector + linked list — grow như vector, data **(hầu như) liền kề**, nhưng khi grow thì **cấp khối mới làm node của linked list** thay vì move data cũ → **không invalidate tham chiếu khi grow** (do đó "stable"). Có gần hết ưu điểm cả hai, chỉ phạt nhẹ khi duyệt (phải theo con trỏ ở cuối mỗi khối). `std::deque` theo pattern tương tự. Có ở Boost.Container. Thay thế nhẹ hơn: `std::vector<std::unique_ptr<T>>` — đảm bảo **value-pointer stability** (không iterator stability) nhưng có phạt hiệu năng (tr. 135).

### Insight đáng nhớ

- Câu về traversal linked list (tr. 134) là **lý do cache** để mặc định *không* dùng linked list — 🆕 dù big-O của "duyệt" là O(n) cho cả vector lẫn list, hằng số khác nhau *hàng chục lần* vì list phá vỡ prefetch (mỗi node ở page ngẫu nhiên). Đây là minh chứng nữa cho "machine sympathy > big-O" (ch. 3–4): cùng O(n) traversal, vector thắng list áp đảo. Trùng khớp [13-dsa](../../13-dsa/) và là câu trả lời cho "vì sao std::list gần như không bao giờ nên dùng".
- Stable vector / `vector<unique_ptr<T>>` (tr. 135) cho thấy **stability và cache locality là trade-off**: 🆕 muốn tham chiếu không hỏng khi grow thì phải hy sinh phần nào tính liền kề (thêm indirection). Chọn theo nhu cầu: cần giữ con trỏ lâu qua các lần grow → stable; chỉ cần duyệt nhanh → vector thường + dùng index thay con trỏ (cụm 4).

---

## Cụm 6 — Maps, sets, hash & benchmark (tr. 135–142) 🎯

### Nội dung chính

**Set & map order-based (tr. 135–136):** **set** = container mà mọi phần tử **không bằng nhau** (thêm bản trùng không tạo phần tử mới) — theo dõi "đã thấy những gì". **map** = set mà một phần key không tham gia so sánh bằng → quan hệ **key → value**, cực hữu ích cho **memoize** kết quả tính đắt. Std `map`/`set` **lưu có thứ tự + binary search** → lookup **O(log n)**; cài bằng **cây nhị phân** (thường **red-black tree** — Figure 5.2: node có con trỏ left/right/parent + data + màu đỏ/đen).

**Flat map & flat set (tr. 136–137):** cài set/map trong **vector** thay linked list → *"far greater performance"* cho **truy cập** (linear memory thắng linked list — như đã thấy); nhưng **chèn/xóa đắt hơn** (vector). C++23 có adapter `std::flat_set`/`std::flat_map` (flat_map = flat_set của key + vector value). Boost.Container cài khác: key+value chung trong `std::pair` trong một vector (ít linh hoạt nhưng nhanh hơn — không phải nhìn sang khối key riêng). *"flat maps have similar algorithmic properties to std::map based on linked lists"* (tr. 137) — cùng O(log n) nhưng hằng số nhỏ hơn.

**Hash functions (tr. 137–138):** map order-based cần kiểu **có thứ tự** + lookup O(log n) (thành vấn đề khi nhiều phần tử). Thay bằng **hash**: hàm biến object thành `size_t` thỏa: nếu `obj1 == obj2` thì `h(obj1) == h(obj2)`; phân bố đều; **nhanh**. Std có `std::hash` nhưng hạn chế: phải tự cài cho kiểu riêng, và **`std::pair` không có `std::hash`** (pitfall lớn). Vấn đề **kết hợp hash** (tr. 137):

```cpp
struct KeyObject { std::string name; int index; };
// Ý định cài std::hash cho KeyObject bằng cách kết hợp hash từng field:
namespace std {
template <> struct hash<KeyObject> {
    size_t operator()(const KeyObject& obj) noexcept {
        auto h1 = std::hash<std::string>{}(obj.name);
        auto h2 = std::hash<int>{}(obj.index);
        return h1 + h2;    // ⚠️ SAI — cộng đơn giản phá vỡ tính chất phân bố của hash
    }
};
}
```
Std **không** cung cấp cách kết hợp hash *"and for good reason. A general-purpose combine function would disrupt the carefully crafted properties of the underlying hash algorithm"* (tr. 138). Giải pháp tốt: Boost `hash_combine`, hoặc **Abseil** (tốt hơn) — cài **friend function** để lộ trạng thái nội bộ của thuật toán hash:

```cpp
struct KeyObject {
    std::string name;
    int index;
    template <typename H>
    friend H AbslHashValue(H hasher, const KeyObject& obj) noexcept {
        return H::combine(std::move(hasher), obj.name, obj.index);   // combine đúng cách
    }
};
// absl::Hash<KeyObject> trở thành drop-in thay std::hash<KeyObject>
```

**Hash set & hash map (tr. 138–139):** lưu data trong **bucket**, hash quyết định bucket. **Collision** = hai giá trị cùng hash → cùng bucket (không thảm họa, phải xử lý). **Load factor** = số phần tử / số bucket (trung bình mỗi bucket). Load factor lớn → hiệu năng giảm → chèn quá ngưỡng kích hoạt **rehash** (resize mảng bucket — nên hiếm xảy ra; open addressing thường đặt max load factor ~**0.7**). Lookup **O(1)** nhưng hằng số **lớn** (tính hash + giải quyết collision); chèn **amortized O(1)**, worst-case O(n) (rehash chạm mọi phần tử) (tr. 139). Std: **`std::unordered_set`/`std::unordered_map`** ("unordered" vì phần tử trong bucket không theo thứ tự nào); cài trên linked list → tốt cho chèn/xóa nhưng phạt truy cập/duyệt. Dùng `boost::hash`/`absl::hash` cho phép dùng `std::pair`/`tuple` làm key không cần code thêm (tr. 139).

**Flat hash map (tr. 139):** hash set/map lưu trong **linear memory** thay linked list → truy cập/duyệt nhanh hơn, chèn/xóa phức tạp hơn. Không có trong std; có ở **Boost (unordered)** và **Abseil**. ⚠️ Không phải drop-in thay `std::unordered_*` (tr. 139): phần tử phải **move-constructible**, và **rehash invalidate con trỏ/tham chiếu** tới value bên trong.

**Map benchmarks (tr. 140–142)** 🎯 — dùng **Google benchmark**; đo random access (key ngẫu nhiên uniform để phá cache locality của key cố định). Điểm kỹ thuật (tr. 141): `benchmark::DoNotOptimize(value)` ép compiler giữ lệnh lookup (không xóa vì value không dùng). Bảng kết quả (tr. 142, AMD Ryzen 7900x, thời gian ns):

| Map class | Arg 256 | Arg 1024 |
|---|---|---|
| `std::map` (order/RB-tree) | 23.3 | 34.9 |
| `std::unordered_map` (hash) | **3.56** | **3.81** |
| `absl::btree_map` | 19.5 | 24.1 |
| `absl::flat_hash_map` | 3.81 | 3.89 |
| `absl::node_hash_map` | 3.95 | 4.29 |
| `boost::container::flat_map` (order) | 25.9 | 31.4 |
| `boost::unordered_map` (hash) | 3.68 | 3.66 |
| `boost::unordered_flat_map` | 3.93 | 4.03 |

Hai kết luận sách rút (tr. 142): (1) **hash map nhanh hơn order map ~10×** *"universally"*; (2) **hash map access ~hằng số theo kích thước** (3.56→3.81 khi tăng 4×) còn order map thì **tăng** (23.3→34.9) — đúng lý thuyết: binary search O(log N) vs hash O(1). Bất ngờ: flat vs node variant **không khác biệt rõ** — *"probably because the number of elements is small enough to fit into higher levels of cache"* (tr. 142).

### Insight đáng nhớ

- Bảng benchmark (tr. 142) là **bằng chứng số cho "std::map vs std::unordered_map"** — câu hỏi phỏng vấn quốc dân: 🆕 hash map ~10× nhanh hơn cho lookup **và** access time không tăng theo size (O(1) vs O(log N)). Nhưng lưu ý bối cảnh: benchmark chỉ đo **random access**; order map thắng khi cần **duyệt theo thứ tự**, range query, hoặc key không hash được. "Nhanh hơn" luôn kèm "cho thao tác nào".
- Vấn đề kết hợp hash (`h1 + h2` sai, tr. 137–138) dạy bài quan trọng: 🆕 **không tự chế hash combine bằng phép cộng/XOR ngây thơ** — phá phân bố, tăng collision. Dùng `boost::hash_combine`/Abseil `AbslHashValue`. Và nhớ pitfall `std::pair`/`std::tuple` **không có** `std::hash` sẵn → dùng key là pair phải mang boost/absl hash (hoặc tự viết đúng).
- `benchmark::DoNotOptimize` (tr. 141) là kỹ thuật benchmark thật phải biết: 🆕 compiler xóa code có kết quả không dùng → benchmark đo nhầm "0ns"; `DoNotOptimize`/`ClobberMemory` ép giữ lệnh. Trùng tinh thần ch. 3–4 "measure accurately" — và đo sai còn tệ hơn không đo.

---

## Góc interview

**Câu 1 (🎯 embedded):** Hệ nhúng của bạn cấm cấp phát động (no heap). Bạn cần một "vector" — làm thế nào? Trình bày static vector và các điểm cài đặt tinh tế.

<details><summary>Đáp án</summary>

- **Static vector** (tr. 130): vector có **capacity cố định lúc compile**, storage nằm **inline** (không heap); thêm/xóa được nhưng **không grow** khi đầy. Đúng cho hệ nhúng/bare-metal cấm malloc (không xác định thời gian, phân mảnh, hoặc không có heap). Có ở Boost.Container; C++26 sẽ chuẩn hóa.
- **Đặc tính:** iterator **không bao giờ invalidate** (không resize), chèn/xóa ở cuối **O(1) không cần resize logic** → thời gian xác định (tốt cho realtime).
- **Điểm cài đặt tinh tế** (tr. 131–132):
  1. Chèn có thể **fail** (hết chỗ) → **không throw** (throw cần cấp phát heap — thứ ta đang tránh) → trả `bool`/`std::optional` để caller xử lý.
  2. T có thể **không default-constructible** → không dùng `std::array<T,N>` làm storage → dùng **raw storage** `alignas(T) char storage_[N*sizeof(T)]` (bộ nhớ chưa khởi tạo, căn lề đúng T).
  3. Dựng object bằng **placement new** (`::new (data()+i) T(...)`) tại vị trí đã có; huỷ phải gọi destructor tường minh.
  4. `std::launder` (C++17) quanh `reinterpret_cast` để compiler không dùng metadata kiểu char cũ (C++23: `begin_lifetime_as`).
  5. Nên `noexcept` có điều kiện.
- 🆕 Nối nghề: đây là hiện thân tầng-container của nguyên tắc "pre-allocate, cấm malloc trong hot/RT path" ([MELP debug-realtime](../melp/debug-realtime.md)); và cùng kỹ thuật raw-storage + placement new mà `std::optional`/`variant`/`vector` dùng bên trong. **Small vector** (buffer tĩnh nhỏ + fallback heap khi vượt) là lựa chọn khi "thường nhỏ nhưng đôi khi lớn" và heap *có* tồn tại.

</details>

**Câu 2 (🎯):** `std::map` vs `std::unordered_map` vs `flat_map` — khác nhau thế nào về cài đặt và hiệu năng? Khi nào chọn cái nào? (dùng số nếu nhớ)

<details><summary>Đáp án</summary>

- **`std::map`** (order-based): cài bằng **red-black tree**, key **có thứ tự**, lookup **O(log N)** bằng binary search; backed bởi node linked → cache locality kém. Số của sách (tr. 142): 23.3ns (256 phần tử) → 34.9ns (1024) — **access time tăng theo size**.
- **`std::unordered_map`** (hash): key vào **bucket** theo hash, lookup **O(1)** (hằng số lớn: tính hash + giải collision); std cài trên linked list. Số: 3.56ns → 3.81ns — **~10× nhanh hơn map và gần như phẳng theo size** (đúng O(1) vs O(log N)).
- **`flat_map`/`flat_hash_map`** (tr. 136–139): cài trên **linear memory (vector)** thay linked list → truy cập/duyệt nhanh hơn (cache locality), nhưng **chèn/xóa đắt hơn** (dời phần tử vector) và **rehash invalidate con trỏ**. Có ở C++23 (`std::flat_map`), Boost, Abseil.
- **Chọn:**
  - Cần **lookup nhanh nhất**, không cần thứ tự → `unordered_map` (hoặc flat_hash_map nếu ít chèn/xóa + muốn cache tốt hơn).
  - Cần **duyệt theo thứ tự / range query / key không hash được** → `std::map`.
  - Cần **memoize** (bảng tra cứu, ít đổi, đọc nhiều) → flat hash map (đọc cực nhanh).
- ⚠️ Cảnh báo (tr. 139): flat variant **không** drop-in thay `std::unordered_*` (phần tử phải move-constructible; rehash làm hỏng con trỏ tới value). Và benchmark chỉ đo **random access** — "nhanh hơn" luôn cho *thao tác cụ thể*.
- 🆕 Điểm cộng: dùng `boost::hash`/`absl::hash` để dùng `std::pair`/`tuple` làm key (std::hash không có sẵn cho pair); và `benchmark::DoNotOptimize` khi tự đo (tránh compiler xóa lookup không dùng).

</details>

**Câu 3:** Vì sao `std::list` (linked list) gần như không bao giờ nên dùng dù big-O của chèn là O(1)? Khi nào nó *thực sự* phù hợp?

<details><summary>Đáp án</summary>

- Big-O đánh lừa: chèn tại điểm đã biết của list là O(1) (chỉ sửa con trỏ), của vector là O(n) (dời phần tử). Nhưng thực tế **duyệt** list rất chậm (tr. 134): mỗi bước node→node là một **con trỏ nhảy tới page ngẫu nhiên** → *"disrupts the operation of the cache"*, phá prefetch. Cùng O(n) traversal, vector thắng list **hàng chục lần** vì dữ liệu liền kề (một cache line 64 byte chứa nhiều phần tử, prefetch đoán đúng).
- Thêm nữa: để chèn O(1) bạn phải **đã có iterator tới vị trí đó** — mà tìm vị trí đó là O(n) traversal chậm; nên "chèn O(1)" hiếm khi là lợi thế thực tế.
- **Thực sự phù hợp** (tr. 135): dữ liệu **lớn** (copy/move đắt), chèn/xóa **rất thường xuyên ở vị trí đã có iterator**, và cần thao tác **không invalidate tham chiếu** tới phần tử khác (list là **stable container**, vector thì không). Ví dụ: free list của memory page (ch. 4), LRU cache (splice O(1) không move data).
- 🆕 Lựa chọn trung dung: cần stability mà vẫn muốn cache tốt → **stable vector** (Boost) hoặc `std::vector<std::unique_ptr<T>>` (value-pointer stable), hoặc **vector + index** thay con trỏ. Quy tắc thực chiến: mặc định vector; chỉ dùng list khi *đo được* rằng chèn/xóa/stability thắng chi phí traversal — hiếm.
- Chốt: đây là minh chứng kinh điển cho **"machine sympathy > big-O"** (ch. 3–4) — hằng số và cache behavior thường quan trọng hơn lớp complexity với dữ liệu thực tế.

</details>

**Câu 4:** Vì sao không nên tự viết hàm kết hợp hash bằng `h1 + h2` (hoặc `h1 ^ h2`)? Cách đúng để làm hashable một struct nhiều field?

<details><summary>Đáp án</summary>

- **Vì sao sai** (tr. 137–138): mỗi `std::hash<T>` được thiết kế để phân bố đều trên toàn dải `size_t`. Cộng/XOR ngây thơ **phá tính chất đó**: `h1 + h2` dễ trùng (hai cặp field khác nhau cho cùng tổng), XOR của hai giá trị giống nhau ra 0, và cả hai không trộn bit tốt → **tăng collision** → hash map tụt về O(n) trong bucket. Sách nói thẳng: std **không** cung cấp combine *"for good reason. A general-purpose combine function would disrupt the carefully crafted properties of the underlying hash algorithm."*
- **Cách đúng:**
  1. **Boost `hash_combine`** — trộn bit có tính toán (nhân số nguyên tố lớn + shift), gọi tuần tự cho từng field.
  2. **Abseil** (sách khen tốt hơn, tr. 138) — cài **friend function** `AbslHashValue` để lộ trạng thái hasher nội bộ:
     ```cpp
     template <typename H>
     friend H AbslHashValue(H hasher, const KeyObject& obj) noexcept {
         return H::combine(std::move(hasher), obj.name, obj.index);
     }
     ```
     rồi `absl::Hash<KeyObject>` thay `std::hash<KeyObject>`.
- Pitfall liên quan (tr. 137): **`std::pair`/`std::tuple` không có `std::hash` sẵn** → dùng chúng làm key của `unordered_map` không compile với std::hash; dùng `boost::hash`/`absl::hash` (chúng hỗ trợ pair/tuple sẵn) hoặc tự viết combine đúng.
- 🆕 Kiểm chứng chất lượng hash: đo **load factor** thực tế và phân bố bucket; hash tồi lộ ra ở collision rate cao / lookup chậm bất thường dù O(1) lý thuyết.

</details>

### Đọc thêm (tùy chọn)

- [understanding-the-machine.md](understanding-the-machine.md) — ch. 4 cache line / SoA-AoS (câu interview SoA/AoS đầy đủ ở đó); alignas/allocator.
- [algorithmic-complexity.md](algorithmic-complexity.md) — ch. 3 amortized (vector push_back), "machine sympathy > big-O".
- [13-dsa/](../../13-dsa/) — Big-O, container, trade-off của repo.
- [EMC++ Items 18–22](../effective-modern-cpp.md) — smart pointer (unique/shared/weak, control block, make_shared) chi tiết; Item 29 (SSO/move).
- [OSTEP concurrency.md](../ostep/concurrency.md) — vòng shared_ptr & weak_ptr phá vòng.
