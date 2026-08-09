# TLPI — Cụm 08: Memory Mappings & Shared Libraries (ch. 41, 42, 49, 50)

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 41–42 (tr. 833–876) và 49–50 (tr. 1017–1056).
> **Định vị:** phần tra cứu. Nhưng **ch. 41–42 sát công việc của bạn** (viết C++ shared library) — góc ôn thi đã có ở [07-shared-libraries](../../07-shared-libraries/); cụm này bổ sung **góc thực chiến ở tầng công cụ** (`soname`, `ldd`, `dlopen`) và **`mmap` — thứ 07 không phủ**.

---

## 1. `mmap()` — bảng 4 ô quyết định tất cả (ch. 49, tr. 1017–1044) 🎯

### 1.1. Hai trục, bốn tổ hợp

Kerrisk tổ chức cả chương quanh **hai câu hỏi độc lập** (§49.2, tr. 1018):

- **File-backed hay anonymous?** — ánh xạ nội dung một file, hay chỉ xin bộ nhớ trắng?
- **`MAP_PRIVATE` hay `MAP_SHARED`?** — thay đổi có lộ ra ngoài không?

| Thay đổi thấy được ở đâu | **File** | **Anonymous** |
|---|---|---|
| **Private** (`MAP_PRIVATE`) | Khởi tạo bộ nhớ từ nội dung file | **Cấp phát bộ nhớ** |
| **Shared** (`MAP_SHARED`) | **Memory-mapped I/O**; chia sẻ bộ nhớ giữa process (IPC) | **Chia sẻ bộ nhớ giữa process** (IPC) |

*(Table 49-1, tr. 1018)*

**Bốn ô này là bốn thứ hoàn toàn khác nhau, và mỗi ô có một "người dùng" nổi tiếng:**

```
   ① File + PRIVATE   → nạp SEGMENT TEXT và DATA của chương trình/thư viện
                         (đây chính là cách kernel nạp file thực thi và .so)
   ② File + SHARED    → memory-mapped I/O: đọc/ghi file bằng phép gán bộ nhớ,
                         thay đổi ĐI XUỐNG FILE; nhiều process cùng thấy
   ③ Anon + PRIVATE   → malloc() cho khối lớn (glibc dùng từ ~128 KB)
   ④ Anon + SHARED    → shared memory giữa process HỌ HÀNG (không cần file)
```

**Định nghĩa chính xác của sách** (tr. 1018):
> - `MAP_PRIVATE`: *"Modifications... are **not visible** to other processes and, for a file mapping, are **not carried through to the underlying file**."* Kernel làm được điều đó bằng **copy-on-write** — nên nó còn được gọi là *"private, copy-on-write mapping"*.
> - `MAP_SHARED`: *"Modifications... **are visible** to other processes that share the same mapping and, for a file mapping, **are carried through to the underlying file**."*

### 1.2. Ánh xạ file — đọc file bằng phép gán

```c
int fd = open(path, O_RDWR);
struct stat sb;
fstat(fd, &sb);

char *addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, 0);
if (addr == MAP_FAILED) errExit("mmap");    // ⚠️ MAP_FAILED, KHÔNG phải NULL
close(fd);                                   // ✅ hợp lệ — ánh xạ giữ tham chiếu riêng

addr[0] = 'X';                               // ← ghi vào FILE bằng phép gán bộ nhớ
msync(addr, sb.st_size, MS_SYNC);            // ép xuống đĩa (như fsync)
munmap(addr, sb.st_size);
```

**`mmap` vs `read`/`write` — đánh đổi:**

| | `read()`/`write()` | `mmap()` |
|---|---|---|
| Số lần chép | **2** (đĩa→page cache→buffer user) | **1** (đĩa→page cache, user *dùng thẳng*) |
| Syscall mỗi lần truy cập | Có | **Không** (sau lần `mmap` đầu) |
| Truy cập ngẫu nhiên | Cần `lseek` mỗi lần | **Chỉ là chỉ số mảng** |
| File nhỏ / đọc tuần tự một lần | **Tốt hơn** — `mmap` tốn chi phí thiết lập + page fault | Không đáng |
| Đổi kích thước file | Dễ | **Khó** — phải `munmap`/`mmap` lại |
| Lỗi I/O | Trả `-1` + `errno` | **`SIGBUS`** — khó xử lý hơn nhiều |

