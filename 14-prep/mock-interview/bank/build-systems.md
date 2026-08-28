# BLD — Build systems (CMake, Yocto, cross-compile, CI)

> Domain `BLD`. Hệ build cho embedded Linux: CMake, Yocto, cross-compile, **CI/CD & automated test farm**. Nguồn: [06-build-systems](../../../06-build-systems/). Track dùng: `bsp`, `cpp-system`, `melp`. Yocto/BSP-layer chuyên sâu còn ở [BSP-017…019](bsp.md).
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Biên dịch & Make | 4 |
| **B** | CMake | 4 |
| **C** | Cross-compilation | 4 |
| **D** | Yocto | 6 |
| **E** | CI cho embedded & test farm | 18 |

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

## E — CI cho embedded & test farm

#### BLD-030 · 🟢 · concept · [→ ci-and-test-farm §1.1](../../../06-build-systems/ci-and-test-farm.md)
**Continuous Integration là gì, và nó sinh ra để giải vấn đề gì?**
<details><summary>Đáp án</summary>

Gộp code vào nhánh chung **thường xuyên** (≥1 lần/ngày), và mỗi lần gộp có máy **tự build + test**. Giải *integration hell*: để lâu không gộp thì các bản phân kỳ, xung đột dồn thành một đống không gỡ nổi. ⚠️ Chữ *"continuous"* nói về **tần suất gộp**, không phải về việc có Jenkins.
</details>

---

#### BLD-031 · 🟡 · concept · [→ ci-and-test-farm §1.2](../../../06-build-systems/ci-and-test-farm.md)
**Phân biệt CI, Continuous Delivery và Continuous Deployment. Ở embedded thường dừng ở đâu, vì sao?**
<details><summary>Đáp án</summary>

| Thuật ngữ | Tự động tới đâu | Bước cuối do ai |
|---|---|---|
| **CI** — Integration | gộp → build → test | dừng ở đây |
| **CD** — **Delivery** | + đóng gói thành **bản phát hành sẵn sàng** | **người** bấm nút |
| **CD** — **Deployment** | + **tự đưa ra thật** | không ai bấm |

**⭐ Ở embedded "deploy" = OTA xuống thiết bị của khách**, không phải đổi container trên server:

| | Web | Embedded |
|---|---|---|
| Rollback | đổi container, **giây** | phải OTA lần nữa; máy có thể đã **brick** |
| Nơi chạy | máy chủ mình kiểm soát | **thiết bị của khách**, mất điện giữa chừng được |
| Xấu nhất | site lỗi vài phút | **cục gạch**, phải thu hồi |

⇒ Embedded gần như luôn dừng ở **Continuous Delivery**: máy lo tới *"đã ký, đã test, sẵn sàng phát hành"*, còn **phát hành là quyết định của người**, thường theo đợt (canary/staged rollout).

**Chốt:** *"Ở web deploy là thao tác đảo được trong vài giây; ở embedded nó là thứ có thể biến máy của khách thành cục gạch — nên bước cuối luôn có người."*
</details>

---

#### BLD-032 · 🟡 · concept · [→ ci-and-test-farm §1.3](../../../06-build-systems/ci-and-test-farm.md)
**Giải thích các thuật ngữ pipeline: stage, job, runner, artifact, trigger, gate, DUT, HIL — và chúng là cái gì trong ngữ cảnh embedded?**
<details><summary>Đáp án</summary>

| Từ | Nghĩa chung | Ở embedded |
|---|---|---|
| **Pipeline** | chuỗi bước tự động sau một sự kiện | submit → build 10 platform → test trên board |
| **Stage** | nhóm bước, xong mới sang nhóm sau | `build` → `smoke` → `robustness` |
| **Job** | việc chạy độc lập, song song được | "build cho platform A" |
| **Runner/agent** | máy thực thi job | máy build **x86** — **không** chạy được binary ARM |
| **Artifact** | sản phẩm giữ lại sau job | **image**, kernel, SDK, gói `-dbg`, manifest |
| **Trigger** | cái làm pipeline chạy | submit, nightly, gọi tay |
| **Gate** | điều kiện phải pass mới đi tiếp | *"pass hết mới vào source chính"* |
| **DUT** | Device Under Test | chính thiết bị trên bàn |
| **HIL** | Hardware In the Loop | test có **phần cứng thật** trong vòng lặp |

