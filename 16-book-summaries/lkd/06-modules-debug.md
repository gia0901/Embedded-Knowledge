# Ch. 17, 18 & 19 — Devices & Modules, Debugging, Portability (tr. 337–361, 363–378, 379–393)

> Thuộc [LKD](README.md) · **[⏮ 05 VFS/Block/Page Cache](05-vfs-block-pagecache.md)** · **⏭ (hết — file 06)**
> Nguồn: **đọc trực tiếp PDF** (Linux Kernel Development, Robert Love, 3rd ed. 2010), **trang sách = trang PDF − 27**.
> Ký hiệu: không đánh dấu = nội dung sách · **🆕 = bổ sung/liên hệ ngoài sách** · **⚠️ = cần cẩn trọng / sách lỗi thời** · **🎯 = câu hỏi phỏng vấn kinh điển** · trích dẫn kèm `(tr. X)`.
> Ba chương khép sách, chuyển từ *subsystem cụ thể* sang *nghề viết & gỡ kernel*: **ch. 17** = kernel **tổ chức thiết bị** thế nào (module nạp động, kobject/sysfs = cây thiết bị dưới `/sys`); **ch. 18** = **gỡ lỗi kernel** (printk, oops, SysRq, git bisect — không có debugger như userspace); **ch. 19** = viết code **portable** (word size, endian, alignment, char signedness). Ví dụ xương sống: module **`fishing.ko`** (ch.17), một **oops back trace thật** (ch.18).

---

## Cụm 1 — Devices, Modules, kobject & sysfs (ch. 17, tr. 337–362)

### 1.1 Ba loại device (tr. 337–338)

Unix chia device làm **ba loại**:
- **Block device** (blkdev) — địa chỉ theo **block**, **seek** được; truy cập qua **block device node**, thường mount làm filesystem (đĩa, flash — [05](05-vfs-block-pagecache.md)).
- **Character device** (cdev) — **luồng byte** tuần tự; app tương tác **trực tiếp qua char device node** (bàn phím, chuột, pseudo-device).
- **Network device** — truy cập mạng; ⚠️ **phá nguyên tắc "everything is a file"** — không có device node, dùng **socket API**.

🆕 Thêm: **miscdev** (char device đơn giản hóa), **pseudo device** (`/dev/null`, `/dev/zero`, `/dev/random` — driver ảo, không phần cứng thật).

### 1.2 Module — nạp/gỡ code động (tr. 338–348)

Kernel **monolithic** (chạy một address space) **nhưng modular** — nạp/gỡ code lúc runtime. Module (`.ko`) = *loadable kernel object*: base kernel tối thiểu + tính năng/driver nạp riêng khi cần (hot-plug).

**Hello World (tr. 339):**
```c
#include <linux/module.h>
static int hello_init(void) {                 // ENTRY — kernel gọi khi nạp
        printk(KERN_ALERT "I bear a charmed life.\n");
        return 0;                              // 0 = thành công; khác 0 = fail (phải unwind)
}
static void hello_exit(void) {                 // EXIT — gọi khi gỡ (dọn dẹp)
        printk(KERN_ALERT "Out, out, brief candle!\n");
}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");                          // ⚠️ non-GPL → set cờ "tainted"
MODULE_AUTHOR("Shakespeare");
```
🆕 **`MODULE_LICENSE` (tr. 340)** hai tác dụng: (1) non-GPL nạp vào → kernel **tainted** (dev bỏ qua bug report vì nghi module binary không debug được); (2) **non-GPL không gọi được GPL-only symbol** (`EXPORT_SYMBOL_GPL`). Nếu compile **tĩnh vào kernel** thì `init` chạy lúc boot, `exit` không tồn tại (code tĩnh không gỡ được).

**Build (kbuild, tr. 341):** `obj-$(CONFIG_FISHING_POLE) += fishing.o` → build `fishing.ko`. **Nạp (tr. 344):** `insmod`/`rmmod` (**ngu** — không giải phụ thuộc) vs **`modprobe`** (giải phụ thuộc từ `modules.dep` do `depmod` sinh — **khuyến nghị**). **Kconfig (tr. 345):** `tristate` (Y=nội, M=module, N=không), `depends on`/`select`.