> ⚠️ **Bẫy `SIGBUS`:** truy cập vùng ánh xạ vượt quá kích thước file thật (vd file bị `ftruncate` nhỏ đi sau khi `mmap`) → **`SIGBUS`**, không phải `SIGSEGV`, và không có `errno` để kiểm tra. Đây là lý do phải `ftruncate()` **trước** khi `mmap` vùng shared memory ([cụm 06 §3.3](06-ipc.md)).

### 1.3. Anonymous mapping — cách `malloc` lấy khối lớn

```c
/* Xin bộ nhớ trắng, không gắn file */
void *p = mmap(NULL, len, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
```

🆕 Đây là ô ③ — và là câu trả lời cho *"`free()` rồi mà RSS không giảm"* ở [cụm 02 §1.3](02-processes-and-exec.md): khối lấy bằng `brk`/`sbrk` chỉ trả về hệ điều hành nếu nằm sát đỉnh heap, còn khối lấy bằng `mmap` thì `free()` gọi `munmap()` **trả lại ngay**. glibc chuyển sang `mmap` từ ngưỡng `M_MMAP_THRESHOLD` (mặc định 128 KB).

**`MAP_ANONYMOUS | MAP_SHARED`** (ô ④) là cách chia sẻ bộ nhớ giữa **process họ hàng** mà **không cần file nào** — `mmap` trước, `fork` sau, cả hai thấy chung. Đơn giản hơn `shm_open` khi chỉ cần cha–con.

### 1.4. Các thao tác bộ nhớ ảo (ch. 50, tr. 1045–1056)

| Hàm | Làm gì | Dùng khi |
|---|---|---|
| `mprotect()` | Đổi quyền (`PROT_READ`/`WRITE`/`EXEC`) của vùng đã ánh xạ | Bảo vệ vùng chỉ đọc; **guard page** tự cài; JIT (ghi xong đổi sang exec) |
| `mlock()` / `mlockall()` | **Khoá trang trong RAM**, cấm swap | **Realtime** (tránh page fault bất ngờ), giữ khoá mật mã không rơi ra swap |
| `madvise()` | Gợi ý kernel về mẫu truy cập (`MADV_SEQUENTIAL`, `MADV_WILLNEED`, `MADV_DONTNEED`) | Tối ưu đọc file lớn |
| `mremap()` | Đổi kích thước vùng ánh xạ | `realloc` cho vùng `mmap` |
| `msync()` | Ép nội dung ánh xạ xuống đĩa | Như `fsync` nhưng cho `mmap` |

> 🆕 **`mlockall(MCL_CURRENT | MCL_FUTURE)` là một trong những dòng đầu tiên của mọi ứng dụng realtime trên Linux** — một page fault giữa vòng điều khiển có thể tốn hàng mili-giây, đủ trượt deadline. Nối với [08/rtos-vs-linux.md](../../08-embedded-systems/rtos-vs-linux.md).

---

## 2. Shared library — góc thực chiến (ch. 41–42, tr. 833–876)

### 2.1. Ba cái tên của một thư viện — nguồn của mọi nhầm lẫn

Kerrisk giải thích rất rõ (§41.8, tr. 843–845), và đây là phần đáng nhớ nhất:

```
   ① REAL NAME     libfoo.so.1.0.3      ← file THẬT trên đĩa
                        │
   ② SONAME        libfoo.so.1          ← nhúng VÀO thư viện (tag DT_SONAME);
                        │                  đây là tên được ghi vào chương trình
                        │                  lúc LINK, và loader tìm lúc CHẠY
   ③ LINKER NAME   libfoo.so            ← symlink, CHỈ dùng lúc biên dịch (-lfoo)

   Trên đĩa:
      libfoo.so        → libfoo.so.1        (symlink, do dev tạo)
      libfoo.so.1      → libfoo.so.1.0.3    (symlink, do ldconfig tạo)
      libfoo.so.1.0.3                       (file thật)
```

```sh
gcc -shared -Wl,-soname,libfoo.so.1 -o libfoo.so.1.0.3 mod1.o mod2.o
```

