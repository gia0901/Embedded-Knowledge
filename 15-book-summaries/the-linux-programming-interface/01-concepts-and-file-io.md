# TLPI — Cụm 01: Khái niệm nền & File I/O (ch. 1–5, 13) 🎯

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Xem [README cụm](README.md).
> **Vì sao cụm này đứng đầu:** mọi thứ còn lại của cuốn sách dựng trên hai ý ở đây — *file descriptor* và *syscall*. Và đây là cụm dày câu phỏng vấn nhất trên mỗi trang đọc.

---

## 1. Kernel, syscall, và ranh giới user/kernel (ch. 2–3, tr. 21–68)

### 1.1. Syscall thực sự là gì

**Syscall = điểm vào có kiểm soát để chương trình userspace nhờ kernel làm việc mà nó không được phép tự làm** (đụng phần cứng, cấp bộ nhớ, tạo process). Không phải "gọi hàm bình thường": nó **chuyển CPU từ user mode sang kernel mode**.

Kerrisk mô tả các bước một syscall thực sự đi qua (§3.1, tr. 44) — đáng nhớ vì nó giải thích *vì sao syscall đắt*:

```
   ① Chương trình gọi wrapper của thư viện C (vd read())
        │
   ② wrapper đặt SỐ HIỆU syscall vào thanh ghi (%eax trên x86-32)
        │   và đặt các đối số vào các thanh ghi khác
        │
   ③ thực thi lệnh trap (int 0x80 / sysenter / syscall)
        │   → CPU chuyển từ USER MODE sang KERNEL MODE
        │   → nhảy tới trình xử lý trap của kernel (system_call())
        │
   ④ kernel lưu thanh ghi vào kernel stack, KIỂM TRA số hiệu hợp lệ,
        │   KIỂM TRA từng đối số (con trỏ có nằm trong không gian user không?)
        │
   ⑤ gọi service routine tương ứng, làm việc, đặt kết quả trả về
        │
   ⑥ khôi phục thanh ghi, chuyển NGƯỢC về USER MODE, trả về wrapper
        │
   ⑦ wrapper kiểm tra kết quả < 0 → đặt errno, trả -1
```

*(sơ đồ vẽ lại theo Figure 3-1, tr. 45)*

**Insight neo trang:** Kerrisk đo và ghi rõ *"even a simple system call... `getppid()`... typically takes a small but nonzero amount of time"* — trên hệ thử của ông, 10 triệu lần `getppid()` mất ~2,2 giây, tức **~0,22 µs/lần** (§3.1, tr. 46). Con số đó là lý do tồn tại của mọi kỹ thuật *giảm số lần vượt biên*: buffer stdio, `readv`/`writev`, `sendfile`, `io_uring`, `vDSO`. 🆕 Nó cũng là lý do bạn không đặt `printf` trong hot path.

> ⚠️ **Bẫy hay bị hỏi:** *"`read()` là syscall hay hàm thư viện?"* — Cái bạn gọi là **wrapper trong glibc**; syscall thật nằm sau lệnh trap. Phân biệt này quan trọng vì nó giải thích được `strace` thấy gì (syscall thật) so với `ltrace` thấy gì (lời gọi thư viện).

### 1.2. `errno` — và quy tắc kiểm tra đúng

`errno` là biến toàn cục (thực chất **per-thread** trong môi trường đa luồng, §3.4) chỉ có nghĩa **khi lời gọi vừa báo lỗi**. Hai quy tắc Kerrisk nhấn mạnh (§3.4, tr. 48–49):

```c
fd = open(path, O_RDONLY);
if (fd == -1) {                 // ① LUÔN kiểm tra giá trị trả về TRƯỚC
    /* errno CHỈ có nghĩa ở đây */
}

// ② Ca đặc biệt: hàm mà -1 cũng là giá trị hợp lệ (getpriority, ...)
errno = 0;                      // phải tự xoá trước
int prio = getpriority(PRIO_PROCESS, 0);
if (prio == -1 && errno != 0) { /* lỗi thật */ }
```

**Bẫy:** gọi bất cứ hàm nào khác (kể cả `printf`) giữa lời gọi lỗi và lúc đọc `errno` là có thể **ghi đè `errno`**. Lưu lại nếu cần dùng sau.

---

