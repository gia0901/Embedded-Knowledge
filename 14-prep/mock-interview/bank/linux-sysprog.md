# LNX — Linux System Programming

> Domain `LNX`. Track dùng: `linux-sysprog`, `bsp`, `cpp-system`, `ostep`.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mục | Nội dung | Câu |
|---|---|---|
| **A** | File I/O & file descriptor | 11 |
| **B** | Process, signal & daemon | 10 |
| **C** | I/O multiplexing & event loop | 6 |
| **D** | IPC | 11 |
| **E** | Môi trường chạy: thời gian, container, mạng | 4 |

---

## A — File I/O & file descriptor

#### LNX-001 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**File descriptor là gì?**
<details><summary>Đáp án</summary>

Một số nguyên không âm, là chỉ mục vào bảng fd riêng của process; mỗi entry trỏ tới open file description trong kernel (offset, cờ) và tới đối tượng thực (inode, socket...). fd 0/1/2 = stdin/stdout/stderr. "Everything is a file" — file, pipe, socket, device đều thao tác qua fd với cùng syscall.
</details>

#### LNX-002 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md), [kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Syscall là gì và vì sao có chi phí?**
<details><summary>Đáp án</summary>

Là cách user yêu cầu kernel làm việc đặc quyền (I/O, cấp bộ nhớ): wrapper libc đặt số syscall + tham số, thực thi lệnh trap, CPU chuyển sang kernel mode, kernel thực hiện rồi trả về. Chi phí từ chuyển ngữ cảnh user↔kernel (đổi mức đặc quyền, lưu/khôi phục trạng thái) và đôi khi context switch.
</details>

#### LNX-003 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Blocking và non-blocking I/O khác nhau gì?**
<details><summary>Đáp án</summary>

Blocking (mặc định): `read` trên fd chưa có dữ liệu làm thread ngủ chờ. Non-blocking (`O_NONBLOCK`): trả về ngay, nếu chưa có dữ liệu thì `errno == EAGAIN`. Non-blocking cho phép một thread phục vụ nhiều fd (nền cho event loop với epoll).
</details>

#### LNX-019 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**`mmap` là gì? Kể vài công dụng.**
<details><summary>Đáp án</summary>

`mmap` ánh xạ một vùng vào không gian địa chỉ process, trả con trỏ để truy cập như bộ nhớ thường (kernel lo demand-paging). Công dụng: (1) **map file** — đọc/ghi file qua con trỏ, không cần read/write, hệ tự đồng bộ qua page cache (nhanh cho truy cập ngẫu nhiên file lớn); (2) **anonymous mapping** — cấp vùng nhớ lớn (nền của malloc cho khối lớn); (3) **shared memory** giữa process (`MAP_SHARED`); (4) map **MMIO/`/dev/mem`** để chọc thanh ghi từ userspace (bring-up). Đồng bộ file: `msync`.
</details>

#### LNX-021 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**`dup`/`dup2` để làm gì? Shell hiện thực `cmd > file` thế nào?**
<details><summary>Đáp án</summary>

`dup2(oldfd, newfd)` làm `newfd` trỏ tới **cùng open file description** với `oldfd` (đóng newfd nếu đang mở). Shell làm `cmd > file`: sau `fork`, trong con `open("file")` được fd f → `dup2(f, STDOUT_FILENO)` (fd 1 giờ trỏ file) → `close(f)` → `exec(cmd)`. Chương trình cứ ghi stdout như thường nhưng dữ liệu vào file. Cùng cơ chế cho `2>&1` (dup2 fd2 về fd1) và pipe (`dup2` đầu pipe vào stdin/stdout).

**Vế hay bị bỏ sót — hệ quả của "cùng file description":**

| | Chia sẻ gì | Offset |
|---|---|---|
| `dup`/`dup2` | cùng **file description** | **CHUNG** — ghi qua fd này, offset fd kia cũng nhảy |
| `fork()` | cùng **file description** | **CHUNG** giữa cha và con |
| `open()` cùng file **hai lần** | hai file description **khác nhau** | **RIÊNG**, độc lập |

Đó là lý do `cmd1 >> log` và `cmd2 >> log` chạy song song không đè nhau (cùng `O_APPEND`), còn hai lần `open` không `O_APPEND` thì có. ⚠️ `FD_CLOEXEC` nằm ở **tầng fd**, không phải file description → **`dup()` không sao chép nó**.
</details>

#### LNX-028 · 🟡 · concept · ⭐ · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Vì sao cần `O_APPEND`? `lseek()` rồi `write()` sai ở đâu?**
<details><summary>Đáp án</summary>

Vì đó là **hai syscall**, và giữa chúng process khác chen vào được:
```
   A: lseek(fd,0,SEEK_END) → 1000
                              B: lseek → 1000 ; B: write("B") → file dài 1001
   A: write("A")          → ghi tại 1000, ĐÈ MẤT "B"
```
`O_APPEND` khiến kernel làm **"nhảy tới cuối rồi ghi" như MỘT thao tác nguyên tử**. Đó là lý do `>>` của shell và nhiều process cùng ghi một log file hoạt động đúng.

**Cùng mẫu tư duy ở hai chỗ nữa:**
- **`O_CREAT | O_EXCL`** — "tạo chỉ khi chưa tồn tại" nguyên tử, thay cho `open` rồi kiểm tra `ENOENT` (nền của lock file).
- **`pread`/`pwrite`** — đọc/ghi tại offset chỉ định **không đụng offset chung**; bắt buộc khi **nhiều thread dùng chung một fd**, vì thread chia sẻ offset nên `lseek`+`read` từ hai thread giẫm lên nhau.

**Chốt:** *hễ hai syscall không được để ai chen vào giữa, hãy tìm cờ hoặc syscall gộp cả hai.* Mẫu này còn sinh ra `O_CLOEXEC`, `accept4()`, `epoll_create1()`, `sigsuspend()`, `rename()`.
</details>

#### LNX-032 · 🟡 · concept · 📦 2026-08-13 · [→ file-io](../../../04-linux-system-programming/file-io.md)
**`read()` và `fread()` khác nhau ở đâu? Khi nào chọn cái nào — và vì sao trộn hai thứ trên cùng một file là bug?**
<details><summary>Đáp án</summary>

**Khác nhau về TẦNG, không phải về tính năng:**

```
   fread()  ──► buffer stdio (libc, user space)  ──► read() ──► page cache (kernel) ──► đĩa
   read()   ──────────────────────────────────────────────────► page cache (kernel) ──► đĩa
```

| | `read()` (syscall) | `fread()` (stdio) |
|---|---|---|
| Tầng | Kernel — **syscall trần** | libc, có **buffer riêng** ở user space |
| Đọc 1 byte × 1000 lần | **1000 syscall** | ~**1 syscall** (nạp cả buffer rồi phát dần) |
| Trả về | Số **byte** (có thể ít hơn — short read) | Số **phần tử** đọc đủ |
| Báo lỗi | `-1` + `errno` | `NULL`/ngắn + `ferror()`/`feof()` |
| Async-signal-safe | ✅ **Có** | ❌ **Không** (khoá stdio) |

**Chọn cái nào:**
- **`fread`/`fprintf`** khi đọc/ghi **nhiều lần với khối nhỏ** (parse text theo dòng) — buffer gom lại giúp giảm syscall, đây là toàn bộ lý do stdio tồn tại.
- **`read`/`write`** khi: khối **lớn** (buffer stdio chỉ thêm một lần copy thừa) · cần **kiểm soát chính xác** thời điểm dữ liệu xuống kernel · làm việc với **socket/pipe/thiết bị** · trong **signal handler** · cần `O_*` flags và fd thật.

**⚠️ Vì sao KHÔNG được trộn hai thứ trên cùng file:** mỗi bên giữ **vị trí riêng**. `fread` đã nạp sẵn 4 KB vào buffer stdio thì offset của fd đã nhảy 4 KB; gọi `read(fileno(f), ...)` xen vào sẽ đọc **từ sau chỗ đó**, và dữ liệu trong buffer stdio bị bỏ qua ⇒ **mất dữ liệu, không báo lỗi**. Buộc phải trộn thì `fflush()` trước, và ý thức rằng vị trí đã dịch.

**Bẫy khác:** (1) `fwrite` **thành công không có nghĩa dữ liệu đã tới kernel** — mới vào buffer libc; cần `fflush()` (→ kernel) rồi `fsync()` (→ đĩa) ([LNX-007](linux-sysprog.md)); (2) `exit()` flush stdio nhưng **`_exit()` thì không** ⇒ mất log; (3) stdio **đổi kiểu buffer theo đích**: line-buffered khi ra terminal, **full-buffered (4 KB) khi ra pipe/file** — nên log chạy dưới systemd (stdout là pipe) hành xử khác hẳn lúc chạy tay.

**Chốt:** *"`fread` là `read` cộng thêm một buffer ở user space để gom syscall. Chọn theo kích thước khối và mức kiểm soát cần — và đừng bao giờ trộn hai tầng trên cùng một file."*
</details>

#### LNX-012 · 🟠 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Điều gì xảy ra với fd qua fork và exec?**
<details><summary>Đáp án</summary>

**Phải tách hai tầng thì mới trả lời đúng:**
- **File descriptor** = số nguyên trong **bảng fd riêng của mỗi process**.
- **Open file description** = cấu trúc trong **kernel**, giữ **offset**, cờ trạng thái, và trỏ tới inode. Nhiều fd (nhiều process) có thể trỏ về **cùng một** description.

| | `fork` | `exec` |
|---|---|---|
| Bảng fd | Con **copy** bảng fd của cha | **Giữ nguyên** — không phải chương trình mới thì reset |
| Trỏ tới đâu | fd cha & con trỏ về **cùng open file description** | như trước |
| **Offset** | **Dùng chung** — con `read` thì offset của cha cũng dịch | như trước |
| Ngoại lệ | — | fd có cờ **close-on-exec** (`O_CLOEXEC`/`FD_CLOEXEC`) **tự đóng** |

**Hệ quả thực tế của "chung offset":** cha và con cùng ghi vào một file đã mở trước `fork` sẽ **nối tiếp** nhau, không đè lên nhau — đó là lý do shell redirect (`cmd > file`) hoạt động đúng khi có nhiều process. Ngược lại, nếu mỗi bên `open()` riêng thì có **hai** description, **hai** offset độc lập → ghi đè nhau.

```c
int fd = open("f.txt", O_RDWR | O_CLOEXEC);   // ✅ mặc định nên có O_CLOEXEC
// fork() -> con vẫn có fd, chung offset
// exec() -> fd TỰ ĐÓNG nhờ O_CLOEXEC
```

**Vì sao `O_CLOEXEC` là mặc định nên dùng:** không có nó, **mọi** fd đang mở (socket, file cấu hình chứa secret, fd của thư viện khác) đều **rò sang chương trình con** — vừa là lỗ hổng bảo mật, vừa giữ tài nguyên không giải phóng (port vẫn bị chiếm dù server chính đã đóng). Trong chương trình **đa luồng**, còn phải dùng `O_CLOEXEC` **ngay trong `open()`** thay vì `fcntl` sau đó, vì giữa hai lời gọi có thể có thread khác `fork`.

**Bẫy:** (1) tưởng `fork` copy cả offset thành hai bản độc lập — **sai**, chung offset; (2) tưởng `exec` đóng hết fd — **sai**, giữ nguyên trừ CLOEXEC (đây chính là cách stdin/stdout/stderr sống sót qua exec); (3) sau `fork`, quên đóng **đầu không dùng của pipe** ở cả hai phía → `read` không bao giờ thấy EOF, treo vĩnh viễn.

**Chốt:** *"`fork` copy bảng fd nhưng dùng chung offset; `exec` giữ nguyên fd trừ khi có `O_CLOEXEC`. Cứ mở fd là đặt `O_CLOEXEC`, trừ khi cố ý muốn truyền sang con."*
</details>

#### LNX-007 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Phân biệt stdio buffer, page cache và `fsync`. Thiết bị cầm tay mất điện ngay sau khi màn hình báo "Đã lưu" mà file config lại RỖNG 0 byte — dữ liệu đã đi tới đâu, và sửa thế nào?**
<details><summary>Đáp án</summary>

**Ba tầng dữ liệu phải đi qua:**

```
   fprintf ──► ① buffer stdio (libc, USER space)
                    │ fflush()  /  buffer đầy  /  fclose()  /  exit()
                    ▼
               ② page cache (KERNEL, trong RAM)      ← write() thành công CHỈ tới đây
                    │ fsync()  /  kernel tự flush sau vài giây
                    ▼
               ③ đĩa / eMMC  ← chỉ tới đây mới sống sót qua mất điện
```

| Tầng | Ở đâu | Mất khi | Đẩy xuống bằng |
|---|---|---|---|
| ① stdio buffer | libc, user space | **Process chết** (kể cả `_exit`) | `fflush()` |
| ② page cache | Kernel, RAM | **Mất điện / panic** | `fsync()` |
| ③ Thiết bị | Flash/đĩa | — | — |

**Trả lời tình huống — thủ phạm KHÔNG phải buffer, mà là cờ `"w"`:**

`fopen(path, "w")` **cắt file về 0 byte NGAY LẬP TỨC**, trước khi bạn ghi byte nào. Cửa sổ chết nằm giữa lúc cắt và lúc dữ liệu mới xuống đĩa — mất điện trong đó thì **bản cũ đã mất, bản mới chưa có**. Đo thật:
```
ban cu tren dia                    -> 29 byte
ngay sau fopen("w"), CHUA ghi gi   -> 0 byte   <-- mat dien o day = mat sach
da fputs nhung CHUA fclose         -> 0 byte   (con trong buffer stdio)
sau fclose                         -> 29 byte  (moi xuong page cache, VAN chua xuong dia)
```

