# Ch. 11 — Testing and debugging multithreaded applications (tr. 339–353) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 10 — Parallel algorithms](10-parallel-algorithms.md)** · **⏭ (hết sách — còn phụ lục A–D, tra cứu)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Chương cuối — kỹ năng debug/test đa luồng.** Đặc biệt liên quan vì **debug là điểm yếu cần đầu tư** theo CLAUDE.md của repo. Sách nói thẳng (tr. 339): *"Testing and debugging concurrent code is **hard**."* — không có cách dễ, chỉ có kỹ thuật làm bớt khó.

**Sách nêu mục tiêu (tr. 339):** các loại bug concurrency · định vị bug qua test + code review · thiết kế test đa luồng · test hiệu năng.

---

## Cụm 1 — Các loại bug concurrency (tr. 340–342)

Hai nhóm lớn: **unwanted blocking** và **race conditions**.

### 1.1 Unwanted blocking (tr. 340–341)

Thread block khi chờ gì đó (mutex, condvar, future, I/O). Không mong muốn khi **thread khác lại đang chờ thread bị block này**:
| Loại | Là gì | Triệu chứng |
|---|---|---|
| **Deadlock** (ch. 3) | A chờ B, B chờ A (blocking wait) | App **treo**, UI không phản hồi / task không xong |
| **Livelock** | A chờ B, B chờ A nhưng bằng **vòng lặp active** (spinlock) | Như deadlock nhưng **CPU cao** (thread vẫn chạy, chặn nhau); có thể tự giải nhờ scheduling ngẫu nhiên (trễ lâu) |
| **Block trên I/O/input ngoài** | Thread chờ input có thể không bao giờ đến | Không tiến được; tệ nếu thread khác chờ nó |

### 1.2 Race conditions (tr. 341–342)

**Nguồn gây bug phổ biến nhất** (nhiều deadlock/livelock chỉ lộ ra vì race). Không phải race nào cũng xấu (race lành tính: worker nào lấy task kế không quan trọng). Ba loại race có vấn đề:
- **⚠️ Data race (ch. 5):** truy cập đồng thời không đồng bộ vào một memory location → **UB**. Do dùng sai atomic hoặc quên khóa mutex.
- **⚠️ Broken invariant:** biểu hiện đa dạng — **dangling pointer** (thread khác xóa data), **memory corruption** (đọc giá trị nửa vời), **double-free** (hai thread pop cùng giá trị → cùng delete). Invariant có thể **theo thời gian** (thao tác phải theo thứ tự) chứ không chỉ theo giá trị.
- **⚠️ Lifetime issue:** **thread sống lâu hơn data nó truy cập** → truy cập data đã hủy (storage có thể đã tái dùng cho object khác). Điển hình: thread giữ reference tới biến local hết scope trước khi thread xong. → **Phải bảo đảm `join()` không bị bỏ qua khi có exception** (exception safety cho thread).

**⚠️ Vì sao race chí mạng hơn deadlock (tr. 342):** deadlock/livelock thì app **treo** → attach debugger tìm được thread nào kẹt. Nhưng data race/broken invariant/lifetime → **triệu chứng lộ ở CHỖ KHÁC** (ghi đè bộ nhớ phần khác của hệ, lỗi lộ ra ở code hoàn toàn không liên quan, có thể **rất lâu sau**). Đây là **lời nguyền của shared memory**: bất kỳ thread nào cũng có thể ghi đè data thread khác.

### Insight đáng nhớ (Cụm 1)
- **Hai nhóm bug: blocking (deadlock/livelock — app treo, dễ tìm bằng debugger) vs race (data race/broken invariant/lifetime — lộ ở chỗ khác, rất khó truy).**
- **Race là kẻ giết người thật sự** vì triệu chứng xa nguyên nhân — có thể crash ở code vô can, rất lâu sau khi bug xảy ra.
- **Lifetime issue = thread sống lâu hơn data.** Luôn buộc vòng đời thread với vòng đời data (RAII, `join` không bị skip khi exception).

---

## Cụm 2 — Định vị bug: code review + testing (tr. 342–350)

### 2.1 Code review — 8 câu hỏi vàng (tr. 342–344)

