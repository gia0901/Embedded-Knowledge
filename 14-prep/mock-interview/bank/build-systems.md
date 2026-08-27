# BLD — Build systems (CMake, Yocto, cross-compile, CI)

> Domain `BLD`. Hệ build cho embedded Linux: CMake, Yocto, cross-compile, CI. Nguồn: [06-build-systems](../../../06-build-systems/). Track dùng: `bsp`, `cpp-system`, `melp`. Yocto/BSP-layer chuyên sâu còn ở [BSP-017…019](bsp.md).
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Biên dịch & Make | 4 |
| **B** | CMake | 4 |
| **C** | Cross-compilation | 4 |
| **D** | Yocto | 6 |
| **E** | CI cho embedded | 1 |

---

## A — Biên dịch & Make

#### BLD-016 · 🟢 · concept · 📦 2026-08-13 · [→ makefile](../../../06-build-systems/makefile.md)
**Kể các bước từ file `.cpp` tới file thực thi.**
<details><summary>Đáp án</summary>

```
 main.cpp ─①preprocess─► main.i ─②compile─► main.s ─③assemble─► main.o ─┐
                                                                        ├④link─► a.out
 util.cpp ────────────────────────────────────────────────► util.o ─────┘   + thư viện
```

| # | Bước | Làm gì | Lỗi điển hình ở bước này |
|---|---|---|---|
| **①** | **Preprocess** | Xử lý `#include`, `#define`, `#ifdef` — thuần **văn bản** | `No such file or directory` (sai include path) |
| **②** | **Compile** | Phân tích cú pháp, kiểm kiểu, sinh mã assembly, tối ưu | Lỗi cú pháp, sai kiểu, **cảnh báo** |
| **③** | **Assemble** | Assembly → mã máy, tạo **object file** kèm **bảng symbol** | Hiếm |
| **④** | **Link** | Ghép các `.o` + thư viện, **phân giải symbol**, gán địa chỉ | ⭐ **undefined reference**, **multiple definition** |

**⭐ Điểm quan trọng nhất:** mỗi `.cpp` được biên dịch **hoàn toàn độc lập** thành một *đơn vị dịch*. Compiler xử lý `main.cpp` **không biết gì** về nội dung `util.cpp` — nó chỉ tin vào **khai báo** trong header. Việc nối chúng lại là của **linker**.

⇒ Từ đó suy ra hai điều hay bị hỏi:
- **`undefined reference` là lỗi LINK**, không phải lỗi compile: khai báo có (nên compile qua), nhưng không ai **định nghĩa** ([SD-026](system-design.md)).
- **Template phải định nghĩa trong header**, vì đơn vị dịch đang dùng nó cần thấy toàn bộ thân hàm để sinh mã ([CPP-009](cpp.md)).

**Chốt:** *"Preprocess → compile → assemble → link. Mỗi `.cpp` là một đơn vị dịch độc lập, chỉ tin vào header — nên undefined reference là lỗi của bước link chứ không phải compile."*
</details>

#### BLD-017 · 🟢 · concept · 📦 2026-08-13 · [→ makefile](../../../06-build-systems/makefile.md)
**Include guard / `#pragma once` để làm gì? Nó KHÔNG giải quyết được chuyện gì?**
<details><summary>Đáp án</summary>

**Vấn đề:** cùng một header bị `#include` **nhiều lần trong một đơn vị dịch** (thường gián tiếp, qua nhiều tầng include) ⇒ định nghĩa lớp/kiểu bị lặp ⇒ **lỗi biên dịch**.

```cpp
#ifndef MYLIB_WIDGET_H        // include guard — chuẩn, di động
#define MYLIB_WIDGET_H
class Widget { };
#endif

#pragma once                  // ngắn hơn, không lo trùng tên macro; hầu hết compiler hỗ trợ
```

| | **Include guard** | **`#pragma once`** |
|---|---|---|
| Chuẩn hoá | ✅ Chuẩn C++ | ❌ Mở rộng (nhưng hầu như ở đâu cũng có) |
| Rủi ro | **Trùng tên macro** giữa hai header ⇒ một cái bị nuốt im lặng | Có thể nhầm khi cùng file tới qua **hai đường dẫn khác nhau** (symlink, hard link) |

**⚠️ Điều nó KHÔNG giải quyết — chỗ hay hiểu nhầm nhất:**

Guard chỉ có tác dụng **trong MỘT đơn vị dịch**. Nó **không** ngăn hai file `.cpp` khác nhau cùng sinh ra một định nghĩa ⇒ vẫn **`multiple definition`** ở bước **link**:

```cpp
// widget.h — có guard đầy đủ
int counter = 0;              // ❌ ĐỊNH NGHĨA biến trong header
void helper() { }             // ❌ ĐỊNH NGHĨA hàm không inline
```
`a.cpp` và `b.cpp` cùng include ⇒ **hai** định nghĩa `counter` ⇒ linker báo lỗi, dù guard hoạt động hoàn hảo.

**Sửa:** `extern int counter;` ở header + định nghĩa ở **đúng một** `.cpp`; hàm trong header thì đánh dấu **`inline`**; C++17 có `inline` cho cả biến ([SD-026](system-design.md)).

**Chốt:** *"Guard chống include lặp trong **một** đơn vị dịch, không chống định nghĩa trùng giữa các file — nên header phải khai báo chứ đừng định nghĩa."*
</details>

#### BLD-018 · 🟡 · concept · 📦 2026-08-13 · [→ makefile](../../../06-build-systems/makefile.md)
**Make quyết định build lại cái gì dựa vào đâu? Vì sao incremental build đôi khi SAI, và sửa thế nào?**
<details><summary>Đáp án</summary>

