# Ch. 8 — Designing concurrent code (tr. 251–299) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 7 — Lock-free structures](07-lock-free-structures.md)** · **[Ch. 9 → Advanced thread management](09-advanced-thread-management.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **🎯 Chương thiết kế code concurrent nói chung** — từ cấu trúc dữ liệu (ch. 6–7) mở rộng ra **thiết kế toàn ứng dụng**. Giá trị interview: **false sharing, cache ping-pong, Amdahl's law** là các khái niệm hiệu năng hay bị hỏi, và **giao với BSP** ([cpp-mindset ch.4](../cpp-mindset/understanding-the-machine.md) — cache line, MESI). Đây cũng là chương giải thích **vì sao lock-free (ch. 7) đôi khi chậm hơn lock-based**.

**Sách nêu mục tiêu chương (tr. 251):** kỹ thuật chia dữ liệu giữa thread · các yếu tố ảnh hưởng hiệu năng · ảnh hưởng tới thiết kế cấu trúc dữ liệu · exception safety đa luồng · scalability · ví dụ parallel algorithm.

---

## Cụm 1 — Kỹ thuật chia việc giữa thread (tr. 252–260)

**Ẩn dụ xây nhà (tr. 252–253):** làm một mình lâu; thuê "thợ đa năng" (ai cũng làm mọi việc, đổi tool liên tục) hay "thợ chuyên" (thợ điện, thợ nước — nhanh hơn nhưng có lúc ngồi chơi khi không có việc của mình). Với thread cũng vậy: chọn số thread + thread "generalist" hay "specialist".

### 1.1 Chia DỮ LIỆU trước khi xử lý (tr. 253–254)

Đơn giản nhất: chia dải thành khối liền nhau, mỗi thread một khối, **xử lý độc lập không giao tiếp** tới khi xong. Quen thuộc với ai dùng **MPI/OpenMP**: chia task song song → worker chạy độc lập → **bước reduction cuối** gộp kết quả (như `parallel_accumulate` §2.4). **⚠️ Nhận ra bước cuối là reduction quan trọng** — bản ngây thơ làm reduction **tuần tự**, nhưng thường **song song hóa được** (đệ quy, hoặc worker tự làm một phần reduction khi xong).

### 1.2 Chia dữ liệu ĐỆ QUY (tr. 254–256)

Quicksort **không chia trước được** (chỉ biết phần tử thuộc "nửa" nào sau khi xử lý). Dùng **bản chất đệ quy**: các lời gọi đệ quy độc lập (truy cập tập phần tử riêng) → chạy concurrent. Bản ch. 4 dùng `std::async` cho nửa dưới. **⚠️ Nhưng spawn thread mỗi lần đệ quy → quá nhiều thread** → chậm, hoặc cạn thread. Giải: dùng `hardware_concurrency()` chọn số thread cố định + **thread-safe stack** để đẩy chunk chờ sort; thread rảnh thì lấy chunk từ stack (Listing 8.1 — "help" bằng cách tự sort chunk còn dang dở).

### 1.3 Chia việc theo LOẠI TASK (tr. 258–260)

Thread "specialist" — mỗi thread một **task khác nhau** (separation of concerns). Đây là good design cơ bản: mỗi phần code một trách nhiệm.

**Tách concern (tr. 258–259):** đơn luồng phải tự tay xen kẽ "làm tí A, tí B, kiểm phím, kiểm mạng" → code A phức tạp vì phải lưu state + trả điều khiển về main loop định kỳ. Đa luồng: mỗi task một thread, **OS tự lưu state + switch** → code đơn giản hơn, UI phản hồi kịp thời.

**⚠️ Hai nguy hiểm khi tách concern (tr. 259):** (1) **tách nhầm concern** — triệu chứng: **nhiều dữ liệu chung** giữa thread hoặc thread **chờ nhau nhiều** (= giao tiếp quá nhiều). Nếu vậy: gộp việc liên quan vào một thread, hoặc gộp hai thread hay giao tiếp thành một.

