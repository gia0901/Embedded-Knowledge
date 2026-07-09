# 📙 FILE 4 — NGÂN HÀNG CÂU HỎI CHUYÊN SÂU BSP (có lời giải đầy đủ)
### Embedded Software Engineer — BSP · bổ sung cho FILE 2 (không lặp lại)

> **Quan hệ với FILE 2:** `02_question_bank.md` phủ nền chung (C++/system/bus/debug cơ bản). File này **khoan sâu đúng mảng BSP**: boot chain, device tree/probe, interrupt/DMA, flash & OTA, Yocto, power, real-time, và tình huống bring-up.
> **Cách luyện:** tự trả lời thành tiếng trước, mở `<details>` đối chiếu. Mỗi câu có link "→ đào sâu" về tài liệu topic 16 (MELP/OSTEP) nơi có ngữ cảnh đầy đủ.
> Ký hiệu: ⭐ = xác suất gặp rất cao với JD BSP · 🏗️ = câu thiết kế/tình huống (trả lời theo khung, không có đáp án duy nhất).

---

## PHẦN A — BOOT CHAIN & BOOTLOADER

**A1. ⭐ Kể chi tiết quá trình boot từ lúc cấp nguồn đến shell, và giải thích VÌ SAO phải nhiều giai đoạn.**

<details><summary>Đáp án</summary>

- Chuỗi: **ROM code** (trong silicon — đọc boot pins chọn nguồn, nạp giai đoạn kế vào **SRAM nội**) → **SPL** (vừa SRAM; việc chính: **init DRAM** + clock/PMIC tối thiểu, nạp U-Boot vào DRAM) → **U-Boot proper** (init storage/console, đọc env, nạp kernel Image + **DTB** + initramfs vào RAM, `booti` với con trỏ DTB trong x0) → **kernel** (parse DTB, bật MMU, probe driver, mount rootfs theo `root=`) → **init PID 1** → service → shell.
- Vì sao nhiều giai đoạn: **chuỗi bootstrap tài nguyên** — lúc cấp nguồn DRAM chưa hoạt động, chỉ có SRAM vài chục–trăm KB; U-Boot đầy đủ không vừa SRAM → cần SPL đủ nhỏ để dựng DRAM trước. Mỗi tầng chỉ đủ khả năng dựng tầng kế.
- ARM64 + secure boot: chuỗi TF-A `BL1→BL2(≈SPL)→BL31(secure monitor, PSCI, ở lại runtime)→BL33(U-Boot)`, mỗi bước **verify chữ ký** bước sau = chain of trust từ khóa trong eFuse.
- Mốc debug từng đoạn: không gì trên UART = trước SPL; banner U-Boot mà không "Starting kernel" = load/bootargs; dừng ở VFS = root=.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**A2. ⭐ `bootargs` gồm những gì quan trọng? Kể 3 lỗi bootargs kinh điển và triệu chứng.**

<details><summary>Đáp án</summary>

- `bootargs` = **kernel command line** — hợp đồng bootloader→kernel: `console=ttymxc0,115200` (console), `earlycon` (console siêu sớm không cần driver), `root=/dev/mmcblk0p2` + `rootwait` (rootfs ở đâu, chờ device), `rootfstype=`, `init=`, `loglevel=`, `mem=`, `nfsroot=`/`ip=` (NFS boot).
- Ba lỗi kinh điển:
  1. **Sai `console=`** (tên device của kernel khác U-Boot: ttyS0 vs ttymxc0 vs ttyAMA0) → kernel boot **hoàn toàn im lặng** sau "Starting kernel" dù hệ có thể vẫn lên. Chẩn đoán: thêm `earlycon`.
  2. **Sai `root=`/thiếu driver FS** → panic `VFS: Unable to mount root fs` — đọc dòng "available partitions" ngay trên panic để đối chiếu.
  3. **Thiếu `rootwait`** với eMMC/SD/USB → kernel tìm root *trước khi* storage enumerate xong → panic chập chờn tùy lần boot (bug "thỉnh thoảng không lên" kinh điển).

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**A3. FIT image là gì, hơn uImage cũ chỗ nào?**

<details><summary>Đáp án</summary>

- **FIT (Flattened Image Tree)**: một file `.itb` đóng gói **kernel + nhiều DTB + initramfs + configuration**, mô tả bằng nguồn `.its` (cú pháp device tree), build bằng `mkimage`.
- Hơn uImage (chỉ bọc một ảnh + header CRC): (1) **nhiều thành phần + nhiều cấu hình** — một ảnh phục vụ nhiều biến thể board, U-Boot chọn config theo board lúc boot; (2) **hash/chữ ký từng thành phần** (SHA + RSA) — nền của **verified boot** trong U-Boot (khóa public nhúng trong DTB của U-Boot); (3) metadata rõ ràng (load address, entry, compression).
- Thực dụng: sản phẩm cần secure boot hoặc nhiều SKU chung một firmware → FIT gần như bắt buộc.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**A4. 🏗️ Bootloader có được OTA update không? Rủi ro và cách làm đúng.**

