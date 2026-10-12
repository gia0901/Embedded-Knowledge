# BLD — Build systems (CMake, Yocto, cross-compile, CI)

> Domain `BLD`. Hệ build cho embedded Linux: CMake, Yocto, cross-compile, **CI/CD & automated test farm**. Nguồn: [06-build-systems](../../../06-build-systems/). Track dùng: `bsp`, `cpp-system`, `melp`. Yocto/BSP-layer chuyên sâu còn ở [BSP-017…019](bsp.md).
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | Biên dịch & Make | 4 |
| **B** | CMake | 4 |
| **C** | Cross-compilation | 4 |
| **D** | Yocto | 9 |
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
LIC_FILES_CHKSUM = "file://LICENSE;md5=0835ade..."   # đổi nội dung license -> BUILD FAIL
SRC_URI  = "git://git.example.com/sensord.git;branch=main;protocol=https \
            file://0001-fix-i2c-timeout.patch"
SRCREV   = "a1b2c3d4"        # ⭐ ghim commit; ${AUTOREV} làm MẤT tái lập
S        = "${WORKDIR}/git"
DEPENDS  = "libgpiod"        # lúc BUILD
RDEPENDS:${PN} = "i2c-tools" # lúc CHẠY
inherit cmake systemd        # mượn hành vi build chuẩn
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
devtool modify sensord                  # kéo source ra workspace/, thành git repo THẬT
#   ... sửa code, commit từng bước như bình thường ...
devtool build sensord                   # build lại, chỉ phần đổi
devtool deploy-target sensord root@192.168.1.10   # ⭐ đẩy thẳng lên board đang chạy
devtool finish sensord ../meta-myproduct          # gói thành patch + bbappend trong layer
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
# image recipe — "tôi muốn image này có sensord và i2c-tools"
IMAGE_INSTALL:append = " sensord i2c-tools"

# recipe của sensord — "sensord KHÔNG CHẠY ĐƯỢC nếu thiếu libfoo"
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
# 1. local.conf  -> CHỈ ĐỂ THỬ. Không tái lập được trên CI/máy đồng nghiệp
IMAGE_INSTALL:append = " strace"

# 2. image recipe riêng của bạn  -> ✅ cách đúng cho sản phẩm
require recipes-core/images/core-image-minimal.bb
IMAGE_INSTALL:append = " sensord i2c-tools"