**Pipeline — chia CHUỖI task (tr. 259–260):** cùng chuỗi thao tác trên nhiều dữ liệu độc lập → mỗi thread một **stage**; xong thì đẩy vào queue cho thread kế. Hợp khi (a) dữ liệu chưa biết hết lúc bắt đầu (đến qua mạng); (b) mỗi thao tác tốn giờ. **Khác chia-dữ-liệu ở performance profile:** chia dữ liệu xử lý theo lô (4 item mỗi 12s); pipeline cho ra **đều đặn** (1 item mỗi 3s sau khi "mồi"). → Tốt cho video (25 fps đều đặn quan trọng hơn throughput cao mà giật).

### Insight đáng nhớ (Cụm 1)
- **Ba cách chia việc:** dữ liệu trước (biết trước, reduction cuối), đệ quy (quicksort, phải giới hạn số thread), theo task (specialist, pipeline). Chọn theo bản chất bài toán.
- **Pipeline đổi throughput lấy độ đều đặn** — hợp khi latency đều quan trọng hơn tổng thời gian (video, streaming).
- **Tách nhầm concern = giao tiếp quá nhiều.** Dấu hiệu: nhiều shared data hoặc thread chờ nhau → xem lại ranh giới thread.

---

## Cụm 2 — 🎯 Các yếu tố ảnh hưởng hiệu năng (tr. 260–266)

Đây là cụm giá trị interview cao nhất chương.

### 2.1 Số processor & oversubscription (tr. 261–262)

Một 16-core ≈ 4 quad-core ≈ 16 single-core (chạy 16 thread concurrent). **< 16 thread → lãng phí processor; > 16 thread sẵn sàng chạy → oversubscription** (phí thời gian switch). `hardware_concurrency()` giúp scale. **⚠️ Nhưng dùng trực tiếp phải cẩn thận (tr. 261):** nó **không biết các thread khác** trong hệ; nếu nhiều hàm cùng gọi nó để scale → **oversubscription khủng khiếp**. `std::async` tránh được (thư viện biết mọi lời gọi); thread pool cũng vậy.

### 2.2 🎯 Data contention & cache ping-pong (tr. 262–264)

**Nền tảng (tr. 262):** hai thread cùng **đọc** cùng dữ liệu → ok (copy vào cache mỗi core). Một thread **ghi** → thay đổi phải **lan sang cache core kia** → tốn thời gian, có thể **khiến core kia dừng chờ**. Về mặt lệnh CPU, **cực chậm — tương đương hàng trăm lệnh**.

```cpp
std::atomic<unsigned long> counter(0);
void processing_loop() {
    while (counter.fetch_add(1, std::memory_order_relaxed) < 100000000)
        do_something();
}
```
`counter` global → mỗi increment, processor phải **có bản mới nhất trong cache, sửa, công bố cho core khác**. Dù `relaxed`, `fetch_add` là RMW nên phải lấy giá trị mới nhất. Nhiều core chạy code này → **counter bị chuyền qua lại giữa các cache nhiều lần = cache ping-pong** (tr. 263). Processor dừng chờ cache transfer = không làm được gì.
- **High contention:** processor thường xuyên phải chờ nhau.
- **Low contention:** hiếm khi chờ.

**⚠️ Mutex cũng bị (tr. 263–264):** lock mutex = transfer + sửa dữ liệu mutex; unlock = sửa lại. Càng nhiều core đụng cùng dữ liệu + mutex → **càng nhiều contention**. Điểm khác: mutex **serialize ở tầng OS** (OS schedule thread khác khi một thread chờ mutex), còn atomic **stall ở tầng processor** (không thread nào chạy được trên core đó). **⚠️ Cả reader-writer mutex cũng bị:** mọi thread (kể cả reader) vẫn phải **sửa chính mutex** → cache line chứa mutex bị chuyền → nuốt hết lợi ích của `shared_mutex`.

**Cách tránh (tr. 264):** **giảm khả năng hai thread tranh cùng memory location.**

### 2.3 🎯 False sharing (tr. 264–265)

**Cache không xử lý từng byte mà theo CACHE LINE (thường 32/64 byte) (tr. 264).** Dữ liệu nhỏ liền kề → **cùng một cache line**.

