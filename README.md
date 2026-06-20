# Embedded Interview Course — Mục lục & Lộ trình

Bộ tài liệu ôn luyện phỏng vấn **Embedded Linux / C++**, định hướng nâng trình từ **Middle → Senior**.

> 📌 Định hướng, quy ước viết và vai trò của trợ lý được mô tả trong [CLAUDE.md](CLAUDE.md). Đọc file đó trước nếu bạn là người mới tham gia (hoặc là AI bắt đầu một phiên làm việc mới).
>
> 🗺️ **Muốn thấy bức tranh lớn — 15 topic liên kết với nhau thế nào?** Đọc [OVERVIEW.md](OVERVIEW.md) (bản đồ tư duy toàn bộ). Mỗi README topic cũng có mục "Bức tranh tổng thể" riêng.

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
| 00 | [Glossary](00-glossary.md) | tra cứu nhanh thuật ngữ EN | ✅ |

---

## 3. Lộ trình học gợi ý (1–3 tháng)

Lộ trình cân bằng giữa **xây nền tảng** và **luyện phỏng vấn**. Mỗi giai đoạn nên dành thời gian cuối tuần làm **mock interview** với topic 11.

### Giai đoạn 1 — Nền tảng ngôn ngữ (Tuần 1–3)
Trọng tâm phỏng vấn thường rơi nhiều nhất vào C++.
- **Tuần 1:** `01 C/C++ Fundamentals` — memory model, con trỏ/tham chiếu, OOP, vtable.
- **Tuần 2:** `02 Modern C++` — RAII & smart pointer, move semantics, perfect forwarding.
- **Tuần 3:** `02 Modern C++` (tiếp) — concurrency: thread, mutex, atomic, memory order.
- 🔁 Song song: làm câu hỏi `11/cpp.md`.

### Giai đoạn 2 — Hệ điều hành & Linux (Tuần 4–6)
Phần lõi để hiểu hệ thống — nền cho cả driver lẫn debugging.
- **Tuần 4:** `03 Operating System` — process/thread, context switch, scheduling.
- **Tuần 5:** `03 Operating System` — virtual memory/paging, IPC, sync primitives & deadlock.
- **Tuần 6:** `04 Linux System Programming` — file I/O, signal, fork/exec/wait, epoll.
- 🔁 Song song: câu hỏi `11/operating-system.md`, `11/linux.md`.

### Giai đoạn 3 — Chuyên sâu Embedded (Tuần 7–9)
Phần đặc thù vai trò System/Embedded Software.
- **Tuần 7:** `07 Shared Libraries` — linking/loading, ABI/soname, thiết kế C++ API (rất sát công việc hiện tại của bạn).
- **Tuần 8:** `05 Drivers & Device Tree` + `06 Build Systems` (cross-compilation).
- **Tuần 9:** `08 Embedded Systems` — SoC, boot process, RTOS vs Linux, constraints.
- 🔁 Song song: câu hỏi `11/drivers.md`.

### Giai đoạn 4 — Debug & Tư duy (Tuần 10–12)
Củng cố điểm yếu (debug) và tư duy Senior.
- **Tuần 10:** `09 Debugging` — mindset + gdb + sanitizers + memory bugs.
- **Tuần 11:** `09 Debugging` (tiếp) — strace/ltrace/perf, kernel debugging.
- **Tuần 12:** `10 Thinking` — problem solving & system design; tổng ôn `11/system-design.md`.
- 🔁 Mock interview tổng hợp toàn bộ topic.

### Topic bổ sung (đan xen theo nhu cầu)
Các topic 12–14 + glossary thêm sau, lồng vào lộ trình khi phù hợp:
- **`12 Design Patterns`** — đọc cùng giai đoạn C++ (Tuần 1–3); SOLID là nền tảng, đáng đọc sớm. (Điểm cần nâng cấp.)
- **`13 DSA`** — luyện song song **xuyên suốt** trên Leetcode (mỗi tuần vài bài theo pattern).
- **`14 Networking`** — đọc cùng/sau giai đoạn Linux (Tuần 6) nếu vị trí ứng tuyển có liên quan mạng.
- **`00 Glossary`** — tra cứu bất cứ lúc nào gặp thuật ngữ chưa rõ.

> ⏩ **Nếu gấp:** ưu tiên Tuần 1–2 (C++ core), Tuần 4–5 (OS core), topic 11 tương ứng, và `09 Debugging/mindset.md`. Các phần còn lại đọc lướt TL;DR.

---

## 4. Đóng góp & cập nhật

- Khi hoàn thành một mục, cập nhật cột **Trạng thái** ở Mục 2 (file này) **và** bảng tiến độ trong [CLAUDE.md](CLAUDE.md#7-tiến-độ-cập-nhật-mỗi-khi-hoàn-thành-một-mục).
- Khi thêm/bớt topic, cập nhật cả Mục 5 trong [CLAUDE.md](CLAUDE.md) lẫn mục lục ở đây.