**Module parameter (tr. 346):** `module_param(name, type, perm)` — user đặt lúc boot/nạp, **hiện trong sysfs**. **Exported symbol (tr. 348):** `EXPORT_SYMBOL(fn)` / `EXPORT_SYMBOL_GPL(fn)` — chỉ symbol export mới gọi được từ module (module link động, như userspace).

### 1.3 Device Model & kobject (tr. 348–352)

**Device model (tr. 348):** cây thống nhất biểu diễn **mọi thiết bị + topology**. 🆕 **Động cơ gốc là power management (tr. 349):** tắt nguồn phải **từ lá lên gốc** — tắt chuột USB *trước* USB controller *trước* PCI bus → cần cây thiết bị chính xác. Lợi ích kèm: liệt kê thiết bị, link **device↔driver**, phân loại theo class.

**kobject (tr. 349)** — hạt nhân, *"similar to the Object class in... C# or Java"*: cho struct kernel **ref count + tên + parent** (dựng cây):
```c
struct kobject {
        const char       *name;
        struct kobject   *parent;     // → dựng cây (chính là /sys)
        struct kset      *kset;        // nhóm ("mọi block device")
        struct kobj_type *ktype;       // hành vi chung (release/sysfs_ops/default_attrs)
        struct kref       kref;        // ref count
        ...
};
struct cdev {                          // kobject NHÚNG trong struct khác:
        struct kobject kobj;           // cdev có kobject → vào được cây device model
        const struct file_operations *ops;
        ...
};
```
Ba struct liên quan (tr. 350–351): **ktype** (kobj_type — **hành vi chung** của một họ kobject: `release`, `sysfs_ops`, `default_attrs`); **kset** (**tập hợp** kobject liên quan, vd "mọi block device" — có `uevent_ops` cho hotplug). 🆕 Phân biệt: **ktype = shared operations, kset = grouping** (ít ktype, nhiều kset).

**kref (tr. 354):** ref count chuẩn — `kref_get` (+1), `kref_put(kref, release)` gọi `release` khi về 0 (`atomic_dec_and_test`). Khuyến nghị dùng `kref` thay vì tự chế `atomic_t` get/put.

### 1.4 sysfs — cây kobject dưới `/sys` (tr. 355–362)

**sysfs (tr. 355):** filesystem ảo trong RAM, **view của cây kobject**. Ma thuật đơn giản: **kobject ↔ dentry** ([05](05-vfs-block-pagecache.md)) → kobject map thành thư mục → cây kobject **là** cây `/sys`. Thư mục gốc (tr. 357): `block`, `bus`, `class`, `dev`, **`devices`** (quan trọng nhất — **topology thiết bị thật**), `module`, `power`. Nhiều thư mục khác chỉ là **cách tổ chức lại** dữ liệu của `devices` (vd `/sys/class/net/eth0` symlink về `devices`).

**File trong sysfs = attribute (tr. 358):** map biến kernel → file. `sysfs_ops` có **`show`** (đọc → copy giá trị vào buffer) / **`store`** (ghi → đọc từ buffer vào biến). Quy ước (tr. 360): **một giá trị một file**, text → dễ đọc/ghi từ shell. ⚠️ **sysfs là user-space ABI** — *"Changing existing files in any way is discouraged"* (userspace dựa vào vị trí/tên/hành vi). 🆕 sysfs **thay ioctl/procfs** cho cấu hình driver hiện đại.

**Kernel event / uevent (tr. 361):** `kobject_uevent(kobj, KOBJ_ADD/REMOVE/CHANGE)` phát sự kiện (nguồn = sysfs path của kobject) qua **netlink** lên userspace → **udev**/D-BUS. 🆕 Đây là **nền của hotplug hiện đại** (cắm USB → uevent → udev tạo `/dev` node).

### Insight đáng nhớ (cụm 1)