# 3. packagegroup  -> ✅ khi nhiều image dùng chung một bộ gói
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
bitbake -c clean <recipe>        # xoá tmp/work của recipe, GIỮ sstate -> thường không đủ
bitbake -c cleansstate <recipe>  # xoá cả sstate của recipe   -> dùng khi nghi cache cũ
rm -rf tmp/                      # xoá thư mục làm việc, GIỮ sstate-cache/ -> sạch mà vẫn nhanh
bitbake -S printdiff <target>    # ⭐ CHẨN ĐOÁN: hash nào đổi so với lần trước
```

**⚠️ Bẫy:** (1) `cleansstate` **toàn bộ** cho chắc — vứt hàng giờ build mà thường chỉ cần một recipe; (2) để `SSTATE_DIR`/`DL_DIR` **bên trong** `build/` ⇒ `rm -rf tmp/` an toàn nhưng lỡ tay xoá `build/` là mất sạch — tách ra ngoài; (3) tưởng sstate làm build **đầu tiên** nhanh — không, nó chỉ giúp từ lần **thứ hai** hoặc khi tải cache dùng chung của CI.

**Chốt:** *"sstate cache theo **task** và khoá theo **hash đầu vào** của task đó. Nên khi build không nhận thay đổi, câu hỏi đúng là 'thay đổi của tôi có nằm trong thứ được băm không?' — sửa trong `tmp/work` thì không, và đó là ca kinh điển."*
</details>

#### BLD-037 · 🟡 · concept · ⭐ · 🎤 2026-09-06 · [→ yocto §3](../../../06-build-systems/yocto.md)
**"Yocto, Poky, BitBake, OpenEmbedded — bốn cái tên này quan hệ với nhau thế nào?"**
<details><summary>Đáp án</summary>

**Câu mở màn kinh điển.** Trả lời lộn xộn ở đây là mất niềm tin ngay câu đầu.

| Tên | Là gì | Ví von |
|---|---|---|
| **BitBake** | **engine** — đọc metadata, dựng đồ thị phụ thuộc, chạy task | trình thông dịch |
| **OpenEmbedded-Core** (`meta`) | **metadata nền** — recipe cho toolchain, libc, hàng nghìn package cơ bản | thư viện chuẩn |
| **Poky** | **bản tham chiếu** = BitBake + OE-Core + `meta-poky` + `meta-yocto-bsp` | một distro mẫu để bạn clone rồi sửa |
| **Yocto Project** | **dự án/tổ chức** bảo trợ tất cả, cộng hạ tầng (autobuilder, LTS, license/CVE tooling) | không phải phần mềm |

**⭐ Hai câu chốt phải bật ra được:**
1. *"**Yocto không phải một distro** — nó là **framework để mình sinh ra distro của mình**. Poky mới là distro (bản mẫu)."*
2. *"`git clone poky` là lấy **cả bộ** — engine + metadata nền + BSP mẫu. Từ đó mình **thêm layer riêng**, không sửa vào poky."*

⚠️ **Bẫy:** ① nói *"Yocto là một bản Linux nhúng"* — sai bản chất · ② nhầm **Poky = BitBake** · ③ tưởng phải cài BitBake riêng (nó **nằm trong** poky, dùng qua `source oe-init-build-env`).
</details>

---

#### BLD-038 · 🟠 · concept · ⭐ · [→ yocto §5](../../../06-build-systems/yocto.md)
**Trong `.bbappend` bạn viết `EXTRA_OEMAKE += "foo"` nhưng giá trị không có tác dụng, còn `SRC_URI:append = " file://x.patch"` thì chạy. Vì sao? Và vì sao đoạn cũ tra được trên mạng dùng `SRC_URI_append` lại KHÔNG báo lỗi mà cũng KHÔNG chạy?**
<details><summary>Đáp án</summary>

**Cơ chế — hai toán tử khác nhau ở THỜI ĐIỂM áp dụng:**

| | Áp dụng khi nào | Hệ quả trong `.bbappend` |
|---|---|---|
| `+=` / `=` | **ngay lúc parse dòng đó** | Recipe gốc parse **sau** có thể **ghi đè mất** ⇒ giá trị của bạn biến mất |
| **`:append` / `:prepend`** | **hoãn tới cuối**, sau khi mọi thứ đã parse xong | 🟢 **Luôn thắng** — đây là lý do `.bbappend` phải dùng nó |

⇒ **Luật thực hành: trong `.bbappend` gần như luôn dùng `:append`, không dùng `+=`.**

⚠️ **Chi tiết dễ mất cả buổi:** `:append` **không tự thêm dấu cách**. Phải tự viết `" file://x.patch"` — thiếu khoảng trắng đầu là hai giá trị **dính liền** thành một chuỗi vô nghĩa. (`+=` thì có thêm dấu cách — đúng chỗ dễ nhầm lẫn ngược.)

### 🔴 Vế thứ hai — vì sao `SRC_URI_append` "im lặng không làm gì"

Từ **Yocto 3.4 (honister)**, ký tự phân tách override đổi từ **`_`** sang **`:`**. Với BitBake mới:
- `SRC_URI:append` ⇒ hiểu là **toán tử append** ✅
- `SRC_URI_append` ⇒ **không còn là toán tử nữa** — nó chỉ là **tên một biến bình thường** tên là `SRC_URI_append`. Biến đó **không ai đọc**, nên **không lỗi, không cảnh báo, không tác dụng**.

⭐ **Đây là lớp lỗi tệ nhất: sai mà không báo.** Mọi tài liệu/StackOverflow trước 2021 đều dùng `_`, nên copy về là dính. **Nói được ý này ở phỏng vấn là dấu hiệu đã dùng thật**, không phải chỉ đọc.

**Chẩn đoán:** `bitbake -e <recipe> | grep ^SRC_URI=` — xem **giá trị cuối cùng** sau khi mọi override đã áp. Không thấy thay đổi của mình ⇒ hoặc sai toán tử, hoặc `.bbappend` **không được nhặt** (`bitbake-layers show-appends`).

**Chốt:** *"`+=` áp ngay lúc parse nên trong bbappend dễ bị recipe gốc ghi đè; `:append` hoãn tới cuối nên luôn thắng. Và cú pháp `_` đời cũ giờ **im lặng không làm gì** — đó là bẫy tôi kiểm bằng `bitbake -e`."*
</details>

---

#### BLD-039 · 🟡 · concept · ⭐ · 🎤 2026-09-06 · [→ yocto §6](../../../06-build-systems/yocto.md)
**Bạn xoá sạch `tmp/` của một build Yocto rồi build lại — nó xong sau ~51 giây, trong khi build lần đầu mất ~31 phút. Vì sao? `tmp/` chứa gì, và nó khác `sstate-cache/` chỗ nào?**
<details><summary>Đáp án</summary>

**Ba thư mục, ba vai trò — phân biệt được là trả lời xong nửa câu:**

| Thư mục | Là gì | Xoá đi thì mất gì |
|---|---|---|
| `downloads/` (`DL_DIR`) | **nguyên liệu** — tarball/git source | phải tải lại ⇒ tốn **mạng** |
| **`sstate-cache/`** | **thành phẩm của từng task**, đóng gói theo **hash đầu vào** | 🔴 phải build lại thật ⇒ tốn **CPU** |
| `tmp/` | **xưởng** — nơi diễn ra việc | chỉ mất chỗ làm việc, **không mất thành phẩm** |

**`tmp/` chứa:** `work/<arch>/<recipe>/<ver>/` (source đã giải nén `${S}`, cây build `${B}`, thư mục cài tạm `${D}`, log) · `deploy/` (image, gói, SDK) · `sysroots-components/` + `recipe-sysroot*` · `stamps/` (dấu task đã chạy với hash nào) · `cache/` (metadata đã parse). **Toàn bộ là thứ suy ra được** — Yocto coi `tmp/` là **vứt đi được**, đó là thiết kế.

### ⭐ Cơ chế: sstate cache **ĐẦU RA CỦA TASK**, khoá theo **HASH ĐẦU VÀO**

Với mỗi task bật sstate, BitBake ① tính **hash toàn bộ đầu vào** (code của task + biến nó dùng + **hash của mọi task nó phụ thuộc**) ② chạy xong thì **đóng gói đầu ra** thành tarball đặt tên theo chính hash đó:
```
sstate:u-boot:...:9f3c2a…_populate_sysroot.tar.zst
                  ↑ hash đầu vào       ↑ tên task
