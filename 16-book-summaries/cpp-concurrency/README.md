# C++ Concurrency in Action — Anthony Williams (**2nd edition, 2019**, Manning)

> ✅ **HOÀN THÀNH toàn bộ 11 chương** (đọc sâu toàn văn từ PDF). Ưu tiên đầu tư nặng nhất: **ch. 5 (memory model)** + **ch. 7 (lock-free)** — hai chương phân loại senior mà repo chưa có nguồn nào khác phủ.
> **Nguồn summary:** đọc trực tiếp PDF `C++ Concurrency in Action by Anthony Williams (z-lib.org).pdf` (ISBN 9781617294693). **Đã đối chiếu mục lục + xác minh offset: trang sách = trang PDF − 23.** Mọi số trang `(tr. X)` trong các file là **trang in của sách**.
> **Phiên bản:** 2nd ed phủ **C++11/14/17** (thêm `std::scoped_lock`, `std::shared_mutex`, parallel algorithms/execution policies so với 1st ed). Tác giả là **primary developer cũ của Boost.Thread** và thành viên C++ Standards panel — đây là sách "từ miệng người viết ra chuẩn".
> **Quy ước:** không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.

---

## Vì sao đọc cuốn này — và nó **bổ sung** gì cho repo

Repo đã có concurrency ở **ba tầng khác nhau**, nhưng đều dừng ở mức nền:

| Tài liệu sẵn có | Tầng | Dừng ở đâu |
|---|---|---|
| [ostep/concurrency.md](../ostep/concurrency.md) | **OS** — lock/CV/semaphore xây từ phần cứng | Không có góc C++ |
| [02-modern-cpp/concurrency.md](../../02-modern-cpp/concurrency.md), [03/sync-primitives](../../03-operating-system/sync-primitives.md) | Cô đọng | Mức API cơ bản |
| [EMC++ cụm 7](../effective-modern-cpp.md) (Items 35–40) | C++ | 6 lời khuyên, **không** dạy memory model |

→ **Giá trị riêng của cuốn này nằm ở hai chỗ mà cả repo chưa có tài liệu nào phủ sâu:**

1. **🎯🎯 Chương 5 — C++ memory model & atomic ordering** (tr. 124–172): happens-before, synchronizes-with, `memory_order_*` (seq_cst / acquire-release / relaxed / consume), fences, release sequences. **Đây là chủ đề phân loại ứng viên senior C++** và là thứ khó tự học nhất — không tài liệu nào khác trong repo chạm tới.
2. **🎯 Chương 7 — Lock-free data structures** (tr. 205–250): hazard pointers, reference counting, **ABA problem**, memory reclamation. Cũng chưa có ở đâu trong repo.

Các chương còn lại **chồng lấn có chủ đích** với tài liệu sẵn có — nhưng theo [quy ước Mục 1](../README.md), summary vẫn **viết đầy đủ tại chỗ, chấp nhận lặp**, để đọc liền mạch không phải nhảy tài liệu; link chéo chỉ đặt cuối file như "đọc thêm (tùy chọn)".

**Liên hệ nghề:** trúng thẳng hướng **System Software / C++** ([cpp-systemsw-plan](../../15_prep/study-plans/cpp-systemsw-plan.md) — Tuần 2 là toàn bộ concurrency). Với **BSP**: chương 5 (memory ordering ↔ barrier trong kernel — [lkd/sync-timers](../lkd/03-sync-timers.md)) và chương 8 (false sharing, cache ping-pong ↔ [cpp-mindset ch.4](../cpp-mindset/understanding-the-machine.md)) là phần giao nhau đáng đọc.

---

## TL;DR — cuốn sách này dạy gì

Viết code đa luồng C++ **đúng trước, nhanh sau**, đi từ API tới bản chất: quản lý vòng đời thread (`std::thread`, join/detach, chuyển ownership) → bảo vệ dữ liệu chung (mutex, deadlock, granularity) → đồng bộ *sự kiện* (condition variable, future/promise/`async`) → **memory model** (thứ giải thích *vì sao* atomic hoạt động và khi nào được nới lỏng ordering) → thiết kế **cấu trúc dữ liệu concurrent** (lock-based rồi lock-free) → **thiết kế code concurrent** (chia việc, false sharing, Amdahl) → thread pool, parallel algorithms (C++17), và cuối cùng là **test/debug code đa luồng**.

