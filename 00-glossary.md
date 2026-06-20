# 00 — Glossary (Thuật ngữ tra cứu nhanh)

Giải thích ngắn gọn các thuật ngữ tiếng Anh dùng xuyên suốt tài liệu, kèm link về topic giải thích sâu. Dùng để tra nhanh khi gặp thuật ngữ chưa rõ. Sắp xếp theo nhóm chủ đề.

> Quy ước: định nghĩa một câu. Muốn hiểu sâu → theo link tới topic gốc.

---

## C/C++ & Modern C++

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **RAII** | Ràng buộc vòng đời tài nguyên vào vòng đời object (ctor giành, dtor trả) | [02](02-modern-cpp/raii-smart-pointers.md) |
| **UB** (Undefined Behavior) | Hành vi chuẩn không định nghĩa; compiler được làm bất cứ gì | [01](01-cpp-fundamentals/memory-model.md) |
| **vtable / vptr** | Bảng con trỏ hàm virtual / con trỏ ẩn trong object trỏ tới vtable | [01](01-cpp-fundamentals/oop.md) |
| **ODR** (One Definition Rule) | Mỗi entity chỉ được định nghĩa đúng một lần toàn chương trình | [06](06-build-systems/makefile.md) |
| **lvalue / rvalue** | Biểu thức có tên/địa chỉ bền / giá trị tạm sắp hết hạn | [02](02-modern-cpp/move-semantics.md) |
| **Move semantics** | "Ăn cắp" tài nguyên thay vì copy sâu | [02](02-modern-cpp/move-semantics.md) |
| **Perfect forwarding** | Truyền tham số giữ nguyên tính lvalue/rvalue | [02](02-modern-cpp/move-semantics.md) |
| **RVO** | Tối ưu bỏ qua copy/move khi trả về object | [02](02-modern-cpp/move-semantics.md) |
| **Smart pointer** | `unique_ptr`/`shared_ptr`/`weak_ptr` — quản lý heap tự động | [02](02-modern-cpp/raii-smart-pointers.md) |
| **Data race** | ≥2 thread truy cập cùng dữ liệu, ≥1 ghi, không đồng bộ → UB | [02](02-modern-cpp/concurrency.md) |
| **Name mangling** | Mã hóa kiểu vào tên symbol để hỗ trợ overload | [07](07-shared-libraries/linking-loading.md) |
| **Pimpl** | Giấu data member sau con trỏ Impl (bảo vệ ABI) | [07](07-shared-libraries/api-design.md) |

## Operating System

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **Process / Thread** | Chương trình chạy có address space riêng / luồng trong process chia sẻ address space | [03](03-operating-system/process-thread.md) |
| **Context switch** | Lưu/khôi phục trạng thái CPU khi đổi tác vụ | [03](03-operating-system/process-thread.md) |
| **Virtual memory** | Mỗi process thấy không gian địa chỉ ảo riêng, MMU ánh xạ tới vật lý | [03](03-operating-system/memory-management.md) |
| **MMU / TLB** | Phần cứng dịch địa chỉ ảo→vật lý / cache của page table | [03](03-operating-system/memory-management.md) |
| **Page fault** | Trap khi truy cập page chưa có trong RAM (minor/major/invalid) | [03](03-operating-system/memory-management.md) |
| **Mutex / Spinlock** | Khóa ngủ khi chờ / khóa bận xoay khi chờ | [03](03-operating-system/sync-primitives.md) |
| **Semaphore** | Bộ đếm wait/signal không ownership, báo hiệu/quản N tài nguyên | [03](03-operating-system/sync-primitives.md) |
| **Deadlock** | Các thread chờ vòng tròn tài nguyên, kẹt vĩnh viễn | [03](03-operating-system/sync-primitives.md) |
| **CFS** | Completely Fair Scheduler — chia CPU công bằng theo vruntime | [03](03-operating-system/scheduling.md) |
| **Priority inversion** | Tác vụ cao bị tác vụ thấp chặn gián tiếp | [03](03-operating-system/scheduling.md) |
| **IPC** | Inter-Process Communication: pipe, shm, mq, socket, signal | [03](03-operating-system/ipc.md) |
| **COW** (Copy-on-write) | Chia sẻ page read-only, chỉ copy khi ghi | [03](03-operating-system/process-thread.md) |

