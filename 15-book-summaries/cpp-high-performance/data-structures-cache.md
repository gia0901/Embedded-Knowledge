# Data Structures & Cache — *C++ High Performance* ch. 4 (tr. 99–130) 🎯

> **Nguồn summary:** đã đối chiếu PDF *C++ High Performance, 2nd Edition* (Andrist & Sehr, Packt 12/2020). **Offset: trang PDF = trang sách + 27.**
> **Vì sao cụm này:** repo đã có bảng tra container ([12-dsa/complexity-and-structures §4](../../12-dsa/complexity-and-structures.md)) và **đúng một đoạn văn** về cache locality (§5). Chương này biến đoạn văn đó thành **số đo cụ thể** — cùng một thuật toán, cùng độ phức tạp, chênh nhau **20 lần** chỉ vì thứ tự truy cập bộ nhớ.

---

## TL;DR cụm này

1. **Big-O nói dối về hằng số, và hằng số ở đây là ~200 lần.** L1 0,5 ns vs RAM 100 ns. Hai vòng lặp cùng O(n²) chênh nhau **40 ms vs 800 ms** chỉ vì đảo `[i][j]` thành `[j][i]`.
2. **Object nhỏ hơn ⇒ duyệt nhanh hơn**, tỉ lệ gần như tuyến tính. Sách đo: cùng một phép cộng, object 8 byte mất **1 ms**, object 260 byte mất **10 ms**.
3. **`std::vector` chỉ đổi capacity ở đúng 3 chỗ** ⇒ dự đoán được ⇒ **dùng được cả trong ngữ cảnh real-time**.
4. **Parallel arrays (SoA)** là vũ khí mạnh nhất và nguy hiểm nhất của chương: **11 ms → 0,7 ms**, đổi lại phá vỡ đóng gói và làm các mảng phải tự đồng bộ index.
5. **Bảo đảm độ phức tạp chỉ có ý nghĩa khi n đủ lớn.** Với n nhỏ, thuật toán O(n) thường thắng O(log n) vì overhead.

---

## Cụm 1 — Kiến trúc bộ nhớ & cache (tr. 99–103) 🎯

### Nội dung chính

**Xuất phát điểm sai lầm:** truy cập một byte theo địa chỉ là O(1) ⇒ dễ tin **mọi ô nhớ đều nhanh như nhau**. Sai, vì máy tính hiện đại có **phân cấp cache**, và khoảng cách giữa các tầng ngày càng lớn khi CPU nhanh lên còn RAM thì không.

Sách chạy `sysctl -a hw` trên MacBook Pro 2018 (Intel Quad-Core i7) và in ra số thật (tr. 100):

```
hw.memsize:       17179869184    (16 GB RAM)
hw.cachelinesize: 64             ← ⭐ CACHE LINE = 64 byte
hw.l1icachesize:  32768          (32 KB — cache lệnh, mỗi core)
hw.l1dcachesize:  32768          (32 KB — cache dữ liệu, mỗi core)
hw.l2cachesize:   262144         (256 KB — mỗi core)
hw.l3cachesize:   8388608        (8 MB — DÙNG CHUNG cả 4 core)
```

```
      ┌──core 0──┐ ┌──core 1──┐ ┌──core 2──┐ ┌──core 3──┐
      │ L1i  L1d │ │ L1i  L1d │ │ L1i  L1d │ │ L1i  L1d │  32 KB mỗi cái
      │    L2    │ │    L2    │ │    L2    │ │    L2    │  256 KB
      └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘
           └────────────┴──────┬─────┴────────────┘
                    ┌──────────┴──────────┐
                    │      L3  8 MB       │  (chia sẻ)
                    └──────────┬──────────┘
                    ┌──────────┴──────────┐
                    │     RAM  16 GB      │
                    └─────────────────────┘        (hình theo Figure 4.2)
```

> **Nhận xét của sách:** *"cache nhỏ đến mức nào so với lượng main memory sẵn có"* — 8 MB L3 trên 16 GB RAM là **0,05%**.

**⭐ Điểm quan trọng nhất về cache line:** khi truy cập **một byte**, máy **không lấy một byte** — nó luôn nạp **cả một cache line 64 byte**. Mọi tầng cache đều theo dõi theo khối 64 byte, không theo byte lẻ.

**Bảng độ trễ** sách trích từ *Latency numbers every programmer should know* (Jeff Dean, qua Peter Norvig 2001) (tr. 101):

| Thao tác | Độ trễ | So với L1 |
|---|---|---|
| **L1 cache reference** | **0,5 ns** | 1× |
| **L2 cache reference** | **7 ns** | **14×** |
| **Main memory reference** | **100 ns** | **200×** |

**Hai loại locality** — thuật ngữ phải nói đúng trong phỏng vấn:

| Loại | Nghĩa | Cách khai thác |
|---|---|---|
| **Temporal locality** | Dữ liệu **vừa dùng** có khả năng còn trong cache | dùng lại sớm, gom vòng lặp |
| **Spatial locality** | Dữ liệu **nằm gần** dữ liệu đang dùng có khả năng đã được nạp cùng cache line | lưu **liền mạch**, duyệt tuần tự |

**Cache thrashing — ví dụ số thật** (tr. 102):

```cpp
constexpr auto kL1CacheCapacity = 32768;              // đúng bằng L1 data cache
constexpr auto kSize = kL1CacheCapacity / sizeof(int);
using MatrixType = std::array<std::array<int, kSize>, kSize>;

auto cache_thrashing(MatrixType& matrix) {
    auto counter = 0;
    for (auto i = 0; i < kSize; ++i)
        for (auto j = 0; j < kSize; ++j)
            matrix[i][j] = counter++;    // ✅ ~40 ms
         // matrix[j][i] = counter++;    // ❌ >800 ms — CHỈ ĐỔI CHỖ i và j
}
```