```
Lần build sau, **trước khi chạy** task nó tính hash trước. Có tarball khớp ⇒ **không chạy task**, mà chạy **`do_<task>_setscene`**: **bung tarball vào đúng chỗ**.
```
Bình thường:   fetch → unpack → patch → configure → compile → install → populate_sysroot
Có sstate:     ─────────── BỎ QUA TẤT CẢ ───────────→ [bung tarball]  ⚡
```

⇒ **Xoá `tmp/` chỉ xoá cái xưởng; thành phẩm vẫn nằm trong `sstate-cache/`.** Bung tarball tốn **giây**, compile tốn **phút–giờ**.

**⭐ Điểm tinh tế hay bị nói sai:** sstate **không cache "việc compile"** — `do_compile`/`do_configure` **không** được cache trực tiếp. Nó cache **sản phẩm giao ra**: nội dung đã staging vào sysroot, gói đã đóng, artifact đã deploy. Vì thế khôi phục từ sstate là **không bao giờ compile**, chứ không phải "compile nhanh hơn".

**📌 Vì sao đây là câu đáng hỏi:** nó là lý do **CI Yocto khả thi**. Mỗi job CI có `tmp/` mới tinh nhưng **mount chung `sstate-cache/` + `downloads/`** ⇒ build vài phút thay vì vài giờ. Không có sstate thì gated check-in trên 10 platform ([BLD-021](build-systems.md)) là bất khả thi.

### ⭐ Đọc `Sstate summary` — công cụ chẩn đoán tốt nhất

Mỗi build in ra dòng này; đọc được nó là biết **build tới từ đâu**:
```
Sstate summary: Wanted 163 Local 150 Mirrors 0 Missed 13 Current 200 (92% match, 96% complete)
Tasks Summary:  Attempted 1082 tasks of which 1051 didn't need to be rerun
```

| Trường | Nghĩa |
|---|---|
| `Wanted` | số sstate object build này cần |
| `Local` | lấy được từ `sstate-cache/` trên máy ⚡ |
| `Mirrors` | tải từ sstate mirror (nếu có cấu hình) |
| 🔴 `Missed` | **không tìm thấy ⇒ task phải chạy THẬT** |
| `Current` | đã đúng sẵn trong `tmp/`, không phải làm gì |

⇒ **`Attempted N − didn't need rerun M` = số task thật sự chạy.** Đây chính là thước đo *"thay đổi của tôi làm hỏng hash của bao nhiêu task"* — câu hỏi đúng, thay cho *"sstate nhanh không"*.

**Số đo thật** ([BSP-038](bsp.md), 24 threads, scarthgap, `beaglebone-yocto`): lần đầu **~31 phút** → xoá `tmp/` giữ sstate: **51 giây** (**~36×**). Sàn parse metadata: **6 giây**. `cleansstate u-boot` rồi build lại: **~17–20 giây** (chỉ **31/1082 task** chạy thật).

⭐ **Một ca đáng nhớ từ chính số đo trên:** `cleansstate u-boot` rồi build **cả image** (16,8s) **không lâu hơn** build **riêng u-boot** (20,1s). Vì sstate khoá theo **hash đầu vào**: xoá thành phẩm của u-boot **không đổi đầu vào** của task image phía sau ⇒ chúng vẫn khôi phục được từ sstate. **Đây là bằng chứng trực tiếp rằng sstate khoá theo input, không phải output.**

⚠️ **Bẫy:** ① tưởng sstate cache theo **recipe** — không, theo **task** ② tưởng xoá `tmp/` là mất hết ③ quên `downloads/` là **cache thứ ba, riêng biệt** (mạng, không phải CPU) ④ nói *"sstate làm compile nhanh hơn"* — sai, nó **bỏ qua compile hoàn toàn**.
</details>

---

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
<details><summary>🎙️ Trả lời mẫu</summary>

> *"**CI** — tích hợp liên tục: mỗi thay đổi được gộp, build và test tự động. **Continuous Delivery**: thêm bước đóng gói thành một bản phát hành **sẵn sàng**, nhưng **người** bấm nút phát hành. **Continuous Deployment**: tự đưa ra môi trường thật luôn, không ai bấm.*
>
> *Ở embedded, *"đưa ra thật"* nghĩa là cập nhật firmware xuống **thiết bị của khách**. Web có lỗi thì đổi lại bản cũ trong vài giây; còn thiết bị mất điện giữa lúc cập nhật có thể thành **cục gạch** và phải thu hồi. Nên embedded gần như luôn dừng ở **Continuous Delivery**: máy lo tới mức *"đã build, đã ký, đã test, sẵn sàng"*, còn phát hành là quyết định của người, thường theo từng đợt nhỏ trước.*
>
> *Phần em chạm hằng ngày là CI: build tích hợp và các cổng kiểm tra trước khi vào nhánh chính."*

**Nếu bị hỏi tiếp**
- *"Phát hành theo đợt là gì?"* — Đưa cho một nhóm nhỏ thiết bị trước, theo dõi, ổn rồi mới mở rộng.

**Móc nền tảng:** gated check-in ([BLD-021](build-systems.md)) · cập nhật A/B và rollback.

</details>

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
<details><summary>🎙️ Trả lời mẫu</summary>

> *"**Pipeline** là chuỗi bước tự động chạy sau một sự kiện — với em là: submit, build cho các nền tảng, rồi test. **Trigger** là cái làm nó chạy: một lần submit, lịch hằng đêm, hay người gọi tay. **Stage** là một nhóm bước, xong nhóm này mới sang nhóm sau — build, rồi test nhanh, rồi test dài. **Job** là một việc chạy độc lập, chạy song song được — ví dụ *"build cho nền tảng A"*. **Runner** hay agent là **máy** chạy job — máy build thường là x86, không chạy được binary ARM. **Artifact** là thứ giữ lại sau job: image, kernel, gói có symbol. **Gate** là điều kiện phải qua mới đi tiếp. **DUT** là thiết bị đang được test; **HIL** là test có phần cứng thật trong vòng lặp.*
>
> *Ở embedded em nhấn vào **artifact**: thiết bị bán ra sống năm đến mười năm. Hai năm sau có crash ngoài hiện trường thì cần **đúng image đó** và **đúng file symbol** của bản build đó để đọc backtrace. Không lưu artifact thì không điều tra được."*

**Nếu bị hỏi tiếp**
- *"Jenkinsfile là gì?"* — Mô tả pipeline dưới dạng code, nằm trong repo. Em chưa viết ở công ty; ở lab em giữ nó là vỏ mỏng, mỗi stage chỉ gọi một script trong repo để chạy lại y hệt trên máy mình ([unit-test-and-code-quality §2](../../../06-build-systems/unit-test-and-code-quality.md)).

**Móc nền tảng:** symbol và crash ([DBG-045](debugging.md)) · test trên thiết bị thật ([BLD-035](build-systems.md)).

