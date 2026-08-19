# Yocto Project — build một Linux distro tùy biến

> Chuẩn thực tế để dựng BSP/distro cho sản phẩm embedded Linux (vendor NXP/TI/ST phát hành BSP dạng Yocto layer). Trọng tâm phỏng vấn BSP (vd JD Datalogic yêu cầu "Yocto project experience"). Bổ trợ [cross-compilation.md](cross-compilation.md), [cmake.md](cmake.md).
> Ôn dạng phỏng vấn: bank [BLD-004…009](../14-prep/mock-interview/bank/build-systems.md) + [BSP-017…019](../14-prep/mock-interview/bank/bsp.md).

---

## 1. Yocto là gì (và không là gì)

- **Không phải một distro** — mà là **framework build ra distro tùy biến của bạn**.
- Thành phần: **BitBake** (build engine — đọc metadata, dựng task graph, thực thi) + **OpenEmbedded-Core** (metadata nền) + **Poky** (bản tham chiếu gộp sẵn để bắt đầu).
- vs **Buildroot**: Buildroot = Makefile+Kconfig sinh *một image*, học nhanh; Yocto = metadata layer sinh *một distro* cho **nhiều SKU**, có **sstate cache**, **SDK**, license/CVE tooling — giá là đường học dốc. Sản phẩm nghiêm túc/nhiều biến thể → Yocto.

## 2. Kiến trúc metadata (khái niệm cốt lõi)

- **Recipe (`.bb`)**: công thức build **một package** — `SRC_URI` (nguồn), các **task** `do_fetch → do_unpack → do_patch → do_configure → do_compile → do_install → do_package`, `DEPENDS`/`RDEPENDS`, `PACKAGES`.
- **Layer (`meta-*`)**: tập recipe + conf, có **priority**; phân loại: **BSP layer** (`meta-<board>`), distro layer, app/software layer. Bật trong `bblayers.conf`.
- **`.bbappend`**: **sửa/mở rộng recipe của layer khác mà KHÔNG đụng gốc** (thêm patch, config) — nguyên tắc vàng: tùy biến bằng bbappend/patch trong **layer riêng**, không sửa poky/vendor layer.
- **Class (`.bbclass`)** + `inherit`: hành vi build dùng chung (`autotools`, `cmake`, `kernel`, `systemd`, `module`).
- **Config**: `conf/local.conf` (`MACHINE`, `DISTRO`, `IMAGE_INSTALL`), `conf/machine/<board>.conf` (BSP: `KERNEL_DEVICETREE`, preferred u-boot/kernel, tune), `conf/distro/<distro>.conf`.
- **Image recipe** (`core-image-*`): danh sách package → sinh **rootfs**.

## 3. Build flow & sstate

```
bitbake core-image-minimal
```
BitBake dựng **task graph** cho toàn bộ dependency rồi chạy các `do_*` task. Điểm ăn tiền:

- **sstate-cache** (shared state): kết quả mỗi task được cache theo hash đầu vào → **build lại chỉ phần đổi**, chia sẻ được giữa dev/CI (giảm build từ giờ xuống phút). Hash đổi (đổi recipe/config) → task chạy lại.
- **`tmp/work/...`**: nơi build tạm — **đừng sửa trực tiếp** (bị nghiền lần build sau); mọi thay đổi phải ở recipe/layer.
- **sysroot** (`recipe-sysroot`): headers/lib mà recipe thấy lúc build (build-time = `DEPENDS`).

## 4. BSP layer (`meta-<board>`) — gồm gì

- `conf/machine/<board>.conf`: DTB (`KERNEL_DEVICETREE`), `PREFERRED_PROVIDER_virtual/kernel` + u-boot config, console, `DEFAULTTUNE`.
- **kernel bbappend** (`linux-*.bbappend`): patch DT/driver, **config fragment `.cfg`** (bật CONFIG), defconfig.
- **u-boot bbappend**; firmware blob; image recipe tham chiếu machine.

## 5. DEPENDS vs RDEPENDS & packaging

- **DEPENDS** = build-time (trong sysroot lúc compile); **RDEPENDS** = runtime (phải **có mặt trong image**).
- Compile OK mà chạy thiếu lib: thường do `dlopen()`/plugin/`system()` (shlibs scan không thấy) hoặc package bị **tách** (`-dev`,`-dbg`,`-doc`,`-staticdev`). Fix: `RDEPENDS:${PN} += "libfoo"`; soi bằng `oe-pkgdata-util`. (Chi tiết: [BSP-018](../14-prep/mock-interview/bank/bsp.md).)

## 6. Công cụ phát triển hằng ngày

- **`devtool`**: `devtool modify <recipe>` (kéo source ra chỉnh + build lại nhanh), `devtool add` (tạo recipe mới từ source), `devtool upgrade`, `devtool finish` (đẩy thay đổi thành bbappend/patch). Cách hiện đại để phát triển recipe.
- **`bitbake -c <task> <recipe>`** (chạy một task: `compile`, `devshell`, `cleansstate`), **`bitbake-layers`** (thêm/xem layer), **`recipetool`**, **`oe-pkgdata-util`**.
- **SDK / eSDK** (`bitbake <image> -c populate_sdk`): toolchain + sysroot đóng gói cho **đội app** build ngoài Yocto; eSDK còn kèm devtool.

## 7. License & CVE

`LICENSE` + `LIC_FILES_CHKSUM` mỗi recipe (build fail nếu license đổi mà chưa duyệt) → sinh **manifest license** cho tuân thủ; **`cve-check`** class quét CVE theo version package. Đây là lý do lớn chọn Yocto cho sản phẩm thương mại.

## 8. Bẫy thực chiến (hay hỏi)

- Sửa `tmp/work` hoặc poky/vendor layer → **mất khi build lại**; luôn bbappend/patch trong layer riêng.
- **sstate stale**: build "không nhận thay đổi" → `bitbake -c cleansstate <recipe>` rồi build lại.
- `MACHINE` mismatch với sstate/tmp cũ → dọn hoặc dùng `TMPDIR` riêng.
- Thêm package vào image: sửa **image recipe / `IMAGE_INSTALL`**, không nhét vào recipe package khác.
- Quên `RDEPENDS` cho dependency runtime (dlopen) → image thiếu lib dù compile OK.

---

## Ôn tập (bank)

[BLD-004…009](../14-prep/mock-interview/bank/build-systems.md) (Yocto), [BSP-017](../14-prep/mock-interview/bank/bsp.md) (Buildroot vs Yocto + BSP layer), [BSP-018](../14-prep/mock-interview/bank/bsp.md) (DEPENDS/RDEPENDS), [BSP-019](../14-prep/mock-interview/bank/bsp.md) (cross-compile "not found"). Đọc thêm: [melp/build-systems](../15-book-summaries/melp/build-systems.md).
