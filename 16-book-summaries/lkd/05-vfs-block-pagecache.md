# Ch. 13, 14 & 16 — VFS, Block I/O Layer & Page Cache (tr. 261–288, 289–304, 323–336)

> Thuộc [LKD](README.md) · **[⏮ 04 Memory](04-memory.md)** · **[06 Devices/Modules/Debug → ⏭](06-modules-debug.md)**
> Nguồn: **đọc trực tiếp PDF** (Linux Kernel Development, Robert Love, 3rd ed. 2010), **trang sách = trang PDF − 27**.
> Ký hiệu: không đánh dấu = nội dung sách · **🆕 = bổ sung/liên hệ ngoài sách** · **⚠️ = cần cẩn trọng / sách lỗi thời** · **🎯 = câu hỏi phỏng vấn kinh điển** · trích dẫn kèm `(tr. X)`.
> Ba chương này lần theo **đường đi của một byte từ `read()`/`write()` xuống đĩa và ngược lại**: **ch. 13 (VFS)** trừu tượng hóa "file là gì" thành **bốn object** để mọi filesystem trông như một; **ch. 14 (Block I/O)** đưa yêu cầu xuống thiết bị khối, xếp lịch giảm seek; **ch. 16 (Page Cache)** chèn một **cache RAM** giữa hai tầng để né đĩa. Ví dụ xương sống: chuỗi **`read(fd, ...)` → `file` → `dentry` → `inode` → page cache → bio → I/O scheduler → đĩa**.
> 🆕 Nền lý thuyết FS (inode, journaling, đường read/write) đã có ở [ostep/persistence.md](../ostep/persistence.md) — bạn **không cần** đọc nó để theo file này; ở đây ta xem **kernel trừu tượng hóa** ra struct nào.

---

## Cụm 1 — VFS: bốn object (ch. 13, tr. 261–288)

### 1.1 Vì sao có VFS — OOP kiểu C (tr. 262–263)

**Bức tranh:** cùng một `write(fd, buf, len)` chạy được trên ext4/NTFS/NFS/procfs — nhờ **VFS (Virtual Filesystem)**, một **lớp trừu tượng** quanh interface filesystem cấp thấp. VFS định nghĩa một **common file model** (mô hình file chung, thiên về Unix); mỗi filesystem "uốn" khái niệm của mình ("mở file với tôi là thế này") cho khớp kỳ vọng VFS. Kết quả (tr. 262): *"nothing in the kernel needs to understand the underlying details of the filesystems, except the filesystems themselves."*

Chuỗi của `write()` (Figure 13.2, tr. 263):
```
write(fd,buf,len)  →  sys_write()  →  filesystem's write method  →  physical media
   user-space          VFS (frontend)     filesystem (backend)
```
VFS gọi **function pointer** của filesystem cụ thể → đây là **OOP kiểu C**: struct chứa cả dữ liệu lẫn con trỏ hàm thao tác trên dữ liệu đó. Sách nhấn (chú thích tr. 265): *"The VFS is a good example of how to do clean and efficient OOP in C, which is a language that lacks any OOP constructs."*

### 1.2 Bốn khái niệm Unix & bốn object VFS (tr. 263–265)

Unix có **bốn trừu tượng filesystem** (tr. 263): **file** (chuỗi byte có thứ tự), **directory entry** (thành phần đường dẫn — thư mục *là* file liệt kê file bên trong), **inode** (metadata của file — quyền, size, owner, time — **tách rời** khỏi dữ liệu), **mount point**. 🆕 Unix có **một namespace hợp nhất** (mount vào một cây duy nhất) — khác DOS/Windows chia theo ổ `C:` (rò chi tiết phần cứng vào trừu tượng file). **superblock** = thông tin điều khiển của cả filesystem.

VFS hiện thực bốn khái niệm này thành **bốn object chính** (tr. 265):

| Object | Đại diện | Ops table | Ghi chú |
|---|---|---|---|
| **superblock** | Một **filesystem đã mount** | `super_operations` (`write_inode`, `sync_fs`) | Ứng với superblock on-disk (sector đặc biệt); fs ảo (sysfs) *chế* trong RAM |
| **inode** | Một **file/dir** (metadata) | `inode_operations` (`create`, `lookup`, `mkdir`) | Xây trong RAM khi file được truy cập |
| **dentry** | **Một thành phần đường dẫn** (`/`, `bin`, `vi` — mỗi cái một dentry) | `dentry_operations` (`d_compare`, `d_delete`) | **KHÔNG có bản on-disk** |
| **file** | Một **file đang mở bởi một process** | `file_operations` (`read`, `write`, `mmap`) | Ứng với fd; nhiều file object → 1 dentry → 1 inode |

