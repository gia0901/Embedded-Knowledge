# Embedded Interview Course

**Bộ tài liệu đầy đủ - toàn diện - cô đọng kiến thức nền tảng và tư duy cần thiết để up level bản thân từ Middle → Senior Software Engineer (Embedded Linux, C++).**

> File này là "bộ não" của dự án. Khi một conversation kết thúc và bắt đầu conversation mới, đọc file này là đủ để hiểu mục tiêu, cấu trúc tài liệu, quy ước viết, tiến độ và vai trò của trợ lý (Claude).

---

## 1. Trạng thái dự án

- **Giai đoạn hiện tại:** ✅ Đã viết xong toàn bộ **13 topic kiến thức (01–13)** + `14-prep` (điều phối ôn) + `15-book-summaries`. Sang giai đoạn mentor: giải đáp & review câu trả lời, tinh chỉnh nội dung khi cần.
- File này đã được hoàn thiện (không còn là bản nháp mơ hồ). Vẫn có thể tinh chỉnh khi phát sinh nhu cầu mới.
- Xem **Mục 7 — Tiến độ** để biết tài liệu nào đã/đang/chưa viết.

> 🎯 **ĐANG THỰC THI PLAN PHỎNG VẤN (Datalogic).** Conversation mới muốn tiếp tục ôn: mở **[`14-prep/study-plans/datalogic-plan.md`](14-prep/study-plans/datalogic-plan.md) → §📍 Tiến độ hiện tại** (ngay đầu file) để biết đang ở buổi nào + lệnh làm tiếp. Đó là **nguồn tracking DUY NHẤT** cho tiến độ ôn — không suy đoán từ nơi khác.

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
6. Mỗi tài liệu con **PHẢI** có: **TL;DR** đầu file + mục **"Câu hỏi phỏng vấn liên quan"** cuối file.
7. ⚠️ **MỘT ĐÁP ÁN, MỘT CHỖ — đáp án chỉ sống trong [`bank/`](14-prep/mock-interview/bank/), KHÔNG chép vào tài liệu topic.**

   Mục cuối file chỉ được là **danh sách trỏ sang bank**, không có `<details>` đáp án:
   ```markdown
   ## Câu hỏi phỏng vấn liên quan

   | ID | Câu hỏi |
   |----|---------|
   | [LNX-005](../14-prep/mock-interview/bank/linux-sysprog.md) | short read — vì sao lab đúng mà hiện trường sai |
   ```
   Đáp án vẫn **ẩn trong `<details>`** — nhưng ẩn ở **bank**, nơi nó được định nghĩa một lần.

   > **Vì sao (sự cố thật 2026-08-13):** rà soát đếm được **262 câu hỏi có đáp án chép thẳng vào tài liệu topic 01–13**, song song với 303 câu trong bank — tức 262 bản sao **không có cơ chế đồng bộ**. Cùng ngày, bank `LNX-005` được nâng cấp từ 277 → 2.736 ký tự (thêm cơ chế TCP luồng byte), nhưng bản sao trong `04-.../file-io.md` **vẫn y nguyên bản cũ**. Người học mở tài liệu ra ôn thì đọc đúng phiên bản đã bị kết luận là không đủ. Mâu thuẫn gốc: §4.6 (cũ) nói *"link tới bank"* còn thực tế các file chép nguyên đáp án vào — hai thế hệ quy ước cùng tồn tại.
   >
   > **Luật thi hành:** thấy `<details>` chứa đáp án trong tài liệu topic ⇒ đó là **nợ kỹ thuật**, phải chuyển về bank. Câu nào **chưa có** trong bank thì **thêm vào bank trước**, rồi mới thay bằng link — **không bao giờ xoá thẳng**.

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
│   ├── bus-protocols.md          # UART/I2C/SPI — so sánh, clock stretching, CPOL/CPHA
│   └── pci-usb-drivers.md        # PCI (BAR/MSI-X) + USB (descriptor/URB/gadget)
│
├── 06-build-systems/             # Build system
│   ├── makefile.md
│   ├── cmake.md
│   ├── cross-compilation.md      # toolchain, cross-compile cho embedded
│   └── yocto.md                  # BitBake, layer/recipe/bbappend, sstate, SDK
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
│   ├── rtos-vs-linux.md · rtos-programming.md
│   ├── bare-metal-c.md · memory-and-startup.md · interrupts-bare-metal.md
│   ├── hardware-debug.md         # JTAG/SWD, GPIO+scope, crash log qua reset
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
├── 11-design-patterns/           # Mẫu thiết kế (bổ sung)
│   ├── solid-principles.md       # SOLID — nền tảng
│   ├── creational.md             # Singleton, Factory, Builder
│   ├── structural.md             # Adapter, Bridge/Pimpl, Facade, Proxy
│   └── behavioral.md             # Strategy, Observer, State, Command
│
├── 12-dsa/                       # Data Structures & Algorithms (bổ sung)
│   ├── complexity-and-structures.md  # Big-O, container, trade-off
│   ├── algorithm-patterns.md     # two-pointer, sliding window, BFS/DFS, DP
│   └── ring-buffer.md            # 🎯 5 tầng: 1 luồng → lock-free SPSC; kfifo/pipe/ALSA/io_uring
│
├── 13-networking/                # Mạng (bổ sung)
│   ├── tcp-ip.md                 # mô hình, TCP vs UDP, handshake
│   └── sockets-and-protocols.md  # socket API, HTTP/TLS/MQTT
│
├── 14-prep/                      # Chuẩn bị phỏng vấn thực tế
│   ├── study-plans/              # ⭐ Plan bám JD đang nhắm (datalogic-plan, có §📍 tracking) + gap-register
│   └── mock-interview/           # ⭐ Phỏng vấn thử tương tác (/mock) + NGÂN HÀNG CÂU HỎI DUY NHẤT (bank/) + sessions/weak-register/coding-arena(+reviewed/)
│
├── 15-book-summaries/            # Tóm tắt sách chuyên ngành theo cụm chủ đề
│   └── README.md                 # ⚠️ QUY ƯỚC RIÊNG của topic này — đọc trước khi viết/sửa file trong đó
│
├── 00-glossary.md                # Tra cứu nhanh thuật ngữ EN ↔ giải thích
└── OVERVIEW.md                   # Bản đồ tư duy: các topic kiến thức (01–13) liên kết thế nào
```

> **Hai tầng "bức tranh tổng thể":** `OVERVIEW.md` nối *toàn bộ* các topic kiến thức (01–13); mỗi README topic có mục "🗺️ Bức tranh tổng thể" nối *các file con* trong topic đó. Đây là phần thể hiện "hiểu kiến trúc cốt lõi" thay vì kiến thức rời rạc.

> Danh sách file con ở trên là **khung gợi ý**, sẽ điều chỉnh/thêm bớt khi viết. Topic mới có thể bổ sung nếu cần.
> Topic 11–13 + glossary là **bổ sung sau** so với kế hoạch gốc (Design Patterns, DSA, Networking) theo background người học.

---

## 6. Câu hỏi phỏng vấn & Mock interview — quy ước riêng

- **Ngân hàng câu hỏi DUY NHẤT:** mọi câu hỏi sống ở `14-prep/mock-interview/bank/` (chia file theo domain, **ID ổn định** `CPP-001`…, `BSP-002`…). Các bộ câu hỏi cũ (`11-interview-questions/`, `14-prep/technical_round/`) đã gộp về bank và **xoá hẳn** (2026-08-09) — **không tạo bank thứ hai**, câu mới luôn thêm vào bank.
- Mỗi câu: dòng metadata (`#### <ID> · level · type [· ⭐] [· 🏗️]`) → đề bài in đậm → **đáp án ẩn trong `<details>`**; phân độ khó 🟢🟡🟠🔴; link ngược tài liệu nền tảng.
- 🧪 **Câu `lab` — bài NGỒI MÁY LÀM** (thêm 2026-08-17): type thứ tư bên cạnh `concept`/`coding`/`design`. Code có bug thật + nhiệm vụ đánh số + `<details>` chứa lệnh cần gõ và **OUTPUT THẬT đã chạy, dán nguyên văn** (cấm viết tay/phỏng đoán — người học đối chiếu với màn hình của họ). Không hỏi ở phiên mock miệng; đó là bài tự luyện, phiên mock hỏi câu `concept` tương ứng. Lý do ra đời: đo được **T1 3.67 / T2 2.1** qua hai phiên liên tiếp ⇒ *biết* công cụ nhưng *chưa dùng* công cụ, câu `concept` không vá được khoảng cách đó. Quy ước đầy đủ: [bank/README.md](14-prep/mock-interview/bank/README.md).
- **Mock interview (`/mock`):** Claude đóng vai **interviewer** — đọc `mock-interview/config.md`, hỏi từng câu theo track+type, **không chấm giữa chừng** (được đào sâu/follow-up), review ở cuối, rồi cập nhật `sessions/` + `weak-register.md` + thêm câu mới vào `bank/`. Câu yếu được hỏi lại (không có luật "đúng rồi thôi"). Chi tiết: [14-prep/mock-interview/README.md](14-prep/mock-interview/README.md).
- 📐 **Luật vận hành KHÔNG chép vào file này.** Chúng sống ở [`mock-interview/config.md`](14-prep/mock-interview/config.md) và phải đọc trước mọi phiên: **§⚖️** thứ tự ưu tiên khi hai file mâu thuẫn · **§6** hợp đồng độ sâu (T1/T2/T3, trần mặc định **T2**, chống phiên nông *và* phiên lệch tầng) · **§7** đo độ phủ bank mỗi 5 phiên · **§1 Bước 4** cập nhật bắt buộc sau phiên.
  > *Trước 2026-08-18, bốn luật này được chép nguyên văn vào đây, vào `mock-interview/README.md` và vào `interview-types.md` — cùng một luật ở 3–5 chỗ, đúng cái bệnh "một sự thật, hai chỗ" mà repo đã ghi nhận 4 lần. Nay chỉ còn con trỏ.*
