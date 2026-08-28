# BSP — Board Support Package (chuyên sâu)

> Domain `BSP`. Khoan sâu: boot chain, device tree/probe, interrupt/DMA/MMIO, flash & OTA, Yocto, power, real-time, bring-up. Track dùng: `bsp`, `drivers-dt`, `melp`, `lkd`.
> 🏗️ = câu thiết kế/tình huống — chấm theo **khung**, không đáp án duy nhất.
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

---

## A — Boot chain & bootloader

#### BSP-040 · 🟢 · concept · ⭐ · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Kể các giai đoạn boot của một thiết bị embedded Linux, theo thứ tự.**
<details><summary>Đáp án</summary>

**ROM code** (nằm trong chip, không sửa được) → **SPL/MLO** (nhỏ, chạy trong SRAM, nhiệm vụ chính là khởi tạo DRAM) → **U-Boot** (đầy đủ, nạp kernel + DTB vào RAM) → **kernel** (khởi tạo driver, mount rootfs) → **init/PID 1** (dựng userspace). Mỗi tầng chỉ cần đủ sức nạp tầng kế tiếp. Chi tiết *vì sao* nhiều tầng: [BSP-002](bsp.md).
</details>

---

#### BSP-001 · 🟡 · concept · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
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

#### BSP-002 · 🟠 · concept · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**Kể chi tiết quá trình boot từ lúc cấp nguồn đến shell, và giải thích VÌ SAO phải nhiều giai đoạn.**
<details><summary>Đáp án</summary>

- Chuỗi: **ROM code** (trong silicon — đọc boot pins chọn nguồn, nạp giai đoạn kế vào **SRAM nội**) → **SPL** (vừa SRAM; việc chính: **init DRAM** + clock/PMIC tối thiểu, nạp U-Boot vào DRAM) → **U-Boot proper** (init storage/console, đọc env, nạp kernel Image + **DTB** + initramfs vào RAM, `booti` với con trỏ DTB trong x0) → **kernel** (parse DTB, bật MMU, probe driver, mount rootfs theo `root=`) → **init PID 1** → service → shell.
- Vì sao nhiều giai đoạn: **chuỗi bootstrap tài nguyên** — lúc cấp nguồn DRAM chưa hoạt động, chỉ có SRAM vài chục–trăm KB; U-Boot đầy đủ không vừa SRAM → cần SPL đủ nhỏ để dựng DRAM trước. Mỗi tầng chỉ đủ khả năng dựng tầng kế.
- ARM64 + secure boot: chuỗi TF-A `BL1→BL2(≈SPL)→BL31(secure monitor, PSCI, ở lại runtime)→BL33(U-Boot)`, mỗi bước **verify chữ ký** bước sau = chain of trust từ khóa trong eFuse.
- Mốc debug từng đoạn: không gì trên UART = trước SPL; banner U-Boot mà không "Starting kernel" = load/bootargs; dừng ở VFS = root=.
</details>

#### BSP-003 · 🟠 · concept · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**`bootargs` gồm những gì quan trọng? Kể 3 lỗi bootargs kinh điển và triệu chứng.**
<details><summary>Đáp án</summary>

- `bootargs` = **kernel command line** — hợp đồng bootloader→kernel: `console=ttymxc0,115200` (console), `earlycon` (console siêu sớm không cần driver), `root=/dev/mmcblk0p2` + `rootwait` (rootfs ở đâu, chờ device), `rootfstype=`, `init=`, `loglevel=`, `mem=`, `nfsroot=`/`ip=` (NFS boot).
- Ba lỗi kinh điển:
  1. **Sai `console=`** (tên device của kernel khác U-Boot: ttyS0 vs ttymxc0 vs ttyAMA0) → kernel boot **hoàn toàn im lặng** sau "Starting kernel" dù hệ có thể vẫn lên. Chẩn đoán: thêm `earlycon`.
  2. **Sai `root=`/thiếu driver FS** → panic `VFS: Unable to mount root fs` — đọc dòng "available partitions" ngay trên panic để đối chiếu.
  3. **Thiếu `rootwait`** với eMMC/SD/USB → kernel tìm root *trước khi* storage enumerate xong → panic chập chờn tùy lần boot.
</details>

#### BSP-004 · 🟡 · concept · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**FIT image là gì, hơn uImage cũ chỗ nào?**
<details><summary>Đáp án</summary>

- **FIT (Flattened Image Tree)**: một file `.itb` đóng gói **kernel + nhiều DTB + initramfs + configuration**, mô tả bằng nguồn `.its`, build bằng `mkimage`.
- Hơn uImage (chỉ bọc **một image** + header CRC): (1) **nhiều thành phần + nhiều cấu hình** — một ảnh phục vụ nhiều biến thể board; (2) **hash/chữ ký từng thành phần** (SHA + RSA) — nền của **verified boot** trong U-Boot; (3) metadata rõ ràng (load address, entry, compression).
- Thực dụng: sản phẩm cần secure boot hoặc nhiều SKU chung firmware → FIT gần như bắt buộc.
</details>

#### BSP-005 · 🔴 · design · 🏗️ · [→ melp/storage-update](../../../15-book-summaries/melp/storage-update.md)
**Bootloader có được OTA update không? Rủi ro và cách làm đúng.**
<details><summary>Đáp án</summary>

- Nguyên tắc: **hạn chế tối đa** — bootloader là mắt xích không có ai đứng sau: ghi dở SPL/U-Boot = ROM code không còn gì hợp lệ để nạp = brick sâu (chỉ cứu bằng UART/USB boot mode ở xưởng).
- Nếu buộc phải update, dùng cơ chế **phần cứng có dự phòng**: eMMC có **2 boot partition** — ghi bản mới vào partition không active, verify, rồi đổi `mmc bootpart`; một số SoC hỗ trợ nhiều bản ảnh + fallback trong ROM. Không có → cân nhắc *không* update bootloader ngoài hiện trường.
- Thiết kế phòng ngừa: bootloader **tối giản và đóng băng** — mọi logic hay đổi (chọn bank A/B, health check) đẩy lên kernel/userspace hoặc thành script/env thay được không đụng binary.
</details>

#### BSP-029 · 🟡 · concept · 📦 2026-08-13 · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**`initramfs` để làm gì? Thiết bị nhúng có luôn cần nó không?**
<details><summary>Đáp án</summary>

**`initramfs` là một filesystem tối giản nằm trong RAM**, được bootloader nạp cùng kernel.

> **Ai nạp nó — chính xác là giai đoạn nào** *(bổ sung 2026-08-19)*: **U-Boot proper**, không phải SPL/MLO. SPL/MLO là giai đoạn 1 cực nhỏ (chạy trong SRAM on-chip, vài chục KB), nhiệm vụ **duy nhất** là init DRAM rồi nạp U-Boot proper vào đó. Nó không biết gì về kernel/initramfs. U-Boot proper mới đọc kernel + DTB + initramfs vào RAM, đặt địa chỉ vào `bootm`/`booti` rồi trao quyền. Kernel mount nó làm root **tạm thời**, chạy một script khởi động, rồi **chuyển sang** rootfs thật.

**Vấn đề nó giải quyết — bài toán con gà quả trứng:** để mount được rootfs, kernel cần **driver** cho thiết bị lưu trữ (và có khi cả LVM, RAID, mã hoá, mạng). Nhưng nếu driver đó nằm dạng **module trong chính rootfs** thì kernel không đọc được nó ⇒ kẹt. initramfs mang sẵn các module đó trong RAM để phá vòng lặp.

**Vì sao bản phân phối desktop luôn dùng:** một kernel phải chạy trên **mọi** máy ⇒ không thể biên dịch thẳng mọi driver lưu trữ vào kernel (quá lớn) ⇒ đóng chúng thành module và dùng initramfs nạp đúng cái cần.

**⭐ Thiết bị nhúng thì thường KHÔNG cần:** bạn **biết trước** phần cứng ⇒ **biên dịch thẳng driver lưu trữ vào kernel** rồi bỏ initramfs. Lợi ích thật:
- **Boot nhanh hơn** — bớt một giai đoạn nạp và chuyển đổi.
- **Ít thứ hỏng hơn** — mất một thành phần trong chuỗi boot.
- Ảnh hệ thống đơn giản hơn, dễ kiểm chứng.

**Khi nhúng vẫn nên dùng:** cần logic phức tạp trước khi mount (chọn A/B partition, kiểm chữ ký, mở khoá mã hoá, phục hồi khi rootfs hỏng) — lúc đó initramfs là **nơi đặt chính sách khởi động**, và giá trị của nó không còn là "nạp driver" nữa.

**Bẫy:** (1) quên cập nhật initramfs sau khi đổi driver ⇒ kernel mới chạy với module cũ, lỗi rất khó hiểu; (2) nhét quá nhiều vào initramfs ⇒ tốn RAM vĩnh viễn nếu không giải phóng đúng cách; (3) tưởng nó là bắt buộc — nhiều BSP nhúng chạy hoàn toàn không có nó.

**Chốt:** *"initramfs phá thế con-gà-quả-trứng: cần driver để mount rootfs mà driver lại nằm trong rootfs. Thiết bị nhúng biết trước phần cứng nên thường biên dịch thẳng driver vào kernel và bỏ hẳn nó — boot nhanh hơn, ít thứ hỏng hơn."*
</details>

#### BSP-030 · 🟡 · concept · 📦 2026-08-13 · [→ boot-process](../../../08-embedded-systems/boot-process.md)
**Tiến trình `init` (PID 1) là gì và vai trò của nó? Điều gì xảy ra nếu nó chết?**
<details><summary>Đáp án</summary>

**PID 1 là tiến trình đầu tiên kernel tạo ra ở user space**, và là **tổ tiên của mọi tiến trình khác**. Kernel nạp nó rồi rút lui khỏi việc quản lý user space.

**Ba vai trò:**
1. **Khởi động hệ thống** — mount filesystem, cấu hình mạng, chạy các service theo đúng thứ tự phụ thuộc.
2. ⭐ **Nhận nuôi tiến trình mồ côi.** Cha chết trước con ⇒ con được **PID 1 nhận nuôi**, và PID 1 phải `wait()` để thu hồi ⇒ **ngăn zombie tích tụ** toàn hệ thống ([OS-009](os.md)).
3. **Giám sát và khởi động lại** service chết (với init hiện đại như systemd).

**⚠️ PID 1 chết ⇒ KERNEL PANIC.** Kernel coi đó là lỗi không thể phục hồi — vì không còn ai nhận nuôi tiến trình mồ côi và không còn ai quản lý user space. Đây là lý do PID 1 phải cực kỳ đơn giản và ổn định.

**Trên thiết bị nhúng — ba lựa chọn:**