Review kỹ (nhờ người khác càng tốt — họ đọc fresh). **Danh sách câu hỏi khi review code đa luồng (tr. 343):**
1. Dữ liệu nào cần bảo vệ khỏi truy cập đồng thời?
2. Bảo đảm được bảo vệ bằng cách nào?
3. **Thread khác có thể đang ở đâu trong code lúc này?**
4. Thread này đang giữ mutex nào?
5. Thread khác có thể đang giữ mutex nào?
6. Có yêu cầu thứ tự giữa thao tác thread này và thread khác? Ép bằng cách nào?
7. Dữ liệu thread này load còn hợp lệ không? Có thể đã bị thread khác sửa?
8. **⭐ Câu ưa thích của tác giả:** *"nếu giả định thread khác ĐANG sửa dữ liệu này, thì nghĩa là gì và làm sao đảm bảo điều đó không bao giờ xảy ra?"*

**Câu 8 (tr. 343):** giả định có bug ở một dòng rồi **làm thám tử truy nguyên** — để tự thuyết phục không có bug, phải xét mọi corner case và thứ tự. Đặc biệt hữu ích khi data được bảo vệ bởi **nhiều mutex** (như queue head/tail ch. 6).

**Câu 7 (tr. 344):** **⚠️ nếu nhả rồi lấy lại mutex, phải giả định thread khác ĐÃ sửa data.** Dễ mắc khi lock ẩn trong object → đây chính là **race-trong-interface** ch. 6 (`top()`/`pop()` tách đôi → lock nhả giữa hai lời gọi → sửa bằng gộp lại).

### 2.2 Testing — vì sao khó (tr. 344–346)

**⚠️ Test đa luồng khó hơn một bậc (tr. 344):** scheduling **không xác định, khác nhau mỗi lần chạy** → cùng input có thể đúng lúc này, sai lúc khác. **Có race ≠ luôn fail, chỉ là CÓ THỂ fail đôi khi.**

Nguyên tắc:
- **Test đơn vị nhỏ nhất có thể lộ bug** (test queue trực tiếp, không qua cả khối code dùng queue).
- **⚠️ Loại bỏ concurrency để xác nhận bug LÀ do concurrency (tr. 345):** nếu bug xảy ra khi chạy **một thread** → bug thường, không phải concurrency. Nếu bug **biến mất trên single-core nhưng còn trên multicore** → **race condition** (có thể lỗi đồng bộ/memory ordering).
- **Nghĩ nhiều kịch bản** (tr. 345): 1 thread push/pop; 1 push + 1 pop; nhiều push queue rỗng/đầy; nhiều pop rỗng/đầy; nhiều push đồng thời nhiều pop...
- **Yếu tố môi trường (tr. 346):** "nhiều thread" nghĩa là mấy (3, 4, 1024?); đủ core cho mỗi thread một core không; test trên kiến trúc CPU nào; đảm bảo scheduling cho phần "while" ra sao.

### 2.3 Designing for testability (tr. 346–347)

Code dễ test khi: trách nhiệm rõ, hàm ngắn, test kiểm soát được môi trường, code thao tác gần nhau, **nghĩ cách test trước khi viết**. **⭐ Cách tốt nhất: loại bỏ concurrency (tr. 346)** — tách code thành (a) phần **giao tiếp giữa thread** (khó test) và (b) phần **thao tác data trong một thread** (test bằng kỹ thuật single-thread thường). Ví dụ state machine đa luồng: test **state logic từng thread** bằng single-thread, test **routing/delivery** riêng. **⚠️ Cảnh giác library call dùng internal state** → thành shared khi nhiều thread dùng (không lộ rõ là shared data).

### 2.4 Ba kỹ thuật test đa luồng (tr. 347–350)

