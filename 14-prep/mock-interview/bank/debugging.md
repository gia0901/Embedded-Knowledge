# DBG — Debugging & Tools

> Domain `DBG`. Nhiều câu dạng tình huống ("bạn sẽ làm gì nếu…"). Track dùng: `debugging`, `bsp`, `cpp-system`.

---

#### DBG-001 · 🟢 · concept · [→ gdb](../../../09-debugging/gdb.md)
**Vì sao build với -g và nên -O0 khi debug?**
<details><summary>Đáp án</summary>

`-g` nhúng debug symbol (tên biến, kiểu, ánh xạ địa chỉ↔dòng) để gdb hiển thị source và in biến. `-O0` tắt tối ưu vì `-O2/-O3` inline/reorder/loại biến → step nhảy loạn, biến "optimized out". Khi buộc debug bản tối ưu, dùng `-Og`.
</details>

#### DBG-002 · 🟢 · concept · [→ gdb](../../../09-debugging/gdb.md)
**step và next trong gdb khác nhau gì?**
<details><summary>Đáp án</summary>

Cả hai chạy một dòng. `step` đi **vào trong** hàm được gọi; `next` **bước qua** (chạy hết hàm đó rồi dừng ở dòng kế). `finish` chạy tới khi hàm hiện tại return.
</details>

#### DBG-003 · 🟢 · concept · [→ gdb](../../../09-debugging/gdb.md)
**Core dump là gì và dùng để làm gì? Phân tích thế nào?**
<details><summary>Đáp án</summary>

Ảnh chụp bộ nhớ process lúc crash (stack, heap, register), lưu ra file. Nạp vào gdb (`gdb ./app core`) để phân tích post-mortem (`bt`, `print`) mà không cần tái hiện — cực giá trị cho bug khó tái hiện hoặc crash ở field. Cần binary có symbol (`-g`) và `ulimit -c unlimited`.
</details>

#### DBG-004 · 🟡 · concept · [→ gdb](../../../09-debugging/gdb.md), [memory-bugs](../../../09-debugging/memory-bugs.md)
**Chương trình segfault. Bạn điều tra thế nào — và làm gì khi backtrace toàn `??`?**
<details><summary>Đáp án</summary>

**Trình tự:**
1. **Lấy được crash đã.** Bật core dump (giới hạn kích thước thường đang là 0), hoặc chạy thẳng dưới debugger nếu tái hiện được.
2. **`backtrace` — nhưng đọc từ dưới lên.** Khung trên cùng là *nơi nổ*, thường **không phải nơi sai**. Đi ngược xuống tới khung code **của bạn** gần nhất.
3. **Xem con trỏ nào là `NULL` hay rác.** In các biến ở khung đó; giá trị kiểu `0x0`, `0xdeadbeef`, hoặc địa chỉ lệch lung tung là dấu hiệu.
4. **Hỏi "con trỏ này đến từ đâu"** — đây mới là bước tìm ra bug. Nó là kết quả trả về không được kiểm? Là thành viên của đối tượng đã bị huỷ? Là phần tử của `vector` vừa reallocate?

**⚠️ Backtrace toàn `??` — ba nguyên nhân:**

| Dấu hiệu | Nguyên nhân | Cách chữa |
|---|---|---|
| Toàn bộ `??` | **Thiếu symbol** — binary đã strip, hoặc không đúng bản build | Giữ file symbol **theo từng version** ở CI; nạp đúng bản khớp |
| Vài khung `??` xen kẽ | Thư viện bên thứ ba không có symbol | Cài gói debug tương ứng |
| Backtrace **vô lý** (hàm không thể gọi nhau) | **Stack đã bị ghi đè** — chính bug tràn bộ đệm | Đừng tin backtrace nữa; chuyển sang ASan |

⇒ Ô cuối là chỗ hay mất thời gian nhất: người ta cố đọc một backtrace **đã hỏng**. Backtrace vô lý **chính là bằng chứng** của tràn stack.

**Khi crash không tái hiện được:** đừng ngồi đoán — chuyển sang **phát hiện chủ động** bằng ASan/UBSan, vì segfault chỉ là *hậu quả muộn* của một vi phạm đã xảy ra trước đó ([DBG-011](debugging.md), [DBG-024](debugging.md)).

**Chốt:** *"Đọc backtrace từ dưới lên tới code của mình, rồi hỏi con trỏ hỏng đến từ đâu. Backtrace vô lý không phải là công cụ hỏng — nó là bằng chứng stack đã bị ghi đè."*
</details>

#### DBG-005 · 🟡 · concept · [→ tools](../../../09-debugging/tools.md)
**Chương trình báo lỗi mơ hồ "không khởi động được". strace giúp gì?**
<details><summary>Đáp án</summary>

`strace` ghi mọi syscall + kết quả. Thường thấy ngay `openat("/path/config", ...) = -1 ENOENT` (sai đường dẫn) hoặc `EACCES` (quyền), hoặc thiếu thư viện. Lớp bug "thiếu file/lib/quyền/mạng" giải quyết rất nhanh bằng strace vì nó cho thấy chương trình **thực sự** tìm gì.
</details>

#### DBG-006 · 🟡 · concept · [→ tools](../../../09-debugging/tools.md)
**Chương trình chạy chậm. Tìm nguyên nhân thế nào — và vì sao "đọc code rồi đoán chỗ chậm" gần như luôn sai?**
<details><summary>Đáp án</summary>

**Bước 0 — phân loại trước khi đo.** *"Chậm"* có ít nhất bốn dạng, mỗi dạng điều tra theo hướng khác hẳn:

| Dạng | Dấu hiệu | Nghi gì |
|---|---|---|
| **CPU-bound** | CPU ~100%, một lõi hoặc nhiều lõi | Thuật toán, vòng lặp nóng |
| **I/O-bound** | CPU **thấp**, tiến trình hay ở trạng thái chờ | Đĩa, mạng, syscall quá nhiều |
| **Chờ khoá** | CPU thấp, nhiều thread mà không cái nào chạy | Tranh chấp mutex, khoá giữ quá lâu |
| **Chờ bộ nhớ** | CPU cao nhưng "không làm gì" | Cache miss, **major page fault** ([OS-010](os.md)) |

⇒ Nhìn **CPU cao hay thấp** trước tiên là đã chia đôi được không gian nghi ngờ.

**⚠️ Vì sao đọc code rồi đoán luôn sai:** nút thắt gần như **luôn nằm ở chỗ bạn không ngờ** — một hàm trông vô hại gọi 10 triệu lần, một `std::string` bị copy trong vòng lặp, một `open()` lặp lại mỗi lần lặp. Trực giác của lập trình viên về hiệu năng nổi tiếng là kém, vì nó dựa trên *độ phức tạp của code khi đọc*, không phải *số lần thực thi*.

**Trình tự đo:**
1. **Đo tổng thể trước** — bao nhiêu thời gian ở user, bao nhiêu ở kernel, bao nhiêu là chờ. Tỉ lệ này chỉ ngay ra dạng nào ở bảng trên.
2. **Profiler lấy mẫu** để tìm hàm nóng (chi phí thấp, chạy được cả trên production).
3. **Đo lại sau khi sửa** — cùng tải, cùng điều kiện.

**Ba bẫy:**
1. ⭐ **Tối ưu hàm chiếm 2% thời gian.** Có nhanh gấp đôi cũng chỉ được 1%. **Luôn tấn công chỗ chiếm phần lớn thời gian trước** — và biết khi nào dừng.
2. **Đo trên máy dev rồi kết luận cho thiết bị.** x86 khác ARM về cache, mô hình bộ nhớ, tốc độ lưu trữ. Ca kinh điển: máy dev không bao giờ có major page fault, thiết bị thì có ([OS-010](os.md)).
3. **Đo trung bình mà bỏ qua đuôi.** Người dùng cảm nhận **p99**, không cảm nhận trung bình. Với hệ realtime thì chỉ **giá trị xấu nhất** mới có nghĩa.

**Chốt:** *"Phân loại trước (CPU cao hay thấp?) rồi mới đo — và luôn đo thay vì đoán, vì nút thắt gần như luôn ở chỗ không ai ngờ. Tối ưu chỗ chiếm 2% thì có nhanh vô hạn cũng vô ích."*
</details>

#### DBG-007 · 🟡 · concept · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Memory leak: phát hiện thế nào?**
<details><summary>Đáp án</summary>

AddressSanitizer/LeakSanitizer (`-fsanitize=address`) hoặc `valgrind --leak-check=full` — báo nơi cấp phát mà không giải phóng kèm stack. Theo dõi RSS (`/proc/<PID>/status`) qua thời gian để xác nhận rò rỉ. Phòng ngừa: RAII/smart pointer.
</details>

#### DBG-008 · 🟠 · concept · ⭐ · 🏗️ · [→ mindset](../../../09-debugging/mindset.md)
**Ticket: *"Máy quét thỉnh thoảng trả sai mã, khoảng 1 lần / 2000 lần quét. Khách ở Đức, ta ở VN."* Bạn không có thiết bị đó trong tay. Kể tôi nghe bạn làm gì — theo thứ tự, và **vì sao thứ tự đó**.**
<details><summary>Đáp án</summary>

**Cơ chế của việc debug — mọi bước chỉ phục vụ MỘT mục tiêu: chia đôi không gian nghi ngờ.**