## 2. File descriptor — trừu tượng trung tâm (ch. 4, tr. 69–88) 🎯

### 2.1. "Everything is a file" nghĩa là gì

Bốn syscall duy nhất — `open()`, `read()`, `write()`, `close()` — hoạt động trên **mọi loại file**: file thường, pipe, FIFO, socket, terminal, thiết bị. Kerrisk gọi đây là *universality of I/O* (§4.1, tr. 70) và chỉ ra nó có được nhờ **kernel dịch mọi thao tác qua một tập hàm riêng của từng loại filesystem/thiết bị** — tức VFS.

Ba fd mở sẵn mà shell chuẩn bị cho mọi process:

| fd | Hằng số POSIX | stdio | Mục đích |
|---|---|---|---|
| 0 | `STDIN_FILENO` | `stdin` | đầu vào chuẩn |
| 1 | `STDOUT_FILENO` | `stdout` | đầu ra chuẩn |
| 2 | `STDERR_FILENO` | `stderr` | lỗi chuẩn (**không buffer**) |

🆕 Đây chính là cơ chế đứng sau `ls > out.txt`: shell `fork()`, rồi ở process con nó `open()` file và `dup2()` lên fd 1 **trước khi `exec()`**. Chương trình `ls` không hề biết — nó vẫn ghi vào fd 1 như thường. Toàn bộ khả năng redirect/pipe của Unix nằm ở việc **fd là số nhỏ, và số đó gán lại được**.

### 2.2. Ngữ nghĩa `read()`/`write()` — chỗ sai nhiều nhất

```c
ssize_t numRead  = read(fd, buffer, count);
ssize_t numWrite = write(fd, buffer, count);
```

Ba điều Kerrisk cảnh báo rõ (§4.3–4.4, tr. 72–74) và là ba lỗi kinh điển:

1. **`read()` có thể trả về ÍT hơn `count`** dù chưa hết file — với pipe, socket, terminal thì đó là chuyện *bình thường*, không phải lỗi. Trả về `0` mới là **end-of-file**.
2. **`read()` KHÔNG thêm `'\0'`.** Muốn in như chuỗi thì phải tự đặt, và phải cấp `count + 1` byte:
   ```c
   char buf[MAX_READ + 1];
   ssize_t n = read(STDIN_FILENO, buf, MAX_READ);
   if (n == -1) errExit("read");
   buf[n] = '\0';                 // ← tự kết chuỗi; thiếu dòng này là đọc tràn
   printf("Đọc %ld byte: %s\n", (long) n, buf);
   ```
3. **`write()` có thể ghi ÍT hơn `count`** (partial write) — đĩa đầy, hoặc vượt giới hạn `RLIMIT_FSIZE`, hoặc ghi vào pipe/socket. **Luôn phải lặp** cho tới khi ghi hết.

> 🆕 Đây là lý do mọi codebase nghiêm túc đều có hàm `writen()`/`readn()` bọc vòng lặp. Trong phỏng vấn, viết `write(fd, buf, n)` rồi coi như xong là mất điểm ngay.

### 2.3. Ba bảng của kernel — hình quan trọng nhất chương 5

Đây là hình cần vẽ lại được trên giấy khi bị hỏi *"`dup2` khác `open` cùng file ở chỗ nào?"* hoặc *"`fork` xong hai process chia sẻ gì?"*:

```
   Bảng fd của process A          Bảng file description             Bảng i-node
   (mỗi process MỘT bảng)         (TOÀN HỆ THỐNG, một bảng)         (toàn hệ thống)
   ┌────┬──────────┐              ┌─────┬───────────────┐           ┌──────┬─────────┐
   │ fd │ flags    │              │ #   │ offset        │           │ i-no │ type    │
   ├────┼──────────┤              │     │ status flags  │           │      │ locks   │
   │ 0  │ ─────────┼─────────────►│ 23  │ 0             │──────────►│ 1976 │ perms   │
   │ 1  │ ─────────┼──────┐       │     │ i-node ptr    │     ┌────►│      │ size... │
   │ 2  │          │      │       ├─────┼───────────────┤     │     └──────┴─────────┘
   │ 3  │ ─────────┼──┐   └──────►│ 73  │ 1024          │─────┤
   └────┴──────────┘  │           │     │               │     │
                      │           ├─────┼───────────────┤     │
   Bảng fd process B  └──────────►│ 86  │ 0             │─────┘
   ┌────┬──────────┐              └─────┴───────────────┘
   │ 3  │ ─────────┼──────────────────►(tới #86)
   └────┴──────────┘
```