<details><summary>Đáp án</summary>

- Nguyên tắc: **hạn chế tối đa** — bootloader là mắt xích không có ai đứng sau: ghi dở SPL/U-Boot = ROM code không còn gì hợp lệ để nạp = brick sâu (chỉ cứu bằng UART/USB boot mode ở xưởng).
- Nếu buộc phải update, dùng cơ chế **phần cứng có dự phòng**: eMMC có **2 boot partition** — ghi bản mới vào partition không active, verify, rồi đổi `mmc bootpart`; một số SoC hỗ trợ nhiều bản ảnh + fallback trong ROM (đọc bản kế khi bản đầu hỏng CRC). Không có các cơ chế đó → cân nhắc *không* update bootloader ngoài hiện trường.
- Thiết kế phòng ngừa từ đầu: bootloader **tối giản và đóng băng** — mọi logic hay đổi (chọn bank A/B, health check) đẩy lên kernel/userspace hoặc thành script/env có thể thay không đụng binary.

→ đào sâu: [melp/storage-update.md](../../16-book-summaries/melp/storage-update.md)
</details>

---

## PHẦN B — KERNEL & DEVICE TREE

**B1. ⭐ Trình bày đường đi từ một node device tree đến hàm `probe()` của driver. EPROBE_DEFER là gì?**

<details><summary>Đáp án</summary>

- Bootloader trao DTB → kernel parse, dựng **platform_device** cho các node (node con của I2C/SPI do driver bus controller tạo khi chính nó probe). Driver đăng ký `platform_driver` với **`of_match_table`** (danh sách chuỗi `compatible`). Kernel match node ↔ driver theo compatible (so từ chuỗi cụ thể nhất) → gọi **`probe(pdev)`**: đọc `reg` → `devm_ioremap_resource`, `interrupts` → `devm_request_irq`, `clocks/…-supply` → clk/regulator API, property riêng → `device_property_read_*`, rồi đăng ký vào subsystem.
- **EPROBE_DEFER**: probe cần tài nguyên mà driver cung cấp nó **chưa probe** (clock, regulator, GPIO expander) → return `-EPROBE_DEFER` → kernel xếp lại, **probe lại sau mỗi lần có driver mới xong** — thứ tự phụ thuộc tự hội tụ, không cần hard-code. Thiết bị "mất tích" không lỗi → xem `/sys/kernel/debug/devices_deferred`.
- Lưu ý hay bị bắt bẻ: DT **không nạp driver** — driver phải có sẵn (built-in, hoặc module được udev nạp theo **modalias/uevent**, cần `MODULE_DEVICE_TABLE(of, …)`); DT chỉ là dữ liệu khớp nối.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**B2. ⭐ Thêm một thiết bị I2C mới vào board đang chạy: các bước và các lỗi thường gặp.**

<details><summary>Đáp án</summary>

- Các bước: (1) tìm driver theo compatible trong kernel, bật CONFIG; (2) sửa DT: trong `&i2cN` (đủ `status="okay"` + pinctrl), thêm node con `compatible` + `reg=<addr>` (+ interrupt/supply nếu dùng), đúng **bindings YAML** (`make dtbs_check`); (3) build **chỉ dtbs**, deploy DTB (hoặc overlay `.dtbo`); (4) kiểm chứng theo tầng: `i2cdetect` → dmesg probe → node trong `/sys/bus/i2c/devices/` → đọc giá trị.
- Lỗi thường gặp: sai địa chỉ I2C (strap pin đổi addr — đối chiếu datasheet với `i2cdetect`); **quên pinmux** SDA/SCL (bus timeout); thiếu pull-up phần cứng; driver để `m` mà module không có trên rootfs; `vcc-supply` trỏ regulator disabled → **EPROBE_DEFER treo vĩnh viễn**; compatible gõ sai một ký tự (match là so chuỗi tuyệt đối); i2cdetect thấy mà driver không bind → gần như chắc chắn lỗi compatible/CONFIG.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**B3. Driver nên built-in (`y`) hay module (`m`)? Nêu tiêu chí và ví dụ nghịch lý gà-trứng.**

<details><summary>Đáp án</summary>

