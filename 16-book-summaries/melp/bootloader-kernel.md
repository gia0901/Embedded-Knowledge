# MELP — Bootloader, Device Tree & Kernel (ch. 3 tr. 41, ch. 4 tr. 69) 🎯🎯

> Thuộc [MELP](README.md). **Nguồn summary: ĐÃ ĐỐI CHIẾU SÂU với PDF** `Mastering Embedded Linux Programming.pdf` — **1st edition (2015)**; mọi số trang `(tr. X)` theo đúng bản PDF này.
> **Cách đọc các ký hiệu:** nội dung không đánh dấu = có trong sách. **🆕 = bổ sung ngoài sách** (kiến thức hiện đại sách 2015 chưa có — cần cho phỏng vấn 2026). **⚠️ = sách đã lỗi thời ở điểm đó.**
> Đây là file quan trọng nhất với vị trí BSP — boot flow và device tree xuất hiện trong gần như mọi buổi phỏng vấn.

---

## Cụm 1 — Bootloader làm gì, và vì sao boot phải chia 3 pha (ch. 3, tr. 41–46)

### Nội dung chính

**Bootloader có đúng 2 nhiệm vụ (tr. 41):** (1) **khởi tạo hệ thống ở mức tối thiểu** và (2) **nạp kernel**. Sách nhấn mạnh: nhiệm vụ (1) chỉ là phục vụ cho (2) — bootloader không cần dựng *cả* hệ thống, chỉ cần dựng *đủ để nạp được kernel* (RAM + thiết bị chứa kernel + console). Nhiệm vụ phụ (tr. 42): làm **chế độ bảo trì** — sửa cấu hình boot, nạp ảnh mới, chẩn đoán — qua giao diện dòng lệnh trên cổng serial.

**Xuất phát điểm để hiểu mọi thứ: lúc vừa cấp nguồn, hệ thống gần như "mù".** DRAM controller chưa được cấu hình → **RAM chính chưa dùng được**; NAND/MMC controller chưa init → **storage chưa đọc được**. Tài nguyên duy nhất: một CPU core + **SRAM nội** (on-chip static RAM) + ROM. Vì vậy boot phải là chuỗi nhiều pha, **mỗi pha "mở khóa" thêm tài nguyên cho pha sau** (tr. 41–42).

*(Bối cảnh lịch sử để hiểu vì sao — tr. 42: thời xưa dùng NOR flash, chip map thẳng vào address space, CPU thực thi lệnh trực tiếp trên flash (XIP — execute in place) từ reset vector 0xfffffffc → chỉ cần MỘT pha. Rời khỏi NOR (NAND/eMMC không map thẳng được) thì bắt buộc phải nhiều pha.)*

**Ba pha — thuật ngữ theo sách (tr. 43–45):**

```
Pha 1: ROM CODE (tr. 43)                       chạy từ: ROM trong SoC
  Khắc cứng trong chip khi sản xuất — không thay được, thường độc quyền.
  Không biết gì về phần cứng NGOÀI chip (kể cả loại DRAM!) 
  → chỉ dùng được SRAM nội (4KiB đến vài trăm KiB tùy SoC).
  Việc: đọc boot pins/cấu hình → nạp SPL từ vị trí quy ước vào SRAM.
  Ví dụ TI OMAP/Sitara: thử các trang đầu NAND → SPI flash → sector đầu
  MMC/eMMC → file tên "MLO" trên partition FAT đầu tiên của thẻ SD
  → cuối cùng thử nhận qua Ethernet/USB/UART (đường nạp lúc sản xuất).
        ▼
Pha 2: SPL — Secondary Program Loader (tr. 44)  chạy từ: SRAM
  Bị giới hạn kích thước bởi SRAM → chức năng tối thiểu, thường không
  tương tác. Việc CHÍNH: cấu hình memory controller (dựng DRAM!) 
  + nạp pha 3 vào DRAM (đọc từ offset quy ước, hoặc file "u-boot.bin"/
  "u-boot.img"). Có thể là mã nguồn mở (TI x-loader, AT91Bootstrap)
  hoặc blob độc quyền của hãng.
        ▼
Pha 3: TPL — Tertiary Program Loader (tr. 45)   chạy từ: DRAM
  Bootloader đầy đủ (U-Boot/Barebox): có dòng lệnh, env, script.
  Việc: nạp kernel (+ DTB, + initramdisk tùy chọn) vào DRAM → nhảy vào.
  Sau khi kernel chạy, bootloader BIẾN MẤT — RAM nó chiếm được thu hồi,
  nó không còn vai trò gì trong hệ đang chạy (tr. 45).
```

⚠️ **Lưu ý thuật ngữ:** sách gọi U-Boot đầy đủ là "TPL". Trong mã nguồn U-Boot hiện đại, "TPL" lại nghĩa là *Tiny* Program Loader — một pha **trước cả SPL** cho SoC có SRAM cực nhỏ (chuỗi: ROM → TPL → SPL → U-Boot proper). Đi phỏng vấn cứ dùng "SPL → U-Boot proper" là an toàn nhất.

**UEFI (tr. 46):** thiết bị embedded gốc PC (và một số ARM server) dùng chuẩn UEFI thay chuỗi trên, nhưng **bản chất y hệt 3 pha**: firmware/boot manager (≈SPL) → nạp bootloader từ **ESP** (EFI System Partition — partition FAT32, file `/efi/boot/bootx64.efi`) → GRUB2 hoặc systemd-boot (sách gọi tên cũ "gummiboot") nạp kernel.

**🆕 Bổ sung ngoài sách — secure boot hiện đại (ARM64):** sách 2015 chưa cover. Chuỗi hiện đại chuẩn hóa bằng **TF-A (Trusted Firmware-A)**: `BL1 (ROM) → BL2 (≈SPL) → BL31 (secure monitor ở EL3 — ở lại runtime, cung cấp PSCI: bật/tắt core, suspend) → BL33 (U-Boot) → kernel`; mỗi pha **verify chữ ký** pha sau — **chain of trust** bắt rễ từ khóa ghi trong eFuse/OTP. Từ khóa này gần như chắc chắn xuất hiện trong JD BSP hiện nay.

### Insight đáng nhớ

- Câu gói cả cụm: **"chuỗi boot là chuỗi bootstrap tài nguyên — ROM chỉ biết đọc thiết bị, SPL chỉ biết dựng DRAM, U-Boot biết phần còn lại; mỗi tầng chỉ đủ khả năng dựng tầng kế."** Trả lời "vì sao cần SPL" bằng một câu: *vì U-Boot đầy đủ không vừa SRAM, mà DRAM thì phải có code chạy trước để dựng lên* (tr. 43–44).
- Chi tiết "file MLO trên partition FAT" (tr. 43) đáng nhớ vì nó lý giải trải nghiệm thực tế: vì sao chỉ cần chép 2 file (MLO + u-boot.img) vào thẻ SD FAT là board TI boot được — ROM code biết đọc FAT ở mức tối thiểu.

### Góc interview

**Câu 1 (🎯 mở màn kinh điển):** Kể chi tiết quá trình từ lúc cấp nguồn đến khi có shell, trên board ARM chạy Linux. Vì sao không nạp thẳng U-Boot từ ROM?

