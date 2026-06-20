# CMake — Meta-build System

> **TL;DR**
> - CMake là **meta-build system**: từ `CMakeLists.txt` nó **sinh** ra build system thật (Makefile, Ninja, Visual Studio project...) cho từng nền tảng → một mô tả build, chạy được mọi nơi.
> - Build **out-of-source** (thư mục `build/` riêng) giữ source sạch.
> - **Modern CMake (target-based)**: nghĩ theo **target** (`add_library`, `add_executable`) và mô tả thuộc tính (include dir, flag, dependency) qua `target_*` với phạm vi **PUBLIC/PRIVATE/INTERFACE** — thay vì biến toàn cục kiểu cũ.
> - **`find_package`** + import target để dùng thư viện ngoài; dependency lan truyền tự động (transitive).
> - Hai bước: **configure** (`cmake -S . -B build`) rồi **build** (`cmake --build build`).

---

## 1. CMake giải quyết gì?

Makefile viết tay không di động: đường dẫn, flag, cách tìm thư viện, cách link khác nhau giữa Linux/macOS/Windows và giữa gcc/clang/MSVC. CMake tách **"mô tả ý định build"** (CMakeLists.txt) khỏi **"build system cụ thể"**: bạn viết một lần, CMake sinh ra công cụ phù hợp môi trường (Unix Makefiles, Ninja, Xcode, VS). Nó cũng chuẩn hóa việc tìm thư viện, quản lý dependency, cài đặt, test (CTest), đóng gói (CPack).

---

## 2. CMakeLists.txt cơ bản

```cmake
cmake_minimum_required(VERSION 3.16)
project(myapp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(myapp
    src/main.cpp
    src/utils.cpp
)

target_include_directories(myapp PRIVATE include)
```

Build (out-of-source, two-step):
```sh
cmake -S . -B build              # ① configure: đọc CMakeLists, sinh build system vào build/
cmake --build build              # ② build: gọi make/ninja thực sự
cmake --build build --target install
```

> **Out-of-source**: mọi sản phẩm build nằm trong `build/`, source không bị "bẩn" → xóa `build/` là sạch hoàn toàn, dễ tạo nhiều cấu hình (Debug/Release) song song.

---

## 3. Modern CMake: target-based + scope

Cách **cũ** (tránh): biến toàn cục như `include_directories()`, `add_definitions()` áp lên *mọi* target → khó kiểm soát, dễ rò.

Cách **modern**: gắn thuộc tính vào **target** với phạm vi rõ ràng:

```cmake
add_library(mathlib src/math.cpp)

target_include_directories(mathlib
    PUBLIC  include      # ai dùng mathlib CŨNG thấy include này
    PRIVATE src)         # chỉ mathlib dùng khi build chính nó

target_compile_features(mathlib PUBLIC cxx_std_17)

add_executable(myapp src/main.cpp)
target_link_libraries(myapp PRIVATE mathlib)   # tự kế thừa PUBLIC props của mathlib
```

**Ý nghĩa scope** (then chốt, hay được hỏi):
| Scope | Áp cho chính target? | Lan tới target dùng nó? |
|-------|----------------------|--------------------------|
| **PRIVATE** | Có | Không |
| **PUBLIC** | Có | Có |
| **INTERFACE** | Không | Có |

→ Nhờ vậy dependency **lan truyền (transitive)** tự động: `myapp` link `mathlib` thì tự nhận include dir/flag PUBLIC của `mathlib`, không phải khai lại. Đây là điểm mạnh lớn nhất của modern CMake.

---

## 4. Dùng thư viện ngoài: find_package

```cmake
find_package(Threads REQUIRED)
target_link_libraries(myapp PRIVATE Threads::Threads)

find_package(fmt REQUIRED)
target_link_libraries(myapp PRIVATE fmt::fmt)   # import target mang sẵn include + flag
```

- `find_package` định vị thư viện đã cài và cung cấp **imported target** (vd `fmt::fmt`) gói sẵn include path, library, định nghĩa → chỉ cần `target_link_libraries`.
- Cách lấy dependency khác: `FetchContent` (tải & build cùng dự án), `add_subdirectory` (sub-project), hoặc package manager (vcpkg/Conan).

---

## 5. Biến hữu ích & build type