⚠️ **Điểm dễ trả lời hời hợt — artifact ở embedded nặng ký hơn ở web:** thiết bị bán ra sống **5–10 năm**; hai năm sau có lỗi hiện trường thì cần **đúng image đó** và **đúng symbol của bản build đó** để đọc backtrace ([BLD-028](build-systems.md)). Không lưu artifact + manifest version = **không điều tra được**.
</details>

---

#### BLD-033 · 🟡 · concept · [→ ci-and-test-farm §1.5](../../../06-build-systems/ci-and-test-farm.md)
**Unit / integration / system test và smoke / regression / soak test — quan hệ giữa hai bộ khái niệm này là gì?**
<details><summary>Đáp án</summary>

Chúng là **hai trục vuông góc**, không phải hai mức của cùng một thang:

| Trục | Các mức | Trả lời |
|---|---|---|
| **Phạm vi** (test *cái gì*) | unit → integration → system → acceptance | *"kiểm một hàm, hay cả cái TV?"* |
| **Mục đích** (test *để làm gì*) | smoke · functional/robustness · **regression** · soak/stress · performance | *"tôi muốn biết điều gì?"* |

Mỗi bài test có **một toạ độ trên mỗi trục**: *smoke* thường là **system** + mục đích *"còn sống không"*; *regression* có thể là **unit** (chạy lại test cũ sau mỗi sửa) hoặc **system**; *soak* gần như luôn **system**.

**Chốt:** *"'unit/integration/system' nói **phạm vi**, 'smoke/regression/soak' nói **mục đích** — hỏi 'smoke là unit hay system' là hỏi sai trục."*
</details>

---

#### BLD-034 · 🟢 · concept · [→ ci-and-test-farm §1.4](../../../06-build-systems/ci-and-test-farm.md)
**Test farm là gì? DUT và HIL nghĩa là gì?**
<details><summary>Đáp án</summary>

**Test farm**: một nhóm **thiết bị thật** được nối vào hạ tầng điều khiển từ xa (nguồn, serial, cài image, bơm input, thu output) để pipeline **tự chạy test trên phần cứng, không cần người**. **DUT** = Device Under Test, thiết bị đang bị test. **HIL** = Hardware In the Loop: vòng lặp test có phần cứng thật bên trong.
</details>

---

#### BLD-035 · 🟡 · concept · ⭐ · [→ ci-and-test-farm §1.4](../../../06-build-systems/ci-and-test-farm.md)
**Vì sao embedded bắt buộc phải test trên board thật? Test trên host và QEMU không đủ ở chỗ nào — và nên chia việc thế nào?**
<details><summary>Đáp án</summary>

**Bản chất:** phần mềm embedded chỉ **đúng khi đứng cùng phần cứng của nó**. Nhiều lớp lỗi **không thể** lộ trên máy build: timing thật, ngắt thật, tín hiệu thật, panel/cảm biến thật, nhiệt, nguồn.

| Cách chạy | Bắt được | **Không** bắt được |
|---|---|---|
| **Host** (unit test, ASan/TSan) | logic thuần, lỗi bộ nhớ | mọi thứ dính phần cứng |
| **QEMU / mô phỏng** | boot, userspace, phần nhiều kernel | **timing thật**, ngoại vi thật, panel |
| **Board thật (HIL)** | ✅ hành vi thật, **số đo thật** | (đổi lại: chậm, đắt, **board hỏng được**) |

**⭐ Chia việc — đây mới là phần ăn điểm:** **đẩy càng nhiều test xuống host càng tốt** (rẻ, nhanh, không giòn), **để dành farm cho thứ chỉ phần cứng mới trả lời được**. Farm là tài nguyên **đắt nhất** trong pipeline — dùng nó cho việc host làm được là lãng phí, và còn làm hàng đợi gate dài thêm ([BLD-021](build-systems.md)).

⚠️ **Bẫy:** trả lời *"vì kiến trúc ARM khác x86"* là mới chạm nửa vấn đề — QEMU giải được đúng phần đó mà vẫn không thay được board.
</details>

