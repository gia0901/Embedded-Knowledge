# MELP — Bootloader & Kernel & Device Tree (ch. 3–4) 🎯🎯

> Thuộc [MELP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Đây là file quan trọng nhất của cuốn sách với vị trí BSP — boot flow và device tree xuất hiện trong gần như mọi buổi phỏng vấn.

---

## Cụm 1 — Bootloader: từ cấp nguồn đến kernel (ch. 3) 🎯

### Nội dung chính

**Boot flow nhiều giai đoạn — và VÌ SAO phải nhiều giai đoạn.** Ràng buộc vật lý: lúc cấp nguồn, **DRAM chưa hoạt động** (controller chưa được init/training), CPU chỉ có **on-chip SRAM** (vài chục–trăm KB) và ROM. Mỗi giai đoạn tồn tại để vượt một ràng buộc:

```
[1] ROM code (Boot ROM — nằm trong silicon, không đổi được)
    • Đọc boot pins/strap/eFuse → chọn nguồn boot (eMMC/SD/SPI-NOR/UART/USB)
    • Nạp GIAI ĐOẠN KẾ vào SRAM (vì DRAM chưa có!) → giới hạn size gắt gao
        ▼
[2] SPL / MLO (Secondary Program Loader — U-Boot rút gọn vừa SRAM)
    • Việc số 1: INIT DRAM (điều lớn nhất nó làm) + clock/PMIC tối thiểu
    • Nạp U-Boot đầy đủ vào DRAM
        ▼
[3] U-Boot proper (full bootloader, chạy trong DRAM)
    • Init phần cứng còn lại (storage, network, console)
    • Môi trường biến (env), script, shell tương tác
    • Nạp: kernel image + DTB (+ initramfs) vào RAM → bootm/booti
        ▼
[4] Kernel: giải nén, early init, mount rootfs, chạy init (PID 1)
```

Trên **ARM64 + secure boot**, chuỗi chuẩn hóa bằng **TF-A (Trusted Firmware-A)**: `BL1 (ROM) → BL2 (≈SPL) → BL31 (secure monitor EL3, ở lại runtime — PSCI: bật/tắt core, suspend) → BL33 (U-Boot) → kernel`; mỗi bước **verify chữ ký** bước sau = **chain of trust** bắt rễ từ khóa trong eFuse/OTP.

**U-Boot — những thứ phải thuộc:**
- **Environment**: bộ biến key=value lưu ở vị trí cấu hình sẵn (offset trong eMMC/SPI flash, file `uboot.env`, hoặc trong FS). Quan trọng nhất: `bootcmd` (lệnh tự chạy khi hết `bootdelay`), `bootargs` (**kernel command line** — `console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait`), địa chỉ nạp (`loadaddr`...). Lệnh: `printenv/setenv/saveenv`.
- **Nạp & boot**: `load mmc 0:1 ${loadaddr} Image` + `booti ${loadaddr} - ${fdtaddr}` (ARM64; ARM32: `bootz`/`bootm`). Kernel + DTB (+ initramfs) phải nằm ở địa chỉ RAM không đè nhau.
- **FIT image** (Flattened Image Tree): gói kernel + nhiều DTB + initramfs + **chữ ký** trong một file (nguồn `.its` → `mkimage` → `.itb`) — chọn cấu hình theo board lúc boot, nền của **verified boot** trong U-Boot. Thay thế uImage legacy.
- U-Boot cũng dùng **device tree cho chính nó** (`u-boot.dtb`) và có driver model (DM) riêng — cùng bindings với kernel ở mức lớn.
- **Falcon mode**: SPL nạp thẳng kernel bỏ qua U-Boot proper — cắt thời gian boot (yêu cầu boot nhanh <1–2s là chuyện thật của automotive/camera).
- Công cụ đối chứng: nhìn boot log tách được từng giai đoạn (banner SPL → banner U-Boot → `Starting kernel ...`).

### Insight đáng nhớ

- Câu trả lời "vì sao cần SPL" gói trong một câu: **"chuỗi boot là chuỗi bootstrap tài nguyên — mỗi tầng chỉ đủ khả năng dựng tầng kế: ROM biết đọc thiết bị, SPL biết dựng DRAM, U-Boot biết mọi thứ còn lại."** Nói được câu này là qua điểm boot flow.
- `bootargs` là **hợp đồng giữa bootloader và kernel** — nửa số lỗi "boot treo" nằm ở đây: sai `console=` (im lặng như chết), sai `root=` (panic VFS unable to mount), thiếu `rootwait` (mmc chưa kịp enumerate). Debug boot = đọc kỹ hợp đồng này trước.

### Ít quan trọng

- Barebox và các bootloader khác (ch. 3 phần so sánh) — biết tên; chi tiết port U-Boot sang board mới từng file (đọc khi làm thật, khung: `configs/<board>_defconfig`, DT của board, `board/<vendor>/<board>/`).

### Góc interview

**Câu 1 (🎯 mở màn kinh điển):** Kể chi tiết quá trình từ lúc cấp nguồn đến khi có shell trên board ARM chạy Linux. Vì sao không nạp thẳng U-Boot từ ROM?

<details><summary>Đáp án</summary>

1. **ROM code** (khắc trong SoC): đọc **boot pins/eFuse** chọn nguồn boot; nạp SPL từ offset quy ước của nguồn đó vào **SRAM nội** (DRAM chưa init — đây là lý do không nạp thẳng U-Boot: U-Boot proper vài trăm KB–MB **không vừa SRAM**, mà DRAM thì chưa tồn tại với hệ thống).
2. **SPL** (vừa SRAM, vài chục KB): init **PMIC/clock tối thiểu + DRAM controller** (training); nạp U-Boot proper vào DRAM. (ARM64 secure: TF-A BL2 ở vai này, BL31 cài secure monitor/PSCI ở lại runtime.)
3. **U-Boot proper**: init console UART, storage, (network); đọc **env**; chạy `bootcmd`: nạp kernel Image + **DTB** (+initramfs) vào RAM, đặt `bootargs`, gọi `booti` — tắt cache/MMU theo yêu cầu ABI boot, nhảy vào kernel entry kèm **con trỏ DTB trong register** (x0 trên ARM64).
4. **Kernel**: giải nén (nếu có), early init (parse DTB, dựng bảng trang, bật MMU, init console sớm `earlycon`), khởi tạo subsystem + probe driver theo DT, mount **rootfs** theo `root=` (hoặc unpack initramfs), exec **init** (PID 1).
5. **Init** (BusyBox/systemd): mount /proc /sys, dịch vụ, getty trên console → **shell**.
- Điểm cộng: nêu chain of trust (mỗi tầng verify tầng sau — secure boot); biết mốc debug từng tầng (không có gì trên UART = chết trước SPL console; banner U-Boot mà không "Starting kernel" = sai load/bootargs; kernel log dừng ở VFS = sai root=).

</details>

**Câu 2:** Board mới bring-up không lên gì trên console. Trình tự debug của bạn?

<details><summary>Đáp án</summary>

Chia đôi theo chuỗi boot, từ sớm về muộn:
1. **Phần cứng trước**: nguồn/PMIC rail đúng thứ tự chưa (đo), reset released chưa, **boot strap pins** đúng nguồn boot chưa (lỗi số 1 khi board mới), quartz/clock chạy chưa; UART đúng chân TX/RX chưa (đảo cặp — lỗi số 2), đúng baud/level chưa.
2. **ROM code có chạy không**: nhiều SoC có **UART/USB boot mode** — ép boot mode đó, nếu tool vendor bắt tay được với ROM (vd `imx_usb`, `sunxi-fel`) → SoC sống, vấn đề ở nguồn boot/ảnh SPL.
3. **SPL**: nạp SPL có build **debug UART sớm** đúng chân; SPL banner ra → DRAM init là nghi phạm kế (sai timing DDR — so datasheet, chạy memory test của SPL).
4. **U-Boot**: banner ra thì đã dễ — env, `bootargs` (`console=`, `earlycon` để thấy kernel log sớm nhất), load address chồng lấn.
5. **Kernel im lặng sau "Starting kernel"**: 90% là `console=` sai hoặc thiếu clock/pinmux UART trong **DT** — bật `earlycon` (dùng UART do bootloader để lại, không cần driver) để lấy log giai đoạn mù.
6. Song song mọi bước: **JTAG** nếu có (attach xem PC đang ở đâu), LED/GPIO gõ nhịp làm "printf bằng chân".
- Cấu trúc trả lời "chia đôi chuỗi boot + công cụ từng đoạn" ăn điểm hơn liệt kê mẹo rời rạc.

</details>

---

## Cụm 2 — Build & cấu hình Kernel (ch. 4) 🎯

### Nội dung chính

**Kernel nhận gì từ bootloader:** device tree blob (DTB), command line (`bootargs`), (tùy chọn) initramfs — và **không nhận gì khác**: mọi hiểu biết về board đến từ DT + config.

**Cấu hình — Kconfig:**
- `make menuconfig` — chỉnh; kết quả là file `.config` (từng `CONFIG_*=y/m/n`).
- **defconfig**: cấu hình chuẩn cho dòng máy — `make defconfig` (x86), `make imx_v8_defconfig`...; ARM64 chủ trương **một Image chạy mọi board** (đa dạng nằm ở DTB), khác thời ARM32 mỗi board một config.
- `y` = built-in, `m` = **module** (.ko — nạp sau khi rootfs mount; driver cần trước lúc đó — storage, console — phải `y` hoặc bỏ vào initramfs).
- Build: `make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image dtbs modules` → `arch/arm64/boot/Image`, `*.dtb`, rồi `make modules_install INSTALL_MOD_PATH=<rootfs>` (vào `/lib/modules/$(uname -r)`).
- Ảnh kernel: `Image` (ARM64, raw), `zImage` (ARM32 nén tự giải), `uImage` (zImage + header U-Boot cũ), FIT `.itb` (hiện đại).
- **Kernel command line** đáng thuộc: `console=`, `earlycon`, `root=`, `rootwait`, `rootfstype=`, `init=`, `loglevel=`, `mem=`, `initcall_debug` (debug treo boot), `nfsroot=` (cụm rootfs).

**Kernel boot đến rootfs:** early setup (MMU, per-cpu, timer) → init các subsystem → **probe driver khớp DT** → mount root → `run_init_process("/sbin/init")`. Panic quen mặt: `VFS: Unable to mount root fs` (sai root=/thiếu driver/thiếu FS config) — đọc dòng "available partitions" ngay phía trên là ra manh mối.

### Insight đáng nhớ

- Mental model tuyển dụng hay kiểm tra: **"kernel giống nhau, board khác nhau = DTB khác nhau"** (ARM64) — tách *code* khỏi *mô tả phần cứng* là chính sách lớn của kernel hiện đại; hệ quả trực tiếp: sửa chân GPIO không cần rebuild kernel, chỉ rebuild DTB.
- `y` vs `m` không phải khẩu vị: nó là câu hỏi **thứ tự phụ thuộc lúc boot** (cần trước root mount thì built-in/initramfs) và chính sách vendor (module ký, tải động cho họ SKU).

### Ít quan trọng

- Kernel headers cho userspace (`make headers_install`), out-of-tree module build chi tiết Makefile (ch. 4 cuối) — tra khi làm; các target make phụ (`nconfig`, `xconfig`).

### Góc interview

**Câu 1:** Khác biệt giữa built-in (`y`) và module (`m`)? Driver eMMC controller trên board bạn nên là gì, vì sao?

<details><summary>Đáp án</summary>

- `y`: link thẳng vào Image — có mặt từ đầu boot, không gỡ được, tăng size ảnh. `m`: file `.ko` trong `/lib/modules/...` trên **rootfs** — nạp bằng udev/modprobe **sau khi rootfs đã mount**; gỡ/nạp lại được (tiện dev), thu nhỏ Image.
- Suy ra nghịch lý phải tránh: **driver của thiết bị chứa rootfs không thể là module nằm trên chính rootfs đó** — gà và trứng. eMMC controller (+ MMC core, block layer, FS của rootfs) khi root trên eMMC: **`y`**, hoặc `m` nhưng đặt trong **initramfs** (chính là cách distro desktop làm — initramfs chứa driver storage, unpack từ RAM trước khi cần rootfs thật).
- Tiêu chí chọn chung: cần-trước-root-mount → y/initramfs; phần cứng tùy chọn/cắm rút, nhiều biến thể SKU → m; sản phẩm khóa cấu hình, ít RAM → nghiêng y (bỏ hạ tầng module luôn — tiết kiệm và thu hẹp bề mặt tấn công).

</details>

---

## Cụm 3 — Device Tree (nửa sau ch. 3–4, gộp chủ đề) 🎯🎯

### Nội dung chính

**Device tree = mô tả phần cứng KHÔNG dò tìm được** (khác USB/PCIe tự enumerate): SoC bus nào có thiết bị gì tại địa chỉ nào, ngắt nào, clock nào. Nguồn `.dts`/`.dtsi` (include — `.dtsi` cho SoC, `.dts` cho board) → compile bằng `dtc` → `.dtb`.

```dts
/dts-v1/;
#include "imx8mm.dtsi"                 // mô tả SoC (vendor viết)
/ {
    model = "My Board";
    compatible = "myco,myboard", "fsl,imx8mm";
};
&uart1 {                               // node đã có trong .dtsi — reference sửa đè
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_uart1>;      // pinmux
    status = "okay";                   // .dtsi thường để "disabled" — board bật
};
&i2c1 {
    status = "okay";
    temp-sensor@48 {                   // thiết bị trên bus I2C, addr 0x48
        compatible = "ti,tmp102";      // ← chuỗi khớp driver
        reg = <0x48>;
        interrupt-parent = <&gpio3>;
        interrupts = <7 IRQ_TYPE_LEVEL_LOW>;
        vcc-supply = <&reg_3v3>;       // regulator
    };
};
```

Các property nền: `compatible` (danh sách "cụ thể → tổng quát"), `reg` (địa chỉ/độ dài — cách đọc theo `#address-cells/#size-cells` của node cha), `interrupts`, `clocks`, `status`, nhãn `&label`, `phandle` (tham chiếu chéo). **Bindings** (hợp đồng property của từng compatible) nằm ở `Documentation/devicetree/bindings/` — giờ là YAML schema, có `make dtbs_check`.

**Cơ chế khớp driver — trái tim của cụm:**

```
Kernel boot → parse DTB → tạo platform_device cho các node bus
Driver đăng ký platform_driver với of_match_table = {"ti,tmp102", ...}
        ▼
Khớp compatible → gọi probe(dev):
    đọc property (reg, interrupts, clocks...) qua API of_*/device property
    xin tài nguyên: devm_ioremap_resource, devm_clk_get, devm_request_irq...
        ▼
Tài nguyên CHƯA SẴN SÀNG (clock provider/regulator/GPIO expander chưa probe)?
    → return -EPROBE_DEFER → kernel xếp lại hàng, probe LẠI sau khi
      có driver mới probe xong — cơ chế giải bài toán THỨ TỰ phụ thuộc
```

- **Deferred probe** là câu trả lời cho "kernel làm sao biết probe cái gì trước": không cần biết — thử, thiếu thì hoãn, lặp tới hội tụ. Triệu chứng lỗi liên quan: thiết bị "mất tích" không lỗi rõ ràng → xem `/sys/kernel/debug/devices_deferred`.
- **Overlay (`.dtbo`)**: vá DT lúc runtime/boot — nền của cape/HAT (BeagleBone, RPi), cấu hình biến thể board không cần N file DTB đầy đủ.
- Nhìn DT thực tế đang chạy: `/proc/device-tree/` (hay `/sys/firmware/devicetree/`).

### Insight đáng nhớ

- Câu định vị: **"DT là dữ liệu, driver là code — compatible là khóa ngoại nối hai thế giới."** Sửa mô tả phần cứng không đụng code; một driver phục vụ N board.
- Ba lỗi DT chiếm đa số thực chiến bring-up: (1) `status` quên `"okay"`; (2) **pinmux sai/thiếu** (thiết bị probe OK mà tín hiệu không ra chân); (3) phụ thuộc chéo gây **EPROBE_DEFER vĩnh viễn** (supply/clock trỏ node bị disabled). Kể được bộ ba này = người từng làm thật.

### Ít quan trọng

- Cú pháp cells nâng cao (`interrupt-map`, `ranges` dịch địa chỉ qua bus) — tra spec khi gặp; lịch sử board file ARM32 trước DT (biết để trả lời "vì sao DT ra đời": nghìn file board `mach-*` copy-paste, Linus nổi giận 2011 — dẫn tới chuẩn hóa DT).

### Góc interview

**Câu 1 (🎯):** Device tree giải quyết vấn đề gì? Trình bày đường đi từ một node DT đến hàm `probe()` của driver.

<details><summary>Đáp án</summary>

- **Vấn đề:** phần cứng ARM SoC **không tự mô tả** (không như PCI/USB có enumeration) — trước DT, mỗi board một mớ **board file C** trong kernel (đăng ký tay từng device, địa chỉ, ngắt): nghìn file trùng lặp, sửa board phải rebuild kernel, không thể một ảnh nhiều board. DT tách **mô tả phần cứng thành dữ liệu** đi kèm boot: một kernel Image + N DTB.
- **Đường đi:** bootloader đặt DTB vào RAM, trao địa chỉ cho kernel (x0/r2) → early parse (chọn machine theo `compatible` gốc, memory, chosen/bootargs) → core dựng **platform_device** cho các node (bus simple-bus, node con I2C/SPI do controller driver tạo khi probe bus) → driver đăng ký **`of_match_table`** chứa các chuỗi compatible → match (so từ chuỗi cụ thể nhất) → **`probe(pdev)`** chạy: đọc `reg`→`devm_ioremap_resource`, `interrupts`→`devm_request_irq`, `clocks`→`clk_get/enable`, đọc property riêng qua `device_property_read_*` → đăng ký vào subsystem (char dev, iio, net...).
- Nêu thêm **EPROBE_DEFER**: tài nguyên chưa có → hoãn, kernel probe lại sau — thứ tự driver tự hội tụ, không hard-code.
- **Bẫy khi trả lời:** nói "kernel đọc DT rồi *nạp* driver" — DT không nạp gì; driver phải **có sẵn** (built-in/module do udev modalias nạp), DT chỉ là **dữ liệu khớp nối**; và quên rằng node dưới I2C/SPI do driver của bus controller tạo ra khi chính nó probe (đệ quy).

</details>

**Câu 2 (🎯):** Bạn thêm một sensor I2C mới lên board đang chạy. Liệt kê đầy đủ những gì phải làm, và các lỗi thường gặp.

<details><summary>Đáp án</summary>

1. **Kiểm tra driver tồn tại**: tìm compatible trong kernel (`grep -r "ti,tmp102" drivers/`), bật CONFIG tương ứng (y/m). Chưa có driver → viết (platform/i2c driver + bindings) hoặc dùng userspace (`i2c-dev`) tạm.
2. **Sửa DT board**: trong node `&i2cN` (bảo đảm `status = "okay"` + pinctrl đúng), thêm node con: `compatible`, `reg = <addr I2C>`, ngắt/GPIO/supply nếu sensor dùng — **đúng bindings** (đọc YAML trong Documentation, chạy `make dtbs_check`).
3. **Build & deploy DTB** (chỉ `make dtbs` — không cần rebuild kernel), hoặc **overlay .dtbo** nếu hệ hỗ trợ.
4. **Kiểm chứng theo tầng**: `i2cdetect -y N` thấy địa chỉ chưa (thấy = phần cứng + bus OK, chưa cần driver); dmesg lúc boot có probe không; `/sys/bus/i2c/devices/N-0048/` xuất hiện chưa; giá trị đọc được qua sysfs/iio.
- **Lỗi thường gặp:** sai địa chỉ I2C (strap pin đổi addr — đọc datasheet, i2cdetect đối chứng); quên pinmux cho bus (SDA/SCL không ra chân — bus timeout); pull-up thiếu (phần cứng); driver bị `m` mà thiếu module trên rootfs; **EPROBE_DEFER treo vĩnh viễn** vì `vcc-supply` trỏ regulator bị disabled; trùng địa chỉ với thiết bị khác trên bus; và i2cdetect thấy nhưng driver không bind — compatible gõ sai một ký tự (khớp chuỗi tuyệt đối).
- Trả lời phân tầng "phần cứng → bus → DT → driver → sysfs" là format điểm cao.

</details>

**Câu 3:** `bootargs` của bạn có `console=ttyS0` nhưng kernel im lặng dù U-Boot in bình thường trên đúng UART đó. Các nguyên nhân khả dĩ?

<details><summary>Đáp án</summary>

U-Boot in được = phần cứng UART sống — vấn đề nằm ở phía kernel:
1. **Tên device sai theo driver**: driver serial của SoC có thể đặt `ttymxc0` (i.MX), `ttyAMA0` (PL011), `ttyS0` (8250) — `console=` phải khớp tên + số port đúng của kernel, không phải của U-Boot.
2. **Driver serial/console không được bật** trong config (`CONFIG_SERIAL_..._CONSOLE=y`), hoặc là module (console phải built-in).
3. **DT**: node UART bị `status = "disabled"`, thiếu pinctrl/clock — driver không probe (U-Boot chạy được vì nó tự cấu hình, rồi kernel *reset lại* theo DT của mình).
4. Baudrate khác (`console=ttymxc0,115200`), hoặc console bị trỏ đi nơi khác qua `stdout-path` trong `/chosen`.
- Công cụ chẩn đoán số 1: thêm **`earlycon`** — console siêu sớm dùng UART theo trạng thái bootloader để lại (không cần driver probe) → nhìn được kernel log giai đoạn "mù" và thấy nó chết ở đâu (thường lộ ngay lỗi probe UART thật).

</details>

### Đọc thêm (tùy chọn)

- [05/device-tree.md](../../05-drivers-device-tree/device-tree.md), [05/driver-basics.md](../../05-drivers-device-tree/driver-basics.md) — nền DT/driver của repo.
- [08/boot-process.md](../../08-embedded-systems/boot-process.md) — boot process bản cô đọng.
- [OSTEP virtualization-cpu](../ostep/virtualization-cpu.md) — LDE/trap: chuyện xảy ra sau khi kernel nắm quyền.