**Chênh 20 lần. Cùng thuật toán, cùng O(n²), cùng số phép gán.**

Vì sao? **Không tồn tại bộ nhớ 2 chiều.** Ma trận nằm trong bộ nhớ 1 chiều, **hàng nối tiếp hàng**:

```
ma trận 3×3 ta hình dung:        thực tế trong bộ nhớ:
    ┌───┬───┬───┐
    │ 0 │ 1 │ 2 │                [0][1][2][3][4][5][6][7][8]
    ├───┼───┼───┤                 └─hàng 0─┘└─hàng 1─┘└─hàng 2─┘
    │ 3 │ 4 │ 5 │
    ├───┼───┼───┤   NHANH  [i][j]: 0→1→2→3→4...   stride = 1, cùng cache line
    │ 6 │ 7 │ 8 │   CHẬM   [j][i]: 0→3→6→1→4...   stride = kSize, MỖI BƯỚC MỘT LINE MỚI
    └───┴───┴───┘
                                          (hình theo Figure 4.3–4.6)
```

Bản chậm: mỗi lần truy cập nhảy `kSize` phần tử ⇒ **mỗi phép gán là một cache miss L1**. Bản nhanh: 64 byte nạp về chứa **16 số `int` liên tiếp** ⇒ 15 lần truy cập kế tiếp đều là cache hit.

**Prefetcher** (tr. 103): CPU hiện đại có bộ **tiên đoán truy cập** tự nạp trước dữ liệu sắp cần. Nó *"hoạt động tốt nhất với stride nhỏ"* — thêm một lý do nữa để duyệt tuần tự.

### Insight đáng nhớ

- **Cache line 64 byte là hằng số cần thuộc**, vì nó giải thích cả ba thứ: (1) vì sao `vector` thắng `list`, (2) vì sao object nhỏ duyệt nhanh, (3) vì sao có false sharing.
- 🆕 Đoạn code thrashing này là **bài lab tự nhiên**: gõ ra, đổi `[i][j]` ↔ `[j][i]`, đo bằng `ScopedTimer` ở [measuring-performance.md](measuring-performance.md). Đây đúng loại bài mà repo đã kết luận là cần — *biết* khác *đã làm*.

### Góc interview

**Q1. Hai vòng lặp cùng O(n²), cùng số phép gán, một cái chậm hơn 20 lần. Giải thích.**
<details><summary>Đáp án</summary>

**Cache miss do thứ tự truy cập (stride), không phải do độ phức tạp.**

Ma trận 2 chiều thực chất nằm trong bộ nhớ **1 chiều, hàng nối tiếp hàng** (row-major trong C/C++).

- `matrix[i][j]` với `j` là vòng trong ⇒ đi **tuần tự** theo bộ nhớ, stride = 1. Một cache line 64 byte chứa **16 `int`** ⇒ 1 miss rồi 15 hit.
- `matrix[j][i]` ⇒ mỗi bước nhảy nguyên **một hàng** (stride = kSize × 4 byte). Nếu hàng lớn hơn cache line, **mỗi lần truy cập là một cache line mới** ⇒ miss 100%.

**Con số neo:** L1 ≈ 0,5 ns, RAM ≈ 100 ns ⇒ chênh ~200×. Kết hợp với prefetcher (chỉ hiệu quả với stride nhỏ), sách đo được **40 ms vs 800 ms**.

**Nâng lên T2 — nói được ba hệ quả kéo theo:**
1. **Big-O bỏ qua hằng số**, mà ở đây hằng số là ~200. Với dữ liệu lớn hơn cache, **mẫu truy cập quan trọng ngang thuật toán**.
2. Đây là lý do **`std::vector` thường thắng `std::list`** dù cùng O(n) khi duyệt.
3. Đây là lý do **duyệt ma trận nên theo row-major trong C++** — và ngược lại trong Fortran/MATLAB (column-major).

**Bẫy:** trả lời *"vì vòng lặp trong chạy nhiều hơn"* — số phép gán **giống hệt nhau**. Thứ khác nhau là **thứ tự chạm bộ nhớ**.

Liên hệ bank [DSA-009](../../14-prep/mock-interview/bank/dsa.md).
</details>

---

## Cụm 2 — Sequence container (tr. 103–109)

### Nội dung chính

**Bốn câu phải trả lời trước khi chọn** (tr. 104):
1. Bao nhiêu phần tử (bậc độ lớn)?
2. Mẫu sử dụng: thêm/đọc/xoá/sắp xếp — cái nào thường xuyên?
3. Thêm ở **đâu**: đầu, cuối, hay giữa?
4. Có cần thứ tự không?

#### `std::vector`

**Size vs capacity** (tr. 105):

```
        ┌──────────────── capacity ────────────────┐
        │◄──── size ────►│                          │
buffer: [a][b][c][d][e][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
                          ↑ đã cấp phát, chưa dựng object
                                        (hình theo Figure 4.7)
```

`push_back` nhanh **miễn `size < capacity`**. Khi đầy, vector cấp buffer mới và **chuyển toàn bộ phần tử** sang. Capacity tăng theo cấp số nhân ⇒ `push_back` là **amortized O(1)**.

**⭐ Chi tiết ít ai biết — vector copy hay move?** (tr. 105–106)

```cpp
Person(Person&& other) { ... }            // ❌ vector sẽ COPY
Person(Person&& other) noexcept { ... }   // ✅ vector sẽ MOVE
```

Vector dùng `std::move_if_noexcept`. Nếu move constructor **không** `noexcept`, vector buộc phải **copy** để giữ **strong exception safety** (nếu move nửa chừng mà ném thì không khôi phục được trạng thái cũ). Kiểm tra lúc biên dịch:

```cpp
static_assert(std::is_nothrow_move_constructible<Person>::value);
```

**⭐ Capacity chỉ đổi ở đúng ba chỗ** (tr. 106):
1. `push_back`/`insert` khi `capacity == size`
2. `reserve()`
3. `shrink_to_fit()`

