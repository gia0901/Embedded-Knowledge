# GDB — GNU Debugger

> **TL;DR**
> - Build với **`-g`** (debug symbol) và tốt nhất **`-O0`** (tối ưu làm code "nhảy lung tung", khó debug).
> - Luồng cơ bản: đặt **breakpoint** → `run` → khi dừng, xem **`backtrace`** (call stack), `print` biến, `step/next` từng dòng.
> - **Core dump**: phân tích crash *sau khi xảy ra* (post-mortem) — `gdb ./app core`, rồi `bt`.
> - **`attach`** vào process đang chạy (vd bị treo) để xem nó đang làm gì; `thread apply all bt` cho mọi thread (tìm deadlock).
> - **Watchpoint** dừng khi một biến *bị thay đổi* — mạnh để bắt "ai ghi đè giá trị này".

---

## 1. Chuẩn bị: build để debug được

```sh
g++ -g -O0 main.cpp -o app     # -g: debug symbol; -O0: không tối ưu
```
- **`-g`** nhúng thông tin debug (tên biến, số dòng, kiểu) → gdb ánh xạ địa chỉ máy ↔ source.
- **`-O0`**: với `-O2`, compiler inline/reorder/loại biến → step nhảy loạn, biến "optimized out". Khi buộc debug code đã tối ưu, dùng `-Og` (tối ưu vừa phải, giữ debug được).

---

## 2. Breakpoint & chạy

```gdb
gdb ./app
(gdb) break main              # hoặc: break file.cpp:42  /  break MyClass::foo
(gdb) break foo if x > 100    # conditional breakpoint — chỉ dừng khi điều kiện đúng
(gdb) run arg1 arg2           # chạy chương trình (kèm tham số)
(gdb) continue                # (c) chạy tiếp tới breakpoint kế
(gdb) info breakpoints        # liệt kê; delete N để xóa
```

- **Conditional breakpoint** cực hữu ích: thay vì dừng 1000 lần, chỉ dừng khi điều kiện nghi ngờ đúng.
- `tbreak`: breakpoint dùng một lần. `watch`/`rwatch`: xem mục 6.

---

## 3. Khi đã dừng: quan sát

```gdb
(gdb) backtrace               # (bt) call stack — đang ở đâu, được gọi từ đâu
(gdb) bt full                 # kèm biến local mỗi frame
(gdb) frame 2                 # (f 2) chuyển sang frame số 2 trong stack
(gdb) print x                 # (p) in giá trị biến/biểu thức
(gdb) print *ptr              # dereference
(gdb) print arr@10            # in 10 phần tử từ arr
(gdb) info locals             # mọi biến local
(gdb) info args               # tham số hàm hiện tại
(gdb) list                    # xem source quanh vị trí hiện tại
```

**`backtrace` là bạn thân nhất**: khi crash hay dừng, nó cho biết toàn bộ chuỗi gọi hàm dẫn tới đây. Đọc từ trên (chỗ dừng) xuống để hiểu ngữ cảnh.

---

## 4. Điều khiển thực thi

```gdb
(gdb) step       # (s) chạy một dòng, ĐI VÀO hàm được gọi
(gdb) next       # (n) chạy một dòng, BƯỚC QUA lời gọi hàm (không vào)
(gdb) finish     # chạy tới khi hàm hiện tại return (xem giá trị trả về)
(gdb) until      # chạy tới dòng cao hơn (thoát vòng lặp)
(gdb) continue   # chạy tiếp
```

`step` vào trong hàm để xem chi tiết; `next` để lướt qua khi tin hàm đó đúng. Đây là cách "đi bộ" qua logic để xem nó lệch kỳ vọng ở đâu.

---

## 5. Core dump — khám nghiệm sau crash

Core dump là ảnh chụp bộ nhớ process lúc crash → phân tích *sau* mà không cần tái hiện live.

