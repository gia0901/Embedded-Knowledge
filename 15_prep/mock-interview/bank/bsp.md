# BSP — Board Support Package (chuyên sâu)

> Domain `BSP`. Gộp từ [technical_round/04_bsp_question_bank.md](../../technical_round/04_bsp_question_bank.md) + phần BSP của [02](../../technical_round/02_question_bank.md). Khoan sâu: boot chain, device tree/probe, interrupt/DMA/MMIO, flash & OTA, Yocto, power, real-time, bring-up. Track dùng: `bsp`, `drivers-dt`, `melp`, `lkd`.
> 🏗️ = câu thiết kế/tình huống — chấm theo **khung**, không đáp án duy nhất.

---

## A — Boot chain & bootloader

#### BSP-001 · 🟡 · concept · ⭐ · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**HAL là gì, giải quyết vấn đề gì?**
<details><summary>Đáp án</summary>

**HAL (Hardware Abstraction Layer)** = lớp giữa ứng dụng và phần cứng, phơi ra **interface ổn định** theo *chức năng* ("đọc nhiệt độ", "gửi frame") thay vì theo *chip* ("ghi thanh ghi 0x40 của ADC XYZ").

**Vấn đề nó giải — chi phí đổi phần cứng.** Không có HAL, chi tiết chip rò rỉ khắp codebase; đổi chipset (EOL, thiếu hàng, hạ giá thành) phải sửa **mọi nơi**. Có HAL, ranh giới thay đổi bị **khoanh vùng**: chỉ viết một implementation mới.

```
┌─────────────── Application / business logic ─────────────┐   ← không đổi
├──────────── HAL interface (thuần chức năng) ─────────────┤   ← hợp đồng ổn định
│  SensorX impl │ SensorY impl │ MockSensor (unit test) │      ← chỉ tầng này thay
└────────────── thanh ghi / driver / SoC ──────────────────┘
```

```cpp
class ITempSensor {                       // interface ổn định
public:
    virtual ~ITempSensor() = default;
    virtual float readCelsius() = 0;
};
class Bmp280 : public ITempSensor { … };  // chipset A
class Sht31  : public ITempSensor { … };  // chipset B — đổi chip = thêm 1 class
class MockTempSensor : public ITempSensor { … };   // ⭐ test không cần phần cứng
```

**Ba lợi ích, và lợi ích thứ ba mới là thứ interviewer muốn nghe:**
1. **Portability** — đổi chipset/SoC chỉ thay implementation.
2. **Phân chia công việc** — team app và team BSP làm song song sau khi chốt interface.
3. ⭐ **Testability** — cắm mock vào chỗ phần cứng → **unit test chạy trên host, trong CI**, không cần board. Đây thường là giá trị lớn nhất trong thực tế ([SD-007](system-design.md), [DP-011](design-patterns.md)).

**Đánh đổi (nên chủ động nêu):** thêm một lớp gián tiếp — với virtual thì tốn vptr + chặn inline; trên MCU chật có thể thay bằng **template/CRTP** hoặc con trỏ hàm trong struct (kiểu C, như `struct file_operations` của Linux). Và HAL **quá tổng quát** sẽ hoặc là mẫu số chung nghèo nàn, hoặc rò rỉ chi tiết chip qua interface — thiết kế theo *ca sử dụng thật*, đừng cố phủ mọi khả năng của chip.

**Chốt:** *"HAL đóng băng cái *làm gì*, cô lập cái *làm thế nào*. Đo chất lượng một HAL bằng câu hỏi: đổi chipset thì phải sửa bao nhiêu file — và có test được khi không có board không?"*
</details>

#### BSP-002 · 🟠 · concept · ⭐ · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**Kể chi tiết quá trình boot từ lúc cấp nguồn đến shell, và giải thích VÌ SAO phải nhiều giai đoạn.**
<details><summary>Đáp án</summary>

- Chuỗi: **ROM code** (trong silicon — đọc boot pins chọn nguồn, nạp giai đoạn kế vào **SRAM nội**) → **SPL** (vừa SRAM; việc chính: **init DRAM** + clock/PMIC tối thiểu, nạp U-Boot vào DRAM) → **U-Boot proper** (init storage/console, đọc env, nạp kernel Image + **DTB** + initramfs vào RAM, `booti` với con trỏ DTB trong x0) → **kernel** (parse DTB, bật MMU, probe driver, mount rootfs theo `root=`) → **init PID 1** → service → shell.
- Vì sao nhiều giai đoạn: **chuỗi bootstrap tài nguyên** — lúc cấp nguồn DRAM chưa hoạt động, chỉ có SRAM vài chục–trăm KB; U-Boot đầy đủ không vừa SRAM → cần SPL đủ nhỏ để dựng DRAM trước. Mỗi tầng chỉ đủ khả năng dựng tầng kế.
- ARM64 + secure boot: chuỗi TF-A `BL1→BL2(≈SPL)→BL31(secure monitor, PSCI, ở lại runtime)→BL33(U-Boot)`, mỗi bước **verify chữ ký** bước sau = chain of trust từ khóa trong eFuse.
- Mốc debug từng đoạn: không gì trên UART = trước SPL; banner U-Boot mà không "Starting kernel" = load/bootargs; dừng ở VFS = root=.
</details>

