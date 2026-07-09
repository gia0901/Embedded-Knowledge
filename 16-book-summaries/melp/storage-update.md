# MELP — Storage trên Flash & Cập nhật ngoài hiện trường (ch. 9–10) 🎯🎯

> Thuộc [MELP](README.md). **Nguồn:** kiến thức Claude, chưa đối chiếu PDF.
> Cụm "sống còn" thứ hai của BSP: chọn sai storage/update là sản phẩm chết ngoài hiện trường — interviewer BSP giàu kinh nghiệm gần như chắc chắn hỏi mảng này.

---

## Cụm 1 — Storage: NOR/NAND thô, MTD/UBI vs eMMC (ch. 9) 🎯

### Nội dung chính

**Phân loại phần cứng — quyết định toàn bộ software stack phía trên:**

| | NOR flash | NAND thô (raw) | eMMC / SD |
|---|---|---|---|
| Đặc tính | Nhỏ (MB), đắt/bit, **XIP** (chạy code trực tiếp), tin cậy | Rẻ/bit, dung lượng lớn, **có bad block + bit flip** (cần ECC) | NAND + **controller FTL tích hợp** — trình diện block device |
| Ai quản lý lỗi/mòn | Gần như không cần | **Phần mềm phía host** (kernel) | Controller lo (hộp đen) |
| Stack phần mềm | MTD → JFFS2/UBI | MTD → **UBI → UBIFS** | Block → ext4/f2fs |
| Dùng cho | Bootloader, env, config nhỏ | Thiết bị tối ưu giá | Đại đa số thiết bị hiện đại |

**MTD (Memory Technology Device)** = lớp trừu tượng của kernel cho flash **thô** (không phải block device! — erase block lớn, ghi phải erase trước, có bad block): phân vùng khai trong **DT** (hoặc cmdline `mtdparts=`), lộ ra `/dev/mtdN` (char) + `/dev/mtdblockN`.

**UBI/UBIFS** — cặp chuẩn cho NAND thô:
- **UBI** (ở trên MTD, dưới FS): quản lý **wear leveling toàn partition**, **bad block**, ánh xạ *logical erase block → physical* (giống LVM cho flash); chia **volume**.
- **UBIFS** (trên UBI): filesystem journaling, nén, power-cut tolerant — thay thế JFFS2 (mount chậm, scale kém).
- Phân biệt hay bị hỏi: **JFFS2/UBIFS chạy trên flash thô; ext4/f2fs chạy trên block device (eMMC)** — đặt UBIFS lên eMMC hay ext4 lên NAND thô đều là **sai loại** (eMMC giấu mất geometry flash; NAND thô không có FTL cho block FS).
- **f2fs**: FS thiết kế cho FTL-device (eMMC/SD) — ghi kiểu log-structured hợp FTL ([OSTEP persistence](../ostep/persistence.md) cụm LFS/FTL là nền lý thuyết).

**eMMC — chi tiết BSP cần biết:** ngoài user area còn **2 boot partition** phần cứng (`/dev/mmcblkNboot0/1` — ROM code nhiều SoC đọc SPL/bootloader từ đây) + **RPMB** (replay-protected — lưu secure data); reliable write; `mmc` utils đọc health/life-time estimate. SD card công nghiệp vs tiêu dùng: khác biệt thật về power-loss và endurance.

**Ba nguyên tắc thiết kế partition của sách:** (1) tách **hệ điều hành (read-only)** khỏi **dữ liệu (read-write)** — rootfs mount **RO** + partition data riêng (RO rootfs = không thể hỏng do power cut, tiền đề update sạch); (2) bootloader/env ở vùng ổn định nhất; (3) chừa chỗ cho update (cụm 2).

### Insight đáng nhớ

- Câu hỏi định vị nhanh mọi bài toán storage nhúng: **"FTL nằm ở đâu — trong chip (eMMC) hay trong kernel (UBI)?"** Trả lời được là chọn đúng cả stack: FS nào, tool nào, ai lo wear/bad block, tin ai khi mất điện.
- **Rootfs read-only** là "một mũi tên ba đích": sống sót power-cut, update atomic dễ (swap cả partition), bảo mật (immutable). Squashfs cho rootfs RO + overlayfs cho vùng ghi là combo phổ biến đáng kể tên.