<details><summary>Đáp án</summary>

1. **ROM code** (trong SoC, tr. 43): đọc **boot pins/eFuse** để biết boot từ đâu; nạp **SPL** từ vị trí quy ước (trang đầu NAND / sector đầu eMMC / file MLO trên FAT / SPI flash) vào **SRAM nội** — vì DRAM chưa được init, và đây chính là câu trả lời cho vế hai: **U-Boot đầy đủ (vài trăm KB–MB) không vừa SRAM (vài chục KB), còn DRAM thì "chưa tồn tại"** với hệ thống — phải có code dựng DRAM trước, code đó phải đủ nhỏ để vừa SRAM → đó là SPL.
2. **SPL** (tr. 44): cấu hình **DRAM controller** (+ clock/PMIC tối thiểu) → nạp **U-Boot proper** vào DRAM → nhảy sang.
3. **U-Boot** (tr. 45, 56–60): init console/storage/network; đọc **environment**; hết `bootdelay` thì chạy `bootcmd`: nạp kernel + **DTB** (+ ramdisk nếu có) vào các địa chỉ RAM đã định, đặt `bootargs`, gọi `bootz/bootm` — trao con trỏ DTB cho kernel rồi biến mất.
4. **Kernel**: giải nén, init, parse DTB, probe driver, mount **rootfs** theo `root=` trong bootargs (tr. 88) → chạy **init** (PID 1, thử lần lượt `/sbin/init`, `/etc/init`, `/bin/init`, `/bin/sh` — tr. 88).
5. **Init** (BusyBox/systemd — ch. 9): mount /proc /sys, khởi động service, getty trên console → **shell**.
- 🆕 Điểm cộng hiện đại: nhắc chuỗi TF-A BL1→BL2→BL31→BL33 và chain of trust (verify chữ ký từng pha) nếu JD có secure boot; và biết mốc debug từng pha (không gì trên UART = chết trước SPL; banner U-Boot mà không "Starting kernel" = lỗi nạp/bootargs; dừng ở "VFS: Unable to mount root fs" = lỗi root=).

</details>

---

## Cụm 2 — Trao quyền cho kernel & Device Tree (ch. 3, tr. 46–52)

### Nội dung chính

**Bootloader trao gì cho kernel (tr. 46–47):** khi nhảy vào kernel, bootloader phải truyền: số định danh loại SoC (machine number — cơ chế cũ), thông tin phần cứng cơ bản (ít nhất: vị trí + kích thước RAM), **kernel command line** (chuỗi ASCII điều khiển Linux — vd `root=` chỉ đâu là rootfs), và tùy chọn: vị trí **DTB**, vị trí initramfs. *Lịch sử để hiểu vì sao có DT (tr. 47):* trước kia ARM truyền "A-tags" — một danh sách thông tin rất ít ỏi, phần còn lại phải **hard-code trong kernel** ("platform data") → **mỗi board phải build một kernel riêng đã sửa code**. Cách tốt hơn là mô tả phần cứng bằng dữ liệu: **device tree** — ARM chuyển mạnh từ Linux 3.8 (2/2013).

**Device tree là gì (tr. 47–48):** cách **định nghĩa các thành phần phần cứng của hệ thống bằng dữ liệu** — không phải code. Nguồn gốc: bootloader OpenBoot của Sun → chuẩn Open Firmware (IEEE 1275) → PowerPC Mac → ARM Linux. Bootloader nạp DTB và trao cho kernel (cũng có thể gộp DTB vào ảnh kernel cho bootloader cũ). Nguồn `.dts` là **văn bản: cây các node, mỗi node chứa property dạng `tên = "giá trị"`**, compile ra nhị phân `.dtb` (device tree blob / FDT — flattened device tree).

```dts
/dts-v1/;
/ {                                    // node gốc "/"
    model = "TI AM335x BeagleBone";
    compatible = "ti,am33xx";          // chuỗi định danh — xem bên dưới
    #address-cells = <1>;              // quy ước đọc 'reg' của node con: 
    #size-cells = <1>;                 //   địa chỉ 1 cell (32-bit), size 1 cell
    cpus { ... cpu@0 { compatible = "arm,cortex-a8"; reg = <0>; }; };
    memory@0x80000000 {
        device_type = "memory";
        reg = <0x80000000 0x20000000>; // RAM bắt đầu 0x80000000, dài 0x20000000 (512MB)
    };
};
```

Bốn khái niệm nền — sách giải thích rất tuần tự (tr. 48–50), tóm lại:

1. **`compatible`** (tr. 49): "khóa" để kernel tìm driver — driver khai danh sách chuỗi của mình trong `struct of_device_id`, kernel **so chuỗi** compatible của node với danh sách đó để bind. Quy ước giá trị: `"hãng,tên-thiết-bị"` (`ti,am33xx`). Node có thể liệt kê **nhiều** compatible từ cụ thể → tổng quát (fallback: driver riêng chưa có thì driver họ chung nhận).
2. **`reg`** (tr. 49): dải địa chỉ của thiết bị = cặp (địa chỉ bắt đầu, độ dài), viết bằng các số 32-bit gọi là **cell**. Bao nhiêu cell cho mỗi phần do node **cha** khai báo qua `#address-cells` / `#size-cells` — ví dụ hệ 64-bit cần 2 cell cho địa chỉ: `#address-cells = <2>` → `reg = <0x00000000 0x80000000  0 0x80000000>`. Đọc `reg` là phải **ngược lên cây tìm hai khai báo này**. (CPU cũng có "địa chỉ": số hiệu core — cpus khai `#address-cells=<1>, #size-cells=<0>` nên `cpu@0 { reg = <0>; }`.)
3. **phandle & interrupts** (tr. 50): cây chỉ mô tả được **một** quan hệ cha-con, nhưng thiết bị còn nối với interrupt controller, clock, regulator... — các quan hệ "chéo cây" này biểu diễn bằng **phandle**: node đích có nhãn (`intc:`), node dùng tham chiếu `<&intc>`:

```dts
intc: interrupt-controller@48200000 {
    compatible = "ti,am33xx-intc";
    interrupt-controller;
    #interrupt-cells = <1>;            // mô tả 1 ngắt cần mấy cell (ở đây: 1 = số IRQ;
};                                     //  nhiều hệ dùng 2-3 cell: số IRQ + kiểu edge/level)
serial@44e09000 {
    compatible = "ti,omap3-uart";
    reg = <0x44e09000 0x2000>;
    interrupt-parent = <&intc>;        // ngắt của tôi nối vào controller NÀY (phandle)
    interrupts = <72>;                 // ... tại đường số 72
    clock-frequency = <48000000>;      // property riêng theo BINDINGS của driver này
};
```

4. **Bindings** (tr. 50): mỗi loại compatible có "hợp đồng" quy định node phải/được có property nào — nằm tại `Documentation/devicetree/bindings/` trong nguồn kernel. Property lạ (vd `ti,hwmods`) là đọc bindings mà tra.