**⇒ Sửa: thay thế nguyên tử (atomic replace via rename)** — `fsync` một mình **không đủ**:
```c
int fd = open("/data/config.json.tmp", O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC, 0644);
write_all(fd, json, len);
fsync(fd);                                              // ① nội dung MỚI chắc chắn trên đĩa
close(fd);
rename("/data/config.json.tmp", "/data/config.json");   // ② thay thế NGUYÊN TỬ
int dfd = open("/data", O_RDONLY|O_DIRECTORY);
fsync(dfd); close(dfd);                                 // ③ ép cả thao tác ĐỔI TÊN xuống đĩa
```
`rename()` trong cùng filesystem là **nguyên tử**: cái tên chỉ có thể trỏ tới inode **cũ** hoặc **mới**, không có trạng thái ở giữa. Mất điện trước ② ⇒ còn nguyên bản cũ; sau ② ⇒ có bản mới đầy đủ.

**Ba chi tiết hay bị bỏ:**
1. **Thiếu ① thì ② vô nghĩa** — đổi tên vào một file mà nội dung còn nằm trong page cache.
2. **Bước ③ hay bị quên:** `fsync(fd)` chỉ đảm bảo **nội dung**, không đảm bảo **entry thư mục**.
3. Đây đúng là mẫu `git`, `sqlite`, `dpkg` dùng.

**Bẫy:** (1) `fwrite` thành công ≠ dữ liệu đã tới kernel (mới ở tầng ①); (2) `_exit()` **không flush stdio** ⇒ mất log shutdown ([LNX-030](linux-sysprog.md)); (3) `fsync` mỗi lần lưu làm thiết bị giật và **mòn flash** — cân nhắc gộp ghi, đừng `fsync` bừa.

**Chốt:** *"`fflush` đưa dữ liệu từ libc xuống kernel, `fsync` đưa từ kernel xuống đĩa — nhưng cả hai vẫn không cứu được cờ `\"w\"` cắt file. Muốn an toàn phải ghi file tạm rồi `rename`."*
</details>

#### LNX-005 · 🟡 · concept · ⭐ · [→ file-io](../../../04-linux-system-programming/file-io.md), [tcp-ip](../../../13-networking/tcp-ip.md)
**Đoạn này chạy hoàn hảo trên LAN phòng lab, ra hiện trường qua Wi-Fi thì thỉnh thoảng khách báo "mất kết nối". Vì sao?**

```c
uint8_t hdr[8];
ssize_t n = read(sock, hdr, 8);
if (n != 8) { log_error("header lỗi"); close(sock); return -1; }
```
<details><summary>Đáp án</summary>

**Cơ chế — `read` trả về ít hơn yêu cầu là chuyện BÌNH THƯỜNG, không phải lỗi** (*short read*). `read`/`write` trả **số byte thực sự xử lý**, và nó trả về **ngay khi có byte nào đó**, không đợi cho đủ `count`.

**Vì sao — hai tầng:**

- **Tầng nông (ai cũng nói được):** *"mạng không ổn định"*. Đúng nhưng vô dụng — không chỉ ra được phải sửa gì.
- **Tầng thật:** **TCP là luồng byte, KHÔNG có ranh giới message.** TCP chỉ hứa hai điều: byte tới **đủ** và **đúng thứ tự**. Nó **không** hứa *"bên kia `write` 8 byte một lần thì bạn nhận 8 byte một lần"*. 8 byte tới thành 3 + 5 vì: **MSS/phân mảnh** (header vắt qua hai segment) · **Nagle + delayed ACK** · **retransmit**.

**Vì sao lab không lộ:** LAN có RTT ~0.1 ms, không mất gói ⇒ 8 byte luôn gọn trong một segment và **đã nằm sẵn** trong receive buffer trước khi bạn gọi `read()`. Wi-Fi hiện trường có retransmit + jitter + RTT vài chục ms ⇒ cửa sổ *"mới tới một phần"* mở ra thật.

⚠️ **Đây là mẫu bug đắt nhất của nghề: đúng trên máy dev, sai ở khách — vì môi trường dev không tạo được điều kiện race.**

**Các giá trị `n` và ý nghĩa:**

| `n` | Nghĩa | Làm gì |
|---|---|---|
| `= count` | đủ | đi tiếp |
| `1 … count-1` | **short read** — mới có một phần | **đọc tiếp phần còn lại** |
| `0` | peer đã đóng (EOF) | dọn dẹp, đóng |
| `-1`, `errno == EINTR` | bị signal cắt — **không phải lỗi** | gọi lại |
| `-1`, `errno == EAGAIN` | fd non-blocking, chưa có dữ liệu | quay lại event loop |
| `-1`, khác | lỗi thật | xử lý lỗi |

```c
// ✅ Mọi read trên socket/pipe đều phải có dạng này
static int read_full(int fd, void* buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t n = read(fd, (char*)buf + got, len - got);
        if (n > 0)  { got += n; continue; }
        if (n == 0) return 0;                  // peer đóng sớm
        if (errno == EINTR) continue;
        return -1;
    }
    return 1;
}
```

**Hệ quả thiết kế bắt buộc rút ra:** **mọi protocol chạy trên TCP đều phải tự framing** — hoặc **length-prefix** (gửi độ dài trước), hoặc **delimiter** (`\n`, kèm **giới hạn buffer** để không bị gói khổng lồ làm cạn RAM).

**Bẫy:** (1) `if (n != count) return -1;` — bug kinh điển, chỉ lộ ở khách; (2) chỉ lặp cho `read` mà quên **`write` cũng short** (LNX-028); (3) test bằng file trên đĩa — file thường **luôn** trả đủ, nên bug không bao giờ lộ khi test; phải test bằng socket/pipe thật.

**Chốt:** *"TCP giao đủ byte, đúng thứ tự — nhưng không giao đúng lô. Mọi `read` phải lặp, mọi protocol phải tự đóng khung."*

> 🎤 Viết lại 2026-08-13 sau khi ứng viên đạt **2 điểm** — liệt kê đúng cả 4 giá trị `n` nhưng **cơ chế TCP luồng byte trắng hoàn toàn**. Bản cũ (277 ký tự) không hề nói tới TCP.
</details>

#### LNX-026 · 🟠 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Đọc/ghi file qua `mmap` lợi/hại gì so với `read`/`write`? Vai trò `msync`.**
<details><summary>Đáp án</summary>

**`mmap` không phải "read nhanh hơn". Nó đổi hai thứ cùng lúc — và cả hai đều có thể lỗ:**

| | `read()`/`write()` | `mmap` |
|---|---|---|
| Chuyển dữ liệu | Copy kernel ↔ user mỗi lần | **Không copy** — truy cập thẳng page cache |
| Chi phí lặp lại | 1 syscall / lần đọc | **1 page fault / 4 KB** chạm lần đầu |
| Báo lỗi I/O | **Giá trị trả về** (`-1`, `errno`) — xử lý được | **`SIGBUS`** — mặc định là chết |
| Nhảy lung tung | `lseek` + `read` mỗi lần | Con trỏ, không tốn gì thêm |
| Chia sẻ giữa process | Mỗi process một bản trong RAM | **Một** bản vật lý, mọi process dùng chung |

### ⚠️ Vì sao quét tuần tự một lần thì `mmap` KHÔNG thắng

Đây là ca hay bị hiểu sai nhất — "bỏ được copy" nghe như luôn thắng, nhưng:

- **`read()` có readahead.** Kernel nhận ra mẫu tuần tự, **nạp trước** các block kế tiếp ⇒ I/O chồng lấn với xử lý ⇒ phần copy 64 KB gần như miễn phí.
- **`mmap` trả page fault theo từng trang.** File 300 MB / 4 KB = **~76.800 lần trap vào kernel**, cộng áp lực TLB. Nhiều hơn hẳn 4.800 syscall của vòng `read()` buffer 64 KB.

⇒ Đọc **một lần, tuần tự**: `read()` thắng. Đây là con số nên tự tính ra trong đầu khi có người nói *"zero-copy nên nhanh hơn"*.

### `SIGBUS` đến từ đâu — hai kịch bản thật

```
1. FILE CO LAI DUOI CHAN MAPPING
   process A: mmap(file, 300MB) ... dang doc o offset 200MB
   process B: ftruncate(file, 50MB)
   process A: cham trang 200MB  -> khong con ung voi du lieu nao -> SIGBUS

2. LOI DOC THIET BI (flash mon, the SD hong)
   cham trang -> kernel doc block -> I/O error
   -> mmap KHONG CO gia tri tra ve o moi lan cham
   -> khong con duong nao bao loi  -> SIGBUS
```

**Đây là khác biệt bản chất đáng nhớ nhất:** đổi mô hình truy cập là **đổi luôn mô hình báo lỗi**. Với `read()` bạn kiểm `errno` rồi xử lý; với `mmap` bạn phải bắt signal hoặc chấp nhận chết. Cũng giải thích vì sao lớp bug này **chỉ nổ ở hiện trường**: flash mòn và tiến trình cập nhật chạy song song đều không có ở bàn làm việc.

### `msync` — và vì sao ghi bằng mmap khó hơn đọc

Ghi vào vùng map chỉ làm trang thành **dirty**; kernel tự đẩy xuống đĩa **lúc nào nó muốn**. Muốn đảm bảo độ bền phải `msync(addr, len, MS_SYNC)`. Mất điện giữa chừng ⇒ **không có thứ tự nào được đảm bảo** giữa các trang — khác với `write()`+`fsync()` nơi bạn kiểm soát được điểm chốt. Với dữ liệu cần toàn vẹn sau mất điện (log giao dịch, config), `write`+`fsync`+`rename` nguyên tử là mẫu an toàn hơn.

### Tiêu chí quyết định — một câu

> Chọn theo **mẫu truy cập**: **ngẫu nhiên** hoặc **chia sẻ giữa process** → `mmap`; **tuần tự / streaming / file nhỏ** → `read`/`write`.

Chữ **ngẫu nhiên** là chìa khoá (bỏ được `lseek`+`read` mỗi lần nhảy, và chỉ nạp đúng trang chạm tới). **Chia sẻ** là lý do thứ hai: 5 process map cùng file 100 MB = **100 MB** trong RAM, không phải 500 MB.

**Bẫy:** (1) `mmap` file đang bị process khác ghi ⇒ `SIGBUS` hoặc đọc ra dữ liệu nửa vời; (2) map file lớn trên hệ 32-bit ⇒ hết không gian địa chỉ ảo dù RAM còn thừa; (3) quên `msync` rồi tưởng đã ghi xong; (4) dùng `mmap` cho file nhỏ ⇒ chi phí thiết lập mapping át mọi lợi ích; (5) đo hiệu năng lần chạy **thứ hai** ⇒ page cache đã nóng, cả hai cách đều nhanh, kết luận sai.

**Chốt:** *"`mmap` đổi copy lấy page fault, và đổi mã lỗi lấy signal. Truy cập ngẫu nhiên hoặc chia sẻ thì hai vế đều lãi; quét tuần tự một lần thì cả hai đều lỗ."*
</details>

---

## B — Process, signal & daemon

#### LNX-004 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**fork và exec khác nhau? Vì sao gần như luôn dùng chung — và "khoảng giữa" hai lời gọi đó để làm gì?**
<details><summary>Đáp án</summary>

| | `fork()` | `exec()` |
|---|---|---|
| Làm gì | Tạo **process mới**, là bản sao của process gọi | **Thay thế** chương trình đang chạy bằng chương trình khác |
| PID | Con có **PID mới** | **Giữ nguyên** PID |
| Trả về | **Hai lần**: `0` ở con, PID con ở cha, `-1` lỗi | **Không trả về** nếu thành công |
| Bộ nhớ | Sao chép **copy-on-write** — rẻ | Xoá sạch, nạp image mới |

⇒ Chúng làm **hai việc trực giao**: `fork` sinh ra một *process*, `exec` quyết định process đó *chạy chương trình gì*.

**Vì sao tách làm hai — và "khoảng giữa" là điểm mấu chốt.** Sau `fork` và trước `exec`, process con đã tồn tại nhưng **vẫn đang chạy code của bạn**. Đó là **cửa sổ duy nhất** để chuẩn bị môi trường cho chương trình sắp chạy:

```c
if (fork() == 0) {                       // ---- trong process CON ----
    int fd = open("out.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    dup2(fd, STDOUT_FILENO);             // ① đổi hướng stdout
    close(fd);
    setuid(nobody_uid);                  // ② hạ quyền
    chdir("/srv");                       // ③ đổi thư mục làm việc
    execlp("tool", "tool", NULL);        // ④ giờ mới nạp chương trình mới
    _exit(127);                          // exec lỗi mới tới đây
}
```
`tool` không hề biết stdout của nó đã bị đổi hướng — nó cứ ghi stdout như thường. **Đây chính là cách shell hiện thực `cmd > file`, `cmd1 | cmd2`, và cách daemon hạ quyền trước khi chạy tác vụ.** Gộp thành một syscall thì mất hẳn khả năng này.

**Bẫy:**
1. ⭐ **`fork()` trong chương trình ĐA LUỒNG chỉ an toàn nếu `exec()` ngay.** Con chỉ thừa hưởng thread đang gọi; các thread khác biến mất *giữa chừng* cùng mọi mutex chúng đang giữ ⇒ con gọi `malloc`/`printf` là **treo**. Giữa `fork` và `exec` chỉ được gọi hàm **async-signal-safe** ([OS-021](os.md)).
2. **fd rò sang chương trình con.** `exec` **giữ nguyên** mọi fd trừ khi có `O_CLOEXEC` ⇒ socket/secret của bạn lọt sang `tool`, và cổng vẫn bị giữ sau khi daemon chết ([LNX-012](linux-sysprog.md)).
3. **Quên `_exit()` sau `exec`** — `exec` lỗi thì con **chạy tiếp code của cha**, sinh ra hai process cùng làm một việc.
4. **Không `wait()`** ⇒ zombie tích tụ, cạn PID ([OS-009](os.md)).
5. Dùng `exit()` thay `_exit()` trong con ⇒ chạy `atexit` và **flush buffer stdio đã sao chép từ cha** ⇒ output bị in **hai lần**.

