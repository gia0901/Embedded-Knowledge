# C++ High Performance — Björn Andrist & Viktor Sehr (2nd Edition, Packt 12/2020)

> **Nguồn summary:** đã đối chiếu PDF *C++ High Performance, 2nd Edition* (541 trang). **Offset xác minh: trang PDF = trang sách + 27** (kiểm 3 điểm: PDF 100 → tr. 73, PDF 120 → tr. 93, PDF 140 → tr. 113).
> **Vì sao đọc cuốn này:** repo đã dạy *"tránh cấp phát động trong đường nóng"* và *"vector nhanh hơn list vì cache"* — nhưng ở mức **một đoạn văn**. Cuốn này là nơi hai câu đó có **code chạy được** và **số đo thật**.

---

## ⚠️ Đọc mục này trước: bản summary này là CHỌN LỌC, không phải cả cuốn

Sách có **14 chương**. Bản summary này cố ý làm **3 chương**, và việc bỏ 11 chương còn lại là **quyết định có cân nhắc**, không phải dở dang.

| Chương | Vì sao BỎ |
|---|---|
| **1. A Brief Introduction to C++**<br>**2. Essential C++ Techniques** | Trùng nặng với [effective-modern-cpp.md](../effective-modern-cpp.md) (42 Item, đã đối chiếu PDF) và [01](../../01-cpp-fundamentals/)/[02](../../02-modern-cpp/). `auto`, move semantics, lambda, error handling — repo đã có sâu hơn |
| **11. Concurrency** | Trùng với [cpp-concurrency/](../cpp-concurrency/) — **11 chương của Anthony Williams**, đã đọc sâu toàn văn. Một chương của Andrist không thêm được gì |
| **5. Algorithms** (phần lớn)<br>**9. Essential Utilities**<br>**10. Proxy Objects & Lazy Evaluation** | Có giá trị nhưng **xác suất bị hỏi thấp** với JD embedded. Phần đáng nhất của ch. 5 (chọn container theo thao tác chủ đạo) đã gộp vào [data-structures-cache.md](data-structures-cache.md) |
| **6. Ranges and Views**<br>**8. Compile-Time Programming** (phần concepts)<br>**12. Coroutines and Lazy Generators**<br>**13. Asynchronous Programming with Coroutines**<br>**14. Parallel Algorithms** | 🔴 **Đều là C++20.** JD Datalogic ghi rõ **C++17**. Ch. 13 còn phụ thuộc **Boost.Asio**. Đây là ~30% cuốn sách **không dùng được** trên codebase hiện tại |

> **Nếu về sau chuyển sang C++20** thì ba chương đáng làm tiếp, theo thứ tự: **8** (concepts — thay SFINAE), **6** (ranges), **12** (coroutines). Ghi ở [gap-register](../../14-prep/study-plans/gap-register.md) dòng *"C++20/23 depth"*.

---

## TL;DR — cuốn sách này dạy gì

**Luận điểm trung tâm: hiệu năng C++ hiện đại không đến từ việc viết vòng lặp khéo hơn, mà đến từ việc BỘ NHỚ được bố trí thế nào.** Ba hệ quả xuyên suốt:

1. **Big-O bỏ qua hằng số, và hằng số ở đây là ~200×** (L1 0,5 ns vs RAM 100 ns). Hai vòng lặp cùng O(n²) chênh nhau **40 ms vs 800 ms** chỉ vì đổi `[i][j]` thành `[j][i]`.
2. **Kích thước object quyết định tốc độ duyệt.** Sách đo: cùng một phép cộng trên 1 triệu phần tử — object 8 byte mất **1 ms**, object 260 byte mất **10 ms**. Và `sizeof` đổi được **24 → 16 byte** chỉ bằng sắp lại thứ tự khai báo member.
3. **Cấp phát động là thứ đắt nhất bạn có thể làm trong đường nóng** — và C++17 đã chuẩn hoá cách tránh nó (`std::pmr`) mà rất ít người dùng.

---

## 🕐 Đọc gì nếu chỉ có N giờ

| Có | Đọc |
|---|---|
| **30 phút** | [memory-management.md](memory-management.md) **Cụm 3 (alignment/padding)** + [data-structures-cache.md](data-structures-cache.md) **Cụm 1 (cache)**. Hai thứ này giải thích được **phần lớn** câu hỏi hiệu năng ở phỏng vấn |
| **2 giờ** | Trọn [data-structures-cache.md](data-structures-cache.md) + [memory-management.md](memory-management.md) Cụm 1–3 |
| **1 buổi** | Cả ba file, theo thứ tự: cache → memory → measuring |
| **Có máy, muốn LÀM** | Gõ lại ba đoạn đo được: (1) ma trận `[i][j]` vs `[j][i]`, (2) `sizeof` trước/sau khi sắp lại member, (3) `std::string` 22 vs 23 ký tự với `operator new` bị nạp chồng. Cả ba đều chạy được trong **10 phút** và cho **số trên máy của bạn** |

---

## 🎯 Lộ trình ôn nhanh trước interview

Xếp theo **xác suất bị hỏi × độ chắc chắn ăn điểm**:

