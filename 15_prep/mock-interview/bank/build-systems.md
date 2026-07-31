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

Truyền **toolchain file** (`cmake -DCMAKE_TOOLCHAIN_FILE=arm.cmake`) khai target khác host: `CMAKE_SYSTEM_NAME` (Linux), `CMAKE_SYSTEM_PROCESSOR` (aarch64), `CMAKE_C/CXX_COMPILER` (cross gcc), `CMAKE_SYSROOT` (rootfs target — nơi tìm headers/lib), và `CMAKE_FIND_ROOT_PATH_MODE_*` (bắt `find_*` tìm trong sysroot, không dính host). Điểm gốc mọi rắc rối cross-compile: **"cái này đến từ host hay sysroot?"**. Với Yocto, SDK sinh sẵn toolchain file + environment (`source environment-setup-*`). *(Lỗi "not found" khi chạy: [BSP-019](bsp.md).)*
</details>

#### BLD-004 · 🟡 · concept · [→ yocto §2](../../../06-build-systems/yocto.md)
**Một Yocto recipe (`.bb`) gồm những gì?**
<details><summary>Đáp án</summary>

Công thức build **một package**: `SRC_URI` (nguồn: git/tarball/file + patch), `LICENSE` + `LIC_FILES_CHKSUM`, `DEPENDS`/`RDEPENDS`, và các **task** `do_fetch → do_unpack → do_patch → do_configure → do_compile → do_install → do_package`. Thường `inherit` một class (`autotools`, `cmake`, `kernel`, `systemd`) để có hành vi build chuẩn thay vì viết tay. `PACKAGES` chia output thành nhiều gói (`-dev`,`-dbg`,`-doc`). BitBake dựng task graph từ dependency giữa các recipe.
</details>

#### BLD-005 · 🟡 · concept · ⭐ · [→ yocto §2](../../../06-build-systems/yocto.md)
**Layer và `.bbappend` là gì? Vì sao không sửa recipe gốc?**
<details><summary>Đáp án</summary>

**Layer (`meta-*`)** = tập recipe + conf có priority (BSP layer, distro layer, app layer), bật trong `bblayers.conf`. **`.bbappend`** = file mở rộng/sửa một recipe của layer khác (thêm patch, config) **mà không đụng file gốc**. Nguyên tắc vàng: mọi tùy biến ở **layer riêng của bạn** qua bbappend/patch — **không sửa poky/vendor layer** (mất khi cập nhật, phá tái lập). Cũng không sửa `tmp/work` (bị nghiền build sau). Giữ vậy để nâng cấp vendor BSP không mất tùy biến.
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
