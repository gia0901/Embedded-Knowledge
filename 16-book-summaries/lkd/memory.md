# LKD — Kernel Memory & Process Address Space (ch. 12 tr. 231, ch. 15 tr. 305) 🎯

> Thuộc [LKD](README.md). Nguồn: kiến thức Claude, số trang đối chiếu PDF 3rd ed.

---

## Cụm 1 — Cấp phát bộ nhớ TRONG kernel (ch. 12, tr. 231–260) 🎯

### Nội dung chính

**Tầng thấp nhất — page & zone (tr. 231–235):** đơn vị cấp phát gốc là **page frame** (`struct page` cho từng frame vật lý); RAM chia **zone** theo ràng buộc phần cứng: `ZONE_DMA` (thiết bị cũ chỉ DMA được vùng thấp), `ZONE_NORMAL`, `ZONE_HIGHMEM` (32-bit: vùng kernel không map thường trực). `alloc_pages(gfp, order)` cấp 2^order trang **liền vật lý** (buddy allocator — [OSTEP](../ostep/virtualization-memory.md)); `__get_free_pages`, `get_zeroed_page` (tr. 236 — zero để không rò dữ liệu sang userspace).

**kmalloc & GFP flags (tr. 238–243)** — cấp phát thường ngày của driver: trả vùng **liền vật lý**, tối đa cỡ vài MB. `gfp_mask` là câu hỏi phỏng vấn kinh điển:

| Flag | Nghĩa | Dùng khi |
|---|---|---|
| **GFP_KERNEL** | Được phép **ngủ** (chờ reclaim/I/O để lấy trang) — cấp phát "cố hết sức" | Process context — **mặc định** |
| **GFP_ATOMIC** | **Không ngủ** — lấy được thì lấy ngay, dễ fail hơn (dùng cả pool dự trữ) | ISR, softirq/tasklet, đang giữ spinlock |
| GFP_DMA / GFP_DMA32 | Ép zone thấp | Thiết bị DMA giới hạn địa chỉ (thời DT: dùng DMA API đúng hơn — [MELP C2](../../15_prep/technical_round/04_bsp_question_bank.md)) |
| __GFP_ZERO | Trang zero sẵn | `kzalloc` = kmalloc + zero |

**vmalloc (tr. 244):** trả vùng **liền ảo, không cần liền vật lý** — cấp vùng lớn khi vật lý phân mảnh; giá: phải sửa page table + mỗi trang một mapping (TLB tốn hơn), **không dùng cho DMA thường** (thiết bị cần liền vật lý/địa chỉ bus). Quy tắc sách: **mặc định kmalloc; vmalloc chỉ khi vùng lớn và chỉ CPU đụng** (buffer nạp firmware, module text).

**Slab layer (tr. 245–251):** giữa buddy (cấp page) và người dùng: **cache các object cùng kích thước** — mỗi loại object hay cấp/thu (task_struct, inode, dentry...) một `kmem_cache`; lấy/trả object đã khởi tạo sẵn, không đập vào buddy mỗi lần, chống phân mảnh, tận dụng cache CPU. Driver có object riêng cấp/thu dày đặc → `kmem_cache_create` + `kmem_cache_alloc`. kmalloc chính là bộ slab cache theo size (32, 64, 128...). Nhìn thấy được: `/proc/slabinfo`, `slabtop`.

**Kernel stack nhỏ, cố định (tr. 252):** 1–2 page/thread (⚠️ nay 16KB trên x86-64/ARM64, có `CONFIG_VMAP_STACK` — stack từ vmalloc + guard page để tràn nổ ngay thay vì ghi đè thầm lặng) — hệ quả: **không mảng to/đệ quy sâu trong kernel**; cấp lớn thì kmalloc.

**High memory (tr. 253) — ⚠️ chuyện 32-bit:** kernel 32-bit chỉ map thường trực ~896MB đầu; trang cao hơn phải `kmap/kunmap` tạm. **ARM64/x86-64: RAM map thẳng hết, HIGHMEM biến mất** — đọc để hiểu hệ 32-bit cũ (nhiều thiết bị nhúng đời trước vẫn chạy).

**Per-CPU (tr. 255–259):** đã phân tích ở [sync-timers.md](sync-timers.md) — cấp phát bằng `alloc_percpu`, truy cập qua `get_cpu_var/put_cpu_var`.

### Insight đáng nhớ

- Cây quyết định cấp phát (tr. 259 đóng chương đúng bằng câu này): **liền vật lý (DMA/phần cứng)? → kmalloc/alloc_pages. Chỉ CPU đụng + lớn? → vmalloc. Object lặp đi lặp lại? → kmem_cache. Ngủ được không quyết định GFP flag.** Bốn câu phủ 99% quyết định thực tế.
- Điểm nối BSP đắt giá: kmalloc liền vật lý *nhưng* DMA còn cần **cache coherency + địa chỉ bus** — kmalloc xong vẫn phải qua **DMA API** (`dma_map_single`...), không đưa thẳng `virt_to_phys` cho thiết bị (xem [FILE 4 câu C2](../../15_prep/technical_round/04_bsp_question_bank.md)).