#### BSP-003 · 🟠 · concept · ⭐ · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**`bootargs` gồm những gì quan trọng? Kể 3 lỗi bootargs kinh điển và triệu chứng.**
<details><summary>Đáp án</summary>

- `bootargs` = **kernel command line** — hợp đồng bootloader→kernel: `console=ttymxc0,115200` (console), `earlycon` (console siêu sớm không cần driver), `root=/dev/mmcblk0p2` + `rootwait` (rootfs ở đâu, chờ device), `rootfstype=`, `init=`, `loglevel=`, `mem=`, `nfsroot=`/`ip=` (NFS boot).
- Ba lỗi kinh điển:
  1. **Sai `console=`** (tên device của kernel khác U-Boot: ttyS0 vs ttymxc0 vs ttyAMA0) → kernel boot **hoàn toàn im lặng** sau "Starting kernel" dù hệ có thể vẫn lên. Chẩn đoán: thêm `earlycon`.
  2. **Sai `root=`/thiếu driver FS** → panic `VFS: Unable to mount root fs` — đọc dòng "available partitions" ngay trên panic để đối chiếu.
  3. **Thiếu `rootwait`** với eMMC/SD/USB → kernel tìm root *trước khi* storage enumerate xong → panic chập chờn tùy lần boot.
</details>

#### BSP-004 · 🟡 · concept · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**FIT image là gì, hơn uImage cũ chỗ nào?**
<details><summary>Đáp án</summary>

- **FIT (Flattened Image Tree)**: một file `.itb` đóng gói **kernel + nhiều DTB + initramfs + configuration**, mô tả bằng nguồn `.its`, build bằng `mkimage`.
- Hơn uImage (chỉ bọc một ảnh + header CRC): (1) **nhiều thành phần + nhiều cấu hình** — một ảnh phục vụ nhiều biến thể board; (2) **hash/chữ ký từng thành phần** (SHA + RSA) — nền của **verified boot** trong U-Boot; (3) metadata rõ ràng (load address, entry, compression).
- Thực dụng: sản phẩm cần secure boot hoặc nhiều SKU chung firmware → FIT gần như bắt buộc.
</details>

#### BSP-005 · 🔴 · design · 🏗️ · [→ melp/storage-update](../../../16-book-summaries/melp/storage-update.md)
**Bootloader có được OTA update không? Rủi ro và cách làm đúng.**
<details><summary>Đáp án</summary>

- Nguyên tắc: **hạn chế tối đa** — bootloader là mắt xích không có ai đứng sau: ghi dở SPL/U-Boot = ROM code không còn gì hợp lệ để nạp = brick sâu (chỉ cứu bằng UART/USB boot mode ở xưởng).
- Nếu buộc phải update, dùng cơ chế **phần cứng có dự phòng**: eMMC có **2 boot partition** — ghi bản mới vào partition không active, verify, rồi đổi `mmc bootpart`; một số SoC hỗ trợ nhiều bản ảnh + fallback trong ROM. Không có → cân nhắc *không* update bootloader ngoài hiện trường.
- Thiết kế phòng ngừa: bootloader **tối giản và đóng băng** — mọi logic hay đổi (chọn bank A/B, health check) đẩy lên kernel/userspace hoặc thành script/env thay được không đụng binary.
</details>

---

## B — Kernel & Device Tree

#### BSP-006 · 🟠 · concept · ⭐ · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**Trình bày đường đi từ một node device tree đến hàm `probe()` của driver. EPROBE_DEFER là gì?**
<details><summary>Đáp án</summary>