**Cơ chế: so sánh THỜI GIAN SỬA (mtime).** Make dựng đồ thị phụ thuộc từ các luật `đích: nguồn`; nếu **nguồn mới hơn đích** ⇒ chạy lại lệnh. Nhờ vậy sửa một file chỉ build lại phần liên quan thay vì toàn bộ.

**⚠️ Bốn ca incremental build cho kết quả SAI:**

| Ca | Vì sao |
|---|---|
| ⭐ **Thiếu phụ thuộc header** | Luật chỉ khai `main.o: main.cpp`, quên `widget.h`. Sửa header ⇒ **make không build lại** ⇒ binary trộn mã cũ và mới ⇒ crash rất khó hiểu |
| **Đổi cờ biên dịch** | Đổi `-O2` sang `-O0` hay thêm `-DDEBUG` **không đổi mtime** của file nguồn ⇒ make tưởng không có gì đổi |
| **Đồng hồ lệch** | Chép file từ máy khác, hoặc mount mạng lệch giờ ⇒ file mới lại có mtime **cũ hơn** |
| **Sinh file bằng script** không khai vào đồ thị | Make không biết nó tồn tại |

⇒ **Ca đầu nguy hiểm nhất** vì nó cho ra một binary "nửa cũ nửa mới" — bố cục struct trong hai object file **không khớp nhau**, hỏng đúng kiểu ABI break ([SD-017](system-design.md)).

**Cách xử lý:**
1. **Sinh phụ thuộc header tự động** — trình biên dịch có thể xuất ra danh sách header đã dùng để make nạp vào. Đây là việc **bắt buộc** với Makefile viết tay; hệ thống build hiện đại (CMake/Ninja) làm sẵn.
2. **Đưa cờ biên dịch vào đồ thị phụ thuộc** (ghi cờ ra một file, coi file đó là phụ thuộc) — hoặc dùng thư mục build riêng cho mỗi cấu hình ([BLD-011](build-systems.md)).
3. **Khi nghi ngờ thì build sạch.** Và nếu một bug chỉ biến mất sau khi build sạch ⇒ đó **chính là** dấu hiệu đồ thị phụ thuộc đang sai — đừng bỏ qua, hãy đi sửa nó.

**Chốt:** *"Make so mtime theo đồ thị phụ thuộc. Nó sai khi đồ thị thiếu — nhất là thiếu header — cho ra binary nửa cũ nửa mới. Bug biến mất sau khi build sạch là dấu hiệu đồ thị phụ thuộc có vấn đề."*
</details>

#### BLD-019 · 🟡 · concept · 📦 2026-08-13 · [→ makefile](../../../06-build-systems/makefile.md)
**Khi nào nên viết Makefile tay, khi nào dùng CMake?**
<details><summary>Đáp án</summary>

| | **Makefile viết tay** | **CMake** |
|---|---|---|
| Hợp với | Dự án **nhỏ**, một nền tảng, ít file; hoặc **bare-metal** cần kiểm soát chính xác từng cờ và bước linker | Dự án **nhiều nền tảng**, nhiều phụ thuộc, cần IDE, cần cross-compile |
| Cross-compile | Tự lo hết | **Toolchain file** ([BLD-003](build-systems.md)) |
| Tìm thư viện | Tự lo | `find_package` ([BLD-012](build-systems.md)) |
| Phụ thuộc header | **Phải tự sinh** — quên là build sai ([BLD-018](build-systems.md)) | Tự động |
| Đường cong học | Thấp lúc đầu, **dốc dần** khi dự án lớn | Cao lúc đầu, phẳng về sau |
| Minh bạch | **Thấy chính xác lệnh chạy** | Có một tầng trừu tượng ở giữa |

**Chọn thế nào — hỏi ba câu:**
1. *"Có nhiều hơn một nền tảng/toolchain không?"* — có ⇒ **CMake**.
2. *"Có phụ thuộc bên ngoài không?"* — có ⇒ **CMake**.
3. *"Cần kiểm soát chính xác từng bước (linker script, section, thứ tự đặc biệt) không?"* — có, và chỉ một nền tảng ⇒ **Makefile** vẫn hợp lý (rất phổ biến trong firmware bare-metal).

⚠️ **Sai lầm hay gặp theo cả hai chiều:** (1) dùng Makefile tay cho dự án đã có 5 phụ thuộc và 3 nền tảng ⇒ tự viết lại một CMake tồi; (2) dùng CMake cho một firmware 10 file bare-metal ⇒ thêm một tầng trừu tượng che mất thứ bạn cần kiểm soát.

**Điểm chung quan trọng nhất, dù chọn cách nào:** build phải **tái lập được** — cùng nguồn + cùng toolchain ⇒ cùng kết quả, trên máy bất kỳ. Cách đảm bảo là **đóng băng toolchain** (container/SDK) và **commit cấu hình build**, chứ không phải chọn công cụ nào ([BLD-010](build-systems.md)).

**Chốt:** *"Nhiều nền tảng hoặc nhiều phụ thuộc ⇒ CMake. Một nền tảng và cần kiểm soát chính xác từng cờ (bare-metal) ⇒ Makefile vẫn tốt. Quan trọng hơn cả hai là build phải tái lập được."*
</details>

---
⬅️ [Bank index](README.md)

---

## B — CMake

#### BLD-011 · 🟢 · concept · 📦 2026-08-13 · [→ cmake](../../../06-build-systems/cmake.md)
**Hai bước configure và build trong CMake là gì? Out-of-source build nghĩa là gì và vì sao nên dùng?**
<details><summary>Đáp án</summary>

**CMake không phải build system — nó là bộ SINH RA build system.** Nên luôn có hai bước:

| Bước | Làm gì | Chạy lại khi nào |
|---|---|---|
| **① Configure** | Đọc `CMakeLists.txt`, dò trình biên dịch và thư viện, **sinh ra** file build thật (Makefile, Ninja, project IDE) | Khi đổi `CMakeLists.txt` hoặc đổi tuỳ chọn |
| **② Build** | Chạy build system vừa sinh ra để biên dịch | Mỗi lần sửa mã nguồn |

**Out-of-source build:** mọi thứ sinh ra nằm trong **thư mục build riêng**, tách khỏi mã nguồn.

**Bốn lý do nên dùng (và là mặc định của mọi dự án nghiêm túc):**
1. **Xoá sạch = xoá một thư mục** — không có thứ rác nào lẫn vào cây nguồn.
2. **Nhiều cấu hình song song** — Debug, Release, và **một thư mục build cho mỗi target cross-compile**, tất cả cùng tồn tại từ một cây nguồn. Với embedded đây là lý do quan trọng nhất.
3. **Cây nguồn sạch** ⇒ `git status` không ngập file sinh ra.
4. Cây nguồn có thể để **chỉ đọc** (build từ nguồn dùng chung).

⚠️ **Bẫy:** lỡ chạy in-source một lần thì cây nguồn sinh ra `CMakeCache.txt` và `CMakeFiles/`; các lần build out-of-source sau đó có thể **nhặt nhầm cache cũ** và lỗi rất khó hiểu ⇒ phải xoá hẳn hai thứ đó.

⚠️ **Bẫy thứ hai:** đổi biến cấu hình (vd toolchain) trên một thư mục build **đã configure rồi** thường không ăn — cache giữ giá trị cũ. Đổi toolchain thì **tạo thư mục build mới**, đừng cố sửa cái cũ.

**Chốt:** *"CMake sinh ra build system rồi mới build — hai bước. Build ngoài cây nguồn để xoá sạch dễ, giữ nguồn sạch, và chạy song song nhiều cấu hình/nhiều target cross-compile."*
</details>

#### BLD-001 · 🟡 · concept · [→ cmake](../../../06-build-systems/cmake.md)
**CMake là gì, khác Make thế nào? "Modern CMake" nghĩa là gì?**
<details><summary>Đáp án</summary>

`make` chạy trực tiếp một Makefile (luật + lệnh). **CMake** là **meta-build**: mô tả dự án ở mức cao (`CMakeLists.txt`) rồi **sinh** build system cho generator chọn (Makefiles, Ninja, IDE) → portable đa nền tảng/compiler. **Modern CMake** (≥3.x) = **target-based**: mọi thứ gắn vào **target** (`add_library`/`add_executable`) qua `target_link_libraries`, `target_include_directories`, `target_compile_features` với phạm vi `PUBLIC/PRIVATE/INTERFACE` — thay cho biến toàn cục cũ (`include_directories`, `CMAKE_CXX_FLAGS`). Dependency và cờ **lan truyền theo target** (usage requirements), sạch và ít lỗi hơn.
</details>

#### BLD-002 · 🟡 · concept · [→ cmake](../../../06-build-systems/cmake.md)
**`target_link_libraries` với `PUBLIC` / `PRIVATE` / `INTERFACE` khác nhau gì?**
<details><summary>Đáp án</summary>

Chỉ **phạm vi lan truyền usage requirement** (include dir, define, link) sang target khác dùng target này: **`PRIVATE`** — chỉ dùng khi *build chính target này*, không lan cho consumer (vd lib phụ chỉ dùng nội bộ). **`INTERFACE`** — không dùng để build target này nhưng **lan cho consumer** (header-only lib). **`PUBLIC`** — cả hai (target dùng *và* consumer cũng cần — vd lib xuất hiện trong header public của bạn). Đặt đúng phạm vi tránh "leak" dependency và giảm rebuild.
</details>

#### BLD-012 · 🟡 · concept · 📦 2026-08-13 · [→ cmake](../../../06-build-systems/cmake.md)
**`find_package` làm gì? Có những cách nào để dùng thư viện ngoài trong CMake, và chọn thế nào?**
<details><summary>Đáp án</summary>

**`find_package` đi TÌM một thư viện đã cài sẵn** trên máy build và khai báo nó thành **target** để bạn liên kết vào.

**Bốn cách dùng thư viện ngoài — chọn theo *ai kiểm soát môi trường build*:**

| Cách | Thư viện đến từ đâu | Hợp khi |
|---|---|---|
| **`find_package`** | Đã cài sẵn trên hệ thống / trong sysroot | Thư viện phổ biến, môi trường build được kiểm soát (Yocto SDK, container) |
| **Lấy về lúc configure** (`FetchContent`) | Tải mã nguồn và build cùng dự án | Thư viện nhỏ, muốn build **tái lập được** không phụ thuộc máy |
| **Submodule + `add_subdirectory`** | Nằm trong repo | Muốn khoá đúng phiên bản, sửa được |
| **Trình quản lý gói** (Conan, vcpkg) | Kho gói riêng | Dự án lớn, nhiều phụ thuộc |

**⭐ Điểm quan trọng nhất khi cross-compile:** `find_package` phải tìm trong **sysroot của target**, **không phải** thư viện của máy build. Sai chỗ này thì bạn liên kết nhầm thư viện x86 vào firmware ARM — và lỗi thường chỉ lộ ra **lúc chạy trên bo**, hoặc lộ ra bằng thông báo rất khó hiểu lúc link ([BLD-014](build-systems.md)).

**Nguyên tắc dùng kết quả:** luôn liên kết bằng **target** (`Foo::Foo`) thay vì biến đường dẫn/cờ rời rạc — target mang theo cả include path và phụ thuộc bắc cầu, đúng tinh thần modern CMake ([BLD-002](build-systems.md)).

⚠️ **Bẫy:** không kiểm tra `find_package` có thành công không ⇒ build đi tiếp rồi hỏng ở chỗ khác; nên khai báo **REQUIRED** để fail ngay và fail rõ.