| Kỹ thuật | Cách | Ưu | Nhược |
|---|---|---|---|
| **Brute-force / stress test** | Chạy nhiều lần (tỉ lần), nhiều thread | Đơn giản, nhiều lần pass → tự tin | **⚠️ False confidence** — nếu test không tạo được tình huống lỗi thì chạy mãi vẫn pass |
| **Combination simulation test** | Phần mềm ghi lại sequence access/lock/atomic, **lặp lại mọi tổ hợp** theo memory model | **Đảm bảo tìm hết bug** | **⚠️ Bùng nổ tổ hợp** (exponential) — chỉ cho test đơn vị nhỏ; cần phần mềm chuyên |
| **Special library** | Bản mutex/lock/condvar đặc biệt phát hiện lỗi khi chạy | Kiểm mutex đúng đã khóa chưa; ghi thứ tự lock (phát hiện deadlock tiềm ẩn dù test không deadlock); điều khiển thread nào được lock | Không exhaustive như combination |

**⚠️ False confidence — bẫy kinh điển của brute-force (tr. 348):** test app đa luồng trên **single-processor** → mọi thread serialize → **nhiều race + cache ping-pong biến mất**. Và **kiến trúc CPU khác nhau đồng bộ khác nhau:** trên x86/x86-64, atomic load **`relaxed` giống hệt `seq_cst`** → code dùng relaxed ordering **chạy ổn trên x86 nhưng fail trên SPARC/ARM** (memory ordering mịn hơn). → **Test trên các kiến trúc đại diện của hệ đích.**

🆕 **Công cụ thực tế (ngoài sách):** **ThreadSanitizer (TSan)** (`-fsanitize=thread`, GCC/Clang) là "special library" của thực tế — phát hiện data race lúc runtime; **Helgrind/DRD** (Valgrind) tương tự. Đây là công cụ đầu tiên nên dùng khi nghi race.

### 2.5 Cấu trúc test code — dùng promise để đồng bộ start (Listing 11.1, tr. 350–352)

Để ép các thread **chạy phần cần-song-song đúng lúc**, dùng `std::promise` báo sẵn sàng + `shared_future` làm cờ "go" chung:
```cpp
void test_concurrent_push_and_pop_on_empty_queue() {
    threadsafe_queue<int> q;
    std::promise<void> go, push_ready, pop_ready;
    std::shared_future<void> ready(go.get_future());   // cờ "go" chung
    std::future<void> push_done;  std::future<int> pop_done;
    try {
        push_done = std::async(std::launch::async, [&q, ready, &push_ready]() {
            push_ready.set_value();                    // báo thread này sẵn sàng
            ready.wait();                              // chờ tín hiệu go
            q.push(42);                                // ← phần cần chạy đồng thời
        });
        pop_done = std::async(std::launch::async, [&q, ready, &pop_ready]() {
            pop_ready.set_value();
            ready.wait();
            return q.pop();
        });
        push_ready.get_future().wait();                // main chờ MỌI thread sẵn sàng
        pop_ready.get_future().wait();
        go.set_value();                                // ← bắn tín hiệu go: cả hai chạy CÙNG lúc
        // ... assert: pop trả 42, queue rỗng
    } catch (...) { go.set_value(); throw; }           // đảm bảo không thread nào kẹt chờ go
}
```
Mỗi thread: setup → set promise "sẵn sàng" → `ready.wait()` (chờ go). Main chờ mọi promise sẵn sàng → `go.set_value()` → tất cả chạy phần đồng thời **cùng lúc** (tối đa hóa khả năng lộ race). **⚠️ Catch để `go.set_value()` tránh thread kẹt chờ go mãi khi có exception.**

### Insight đáng nhớ (Cụm 2)
- **⭐ Câu review vàng: "giả sử thread khác đang sửa data này — nghĩa là gì?"** Làm thám tử truy mọi thứ tự.
- **Loại concurrency để phân loại bug:** chạy 1 thread còn lỗi = bug thường; biến mất single-core nhưng còn multicore = race.
- **⚠️ Brute-force cho false confidence** — nhất là test trên single-core (serialize hết, race biến mất) hoặc x86 (relaxed = seq_cst). **Test trên kiến trúc đại diện + dùng TSan.**
- **Promise + shared_future "go"** = mẫu chuẩn ép nhiều thread chạy phần đồng thời cùng lúc để lộ race.

---

## Cụm 3 — Test hiệu năng & scalability (tr. 352–353)

