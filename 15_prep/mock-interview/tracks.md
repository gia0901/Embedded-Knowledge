# 🧭 Tracks — Chọn mảng phỏng vấn

> Track = tập **domain** trong [bank/](bank/) mà interviewer rút câu hỏi cho phiên. Chọn theo 3 cách: **công việc**, **phần kiến thức**, hoặc **cuốn sách**. Domain ID: xem [bank/README.md](bank/README.md).
> Cú pháp gọi: `mock <type> track <track-id>` — vd `mock comprehensive track bsp`.

---

## A. Theo công việc (job)

| Track | Domain rút câu | Ghi chú |
|---|---|---|
| **`bsp`** ⭐ | `BSP`, `DRV`, `BUS`, `OS`(mem/sched/sync), `LNX`(ipc/signal/io), `DBG`, `SD`(embedded) | Embedded Linux / BSP Engineer — **ưu tiên 1**. Nghiêng kernel/phần cứng/bring-up. |
| **`cpp-system`** | `CPP`, `OS`(concurrency/sync), `DP`, `DBG`, `SD`(abi/lib), `DSA`, `LNX`(basics) | System Software / C++ Engineer. Nghiêng ngôn ngữ/ABI/concurrency. |

> Hai track dùng chung nhiều nền (OS, debug, DSA). Ứng tuyển cả hai → chạy track sắp phỏng vấn trước; phần chung tự khớp.

## B. Theo phần kiến thức (part) — hẹp, luyện sâu 1 mảng

| Track | Domain | Topic gốc |
|---|---|---|
| `modern-cpp` | `CPP` | [01](../../01-cpp-fundamentals/), [02](../../02-modern-cpp/) |
| `os` | `OS` | [03](../../03-operating-system/) |
| `linux-sysprog` | `LNX` | [04](../../04-linux-system-programming/) |
| `drivers-dt` | `DRV`, `BSP`(B), `BUS` | [05](../../05-drivers-device-tree/), [08](../../08-embedded-systems/) |
| `debugging` | `DBG` | [09](../../09-debugging/) |
| `dsa` | `DSA`, `COD` | [13](../../13-dsa/) |
| `design-patterns` | `DP` | [12](../../12-design-patterns/) |
| `networking` | `NET` | [14](../../14-networking/) |
| `system-design` | `SD` | [10](../../10-thinking/), [07](../../07-shared-libraries/) |
| `behavioral` | `BEH` | HR/behavioral — mọi vị trí |

## C. Theo sách đã summary (book)

Track sách rút câu **2 nguồn**: (1) câu trong bank có **link nguồn tới cuốn đó**; (2) câu hỏi ẩn (`<details>Đáp án`) **nhúng sẵn trong bản summary** — interviewer có thể mở summary rút câu live; câu hay chưa có trong bank thì **thêm vào bank** sau phiên (đồng bộ theo [config.md §3](config.md)).

| Track | Domain trong bank | Summary nguồn |
|---|---|---|
| `emc` | `CPP` (tag emc) | [effective-modern-cpp.md](../../16-book-summaries/effective-modern-cpp.md) |
| `cpp-concurrency` | `CPP`(concurrency), `OS`(sync) | [cpp-concurrency/](../../16-book-summaries/cpp-concurrency/) |
| `melp` | `BSP`, `DRV`, `BUS` | [melp/](../../16-book-summaries/melp/) |
| `ostep` | `OS`, `LNX` | [ostep/](../../16-book-summaries/ostep/) |
| `lkd` | `DRV`, `BSP` | [lkd/](../../16-book-summaries/lkd/) |
| `cpp-mindset` | `DSA`, `CPP`, `SD` | [cpp-mindset/](../../16-book-summaries/cpp-mindset/) |

---

## Mix / all

- `track all` — rút câu toàn bank (cho `comprehensive` giả lập vòng thật đa dạng, hoặc `daily`).
- `track bsp+cpp-system` — trộn hai job track (khi JD lai).
- Không nêu track → mặc định `bsp` (theo [config.md](config.md)).