- **`/sys` = cây kobject** (tr. 355): device model dựng cây (`parent`) cho **power management** (tắt từ lá lên gốc), và sysfs chỉ là **export cây đó** ra userspace. Hiểu điều này là hiểu vì sao `/sys/devices` phản ánh topology thật, các thư mục khác là view.
- **kobject = "Object base class kiểu C"** nhúng vào struct (như `cdev.kobj`) — cùng pattern OOP-in-C với VFS `s_op`/`f_op` ([05](05-vfs-block-pagecache.md)) và VMA `vm_ops` ([04](04-memory.md)). `kref` là ref-count chuẩn của kernel.
- **`MODULE_LICENSE`/tainted/`EXPORT_SYMBOL_GPL`** là ranh giới pháp lý-kỹ thuật: non-GPL module bị hạn chế symbol + làm bug report mất tin cậy.

### ⚠️ Đã thay đổi so với sách

- **udev → systemd-udevd** (udev sáp nhập systemd); uevent qua netlink vẫn nguyên. **devtmpfs** tự tạo `/dev` node sớm lúc boot.
- Driver hiện đại hiếm chạm kobject trực tiếp — dùng **device model API cấp cao** (`struct device`, `platform_driver`, `struct class`, `device_create`); **Device Tree** ([melp](../melp/README.md)) mô tả topology thay hard-code. Nhóm attribute qua `ATTRIBUTE_GROUPS`/`DEVICE_ATTR`.
- Build ngoài cây: `make -C /lib/modules/$(uname -r)/build M=$PWD` (`M=` thay `SUBDIRS=`). `EXTRA_CFLAGS` → `ccflags-y`.

### Góc interview (cụm 1)

**Câu 1:** `/sys` là gì, và nó liên quan device model / kobject thế nào? Vì sao device model ra đời?

<details><summary>Đáp án</summary>

- **Device model (tr. 348):** cây thống nhất biểu diễn mọi thiết bị + topology (bus↔device↔driver). **Động cơ gốc: power management** — tắt nguồn phải **từ lá lên gốc** (chuột USB → USB controller → PCI bus), cần cây chính xác. Hạt nhân là **kobject** (`struct kobject` — ref count + tên + `parent` dựng cây, thường nhúng trong struct như `cdev.kobj`).
- **`/sys` = export cây kobject ra userspace (tr. 355):** kobject ↔ dentry → mỗi kobject là một thư mục; `/sys/devices` phản ánh **topology thật**, các thư mục khác (`class`, `bus`) là **view tổ chức lại**. File dưới đó = **attribute** (map biến kernel, `show`/`store`).
- **Điểm cộng:** sysfs là **user-space ABI** (không đổi tên/vị trí tùy tiện); thay ioctl/procfs cho cấu hình; **uevent qua netlink → udev** tạo `/dev` node khi hot-plug. Driver hiện đại dùng `struct device`/`platform_driver` (bọc kobject) + Device Tree.

</details>

---

## Cụm 2 — Debugging kernel (ch. 18, tr. 363–378)

### 2.1 Vì sao gỡ kernel khó & bắt đầu từ đâu (tr. 363–364)

Khác userspace: *"A fault in the kernel can bring down the whole system"* (tr. 363), và thành công gỡ lỗi **là hàm của mức hiểu kernel**. Cần: **một bug rõ ràng**, **phiên bản kernel** có bug, **hiểu code** liên quan. Điểm mấu chốt (tr. 363): *"Your success in debugging relies on your ability to duplicate the problem"* — **tái hiện được là đi được nửa đường**. Bug kernel thường mờ hơn userspace (race chỉ hiện 1/triệu lần, tùy cấu hình/máy).

🆕 **Chuỗi sự kiện điển hình (tr. 364):** struct thiếu ref count → race → process A free trong khi B còn dùng → B deref con trỏ chết → **NULL deref → oops** (hoặc đọc rác → corrupt → hành vi sai). Dev phải **lần ngược** từ triệu chứng về nguyên nhân (thêm ref count). Đây là mẫu bug kernel phải nhận ra.

### 2.2 printk — in để gỡ (tr. 364–367)

