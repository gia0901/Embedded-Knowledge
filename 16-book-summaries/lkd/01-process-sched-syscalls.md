# Ch. 3–5 — Process, Scheduler & System Calls (tr. 23–83)

> Thuộc [LKD](README.md) · **⏮ (đầu — file 01)** · **[02 Interrupts & Bottom Halves → ⏭](02-interrupts-bottomhalves.md)**
> Nguồn: **đọc trực tiếp PDF** (Linux Kernel Development, Robert Love, 3rd ed. 2010), **trang sách = trang PDF − 27**.
> Ký hiệu: không đánh dấu = nội dung sách · **🆕 = bổ sung/liên hệ ngoài sách của người viết** · **⚠️ = điểm cần cẩn trọng / sách lỗi thời** · **🎯 = trọng tâm phỏng vấn** · trích dẫn kèm `(tr. X)`.
> File này gộp **ba chương liền mạch** — chương 3 (process là gì, kernel lưu nó ra sao), chương 4 (scheduler chọn process nào chạy), chương 5 (userspace gọi vào kernel bằng cách nào). Ba chương này trả lời câu hỏi nền: *"một tiến trình sống trong kernel trông như thế nào, từ lúc sinh ra, khi được xếp lịch, đến khi gọi dịch vụ của kernel."*
> 🆕 Lý thuyết process/scheduling ở góc OS tổng quát đã có ở [ostep/virtualization-cpu.md](../ostep/virtualization-cpu.md) — bạn **không cần** đọc nó để theo file này; ở đây ta xem **Linux hiện thực hóa** các khái niệm đó ra struct nào, đường code nào.

---

## Cụm 1 — Process trong kernel: task_struct, fork, thread (ch. 3, tr. 23–40)

### 1.1 Process là gì, và vì sao Linux "không có thread" (tr. 23–24)

**Bức tranh trước:** một **process** không chỉ là đoạn code đang chạy. Sách định nghĩa (tr. 23): *"A process is a program... in the midst of execution"* — nhưng nó **kèm theo cả một bộ tài nguyên**: file đang mở, signal đang chờ, trạng thái processor, một **address space** (không gian địa chỉ ảo) với các memory mapping, một hay nhiều **thread of execution**, và vùng dữ liệu global. Nói gọn: *"Processes... are the living result of running program code"* (tr. 23).

**Thread** (tr. 23) là **đối tượng hoạt động bên trong process** — mỗi thread có program counter riêng, stack riêng, tập register riêng. Câu quan trọng nhất của cả mục, đặt nền cho toàn chương: *"The kernel schedules individual threads, not processes"* — **kernel xếp lịch cho thread, không phải process**. Và:

> *"Linux has a unique implementation of threads: It does not differentiate between threads and processes. To Linux, a thread is just a special kind of process."* (tr. 23)

🆕 Đây là ý **phân loại kiến thức nền** hay bị hỏi: với Linux, thread và process **cùng một loại đối tượng** (`task_struct`) — chúng chỉ khác nhau ở chỗ *chia sẻ bao nhiêu tài nguyên*. Ta sẽ thấy điều này thành sự thật cụ thể ở mục 1.5 (thread = `clone()` với cờ chia sẻ).

Process cung cấp **hai ảo hóa** (tr. 23): *virtualized processor* (ảo giác độc chiếm CPU — chính là việc của chương 4) và *virtual memory* (ảo giác độc chiếm RAM — chương 12, ở [04-memory.md](04-memory.md)). Điểm tinh tế đáng nhớ (tr. 24): *"threads share the virtual memory abstraction, whereas each receives its own virtualized processor"* — **các thread chung address space nhưng mỗi thread có CPU ảo riêng**.

> **Note của sách (tr. 24):** tên khác của process trong kernel là **task**. Sách dùng lẫn lộn hai từ; "task" nghiêng về "process nhìn từ phía kernel". Trong toàn bộ code kernel bạn sẽ chỉ thấy `task_struct`, `task_list`, `current` — không thấy chữ "process".

### 1.2 task_struct — "thẻ căn cước" của mỗi process (tr. 24–26)

Kernel giữ mọi process trong một **danh sách liên kết đôi vòng** (circular doubly linked list) gọi là **task list** (tr. 24). Mỗi phần tử là một **process descriptor** kiểu `struct task_struct`, khai trong `<linux/sched.h>`. Đây là "PCB (Process Control Block) của Linux" — chứa **tất cả** thông tin kernel cần về một process.

Kích thước: *"around 1.7 kilobytes on a 32-bit machine"* (tr. 24) — sách nhấn: **nhỏ đến bất ngờ** so với lượng thông tin nó chứa (state, address space, open files, pending signals, và nhiều hơn).

**task_struct được cấp phát ở đâu?** (tr. 25) Qua **slab allocator** — bộ cấp phát chuyên tái dùng object cùng loại (chi tiết ở [04-memory.md](04-memory.md)). Vì sao slab? Vì fork xảy ra **rất dày** → cần cấp/thu `task_struct` cực nhanh và tận dụng cache; đây đúng bài toán slab sinh ra để giải.

**Tìm "task đang chạy" — macro `current` và `thread_info` (tr. 25–26):** kernel cần liên tục hỏi *"process nào đang chạy trên CPU này?"* Trước 2.6, `task_struct` nằm ngay cuối kernel stack. Nay nó cấp động qua slab, nên người ta để một struct nhỏ **`thread_info`** ở đáy stack, và `thread_info` trỏ tới `task_struct`:

```c
struct thread_info {
        struct task_struct  *task;          // ← con trỏ tới task_struct thật
        struct exec_domain  *exec_domain;
        __u32               flags;
        __u32               status;
        __u32               cpu;
        int                 preempt_count;  // ← đếm mức "cấm bị chen" (dùng ở cụm 2)
        ...
};
```

Trên x86 (ít register để phung phí), `current` được tính bằng cách **che các bit thấp của stack pointer** để ra địa chỉ `thread_info`, rồi lấy `->task` (tr. 26):

```asm
movl $-8192, %eax        ; giả định stack 8KB → mask 13 bit thấp của %esp
andl %esp, %eax          ; %eax giờ trỏ tới thread_info của task hiện tại
```

```c
current_thread_info()->task;   // rồi deref ->task ra task_struct
```

> 🆕 Đối chiếu kiến trúc: PowerPC dư register nên **cất thẳng `current` vào register r2** (tr. 26) → `current` chỉ là đọc register. Ý bất biến qua mọi arch: **luôn lấy được task của mình trong O(1)**. ⚠️ Kernel hiện đại x86-64/arm64 đã bỏ cách "mask stack pointer", chuyển `current` vào **per-CPU variable / register chuyên dụng** — nhưng tinh thần y hệt.

### 1.3 Năm trạng thái của process (tr. 27–28)

Trường `state` cho biết process đang ở đâu trong vòng đời (Figure 3.3, tr. 28). Mỗi process ở **đúng một** trong năm state:

| State | Nghĩa | Ghi nhớ |
|---|---|---|
| **TASK_RUNNING** | Runnable: *đang chạy* **hoặc** *nằm trên runqueue chờ tới lượt* | ⚠️ Linux **gộp** "đang chạy" và "sẵn sàng chạy" vào một state — đây là điểm hay nhầm. Đây là state duy nhất của code chạy ở user-space |
| **TASK_INTERRUPTIBLE** | Ngủ (blocked), chờ một điều kiện. Điều kiện tới → kernel đặt lại `TASK_RUNNING`. **Signal cũng đánh thức sớm** | Kiểu ngủ mặc định, đúng |
| **TASK_UNINTERRUPTIBLE** | Y hệt trên **nhưng KHÔNG dậy khi có signal** | Dùng khi phải chờ trọn vẹn, không thể dở dang; xem interview Câu 1 |
| **__TASK_TRACED** | Đang bị process khác trace (debugger qua `ptrace`) | |
| **__TASK_STOPPED** | Dừng thực thi, không chạy cũng không đủ tư cách chạy | Do nhận SIGSTOP/SIGTSTP/SIGTTIN/SIGTTOU |