**Include & layering — cách DT thật được tổ chức (tr. 51–52):** phần cứng chung của cả họ SoC tách vào **`.dtsi`**; file board `.dts` include nó. DT nguồn được chạy qua **C preprocessor** (`#include`, `#define` dùng được — macro trong `include/dt-bindings/`). Quan trọng nhất là cơ chế **chồng lớp (overlay node)**: file include sau **đè/bổ sung** lên node đã có, tham chiếu qua nhãn `&mmc1`. Ví dụ xương sống của sách — một thiết bị đi qua 3 tầng file:

```
am33xx.dtsi          (mọi SoC am33xx):  mmc1: mmc@48060000 { compatible = "ti,omap4-hsmmc";
                                          ...; status = "disabled"; }   ← khai đủ, nhưng TẮT
am335x-bone-common.dtsi (mọi BeagleBone): &mmc1 { status = "okay";      ← board có MMC1 thật → BẬT
                                          pinctrl-0 = <&mmc1_pins>;     ← nối pinmux
                                          cd-gpios = <&gpio0 6 ...>; }  ← chân card-detect
am335x-boneblack.dts    (riêng Black):    &mmc1 { vmmc-supply = <&vmmcsd_fixed>; }  ← nối nguồn
```

→ Trả lời được câu "vì sao trong .dtsi thiết bị nào cũng `status = "disabled"`": *SoC có sẵn controller đó, nhưng board có nối nó ra ngoài không thì chỉ file board biết — board nào dùng thì tự bật.*

**Compile (tr. 52):** `dtc` biên dịch `.dts` (đơn giản) → `.dtb`; ⚠️ sách cảnh báo dtc thời đó **báo lỗi rất kém, chỉ check cú pháp** — 🆕 nay đã có schema YAML + `make dtbs_check` bắt được lỗi bindings. File dùng `#include` thì build qua kbuild của kernel: `make ARCH=arm dtbs` (tr. 84).

**🆕 Bổ sung ngoài sách — những mảnh DT phỏng vấn hay hỏi mà 1st ed chưa có:**
- **Đường đi node → probe:** kernel parse DTB → tạo `platform_device` cho các node (node con trên bus I2C/SPI do driver bus controller tạo khi chính nó probe) → match compatible với `of_match_table` của driver → gọi **`probe()`** — trong probe driver đọc `reg`/`interrupts`/`clocks` qua API `of_*`/`platform_get_*`.
- **EPROBE_DEFER:** probe cần tài nguyên mà driver cung cấp chưa sẵn (clock, regulator, GPIO expander) → trả `-EPROBE_DEFER`, kernel **xếp lại và probe lại sau** — thứ tự driver tự hội tụ, không cần hard-code. Thiết bị "mất tích" không lỗi rõ → xem `/sys/kernel/debug/devices_deferred`.
- **pinctrl:** chân SoC đa năng (mux); nhóm `pinctrl-0 = <&...>` được driver core **tự áp khi probe**. Ba lỗi DT thực chiến phổ biến nhất: quên `status="okay"`, **pinmux sai/thiếu** (probe OK mà tín hiệu không ra chân!), phụ thuộc chéo gây EPROBE_DEFER treo.
- **Overlay runtime (`.dtbo`)** cho cape/HAT; xem DT hệ đang chạy tại `/proc/device-tree/`.

### Insight đáng nhớ

- **"DT là dữ liệu, driver là code — `compatible` là khóa ngoại nối hai thế giới."** Toàn bộ giá trị của DT nằm ở việc tách *mô tả board* khỏi *kernel*: sửa chân GPIO chỉ cần build lại DTB vài giây, không đụng kernel (so với thời platform data: sửa code + build cả kernel — tr. 47).
- Cơ chế 3 tầng `.dtsi` → common → board (tr. 51–52) chính là "kế thừa" phiên bản dữ liệu: SoC khai đầy đủ nhưng disabled, board bật và nối dây (pinctrl, supply, GPIO). Hiểu tầng nào khai gì là đọc được mọi DT thật.

### Góc interview

**Câu 1 (🎯):** Device tree giải quyết vấn đề gì? Trình bày đường đi từ một node DT đến hàm `probe()` của driver.

<details><summary>Đáp án</summary>

- **Vấn đề (tr. 47):** phần cứng trên bus SoC **không tự khai báo** (khác USB/PCI có enumeration). Trước DT, ARM truyền A-tags (quá ít thông tin) + phần còn lại **hard-code trong kernel** (platform data) → mỗi board một kernel bị sửa code riêng, nghìn board file trùng lặp. DT biến mô tả phần cứng thành **dữ liệu** đi kèm boot → một kernel + N file DTB.
- **Đường đi:** U-Boot nạp DTB vào RAM, trao địa chỉ cho kernel lúc `bootz/bootm` → kernel early parse (nhận diện machine qua `compatible` của node gốc, lấy memory, cmdline từ `/chosen`) → dựng **platform_device** cho các node có compatible (node treo dưới I2C/SPI do driver của bus controller tạo khi nó probe — đệ quy) → driver đăng ký danh sách compatible trong **`of_device_id`/`of_match_table`** (tr. 49) → match (so chuỗi, ưu tiên chuỗi cụ thể nhất trong danh sách fallback) → **`probe(pdev)`**: đọc `reg` → ioremap, `interrupts` → request_irq, property riêng theo **bindings** (tr. 50) → đăng ký vào subsystem.
- 🆕 Nêu thêm **EPROBE_DEFER** (thiếu clock/regulator → hoãn, kernel thử lại sau khi có driver mới probe xong — thứ tự tự hội tụ) là điểm cộng lớn.
- **Bẫy khi trả lời:** nói "kernel đọc DT rồi *nạp* driver" — DT không nạp gì cả; driver phải có sẵn (built-in, hoặc module được udev nạp theo alias). DT chỉ là **dữ liệu khớp nối**.

</details>

**Câu 2 (🎯):** Thêm một sensor I2C mới lên board đang chạy: các bước và các lỗi thường gặp?

<details><summary>Đáp án</summary>

1. **Driver có chưa:** tìm compatible trong kernel (`grep -r "ti,tmp102" drivers/`), bật CONFIG (y/m). Chưa có → viết driver hoặc tạm dùng userspace (`i2c-dev`).
2. **Sửa DT board** (đúng mô hình 3 tầng tr. 51–52): trong node `&i2cN` (kiểm tra nó đã `status="okay"` + pinctrl đúng chưa), thêm node con: `compatible = "hãng,chip"`, `reg = <địa chỉ I2C>`, thêm `interrupts`/`vcc-supply` nếu sensor dùng — property đúng theo **bindings** trong `Documentation/devicetree/bindings/` (tr. 50); 🆕 chạy `make dtbs_check`.
3. **Build chỉ DTB** — `make ARCH=arm dtbs` (tr. 84), vài giây, không cần build kernel; deploy DTB (hoặc 🆕 overlay .dtbo nếu hệ hỗ trợ).
4. **Kiểm chứng theo tầng:** `i2cdetect -y N` thấy địa chỉ chưa (thấy = dây + bus OK, chưa liên quan driver) → dmesg có probe không → node trong `/sys/bus/i2c/devices/N-00xx/` → đọc giá trị.
- **Lỗi thường gặp:** sai địa chỉ I2C (chân strap đổi address — đối chiếu datasheet với i2cdetect); **quên pinmux** SDA/SCL (bus timeout); thiếu pull-up (phần cứng); driver để `m` mà module không có trên rootfs; compatible gõ sai một ký tự (match là **so chuỗi tuyệt đối** — tr. 49); 🆕 `vcc-supply` trỏ regulator đang disabled → EPROBE_DEFER treo vĩnh viễn; i2cdetect thấy mà driver không bind → gần như chắc lỗi compatible/CONFIG.
- Trả lời phân tầng "phần cứng → bus → DT → driver → sysfs" là format điểm cao.