*(vẽ lại theo Figure 5-2, tr. 95)*

**Đọc ra ba kết luận — đây mới là phần ăn điểm:**

| Tình huống | Chia sẻ gì | Hệ quả |
|---|---|---|
| **`dup()`/`dup2()`** trong cùng process | Hai fd → **CÙNG một file description** | **Chung offset** và **chung status flags**. Ghi qua fd này thì offset của fd kia cũng nhảy |
| **`fork()`** | fd của con → **cùng file description** với cha | Cha con **chung offset**. Đây là lý do `ls >> log` và `date >> log` từ hai process không đè nhau khi dùng `O_APPEND` |
| **`open()` cùng file hai lần** | Hai file description **khác nhau**, cùng i-node | **Offset độc lập**. Ghi qua fd này không dịch offset fd kia |

Còn một tầng nữa dễ nhầm: **`close-on-exec` (`FD_CLOEXEC`) là thuộc tính của FD, không phải của file description** (§5.4, tr. 97). Nên `dup()` **không** sao chép cờ đó — hai fd trỏ chung file description vẫn có `FD_CLOEXEC` riêng.

### 2.4. Atomicity — vì sao `O_APPEND` và `O_EXCL` tồn tại 🎯

Kerrisk dành cả §5.1 (tr. 90–92) cho ý này, và nó là **câu hỏi phỏng vấn rất tốt** vì trả lời được là chứng tỏ hiểu race condition ở tầng syscall.

**Ca 1 — nối file từ nhiều process.** Cách "tự làm" bị race:

```c
/* ❌ SAI: hai lệnh, có KHE HỞ ở giữa */
lseek(fd, 0, SEEK_END);        // ① tìm tới cuối
                               //    ← process khác ghi chen vào ĐÂY
write(fd, buf, len);           // ② ghi vào offset đã CŨ → ĐÈ MẤT dữ liệu bên kia

/* ✅ ĐÚNG: một syscall, kernel đảm bảo nguyên tử */
fd = open(path, O_WRONLY | O_APPEND);
write(fd, buf, len);           // seek-tới-cuối + ghi là MỘT thao tác không chia cắt
```

**Ca 2 — tạo file "chỉ khi chưa tồn tại".** Cách tự làm cũng bị race:

```c
/* ❌ SAI */
fd = open(path, O_WRONLY);
if (fd == -1 && errno == ENOENT) {   // ← process khác TẠO file đúng lúc này
    fd = open(path, O_WRONLY | O_CREAT, 0600);   // ta tưởng mình tạo, thật ra đè
}

/* ✅ ĐÚNG */
fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
if (fd == -1 && errno == EEXIST) { /* file đã có — biết CHẮC CHẮN */ }
```

> **Câu chốt để nhớ:** *"Bất cứ khi nào bạn phải làm hai syscall mà giữa chúng không được có ai chen vào, hãy tìm cờ hoặc syscall làm gộp cả hai."* Đây là mẫu chung, xuất hiện lại ở `pread`/`pwrite` (đọc/ghi tại offset mà không đụng offset chung — §5.4) và ở `openat()` (§18.11).

### 2.5. `fcntl()` — sửa cờ mà không đóng file

```c
int flags = fcntl(fd, F_GETFL);            // ① đọc cờ hiện tại
if (flags == -1) errExit("fcntl");
flags |= O_APPEND;                          // ② thêm cờ muốn bật
if (fcntl(fd, F_SETFL, flags) == -1)        // ③ ghi lại — KHÔNG bao giờ F_SETFL trực tiếp
    errExit("fcntl");                       //    mà không qua bước ① (sẽ xoá sạch cờ khác)
```

Không phải cờ nào cũng sửa được sau `open()`: `O_RDONLY`/`O_WRONLY`/`O_RDWR` và `O_CREAT`/`O_EXCL`/`O_TRUNC` là **cố định**; sửa được là `O_APPEND`, `O_NONBLOCK`, `O_ASYNC`, `O_SYNC` (§5.3, tr. 92–93).

