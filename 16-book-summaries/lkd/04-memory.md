# Ch. 12 & 15 — Memory Management & Process Address Space (tr. 231–260, 305–322)

> Thuộc [LKD](README.md) · **[⏮ 03 Sync & Timers](03-sync-timers.md)** · **[05 VFS/Block/Page Cache → ⏭](05-vfs-block-pagecache.md)**
> Nguồn: **đọc trực tiếp PDF** (Linux Kernel Development, Robert Love, 3rd ed. 2010), **trang sách = trang PDF − 27**.
> Ký hiệu: không đánh dấu = nội dung sách · **🆕 = bổ sung/liên hệ ngoài sách** · **⚠️ = cần cẩn trọng / sách lỗi thời** · **🎯 = câu hỏi phỏng vấn kinh điển** · trích dẫn kèm `(tr. X)`.
> Hai chương này gộp vì cùng chủ đề bộ nhớ, hai chiều đối xứng: **ch. 12** = kernel **xin bộ nhớ cho chính nó thế nào** (page/zone/kmalloc/slab — bạn gọi hằng ngày khi viết driver); **ch. 15** = kernel **quản address space của process user thế nào** (mm_struct/VMA/page table — cái đứng sau mọi `mmap`, `malloc`, Segmentation Fault). Câu mở đầu của cả cụm (tr. 231): *"the kernel is not always afforded the capability to easily allocate memory... the kernel cannot easily deal with memory allocation errors, and the kernel often cannot sleep"* — đây là **lý do gốc** mọi thứ ở đây khác userspace.

---

## Cụm 1 — Cấp phát bộ nhớ trong kernel (ch. 12, tr. 231–260)

### 1.1 Page — đơn vị cơ bản & `struct page` (tr. 231–233)

**Bức tranh:** kernel quản bộ nhớ theo **page vật lý** (không phải byte). Vì **MMU** (memory management unit — phần cứng dịch địa chỉ ảo→vật lý) làm việc theo trang → page table cũng theo trang. Kích thước trang tùy arch: **32-bit thường 4KB, 64-bit thường 8KB** (tr. 231). Máy 4KB-page + 1GB → **262.144 trang**.

Mỗi trang vật lý có một **`struct page`** (tr. 232):
```c
struct page {
        unsigned long        flags;      // trạng thái trang: dirty? locked?
        atomic_t             _count;     // usage count — dùng page_count() để đọc (0 = free)
        atomic_t             _mapcount;
        struct address_space *mapping;   // nếu thuộc page cache → trỏ tới address_space
        void                 *virtual;   // địa chỉ ảo; NULL nếu là high memory chưa map
        ...
};
```
Điểm cốt lõi (tr. 232): *"the page structure is associated with physical pages, not virtual pages"* — nó mô tả **trang vật lý**, không phải dữ liệu trong đó (dữ liệu có thể bị swap sang trang khác). 🆕 Người mới hay hoảng "cấp một `struct page` cho *mỗi* trang — phí RAM quá!" — sách tính (tr. 232): 4GB, trang 8KB, `struct page` ~40 byte → **~20MB** = *"only a small fraction of a percent"* của 4GB. Không đắt.

### 1.2 Zones — vì sao không phải trang nào cũng như nhau (tr. 233–235)

Phần cứng có **hai giới hạn** buộc kernel phải phân loại trang (tr. 233):
1. Vài thiết bị **DMA** chỉ tới được **một dải địa chỉ nhất định**;
2. Vài arch **địa chỉ vật lý được nhiều hơn địa chỉ ảo** → có vùng nhớ **không map thường trực** vào kernel.

→ Linux chia trang thành **zone** (nhóm logic, không phải phân vùng vật lý):

| Zone | Là gì | x86-32 |
|---|---|---|
| **ZONE_DMA** | Trang DMA được | < 16MB (ISA chỉ tới 16MB đầu) |
| **ZONE_DMA32** | DMA cho thiết bị 32-bit | (tùy arch) |
| **ZONE_NORMAL** | Trang bình thường, map thẳng | 16–896MB |
| **ZONE_HIGHMEM** | "High memory" — **không map thường trực** vào kernel | > 896MB |

⚠️ Layout tùy arch (tr. 234): **x86-64** map được cả 64-bit → **không có ZONE_HIGHMEM** (mọi thứ trong DMA/NORMAL). Cấp phát **không vượt biên zone**, kernel ưu tiên lấy từ NORMAL để dành DMA cho ai cần. 🆕 `high memory > 896MB` là chuyện đặc thù **x86-32** — trên 64-bit hiện đại gần như vô nghĩa, nhưng khái niệm "vùng không map thường trực, phải `kmap`" vẫn còn (mục 1.7).

### 1.3 Xin trang: `alloc_pages` & bạn bè (tr. 235–237)

