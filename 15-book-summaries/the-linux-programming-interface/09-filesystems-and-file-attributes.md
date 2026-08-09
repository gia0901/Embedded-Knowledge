# TLPI — Cụm 09: File Systems, Attributes, Directories & Links (ch. 8, 14–19)

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 8 (tr. 153–166) và 14–19 (tr. 251–386).
> **Định vị:** phần tra cứu. Nhưng **§1 (i-node) là kiến thức nền thật sự đáng nắm** — nó giải thích hard link, `unlink` không xoá ngay, `rename` nguyên tử, và vì sao `df` với `du` cho số khác nhau.

---

## 1. i-node — trung tâm của filesystem (ch. 14, tr. 251–278) 🎯

### 1.1. Bố cục một filesystem

```
   Ổ đĩa
   ┌──────────────┬──────────────┬──────────────┐
   │ Partition 1  │ Partition 2  │ Partition 3  │
   └──────┬───────┴──────────────┴──────────────┘
          │  mỗi partition chứa MỘT filesystem
          ▼
   ┌───────────┬────────────┬──────────────┬─────────────────────────┐
   │ Boot block│ Superblock │ I-NODE TABLE │      DATA BLOCKS        │
   └───────────┴─────┬──────┴──────┬───────┴────────────┬────────────┘
                     │             │                    │
        thông tin về │      một i-node cho MỖI     nội dung file
        filesystem:  │      file trong fs           thật sự nằm đây
        kích thước,  │      (đánh số từ 1)
        số i-node,   │
        vị trí bảng  │
```

*(vẽ lại theo Figure 14-1, tr. 255)*

### 1.2. i-node chứa gì — và KHÔNG chứa gì

| i-node **CÓ** | i-node **KHÔNG có** |
|---|---|
| Loại file (thường/thư mục/symlink/device…) | **TÊN FILE** ⚠️ |
| Quyền (mode), UID, GID chủ sở hữu | |
| **Số link cứng** (link count) | |
| Kích thước, số block chiếm | |
| Ba mốc thời gian: `atime`, `mtime`, `ctime` | |
| **Con trỏ tới các data block** | |

> ⚠️ **i-node KHÔNG chứa tên file.** Tên nằm trong **thư mục** — mà thư mục thực chất là một **bảng ánh xạ (tên → số i-node)**. Đây là chi tiết đơn lẻ giải thích nhiều hành vi lạ nhất của Unix filesystem, xem §2.

**Ba mốc thời gian dễ nhầm:**

| | Cập nhật khi | Đổi được bằng tay? |
|---|---|---|
| `atime` | **Đọc** nội dung file | `utime()` |
| `mtime` | **Sửa nội dung** file | `utime()` |
| `ctime` | Sửa **i-node** (quyền, chủ, link count) **hoặc** nội dung | **Không** — kernel giữ |

🆕 `ctime` **không** phải "creation time" — đây là hiểu nhầm rất phổ biến. Nó là *change time* của i-node. Chính vì không sửa được bằng tay nên nó có giá trị trong **điều tra pháp chứng**.

---

## 2. Thư mục và link (ch. 18, tr. 339–374) 🎯

### 2.1. Thư mục là một bảng (tên → i-node)

```
   Thư mục /etc                          Bảng i-node
   ┌──────────────┬─────────┐            ┌───────┬────────────────────┐
   │ tên          │ i-node  │            │ i-no  │ metadata + con trỏ │
   ├──────────────┼─────────┤            ├───────┼────────────────────┤
   │ .            │  2003   │            │  2    │ (root — LUÔN là 2) │
   │ ..           │    2    │            │  ...  │                    │
   │ passwd       │ ───────────────────► │ 4132  │ link count = 2  ───┼──► data
   │ group        │  4139   │       ┌──► │       │ mode, uid, size... │    blocks
   └──────────────┴─────────┘       │    └───────┴────────────────────┘
                                    │
   Thư mục /home/gia                │
   ┌──────────────┬─────────┐       │
   │ mypasswd     │ ────────────────┘   ← HARD LINK: TÊN KHÁC, CÙNG i-node
   └──────────────┴─────────┘              → link count của i-node 4132 = 2
```

*(vẽ lại theo Figure 18-1, tr. 341)*

> Chi tiết Kerrisk nêu (tr. 341): **thư mục gốc luôn nằm ở i-node số 2** — kernel dựa vào đó để bắt đầu duyệt đường dẫn.

### 2.2. Hard link vs symbolic link