### Ít quan trọng

- Chi tiết lệnh `flash_erase`, `nandwrite`, `ubiformat`, tham số tạo UBIFS image (`mkfs.ubifs -m -e -c`) — tra khi làm (ch. 9 giữa); các FS lịch sử (YAFFS2).

### Góc interview

**Câu 1 (🎯):** Sản phẩm mới: chọn giữa NAND thô + UBIFS và eMMC + ext4/f2fs. Trình bày trade-off và stack phần mềm mỗi bên.

<details><summary>Đáp án</summary>

- **eMMC + ext4/f2fs** (mặc định hiện đại): FTL trong chip lo wear/bad block/ECC → phần mềm đơn giản (block device chuẩn, tool chuẩn, fsck chuẩn, update dễ — dd cả partition được); mua "sự yên tâm đóng gói". Trade-off: FTL là **hộp đen** (không audit được hành vi power-cut — phải test rút điện + chọn hãng có power-loss protection), đắt hơn NAND thô một chút, hết đời chip là thay (không can thiệp được).
- **NAND thô + MTD/UBI/UBIFS**: rẻ nhất trên mỗi GB ở dung lượng nhỏ, **kiểm soát trọn** hành vi flash (wear, bad block, ECC — audit được power-cut từng lớp), phù hợp khối lượng lớn tối ưu giá. Trade-off: stack phức tạp (bring-up ECC/timing NAND controller, UBI attach, ảnh factory flash riêng công cụ), đội ngũ phải hiểu flash thật, ít linh kiện NAND SLC dung lượng nhỏ dần theo thị trường.
- Stack: NAND: `NAND controller (ECC) → MTD → UBI (wear/bad block) → UBIFS`; eMMC: `eMMC controller → block layer → ext4 (data=ordered) hoặc f2fs`.
- Chốt kiểu senior: sản lượng nhỏ/vừa + thời gian phát triển ngắn → eMMC; BOM cực nhạy giá + dung lượng nhỏ (≤512MB) + đội có kinh nghiệm flash → NAND thô. Và dù chọn gì: **rootfs read-only + test rút điện tự động** không thương lượng.

</details>

**Câu 2:** Vì sao không đặt ext4 lên NAND thô, hay UBIFS lên eMMC?

<details><summary>Đáp án</summary>

- **ext4 lên NAND thô**: ext4 giả định **block device** ghi-đè-tại-chỗ được, không biết erase block/bad block/bit flip — NAND thô không cho ghi đè (phải erase), có block hỏng xuất xưởng và phát sinh, có bit flip cần ECC + scrubbing. Thiếu FTL ở giữa thì ext4 hỏng ngay từ nguyên lý (mtdblock chỉ là shim đọc-chủ-yếu, không wear leveling — dùng cho ghi thật là phá flash).
- **UBIFS lên eMMC**: UBIFS cần thấy **flash thô qua MTD/UBI** (erase block, thông số NAND) để tự quản wear/ECC — eMMC đã **giấu tất cả sau FTL**, trình diện block 512B; không có MTD ở đây (về kỹ thuật không attach được), và nếu ép qua lớp giả lập thì thành **hai tầng wear leveling chồng nhau** — vô ích và phản tác dụng (write amplification ×2 tầng).
- Quy tắc một dòng: **FS flash-aware (JFFS2/UBIFS) đi với flash thô; FS block (ext4/f2fs) đi với thiết bị có FTL** — f2fs là "block FS *hiểu ý* FTL", điểm giữa tốt cho eMMC.

</details>

---

## Cụm 2 — Software Update ngoài hiện trường: A/B, atomic, chống brick (ch. 10) 🎯🎯

### Nội dung chính

**Bài toán:** thiết bị ở nơi không ai chạm tới, update **có thể mất điện/mất mạng giữa chừng, ảnh có thể hỏng, phiên bản mới có thể có bug** — mọi kịch bản đều **không được tạo ra cục gạch (brick)**. Yêu cầu rút thành 3 chữ: **atomic** (chạy bản cũ hoặc bản mới trọn vẹn, không bao giờ nửa nạc nửa mỡ), **robust** (fail thì tự lành), **secure** (chỉ nhận ảnh có chữ ký).