**⚠️ False sharing:** mảng `int`, mỗi thread ghi **phần tử RIÊNG** của mình. Nhưng vì `int` nhỏ hơn cache line, **nhiều phần tử chung một cache line** → mỗi lần thread ghi phần tử 0, **cache line phải chuyển sang core đó**, rồi lại chuyển sang core ghi phần tử 1... **Cache line bị chia sẻ dù DỮ LIỆU thì không** → gọi là *false sharing*.

**Giải (tr. 265):** cấu trúc dữ liệu sao cho phần tử **cùng thread truy cập thì gần nhau** (cùng cache line), phần tử **thread khác truy cập thì xa nhau** (khác cache line). **🆕 C++17: `std::hardware_destructive_interference_size`** (header `<new>`) = số byte tối đa có thể bị false sharing → đặt dữ liệu cách nhau ít nhất chừng đó là hết false sharing.

### 2.4 Data proximity — dữ liệu của MỘT thread gần nhau không (tr. 265–266)

Ngược với false sharing: nếu dữ liệu của **một thread** rải rác → nằm trên **nhiều cache line** → tăng latency, phí chỗ cache (cache line chứa cả dữ liệu không cần → cache miss nhiều hơn). **🆕 C++17: `std::hardware_constructive_interference_size`** = số byte tối đa đảm bảo cùng cache line → nhét dữ liệu cần-cùng-nhau trong chừng đó → giảm cache miss.

**⚠️ Task switching làm nặng thêm (tr. 265–266):** nhiều thread hơn core → mỗi core chạy nhiều thread → switch thì phải **reload cache line**; dữ liệu rải rác → reload nhiều hơn. OS còn có thể schedule thread lên core khác giữa các time slice → **transfer cache line giữa core**.

### 2.5 Oversubscription & task switching thừa (tr. 266)

Thường có **nhiều thread hơn processor** (thread chờ I/O/mutex/condvar) — bình thường ok (thread thừa làm việc có ích khi thread khác chờ). **⚠️ Nhưng quá nhiều thread sẵn-sàng-chạy → OS switch nhiều → overhead + cache problem.** Nguồn oversubscription: spawn thread không giới hạn (quicksort đệ quy ch. 4), hoặc số thread tự nhiên khi chia theo task > số core mà việc lại CPU-bound. Giải: **giới hạn số worker thread**.

### Insight đáng nhớ (Cụm 2)
- **Cache ping-pong = dữ liệu chung bị chuyền qua lại giữa cache các core.** Chậm khủng khiếp (~hàng trăm lệnh). Xảy ra cả với atomic counter LẪN mutex (kể cả reader-writer). **Giảm bằng cách giảm memory chung giữa thread.**
- **🎯 False sharing = dữ liệu KHÁC nhau nhưng chung cache line.** Mỗi thread ghi biến riêng vẫn ping-pong. Giải: **padding/align** để mỗi thread một cache line (`hardware_destructive_interference_size`).
- **Đây là lý do lock-free đôi khi chậm hơn lock-based:** atomic tạo cache ping-pong nặng (ch. 7 đã nhắc). Luôn **đo**.

---

## Cụm 3 — Thiết kế cấu trúc dữ liệu cho đa luồng (tr. 266–276)

Ba yếu tố hiệu năng cần cân nhắc khi thiết kế: **contention, false sharing, data proximity** (tr. 271).

**Ví dụ chia phần tử ma trận (tr. 271–272):** nhân ma trận — chia theo **hàng, cột, hay khối chữ nhật**? Chia theo **cột** (mỗi thread một dải cột liền nhau) → mỗi thread đọc **cả ma trận A + một dải B**, ghi một dải C liền nhau → **ít false sharing** (thread làm trên cache line riêng). Chọn cách chia để **tối đa dữ liệu cùng-thread gần nhau, tối thiểu chung cache line**.

**Test false sharing (tr. 274):** thêm **padding lớn** giữa các phần tử; nếu hiệu năng cải thiện → false sharing là vấn đề (rồi tìm cách sửa gọn hơn). 🆕 Đây là kỹ thuật chẩn đoán thực dụng: nghi false sharing thì thử padding, đo trước-sau.

### Insight đáng nhớ (Cụm 3)
- **Cách chia dữ liệu quyết định hiệu năng nhiều như bản thân thuật toán.** Cùng thuật toán, chia theo hàng/cột/khối cho hiệu năng khác hẳn (false sharing + proximity).
- **Chẩn đoán false sharing bằng padding thử nghiệm** — cải thiện → có false sharing.

