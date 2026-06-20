# Câu hỏi phỏng vấn — Debugging

> Topic gốc: [09 Debugging](../09-debugging/). Đây là phần phỏng vấn rất hay hỏi dạng tình huống ("bạn sẽ làm gì nếu..."). Tự trả lời trước.

---

## 🟢 Cơ bản

<details><summary>1) Vì sao build với -g và nên -O0 khi debug?</summary>

`-g` nhúng debug symbol (tên biến, kiểu, ánh xạ địa chỉ↔dòng) để gdb hiển thị source và in biến. `-O0` tắt tối ưu vì `-O2/-O3` inline/reorder/loại biến → step nhảy loạn, biến "optimized out". Khi buộc debug bản tối ưu, dùng `-Og`. → [gdb](../09-debugging/gdb.md)
</details>

<details><summary>2) step và next trong gdb khác nhau gì?</summary>

Cả hai chạy một dòng. `step` đi **vào trong** hàm được gọi; `next` **bước qua** (chạy hết hàm đó rồi dừng ở dòng kế). `finish` chạy tới khi hàm hiện tại return. → [gdb](../09-debugging/gdb.md)
</details>

<details><summary>3) Core dump là gì và dùng để làm gì?</summary>

Ảnh chụp bộ nhớ process lúc crash (stack, heap, register). Nạp vào gdb (`gdb ./app core`) để phân tích post-mortem (`bt`, `print`) mà không cần tái hiện — cực giá trị cho bug khó tái hiện hoặc crash ở field. Cần `ulimit -c unlimited`. → [gdb](../09-debugging/gdb.md)
</details>

---

## 🟡 Trung bình

<details><summary>4) Chương trình segfault. Bạn điều tra thế nào?</summary>

Chạy trong gdb tới khi crash rồi `bt` xem call stack tại điểm crash (lộ hàm lỗi + đường dẫn gọi), `frame`/`print`/`info locals` xem biến (null? index sai?). Nếu không chạy live được hoặc crash ở field, dùng core dump phân tích post-mortem. Hoặc build với AddressSanitizer để bắt chính xác chỗ truy cập sai + nơi cấp phát/giải phóng. → [gdb](../09-debugging/gdb.md), [memory-bugs](../09-debugging/memory-bugs.md)
</details>

<details><summary>5) Chương trình báo lỗi mơ hồ "không khởi động được". strace giúp gì?</summary>

`strace` ghi mọi syscall + kết quả. Thường thấy ngay `openat("/path/config", ...) = -1 ENOENT` (sai đường dẫn) hoặc `EACCES` (quyền), hoặc thiếu thư viện. Lớp bug "thiếu file/lib/quyền/mạng" giải quyết rất nhanh bằng strace vì nó cho thấy chương trình **thực sự** tìm gì. → [tools](../09-debugging/tools.md)
</details>

<details><summary>6) Chương trình chạy chậm. Tìm nguyên nhân thế nào?</summary>

Không đoán hotspot mà **đo** bằng perf: `perf stat` (tổng quan cycle, cache miss), `perf record -g` + `perf report` (hàm nào chiếm CPU — flame graph) để biết chỗ đáng tối ưu. Nếu chậm do chờ I/O chứ không phải CPU, perf cho thấy CPU rảnh → nhìn sang strace `-T` (thời gian mỗi syscall) hoặc lock contention. → [tools](../09-debugging/tools.md)
</details>

<details><summary>7) Memory leak: phát hiện thế nào?</summary>

AddressSanitizer/LeakSanitizer (`-fsanitize=address`) hoặc `valgrind --leak-check=full` — báo nơi cấp phát mà không giải phóng kèm stack. Theo dõi RSS (`/proc/<PID>/status`) qua thời gian để xác nhận rò rỉ. Phòng ngừa: RAII/smart pointer. → [memory-bugs](../09-debugging/memory-bugs.md)
</details>

---

## 🟠 Khó

<details><summary>8) Mô tả quy trình debug có hệ thống của bạn.</summary>

Phương pháp khoa học: (1) Reproduce ổn định, tối thiểu; (2) Isolate — thu hẹp bằng chia để trị (binary search trong code, git bisect, đơn giản hóa input); (3) Hypothesize giả thuyết cụ thể kiểm chứng được; (4) Test — đổi một biến mỗi lần, xác nhận/bác bỏ; (5) Fix nguyên nhân gốc (không phải triệu chứng, "5 whys"); (6) Verify + thêm test ngăn tái diễn. Xuyên suốt: tin dữ liệu hơn trực giác, kiểm tra giả định trước. → [mindset](../09-debugging/mindset.md)
</details>

<details><summary>9) Một biến "bỗng nhiên" có giá trị sai mà không rõ ai sửa. Làm sao tìm?</summary>