Debug là **tìm kiếm nhị phân trên tập nguyên nhân**. Mỗi thí nghiệm phải **loại bỏ được một nửa**; thí nghiệm không loại được gì là thí nghiệm lãng phí. Cả quy trình dưới đây chỉ là cách làm việc chia đôi đó cho có kỷ luật.

**Sáu bước — và vì sao đúng thứ tự này:**

| # | Bước | **Vì sao phải ở đây** |
|---|---|---|
| 1 | **Tái hiện** — và làm cho nó **rẻ** | Chi phí một lần thử **quyết định bạn kiểm được bao nhiêu giả thuyết**. Tái hiện 1/2000 lần × 5 phút/lần = mỗi thí nghiệm mất một tuần ⇒ **việc đầu tiên không phải tìm bug, mà là rút ngắn vòng lặp thử** |
| 2 | **Thu hẹp** theo 3 trục | Trục **thời gian** (bản nào bắt đầu hỏng — `git bisect`) · trục **không gian** (tầng nào: cảm biến → giải mã → truyền → hiển thị) · trục **dữ liệu** (rút gọn input tới mức tối thiểu vẫn lỗi) |
| 3 | **Giả thuyết PHẢI bác bỏ được** | *"Chắc do timing"* không kiểm chứng được ⇒ vô dụng. Giả thuyết tốt **tiên đoán một thứ bạn CHƯA quan sát**: *"nếu đúng là mã vắt qua 2 gói TCP, thì lỗi phải tăng khi tôi ép MTU nhỏ"* |
| 4 | **Đổi MỘT biến mỗi lần** | Đổi hai thứ rồi hết lỗi ⇒ **không biết cái nào chữa** ⇒ mất luôn thông tin vừa mua bằng thời gian |
| 5 | **Sửa nguyên nhân gốc** | Hỏi *"vì sao"* tới khi chạm thứ **sửa được**. Mã sai → vì buffer ghi đè → vì độ dài lấy từ gói → **vì không kiểm độ dài** ⇒ chỗ này mới là chỗ sửa |
| 6 | **Kiểm chứng + test hồi quy** | Bug 1/2000 mà "hết thấy" **không chứng minh được gì**. Phải chỉ ra **cơ chế** đã bị chặn, rồi viết test ép đúng điều kiện đó |

**Kỷ luật quan trọng nhất, hay bị bỏ — GHI LẠI CÁI ĐÃ LOẠI TRỪ.** Không có sổ này thì sau 2 ngày bạn kiểm lại đúng những thứ đã kiểm, và tệ hơn: **quên mất mình từng loại nó dựa trên giả định nào**. Khi bí, đọc lại sổ và hỏi *"giả định nào ở đây chưa từng được kiểm?"* — bug hầu như luôn nấp trong một giả định chưa kiểm.

**Áp vào chính ticket này — bốn câu hỏi trước khi động vào code:**
1. **"Sai" là sai thế nào?** — sai vài ký tự, hay trả mã của lần quét trước, hay mã rác? *(Mã lần trước ⇒ nghi buffer/state; vài ký tự ⇒ nghi tầng vật lý/giải mã.)*
2. **Có tương quan với gì không?** — loại mã vạch, ánh sáng, nhiệt độ, thời điểm sau khi bật máy, một site cụ thể?
3. **Bản firmware nào bắt đầu có?** — có mốc là có `git bisect`, tiết kiệm cả tuần.
4. **1/2000 là trên một máy hay nhiều máy?** — một máy ⇒ nghi phần cứng/lô linh kiện; nhiều máy ⇒ nghi phần mềm.

**Bẫy:**
1. **Nhảy vào sửa khi chưa tái hiện được** — sửa xong không có cách nào biết đã hết; đây là nguồn gốc của *"đã sửa rồi mà khách vẫn báo"*.
2. **Tin trực giác hơn dữ liệu** — *"chỗ này chắc chắn không thể sai"* chính là chỗ nó sai.
3. **Sửa triệu chứng**: thêm retry, thêm `sleep`, kiểm tra lại kết quả rồi bỏ qua giá trị lạ. Bug vẫn còn, chỉ hiếm hơn — **và lần sau khó lần hơn gấp bội**.
4. **Không hỏi khách đủ** trước khi tốn hai tuần đoán. Câu (2) ở trên nhiều khi giải quyết cả ca.

**Chốt:** *"Debug là tìm kiếm nhị phân trên tập nguyên nhân. Việc đầu tiên không phải tìm bug mà là làm cho vòng lặp thử đủ rẻ — sau đó mỗi thí nghiệm phải loại được một nửa, và phải ghi lại cái đã loại."*

> 🎤 Viết lại 2026-08-13 (nợ chất lượng bank). Bản cũ chỉ liệt kê 6 bước không nói **vì sao thứ tự đó** — thuộc được nhưng không dùng được.
</details>

#### DBG-009 · 🟠 · concept · [→ gdb](../../../09-debugging/gdb.md), [memory-bugs](../../../09-debugging/memory-bugs.md)
**Một biến "bỗng nhiên" có giá trị sai mà không rõ ai sửa. Làm sao tìm?**
<details><summary>Đáp án</summary>

**Công cụ đúng: watchpoint (data breakpoint).** Breakpoint thường dừng theo *vị trí code* — vô dụng khi bạn **không biết** code nào ghi. Watchpoint dừng theo *dữ liệu*: CPU tự báo khi vùng nhớ đó bị chạm.

```gdb
(gdb) watch myVar              # dừng khi bị GHI (thay đổi giá trị)
(gdb) rwatch myVar             # dừng khi bị ĐỌC
(gdb) watch *(int*)0x601234    # theo ĐỊA CHỈ — dùng khi biến đã ra khỏi scope
(gdb) bt                       # ← khi dừng: ai vừa ghi
```

**Quy trình:** đặt watchpoint → chạy → gdb dừng **ngay tại lệnh ghi** → `bt` chỉ thẳng thủ phạm. Thường thủ phạm không phải code đụng tới biến đó, mà là **buffer overflow của biến lân cận** hoặc con trỏ hỏng trỏ trúng.

**Chọn công cụ theo triệu chứng:**

| Tình huống | Dùng |
|---|---|
| Biết biến nào hỏng, không biết ai ghi | **gdb watchpoint** |
| Nghi overflow/UAF rộng hơn, muốn chỉ đúng dòng | **ASan** (`-fsanitize=address`) |
| Nhiều thread, giá trị sai không tất định | **TSan** (`-fsanitize=thread`) — nghi data race, không phải corruption |
| Biến sai chỉ ở bản `-O2`, `-O0` chạy đúng | Nghi **UB** → **UBSan**; hoặc thiếu `volatile`/race |

**Bẫy:** (1) **hardware watchpoint có giới hạn** (thường 4 trên x86/ARM) — vượt quá, gdb rơi về software watchpoint chạy **chậm hàng trăm lần**; (2) `watch myVar` theo *scope*: biến local ra khỏi scope thì watchpoint tự xoá → theo **địa chỉ** nếu cần sống lâu hơn; (3) trên biến bị ghi rất thường xuyên thì watchpoint không thực dụng — chuyển sang ASan; (4) nếu giá trị sai **không tất định** giữa các lần chạy, đó là mùi **race**, đừng đuổi theo watchpoint.

**Chốt:** *"Không biết ai ghi → đừng đặt breakpoint theo code, đặt watchpoint theo dữ liệu rồi `bt`."*
</details>

#### DBG-010 · 🟠 · concept · [→ gdb](../../../09-debugging/gdb.md)
**Chương trình bị treo (hang). Điều tra thế nào?**
<details><summary>Đáp án</summary>

**Lợi thế của bug treo: nó *đứng yên*.** Không cần tái hiện, không cần log — trạng thái lỗi đang nằm sẵn đó, cứ attach vào mà đọc. **Không khởi động lại process** (làm vậy là mất hiện trường).

```bash
top -H -p <PID>          # bước 0: CPU 0% hay 100%? -> tách hai loại treo ngay
gdb -p <PID>
(gdb) thread apply all bt   # ⭐ lệnh quan trọng nhất: backtrace MỌI thread
(gdb) info threads
pstack <PID>             # cách nhanh, không cần gdb tương tác
strace -p <PID>           # đang kẹt ở syscall nào
cat /proc/<PID>/wchan     # kernel đang chờ gì
```

**Đọc kết quả — CPU% chia đôi chẩn đoán ngay:**

| Triệu chứng | Backtrace cho thấy | Kết luận |
|---|---|---|
| **CPU ~0%** | Nhiều thread kẹt ở `pthread_mutex_lock`/`futex`, mỗi thread chờ lock thread kia giữ | **Deadlock** → truy thứ tự khoá, áp lock ordering ([OS-003](os.md)) |
| **CPU ~0%** | Một thread kẹt ở `read`/`recv`/`connect`/`poll` | **Chờ I/O** — peer không trả lời, thiếu timeout |
| **CPU ~0%** | Kẹt ở `cv.wait` | **Lost wakeup** — notify bay trước khi wait ([OS-012](os.md)) |
| **CPU ~100%** | Backtrace không đổi giữa hai lần lấy, kẹt trong một vòng lặp | **Vòng lặp vô tận** / spin không thoát |
| **CPU ~100%** | Backtrace **đổi** liên tục giữa vài trạng thái | **Livelock** — có tiến triển giả, không hoàn thành |

