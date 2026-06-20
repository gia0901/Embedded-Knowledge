# Câu hỏi phỏng vấn — Operating System

> Topic gốc: [03 Operating System](../03-operating-system/). Tự trả lời trước khi mở đáp án.

---

## 🟢 Cơ bản

<details><summary>1) Process và thread khác nhau thế nào?</summary>

Process có không gian địa chỉ riêng và tài nguyên riêng, được cô lập. Thread là luồng thực thi trong process, chia sẻ code/data/heap/fd với thread cùng process nhưng có stack/register riêng. Thread tạo và giao tiếp rẻ (qua bộ nhớ chung) nhưng dễ data race và một thread lỗi sập cả process; process cô lập tốt nhưng tạo/giao tiếp đắt hơn. → [process-thread](../03-operating-system/process-thread.md)
</details>

<details><summary>2) Thread chia sẻ gì, có riêng gì?</summary>

Chia sẻ: code, data/bss, heap, file descriptor, signal handler. Riêng: stack, register (gồm PC, SP), errno, thread-local storage. → [process-thread](../03-operating-system/process-thread.md)
</details>

<details><summary>3) Deadlock là gì? Bốn điều kiện Coffman?</summary>

Các thread chờ vòng tròn tài nguyên do nhau giữ nên kẹt vĩnh viễn. Bốn điều kiện cần đủ: mutual exclusion, hold-and-wait, no preemption, circular wait. Phá một điều kiện là tránh được (phổ biến: lock theo thứ tự cố định). → [sync-primitives](../03-operating-system/sync-primitives.md)
</details>

<details><summary>4) Race condition là gì?</summary>

Kết quả phụ thuộc thứ tự/timing không kiểm soát giữa nhiều luồng truy cập dữ liệu chung (ít nhất một ghi). Vùng cần loại trừ lẫn nhau là critical section; bảo vệ bằng mutex hoặc atomic. → [sync-primitives](../03-operating-system/sync-primitives.md)
</details>

---

## 🟡 Trung bình

<details><summary>5) Context switch là gì? Vì sao switch process tốn hơn switch thread?</summary>

Là lưu trạng thái CPU (register, PC, SP) của tác vụ đang chạy và khôi phục của tác vụ kế. Switch giữa hai process còn phải đổi không gian địa chỉ (đổi page table) và thường flush TLB, làm cache/TLB lạnh → đắt. Thread cùng process dùng chung address space nên bỏ qua bước này. → [process-thread](../03-operating-system/process-thread.md)
</details>

<details><summary>6) Mutex và spinlock khác nhau? Khi nào dùng spinlock?</summary>

Khi chờ lock, mutex đưa thread vào ngủ (nhường CPU), spinlock bận xoay (busy-wait). Spinlock tránh chi phí context switch nên nhanh khi lock giữ cực ngắn và có nhiều core; nhưng đốt CPU nếu chờ lâu. Dùng spinlock trong kernel/SMP cho critical section vài lệnh, không bao giờ ngủ khi giữ spinlock. → [sync-primitives](../03-operating-system/sync-primitives.md)
</details>

<details><summary>7) Mutex và semaphore khác nhau?</summary>

Mutex có ownership (ai lock thì chính nó unlock), bảo vệ critical section. Semaphore là bộ đếm wait/signal không ownership, dùng báo hiệu giữa luồng hoặc quản N tài nguyên. Dùng binary semaphore thay mutex làm mất priority inheritance và dễ lỗi. → [sync-primitives](../03-operating-system/sync-primitives.md)
</details>

<details><summary>8) Virtual memory giải quyết vấn đề gì?</summary>

Cho mỗi process không gian địa chỉ ảo riêng được MMU ánh xạ tới vật lý: (1) bảo vệ/cô lập giữa các process; (2) đơn giản hóa — mỗi process thấy không gian liền mạch, tránh fragmentation ngoài; (3) overcommit — bộ nhớ ảo vượt RAM nhờ swap + demand paging. → [memory-management](../03-operating-system/memory-management.md)
</details>

<details><summary>9) Zombie và orphan process là gì?</summary>

Zombie: con đã kết thúc nhưng cha chưa `wait()` đọc exit status → entry còn trong bảng process. Orphan: cha chết trước con → con được init/systemd (PID 1) nhận nuôi và reap. → [process-thread](../03-operating-system/process-thread.md)
</details>

---

## 🟠 Khó

<details><summary>10) Page fault có phải luôn là lỗi không?</summary>

Không. Minor fault (page đã trong RAM nhưng chưa map, hoặc COW) chỉ cập nhật page table — nhanh; major fault (page trên disk/swap) phải đọc I/O — chậm; chỉ invalid fault (địa chỉ không hợp lệ) mới sinh SIGSEGV. Cơ chế này cho phép demand paging. → [memory-management](../03-operating-system/memory-management.md)
</details>