**Chốt:** *"`find_package` tìm thư viện đã cài và biến nó thành target để liên kết. Khi cross-compile, điều quan trọng nhất là nó phải tìm trong sysroot của target chứ không phải máy build."*
</details>

---

## C — Cross-compilation

#### BLD-013 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ cross-compilation](../../../06-build-systems/cross-compilation.md)
**Cross-compilation là gì, vì sao embedded gần như luôn cần? Phân biệt build / host / target machine.**
<details><summary>Đáp án</summary>

**Cross-compilation:** biên dịch trên một máy, để chạy trên **kiến trúc khác**. Ví dụ điển hình: build trên PC x86-64, chạy trên bo ARM.

**Vì sao embedded gần như luôn cần:**
1. **Thiết bị quá yếu** để tự biên dịch — vài trăm MB RAM, CPU chậm; build kernel trên bo có thể mất **hàng chục giờ** thay vì vài phút.
2. **Thiết bị không có toolchain** — rootfs sản phẩm tối giản, không chứa compiler (và **không nên** chứa, vì lý do an ninh và dung lượng).
3. **Quy trình phát triển** — sửa code trên PC, build trong vài giây, nạp xuống bo.

**Ba "machine" — thuật ngữ hay bị lẫn:**

| Tên | Là gì | Ví dụ |
|---|---|---|
| **build** | Máy **chạy compiler** | PC x86-64 của bạn |
| **host** | Máy mà **chương trình sinh ra sẽ chạy** | Bo ARM |
| **target** | Chỉ có nghĩa khi bạn build **chính compiler**: máy mà **compiler đó sẽ sinh mã cho** | ARM |

⇒ Với người dùng thông thường chỉ cần **build ≠ host** là đã cross-compile. Chữ *target* chỉ dùng khi làm **toolchain** (build một compiler chạy trên A sinh mã cho B) — đây là chỗ hay trả lời nhầm.

**Toolchain cross gồm gì:** compiler + linker + thư viện chuẩn **của target** + **sysroot** (bản sao header và thư viện của hệ thống target — [BLD-014](build-systems.md)). Tên toolchain thường mã hoá kiến trúc/hệ/ABI, ví dụ `arm-linux-gnueabihf-` cho ARM/Linux/hard-float.

**Chốt:** *"Cross-compile là build trên kiến trúc này để chạy trên kiến trúc khác — bắt buộc với embedded vì thiết bị quá yếu và không có toolchain. build = máy chạy compiler, host = máy chạy chương trình; target chỉ có nghĩa khi bạn đang build chính compiler."*
</details>

#### BLD-014 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ cross-compilation](../../../06-build-systems/cross-compilation.md)
**Sysroot là gì và vì sao nó quan trọng khi cross-compile?**
<details><summary>Đáp án</summary>

**Sysroot là bản sao thư mục hệ thống của TARGET** (`/usr/include`, `/usr/lib`, `/lib`…) nằm trên **máy build**. Compiler được bảo *"coi thư mục này là `/` khi đi tìm header và thư viện"*.

**⭐ Vì sao bắt buộc:** header và thư viện **của máy build là của x86**. Không có sysroot thì compiler sẽ:
- Nhặt `/usr/include` của PC ⇒ header **sai kiến trúc**, sai phiên bản libc, sai kích thước kiểu.
- Liên kết thư viện **x86** vào chương trình ARM ⇒ hoặc lỗi lúc link (thường khó hiểu), hoặc tệ hơn: **link được** nhưng chạy trên bo thì lỗi tuỳ lúc.

⇒ Sysroot **cách ly** hoàn toàn môi trường target khỏi môi trường build. Trong Yocto/Buildroot, sysroot được sinh ra tự động và SDK cấu hình sẵn cho bạn.

**⚠️ Triệu chứng kinh điển khi sysroot sai hoặc thiếu:**

| Triệu chứng | Nguyên nhân |
|---|---|
| Chương trình chạy trên bo báo *"not found"* dù **file có ở đó** | Thiếu **dynamic loader** đúng kiến trúc, hoặc binary build cho ABI khác ⇒ thông báo nói về **loader**, không phải về file của bạn ([BSP-019](bsp.md)) |
| Link lỗi *"incompatible architecture"* | Nhặt thư viện x86 |
| Build được nhưng bo **crash ngay khi khởi động** | Header lệch phiên bản với thư viện thật trên bo |

**Nguyên tắc thực dụng:** đừng tự lắp sysroot bằng tay — dùng **SDK do hệ thống build sinh ra** (Yocto SDK/eSDK, hoặc Buildroot toolchain), vì nó đảm bảo header, thư viện và trình biên dịch **khớp đúng** với ảnh hệ thống đang chạy trên bo ([BLD-009](build-systems.md)).

**Chốt:** *"Sysroot là bản sao `/usr/include` và `/usr/lib` của target đặt trên máy build, để compiler không nhặt nhầm header và thư viện x86. Thiếu nó thì hoặc link lỗi, hoặc tệ hơn là link được rồi hỏng trên bo."*
</details>

#### BLD-003 · 🟠 · concept · ⭐ · [→ cross-compilation](../../../06-build-systems/cross-compilation.md)
**Cross-compile với CMake làm thế nào? Toolchain file chứa gì?**
<details><summary>Đáp án</summary>

**Vấn đề gốc của mọi cross-compile — một câu hỏi duy nhất, lặp lại ở mọi bước: "thứ này đến từ *host* hay từ *target sysroot*?"** Compiler, header, thư viện, công cụ phụ trợ, kết quả `find_package` — mỗi thứ đều có thể lấy nhầm bên, và nhầm thì lỗi hoặc là "wrong architecture" lúc link, hoặc tệ hơn: **build sạch nhưng crash trên board**.