**Vì sao cần ba tên — mấu chốt là soname.** Kerrisk viết: *"the purpose of the soname is to provide a level of indirection"* (tr. 843). Cụ thể:

- Lúc **link tĩnh**, linker nhúng **soname** (không phải real name) vào file thực thi.
- Lúc **chạy**, loader tìm file tên đúng bằng **soname**.

⇒ Bạn có thể thay `libfoo.so.1.0.3` bằng `libfoo.so.1.0.4` (sửa lỗi, **cùng ABI**) chỉ bằng cách trỏ lại symlink `libfoo.so.1` — **mọi chương trình đã build vẫn chạy, không cần build lại**. Nhưng nếu **phá vỡ ABI**, bạn đổi soname thành `libfoo.so.2`, và hai phiên bản **cùng tồn tại** trên máy: chương trình cũ dùng `.so.1`, chương trình mới dùng `.so.2`.

> 🆕 **Đây chính là câu trả lời cho "vì sao Linux không có DLL hell".** Quy ước: **thay đổi tương thích → tăng minor/release; phá ABI → tăng số trong soname**. Nối với [07/abi-versioning.md](../../07-shared-libraries/abi-versioning.md).

### 2.2. Công cụ điều tra — phần đáng giá cho debug

```sh
ldd ./myapp                 # thư viện nào được dùng, giải ra file nào
                            # ⚠️ KHÔNG chạy ldd trên binary không tin cậy (nó có thể thực thi code)

readelf -d libfoo.so.1.0.3 | grep SONAME     # soname thật của thư viện
readelf -d ./myapp | grep NEEDED             # app cần những soname nào

nm -D --defined-only libfoo.so | head        # symbol thư viện XUẤT ra
nm -D --undefined-only ./myapp               # symbol app CẦN

ldconfig -p | grep libfoo   # cache của loader (/etc/ld.so.cache)
ldconfig -v                 # quét lại, tạo/cập nhật symlink soname
```

**Loader tìm thư viện theo thứ tự** (§41.11, tr. 850):
1. `DT_RPATH` / `DT_RUNPATH` nhúng trong binary (`-Wl,-rpath`)
2. Biến môi trường **`LD_LIBRARY_PATH`** ⚠️
3. Cache `/etc/ld.so.cache` (do `ldconfig` sinh)
4. `/lib`, `/usr/lib`

> ⚠️ **`LD_LIBRARY_PATH` bị BỎ QUA với chương trình set-UID/set-GID** — cố ý, vì nếu không thì bất kỳ ai cũng ép chương trình đặc quyền nạp thư viện của họ. Cùng lý do đó, **đừng dùng `LD_LIBRARY_PATH` cho triển khai thật** — dùng `-rpath` hoặc `ldconfig`.

### 2.3. Nạp động — `dlopen()`

```c
void *handle = dlopen("libfoo.so.1", RTLD_LAZY);   // hoặc RTLD_NOW
if (handle == NULL) fatal("%s", dlerror());

/* ⚠️ Cách ép kiểu đúng — dlsym trả void*, chuẩn C không cho ép sang con trỏ hàm */
void (*func)(int);
*(void **) (&func) = dlsym(handle, "myFunction");
char *err = dlerror();                              // kiểm tra bằng dlerror(),
if (err != NULL) fatal("%s", err);                  // KHÔNG bằng func == NULL
                                                    // (symbol hợp lệ CÓ THỂ có giá trị NULL)
func(42);
dlclose(handle);
```

| Cờ | Ý nghĩa |
|---|---|
| `RTLD_LAZY` | Giải quyết symbol **khi được gọi lần đầu** — nạp nhanh hơn |
| `RTLD_NOW` | Giải quyết **hết ngay** — chậm hơn nhưng lỗi lộ ra **ngay lúc `dlopen`**, không phải giữa lúc chạy |
| `RTLD_GLOBAL` | Symbol của thư viện này dùng được cho các thư viện nạp sau |

🆕 Dùng cho **plugin architecture** — và cũng là cách một BSP nạp module theo cấu hình phần cứng lúc chạy. Với C++ nhớ **`extern "C"`** cho hàm entry, nếu không tên bị **mangle** và `dlsym` không tìm thấy.