- Tiêu chí: cần **trước khi mount rootfs** (storage controller, FS của root, console) → `y` hoặc `m`-trong-initramfs; phần cứng tùy chọn/nhiều SKU → `m` (nạp theo modalias); hệ khóa cấu hình ít RAM → nghiêng `y` toàn bộ, có thể tắt hẳn hạ tầng module (nhỏ + thu hẹp bề mặt tấn công).
- Nghịch lý gà-trứng: **driver eMMC là module nằm trên rootfs trong chính eMMC** → không bao giờ nạp được (muốn đọc rootfs phải có driver, muốn có driver phải đọc rootfs). Giải: built-in, hoặc bỏ module vào initramfs (cách của distro desktop).
- Chi tiết cộng điểm: console driver phải built-in; `CONFIG_MODULES=n` là một lựa chọn sản phẩm hợp lệ.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**B4. Pinctrl trong device tree đóng vai trò gì? Triệu chứng khi thiếu/sai pinmux?**

<details><summary>Đáp án</summary>

- Mỗi chân SoC đa năng (một pad → GPIO/UART/I2C/PWM… chọn bằng mux register). **pinctrl** mô tả cấu hình pad trong DT: nhóm `pinctrl_uart1 { fsl,pins = <...> }` (mux + drive strength/pull), node thiết bị tham chiếu qua `pinctrl-names = "default"` + `pinctrl-0 = <&pinctrl_uart1>` — driver core tự áp khi probe.
- Triệu chứng thiếu/sai: **driver probe thành công, phần mềm chạy "bình thường", nhưng tín hiệu không ra chân** (hoặc ra chân khác) — nhóm lỗi khó chịu nhất vì mọi log đều sạch. UART câm, I2C timeout, LED không sáng… Chẩn đoán: đo chân bằng scope/logic analyzer + đọc thanh ghi mux (`devmem`) đối chiếu datasheet; nhớ rằng **U-Boot chạy được không có nghĩa kernel đúng** — kernel áp lại pinmux theo DT của nó, không thừa kế U-Boot.
- Bộ ba lỗi DT thực chiến: status quên "okay", pinmux sai, EPROBE_DEFER vòng — thuộc lòng.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

---

## PHẦN C — INTERRUPT, DMA & MEMORY-MAPPED I/O

**C1. ⭐ Vì sao interrupt handler không được ngủ? Threaded IRQ / top-bottom half giải quyết gì?**

<details><summary>Đáp án</summary>

- Handler chạy trong **interrupt context** — không có task_struct, không phải thực thể schedule được; "ngủ" = gọi scheduler nhường CPU chờ wake — không có chỗ đứng trong hàng đợi scheduler → `scheduling while atomic`/treo. Kèm theo: dòng IRQ đang mask, ngủ lâu là mất ngắt/deadlock với lock mà process bên dưới giữ. Hệ quả: cấm mutex, `kmalloc(GFP_KERNEL)`, `copy_*_user` — chỉ spinlock, `GFP_ATOMIC`, việc ngắn.
- **Top/bottom half**: top ack thiết bị + gom tối thiểu + lên lịch phần còn lại; bottom chạy sau với ngắt mở — softirq/tasklet (vẫn atomic) hoặc **workqueue** (process context, ngủ được).
- **Threaded IRQ** (`request_threaded_irq` + `IRQF_ONESHOT`): phần chính chạy trong **kernel thread riêng** — ngủ được, mutex được, **priority chỉnh được** (nền của PREEMPT_RT: hầu hết IRQ thành threaded, vòng RT của bạn có thể cao hơn "ngắt" card mạng).

→ đào sâu: [melp/drivers-init-power.md](../../16-book-summaries/melp/drivers-init-power.md), [ostep/concurrency.md](../../16-book-summaries/ostep/concurrency.md)
</details>

**C2. ⭐ Phân biệt coherent DMA và streaming DMA. Vì sao cần cache maintenance khi DMA?**

<details><summary>Đáp án</summary>

- **Vấn đề gốc:** DMA engine ghi/đọc **RAM trực tiếp, không qua cache CPU** (trên nhiều SoC ARM không có hardware coherency cho DMA) → hai bản sự thật: CPU nhìn cache, device nhìn RAM. Device vừa DMA dữ liệu vào RAM mà CPU đọc trúng **cache line cũ** → dữ liệu rác; CPU vừa ghi buffer (còn trong cache, chưa xuống RAM) mà bảo device đọc → device đọc đồ cũ.
- **Coherent (consistent) DMA** — `dma_alloc_coherent()`: cấp vùng **uncached** (hoặc qua đường coherent phần cứng) — CPU và device luôn thấy cùng dữ liệu, không cần bảo trì; đổi lại truy cập CPU chậm (không cache). Dùng cho: descriptor ring, control block — cấu trúc nhỏ hai bên chọc thường xuyên.
- **Streaming DMA** — `dma_map_single/sg()` trước khi giao device, `dma_unmap_*` sau khi xong: buffer cached bình thường, kernel **flush (to-device) / invalidate (from-device) cache** đúng lúc map/unmap theo `DMA_TO_DEVICE`/`FROM_DEVICE`. Dùng cho: payload lớn một chiều (packet, frame) — CPU xử lý nhanh nhờ cache, trả giá thao tác maintenance mỗi lần.
- Quy tắc kèm: **giữa map và unmap, CPU không được đụng buffer**; địa chỉ giao device là **dma_addr_t** (bus address — qua IOMMU/offset, không phải virtual, không phải cứ physical); buffer phải từ kmalloc/DMA-safe (không phải stack!).

