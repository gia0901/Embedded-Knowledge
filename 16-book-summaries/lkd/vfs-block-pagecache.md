# LKD — VFS, Block Layer & Page Cache (ch. 13 tr. 261, ch. 14 tr. 289, ch. 16 tr. 323)

> Thuộc [LKD](README.md). Nguồn: kiến thức Claude, số trang đối chiếu PDF 3rd ed.
> Nền lý thuyết FS (inode, journaling, đường read/write) đã ở [ostep/persistence.md](../ostep/persistence.md) — file này bổ sung **cách kernel trừu tượng hóa**: bốn object VFS, bio, writeback.

---

## Cụm 1 — VFS: bốn object (ch. 13, tr. 261–288)

### Nội dung chính

**VFS = lớp trừu tượng để một `read()` chạy trên ext4/UBIFS/NFS/procfs như nhau** (tr. 262): syscall gọi hàm VFS → VFS gọi **function pointer** của FS cụ thể (OOP bằng C: struct ops — cùng pattern mọi subsystem kernel).

**Bốn object phải thuộc (tr. 265):**

| Object | Đại diện | Ops tiêu biểu | Ghi chú |
|---|---|---|---|
| **superblock** (tr. 266) | Một FS đã mount | alloc_inode, write_super, sync_fs | Ứng bản on-disk superblock ([OSTEP vsfs](../ostep/persistence.md)) |
| **inode** (tr. 270) | Một file/dir (metadata) | create, lookup, mkdir, permission | In-memory; FS ảo (procfs) *chế* inode khi cần |
| **dentry** (tr. 275) | **Một thành phần đường dẫn** (`/`, `usr`, `bin`, `vim` — mỗi cái một dentry) | d_compare, d_delete | KHÔNG có bản on-disk — sinh khi phân giải path; 3 trạng thái used/unused/negative |
| **file** (tr. 279) | File **đang mở bởi một process** (góc nhìn + offset + flags) | read, write, mmap, llseek | Ứng fd; nhiều file object → 1 dentry → 1 inode |

- **Dentry cache (tr. 276)**: phân giải path (`/usr/bin/vim` = walk từng cấp, mỗi cấp một lần lookup) đắt → cache dentry (kể cả **negative dentry** — "tên này KHÔNG tồn tại", tăng tốc open fail lặp lại); dcache kéo icache theo (dentry pin inode). Đây chính là phần "slab dentry/inode phình to chiếm RAM" — cache thu hồi được, không phải leak.
- Chuỗi liên kết khi `read(fd, ...)`: fd → `file` (của process, giữ **f_pos**) → dentry → inode → `inode->i_fop->read` của FS thật → page cache (cụm 3).
- `files_struct` (bảng fd per-process), `fs_struct` (cwd/root), namespace (tr. 286) — cái được chia sẻ/copy theo cờ clone ([process-sched-syscalls.md](process-sched-syscalls.md)).

### Góc interview

**Câu 1:** Phân biệt file object, dentry, inode. Hai process cùng mở một file thì cái gì chung, cái gì riêng?

<details><summary>Đáp án</summary>

- **inode** = bản thân file (metadata + nơi trỏ dữ liệu) — **một** cho mỗi file bất kể ai mở; **dentry** = một mắt xích tên trên đường dẫn (cache phân giải path — nhiều tên/hard link → nhiều dentry cùng trỏ một inode); **file object** = "phiên mở": offset (f_pos), flags (O_APPEND...), mode — mỗi lần `open()` một cái.
- Hai process mở cùng file: **chung inode** (và thường chung dentry), **riêng file object** → offset độc lập (ai đọc nấy tiến). Ngoại lệ phải nêu: **fork/dup** thì fd của cha con trỏ **cùng file object** → **chung offset** — nguồn của hành vi ghi xen kẽ nối tiếp nhau sau fork, và là cơ chế shell redirect hoạt động ([OSTEP](../ostep/virtualization-cpu.md)).
- Chuỗi đầy đủ: fd —(bảng fd per-process)→ file → dentry → inode → dữ liệu/page cache. Trả lời vẽ được chuỗi này là đạt.

