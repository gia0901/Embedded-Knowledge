# Yocto Project — build một Linux distro tùy biến

> **TL;DR**
> - Yocto **không phải distro** — là **framework** để bạn sinh ra distro của riêng mình. Ba tầng: **BitBake** (engine) + **OE-Core** (metadata nền) + **Poky** (bản tham chiếu).
> - **Vấn đề nó giải không phải "build được Linux"** (Buildroot cũng làm được) mà là **tái lập được sau nhiều năm, trên nhiều biến thể phần cứng, có bằng chứng license/CVE**.
> - Đơn vị tư duy: **recipe** (build một package) → **layer** (đóng gói + chia trách nhiệm) → **image** (danh sách package thành rootfs).
> - Luật vàng: **mọi tuỳ biến sống trong layer của bạn** bằng `.bbappend`/patch. Không sửa poky, không sửa vendor layer, không sửa `tmp/work`.
> - Cái giá: đường học dốc, build đầu **hàng chục phút–vài giờ**, đĩa **~50–100 GB**. Đổi lại **sstate** làm build lần sau còn vài phút.
>
> Bổ trợ [cross-compilation.md](cross-compilation.md), [cmake.md](cmake.md), [ci-and-test-farm.md](ci-and-test-farm.md) (chỗ sstate + `MACHINE` chứng minh giá trị: build 10 platform mỗi lần submit). 🧪 Thực hành: [BSP-036…038](../14-prep/mock-interview/bank/bsp.md).

---

## 1. Bản chất — Yocto giải quyết vấn đề gì

**Câu trả lời nông** (ai cũng nói được): *"để build một Linux tuỳ biến cho board của mình."*
Nhưng script `make` + `busybox` cũng làm được, và **Buildroot làm việc đó nhanh hơn nhiều**. Nếu đó là toàn bộ nhu cầu thì **đừng dùng Yocto**.

**⭐ Câu trả lời sâu — ba câu hỏi mà cách làm tay không trả lời nổi:**

| Câu hỏi thật của sản phẩm | Vì sao script tay chết |
|---|---|
| *"Ba năm sau, người viết đã nghỉ. Build lại đúng firmware đã bán cho khách được không?"* | Script phụ thuộc **máy ai chạy nó**: version toolchain, gói host, thứ tự cài. Không có gì ghim lại |
| *"Năm SKU dùng chung 90% phần mềm, khác nhau ở DTB và vài driver. Quản thế nào?"* | Copy 5 nhánh rồi phân kỳ dần — đến SKU thứ ba là không ai biết nhánh nào có bản vá nào |
| *"Khách hàng đòi danh sách license và báo cáo CVE của toàn bộ firmware."* | Không có metadata thì phải **đọc tay từng package** |

⇒ **Yocto bán thứ gọi là *tái lập được, có metadata*.** Mọi thứ — nguồn, patch, license, cấu hình, phiên bản toolchain — đều là **văn bản nằm trong git**. Build là **hàm** của tập văn bản đó.

Đó cũng là lý do **vendor SoC (NXP, TI, ST, Qualcomm) phát hành BSP dạng Yocto layer**: họ giao cho bạn *metadata*, không phải một image.

> 📌 **Câu chốt dùng ở phỏng vấn:** *"Buildroot cho tôi một image. Yocto cho tôi một **định nghĩa tái lập được** của image đó — và với sản phẩm sống 5–10 năm, thứ thứ hai mới là thứ tôi cần."*

---

## 2. Khi nào **KHÔNG** dùng Yocto

Đây là nửa câu trả lời hay bị bỏ, và là chỗ phân biệt người đã dùng thật với người đã đọc.

| Tình huống | Chọn | Vì sao |
|---|---|---|
| Một sản phẩm, một biến thể, đội 1–3 người | **Buildroot** | Yocto tốn nhiều tuần học mà không trả lại gì; Buildroot đọc hết `Config.in` trong một buổi |
| Prototype, cần chạy trong tuần này | **Image dựng sẵn** (Debian cho board) | Không build gì cả |
| Cần **cập nhật gói lẻ ngoài hiện trường** | Yocto **hoặc** distro thật | Buildroot **không có package manager** — muốn vá một lib phải flash lại cả image |
| Nhiều SKU · vòng đời dài · cần SDK cho đội app · cần audit license/CVE | ⭐ **Yocto** | Đúng bài toán nó sinh ra để giải |
| Đội app không biết embedded, chỉ muốn compile app | Yocto **+ phát SDK** | Họ dùng SDK, không cần biết Yocto |

