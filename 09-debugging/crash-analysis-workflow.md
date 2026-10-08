# Phân tích crash — từ địa chỉ trong log tới dòng code

> **TL;DR**
> - Tài liệu này nối **luồng xử lý crash bạn đang làm thật** với kiến thức nền phía sau nó, ở **mức hiểu cơ bản**, kèm hai bài lab chạy được trên máy dev.
> - **Ba tình huống của bạn** (§1): bản **debug** — log tự in tên hàm · bản **release** — hệ thống nội bộ phân tích coredump từ image · **không boot được** — đọc trace log (oops/panic) rồi dùng `addr2line` + symbol của đúng image để ra chỗ chết trong package của mình.
> - **Một ý xuyên suốt** (§2): crash chỉ để lại **địa chỉ**. Biến địa chỉ thành *hàm + dòng* cần ba thứ: **offset** trong file (không phải địa chỉ tuyệt đối) · **file symbol khớp đúng bản build** · hiểu **địa chỉ trả về** lệch một lệnh.
> - 🧪 **Lab 1** (§3): log release chỉ có `libsensor.so(+0x112d)` ⟹ `addr2line` + file symbol ⟹ `sum_samples sensor.c:8`.
> - 🧪 **Lab 2** (§4): **lần đầu mở coredump bằng gdb** — `bt`, `frame`, `info locals`; thấy tận mắt vì sao thiếu file symbol thì ra `??`.
> - §5: phía kernel (oops/panic) dùng **cùng ý tưởng**, chỉ khác dạng địa chỉ `hàm+0xoff/0xsize`.

---

## 1. Ba tình huống của bạn — bằng chứng có gì, dùng công cụ gì

| Tình huống | Bằng chứng trong tay | Công cụ | Ra được gì |
|---|---|---|---|
| **Bản debug** | Log tự in backtrace kèm tên hàm | Đọc log | Hàm — nhưng chỉ hàm **có trong bảng symbol động**; hàm `static` vẫn chỉ hiện offset (Lab 1) |
| **Bản release** | Coredump + **image** đang chạy | Hệ thống phân tích nội bộ (bên trong là gdb + symbol của image) | Backtrace đầy đủ, biến cục bộ |
| **Không boot được** | Trace log: kernel oops / panic, hoặc log crash user-space | `addr2line` + **symbol của đúng image** | Hàm + dòng trong package của bạn |

**Nền chung của cả ba:** hệ thống phân tích nội bộ ở tình huống 2 làm **đúng việc** Lab 2 làm bằng tay. Hiểu Lab 2 thì giải thích được hệ thống đó đang làm gì, và làm được khi hệ thống không có sẵn.

---

## 2. Nền: từ địa chỉ tới dòng code cần ba thứ

**① Offset, không phải địa chỉ tuyệt đối.** Mỗi lần chạy, `.so` được nạp ở một **địa chỉ gốc** khác (ASLR). Địa chỉ trong file `.so` là cố định ⟹ `offset = địa chỉ tuyệt đối − địa chỉ gốc của .so`. Log `libsensor.so(+0x112d)[0x70a2134a512d]` đã tính sẵn offset; nếu log chỉ có địa chỉ tuyệt đối thì lấy địa chỉ gốc từ `/proc/<pid>/maps` (hoặc dòng `segfault at … in libx.so[gốc+kích thước]` của kernel).

**② File symbol khớp ĐÚNG bản build.** Bản ship lên thiết bị thường đã **strip** (bỏ debug info để nhỏ). Debug info được **tách ra file riêng** và giữ ở nhà:

| File | Chứa | Ở đâu |
|---|---|---|
| `libsensor.so` (đã strip) | Code + bảng symbol **động** (hàm export) | Trên thiết bị |
| `libsensor.debug` | **Debug info** (DWARF): địa chỉ ↔ file:dòng, tên biến, hàm `static` | Ở server build, gắn với **đúng** bản build đó |