| Ưu tiên | Nội dung | Ở đâu |
|---|---|---|
| **1** 🔴 | **Alignment & padding** — tính `sizeof` một struct và sắp lại cho nhỏ đi | [memory-management](memory-management.md) Cụm 3 · Q4 |
| **2** 🔴 | **Cache line & locality** — vì sao `vector` thắng `list`, vì sao stride quan trọng | [data-structures-cache](data-structures-cache.md) Cụm 1 · Q1 |
| **3** 🔴 | **Vì sao `vector` dùng được trong real-time** (capacity chỉ đổi ở 3 chỗ) | [data-structures-cache](data-structures-cache.md) Cụm 2 · Q2 |
| **4** 🟠 | **Tránh heap trên embedded**: `std::pmr` + `monotonic_buffer_resource` + `null_memory_resource` | [memory-management](memory-management.md) Cụm 6 · Q8 |
| **5** 🟠 | **`make_shared` vs `shared_ptr{new}`** (1 lần cấp phát vs 2) | [memory-management](memory-management.md) Cụm 4 · Q6 |
| **6** 🟠 | **Sampling profiler mù với thread đang ngủ** — CPU 5% mà vẫn chậm | [measuring-performance](measuring-performance.md) Cụm 2 · Q1 |
| **7** 🟡 | **Amdahl's law** — biết trần trước khi bỏ công tối ưu | [measuring-performance](measuring-performance.md) Cụm 3 · Q3 |
| **8** 🟡 | **SSO** — `std::string` 24 byte chứa 22 ký tự không chạm heap | [memory-management](memory-management.md) Cụm 5 · Q7 |
| **9** 🟡 | **Parallel arrays / SoA** + ba nhược điểm của nó | [data-structures-cache](data-structures-cache.md) Cụm 6 · Q6 |

---

## 🗺️ Bản đồ: chương ↔ cụm ↔ topic liên quan

| File | Chương sách | Cụm bên trong | Topic repo liên quan |
|---|---|---|---|
| **[memory-management.md](memory-management.md)** 🎯 | **ch. 7** (tr. 191–236) | 1 bộ nhớ máy tính & tiến trình · 2 `new`/placement new · **3 alignment & padding** 🎯 · 4 ownership & smart pointer · 5 small object optimization · **6 arena/allocator/PMR** 🎯 | [03/memory-management](../../03-operating-system/memory-management.md) · [08/constraints](../../08-embedded-systems/constraints.md) · [08/memory-and-startup](../../08-embedded-systems/memory-and-startup.md) · [02/raii-smart-pointers](../../02-modern-cpp/raii-smart-pointers.md) |
| **[data-structures-cache.md](data-structures-cache.md)** 🎯 | **ch. 4** (tr. 99–130) + phần chọn container của ch. 5 | **1 cache hierarchy** 🎯 · 2 sequence container · 3 associative & hash · 4 `string_view`/`span` · 5 độ phức tạp vs overhead · **6 parallel arrays** 🎯 | [12-dsa/complexity-and-structures](../../12-dsa/complexity-and-structures.md) · [12-dsa/ring-buffer](../../12-dsa/ring-buffer.md) |
| **[measuring-performance.md](measuring-performance.md)** | **ch. 3** (tr. 71–98) | 1 nói đúng ta đang đo gì · **2 hai loại profiler** 🎯 · **3 microbenchmark & Amdahl** 🎯 | [09-debugging/tools](../../09-debugging/tools.md) · [cpp-mindset/scale-gpu-profiling](../cpp-mindset/scale-gpu-profiling.md) |

---

## 🔢 Mọi con số đo được trong bản summary này

Để tra nhanh — tất cả đều là **số thật sách in ra**, không phải ước lượng:

| Số đo | Giá trị | Ở đâu |
|---|---|---|
| Cache line | **64 byte** | data-structures Cụm 1 |
| L1 / L2 / RAM latency | **0,5 / 7 / 100 ns** | data-structures Cụm 1 |
| L1d / L2 / L3 | **32 KB / 256 KB / 8 MB** | data-structures Cụm 1 |
| Ma trận `[i][j]` vs `[j][i]` | **40 ms vs 800 ms** (20×) | data-structures Cụm 1 |
| Duyệt object 8 B vs 260 B | **1 ms vs 10 ms** | data-structures Cụm 6 |
| User 128 B → 40 B → SoA | **11 ms → 4 ms → 0,7 ms** | data-structures Cụm 6 |
| `std::vector<bool>` count | **0,03 ms** (bit array) | data-structures Cụm 6 |
| Stack frame mỗi lời gọi | **24 byte** (int 4 B + 20 B return addr/padding) | memory Cụm 1 |
| Stack tối đa | **~8 MB** (macOS/Linux) · **1 MB** (Windows) | memory Cụm 1 |
| `Document` sắp lại member | **24 → 16 byte** | memory Cụm 3 |
| Hai `new char` liên tiếp cách nhau | **16 byte** (`max_align_t`) | memory Cụm 3 |
| `make_shared` vs `shared_ptr{new}` | **1 lần (32 B) vs 2 lần (4 B + 32 B)** | memory Cụm 4 |
| `std::string` SSO (libc++) | **24 B stack, 22 ký tự, 0 B heap**; ký tự thứ 23 → 32 B heap | memory Cụm 5 |
| `sizeof(vector<int>)` vs có allocator có state | **24 vs 32 byte** | memory Cụm 6 |
| Amdahl: s=2, p=0,01 | overall **1,005×** | measuring Cụm 3 |

---

## ⚠️ Những chỗ đánh dấu 🆕 trong ba file

Theo [quy ước topic 15 §4](../README.md), mọi ý **không phải của sách** đều mang 🆕. Trong bản này chúng gồm: liên hệ iOS/Android → thiết bị nhúng Linux tắt swap; `steady_clock` ↔ `CLOCK_MONOTONIC`; `null_memory_resource` làm lá chắn ngân sách bộ nhớ; cách đọc nhanh output `perf stat`; ghi chú C++17 vs C++20 cho `contains`/`erase`/`span`; và nhận xét SoA hợp ECS chứ không hợp mọi thứ.