⚠️ **Không có "directory object"** (tr. 265): thư mục *là* một loại file; `dentry` (thành phần path) **khác** directory. OOP kiểu C phải **truyền object cha vào method** (tr. 268): `sb->s_op->write_super(sb)` — C++ chỉ cần `sb.write_super()`, nhưng C không tự lấy được `this` nên phải truyền `sb`.

### 1.3 Superblock & inode (tr. 266–274)

**Superblock (tr. 266):** `struct super_block` — mô tả cả filesystem; trường quan trọng nhất là **`s_op`** (bảng thao tác). Filesystem trên đĩa đọc superblock từ sector đặc biệt; fs ảo (sysfs) sinh trong RAM. `super_operations` (tr. 267): `alloc_inode`, `write_inode` (ghi inode xuống đĩa), `dirty_inode` (journaling ext3/4 dùng cập nhật journal), `sync_fs`... — **chạy ở process context, được block** (trừ `dirty_inode`).

**Inode (tr. 270):** `struct inode` — mọi thông tin kernel cần để thao tác một file/dir. Với Unix fs, đọc thẳng từ inode on-disk; fs không có inode phải **dựng struct inode trong RAM như thể có**.
```c
struct inode {
        unsigned long        i_ino;        // số inode
        atomic_t             i_count;      // ref count
        unsigned int         i_nlink;      // số hard link
        loff_t               i_size;       // kích thước file (byte)
        struct timespec      i_atime, i_mtime, i_ctime;   // access/modify/change time
        umode_t              i_mode;       // quyền
        struct inode_operations *i_op;     // bảng ops trên inode
        struct file_operations  *i_fop;    // bảng ops mặc định cho file mở từ inode này
        struct super_block   *i_sb;
        struct address_space *i_mapping;   // page cache của file này (cụm 3)
        union {
                struct pipe_inode_info *i_pipe;   // inode có thể là pipe...
                struct block_device    *i_bdev;   // ...hoặc block device...
                struct cdev            *i_cdev;   // ...hoặc char device (union — chỉ 1 lúc)
        };
        ...
};
```
🆕 `union i_pipe/i_bdev/i_cdev` cho thấy inode cũng biểu diễn **special file** (device node, pipe) — nối [06](06-modules-debug.md). `inode_operations` (tr. 271): `create` (từ `open`/`creat`), **`lookup`** (tìm inode theo tên trong dir — trái tim của path walk), `link`/`unlink`/`symlink`/`mkdir`/`rename`/`truncate`/`permission`.

### 1.4 Dentry, dentry cache & negative dentry (tr. 275–278)

**Dentry (tr. 275):** phân giải path `/bin/vi` = walk từng cấp (`/` → `bin` → `vi`), mỗi cấp cần một `lookup` — **đắt, nặng thao tác chuỗi**. VFS dùng **dentry** (directory entry) cho mỗi thành phần path (kể cả file cuối); **dựng on-the-fly**, **không có bản on-disk** (nên không có cờ "dirty").

**Ba trạng thái dentry (tr. 276):**
- **used** — trỏ inode hợp lệ, `d_count` > 0, đang dùng → không vứt được;
- **unused** — trỏ inode hợp lệ, `d_count` = 0 → **cache lại** (khỏi dựng lại nếu cần), thu hồi được khi thiếu RAM;
- **negative** — **`d_inode` = NULL** (inode bị xóa hoặc path chưa từng đúng). ⚠️ **Negative dentry rất hữu ích** (tr. 276): ví dụ daemon liên tục mở một config **không tồn tại** → `open` trả `ENOENT`; cache "tên này KHÔNG có" giúp lần mở-fail sau **khỏi walk lại on-disk**.

**Dentry cache — dcache (tr. 277):** ba phần — list "used" gắn theo inode (`i_dentry` — một inode nhiều hard link → nhiều dentry), **LRU list** unused+negative (thu hồi từ đuôi), **hash table** (`d_lookup` phân giải path→dentry nhanh). 🆕 **dcache kéo icache theo** (tr. 277): dentry **giữ inode dương ref** → *"as long as the dentry is cached, the corresponding inodes are cached, too"*. Đây chính là phần "slab dentry/inode phình chiếm RAM" — **cache thu hồi được, không phải leak**. Hiệu quả nhờ **spatial + temporal locality** (chương hay mở lại cùng file, và mở nhiều file cùng thư mục).

### 1.5 File object & `file_operations` (tr. 279–284)