Bản ship mang `.gnu_debuglink` — tên file debug + checksum — để công cụ tự tìm. Symbol của **bản build khác** (dù chỉ khác một dòng) cho ra dòng **sai** mà không báo lỗi — đó là lý do phải dùng *"symbol tương ứng image"*.

**③ Địa chỉ trả về lệch một lệnh.** Frame 0 (chỗ bị signal) là địa chỉ **đang chạy**. Các frame phía trên là **địa chỉ trả về** — lệnh **sau** lệnh `call`. Muốn ra đúng dòng **gọi**, tra `địa chỉ − 1`. Ở dòng đơn giản thì cùng một dòng; ở dòng có nhiều lệnh gọi liền nhau thì có thể lệch.

---

## 3. 🧪 Lab 1 — log release chỉ có offset ⟹ `addr2line` ra dòng code

**Đối tượng:** một thư viện `libsensor.so` ("package của bạn") có bug: cảm biến vắng mặt thì `buf == NULL`. App có crash handler in backtrace ra log — giống bản debug của bạn.

<details><summary><b>📦 Mã nguồn lab (3 file) + lệnh build</b></summary>

**`sensor.h`**
```c
#pragma once
#include <stddef.h>
int sensor_average(const int *buf, size_t n);
```

**`sensor.c`**
```c
/* libsensor.so — "package cua ban". Bug: cam bien vang mat thi buf == NULL. */
#include <stddef.h>
#include "sensor.h"

static int sum_samples(const int *buf, size_t n) {
    int s = 0;
    for (size_t i = 0; i < n; ++i)
        s += buf[i];                       /* <-- crash khi buf == NULL */
    return s;
}

int sensor_average(const int *buf, size_t n) {
    if (n == 0) return 0;
    return sum_samples(buf, n) / (int)n;
}
```

**`app.c`**
```c
/* app — "service" goi vao libsensor.so. Co crash handler in backtrace ra log
 * (giong ban debug: log tu in dia chi). LAB_NO_HANDLER=1 => de crash sinh core. */
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensor.h"

static void crash_handler(int sig) {
    void *frames[32];
    int n = backtrace(frames, 32);
    dprintf(STDERR_FILENO, "*** caught signal %d, backtrace:\n", sig);
    backtrace_symbols_fd(frames, n, STDERR_FILENO);   /* ghi thang ra fd, khong malloc */
    _exit(128 + sig);
}

static int read_light_sensor(int present) {
    static const int samples[4] = { 100, 120, 110, 130 };
    const int *buf = present ? samples : NULL;      /* cam bien vang mat => NULL */
    return sensor_average(buf, 4);
}

int main(int argc, char **argv) {
    if (!getenv("LAB_NO_HANDLER")) signal(SIGSEGV, crash_handler);
    int present = (argc > 1) ? atoi(argv[1]) : 1;
    printf("lux = %d\n", read_light_sensor(present));
    return 0;
}
```

**Build như một bản release thật — tách symbol rồi strip:**
```bash
gcc -g -O0 -fPIC -shared sensor.c -o libsensor.so.full
objcopy --only-keep-debug libsensor.so.full libsensor.debug            # file symbol giữ ở nhà
objcopy --strip-debug --strip-unneeded libsensor.so.full libsensor.so  # bản ship lên thiết bị
objcopy --add-gnu-debuglink=libsensor.debug libsensor.so
gcc -g -O1 app.c -L. -lsensor -Wl,-rpath,'$ORIGIN' -rdynamic -o app
```
Kích thước thật: `libsensor.so.full` 16680 byte · `libsensor.so` 14144 byte · `libsensor.debug` 5104 byte.
</details>