⚠️ **Bẫy tư duy phổ biến:** *"dự án nghiêm túc thì phải Yocto"*. Sai — **quy mô biến thể và vòng đời** mới là tiêu chí, không phải mức độ nghiêm túc.

---

## 3. Ba tầng: BitBake · OE-Core · Poky

Câu này hay bị hỏi vì nhiều người dùng Yocto vài tháng vẫn nói lẫn lộn.

| Tầng | Là gì | Ví von |
|---|---|---|
| **BitBake** | **Engine** — đọc metadata, dựng task graph, chạy task, quản sstate. **Không biết gì về Linux** | Như `make`, nhưng cho hàng nghìn package |
| **OpenEmbedded-Core** (`meta`) | **Metadata nền** — recipe cho toolchain, libc, kernel, busybox, hàng nghìn package chung | Thư viện chuẩn |
| **Poky** | **Bản tham chiếu** = BitBake + OE-Core + `meta-poky` + `meta-yocto-bsp`, gói sẵn để chạy ngay | Một bản phân phối của hai thứ trên |
| **Yocto Project** | **Dự án ô** — quản Poky, tài liệu, autobuilder, chu kỳ phát hành LTS | Tổ chức |

> **Chốt:** *"BitBake là engine, OE-Core là metadata, Poky là bản gộp sẵn để bắt đầu. Yocto là dự án ô quản cả ba."*

---

## 4. Metadata — bốn thứ phải phân biệt

### 4.1 Recipe (`.bb`) — công thức build **một** package

```bitbake
# meta-myproduct/recipes-apps/sensord/sensord_1.2.bb
SUMMARY  = "Ambient light sensor daemon"
LICENSE  = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI  = "git://git.example.com/sensord.git;branch=main;protocol=https \
            file://0001-fix-i2c-timeout.patch \
            file://sensord.service"
SRCREV   = "a1b2c3d4e5f6"          # ⭐ ghim commit — KHONG dung ${AUTOREV} cho san pham

S = "${WORKDIR}/git"

DEPENDS  = "libgpiod"              # can luc BUILD (header + lib de compile)
RDEPENDS:${PN} = "i2c-tools"       # can luc CHAY  (phai co trong image)

inherit cmake systemd              # muon hanh vi build chuan, khong viet tay
SYSTEMD_SERVICE:${PN} = "sensord.service"
```

**Task mặc định BitBake chạy cho recipe này**, theo thứ tự:

| Task | Làm gì | Hay hỏng vì |
|---|---|---|
| `do_fetch` | Tải nguồn về `DL_DIR` | Mạng, `SRCREV` sai, thiếu `protocol=` |
| `do_unpack` | Giải nén vào `${WORKDIR}` | |
| `do_patch` | Áp patch trong `SRC_URI` | Patch không áp được sau khi vendor nâng version |
| `do_configure` | `cmake`/`configure` | Thiếu `DEPENDS` ⇒ không tìm thấy header |
| `do_compile` | Build | |
| `do_install` | Cài vào `${D}` (**thư mục dàn dựng**, không phải rootfs) | Quên `${D}` ⇒ ghi ra máy host |
| `do_package` | Chia `${D}` thành `-dev`, `-dbg`, `-doc`, `${PN}` | File rơi vào gói sai ⇒ image thiếu |

> 📌 **`${D}` không phải rootfs.** Nó là thư mục dàn dựng riêng của recipe. Rootfs được ghép **sau**, từ các package đã đóng gói. Hiểu sai chỗ này là nguồn của lỗi *"tôi cài rồi mà image không có"*.

### 4.2 Layer (`meta-*`) — đóng gói và **chia trách nhiệm**

```
meta-myproduct/
├── conf/layer.conf              # BBFILE_PRIORITY, pattern nhan dien
├── recipes-apps/sensord/...     # recipe cua BAN
├── recipes-kernel/linux/
│   ├── linux-ti_%.bbappend      # mo rong recipe kernel CUA VENDOR
│   └── linux-ti/
│       ├── 0001-add-my-driver.patch
│       └── my-feature.cfg       # config fragment
└── recipes-bsp/u-boot/u-boot-ti_%.bbappend
```

Bật trong `conf/bblayers.conf`. **Priority** (`BBFILE_PRIORITY` trong `layer.conf`) quyết định layer nào thắng khi hai layer có **cùng recipe cùng version** — số lớn thắng.