---

## Cụm 4 — Exception safety, Amdahl's law, ẩn latency, responsiveness (tr. 276–282)

### 4.1 🎯 Exception safety trong parallel algorithm (tr. 276–277)

**⚠️ Khó hơn code tuần tự (tr. 276):** trong code tuần tự, exception cứ propagate lên caller. Trong parallel, thao tác chạy trên **thread khác** → exception **không propagate được** (sai call stack); **hàm spawn trên thread mới thoát bằng exception → `std::terminate`, cả app chết.**

**`parallel_accumulate` ngây thơ (Listing 8.2) KHÔNG exception-safe:** nhiều điểm ném — sau khi tạo thread đầu tiên, bất kỳ exception nào → destructor `std::thread` gọi `terminate`; `accumulate_block` trên thread mới ném → không catch → `terminate`.

**Giải 1 — `std::packaged_task` + `std::future` (Listing 8.3, tr. 273):** đổi `accumulate_block` trả giá trị trực tiếp; dùng `packaged_task`/`future` **truyền cả kết quả LẪN exception** — `future.get()` **ném lại** exception thread worker bắt được. (Nếu nhiều thread ném, chỉ một được propagate; dùng `std::nested_exception` để bắt hết.) Vẫn cần catch để join thread còn joinable rồi rethrow (tránh leak thread).

**Giải 2 — `std::async` (Listing 8.5, tr. 276–277) — GỌN NHẤT & tự động exception-safe:**
```cpp
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    unsigned long const max_chunk_size = 25;
    if (length <= max_chunk_size) {
        return std::accumulate(first, last, init);            // đủ nhỏ → làm trực tiếp
    } else {
        Iterator mid_point = first;
        std::advance(mid_point, length / 2);
        std::future<T> first_half = std::async(               // nửa đầu: async
            parallel_accumulate<Iterator, T>, first, mid_point, init);
        T second_half = parallel_accumulate(mid_point, last, T());  // nửa sau: đệ quy trực tiếp
        return first_half.get() + second_half;                // gộp
    }
}
```
**Vì sao tự động exception-safe (tr. 277):** nếu lời gọi đệ quy (`second_half`) ném → `future` của `std::async` **bị hủy khi exception propagate** → destructor **chờ task async xong** → không leak thread; nếu task async ném → `future` bắt, `get()` **ném lại**. Thư viện tự dùng hardware thread hợp lý (một số "async" chạy đồng bộ trong `get()`).

### 4.2 🎯 Scalability & Amdahl's law (tr. 277–279)

**Chia chương trình thành phần "serial" (một thread làm) và "parallel" (mọi processor làm) (tr. 278).** Với N processor, phần parallel nhanh hơn, phần serial giữ nguyên. **Amdahl's law:**

```
              1
    P = ───────────────
              1 − fₛ
        fₛ + ────────
                 N
```
Trong đó `fₛ` = phần serial, N = số processor, P = speedup.

**⚠️ Hệ quả quan trọng (tr. 278):**
- `fₛ = 0` (song song hết) → speedup = N.
- `fₛ = 1/3` → **dù N vô hạn, speedup không quá 3.**

→ **Phần serial là trần cứng của speedup.** Muốn scale: **giảm phần serial** hoặc **giảm thread chờ nhau**, hoặc **cho nhiều dữ liệu hơn** (giữ phần parallel luôn có việc → giảm tỉ lệ serial). **Scalability (tr. 278) = giảm thời gian / tăng lượng dữ liệu xử lý được khi thêm processor.**

### 4.3 Ẩn latency bằng nhiều thread (tr. 279–280)

Thread thường **block chờ** (I/O, mutex, condvar, sleep). Nếu chỉ có đúng số thread = số core → thread block = **phí CPU** (core ngồi không). Giải: **chạy thêm thread** để tận dụng CPU rảnh (vd virus scanner: 1 thread tìm file I/O-bound + nhiều thread scan). **⚠️ Nhưng quá nhiều → chậm lại (task switching).** Đây là **tối ưu → đo trước-sau.** Cách khác không cần thêm thread: **async I/O**, hoặc thread chờ **tự làm việc mình đang chờ** (như lock-free queue helping ch. 7).