| | **Hard link** (`ln`) | **Symbolic link** (`ln -s`) |
|---|---|---|
| Bản chất | **Tên thứ hai** cho cùng i-node | **File riêng**, nội dung là một **đường dẫn** |
| i-node | **Chung** | **Riêng** |
| Qua filesystem khác | ❌ (số i-node chỉ có nghĩa trong một fs) | ✅ |
| Trỏ tới thư mục | ❌ (cấm — sẽ tạo chu trình) | ✅ |
| Xoá đích | File **vẫn còn** (link count giảm) | Thành **dangling link** (trỏ vào hư không) |
| Tạo trước đích | ❌ | ✅ (link treo cho tới khi đích xuất hiện) |

### 2.3. `unlink()` — vì sao xoá file mà dung lượng không giảm 🎯

**`unlink()` không "xoá file"** — nó **gỡ một tên khỏi thư mục** và giảm **link count** của i-node đi 1. Dữ liệu chỉ thật sự được giải phóng khi **CẢ HAI** điều kiện đúng:

```
   ① link count == 0        (không còn tên nào trỏ tới)
        VÀ
   ② không process nào còn MỞ file đó   (không còn file description nào)
```

```sh
# Bug vận hành kinh điển trên server:
rm /var/log/huge.log        # ✅ ls không còn thấy; df KHÔNG giảm dung lượng!
                            # vì daemon vẫn đang MỞ file đó → điều kiện ② chưa thoả
lsof | grep deleted         # ← thấy thủ phạm
systemctl restart mydaemon  # daemon đóng fd → giờ dung lượng mới được trả về
```

> 🆕 Đây là lý do `df` và `du` cho số khác nhau, và là câu hỏi vận hành rất hay gặp. Cùng cơ chế cũng cho một **mẹo hữu ích**: tạo file tạm rồi `unlink()` **ngay** trong khi vẫn giữ fd — file **không có tên nào** nên không ai thấy hay xoá được, và **tự biến mất** khi process kết thúc, kể cả khi crash.

### 2.4. `rename()` nguyên tử — nền của mọi cập nhật file an toàn

`rename()` là **nguyên tử**: tại mọi thời điểm, đường dẫn đích trỏ tới **hoặc file cũ, hoặc file mới**, không bao giờ có trạng thái nửa vời.

```c
/* Mẫu chuẩn cập nhật file cấu hình mà không bao giờ để lại file hỏng */
int fd = open("config.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
write(fd, data, len);
fsync(fd);                       // ① ép DỮ LIỆU xuống đĩa TRƯỚC (cụm 01 §3.1)
close(fd);
rename("config.tmp", "config");  // ② đổi tên nguyên tử — không có khoảnh khắc nào
                                 //    "config" ở trạng thái viết dở
/* ③ fsync THƯ MỤC để bản thân thao tác rename bền vững qua mất điện */
int dfd = open(".", O_RDONLY);  fsync(dfd);  close(dfd);
```

⚠️ **Thiếu bước ① là bug thật:** `rename` xong nhưng dữ liệu còn trong buffer cache; mất điện → tên mới trỏ vào file **rỗng hoặc viết dở**. Nhiều ứng dụng từng dính lỗi này.

---

## 3. Quyền và thuộc tính file (ch. 15–17, tr. 279–338)

### 3.1. Mô hình quyền cổ điển

```
   -rwxr-xr--   1 gia  dev  4096 ...
    │└┬┘└┬┘└┬┘
    │ │  │  └── other :  r--
    │ │  └───── group :  r-x
    │ └──────── owner :  rwx
    └────────── loại file
```

**Với thư mục, ba bit có nghĩa KHÁC:**

| Bit | Trên file thường | **Trên thư mục** |
|---|---|---|
| `r` | Đọc nội dung | **Liệt kê tên** trong đó (`ls`) |
| `w` | Ghi nội dung | **Tạo/xoá/đổi tên** file trong đó |
| `x` | Thực thi | **Đi xuyên qua** (truy cập file bên trong nếu biết tên) |

> ⚠️ **Hệ quả rất hay bị hỏi:** xoá một file phụ thuộc quyền **`w` của THƯ MỤC chứa nó**, **không** phụ thuộc quyền của chính file. Nên bạn xoá được file `r--r--r--` của người khác nếu có quyền ghi thư mục. Đó là lý do `/tmp` có **sticky bit** (`drwxrwxrwt`) — chỉ chủ sở hữu file mới xoá được file của mình.
>
> Ngược lại: thư mục có `x` nhưng **không có `r`** → bạn **mở được** file bên trong nếu biết chính xác tên, nhưng **không liệt kê được**. Đây là cách thường dùng để làm thư mục "chỉ ai biết đường dẫn mới vào".

