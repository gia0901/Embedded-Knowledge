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

Dùng gdb **watchpoint** (`watch myVar`): gdb dừng ngay tại lệnh ghi vào biến, `bt` chỉ ra thủ phạm. Hiệu quả cho memory corruption/ghi đè ngoài ý muốn (vd buffer overflow ghi sang biến lân cận). Nếu nghi overflow rộng hơn, build với ASan để bắt chính xác.
</details>

#### DBG-010 · 🟠 · concept · [→ gdb](../../../09-debugging/gdb.md)
**Chương trình bị treo (hang). Điều tra thế nào?**
<details><summary>Đáp án</summary>

`gdb -p <PID>` attach vào process đang chạy, `thread apply all bt` in backtrace mọi thread. Deadlock: thấy hai thread cùng dừng trong hàm khóa mutex, mỗi cái chờ lock cái kia giữ. Treo do vòng lặp/chờ I/O: backtrace cho thấy thread chính kẹt ở đâu. Có thể dùng `pstack` nhanh. Sau đó truy ngược thứ tự khóa để sửa.
</details>

#### DBG-011 · 🟠 · concept · [→ memory-bugs](../../../09-debugging/memory-bugs.md)
**ASan và Valgrind khác nhau? Khi nào dùng cái nào?**
<details><summary>Đáp án</summary>

ASan cần build lại (chèn kiểm tra), nhanh (~2x), bắt stack/global overflow tốt, báo lỗi ngay khi xảy ra. Valgrind không cần build lại, rất tỉ mỉ với uninitialized reads, nhưng chậm (10–50x). Dùng ASan trong dev/CI; Valgrind khi chỉ có binary hoặc cần bắt loại lỗi ASan bỏ sót.
</details>

#### DBG-012 · 🔴 · concept · [→ mindset](../../../09-debugging/mindset.md), [memory-bugs](../../../09-debugging/memory-bugs.md)
**Heisenbug là gì? Bug không tái hiện được thì làm sao?**
<details><summary>Đáp án</summary>

Heisenbug: bug đổi hành vi/biến mất khi quan sát (thêm log/debugger đổi timing) — dấu hiệu data race hoặc UB (uninitialized, out-of-bounds). Xử lý: thay quan sát thụ động (printf) bằng công cụ phát hiện chủ động — TSan cho data race, ASan/UBSan/Valgrind cho lỗi bộ nhớ/UB; chạy lặp nhiều lần, tăng tải, ép điều kiện biên; thu core dump khi crash. Đầu tiên cố làm cho bug tái hiện được.
</details>

#### DBG-013 · 🔴 · concept · [→ memory-bugs](../../../09-debugging/memory-bugs.md), [concurrency](../../../02-modern-cpp/concurrency.md)
**Làm sao phát hiện data race? Vì sao khó debug bằng cách thường?**
<details><summary>Đáp án</summary>

ThreadSanitizer (`-fsanitize=thread`) theo dõi truy cập bộ nhớ + quan hệ happens-before, phát hiện hai thread truy cập cùng vùng (≥1 ghi) không đồng bộ — kể cả khi lần chạy đó chưa lỗi. Race khó debug vì không tất định (phụ thuộc timing) và thêm log/debugger làm đổi timing che bug (Heisenbug), nên printf không đáng tin.
</details>

#### DBG-014 · 🔴 · design · ⭐ · [→ tools](../../../09-debugging/tools.md), [kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Crash ngẫu nhiên xảy ra ở thiết bị ngoài field (không gdb được). Chiến lược?**
<details><summary>Đáp án</summary>

Kết hợp: (1) logging có cấu trúc với mức + timestamp + ngữ cảnh — vũ khí chính ở production. (2) Thu **core dump** (hoặc kdump/vmcore cho kernel) từ thiết bị về máy có symbol để phân tích offline. (3) Build cùng code trên host và chạy ASan/TSan/Valgrind để bắt lỗi tiềm ẩn. (4) Nếu là driver/kernel: đọc oops (RIP + Call Trace + addr2line), bật KASAN/LOCKDEP khi dev, tái hiện trong QEMU + kgdb nếu được.
</details>

#### DBG-015 · 🔴 · concept · ⭐ · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Kernel oops: bạn đọc thông tin gì để tìm nguyên nhân?**
<details><summary>Đáp án</summary>

Dòng lỗi (loại + địa chỉ, vd NULL pointer deref), thanh ghi RIP/PC (hàm + offset đang chạy, vd `my_driver_read+0x2c [my_driver]` — biết module/hàm), Call Trace (chuỗi gọi dẫn tới lỗi), "Modules linked in" + cờ tainted. Dùng `addr2line -e my_driver.ko <offset>` (hoặc gdb trên .ko có debug info) map offset → dòng source, kết hợp loại lỗi truy nguyên nhân gốc.
</details>

#### DBG-016 · 🟠 · design · ⭐ · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Bạn debug một lỗi cross-layer thế nào?**
<details><summary>Đáp án</summary>

Lần theo dữ liệu qua từng tầng: user-space (log/GDB xác nhận giá trị gửi) → ranh giới (ioctl có xuống driver không) → kernel (`dmesg`/`printk` xem driver nhận gì) → tìm gốc (vd sai offset thanh ghi/`copy_from_user`) → fix → verify lại từ app xuống driver. Không đoán, luôn xác nhận ở mỗi tầng.
</details>

#### DBG-017 · 🟢 · concept · ⭐ · [→ gdb](../../../09-debugging/gdb.md)
**Các lệnh GDB cốt lõi?**
<details><summary>Đáp án</summary>

`break` đặt breakpoint, `run`, `next`/`step` (không/có vào hàm), `continue`, `finish`, `print` in biến, **`backtrace`/`bt`** in call stack, `frame N`, `info locals`, **`watch`** dừng khi biến đổi giá trị.
</details>

#### DBG-018 · 🟡 · concept · ⭐ · [→ gdb](../../../09-debugging/gdb.md)
**Debug binary chạy trên thiết bị target không có GDB đầy đủ thì sao?**
<details><summary>Đáp án</summary>

Dùng **remote debug**: chạy `gdbserver :PORT ./prog` trên target, GDB đầy đủ trên host kết nối `target remote ip:port`. Debug trên phần cứng thật mà không cần cài cả GDB lên thiết bị.
</details>

#### DBG-019 · 🟡 · concept · [→ kernel-debugging](../../../09-debugging/kernel-debugging.md)
**Phân biệt kernel oops và panic?**
<details><summary>Đáp án</summary>

Oops: lỗi nghiêm trọng nhưng kernel cố tiếp tục (kill process lỗi). Panic: không thể tiếp tục, dừng hệ thống. Cả hai in stack trace ra `dmesg`.
</details>

---
⬅️ [Bank index](README.md)