### 4.4 Tăng responsiveness (tr. 280–282)

GUI event-driven có **event loop**. Đơn luồng → task dài làm UI đơ. Giải: **tách task sang thread riêng, GUI thread chỉ xử lý event** (Listing 8.6):
```cpp
std::thread task_thread;
std::atomic<bool> task_cancelled(false);
void process(event_data const& event) {
    switch (event.type) {
    case start_task:
        task_cancelled = false;
        task_thread = std::thread(task);       // task chạy thread riêng
        break;
    case stop_task:
        task_cancelled = true;                 // báo hủy qua atomic flag
        task_thread.join();
        break;
    case task_complete:
        task_thread.join();
        display_results();
        break;
    }
}
```
GUI thread luôn phản hồi event kịp thời (resize/repaint) dù task chạy lâu; giao tiếp qua atomic flag + post event.

### Insight đáng nhớ (Cụm 4)
- **🎯 `std::async` cho parallel algorithm vừa gọn vừa TỰ ĐỘNG exception-safe** — future hủy khi exception propagate → chờ task xong (không leak thread); `get()` ném lại exception worker. Ưu tiên hơn tự quản `std::thread`.
- **🎯 Amdahl's law: phần serial là trần cứng.** `fₛ = 1/3` → speedup tối đa 3 dù N vô hạn. Muốn scale phải giảm phần serial, không phải thêm core.
- **Thêm thread để ẩn latency** (khi thread hay chờ I/O) và **tăng responsiveness** (tách GUI thread) — hai lý do dùng thread ngoài "tăng throughput".

---

## Cụm 5 — Parallel algorithm thực hành: `for_each`, `find`, `partial_sum` (tr. 282–299)

Sách cài ba thuật toán Standard Library song song để minh họa các vấn đề (không phải state-of-the-art — bản tốt hơn ở Intel TBB).

### 5.1 Parallel `for_each` (tr. 283–286)

Đơn giản nhất: gọi hàm user trên mỗi phần tử. Khác bản tuần tự: **không đảm bảo thứ tự**, xử lý concurrent. Chia dữ liệu trước (§8.1.1, biết số phần tử), `hardware_concurrency()` chọn số thread, **khối liền nhau tránh false sharing** (§8.2.3). Dùng `std::packaged_task` + `join_threads` (RAII join mọi thread) cho exception safety.

### 5.2 Parallel `find` (tr. 289–297)

**Khác biệt then chốt (tr. 290):** `find` có thể **kết thúc SỚM** — tìm thấy là dừng, không cần xử lý hết. **⚠️ Phải báo các thread khác dừng** khi một thread tìm thấy → dùng **`std::atomic<bool> done` flag**: mỗi thread kiểm `done` trong vòng lặp, tìm thấy thì set `done = true`. Đây là mẫu "chia việc + cờ dừng chung" (đã thấy ở `when_any` ch. 4). Dùng `std::promise`/`packaged_task` để trả kết quả + exception.

### 5.3 Parallel `partial_sum` (tr. 297–299)

Khó nhất — mỗi phần tử phụ thuộc kết quả trước (`out[i] = in[0]+...+in[i]`). **⚠️ Không chia độc lập được** (dependency). Hai cách: (a) chia khối, mỗi khối tính partial sum cục bộ, rồi cộng dồn "carry" giữa khối; (b) dùng barrier chia thành các bước, mỗi bước cộng phần tử cách `2^k`. **⚠️ Cách (b) có nhiều cache ping-pong** (tr. 299 — dữ liệu chung nhiều) → phải cẩn thận. Minh họa: thuật toán song song đôi khi **làm nhiều phép tính hơn** bản tuần tự nhưng vẫn nhanh hơn nhờ chia cho nhiều core (§8.2.1).

### Insight đáng nhớ (Cụm 5)
- **`for_each` (độc lập hoàn toàn) < `find` (kết thúc sớm, cần cờ dừng) < `partial_sum` (có dependency) — độ khó song song hóa tăng dần.** Dependency giữa phần tử là kẻ thù của song song.
- **Cờ `atomic<bool> done` chung** = mẫu chuẩn để thread báo nhau dừng sớm (find, search).
- **Song song hóa có thể làm nhiều phép tính hơn** nhưng vẫn nhanh nếu chia đủ core — nhưng coi chừng cache ping-pong (partial_sum).