</details>

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
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Đó là **hai trục khác nhau**, không phải hai mức của cùng một thang.*
>
> *Trục **phạm vi** trả lời *"test cái gì"*: **unit** là một hàm, một class, tách khỏi phụ thuộc; **integration** là vài phần ghép lại — ví dụ app thật gọi qua `.so` thật; **system** là cả thiết bị.*
>
> *Trục **mục đích** trả lời *"test để biết gì"*: **smoke** — còn sống không, khởi động được và chức năng chính chạy không; **regression** — thứ trước đây đúng có còn đúng sau thay đổi; **soak** hay stress — chạy lâu, chạy nặng có rò rỉ hay treo không.*
>
> *Mỗi bài test có một vị trí trên cả hai trục: smoke thường ở mức system; regression có thể là unit hoặc system; soak gần như luôn là system. Hỏi *"smoke là unit hay system"* là hỏi nhầm trục.*
>
> *Ở chỗ em: unit test cho library và driver chạy trên máy build; sau đó test trên thiết bị, vì một thay đổi chạy trên rất nhiều model."*

**Nếu bị hỏi tiếp**
- *"Tầng nào nhiều test nhất?"* — Tháp test: nhiều unit test rẻ ở đáy, ít test hệ thống đắt ở đỉnh; cổng rẻ chạy trước.

**Móc nền tảng:** GoogleTest/KUnit ([BLD-044](build-systems.md)) · test trên thiết bị ([BLD-035](build-systems.md)).

</details>

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
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Vì phần mềm embedded chỉ đúng khi đứng **cùng phần cứng** của nó. Có những lớp lỗi không thể lộ trên máy build: thời gian thật, ngắt thật, cảm biến và tấm màn thật, nhiệt độ, nguồn điện.*
>
> *Test trên máy build — unit test, sanitizer — bắt được logic thuần và lỗi bộ nhớ, rẻ và nhanh. Máy giả lập chạy được khởi động và phần lớn user-space, nhưng không có thời gian thật và ngoại vi thật. Chỉ thiết bị thật cho **hành vi thật và số đo thật** — đổi lại chậm, đắt, và thiết bị có thể hỏng.*
>
> *Nên em chia việc: **đẩy càng nhiều test xuống máy build càng tốt**, để dành thiết bị cho thứ chỉ phần cứng mới trả lời được. Thiết bị là tài nguyên đắt nhất trong pipeline; dùng nó cho việc máy build làm được là lãng phí, và còn làm hàng đợi dài thêm.*
>
> *Ở chỗ em, một thay đổi trong library chạy trên rất nhiều model, nên sau unit test luôn có bước test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Chỉ vì ARM khác x86 thôi à?"* — Không; giả lập giải được chuyện kiến trúc CPU mà vẫn không thay được board.

**Móc nền tảng:** tháp test ([BLD-033](build-systems.md)) · gated check-in ([BLD-021](build-systems.md)).

</details>

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
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Mô hình đó gọi là **gated check-in** — kiểm tra trước khi gộp. Ở chỗ em, em là **người dùng** pipeline: lấy code, đưa lên review chéo trên Perforce, submit; thay đổi đi vào build tích hợp và hệ thống CI/CD nội bộ; chỉ khi qua hết mới vào nhánh chính. Không có đường nào khác.*
>
> *Được: **nhánh chính gần như không bao giờ hỏng**. Với nhiều nền tảng và nhiều team, nhánh chính hỏng không phải là một người build lỗi, mà là **cả chục kỹ sư bị chặn**, cộng thời gian đi tìm ai gây ra — người gây hỏng thường không phải người phát hiện.*
>
> *Cái giá: một là **chờ** — mỗi thay đổi phải đợi trọn pipeline, và hàng đợi giới hạn số thay đổi vào được mỗi ngày. Hai là khi gom nhiều thay đổi test một lượt cho nhanh, hỏng thì **không biết cái nào** gây ra, phải chia đôi lô để tìm. Ba là bị chặn oan khi lỗi là của người khác trong cùng lô. Và nếu cổng chậm quá, người ta sẽ đòi đường tắt cho *"bản gấp"* — lúc đó cổng bắt đầu mất tác dụng."*

**Nếu bị hỏi tiếp**
- *"Đối lập với gated là gì?"* — Gộp trước, hỏng thì revert. Hợp khi ít nền tảng và revert rẻ.

**Móc nền tảng:** flaky test trên cổng chặn ([BLD-026](build-systems.md)) · các tầng test ([BLD-033](build-systems.md)).

</details>

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
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Flaky test là test cho kết quả khác nhau trên **cùng một code** — lúc xanh lúc đỏ.*
>
> *Trên cổng chặn, nó nguy hiểm không phải vì chặn nhầm, mà vì nó làm cổng **ngừng chặn**. Chuỗi diễn ra thế này: test hỏng ngẫu nhiên chặn oan người vô tội; mọi người học được *"cứ bấm chạy lại"*; chạy lại thành phản xạ với **mọi** lỗi; đến khi có lỗi thật, nó cũng được chạy lại cho qua. Và nó cộng dồn rất nhanh: 200 test, mỗi cái chỉ 0,5% flaky, thì khoảng 63% số lần chạy có ít nhất một lỗi oan.*
>
> *Nguồn flaky đặc trưng ở embedded: dùng `sleep` cố định thay vì chờ một điều kiện; trạng thái còn sót từ job trước; một board trong dàn test hỏng dần; cáp lỏng, ánh sáng phòng.*
>
> *Cách xử lý: **đo** tỉ lệ pass/fail theo thời gian trước đã; **cách ly** test đó khỏi cổng, chuyển sang chạy hằng đêm, có hạn sửa; thay `sleep` bằng **chờ theo điều kiện** có timeout; kiểm sức khoẻ board trước mỗi job, board hỏng tự rút khỏi dàn; và tách *"hỏng do hạ tầng"* khỏi *"hỏng do code"* trên báo cáo."*

**Nếu bị hỏi tiếp**
- *"Một ví dụ trạng thái sót lại?"* — Ở lab của em: test bị giết khi đang giữ semaphore trong shared memory; lần chạy sau mở lại cùng tên và treo ([BLD-045](build-systems.md)).

**Móc nền tảng:** gated check-in ([BLD-021](build-systems.md)) · cô lập tài nguyên giữa các test.