**Mẹo phân biệt treo thật với chậm:** lấy backtrace **hai lần cách nhau vài giây**. Giống hệt nhau → thật sự kẹt. Khác nhau → nó đang chạy, chỉ là chậm → chuyển sang `perf` chứ không phải gdb.

**Bẫy:** (1) chỉ chạy `bt` (một thread) thay vì `thread apply all bt` — deadlock **luôn** cần nhìn nhiều thread mới thấy vòng chờ; (2) binary **strip** hết symbol → backtrace toàn `??`, cần giữ file có symbol khớp bản build; (3) treo do **deadlock với chính ISR/signal handler** hoặc chờ lock trong kernel → xem `/proc/<PID>/stack`, `dmesg` (hung task detector); (4) trong kernel/driver: bật **LOCKDEP** (`CONFIG_PROVE_LOCKING`) để nó cảnh báo thứ tự khoá sai **trước khi** deadlock thật xảy ra.

**Chốt:** *"Treo là món quà — hiện trường còn nguyên. `top -H` để biết 0% hay 100%, rồi `thread apply all bt` để xem ai chờ ai."*
</details>

#### DBG-011 · 🟠 · concept · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**ASan và Valgrind khác nhau? Khi nào dùng cái nào?**
<details><summary>Đáp án</summary>

**Khác nhau ở *cách* kiểm tra, mọi đánh đổi còn lại là hệ quả:**
- **ASan** — **compile-time instrumentation**: compiler **chèn lệnh kiểm tra** vào binary lúc build (`-fsanitize=address -g`), cộng redzone quanh mỗi allocation. → Phải **build lại**, nhưng chạy gần tốc độ thật.
- **Valgrind** — **mô phỏng CPU**: chạy binary trên một máy ảo, kiểm tra **từng lệnh máy** và từng bit "đã khởi tạo chưa". → **Không** cần build lại, nhưng chậm khủng khiếp.

| | ASan | Valgrind (memcheck) |
|---|---|---|
| Cần build lại | ✅ có | ❌ không — chạy binary có sẵn |
| Tốc độ | ~**2x** chậm hơn | ~**10–50x** |
| Stack / global overflow | ✅ bắt tốt | ⚠️ yếu (chỉ mạnh với heap) |
| **Uninitialized read** | ❌ (cần MSan) | ✅ điểm mạnh nhất — `--track-origins=yes` truy nguồn |
| Thời điểm báo | **Ngay khi xảy ra**, in cả nơi alloc & free | Ngay khi xảy ra |

```bash
g++ -fsanitize=address -g -O1 main.cpp -o app && ./app     # dev/CI mặc định
valgrind --leak-check=full --track-origins=yes ./app        # khi chỉ có binary
```

**Anh em cùng họ:** **UBSan** (`-fsanitize=undefined`) bắt UB số học/alignment/null; **TSan** (`-fsanitize=thread`) bắt **data race**. ⚠️ **ASan và TSan xung khắc** — không bật chung, chạy lần lượt.

**Bẫy:** (1) coi hai cái là thay thế nhau — chúng bắt **tập lỗi khác nhau**, uninitialized read là vùng ASan **không** thấy; (2) chạy Valgrind trong CI trên toàn test suite → pipeline chết vì chậm, nên dùng ASan cho CI và Valgrind cho ca cụ thể; (3) **embedded**: cả hai thường **không chạy được trên target** (thiếu RAM/OS) → build cùng code chạy sanitizer **trên host**, target thì dựa vào review + thiết kế phòng ngừa.

**Chốt:** *"ASan = build lại, nhanh, mặc định cho dev/CI. Valgrind = không build lại, chậm, dùng khi chỉ có binary hoặc cần soi uninitialized read."*
</details>

#### DBG-012 · 🔴 · concept · ⭐ · [→ mindset](../../../09-debugging/mindset.md), [memory-bugs](../../../09-debugging/memory-bugs.md)
**Bạn thêm một dòng `printf` vào đúng chỗ nghi ngờ để xem giá trị biến — và bug BIẾN MẤT. Bỏ dòng đó ra thì bug quay lại. Bạn kết luận gì, và làm gì tiếp?**
<details><summary>Đáp án</summary>

**Cơ chế — dòng `printf` đó không "vô hại", nó vừa đổi hai thứ:**

| Nó đổi gì | Hệ quả | Lớp bug bị che |
|---|---|---|
| **Thời gian** — syscall + khoá stdio, tốn hàng chục µs–ms | Cửa sổ race đóng lại | **Data race**, lỗi thứ tự, timing với phần cứng |
| **Bố cục bộ nhớ** — thêm biến/tham số làm đổi stack frame, đổi thanh ghi | Ghi tràn giờ rơi vào ô vô hại | **Buffer overflow**, biến chưa khởi tạo, use-after-free |

⇒ **Bug biến mất khi quan sát chính là MỘT KẾT QUẢ CHẨN ĐOÁN**, không phải chuyện xui. Nó thu hẹp phạm vi rất mạnh: gần như chắc chắn là **race** hoặc **UB bộ nhớ** — chứ không phải lỗi logic thuần (lỗi logic không quan tâm bạn có in gì không).

Tên gọi: **Heisenbug**.

**Vì sao — hai tầng:**
- **Tầng nông:** *"do thêm log làm đổi timing"*. Đúng, nhưng dẫn tới hành động sai: đi thêm/bớt log để "né".
- **Tầng thật:** ⚠️ **`printf` biến mất bug KHÔNG có nghĩa bug đã hết — chỉ là xác suất tụt xuống dưới ngưỡng bạn nhìn thấy.** Nó vẫn ở đó, và sẽ nổ ở nhà khách với compiler khác, tải khác, CPU khác. **Không bao giờ được coi đây là "đã sửa".**

**Hành động — đảo ngược hai nguyên tắc:**

**① Thôi QUAN SÁT THỤ ĐỘNG, chuyển sang PHÁT HIỆN CHỦ ĐỘNG.**
`printf` hỏi *"giá trị là gì"*; sanitizer hỏi *"có ai vừa làm sai luật không"* và **báo ngay tại chỗ vi phạm**, kể cả khi hậu quả chưa xuất hiện. Đó là khác biệt về chất.

| Nghi gì | Dùng gì | Giá phải trả (**phải biết để chọn**) |
|---|---|---|
| Data race | TSan | chậm **~5–15×**, RAM tăng nhiều ⇒ hầu như **không chạy nổi trên thiết bị**, phải chạy trên host |
| Tràn/UAF/rò | ASan | chậm **~2×**, RAM **~3×** ⇒ đôi khi vừa với thiết bị, thường vẫn phải host |
| UB (tràn số, lệch alignment) | UBSan | rẻ, gần như luôn bật được |

⇒ **Đây là lý do phải giữ được khả năng build cùng codebase trên x86 host.** Không có nó, cả nhóm công cụ này vô dụng — và đó là quyết định **kiến trúc**, phải làm từ đầu chứ không phải lúc có bug.

**② Thôi né bug — ép nó XẢY RA NHIỀU HƠN.**
Mục tiêu ngược với trực giác: **tăng tần suất** để mỗi thí nghiệm rẻ đi (DBG-008 bước 1).
- Tăng tải, tăng số luồng, chạy vòng lặp hàng nghìn lần.
- **Ghim tất cả về 1 core** (làm lộ giả định "chắc chạy song song mới lỗi") — hoặc ngược lại, ép chạy trên nhiều core.
- **Chèn `sleep`/delay ngẫu nhiên vào đúng cửa sổ nghi ngờ** — nếu tần suất lỗi **tăng vọt**, bạn vừa xác nhận giả thuyết race *và* khoanh được vị trí.
- Chạy trên phần cứng **yếu hơn/khác kiến trúc** (ARM vs x86 — mô hình bộ nhớ lỏng hơn, race lộ dễ hơn nhiều).

**Bẫy:**
1. **Coi việc thêm log là đã sửa** — lỗi nghiêm trọng nhất ở câu này.
2. **Dùng debugger để bắt race** — breakpoint làm chậm gấp nghìn lần, race không bao giờ xảy ra. Sai công cụ.
3. **Kết luận "đã hết" sau vài trăm lần chạy sạch** — với bug 1/2000 thì đó là **kỳ vọng thống kê bình thường**, không phải bằng chứng. Phải chỉ ra **cơ chế** đã bị chặn.
4. **Chỉ test trên máy dev x86** — x86 có mô hình bộ nhớ mạnh, che rất nhiều lỗi memory-order sẽ nổ trên ARM.

**Chốt:** *"Bug biến mất khi quan sát là một kết luận chẩn đoán: gần như chắc là race hoặc UB bộ nhớ. Đừng quan sát thụ động nữa — đổi sang công cụ phát hiện chủ động, và ép bug xảy ra nhiều hơn thay vì né nó."*

> 🎤 Viết lại 2026-08-13 (nợ chất lượng bank). Bản cũ nêu đúng kết luận nhưng không có **cơ chế vì sao printf che bug**, không có **giá phải trả của từng sanitizer** (thứ quyết định chạy được trên thiết bị hay không), và không có phần **ép bug xảy ra nhiều hơn**.
</details>