---

## Tóm tắt chương (theo sách, tr. 298–299)

Chương phủ: kỹ thuật chia việc (dữ liệu trước / pipeline); yếu tố hiệu năng low-level (**false sharing, data contention**); pattern truy cập dữ liệu ảnh hưởng hiệu năng; **exception safety** và **scalability**; ví dụ parallel algorithm.

**Câu chuyển sang ch. 9 (tr. 299):** ý tưởng **thread pool** đã xuất hiện vài lần — nhóm thread cấu hình sẵn chạy task được giao. Thiết kế thread pool tốt cần nhiều suy nghĩ → ch. 9.

**🆕 Bảng chốt — các yếu tố hiệu năng & cách xử lý:**
| Yếu tố | Là gì | Cách giảm |
|---|---|---|
| **Cache ping-pong** | Dữ liệu chung chuyền giữa cache các core | Giảm memory chung giữa thread |
| **False sharing** | Dữ liệu **khác** chung cache line | Padding/align (`hardware_destructive_interference_size`) |
| **Data proximity** | Dữ liệu một thread rải rác | Nhét gần nhau (`hardware_constructive_interference_size`) |
| **Oversubscription** | Thread sẵn-chạy > core | Giới hạn worker, thread pool, `std::async` |
| **Serial fraction** (Amdahl) | Phần chỉ một thread làm | Giảm phần serial, thêm dữ liệu |

---

## Góc interview

**Câu 1 (🎯🎯 hay hỏi, giao cả BSP):** Giải thích **false sharing** và **cache ping-pong**. Chúng khác nhau thế nào? Cách phát hiện & khắc phục?

<details><summary>Đáp án</summary>

**Cache ping-pong (tr. 262–264):** khi nhiều core cùng **ghi** một dữ liệu chung, dữ liệu đó bị **chuyền qua lại giữa cache các core** liên tục. Mỗi lần một core ghi, nó phải giành quyền sở hữu cache line (invalidate cache core khác) → core khác stall chờ. **Cực chậm — tương đương hàng trăm lệnh CPU.** Xảy ra với:
- **Atomic counter chung:** `counter.fetch_add(1)` từ nhiều core.
- **Mutex** (kể cả reader-writer!): mọi thread phải sửa chính dữ liệu mutex để lock/unlock → cache line mutex bị chuyền.

**False sharing (tr. 264–265):** **trường hợp đặc biệt** của cache ping-pong khi dữ liệu **KHÔNG thật sự chia sẻ**. Cache xử lý theo **cache line (32/64 byte)**, không theo byte. Nếu mảng `int` mà mỗi thread ghi phần tử **riêng** của mình, nhưng nhiều `int` **chung một cache line** → mỗi lần thread ghi phần tử 0, cache line (chứa cả phần tử 1) phải chuyển sang core đó → thread ghi phần tử 1 phải chờ. **Cache line chia sẻ dù dữ liệu thì không** → "false" sharing.

**Khác nhau:**
| | Cache ping-pong (thật) | False sharing |
|---|---|---|
| Dữ liệu | **Thật sự** chia sẻ (counter, mutex) | **Không** chia sẻ (biến riêng, chung cache line) |
| Sửa | Giảm shared data / dùng ít atomic/lock hơn | **Padding/align** để tách cache line |

**Phát hiện (tr. 274):** thêm **padding lớn** giữa các phần tử; nếu hiệu năng cải thiện → có false sharing.

**Khắc phục:**
- **False sharing:** đảm bảo dữ liệu thread khác truy cập **cách nhau ≥ 1 cache line**. C++17: `std::hardware_destructive_interference_size` (`<new>`) = số byte có thể bị false sharing; align/pad theo đó. Mỗi thread tích lũy vào **biến local trên stack** rồi ghi ra một lần cũng tránh được.
- **Cache ping-pong thật:** giảm memory chung — mỗi thread giữ **bản cục bộ** (thread-local), gộp cuối; giảm số atomic/lock trên đường nóng.

