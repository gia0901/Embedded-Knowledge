# OS — Operating System

> Domain `OS`. Gộp từ [11/operating-system.md](../../../11-interview-questions/operating-system.md) + [technical_round/02 PHẦN 2](../../technical_round/02_question_bank.md). Track dùng: `os`, `bsp`, `cpp-system`, `ostep`, `cpp-concurrency`.

---

#### OS-001 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Process và thread khác nhau thế nào?**
<details><summary>Đáp án</summary>

Process có không gian địa chỉ riêng và tài nguyên riêng, được cô lập. Thread là luồng thực thi trong process, chia sẻ code/data/heap/fd với thread cùng process nhưng có stack/register riêng. Thread tạo và giao tiếp rẻ (qua bộ nhớ chung) nhưng dễ data race và một thread lỗi sập cả process; process cô lập tốt nhưng tạo/giao tiếp đắt hơn.
</details>

#### OS-002 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Thread chia sẻ gì, có riêng gì?**
<details><summary>Đáp án</summary>

Chia sẻ: code, data/bss, heap, file descriptor, signal handler. Riêng: stack, register (gồm PC, SP), errno, thread-local storage.
</details>

#### OS-003 · 🟡 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Deadlock là gì? Bốn điều kiện Coffman? Cách tránh?**
<details><summary>Đáp án</summary>

Các thread chờ vòng tròn tài nguyên do nhau giữ nên kẹt vĩnh viễn. Bốn điều kiện cần đủ: mutual exclusion, hold-and-wait, no preemption, circular wait. Phá một điều kiện là tránh được — phổ biến nhất: **lock ordering** (luôn khóa nhiều mutex theo cùng thứ tự) để phá circular wait; hoặc `try_lock`+timeout, `scoped_lock`.
</details>

#### OS-004 · 🟢 · concept · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Race condition là gì?**
<details><summary>Đáp án</summary>

Kết quả phụ thuộc thứ tự/timing không kiểm soát giữa nhiều luồng truy cập dữ liệu chung (ít nhất một ghi). Vùng cần loại trừ lẫn nhau là critical section; bảo vệ bằng mutex hoặc atomic.
</details>

#### OS-005 · 🟡 · concept · ⭐ · [→ process-thread](../../../03-operating-system/process-thread.md)
**Context switch là gì? Vì sao switch process tốn hơn switch thread?**
<details><summary>Đáp án</summary>

Là lưu trạng thái CPU (register, PC, SP) của tác vụ đang chạy và khôi phục của tác vụ kế. Switch giữa hai process còn phải đổi không gian địa chỉ (đổi page table) và thường flush TLB, làm cache/TLB lạnh → đắt. Thread cùng process dùng chung address space nên bỏ qua bước này. *(Câu xuyên-topic OS+memory.)*
</details>

#### OS-006 · 🟡 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Mutex và spinlock khác nhau? Khi nào dùng spinlock?**
<details><summary>Đáp án</summary>

Khi chờ lock, mutex đưa thread vào ngủ (nhường CPU), spinlock bận xoay (busy-wait). Spinlock tránh chi phí context switch nên nhanh khi lock giữ cực ngắn và có nhiều core; nhưng đốt CPU nếu chờ lâu. Dùng spinlock trong kernel/SMP cho critical section vài lệnh, không bao giờ ngủ khi giữ spinlock.
</details>

#### OS-007 · 🟡 · concept · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Mutex và semaphore khác nhau?**
<details><summary>Đáp án</summary>

Mutex có ownership (ai lock thì chính nó unlock), bảo vệ critical section. Semaphore là bộ đếm wait/signal không ownership, dùng báo hiệu giữa luồng hoặc quản N tài nguyên. Dùng binary semaphore thay mutex làm mất priority inheritance và dễ lỗi.
</details>

#### OS-008 · 🟡 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Virtual memory giải quyết vấn đề gì?**
<details><summary>Đáp án</summary>

Cho mỗi process không gian địa chỉ ảo riêng được MMU ánh xạ tới vật lý: (1) bảo vệ/cô lập giữa các process; (2) đơn giản hóa — mỗi process thấy không gian liền mạch, tránh fragmentation ngoài; (3) overcommit — bộ nhớ ảo vượt RAM nhờ swap + demand paging.
</details>

#### OS-009 · 🟡 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Zombie và orphan process là gì?**
<details><summary>Đáp án</summary>

Zombie: con đã kết thúc nhưng cha chưa `wait()` đọc exit status → entry còn trong bảng process; dọn bằng `wait()/waitpid()`. Orphan: cha chết trước con → con được init/systemd (PID 1) nhận nuôi và reap.
</details>

#### OS-010 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Page fault có phải luôn là lỗi không?**
<details><summary>Đáp án</summary>

Không. Minor fault (page đã trong RAM nhưng chưa map, hoặc COW) chỉ cập nhật page table — nhanh; major fault (page trên disk/swap) phải đọc I/O — chậm; chỉ invalid fault (địa chỉ không hợp lệ) mới sinh SIGSEGV. Cơ chế này cho phép demand paging.
</details>

#### OS-011 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**TLB là gì và vì sao quan trọng cho hiệu năng?**
<details><summary>Đáp án</summary>

TLB (Translation Lookaside Buffer) là cache trong MMU lưu các ánh xạ page→frame gần đây. TLB hit cho dịch địa chỉ tức thì; miss buộc đi bộ qua multi-level page table (nhiều lần đọc RAM) rồi nạp TLB. Switch process thường phải flush TLB (nếu không có ASID), góp phần làm switch process đắt.
</details>