<details><summary>11) TLB là gì và vì sao quan trọng cho hiệu năng?</summary>

TLB (Translation Lookaside Buffer) là cache trong MMU lưu các ánh xạ page→frame gần đây. TLB hit cho dịch địa chỉ tức thì; miss buộc đi bộ qua multi-level page table (nhiều lần đọc RAM) rồi nạp TLB. Switch process thường phải flush TLB (nếu không có ASID), góp phần làm switch process đắt. → [memory-management](../03-operating-system/memory-management.md)
</details>

<details><summary>12) Condition variable: vì sao phải đi kèm mutex và predicate?</summary>

Đi kèm mutex vì điều kiện dựa trên dữ liệu chung cần bảo vệ; `wait` nhả mutex khi ngủ (để thread khác sửa điều kiện) và giành lại khi thức. Predicate (`wait(lock, pred)`) chống spurious wakeup (thức không do notify) và kiểm tra điều kiện trước/sau khi chờ. → [sync-primitives](../03-operating-system/sync-primitives.md)
</details>

<details><summary>13) Copy-on-write trong fork hoạt động thế nào?</summary>

Thay vì sao chép toàn bộ bộ nhớ khi fork, kernel cho cha/con cùng tham chiếu các page ở chế độ read-only; chỉ khi một bên **ghi** thì page đó mới được nhân bản. Nhờ vậy fork nhanh và tiết kiệm bộ nhớ, đặc biệt khi con gọi exec ngay sau. → [process-thread](../03-operating-system/process-thread.md)
</details>

<details><summary>14) Thrashing là gì?</summary>

Khi tổng working set của các process vượt RAM, hệ thống liên tục swap in/out và dành phần lớn thời gian cho I/O thay vì tính toán → hiệu năng sụp đổ. Khắc phục: giảm tải, thêm RAM, hoặc OOM killer kết thúc process ngốn bộ nhớ. → [memory-management](../03-operating-system/memory-management.md)
</details>

---

## 🔴 Senior

<details><summary>15) Priority inversion là gì? Khắc phục thế nào?</summary>

Tác vụ ưu tiên cao (H) bị chặn gián tiếp: tác vụ thấp (L) giữ mutex H cần, trong khi tác vụ trung bình (M) không liên quan preempt L, khiến L không nhả lock và H bị M chặn vô thời hạn. Khắc phục: priority inheritance (L tạm thừa kế ưu tiên H để nhả lock nhanh) hoặc priority ceiling. Sự cố nổi tiếng Mars Pathfinder. → [scheduling](../03-operating-system/scheduling.md)
</details>

<details><summary>16) CFS lập lịch theo nguyên lý nào?</summary>

CFS mô phỏng CPU đa nhiệm lý tưởng: theo dõi vruntime (thời gian CPU ảo có trọng số theo nice) của mỗi tác vụ và luôn chọn chạy tác vụ có vruntime nhỏ nhất (dùng cây đỏ-đen). Tác vụ nice thấp (ưu tiên cao) có vruntime tăng chậm nên được nhiều CPU hơn → chia sẻ công bằng theo trọng số. → [scheduling](../03-operating-system/scheduling.md)
</details>

<details><summary>17) Khi nào dùng nhiều process thay vì nhiều thread cho một ứng dụng?</summary>

Dùng process khi cần cô lập/độ tin cậy cao (một thành phần crash không kéo sập phần khác — vd browser tách tab), thành phần độc lập hoặc cần quyền hạn/bảo mật khác nhau, fault containment mạnh. Dùng thread khi cần chia sẻ dữ liệu lớn thường xuyên, song song trong cùng ứng dụng, giảm overhead. Đánh đổi: cô lập & an toàn vs nhẹ & chia sẻ nhanh. → [process-thread](../03-operating-system/process-thread.md)
</details>

<details><summary>18) Thiết kế producer–consumer an toàn giữa các thread thế nào?</summary>

Dùng một hàng đợi chia sẻ được bảo vệ bằng mutex, cộng condition variable: consumer `cv.wait(lock, []{return !queue.empty();})` (nhả lock khi ngủ, predicate chống spurious wakeup); producer khóa mutex, push, rồi `notify_one`. Cân nhắc bounded queue (thêm cv cho "không đầy") để chống tràn bộ nhớ, và `notify_all` khi nhiều consumer. Trên hệ một-biến đơn giản có thể dùng lock-free queue + atomic nhưng khó đúng. → [sync-primitives](../03-operating-system/sync-primitives.md), [concurrency](../02-modern-cpp/concurrency.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