Ngoài ba chỗ đó, vector **không cấp và không giải phóng** bộ nhớ động. Đáng chú ý: **`clear()` làm rỗng nhưng KHÔNG đổi capacity.**

> 🎯 Nguyên văn kết luận của sách (tr. 106): *"These memory guarantees make the vector usable even in real-time contexts."* — **Chính bảo đảm này khiến `std::vector` dùng được cả trong ngữ cảnh real-time**: `reserve()` một lần lúc khởi tạo, sau đó `push_back`/`clear` tuỳ ý mà **không hề chạm allocator**.

**C++20:** `std::erase(v, value)` và `std::erase_if(v, pred)` thay cho erase-remove idiom.
```cpp
auto v = std::vector{-1, 5, 2, -3, 4, -5, 5};
std::erase(v, 5);                                 // v: [-1,2,-3,4,-5]
std::erase_if(v, [](auto x) { return x < 0; });   // v: [2,4]
```
> ⚠️ **C++20** — với JD C++17 vẫn phải dùng `v.erase(std::remove(v.begin(), v.end(), 5), v.end())`.

#### `std::array` và array decay

`std::array` cỡ cố định, **nằm trên stack**, và **kích thước là một phần của kiểu**:

```cpp
auto a = std::array<int, 16>{};
auto b = std::array<int, 1024>{};    // KHÁC KIỂU với a
auto f(const std::array<int, 1024>& input) { }
f(a);   // ❌ không biên dịch — và đó là ƯU ĐIỂM
```

So với mảng C — nơi kích thước **biến mất** khi truyền vào hàm:
```cpp
auto f(const int input[]) { }   // "trông như mảng", thực chất là CON TRỎ
int a[16];
f(a);   // ✅ biên dịch được, nhưng KHÔNG AN TOÀN: đã mất size
```
Hiện tượng này gọi là **array decay**. Sách nói `std::span` (C++20) là cách chữa tổng quát — xem cụm 4.

#### `deque`, `list`, `forward_list`, `basic_string`

| Container | Cấu trúc | Điểm riêng |
|---|---|---|
| `std::deque` | **tập hợp các mảng cỡ cố định** | O(1) truy cập theo index, thêm nhanh ở **cả hai đầu**, nhưng **KHÔNG liền mạch** toàn bộ |
| `std::list` | doubly linked list | duyệt được hai chiều; **tốn thêm con trỏ ngược** |
| `std::forward_list` | singly linked list | **rỗng chỉ chiếm 1 word** ⇒ hợp dữ liệu **thưa** (mảng lớn các list phần lớn rỗng) |
| `std::basic_string` | — | **C++17 bảo đảm liền mạch** ⇒ truyền được cho API cần mảng ký tự; hầu hết hiện thực dùng **SSO** |

**Splicing — thứ chỉ list có** (tr. 109): chuyển phần tử giữa hai list **không copy, không move**, chỉ đổi con trỏ. Nối hai list là **O(1)**; mọi container khác cần ít nhất tuyến tính.

> ⚠️ **Nhưng:** list **không liền mạch** ⇒ duyệt sinh **rất nhiều cache miss** so với vector. Đây là lý do `vector` thường thắng dù lý thuyết nói ngược.

**Đọc cả file vào string** (tr. 109) — mẹo gọn đáng nhớ:
```cpp
auto in = std::ifstream{"file.txt", std::ios::binary | std::ios::ate};  // ate = nhảy tới CUỐI
if (in.is_open()) {
    auto size = in.tellg();                    // ⇒ tellg() cho ngay kích thước file
    auto content = std::string(size, '\0');
    in.seekg(0);
    in.read(&content[0], size);
}
```

### Góc interview

**Q2. Vì sao `std::vector` dùng được trong code real-time còn `std::list` thì không nên?**
<details><summary>Đáp án</summary>

**Hai lý do độc lập.**

**① Vector có bảo đảm về CẤP PHÁT; list thì không.** Capacity của vector chỉ đổi ở **ba chỗ**: `push_back`/`insert` khi đầy, `reserve()`, `shrink_to_fit()`. Ngoài ra **không chạm allocator** — kể cả `clear()`.

⇒ Mẫu dùng được trong đường tới hạn:
```cpp
v.reserve(kMaxItems);      // lúc KHỞI TẠO — được phép cấp phát
// ... trong vòng lặp real-time:
v.clear();                 // KHÔNG cấp phát, KHÔNG giải phóng
v.push_back(x);            // KHÔNG cấp phát (còn trong capacity)
```

`std::list` **cấp phát một node mỗi lần thêm phần tử** và giải phóng mỗi lần xoá ⇒ mỗi thao tác là một lần gọi allocator, **thời gian không tất định**, và làm phân mảnh heap dần.

**② Cache.** Vector liền mạch ⇒ duyệt tuần tự, prefetcher hoạt động tốt. List rải rác ⇒ mỗi bước là một cache miss tiềm năng (~100 ns so với ~0,5 ns).

**Nâng lên T2 — khi nào `list` VẪN thắng:**
- Cần **splice**: nối/chuyển phần tử giữa hai list là **O(1)**, không copy/move.
- Cần **iterator/reference không bị vô hiệu hoá** khi thêm/xoá phần tử khác — vector reallocate là **mọi iterator hết hạn**.
- Phần tử **rất lớn và copy/move đắt**, và ta chèn/xoá ở giữa liên tục.

**Bẫy:** nói *"list chèn giữa O(1) nên nhanh hơn"* — O(1) **chỉ tính từ lúc đã có iterator**. Tìm được vị trí đó vẫn là O(n) và mỗi bước tìm là một cache miss.

Liên hệ bank [DSA-003](../../14-prep/mock-interview/bank/dsa.md), [DSA-009](../../14-prep/mock-interview/bank/dsa.md), [CPP-030](../../14-prep/mock-interview/bank/cpp.md).
</details>

