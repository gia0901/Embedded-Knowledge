# MELP — Build Systems: Buildroot & Yocto (ch. 6–8) 🎯

> Thuộc [MELP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Hầu hết công ty làm BSP dùng Yocto — cụm này gần như chắc chắn được hỏi ở mức "bạn đã dùng đến đâu".

---

## Cụm 1 — Vì sao cần build system; Buildroot (ch. 6)

### Nội dung chính

Build tay 4 thành phần (toolchain, bootloader, kernel, rootfs) không tái lập được và không scale — build system tự động hóa **toàn bộ**: tải nguồn (kèm checksum), build theo đúng thứ tự dependency, sinh image flash được; giá trị cốt lõi là **reproducibility** (ai build cũng ra đúng một ảnh — điều kiện cần của sản phẩm nghiêm túc, truy vết CVE, chứng nhận).

**Buildroot:** hệ **makefile + Kconfig** (menuconfig y hệt kernel):
- `make menuconfig` chọn target arch, toolchain, package, kernel, bootloader → `make` → `output/images/` (rootfs.ext4, Image, u-boot.bin...).
- Cấu hình board = **defconfig** (`make <board>_defconfig`); tùy biến bằng overlay rootfs, post-build script, external tree (`BR2_EXTERNAL` — giữ tùy biến ngoài cây Buildroot).
- Triết lý: **đơn giản, minh bạch, build cả hệ từ đầu**; **không có package manager trên target** (ảnh nguyên khối — update là thay cả ảnh); thời gian build toàn bộ mỗi khi đổi lớn (ccache đỡ một phần).
- Hợp với: hệ nhỏ-vừa, một sản phẩm một cấu hình, đội nhỏ, muốn hiểu và kiểm soát từng dòng.

### Insight đáng nhớ

- Buildroot vs Yocto là câu "chọn phe" phổ biến; câu trả lời trưởng thành: **Buildroot = cái makefile lớn dễ hiểu; Yocto = cái distro generator mạnh nhưng dốc** — chọn theo *quy mô biến thể sản phẩm và đội ngũ*, không theo mốt (một board một ảnh → Buildroot vài ngày là chạy; nhiều SKU, nhiều team, cần SDK/feed → Yocto trả lãi về sau).

### Ít quan trọng

- Danh sách biến BR2_* thường dùng và cấu trúc package/*.mk (ch. 6 giữa) — tra manual Buildroot khi làm.

### Góc interview

**Câu 1:** So sánh Buildroot và Yocto — khi nào chọn cái nào?

<details><summary>Đáp án</summary>

| | Buildroot | Yocto |
|---|---|---|
| Bản chất | Makefile + Kconfig sinh **một ảnh** | Framework metadata (bitbake) sinh **một distro** |
| Đường học | Ngày–tuần | Tuần–tháng (khái niệm riêng: recipe, layer, sstate...) |
| Tùy biến nhiều sản phẩm/SKU | Mỗi cấu hình một defconfig — nhân bản thủ công | **Layer/override** — thiết kế cho việc này |
| Package manager trên target | Không (ảnh nguyên khối) | Có thể (rpm/ipk feed) |
| Build lại sau thay đổi nhỏ | Kém thông minh (hay phải rebuild rộng) | **sstate cache** — chỉ build phần đổi, chia sẻ cache CI/team |
| SDK cho đội app | Thô sơ | **eSDK/SDK chuẩn** sinh từ chính image |
| Truy vết license/CVE | Cơ bản | Trưởng thành (SPDX, cve-check) |

- Chọn **Buildroot**: sản phẩm đơn, đội nhỏ, ưu tiên tốc độ khởi động dự án + minh bạch. Chọn **Yocto**: nhiều biến thể phần cứng/SKU, nhiều team (BSP tách app qua SDK), cần vendor BSP layer (NXP/TI/ST đều phát hành Yocto layer — thực tế ngành ép về Yocto), vòng đời dài cần vá CVE có hệ thống.
- Điểm cộng: nhắc lựa chọn thứ ba thực dụng — dùng **vendor Yocto BSP** làm gốc thay vì tự dựng từ zero; và cảnh báo chi phí vận hành Yocto (build server, disk trăm GB) phải được tính.

</details>

---

## Cụm 2 — Yocto Project: khái niệm cốt lõi (ch. 7–8) 🎯

### Nội dung chính

**Bộ từ vựng bắt buộc** (phỏng vấn kiểm tra bằng chính các từ này):

```
bitbake        — engine thực thi task theo dependency (fetch→unpack→patch→
                 configure→compile→install→package), lập lịch song song
recipe (.bb)   — "cách build MỘT package": SRC_URI, LICENSE, dependency
                 (DEPENDS=build-time, RDEPENDS=runtime), do_compile()...
append (.bbappend) — VÁ một recipe của layer khác không sửa gốc
                 (thêm patch, đổi config) — cơ chế tùy biến chuẩn mực
layer (meta-*) — nhóm recipe/config theo chủ đề, có thứ tự ưu tiên:
                 poky(core) + meta-arm + meta-<vendor BSP> + meta-<công-ty-bạn>
                 → tùy biến CHỒNG lên nhau, không fork
MACHINE        — cấu hình PHẦN CỨNG (conf/machine/*.conf: DT nào, kernel nào,
                 u-boot nào, tune ISA) — do BSP layer cung cấp
DISTRO         — chính sách PHẦN MỀM (libc, init system, feature toàn cục)
image recipe   — danh sách package vào rootfs (core-image-minimal...) 
                 → sinh .wic/.ext4/... flash được
```

- **Tách MACHINE khỏi DISTRO khỏi IMAGE** là kiến trúc đáng giá nhất của Yocto: đổi board không đụng chính sách phần mềm, đổi distro không đụng BSP — trả lời được "vì sao tách" là hiểu Yocto thật.
- **sstate cache (shared state)**: kết quả từng task được hash theo **toàn bộ đầu vào** (nguồn, config, dependency) — trùng hash thì lấy từ cache thay vì build; chia sẻ được giữa máy/CI → build lần đầu hàng giờ, lần sau phút.
- **devtool** (workflow chỉnh sửa hiện đại): `devtool modify <recipe>` — checkout nguồn ra workspace, sửa/build/test nhanh trên board, `devtool finish` sinh patch + bbappend về layer của mình.
- **SDK/eSDK**: sinh từ image (`bitbake -c populate_sdk`) — toolchain + sysroot **khớp chính xác** ảnh đang chạy, phát cho đội app: BSP và app tách vai sạch (đúng mô hình team lớn).
- Nơi ngó khi lỗi: `bitbake -e <recipe>` (biến sau mọi override), log từng task trong `tmp/work/.../temp/log.do_*`, `bitbake-layers show-layers/show-appends`.
- Tùy biến kernel/U-Boot trong Yocto: bbappend vào recipe kernel (`linux-<vendor>`), fragment `.cfg` cho Kconfig, patch qua `SRC_URI` — **không sửa thẳng trong tmp/work** (mất sau lần build sau).

### Insight đáng nhớ

- Câu định vị Yocto một dòng: **"Yocto không build phần mềm — nó build DISTRO; đơn vị tái sử dụng là layer, đơn vị tùy biến là bbappend/override."** Người mới sai chỗ nào? — sửa file trong `tmp/` (bị nghiền), fork recipe thay vì bbappend, nhét mọi thứ vào một layer.
- Với vị trí BSP: bạn sẽ **sống trong meta-BSP layer** — machine conf, kernel recipe + defconfig fragment, U-Boot recipe, DT patch, firmware — nói được đúng "một BSP layer gồm những gì" là điểm ăn tiền phỏng vấn.

### Ít quan trọng

- Lịch sử OpenEmbedded/poky/bitbake, cấu trúc thư mục poky đầy đủ, các class (.bbclass) nâng cao ngoài image/kernel (ch. 8) — tích lũy khi dùng thật.

### Góc interview

**Câu 1 (🎯):** Công ty giao bạn port sản phẩm sang board mới dùng Yocto với vendor BSP layer. Mô tả cấu trúc layer và các bước bạn làm.

<details><summary>Đáp án</summary>

1. **Dựng khung**: lấy poky đúng release LTS + vendor BSP layer (meta-imx/meta-ti...) **cùng branch codename** (trộn release là nguồn lỗi số 1); tạo **layer riêng `meta-<sảnphẩm>`** — mọi tùy biến của mình nằm đây, không đụng poky/vendor.
2. **MACHINE mới**: `conf/machine/myboard.conf` — thường `require` machine gần nhất của vendor rồi override: DTB (`KERNEL_DEVICETREE`), U-Boot config, console, feature phần cứng.
3. **Kernel & U-Boot**: `.bbappend` vào recipe vendor — thêm patch (DT board mình, driver), config fragment `.cfg`; giữ nguồn qua `SRC_URI` + git để tái lập.
4. **Image & distro**: image recipe riêng (gói cần thiết, user, RO rootfs), distro conf nếu cần chính sách riêng (systemd/sysvinit, libc).
5. **Chu trình làm việc**: `devtool modify` khi sửa nguồn dài hơi; `bitbake -e`/log task khi lỗi; sstate + CI build; sinh **SDK** phát cho đội app.
- Cấu trúc trả lời "poky + vendor + **layer của mình**, machine tách khỏi distro, mọi thay đổi là append/patch có nguồn gốc" chính là đáp án chuẩn; nêu thêm anti-pattern (sửa tmp/work, fork recipe, layer trộn lẫn BSP với app) để chốt điểm.

</details>

**Câu 2:** DEPENDS khác RDEPENDS thế nào trong recipe? Vì sao ảnh build xong thiếu một thư viện lúc chạy dù build không lỗi?

<details><summary>Đáp án</summary>

- **DEPENDS** = cần lúc **build** (headers, lib để link, tool native) — bảo đảm sysroot của recipe có đủ đồ khi compile; **RDEPENDS** = cần lúc **chạy** (lib .so, binary gọi lúc runtime, script interpreter) — quyết định cái gì được **kéo vào image** cùng package.
- Build không lỗi mà chạy thiếu lib: (1) lib được link có mặt lúc build (DEPENDS/sysroot) nhưng **không ai khai RDEPENDS** → không được cài vào image — với .so link trực tiếp Yocto thường tự phát hiện (shlibs scan), nên trường hợp lọt lưới điển hình là **dlopen()** (nạp runtime — scanner không thấy), plugin, hoặc gọi binary ngoài qua system(); (2) package tách nhỏ (`-dev`, `-libs`) mà image chỉ cài package chính.
- Fix: thêm `RDEPENDS:${PN} += "libfoo"` vào recipe (không phải nhét vào image tay — để dependency đi theo package), kiểm tra bằng `oe-pkgdata-util` xem package chứa/kéo gì.
- Trả lời phân biệt "build-time sysroot vs runtime image" + kể đúng cái bẫy dlopen là mức đã-từng-đau-mới-biết.

</details>

### Đọc thêm (tùy chọn)

- [06-build-systems/](../../06-build-systems/) — Make/CMake/cross-compile nền tảng bên dưới mọi recipe.
- [toolchain-rootfs.md](toolchain-rootfs.md) — những gì Yocto/Buildroot đang tự động hóa hộ bạn.