→ đào sâu: nền lý thuyết cache tại [ostep/virtualization-memory.md](../../16-book-summaries/ostep/virtualization-memory.md); (chủ đề này sẽ dày thêm khi làm LDD3/Bootlin)
</details>

**C3. `ioremap` là gì? Vì sao không dùng thẳng địa chỉ vật lý của thanh ghi? `/dev/mem` đứng đâu?**

<details><summary>Đáp án</summary>

- Kernel chạy với **MMU bật** — mọi truy cập là virtual address; địa chỉ vật lý của thanh ghi (từ datasheet/`reg` trong DT) chưa được map thì deref = fault. **`ioremap()`** (driver dùng bản `devm_ioremap_resource`) tạo mapping virtual→physical cho vùng MMIO với thuộc tính đúng: **uncached, non-bufferable** (Device memory trên ARM) — thanh ghi mà bị cache/gộp/prefetch là sai hành vi ngay (đọc status 2 lần phải là 2 lần đọc thật — cùng lý do `volatile`, xem FILE 2 Q10).
- Đọc/ghi qua `readl/writel` (kèm memory barrier đúng chỗ) thay vì deref con trỏ trần — API chuẩn xử lý ordering + endianness.
- **`/dev/mem`**: cửa userspace mmap thẳng physical memory — **vũ khí bring-up** (devmem đọc/ghi thanh ghi kiểm tra clock/pinmux trước khi đổ lỗi driver) nhưng **cấm trong sản phẩm**: vượt mặt mọi driver đang chạy (tranh chấp trạng thái), lỗ hổng bảo mật toàn hệ (đọc được cả RAM), nhiều distro chặn bằng `CONFIG_STRICT_DEVMEM`.

→ đào sâu: [melp/drivers-init-power.md](../../16-book-summaries/melp/drivers-init-power.md)
</details>

**C4. 🏗️ Sensor báo dữ liệu qua ngắt 5kHz. Thiết kế đường dữ liệu từ ISR đến ứng dụng userspace.**

<details><summary>Đáp án (khung)</summary>

- 5kHz = 200µs/sự kiện — **không được** làm việc nặng trong ISR, càng không round-trip userspace mỗi sự kiện.
- Khung thiết kế: **ISR tối giản** (ack + đọc/kick DMA) → dữ liệu vào **ring buffer trong kernel** (hoặc DMA thẳng vào buffer vòng — tốt nhất: sensor→DMA→RAM không CPU) → báo userspace **theo lô**: wake `poll/read` khi đủ N mẫu hoặc timeout (giảm wakeup từ 5k/s xuống ~vài chục/s) → userspace `read()`/`mmap` lấy cả block.
- Chuẩn hóa: dùng subsystem sẵn — **IIO** (industrial I/O) có đúng mô hình này: trigger + kfifo buffer + watermark; đừng tự phát minh char device nếu IIO khớp.
- Các con số phải nêu khi chốt: tần suất wakeup userspace, độ trễ chấp nhận được (đánh đổi với batch size), chiến lược overflow (drop oldest + counter — phải *đo được* việc mất mẫu).
- Nếu latency từng mẫu là yêu cầu cứng (điều khiển): cân nhắc xử lý ngay trong threaded IRQ priority cao / đẩy xuống co-processor (Cortex-M) — đừng kéo lên userspace.

→ đào sâu: [melp/drivers-init-power.md](../../16-book-summaries/melp/drivers-init-power.md) (câu encoder cùng khung)
</details>

---

## PHẦN D — STORAGE FLASH & OTA UPDATE

**D1. ⭐ NAND thô + UBIFS vs eMMC + ext4/f2fs — trade-off và stack phần mềm từng bên? Vì sao không trộn chéo?**

<details><summary>Đáp án</summary>

- Câu định vị: **FTL nằm ở đâu?** eMMC: FTL trong chip (hộp đen lo wear/bad block/ECC) → host thấy block device → ext4/f2fs, tool chuẩn, đơn giản — mặc định hiện đại. NAND thô: host lo tất → stack `NAND controller (ECC) → MTD → UBI (wear leveling, bad block, logical erase block) → UBIFS` — rẻ/kiểm soát trọn, trả giá độ phức tạp.
- Không trộn chéo: **ext4 lên NAND thô** — ext4 giả định ghi-đè-tại-chỗ, không biết erase block/bad block/bit flip → hỏng từ nguyên lý (mtdblock không wear leveling). **UBIFS lên eMMC** — UBIFS cần thấy flash thô qua MTD; eMMC giấu sau FTL → không attach được, ép qua giả lập = hai tầng wear leveling chồng nhau phản tác dụng.
- f2fs = "block FS hiểu ý FTL" (ghi kiểu log-structured) — điểm giữa tốt cho eMMC.
- Chốt senior: dù chọn gì — **rootfs read-only + partition data riêng + test rút điện tự động**.