### 2.4. Kiểm soát symbol xuất ra — quan trọng cho người viết thư viện

Mặc định, shared library **xuất ra MỌI symbol toàn cục** — gây ba vấn đề: xung đột tên, nạp chậm (bảng symbol lớn), và **khoá bạn vào một ABI bạn không định cam kết**.

```c
/* Cách 1 — thuộc tính visibility (gcc) */
#define DLL_PUBLIC __attribute__((visibility("default")))
#define DLL_LOCAL  __attribute__((visibility("hidden")))

DLL_PUBLIC int api_function(void);      // xuất
static  int helper(void);               // static: không bao giờ xuất
```

```sh
# Cách 2 — mặc định ẩn hết, chỉ mở cái đánh dấu (nên dùng)
gcc -fvisibility=hidden -shared -o libfoo.so.1.0.0 ...

# Cách 3 — version script, kiểm soát chính xác nhất
gcc -shared -Wl,--version-script=libfoo.map ...
```

**Version script** (§42.2, tr. 861) còn cho phép **nhiều phiên bản của cùng một hàm** tồn tại trong một thư viện — cách glibc giữ tương thích ngược hàng chục năm mà không cần tăng soname.

---

## 3. Ít quan trọng — 1 dòng + tham chiếu

- **§41.1–41.3 — Static library, `ar`** (tr. 834): `ar rcs libfoo.a *.o`; lý do chuyển sang shared (tiết kiệm đĩa/RAM, cập nhật không cần build lại).
- **§41.4–41.6 — Tạo và dùng shared library, PIC** (tr. 837): **`-fPIC`** bắt buộc cho shared library — mã độc lập vị trí, nạp được ở địa chỉ bất kỳ.
- **§41.12 — `LD_PRELOAD`** (tr. 852): nạp thư viện **trước** mọi thư viện khác → **ghi đè hàm thư viện**. Rất mạnh cho debug/profiling (chèn `malloc` của mình để bắt leak) — và cũng là kỹ thuật của rootkit. Cũng bị bỏ qua với set-UID.
- **§41.13 — `ldd` chi tiết** (tr. 853).
- **§42.1 — Symbol versioning** (tr. 859).
- **§42.4 — Khởi tạo/kết thúc thư viện** (tr. 866): `__attribute__((constructor))` / `(destructor)` — chạy khi thư viện được nạp/gỡ.
- **§42.5 — Preloading, `ldconfig` chi tiết** (tr. 868).
- **§49.6–49.7 — `MAP_FIXED`, `MAP_NORESERVE`, huge pages** (tr. 1033).
- **§50.1–50.3 — `mprotect`, `mlock`, `mincore`** (tr. 1045).

---

## 4. Góc interview

<details><summary>1) <code>MAP_PRIVATE</code> và <code>MAP_SHARED</code> khác nhau thế nào? Bốn tổ hợp dùng làm gì?</summary>

Hai trục **độc lập** nhau: *file-backed vs anonymous* và *private vs shared*.

- **`MAP_PRIVATE`** — thay đổi **không** thấy được từ process khác, và với file mapping thì **không đi xuống file**. Kernel làm bằng **copy-on-write**: trang ban đầu dùng chung, ai ghi thì được chép riêng. Vì vậy nó còn gọi là *private, copy-on-write mapping*.
- **`MAP_SHARED`** — thay đổi **thấy được** từ mọi process ánh xạ chung, và với file mapping thì **đi xuống file thật**.

| | **File** | **Anonymous** |
|---|---|---|
| **Private** | Khởi tạo bộ nhớ từ nội dung file — *chính là cách kernel nạp `.text`/`.data` của chương trình và `.so`* | **Cấp phát bộ nhớ** — `malloc` dùng cho khối lớn (glibc: từ ~128 KB) |
| **Shared** | **Memory-mapped I/O** — đọc/ghi file bằng phép gán; nhiều process chung | **Shared memory giữa process họ hàng** — `mmap` trước, `fork` sau, không cần file |