| | Ưu | Nhược |
|---|---|---|
| **BusyBox init** | Rất nhỏ, đơn giản, dễ hiểu toàn bộ | Ít tính năng: không phụ thuộc thứ tự, giám sát yếu |
| **systemd** | Phụ thuộc theo thứ tự, khởi động song song (**boot nhanh hơn**), giám sát + watchdog, log tập trung | Lớn, phức tạp, kéo theo nhiều thứ |
| **Script riêng** | Kiểm soát tuyệt đối | Phải tự viết lại mọi thứ, dễ thiếu ca biên |

⚠️ **Bẫy đặc thù nhúng:** nếu bạn chạy **ứng dụng của mình làm PID 1** (một số thiết bị làm vậy cho nhanh) thì phải tự gánh cả **vai trò 2** — không thu hồi tiến trình con là zombie tích tụ tới cạn PID; và ứng dụng crash là **panic cả máy**, không có ai khởi động lại nó.

**Chốt:** *"PID 1 khởi động hệ thống, nhận nuôi tiến trình mồ côi và giám sát service. Nó chết là kernel panic — nên nếu đặt ứng dụng của mình làm PID 1 thì phải tự thu hồi zombie và tự lo chuyện không được phép crash."*
</details>

---
⬅️ [Bank index](README.md)

## B — Kernel & Device Tree

#### BSP-006 · 🟠 · concept · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**Trình bày đường đi từ một node device tree đến hàm `probe()` của driver. EPROBE_DEFER là gì?**
<details><summary>Đáp án</summary>

- Bootloader trao DTB → kernel parse, dựng **platform_device** cho các node. Driver đăng ký `platform_driver` với **`of_match_table`** (danh sách `compatible`). Kernel match node ↔ driver theo compatible (so từ chuỗi cụ thể nhất) → gọi **`probe(pdev)`**: đọc `reg` → `devm_ioremap_resource`, `interrupts` → `devm_request_irq`, `clocks/…-supply` → clk/regulator API, property riêng → `device_property_read_*`, rồi đăng ký subsystem.
- **EPROBE_DEFER**: probe cần tài nguyên mà driver cung cấp nó **chưa probe** (clock, regulator, GPIO expander) → return `-EPROBE_DEFER` → kernel xếp lại, **probe lại sau mỗi lần có driver mới xong** — thứ tự tự hội tụ, không hard-code. Thiết bị "mất tích" không lỗi → xem `/sys/kernel/debug/devices_deferred`.
- Bẫy hay bị bắt bẻ: DT **không nạp driver** — driver phải có sẵn (built-in, hoặc module udev nạp theo modalias/uevent, cần `MODULE_DEVICE_TABLE(of, …)`); DT chỉ là dữ liệu khớp nối.
</details>

#### BSP-007 · 🟠 · design · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**Thêm một thiết bị I2C mới vào board đang chạy: các bước và các lỗi thường gặp.**
<details><summary>Đáp án</summary>

- Các bước: (1) tìm driver theo compatible trong kernel, bật CONFIG; (2) sửa DT: trong `&i2cN` (đủ `status="okay"` + pinctrl), thêm node con `compatible` + `reg=<addr>` (+ interrupt/supply nếu dùng), đúng **bindings YAML** (`make dtbs_check`); (3) build **chỉ dtbs**, deploy DTB (hoặc overlay `.dtbo`); (4) kiểm chứng theo tầng: `i2cdetect` → dmesg probe → node trong `/sys/bus/i2c/devices/` → đọc giá trị.
- Lỗi thường gặp: sai địa chỉ I2C (strap pin đổi addr); **quên pinmux** SDA/SCL (bus timeout); thiếu pull-up phần cứng; driver để `m` mà module không có trên rootfs; `vcc-supply` trỏ regulator disabled → **EPROBE_DEFER treo vĩnh viễn**; compatible gõ sai (match là so chuỗi tuyệt đối); i2cdetect thấy mà driver không bind → gần như chắc chắn lỗi compatible/CONFIG.
</details>

#### BSP-008 · 🟡 · concept · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**Driver nên built-in (`y`) hay module (`m`)? Tiêu chí và ví dụ nghịch lý gà-trứng.**
<details><summary>Đáp án</summary>

- Tiêu chí: cần **trước khi mount rootfs** (storage controller, FS của root, console) → `y` hoặc `m`-trong-initramfs; phần cứng tùy chọn/nhiều SKU → `m` (nạp theo modalias); hệ ít RAM → nghiêng `y` toàn bộ, có thể tắt hạ tầng module.
- Nghịch lý gà-trứng: **driver eMMC là module nằm trên rootfs trong chính eMMC** → không bao giờ nạp được. Giải: built-in, hoặc bỏ module vào initramfs.
- Cộng điểm: console driver phải built-in; `CONFIG_MODULES=n` là lựa chọn sản phẩm hợp lệ.
</details>

#### BSP-009 · 🟠 · concept · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**Pinctrl trong device tree đóng vai trò gì? Triệu chứng khi thiếu/sai pinmux?**
<details><summary>Đáp án</summary>

- Mỗi chân SoC đa năng (một pad → GPIO/UART/I2C/PWM… chọn bằng mux register). **pinctrl** mô tả cấu hình pad trong DT: nhóm `pinctrl_uart1 { fsl,pins = <...> }`, node thiết bị tham chiếu qua `pinctrl-names="default"` + `pinctrl-0=<&pinctrl_uart1>` — driver core tự áp khi probe.
- Triệu chứng thiếu/sai: **driver probe thành công, phần mềm chạy "bình thường", nhưng tín hiệu không ra chân** — nhóm lỗi khó chịu nhất vì mọi log sạch. UART câm, I2C timeout, LED không sáng. Chẩn đoán: scope/logic analyzer + đọc thanh ghi mux (`devmem`) đối chiếu datasheet; nhớ **U-Boot chạy được không có nghĩa kernel đúng** — kernel áp lại pinmux theo DT của nó.
- Bộ ba lỗi DT thực chiến: quên `status="okay"`, pinmux sai, EPROBE_DEFER vòng.
</details>

#### BSP-027 · 🟡 · concept · ⭐ · [→ lkd/](../../../15-book-summaries/lkd/)
**Kể quá trình kernel migration (vd 5.10 → 6.12).**
<details><summary>Đáp án</summary>

**Bốn nhóm thứ vỡ, xếp theo *mức độ khó phát hiện* — nhóm sau nguy hiểm hơn nhóm trước:**

| Nhóm | Ví dụ | Vì sao dễ/khó |
|---|---|---|
| **① Đổi chữ ký hàm** | `probe()`, `class_create()`, `device_create()`, `remove()` trả `void` | ✅ **Dễ nhất** — build lỗi, compiler chỉ thẳng chỗ |
| **② Hàm bị xoá** | `set_fs()`/`get_fs()` (gỡ ~5.18), `ioremap_nocache()`, `pci_set_dma_mask()` | ✅ Build lỗi, nhưng phải **tìm cơ chế thay thế**, không chỉ đổi tên |
| **③ Siết cảnh báo** | `-Werror` bắt nhiều thứ trước chỉ warning; cấm biến unused; bắt buộc prototype | 🟡 Build lỗi hàng loạt, phần lớn sửa máy móc |
| ⭐ **④ Đổi HÀNH VI, giữ nguyên API** | Mặc định scheduler/PM đổi · thứ tự probe đổi · timer resolution đổi · DT binding siết chặt hơn | 🔴 **Nguy hiểm nhất — BUILD SẠCH, chạy mới sai.** Không có công cụ nào bắt được |

**⭐ Nhóm ④ mới là phần đáng nói ở phỏng vấn** — nó là lý do migration mất hàng tuần chứ không phải hàng giờ. Build xanh **không** nghĩa là xong; phải có **bộ test chạy trên phần cứng thật** so sánh hành vi trước/sau.

**Giữ tương thích ngược — thực tế sản phẩm luôn phải chạy song song hai đời kernel:**
```c
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
    ret = kernel_read(f, buf, len, &pos);        // cach moi
#else
    mm_segment_t old = get_fs(); set_fs(KERNEL_DS);   // cach cu
    ret = vfs_read(f, buf, len, &pos);
    set_fs(old);
#endif
```
⚠️ Nhưng `#if` rải khắp nơi là **nợ kỹ thuật**: gom vào **một lớp compat riêng**, đừng để lan vào logic nghiệp vụ.

**Quy trình kể ở phỏng vấn — có thứ tự, không phải danh sách:**
① build ⇒ vá nhóm ①②③ → ② **đọc changelog/`Documentation/` của các subsystem mình chạm** ⇒ tìm nhóm ④ → ③ boot, so `dmesg` **trước/sau** → ④ chạy bộ test chức năng trên **phần cứng thật** → ⑤ đo lại **hiệu năng/latency** *(đổi scheduler/PM có thể làm chậm mà không lỗi)*.

**⚠️ Bẫy:** (1) 🔴 **tưởng build xanh là xong** — nhóm ④ không lộ ra lúc build; (2) sửa hết `#if` rải rác rồi không ai gỡ được khi bỏ kernel cũ; (3) quên rằng **DT binding cũng siết theo** — DTS cũ có thể bị từ chối; (4) nâng kernel **và** đổi thứ khác cùng lúc ⇒ hỏng không biết do cái nào.

**Chốt:** *"Ba nhóm đầu compiler bắt hộ, sửa là xong. Nhóm thứ tư — **API giữ nguyên nhưng hành vi đổi** — mới là phần tốn thời gian, vì build sạch mà chạy sai. Nên nghiệm thu bằng test trên phần cứng thật, không bằng build xanh."*
</details>

## C — Interrupt, DMA & MMIO

#### BSP-041 · 🟢 · concept · ⭐ · [→ architecture](../../../08-embedded-systems/architecture.md)
**MMIO (memory-mapped I/O) là gì?**
<details><summary>Đáp án</summary>

Thanh ghi của ngoại vi được **ánh xạ vào không gian địa chỉ**, nên đọc/ghi thanh ghi dùng chính lệnh load/store như bộ nhớ thường — khác *port I/O* (x86 có lệnh `in`/`out` riêng). Hệ quả: phải khai **`volatile`** để compiler không tối ưu mất lần truy cập, và cần **memory barrier** để giữ thứ tự. Trong kernel dùng [`ioremap` + `readl/writel`](bsp.md), không dùng địa chỉ vật lý trần.
</details>

---

#### BSP-042 · 🟢 · concept · ⭐ · [→ architecture](../../../08-embedded-systems/architecture.md)
**DMA là gì, giải quyết vấn đề gì?**
<details><summary>Đáp án</summary>

**Direct Memory Access**: một bộ điều khiển riêng chuyển dữ liệu giữa ngoại vi và RAM **không cần CPU copy từng byte**. CPU chỉ cấu hình (nguồn, đích, độ dài) rồi làm việc khác, và nhận **ngắt báo xong**. Lợi: giải phóng CPU + băng thông cao hơn. Cái giá: **cache có thể lệch với RAM** ⇒ cần cache maintenance ([BSP-011](bsp.md)).
</details>

---