**Q3. Class của bạn có move constructor nhưng `std::vector` vẫn copy khi nó lớn lên. Vì sao?**
<details><summary>Đáp án</summary>

**Vì move constructor không được đánh dấu `noexcept`.**

Khi vector reallocate, nó phải chuyển toàn bộ phần tử sang buffer mới. Chuẩn yêu cầu `push_back` có **strong exception guarantee**: hoặc thành công, hoặc container **giữ nguyên như cũ**.

- Nếu vector **copy** và có exception giữa chừng: buffer cũ **vẫn nguyên vẹn** ⇒ khôi phục được.
- Nếu vector **move** và có exception ở phần tử thứ k: các phần tử 0..k-1 đã bị "rút ruột" khỏi buffer cũ. Không có cách nào lùi lại ⇒ **vi phạm bảo đảm**.

Nên vector dùng `std::move_if_noexcept`: chỉ move khi move constructor **cam kết không ném**.

**Cách sửa và cách kiểm chứng:**
```cpp
Person(Person&& other) noexcept { /* ... */ }        // ✅
static_assert(std::is_nothrow_move_constructible<Person>::value);
```

**Nâng lên T2 — vì sao đây là bug hiệu năng KHÓ THẤY:** code vẫn đúng, test vẫn pass, không warning nào. Chỉ khi vector lớn lên mới lộ, và profiler chỉ vào constructor copy chứ không nói vì sao. Với `std::string`/`std::vector` làm member, copy nghĩa là **cấp phát heap cho từng phần tử**.

**Chi tiết dễ quên:** nếu class có member mà move ctor của member đó có thể ném (hiếm), hoặc bạn viết tay move ctor có gọi hàm có thể ném, thì **không được** đánh `noexcept` bừa — sai là `std::terminate`.

Liên hệ bank [CPP-011](../../14-prep/mock-interview/bank/cpp.md).
</details>

---

## Cụm 3 — Associative container & hash (tr. 109–116)

### Nội dung chính

**Hai họ** (tr. 110):

| | Ordered | Unordered |
|---|---|---|
| Cấu trúc | **cây** (self-balancing BST) | **hash table** |
| Yêu cầu ở key | so sánh `<` | so sánh `==` **và** hàm hash |
| Thêm/xoá/tìm | **O(log n)** | **O(1)** trung bình |
| Tên | `set`, `map`, `multiset`, `multimap` | `unordered_*` |
| Cấp phát | **mỗi lần chèn cấp một node**, mỗi lần xoá giải phóng | theo bucket |

**Hash table hoạt động thế nào** (tr. 111–112): tính hash của phần tử → giới hạn về kích thước mảng (thường bằng modulo) → dùng làm index vào mảng **bucket**.

**Hash collision** là chuyện **thường xuyên**, không phải trường hợp biên. Thư viện chuẩn xử lý bằng **separate chaining**: mỗi bucket là một container con (thường coi như linked list).

```
buckets:  0    1    2      3    4     5    6    7
        [ ] [ ] [•] [ ] [•] [•] [ ] [ ]
                   │       │    │
                   ▼       ▼    ▼
                  [e][e][e][e]  [e][e]  [e]      ← 4, 2, 1 phần tử
                                        (hình theo Figure 4.12)
```

**Hợp đồng hash ↔ equals** (tr. 112–113) — hỏi rất hay bị hỏi:
- Hai key **bằng nhau** ⇒ **bắt buộc** hash giống nhau.
- Hai key hash giống nhau **không bắt buộc** phải bằng nhau (đó là collision, hợp lệ).

Hàm hash **hợp lệ nhưng tệ hại** sách đưa ra:
```cpp
auto my_hash = [](const Person& person) { return 47; };   // hợp lệ! nhưng mọi thứ vào 1 bucket
```
⇒ tìm kiếm tụt về **O(n)**.

**Viết hash cho kiểu tự định nghĩa** (tr. 113):
```cpp
auto person_eq = [](const Person& lhs, const Person& rhs) {
    return lhs.name() == rhs.name() && lhs.age() == rhs.age();
};
auto person_hash = [](const Person& person) {
    auto seed = size_t{0};
    boost::hash_combine(seed, person.name());   // chuẩn C++ CHƯA có hàm trộn hash
    boost::hash_combine(seed, person.age());
    return seed;
};
using Set = std::unordered_set<Person, decltype(person_hash), decltype(person_eq)>;
auto persons = Set{100, person_hash, person_eq};
```

**Luật vàng của sách:** *dùng ĐÚNG các member đã dùng trong hàm equals để tính hash.*
- Dùng **thiếu** (chỉ `name`) ⇒ đúng nhưng **kém**: mọi người trùng tên vào chung bucket.
- Dùng **thừa** (thêm member không có trong equals) ⇒ **thảm hoạ**: hai object *bằng nhau* lại ra hash khác nhau ⇒ **không tìm thấy phần tử vốn có trong set**.

**Load factor** (tr. 114): số phần tử trung bình mỗi bucket. Chạm `max_load_factor` ⇒ tăng số bucket và **rehash toàn bộ**. Ép thủ công bằng `rehash()`/`reserve()`.

**Dùng đúng API** (tr. 121–122) — mục nhỏ nhưng đáng nhớ:

```cpp
if (bag.count(word) > 0)               // đếm HẾT các phần tử khớp — thừa việc
if (bag.find(word) != bag.end())       // vẫn trả về phần tử
if (bag.lower_bound(word) != bag.end())// cách khuyến nghị TRƯỚC C++20
if (bag.contains(word))                // ✅ C++20 — rõ ý và hiệu quả nhất
```

Tương tự: **luôn dùng `empty()` chứ đừng `size() == 0`** — với `std::list`, `size()` có thể là **O(n)** còn `empty()` luôn O(1).