Nguyên văn về UNINTERRUPTIBLE (tr. 28): *"it does not wake up and become runnable if it receives a signal. This is used in situations where the process must wait without interruption or when the event is expected to occur quite quickly."* Và chú thích chân trang tr. 28 chính là nguồn gốc câu hỏi kinh điển: *"This is why you have those dreaded unkillable processes with state D in ps(1)... you cannot send it a SIGKILL signal"* — **process kẹt `D` không kill được**.

**Đổi state đúng cách (tr. 29):**
```c
set_task_state(task, state);      // đặt state; kèm memory barrier ép ordering trên SMP
set_current_state(state);         // = set_task_state(current, state)
```
🆕 Vì sao cần barrier ở đây thì phải đọc cụm 2 (03-sync) — đặt state rồi mới kiểm tra điều kiện là mấu chốt chống lost-wakeup (mục 2.5).

### 1.4 Process context & cây gia phả (tr. 29–30)

**Process context (tr. 29)** — khái niệm phải thuộc, đối lập với *interrupt context* (file 02): code user chạy ở user-space; khi nó gọi **system call** (cụm 3) hoặc gây **exception**, nó **bước vào kernel-space**. Lúc đó *"the kernel is said to be 'executing on behalf of the process' and is in process context. When in process context, the current macro is valid."* → **Trong process context, kernel chạy THAY MẶT một task cụ thể; `current` có nghĩa; được ngủ, được schedule.** (Trái ngược: interrupt context không có task đứng sau — file 02.)

Câu chốt về ranh giới (tr. 29): *"A process can begin executing in kernel-space only through... system calls and exception handlers"* — **chỉ hai cửa đó** vào kernel, không có cửa thứ ba.

**Cây process (tr. 29–30):** mọi process là hậu duệ của **init** (PID 1), tiến trình kernel khởi động ở bước cuối của boot. Mỗi process có **đúng một parent** và **0 hoặc nhiều children**; con cùng parent là **siblings**. Quan hệ lưu ngay trong descriptor:
```c
struct task_struct *my_parent = current->parent;      // lên cha
// duyệt các con:
struct task_struct *task;
struct list_head *list;
list_for_each(list, &current->children) {
        task = list_entry(list, struct task_struct, sibling);
        /* task giờ trỏ tới một con của current */
}
```
🆕 `list_for_each`/`list_entry` là API của `list_head` (ch. 6 — kernel data structures); ý là "từ một `list_head` nhúng trong struct, lần ngược ra struct chứa nó". Có macro `for_each_process(task)` duyệt **toàn bộ** task list — kèm **Caution của sách** (tr. 30): duyệt hết mọi task rất đắt, đừng làm nếu không thật cần.

### 1.5 fork, Copy-on-Write, và thread = clone (tr. 31–35)

**Unix tách đôi việc tạo process (tr. 31):** khác đa số OS (một lệnh `spawn`), Unix chia làm hai — `fork()` tạo bản sao của process gọi (chỉ khác PID, PPID, vài tài nguyên/thống kê không kế thừa), rồi `exec()` mới nạp chương trình mới vào address space và chạy. `fork()` **trả về hai lần** (tr. 24): một lần ở cha, một lần ở con.

**Copy-on-Write — tối ưu cốt lõi (tr. 31):** copy toàn bộ tài nguyên khi fork là *"naive and inefficient"*, và tệ hơn nếu con `exec()` ngay thì mọi copy đổ sông. Linux hiện thực fork bằng **COW**:

> *"Rather than duplicate the process address space, the parent and the child can share a single copy. The data, however, is marked in such a way that if it is written to, a duplicate is made and each process receives a unique copy."* (tr. 31)

→ Cha con **dùng chung** các trang, đánh dấu read-only; ai **ghi** vào trang nào thì trang đó mới được nhân bản. Nếu con `exec()` ngay (trang chưa kịp bị ghi) thì **không trang nào phải copy**. Chi phí duy nhất của `fork()`: *"the duplication of the parent's page tables and the creation of a unique process descriptor for the child"* (tr. 31). Đây là lý do triết lý Unix cổ vũ "tạo process nhanh".

**`fork()` thực chất gọi `clone()` → `do_fork()` → `copy_process()` (tr. 32).** Sách liệt kê 8 bước của `copy_process()` — đáng đọc để thấy "con tự tách khỏi cha" cụ thể ra sao:

1. `dup_task_struct()` — tạo kernel stack, `thread_info`, `task_struct` mới; **giá trị y hệt cha** (lúc này cha con giống nhau như đúc).
2. Kiểm tra không vượt giới hạn số process của user.
3. Con phải **khác cha**: nhiều trường bị xóa/đặt lại (chủ yếu là thông tin thống kê); **phần lớn `task_struct` giữ nguyên**.
4. State con đặt `TASK_UNINTERRUPTIBLE` để **chưa chạy vội**.
5. `copy_flags()` — xóa `PF_SUPERPRIV` (đã dùng quyền root?), đặt `PF_FORKNOEXEC` (chưa gọi exec).
6. `alloc_pid()` — cấp PID mới.
7. **Tùy cờ `clone()`**: hoặc **copy** hoặc **chia sẻ** open files, filesystem info, signal handlers, address space, namespace. (Đây chính là chỗ "thread hay process" được quyết định — xem dưới.)
8. Dọn dẹp, trả con trỏ tới con.

**Kernel chạy con TRƯỚC (tr. 32):** *"Deliberately, the kernel runs the child process first."* Vì sao? Trường hợp phổ biến là con `exec()` ngay — chạy con trước thì **triệt tiêu COW-fault vô ích** mà cha sẽ gây ra nếu chạy trước và bắt đầu ghi vào address space. ⚠️ Chú thích chân trang tr. 32 thành thật: *"This does not currently function correctly, although the goal is for the child to run first"* — thời điểm sách in, cơ chế này chưa chạy đúng như ý.

**Thread = process chia sẻ tài nguyên (tr. 33–34).** Đây là hiện thực cụ thể của lời hứa ở mục 1.1. Với Linux *"there is no concept of a thread. Linux implements all threads as standard processes"*. Tạo thread = gọi `clone()` với các cờ **chia sẻ**:
```c
clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND, 0);   // → "thread"
// chia sẻ: address space, filesystem info, file descriptors, signal handlers
```
So sánh (tr. 34):
```c
clone(SIGCHLD, 0);                              // = fork() thường (không chia sẻ gì)
clone(CLONE_VFORK | CLONE_VM | SIGCHLD, 0);     // = vfork()
```
🆕 So với Windows/Solaris gọi thread là *"lightweight process"* (tr. 34): các OS đó xem thread là đơn vị nhẹ hơn process nặng; còn Linux xem **thread chỉ là cách chia sẻ tài nguyên giữa các process vốn đã rất nhẹ**. Bảng 3.1 (tr. 35) liệt kê các cờ `CLONE_*` (VM, FS, FILES, SIGHAND, THREAD, NEWNS, VFORK, SETTLS...).

**Kernel threads (tr. 35–36):** process chỉ sống trong kernel-space, làm việc nền (ví dụ `ksoftirqd`, các flush task). Khác biệt duy nhất so với process thường: **không có address space** — *"Their mm pointer... is NULL"*. Chúng vẫn schedulable và preemptable như mọi task. Tạo bằng:
```c
struct task_struct *kthread_create(int (*threadfn)(void *data),
                                   void *data, const char namefmt[], ...);
// tạo ở trạng thái CHƯA chạy → phải wake_up_process() mới chạy
struct task_struct *kthread_run(...);   // = kthread_create + wake_up_process (chạy luôn)
```
Kernel thread sống đến khi tự `do_exit()` hoặc bị `kthread_stop(k)` (tr. 36). 🆕 `mm == NULL` là dấu hiệu nhận biết kernel thread — và là lý do kernel thread không `copy_*_user` được (không có user address space nào để chép sang).

### 1.6 Cái chết của process: exit → zombie → reap (tr. 36–39)

**`do_exit()` (tr. 36–37)** làm phần lớn việc khi process kết thúc (dù tự `exit()` hay bị signal giết). 9 bước — điểm cốt lõi là **giải phóng dần tài nguyên nhưng chưa xóa hẳn descriptor**:

- (1) đặt cờ `PF_EXITING`; (2) `del_timer_sync()` gỡ mọi kernel timer (đảm bảo không handler timer nào còn chạy — nối với file 03); (4) `exit_mm()` trả `mm_struct` (hủy address space nếu không ai dùng chung); (6) `exit_files()`/`exit_fs()` giảm usage count của fd/filesystem; (7) lưu **exit code** vào `exit_code` cho cha đọc; (8) `exit_notify()` báo cha, **reparent các con**, đặt `exit_state = EXIT_ZOMBIE`; (9) gọi `schedule()` sang process khác — **`do_exit()` không bao giờ return**.

Sau bước này (tr. 37): task **không còn chạy được, không còn address space**, ở trạng thái **zombie**. Bộ nhớ duy nhất nó còn chiếm: *"its kernel stack, the thread_info structure, and the task_struct structure. The task exists solely to provide information to its parent."*

**Vì sao có zombie? (tr. 37–38)** Việc "dọn dẹp process" và việc "xóa descriptor" được **tách rời có chủ đích** — để cha còn cơ hội đọc thông tin về con đã chết (exit status). `wait()` (hiện thực qua một syscall `wait4()`) treo cha đến khi một con thoát, trả về PID + exit code. Xong đâu đấy, `release_task()` mới thật sự giải phóng nốt (gỡ khỏi pidhash + task list, `put_task_struct()` free kernel stack/thread_info + slab của task_struct).

**Con mồ côi — reparent về init (tr. 38–39):** nếu **cha chết trước con**, phải gả các con cho parent mới, kẻo *"parentless terminated processes would forever remain zombies, wasting system memory."* `find_new_reaper()` tìm một thread khác trong cùng thread group; không có thì gả về **init**. Và init *"routinely calls wait() on its children, cleaning up any zombies assigned to it"* (tr. 39) — init dọn zombie giúp.

### Insight đáng nhớ (cụm 1)

- **"To Linux, a thread is just a special kind of process" (tr. 23)** không phải khẩu hiệu suông — nó **thành sự thật cụ thể** ở tr. 34: thread = `clone(CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND)`. Nắm được cầu nối "khái niệm ↔ cờ clone" là nắm trọn mô hình process của Linux.
- **Zombie không phải bug** — nó là **khoảng thời gian cố ý giữ lại descriptor tối thiểu** giữa lúc con `do_exit()` và lúc cha `wait()` (tr. 37). Zombie tồn đọng thật sự = **cha quên `wait()`**, không phải "con lỗi".
- COW (tr. 31) + child-runs-first (tr. 32) là **một cặp thiết kế**: cả hai cùng nhắm vào tối ưu trường hợp "fork rồi exec ngay" — mẫu dùng phổ biến nhất của Unix.

### ⚠️ Đã thay đổi so với sách

- `current` không còn "mask stack pointer" trên x86-64/arm64 → dùng per-CPU/register. Ý O(1) giữ nguyên.
- `TASK_KILLABLE` (2.6.25 — sau bản in này) là trạng thái ngủ **lì nhưng nhận fatal signal**, nay là chuẩn cho chỗ chờ I/O dài (thay dần `TASK_UNINTERRUPTIBLE` thuần) — xem interview Câu 1.
- `task_struct` đã phình lớn hơn 1.7KB nhiều (thêm cgroup, namespace, sched info...) — con số của sách chỉ để cảm nhận "nhỏ đến bất ngờ", đừng nhớ như hằng số.

### Góc interview (cụm 1)

**Câu 1 (🎯):** Trạng thái **D** (`TASK_UNINTERRUPTIBLE`) là gì, vì sao nguy hiểm, và liên hệ với hiện tượng *"load average cao mà CPU idle"*?

<details><summary>Đáp án</summary>

- **Gốc từ sách (tr. 28):** `TASK_UNINTERRUPTIBLE` giống `TASK_INTERRUPTIBLE` nhưng **không dậy khi có signal** — dùng khi task phải chờ **không thể bị ngắt quãng** (thường là I/O với thiết bị/NFS: thức dậy giữa chừng bởi Ctrl-C sẽ để phần cứng/dữ liệu ở trạng thái lửng lơ). Trong `ps` nó hiện cột **`D`**.
- **Vì sao nguy hiểm:** chú thích tr. 28 nói thẳng — *"you cannot send it a SIGKILL signal"*. Signal chỉ được xử lý khi task chạy lại, mà nó không dậy → **`kill -9` vô tác dụng**. Task kẹt D vĩnh viễn nghĩa là tầng dưới (driver/storage/NFS) treo → thường chỉ còn reboot; hàng loạt task D là triệu chứng **I/O subsystem có bệnh**.
- **Load average:** 🆕 load của Linux đếm task **runnable + uninterruptible** (khác Unix cổ chỉ đếm runnable). Ổ đĩa/NFS chết → nhiều task rơi vào D → load = 50 trong khi CPU idle 95%. Đọc load phải kèm `vmstat`/`ps aux` xem cột state là `R` (thật sự tính toán) hay `D` (kẹt I/O). Chẩn đoán task D: `cat /proc/<pid>/stack` (kẹt ở hàm kernel nào), `echo w > /proc/sysrq-trigger` (dump các task blocked).
- **Điểm cộng:** `TASK_KILLABLE` (2.6.25) — ngủ lì nhưng **nhận fatal signal** — là giải pháp hiện đại cho chỗ chờ I/O dài, gỡ đúng nhược điểm "không giết được".

</details>

**Câu 2:** Trên Linux, thread và process khác nhau ở đâu *trong kernel*? Kể chuỗi từ `pthread_create` xuống.

<details><summary>Đáp án</summary>

- Cốt lõi (tr. 23, 33–34): **kernel không có kiểu dữ liệu "thread" riêng**; mỗi thread là **một `task_struct` độc lập** — chỉ khác ở chỗ *chia sẻ tài nguyên nào* với các task khác. `The kernel schedules individual threads, not processes` → đơn vị xếp lịch là task.
- Chuỗi: `pthread_create` (glibc/NPTL) → `clone()` với **`CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | ...`** → `do_fork()` → `copy_process()`. Ở **bước 7** của `copy_process()` (tr. 32), tùy cờ mà address space/files/signal được **chia sẻ** (thread) hay **copy** (process). `fork()` thường thì gọi cùng đường nhưng cờ = `SIGCHLD` (không chia sẻ gì).
- Hệ quả đáng nêu: vì thread chung address space, bug data race là chuyện của người lập trình (chương 3–5 C++ / [ostep/concurrency.md](../ostep/concurrency.md)); vì mọi thread là task riêng nên `getpid()` trả **TGID** (thread group ID) để mọi thread trong nhóm thấy cùng PID (tr. 71, chú thích).

</details>

**Câu 3:** Giải thích **Copy-on-Write** khi `fork()`. Chi phí thật của `fork()` là gì, và vì sao kernel "chạy con trước"?

<details><summary>Đáp án</summary>

- **COW (tr. 31):** thay vì copy toàn bộ address space, cha con **dùng chung** một bản, mọi trang đánh dấu read-only. Khi **một bên ghi** vào một trang → CPU sinh fault → kernel **nhân bản riêng trang đó** cho bên ghi. Trang không bao giờ bị ghi thì không bao giờ bị copy.
- **Chi phí thật của `fork()`:** chỉ là *"the duplication of the parent's page tables and the creation of a unique process descriptor for the child"* — copy **page table** (không phải dữ liệu) + tạo `task_struct` mới. Address space vài chục MB không bị copy.
- **Chạy con trước (tr. 32):** trường hợp phổ biến là con `exec()` ngay (thay address space) → nếu cha chạy trước và ghi vào bộ nhớ, sẽ gây COW-fault nhân bản trang **vô ích** (vì con sắp vứt cả address space). Chạy con trước triệt tiêu lãng phí đó. ⚠️ Điểm cộng thành thật: sách ghi cơ chế này *"does not currently function correctly"* tại thời điểm in.
- 🆕 Liên hệ embedded: đây là lý do `vfork()`/`posix_spawn` vẫn hữu ích trên thiết bị RAM ít — và tại sao "fork một process khổng lồ chỉ để exec" vẫn có thể tốn (copy page table lớn); nhiều hệ nhúng chuộng `posix_spawn`/`vfork`.

</details>

---

## Cụm 2 — CFS: kernel chọn process nào chạy (ch. 4, tr. 41–67)

### 2.1 Bài toán của scheduler & vì sao preemptive (tr. 41–42)