#### BSP-010 · 🟠 · concept · ⭐ · [→ melp/drivers-init-power](../../../15-book-summaries/melp/drivers-init-power.md), [ostep/concurrency](../../../15-book-summaries/ostep/concurrency.md)
**Vì sao interrupt handler không được ngủ? Threaded IRQ / top-bottom half giải quyết gì?**
<details><summary>Đáp án</summary>

- Handler chạy trong **interrupt context** — không có task_struct, không schedule được; "ngủ" = gọi scheduler nhường CPU chờ wake — không có chỗ trong hàng đợi scheduler → `scheduling while atomic`/treo. Kèm: dòng IRQ đang mask, ngủ lâu là mất ngắt/deadlock. Hệ quả: cấm mutex, `kmalloc(GFP_KERNEL)`, `copy_*_user` — chỉ spinlock, `GFP_ATOMIC`, việc ngắn.
- **Top/bottom half**: top ack thiết bị + gom tối thiểu + lên lịch phần còn lại; bottom chạy sau với ngắt mở — softirq/tasklet (vẫn atomic) hoặc **workqueue** (process context, ngủ được).
- **Threaded IRQ** (`request_threaded_irq` + `IRQF_ONESHOT`): phần chính chạy trong **kernel thread riêng** — ngủ được, mutex được, **priority chỉnh được** (nền của PREEMPT_RT).
</details>

#### BSP-011 · 🟠 · concept · ⭐ · [→ ostep/virtualization-memory](../../../15-book-summaries/ostep/virtualization-memory.md)
**Phân biệt coherent DMA và streaming DMA. Vì sao cần cache maintenance khi DMA?**
<details><summary>Đáp án</summary>

- **Vấn đề gốc:** DMA engine ghi/đọc **RAM trực tiếp, không qua cache CPU** (nhiều SoC ARM không có hardware coherency cho DMA) → hai bản sự thật: CPU nhìn cache, device nhìn RAM. Device DMA vào RAM mà CPU đọc trúng **cache line cũ** → rác; CPU ghi buffer (còn trong cache) mà bảo device đọc → device đọc đồ cũ.
- **Coherent DMA** — `dma_alloc_coherent()`: cấp vùng **uncached** — CPU và device luôn thấy cùng dữ liệu, không cần bảo trì; đổi lại truy cập CPU chậm. Dùng cho: descriptor ring, control block.
- **Streaming DMA** — `dma_map_single/sg()` trước khi giao device, `dma_unmap_*` sau: buffer cached, kernel **flush (to-device) / invalidate (from-device)** đúng lúc map/unmap theo `DMA_TO_DEVICE`/`FROM_DEVICE`. Dùng cho: payload lớn một chiều (packet, frame).
- Quy tắc: **giữa map và unmap CPU không đụng buffer**; địa chỉ giao device là **dma_addr_t** (bus address); buffer từ kmalloc/DMA-safe (không phải stack!).
</details>

#### BSP-012 · 🟠 · concept · [→ melp/drivers-init-power](../../../15-book-summaries/melp/drivers-init-power.md)
**`ioremap` là gì? Vì sao không dùng thẳng địa chỉ vật lý của thanh ghi? `/dev/mem` đứng đâu?**
<details><summary>Đáp án</summary>

- Kernel chạy với **MMU bật** — mọi truy cập là virtual address; địa chỉ vật lý thanh ghi (từ `reg` trong DT) chưa map thì deref = fault. **`ioremap()`** (driver dùng `devm_ioremap_resource`) tạo mapping virtual→physical cho vùng MMIO với thuộc tính đúng: **uncached, non-bufferable** (Device memory trên ARM) — thanh ghi bị cache/gộp/prefetch là sai (đọc status 2 lần phải là 2 lần đọc thật — cùng lý do `volatile`).
- Đọc/ghi qua `readl/writel` (kèm memory barrier) thay vì deref con trỏ trần — API chuẩn xử lý ordering + endianness.
- **`/dev/mem`**: cửa userspace mmap thẳng physical — **vũ khí bring-up** (devmem kiểm tra clock/pinmux trước khi đổ lỗi driver) nhưng **cấm trong sản phẩm** (vượt mặt driver, lỗ hổng bảo mật toàn hệ; chặn bằng `CONFIG_STRICT_DEVMEM`).
</details>

#### BSP-013 · 🔴 · design · 🏗️ · [→ melp/drivers-init-power](../../../15-book-summaries/melp/drivers-init-power.md)
**Sensor báo dữ liệu qua ngắt 5kHz. Thiết kế đường dữ liệu từ ISR đến ứng dụng userspace.**
<details><summary>Đáp án (khung)</summary>

- 5kHz = 200µs/sự kiện — **không** làm việc nặng trong ISR, càng không round-trip userspace mỗi sự kiện.
- Khung: **ISR tối giản** (ack + đọc/kick DMA) → dữ liệu vào **ring buffer trong kernel** (hoặc DMA thẳng vào buffer vòng — tốt nhất: sensor→DMA→RAM không CPU) → báo userspace **theo lô**: wake `poll/read` khi đủ N mẫu hoặc timeout (giảm wakeup từ 5k/s xuống vài chục/s) → userspace `read()`/`mmap` lấy cả block.
- Chuẩn hóa: dùng subsystem **IIO** (industrial I/O) có sẵn mô hình trigger + kfifo + watermark; đừng tự phát minh char device.
- Con số phải nêu: tần suất wakeup, độ trễ chấp nhận (đổi với batch size), chiến lược overflow (drop oldest + counter đo được).
- Nếu latency từng mẫu là yêu cầu cứng: xử lý trong threaded IRQ priority cao / đẩy xuống co-processor — đừng kéo lên userspace.
</details>

## D — Storage flash & OTA

#### BSP-043 · 🟡 · concept · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**Flash (NAND/eMMC) khác RAM và đĩa từ ở chỗ nào? Ba hệ quả cho phần mềm là gì?**
<details><summary>Đáp án</summary>

**Cơ chế — điểm khác gốc rễ:** ghi vào flash chỉ **chuyển bit 1 → 0**. Muốn đưa bit về 1 phải **erase**, mà erase làm theo **cả block** — lớn hơn page rất nhiều.

| | Đơn vị đọc | Đơn vị ghi | Đơn vị **xoá** |
|---|---|---|---|
| RAM | byte | byte | — |
| Đĩa từ | sector | sector (ghi đè tại chỗ) | — |
| **NAND flash** | page (~2–4 KB) | page | 🔴 **block (~128–256 KB)** |

**Ba hệ quả:**
1. **Không ghi đè tại chỗ được** ⇒ phải có lớp dịch: **FTL** (trong eMMC/SD, phần cứng lo) hoặc **filesystem hiểu flash** (UBIFS/JFFS2 trên NAND thô).
2. **Số lần erase mỗi block là hữu hạn** (P/E cycle) ⇒ cần **wear leveling** để không mòn dồn một chỗ ([BSP-016](bsp.md)).
3. **NAND sinh bit lỗi và có bad block** ⇒ cần **ECC** + quản lý bad block.

⚠️ **Bẫy:** thấy eMMC "dùng như đĩa" rồi tưởng không còn ràng buộc flash — FTL chỉ **giấu** chúng đi, mòn và write amplification vẫn còn nguyên.
</details>

---

#### BSP-044 · 🟢 · concept · ⭐ · [→ secure-boot](../../../08-embedded-systems/secure-boot.md)
**OTA update là gì, gồm những bước nào?**
<details><summary>Đáp án</summary>

**Over-The-Air**: cập nhật firmware/phần mềm từ xa, không cần cầm thiết bị. Các bước: máy chủ phát hành bản **đã ký** → thiết bị tải về → **xác minh chữ ký** → ghi vào nơi lưu trữ → khởi động sang bản mới → xác nhận thành công. Yêu cầu sống còn: **mất điện giữa chừng không được brick** ⇒ A/B partition hoặc recovery ([BSP-015](bsp.md)).
</details>

---

#### BSP-014 · 🟠 · concept · ⭐ · [→ melp/storage-update](../../../15-book-summaries/melp/storage-update.md), [ostep/persistence](../../../15-book-summaries/ostep/persistence.md)
**NAND thô + UBIFS vs eMMC + ext4/f2fs — trade-off và stack phần mềm? Vì sao không trộn chéo?**
<details><summary>Đáp án</summary>

- Câu định vị: **FTL nằm ở đâu?** eMMC: FTL trong chip (lo wear/bad block/ECC) → host thấy block device → ext4/f2fs, tool chuẩn — mặc định hiện đại. NAND thô: host lo tất → stack `NAND controller (ECC) → MTD → UBI (wear leveling, bad block, logical erase block) → UBIFS` — rẻ/kiểm soát trọn, trả giá độ phức tạp.
- Không trộn chéo: **ext4 lên NAND thô** — ext4 giả định ghi-đè-tại-chỗ, không biết erase block/bad block/bit flip → hỏng từ nguyên lý. **UBIFS lên eMMC** — UBIFS cần thấy flash thô qua MTD; eMMC giấu sau FTL → hai tầng wear leveling chồng nhau phản tác dụng.
- f2fs = "block FS hiểu ý FTL" (log-structured) — điểm giữa tốt cho eMMC.
- Chốt senior: dù chọn gì — **rootfs read-only + partition data riêng + test rút điện tự động**.
</details>

#### BSP-015 · 🔴 · design · 🏗️ · ⭐ · [→ melp/storage-update](../../../15-book-summaries/melp/storage-update.md)
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

#### BSP-016 · 🟠 · design · [→ ostep/persistence](../../../15-book-summaries/ostep/persistence.md), [melp/storage-update](../../../15-book-summaries/melp/storage-update.md)
**Vì sao ghi nhỏ rải rác có hại cho eMMC/SD? Thiết kế logging trên thiết bị flash thế nào?**
<details><summary>Đáp án</summary>

- Flash erase theo block lớn, ghi theo page vào chỗ đã erase → ghi nhỏ rải rác làm block lốm đốm → GC của FTL **chép page sống trước khi erase** → **write amplification** → chậm + **mòn** (P/E cycles hữu hạn).
- Thiết kế logging: (1) **gom lô** — buffer RAM, flush theo chu kỳ/ngưỡng, không fsync từng dòng; (2) **ring buffer kích thước cố định** (logrotate theo size); (3) log "nóng" để tmpfs, chỉ đổ flash khi có sự cố (flush-on-crash); (4) **budget TBW**: ước byte/ngày × WA × tuổi thọ so datasheet; (5) TRIM/discard định kỳ; (6) linh kiện: eMMC công nghiệp/SLC-mode cho vùng ghi nóng.
- Bug thật: thiết bị chết hàng loạt sau 2 năm vì log 1 dòng/giây fsync liên tục lên SD rẻ.
</details>

## E — Build system & Yocto

#### BSP-017 · 🟠 · concept · ⭐ · [→ melp/build-systems](../../../15-book-summaries/melp/build-systems.md)
**Buildroot vs Yocto — chọn thế nào? Một BSP layer trong Yocto gồm những gì?**
<details><summary>Đáp án</summary>

