# Cross-Compilation — Build cho kiến trúc khác

> **TL;DR**
> - **Cross-compile** = build trên một máy (host) để tạo binary chạy trên máy khác kiến trúc (target), vd build trên PC x86-64 cho board ARM.
> - 3 khái niệm: **build** (máy biên dịch), **host** (máy chạy binary), **target** (kiến trúc binary tạo ra sinh code cho — chủ yếu liên quan compiler/tooling). Trong cross-compile thường gặp: build = x86-64, host = ARM.
> - Cần **cross toolchain** (compiler + linker + libc cho target, vd `arm-linux-gnueabihf-gcc`) và **sysroot** (bản sao header + thư viện của target để link đúng).
> - CMake cross-compile qua **toolchain file** (`-DCMAKE_TOOLCHAIN_FILE=...`) khai báo compiler, sysroot, kiến trúc.
> - Hệ sinh ảnh embedded: **Yocto** (mạnh, tùy biến cao, học khó), **Buildroot** (đơn giản, nhanh) tự lo toolchain + rootfs.

---

## 1. Vì sao cross-compile?

Board embedded (ARM, RISC-V, MIPS) thường **yếu** (CPU chậm, ít RAM/flash) — biên dịch ngay trên đó (native) rất chậm hoặc bất khả thi, và nhiều board còn chẳng có toolchain/đủ chỗ. Giải pháp: dùng **máy host mạnh (PC x86-64)** để biên dịch, tạo binary cho **target ARM**, rồi copy sang board chạy. Đó là cross-compilation.

---

## 2. build / host / target — đừng nhầm

Thuật ngữ chuẩn (GNU):
- **build**: máy *thực hiện việc biên dịch* (nơi compiler chạy).
- **host**: máy mà *binary tạo ra sẽ chạy trên đó*.
- **target**: kiến trúc mà *binary tạo ra sẽ sinh code cho* — chỉ có ý nghĩa khi bản thân sản phẩm là một compiler/công cụ.

| Trường hợp | build | host | Ví dụ |
|------------|-------|------|-------|
| Native | x86-64 | x86-64 | Build app PC chạy trên PC |
| **Cross** | x86-64 | ARM | Build trên PC cho board ARM |

Đa số tình huống embedded chỉ cần phân biệt **build (x86-64)** vs **host (ARM)**.

---

## 3. Cross toolchain & sysroot

**Cross toolchain**: bộ công cụ chạy trên host build nhưng sinh mã cho target. Tên thường theo dạng **`<arch>-<vendor>-<os>-<abi>`**:
```
arm-linux-gnueabihf-gcc       # ARM, Linux, EABI hard-float
aarch64-linux-gnu-gcc         # ARM 64-bit
riscv64-unknown-elf-gcc       # RISC-V bare-metal (elf = không OS)
```
Gồm: cross compiler (`gcc`/`g++`), linker, assembler (binutils), và **libc cho target** (glibc/musl/newlib).

**Sysroot**: một thư mục chứa **bản sao header + thư viện của target** (như cây `/usr` của board). Khi link, compiler tìm thư viện trong sysroot (không phải của máy host x86-64 — nếu nhầm sẽ link sai kiến trúc):
```sh
arm-linux-gnueabihf-g++ main.cpp -o app \
    --sysroot=/path/to/target-sysroot \
    -I.../include -L.../lib -lfoo
```

> Bug kinh điển: link nhầm thư viện của host → lỗi "wrong architecture" hoặc chạy crash trên board. Sysroot đúng giải quyết việc này.

---

## 4. Cross-compile với CMake: toolchain file

CMake cross-compile bằng một **toolchain file** mô tả môi trường target, truyền lúc configure:

```cmake
# arm-toolchain.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_SYSROOT /path/to/target-sysroot)

# Tìm thư viện/header trong sysroot, không phải host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # chương trình build vẫn dùng của host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # thư viện: chỉ trong sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # header: chỉ trong sysroot
```
```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake
cmake --build build
```

- `CMAKE_SYSTEM_NAME` được set ⇒ CMake biết đang cross-compile (bật `CMAKE_CROSSCOMPILING`).
- `CMAKE_FIND_ROOT_PATH_MODE_*` quan trọng: bắt `find_*`/`find_package` tìm trong sysroot của target, tránh lẫn với host.

---

## 5. Thách thức riêng của cross-compile

- **Không chạy được binary target trên host** → test phải qua **QEMU** (giả lập) hoặc chạy thẳng trên board. Các bước build cố gắng "chạy thử" (try-run) phải khai báo kết quả thủ công.
- **Endianness / word size / alignment** khác giữa host và target → code phụ thuộc layout (cast con trỏ, đọc struct nhị phân) dễ vỡ.
- **Thư viện phụ thuộc** cũng phải có bản cross-compiled cho target (trong sysroot) — không dùng `apt install` của host được.
- **Float**: hard-float vs soft-float ABI phải khớp toàn bộ (lẫn lộn → link/runtime lỗi).

---

## 6. Yocto vs Buildroot — tạo cả hệ điều hành nhúng

Khi cần cả **rootfs + kernel + toolchain** cho sản phẩm, người ta dùng build system chuyên dụng:

| | Buildroot | Yocto Project |
|--|-----------|---------------|
| Triết lý | Đơn giản, tạo image nhanh | Linh hoạt, công nghiệp, tùy biến sâu |
| Cấu hình | Kiểu `menuconfig` (như kernel) | Layer + recipe (BitBake) |
| Học | Dễ, nhanh có kết quả | Dốc, nhiều khái niệm |
| Phù hợp | Sản phẩm nhỏ/gọn, prototyping | Sản phẩm lớn, nhiều biến thể, bảo trì dài hạn |
| Sản phẩm | toolchain + rootfs image | SDK + image + package feed |

Cả hai đều tự sinh **cross toolchain + sysroot** phù hợp, giải quyết khâu khó nhất của cross-compile.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Cross-compilation là gì và vì sao cần trong embedded?</summary>

Cross-compilation là biên dịch trên một máy (host build, vd PC x86-64) để tạo ra binary chạy trên một máy có kiến trúc khác (target, vd board ARM). Cần trong embedded vì board target thường yếu (CPU chậm, ít RAM/flash) hoặc thiếu toolchain, khiến biên dịch native trên board rất chậm hoặc bất khả thi; dùng máy host mạnh để build nhanh rồi copy binary sang board là cách thực tế. Nó đòi hỏi một cross toolchain (compiler sinh mã cho target) và sysroot (header + thư viện của target để link đúng).
</details>

<details><summary>2) Phân biệt build, host, target machine.</summary>

build là máy thực hiện việc biên dịch (nơi compiler chạy). host là máy mà binary tạo ra sẽ chạy trên đó. target là kiến trúc mà binary tạo ra sẽ **sinh mã cho** — khái niệm này chỉ có ý nghĩa khi sản phẩm build chính là một compiler/toolchain (vd build một cross-compiler). Trong cross-compile ứng dụng thông thường, ta chỉ cần phân biệt build (vd x86-64) khác host (vd ARM); target trùng host. Trường hợp build = host là biên dịch native.
</details>

<details><summary>3) Sysroot là gì và vì sao quan trọng khi cross-compile?</summary>

Sysroot là một thư mục chứa bản sao hệ thống file của target — chủ yếu là header và thư viện (giống cây `/usr`, `/lib` của board). Khi cross-compile, compiler và linker phải tìm header/thư viện của **target** trong sysroot, chứ không phải của máy host (vốn là kiến trúc khác). Nếu thiếu sysroot hoặc cấu hình sai, build dễ vô tình link với thư viện x86-64 của host → lỗi "wrong architecture" hoặc binary crash trên board. Sysroot đảm bảo toàn bộ quá trình link khớp với môi trường target.
</details>

<details><summary>4) CMake cross-compile bằng cách nào?</summary>

Qua một **toolchain file** truyền vào lúc configure bằng `-DCMAKE_TOOLCHAIN_FILE=...`. File này khai báo `CMAKE_SYSTEM_NAME` (đặt giá trị này báo cho CMake biết đang cross-compile), `CMAKE_SYSTEM_PROCESSOR`, đường dẫn cross compiler (`CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER`), `CMAKE_SYSROOT`, và các `CMAKE_FIND_ROOT_PATH_MODE_*` để buộc `find_package`/`find_library` tìm trong sysroot của target thay vì host. Nhờ tách cấu hình môi trường ra toolchain file, cùng một CMakeLists.txt build được cả native lẫn nhiều target khác nhau.
</details>

<details><summary>5) Những khó khăn đặc thù khi cross-compile là gì?</summary>

Một số điểm chính: (1) không chạy được binary target trên host nên test phải qua giả lập QEMU hoặc trực tiếp trên board, và các bước build cần "chạy thử" phải khai kết quả thủ công. (2) Khác biệt endianness, word size (32/64-bit), alignment giữa host và target làm code phụ thuộc layout nhị phân dễ lỗi. (3) Mọi thư viện phụ thuộc phải có bản đã cross-compiled cho target trong sysroot — không dùng package của host được. (4) Phải khớp ABI, đặc biệt hard-float vs soft-float; lẫn lộn gây lỗi link hoặc runtime. Các hệ như Yocto/Buildroot ra đời để quản lý những phức tạp này một cách hệ thống.
</details>

<details><summary>6) Yocto và Buildroot khác nhau thế nào?</summary>

Cả hai là build system tạo ra hệ điều hành Linux nhúng hoàn chỉnh (toolchain + kernel + rootfs) cho target. Buildroot đơn giản, cấu hình kiểu `menuconfig`, học nhanh và tạo image gọn — phù hợp sản phẩm nhỏ, prototyping, khi yêu cầu tùy biến vừa phải. Yocto Project phức tạp hơn với mô hình layer + recipe (BitBake), học khó nhưng rất linh hoạt và mạnh cho môi trường công nghiệp: quản lý nhiều biến thể phần cứng, tạo SDK, package feed, bảo trì dài hạn và tái lập build. Chọn Buildroot khi cần nhanh gọn; chọn Yocto khi cần tùy biến sâu và quy mô lớn.
</details>

---
⬅️ [cmake.md](cmake.md) · ➡️ Tiếp theo: [07-shared-libraries/](../07-shared-libraries/)