- Bootloader trao DTB → kernel parse, dựng **platform_device** cho các node. Driver đăng ký `platform_driver` với **`of_match_table`** (danh sách `compatible`). Kernel match node ↔ driver theo compatible (so từ chuỗi cụ thể nhất) → gọi **`probe(pdev)`**: đọc `reg` → `devm_ioremap_resource`, `interrupts` → `devm_request_irq`, `clocks/…-supply` → clk/regulator API, property riêng → `device_property_read_*`, rồi đăng ký subsystem.
- **EPROBE_DEFER**: probe cần tài nguyên mà driver cung cấp nó **chưa probe** (clock, regulator, GPIO expander) → return `-EPROBE_DEFER` → kernel xếp lại, **probe lại sau mỗi lần có driver mới xong** — thứ tự tự hội tụ, không hard-code. Thiết bị "mất tích" không lỗi → xem `/sys/kernel/debug/devices_deferred`.
- Bẫy hay bị bắt bẻ: DT **không nạp driver** — driver phải có sẵn (built-in, hoặc module udev nạp theo modalias/uevent, cần `MODULE_DEVICE_TABLE(of, …)`); DT chỉ là dữ liệu khớp nối.
</details>

#### BSP-007 · 🟠 · design · ⭐ · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**Thêm một thiết bị I2C mới vào board đang chạy: các bước và các lỗi thường gặp.**
<details><summary>Đáp án</summary>

- Các bước: (1) tìm driver theo compatible trong kernel, bật CONFIG; (2) sửa DT: trong `&i2cN` (đủ `status="okay"` + pinctrl), thêm node con `compatible` + `reg=<addr>` (+ interrupt/supply nếu dùng), đúng **bindings YAML** (`make dtbs_check`); (3) build **chỉ dtbs**, deploy DTB (hoặc overlay `.dtbo`); (4) kiểm chứng theo tầng: `i2cdetect` → dmesg probe → node trong `/sys/bus/i2c/devices/` → đọc giá trị.
- Lỗi thường gặp: sai địa chỉ I2C (strap pin đổi addr); **quên pinmux** SDA/SCL (bus timeout); thiếu pull-up phần cứng; driver để `m` mà module không có trên rootfs; `vcc-supply` trỏ regulator disabled → **EPROBE_DEFER treo vĩnh viễn**; compatible gõ sai (match là so chuỗi tuyệt đối); i2cdetect thấy mà driver không bind → gần như chắc chắn lỗi compatible/CONFIG.
</details>

#### BSP-008 · 🟡 · concept · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**Driver nên built-in (`y`) hay module (`m`)? Tiêu chí và ví dụ nghịch lý gà-trứng.**
<details><summary>Đáp án</summary>

- Tiêu chí: cần **trước khi mount rootfs** (storage controller, FS của root, console) → `y` hoặc `m`-trong-initramfs; phần cứng tùy chọn/nhiều SKU → `m` (nạp theo modalias); hệ ít RAM → nghiêng `y` toàn bộ, có thể tắt hạ tầng module.
- Nghịch lý gà-trứng: **driver eMMC là module nằm trên rootfs trong chính eMMC** → không bao giờ nạp được. Giải: built-in, hoặc bỏ module vào initramfs.
- Cộng điểm: console driver phải built-in; `CONFIG_MODULES=n` là lựa chọn sản phẩm hợp lệ.
</details>

#### BSP-009 · 🟠 · concept · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**Pinctrl trong device tree đóng vai trò gì? Triệu chứng khi thiếu/sai pinmux?**
<details><summary>Đáp án</summary>

- Mỗi chân SoC đa năng (một pad → GPIO/UART/I2C/PWM… chọn bằng mux register). **pinctrl** mô tả cấu hình pad trong DT: nhóm `pinctrl_uart1 { fsl,pins = <...> }`, node thiết bị tham chiếu qua `pinctrl-names="default"` + `pinctrl-0=<&pinctrl_uart1>` — driver core tự áp khi probe.
- Triệu chứng thiếu/sai: **driver probe thành công, phần mềm chạy "bình thường", nhưng tín hiệu không ra chân** — nhóm lỗi khó chịu nhất vì mọi log sạch. UART câm, I2C timeout, LED không sáng. Chẩn đoán: scope/logic analyzer + đọc thanh ghi mux (`devmem`) đối chiếu datasheet; nhớ **U-Boot chạy được không có nghĩa kernel đúng** — kernel áp lại pinmux theo DT của nó.
- Bộ ba lỗi DT thực chiến: quên `status="okay"`, pinmux sai, EPROBE_DEFER vòng.
</details>

---

## C — Interrupt, DMA & MMIO

#### BSP-010 · 🟠 · concept · ⭐ · [→ melp/drivers-init-power](../../../16-book-summaries/melp/drivers-init-power.md), [ostep/concurrency](../../../16-book-summaries/ostep/concurrency.md)
**Vì sao interrupt handler không được ngủ? Threaded IRQ / top-bottom half giải quyết gì?**
<details><summary>Đáp án</summary>

