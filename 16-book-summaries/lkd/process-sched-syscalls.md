# LKD — Process, Scheduler & System Calls kernel-side (ch. 3 tr. 23, ch. 4 tr. 41, ch. 5 tr. 69) 🎯

> Thuộc [LKD](README.md). Nguồn: kiến thức Claude, số trang đối chiếu PDF 3rd ed.
> Lý thuyết process/scheduling đã có ở [OSTEP](../ostep/virtualization-cpu.md) — file này chỉ lấy phần **kernel hiện thực hóa thế nào**: struct nào, đường code nào, quyết định thiết kế nào.

---

## Cụm 1 — Process trong kernel: task_struct & fork (ch. 3, tr. 23–40)

### Nội dung chính

- **task_struct (tr. 24)** — "PCB của Linux": ~1.7KB chứa state, mm (address space — [memory.md](memory.md)), files, tín hiệu, cha/con/anh em (process tree tr. 29 — duyệt bằng `list_head`, cấu trúc ch. 6), scheduling info. Cấp từ **slab** (fork dày đặc — đúng bài kmem_cache). **`current`** (tr. 26): macro trả task đang chạy trên CPU này (thời sách: giấu ở đáy kernel stack qua thread_info; ⚠️ nay x86/arm64 để trong per-CPU/register — ý tưởng "luôn tìm được task của mình trong O(1)" giữ nguyên).
- **State (tr. 27):** TASK_RUNNING (đang chạy *hoặc* sẵn sàng — Linux không tách hai trạng thái này!), TASK_INTERRUPTIBLE (ngủ, signal đánh thức được — mặc định đúng), TASK_UNINTERRUPTIBLE (ngủ lì — cột `D` trong ps, không kill được: thường đang kẹt I/O; nguồn "load average cao mà CPU idle" kinh điển), __TASK_STOPPED, TASK_TRACED.
- **fork = clone (tr. 31–33):** mọi con đường tạo task (`fork/vfork/pthread_create`) đều xuống **`clone()`** với bộ cờ chia sẻ khác nhau → `copy_process()`: nhân bản task_struct từ slab, **COW address space** (copy page table, hai bên read-only — [OSTEP](../ostep/virtualization-memory.md)), copy/chia sẻ files/signal theo cờ. Chi tiết tinh tế (tr. 32): sau fork **chạy con trước** (nếu con exec ngay thì đỡ COW-fault vô ích của cha) — loại quyết định nhỏ đầy chủ đích đáng kể trong interview.
- **Thread = task chung tài nguyên (tr. 33):** `clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND)` — kernel **không có khái niệm thread riêng**, chỉ có task_struct chia sẻ nhiều/ít. Kernel threads (tr. 35): mm = NULL ([memory.md](memory.md)).
- **Exit & zombie (tr. 36–39):** `do_exit` giải phóng dần tài nguyên nhưng **giữ task_struct tối thiểu** cho cha đọc exit status — zombie; cha `wait` xong mới `release_task`. Cha chết trước → con **reparent về init** (tr. 38) — nguồn gốc kernel của câu zombie/orphan ([FILE 2 Q19](../../15_prep/technical_round/02_question_bank.md)).

### Góc interview

**Câu 1:** Trạng thái D (uninterruptible sleep) là gì, vì sao nguy hiểm, và liên hệ với "load average cao mà CPU idle"?

<details><summary>Đáp án</summary>

- `TASK_UNINTERRUPTIBLE` — ngủ **không nhận signal**: dùng khi task phải chờ trọn một thao tác không thể dở dang an toàn (thường I/O với thiết bị/NFS — thức dậy giữa chừng bởi Ctrl-C thì trạng thái phần cứng/dữ liệu lửng lơ).
- Nguy hiểm: **kill -9 vô tác dụng** (signal chỉ được xử lý khi task chạy lại — mà nó không dậy); task kẹt D vĩnh viễn = driver/storage/NFS bên dưới treo → chỉ còn reboot; nhiều task D là triệu chứng I/O subsystem có vấn đề.
- Load average của Linux đếm task **runnable + uninterruptible** → ổ đĩa/NFS chết làm hàng loạt task rơi vào D → load 50 trong khi CPU idle 95% — đọc load phải kèm `vmstat`/`ps aux | awk '$8 ~ /D/'` xem R hay D chiếm. Chẩn đoán task D: `cat /proc/<pid>/stack` (kẹt ở hàm kernel nào), `echo w > /proc/sysrq-trigger`.
- Điểm cộng: `TASK_KILLABLE` (2.6.25) — ngủ lì nhưng nhận fatal signal, chuẩn hiện đại cho chỗ chờ I/O dài.