**Chốt:** *"`fork` sinh process, `exec` quyết định nó chạy gì — tách ra để có khoảng giữa mà chỉnh fd, quyền, thư mục. Đó chính là cách shell làm redirect và pipe."*
</details>

#### LNX-037 · 🟢 · concept · 📦 2026-08-13 · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Signal nào không thể bắt, chặn hay bỏ qua? Vì sao?**
<details><summary>Đáp án</summary>

**`SIGKILL` (9)** và **`SIGSTOP` (19)**. Kernel từ chối mọi `sigaction`/`sigprocmask` lên chúng.

**Vì sao:** để hệ điều hành **luôn** còn đường giết hoặc dừng một tiến trình bất trị. Nếu chặn được thì một tiến trình lỗi (hoặc độc hại) có thể tự làm mình bất tử.

**Hệ quả thực tế:** `SIGKILL` **không cho tiến trình cơ hội dọn dẹp** — không chạy handler, không flush buffer, không xoá file tạm, không nhả khoá trong shared memory. Vì vậy trình tự đúng luôn là **`SIGTERM` trước, `SIGKILL` sau** (systemd làm đúng vậy: gửi `SIGTERM`, chờ `TimeoutStopSec`, rồi mới `SIGKILL`).

⚠️ **Ngoại lệ hay bị quên:** tiến trình ở **`D` state** (uninterruptible sleep) **không chết ngay cả với `SIGKILL`** — signal chỉ được xử lý khi tiến trình quay về user space, mà nó đang kẹt trong kernel chờ I/O ([LNX-022](linux-sysprog.md)).

**Chốt:** *"`SIGKILL`/`SIGSTOP` không chặn được để OS luôn còn quyền cuối — đổi lại chúng không cho dọn dẹp, nên luôn `SIGTERM` trước."*
</details>

#### LNX-006 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Vì sao nên dùng `sigaction` thay vì `signal`? Nêu một bug thật mà `signal()` gây ra.**
<details><summary>Đáp án</summary>

**Vấn đề gốc: `signal()` có ngữ nghĩa KHÔNG THỐNG NHẤT.** Cùng một dòng code cho hành vi khác nhau tuỳ hệ/tuỳ libc:

| | Kiểu System V | Kiểu BSD |
|---|---|---|
| Sau khi handler chạy lần đầu | **Reset về hành động mặc định** | Giữ nguyên handler |
| Signal cùng loại lúc handler đang chạy | **Không chặn** ⇒ handler tự chen vào chính nó | Chặn |
| Syscall bị ngắt | Trả `EINTR` | Tự restart |

**Bug thật do kiểu System V:**
```c
signal(SIGINT, handler);   // ❌ trên hệ reset-về-mặc-định
```
Người dùng nhấn Ctrl-C **lần đầu** → handler chạy, đồng thời `SIGINT` **trở lại mặc định**. Nhấn **lần hai** → chương trình **chết ngay**, không kịp dọn dẹp. Triệu chứng *"lần đầu thì xử lý đúng, lần sau chết"* — và **không tái hiện được** trên máy dev nếu libc ở đó theo kiểu BSD.

Bug thứ hai: handler chưa chạy xong đã bị chính signal đó chen vào ⇒ **đệ quy handler**, tràn stack, hoặc hỏng trạng thái dở dang.

**`sigaction` khắc phục vì nó BUỘC BẠN NÓI RÕ:**
```c
struct sigaction sa;
memset(&sa, 0, sizeof sa);          // ✅ luôn zero trước — có trường ẩn
sa.sa_handler = handler;
sigemptyset(&sa.sa_mask);           // ① signal nào bị chặn TRONG lúc handler chạy
sa.sa_flags   = 0;                  // ② hành vi restart: 0 = để EINTR đánh thức
sigaction(SIGINT, &sa, NULL);       //    handler KHÔNG bị reset — hành vi xác định
```
- **`sa_mask`** — kiểm soát tường minh signal nào bị chặn khi handler chạy (chống đệ quy).
- **`sa_flags`** — `SA_RESTART` (tự thử lại syscall), `SA_SIGINFO` (nhận thêm thông tin: ai gửi, địa chỉ gây lỗi), `SA_NOCLDWAIT` (kernel tự thu zombie).
- Handler **không bị reset**, hành vi **giống nhau trên mọi hệ POSIX**.

⚠️ **Nhưng `SA_RESTART` không phải "bật cho chắc".** Nó khiến syscall đang chặn **tự chạy lại** — nếu bạn đang dùng signal để đánh thức vòng lặp mà thoát, nó làm daemon **không tắt được** ([LNX-030](linux-sysprog.md)). Và nó **không bao giờ** restart `epoll_wait`/`select`/`poll`/`sleep` ([LNX-027](linux-sysprog.md)).

**Chốt:** *"`signal()` cho hành vi khác nhau tuỳ hệ — kinh điển là handler bị reset nên lần Ctrl-C thứ hai giết chương trình. `sigaction` buộc khai báo rõ mask và flags nên xác định trên mọi hệ."*
</details>

#### LNX-011 · 🟠 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Vì sao trong signal handler chỉ được gọi hàm async-signal-safe?**
<details><summary>Đáp án</summary>

**Cơ chế:** signal handler **chen ngang** luồng chính tại một điểm **bất kỳ** — kể cả **giữa chừng** một hàm thư viện. Nó không phải một thread khác; nó chạy **trên cùng thread**, mượn stack của nó.

Kịch bản chết người:

```
main:    malloc(…)  ──► đã lấy khoá heap, đang sửa dở free-list
           ↓ SIGALRM chen vào
handler: printf(…)  ──► printf gọi malloc  ──► xin CÙNG khoá heap
                        khoá đang do main giữ, mà main KHÔNG chạy được (đang bị chen)
                        → DEADLOCK, hoặc free-list dở dang → HEAP CORRUPTION
```

Không phải chuyện lý thuyết: `printf`, `malloc`, `free`, `syslog` đều **không reentrant** vì giữ trạng thái toàn cục + khoá. Do đó POSIX chỉ đảm bảo một danh sách hàm **async-signal-safe**: `write`, `_exit`, `signal`, `sigaction`, `kill`, `read`… (không có `printf`).

**Mẫu an toàn — handler làm ít nhất có thể:**

```c
volatile sig_atomic_t g_stop = 0;      // ✅ sig_atomic_t: ghi nguyên tử, volatile: không bị tối ưu

void handler(int sig) { g_stop = 1; }  // ✅ chỉ đặt cờ, không làm gì khác

int main(void) {
    while (!g_stop) { do_work(); }     // ✅ xử lý THẬT ở main loop
}
```

**Hai lựa chọn tốt hơn nữa** (đưa signal về mô hình event loop, hợp kiến trúc server/daemon):
- **`signalfd`** — nhận signal qua **fd**, đọc bằng `read()`, cắm thẳng vào `epoll`. Không cần handler.
- **self-pipe trick** — handler chỉ `write(pipefd, "x", 1)` (`write` là async-signal-safe), main loop đọc từ pipe. Cách di động khi không có `signalfd`.

**Bẫy:** (1) dùng `int` thay `sig_atomic_t` cho cờ — không đảm bảo ghi nguyên tử; thiếu `volatile` → compiler cache biến vào thanh ghi, vòng lặp không bao giờ thấy cờ đổi; (2) `printf` gỡ lỗi trong handler — "chạy được" phần lớn thời gian, rồi treo ngẫu nhiên; dùng `write(2, msg, len)`; (3) handler **phải bảo toàn `errno`** (lưu đầu, khôi phục cuối) — nếu không nó ghi đè `errno` của main; (4) `SIGKILL`/`SIGSTOP` **không** bắt/chặn được.

**Chốt:** *"Handler chen vào giữa chương trình trên cùng thread — gọi hàm không reentrant là tự khoá chính mình. Chỉ set cờ, hoặc chuyển signal thành fd (`signalfd`/self-pipe) rồi xử lý ở main loop."*
</details>

#### LNX-022 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Các trạng thái process trong Linux? `D` state nghĩa là gì và vì sao đáng ngại?**
<details><summary>Đáp án</summary>

`R` running/runnable, `S` sleep ngắt được (chờ sự kiện, nhận signal), `D` **uninterruptible sleep** (chờ I/O, *không* nhận signal), `T` stopped, `Z` zombie. **`D` state**: process kẹt trong syscall I/O ở kernel không thể bị đánh thức/kill (kể cả `kill -9`) tới khi I/O xong — nếu I/O treo (NFS chết, disk hỏng) process **kẹt vĩnh viễn**, load average tăng vọt dù CPU rảnh. Thấy nhiều process `D` = nghi tầng storage/driver. Xem cột STAT trong `ps`, `/proc/<pid>/stack` để biết kẹt ở đâu.
</details>

#### LNX-027 · 🟡 · concept · ⭐ · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**`EINTR` là gì? Đặt `SA_RESTART` có giải quyết hết không?**
<details><summary>Đáp án</summary>

`EINTR` = syscall đang **chặn** thì bị signal handler cắt ngang → trả `-1` với `errno == EINTR`. **Không phải lỗi thật**, chỉ là "bị ngắt, chưa làm xong". Code coi mọi `-1` là lỗi rồi thoát sẽ **chết ngẫu nhiên** khi signal tới đúng lúc.

**`SA_RESTART` KHÔNG giải quyết hết** — đây là ý phân loại. Nhóm **không bao giờ** restart dù đặt cờ:
- **`select`, `poll`, `epoll_wait`** (và `pselect`/`ppoll`/`epoll_pwait`)
- **`sleep`, `nanosleep`**, **`pause`, `sigsuspend`**
- `semop`, `msgrcv`, `msgsnd`; `read()` từ fd inotify

⇒ **Vòng lặp `EINTR` quanh `epoll_wait`/`select`/`poll` là bắt buộc**, không cờ nào thay được:
```c
int n;
do { n = epoll_wait(epfd, ev, MAX, -1); } while (n == -1 && errno == EINTR);
```

**Hai chi tiết ăn điểm:**
1. `read`/`write` chỉ restart trên **"thiết bị chậm"** (terminal, pipe, FIFO, socket). **File trên đĩa không thuộc nhóm này** → bug này **không lộ khi test với file**, chỉ lộ với socket/pipe thật.
2. Nếu `read`/`write` **đã chuyển được một phần** rồi mới bị cắt thì **không** restart mà **trả về thành công với số byte đã chuyển** — thêm một nguồn của short read/write (LNX-005).

Cần chờ fd **và** signal cùng lúc không race → `epoll_pwait`/`ppoll` (đặt sigmask nguyên tử), hoặc kéo signal vào event loop bằng `signalfd` (LNX-014).
</details>

#### LNX-038 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Cờ dùng chung giữa signal handler và main loop phải khai `volatile sig_atomic_t`. Bỏ `volatile` thì hỏng gì? Bỏ `sig_atomic_t` thì hỏng gì?**
<details><summary>Đáp án</summary>

Hai từ khoá chữa **hai bệnh khác nhau** — đây chính là điều câu hỏi kiểm tra.

**① Bỏ `volatile` ⇒ vòng lặp không bao giờ thấy cờ đổi.**
```c
int stop = 0;                     // ❌ thiếu volatile
while (!stop) { do_work(); }
```
Compiler thấy trong vòng lặp **không ai gán `stop`**, nên được phép nạp nó vào **thanh ghi một lần** rồi kiểm thanh ghi mãi mãi. Handler ghi vào **bộ nhớ**, vòng lặp đọc **thanh ghi** ⇒ chạy vĩnh viễn. `volatile` buộc **đọc lại từ bộ nhớ mỗi lần**.
⚠️ Triệu chứng đặc trưng: **bản `-O0` chạy đúng, bản `-O2` treo** — bug chỉ xuất hiện khi bật tối ưu.

**② Bỏ `sig_atomic_t` ⇒ handler có thể chen vào GIỮA một phép ghi.**
Handler chạy trên **cùng thread**, chen vào tại điểm bất kỳ — kể cả giữa hai lệnh máy của một phép gán. Kiểu rộng hơn thanh ghi (vd `long long` trên máy 32-bit) được ghi làm **nhiều lệnh** ⇒ main loop có thể đọc được **giá trị lai** chưa từng tồn tại. `sig_atomic_t` là kiểu mà chuẩn C **đảm bảo đọc/ghi bằng một lệnh không chia cắt**.

| Thiếu | Bệnh | Khi nào lộ |
|---|---|---|
| `volatile` | Cờ đổi mà không ai thấy | Bật `-O2`, chạy release |
| `sig_atomic_t` | Đọc trúng giá trị dở dang | Kiểu rộng, kiến trúc hẹp — hiếm nhưng thật |

**⚠️ `volatile` KHÔNG phải công cụ đa luồng.** Nó chỉ chống **compiler tối ưu**, hoàn toàn **không** cho đảm bảo nguyên tử hay hàng rào bộ nhớ giữa các **thread** (CPU vẫn được sắp xếp lại). Chia sẻ giữa thread phải dùng `std::atomic`/mutex. `volatile sig_atomic_t` đúng ở đây **chỉ vì** handler chạy trên cùng thread — không có chuyện hai core cùng chạm.

**Kiến trúc tốt hơn cả cờ:** **`signalfd`** hoặc **self-pipe** — signal thành một fd trong `epoll`, không còn handler, không còn `volatile`, không còn `EINTR` ([LNX-030](linux-sysprog.md)).

**Chốt:** *"`volatile` chống compiler cache biến vào thanh ghi; `sig_atomic_t` đảm bảo ghi không bị cắt đôi. Hai bệnh khác nhau — và `volatile` không bao giờ là công cụ đồng bộ đa luồng."*
</details>