**Nhiệm vụ một câu (tr. 41):** scheduler *"decides which process runs, when, and for how long"* — chia tài nguyên hữu hạn (thời gian CPU) cho các process runnable. Ý tưởng nền cực đơn giản: nếu còn process runnable thì **luôn phải có process đang chạy**; khi runnable nhiều hơn số CPU thì phải **chọn ai chạy tiếp** — đó là quyết định cốt lõi.

**Hai kiểu multitasking (tr. 41–42):**
- **Cooperative:** process chỉ nhường CPU khi **tự nguyện** (gọi là *yielding*). Nhược điểm chí mạng: một process treo mà không yield có thể **kéo sập cả hệ** (Mac OS 9, Windows 3.1 thời xưa).
- **Preemptive** (Linux + mọi Unix): scheduler **cưỡng bức** dừng process đang chạy để chạy cái khác — hành động đó gọi là **preemption**. Thời gian một process được chạy trước khi bị chen gọi là **timeslice**. ⚠️ Nhưng: *"Linux's unique 'fair' scheduler does not employ timeslices per se, to interesting effect"* (tr. 42) — Linux **bỏ timeslice cố định**, ta sẽ thấy vì sao.

**Lịch sử scheduler (tr. 42–43):** Linux 1991→2.4 dùng scheduler đơn giản, scale kém. 2.5 thay bằng **O(1) scheduler** (timeslice tính hằng thời gian + runqueue per-CPU) — scale tốt trên server nhiều CPU, **nhưng dở với interactive** (ứng dụng người dùng tương tác). Cuối cùng **CFS (Completely Fair Scheduler)** thay O(1) ở **kernel 2.6.23**, lấy cảm hứng từ *fair scheduling* của Rotating Staircase Deadline scheduler.

### 2.2 Policy: I/O-bound vs processor-bound (tr. 43–44)

Process chia hai loại (tr. 43):
- **I/O-bound:** phần lớn thời gian **chờ I/O** (I/O = mọi tài nguyên blockable: bàn phím, mạng, đĩa) → chạy từng đoạn ngắn rồi lại block. Hầu hết app GUI là I/O-bound (chờ chuột/phím) dù chẳng đọc đĩa.
- **Processor-bound:** phần lớn thời gian **đốt CPU** → chạy đến khi bị preempt. Ví dụ điển hình: vòng lặp vô hạn, `ssh-keygen`, MATLAB.

Không loại trừ nhau (X server vừa CPU vừa I/O; word processor bình thường chờ phím nhưng có lúc "phát điên" spell-check). Scheduler phải cân **hai mục tiêu mâu thuẫn (tr. 43–44)**: *"fast process response time (low latency)"* và *"maximal system utilization (high throughput)"*. Linux — nhắm desktop/interactive — **ưu tiên latency**, tức thiên vị I/O-bound, *"in a creative manner that does not neglect processor-bound processes"* (chính là CFS).

### 2.3 Priority: nice và real-time (tr. 44)

Linux có **hai dải priority tách rời**:
- **nice value:** −20 … +19, mặc định 0. **nice cao = priority thấp** ("nhường" người khác). ⚠️ Đảo trực giác: nice +19 là "hiền nhất/thấp nhất". Trên Linux nice điều khiển **tỉ lệ** CPU (khác Mac OS X dùng nice cho timeslice tuyệt đối). Xem cột `NI` trong `ps -el`.
- **real-time priority:** mặc định 0…99, **cao = ưu tiên hơn** (ngược nice). *"All real-time processes are at a higher priority than normal processes"* — mọi RT task đứng trên mọi task thường. Theo POSIX.1b. Xem cột `RTPRIO`.

### 2.4 Vì sao CFS bỏ timeslice: 4 bệnh của Unix cổ điển (tr. 45–48)

**Timeslice (tr. 45):** đặt timeslice bao nhiêu là bài toán khó — dài quá thì interactive tệ (cảm giác không "đồng thời"); ngắn quá thì phí CPU cho overhead chuyển process. Và mâu thuẫn I/O-bound (thích chạy *thường xuyên*, không cần dài) vs processor-bound (thích timeslice *dài* để giữ cache nóng) lại hiện ra.

Sách nêu **4 bệnh** của mô hình "nice → timeslice tuyệt đối" (tr. 47–48) — đọc để hiểu *vì sao* CFS ra đời:

1. **Ánh xạ nice→timeslice tuyệt đối cho hành vi switch dở.** Ví dụ: nice 0 = 100ms, nice +20 = 5ms. Chạy **hai** process nice 0 → mỗi cái 100ms/lượt (switch mỗi 200ms — ổn). Chạy **hai** process nice +20 → mỗi cái chỉ 5ms/lượt → **switch mỗi 10ms** (phí!). Tệ hơn: process nice cao thường là nền/CPU-nặng, lại bị switch dày — **ngược hoàn toàn với mong muốn**.
2. **nice tương đối cho hiệu ứng phi lý.** nice 0→1 map 100ms→95ms (chênh không đáng kể). Nhưng nice 18→19 map 10ms→5ms — **gấp đôi thời gian CPU** chỉ vì lệch 1 nấc! Vì nice thường dùng **tương đối** (`nice -n +1`), "nice down 1 nấc" có tác dụng khác hẳn nhau tùy điểm xuất phát.
3. **Timeslice tuyệt đối bị buộc theo timer tick.** Muốn đo timeslice, kernel phải đo bằng **bội số của tick** (chương 11 — file 03) → sàn timeslice = chu kỳ tick (1–10ms), và timeslice đổi khi HZ đổi.
4. **Xử lý wake-up trong scheduler priority-based bị game.** Muốn interactive tốt thì "thưởng" task vừa dậy được chạy ngay (dù hết timeslice) → mở đường cho **kịch bản lạm dụng**: một số mẫu sleep/wake có thể **lừa scheduler cấp CPU không công bằng**.

**Kết luận của CFS (tr. 48):** vấn đề gốc là *"assigning absolute timeslices yields a constant switching rate but variable fairness"*. CFS đảo ngược triệt để: **bỏ timeslice, gán mỗi process một TỈ LỆ (proportion) của CPU** → *"constant fairness but a variable switching rate"* (công bằng cố định, nhịp switch thay đổi).

### 2.5 CFS hoạt động: perfect multitasking → vruntime → rbtree (tr. 48–57)

**Mô hình lý tưởng (tr. 48):** hình dung một CPU *"ideal, perfectly multitasking"* chạy **cả n process cùng lúc**, mỗi cái ở 1/n công suất. Hai process → chạy song song 10ms, mỗi cái 50% sức. Gọi là **perfect multitasking**. Thực tế không làm được (một CPU không chạy song song thật, và chạy từng lát vô cùng nhỏ thì overhead switch quá lớn), nên CFS **xấp xỉ**: chạy round-robin, **luôn chọn tiếp process đã chạy ít nhất**.

- **Weight từ nice:** CFS dùng nice để **cân tỉ lệ** CPU (không phải để tính timeslice): nice thấp (priority cao) → weight lớn → nhiều CPU hơn.
- **Targeted latency (tr. 48):** mục tiêu xấp xỉ cho "khoảng vô cùng nhỏ" của perfect multitasking. Ví dụ targeted latency = 20ms, hai task cùng priority → mỗi cái chạy 10ms/lượt; bốn task → 5ms; hai mươi task → 1ms.
- **Minimum granularity (tr. 49):** khi số task → ∞, timeslice → 0 → overhead switch không chấp nhận được. CFS đặt **sàn** ~**1ms**. ⚠️ Sách thừa nhận thẳng: quá nhiều task thì CFS **không còn perfectly fair** (bị sàn min granularity chặn) — đây là trade-off *cố ý*; với vài task (trường hợp thường) thì CFS công bằng tuyệt đối.
- **Chỉ nice TƯƠNG ĐỐI quan trọng (tr. 49–50):** hai process nice {0, 5} và hai process nice {10, 15} nhận **cùng** tỉ lệ (15ms/5ms) — vì chỉ *hiệu* nice quyết định tỉ lệ. nice cho **hiệu ứng hình học (geometric)**, không cộng tuyến tính.