**Điểm cộng — nối với BSP:** đây là biểu hiện của **MESI cache coherence protocol** ở tầng phần cứng; cache line invalidation là chi phí thật. Cũng là **lý do lock-free (ch. 7) đôi khi chậm hơn lock-based** — atomic tạo ping-pong nặng. Luôn **profile**.

</details>

**Câu 2 (🎯 kinh điển):** Phát biểu **Amdahl's law**. Nếu 25% code phải chạy tuần tự, speedup tối đa với vô hạn core là bao nhiêu? Ý nghĩa cho thiết kế?

<details><summary>Đáp án</summary>

**Amdahl's law (tr. 278):** chia chương trình thành phần **serial** (`fₛ`, một thread làm) và **parallel** (chạy được trên mọi core). Speedup với N processor:
```
              1
    P = ───────────────
              1 − fₛ
        fₛ + ────────
                 N
```

**25% serial (`fₛ = 0.25`), N → ∞:** phần `(1−fₛ)/N → 0`, nên `P → 1/fₛ = 1/0.25 = **4**`. → **Dù vô hạn core, speedup tối đa chỉ 4×.**

Các mốc:
- `fₛ = 0` (song song hết) → P = N (lý tưởng).
- `fₛ = 1/3` → P ≤ 3 dù N vô hạn.
- `fₛ = 0.25` → P ≤ 4.

**Ý nghĩa cho thiết kế (tr. 278):**
- **Phần serial là TRẦN CỨNG của speedup** — không thêm core nào phá được. Muốn scale phải **giảm phần serial**, không phải thêm processor.
- Cách giảm: giảm phần chỉ-một-thread-làm; giảm thread **chờ nhau**; hoặc **cho nhiều dữ liệu hơn** (giữ phần parallel luôn có việc → hạ tỉ lệ serial tương đối).
- **Scalability = giảm thời gian / tăng lượng dữ liệu xử lý được khi thêm core.** Xác định cái nào quan trọng trước khi chọn cách chia việc.

**⚠️ Giới hạn của Amdahl (tr. 278):** giả định task chia được vô hạn và mọi thứ CPU-bound — thực tế thread còn chờ I/O, task không chia mịn vô hạn. Là ước lượng thô nhưng cho thấy đúng bản chất: **serial fraction giới hạn tất cả.**

</details>

**Câu 3 (🎯):** Viết một parallel `accumulate`/`reduce` **exception-safe**. Vì sao exception safety khó hơn trong parallel?

<details><summary>Đáp án</summary>

**Vì sao khó hơn (tr. 276):** code tuần tự — exception cứ propagate lên caller. Parallel — thao tác chạy trên **thread khác**; **hàm trên thread mới thoát bằng exception → `std::terminate`, cả app chết** (exception không qua được ranh giới thread/call stack). Và nếu exception ném **giữa lúc spawn thread và join** → các `std::thread` chưa join bị hủy → cũng `terminate`.

**Giải tốt nhất — `std::async` (Listing 8.5), gọn + tự động exception-safe:**
```cpp
template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    unsigned long const length = std::distance(first, last);
    unsigned long const max_chunk_size = 25;
    if (length <= max_chunk_size)
        return std::accumulate(first, last, init);        // đủ nhỏ → tuần tự
    Iterator mid = first;
    std::advance(mid, length / 2);
    std::future<T> first_half =
        std::async(parallel_accumulate<Iterator, T>, first, mid, init);  // nửa đầu: async
    T second_half = parallel_accumulate(mid, last, T());  // nửa sau: đệ quy trực tiếp
    return first_half.get() + second_half;                // gộp (get() ném lại nếu worker ném)
}
```

**Vì sao tự động exception-safe (tr. 277):**
1. Nếu **lời gọi đệ quy trực tiếp ném** → `first_half` (future của async) **bị hủy khi exception propagate** → destructor `future` **chờ task async xong** → **không leak thread đang chạy**.
2. Nếu **task async ném** → exception được **future bắt**, và `first_half.get()` **ném lại** trong thread chính.
3. Thư viện tự dùng hardware thread hợp lý (một số async chạy đồng bộ trong `get()`) → không oversubscription.