</details>

---

## Cụm 3 — U-Boot thực hành: build, env, nạp ảnh, boot, porting (ch. 3, tr. 53–68)

### Nội dung chính

**Chọn bootloader (tr. 53):** bảng của sách: **U-Boot** (ARM/MIPS/PowerPC/SH — phổ biến nhất, cộng đồng lớn), Barebox, GRUB2 (x86), RedBoot, CFE, YAMON. Sách khuyên: nhận bootloader từ vendor thì **hỏi rõ nguồn code, chính sách update** — và cân nhắc thay bằng bản mở chính chủ.

**Build U-Boot (tr. 54):** quy trình y hệt kernel:

```bash
git clone git://git.denx.de/u-boot.git ; cd u-boot ; git checkout v2015.07
# tìm cấu hình board: thư mục configs/ có >1000 file _defconfig; đọc board/<vendor>/<board>/README
make CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- am335x_boneblack_defconfig
make CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf-
```

Kết quả (tr. 54–55): `u-boot` (ELF — để debug), `u-boot.bin` (nhị phân thô), `u-boot.img` (bin + header U-Boot — để chính U-Boot đang chạy nạp), `u-boot.srec` (format truyền qua serial), và **`MLO`** — chính là **SPL** cho board TI, build cùng lúc.

**Cài lần đầu (tr. 55–56):** board "trắng" cần trợ giúp ngoài: **JTAG** nạp U-Boot vào RAM chạy trực tiếp, hoặc tận dụng **ROM code đọc SD**: format thẻ partition 1 FAT, chép `MLO` + `u-boot.img` vào, giữ nút boot switch khi cấp nguồn (BeagleBone Black) → prompt `U-Boot#` trên serial 115200.

**Dùng U-Boot (tr. 56–60)** — các khối lệnh phải thuộc:

- **Quy ước số:** mọi số mặc định **hex**. `nand read 82000000 400000 200000` = đọc 0x200000 byte từ offset 0x400000 của NAND vào RAM 0x82000000 (tr. 57).
- **Environment (tr. 57):** bộ biến `name=value` — `setenv foo bar` (không có dấu `=`!), `printenv`, `setenv foo` (xóa), **`saveenv`** ghi xuống nơi lưu trữ. Nơi lưu tùy board: một **erase block riêng của NAND/NOR** (thường kèm **bản dự phòng thứ hai chống hỏng khi mất điện giữa lúc ghi**), file trên partition FAT của eMMC/SD, EEPROM, NVRAM. Giá trị khởi tạo nhúng trong header board (`CONFIG_EXTRA_ENV_SETTINGS`).
- **Boot image format (tr. 57–58):** U-Boot cổ điển không có filesystem — nó nhận diện ảnh qua **header 64 byte** do lệnh `mkimage` gắn: `mkimage -A arm -O linux -T kernel -C gzip -a 0x80008000 -e 0x80008000 -n 'Linux' -d zImage uImage` (-a: load address, -e: entry point) → ra **uImage**. 🆕 *(FIT image — gói kernel+DTB+ramdisk có chữ ký, `mkimage -f file.its` — sách chỉ nhắc một dòng; nay là chuẩn cho secure boot, đáng tự tìm hiểu thêm.)*
- **Nạp ảnh vào RAM (tr. 58–59):** từ SD/eMMC: `mmc rescan` → `fatload mmc 0:1 82000000 uImage` (thiết bị 0, partition 1) → `iminfo 82000000` (xem header). Qua mạng: **TFTP** — `setenv ipaddr 192.168.159.42; setenv serverip 192.168.159.99; tftp 82000000 uImage`. Ghi vào NAND: `nandecc hw; nand erase 280000 400000; nand write 82000000 280000 400000`; đọc lại: `nand read`.
- **Boot (tr. 60):** **`bootm [addr kernel] [addr ramdisk] [addr dtb]`** — không có ramdisk thì thay bằng **dấu gạch**: `bootm 82000000 - 83000000`. (Kernel zImage đời mới dùng `bootz` — ví dụ đầy đủ ở cụm 5.)
- **Tự động hóa (tr. 61):** biến đặc biệt **`bootcmd`** chạy sau `bootdelay` giây (đếm ngược trên console, bấm phím để vào chế độ tương tác): `setenv bootcmd nand read 82000000 400000 200000\;bootm 82000000` (dấu `;` phải escape).

**Porting U-Boot sang board mới (tr. 61–65)** — sách làm ví dụ board "Nova" chế từ BeagleBone Black; khung việc:

1. **Bố cục nguồn (tr. 61–62):** `arch/` (code theo kiến trúc — arch/arm/cpu/armv7...), `board/<vendor>/<board>/` (code riêng board), `common/` (shell + lệnh), `doc/`, `include/configs/` (**header cấu hình board — nơi chứa phần lớn cấu hình thời đó**).
2. Từ **v2014.10 U-Boot dùng Kconfig/Kbuild như kernel** (tr. 62); một build sinh tối đa 3 binary (U-Boot thường / SPL / TPL) nên dòng config có tiền tố phạm vi: `S:` chỉ SPL, `+S:` cả hai...
3. **Các file phải tạo:** `configs/nova_defconfig` (CONFIG_SPL=y, CONFIG_TARGET_NOVA..., tr. 62–63); `board/nova/` gồm Kconfig (SYS_CPU="armv7", SYS_BOARD="nova", SYS_SOC="am33xx", SYS_CONFIG_NAME="nova"), MAINTAINERS, Makefile, source board + linker script `u-boot.lds` (tr. 63–64); header `include/configs/nova.h` (copy từ board gần nhất rồi sửa — env size, prompt...; tr. 64–65). Build: `make nova_defconfig && make` → chép MLO + u-boot.img.
   ⚠️ *Chi tiết file-by-file đã đổi nhiều theo thời gian (Kconfig hóa xong từ lâu, DM driver model...) — nhớ **khung**: defconfig + thư mục board + header cấu hình, đừng nhớ từng dòng.*
4. **Falcon mode (tr. 65–66):** chế độ boot tắt — **SPL nạp thẳng kernel, bỏ qua U-Boot proper** (không tương tác, không script, đọc kernel từ vị trí cố định + tham số soạn sẵn) → giảm số pha, tăng tốc boot. Tên đặt theo chim cắt — loài chim nhanh nhất. Dùng khi yêu cầu boot nhanh (camera, automotive).

**Barebox (tr. 66–68):** "U-Boot v2" tách ra — trộn ý tưởng U-Boot với Linux (API kiểu POSIX, filesystem mount được). Sạch hơn nhưng hệ sinh thái nhỏ hơn nhiều — biết tên là đủ.

### Insight đáng nhớ