---

#### BLD-010 · 🟠 · design · [→ yocto](../../../06-build-systems/yocto.md)
**Thiết kế CI (vd Jenkins) cho một dự án embedded Linux?**
<details><summary>Đáp án</summary>

- **Build**: pipeline chạy `bitbake <image>` với **sstate-cache dùng chung** (mount/copy sstate + downloads giữa các lần build) → build tăng tốc; pin version layer (manifest/`repo`/submodule) để tái lập.
- **Chất lượng**: build cả bản có warning-as-error, chạy static analysis (MISRA/cppcheck), unit test **trên host** (build cùng code, chạy ASan/TSan), license/CVE check (`cve-check`).
- **Test trên target**: sau build, deploy image lên **board thật / QEMU / HIL** (hardware-in-the-loop) chạy smoke test + integration; thu log/artifact.
- **Artifact**: lưu image + SDK + manifest version cho truy vết; tag theo commit.
- Đánh đổi: build Yocto nặng → sstate + build node mạnh; test HIL cần hạ tầng phần cứng. Nêu được "sstate cache để CI không build lại từ đầu" là điểm cộng.
</details>

#### BLD-020 · 🟡 · concept · [→ ci-and-test-farm §2](../../../06-build-systems/ci-and-test-farm.md)
**CI cho embedded khác CI cho một dịch vụ web ở chỗ nào? Nêu hệ quả hạ tầng của từng khác biệt.**
<details><summary>Đáp án</summary>

Ba khác biệt, mỗi cái ép ra một yêu cầu hạ tầng:

| Khác biệt | Web | Embedded | Hệ quả bắt buộc |
|---|---|---|---|
| Chi phí build | giây–phút | **phút–giờ**, cross-compile, có khi cả rootfs | **sstate/artifact cache** dùng chung, build node mạnh |
| Nơi chạy test | ngay trên runner | ❌ binary ARM **không chạy trên runner x86** | board thật / QEMU / **HIL** |
| Trạng thái sau test | container xoá là sạch | 🔴 board **giữ trạng thái**, treo được, **hỏng thật** | flash lại + **cắt nguồn từ xa** + health check |

**Vì sao — tầng nông:** "vì phải chạy trên phần cứng."
**⭐ Tầng sâu:** CI web coi runner là tài nguyên **vô hạn, dùng một lần**; CI embedded coi board là tài nguyên **hữu hạn, dùng lại, biết hỏng**. Gần như mọi phức tạp của test farm sinh ra từ câu đó — quá nửa công sức không nằm ở *chạy test* mà ở **đưa board về trạng thái sạch, biết nó đang hỏng, và cứu nó không cần người tới bàn**.

**Chốt:** *"Khác biệt lớn nhất không phải kiến trúc CPU, mà là runner vứt đi được còn board thì phải bảo trì."*
</details>

---

#### BLD-021 · 🟠 · concept · ⭐ · [→ ci-and-test-farm §3](../../../06-build-systems/ci-and-test-farm.md)
**Dự án bắt buộc mọi thay đổi phải qua pipeline mới vào được source chính, không có đường submit tay. Mô hình này tên gì, được lợi gì, và cái giá là gì?**
<details><summary>Đáp án</summary>

**Cơ chế** — gọi là **pre-merge gating / gated check-in**: thay đổi vào **hàng đợi** → build toàn bộ matrix + test → **pass mới chạm trunk**. Đối lập là **post-merge**: vào trunk trước, hỏng thì revert.

| | Post-merge | **Pre-merge (gated)** |
|---|---|---|
| Trunk có bao giờ hỏng | **có** | ~không |
| Độ trễ cho người submit | phút | **cả pipeline** |
| Thông lượng | cao | **giới hạn bởi hàng đợi** |
| Hợp khi | ít platform, revert rẻ | **nhiều platform, trunk hỏng đắt** |