### ⚠️ Đã thay đổi so với sách

- Slab có ba hiện thân: SLAB/SLUB/SLOB — **SLUB là mặc định** từ lâu, SLOB/SLAB đã bị gỡ (6.4/6.8); API `kmem_cache_*` giữ nguyên.
- Kernel stack: 16KB + VMAP_STACK (4.9) như ghi trên; `kvmalloc` (4.12) = thử kmalloc, to quá rơi về vmalloc — tiện cho buffer "thường nhỏ đôi khi to".
- HIGHMEM chỉ còn di sản 32-bit.

### Góc interview

**Câu 1 (🎯):** kmalloc vs vmalloc — khác nhau thế nào, khi nào bắt buộc dùng cái nào? Vì sao buffer DMA không dùng vmalloc?

<details><summary>Đáp án</summary>

- **kmalloc**: liền **cả ảo lẫn vật lý** (cắt từ slab trên trang liền buddy cấp), nhanh (không sửa page table — dùng direct mapping có sẵn), giới hạn cỡ vài MB và fail khi vật lý phân mảnh. **vmalloc**: liền **ảo thôi** — gom trang vật lý rời rạc, sửa page table từng trang; cấp được vùng lớn kể cả khi phân mảnh; chậm hơn khi cấp + tốn TLB hơn khi dùng.
- Bắt buộc kmalloc (hoặc alloc_pages/CMA): mọi thứ **phần cứng đụng vào** — DMA buffer, descriptor ring; vì thiết bị/DMA engine làm việc với **địa chỉ bus/vật lý và không đi qua MMU của CPU**: vùng vmalloc liền ảo nhưng vật lý rời rạc → thiết bị ghi "buffer + offset" sẽ rơi vào trang vật lý chẳng liên quan. (Ngoại lệ đúng chuẩn: thiết bị sau **IOMMU/SMMU** có thể scatter-gather — nhưng khi đó dùng DMA API với sg-list, vẫn không phải vmalloc trần.)
- Bắt buộc vmalloc: vùng lớn chỉ CPU đọc/ghi khi kmalloc fail vì phân mảnh (firmware blob, bảng lớn); module text load bằng vmalloc.
- Điểm cộng: `kvmalloc` cho trường hợp lưỡng lự; và kmalloc trong ISR phải `GFP_ATOMIC`.

</details>

**Câu 2:** GFP_KERNEL vs GFP_ATOMIC khác gì? Chuyện gì xảy ra "bên trong" khi cấp phát GFP_KERNEL lúc RAM căng?

<details><summary>Đáp án</summary>

- **GFP_KERNEL**: allocator được phép **ngủ để kiếm trang** — chạy direct reclaim (đuổi page cache, ghi trang dirty xuống đĩa — tức có thể **chờ I/O**), thậm chí kích OOM killer; gần như luôn thành công nhưng thời gian không chặn trên → **chỉ process context**, không cầm spinlock.
- **GFP_ATOMIC**: không bao giờ ngủ — lấy từ trang trống sẵn có (kể cả phần dự trữ dưới watermark min); RAM căng là **fail thẳng** → code phải xử lý NULL tử tế (drop packet có đếm, retry sau) — dùng trong ISR/softirq/đang giữ spinlock.
- Bên trong GFP_KERNEL lúc căng: thử free list → thiếu thì wake kswapd (reclaim nền) → vẫn thiếu thì **direct reclaim** ngay trên đường cấp phát (nguồn latency spike kinh điển: một `kmalloc` vô tội mất hàng chục ms vì đi ghi dirty page hộ hệ thống!) → compaction (ghép trang liền cho order cao) → OOM. Hiểu chuỗi này giải thích: vì sao hệ "gần đầy RAM chạy giật cục", vì sao RT path phải pre-allocate ([melp/debug-realtime.md](../melp/debug-realtime.md)).
- Bẫy khi trả lời: nói "GFP_ATOMIC ưu tiên cao nên tốt hơn" — sai; nó *kém tin cậy hơn* (fail dễ) và ăn vào pool khẩn cấp — chỉ dùng khi không còn lựa chọn ngủ.

</details>

**Câu 3:** Slab allocator tồn tại để giải quyết gì mà buddy không giải quyết được?

<details><summary>Đáp án</summary>

- Buddy cấp **theo trang** (4KB, 8KB...): xin 100 byte lấy 4KB — internal fragmentation khổng lồ; và cấp/thu dày đặc cùng loại object (task_struct mỗi fork, inode/dentry mỗi lần mở file) đập liên tục vào buddy vừa chậm vừa nát.
- Slab chen giữa: xin **1 trang lớn** từ buddy rồi **cắt thành N object cùng cỡ** thành cache; alloc/free = lấy/trả object trong danh sách — O(1), không đụng buddy, object có thể giữ trạng thái khởi tạo (constructor), có per-CPU freelist (nóng cache, không lock). Kết quả: cấp object nhỏ nhanh, ít phân mảnh, thân thiện cache CPU.
- kmalloc là ứng dụng số 1 của chính nó: dãy cache kmalloc-32/64/.../8k. Quan sát: `slabtop` — khi "RAM biến đâu mất" trên thiết bị, slab (dentry/inode cache phình) là nghi phạm thường trực, và đó là **cache thu hồi được**, không phải leak — phân biệt này ăn điểm ([FILE 4 câu H4](../../15_prep/technical_round/04_bsp_question_bank.md)).