---

## 3. Hai tầng buffer — `fflush` ≠ `fsync` (ch. 13, tr. 233–250) 🎯

Đây là cụm tôi cho là **giá trị nhất chương 13**, vì nó gỡ đúng chỗ hầu hết người dùng Linux mơ hồ.

### 3.1. Dữ liệu đi qua mấy tầng trước khi nằm trên đĩa

```
   Chương trình
   ┌──────────────────────────────┐
   │  printf() / fwrite()         │
   └──────────────┬───────────────┘
                  │
       ┌──────────▼──────────┐
       │  ① BUFFER stdio     │  ← trong KHÔNG GIAN NGƯỜI DÙNG (thư viện C)
       │     (userspace)     │     xả bằng  fflush()  hoặc khi buffer đầy
       └──────────┬──────────┘
                  │  write()  ← vượt biên user→kernel (§1.1: ~0,2 µs mỗi lần)
       ┌──────────▼──────────┐
       │  ② BUFFER CACHE     │  ← trong KERNEL
       │     (kernel)        │     xả bằng  fsync() / fdatasync() / O_SYNC
       └──────────┬──────────┘
                  │
       ┌──────────▼──────────┐
       │  ③ Cache của Ổ ĐĨA  │  ← phần cứng; fsync() KHÔNG luôn với tới được
       └──────────┬──────────┘
                  ▼
              đĩa vật lý
```

*(sơ đồ tổng hợp từ Figure 13-1, tr. 240)*

| Muốn | Gọi | Xả tầng nào |
|---|---|---|
| Đẩy stdio → kernel | `fflush(fp)` | ① → ② |
| Đẩy kernel → đĩa (dữ liệu **+ metadata**) | `fsync(fd)` | ② → ③ |
| Đẩy kernel → đĩa (**chỉ dữ liệu** + metadata cần thiết) | `fdatasync(fd)` | ② → ③, **ít thao tác đĩa hơn** |
| Mọi `write()` tự động đồng bộ | `open(..., O_SYNC)` | ② → ③ mỗi lần ghi |

**Vì sao có `fdatasync()`:** Kerrisk giải thích (§13.3, tr. 240) nó chỉ ép **dữ liệu** và những metadata *bắt buộc để đọc lại được* — bỏ qua thứ như thời gian sửa file. *"Using `fdatasync()` potentially reduces the number of disk operations from the two required by `fsync()` to one."* Với ứng dụng ghi log/DB tần suất cao, đó là **giảm một nửa** thao tác đĩa.

> ⚠️ **Bẫy chí tử, hỏi rất hay:** `fflush()` **không** làm dữ liệu an toàn khi mất điện — nó mới chỉ chuyển từ buffer thư viện sang buffer *kernel*. Muốn bền vững phải `fsync()`. Và ngay cả `fsync()` cũng có thể chưa đủ nếu **cache của ổ đĩa** bật write-back mà không có siêu tụ/pin (§13.3, tr. 241). 🆕 Trên thiết bị nhúng dùng eMMC/SD, đây đúng là nguyên nhân "mất file config sau khi rút điện" — và là lý do người ta dùng mẫu **ghi file tạm → `fsync` → `rename()` nguyên tử**.

### 3.2. Kích thước buffer đáng giá bao nhiêu — số thật từ sách

Kerrisk chép 100 MB bằng `read`/`write` với các `BUF_SIZE` khác nhau (Table 13-1, tr. 234; kernel 2.6.30, ext2 block 4096):

| `BUF_SIZE` (byte) | Thời gian thực (giây) |
|---|---|
| 1 | **107,43** |
| 2 | 54,16 |
| 4 | 31,72 |
| 8 | 15,59 |
| 16 | 7,50 |
| 32 | 3,76 |
| 64 | 2,19 |
| 128 | 2,16 |
| 256 | 2,06 |
| 1024 | **2,05** |

**Đọc ra hai điều — và điều thứ hai mới là điều đắt:**