- Handler chạy trong **interrupt context** — không có task_struct, không schedule được; "ngủ" = gọi scheduler nhường CPU chờ wake — không có chỗ trong hàng đợi scheduler → `scheduling while atomic`/treo. Kèm: dòng IRQ đang mask, ngủ lâu là mất ngắt/deadlock. Hệ quả: cấm mutex, `kmalloc(GFP_KERNEL)`, `copy_*_user` — chỉ spinlock, `GFP_ATOMIC`, việc ngắn.
- **Top/bottom half**: top ack thiết bị + gom tối thiểu + lên lịch phần còn lại; bottom chạy sau với ngắt mở — softirq/tasklet (vẫn atomic) hoặc **workqueue** (process context, ngủ được).
- **Threaded IRQ** (`request_threaded_irq` + `IRQF_ONESHOT`): phần chính chạy trong **kernel thread riêng** — ngủ được, mutex được, **priority chỉnh được** (nền của PREEMPT_RT).
</details>

#### BSP-011 · 🟠 · concept · ⭐ · [→ ostep/virtualization-memory](../../../16-book-summaries/ostep/virtualization-memory.md)
**Phân biệt coherent DMA và streaming DMA. Vì sao cần cache maintenance khi DMA?**
<details><summary>Đáp án</summary>

- **Vấn đề gốc:** DMA engine ghi/đọc **RAM trực tiếp, không qua cache CPU** (nhiều SoC ARM không có hardware coherency cho DMA) → hai bản sự thật: CPU nhìn cache, device nhìn RAM. Device DMA vào RAM mà CPU đọc trúng **cache line cũ** → rác; CPU ghi buffer (còn trong cache) mà bảo device đọc → device đọc đồ cũ.
- **Coherent DMA** — `dma_alloc_coherent()`: cấp vùng **uncached** — CPU và device luôn thấy cùng dữ liệu, không cần bảo trì; đổi lại truy cập CPU chậm. Dùng cho: descriptor ring, control block.
- **Streaming DMA** — `dma_map_single/sg()` trước khi giao device, `dma_unmap_*` sau: buffer cached, kernel **flush (to-device) / invalidate (from-device)** đúng lúc map/unmap theo `DMA_TO_DEVICE`/`FROM_DEVICE`. Dùng cho: payload lớn một chiều (packet, frame).
- Quy tắc: **giữa map và unmap CPU không đụng buffer**; địa chỉ giao device là **dma_addr_t** (bus address); buffer từ kmalloc/DMA-safe (không phải stack!).
</details>

#### BSP-012 · 🟠 · concept · [→ melp/drivers-init-power](../../../16-book-summaries/melp/drivers-init-power.md)
**`ioremap` là gì? Vì sao không dùng thẳng địa chỉ vật lý của thanh ghi? `/dev/mem` đứng đâu?**
<details><summary>Đáp án</summary>

- Kernel chạy với **MMU bật** — mọi truy cập là virtual address; địa chỉ vật lý thanh ghi (từ `reg` trong DT) chưa map thì deref = fault. **`ioremap()`** (driver dùng `devm_ioremap_resource`) tạo mapping virtual→physical cho vùng MMIO với thuộc tính đúng: **uncached, non-bufferable** (Device memory trên ARM) — thanh ghi bị cache/gộp/prefetch là sai (đọc status 2 lần phải là 2 lần đọc thật — cùng lý do `volatile`).
- Đọc/ghi qua `readl/writel` (kèm memory barrier) thay vì deref con trỏ trần — API chuẩn xử lý ordering + endianness.
- **`/dev/mem`**: cửa userspace mmap thẳng physical — **vũ khí bring-up** (devmem kiểm tra clock/pinmux trước khi đổ lỗi driver) nhưng **cấm trong sản phẩm** (vượt mặt driver, lỗ hổng bảo mật toàn hệ; chặn bằng `CONFIG_STRICT_DEVMEM`).
</details>

#### BSP-013 · 🔴 · design · 🏗️ · [→ melp/drivers-init-power](../../../16-book-summaries/melp/drivers-init-power.md)
**Sensor báo dữ liệu qua ngắt 5kHz. Thiết kế đường dữ liệu từ ISR đến ứng dụng userspace.**
<details><summary>Đáp án (khung)</summary>

