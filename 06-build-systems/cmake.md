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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [BLD-001](../14-prep/mock-interview/bank/build-systems.md) | CMake là gì? Nó khác Make thế nào? |
| [BLD-011](../14-prep/mock-interview/bank/build-systems.md) | Hai bước configure và build trong CMake là gì? Out-of-source build là gì? |
| [BLD-002](../14-prep/mock-interview/bank/build-systems.md) | PUBLIC, PRIVATE, INTERFACE trong target_link_libraries/target_include_directories nghĩa là gì? |
| [BLD-001](../14-prep/mock-interview/bank/build-systems.md) | Modern CMake (target-based) khác cách cũ ở điểm nào và vì sao tốt hơn? |
| [BLD-012](../14-prep/mock-interview/bank/build-systems.md) | find_package làm gì? Có những cách nào để dùng thư viện ngoài trong CMake? |

---
⬅️ [makefile.md](makefile.md) · ➡️ Tiếp theo: [cross-compilation.md](cross-compilation.md)
