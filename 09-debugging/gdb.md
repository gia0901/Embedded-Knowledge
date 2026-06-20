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

## 8. Mẹo thực dụng

- **`-tui`** hoặc `Ctrl-X A`: chế độ giao diện text hiện source + lệnh song song.
- **`.gdbinit`**: lưu lệnh khởi tạo (pretty-printer cho STL...).
- **pretty printing**: gdb hiện đại in `std::vector`, `std::string`... dễ đọc; `set print pretty on`.
- **`display x`**: tự in `x` mỗi lần dừng.
- Lệnh viết tắt: `b`, `r`, `c`, `n`, `s`, `p`, `bt` — dùng quen sẽ nhanh.
- **Reverse debugging** (`record` + `reverse-step`): chạy lùi để tìm điểm gây lỗi (hỗ trợ hạn chế, hữu ích khi có).

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Vì sao cần build với -g và nên dùng -O0 khi debug?</summary>

`-g` nhúng thông tin debug (tên biến, kiểu, ánh xạ địa chỉ máy ↔ số dòng source) vào binary, để gdb có thể hiển thị code nguồn, in biến theo tên và đặt breakpoint theo dòng/hàm. `-O0` tắt tối ưu vì khi tối ưu (`-O2/-O3`), compiler inline hàm, sắp xếp lại lệnh, loại bỏ hoặc gộp biến — khiến việc step nhảy không theo thứ tự source và nhiều biến báo "optimized out", rất khó theo dõi. Khi buộc phải debug code tối ưu (vd bug chỉ xuất hiện ở bản release), dùng `-Og` để có mức tối ưu vừa phải mà vẫn debug được tương đối.
</details>

<details><summary>2) Khi chương trình crash, bạn dùng gdb thế nào để tìm nguyên nhân?</summary>

Cách trực tiếp nhất là dùng backtrace. Nếu chạy live: `run` trong gdb tới khi crash, rồi `bt` để xem call stack tại điểm crash — nó cho biết hàm nào lỗi và chuỗi gọi dẫn tới đó; dùng `frame N` để chuyển frame và `print`/`info locals` xem giá trị biến (vd con trỏ null, index sai). Nếu không tiện chạy live hoặc crash ở field, dùng **core dump**: bật `ulimit -c unlimited`, để chương trình crash sinh file core, rồi `gdb ./app core` và `bt` để phân tích post-mortem trạng thái lúc chết mà không cần tái hiện. Đọc kỹ stack và giá trị biến thường lộ ngay nguyên nhân (dereference null, truy cập ngoài mảng, gọi sai...).
</details>

<details><summary>3) step và next khác nhau thế nào?</summary>

Cả hai chạy một dòng nguồn rồi dừng, nhưng khác cách xử lý lời gọi hàm: `step` (s) **đi vào trong** hàm được gọi ở dòng đó để debug chi tiết nó; `next` (n) **bước qua** lời gọi hàm — thực thi toàn bộ hàm đó rồi dừng ở dòng kế tiếp của hàm hiện tại, không vào trong. Dùng `step` khi nghi ngờ hàm con có vấn đề và muốn theo dõi bên trong; dùng `next` khi tin hàm con đúng và chỉ muốn lướt qua. Ngoài ra `finish` chạy tới khi hàm hiện tại return (và in giá trị trả về), `until` giúp thoát vòng lặp.
</details>

<details><summary>4) Core dump là gì và dùng để làm gì?</summary>

Core dump là một ảnh chụp toàn bộ trạng thái bộ nhớ của tiến trình tại thời điểm nó crash (gồm stack, heap, thanh ghi). Nó cho phép phân tích **post-mortem**: nạp vào gdb (`gdb ./app core`) và dùng `bt`, `print`, `info locals` để xem call stack và giá trị biến lúc chết, mà không cần tái hiện lỗi trực tiếp. Điều này cực kỳ giá trị với bug khó tái hiện hoặc crash xảy ra ở production/thiết bị field (embedded): chỉ cần thu file core về máy có symbol để điều tra offline. Cần bật `ulimit -c unlimited` (và cấu hình `core_pattern`) để hệ thống sinh core khi crash.
</details>

<details><summary>5) Làm sao điều tra một chương trình bị treo (hang/deadlock) bằng gdb?</summary>

Attach gdb vào tiến trình đang chạy bằng `gdb -p <PID>` (không cần khởi động lại), rồi chạy `thread apply all bt` để in backtrace của **mọi** thread — xem từng thread đang kẹt ở đâu. Với deadlock, thường thấy hai (hoặc nhiều) thread cùng dừng trong hàm khóa mutex, mỗi thread đang chờ một lock mà thread kia đang giữ — chuỗi chờ vòng tròn lộ ra ngay. Với treo do vòng lặp vô tận hoặc chờ I/O, backtrace cho thấy thread chính kẹt ở đâu. Có thể kết hợp `info threads`, `thread N` để chuyển và xem chi tiết, hoặc dùng công cụ nhanh như `pstack`/`gstack`. Sau đó truy ngược thứ tự khóa để sửa (vd áp đặt thứ tự lock thống nhất).
</details>

<details><summary>6) Watchpoint là gì? Khi nào dùng?</summary>

Watchpoint (data breakpoint) khiến gdb dừng khi một biến hoặc vùng nhớ **bị thay đổi** (`watch`), bị đọc (`rwatch`), hoặc cả hai. Dùng khi một giá trị "bỗng nhiên" sai mà không biết đoạn code nào ghi đè nó — đặt watchpoint lên biến/địa chỉ đó, gdb sẽ dừng ngay tại lệnh ghi và `bt` chỉ ra thủ phạm. Rất hiệu quả cho lỗi memory corruption hoặc ghi đè ngoài ý muốn (vd buffer overflow ghi sang biến lân cận, con trỏ sai trỏ trúng biến này). Lưu ý hardware watchpoint nhanh nhưng giới hạn số lượng; watch vùng lớn bằng phần mềm có thể làm chương trình chạy chậm hẳn.
</details>

---
⬅️ [mindset.md](mindset.md) · ➡️ Tiếp theo: [tools.md](tools.md)