Mạch xuyên suốt: mỗi cơ chế được giới thiệu qua **một bug cụ thể mà nó sửa** — race condition trong interface, deadlock do lock ordering, lost wakeup, ABA — nên đọc xong là có sẵn kho ví dụ để kể trong phỏng vấn.

## 🕐 Đọc gì nếu chỉ có N giờ

- **~3 giờ:** [03-sharing-data.md](03-sharing-data.md) (mutex, deadlock, race trong interface) + [04-synchronization.md](04-synchronization.md) (condvar, future). Đây là phần dùng hằng ngày.
- **~6 giờ:** thêm [05-memory-model.md](05-memory-model.md) — khó nhất, giá trị interview cao nhất.
- **~9 giờ:** thêm [08-designing-concurrent-code.md](08-designing-concurrent-code.md) (false sharing, Amdahl) + [02-managing-threads.md](02-managing-threads.md).

## 🎯 Lộ trình ôn nhanh trước interview (hướng C++/System SW)

1. **Sharing data** (ch. 3) — mutex, **race condition vốn có trong interface** (ví dụ `stack::top()`+`pop()`), deadlock + lock ordering, `std::lock`/`scoped_lock`, `unique_lock`, granularity.
2. **Memory model** (ch. 5) 🎯🎯 — atomic, happens-before/synchronizes-with, 6 `memory_order`, khi nào relaxed là đủ, fences.
3. **Synchronization** (ch. 4) — condition variable (**while, không if**), `future`/`promise`/`packaged_task`/`async`, timeout.
4. **Managing threads** (ch. 2) — join/detach, RAII thread guard, chuyển ownership, `hardware_concurrency`.
5. **Designing concurrent code** (ch. 8) — false sharing, cache ping-pong, oversubscription, **Amdahl's law**, exception safety song song.
6. **Lock-free** (ch. 7) — câu hỏi nâng cao: ABA, hazard pointer, vì sao lock-free ≠ nhanh hơn.
7. Còn lại: lock-based structures (ch. 6), thread pool (ch. 9), parallel algorithms (ch. 10), testing (ch. 11).

## 🗺️ Bản đồ: chương ↔ file ↔ trạng thái

> **Quy ước tên file: số file = số chương** (1:1) — nhìn tên là biết đang ở phần nào của sách. Mỗi file có link **⏮ chương trước / chương sau ⏭** ở đầu và cuối để đọc liền mạch.