#### LNX-031 · 🟠 · concept · ⭐ · 🎤 2026-08-13 · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Ta vẫn dạy "signal handler chỉ nên set cờ". Nhưng mẫu chuẩn để thu hồi zombie lại là một VÒNG LẶP đặt ngay trong handler. Mâu thuẫn ở đâu — và nó có thật không?**

```c
void on_sigchld(int) {
    while (waitpid(-1, NULL, WNOHANG) > 0) { }   // vòng lặp trong handler — ổn không?
}
```

<details><summary>Đáp án</summary>

**Không mâu thuẫn. "Chỉ set cờ" là kinh nghiệm rút gọn, không phải luật gốc.**

Ba luật thật của một handler:

| Luật | Vòng lặp `waitpid` có vi phạm? |
|---|---|
| ① Chỉ gọi hàm **async-signal-safe** | ❌ Không — `waitpid(2)` **có** trong `man 7 signal-safety` |
| ② **Không bao giờ chặn** | ❌ Không — `WNOHANG` đảm bảo trả về ngay |
| ③ **Bảo toàn `errno`** | ⚠️ **Có rủi ro — chỗ nguy hiểm thật sự** |

Vòng lặp **hữu hạn và tự dừng**: mỗi vòng thu một con đã thoát; hết con thì `waitpid` trả `0` và thoát. Nó là O(số con vừa chết). **"Ngắn" đo bằng *có chặn không*, không đo bằng số dòng.**

So sánh handler hỏng ở [OS-020](os.md): `printf` giữ khoá stdio, `free` giữ khoá heap ⇒ vi phạm luật ①, có thể **deadlock vĩnh viễn**. Khác **bản chất**, không phải khác độ dài.

### ⚠️ Luật ③ — bug thật, chạy ra được

`waitpid` **ghi đè `errno`** (đặt `ECHILD` khi hết con). Handler chen vào lúc main vừa gặp lỗi mà chưa kịp đọc `errno`:

```
KHONG khoi phuc errno  -> main doc errno = No child processes (that ra la No such file or directory) <-- SAI
CO khoi phuc errno     -> main doc errno = No such file or directory                                  OK
```

```c
void on_sigchld(int) {
    int saved = errno;                             // ✅ bắt buộc
    while (waitpid(-1, NULL, WNOHANG) > 0) { }
    errno = saved;
}
```

### Chỗ vòng-lặp-trong-handler HẾT ổn

Không phải vì dài, mà vì **nó chỉ làm được đúng một việc: thu xác**. Cần làm gì đó *với* exit status — cập nhật bảng job, log job nào hỏng, giải phóng tài nguyên — thì cần `malloc`/khoá/`printf`, **tất cả đều cấm**. Handler đụng trần ngay tại đó.

**Thang bốn bậc, chọn theo *bạn cần gì ở exit status*:**

| Bậc | Cách | Dùng khi | Giá |
|---|---|---|---|
| 0 | `SIG_IGN` / `SA_NOCLDWAIT` — kernel tự thu | **Không cần** exit status | Mất sạch exit status |
| 1 | Vòng `waitpid(WNOHANG)` **trong handler** | Chỉ chống zombie | Phải bảo toàn `errno` |
| 2 | Handler set cờ → main loop vét cạn | Cần xử lý exit status nhẹ | Trễ tới vòng lặp kế |
| 3 ⭐ | **`signalfd`** / self-pipe → vét cạn ở event loop | Daemon thật, đã có `epoll` | Thêm một fd |

**Bậc 3 là câu trả lời sạch về triết lý:** không còn handler nào cả — `SIGCHLD` thành một fd bình thường trong `epoll`, `waitpid` chạy ở ngữ cảnh bình thường, gọi gì cũng được.

⚠️ **Bẫy ở bậc 2:** vòng lặp `while (waitpid(...WNOHANG) > 0)` **vẫn phải có** ở main loop — cờ chỉ nói *"có ít nhất một con chết"*, không nói mấy con, vì **`SIGCHLD` không xếp hàng** ([LNX-004](linux-sysprog.md)). Né vòng lặp là không né được; chỉ chọn được nó chạy **ở đâu**.

**Chốt:** *"Handler bị cấm CHẶN và cấm gọi hàm không AS-safe — không bị cấm LẶP. Nhưng vòng lặp trong handler là bậc thấp nhất còn dùng được; cần hơn 'thu xác' thì phải lên signalfd."*

> Nguồn: ứng viên tự đặt sau phiên 2026-08-13 khi phát hiện mâu thuẫn biểu kiến giữa [OS-020](os.md) và [LNX-004](linux-sysprog.md). Câu hỏi tốt — giữ lại.
</details>

#### LNX-025 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Cách tạo một daemon đúng? systemd làm nhẹ việc này ra sao?**
<details><summary>Đáp án</summary>

Daemon "cổ điển" (double-fork): `fork` + parent thoát (con thành orphan, không phải leader), `setsid` (tách khỏi terminal, thành session leader), fork lần 2 (chắc chắn không giành lại tty), `chdir("/")`, đặt `umask`, đóng/redirect stdin/out/err về `/dev/null`, xử lý `SIGTERM` để tắt sạch. **Với systemd**: hầu hết việc trên là thừa — viết service kiểu `Type=simple` **chạy foreground**, systemd lo tách session, log (journald), restart (`Restart=on-failure`), phụ thuộc thứ tự (`After=`), sandbox, watchdog (`WatchdogSec`). Chỉ cần chương trình chạy tiền cảnh + xử lý SIGTERM. Đây là chuẩn hiện đại trên embedded Linux.
</details>

#### LNX-030 · 🟡 · concept · ⭐ · 🎤 2026-08-13 · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Daemon dưới systemd không tắt được bằng `systemctl stop` — phải `SIGKILL`. Handler đã viết đúng chuẩn "chỉ set cờ". Vấn đề ở đâu?**

```c
volatile sig_atomic_t stop = 0;
void on_sigterm(int sig) { stop = 1; }            // ✅ chuẩn sách vở

int main(void) {
    struct sigaction sa = { .sa_handler = on_sigterm };
    sa.sa_flags = SA_RESTART;                     // "cho chắc, tránh EINTR"
    sigaction(SIGTERM, &sa, NULL);

    for (;;) {
        char* ev = read_next_event();             // read() CHẶN trên socket
        process(ev);
        if (stop) break;
    }
}
```

<details><summary>Đáp án</summary>

**Handler đúng, nhưng `SA_RESTART` vô hiệu hoá chính nó.**

`SA_RESTART` khiến `read()` **tự chạy lại** sau khi handler chạy xong. Thiết bị lúc rảnh không có event ⇒ `read()` chặn vĩnh viễn ⇒ vòng lặp **không bao giờ tới dòng `if (stop)`**. Cờ được set nhưng không ai đọc.

**Đây là lỗi tư duy ngược:** `EINTR` bị coi là "phiền toái phải dập". Thật ra nó là **cơ chế duy nhất để signal đánh thức bạn ra khỏi chỗ đang chặn** — đúng thứ shutdown cần.

| | `sa_flags = 0` | `sa_flags = SA_RESTART` |
|---|---|---|
| `read()` khi có signal | trả `-1`, `errno = EINTR` | tự gọi lại, **không trả về** |
| Vòng lặp kiểm tra được cờ | ✅ | ❌ **không bao giờ** |
| `systemctl stop` | thoát sạch | treo tới `TimeoutStopSec` (**mặc định 90 s**) rồi **SIGKILL** |

**Hệ quả thực tế** (chứ không chỉ "chậm"): mỗi lần restart treo 90 giây; OTA update rơi vào timeout; bị SIGKILL nên **không kịp flush trạng thái** → mất dữ liệu.

**Sửa — xử lý `EINTR` theo *ý định*, không dập bằng cờ:**
```c
sa.sa_flags = 0;                       // ✅ để EINTR đánh thức
...
ssize_t n = read(fd, buf, len);
if (n < 0 && errno == EINTR) {
    if (stop) break;                   // ✅ signal shutdown → thoát
    continue;                          // ✅ signal khác     → thử lại
}
```

**Kiến trúc tốt hơn:** kéo signal vào event loop bằng **`signalfd`** ([LNX-014](linux-sysprog.md)) hoặc **self-pipe trick** — khi đó `epoll_wait` thấy signal như một fd bình thường, không còn cờ `volatile` lẫn `EINTR` để xử lý.

⚠️ **Phân biệt với [LNX-027](linux-sysprog.md):** LNX-027 nói *"`SA_RESTART` không cứu được mọi syscall"* (thiếu). Câu này là vế ngược, ít người biết hơn: *"`SA_RESTART` còn có hại — nó phá shutdown"*.

**Bẫy liên quan cùng đoạn code:** `printf(...)` rồi `_exit(0)` lúc thoát ⇒ `_exit` **không flush stdio**; dưới systemd `stdout` là **pipe tới journald** nên full-buffered ⇒ **mất đúng dòng log shutdown**. Dùng `fflush` + `_exit`, hoặc `exit()`, hoặc `write(2, ...)`.

**Chốt:** *"`EINTR` không phải lỗi cần dập — nó là cách signal gọi bạn dậy. Dập nó bằng `SA_RESTART` là tự làm daemon không tắt được."*

> 🔬 Kiểm chứng chạy thật + bản sửa đầy đủ: [coding-arena/reviewed/2026-08-13--OS-020--sigterm-handler.cpp](../coding-arena/reviewed/2026-08-13--OS-020--sigterm-handler.cpp)
</details>

---

## C — I/O multiplexing & event loop

#### LNX-033 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**I/O multiplexing giải quyết vấn đề gì? Vì sao hai cách "hiển nhiên" đều không dùng được?**
<details><summary>Đáp án</summary>

**Vấn đề:** một chương trình cần phục vụ **nhiều fd cùng lúc** (nhiều kết nối, cộng timer, cộng signal). Nhưng `read()` chặn **trên một fd** — đang chờ fd A thì fd B có dữ liệu cũng không biết.

**Hai cách ngây thơ và vì sao hỏng:**

| Cách | Hỏng ở đâu |
|---|---|
| **Một thread chặn / kết nối** | 10.000 kết nối = 10.000 thread. Mỗi thread ~8 KB kernel stack + không gian ảo; và **context switch** giữa chúng ăn hết CPU. Cache liên tục bị thổi bay |
| **Non-blocking + quét vòng tròn** (busy-poll) | Đúng về logic nhưng **đốt 100% CPU** để hỏi 10.000 fd *"có gì chưa?"* trong khi 9.990 cái đang rỗng |

**Lời giải:** hỏi kernel **một câu duy nhất** — *"trong tập fd này, cái nào đã sẵn sàng?"* — rồi **ngủ** cho tới khi có câu trả lời. CPU bằng 0 lúc chờ, và chỉ đụng vào fd thật sự có việc.

⇒ Đây là bài toán **C10K**, và là nền của Nginx/Redis/Node.js. Ba thế hệ API: `select` → `poll` → **`epoll`** (Linux), khác nhau ở chỗ *ai giữ danh sách fd* ([LNX-009](linux-sysprog.md)).

**Hai điều kiện bắt buộc đi kèm — hay bị quên:**
1. **fd phải non-blocking.** epoll chỉ nói *"có vẻ sẵn sàng"*; đọc thật vẫn có thể không có gì (spurious wakeup, dữ liệu bị checksum loại). fd blocking ⇒ treo cả loop.
2. **Không bao giờ được chặn trong loop** — kể cả tính toán nặng, `malloc` lớn, tra DNS đồng bộ, hay một mutex có thể bị giữ lâu. Một chỗ chặn 100 ms là **mọi** kết nối cùng trễ 100 ms.

**Bẫy:** multiplexing **không** làm I/O nhanh hơn — nó chỉ cho phép **một thread chờ nhiều thứ**. Tải **CPU-bound** thì nó không giúp gì (cần thread pool), và **file thường** thì nó vô dụng vì file luôn báo "sẵn sàng" ([io-multiplexing.md §7](../../../04-linux-system-programming/io-multiplexing.md)).

**Chốt:** *"Chặn thì chỉ chờ được một fd; busy-poll thì đốt CPU. Multiplexing là cách hỏi kernel một câu rồi ngủ — trả lời được cả hai vấn đề bằng một syscall."*
</details>

#### LNX-009 · 🟠 · concept · ⭐ · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**epoll khác select/poll thế nào? Vì sao scale tốt hơn?**
<details><summary>Đáp án</summary>

**Khác biệt gốc: *ai giữ danh sách fd*.**
- `select`/`poll`: **stateless** — mỗi lần gọi, user phải truyền **toàn bộ** tập fd xuống kernel, kernel **quét tuyến tính tất cả** để xem cái nào sẵn sàng, rồi copy kết quả ngược lên. Làm lại từ đầu **mỗi vòng lặp**.
- `epoll`: **stateful** — đăng ký fd **một lần** bằng `epoll_ctl`; kernel giữ tập theo dõi trong một cấu trúc bền, và khi một fd sẵn sàng thì **callback của driver đẩy nó vào danh sách ready**. `epoll_wait` chỉ việc lấy danh sách ready ra.

| | `select` | `poll` | `epoll` |
|---|---|---|---|
| Giới hạn số fd | ~**1024** (`FD_SETSIZE`) | không | không |
| Chi phí mỗi lần gọi | **O(n)** | **O(n)** | **O(k)** — k = số fd **sẵn sàng** |
| Truyền tập fd mỗi lần | ✅ phải truyền lại | ✅ | ❌ đăng ký một lần |
| Tập fd bị phá sau khi gọi | ✅ phải dựng lại | ❌ | ❌ |
| Di động | POSIX | POSIX | **chỉ Linux** (BSD: `kqueue`) |

**Vì sao chênh lệch lớn đến vậy:** với 10.000 kết nối mà chỉ 10 cái có dữ liệu, `poll` vẫn duyệt đủ 10.000 mỗi vòng; `epoll` chạm đúng 10. Đây chính là bài toán **C10K**, và là lý do Nginx/Redis/libuv đều dựng trên epoll.

