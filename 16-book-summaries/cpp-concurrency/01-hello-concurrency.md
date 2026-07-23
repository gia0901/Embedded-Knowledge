# Ch. 1 — Hello, world of concurrency in C++! (tr. 1–15) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **⏮ (đầu sách)** · **[Ch. 2 → Managing threads](02-managing-threads.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> Chương này là chương **khái niệm nền** — không có API phức tạp, nhưng đặt ra bộ từ vựng dùng suốt sách và chứa **3 câu hỏi phỏng vấn kinh điển** (concurrency vs parallelism, process vs thread, khi nào KHÔNG nên dùng concurrency).

---

## Cụm 1 — Concurrency là gì (tr. 2–7)

### 1.1 Hai cách máy tính "làm nhiều việc cùng lúc" (tr. 2–4)

**Định nghĩa nền (tr. 2):** ở mức đơn giản nhất, concurrency là **hai hoặc nhiều hoạt động riêng biệt xảy ra cùng lúc**. Trong máy tính: *"a single system performing multiple independent activities in parallel, rather than sequentially, or one after the other"* (tr. 2).

Có **hai cơ chế** tạo ra hiện tượng đó, và phân biệt chúng là nền của cả chương:

**(a) Task switching — ảo giác concurrency (tr. 3).** Máy một core chỉ chạy **một task tại một thời điểm**, nhưng **chuyển đổi giữa các task rất nhiều lần mỗi giây**: làm một chút của task này, một chút của task kia. Vì chuyển quá nhanh, người dùng *và cả ứng dụng* không nhận ra → *"The task switching provides the illusion of concurrency to both the user and the applications themselves"* (tr. 3).

**(b) Hardware concurrency — concurrency thật (tr. 3).** Máy nhiều processor / nhiều **core** trên một chip (multicore) chạy được **nhiều task thật sự song song**. Con số cần quan tâm là **hardware threads** — *"the measure of how many independent tasks the hardware can genuinely run concurrently"* (tr. 4). Lưu ý: một số processor chạy được **nhiều thread trên một core** (hyper-threading), nên hardware threads ≠ số core vật lý.

Sách vẽ (Figure 1.1, tr. 3) hai task, mỗi task chia 10 khối đều nhau:

```
Dual core:   Core 1  [ task A: ▓▓▓▓▓▓▓▓▓▓ ]        ← mỗi task một core, chạy THẬT song song
             Core 2  [ task B: ▓▓▓▓▓▓▓▓▓▓ ]

Single core:         [A|B|A|B|A|B|A|B|A|B|A|B...]  ← đan xen; các vạch ngăn DÀY hơn
                        ↑ mỗi lần đổi = CONTEXT SWITCH = mất thời gian
```

Chi tiết vì sao vạch ngăn ở bản single-core dày hơn — **cái giá của context switch (tr. 3):** OS phải **lưu CPU state + instruction pointer** của task đang chạy, **tìm task kế tiếp**, rồi **nạp lại CPU state** cho task đó. Chưa hết: *"The CPU will then potentially have to load the memory for the instructions and data for the new task into the cache, which can prevent the CPU from executing any instructions, causing further delay"* (tr. 3) — tức cache nguội là phần chi phí *gián tiếp*, thường lớn hơn phần lưu/nạp thanh ghi.

> 🆕 *Liên hệ (tùy chọn, đọc để hiểu sâu tầng OS):* cơ chế context switch đầy đủ — ai kích hoạt, timer interrupt, TLB flush — ở [ostep/virtualization-cpu.md](../ostep/virtualization-cpu.md) cụm 2; tầng cache/TLB ở [cpp-mindset/understanding-the-machine.md](../cpp-mindset/understanding-the-machine.md). Bạn **không cần** đọc chúng để theo cuốn này; phần cần thiết đã tóm ở trên.

**Thực tế luôn là hỗn hợp (tr. 4):** ngay cả máy có hardware concurrency thật, số task vẫn thường **nhiều hơn** số hardware thread (một desktop bình thường có hàng trăm task chạy nền) → task switching vẫn được dùng. Figure 1.2 (tr. 4) minh họa 4 task đan xen trên 2 core.

**⚠️ Cảnh báo quan trọng nhất của mục này (tr. 3)** — và là lý do nó không chỉ là lý thuyết:

> *"Because there is only the illusion of concurrency, the behavior of applications may be subtly different when executing in a single-processor task-switching environment compared to when executing in an environment with true concurrency. In particular, **incorrect assumptions about the memory model (covered in chapter 5) may not show up in such an environment**."*