</details>

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

## F — Unit test & code quality (bám việc thật, mức cơ bản)

#### BLD-040 · 🟡 · concept · ⭐ · [→ unit-test-and-code-quality §3](../../../06-build-systems/unit-test-and-code-quality.md)
**Bạn muốn unit test thuật toán dimming (tính độ sáng theo nội dung hình) trên máy build, không có panel, không có chip. Làm thế nào? Fake, mock, stub khác nhau ở đâu?**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Muốn test một phần mà không có phần cứng thì phần đó phải **gọi phần cứng qua một interface**. Thuật toán độ sáng của em giữ con trỏ tới `IDimmingBackend` — interface ghi xuống phần cứng — nên trong test em cắm vào một **backend giả** chỉ ghi lại các giá trị thuật toán yêu cầu ghi, rồi kiểm danh sách đó.*
>
> *Ba loại bản giả: **stub** chỉ trả một giá trị cố định để code chạy qua; **fake** là một cài đặt đơn giản nhưng chạy thật — như backend ghi vào vector — dùng khi muốn kiểm **kết quả**; **mock** kiểm **lời gọi** — được gọi mấy lần, với tham số gì — dùng khi muốn kiểm **tương tác**.*
>
> *Điểm em muốn nhấn: **test được hay không là hệ quả của thiết kế**. Nếu thuật toán gọi thẳng `ioctl` thì không test được khi không có thiết bị. Tách thuật toán khỏi phần ghi phần cứng là thứ tạo ra chỗ cắm.*
>
> *Hai bẫy: mock mọi thứ thì test chỉ kiểm lại cách cài đặt — đổi code là vỡ test dù hành vi vẫn đúng; và fake chạy đúng **không** chứng minh backend thật đúng — vẫn cần test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Coverage sau khi dùng fake?"* — Backend thật sẽ không được chạy trong unit test; coverage chỉ ra đúng chỗ đó ([BLD-041](build-systems.md)).

**Móc nền tảng:** Bridge / dependency inversion ([DP-038](design-patterns.md)) · GoogleTest ([BLD-044](build-systems.md)).

</details>

<details><summary>Đáp án</summary>

**Cơ chế:** muốn test một phần tách khỏi phần cứng thì phần đó phải **gọi phần cứng qua một interface**, để test **cắm thay** được. Thuật toán dimming giữ `IDimmingBackend*` (Bridge) ⟹ test đưa vào một backend giả, rồi kiểm **những gì thuật toán đã yêu cầu ghi**.

```cpp
struct FakeBackend : IDimmingBackend {                 // ghi lại thay vì chạm phần cứng
    std::vector<int32_t> writes;
    uint32_t t_Set2DFinalDuty(BackendGd2DFinalDuty_t* p) override { writes.push_back(p->value); return LIB_OK; }
};
GlobalDimming algo(&fake, shm);
algo.SetBacklight(25);
for (int i = 0; i < 5; ++i) algo.t_vSyncCallBack();
EXPECT_EQ(fake.writes, (std::vector<int32_t>{10, 20, 25}));   // chạy thật: PASS
```

| Test double | Làm gì | Khi nào dùng |
|---|---|---|
| **Stub** | Trả giá trị cố định | Chỉ cần phụ thuộc "đứng đó" cho code chạy qua |
| **Fake** | Cài đặt đơn giản nhưng chạy được (vector, file tạm, shm giả) | Muốn kiểm **kết quả** |
| **Mock** | Kiểm **lời gọi**: gọi mấy lần, đối số gì (gMock `EXPECT_CALL`) | Muốn kiểm **tương tác** |

**"Vì sao" hai tầng:**
- *Nông:* "dùng mock".
- *Sâu:* **testability là hệ quả của thiết kế.** Thuật toán gọi thẳng `ioctl` thì không test được nếu không có thiết bị. Bridge/DIP tạo ra đúng chỗ cắm.

**Bẫy:** ① mock **mọi thứ** ⟹ test chỉ kiểm lại cách cài đặt, đổi code là vỡ test dù hành vi đúng · ② quên rằng fake chạy đúng **không** chứng minh backend thật đúng — cần integration test / test trên thiết bị.

**Chốt:** *"Thuật toán gọi phần cứng qua interface nên test cắm fake backend vào và kiểm những giá trị nó yêu cầu ghi. Fake kiểm kết quả, mock kiểm lời gọi, stub chỉ đứng đó."*
</details>

#### BLD-041 · 🟡 · concept · ⭐ · [→ unit-test-and-code-quality §4](../../../06-build-systems/unit-test-and-code-quality.md)
**Báo cáo coverage của library ghi 90% line coverage. Con số đó nói lên điều gì, KHÔNG nói lên điều gì? Bạn đọc báo cáo thế nào?**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Line coverage 90% nghĩa là **90% số dòng đã chạy ít nhất một lần** trong lúc test. Nó cho biết **dòng nào chưa từng chạy** — tức chỗ chắc chắn chưa được test. Nó **không** nói những dòng đã chạy là đúng: một test không có kiểm tra nào vẫn làm coverage tăng.*
>
> *Nên em đọc cột *"dòng chưa chạy"*, không đọc con số tổng. Ở lab em đo được 89%, nghe tốt, nhưng mấy dòng chưa chạy lại chính là **backend thật** ghi xuống phần cứng: unit test dùng backend giả, còn integration test chạy model khác. Con đường thật *"thuật toán → backend → phần cứng"* chưa có test nào đi qua.*
>
> *Em cũng nói thẳng: ở công ty em không đo coverage; em hiểu nó đo gì và đã thử bằng gcov/gcovr ở lab. Em không đặt chỉ tiêu cứng như *"phải trên 80%"*, vì nó dễ sinh ra test chỉ để chạy qua code."*

**Nếu bị hỏi tiếp**
- *"Line khác branch coverage?"* — Line: dòng đã chạy; branch: mỗi nhánh `if`/`switch` đã đi qua chưa — một dòng `if` chạy rồi nhưng có thể mới đi một nhánh.