⚠️ Priority **không** áp cho `.bbappend`: **mọi** bbappend khớp đều được áp. Đây là chỗ hai layer cùng append một recipe rồi đá nhau.

### 4.3 `.bbappend` — sửa đồ người khác mà không chạm vào nó

```bitbake
# meta-myproduct/recipes-kernel/linux/linux-ti_%.bbappend
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"     # de tim thay file cua minh
SRC_URI += "file://0001-add-my-driver.patch \
            file://my-feature.cfg"
```
```
# my-feature.cfg  --  config fragment, KHONG phai defconfig day du
CONFIG_GPIO_SYSFS=y
CONFIG_I2C_CHARDEV=y
```

⭐ **Vì sao dùng fragment thay vì thay cả defconfig:** defconfig của vendor đổi theo mỗi bản BSP. Thay cả file ⇒ mỗi lần nâng cấp bạn **mất hết thay đổi của vendor**. Fragment chỉ nói *"thêm ba dòng này"* ⇒ **merge được**.

### 4.4 Class (`.bbclass`) + `inherit`

Hành vi build dùng chung. `inherit cmake` ⇒ recipe không cần viết `do_configure`/`do_compile`. Class hay gặp: `autotools`, `cmake`, `meson`, `kernel`, `module`, `systemd`, `pkgconfig`, `native`/`nativesdk`.

---

## 5. ⭐ Cú pháp override — chỗ sai nhiều nhất, và hay bị hỏi

### 5.1 `+=` khác `:append` thế nào — **không phải chuyện thẩm mỹ**

```bitbake
FOO += "b"          # gan NGAY luc parse dong nay
FOO:append = " b"   # HOAN LAI, gan sau khi parse xong toan bo
```

Khác biệt lộ ra khi nhiều file cùng đụng một biến:

```bitbake
# recipe goc
FOO = "a"

# bbappend cua ban
FOO += "b"           # neu bbappend parse TRUOC dong `FOO = "a"` -> BI GHI DE, mat "b"
FOO:append = " b"    # ✅ luon an toan: ap sau cung
```

> **Luật thực dụng: trong `.bbappend` hầu như luôn dùng `:append`/`:prepend`/`:remove`, không dùng `+=`.**
> `+=` an toàn khi bạn là **chủ** recipe đó và chắc thứ tự parse.

⚠️ **`:append` không tự thêm dấu cách** — `FOO:append = "b"` cho ra `"ab"`. Luôn viết `" b"` với khoảng trắng đầu.

### 5.2 Override theo điều kiện

```bitbake
IMAGE_INSTALL:append                  = " sensord"       # moi machine
IMAGE_INSTALL:append:beaglebone-yocto = " i2c-tools"     # chi machine nay
RDEPENDS:${PN}:remove                 = "some-heavy-lib" # go khoi danh sach
```

### 5.3 ⚠️ Cú pháp cũ dùng `_` — sẽ gặp khi tra Google

Yocto **3.4 (honister, 2021)** đổi dấu phân tách override từ `_` sang `:`.

| Cũ (≤ 3.3) | Mới (≥ 3.4) |
|---|---|
| `IMAGE_INSTALL_append` | `IMAGE_INSTALL:append` |
| `RDEPENDS_${PN}` | `RDEPENDS:${PN}` |
| `SRC_URI_append_imx6` | `SRC_URI:append:imx6` |

⚠️ Phần lớn kết quả StackOverflow còn dùng cú pháp cũ. Trên bản mới nó **không báo lỗi rõ ràng** — biến chỉ **im lặng không có tác dụng**. Đây là một trong những lỗi tốn thời gian nhất với người mới.

---

## 6. Build flow & sstate — cơ chế, không phải khẩu hiệu

```bash
bitbake core-image-minimal
```

BitBake **parse toàn bộ metadata** → dựng **task graph** (`do_*` của mọi recipe trong cây phụ thuộc) → chạy song song theo `BB_NUMBER_THREADS`.

### ⭐ sstate thật sự hoạt động thế nào

sstate cache **theo TASK, không theo recipe**, và khoá bằng **hash của mọi đầu vào task đó**: nội dung recipe, giá trị biến nó dùng, hash của các task nó phụ thuộc, phiên bản class…

⇒ Hệ quả quan trọng, và đây là **câu trả lời phân biệt**:

> Câu hỏi thật không phải *"sstate có nhanh không"* mà là ***"thay đổi của tôi làm hỏng hash của bao nhiêu task?"***

| Bạn đổi gì | Hỏng bao nhiêu | Vì sao |
|---|---|---|
| Nội dung một recipe app lá | **Ít** — chỉ recipe đó + đóng gói image | Không ai phụ thuộc nó |
| `DISTRO_FEATURES` hoặc `TUNE` | **Gần như tất cả** | Mọi task compile đều đọc biến này |
| Nâng version toolchain | **Tất cả** | Là gốc của cây |
| Sửa thẳng trong `tmp/work/` | 🔴 **KHÔNG cái nào** | Hash không nhìn vào đó ⇒ dùng lại cache cũ ⇒ *"build không nhận thay đổi"* |

**Ba mức dọn, đừng dùng nhầm:**

| Lệnh | Xoá gì | Khi nào |
|---|---|---|
| `bitbake -c clean <recipe>` | `tmp/work` của recipe — **giữ sstate** | Hiếm khi đủ (build lại sẽ lấy từ sstate) |
| `bitbake -c cleansstate <recipe>` | `tmp/work` **và** sstate của recipe | Khi nghi sstate cũ |
| `rm -rf tmp/` | Cả thư mục làm việc, **giữ `sstate-cache/`** | Dọn sạch mà vẫn nhanh — 🧪 [BSP-038](../14-prep/mock-interview/bank/bsp.md) đo chính cái này |

📌 Tách `DL_DIR` và `SSTATE_DIR` **ra ngoài `build/`** để `rm -rf tmp/` không giết chúng:
```
DL_DIR     = "${TOPDIR}/../downloads"
SSTATE_DIR = "${TOPDIR}/../sstate-cache"
```

**Con số điển hình** *(tham khảo — số thật của máy bạn sẽ có sau [BSP-038](../14-prep/mock-interview/bank/bsp.md))*: build đầu `core-image-minimal` **20 phút–2 giờ** tuỳ CPU; build lại không đổi gì **dưới 1 phút**; `rm -rf tmp/` rồi build lại với sstate còn nguyên **vài phút**. Đĩa **50–100 GB**.

---

## 7. `DEPENDS` vs `RDEPENDS` — và chỗ Yocto **mù**

| | `DEPENDS` | `RDEPENDS` |
|---|---|---|
| Lúc nào | **BUILD** | **CHẠY** |
| Nội dung là gì | Tên **recipe** | Tên **package** |
| Ở đâu | `recipe-sysroot` (header + lib để compile/link) | **Trong image** trên thiết bị |
| Sai thì lỗi lúc | `do_configure`/`do_compile` — *"không tìm thấy header"* | **Ngoài hiện trường** — chạy mới lòi |

### ⭐ Vì sao compile OK mà chạy thiếu lib

Yocto **tự sinh phần lớn `RDEPENDS`** nhờ **shlibs scan**: sau `do_package`, nó đọc trường **`NEEDED`** trong ELF của binary và suy ra runtime dependency.

⇒ **Nó chỉ thấy được thứ linker ghi vào ELF.** Bốn ca nó **mù**:

| Ca | Vì sao mù |
|---|---|
| ⭐ **`dlopen("libfoo.so")`** | Phân giải **lúc chạy theo chuỗi tên** — không tạo `NEEDED` |
| Gọi binary ngoài bằng `system()` / `popen()` | Chỉ là một chuỗi trong code |
| Script, file cấu hình, dữ liệu, firmware blob | Không phải ELF |
| Plugin nạp động | Như `dlopen` |

```bitbake
RDEPENDS:${PN} += "libfoo curl"     # phai khai TAY
```

**Kiểm chứng:**
```bash
readelf -d myapp | grep NEEDED                    # linker ghi gi vao ELF
oe-pkgdata-util list-pkg-files -p sensord         # goi nay chua file gi
oe-pkgdata-util find-path /usr/lib/libfoo.so.1    # file nay thuoc goi nao
```

🧪 Nhìn thấy nó hỏng một lần: [BSP-037](../14-prep/mock-interview/bank/bsp.md).

### `IMAGE_INSTALL` khác `RDEPENDS`

| Ý định | Dùng |
|---|---|
| *"Tôi muốn **image này** có X"* | `IMAGE_INSTALL:append = " X"` trong **image recipe** |
| *"Package A **không chạy được** nếu thiếu B"* | `RDEPENDS:${PN} += "B"` trong **recipe của A** |