- 5kHz = 200µs/sự kiện — **không** làm việc nặng trong ISR, càng không round-trip userspace mỗi sự kiện.
- Khung: **ISR tối giản** (ack + đọc/kick DMA) → dữ liệu vào **ring buffer trong kernel** (hoặc DMA thẳng vào buffer vòng — tốt nhất: sensor→DMA→RAM không CPU) → báo userspace **theo lô**: wake `poll/read` khi đủ N mẫu hoặc timeout (giảm wakeup từ 5k/s xuống vài chục/s) → userspace `read()`/`mmap` lấy cả block.
- Chuẩn hóa: dùng subsystem **IIO** (industrial I/O) có sẵn mô hình trigger + kfifo + watermark; đừng tự phát minh char device.
- Con số phải nêu: tần suất wakeup, độ trễ chấp nhận (đổi với batch size), chiến lược overflow (drop oldest + counter đo được).
- Nếu latency từng mẫu là yêu cầu cứng: xử lý trong threaded IRQ priority cao / đẩy xuống co-processor — đừng kéo lên userspace.
</details>

---

## D — Storage flash & OTA

#### BSP-014 · 🟠 · concept · ⭐ · [→ melp/storage-update](../../../16-book-summaries/melp/storage-update.md), [ostep/persistence](../../../16-book-summaries/ostep/persistence.md)
**NAND thô + UBIFS vs eMMC + ext4/f2fs — trade-off và stack phần mềm? Vì sao không trộn chéo?**
<details><summary>Đáp án</summary>

- Câu định vị: **FTL nằm ở đâu?** eMMC: FTL trong chip (lo wear/bad block/ECC) → host thấy block device → ext4/f2fs, tool chuẩn — mặc định hiện đại. NAND thô: host lo tất → stack `NAND controller (ECC) → MTD → UBI (wear leveling, bad block, logical erase block) → UBIFS` — rẻ/kiểm soát trọn, trả giá độ phức tạp.
- Không trộn chéo: **ext4 lên NAND thô** — ext4 giả định ghi-đè-tại-chỗ, không biết erase block/bad block/bit flip → hỏng từ nguyên lý. **UBIFS lên eMMC** — UBIFS cần thấy flash thô qua MTD; eMMC giấu sau FTL → hai tầng wear leveling chồng nhau phản tác dụng.
- f2fs = "block FS hiểu ý FTL" (log-structured) — điểm giữa tốt cho eMMC.
- Chốt senior: dù chọn gì — **rootfs read-only + partition data riêng + test rút điện tự động**.
</details>

#### BSP-015 · 🔴 · design · 🏗️ · ⭐ · [→ melp/storage-update](../../../16-book-summaries/melp/storage-update.md)
**Thiết kế OTA update không được phép brick — trình bày đầy đủ.**
<details><summary>Đáp án</summary>

Trả lời theo 4 lớp:
1. **Bố cục storage**: bootloader+env (đóng băng) | kernel+rootfs **A** | **B** | data (ngoài A/B). Rootfs read-only.
2. **Luồng**: tải (resume được) → **verify chữ ký TRƯỚC khi ghi** → ghi bank không chạy → read-back verify → set cờ "thử B, N lần".
3. **Rollback tự động**: U-Boot đọc cờ + **bootcount** (giảm mỗi lần thử); hệ mới phải qua **health check** (dịch vụ chủ chốt + self-test) rồi mới **commit** (xóa cờ); chết trước commit → hết N lần → boot lại A. **Watchdog phần cứng** phủ treo cứng.
4. **Bảo mật/vận hành**: chuỗi ký nối từ secure boot; **anti-rollback** version counter; data schema có version + đường lùi; rollout theo vòng (canary → toàn bộ); dùng framework sẵn (**RAUC/Mender/SWUpdate**).
- Bẫy chấm điểm: quên watchdog; commit ngay khi kernel lên; không verify trước khi ghi; user data trong bank A/B.
- Insight: update = **journaling phóng to** — ghi chỗ riêng, commit bằng một hành động atomic nhỏ (đổi cờ), chưa commit thì rollback.
</details>

#### BSP-016 · 🟠 · design · [→ ostep/persistence](../../../16-book-summaries/ostep/persistence.md), [melp/storage-update](../../../16-book-summaries/melp/storage-update.md)
**Vì sao ghi nhỏ rải rác có hại cho eMMC/SD? Thiết kế logging trên thiết bị flash thế nào?**
<details><summary>Đáp án</summary>

- Flash erase theo block lớn, ghi theo page vào chỗ đã erase → ghi nhỏ rải rác làm block lốm đốm → GC của FTL **chép page sống trước khi erase** → **write amplification** → chậm + **mòn** (P/E cycles hữu hạn).
- Thiết kế logging: (1) **gom lô** — buffer RAM, flush theo chu kỳ/ngưỡng, không fsync từng dòng; (2) **ring buffer kích thước cố định** (logrotate theo size); (3) log "nóng" để tmpfs, chỉ đổ flash khi có sự cố (flush-on-crash); (4) **budget TBW**: ước byte/ngày × WA × tuổi thọ so datasheet; (5) TRIM/discard định kỳ; (6) linh kiện: eMMC công nghiệp/SLC-mode cho vùng ghi nóng.
- Bug thật: thiết bị chết hàng loạt sau 2 năm vì log 1 dòng/giây fsync liên tục lên SD rẻ.
</details>

