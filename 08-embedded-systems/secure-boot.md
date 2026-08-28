# Secure Boot & Chuỗi tin cậy — từ eFuse tới gói RPM

> **TL;DR**
> - Secure boot trả lời **một** câu: *"code sắp chạy có đúng là code chúng tôi đã phát hành không?"* — **toàn vẹn + xác thực**, **KHÔNG phải bảo mật nội dung**.
> - Cơ chế: **chuỗi tin cậy** — mỗi tầng **xác minh chữ ký** của tầng kế trước khi trao quyền. Chuỗi chỉ mạnh bằng **mắt xích đầu**, nên gốc phải **bất biến**: BootROM + khoá (hoặc hash khoá) ghi vào **eFuse/OTP**.
> - Ký = **hash nội dung** rồi **ký hash bằng khoá riêng**. Thiết bị giữ **khoá công khai** để kiểm. Không phải mã hoá.
> - ⭐ **Nó không dừng ở boot.** Sản phẩm thật còn có **dm-verity** (rootfs) và **IMA/EVM** (từng file) — đó là lý do **cài một gói RPM rời lúc phát triển cũng phải ký**.
> - Cái giá: **brick được thật**, mất khoá là mất sản phẩm, và **tốc độ phát triển giảm** — mọi thứ phải qua khâu ký.
>
> Bổ trợ [boot-process.md](boot-process.md). 🧪 Thực hành: [BSP-035](../14-prep/mock-interview/bank/bsp.md) (FIT + ký RSA trên BeagleBone Black).

---

## 1. Bản chất — nó giải quyết gì, và **không** giải quyết gì

**Câu hỏi nó trả lời:** *"Cái sắp chạy có phải thứ tôi đã phát hành không?"*

| Tính chất | Secure boot có lo? | Ghi chú |
|---|---|---|
| **Integrity** — nội dung có bị sửa? | ✅ | Hash |
| **Authenticity** — ai phát hành? | ✅ | Chữ ký bất đối xứng |
| **Confidentiality** — người khác đọc được nội dung? | ❌ **KHÔNG** | Cần **mã hoá** riêng (encrypted rootfs / firmware encryption) |
| Chống lỗi truyền dữ liệu | 🟡 phụ phẩm | Đó là việc của CRC/ECC |

> ⚠️ **Hiểu lầm phổ biến nhất:** *"secure boot mã hoá firmware nên không ai đọc được."* **Sai.** Firmware đã ký vẫn **đọc được nguyên vẹn** — ai cũng dump ra và đọc được. Nó chỉ **không sửa được mà vẫn boot**.

**Nó chống mối đe doạ nào:** kẻ tấn công **có phần cứng trong tay** — tháo flash ra ghi firmware của họ, hoặc đẩy một bản cập nhật giả. Với thiết bị **rời khỏi tầm kiểm soát** (máy quét ở cửa hàng, thiết bị đặt ngoài trời, POS), đó là mô hình đe doạ thật.

---

## 2. Chuỗi tin cậy — và vì sao gốc phải **bất biến**

```
┌─ BootROM ──────────┐  bat bien (mask ROM, khong ghi lai duoc)
│  hash(pubkey) o eFuse│  ← GOC TIN CAY (root of trust)
└─────────┬──────────┘
          │ verify chu ky
┌─────────▼──────────┐
│  SPL / TF-A BL2    │
└─────────┬──────────┘
          │ verify
┌─────────▼──────────┐
│  U-Boot / BL33     │
└─────────┬──────────┘
          │ verify (FIT signature)
┌─────────▼──────────┐
│  kernel + DTB      │
└─────────┬──────────┘
          │ dm-verity
┌─────────▼──────────┐
│  rootfs            │
└─────────┬──────────┘
          │ IMA/EVM
┌─────────▼──────────┐
│  tung file, tung   │
│  lan execve()      │
└────────────────────┘
```

**⭐ Vì sao gốc phải bất biến — lập luận phải nói được ở phỏng vấn:**

Nếu kẻ tấn công **sửa được** tầng kiểm tra đầu tiên, họ chỉ cần **tắt việc kiểm tra**, rồi mọi tầng sau chạy tự do. ⇒ Mắt xích đầu **không được nằm trên bộ nhớ ghi lại được**. Thực tế:

| Thành phần | Ở đâu | Vì sao |
|---|---|---|
| Code kiểm tra đầu tiên | **Mask ROM** trong SoC | Cố định lúc sản xuất chip, **không ghi lại được bằng phần mềm** |
| Khoá công khai (hoặc **hash** của nó) | **eFuse / OTP** | Ghi **một lần**, đứt cầu chì vật lý ⇒ không đảo ngược |
| Cờ *"bắt buộc secure boot"* | eFuse | Blow rồi thì **không tắt được nữa** |