**Vì sao — tầng nông:** "để trunk không hỏng."
**⭐ Tầng sâu (định lượng):** trunk hỏng **không phải** "một người build lỗi" mà là **N kỹ sư × số giờ bị chặn**; tệ hơn, người gây hỏng thường **không phải** người phát hiện, nên thời gian truy thủ phạm cũng là chi phí. Càng nhiều platform × nhiều team thì N càng lớn ⇒ gated thắng.

**Cái giá — phải nêu, nếu không là trả lời một chiều:**
1. **Hàng đợi thành nút cổ chai**: pipeline mất *T* ⇒ thông lượng trần `1/T`.
2. **Gom lô đổi thông lượng lấy khả năng chẩn đoán**: test 8 thay đổi một lượt, pass thì nhanh gấp 8; hỏng thì **không biết cái nào** ⇒ phải **bisect** lô.
3. **Bị chặn oan** khi lô chung hỏng vì lỗi người khác.
4. **Cổng chậm quá sẽ đẻ ra đường tắt** cho "bản gấp" — lúc đó cổng bắt đầu chết.

⚠️ **Bẫy:** khen gated mà không nói được nút cổ chai + bisect ⇒ nghe như chưa từng đứng trong hàng đợi đó.
</details>

---

#### BLD-022 · 🟡 · concept · [→ ci-and-test-farm §4](../../../06-build-systems/ci-and-test-farm.md)
**Pipeline build cùng một thay đổi trên ~10 platform khác chip. Vì sao không build một lần cho xong? Cái gì thực sự khác nhau giữa các platform?**
<details><summary>Đáp án</summary>

| Khác nhau ở | Lỗi chỉ lộ trên một platform |
|---|---|
| **Toolchain/compiler version** | warning mới hoá lỗi với `-Werror` |
| **Kernel version** | API kernel đổi (5.10 → 6.x) ⇒ driver không build; symbol thành `EXPORT_SYMBOL_GPL` |
| **Kernel config / `#ifdef`** | code nằm trong `#ifdef CONFIG_X` — platform tắt config đó **chưa từng build tới dòng đó** |
| **Kiến trúc/ABI** | 32 vs 64-bit: `long`, con trỏ, alignment |
| **Device tree / driver set** | node thiếu ⇒ `-EPROBE_DEFER` mãi mãi, chỉ lộ lúc chạy |
| **Vendor BSP fork** | mỗi vendor vá kernel khác nhau |

⚠️ **Bẫy phổ biến:** *"code C++ thuần thì build đâu cũng như nhau"* — sai ở dòng `#ifdef`: nhánh bị tắt **chưa từng được compiler đọc**, tức chưa cả được kiểm cú pháp, chứ không phải "chưa test".

**Nối Yocto:** mỗi dòng matrix ≈ một **`MACHINE`**; build 10 lần vẫn chịu nổi nhờ **sstate-cache** dùng chung (phần `allarch`/native tái dùng, chỉ phần machine-specific build lại) — xem [BLD-006](build-systems.md).
</details>

---

#### BLD-023 · 🟡 · concept · ⭐ · [→ ci-and-test-farm §5](../../../06-build-systems/ci-and-test-farm.md)
**Smoke test gồm những gì, và khác robustness/functional test ở chỗ nào? Thứ tự chạy nên thế nào?**
<details><summary>Đáp án</summary>

**Smoke** trả lời đúng **một** câu: *"bản build này có đáng để đổ hàng giờ test lên không?"* (tên gốc từ phần cứng: cắm điện xem có bốc khói không). Trên thiết bị hiển thị:

1. **Boot tới nơi** — bootloader → kernel → init → UI; bắt panic/watchdog/boot time.
2. ⭐ **Đúng bản** — image báo đúng version. *Thiếu bước này thì flash trượt ⇒ test nguyên image cũ và **xanh hết**.*
3. **Không crash** — không coredump mới, không `Oops` trong `dmesg`, service không failed.
4. **Input có phản hồi** — remote ăn phím, đổi volume/nguồn vào.
5. **Có hình/tiếng** — capture khung hình **không đen**, có audio.
6. **Mạng lên**; 7. **Reboot 3–5 lần**; 8. **Quét log** (`FATAL`, `segfault`, `Call Trace`).