---

## E — Build system & Yocto

#### BSP-017 · 🟠 · concept · ⭐ · [→ melp/build-systems](../../../16-book-summaries/melp/build-systems.md)
**Buildroot vs Yocto — chọn thế nào? Một BSP layer trong Yocto gồm những gì?**
<details><summary>Đáp án</summary>

- **Buildroot**: makefile+Kconfig sinh *một ảnh* — học nhanh, minh bạch, hợp sản phẩm đơn/đội nhỏ; không package manager, sstate thô sơ. **Yocto**: framework metadata sinh *một distro* — layer/override cho nhiều SKU, **sstate cache** (build lại chỉ phần đổi), **SDK** cho đội app, license/CVE tooling; giá = đường học dốc. Thực tế: vendor (NXP/TI/ST) phát hành BSP dạng **Yocto layer** → Yocto là mặc định khi sản phẩm nghiêm túc/nhiều biến thể.
- **BSP layer (`meta-<board>`)** gồm: `conf/machine/<board>.conf` (DTB nào — `KERNEL_DEVICETREE`, u-boot config, console, tune); recipe/append **kernel** (`linux-*.bbappend`: patch DT/driver, config fragment `.cfg`); recipe/append **U-Boot**; firmware blob; image recipe. Nguyên tắc: mọi tùy biến là **bbappend/patch trong layer riêng** — không sửa poky/vendor layer, không sửa `tmp/work`.
</details>

#### BSP-018 · 🟡 · concept · [→ melp/build-systems](../../../16-book-summaries/melp/build-systems.md)
**DEPENDS vs RDEPENDS? Vì sao build xong chạy thiếu lib dù compile không lỗi?**
<details><summary>Đáp án</summary>

- **DEPENDS** = build-time (headers/lib trong sysroot lúc compile, tool native); **RDEPENDS** = runtime (thứ phải **có mặt trong image**).
- Compile OK mà chạy thiếu: lib có trong sysroot lúc link nhưng không vào image. Với `.so` link trực tiếp Yocto tự bắt (shlibs scan) — ca lọt lưới điển hình là **`dlopen()`** (nạp runtime, scanner không thấy), plugin, gọi binary ngoài qua `system()`, hoặc package tách (`-libs`,`-dev`).
- Fix: `RDEPENDS:${PN} += "libfoo"` trong recipe — không nhét thẳng vào image; soi bằng `oe-pkgdata-util`.
</details>

#### BSP-019 · 🟠 · concept · [→ melp/toolchain-rootfs](../../../16-book-summaries/melp/toolchain-rootfs.md)
**Cross-compile: binary chạy trên board báo `not found` dù file có mặt — chẩn đoán?**
<details><summary>Đáp án</summary>

- `not found` thật ra là **thiếu interpreter/lib**, không phải thiếu binary: (1) **dynamic loader sai** — binary đòi `/lib/ld-linux-aarch64.so.1` mà rootfs dùng musl/thiếu loader; (2) thiếu **NEEDED lib** trên rootfs; (3) sai kiến trúc/ABI (ARM32 hf vs soft-float).
- Chẩn đoán tuần tự: `file ./bin` (arch + interpreter) → `ls /lib/ld-*` đối chiếu → `readelf -d | grep NEEDED` đối chiếu rootfs → fix: build đúng libc rootfs / cài đủ lib / static link.
- Khái niệm gốc: **sysroot** — mọi rắc rối cross-compile quy về "cái này đến từ host hay sysroot?".
</details>

---

## F — Power management

#### BSP-020 · 🟠 · concept · [→ melp/drivers-init-power](../../../16-book-summaries/melp/drivers-init-power.md)
**Bốn tầng power management của Linux? Suspend-to-RAM hoạt động thế nào, wakeup source là gì?**
<details><summary>Đáp án</summary>