> ⚠️ `contains()` là **C++20**. Với C++17 dùng `find() != end()`.

**Priority queue để partial sort** (tr. 115–116). Bài toán: n kết quả tìm kiếm, chỉ cần **m cái điểm cao nhất**, và **chỉ có forward iterator** (không random access ⇒ không dùng được `std::sort`/`partial_sort`).

```cpp
template<typename It>
auto sort_hits(It begin, It end, size_t m) -> std::vector<Hit> {
    // so sánh NGƯỢC: giữ phần tử điểm THẤP NHẤT ở đỉnh
    auto cmp = [](const Hit& a, const Hit& b) { return a.rank_ > b.rank_; };
    auto queue = std::priority_queue<Hit, std::vector<Hit>, decltype(cmp)>{cmp};

    for (auto it = begin; it != end; ++it) {
        if (queue.size() < m) {
            queue.push(*it);
        } else if (it->rank_ > queue.top().rank_) {   // hơn cả thằng kém nhất đang giữ
            queue.pop();
            queue.push(*it);
        }
    }
    auto result = std::vector<Hit>{};
    while (!queue.empty()) { result.push_back(queue.top()); queue.pop(); }
    std::reverse(result.begin(), result.end());
    return result;
}
```

**Chi phí: bộ nhớ O(m), thời gian O(n · log m)** — so với sắp xếp toàn bộ là O(n log n) bộ nhớ O(n). Khi **m ≪ n** thì khác biệt rất lớn.

> 🆕 Đây đúng là mẫu *"top-K trong luồng dữ liệu"* — hợp cả với embedded: giữ **m mẫu cực trị** từ luồng sensor mà **không cần đệm toàn bộ luồng**.

### Góc interview

**Q4. Bạn dùng `unordered_set` với kiểu tự định nghĩa. `insert` chạy tốt nhưng `find` không bao giờ tìm thấy. Vì sao?**
<details><summary>Đáp án</summary>

**Gần như chắc chắn: hàm hash và hàm equals KHÔNG NHẤT QUÁN.**

Hợp đồng bắt buộc: **hai key bằng nhau (theo `equals`) thì PHẢI có cùng hash**. Chiều ngược lại không bắt buộc.

**Lỗi kinh điển:** hash dùng **nhiều member hơn** equals.
```cpp
// equals: chỉ so id
auto eq   = [](const P& a, const P& b) { return a.id == b.id; };
// hash: trộn cả id LẪN timestamp   ← ❌
auto hash = [](const P& p) { return h(p.id) ^ h(p.timestamp); };
```
Hai object cùng `id` nhưng khác `timestamp` là **bằng nhau** theo equals, nhưng ra **hash khác nhau** ⇒ rơi vào **bucket khác** ⇒ `find` tìm ở bucket sai, **không thấy** phần tử vốn đang nằm trong set.

**Cách tìm ra:** in `hash(a)`, `hash(b)`, `eq(a,b)` với hai object mà bạn cho là bằng nhau. Thấy `eq == true` nhưng hash khác ⇒ đúng bệnh.

**Luật vàng:** *dùng đúng tập member trong equals để tính hash — không thừa, không thiếu.*
- **Thừa** ⇒ sai chức năng (bug này).
- **Thiếu** ⇒ vẫn đúng nhưng chậm: nhiều phần tử dồn một bucket, O(1) tụt về O(n).

**Bẫy phụ thường gặp cùng lúc:** hash/equals **không phải `const`**, hoặc key **bị sửa sau khi đã chèn** — sửa key đang nằm trong container là UB cho cả `set` lẫn `unordered_set`.

**Chi tiết bonus:** C++ chuẩn **chưa có** hàm trộn hash; dùng `boost::hash_combine` (một dòng, copy được từ tài liệu Boost).

Liên hệ bank [DSA-002](../../14-prep/mock-interview/bank/dsa.md).
</details>

---

## Cụm 4 — View: `string_view` và `span` (tr. 117–120)

### Nội dung chính

**Định nghĩa chung:** view **không phải container**. Chúng là **con trỏ + kích thước**, **không sở hữu** bộ nhớ, **không cấp phát**, và **không bảo đảm gì về vòng đời** vùng nhớ chúng trỏ tới. Thiết kế để **truyền theo giá trị**.

#### `std::string_view` (C++17)

```
std::string:      [ptr][size][cap] ──► "Hello beautiful world\0"  (sở hữu heap)
                                             ▲        ▲
std::string_view: [ptr]───────────────────────┘        │
                  [size]───────────────────────────────┘
                                        (hình theo Figure 4.13)
```

**Hai lợi ích đo được:**

**① `substr` từ O(n) xuống O(1).** `std::string::substr` phải **cấp phát chuỗi mới** (vì `std::string` cần `'\0'` cuối để `c_str()` hoạt động). `string_view::substr` chỉ **dịch con trỏ và đổi size** ⇒ **hằng số**.

**② Không tạo `std::string` tạm khi truyền literal:**
```cpp
auto some_func(const std::string& s) { }
some_func("A string literal");     // ❌ compiler PHẢI dựng một std::string tạm (có thể cấp phát)

auto some_func(std::string_view s) { }   // truyền theo GIÁ TRỊ
some_func("A string literal");     // ✅ không dựng gì cả
```

> ⚠️ `string_view` **không yêu cầu null-terminated**. Nên **không được** truyền `sv.data()` cho hàm C mong đợi chuỗi kết thúc `'\0'`.

#### `std::span` (C++20) — chữa array decay

