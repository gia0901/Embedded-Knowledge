# Kernel Debugging — Debug kernel & driver

> **TL;DR**
> - Kernel không debug như userspace: không gdb chạy thẳng, lỗi = sập cả hệ thống. Công cụ chính: **`printk`/`dmesg`**, **oops/panic log**, **ftrace**, **dynamic debug**, và **kgdb** (cần máy thứ hai).
> - **`printk`** + xem qua **`dmesg`** là chủ lực (như printf của kernel), có mức log (`KERN_ERR`, `KERN_DEBUG`...).
> - **Kernel oops**: lỗi nghiêm trọng nhưng có thể không sập ngay; **panic**: sập hẳn. Cả hai in **call stack + register + thông tin** → đọc để tìm nguyên nhân.
> - **ftrace**: tracer tích hợp kernel — theo dõi function call, latency, sự kiện mà không cần build lại.
> - **kgdb/KASAN**: debug bằng gdb qua máy thứ hai; KASAN = AddressSanitizer cho kernel (bắt lỗi bộ nhớ trong kernel).

---

## 1. Vì sao kernel khó debug hơn userspace?

- **Không có ranh giới bảo vệ**: bug trong kernel (con trỏ sai) làm hỏng cả hệ thống/panic, không phải chỉ một process crash cô lập.
- **Không gdb trực tiếp**: bản thân kernel là cái chạy gdb của userspace — không thể đơn giản "gdb ./kernel". Debug live cần cơ chế đặc biệt (kgdb + máy host thứ hai, hoặc JTAG trên embedded).
- **Ngữ cảnh hạn chế**: trong interrupt handler không được ngủ, không gọi nhiều hàm; công cụ phải phù hợp ngữ cảnh.
- → Phần lớn debug kernel dựa vào **để lại dấu vết** (log, trace) và **đọc crash dump**, hơn là step live.

---

## 2. `printk` & `dmesg` — chủ lực

`printk` là "printf của kernel", ghi vào **kernel ring buffer**, đọc bằng `dmesg`:

```c
printk(KERN_ERR  "mydrv: failed to map, err=%d\n", ret);
pr_info("mydrv: probe ok\n");      // macro tiện: pr_err/pr_warn/pr_info/pr_debug
dev_err(&pdev->dev, "init failed\n");  // gắn kèm tên device — nên dùng trong driver
```
```sh
dmesg                  # xem ring buffer
dmesg -w              # theo dõi realtime (như tail -f)
dmesg --level=err     # lọc theo mức
```

- **Mức log** (`KERN_EMERG`...`KERN_DEBUG`) điều khiển cái gì hiện ra (console loglevel).
- `dev_err/dev_dbg` (gắn device) tốt hơn `printk` trơn trong driver — kèm ngữ cảnh thiết bị.
- Như userspace: đây là "log đúng chỗ" — vũ khí số một, đặc biệt cho bug khó.

---

## 3. Đọc Oops & Panic

Khi kernel gặp lỗi nghiêm trọng, nó in một **oops** (hoặc **panic** nếu không thể tiếp tục). Bản dump chứa thông tin vàng:

```
BUG: kernel NULL pointer dereference at 0x0000000000000018
...
RIP: 0010:my_driver_read+0x2c/0x80 [my_driver]     ← hàm + offset gây lỗi
Call Trace:                                          ← stack: dẫn tới đây thế nào
 vfs_read+0x...
 ksys_read+0x...
 do_syscall_64+0x...
Modules linked in: my_driver ...                     ← module liên quan
```

Cách đọc:
- **Dòng lỗi**: loại lỗi (vd NULL pointer deref) + địa chỉ.
- **RIP/PC**: hàm + offset đang chạy khi lỗi → `my_driver_read+0x2c` cho biết driver & hàm.
- **Call Trace**: chuỗi gọi dẫn tới lỗi (như backtrace).
- Dùng **`addr2line`** / `gdb` trên file `.ko` (có debug symbol) để map offset → dòng source:
  ```sh
  addr2line -e my_driver.ko 0x2c
  ```
- "**tainted**" flag: kernel bị "làm bẩn" bởi module non-GPL/đã oops trước → lưu ý khi phân tích.

---

## 4. ftrace — tracer tích hợp