→ đào sâu: [melp/storage-update.md](../../16-book-summaries/melp/storage-update.md), nền FTL: [ostep/persistence.md](../../16-book-summaries/ostep/persistence.md)
</details>

**D2. ⭐🏗️ Thiết kế OTA update không được phép brick — trình bày đầy đủ. (Câu gần như chắc gặp)**

<details><summary>Đáp án</summary>

Trả lời theo 4 lớp:
1. **Bố cục storage**: bootloader+env (đóng băng) | kernel+rootfs **A** | **B** | data (ngoài A/B). Rootfs read-only.
2. **Luồng**: tải (resume được) → **verify chữ ký TRƯỚC khi ghi** → ghi bank không chạy → read-back verify → set cờ "thử B, N lần".
3. **Rollback tự động**: U-Boot đọc cờ + **bootcount** (giảm mỗi lần thử); hệ mới phải qua **health check** (dịch vụ chủ chốt chạy + self-test) rồi mới **commit** (xóa cờ); chết trước commit → hết N lần → boot lại A. **Watchdog phần cứng** phủ treo cứng (panic sớm không code nào chạy được — watchdog reset → bootcount làm việc).
4. **Bảo mật/vận hành**: chuỗi ký nối dài từ secure boot; **anti-rollback** version counter; data schema có version + đường lùi; rollout theo vòng (canary → toàn bộ); dùng framework sẵn (**RAUC/Mender/SWUpdate**).
- Bẫy chấm điểm: quên watchdog; commit ngay khi kernel lên (health check vô nghĩa); không verify trước khi ghi (ảnh rác đè mất bank dự phòng); user data trong bank A/B.
- Insight nếu được hỏi "nguyên lý": update = **journaling phóng to** — ghi chỗ riêng, commit bằng một hành động atomic nhỏ (đổi cờ), chưa commit thì rollback.

→ đào sâu: [melp/storage-update.md](../../16-book-summaries/melp/storage-update.md)
</details>

**D3. Vì sao ghi nhỏ rải rác có hại cho eMMC/SD? Bạn thiết kế logging trên thiết bị flash thế nào?**

<details><summary>Đáp án</summary>

- Flash erase theo block lớn, ghi theo page vào chỗ đã erase → ghi nhỏ rải rác làm block lốm đốm sống/chết → GC của FTL phải **chép page sống trước khi erase** → **write amplification** (1 ghi logic = nhiều ghi vật lý) → chậm + **mòn** (P/E cycles hữu hạn, TLC càng ít).
- Thiết kế logging: (1) **gom lô** — buffer RAM, flush theo chu kỳ/ngưỡng, không fsync từng dòng; (2) **ring buffer kích thước cố định** (logrotate cắt theo size) — không append vô hạn; (3) log "nóng" (debug dày) để tmpfs, chỉ đổ xuống flash khi có sự cố (flush-on-crash); (4) **budget TBW**: ước byte/ngày × WA × tuổi thọ so datasheet — ra quyết định bằng số; (5) bật TRIM/discard định kỳ; (6) linh kiện: eMMC công nghiệp/SLC-mode partition cho vùng ghi nóng.
- Bug thật đáng kể khi phỏng vấn: thiết bị chết hàng loạt sau 2 năm vì log 1 dòng/giây fsync liên tục lên SD rẻ — loại chuyện interviewer BSP nào cũng gặp/thích nghe cách phòng.

→ đào sâu: [ostep/persistence.md](../../16-book-summaries/ostep/persistence.md), [melp/storage-update.md](../../16-book-summaries/melp/storage-update.md)
</details>

---

## PHẦN E — BUILD SYSTEM & YOCTO

**E1. ⭐ Buildroot vs Yocto — chọn thế nào? Một BSP layer trong Yocto gồm những gì?**

<details><summary>Đáp án</summary>

- **Buildroot**: makefile+Kconfig sinh *một ảnh* — học nhanh, minh bạch, hợp sản phẩm đơn/đội nhỏ; không package manager, sstate thô sơ. **Yocto**: framework metadata sinh *một distro* — layer/override cho nhiều SKU, **sstate cache** (build lại chỉ phần đổi, share CI), **SDK** cho đội app, license/CVE tooling; giá = đường học dốc + hạ tầng build. Thực tế ngành: vendor (NXP/TI/ST) phát hành BSP dạng **Yocto layer** → về Yocto là mặc định khi sản phẩm nghiêm túc/nhiều biến thể.
- **BSP layer (`meta-<board>`)** gồm: `conf/machine/<board>.conf` (DTB nào — `KERNEL_DEVICETREE`, u-boot config, console, tune); recipe/append **kernel** (`linux-*.bbappend`: patch DT/driver, config fragment `.cfg`); recipe/append **U-Boot**; firmware blob; image recipe tham chiếu machine. Nguyên tắc: mọi tùy biến là **bbappend/patch có nguồn gốc trong layer riêng** — không sửa poky/vendor layer, không sửa `tmp/work` (bị nghiền lần build sau).

