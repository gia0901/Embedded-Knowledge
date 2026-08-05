# BLD — Build systems (CMake, Yocto, cross-compile, CI)

> Domain `BLD`. Hệ build cho embedded Linux: CMake, Yocto, cross-compile, CI. Nguồn: [06-build-systems](../../../06-build-systems/). Track dùng: `bsp`, `cpp-system`, `melp`. Yocto/BSP-layer chuyên sâu còn ở [BSP-017…019](bsp.md).

---

#### BLD-001 · 🟡 · concept · [→ cmake](../../../06-build-systems/cmake.md)
**CMake là gì, khác Make thế nào? "Modern CMake" nghĩa là gì?**
<details><summary>Đáp án</summary>

`make` chạy trực tiếp một Makefile (luật + lệnh). **CMake** là **meta-build**: mô tả dự án ở mức cao (`CMakeLists.txt`) rồi **sinh** build system cho generator chọn (Makefiles, Ninja, IDE) → portable đa nền tảng/compiler. **Modern CMake** (≥3.x) = **target-based**: mọi thứ gắn vào **target** (`add_library`/`add_executable`) qua `target_link_libraries`, `target_include_directories`, `target_compile_features` với phạm vi `PUBLIC/PRIVATE/INTERFACE` — thay cho biến toàn cục cũ (`include_directories`, `CMAKE_CXX_FLAGS`). Dependency và cờ **lan truyền theo target** (usage requirements), sạch và ít lỗi hơn.
</details>

#### BLD-002 · 🟡 · concept · [→ cmake](../../../06-build-systems/cmake.md)
**`target_link_libraries` với `PUBLIC` / `PRIVATE` / `INTERFACE` khác nhau gì?**
<details><summary>Đáp án</summary>

Chỉ **phạm vi lan truyền usage requirement** (include dir, define, link) sang target khác dùng target này: **`PRIVATE`** — chỉ dùng khi *build chính target này*, không lan cho consumer (vd lib phụ chỉ dùng nội bộ). **`INTERFACE`** — không dùng để build target này nhưng **lan cho consumer** (header-only lib). **`PUBLIC`** — cả hai (target dùng *và* consumer cũng cần — vd lib xuất hiện trong header public của bạn). Đặt đúng phạm vi tránh "leak" dependency và giảm rebuild.
</details>

#### BLD-003 · 🟠 · concept · ⭐ · [→ cross-compilation](../../../06-build-systems/cross-compilation.md)
**Cross-compile với CMake làm thế nào? Toolchain file chứa gì?**
<details><summary>Đáp án</summary>

**Vấn đề gốc của mọi cross-compile — một câu hỏi duy nhất, lặp lại ở mọi bước: "thứ này đến từ *host* hay từ *target sysroot*?"** Compiler, header, thư viện, công cụ phụ trợ, kết quả `find_package` — mỗi thứ đều có thể lấy nhầm bên, và nhầm thì lỗi hoặc là "wrong architecture" lúc link, hoặc tệ hơn: **build sạch nhưng crash trên board**.

**Cách làm:** không sửa `CMakeLists.txt`, mà truyền **toolchain file** lúc configure — giữ project không biết gì về target cụ thể.

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake
```

```cmake
# arm-toolchain.cmake
set(CMAKE_SYSTEM_NAME      Linux)          # ⭐ đặt biến này = báo CMake "đang cross-compile"
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_SYSROOT      /path/to/target-sysroot)   # header + lib CỦA TARGET

# Chặn find_* lôi nhầm đồ của host:
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BEFORE)  # chương trình: chạy trên HOST -> lấy host
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # thư viện: CHỈ trong sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # header:   CHỈ trong sysroot
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

| Thành phần | Vai trò |
|---|---|
| `CMAKE_SYSTEM_NAME` | Bật chế độ cross (`CMAKE_CROSSCOMPILING` = TRUE) — thiếu nó thì mọi thứ khác vô nghĩa |
| `CMAKE_SYSROOT` | Cây `/usr` của board: nơi tìm header/lib **của target** |
| `CMAKE_FIND_ROOT_PATH_MODE_*` | ⭐ Tách "công cụ chạy trên host" khỏi "thư viện link cho target". `PROGRAM` phải lấy **host** (protoc, flex…), `LIBRARY/INCLUDE` phải **ONLY** sysroot |

**Với Yocto:** không tự viết — SDK sinh sẵn toolchain file + môi trường, chỉ cần `source environment-setup-<arch>-poky-linux` rồi cmake bình thường.