**Móc nền tảng:** chất lượng test ([BLD-043](build-systems.md)) · `--coverage` / gcov.

</details>

<details><summary>Đáp án</summary>

**Cơ chế:** build với `--coverage`, compiler chèn bộ đếm vào mỗi dòng/nhánh; chạy test xong, `gcov`/`gcovr` đọc bộ đếm. Kết quả trả lời **một** câu hỏi: *dòng nào đã chạy ít nhất một lần khi test*.

| Coverage nói | Coverage KHÔNG nói |
|---|---|
| Dòng/nhánh nào **chưa từng chạy** ⟹ chắc chắn chưa được test | Dòng đã chạy có **đúng** không |
| Chỗ nên viết test tiếp | Test có `EXPECT` có ý nghĩa không |
| — | Các tổ hợp cấu hình (model, chip) đã được thử chưa |

**Đọc báo cáo thế nào — ví dụ chạy thật** (`libdisplay_lab`, tổng 89%):
```
src/dimming/DimmingBackendChipA.cpp            5        3    60%   4-5
```
Dòng 4–5 là **backend Global thật**. Unit test dùng fake backend, integration test chạy model `local` ⟹ đường *thuật toán Global → backend thật → Panel Control* **chưa có test nào đi qua**, dù tổng là 89%. Đọc cột **Missing**, đừng đọc cột tổng.

**"Vì sao" hai tầng:**
- *Nông:* "coverage càng cao càng tốt".
- *Sâu:* coverage là **thông tin**, không phải **ngưỡng chất lượng**. Đặt chỉ tiêu cứng (vd ≥ 80%) dễ sinh ra test không có `EXPECT`, chỉ để chạy qua code.

**Bẫy:** ① *"100% coverage nghĩa là không còn bug"* · ② không nói thẳng *"ở công ty em không đo coverage"* mà cố tả như đã làm.

**Chốt:** *"Coverage chỉ ra chỗ chưa test, không chứng minh chỗ đã test là đúng. Em đọc cột Missing để tìm đường quan trọng chưa ai đi qua."*
</details>

#### BLD-042 · 🟡 · concept · ⭐ · [→ unit-test-and-code-quality §5](../../../06-build-systems/unit-test-and-code-quality.md)
**`-Werror`, static analysis, sanitizer, unit test, code review — mỗi cái bắt loại lỗi nào? Vì sao không chọn một cái là đủ?**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"Mỗi công cụ bắt một **loại** lỗi và có điểm mù riêng, nên phải chồng lên nhau. **Review** bắt logic, thiết kế, đặt tên — thứ máy không hiểu. **`-Wall -Wextra -Werror`** bắt code đáng ngờ mà compiler nhìn thấy được lúc build. **Phân tích tĩnh** bắt mẫu lỗi qua luồng code — rò rỉ, dereference `NULL` — nhưng hay báo nhầm. **ASan/UBSan** bắt lỗi bộ nhớ và hành vi không xác định **lúc chạy**, chỉ trên đường code đã chạy. **TSan** bắt data race đã xảy ra khi chạy. **Unit test** bắt hành vi sai so với mong đợi — chỉ những gì test đã nghĩ tới.*
>
> *Em có một bằng chứng tự đo ở lab: cố ý đọc phần tử thứ năm của một mảng bốn phần tử. Build Debug thì **im lặng**, chạy vẫn ra kết quả; build Release có tối ưu cộng `-Werror` thì compiler báo vượt biên và build hỏng; ASan thì bắt lúc chạy, chỉ đúng dòng. Và TSan trên cùng library bắt được một data race mà không công cụ nào khác thấy.*
>
> *Thứ tự em xếp theo **chi phí**: rẻ thì chạy mọi lần — warning, unit test; đắt thì chạy sau hoặc định kỳ — sanitizer, test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Coding standard ở chỗ em?"* — Không có bộ quy tắc riêng: library theo Modern C++, driver theo kernel coding style; kernel có `checkpatch.pl` để kiểm tự động.

**Móc nền tảng:** chọn sanitizer ([DBG-042](debugging.md)) · `-Warray-bounds` chỉ bật khi có tối ưu.

</details>

<details><summary>Đáp án</summary>

**Cơ chế:** mỗi công cụ nhìn code từ một góc khác nhau ⟹ mỗi cái có **điểm mù riêng**.

| Công cụ | Nhìn thấy | Điểm mù |
|---|---|---|
| Code review | Logic, thiết kế, đặt tên | Lỗi cơ học dễ trượt mắt |
| `-Wall -Wextra -Werror` | Code đáng ngờ **biết được lúc compile** | Lỗi chỉ lộ lúc chạy |
| Static analysis (clang-tidy, cppcheck, `-fanalyzer`; kernel: `checkpatch.pl`, `sparse`) | Mẫu lỗi qua phân tích luồng | Báo nhầm; không biết dữ liệu thật |
| ASan / UBSan | Lỗi bộ nhớ, UB **trên đường đã chạy** | Đường không chạy thì không thấy |
| TSan | Data race **đã xảy ra** khi chạy | Race không bị kích hoạt thì không thấy |
| Unit test | Hành vi sai so với mong đợi | Chỉ những gì test đã nghĩ tới |

**Bằng chứng chạy thật** (`libdisplay_lab`, cố ý đọc `zones[4]` trên mảng 4 phần tử):
- Build **Debug `-O0`**: **im lặng**, chạy ra `backlight=60`, exit 0 — trong khi phần cứng nhận giá trị rác (`set_brightness(37)` thay vì 45).
- Build **Release `-O2`** + `-Werror`: `error: array subscript 4 is above array bounds of 'int32_t [4]'` ⟹ build fail. *(Cảnh báo này cần tối ưu mới bật.)*
- **ASan**: `stack-buffer-overflow ... DimmingBackendChipA.cpp:8`.
- **TSan** trên cùng lab bắt được race vòng vsync mà không công cụ nào khác thấy.