**vruntime — sổ ghi công bằng (tr. 50–52).** CFS không có timeslice nhưng vẫn phải đếm thời gian mỗi process chạy → dùng `struct sched_entity` (nhúng trong `task_struct` tên `se`):
```c
struct sched_entity {
        struct load_weight  load;              // weight (từ nice)
        struct rb_node      run_node;          // node trong red-black tree
        unsigned int        on_rq;
        u64                 exec_start;
        u64                 sum_exec_runtime;
        u64                 vruntime;          // ← "thời gian chạy ảo", đơn vị ns
        ...
};
```
**`vruntime`** = thời gian chạy thực **chuẩn hóa theo weight** (đơn vị ns, **tách rời timer tick**). Trên CPU lý tưởng, mọi process cùng priority sẽ có vruntime **bằng nhau**. CFS cập nhật nó trong `update_curr()`:
```c
static void update_curr(struct cfs_rq *cfs_rq)
{
        struct sched_entity *curr = cfs_rq->curr;
        u64 now = rq_of(cfs_rq)->clock;
        unsigned long delta_exec;
        ...
        delta_exec = (unsigned long)(now - curr->exec_start);  // đã chạy bao lâu từ lần trước
        if (!delta_exec) return;
        __update_curr(cfs_rq, curr, delta_exec);   // cộng vào vruntime, có cân theo weight
        curr->exec_start = now;
        ...
}
```
`update_curr()` chạy **định kỳ theo system timer** và mỗi khi process runnable/block. Bên trong, `__update_curr()` cộng `delta_exec` (đã **cân theo weight** qua `calc_delta_fair`) vào `curr->vruntime`.

**Chọn task tiếp theo — quy tắc một dòng (tr. 52):**
> *"When CFS is deciding what process to run next, it picks the process with the smallest vruntime. That's it!"*

CFS lưu các process runnable trong một **red-black tree** (rbtree — cây nhị phân tìm kiếm tự cân bằng, ch. 6), **key = vruntime**. Process cần chạy tiếp (vruntime nhỏ nhất) chính là **node trái nhất** của cây:
```c
static struct sched_entity *__pick_next_entity(struct cfs_rq *cfs_rq)
{
        struct rb_node *left = cfs_rq->rb_leftmost;   // KHÔNG duyệt cây!
        if (!left) return NULL;                        // cây rỗng → chạy idle task
        return rb_entry(left, struct sched_entity, run_node);
}
```
🆕 Điểm hay của thiết kế (tr. 53): tìm node trái nhất đáng lẽ O(log n), nhưng CFS **cache** con trỏ `rb_leftmost` → **đọc O(1)**. Chèn/xóa mới là O(log n) (`__enqueue_entity` duyệt cây đặt node đúng chỗ + cập nhật cache leftmost; `__dequeue_entity` gọi `rb_erase` + nếu gỡ đúng leftmost thì `rb_next` tìm leftmost mới).

**Điểm vào scheduler (tr. 57):** `schedule()` (trong `kernel/sched.c`) **generic theo scheduler class** — nó gọi `pick_next_task()`, hàm này duyệt các class **theo thứ tự priority**, class cao nhất có process runnable thắng:
```c
static inline struct task_struct *pick_next_task(struct rq *rq)
{
        /* Tối ưu: nếu MỌI task đều thuộc fair class → gọi thẳng CFS */
        if (likely(rq->nr_running == rq->cfs.nr_running)) {
                p = fair_sched_class.pick_next_task(rq);
                if (likely(p)) return p;
        }
        class = sched_class_highest;
        for ( ; ; ) {                          // duyệt class từ cao xuống thấp
                p = class->pick_next_task(rq);
                if (p) return p;               // class đầu tiên có task → thắng
                class = class->next;           // idle class luôn trả non-NULL
        }
}
```
Đây là hiện thực của **scheduler classes** (tr. 46): các thuật toán khác nhau (RT, CFS, idle) **cắm plug-in**, xếp theo priority — RT class đứng trên CFS class (mục 2.7).

### 2.6 Ngủ & đánh thức: wait queue và bug lost-wakeup (tr. 58–61)

Task ngủ (blocked) ở state đặc biệt **không runnable** — nếu không có state này, scheduler sẽ chọn nhầm task không muốn chạy, hoặc "ngủ" phải hiện thực bằng busy-loop (đốt CPU). Task ngủ khi **chờ một sự kiện** (hết một khoảng thời gian, dữ liệu file I/O, sự kiện phần cứng, hoặc chờ semaphore contended — file 03). Cơ chế: **wait queue** — *"a simple list of processes waiting for an event to occur"* (tr. 58).

**Mẫu ngủ ĐÚNG (tr. 59)** — sách cảnh báo các interface đơn giản cũ **có race**: *"It is possible to go to sleep after the condition becomes true. In that case, the task might sleep indefinitely"* (lost wakeup). Mẫu khuyến nghị:
```c
/* 'q' là wait queue ta muốn ngủ trên đó */
DEFINE_WAIT(wait);

add_wait_queue(q, &wait);
while (!condition) {           // (5) DẬY thì kiểm tra lại điều kiện — ngủ trong VÒNG LẶP
        prepare_to_wait(&q, &wait, TASK_INTERRUPTIBLE);   // (3) đặt state TRƯỚC
        if (signal_pending(current))
                /* (4) xử lý signal — spurious wakeup */;
        schedule();            // nhường CPU, ngủ
}
finish_wait(&q, &wait);        // (6) đặt lại TASK_RUNNING + gỡ khỏi queue
```
Sáu bước (tr. 59): (1) tạo entry `DEFINE_WAIT`; (2) `add_wait_queue`; (3) `prepare_to_wait` đổi state sang `TASK_INTERRUPTIBLE`/`UNINTERRUPTIBLE`; (4) nếu INTERRUPTIBLE, signal đánh thức → **spurious wakeup**, phải kiểm tra & xử lý; (5) **dậy thì kiểm tra lại điều kiện**, chưa thỏa thì `schedule()` lại; (6) điều kiện thỏa → `finish_wait`.

**Mấu chốt chống lost-wakeup:** đặt state (`prepare_to_wait`) **trước** khi kiểm tra `condition`, và kiểm tra **trong vòng lặp**. Nếu điều kiện xảy ra **trước** khi task kịp ngủ, vòng `while` thoát ngay — *"the task does not erroneously go to sleep"* (tr. 59). 🆕 Đây là phiên bản kernel của quy tắc "condition variable phải dùng `while`, không `if`" ([ostep/concurrency.md](../ostep/concurrency.md)); sách còn dẫn ví dụ thật `inotify_read()` (tr. 60) theo đúng mẫu này.

**Đánh thức (tr. 61):** `wake_up()` đánh thức **mọi** task trên queue → gọi `try_to_wake_up()`: đặt `TASK_RUNNING`, thêm task vào rbtree, và **set `need_resched`** nếu task vừa dậy có priority cao hơn task đang chạy. Nhắc lại điểm sinh tử: *"there are spurious wake-ups... sleeping should always be handled in a loop"* (tr. 61).

### 2.7 Preemption, context switch, và RT (tr. 62–66)

**Context switch (tr. 62)** — `context_switch()` được `schedule()` gọi, làm **hai việc**:
- `switch_mm()` — đổi **memory mapping** (address space) từ process cũ sang mới;
- `switch_to()` — đổi **processor state**: lưu/khôi phục stack + register + trạng thái đặc thù arch.

🆕 Đúng hai nửa "đổi không gian nhớ" + "đổi CPU state" — cùng khung với mọi OS.

**`need_resched` — cờ báo "cần reschedule" (tr. 62):** kernel không thể chỉ gọi `schedule()` khi code chủ động gọi (userspace sẽ chạy vô hạn). Nên có cờ `need_resched`, **set** bởi `scheduler_tick()` (khi task nên bị preempt) và `try_to_wake_up()` (khi task priority cao hơn vừa dậy). Cờ nằm **per-process** trong `thread_info` (nhanh vì `current` thường cache-hot). **Set cờ chưa phải switch** — switch chỉ xảy ra ở **điểm kiểm tra cờ**:

- **User preemption (tr. 62–63):** khi kernel **sắp quay về user-space** (sau syscall hoặc sau interrupt) và `need_resched` set → gọi scheduler. An toàn vì lúc quay về user, kernel *"knows it is in a safe quiescent state"*.
- **Kernel preemption (tr. 63–64):** Linux là **fully preemptive kernel** (từ 2.6) — có thể chen task **ngay trong kernel**, miễn *"it is safe to reschedule"*. Khi nào an toàn? **Khi không giữ lock nào.** Cơ chế: **`preempt_count`** trong `thread_info` — bắt đầu 0, **+1 mỗi lần lấy lock, −1 mỗi lần nhả**. Khi `preempt_count == 0` → kernel preemptible. Lúc return từ interrupt về kernel-space, kiểm tra: `need_resched` set **và** `preempt_count == 0` → an toàn, gọi scheduler.

Kernel preemption xảy ra khi (tr. 64): interrupt handler thoát trước khi về kernel-space; kernel code trở lại preemptible; task trong kernel tự gọi `schedule()`; hoặc task trong kernel block.

**Real-time policies (tr. 64–65):** `SCHED_FIFO` và `SCHED_RR`, quản bởi **rt scheduler riêng** (`kernel/sched_rt.c`), **xếp trên** CFS qua scheduler classes:
- **SCHED_FIFO:** FIFO không timeslice. Runnable là chạy đến khi **tự block hoặc yield** — **không có timeslice, chạy vô hạn**. Chỉ FIFO/RR priority cao hơn mới preempt được nó. Mọi task priority thấp hơn **không được chạy** đến khi nó ngừng.
- **SCHED_RR:** = FIFO **có timeslice** (round-robin giữa các task **cùng priority**). Hết timeslice thì nhường cho task cùng priority; task priority thấp hơn vẫn không chen được.
- Cả hai **priority tĩnh** (kernel không tính priority động cho RT) → RT priority cao **luôn** preempt priority thấp.
- ⚠️ Linux cho **soft real-time**: *"the kernel tries to schedule applications within timing deadlines, but... does not promise"* — không phải hard RT. RT priority 0…99 chia sẻ không gian với nice (nice −20…+19 map vào 100…139).

**Scheduler syscalls (tr. 65–66):** `nice()`, `sched_setscheduler()`/`getscheduler` (đổi policy + rt priority — thực chất chỉ đọc/ghi `policy` và `rt_priority` trong `task_struct`), `sched_setaffinity()`/`getaffinity` (**hard affinity** — bitmask `cpus_allowed`, ép task chỉ chạy trên tập CPU chỉ định), `sched_yield()` (chủ động nhường — *"applications and even kernel code should be certain they truly want to give up the processor"*).

### Insight đáng nhớ (cụm 2)

- **Cả CFS gói trong một câu (tr. 52): "pick the process with the smallest vruntime."** Mọi thứ còn lại (weight, rbtree, leftmost cache) chỉ là **hiện thực hiệu quả** cho câu đó. Trả lời phỏng vấn nên bắt đầu từ câu này rồi mới đi vào rbtree.
- **4 bệnh của "nice→timeslice tuyệt đối" (tr. 47–48)** là *lý do tồn tại* của CFS — nhớ ít nhất bệnh #1 (số task cùng priority làm nhịp switch nhảy lung tung) và #2 (nice tương đối cho hiệu ứng phi lý). CFS đổi "timeslice tuyệt đối" → "tỉ lệ CPU" để được **công bằng cố định, nhịp switch biến thiên** thay vì ngược lại.
- **`need_resched` + `preempt_count` (tr. 62–64)** là bộ đôi quyết định "khi nào được chen": set cờ ≠ switch; switch chỉ ở **điểm kiểm tra** (về userspace, hoặc `preempt_count==0`). **Giữ lock = `preempt_count > 0` = cấm chen** — đây là cầu nối trực tiếp sang file 03 (vì sao "giữ spinlock thì không được ngủ").
- Mẫu wait-queue (tr. 59) — **đặt state trước, kiểm tra điều kiện trong vòng lặp** — là bug lost-wakeup phiên bản kernel; nhớ được là ăn điểm cả ở câu C++ condition variable lẫn câu kernel.

### ⚠️ Đã thay đổi so với sách

- **CFS đã bị thay bằng EEVDF** (kernel 6.6, cuối 2023) — vẫn họ fair/vruntime nhưng chọn theo *virtual deadline sớm nhất*, kỷ luật latency tốt hơn, gỡ đống heuristic wakeup. Khung khái niệm (weight, vruntime, rbtree, sched_class) của sách **vẫn là nền để hiểu EEVDF**.
- **SCHED_DEADLINE** (3.14 — EDF thật, đứng **trên** cả FIFO/RR), **core scheduling** (chống rò L1TF/MDS giữa hyperthread), **EAS** (Energy-Aware Scheduling — chọn core theo năng lượng trên big.LITTLE, đúng chuyện embedded).
- `sched_yield()` mô tả trong sách (active/expired array) là của **O(1) scheduler**, không còn đúng với CFS — nhưng "nên chắc chắn trước khi yield" thì vẫn đúng.
- **PREEMPT_RT** (nay đã vào mainline) đẩy preemption đi xa hơn nữa — biến hầu hết spinlock thành sleeping lock để `preempt_count==0` càng nhiều càng tốt ([melp/debug-realtime.md](../melp/debug-realtime.md)).

### Góc interview (cụm 2)

**Câu 1 (🎯):** Giải thích CFS trong ~4 câu. Vì sao "công bằng" nhưng vẫn phản hồi nhanh cho ứng dụng interactive?

<details><summary>Đáp án</summary>

- **Cốt lõi (tr. 52):** CFS mô hình hóa một CPU *perfectly multitasking* (chạy cả n process cùng lúc, mỗi cái 1/n sức); vì không làm được thật, nó chạy round-robin và **luôn chọn process có `vruntime` nhỏ nhất** — `vruntime` = thời gian đã chạy **chuẩn hóa theo weight** (nice). Cấu trúc: rbtree key=vruntime, chọn **node trái nhất** (cache O(1)).
- **Bỏ timeslice tuyệt đối → gán TỈ LỆ CPU (tr. 48):** nhờ đó tránh 4 bệnh của Unix cổ điển (nhịp switch nhảy loạn theo mix priority, nice tương đối phi lý, phụ thuộc tick, wake-up bị game).
- **Vì sao interactive vẫn nhanh (tr. 45–46, ví dụ text editor vs video encoder):** text editor I/O-bound ngủ gần hết thời gian → **vruntime của nó nhỏ**. Khi user gõ phím, nó dậy, CFS thấy vruntime nó **nhỏ hơn** video encoder → **preempt encoder cho editor chạy ngay**. Nó xử lý phím rồi ngủ tiếp — công bằng theo tỉ lệ **tự nhiên** cho ra phản hồi nhanh, không cần heuristic "đoán ai interactive".
- ⚠️ Điểm cộng thời sự: từ 6.6 CFS được thay bằng **EEVDF** (virtual deadline) — nêu được là đang cập nhật.

</details>

**Câu 2 (🎯):** Kernel quyết định "đến lúc reschedule" ở những điểm nào? **Kernel preemption** khác **user preemption** ra sao, và liên quan gì tới lock?

<details><summary>Đáp án</summary>

- Cờ **`need_resched`** (tr. 62) được **set** bởi `scheduler_tick()` (task vượt phần công bằng) hoặc `try_to_wake_up()` (task priority cao hơn vừa dậy). **Set cờ ≠ switch** — switch chỉ ở **điểm kiểm tra cờ**:
  1. **User preemption (tr. 62):** trên đường **quay về user-space** sau syscall/interrupt — luôn an toàn vì trạng thái kernel đã "gói xong" (quiescent).
  2. **Kernel preemption (tr. 63):** ngay **trong kernel**, khi an toàn — điều kiện là **`preempt_count == 0`** (không giữ lock, không trong atomic). Kiểm tra lúc interrupt return về kernel và khi `preempt_enable`.
- **Vai trò của lock:** *"locks are used as markers of regions of nonpreemptibility"* (tr. 63) — `preempt_count` **+1 mỗi lock, −1 mỗi unlock**; giữ lock ⇒ `preempt_count > 0` ⇒ **cấm chen trên CPU đó**. Đây chính là lý do sâu xa "giữ spinlock không được ngủ/không được bị preempt" (đào tiếp ở [03-sync-timers.md](03-sync-timers.md)).
- Kernel không preempt (PREEMPT_NONE — server) thì task trong kernel chạy tới khi tự ra/tự block → latency tệ hơn nhưng throughput có thể nhỉnh. 🆕 PREEMPT_RT = làm cho `preempt_count==0` càng nhiều càng tốt (spinlock→rt_mutex) để latency thấp — nền của Linux realtime.