### 3.2. Set-UID / Set-GID / Sticky

| Bit | Trên file thực thi | Trên thư mục |
|---|---|---|
| **set-UID** (`u+s`) | Chạy với **UID hiệu lực = chủ sở hữu file** (vd `passwd` chạy như root) | — |
| **set-GID** (`g+s`) | Chạy với GID = nhóm của file | File tạo trong đó **kế thừa nhóm** của thư mục |
| **sticky** (`+t`) | (lỗi thời) | **Chỉ chủ file mới xoá/đổi tên được** — dùng cho `/tmp` |

**`umask`** quyết định quyền nào bị **loại bỏ** khi tạo file: quyền cuối = `mode & ~umask`. Mặc định `022` → file tạo với `0666` thành `0644`.

### 3.3. Ngoài mô hình cổ điển (ch. 16–17)

- **Extended attributes (EA)** — cặp tên/giá trị gắn kèm file (`getfattr`/`setfattr`). Nền cho ACL và **capabilities gắn vào file**.
- **ACL** — quyền chi tiết theo **từng user/nhóm cụ thể**, vượt qua giới hạn ba nhóm owner/group/other (`getfacl`/`setfacl`). ⚠️ Kerrisk lưu ý ACL phức tạp và **hỗ trợ không đồng đều** giữa các công cụ — cân nhắc trước khi phụ thuộc vào nó.

---

## 4. Theo dõi sự kiện file — `inotify` (ch. 19, tr. 375–386)

```c
int ifd = inotify_init();                       // ← trả về một FILE DESCRIPTOR
int wd  = inotify_add_watch(ifd, "/etc/myapp", IN_MODIFY | IN_CREATE | IN_DELETE);

char buf[BUF_LEN];
ssize_t n = read(ifd, buf, BUF_LEN);            // đọc như file thường
for (char *p = buf; p < buf + n; ) {
    struct inotify_event *ev = (struct inotify_event *) p;
    if (ev->len > 0) printf("sự kiện trên %s\n", ev->name);
    p += sizeof(struct inotify_event) + ev->len;  // ⚠️ độ dài THAY ĐỔI theo từng event
}
```

**Vì sao thiết kế "trả về fd" là điểm mạnh:** `ifd` đưa thẳng vào **`epoll`** cùng mọi fd khác ([cụm 05](05-alternative-io-models.md)) — không cần polling thư mục, không cần thread riêng. Cùng triết lý với `signalfd`, `timerfd`, `eventfd`.

⚠️ **Ba hạn chế phải biết:**
1. **Không đệ quy** — theo dõi thư mục con phải tự thêm watch cho từng cái, và tự thêm khi có thư mục mới được tạo.
2. **Hàng đợi có thể tràn** → sự kiện `IN_Q_OVERFLOW`, và bạn **mất sự kiện**. Phải quét lại toàn bộ để đồng bộ.
3. `read()` từ fd inotify nằm trong nhóm **không bao giờ restart** dù có `SA_RESTART` — phải tự xử `EINTR` ([cụm 03 §3.3](03-signals-and-timers.md)).

---

## 5. Ít quan trọng — 1 dòng + tham chiếu