Hàm lõi (khai trong `<linux/gfp.h>`), cấp theo **page-sized**:
```c
struct page *alloc_pages(gfp_t gfp_mask, unsigned int order);  // cấp 2^order trang liền nhau
void *page_address(struct page *page);                          // page → địa chỉ logic
unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order);  // như trên, trả thẳng địa chỉ logic
unsigned long __get_free_page(gfp_t gfp_mask);                  // 1 trang (order=0)
unsigned long get_zeroed_page(unsigned int gfp_mask);           // 1 trang ĐÃ xóa về 0
```
⚠️ **`get_zeroed_page` quan trọng cho an ninh (tr. 236):** trang cấp cho user **phải** xóa 0 trước — *"the random garbage in an allocated page is not so random; it might contain sensitive data"* (rác có thể là dữ liệu nhạy cảm của process khác). Ví dụ cấp 8 trang + kiểm lỗi (tr. 237):
```c
unsigned long page = __get_free_pages(GFP_KERNEL, 3);   // 2^3 = 8 trang
if (!page) return -ENOMEM;      // ⚠️ cấp phát kernel CÓ THỂ FAIL — luôn phải kiểm!
/* ... dùng ... */
free_pages(page, 3);            // free đúng order đã cấp
```
⚠️ Sách nhấn (tr. 237): *"the kernel trusts itself... the kernel will happily hang itself if you ask it"* — free sai `struct page`/order = corrupt, kernel không kiểm hộ như user.

### 1.4 `kmalloc` — cấp theo byte (tr. 238–239)

Giống `malloc` userspace + tham số `flags`; trả vùng **liền nhau về vật lý** (physically contiguous):
```c
void *kmalloc(size_t size, gfp_t flags);
struct dog *p = kmalloc(sizeof(struct dog), GFP_KERNEL);
if (!p) /* xử lý lỗi */;
```
🆕 `kmalloc` là **interface mặc định** cho hầu hết cấp phát kernel. ⚠️ Nó có thể cấp **hơn** size yêu cầu (làm tròn theo trang) nhưng **không bao giờ ít hơn** (tr. 239).

### 1.5 GFP flags — linh hồn của cấp phát kernel (tr. 239–243)

`gfp` = *get free pages*. Flags chia ba loại (tr. 240): **action modifier** (cấp *như thế nào*), **zone modifier** (cấp *từ đâu*), **type flag** (gộp sẵn action+zone cho một *tình huống*). Bạn hầu như chỉ dùng **type flag**:

| Type flag | Ý nghĩa |
|---|---|
| **GFP_KERNEL** | Cấp bình thường, **có thể ngủ** — dùng ở **process context an toàn để ngủ**. *"This flag should be your default choice."* |
| **GFP_ATOMIC** | **Không được ngủ** — dùng trong **ISR, softirq, tasklet, khi đang giữ spinlock** |
| **GFP_NOIO** | Có thể block nhưng **không khởi I/O đĩa** (code block I/O) |
| **GFP_NOFS** | Có thể I/O đĩa nhưng **không thao tác filesystem** (code filesystem) |
| **GFP_DMA** | Từ **ZONE_DMA** — driver cần bộ nhớ DMA (thường gộp với GFP_KERNEL/ATOMIC) |
| **GFP_USER** / **GFP_HIGHUSER** | Cấp cho process user (HIGHUSER từ ZONE_HIGHMEM) |

**Vì sao GFP_ATOMIC hay fail hơn (tr. 242) — điểm phân loại:** `GFP_KERNEL` có thể **ngủ để giải phóng bộ nhớ** (swap trang inactive ra đĩa, flush dirty page) → khả năng thành công cao. `GFP_ATOMIC` **không ngủ được** → không làm được các việc đó → *"has less of a chance of succeeding (at least when memory is low)"*. Nhưng nó là **lựa chọn duy nhất** khi code không được ngủ (ISR/softirq/tasklet — nối thẳng [02](02-interrupts-bottomhalves.md), [03](03-sync-timers.md)).

**Vì sao GFP_NOFS/GFP_NOIO tồn tại (tr. 242) — chống đệ quy chết:** nếu code filesystem cấp phát mà **không** dùng `GFP_NOFS`, việc cấp phát có thể **khởi thêm thao tác filesystem → cần cấp phát nữa → thêm filesystem op → ... vô hạn / deadlock**. Bảng chọn (tr. 243):

| Tình huống | Flag |
|---|---|
| Process context, ngủ được | `GFP_KERNEL` |
| Process context, không ngủ được | `GFP_ATOMIC` (hoặc cấp trước/sau lúc ngủ được) |
| ISR / softirq / tasklet | `GFP_ATOMIC` |
| Cần DMA, ngủ được | `GFP_DMA \| GFP_KERNEL` |

Free bằng `kfree()` (tr. 243). ✅ `kfree(NULL)` an toàn (được kiểm sẵn).

### 1.6 `vmalloc` vs `kmalloc` (tr. 244–245)

`vmalloc()` cấp bộ nhớ **liền nhau về ẢO nhưng KHÔNG nhất thiết liền nhau về vật lý** — như `malloc` userspace: gom các mảnh vật lý rời rạc rồi "vá" page table cho liền ảo. So sánh:

| | `kmalloc` | `vmalloc` |
|---|---|---|
| Liền vật lý? | **Có** | Không (chỉ liền ảo) |
| Dùng cho | Hầu hết mọi thứ, và **thiết bị (DMA cần liền vật lý)** | Vùng **lớn** không cần liền vật lý (vd nạp **module**) |
| Chi phí | Rẻ | Phải set page table riêng → **TLB thrashing** cao hơn |
| Ngủ? | Tùy flag | **Có thể ngủ** → cấm interrupt context |

🆕 Vì sao thiết bị cần liền vật lý (tr. 244): *"hardware devices live on the other side of the MMU and, thus, do not understand virtual addresses"* — DMA thao tác địa chỉ vật lý thô. Kết luận (tr. 245): dùng `kmalloc` là chính; `vmalloc` **chỉ khi thật cần** (vùng lớn).

### 1.7 Slab layer — bộ cache đối tượng (tr. 245–252)

**Vấn đề (tr. 245):** cấp/thu struct cùng loại **rất thường xuyên** trong kernel → mẫu **free list** (giữ sẵn các struct đã cấp, dùng lại thay vì cấp mới). Nhưng free list tự chế **không có quản lý toàn cục** — kernel không biết để bảo "co lại" khi thiếu RAM. **Slab layer** (từ SunOS, tr. 246) là free list *tổng quát, có quản lý*, dựa trên các nguyên tắc: cache struct hay dùng, xếp liền nhau chống phân mảnh, phần per-CPU khỏi lock SMP, NUMA-aware, **coloring** chống nhiều object trùng cache line.

**Cấu trúc (tr. 246–247):** **cache** (một per loại object, vd `inode_cachep`) → chia thành **slab** (một/vài trang liền nhau) → chứa nhiều **object**. Mỗi slab ở một trong ba trạng thái: **full / partial / empty**. Xin object → lấy từ **partial** trước (giảm phân mảnh), hết thì **empty**, hết nữa mới cấp trang mới.

**Interface (tr. 249–251):**
```c
struct kmem_cache *kmem_cache_create(const char *name, size_t size, size_t align,
                                     unsigned long flags, void (*ctor)(void *));
void *kmem_cache_alloc(struct kmem_cache *cachep, gfp_t flags);   // xin 1 object
void kmem_cache_free(struct kmem_cache *cachep, void *objp);      // trả object
int kmem_cache_destroy(struct kmem_cache *cachep);
```
Cờ đáng nhớ: `SLAB_HWCACHE_ALIGN` (căn cache line — chống **false sharing**), `SLAB_POISON` (đổ giá trị `a5a5a5a5` bắt truy cập chưa init), `SLAB_RED_ZONE` (bắt buffer overrun), `SLAB_PANIC` (fail thì panic).

**Ví dụ thật — `task_struct` (tr. 251):** đây là chỗ [01](01-process-sched-syscalls.md) mục 1.2 nói "task_struct cấp từ slab":
```c
struct kmem_cache *task_struct_cachep;
// fork_init():
task_struct_cachep = kmem_cache_create("task_struct", sizeof(struct task_struct),
                                       ARCH_MIN_TASKALIGN, SLAB_PANIC | SLAB_NOTRACK, NULL);
// dup_task_struct() mỗi lần fork:
tsk = kmem_cache_alloc(task_struct_cachep, GFP_KERNEL);
// free_task_struct() khi task chết:
kmem_cache_free(task_struct_cachep, tsk);
```
🆕 `SLAB_PANIC` ở đây vì cache này **thiết yếu** (không có process descriptor thì máy vô dụng) → khỏi kiểm NULL. `kmalloc` thực chất **xây trên slab** (một họ cache đa dụng — tr. 246). Lời khuyên (tr. 252): tạo/hủy nhiều object cùng loại → **dùng slab cache, đừng tự viết free list!**

### 1.8 Kernel stack nhỏ & cố định (tr. 252–253)

Khác userspace (stack lớn, tự lớn), **kernel stack nhỏ và cố định** — *"When each process is given a small, fixed stack, memory consumption is minimized"* (tr. 252). Lịch sử: **2 trang/process** (8KB 32-bit, 16KB 64-bit). Từ 2.6 có tùy chọn **1 trang** (4KB/8KB) — vì (a) tiết kiệm, (b) *"as uptime increases, it becomes increasingly hard to find two physically contiguous unallocated pages"* (RAM phân mảnh). Kèm theo: **interrupt stack** riêng (một trang/CPU) để ISR không còn ngốn stack của process bị ngắt ([02](02-interrupts-bottomhalves.md) mục 1.2).

⚠️ **Hệ quả (tr. 253):** stack tràn xảy ra **im lặng** → tràn vào `thread_info` ở đáy stack, hoặc dữ liệu kernel bất kỳ → *"At best, the machine will crash... At worst, the overflow will silently corrupt data."* → Giữ tổng biến local **vài trăm byte**, **không mảng/struct lớn trên stack, không đệ quy vô hạn**, không `alloca()` — dùng cấp phát động.

### 1.9 High memory & per-CPU (tr. 253–259)