`printk()` ~ `printf()` nhưng **robust (tr. 365):** gọi được từ **bất cứ đâu** — interrupt context, đang giữ lock, nhiều CPU cùng lúc **không cần lock**. ⚠️ **Chỉ chết một chỗ:** trước khi console init (boot sớm) → dùng **`early_printk()`** (không portable mọi arch). **Loglevel (tr. 365):**
```c
printk(KERN_WARNING "cảnh báo!\n");   // <4>
printk(KERN_DEBUG   "debug!\n");      // <7>
```
Kernel in ra console mọi message có loglevel **dưới `console_loglevel`**. Thang: `KERN_EMERG`(`<0>` — hệ chết) → ... → `KERN_DEBUG`(`<7>` — thừa). **Log buffer (tr. 366):** vòng tròn `LOG_BUF_LEN` (mặc định 16KB) → đầy thì đè cái cũ (đơn giản, robust, dùng được từ interrupt). **`klogd`** đọc buffer → **`syslogd`** ghi `/var/log/messages`. 🆕 Nay là `dmesg`/journalctl.

### 2.3 Oops — kernel báo "có chuyện" (tr. 367–369)

Kernel không tự sửa/tự kill như userspace → phát **oops**: in error + **dump register + back trace**. ⚠️ **Oops ở đâu quyết định hậu quả (tr. 368):** trong **interrupt context** hoặc **idle(pid 0)/init(pid 1)** → **panic (dừng máy ngay)**; process khác → **giết process, cố chạy tiếp**. Oops thật (PPC, tr. 368):
```
Unable to handle kernel NULL pointer dereference at virtual address 00000001
NIP: C013A7F0 ...   TASK = c0712530[0] 'swapper'
Call trace:
[c013ab30] tulip_timer+0x128/0x1c4      ← handler timer gây NULL deref
[c0020744] run_timer_softirq+0x10c/0x164 ← chạy trong softirq (timer — [03])
[c001b864] do_softirq+0x88/0x104
[c0007e80] timer_interrupt+0x284/0x298
[c0007bf8] cpu_idle+0x34/0x38            ← máy đang idle
```
🆕 **Đọc back trace = đọc chuỗi gọi hàm dẫn tới lỗi** (tr. 369): ở đây máy idle → timer interrupt → timer softirq → `tulip_timer()` deref NULL. **Offset `0x128/0x1c4`** chỉ đúng dòng lỗi. Register cho biết register nào chứa NULL. **Decode địa chỉ→tên hàm:** `ksymoops` (cũ, cần `System.map`) → **`CONFIG_KALLSYMS`** (nay, kernel tự nhúng bảng symbol → oops tự decode).

### 2.4 Công cụ gỡ: option, BUG, SysRq (tr. 370–372)

**Debug options (tr. 370)** (menu *Kernel Hacking*, cần `CONFIG_DEBUG_KERNEL`): slab/spinlock/stack-overflow debug, và quan trọng nhất **sleep-inside-spinlock** (`CONFIG_DEBUG_SPINLOCK_SLEEP` / atomicity) — bắt lỗi **ngủ khi atomic** ([03](03-sync-timers.md)): `schedule()`/`kmalloc(GFP_KERNEL)`/ngủ khi giữ lock → in cảnh báo + back trace. 🆕 Đây là bộ bắt bug số một cho lỗi khóa.

**Assert/dump (tr. 370):** `BUG_ON(condition)` (lệnh bất hợp lệ → oops; wrap `unlikely`), `panic("...")` (dừng máy), **`dump_stack()`** (chỉ in register + back trace, không chết), `BUILD_BUG_ON` (**compile-time**).

**Magic SysRq (tr. 371):** `CONFIG_MAGIC_SYSRQ` + `echo 1 > /proc/sys/kernel/sysrq`. Tổ hợp phím "nói chuyện với kernel bất kể nó đang làm gì" — **cứu máy đang treo**: **`SysRq-s`** (sync đĩa), **`SysRq-u`** (unmount), **`SysRq-b`** (reboot) — gõ 3 cái này **an toàn hơn nhấn nút reset**; `SysRq-t` (dump task), `SysRq-m` (memory), `SysRq-p` (register). 🆕 Mnemonic thứ tự an toàn: **"Raising Elephants Is So Utterly Boring"** (R-E-I-S-U-B).