📌 Thường eFuse chỉ chứa **hash của khoá công khai** (32 byte) chứ không chứa cả khoá (256+ byte) — tiết kiệm fuse. Firmware mang theo khoá công khai đầy đủ, ROM băm ra rồi so với fuse.

**TF-A (ARM64) — tên các pha hay xuất hiện trong JD:**
`BL1` (ROM) → `BL2` (≈ SPL, dựng DRAM) → `BL31` (**secure monitor ở EL3, ở lại runtime**, cung cấp PSCI) → `BL33` (U-Boot) → kernel. Mỗi pha verify pha sau.

### ⚠️ Verified boot ≠ Secure boot

| | **Verified boot** (ở U-Boot) | **Secure boot** (đầy đủ) |
|---|---|---|
| Gốc tin cậy | **U-Boot** — mà U-Boot thì **chưa ai kiểm** | **ROM + eFuse**, bất biến |
| Chặn được gì | Đổi kernel/rootfs | Đổi **bất cứ thứ gì**, kể cả bootloader |
| Làm được trên BBB? | ✅ ([BSP-035](../14-prep/mock-interview/bank/bsp.md)) | ❌ AM335x bán lẻ là **GP silicon**, fuse chưa blow |

> ⭐ **Câu đáng nhớ:** *"Chuỗi tin cậy chỉ mạnh bằng mắt xích đầu tiên. U-Boot verified boot kiểm kernel — nhưng nếu không ai kiểm U-Boot thì kẻ tấn công chỉ cần thay U-Boot."*

---

## 3. Ký thế nào — hash + chữ ký bất đối xứng

```
LUC BUILD (tren may co khoa RIENG)               LUC BOOT (tren thiet bi)
  image ──hash──> H                                image ──hash──> H'
                  │                                                │
       khoa RIENG ─┴─> sign ──> chu ky S            chu ky S ──┐    │
                                                    khoa CONG ─┴──> verify(S) => H
  goi kem:  [ image | S | pubkey ]                                  │
                                                          H == H' ?  ──> boot / tu choi
```

**Vì sao hash trước rồi mới ký, không ký thẳng cả image:** ký bất đối xứng (RSA/ECDSA) **rất chậm** và chỉ làm việc trên khối dữ liệu nhỏ. Hash nén image nhiều MB thành 32 byte ⇒ ký một lần trên 32 byte đó.

| Vai trò | Ai giữ | Nếu lộ |
|---|---|---|
| **Khoá riêng** (ký) | 🔴 **Chỉ hệ thống ký của công ty** — HSM / signing server | **Thảm hoạ** — ai cũng ký được firmware chạy trên mọi thiết bị đã bán |
| **Khoá công khai** (kiểm) | Nằm trong firmware + hash trong eFuse | Vô hại — công khai là đúng bản chất |

⚠️ **Mất khoá riêng còn tệ hơn lộ:** không ký được bản cập nhật nào nữa cho **toàn bộ thiết bị đã bán** — mà fuse thì không đảo ngược được.

**FIT image** là định dạng đóng gói chuẩn cho việc này: gói kernel + nhiều DTB + initramfs, **hash từng thành phần**, **chữ ký ở configuration**. Xem [BSP-004](../14-prep/mock-interview/bank/bsp.md).

---

## 4. ⭐ Ba tình huống bạn gặp ở công ty — và cơ chế đằng sau

> Đây là phần đáng giá nhất khi đi phỏng vấn: hầu hết ứng viên chỉ biết secure boot ở mức *"chuỗi verify"*. Ai kể được **secure boot ảnh hưởng tới quy trình làm việc hằng ngày thế nào** thì rõ ràng đã làm sản phẩm thật.

### 4.1 *"Build image trên máy host phải ký"*

**Vì sao:** image không có chữ ký hợp lệ thì **bootloader từ chối** ⇒ board không boot. Nên bước ký là **một bước của quy trình build**, không phải việc làm thêm.

**Vấn đề nó tạo ra:** máy build phải **chạm tới khoá riêng**. Mà khoá riêng là tài sản nguy hiểm nhất.

⇒ Cách công nghiệp giải:

| Môi trường | Khoá dùng | Ở đâu |
|---|---|---|
| Máy dev / CI nội bộ | **Khoá DEV** | File trên máy build, hoặc key server nội bộ |
| Bản phát hành | **Khoá PRODUCTION** | 🔴 **HSM / signing service** — build server **gửi hash đi ký**, không bao giờ giữ khoá |