| File | Chương (trang sách) | Nội dung | Ưu tiên | Trạng thái |
|------|--------------------|----------|---------|-----------|
| [01-hello-concurrency.md](01-hello-concurrency.md) | 1 (tr. 1–15) | Task switching vs hardware concurrency; process vs thread; **concurrency vs parallelism**; khi nào KHÔNG dùng concurrency; lịch sử C++98→17; Hello Concurrent World | 🟠 | ✅ **Xong** |
| [02-managing-threads.md](02-managing-threads.md) | 2 (tr. 16–35) | Khởi chạy thread (most vexing parse), **join/detach + `std::terminate`**, RAII `thread_guard`/`scoped_thread` (⚠️ nay là `std::jthread` C++20), truyền tham số (`std::ref`/`std::move`, bẫy convert muộn), **chuyển ownership**, `hardware_concurrency` + `parallel_accumulate`, `thread::id` | 🎯 | ✅ **Xong** |
| [03-sharing-data.md](03-sharing-data.md) | 3 (tr. 36–71) | Race condition vs **data race**; mutex & `lock_guard` (bẫy rò rỉ reference); **race vốn có trong interface** (`stack` + nghịch lý Cargill + `threadsafe_stack`); **deadlock + `std::lock`/`scoped_lock`**, guideline + hierarchical mutex; `unique_lock` (defer/transfer/unlock sớm); granularity; **`call_once`/static-local**; **`shared_mutex`**; `recursive_mutex` | 🎯 | ✅ **Xong** |
| [04-synchronization.md](04-synchronization.md) | 4 (tr. 72–123) | **Condition variable** (while-không-if, spurious wakeup) & thread-safe queue; **future/promise/packaged_task/async** (+ exception, launch policy); `shared_future`; timeout & `steady_clock`; FP-style parallel quicksort; message passing (Actor/CSP); continuations `.then` (TS); **latch/barrier** (⚠️ nay `std::latch`/`std::barrier` C++20) | 🎯 | ✅ **Xong** |
| [05-memory-model.md](05-memory-model.md) | 5 (tr. 124–172) | **Memory location & data race & modification order; atomic types (`atomic_flag` spinlock, CAS weak/strong, `atomic<T*>`, UDT); 🎯🎯 synchronizes-with / happens-before; 6 memory_order (seq_cst/acquire-release/relaxed/consume); release sequence; fences; ordering non-atomic** | 🎯🎯 | ✅ **Xong** |
| [06-lock-based-structures.md](06-lock-based-structures.md) | 6 (tr. 173–204) | Thiết kế cho concurrency (4 tiêu chí + serialization); thread-safe **stack/queue coarse→fine-grained** (dummy node, head/tail mutex, bẫy `get_tail`); **lookup table** (hash + `shared_mutex`/bucket); **list** (hand-over-hand locking) | 🟠 | ✅ **Xong** |
| [07-lock-free-structures.md](07-lock-free-structures.md) | 7 (tr. 205–250) | **Nonblocking vs lock-free vs wait-free**; lock-free stack/queue (CAS-loop); **3 kỹ thuật memory reclamation** (đếm thread / **hazard pointer** / **split reference counting**); nới lỏng memory order; **ABA problem**; helping | 🎯 | ✅ **Xong** |
| [08-designing-concurrent-code.md](08-designing-concurrent-code.md) | 8 (tr. 251–299) | Chia việc (dữ liệu/đệ quy/task + pipeline); **cache ping-pong, false sharing, data proximity, oversubscription**; **exception safety** (`async`/`packaged_task`); **Amdahl's law**; parallel `for_each`/`find`/`partial_sum` | 🎯 | ✅ **Xong** |
| [09-advanced-thread-management.md](09-advanced-thread-management.md) | 9 (tr. 300–326) | **Thread pool** tiến hóa 5 bước (waitable→run_pending_task→local queue→**work stealing**); **interrupting threads** (⚠️ nay `std::jthread`+`std::stop_token` C++20) | 🟠 | ✅ **Xong** |
| [10-parallel-algorithms.md](10-parallel-algorithms.md) | 10 (tr. 327–338) | **Parallel algorithms C++17**: execution policies `seq`/`par`/`par_unseq` (ràng buộc code + bẫy exception→terminate); Forward Iterator; `transform_reduce` (map-reduce) | 🟠 | ✅ **Xong** |
| [11-testing-debugging.md](11-testing-debugging.md) | 11 (tr. 339–353) | Loại bug (blocking vs race); **định vị**: 8 câu hỏi review, phân loại bug, brute-force/combination/**TSan**; cấu trúc test (promise "go"); scalability | 🟠 | ✅ **Xong** |

> Phụ lục: A (C++11 features, tr. 354) · B (so sánh thư viện concurrency, tr. 382) · C (message-passing + ATM example, tr. 384) · D (**Thread Library reference**, tr. 401–550 — tra cứu API, không summary).

---

## Ghi chú khi viết summary (cho phiên sau)

- **✅ TỰ CHỨA — CHO PHÉP LẶP** (theo [quy ước chung](../README.md) Mục 1): viết **đầy đủ, mạch lạc tại chỗ**, kể cả khi trùng với [ostep/concurrency](../ostep/concurrency.md) (tầng OS) hay [EMC++ cụm 7](../effective-modern-cpp.md). Người đọc **không phải nhảy sang tài liệu khác giữa chừng**; link chéo chỉ đặt như **"đọc thêm (tùy chọn)"** để tự tra khi muốn đào sâu.
- **Đọc tuần tự từ chương 1**, mỗi file có điều hướng ⏮/⏭ ở đầu và cuối.
- **Ưu tiên tuyệt đối cho ch. 5** — đây là lý do chính đọc cuốn này; đầu tư nhiều code + sơ đồ (happens-before graph) hơn các chương khác.
- Bám [quy ước chung](../README.md) Mục 4: code thật (rút gọn, comment tiếng Việt), insight **neo vào trích dẫn + số trang**, câu hỏi interview có đáp án đầy đủ ẩn trong `<details>`.