**Cách làm:** không sửa `CMakeLists.txt`, mà truyền **toolchain file** lúc configure — giữ project không biết gì về target cụ thể.

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake
```

```cmake
# arm-toolchain.cmake
set(CMAKE_SYSTEM_NAME      Linux)          # ⭐ đặt biến này = báo CMake "đang cross-compile"
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_SYSROOT      /path/to/target-sysroot)   # header + lib CỦA TARGET

# Chặn find_* lôi nhầm đồ của host:
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BEFORE)  # chương trình: chạy trên HOST -> lấy host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # thư viện: CHỈ trong sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # header:   CHỈ trong sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

| Thành phần | Vai trò |
|---|---|
| `CMAKE_SYSTEM_NAME` | Bật chế độ cross (`CMAKE_CROSSCOMPILING` = TRUE) — thiếu nó thì mọi thứ khác vô nghĩa |
| `CMAKE_SYSROOT` | Cây `/usr` của board: nơi tìm header/lib **của target** |
| `CMAKE_FIND_ROOT_PATH_MODE_*` | ⭐ Tách "công cụ chạy trên host" khỏi "thư viện link cho target". `PROGRAM` phải lấy **host** (protoc, flex…), `LIBRARY/INCLUDE` phải **ONLY** sysroot |

**Với Yocto:** không tự viết — SDK sinh sẵn toolchain file + môi trường, chỉ cần `source environment-setup-<arch>-poky-linux` rồi cmake bình thường.

**Bẫy:** (1) quên `CMAKE_SYSTEM_NAME` → CMake tưởng build native, `try_run()` và mọi kiểm tra tính năng chạy trên host cho kết quả **sai**; (2) `find_package` lấy `.so` của host → link "thành công" rồi lỗi kiến trúc; đó chính là lý do có `FIND_ROOT_PATH_MODE`; (3) **cache CMake cũ** — đổi toolchain file phải **xoá thư mục build**, không thì nó giữ compiler cũ; (4) chạy trên board báo `not found` dù file có thật → thường là thiếu **dynamic loader**/lib đúng ABI, kiểm bằng `file` và `readelf -l` ([BSP-019](bsp.md)); (5) chương trình sinh code chạy lúc build (code generator) phải build **cho host**, không phải target — dấu hiệu cần chia hai bước build.

**Chốt:** *"Toolchain file trả lời 'host hay sysroot?' một lần cho toàn dự án: compiler + sysroot + `FIND_ROOT_PATH_MODE`. Project không cần biết mình đang được cross-compile."*
</details>

#### BLD-015 · 🟠 · concept · 📦 2026-08-13 · [→ cross-compilation](../../../06-build-systems/cross-compilation.md)
**Những khó khăn đặc thù khi cross-compile là gì?**
<details><summary>Đáp án</summary>

| Khó khăn | Vì sao xảy ra | Cách xử lý |
|---|---|---|
| ⭐ **Không CHẠY được thứ vừa build** | Nhiều hệ thống build muốn chạy thử một chương trình nhỏ để dò tính năng (*configure test*) — mà binary ARM không chạy trên PC | Khai báo sẵn kết quả dò tính năng; dùng toolchain file/SDK đã cấu hình |
| **Công cụ build cho chính máy build** | Dự án sinh ra một công cụ rồi dùng nó trong quá trình build (bộ sinh mã, trình đóng gói) — công cụ đó phải là **x86**, còn sản phẩm là **ARM** ⇒ **hai toolchain trong một lần build** | Tách rõ "native tool" và "target binary"; Yocto có khái niệm riêng cho việc này |
| **Nhặt nhầm thư viện của máy build** | Thiếu sysroot hoặc đường dẫn tìm kiếm sai | [BLD-014](build-systems.md) |
| **Khác biệt kiến trúc âm thầm** | Kích thước kiểu, **căn lề**, **endianness**, `char` mặc định có dấu hay không, FPU/soft-float | Dùng kiểu độ rộng cố định ([EMB-002](embedded-fundamentals.md)); test trên bo thật, đừng tin test trên PC |
| **Không debug được như thường** | gdb chạy trên PC, chương trình chạy trên bo | Remote debug qua gdbserver; hoặc dựa vào log + core dump ([DBG-018](debugging.md)) |
| **Tái lập build** | "Chạy trên máy tôi" — vì mỗi người một toolchain, một sysroot | Đóng băng toolchain trong container/SDK và commit cấu hình |

**⭐ Bài học chung:** phần lớn khó khăn không nằm ở việc **biên dịch**, mà nằm ở chỗ **hệ thống build ngầm giả định "build và chạy trên cùng một máy"**. Mỗi chỗ giả định đó là một chỗ vỡ.

⚠️ **Cạm bẫy nguy hiểm nhất là nhóm "khác biệt kiến trúc âm thầm"** — nó **không** gây lỗi build, mà tạo ra bug chỉ xuất hiện trên bo: mô hình bộ nhớ ARM lỏng hơn x86 nên **race ẩn trên PC sẽ lộ trên bo** ([CPP-019](cpp.md)), và dữ liệu nhị phân trao đổi giữa hai bên phải để ý endianness và padding.

**Chốt:** *"Khó khăn thật không phải ở việc biên dịch mà ở chỗ hệ thống build giả định build và chạy trên cùng máy — không chạy được thứ vừa build, cần hai toolchain, dễ nhặt nhầm thư viện. Và nguy hiểm nhất là khác biệt kiến trúc âm thầm, vì nó chỉ lộ trên bo."*
</details>

---

## D — Yocto

#### BLD-004 · 🟡 · concept · [→ yocto §2](../../../06-build-systems/yocto.md)
**Một Yocto recipe (`.bb`) gồm những gì?**
<details><summary>Đáp án</summary>