### 2.5 Debugger & mẹo poke hệ thống (tr. 372–376)

⚠️ **Linus không muốn debugger trong tree** (tr. 372): *"debuggers lead to bad fixes by misinformed developers"* — fix từ **hiểu code thật** đúng hơn. Có thể dùng:
- **`gdb vmlinux /proc/kcore`** — đọc biến/disassemble, nhưng **chỉ đọc** (không sửa data, không breakpoint/single-step).
- **`kgdb`** — gdb đầy đủ **từ xa qua serial** (2 máy: một chạy kernel patch kgdb, một debug) — breakpoint/watchpoint/single-step.

**Mẹo poke (tr. 373–376):** **UID conditional** (`if (current->uid != 7777) old else new` — test thuật toán mới song song); **condition variable** (biến global bật/tắt code path); **statistics** (đếm sự kiện qua biến global → export); **rate/occurrence limiting** — hàm gọi nghìn lần/giây thì `printk` làm ngập máy:
```c
if (error && printk_ratelimit())          // rate limit: mặc định 1 msg/5s, burst 10
        printk(KERN_DEBUG "error=%d\n", error);
static unsigned long limit = 0;           // occurrence limit: chỉ in 5 lần
if (limit < 5) { limit++; printk(...); }
```

### 2.6 Bổ đôi tìm commit lỗi — git bisect (tr. 376–377)

Không biết version nào sinh bug → **binary search** giữa **known-good** và **known-bad**, mỗi bước test kernel giữa. **`git bisect`** tự động hóa **ở mức commit**:
```bash
git bisect start
git bisect bad              # revision hỏng sớm nhất (hoặc bad <rev>)
git bisect good v2.6.28     # revision chạy tốt
# git tự checkout điểm giữa → compile/test → git bisect good | bad → lặp
# cuối cùng git in đúng commit gây bug
git bisect start -- arch/x86   # thu hẹp theo thư mục nếu đoán được nguồn
```
🆕 `git bisect run <script>` tự chạy hoàn toàn nếu có script test pass/fail. Khi hết cách → **LKML** (mailing list).

### Insight đáng nhớ (cụm 2)

- **Tái hiện được = nửa đường (tr. 363):** ưu tiên số một khi gỡ kernel. Bug kernel là **chuỗi sự kiện** (race → use-after-free → NULL deref → oops) — phải lần ngược, không chỉ nhìn triệu chứng.
- **Đọc oops = đọc back trace từ dưới lên** (idle → interrupt → softirq → hàm lỗi) + **offset chỉ đúng dòng**; `CONFIG_KALLSYMS` decode tự động. Đây là kỹ năng BSP số một.
- **`CONFIG_DEBUG_SPINLOCK_SLEEP`/atomicity** bắt "ngủ khi atomic" tự động — bật khi phát triển. **SysRq-s-u-b** cứu máy treo an toàn hơn reset.
- **Không có debugger tiện như userspace** → nghề gỡ kernel dựa vào **printk (rate-limited), oops, ftrace, git bisect** — Linus *cố ý* để vậy (ép hiểu code).

### ⚠️ Đã thay đổi so với sách

- **`ftrace`** (function tracer, tracepoint, `/sys/kernel/debug/tracing`), **`perf`**, **`trace-cmd`/KernelShark**, **eBPF/`bpftrace`** — công cụ quan sát mạnh, sách 2010 chưa có; nay là **chủ lực gỡ hiệu năng/hành vi kernel**.
- **KASAN** (bắt use-after-free/out-of-bounds — như ASan cho kernel), **KFENCE**, **lockdep** (`CONFIG_PROVE_LOCKING` — chứng minh lock ordering, bắt ABBA), **UBSAN**, **kmemleak** — bộ sanitizer hiện đại, mạnh hơn hẳn debug option của sách.
- **`kdb`/`kgdb`** đã vào mainline (không còn patch ngoài). `printk_ratelimit` → macro `pr_*_ratelimited`. `ksymoops` đã chết (kallsyms mặc định). `crash` + kdump/vmcore cho phân tích sau crash.