**Bẫy:** (1) nói "epoll nhanh hơn" mà không nêu điều kiện — với **ít fd** (vài chục) `poll` có thể **nhanh hơn** vì không tốn syscall `epoll_ctl` và cấu trúc kernel; ưu thế chỉ xuất hiện khi **n lớn và phần lớn idle**; (2) quên `epoll` **không di động** — code cần chạy trên BSD/macOS phải trừu tượng hoá (libevent/libuv); (3) sau `fork`, epoll instance được kế thừa và **chia sẻ** — hai process cùng `epoll_wait` một fd sinh thundering herd (dùng `EPOLLEXCLUSIVE`).

**Chốt:** *"select/poll hỏi lại toàn bộ danh sách mỗi lần; epoll đăng ký một lần rồi chỉ nhận về cái đã sẵn sàng. Chi phí đi từ O(tổng số fd) xuống O(số fd có việc)."*
</details>

#### LNX-010 · 🟠 · concept · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Level-triggered và edge-triggered khác nhau? ET cần lưu ý gì?**
<details><summary>Đáp án</summary>

**Ẩn dụ điện tử đúng nguyên nghĩa:** LT báo theo **mức** (còn dữ liệu là còn báo), ET báo theo **sườn** (chỉ báo lúc **chuyển trạng thái** từ "không có" sang "có").

| | **Level-triggered** (mặc định) | **Edge-triggered** (`EPOLLET`) |
|---|---|---|
| Khi nào báo | **Mỗi lần** `epoll_wait` nếu buffer còn dữ liệu | **Một lần duy nhất** khi dữ liệu mới đến |
| Đọc chưa hết thì | Lần sau vẫn được báo lại → an toàn | **Không báo lại** → dữ liệu kẹt, kết nối "treo" |
| fd non-blocking | Nên có | **Bắt buộc** |
| Độ khó | Dễ đúng | Dễ sai |

```c
// ✅ Mẫu ET BẮT BUỘC: vét cạn tới khi EAGAIN
for (;;) {
    ssize_t n = read(fd, buf, sizeof buf);
    if (n > 0)  { handle(buf, n); continue; }        // còn -> đọc tiếp
    if (n == 0) { close(fd); break; }                 // peer đóng
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;   // ✅ đã cạn, thoát đúng chỗ
    if (errno == EINTR) continue;
    perror("read"); break;
}
```

**Vì sao ET *bắt buộc* non-blocking:** vòng lặp trên phải chạy tới khi hết dữ liệu. Nếu fd là **blocking**, lần `read` cuối (khi đã cạn) sẽ **treo cả thread** — event loop đứng hình, mọi kết nối khác chết theo. Non-blocking mới trả `EAGAIN` để bạn thoát.

**Khi nào chọn ET:** giảm số lần `epoll_wait` quay lại cho cùng một fd → có ý nghĩa ở tải rất cao, hoặc khi dùng chung một epoll cho nhiều thread (LT gây **thundering herd**: mọi thread cùng được báo). Còn lại: **mặc định LT**, đúng trước rồi mới tối ưu.

**Bẫy:** (1) quên vét cạn → bug "kết nối im lặng sau vài request", cực khó lần vì phụ thuộc kích thước dữ liệu; (2) chỉ vét cạn phía `read` mà quên phía `write` (`EPOLLOUT` cũng edge); (3) một fd "cạn" không có nghĩa các fd khác cũng vậy — vẫn phải duyệt hết mảng `epoll_wait` trả về.

**Chốt:** *"LT = 'còn hàng là còn nhắc'. ET = 'chỉ báo lúc hàng vừa tới, không nhắc lại' → bắt buộc non-blocking + đọc tới `EAGAIN`."*
</details>

#### LNX-041 · 🟠 · concept · 🎤 2026-08-15 · [→ io-multiplexing §5, §8](../../../04-linux-system-programming/io-multiplexing.md)
**Event loop `epoll` level-triggered ăn 100% CPU một core dù không client nào gửi gì. Chỉ đúng dòng gây ra, rồi sửa bằng HAI cách khác nhau về bản chất.**

```c
ev.events = EPOLLIN | EPOLLOUT;                    // đăng ký một lần lúc accept
epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
...
if (events[i].events & EPOLLOUT) do_write(fd);     // không có gì gửi thì return luôn
```

<details><summary>Đáp án</summary>

**Dòng gây bệnh: `ev.events = EPOLLIN | EPOLLOUT` đăng ký thường trực.**

**Bản chất — một câu:** `EPOLLOUT` nghĩa là ***có chỗ trống để ghi***, **không** phải ***có dữ liệu để ghi***. Socket vừa mở thì buffer gửi trống rỗng ⇒ **luôn** sẵn sàng ghi ⇒ LT báo **mỗi vòng** ⇒ `epoll_wait` không bao giờ ngủ.

> LT báo theo **mức**: *"còn ghi được là còn nhắc"* — mà socket thì gần như luôn ghi được. Đây là lý do `EPOLLOUT` **khác hẳn** `EPOLLIN` về cách dùng: `EPOLLIN` thường trực là đúng, `EPOLLOUT` thường trực là bug.

**Chạy thật** (socketpair, đăng ký `EPOLLOUT`, không ghi gì, `epoll_wait` 5 vòng):
```
LT  (EPOLLOUT):              epoll_wait bao san sang 5/5 vong   <- busy-loop
ET  (EPOLLOUT|EPOLLET):      epoll_wait bao san sang 1/5 vong   <- chi canh dau tien
```

### Hai cách sửa

| | ① `EPOLL_CTL_MOD` bật/tắt theo nhu cầu | ② `EPOLLOUT \| EPOLLET` |
|---|---|---|
| Cách làm | Chỉ đăng ký `EPOLLOUT` khi **có dữ liệu chờ gửi**; gửi hết thì gỡ ra | Đăng ký một lần; ET chỉ báo lúc **chuyển trạng thái** "đầy → có chỗ" |
| Chi phí | **2 syscall** mỗi lượt gửi | 0 syscall thừa |
| Rủi ro | Thấp, dễ đúng | Quên vét cạn `write` tới `EAGAIN` ⇒ **kẹt vĩnh viễn** |
| Chọn khi | **Mặc định** | Tải rất cao, đã có kỷ luật vét cạn cả hai chiều |

⚠️ **Cùng một luật ET áp cho cả hai chiều.** Nhiều người nhớ *"ET phải đọc tới `EAGAIN`"* nhưng không chuyển được sang *"ET phải **ghi** tới `EAGAIN`"* — trong khi cơ chế giống hệt: không vét cạn thì không có sườn tiếp theo.

**Bẫy:** (1) "sửa" bằng cách thêm timeout cho `epoll_wait` ⇒ giấu triệu chứng, CPU vẫn cháy; (2) gỡ `EPOLLOUT` mà quên gắn lại khi có dữ liệu mới ⇒ response không bao giờ được gửi; (3) bật/tắt `EPOLLOUT` theo **từng byte** thay vì theo **trạng thái hàng đợi rỗng/không rỗng** ⇒ syscall storm.

**Chốt:** *"`EPOLLIN` hỏi 'có gì để đọc không' — đăng ký thường trực là đúng. `EPOLLOUT` hỏi 'có chỗ để ghi không' — câu trả lời gần như luôn là CÓ, nên đăng ký thường trực là busy-loop."*
</details>

#### LNX-013 · 🔴 · design · ⭐ · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Mô tả kiến trúc event loop. Nguyên tắc cốt lõi là gì?**
<details><summary>Đáp án</summary>

Vòng lặp gọi `epoll_wait` ngủ tới khi có fd sẵn sàng, rồi với mỗi fd: accept kết nối mới (đăng ký vào epoll) hoặc đọc/ghi non-blocking và xử lý, quay lại chờ. Một thread phục vụ rất nhiều kết nối → ít RAM, ít context switch (Nginx/Redis/Node). Nguyên tắc: **không bao giờ block** trong event loop; mọi I/O non-blocking, tác vụ CPU nặng đẩy sang thread riêng. Mở rộng: nhiều loop trên nhiều core + thread pool.
</details>

#### LNX-039 · 🟠 · design · ⭐ · 🏗️ · 🎤 2026-08-15 · [→ io-multiplexing §8⑤](../../../04-linux-system-programming/io-multiplexing.md)
**Gateway đọc từ nhiều nguồn rồi đẩy lên uplink chậm. Sau vài phút process bị OOM killer giết. TCP vốn đã có cửa sổ nhận để điều tiết — vì sao nó không cứu được?**

```c
void on_readable(int in_fd) {
    ssize_t n = read(in_fd, buf, sizeof buf);
    if (n > 0) queue_push(&uplink_queue, buf, n);   // hàng đợi userspace, không trần
}
void on_writable(int uplink_fd) {
    // lấy từ uplink_queue ra, write() ra uplink
}
```
*Đầu vào ~2 MB/s · uplink (4G sóng yếu) 500 KB/s · thiết bị 256 MB RAM.*

<details><summary>Đáp án</summary>

**Tên bệnh: thiếu backpressure.** Producer nhanh hơn consumer, phần chênh **dồn vào buffer ứng dụng**. Không có trần ⇒ phình tới hết RAM. Số học: chênh 1,5 MB/s trên máy 256 MB ⇒ chết trong khoảng 3 phút.

### Vì sao TCP không cứu — chỗ hiểu sai phổ biến nhất

TCP **có** flow control và nó **vẫn đang chạy hoàn hảo**. Vấn đề là **chính bạn đã vô hiệu hoá nó**:

```
read() vo dieu kien:
  kernel recv buffer  --read()-->  queue cua ban (khong tran)
         ^                                ^
    LUON TRONG                       PHINH MAI
         |
  => kernel quang cao cua so DAY => ben gui cu gui het toc luc

khong goi read():
  kernel recv buffer  ---X
         ^
     DAY DAN => cua so co ve 0 => ben gui TU DUNG   <- backpressure MIEN PHI
```

> **Bạn không mất backpressure — bạn dời chỗ tắc từ kernel (có trần cứng, có kiểm soát) sang heap của mình (không trần).**

### Hai hướng xử lý — khác nhau về BẢN CHẤT

| | ① Bỏ dữ liệu (ring buffer đè cái cũ) | ② Chặn nguồn (bounded queue + backpressure) |
|---|---|---|
| Hy sinh | **Mất dữ liệu** — phải đếm được số mất | **Độ trễ** dồn ngược về nguồn |
| Hợp với | Telemetry, video preview, log — *"mới nhất là quan trọng nhất"* | **Giao dịch không được mất** — mã vạch, thanh toán, đo lường |
| Bẫy | Mất im lặng, không ai biết ⇒ **bắt buộc có counter** | Nguồn cũng đầy ⇒ áp lực lùi tiếp về sensor |

**Với máy quét mã vạch: chọn ②** — một lần quét mất là một giao dịch mất.

### Cách thi hành ② trong event loop

```c
if (queue_bytes(&q) > HIGH_WATER)          // vd 4 MB
    epoll_ctl(ep, EPOLL_CTL_MOD, in_fd, &(struct epoll_event){ .events = 0 });  // gỡ EPOLLIN
...
if (queue_bytes(&q) < LOW_WATER)           // vd 1 MB — HAI ngưỡng, không phải một
    epoll_ctl(ep, EPOLL_CTL_MOD, in_fd, &(struct epoll_event){ .events = EPOLLIN });
```

⚠️ **Phải là hysteresis hai ngưỡng.** Một ngưỡng duy nhất ⇒ queue dao động quanh nó ⇒ bật/tắt `EPOLLIN` mỗi vòng ⇒ **syscall storm**, đổi bug OOM lấy bug CPU.

**Chuỗi áp lực đúng thiết kế:** uplink chậm → queue đầy → gỡ `EPOLLIN` → kernel buffer đầy → cửa sổ về 0 → thiết bị gửi bị chặn → *nó* quyết định bỏ frame nào. **Chỗ tắc hiện ra ở nơi có đủ thông tin để xử lý.**

**Bẫy:** (1) đặt trần theo **số phần tử** thay vì **số byte** — 10.000 message 1 byte khác hẳn 10.000 message 1 MB; (2) chỉ đặt trần mà không log khi chạm trần ⇒ mất tín hiệu sớm nhất báo hệ thống quá tải; (3) tưởng `SO_RCVBUF` nhỏ là đủ — không, chừng nào còn `read()` vô điều kiện thì buffer kernel luôn trống.

**Chốt:** *"Mọi buffer userspace không có trần đều là bom hẹn giờ. TCP đã cho sẵn backpressure — `read()` vô điều kiện chính là hành động vứt nó đi."*
</details>

---

## D — IPC

#### LNX-018 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**So sánh các cơ chế IPC.**
<details><summary>Đáp án</summary>

Pipe/FIFO: byte stream đơn giản, 1 chiều. Message queue: có ranh giới message + priority. Shared memory: nhanh nhất, tự đồng bộ. Socket: liên máy/mạng. Signal: báo sự kiện, ít dữ liệu. Chọn theo: tốc độ vs ranh giới message vs phạm vi (cùng máy/khác máy).
</details>

#### LNX-008 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Cơ chế IPC nào nhanh nhất, đánh đổi gì? Và khi nào cái "nhanh nhất" đó KHÔNG đáng chọn?**
<details><summary>Đáp án</summary>

**Shared memory** — hai process map cùng khung trang vật lý, đọc/ghi trực tiếp. **0 lần copy, 0 syscall** khi truyền. Mọi cơ chế khác (pipe, socket, mq) đều **copy 2 lần** (user→kernel→user) + 2 syscall.

**Nhưng đây mới là câu trả lời đầy đủ — trả giá ba khoản:**