</details>

---

## Cụm 2 — CFS: scheduler hiện thực (ch. 4, tr. 41–67)

### Nội dung chính

(Nguyên lý fair scheduling + so sánh thuật toán đã ở [OSTEP](../ostep/virtualization-cpu.md) — đây là phần "trong máu" Linux.)

- **Vấn đề của scheduler O(1) cũ (tr. 43–47):** timeslice cố định theo priority sinh đủ bệnh (nice 0 vs 19 chênh lệch phi lý, phụ thuộc HZ, interactive heuristics chắp vá) → CFS bỏ hẳn khái niệm timeslice cố định.
- **CFS (tr. 48–57):** mô hình "multitasking hoàn hảo" — mỗi task nhận **tỉ lệ** CPU theo weight (nice → weight, bảng hệ số ~1.25×/nấc):
  - **vruntime** (tr. 51): thời gian chạy chuẩn hóa theo weight — `scheduler_entity` ghi sổ;
  - chọn task: **vruntime nhỏ nhất** = node **trái nhất của red-black tree** (tr. 52–53; cached leftmost — O(1) đọc, O(log n) chèn/xóa);
  - timeslice động = `sched_latency × weight/tổng-weight`, chặn dưới `min_granularity`;
  - ngủ dậy: vruntime kéo về ~min tree — thưởng interactive *có giới hạn*, không tích lũy "nợ" (tr. 54).
- **Sleep & wakeup (tr. 58–61):** wait queue — pattern chuẩn chống lost wakeup: `add_wait_queue → set_current_state(TASK_INTERRUPTIBLE) → kiểm tra điều kiện lần cuối → schedule()` — đặt trạng thái **trước** khi kiểm tra là mấu chốt (đảo lại là race kinh điển — đúng bài [OSTEP CV](../ostep/concurrency.md) phiên bản kernel).
- **Preemption & context switch (tr. 62–64):** `need_resched` flag — set bởi tick/wakeup; kiểm tra tại **điểm quay về userspace** và **khi bật lại preemption** trong kernel (`preempt_count == 0`); `context_switch()` = `switch_mm` (đổi address space) + `switch_to` (registers/stack) — đúng hai nửa đã học ở OSTEP.
- **RT policies (tr. 64):** SCHED_FIFO/SCHED_RR nằm ở **rt sched_class xếp trên fair class** — mọi task RT runnable chặn toàn bộ CFS; syscall liên quan (tr. 65–66): `sched_setscheduler`, `sched_setaffinity`, `nice`.

### ⚠️ Đã thay đổi so với sách

- **CFS đã bị thay bằng EEVDF** (kernel 6.6): vẫn họ fair/vruntime nhưng chọn theo *virtual deadline sớm nhất* — công bằng + latency có kỷ luật hơn, bỏ đống heuristics wakeup. Khung khái niệm (weight, vruntime, rb-tree, sched_class) vẫn là nền để hiểu EEVDF.
- Thêm **SCHED_DEADLINE** (3.14 — EDF thật sự, đứng trên cả FIFO), core scheduling, EAS (energy-aware — chọn core theo năng lượng trên big.LITTLE: đúng chuyện embedded).

### Góc interview

**Câu 1:** Kernel quyết định "đến lúc phải reschedule" ở những điểm nào? Kernel preemption khác user preemption ra sao?

<details><summary>Đáp án</summary>

- Cờ **`need_resched`** được **set** khi: tick thấy task vượt phần công bằng (hoặc RT cao hơn dậy), wakeup task có quyền chen. Nhưng set cờ chưa phải switch — switch chỉ xảy ra ở **điểm kiểm tra**:
  1. **User preemption:** trên đường **quay về userspace** (sau syscall/interrupt) — luôn an toàn vì trạng thái kernel đã gói xong.
  2. **Kernel preemption** (CONFIG_PREEMPT): ngay **trong kernel** khi `preempt_count == 0` (không giữ spinlock/không trong atomic) — kiểm tra lúc bật lại preemption (`preempt_enable`) và khi interrupt trở về kernel code. Không có nó (PREEMPT_NONE/server), task trong kernel chạy tới khi tự ra — latency tệ.