Dùng concurrency để tăng hiệu năng → **phải test xác nhận thực sự nhanh hơn** (như mọi tối ưu). Vấn đề chính: **scalability** — muốn code chạy ~24× nhanh trên 24-core, không phải nhanh 2× trên dual-core rồi **chậm hơn trên 24-core**. (Amdahl §8.4.2: phần serial giới hạn — biết trước hi vọng 24× hay tối đa 3×.)

**⚠️ Contention tăng theo số processor (tr. 353):** cái scale tốt khi ít processor có thể **tệ khi nhiều processor** vì contention bùng nổ. → **Test trên nhiều cấu hình nhất có thể** (ít nhất: single-processor + máy nhiều core nhất có) để vẽ **scalability graph**.

### Insight đáng nhớ (Cụm 3)
- **Test scalability, không chỉ "nhanh hơn 1 lần":** đo trên nhiều số core để vẽ đồ thị — cái scale ở 4 core có thể sụp ở 24 core do contention.
- **Biết trần Amdahl trước khi test:** phần serial quyết định hi vọng 24× hay 3×.

---

## Tóm tắt chương (theo sách, tr. 353)

Các loại bug concurrency (deadlock/livelock → data race và race có vấn đề khác); kỹ thuật định vị bug (câu hỏi khi code review, guideline viết code test được, cấu trúc test đa luồng); công cụ hỗ trợ test.

**🆕 Bảng chốt — định vị bug concurrency:**
| Bước | Kỹ thuật |
|---|---|
| Hiểu bug | Blocking (deadlock/livelock) vs race (data race/broken invariant/lifetime) |
| Review | 8 câu hỏi, đặc biệt "giả sử thread khác đang sửa data này" |
| Phân loại | Chạy 1 thread → bug thường; biến mất single-core → race |
| Test | Brute-force (⚠️ false confidence) / combination sim / special library (**TSan**) |
| Cấu trúc test | Promise + shared_future "go" chạy đồng thời cùng lúc |
| Hiệu năng | Đo scalability trên nhiều số core (Amdahl) |

---

## Góc interview

**Câu 1 (🎯 hay hỏi, giao debug — điểm yếu cần đầu tư):** Kể các loại bug concurrency. Vì sao **data race khó debug hơn deadlock**? Bạn định vị chúng thế nào?

<details><summary>Đáp án</summary>

**Hai nhóm bug (tr. 340–342):**
- **Unwanted blocking:** **deadlock** (A chờ B, B chờ A, blocking); **livelock** (như deadlock nhưng vòng lặp active/spinlock → CPU cao); block trên I/O không bao giờ đến.
- **Race conditions:** **data race** (truy cập đồng thời không đồng bộ → UB); **broken invariant** (dangling pointer, memory corruption, double-free); **lifetime issue** (thread sống lâu hơn data).

**Vì sao data race khó hơn deadlock (tr. 342):**
- **Deadlock/livelock:** app **treo** → attach debugger, xem thread nào đang chờ, tranh mutex nào → **định vị được ngay**.
- **Data race/broken invariant/lifetime:** **triệu chứng lộ ở CHỖ KHÁC** — ghi đè bộ nhớ phần hệ khác, crash/output sai ở code **hoàn toàn không liên quan**, có thể **rất lâu sau**. Nguyên nhân xa triệu chứng → cực khó truy. Đây là "lời nguyền của shared memory".

**Định vị (tr. 342–350):**
1. **Code review** — 8 câu hỏi (tr. 343), đặc biệt *"giả sử thread khác đang sửa data này — nghĩa là gì, làm sao đảm bảo không xảy ra?"* → làm thám tử truy mọi thứ tự.
2. **Phân loại bằng cách loại concurrency:** chạy **1 thread** còn lỗi → bug thường; **biến mất single-core nhưng còn multicore** → **race** (lỗi đồng bộ/memory ordering).
3. **Test:** brute-force (stress, ⚠️ false confidence), combination simulation (exhaustive, exponential), **special library** — thực tế là **ThreadSanitizer** (`-fsanitize=thread`) / Helgrind — phát hiện data race lúc runtime.
4. **Cấu trúc test:** promise + shared_future "go" để nhiều thread chạy phần đồng thời **cùng lúc** (lộ race).

