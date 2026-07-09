# OSTEP — Phần III: Persistence (ch. 36–45, lướt 48–50)

> Thuộc [OSTEP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Crux của phần này: *dữ liệu phải đúng và còn nguyên sau khi mất điện — trên thiết bị chậm hơn RAM hàng nghìn lần và có thể hỏng nửa chừng khi đang ghi.*

---

## Cụm 1 — I/O Devices & Hard Disk (ch. 36–37)

### Nội dung chính

**Giao tiếp thiết bị (ch. 36).** Device = interface (registers: status/command/data) + internals. Hệ thống nói chuyện với device qua hai cách: **explicit I/O instructions** (x86 `in/out`) hoặc **memory-mapped I/O** — thanh ghi map vào address space, đọc/ghi như bộ nhớ (chuẩn mực trên ARM/embedded — chính là chỗ `volatile` sống, xem [EMC++ Item 40](../effective-modern-cpp.md)).

Chuỗi tiến hóa hiệu quả — mỗi bước bớt lãng phí CPU:
1. **Polling**: CPU quay vòng đọc status chờ device xong — phí CPU khi device chậm.
2. **Interrupt**: CPU giao việc rồi đi làm việc khác; device xong thì **ngắt** → overlap CPU/I/O. Nhưng interrupt **không luôn thắng**: device nhanh (NVMe, mạng 10G+) → chi phí interrupt + context switch vượt thời gian chờ → **polling lại tốt hơn**; dòng sự kiện dày → **livelock** (CPU chỉ lo trả interrupt) → **hybrid/coalescing**: gom nhiều completion một interrupt (NAPI của Linux network: interrupt đến thì chuyển sang poll một lúc).
3. **DMA**: CPU đừng chép từng byte (programmed I/O) — giao DMA engine chép thẳng RAM↔device, xong mới ngắt một phát.

**Device driver** = lớp trừu tượng hóa để kernel không cần biết chi tiết từng device — chiếm ~70% code kernel; đa số bug kernel nằm ở driver (đúng nghề của bạn — [05/driver-basics.md](../../05-drivers-device-tree/driver-basics.md)).

**HDD (ch. 37):** đĩa quay + đầu đọc: thời gian I/O = **seek** (dời đầu đọc, ~ms) + **rotational delay** (chờ sector quay tới, ~ms) + **transfer** (µs). Hệ quả chi phối mọi thiết kế FS thời đĩa quay: **sequential I/O nhanh hơn random I/O hàng trăm lần**. Disk scheduling: SSTF (gần nhất trước — đói vùng xa), SCAN/C-SCAN (quét như thang máy), SPTF (tối ưu tổng seek+rotation — cần biết geometry, làm trong firmware đĩa).

### Insight đáng nhớ

- Bảng cửu chương của persistence: **"tuần tự thắng ngẫu nhiên"** — mọi thiết kế phần sau (FFS đặt gần, journaling ghi log tuần tự, LFS ghi thuần log, thậm chí SSD ghi theo block) đều là biến tấu của câu này.
- Polling vs interrupt là trade-off **tần suất sự kiện vs chi phí mỗi sự kiện** — cùng khung với spinlock vs mutex, busy-wait vs sleep. Nhận ra "cùng một trade-off mặc nhiều bộ áo" là loại insight interviewer đánh giá cao.

### Ít quan trọng

- Công thức tính geometry đĩa, track skew, số liệu đĩa Cheetah/Barracuda (ch. 37 giữa) — thời đại HDD làm storage chính đã qua với embedded.

### Góc interview

**Câu 1:** Khi nào polling tốt hơn interrupt? Liên hệ một ví dụ thực tế.

<details><summary>Đáp án</summary>

- Interrupt thắng khi device **chậm so với chi phí interrupt** (context switch, cache pollution, vào/ra handler ~µs): giao việc, đi làm việc khác, được gọi khi xong. Polling thắng khi: (1) device **rất nhanh** — kết quả về trong thời gian ngắn hơn chi phí một interrupt (NVMe µs-class, DPDK network); (2) sự kiện **dày đặc** — interrupt mỗi packet ở 10Gbps là livelock: CPU 100% chỉ để vào/ra handler, không ai xử lý dữ liệu.
- Thực tế: **NAPI** (Linux networking) — interrupt đầu tiên đến thì *tắt interrupt của NIC*, chuyển sang poll theo budget, vãn việc mới bật lại: tự thích nghi tải thưa (interrupt) ↔ tải dày (poll). `io_uring` với `IORING_SETUP_IOPOLL`, DPDK/SPDK bỏ hẳn interrupt cho hot path. Embedded: UART tốc độ thấp dùng interrupt + FIFO; còn vòng điều khiển chu kỳ cố định đọc sensor qua SPI có thể poll trong khung thời gian đã trả tiền sẵn.
- Điểm cộng: interrupt coalescing (gom N completion/timeout một ngắt) là nấc giữa; và mọi lựa chọn quy về **latency vs throughput vs CPU budget**.

</details>

---

## Cụm 2 — RAID (ch. 38)

### Nội dung chính

RAID = nhiều đĩa giả làm một: đổi tiền lấy **capacity / performance / reliability** — ba trục không cùng thắng:

| Mức | Cơ chế | Capacity (N đĩa) | Chịu hỏng | Đặc điểm |
|---|---|---|---|---|
| RAID-0 | Striping (rải block) | N | 0 đĩa | Nhanh nhất, chết một là mất hết |
| RAID-1 | Mirroring | N/2 | 1/cặp | Đọc nhanh (2 nguồn), ghi = 2 lần ghi |
| RAID-4 | Striping + 1 đĩa parity | N−1 | 1 | **Parity disk nghẽn**: mọi small write đều đụng nó |
| RAID-5 | Parity **xoay vòng** qua các đĩa | N−1 | 1 | Chuẩn phổ biến; small write vẫn tốn 4 I/O (đọc cũ+parity, ghi mới+parity) |

Parity = XOR các block cùng stripe; mất một đĩa → khôi phục bằng XOR phần còn lại. **Small-write problem** của RAID-4/5: sửa 1 block phải đọc-sửa-ghi cả parity (4 thao tác I/O) — lý do RAID-1 vẫn được chuộng cho workload ghi nhỏ ngẫu nhiên.

### Insight đáng nhớ

- Câu chốt: **RAID chống hỏng ĐĨA, không chống xóa nhầm/ransomware/bug ghi bậy — RAID không phải backup.** Câu này phân biệt người hiểu với người thuộc bảng.
- Tư duy "một lớp gián tiếp biến nhiều thành một" (transparency) của RAID lặp lại ở LVM, FTL của SSD (cụm 6), thậm chí virtual memory — pattern trừu tượng hóa quen thuộc.

### Ít quan trọng

- Phân tích throughput từng mức theo mô hình tuần tự/ngẫu nhiên chi tiết (ch. 38 giữa) — đọc khi cần tính toán thiết kế storage thật.

### Góc interview

**Câu 1:** Vì sao RAID-5 ghi nhỏ chậm? Hệ thống của bạn ghi log nhỏ liên tục — chọn RAID nào?

<details><summary>Đáp án</summary>

- Ghi 1 block trong stripe RAID-5 phải cập nhật parity **nhất quán**: đọc block cũ + parity cũ, tính parity mới = parity_cũ XOR data_cũ XOR data_mới, ghi block mới + parity mới → **4 I/O cho 1 block ghi** (2 đọc + 2 ghi), lại thêm 2 đĩa bị chiếm. Ghi full-stripe thì không phải đọc gì (tính parity từ dữ liệu mới toàn bộ) — nên RAID-5 hợp ghi lớn tuần tự.
- Log nhỏ liên tục = small random-ish write → RAID-5 trả 4× penalty mỗi lần → chọn **RAID-1 (hoặc RAID-10)**: mỗi ghi chỉ là 2 ghi song song, latency thấp, không read-modify-write. Trả giá capacity 50%.
- Điểm cộng: nêu write-back cache có pin/NVRAM của controller làm dịu penalty; và cảnh báo **write hole** của RAID-5 khi mất điện giữa data và parity (cần journal/battery) — nối thẳng sang chủ đề crash consistency (cụm 5).

</details>

---

## Cụm 3 — File & Directory API (ch. 39)

### Nội dung chính

Hai trừu tượng của storage ảo hóa: **file** (mảng byte, tên thật là **inode number**) và **directory** (danh sách cặp `(tên người đọc được, inode number)` — bản thân directory cũng là file). Từ đó cây tên `/foo/bar` chỉ là chuỗi tra cặp qua từng cấp.

Điểm API đáng chú ý (phần lớn repo 04 đã kỹ — đây là các ý *vì sao* của sách):
- `open()` trả **fd**; bảng ba tầng: fd table (per-process) → **open file table** toàn hệ (giữ offset, flags) → inode. `fork` chia sẻ entry open file table (**chung offset**); `dup` cũng vậy — giải thích redirect shell.
- **`write()` thành công ≠ dữ liệu trên đĩa** — nó vào page cache; muốn bền phải `fsync(fd)` (và với file mới: fsync cả **thư mục cha** — entry tên nằm ở đó!). Đây là nguồn bug mất dữ liệu số một khi mất điện.
- **Hard link** (`ln`): thêm một cặp tên→cùng inode; inode có **link count**, `rm` chỉ là `unlink` — giảm count, count=0 *và không ai mở* mới thật sự xóa (file đang mở bị rm vẫn dùng được — trick file tạm). Không hard link được qua FS khác/directory (tránh chu trình).
- **Symbolic link**: file riêng chứa **đường dẫn** — link được qua FS, tới directory; trả giá **dangling** (đích bị xóa, symlink thành trỏ ma).
- `mkfs` tạo FS trên partition; `mount` ghép cây — mọi FS khác loại hợp thành **một cây tên duy nhất** (điểm hay của UNIX so với ổ C:/D:).

### Insight đáng nhớ

- **"Tên thật của file là inode number; tên chữ chỉ là bí danh trong directory"** — một câu mở khóa: hard vs soft link, vì sao rm file đang mở vẫn chạy, vì sao rename cùng FS là atomic còn copy qua FS thì không, vì sao phải fsync thư mục.
- `write → page cache → fsync` là ranh giới trách nhiệm OS/app: OS tối ưu (gom, trì hoãn ghi), app muốn **durability tại thời điểm cụ thể** phải tự đòi. Pattern chuẩn ghi file an toàn: *write temp → fsync temp → rename đè → fsync dir* (dùng tính atomic của rename).

### Ít quan trọng

- Từng syscall `stat/lseek/rename/getdents` và bit permission/umask (ch. 39) — repo [04/file-io.md](../../04-linux-system-programming/file-io.md) đã bao; TOCTOU aside — đáng biết tên (race giữa check và use trên đường dẫn — cùng họ atomicity violation).

### Góc interview

**Câu 1:** Hard link vs symbolic link — khác nhau thế nào? Vì sao xóa file đang được process mở mà process vẫn đọc ghi bình thường?

<details><summary>Đáp án</summary>

| | Hard link | Symlink |
|---|---|---|
| Bản chất | Cặp tên→**inode** thêm trong directory | File riêng (inode riêng) chứa **chuỗi đường dẫn** |
| Qua filesystem khác | ❌ (inode number chỉ có nghĩa nội bộ FS) | ✅ |
| Trỏ directory | ❌ (chống chu trình trong cây) | ✅ |
| Đích bị xóa | Không có khái niệm "đích chết" — dữ liệu còn khi còn ≥1 link | **Dangling** — trỏ vào hư không |
| Cách xóa dữ liệu thật | unlink đến khi link count = 0 | Xóa symlink không ảnh hưởng đích |

- File đang mở bị `rm`: `rm` = `unlink()` — gỡ **tên** khỏi directory, giảm link count. Dữ liệu/inode chỉ được giải phóng khi **link count = 0 VÀ open count = 0**. Process đang mở giữ tham chiếu qua open file table → inode sống tiếp, đọc/ghi vô tư; đóng fd cuối cùng thì FS mới thu hồi block. Đây là kỹ thuật file tạm tự hủy (`open` + `unlink` ngay), và giải thích hiện tượng "df đầy mà du không thấy" — log khổng lồ bị rm nhưng daemon còn mở (`lsof +L1` tìm).

</details>

---

## Cụm 4 — File System Implementation: vsfs (ch. 40) 🎯

### Nội dung chính

vsfs (very simple FS) — mô hình tối giản mang cấu trúc của ext2. **Bố cục on-disk** (đĩa chia block 4KB):

```
[ S | ib | db | I I I I I | D D D D D D D ... ]
  S  = superblock: magic, tổng số inode/block, vị trí các vùng — mount đọc nó trước
  ib = inode bitmap  (inode nào đã dùng)
  db = data bitmap   (block nào đã dùng)
  I  = inode table   (mảng inode, mỗi cái ~128-256B — inode NUMBER là chỉ số mảng này)
  D  = data blocks
```

**Inode** = toàn bộ metadata một file: type, size, permissions, uid/gid, timestamps, link count, **và các con trỏ tới data block**. Cấu trúc con trỏ — **multi-level index**:

```
inode
├── 12 direct pointers          → 12 × 4KB = 48KB đầu tiên
├── single indirect ──► block chứa 1024 con trỏ  → +4MB
├── double indirect ──► 1024 × 1024              → +4GB
└── triple indirect                              → +4TB
```

Thiết kế **bất đối xứng có chủ đích**: đa số file **nhỏ** (thống kê: phần lớn < vài chục KB) → 12 direct pointer phục vụ họ với 0 lần đọc phụ; file lớn hiếm mới trả giá indirect. (Cách khác: extent — dải liền `(start, len)` như ext4, nhỏ gọn hơn cho file liền mạch.)

**Directory** = file có type=dir, nội dung là mảng entry `(inode#, reclen, strlen, name)` — xóa entry để lại lỗ (reclen phủ) cho lần thêm sau.

**Đường đi của `open("/foo/bar", O_RDONLY)` — traversal phải kể được:**

```
inode 2 (quy ước: root "/") → đọc data block của root → tìm "foo" → inode# foo
→ đọc inode foo (check permission!) → đọc data → tìm "bar" → inode# bar
→ đọc inode bar → cấp fd trỏ tới nó
```

Mỗi cấp path = ≥2 lần đọc (inode + data) — path dài, thư mục lớn là tốn; **page cache + dentry cache** cứu: lần mở sau gần như 0 I/O.

- `read()` sau open: tra inode (đã cache) → tính block từ offset → đọc data; cập nhật atime (lại ghi inode!).
- `write()/creat` đắt hơn nhiều: tạo file mới = đọc+ghi inode bitmap, ghi inode mới, đọc+ghi data block directory, ghi inode directory... — **một creat ~10 I/O**; ghi thêm block mới = cập nhật data bitmap + inode + data. Đây là lý do caching + **write buffering** (gom, trì hoãn 5–30s, ghi lô) là bắt buộc — và là cái giá crash consistency phải trả ở cụm 5.

### Insight đáng nhớ

- **"FS = hai bảng tra + hai bitmap":** tên→inode# (directory), inode#→block (inode). Nhớ khung này thì mọi câu hỏi FS (link, quota, du vs df, fragmentation) đều tự suy ra được.
- Mọi thao tác "một lệnh" của user (creat, write append) là **nhiều bước ghi rời rạc trên đĩa** — quan sát này chính là *lý do tồn tại* của journaling: đứt điện giữa các bước là FS sai lệch.

### Ít quan trọng

- Bảng đếm từng I/O cho mỗi syscall trong các kịch bản (ch. 40 giữa) — đọc lại khi cần đếm chính xác; free-list thay bitmap (lịch sử).

### Góc interview

**Câu 1 (🎯):** Inode chứa gì và KHÔNG chứa gì? Từ inode, FS tìm dữ liệu ở offset 5MB của file thế nào?

<details><summary>Đáp án</summary>

- **Chứa:** type, size, permission, owner, timestamps, **link count**, số block đã cấp, và **bảng con trỏ data block** (12 direct + single/double/triple indirect trong mô hình ext2). **Không chứa: TÊN FILE** — tên nằm trong data block của *directory* (cặp tên→inode#). Một inode có thể nhiều tên (hard link).
- Offset 5MB (block 4KB): block logic số 5MB/4KB = **1280**. 0–11: direct; 12–1035: single indirect (1024 con trỏ); 1280 rơi vào **double indirect**: index trong vùng double = 1280−1036 = 244 → block con trỏ cấp 1 số 244/1024 = **0**, vị trí 244%1024 = **244**. Đường đi: inode → đọc block double-indirect → entry 0 → đọc block indirect cấp 2 → entry 244 → data block. **3 lần đọc phụ** trước khi chạm dữ liệu (cache thường che hết).
- Điểm cộng: giải thích thiết kế nghiêng về file nhỏ (12 direct = 48KB free lookup); so sánh **extent-based** (ext4: dải `(start,len)` — gọn cho file liền, kém linh hoạt khi phân mảnh); và sparse file — offset chưa ghi = con trỏ null, đọc ra zero, không tốn đĩa.

</details>

**Câu 2:** `df` báo hết dung lượng nhưng `du` cộng lại còn xa mới đầy — nêu các nguyên nhân theo cấu trúc FS.

<details><summary>Đáp án</summary>

1. **File bị unlink nhưng còn mở** (phổ biến nhất): tên đã gỡ (du không đếm — nó duyệt theo tên) nhưng inode + block còn giữ vì open count > 0 (df đếm block đã cấp). Tìm: `lsof +L1`; xử lý: restart/`truncate` qua `/proc/<pid>/fd`.
2. **Hết inode chứ không hết block** (`df -i`): triệu file bé (cache, session) cạn inode table — FS "đầy" mà block trống. Bài học: inode cấp cố định lúc mkfs (ext4), phải chọn `-i bytes-per-inode` cho workload nhiều file nhỏ.
3. Reserved blocks cho root (ext mặc định 5%), block bị chiếm bởi journal, hoặc mount point bị **che** (dữ liệu ghi vào /mnt lúc chưa mount, giờ nằm dưới mount — du không thấy).
- Trả lời có cấu trúc "tên vs inode vs block" thể hiện nắm mô hình ba tầng của FS thay vì mẹo vặt.

</details>

---

## Cụm 5 — Locality (FFS) & Crash Consistency: fsck → Journaling (ch. 41–42) 🎯

### Nội dung chính

**FFS (ch. 41) — "disk awareness":** FS đời đầu vứt inode một đầu, data một đầu → mỗi lần đọc file là seek xuyên đĩa. FFS chia đĩa thành **cylinder/block group**, giữ **inode + data + bitmap của nhau ở gần nhau**; chính sách: file cùng directory → cùng group; directory mới → group vắng. Ngoại lệ file lớn: rải mỗi vài MB sang group khác (amortize seek — không cho một file nuốt trọn group). Di sản: mọi FS hiện đại (ext4 block groups) vẫn là con cháu tư tưởng "đặt gần thứ dùng cùng nhau".

**Crash consistency (ch. 42) — chương đắt nhất phần này.** Ví dụ chuẩn: append 1 block vào file = ghi **3 thứ**: data bitmap (B), inode (I), data block (D). Mất điện giữa chừng — xét từng tổ hợp ghi được 1/3, 2/3:

| Ghi được | Hậu quả |
|---|---|
| chỉ D | Vô hại (data mồ côi, không ai trỏ) |
| chỉ I | Inode trỏ block rác; **I−B mâu thuẫn** |
| chỉ B | **Space leak** — block bị đánh dấu dùng mà không ai trỏ |
| I+B (thiếu D) | Nguy hiểm nhất loại 1: đọc ra **rác như thể dữ liệu thật** |
| I+D (thiếu B) | Mâu thuẫn metadata → block có thể bị cấp lần nữa → hai file chung block |
| B+D (thiếu I) | Leak — không ai biết block của ai |

**Cách 1 — fsck:** để crash tùy ý, boot lên **quét toàn bộ** sửa mâu thuẫn (bitmap vs thực tế, link count, block trùng chủ, entry mồ côi → lost+found). Hai điểm chết: (1) chỉ sửa **metadata nhất quán**, không cứu nổi dữ liệu (case I+B thiếu D — fsck thấy "hợp lệ", dữ liệu là rác); (2) **quá chậm** — quét cả volume TB mất hàng giờ cho một lần crash vài giây ("ngớ ngẩn một cách hệ thống": không biết chỗ nào đang dở thì kiểm tra tất cả).

**Cách 2 — Journaling (write-ahead logging)** — ext3/ext4, XFS, NTFS: **viết ý định trước, làm thật sau**:

```
1. Ghi vào JOURNAL: TxBegin | I' | B' | D' (nội dung sẽ ghi)     ── journal write
2. Ghi TxEnd (commit record)                                      ── commit
3. CHECKPOINT: ghi I', B', D' vào vị trí thật                     ── checkpoint
4. Free entry journal (vòng tròn tái sử dụng)

Crash TRƯỚC bước 2 → transaction chưa commit → bỏ qua — FS như chưa hề có thao tác
Crash SAU bước 2  → boot: REPLAY các transaction đã commit (redo logging) → hoàn tất nốt
→ mọi thao tác thành ATOMIC: hoặc trọn vẹn, hoặc không gì cả; recovery = đọc journal (giây)
```

Chi tiết tinh tế đáng nhớ: bước 1 và 2 phải **có barrier/thứ tự** — TxEnd chỉ được ghi sau khi toàn bộ nội dung transaction đã nằm trên đĩa (đĩa reorder ghi! — đây là chỗ write cache của đĩa/fsync/FUA can dự); checkpoint chỉ sau commit.

**Data vs metadata journaling:**
- **Data journaling** (ext3 `data=journal`): journal cả D — mọi block **ghi hai lần** (journal rồi chỗ thật) → an toàn nhất, chậm (băng thông ghi ÷2).
- **Metadata/ordered journaling** (`data=ordered` — **mặc định ext4**): chỉ journal metadata (I, B); D ghi **thẳng chỗ thật, TRƯỚC khi commit metadata** — tránh "inode trỏ rác" mà không trả giá ghi đôi. Thứ tự là tất cả: *data trước, metadata commit sau*.
- `data=writeback`: không ràng thứ tự data — nhanh, sau crash file có thể chứa rác cũ.

Tối ưu khác: batch nhiều thao tác một transaction; journal xoay vòng; revoke record (chống replay đè lên block đã tái cấp).

### Insight đáng nhớ

- **Journaling là ý tưởng tổng quát nhất bạn học được từ FS**: write-ahead log + commit record + replay = cách biến chuỗi bước thành atomic. Cùng một bài: database WAL/redo log, ARM/flash bootloader A/B update, RAID write-hole fix, thậm chí "write temp + rename" của app — nhận ra họ hàng này là câu trả lời system design đẹp.
- Câu hỏi "crash giữa chừng thì sao?" phải trở thành **phản xạ thiết kế** — bảng 6 tổ hợp ở trên là bài tập tư duy mẫu: liệt kê thứ tự ghi, xét từng điểm đứt.
- Embedded: thiết bị **rút điện bất cứ lúc nào** là chuyện thường ngày → chọn `data=ordered`+ext4, hoặc f2fs, kèm test rút điện thật; và write cache của eMMC có thể nuốt barrier — cần reliable write/cache flush đúng chuẩn (nối cụm 6).

### Ít quan trọng

- Chi tiết fsck từng pha kiểm tra; các biến thể soft updates/copy-on-write (ZFS/btrfs — nhắc tên ở cụm 6), backpointer-based consistency, optimistic crash consistency (ch. 42 cuối) — biết tồn tại là đủ ở mức interview.

### Góc interview

**Câu 1 (🎯):** Journaling giải quyết bài toán gì và bằng cơ chế nào? Vì sao ext4 mặc định chỉ journal metadata mà vẫn an toàn?

<details><summary>Đáp án</summary>

- **Bài toán:** một thao tác FS = nhiều lần ghi rời rạc (bitmap, inode, data); mất điện giữa chừng → trạng thái nửa vời (inode trỏ rác, block leak, hai file chung block). Cần **atomicity cho nhóm ghi** trên thiết bị chỉ atomic từng sector.
- **Cơ chế — write-ahead logging:** (1) ghi *toàn bộ nội dung định ghi* vào vùng journal; (2) ghi **commit record** (chỉ sau khi (1) đã bền — cần barrier vì đĩa reorder); (3) **checkpoint** — ghi vào vị trí thật; (4) giải phóng entry. Recovery sau crash: transaction **có commit** → replay (redo); **không có commit** → bỏ — hai trạng thái đều nhất quán, quét chỉ journal (giây) thay vì cả volume như fsck (giờ).
- **ext4 `data=ordered`:** journal metadata thôi nhưng **ép thứ tự: data block ghi xong chỗ thật TRƯỚC khi commit metadata**. Nhờ đó tình huống "inode đã trỏ mà data chưa ghi" (đọc ra rác — rủi ro *bảo mật* lẫn đúng đắn) không xảy ra; đổi lại không tốn ghi-đôi data như `data=journal`. Điều mất so với full journaling: nội dung **ghi đè giữa file** sau crash có thể dở dang (atomicity chỉ cho metadata) — app cần atomic content tự lo (rename pattern, fsync).
- **Bẫy khi trả lời:** nói "journal chống mất dữ liệu" — sai trọng tâm: nó bảo đảm **nhất quán (consistency)**; dữ liệu chưa fsync vẫn mất như thường. Và quên vế thứ tự/barrier — không có nó toàn bộ lâu đài sụp (commit record ghi trước nội dung = replay rác).

</details>

**Câu 2:** Thiết bị nhúng của bạn bị rút điện thường xuyên. Liệt kê các tầng bạn phải rà để "ghi file cấu hình an toàn tuyệt đối".

<details><summary>Đáp án</summary>

Từ app xuống phần cứng — đứt tầng nào hỏng tầng đó:
1. **App — atomic replace pattern:** ghi `config.tmp` → `fsync(tmp)` → `rename(tmp, config)` → `fsync(thư mục)`. rename cùng FS là atomic; fsync directory để entry tên bền. Không bao giờ ghi đè trực tiếp file cũ (crash giữa chừng = mất cả cũ lẫn mới).
2. **FS:** ext4 `data=ordered` trở lên (hoặc f2fs cho flash); kiểm tra mount option — `nobarrier` là tự sát trên thiết bị mất điện đột ngột.
3. **Block/thiết bị:** write cache của eMMC/SD có thể nhận fsync mà chưa ghi thật — cần cache flush/FUA hoạt động đúng (eMMC reliable write); SD card rẻ tiền nổi tiếng nói dối flush → chọn linh kiện công nghiệp có power-loss protection, hoặc thiết kế tụ giữ điện đủ flush.
4. **Kiến trúc:** dữ liệu quý ghi kiểu append-only + checksum từng record (đọc lại bỏ record cụt), hoặc A/B hai bản + sequence number + CRC — boot chọn bản hợp lệ mới nhất (chính là double-buffer/journal thủ công).
5. **Kiểm chứng:** test rút điện tự động hàng nghìn chu kỳ (relay cắt nguồn trong khi ghi) — niềm tin không thay được số liệu.
- Trả lời theo tầng như trên (app → FS → device → kiến trúc → test) chính là format senior được chờ đợi.

</details>

---

## Cụm 6 — LFS & Flash/SSD (ch. 43–44)

### Nội dung chính

**LFS — Log-structured FS (ch. 43):** quan sát: RAM cache lo phần đọc → traffic đĩa còn lại chủ yếu là **ghi**; vậy tối ưu ghi triệt để: **không bao giờ ghi đè tại chỗ** — mọi thứ (data + inode mới) gom vào **segment** lớn trong RAM, đầy thì ghi **tuần tự** xuống cuối log. Inode giờ "trôi" theo log → thêm **inode map** (imap — inode# → vị trí inode mới nhất, bản thân imap cũng ghi vào log, checkpoint region cố định trỏ imap). Cái giá: dữ liệu cũ thành **rác rải khắp log** → cần **garbage collection/cleaner**: đọc segment cũ, chép phần còn sống sang segment mới, giải phóng — cleaner chính là gót chân Achilles (tốn I/O nền, tranh băng thông).

**Flash/SSD (ch. 44):** NAND flash — ba thao tác bất đối xứng: **read** (µs, theo page ~4KB), **program** (ghi page — chỉ được ghi vào page **đã erase**), **erase** (theo **block** ~256KB–vài MB, chậm ms, và **mòn**: mỗi block chịu hữu hạn chu kỳ P/E — SLC ~100k, MLC ~10k, TLC/QLC ít hơn nữa). Không ghi đè tại chỗ được → **FTL (flash translation layer)** trong firmware SSD/eMMC:
- **Log-structured mapping**: ghi luôn vào chỗ đã erase sẵn, giữ bảng map (logical→physical) — *đúng ý tưởng LFS trong phần cứng*;
- **Garbage collection**: dồn page sống, erase block — sinh **write amplification** (1 ghi logic = nhiều ghi vật lý);
- **Wear leveling**: xoay đều P/E khắp block — kể cả dữ liệu "nằm im" cũng bị di dời để chia sẻ độ mòn;
- **TRIM/discard**: OS báo "block logic này đã free" để FTL khỏi bảo tồn rác khi GC.
- Overprovisioning (vùng dự phòng ẩn) cho GC thở; mapping lai block/page để bảng map vừa RAM.

### Insight đáng nhớ

- Vòng lặp lịch sử đẹp nhất sách: **LFS (1991) bị chê vì cleaner — 20 năm sau NAND flash *bắt buộc* mọi thiết bị làm LFS trong firmware (FTL), và f2fs mang nó về lại kernel.** Ý tưởng đúng chờ được phần cứng đúng.
- Embedded phải thuộc: eMMC/SD **đang chạy một FS ẩn (FTL) dưới FS của bạn** — hai tầng GC chồng nhau; hệ quả thực dụng: bật discard/TRIM định kỳ, tránh ghi nhỏ rải rác (write amplification giết tuổi thọ), chọn f2fs/ext4 tùy media, và **budget tuổi thọ ghi** (TBW) ngay từ thiết kế — log 1 dòng/giây × vài năm là giết thẻ SD rẻ.

### Ít quan trọng

- Số liệu chi tiết bảng mapping lai, cost model cleaner của LFS (ch. 43–44 giữa) — đọc khi làm việc trực tiếp với FTL/f2fs tuning.

### Góc interview

**Câu 1:** Vì sao ghi ngẫu nhiên nhỏ có hại cho SSD/eMMC? Write amplification là gì và giảm bằng cách nào?

<details><summary>Đáp án</summary>

- Flash chỉ ghi vào page đã erase, erase theo block lớn (hàng trăm page). Ghi nhỏ rải rác làm block nào cũng lốm đốm page sống/chết → GC muốn giải phóng phải **chép page sống đi nơi khác rồi mới erase** → một ghi logic kéo theo nhiều ghi vật lý = **write amplification (WA = ghi vật lý/ghi logic)**. WA cao → chậm (băng thông ăn vào GC) + **mòn nhanh** (P/E cycles hữu hạn).
- Giảm WA: (1) **ghi tuần tự/gom lô** — append log thay vì update rải (chính là lý do f2fs/LFS hợp flash); (2) **TRIM/discard** — báo FTL trang nào là rác thật để GC khỏi chép; (3) **overprovisioning** — chừa chỗ trống nhiều, GC nhàn; (4) align partition/FS với erase block; (5) giảm fsync vụn (mỗi fsync ép flush journal nhỏ giọt) — gom transaction.
- Điểm cộng embedded: kể TBW/endurance budget: ước lượng byte ghi/ngày × WA × năm so với datasheet; pattern "ghi log vòng tròn kích thước cố định + fsync theo chu kỳ" thay vì append vô hạn; SLC-mode partition của eMMC cho vùng ghi nóng.

</details>

---

## Cụm 7 — Data Integrity & Distributed (ch. 45, 48–50) — lướt

### Nội dung chính (rút gọn có chủ đích)

- **Data integrity (ch. 45):** đĩa không chỉ "chết hẳn" — còn **latent sector error** (đọc ra lỗi) và đáng sợ hơn: **silent corruption** (đọc ra dữ liệu SAI mà không báo lỗi — bit rot, misdirected write, lost write). Chống: **checksum mỗi block** (lưu tách khỏi data), scrubbing định kỳ đọc-kiểm tra toàn bộ; ZFS/btrfs làm chuẩn này ở FS. Bài học một câu: **"đĩa nói ghi xong" và "dữ liệu đúng" là hai mệnh đề khác nhau — chỉ checksum end-to-end mới nối chúng** (end-to-end argument).
- **Distributed (ch. 48–50):** mạng thêm chiều lỗi mới — **mất gói, trễ, partition**; RPC làm remote call giả dạng local call (nhưng ngữ nghĩa lỗi khác hẳn: timeout không biết đã thực thi chưa → cần **idempotency/at-least-once vs at-most-once**). **NFS**: server **stateless** + operation idempotent → server crash, client chỉ việc retry (triết lý "recovery = retry"); cache client sinh **stale data** (attribute cache + close-to-open consistency). **AFS**: cache cả file + **callback** (server hứa báo khi file đổi) — consistency rõ hơn, server giữ state. Cặp NFS/AFS = bài mẫu trade-off stateless-dễ-recover vs stateful-consistency-tốt.

### Góc interview

**Câu 1:** "Ghi thành công" từ góc nhìn app có bao nhiêu tầng có thể nói dối? Kể từ trên xuống và cách kiểm soát từng tầng.

<details><summary>Đáp án</summary>

1. **`write()` trả OK** — mới vào page cache (RAM). Kiểm soát: `fsync`/`fdatasync`/`O_SYNC`.
2. **FS/journal** — fsync xong nhưng thứ tự ghi journal cần barrier đúng (mount option, không `nobarrier`).
3. **Write cache của thiết bị** (eMMC/SSD/HDD cache DRAM) — nhận flush nhưng firmware rẻ tiền có thể nói dối; cần thiết bị có power-loss protection/reliable write, hoặc test rút điện chứng minh.
4. **Media** — ghi thật nhưng sau này **silent corruption** (bit rot, lost write): chỉ **checksum end-to-end** (app hoặc FS kiểu ZFS/btrfs, scrub định kỳ) phát hiện được; RAID/redundancy để sửa.
- Chốt bằng end-to-end argument: tầng dưới không thể tự chứng minh tính đúng cho tầng trên — muốn chắc, **bên sở hữu dữ liệu phải tự kiểm tra** (checksum trong format file/record). Trả lời 4 tầng + nguyên lý này là mức senior.

</details>

### Đọc thêm (tùy chọn)

- [04/file-io.md](../../04-linux-system-programming/file-io.md) — fd, page cache, fsync mức API.
- [05/driver-basics.md](../../05-drivers-device-tree/driver-basics.md) — block device, cấu trúc driver.
- [08/constraints.md](../../08-embedded-systems/constraints.md) — ràng buộc flash/power của thiết bị nhúng.