**High memory mapping (tr. 253–254):** trang high memory **không có địa chỉ logic thường trực** → phải map tạm:
- `kmap(page)` — map **thường trực** (high→địa chỉ logic); **có thể ngủ** → chỉ process context; số slot có hạn nên xong phải `kunmap()`.
- `kmap_atomic(page, type)` — map **tạm/atomic**, **không block** → **dùng được trong ISR**; tắt kernel preemption (map là per-CPU). `kunmap_atomic()`.

**Per-CPU (tr. 255–259):** dữ liệu **riêng mỗi CPU** → không chia sẻ → **không cần lock**. Cách cũ (mảng) và interface mới:
```c
DEFINE_PER_CPU(int, my_count);       // biến per-CPU compile-time
get_cpu_var(my_count)++;             // truy cập bản của CPU hiện tại + TẮT preemption
put_cpu_var(my_count);               // bật lại preemption
// động (dùng được trong module):
void *ptr = alloc_percpu(unsigned long);
foo = get_cpu_var(ptr);  /* ... */  put_cpu_var(ptr);
free_percpu(ptr);
```
**Vì sao vẫn phải `get_cpu_var`/`put_cpu_var` (tr. 256):** hai nguy cơ khi preempt — (1) bị **dời sang CPU khác** giữa chừng → `cpu` cũ sai; (2) task khác preempt → cùng CPU đụng cùng dữ liệu (race). `get_cpu*` **tắt preemption** khép cả hai lỗ. ⚠️ `smp_processor_id()` **không** tắt preemption — phải dùng `get_cpu*`. Lợi ích lớn (tr. 258): giảm lock + giảm **cache invalidation/thrashing** (một counter global bị N CPU ghi là thảm họa cache coherence) — nhưng **không được ngủ giữa lúc thao tác per-CPU** (kẻo dời CPU).

**Chọn cách nào (tr. 259):** cần **liền vật lý** → `kmalloc`/low-level page (GFP_ATOMIC nếu không ngủ, GFP_KERNEL nếu ngủ được); **high memory** → `alloc_pages` + `kmap`; **chỉ liền ảo, vùng lớn** → `vmalloc`; **nhiều object cùng loại** → slab cache.

### Insight đáng nhớ (cụm 1)

- **GFP_KERNEL vs GFP_ATOMIC** là câu quyết định gói cả chương: `GFP_KERNEL` ngủ được → giải phóng được bộ nhớ khi thiếu → ít fail; `GFP_ATOMIC` không ngủ → hay fail hơn nhưng là lựa chọn **duy nhất** trong atomic context. Bảng "tình huống → flag" (tr. 243) là thứ tra hằng ngày khi viết driver.
- **kmalloc (liền vật lý) vs vmalloc (chỉ liền ảo)** — thiết bị DMA **buộc** liền vật lý vì ở bên kia MMU; vmalloc dành vùng lớn (module). Nhớ được lý do "bên kia MMU" là trả lời trọn câu.
- **Slab = free list có quản lý toàn cục + coloring + per-CPU**; `task_struct`/`inode` là user điển hình. `kmalloc` xây trên slab. Đừng tự viết free list.
- **Kernel stack nhỏ (1–2 trang) + tràn im lặng** → không mảng lớn/đệ quy trên stack. Cùng ràng buộc với ISR ([02](02-interrupts-bottomhalves.md)).

### ⚠️ Đã thay đổi so với sách

- **Slab allocator có ba biến thể:** SLAB (gốc, sách mô tả), **SLUB** (mặc định từ ~2.6.23 — đơn giản hơn, ít metadata, scale tốt hơn), SLOB (siêu nhỏ cho hệ nhúng ít RAM, đã bỏ ~6.4). API `kmem_cache_*`/`kmalloc` **giữ nguyên**, nội tạng khác. `kmem_cache_create` nay có thêm cờ (`SLAB_ACCOUNT`...) và `kmem_cache_create_usercopy`.
- **kmap_atomic** đổi API: bỏ tham số `km_type` (nay `kmap_atomic(page)`); có `kmap_local_page` (bản mới, không tắt preemption). High memory gần như biến mất trên 64-bit.
- **`kvmalloc()`** — thử `kmalloc` trước (liền vật lý, nhanh), fallback `vmalloc` nếu vùng lớn — tiện cho vùng cỡ trung. **`devm_kmalloc`** (managed, tự free theo device). CMA (Contiguous Memory Allocator) cho vùng DMA lớn liền vật lý trên embedded.
- Per-CPU: `this_cpu_*` operations (thao tác per-CPU nguyên tử không cần `get_cpu_var`).

### Góc interview (cụm 1)

**Câu 1 (🎯):** Phân biệt `kmalloc` và `vmalloc`. Khi nào **bắt buộc** dùng cái nào?

<details><summary>Đáp án</summary>