**`mmap` vs `read`/`write`:** `mmap` bỏ được **một lần chép** (đĩa→page cache→*dùng thẳng*, thay vì thêm bước chép sang buffer user) và bỏ **syscall mỗi lần truy cập**; truy cập ngẫu nhiên trở thành chỉ số mảng. Đổi lại: chi phí thiết lập + page fault, khó khi file đổi kích thước, và **lỗi I/O báo bằng `SIGBUS`** thay vì mã lỗi.

⚠️ **`mmap` trả về `MAP_FAILED`, không phải `NULL`** khi lỗi. Và truy cập vùng vượt quá kích thước file thật → **`SIGBUS`** — đó là lý do phải `ftruncate()` **trước** khi `mmap` vùng shared memory.
</details>

<details><summary>2) Vì sao một thư viện có ba cái tên? <code>soname</code> giải quyết vấn đề gì?</summary>

```
   Real name    libfoo.so.1.0.3    ← file thật trên đĩa
   soname       libfoo.so.1        ← nhúng trong thư viện; ghi vào app lúc LINK,
                                      loader tìm bằng tên này lúc CHẠY
   Linker name  libfoo.so          ← symlink, chỉ dùng lúc biên dịch (-lfoo)
```

**soname tạo ra một lớp gián tiếp** — chính từ TLPI: *"the purpose of the soname is to provide a level of indirection."*

Lúc link, linker nhúng **soname** (không phải real name) vào file thực thi; lúc chạy, loader tìm theo **soname**. Hệ quả:

- **Cập nhật tương thích** (sửa lỗi, thêm hàm, **giữ ABI**): thay `libfoo.so.1.0.3` → `libfoo.so.1.0.4`, trỏ lại symlink `libfoo.so.1`. **Mọi chương trình đã build vẫn chạy, không build lại.**
- **Phá vỡ ABI**: đổi soname thành `libfoo.so.2`. Hai phiên bản **cùng tồn tại** — app cũ dùng `.so.1`, app mới dùng `.so.2`, không xung đột.

Đây là lý do Linux không có "DLL hell". Quy ước: **tương thích → tăng minor; phá ABI → tăng số trong soname**.

```sh
gcc -shared -Wl,-soname,libfoo.so.1 -o libfoo.so.1.0.3 *.o
readelf -d libfoo.so.1.0.3 | grep SONAME    # xác nhận
readelf -d ./myapp | grep NEEDED            # app cần soname nào
```

**Thứ tự loader tìm thư viện:** `DT_RPATH`/`DT_RUNPATH` → `LD_LIBRARY_PATH` → `/etc/ld.so.cache` → `/lib`, `/usr/lib`.

⚠️ **`LD_LIBRARY_PATH` bị bỏ qua với chương trình set-UID/set-GID** — cố ý, nếu không thì ai cũng ép chương trình đặc quyền nạp thư viện của họ. Vì vậy đừng dùng nó cho triển khai thật; dùng `-rpath` hoặc `ldconfig`.
</details>

<details><summary>3) Khi nào dùng <code>dlopen()</code>? Cần lưu ý gì với C++?</summary>

**Dùng khi cần quyết định LÚC CHẠY** nạp gì: kiến trúc **plugin**, nạp backend theo cấu hình, hoặc BSP nạp module theo phần cứng phát hiện được lúc boot. Nếu biết trước lúc build thì cứ link bình thường — `dlopen` chỉ thêm phức tạp.

```c
void *h = dlopen("libplugin.so.1", RTLD_NOW);
if (!h) fatal("%s", dlerror());

void (*init)(void);
*(void **)(&init) = dlsym(h, "plugin_init");   // ép kiểu qua void** — chuẩn C
                                               // không cho ép thẳng void*→con trỏ hàm
char *err = dlerror();
if (err) fatal("%s", err);                     // ⚠️ kiểm tra bằng dlerror(),
                                               //    KHÔNG bằng init == NULL
init();
dlclose(h);
```

**`RTLD_LAZY` vs `RTLD_NOW`:** lazy giải quyết symbol khi gọi lần đầu (nạp nhanh); **`RTLD_NOW` giải quyết hết ngay** nên **lỗi thiếu symbol lộ ra ngay lúc `dlopen`** thay vì nổ giữa lúc chạy. Với hệ cần độ tin cậy → chọn `RTLD_NOW`.