⭐ **Điểm mấu chốt:** signing service nhận **hash**, trả về **chữ ký**. Khoá riêng **không bao giờ rời HSM**. Nói được chi tiết này cho thấy bạn hiểu vì sao quy trình phiền như vậy.

### 4.2 *"Flash image lên thiết bị"*

Có **hai lớp kiểm** khác nhau, và người ta hay nhầm:

| Lớp | Kiểm khi nào | Hỏng thì thấy gì |
|---|---|---|
| **Công cụ flash** kiểm trước khi ghi | Lúc ghi | Tool báo lỗi, **chưa ghi gì** — an toàn |
| **Bootloader** kiểm lúc boot | Lần bật kế tiếp | 🔴 Đã ghi rồi mới phát hiện ⇒ **có thể brick** nếu không có phân vùng dự phòng |

⚠️ Đây là lý do sản phẩm nghiêm túc dùng **A/B partition**: ghi vào slot không dùng, boot thử; hỏng thì bootloader tự quay về slot cũ ([BSP-015](../14-prep/mock-interview/bank/bsp.md)).

### 4.3 🎯 *"Build một gói RPM rời để cài lúc phát triển — cũng phải ký"*

Đây là chỗ khó hiểu nhất, và câu trả lời là: **secure boot không dừng ở kernel.**

Boot xong thì rootfs mới bắt đầu chạy — nếu không ai kiểm rootfs thì kẻ tấn công chỉ cần sửa một file trong đó. Nên sản phẩm thật có thêm **hai cơ chế runtime**, và chúng là **hai thứ khác nhau**:

| | **dm-verity** | **IMA / EVM** |
|---|---|---|
| Bảo vệ cái gì | **Cả phân vùng** rootfs | **Từng file** riêng lẻ |
| Cơ chế | Cây hash (Merkle tree); **hash gốc được ký** | Chữ ký nằm trong **xattr** `security.ima` của file |
| Kiểm khi nào | **Mỗi lần đọc block** | Mỗi lần `execve()` / `open()` |
| Rootfs ghi được không | ❌ **Chỉ đọc** — sửa một byte là hỏng cả cây | ✅ ghi được, nhưng file mới phải **có chữ ký hợp lệ** |
| ⇒ Cài gói rời | **Không thể** — phải build lại cả image | ✅ **Được** — nếu gói mang file **đã ký** |

**⇒ Vì sao RPM của bạn phải ký — và ký ở HAI tầng khác nhau:**

| Tầng | Cái gì được ký | Ai kiểm | Lúc nào | Chặn được gì |
|---|---|---|---|---|
| **Chữ ký RPM** (GPG) | **Cả file `.rpm`** | `rpm` / package manager | Lúc **cài** | Gói giả từ kênh phân phối |
| ⭐ **Chữ ký IMA** | **Từng file bên trong** gói | **Kernel** | Mỗi lần **chạy file đó** | Ai đó chép file lạ vào máy sau khi đã cài |

⚠️ **Hai tầng này độc lập.** Gói ký GPG đúng nhưng file bên trong **không có chữ ký IMA** ⇒ `rpm` cài xong không lỗi gì, nhưng lúc chạy binary thì **kernel từ chối `execve()`** — triệu chứng là *"Permission denied"* trên một file rõ ràng có quyền `x`. Đây là lớp lỗi rất dễ mất thời gian nếu không biết IMA đang bật.

**⇒ Nên máy build phải ký từng file lúc đóng gói** — đó chính là lý do khâu build gói lẻ của bạn cũng cần khoá.

**Chẩn đoán khi gặp:**
```bash
getfattr -m . -d /usr/bin/myapp        # co security.ima chua?
dmesg | grep -i "ima\|appraise"        # kernel tu choi vi sao
cat /sys/kernel/security/ima/policy    # chinh sach dang ap
rpm -K mypackage.rpm                   # chu ky GPG cua goi co hop le khong
```

---

## 5. Khoá DEV vs khoá PRODUCTION — vì sao không lẫn được

| | Thiết bị **DEV** | Thiết bị **PRODUCTION** |
|---|---|---|
| eFuse | Chưa blow, hoặc blow **khoá dev** | Blow **khoá production** |
| Chấp nhận build nào | Ký bằng khoá dev | **Chỉ** ký bằng khoá production |
| Bật/tắt secure boot | Thường tắt được | 🔴 **Không** — fuse không đảo ngược |
| dm-verity / IMA | Thường **tắt** để dev nhanh | Bật |