- Chuỗi này chính là nền của [PREEMPT_RT](../melp/debug-realtime.md): RT = làm cho `preempt_count == 0` càng nhiều càng tốt (spinlock→rt_mutex chính là để đoạn giữ lock không còn tắt preemption).
- Điểm cộng: `preempt_count` per-task đếm lồng spinlock/BH/IRQ — một số nguyên quyết định "có được chen không"; xem qua `/proc/sys/kernel` không được — nó nằm trong thread_info/per-cpu.

</details>

---

## Cụm 3 — System call: từ trap đến sys_* (ch. 5, tr. 69–83)

### Nội dung chính

(Cơ chế trap/mode đã ở [OSTEP LDE](../ostep/virtualization-cpu.md) — đây là bản "giải phẫu Linux".)

- Đường đi (tr. 71–74): libc wrapper đặt **syscall number** vào register quy ước (+ tham số vào registers) → lệnh trap (`syscall`/`svc`) → `system_call` handler → tra **`sys_call_table[number]`** → `sys_read(...)` → trả về errno âm/kết quả → quay về user. Mỗi arch một bảng + một ABI truyền tham số.
- **Verify mọi thứ từ user (tr. 75–78)** — nguyên tắc an ninh số 1: pointer từ user **không được deref trực tiếp** — phải `copy_from_user`/`copy_to_user`: (1) kiểm tra địa chỉ thuộc user space; (2) xử lý page fault tử tế (trang chưa map → fault → nạp, hoặc trả -EFAULT — vì thế **có thể ngủ**, cấm trong atomic context); (3) chống đọc/ghi lộ dữ liệu kernel. `capable(CAP_SYS_ADMIN)` cho kiểm tra quyền (tr. 78).
- Syscall context (tr. 78): chạy trong **process context** thay mặt task gọi (`current` chính là nó) — ngủ được, schedule được — khác hẳn interrupt context.
- "Vì sao KHÔNG thêm syscall mới" (tr. 82) — lời khuyên đắt: syscall là **ABI vĩnh viễn** (không xóa/đổi được), cần số hiệu mọi arch, không mở rộng dễ; hầu hết nhu cầu driver → **char device + ioctl, sysfs, netlink** tốt hơn ([05/kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md)).

### ⚠️ Đã thay đổi so với sách

- Lệnh trap hiện đại (`syscall`/`sysenter` thay int 0x80 — sách có nói), **vDSO** (gettimeofday không trap), seccomp filter chặn syscall theo profile, io_uring như "đường vòng syscall theo lô" — các bổ sung nên biết tên khi bàn chi phí syscall.

### Góc interview

**Câu 1:** Vì sao kernel không được deref trực tiếp con trỏ từ userspace mà phải `copy_from_user`? Đủ các lý do.

<details><summary>Đáp án</summary>

1. **An ninh — pointer có thể trỏ vào kernel:** user truyền địa chỉ vùng kernel; kernel deref hộ rồi ghi kết quả về cho user = nguyên thủy đọc/ghi bộ nhớ kernel tùy ý (privilege escalation cổ điển). `copy_*_user` kiểm tra dải địa chỉ trước (`access_ok`).
2. **Đúng đắn — trang có thể chưa/không map:** địa chỉ hợp lệ về dải nhưng trang chưa nạp (demand paging) hoặc rác → deref trần gây fault trong kernel; `copy_*_user` có **bảng exception** — fault trong nó được handler định tuyến thành trả `-EFAULT` sạch sẽ thay vì oops.
3. **TOCTOU/SMAP:** user có thể đổi nội dung giữa hai lần đọc (copy một lần vào buffer kernel rồi mới validate — không validate tại chỗ trên bộ nhớ user); phần cứng hiện đại (SMAP/PAN trên ARM) **cấm kernel chạm bộ nhớ user tùy tiện** — copy_*_user mở cửa sổ hợp lệ có kiểm soát.
- Hệ quả context: copy_*_user **có thể ngủ** (page fault) → cấm khi giữ spinlock/trong ISR — nối lại đúng luật của [sync-timers.md](sync-timers.md). Trả lời đủ "an ninh + fault an toàn + TOCTOU/SMAP + có thể ngủ" là trọn vẹn.

</details>

### Đọc thêm (tùy chọn)

- [ostep/virtualization-cpu.md](../ostep/virtualization-cpu.md) — nền lý thuyết LDE/scheduling; [03/process-thread.md](../../03-operating-system/process-thread.md), [03/scheduling.md](../../03-operating-system/scheduling.md).
- [05/kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md) — ioctl/sysfs thay cho syscall mới.
