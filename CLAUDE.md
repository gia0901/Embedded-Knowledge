# Embedded Interview Course

**Bộ tài liệu đầy đủ - toàn diện - cô đọng kiến thức nền tảng và tư duy cần thiết để up level bản thân từ Middle → Senior Software Engineer (Embedded Linux, C++).**

> File này là "bộ não" của dự án. Khi một conversation kết thúc và bắt đầu conversation mới, đọc file này là đủ để hiểu mục tiêu, cấu trúc tài liệu, quy ước viết, tiến độ và vai trò của trợ lý (Claude).

---

## 1. Trạng thái dự án

- **Giai đoạn hiện tại:** ✅ Đã viết xong toàn bộ 11 topic (bản đầu đầy đủ). Sang giai đoạn mentor: giải đáp & review câu trả lời, tinh chỉnh nội dung khi cần.
- File này đã được hoàn thiện (không còn là bản nháp mơ hồ). Vẫn có thể tinh chỉnh khi phát sinh nhu cầu mới.
- Xem **Mục 7 — Tiến độ** để biết tài liệu nào đã/đang/chưa viết.

---

## 2. Về người học (background)

- System Software Engineer, **3 năm** kinh nghiệm phát triển phần mềm cho Linux OS.
- Làm ở tầng **System Software** (C++ shared library), viết **C++ API interface** cho lớp trên.
- Phát triển **Kernel driver** (Kernel API) tương tác với **SoC driver**.
- Nền tảng: C/C++ cơ bản, Linux system programming, một chút device driver, device tree, build system (CMake, Makefile).
- DSA ổn: Leetcode easy tốt, medium tàm tạm.
- Design pattern: mới biết vài pattern đơn giản (singleton...).
- **Điểm yếu cần cải thiện:** kỹ năng debug (hiện chủ yếu đọc log + so code + suy luận).

---

## 3. Mục tiêu

- **Ngắn hạn:** Chuẩn bị đầy đủ cho technical interview (mốc **1-3 tháng**).
- **Trung hạn:** Nâng trình lên **Middle+** (hướng Senior).
- **Dài hạn:** Tư duy cốt lõi/nền tảng thay vì kiến thức bề nổi; làm việc theo hướng **AI-driven** — nắm rõ bản chất & kiến trúc cốt lõi, dùng AI cho các tác vụ nhỏ/lặp lại.

---

## 4. Quy ước viết tài liệu (rất quan trọng — áp dụng cho mọi file)

1. **Ngôn ngữ:** Tiếng Việt để giải thích; **giữ nguyên thuật ngữ kỹ thuật tiếng Anh** (mutex, scheduling, page fault, RAII...). Không dịch ép thuật ngữ.
2. **Độ chi tiết:** **Cân bằng** — giải thích *bản chất* + *ví dụ thực tế*, nhưng vẫn cô đọng. Tránh dài dòng học thuật, tránh liệt kê suông.
3. Với mỗi khái niệm, cố gắng làm rõ:
   - **Bản chất** vấn đề (tại sao tồn tại, giải quyết gì).
   - **Khi nào dùng / khi nào không.**
   - **So sánh & lựa chọn** giữa các phương án (thường bằng bảng).
   - **Ví dụ thực tế / code minh hoạ ngắn.**
4. **Hình vẽ & sơ đồ:** ưu tiên ASCII diagram hoặc Mermaid khi giúp dễ hình dung (memory layout, kiến trúc, luồng xử lý, state machine...).
5. **Code:** ngắn gọn, đúng trọng tâm, có comment giải thích chỗ quan trọng. Mặc định **C++ hiện đại (C++17)** trừ khi chủ đề yêu cầu khác.
6. Mỗi tài liệu con nên có: phần tóm tắt đầu file (TL;DR) + danh sách "câu hỏi phỏng vấn liên quan" cuối file (link tới topic 11).
7. **Đáp án câu hỏi phỏng vấn PHẢI được ẩn đi** để người học tự tư duy trước, dùng cú pháp:
   ```markdown
   <details><summary>Đáp án</summary>

   ... nội dung đáp án đầy đủ ...

   </details>
   ```

---

## 5. Cấu trúc thư mục & topic

> Mỗi topic là một thư mục; bên trong chia thành các tài liệu con `.md` hợp lý. Đánh số để giữ thứ tự đọc.