**"Vì sao" hai tầng:**
- *Nông:* "càng nhiều công cụ càng tốt".
- *Sâu:* xếp công cụ theo **chi phí phát hiện**: rẻ chạy trước, mọi lần (warning, unit test); đắt chạy sau hoặc định kỳ (sanitizer, test trên thiết bị).

**Bẫy:** ① chạy `-Werror` chỉ trên Debug ⟹ mất các cảnh báo cần tối ưu · ② coi sanitizer xanh là "không có bug" — nó chỉ thấy đường đã chạy.

**Chốt:** *"Mỗi công cụ có điểm mù riêng nên phải chồng lên nhau. Rẻ thì chạy mọi lần, đắt thì chạy sau."*
</details>

#### BLD-043 · 🟡 · concept · ⭐ · 🏗️ · [→ unit-test-and-code-quality §3](../../../06-build-systems/unit-test-and-code-quality.md), [RES-016](resume.md)
**"Mỗi lần submit em đưa scenario cho AI, AI sinh unit test theo bộ khung có sẵn." — Vậy em kiểm chất lượng của test đó thế nào? Test xanh có nghĩa là gì?**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"AI viết nhanh, nhưng quyết định test **đủ hay chưa** vẫn là của em. Em kiểm ba thứ.*
>
> *Một, test có **kiểm kết quả** không: em đọc các dòng `EXPECT` — chúng kiểm kết quả mong đợi của scenario, hay chỉ gọi hàm cho chạy qua? Test không có kiểm tra vẫn xanh, và vẫn làm coverage tăng.*
>
> *Hai, **làm hỏng code thì test có đỏ không**: em cố ý đổi một điều kiện, ví dụ `>` thành `>=`, rồi chạy lại. Vẫn xanh thì test không bảo vệ gì.*
>
> *Ba, scenario có **ca lỗi** không: AI viết theo scenario em đưa; scenario chỉ có đường thành công thì test chỉ có đường thành công. Tham số sai, phần cứng trả lỗi — đó là trách nhiệm của người viết scenario, tức là em.*
>
> *Còn *"test xanh"* chỉ có nghĩa là code làm đúng những gì test kiểm. Một thay đổi của em chạy trên rất nhiều model, nên unit test xanh vẫn phải qua test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Làm hỏng code để kiểm test có tên gì?"* — Mutation testing; ở lab em thử bằng tay — đổi `zones[3]` thành `zones[4]` và xem cổng nào bắt được ([unit-test-and-code-quality §6](../../../06-build-systems/unit-test-and-code-quality.md)).

**Móc nền tảng:** dòng AI trên resume ([RES-016](resume.md)) · coverage ([BLD-041](build-systems.md)).

</details>

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn **sở hữu** chất lượng hay chỉ chuyển tiếp output của công cụ (cùng gốc với [RES-016](resume.md)).

**Câu trả lời tốt — ba phép kiểm, mỗi phép một câu:**
1. **Test có kiểm hành vi không?** Đọc các `EXPECT`: chúng kiểm **kết quả mong đợi** của scenario, hay chỉ gọi hàm cho chạy qua? Test không có `EXPECT` có ý nghĩa vẫn xanh — và vẫn tăng coverage ([BLD-041](build-systems.md)).
2. **Làm hỏng code, test có đỏ không?** Cố ý đổi một điều kiện (`>` thành `>=`), chạy lại. Vẫn xanh ⟹ test không bảo vệ gì. Đây là ý tưởng của *mutation testing*.
3. **Scenario có phủ đường lỗi không?** AI viết theo scenario bạn đưa. Scenario chỉ có đường thành công thì test chỉ có đường thành công — đường lỗi (tham số sai, phần cứng trả lỗi) là trách nhiệm của **người viết scenario**.

**Vế sau — test xanh nghĩa là gì:** chỉ nghĩa là *"code làm đúng những gì test kiểm"*. Một thay đổi chạy trên **rất nhiều model** ⟹ unit test xanh vẫn phải qua test trên thiết bị gốc.

**Nền kỹ thuật:** test double ([BLD-040](build-systems.md)) · coverage ([BLD-041](build-systems.md)).

**Bẫy:** ① *"AI viết test rồi nên em yên tâm"* · ② thanh minh dài · ③ không nêu được **một** cách cụ thể để biết test vô dụng.

**Chốt:** *"Em kiểm ba thứ: test có kiểm kết quả không, làm hỏng code thì test có đỏ không, và scenario có ca lỗi không. AI viết nhanh, còn quyết định test đủ hay chưa vẫn là của em."*
</details>

#### BLD-044 · 🟢 · concept · [→ unit-test-and-code-quality §3](../../../06-build-systems/unit-test-and-code-quality.md)
**GoogleTest ở library và KUnit ở driver — mỗi cái test cái gì, chạy ở đâu?**
<details><summary>🎙️ Trả lời mẫu</summary>

> *"**GoogleTest** dành cho library C++: test là một chương trình user-space bình thường chạy trên máy build, gọi thẳng các class và hàm của library, và thay phần phụ thuộc phần cứng bằng bản giả. Ví dụ em test thuật toán độ sáng bằng một backend giả ghi lại các giá trị được yêu cầu ghi.*
>
> *Ví dụ test của library:*
> ```cpp
> TEST(GlobalDimming, VSyncRampsInStepsOf10) {
>     GlobalDimmingForShm shm{};  FakeBackend be;
>     GlobalDimming algo(&be, shm);
>     algo.SetBacklight(25);
>     for (int i = 0; i < 5; ++i) algo.t_vSyncCallBack();
>     EXPECT_EQ(be.writes, (std::vector<int32_t>{10, 20, 25}));
> }
> ```
>
> *Code trong kernel không chạy được như một chương trình thường, nên driver dùng **KUnit** — framework test **bên trong kernel**. Test được build cùng kernel hoặc thành module và thường chạy trên một kernel giả lập, không cần board. KUnit test được **logic** của driver — kiểm tham số, bảng hàm, máy trạng thái — **không** test được phần cứng thật.*
>
> *Cả hai đều không thay được test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Ai viết test ở chỗ em?"* — Em viết scenario, AI sinh test theo bộ khung có sẵn, em kiểm lại ([BLD-043](build-systems.md)).