- **Vai trò khi user tự ôn:** nếu user nhờ **review câu trả lời** (ngoài phiên `/mock`), đối chiếu bank, chỉ ra thiếu/sai/lệch, chấm 0–4, link ôn lại — không chỉ đọc đáp án mẫu.

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
| 05 Drivers & Device Tree | ✅ Xong | driver-basics, kernel-userspace, device-tree, **bus-protocols**, pci-usb-drivers |
| 06 Build systems | ✅ Xong | makefile, cmake, cross-compilation, yocto |
| 07 Shared libraries | ✅ Xong | static-vs-shared, linking-loading, abi-versioning, api-design |
| 08 Embedded systems | ✅ Xong | architecture, boot-process, rtos-vs-linux, rtos-programming, bare-metal-c, memory-and-startup, interrupts-bare-metal, hardware-debug, constraints |
| 09 Debugging | ✅ Xong | mindset, gdb, tools, memory-bugs, kernel-debugging |
| 10 Thinking | ✅ Xong | problem-solving, system-design |
| 11 Design patterns | ✅ Xong | solid-principles, creational, structural, behavioral (bổ sung) |
| 12 DSA | ✅ Xong | complexity-and-structures, algorithm-patterns, **ring-buffer** (bổ sung) |
| 13 Networking | ✅ Xong | tcp-ip, sockets-and-protocols (bổ sung) |
| 00 Glossary | ✅ Xong | tra cứu nhanh thuật ngữ (bổ sung) |
| 14 Prep | 🟡 Đang dùng | **study-plans/** (`datalogic-plan` — plan JD đang chạy, §📍 là nguồn tracking duy nhất — + gap-register); **mock-interview/** (phỏng vấn thử `/mock` + **ngân hàng câu hỏi duy nhất** `bank/` + sessions/weak-register/coding-arena + coding-arena/reviewed git-track) |
| 15 Book summaries | 🟡 Đang viết | **7 cuốn đã xong**, tất cả đối chiếu PDF (Effective Modern C++, OSTEP, LKD, MELP, cpp-concurrency, cpp-mindset — đều "đọc trực tiếp PDF", neo trang/§, tự chứa); TLPI ✅ 10/10 cụm (đủ 64 chương); còn LDD3/Bootlin ⬜. Tiến độ chi tiết từng cuốn xem `15-book-summaries/README.md` |

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