### Góc interview (cụm 2)

**Câu 1 (🎯):** Bạn nhận một **kernel oops** trong log. Đọc nó thế nào? Cần gì để địa chỉ hiện ra tên hàm?

<details><summary>Đáp án</summary>

- **Oops (tr. 367) = kernel báo lỗi nghiêm trọng:** in dòng lỗi (vd `NULL pointer dereference at ...`) + **dump register** + **back trace** (call chain). ⚠️ Nếu ở **interrupt context / idle(0) / init(1)** → **panic** (dừng máy); process thường → giết process.
- **Đọc back trace từ dưới lên (tr. 369):** chuỗi gọi hàm dẫn tới lỗi — ví dụ tulip: `cpu_idle → timer_interrupt → do_softirq → run_timer_softirq → tulip_timer` (timer handler chạy trong **softirq** — [03](03-sync-timers.md) — deref NULL). **Offset `0x128/0x1c4`** trỏ đúng dòng trong hàm; **register** cho biết cái nào chứa NULL → thường là **race** (giữa timer và phần khác của driver).
- **Decode địa chỉ → tên (tr. 369):** cần **`CONFIG_KALLSYMS`** (kernel tự nhúng bảng symbol → oops tự decode) — nay mặc định; xưa dùng `ksymoops` + `System.map` (dễ mismatch). 🆕 Thêm: `addr2line`/`gdb` trên `vmlinux` để ra đúng file:line; `decode_stacktrace.sh`.
- **Điểm cộng:** cờ **`Tainted`** trong oops (module non-GPL/đã oops trước) ảnh hưởng độ tin; nếu process bị giết mà hệ chạy tiếp thì đó là oops (không panic).

</details>

**Câu 2 (🎯):** Kernel không có debugger tiện như gdb userspace. Bạn gỡ một bug **khó tái hiện, xuất hiện gần đây** trong driver thế nào?

<details><summary>Đáp án</summary>

- **Tái hiện trước (tr. 363):** cố tạo kịch bản reproduce ổn định (tải/timing/cấu hình) — *"halfway home"*. Bug kernel thường là **race** hiện hiếm.
- **In có kiểm soát:** `printk` với loglevel (tr. 365) — nhưng hàm nóng thì **`printk_ratelimit`/`pr_*_ratelimited`** (tr. 375) tránh ngập log; hoặc occurrence-limit.
- **Bật debug infra (tr. 370):** `CONFIG_DEBUG_SPINLOCK_SLEEP`/atomicity (bắt ngủ-khi-atomic), 🆕 **KASAN** (use-after-free/OOB — hợp bug con trỏ chết), **lockdep** (ABBA/lock ordering), `dump_stack()` tại chỗ nghi.
- **Định vị commit gây bug — `git bisect` (tr. 376):** `bisect start` → `bad`/`good <rev>` → compile/test từng điểm giữa → git chỉ đúng commit; `git bisect run <script>` tự động; thu hẹp theo thư mục.
- **Quan sát không xâm lấn (🆕 hiện đại):** **ftrace** (function_graph, tracepoint), **perf**, **bpftrace** — xem đường đi/latency mà không sửa code. **SysRq-t** dump task khi treo. Cuối cùng: **LKML**.
- **Điểm chấm:** nêu được **git bisect + KASAN/lockdep + ftrace** (ngoài printk) là trả lời "senior"; giải thích Linus *cố ý* không cho debugger để ép hiểu code (tr. 372).

</details>

---

## Cụm 3 — Portability (ch. 19, tr. 379–393)

### 3.1 Triết lý & word size (tr. 380–384)

Linux đi **đường giữa (tr. 380):** interface/core là **C độc lập arch**; đường nóng/cấp thấp thì **asm riêng arch** (perf thắng portability khi xung đột). Ví dụ: `context_switch()` C chung gọi `switch_to`/`switch_mm` **riêng mỗi arch** ([01](01-process-sched-syscalls.md)). Code arch ở `arch/<arch>/`.