**Các kiến trúc update:**

| Kiến trúc | Cách hoạt động | Trade-off |
|---|---|---|
| **A/B (symmetric, dual bank)** 🎯 | 2 bộ partition hệ thống (rootfs A/B, có thể cả kernel); chạy A → ghi bản mới vào B → đổi cờ boot → reboot vào B; hỏng → **rollback về A** | Chuẩn vàng độ bền; giá = **×2 dung lượng** hệ điều hành |
| Recovery (asymmetric) | Partition recovery tối giản riêng; update chính bằng cách boot vào recovery rồi ghi đè main | Tiết kiệm chỗ; lúc đang ghi main **không có bản chạy dự phòng** — recovery hỏng là hết |
| Atomic file-based (OSTree/Nix kiểu) | FS immutable + chuyển đổi bằng đổi con trỏ deployment | Tiết kiệm (delta theo file), rollback tốt; stack phức tạp hơn |
| Container/app-level | Chỉ update tầng app (docker, package) | Không cứu được kernel/rootfs — bổ trợ, không thay thế |

**Chuỗi A/B chống brick — từng mắt xích đều bắt buộc:**

```
1. Tải ảnh (đầy đủ hoặc delta) → VERIFY chữ ký + checksum TRƯỚC KHI GHI
2. Ghi vào bank KHÔNG chạy (B) → verify lại sau ghi (read-back)
3. Đặt cờ "thử B, còn N lần" (bootcount) trong U-Boot env/biến riêng
4. Reboot → U-Boot: đọc cờ → boot B, GIẢM bootcount
5. Hệ B lên tốt → userspace "chốt đơn": xóa cờ thử, B thành chính thức
   Hệ B chết trước khi chốt → bootcount về 0 → U-Boot TỰ boot lại A (rollback)
6. WATCHDOG phần cứng chạy suốt: kernel treo cứng (không kịp làm gì)
   → reset → quay lại vòng 4/5 → hết lượt → về A
```

- **Bootloader là mắt xích rollback** — U-Boot có sẵn cơ chế `bootcount`; logic chọn bank viết trong env/script. Bootloader bản thân **hiếm khi được update** (brick không cứu nổi) — nếu buộc phải, cần cơ chế riêng của SoC (dual boot partition eMMC + ROM fallback).
- **"Lên tốt" phải định nghĩa được** — không phải "kernel boot xong" mà "dịch vụ chủ chốt chạy + tự kiểm tra pass" rồi mới chốt đơn (health check trước khi commit).
- **Các framework nên biết tên:** **SWUpdate** (linh hoạt, script Lua, format .swu), **RAUC** (A/B chặt chẽ, bundle ký, tích hợp Yocto/systemd tốt), **Mender** (kèm server quản lý fleet OTA), OSTree (file-based). Chọn framework có sẵn thay vì tự viết — người tự viết thường quên đúng những mắt xích trên.
- **Secure update**: ảnh ký (chuỗi tin từ secure boot kéo dài lên update), chống **rollback attack** (version counter — không cho cài lại bản cũ có lỗ hổng), TLS cho kênh tải.
- Dữ liệu người dùng nằm **ngoài** bank A/B (partition data riêng) + migration schema có version — update OS không đụng data.

### Insight đáng nhớ

- **Update = journaling phóng to cấp hệ thống**: ghi bản mới ra chỗ riêng (journal) → commit bằng một hành động atomic (đổi cờ boot) → rollback nếu chưa commit. Nhận ra nó cùng khuôn với [journaling FS](../ostep/persistence.md) và "write temp + rename" là mức hiểu nguyên lý interviewer tìm kiếm.
- Câu thần chú thiết kế: **"đường lành phải là đường mặc định"** — mọi thứ (bootcount, watchdog, health check) sắp đặt sao cho *không làm gì thêm* thì hệ tự quay về trạng thái chạy được; con người/mạng chỉ cần thiết cho lần thử tiếp, không cần cho sự sống của thiết bị.

### Ít quan trọng