| Trả giá | Cụ thể |
|---|---|
| **Tự đồng bộ** | Kernel không khoá hộ. Phải mutex/semaphore process-shared, và **bắt buộc** `PTHREAD_PROCESS_SHARED` (thiếu thì hỏng **im lặng**) |
| ⭐ **Tự lo ca một bên chết** | Pipe/socket: kernel đóng fd hộ ⇒ bên kia nhận **EOF/EPIPE** và biết đường xử lý. Shared memory: chủ khoá chết ⇒ **khoá kẹt vĩnh viễn**, bên kia treo mãi. Chữa bằng `PTHREAD_MUTEX_ROBUST` (`EOWNERDEAD` + `pthread_mutex_consistent`), hoặc bỏ khoá ở đường nóng, hoặc watchdog |
| **Không có cơ chế báo hiệu** | `mmap` không cho biết *"khi nào có dữ liệu mới"* ⇒ thực tế **shm hiếm khi đi một mình**, thường kèm `eventfd`/semaphore |

**Khi nào KHÔNG đáng chọn — phép thử một câu: *"đã ĐO thấy copy là nút thắt chưa?"***
- **Message nhỏ** (vài trăm byte): chi phí bị **syscall** chi phối, shm gần như **không nhanh hơn** mà vẫn trả đủ ba khoản trên.
- **Hai bên hay crash / vòng đời độc lập** ⇒ cột "một bên chết" quan trọng hơn tốc độ.
- **Có thể phải chạy khác máy** ⇒ socket ngay từ đầu.

**Con số để quyết định:** ảnh 1920×1080 8-bit @30 fps ≈ **60 MB/s** — qua socket là **120 MB/s memcpy** liên tục, shm thắng rõ. Cùng hệ thống đó truyền **lệnh điều khiển vài chục byte** thì socket, đừng đụng shm.

⇒ **Kiến trúc "chín" hay gặp: shm cho khối dữ liệu lớn + socket/eventfd cho tín hiệu và lệnh.** Lấy tốc độ ở chỗ cần, giữ an toàn ở chỗ còn lại.

**Chốt:** *"Shared memory nhanh nhất vì 0 copy — nhưng nó trả lại cho bạn đúng phần kernel vẫn làm hộ: đồng bộ và dọn dẹp khi một bên chết. Chưa đo thì đừng chọn."*
</details>

#### LNX-016 · 🟡 · concept · ⭐ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Khi nào dùng POSIX message queue thay vì shared memory?**
<details><summary>Đáp án</summary>

Message queue cho **ranh giới message rõ ràng**, có priority, và decouple producer–consumer — hợp khi dữ liệu là chuỗi giá trị/sự kiện theo thời gian và throughput không phải nút thắt. Shared memory nhanh hơn nhưng phải tự lo mutex/semaphore và dễ race. Chọn message queue khi an toàn/dễ bảo trì quan trọng hơn tốc độ tối đa.
</details>

#### LNX-020 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Pipe và FIFO khác nhau thế nào? Process con đã thoát mà `read()` của cha vẫn không trả về EOF — vì sao?**
<details><summary>Đáp án</summary>

| | **Pipe** (`pipe()`) | **FIFO** (`mkfifo()`) |
|---|---|---|
| Tên | Ẩn danh | **Có tên trên filesystem** |
| Ai dùng được | Chỉ process **có quan hệ** (chia sẻ fd qua `fork`) | Hai process **bất kỳ**, mở qua đường dẫn |
| Vòng đời | Sống theo fd | File tên còn lại tới khi `unlink` |
| Điển hình | Shell nối `a | b` | Hai chương trình rời nhau |

Cả hai là **byte stream một chiều**, **không có ranh giới message** ⇒ phải tự framing.

**⭐ EOF xảy ra khi nào — luật hay bị hiểu sai nhất:**

> `read()` trả `0` (EOF) chỉ khi **MỌI fd trỏ tới đầu GHI đã đóng** — không phải khi process con thoát.

```c
int p[2]; pipe(p);
if (fork() == 0) { dup2(p[1], STDOUT_FILENO); close(p[0]); execlp("tool","tool",NULL); }
close(p[1]);                     // ✅✅ THIẾU DÒNG NÀY LÀ TREO VĨNH VIỄN
while ((n = read(p[0], buf, sizeof buf)) > 0) { /* ... */ }
```
Cha `fork` xong vẫn giữ **bản sao** của `p[1]`. Kernel thấy còn writer ⇒ không bao giờ báo EOF, dù con đã chết từ lâu. Đo thật:
```
cha QUEN close(p[1])   -> TREO vinh vien trong read()
cha CO   close(p[1])   -> doc xong, thay EOF, thoat sach
```
⇒ **Quy tắc: sau `fork`, mỗi bên đóng NGAY đầu pipe mình không dùng.** Đây là bug pipe phổ biến số một.

**Ca ngược lại — ghi khi không còn reader ⇒ mặc định CHẾT.** Kernel gửi **`SIGPIPE`**, hành vi mặc định là **giết process** — im lặng, không log. Đây là lý do daemon "tự nhiên biến mất" khi client ngắt giữa chừng:
```c
signal(SIGPIPE, SIG_IGN);   // ✅ gần như bắt buộc cho mọi daemon
if (write(fd, buf, n) < 0 && errno == EPIPE) { /* peer đã đóng */ }
```

**Sức chứa & nguyên tử:** pipe chứa **64 KB** (đầy thì chặn — đây là **backpressure miễn phí**); ghi **≤ `PIPE_BUF` = 4096 byte** là **nguyên tử**, trên ngưỡng đó **bị xé** và nhiều nguồn ghi chung sẽ trộn vào nhau.

**Bẫy deadlock hai chiều:** cha ghi cho con qua pipe A, con ghi cho cha qua pipe B, không ai đọc ⇒ cả hai đầy 64 KB ⇒ **cả hai cùng chặn khi ghi**. Cùng họ: đọc `stdout` của con bằng vòng `read` trong khi con xả rất nhiều ra `stderr` mà không ai đọc ⇒ con chặn, cha chờ mãi. Chữa bằng `poll`/`epoll` trên **cả hai** đầu.

**Chốt:** *"Pipe chỉ báo EOF khi đầu ghi CUỐI CÙNG đóng — nên quên `close(p[1])` ở cha là treo vĩnh viễn. Và ghi vào pipe không còn reader thì mặc định bạn CHẾT, không phải nhận lỗi."*
</details>

#### LNX-035 · 🟡 · concept · ⭐ · 📦 2026-08-13 · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Unix domain socket hơn TCP loopback và hơn pipe ở chỗ nào? Vì sao nó nên là lựa chọn MẶC ĐỊNH cho IPC cùng máy?**
<details><summary>Đáp án</summary>

**Hơn TCP loopback:** không đi qua stack TCP/IP — không checksum, không định tuyến, không cửa sổ tắc nghẽn, không handshake ⇒ nhanh và trễ thấp hơn. Nhưng **cùng API socket**, nên đổi sang TCP chỉ là đổi address family. **Cộng thêm:** kernel biết được **UID/PID của bên kia** (`SO_PEERCRED`) — xác thực cục bộ mà TCP không cho.

**Hơn pipe — ba thứ pipe không có:**

| | Vì sao quan trọng |
|---|---|
| **Hai chiều + nhiều client** | Pipe một chiều, chỉ giữa process họ hàng (FIFO thì bất kỳ nhưng vẫn một chiều) |
| **`SOCK_SEQPACKET`** | **Giữ nguyên ranh giới message** *và* vẫn tin cậy, đúng thứ tự — điểm giữa của `SOCK_STREAM` (phải tự framing) và `SOCK_DGRAM`. Ít người biết, rất đáng dùng |
| ⭐ **Truyền được file descriptor** (`SCM_RIGHTS`) | Process A `accept` một kết nối rồi **chuyển hẳn fd đó** cho worker B. **Không cơ chế IPC nào khác làm được.** Nền của systemd socket activation và của việc truyền `memfd` để zero-copy |

**Vì sao nên là mặc định:** nó cân bằng tốt nhất bốn trục quyết định — giữ được ranh giới message, **kernel dọn dẹp hộ khi một bên chết** (bên kia nhận EOF/EPIPE thay vì treo), đủ nhanh cho hầu hết tải, và đổi sang mạng gần như miễn phí. Shared memory là **tối ưu hoá** cho ca băng thông cao, chỉ dùng sau khi **đo** ([LNX-008](linux-sysprog.md)).

**Bẫy:** (1) `sun_path` chỉ **108 byte** — đường dẫn dài bị cắt **âm thầm**; (2) file socket **ở lại trên đĩa** sau khi process chết ⇒ lần khởi động sau `bind` lỗi `EADDRINUSE`, phải `unlink` trước — hoặc dùng **abstract namespace** (`sun_path[0]='\0'`, Linux-only) để khỏi đụng filesystem và tự biến mất; (3) quên chặn `SIGPIPE` ⇒ peer đóng là **process chết im lặng**.

**Chốt:** *"Unix socket giữ ranh giới (SEQPACKET), truyền được cả fd, và kernel báo cho bạn khi bên kia chết — đó là lý do nó nên là mặc định, còn shared memory chỉ là tối ưu hoá sau khi đo."*
</details>

#### LNX-036 · 🟡 · concept · 📦 2026-08-13 · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**POSIX IPC và System V IPC khác nhau? Nên dùng cái nào, vì sao?**
<details><summary>Đáp án</summary>

| | **POSIX** (nên dùng) | **System V** (legacy) |
|---|---|---|
| Shared memory | `shm_open` + `mmap` | `shmget`/`shmat` |
| Message queue | `mq_open` | `msgget`/`msgsnd` |
| Semaphore | `sem_open`/`sem_init` | `semget`/`semop` |
| Định danh | Tên dạng **`/name`** | **key** (`ftok`) + ID số |
| Là fd? | ✅ Phần lớn ⇒ **cắm được vào `epoll`** | ❌ Không |
| Dọn dẹp | `*_unlink`, thấy được trong `/dev/shm` | `ipcrm` — **dễ rò**, không thấy trong filesystem |

**Ba lý do thật để chọn POSIX** (không phải "vì nó mới hơn"):
1. ⭐ **Là fd** ⇒ gộp được vào event loop cùng socket/timer/signal. System V dùng ID riêng nên **không thể** chờ nó cùng lúc với các nguồn sự kiện khác — buộc phải thêm thread hoặc polling.
2. **Vòng đời rõ ràng:** đối tượng POSIX hiện ra trong namespace filesystem, thấy được và xoá được bằng công cụ thường.
3. **`ftok()` có thể ĐỤNG key.** Nó băm từ (inode, device) ⇒ hai ứng dụng khác nhau có thể **vô tình sinh ra cùng key** và giẫm lên nhau — hỏng theo kiểu cực khó lần. POSIX dùng tên tường minh nên không có lớp lỗi này.

**Biết System V để làm gì:** đọc và bảo trì code cũ. Và một điểm System V thực sự có mà POSIX không: **semaphore set** (thao tác nguyên tử trên **nhiều** semaphore cùng lúc) — hiếm cần, nhưng nếu gặp code cũ dùng nó thì đừng vội "hiện đại hoá".

**Bẫy:** cả hai loại đối tượng đều **sống lâu hơn process** — chết mà quên unlink thì lần khởi động sau gặp lại trạng thái cũ (khoá đang bị giữ, dữ liệu cũ). Đây là lỗi chung, không phải lỗi riêng của System V.

**Chốt:** *"Chọn POSIX vì nó là fd — cắm được vào epoll, thấy được, xoá được. Còn `ftok` của System V có thể đụng key giữa hai ứng dụng không liên quan."*
</details>

#### LNX-042 · 🟠 · concept · 🎤 2026-08-15 · [→ ipc-linux §4.3](../../../04-linux-system-programming/ipc-linux.md)
**Bạn đặt một `pthread_mutex_t` vào vùng shared memory cho hai process dùng chung, nhưng QUÊN `PTHREAD_PROCESS_SHARED`. Hai process đang thao tác lên MỘT mutex hay HAI bản sao — và chuyện gì xảy ra?**

<details><summary>Đáp án</summary>

**MỘT.** Hai process `mmap` cùng vùng ⇒ cùng **một trang vật lý** ⇒ đúng một đối tượng mutex, không có bản sao nào cả. Đây là chỗ trực giác hay sai: *"mỗi process một không gian địa chỉ"* đúng với **địa chỉ ảo**, nhưng shm cố tình cho hai địa chỉ ảo khác nhau **trỏ về cùng RAM vật lý**.

**Vậy cờ đó đổi cái gì?** Nó không tạo/không nhân bản gì — nó **cấm thư viện dùng các tối ưu chỉ đúng trong một process**:

```
PROCESS_PRIVATE (mac dinh):  futex "private"
     -> khoa duoc danh theo KHONG GIAN DIA CHI cua process
     -> process A ngu tren khoa K_A, process B danh thuc khoa K_B
     -> HAI KHOA KHAC NHAU cho cung mot dia chi vat ly  => MAT WAKEUP

PROCESS_SHARED:              futex "shared"
     -> khoa danh theo TRANG VAT LY  => hai ben khop nhau
```

**Mặc định là `PTHREAD_PROCESS_PRIVATE`** — tức **mặc định là cái sai** khi đặt mutex vào shm.

**Vì sao nguy hiểm hơn một lỗi bình thường:**

> ⚠️ Thiếu cờ thì mutex chỉ đúng trong **một** process — hai process vẫn chạy song song vào vùng dữ liệu mà **không hề báo lỗi**. Đây là **bug im lặng**, rất khó lần. — [ipc-linux.md §4.3](../../../04-linux-system-programming/ipc-linux.md)

Không crash, không mã lỗi, không log. `pthread_mutex_lock` vẫn trả về 0. Nó **chạy đúng phần lớn thời gian** (khi hai bên không thật sự tranh chấp), rồi thỉnh thoảng hỏng dữ liệu — Heisenbug kinh điển.