**⭐ Đừng trả lời "Yocto mạnh hơn". Cả hai đều build được Linux — chúng bán hai thứ khác nhau.**

| | **Buildroot** | **Yocto** |
|---|---|---|
| Sinh ra cái gì | **Một image** | **Một định nghĩa tái lập được** của image |
| Cơ chế | Makefile + Kconfig | Metadata (recipe/layer) + BitBake |
| Đọc hết mất bao lâu | Một buổi | Nhiều tuần |
| Nhiều biến thể (SKU) | Copy config rồi phân kỳ dần | **Layer + override** — chia sẻ 90% chung |
| Build lại sau khi đổi 1 dòng | Thường build lại nhiều | **sstate** — chỉ task có hash đổi |
| Package manager trên thiết bị | ❌ **không** — vá 1 lib phải flash cả image | ✅ có (rpm/ipk/deb) |
| SDK cho đội app | thô sơ | ✅ **SDK/eSDK** — đội app build mà không cần biết Yocto |
| License / CVE audit | tay | ✅ `LIC_FILES_CHKSUM` cưỡng chế + `cve-check` |

**"Vì sao" hai tầng:**
- *Tầng nông*: *"Yocto linh hoạt hơn, Buildroot đơn giản hơn."* Đúng, nhưng không giúp ai quyết định.
- ⭐ *Tầng sâu*: **tiêu chí không phải mức độ "nghiêm túc" của dự án, mà là SỐ BIẾN THỂ × VÒNG ĐỜI.** Một sản phẩm sống 2 năm → Buildroot thắng vì bạn tiêu ít thời gian hơn để tới đích. Năm SKU sống 8 năm → Yocto thắng vì chi phí học trả một lần, còn chi phí phân kỳ nhánh trả **mãi mãi**.

**Khi nào chọn Buildroot — nửa câu trả lời hay bị bỏ:**

| Tình huống | Chọn |
|---|---|
| Một sản phẩm, một biến thể, đội 1–3 người | **Buildroot** |
| Prototype cần chạy tuần này | **Image dựng sẵn**, không build gì |
| Cần cập nhật gói lẻ ngoài hiện trường | Yocto (Buildroot không có package manager) |
| Nhiều SKU · vòng đời dài · cần SDK · cần audit | ⭐ **Yocto** |

📌 **Thực tế quyết định hộ bạn:** vendor SoC (NXP/TI/ST/Qualcomm) phát hành BSP **dạng Yocto layer**. Chọn Buildroot nghĩa là **tự port BSP của vendor sang** — chi phí thường lớn hơn chi phí học Yocto.

**BSP layer (`meta-<board>`) gồm gì:**
```
meta-mybsp/
├── conf/machine/myboard.conf        # KERNEL_DEVICETREE, PREFERRED_PROVIDER_virtual/kernel,
│                                    # UBOOT_MACHINE, SERIAL_CONSOLES, MACHINE_FEATURES
├── recipes-kernel/linux/linux-ti_%.bbappend    # patch DT/driver + config fragment .cfg
├── recipes-bsp/u-boot/u-boot-ti_%.bbappend
└── recipes-bsp/firmware/...                    # blob wifi/bluetooth
```

**Ranh giới trách nhiệm** — cũng là câu trả lời cho *"anh đặt thay đổi của mình ở đâu?"*:

| Thứ | Layer nào |
|---|---|
| DTB, kernel config cho **board** | **BSP layer** (vendor giữ) |
| Init system, libc, feature toàn hệ | **Distro layer** |
| Ứng dụng + **mọi tuỳ biến của bạn** | **Layer sản phẩm của bạn** |

**⚠️ Bẫy:** (1) trả lời *"Yocto tốt hơn"* mà không nêu được ca nào chọn Buildroot ⇒ lộ ra chưa từng phải quyết định thật; (2) sửa thẳng poky/vendor layer ⇒ mất sạch khi vendor phát hành BSP mới; (3) sửa `tmp/work` ⇒ bị nghiền, và sstate không nhận ra; (4) tưởng Buildroot "không dùng được cho sản phẩm thật" — nhiều sản phẩm bán hàng triệu máy chạy Buildroot.

**Chốt:** *"Buildroot cho tôi một image, Yocto cho tôi một định nghĩa tái lập được của image đó. Tiêu chí chọn là **số biến thể × vòng đời sản phẩm**, không phải mức độ nghiêm túc — và trên thực tế vendor phát hành BSP dạng Yocto layer nên với sản phẩm nhiều SKU thì Yocto gần như mặc định."*
</details>

#### BSP-018 · 🟡 · concept · [→ yocto §5](../../../06-build-systems/yocto.md) · [→ melp/build-systems](../../../15-book-summaries/melp/build-systems.md)
**DEPENDS vs RDEPENDS? Vì sao build xong chạy thiếu lib dù compile không lỗi?**
<details><summary>Đáp án</summary>

- **DEPENDS** = build-time (headers/lib trong sysroot lúc compile, tool native); **RDEPENDS** = runtime (thứ phải **có mặt trong image**).
- Compile OK mà chạy thiếu: lib có trong sysroot lúc link nhưng không vào image. Với `.so` link trực tiếp Yocto tự bắt (shlibs scan) — ca lọt lưới điển hình là **`dlopen()`** (nạp runtime, scanner không thấy), plugin, gọi binary ngoài qua `system()`, hoặc package tách (`-libs`,`-dev`).
- Fix: `RDEPENDS:${PN} += "libfoo"` trong recipe — không nhét thẳng vào image; soi bằng `oe-pkgdata-util`.
</details>

#### BSP-019 · 🟠 · concept · [→ melp/toolchain-rootfs](../../../15-book-summaries/melp/toolchain-rootfs.md)
**Cross-compile: binary chạy trên board báo `not found` dù file có mặt — chẩn đoán?**
<details><summary>Đáp án</summary>

**Thông báo nói dối — thứ "không tìm thấy" KHÔNG phải binary của bạn.**

```
$ ls -l /usr/bin/myapp
-rwxr-xr-x 1 root root 21384 myapp        <- FILE CO THAT
$ ./myapp
-sh: ./myapp: No such file or directory   <- ???
```

**Cơ chế:** binary link động có trường **`PT_INTERP`** trong header ELF, trỏ tới **dynamic loader** (vd `/lib/ld-linux-armhf.so.3`). Kernel đọc trường đó **trước khi** chạy binary. Loader không tồn tại trên rootfs ⇒ `execve()` trả **`ENOENT`** ⇒ shell in *"No such file or directory"* — **về file loader, không phải về binary của bạn**.

```bash
readelf -l myapp | grep -A1 INTERP    # target doi loader NAO
#   [Requesting program interpreter: /lib/ld-linux-armhf.so.3]
ls -l /lib/ld-linux-armhf.so.3        # tren TARGET co that khong?
file myapp                            # kien truc + dong hay tinh
ldd myapp                             # (chay tren target) lib nao thieu
```

**Ba nguyên nhân, theo thứ tự hay gặp:**

| Nguyên nhân | Dấu hiệu | Sửa |
|---|---|---|
| ⭐ **Lệch ABI float** — build hard-float, rootfs soft-float (hoặc ngược) | Đòi `ld-linux-armhf.so.3` mà target chỉ có `ld-linux.so.3` | Đúng tuple: `arm-linux-gnueabi**hf**-` vs `arm-linux-gnueabi-` |
| **Lệch libc** — build glibc, rootfs **musl** | Loader tên khác hẳn (`ld-musl-armhf.so.1`) | Toolchain khớp rootfs |
| **Sai kiến trúc** | `file` báo `x86-64` / `aarch64` khi target là `arm` | Sai toolchain hoàn toàn |

📌 **Cách phân biệt nhanh:** build lại với **`-static`**. Chạy được ⇒ **chắc chắn** là vấn đề loader/lib động, không phải kiến trúc. Đây là phép thử một bước, làm trước mọi thứ khác.

**⚠️ Bẫy:** (1) đi tìm bug trong code ứng dụng — sai hướng hoàn toàn, lỗi nằm ở **ABI/rootfs**; (2) `ldd` chạy trên **host** cho kết quả vô nghĩa, phải chạy trên **target**; (3) copy đại lib từ host sang target để "vá" ⇒ lệch version, crash ở chỗ khác; (4) tưởng cứ `apt install` lib trên host là đủ — cross-compile cần bản **cho target**, trong **sysroot**.

**Chốt:** *"`not found` ở đây gần như luôn là **loader** không tìm thấy chứ không phải binary. `readelf -l` xem nó đòi loader nào, kiểm trên rootfs xem có không — thường là lệch hard-float/soft-float hoặc glibc/musl. Build `-static` để xác nhận trong một bước."*
</details>

## F — Power management

#### BSP-020 · 🟠 · concept · [→ melp/drivers-init-power](../../../15-book-summaries/melp/drivers-init-power.md)
**Bốn tầng power management của Linux? Suspend-to-RAM hoạt động thế nào, wakeup source là gì?**
<details><summary>Đáp án</summary>

- Bốn tầng: **cpufreq** (DVFS khi chạy — governor, OPP table trong DT) → **cpuidle** (rảnh ngủ C-state — sâu hơn tiết kiệm hơn nhưng exit latency cao) → **runtime PM** (từng thiết bị tự tắt khi không dùng — usage count, `runtime_suspend/resume`) → **system suspend** (cả hệ ngủ).
- **Suspend-to-RAM** (`echo mem > /sys/power/state`): freeze userspace → suspend device **theo thứ tự ngược cây thiết bị** (mỗi driver cần `.suspend/.resume`) → tắt CPU phụ, CPU cuối vào ngủ sâu; **RAM ở self-refresh** (giữ nội dung, tốn µA). Resume đi ngược.
- **Wakeup source**: khai `wakeup-source` trong DT/driver `device_init_wakeup`: GPIO nút nhấn, RTC alarm, WoL. Debug: `/sys/kernel/debug/wakeup_sources`, delta `/proc/interrupts`.
- Bug PM kinh điển: resume treo vì một driver thiếu resume handler / sai thứ tự clock; công cụ `pm_test` chia đôi tầng.