**File object (tr. 279):** `struct file` — biểu diễn **một file đang mở bởi một process** (góc nhìn của process). Tạo khi `open()`, hủy khi `close()`. **Nhiều process mở cùng file → nhiều file object** (mỗi cái một offset); tất cả trỏ về **cùng dentry → cùng inode**.
```c
struct file {
        struct path             f_path;   // chứa dentry
        struct file_operations *f_op;     // bảng ops
        atomic_t                f_count;  // ref count
        unsigned int            f_flags;  // cờ lúc open (O_APPEND...)
        mode_t                  f_mode;   // chế độ truy cập
        loff_t                  f_pos;    // OFFSET hiện tại (con trỏ file) ← riêng mỗi lần mở
        struct address_space   *f_mapping;// page cache (cụm 3)
        ...
};
```
`file_operations` (tr. 280) — chính là các **syscall Unix quen thuộc**: `llseek`, `read`, `write`, `poll`, **`unlocked_ioctl`/`compat_ioctl`**, `mmap`, `open`, `release`, `fsync`, `flock`... 🆕 **Ba ioctl (sidebar tr. 284):** `ioctl` cũ giữ **BKL** (chậm) → dùng **`unlocked_ioctl`** (không BKL, tự lo lock); **`compat_ioctl`** cho app 32-bit trên kernel 64-bit — driver mới thiết kế ioctl **kích thước tường minh** rồi trỏ cả hai vào cùng hàm. Đây là interface **cốt lõi của viết driver** ([06](06-modules-debug.md), [05/kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md)).

### 1.6 file_system_type, vfsmount & per-process (tr. 285–288)

- **`file_system_type` (tr. 285):** **một per loại filesystem** (ext4, UDF...); `get_sb`/`kill_sb` đọc/hủy superblock lúc mount.
- **`vfsmount` (tr. 285):** **một instance đã mount** (một mount point). Cờ mount đáng nhớ: `MNT_NOSUID` (cấm setuid), `MNT_NODEV` (cấm device file), `MNT_NOEXEC` (cấm chạy binary) — hữu ích cho thiết bị tháo rời không tin cậy.
- **Ba struct per-process (tr. 286–288)** nối VFS với process: **`files_struct`** (bảng fd — `fd_array` sẵn 64 slot, hơn thì cấp mảng mới), **`fs_struct`** (`root` + `pwd` = cwd), **`mnt_namespace`** (cây mount riêng). 🆕 `CLONE_FILES`/`CLONE_FS` → **chung** `files_struct`/`fs_struct` (thread); `CLONE_NEWNS` → namespace riêng — nối thẳng clone flags của [01](01-process-sched-syscalls.md). **Nền kernel của container** là namespace này.

### Insight đáng nhớ (cụm 1)

- **Chuỗi `fd → file → dentry → inode → dữ liệu`** là xương sống: `file` giữ **offset riêng** (mỗi lần open); `inode` là **file thật** (một cho mỗi file); `dentry` là **mắt xích tên** (cache path). Vẽ được chuỗi này là trả lời trọn "phân biệt file/dentry/inode".
- **Negative dentry (tr. 276)** dạy nguyên tắc cache tổng quát: **cache cả kết quả "không tồn tại"** vì lookup-fail cũng đắt. Và **dcache pin icache** giải thích "slab dentry/inode to" — cache, không phải leak.
- **OOP kiểu C** (`s_op`/`i_op`/`f_op` là bảng con trỏ hàm) lặp lại khắp kernel (VMA `vm_ops` [04], sau này kobject [06]) — nhận ra pattern là hiểu kiến trúc.

### ⚠️ Đã thay đổi so với sách

- **BKL đã xóa** → `ioctl` cũ (giữ BKL) không còn; driver dùng **`unlocked_ioctl`** (+ `compat_ioctl`). `f_dentry` đổi thành `f_path.dentry`.
- Path walk hiện đại có **RCU-walk** (lockless, nhanh) rồi fallback **ref-walk** — dcache dùng RCU nặng. `mnt_namespace` + các namespace khác (PID/net/user...) là **nền của container** (Docker/LXC), phần lớn ra đời sau bản in.
- `super_operations`/`inode_operations` đã đổi chữ ký nhiều (vd `->lookup` thêm cờ, iversion...) — khung 4-object vẫn nguyên.

### Góc interview (cụm 1)

**Câu 1 (🎯):** Phân biệt **file object, dentry, inode**. Hai process cùng mở một file thì cái gì **chung**, cái gì **riêng**?

<details><summary>Đáp án</summary>