- Chi tiết cấu hình từng framework (recipe Mender/RAUC trong Yocto, cú pháp sw-description của SWUpdate) — tra tài liệu framework khi triển khai (ch. 10 giữa-cuối); local update qua USB/SD như biến thể đơn giản.

### Góc interview

**Câu 1 (🎯 gần như chắc gặp):** Thiết kế cơ chế OTA update cho thiết bị nhúng không được phép brick. Trình bày đầy đủ.

<details><summary>Đáp án</summary>

Trình bày theo 4 lớp:
1. **Bố cục storage:** bootloader + env (vùng ổn định, gần như không update) | kernel+rootfs **A** | kernel+rootfs **B** | data (user, ngoài A/B). Rootfs **read-only** (squashfs/ext4 RO) — bản chạy không tự hỏng.
2. **Luồng update:** tải ảnh (resume được) → **verify chữ ký trước khi ghi** → ghi vào bank không chạy → read-back verify → set cờ "boot thử B, tối đa N lần" → reboot.
3. **Cơ chế rollback tự động:** U-Boot đọc cờ + **bootcount** (giảm mỗi lần boot thử); hệ mới phải vượt **health check** (dịch vụ chủ chốt lên, tự test pass) rồi userspace mới **commit** (xóa cờ thử). Chết ở bất kỳ đâu trước commit → hết N lần → U-Boot boot lại bank cũ. **Watchdog phần cứng** phủ kịch bản treo cứng (không code nào chạy được để cứu): treo → reset → bootcount làm việc của nó.
4. **Bảo mật & vận hành:** chuỗi ký từ secure boot lên ảnh update; **anti-rollback** version counter; data schema migration có version (và đường lùi nếu rollback); triển khai fleet theo vòng (canary → 10% → toàn bộ) với khả năng dừng từ server; dùng framework sẵn (RAUC/Mender/SWUpdate) thay vì tự chế.
- **Bẫy khi trả lời:** quên watchdog (health check bằng phần mềm không cứu được kernel panic sớm/treo cứng); quên định nghĩa "boot thành công" (commit ngay khi kernel lên = rollback vô dụng với bug ở service); verify *sau khi* ghi mà không verify *trước* (ghi ảnh rác đè bank dự phòng — mất luôn đường lùi thứ hai); để user data trong bank A/B.

</details>

**Câu 2:** Update đang ghi thì mất điện — chuyện gì xảy ra trong thiết kế A/B, và trong thiết kế ghi-đè-một-bank? Vì sao bootloader hiếm khi được update?

<details><summary>Đáp án</summary>

- **A/B:** mất điện lúc ghi bank B — bank A **nguyên vẹn và vẫn là bank boot** (cờ chỉ đổi *sau khi* ghi + verify xong). Boot lại → chạy A như thường, agent update phát hiện dở dang → ghi lại từ đầu. Người dùng không cảm nhận gì. Đây chính là tính atomic: hành động commit là *một lần ghi cờ nhỏ* (env có redundant copy — U-Boot ghi env hai bản chống chính nó dở dang), không phải quá trình dài.
- **Ghi đè một bank:** mất điện giữa chừng = bank duy nhất **nửa cũ nửa mới** → không boot được → brick, trừ khi có recovery partition (và recovery chính là "bank B thu nhỏ" — quay về ý tưởng A/B).
- **Bootloader hiếm được update** vì nó là **mắt xích không có ai đứng sau**: ghi dở SPL/U-Boot là ROM code không còn gì hợp lệ để nạp — brick sâu (cứu bằng đường xưởng: UART/USB boot mode). Muốn update buộc dùng cơ chế phần cứng: eMMC có **hai boot partition** (ghi bản mới vào partition boot không active rồi đổi qua `mmc bootpart`), hoặc SoC hỗ trợ ảnh dự phòng/fallback trong ROM. Nguyên tắc thiết kế: dồn mọi thứ hay đổi lên tầng trên (kernel/rootfs), giữ bootloader tối giản và đóng băng.

</details>

### Đọc thêm (tùy chọn)

- [OSTEP persistence](../ostep/persistence.md) — FTL, write amplification, journaling: nền lý thuyết của toàn bộ file này.
- [08/constraints.md](../../08-embedded-systems/constraints.md) — ràng buộc power/flash của thiết bị nhúng.
