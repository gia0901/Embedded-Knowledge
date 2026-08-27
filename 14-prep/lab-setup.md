# 🔧 Lab Setup — BeagleBone Black

> **Hạ tầng cho bộ 8 bài [🧪 BSP-031…038](mock-interview/bank/bsp.md) (mục *J — LAB NGỒI MÁY*).**
> Đặt ở đây, **không chép vào từng lab** — 8 bài dùng chung một setup, giữ 8 bản sao là tự tạo nợ ([CLAUDE.md §4.7](../CLAUDE.md)).
>
> ⚠️ **Về độ tin cậy của file này:** phần **danh sách phần cứng, gói host, và bẫy đã biết** là chắc chắn. Phần **lệnh cụ thể** (tên biến U-Boot, địa chỉ nạp, đường dẫn thiết bị) **đổi theo phiên bản và theo máy** — coi là *chỉ dẫn cần đối chiếu*, không phải công thức đã kiểm chứng. Chạy được rồi thì **sửa lại file này cho đúng máy mình**.

---

## 1. Phần cứng — mua/chuẩn bị trước

| Thứ | Bắt buộc | Ghi chú |
|---|---|---|
| **BeagleBone Black** (rev C) | 🔴 | Rev C có 4 GB eMMC — xem bẫy ① |
| **Cáp USB–TTL 3.3V** (FTDI FT232 / CP2102 / PL2303) | 🔴 **BẮT BUỘC** | 🔴 **Phải là 3.3V. Cáp 5V làm hỏng board.** Không có console = mù hoàn toàn ⇒ toàn bộ Thẻ A vô nghĩa |
| **Nguồn 5V/2A jack tròn** | 🟠 rất nên | Cấp nguồn qua USB thường **thiếu dòng** lúc boot ⇒ board reset giữa chừng, triệu chứng **giống hệt** lỗi phần mềm |
| **2 × microSD 8–16 GB, class 10** | 🔴 | Thẻ **A** = chuỗi boot tay · Thẻ **B** = Yocto |
| Đầu đọc microSD trên máy host | 🔴 | |

### Đấu nối serial

BBB có header **J1, 6 chân**, cạnh jack nguồn. **Chân 1 có dấu chấm trắng trên board.**

```
J1 (BBB)          Cap FTDI 3.3V
 1  GND    <-----  GND   (day den)
 4  RX     <-----  TX
 5  TX     ----->  RX
```

🔴 **KHÔNG nối chân VCC/nguồn của cáp.** Board tự cấp nguồn riêng; nối nguồn từ hai đường là cách phổ biến nhất để hỏng board.

> ⚠️ Đối chiếu lại với **BBB System Reference Manual** trước khi cắm lần đầu — sơ đồ trên là chỉ dẫn, không thay được tài liệu board.

**Tham số console:** `115200 8N1`, không flow control.

---

## 2. Máy host — cài gì

### 2.1 Serial console

```bash
sudo apt install picocom      # hoac: minicom, screen
```

🔴 **Bẫy gần như ai cũng dính lần đầu — `Permission denied` trên `/dev/ttyUSB0`:**
```bash
sudo usermod -aG dialout $USER
# BAT BUOC dang xuat/dang nhap lai (hoac reboot) thi nhom moi co hieu luc
groups | grep dialout          # kiem tra
```

Mở console + ghi log (bước ② của mọi lab đều cần log thật):
```bash
picocom -b 115200 /dev/ttyUSB0 --logfile boot.log
# thoat: Ctrl-A Ctrl-X
```

### 2.2 Công cụ thẻ SD & bootloader

```bash
sudo apt install dosfstools e2fsprogs parted u-boot-tools device-tree-compiler
```
- `u-boot-tools` → **`mkimage`** (bài 035 cần)
- `device-tree-compiler` → `dtc`

### 2.3 Cross toolchain *(cần từ bài 035 trở đi)*

```bash
sudo apt install gcc-arm-linux-gnueabihf
arm-linux-gnueabihf-gcc --version
```

### 2.4 Yocto host packages *(Thẻ B)*

Danh sách chính thức cho Ubuntu 22.04:
```bash
sudo apt install gawk wget git diffstat unzip texinfo gcc build-essential \
     chrpath socat cpio python3 python3-pip python3-pexpect xz-utils \
     debianutils iputils-ping python3-git python3-jinja2 python3-subunit \
     zstd liblz4-tool file locales libacl1
sudo locale-gen en_US.UTF-8
```

**Đĩa cần:** ~50 GB cho `core-image-minimal`. Kiểm `df -h`.

---

## 3. Thẻ A — chuỗi boot bằng tay

**Hai đường, chọn theo bài đang làm:**

| Đường | Dùng cho | Vì sao |
|---|---|---|
| **① Image Debian dựng sẵn** *(khuyên dùng để bắt đầu)* | Bài **031–034** | Nhanh, có ngay chuỗi boot chạy được để **quan sát** (bước ②) và **phá** (bước ③). Không cần build gì |
| **② Build U-Boot từ nguồn** | Từ bài **035** | Bài 035 cần `CONFIG_FIT_SIGNATURE=y` + nhúng public key vào `u-boot.dtb` ⇒ **bắt buộc** tự build |