- Bốn tầng: **cpufreq** (DVFS khi chạy — governor, OPP table trong DT) → **cpuidle** (rảnh ngủ C-state — sâu hơn tiết kiệm hơn nhưng exit latency cao) → **runtime PM** (từng thiết bị tự tắt khi không dùng — usage count, `runtime_suspend/resume`) → **system suspend** (cả hệ ngủ).
- **Suspend-to-RAM** (`echo mem > /sys/power/state`): freeze userspace → suspend device **theo thứ tự ngược cây thiết bị** (mỗi driver cần `.suspend/.resume`) → tắt CPU phụ, CPU cuối vào ngủ sâu; **RAM ở self-refresh** (giữ nội dung, tốn µA). Resume đi ngược.
- **Wakeup source**: khai `wakeup-source` trong DT/driver `device_init_wakeup`: GPIO nút nhấn, RTC alarm, WoL. Debug: `/sys/kernel/debug/wakeup_sources`, delta `/proc/interrupts`.
- Bug PM kinh điển: resume treo vì một driver thiếu resume handler / sai thứ tự clock; công cụ `pm_test` chia đôi tầng.
</details>

---

## G — Real-time

#### BSP-021 · 🔴 · concept · ⭐ · [→ melp/debug-realtime](../../../16-book-summaries/melp/debug-realtime.md)
**Linux có phải hệ real-time không? PREEMPT_RT làm gì? (Vì sao vanilla không hard-RT?)**
<details><summary>Đáp án</summary>

- Ba nấc: (1) **vanilla**: không bảo đảm — latency đuôi ms→chục ms (đoạn kernel không preempt được, IRQ chen, page fault, lock contention); (2) **PREEMPT_RT tuned**: soft/firm RT — worst-case chục–trăm µs, *đo bằng cyclictest dưới tải*; (3) **hard RT an toàn tính mạng**: RTOS riêng hoặc kiến trúc lai (Cortex-M/AMP).
- PREEMPT_RT làm gì: **spinlock kernel → rt_mutex ngủ được** (đoạn giữ lock preempt được), **IRQ handler → kernel thread có priority**, priority inheritance phủ rộng, high-res timer. Đổi **throughput lấy chặn trên latency** (RT = deterministic, không phải nhanh). Mainline ~6.12.
- Bật RT chưa đủ — app phải: `SCHED_FIFO`, **`mlockall` + pre-fault**, mutex **PTHREAD_PRIO_INHERIT**, cấm malloc/I/O trong vòng RT, **isolcpus + IRQ affinity**, giới hạn C-state; kiểm chứng **cyclictest Max dưới tải đại diện**.
</details>

---

## H — Tình huống bring-up & debug (🏗️ trả lời theo khung)

#### BSP-022 · 🔴 · design · 🏗️ · ⭐ · [→ melp/bootloader-kernel](../../../16-book-summaries/melp/bootloader-kernel.md)
**Board mới không lên gì trên console — trình tự debug?**
<details><summary>Đáp án (khung "chia đôi chuỗi boot")</summary>