#### DBG-013 · 🔴 · concept · [→ memory-bugs](../../../09-debugging/memory-bugs.md), [concurrency](../../../02-modern-cpp/concurrency.md)
**Làm sao phát hiện data race? Vì sao khó debug bằng cách thường?**
<details><summary>Đáp án</summary>

**Vì sao cách thường thất bại — ba tính chất khiến race miễn nhiễm với debug truyền thống:**
1. **Không tất định** — phụ thuộc timing/scheduling; chạy 1000 lần đúng, lần 1001 sai. Không tái hiện được theo yêu cầu.
2. **Quan sát làm nó biến mất** — thêm `printf` hoặc chạy dưới debugger **đổi timing**, cửa sổ race đóng lại. Đây là **Heisenbug**.
3. **Hậu quả cách xa nguyên nhân** — dữ liệu hỏng lúc này, crash 10 phút sau ở chỗ hoàn toàn khác.

**Công cụ đúng: ThreadSanitizer** — nó không đợi bug xảy ra, mà **suy luận** về nó.

```bash
g++ -fsanitize=thread -g -O1 main.cpp -o app && ./app
# ⚠️ ASan và TSan XUNG KHẮC — không bật chung, chạy lần lượt
```

TSan dựng quan hệ **happens-before** giữa các truy cập: nó ghi nhận thread nào chạm địa chỉ nào, qua lock/atomic nào. Nếu hai thread chạm cùng vùng nhớ, **≥1 là ghi**, mà **không có** quan hệ đồng bộ nào giữa hai lần chạm → báo race — **kể cả khi lần chạy đó cho kết quả đúng**. Đây là điểm mấu chốt: nó bắt được race **tiềm ẩn**, không phải chỉ race **đã phát tác**.

| Công cụ | Bắt được | Ghi chú |
|---|---|---|
| **TSan** | Data race (userspace) | ~5–15x chậm, tốn RAM; chuẩn cho C++ đa luồng |
| **Helgrind/DRD** (Valgrind) | Race + lỗi thứ tự lock | Không cần build lại, rất chậm |
| **LOCKDEP** (`CONFIG_PROVE_LOCKING`) | Thứ tự khoá sai **trong kernel** | Cảnh báo *trước khi* deadlock xảy ra |
| **KCSAN** | Data race **trong kernel** | Bản kernel của TSan |

**Chiến lược khi không dùng được sanitizer** (target hạn chế): (1) chạy TSan **trên host** với cùng code; (2) **stress test** — tăng số thread, thêm `sched_yield()`/delay ngẫu nhiên để **mở rộng** cửa sổ race thay vì thu hẹp; (3) chạy trên máy **nhiều core thật** (race hiếm lộ trên 1 core); (4) đọc code theo hướng "biến chia sẻ nào không có lock bảo vệ" — lập bảng biến ↔ lock.

**Bẫy:** (1) `printf` để debug race — chính nó có khoá nội bộ, **tự tạo đồng bộ** và giấu bug; (2) "chạy 100 lần không lỗi nên hết bug" — race không chứng minh được bằng test, chỉ bằng phân tích; (3) sửa bằng cách thêm `volatile` — không giải quyết gì ([CPP-022](cpp.md)).

**Chốt:** *"Race không tái hiện theo ý muốn và quan sát thì nó trốn — nên đừng săn bằng printf. Dùng TSan: nó bắt race tiềm ẩn kể cả khi lần chạy đó chưa sai."*
</details>

#### DBG-014 · 🔴 · design · ⭐ · 🏗️ · [→ tools](../../../09-debugging/tools.md), [kernel-debugging](../../../09-debugging/kernel-debugging.md)
**500 thiết bị đang chạy ở kho hàng của khách. Vài cái tự khởi động lại, vài ngày một lần, không theo quy luật. Bạn không SSH vào được, không gắn debugger, và khách không cho dừng dây chuyền. Thiết kế cách điều tra.**
<details><summary>Đáp án</summary>

**Đổi bài toán trước khi giải: bạn không debug được thiết bị đó — bạn chỉ có thể LÀM CHO NÓ TỰ KỂ LẠI.**

Toàn bộ thiết kế xoay quanh một câu: *"lần crash tới, thiết bị phải để lại đủ dấu vết để tôi không cần lần thứ hai."* Vì mỗi lần chờ là **vài ngày**, thí nghiệm cực đắt (DBG-008 bước 1) ⇒ ưu tiên **thu thập tối đa mỗi lần**, không phải đoán nhanh.

**Bốn tầng, theo thứ tự triển khai:**

| Tầng | Làm gì | Đổi lại được gì |
|---|---|---|
| **① Khoanh vùng bằng dữ liệu đã có** | 500 máy mà chỉ vài cái lỗi ⇒ **so cái hỏng với cái lành**: cùng firmware? cùng lô sản xuất? cùng vị trí trong kho (nhiệt độ/Wi-Fi)? crash sau bao lâu kể từ khi bật? | Rẻ nhất, làm trước tiên. **"Sau ~3 ngày uptime"** ⇒ nghi rò tài nguyên; **"chỉ 3 máy cùng lô"** ⇒ nghi phần cứng |
| **② Hộp đen sống sót qua reboot** | Ghi vào vùng **không bị xoá khi khởi động lại** (phân vùng riêng / vùng RAM được giữ / flash): lý do reset, uptime, RAM trống, dấu vết ngăn xếp cuối | **Đây là thứ quyết định.** Không có nó thì mỗi lần crash = mất sạch bằng chứng |
| **③ Phân biệt kiểu chết** | Reset do **watchdog** (treo) · do **kernel panic** (lỗi kernel/driver) · do **OOM killer** (hết RAM) · do **sụt nguồn** (phần cứng) — bốn thứ này điều tra theo bốn hướng **hoàn toàn khác nhau** | Không phân biệt được thì đi sai hướng cả tuần |
| **④ Vòng phòng ngừa** | Build **cùng codebase** trên host chạy sanitizer + soak test dài ngày trong phòng lab | Bắt được bug tiềm ẩn mà không cần chờ khách |

**Hai quyết định phải làm TRƯỚC khi có crash — sau đó là quá muộn:**

**① Lưu symbol theo từng bản phát hành.** Dump/backtrace từ thiết bị chỉ là **địa chỉ**. Không có đúng file symbol **của đúng bản build đó** thì nó là số vô nghĩa. ⇒ CI phải **lưu trữ symbol theo version**, và firmware phải **tự báo version của mình** trong log. Đây là lỗi tổ chức hay gặp nhất: có dump mà không giải mã được.

**② Chọn cái để log — và trả giá cho nó.**

| Đánh đổi | Vì sao đau ở embedded |
|---|---|
| Log nhiều ↔ **mòn flash** | eMMC/SD có số lần ghi hữu hạn; log dày tay có thể **tự tạo ra** hỏng hóc mới |
| Log nhiều ↔ **đầy ổ** | Phải xoay vòng log; xoay vòng lại **xoá mất đoạn ngay trước crash** — đúng đoạn cần nhất ⇒ giữ riêng một vùng nhỏ chỉ ghi khi sắp chết |
| Gửi log về ↔ **băng thông / quyền riêng tư** | Kho hàng thường mạng yếu; và dữ liệu khách có thể không được phép rời khỏi site |
| Log đồng bộ ↔ **đổi timing** | Log dày trên đường nóng có thể **che mất chính con bug** (DBG-012) |

**Áp vào ca này — giả thuyết xếp theo xác suất:** *"vài ngày một lần, vài máy"* → **rò tài nguyên tích tụ** (bộ nhớ, fd, thread) là nghi can số một, vì nó khớp với **tương quan theo uptime**. Kiểm rẻ nhất: cho thiết bị **tự ghi định kỳ RAM trống + số fd đang mở**; nếu đồ thị đi lên đều ⇒ xong, không cần chờ crash nữa.

⇒ **Đó là mẹo đáng nhớ nhất của câu này: đừng chỉ điều tra lúc crash — hãy đo thứ TĂNG DẦN TRƯỚC KHI crash.** Bug tích tụ lộ ra ở xu hướng, sớm hơn nhiều so với lúc nổ.

**Bẫy:**
1. **Đòi tái hiện trong lab trước khi làm gì cả** — vài ngày/lần trên 500 máy có thể là điều kiện lab không bao giờ dựng được. Phải song song: dựng hộp đen **ngay**, đừng chờ.
2. **Thu được dump nhưng không có symbol khớp bản build** — coi như không có gì.
3. **Log xoay vòng nuốt mất đoạn trước crash** — giữ riêng vùng "phút cuối".
4. **Cho rằng crash = lỗi phần mềm** — watchdog reset và sụt nguồn trông giống hệt nhau từ phía log; ③ tồn tại để tách chúng.
5. **Chỉ sửa cho máy đang lỗi** — 500 máy cùng firmware thì đó là bug của **cả lô**, chỉ khác ở điều kiện kích hoạt.

**Chốt:** *"Không debug được thiết bị ở xa thì phải thiết kế cho nó tự kể lại: hộp đen sống sót qua reboot + symbol lưu theo version + phân biệt được KIỂU chết. Và đo thứ tăng dần trước khi crash, đừng chỉ chờ lúc crash."*

> 🎤 Viết lại 2026-08-13 (nợ chất lượng bank). Bản cũ là **danh sách 4 công cụ** — đúng nhưng nặng tên lệnh (T3) và thiếu toàn bộ phần **quyết định** (lưu symbol, đánh đổi khi log, phân biệt kiểu chết, đo xu hướng).
</details>