**Bẫy:** (1) quên `CMAKE_SYSTEM_NAME` → CMake tưởng build native, `try_run()` và mọi kiểm tra tính năng chạy trên host cho kết quả **sai**; (2) `find_package` lấy `.so` của host → link "thành công" rồi lỗi kiến trúc; đó chính là lý do có `FIND_ROOT_PATH_MODE`; (3) **cache CMake cũ** — đổi toolchain file phải **xoá thư mục build**, không thì nó giữ compiler cũ; (4) chạy trên board báo `not found` dù file có thật → thường là thiếu **dynamic loader**/lib đúng ABI, kiểm bằng `file` và `readelf -l` ([BSP-019](bsp.md)); (5) chương trình sinh code chạy lúc build (code generator) phải build **cho host**, không phải target — dấu hiệu cần chia hai bước build.

**Chốt:** *"Toolchain file trả lời 'host hay sysroot?' một lần cho toàn dự án: compiler + sysroot + `FIND_ROOT_PATH_MODE`. Project không cần biết mình đang được cross-compile."*
</details>

#### BLD-004 · 🟡 · concept · [→ yocto §2](../../../06-build-systems/yocto.md)
**Một Yocto recipe (`.bb`) gồm những gì?**
<details><summary>Đáp án</summary>

Công thức build **một package**: `SRC_URI` (nguồn: git/tarball/file + patch), `LICENSE` + `LIC_FILES_CHKSUM`, `DEPENDS`/`RDEPENDS`, và các **task** `do_fetch → do_unpack → do_patch → do_configure → do_compile → do_install → do_package`. Thường `inherit` một class (`autotools`, `cmake`, `kernel`, `systemd`) để có hành vi build chuẩn thay vì viết tay. `PACKAGES` chia output thành nhiều gói (`-dev`,`-dbg`,`-doc`). BitBake dựng task graph từ dependency giữa các recipe.
</details>

#### BLD-005 · 🟡 · concept · ⭐ · [→ yocto §2](../../../06-build-systems/yocto.md)
**Layer và `.bbappend` là gì? Vì sao không sửa recipe gốc?**
<details><summary>Đáp án</summary>

**Layer (`meta-*`)** = một tập recipe + conf đóng gói theo mối quan tâm, có **priority**, bật trong `bblayers.conf`:

| Loại layer | Ai giữ | Ví dụ |
|---|---|---|
| **BSP layer** (`meta-<board>`) | Vendor SoC (NXP/TI/ST) | kernel + u-boot bbappend, machine conf, firmware |
| **Distro layer** | Tổ chức bạn | chính sách chung: init system, libc, feature |
| **App/software layer** (`meta-<sản phẩm>`) | **Bạn** | recipe ứng dụng + **mọi tuỳ biến** |

**`.bbappend`** = file mở rộng/sửa một recipe **thuộc layer khác** mà **không đụng vào file gốc**. Tên phải khớp recipe (`linux-imx_%.bbappend` — `%` khớp mọi version).

```
meta-myproduct/
└── recipes-kernel/linux/
    ├── linux-imx_%.bbappend        # ⭐ mở rộng recipe kernel của vendor
    └── linux-imx/
        ├── 0001-add-my-driver.patch
        └── my-feature.cfg          # config fragment: bật CONFIG_*
```
```bitbake
# linux-imx_%.bbappend
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-add-my-driver.patch \
            file://my-feature.cfg"
```

**Vì sao tuyệt đối không sửa recipe gốc — ba lý do, lý do (3) mới là lý do thật:**
1. **Mất khi cập nhật** — vendor phát hành BSP mới, bạn `git pull`/thay layer → tuỳ biến bay sạch hoặc xung đột.
2. **Không tái lập được** — người khác clone repo không có sửa đổi của bạn; build của bạn ≠ build của CI.
3. ⭐ **Không tách được "cái của mình" khỏi "cái của vendor"** — khi nâng cấp BSP (hoặc lên kernel mới), bạn cần trả lời *"tôi đã đổi những gì?"*. Với bbappend, câu trả lời là **một danh sách patch tường minh** trong layer của bạn. Với sửa trực tiếp, bạn phải `diff` cả cây và đoán.

**Bẫy:** (1) ⚠️ sửa trong **`tmp/work/...`** để "thử nhanh" rồi quên — bị **nghiền sạch** lần build sau, và nhiều giờ debug đi theo; muốn thử nhanh thì dùng **`devtool modify <recipe>`** rồi **`devtool finish`** để đẩy thành bbappend/patch; (2) bbappend **không khớp version** recipe → BitBake **âm thầm bỏ qua** (dùng `%` hoặc kiểm bằng `bitbake-layers show-appends`); (3) quên `FILESEXTRAPATHS:prepend` → không tìm thấy file patch; (4) sửa `local.conf` thay vì layer — tiện nhưng cũng không tái lập được cho team/CI.