```
Embedded-Interview/
├── CLAUDE.md                     # File này — bộ não dự án
├── README.md                     # Mục lục tổng + lộ trình học gợi ý
│
├── 01-cpp-fundamentals/          # C/C++ nền tảng
│   ├── memory-model.md           # stack/heap, con trỏ, tham chiếu, layout
│   ├── oop.md                    # class, kế thừa, virtual, vtable
│   ├── templates.md              # template, generic programming
│   └── ...
│
├── 02-modern-cpp/                # Modern C++ (C++11/14/17/20)
│   ├── raii-smart-pointers.md    # RAII, unique/shared/weak_ptr
│   ├── move-semantics.md         # move, rvalue ref, perfect forwarding
│   ├── concurrency.md            # thread, mutex, atomic, memory order
│   ├── lambdas-functional.md
│   └── ...
│
├── 03-operating-system/          # Bản chất OS
│   ├── process-thread.md         # process vs thread, context switch
│   ├── scheduling.md             # scheduler, ưu tiên, realtime
│   ├── memory-management.md      # virtual memory, paging, MMU
│   ├── ipc.md                    # pipe, shm, message queue, socket
│   ├── sync-primitives.md        # mutex, semaphore, spinlock, deadlock
│   └── ...
│
├── 04-linux-system-programming/  # Lập trình hệ thống Linux
│   ├── file-io.md                # fd, syscall, blocking/non-blocking
│   ├── processes-signals.md      # fork/exec/wait, signal
│   ├── ipc-linux.md
│   ├── io-multiplexing.md        # select/poll/epoll
│   └── ...
│
├── 05-drivers-device-tree/       # Device driver & Device Tree
│   ├── driver-basics.md          # char/block/net driver, module
│   ├── kernel-userspace.md       # ranh giới kernel/user, ioctl, sysfs
│   ├── device-tree.md
│   └── ...
│
├── 06-build-systems/             # Build system
│   ├── makefile.md
│   ├── cmake.md
│   └── cross-compilation.md      # toolchain, cross-compile cho embedded
│
├── 07-shared-libraries/          # Thư viện chia sẻ
│   ├── static-vs-shared.md
│   ├── linking-loading.md        # symbol, PLT/GOT, dynamic loader
│   ├── abi-versioning.md         # ABI, soname, versioning
│   └── api-design.md             # thiết kế C++ API interface tốt
│
├── 08-embedded-systems/          # Hệ thống nhúng
│   ├── architecture.md           # SoC, bus, memory-mapped IO
│   ├── boot-process.md           # bootloader → kernel → init
│   ├── rtos-vs-linux.md
│   └── constraints.md            # power, memory, realtime constraints
│
├── 09-debugging/                 # Kỹ năng debug (điểm yếu — đầu tư kỹ)
│   ├── mindset.md                # tư duy debug có hệ thống
│   ├── gdb.md
│   ├── tools.md                  # valgrind, strace, ltrace, perf, addr2line
│   ├── memory-bugs.md            # leak, corruption, UB, sanitizers
│   └── kernel-debugging.md       # dmesg, ftrace, crash dump
│
├── 10-thinking/                  # Tư duy (xuyên suốt)
│   ├── problem-solving.md        # phương pháp giải quyết vấn đề
│   └── system-design.md          # thiết kế hệ thống hướng Embedded Linux
│
├── 11-interview-questions/       # Câu hỏi phỏng vấn (đáp án ẩn)
│   ├── README.md                 # cách dùng, phân loại theo độ khó
│   ├── cpp.md
│   ├── operating-system.md
│   ├── linux.md
│   ├── drivers.md
│   ├── debugging.md
│   ├── design-patterns.md
│   ├── dsa.md
│   ├── networking.md
│   └── system-design.md
│
├── 12-design-patterns/           # Mẫu thiết kế (bổ sung)
│   ├── solid-principles.md       # SOLID — nền tảng
│   ├── creational.md             # Singleton, Factory, Builder
│   ├── structural.md             # Adapter, Bridge/Pimpl, Facade, Proxy
│   └── behavioral.md             # Strategy, Observer, State, Command
│
├── 13-dsa/                       # Data Structures & Algorithms (bổ sung)
│   ├── complexity-and-structures.md  # Big-O, container, trade-off
│   └── algorithm-patterns.md     # two-pointer, sliding window, BFS/DFS, DP
│
├── 14-networking/                # Mạng (bổ sung)
│   ├── tcp-ip.md                 # mô hình, TCP vs UDP, handshake
│   └── sockets-and-protocols.md  # socket API, HTTP/TLS/MQTT
│
├── 15_prep/                      # Chuẩn bị phỏng vấn thực tế
│   ├── study-plans/              # ⭐ Chiến lược ôn theo 2 vị trí: BSP & C++ (sprint + nền tảng) + gap-register
│   └── technical_round/          # Lý thuyết cốt lõi, question bank (C++/BSP), behavioral STAR
│
├── 16-book-summaries/            # Tóm tắt sách chuyên ngành theo cụm chủ đề
│   └── README.md                 # ⚠️ QUY ƯỚC RIÊNG của topic này — đọc trước khi viết/sửa file trong đó
│
├── 00-glossary.md                # Tra cứu nhanh thuật ngữ EN ↔ giải thích
└── OVERVIEW.md                   # Bản đồ tư duy: các topic kiến thức (01–14) liên kết thế nào
```