```cpp
auto f1(float buffer[]) {
    const auto n = std::size(buffer);   // ❌ không biên dịch — size đã mất
}
auto f2(float buffer[], size_t n) { }   // 😐 chạy được nhưng dễ sai
float a[256];
f2(a, 256);
f2(a, sizeof(a)/sizeof(a[0]));          // mẫu cũ, dài dòng
f2(a, std::size(a));

auto f3(std::span<float> buffer) {      // ✅ con trỏ + size trong MỘT object
    for (auto&& b : buffer) { }         // dùng được range-based for
}
float a[256];  f3(a);                   // ✅ mảng C
auto v = std::vector{1.f,2.f,3.f};  f3(v);  // ✅ vector — CÙNG một hàm
```

**`string_view` vs `span`:**

| | `std::string_view` | `std::span` |
|---|---|---|
| Chuẩn | **C++17** | **C++20** |
| Bộ nhớ trỏ tới | **bất biến** (const) | **sửa được** |
| Hàm riêng | `hash()`, `substr()` | — |
| So sánh `==`, `<` | ✅ | ❌ **không có `compare()`** |

> 🎯 **Với JD C++17:** `string_view` dùng được ngay. `span` thì chưa — 🆕 thay bằng `gsl::span`, hoặc tự viết struct `{T* data; size_t size;}`, hoặc viết hàm template trên cặp iterator.

### Góc interview

**Q5. `std::string_view` lợi gì? Bẫy nguy hiểm nhất của nó là gì?**
<details><summary>Đáp án</summary>

**Lợi — hai thứ:**

**① `substr` O(1) thay vì O(n).** `std::string::substr` phải cấp phát chuỗi mới vì `std::string` luôn giữ `'\0'` ở cuối để `c_str()` chạy được. `string_view` không cần null terminator ⇒ `substr` chỉ dịch con trỏ + đổi size.

**② Không sinh `std::string` tạm** khi truyền string literal hay chuỗi con vào hàm. Dùng làm **kiểu tham số** thì cả `std::string`, string literal, và buffer đều truyền vào được **không cấp phát**.

**Bẫy nguy hiểm nhất — DANGLING.** `string_view` **không sở hữu** gì. Nó sống lâu hơn chuỗi gốc thì thành con trỏ treo:

```cpp
std::string_view sv = std::string{"tạm"};      // ❌ string tạm chết ngay cuối câu lệnh
auto f() { std::string s = load(); return std::string_view{s}; }   // ❌ s chết khi return
```

Nguy hiểm vì **không warning, không crash ngay** — nó đọc bộ nhớ đã giải phóng và thường vẫn ra dữ liệu cũ đúng, cho tới khi vùng đó bị dùng lại.

**Bẫy thứ hai — không null-terminated.** Truyền `sv.data()` cho hàm C (`fopen`, `strlen`, `printf("%s")`) là **đọc tràn** cho tới khi gặp `'\0'` ngẫu nhiên nào đó.

**Quy tắc thực hành:** `string_view` hợp làm **tham số hàm** (sống trong phạm vi lời gọi), **không** hợp làm **data member** hay **giá trị trả về** — trừ khi bạn kiểm soát được vòng đời nguồn.

Liên hệ bank [CPP-037](../../14-prep/mock-interview/bank/cpp.md).
</details>

---

## Cụm 5 — Đánh đổi độ phức tạp vs overhead (tr. 120–122)

### Nội dung chính

**Bảo đảm độ phức tạp chỉ có ý nghĩa khi n đủ lớn.** Mỗi container mang một **chi phí cố định** (overhead) ảnh hưởng mạnh hơn với dữ liệu nhỏ.

```
thời gian
   ▲
   │        O(log n) — overhead lớn
   │      ╱
   │    ╱ ─────────────────────────  O(n)
   │  ╱ ╱
   │╱ ╱     ← dưới ngưỡng này, O(n) NHANH HƠN O(log n)
   └────────┬─────────────────────────► n
         ngưỡng            (hình theo Figure 4.14)
```

**Và cache đẩy ngưỡng đó đi xa hơn nữa** — điều này *"thường nói lợi cho `std::vector`"*: overhead bộ nhớ thấp + liền mạch ⇒ truy cập và duyệt nhanh hơn.

> **Kết luận thực hành:** *"Bạn phải tự quyết định 'đủ lớn' nghĩa là gì trong ca của mình — và một lần nữa, phải ĐO chương trình khi nó đang chạy."*

🆕 **Ứng dụng thẳng cho embedded:** với **vài chục phần tử** — quy mô rất thường gặp trong firmware (danh sách thiết bị, hàng đợi lệnh, bảng cấu hình) — **`std::vector` + tìm tuyến tính thường thắng `std::map`**, dù lý thuyết nói O(n) thua O(log n). Lý do: `map` cấp phát một node mỗi lần chèn, node rải rác, và mỗi bước xuống cây là một cache miss.

---

## Cụm 6 — Parallel arrays / SoA (tr. 123–130) 🎯

### Nội dung chính

#### Bước 0 — chứng minh "object nhỏ thì duyệt nhanh"

```cpp
struct SmallObject { std::array<char, 4>   data_{}; int score_{std::rand()}; };  // sizeof = 8
struct BigObject   { std::array<char, 256> data_{}; int score_{std::rand()}; };  // sizeof = 260

template <class T>
auto sum_scores(const std::vector<T>& objects) {
    ScopedTimer t{"sum_scores"};          // xem measuring-performance.md
    auto sum = 0;
    for (const auto& obj : objects) sum += obj.score_;
    return sum;
}
auto small_objects = std::vector<SmallObject>(1'000'000);
auto big_objects   = std::vector<BigObject>(1'000'000);
```

| | `sizeof` | Thời gian |
|---|---|---|
| `SmallObject` | 8 byte | **~1 ms** |
| `BigObject` | 260 byte | **~10 ms** |

**Cùng phép cộng, cùng số phần tử, chênh 10 lần.** Vì với object 8 byte, một cache line 64 byte chứa **8 object**; với object 260 byte, mỗi object cần **hơn 4 cache line** — mà ta chỉ dùng 4 byte `score_` trong đó.

#### Bước 1 — tách trường ít dùng ra sau con trỏ