**Bước 1 — cho nó crash.** Output thật (đường dẫn rút gọn):
```
$ ./app 0
*** caught signal 11, backtrace:
./app(+0x127d)[0x56a00cd8d27d]
/lib/x86_64-linux-gnu/libc.so.6(+0x42520)[0x70a213242520]
libsensor.so(+0x112d)[0x70a2134a512d]
libsensor.so(sensor_average+0x35)[0x70a2134a517b]
./app(main+0x36)[0x56a00cd8d2ec]
/lib/x86_64-linux-gnu/libc.so.6(+0x29d90)[0x70a213229d90]
/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0x80)[0x70a213229e40]
./app(_start+0x25)[0x56a00cd8d185]
```
Đọc log:
- Dòng `libc.so.6(+0x42520)` là **bộ chuyển signal** của libc. Frame **ngay dưới** nó là chỗ bị crash: `libsensor.so(+0x112d)`.
- Chỗ crash **không có tên hàm** — `sum_samples` là `static`, không nằm trong bảng symbol động mà `backtrace_symbols` dùng. Cùng lý do, `crash_handler` (cũng `static`) hiện thành `./app(+0x127d)`.
- `sensor_average` **có** tên vì nó được export.

**Bước 2 — `addr2line` với file symbol.** Output thật:
```
$ addr2line -f -e symbols/libsensor.debug 0x112d
sum_samples
sensor.c:8 (discriminator 3)
```
Dòng 8 là `s += buf[i];` — đúng chỗ bug.

**Bước 3 — thiếu file symbol thì sao.** Cất `libsensor.debug` sang thư mục khác rồi chạy trên bản ship:
```
$ addr2line -f -e libsensor.so 0x112d
??
??:0
```
⚠️ Nhưng khi `libsensor.debug` **nằm cạnh** `libsensor.so`, chính lệnh đó ra đúng `sum_samples sensor.c:8`: `addr2line` đi theo `.gnu_debuglink` để tìm file symbol. Kiểm bằng:
```
$ readelf --string-dump=.gnu_debuglink libsensor.so
  [     0]  libsensor.debug
```

**Bước 4 — frame phía trên là địa chỉ trả về.** `sensor_average+0x35` = `0x1146 + 0x35` = `0x117b`:
```
$ addr2line -f -e libsensor.debug 0x117b      # địa chỉ trả về
sensor_average
sensor.c:14
$ addr2line -f -e libsensor.debug 0x117a      # trừ 1: lệnh gọi
sensor_average
sensor.c:14
```
Ở đây cùng dòng 14 (`return sum_samples(buf, n) / (int)n;`); quy tắc *"trừ 1"* để chắc chắn không bị lệch sang dòng kế.

**Bước 5 — nếu log chỉ có địa chỉ tuyệt đối:** `0x70a2134a512d − 0x112d = 0x70a2134a4000` — địa chỉ gốc của `.so`, tròn trang 4 KB. Có địa chỉ gốc (từ `/proc/<pid>/maps`) thì tự trừ ra offset.

**Bước 6 — dạng `hàm+offset` (giống kernel oops) tra bằng gdb:**
```
$ gdb -batch -q -ex 'info line *(sensor_average+0x35)' -ex 'list *(sensor_average+0x34)' libsensor.debug
Line 14 of "sensor.c" starts at address 0x117b <sensor_average+53> and ends at 0x1181 <sensor_average+59>.
0x117a is in sensor_average (sensor.c:14).
```

> ⚠️ `backtrace_symbols_fd` trong signal handler **không** nằm trong danh sách async-signal-safe chính thức; dùng được cho lab và thường thấy trong thực tế, nhưng có thể treo nếu crash xảy ra giữa lúc đang cấp phát. Hệ thống nghiêm túc ghi coredump thay vì tự in backtrace.

---

## 4. 🧪 Lab 2 — lần đầu mở coredump bằng gdb

**Coredump là gì:** ảnh chụp bộ nhớ và thanh ghi của process **ngay lúc chết**. Mang về máy khác vẫn phân tích được — **không cần tái hiện bug**.