#### DBG-015 · 🔴 · concept · ⭐ · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Kernel oops: bạn đọc thông tin gì để tìm nguyên nhân?**
<details><summary>Đáp án</summary>

**Oops = "core dump của kernel in ra dmesg".** Nó chứa đủ để truy về **đúng dòng source**, chỉ cần đọc theo thứ tự.

```
BUG: kernel NULL pointer dereference, address: 0000000000000000   ← ① loại lỗi + địa chỉ
RIP: 0010:my_driver_read+0x2c/0x80 [my_driver]                    ← ② hàm + offset + MODULE
Call Trace:                                                        ← ③ đường đi tới đây
 vfs_read+0x9d/0x150
 ksys_read+0x5f/0xe0
Modules linked in: my_driver(OE) …                                 ← ④ module nào đang nạp
Tainted: G           OE     5.15.0                                 ← ⑤ kernel đã "bẩn" chưa
```

**Đọc theo thứ tự này:**

| # | Trường | Nói lên điều gì |
|---|---|---|
| ① | Loại lỗi + **địa chỉ** | `NULL deref` (địa chỉ ~0) → con trỏ chưa init / đã free. Địa chỉ như `6b6b6b6b` → **poison pattern** = dùng vùng đã free. Địa chỉ lớn kỳ lạ → con trỏ rác |
| ② | **RIP/PC** | Nơi **chết**: hàm + offset + **module nào**. Đây là điểm neo để map về source |
| ③ | **Call Trace** | Nơi **đi tới** — đường thực thi. Cho biết vào từ syscall nào, hay từ ngữ cảnh ngắt |
| ④ | Modules linked in | Xác nhận driver của bạn đang nạp; `(OE)` = out-of-tree + không dấu |
| ⑤ | **Tainted** | `G/P` license, `O` out-of-tree, `D` đã oops trước đó. ⚠️ Nếu đã oops trước → dữ liệu **có thể không tin được** |

**Map offset → dòng source (bước quyết định):**
```bash
addr2line -e my_driver.ko 0x2c                    # cần .ko có debug info
gdb my_driver.ko -ex 'list *(my_driver_read+0x2c)'
# hoặc: ./scripts/decode_stacktrace.sh < oops.txt  (kernel source tree)
```

**Bẫy:** (1) `.ko` dùng để giải mã phải là **đúng bản build** đã nạp — lệch một lần compile là offset trỏ sai hàm, dẫn đi lạc hoàn toàn; (2) đọc **Call Trace** mà quên rằng có dòng `?` là **suy đoán từ stack cũ**, không chắc đúng; (3) chỉ nhìn hàm cuối mà bỏ **ngữ cảnh** — oops trong **interrupt context** có nghĩa nghi phạm là "gọi hàm ngủ trong ISR", khác hẳn oops ở process context; (4) oops ≠ panic — oops có thể để hệ thống chạy tiếp nhưng **trạng thái đã không tin được**, phải reboot khi debug xong.

**Đi xa hơn oops:** **KASAN** (bản kernel của ASan) bắt UAF/overflow ngay chỗ xảy ra thay vì để crash muộn; **kdump/crash** thu `vmcore` phân tích offline khi panic ở field; **LOCKDEP** cho lỗi khoá.

**Chốt:** *"RIP cho biết chết ở đâu, Call Trace cho biết đi tới đó bằng đường nào, `addr2line` đưa về đúng dòng. Kiểm tra tainted trước khi tin bản dump."*
</details>

#### DBG-016 · 🟠 · design · ⭐ · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Bạn debug một lỗi cross-layer thế nào?**
<details><summary>Đáp án</summary>

**Nguyên tắc: bisect theo *tầng*, không đoán.** Với lỗi xuyên app → driver → phần cứng, sai lầm kinh điển là nhảy thẳng vào tầng mình quen nhất. Thay vào đó, **xác nhận dữ liệu ở từng ranh giới** để thu hẹp: chỉ cần biết dữ liệu **đúng ở tầng N** và **sai ở tầng N+1** là khoanh được vùng lỗi.

| Tầng | Câu hỏi cần trả lời | Công cụ |
|---|---|---|
| **App (user)** | Giá trị gửi đi có đúng như mình nghĩ? | gdb / log, `strace` xem syscall thật sự phát ra |
| **Ranh giới user↔kernel** | Lời gọi có **xuống tới** driver không? Tham số còn nguyên? | `strace`, `printk` ở đầu `ioctl`/`write` |
| **Driver (kernel)** | Driver **nhận** được gì? Ghi ra thanh ghi nào? | `dmesg`/`printk`, `ftrace`, `/sys` `/debugfs` |
| **Phần cứng** | Thanh ghi có nhận đúng giá trị? Thiết bị có phản hồi? | đọc lại register, `devmem`, oscilloscope/logic analyzer, JTAG |

**Nghi phạm hay gặp ở từng ranh giới:** sai **offset thanh ghi** hoặc thiếu barrier; `copy_from_user`/`copy_to_user` sai kích thước; **struct padding/alignment khác nhau** giữa user và kernel (32-bit app trên kernel 64-bit); endianness; sai **số hiệu `ioctl`** (`_IOR`/`_IOW` không khớp); buffer chưa `dma_map` mà đã cho thiết bị đọc.

**Bẫy:** (1) sửa nhiều chỗ cùng lúc rồi thấy hết lỗi — **không biết cái nào chữa**, và thường đã cài thêm bug; đổi **một biến mỗi lần**; (2) tin log mà không tin số liệu — `printk` có thể bị **ratelimit**/mất khi crash; (3) quên rằng thêm `printk` **làm đổi timing** → bug race biến mất (heisenbug), lúc đó phải dùng ftrace/tracepoint thay vì print; (4) không **verify ngược** sau khi fix: phải chạy lại từ app xuống tới phần cứng, không chỉ test tầng vừa sửa.

**Chốt:** *"Chia đôi theo tầng và xác nhận dữ liệu ở từng ranh giới — mỗi lần xác nhận cắt đôi vùng nghi ngờ. Không đoán, và mỗi lần chỉ đổi một thứ."*
</details>

#### DBG-017 · 🟢 · concept · ⭐ · [→ gdb](../../../09-debugging/gdb.md)
**Các lệnh GDB cốt lõi?**
<details><summary>Đáp án</summary>

`break` đặt breakpoint, `run`, `next`/`step` (không/có vào hàm), `continue`, `finish`, `print` in biến, **`backtrace`/`bt`** in call stack, `frame N`, `info locals`, **`watch`** dừng khi biến đổi giá trị.
</details>

#### DBG-018 · 🟡 · concept · ⭐ · [→ gdb](../../../09-debugging/gdb.md)
**Debug binary chạy trên thiết bị target không có GDB đầy đủ thì sao?**
<details><summary>Đáp án</summary>

**Remote debug — tách đôi công việc:** `gdbserver` (rất nhẹ, vài trăm KB, chỉ biết dừng/chạy/đọc-ghi bộ nhớ) chạy **trên target**; **GDB đầy đủ** cùng với **symbol + source** chạy **trên host**, hai bên nói chuyện qua **GDB Remote Serial Protocol** (TCP hoặc serial).

```bash
# --- trên TARGET (thiết bị nhúng) ---
gdbserver :2345 ./myapp              # chờ host kết nối
gdbserver :2345 --attach <pid>       # gắn vào tiến trình đang chạy

# --- trên HOST ---
arm-linux-gnueabihf-gdb ./myapp      # ⚠️ GDB của TOOLCHAIN, không phải gdb của host
(gdb) set sysroot /path/to/target/rootfs   # để nạp symbol của libc/thư viện target
(gdb) target remote 192.168.1.10:2345
(gdb) break main
(gdb) continue
```

**Ba điều kiện phải đúng, thiếu một là hỏng:**

| | Vì sao |
|---|---|
| GDB **cross** đúng kiến trúc (`arm-linux-…-gdb`) | GDB của host không giải mã được lệnh ARM |
| Binary trên host có **symbol** (`-g`), binary trên target có thể đã **strip** | Symbol nằm ở host — đó chính là lý do target không cần GDB đầy đủ. Hai bản phải **cùng một lần build** |
| **`set sysroot`** trỏ tới rootfs của target | Không có thì không giải được symbol trong `libc`/thư viện dùng chung → backtrace toàn `??` |

**Khi ngay cả `gdbserver` cũng không chạy được:** (1) **core dump** — bật `ulimit -c unlimited`, lấy file core về host phân tích post-mortem (`gdb ./app core` → `bt`), hợp với lỗi ở field khó tái hiện; (2) **JTAG/SWD + OpenOCD** khi debug ở mức bare-metal/bootloader, trước cả khi có OS; (3) log + `strace`.

**Bẫy:** (1) build lại giữa chừng làm **symbol lệch** binary trên target → breakpoint đặt sai chỗ, giá trị biến vô nghĩa; (2) quên `set sysroot` rồi kết luận "stack bị hỏng" trong khi chỉ là thiếu symbol; (3) firewall/port đóng — thử `nc` trước; (4) `-O2` trên target khiến biến bị optimize-out (`<optimized out>`) → build bản debug riêng nếu được.

**Chốt:** *"gdbserver ở target giữ tiến trình, GDB + symbol ở host giữ trí tuệ. Không tái hiện được thì lấy core dump về host."*
</details>