```cpp
struct User {                       // ❌ sizeof = 128 byte
    std::string name_;
    std::string username_;
    std::string password_;          // 4 trường xác thực — RẤT ÍT khi dùng
    std::string security_question_;
    std::string security_answer_;
    short level_{};
    bool  is_playing_{};
};
```

Hai hàm chạy rất thường xuyên, đều duyệt toàn bộ vector 1 triệu user:
```cpp
auto num_users_at_level(const std::vector<User>& users, short level) {
    auto num_users = 0;
    for (const auto& user : users) if (user.level_ == level) ++num_users;
    return num_users;
}
auto num_playing_users(const std::vector<User>& users) {
    return std::count_if(users.begin(), users.end(),
                         [](const auto& user) { return user.is_playing_; });
}
```

Tách phần xác thực ra:
```cpp
struct AuthInfo {
    std::string username_, password_, security_question_, security_answer_;
};
struct User {                            // ✅ sizeof = 40 byte (từ 128)
    std::string name_;
    std::unique_ptr<AuthInfo> auth_info_;   // chỉ còn một con trỏ
    short level_{};
    bool  is_playing_{};
};
```

> **Lưu ý của sách:** *"thay đổi này cũng hợp lý về mặt thiết kế"* — tách dữ liệu xác thực ra làm **tăng tính cohesion** của `User`. **Tổng bộ nhớ không giảm**, nhưng cái ta cần là **thu nhỏ thứ đang được duyệt**.

#### Bước 2 — parallel arrays (Structure of Arrays)

Thay vì một mảng object, dùng **nhiều mảng song song**, phần tử **cùng index** thuộc về cùng một user:

```
AoS (Array of Structures) — một mảng:
  [name|ptr|lvl|play][name|ptr|lvl|play][name|ptr|lvl|play] ...   40 byte/phần tử
   ↑ đọc lvl phải kéo về cả 40 byte

SoA (Structure of Arrays) — ba mảng:
  users:         [name|ptr][name|ptr][name|ptr] ...
  levels:        [lvl][lvl][lvl][lvl][lvl][lvl] ...    2 byte/phần tử
  playing_users: [b][b][b][b][b][b][b][b] ...          1 BIT/phần tử (!)
                                        (hình theo Figure 4.17–4.18)
```

```cpp
auto num_users_at_level(const std::vector<int>& levels, short level) {
    return std::count(levels.begin(), levels.end(), level);      // chỉ chạm mảng levels
}
auto num_playing_users(const std::vector<bool>& playing) {
    return std::count(playing.begin(), playing.end(), true);
}
```

#### Kết quả đo — bảng tổng hợp

| Phiên bản | `num_users_at_level` | `num_playing_users` |
|---|---|---|
| `User` **128 byte** | 11 ms | 10 ms |
| `User` **40 byte** (tách auth) | **4 ms** | **3 ms** |
| **Parallel arrays** | **0,7 ms** | **0,03 ms** |
| **Tổng cải thiện** | **~16×** | **~330×** |

**Vì sao `num_playing_users` nhanh khủng khiếp (0,03 ms)?** Vì **`std::vector<bool>` KHÔNG phải vector của `bool`** — nó là **bit array**. Mỗi phần tử **1 bit**. `count()` xử lý được **64 bit mỗi lần** (hoặc hơn với SIMD).

> ⚠️ Sách lưu ý: tương lai `std::vector<bool>` **không rõ ràng**, có thể bị deprecate để nhường cho `std::bitset` cỡ cố định và một dynamic bitset mới. Hiện có `boost::dynamic_bitset`.

#### Ba nhược điểm — sách cảnh báo TRƯỚC khi trình bày kỹ thuật

Nguyên văn (tr. 128): *"trước hết, một lời cảnh báo: đây là tối ưu mà trong nhiều trường hợp có **quá nhiều nhược điểm** để là một lựa chọn khả thi. **Đừng coi đây là kỹ thuật tổng quát và áp dụng mà không nghĩ hai lần.**"*

| Nhược điểm | Chi tiết |
|---|---|
| **① Phá vỡ đóng gói** | Trường bị lôi ra khỏi class mà nó thuộc về; dữ liệu đáng lẽ ẩn sau interface thì bị phơi ra |
| **② Quan hệ ngầm, dễ sai** | Phải **tự bảo đảm mọi mảng đồng bộ index**. Xoá một user = phải xoá đúng vị trí ở **mọi** mảng. Không compiler nào kiểm giúp |
| **③ CHẬM HƠN nếu truy cập nhiều trường cùng lúc** | Thuật toán cần đọc `level_` **và** `is_playing_` cùng lúc sẽ **chậm hơn hẳn** so với duyệt một mảng object lớn — vì giờ phải kéo về **hai** dòng cache ở hai vùng bộ nhớ khác nhau |

**Kết luận của sách:** *"Đừng cân nhắc parallel arrays trước khi bạn thật sự gặp vấn đề hiệu năng. Luôn ưu tiên nguyên tắc thiết kế lành mạnh trước, và ưu tiên cách diễn đạt TƯỜNG MINH quan hệ giữa các object hơn là cách ngầm định."*

### Insight đáng nhớ

- **Ba mức tối ưu, ba mức rủi ro** — và sách trình bày theo đúng thứ tự nên áp dụng:
  1. **Sắp lại member để giảm padding** (ch. 7) — 0 rủi ro, luôn nên làm.
  2. **Tách trường ít dùng ra sau con trỏ** — rủi ro thấp, **còn cải thiện thiết kế** (cohesion). 128→40 byte cho **~3×**.
  3. **Parallel arrays** — rủi ro cao, chỉ khi đã đo. Cho thêm **~5×** nữa.