⚠️ Nhét vào `local.conf` thì **tiện nhưng không tái lập được** cho team/CI — chỉ dùng để thử.

---

## 8. BSP layer (`meta-<board>`) — gồm gì

```
meta-mybsp/
├── conf/machine/myboard.conf
│     KERNEL_DEVICETREE = "ti/am335x-myboard.dtb"
│     PREFERRED_PROVIDER_virtual/kernel     = "linux-ti"
│     PREFERRED_PROVIDER_virtual/bootloader = "u-boot-ti"
│     UBOOT_MACHINE    = "myboard_defconfig"
│     SERIAL_CONSOLES  = "115200;ttyS0"
│     MACHINE_FEATURES = "usbhost vfat ext2"
├── recipes-kernel/linux/linux-ti_%.bbappend      # patch + config fragment
├── recipes-bsp/u-boot/u-boot-ti_%.bbappend
└── recipes-bsp/firmware/...                      # blob wifi/bluetooth
```

**Ranh giới trách nhiệm** — cũng là câu trả lời cho *"anh sẽ đặt thay đổi của mình ở đâu?"*:

| Thứ | Thuộc layer nào |
|---|---|
| DTB, kernel config cho **board** | **BSP layer** |
| Chính sách chung: init system, libc, feature | **Distro layer** |
| Ứng dụng của bạn + mọi tuỳ biến | **Layer sản phẩm của bạn** |

---

## 9. `devtool` — vòng lặp phát triển hiện đại

Vòng lặp cũ *(chậm, dễ sai)*: sửa recipe → `cleansstate` → `bitbake` → đợi → lặp.

```bash
devtool modify sensord        # keo source ra workspace/, thanh git repo that
#   ... sua code binh thuong, commit tung buoc ...
devtool build sensord         # build lai nhanh, chi phan doi
devtool deploy-target sensord root@192.168.1.10   # ⭐ day thang len board
devtool finish sensord ../meta-myproduct          # goi thanh patch + bbappend
```

⭐ `devtool deploy-target` đổi hẳn nhịp làm việc: sửa code → thấy trên board trong **vài giây**, không cần dựng lại image.

`devtool add <url>` tạo recipe mới từ source; `devtool upgrade` nâng version.

---

## 10. SDK / eSDK — cho đội **không** dùng Yocto

```bash
bitbake core-image-minimal -c populate_sdk        # SDK: toolchain + sysroot
bitbake core-image-minimal -c populate_sdk_ext    # eSDK: kem devtool
```

Đội app cài SDK, `source environment-setup-*`, rồi build bằng CMake/Make như bình thường — **không cần biết Yocto tồn tại**.

⭐ Đây là câu trả lời cho *"làm sao 20 kỹ sư app làm việc mà không ai phải học Yocto?"* — và là một lý do lớn chọn Yocto ở tổ chức đông người.

---

## 11. License & CVE — lý do thương mại

- Mỗi recipe **bắt buộc** khai `LICENSE` + `LIC_FILES_CHKSUM`. File license đổi nội dung ⇒ **build FAIL** cho tới khi người ta xem lại. Đây là **cơ chế cưỡng chế**, không phải tài liệu.
- `INCOMPATIBLE_LICENSE = "GPL-3.0*"` — chặn từ gốc nhóm license không muốn đưa vào sản phẩm.
- `inherit cve-check` → quét CVE theo version package, sinh báo cáo.
- Sinh **license manifest** cho toàn image ⇒ đáp ứng nghĩa vụ tuân thủ khi bán hàng.

---

## 12. ⚠️ Bẫy thực chiến

**① Sửa trong `tmp/work/` để "thử nhanh".** Bị **nghiền sạch** lần build sau, và **sstate không nhận ra** nên build vẫn "thành công" với code cũ. Nhiều giờ debug đi theo. → Dùng `devtool modify`.

**② `.bbappend` không khớp version ⇒ BitBake ÂM THẦM BỎ QUA.** Không cảnh báo, không lỗi. `linux-ti_5.10.bbappend` **không** áp cho `linux-ti_6.1`. → Dùng `%`, và kiểm bằng `bitbake-layers show-appends`.

**③ Quên `FILESEXTRAPATHS:prepend`** ⇒ patch/fragment trong bbappend không tìm thấy.