- **Ch. 8 — Users and Groups** (tr. 153): `/etc/passwd`, `/etc/shadow`, `/etc/group`; `getpwnam()`/`getgrnam()` (⚠️ **không** thread-safe — dùng bản `_r`); `crypt()` cho mật khẩu.
- **§14.5–14.6 — Virtual File System (VFS), journaling** (tr. 261): VFS là lớp trừu tượng cho phép cùng syscall chạy trên ext4/XFS/NFS; journaling ghi ý định trước khi ghi thật → **fsck nhanh sau mất điện**.
- **§14.8 — `mount()`/`umount()`, `/proc/mounts`** (tr. 267): cờ `MS_RDONLY`, `MS_NOEXEC`, `MS_NOSUID` — **rất đáng biết cho embedded**: rootfs mount read-only để chống hỏng flash khi mất điện.
- **§14.10 — Virtual filesystem: `tmpfs`** (tr. 275): nằm trong RAM; `/dev/shm` chính là tmpfs.
- **§15.1 — `stat()`, `lstat()`, `fstat()`** (tr. 279): ⚠️ `lstat()` **không** đi theo symlink — đúng cái bạn cần khi muốn kiểm tra chính symlink đó.
- **§15.4 — Macro `S_ISREG()`, `S_ISDIR()`, `S_ISLNK()`** (tr. 288).
- **§15.5 — `chmod()`, `chown()`, `utime()`** (tr. 294).
- **§15.7 — Cờ i-node (`chattr`/`lsattr`)** (tr. 305): `+i` immutable, `+a` append-only — hữu ích cho file log không được sửa.
- **§18.2–18.4 — `link()`, `unlink()`, `rename()`, `symlink()`, `readlink()`** (tr. 342).
- **§18.8–18.9 — `opendir()`, `readdir()`, `nftw()`** (tr. 356): duyệt cây thư mục; `nftw()` gọi callback cho từng file.
- **§18.10 — `chdir()`, `getcwd()`, `chroot()`** (tr. 366): ⚠️ **`chroot()` KHÔNG phải cơ chế bảo mật** — process còn quyền root thoát ra được. Cách ly thật cần **namespace/container**.
- **§18.11 — Họ `*at()`** (tr. 371): `openat()`, `fstatat()`, `unlinkat()` — nhận **fd thư mục** thay vì đường dẫn tuyệt đối. Giải hai bài toán: **race khi đường dẫn bị đổi giữa chừng** (TOCTOU), và thư mục làm việc riêng cho từng thread.

---

## 6. Góc interview

<details><summary>1) Hard link và symbolic link khác nhau thế nào?</summary>

Gốc rễ nằm ở chỗ: **i-node chứa mọi metadata của file NHƯNG KHÔNG chứa tên**. Tên nằm trong **thư mục**, vốn chỉ là một bảng ánh xạ **(tên → số i-node)**.

- **Hard link** = **thêm một mục (tên → i-node)** trỏ vào **cùng i-node**. Hai tên hoàn toàn **ngang hàng** — không có cái nào là "bản gốc". `link count` của i-node tăng lên.
- **Symbolic link** = một **file riêng, có i-node riêng**, nội dung là **một chuỗi đường dẫn**. Kernel đi theo chuỗi đó khi truy cập.

| | Hard link | Symlink |
|---|---|---|
| Qua filesystem khác | ❌ (số i-node chỉ có nghĩa trong một fs) | ✅ |
| Trỏ tới thư mục | ❌ (cấm — tạo chu trình, phá việc duyệt cây) | ✅ |
| Xoá đích | File **vẫn còn** (link count giảm) | Thành **dangling link** |
| Tạo trước khi đích tồn tại | ❌ | ✅ |
| Kích thước | Không tốn thêm | Tốn một i-node + chuỗi đường dẫn |

**`lstat()` vs `stat()`:** `stat()` **đi theo** symlink (trả metadata của đích); `lstat()` **không** — trả metadata của chính symlink. Đây là hàm bạn cần khi muốn kiểm tra "đây có phải symlink không".
</details>

<details><summary>2) Vì sao <code>rm</code> file lớn mà <code>df</code> không giảm dung lượng?</summary>

Vì **`unlink()` không xoá file** — nó chỉ **gỡ một tên khỏi thư mục** và giảm **link count** của i-node đi 1. Dữ liệu chỉ thật sự được giải phóng khi **cả hai** điều kiện đúng:

1. **link count == 0** — không còn tên nào trỏ tới, **VÀ**
2. **không process nào còn MỞ file** — không còn file description nào tham chiếu.

Kịch bản kinh điển trên server: daemon đang mở `/var/log/huge.log` và ghi liên tục; bạn `rm` nó. Tên biến mất (`ls` không thấy nữa), link count về 0, **nhưng điều kiện ② chưa thoả** → **các block đĩa vẫn bị chiếm**.

```sh
lsof | grep deleted          # ← tìm ra process đang giữ file đã xoá
# Chữa: restart service, hoặc bảo nó đóng/mở lại file (thường bằng SIGHUP)
# Chữa nhanh không cần restart: truncate file qua /proc
: > /proc/<pid>/fd/<n>
```

Đây cũng là lý do **`df` và `du` cho số khác nhau**: `du` cộng theo tên file còn tồn tại, `df` hỏi filesystem về block thật sự đang bị chiếm.

🆕 Cùng cơ chế cho một **mẹo hữu ích**: tạo file tạm rồi `unlink()` **ngay lập tức** trong khi vẫn giữ fd. File **không còn tên nào** nên không ai thấy hay xoá nhầm được, và nó **tự biến mất khi process kết thúc** — kể cả khi crash hoặc bị `SIGKILL`. Đây là cách làm file tạm an toàn nhất.
</details>

