# OSTEP — Phần I: Virtualization (CPU) (ch. 4–10)

> Thuộc [OSTEP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Crux của phần này: *làm sao tạo ảo giác mỗi process có CPU riêng, trong khi CPU vật lý chỉ có vài cái — mà vẫn giữ hiệu năng và giữ quyền kiểm soát?*

---

## Cụm 1 — Process & Process API (ch. 4–5)

### Nội dung chính

**Process = chương trình đang chạy.** Program là file tĩnh trên đĩa; process là "machine state" sống: **address space** (memory nó thấy), **registers** (gồm PC — program counter, stack pointer), và **I/O state** (file descriptors đang mở). OS tạo ảo giác nhiều CPU bằng **time sharing**: luân phiên chạy các process.

**Trạng thái process** — máy trạng thái tối giản của sách (Linux thực tế thêm chi tiết nhưng cùng khung):

```
            scheduled (OS chọn chạy)
   READY ────────────────────────► RUNNING
     ▲  ◄──────────────────────────┘ │
     │      descheduled (hết lượt)    │ chờ I/O / event
     │                                ▼
     └────────────────────────── BLOCKED
          I/O xong / event đến
```

Insight quan trọng: process **BLOCKED không tốn CPU** — chính nhờ chuyển sang blocked khi chờ I/O mà OS overlap được I/O với tính toán của process khác. OS lưu mỗi process trong **PCB (process control block)** — chứa register context (để khôi phục khi context switch), state, PID, danh sách file mở...

**Process API của UNIX — bộ ba `fork()/exec()/wait()` (ch. 5):**

```c
pid_t rc = fork();               // NHÂN ĐÔI process hiện tại
if (rc < 0) { /* fail */ }
else if (rc == 0) {              // con: fork() trả về 0
    execvp("wc", args);          // THAY toàn bộ ảnh chương trình bằng wc
    // exec KHÔNG trả về nếu thành công — code sau exec chỉ chạy khi exec lỗi
} else {                         // cha: fork() trả về PID của con
    int status;
    wait(&status);               // chờ con kết thúc, thu hồi (tránh zombie)
}
```

- `fork()`: tạo bản sao (address space, fd, registers) — **một lời gọi, trả về hai lần**; khác nhau duy nhất ở giá trị trả về. Thứ tự chạy cha/con sau fork **không xác định** (scheduler quyết).
- `exec()`: **không tạo process mới** — nạp binary mới đè lên process hiện tại (code, data, heap, stack làm mới), **giữ nguyên** PID và bảng file descriptor.
- `wait()`: cha chờ con, đọc exit status, giải phóng PCB con.

**Vì sao tách fork/exec thay vì một hàm "spawn"? — insight đắt nhất chương.** Khoảng trống *giữa* fork và exec là chỗ shell làm phép: sửa soạn môi trường cho chương trình con **trước khi** nó chạy — chính là cách redirection/pipe hoạt động:

```c
// Shell thực hiện: wc file.txt > out.txt
if (fork() == 0) {
    close(STDOUT_FILENO);                    // đóng fd 1
    open("out.txt", O_CREAT | O_WRONLY);     // open lấy fd NHỎ NHẤT đang trống → 1!
    execvp("wc", args);                      // wc cứ ghi "màn hình" (fd 1) → vào file
}
// pipe: thay open bằng pipe() + dup2 — stdout của lệnh này nối stdin lệnh kia
```

`wc` không hề biết mình bị redirect — **cơ chế fd + quy ước "lấy số nhỏ nhất" + tách fork/exec** ghép thành tính năng lớn. Đây là ví dụ mẫu mực của triết lý UNIX: cơ chế nhỏ, ghép được.

### Insight đáng nhớ

- Ảo giác "CPU riêng" = time sharing + lưu/khôi phục context; ảo giác "memory riêng" = address space (phần Memory). Toàn bộ phần Virtualization chỉ là hai ảo giác này làm kỹ.
- fork/exec/wait là câu trả lời cho "process được tạo thế nào trên Linux" — kể cả `system()`, `posix_spawn`, container runtime... bên dưới vẫn là bộ ba này (Linux: `fork` nay là `clone`, COW — sách để dành cho phần memory).

### Ít quan trọng

- Chi tiết nạp chương trình (load ảnh từ đĩa, lazy loading — sách hẹn ở phần paging) (ch. 4).
- Các biến thể exec (`execl`, `execvp`, `execle`...) — chỉ khác cách truyền args/env (ch. 5).
- `kill()`, signal sơ lược — repo đã có chi tiết hơn ở [04/processes-signals.md](../../04-linux-system-programming/processes-signals.md) (ch. 5, cuối).

### Góc interview

**Câu 1:** `fork()` trả về gì? Sau fork, biến trong cha và con có chia sẻ không?

<details><summary>Đáp án</summary>

- Trả về **hai lần**: trong **cha** → PID của con (>0); trong **con** → `0`; lỗi → `-1` (chỉ trả một lần, không có con). Phân nhánh code cha/con dựa trên giá trị này.
- Biến **không chia sẻ**: con nhận **bản sao** address space tại thời điểm fork — sửa biến bên nào bên đó thấy, bên kia không. (Cơ chế thật trên Linux: **copy-on-write** — hai bên map chung page vật lý read-only, ai ghi thì kernel copy page đó ra riêng lúc page fault → fork rẻ dù address space lớn.)
- Những gì **chia sẻ thật**: bảng file descriptor được **nhân bản nhưng trỏ chung open file description** → chung file offset — hai bên cùng ghi một fd sẽ nối tiếp nhau chứ không đè.
- Bẫy hay hỏi kèm: gọi fork trong process nhiều thread → con chỉ có **một thread** (thread đang gọi fork); mutex do thread khác đang giữ sẽ kẹt vĩnh viễn trong con → quy tắc "sau fork trong tiến trình đa luồng, chỉ gọi async-signal-safe rồi exec ngay".

</details>

**Câu 2:** Zombie process là gì, khác orphan thế nào, xử lý ra sao?

<details><summary>Đáp án</summary>

- **Zombie**: con đã `exit()` nhưng cha **chưa `wait()`** — process chết nhưng PCB (PID, exit status) còn giữ để cha đọc. Không tốn RAM đáng kể nhưng **chiếm PID**; rò rỉ hàng loạt → cạn bảng process.
- **Orphan**: cha chết trước con — con được **init/systemd (PID 1) nhận nuôi**, PID 1 wait hộ nên orphan không thành zombie lâu dài.
- Xử lý zombie: (1) cha gọi `wait()/waitpid()` — blocking hoặc `WNOHANG` polling; (2) handler `SIGCHLD` gọi `waitpid(-1, ..., WNOHANG)` trong vòng lặp; (3) `signal(SIGCHLD, SIG_IGN)` — báo kernel tự thu dọn, không cần status; (4) kỹ thuật double-fork cho daemon (con fork cháu rồi exit ngay — cháu thành con của init).

</details>

---

## Cụm 2 — Limited Direct Execution: syscall & context switch (ch. 6) 🎯

### Nội dung chính

**Crux:** chạy nhanh → phải chạy **trực tiếp trên CPU thật** (direct execution); nhưng chạy trực tiếp thì OS làm sao (1) **ngăn process làm bậy**, (2) **lấy lại CPU** để chia cho process khác? Giải pháp = direct execution + **giới hạn** (limited) bằng hai cơ chế phần cứng.

**Cơ chế 1 — hai mode CPU + trap.** CPU có **user mode** (lệnh đặc quyền bị cấm: truy cập I/O, sửa bảng trang...) và **kernel mode** (làm được tất). Process chạy user mode; cần việc đặc quyền → **system call**:

```
User process                         Kernel
────────────                         ──────
read(fd, buf, n)
  → thư viện C đặt syscall number
    vào register, thực hiện TRAP ──► CPU: nâng lên kernel mode,
                                     nhảy vào TRAP HANDLER (địa chỉ do kernel
                                     đăng ký lúc BOOT qua trap table —
                                     process KHÔNG chọn được nhảy đâu!)
                                     ... kernel làm việc, kiểm tra tham số ...
  tiếp tục chạy sau read() ◄──────── return-from-trap: hạ về user mode,
                                     khôi phục registers, trả kết quả
```

Chi tiết đáng giá: khi trap, phần cứng tự lưu registers của process (vào **kernel stack** của process đó); trap table đăng ký **một lần lúc boot** (bản thân việc đăng ký là lệnh đặc quyền) — user code chỉ chọn được *syscall number*, không chọn được *địa chỉ nhảy* → kernel giữ toàn quyền kiểm soát điểm vào.

**Cơ chế 2 — timer interrupt: lấy lại CPU không cần xin phép.** Cách "cộng tác" (chờ process tự gọi syscall/yield) thất bại với vòng lặp vô hạn — **reboot là giải pháp duy nhất**. Giải pháp không cộng tác: **timer interrupt** — phần cứng phát ngắt định kỳ (vd mỗi vài ms), CPU tự động dừng process, nhảy vào interrupt handler của kernel → kernel có cơ hội quyết định: cho chạy tiếp hay **context switch**.

**Context switch** — kernel lưu register context của process A (vào PCB/kernel stack của A), nạp context của B, đổi kernel stack, return-from-trap "về" B như thể B chưa từng dừng:

```
Timer interrupt (đang chạy A)
  → HW lưu user registers của A vào kernel stack A, vào kernel mode
  → OS (scheduler) quyết định: chuyển sang B
  → switch(): lưu kernel registers A vào PCB(A); nạp PCB(B); đổi sang kernel stack B
  → return-from-trap → khôi phục user registers của B → B chạy tiếp
```

Chi phí context switch không chỉ là lưu/nạp registers (~sub-µs) mà chủ yếu là **chi phí gián tiếp**: nguội cache/TLB/branch predictor — lý do "ít context switch hơn" là mục tiêu thiết kế (liên hệ: spinlock vs mutex, event-driven ở phần Concurrency).

Câu hỏi kinh điển sách nêu: *interrupt đang xử lý mà interrupt khác đến?* — disable interrupt trong lúc xử lý (ngắn thôi!), hoặc các cơ chế lock nội bộ kernel — mở đường sang phần Concurrency.

### Insight đáng nhớ

- Cụm này là **câu trả lời chuẩn cho "syscall hoạt động thế nào"** — câu hỏi gần như chắc chắn gặp ở vị trí system software. Từ khóa phải bật ra: user/kernel mode, trap, trap table đăng ký lúc boot, kernel stack, return-from-trap, timer interrupt.
- "OS is just code + a trick": OS không "chạy ngầm giám sát" — OS **chỉ chạy khi được gọi** (trap) hoặc **bị kích** (interrupt). Giữa hai thời điểm đó, CPU hoàn toàn thuộc về user process. Hiểu điều này sửa được mental model sai rất phổ biến.

### Ít quan trọng

- Đo đạc chi phí syscall/context switch bằng lmbench và cách tự đo (ch. 6, aside).
- Lịch sử cooperative multitasking (Mac OS 9, Windows 3.x) (ch. 6, aside).

### Góc interview

**Câu 1 (🎯):** Trình bày đường đi đầy đủ của một lời gọi `read()` từ user code đến kernel và quay lại. Vì sao user process không thể "nhảy thẳng" vào code kernel?

<details><summary>Đáp án</summary>

1. User code gọi `read(fd, buf, n)` → thực chất gọi **wrapper trong libc**.
2. Wrapper đặt **syscall number** của read + tham số vào register theo quy ước ABI (x86-64: number vào `rax`, tham số `rdi/rsi/rdx`...), rồi thực thi lệnh **trap** (`syscall` trên x86-64, `svc` trên ARM).
3. CPU: nâng lên **kernel mode**, lưu user context (PC, registers) vào **kernel stack** của process, nhảy tới **địa chỉ handler đã đăng ký trong trap table lúc boot** (x86-64: MSR `LSTAR`).
4. Kernel: syscall dispatcher tra bảng theo number → `sys_read`; **kiểm tra tham số** (fd hợp lệ? `buf` nằm trong address space user? — không tin bất kỳ pointer nào từ user); thực hiện; có thể **block** process (chuyển BLOCKED, schedule process khác) nếu dữ liệu chưa sẵn.
5. Xong: đặt giá trị trả về vào register quy ước, **return-from-trap** (`sysret`/`eret`): khôi phục user context, hạ về user mode, chạy tiếp lệnh sau trap.
- Không nhảy thẳng được vì: (1) địa chỉ kernel được bảo vệ — user mode truy cập → fault; (2) điểm vào kernel **duy nhất qua trap table** mà chỉ kernel mode ghi được — user code chọn "số dịch vụ", không bao giờ chọn "địa chỉ chạy". Nếu cho nhảy tùy ý, mọi cơ chế bảo vệ sụp đổ.
- Điểm cộng: nhắc `vDSO` (một số "syscall" như `gettimeofday` không trap thật — kernel map sẵn data/code vào user space vì chỉ cần đọc), và chi phí mode switch là lý do batch syscall (`readv`, `io_uring`).

</details>

**Câu 2:** Context switch xảy ra khi nào, gồm những bước gì, và cái giá thật sự nằm ở đâu?

<details><summary>Đáp án</summary>

- **Khi nào:** (1) timer interrupt + scheduler quyết định hết lượt; (2) process **block** (syscall chờ I/O, lock, sleep); (3) process exit/yield; (4) preemption bởi process ưu tiên cao hơn thức dậy.
- **Các bước:** vào kernel (interrupt/trap) → lưu context A (registers, PC — phần user do HW/trap entry lưu, phần kernel do hàm switch lưu vào PCB) → chọn process kế (scheduler) → đổi address space (**nạp page table base register** — CR3/TTBR) → đổi kernel stack → nạp context B → return-from-trap về B.
- **Giá trực tiếp:** lưu/nạp registers + vào/ra kernel — cỡ trăm ns đến µs. **Giá gián tiếp (lớn hơn):** mất nóng **cache** (L1/L2 chứa dữ liệu process cũ), **TLB flush** (đổi address space — trừ khi có ASID/PCID), branch predictor — process mới chạy chậm một đoạn dài sau switch.
- Phân biệt ăn điểm: **thread switch cùng process** không đổi address space → không flush TLB → rẻ hơn process switch đáng kể; **mode switch** (syscall vào/ra kernel, không đổi process) còn rẻ hơn nữa — ba khái niệm hay bị nhập nhèm khi trả lời.

</details>

---

## Cụm 3 — Scheduling: từ FIFO đến MLFQ, CFS (ch. 7–9) 🎯

### Nội dung chính

**Hai họ metric xung đột (ch. 7):**
- **Turnaround time** = T_hoànthành − T_đến — đo hiệu quả cho batch/throughput.
- **Response time** = T_lầnđầuchạy − T_đến — đo tương tác (interactive).

Chuỗi phát triển thuật toán — mỗi cái sửa lỗi của cái trước:

| Thuật toán | Ý tưởng | Chết vì |
|---|---|---|
| **FIFO** | Đến trước chạy trước, chạy hết | **Convoy effect**: job ngắn kẹt sau job dài — turnaround tệ |
| **SJF** | Job ngắn nhất trước (non-preemptive) | Job ngắn đến *sau khi* job dài đã chạy → vẫn kẹt |
| **STCF** | SJF + preemption: ai còn ít nhất chạy | Turnaround tối ưu, nhưng response time tệ + **cần biết trước độ dài job** (không thể) |
| **RR (Round Robin)** | Chia lát thời gian (time slice), xoay vòng | Response tốt; turnaround **tệ nhất** (kéo dài mọi job); slice ngắn → chi phí context switch ăn hết |

Trade-off cốt lõi: **fair/response ↔ turnaround** — không thuật toán nào thắng cả hai. Chọn time slice là cân **amortization**: slice 10ms + switch 1ms → 10% overhead; slice 100ms → 1% nhưng phản hồi kém.

Thêm I/O: khi job block chờ I/O, scheduler chạy job khác — **overlap** I/O và CPU; job tương tác được xem như nhiều "sub-job ngắn" giữa các lần I/O.

**MLFQ — Multi-Level Feedback Queue (ch. 8): học quá khứ để đoán tương lai,** giải quyết "không biết trước độ dài job". Nhiều hàng đợi theo priority; bộ quy tắc:

```
Rule 1: priority cao chạy trước.        Rule 2: cùng priority → RR.
Rule 3: job mới vào → priority CAO NHẤT (giả định ngắn/tương tác — cho chứng minh).
Rule 4: dùng hết TỔNG allotment ở một mức → tụt xuống mức dưới
        (tính tổng, kể cả khi nhả CPU giữa chừng — chống GAME hệ thống
         bằng cách cố tình I/O ngay trước khi hết slice).
Rule 5: định kỳ PRIORITY BOOST — đẩy tất cả lên mức cao nhất
        (chống STARVATION của job dài, và cứu job đổi hành vi từ batch → interactive).
```

```
Q_cao   [interactive, job mới]   ← boost định kỳ đẩy tất cả lên đây
Q_giữa  [đã dùng kha khá CPU]
Q_thấp  [CPU-bound lâu dài]      ← tụt dần xuống theo lượng CPU đã dùng
```

MLFQ xấp xỉ SJF **không cần tiên tri** — nó *quan sát hành vi* (dùng hết slice → chắc là CPU-bound). Đây là khung của scheduler Windows, macOS, Solaris.

**Proportional share (ch. 9):** mục tiêu khác — không tối ưu turnaround/response mà **chia CPU theo tỉ lệ**:
- **Lottery**: mỗi process cầm số vé theo tỉ lệ; mỗi lượt quay số ngẫu nhiên — đơn giản, nhẹ, đúng tỉ lệ *theo kỳ vọng*.
- **Stride**: bản deterministic của lottery — mỗi process có stride ∝ 1/vé, ai có pass nhỏ nhất chạy rồi cộng stride — chính xác từng chu kỳ, nhưng cần giữ state toàn cục.
- **Linux CFS (Completely Fair Scheduler)** — cái thật sự cần nắm: mỗi task tích lũy **vruntime** (thời gian chạy ảo, cân theo **nice/weight** — nice thấp → vruntime tăng chậm → được chạy nhiều hơn); scheduler luôn chọn task có **vruntime nhỏ nhất** (lưu trong **red-black tree**, O(log n)); `sched_latency` chia đều cho số task ra time slice động (chặn dưới bởi `min_granularity`); task mới/mới thức dậy nhận vruntime ≈ min hiện tại (không được "trả thù" tích lũy). ⚠️ Từ kernel 6.6 CFS được thay bằng **EEVDF** — cùng họ fair scheduling, thêm khái niệm deadline ảo; sách chưa cover.

### Insight đáng nhớ

- Mạch "FIFO→SJF→STCF→RR→MLFQ" là **cách kể chuyện ăn điểm nhất** khi được hỏi về scheduling: mỗi thuật toán là một câu trả lời cho thất bại của cái trước — thể hiện hiểu *vì sao*, không phải thuộc lòng.
- MLFQ Rule 4 (tính tổng allotment) là bài học thiết kế hệ thống tổng quát: **mọi chính sách dựa trên hành vi đều sẽ bị game** — phải thiết kế sao cho gian lận không có lợi.
- Với embedded Linux: CFS lo phần "fair", còn **SCHED_FIFO/SCHED_RR (realtime class) đứng NGOÀI và trên CFS** — task RT runnable là CFS không được chạy. Liên hệ trực tiếp câu hỏi "làm sao đảm bảo task điều khiển chạy đúng hạn" — xem [03/scheduling.md](../../03-operating-system/scheduling.md).

### Ít quan trọng

- Ví dụ số turnaround/response từng bảng của ch. 7–8 — đọc lại khi cần bài tập tính tay (ch. 7 §7.x, ch. 8).
- Cài đặt lottery bằng danh sách + số ngẫu nhiên, và tranh luận ticket assignment (ch. 9).

### Góc interview

**Câu 1 (🎯):** So sánh SJF/STCF với Round Robin theo turnaround và response time. Vì sao không có scheduler "tốt nhất cho mọi việc"?

<details><summary>Đáp án</summary>

- **STCF** (SJF + preemption) **tối ưu turnaround**: luôn dồn sức cho job xong sớm nhất → tổng thời gian chờ nhỏ nhất. Nhưng response time tệ: job dài không được đụng CPU cho tới khi các job ngắn xong — người dùng gõ phím phải chờ.
- **RR** đảo ngược: mọi job sớm được chạy lát đầu tiên → **response tốt nhất**; nhưng vì kéo mọi job cùng về đích muộn (chia nhỏ đều), **turnaround gần như tệ nhất có thể**.
- Không có "tốt nhất" vì hai metric **mâu thuẫn về bản chất**: turnaround muốn *dồn toa* (chạy xong hẳn một job), response muốn *chia đều* (ai cũng được chạy sớm). Tối ưu cái này là hy sinh cái kia — scheduler thực tế (MLFQ, CFS) là các cách **thỏa hiệp có nguyên tắc**: đoán job tương tác để ưu tiên response, job nền thì chấp nhận turnaround.
- Điểm cộng: nêu thêm ràng buộc STCF phi thực tế (cần biết trước độ dài job) → MLFQ dùng lịch sử hành vi để xấp xỉ; và chi phí context switch giới hạn độ mịn của RR (amortization).

</details>

**Câu 2 (🎯):** MLFQ chống starvation và chống gian lận bằng quy tắc nào? Giải thích tình huống cụ thể từng quy tắc cứu.

<details><summary>Đáp án</summary>

- **Chống starvation — priority boost định kỳ (Rule 5):** không có nó, chỉ cần dòng job tương tác đến liên tục là job CPU-bound ở đáy **không bao giờ chạy**. Boost đẩy tất cả lên hàng đợi cao nhất mỗi chu kỳ T → job đáy tối thiểu được chạy theo nhịp boost. Boost cũng cứu job **đổi hành vi**: process từng cày CPU (bị tụt đáy) chuyển sang pha tương tác — không có boost nó kẹt ở đáy với hành vi mới.
- **Chống gian lận — tính tổng allotment (Rule 4):** phiên bản ngây thơ "hết slice mới tụt hạng, nhả CPU trước khi hết slice thì giữ hạng" bị khai thác: process xấu chạy 99% slice rồi **cố tình gọi I/O vu vơ** → giữ priority cao mãi, chiếm gần trọn CPU. Fix: đếm **tổng CPU đã dùng ở mỗi mức** bất kể nhả bao nhiêu lần — dùng đủ allotment là tụt, gian lận hết tác dụng.
- **Bẫy khi trả lời:** chỉ nhớ "job mới vào mức cao nhất" mà không giải thích *vì sao* (cho job cơ hội chứng minh mình ngắn/tương tác — nếu không phải, nó tự tụt); và quên rằng cả hai quy tắc trên tồn tại vì MLFQ **suy priority từ hành vi quá khứ** — suy được thì cũng giả mạo được.

</details>

**Câu 3:** CFS của Linux chọn task kế tiếp thế nào? `nice` ảnh hưởng ra sao? Task RT (SCHED_FIFO) đứng đâu trong bức tranh?

<details><summary>Đáp án</summary>

- CFS theo dõi **vruntime** của mỗi task = thời gian chạy thực × (weight chuẩn / weight của task). Luôn **chọn task có vruntime nhỏ nhất** — task chạy ít nhất so với "phần công bằng" của nó. Cấu trúc: **red-black tree** sắp theo vruntime, lấy min = node trái nhất, O(log n) mọi thao tác.
- **nice** (−20..+19) map sang **weight** theo bảng hệ số (~1.25×/bậc): nice thấp → weight lớn → vruntime **tăng chậm** → task "trông như chạy ít" → được chọn nhiều hơn. Nice không phải priority cứng — chỉ đổi **tỉ lệ chia CPU**.
- Time slice **động**: `sched_latency` (chu kỳ mà mọi task runnable được chạy ít nhất 1 lần) chia theo weight; chặn dưới `min_granularity` để không context switch quá dày. Task mới thức dậy được gán vruntime ≈ min của tree — ngủ lâu không được "gộp sổ đòi nợ".
- **SCHED_FIFO/SCHED_RR** thuộc **realtime class, xếp trên CFS tuyệt đối**: còn task RT runnable thì mọi task CFS chờ; FIFO chạy tới khi tự nhả/bị RT priority cao hơn cướp, RR thêm time slice giữa các task cùng priority. Embedded dùng cho luồng điều khiển cần deadline — kèm rủi ro chiếm trọn CPU (throttling `sched_rt_runtime_us` là lưới an toàn).
- ⚠️ Kernel ≥ 6.6: CFS đã bị thay bằng **EEVDF** — nếu bị hỏi sâu, nêu được tên và ý tưởng (chọn theo virtual deadline sớm nhất, vẫn họ fair) là điểm cộng lớn.

</details>

---

## Cụm 4 — Multiprocessor Scheduling (ch. 10)

### Nội dung chính

Đa CPU đặt thêm hai vấn đề mà scheduler đơn CPU không có:

1. **Cache coherence & affinity:** mỗi CPU có cache riêng; process chạy trên CPU A đã "làm nóng" cache A — schedule nó sang CPU B là chạy nguội (mọi truy cập bắn xuống memory). Phần cứng lo **coherence** (giao thức bus snooping/MESI — cache các CPU thấy giá trị nhất quán), nhưng **affinity** (giữ process ở CPU quen) là việc của scheduler.
2. **Đồng bộ chính scheduler:** hàng đợi chung là cấu trúc dữ liệu bị nhiều CPU truy cập → cần lock → nghẽn khi nhiều core.

Hai kiến trúc:

| | SQMS (single queue) | MQMS (multi queue) |
|---|---|---|
| Ý tưởng | 1 hàng đợi chung, CPU nào rảnh lấy việc | Mỗi CPU một hàng đợi riêng |
| Ưu | Load balance tự nhiên, đơn giản | Scale tốt (không tranh lock), affinity tự nhiên |
| Nhược | Lock contention khi nhiều core; phá affinity (job nhảy CPU) | **Mất cân bằng tải** (CPU này rảnh, CPU kia xếp hàng) → cần **work stealing/migration** — kéo việc từ queue đông sang, nhưng steal nhiều thì lại mất lợi thế |

Linux thực tế: per-CPU runqueue (họ MQMS) + cân bằng tải định kỳ theo **scheduling domains** (phân cấp SMT → core → socket/NUMA — ưu tiên di cư trong phạm vi gần trước để đỡ mất cache).

### Insight đáng nhớ

- Chương này là scheduling gặp **phần cứng thật**: câu trả lời đúng về multiprocessor scheduling luôn xoay quanh **cache** — affinity, coherence traffic, NUMA — chứ không phải thuật toán chọn job.
- Với embedded: `pthread_setaffinity_np`/`taskset` + `isolcpus` là công cụ trực tiếp từ bài học này — ghim luồng realtime vào core riêng để giữ cache nóng và tránh nhiễu (liên hệ Item 35 của [EMC++](../effective-modern-cpp.md) — lý do cần `native_handle`).

### Ít quan trọng

- Chi tiết ví dụ bus snooping và bàn luận lịch sử Linux O(1)/CFS/BFS ở multiprocessor (ch. 10, cuối).

### Góc interview

**Câu 1:** Cache affinity là gì và scheduler xử lý thế nào? Khi nào bạn chủ động ghim (pin) thread vào core trên embedded Linux?

<details><summary>Đáp án</summary>

- **Cache affinity:** process/thread vừa chạy trên core nào thì cache/TLB core đó đang giữ dữ liệu của nó — chạy lại nơi cũ nhanh hơn hẳn chạy nơi mới (tránh cache miss hàng loạt). Scheduler vì thế cố giữ task ở core cũ (per-CPU runqueue), chỉ **migrate** khi mất cân bằng tải đáng kể, và ưu tiên migrate trong phạm vi "gần" (cùng cluster/socket) theo sched domain.
- Chủ động pin (`pthread_setaffinity_np`, `taskset`, cgroup cpuset) khi: (1) luồng **realtime/latency-critical** — kết hợp `isolcpus`/`nohz_full` dành riêng core, loại nhiễu từ task khác và cả tick của kernel; (2) luồng xử lý **interrupt/DMA của thiết bị** — pin gần core nhận IRQ (IRQ affinity) để dữ liệu nằm sẵn cache; (3) tránh **NUMA remote access**; (4) benchmark cần kết quả ổn định.
- Trade-off phải nêu: pin tay = tắt load balancing của kernel cho task đó — pin sai chỗ thì core quá tải trong khi core khác rảnh; chỉ pin khi đo được lợi ích.

</details>

### Đọc thêm (tùy chọn)

- [03/process-thread.md](../../03-operating-system/process-thread.md), [03/scheduling.md](../../03-operating-system/scheduling.md) — bản cô đọng của repo.
- [04/processes-signals.md](../../04-linux-system-programming/processes-signals.md) — fork/exec/signal mức API Linux.