- **`kmalloc` (tr. 238, 244):** trả vùng **liền nhau cả vật lý lẫn ảo**, rẻ, là mặc định. **`vmalloc` (tr. 244):** chỉ **liền ảo** (gom mảnh vật lý rời rạc + vá page table), đắt hơn (**TLB thrashing** vì map từng trang), **có thể ngủ** → cấm interrupt context.
- **Bắt buộc `kmalloc` (liền vật lý)** khi bộ nhớ cấp cho **DMA/thiết bị** — *"hardware devices live on the other side of the MMU and do not understand virtual addresses"* (tr. 244), DMA thao tác địa chỉ vật lý thô nên phải liền vật lý.
- **Nên `vmalloc`** khi cần **vùng lớn** mà chỉ phần mềm dùng (không cần liền vật lý) — ví dụ **nạp module** vào bộ nhớ `vmalloc` (tr. 244); vùng lớn liền vật lý khó kiếm khi RAM phân mảnh.
- **Điểm cộng:** 🆕 hiện đại có `kvmalloc()` (thử kmalloc rồi fallback vmalloc), và **CMA/dma_alloc_coherent** cho buffer DMA lớn liền vật lý trên embedded.

</details>

**Câu 2 (🎯):** Trong ISR bạn cần cấp bộ nhớ — dùng flag gì? Vì sao `GFP_ATOMIC` **dễ fail hơn** `GFP_KERNEL`?

<details><summary>Đáp án</summary>

- **`GFP_ATOMIC`** (tr. 242) — vì ISR **không được ngủ** ([02](02-interrupts-bottomhalves.md)); `GFP_KERNEL` có thể ngủ → cấm trong ISR/softirq/tasklet/khi giữ spinlock.
- **Vì sao ATOMIC dễ fail:** khi thiếu RAM, `GFP_KERNEL` **được ngủ để giải phóng bộ nhớ** — swap trang inactive ra đĩa, flush dirty page, reclaim — rồi mới trả về → khả năng thành công cao. `GFP_ATOMIC` **không làm được các việc đó** (đều có thể ngủ) → *"has less of a chance of succeeding at least when memory is low"* (tr. 242). Nó chỉ vét được từ pool khẩn cấp.
- **Hệ quả thực hành:** đường nóng cần cấp phát → **cấp trước ở process context** (probe/open, `GFP_KERNEL`) rồi ISR chỉ dùng, tránh `GFP_ATOMIC` trong đường nóng. Luôn **kiểm NULL** dù flag nào (cấp phát kernel có thể fail — tr. 237).

</details>

**Câu 3:** Slab allocator giải quyết vấn đề gì? Vì sao `task_struct` dùng nó?

<details><summary>Đáp án</summary>

- **Vấn đề (tr. 245):** cấp/thu struct cùng loại rất thường xuyên; free list tự chế **không có quản lý toàn cục** (kernel không co được khi thiếu RAM) + gây **phân mảnh**. Slab là free list *tổng quát, có quản lý*: cache theo loại object, slab xếp liền nhau chống phân mảnh, phần **per-CPU khỏi lock**, **coloring** chống trùng cache line.
- **`task_struct` (tr. 251):** `fork()` xảy ra **rất dày**, mỗi lần cần một `task_struct` mới → đúng bài toán slab: `kmem_cache_alloc(task_struct_cachep, ...)` thường trả object có sẵn từ **partial slab** thay vì cấp trang mới → nhanh, ít phân mảnh, cache nóng. Dùng `SLAB_PANIC` vì cache thiết yếu.
- **Điểm cộng:** `kmalloc` **xây trên** slab (họ cache đa dụng); ⚠️ nay mặc định là **SLUB** (nội tạng khác, API như cũ). Cờ debug `SLAB_POISON`/`SLAB_RED_ZONE` bắt use-after-free/overrun.

</details>

---

## Cụm 2 — Process Address Space (ch. 15, tr. 305–322)

### 2.1 Address space & memory area (tr. 305–306)

**Process address space** = bộ nhớ ảo mà một process user thấy — *"as if it alone has full access to the system's physical memory... can be much larger than physical memory"* (tr. 305). Mỗi process một **flat address space** (0..4GB trên 32-bit), **riêng biệt** (địa chỉ `0x4021f000` ở process A không liên quan gì địa chỉ đó ở process B) — trừ khi cố ý chia sẻ: đó là **thread**.

Process **không được đụng cả 4GB** — chỉ các **khoảng hợp lệ** gọi là **memory area** (vd `08048000-0804c000`). Đụng địa chỉ ngoài memory area hợp lệ (hoặc sai quyền) → kernel giết process bằng *"the dreaded 'Segmentation Fault'"* (tr. 306). 🆕 Đây chính là **nguồn gốc kernel của Segfault**. Memory area chứa (tr. 306): **text** (code), **data** (biến global đã init), **bss** (biến global chưa init, map trang-0), **stack user**, text/data/bss của **mỗi shared library** (libc, ld.so), **file mmap**, **shared memory**, **anonymous mapping** (`malloc`). Quan trọng: *"All valid addresses... exist in exactly one area; memory areas do not overlap."*

### 2.2 Memory descriptor — `mm_struct` (tr. 306–309)