## Linux System Programming

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **File descriptor (fd)** | Số nguyên định danh tài nguyên I/O đang mở | [04](04-linux-system-programming/file-io.md) |
| **Syscall** | Cổng để user yêu cầu kernel làm việc đặc quyền | [04](04-linux-system-programming/file-io.md) |
| **epoll** | Cơ chế I/O multiplexing scale của Linux | [04](04-linux-system-programming/io-multiplexing.md) |
| **Level/Edge triggered** | epoll báo liên tục / chỉ khi chuyển trạng thái | [04](04-linux-system-programming/io-multiplexing.md) |
| **Event loop** | Một thread + non-blocking I/O + epoll phục vụ nhiều kết nối | [04](04-linux-system-programming/io-multiplexing.md) |
| **Zombie / Orphan** | Con đã chết chưa được wait / con bị mất cha | [03](03-operating-system/process-thread.md) |
| **async-signal-safe** | Hàm an toàn gọi trong signal handler | [04](04-linux-system-programming/processes-signals.md) |
| **Page cache** | Kernel cache nội dung file trong RAM | [04](04-linux-system-programming/file-io.md) |

## Drivers, Device Tree & Embedded

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **Kernel module** | Code kernel nạp/gỡ động (`.ko`) | [05](05-drivers-device-tree/driver-basics.md) |
| **file_operations** | Struct ánh xạ open/read/write của user xuống driver | [05](05-drivers-device-tree/driver-basics.md) |
| **Major/Minor number** | Số xác định driver / instance thiết bị | [05](05-drivers-device-tree/driver-basics.md) |
| **probe()** | Hàm khởi tạo khi device khớp driver | [05](05-drivers-device-tree/driver-basics.md) |
| **Top/Bottom half** | Phần ISR nhanh / phần xử lý nặng hoãn lại | [05](05-drivers-device-tree/driver-basics.md) |
| **copy_to/from_user** | Truy cập an toàn con trỏ user trong kernel | [05](05-drivers-device-tree/kernel-userspace.md) |
| **Device tree (DTS/DTB)** | Mô tả phần cứng dạng dữ liệu cho kernel | [05](05-drivers-device-tree/device-tree.md) |
| **compatible** | Property khớp device tree node với driver | [05](05-drivers-device-tree/device-tree.md) |
| **SoC** | System on Chip — tích hợp CPU + peripheral một chip | [08](08-embedded-systems/architecture.md) |
| **MCU / MPU** | Vi điều khiển (không MMU, RTOS) / vi xử lý (có MMU, Linux) | [08](08-embedded-systems/architecture.md) |
| **MMIO** | Memory-mapped I/O — thanh ghi ánh xạ vào không gian địa chỉ | [08](08-embedded-systems/architecture.md) |
| **DMA** | Chuyển dữ liệu peripheral↔RAM không cần CPU | [08](08-embedded-systems/architecture.md) |
| **U-Boot / SPL** | Bootloader chính / giai đoạn 1 khởi tạo DRAM | [08](08-embedded-systems/boot-process.md) |
| **RTOS** | Real-Time OS — tất định, cho hard realtime | [08](08-embedded-systems/rtos-vs-linux.md) |
| **Watchdog** | Timer reset thiết bị khi hệ treo | [08](08-embedded-systems/constraints.md) |

