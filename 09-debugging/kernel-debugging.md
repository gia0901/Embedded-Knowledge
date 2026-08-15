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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DBG-020](../14-prep/mock-interview/bank/debugging.md) | Vì sao debug kernel khó hơn debug userspace? |
| [DBG-021](../14-prep/mock-interview/bank/debugging.md) | printk/dmesg là gì và dùng thế nào trong driver? |
| [DBG-015](../14-prep/mock-interview/bank/debugging.md) | Khi gặp kernel oops, bạn đọc thông tin gì để tìm nguyên nhân? |
| [DBG-022](../14-prep/mock-interview/bank/debugging.md) | ftrace dùng để làm gì? |
| [DBG-023](../14-prep/mock-interview/bank/debugging.md) | Có những cách nào debug memory bug và deadlock trong kernel? |

---
⬅️ [memory-bugs.md](memory-bugs.md) · ➡️ Tiếp theo: [10-thinking/](../10-thinking/)
