# OSTEP — Phần I: Virtualization (Memory) (ch. 13–24)

> Thuộc [OSTEP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Crux của phần này: *làm sao cho mỗi process ảo giác "toàn bộ bộ nhớ là của mình, liền mạch từ 0" — trong khi RAM vật lý chung, ít, và phân mảnh — mà mỗi truy cập vẫn nhanh?*

---

## Cụm 1 — Address Space & Memory API (ch. 13–14)

### Nội dung chính

**Address space (ch. 13)** = bộ nhớ **mà process nhìn thấy**: code, heap (lớn dần xuống), stack (lớn dần lên) — mọi địa chỉ process thấy đều là **virtual address**; OS + phần cứng dịch sang physical. Ba mục tiêu của virtual memory: **transparency** (process không biết mình bị ảo), **efficiency** (dịch nhanh — cần phần cứng), **protection** (không đọc/ghi được của người khác).

```
0x000...  ┌──────────────┐
          │    code      │
          ├──────────────┤
          │    heap      │
          │      ↓       │
          │   (trống)    │
          │      ↑       │
          │    stack     │
0xFFF...  └──────────────┘   ← mọi process đều "thấy" layout này từ 0
```

**Memory API (ch. 14):** `malloc/free` là **thư viện** quản lý heap trong user space; xin thêm heap từ OS bằng syscall `brk/sbrk` hoặc `mmap` — phân biệt hai tầng này là điểm hay bị hỏi. Catalog lỗi kinh điển: quên cấp phát (`strcpy` vào pointer rác), cấp thiếu (off-by-one cho `\0`), quên init, **quên free (leak)**, **free sớm (dangling → use-after-free)**, **free hai lần**, free pointer không từ malloc. Sách nhấn: với process ngắn hạn, exit là OS thu hồi **toàn bộ** — leak "chết người" là ở process chạy dài (daemon, server, middleware nhúng).

### Insight đáng nhớ

- Câu "mọi địa chỉ bạn từng in ra trong chương trình đều là địa chỉ ảo" — thử `printf("%p")` ở hai lần chạy với ASLR là thấy. Mental model đúng: **pointer = virtual address = cái tên**, không phải vị trí vật lý.
- Repo đã có catalog memory bug + tool ở [09/memory-bugs.md](../../09-debugging/memory-bugs.md) (valgrind, ASan) — OSTEP cho phần "vì sao các bug này tồn tại": vì malloc/free là hợp đồng thủ công không ai kiểm tra.

### Ít quan trọng

- Lịch sử multiprogramming/time-sharing dẫn tới address space (ch. 13 đầu); các hàm phụ `calloc/realloc/strdup` (ch. 14).

### Góc interview

**Câu 1:** `malloc` có phải syscall không? Trình bày quan hệ giữa `malloc` và `brk`/`mmap`.

<details><summary>Đáp án</summary>

- **Không.** `malloc` là hàm **thư viện** (glibc: ptmalloc; hoặc jemalloc/tcmalloc) chạy hoàn toàn trong user space: nó quản lý một vùng heap đã xin sẵn, cắt/ghép các block, dùng free list/bins.
- Chỉ khi **hết vùng quản lý**, malloc mới gọi syscall xin OS: `brk/sbrk` (nới đỉnh heap liền mạch) cho vùng nhỏ, hoặc `mmap(MAP_ANONYMOUS)` cho cấp phát lớn (glibc mặc định ngưỡng ~128KB) — vùng mmap khi free được `munmap` trả OS ngay, còn vùng brk khó trả (chỉ co được từ đỉnh).
- Hệ quả thực dụng đáng nói: (1) đa số lời gọi malloc **không tốn syscall** → nhanh; (2) `free()` thường **không trả RAM cho OS** — RSS không giảm sau free là bình thường; (3) OS cấp page **lazy** — malloc thành công mới chỉ là "đặt chỗ" virtual, page vật lý cấp khi chạm vào lần đầu (page fault) — liên hệ cụm Swapping.

</details>

---

## Cụm 2 — Address Translation: Base/Bounds → Segmentation (ch. 15–16)

### Nội dung chính

**Nguyên tắc chung: hardware-based address translation** — **mọi** truy cập bộ nhớ đều được phần cứng (**MMU**) dịch virtual → physical; OS chỉ **quản lý** (set up bảng, xử lý ngoại lệ), không nằm trên đường truy cập từng lệnh (quá chậm nếu bằng phần mềm).

**Base & bounds (ch. 15)** — giải pháp đầu tiên: mỗi process một cặp thanh ghi: `physical = virtual + base`; `virtual ≥ bounds` → **fault**. Nạp base/bounds là lệnh đặc quyền, đổi khi context switch. Đơn giản, nhanh, bảo vệ tốt — nhưng cả address space phải nằm **liền một khối** → khoảng trống giữa heap và stack chiếm chỗ vô ích (**internal waste**), không co giãn.

**Segmentation (ch. 16)** — tổng quát hóa: mỗi **đoạn logic** (code, heap, stack) một cặp base/bounds riêng → các đoạn đặt rời nhau trong RAM, khoảng trống ảo không tốn vật lý; thêm protection bits mỗi segment (code read-only chia sẻ được), stack cần bit "grows negative". Địa chỉ ảo = [segment | offset]. Cái giá: RAM thành các khối kích thước **khác nhau** → **external fragmentation** — tổng chỗ trống đủ nhưng không liền mạch để cấp; phải compact (đắt) hoặc sống chung bằng thuật toán free-list. Chính thất bại này dẫn đến paging.

### Insight đáng nhớ

- Sự tiến hóa base/bounds → segmentation → paging là chuỗi "đổi vấn đề": liền khối lãng phí → chia theo đoạn (bị external fragmentation) → chia **đều tăm tắp** (paging — hết external, đổi lấy internal fragmentation + bảng dịch to). Kể được chuỗi này là nắm chương.
- Di sản segmentation còn sống trong từ vựng: **segmentation fault** = truy cập ngoài đoạn hợp lệ — dù Linux/x86-64 ngày nay gần như thuần paging.

### Ít quan trọng

- Chi tiết cách tách segment bits tường minh/ngầm và ví dụ dịch địa chỉ từng bước (ch. 16); hardware relocation lịch sử (ch. 15).

### Góc interview

**Câu 1:** External vs internal fragmentation — định nghĩa, cơ chế nào gây ra cái nào, cách khắc phục?

<details><summary>Đáp án</summary>

- **External fragmentation:** vùng trống bị **băm vụn giữa các khối đã cấp** — tổng trống đủ nhưng không có khối liền đủ lớn. Gây bởi cấp phát **kích thước thay đổi**: segmentation, malloc trên heap. Khắc phục: compaction (dồn — đắt, phải sửa mọi tham chiếu), thuật toán chọn khối tốt hơn (best fit...), hoặc **triệt tiêu tận gốc bằng paging** — mọi đơn vị cấp phát bằng nhau thì mọi khối trống đều dùng được.
- **Internal fragmentation:** lãng phí **bên trong** khối đã cấp — xin 5KB nhận 8KB (page 4KB × 2), 3KB thừa không ai dùng được. Gây bởi cấp phát **theo đơn vị cố định**: paging, buddy allocator, size-class của malloc. Khắc phục: đơn vị nhỏ hơn (nhưng bảng quản lý to ra), size class dày hơn, slab allocator cho object cùng cỡ (kernel).
- Chốt: hai loại là **hai mặt của một trade-off** — kích thước biến thiên bị external, kích thước cố định bị internal; thiết kế allocator là chọn điểm đứng giữa. Điểm cộng: nêu huge page (2MB) làm internal fragmentation to hơn nhưng đổi lấy TLB reach.

</details>

---

## Cụm 3 — Free-Space Management (ch. 17)

### Nội dung chính

Vấn đề của mọi allocator kích thước biến thiên (malloc, OS quản lý segment): quản lý **free list** thế nào để nhanh + ít phân mảnh.

**Cơ chế nền:**
- **Splitting**: cấp 1 byte từ khối 10 byte → cắt đôi, trả 1, giữ 9.
- **Coalescing**: khi free, **gộp với khối trống kề bên** — không gộp thì free list toàn mảnh vụn dù bộ nhớ trống liền nhau.
- **Header**: mỗi khối cấp ra có header ngay trước (size + magic) — lý do `free(ptr)` không cần truyền size: `header = ptr - sizeof(header)`.

**Chiến lược chọn khối:**

| Chiến lược | Cách chọn | Đặc điểm |
|---|---|---|
| **Best fit** | Khối nhỏ nhất đủ dùng | Ít thừa mỗi lần, nhưng để lại **mảnh siêu nhỏ vô dụng** + phải quét hết |
| **Worst fit** | Khối to nhất | Ý đồ "mảnh còn lại to, còn dùng được" — thực nghiệm tệ |
| **First fit** | Khối đầu tiên đủ | Nhanh; đầu list vụn dần |
| **Next fit** | First fit nhưng đi tiếp từ chỗ cũ | Rải đều hơn first fit, tương đương tốc độ |

Nâng cao: **segregated lists** (mỗi size phổ biến một list riêng — slab allocator của kernel: cache object cùng kích thước, khởi tạo sẵn) và **buddy allocator** (chia đôi đệ quy theo lũy thừa 2; free thì gộp "buddy" đệ quy — coalescing cực nhanh, trả giá internal fragmentation; chính là allocator trang vật lý của Linux).

### Insight đáng nhớ

- Không có chiến lược thắng tuyệt đối — allocator thật (glibc ptmalloc, jemalloc) là **lai**: size class nhỏ dùng segregated bins (kiểu slab), vùng lớn mmap thẳng, giữa dùng best-fit trên cây. Trả lời interview về malloc internals theo khung "bins theo size + split/coalesce + header" là đủ khung xương.
- Buddy + slab là cặp đôi đáng nhớ cho embedded: buddy cấp **page**, slab cắt page thành **object** (inode, task_struct...) — `cat /proc/buddyinfo`, `/proc/slabinfo` nhìn được trực tiếp.

### Ít quan trọng

- Ví dụ từng bước split/coalesce với địa chỉ cụ thể và code header/magic đầy đủ (ch. 17 giữa chương).

### Góc interview

**Câu 1:** Vì sao `free(ptr)` không cần truyền kích thước? Điều gì xảy ra nếu ghi tràn qua cuối một khối malloc?

<details><summary>Đáp án</summary>

- Allocator giấu **header** ngay trước vùng trả về: `[header: size|magic][vùng user ...]` — `free` lùi con trỏ về `ptr - sizeof(header)` đọc size (và check magic). Kích thước là "sổ sách" của allocator, không phải của caller.
- Ghi tràn qua cuối khối → đè lên **header/metadata của khối kế** (size, con trỏ free-list) → hỏng sổ sách: malloc/free sau đó crash "ở chỗ chẳng liên quan", coalescing gộp sai, hoặc bị khai thác (heap exploitation cổ điển đè con trỏ free-list). Triệu chứng đặc trưng: crash **trong malloc/free chứ không tại chỗ ghi tràn** — lý do phải dùng ASan/valgrind thay vì nhìn stack trace tin ngay ([09/memory-bugs.md](../../09-debugging/memory-bugs.md)).

</details>

---

## Cụm 4 — Paging & TLB (ch. 18–19) 🎯

### Nội dung chính

**Paging (ch. 18):** chia address space ảo thành **page** kích thước cố định (4KB điển hình), RAM vật lý thành **page frame** cùng cỡ; ánh xạ page → frame **tùy ý** qua **page table** (mỗi process một bảng). Hết external fragmentation (mọi khối bằng nhau), cấp phát đơn giản (free list các frame), address space thưa thớt thoải mái.

**Dịch địa chỉ:** virtual address = [**VPN** (virtual page number) | **offset**]:

```
VA (32-bit, page 4KB):  [ VPN: 20 bit ][ offset: 12 bit ]
                              │              │ (giữ nguyên)
                    page table[VPN] → PFN    │
                              ▼              ▼
PA:                     [ PFN        ][ offset ]
```

**PTE (page table entry)** chứa PFN + các bit trạng thái — thuộc lòng nhóm này:
- **valid/present** — trang có được map không (bảo vệ vùng trống; và phân biệt "chưa cấp" vs "đang swap")
- **protection** (R/W/X — NX bit chống thực thi stack/heap)
- **accessed** (phần cứng bật khi đọc — đầu vào cho thuật toán thay trang)
- **dirty** (bật khi ghi — trang sạch khỏi ghi lại đĩa khi bị thu hồi)
- **user/supervisor** (chặn user đụng trang kernel)

Vấn đề sinh đôi của paging: (1) bảng **to** (2²⁰ entry × 4B = 4MB *mỗi process* cho 32-bit — cụm 5 xử lý); (2) **chậm** — mỗi truy cập bộ nhớ cần thêm ≥1 lần đọc page table trong RAM → TLB xử lý.

**TLB (ch. 19) — cache của phép dịch địa chỉ, nằm trong MMU.** "Address translation cache": giữ các cặp VPN→PFN mới dùng:

```
truy cập VA → tách VPN → tra TLB
   ├─ HIT  (đại đa số): lấy PFN, ghép offset — gần như miễn phí
   └─ MISS: đi bộ page table trong RAM (HW page-table walker trên x86/ARM;
            một số kiến trúc MIPS/SPARC: trap vào OS xử lý bằng phần mềm)
            → nạp entry vào TLB → chạy lại lệnh
```

- TLB hiệu quả nhờ **locality**: spatial (duyệt array — cả page dùng chung 1 entry) + temporal (lặp lại địa chỉ vừa dùng). Ví dụ đắt của sách: duyệt array theo hàng vs theo cột — cùng số phép truy cập, khác hẳn số TLB miss.
- TLB nhỏ (vài chục–vài trăm entry) → **TLB reach** = số entry × page size (~64×4KB=256KB) — workload chạm quá reach là miss liên tục; **huge page** (2MB) nới reach — kỹ thuật thật cho DB/DPDK/embedded hiệu năng cao.
- **Context switch và TLB:** ánh xạ là **của từng process** — switch process thì entry cũ vô nghĩa. Hoặc **flush toàn bộ** (đắt: sau switch toàn miss), hoặc gắn **ASID** (address space ID — x86: PCID) vào entry để TLB chứa chung nhiều process → không phải flush. Đây là phần lớn "chi phí gián tiếp của context switch" đã nói ở [virtualization-cpu.md](virtualization-cpu.md).

### Insight đáng nhớ

- Câu chốt đáng thuộc: **"paging đổi external fragmentation lấy internal fragmentation + một tầng gián tiếp; TLB tồn tại để trả lại tốc độ cho tầng gián tiếp đó."** Mọi câu hỏi paging/TLB đều xoay quanh câu này.
- Cache CPU (L1/L2) cache **dữ liệu**; TLB cache **phép dịch** — hai thứ độc lập, miss độc lập. Truy cập "lạnh" hoàn toàn có thể trả giá: TLB miss (+walk nhiều lần đọc RAM) rồi mới cache miss cho chính dữ liệu.
- Với embedded: page size, TLB size là thông số thật trong datasheet SoC; hiểu TLB giải thích vì sao code xử lý buffer lớn nên đi **tuần tự theo địa chỉ**.

### Ít quan trọng

- Ví dụ bit-by-bit dịch địa chỉ với page 16 byte, và bảng đo miss/hit từng vòng lặp (ch. 18–19 giữa chương).
- TLB software-managed (MIPS) chi tiết lệnh nạp TLB (ch. 19) — chỉ cần biết "tồn tại kiểu này".

### Góc interview

**Câu 1 (🎯):** Trình bày đường đi đầy đủ của một lần truy cập bộ nhớ: TLB, page table, cache. Điều gì xảy ra khi TLB miss? Khi PTE invalid?

<details><summary>Đáp án</summary>

1. CPU phát **virtual address** → tách [VPN | offset].
2. **Tra TLB** theo VPN: **hit** → có PFN ngay (thêm check protection bits — vi phạm thì fault) → physical address = [PFN|offset] → truy cập qua hệ cache dữ liệu (L1→L2→L3→RAM).
3. **TLB miss** → **page table walk**: phần cứng (x86/ARM) đi các cấp bảng trang trong RAM (mỗi cấp một lần đọc bộ nhớ — 4 cấp trên x86-64) tìm PTE:
   - PTE **valid** → nạp vào TLB (đuổi một entry theo LRU xấp xỉ) → **retry lệnh** → giờ hit.
   - PTE **invalid/not present** → phần cứng **trap: page fault** → OS xử lý (sang bước 4).
4. OS page fault handler phân loại: (a) trang **hợp lệ nhưng chưa cấp/đang swap** (minor/major fault) → cấp frame, nạp dữ liệu từ đĩa nếu major, sửa PTE, retry; (b) truy cập **bất hợp pháp** → gửi **SIGSEGV** cho process.
- **Bẫy khi trả lời:** (1) nói "TLB miss = page fault" — sai, TLB miss chỉ là *cache miss của phép dịch*, thường được giải quyết bằng walk mà không có fault; page fault chỉ khi PTE không hợp lệ; (2) quên rằng walk = **nhiều lần đọc RAM** (nên mới cần TLB); (3) quên bước **retry** lệnh sau khi xử lý.

</details>

**Câu 2:** Vì sao context switch làm chậm chương trình cả *sau khi* switch xong? ASID/PCID giúp gì?

<details><summary>Đáp án</summary>

- Sau switch, chi phí trực tiếp (lưu/nạp registers) đã trả xong, nhưng process mới chạy trên **cache/TLB đang chứa dữ liệu của process cũ**:
  - **TLB**: nếu kiến trúc phải flush khi đổi page table → mọi truy cập đầu là miss + walk; 
  - **cache dữ liệu**: dữ liệu process mới chưa có trong L1/L2 → miss hàng loạt;
  - branch predictor cũng nguội.
  Chuỗi miss này kéo dài hàng chục nghìn chu kỳ — thường lớn hơn chi phí switch trực tiếp cả bậc.
- **ASID (ARM) / PCID (x86)**: gắn ID address space vào từng TLB entry → entry của nhiều process **chung sống** trong TLB, đổi process chỉ đổi ID hiện hành, **không flush**; quay lại process cũ thì entry cũ còn đó (nếu chưa bị đuổi). OS phải quản lý cấp phát/tái sử dụng ID (số bit hữu hạn).
- Điểm cộng: liên hệ thread vs process switch — thread cùng process **không đổi address space** → không có vấn đề TLB, chỉ còn cache/registers → lý do thread switch rẻ hơn; và kể tên kỹ thuật giảm tần suất switch: batching, io_uring, busy-poll cho NIC.

</details>

**Câu 3:** Huge page là gì, giúp gì và trả giá gì?

<details><summary>Đáp án</summary>

- Page lớn hơn chuẩn (x86-64: 2MB/1GB thay vì 4KB; ARM tương tự theo cấp bảng). Một TLB entry phủ 2MB thay vì 4KB → **TLB reach tăng 512×** → workload dữ liệu lớn (DB, packet processing, ML) giảm hẳn TLB miss; walk cũng ngắn hơn một cấp.
- Giá: (1) **internal fragmentation** — cấp 2MB cho nhu cầu lẻ; (2) cần **2MB vật lý liền mạch** — hệ chạy lâu RAM phân mảnh khó kiếm (buddy allocator), transparent huge page (THP) phải nén/dồn nền → latency spike (nhiều hệ latency-sensitive **tắt THP**, dùng hugetlbfs cấp tĩnh từ boot); (3) đơn vị swap/COW to hơn → copy-on-write một byte đụng cả 2MB.
- Embedded angle: cấp hugetlbfs tĩnh lúc boot cho DMA buffer/vùng chia sẻ hiệu năng cao là pattern phổ biến — vừa TLB tốt vừa liền mạch vật lý.

</details>

---

## Cụm 5 — Page Table nhiều cấp (ch. 20) 🎯

### Nội dung chính

**Crux:** bảng phẳng 32-bit = 4MB/process bất kể dùng bao nhiêu — đại đa số address space **trống** mà vẫn tốn entry. Các hướng:

- **Kết hợp segmentation** (bounds cho từng đoạn bảng) — nửa vời, kế thừa vấn đề segmentation.
- **Multi-level page table** — giải pháp thắng cuộc: chặt bảng thành các **trang của bảng**, thêm **page directory** trỏ tới chúng; vùng address space trống → directory entry invalid → **cả trang bảng con không tồn tại**:

```
VA: [ PD index ][ PT index ][ offset ]
        │            │
        ▼            │
  Page Directory     │        Mức nào invalid → dừng ngay,
  ├─ PDE 0: valid ───┼──► Page Table page ─► PTE → PFN
  ├─ PDE 1: INVALID  │    (vùng trống: không tốn cả trang bảng con)
  └─ ...             ▼
```

- Trade-off gọi tên chính xác: **time-space trade-off** — tiết kiệm chỗ (bảng tỉ lệ theo *phần được dùng* của address space), trả bằng walk **nhiều lần đọc RAM hơn** khi TLB miss (mỗi cấp một lần). x86-64 hiện dùng **4 cấp** (PML4→PDPT→PD→PT, 9 bit/cấp + 12 bit offset = 48-bit VA; 5 cấp cho 57-bit đã có). TLB hit thì số cấp không thành vấn đề — lại là TLB cứu.
- **Inverted page table** — hướng ngược: một bảng **toàn hệ thống** theo frame vật lý (frame nào đang chứa page nào của process nào), tra bằng hash — tiết kiệm cực đại, tra phức tạp (PowerPC dùng).
- Bảng trang của process nằm trong **kernel virtual memory** — chính nó cũng có thể được swap (sách nhắc để đóng khung: "bảng cũng chỉ là dữ liệu").

### Insight đáng nhớ

- Câu trả lời một dòng đáng nhớ: **"multi-level page table = cây thưa (sparse tree) thay cho mảng đặc"** — cấu trúc dữ liệu quen thuộc áp vào bài toán hệ thống; vùng trống không tốn node.
- Số cấp × 1 lần đọc RAM mỗi walk là lý do phần cứng có **page-walk cache** (cache các PDE giữa chừng) — và lại một lần nữa: mọi thứ sống được nhờ TLB hit rate cao.

### Ít quan trọng

- Ví dụ dựng bảng 2 cấp bit-by-bit với address space 16KB (ch. 20 giữa); chi tiết inverted + hashed page table (cuối chương).

### Góc interview

**Câu 1 (🎯):** Vì sao page table phải nhiều cấp? Tính kích thước bảng phẳng cho 48-bit VA, page 4KB để chứng minh.

<details><summary>Đáp án</summary>

- Bảng phẳng 48-bit VA, page 4KB: 2⁴⁸/2¹² = **2³⁶ entry** × 8B = **512GB cho MỖI process** — phi lý tuyệt đối. Kể cả 32-bit: 4MB/process × trăm process = hàng GB chỉ cho bảng.
- Bản chất vấn đề: address space **cực thưa** — vài trăm MB dùng thật rải trong 256TB ảo; mảng đặc trả tiền cho cả vùng trống.
- Multi-level = cây thưa: chỉ tồn tại các nhánh dẫn tới vùng **có dùng**; directory entry invalid cắt cả cây con. Bảng thực tế tỉ lệ theo memory *đang dùng* (vài trang bảng cho mỗi vài MB được map).
- Giá phải trả: TLB miss → walk 4 cấp = **4 lần đọc RAM** (x86-64) thay vì 1; được bù bởi TLB + page-walk cache. Đây là time-space trade-off kinh điển — nói đúng cụm từ này là điểm cộng.
- Bẫy: đừng nói "nhiều cấp để dịch nhanh hơn" — **ngược lại**, nhiều cấp dịch *chậm hơn* khi miss; mục đích duy nhất là tiết kiệm bộ nhớ bảng.

</details>

---

## Cụm 6 — Swapping: cơ chế & chính sách thay trang (ch. 21–22) 🎯

### Nội dung chính

**Cơ chế (ch. 21):** muốn ảo giác "bộ nhớ lớn hơn RAM" → dùng đĩa làm tầng dưới (**swap space**). PTE có **present bit**: trang thuộc process nhưng đang ở đĩa → present=0 + PTE chứa **địa chỉ trên swap**. Truy cập trang vắng mặt:

```
truy cập VA → TLB miss → walk → PTE: present = 0
   → PAGE FAULT (trap vào OS — "major fault" vì phải đụng đĩa)
   → OS: tìm frame trống (không có → ĐUỔI một trang: chính sách ở dưới;
          trang bị đuổi mà dirty → phải GHI về đĩa trước)
   → đọc trang từ swap vào frame (process BLOCKED trong lúc chờ đĩa —
      OS chạy process khác: lại là overlap)
   → sửa PTE (present=1, PFN mới) → retry lệnh
```

OS không chờ hết sạch RAM mới đuổi: **swap daemon (kswapd)** giữ lượng frame trống trong khoảng [low watermark, high watermark] — đuổi **trước, theo lô, ở nền** (gộp ghi đĩa, để đường cấp phát nhanh luôn có sẵn frame).

**Chính sách — đuổi trang nào? (ch. 22):** mục tiêu = tối thiểu miss (AMAT bị chi phối hoàn toàn bởi miss đĩa: RAM ~100ns vs đĩa ~ms — **chậm hơn hàng vạn lần**, hit rate 99% vẫn chưa đủ tốt).

| Chính sách | Ý tưởng | Vấn đề |
|---|---|---|
| **OPT/MIN** | Đuổi trang được dùng lại **xa nhất trong tương lai** | Không cài được (cần tiên tri) — chỉ làm **mốc so sánh** |
| **FIFO** | Đuổi trang vào lâu nhất | Đuổi cả trang nóng; dính **Belady's anomaly** (thêm RAM lại miss nhiều hơn) |
| **Random** | Đuổi bừa | Bất ngờ: không tệ, không bị góc chết |
| **LRU** | Đuổi trang **lâu chưa dùng nhất** (dùng quá khứ đoán tương lai — locality) | Đúng nguyên lý, nhưng cài chính xác **quá đắt** (cập nhật cấu trúc mỗi lần truy cập bộ nhớ!) |
| **Clock (LRU xấp xỉ)** | Vòng tròn frame + **accessed bit**: kim quét, bit=1 → xóa về 0 đi tiếp, bit=0 → đuổi | Cái được dùng thật; thêm **dirty bit**: ưu tiên đuổi trang sạch (khỏi ghi đĩa) |

LRU/Clock có stack property → không dính Belady's anomaly. Các hệ thật (Linux) là biến thể clock nhiều mức (active/inactive list, LRU-gen gần đây).

**Thrashing:** tổng working set các process > RAM → hệ chỉ lo swap in/out, CPU idle chờ đĩa, throughput sập. Giải pháp cổ điển: admission control (chạy ít process hơn — "làm ít việc cho đúng còn hơn nhiều việc dở"); Linux hiện đại: **OOM killer** — chọn process "nặng ký" giết để cứu hệ.

### Insight đáng nhớ

- **Page fault không phải lỗi** — nó là *cơ chế vận hành bình thường* của demand paging (lazy allocation, mmap file, COW của fork, swap đều chạy trên page fault). Phân biệt minor (chỉ sửa map, không đụng đĩa) vs major (đọc đĩa) là ranh giới hiệu năng — `ps -o min_flt,maj_flt`, `perf stat`.
- Watermark + kswapd là pattern tổng quát đáng học cho system design: **đừng xử lý cạn kiệt tại đường nóng — dọn nền theo lô giữ sẵn headroom** (giống free list allocator, giống buffer pool).
- Embedded angle đắt giá: thiết bị nhúng thường **không có swap** (flash chậm + mòn) → hết RAM đi thẳng tới OOM killer → tuning `overcommit`, `oom_score_adj`, hoặc cgroup memory limit là kỹ năng thực chiến; và `mlock()` giữ trang realtime không bị đuổi.

### Ít quan trọng

- Mô phỏng hit rate các policy trên các workload (no-locality / 80-20 / looping) — đọc khi cần trực giác đồ thị (ch. 22 giữa).
- Prefetching, clustering ghi swap (ch. 21 cuối); page selection/replacement cho VAX (để dành cụm 7).

### Góc interview

**Câu 1 (🎯):** Phân biệt minor page fault, major page fault, segfault. Cho ví dụ tình huống sinh ra từng loại.

<details><summary>Đáp án</summary>

- **Minor fault:** trang chưa map vào process nhưng **không cần đọc đĩa** — chỉ sửa page table. Ví dụ: chạm lần đầu vào vùng malloc lớn (lazy allocation — cấp frame zero), fork rồi con/cha ghi vào trang COW (copy page, sửa map), trang đã nằm sẵn trong page cache (mmap file mà file đang được cache).
- **Major fault:** phải **đọc từ đĩa** — trang ở swap, hoặc mmap file chưa được cache (bao gồm cả **nạp code lần đầu**: binary được mmap, chạm hàm nào nạp trang đó). Đắt gấp vạn lần minor — theo dõi bằng `maj_flt`, `perf`.
- **Segfault (SIGSEGV):** truy cập **không hợp lệ theo sổ sách của OS** — vùng chưa từng map, hoặc sai quyền (ghi vào trang read-only ngoài COW, thực thi trang NX). OS tra VMA (vùng hợp lệ của process): fault địa chỉ nằm ngoài mọi VMA → gửi signal thay vì sửa map.
- Chốt ăn điểm: **cùng một cơ chế phần cứng** (trap khi PTE không thỏa) — khác nhau ở *cách OS phân loại và xử lý*: sửa map + retry (fault lành) hay SIGSEGV (fault bệnh). Hiểu điều này giải thích luôn vì sao debugger/ASan bắt được truy cập sai: chúng thao túng chính lớp map/protection này.

</details>

**Câu 2:** Vì sao LRU đúng nguyên lý nhưng không được cài chính xác? Clock algorithm xấp xỉ nó thế nào?

<details><summary>Đáp án</summary>

- LRU chính xác đòi cập nhật "thời điểm dùng gần nhất" **mỗi lần truy cập bộ nhớ** — tức mỗi lệnh load/store phải kèm cập nhật một cấu trúc (list/timestamp). Làm bằng phần mềm thì mỗi truy cập thành trap (chậm không dùng nổi); phần cứng thì không ai gắn timestamp per-access per-page. → chỉ cần **xấp xỉ đủ tốt**.
- Phần cứng cho đúng **1 bit rẻ tiền**: **accessed/reference bit** — MMU bật lên khi trang được đụng tới. **Clock**: xếp frame thành vòng tròn, kim quét khi cần đuổi:
  - bit = 1 → "được dùng gần đây" → xóa về 0, tha, đi tiếp (cho một vòng ân hạn);
  - bit = 0 → "cả vòng qua không ai đụng" → đuổi.
- Tinh chỉnh thật: ưu tiên đuổi trang **not-accessed + not-dirty** (khỏi ghi đĩa); Linux dùng biến thể hai danh sách active/inactive (thăng/giáng theo accessed bit) — cùng họ clock.
- Điểm cộng: nêu **Belady's anomaly** — FIFO thêm frame có thể tăng miss; LRU/clock thuộc lớp stack algorithm nên miễn nhiễm; và nêu vì sao Random "không tệ" — không có workload góc chết, đáng nhớ khi thiết kế cache đơn giản.

</details>

**Câu 3:** Thiết bị nhúng của bạn không có swap. Hết RAM thì chuyện gì xảy ra, và bạn phòng bị thế nào?

<details><summary>Đáp án</summary>

- Không swap → kernel không có "van xả" trang anonymous (heap/stack); nó chỉ còn: (1) đuổi **page cache** và trang **file-backed sạch** (code có thể bị đuổi rồi nạp lại từ flash — hệ chậm rề rề trước khi chết: "gần OOM thì mọi thứ giật cục" chính là code bị đuổi/nạp lặp lại); (2) hết đường → **OOM killer** chọn process theo oom_score (ưu tiên RSS lớn) và **SIGKILL** — không phải lỗi malloc trả NULL như nhiều người tưởng, vì Linux mặc định **overcommit**: malloc thành công không hứa có RAM thật, chết lúc *chạm* trang.
- Phòng bị thực chiến:
  1. **Giới hạn theo nhóm**: cgroup memory limit cho từng service — chết cục bộ, không kéo cả hệ.
  2. **Điều chỉnh nạn nhân**: `oom_score_adj` — bảo vệ process điều khiển tối quan trọng, hy sinh process phụ.
  3. `vm.overcommit_memory` chế độ chặt nếu muốn malloc fail sớm thay vì OOM muộn (trade-off: fork/mmap lớn dễ fail).
  4. **mlock/mlockall** cho vùng realtime — không cho đuổi trang code/data của luồng điều khiển.
  5. Giám sát PSI (`/proc/pressure/memory`)/watermark để phát hiện áp lực sớm thay vì đợi OOM log.
- Điểm cộng: biết OOM killer log trong `dmesg` (điểm số từng process) — kỹ năng đọc hậu quả thực tế.

</details>

---

## Cụm 7 — Case study: VAX/VMS & tổng kết (ch. 23–24)

### Nội dung chính

Đọc nhanh — giá trị là thấy **các cơ chế ghép lại thành hệ thật** và vài mẹo còn sống đến nay:
- VAX/VMS đối phó phần cứng yếu (không accessed bit): **segmented FIFO** — FIFO per-process + second-chance list toàn cục (trang bị đuổi vào list chờ, đụng lại thì cứu về — xấp xỉ LRU không cần bit).
- Hai phát minh sống mãi: **demand zeroing** (trang zero cấp lazy — chỉ zero khi chạm thật) và **copy-on-write** (fork/`vfork`/chia sẻ trang: map chung read-only, ghi mới copy) — cả hai đều là "lười có nguyên tắc" trên page fault.
- Page 512B quá nhỏ → clustering (gom lô khi swap) — bài học "đơn vị phần cứng không khớp đơn vị hiệu quả thì gom lô".

### Ít quan trọng

- Toàn bộ chi tiết layout address space VAX và số liệu thời đó (ch. 23) — đọc như tư liệu lịch sử; ch. 24 là summary của sách.

### Góc interview

**Câu 1:** Copy-on-write hoạt động thế nào và vì sao làm fork rẻ?

<details><summary>Đáp án</summary>

- Khi fork, thay vì copy toàn bộ address space, kernel cho cha con **map chung mọi trang** và đánh dấu cả hai bên **read-only**. Ai **ghi** vào trang chung → protection fault → kernel nhận ra đây là trang COW: **copy trang đó ra frame mới**, sửa PTE bên ghi thành writable trỏ trang mới, retry. Trang chỉ đọc (code, phần lớn data) **không bao giờ bị copy**.
- Fork process 1GB: chi phí thật chỉ là **copy page table + đánh dấu RO** (ms thay vì giây); mẫu fork+exec càng lợi — exec vứt address space ngay, copy trước là công cốc.
- Bẫy liên quan đáng nêu: fork từ process RSS lớn vẫn tốn (copy bảng, và overcommit accounting); `vfork`/`posix_spawn` cho đường tắt; và COW là lý do ghi vào biến sau fork làm **RSS tăng đột ngột** ở một trong hai bên.

</details>

### Đọc thêm (tùy chọn)

- [03/memory-management.md](../../03-operating-system/memory-management.md) — bản cô đọng virtual memory của repo.
- [01/memory-model.md](../../01-cpp-fundamentals/memory-model.md) — layout process nhìn từ C/C++.
- [09/memory-bugs.md](../../09-debugging/memory-bugs.md) — bug heap và tool, nối với cụm 1, 3.