## Build & Libraries

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **Translation unit (TU)** | Một `.cpp` sau preprocess, đơn vị biên dịch | [06](06-build-systems/makefile.md) |
| **Linking** | Phân giải symbol + relocate để tạo binary | [07](07-shared-libraries/linking-loading.md) |
| **Static / Shared lib** | `.a` nhúng lúc build / `.so` nạp lúc runtime | [07](07-shared-libraries/static-vs-shared.md) |
| **PLT / GOT** | Bảng stub gọi hàm / bảng địa chỉ symbol (lazy binding) | [07](07-shared-libraries/linking-loading.md) |
| **API / ABI** | Hợp đồng mức source / mức nhị phân | [07](07-shared-libraries/abi-versioning.md) |
| **soname** | Tên nhúng trong `.so` mã hóa major version (mức ABI) | [07](07-shared-libraries/abi-versioning.md) |
| **CMake** | Meta-build sinh ra build system (Make/Ninja...) | [06](06-build-systems/cmake.md) |
| **Cross-compile** | Build trên host cho target khác kiến trúc | [06](06-build-systems/cross-compilation.md) |
| **Sysroot** | Bản sao header+thư viện của target để link đúng | [06](06-build-systems/cross-compilation.md) |

## Debugging

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **Core dump** | Ảnh chụp bộ nhớ lúc crash để phân tích post-mortem | [09](09-debugging/gdb.md) |
| **Watchpoint** | gdb dừng khi một biến bị thay đổi | [09](09-debugging/gdb.md) |
| **ASan / Valgrind** | Công cụ bắt lỗi bộ nhớ (sanitizer / runtime instrumentation) | [09](09-debugging/memory-bugs.md) |
| **TSan** | ThreadSanitizer — bắt data race | [09](09-debugging/memory-bugs.md) |
| **strace / ltrace** | Theo dõi syscall / lời gọi thư viện | [09](09-debugging/tools.md) |
| **perf** | Profiler hiệu năng (hotspot, cache miss) | [09](09-debugging/tools.md) |
| **Heisenbug** | Bug biến mất khi quan sát (thường race/UB) | [09](09-debugging/mindset.md) |
| **Kernel oops/panic** | Lỗi kernel nghiêm trọng / sập hẳn | [09](09-debugging/kernel-debugging.md) |
| **ftrace** | Tracer tích hợp kernel | [09](09-debugging/kernel-debugging.md) |

## Networking

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **TCP / UDP** | Tin cậy hướng kết nối / không kết nối, nhẹ | [14](14-networking/tcp-ip.md) |
| **Three-way handshake** | SYN → SYN-ACK → ACK thiết lập kết nối TCP | [14](14-networking/tcp-ip.md) |
| **Socket** | Endpoint giao tiếp (TCP/UDP/Unix) | [14](14-networking/sockets-and-protocols.md) |
| **HTTP / TLS** | Giao thức request/response / lớp mã hóa (→HTTPS) | [14](14-networking/sockets-and-protocols.md) |
| **MQTT** | Giao thức pub/sub nhẹ cho IoT | [14](14-networking/sockets-and-protocols.md) |
| **Flow / Congestion control** | Bảo vệ bên nhận / bảo vệ mạng | [14](14-networking/tcp-ip.md) |

## Design & Thinking

| Thuật ngữ | Giải thích ngắn | Chi tiết |
|-----------|-----------------|----------|
| **SOLID** | 5 nguyên lý thiết kế OOP (SRP/OCP/LSP/ISP/DIP) | [12](12-design-patterns/solid-principles.md) |
| **Design pattern** | Giải pháp tái sử dụng cho vấn đề thiết kế lặp lại | [12](12-design-patterns/README.md) |
| **Dependency injection** | Inject implementation qua interface để dễ test | [12](12-design-patterns/solid-principles.md) |
| **First principles** | Suy luận từ nguyên lý cơ bản thay vì khuôn mẫu | [10](10-thinking/problem-solving.md) |
| **Trade-off** | Đánh đổi giữa các mục tiêu tùy ràng buộc | [10](10-thinking/problem-solving.md) |
| **HAL** | Hardware Abstraction Layer — tách logic khỏi phần cứng | [10](10-thinking/system-design.md) |
| **Big-O** | Độ phức tạp time/space theo kích thước input | [13](13-dsa/complexity-and-structures.md) |

---
➡️ Về [README chính](README.md) · [CLAUDE.md](CLAUDE.md)