**④ Dùng `+=` trong bbappend** ⇒ có thể bị recipe gốc ghi đè tuỳ thứ tự parse. → Dùng `:append` (§5.1).

**⑤ Cú pháp `_` cũ trên Yocto ≥ 3.4** ⇒ biến **im lặng** không tác dụng (§5.3).

**⑥ Quên `RDEPENDS` cho `dlopen`** ⇒ image thiếu lib dù compile sạch (§7).

**⑦ `${AUTOREV}` trong sản phẩm** ⇒ mỗi lần build lấy commit mới nhất ⇒ **mất tái lập**, đúng thứ bạn chọn Yocto vì nó. Chỉ dùng lúc phát triển; sản phẩm phải **ghim `SRCREV`**.

**⑧ Đổi `MACHINE` mà giữ `tmp/` cũ** ⇒ lẫn artifact. → `TMPDIR` riêng cho từng machine, hoặc `rm -rf tmp/`.

**⑨ Nhét tuỳ biến vào `local.conf`.** Chạy được trên máy bạn, không tái lập trên CI hay máy đồng nghiệp. `local.conf` là **chỗ thử**, không phải chỗ lưu.

**⑩ Quên `${D}` trong `do_install`** ⇒ ghi thẳng vào máy host.

---

## 13. 🔧 Lệnh gỡ rối — bốn lệnh cứu phần lớn tình huống

```bash
# 1. Bien nay THUC SU mang gia tri gi, va AI da gan no?   <- lenh huu ich nhat
bitbake -e core-image-minimal | grep -B4 "^IMAGE_INSTALL="

# 2. Vi sao thay doi cua toi khong lam gi ca? -> so hash truoc/sau
bitbake -S printdiff core-image-minimal

# 3. bbappend cua toi co duoc ap khong?
bitbake-layers show-appends
bitbake-layers show-recipes linux-ti      # layer nao dang cung cap, version nao thang

# 4. File nay thuoc goi nao / goi nay chua gi?
oe-pkgdata-util find-path /usr/lib/libfoo.so.1
oe-pkgdata-util list-pkg-files -p sensord
```

⭐ **`bitbake -e` là lệnh đáng thuộc nhất.** Nó in ra giá trị cuối cùng của mọi biến **kèm lịch sử ai gán ở đâu** — trả lời trực tiếp câu *"tôi set rồi mà sao không ăn?"*, vốn là phần lớn vấn đề của người mới.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [BSP-017](../14-prep/mock-interview/bank/bsp.md) | Buildroot vs Yocto — chọn thế nào? BSP layer gồm gì? |
| [BSP-018](../14-prep/mock-interview/bank/bsp.md) | `DEPENDS` vs `RDEPENDS`? Vì sao compile OK mà chạy thiếu lib? |
| [BSP-019](../14-prep/mock-interview/bank/bsp.md) | Cross-compile xong chạy báo `not found` — vì sao? |
| [BLD-004](../14-prep/mock-interview/bank/build-systems.md) | Một recipe `.bb` gồm những gì? |
| [BLD-005](../14-prep/mock-interview/bank/build-systems.md) | Layer và `.bbappend` là gì? Vì sao không sửa recipe gốc? |
| [BLD-006](../14-prep/mock-interview/bank/build-systems.md) | sstate-cache là gì? Vì sao build "không nhận thay đổi"? |
| [BLD-007](../14-prep/mock-interview/bank/build-systems.md) | `devtool` dùng để làm gì? |
| [BLD-008](../14-prep/mock-interview/bank/build-systems.md) | Thêm package vào image thế nào? `IMAGE_INSTALL` vs `RDEPENDS`? |
| [BLD-009](../14-prep/mock-interview/bank/build-systems.md) | Yocto SDK / eSDK để làm gì? |

🧪 **Thực hành:** [BSP-036](../14-prep/mock-interview/bank/bsp.md) (Yocto dựng lại chuỗi boot) · [BSP-037](../14-prep/mock-interview/bank/bsp.md) (bbappend + `dlopen`/`RDEPENDS`) · [BSP-038](../14-prep/mock-interview/bank/bsp.md) (sstate — đo thật). Setup: [lab-setup.md](../14-prep/lab-setup.md).

📖 Đọc thêm: [melp/build-systems](../15-book-summaries/melp/build-systems.md).

---
⬅️ [cross-compilation.md](cross-compilation.md) · ➡️ [06-build-systems](README.md)