</details>

---

## Cụm 2 — Block I/O Layer (ch. 14, tr. 289–304)

### Nội dung chính

- Block device = truy cập ngẫu nhiên theo **sector** (512B) / block FS (tr. 290); `struct bio` (tr. 294) — một thao tác I/O đang bay: **vector các đoạn (page, offset, len)** — scatter-gather ngay trong cấu trúc (một bio ghi được các trang không liền nhau); buffer_head cũ chỉ mô tả một block (tr. 291, 296 — legacy).
- **Request queue + I/O scheduler (tr. 297–304):** không đưa thẳng bio xuống driver — xếp hàng để **merge** (gộp bio liền kề thành request lớn) và **sort** (xếp theo vị trí đĩa — giảm seek):
  - Linus Elevator (tr. 299): merge+sort thuần — đói ở góc xa;
  - **Deadline** (tr. 300): thêm hạn chót mỗi request (đọc 500ms, ghi 5s — **đọc ưu tiên** vì process thường *chờ đồng bộ* kết quả đọc, còn ghi đã buffer) — chống đói;
  - Anticipatory (tr. 302): đoán "sắp có đọc kế tiếp gần đây" nên nán lại — chết theo thời đại;
  - **CFQ** (tr. 303): chia băng thông công bằng theo process — default desktop thời sách;
  - Noop (tr. 303): chỉ merge — cho thiết bị không seek penalty.

### ⚠️ Đã thay đổi so với sách

- Toàn bộ single-queue elevator đã thay bằng **blk-mq (multi-queue, 5.0)**: hàng đợi per-CPU + hardware queue (NVMe nhiều queue thật) — scheduler nay: **mq-deadline, BFQ** (fair, desktop/rotational), **kyber**, **none** (NVMe nhanh: không schedule gì là nhanh nhất). CFQ/anticipatory/legacy elevator **đã xóa**.
- Ý còn nguyên giá trị: **merge/sort/chống đói/ưu tiên đọc** — và với eMMC/SD embedded (một queue, chậm): scheduler vẫn quan trọng (`mq-deadline`/`bfq` — kiểm tra `/sys/block/mmcblk0/queue/scheduler`).

### Góc interview

**Câu 1:** Vì sao I/O scheduler ưu tiên đọc hơn ghi? Điều này còn đúng trên eMMC/SSD không?

<details><summary>Đáp án</summary>