**Chốt:** *"Layer là đơn vị đóng gói và chia trách nhiệm; bbappend là cách sửa đồ của người khác mà không chạm vào nó. Mọi tuỳ biến nằm trong layer của bạn — để nâng cấp BSP vendor không xoá mất công sức."*
</details>

#### BLD-006 · 🟠 · concept · [→ yocto §3](../../../06-build-systems/yocto.md)
**sstate-cache là gì? Vì sao build "không nhận thay đổi" và cách xử lý?**
<details><summary>Đáp án</summary>

**sstate (shared state) cache**: kết quả mỗi task được cache theo **hash đầu vào** (recipe + config + dependency) → lần sau build **chỉ chạy lại task có hash đổi**, phần còn lại lấy từ cache → build từ giờ xuống phút, **chia sẻ được giữa dev/CI**. Mặt trái: nếu bạn sửa nguồn theo cách hash không bắt được (vd sửa thẳng `tmp/work`), BitBake **dùng lại cache cũ** → "không nhận thay đổi". Xử lý: sửa đúng chỗ (recipe/layer) để hash đổi, hoặc `bitbake -c cleansstate <recipe>` rồi build lại.
</details>

#### BLD-007 · 🟡 · concept · [→ yocto §6](../../../06-build-systems/yocto.md)
**`devtool` dùng để làm gì?**
<details><summary>Đáp án</summary>

Công cụ phát triển recipe nhanh: `devtool modify <recipe>` kéo source ra workspace để **sửa + build lại lặp nhanh** (không phải sửa recipe thủ công mỗi vòng); `devtool add <url>` tạo recipe mới từ source; `devtool upgrade` nâng version; `devtool finish` gói thay đổi thành **bbappend/patch** trong layer của bạn. Là cách hiện đại thay cho vòng lặp "sửa recipe → cleansstate → bitbake" chậm chạp.
</details>

#### BLD-008 · 🟠 · concept · [→ yocto §5](../../../06-build-systems/yocto.md)
**Thêm một package vào image thế nào? `IMAGE_INSTALL` vs `RDEPENDS` khác gì?**
<details><summary>Đáp án</summary>

Thêm gói vào rootfs: sửa **image recipe** — `IMAGE_INSTALL:append = " mypkg"` (hoặc qua `PACKAGE_GROUP`). Đừng nhét vào recipe của package khác. **`RDEPENDS`** khác mục đích: khai **dependency runtime của một package** (khi cài package A thì cần B đi kèm) — dùng khi B phải theo A ở mọi image (vd `dlopen` lib). Quy tắc: "tôi muốn image có X" → `IMAGE_INSTALL`; "package A không chạy được nếu thiếu B" → `RDEPENDS:${PN}-A += "B"`. *(Chi tiết DEPENDS/RDEPENDS: [BSP-018](bsp.md).)*
</details>

#### BLD-009 · 🟡 · concept · [→ yocto §6](../../../06-build-systems/yocto.md)
**Yocto SDK / eSDK để làm gì?**
<details><summary>Đáp án</summary>

`bitbake <image> -c populate_sdk` sinh **SDK** = toolchain cross + **sysroot** (headers/lib target) đóng gói cho **đội app** build phần mềm **ngoài Yocto** (không cần cả build tree): `source environment-setup-*` rồi `cmake`/`make` như thường, đúng target. **eSDK** (extensible SDK) còn kèm **devtool** để thêm/sửa recipe và tích hợp ngược vào build. Đây là cách tách vai: đội BSP dựng distro, đội app dùng SDK.
</details>

#### BLD-010 · 🟠 · design · [→ yocto](../../../06-build-systems/yocto.md)
**Thiết kế CI (vd Jenkins) cho một dự án embedded Linux?**
<details><summary>Đáp án</summary>

- **Build**: pipeline chạy `bitbake <image>` với **sstate-cache dùng chung** (mount/copy sstate + downloads giữa các lần build) → build tăng tốc; pin version layer (manifest/`repo`/submodule) để tái lập.
- **Chất lượng**: build cả bản có warning-as-error, chạy static analysis (MISRA/cppcheck), unit test **trên host** (build cùng code, chạy ASan/TSan), license/CVE check (`cve-check`).
- **Test trên target**: sau build, deploy image lên **board thật / QEMU / HIL** (hardware-in-the-loop) chạy smoke test + integration; thu log/artifact.
- **Artifact**: lưu image + SDK + manifest version cho truy vết; tag theo commit.
- Đánh đổi: build Yocto nặng → sstate + build node mạnh; test HIL cần hạ tầng phần cứng. Nêu được "sstate cache để CI không build lại từ đầu" là điểm cộng.
</details>

---
⬅️ [Bank index](README.md)