- `bootargs` + `bootcmd` là **hai biến env quan trọng nhất** — một cái là "hợp đồng với kernel" (cụm 5), một cái là "kịch bản tự chạy". Debug boot = đọc kỹ hai biến này trước tiên (`printenv`).
- Env có **bản dự phòng redundant** khi lưu trên NAND (tr. 57) — chi tiết nhỏ nhưng chính là bài "ghi atomic chống mất điện" ([storage-update.md](storage-update.md)) xuất hiện lần đầu, ở ngay bootloader.
- Porting U-Boot/kernel cho board "chế từ board mẫu" (kiểu Nova) là **việc thật phổ biến nhất của BSP junior/middle**: copy cấu hình board gốc → đổi tên → sửa dần khác biệt. Kể được quy trình này bằng kinh nghiệm là rất ăn điểm.

### Góc interview

**Câu 1:** U-Boot environment là gì, lưu ở đâu, và điều gì xảy ra nếu mất điện đúng lúc `saveenv`?

<details><summary>Đáp án</summary>

- Env = tập biến `name=value` điều khiển U-Boot (tr. 57): `bootcmd` (kịch bản boot tự động sau `bootdelay`), `bootargs` (cmdline trao kernel), `ipaddr/serverip` (TFTP), địa chỉ nạp... Lệnh: `printenv/setenv/saveenv`; giá trị mặc định nhúng trong build (`CONFIG_EXTRA_ENV_SETTINGS`).
- Nơi lưu (tr. 57): erase block dành riêng trên NOR/NAND, file (`uboot.env`) trên partition FAT của eMMC/SD, EEPROM I2C/SPI, NVRAM — vị trí/offset khai khi build (env phải đọc được từ rất sớm).
- Mất điện lúc saveenv: erase-rồi-ghi trên flash **không atomic** — vùng env có thể dở dang (CRC fail). Vì thế U-Boot hỗ trợ **redundant environment**: hai bản luân phiên + cờ/CRC — ghi bản mới xong mới coi là hiện hành, hỏng bản nào dùng bản kia; không bật redundant thì env hỏng → U-Boot rơi về **default env trong binary** (thiết bị vẫn boot được nếu default đủ dùng — một lý do nên giữ default env "boot được sản phẩm", đừng chỉ dựa env đã save).
- Liên hệ ăn điểm: đây là phiên bản mini của bài toán A/B update ([storage-update.md](storage-update.md)) — cùng nguyên tắc "hai bản + commit atomic".

</details>

**Câu 2 (🏗️):** Board mới bring-up không lên gì trên console — trình tự debug?

<details><summary>Đáp án (khung "chia đôi chuỗi boot")</summary>