⇒ Hệ quả thực tế bạn đã gặp: **build dev không chạy trên máy production**, và ngược lại. Không phải "cấu hình sai" — mà là **hai gốc tin cậy khác nhau**, đúng như thiết kế.

📌 **Vì sao vẫn phải có thiết bị production để test:** nếu chỉ test trên máy dev (verity/IMA tắt) thì **đúng lớp lỗi mà secure boot gây ra sẽ không bao giờ lộ ra** trước khi xuất xưởng.

---

## 6. Anti-rollback — chữ ký hợp lệ vẫn có thể là tấn công

Kẻ tấn công lấy **firmware CŨ, ký thật, hợp lệ** — nhưng có CVE đã biết — rồi flash đè. Chữ ký kiểm **đạt**, vì nó thật.

⇒ Cần thêm **số phiên bản đơn điệu**: firmware mang `version`, thiết bị lưu ngưỡng trong **fuse/counter**, và **từ chối mọi bản thấp hơn**.

⚠️ **Cái giá:** tăng counter là **không đảo ngược** ⇒ phát hành một bản có bug rồi tăng version thì **không quay lại bản cũ được nữa**. Nên thực tế người ta chỉ tăng counter khi vá **lỗ hổng bảo mật thật**, không tăng theo mỗi bản build.

---

## 7. ⚠️ Bẫy & cái giá thực chiến

**① Nhầm secure boot với mã hoá.** Ký ≠ giấu. Firmware đã ký vẫn đọc được toàn bộ (§1).

**② Nhầm *verified boot* với *secure boot*.** Không neo ở ROM/eFuse thì kẻ tấn công chỉ cần thay chính tầng đi kiểm (§2).

**③ Blow fuse nhầm.** **Không đảo ngược được.** Blow khoá sai, hoặc bật cờ secure boot khi chưa có build ký đúng ⇒ **brick vĩnh viễn**. Đây là lý do quy trình blow fuse luôn tách khỏi quy trình build.

**④ Mất khoá riêng.** Tệ hơn lộ khoá: **không cập nhật được cho toàn bộ thiết bị đã bán**.

**⑤ Không có A/B partition.** Ghi một image hỏng chữ ký ⇒ boot không lên, không còn đường quay lại (§4.2).

**⑥ Chỉ test trên máy dev.** Verity/IMA tắt ⇒ lớp lỗi do chúng gây ra không lộ ra trước khi xuất xưởng (§5).

**⑦ Cài file bằng tay lên máy có IMA.** `scp` một binary lên rồi chạy ⇒ **`Permission denied`** dù `chmod +x` đủ — vì thiếu chữ ký trong xattr, không phải vì quyền file (§4.3).

**⑧ Quên chữ ký là một khâu tốn thời gian.** Mỗi lần build phải ký; ký production phải qua HSM/dịch vụ ⇒ **vòng lặp phát triển chậm hẳn**. Đây là cái giá thật, và là lý do môi trường dev dùng khoá riêng của mình.

**⑨ Tưởng secure boot chống được kẻ có quyền root.** Không — nó chống **thay đổi code khởi động và code chạy**. Kẻ đã có root trên hệ đang chạy là mô hình đe doạ khác (đó là việc của IMA/EVM, MAC, và giảm bề mặt tấn công).

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [BSP-026](../14-prep/mock-interview/bank/bsp.md) | Secure boot hoạt động thế nào? Vì sao mọi package phải được ký? |
| [BSP-039](../14-prep/mock-interview/bank/bsp.md) | 🎯 Vì sao build một gói RPM rời lúc phát triển cũng phải ký? |
| [BSP-004](../14-prep/mock-interview/bank/bsp.md) | FIT image là gì, hơn uImage cũ chỗ nào? |
| [BSP-015](../14-prep/mock-interview/bank/bsp.md) | Thiết kế OTA an toàn (A/B partition, rollback) |

🧪 **Thực hành:** [BSP-035](../14-prep/mock-interview/bank/bsp.md) — FIT + ký RSA, làm U-Boot **từ chối** image đã sửa một byte. Setup: [lab-setup.md](../14-prep/lab-setup.md).

📖 Đọc thêm: [melp/bootloader-kernel](../15-book-summaries/melp/bootloader-kernel.md) (TF-A BL1→BL33), [melp/storage-update](../15-book-summaries/melp/storage-update.md) (OTA).

---
⬅️ [boot-process.md](boot-process.md) · ➡️ [08-embedded-systems](README.md)