**Giải thủ công — `std::packaged_task` + `future` (Listing 8.3):** nếu tự quản `std::thread`, dùng `packaged_task` để **truyền cả kết quả lẫn exception** (`future.get()` ném lại); và **phải catch để join thread còn joinable rồi rethrow** (tránh leak). Phức tạp hơn nhiều `std::async`. (Nếu nhiều thread ném, chỉ một propagate — dùng `std::nested_exception` để bắt hết.)

**Chốt:** ưu tiên `std::async` cho parallel algorithm — exception safety gần như miễn phí. Tự quản `std::thread` chỉ khi cần kiểm soát chi tiết.

</details>

**Câu 4 (🟠):** Kể **ba cách chia việc giữa thread**. Khi nào dùng pipeline thay vì chia dữ liệu?

<details><summary>Đáp án</summary>

**Ba cách (tr. 253–260):**
1. **Chia dữ liệu trước khi xử lý (§8.1.1):** biết số phần tử trước → chia khối liền nhau, mỗi thread một khối, xử lý độc lập, **reduction cuối** gộp kết quả (mô hình MPI/OpenMP). Hợp `for_each`, `accumulate`.
2. **Chia đệ quy (§8.1.2):** khi không chia trước được (quicksort — chỉ biết phần tử thuộc nửa nào sau khi xử lý). Dùng bản chất đệ quy + `std::async` hoặc thread-safe stack. **⚠️ Phải giới hạn số thread** (đệ quy spawn vô hạn → oversubscription).
3. **Chia theo task type (§8.1.3):** thread "specialist", mỗi thread một task khác (separation of concerns). Gồm cả **pipeline** (mỗi thread một stage của chuỗi thao tác).

**Pipeline vs chia dữ liệu (tr. 259–260):**
- **Dùng pipeline khi:**
  - **Dữ liệu chưa biết hết lúc bắt đầu** (đến qua mạng, hoặc phải scan filesystem để tìm file).
  - **Mỗi thao tác tốn giờ** và ta muốn **output đều đặn** thay vì theo lô.
- **Khác biệt performance profile (ví dụ 20 item, 4 core, 4 bước × 3s):**
  - **Chia dữ liệu:** mỗi thread 5 item; ra theo **lô** (4 item mỗi 12s), xong hết sau ~1 phút.
  - **Pipeline:** item đầu mất full 12s (qua 4 stage), nhưng sau khi "mồi" xong → **1 item mỗi 3s đều đặn**. Tổng lâu hơn (chờ mồi) nhưng **đều**.
- **Ví dụ điển hình pipeline:** phát video HD — cần **25 fps đều đặn** (không giật); ứng dụng decode 100 fps rồi khựng 1 giây là vô dụng. Pipeline cho frame đều > throughput cao mà giật.

**Điểm cộng:** pipeline cũng giúp **ẩn latency** — stage I/O-bound (đọc file) chạy song song stage CPU-bound (xử lý), tận dụng CPU rảnh (§8.4.3).

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [cpp-mindset/understanding-the-machine.md](../cpp-mindset/understanding-the-machine.md) — **cache line, MESI cache coherence, SoA vs AoS** — nền phần cứng của false sharing/cache ping-pong (giao với BSP).
- [07-lock-free-structures.md](07-lock-free-structures.md) — vì sao lock-free đôi khi chậm hơn: atomic + cache ping-pong (chương này giải thích chi tiết).
- [04-synchronization.md](04-synchronization.md) — `std::async`, `packaged_task`, `future` (công cụ exception safety ở đây); `when_any` (mẫu cờ dừng như parallel find).
- [02-managing-threads.md](02-managing-threads.md) — `parallel_accumulate` gốc (Listing 2.9), `hardware_concurrency`, oversubscription.
- [10-thinking/system-design.md](../../10-thinking/system-design.md) — thiết kế hệ thống, pipeline, trade-off throughput vs latency.

**Chương tiếp theo:** [Ch. 9 — Advanced thread management →](09-advanced-thread-management.md) (thread pool từ đơn giản tới work-stealing; **interrupting threads**) — hiện thực ý tưởng thread pool đã xuất hiện nhiều lần trong chương này.