Công thức build **một package**:

```bitbake
# sensord_1.2.bb
SUMMARY  = "Ambient light sensor daemon"
LICENSE  = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0835ade..."   # doi noi dung license -> BUILD FAIL
SRC_URI  = "git://git.example.com/sensord.git;branch=main;protocol=https \
            file://0001-fix-i2c-timeout.patch"
SRCREV   = "a1b2c3d4"        # ⭐ ghim commit; ${AUTOREV} lam MAT tai lap
S        = "${WORKDIR}/git"
DEPENDS  = "libgpiod"        # luc BUILD
RDEPENDS:${PN} = "i2c-tools" # luc CHAY
inherit cmake systemd        # muon hanh vi build chuan
```

**Task chạy theo thứ tự** — biết task nào hỏng là biết nghi gì:

| Task | Làm gì | Hỏng vì |
|---|---|---|
| `do_fetch` → `do_unpack` → `do_patch` | Tải, giải nén, áp patch | `SRCREV` sai · patch không áp được sau khi vendor nâng version |
| `do_configure` → `do_compile` | Cấu hình, biên dịch | Thiếu **`DEPENDS`** ⇒ không thấy header |
| `do_install` | Cài vào **`${D}`** | Quên `${D}` ⇒ ghi ra máy host |
| `do_package` | Chia `${D}` thành `${PN}`, `-dev`, `-dbg`, `-doc` | File rơi vào gói sai ⇒ image thiếu |

📌 **`${D}` KHÔNG phải rootfs** — nó là thư mục dàn dựng riêng của recipe. Rootfs ghép **sau**, từ các package đã đóng gói. Hiểu nhầm chỗ này sinh ra lỗi *"tôi cài rồi mà image không có"*.

`inherit` cho hành vi build chuẩn (`autotools`, `cmake`, `meson`, `kernel`, `module`, `systemd`) thay vì tự viết `do_configure`/`do_compile`. BitBake dựng **task graph** từ dependency giữa các recipe rồi chạy song song.
</details>

#### BLD-005 · 🟡 · concept · ⭐ · [→ yocto §2](../../../06-build-systems/yocto.md)
**Layer và `.bbappend` là gì? Vì sao không sửa recipe gốc?**
<details><summary>Đáp án</summary>

**Layer (`meta-*`)** = một tập recipe + conf đóng gói theo mối quan tâm, có **priority**, bật trong `bblayers.conf`:

| Loại layer | Ai giữ | Ví dụ |
|---|---|---|
| **BSP layer** (`meta-<board>`) | Vendor SoC (NXP/TI/ST) | kernel + u-boot bbappend, machine conf, firmware |
| **Distro layer** | Tổ chức bạn | chính sách chung: init system, libc, feature |
| **App/software layer** (`meta-<sản phẩm>`) | **Bạn** | recipe ứng dụng + **mọi tuỳ biến** |

**`.bbappend`** = file mở rộng/sửa một recipe **thuộc layer khác** mà **không đụng vào file gốc**. Tên phải khớp recipe (`linux-imx_%.bbappend` — `%` khớp mọi version).

```
meta-myproduct/
└── recipes-kernel/linux/
    ├── linux-imx_%.bbappend        # ⭐ mở rộng recipe kernel của vendor
    └── linux-imx/
        ├── 0001-add-my-driver.patch
        └── my-feature.cfg          # config fragment: bật CONFIG_*
```
```bitbake
# linux-imx_%.bbappend
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-add-my-driver.patch \
            file://my-feature.cfg"
```

**Vì sao tuyệt đối không sửa recipe gốc — ba lý do, lý do (3) mới là lý do thật:**
1. **Mất khi cập nhật** — vendor phát hành BSP mới, bạn `git pull`/thay layer → tuỳ biến bay sạch hoặc xung đột.
2. **Không tái lập được** — người khác clone repo không có sửa đổi của bạn; build của bạn ≠ build của CI.
3. ⭐ **Không tách được "cái của mình" khỏi "cái của vendor"** — khi nâng cấp BSP (hoặc lên kernel mới), bạn cần trả lời *"tôi đã đổi những gì?"*. Với bbappend, câu trả lời là **một danh sách patch tường minh** trong layer của bạn. Với sửa trực tiếp, bạn phải `diff` cả cây và đoán.

**Bẫy:** (1) ⚠️ sửa trong **`tmp/work/...`** để "thử nhanh" rồi quên — bị **nghiền sạch** lần build sau, và nhiều giờ debug đi theo; muốn thử nhanh thì dùng **`devtool modify <recipe>`** rồi **`devtool finish`** để đẩy thành bbappend/patch; (2) bbappend **không khớp version** recipe → BitBake **âm thầm bỏ qua** (dùng `%` hoặc kiểm bằng `bitbake-layers show-appends`); (3) quên `FILESEXTRAPATHS:prepend` → không tìm thấy file patch; (4) sửa `local.conf` thay vì layer — tiện nhưng cũng không tái lập được cho team/CI.

**Chốt:** *"Layer là đơn vị đóng gói và chia trách nhiệm; bbappend là cách sửa đồ của người khác mà không chạm vào nó. Mọi tuỳ biến nằm trong layer của bạn — để nâng cấp BSP vendor không xoá mất công sức."*
</details>

#### BLD-007 · 🟡 · concept · [→ yocto §6](../../../06-build-systems/yocto.md)
**`devtool` dùng để làm gì?**
<details><summary>Đáp án</summary>

Nó thay **vòng lặp phát triển**, không chỉ là một lệnh tiện tay.