</details>

---

## Cụm 3 — System call: từ trap đến `sys_*` (ch. 5, tr. 69–83)

### 3.1 Vì sao cần syscall & quan hệ với C library (tr. 69–71)

**Ba mục đích của syscall (tr. 69):** là **lớp giữa** hardware và process user. (1) **Trừu tượng hóa phần cứng** — `read()` không cần biết loại đĩa/filesystem; (2) **an ninh & ổn định** — kernel làm trung gian, phân xử truy cập theo quyền/user; (3) **ảo hóa hệ thống** — nếu app truy cập tài nguyên mà kernel không biết thì không thể có multitasking/virtual memory ổn định. Câu chốt (tr. 69): *"system calls are the only means user-space has of interfacing with the kernel; they are the only legal entry point into the kernel other than exceptions and traps."* Ngay cả `/proc`, device file cũng **cuối cùng đi qua syscall**.

**API ≠ syscall (tr. 70):** app lập trình theo **API** (thường POSIX), không gọi thẳng syscall. Một API có thể là 0, 1 hay nhiều syscall. Trên Unix, **C library** hiện thực API chính + cung cấp syscall interface + phần lớn POSIX API. Figure 5.1 minh họa: `printf()` → `write()` trong C library → `write()` syscall. Từ góc app: *"system calls are irrelevant; all the programmer is concerned with is the API"*. Meme Unix đáng nhớ (tr. 71): **"Provide mechanism, not policy"** — syscall cung cấp *cơ chế* trừu tượng, *cách dùng* không phải việc của kernel.

### 3.2 Một syscall trông thế nào trong kernel (tr. 71–72)

Syscall trả về `long` (tương thích 64-bit); thường (không luôn luôn) **âm = lỗi**, 0 = thành công. Khi lỗi, C library ghi mã lỗi vào biến global **`errno`** (dịch ra chữ bằng `perror()`). Ví dụ đơn giản nhất — `getpid()`:
```c
SYSCALL_DEFINE0(getpid)            // 0 = số tham số
{
        return task_tgid_vnr(current);   // trả current->tgid
}
```
`SYSCALL_DEFINE0` là **macro** khai một syscall không tham số; nó nở ra:
```c
asmlinkage long sys_getpid(void);
```
Ba điểm phải nhớ (tr. 72):
- **`asmlinkage`** — báo compiler *"chỉ tìm tham số của hàm này trên STACK"* (bắt buộc với mọi syscall).
- **trả `long`** — để tương thích 32/64-bit (syscall khai `int` ở user trả `long` trong kernel).
- **quy ước tên `sys_`** — syscall `bar()` hiện thực trong kernel là `sys_bar()`.

🆕 Vì sao `getpid()` trả **TGID** (thread group id) chứ không PID? (chú thích tr. 71) Để **mọi thread trong một nhóm thấy cùng một PID** — nối lại với "thread là task chia sẻ" ở cụm 1.

### 3.3 Syscall number & bảng syscall (tr. 72)

Mỗi syscall có một **syscall number** duy nhất — process gọi bằng **số**, không bằng tên. Luật bất di bất dịch: *"when assigned, it cannot change... if a system call is removed, its system call number cannot be recycled"* — **số đã cấp không đổi, không tái dùng**, kẻo binary cũ gọi nhầm. Chỗ trống để lại được "bịt" bằng `sys_ni_syscall()` (chỉ trả `-ENOSYS`). Kernel giữ mọi syscall đã đăng ký trong **`sys_call_table`**, mỗi arch một bảng riêng (ánh xạ số → hàm).

### 3.4 Đường đi của một syscall: trap → handler → sys_* (tr. 73–74)

App **không thể** gọi thẳng hàm kernel (kernel ở vùng nhớ được bảo vệ). Cách vào: **software interrupt** — gây một exception để CPU **chuyển sang kernel mode** và chạy exception handler; ở đây handler chính là **system call handler**.

- Trên x86 cổ điển: **`int $0x80`** (interrupt số **128**) → chạy vector 128 = hàm **`system_call()`** (assembly, `entry_64.S`). ⚠️ x86 sau này thêm **`sysenter`** — đường trap nhanh hơn `int`; nay dùng lệnh **`syscall`** (x86-64) / **`svc`** (ARM). Điểm bất biến: *"somehow user-space causes an exception or trap to enter the kernel"* (tr. 73).
- **Truyền syscall number (tr. 73):** trên x86 để trong register **`eax`**; `system_call()` đọc `eax`, so với `NR_syscalls` (≥ thì trả `-ENOSYS`), rồi:
  ```asm
  call *sys_call_table(,%rax,8)    ; mỗi entry 8 byte (x86-64) → nhân số với 8 để ra vị trí
  ```
- **Truyền tham số (tr. 74):** cũng qua register — x86-32 dùng `ebx, ecx, edx, esi, edi` cho 5 tham số đầu; ≥6 tham số thì một register giữ **con trỏ** tới vùng user chứa hết tham số. **Giá trị trả về** về user cũng qua register (`eax` trên x86).

Figure 5.2 (tr. 74): `read()` → wrapper trong C library → `system_call()` → `sys_read()`.

### 3.5 An ninh: KHÔNG BAO GIỜ deref thẳng con trỏ user (tr. 75–76)

Đây là **nguyên tắc an ninh số 1** của viết syscall. Syscall chạy trong kernel-space; nếu user truyền input bậy vào không kiểm tra, an ninh/ổn định sập. Quan trọng nhất là **con trỏ**: nếu process truyền con trỏ tùy ý (kể cả con trỏ nó không có quyền đọc!), *"Processes could then trick the kernel into copying data for which they did not have access permission."* Trước khi theo một con trỏ vào user-space, kernel **phải** đảm bảo (tr. 75):

1. Con trỏ **trỏ vào vùng user-space** (không lừa kernel đọc bộ nhớ kernel hộ).
2. Con trỏ **trỏ trong address space của chính process** (không đọc trộm dữ liệu process khác).
3. Quyền khớp: đọc → vùng phải **readable**; ghi → **writable**; chạy → **executable**.

> *"kernel code must never blindly follow a pointer into user-space! One of these two methods must always be used."* (tr. 75)

Hai method đó: **`copy_to_user()`** (ghi ra user) và **`copy_from_user()`** (đọc từ user), đều 3 tham số (dst, src, len), trả về **số byte copy hụt** (0 = thành công); lỗi thì syscall trả `-EFAULT`. Ví dụ của sách (`silly_copy`, tr. 76):
```c
SYSCALL_DEFINE3(silly_copy,
                unsigned long *, src,
                unsigned long *, dst,
                unsigned long len)
{
        unsigned long buf;
        /* copy src (nằm trong address space của user) vào buf trong kernel */
        if (copy_from_user(&buf, src, len))
                return -EFAULT;
        /* copy buf ra dst (cũng trong address space user) */
        if (copy_to_user(dst, &buf, len))
                return -EFAULT;
        return len;
}
```
**Điểm sinh tử về context (tr. 76):** *"Both copy_to_user() and copy_from_user() may block."* Vì trang chứa dữ liệu user có thể đang bị swap ra đĩa → process **ngủ** chờ page fault handler nạp trang về. → 🆕 **Do đó `copy_*_user` CẤM dùng trong atomic context** (ISR, giữ spinlock) — nối thẳng luật của [02-interrupts-bottomhalves.md](02-interrupts-bottomhalves.md) và [03-sync-timers.md](03-sync-timers.md).

**Kiểm tra quyền — `capable()` (tr. 77):** thay `suser()` cũ (chỉ hỏi "có phải root?"), nay là hệ **capabilities** mịn hơn: `capable(CAP_SYS_NICE)` hỏi "có quyền đổi nice của process khác không?". Ví dụ `reboot()` mở đầu bằng:
```c
SYSCALL_DEFINE4(reboot, int, magic1, int, magic2, unsigned int, cmd, void __user *, arg)
{
        /* chỉ tin superuser với việc reboot */
        if (!capable(CAP_SYS_BOOT))
                return -EPERM;
        ...
}
```
Sách nhấn: *"If that one conditional statement were removed, any process could reboot the system"* — một dòng `capable()` là ranh giới giữa an toàn và thảm họa.

