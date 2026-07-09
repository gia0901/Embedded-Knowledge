# MELP — Debug, Profiling & Real-Time (ch. 16–21)

> Thuộc [MELP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Ch. 16 (Packaging Python) bỏ qua có chủ đích — ngoài trọng tâm BSP/C++. Ch. 17–18 (process/thread, memory) lướt vì trùng [OSTEP](../ostep/README.md) + topic 03/04. Trọng tâm file này: **debug từ xa, tracing, và PREEMPT_RT 🎯**.

---

## Cụm 1 — Process/Thread & Memory trên thiết bị (ch. 17–18) — lướt có chủ đích

### Nội dung chính (chỉ phần góc nhìn embedded, phần lý thuyết xem OSTEP)

- Chọn **process vs thread** trên thiết bị: sách nghiêng "process cho cô lập, thread khi buộc phải chia sẻ" — trùng khung [OSTEP concurrency](../ostep/concurrency.md); điểm nhấn embedded: **watchdog + restart từng process** là chiến lược tự phục hồi rẻ nhất (process chết được restart sạch, thread chết kéo cả process).
- Memory góc thiết bị nhỏ: hiểu **VSS/RSS/PSS/USS** (`smem`, `/proc/<pid>/smaps`) — RSS cộng các process **> RAM thật** vì shared lib đếm trùng; **PSS** (chia đều phần chung) mới là con số quy hoạch RAM đúng. Swap thường không có → tràn là gặp OOM killer ([OSTEP memory](../ostep/virtualization-memory.md) đã có câu hỏi này). `mtrace`/valgrind bắt leak — trên target yếu thì chạy dưới QEMU/host trước.

### Góc interview

**Câu 1:** Cộng RSS mọi process ra 900MB trên board 512MB RAM — vì sao vẫn chạy bình thường? Con số nào mới dùng để quy hoạch RAM?

<details><summary>Đáp án</summary>

- RSS đếm **mọi trang resident của process, kể cả trang chia sẻ** — libc.so map vào 30 process thì bị cộng 30 lần dù RAM chỉ có một bản; thêm nữa trang file-backed (code) đứng chung page cache. Tổng RSS vì thế vô nghĩa về "RAM đang tiêu".
- Dùng **PSS** (proportional set size — trang chia sẻ chia đều cho số người dùng: libc 1MB share 10 process → mỗi process cộng 100KB; tổng PSS ≈ RAM anonymous+file thật sự bị chiếm) — xem `smem` hoặc `/proc/<pid>/smaps_rollup`. **USS** (trang riêng hoàn toàn) = "giết process này lấy lại được bao nhiêu".
- Quy hoạch: tổng PSS các process + kernel (slab, page tables — `/proc/meminfo`) + page cache tối thiểu chấp nhận được + headroom cho spike; theo dõi áp lực bằng PSI thay vì chỉ "free" (free thấp là bình thường — cache tận dụng RAM trống).

</details>

---

## Cụm 2 — Debug từ xa với GDB (ch. 19) 🎯

### Nội dung chính

**Mô hình chuẩn trên embedded — target yếu, host mạnh:**

```
Target (board)                        Host (PC)
gdbserver :10000 ./app     ◄─TCP─►    aarch64-linux-gnu-gdb ./app-CÓ-SYMBOL
(nhẹ, không cần symbol)               (gdb) target remote <ip>:10000
                                      (gdb) set sysroot /path/to/rootfs-symbols
```

- **Symbol ở host, binary strip ở target**: rootfs sản phẩm strip cho nhỏ; giữ bản **chưa strip** (hoặc debug symbols tách riêng — Yocto: `dbg-image`/`-dbg` packages, `debug split`) trên host; `set sysroot` để GDB tìm đúng .so có symbol — thiếu bước này backtrace xuyên shared lib thành dấu hỏi.
- **Attach process đang chạy**: `gdbserver --attach :10000 <pid>` — debug daemon lỗi ngoài hiện trường không cần restart.
- **Core dump** — debug "hậu sự": bật `ulimit -c unlimited` + `/proc/sys/kernel/core_pattern` (ghi vào đường ổn định/piped tới handler); mở trên host: `cross-gdb app core` + sysroot → backtrace tại thời điểm chết. Với fleet: tự động thu core + symbol server là hạ tầng đáng đầu tư.
- Debug **kernel**: KGDB (qua serial — gdb vào chính kernel), nhưng thực chiến BSP dùng nhiều hơn: **oops/panic log decode** (`decode_stacktrace.sh`, addr2line từ vmlinux), JTAG/OpenOCD cho lúc kernel chưa sống ([09/kernel-debugging.md](../../09-debugging/kernel-debugging.md) chi tiết).
- VS Code/IDE remote debug bọc đúng cơ chế gdbserver này — biết lớp dưới thì cấu hình lớp trên không còn ma thuật.

### Insight đáng nhớ

- Ba chế độ phải thạo đủ: **live remote** (gdbserver), **attach** (đang chạy), **post-mortem** (core) — phỏng vấn debug hay xoáy "app chết ngoài hiện trường 1 lần/tuần, không tái hiện được ở lab, làm sao?" → đáp án là chế độ 3 + hạ tầng thu core, không phải "thêm log" (log là bổ trợ).

### Ít quan trọng

- Cú pháp lệnh GDB cơ bản (đã có [09/gdb.md](../../09-debugging/gdb.md)); Python pretty-printer cho STL — biết tồn tại.

### Góc interview

**Câu 1:** Daemon trên thiết bị khách hàng crash ngẫu nhiên vài ngày một lần. Không tái hiện được ở lab. Chiến lược của bạn?

<details><summary>Đáp án</summary>

1. **Hạ tầng post-mortem trước tiên**: bật core dump (`ulimit -c`, `core_pattern` ghi vào partition data, giới hạn kích thước/số lượng), giữ **binary + .so đúng phiên bản chưa strip** theo từng release (build ID khớp — Yocto giữ artifacts); thiết bị gửi core + log + version về (hoặc thu khi bảo trì). Một core tốt = backtrace, biến, thread — thường đủ đóng án.
2. **Nếu core không đủ** (heap corruption chết xa nguồn): build bản có **ASan** (nếu tài nguyên cho phép — x86 staging/QEMU chạy song song), hoặc trap sớm bằng `MALLOC_CHECK_`, electric fence; nghi race → TSan trên bản mô phỏng tải.
3. **Log có chủ đích**: ring buffer trong RAM flush khi crash (không phải log spam mòn flash); watchdog + restart để khách không gián đoạn trong lúc điều tra (nhưng **giữ core trước khi restart**).
4. Khoanh tương quan: crash trùng sự kiện gì (uptime — leak/overflow theo thời gian? giờ — cron/log rotate? thao tác khách?) từ metadata các lần chết.
- Điểm chấm: ứng viên nghĩ theo "hạ tầng bắt bằng chứng" thay vì "thử chạy lại và nhìn" — câu chữ then chốt: core dump + symbol khớp build + build ID.

</details>

---

## Cụm 3 — Profiling & Tracing (ch. 20)

### Nội dung chính

Nguyên tắc sách: **"đo trước, đoán sau"** — và chọn công cụ theo tầng câu hỏi:

| Câu hỏi | Công cụ |
|---|---|
| Cái gì đang ăn CPU? | `top/htop`, `perf top` |
| Hàm nào nóng? (sampling toàn hệ) | **perf record/report** (+ flame graph) |
| Syscall nào, bao lâu? | `strace -c -T` (chú ý: strace làm chậm đáng kể) |
| Chuyện gì xảy ra theo THỜI GIAN — scheduling, IRQ, latency? | **ftrace/trace-cmd + kernelshark**, sự kiện tĩnh tracepoint |
| Đường code kernel cụ thể | ftrace function_graph, kprobes |
| Tổng hợp lập trình được | eBPF/bcc (nếu kernel/toolchain cho phép) |

- **perf** trên embedded: cần `CONFIG_PERF_EVENTS` + perf build khớp; sampling ít xâm lấn — dùng được gần-sản-phẩm; kết quả cần **symbol** (lại bài -dbg/unstripped).
- **ftrace** là "dao mổ scheduling/latency": `trace-cmd record -e sched_switch -e irq*` → kernelshark nhìn timeline ai chiếm CPU, IRQ nào chen, wakeup-to-run mất bao lâu — công cụ đúng cho câu "thỉnh thoảng giật 50ms" mà profiler trung bình hóa mất.
- Phần cứng thật: đo nhiệt/throttling (cpufreq giảm tần số vì nhiệt làm "chậm bí ẩn"), đo I/O (`iostat`, blktrace) — flash chậm/mòn cũng hiện hình ở đây.

### Insight đáng nhớ

- Cặp khái niệm định hướng mọi buổi điều tra hiệu năng: **profiling trả lời "trung bình ai tốn"; tracing trả lời "khoảnh khắc đó chuyện gì xảy ra"** — chọn sai loại là điều tra mù (giật cục/latency spike phải dùng tracing; CPU nóng đều dùng profiling).

### Ít quan trọng

- Cài đặt/flags từng tool chi tiết (repo [09/tools.md](../../09-debugging/tools.md) đã có); Callgrind/gprof (nặng/lỗi thời trên target).

### Góc interview

**Câu 1:** Ứng dụng điều khiển thỉnh thoảng trễ chu kỳ 10ms một lần vài phút, CPU trung bình chỉ 30%. Điều tra thế nào?

<details><summary>Đáp án</summary>

- CPU trung bình thấp + spike hiếm → **bài tracing, không phải profiling** (perf report sẽ chỉ thấy 30% đều đều vô tội).
- Cách đánh: `trace-cmd record -e sched* -e irq* -e timer*` (thêm `-e block*`, `-e power*` nếu nghi I/O/cpufreq) chạy chờ dính spike — app tự phát hiện trễ (đo timestamp chu kỳ) và **ghi marker vào trace** (`trace_marker`) để tua đúng chỗ; mở kernelshark xem **khoảnh khắc trễ**: thread bị gì —
  1. **Bị chiếm CPU**: task khác priority cao hơn/cùng CFS chen (→ nâng SCHED_FIFO, affinity/isolcpus);
  2. **IRQ storm/softirq** (network burst, flash ops) chen ngang (→ IRQ affinity sang core khác, threaded IRQ hạ priority);
  3. **Chờ lock/tài nguyên**: mutex bị thread thấp giữ — **priority inversion** (→ PI mutex), hoặc chờ I/O (log ghi flash đồng bộ trong đường realtime — lỗi kiến trúc kinh điển);
  4. **Cpuidle/cpufreq**: core dậy từ idle sâu (exit latency) hay đang tụt tần số (→ giới hạn C-state, governor performance cho core RT);
  5. Page fault (major) do trang bị đuổi (→ mlockall).
- Trả lời nêu được "marker + timeline + 5 họ nghi phạm" là đúng nghề; chốt thêm cyclictest để định lượng nền tảng (cụm 4).

</details>

---

## Cụm 4 — Real-Time: PREEMPT_RT & thiết kế ứng dụng RT (ch. 21) 🎯

### Nội dung chính

**Định nghĩa đúng:** real-time = **deterministic** (deadline được đảm bảo/chặn trên), không phải "nhanh" — hệ RT có thể *chậm hơn* về throughput; thứ cần tối thiểu hóa là **latency đuôi (worst-case)**, không phải trung bình.

**Nguồn latency từ ngắt tới handler ứng dụng — chuỗi phải kể được:**

```
sự kiện phần cứng → [IRQ latency: ngắt bị mask bao lâu]
→ handler → [scheduling latency: preemption bị tắt bao lâu — đoạn kernel
   không nhường được CPU: spinlock giữ lâu, softirq dài...]
→ wakeup task RT → [chạy: cache/TLB nguội, page fault nếu chưa mlock]
```

**PREEMPT_RT** (patch lịch sử, các phần chính đã **mainline ~6.12** ⚠️) — biến kernel thành preemptible gần như mọi nơi:
- **Spinlock → sleeping lock (rt_mutex)** trong hầu hết kernel: đoạn giữ lock không còn tắt preemption → task RT chen được;
- **Hầu hết IRQ handler → threaded** (kernel thread có priority — RT task quan trọng hơn cả "ngắt" thường);
- Priority inheritance rộng rãi; tick/timer độ phân giải cao.
- Không có RT: `PREEMPT_NONE/VOLUNTARY/PREEMPT` — mức độ điểm nhường CPU trong kernel; PREEMPT_RT là nấc cuối.

**Đo:** **cyclictest** — chuẩn ngành: thread RT ngủ chu kỳ, đo sai lệch thức dậy; con số quan tâm là **Max** (worst-case) sau thời gian dài + **dưới tải trọng đại diện** (stress-ng, tải I/O — không tải thì số đẹp vô nghĩa). Kernel thường: max hàng ms–chục ms; PREEMPT_RT tuned: max ~vài chục–trăm µs.

**Checklist ứng dụng RT trên Linux** (đây là phần dùng hằng ngày):
1. `SCHED_FIFO` priority hợp lý (**không phải 99** — chừa chỗ cho threaded IRQ quan trọng hơn; và cẩn thận RT throttling `sched_rt_runtime_us`);
2. **`mlockall(MCL_CURRENT|MCL_FUTURE)`** + pre-fault stack/heap (chạm trước các trang) — cấm page fault trong đường RT;
3. Mutex dùng **PI** (`PTHREAD_PRIO_INHERIT`) — chống priority inversion;
4. **Cấm** trong vòng RT: malloc (khóa + có thể mmap/page fault), I/O đồng bộ, log ra flash, syscall không chặn trên xác định — pre-allocate tất cả, giao tiếp với phần non-RT qua **lock-free ring buffer**;
5. Cách ly: `isolcpus`/`cpuset` + IRQ affinity dồn core khác + `nohz_full` — core RT sạch nhiễu;
6. Giới hạn cpuidle sâu/cpufreq cho core RT (exit latency).

### Insight đáng nhớ

- Câu trả lời chuẩn cho "Linux có phải RTOS không?": **"Linux vanilla: không có bảo đảm; PREEMPT_RT tuned: soft/firm RT mức trăm µs đo được bằng cyclictest; hard RT an toàn tính mạng: RTOS riêng hoặc kiến trúc lai (core MCU/AMP chạy vòng điều khiển + Linux làm phần giàu tính năng — chuẩn của SoC hiện đại: Cortex-A + Cortex-M)."** Ba nấc, mỗi nấc có bằng chứng.
- **Priority inversion** phải kể được như chuyện thuộc lòng: task cao chờ mutex mà task thấp đang giữ, task trung chen giữa chiếm CPU của task thấp → cao chờ vô hạn; thuốc: **priority inheritance** (chủ mutex được "mượn" priority người chờ). Kèm tên Mars Pathfinder làm bằng chứng lịch sử.

### Ít quan trọng

- Xenomai/RTAI (co-kernel — đường cũ, thu hẹp dần khi PREEMPT_RT mainline); chi tiết bảng so latency từng phiên bản kernel (ch. 21 giữa).

### Góc interview

**Câu 1 (🎯):** PREEMPT_RT làm gì với kernel để giảm worst-case latency? Vì sao nó có thể làm *giảm* throughput?

<details><summary>Đáp án</summary>

- Hai đòn chính: (1) **spinlock thường của kernel → rt_mutex ngủ được** — trước đây giữ spinlock = tắt preemption (task RT phải chờ đoạn kernel đó xong, dù dài); giờ đoạn giữ lock **preempt được**, task RT chen ngay; (2) **IRQ handler → kernel thread có priority** — "ngắt" không còn mặc nhiên trên cơ mọi thứ: vòng điều khiển RT của bạn có thể đặt **cao hơn** handler card mạng. Kèm: priority inheritance phủ rộng (chuỗi khóa trong kernel không gây inversion), high-res timer, tối thiểu vùng tắt preemption còn lại.
- **Giảm throughput vì**: mỗi spinlock giờ là mutex có thể ngủ → thêm chi phí switch/quản lý thay vì spin vài chục ns; IRQ qua thread = thêm một lần schedule mỗi ngắt; nhiều điểm preempt = nhiều switch hơn, cache nguội hơn. Bản chất: **đổi throughput trung bình lấy chặn trên latency** — đúng định nghĩa RT (deterministic ≠ nhanh).
- Điểm cộng: biết trạng thái mainline (~6.12, ARM64/x86 trước), và rằng bật PREEMPT_RT **không đủ** — vẫn phải tune app (mlock, PI mutex, isolcpus, cyclictest kiểm chứng dưới tải).

</details>

**Câu 2 (🎯):** Giải thích priority inversion và priority inheritance. Vì sao semaphore không có thuốc chữa này?

<details><summary>Đáp án</summary>

- **Inversion:** L (thấp) giữ mutex M; H (cao) cần M → block; M (trung) không liên quan M nhưng priority > L → **chiếm CPU của L** → L không chạy được để nhả M → H (cao nhất hệ!) chờ theo — thứ tự ưu tiên bị lộn ngược, thời gian chờ của H vô chừng (Mars Pathfinder reset liên tục vì đúng kịch bản này).
- **Priority inheritance (PI):** khi H block trên M, **chủ M (L) tạm được nâng lên priority của H** → L vượt mặt M (trung), chạy xong nhả M, tụt về priority cũ → thời gian H chờ = đúng đoạn critical section của L, chặn trên được. (Giải pháp khác: priority ceiling — mutex có trần, ai giữ chạy ở trần.)
- **Semaphore không có PI được** vì PI cần biết **chủ sở hữu để nâng ai** — semaphore *không có khái niệm chủ* ([OSTEP concurrency](../ostep/concurrency.md): thread A wait, thread B post là hợp lệ, không ai "giữ" semaphore). Đây là lý do kỹ thuật khiến quy tắc "mutex cho mutual exclusion, semaphore cho signaling" thành nguyên tắc cứng trong hệ RT.
- Thực dụng: pthread mutex phải khai `PTHREAD_PRIO_INHERIT` **tường minh** (mặc định là NONE!) — chi tiết nhỏ này phân biệt người từng viết app RT thật.

</details>

### Đọc thêm (tùy chọn)

- [09-debugging/](../../09-debugging/) — gdb, tools, kernel debugging bản đầy đủ của repo.
- [03/scheduling.md](../../03-operating-system/scheduling.md) + [OSTEP virtualization-cpu](../ostep/virtualization-cpu.md) — SCHED_FIFO đứng đâu so với CFS.
- [OSTEP concurrency](../ostep/concurrency.md) — lock, semaphore: nền của inversion/PI.