- 🆕 **Nhược điểm ③ là thứ hay bị bỏ qua nhất.** SoA thắng khi thuật toán **quét một trường trên nhiều phần tử**; nó **thua** khi thuật toán **đọc nhiều trường của một phần tử**. Đây chính là lý do các game engine dùng SoA cho hệ thống ECS (mỗi system chỉ quan tâm vài component) mà không dùng cho mọi thứ.

### Góc interview

**Q6. Bạn có `std::vector<Sensor>` một triệu phần tử, `Sensor` 128 byte. Có một hàm chạy 100 lần/giây chỉ đọc trường `status` (1 byte) của mọi sensor. Tối ưu thế nào?**
<details><summary>Đáp án</summary>

**Chẩn đoán:** ta đang kéo về **128 byte** để dùng **1 byte**. Với cache line 64 byte, mỗi `Sensor` chiếm **2 cache line** mà chỉ 1 byte hữu ích ⇒ **lãng phí ~99% băng thông bộ nhớ**.

**Ba mức tối ưu, làm theo thứ tự rủi ro tăng dần:**

**① Sắp lại member để giảm padding** *(rủi ro 0)*. Đặt member lớn nhất lên đầu. Có thể giảm `sizeof` vài chục % mà không đổi gì khác. Xem [memory-management.md](memory-management.md) cụm 3.

**② Tách trường ít dùng ra sau con trỏ** *(rủi ro thấp, còn cải thiện thiết kế)*:
```cpp
struct SensorDetails { std::string name, location, calibration_data; };
struct Sensor {
    std::unique_ptr<SensorDetails> details_;   // ít dùng ⇒ đẩy ra ngoài
    uint32_t id_;
    uint8_t  status_;
};
```
Sách đo mức này: **128 → 40 byte**, thời gian **11 ms → 4 ms** (~3×). Bonus: cohesion tốt hơn.

**③ Parallel arrays (SoA)** *(rủi ro cao — chỉ khi đã đo và vẫn chưa đủ)*:
```cpp
std::vector<Sensor>  sensors;      // dữ liệu đầy đủ
std::vector<uint8_t> statuses;     // mảng SONG SONG, cùng index
// quét status giờ chỉ chạm 1 MB thay vì 128 MB
```
Sách đo: **4 ms → 0,7 ms**. Nếu `status` là boolean thì `std::vector<bool>` (bit array) cho **0,03 ms** vì `count()` xử lý 64 bit mỗi lần.

**Phải nêu được ba đánh đổi của ③ mới đạt điểm cao:**
1. **Phá vỡ đóng gói** — trường bị lôi khỏi class của nó.
2. **Đồng bộ index thủ công** — xoá một sensor phải xoá đúng vị trí ở **mọi** mảng, không compiler nào kiểm.
3. **Chậm hơn nếu về sau cần đọc nhiều trường cùng lúc** — lúc đó phải kéo về hai vùng bộ nhớ tách rời.

**Câu chốt:** *"Bắt đầu bằng đo, rồi làm ① vì nó miễn phí, rồi ② vì nó còn tốt cho thiết kế. Chỉ làm ③ khi đã đo và vẫn chưa đủ — nó đánh đổi tính đúng đắn dài hạn lấy tốc độ."*
</details>

**Q7. `std::vector<bool>` có gì đặc biệt? Vì sao nó vừa là ưu điểm vừa là bẫy?**
<details><summary>Đáp án</summary>

**Nó KHÔNG phải `std::vector` của `bool`.** Chuẩn đặc biệt hoá nó thành **bit array**: mỗi phần tử chiếm **1 bit** thay vì 1 byte.

**Ưu điểm — đo được:**
- **Tiết kiệm 8× bộ nhớ.** Một triệu bool = 125 KB thay vì 1 MB ⇒ **lọt vào L2 cache**.
- **Thao tác theo khối:** `count()`, `find()` xử lý **64 bit mỗi lần** (hoặc hơn với thanh ghi SIMD). Sách đo `std::count` trên một triệu phần tử: **0,03 ms** — nhanh hơn ~330 lần so với duyệt vector object 128 byte.

**Bẫy — nghiêm trọng:**
1. **`operator[]` không trả về `bool&`** mà trả về một **proxy object**. Nên `auto b = v[0];` cho ra proxy, **không phải `bool`** — sửa `b` là sửa vector.
2. **Không lấy được địa chỉ phần tử:** `&v[0]` không cho `bool*`. Không `memcpy` được, không truyền cho API C được.
3. **Phá vỡ hợp đồng của container:** nó không thoả mãn yêu cầu `Container` của chuẩn, nên một số thuật toán/template generic gãy khi gặp nó.
4. **Không thread-safe theo phần tử** — hai thread ghi hai *phần tử* khác nhau vẫn có thể ghi **cùng một byte** ⇒ **data race thật**, khác hoàn toàn với `vector<char>`.

**Kết luận thực hành:** dùng khi bạn **thật sự muốn bit array** (tập bit lớn, đếm/tìm nhiều). Muốn một mảng boolean bình thường thì dùng **`std::vector<char>`** hoặc `std::deque<bool>`; muốn cỡ cố định thì **`std::bitset`**.

Sách lưu ý: tương lai `std::vector<bool>` không rõ ràng, có thể bị deprecate để nhường cho `std::bitset` + một dynamic bitset mới (`boost::dynamic_bitset` đã có).
</details>

---

## Đọc thêm (tuỳ chọn — nội dung trên đã tự chứa)

- [12-dsa/complexity-and-structures](../../12-dsa/complexity-and-structures.md) — Big-O, bảng tra container (chương này là phần *"và hằng số thì sao"*)
- [memory-management.md](memory-management.md) — cùng cuốn: padding, alignment, arena, PMR
- [measuring-performance.md](measuring-performance.md) — cùng cuốn: `ScopedTimer` dùng trong mọi phép đo ở trên
- [12-dsa/ring-buffer](../../12-dsa/ring-buffer.md) — cache line & false sharing ở góc cấu trúc dữ liệu