- **inode (tr. 270)** = bản thân file (metadata + nơi trỏ dữ liệu) — **một** cho mỗi file bất kể ai mở. **dentry (tr. 275)** = một mắt xích tên trên path (cache phân giải path; nhiều hard link → nhiều dentry cùng trỏ một inode). **file object (tr. 279)** = "phiên mở": offset (`f_pos`), flags (`O_APPEND`...), mode — **mỗi lần `open()` một cái**.
- **Hai process mở cùng file:** **chung inode** (và thường chung dentry), **riêng file object** → **offset độc lập** (ai đọc nấy tiến).
- **Ngoại lệ phải nêu — fork/dup:** fd của cha con (sau `fork`) hoặc `dup` trỏ **cùng file object** → **CHUNG offset** → nguồn của hành vi ghi nối tiếp nhau sau fork, và là cơ chế **shell redirect** hoạt động ([ostep/virtualization-cpu.md](../ostep/virtualization-cpu.md)).
- **Chuỗi đầy đủ:** `fd —(bảng fd per-process, files_struct)→ file → dentry → inode → dữ liệu/page cache`. Vẽ được chuỗi này là đạt.

</details>

**Câu 2:** VFS là "OOP trong C" — giải thích, và vì sao `sb->s_op->write_super(sb)` phải truyền `sb` hai lần?

<details><summary>Đáp án</summary>

- **VFS = common file model** (tr. 262): mỗi object (`super_block`/`inode`/`dentry`/`file`) chứa **dữ liệu + một bảng con trỏ hàm** (`s_op`/`i_op`/`d_op`/`f_op`) — chính là "class + virtual method" hiện thực bằng struct + function pointer. Filesystem cụ thể điền con trỏ vào bảng; VFS gọi qua con trỏ mà không cần biết chi tiết.
- **Truyền `sb` hai lần (tr. 268):** `sb->s_op->write_super` là *lấy* hàm; nhưng C **không có `this` ngầm** — hàm không tự biết nó thuộc superblock nào → phải **truyền `sb`** làm tham số. C++ `sb.write_super()` giấu `this`; C phải hiện.
- **Điểm cộng:** pattern này lặp khắp kernel (`file_operations` của driver, `vm_operations` của VMA — [04](04-memory.md)); "điền `NULL` thì VFS dùng hàm generic hoặc bỏ qua" là cách kế thừa mặc định.

</details>

---

## Cụm 2 — Block I/O Layer (ch. 14, tr. 289–304)

### 2.1 Block vs char device; sector vs block (tr. 289–291)

**Block device (tr. 289):** truy cập **ngẫu nhiên** các **khối (block)** cỡ cố định — **seek** được (đĩa cứng, flash). **Char device:** **luồng byte tuần tự** (bàn phím, serial). Phân biệt cốt lõi: *"whether the device can seek."* Bàn phím gõ `wolf` trả đúng `w,o,l,f` theo thứ tự — char; đĩa đọc block bất kỳ theo thứ tự bất kỳ — block. Kernel dành **cả một subsystem** cho block device vì chúng **nhạy hiệu năng** (vắt kiệt tốc độ đĩa quan trọng hơn bàn phím).

**Sector vs block (tr. 290):**
- **Sector** = đơn vị nhỏ nhất **thiết bị** địa chỉ được — vật lý, **thường 512B** (CD 2KB);
- **Block** = đơn vị nhỏ nhất **filesystem** — trừu tượng phần mềm, **bội số lũy thừa 2 của sector, ≤ kích thước trang** (512B/1KB/4KB). Kernel làm mọi thao tác đĩa theo **block**; block xây trên sector.

### 2.2 Buffer head → bio (tr. 291–296)

**Buffer & buffer head (tr. 291):** một block nằm trong RAM = một **buffer**; mô tả nó = **`struct buffer_head`** (`b_page` trang chứa, `b_blocknr` số block, `b_bdev` thiết bị, `b_state` — `BH_Uptodate`/`BH_Dirty`/`BH_Lock`). ⚠️ **Trước 2.6, buffer_head LÀ đơn vị I/O** — hai vấn đề (tr. 293): (1) struct **to, cồng kềnh**; (2) **chỉ mô tả một buffer** → chẻ I/O lớn thành nhiều `buffer_head` → overhead. → sinh ra **bio**.

**`bio` structure (tr. 294)** — container cho block I/O **đang bay**, biểu diễn thành **danh sách segment**:
```c
struct bio {
        sector_t             bi_sector;    // sector trên đĩa
        struct block_device *bi_bdev;
        unsigned long        bi_rw;        // đọc hay ghi?
        unsigned short       bi_vcnt;      // số bio_vec
        unsigned short       bi_idx;       // bio_vec hiện tại (RAID split / I/O dở dang)
        struct bio_vec      *bi_io_vec;    // mảng segment
        bio_end_io_t        *bi_end_io;    // callback khi I/O xong
        ...
};
struct bio_vec {                 // MỖI segment = <page, offset, len>
        struct page *bv_page;    // trang vật lý chứa buffer
        unsigned int bv_len;     // độ dài
        unsigned int bv_offset;  // offset trong trang
};
```
🆕 `bio_vec` = `<page, offset, len>` → **scatter-gather I/O** (tr. 295): một bio ghi được **các trang KHÔNG liền nhau** trong RAM (buffer rời rạc). Đây là lợi thế lớn nhất so với buffer_head. **bio vs buffer_head (tr. 296):** bio = một *thao tác I/O* (nhiều trang, high memory, direct I/O, scatter-gather, nhẹ); buffer_head = mô tả *một buffer* (một block↔trang). **Cả hai vẫn cần** — buffer_head làm descriptor block→page, bio làm I/O đang bay; tách nhỏ để mỗi cái gọn.