#### DBG-019 · 🟡 · concept · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Phân biệt kernel oops và panic?**
<details><summary>Đáp án</summary>

Oops: lỗi nghiêm trọng nhưng kernel cố tiếp tục (kill process lỗi). Panic: không thể tiếp tục, dừng hệ thống. Cả hai in stack trace ra `dmesg`.
</details>

#### DBG-020 · 🟡 · concept · 📦 2026-08-13 · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Vì sao debug kernel khó hơn hẳn debug userspace? Kể những thứ bạn MẤT khi bước qua ranh giới đó.**
<details><summary>Đáp án</summary>

**Bản chất: ở userspace bạn được kernel bảo vệ và phục vụ. Trong kernel, bạn CHÍNH LÀ cái đó.**

| | Userspace | Kernel |
|---|---|---|
| Lỗi con trỏ | SIGSEGV — **chết một process**, hệ vẫn sống | **Oops/panic** — có thể chết cả máy, mất luôn bằng chứng |
| Chạy lại thử | `./a.out` — vài giây | **Reboot** — vài chục giây, và mất trạng thái |
| Debugger | `gdb` gắn vào, đặt breakpoint thoải mái | Dừng kernel là **dừng luôn thứ đang phục vụ debugger** ⇒ cần máy thứ hai (kgdb qua serial) hoặc máy ảo |
| In ra màn hình | `printf` | `printk` — nhưng **in trong ngữ cảnh ngắt** có thể tự gây deadlock |
| Bộ nhớ | Ảo, có bảo vệ, cấp phát thoải mái | Địa chỉ thật, **stack chỉ 8–16 KB**, tràn là hỏng im lặng |
| Sanitizer | ASan/TSan/Valgrind | Phải **biên dịch lại kernel** với KASAN/LOCKDEP |

**Ba thứ mất đi đau nhất:**
1. **Mất khả năng thử nhanh.** Vòng lặp "sửa → chạy → xem" từ vài giây thành vài phút ⇒ theo [DBG-008](debugging.md), chi phí một thí nghiệm tăng cả trăm lần ⇒ phải **suy luận nhiều hơn, thử ít hơn**, và mỗi lần chạy phải thu thập tối đa.
2. **Mất sự cô lập.** Bug trong driver của bạn có thể biểu hiện thành hỏng **ở subsystem khác** — vì mọi thứ dùng chung một không gian địa chỉ.
3. **Mất bằng chứng khi chết.** Máy panic là log trong RAM bay mất ⇒ phải chuẩn bị **trước**: log ra serial console, hoặc vùng nhớ giữ qua reboot, hoặc kdump.

⇒ **Hệ quả về cách làm việc:** ưu tiên **tái hiện trong máy ảo** (snapshot, khởi động lại nhanh, gắn được debugger), và **đẩy logic ra userspace** bất cứ khi nào có thể — code ở userspace debug rẻ hơn cả bậc độ lớn.

**Chốt:** *"Trong kernel bạn mất khả năng thử nhanh, mất cô lập, và mất bằng chứng khi chết — nên phải chuyển từ 'thử cho tới khi hết lỗi' sang 'suy luận rồi thu thập tối đa mỗi lần chạy'."*
</details>

#### DBG-021 · 🟡 · concept · 📦 2026-08-13 · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**`printk`/`dmesg` dùng thế nào trong driver? Vì sao nó khác `printf`, và khi nào KHÔNG được dùng?**
<details><summary>Đáp án</summary>

**Cơ chế:** `printk` ghi vào một **ring buffer trong kernel** (kích thước cố định); `dmesg` chỉ là công cụ đọc buffer đó. Vì là ring buffer nên **log cũ bị đè** khi đầy — thông điệp quan trọng có thể biến mất trước khi bạn kịp xem.

**Khác `printf` ở ba điểm quan trọng:**
1. **Có mức ưu tiên** (`KERN_ERR`, `KERN_WARNING`, `KERN_INFO`, `KERN_DEBUG`) — và console chỉ hiện những mức dưới ngưỡng hiện hành. In `KERN_DEBUG` mà không thấy gì thường là do **ngưỡng**, không phải do code không chạy.
2. **Không có buffer stdio** — nó xuống ring buffer ngay, nên sống sót tốt hơn khi crash.
3. **Có thể tự gây vấn đề:** in ra console **serial** là thao tác **chậm và đồng bộ** (hàng ms cho một dòng ở 115200 baud).

**⚠️ Khi nào KHÔNG được dùng:**
- **Trong đường nóng / ngữ cảnh ngắt:** một dòng ra serial mất hàng ms trong khi ISR phải xong trong µs ⇒ trễ lan khắp hệ, mất dữ liệu, watchdog reset. Và bản thân việc in **làm đổi timing** ⇒ che mất bug bạn đang tìm ([DBG-012](debugging.md)).
- **Trong vòng lặp tần suất cao:** ring buffer bị đè liên tục ⇒ **tự xoá mất** thông điệp hữu ích, còn console ngập rác.

**Dùng gì thay:** **ftrace** (chi phí rất thấp, ghi vào buffer nhị phân — [DBG-022](debugging.md)) · **`printk_ratelimited`** cho thông điệp có thể lặp · bật/tắt log động thay vì biên dịch lại · và với bug timing thì **nhấp một chân GPIO** rồi đo bằng scope ([08/hardware-debug.md](../../../08-embedded-systems/hardware-debug.md)).

**Chốt:** *"`printk` ghi vào ring buffer có mức ưu tiên, không phải stdout. Nó chậm và làm đổi timing, nên tuyệt đối tránh ở đường nóng và trong ngắt — chỗ đó dùng ftrace hoặc GPIO."*
</details>

#### DBG-022 · 🟠 · concept · 📦 2026-08-13 · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**`ftrace` giải quyết vấn đề gì mà `printk` không giải quyết được?**
<details><summary>Đáp án</summary>

**Vấn đề `printk` không giải được: quan sát mà KHÔNG làm đổi cái đang quan sát.** In ra serial mất hàng ms ⇒ với bug timing/latency thì việc đo đã phá luôn phép đo (nguyên lý Heisenbug — [DBG-012](debugging.md)).

**ftrace** là hạ tầng trace **có sẵn trong kernel**, chi phí cực thấp: ghi bản ghi **nhị phân** vào ring buffer trong RAM, không định dạng chuỗi, không chạm I/O. Bật/tắt **lúc chạy** qua filesystem, không cần biên dịch lại.

**Nó trả lời được ba câu mà `printk` không:**

| Câu hỏi | ftrace cho gì |
|---|---|
| *"Đường đi thực sự trong kernel là gì?"* | **Function graph** — cây lời gọi kèm **thời gian từng hàm**, thấy ngay hàm nào ngốn thời gian |
| *"Vì sao thỉnh thoảng trễ 40 ms?"* | **Tracer đo độ trễ** — ghi lại quãng cấm ngắt / cấm preempt **dài nhất**, kèm đúng vết dẫn tới nó |
| *"Ai gọi hàm này, với tham số gì, bao nhiêu lần?"* | **Tracepoint** cắm sẵn khắp kernel (lập lịch, I/O, mạng, IRQ) — bật lên là có, không cần sửa code |

⇒ **Điểm mạnh thật sự: bật được trên hệ ĐANG CHẠY, kể cả production**, và bắt được **giá trị xấu nhất** thay vì trung bình — thứ quyết định với hệ realtime.

**Đánh đổi:** dữ liệu nhị phân trong ring buffer (đầy thì đè, phải lấy ra kịp) · bật rộng quá thì vẫn ảnh hưởng hiệu năng · cần kernel được cấu hình bật ftrace (phần lớn bản phân phối có sẵn; **kernel nhúng tự dựng thì hay bị tắt** — kiểm trước khi cần tới).

**Chốt:** *"`printk` làm đổi timing nên vô dụng với bug timing. ftrace ghi nhị phân vào RAM với chi phí rất thấp, bật được lúc chạy, và đo được giá trị xấu nhất — đó là thứ printk không bao giờ làm được."*
</details>

#### DBG-023 · 🔴 · concept · 📦 2026-08-13 · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Trong kernel không có Valgrind/ASan/TSan. Vậy bắt lỗi bộ nhớ và deadlock bằng cách nào?**
<details><summary>Đáp án</summary>

**Kernel có bộ công cụ tương đương, nhưng phải BẬT LÚC BIÊN DỊCH** — đó là khác biệt cốt lõi so với userspace (nơi chỉ cần chạy lại chương trình dưới công cụ).

| Cần bắt | Cơ chế trong kernel | Tương đương userspace |
|---|---|---|
| Tràn bộ đệm, use-after-free | **KASAN** — shadow memory, báo **ngay tại chỗ vi phạm** kèm vết cấp phát/giải phóng | ASan |
| Biến chưa khởi tạo | **KMSAN** | MSan |
| Rò rỉ bộ nhớ kernel | **kmemleak** — quét kiểu gom rác, báo vùng cấp mà không ai còn trỏ tới | Valgrind |
| **Deadlock / sai thứ tự khoá** ⭐ | **LOCKDEP** | TSan |