Kernel biểu diễn address space bằng **`struct mm_struct`** (nhúng trong `task_struct` qua trường `mm`; `current->mm` là của process hiện tại):
```c
struct mm_struct {
        struct vm_area_struct *mmap;          // DANH SÁCH LIÊN KẾT các memory area
        struct rb_root        mm_rb;          // và RED-BLACK TREE các VMA (cùng dữ liệu, 2 cách)
        pgd_t                 *pgd;           // page global directory (page table gốc)
        atomic_t              mm_users;       // số process/thread ĐANG dùng address space
        atomic_t              mm_count;       // reference count CHÍNH
        int                   map_count;      // số memory area
        struct rw_semaphore   mmap_sem;       // semaphore bảo vệ vùng nhớ
        spinlock_t            page_table_lock;
        unsigned long         start_code, end_code, start_data, end_data;
        unsigned long         start_brk, brk, start_stack;   // heap, stack
        ...
};
```
**`mm_users` vs `mm_count` — hay hỏi (tr. 307):** `mm_users` = số luồng đang dùng (9 thread chia sẻ → `mm_users`=9); `mm_count` = ref count chính, **mọi `mm_users` chỉ tính là MỘT** lần tăng `mm_count`. Khi `mm_users`→0 thì `mm_count` giảm 1; `mm_count`→0 mới **free `mm_struct`**. 🆕 Vì sao hai bộ đếm? Để phân biệt "có process nào dùng không" (`mm_users`) với "kernel còn giữ tham chiếu tạm không" (`mm_count`) — kernel thao tác address space thì bump `mm_count` mà không phải là user.

**`mmap` (list) + `mm_rb` (rbtree) cùng trỏ một tập VMA (tr. 308) — "threaded tree":** list để **duyệt tuần tự** hiệu quả; rbtree để **tìm một VMA theo địa chỉ** (O(log n)). Redundant có chủ đích, tối ưu cả hai thao tác.

**Cấp/hủy (tr. 308–309):** `mm_struct` cấp từ slab `mm_cachep`; `copy_mm()` khi fork — nếu **`CLONE_VM`** thì **không cấp mới**, con **dùng chung `mm` của cha** (`tsk->mm = current->mm`) → đây **chính là thread** ([01](01-process-sched-syscalls.md)). Hủy: `exit_mm` → `mmput` (giảm `mm_users`) → `mmdrop` (giảm `mm_count`) → free.

**Kernel thread & `mm == NULL` (tr. 309):** kernel thread **không có address space** → `mm` = NULL (đây là *định nghĩa* kernel thread — không có user context). Khi được schedule, kernel thấy `mm==NULL` nên **giữ nguyên address space của task chạy trước** và trỏ `active_mm` vào đó (dùng page table của process trước cho phần kernel — chung cho mọi process) → khỏi tốn `mm_struct`/page table riêng, khỏi switch address space. 🆕 Nối [01](01-process-sched-syscalls.md): `mm==NULL` là dấu hiệu kernel thread, và lý do nó không `copy_*_user` được.

### 2.3 VMA — `vm_area_struct` (tr. 309–313)

Mỗi **memory area** = một **`vm_area_struct`** (VMA) — một khoảng **liền nhau** trong address space:
```c
struct vm_area_struct {
        struct mm_struct  *vm_mm;        // mm_struct sở hữu VMA này
        unsigned long     vm_start;      // địa chỉ đầu (inclusive)
        unsigned long     vm_end;        // địa chỉ cuối (EXCLUSIVE) → độ dài = vm_end - vm_start
        struct vm_area_struct *vm_next;  // link trong list
        unsigned long     vm_flags;      // VM_READ/WRITE/EXEC/SHARED...
        struct rb_node    vm_rb;         // node trong rbtree
        struct vm_operations_struct *vm_ops;   // bảng thao tác (OOP kiểu C — như VFS)
        struct file       *vm_file;      // file được map (nếu có)
        ...
};
```
**Khoảng `[vm_start, vm_end)`** — không VMA nào chồng nhau (tr. 310). Mỗi VMA **riêng của một `mm_struct`**: hai process cùng map một file → **mỗi bên một VMA**; hai thread chung address space → **chung mọi VMA**.

**`vm_flags` (tr. 311):** `VM_READ`/`VM_WRITE`/`VM_EXEC` (quyền — text = READ|EXEC, data = READ|WRITE, không EXEC), `VM_SHARED` (shared vs private mapping), `VM_IO` (map I/O space thiết bị — driver set khi `mmap`; loại khỏi core dump), `VM_SEQ_READ`/`VM_RAND_READ` (gợi ý read-ahead — set qua `madvise`).

**`vm_ops` (tr. 312) — OOP kiểu C** (giống VFS — [05](05-vfs-block-pagecache.md)):
```c
struct vm_operations_struct {
        void (*open)(struct vm_area_struct *);    // khi VMA thêm vào address space
        void (*close)(struct vm_area_struct *);   // khi gỡ
        int  (*fault)(struct vm_area_struct *, struct vm_fault *);        // page fault: trang chưa có
        int  (*page_mkwrite)(struct vm_area_struct *, struct vm_fault *); // read-only → writable (COW)
        ...
};
```