1. Từ 1 byte lên 1024 byte: **nhanh hơn ~52 lần**. Tổng dữ liệu và số thao tác đĩa **y hệt nhau** — khác biệt hoàn toàn là **chi phí gọi syscall** (100 triệu lần vs ~100 nghìn lần).
2. **Từ 4096 trở lên gần như không cải thiện nữa.** Vì đến lúc đó chi phí syscall đã nhỏ hơn hẳn chi phí I/O thật. 🆕 Đây là căn cứ định lượng cho việc `BUFSIZ` của stdio thường là 4096/8192 — và là câu trả lời cho *"vì sao không dùng buffer 1 MB cho chắc?"*: **không được gì thêm, chỉ tốn RAM** — điều đáng kể trên thiết bị nhúng.

### 3.3. Ba chế độ buffer của stdio — nguồn của một bug kinh điển

`setvbuf()` chọn một trong ba (§13.2, tr. 236):

| Chế độ | Hằng số | Mặc định cho |
|---|---|---|
| Không buffer | `_IONBF` | **`stderr`** (để thông báo lỗi ra ngay) |
| Theo dòng | `_IOLBF` | terminal (khi stdout là tty) |
| Theo khối | `_IOFBF` | **file thường / pipe** |

```c
setvbuf(stdout, NULL, _IONBF, 0);   // tắt buffer stdout — hữu ích khi debug
```

> 🆕 **Bug kinh điển suy ra từ bảng này:** `./prog` in ra terminal thì thứ tự log đúng; `./prog > out.txt` thì thứ tự **loạn** hoặc mất dòng cuối khi crash. Nguyên nhân: chuyển từ **line-buffered** sang **fully-buffered** vì stdout không còn là tty. Cùng cơ chế đó gây ra **dòng bị in hai lần sau `fork()`** — buffer stdio chưa xả đã bị sao chép sang process con. Cách chữa: `fflush()` trước `fork()`, hoặc `_IONBF` khi debug.

---

## 4. Ít quan trọng — 1 dòng + tham chiếu

- **Ch. 1 — Lịch sử UNIX/Linux & chuẩn hoá** (tr. 1–20): bối cảnh SUSv3/SUSv4, POSIX.1-2001/2008. Đọc lướt; chỉ cần biết *"chuẩn nào" là câu trả lời cho "code có port được không"*.
- **§2.x — Tổng quan mọi khái niệm** (tr. 21–42): chương "xem trước" toàn sách; bỏ qua được nếu đọc các cụm chi tiết.
- **§3.5–3.6 — Quy ước chương trình mẫu & `tlpi_hdr.h`** (tr. 50–66): cách tác giả tổ chức code ví dụ, hàm `errExit()`. Biết để đọc listing.
- **§4.7 — `lseek()` và file lỗ hổng (sparse file)** (tr. 79): seek quá cuối rồi ghi → tạo "lỗ" không tốn khối đĩa. Tra khi làm việc với file ảnh/đĩa.
- **§4.8 — `ioctl()`** (tr. 84): cửa hậu cho thao tác không vừa mô hình `read`/`write`. Quan trọng ở **driver** hơn ở đây — xem [05/kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md).
- **§5.11 — `/dev/fd`** (tr. 108): `/dev/fd/N` là đường dẫn tới fd N của chính process. Mẹo cho chương trình chỉ nhận tên file.
- **§13.5–13.7 — `posix_fadvise()`, direct I/O (`O_DIRECT`), `mmap` sơ lược** (tr. 244–250): gợi ý kernel về mẫu truy cập; `O_DIRECT` bỏ qua buffer cache (chỉ dùng khi tự cài cache, vd DB).

---

## 5. Góc interview 🎯

<details><summary>1) File descriptor là gì? Vì sao <code>dup2()</code> khác với mở lại cùng file?</summary>

**fd là một số nguyên nhỏ, làm chỉ mục vào bảng file descriptor của process.** Nó không chứa thông tin gì — thông tin nằm ở hai bảng phía sau. Kernel giữ **ba** bảng:

1. **Bảng fd** — mỗi process một bảng riêng; mỗi mục có cờ (đáng kể nhất là `FD_CLOEXEC`) và con trỏ tới ②.
2. **Bảng file description** (toàn hệ thống) — mỗi mục giữ **offset hiện tại**, **status flags** (`O_APPEND`, `O_NONBLOCK`…), và con trỏ tới ③.
3. **Bảng i-node** (toàn hệ thống) — thông tin file thật: loại, quyền, kích thước, khoá.

Khác biệt suy ra trực tiếp:

- **`dup2(oldfd, newfd)`** làm `newfd` trỏ **vào cùng một file description** với `oldfd` → **chung offset, chung status flags**. Ghi qua fd nào thì offset của cả hai đều nhảy.
- **`open()` cùng file hai lần** tạo **hai file description khác nhau** (cùng i-node) → **offset độc lập**.

Ứng dụng thực tế của `dup2`: shell redirect. `ls > out.txt` = con `open("out.txt")` rồi `dup2(fd, STDOUT_FILENO)` trước `exec()`; `ls` cứ ghi vào fd 1 mà không biết gì.

Điểm hay bị bỏ sót: `FD_CLOEXEC` nằm ở tầng ① nên **`dup()` không sao chép nó** — bản sao mới luôn *tắt* close-on-exec.
</details>

<details><summary>2) <code>read()</code> trả về ít hơn số byte yêu cầu — bug hay bình thường?</summary>

**Bình thường, và không xử lý mới là bug.** `read()` trả về số byte *thực sự đọc được*, có thể ít hơn `count` mà chưa hết file. Rất hay xảy ra với **pipe, socket, terminal** (dữ liệu tới theo từng phần), và cả với file thường khi gần cuối file.

Ba giá trị trả về phải phân biệt:
- `> 0` — số byte đọc được (có thể < `count`)
- `= 0` — **end-of-file** (với socket: peer đã đóng)
- `= -1` — lỗi, xem `errno`. Riêng `EINTR` nghĩa là bị signal cắt ngang → thường phải **gọi lại**, không phải lỗi thật.

`write()` đối xứng: **partial write** khi đĩa đầy, vượt `RLIMIT_FSIZE`, hoặc pipe/socket đầy. Vì vậy code đúng phải bọc vòng lặp:

```c
ssize_t writeAll(int fd, const void *buf, size_t count) {
    const char *p = buf;
    size_t left = count;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n == -1) {
            if (errno == EINTR) continue;   // bị signal cắt → thử lại
            return -1;                       // lỗi thật
        }
        p += n; left -= n;                   // ghi được bao nhiêu trừ bấy nhiêu
    }
    return count;
}
```

**Bẫy phụ:** `read()` **không** thêm `'\0'`; muốn dùng như chuỗi phải tự đặt và cấp dư 1 byte.
</details>

<details><summary>3) <code>fflush()</code> và <code>fsync()</code> khác nhau thế nào? Khi nào cần cái nào?</summary>

Chúng xả **hai tầng buffer khác nhau**:

```
  printf() ──► [buffer stdio, USERSPACE] ──fflush()──► [buffer cache, KERNEL] ──fsync()──► đĩa
```

- **`fflush(fp)`** đẩy dữ liệu từ **buffer của thư viện C** xuống kernel bằng `write()`. Sau bước này, **process khác đọc file sẽ thấy dữ liệu** — nhưng **mất điện thì vẫn mất**.
- **`fsync(fd)`** ép kernel ghi **dữ liệu + metadata** của file xuống **đĩa vật lý**, và chỉ trả về khi xong. Đây mới là bước cho tính **bền vững**.
- **`fdatasync(fd)`** như `fsync` nhưng bỏ qua metadata không cần thiết (vd thời gian sửa) → có thể **giảm từ 2 thao tác đĩa xuống 1**; hợp cho log/DB ghi liên tục.
- **`open(..., O_SYNC)`** biến *mọi* `write()` thành đồng bộ — an toàn nhất, chậm nhất.

Thứ tự đúng khi dùng stdio và cần bền vững: **`fflush(fp)` rồi `fsync(fileno(fp))`** — thiếu bước đầu thì `fsync` chẳng ép được gì vì dữ liệu còn kẹt ở userspace.

**Cảnh báo cuối:** ngay cả `fsync()` cũng có thể chưa đủ nếu **cache của ổ đĩa** ở chế độ write-back mà không có nguồn dự phòng. Trên thiết bị nhúng (eMMC/SD), đây là nguyên nhân thật của "mất file cấu hình sau khi rút điện". Mẫu an toàn: **ghi ra file tạm → `fsync` → `rename()`** (rename là nguyên tử), và `fsync` cả thư mục chứa.
</details>