### 3.6 System call context & vì sao không nên thêm syscall (tr. 78–83)

**Syscall context (tr. 78):** như đã nói ở cụm 1, syscall chạy trong **process context** — `current` trỏ tới process gọi. Hai hệ quả lớn: **được ngủ** (block/`schedule()`) → dùng được phần lớn tính năng kernel; và **fully preemptible** → task có thể bị task khác chen, mà task đó có thể **gọi lại chính syscall này** → *"care must be exercised to ensure that system calls are reentrant"* (đồng bộ reentrancy — file 03).

**Đăng ký một syscall mới (tr. 79–82):** dễ đến bất ngờ — (1) thêm entry vào cuối `sys_call_table` cho **mỗi arch**; (2) khai số trong `<asm/unistd.h>`; (3) compile vào **kernel image** (không phải module). Ví dụ `sys_foo` trả `THREAD_SIZE` (kích thước kernel stack) minh họa cả quy trình. Từ user-space, macro `_syscalln()` (n = số tham số) dựng register + phát lệnh trap.

**"Vì sao KHÔNG nên thêm syscall" (tr. 82–83)** — lời khuyên đắt giá. Dù dễ implement, sách khuyên **hết sức dè dặt**. **Cons:**
- Cần một **syscall number được cấp chính thức**;
- Một khi vào stable kernel, *"it is written in stone. The interface cannot change without breaking user-space applications"* — **ABI vĩnh viễn**;
- **Mỗi arch** phải đăng ký + hỗ trợ riêng;
- Không dùng được từ script, không truy cập trực tiếp từ filesystem;
- Khó bảo trì ngoài cây kernel chính; với trao đổi thông tin đơn giản thì syscall là **overkill**.

**Thay thế (tr. 83):** device node + `read()`/`write()` + `ioctl()`; biểu diễn interface bằng **file descriptor**; hoặc **thêm file vào sysfs**. 🆕 Cho driver, gần như **luôn** chọn char device + ioctl / sysfs / netlink thay vì syscall mới ([05/kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md)). Tốc độ chậm thêm syscall mới là **dấu hiệu Linux đã ổn định** (tr. 83).

### Insight đáng nhớ (cụm 3)

- **"Only legal entry point into the kernel other than exceptions and traps" (tr. 69)** — mọi thứ user làm với kernel (kể cả `/proc`, device file) **cuối cùng là syscall**. Nắm điều này là nắm ranh giới user/kernel.
- **`copy_*_user` MAY BLOCK (tr. 76)** là một trong những sự thật hay bị quên nhất và là **cầu nối ba chương**: nó ngủ (page fault) → cấm trong ISR/spinlock (file 02, 03). Ba lý do phải dùng nó (an ninh con trỏ kernel, fault an toàn qua exception table, quyền đọc/ghi) là câu hỏi ăn tiền — xem interview.
- **"Provide mechanism, not policy" + "syscall là ABI vĩnh viễn"** (tr. 71, 82) là hai kim chỉ nam thiết kế interface kernel — và là lý do triết học cho việc "driver nên dùng ioctl/sysfs, đừng đẻ syscall".

### ⚠️ Đã thay đổi so với sách

- Lệnh trap: `int $0x80` (sách) → **`syscall`/`sysenter`** (x86-64), **`svc`** (ARM). Số syscall và bảng vẫn vậy về ý.
- **vDSO** (virtual dynamic shared object): `gettimeofday()`/`clock_gettime()` chạy **không cần trap** (kernel map sẵn code + dữ liệu thời gian vào user-space) → giảm chi phí syscall cho các call đọc-thời-gian nóng.
- **seccomp** (filter chặn syscall theo profile BPF — nền của sandbox container), **io_uring** ("đường vòng syscall theo lô" — submit/complete queue chia sẻ, giảm số lần trap cho I/O throughput cao). Biết tên khi bàn "chi phí syscall" là điểm cộng.
- Con số "~335 syscall trên x86" (tr. 70) nay đã hơn 400 — chỉ để cảm nhận độ lớn.

### Góc interview (cụm 3)

**Câu 1 (🎯):** Vì sao kernel **không được** deref thẳng con trỏ từ user-space mà phải `copy_from_user`/`copy_to_user`? Nêu đủ lý do và hệ quả về context.

<details><summary>Đáp án</summary>

Ba lý do (bám tr. 75–76):
1. **An ninh — con trỏ có thể trỏ vào kernel/process khác:** user truyền địa chỉ vùng kernel (hoặc vùng process khác); nếu kernel deref hộ rồi trả kết quả về user = **nguyên thủy đọc/ghi bộ nhớ tùy ý** (privilege escalation). `copy_*_user` kiểm tra (`access_ok`) rằng con trỏ **thuộc user-space** và **trong address space của chính process**.
2. **Đúng đắn — trang có thể chưa/không map:** địa chỉ hợp lệ về dải nhưng trang bị **swap ra đĩa** hoặc chưa nạp (demand paging) → deref trần gây fault **trong kernel** (oops). `copy_*_user` có **exception table**: fault trong nó được định tuyến thành trả **`-EFAULT`** sạch sẽ.
3. **Quyền khớp:** đọc thì vùng phải readable, ghi thì writable (tr. 75). 🆕 Phần cứng hiện đại còn **cấm kernel chạm bộ nhớ user tùy tiện** (SMAP trên x86, PAN trên ARM) — `copy_*_user` mở "cửa sổ hợp lệ" có kiểm soát; deref trần sẽ fault ngay.
- **Hệ quả context (tr. 76):** `copy_*_user` **có thể ngủ** (chờ page fault nạp trang bị swap) → **cấm khi giữ spinlock / trong ISR** — nối luật của [03-sync-timers.md](03-sync-timers.md). Trả lời gồm "an ninh + fault an toàn + quyền/SMAP + có thể ngủ" là trọn.

</details>

**Câu 2:** Bạn cần đưa một tính năng mới từ kernel ra user-space. Có nên thêm một syscall mới không? Vì sao, và có gì thay thế?

<details><summary>Đáp án</summary>

- **Mặc định: KHÔNG** (tr. 82). Lý do chính: syscall một khi vào stable kernel là **ABI vĩnh viễn** — *"written in stone"*, không đổi/không xóa được, cần số hiệu cấp chính thức và **mỗi arch** phải đăng ký riêng; không gọi được từ script/filesystem; với trao đổi thông tin đơn giản là overkill.
- **Thay thế (tr. 83):** char device + `read()`/`write()` + **`ioctl()`** cho cấu hình/truy vấn; biểu diễn qua **file descriptor**; hoặc **sysfs** (một file dưới `/sys`); 🆕 thêm **netlink** (giao tiếp có cấu trúc, event-driven, hay dùng cho network/udev), **debugfs** (cho debug), **procfs** (legacy). Nguyên tắc "provide mechanism, not policy" (tr. 71) + "một syscall một mục đích, đừng multiplex như `ioctl` là ví dụ *phản diện*" (tr. 74).
- **Khi nào vẫn nên là syscall:** ngữ nghĩa cơ bản, tổng quát, dùng bởi nhiều chương trình, cần nhanh và ổn định lâu dài (như các syscall Unix "sống sót 30 năm" — tr. 75). Điểm cộng: nêu được tốc độ thêm syscall chậm là dấu hiệu OS trưởng thành.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [ostep/virtualization-cpu.md](../ostep/virtualization-cpu.md) — nền lý thuyết LDE (limited direct execution)/scheduling nhìn từ góc OS tổng quát.
- [03-operating-system/process-thread.md](../../03-operating-system/process-thread.md), [03-operating-system/scheduling.md](../../03-operating-system/scheduling.md) — bản cô đọng process/thread/scheduling của repo.
- [04-memory.md](04-memory.md) — slab allocator (cấp `task_struct`), `mm_struct`/address space (cái `fork` COW và `exit_mm` thao tác).
- [05-drivers-device-tree/kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md) — ioctl/sysfs/netlink: các "thay thế cho syscall mới".

**Chương tiếp theo:** [02 — Interrupts & Bottom Halves →](02-interrupts-bottomhalves.md) (interrupt context — nơi `current` mất nghĩa và mọi luật ở đây bị đảo).