**Khai báo đúng, đủ cả hai cờ:**
```c
pthread_mutexattr_t a;
pthread_mutexattr_init(&a);
pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);   // ① bắt buộc cho shm
pthread_mutexattr_setrobust(&a, PTHREAD_MUTEX_ROBUST);      // ② cứu khi một bên chết
pthread_mutex_init(&hdr->lock, &a);
```
① mà thiếu ② thì một bên chết khi đang giữ khoá ⇒ bên kia **treo vĩnh viễn** (xem [LNX-015](linux-sysprog.md)).

**Cùng luật này áp cho:** `pthread_cond_t`, `pthread_rwlock_t`, semaphore — mọi primitive đặt trong shm đều cần `pshared`.

**Chốt:** *"Đặt mutex vào shm là chia sẻ đúng một đối tượng — nhưng thư viện không tự biết điều đó, và mặc định nó giả định ngược lại. Quên cờ là mất đồng bộ trong im lặng."*
</details>

---
⬅️ [Bank index](README.md)

#### LNX-017 · 🟡 · design · ⭐ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Sensor bắn 200 mẫu/giây vào message queue, luồng xử lý chỉ kịp 50/giây. Sau vài phút `mq_send` bắt đầu chặn và kéo trễ cả luồng đọc sensor. Xử lý thế nào?**
<details><summary>Đáp án</summary>

**Cơ chế:** mặc định `mq_send` **chặn** khi hàng đầy; `O_NONBLOCK` thì trả `-1`/`EAGAIN`.

⚠️ Nhưng **đổi cờ chưa phải lời giải** — nó chỉ đổi *cách hỏng*. Câu hỏi thật là **nghiệp vụ**: *dữ liệu này mất được không?*

| Loại dữ liệu | Đầy thì làm gì | Vì sao |
|---|---|---|
| **Trạng thái** (ánh sáng, nhiệt độ, vị trí, tốc độ) | ⭐ **Đè cái cũ** — latest-value-wins / drop-oldest | Mẫu cũ đã **SAI** so với hiện tại; giữ nó lại là giữ rác **và** làm hệ phản ứng theo dữ liệu quá khứ |
| **Sự kiện / lệnh** (job in, phím bấm, giao dịch) | **Chặn / backpressure / lưu bền** | Mỗi phần tử là một việc phải làm; mất là **sai nghiệp vụ** |

**Áp vào ca này:** độ sáng là **trạng thái** ⇒ đè cái cũ. Consumer chỉ cần **giá trị mới nhất** để ramp tới, không cần lịch sử 4 mẫu đã lỗi thời.

**Ba việc phải làm cùng lúc:**
1. **Đè cái cũ** — hoặc dùng một ô "giá trị đích" (không cần hàng đợi), hoặc ring buffer ghi đè.
2. ⭐ **Đếm số mẫu bị bỏ** (`dropped_count`). Mất dữ liệu thì được, **không biết mình mất** thì không — không có biến này thì sau không ai chứng minh được hệ có bỏ mẫu hay không.
3. **Giảm ngay từ nguồn** (rate limiting). 200 Hz cho một cái đèn nền là **thừa** — hạ tần số lấy mẫu rẻ hơn mọi cách xử lý phía sau.

**⚠️ Vì sao "cứ để nó chặn" là lựa chọn tệ ở đây:** chặn `mq_send` làm **luồng đọc sensor trễ theo**, nên nó bỏ lỡ luôn các mẫu **tương lai** — tức là bạn vẫn mất dữ liệu, nhưng mất **ngẫu nhiên và không đếm được**, lại còn phá timing của luồng đọc. Chặn chỉ đúng khi phía trên **có thể chậm lại thật** (backpressure lan tới tận nguồn).

**Bẫy:** (1) giới hạn mặc định của POSIX mq rất thấp — `msg_max` **10**, nâng cần root ⇒ hàng đầy sớm hơn tưởng; (2) tăng kích thước hàng để "hết đầy" chỉ **dời thời điểm hỏng** và **tăng độ trễ** (dữ liệu càng cũ khi tới tay consumer); (3) quên rằng consumer chậm là **vấn đề gốc** — nếu nó chậm vĩnh viễn thì không chính sách hàng đợi nào cứu được.

**Chốt:** *"Hàng đầy không phải lỗi kỹ thuật mà là câu hỏi nghiệp vụ: dữ liệu trạng thái thì đè cái cũ và đếm số bỏ; dữ liệu sự kiện thì phải backpressure. Và luôn hỏi vì sao producer nhanh gấp 4 lần consumer."*
</details>

#### LNX-014 · 🔴 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Bạn dùng những cơ chế Linux nào để tích hợp signal/timer vào event loop?**
<details><summary>Đáp án</summary>

Các fd-based primitives: `signalfd` (nhận signal qua fd, tránh handler async-signal-safe), `timerfd` (timer phát sự kiện qua fd cho timeout), `eventfd` (đếm/báo hiệu nhẹ giữa thread/process), `memfd` (vùng nhớ ẩn danh có fd). Nhờ biến mọi nguồn sự kiện thành fd, event loop xử lý đồng nhất socket + timer + signal + notify chỉ qua epoll.
</details>

#### LNX-015 · 🔴 · design · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Thiết kế kênh giao tiếp hiệu năng cao giữa hai process trên cùng máy — và làm sao nó sống sót khi một bên crash?**
<details><summary>Đáp án</summary>

**Khung thiết kế — dữ liệu đi một đường, tín hiệu đi đường khác:**

| Thành phần | Chọn gì | Vì sao |
|---|---|---|
| **Đường dữ liệu lớn** | Shared memory (`shm_open` + `ftruncate` + `mmap`) | **0 copy, 0 syscall** — các IPC khác copy 2 lần qua kernel |
| **Cấu trúc trong vùng shm** | Ring buffer + **header có magic/version** | Producer–consumer; magic để nhận biết vùng cũ còn sót lại sau crash |
| **Báo hiệu** | **`eventfd`** (hoặc semaphore) | shm **không tự báo** có dữ liệu mới; eventfd cắm thẳng vào `epoll` |
| **Kênh điều khiển** | **Unix domain socket** | Có ranh giới message, truyền được fd, kernel báo khi bên kia chết |

⇒ **Đừng dùng shm cho mọi thứ.** Message nhỏ (lệnh, cấu hình) thì chi phí bị **syscall** chi phối, shm không nhanh hơn mà vẫn phải trả giá đồng bộ.

**⭐ Phần quyết định: một bên crash thì sao?**

Pipe/socket được kernel dọn hộ (bên kia nhận EOF/EPIPE). Shared memory thì **không** — mutex nằm **trong vùng nhớ**, kernel không biết nó là gì ⇒ chủ khoá chết là **khoá kẹt vĩnh viễn**. Đo thật:
```
mutex thuong    lock() -> ETIMEDOUT    <-- dung lock() thuong la TREO VINH VIEN
ROBUST          lock() -> EOWNERDEAD   <-- cuu duoc, goi mutex_consistent()
```

```c
pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);   // ① BẮT BUỘC — thiếu thì hỏng IM LẶNG
pthread_mutexattr_setrobust(&a, PTHREAD_MUTEX_ROBUST);      // ② sống sót khi chủ chết
...
if (pthread_mutex_lock(&hdr->lock) == EOWNERDEAD) {
    repair_shared_state(hdr);                 // dữ liệu có thể DỞ DANG
    pthread_mutex_consistent(&hdr->lock);
}
```

**Hoặc bỏ hẳn khoá ở đường nóng** ⭐ — ring buffer với **chỉ số đọc/ghi riêng** mỗi bên: consumer chết thì producer vẫn chạy, chỉ đè lên ô cũ. Hợp nhất khi mất dữ liệu cũ là chấp nhận được (ảnh, mẫu cảm biến) — và tránh được toàn bộ lớp vấn đề stale lock.

**Bẫy phải nêu:**
1. **Quên `ftruncate`** ⇒ đối tượng cỡ 0, `mmap` vẫn **thành công**, chạm vào là **`SIGBUS`** (kiểm giá trị trả về của `mmap` không bắt được lỗi này).
2. **Quên `shm_unlink`** ⇒ vùng shm **sống lâu hơn process**; lần khởi động sau gặp lại dữ liệu cũ + khoá cũ đang bị giữ.
3. **Quên `PTHREAD_PROCESS_SHARED`** ⇒ mutex chỉ đúng trong một process, hai bên vẫn giẫm lên nhau mà **không báo lỗi**.
4. **False sharing:** đặt chỉ số đọc và chỉ số ghi **cùng một cache line** ⇒ hai core bật qua lại cache line đó, mất phần lớn hiệu năng vừa giành được.

**Chốt:** *"shm cho khối dữ liệu + eventfd cho tín hiệu + socket cho lệnh. Và câu hỏi thật không phải 'nhanh cỡ nào' mà là 'một bên chết thì bên kia có treo không' — đó là thứ shm bắt bạn tự lo."*
</details>

#### LNX-034 · 🔴 · design · ⭐ · 🏗️ · 📦 2026-08-13 · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Process `scanner` đọc ảnh 1920×1080 8-bit @30fps (~60 MB/s) đưa cho process `decoder` giải mã. Tách hai process vì decoder hay crash, không được kéo scanner chết theo. Chọn IPC nào?**
<details><summary>Đáp án</summary>

Trả lời theo **bốn trục** ([ipc-linux.md §2](../../../04-linux-system-programming/ipc-linux.md)), đừng nhảy thẳng vào tên cơ chế.

**① Băng thông:** 60 MB/s qua socket/pipe nghĩa là **120 MB/s memcpy** liên tục (copy 2 lần) cộng 2 syscall mỗi khung. Ở mức này chi phí copy là **thật** ⇒ **shared memory** hợp lý. *(Ngược lại: lệnh điều khiển vài chục byte thì socket, đừng đụng shm.)*

**② Một bên chết — đề đã cài sẵn bẫy ở đây.** *"decoder hay crash"*: nếu đồng bộ bằng mutex process-shared thường, decoder chết khi **đang giữ khoá** ⇒ scanner gọi `lock()` và **treo vĩnh viễn**.

⇒ **Nghịch lý phải nói ra được:** tách hai process để crash bên này không giết bên kia — rồi đặt một mutex chung vào giữa, **nối lại đúng thứ vừa tách**.

| Cách xử lý | Cơ chế | Đánh đổi |
|---|---|---|
| **Robust mutex** | `PTHREAD_MUTEX_ROBUST` ⇒ `lock()` trả **`EOWNERDEAD`** thay vì treo; gọi `pthread_mutex_consistent()` | Phải viết code khôi phục — dữ liệu có thể **dở dang** |
| **Bỏ khoá ở đường nóng** ⭐ | Ring buffer, chỉ số đọc/ghi **riêng** mỗi bên; consumer chết thì producer vẫn chạy, chỉ đè khung cũ | Khó hơn — nhưng **hợp nhất bài này** vì mất khung ảnh cũ là chấp nhận được |
| **Watchdog** | Giám sát thấy decoder chết ⇒ reset vùng shm + khởi động lại | Đơn giản nhất, có gián đoạn |

**③ Báo hiệu:** shm **không tự báo** có khung mới ⇒ kèm **`eventfd`** để đánh thức, cắm thẳng vào `epoll`. Thiếu ý này là trả lời cụt.

**④ Khác máy?** Không ⇒ khỏi cần socket cho đường dữ liệu.

**Kiến trúc chốt:** **shm cho khung ảnh + eventfd cho tín hiệu + ring buffer không khoá, đè khung cũ khi consumer chậm, kèm biến đếm khung mất.** Lệnh điều khiển đi Unix socket riêng.

⚠️ Nhớ hai chi tiết dễ mất điểm: vùng shm phải `ftruncate` trước khi `mmap` (thiếu ⇒ **`SIGBUS`** lúc chạm), và shm **sống lâu hơn process** ⇒ phải quyết định lúc khởi động là dùng lại hay xoá làm mới (đặt magic + version ở header).

**Chốt:** *"60 MB/s thì shm đáng giá, nhưng 'decoder hay crash' mới là vế quyết định — phải chọn cơ chế đồng bộ sống sót được khi một bên chết, nếu không thì việc tách process thành vô nghĩa."*
</details>

---

## E — Môi trường chạy: thời gian, container, mạng

#### LNX-029 · 🟡 · concept · ⭐ · [→ TLPI cụm 03 §5](../../../15-book-summaries/the-linux-programming-interface/03-signals-and-timers.md)
**Thiết bị nhúng KHÔNG có RTC pin nuôi — mất điện là đồng hồ về mốc mặc định, tới khi NTP đồng bộ được (30–60 s sau boot). Đoạn timeout này sinh ra HAI triệu chứng ngược nhau ngoài hiện trường. Vì sao?**

```c
time_t start = time(NULL);
send_scan_command();
while (!scan_done()) {
    if (time(NULL) - start > 5) { log("timeout"); return -1; }
}
```
- ① Đôi khi **timeout ngay lập tức** dù module trả lời bình thường.
- ② Đôi khi **treo hàng giờ** mới chịu timeout.

<details><summary>Đáp án</summary>

**Cơ chế — `time()` đọc `CLOCK_REALTIME`, mà giờ theo lịch thì NHẢY ĐƯỢC.**

Một triệu chứng thì dễ đoán; **hai triệu chứng ngược nhau từ cùng một dòng code** chính là chữ ký của *"có ai đó sửa đồng hồ giữa hai lần đọc"*:

| | NTP chỉnh **tiến** (nhảy vọt lên) | NTP chỉnh **lùi** |
|---|---|---|
| `time(NULL) - start` | vọt lên hàng nghìn giây | thành số **âm** / rất nhỏ |
| Triệu chứng | ① **timeout ngay lập tức** | ② **treo hàng giờ** |