**Request queue (tr. 297):** `request_queue` — hàng đợi block I/O chờ; mỗi phần tử `struct request` gồm **nhiều bio** (block on-disk liền nhau, nhưng trong RAM không cần).

### 2.3 I/O scheduler — giảm seek (tr. 297–304)

**Vì sao (tr. 297):** gửi thẳng request theo thứ tự nhận = tệ, vì **disk seek** (định vị đầu đọc) tốn hàng **ms** — chậm nhất máy tính hiện đại. **I/O scheduler** làm **hai việc** để giảm seek:
- **Merging** — gộp request tới **sector liền kề** thành một → ít lệnh, ít seek;
- **Sorting** — giữ queue **xếp theo vị trí sector** để đầu đọc quét **một chiều** (như thang máy) → gọi là **elevator**.

⚠️ Scheduler **cố ý bất công với vài request** để tăng **throughput toàn cục** (tr. 298).

**Các scheduler:**
- **Linus Elevator (tr. 299):** merge (front/back) + sort + **age check** (request quá cũ → đẩy về đuôi chống đói). Nhưng age check yếu → **vẫn đói** (must-fix của 2.4).
- **Deadline (tr. 300):** thêm **hạn chót** mỗi request — **đọc 500ms, ghi 5s**. Vì sao ưu tiên đọc? Sách nêu **writes-starving-reads**:
  > *"read requests occur synchronously with respect to the submitting application"* — process `read()` **đứng chờ** kết quả; còn ghi *"can usually be committed to disk whenever the kernel gets around to them, entirely asynchronous"* (đã buffer). Trễ đọc = trễ người thật; trễ ghi = chỉ trễ daemon nền.

  Thêm: đọc **phụ thuộc nhau** (đọc chunk này xong mới biết đọc gì tiếp) → đói một đọc là dây chuyền. Ba queue: **sorted** (theo sector) + **read FIFO** + **write FIFO**; request hết hạn → phục vụ từ FIFO.
- **Anticipatory (tr. 302):** Deadline + **anticipation heuristic** — sau một đọc, **nán lại ~6ms** đoán "sắp có đọc gần đây" (thay vì seek đi ngay rồi seek về) → giảm bão seek. Chết theo thời đại SSD.
- **CFQ — Complete Fair Queuing (tr. 303):** **một queue mỗi process**, phục vụ **round-robin** (mặc định 4 request/lượt) → công bằng băng thông theo process (audio player luôn kịp refill buffer). **Default** thời sách, hợp desktop.
- **Noop (tr. 303):** **chỉ merge, không sort** — cho thiết bị **random-access thật** (flash) không có seek penalty → sort là phí.

**Chọn (tr. 304):** boot option `elevator=cfq|deadline|as|noop`.

### Insight đáng nhớ (cụm 2)

- **bio = scatter-gather qua `bio_vec <page,offset,len>`** (tr. 295) là nâng cấp cốt lõi so với buffer_head "một block một trang": một I/O gom được nhiều trang rời rạc → ít lệnh, hỗ trợ direct I/O/high memory.
- **Writes-starving-reads (tr. 300)** là *lý do* ưu tiên đọc: **đọc đồng bộ (có người chờ) + phụ thuộc dây chuyền**, ghi bất đồng bộ (đã buffer). Đây là insight neo cho câu interview số 1.
- **Elevator = sort như thang máy** để đầu đọc quét một chiều; **Noop cho flash** vì flash không seek → sort vô nghĩa. Nhận ra "thuật toán tùy đặc tính thiết bị" là điểm senior.

### ⚠️ Đã thay đổi so với sách