ftrace là framework trace mạnh, không cần build lại kernel, điều khiển qua `/sys/kernel/tracing`:

```sh
cd /sys/kernel/tracing
echo function > current_tracer      # trace mọi function call trong kernel
echo my_driver_read > set_ftrace_filter   # chỉ trace hàm quan tâm
cat trace                           # xem kết quả

echo function_graph > current_tracer  # đồ thị gọi hàm + thời gian mỗi hàm
```

Dùng cho:
- **function tracer**: hàm nào được gọi, theo thứ tự nào.
- **function_graph**: cây gọi hàm + thời lượng → tìm hàm chậm/latency.
- **các tracer chuyên biệt**: `irqsoff` (vùng tắt interrupt lâu — quan trọng cho realtime), `wakeup` (latency lập lịch), event tracing (`sched`, `irq`...).
- Công cụ cấp cao dựng trên ftrace: **trace-cmd**, **perf**, và **bpftrace/eBPF** (rất mạnh, lập trình được).

---

## 5. Dynamic debug & cấu hình debug

```sh
# Bật pr_debug() của một file/module lúc runtime mà không build lại
echo 'file my_driver.c +p' > /sys/kernel/debug/dynamic_debug/control
echo 'module my_driver +p' > /sys/kernel/debug/dynamic_debug/control
```
- **Dynamic debug** cho phép bật/tắt từng `pr_debug`/`dev_dbg` theo file/hàm/dòng lúc chạy → lấy log chi tiết khi cần mà bình thường im lặng.
- Kernel debug options khi build: `CONFIG_DEBUG_KERNEL`, `CONFIG_DEBUG_INFO` (symbol), `CONFIG_KASAN`, `CONFIG_LOCKDEP`...

---

## 6. Công cụ chuyên sâu

| Công cụ | Dùng cho |
|---------|----------|
| **kgdb** | Debug kernel bằng gdb qua máy host thứ hai (serial/ethernet) — đặt breakpoint, step, inspect như userspace |
| **KASAN** | Kernel Address Sanitizer — bắt buffer overflow/UAF trong kernel (như ASan), build với `CONFIG_KASAN` |
| **LOCKDEP** | Phát hiện nguy cơ deadlock/thứ tự khóa sai lúc runtime |
| **kdump/crash** | Thu thập & phân tích **vmcore** (crash dump toàn kernel) sau panic — như core dump cho kernel |
| **JTAG** | Debug phần cứng cấp thấp trên embedded (cả bootloader/kernel sớm), cần đầu nối JTAG |
| **eBPF/bpftrace** | Quan sát kernel có lập trình, an toàn, không cần module — hiện đại & rất mạnh |

---

## 7. Chiến lược thực tế cho driver embedded

1. **`dev_dbg`/dynamic debug** rải ở các điểm quan trọng của driver (probe, ISR setup, I/O).
2. Khi oops: đọc **Call Trace + RIP**, dùng `addr2line` map về dòng → khoanh vùng nhanh.
3. Nghi memory bug trong driver → build kernel với **KASAN**.
4. Vấn đề latency/realtime → **ftrace** (`irqsoff`, `function_graph`).
5. Bug sâu cần step → **kgdb** (QEMU rất tiện: chạy kernel trong QEMU + gdb attach) hoặc **JTAG** trên board thật.
6. Panic ngẫu nhiên ở field → cấu hình **kdump** để thu vmcore phân tích offline.

> Mẹo: nhiều bug driver tái hiện được trong **QEMU** + kgdb dễ hơn nhiều so với debug trên board thật — tận dụng nếu có thể.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Vì sao debug kernel khó hơn debug userspace?</summary>

Vì kernel không có ranh giới bảo vệ như process userspace: một bug (vd dereference con trỏ sai) trong kernel có thể làm hỏng hoặc sập cả hệ thống (oops/panic) thay vì chỉ crash một process cô lập. Không thể chạy gdb trực tiếp lên kernel đang chạy như với một chương trình userspace — kernel chính là nền mà gdb userspace dựa vào; debug live cần cơ chế đặc biệt (kgdb với máy host thứ hai, hoặc JTAG trên embedded). Ngoài ra một số ngữ cảnh kernel (interrupt handler) bị hạn chế nghiêm ngặt (không được ngủ, không gọi nhiều hàm), nên công cụ phải phù hợp. Hệ quả là debug kernel dựa nhiều vào để lại dấu vết (printk, ftrace) và phân tích crash dump hơn là step trực tiếp.
</details>