→ đào sâu: [melp/build-systems.md](../../16-book-summaries/melp/build-systems.md)
</details>

**E2. DEPENDS vs RDEPENDS? Vì sao build xong chạy thiếu lib dù compile không lỗi?**

<details><summary>Đáp án</summary>

- **DEPENDS** = build-time (headers/lib trong sysroot lúc compile, tool native); **RDEPENDS** = runtime (thứ phải **có mặt trong image** cùng package).
- Compile OK mà chạy thiếu: lib có trong sysroot lúc link nhưng không vào image. Với `.so` link trực tiếp Yocto tự bắt (shlibs scan) — nên ca lọt lưới điển hình là **`dlopen()`** (nạp runtime, scanner không thấy), plugin, gọi binary ngoài qua `system()`, hoặc package bị tách (`-libs`,`-dev`) mà image chỉ cài gói chính.
- Fix đúng chỗ: `RDEPENDS:${PN} += "libfoo"` trong recipe (dependency đi theo package) — không nhét thẳng vào image; soi bằng `oe-pkgdata-util`.

→ đào sâu: [melp/build-systems.md](../../16-book-summaries/melp/build-systems.md)
</details>

**E3. Cross-compile: binary chạy trên board báo `not found` dù file có mặt — chẩn đoán?**

<details><summary>Đáp án</summary>

- `not found` thật ra là **thiếu interpreter/lib**, không phải thiếu binary: (1) **dynamic loader sai** — binary đòi `/lib/ld-linux-aarch64.so.1` (ELF interp) mà rootfs dùng musl/thiếu loader (build toolchain glibc cho rootfs musl); (2) thiếu **NEEDED lib** trên rootfs; (3) sai kiến trúc/ABI (ARM32 hf vs soft-float, x86 vs ARM).
- Chẩn đoán tuần tự: `file ./bin` (arch + interpreter) → `ls /lib/ld-*` đối chiếu → `readelf -d | grep NEEDED` đối chiếu rootfs (trên board: `/lib/ld-*.so --list ./bin`) → fix: build đúng libc rootfs / cài đủ lib / static link (musl static cho tool nhỏ).
- Nêu thêm khái niệm gốc: **sysroot** — mọi rắc rối cross-compile quy về "cái này đến từ host hay sysroot?".

→ đào sâu: [melp/toolchain-rootfs.md](../../16-book-summaries/melp/toolchain-rootfs.md)
</details>

---

## PHẦN F — POWER MANAGEMENT

**F1. Bốn tầng power management của Linux? Suspend-to-RAM hoạt động thế nào, wakeup source là gì?**

<details><summary>Đáp án</summary>

- Bốn tầng: **cpufreq** (DVFS khi chạy — governor schedutil/ondemand/performance, OPP table trong DT) → **cpuidle** (rảnh ngủ C-state — sâu hơn tiết kiệm hơn nhưng exit latency cao) → **runtime PM** (từng thiết bị tự tắt khi không dùng — usage count, `runtime_suspend/resume` trong driver) → **system suspend** (cả hệ ngủ).
- **Suspend-to-RAM** (`echo mem > /sys/power/state`): freeze userspace → suspend device **theo thứ tự ngược cây thiết bị** (con trước cha — mỗi driver cần `.suspend/.resume` đúng) → tắt CPU phụ, CPU cuối vào trạng thái ngủ sâu; **RAM ở chế độ self-refresh** (giữ nội dung, tốn µA). Resume đi ngược lại.
- **Wakeup source**: thứ được phép đánh thức — khai `wakeup-source` trong DT/driver gọi `device_init_wakeup`: GPIO nút nhấn, RTC alarm, WoL... Debug thức không rõ lý do: `/sys/kernel/debug/wakeup_sources`, đếm `/proc/interrupts` delta.
- Bug PM kinh điển: **resume hỏng vì một driver thiếu resume handler** hoặc thứ tự clock — triệu chứng ngủ được mà dậy treo; công cụ `pm_test` mode chia đôi tầng.

→ đào sâu: [melp/drivers-init-power.md](../../16-book-summaries/melp/drivers-init-power.md)
</details>

---

## PHẦN G — REAL-TIME

**G1. ⭐ Linux có phải hệ real-time không? PREEMPT_RT làm gì?**

<details><summary>Đáp án</summary>