**Bước 1 — sinh core.** Chạy không có handler để process chết thật:
```
$ ulimit -c unlimited
$ LAB_NO_HANDLER=1 ./app 0
Segmentation fault (core dumped)
```
⚠️ **Không thấy file `core` trong thư mục hiện tại?** Xem `cat /proc/sys/kernel/core_pattern`. Trên máy lab (Ubuntu 22.04) nó là `|/usr/share/apport/apport …` ⟹ core được **đẩy qua pipe** cho apport, rồi nằm ở `/var/lib/apport/coredump/core.<đường-dẫn>.<uid>.<boot-id>.<pid>.<time>`. Cùng lớp vấn đề với [DBG-033](../14-prep/mock-interview/bank/debugging.md). Trên thiết bị nhúng, `core_pattern` thường trỏ tới một script/daemon thu core về.

**Bước 2 — mở core khi THIẾU file symbol của `.so`.** Output thật:
```
$ gdb -batch -q -ex bt ./app core.app
Program terminated with signal SIGSEGV, Segmentation fault.
#0  0x00007e8c7768012d in ?? () from libsensor.so
#1  0x00007e8c7768017b in sensor_average () from libsensor.so
#2  0x00005ded84b582ec in read_light_sensor (present=<optimized out>) at app.c:21
#3  main (argc=2, argv=0x7fff7fb4c0c8) at app.c:27
```
Frame #0 là `??`: thiếu debug info của `libsensor.so`. `app` có `-g` nên frame #2, #3 vẫn ra dòng.