**⭐ Vì sao suspend đi NGƯỢC cây thiết bị** *(bổ sung 2026-08-19 — trước đây chỉ nêu tên, không nêu lý do)*: device model của Linux là một **cây phụ thuộc** — thiết bị con sống nhờ tài nguyên của cha (bus, clock, regulator, power domain). Ví dụ: cảm biến I2C ← I2C controller ← clock + regulator của SoC.
- **Suspend đi từ LÁ vào GỐC**: phải cho cảm biến ngủ **trước**, vì tắt I2C controller trước thì cảm biến mất đường giao tiếp, driver của nó không kịp lưu trạng thái/ghi thanh ghi cuối.
- **Resume đi ngược lại, từ GỐC ra LÁ**: bật clock/regulator/bus **trước**, rồi mới đánh thức thiết bị con — nếu không, driver con ghi vào thanh ghi trong khi bus chưa có clock ⇒ treo hoặc bus error.
- ⇒ Đây chính là gốc của bug *"resume treo"* nói ở trên: **không phải driver viết sai, mà là sai THỨ TỰ**. Đó cũng là lý do `.suspend/.resume` phải nằm ở **driver**, nơi kernel biết vị trí của nó trên cây.
- **Đánh đổi khi thiết kế:** càng nhiều thiết bị khai `wakeup-source` thì càng tốn dòng lúc ngủ (phần đó không được tắt hẳn) — chọn wakeup source là chọn giữa **thời gian pin** và **độ nhạy đánh thức**.
</details>

## G — Real-time

#### BSP-021 · 🔴 · concept · ⭐ · [→ melp/debug-realtime](../../../15-book-summaries/melp/debug-realtime.md)
**Linux có phải hệ real-time không? PREEMPT_RT làm gì? (Vì sao vanilla không hard-RT?)**
<details><summary>Đáp án</summary>

- Ba nấc: (1) **vanilla**: không bảo đảm — latency đuôi ms→chục ms (đoạn kernel không preempt được, IRQ chen, page fault, lock contention); (2) **PREEMPT_RT tuned**: soft/firm RT — worst-case chục–trăm µs, *đo bằng cyclictest dưới tải*; (3) **hard RT an toàn tính mạng**: RTOS riêng hoặc kiến trúc lai (Cortex-M/AMP).
- PREEMPT_RT làm gì: **spinlock kernel → rt_mutex ngủ được** (đoạn giữ lock preempt được), **IRQ handler → kernel thread có priority**, priority inheritance phủ rộng, high-res timer. Đổi **throughput lấy chặn trên latency** (RT = deterministic, không phải nhanh). Mainline ~6.12.
- Bật RT chưa đủ — app phải: `SCHED_FIFO`, **`mlockall` + pre-fault**, mutex **PTHREAD_PRIO_INHERIT**, cấm malloc/I/O trong vòng RT, **isolcpus + IRQ affinity**, giới hạn C-state; kiểm chứng **cyclictest Max dưới tải đại diện**.
</details>

#### BSP-028 · 🔴 · design · ⭐ · 🏗️ · 📦 2026-08-13 · [→ rtos-vs-linux](../../../08-embedded-systems/rtos-vs-linux.md)
**Kiến trúc kết hợp RTOS + Linux trên cùng một thiết bị hoạt động thế nào, và giải quyết vấn đề gì?**
<details><summary>Đáp án</summary>

**Vấn đề:** sản phẩm cần **cả hai** thứ mà một hệ điều hành khó cho cùng lúc — **điều khiển tất định** (vòng lặp servo 10 kHz, không được trễ) **và** tính năng phong phú (màn hình, mạng, cập nhật OTA, filesystem). Linux giỏi vế sau, dở vế trước ([BSP-021](bsp.md)); RTOS thì ngược lại.

**Ba cách hiện thực, từ đơn giản tới phức tạp:**

| Cách | Mô hình | Đánh đổi |
|---|---|---|
| **Hai chip** | MCU riêng chạy RTOS + SoC chạy Linux, nối qua SPI/UART/Ethernet | Đơn giản nhất, **cách ly thật sự**; tốn linh kiện và không gian bo |
| ⭐ **SoC không đối xứng (AMP)** | Một SoC có **nhiều nhân khác loại** — nhân Cortex-A chạy Linux, nhân Cortex-M chạy RTOS, chia sẻ bộ nhớ | Phổ biến nhất hiện nay; một chip, cách ly vẫn tốt |
| **Hypervisor / đồng nhân** | Lớp ảo hoá hoặc lớp thời gian thực chen dưới Linux | Mạnh nhưng phức tạp, khó gỡ lỗi |

**Giao tiếp giữa hai bên** — đây là phần thiết kế thật:
- **Shared memory + hàng đợi vòng** cho dữ liệu tần suất cao, kèm cơ chế báo hiệu bằng ngắt liên nhân.
- **Nguyên tắc bất di dịch: phía realtime KHÔNG BAO GIỜ được chặn chờ phía Linux.** Hàng đợi phải **không khoá** hoặc ít nhất không cho phía RT chờ; Linux chậm hay treo thì vòng điều khiển **vẫn phải chạy đúng hạn**.
- Dữ liệu trạng thái ⇒ **đè cái cũ**, đừng chặn ([LNX-017](linux-sysprog.md)).

**Bốn vấn đề phải giải quyết ngay từ thiết kế:**
1. **Phân chia tài nguyên:** ngoại vi nào thuộc bên nào? Hai bên cùng chạm một khối ngoại vi là nguồn bug rất khó lần.
2. **Băng thông bộ nhớ dùng chung** — Linux quét bộ nhớ mạnh có thể làm **trễ** phía RT dù hai nhân độc lập.
3. **Khởi động và giám sát:** ai boot trước, ai nạp firmware cho ai, một bên chết thì bên kia làm gì.
4. **Gỡ lỗi và cập nhật** cho hai môi trường khác nhau — thường bị đánh giá thấp.

**Chốt:** *"Tách phần tất định sang RTOS (chip riêng hoặc nhân riêng) và để Linux lo phần phong phú, nối bằng shared memory + ring buffer. Luật quan trọng nhất: phía realtime không bao giờ được chặn chờ phía Linux."*
</details>

## H — Tình huống bring-up & debug (🏗️ trả lời theo khung)

#### BSP-022 · 🔴 · design · 🏗️ · ⭐ · [→ melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
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

#### BSP-023 · 🔴 · design · 🏗️ · [→ melp/debug-realtime](../../../15-book-summaries/melp/debug-realtime.md)
**Daemon crash ngẫu nhiên vài ngày/lần ngoài hiện trường, lab không tái hiện được?**
<details><summary>Đáp án (khung "hạ tầng bắt bằng chứng")</summary>

1. **Post-mortem trước**: bật core dump (`ulimit -c`, `core_pattern` vào partition data), giữ binary+.so **chưa strip đúng build** (build ID khớp), thu core+log+version về. Một core tốt thường đủ đóng án.
2. Core không đủ (heap corruption chết xa nguồn): bản ASan chạy staging/QEMU song song; nghi race → TSan trên mô phỏng tải.
3. **Log có chủ đích**: ring buffer RAM, flush khi crash; watchdog restart nhưng **giữ core trước khi restart**.
4. Tương quan metadata các lần chết: uptime (leak?), giờ (cron/logrotate?), thao tác khách.
- Từ khóa chấm điểm: core dump + symbol khớp build ID; tư duy "hạ tầng bằng chứng" thay vì "thêm log rồi chờ".
</details>

#### BSP-024 · 🔴 · design · 🏗️ · [→ melp/debug-realtime](../../../15-book-summaries/melp/debug-realtime.md)
**Ứng dụng điều khiển thỉnh thoảng trễ chu kỳ (CPU trung bình thấp)?**
<details><summary>Đáp án (khung "tracing, không phải profiling")</summary>

- Spike hiếm + CPU trung bình thấp → **tracing** (`trace-cmd record -e sched* -e irq*`), app tự phát hiện trễ ghi **trace_marker**; mở kernelshark xem thread bị gì.
- 5 họ nghi phạm: (1) bị task khác chen (→ SCHED_FIFO/affinity); (2) IRQ/softirq storm (→ IRQ affinity, threaded IRQ hạ priority); (3) chờ lock — **priority inversion** (→ PI mutex) hoặc I/O đồng bộ trong đường RT (log ghi flash); (4) cpuidle exit latency/cpufreq tụt (→ giới hạn C-state, governor); (5) major page fault (→ mlockall).
- Chốt: cyclictest định lượng nền tảng; phân biệt profiling ("trung bình ai tốn") vs tracing ("khoảnh khắc đó chuyện gì").
</details>

#### BSP-025 · 🔴 · design · 🏗️ · [→ ostep/virtualization-memory](../../../15-book-summaries/ostep/virtualization-memory.md), [melp/debug-realtime](../../../15-book-summaries/melp/debug-realtime.md)
**Thiết bị chạy vài tuần thì chậm dần rồi bị kill — hướng điều tra?**
<details><summary>Đáp án (khung memory)</summary>

- Triệu chứng khớp **memory leak → OOM killer** (Linux overcommit: malloc không fail, chết lúc chạm trang): xác nhận bằng `dmesg` (OOM log có bảng điểm process) + đồ thị **PSS theo thời gian** từng process (`smaps_rollup`, smem).
- Khoanh thủ phạm: process nào PSS tăng tuyến tính; trong process: valgrind/ASan ở staging, so `smaps` theo vùng (heap tăng = leak malloc, số mapping tăng = leak mmap/fd — kiểm `ls /proc/pid/fd | wc`).
- Nghi phạm ngoài heap: **page cache đầy là bình thường** ("free thấp" ≠ hết RAM); kernel leak (slab — slabtop); tmpfs phình (log vào /tmp!); fragmentation.
- Phòng bị: cgroup memory limit per-service, `oom_score_adj` bảo vệ process điều khiển, PSI monitoring cảnh báo sớm.
</details>

## I — Secure boot & chuỗi tin cậy

#### BSP-039 · 🟠 · concept · ⭐ · 🎤 2026-08-28 · [→ secure-boot §4.3](../../../08-embedded-systems/secure-boot.md)
**Trên thiết bị có secure boot, vì sao build một gói RPM rời để cài lúc phát triển cũng phải ký? Ký ở đâu, ai kiểm?**
<details><summary>Đáp án</summary>

**Vì secure boot KHÔNG dừng ở kernel.** Boot xong rootfs mới bắt đầu chạy — nếu không ai kiểm rootfs thì kẻ tấn công chỉ cần sửa một file trong đó. Nên sản phẩm thật có thêm cơ chế **runtime integrity**, và gói cài thêm phải thoả nó.

**⭐ Có HAI tầng ký, độc lập nhau — đây là phần bị nhầm nhiều nhất:**

| Tầng | Cái gì được ký | **Ai kiểm** | **Lúc nào** | Chặn được gì |
|---|---|---|---|---|
| **Chữ ký RPM (GPG)** | **Cả file `.rpm`** | `rpm` / package manager | Lúc **cài** | Gói giả từ kênh phân phối |
| ⭐ **Chữ ký IMA** | **Từng file bên trong** gói | 🔴 **Kernel** | Mỗi lần **chạy file đó** | Ai đó chép file lạ vào máy **sau khi** đã cài |

⇒ **Hệ quả rất dễ mất thời gian:** gói ký GPG hợp lệ nhưng file bên trong **thiếu chữ ký IMA** ⇒ `rpm` cài **không lỗi gì**, rồi lúc chạy binary thì kernel **từ chối `execve()`** — triệu chứng là **`Permission denied`** trên một file rõ ràng có quyền `x`. Người không biết IMA đang bật sẽ đi sửa `chmod`/`chown` vô ích.