**Word size (tr. 381):** **word = kích thước GPR = kích thước con trỏ = kích thước `long`**; `int` có thể **nhỏ hơn** word. `BITS_PER_LONG`. **Mô hình (tr. 384):** Linux 64-bit = **LP64** (`long`+con trỏ 64-bit, **`int` vẫn 32-bit**); 32-bit = **ILP32**. ⚠️ Quy tắc vàng: **đừng giả định `sizeof(int)==sizeof(long)`, đừng giả định con trỏ == `int`** (Windows 64-bit là LLP64 — `long` 32-bit, khác Linux).

### 3.2 Kiểu dữ liệu: opaque, explicit, char signedness (tr. 384–386)

- **Opaque type (tr. 384):** `pid_t`, `atomic_t`, `dev_t`, `uid_t` — **không giả định size, không cast về kiểu C chuẩn** (kernel dev đổi size được — `pid_t` từng là `short`).
- **Special type:** cờ interrupt (`flags` của `spin_lock_irqsave` — [03](03-sync-timers.md)) **luôn `unsigned long`**.
- **Explicit-size (tr. 385):** `u8/u16/u32/u64` (và `s*`) khi khớp phần cứng/mạng/file; bản user-visible thêm `__` (`__u32`).
- **⚠️ Char signedness (tr. 386) — bẫy embedded kinh điển:** C không quy định `char` signed hay unsigned. **x86: `char` signed** (−128..127); **ARM: `char` unsigned** (0..255)! → `char i = -1;` trên ARM lưu **255**. Cần dấu rõ ràng thì khai `signed char`/`unsigned char`.

### 3.3 Alignment, padding, endian (tr. 386–390)

- **Alignment (tr. 386):** biến **naturally aligned** = địa chỉ là bội của size. RISC (ARM cũ) **unaligned → trap**; x86 → chỉ chậm. ⚠️ Cast con trỏ `char*` → `unsigned long*` rồi deref = nguồn lỗi alignment.
- **Structure padding (tr. 388):** compiler **chèn pad** để mỗi field aligned. Đảo thứ tự field (lớn→nhỏ) **thu nhỏ struct** (`animal_struct` 12→8 byte). ⚠️ **Đừng `memcmp` struct** (pad chứa rác) hay **gửi struct thô qua mạng/lưu đĩa** (pad khác nhau giữa arch) → serialize tường minh.
- **Byte order / endian (tr. 389):** **big-endian** (byte quan trọng nhất ở địa chỉ nhỏ) vs **little-endian**. **x86 little; nhiều arch khác big.** ⚠️ Đừng giả định endian; dùng `cpu_to_be32`/`le32_to_cpu`... khi đọc phần cứng/mạng (network = big-endian).
```c
int x = 1;
if (*(char *)&x == 1) /* little endian */ else /* big endian */;
```

### 3.4 Time, page size, ordering, SMP/preempt/highmem (tr. 391–393)

- **Time (tr. 391):** **đừng giả định tần số tick** — luôn scale qua **`HZ`** (`HZ`=1 giây, `HZ/100`=10ms). x86 `HZ`=100 (từng 1000), ARM 100, Alpha 1024 ([03](03-sync-timers.md)).
- **Page size (tr. 391):** **đừng giả định 4KB** — dùng **`PAGE_SIZE`**/`PAGE_SHIFT`. ARM có 4/16/64KB, Alpha 8KB ([04](04-memory.md)).
- **Processor ordering (tr. 392):** arch **weak ordering** (ARM) reorder load/store → dùng **barrier** (`rmb`/`wmb` — [03](03-sync-timers.md)) nếu phụ thuộc thứ tự.
- **SMP/preempt/highmem (tr. 393):** **luôn giả định cả ba bật** → dùng lock đúng, `kmap` cho high memory. Kết luận sách (tr. 393): *"code for the greatest common factor... assume only the lowest common denominator is available."*

### Insight đáng nhớ (cụm 3)