```sh
ulimit -c unlimited                  # cho phép sinh core (kiểm tra /proc/sys/kernel/core_pattern)
./app                                # crash → sinh file core
gdb ./app core                       # nạp core
(gdb) bt                             # xem stack tại thời điểm crash → thường lộ ngay nguyên nhân
(gdb) print someVar                  # xem trạng thái biến lúc chết
```

Cực giá trị cho bug khó tái hiện hoặc crash ở production/field (embedded): thu core về phân tích offline. (`coredumpctl` trên hệ dùng systemd.)

---

## 6. Watchpoint — bắt "ai thay đổi giá trị này"

```gdb
(gdb) watch myVar             # dừng khi myVar bị GHI (thay đổi)
(gdb) rwatch myVar            # dừng khi myVar bị ĐỌC
(gdb) watch *(int*)0x601234   # theo địa chỉ
```

Khi một biến "bỗng nhiên" có giá trị sai mà không rõ ai sửa, watchpoint dừng đúng lúc nó bị ghi → `bt` cho thấy thủ phạm. Rất mạnh cho memory corruption / ghi đè ngoài ý muốn. (Hardware watchpoint nhanh; phạm vi lớn có thể chậm.)

---

## 7. Debug đa luồng

```gdb
(gdb) info threads                   # liệt kê mọi thread, * là thread hiện tại
(gdb) thread 3                       # chuyển sang thread 3
(gdb) thread apply all bt            # IN BACKTRACE CỦA MỌI THREAD
```

`thread apply all bt` là chìa khóa tìm **deadlock**: nhìn mọi thread đang kẹt ở đâu → thường thấy hai thread chờ lock của nhau. Kết hợp `attach` vào process bị treo:
```sh
gdb -p <PID>                         # attach vào process đang chạy
(gdb) thread apply all bt            # nó đang làm gì?
```

---

## 8. Remote debug — target không có GDB đầy đủ 🎯

Tình huống mặc định của Embedded Linux: binary chạy trên **target ARM**, nhưng target không đủ chỗ (hoặc không nên) cài cả GDB + symbol + source. Giải pháp: **tách đôi** — phần nhẹ chạy trên target, phần nặng chạy trên host.

```
   HOST (x86-64, có source + binary CÓ symbol)      TARGET (ARM, chỉ cần binary)
   ┌────────────────────────────────────┐          ┌──────────────────────────┐
   │ gdb-multiarch ./app                │◄── TCP ─►│ gdbserver :1234 ./app    │
   │  (đọc symbol, source, pretty-print)│  :1234   │  (~vài trăm KB, không    │
   │                                    │          │   cần symbol/source)     │
   └────────────────────────────────────┘          └──────────────────────────┘
```

```sh
# ── trên TARGET ──
gdbserver :1234 ./app                  # chạy mới
gdbserver :1234 --attach <pid>         # gắn vào process đang chạy

# ── trên HOST ──
gdb-multiarch ./app                    # ← binary CÓ symbol (bản chưa strip!)
(gdb) set sysroot /path/to/sdk/sysroot # để gdb tìm đúng libc/.so của TARGET
(gdb) set substitute-path /build/src /home/me/src   # ánh xạ đường dẫn source
(gdb) target remote 192.168.1.50:1234
(gdb) b main
(gdb) c
```

**Bốn chỗ hay sai — gần như luôn là một trong bốn cái này:**

| Triệu chứng | Nguyên nhân |
|---|---|
| Không có tên hàm, chỉ thấy `??` | Nạp nhầm **binary đã strip** trên host. Phải giữ bản **chưa strip** để debug, bản strip mới đem lên target (Yocto sinh sẵn ở `build/tmp/work/.../package/`) |
| Backtrace đứt khi vào thư viện | Chưa `set sysroot` → gdb đọc `.so` của **host** thay vì của target |
| Breakpoint theo dòng không khớp | Đường dẫn source lúc build khác lúc debug → `set substitute-path` (hoặc `dir`) |
| `Remote 'g' packet reply is too long` | **Sai kiến trúc**: dùng `gdb` x86 thay vì `gdb-multiarch` / `arm-linux-gnueabihf-gdb` |