**⇒ Vì sao máy build phải chạm khoá:** chữ ký IMA nằm trong **xattr `security.ima`** của **từng file**, nên phải ký **lúc đóng gói**. Đó chính là lý do khâu build gói lẻ cũng cần khoá.

**Phân biệt với dm-verity — quyết định gói rời có cài được không:**

| | **dm-verity** | **IMA/EVM** |
|---|---|---|
| Phạm vi | Cả phân vùng (cây hash, hash gốc được ký) | Từng file |
| Rootfs | **Chỉ đọc** ⇒ 🔴 **KHÔNG cài gói rời được**, phải build lại cả image | Ghi được, gói rời **cài được** nếu file đã ký |

**Chẩn đoán:**
```bash
getfattr -m . -d /usr/bin/myapp        # co security.ima chua?
dmesg | grep -i "ima\|appraise"        # kernel tu choi vi sao
cat /sys/kernel/security/ima/policy    # chinh sach dang ap
rpm -K mypackage.rpm                   # chu ky GPG cua goi
```

**⚠️ Bẫy:** (1) 🔴 tưởng `Permission denied` là **lỗi quyền file** — thực ra là **IMA từ chối**; (2) nhầm chữ ký RPM với chữ ký IMA — hai tầng, hai người kiểm, hai thời điểm; (3) `scp` binary lên máy rồi `chmod +x` và mong nó chạy — thiếu xattr thì không chạy; (4) test trên máy **dev** (IMA/verity **tắt**) rồi tưởng xong — đúng lớp lỗi này **chỉ lộ ra trên máy production**; (5) dùng **khoá production** trên máy dev để cho tiện — phá đúng lý do secure boot tồn tại.

**Chốt:** *"Secure boot không dừng ở kernel — rootfs có dm-verity, từng file có IMA. Gói RPM phải ký ở **hai tầng**: GPG cho cả gói (package manager kiểm lúc cài) và IMA cho từng file (kernel kiểm lúc chạy). Thiếu tầng hai thì cài xong vẫn `Permission denied`."*
</details>

#### BSP-026 · 🟠 · concept · ⭐ · [→ secure-boot](../../../08-embedded-systems/secure-boot.md)
**Secure boot hoạt động thế nào? Vì sao mọi package phải được ký?**
<details><summary>Đáp án</summary>

**Cơ chế — chuỗi tin cậy, mỗi tầng verify tầng kế:**

```
BootROM (bat bien) ─verify─> SPL/BL2 ─verify─> U-Boot/BL33 ─verify─> kernel
   hash(pubkey) o eFuse                                                │
   ← GOC TIN CAY                                              dm-verity│ rootfs
                                                              IMA/EVM  │ tung file
```

Ký = **hash nội dung** rồi **ký hash bằng khoá riêng**; thiết bị giữ **khoá công khai** để kiểm. Hash trước vì ký bất đối xứng rất chậm và chỉ làm việc trên khối nhỏ.

**⭐ "Vì sao" hai tầng:**
- *Tầng nông*: *"để không ai thay được firmware."*
- *Tầng sâu*: **chuỗi chỉ mạnh bằng mắt xích đầu tiên.** Nếu tầng đi kiểm mà **sửa được** thì kẻ tấn công chỉ cần **tắt việc kiểm**. ⇒ gốc phải nằm ở **mask ROM** (cố định lúc sản xuất chip) + khoá trong **eFuse/OTP** (ghi một lần, đứt cầu chì vật lý). Đó là lý do **verified boot ở U-Boot ≠ secure boot**: U-Boot kiểm kernel, nhưng **không ai kiểm U-Boot**.

**⚠️ Không phải mã hoá.** Secure boot lo **toàn vẹn + xác thực**, KHÔNG lo **bí mật**. Firmware đã ký vẫn dump ra đọc được nguyên vẹn — nó chỉ **không sửa được mà vẫn boot**.

**⇒ Vì sao "mọi package phải được ký" — vì nó KHÔNG dừng ở kernel.** Boot xong rootfs mới chạy; không ai kiểm rootfs thì chỉ cần sửa một file trong đó. Nên có thêm hai cơ chế runtime, **khác nhau**:

| | **dm-verity** | **IMA/EVM** |
|---|---|---|
| Bảo vệ | **Cả phân vùng** (cây hash, hash gốc được ký) | **Từng file** (chữ ký trong xattr `security.ima`) |
| Kiểm khi | Mỗi lần đọc block | Mỗi lần `execve()`/`open()` |
| Rootfs | **Chỉ đọc** ⇒ **không cài gói rời được** | Ghi được, nhưng file mới phải có chữ ký hợp lệ |

**⚠️ Bẫy:** (1) nhầm ký với **mã hoá**; (2) nhầm **verified boot** (neo ở U-Boot) với **secure boot** (neo ở ROM/eFuse); (3) tưởng nó chống được kẻ **đã có root** — không, đó là mô hình đe doạ khác; (4) **mất khoá riêng còn tệ hơn lộ**: không cập nhật được cho toàn bộ thiết bị đã bán, mà fuse thì không đảo ngược; (5) quên **anti-rollback** — firmware cũ *ký thật, hợp lệ* nhưng có CVE vẫn flash đè được, phải có counter đơn điệu.

**Chốt:** *"Mỗi tầng verify tầng kế, và chuỗi chỉ mạnh bằng mắt xích đầu — nên gốc phải bất biến: ROM + eFuse. Nó không dừng ở kernel: rootfs có dm-verity, từng file có IMA — đó mới là lý do package cũng phải ký."*
</details>

---

## 🧪 J — LAB NGỒI MÁY (BeagleBone Black)

> **Bộ 8 bài thực hành trên phần cứng thật**, thêm 2026-08-27. Khác bộ [🧪 DBG-030…036](debugging.md) ở **cấu trúc**, và khác có chủ đích:
>
> | | DBG lab | **BSP lab (bộ này)** |
> |---|---|---|
> | Lỗ hổng nhắm tới | *"chẩn đoán được, **chọn công cụ** không được"* | *"**thuộc bài ≠ hiểu bài**"* (đo 19/08) |
> | Cấu trúc | **Triệu chứng trước** | **Khái niệm trước**, triệu chứng thành bước kiểm chứng |
>
> Ở DBG, mô hình đã có — chỉ thiếu ánh xạ triệu chứng→công cụ. Ở BSP thì **mô hình chưa có**, nên đưa triệu chứng ra trước là bắt chẩn đoán một hệ thống chưa hình dung được.
>
> **Bốn bước, áp cho mọi bài:**
> **① ĐỌC** (khái niệm, ngắn) → **② QUAN SÁT** nó chạy đúng trên board, bắt log thật → **③ PHÁ có chủ đích** (viết **dự đoán ra giấy TRƯỚC** khi cắm điện) → **④ ĐỐI CHIẾU** bảng mình tự dựng với bảng trong repo.
>
> ⭐ **Bước ③ là chỗ khác biệt lớn nhất: bạn KHÔNG chờ triệu chứng — bạn gây ra nó.** Tự tay phá thì biết ground truth, nên mọi suy luận ở ④ kiểm chứng được ngay. Chỗ **dự đoán sai** chính là chỗ mô hình còn hổng — đó là dữ liệu quý nhất của cả buổi.

### ⚠️ Điều kiện cần

> 🔧 **Toàn bộ hướng dẫn setup sống ở [14-prep/lab-setup.md](../../lab-setup.md)** — phần cứng cần mua, đấu nối serial J1, gói host, bố cục thẻ SD, **6 bẫy đã biết**, và **checklist sẵn sàng**.
>
> **Không chép lại ở đây.** 8 bài dùng chung một setup; giữ nhiều bản là đúng cái bệnh *"một sự thật, hai chỗ"* mà repo đã dính 4 lần ([CLAUDE.md §4.7](../../../CLAUDE.md)).
>
> 🔴 **Ba thứ chặn đường, kiểm trước khi mở bài 031:** ① **cáp USB–TTL 3.3V** (cáp 5V làm hỏng board — không có console thì cả Thẻ A vô nghĩa) · ② **nút S2** (eMMC tranh boot với thẻ SD, triệu chứng giống hệt *"image mình hỏng"*) · ③ **thẻ A luôn boot được** làm thẻ cứu hộ.

> ⚠️ **Về lệnh trong các bài dưới:** chưa chạy trên board của bạn — tên biến U-Boot, địa chỉ nạp, đường dẫn **đổi theo phiên bản**. Coi là *chỉ dẫn cần đối chiếu*. Ô **OUTPUT** để trống là **có chủ đích**: theo [bank/README.md](README.md), output câu `lab` phải là **output chạy thật, dán nguyên văn** — cấm viết tay, cấm phỏng đoán.

---

#### BSP-031 · 🟡 · lab 🧪 · ⭐ · 🎤 2026-08-27 · [→ boot-process §1–3, §7](../../../08-embedded-systems/boot-process.md)
**🧪 Bốn giai đoạn boot — nhận ra từng cái trên log THẬT của mình.**

**Bối cảnh thực tế:** bạn nhận một board mới, cắm điện, và phải nói được *"nó đang chết ở giai đoạn nào"*. Không nói được điều đó thì mọi bước bring-up sau chỉ là đoán.

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC (~20 phút, không hơn)
[boot-process.md §1–§3](../../../08-embedded-systems/boot-process.md). Chỉ cần trả lời **một** câu: *vì sao boot phải nhiều giai đoạn?*

> Đáp án đúng đã có trong doc: **không phải "cho gọn"**, mà vì **SRAM nội quá nhỏ** (vài chục–vài trăm KB). BootROM chỉ nạp nổi **SPL** vào SRAM; SPL có mỗi một việc quan trọng: **khởi tạo DRAM**, để từ đó mới nạp nổi U-Boot đầy đủ rồi tới kernel. **Ràng buộc kích thước SRAM chính là lý do tồn tại của SPL.**

### ② QUAN SÁT — sản phẩm quan trọng nhất của cả bài
Boot thẻ A (đang chạy được), bắt **toàn bộ** log từ lúc cấp nguồn:
```bash
# tren may host
picocom -b 115200 /dev/ttyUSB0 --logfile boot-ok.log
#  hoac:  screen /dev/ttyUSB0 115200      (Ctrl-A H de bat log)
```
Rồi **tự đánh dấu bốn ranh giới**:

| Dòng trong log | Nghĩa là gì |
|---|---|
| `U-Boot SPL 20xx.xx` | SPL đang chạy **trong SRAM** — DRAM **chưa** có |
| `U-Boot 20xx.xx` | U-Boot proper, giờ **đã ở trong DRAM** |
| `Starting kernel ...` | **Trao tay** — hết phần bootloader |
| `Linux version ...` | Kernel banner |
| `... login:` | Userspace, init đã chạy |