**Vòng lặp cũ** *(chậm và dễ sai)*: sửa recipe → `cleansstate` → `bitbake` → đợi → flash image → thử → lặp.
⚠️ Vì chậm nên người ta hay "thử nhanh" bằng cách sửa thẳng trong **`tmp/work/`** — bị nghiền sạch lần build sau, và sstate **không nhận ra** nên build vẫn "thành công" với code cũ ([BLD-006](build-systems.md)).

**Vòng lặp với `devtool`:**
```bash
devtool modify sensord                  # keo source ra workspace/, thanh git repo THAT
#   ... sua code, commit tung buoc nhu binh thuong ...
devtool build sensord                   # build lai, chi phan doi
devtool deploy-target sensord root@192.168.1.10   # ⭐ day thang len board dang chay
devtool finish sensord ../meta-myproduct          # goi thanh patch + bbappend trong layer
```

⭐ **`deploy-target` là thứ đổi hẳn nhịp làm việc**: sửa code → thấy trên board trong **vài giây**, không dựng lại image, không flash thẻ.
⭐ **`finish` là thứ giữ kỷ luật**: nó biến các commit của bạn thành **patch + bbappend trong layer riêng** — tức đúng cách tuỳ biến mà [BLD-005](build-systems.md) yêu cầu, tự động.

Còn có `devtool add <url>` (sinh recipe mới từ source) và `devtool upgrade` (nâng version).

**⚠️ Bẫy:** (1) quên `devtool finish` ⇒ thay đổi sống trong `workspace/`, **không vào git của layer**, đồng nghiệp clone về không có; (2) `devtool reset` trước khi finish ⇒ mất công sức; (3) tưởng `deploy-target` đã cập nhật image — **không**, image chỉ đổi sau khi `finish` + build lại.
</details>

#### BLD-009 · 🟡 · concept · [→ yocto §6](../../../06-build-systems/yocto.md)
**Yocto SDK / eSDK để làm gì?**
<details><summary>Đáp án</summary>

`bitbake <image> -c populate_sdk` sinh **SDK** = toolchain cross + **sysroot** (headers/lib target) đóng gói cho **đội app** build phần mềm **ngoài Yocto** (không cần cả build tree): `source environment-setup-*` rồi `cmake`/`make` như thường, đúng target. **eSDK** (extensible SDK) còn kèm **devtool** để thêm/sửa recipe và tích hợp ngược vào build. Đây là cách tách vai: đội BSP dựng distro, đội app dùng SDK.
</details>

#### BLD-008 · 🟠 · concept · [→ yocto §5](../../../06-build-systems/yocto.md)
**Thêm một package vào image thế nào? `IMAGE_INSTALL` vs `RDEPENDS` khác gì?**
<details><summary>Đáp án</summary>

**Cơ chế — hai biến trả lời hai câu hỏi KHÁC nhau:**

| | `IMAGE_INSTALL` | `RDEPENDS` |
|---|---|---|
| Trả lời câu | *"**Image này** cần có gì?"* | *"**Package này** không chạy được nếu thiếu gì?"* |
| Viết ở đâu | **Image recipe** (`core-image-*.bb`) hoặc `local.conf` (chỉ để thử) | **Recipe của package đó** |
| Phạm vi | Đúng **một image** | **Mọi** image có chứa package đó |
| Ai đọc | Bước dựng rootfs | Package manager, khi giải dependency |

```bitbake
# image recipe — "toi muon image nay co sensord va i2c-tools"
IMAGE_INSTALL:append = " sensord i2c-tools"

# recipe cua sensord — "sensord KHONG CHAY DUOC neu thieu libfoo"
RDEPENDS:${PN} += "libfoo"
```

**⭐ "Vì sao" — vì sao không dùng `IMAGE_INSTALL` cho tất cả cho gọn?**

Vì `IMAGE_INSTALL` **không mang theo tri thức**. Khi bạn khai `RDEPENDS:${PN} += "libfoo"`, bạn đang ghi lại một **sự thật về package** — và sự thật đó đi theo package tới **mọi** image, mọi sản phẩm, mọi người dùng lại recipe của bạn. Khai ở `IMAGE_INSTALL` thì sự thật đó chỉ tồn tại trong **một** image; image thứ hai quên khai là hỏng, và **hỏng lúc chạy trên thiết bị**, không phải lúc build.

⇒ Quy tắc quyết định:

| Bạn đang nói gì | Dùng |
|---|---|
| *"Sản phẩm này cần công cụ X"* (chủ ý sản phẩm) | `IMAGE_INSTALL` |
| *"A cần B mới chạy được"* (sự thật kỹ thuật) | `RDEPENDS` |

**Ba cách thêm gói, theo mức độ bền vững:**

```bitbake
# 1. local.conf  -> CHI DE THU. Khong tai lap duoc tren CI/may dong nghiep
IMAGE_INSTALL:append = " strace"

# 2. image recipe rieng cua ban  -> ✅ cach dung cho san pham
require recipes-core/images/core-image-minimal.bb
IMAGE_INSTALL:append = " sensord i2c-tools"

# 3. packagegroup  -> ✅ khi nhieu image dung chung mot bo goi
IMAGE_INSTALL:append = " packagegroup-myproduct-tools"
```

**⚠️ Bẫy:** (1) 🔴 nhét `IMAGE_INSTALL` vào **`local.conf`** rồi để đó — chạy trên máy bạn, CI build ra image thiếu gói, và không ai hiểu vì sao; (2) `IMAGE_INSTALL = "..."` thay vì `:append` ⇒ **xoá sạch** danh sách gốc, image mất cả busybox lẫn init; (3) khai `RDEPENDS` cho thứ chỉ **một** sản phẩm cần ⇒ mọi sản phẩm khác cũng phải mang nó; (4) quên `RDEPENDS` cho lib nạp bằng `dlopen` ⇒ compile sạch, chạy mới chết ([BSP-018](bsp.md)).

