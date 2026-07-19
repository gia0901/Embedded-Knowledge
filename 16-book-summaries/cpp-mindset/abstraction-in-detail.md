# C++ Mindset — Ch. 2: Abstraction in Detail (tr. 37–60) ✅ đã đọc sâu toàn văn

> Thuộc [The C++ Programmer's Mindset](README.md). Nguồn: đọc trực tiếp PDF, trang sách = trang PDF − 25.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng trong sách** · trích dẫn kèm `(tr. X)`.
> Ý đồ chương (tr. 37): chương 1 nói abstraction **trong bài toán**; chương này nối sang **cơ chế abstraction của C++** — 4 phương tiện: standard algorithms, functions, classes, templates — và chiều ngược lại: *"these mechanisms can also teach us how to think about the problem"*.

---

## Cụm 1 — Bốn loại bài toán & bài học từ IO (tr. 38–42)

### Nội dung chính

**Vì sao phải phân loại (tr. 38):** *"All problems can be broken down into a set of basic problems via a sequence of reductions"* — các bài cơ bản là thứ đã biết cách giải (cấu trúc dữ liệu/thuật toán kinh điển); càng kinh nghiệm càng cần ít bước reduce. Bốn domain (chồng lấn nhau):

| Loại (tr. 38–40) | Nội dung | Ví dụ sách nêu |
|---|---|---|
| **Combinatorial** | đếm, sort/sắp xếp lại, search, ghép phần tử, đồ thị | tìm substring chung, regex, route finding trong không gian 2D/3D né chướng ngại (**A\***) |
| **Input-output (IO)** | tìm + nạp dữ liệu vào address space và ngược lại | file trên đĩa, dữ liệu qua mạng, **sensor phát liên tục** ("ephemeral" — khó hơn); cả **chuyển dữ liệu sang GPU** |
| **Numerical** | tính toán trực tiếp trên dữ liệu | mã hóa/giải mã, tối ưu, thống kê/suy luận — khó nhận ra "in the wild": *"It takes some thinking to turn a recommendation problem into a problem of linear algebra"* (tr. 39) |
| **Interface** | người/chương trình khác tương tác với lời giải thế nào | CLI? API? website? — *"if nobody can interact with your solution to the problem, then it doesn't really exist"* (tr. 40) |

**Đường abstraction đặc trưng từng loại (tr. 40)** — đây là "bản đồ chỉ hướng" của chương: bài categorical → abstraction nằm ở **dữ liệu + phép toán** (*"the algorithm for sorting is identical, regardless of whether the ordering is done by less-than or greater-than"*); bài IO → ở **interface với OS** + format dữ liệu; bài numerical → ở **biến đổi dữ liệu/phương pháp** (ví dụ đắt: *"large language models operate on integer tokens and not strings containing words or letters"*); bài interface → chính **hàm/class của giao diện** là abstraction, giấu cài đặt.

**Nối vào C++ (tr. 40–41):** bước một luôn là nhận diện loại bài trong bài toán của mình (*"at least interfaces will be involved, and IO is also likely"* — và nhận diện loại chính là một cách bắt đầu decompose). C++ cho: hàm (đóng gói chuỗi thao tác), template/concepts (viết một lần cho nhiều kiểu — *"Concepts are a great way to think about data and functionality. Each time you see a new problem, try to understand, from a conceptual point of view, what the requirements are"*, tr. 41), và standard library như **bộ sưu tập abstraction chuẩn** (filesystem, container, toán, text/regex, algorithms).

**IO với C++ và bài học xếp tầng (tr. 41–42):** stream = object cho đọc/ghi byte có/không cấu trúc; `ifstream` cho file, `cin` cho terminal:

```cpp
double value;
std::cin >> value;   // giả định chuỗi byte hiện tại là một double dạng text
                     // sai (vd gặp 'a') → set error state: failbit hoặc exception
```

Từ đó sách định nghĩa **serialization** (tr. 42): *"taking a value and producing a representation that can be stored, independent of the internal state of the program, can be loaded later, and 'exactly' recover the value"* — JSON/XML/Protocol Buffers đều là format serialize object phức tạp; và điểm chốt: *"most serialization libraries are built on top of this [stream] interface"* — minh họa cho nguyên lý *"stacking relatively simple abstractions can build very powerful tools... a concept that will appear many times within this book"* (tr. 42).

### Insight đáng nhớ

- Bảng 4 loại (tr. 38–40) là **công cụ decompose thực dụng**: câu đầu tiên khi nhận bài mới — "bài này gồm những mảnh thuộc loại nào?" — vì mỗi loại chỉ sẵn hướng abstraction riêng (tr. 40). 🆕 Áp vào nghề BSP: một driver hầu như luôn = IO (đọc thanh ghi/DMA) + interface (sysfs/ioctl lên userspace) + đôi khi numerical (hiệu chuẩn sensor) — nhận diện xong là thấy ngay chỗ cần thiết kế kỹ.
- Chuỗi stream → operator>> → serialization lib (tr. 41–42) được sách dùng làm **bằng chứng sống cho "xếp tầng abstraction đơn giản thành công cụ mạnh"** — cùng khuôn với những gì đã thấy ở [OSTEP](../ostep/README.md): FS xếp trên block layer xếp trên driver.

---

## Cụm 2 — Standard algorithms: sức mạnh nằm ở cách đóng khung bài toán (tr. 42–45)

### Nội dung chính

**Header `<algorithm>` là gì (tr. 42):** không phải "các thuật toán" mà là *"implementations of common (families of) algorithms for solving common abstract problems"* — sức mạnh đến từ template hóa **mọi mặt** của thao tác: predicate, comparison/ordering, indirection, projection. Bảy nhóm chính (tr. 43): **search** (tìm phần tử thỏa điều kiện) · **copying** (copy/move dữ liệu) · **transformation** (biến range này thành range khác) · **permutations** (đổi thứ tự) · **sorting & partitioning** · **binary searching** (tìm nhanh nhờ ordering) · **generating** (điền range).

Nhưng: *"Sometimes it can appear as if none of these functions are appropriate, until you frame the problem (via abstraction) in the correct way"* (tr. 43) — và sách chứng minh bằng ví dụ xương sống của chương:

**Bài toán grid signal (tr. 43–45):** tìm tín hiệu dương **gần nhất** với một vị trí cho trước trong lưới 5×5 (observer ở giữa; tín hiệu = điểm có `compute_signal_intensity(x,y) > detection_intensity`).

**Cách 1 — brute force (tr. 43–44):** quét cả lưới gom mọi tín hiệu dương, rồi `min_element` với ordering tự chế:

```cpp
struct Pos { int x; int y; };                    // abstraction tối giản cho vị trí lưới

std::vector<Pos> signals;
signals.reserve(dim_x * dim_y);
for (int y = 0; y < dim_y; ++y) {
    for (int x = 0; x < dim_x; ++x) {
        if (compute_signal_intensity(x, y) > detection_intensity) {
            signals.emplace_back(x, y);
        }
    }
}

Pos start {2, 2};                                // giữa lưới
auto dist_to_start = [&start](const Pos& pos) {  // metric Chebyshev — "will work nicely"
    return std::max(std::abs(pos.x - start.x), std::abs(pos.y - start.y));
};
auto ordering = [&dist_to_start](const Pos& a, const Pos& b) {
    return dist_to_start(a) < dist_to_start(b);
};
auto closest_pos = std::min_element(signals, ordering);
```

Phê bình của sách (tr. 44): *"we're not making use of any explicit abstraction, which leads to a functional but not efficient solution. **The crucial information that we are forgetting is that the search is not global** — we don't care about signals that appear far away... unless there are none closer."*

**Cách 2 — đổi khung: tìm-đầu-tiên trên thứ tự duyệt đúng (tr. 45):** nếu có một range duyệt lưới **loang từ tâm ra ngoài** (đặt tên `ExpandingSearchRange`) thì bài toán "tìm gần nhất" sụp về `std::find` — thuật toán *dừng ngay khi gặp kết quả đầu tiên*:

```cpp
auto predicate = [detection_intensity](int x, int y) {
    return compute_signal_intensity(x, y) > detection_intensity;
};
ExpandingSearchRange range(pos_x, pos_y);        // duyệt vòng tròn loang từ tâm
auto closest_pos = std::ranges::find(range, predicate);
```

Hai lợi ích sách nêu (tr. 45): số lần gọi `compute_signal_intensity` *"dramatically smaller"* so với dim_x×dim_y bảo đảm của cách 1; và **linh hoạt**: *"should our objectives change or if additional constraints are imposed, we can simply swap ExpandingSearchRange with a modified version"*. Sách **cố ý không cài** `ExpandingSearchRange` — *"you should think about how this might be implemented"*.

### Insight đáng nhớ

- Cặp cách 1 → cách 2 (tr. 43–45) là minh chứng cụ thể nhất sách cho luận điểm *"frame the problem in the correct way"*: **thuật toán không đổi độ thông minh — thứ thay đổi là RANGE được duyệt**. Chuyển tri thức bài toán ("gần trước, xa sau") từ *bước hậu xử lý* (min_element) vào *thứ tự duyệt* (custom range) biến O(N²)-luôn-luôn thành dừng-sớm. 🆕 Đây là mẫu tư duy tổng quát đáng giữ: khi thấy "quét hết rồi lọc", hỏi ngay "có duyệt theo thứ tự nào để lấy-cái-đầu-tiên không?" — cùng họ với priority queue thay sort, early-exit thay full scan.
- Danh sách 7 nhóm (tr. 43) đáng dán trước mặt khi giải bài: bước "bài này thuộc nhóm search/transform/partition nào?" thường chỉ thẳng hàm chuẩn cần dùng.

---

## Cụm 3 — Functions: đơn vị đóng gói rẻ nhất (tr. 45–51)

### Nội dung chính

**Pure function (tr. 45–46):** hàm *"independent of the global program state; only the input data has any effect on the outputs"* — gốc toán học: quan hệ mỗi cấu hình input ứng đúng một output. Sách lưu ý các chỗ C++ **lệch khỏi** hình mẫu toán: chỉ trả được một giá trị (khắc phục: `std::pair/std::tuple`, hoặc kiểu C ghi qua con trỏ out-param); và hàm C++ **có thể fail giữa chừng** — *"In mathematics, the domain of a function can be limited by any number of constraints, whereas C++ can only limit function arguments by type; checking values must be done at runtime"* (tr. 46). Hàm cũng là phương tiện giấu cài đặt **rẻ nhất** (*"very low-cost (especially if inlined)"*) cho các mảnh nhỏ: hàm khoảng cách, ordering, predicate.

**Interface bằng hàm (tr. 46–48):** lợi thế quyết định — *"a simple concept that transfers well across boundaries. For instance, C++ functions can be made to use C calling conventions, making them usable from other languages"* (tr. 46). Khi cần cấu hình phức tạp: cặp **hàm + configuration object có default hợp lý**:

```cpp
struct SummaryStatistics;            // định nghĩa lược — không quan trọng ở đây

class Configuration {
    bool b_include_mean = true;      // default hợp lý — người dùng cơ bản
    bool b_include_std  = true;      //   không phải cấu hình gì cả
public:
    bool include_mean() const noexcept { return b_include_mean; }
    void include_mean(bool setting) noexcept { b_include_mean = setting; }
    // ... các setting khác
};

std::vector<SummaryStatistics>
compute_statistics(const Configuration& config,
                   std::span<const std::string> sources);   // nguồn dữ liệu dạng URI
```

⚠️ Cảnh báo ABI của chính sách (tr. 47): Configuration *"is entirely inline, but it is still part of the interface... if this class changes (by adding new settings...), the function would have to be recompiled and would likely break backwards compatibility"* — dẫn tới lời khuyên *"making your programming interface as minimal as possible"* (tr. 48); class-based interface lộ thêm chi tiết cài đặt (cả lo ngại IP). 🆕 Đây chính là bài toán trung tâm của [07/abi-versioning.md](../../07-shared-libraries/abi-versioning.md) và lý do Pimpl tồn tại ([EMC++ Item 22](../effective-modern-cpp.md)) — sách chạm đúng vấn đề nghề của bạn.

**Functions as building blocks (tr. 48–50):** giải phẫu bài combinatorial/numerical: *"At the outer level, there is typically some kind of **driving operation** that performs an iteration over the problem domain. Inside this driver is a **computation aspect** and a **decision aspect**"* (sort: computation = so sánh cặp, decision = có swap không). Ví dụ: tìm nghiệm f(t)=0 bằng **chia đôi (bisection)** — cần 3 thứ: hàm liên tục, một điểm f dương, một điểm f âm:

```cpp
template <typename Function, typename Real>
Real find_root_bisect(Function&& function, Real pos, Real neg, Real tol) {
    auto fpos = function(pos);

    while (compare_reals_equal(pos, neg)) {      // driving loop
        auto m  = midpoint(pos, neg);            // building block 2
        auto fm = function(m);

        if (std::abs(fm) < tol) { return m; }    // đã (gần) bằng 0 → xong sớm

        if (std::signbit(fm) == std::signbit(fpos)) {  // DECISION: thu hẹp về phía nào
            pos = m; fpos = fm;
        } else {
            neg = m;
        }
    }
    return fpos;
}
```

Hai building block (`compare_reals_equal` — double chỉ ~15 chữ số thập phân chính xác nên "bằng nhau" phải có định nghĩa; `midpoint`) được tách hàm **có chủ đích** (tr. 49): *"Keeping these as functions allows us to replace them more easily later (abstracting the algorithm)"* — thuật toán họ hàng chỉ khác cách chọn điểm kế tiếp; và giữ generic theo `Real` (kiểu không có `operator+` vẫn dùng được nếu cung cấp midpoint riêng). Phân tích requirement (tr. 49–50): tính **liên tục không kiểm tra được trong code** — vi phạm thì *"garbage in, garbage out... quite typical of numerical algorithms"*; các điều kiện khác (một điểm dương, một điểm âm) check được nhưng sách lược cho gọn.

**Function-like objects (tr. 50–51):** class có `operator()` — gọi như hàm, nhưng *"interact better with the template mechanism. (Function pointers cannot be meaningfully default-constructed, but function-like objects can.)"* — lý do `std::less`, `std::hash` làm **default template parameter** cho `std::map`/`unordered_map`. **Lambda = đường cú pháp**: *"the compiler turns [it] into a class definition during compilation. Captured variables are just data members"* (🆕 trùng khớp mô hình closure-class đã học ở [EMC++ Cụm 6](../effective-modern-cpp.md)). Giá trị riêng: callable class **mang state** (non-pure) — ví dụ sách, hàm có nhiễu ngẫu nhiên để sinh dữ liệu test:

```cpp
#include <random>
class FunctionWithNoise {
    std::mt19937 m_rng;                          // state: bộ sinh ngẫu nhiên sống qua các lần gọi
    std::normal_distribution<double> m_dist;
public:
    double operator()(double arg) noexcept {
        auto noise = m_dist(m_rng);
        return 2.*arg + 1 + noise;               // xu hướng biết trước + nhiễu
    }
};
// dùng: sinh dữ liệu theo trend đã biết để test pipeline suy luận (tr. 51)
```

Chốt chuyển cụm (tr. 51): *"Functions are very useful, but they are limited by the fact that they cannot usually hold state."*

### Insight đáng nhớ

- Tam giác **driver – computation – decision** (tr. 48) là lăng kính đọc-và-viết thuật toán: nhìn `find_root_bisect` thấy ngay chỗ nào là khung cố định, chỗ nào là điểm mở (hai building block). 🆕 Nó cũng chính là "template method pattern" mà ch. 1 (tr. 17) gán cho STL — giờ thấy ở quy mô một hàm tự viết.
- Cảnh báo *"inline class vẫn là interface — đổi là vỡ tương thích"* (tr. 47) là câu ăn tiền cho dân viết shared library: ranh giới ABI không nằm ở chỗ có export hay không, mà ở **những gì client đã compile vào binary của họ**.

---

## Cụm 4 — Classes: hai vai, và cái giá của polymorphism (tr. 51–55)

### Nội dung chính

**Hai cách dùng class — sách tách bạch ngay đầu (tr. 51):** (1) **structured container giữ bất biến** (invariant) dùng được trong thuật toán qua method — ví dụ `std::vector`; (2) **abstract interface giấu chi tiết** — viết code theo interface, chạy với mọi object hiện thực nó (IO stream của std là ví dụ). Cả hai đều là abstraction, "go about it in (somewhat) different ways".

**Dynamic polymorphism và cái giá (tr. 51):** virtual function = *"pointers to the method implementations are placed in a lookup table that is queried at runtime"* — chi phí nhỏ nhưng có thật. Nguyên tắc đặt chỗ của sách: *"avoid using polymorphic objects in the performance-critical portions of code where the added time to call a virtual function will accumulate quickly. On the other hand, using polymorphic objects on an interface boundary, especially those between a program and the user or with IO, can effectively hide the added cost"* — polymorphism lý tưởng cho *"interacting with external concerns where the latency of the operation itself is the greatest cost"*.

**Vai 1 — class cho raw data (tr. 52–53):** muốn dùng `std::find` thì dữ liệu phải có interface chuẩn (equality). Ví dụ sổ địa chỉ:

```cpp
struct AddressBookRecord {
    size_t id;                        // định danh DUY NHẤT — các field khác thì không
    int house_number;
    std::string street_address;
    std::string city_and_state;
    int zip_code;
};

inline bool operator==(const AddressBookRecord& lhs,
                       const AddressBookRecord& rhs) noexcept {
    return lhs.id == rhs.id;          // equality qua id là ĐỦ và ĐÚNG
}
```

Ordering thì ngược lại — **nhiều lựa chọn hợp lý** (zip_code → city_and_state... kiểu từ điển), mà `operator<` *"can only be implemented once"* → giải pháp: **hàm so sánh có tên** (tr. 53):

```cpp
bool compare_house_number(const AddressBookRecord&, const AddressBookRecord&);
bool compare_zip_code(const AddressBookRecord&, const AddressBookRecord&);
// → truyền vào sort/min_element như ordering tùy bài — "naming these operators
//   will help make the code more readable"
```

Và khi **copy dữ liệu đắt** → view nhẹ (tr. 53): *"use lightweight views that contain a reference, which can be an actual reference (&), a pointer (*), or a selection of views into certain fields (e.g., string_view)"*:

```cpp
class RecordView {
    const AddressBookRecord* p_data;
public:
    size_t id() const noexcept { return p_data->id; }
};
inline bool operator==(const RecordView& lhs, const RecordView& rhs) noexcept {
    return lhs.id() == rhs.id();
}
// đổi kiểu view = đổi hành vi (ordering khác...) mà không đụng dữ liệu gốc
```

**Vai 2 — class cho "physical objects" (tr. 53–55):** hierarchy + interface trừu tượng hợp nhất với **vật thể thật** (cửa sổ desktop, thiết bị lưu trữ, file). Lý lẽ chi phí bằng con số (tr. 54): cửa sổ redraw ở 60Hz → logic có ~16ms; virtual lookup tốn *"at worst, a few microseconds"* — **chi phí thao tác thật nuốt chửng chi phí abstraction**. Ví dụ hoàn chỉnh — giám sát cảm biến nhiệt:

```cpp
class TempSensor {
public:
    virtual ~TempSensor() = default;
    virtual std::string_view id() const noexcept = 0;
    virtual float temperature_kelvin() const noexcept = 0;
    // tên hàm KÈM ĐƠN VỊ: "a reminder to the programmer that, when adding new
    // implementations, they should return Kelvin and not Fahrenheit or Celsius" (tr. 54)
};

void check_sensors(std::span<const TempSensor*> sensors, float threshold) {
    for (const auto& sensor : sensors) {
        auto temp = sensor->temperature_kelvin();
        if (temp > threshold) {
            throw std::runtime_error(
                std::format("Sensor {} reports temperature {}",
                            sensor->id(), temp));
        }
    }
}
```

Tiêu chí thiết kế interface sách rút từ ví dụ (tr. 55): *"Interfaces should generally be **sufficient and minimal**... TempSensor satisfies both conditions; it does not require anything that isn't used or provide anything that isn't strictly necessary"* (để ý: `id()` chỉ được gọi khi vượt ngưỡng — interface không ép tính gì thừa).

### Insight đáng nhớ

- Quy tắc đặt polymorphism của sách (tr. 51, 54) quy về một phép so sánh: **chi phí virtual lookup vs chi phí bản thân thao tác** — 16ms redraw ≫ vài µs lookup → OK; còn trong hot loop mà thao tác chỉ vài ns thì lookup là kẻ chiếm sóng. 🆕 Với embedded: đọc sensor qua I2C tốn hàng trăm µs–ms → interface ảo kiểu `TempSensor` hoàn toàn xứng đáng; nhưng xử lý mẫu ADC trong vòng lặp 5kHz thì không — đúng chỗ cụm 5 (template/static polymorphism) tiếp quản.
- Bộ ba `operator==` theo id + named comparison functions + RecordView (tr. 52–53) là bài mẫu nhỏ mà đủ về **tách "định danh" khỏi "thứ tự" khỏi "cách nhìn dữ liệu"** — ba thứ hay bị nhét chung vào class rồi hối hận. Ví dụ TempSensor thêm chiêu thứ tư: **mã hóa đơn vị vào tên hàm** — quay lại đúng bài học Kelvin của ch. 1 (tr. 11).

---

## Cụm 5 — Templates, concepts & traits: polymorphism tĩnh (tr. 55–59)

### Nội dung chính

**Định vị (tr. 55):** template = tính năng mạnh nhất *"at least until C++26 brings first-class support for reflection"* (⚠️ ghi nhận: sách viết khi C++26 chưa chốt — theo dõi thực tế). Cơ chế nền: *"try first and unwind on failure"* — tên chính thức **SFINAE** (substitution failure is not an error); concepts làm **ngược**: *"requirements should be listed up front and checked before the template is instantiated"*. Và chiều tư duy đảo — đúng tinh thần chương: *"It's quite rare that one starts writing code... by writing template code, but **thinking in terms of templates can sometimes help to find the correct formulation of an abstraction**"*; câu hỏi đúng để hỏi (tr. 56): *"what methods need to exist, and what do I expect them to do?"*

**Concepts cho dữ liệu cơ bản (tr. 56–57):** quay lại `Pos` của cụm 2 — thuật toán chỉ cần "có x, y kiểu int":

```cpp
#include <concepts>
#include <type_traits>

template <typename T>
concept GridPosition = requires(T t) {
    std::is_same_v<decltype(t.x), int>;
    std::is_same_v<decltype(t.x), int>;   // ⚠️ SÁCH IN NHẦM: lặp t.x hai lần —
};                                        //    dòng hai phải là decltype(t.y)
```

Sách tự nhận đây là toy example (int có thể không đủ cho lưới lớn — tr. 57) để minh họa *loại* check khả dĩ. Các concept chuẩn nên biết (tr. 57): `std::totally_ordered` (yêu cầu của sort), `std::predicate` (function-like trả về convertible-to-bool), `std::input_range`/`std::input_iterator`. Tình huống thật sách nêu: dữ liệu là **records từ database** — concept hóa "collection dạng range" + "record có field cần thiết" → *"writing generic code with concepts might make your code easier to maintain later, if you decide to change the database driver"* (tr. 57).

**Traits — cầu nối giữa interface cố định và kiểu bất kỳ (tr. 57–59):** định nghĩa qua ví dụ chuẩn `std::iterator_traits` — *"provides information about an iterator type, abstracting away the actual nature of the iterator itself"*; câu định vị đắt nhất cụm: traits là *"**the compile-time equivalent of abstract interface classes**. They don't incur a runtime-performance cost but instead take longer to compile"* (tr. 57). Quan hệ với concepts: *"you might think of concepts as a subset of traits"* (tr. 58) — concepts chỉ *kiểm tra*, traits còn *mở rộng/thích nghi* hành vi. Hai công dụng: facade nhẹ trên plain data (nhìn `AddressBookRecord` theo nhiều cách **tại compile time**, không copy); và cầu nối interface cố định ↔ kiểu generic.

**Ví dụ hoàn chỉnh — ExactConversionTraits (tr. 58–59):** framework chuyển đổi số **chính xác tuyệt đối** (int32 → double được vì double có 53 bit mantissa; int64 → double thì không; C++ cho static_cast nhưng *"makes no guarantees about exactness"* — không sửa được built-in type → dùng trait):

```cpp
// Primary template: mặc định KHÔNG có chuyển đổi chính xác
template <typename From, typename To, typename = void>
struct ExactConversionTraits {
    using from_ref = const From&;
    using to_ref   = To&;
    static void convert(to_ref to, from_ref from) {
        throw std::runtime_error("invalid exact conversion");
    }
};

// Partial specialization cho cặp số nguyên (constrain bằng concept std::integral;
// pre-C++20 thì dùng enable_if_t + is_integral_v ở tham số template cuối — tr. 59)
template <std::integral From, std::integral To>
struct ExactConversionTraits<From, To> {
    using from_ref = const From&;
    using to_ref   = To&;
    static void convert(to_ref to, from_ref from) {
        if (from <= std::numeric_limits<To>::max
              && from >= std::numeric_limits<To>::min) {   // ⚠️ xem ghi chú lỗi bên dưới
            throw std::runtime_error("invalid exact conversion");
        }
        to = static_cast<To>(from);
    }
};
```

⚠️ **Code sách có lỗi (tr. 59) — phát hiện khi đối chiếu:** (1) điều kiện **ngược logic**: như in, nó *ném exception khi giá trị NẰM TRONG khoảng biểu diễn được* — phải là `if (from > max || from < min) throw`; (2) `numeric_limits<To>::max/min` thiếu `()` — là hàm, phải `max()`/`min()`. Ý định đúng của đoạn văn quanh nó ("chỉ chuyển khi chính xác, không thì ném") xác nhận đây là lỗi in.

Hai điểm thiết kế sách phân tích (tr. 59): **tối ưu tiếp bằng compile-time check** — From=int32/To=int64 thì luôn biểu diễn được → *"the runtime cost of using this trait is zero"* (bỏ hẳn bounds check); và **chữ ký `convert(to_ref, from_ref)`** nhận hai reference thay vì trả về To — *"to accommodate types that might not be easily constructed, such as those that must be hidden behind a pointer. A concrete example... is a GNU multi-precision (GMP) rational number mpq_t that is usually passed as a pointer, since it is implemented in C."*

**Tổng kết chương (tr. 60):** cơ chế abstraction của C++ phục vụ hai mục đích — *"help guide the way we formulate abstractions within the problem itself"* và *"provide the possible routes that we might take"*; template/concepts *"shift work to the compiler, further increasing runtime performance"*. Tài liệu tham khảo sách dẫn: *C++ Templates: The Complete Guide* (Vandevoorde, Josuttis, Gregor).

### Insight đáng nhớ

- Câu *"traits = abstract interface classes của compile time — không tốn runtime, tốn thời gian compile"* (tr. 57) hoàn tất **bảng chọn polymorphism** mà cụm 4 mở ra: virtual (linh hoạt runtime, tốn lookup) ↔ template/traits (khóa lúc compile, zero cost runtime) — tiêu chí chọn là *thời điểm biết kiểu* và *chỗ đứng trong hot path*. 🆕 Bảng này chính là câu trả lời chuẩn cho câu phỏng vấn "static vs dynamic polymorphism" (xem Góc interview).
- Chữ ký `convert(to_ref, from_ref)` cho GMP `mpq_t` (tr. 59) là chi tiết nhỏ dạy bài lớn: **thiết kế generic phải chừa chỗ cho kiểu "khó tính"** (không default-construct được, sống sau con trỏ C) — kiểu suy nghĩ phân biệt thư viện dùng được rộng rãi với thư viện chỉ chạy demo.
- ⚠️ Hai lỗi in trong code (GridPosition lặp `t.x`; điều kiện ngược ở ExactConversionTraits) — nhắc lại thói quen đã ghi ở quy ước: **code trong sách cũng phải đọc phản biện**; summary này đã sửa kèm ghi chú để bạn không chép nhầm.

### Góc interview

**Câu 1 (🎯):** Static polymorphism (template) vs dynamic polymorphism (virtual) — trade-off và tiêu chí chọn? Cho ví dụ cụ thể mỗi bên.

<details><summary>Đáp án</summary>

- **Dynamic (virtual):** một interface, nhiều hiện thực **hoán đổi lúc runtime**; chi phí = vtable lookup mỗi lời gọi + cản inline. Tiêu chí đặt chỗ của sách (tr. 51): tránh ở *"performance-critical portions... where the added time to call a virtual function will accumulate quickly"*; lý tưởng ở **interface boundary/IO** nơi *"the latency of the operation itself is the greatest cost"* — con số minh họa của sách: window redraw 60Hz có budget ~16ms, virtual lookup *"at worst, a few microseconds"* (tr. 54). Ví dụ: `TempSensor` với `temperature_kelvin()` thuần ảo (tr. 54) — mỗi lần đọc sensor vốn đã tốn I/O, lookup không đáng kể; danh sách sensor không thuần nhất chỉ biết lúc runtime.
- **Static (template/concepts/traits):** kiểu chốt **lúc compile** → không lookup, inline được, *"shift work to the compiler"* (tr. 60); traits = *"compile-time equivalent of abstract interface classes... don't incur a runtime-performance cost but instead take longer to compile"* (tr. 57). Giá: mọi kiểu phải biết lúc build, code ở header, compile lâu, một instantiation mỗi kiểu (code size). Ví dụ: `find_root_bisect<Function, Real>` (tr. 48) — hàm f gọi hàng nghìn lần trong loop, virtual ở đây là chi phí tích lũy thật; `ExactConversionTraits` (tr. 59) — chuyển đổi chính xác với **zero runtime cost** khi cặp kiểu an toàn chứng minh được lúc compile.
- Tiêu chí chọn gộp thành hai câu hỏi: **(1) tập kiểu biết lúc compile hay runtime? (2) lời gọi nằm trong hot loop hay sau một thao tác vốn đã đắt?** — compile-time + hot loop → template; runtime + boundary → virtual. 🆕 Embedded cụ thể: sensor qua I2C (trăm µs/lần đọc) → interface ảo thoải mái; xử lý mẫu trong ISR/vòng 5kHz → template/CRTP, cấm virtual.
- Điểm cộng: nêu đường giữa — `std::variant` + `std::visit` (closed set of types, không cần vtable per-object), và CRTP (🆕 — sách chưa nhắc ở chương này).

</details>

**Câu 2:** Vì sao interface public của một shared library nên "function-based và tối giản"? Điều gì vỡ khi thêm field vào một class inline trong header public?

<details><summary>Đáp án</summary>

- Lợi thế hàm (tr. 46): *"a simple concept that transfers well across boundaries"* — ép được về **C calling convention** → gọi được từ mọi ngôn ngữ link C; và giấu trọn cài đặt phía sau (*"once inside the interface function, you're free to use any of the mechanisms at your disposal"*). Class-based interface lộ chi tiết (layout, method inline) — sách nêu cả động cơ IP (tr. 48).
- Thêm field vào class inline trong header public — chuỗi vỡ: client **đã compile** size/offset của class vào binary của họ (constructor inline, truy cập member inline đều nướng layout cũ vào code client) → thư viện mới đổi layout mà client chưa rebuild → đọc/ghi lệch offset, stack corruption — đúng cảnh báo sách (tr. 47): *"if this class changes... would likely break backwards compatibility"*. Đây là **ABI break** dù API (mã nguồn) không đổi.
- 🆕 Nghề shared library (topic [07](../../07-shared-libraries/api-design.md)) xử lý bằng: interface C thuần + opaque pointer/handle, Pimpl ([EMC++ Item 22](../effective-modern-cpp.md)), version symbol, "đừng bao giờ đổi struct đã public — chỉ thêm hàm mới". Quy tắc sách *"sufficient and minimal"* (tr. 55) chính là phòng bệnh từ thiết kế: thứ chưa từng public thì không bao giờ phải giữ tương thích.

</details>

**Câu 3:** Khi nào gói dữ liệu vào class với operator==/orderings, khi nào để struct trần + hàm rời? Dùng ví dụ AddressBookRecord.

<details><summary>Đáp án</summary>

- Nguyên tắc từ ví dụ sách (tr. 52–53): đưa vào class/operator những gì là **bản chất bất biến** của dữ liệu — `operator==` theo `id` vì id là định danh duy nhất (đúng cho *mọi* bài toán dùng record này); còn thứ **tùy bài toán** thì để ngoài — ordering có nhiều lựa chọn hợp lý (zip trước hay tên phố trước?) mà `operator<` *"can only be implemented once"* → **named comparison functions** (`compare_zip_code`...) truyền vào thuật toán như tham số.
- Khi dữ liệu to/copy đắt → thêm tầng **view** (`RecordView` giữ con trỏ, tr. 53): interface mới trên dữ liệu cũ, *"change the view type if different behavior is required"* — không copy, không sửa struct gốc.
- Cân nhắc thêm operator: `==` cần cho `std::find`/hash map; `<` chỉ nên có khi tồn tại **một** thứ tự tự nhiên không gây tranh cãi (số, chuỗi) — không thì named functions trung thực hơn. 🆕 C++20 có `<=>` (spaceship) sinh cả bộ so sánh — vẫn chỉ nên dùng khi thứ tự tự nhiên tồn tại; và lưu ý hệ quả thực dụng: kiểu làm key của `std::map` cần ordering — chọn ordering nào là quyết định *thiết kế*, không phải chi tiết.
- Format trả lời: "bất biến → vào class; tùy bài → hàm rời; copy đắt → view" — ba mệnh đề phủ hết ví dụ của sách.

</details>

### Đọc thêm (tùy chọn)

- [thinking-computationally.md](thinking-computationally.md) — cụm 3 (functional/structural patterns) và cụm 5 (concepts) là nền của chương này.
- [07/api-design.md](../../07-shared-libraries/api-design.md), [07/abi-versioning.md](../../07-shared-libraries/abi-versioning.md) — chuyện interface/ABI ở mức nghề.
- [12-design-patterns/](../../12-design-patterns/) — strategy/adapter/decorator chi tiết.
- [EMC++](../effective-modern-cpp.md) — closure class (Cụm 6), Pimpl (Item 22), noexcept (Item 14).