Dùng gdb **watchpoint** (`watch myVar`): gdb dừng ngay tại lệnh ghi vào biến, `bt` chỉ ra thủ phạm. Hiệu quả cho memory corruption/ghi đè ngoài ý muốn (vd buffer overflow ghi sang biến lân cận). Nếu nghi overflow rộng hơn, build với ASan để bắt chính xác. → [gdb](../09-debugging/gdb.md), [memory-bugs](../09-debugging/memory-bugs.md)
</details>

<details><summary>10) Chương trình bị treo (hang). Điều tra thế nào?</summary>

`gdb -p <PID>` attach vào process đang chạy, `thread apply all bt` in backtrace mọi thread. Deadlock: thấy hai thread cùng dừng trong hàm khóa mutex, mỗi cái chờ lock cái kia giữ (chờ vòng tròn). Treo do vòng lặp/chờ I/O: backtrace cho thấy thread chính kẹt ở đâu. Có thể dùng `pstack` nhanh. Sau đó truy ngược thứ tự khóa để sửa. → [gdb](../09-debugging/gdb.md)
</details>

<details><summary>11) ASan và Valgrind khác nhau? Khi nào dùng cái nào?</summary>

ASan cần build lại (chèn kiểm tra), nhanh (~2x), bắt stack/global overflow tốt, báo lỗi ngay khi xảy ra. Valgrind không cần build lại, rất tỉ mỉ với uninitialized reads, nhưng chậm (10–50x). Dùng ASan trong dev/CI; Valgrind khi chỉ có binary hoặc cần bắt loại lỗi ASan bỏ sót. → [memory-bugs](../09-debugging/memory-bugs.md)
</details>

---

## 🔴 Senior

<details><summary>12) Heisenbug là gì? Bug không tái hiện được thì làm sao?</summary>

Heisenbug: bug đổi hành vi/biến mất khi quan sát (thêm log/debugger đổi timing) — dấu hiệu của data race hoặc UB (uninitialized, out-of-bounds). Xử lý: thay quan sát thụ động (printf) bằng công cụ phát hiện chủ động không phụ thuộc may rủi — TSan cho data race, ASan/UBSan/Valgrind cho lỗi bộ nhớ/UB; chạy lặp nhiều lần, tăng tải, ép điều kiện biên; thu core dump khi crash. Đầu tiên cố làm cho bug tái hiện được. → [mindset](../09-debugging/mindset.md), [memory-bugs](../09-debugging/memory-bugs.md)
</details>

<details><summary>13) Làm sao phát hiện data race? Vì sao khó debug bằng cách thường?</summary>

ThreadSanitizer (`-fsanitize=thread`) theo dõi truy cập bộ nhớ + quan hệ happens-before, phát hiện hai thread truy cập cùng vùng (≥1 ghi) không đồng bộ — kể cả khi lần chạy đó chưa lỗi. Race khó debug vì không tất định (phụ thuộc timing) và thêm log/debugger làm đổi timing che bug (Heisenbug), nên printf không đáng tin. → [memory-bugs](../09-debugging/memory-bugs.md), [concurrency](../02-modern-cpp/concurrency.md)
</details>

<details><summary>14) Crash ngẫu nhiên xảy ra ở thiết bị ngoài field (không gdb được). Chiến lược?</summary>

Kết hợp: (1) logging có cấu trúc với mức + timestamp + ngữ cảnh để bắt tình huống khi nó xảy ra — vũ khí chính ở production. (2) Thu thập **core dump** (hoặc kdump/vmcore cho kernel) từ thiết bị về máy có symbol để phân tích offline. (3) Build cùng code trên host và chạy ASan/TSan/Valgrind để bắt lỗi tiềm ẩn. (4) Nếu là driver/kernel: đọc oops (RIP + Call Trace + addr2line), bật KASAN/LOCKDEP khi dev, tái hiện trong QEMU + kgdb nếu được. → [tools](../09-debugging/tools.md), [kernel-debugging](../09-debugging/kernel-debugging.md)
</details>

<details><summary>15) Kernel oops: bạn đọc thông tin gì để tìm nguyên nhân?</summary>

Dòng lỗi (loại + địa chỉ, vd NULL pointer deref), thanh ghi RIP/PC (hàm + offset đang chạy, vd `my_driver_read+0x2c [my_driver]` — biết module/hàm), Call Trace (chuỗi gọi dẫn tới lỗi), "Modules linked in" + cờ tainted. Dùng `addr2line -e my_driver.ko <offset>` (hoặc gdb trên .ko có debug info) map offset → dòng source, kết hợp loại lỗi truy nguyên nhân gốc. → [kernel-debugging](../09-debugging/kernel-debugging.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