Nghĩa là: **code sai memory model có thể chạy "đúng" trên máy 1 core rồi vỡ trên máy nhiều core.** Đây là lý do bug concurrency nổi tiếng khó tái hiện (sách quay lại chủ đề này ở ch. 11 — testing).

### 1.2 Hai cách tổ chức concurrency: nhiều process hay nhiều thread (tr. 4–6)

Sách mở đầu bằng **phép ẩn dụ văn phòng** (tr. 4–5), rất đáng nhớ: hai lập trình viên làm chung một dự án —

- **Mỗi người một phòng riêng:** làm việc yên tĩnh, mỗi người có bộ tài liệu tham khảo riêng. Nhưng **giao tiếp phiền phức** (phải gọi điện/email/đi bộ sang phòng kia), và **tốn kém** (hai phòng phải quản lý, nhiều bản tài liệu phải mua).
- **Hai người chung một phòng:** nói chuyện tự do, vẽ chung lên bảng; chỉ một phòng, một bộ tài liệu. Nhưng **khó tập trung hơn** và **tranh chấp tài nguyên** (*"Where's the reference manual gone?"*).

→ Ánh xạ (tr. 5): **mỗi lập trình viên = một thread, mỗi phòng = một process.** Cách 1 = nhiều process single-thread; cách 2 = nhiều thread trong một process.

**Concurrency với nhiều PROCESS (tr. 5):** chia ứng dụng thành **nhiều process riêng biệt, độc lập**, giao tiếp qua các kênh **IPC** thông thường (signal, socket, file, pipe...) — Figure 1.3.

| Mặt trái (tr. 5) | Mặt phải (tr. 5) |
|---|---|
| Giao tiếp giữa process **phức tạp để dựng, hoặc chậm, hoặc cả hai** — vì OS *"typically provide a lot of protection between processes to avoid one process accidentally modifying data belonging to another"* | Chính **lớp bảo vệ đó** làm việc viết code concurrent an toàn **dễ hơn** so với thread |
| **Overhead cố hữu**: khởi động process tốn thời gian, OS phải cấp tài nguyên nội bộ để quản lý | Có thể chạy các process **trên các máy khác nhau nối qua mạng** → tăng parallelism sẵn có, chi phí giao tiếp cao hơn nhưng thiết kế khéo thì rất đáng |

Sách nêu ví dụ: **Erlang** dùng process làm khối xây dựng cơ bản của concurrency, rất hiệu quả (tr. 5).

**Concurrency với nhiều THREAD (tr. 6):** nhiều thread trong **một** process — Figure 1.4. Thread giống "process nhẹ": mỗi thread chạy độc lập, có thể chạy chuỗi lệnh khác nhau, **nhưng mọi thread chia sẻ cùng một address space**: biến global truy cập được từ mọi thread, pointer/reference truyền qua lại được.

- **Được:** overhead thấp hơn hẳn nhiều process, vì *"the operating system has less bookkeeping to do"* (tr. 6).
- **Trả giá:** *"if data is accessed by multiple threads, the application programmer must ensure that the view of data seen by each thread is consistent whenever it's accessed"* (tr. 6) — đây chính là chủ đề của chương 3, 4, 5, 8.

**Vì sao cuốn sách chỉ nói về thread (tr. 6)** — lý do rất thực tế, đáng biết: (1) overhead thấp khiến thread là *"the favored approach to concurrency in mainstream languages, including C++"*; (2) **C++ Standard không hề hỗ trợ giao tiếp giữa process** → ứng dụng đa process buộc phải dùng API riêng của nền tảng. Vì vậy *"future references to concurrency assume that this is achieved by using multiple threads."*

> 🆕 *Liên hệ (tùy chọn):* so sánh process vs thread ở tầng OS (address space, clone flags, cô lập lỗi) có ở [ostep/concurrency.md](../ostep/concurrency.md) cụm 1 và [lkd/process-sched-syscalls.md](../lkd/process-sched-syscalls.md). Bảng so sánh đủ dùng cho phỏng vấn đã nằm trong **Góc interview** cuối file này.

### 1.3 Concurrency vs parallelism — khác nhau ở đâu (tr. 6–7)

Hai từ **chồng lấn nghĩa rất nhiều**; với code đa luồng, *"to many, they mean the same thing"* (tr. 6). Khác biệt nằm ở **sắc thái, trọng tâm và ý định**:

| | **Parallelism** | **Concurrency** |
|---|---|---|
| Mối bận tâm chính | **Hiệu năng** — tận dụng phần cứng sẵn có để xử lý **khối dữ liệu lớn** nhanh hơn | **Separation of concerns** (tách bạch mối quan tâm) hoặc **responsiveness** (đáp ứng) |
| Câu hỏi điển hình | "Chia dữ liệu ra cho 8 core thì có nhanh gấp 8 không?" | "Làm sao UI không đơ khi đang xử lý nền?" |

Nguyên văn (tr. 7): *"People talk about parallelism when their primary concern is taking advantage of the available hardware to increase the performance of bulk data processing, whereas people talk about concurrency when their primary concern is separation of concerns, or responsiveness."* Sách cũng thành thật: *"This dichotomy is not cut and dried"* — ranh giới không tuyệt đối, nhưng biết nó giúp thảo luận rõ ràng hơn.

### Insight đáng nhớ

- Câu **⚠️ ở tr. 3** ("giả định sai về memory model có thể không lộ ra trên môi trường task-switching một core") là mảnh ghép giải thích vì sao concurrency bug đáng sợ: **test pass không chứng minh code đúng**. 🆕 Hệ quả thực hành: code đa luồng phải được lập luận đúng theo **memory model** (ch. 5), không phải "chạy thử thấy ổn"; và nên test trên máy nhiều core, tải cao, nhiều kiến trúc (ARM ordering yếu hơn x86 — bug lộ ra ở ARM mà không lộ ở x86 là chuyện thường).
- Phép ẩn dụ văn phòng (tr. 4–5) nén trọn trade-off process/thread thành một hình ảnh nhớ được: **phòng riêng = an toàn nhưng giao tiếp đắt; chung phòng = giao tiếp rẻ nhưng phải giữ trật tự**. Dùng luôn được khi trả lời phỏng vấn.
- Cặp "parallelism = hiệu năng / concurrency = tách bạch & đáp ứng" (tr. 7) là **câu trả lời chuẩn** cho câu hỏi phân biệt hai khái niệm — và quan trọng hơn: nó cho biết **tiêu chí đánh giá thiết kế của bạn** là gì (throughput hay latency/responsiveness).

---

## Cụm 2 — Vì sao dùng (và vì sao KHÔNG dùng) concurrency (tr. 7–10)

Sách khẳng định chỉ có **hai lý do chính** (tr. 7): *"separation of concerns and performance. In fact, I'd go so far as to say that they're almost the only reasons"*.

### 2.1 Concurrency cho separation of concerns (tr. 7–8)

Tách bạch mối quan tâm gần như luôn là ý hay khi viết phần mềm: **gom code liên quan lại, tách code không liên quan ra** → dễ hiểu, dễ test, ít bug. Concurrency cho phép tách các mảng chức năng **ngay cả khi chúng cần xảy ra cùng lúc**.

**Ví dụ xương sống — ứng dụng phát DVD (tr. 7):** hai nhóm trách nhiệm — (1) đọc dữ liệu từ đĩa, giải mã hình/tiếng, đẩy ra phần cứng đồ họa/âm thanh **đúng nhịp** để phim không giật; (2) nhận input người dùng (bấm Pause, Return To Menu, Quit).

- **Một thread:** phải **kiểm tra input định kỳ giữa lúc phát**, trộn lẫn code UI vào code phát DVD → rối.
- **Đa luồng:** một thread lo UI, một thread lo phát → *"the user interface code and DVD playback code no longer have to be so closely intertwined"*. Vẫn có tương tác (khi bấm Pause) nhưng *"now these interactions are directly related to the task at hand"* (tr. 7).

Kết quả là **ảo giác đáp ứng (responsiveness)**: thread UI phản hồi ngay tức thì, kể cả khi phản hồi chỉ là hiện con trỏ bận hay thông báo "Please Wait" trong lúc thread khác làm việc (tr. 7).

**Điểm quan trọng dễ bỏ qua (tr. 8):** trong kiểu dùng này, *"the number of threads is independent of the number of CPU cores available, because the division into threads is based on the conceptual design rather than an attempt to increase throughput."* → Chia thread theo **thiết kế khái niệm**, không theo số core.

### 2.2 Concurrency cho hiệu năng: task parallelism vs data parallelism (tr. 8–9)