**Bước 3 — đặt file symbol cạnh `.so`, mở lại.** Output thật:
```
$ gdb -batch -q -ex bt -ex 'frame 0' -ex 'info locals' -ex 'p buf' -ex 'frame 1' -ex 'info args' ./app core.app
#0  0x00007e8c7768012d in sum_samples (buf=0x0, n=4) at sensor.c:8
#1  0x00007e8c7768017b in sensor_average (buf=0x0, n=4) at sensor.c:14
#2  0x00005ded84b582ec in read_light_sensor (present=<optimized out>) at app.c:21
#3  main (argc=2, argv=0x7fff7fb4c0c8) at app.c:27
#0  0x00007e8c7768012d in sum_samples (buf=0x0, n=4) at sensor.c:8
8	        s += buf[i];                       /* <-- crash khi buf == NULL */
i = 0
s = 0
$1 = (const int *) 0x0
#1  0x00007e8c7768017b in sensor_average (buf=0x0, n=4) at sensor.c:14
14	    return sum_samples(buf, n) / (int)n;
buf = 0x0
n = 4
```
Đọc ra: crash ở lần lặp **đầu tiên** (`i = 0`) vì `buf = 0x0`; `buf` đã là `NULL` từ **người gọi** (`sensor_average`, frame #1) ⟹ đi ngược lên `read_light_sensor`: cảm biến vắng mặt. Coredump cho **cả chuỗi nhân quả**, `addr2line` chỉ cho **một điểm**.

**Năm lệnh gdb đủ dùng cho core:**

| Lệnh | Làm gì |
|---|---|
| `bt` | Backtrace — chuỗi hàm gọi tới chỗ chết |
| `frame N` | Nhảy tới frame N |
| `info locals` · `info args` | Biến cục bộ · tham số của frame đang đứng |
| `p <biểu thức>` | In giá trị (`p buf`, `p *ptr`, `p arr[0]@4`) |
| `info sharedlibrary` | `.so` nào đã nạp, ở địa chỉ nào, **có symbol chưa** (cột `Syms Read`) |

**Mang core về host — điều kiện bắt buộc:** cùng **đúng bản** `app` + mọi `.so` + file symbol khớp. Với thiết bị ARM: dùng `gdb-multiarch`, `set sysroot <rootfs của image>` để gdb tìm `.so` đúng bản, `set solib-search-path` cho thư viện của bạn. *(Phần cross-target này là minh hoạ, lab chạy trên x86.)* Chi tiết: [gdb §5, §8](gdb.md).

---

## 5. Phía kernel — cùng ý tưởng, khác dạng địa chỉ

*(Minh hoạ — không chạy được trên máy dev vì cần build module kernel có debug info.)*

Kernel oops/panic in địa chỉ dạng **`hàm+0xoffset/0xkích-thước [module]`**, ví dụ:
```
pc : panel_set_freq+0x24/0x80 [drv_panel_chipA]
Call trace:
 panel_set_freq+0x24/0x80 [drv_panel_chipA]
 drv_panel_core_ioctl+0x5c/0x120 [drv_panel_core]
```

| Bước | User-space (Lab 1) | Kernel |
|---|---|---|
| Dạng địa chỉ | `libsensor.so(+0x112d)` hoặc `hàm+0x35` | `panel_set_freq+0x24/0x80 [drv_panel_chipA]` |
| File symbol | `libsensor.debug` của đúng bản build | `drv_panel_chipA.ko` build có `CONFIG_DEBUG_INFO`, **đúng bản trong image** (hoặc `vmlinux` nếu crash trong kernel lõi) |
| Tra ra dòng | `addr2line -e libsensor.debug 0x112d` · gdb `info line *(hàm+0x35)` | `scripts/faddr2line drv_panel_chipA.ko panel_set_freq+0x24/0x80` · hoặc `gdb drv_panel_chipA.ko` rồi `list *(panel_set_freq+0x24)` |
| Cả call trace | — | `scripts/decode_stacktrace.sh vmlinux <đường-dẫn-module> < oops.txt` |

Ý giống hệt §2: **offset trong file**, **symbol khớp đúng image**, và **địa chỉ trả về** ở các dòng call trace. Chi tiết kernel: [kernel-debugging](kernel-debugging.md).

---

## 6. Lab có sẵn trong bank — nên làm tiếp

| Lab | Vì sao sát việc của bạn |
|---|---|
| [DBG-033](../14-prep/mock-interview/bank/debugging.md) 🧪 | Shell báo `(core dumped)` mà không có file core — đúng hiện tượng apport ở Lab 2 |
| [DBG-039](../14-prep/mock-interview/bank/debugging.md) 🧪 | Tự strip binary rồi lấy lại backtrace có tên hàm — luồng bản release |
| [DBG-041](../14-prep/mock-interview/bank/debugging.md) 🧪 | Breakpoint trong `.so` chưa nạp — library của bạn nạp vào nhiều process |
| [DBG-034](../14-prep/mock-interview/bank/debugging.md) 🧪 | Daemon treo, không crash, không log — tình huống "hệ thống không chạy" phía user-space |

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DBG-045](../14-prep/mock-interview/bank/debugging.md) ⭐ | Ba tình huống crash (debug build / release / không boot) — bằng chứng gì, công cụ gì, cần gì để ra dòng code? |
| [DBG-043](../14-prep/mock-interview/bank/debugging.md) 🧪 | Log release chỉ có `libsensor.so(+0x112d)` — tìm ra hàm và dòng |
| [DBG-044](../14-prep/mock-interview/bank/debugging.md) 🧪 | Lần đầu mở coredump bằng gdb — vì sao frame 0 là `??`, sửa thế nào |
| [DBG-033](../14-prep/mock-interview/bank/debugging.md) 🧪 | `(core dumped)` mà không có file core |
| [DBG-039](../14-prep/mock-interview/bank/debugging.md) 🧪 | Strip binary rồi lấy lại backtrace có tên hàm |
| [RES-008](../14-prep/mock-interview/bank/resume.md) ⭐ | Debug xuyên tầng user ↔ kernel — kể một ca cụ thể |

---
⬅️ [Về README topic](README.md) · ➡️ [gdb.md](gdb.md) · [kernel-debugging.md](kernel-debugging.md)