</details>

---

## Cụm 2 — Process Address Space nhìn từ kernel (ch. 15, tr. 305–322)

### Nội dung chính

- **mm_struct (tr. 306):** sổ địa chính address space của process — danh sách VMA, page table gốc (pgd), start/end của code/data/heap/stack, số người dùng (thread chung mm — chính **định nghĩa thread trong Linux**: `clone(CLONE_VM)` = chung mm_struct; **kernel thread không có mm** (tr. 309) — mượn mm của task trước đó (active_mm) vì chỉ đụng phần kernel).
- **VMA — vm_area_struct (tr. 309–314):** mỗi **vùng** liên tục cùng thuộc tính một VMA: code (r-x, file-backed), data (rw-, file), heap (rw-, anonymous), stack, mỗi mmap một cái — chính là từng dòng `/proc/<pid>/maps`. Cờ VM_READ/WRITE/EXEC/SHARED (tr. 311); tra cứu bằng **red-black tree** (`find_vma` tr. 316 — ⚠️ nay là **maple tree**, 6.1).
- **Page fault dùng VMA làm trọng tài**: địa chỉ fault ∈ VMA nào? → hợp lệ (demand paging/COW/grow stack — sửa map) hay **SIGSEGV** (không thuộc VMA nào/sai quyền) — đây là "sổ sách của OS" trong câu minor/major/segfault của [OSTEP](../ostep/virtualization-memory.md).
- **mmap/do_mmap (tr. 318):** tạo/mở rộng VMA (file-backed hay anonymous); munmap gỡ. malloc lớn → mmap anonymous chính là tạo VMA — nối mạch với [OSTEP câu malloc/brk](../ostep/virtualization-memory.md).
- **Page tables (tr. 320):** 3 cấp thời sách (PGD→PMD→PTE; ⚠️ nay 4–5 cấp) — kernel sửa bảng, MMU đọc; TLB flush khi sửa. Chi tiết lý thuyết đã có ở OSTEP — ở đây chỉ cần map tên struct với khái niệm.

### Insight đáng nhớ

- Chuỗi trả lời chuẩn "điều gì xảy ra khi `malloc` 1GB rồi chạm trang đầu": malloc → mmap anonymous → **chỉ tạo VMA** (sổ sách, chưa có trang nào) → chạm trang → page fault → kernel thấy địa chỉ ∈ VMA hợp lệ → cấp trang zero, sửa PTE → chạy tiếp. VMA là mắt xích sách này bổ sung cho bức tranh OSTEP.
- `/proc/<pid>/maps` = danh sách VMA in ra — kỹ năng đọc nó (đâu là heap, đâu là .so nào, guard page) là kỹ năng debug thực chiến ([09/memory-bugs.md](../../09-debugging/memory-bugs.md)).

### Góc interview

**Câu 1:** Kernel thread khác user thread ở điểm nào về bộ nhớ? `current->mm == NULL` nghĩa là gì?

<details><summary>Đáp án</summary>

- **Kernel thread** (kworker, ksoftirqd, kswapd...): chạy hoàn toàn trong kernel space, **không có address space user** → `mm = NULL`. Khi chạy, nó **mượn** `active_mm` của task vừa chạy trước trên CPU đó — vì phần kernel của mọi page table giống hệt nhau, mượn ai cũng được, đỡ một lần đổi page table (đỡ TLB flush).
- Hệ quả: kernel thread không thể `copy_to_user` tùy tiện (không có user mapping của "ai" cụ thể), không có VMA/heap/stack user; tạo bằng `kthread_create/kthread_run`, hiện trong ps với tên `[ngoặc vuông]`.
- User thread: `clone(CLONE_VM)` — **chung mm_struct** với thread anh em (chung VMA, chung page table — đúng định nghĩa "thread = chung address space" của [OSTEP](../ostep/concurrency.md)), riêng stack (một VMA riêng) + registers.
- Điểm cộng: câu này kiểm tra hiểu "Linux không phân biệt process/thread — chỉ là task_struct chia sẻ nhiều hay ít qua cờ clone".

</details>

### Đọc thêm (tùy chọn)

- [ostep/virtualization-memory.md](../ostep/virtualization-memory.md) — paging/TLB/swap: nửa lý thuyết của cùng bức tranh.
- [01/memory-model.md](../../01-cpp-fundamentals/memory-model.md), [09/memory-bugs.md](../../09-debugging/memory-bugs.md).