**Biến thể theo tình huống:**
- **Crash ngoài field, không gắn được máy** → không dùng remote, dùng **core dump** (§5) mang về host: `gdb-multiarch ./app-chưa-strip core`.
- **Debug kernel/driver**, không phải userspace → `gdbserver` không dùng được; cần **KGDB** qua serial/ethernet, hoặc JTAG ([08/hardware-debug](../08-embedded-systems/hardware-debug.md)).
- **Bug timing/realtime** → GDB dừng CPU là phá luôn cái đang đo; dùng `perf`/ftrace/GPIO+scope thay vì breakpoint.

> Câu nên nói khi phỏng vấn: *"Trên target tôi chạy `gdbserver`, còn GDB đầy đủ + symbol + source ở host, nối qua TCP. Hai thứ phải khớp là **sysroot** (để đọc đúng thư viện của target) và **binary chưa strip** ở phía host."* Đây là câu phân biệt người từng debug trên board thật với người chỉ đọc tài liệu.

---

## 9. Mẹo thực dụng

- **`-tui`** hoặc `Ctrl-X A`: chế độ giao diện text hiện source + lệnh song song.
- **`.gdbinit`**: lưu lệnh khởi tạo (pretty-printer cho STL...).
- **pretty printing**: gdb hiện đại in `std::vector`, `std::string`... dễ đọc; `set print pretty on`.
- **`display x`**: tự in `x` mỗi lần dừng.
- Lệnh viết tắt: `b`, `r`, `c`, `n`, `s`, `p`, `bt` — dùng quen sẽ nhanh.
- **Reverse debugging** (`record` + `reverse-step`): chạy lùi để tìm điểm gây lỗi (hỗ trợ hạn chế, hữu ích khi có).

---

## 🧪 Bài NGỒI MÁY LÀM

> **Không phải câu trả lời miệng** — code có bug thật + nhiệm vụ + **output thật đã chạy** để đối chiếu. 10–15′/bài, chạy trên Linux.

| ID | Bài | Bẫy môi trường bài dạy luôn |
|----|-----|------------------------------|
| [DBG-033](../14-prep/mock-interview/bank/debugging.md) 🧪 | Segfault, shell báo `(core dumped)` nhưng **không có file core** → lấy `bt` bằng đường khác; frame #0 nằm trong libc thì đọc thế nào | `core_pattern` pipe vào apport ⇒ `ulimit -c` một mình **không đủ** |
| [DBG-034](../14-prep/mock-interview/bank/debugging.md) 🧪 | Daemon **treo**, không crash không log → `/proc` `State` + `wchan` định vị chỗ kẹt | `ptrace_scope=1` chặn `gdb -p` |

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DBG-001](../14-prep/mock-interview/bank/debugging.md) | Vì sao cần build với -g và nên dùng -O0 khi debug? |
| [DBG-004](../14-prep/mock-interview/bank/debugging.md) | Khi chương trình crash, bạn dùng gdb thế nào để tìm nguyên nhân? |
| [DBG-002](../14-prep/mock-interview/bank/debugging.md) | step và next khác nhau thế nào? |
| [DBG-003](../14-prep/mock-interview/bank/debugging.md) | Core dump là gì và dùng để làm gì? |
| [DBG-010](../14-prep/mock-interview/bank/debugging.md) | Làm sao điều tra một chương trình bị treo (hang/deadlock) bằng gdb? |
| [DBG-009](../14-prep/mock-interview/bank/debugging.md) | Watchpoint là gì? Khi nào dùng? |

---
⬅️ [mindset.md](mindset.md) · ➡️ Tiếp theo: [tools.md](tools.md)