**Móc nền tảng:** test double ([BLD-040](build-systems.md)) · Bridge tạo chỗ cắm cho test ([DP-038](design-patterns.md)).

</details>

<details><summary>Đáp án</summary>

**GoogleTest** chạy như một chương trình user-space bình thường trên máy build; test class/hàm C++ của library, cắm fake thay phần phụ thuộc. **KUnit** là framework unit test **bên trong kernel**: test built-in hoặc module, thường chạy bằng `tools/testing/kunit/kunit.py run` trên kernel UML — không cần board; test **logic** của driver (kiểm tham số, bảng ops, máy trạng thái), **không** test được phần cứng thật. Cả hai đều không thay được test trên thiết bị.
</details>

#### BLD-045 · 🟡 · concept · ⭐ · 🎤 2026-10-11 · [→ unit-test-and-code-quality §6](../../../06-build-systems/unit-test-and-code-quality.md), [LNX-045](linux-sysprog.md)
**Một commit cài lỗi tràn mảng. Cổng ASan đáng lẽ đỏ sau 10 giây, nhưng pipeline treo 25 phút rồi mới báo timeout. Test của bạn có dùng shared memory + named semaphore của library. Chuyện gì đã xảy ra, và bạn sửa pipeline thế nào?**

<details><summary>🎙️ Trả lời mẫu</summary>

> *"Chuyện này em gặp thật ở lab của mình. Commit cài lỗi tràn mảng **bên trong một hàm API của library**. ASan phát hiện và **giết process ngay**, đúng lúc nó **đang giữ semaphore** dùng chung. Semaphore không có chủ, nên kernel không nhả hộ — nó kẹt ở 0. Cổng ASan lúc đó chạy test **lần thứ hai** để lấy mã thoát, và integration test dùng **cùng một tên shared memory cố định**, nên process mới treo ở `sem_wait` ngay lúc library khởi tạo. Test không có timeout, nên chờ tới hạn mặc định 1500 giây — 25 phút.*
>
> *Em sửa ba lớp, mỗi lớp chặn một mắt xích: **mỗi lần chạy pipeline một tên shared memory riêng** và dọn khi xong, để rác của lần trước không lan sang lần sau; **timeout 60 giây cho mọi lần chạy test**, để treo biến thành đỏ nhanh; và cổng chỉ chạy test **một lần**, giữ mã thoát của chính lần đó.*
>
> *Bài học: timeout chỉ biến *treo* thành *đỏ*; gốc là **trạng thái dùng chung sống lâu hơn process** rò từ test này sang test khác. Và cùng một điểm yếu thiết kế của library — semaphore không có chủ — đã lộ ra trong CI trước khi lộ trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Sao không chỉ tăng timeout của job Jenkins?"* — Vẫn treo, chỉ treo lâu hơn hoặc ngắn hơn; và che mất lỗi thật.

**Móc nền tảng:** process chết khi giữ semaphore ([LNX-045](linux-sysprog.md)) · flaky test ([BLD-026](build-systems.md)).

</details>

<details><summary>Đáp án</summary>

**Cơ chế — xảy ra thật ở `libdisplay_lab` ([unit-test-and-code-quality §6](../../../06-build-systems/unit-test-and-code-quality.md)):**
1. ASan phát hiện tràn mảng **bên trong `lib_api_set_backlight`** ⟹ giết process ngay (`-fno-sanitize-recover`). Lúc đó process **đang giữ semaphore**.
2. Semaphore không có chủ ⟹ kernel không nhả hộ ⟹ giá trị kẹt ở 0 ([LNX-045](linux-sysprog.md)).
3. Cổng ASan chạy `ctest` **lần hai** (để lấy exit code), integration test dùng **cùng tên shm cố định** ⟹ process mới treo ở `sem_wait` ngay trong constructor của `.so`.
4. `ctest` không đặt timeout ⟹ chờ tới hạn mặc định **1500 giây**.

**Sửa — ba lớp, mỗi lớp chặn một mắt xích:**

| Lớp | Làm gì | Chặn mắt xích |
|---|---|---|
| Cô lập | Mỗi lần chạy pipeline một tên shm riêng (`/libdisplay_ci_$$`), dọn khi xong | Rác của lần chạy trước (hoặc test trước) không lan sang lần sau |
| Timeout | `ctest --timeout 60` ở **mọi** cổng | Treo thành **FAIL** nhanh, không chiếm máy CI 25 phút |
| Không chạy lặp | Cổng chạy test **một lần**, giữ exit code của chính lần đó | Không tạo ra lần chạy thứ hai để dính rác của lần đầu |

**"Vì sao" hai tầng:**
- *Nông:* "test bị treo thì thêm timeout".
- *Sâu:* timeout chỉ biến **treo** thành **đỏ**; gốc là **trạng thái dùng chung sống lâu hơn process** (shm, semaphore, file lock, port) rò từ test này sang test khác. Test tốt phải **cô lập** tài nguyên đó. Và cùng một lỗi thiết kế của hệ thật (semaphore không có chủ) lộ ra **trong CI** trước khi lộ trên TV.

**Bẫy:**
1. Chỉ tăng timeout của job Jenkins — vẫn treo, chỉ treo lâu hơn hoặc ngắn hơn.
2. Đổ lỗi "flaky" rồi bấm chạy lại — lần sau dùng tên shm mới thì xanh, che mất lỗi thật.
3. Quên rằng ASan **giết** process: mọi tài nguyên process đang giữ mà kernel không tự dọn (semaphore POSIX, shm) sẽ ở lại.

**Chốt:** *"Cổng chặn không bao giờ được treo — treo phải thành đỏ. Muốn vậy cần hai thứ: timeout ở mọi lần chạy test, và mỗi lần chạy một bộ tài nguyên IPC riêng."*
</details>

---

⬅️ [Bank index](README.md)