<details><summary>2) printk/dmesg là gì và dùng thế nào trong driver?</summary>

`printk` là hàm in của kernel (tương đương printf), ghi vào kernel ring buffer; người dùng đọc qua lệnh `dmesg` (có `dmesg -w` để theo dõi realtime, lọc theo mức). Nó có các mức log (`KERN_ERR`, `KERN_WARNING`, `KERN_INFO`, `KERN_DEBUG`...) quyết định thông điệp nào hiện ra console theo loglevel hiện tại. Trong driver nên dùng các macro tiện như `pr_err/pr_info/pr_debug`, và đặc biệt `dev_err/dev_dbg` vì chúng gắn kèm tên thiết bị vào thông điệp, cho ngữ cảnh rõ ràng. Như ở userspace, đặt log đúng chỗ (probe, cấu hình IRQ, đường I/O, nhánh lỗi) là cách điều tra chủ lực, nhất là với bug khó tái hiện.
</details>

<details><summary>3) Khi gặp kernel oops, bạn đọc thông tin gì để tìm nguyên nhân?</summary>

Bản oops chứa: dòng mô tả lỗi (vd "NULL pointer dereference" kèm địa chỉ), thanh ghi **RIP/PC** chỉ hàm và offset đang chạy khi lỗi (vd `my_driver_read+0x2c [my_driver]` cho biết module và hàm), **Call Trace** là chuỗi gọi hàm dẫn tới điểm lỗi (như backtrace), và danh sách "Modules linked in" cùng cờ tainted. Cách điều tra: xác định hàm gây lỗi từ RIP và module liên quan, đọc Call Trace để hiểu đường dẫn thực thi, rồi dùng `addr2line -e my_driver.ko <offset>` (hoặc gdb trên `.ko` có debug info) để map offset về đúng dòng source. Kết hợp với hiểu loại lỗi (NULL deref → con trỏ chưa khởi tạo/đã free) để truy nguyên nhân gốc.
</details>

<details><summary>4) ftrace dùng để làm gì?</summary>

ftrace là framework tracing tích hợp sẵn trong kernel, điều khiển qua `/sys/kernel/tracing`, không cần build lại kernel. Nó cho phép: trace các function call trong kernel (function tracer) để biết hàm nào được gọi và theo thứ tự nào; vẽ cây gọi hàm kèm thời lượng mỗi hàm (function_graph) để tìm hotspot/latency; và dùng các tracer chuyên biệt như `irqsoff` (đo các vùng tắt interrupt lâu — quan trọng cho realtime), `wakeup` (latency lập lịch), hay event tracing cho các subsystem (sched, irq, block...). Nó là nền cho các công cụ cấp cao như trace-cmd, perf và một phần eBPF. Rất hữu ích để điều tra vấn đề hiệu năng, latency và luồng thực thi trong kernel mà printk khó nắm bắt.
</details>

<details><summary>5) Có những cách nào debug memory bug và deadlock trong kernel?</summary>

Cho memory bug trong kernel: build kernel với **KASAN** (Kernel Address Sanitizer) — tương tự AddressSanitizer ở userspace, bắt buffer overflow và use-after-free trong kernel/driver và in báo cáo với stack trace nơi truy cập, cấp phát và giải phóng. Cho deadlock và lỗi thứ tự khóa: bật **LOCKDEP** (`CONFIG_PROVE_LOCKING`) — nó theo dõi thứ tự acquire các khóa lúc runtime và cảnh báo khi phát hiện khả năng deadlock (vd hai khóa được lấy theo hai thứ tự ngược nhau ở các đường khác nhau) ngay cả khi deadlock chưa thực sự xảy ra. Ngoài ra có thể dùng kgdb để step và inspect, kdump/crash để phân tích vmcore sau panic, và eBPF/bpftrace để quan sát động. Các tùy chọn debug này thường bật khi phát triển và tắt ở bản production vì có overhead.
</details>

---
⬅️ [memory-bugs.md](memory-bugs.md) · ➡️ Tiếp theo: [10-thinking/](../10-thinking/)