**Ba lưu ý riêng cho C++:**
1. **`extern "C"` cho hàm entry** — nếu không, tên bị **mangle** (`_Z11plugin_initv`) và `dlsym("plugin_init")` không tìm thấy.
2. **Trả về đối tượng qua factory function**, đừng export class trực tiếp — layout class là chi tiết ABI rất dễ vỡ (đổi thứ tự thành viên, thêm virtual function là hỏng).
3. **Cẩn thận `dlclose()`** khi còn đối tượng do thư viện tạo hoặc còn exception/RTTI tham chiếu vào nó — gỡ thư viện ra khỏi bộ nhớ khi code của nó còn được dùng là crash. Nhiều hệ đơn giản là **không bao giờ `dlclose`**.

🆕 Liên quan: **`LD_PRELOAD`** nạp thư viện **trước** mọi thư viện khác, cho phép **ghi đè hàm thư viện** — rất mạnh để chèn `malloc` của mình bắt memory leak, hoặc giả lập lỗi khi test. Cũng bị bỏ qua với set-UID vì lý do bảo mật.
</details>

<details><summary>4) Viết shared library thì nên kiểm soát symbol xuất ra thế nào? Vì sao?</summary>

**Mặc định shared library xuất ra MỌI symbol toàn cục** — điều này gây ba vấn đề thật:

1. **Xung đột tên** — hàm nội bộ tên `init()` của bạn có thể ghi đè hoặc bị ghi đè bởi thư viện khác.
2. **Nạp chậm** — bảng symbol động lớn làm loader mất nhiều thời gian giải quyết symbol.
3. **Khoá bạn vào một ABI bạn không định cam kết** — người dùng gọi được hàm nội bộ, sau này bạn đổi nó là **phá vỡ họ**, dù bạn chưa bao giờ coi nó là API công khai.

**Ba cách kiểm soát, tăng dần độ chặt:**

```c
/* ① Đánh dấu từng symbol */
#define API_PUBLIC __attribute__((visibility("default")))
API_PUBLIC int foo_init(void);
static int helper(void);          /* static → không bao giờ xuất */
```

```sh
# ② Đảo mặc định: ẩn hết, chỉ mở cái đánh dấu — NÊN DÙNG
gcc -fvisibility=hidden -fPIC -shared -o libfoo.so.1.0.0 *.o

# ③ Version script — kiểm soát chính xác nhất
gcc -shared -Wl,--version-script=libfoo.map -o libfoo.so.1.0.0 *.o
```
```
/* libfoo.map */
LIBFOO_1.0 { global: foo_init; foo_process; local: *; };
```

Cách ② là mặc định tốt: **an toàn theo thiết kế** — quên đánh dấu thì symbol bị ẩn (lỗi lộ ra lúc link, dễ sửa), thay vì vô tình xuất ra và bị khoá vào ABI vĩnh viễn.

**Version script còn làm được điều ③ không cách nào khác làm được:** giữ **nhiều phiên bản của cùng một hàm** trong một thư viện, để chương trình cũ gọi bản cũ và chương trình mới gọi bản mới. Đây là cách glibc giữ tương thích ngược hàng chục năm mà **không phải tăng soname**.

Kiểm tra kết quả: `nm -D --defined-only libfoo.so.1.0.0` — chỉ nên thấy đúng những gì bạn định xuất.
</details>

---

## 5. Đọc thêm (tùy chọn)

- **Góc ôn thi của shared library** (static vs shared, PLT/GOT, ABI, thiết kế API): [07-shared-libraries](../../07-shared-libraries/) — gọn hơn, hợp ôn phỏng vấn.
- Bank: `SD-*` (ABI/thư viện) trong [system-design.md](../../14-prep/mock-interview/bank/system-design.md); `CPP-009` (template ở header).
- `mmap` cho shared memory IPC: [cụm 06 §3.3](06-ipc.md).
- Vì sao `free()` không trả RAM: [cụm 02 §1.3](02-processes-and-exec.md).
- `mlockall` cho realtime: [08/rtos-vs-linux.md](../../08-embedded-systems/rtos-vs-linux.md).

---
⬅️ [Cụm 07 — Sockets](07-sockets.md) · [README cụm TLPI](README.md)