📋 **DÁN LOG CÓ CHÚ THÍCH CỦA BẠN VÀO ĐÂY** *(chưa chạy)*
```
(chua chay)
```

⭐ Đây là thứ mang vào phòng phỏng vấn được. Khi bị hỏi *"kể chuỗi boot"*, bạn kể **từ một cái log mình từng đọc**, không phải từ danh sách học thuộc — và người nghe phân biệt được hai thứ đó.

### ③ PHÁ CÓ CHỦ ĐÍCH
**Luật: viết dự đoán ra giấy TRƯỚC khi cắm điện.**

| Phá gì | Dự đoán của bạn | Thực tế |
|---|---|---|
| Đổi tên `MLO` → `MLO.bak` | *(điền trước)* | *(chưa chạy)* |
| Khôi phục; đổi tên `u-boot.img` | *(điền trước)* | *(chưa chạy)* |
| Khôi phục; sửa `bootargs` sai `root=` | *(điền trước)* | *(chưa chạy)* |

### ④ ĐỐI CHIẾU
[boot-process.md §7](../../../08-embedded-systems/boot-process.md) đã có sẵn bảng **"Chẩn đoán theo CHỖ CHẾT"** (6 dòng). Đối chiếu bảng bạn vừa dựng với nó — ba khả năng, cả ba đều có ích:

- **Khớp** ⇒ khái niệm giờ có bằng chứng của chính bạn đứng sau
- **Bạn thiếu dòng** ⇒ ca chưa gặp, đọc kỹ dòng đó
- **Bạn có dòng bảng không có** ⇒ 📌 **bổ sung vào doc**, repo tốt lên

**Vì sao bài này quan trọng nhất bộ:** nó đóng đúng vòng lặp đã hỏng ngày 19/08 — *bảng trên giấy → bảng do tay mình xác nhận*.