Điều kiện của đề — **không RTC**, NTP đồng bộ 30–60 s sau boot — làm cú nhảy **gần như chắc chắn xảy ra**, đúng lúc mọi service đang khởi động với timeout. Trên thiết bị nhúng đây là ca **mặc định**, không phải ca hiếm.

**Ai sửa được `CLOCK_REALTIME`:** NTP, admin đổi giờ, đổi múi giờ/DST, leap second.
**`CLOCK_MONOTONIC`:** đếm đơn điệu từ một mốc bất kỳ (thường là lúc boot) — **không ai chỉnh được, không bao giờ nhảy lùi**.

```c
// ✅ Đo khoảng thời gian
struct timespec t0, t1;
clock_gettime(CLOCK_MONOTONIC, &t0);
...
clock_gettime(CLOCK_MONOTONIC, &t1);
double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
```

**Quy tắc chọn — hỏi đúng một câu:**

| Bạn đang hỏi gì | C | C++ |
|---|---|---|
| *"bây giờ là mấy giờ"* — timestamp log, mtime, hạn chứng chỉ | `CLOCK_REALTIME` | `std::chrono::system_clock` |
| *"bao lâu"* — timeout, đo hiệu năng, lập lịch định kỳ | **`CLOCK_MONOTONIC`** | **`std::chrono::steady_clock`** |

> 💡 `steady_clock` tồn tại song song với `system_clock` trong C++ **chính vì lý do này** — đây là câu hỏi C++ hay được hỏi kèm.

**Bẫy:** (1) dùng `system_clock` cho `condition_variable::wait_for` — NTP nhảy làm wait sai/treo; (2) tưởng `CLOCK_MONOTONIC` đếm cả lúc máy **suspend** — không, cần `CLOCK_BOOTTIME` nếu thiết bị có ngủ; (3) lấy hiệu hai `time_t` rồi gán vào biến **unsigned** — số âm hoá thành số khổng lồ, timeout gần như vĩnh viễn.

**Chốt:** *"Hỏi 'mấy giờ' thì REALTIME. Hỏi 'bao lâu' thì MONOTONIC. Nhầm chỗ là sinh timeout âm hoặc timeout vô tận."*

> 🎤 Viết lại 2026-08-13 sau khi ứng viên đạt **1 điểm** (trắng, phải thu hẹp mới ra được epoch + 1 triệu chứng). Link nguồn cũ trỏ `→ processes-signals` không có mục clock; nội dung thật ở TLPI cụm 03 §5.

*(NTP = Network Time Protocol — giao thức đồng bộ đồng hồ máy qua mạng. Nó **chỉnh giờ hệ thống**, đó là toàn bộ lý do câu này tồn tại.)*
</details>

#### LNX-023 · 🟡 · concept · ⭐ · [→ tools](../../../09-debugging/tools.md), [kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Thiết bị nhúng rootfs tối giản: không có `lsof`, không có `strace`. Bạn cần biết một daemon đang mở fd nào và đang kẹt ở đâu trong kernel. Lấy thông tin đó ở đâu, và vì sao "đọc file" lại ra được trạng thái sống của kernel?**
<details><summary>Đáp án</summary>

**Cơ chế trước — chúng KHÔNG phải file.**

`/proc` và `/sys` là **virtual filesystem**: **không có byte nào nằm trên đĩa**. Khi bạn `read()`, kernel **chạy một hàm sinh nội dung ngay tại thời điểm đó** rồi trả về. Mỗi lần đọc là một lần chụp trạng thái mới.

⇒ Đó là *"everything is a file"* áp cho **trạng thái kernel**: tra được mọi thứ chỉ bằng `open`/`read`, **không cần syscall riêng, không cần cài công cụ**.

**Hệ quả ăn điểm:** `ps` và `top` **không có syscall riêng** — không tồn tại `get_process_list()`. Chúng chỉ **duyệt `/proc/<pid>/`**. Nghĩa là bạn luôn tự làm được việc của chúng, kể cả trên rootfs trống trơn.

**Phân vai hai thư mục:**

| | `/proc` | `/sys` (sysfs) |
|---|---|---|
| Về cái gì | **Process** + thông tin hệ thống lịch sử | **Thiết bị & driver** — device model (bus/device/driver/class) |
| Cấu trúc | tạp, mỗi file một định dạng riêng | có cấu trúc, **một giá trị / một file** |
| Sinh ra khi nào | Unix cổ, gom dần đủ thứ vào | Sau này, để dọn đúng phần device |
| Ví dụ | `/proc/<pid>/fd/`, `/proc/<pid>/stack`, `/proc/<pid>/maps`, `/proc/meminfo`, `/proc/interrupts` | `/sys/class/gpio/`, `/sys/class/thermal/thermal_zone0/temp`, `/sys/bus/i2c/devices/` |

**Thay công cụ khi không có công cụ:**

| Muốn biết | Công cụ quen | Thay bằng |
|---|---|---|
| Process mở fd nào | `lsof` | `ls -l /proc/<pid>/fd/` |
| Process `D` kẹt ở đâu trong kernel | `strace` (không dùng được — nó **không** trace được process đã kẹt) | `cat /proc/<pid>/stack`, `/proc/<pid>/wchan` |
| Bản đồ bộ nhớ / rò mmap | `pmap` | `cat /proc/<pid>/maps` |
| IRQ nào bắn, bắn bao nhiêu | — | `cat /proc/interrupts` (đọc 2 lần, lấy hiệu) |
| Bật GPIO, đọc nhiệt độ CPU | — | **`/sys`** |

**Bẫy:** (1) tưởng `/proc` đọc từ đĩa nên "chậm, tránh dùng" — sai, nó là hàm kernel, rẻ; (2) dùng `strace` để soi process đang `D` — vô ích, `ptrace` cần process **nhận được signal**, mà `D` thì không; đúng chỗ phải xem `/proc/<pid>/stack`; (3) coi `/proc/<pid>/fd/` là bản sao — nó là **symlink sống**, đếm được cả socket và pipe (dùng để bắt **rò fd**).

**Chốt:** *"Thiết bị nhúng không có công cụ gì thì vẫn còn `/proc` và `/sys` — kernel tự phơi trạng thái ra dưới dạng file, và `ps`/`top` cũng chỉ đang đọc chỗ đó."*

> 🎤 Viết lại 2026-08-13 sau khi ứng viên được **0 điểm** (trắng cả 2 lần hỏi) — bản cũ là đoạn khẳng định 588 ký tự, không dựng lại được lập luận. Link nguồn cũ trỏ `→ file-io` là **sai**: file đó không có mục nào về hai filesystem này.
</details>

#### LNX-024 · 🟠 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Namespaces và cgroups là gì? Vì sao là nền tảng của container?**
<details><summary>Đáp án</summary>

**Hai cơ chế độc lập, giải quyết hai vấn đề khác nhau** — trộn lẫn chúng là lỗi phổ biến nhất:

| | **Namespaces** | **cgroups** |
|---|---|---|
| Câu hỏi nó trả lời | *"Process này **THẤY** được gì?"* | *"Process này được **DÙNG** bao nhiêu?"* |
| Bản chất | **Cô lập tầm nhìn** | **Giới hạn & hạch toán tài nguyên** |
| Các loại | pid · mount · net · uts · ipc · user | memory · cpu · io · pids |
| Thiếu nó thì | App lạ thấy/giết được process của bạn, đụng cổng, đụng `/tmp` | App lạ ăn hết RAM/CPU, kéo cả máy chết |

### Tình huống điển hình — vì sao cần CẢ HAI

Thiết bị chạy daemon của bạn + app đối tác. App đối tác rò bộ nhớ ⇒ RAM cạn ⇒ **OOM killer giết nhầm daemon của bạn** (nó chọn nạn nhân theo điểm số, không theo "ai có lỗi").

| Triệu chứng | Cơ chế xử lý | Hiệu quả |
|---|---|---|
| Rò RAM giết nhầm process khác | **cgroup memory limit** | OOM trở thành **cục bộ trong nhóm đó** — kernel giết đúng thủ phạm |
| Ghi log đầy `/tmp` | **mount namespace** | `/tmp` riêng, đầy cũng chỉ đầy của nó |
| Mở trùng cổng TCP | **net namespace** | Stack mạng riêng hoàn toàn |
| `kill` nhầm process của bạn | **pid namespace** | Không nhìn thấy thì không giết được |
| Fork bomb | **cgroup `pids`** | Trần số task |

### Container là gì, phát biểu chính xác

> **Container = namespaces (cô lập) + cgroups (giới hạn) + rootfs riêng.** Vẫn **chung kernel host** — không phải máy ảo, không có hypervisor, không boot kernel riêng.

Đó là lý do container **khởi động trong mili-giây** (chỉ là process bình thường có tầm nhìn bị cắt) nhưng **không cô lập bằng VM** (thủng kernel là thủng tất cả).

### 🎯 Trên embedded: bạn gặp nó qua systemd, không phải Docker

Đây là dạng gặp thật nhất trên thiết bị — cùng hai cơ chế, gói trong unit file:

```ini
[Service]
MemoryMax=64M                      # cgroup memory -> OOM cuc bo
CPUQuota=20%                       # cgroup cpu
TasksMax=32                        # cgroup pids   -> chong fork bomb
PrivateTmp=yes                     # mount namespace -> /tmp rieng
ProtectSystem=strict               # mount namespace -> rootfs read-only
RestrictAddressFamilies=AF_UNIX    # han che mang
```

**Bẫy:** (1) tưởng namespace giới hạn được tài nguyên — **không**, cô lập tầm nhìn không ngăn app ăn hết RAM; (2) tưởng cgroup cô lập được — **không**, cùng cgroup vẫn thấy và giết nhau được; (3) tưởng container an toàn như VM — chung kernel, một lỗ hổng kernel là chung số phận; (4) đặt `MemoryMax` rồi tưởng xong — process bị OOM trong cgroup vẫn **chết**, phải kèm `Restart=on-failure` thì dịch vụ mới tự hồi.

**Chốt:** *"Namespace trả lời 'thấy được gì', cgroup trả lời 'dùng được bao nhiêu'. Cần cả hai mới cô lập được một app không tin cậy — và trên embedded thì systemd unit file là cách dùng chúng, không phải Docker."*
</details>

#### LNX-040 · 🟠 · concept · ⭐ · 🎤 2026-08-15 · [→ tcp-ip](../../../13-networking/tcp-ip.md)
**Server đã `SIG_IGN` SIGPIPE và kiểm `EPIPE` đầy đủ. Một client BỊ RÚT DÂY MẠNG — không `close()`, không FIN, không RST. Server vẫn `write()`. Bao lâu thì nó biết?**

<details><summary>Đáp án</summary>

**① `write()` đầu tiên: THÀNH CÔNG**, trả về đủ số byte.

`EPIPE` là câu trả lời sai — và sai vì một lý do đáng nhớ: **`EPIPE` cần một RST bay về**, mà ở đây **không có gì bay về cả**. Máy đã biến mất.

> `write()` chỉ có nghĩa *"đã chép vào buffer gửi của kernel"*, **không bao giờ** có nghĩa *"bên kia đã nhận"*.

Ghi tiếp cho tới khi buffer gửi đầy ⇒ lúc đó mới `EAGAIN` (non-blocking) hoặc chặn (blocking).

**② Ai phát hiện, sau bao lâu:** kernel **retransmit theo thang tăng gấp đôi** (~200 ms, 400 ms, 800 ms, 1,6 s…) tới hết số lần cho phép, rồi `write()`/`read()` trả **`ETIMEDOUT`**. Với mặc định Linux: **khoảng 15–20 phút**.

| Ca | Server biết sau | Vì sao |
|---|---|---|
| Client `close()` sạch | **tức thì** | FIN → `read()` trả 0 |
| Client crash (OS còn sống) | **tức thì** | OS gửi RST → `EPIPE`/`ECONNRESET` |
| **Rút dây / mất sóng / máy chết** | **~15 phút** | Không ai gửi gì cả — chỉ còn timeout |
| Client treo nhưng TCP sống | **không bao giờ** | TCP hoàn toàn khoẻ mạnh |

**Hàng cuối là hàng quan trọng nhất:** TCP không biết gì về việc app bên kia còn xử lý được hay không.

### ③ Hai cách rút ngắn — hai tầng khác nhau

| Tầng | Cách | Được | Mất |
|---|---|---|---|
| **Kernel/socket** | `SO_KEEPALIVE` + `TCP_KEEPIDLE` / `KEEPINTVL` / `KEEPCNT` (vd 30 s + 5 s × 3 ⇒ ~45 s) | Không phải sửa protocol, áp cho mọi kết nối | Tham số Linux per-socket; **chỉ phát hiện đường chết**, không phát hiện app treo |
| **Application** | **Heartbeat**: hai bên ping định kỳ, thiếu N nhịp thì tự đóng | Phát hiện **cả app treo mà TCP vẫn sống**; di động mọi OS; đo được cả độ trễ | Phải sửa protocol; tốn băng thông (đáng lưu ý trên 4G tính tiền theo byte) |

**Chọn thế nào:** thiết bị nhiều, mạng di động, cần biết *app* còn sống ⇒ **heartbeat**. Chỉ cần dọn kết nối chết để giải phóng tài nguyên ⇒ **keepalive** là đủ và rẻ hơn.

**Bẫy:** (1) đặt keepalive quá ngắn cho hàng nghìn kết nối trên mạng tính tiền theo byte ⇒ hoá đơn + hao pin; (2) tưởng `write()` thành công là bên kia đã nhận — sai cả với mạng khoẻ; (3) quên rằng **nửa kết nối vẫn ghi được**: peer `close()` rồi, `write()` đầu tiên vẫn OK, phải tới lần sau mới `EPIPE`.

**Chốt:** *"TCP không có tín hiệu 'đối tác còn sống'. **Im lặng và khoẻ mạnh trông giống hệt nhau.** Muốn biết bên kia còn sống thì phải chủ động hỏi."*
</details>

---

⬅️ [Bank index](README.md)