Bối cảnh lịch sử (tr. 8): các hãng chip ngày càng chọn thiết kế **multicore** (2, 4, 16+ core trên một chip) thay vì làm một core nhanh hơn. Sức mạnh tăng thêm **không đến từ chạy một task nhanh hơn mà từ chạy nhiều task song song**. Sách trích câu nổi tiếng của **Herb Sutter: "The free lunch is over"** (bài báo 2005, Dr. Dobb's Journal — tr. 8): phần mềm muốn hưởng sức mạnh mới thì **phải được thiết kế để chạy nhiều task đồng thời**.

Hai cách dùng concurrency cho hiệu năng:

1. **Task parallelism (tr. 8):** chia **một task** thành nhiều phần, chạy song song → giảm tổng thời gian chạy. Nghe đơn giản nhưng có thể phức tạp vì **phụ thuộc giữa các phần**. Chia theo hai kiểu:
   - theo **xử lý**: thread này làm phần này của thuật toán, thread kia làm phần khác;
   - theo **dữ liệu**: mỗi thread làm **cùng một thao tác** trên **phần dữ liệu khác nhau** → gọi riêng là **data parallelism**.
2. **Dùng parallelism để giải bài toán LỚN HƠN (tr. 8–9):** thay vì xử lý một file tại một thời điểm, xử lý 2/10/20 file. *"It still takes the same amount of time to process one chunk of data, but now more data can be processed in the same amount of time"* — cùng là data parallelism nhưng **trọng tâm khác**: không phải làm nhanh hơn mà là **làm được nhiều hơn / làm được thứ trước đây bất khả** (ví dụ sách nêu: tăng độ phân giải video nếu các vùng ảnh xử lý song song).

**Thuật ngữ đáng nhớ — "embarrassingly parallel" (tr. 8):** thuật toán **dễ song song hóa đến mức "ngượng"**. Tên khác: *naturally parallel*, *conveniently concurrent*. Đặc tính quý: **scalability tốt** — hardware thread tăng bao nhiêu thì tăng song song bấy nhiêu, đúng câu *"Many hands make light work."* Phần **không** embarrassingly parallel thì thường chỉ chia được thành **số task cố định** (do đó **không** scale).

### 2.3 Khi nào KHÔNG nên dùng concurrency (tr. 9–10) 🎯

Sách nhấn: *"It's just as important to know when not to use concurrency as it is to know when to use it"* (tr. 9). Nguyên tắc gốc: **chỉ không dùng khi lợi ích không đáng cái giá**. Các cái giá cụ thể:

1. **Chi phí trí tuệ (tr. 9):** code concurrent **khó hiểu hơn** → tốn công viết + bảo trì, và **phức tạp thêm dẫn tới nhiều bug hơn**. Nếu lợi ích hiệu năng không đủ lớn, hoặc separation of concerns không đủ rõ để biện minh cho thời gian phát triển + chi phí bảo trì → **đừng dùng**.
2. **Chi phí khởi động thread (tr. 9):** OS phải cấp tài nguyên kernel + stack space, rồi thêm thread vào scheduler. Nếu task chạy **xong rất nhanh**, *"the time taken by the task may be dwarfed by the time taken to launch the thread, possibly making the overall performance of the application worse than if the task had been executed directly by the spawning thread."*
3. **Thread là tài nguyên hữu hạn (tr. 9)** — con số đáng nhớ: mỗi thread cần **stack riêng**. Trên process **32-bit** với address space phẳng 4 GB, nếu mỗi thread có stack **1 MB** (điển hình), thì **~4.096 thread là cạn sạch address space**, chưa chừa chỗ cho code, static data, heap. Hệ 64-bit không dính giới hạn trực tiếp này nhưng vẫn hữu hạn.
4. **Quá nhiều thread → context switching ăn hết lợi ích (tr. 10):** *"Each context switch takes time that could be spent doing useful work, so at some point, adding an extra thread will reduce overall application performance rather than increase it."* → phải **điều chỉnh số thread theo hardware concurrency sẵn có**.
5. **Server một-thread-mỗi-kết-nối (tr. 9):** ổn với ít kết nối, nhưng *"can quickly exhaust system resources"* với server tải cao → dùng **thread pool** (ch. 9).

Câu chốt (tr. 10): *"The use of concurrency for performance is like any other optimization strategy... it's only worth doing for those performance-critical parts of the application where there's the potential for measurable gain."*

### Insight đáng nhớ

- Mục 2.3 là **nguồn câu hỏi phỏng vấn "nhiều thread hơn có nhanh hơn không?"** với đủ đạn: overhead khởi động, giới hạn address space (~4096 thread/32-bit), context switching ăn lợi ích, oversubscription. Trả lời có **con số** (1MB stack × 4096 = 4GB) luôn ăn điểm hơn nói chung chung.
- Phân biệt hai *kiểu chia thread* rất đáng nhớ vì nó đổi cách bạn thiết kế: chia vì **separation of concerns** → số thread theo **thiết kế khái niệm**, độc lập số core (tr. 8); chia vì **hiệu năng** → số thread **phải bám hardware concurrency** (tr. 10). Nhầm hai cái này là nguồn của cả oversubscription lẫn thiết kế rối.
- 🆕 Với embedded/BSP: cả 5 cái giá trên đều **nặng hơn** trên thiết bị nhúng (RAM ít → stack thread là chi phí thật; core ít → oversubscription đến sớm; realtime → context switch là nguồn jitter). Nguyên tắc "pre-allocate + thread pool cố định, không tạo thread động trong đường nóng" xuất phát từ đây.

---

## Cụm 3 — Concurrency trong C++: lịch sử và hiện trạng (tr. 10–13)

### 3.1 Trước C++11: không hề có thread trong chuẩn (tr. 10–11)

**C++98 không thừa nhận sự tồn tại của thread (tr. 10):** hiệu ứng của các thành phần ngôn ngữ được mô tả theo một **máy trừu tượng tuần tự** (sequential abstract machine). Nghiêm trọng hơn: *"the memory model isn't formally defined, so you can't write multithreaded applications without compiler-specific extensions to the 1998 C++ Standard."*

Thực tế thời đó (tr. 10–11): các compiler vendor tự thêm extension, thường là cho phép dùng **C API của nền tảng** (POSIX threads, Windows API) và bảo đảm C++ Runtime Library (vd cơ chế exception handling) hoạt động khi có nhiều thread. Rồi các **class library** (MFC, **Boost**, ACE) bọc API nền tảng thành lớp C++.

**Một thiết kế chung của các class library đó rất đáng chú ý (tr. 11):** *"the use of the Resource Acquisition Is Initialization (**RAII**) idiom with locks to ensure that mutexes are unlocked when the relevant scope is exited."* → 🆕 Đây chính là tổ tiên của `std::lock_guard`/`std::unique_lock`/`std::scoped_lock` mà bạn sẽ gặp ở chương 3.

Vấn đề còn lại của thời kỳ này (tr. 11): thiếu **memory model nhận biết thread** gây rắc rối, đặc biệt cho người muốn *"gain higher performance by using knowledge of the processor hardware"* và người viết code cross-platform.

### 3.2 C++11 và các bản sau (tr. 11–12)

**C++11 (tr. 11)** đổi tất cả: có **thread-aware memory model** + thư viện chuẩn gồm các lớp cho: quản lý thread (ch. 2), bảo vệ dữ liệu chung (ch. 3), đồng bộ thao tác giữa thread (ch. 4), và **atomic operations mức thấp** (ch. 5). Nguồn gốc trực tiếp: *"The C++11 Thread Library is heavily based on the prior experience accumulated through the use of the C++ class libraries... In particular, the **Boost Thread Library** was used as the primary model"* — nhiều lớp trùng tên và cấu trúc với Boost, và sau đó Boost cũng sửa để khớp chuẩn (dòng chảy hai chiều).

**C++14 (tr. 12):** hỗ trợ concurrency thêm **duy nhất một mutex type mới** để bảo vệ dữ liệu chung (ch. 3). *(🆕 Đó là `std::shared_timed_mutex` — reader-writer lock; `std::shared_mutex` đến ở C++17.)*

**C++17 (tr. 12):** thêm **đáng kể** — *"a full suite of parallel algorithms"* (ch. 10). Ngoài ra có **Technical Specification (TS) cho concurrency** mô tả các mở rộng, đặc biệt quanh việc đồng bộ thao tác giữa thread (ch. 4).

Điểm sách nhấn về **atomic operations (tr. 12):** hỗ trợ atomic trực tiếp trong C++ cho phép viết code hiệu quả **với ngữ nghĩa xác định**, **không cần assembly riêng nền tảng**: *"not only does the compiler take care of the platform specifics, but the optimizer can be written to take into account the semantics of the operations, enabling better optimization of the program as a whole."*

### 3.3 Abstraction penalty — dùng thư viện chuẩn có chậm không? (tr. 12–13)

Mối lo kinh điển của dân HPC: bọc facility mức thấp bằng lớp C++ thì mất bao nhiêu hiệu năng? Chi phí đó gọi là **abstraction penalty** (tr. 12).

Câu trả lời của sách — cũng là **mục tiêu thiết kế** của Standards Committee (tr. 12):
- *"there should be little or no benefit to be gained from using the lower-level APIs directly, where the same facility is to be provided"* → thư viện được thiết kế để **hiện thực hiệu quả (abstraction penalty thấp) trên hầu hết nền tảng chính**.
- Đồng thời C++ cung cấp **đủ facility mức thấp** (atomic types + operations) cho người cần "work close to the metal", thay cho việc phải hạ xuống assembly riêng nền tảng → **portable và dễ bảo trì hơn**.
- Với các abstraction **mức cao hơn**: đôi khi *có* chi phí do code phụ trợ, nhưng *"in general, the cost is no higher than would be incurred by writing equivalent functionality by hand, and the compiler may inline much of the additional code anyway"* (tr. 13).

Và lời khuyên rất thực dụng khi profiling chỉ ra thư viện chuẩn là nút thắt (tr. 13): *"it may be due to poor application design rather than a poor library implementation. For example, if too many threads are competing for a mutex, it will impact the performance significantly. **Rather than trying to shave a small fraction of time off the mutex operations, it would probably be more beneficial to restructure the application so that there's less contention on the mutex**"* → giảm contention (ch. 8) thắng vi-tối-ưu mutex.

### 3.4 Facility riêng nền tảng: `native_handle()` (tr. 13)

Khi cần thứ mà chuẩn không có, các type trong C++ Thread Library **có thể** cung cấp member `native_handle()` trả về handle của hiện thực bên dưới để thao tác bằng API nền tảng. ⚠️ *"any operations performed using native_handle() are entirely platform dependent"* — và nằm ngoài phạm vi sách.

> 🆕 *Liên hệ nghề (tùy chọn):* đây chính là cửa mà embedded/realtime phải dùng — set **CPU affinity**, **scheduling policy `SCHED_FIFO`**, priority cho thread đều phải qua `pthread_*` với `native_handle()`. Chi tiết ở [melp/debug-realtime.md](../melp/debug-realtime.md) và [EMC++ Item 35](../effective-modern-cpp.md).

---

## Cụm 4 — Bắt đầu: Hello, Concurrent World (tr. 13–15)

Chương trình một thread làm mốc so sánh (tr. 14):

```cpp
#include <iostream>
int main() {
    std::cout << "Hello World\n";
}
```

Và bản đa luồng — **Listing 1.1** (tr. 14):

```cpp
#include <iostream>
#include <thread>              // (1) header MỚI: mọi thứ quản lý thread khai ở đây

void hello() {                 // (2) "initial function" của thread mới
    std::cout << "Hello Concurrent World\n";
}

int main() {
    std::thread t(hello);      // (3) khởi chạy thread mới, bắt đầu tại hello()
    t.join();                  // (4) thread gọi (main) CHỜ thread t xong
}
```

Sách giải thích từng khác biệt (tr. 14–15) — đọc kỹ vì mỗi ý là một khái niệm dùng suốt sách:

1. **`#include <thread>`** — khai báo cho hỗ trợ đa luồng nằm ở **header mới**: quản lý thread ở `<thread>`, còn **bảo vệ dữ liệu chung ở các header khác** (ch. 3).
2. **Mọi thread phải có một "initial function"** — nơi luồng thực thi mới bắt đầu. Với **thread khởi đầu** của ứng dụng thì đó là `main()`; với **mọi thread khác**, hàm đó được chỉ định trong **constructor của `std::thread`** — ở đây object `t` có `hello()` làm initial function.
3. Chương trình giờ có **hai thread**: thread khởi đầu (bắt đầu ở `main`) và thread mới (bắt đầu ở `hello`).
4. **Vì sao cần `join()` (tr. 14–15):** sau khi khởi chạy thread mới, thread khởi đầu **chạy tiếp**. Nếu không chờ, nó *"would merrily continue to the end of main() and end the program—possibly before the new thread had a chance to run."* `join()` khiến thread gọi (ở đây là `main`) **chờ** thread gắn với object `std::thread` đó (chi tiết ở ch. 2).

Sách tự thừa nhận (tr. 15): dùng nhiều thread cho việc bé thế này *"is—as described in section 1.2.3—generally not worth the effort"*; ví dụ chỉ để làm quen cú pháp.

**Câu chốt của cả chương (tr. 15):** *"In C++, using multiple threads isn't complicated in and of itself; **the complexity lies in designing the code so that it behaves as intended**."*

### Insight đáng nhớ

- Listing 1.1 chỉ 10 dòng nhưng chứa **ba khái niệm sẽ theo suốt sách**: (a) *initial function* — thread nào cũng phải có điểm bắt đầu; (b) **`std::thread` là một object quản lý vòng đời**, không phải bản thân thread; (c) **join = chờ**, và nếu không join thì có chuyện (ch. 2 sẽ cho thấy hậu quả nghiêm trọng hơn "có thể chưa kịp chạy": destructor của `std::thread` joinable gọi `std::terminate`).
- Câu chốt tr. 15 định vị đúng độ khó của mảng này: **API dễ, thiết kế khó** — đó là lý do 10 chương còn lại không nói về cú pháp mà nói về *đúng đắn* (ch. 3–5) và *thiết kế* (ch. 6–8).

---

## Góc interview

**Câu 1 (🎯 kinh điển):** Phân biệt **concurrency** và **parallelism**. Cho ví dụ mỗi bên.

<details><summary>Đáp án</summary>

- Hai khái niệm **chồng lấn nhiều**, khác nhau ở **trọng tâm và ý định** (tr. 6–7):
  - **Parallelism** — mối bận tâm là **hiệu năng**: tận dụng phần cứng sẵn có để xử lý **khối dữ liệu lớn** nhanh hơn. Ví dụ: chia mảng 10 triệu phần tử cho 8 core cùng tính tổng; xử lý song song các vùng ảnh để tăng độ phân giải video (tr. 9).
  - **Concurrency** — mối bận tâm là **separation of concerns** hoặc **responsiveness**: nhiều hoạt động độc lập tiến triển đồng thời, *không nhất thiết* để nhanh hơn. Ví dụ của sách: ứng dụng phát DVD — một thread lo UI, một thread lo giải mã/phát, để bấm Pause là phản hồi ngay dù đang phát (tr. 7).
- **Dấu hiệu phân biệt thực dụng:** hỏi *"số thread quyết định bởi cái gì?"* — nếu bởi **số core** (để tăng throughput) thì đang làm parallelism; nếu bởi **thiết kế khái niệm**, độc lập số core (tr. 8), thì đang làm concurrency.
- Điểm cộng: nêu rằng concurrency **có thể** đạt được **không cần** hardware song song — máy 1 core dùng **task switching** vẫn cho concurrency (ảo giác), còn parallelism thật thì cần **hardware concurrency** (tr. 3). Và nói rõ ranh giới không tuyệt đối (*"not cut and dried"* — tr. 7).

</details>

**Câu 2 (🎯 kinh điển):** Dùng **nhiều process** hay **nhiều thread** cho concurrency? Trade-off và khi nào chọn cái nào?

<details><summary>Đáp án</summary>

| | **Nhiều process** (tr. 5) | **Nhiều thread** (tr. 6) |
|---|---|---|
| Address space | **Riêng biệt**, OS bảo vệ mạnh | **Chung** — global/heap truy cập trực tiếp, truyền pointer/reference được |
| Giao tiếp | Qua **IPC** (signal, socket, file, pipe) — phức tạp để dựng, hoặc chậm, hoặc cả hai | Trực tiếp qua **shared memory** — nhanh, nhưng phải tự đồng bộ |
| Overhead | Khởi động tốn thời gian; OS tốn tài nguyên quản lý | Thấp hơn hẳn — *"the OS has less bookkeeping to do"* |
| An toàn | **Dễ viết code an toàn hơn** nhờ lớp bảo vệ của OS | Nguy hiểm hơn: lập trình viên phải tự bảo đảm mọi thread thấy dữ liệu **nhất quán** |
| Mở rộng | Chạy được **trên nhiều máy nối mạng** | Chỉ trong một máy/một process |

- **Chọn process** khi: cần **cô lập lỗi/bảo mật** (một thành phần chết không kéo cả hệ), cần chạy phân tán trên nhiều máy, hoặc muốn ưu tiên *độ an toàn khi viết* hơn tốc độ giao tiếp. Sách nêu **Erlang** dùng process làm khối cơ bản rất hiệu quả (tr. 5).
- **Chọn thread** khi: cần chia sẻ **nhiều dữ liệu** thường xuyên, cần spawn/switch rẻ, latency thấp. Đây là lựa chọn chủ đạo của C++ — và **C++ Standard không hỗ trợ giao tiếp giữa process**, nên đa process buộc dùng API riêng nền tảng (tr. 6).
- Ẩn dụ đáng dùng khi trả lời (tr. 4–5): **process = mỗi người một phòng riêng** (yên tĩnh, an toàn, nhưng gọi điện/đi bộ mới nói chuyện được, tốn hai bộ tài liệu); **thread = hai người chung phòng** (nói chuyện tự do, dùng chung tài liệu, nhưng dễ mất tập trung và tranh nhau tài nguyên).
- 🆕 Điểm cộng thực chiến: mô hình lai rất phổ biến — nhiều process (cô lập) mỗi process vài thread (hiệu năng), ví dụ trình duyệt (mỗi tab một process) hay nginx (worker process + event loop).

</details>

**Câu 3 (🎯):** Thêm thread có luôn làm chương trình nhanh hơn không? Khi nào **không** nên dùng concurrency?

<details><summary>Đáp án</summary>

**Không.** Sách dành hẳn mục 1.2.3 cho việc này (tr. 9–10) — nguyên tắc gốc: *"the only reason not to use concurrency is when the benefit isn't worth the cost."* Các chi phí cụ thể:

1. **Chi phí trí tuệ & bảo trì:** code concurrent khó hiểu hơn → tốn thời gian phát triển, **dễ sinh bug hơn**. Nếu lợi ích không đủ lớn để biện minh → đừng dùng (tr. 9).
2. **Overhead khởi động thread:** OS phải cấp kernel resource + stack, thêm vào scheduler. Task quá ngắn thì **thời gian khởi động thread lấn át thời gian làm việc** → chậm hơn cả chạy tuần tự (tr. 9).
3. **Thread là tài nguyên hữu hạn** — con số của sách: process **32-bit**, address space 4 GB, mỗi thread stack **1 MB** → **~4.096 thread là cạn address space**, chưa tính code/static data/heap. Hệ 64-bit không có giới hạn trực tiếp đó nhưng vẫn hữu hạn (tr. 9).
4. **Context switching:** *"at some point, adding an extra thread will reduce overall application performance rather than increase it"* — mỗi lần switch tốn thời gian lẽ ra làm việc có ích, cộng thêm **cache nguội** (tr. 3, 10). Đây là **oversubscription**.
5. **Server một-thread-mỗi-kết-nối:** ổn khi ít kết nối, **cạn tài nguyên hệ thống** khi tải cao → dùng **thread pool** (tr. 9, ch. 9).

**Chốt:** dùng concurrency cho hiệu năng là **một chiến lược tối ưu hóa như mọi tối ưu khác** — chỉ đáng làm ở phần performance-critical *"where there's the potential for measurable gain"* (tr. 10) → **đo trước, đừng đoán**. Ngoại lệ: nếu mục tiêu là **separation of concerns/responsiveness** (không phải throughput) thì vẫn đáng dùng dù không nhanh hơn, và khi đó số thread theo **thiết kế khái niệm**, không theo số core (tr. 8).

🆕 Với embedded: mọi chi phí trên nặng hơn (RAM ít, core ít, realtime nhạy jitter) → ưu tiên **thread pool cố định, pre-allocate, không tạo thread trong đường nóng**.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [ostep/concurrency.md](../ostep/concurrency.md) — thread/lock/CV nhìn từ tầng OS; [ostep/virtualization-cpu.md](../ostep/virtualization-cpu.md) — context switch chi tiết.
- [03-operating-system/process-thread.md](../../03-operating-system/process-thread.md) — bản cô đọng process vs thread của repo.
- [cpp-mindset/understanding-the-machine.md](../cpp-mindset/understanding-the-machine.md) — hardware concurrency, core/hyper-threading, OpenMP.
- [EMC++ cụm 7](../effective-modern-cpp.md) — 6 lời khuyên về concurrency API (async vs thread, `native_handle` gián tiếp qua Item 35).

**Chương tiếp theo:** [Ch. 2 — Managing threads →](02-managing-threads.md) (khởi chạy, join/detach, truyền tham số, chuyển ownership, chọn số thread).