- Trả lời ba nấc: (1) **vanilla**: không có bảo đảm — latency đuôi có thể ms→chục ms (đoạn kernel không preempt được, IRQ chen); (2) **PREEMPT_RT tuned**: soft/firm RT — worst-case cỡ chục–trăm µs, *đo bằng cyclictest dưới tải*; (3) **hard RT an toàn tính mạng**: RTOS riêng hoặc kiến trúc lai (Cortex-M/AMP chạy vòng điều khiển, Linux làm phần giàu tính năng).
- PREEMPT_RT làm gì: **spinlock kernel → rt_mutex ngủ được** (đoạn giữ lock preempt được), **IRQ handler → kernel thread có priority** (vòng RT của bạn có thể cao hơn "ngắt" card mạng), priority inheritance phủ rộng, high-res timer. Đổi **throughput lấy chặn trên latency** (RT = deterministic, không phải nhanh). Mainline ~kernel 6.12.
- Bật RT chưa đủ — app phải: `SCHED_FIFO` (không phải 99), **`mlockall` + pre-fault**, mutex **PTHREAD_PRIO_INHERIT** (mặc định là NONE!), cấm malloc/I/O trong vòng RT, **isolcpus + IRQ affinity** dồn nhiễu sang core khác, giới hạn C-state; kiểm chứng **cyclictest Max dưới tải đại diện**.

→ đào sâu: [melp/debug-realtime.md](../../16-book-summaries/melp/debug-realtime.md)
</details>

**G2. ⭐ Priority inversion là gì? Priority inheritance chữa thế nào, và vì sao semaphore không chữa được?**

<details><summary>Đáp án</summary>

- **Inversion**: L (thấp) giữ mutex; H (cao) chờ mutex đó; M (trung, không liên quan) chiếm CPU của L → L không chạy để nhả → H chờ vô chừng — ưu tiên lộn ngược (Mars Pathfinder reset liên tục vì kịch bản này).
- **Priority inheritance**: khi H block, **chủ mutex (L) tạm nhận priority của H** → vượt M, chạy xong nhả, tụt về cũ — thời gian H chờ chặn trên = critical section của L. (Biến thể: priority ceiling.)
- **Semaphore không PI được** vì PI cần biết **chủ để nâng** — semaphore không có khái niệm ownership (A wait, B post hợp lệ). Đây là lý do kỹ thuật của quy tắc "mutex cho mutual exclusion, semaphore cho signaling" trong hệ RT.
- Chi tiết thực chiến ăn điểm: pthread mutex phải khai `PTHREAD_PRIO_INHERIT` tường minh.

→ đào sâu: [melp/debug-realtime.md](../../16-book-summaries/melp/debug-realtime.md), [ostep/concurrency.md](../../16-book-summaries/ostep/concurrency.md)
</details>

---

## PHẦN H — TÌNH HUỐNG BRING-UP & DEBUG (🏗️ trả lời theo khung)

**H1. ⭐🏗️ Board mới không lên gì trên console — trình tự debug?**

<details><summary>Đáp án (khung "chia đôi chuỗi boot")</summary>