- **Toàn bộ single-queue elevator thay bằng blk-mq (multi-queue, ~5.0):** hàng đợi **per-CPU** + hardware queue (NVMe nhiều queue thật). Scheduler nay: **mq-deadline, BFQ** (fair, desktop/rotational), **kyber**, và **none** (NVMe nhanh — không schedule là nhanh nhất). **CFQ/anticipatory/Linus elevator đã XÓA.**
- Ý còn nguyên giá trị: **merge/sort/chống đói/ưu tiên đọc**. Với **eMMC/SD embedded** (một queue, chậm): scheduler vẫn quan trọng — kiểm `/sys/block/mmcblk0/queue/scheduler` (`mq-deadline`/`bfq`).
- `REQ_SYNC` đánh dấu I/O đồng bộ (kể cả ghi `fsync`) để scheduler ưu tiên đúng.

### Góc interview (cụm 2)

**Câu 1 (🎯):** Vì sao I/O scheduler **ưu tiên đọc hơn ghi**? Điều này còn đúng trên eMMC/SSD không?

<details><summary>Đáp án</summary>

- **Gốc — writes-starving-reads (tr. 300):** **đọc đồng bộ với tiến độ chương trình** — process `read()` **đứng chờ** dữ liệu mới đi tiếp (và đọc **phụ thuộc dây chuyền**: đọc block này xong mới biết đọc gì tiếp — walk inode→indirect→data). Còn **ghi đã được page cache buffer** ([cụm 3](#cụm-3--page-cache--writeback-ch-16-tr-323336)) — writeback chạy nền, process ghi xong `write()` là đi tiếp từ lâu. Trễ một đọc = trễ người thật; trễ ghi = trễ daemon nền → Deadline đặt hạn **đọc 500ms vs ghi 5s**.
- **Trên eMMC/SSD:** hết seek penalty nên phần **sort** mất giá, nhưng **ưu tiên đọc vẫn đúng** (bản chất "đọc = có người chờ" không đổi) và **merge vẫn lợi** (ít lệnh lớn tốt cho FTL hơn nhiều lệnh vụn — [write amplification](../ostep/persistence.md)). Yếu tố mới: ghi vào SLC-cache của eMMC nhanh nhưng **GC nền gây giật** → scheduler công bằng (**BFQ**) giúp app latency-sensitive. NVMe nhiều queue nhanh → **`none`** (chi phí schedule > lợi ích).
- **Điểm cộng:** biết `fsync`/`O_SYNC`/`REQ_SYNC` biến ghi thành đồng bộ — khi đó ghi cũng "có người chờ" và có đường ưu tiên riêng.

</details>

---

## Cụm 3 — Page Cache & Writeback (ch. 16, tr. 323–336)

### 3.1 Vì sao có page cache; write-back (tr. 323–324)

**Page cache** = cache trong RAM của dữ liệu đĩa. Hai lý do (tr. 323): **đĩa chậm hơn RAM hàng bậc độ lớn** (ms vs ns), và **temporal locality** (dữ liệu vừa dùng dễ dùng lại). `read()` → tra cache; **hit** trả từ RAM, **miss** đọc đĩa **rồi nạp vào cache**.

**Ghi — ba chiến lược (tr. 323–324):** **no-write** (không cache ghi — hiếm), **write-through** (ghi cache + đĩa cùng lúc — đơn giản, coherent), **write-back** (**Linux dùng**): ghi thẳng vào page cache, **đánh dấu dirty**, vào **dirty list**; **định kỳ writeback** xuống đĩa rồi bỏ dirty. ⚠️ *"dirty"* nghĩa là **dữ liệu ĐĨA lỗi thời** (cache mới hơn) — tên hơi ngược. Write-back **hơn** write-through vì hoãn ghi → **gộp + ghi cả cục sau** (đổi lấy phức tạp). 🆕 Đây là lý do **`write()` trả OK ≠ đã trên đĩa** — `fsync` là cây cầu.

### 3.2 Eviction: LRU hai danh sách (tr. 324–325)

Thu hồi cache (**cache eviction**): lý tưởng là bỏ trang **ít dùng nhất trong tương lai** — *"clairvoyant algorithm"* (bất khả). Xấp xỉ bằng **LRU** (bỏ trang cũ nhất). ⚠️ LRU thuần hỏng ở **file dùng một lần** (đọc xong không đọc lại nhưng lại nằm đầu LRU). Linux dùng **two-list strategy (LRU/2, tr. 325):** hai list **active** (nóng, không thu hồi) + **inactive** (thu hồi được); trang lên active **chỉ khi được truy cập lúc đang ở inactive** → file dùng-một-lần nằm inactive, bị thu hồi trước. Ví dụ xương sống (tr. 325): build kernel — mở/sửa/compile file **cache warm** nhanh hơn hẳn **cache cold** (fresh reboot); file bị evict nằm inactive, không phải file đang sửa.

### 3.3 `address_space` & radix tree (tr. 326–330)

**`address_space` (tr. 326):** object quản page cache của **một** đối tượng (thường một file). Sách gọi thẳng: *"the physical analogue to the virtual `vm_area_struct`"* ([04](04-memory.md)) — một file có **nhiều `vm_area_struct`** (nhiều process mmap) nhưng **một `address_space`** (tồn tại một lần trong RAM vật lý). ⚠️ Tên gây nhầm — *"A better name is perhaps `page_cache_entity`"*.
```c
struct address_space {
        struct inode          *host;        // inode sở hữu
        struct radix_tree_root page_tree;   // RADIX TREE mọi trang (tra theo offset)
        unsigned long          nrpages;
        struct address_space_operations *a_ops;   // readpage/writepage...
        ...
};
```
`a_ops` (tr. 328): **`readpage`/`writepage`** quan trọng nhất (mỗi backing store — ext3... — tự định nghĩa). Đọc: `find_get_page(mapping, index)` → miss thì `page_cache_alloc_cold` + `add_to_page_cache_lru` + `a_ops->readpage`. Ghi file-mapping: `SetPageDirty(page)`, kernel ghi sau bằng `writepage`. **Mọi page I/O đi qua page cache** (tr. 329).

**Radix tree (tr. 330):** `page_tree` — tra trang theo **offset file** nhanh (`radix_tree_lookup`). 🆕 Thay **global page hash** cũ (bốn tật: **một lock global** contention cao, hash to thừa, miss chậm, tốn RAM).

**Buffer cache (tr. 330):** block đĩa nối vào page cache qua **buffer**; **hợp nhất với page cache từ 2.4** (trước đó hai cache riêng, một block ở cả hai → phí đồng bộ). Nay **một cache đĩa duy nhất** = page cache; buffer chỉ còn mô tả mapping block→trang.

### 3.4 Flusher threads & tham số writeback (tr. 331–334)

Dirty page phải writeback trong **ba tình huống (tr. 331):** (1) **free memory < ngưỡng** (`dirty_background_ratio` — reclaim cần trang **sạch**); (2) **dirty quá già** (`dirty_expire_interval` — chống mất dữ liệu khi crash); (3) app gọi **`sync`/`fsync`**. Thời sách làm bằng **flusher threads** (gang kernel thread, **per-BDI/per-spindle** — mỗi thiết bị một luồng, tránh một đĩa chậm chặn cả hệ).

**Tham số (`/proc/sys/vm`, Bảng 16.1, tr. 332):**

| Biến | Ý nghĩa |
|---|---|
| `dirty_background_ratio` | % RAM dirty → flusher **ghi nền** bắt đầu |
| `dirty_ratio` | % RAM dirty → **process ghi bị bắt ghi đồng bộ** (throttle!) |
| `dirty_expire_interval` | dirty già bao nhiêu (ms) thì phải ghi |
| `dirty_writeback_interval` | flusher thức dậy mỗi bao nhiêu (ms) |
| `laptop_mode` | gom writeback vào lúc đĩa đã quay (tiết kiệm pin, tr. 333) |

🆕 **Nguồn "copy file lớn làm cả hệ khựng":** vượt `dirty_background_ratio` → ghi nền; vượt **`dirty_ratio`** cứng → **process ghi bị throttle ghi đồng bộ**. **Lịch sử (tr. 334):** `bdflush`+`kupdated` (một luồng, kẹt trên một queue nghẽn) → **pdflush** (2–8 luồng động, global) → **per-BDI flusher threads** (2.6.32 — một luồng/thiết bị, bỏ congestion-avoidance phức tạp). ⚠️ Nay là **writeback workqueue per-BDI** — cùng ý tưởng.

### Insight đáng nhớ (cụm 3)

- **`address_space` = "vm_area_struct vật lý"** (tr. 326): một file — nhiều VMA ảo (mỗi process/mmap) nhưng **một** address_space (một bản trong RAM vật lý). Nối [04](04-memory.md) và giải thích "vì sao nhiều process đọc cùng file chia sẻ cache".
- **Write-back + throttle tại `dirty_ratio`** (tr. 324, 332) giải thích cả "`write()` OK ≠ trên đĩa" lẫn "copy file lớn khựng cả hệ". `fsync` là cây cầu về đĩa. Đây là insight neo cho interview embedded.
- **RAM trống "bị tiêu hết" vào page cache là thiết kế đúng** (free thấp ≠ hết RAM — cache thu hồi ngay khi cần, two-list bỏ inactive trước). Câu hỏi bẫy kinh điển về `free -m`.

### ⚠️ Đã thay đổi so với sách

- **Radix tree thay bằng XArray** (4.20) cho page cache — cùng ngữ nghĩa "tra theo offset", API mới. `pdflush`→per-BDI flusher (sách đã nói) nay là **writeback workqueue**.
- **Folio** (5.16+) — nhóm trang liền nhau, giảm overhead per-page cho page cache/THP. **`madvise`/`fadvise`**, **`O_DIRECT`** (bỏ qua page cache — DB/stream lớn), **`cgroup v2` I/O + writeback throttling** (`io.latency`) — điều tiết writeback theo cgroup, quan trọng cho container/embedded multi-app.
- Tham số `*_ratio` (% RAM) nay nên dùng bản **`*_bytes`** trên thiết bị ít/nhiều RAM (ratio của RAM lớn là cục quá to).

### Góc interview (cụm 3)

**Câu 1 (🎯):** Thiết bị embedded ghi log đều; thỉnh thoảng **mọi thao tác khựng 1–2 giây**. Nghi page cache/writeback — giải thích cơ chế và cách trị.

<details><summary>Đáp án</summary>

- **Cơ chế (tr. 324, 332):** log ghi vào **page cache** (nhanh, không chạm flash) → dirty tích lũy; chạm ngưỡng — `dirty_background_ratio` → ghi nền; tệ hơn **`dirty_ratio`** → **throttle mọi process đang ghi** (bắt ghi đồng bộ) — hoặc dirty quá già → writeback **xả cục lớn xuống eMMC/SD chậm** → storage nghẽn, mọi thao tác đụng I/O (kể cả đọc, vì queue đầy) khựng theo. Flash rẻ tệ hơn: **GC nội bộ** nổ đúng lúc xả.
- **Trị theo tầng:**
  1. **Xả đều thay vì xả cục:** giảm `dirty_background_bytes` (dùng `*_bytes` thay `*_ratio` trên thiết bị ít RAM), giảm `dirty_expire_centisecs` — trả giá ghi thường hơn (cân với tuổi thọ flash).
  2. **Cô lập log:** partition/thiết bị riêng cho log (**BDI riêng** — flusher không chặn nhau — tr. 334), hoặc log vào tmpfs + flush chu kỳ có kiểm soát.
  3. **App chủ động:** `fsync` theo lô nhỏ đều đặn (tự trải), `fdatasync` thay `fsync`, `O_DIRECT` cho stream lớn (bỏ qua cache — cân nhắc).
  4. **Đo xác nhận:** `/proc/meminfo` (Dirty/Writeback), `iostat -x` (util/await spike), tracepoint writeback.
- **Điểm chấm:** giải thích được **throttle tại `dirty_ratio`** (không chỉ "đĩa bận") và trade-off với **tuổi thọ flash**.

</details>

**Câu 2:** `free -m` báo RAM còn trống rất ít, phần lớn là "buff/cache". Có phải sắp hết RAM?

<details><summary>Đáp án</summary>

- **Không.** "buff/cache" phần lớn là **page cache** (tr. 323) — RAM trống được **tiêu vào cache đĩa** là **thiết kế đúng** (RAM rảnh mà không cache thì phí). Page cache **thu hồi được ngay** khi có nhu cầu cấp phát thật: kernel evict trang **sạch** từ **inactive list** (two-list, tr. 325) — không cần writeback nếu đã sạch.
- **Nhìn đúng:** cột **available** (không phải **free**) mới là "còn dùng được bao nhiêu" — nó tính cả cache thu hồi được. Chỉ lo khi **dirty** cao mà reclaim phải writeback trước (chậm), hoặc swap tăng.
- **Điểm cộng:** phần cache **không thu hồi ngay** là **dirty page** (phải writeback trước) và **slab pin** (dentry/inode cache dcache-pin-icache — [cụm 1](#15-file-object--file_operations-tr-279284)); đối chiếu `/proc/meminfo` (Cached, Dirty, SReclaimable).

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [04-memory.md](04-memory.md) — `struct page`/`address_space`/VMA (page cache gặp bộ nhớ), `GFP_KERNEL` trong `add_to_page_cache`.
- [ostep/persistence.md](../ostep/persistence.md) — inode/journaling/FTL/write-amplification: nửa lý thuyết của filesystem & flash.
- [01-process-sched-syscalls.md](01-process-sched-syscalls.md) — clone flags (`CLONE_FILES`/`CLONE_FS`/`CLONE_NEWNS`) đứng sau `files_struct`/namespace; [04-linux-system-programming/file-io.md](../../04-linux-system-programming/file-io.md) — fd/`fsync`/`O_DIRECT` mức API.
- [05-drivers-device-tree/kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md) — `file_operations`/`ioctl` góc viết driver.

**Chương tiếp theo:** [06 — Devices, Modules & Debugging →](06-modules-debug.md) (kobject/sysfs, device model, module, printk/oops/ftrace — cách kernel tổ chức thiết bị và cách bạn gỡ lỗi nó).