**Chốt:** *"`IMAGE_INSTALL` là **chủ ý của sản phẩm** — 'image này cần gì'. `RDEPENDS` là **sự thật về package** — 'A không chạy được nếu thiếu B'. Sự thật thì viết vào recipe để nó đi theo package; chủ ý thì viết vào image."*
</details>

#### BLD-006 · 🟠 · concept · [→ yocto §3](../../../06-build-systems/yocto.md)
**sstate-cache là gì? Vì sao build "không nhận thay đổi" và cách xử lý?**
<details><summary>Đáp án</summary>

**Cơ chế — hai chữ quyết định tất cả: TASK và HASH.**

sstate cache kết quả của **từng task** (`do_compile`, `do_install`, `do_package`…), **không phải từng recipe**. Mỗi mục cache khoá bằng **hash của mọi đầu vào task đó**: nội dung recipe, giá trị các biến nó đọc, hash của những task nó phụ thuộc, phiên bản class được `inherit`.

Build lần sau: BitBake tính lại hash từng task → hash trùng thì **lấy kết quả từ cache, không chạy lại**; hash khác thì chạy lại task đó **và mọi task phụ thuộc nó**.

**"Vì sao" hai tầng:**
- *Tầng nông* (ai cũng nói): *"nó cache nên build lại nhanh"* — đúng nhưng không dùng được để chẩn đoán.
- ⭐ *Tầng sâu*: vì khoá là **hash đầu vào**, câu hỏi thực dụng **không phải** *"sstate có nhanh không"* mà là ***"thay đổi của tôi làm hỏng hash của bao nhiêu task?"*** — đó mới là thứ quyết định bạn đợi 30 giây hay 2 tiếng.

| Bạn đổi gì | Hỏng bao nhiêu task | Vì sao |
|---|---|---|
| Nội dung một recipe app lá | **Ít** — recipe đó + đóng gói image | Không recipe nào phụ thuộc nó |
| `DISTRO_FEATURES`, `TUNE_FEATURES` | **Gần như tất cả** | Mọi task compile đều đọc biến này ⇒ hash đổi hàng loạt |
| Nâng version toolchain | **Tất cả** | Là gốc của cây phụ thuộc |
| 🔴 Sửa thẳng trong `tmp/work/` | **KHÔNG cái nào** | Hash **không nhìn vào `tmp/work`** ⇒ dùng lại cache cũ ⇒ *"build không nhận thay đổi"* |

**⭐ Đây chính là lời giải cho câu hỏi thứ hai của đề.** Triệu chứng *"tôi sửa rồi mà build không đổi gì"* gần như luôn là **một trong ba ca**:

| Ca | Vì sao hash không đổi | Cách sửa đúng |
|---|---|---|
| Sửa trong `tmp/work/` | Không phải đầu vào của hash | `devtool modify` — nó biến source thành git repo mà hash **có** theo dõi |
| `.bbappend` không khớp version | BitBake **âm thầm bỏ qua**, metadata không đổi | Dùng `%`; kiểm `bitbake-layers show-appends` |
| Cú pháp `_append` cũ trên Yocto ≥ 3.4 | Biến im lặng không tác dụng | Đổi sang `:append` |

**Ba mức dọn — đừng dùng nhầm:**

```bash
bitbake -c clean <recipe>        # xoa tmp/work cua recipe, GIU sstate -> thuong khong du
bitbake -c cleansstate <recipe>  # xoa ca sstate cua recipe   -> dung khi nghi cache cu
rm -rf tmp/                      # xoa thu muc lam viec, GIU sstate-cache/ -> sach ma van nhanh
bitbake -S printdiff <target>    # ⭐ CHAN DOAN: hash nao doi so voi lan truoc
```

**⚠️ Bẫy:** (1) `cleansstate` **toàn bộ** cho chắc — vứt hàng giờ build mà thường chỉ cần một recipe; (2) để `SSTATE_DIR`/`DL_DIR` **bên trong** `build/` ⇒ `rm -rf tmp/` an toàn nhưng lỡ tay xoá `build/` là mất sạch — tách ra ngoài; (3) tưởng sstate làm build **đầu tiên** nhanh — không, nó chỉ giúp từ lần **thứ hai** hoặc khi tải cache dùng chung của CI.

**Chốt:** *"sstate cache theo **task** và khoá theo **hash đầu vào** của task đó. Nên khi build không nhận thay đổi, câu hỏi đúng là 'thay đổi của tôi có nằm trong thứ được băm không?' — sửa trong `tmp/work` thì không, và đó là ca kinh điển."*
</details>

---

## E — CI cho embedded

#### BLD-010 · 🟠 · design · [→ yocto](../../../06-build-systems/yocto.md)
**Thiết kế CI (vd Jenkins) cho một dự án embedded Linux?**
<details><summary>Đáp án</summary>

- **Build**: pipeline chạy `bitbake <image>` với **sstate-cache dùng chung** (mount/copy sstate + downloads giữa các lần build) → build tăng tốc; pin version layer (manifest/`repo`/submodule) để tái lập.
- **Chất lượng**: build cả bản có warning-as-error, chạy static analysis (MISRA/cppcheck), unit test **trên host** (build cùng code, chạy ASan/TSan), license/CVE check (`cve-check`).
- **Test trên target**: sau build, deploy image lên **board thật / QEMU / HIL** (hardware-in-the-loop) chạy smoke test + integration; thu log/artifact.
- **Artifact**: lưu image + SDK + manifest version cho truy vết; tag theo commit.
- Đánh đổi: build Yocto nặng → sstate + build node mạnh; test HIL cần hạ tầng phần cứng. Nêu được "sstate cache để CI không build lại từ đầu" là điểm cộng.
</details>

---

⬅️ [Bank index](README.md)
