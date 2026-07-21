# Embedded Interview Course — Mục lục & Lộ trình

Bộ tài liệu ôn luyện phỏng vấn **Embedded Linux / C++**, định hướng nâng trình từ **Middle → Senior**.

> 📌 Định hướng, quy ước viết và vai trò của trợ lý được mô tả trong [CLAUDE.md](CLAUDE.md). Đọc file đó trước nếu bạn là người mới tham gia (hoặc là AI bắt đầu một phiên làm việc mới).
>
> 🗺️ **Muốn thấy bức tranh lớn — các topic kiến thức (01–14) liên kết với nhau thế nào?** Đọc [OVERVIEW.md](OVERVIEW.md) (bản đồ tư duy toàn bộ). Mỗi README topic cũng có mục "Bức tranh tổng thể" riêng.

---

## 1. Cách sử dụng bộ tài liệu

- Mỗi **topic** là một thư mục, chia thành các tài liệu con `.md`.
- Mỗi tài liệu con có **TL;DR** ở đầu và danh sách **câu hỏi phỏng vấn liên quan** ở cuối.
- Mọi câu hỏi phỏng vấn đều có **đáp án ẩn** (`<details>`) — hãy **tự trả lời trước**, sau đó mới mở đáp án để đối chiếu.
- Phương châm: hiểu **bản chất** (tại sao tồn tại, khi nào dùng/không, so sánh & lựa chọn) thay vì học thuộc.

**Quy ước trạng thái** dùng trong tài liệu: ✅ Xong · 🟡 Đang viết · ⬜ Chưa bắt đầu

---

## 2. Mục lục các topic

| # | Topic | Nội dung chính | Trạng thái |
|---|-------|----------------|-----------|
| 01 | [C/C++ Fundamentals](01-cpp-fundamentals/) | memory model, OOP, vtable, templates | ✅ |
| 02 | [Modern C++](02-modern-cpp/) | RAII, smart pointer, move semantics, concurrency | ✅ |
| 03 | [Operating System](03-operating-system/) | process/thread, scheduling, virtual memory, IPC, sync | ✅ |
| 04 | [Linux System Programming](04-linux-system-programming/) | file I/O, signal, fork/exec, epoll | ✅ |
| 05 | [Drivers & Device Tree](05-drivers-device-tree/) | char/block driver, kernel↔user, device tree | ✅ |
| 06 | [Build Systems](06-build-systems/) | Makefile, CMake, cross-compilation | ✅ |
| 07 | [Shared Libraries](07-shared-libraries/) | static vs shared, linking/loading, ABI, API design | ✅ |
| 08 | [Embedded Systems](08-embedded-systems/) | SoC/bus, boot process, RTOS vs Linux, constraints | ✅ |
| 09 | [Debugging](09-debugging/) | mindset, gdb, valgrind/strace/perf, sanitizers | ✅ |
| 10 | [Thinking](10-thinking/) | problem solving, system design (embedded) | ✅ |
| 11 | [Interview Questions](11-interview-questions/) | câu hỏi theo độ khó, đáp án ẩn | ✅ |
| 12 | [Design Patterns](12-design-patterns/) | SOLID, creational/structural/behavioral | ✅ |
| 13 | [DSA](13-dsa/) | Big-O, cấu trúc dữ liệu, pattern giải thuật | ✅ |
| 14 | [Networking](14-networking/) | TCP/IP, socket, HTTP/TLS/MQTT | ✅ |
| 15 | [Prep](15_prep/) | chiến lược ôn theo vị trí (study-plans) + technical round (lý thuyết, question bank, behavioral) | 🟡 |
| 16 | [Book Summaries](16-book-summaries/) | tóm tắt sách chuyên ngành (quy ước riêng trong README topic) | 🟡 |
| 00 | [Glossary](00-glossary.md) | tra cứu nhanh thuật ngữ EN | ✅ |

---

## 3. Chiến lược ôn tập theo vị trí

Kế hoạch ôn được tổ chức theo **2 hướng nghề nghiệp đã chọn**, đặt tại **[15_prep/study-plans/](15_prep/study-plans/)** — đây là **nguồn chiến lược duy nhất** (thay cho lộ trình generic trước đây):

- **[Embedded SW Engineer (BSP)](15_prep/study-plans/bsp-plan.md)** — boot/U-Boot, device tree/probe, kernel internals, interrupt/DMA, storage/OTA, real-time.
- **[System Software / C++ Engineer](15_prep/study-plans/cpp-systemsw-plan.md)** — modern C++, ABI/shared library, concurrency, design pattern, OS fundamentals.

Mỗi kế hoạch có **hai tầng**: 🏃 *sprint* (sát phỏng vấn, chỉ mục 🎯) + 📚 *nền tảng* (dài hơi, phủ toàn diện) — ánh xạ thẳng sang topic + book summary + question bank. Backlog "thiếu gì" nằm ở **[gap-register.md](15_prep/study-plans/gap-register.md)**.

> **Tài liệu ôn & tự kiểm tra dùng chung** cho cả hai hướng: [11-interview-questions/](11-interview-questions/) (đáp án ẩn, phân độ khó) và [15_prep/technical_round/](15_prep/) (lý thuyết cốt lõi, question bank C++/BSP, behavioral STAR). Luyện **DSA** ([13-dsa/](13-dsa/)) song song xuyên suốt.

---

## 4. Đóng góp & cập nhật

- Khi hoàn thành một mục, cập nhật cột **Trạng thái** ở Mục 2 (file này) **và** bảng tiến độ trong [CLAUDE.md](CLAUDE.md#7-tiến-độ-cập-nhật-mỗi-khi-hoàn-thành-một-mục).
- Khi thêm/bớt topic, cập nhật cả Mục 5 trong [CLAUDE.md](CLAUDE.md) lẫn mục lục ở đây.
