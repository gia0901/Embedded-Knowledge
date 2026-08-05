# DBG — Debugging & Tools

> Domain `DBG`. Gộp từ [11/debugging.md](../../../11-interview-questions/debugging.md) + [technical_round/02 PHẦN 6](../../technical_round/02_question_bank.md). Nhiều câu dạng tình huống ("bạn sẽ làm gì nếu…"). Track dùng: `debugging`, `bsp`, `cpp-system`.

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
**Chương trình segfault. Bạn điều tra thế nào?**
<details><summary>Đáp án</summary>

Chạy trong gdb tới khi crash rồi `bt` xem call stack tại điểm crash (lộ hàm lỗi + đường dẫn gọi), `frame`/`print`/`info locals` xem biến (null? index sai?). Nếu không chạy live được hoặc crash ở field, dùng core dump. Hoặc build với AddressSanitizer để bắt chính xác chỗ truy cập sai + nơi cấp phát/giải phóng.
</details>

#### DBG-005 · 🟡 · concept · [→ tools](../../../09-debugging/tools.md)
**Chương trình báo lỗi mơ hồ "không khởi động được". strace giúp gì?**
<details><summary>Đáp án</summary>

`strace` ghi mọi syscall + kết quả. Thường thấy ngay `openat("/path/config", ...) = -1 ENOENT` (sai đường dẫn) hoặc `EACCES` (quyền), hoặc thiếu thư viện. Lớp bug "thiếu file/lib/quyền/mạng" giải quyết rất nhanh bằng strace vì nó cho thấy chương trình **thực sự** tìm gì.
</details>

#### DBG-006 · 🟡 · concept · [→ tools](../../../09-debugging/tools.md)
**Chương trình chạy chậm. Tìm nguyên nhân thế nào?**
<details><summary>Đáp án</summary>

Không đoán hotspot mà **đo** bằng perf: `perf stat` (tổng quan cycle, cache miss), `perf record -g` + `perf report` (hàm nào chiếm CPU — flame graph). Nếu chậm do chờ I/O chứ không phải CPU, perf cho thấy CPU rảnh → nhìn sang strace `-T` (thời gian mỗi syscall) hoặc lock contention.
</details>

#### DBG-007 · 🟡 · concept · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**Memory leak: phát hiện thế nào?**
<details><summary>Đáp án</summary>

AddressSanitizer/LeakSanitizer (`-fsanitize=address`) hoặc `valgrind --leak-check=full` — báo nơi cấp phát mà không giải phóng kèm stack. Theo dõi RSS (`/proc/<PID>/status`) qua thời gian để xác nhận rò rỉ. Phòng ngừa: RAII/smart pointer.
</details>

#### DBG-008 · 🟠 · concept · [→ mindset](../../../09-debugging/mindset.md)
**Mô tả quy trình debug có hệ thống của bạn.**
<details><summary>Đáp án</summary>

Phương pháp khoa học: (1) Reproduce ổn định, tối thiểu; (2) Isolate — thu hẹp bằng chia để trị (binary search trong code, git bisect, đơn giản hóa input); (3) Hypothesize giả thuyết cụ thể kiểm chứng được; (4) Test — đổi một biến mỗi lần; (5) Fix nguyên nhân gốc (không phải triệu chứng, "5 whys"); (6) Verify + thêm test ngăn tái diễn. Xuyên suốt: tin dữ liệu hơn trực giác, kiểm tra giả định trước.
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

#### DBG-012 · 🔴 · concept · [→ mindset](../../../09-debugging/mindset.md), [memory-bugs](../../../09-debugging/memory-bugs.md)
**Heisenbug là gì? Bug không tái hiện được thì làm sao?**
<details><summary>Đáp án</summary>

Heisenbug: bug đổi hành vi/biến mất khi quan sát (thêm log/debugger đổi timing) — dấu hiệu data race hoặc UB (uninitialized, out-of-bounds). Xử lý: thay quan sát thụ động (printf) bằng công cụ phát hiện chủ động — TSan cho data race, ASan/UBSan/Valgrind cho lỗi bộ nhớ/UB; chạy lặp nhiều lần, tăng tải, ép điều kiện biên; thu core dump khi crash. Đầu tiên cố làm cho bug tái hiện được.
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

#### DBG-014 · 🔴 · design · ⭐ · [→ tools](../../../09-debugging/tools.md), [kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Crash ngẫu nhiên xảy ra ở thiết bị ngoài field (không gdb được). Chiến lược?**
<details><summary>Đáp án</summary>

Kết hợp: (1) logging có cấu trúc với mức + timestamp + ngữ cảnh — vũ khí chính ở production. (2) Thu **core dump** (hoặc kdump/vmcore cho kernel) từ thiết bị về máy có symbol để phân tích offline. (3) Build cùng code trên host và chạy ASan/TSan/Valgrind để bắt lỗi tiềm ẩn. (4) Nếu là driver/kernel: đọc oops (RIP + Call Trace + addr2line), bật KASAN/LOCKDEP khi dev, tái hiện trong QEMU + kgdb nếu được.
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

---
⬅️ [Bank index](README.md)