### 2.4 VMA trong đời thực: `/proc/<pid>/maps` (tr. 313–315)

Chương trình `int main() { return 0; }` — xem address space thật (tr. 314):
```
00e80000-00faf000 r-xp ... /lib/tls/libc-2.5.1.so     ← text libc (đọc+chạy)
00faf000-00fb2000 rw-p ... /lib/tls/libc-2.5.1.so     ← data libc
00fb2000-00fb4000 rw-p ...                            ← bss libc
08048000-08049000 r-xp ... /home/rlove/src/example    ← text chương trình
08049000-0804a000 rw-p ... /home/rlove/src/example    ← data chương trình
40000000-40015000 r-xp ... /lib/ld-2.5.1.so           ← text linker động
bfffe000-c0000000 rwxp ...                            ← [ stack ]
```
🆕 Đọc ra được nhiều điều (tr. 315): **text** = `r-x` (đọc+chạy, không ghi); **data/bss** = `rw-` (đọc+ghi, không chạy); **stack** = `rwx`. Cả address space ~1340KB nhưng **chỉ 40KB writable/private** — vùng **read-only hoặc shared thì kernel giữ MỘT bản** trong RAM (libc 1212KB nạp một lần, mọi process dùng chung) → tiết kiệm khổng lồ. Vùng không file (device 00:00, inode 0) = **trang-0** map cho bss (ghi vào → COW tạo bản riêng).

### 2.5 Thao tác memory area: `find_vma`, `mmap`/`munmap` (tr. 315–320)

**`find_vma(mm, addr)` (tr. 316):** tìm VMA đầu tiên có `vm_end > addr` (chứa `addr` hoặc bắt đầu sau `addr`). Kết quả **cache** ở `mmap_cache` (hit rate ~30–40% vì thao tác hay lặp trên cùng VMA); miss thì duyệt **rbtree**:
```c
vma = mm->mmap_cache;
if (!(vma && vma->vm_end > addr && vma->vm_start <= addr)) {
        rb_node = mm->mm_rb.rb_node;
        while (rb_node) {                         // duyệt rbtree
                vma_tmp = rb_entry(rb_node, struct vm_area_struct, vm_rb);
                if (vma_tmp->vm_end > addr) {
                        vma = vma_tmp;
                        if (vma_tmp->vm_start <= addr) break;   // tìm thấy VMA chứa addr
                        rb_node = rb_node->rb_left;
                } else rb_node = rb_node->rb_right;
        }
        if (vma) mm->mmap_cache = vma;
}
```

**`do_mmap()` / `mmap()` (tr. 318–320):** thêm một khoảng địa chỉ vào address space — nếu kề VMA cũ **cùng quyền** thì **gộp**, không thì cấp VMA mới từ slab `vm_area_cachep`. File+offset → **file-backed mapping**; không file → **anonymous mapping** (như `malloc`). `munmap()`/`do_munmap()` gỡ khoảng (dưới `down_write(&mm->mmap_sem)` — bảo vệ address space, tr. 320).

### 2.6 Page tables & TLB (tr. 320–322)

Process thao tác **địa chỉ ảo**, CPU thao tác **địa chỉ vật lý** → mỗi truy cập phải **dịch ảo→vật lý** qua **page table**. Linux dùng **3 tầng** (tr. 320) kể cả trên arch chỉ hỗ trợ ít tầng (mẫu số chung, đơn giản hóa bằng compiler):
- **PGD** (Page Global Directory) — tầng đỉnh, `mm_struct->pgd`;
- **PMD** (Page Middle Directory);
- **PTE** (Page Table Entry) — trỏ tới **trang vật lý**.

Mỗi process **page table riêng** (thread chung); thao tác cần `page_table_lock`. Vì mỗi truy cập ảo đều phải dịch → **TLB** (Translation Lookaside Buffer, tr. 321): **cache phần cứng** ánh xạ ảo→vật lý. Truy cập → tra TLB trước; **hit** trả địa chỉ ngay, **miss** mới đi page table. 🆕 Đây là lý do **context switch flush TLB đắt**, và vì sao per-CPU/vmalloc gây TLB thrash. Tương lai (tr. 322): **shared page tables với COW** để fork khỏi copy page table entry.

### Insight đáng nhớ (cụm 2)

- **Segfault có gốc ở đây (tr. 306):** đụng địa chỉ **ngoài mọi VMA hợp lệ** hoặc **sai quyền `vm_flags`** → kernel giết. `/proc/<pid>/maps` cho thấy chính xác các VMA + quyền — công cụ debug segfault số một.
- **`mm_users` vs `mm_count` (tr. 307)** và **`CLONE_VM` → chung `mm`** (tr. 308) là hiện thực cụ thể của "thread = process chia sẻ address space" từ [01](01-process-sched-syscalls.md). `mm==NULL` = kernel thread.
- **VMA read-only/shared → kernel giữ một bản** (tr. 315): libc nạp một lần cho mọi process. Đây là "vì sao nhiều process chạy cùng binary không tốn RAM gấp bội" — và nền của COW.
- **TLB (tr. 321)** là mắt xích hiệu năng: mọi truy cập ảo qua nó; miss đắt; context switch/`vmalloc` gây thrash. Nối [01](01-process-sched-syscalls.md) (`switch_mm` đổi page table) và cụm 1 (vmalloc TLB thrashing).