| | **Smoke** | **Robustness** |
|---|---|---|
| Trả lời | *"còn sống không?"* | *"đủ tốt để ship không?"* |
| Kết quả | **pass/fail** | **số đo + ngưỡng** (ms, fps) |
| Phủ | rộng–nông | hẹp–sâu |
| Thời lượng | **phút** | **giờ** |
| Fail nghĩa là | build hỏng nặng | chất lượng suy giảm |

**⭐ Thứ tự:** sắp cổng theo **chi phí phát hiện một lỗi, rẻ trước** — build/static/unit-test (host) → smoke → robustness → soak (nightly). Robustness chạy trước smoke = đốt hàng giờ để biết thứ 3 phút đã biết.

**Chốt:** *"Smoke là cổng nhị phân rẻ tiền; robustness là phép đo đắt tiền. Đảo thứ tự là tự trả giá tối đa cho một lỗi tối thiểu."*
</details>

---

#### BLD-024 · 🟠 · design · ⭐ · 🏗️ · [→ ci-and-test-farm §6](../../../06-build-systems/ci-and-test-farm.md)
**Thiết kế một test farm tự động chạy test trên TV thật sau mỗi lần build. Cần những khối gì và vì sao mỗi khối là bắt buộc?**
<details><summary>Đáp án</summary>

**Sáu khối, mỗi khối trả lời một câu hỏi "nếu thiếu thì sao":**

| # | Khối | Thiếu thì sao |
|---|---|---|
| ① | **Điều khiển nguồn** (relay/PDU) | Test làm treo thiết bị là **chuyện thường**, không phải ngoại lệ ⇒ mỗi lần treo cần người đi cắm lại ⇒ **farm chết sau 6h chiều** |
| ② | **Serial console** | Là kênh **duy nhất còn sống** khi mạng chưa lên / UI chết / **kernel panic** ⇒ không có = không có bằng chứng cho ca đáng giá nhất |
| ③ | **Flash tự động** (USB/ethernet) | Không đưa được về **trạng thái sạch đã biết** ⇒ job sau nhiễm trạng thái job trước |
| ④ | **Bơm input**: phát **IR**, **pattern generator** cấp source | Không có người bấm remote; cần **nguồn tín hiệu tất định** |
| ⑤ | **Thu output**: capture card + **camera rời** | Cách duy nhất biết "màn hình có hiện đúng không" và **đo được thời gian** |
| ⑥ | **Orchestrator + device pool + thu log** | Xếp hàng job, cấp phát board, **health check**, gỡ board hỏng khỏi pool, gom bằng chứng |

**⭐ Vì sao pattern generator chứ không mở một video thật:**
1. **Tất định** — biết chính xác khung hình *phải* ra sao ⇒ so bằng checksum/histogram, kết quả **quyết định được**.
2. **Không kéo mạng/CDN/DRM vào đường dẫn test** ⇒ tránh flaky không liên quan tới code.
3. **Ép được ca biên** — đổi độ phân giải, tần số quét, HDR/SDR, tín hiệu ngoài chuẩn.

**Vận hành (chỗ hay bị bỏ):** board hỏng dần phải **tự rút khỏi pool**, và báo cáo phải **phân biệt "hỏng do hạ tầng" với "hỏng do code"** — không thì mọi thống kê thành vô nghĩa.
</details>

---

#### BLD-036 · 🟡 · concept · ⭐ · [→ ci-and-test-farm §6.1](../../../06-build-systems/ci-and-test-farm.md)
**Image có thể đưa lên thiết bị bằng USB rời (cắm tay) hoặc qua ethernet. Chọn đường nào cho test farm, và vì sao vẫn phải giữ đường còn lại?**
<details><summary>Đáp án</summary>

| | **USB rời** | **Ethernet** |
|---|---|---|
| Ai thao tác | 🔴 **người cắm tay** | máy — **tự động hoá được** |
| Thiết bị cần ở trạng thái nào | gần như **chết cũng cài được** (recovery/ROM) | phải **boot đủ xa để có mạng** |
| Dùng để | **cứu máy**, bring-up | **vòng lặp hằng ngày** |