```cmake
# Build type: Debug (-g, không tối ưu) / Release (-O2/-O3, NDEBUG) / RelWithDebInfo / MinSizeRel
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Generator: chọn build tool
cmake -S . -B build -G Ninja        # Ninja nhanh hơn Make cho dự án lớn

# Option do mình định nghĩa
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

- **`CMAKE_BUILD_TYPE`** quyết định flag tối ưu/debug (cho single-config generator như Make/Ninja).
- **Generator** (`-G`): Make, **Ninja** (build song song nhanh), Xcode, Visual Studio...
- Cấu hình truyền vào bằng `-D<VAR>=<VALUE>`.

---

## 6. Cấu trúc dự án nhiều thành phần

```cmake
# CMakeLists.txt gốc
add_subdirectory(libs/core)     # mỗi thư mục có CMakeLists riêng định nghĩa target
add_subdirectory(libs/net)
add_subdirectory(app)
# app/CMakeLists.txt: target_link_libraries(app PRIVATE core net)
```

Mỗi thư viện là một target tự mô tả thuộc tính PUBLIC/PRIVATE của mình; thành phần dùng nó chỉ cần link → quan hệ rõ ràng, dễ tái sử dụng.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) CMake là gì? Nó khác Make thế nào?</summary>

CMake là một **meta-build system**: nó không trực tiếp biên dịch mà từ mô tả `CMakeLists.txt` sinh ra một build system cụ thể (Unix Makefiles, Ninja, Visual Studio, Xcode...) phù hợp với nền tảng và công cụ hiện có; rồi build system đó mới thực sự gọi compiler. Make là build system thực thi trực tiếp (đọc Makefile, chạy recipe). Lợi ích của CMake so với Makefile viết tay: di động đa nền tảng/compiler từ một mô tả duy nhất, tự tìm thư viện (`find_package`), quản lý dependency theo target và phạm vi, tích hợp IDE, test (CTest) và đóng gói (CPack).
</details>

<details><summary>2) Hai bước configure và build trong CMake là gì? Out-of-source build là gì?</summary>

Bước **configure** (`cmake -S . -B build`): CMake đọc CMakeLists.txt, dò compiler/thư viện/hệ thống, đánh giá logic, và sinh ra build system thật vào thư mục build. Bước **build** (`cmake --build build`): gọi công cụ thật (make/ninja) để biên dịch và link. Out-of-source build nghĩa là mọi file sinh ra (cache, Makefile, object, binary) nằm trong một thư mục build riêng tách khỏi source — giữ cây source sạch, cho phép nhiều cấu hình song song (Debug/Release ở các thư mục build khác nhau), và xóa thư mục build là dọn sạch hoàn toàn.
</details>

<details><summary>3) PUBLIC, PRIVATE, INTERFACE trong target_link_libraries/target_include_directories nghĩa là gì?</summary>

Chúng quy định phạm vi lan truyền (usage requirement) của một thuộc tính. PRIVATE: thuộc tính chỉ áp dụng khi build **chính target đó**, không lan sang target khác link nó. INTERFACE: ngược lại — không áp cho chính target mà chỉ lan sang target dùng nó (thường cho header-only library). PUBLIC: cả hai — áp cho chính target và lan sang target dùng nó. Nhờ cơ chế này, khi target A link một thư viện B với thuộc tính PUBLIC (vd include dir), bất kỳ target nào link A cũng tự động kế thừa các yêu cầu PUBLIC của B (transitive dependency), không phải khai báo lại thủ công.
</details>

<details><summary>4) Modern CMake (target-based) khác cách cũ ở điểm nào và vì sao tốt hơn?</summary>

Cách cũ dùng các lệnh/biến toàn cục như `include_directories()`, `add_definitions()`, `link_libraries()` áp đặt lên **mọi** target trong scope hiện tại — gây rò rỉ flag/include không mong muốn, khó biết target nào thực sự cần gì, dễ xung đột. Modern CMake gắn mọi thuộc tính trực tiếp vào target qua `target_include_directories`, `target_compile_definitions`, `target_link_libraries` với phạm vi PUBLIC/PRIVATE/INTERFACE rõ ràng. Lợi ích: mỗi target tự mô tả "yêu cầu sử dụng" của nó, dependency lan truyền chính xác và tự động, dự án dễ mở rộng và tái sử dụng (mỗi thư viện là một đơn vị độc lập, kéo theo đúng yêu cầu của nó khi được link).
</details>

<details><summary>5) find_package làm gì? Có những cách nào để dùng thư viện ngoài trong CMake?</summary>

`find_package(X)` định vị thư viện X đã cài trên hệ thống (qua module Find hoặc file config do thư viện cung cấp) và thường tạo ra các **imported target** (vd `fmt::fmt`, `Threads::Threads`) đóng gói sẵn include path, đường dẫn library và các định nghĩa cần thiết; ta chỉ cần `target_link_libraries(app PRIVATE fmt::fmt)` là có đủ. Các cách khác để lấy dependency: `add_subdirectory` (gộp source của thư viện như sub-project), `FetchContent` (tải và build dependency cùng lúc configure), hoặc dùng package manager như vcpkg/Conan tích hợp với CMake.
</details>

---
⬅️ [makefile.md](makefile.md) · ➡️ Tiếp theo: [cross-compilation.md](cross-compilation.md)