### ⚠️ Đã thay đổi so với sách

- **Page table nay thường 4 tầng** (x86-64: PGD→P4D→PUD→PMD→PTE, thậm chí 5 tầng với LA57) — sách viết 3 tầng thời 32-bit. Khung khái niệm "chia địa chỉ thành chunk index vào bảng nhiều tầng" vẫn đúng.
- **`mmap_sem` đổi tên `mmap_lock`** (5.8) và bổ sung **per-VMA lock** (6.x — giảm contention khi nhiều thread fault song song). `mm_rb` rbtree được thay bằng **maple tree** (6.1) — `mm_struct` bỏ luôn danh sách `mmap`.
- **THP (Transparent Huge Pages)**, hugetlbfs (trang 2MB/1GB — giảm TLB miss cho DB/JVM lớn), KSM (gộp trang trùng), userfaultfd (xử lý page fault từ userspace) — các bổ sung lớn của quản lý address space hiện đại.

### Góc interview (cụm 2)

**Câu 1 (🎯):** "Segmentation Fault" xảy ra ở tầng kernel như thế nào? `/proc/<pid>/maps` giúp gì?

<details><summary>Đáp án</summary>

- **Gốc (tr. 306):** address space của process chỉ gồm các **VMA** (`vm_area_struct`) — khoảng địa chỉ hợp lệ, **không chồng nhau**, mỗi cái có quyền `vm_flags` (READ/WRITE/EXEC). Process đụng địa chỉ **không nằm trong VMA nào** (page fault handler gọi `find_vma` không thấy) **hoặc** đụng **sai quyền** (ghi vào vùng chỉ READ, chạy vùng không EXEC) → kernel không giải được fault hợp lệ → gửi **SIGSEGV** giết process.
- **`/proc/<pid>/maps` (tr. 314):** liệt kê mọi VMA + quyền + file backing (`r-xp` text, `rw-p` data/bss, `[stack]`...). Debug segfault: đối chiếu **địa chỉ crash** (từ dmesg/core/gdb) với maps xem nó **rơi ngoài mọi vùng** (wild pointer) hay **trong vùng nhưng sai quyền** (vd ghi vào text read-only, hoặc chạy data — dấu hiệu tấn công/hỏng con trỏ hàm).
- **Điểm cộng:** giải thích text=`r-x` không ghi được (bảo vệ code), data=`rw-` không chạy được (NX/W^X chống shellcode); vùng read-only/shared kernel giữ **một bản** RAM (tr. 315).

</details>

**Câu 2:** Hai process chạy cùng một binary. Cái gì trong bộ nhớ được **chung**, cái gì **riêng**? Liên hệ COW.

<details><summary>Đáp án</summary>

- **Mỗi process một `mm_struct` riêng** (address space độc lập) và **page table riêng** (`pgd`) — nhưng **các VMA có thể trỏ tới cùng trang vật lý**.
- **Chung (một bản RAM):** vùng **read-only/shared** — **text của binary + libc/ld.so** (tr. 315): read-only nên nạp **một lần**, mọi process map chung (libc 1212KB không nhân lên theo số process). Đây là lý do chạy 100 process cùng binary không tốn RAM gấp 100.
- **Riêng:** **data/bss/heap/stack** — writable, mỗi process bản riêng. Ban đầu **COW**: fork xong cha con chung trang read-only ([01](01-process-sched-syscalls.md)); ai **ghi** thì `page_mkwrite`/fault tạo **bản riêng** (bss map trang-0, ghi vào → copy — tr. 315).
- **Điểm cộng:** hai thread (`CLONE_VM`) thì **chung cả `mm_struct` + mọi VMA + page table** (tr. 310) — khác hẳn hai process; đây là ranh giới thread/process ở tầng bộ nhớ.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [01-process-sched-syscalls.md](01-process-sched-syscalls.md) — `task_struct` (cấp từ slab), fork/COW, kernel thread `mm==NULL`, `switch_mm`; [03-sync-timers.md](03-sync-timers.md) — per-CPU/`get_cpu`, vì sao `GFP_ATOMIC` khi giữ spinlock.
- [02-interrupts-bottomhalves.md](02-interrupts-bottomhalves.md) — interrupt stack, `GFP_ATOMIC` trong ISR.
- [ostep/virtualization-memory.md](../ostep/virtualization-memory.md) — nền lý thuyết paging/TLB/COW nhìn từ góc OS; [05-drivers-device-tree/](../../05-drivers-device-tree/) — `mmap` driver, DMA buffer.

**Chương tiếp theo:** [05 — VFS, Block Layer & Page Cache →](05-vfs-block-pagecache.md) (bốn object VFS, bio/I/O scheduler, page cache & writeback — nơi `struct page`/`address_space` gặp filesystem).