#### OS-012 · 🟠 · concept · ⭐ · [→ sync-primitives](../../../03-operating-system/sync-primitives.md)
**Condition variable: vì sao phải đi kèm mutex và predicate (dùng while)?**
<details><summary>Đáp án</summary>

Đi kèm mutex vì điều kiện dựa trên dữ liệu chung cần bảo vệ; `wait` nhả mutex khi ngủ (để thread khác sửa điều kiện) và giành lại khi thức. Predicate (`wait(lock, pred)`) chống **spurious wakeup** (thức không do notify) và race giữa notify/wait — kiểm tra trong vòng lặp đảm bảo chỉ tiếp tục khi điều kiện thật sự đúng.
</details>

#### OS-013 · 🟠 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**Copy-on-write trong fork hoạt động thế nào?**
<details><summary>Đáp án</summary>

Thay vì sao chép toàn bộ bộ nhớ khi fork, kernel cho cha/con cùng tham chiếu các page ở chế độ read-only; chỉ khi một bên **ghi** thì page đó mới được nhân bản. Nhờ vậy fork nhanh và tiết kiệm bộ nhớ, đặc biệt khi con gọi exec ngay sau.
</details>

#### OS-014 · 🟠 · concept · [→ memory-management](../../../03-operating-system/memory-management.md)
**Thrashing là gì?**
<details><summary>Đáp án</summary>

Khi tổng working set của các process vượt RAM, hệ thống liên tục swap in/out và dành phần lớn thời gian cho I/O thay vì tính toán → hiệu năng sụp đổ. Khắc phục: giảm tải, thêm RAM, hoặc OOM killer kết thúc process ngốn bộ nhớ.
</details>

#### OS-015 · 🔴 · concept · ⭐ · [→ scheduling](../../../03-operating-system/scheduling.md)
**Priority inversion là gì? Priority inheritance chữa thế nào, và vì sao semaphore không chữa được?**
<details><summary>Đáp án</summary>

- **Inversion:** L (thấp) giữ mutex; H (cao) chờ mutex đó; M (trung, không liên quan) chiếm CPU của L → L không chạy để nhả → H chờ vô chừng — ưu tiên lộn ngược. Sự cố nổi tiếng **Mars Pathfinder** reset liên tục vì kịch bản này.
- **Priority inheritance:** khi H block, chủ mutex (L) **tạm nhận priority của H** → vượt M, chạy xong nhả, tụt về cũ → thời gian H chờ chặn trên = critical section của L. (Biến thể: priority ceiling.)
- **Semaphore không PI được** vì PI cần biết **chủ để nâng**; semaphore không có ownership (A wait, B post đều hợp lệ). Đây là lý do kỹ thuật của quy tắc "mutex cho mutual exclusion, semaphore cho signaling" trong hệ RT.
- Thực chiến: pthread mutex phải khai `PTHREAD_PRIO_INHERIT` tường minh (mặc định NONE).
</details>

#### OS-016 · 🔴 · concept · [→ scheduling](../../../03-operating-system/scheduling.md)
**CFS lập lịch theo nguyên lý nào?**
<details><summary>Đáp án</summary>

CFS mô phỏng CPU đa nhiệm lý tưởng: theo dõi vruntime (thời gian CPU ảo có trọng số theo nice) của mỗi tác vụ và luôn chọn chạy tác vụ có vruntime nhỏ nhất (dùng cây đỏ-đen). Tác vụ nice thấp (ưu tiên cao) có vruntime tăng chậm nên được nhiều CPU hơn → chia sẻ công bằng theo trọng số.
</details>

#### OS-017 · 🔴 · design · [→ process-thread](../../../03-operating-system/process-thread.md)
**Khi nào dùng nhiều process thay vì nhiều thread cho một ứng dụng?**
<details><summary>Đáp án</summary>

Dùng process khi cần cô lập/độ tin cậy cao (một thành phần crash không kéo sập phần khác — vd browser tách tab), thành phần độc lập hoặc cần quyền hạn/bảo mật khác nhau, fault containment mạnh. Dùng thread khi cần chia sẻ dữ liệu lớn thường xuyên, song song trong cùng ứng dụng, giảm overhead. Đánh đổi: cô lập & an toàn vs nhẹ & chia sẻ nhanh.
</details>

#### OS-018 · 🔴 · design · [→ sync-primitives](../../../03-operating-system/sync-primitives.md), [concurrency](../../../02-modern-cpp/concurrency.md)
**Thiết kế producer–consumer an toàn giữa các thread thế nào?**
<details><summary>Đáp án</summary>

Dùng một hàng đợi chia sẻ được bảo vệ bằng mutex, cộng condition variable: consumer `cv.wait(lock, []{return !queue.empty();})` (nhả lock khi ngủ, predicate chống spurious wakeup); producer khóa mutex, push, rồi `notify_one`. Cân nhắc bounded queue (thêm cv cho "không đầy") để chống tràn bộ nhớ, và `notify_all` khi nhiều consumer. Trên hệ đơn giản có thể dùng lock-free queue + atomic nhưng khó đúng.
</details>

#### OS-019 · 🟢 · concept · [→ process-thread](../../../03-operating-system/process-thread.md)
**`fork()` trả về gì?**
<details><summary>Đáp án</summary>

Trả **0 cho process con**, **PID của con cho process cha**, **-1 nếu lỗi**. Không gian địa chỉ được copy-on-write.
</details>

---
⬅️ [Bank index](README.md)