1. **Phần cứng**: nguồn/PMIC rail (đo), reset, **boot strap pins** (lỗi #1), clock; UART đúng chân/baud (đảo TX-RX là lỗi #2).
2. **ROM sống không**: ép UART/USB boot mode, tool vendor bắt tay ROM (`imx_usb`…) — bắt tay được = SoC sống.
3. **SPL**: build debug UART sớm; banner ra mà chết tiếp → nghi **DDR init** (chạy mem test của SPL).
4. **U-Boot**: env, bootargs, load address.
5. **Kernel im sau "Starting kernel"**: 90% `console=` sai hoặc UART thiếu clock/pinmux trong DT → **`earlycon`** lấy log giai đoạn mù.
6. Song song: JTAG attach xem PC; GPIO/LED làm "printf bằng chân".
- Điểm chấm: cấu trúc chia đôi + công cụ đúng từng đoạn, không phải liệt kê mẹo.
</details>

#### BSP-023 · 🔴 · design · 🏗️ · [→ melp/debug-realtime](../../../16-book-summaries/melp/debug-realtime.md)
**Daemon crash ngẫu nhiên vài ngày/lần ngoài hiện trường, lab không tái hiện được?**
<details><summary>Đáp án (khung "hạ tầng bắt bằng chứng")</summary>

1. **Post-mortem trước**: bật core dump (`ulimit -c`, `core_pattern` vào partition data), giữ binary+.so **chưa strip đúng build** (build ID khớp), thu core+log+version về. Một core tốt thường đủ đóng án.
2. Core không đủ (heap corruption chết xa nguồn): bản ASan chạy staging/QEMU song song; nghi race → TSan trên mô phỏng tải.
3. **Log có chủ đích**: ring buffer RAM, flush khi crash; watchdog restart nhưng **giữ core trước khi restart**.
4. Tương quan metadata các lần chết: uptime (leak?), giờ (cron/logrotate?), thao tác khách.
- Từ khóa chấm điểm: core dump + symbol khớp build ID; tư duy "hạ tầng bằng chứng" thay vì "thêm log rồi chờ".
</details>

#### BSP-024 · 🔴 · design · 🏗️ · [→ melp/debug-realtime](../../../16-book-summaries/melp/debug-realtime.md)
**Ứng dụng điều khiển thỉnh thoảng trễ chu kỳ (CPU trung bình thấp)?**
<details><summary>Đáp án (khung "tracing, không phải profiling")</summary>

- Spike hiếm + CPU trung bình thấp → **tracing** (`trace-cmd record -e sched* -e irq*`), app tự phát hiện trễ ghi **trace_marker**; mở kernelshark xem thread bị gì.
- 5 họ nghi phạm: (1) bị task khác chen (→ SCHED_FIFO/affinity); (2) IRQ/softirq storm (→ IRQ affinity, threaded IRQ hạ priority); (3) chờ lock — **priority inversion** (→ PI mutex) hoặc I/O đồng bộ trong đường RT (log ghi flash); (4) cpuidle exit latency/cpufreq tụt (→ giới hạn C-state, governor); (5) major page fault (→ mlockall).
- Chốt: cyclictest định lượng nền tảng; phân biệt profiling ("trung bình ai tốn") vs tracing ("khoảnh khắc đó chuyện gì").
</details>

#### BSP-025 · 🔴 · design · 🏗️ · [→ ostep/virtualization-memory](../../../16-book-summaries/ostep/virtualization-memory.md), [melp/debug-realtime](../../../16-book-summaries/melp/debug-realtime.md)
**Thiết bị chạy vài tuần thì chậm dần rồi bị kill — hướng điều tra?**
<details><summary>Đáp án (khung memory)</summary>

- Triệu chứng khớp **memory leak → OOM killer** (Linux overcommit: malloc không fail, chết lúc chạm trang): xác nhận bằng `dmesg` (OOM log có bảng điểm process) + đồ thị **PSS theo thời gian** từng process (`smaps_rollup`, smem).
- Khoanh thủ phạm: process nào PSS tăng tuyến tính; trong process: valgrind/ASan ở staging, so `smaps` theo vùng (heap tăng = leak malloc, số mapping tăng = leak mmap/fd — kiểm `ls /proc/pid/fd | wc`).
- Nghi phạm ngoài heap: **page cache đầy là bình thường** ("free thấp" ≠ hết RAM); kernel leak (slab — slabtop); tmpfs phình (log vào /tmp!); fragmentation.
- Phòng bị: cgroup memory limit per-service, `oom_score_adj` bảo vệ process điều khiển, PSI monitoring cảnh báo sớm.
</details>

---

## Bảo mật boot & vòng đời kernel

#### BSP-026 · 🟠 · concept · ⭐ · [→ melp/storage-update](../../../16-book-summaries/melp/storage-update.md)
**Secure boot hoạt động thế nào? Vì sao mọi package phải được ký?**
<details><summary>Đáp án</summary>

- **Chain of trust**: mỗi tầng xác minh **chữ ký** của tầng kế trước khi trao quyền chạy, bắt đầu từ Boot ROM tin cậy tuyệt đối (trong silicon). Nhà sản xuất ký bằng private key; thiết bị xác minh bằng public key nung trong **eFuse/OTP**. Sai chữ ký ở bất kỳ mắt xích nào → dừng boot → chống firmware giả mạo.
- **Ký package**: để thiết bị chỉ chạy phần mềm do nhà sản xuất phát hành — image không ký/sai chữ ký bị từ chối, ngăn cài firmware trái phép hoặc đã bị sửa đổi. Đây là phần mở rộng của chain of trust xuống tới tầng ứng dụng/OTA.
- 💡 Nếu bị hỏi sâu hơn mức biết: "Tôi hiểu ở mức khái niệm và vận hành; phần crypto/fuse cụ thể tôi chưa làm trực tiếp nhưng sẵn sàng học." — trung thực ăn điểm hơn chém.
</details>

#### BSP-027 · 🟡 · concept · ⭐ · [→ lkd/](../../../16-book-summaries/lkd/)
**Kể quá trình kernel migration (vd 5.10 → 6.12).**
<details><summary>Đáp án</summary>

API kernel thay đổi/deprecate giữa các phiên bản → driver phải sửa khớp (chữ ký hàm đổi, header dời, cơ chế cũ bị gỡ). Quy trình: đọc changelog → build bắt lỗi compile → sửa từng API → test trên target → đảm bảo `dmesg` không còn warning. *(Nên kèm 1 ví dụ API cụ thể nếu nhớ được — vd đổi `gpio_*` sang gpiod, thay đổi chữ ký `.remove` callback, header của `class_create`.)*
</details>

---
⬅️ [Bank index](README.md)