**⭐ Ba hệ quả:**
1. **Ethernet là con đường duy nhất cho farm chạy không người.** Còn phải cắm USB nghĩa là còn người trong vòng lặp ⇒ chưa phải test tự động, chỉ là test có script hỗ trợ.
2. 🔴 **Nhưng ethernet không tự cứu được chính nó** — cài qua mạng đòi thiết bị boot đủ xa để có mạng. Đúng lúc cần nhất (image mới làm máy không boot) thì đường đó **biến mất**. ⇒ phải có **đường thoát không phụ thuộc bản đang chạy**: USB/recovery, bootloader nạp qua **serial/TFTP**, hoặc **A/B partition** tự quay về bản cũ.
3. **Đó là lý do khối nguồn + serial không thừa** ([BLD-024](build-systems.md)): chuỗi cứu chuẩn là **cắt nguồn → bật lại → bắt bootloader qua serial → nạp lại**.

**Chốt:** *"Ethernet là đường cài hằng ngày vì tự động hoá được; USB/serial là đường cứu hộ vì không phụ thuộc firmware đang nằm trên máy. Farm cần cả hai — nhưng chỉ đường thứ nhất quyết định farm có chạy qua đêm được không."*
</details>

---

#### BLD-025 · 🟠 · concept · ⭐ · [→ ci-and-test-farm §8](../../../06-build-systems/ci-and-test-farm.md)
**Yêu cầu: "bấm remote tới lúc hình đổi ≤ 200 ms". Vì sao không đo bằng `clock_gettime()` ngay trong phần mềm trên thiết bị?**
<details><summary>Đáp án</summary>

Ba lý do, và cả ba đều là lý do **thiết kế phép đo**, không phải chuyện độ chính xác đồng hồ:

| Vấn đề | Vì sao đo từ trong sai |
|---|---|
| **Không thấy điểm cuối thật** | Phần mềm chỉ biết lúc nó **gửi khung hình đi**; scaler, TCON, thời gian đáp ứng panel nằm **sau** đó ⇒ mắt người thấy chậm hơn số in ra |
| **Thiếu điểm đầu thật** | Đồng hồ chạy từ lúc **driver IR nhận mã**, không tính từ lúc **ngón tay bấm** |
| 🔴 **Hiệu ứng quan sát viên** | Code đo chịu **chính cái tải** đang đo: hệ quá tải thì tiến trình đo cũng bị hoãn ⇒ **báo số đẹp hơn sự thật đúng lúc hệ tệ nhất** |

**Cách đúng — cả hai đầu nằm ngoài thiết bị:** bộ điều khiển đo phát kích thích tại `t0` (IR), camera tốc độ cao / cảm biến quang ghi lúc khung hình đổi thật `t1`; `delay = t1 − t0` **trên cùng một đồng hồ ngoài** ⇒ không cần đồng bộ thời gian với DUT, loại nguồn sai số lớn nhất ngay từ thiết kế. Camera 240 fps ≈ độ phân giải 4 ms; cần hơn thì photodiode dán lên màn.

**Chốt:** *"Không đo được độ trễ của chính mình từ bên trong — cả hai đầu đều nằm ngoài phần mềm, và bản thân phép đo cũng chịu cái tải mà nó đang đo."*
</details>

---

#### BLD-026 · 🟠 · concept · [→ ci-and-test-farm §10](../../../06-build-systems/ci-and-test-farm.md)
**Flaky test là gì, và vì sao trên một cổng chặn (gated) nó nguy hiểm hơn hẳn trên CI thường? Xử lý thế nào?**
<details><summary>Đáp án</summary>

**Định nghĩa:** test cho kết quả khác nhau trên **cùng một code**.