**Nối câu hỏi:** [BSP-001](#) *(chuỗi boot nhiều giai đoạn)* · [BSP-002](#) · [BSP-022](#) *(bring-up 🏗️)*
</details>

#### BSP-032 · 🟡 · lab 🧪 · ⭐ · 🎤 2026-08-27 · [→ boot-process §3](../../../08-embedded-systems/boot-process.md)
**🧪 U-Boot trao tay kernel — nó trao CÁI GÌ, cho AI, ở ĐÂU.**

**Bối cảnh thực tế:** đổi sang kernel mới, board dừng ngay sau `Starting kernel ...` và im lặng. Không hiểu bước trao tay thì không biết nghi gì.

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
[boot-process.md §3](../../../08-embedded-systems/boot-process.md) — ba thứ U-Boot phải đưa cho kernel: **kernel image**, **device tree blob**, **command line (`bootargs`)**. Thiếu hoặc sai địa chỉ bất kỳ cái nào ⇒ chết ngay sau `Starting kernel`.

### ② QUAN SÁT
Nhấn phím bất kỳ để dừng autoboot, rồi:
```
=> printenv bootcmd
=> printenv bootargs
=> printenv loadaddr fdtaddr
=> bdinfo
```
📋 **DÁN OUTPUT THẬT** *(chưa chạy)*
```
(chua chay)
```
**Câu phải trả lời được sau bước này:** `bootz ${loadaddr} - ${fdtaddr}` — **dấu `-` ở giữa nghĩa là gì?** *(gợi ý: vị trí đó dành cho initramfs)*

### ③ PHÁ
| Phá gì | Dự đoán | Thực tế |
|---|---|---|
| `setenv fdtaddr 0x8FFFFFFF` (địa chỉ vô lý) rồi `boot` | | *(chưa chạy)* |
| Khôi phục; bỏ hẳn `console=` khỏi `bootargs` | | *(chưa chạy)* |
| Khôi phục; `bootz ${loadaddr}` (bỏ luôn dtb) | | *(chưa chạy)* |

⭐ Ca thứ hai đáng chú ý nhất: kernel **vẫn boot bình thường**, chỉ là bạn **không thấy gì**. Phân biệt *"chết"* với *"sống nhưng câm"* là một lớp lỗi riêng.

### ④ ĐỐI CHIẾU
Ba ca trên rơi vào dòng nào của bảng §7? Ca nào **không** có trong bảng?

**Nối câu hỏi:** [BSP-002](#) · [BSP-030](#) *(init PID 1)*
</details>

#### BSP-033 · 🟡 · lab 🧪 · ⭐ · 🎤 2026-08-27 · [→ BSP-029](#) · [boot-process §4–5](../../../08-embedded-systems/boot-process.md)
**🧪 Rootfs & initramfs — tự tay chứng minh khi nào KHÔNG cần initramfs.**

> 🔴 **Bài ưu tiên cao nhất bộ.** [BSP-029](#) là câu bạn đạt **2 điểm** ngày 19/08 vì **đảo ngược luật và ngoại lệ** — trình bày *"khi nhúng vẫn nên dùng"* như luật chung, trong khi bank đánh ⭐ vào *"thiết bị nhúng thường **KHÔNG** cần"*. Cách chắc chắn nhất để sửa: **boot một hệ thống không có initramfs bằng chính tay mình.**

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
[BSP-029](#) + [boot-process.md §4–§5](../../../08-embedded-systems/boot-process.md). Câu phải trả lời: **initramfs sinh ra để giải bài toán con-gà-quả-trứng nào?** *(kernel cần driver để đọc rootfs, mà driver lại nằm trong rootfs)*
Và: **vì sao thiết bị nhúng thường thoát được bài toán đó?** *(driver MMC/eMMC build thẳng `=y` vào kernel ⇒ không cần ai nạp hộ)*

### ② QUAN SÁT
```bash
zcat /proc/config.gz | grep -E "CONFIG_MMC_OMAP_HS|CONFIG_EXT4_FS"   # =y hay =m ?
cat /proc/cmdline
ls /boot/                                                             # co initramfs khong?
```
📋 **DÁN OUTPUT THẬT** *(chưa chạy)*
```
(chua chay)
```

### ③ PHÁ
| Phá gì | Dự đoán | Thực tế |
|---|---|---|
| `setenv bootargs ... root=/dev/mmcblk0p9` (phân vùng không tồn tại) | | *(chưa chạy)* |
| Khôi phục; đổi `rootfstype=` sang kiểu sai | | *(chưa chạy)* |
| **Nếu đang có initramfs**: bỏ nó ra, boot thẳng | | *(chưa chạy)* |

### ④ ĐỐI CHIẾU
Trả lời lại [BSP-029](#) **bằng lời mình**, và lần này **nêu LUẬT trước, NGOẠI LỆ sau**:
> *"Nhúng thường **không** cần, vì driver lưu trữ build thẳng vào kernel và phần cứng biết trước. **Cần** khi: rootfs mã hoá/nén, cần chọn rootfs lúc chạy (A/B), hoặc cần driver dạng module trước khi mount."*

**Nối câu hỏi:** 🔴 [BSP-029](#) *(sổ yếu)* · [BSP-030](#)
</details>

#### BSP-034 · 🟠 · lab 🧪 · 🎤 2026-08-27 · [→ boot-process §7](../../../08-embedded-systems/boot-process.md)
**🧪 Boot chậm — đo xem chậm ở đâu, đừng đoán.**

> ⚠️ **Bài DUY NHẤT trong bộ giữ dạng triệu chứng.** Ở đây bạn **đã có** mô hình — chính bạn làm ca `insmod` 3–4 s → <0.5 s. Đây là tối ưu, không phải học mới, nên hợp dạng DBG lab.

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
Ba tầng đo, mỗi tầng một công cụ: **U-Boot** (`bootstage`) · **kernel** (`initcall_debug` + `dmesg` timestamp) · **userspace** (`systemd-analyze blame`).

### ② QUAN SÁT — đo TRƯỚC khi sửa bất cứ gì
```bash
# kernel: them vao bootargs
initcall_debug ignore_loglevel
# sau khi boot:
dmesg | sort -k1 -n | tail -30            # initcall lau nhat
systemd-analyze                            # neu co systemd
systemd-analyze blame | head -20
```
📋 **DÁN OUTPUT THẬT + tổng thời gian boot** *(chưa chạy)*
```
(chua chay)
```

### ③ PHÁ / THỬ
| Thử gì | Dự đoán | Thực tế |
|---|---|---|
| `setenv bootdelay 0` | | *(chưa chạy)* |
| Tắt một service userspace chậm nhất trong `blame` | | *(chưa chạy)* |
| Đổi một driver `=y` thành `=m` (hoặc ngược lại) | | *(chưa chạy)* |

### ④ ĐỐI CHIẾU
Kể lại thành **một câu chuyện có số**, đúng khuôn ca `insmod` của bạn: *trước → nghi gì → đo gì → sửa gì → sau*.

⭐ Bạn sẽ có **câu chuyện tối ưu boot thứ hai** — lần này trên phần cứng **không phải của Samsung**, tức kể được **không vướng NDA**. Đó là tài sản riêng, đáng giá.

**Nối câu hỏi:** [BSP-001](#) · và chính mục *Driver Load-Time Optimization* trên [RESUME](../../../RESUME_embedded_linux.tex)
</details>

#### BSP-035 · 🟠 · lab 🧪 · ⭐ · 🎤 2026-08-27 · [→ BSP-004](#) · [melp/bootloader-kernel](../../../15-book-summaries/melp/bootloader-kernel.md)
**🧪 FIT + ký RSA — làm cho U-Boot TỪ CHỐI một image đã bị sửa một byte.**

> ⭐ **Bài đắt nhất bộ.** Khoảnh khắc U-Boot **từ chối** image bạn cố tình sửa là thứ không sách nào thay được. Nó lấp đúng ô 🟠 *"Secure boot / TF-A chiều sâu"* đang treo trong [gap-register](../../study-plans/gap-register.md), và vá đúng chỗ bạn hỏi *"SHA+RSA là gì?"* ngày 19/08.

> 🔴 **ĐỌC TRƯỚC KHI LÀM — giới hạn của phần cứng.** AM335x trên BBB bán lẻ là silicon **GP (General Purpose)**: eFuse chưa blow ⇒ **KHÔNG có chain of trust từ ROM**, tức **không làm được secure boot đúng nghĩa**. Muốn vậy phải có silicon **HS**, và blow eFuse là **không thể hoàn tác**.
> Thứ làm được — và **đúng là thứ interview hỏi** — là **U-Boot verified boot**: U-Boot tự kiểm chữ ký của FIT trước khi boot. Khi nói ở phỏng vấn, **nêu đúng ranh giới này** — nó **cộng điểm**, vì phân biệt được *verified boot ở bootloader* với *secure boot neo ở ROM/eFuse* là thứ nhiều người nhầm.

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
[BSP-004](#). Ba thứ FIT hơn uImage: **nhiều thành phần + nhiều cấu hình** · **hash/chữ ký từng thành phần** · **metadata rõ (load address, entry, compression)**.
Câu phải trả lời: **SHA và RSA làm hai việc KHÁC nhau — việc gì?**
> **SHA** = *"nội dung có bị đổi không"* (toàn vẹn). **RSA** = *"ai ký cái hash đó"* (xác thực nguồn). Chỉ SHA thì kẻ tấn công sửa image **và** sửa luôn hash. Ký hash bằng khoá riêng, U-Boot kiểm bằng khoá công khai **nhúng trong `u-boot.dtb`** ⇒ sửa image mà không có khoá riêng thì chữ ký hỏng.

### ② QUAN SÁT — dựng FIT chưa ký trước
Viết `.its` mô tả kernel + dtb + configuration, rồi:
```bash
mkimage -f board.its board.itb
mkimage -l board.itb            # xem lai cau truc
```
📋 **DÁN `.its` + output `mkimage -l`** *(chưa chạy)*
```
(chua chay)
```

### ③ KÝ, RỒI PHÁ
```bash
openssl genpkey -algorithm RSA -out keys/dev.key -pkeyopt rsa_keygen_bits:2048
openssl req -batch -new -x509 -key keys/dev.key -out keys/dev.crt
mkimage -f board.its -k keys -K u-boot.dtb -r board.itb     # ky + nhung pubkey vao dtb
# build lai U-Boot voi CONFIG_FIT_SIGNATURE=y va dtb da co khoa
```
| Thử gì | Dự đoán | Thực tế |
|---|---|---|
| Boot FIT **đã ký, chưa sửa** | | *(chưa chạy)* |
| **Sửa 1 byte** trong `.itb` rồi boot | | *(chưa chạy)* |
| Ký bằng **khoá khác** với khoá trong dtb | | *(chưa chạy)* |

📋 **DÁN nguyên văn dòng U-Boot từ chối** *(chưa chạy)* — đây là output đắt nhất cả bộ
```
(chua chay)
```

### ④ ĐỐI CHIẾU
Trả lời lại [BSP-004](#) và thêm một câu về ranh giới:
> *"Đây là **verified boot ở tầng U-Boot** — U-Boot tự kiểm FIT. Nó **không** phải secure boot đầy đủ: gốc tin cậy vẫn là U-Boot, mà U-Boot thì chưa được ai kiểm. Muốn khép chuỗi phải neo ở ROM/eFuse, cần silicon HS."*

⭐ Câu *"gốc tin cậy vẫn là U-Boot, mà U-Boot thì chưa được ai kiểm"* là câu đáng nhớ nhất — nó cho thấy bạn hiểu **chuỗi tin cậy chỉ mạnh bằng mắt xích đầu tiên**.

**Nối câu hỏi:** [BSP-004](#) · mục **I — Secure boot & chuỗi tin cậy**
</details>

#### BSP-036 · 🟡 · lab 🧪 · ⭐ · 🎤 2026-08-27 · [→ BSP-017](#) · [yocto](../../../06-build-systems/yocto.md)
**🧪 Yocto dựng lại ĐÚNG thứ bạn vừa làm tay — rồi so hai bên.**

> **Thẻ B bắt đầu từ đây.** Thứ tự này có chủ đích: làm tay trước khiến giá trị của Yocto trở nên **hiển nhiên**. Nếu Yocto đi trước, U-Boot chỉ là **hộp đen bitbake nhả ra** — bạn học recipe mà không hiểu nó sinh ra cái gì.

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
[BSP-017](#) *(Yocto vs Buildroot)* + [yocto.md §1–2](../../../06-build-systems/yocto.md). Câu phải trả lời **trước khi build**: *Yocto giải quyết vấn đề gì mà làm tay không giải được?* — rồi **sau khi build, đọc lại câu trả lời của mình**.

### ② QUAN SÁT
```bash
git clone -b scarthgap git://git.yoctoproject.org/poky
cd poky && source oe-init-build-env
# conf/local.conf:  MACHINE = "beaglebone-yocto"
time bitbake core-image-minimal
ls tmp/deploy/images/beaglebone-yocto/
```
📋 **DÁN: thời gian build lần đầu + danh sách artifacts** *(chưa chạy)*
```
(chua chay)
```

⭐ **Bảng so sánh — đây mới là bài học, không phải phụ phẩm:**

| Artifact | Thẻ A (tay) — lấy ở đâu | Thẻ B (Yocto) — sinh ra sao |
|---|---|---|
| `MLO` | *(điền)* | *(điền)* |
| `u-boot.img` | *(điền)* | *(điền)* |
| kernel + dtb | *(điền)* | *(điền)* |
| rootfs | *(điền)* | *(điền)* |

### ③ PHÁ / THỬ
| Thử gì | Dự đoán | Thực tế |
|---|---|---|
| Flash thẻ B, boot — log có **giống** thẻ A không? | | *(chưa chạy)* |
| `bitbake -e core-image-minimal \| grep ^MACHINE=` | | *(chưa chạy)* |
| Đổi `MACHINE` sang giá trị sai rồi build | | *(chưa chạy)* |

### ④ ĐỐI CHIẾU
Trả lời lại [BSP-017](#) — lần này bằng **trải nghiệm**, không phải định nghĩa. Câu chốt nên có: *thứ Yocto cho mà làm tay không cho là **tái lập được** — cùng một commit cho ra cùng một image, trên máy khác, sáu tháng sau.*

**Nối câu hỏi:** [BSP-017](#) · [BLD-*](build-systems.md)
</details>

#### BSP-037 · 🟠 · lab 🧪 · ⭐ · 🎤 2026-08-27 · [→ BSP-018](#) · [yocto §5](../../../06-build-systems/yocto.md)
**🧪 bbappend + `DEPENDS` vs `RDEPENDS` — và bắt tận tay chỗ `shlibs` KHÔNG tự đoán được.**

> 🔴 Ngày 19/08 bạn trả lời [BSP-018](#) được **3 điểm** nhưng khi bị hỏi *"khi nào `shlibs` không tự phát hiện được?"* thì **"chưa rõ"**. Bài này thiết kế để **nhìn thấy nó hỏng một lần** — sau đó không quên được.

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
[yocto.md §5](../../../06-build-systems/yocto.md) + [BSP-018](#).
- **`DEPENDS`** = cần lúc **BUILD** (thư viện + header để compile)
- **`RDEPENDS`** = cần lúc **CHẠY** trên thiết bị
- Yocto tự sinh phần lớn `RDEPENDS` nhờ quét **`shlibs`** — đọc `NEEDED` trong ELF của binary đã build.

### ② QUAN SÁT — bbappend không fork source
Viết một `.bbappend` đổi **một** thứ trong U-Boot (ví dụ `bootdelay` hoặc thêm biến env):
```bash
bitbake-layers create-layer ../meta-mylab
bitbake-layers add-layer ../meta-mylab
# meta-mylab/recipes-bsp/u-boot/u-boot_%.bbappend
devtool modify u-boot           # cach de sua + xem diff
```
📋 **DÁN `.bbappend` + xác nhận thay đổi xuất hiện trên board** *(chưa chạy)*
```
(chua chay)
```

### ③ PHÁ — ⭐ phần đắt nhất bài này
Đóng gói **một plugin nạp bằng `dlopen()`** (không link trực tiếp), rồi:

| Thử gì | Dự đoán | Thực tế |
|---|---|---|
| `readelf -d app \| grep NEEDED` — có thấy thư viện plugin không? | | *(chưa chạy)* |
| Cài image lên board, chạy app — nó tìm thấy plugin không? | | *(chưa chạy)* |
| Thêm `RDEPENDS:${PN} += "libplugin"` rồi build lại | | *(chưa chạy)* |

⭐ **Đây chính là câu trả lời bạn thiếu:** `shlibs` quét `NEEDED` trong ELF — mà **`dlopen()` không tạo ra `NEEDED`**. Thư viện được mở **lúc chạy, theo tên chuỗi**, nên build system **không thể** biết. Cùng lớp vấn đề: file cấu hình, script, dữ liệu, binary gọi qua `system()`. Những thứ đó **phải khai `RDEPENDS` bằng tay**.

### ④ ĐỐI CHIẾU
Trả lời lại [BSP-018](#) kèm **ca cụ thể vừa dựng**. Câu chốt: *"`shlibs` chỉ thấy được thứ linker ghi vào ELF. Cái gì phân giải **lúc chạy theo tên** — `dlopen`, script, file cấu hình — thì nó mù, và mình phải khai tay."*

**Nối câu hỏi:** 🔴 [BSP-018](#) · [SD-030](system-design.md) *(`dlopen`/`dlsym`)*
</details>

#### BSP-038 · 🟡 · lab 🧪 · 🎤 2026-08-27 · [→ yocto](../../../06-build-systems/yocto.md)
**🧪 sstate — đo xem nó thật sự tiết kiệm bao nhiêu, đừng tin lời quảng cáo.**

<details><summary>Bốn bước · chỗ dán output · vì sao</summary>

### ① ĐỌC
[yocto.md](../../../06-build-systems/yocto.md) mục sstate. Ý cốt lõi: sstate cache **kết quả từng task**, không phải từng recipe ⇒ đổi một thứ nhỏ thì chỉ **những task phụ thuộc nó** chạy lại.

### ② QUAN SÁT — ba phép đo
```bash
time bitbake core-image-minimal                      # (1) da build roi -> gan nhu tuc thi?
bitbake -c cleansstate u-boot && time bitbake core-image-minimal   # (2) build lai rieng u-boot
rm -rf tmp/ && time bitbake core-image-minimal       # (3) xoa tmp, GIU sstate
```
📋 **DÁN ba con số thật** *(chưa chạy)*

| Phép đo | Thời gian |
|---|---|
| (1) không đổi gì | *(chưa chạy)* |
| (2) sau `cleansstate u-boot` | *(chưa chạy)* |
| (3) xoá `tmp/`, giữ `sstate-cache/` | *(chưa chạy)* |

⭐ **Phép đo (3) là phép đo dạy nhiều nhất** — nó cho thấy sstate là thứ tách *"kết quả build"* khỏi *"thư mục làm việc"*. Đây là lý do CI dùng được nó.

### ③ PHÁ
| Thử gì | Dự đoán | Thực tế |
|---|---|---|
| Đổi một dòng trong `.bbappend` rồi build lại — bao nhiêu task chạy lại? | | *(chưa chạy)* |
| `bitbake -S printdiff core-image-minimal` | | *(chưa chạy)* |

### ④ ĐỐI CHIẾU
Câu chốt cho phỏng vấn: *"sstate cache theo **task** và khoá theo **hash của đầu vào task đó**. Đổi một biến ảnh hưởng tới nhiều task thì nhiều thứ chạy lại — nên câu hỏi thật không phải 'sstate nhanh không' mà là **'thay đổi của tôi làm hỏng hash của bao nhiêu task'**."*

**Nối câu hỏi:** [BSP-017](#) · [BSP-018](#) · [BLD-*](build-systems.md)
</details>

---

⬅️ [Bank index](README.md)