**Điểm cộng — công cụ thực tế:**
- **ThreadSanitizer (TSan):** compile với `-fsanitize=thread`, phát hiện data race + lock-order-inversion (deadlock tiềm ẩn) khi chạy test.
- **Helgrind/DRD (Valgrind):** tương tự, không cần recompile nhưng chậm hơn.
- **GDB:** attach vào process treo (`thread apply all bt`) để xem deadlock — thread nào giữ/chờ mutex nào.
- **Chạy dưới debugger có thể làm race biến mất** (đổi timing) → kết hợp TSan + log + review, không chỉ dựa GDB.

</details>

**Câu 2 (🟠):** Làm sao **test** một thread-safe queue? Các cạm bẫy khi test code đa luồng?

<details><summary>Đáp án</summary>

**Vì sao khó (tr. 344):** scheduling **không xác định** → cùng input có thể đúng lúc này sai lúc khác. **Có race ≠ luôn fail, chỉ CÓ THỂ fail đôi khi.**

**Các kịch bản test (tr. 345):** 1 thread push/pop cơ bản; 1 push + 1 pop; nhiều push queue rỗng/đầy; nhiều pop rỗng/đầy/vơi; nhiều push đồng thời nhiều pop; các tổ hợp "while" (nhiều push **trong khi** một/nhiều pop).

**Cấu trúc test để lộ race (Listing 11.1):** dùng `std::promise` + `shared_future` "go" — mỗi thread setup → báo sẵn sàng → chờ tín hiệu go; main chờ mọi thread sẵn sàng → bắn go → **tất cả chạy phần đồng thời cùng lúc** (tối đa hóa khả năng lộ race). ⚠️ Catch để set go tránh thread kẹt.

**Ba kỹ thuật (tr. 347–350):** brute-force (chạy tỉ lần), combination simulation (exhaustive), special library / **TSan**.

**⚠️ Các cạm bẫy:**
1. **False confidence của brute-force (tr. 348):** test trên **single-processor** → serialize hết → race + cache ping-pong **biến mất**. Nhiều lần pass ≠ đúng.
2. **⚠️ Khác biệt kiến trúc CPU:** trên x86/x86-64, atomic load **`relaxed` giống hệt `seq_cst`** → code relaxed **chạy ổn x86 nhưng fail SPARC/ARM** (memory ordering mịn hơn). → Test trên **kiến trúc đại diện** của hệ đích.
3. **Test đơn vị nhỏ nhất** (queue trực tiếp, không qua cả khối code) để cô lập bug.
4. **Loại concurrency để phân loại:** chạy 1 thread còn lỗi = bug thường, không phải concurrency.
5. **Design for testability:** tách phần giao-tiếp-giữa-thread (khó test) khỏi phần thao-tác-data-một-thread (test single-thread thường).

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [03-sharing-data.md](03-sharing-data.md) — race-trong-interface (`top()`/`pop()`), deadlock, data race (loại bug ở đây).
- [05-memory-model.md](05-memory-model.md) — data race chính xác là gì, vì sao x86 relaxed = seq_cst (bẫy false confidence).
- [06-lock-based-structures.md](06-lock-based-structures.md) — thread-safe queue (đối tượng test ở Listing 11.1); bẫy nhả-rồi-lấy-lại mutex (câu review 7).
- [09-debugging/](../../09-debugging/) — **kỹ năng debug của repo** (mindset, gdb, tools như valgrind/TSan/perf, memory-bugs/sanitizers) — nối trực tiếp với chương này.
- [09-debugging/kernel-debugging.md](../../09-debugging/kernel-debugging.md) — debug concurrency ở tầng kernel (ftrace, lockdep — "special library" của kernel).

**Hết sách (chương nội dung cuối).** Phụ lục A (C++11 features: rvalue ref, move, lambda, `thread_local`...), B (so sánh thư viện concurrency), C (message-passing + ATM), D (**Thread Library reference** tr. 401–550 — tra cứu API) — không summary, dùng tra cứu khi cần. Xem [README](README.md) để ôn lại toàn bộ 11 chương.
