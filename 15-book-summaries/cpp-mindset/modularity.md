# C++ Mindset — Ch. 6: Reusing Your Code and Modularity (tr. 145–161) ✅ đã đọc sâu toàn văn

> Thuộc [The C++ Programmer's Mindset](README.md). Nguồn: đọc trực tiếp PDF, trang sách = trang PDF − 25.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> Ý đồ chương (tr. 145): code chỉ hữu ích nếu **người khác (gồm chính bạn về sau) dùng được** → phải thiết kế interface đơn giản + ổn định, rồi đóng gói thành library. Chương khép lại Phần 1 (lý thuyết). **Đây là chương trúng nghề nhất với bạn** — shared library, ABI stability, export symbol, Pimpl là công việc System Software/BSP hằng ngày (↔ [topic 07](../../07-shared-libraries/)). Sách dẫn nguồn chuẩn: **Linkers & Loaders** (Levine) và **Design Patterns** (GoF) (tr. 161).

---

## Cụm 1 — Static vs shared library, header-only, public/private (tr. 146–148)

### Nội dung chính

**Library = "hộp chứa code để tái dùng" (tr. 146).** Hai dạng chính, khác nhau ở **thời điểm code được nhập vào ứng dụng**:

| | **Static library** (`.a` / `.lib`) | **Shared library** (`.so` / `.dll` / `.dylib`) |
|---|---|---|
| Code nhập vào app khi nào | **Link time** — code được nhúng thẳng vào binary app | **Load time** — code **không** nhúng; dynamic loader (`ld` trên Linux) nạp khi app khởi động |
| Cơ chế | archive của function/class + metadata để linker tìm | dùng **offset table** trong library để loader định vị từng symbol cần |
| Tốc độ | nhanh hơn chút (code có sẵn trong binary, không gián tiếp) | *"modern dynamic loaders and CPU functions make the advantage... somewhat negligible"* (tr. 146) |
| Kích thước binary | **lớn hơn** (code được sao vào) | nhỏ hơn; **một bản dùng cho nhiều app** |
| Rủi ro hiếm | link cùng static lib vào nhiều component → lỗi cấp phát/global init (tr. 146) | — |
| Cập nhật | phải **recompile app** | **upgrade library mà không recompile app** (nếu interface không đổi) — quan trọng cho OS/framework *"that must not break applications"* (tr. 148) |

Ví dụ nền (tr. 146): mọi app C/C++ trên Linux link tới **`libc`** (chứa `malloc`/`free` và phần C standard library nói chuyện với OS); kèm `libm` (toán) và các lib hệ thống khác.

**Header-only library (tr. 147):** library **chỉ gồm template code** không cần thành phần compiled → phân phối thuần header. Phần lớn C++ standard library và nhiều Boost là header-only. Template được **instantiate lúc compile thẳng vào binary app** → hành xử giống static library; cực linh hoạt (compile ở đâu có compiler hợp chuẩn) nhưng trả giá **thời gian compile tăng** (instantiate template đắt, nhất là đệ quy).

**Public vs private components (tr. 147–148):** library chia hai phần — **external-facing (public)** và **internal implementation (private)**. Header public quyết định cái gì client thấy được. Che phần private:
- **Namespace `detail`/`dtl`/`internal`** — quy ước báo hiệu "không dùng ngoài" (tr. 147).
- Comment/documentation đánh dấu "internal use".
- Shared library còn có **kiểm soát export symbol** (cụm này, phần dưới) — static/header-only không có.

Điểm cốt lõi (tr. 147): phân biệt public/private *"is not so much about detail and guarantees but instead about **stability**."* API (programming interface) + **ABI** (binary interface) của phần public phải **ổn định** trong khung thời gian xác định; private thì không cần. Khi buộc đổi public interface → dùng **symbol versioning** qua **inline namespace**:

```cpp
namespace ct {
    inline namespace v1_0 {                       // 'inline' → symbol được RE-EXPORT ra scope ct
        int versioned_function(int a, int b);
    }
}
// Client gọi ct::versioned_function(...) như thường,
// nhưng tên mangled thật là ct::v1_0::versioned_function
// → muốn ra v2: đổi v1_0 thành NON-inline, thêm namespace v2_0 MỚI là inline
//   → hai phiên bản symbol cùng tồn tại, code cũ vẫn link được bản cũ
```
Inline namespace *"re-export the symbols declared within into the surrounding scope"* (như unnamed namespace) (tr. 148) → tạo được version mới **mà không xoá version cũ**. ⚠️ Lưu ý (tr. 148): **C++20 modules** cho kiểm soát reference symbol mịn hơn nhưng **không thay thế namespace** và **không giải quyết versioning**.

### Insight đáng nhớ

- Bảng static vs shared (tr. 146) gói câu hỏi phỏng vấn kinh điển "link time vs load time": 🆕 static = *"code nhập lúc link, mỗi app một bản, đổi lib phải build lại app"*; shared = *"code nạp lúc load, nhiều app chung một bản, đổi lib không cần build lại app (nếu ABI giữ nguyên)"*. Câu chốt để nhớ: **static ưu tiên tính tự chứa/tốc độ; shared ưu tiên chia sẻ/vá được mà không đụng client** — đó là lý do OS và framework (glibc, kernel driver interface) phải là shared và **cực kỳ giữ ABI**.
- Inline namespace cho versioning (tr. 147–148) là kỹ thuật ABI thật ít người biết: 🆕 chính glibc dùng nó (`__asm__(".symver ...")` ở tầng thấp, inline namespace ở tầng C++) để `memcpy@GLIBC_2.2.5` và `memcpy@GLIBC_2.14` cùng tồn tại — binary cũ chạy được trên glibc mới. Nối [07/abi-versioning.md](../../07-shared-libraries/abi-versioning.md): đây là cách "thêm version mới, giữ version cũ" mà không phá client đã compile.

---

## Cụm 2 — Dynamic library & export symbol (tr. 148–151) 🎯

### Nội dung chính

**Ưu thế của shared library (tr. 148):** (1) **size** — một bản dùng chung nhiều app; (2) **upgrade được mà không recompile app** (nếu interface không đổi) → phát hành bugfix/cải thiện hiệu năng không làm gián đoạn ứng dụng đang chạy.

**Cơ chế link/load (tr. 148–149):** đa số shared library được **link tĩnh như static library** ở link time — linker chèn code/data để *định vị và nạp* symbol sẽ được shared library cung cấp; nhưng **địa chỉ symbol chưa được resolve hoàn toàn cho tới run/load time** → đó là việc của **dynamic linker**. Chi tiết nền tảng (đọc sâu: Linkers & Loaders):
- **Linux/macOS:** link chỉ cần library + header; binary mang danh sách đường dẫn **RPATH** để dynamic linker tìm shared library lúc runtime (nhất là lib không qua package manager) (tr. 148).
- **Windows:** link vào **stub file `.lib`** (dễ nhầm với static lib!); shared object (`.dll`) phải đóng gói kèm binary; **không có RPATH** → *"finding DLLs rather frustrating at times"*, nhất là khi test trong build tree (tr. 148).
- **Nạp thủ công (tr. 149):** `dlopen`/`dlsym` (Linux) để nạp **extension/plugin** không thiết yếu lúc chạy. 🆕 Đây chính là cơ chế plugin architecture — nạp `.so` theo tên lúc runtime, tra symbol bằng `dlsym`.

**Exporting symbols (tr. 149–151)** 🎯 — điểm thực hành quan trọng nhất cụm: shared library **không** để lộ mọi function cho dynamic linker.
- **Mặc định GCC/Clang: export TẤT CẢ symbol** (ngược đời!); MSVC: phải đánh dấu `dllexport` tường minh để export, `dllimport` để import từ DLL.
- Thực hành đúng: **chọn tường minh** cái gì export → giấu phần còn lại. CMake lo hết:

```cmake
# Đặt mặc định cho cả project: KHÔNG export gì trừ khi đánh dấu tường minh
set(CMAKE_VISIBILITY_PRESET hidden)          # -fvisibility=hidden
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)      # -fvisibility-inlines-hidden
add_library(MySharedLibrary SHARED)
# ... hoặc set per-target bằng set_target_properties(... VISIBILITY_PRESET hidden ...)

include(GenerateExportHeader)
generate_export_header(MySharedLibrary)      # sinh mysharedlibrary_export.h chứa macro EXPORT
target_include_directories(MySharedLibrary PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```
`generate_export_header` sinh macro **`MYSHAREDLIBRARY_EXPORT`** — build library thì nó expand thành lệnh export của compiler (`__declspec(dllexport)` / `__attribute__((visibility("default")))`); consume library thì expand thành import (`__declspec(dllimport)` trên Windows, rỗng chỗ khác) (tr. 150). Dùng trong header:

```cpp
#include "mysharedlibrary_export.h"
namespace ct {

class MYSHAREDLIBRARY_EXPORT MyExportedClass {   // class được export
public:
    void exported_method();
};

MYSHAREDLIBRARY_EXPORT int exported_function(int arg);   // function được export

namespace dtl {                                   // namespace nội bộ
    class UnexportedInternalClass;                // KHÔNG đánh dấu → KHÔNG export
}
} // namespace ct
```
Kết quả (tr. 150–151): không symbol nào bị export trừ khi đánh dấu tường minh — bất kể compiler/platform. Nếu target không phải shared library, macro expand thành rỗng (cùng code build được cả static lẫn shared).

### Insight đáng nhớ

- `-fvisibility=hidden` + export tường minh (tr. 149–151) là **best practice ABI thật** mà nhiều codebase bỏ qua: 🆕 export tất cả (mặc định GCC/Clang) gây (1) **bề mặt ABI khổng lồ** — mọi symbol thành "lời hứa" phải giữ; (2) load chậm (dynamic linker phải resolve nhiều); (3) rò rỉ chi tiết nội bộ. Đặt hidden mặc định biến ABI thành **"opt-in tối thiểu"** — chỉ giữ lời hứa cho cái thực sự public. Đây là câu trả lời "làm sao giữ ABI ổn định" ở tầng cơ chế, bổ sung cho inline namespace (cụm 1) ở tầng versioning.
- Mẹo `generate_export_header` (tr. 150) đáng dùng ngay: 🆕 tự viết macro `#if defined(_WIN32) __declspec(dllexport)...` là nguồn lỗi cross-platform kinh điển (quên dllimport phía consumer, sai visibility trên Linux); để CMake sinh macro chuẩn hoá theo target. Nối [07/linking-loading.md](../../07-shared-libraries/linking-loading.md) (PLT/GOT, dynamic loader) — visibility quyết định symbol nào vào bảng động.

---

## Cụm 3 — Thiết kế interface ổn định & quản lý bộ nhớ qua boundary (tr. 151–153) 🎯

### Nội dung chính

**Vì sao public interface phải ổn định (tr. 151):** người dùng trung gian (gồm chính bạn) build app/library **consume** interface của bạn; bắt họ recompile mỗi lần bạn phát hành shared library mới là *"unreasonable (and in many cases impossible)."* Static library cũng cần stability (code đã "baked into" app).

**Ba nguyên tắc thiết kế interface linh hoạt từ đầu (tr. 151):**
1. **Layer your interface** — tách **core functionality** khỏi **convenience/user-facing functions**. Template + inline function xây *trên* core để tạo interface đơn giản hoá; core tự do trình bày interface phức tạp phủ nhiều use case. Hai lợi ích: linh hoạt + làm prototype để user tự xây interface riêng quanh core của bạn.
2. **Keep the interface minimal** — *"The larger the interface, the more work to maintain it."* Số function/class public phải **nhỏ**; mở rộng bằng lớp template/inline (dễ đổi hơn).
3. **Encapsulate implementation data** — *"Changing data members in a class is a sure way to break binary compatibility and force end users to recompile."* → giấu data cài đặt sau **opaque object (Pimpl idiom)** để có chỗ đổi nội bộ. Nhiều design pattern GoF dùng abstraction kiểu này.

**Handling memory across boundaries (tr. 152–153)** 🎯 — nguồn lỗi kinh điển:
- **Vấn đề gốc:** free bộ nhớ bằng allocator **khác** cái đã allocate → lỗi. Modern C++ (smart pointer + allocator framework) gần như xoá vấn đề này *trong* C++, nhưng nhiều library có **C interface** phải truyền qua boundary → smart pointer C++ không tự cứu.
- **Ví dụ FILE\* (tr. 152):** `FILE*` từ `fopen` phải đóng bằng `fclose`. Truyền `FILE*` qua boundary interface **nguy hiểm**: client không biết cách đóng an toàn; tệ hơn — nếu client có **phiên bản `fclose` khác** thì dùng bản của họ → **UB**. Giải: bọc trong `unique_ptr` với custom deleter:

```cpp
#include <memory>
#include <cstdio>
struct FileDeleter {
    void operator()(FILE* f) noexcept { fclose(f); }   // deleter dùng ĐÚNG fclose của library
};
using FilePointer = std::unique_ptr<FILE, FileDeleter>;
```
⚠️ Chi tiết ABI (tr. 152): `FileDeleter::operator()` nên định nghĩa trong **`.cpp`** (không inline) để tránh "dual copy" — đảm bảo dùng đúng `fclose` của library, không phải của client. `shared_ptr` cũng làm được (lưu type-erased deleter trong control block trên heap).

- **Opaque type khi encapsulate (Pimpl) (tr. 152–153):**

```cpp
class MyClass {
    struct ImplementationData;                       // khai báo, KHÔNG định nghĩa (opaque/incomplete)
    std::unique_ptr<ImplementationData> data_;       // con trỏ tới impl giấu trong .cpp
    // methods ...
};
```
⚠️ Bẫy (tr. 153): code trên **fail compile** ngoài scope thấy định nghĩa `MyClass::ImplementationData` — vì `unique_ptr` cần **complete type** để sinh destructor (destructor của MyClass phải biết cách huỷ ImplementationData). Fix: **khai destructor trong class, định nghĩa `= default` trong `.cpp`** (nơi ImplementationData đã complete):

```cpp
// MyClass.h:  class MyClass { ...; ~MyClass(); };   // chỉ KHAI BÁO
// MyClass.cpp:
struct MyClass::ImplementationData { /* ... */ };
MyClass::~MyClass() = default;                        // ĐỊNH NGHĨA tại đây — ImplementationData đã complete
```
*"Defining the destructor is certainly easier, and much less intrusive, and is preferable in almost all cases"* (so với dùng custom deleter cho opaque type) (tr. 153).

### Insight đáng nhớ

- Ba nguyên tắc (layer / minimal / encapsulate — tr. 151) quy về một mục tiêu: **giữ ABI ổn định = giữ cho những gì client đã compile vào binary của họ không đổi**. 🆕 "Encapsulate data" là quan trọng nhất về ABI: thêm/xoá/đổi thứ tự **data member** đổi `sizeof(class)` và offset → mọi client đã inline constructor/truy cập member đều vỡ. Pimpl đẩy toàn bộ data ra sau một con trỏ (`sizeof` cố định = 1 pointer) → đổi impl thoải mái. Trùng khớp [EMC++ Item 22](../effective-modern-cpp.md) (Pimpl + destructor trong .cpp) và [07/api-design.md](../../07-shared-libraries/api-design.md).
- `FileDeleter` với deleter trong .cpp (tr. 152) dạy bài "memory ownership qua boundary": 🆕 quy tắc vàng — **ai allocate thì phải là người free**, và qua boundary C/shared-lib, cách free phải đi *kèm* con trỏ (RAII wrapper với deleter của chính library). Đây là lý do nhiều C API có cặp `xxx_create()`/`xxx_destroy()` — client không bao giờ gọi `free()` thẳng lên con trỏ nhận từ library (có thể khác heap/allocator). Cực kỳ trúng nghề System Software C++ shared library của bạn.

---

## Cụm 4 — C++ modules (tr. 153–156)

### Nội dung chính

**Module là gì (tr. 153):** C++20 giới thiệu **module** — cách chia sẻ declaration/definition qua translation unit, **thay thế một phần** header file (giống cách Rust tổ chức code theo module từ cấu trúc file). ⚠️ **Không thay thế namespace** — vẫn dùng namespace cùng module. **Interface module** khai bằng `export module`:

```cpp
// module_example.cpp — interface module unit
export module computational_thinking;              // khai tên module

export namespace ct {
    void exported_fn(int a);                        // export cả namespace
}
int non_exported_fn();                              // KHÔNG export (nhưng module dùng nội bộ được)
export int another_exported_fn(int b);             // export function lẻ (global namespace)

export inline int inline_fn(int c) {               // export inline function
    return non_exported_fn() * c;                  // gọi được symbol KHÔNG export bên trong module
}
export struct MyStruct;                            // export struct (định nghĩa nơi khác)
export template <typename T> T template_fn(T e) { return e; }   // export template
export template <typename T> class TemplateClass { T f; public: T get_f() { return f; } };
```
Điểm mấu chốt (tr. 154): function **không export vẫn dùng được trong module** (kể cả code external gọi qua hàm export như `inline_fn`), nhưng code ngoài **không thể tham chiếu chúng bằng tên**. → kiểm soát symbol **mịn hơn** namespace `detail`.

**Implementation module unit (tr. 154–155):** cài đặt các thứ đã khai trong interface, ở file `.cpp` riêng:

```cpp
module;                                            // bắt đầu GLOBAL MODULE FRAGMENT
#include <iostream>                                // #include nên nằm ở đây (global fragment)

module module_example;                             // mở rộng định nghĩa module
// import <iostream>;  // ưu tiên dùng cách này thay #include, nếu compiler hỗ trợ

void ct::exported_fn(int a) { std::cout << a << std::endl; }
int another_exported_fn(int b) { std::cout << b << std::endl; return b; }
struct MyStruct { int d; };
```
**Global module fragment** (giữa `module;` và `module <name>;`) là nơi đặt `#include` (tr. 155). ⚠️ GCC 14.2 (thời sách viết) chưa có precompiled module cho standard library → phải compile trước khi `import` được. **Semantics module tách rời khỏi cơ chế export/import của shared library** — vẫn cần dllexport/dllimport trong module interface unit; *"It isn't exactly clear yet in the documentation how this should be achieved"* (tr. 155).

**Dùng module (tr. 155):** `import` theo tên (như header), rồi mọi exported symbol khả dụng. Tên module chỉ dùng cho import, **không** đổi tên function (namespace vẫn là cơ chế scope):

```cpp
import computational_thinking;
int main() {
    MyStruct s { 2 };                    // exported, định nghĩa nơi khác
    auto r = another_exported_fn(s.d);   // exported, global namespace
    ct::exported_fn(r);                  // exported trong namespace ct
}
```
**Module fragment** (tr. 156): chia định nghĩa module thành phần nhỏ, `export import` trong main module unit để gom + re-export symbol.

**Đánh giá thẳng thắn của tác giả (tr. 156):** *"C++ modules are a great addition to the language, but writing this chapter made me feel that, at the moment at least, there are too many problems for them to be widely adopted. As of early 2025, compiler support is spotty at best... the problem of shared library exports [is] just not sufficiently explored yet. Until these kinds of issues are resolved, using modules is not going to be a portable and stable experience."*

### Insight đáng nhớ

- Module giải quyết đúng các bệnh của header (🆕 nối [06/build-systems](../../06-build-systems/)): (1) **không parse lại header N lần** (mỗi TU include lại → compile chậm) — module compile một lần; (2) **không rò macro/symbol** qua include (module chỉ export cái được đánh dấu); (3) thứ tự include không còn quan trọng. Nhưng đánh giá của tác giả (tr. 156) là **cảnh báo thực tế quan trọng**: 2025 vẫn chưa portable, và **vấn đề shared library export + module chưa giải quyết** → với nghề shared library như bạn, **chưa nên chuyển sang module** cho public interface, giữ header + visibility (cụm 2).
- Điểm "không export vẫn dùng nội bộ được, chỉ không tham chiếu bằng tên từ ngoài" (tr. 154) là bản nâng cấp của namespace `detail`: 🆕 `detail` chỉ là *quy ước* (client vẫn gọi được nếu cố), còn module là *cơ chế cưỡng chế* (compiler chặn). Đây là lợi ích thật của module cho library — nhưng chỉ dùng được khi toolchain chín.

---

## Cụm 5 — Tách component trong build system (tr. 156–160) 🎯

### Nội dung chính

**Vấn đề (tr. 156):** project lớn khó quản như một khối → tách thành component nhỏ trong build system. Rủi ro: khó làm đúng, nhất là khi output là shared library. **Testing** là chỗ đau: test **internal** của shared library khó vì chúng **không export** → chiến lược: kéo internal vào một **static library** riêng, link vào library lớn **và** test độc lập. Câu chìa khoá (tr. 156): *"The key to modularizing your projects is to understand dependencies"* — module lý tưởng **standalone**, không phụ thuộc module khác cùng project (nếu không tách được thì concept đó **không nên** tách).

**Ví dụ ODE solver (tr. 156–158):** library giải phương trình vi phân, nhiều phương pháp (Euler, Runge-Kutta RK4). Public interface (shared library) chỉ cần **chọn solver**:

```cpp
#include <functional>
#include <span>
#include <vector>
namespace ct::ode {

enum class SolveMethod { Euler, RK4, /* ... */ };

class OdeSolution {
    std::vector<double> solution;
    std::vector<double> step_params;
};

using ODEFunction = std::function<void(std::span<double>, double, std::span<const double>)>;

OdeSolution ode_solve(                              // hàm public DUY NHẤT
    SolveMethod method,                             // enum chọn solver
    ODEFunction func,
    std::span<const double> initial_condition,
    double initial_param, double final_param, double step_size);
} // namespace ct::ode
```
Bên trong `ode_solve` dispatch tới hàm solver theo enum. **Internal interface** (mỗi method cài) có signature giống nhau, **không** dùng định nghĩa nào từ library chính (tr. 158):

```cpp
void solver_method(                                 // interface NỘI BỘ, mỗi method (Euler/RK4) cài
    std::vector<double>& solution,
    std::vector<double>& step_params,
    const ODEFunction& func,
    std::span<const double> initial_condition,
    double initial_param, double final_param, double step_size);
```

**Cấu trúc thư mục + CMake (tr. 158–160):** mỗi solver là **static library riêng, không external dependency**, có test riêng:

```
ctode
├── euler/   CMakeLists.txt, euler.h, euler.cpp, test_euler.cpp
├── rk4/     CMakeLists.txt, rk4.h,  rk4.cpp,   test_rk4.cpp
├── CMakeLists.txt, ctode.h, ctode.cpp, test_ctode.cpp
```
`euler/CMakeLists.txt`:

```cmake
add_library(ctode_euler STATIC euler.h euler.cpp)
set_target_properties(ctode_euler PROPERTIES
    POSITION_INDEPENDENT_CODE ON)                   # -fPIC: BẮT BUỘC vì sẽ link vào shared lib
if (ENABLE_TESTS)
    add_executable(test_ctode_euler test_euler.cpp)
    target_link_libraries(test_ctode_euler PRIVATE ctode_euler GTest::gtest_main)
endif()
```
⚠️ Dòng đáng chú ý nhất (tr. 159): **`POSITION_INDEPENDENT_CODE ON`** (= `-fPIC`) — bắt buộc khi static library sẽ được link **vào một shared library** (code phải position-independent để load ở địa chỉ bất kỳ). Và: **không function nào trong `euler.h` được export** khỏi `ctode` shared library. Top-level `CMakeLists.txt`:

```cmake
if (ENABLE_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
endif()
include(GenerateExportHeader)
add_subdirectory(euler)
add_subdirectory(rk4)
add_library(ctode SHARED ctode.h ctode.cpp)
target_link_libraries(ctode PRIVATE ctode_euler ctode_rk4)   # link 2 static solver vào shared lib
generate_export_header(ctode)
target_include_directories(ctode PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```
Kết quả (tr. 160): solver function khả dụng **nội bộ** trong `ctode.cpp` qua `ode_solve`, nhưng **giấu khỏi user**; đồng thời **test được độc lập** (`test_ctode_euler`). Sách tự chỉ ra (tr. 160): *"Separating concerns is one of the four components of computational thinking"* — vòng lại ch. 1!

**Hai lời khuyên thực hành (tr. 160):**
- Ngay cả khi code cùng một target, dùng **`target_sources`** để mở rộng danh sách source theo nhóm chức năng → người đọc (gồm future you) dễ tìm, dễ tái dùng.
- Khi các component có **dependency chung** (vd solver dùng chung abstract base class) → gom common vào một target riêng (`ctode_common`) rồi link vào mọi solver + shared library chính; linker lo bỏ trùng khi link.

### Insight đáng nhớ

- Pattern "internal = static library có test riêng, public = shared library link các static đó" (tr. 156–160) là **giải pháp chuẩn cho "làm sao test code nội bộ của shared library"** — 🆕 vì code không export thì test bên ngoài không link tới được; kéo vào static lib (test link trực tiếp, thấy mọi symbol) rồi nhúng vào shared lib (export chọn lọc). Rất trúng nghề: đây là cấu trúc bạn sẽ dựng cho một C++ shared library có phần lõi cần unit test.
- `POSITION_INDEPENDENT_CODE ON` cho static lib nhúng vào shared lib (tr. 159) là **lỗi build kinh điển**: 🆕 quên `-fPIC` → link shared library báo *"relocation R_X86_64_... can not be used when making a shared object; recompile with -fPIC"*. Static lib mặc định không PIC; shared lib đòi PIC. Biết dòng này tiết kiệm hàng giờ debug link error. Nối [06/cross-compilation](../../06-build-systems/) và [07/linking-loading](../../07-shared-libraries/linking-loading.md).
- Câu tác giả tự nối "separating concerns = 1 trong 4 thành phần computational thinking" (tr. 160) khép Phần 1 gọn ghẽ: 🆕 decomposition (ch. 1) không chỉ là kỹ thuật giải bài toán *trên giấy* mà hiện hình thành **cấu trúc thư mục + target build**. Modular hoá là decomposition ở tầng kỹ thuật phần mềm.

---

## Góc interview

**Câu 1 (🎯):** Static library vs shared library — khác nhau thế nào, chọn cái nào khi nào? Điều gì được resolve ở link time vs load time?

<details><summary>Đáp án</summary>

- **Static (`.a`/`.lib`):** code được **nhúng vào binary app ở link time** — linker copy function/class cần vào executable. App tự chứa (không phụ thuộc lib khi chạy), nhanh hơn chút (không gián tiếp), nhưng binary lớn hơn, và **đổi lib phải build lại app** (tr. 146).
- **Shared (`.so`/`.dll`/`.dylib`):** code **không** nhúng; link time chỉ chèn thông tin để **dynamic loader** (`ld` trên Linux) nạp library **lúc load** và resolve địa chỉ symbol qua offset table. Một bản library dùng chung nhiều app (tiết kiệm RAM/disk), **upgrade lib mà không build lại app** miễn ABI không đổi (tr. 146, 148).
- **Link time vs load time:** static — symbol resolve **hoàn toàn ở link time** (code có sẵn trong binary). Shared — link time chỉ resolve *có symbol đó tồn tại*, **địa chỉ thật resolve ở load/run time** bởi dynamic linker (dùng RPATH tìm library trên Linux/macOS; Windows không có RPATH nên hay khổ tìm DLL) (tr. 148).
- **Chọn:** shared khi — nhiều app dùng chung, cần vá/upgrade không đụng client, là OS/framework "không được phá app" (glibc, driver interface). Static/header-only khi — muốn binary tự chứa (deploy đơn giản, đúng version), tránh "DLL hell", hoặc lib nhỏ chỉ một app dùng. 🆕 Embedded: static thường được ưa vì deploy tự chứa + không lo tìm .so lúc runtime; nhưng shared cho phần dùng chung nhiều process (tiết kiệm RAM — quan trọng khi RAM ít). `dlopen` cho plugin/extension nạp runtime (tr. 149).
- Điểm cộng: header-only (template) hành xử như static (instantiate vào binary app), đổi lấy compile time tăng (tr. 147).

</details>

**Câu 2 (🎯):** Làm sao giữ ABI của shared library ổn định để không bắt client recompile? Nêu các kỹ thuật.

<details><summary>Đáp án</summary>

- **Vấn đề:** client đã compile binary dựa trên layout/symbol của library cũ; đổi ABI → client vỡ (crash/UB) trừ khi recompile — mà bắt recompile *"unreasonable and in many cases impossible"* (tr. 151). Phân biệt **API** (mã nguồn) với **ABI** (nhị phân: layout class, mangled symbol, calling convention) — API giữ nguyên vẫn có thể vỡ ABI.
- **Kỹ thuật:**
  1. **Encapsulate data — Pimpl** (tr. 151–153): giấu mọi data member sau `std::unique_ptr<Impl>` → `sizeof(class)` cố định (1 con trỏ), đổi data nội bộ không đổi ABI. Nhớ khai destructor trong header, `= default` trong .cpp (unique_ptr cần complete type — [EMC++ Item 22](../effective-modern-cpp.md)).
  2. **Minimal + hidden visibility** (tr. 149–151): `-fvisibility=hidden` mặc định, export tường minh cái public (dùng `generate_export_header` của CMake) → bề mặt ABI nhỏ = ít lời hứa phải giữ.
  3. **Symbol versioning qua inline namespace** (tr. 147–148): `inline namespace v1_0 { ... }`; ra v2 thì bỏ inline khỏi v1_0, thêm `inline namespace v2_0` → hai version symbol cùng tồn tại, binary cũ vẫn link bản cũ.
  4. **Deprecation cycle**: symbol cần bỏ phải qua giai đoạn deprecated trước (tr. 147).
  5. **Layer interface** (tr. 151): core ổn định + tiện ích (template/inline) xây trên, dễ đổi phần ngoài mà không đụng core.
- **Không được làm** (vỡ ABI): thêm/xoá/đổi thứ tự data member (đổi sizeof/offset), thêm virtual function (đổi vtable layout), đổi signature function public, đổi kiểu tham số.
- 🆕 Nối nghề: đây đúng là công việc "viết C++ API interface cho lớp trên" của bạn — Pimpl + hidden visibility + versioning là bộ ba giữ ABI; và **truyền exception qua boundary shared lib là cấm** (dùng error code/`expected` — [EMC++](../effective-modern-cpp.md), ch. 1 của sách này). Chi tiết: [07/abi-versioning.md](../../07-shared-libraries/abi-versioning.md).

</details>

**Câu 3 (🎯):** Truyền con trỏ (heap object hoặc `FILE*`) qua boundary của shared library có gì nguy hiểm? Cách xử lý đúng?

<details><summary>Đáp án</summary>

- **Nguy hiểm (tr. 152):** allocate ở một bên, free ở bên kia bằng allocator/deleter **khác** → lỗi/UB. Cụ thể: client nhận `FILE*` từ library rồi gọi `fclose` — nhưng nếu client link phiên bản `fclose` khác (hoặc heap/CRT khác trên Windows), free bằng bản sai → **UB**. Tổng quát: mỗi shared library/CRT có thể có **heap riêng** → `free()` phải khớp `malloc()` cùng heap.
- **Quy tắc vàng:** *ai allocate thì bên đó phải cung cấp cách free*, và cách free đi **kèm** con trỏ.
- **Cách xử lý:**
  1. **RAII wrapper với deleter của library** (tr. 152): `using FilePointer = std::unique_ptr<FILE, FileDeleter>;` với `FileDeleter::operator()` gọi **đúng `fclose` của library** — và định nghĩa deleter trong **.cpp** (không inline) để tránh "dual copy" dùng nhầm bản của client.
  2. **Cặp create/destroy trong C API:** `xxx_create()` / `xxx_destroy()` — client không bao giờ `free()` thẳng; luôn trả con trỏ về library để nó free bằng đúng allocator của mình. (Đây là lý do mọi C library nghiêm túc có hàm destroy riêng.)
  3. **Opaque handle:** trả `void*`/handle mờ, client không biết layout, không tự huỷ.
  4. Với Pimpl opaque type: `shared_ptr` lưu type-erased deleter trong control block — huỷ đúng cách kể cả khi type incomplete ở chỗ khai báo (tr. 152–153).
- 🆕 Cực trúng nghề System Software: viết C++ shared library expose interface cho lớp trên → **không bao giờ** để client `delete`/`free` con trỏ do library cấp; luôn cung cấp RAII wrapper (nếu client C++) hoặc destroy function (nếu boundary C). Nối [07/api-design.md](../../07-shared-libraries/api-design.md).

</details>

**Câu 4:** Bạn có một shared library với phần lõi cần unit test, nhưng phần lõi không được export ra ngoài. Cấu trúc build thế nào để vừa giấu vừa test được?

<details><summary>Đáp án</summary>

- **Vấn đề (tr. 156):** test bên ngoài không link được tới symbol **không export** của shared library → không test trực tiếp phần internal.
- **Giải pháp (tr. 156–160):** tách phần internal thành **static library** riêng (không external dependency), rồi:
  1. **Link static lib vào shared lib** (`target_link_libraries(ctode PRIVATE ctode_euler ...)`) — code lõi vào trong shared lib nhưng **không export** (không đánh dấu EXPORT).
  2. **Link static lib vào test executable** trực tiếp (`target_link_libraries(test_ctode_euler PRIVATE ctode_euler GTest::gtest_main)`) — test thấy **mọi symbol** của static lib, test độc lập.
- **Chi tiết build bắt buộc:** static lib phải `set_target_properties(... POSITION_INDEPENDENT_CODE ON)` (**-fPIC**) vì sẽ nhúng vào shared lib — quên là link error *"recompile with -fPIC"* (tr. 159).
- Cấu trúc: mỗi component (euler/, rk4/) một thư mục — static lib + test riêng + CMakeLists riêng; top-level `add_subdirectory` gom lại + build shared lib công khai chỉ export `ode_solve` (tr. 158–160).
- 🆕 Đây là pattern chuẩn cho shared library có lõi phức tạp: **"internal = static lib (testable), public = thin shared lib (export chọn lọc)"**. Bonus: dùng `target_sources` để nhóm source ngay cả khi cùng target (dễ đọc); dependency chung của các component gom vào target `_common` link vào tất cả (tr. 160). Và như sách chốt: đây chính là **separation of concerns** (ch. 1) hiện hình ở tầng build.

</details>

### Đọc thêm (tùy chọn)

- [07-shared-libraries/](../../07-shared-libraries/) — static-vs-shared, linking-loading (PLT/GOT, loader), abi-versioning, api-design: **toàn bộ topic này là bản chi tiết của chương 6** — đọc cặp.
- [EMC++ Item 22](../effective-modern-cpp.md) — Pimpl + destructor trong .cpp (giống hệt cụm 3); Item 18–19 (unique_ptr/shared_ptr deleter).
- [06-build-systems/](../../06-build-systems/) — CMake, cross-compilation (-fPIC, target, visibility).
- [thinking-computationally.md](thinking-computationally.md) — ch. 1: separation of concerns mà chương này hiện thực hoá ở tầng build; error handling qua boundary (failure vs error, không ném exception qua ABI).