<details><summary>4) Vì sao <code>O_APPEND</code> tồn tại? <code>lseek()</code> rồi <code>write()</code> sai ở đâu?</summary>

Vì `lseek()` + `write()` là **hai syscall**, và giữa chúng process khác có thể chen vào:

```
   Process A                    Process B
   lseek(fd, 0, SEEK_END)  → offset = 1000
                                lseek(fd, 0, SEEK_END)  → offset = 1000
                                write(fd, "B", 1)       → file dài 1001
   write(fd, "A", 1)       → ghi tại 1000, ĐÈ MẤT "B"
```

`O_APPEND` khiến kernel thực hiện **"nhảy tới cuối rồi ghi" như MỘT thao tác nguyên tử**, không ai chen được vào giữa. Đó là lý do `>>` của shell và nhiều process cùng ghi một log file hoạt động đúng.

Cùng một mẫu tư duy xuất hiện ở:
- **`O_CREAT | O_EXCL`** — "tạo file chỉ khi chưa tồn tại" nguyên tử, thay cho `open` rồi kiểm tra `ENOENT` (đây cũng là cách làm lock file).
- **`pread()`/`pwrite()`** — đọc/ghi tại offset chỉ định **mà không đụng offset chung**, thay cho `lseek`+`read`. Đặc biệt quan trọng với **đa luồng chia sẻ fd**, vì các thread dùng chung offset.

**Câu chốt:** *hễ hai syscall không được để ai chen vào giữa, hãy tìm cờ hoặc syscall gộp cả hai.*

⚠️ Một cảnh báo Kerrisk có nêu và người ta hay quên: `O_APPEND` **không** đảm bảo nguyên tử trên **NFS** ở các bản cũ, vì NFS không hỗ trợ ngữ nghĩa append phía server.
</details>

<details><summary>5) Vì sao đọc/ghi từng byte lại chậm hơn hàng chục lần so với đọc/ghi theo khối?</summary>

Vì chi phí không nằm ở đĩa mà ở **việc vượt biên user↔kernel**. Mỗi `read()`/`write()` là một syscall: trap vào kernel mode, lưu/khôi phục thanh ghi, kernel kiểm tra tính hợp lệ của mọi đối số, rồi quay ra. Kerrisk đo `getppid()` — syscall gần như không làm gì — vẫn tốn **~0,22 µs**.

Số liệu của sách khi chép 100 MB (Table 13-1): buffer **1 byte mất 107,43 giây**, buffer **1024 byte chỉ mất 2,05 giây** — nhanh hơn ~52 lần, trong khi **số thao tác đĩa y hệt nhau**. Toàn bộ khác biệt là 100 triệu syscall so với ~100 nghìn syscall.

Điểm thứ hai, tinh hơn: **quá 4096 byte thì gần như không nhanh thêm** — vì lúc đó chi phí syscall đã nhỏ so với I/O thật. Nên "chọn buffer 1 MB cho chắc" **không được lợi gì, chỉ tốn RAM** — đáng kể trên thiết bị nhúng. Đây cũng là lý do `BUFSIZ` của stdio thường là 4096/8192, và là lý do stdio tồn tại: nó gom nhiều `printf` nhỏ thành ít `write()` lớn.

🆕 Cùng nguyên lý sinh ra `readv`/`writev` (gộp nhiều buffer trong một syscall), `sendfile` (không chép qua userspace), `vDSO` (`gettimeofday` không trap), và `io_uring` (gửi cả lô yêu cầu qua ring buffer chung — xem [12-dsa/ring-buffer.md §8](../../12-dsa/ring-buffer.md)).
</details>

---

## 6. Đọc thêm (tùy chọn)

- Topic repo: [04/file-io.md](../../04-linux-system-programming/file-io.md) — cùng chủ đề, gọn hơn, dạng ôn thi.
- Bank: `LNX-*` trong [linux-sysprog.md](../../14-prep/mock-interview/bank/linux-sysprog.md).
- Góc kernel của cùng câu chuyện: [LKD cụm VFS/page cache](../lkd/05-vfs-block-pagecache.md) — buffer cache ở §3.1 chính là **page cache** nhìn từ trong kernel.

---
⬅️ [README cụm TLPI](README.md) · ➡️ Cụm kế: [05 — Alternative I/O models](05-alternative-io-models.md)