**⭐ LOCKDEP đáng nói riêng — nó mạnh hơn TSan ở một điểm:** nó không đợi deadlock **xảy ra**. Nó ghi lại **thứ tự** mọi cặp khoá từng được lấy, và khi thấy một thứ tự **mâu thuẫn** với lần trước (A→B ở chỗ này, B→A ở chỗ kia) thì **báo ngay**, kể cả khi hai đường đó chưa bao giờ chạy đồng thời. Tức là nó bắt được **deadlock tiềm năng** trước khi nó kịp xảy ra lần đầu.

⇒ Đây là lý do **luôn bật KASAN + LOCKDEP trên kernel dùng để phát triển**, kể cả khi chưa có bug.

**Cái giá — vì sao không bật ở production:** KASAN làm chậm **~2–3×** và ngốn thêm RAM cho shadow memory; LOCKDEP thêm chi phí mỗi lần lấy khoá. Thiết bị nhúng RAM ít nhiều khi **không chạy nổi** ⇒ chiến lược thực tế: chạy chúng trong **máy ảo / bo phát triển**, còn thiết bị field thì dựa vào oops + log giữ qua reboot ([DBG-014](debugging.md)).

**Nguyên tắc chung đáng nhớ:** trong kernel, **quan sát thụ động (`printk`) hầu như không bắt được lớp bug này** — phải chuyển sang **phát hiện chủ động** (công cụ báo tại thời điểm vi phạm), y hệt bài học ở userspace ([DBG-012](debugging.md)).

**Chốt:** *"Kernel có KASAN/kmemleak/LOCKDEP nhưng phải bật lúc build, nên phải chủ động bật sẵn trên bo phát triển. LOCKDEP đặc biệt vì nó bắt sai thứ tự khoá TRƯỚC khi deadlock kịp xảy ra."*
</details>

#### DBG-024 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Kể các loại lỗi bộ nhớ thường gặp trong C/C++. Vì sao chúng nguy hiểm hơn lỗi logic thường?**
<details><summary>Đáp án</summary>

| Loại | Là gì | Vì sao khó |
|---|---|---|
| **Buffer overflow** | Ghi/đọc ngoài biên mảng | Sửa **dữ liệu của người khác** ⇒ triệu chứng hiện ở chỗ hoàn toàn khác |
| **Use-after-free** | Dùng con trỏ sau khi `free`/`delete` | Vùng nhớ có thể **đã cấp cho ai khác** ⇒ hai chủ sở hữu cùng ghi |
| **Double free** | `free` hai lần | Hỏng cấu trúc quản lý heap ⇒ crash ở lần `malloc` **sau đó**, rất xa |
| **Biến chưa khởi tạo** | Đọc giá trị rác | *"Chạy được trên máy tôi"* — vì rác ở đó tình cờ bằng 0 |
| **Memory leak** | Cấp mà không giải phóng | Không crash ngay; chết sau **nhiều ngày** chạy |
| **Dangling pointer / iterator** | Con trỏ tới vùng đã hết hạn (vd `vector` reallocate) | Hoàn toàn hợp lệ về cú pháp |

**⭐ Vì sao nguy hiểm hơn lỗi logic — ba lý do, đây là phần được chấm:**

1. **Nguyên nhân và triệu chứng CÁCH XA NHAU.** Lỗi logic thì sai ngay tại chỗ sai. Lỗi bộ nhớ thì ghi bậy ở module A, đến khi module B đọc phải thì mới nổ — có khi hàng giây và hàng nghìn dòng code sau. Bạn debug đúng chỗ **không có lỗi**.
2. **Là undefined behavior — compiler được phép làm bất cứ gì.** Bật `-O2` có thể làm bug đổi hình dạng hoặc biến mất; thêm một biến để in ra là **đổi bố cục stack** và bug lặn mất ([DBG-012](debugging.md)). *"Test không lỗi"* không chứng minh được gì.
3. **Không tất định.** Phụ thuộc bố cục bộ nhớ, timing, phiên bản compiler, ASLR ⇒ 1/2000 lần ở nhà khách, không bao giờ ở lab.

⇒ **Hệ quả về cách làm:** không đi "quan sát cho tới khi thấy", mà **bật công cụ phát hiện chủ động** (ASan/UBSan/Valgrind) để báo **ngay tại thời điểm vi phạm**, kể cả khi hậu quả chưa xuất hiện ([DBG-011](debugging.md)).

**Chốt:** *"Lỗi bộ nhớ nguy hiểm vì nguyên nhân và triệu chứng cách xa nhau, và vì là UB nên hành vi đổi theo compiler và tối ưu hoá — nên phải bắt bằng công cụ báo tại chỗ vi phạm, không bắt bằng mắt."*
</details>

#### DBG-025 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Làm sao phòng ngừa lỗi bộ nhớ ngay từ đầu bằng C++ hiện đại — thay vì đi bắt chúng về sau?**
<details><summary>Đáp án</summary>

**Nguyên tắc gốc: biến lỗi RUNTIME thành lỗi COMPILE-TIME, hoặc thành thứ không thể viết sai.**

| Thay vì | Dùng | Xoá bỏ lớp bug nào |
|---|---|---|
| `new`/`delete` thủ công | **RAII** — `unique_ptr`, `shared_ptr`, container | leak, double free, rò khi có exception |
| Con trỏ thô sở hữu | `unique_ptr` (sở hữu độc quyền), `shared_ptr` (chia sẻ), **`T&`/`T*` chỉ để *quan sát*** | nhập nhằng **ai chịu trách nhiệm giải phóng** — gốc của phần lớn bug |
| Mảng C + độ dài rời | `std::array`, `std::vector`, **`std::span`** | tràn biên do truyền nhầm độ dài |
| `strcpy`, `char*` | `std::string`, `std::string_view` | tràn bộ đệm |
| `union` thủ công, `void*` | `std::variant`, `std::optional` | đọc sai nhánh kiểu |
| Ép kiểu C | `static_cast`/`dynamic_cast` | ép nhầm im lặng |

**⭐ Ba nguyên tắc quan trọng hơn danh sách API:**

1. **Rule of Zero.** Class **không** tự quản lý tài nguyên thô ⇒ không cần viết destructor/copy/move ⇒ **không thể viết sai** chúng. Cần quản lý tài nguyên thì tách ra một class **chỉ làm mỗi việc đó**.
2. **Sở hữu phải hiện lên trong KIỂU DỮ LIỆU.** `Widget*` không nói được ai phải xoá; `unique_ptr<Widget>` thì nói rõ. Chọn kiểu để **người đọc và compiler cùng biết** — đây là thứ tài liệu không làm thay được.
3. **Vòng đời phải rõ ràng.** Phần lớn use-after-free là *"đối tượng chết trước người dùng nó"*: lambda bắt tham chiếu rồi chạy bất đồng bộ, `string_view` trỏ vào temporary, iterator sau khi `vector` reallocate. ⇒ Hỏi *"cái này sống tới bao giờ?"* mỗi khi lưu một tham chiếu.

⚠️ **Nhưng phòng ngừa không thay được kiểm chứng.** `shared_ptr` không cứu bạn khỏi **vòng tham chiếu** (cần `weak_ptr`), và không có smart pointer nào chặn được **data race**. Vẫn phải chạy ASan/UBSan/TSan trong CI ([DBG-011](debugging.md), [DBG-013](debugging.md)).

**Chốt:** *"Đừng đi bắt lỗi bộ nhớ — làm cho chúng không viết ra được: RAII cho mọi tài nguyên, sở hữu thể hiện trong kiểu, và luôn hỏi 'đối tượng này sống tới bao giờ'."*
</details>

#### DBG-026 · 🟡 · concept · 📦 2026-08-13 · [→ mindset](../../../09-debugging/mindset.md)
**Vì sao "sửa cho hết lỗi" là cách làm nguy hiểm? Phân biệt sửa TRIỆU CHỨNG với sửa NGUYÊN NHÂN.**
<details><summary>Đáp án</summary>

**Sửa triệu chứng** = làm biểu hiện biến mất mà **không hiểu cơ chế**. Dấu hiệu nhận biết — nếu bản vá của bạn trông như dưới đây thì gần như chắc chắn:

| Bản vá | Thực chất đang làm gì |
|---|---|
| Thêm `sleep()` / tăng timeout | **Giấu race** — chỉ thu hẹp cửa sổ, không đóng nó |
| Thêm retry vòng quanh chỗ lỗi | Che một lỗi vẫn đang xảy ra |
| `try/catch` rồi nuốt exception | Xoá bằng chứng |
| Kiểm tra kết quả lạ rồi bỏ qua | Chấp nhận dữ liệu sai |
| Thêm `printf` rồi hết lỗi ⇒ **giữ nguyên `printf`** | Đổi timing/bố cục stack — bug vẫn còn ([DBG-012](debugging.md)) |

**Ba tác hại — theo thứ tự nghiêm trọng:**
1. **Bug vẫn còn nhưng HIẾM HƠN** ⇒ lần sau nổ ở nhà khách, tần suất thấp hơn, **khó lần hơn gấp bội**. Bạn vừa làm bug của mình đắt hơn.
2. **Mất bằng chứng.** Retry/nuốt exception xoá đúng thứ mà lần điều tra sau cần.
3. **Tích tụ.** Mỗi bản vá triệu chứng thêm một nhánh code không ai hiểu vì sao tồn tại; sau vài năm không ai dám gỡ.