1. **Phần cứng**: nguồn/PMIC rail (đo), reset, **boot strap pins** (lỗi #1), clock; UART đúng chân/baud (đảo TX-RX là lỗi #2).
2. **ROM sống không**: ép UART/USB boot mode, tool vendor bắt tay ROM (`imx_usb`…) — bắt tay được = SoC sống, lỗi ở nguồn boot/ảnh.
3. **SPL**: build debug UART sớm; banner ra mà chết tiếp → nghi **DDR init** (timing — chạy mem test của SPL).
4. **U-Boot**: env, bootargs, load address.
5. **Kernel im sau "Starting kernel"**: 90% `console=` sai hoặc UART thiếu clock/pinmux trong DT → **`earlycon`** lấy log giai đoạn mù.
6. Song song: JTAG attach xem PC; GPIO/LED làm "printf bằng chân".
- Điểm chấm: cấu trúc chia đôi + công cụ đúng từng đoạn, không phải liệt kê mẹo.

→ đào sâu: [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md)
</details>

**H2. 🏗️ Daemon crash ngẫu nhiên vài ngày/lần ngoài hiện trường, lab không tái hiện được?**

<details><summary>Đáp án (khung "hạ tầng bắt bằng chứng")</summary>

1. **Post-mortem trước**: bật core dump (`ulimit -c`, `core_pattern` vào partition data, giới hạn size/số), giữ binary+.so **chưa strip đúng build** (build ID khớp), thu core+log+version về. Một core tốt thường đủ đóng án.
2. Core không đủ (heap corruption chết xa nguồn): bản ASan chạy staging/QEMU song song; nghi race → TSan trên mô phỏng tải.
3. **Log có chủ đích**: ring buffer RAM, flush khi crash (không spam mòn flash); watchdog restart để khách không gián đoạn — nhưng **giữ core trước khi restart**.
4. Tương quan metadata các lần chết: uptime (leak?), giờ (cron/logrotate?), thao tác khách.
- Từ khóa chấm điểm: core dump + symbol khớp build ID; tư duy "hạ tầng bằng chứng" thay vì "thêm log rồi chờ".

→ đào sâu: [melp/debug-realtime.md](../../16-book-summaries/melp/debug-realtime.md)
</details>

**H3. 🏗️ Ứng dụng điều khiển thỉnh thoảng trễ chu kỳ (CPU trung bình thấp)?**

<details><summary>Đáp án (khung "tracing, không phải profiling")</summary>

- Spike hiếm + CPU trung bình thấp → **tracing** (`trace-cmd record -e sched* -e irq*`), app tự phát hiện trễ ghi **trace_marker** để tua đúng khoảnh khắc; mở kernelshark xem thread bị gì.
- 5 họ nghi phạm: (1) bị task khác chen (→ SCHED_FIFO/affinity); (2) IRQ/softirq storm (→ IRQ affinity, threaded IRQ hạ priority); (3) chờ lock — **priority inversion** (→ PI mutex) hoặc I/O đồng bộ trong đường RT (log ghi flash — lỗi kiến trúc kinh điển); (4) cpuidle exit latency/cpufreq tụt (→ giới hạn C-state, governor); (5) major page fault (→ mlockall).
- Chốt: cyclictest định lượng nền tảng; phân biệt profiling ("trung bình ai tốn") vs tracing ("khoảnh khắc đó chuyện gì").

→ đào sâu: [melp/debug-realtime.md](../../16-book-summaries/melp/debug-realtime.md)
</details>

**H4. 🏗️ Thiết bị chạy vài tuần thì chậm dần rồi bị kill — hướng điều tra?**

<details><summary>Đáp án (khung memory)</summary>

- Triệu chứng khớp **memory leak → OOM killer** (Linux overcommit: malloc không fail, chết lúc chạm trang): xác nhận bằng `dmesg` (OOM log có bảng điểm process) + đồ thị **PSS theo thời gian** từng process (`smaps_rollup`, smem — RSS cộng dồn đánh lừa vì đếm trùng shared).
- Khoanh thủ phạm: process nào PSS tăng tuyến tính; trong process: valgrind/ASan ở staging, hoặc so `smaps` theo vùng (heap? mmap? — heap tăng = leak malloc, số mapping tăng = leak mmap/fd — kiểm `ls /proc/pid/fd | wc`).
- Đừng quên nghi phạm ngoài heap: **page cache đầy là bình thường** (không phải leak — "free thấp" ≠ hết RAM); kernel leak (slab tăng — `/proc/meminfo`, slabtop); tmpfs phình (log vào /tmp!); fragmentation.
- Phòng bị: cgroup memory limit per-service (chết cục bộ + restart), `oom_score_adj` bảo vệ process điều khiển, PSI monitoring cảnh báo sớm.

→ đào sâu: [ostep/virtualization-memory.md](../../16-book-summaries/ostep/virtualization-memory.md), [melp/debug-realtime.md](../../16-book-summaries/melp/debug-realtime.md)
</details>

---

## BẢNG MAP ÔN NHANH — câu hỏi ↔ tài liệu sâu

| Mảng | Câu ở file này | Đọc sâu |
|---|---|---|
| Boot chain, U-Boot, FIT | A1–A4 | [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md) |
| Device tree, probe, pinctrl | B1–B4 | [melp/bootloader-kernel.md](../../16-book-summaries/melp/bootloader-kernel.md) |
| Interrupt, DMA, MMIO | C1–C4 | [melp/drivers-init-power.md](../../16-book-summaries/melp/drivers-init-power.md) + [ostep/concurrency.md](../../16-book-summaries/ostep/concurrency.md) |
| Flash, OTA | D1–D3 | [melp/storage-update.md](../../16-book-summaries/melp/storage-update.md) + [ostep/persistence.md](../../16-book-summaries/ostep/persistence.md) |
| Yocto, cross-compile | E1–E3 | [melp/build-systems.md](../../16-book-summaries/melp/build-systems.md), [melp/toolchain-rootfs.md](../../16-book-summaries/melp/toolchain-rootfs.md) |
| Power | F1 | [melp/drivers-init-power.md](../../16-book-summaries/melp/drivers-init-power.md) |
| Real-time | G1–G2 | [melp/debug-realtime.md](../../16-book-summaries/melp/debug-realtime.md) |
| Tình huống | H1–H4 | theo từng câu |

> Nền chung (C++/IPC/bus/GDB cơ bản): xem [02_question_bank.md](02_question_bank.md). Lý thuyết hệ thống: [01_theory.md](01_theory.md).