> **Hai tầng "bức tranh tổng thể":** `OVERVIEW.md` nối *toàn bộ* các topic kiến thức (01–14); mỗi README topic có mục "🗺️ Bức tranh tổng thể" nối *các file con* trong topic đó. Đây là phần thể hiện "hiểu kiến trúc cốt lõi" thay vì kiến thức rời rạc.

> Danh sách file con ở trên là **khung gợi ý**, sẽ điều chỉnh/thêm bớt khi viết. Topic mới có thể bổ sung nếu cần.
> Topic 12–14 + glossary là **bổ sung sau** so với kế hoạch gốc (Design Patterns, DSA, Networking) theo background người học.

---

## 6. Topic Interview Questions — quy ước riêng

- Phân loại theo **độ khó**: `Cơ bản` → `Trung bình` → `Khó` → `Mở rộng/Senior`.
- Mỗi câu: đề bài rõ ràng → (tuỳ chọn) gợi ý → **đáp án đầy đủ ẩn trong `<details>`**.
- Đáp án phải đủ sâu để học được bản chất, không chỉ trả lời cụt.
- Liên kết ngược về tài liệu nền tảng tương ứng để tra cứu khi cần.

---

## 7. Tiến độ (cập nhật mỗi khi hoàn thành một mục)

| Topic | Trạng thái | Ghi chú |
|-------|-----------|---------|
| CLAUDE.md | ✅ Xong | Đã hoàn thiện |
| README.md (mục lục + lộ trình) | ✅ Xong | Mục lục + lộ trình 1-3 tháng |
| 01 C++ fundamentals | ✅ Xong | memory-model, oop, templates |
| 02 Modern C++ | ✅ Xong | raii-smart-pointers, move-semantics, lambdas-functional, concurrency |
| 03 Operating System | ✅ Xong | process-thread, scheduling, memory-management, sync-primitives, ipc |
| 04 Linux system programming | ✅ Xong | file-io, processes-signals, io-multiplexing, ipc-linux |
| 05 Drivers & Device Tree | ✅ Xong | driver-basics, kernel-userspace, device-tree |
| 06 Build systems | ✅ Xong | makefile, cmake, cross-compilation |
| 07 Shared libraries | ✅ Xong | static-vs-shared, linking-loading, abi-versioning, api-design |
| 08 Embedded systems | ✅ Xong | architecture, boot-process, rtos-vs-linux, constraints |
| 09 Debugging | ✅ Xong | mindset, gdb, tools, memory-bugs, kernel-debugging |
| 10 Thinking | ✅ Xong | problem-solving, system-design |
| 11 Interview questions | ✅ Xong | cpp, operating-system, linux, drivers, debugging, design-patterns, dsa, networking, system-design |
| 12 Design patterns | ✅ Xong | solid-principles, creational, structural, behavioral (bổ sung) |
| 13 DSA | ✅ Xong | complexity-and-structures, algorithm-patterns (bổ sung) |
| 14 Networking | ✅ Xong | tcp-ip, sockets-and-protocols (bổ sung) |
| 00 Glossary | ✅ Xong | tra cứu nhanh thuật ngữ (bổ sung) |
| 15 Prep | 🟡 Đang dùng | **study-plans/** (chiến lược ôn theo vị trí BSP & C++ + gap-register); technical_round (01 lý thuyết, 02 question bank, 03 behavioral, 04 bsp question bank) |
| 16 Book summaries | 🟡 Đang viết | Quy ước xong; tiến độ chi tiết từng cuốn xem `16-book-summaries/README.md` |

Ký hiệu: ✅ Xong · 🟡 Đang viết · ⬜ Chưa bắt đầu

---

## 8. Vai trò của trợ lý (Claude) trong dự án này

**Giai đoạn xây tài liệu:**
- Viết các tài liệu theo đúng quy ước Mục 4, cấu trúc Mục 5.
- Sau khi viết xong một mục, **cập nhật bảng tiến độ Mục 7**.
- Khi cấu trúc thay đổi, cập nhật lại Mục 5 trong file này.

**Sau khi tài liệu hoàn tất:**
- Là người **giải đáp thắc mắc** về nội dung tài liệu.
- Là người **review câu trả lời** của người học: người học **tự trả lời trước**, sau đó Claude nhận xét, chỉ ra chỗ thiếu/sai, bổ sung, *rồi mới* đối chiếu đáp án mẫu. Không đưa đáp án ngay khi chưa được yêu cầu.
- Giữ vai trò "mentor": ưu tiên giúp người học hiểu **bản chất** và **tư duy**, không học vẹt.