### Đường ① — image dựng sẵn
Tải image BBB từ [beagleboard.org/latest-images](https://www.beagleboard.org/distros), rồi:
```bash
lsblk                                    # XAC DINH DUNG THIET BI THE SD
sudo dd if=<image>.img of=/dev/sdX bs=4M status=progress conv=fsync
```
🔴 **`dd` nhầm ổ là xoá sạch ổ cứng.** Kiểm `lsblk` **hai lần**, và dùng `/dev/sdX` (cả thẻ), không phải `/dev/sdX1`.

### Đường ② — build U-Boot
```bash
git clone https://source.denx.de/u-boot/u-boot.git && cd u-boot
export CROSS_COMPILE=arm-linux-gnueabihf-
make am335x_evm_defconfig
make -j$(nproc)
# san pham: MLO  va  u-boot.img
```

**Bố cục thẻ:** phân vùng 1 **FAT16/32, cờ `boot`**, chứa `MLO` **rồi mới** `u-boot.img`; phân vùng 2 ext4 chứa rootfs.

---

## 4. Thẻ B — Yocto

```bash
git clone -b scarthgap git://git.yoctoproject.org/poky      # doi sang nhanh LTS hien hanh
cd poky && source oe-init-build-env
```
Sửa `conf/local.conf`:
```
MACHINE = "beaglebone-yocto"
DL_DIR    = "${TOPDIR}/../downloads"      # dung chung giua cac build
SSTATE_DIR = "${TOPDIR}/../sstate-cache"  # bai 038 do chinh cai nay
```
```bash
time bitbake core-image-minimal
ls tmp/deploy/images/beaglebone-yocto/
```

📌 `beaglebone-yocto` nằm sẵn trong **poky (`meta-yocto-bsp`)** — **không cần thêm layer** `meta-ti`. Đủ cho toàn bộ Thẻ B.

---

## 5. 🔴 Bẫy đã biết — đọc trước khi đổ lỗi cho phần mềm

**① eMMC tranh boot với thẻ SD.** BBB rev C có eMMC 4 GB và **ROM ưu tiên nó**. Không giữ nút **S2 (BOOT)** lúc cấp nguồn ⇒ board bỏ qua thẻ SD của bạn.
⚠️ **Triệu chứng giống hệt *"image mình hỏng"*** — đây là bẫy số 1 của người mới trên BBB.
- Cách tạm: **giữ S2 rồi mới cắm nguồn**, giữ tới khi thấy chữ trên serial
- Cách bền: xoá header eMMC một lần *(cân nhắc — sau đó không boot từ eMMC được nữa cho tới khi flash lại)*

**② Cáp 5V.** Làm hỏng board, không hoàn tác được. Kiểm trước khi cắm.

**③ Cấp nguồn qua USB thiếu dòng.** Board reset giữa boot ⇒ trông như kernel panic ngẫu nhiên. Dùng jack tròn 5V/2A cho mọi lab.

**④ `Permission denied` trên `/dev/ttyUSB0`.** Thiếu nhóm `dialout`, và **phải đăng xuất lại** mới có hiệu lực.

**⑤ Serial ra ký tự rác.** Sai baud. Thử `115200` trước; nếu vẫn rác thì clock/PLL sai — [boot-process.md §7](../08-embedded-systems/boot-process.md) có dòng riêng cho ca này.

**⑥ 🔴 Secure boot thật KHÔNG làm được trên BBB.** AM335x bán lẻ là silicon **GP (General Purpose)** — eFuse chưa blow ⇒ **không có chain of trust từ ROM**. Muốn vậy phải có silicon **HS**, và blow eFuse là **không thể hoàn tác**.
Làm được — và **đúng là thứ interview hỏi** — là **U-Boot verified boot** (bài [BSP-035](mock-interview/bank/bsp.md)). Nói đúng ranh giới này ở phỏng vấn là **điểm cộng**.

---

## 6. ✅ Checklist sẵn sàng — chạy trước bài BSP-031

- [ ] `groups | grep dialout` ra kết quả
- [ ] `picocom -b 115200 /dev/ttyUSB0` mở được, không lỗi quyền
- [ ] Cắm nguồn (giữ **S2**) → **thấy chữ trên serial**
- [ ] Bắt được log ra file và mở đọc lại được
- [ ] Thẻ **A boot tới `login:`** ⇒ đây là **thẻ cứu hộ**, giữ nguyên trạng
- [ ] `mkimage --version` chạy *(cho bài 035)*
- [ ] `df -h` còn **≥ 50 GB** *(cho Thẻ B)*

> ⭐ **Chưa tick đủ thì đừng bắt đầu bài 031.** Toàn bộ bộ lab dựa trên việc bạn **đọc được console** và **có một thẻ luôn boot được để so sánh**. Thiếu một trong hai thì mọi lỗi trông giống nhau, và bạn sẽ đi sửa phần mềm trong khi vấn đề nằm ở sợi cáp.

---

## 7. Sau khi chạy xong — cập nhật gì

| Nơi | Cập nhật |
|---|---|
| [bank/bsp.md](mock-interview/bank/bsp.md) | Dán **output thật** vào ô `(chua chay)` của bài vừa làm — luật bank: *cấm viết tay, cấm phỏng đoán* |
| [datalogic-plan §📍](study-plans/datalogic-plan.md) | Tick ⬜ → ✅ trong bảng tracking 8 bài |
| **File này** | Lệnh nào không khớp máy bạn thì **sửa lại tại đây**, đừng sửa trong lab |
| [gap-register](study-plans/gap-register.md) | Bài 035 xong ⇒ cập nhật ô *Secure boot* |

---
⬅️ [datalogic-plan §📍](study-plans/datalogic-plan.md) · 🧪 [Bộ lab BSP-031…038](mock-interview/bank/bsp.md)