<details><summary>3) Làm sao cập nhật một file cấu hình mà không bao giờ để lại file hỏng?</summary>

Dùng mẫu **ghi file tạm → `fsync` → `rename`**, dựa trên tính **nguyên tử của `rename()`**:

```c
int fd = open("config.tmp", O_WRONLY|O_CREAT|O_TRUNC, 0644);
write(fd, data, len);
fsync(fd);                        /* ① ép DỮ LIỆU xuống đĩa TRƯỚC */
close(fd);
rename("config.tmp", "config");   /* ② nguyên tử: "config" luôn trỏ tới
                                        HOẶC file cũ HOẶC file mới hoàn chỉnh */
int dfd = open(".", O_RDONLY);    /* ③ fsync THƯ MỤC để bản thân thao tác
fsync(dfd); close(dfd);                 rename bền vững qua mất điện */
```

**Vì sao mỗi bước cần thiết:**
- **`rename()` nguyên tử** — không bao giờ có khoảnh khắc nào `config` ở trạng thái viết dở. Ghi đè trực tiếp lên `config` thì mất điện giữa chừng là mất luôn cả bản cũ lẫn bản mới.
- **⚠️ Thiếu bước ① là bug thật:** `rename` chỉ sửa metadata thư mục. Nếu dữ liệu còn nằm trong buffer cache, mất điện → tên mới trỏ vào file **rỗng hoặc viết dở**. Nhiều ứng dụng lớn từng dính đúng lỗi này.
- **Bước ③** vì bản thân thao tác `rename` cũng nằm trong buffer cache của thư mục.

Trên thiết bị nhúng dùng eMMC/SD, đây là mẫu **bắt buộc** cho mọi file cấu hình — mất điện là chuyện thường ngày. Kèm theo nên mount rootfs **read-only** và chỉ để phân vùng dữ liệu ghi được.
</details>

<details><summary>4) Quyền trên thư mục có ý nghĩa gì khác so với trên file?</summary>

Ba bit `rwx` mang nghĩa **hoàn toàn khác**:

| Bit | Trên file | **Trên thư mục** |
|---|---|---|
| `r` | Đọc nội dung | **Liệt kê tên** bên trong (`ls`) |
| `w` | Ghi nội dung | **Tạo / xoá / đổi tên** file bên trong |
| `x` | Thực thi | **Đi xuyên qua** — truy cập file bên trong **nếu biết tên** |

**Hệ quả quan trọng nhất:** **xoá một file phụ thuộc quyền `w` của THƯ MỤC chứa nó**, chứ **không** phụ thuộc quyền của chính file. Hợp lý khi nhớ rằng "xoá file" thực chất là **gỡ một mục khỏi bảng của thư mục** — tức là sửa **thư mục**, không phải sửa file.

⇒ Bạn xoá được file `r--r--r--` của người khác nếu có quyền ghi thư mục chứa nó. Đó chính là lý do **`/tmp` có sticky bit** (`drwxrwxrwt`): ai cũng tạo file được, nhưng **chỉ chủ file mới xoá được file của mình**.

**Tổ hợp `x` không `r`** (`--x`) rất hữu dụng: mở được file bên trong **nếu biết chính xác tên**, nhưng **không liệt kê được**. Dùng làm thư mục "chỉ ai biết đường dẫn mới vào".

Ngược lại `r` không `x` gần như vô dụng: `ls` thấy tên nhưng `stat` từng file lại thất bại — nên `ls -l` hiện lỗi cho mọi mục.

**`umask`** quyết định quyền bị **loại** khi tạo file: quyền cuối = `mode & ~umask`. Mặc định `022` biến `0666` thành `0644`.
</details>

---

## 7. Đọc thêm (tùy chọn)

- File I/O ở tầng syscall: [cụm 01](01-concepts-and-file-io.md) — ba bảng của kernel, `fsync` vs `fflush`.
- Góc kernel của VFS và page cache: [LKD cụm VFS](../lkd/05-vfs-block-pagecache.md).
- Góc lý thuyết: [OSTEP persistence](../ostep/persistence.md) — i-node, journaling, crash consistency.
- Filesystem cho embedded (mount read-only, chống mòn flash): [MELP storage & update](../melp/storage-update.md).

---
⬅️ [Cụm 08 — mmap & shared libraries](08-memory-mappings-and-shared-libraries.md) · [README cụm TLPI](README.md)
