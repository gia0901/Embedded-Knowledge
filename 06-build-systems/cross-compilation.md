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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [BLD-013](../14-prep/mock-interview/bank/build-systems.md) | Cross-compilation là gì và vì sao cần trong embedded? |
| [BLD-013](../14-prep/mock-interview/bank/build-systems.md) | Phân biệt build, host, target machine. |
| [BLD-014](../14-prep/mock-interview/bank/build-systems.md) | Sysroot là gì và vì sao quan trọng khi cross-compile? |
| [BLD-003](../14-prep/mock-interview/bank/build-systems.md) | CMake cross-compile bằng cách nào? |
| [BLD-015](../14-prep/mock-interview/bank/build-systems.md) | Những khó khăn đặc thù khi cross-compile là gì? |
| [BSP-017](../14-prep/mock-interview/bank/bsp.md) | Yocto và Buildroot khác nhau thế nào? |

---
⬅️ [cmake.md](cmake.md) · ➡️ Tiếp theo: [07-shared-libraries/](../07-shared-libraries/)