1. **Phần cứng trước:** rail nguồn/PMIC đúng trình tự chưa (đo), reset released, **boot pins đúng nguồn boot** (lỗi #1 board mới), clock/quartz chạy; UART đúng chân TX/RX (đảo cặp là lỗi #2), đúng baud.
2. **ROM code sống không:** ép boot mode UART/USB (tr. 43 — ROM có đường nạp qua UART/USB cho sản xuất), tool vendor bắt tay được (imx_usb, sunxi-fel...) → SoC sống, lỗi nằm ở nguồn boot/ảnh SPL (offset sai? MLO đúng partition FAT đầu chưa? — tr. 43, 55).
3. **SPL:** banner SPL có ra không; ra rồi chết → nghi **DRAM init** (timing DDR — chạy mem test, so datasheet).
4. **U-Boot proper:** banner ra là dễ rồi — kiểm tra env (`printenv bootcmd bootargs`), địa chỉ nạp có đè nhau không (tr. 59: vùng nạp uImage phải tránh vùng kernel giải nén tới).
5. **"Starting kernel ..." rồi im:** 90% do `console=` sai trong bootargs (tr. 86: *"without this, we would not see any messages after Starting kernel"* — nguyên văn sách!) hoặc UART thiếu clock/pinmux trong **DT**; 🆕 thêm `earlycon` vào bootargs để thấy log kernel giai đoạn sớm (sách chưa có tham số này).
6. Song song mọi bước: JTAG (attach xem PC ở đâu), GPIO/LED làm "printf bằng chân".
- Điểm chấm: cấu trúc **chia đôi theo pha boot** + công cụ đúng từng pha, không phải liệt kê mẹo rời rạc.

</details>

---

## Cụm 4 — Kernel: chọn nguồn, Kconfig, build (ch. 4, tr. 69–85)

### Nội dung chính

**Kernel làm đúng 3 việc (tr. 70):** quản lý tài nguyên — giao tiếp phần cứng — cung cấp API trừu tượng cho userspace (sơ đồ: Application → C library → syscall handler → generic services → device drivers → hardware). App ở user space chạy **mức đặc quyền thấp**, làm gì "có ích" cũng phải qua kernel; C library dịch hàm POSIX thành syscall (trap/software interrupt chuyển CPU sang kernel mode). Interrupt **chỉ được xử lý trong driver**, không bao giờ ở userspace (tr. 71). *(Chi tiết bản chất đã có ở [OSTEP](../ostep/virtualization-cpu.md) — sách này chỉ cần bức tranh.)*

**Chọn kernel (tr. 71–73):**
- Nhịp phát hành: mỗi **8–12 tuần** một mainline release; chu kỳ = **merge window 2 tuần** (nhận feature) → ổn định hóa qua -rc1...-rc7/8 → release. Theo dõi thay đổi: kernelnewbies.org/LinuxVersions.
- Sau khi release, kernel chuyển từ **mainline** (Linus) sang **stable** (Greg Kroah-Hartman) — chỉ nhận bugfix, đánh số 4.1.1, 4.1.2... Stable thường chỉ được duy trì đến release kế; bản **longterm (LTS)** duy trì **2+ năm** (⚠️ nay thường 6 năm với LTS chính) — sản phẩm nhúng vòng đời dài **chọn LTS mới nhất** (tr. 73).
- **Vendor support (tr. 73):** thực tế phũ: mainline chỉ hỗ trợ tốt một phần thiết bị — thường phải dùng **kernel của vendor SoC**; lời khuyên của sách: *chọn vendor hỗ trợ tốt, tốt nhất là vendor chịu đưa thay đổi lên mainline*.
- **License (tr. 73–74):** GPL v2; gọi kernel qua syscall **không** tạo derivative work (app độc quyền chạy trên Linux vô tư); module kernel là vùng xám — thực hành chấp nhận `MODULE_LICENSE("Proprietary")` nhưng tranh cãi không dứt.

**Bố cục nguồn (tr. 74–75):** kernel 4.1 ≈ 38.000 file / 12,5 triệu dòng. Thư mục cần nhớ: `arch/` (theo kiến trúc), `Documentation/` (**tra đầu tiên**), `drivers/` (nghìn driver, chia theo loại), `fs/`, `include/`, `init/`, `kernel/` (scheduling, locking...), `mm/`, `net/`, `scripts/` (chứa dtc), `tools/` (chứa perf). Mẹo định vị của sách: driver serial nằm ở `drivers/tty/serial` chứ **không** phải `arch/$ARCH/mach-*` — vì nó là driver, không phải code lõi của SoC.

**Kconfig — hệ cấu hình (tr. 75–79):**
- Cấu hình khai trong cây file **Kconfig**; top-level `source "arch/$SRCARCH/Kconfig"` → **phải truyền `ARCH=`** khi make, không thì nó mặc định theo máy host (bẫy cross-compile số 1!). Lưu ý lịch sử: `ARCH=i386/x86_64` đều trỏ arch/x86.
- Cú pháp: `menu/config`; kiểu **bool** (y hoặc không định nghĩa), **tristate** (**y** built-in / **m** module / trống), int, hex, string. Ví dụ sách dùng: `config DEVMEM` — "/dev/mem virtual device support" (đúng cái /dev/mem của [drivers-init-power.md](drivers-init-power.md)!).
- Quan hệ: `depends on MTD` (không bật MTD thì mục này **ẩn hẳn** khỏi menu — lý do "tìm không thấy option"); `select` (kéo ngược: bật tôi là option kia tự bật).
- Kết quả là file **`.config`** (mỗi dòng `CONFIG_DEVMEM=y`); build sinh `include/generated/autoconf.h` để code C `#ifdef` theo config.
- Công cụ: `make ARCH=arm menuconfig` (tìm option: phím **/** — nhập tên **không có** tiền tố CONFIG_); **defconfig**: `make ARCH=arm multi_v7_defconfig` — cấu hình chạy được cho cả họ ARMv7 (một kernel nhiều board — tr. 79); vendor BSP thì dùng defconfig của vendor. **`make oldconfig`**: nâng `.config` cũ lên nguồn kernel mới (chỉ hỏi option mới) — quy trình chuẩn khi **kernel migration** (đúng việc bạn từng làm: 5.10 → 6.12). `.config` đã sửa phải vào **source control**.
- **CONFIG_LOCALVERSION (tr. 79–80):** đóng dấu bản kernel của mình — `CONFIG_LOCALVERSION="-melp-v1.0"` → `uname -r` = `4.1.10-melp-v1.0` (cũng là tên thư mục `/lib/modules/...`). Truy vết field: nhìn version string là biết build nào.

**Module — góc nhìn embedded (tr. 80–81):** desktop cần module (không thể built-in vạn driver); **embedded thì phần cứng đã biết trước lúc build → module kém giá trị**, thậm chí có hại: tạo phụ thuộc phiên bản kernel ↔ rootfs (update lệch nhau là không boot). **Khá phổ biến: kernel embedded không có module nào.** Khi nào module đáng dùng: (1) code độc quyền (lý do license ở trên); (2) **giảm boot time** — hoãn nạp driver không thiết yếu; (3) nhiều driver có thể cần mà built-in hết thì phí RAM (vd USB đủ loại thiết bị).

**Build — kbuild (tr. 81–85):**
- Quy tắc makefile: `obj-y += mem.o` (luôn built-in), `obj-$(CONFIG_TTY_PRINTK) += ttyprintk.o` — CONFIG=y thì built-in, =m thành module, trống thì bỏ. Đây là toàn bộ "phép màu" nối Kconfig với build.
- **Chọn target ảnh theo bootloader (tr. 81):** U-Boot cổ: `uImage`; U-Boot mới: `zImage` (lệnh `bootz`); x86: `bzImage`. Build: `make -j 4 ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- zImage`.
- **Bẫy uImage + multi-platform (tr. 82)** — chi tiết BSP đắt giá: kernel ARM multi-platform (một binary nhiều SoC, từ Linux 3.7) có **nhiều địa chỉ relocation khả dĩ**, mà header uImage chỉ ghi được **một** → build uImage sẽ fail/sai. Giải pháp: chỉ định load address của đúng SoC mình: `make ... LOADADDR=0x80008000 uImage` (tra `zreladdr-y` trong `arch/arm/mach-<soc>/Makefile.boot`).
- Sản phẩm build (tr. 83): **`vmlinux`** — kernel dạng **ELF** (build với `CONFIG_DEBUG_INFO=y` là có symbol — **chính là file mà kgdb/`decode_stacktrace` cần**, nối [debug-realtime.md](debug-realtime.md)); `System.map` — bảng symbol dạng text; từ vmlinux sinh ra: `Image` (nhị phân thô) → `zImage` (Image nén + stub tự giải nén; riêng PowerPC zImage là nén thuần — bootloader phải tự giải) → `uImage` (zImage + header 64B).
- Debug build: `make V=1` xem lệnh compile thật (tr. 84).
- **DTB:** `make ARCH=arm dtbs` — build theo `arch/$ARCH/boot/dts/Makefile`, `.dtb` nằm cạnh nguồn (tr. 84).
- **Modules:** `make ... modules`; cài vào staging rootfs: `make ... INSTALL_MOD_PATH=$HOME/rootfs modules_install` → `/lib/modules/<kernelrelease>/` (tr. 85).
- **Dọn (tr. 85):** `clean` (object) < **`mrproper`** (sạch tuyệt đối, **xóa cả .config**) < `distclean` (+file backup editor).

### Insight đáng nhớ

- Chuỗi ảnh **vmlinux → Image → zImage → uImage** đáng thuộc lòng vì mỗi mắt xích trả lời một câu hỏi thật: cần debug symbol? → vmlinux; bootloader nào? → zImage (bootz) hay uImage (bootm, cần LOADADDR nếu multi-platform).
- Triết lý module của embedded (tr. 80) **ngược với desktop**: mặc định built-in hết, thậm chí tắt CONFIG_MODULES (nhỏ hơn, an toàn hơn, không lệch version với rootfs) — chỉ mở module khi có lý do (proprietary/boot time/USB). Đây là câu trả lời "y hay m" phiên bản embedded — bổ sung cho góc nhìn tổng quát ở câu hỏi bên dưới.
- `make oldconfig` = xương sống của nghiệp vụ **kernel migration** — cầm `.config` cũ lên kernel mới, trả lời phần chênh lệch. Kể quy trình này khi được hỏi về việc nâng kernel là đúng bài thực chiến.

### Góc interview

**Câu 1:** Driver nên built-in (`y`) hay module (`m`) trên thiết bị nhúng? Nêu nghịch lý gà-trứng kinh điển.

<details><summary>Đáp án</summary>

- Khung quyết định: cần **trước khi mount rootfs** (driver storage chứa rootfs, FS của rootfs, console) → bắt buộc `y` (hoặc `m` nhưng nhét vào initramfs). Phần còn lại trên embedded **nghiêng mạnh về `y` toàn bộ** (tr. 80–81): phần cứng biết trước lúc build, module chỉ thêm phụ thuộc version kernel↔rootfs (update lệch là hỏng boot) — nhiều sản phẩm tắt hẳn CONFIG_MODULES.
- Khi nào `m` xứng đáng (tr. 80–81): code **độc quyền** (tách license GPL), **giảm boot time** (nạp muộn driver không thiết yếu — camera cần lên hình 2s không chờ driver Wi-Fi), họ thiết bị nhiều biến thể (USB) built-in hết thì phí RAM.
- **Gà-trứng:** driver eMMC để `m`, file .ko nằm trên rootfs **trong chính eMMC đó** → muốn đọc rootfs phải có driver, muốn có driver phải đọc rootfs → không bao giờ boot. Gặp ở mọi tầng: FS driver, PHY của NFS root... Giải: built-in, hoặc initramfs chứa module (cách của distro desktop).
- Điểm cộng: console/serial driver cũng phải built-in (thấy log sớm); và nhắc `INSTALL_MOD_PATH modules_install` đưa module vào staging rootfs khớp `kernelrelease` (tr. 85) — lệch version string là modprobe không tìm thấy.

</details>

**Câu 2:** Giải thích các file `vmlinux`, `Image`, `zImage`, `uImage`. Vì sao build uImage cho kernel ARM multi-platform bị lỗi, sửa thế nào?

<details><summary>Đáp án</summary>

- **vmlinux** (tr. 83): kernel dạng **ELF** đầy đủ — không boot trực tiếp (bootloader thường không nạp ELF) nhưng là file quý nhất với BSP: chứa **debug symbol** (khi CONFIG_DEBUG_INFO=y) cho kgdb, giải mã oops (`addr2line`/decode_stacktrace), `size`/`objdump` phân tích. `System.map` = bảng symbol text đi kèm.
- **Image**: vmlinux đổ ra **nhị phân thô** (objcopy). **zImage**: Image nén + **stub code tự giải nén và relocate** gắn phía trước (riêng PowerPC: chỉ là bản nén, bootloader tự giải — tr. 83). **uImage**: zImage + **header 64 byte** của U-Boot (magic, loại, load address, entry point, CRC — tr. 57–58) để `bootm` nhận diện; U-Boot mới có `bootz` nạp thẳng zImage, khỏi cần uImage.
- **Lỗi multi-platform (tr. 82):** từ Linux 3.7, một binary ARM chạy nhiều SoC — nhưng mỗi SoC đặt RAM ở địa chỉ vật lý khác nhau → **địa chỉ relocation của kernel khác nhau**, trong khi header uImage chỉ chứa được **một** load address → mkimage không biết điền gì, build fail. Sửa: chỉ định đúng SoC đích — `make LOADADDR=0x80008000 uImage` (giá trị tra `zreladdr-y` trong `arch/arm/mach-<soc>/Makefile.boot`); hoặc tốt hơn: dùng **zImage + bootz** (zImage tự relocate — vốn là lý do nó tồn tại).
- Chi tiết này phân biệt người từng build kernel ARM thật với người đọc lý thuyết — đáng kể trong phỏng vấn BSP.

</details>

---

## Cụm 5 — Boot kernel: panic, early userspace, cmdline, porting board mới (ch. 4, tr. 86–94)

### Nội dung chính

**Trình tự boot chuẩn trên BeagleBone Black (tr. 86)** — thuộc lòng 4 lệnh này là kể được cả câu chuyện:

```
U-Boot# fatload mmc 0:1 0x80200000 zImage                 # nạp kernel vào RAM
U-Boot# fatload mmc 0:1 0x80f00000 am335x-boneblack.dtb   # nạp DTB (địa chỉ khác, không đè)
U-Boot# setenv bootargs console=ttyO0,115200              # hợp đồng trao kernel
U-Boot# bootz 0x80200000 - 0x80f00000                     # boot: kernel, (không ramdisk), dtb
Starting kernel ...
[    0.000000] Booting Linux on physical CPU 0x0
```

Sách nói thẳng (tr. 86): không có `console=` đúng thì **không thấy bất kỳ dòng nào sau "Starting kernel ..."** và không biết hệ sống hay chết. Lưu ý tên console **theo driver của kernel**: BBB là `ttyO0` (driver OMAP), không phải `ttyS0` — đối chiếu bảng tên (ttyS/ttyO/ttymxc/ttyAMA tùy SoC).

**Kernel panic (tr. 87):** ví dụ sách — boot thành công đến tận: `Kernel panic - not syncing: VFS: Unable to mount root fs on unknown-block(0,0)` → kernel **vô dụng nếu không có userspace**: nó không tìm được root filesystem. Mặc định panic là **đứng im**; tham số `panic=N` cho reboot sau N giây (production nên đặt — kèm watchdog). Sửa nhanh để có shell: nạp thêm ramdisk `uRamdisk` và `bootargs ... rdinit=/bin/sh` → `bootz <kernel> <ramdisk> <dtb>` → có prompt `/ #`.

**Early user space — kernel chuyển giao cho userspace thế nào (tr. 88):** code trong `init/main.c`: `rest_init()` tạo thread đầu tiên (PID 1) chạy `kernel_init()`:
1. **Có ramdisk?** → thử chạy **`/init`** trong đó (đổi bằng `rdinit=`).
2. **Không/thất bại** → mount filesystem thật: `prepare_namespace()` dùng **`root=`** (dạng `root=/dev/mmcblk0p1` — thiết bị+partition) → mount xong thử lần lượt **`/sbin/init` → `/etc/init` → `/bin/init` → `/bin/sh`**, dừng ở cái đầu tiên chạy được (đổi bằng `init=`).

**Kernel messages (tr. 88–89):** printk có 8 mức 0–7 (`KERN_EMERG`=0 → `KERN_DEBUG`=7); mọi message vào ring buffer **`__log_buf`** (kích thước 2^`CONFIG_LOG_BUF_SHIFT`), xem bằng `dmesg`; message có mức **thấp hơn console loglevel** (mặc định 7) mới hiện ra console; chỉnh: `loglevel=<n>` trên cmdline hoặc `dmesg -n <n>`.

**Kernel command line (tr. 89–90):** đến từ 3 nguồn — `bootargs` của U-Boot, **trong device tree** (node `/chosen`), hoặc build cứng `CONFIG_CMDLINE`. Bảng tham số hữu ích nhất của sách:

| Tham số | Ý nghĩa |
|---|---|
| `debug` / `quiet` | Console loglevel = 8 (thấy hết) / = 1 (chỉ khẩn cấp — **serial chậm, bớt in là bớt thời gian boot**) |
| `init=` / `rdinit=` | Chương trình PID 1 trên rootfs / trên ramdisk (mặc định `/sbin/init` / `/init`) |
| `root=` / `rootfstype=` | Thiết bị chứa rootfs / loại FS (jffs2 bắt buộc khai) |
| `rootwait` / `rootdelay=` | **Chờ vô hạn** thiết bị root xuất hiện (gần như bắt buộc với mmc!) / chờ N giây |
| `ro` / `rw` | Mount root read-only / read-write |
| `panic=` | Hành vi khi panic: >0 reboot sau N giây, 0 đứng im (mặc định), <0 reboot ngay |
| `lpj=` | Đặt sẵn `loops_per_jiffy` — **né ~250ms** hiệu chuẩn delay loop mỗi lần boot (tr. 90: phần cứng cố định thì giá trị không đổi — đọc số từ log `Calibrating delay loop... (lpj=4980736)` rồi điền vào) |

**Porting Linux sang board mới (tr. 90–94):** phạm vi phụ thuộc board giống board mẫu đến đâu; sách làm tiếp board Nova (≈BBB):
- Bố cục code ARM: mỗi SoC một thư mục **`arch/arm/mach-*`**, code chung nhiều đời SoC ở `plat-*` (tr. 90).
- **Có device tree (tr. 91) — đường hiện đại, RẤT ít việc:** copy `am335x-boneblack.dts` → `nova.dts`, sửa `model = "Nova"` (giữ danh sách compatible cũ + có thể thêm của mình) → build `make ARCH=arm nova.dtb` (thêm vào `arch/arm/boot/dts/Makefile`: `dtb-$(CONFIG_SOC_AM33XX) += nova.dtb` để `make dtbs` tự build) → **boot đúng zImage multi_v7 cũ, chỉ thay DTB** → log hiện `Machine model: Nova`. Muốn tinh gọn thì copy multi_v7_defconfig rồi cắt bớt. **Toàn bộ port = 1 file DTS + 1 dòng Makefile.**
- **Không device tree (tr. 92–94) — đường cũ, để so sánh:** thêm `config MACH_NOVABOARD` vào Kconfig của mach-omap2; viết **board file** `board-nova.c` (copy board-am335xevm.c) + rule Makefile `obj-$(CONFIG_MACH_NOVABOARD) += board-nova.o`; **xin machine number** (danh sách toàn cầu!) thêm vào `mach-types`; macro `MACHINE_START(NOVABOARD, ...)` khai các hàm init (map_io, init_irq, init_machine...); tạo defconfig riêng; **và sửa cả U-Boot** để truyền đúng machine number qua register r1 (`CONFIG_MACH_TYPE`). So sánh hai đường là thấy ngay *vì sao DT thắng*: N file code + số đăng ký toàn cầu + kernel riêng mỗi board, so với 1 file dữ liệu.

### Insight đáng nhớ

- Bảng cmdline (tr. 89–90) chứa sẵn ba câu chuyện boot-nhanh mà interviewer thích: **`quiet`** (in serial 115200 rất chậm — bớt log là bớt hàng trăm ms), **`lpj=`** (né 250ms hiệu chuẩn), và Falcon mode (cụm 3 — bỏ hẳn một pha). Gom lại thành câu trả lời "giảm thời gian boot" có số liệu.
- Trình tự thử init của kernel (`/sbin/init → /etc/init → /bin/init → /bin/sh`, tr. 88) + `init=/bin/sh` là **cửa hậu chẩn đoán** vạn năng: rootfs nghi hỏng init thì boot thẳng vào shell mà xem xét ([toolchain-rootfs.md](toolchain-rootfs.md) có câu hỏi "no init found" dùng đúng chiêu này).
- Câu chuyện port Nova hai đường (tr. 91 vs 92–94) là **minh chứng lịch sử sống** cho câu "DT giải quyết vấn đề gì" — kể được nó là câu trả lời DT có chiều sâu vượt định nghĩa.

### Góc interview

**Câu 1:** Kernel panic `VFS: Unable to mount root fs on unknown-block(0,0)` — nguyên nhân khả dĩ và trình tự xử lý?

<details><summary>Đáp án</summary>

- Ý nghĩa (tr. 87): kernel boot tốt nhưng **không mount được rootfs** — "unknown-block(0,0)" = không có block device nào khớp `root=`. Nhóm nguyên nhân, kiểm tra theo thứ tự:
  1. **`root=` sai** thiết bị/partition (tr. 88: dạng `/dev/mmcblk0p1` — nhớ chữ `p` với mmc): đối chiếu dòng kernel log liệt kê partition có sẵn ngay phía trên panic.
  2. **Thiếu `rootwait`** (tr. 90): mmc/USB enumerate **chậm hơn** thời điểm kernel mount root → lúc được lúc không. Gần như luôn cần `rootwait` với eMMC/SD.
  3. **Driver storage không có trong kernel** (built-in! — để `m` là gà-trứng, xem cụm 4) hoặc **FS của rootfs chưa bật** (`rootfstype=` + CONFIG tương ứng; jffs2 phải khai tường minh — tr. 90).
  4. DTB sai/thiếu node storage → controller không probe (kiểm tra log MMC phía trên).
  5. Rootfs thật sự hỏng (mkfs/flash lỗi).
- Công cụ khoanh nhanh: boot với **ramdisk + `rdinit=/bin/sh`** (tr. 87) — vào được shell chứng tỏ kernel/console ổn, lỗi khoanh vào storage/rootfs; từ shell `cat /proc/partitions` xem kernel thấy gì.
- Phân biệt ăn điểm: panic này ≠ panic "**No working init found**" — cái sau là mount root **thành công** nhưng không chạy được init (thiếu file/sai kiến trúc/thiếu loader — [toolchain-rootfs.md](toolchain-rootfs.md)); hai panic ở hai tầng khác nhau, chẩn đoán khác nhau.

</details>

**Câu 2:** Bạn cần giảm thời gian boot của thiết bị từ 8 giây xuống dưới 3 giây. Nêu các đòn bẩy theo từng pha boot, có định lượng.

<details><summary>Đáp án</summary>

Đi theo pha (đo trước bằng timestamp trên console / grabserial):
1. **Bootloader:** giảm `bootdelay` về 0–1; **Falcon mode** (tr. 65–66) — SPL nạp thẳng kernel bỏ qua U-Boot proper: tiết kiệm cả pha (hàng trăm ms đến cả giây); env gọn, bỏ dò storage/network không cần.
2. **Nạp ảnh:** kernel nhỏ hơn = nạp nhanh hơn (cắt config — từ multi_v7 đầy đủ xuống config riêng SoC, tr. 91); nén phù hợp (LZ4 giải nén nhanh hơn gzip); eMMC mode nhanh.
3. **Kernel:** `quiet` (in serial 115200 tốn hàng trăm ms — tr. 89), **`lpj=`** điền sẵn (né ~250ms hiệu chuẩn — tr. 90), driver không thiết yếu chuyển **module nạp sau** khi UI/dịch vụ chính đã lên (tr. 80 — một trong ba lý do dùng module), bỏ probe tốn thời gian (deferred/async probe), initcall_debug tìm hàm init chậm.
4. **Userspace:** init tối giản (BusyBox script thẳng thay vì systemd nếu hệ nhỏ — [drivers-init-power.md](drivers-init-power.md)), khởi động dịch vụ chính **trước**, phần còn lại lười; rootfs read-only squashfs (mount nhanh, khỏi fsck).
- Chốt phương pháp: **đo từng pha trước khi tối ưu** (ranh giới: banner SPL → banner U-Boot → "Starting kernel" → dòng log init đầu → dịch vụ sẵn sàng), tối ưu chỗ chiếm nhiều nhất — trả lời có phương pháp đo + con số cụ thể (250ms lpj, serial log...) ăn điểm hơn liệt kê chiêu.

</details>

### Đọc thêm (tùy chọn)

- [05/device-tree.md](../../05-drivers-device-tree/device-tree.md), [05/driver-basics.md](../../05-drivers-device-tree/driver-basics.md) — nền DT/driver của repo.
- [08/boot-process.md](../../08-embedded-systems/boot-process.md) — boot process bản cô đọng.
- [lkd/modules-debug.md](../lkd/modules-debug.md) — bus–device–driver: phần móng của cơ chế probe.
- [OSTEP virtualization-cpu](../ostep/virtualization-cpu.md) — trap/syscall: chuyện sau khi kernel nắm quyền.