**⭐ Cơ chế nguy hiểm — không phải "chặn nhầm" mà là "ngừng chặn":**
```
hỏng ngẫu nhiên 5% → chặn oan người vô tội → người ta học "cứ bấm retry"
   → retry thành phản xạ với MỌI lỗi → 🔴 lỗi THẬT cũng bị retry cho qua
   → cổng vẫn tốn thời gian nhưng KHÔNG còn chặn gì
```
Cộng dồn rất nhanh: 200 test, mỗi test flaky 0.5% ⇒ `1 − 0.995²⁰⁰ ≈ **63%** số lần chạy có ít nhất một lỗi oan ⇒ gần như lần nào cũng phải retry.

**Nguồn flaky đặc trưng embedded** (khác web): quang/cơ khí (camera lệch, ánh sáng phòng, cáp lỏng) · **`sleep` cố định** thay vì chờ điều kiện · trạng thái tồn dư từ job trước · **một board trong pool hỏng dần**.

**Xử lý:**
1. **Đo trước** — ghi tỉ lệ pass/fail theo thời gian; không đo thì tranh cãi bằng cảm giác.
2. **Quarantine** — gỡ khỏi gate, chuyển nightly, có hạn sửa. Giữ trên gate là hy sinh cả cổng cho một test.
3. **Bỏ `sleep`, chờ theo điều kiện** (log ra dòng X / cổng mở) có timeout.
4. **Health check board** trước job; board hỏng **tự rút khỏi pool**.
5. **Tách "hỏng do hạ tầng" khỏi "hỏng do code"** trên báo cáo.

**Chốt:** *"Flaky không làm cổng chặn sai — nó làm cổng ngừng chặn."*
</details>

---

#### BLD-027 · 🟠 · concept · ⭐ · [→ ci-and-test-farm §7](../../../06-build-systems/ci-and-test-farm.md)
**Pipeline development đo bằng thiết bị rời (IR, capture, camera), còn verification dưới nhà máy lại dùng test daemon chạy trên máy và app gọi API. Vì sao hai nơi chọn hai kiến trúc khác nhau?**
<details><summary>Đáp án</summary>

| | **Quan sát ngoài (black-box)** | **Test daemon trên thiết bị** |
|---|---|---|
| Thấy được | **đúng cái người dùng thấy** | **giá trị bên trong**: sensor, calib, ID panel |
| Ảnh hưởng lên hệ | ~0 | **có** — chiếm CPU/RAM, và **là phần mềm không có trong bản ship** |
| Tốc độ mỗi phép đo | chậm (khung hình, quang học) | **rất nhanh** (một lời gọi API) |
| Độ giòn | cao (ánh sáng, cáp, đặt camera) | thấp |
| Dùng ở | **pipeline development** | **verification nhà máy** |

**⭐ Ba lý do:**
1. **Đối tượng cần chứng minh khác nhau.** Pipeline dev hỏi *"thay đổi code có làm hỏng **hành vi** không?"* ⇒ phải quan sát **như người dùng**, vì lỗi hay nằm đúng ở khoảng cách giữa *"API trả về OK"* và *"màn hình vẫn sai"*. Nhà máy hỏi *"**cái máy cụ thể này** lắp/hiệu chỉnh đúng chưa?"* — phần mềm đã cố định, cần đọc giá trị bên trong.
2. 🔴 **Daemon làm bạn test một image khác image đem bán.** Thêm phần mềm = đổi bộ nhớ, lịch CPU, thời điểm; với chỉ tiêu **thời gian đáp ứng** thì chính công cụ đo thành nguồn sai số. Nhà máy chấp nhận được vì thứ đang kiểm là **phần cứng**, không phải build.
3. **Ràng buộc thông lượng ngược nhau.** Nhà máy: **hàng nghìn máy × vài chục giây** ⇒ không soi camera nổi. Dev: **vài build/ngày × hàng giờ** ⇒ đủ chỗ cho phép đo quang học chậm mà trung thực.

**Chốt:** *"Pipeline dev đo **build** nên phải đo từ ngoài để khỏi làm sai lệch thứ đang đo; nhà máy đo **từng đơn vị phần cứng** trên một build đã cố định nên đo từ trong là hợp lý và nhanh hơn nhiều."*
</details>

---

#### BLD-028 · 🟡 · concept · [→ ci-and-test-farm §9](../../../06-build-systems/ci-and-test-farm.md)
**Test farm nên thu những bằng chứng gì khi một bài test hỏng, và làm sao chuyển defect tới đúng team một cách tự động?**
<details><summary>Đáp án</summary>

**Vì sao cần tự động:** trên gate nhiều platform, **người làm hỏng thường không phải người đọc log** ⇒ giá trị lớn nhất không phải "lưu log" mà là **nhận dạng + định tuyến**.

| Loại bằng chứng | Thu ở đâu | Ghi chú |
|---|---|---|
| **Message lỗi đặc thù của package** | log ứng dụng / journal | Rẻ nhất, **định tuyến tốt nhất** — chuỗi lỗi đã gắn với chủ sở hữu |
| **Kernel `Oops` / `Call Trace`** | `dmesg` + serial | Hệ có thể còn sống ⇒ lấy qua mạng được |
| 🔴 **Kernel panic** | **chỉ serial** | Mạng chết theo — không console = không bằng chứng |
| **Core dump** | phân vùng riêng / gửi server | **Đắt**: dung lượng, thời gian ghi, có thể chứa dữ liệu nhạy cảm |

**Cơ chế định tuyến:** trích **chữ ký lỗi** (tên tiến trình + hàm trên cùng backtrace + mẫu chuỗi lỗi) → tra bảng ánh xạ sang team sở hữu → mở defect. Kèm hai lợi ích: **gộp trùng** (cùng chữ ký = một defect, không đẻ 40 vé) và **đếm tần suất** ⇒ phân biệt lỗi **luôn xảy ra** với lỗi **flaky** ([BLD-026](build-systems.md)).

⚠️ **Bẫy symbol:** image ship đã **strip** ⇒ backtrace chỉ là địa chỉ trần. Phải **giữ bản có symbol trên server đúng theo từng build** (gói `-dbg` của Yocto) rồi giải mã bằng `addr2line`/`gdb`. Không giữ = có dump mà đọc không ra.
</details>

---

#### BLD-029 · 🟡 · concept · [→ ci-and-test-farm §11](../../../06-build-systems/ci-and-test-farm.md)
**Nguồn nội bộ dùng Perforce, còn source SoC vendor trên Git — sửa trên Git rồi đồng bộ về Perforce. Chỗ nào dễ đau, và quản lý thay đổi trên source vendor thế nào cho đúng?**
<details><summary>Đáp án</summary>

| | **Perforce** | **Git** |
|---|---|---|
| Mô hình | **tập trung** — server là chân lý | **phân tán** |
| Đơn vị | **changelist** (số tăng, nguyên tử) | commit (hash) |
| Commit cục bộ | ❌ không | ✅ có |
| Phạm vi | theo **đường dẫn** depot | theo **cả repo** |
| Sửa lịch sử | gần như không | `rebase`/`amend` **sửa được** |

**Bốn chỗ đau:**
1. **Ai là chân lý** — mirror **một chiều** thì đơn giản; **hai chiều** mới sinh việc: conflict xảy ra **trong cầu nối**, không thuộc máy của ai.
2. **Lịch sử không ánh xạ 1–1** — `rebase`/`squash`/force-push viết lại lịch sử, changelist thì **chỉ tiến** ⇒ hoặc cấm rebase nhánh đã sync, hoặc chấp nhận **gộp phẳng** (mất truy vết).
3. **Cầu nối là điểm hỏng đơn lẻ** — nó chết thì hai bên **âm thầm phân kỳ**.
4. ⭐ **Vendor ra bản mới va vào bản vá của mình.**

**⭐ Cách đúng cho (4) — chính là mô hình Yocto:** giữ upstream nguyên vẹn, mọi thay đổi là **patch có thứ tự**:
```bitbake
SRC_URI = "git://github.com/vendor/driver.git;protocol=https;branch=main \
           file://0001-fix-probe-defer.patch"
SRCREV = "a1b2c3d4"   # ghim commit → build tái lập được
```

| Cách | Khi vendor cập nhật |
|---|---|
| 🔴 Fork rồi sửa thẳng | merge cả cây source; **không ai còn biết dòng nào là của mình** |
| ✅ Patch chồng (`.bbappend`/quilt/`devtool`) | đổi `SRCREV`; patch nào **không áp được nữa thì báo lỗi ngay** — danh sách khác biệt luôn tường minh |

**Chốt:** *"Đừng fork source vendor — giữ upstream nguyên vẹn và diễn đạt mọi thay đổi của mình dưới dạng patch có thứ tự."*
</details>

---

⬅️ [Bank index](README.md)