**Phép thử một câu — trước khi merge, tự hỏi:**
> *"Tôi có giải thích được **vì sao** bản vá này làm bug biến mất không?"*

Trả lời được ⇒ sửa nguyên nhân. Trả lời *"không rõ, nhưng thử thì hết"* ⇒ **sửa triệu chứng**, và bạn chưa xong việc.

**Kỹ thuật "5 whys" áp vào thực tế:** *mã sai* → vì buffer bị ghi đè → vì độ dài lấy từ gói tin → vì **không kiểm biên độ dài** ⇒ **chỗ này mới là chỗ sửa**. Dừng ở tầng 1 thì bản vá là "thử lại khi mã sai".

⚠️ **Ngoại lệ hợp lệ:** giảm nhẹ tạm thời để **dừng chảy máu ở production** là đúng — nhưng phải **ghi lại là tạm thời**, mở ticket, và **giữ lại bằng chứng**. Vấn đề không phải giảm nhẹ, mà là **coi giảm nhẹ là đã xong**.

**Chốt:** *"Nếu không giải thích được vì sao bản vá làm bug biến mất thì bạn chưa sửa bug — bạn vừa làm nó hiếm hơn và khó lần hơn."*
</details>

#### DBG-027 · 🟡 · concept · 📦 2026-08-13 · [→ mindset](../../../09-debugging/mindset.md)
**`git bisect` hoạt động thế nào? Điều kiện nào phải có thì nó mới dùng được?**
<details><summary>Đáp án</summary>

**Cơ chế: tìm kiếm nhị phân trên LỊCH SỬ COMMIT.** Bạn đánh dấu một commit **tốt** đã biết và một commit **xấu** đã biết; git checkout điểm giữa, bạn thử và trả lời tốt/xấu; nó lặp lại. Mỗi lần trả lời **loại một nửa** ⇒ 1000 commit chỉ cần **~10 lần thử** (`log₂1000 ≈ 10`).

Đây là hiện thân trực tiếp của nguyên tắc *"mỗi thí nghiệm phải loại được một nửa"* ([DBG-008](debugging.md)) — chỉ khác là nó bisect trên **trục thời gian** thay vì trục không gian code.

**Ba điều kiện bắt buộc — thiếu một là dùng không được:**

| Điều kiện | Vì sao | Thiếu thì làm gì |
|---|---|---|
| **Tái hiện được ổn định** | Bạn phải trả lời tốt/xấu **chắc chắn** cho mỗi commit | Bug ngẫu nhiên ⇒ chạy N lần mỗi bước và định nghĩa "xấu" theo tỉ lệ |
| **Mỗi commit build & chạy được** | Commit hỏng giữa chừng làm gãy chuỗi | Đánh dấu `skip` — bisect vẫn chạy nhưng kém chính xác hơn |
| **Biết một mốc TỐT** | Không có cận dưới thì không có khoảng để chia | Lùi dần theo cấp số nhân (10, 100, 1000 commit) tới khi tìm được mốc tốt |

**Điểm mạnh ít người khai thác: tự động hoá.** Viết một script trả về 0 (tốt) / khác 0 (xấu) rồi cho bisect **tự chạy** — nó tìm ra commit thủ phạm mà bạn đi làm việc khác. Với bug hiếm, script cho chạy 200 lần rồi báo xấu nếu lỗi xuất hiện ≥1 lần.

**Kết quả bisect nói gì và KHÔNG nói gì:** nó chỉ ra commit **làm bug lộ ra**, không nhất thiết là commit **có lỗi**. Rất thường gặp: một commit vô hại đổi thứ tự khởi tạo, làm lộ một race đã tồn tại từ lâu. ⇒ Đọc kỹ commit tìm được, đừng vội đổ lỗi.

**Hệ quả về thói quen làm việc:** bisect chỉ hữu ích khi lịch sử có **commit nhỏ, mỗi commit build được**. Commit kiểu *"sửa 40 file, gộp 3 tính năng"* làm bisect gần như vô dụng — đây là lý do thực dụng để commit nhỏ.

**Chốt:** *"bisect là tìm kiếm nhị phân trên lịch sử — 1000 commit chỉ cần 10 lần thử. Nhưng nó đòi bug tái hiện ổn định và mọi commit build được, nên nó thưởng cho thói quen commit nhỏ."*
</details>

#### DBG-028 · 🟢 · concept · 📦 2026-08-13 · [→ tools](../../../09-debugging/tools.md)
**`strace` và `ltrace` khác nhau thế nào?**
<details><summary>Đáp án</summary>

Khác ở **ranh giới mà chúng chặn**:

| | `strace` | `ltrace` |
|---|---|---|
| Chặn ở | **Ranh giới user ↔ kernel** (syscall) | Ranh giới **chương trình ↔ thư viện động** |
| Thấy được | `open`, `read`, `mmap`, `connect`, `ioctl`… | `malloc`, `strcpy`, hàm của `.so` bên thứ ba |
| Trả lời câu | *"Chương trình xin OS làm gì, và OS trả lời gì?"* | *"Nó gọi hàm thư viện nào, tham số gì?"* |
| Độ tin cậy | **Cao** — syscall là ranh giới thật, không né được | Thấp hơn — hàm gọi **nội bộ trong `.so`** hoặc được inline thì không thấy |

**Thực tế `strace` hữu dụng hơn nhiều**, vì hầu hết câu hỏi debug thực chiến đều nằm ở ranh giới kernel: *file nào không mở được* (`ENOENT` — sai đường dẫn), *vì sao bị từ chối* (`EACCES` — sai quyền), *có kết nối được không*, *treo ở syscall nào*.

⚠️ Cả hai **làm chậm chương trình đáng kể** (mỗi sự kiện bị chặn) ⇒ có thể **che mất bug timing**, và **không dùng được** với tiến trình đang kẹt ở `D` state (nó không quay lại user space để bị trace).

**Chốt:** *"`strace` xem cuộc nói chuyện với kernel, `ltrace` xem cuộc gọi vào thư viện. Câu hỏi debug thật hầu hết nằm ở ranh giới kernel nên strace dùng nhiều hơn hẳn."*
</details>

#### DBG-029 · 🟠 · design · ⭐ · 🏗️ · 📦 2026-08-13 · [→ tools](../../../09-debugging/tools.md)
**Vì sao logging là công cụ debug quan trọng nhất ở production? Một hệ thống log tốt cần gì?**
<details><summary>Đáp án</summary>

**Vì sao quan trọng nhất:** ở production bạn **không có** debugger, **không thể** dừng hệ thống, và nhiều bug **không tái hiện được** trong lab. Log là **thứ duy nhất còn lại** sau khi sự việc đã xảy ra — và bug hiếm thì bạn chỉ có **một cơ hội mỗi vài ngày** ([DBG-014](debugging.md)).

**Sáu thứ một hệ log tốt phải có:**

| Thành phần | Vì sao |
|---|---|
| **Mức** (error/warn/info/debug) | Chỉnh **lúc chạy**, không phải biên dịch lại — bật chi tiết đúng lúc cần |
| **Timestamp đơn điệu + timestamp lịch** | Đo khoảng phải dùng **monotonic** (NTP nhảy giờ làm sai lệch — [LNX-029](linux-sysprog.md)); ghi thời điểm thì dùng lịch |
| **Ngữ cảnh định danh** | Request/device/session ID để **nối các dòng rời rạc** thành một câu chuyện. Không có thì log đa luồng là vô nghĩa |
| **Có cấu trúc** (khoá=giá trị) | Để **lọc và đếm** được, thay vì đọc bằng mắt |
| **Xoay vòng + trần dung lượng** | Log không được **tự làm đầy ổ** hay mòn flash |
| **Vùng "phút cuối" riêng** | Xoay vòng hay **nuốt mất đoạn ngay trước crash** — đúng đoạn cần nhất |

**⭐ Ba đánh đổi phải nêu (chỗ phân biệt câu trả lời mid với senior):**
1. **Log nhiều ↔ mòn flash.** eMMC/SD có số chu kỳ ghi hữu hạn ⇒ log dày tay có thể **tự tạo ra hỏng hóc mới**.
2. **Log nhiều ↔ đổi timing.** Log đồng bộ trên đường nóng có thể **che mất chính con bug** đang tìm ([DBG-012](debugging.md)).
3. **Log nhiều ↔ băng thông & quyền riêng tư.** Thiết bị field mạng yếu; và dữ liệu khách có thể không được phép rời khỏi site.

**Nguyên tắc chọn nội dung log:** ghi **quyết định và đầu vào của quyết định**, không ghi *"đã tới đây"*. `"chose path=B because temp=71 > limit=70"` giúp được; `"entering handler"` thì không.

⚠️ **Và log không thay được số đo.** Bug tích tụ (rò bộ nhớ, rò fd) lộ ra ở **xu hướng**, không ở một dòng log ⇒ phải ghi **định kỳ các chỉ số** (RAM trống, số fd, độ sâu hàng đợi) để nhìn được đồ thị đi lên **trước khi** crash.

**Chốt:** *"Ở production log là bằng chứng duy nhất, nên phải thiết kế trước: có mức, có timestamp monotonic, có ID nối chuyện, và giữ riêng phút cuối trước khi chết. Nhưng phải trả giá bằng mòn flash và đổi timing — nên ghi quyết định, đừng ghi dấu chân."*
</details>

---
⬅️ [Bank index](README.md)