- **Đọc thường đồng bộ với tiến độ chương trình**: process `read()` là **đứng chờ dữ liệu** mới đi tiếp (chuỗi phụ thuộc: đọc block này xong mới biết đọc gì tiếp — vd walk inode → indirect block → data); còn **ghi đã được page cache buffer** — writeback chạy nền, process ghi xong `write()` là đi tiếp từ lâu ([cụm 3](#cụm-3--page-cache--writeback-ch-16-tr-323336)). Trễ một request đọc = trễ người thật; trễ ghi = chỉ trễ daemon nền → deadline scheduler đặt hạn đọc 500ms vs ghi 5s.
- Trên eMMC/SSD: hết seek penalty nên phần **sort** mất giá, nhưng **ưu tiên đọc vẫn đúng** (bản chất "đọc = có người chờ" không đổi) và **merge vẫn lợi** (ít lệnh lớn tốt cho FTL hơn nhiều lệnh vụn — [write amplification](../ostep/persistence.md)); thêm yếu tố mới: ghi vào SLC cache của eMMC nhanh nhưng GC nền gây giật — scheduler công bằng (BFQ) giúp app latency-sensitive. NVMe nhanh nhiều queue → `none` (chi phí schedule > lợi ích).
- Điểm cộng: biết fsync/O_SYNC biến ghi thành đồng bộ — khi đó ghi cũng "có người chờ" và các FS/scheduler hiện đại có đường ưu tiên riêng (REQ_SYNC).

</details>

---

## Cụm 3 — Page Cache & Writeback (ch. 16, tr. 323–336)

### Nội dung chính

- **Page cache = cache của mọi I/O file theo trang**: `read` trước tiên tra cache (miss → đọc đĩa vào cache rồi copy cho user); `write` = **write-back**: sửa trang trong cache + đánh dấu **dirty** — chưa đụng đĩa (tr. ~327). RAM trống "được tiêu hết" vào page cache là **thiết kế đúng** (free thấp ≠ hết RAM — cache thu hồi ngay khi cần).
- Tra cứu: mỗi file một `address_space` (tên gây nhầm — thực ra là "page cache của inode này") + radix tree tìm trang theo offset (⚠️ nay là **xarray**).
- **Writeback — khi nào dirty page xuống đĩa:** (1) free memory dưới ngưỡng (reclaim cần trang sạch); (2) dirty **quá già** (`dirty_expire_centisecs` ~30s); (3) app gọi **fsync/sync**. Thời sách: **pdflush → per-BDI flusher threads** (tr. ~331 — mỗi thiết bị một luồng ghi, tránh một đĩa chậm chặn cả hệ; ⚠️ nay là writeback workqueue per-BDI — cùng ý tưởng). Tham số `vm.dirty_ratio/dirty_background_ratio`: vượt background → ghi nền; vượt ratio cứng → **process ghi bị bắt ghi đồng bộ** (throttle — nguồn "copy file lớn làm cả hệ khựng").
- Nối vòng về [OSTEP crash consistency](../ostep/persistence.md): write() OK ≠ trên đĩa — chính page cache là lý do; fsync là cây cầu.

### Góc interview

**Câu 1:** Thiết bị embedded của bạn ghi log đều đặn; thỉnh thoảng mọi thao tác khựng 1–2 giây. Nghi phạm page cache/writeback — giải thích cơ chế và cách trị.

<details><summary>Đáp án</summary>

- Cơ chế: log ghi vào **page cache** (nhanh, không chạm flash) → dirty tích lũy; chạm ngưỡng (`dirty_background_ratio` → ghi nền; tệ hơn: `dirty_ratio` → **throttle mọi process đang ghi**, bắt ghi đồng bộ) hoặc dirty quá già → writeback **xả cục lớn xuống eMMC/SD chậm** → storage bận nghẽn, mọi thao tác đụng I/O (kể cả đọc — vì queue đầy) khựng theo. Flash rẻ còn tệ hơn: GC nội bộ nổ đúng lúc xả.
- Trị theo tầng:
  1. **Xả đều thay vì xả cục**: giảm `dirty_background_bytes` (dùng *_bytes thay *_ratio trên thiết bị ít RAM — ratio % của RAM to là cục quá lớn), giảm `dirty_expire_centisecs` — trả giá ghi thường xuyên hơn (cân với tuổi thọ flash).
  2. **Cô lập log**: partition/thiết bị riêng cho log (BDI riêng — writeback không chặn nhau), hoặc log vào tmpfs + flush chu kỳ có kiểm soát ([FILE 4 câu D3](../../15_prep/technical_round/04_bsp_question_bank.md)).
  3. App chủ động: `fsync` theo lô nhỏ đều đặn (tự trải), `fdatasync` thay fsync, O_DIRECT cho stream lớn (bỏ qua cache — cân nhắc).
  4. Đo để xác nhận: `/proc/meminfo` (Dirty/Writeback), `iostat -x` (util, await spike), tracepoint writeback.
- Điểm chấm: giải thích được **throttle tại dirty_ratio** (không chỉ "đĩa bận") và trade-off với tuổi thọ flash.

</details>

### Đọc thêm (tùy chọn)

- [ostep/persistence.md](../ostep/persistence.md) — inode/journaling/FTL: nửa lý thuyết.
- [04/file-io.md](../../04-linux-system-programming/file-io.md) — fd, fsync mức API.