- **Char signedness (x86 signed vs ARM unsigned, tr. 386)** là bẫy **cross-compile embedded** thật: code chạy đúng trên x86 dev, sai trên target ARM. Khai `signed`/`unsigned char` rõ ràng.
- **Đừng giả định `HZ`/`PAGE_SIZE`/word size/endian** — dùng macro (`HZ`, `PAGE_SIZE`, `BITS_PER_LONG`, `cpu_to_le32`). Đây là checklist portability BSP.
- **Struct padding** ảnh hưởng size + cấm memcmp/gửi thô qua mạng — nối [cpp-mindset](../cpp-mindset/README.md) (cache/layout) và giao thức nhị phân.

### ⚠️ Đã thay đổi so với sách

- Số arch đổi (thêm arm64/aarch64, riscv, loongarch; bỏ nhiều arch cũ như cris, frv, blackfin). **arm64 luôn little-endian** (khác arm32 chọn được). `u32` etc. nay có `__aligned`/`__packed` attribute rõ ràng.
- Explicit-size chuẩn C99 `uint32_t`... dùng nhiều hơn; endian macro `cpu_to_le32` (bỏ `__` cho code thường).
- Ordering: `smp_load_acquire`/`smp_store_release`, `READ_ONCE`/`WRITE_ONCE` (thay `barrier()` thủ công nhiều chỗ).

### Góc interview (cụm 3)

**Câu 1 (🎯):** Code chạy đúng trên x86 dev nhưng sai trên target ARM. Nêu các **bẫy portability** kinh điển bạn kiểm.

<details><summary>Đáp án</summary>

- **Char signedness (tr. 386):** **x86 `char` signed, ARM `char` unsigned** — `char c = -1` trên ARM ra 255; so sánh `if (c < 0)` sai. → khai `signed char`/`unsigned char` rõ ràng. **Bẫy phổ biến nhất** khi port x86→ARM.
- **Endian (tr. 389):** x86 little, nhiều arch big (arm cấu hình được; arm64 little). Đọc phần cứng/mạng/file nhị phân phải `cpu_to_le32`/`be32_to_cpu`, không cast thô.
- **Alignment (tr. 386):** ARM cũ **trap** khi unaligned (x86 chỉ chậm) → không cast `char*`→`u32*` deref; struct field phải aligned.
- **Word size/con trỏ (tr. 384):** LP64 vs ILP32 — đừng giả định `sizeof(long)==sizeof(int)` hay con trỏ vừa `int`; dùng `uintptr_t`/`u64` đúng chỗ.
- **`HZ`/`PAGE_SIZE` (tr. 391):** đừng hard-code 100/4096 — scale qua `HZ`/`PAGE_SIZE` (ARM page 4/16/64KB).
- **Ordering (tr. 392):** ARM **weak ordering** — thiếu barrier là **bug thật** (x86 strong che giấu) → `smp_rmb`/`smp_wmb`/`READ_ONCE`.
- **Điểm chấm:** nêu được **char signedness + endian + alignment + weak ordering** là bốn bẫy ARM hàng đầu; và struct padding khi trao đổi nhị phân giữa hai arch.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [05-vfs-block-pagecache.md](05-vfs-block-pagecache.md) — kobject↔dentry, `file_operations`/`ioctl` (module driver dùng); [03-sync-timers.md](03-sync-timers.md) — ngủ-khi-atomic (bắt bằng debug option), barrier/weak ordering, `HZ`.
- [01-process-sched-syscalls.md](01-process-sched-syscalls.md) — `switch_to`/`switch_mm` (arch-specific); [04-memory.md](04-memory.md) — `PAGE_SIZE`, high memory/`kmap`.
- [melp/drivers-init-power.md](../melp/drivers-init-power.md) — device model/DT/`platform_driver` góc dùng-hằng-ngày; [09-debugging/kernel-debugging.md](../../09-debugging/kernel-debugging.md), [09-debugging/tools.md](../../09-debugging/tools.md) — dmesg/ftrace/perf/KASAN mức thực hành.

**Hết sách.** ⏮ Quay lại [README](README.md) hoặc [05 — VFS/Block/Page Cache](05-vfs-block-pagecache.md).
