# Phiên mock — 2026-08-13 · rapid · track linux-sysprog **(phần 2 — hoàn tất Buổi 1)**

- **Level:** mid-level · **Số câu:** 9 (câu 4→12) · **Trần độ sâu: T2**
- **Điểm phần này:** **2.67 / 4** · **Cả Buổi 1 (12 câu): 2.67 / 4**
- **Bối cảnh:** Datalogic plan — **Tuần 2 · Buổi 1**, phần tiếp của [phần 1](2026-08-13--rapid--linux-sysprog.md) (3 câu đầu). Buổi 1 **hoàn tất** ở đây.
- **Đặc điểm phiên:** phân bố **rất lệch** — 2 câu 4đ, 4 câu 3đ, 1 câu 1đ, 1 câu 0đ. Không phải phiên "đều đều 3".

## Kết quả từng câu (nhìn nhanh)

| # | ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|---|----|---------------|------|--------------|
| 4 | LNX-012 | fd rò qua `exec` → `EADDRINUSE` | **4** | Tự nêu race `socket()`+`fcntl()` đa luồng. Thiếu mỗi tên `TIME_WAIT` (cơ chế đã mô tả đúng → không giữ điểm) |
| 5 | LNX-007 | mất điện sau "Đã lưu" → config 0 byte | **3** | Chỉ đúng thủ phạm là cờ `'w'`. Thiếu mẫu `tmp+fsync+rename+fsync(dir)`; đề xuất append (sai cho config) |
| 6 | LNX-022 | `D` state, load cao mà CPU rảnh | **3** | (a)(b) chuẩn, (e) đúng (page cache). Hụt: không ra được **bước tiếp theo** / câu hỏi cho khách |
| 7 | LNX-029 | timeout dùng `time()` — 2 triệu chứng ngược | **1** ⚠️ | Trắng, thu hẹp mới ra epoch + 1 triệu chứng. Không biết `CLOCK_MONOTONIC`/`steady_clock` |
| 8 | LNX-008/015 | IPC cho 60 MB/s ảnh | **3** | (a) chọn shm + đánh đổi rất chắc. Giả định SAI *"1 chết không ảnh hưởng cái kia"* → stale lock; chỉ ra được khi bị đặt tình huống |
| 9 | LNX-023 | `/proc` & `/sys` | **0** ⚠️ | Trắng cả 2 lần hỏi |
| 10 | LNX-020 | pipe không thấy EOF | **3** | (a)(b)(c) đúng gọn, tự nối về câu 4. Không biết `SIGPIPE` giết process mặc định |
| 11 | LNX-004 | zombie tích tụ 3 tuần | **3** | Chỉ đúng **PID cạn**; 2 cách sửa có đánh đổi. Thiếu *vì sao* phải lặp (signal không xếp hàng) + cách `SIG_IGN` |
| 12 | LNX-017 | message queue đầy | **4** | Phân biệt đúng **dữ liệu trạng thái vs sự kiện** cả hai chiều |

---

## ⚖️ Phân định phản hồi *"tài liệu thiếu"* (ứng viên nêu 2 lần trong phiên)

> Theo [config §6 lan can](../config.md): phải phân định từng ý, không gật đại cũng không bảo vệ câu hỏi. Đã grep toàn repo.

| Ý | Phán định | Bằng chứng |
|---|---|---|
| `CLOCK_MONOTONIC` / NTP không có tài liệu | ✅ **ĐÚNG — lỗi thật của repo** | Bank LNX-029 ghi `→ file-io`, nhưng [file-io.md](../../../04-linux-system-programming/file-io.md) **không có mục nào về clock** (mục lục: fd · syscall I/O · atomicity · stdio · blocking · dup2 · fork/exec · hiệu năng). Nội dung thật ở [TLPI cụm 03](../../../15-book-summaries/the-linux-programming-interface/03-signals-and-timers.md) — không nằm trong lịch đọc Tuần 2 |
| `/proc`, `/sys` không có tài liệu | ✅ **ĐÚNG — cùng lỗi** | Bank LNX-023 cũng ghi `→ file-io`; file-io.md không có mục nào. Nội dung rải ở [09-debugging/tools.md](../../../09-debugging/tools.md), [05-drivers-device-tree](../../../05-drivers-device-tree/) |
| `TIME_WAIT`/`SO_REUSEADDR` không có tài liệu | ❌ **KHÔNG đúng** — nhưng ngoài phạm vi đọc | Có ở [sockets-and-protocols.md:56](../../../13-networking/sockets-and-protocols.md) và [tcp-ip.md:67](../../../13-networking/tcp-ip.md). Thuộc topic 13, không nằm trong lịch Tuần 2 → hợp lý khi chưa đọc |

⇒ **Hai việc phải vá repo** (cộng lỗ hổng TCP-framing từ phần 1 là **ba**), đã ghi vào mục "Nợ" của [datalogic-plan §📍](../../study-plans/datalogic-plan.md).

---

## 🔎 Chi tiết ôn

### Câu 4 · LNX-012 · 🟠 · **4/4** ⭐

Daemon mở socket lắng nghe cổng 9100, mỗi khi có job in thì `fork()` + `exec()` một script xử lý ảnh (`/usr/bin/convert`). Script chạy vài giây rồi thoát.

Một hôm bạn `systemctl restart scanner-daemon`. Daemon chết sạch, nhưng process mới **không bind được cổng 9100**: `EADDRINUSE`. `ss -ltnp` cho thấy cổng vẫn bị giữ — bởi một process `convert` đang chạy dở.

**(a)** Vì sao `convert` — chương trình không liên quan, không biết tới socket của bạn — lại đang giữ cổng 9100?
**(b)** Sửa bằng gì, đặt ở đâu?

**🔁 (c)** Nếu viết `int fd = socket(...); fcntl(fd, F_SETFD, FD_CLOEXEC);` — hai dòng liền nhau — có tương đương không? Hỏng trong điều kiện nào?
**🔁 (d)** Đồng nghiệp bảo *"thêm `SO_REUSEADDR` là xong"*. Phản biện thế nào?
**🔁 (e)** `SO_REUSEADDR` sinh ra để giải quyết chính xác **trạng thái nào** của socket?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ *"fork thì con kế thừa bảng fd, bao gồm cả socket. Khi restart daemon thì socket fd của daemon được đóng, nhưng convert vẫn còn sống và giữ fd nên socket không thể bị giải phóng."*
- (b) ✅ `O_CLOEXEC` **ngay khi tạo socket**.
- (c) ✅ **Tự nêu race, không cần gợi ý:** *"Nếu process hiện tại có thread chạy riêng thực hiện `fork()` ngay tại thời điểm `socket()` vừa xong và fd chưa được set CLOEXEC, thì lỗi trên vẫn xảy ra như thường."*
- (d) ✅ *"Như trường hợp trên, `SO_REUSEADDR` sẽ báo lỗi ngay vì addr cũ vẫn đang được sử dụng"* — đúng, nó vô dụng khi có **process sống** đang giữ socket.
- (e) 🟡 Không gọi được tên `TIME_WAIT`, nhưng **mô tả đúng cơ chế**: *"khi close socket, các thông tin cũ vẫn chưa được nhả ra liền, có thể mất thời gian"*.

**✅ Vì sao 4 điểm:** đủ cả T1 (bảng fd, kế thừa) lẫn T2 (race đa luồng ở (c), phản biện `SO_REUSEADDR` ở (d)). Câu (c) là mức **trên mid** — đó đúng là lý do POSIX đẻ ra `O_CLOEXEC`, `accept4()`, `pipe2()`, `epoll_create1()`.

Theo [config §4](../config.md): **không giữ điểm vì thiếu một cái tên khi cơ chế đã đúng.** Thiếu chữ `TIME_WAIT` không hạ điểm.

**Bổ sung cần nhớ:** cổng chỉ thực sự giải phóng khi **tham chiếu cuối cùng** tới socket đóng — daemon chết không đủ nếu con cháu còn cầm fd. Và `TIME_WAIT`: socket bên **chủ động đóng trước** ở trạng thái này ~60 s để gói trễ của kết nối cũ không lẫn sang kết nối mới; `SO_REUSEADDR` cho phép `bind()` đè lên đúng ca đó.

**Trích [file-io.md §6](../../../04-linux-system-programming/file-io.md):**
> Trong chương trình **đa luồng**, còn phải dùng `O_CLOEXEC` **ngay trong `open()`** thay vì `fcntl` sau đó, vì giữa hai lời gọi có thể có thread khác `fork`.

**Chốt:** *"`fork` copy bảng fd, `exec` giữ nguyên — trừ `O_CLOEXEC`. Cứ mở fd là đặt cờ đó ngay trong lời gọi tạo, không phải bước sau."*

**Lần sau sẽ hỏi:** góc mới — *"daemon cần CỐ Ý truyền một fd sang chương trình con (vd socket đã accept). Làm sao, và làm sao con biết fd số mấy?"* (systemd socket activation / `LISTEN_FDS`, hoặc truyền qua `SCM_RIGHTS`).

**Ôn:** [file-io.md §6](../../../04-linux-system-programming/file-io.md) · bank [LNX-012](../bank/linux-sysprog.md) · [sockets-and-protocols.md](../../../13-networking/sockets-and-protocols.md)
</details>

---

### Câu 5 · LNX-007 · 🟡 · **3/4**

Thiết bị đọc mã vạch cầm tay, chạy Linux, **không có pin phụ** — tháo pin là mất điện tức thì.

```c
FILE* f = fopen("/data/config.json", "w");
fprintf(f, "%s", json_text);
fclose(f);
sync_ui_ok();                       // báo "Đã lưu" lên màn hình
```

Khách báo: thỉnh thoảng tháo pin ngay sau khi thấy chữ "Đã lưu", bật lại thì file config **rỗng 0 byte** — thiết bị về default, mất hết cấu hình.

**(a)** Dữ liệu đang **nằm ở đâu** lúc màn hình hiện "Đã lưu"? Kể các tầng nó phải đi qua.
**(b)** Cần thêm gì để chữ "Đã lưu" là thật?

**🔁 (c)** `fclose()` đã tự flush stdio rồi. Vậy tầng nào mới là **thủ phạm thật**?
**🔁 (d)** Vì sao file **rỗng 0 byte** chứ không phải giữ nội dung cũ? Mất điện đúng lúc `fsync()` chạy dở thì sao — thiết kế thế nào để **không bao giờ** mất bản cũ?
**🔁 (e)** Nếu ghi **nối đuôi**, file sau hai lần lưu trông thế nào — parser còn đọc được không?
**🔁 (f)** Có cách nào **thay thế nguyên tử cả file** không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ Đủ ba tầng: buffer stdio (libc) → syscall `write` → page cache của kernel → đĩa. Nêu đúng lý do gom buffer (ít syscall, ít context switch, đỡ hao phần cứng).
- (b) ✅ `fflush()` + `fsync()`.
- (c) ✅ **Rất tốt** — trả lời ngay: *"thủ phạm thực sự là cờ `'w'` của `fopen`"*.
- (d) ✅ Giải thích đúng cơ chế 0 byte (`'w'` xoá sạch nội dung cũ trước khi ghi). ❌ Nhưng đề xuất **ghi nối đuôi** để bảo toàn bản cũ.
- (e) ✅ Tự nhận ra append phá format: *"Không đọc được, không còn đúng format chuẩn."*
- (f) ❌ **"Chưa rõ."**

**✅ Được:** ba tầng buffering nắm chắc, và pha (c) là chỗ phần lớn người bị hỏi sẽ đi lạc vào page cache — bạn chỉ thẳng cờ `'w'`. Bản năng *"không bao giờ phá bản cũ"* ở (d) cũng đúng hướng, chỉ chọn sai công cụ.

**❌ Vì sao chưa 4:** thiếu **mẫu thay thế nguyên tử** — thứ duy nhất thực sự chữa được bug này.

**Đo thật** (`g++ -std=c++17 -Wall -Wextra`):
```
ban cu tren dia                    -> 29 byte
ngay sau fopen("w"), CHUA ghi gi   -> 0 byte  <-- mat dien o day = mat sach
da fputs nhung CHUA fclose         -> 0 byte  (con nam trong buffer stdio)
sau fclose                         -> 29 byte  (moi xuong page cache)
```
Cửa sổ chết nằm giữa dòng 2 và dòng 4: bản cũ **đã mất**, bản mới **chưa có**.

**Mẫu đúng — atomic replace via rename:**
```c
int fd = open("/data/config.json.tmp", O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC, 0644);
write_all(fd, json, len);
fsync(fd);                                              // ① nội dung MỚI chắc chắn trên đĩa
close(fd);
rename("/data/config.json.tmp", "/data/config.json");   // ② thay thế NGUYÊN TỬ
int dfd = open("/data", O_RDONLY|O_DIRECTORY);
fsync(dfd);                                             // ③ ép cả thao tác ĐỔI TÊN xuống đĩa
close(dfd);
```

**Vì sao đúng ở mọi thời điểm mất điện:** `rename()` trong cùng filesystem là **nguyên tử** — một cái tên chỉ trỏ tới inode cũ **hoặc** inode mới, không có trạng thái ở giữa. Mất điện trước ② ⇒ còn nguyên bản cũ; sau ② ⇒ có bản mới đầy đủ.

**Ba chi tiết hay bị bỏ:**
1. **Thiếu ① thì ② vô nghĩa** — đổi tên vào một file mà nội dung còn nằm trong page cache.
2. **Bước ③ (`fsync` thư mục)** hay bị quên: `fsync(fd)` chỉ đảm bảo **nội dung**, không đảm bảo **entry thư mục**.
3. Đây đúng là mẫu `git`, `sqlite`, `dpkg` dùng.

**Trích bank [LNX-007](../bank/linux-sysprog.md):**
> stdio buffer ở user space (libc), gom dữ liệu giảm số syscall; `fflush` đẩy xuống kernel. Page cache ở kernel [...] `write` thành công chỉ đảm bảo tới page cache. `fsync` ép kernel ghi page cache xuống disk vật lý — quan trọng cho độ bền (embedded mất điện).

**Chốt:** *"`fsync` làm dữ liệu bền. `rename` làm việc thay thế nguyên tử. Thiếu cái nào cũng còn cửa sổ mất sạch."*

**Lần sau sẽ hỏi:** góc mới — *"khách dùng thẻ SD rẻ, `fsync` mỗi lần lưu làm thiết bị giật 200 ms. Bạn đánh đổi thế nào?"* (gộp ghi, ghi nền, chấp nhận mất N giây cuối, chọn filesystem).

**Ôn:** [file-io.md §3 + §7](../../../04-linux-system-programming/file-io.md) · bank [LNX-007](../bank/linux-sysprog.md)
</details>

---

### Câu 6 · LNX-022 · 🟡 · **3/4**

Thiết bị ngoài hiện trường "đơ": UI không phản hồi, nhưng `ping` được, SSH vào được. `top` cho **load average 8.5** trong khi **%CPU idle ~98%**. `ps` cho thấy 6 process ở cột `STAT` mang chữ **`D`**, trong đó có daemon ghi log của bạn.

**(a)** `D` nghĩa là gì, vì sao load average cao trong khi CPU rảnh?
**(b)** Bạn `kill -9` cái daemon đó. Chuyện gì xảy ra?
**(c)** Nghi tầng nào hỏng — hỏi khách điều gì tiếp theo?

**🔁 (d)** Sáu process **không liên quan gì nhau** cùng kẹt `D` — thu hẹp nghi ngờ thế nào, chúng buộc phải có **điểm chung** gì?
**🔁 (e)** Nếu chỗ hỏng là eMMC/thẻ SD, vì sao bạn **vẫn SSH vào được** và gõ lệnh vẫn chạy?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ Đầy đủ: `D` = uninterruptible sleep; cả `R` lẫn `D` được tính vào load average; process `D` không đốt CPU nên CPU rảnh.
- (b) ✅ *"Không có tác dụng trên STAT D."*
- (c) 🟡 Đúng hướng (tầng đọc ghi disk), nhưng: *"kiểm tra như thế nào tôi chưa rõ"*.
- (d) 🟡 *"cùng bị vấn đề I/O với disk"* — hơi vòng lại chính nó, chưa nêu điểm chung cụ thể.
- (e) ✅ Đúng dù dè dặt: *"các hạ tầng liên quan SSH đều đang hoạt động trên RAM thay vì đọc từ disk"*.

**✅ Được:** (a)(b) không thiếu ý nào — hiểu đúng vì sao load average của Linux **không phải** "mức bận CPU". (e) đúng cơ chế page cache, và đó chính là dấu hiệu chẩn đoán *"hỏng ở đường xuống thiết bị lưu trữ, không phải CPU/RAM/mạng"*.

**❌ Vì sao chưa 4:** (c)(d) dừng ở **mô tả hiện tượng**, không ra được **hành động tiếp theo**. Ở mức mid trở lên, câu này đo *bước kế*, không đo định nghĩa.

**Đáp án của phần còn thiếu:**

Sáu process không liên quan cùng kẹt ⇒ điểm chung **không phải** "cùng có I/O" (vòng lại), mà cụ thể: **cùng đi qua một mount point / một thiết bị block / một server mạng**. Từ đó ba giả thuyết phân biệt được:

| Giả thuyết | Dấu hiệu phân biệt | Hỏi khách |
|---|---|---|
| eMMC/thẻ SD sắp chết | dmesg đầy lỗi I/O, retry, timeout | *"thiết bị có hay bị rút điện đột ngột không? thẻ SD dùng bao lâu, hãng nào?"* |
| Mount mạng (NFS/CIFS) treo | chỉ process đụng thư mục đó kẹt | *"có thư mục chia sẻ nào được mount không? mạng lúc đó thế nào?"* |
| Driver/thiết bị treo | cả 6 kẹt ở **cùng một hàm kernel** | *"lúc đó có cắm thêm USB/ngoại vi gì không?"* |

**Cách khoanh:** xem `/proc/<pid>/stack` của mấy process `D` — cùng một chỗ trong kernel ⇒ đó là điểm chung cần tìm. *(Chính là thứ câu 9 hỏi và bạn để trắng.)*

**Trích bank [LNX-022](../bank/linux-sysprog.md):**
> nếu I/O treo (NFS chết, disk hỏng) process **kẹt vĩnh viễn**, load average tăng vọt dù CPU rảnh. Thấy nhiều process `D` = nghi tầng storage/driver. Xem cột STAT trong `ps`, `/proc/<pid>/stack` để biết kẹt ở đâu.

**Chốt:** *"Load average của Linux đếm cả process kẹt I/O. Load cao + CPU rảnh + nhiều `D` = nghi tầng lưu trữ, không phải nghi CPU."*

**Lần sau sẽ hỏi:** góc mới — *"thiết bị `D` state nhưng bạn KHÔNG SSH được (sshd cũng kẹt). Còn cách nào lấy thông tin ra?"* (magic SysRq, serial console, watchdog reset + log giữ qua reboot).

**Ôn:** bank [LNX-022](../bank/linux-sysprog.md), [LNX-023](../bank/linux-sysprog.md) · [09-debugging/tools.md](../../../09-debugging/tools.md)
</details>

---

### Câu 7 · LNX-029 · 🟡 · **1/4** ⚠️ *(mới → vào sổ yếu)*

Thiết bị nhúng, **không có RTC pin nuôi** — mất điện là đồng hồ về mốc mặc định, tới khi NTP đồng bộ được qua Wi-Fi (thường 30–60 giây sau boot).

```c
time_t start = time(NULL);
send_scan_command();
while (!scan_done()) {
    if (time(NULL) - start > 5) { log("timeout"); return -1; }
}
```

Khách báo hai triệu chứng khác nhau, đều hiếm:
- ① Đôi khi lệnh **timeout ngay lập tức** dù module trả lời bình thường.
- ② Đôi khi lệnh **treo rất lâu** (hàng giờ) mới chịu timeout.

**(a)** Cùng một dòng code, vì sao sinh ra **hai** triệu chứng ngược nhau?
**(b)** Sửa thế nào — trong C++ thì tương ứng là gì?

**🔁 (c)** `time(NULL)` trả về cái gì, đếm từ mốc nào, ai được phép **sửa**? NTP chỉnh **lùi 1 giờ** giữa vòng lặp thì `time(NULL) - start` ra số gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a)(b) ❌ *"Chưa rõ về phần code hay kiến thức này"*.
- (c) 🟡 Sau khi thu hẹp: ✅ epoch 1970 đúng; ✅ suy đúng triệu chứng ② (*"thời gian đến khi timeout bị kéo dài ra đến 1 giờ nữa"*); ❌ không nêu triệu chứng ①; ❌ hỏi ngược *"NTP là gì?"*.

**❌ Vì sao 1 điểm:** trắng ở dạng câu hỏi thật; chỉ ra được một mảnh sau khi bị chỉ đường. Theo [config §4](../config.md): *"nhớ lõm bõm, thiếu nhiều"*.

> ⚖️ **Ghi nhận công bằng:** như phần phân định đầu log, **repo có lỗi link thật ở câu này** (bank trỏ `→ file-io` mà file-io.md không có mục clock). Đây là lỗ hổng do tài liệu, không phải do ứng viên bỏ ôn.

**Đáp án đầy đủ:**

`time()` / `CLOCK_REALTIME` là **giờ theo lịch** — và giờ theo lịch **nhảy được**: NTP chỉnh, admin đổi giờ, đổi múi giờ/DST. Vì vậy nó **không dùng để đo khoảng thời gian được**.

Hai triệu chứng ngược nhau từ đúng một dòng code:

| | NTP chỉnh **tiến** (nhảy vọt lên) | NTP chỉnh **lùi** |
|---|---|---|
| `time(NULL) - start` | vọt lên hàng nghìn giây | thành số **âm** hoặc rất nhỏ |
| Triệu chứng | ① **timeout ngay lập tức** | ② **treo hàng giờ** |

Điều kiện của đề — *"không RTC, NTP đồng bộ 30–60 s sau boot"* — làm cú nhảy **gần như chắc chắn xảy ra**, đúng lúc thiết bị đang khởi động với đủ loại timeout.

**Sửa — dùng đồng hồ đơn điệu** (đếm từ mốc bất kỳ, thường là lúc boot; **không bao giờ nhảy lùi**, không ai chỉnh được):
```c
struct timespec t0, t1;
clock_gettime(CLOCK_MONOTONIC, &t0);
...
clock_gettime(CLOCK_MONOTONIC, &t1);
double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
```

| Cần gì | C | C++ |
|---|---|---|
| *"bây giờ mấy giờ"* — timestamp log, mtime | `CLOCK_REALTIME` | `std::chrono::system_clock` |
| *"bao lâu"* — timeout, đo hiệu năng, lập lịch | **`CLOCK_MONOTONIC`** | **`std::chrono::steady_clock`** |

*(NTP = Network Time Protocol — giao thức đồng bộ đồng hồ máy qua mạng. Nó **chỉnh giờ hệ thống**, đó là toàn bộ lý do câu này tồn tại.)*

**Trích bank [LNX-029](../bank/linux-sysprog.md):**
> **Quy tắc:** cần *"lúc mấy giờ"* (timestamp log, mtime) → `REALTIME`; cần *"bao lâu"* (timeout, đo hiệu năng, lập lịch định kỳ) → `MONOTONIC`.
> Trong C++ đúng cùng nguyên tắc: **`std::chrono::steady_clock`** cho đo khoảng, `system_clock` cho thời điểm theo lịch — `steady_clock` tồn tại chính vì lý do này.

**Trích [TLPI cụm 03 §5](../../../15-book-summaries/the-linux-programming-interface/03-signals-and-timers.md)** — chỗ nội dung thật đang nằm:
> ⚠️ **Bẫy hay hỏi:** đo elapsed time bằng `CLOCK_REALTIME` là sai — NTP chỉnh giờ lùi 1 giây thì phép đo ra **số âm**, và timeout có thể chờ thêm cả tiếng. **Luôn dùng `CLOCK_MONOTONIC`** cho khoảng thời gian.

**Chốt:** *"Hỏi 'mấy giờ' thì REALTIME. Hỏi 'bao lâu' thì MONOTONIC. Nhầm chỗ là sinh timeout âm hoặc timeout vô tận."*

**Lần sau sẽ hỏi:** nén phần nền còn 1 checkpoint, vào thẳng vận dụng — *"log của bạn dùng timestamp REALTIME, đo hiệu năng dùng MONOTONIC. Giờ cần khớp một dòng log với một phép đo trễ — làm sao?"* (ghi cả hai mốc một lần, hoặc `CLOCK_BOOTTIME`; bàn cả ca thiết bị ngủ/suspend).

**Ôn:** bank [LNX-029](../bank/linux-sysprog.md) · [TLPI cụm 03 §5](../../../15-book-summaries/the-linux-programming-interface/03-signals-and-timers.md)
</details>

---

### Câu 8 · LNX-008/015 · 🟡 · **3/4**

Phần mềm cho máy quét: process `scanner` đọc ảnh từ cảm biến (**1920×1080, 8-bit, 30 fps** ≈ 60 MB/s), process `decoder` giải mã mã vạch. Hai process riêng vì decoder hay crash, không được kéo scanner chết theo.

**(a)** Chọn cơ chế IPC nào để chuyển ảnh? Lý do và **cái giá phải trả**.
**(b)** `decoder` crash giữa lúc đang đọc một khung hình — hệ thống rơi vào trạng thái gì? Phòng bằng cách nào?

**🔁 (c)** Giả sử đặt **mutex process-shared ngay trong vùng shared memory**. `decoder` bị `SIGSEGV` đúng lúc **đang giữ** mutex. `scanner` gọi `lock()` — nhận được gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ **Rất chắc**: chọn shared memory; giải thích zero-copy đúng chỗ (*"60MB/s là con số lớn nếu dùng IPC kiểu copy dữ liệu qua lại"*); nêu đúng cái giá (*"dùng chung thì phải xử lý đồng bộ"*). ❌ Kèm một giả định **sai**: *"Nếu 1 trong 2 chết thì shared memory không bị tác động và kéo process kia chết theo."*
- (b) ❌ **"Chưa rõ."**
- (c) ✅ Khi bị đặt vào tình huống cụ thể thì trả lời đúng ngay: *"nó sẽ bị chặn [...] bị treo vĩnh viễn"*.

**✅ Được:** (a) là một quyết định thiết kế có đánh đổi rõ — đúng thứ T2 đo. Nhận diện đúng rằng chi phí copy 2 lần qua kernel là chỗ chết ở băng thông này.

**❌ Vì sao chưa 4:** vế *"1 chết không ảnh hưởng cái kia"* là giả định sai, và chính (b) hỏi vào đó thì trắng.

**Đáp án đầy đủ — stale lock (mutex mồ côi):**

`decoder` chết khi đang giữ mutex ⇒ mutex **không bao giờ được nhả** ⇒ `scanner` treo vĩnh viễn. Điều này **phá luôn lý do bạn tách hai process**: tách ra để decoder crash không giết scanner, nhưng cái mutex đã nối chúng lại với nhau.

| Cách | Cơ chế | Đánh đổi |
|---|---|---|
| **Robust mutex** | `pthread_mutexattr_setrobust` ⇒ chủ mutex chết thì `lock()` trả `EOWNERDEAD` thay vì treo; bên sống gọi `pthread_mutex_consistent()` để nhận lại | Phải viết code khôi phục — dữ liệu có thể đang **dở dang** |
| **Bỏ hẳn lock ở hot path** ⭐ | Ring buffer với **chỉ số đọc/ghi riêng** mỗi bên; consumer chết thì producer vẫn chạy, chỉ đè lên khung cũ | Thiết kế khó hơn; **hợp đúng bài này** vì mất khung ảnh cũ không sao |
| **Watchdog** | Process giám sát thấy decoder chết ⇒ reset vùng shm + khởi động lại | Đơn giản nhất, có khoảng gián đoạn |

> 🔗 **Nối với câu 12:** cách 2 đúng vì **dữ liệu ảnh/cảm biến cho phép đè cái cũ** — chính nguyên lý bạn trả lời rất hay ở câu 12 nhưng chưa mang sang câu 8.

**Chốt:** *"Tách process để cách ly lỗi, rồi đặt mutex chung vào giữa = nối lại đúng thứ vừa tách. Muốn cách ly thật thì đừng để bên này chờ khoá của bên kia."*

**Lần sau sẽ hỏi:** góc mới — *"scanner ghi khung mới trong khi decoder đang đọc khung đó. Không dùng khoá thì làm sao decoder biết dữ liệu nó vừa đọc còn hợp lệ?"* (seqlock / double buffer / đánh số khung + kiểm tra lại sau khi đọc).

**Ôn:** bank [LNX-008](../bank/linux-sysprog.md), [LNX-015](../bank/linux-sysprog.md) · [ipc-linux.md](../../../04-linux-system-programming/ipc-linux.md) · [12-dsa/ring-buffer.md](../../../12-dsa/ring-buffer.md) §1–§6
</details>

---

### Câu 9 · LNX-023 · 🟡 · **0/4** ⚠️ *(mới → vào sổ yếu)*

Quay lại tình huống câu 6: bạn đã SSH được vào thiết bị, có 6 process kẹt `D`, và muốn biết **process nào đang mở file nào** và **nó đang kẹt ở chỗ nào trong kernel** — nhưng thiết bị nhúng này **không cài `lsof`, không cài `strace`**, rootfs tối giản.

**(a)** Kernel vẫn phơi bày những thông tin đó ra ngoài. Ở **đâu**, và cơ chế nào cho phép "đọc file" mà lại ra được trạng thái sống của kernel?
**(b)** Trên cùng thiết bị có hai thư mục ảo kiểu này. Kể tên cả hai và **phân vai**. Cần bật/tắt chân GPIO hoặc đọc nhiệt độ CPU thì vào cái nào?

**🔁 (c)** `ps` và `top` ở câu 6 lấy số liệu từ đâu ra? Không có syscall nào tên `get_process_list()`.

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:** ❌ **"Chưa rõ"** cả hai lần hỏi.

> ⚖️ **Ghi nhận công bằng:** repo cũng có **lỗi link** ở câu này (bank trỏ `→ file-io` mà file-io.md không có mục nào về `/proc`, `/sys`). Nội dung thật rải ở [09-debugging/tools.md](../../../09-debugging/tools.md) và [05-drivers-device-tree](../../../05-drivers-device-tree/) — chưa tới trong plan.

**Đáp án đầy đủ:**

Cả `/proc` và `/sys` là **virtual filesystem**: **không nằm trên đĩa**. Nội dung được **kernel sinh ra ngay lúc bạn đọc** — mỗi lần `cat` là một lần kernel chạy hàm sinh nội dung rồi trả về. Đó là *"everything is a file"* áp cho **trạng thái kernel**: tra được trạng thái sống của hệ thống chỉ bằng `read()`, không cần syscall riêng, không cần cài công cụ.

Chính vì thế `ps` và `top` **không có syscall riêng** — chúng chỉ **duyệt `/proc/<pid>/`**. Bạn luôn tự làm được việc của chúng, kể cả rootfs trống trơn.

| | `/proc` | `/sys` (sysfs) |
|---|---|---|
| Về cái gì | **Process** + thông tin hệ thống lịch sử | **Thiết bị & driver** (device model: bus/device/driver/class) |
| Cấu trúc | tạp, mỗi file một định dạng | có cấu trúc, **một giá trị / một file** |
| Ví dụ | `/proc/<pid>/fd/`, `/proc/<pid>/stack`, `/proc/<pid>/maps`, `/proc/meminfo`, `/proc/interrupts` | `/sys/class/gpio/`, `/sys/class/thermal/thermal_zone0/temp`, `/sys/bus/i2c/devices/` |

**Trả lời trực tiếp cho câu 6:** thay `lsof` bằng `ls -l /proc/<pid>/fd/`; thay `strace` (một phần) bằng `cat /proc/<pid>/stack` và `/proc/<pid>/wchan` — cho biết process `D` đang kẹt ở hàm kernel nào. **Bật GPIO / đọc nhiệt độ CPU → `/sys`.**

**Trích bank [LNX-023](../bank/linux-sysprog.md):**
> Cả hai là **virtual filesystem** (nội dung sinh bởi kernel lúc đọc, không nằm trên disk) — "everything is a file" áp cho trạng thái kernel. `/proc`: thông tin **process** [...] `/sys` (sysfs): mô hình **thiết bị/driver** có cấu trúc [...] Đại khái: `/proc` cũ + tạp, `/sys` mới + có cấu trúc cho device.

**Chốt:** *"Thiết bị nhúng không có công cụ gì thì vẫn còn `/proc` và `/sys` — kernel tự phơi trạng thái ra dưới dạng file."*

**Lần sau sẽ hỏi:** vẫn ở T1+T2 vì lần này trắng — *"cho một `/proc/<pid>/` cụ thể: cần biết daemon rò fd hay không, bạn xem file nào và nhìn cái gì?"* Rồi mới lên: *"driver bạn viết muốn phơi một tham số cấu hình ra userspace — chọn `/proc` hay `/sys`, vì sao?"*

**Ôn:** bank [LNX-023](../bank/linux-sysprog.md) · [09-debugging/tools.md](../../../09-debugging/tools.md) · [05-drivers-device-tree/kernel-userspace.md](../../../05-drivers-device-tree/kernel-userspace.md)
</details>

---

### Câu 10 · LNX-020 · 🟡 · **3/4**

Daemon chạy script rồi đọc output qua pipe:

```c
int p[2];
pipe(p);
if (fork() == 0) {                    // con
    dup2(p[1], STDOUT_FILENO);
    execlp("scan_tool", "scan_tool", NULL);
}
// cha: đọc hết output của con
char buf[256];
ssize_t n;
while ((n = read(p[0], buf, sizeof buf)) > 0)
    handle(buf, n);
close(p[0]);
waitpid(-1, NULL, 0);
```

`scan_tool` chạy xong và thoát bình thường, nhưng **process cha treo vĩnh viễn** trong vòng `while`.

**(a)** Vì sao `read` không bao giờ trả về 0 (EOF), dù con đã thoát?
**(b)** Sửa thế nào — chính xác là đóng cái gì, ở đâu?

**🔁 (c)** Phía **con** cần đóng cái gì trước `exec`? Nếu không, `scan_tool` thừa hưởng cái gì?
**🔁 (d)** Ca ngược: cha `close(p[0])` khi con vẫn đang ghi. Con nhận được gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ *"Vì ở phía cha chưa đóng `p[1]` (write), nên EOF sẽ không bao giờ trả về."*
- (b) ✅ *"ngay sau khi `fork()`, cha `close(p[1])`"*.
- (c) ✅ *"Phía con cần đóng phần read trước khi exec để tránh `scan_tool` thừa hưởng đầu read của pipe."* — tự nối về câu 4.
- (d) 🟡 *"Con sẽ nhận về 1 lỗi khi không write được, chưa rõ lỗi gì."*

**✅ Được:** ba ý đầu đúng gọn, không lặp, và biết nối sang cơ chế kế thừa fd của câu 4.

**Đo thật:**
```
cha QUEN close(p[1])   -> TREO vinh vien trong read()   <-- bug
cha CO close(p[1])     -> doc xong, thay EOF, thoat sach
```

**Nguyên tắc:** EOF trên pipe xảy ra khi **mọi** fd trỏ tới đầu ghi đã đóng. Cha đang giữ một cái, nên *"con đã thoát"* không đủ.

**❌ Vì sao chưa 4 — (d):** câu trả lời quan trọng hơn bạn nghĩ. Ghi vào pipe **không còn reader** ⇒ kernel gửi **`SIGPIPE`**, mà hành vi **mặc định của `SIGPIPE` là GIẾT process** — im lặng, không log, không core dump mặc định. Đây là bug kinh điển của server: client ngắt kết nối giữa chừng ⇒ `write()` vào socket ⇒ **daemon chết không dấu vết**.

Chỉ khi **chặn hoặc bỏ qua** `SIGPIPE` thì `write` mới trả `-1` với `errno == EPIPE` để bạn xử lý:
```c
signal(SIGPIPE, SIG_IGN);                    // ✅ gần như bắt buộc cho mọi daemon/server
...
if (write(fd, buf, n) < 0 && errno == EPIPE) { /* peer đã đóng, dọn dẹp */ }
```

**Trích bank [LNX-020](../bank/linux-sysprog.md):**
> ghi vào pipe đầy thì block (hoặc EAGAIN nếu nonblock); ghi khi không còn reader → `SIGPIPE`/`EPIPE`; ghi ≤ `PIPE_BUF` là atomic.

*(Chính `PIPE_BUF` này là chỗ bạn hụt ở câu `O_APPEND` phần 1.)*

**Chốt:** *"Pipe chỉ báo EOF khi đầu ghi CUỐI CÙNG đóng. Và ghi vào pipe không còn reader thì mặc định bạn CHẾT, không phải nhận lỗi."*

**Lần sau sẽ hỏi:** góc mới — *"cha đọc stdout của con bằng vòng `read` như trên, con lại ghi rất nhiều ra stderr mà cha không đọc. Chuyện gì xảy ra?"* (pipe stderr đầy → con block → deadlock hai chiều; đây là lý do phải dùng `poll`/`epoll` cho cả hai đầu).

**Ôn:** bank [LNX-020](../bank/linux-sysprog.md), [LNX-012](../bank/linux-sysprog.md) · [ipc-linux.md](../../../04-linux-system-programming/ipc-linux.md)
</details>

---

### Câu 11 · LNX-004 · 🟡 · **3/4**

Daemon in ấn chạy liên tục nhiều tuần. Nó `fork()` một process con cho mỗi job, con làm xong thì thoát. Daemon **không gọi `wait()`** vì *"con tự thoát rồi, có gì đâu"*.

Sau khoảng 3 tuần chạy, thiết bị bắt đầu không tạo được process mới: `fork()` trả `-1`. Nhưng `free -m` cho thấy RAM vẫn còn nhiều, `df` cho thấy disk còn trống.

**(a)** Chuyện gì đã tích tụ suốt 3 tuần? Nó tiêu tốn tài nguyên gì mà `free` và `df` không thấy?
**(b)** Hai cách sửa — nêu đánh đổi giữa chúng.

**🔁 (c)** Handler `SIGCHLD` gọi `waitpid(-1, NULL, WNOHANG)` **một lần** rồi return. Năm con thoát gần như cùng lúc. Đủ chưa?
**🔁 (d)** Có **cách thứ ba** — không `wait()`, không handler, mà zombie vẫn không tích tụ?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ Zombie process; và chỉ đúng tài nguyên bị cạn: **PID**, giải thích được vì sao `free`/`df` không thấy.
- (b) ✅ Hai cách kèm đánh đổi: `wait`/`waitpid` đồng bộ (*"rủi ro daemon bị pending"*) vs handler `SIGCHLD` (*"fork xong làm việc khác"*).
- (c) 🟡 ✅ Kết luận đúng (*"Không đủ [...] cần bỏ trong vòng lặp"*); ❌ không nêu **vì sao**; ❌ hỏi ngược *"bỏ trong signal handler hay main loop?"*.
- (d) ❌ **"Chưa rõ."**

**✅ Được:** (a) chỉ đúng **PID cạn** — đó là chìa khoá của cả câu, vì nó giải thích triệu chứng lạ (fork lỗi mà RAM còn đầy). (b) trả lời đúng kiểu một câu thiết kế: hai phương án + đánh đổi.

Chính xác thêm một chút: zombie **không phải "không tốn gì"** — nó giữ một **entry trong bảng process** (PID + exit status) chờ cha đọc. Nhỏ, nhưng có trần: `/proc/sys/kernel/pid_max`.

**❌ Vì sao chưa 4 — cơ chế ở (c) và cách thứ ba ở (d).**

**(c) — vì sao phải lặp: `SIGCHLD` KHÔNG XẾP HÀNG.** Signal chuẩn chỉ là một bit *"có signal đang chờ"*; năm con thoát gần cùng lúc có thể chỉ sinh ra **một** lần gọi handler. Đo thật:
```
handler waitpid GOI 1 LAN      -> thu hoi trong handler: 3/5, con lai ZOMBIE: 2 <-- ro PID
handler waitpid trong VONG LAP -> thu hoi trong handler: 5/5, con lai ZOMBIE: 0
```
Gọi một lần ⇒ **rò 2 zombie** ngay trong phép thử 5 con.

**Trả lời câu bạn hỏi ngược — đặt vòng lặp TRONG handler:**
```c
void on_sigchld(int) {
    int saved = errno;                             // handler phải bảo toàn errno
    while (waitpid(-1, NULL, WNOHANG) > 0) { }     // ✅ vét cạn
    errno = saved;
}
```
`WNOHANG` là thứ làm việc này an toàn trong handler: hết con thì trả về 0 ngay, không chặn.

**(d) Cách thứ ba — bảo kernel tự dọn:**
```c
signal(SIGCHLD, SIG_IGN);        // hoặc sigaction với SA_NOCLDWAIT
```
Con thoát là kernel thu hồi luôn, không tạo zombie. **Đánh đổi: mất luôn exit status** — không biết con thành công hay thất bại. Dùng được khi fire-and-forget; với daemon in ấn **cần** biết job hỏng hay không thì **không** dùng.

**Trích [processes-signals.md §3 SIGCHLD & thu hồi zombie](../../../04-linux-system-programming/processes-signals.md)** — mục này repo **có**, đúng file được giao đọc.

**Chốt:** *"Zombie ăn PID chứ không ăn RAM. Và `SIGCHLD` không xếp hàng — handler phải vét cạn bằng vòng lặp `WNOHANG`."*

**Lần sau sẽ hỏi:** góc mới — *"daemon dùng `SIG_IGN` cho SIGCHLD, nhưng có một chỗ vẫn gọi `waitpid()` để lấy exit status. Nó nhận được gì?"* (`-1`/`ECHILD` — hai cơ chế đá nhau).

**Ôn:** [processes-signals.md §1 + §3](../../../04-linux-system-programming/processes-signals.md) · bank [LNX-004](../bank/linux-sysprog.md)
</details>

---

### Câu 12 · LNX-017 · 🟡 ⭐ · **4/4** ⭐

Thiết bị có một luồng đọc cảm biến ánh sáng, đẩy giá trị qua **POSIX message queue** cho luồng điều khiển đèn nền (ramp độ sáng theo môi trường). Sensor bắn **200 mẫu/giây**; luồng điều khiển xử lý chậm hơn, khoảng **50/giây**.

Sau vài phút chạy, `mq_send` bắt đầu **chặn**, và luồng đọc cảm biến trễ theo — làm mất luôn cả các mẫu về sau.

**(a)** Mặc định `mq_send` khi hàng đầy làm gì? Có cờ nào đổi hành vi đó?
**(b)** Nhưng đổi cờ chưa phải lời giải đúng. Với **loại dữ liệu này**, đâu mới là cách xử lý đúng khi hàng đầy — và vì sao nó đúng với dữ liệu cảm biến mà **sai** với hàng đợi lệnh in?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ *"Mặc định khi đầy thì sẽ pending [...] Dùng cờ `O_NONBLOCK`, trả về -1 và mã lỗi `EAGAIN`."*
- (b) ✅ **Đúng cả hai chiều.** Cảm biến: *"luôn đảm bảo dữ liệu trong buffer sẽ luôn là mới nhất [...] chấp nhận ghi đè dữ liệu cũ"*. Lệnh in: *"thiết kế ghi đè → mất lệnh, sai logic. Thiết kế đầy thì producer bị pending không thể đưa lệnh mới vào tiếp, gây áp lực xử lý lên tầng trên."*

**✅ Vì sao 4 điểm:** nắm đúng nguyên lý gốc — **bản chất dữ liệu quyết định chính sách khi đầy** — và tự lập luận được cả chiều ngược lại mà không cần gợi ý. Đây là câu thiết kế, chấm theo khung tiếp cận, và khung của bạn đúng.

| Loại dữ liệu | Đầy thì làm gì | Vì sao |
|---|---|---|
| **Trạng thái** (ánh sáng, nhiệt độ, vị trí) | **đè cái cũ** — latest-value-wins | mẫu cũ đã **sai** so với hiện tại; giữ lại là giữ rác |
| **Sự kiện/lệnh** (job in, phím bấm, giao dịch) | **chặn / backpressure / persist** | mỗi phần tử là một việc phải làm, mất là sai nghiệp vụ |

**Hai ý nâng thêm để lần sau nói cho tròn:**
1. **Đếm số mẫu bị bỏ** (`dropped_count`) — mất mẫu thì được, nhưng **không biết mình mất** thì không.
2. **Rate limiting phía producer** — 200 Hz cho một cái đèn nền là thừa; giảm ngay từ nguồn rẻ hơn mọi thứ khác.

**Trích bank [LNX-017](../bank/linux-sysprog.md):**
> nếu chỉ **giá trị đích mới nhất** quan trọng (vd ramp độ sáng), giải pháp đúng là **coalescing/latest-value-wins** [...] hoặc **drop-oldest** [...] kèm **rate limiting** ở producer. Tránh để block làm trễ phản ứng.

**Chốt:** *"Hàng đầy không phải lỗi kỹ thuật, là câu hỏi nghiệp vụ: dữ liệu này mất được không?"*

**Lần sau sẽ hỏi (retention):** góc mới — *"hàng đợi lệnh in đầy thật, producer là UI. Chặn UI thì người dùng thấy máy treo. Bạn thiết kế backpressure thế nào để vừa không mất lệnh vừa không đơ giao diện?"*
</details>

---

## 🎯 Ba lỗ hổng ưu tiên

| # | Lỗ hổng | Vì sao quan trọng | Ôn ở đâu |
|---|---|---|---|
| **1** | **`/proc` & `/sys`** — hoàn toàn trắng | Debug trên thiết bị nhúng rootfs tối giản = **đúng phần JD nhấn mạnh**. Không có nó thì gặp `D` state là bó tay | bank [LNX-023](../bank/linux-sysprog.md) · [09-debugging/tools.md](../../../09-debugging/tools.md) |
| **2** | **Chọn clock cho timeout** — `MONOTONIC` vs `REALTIME` | JD là thiết bị nhúng; *"không RTC + NTP nhảy giờ sau boot"* là điều kiện **mặc định**, không phải ca hiếm | bank [LNX-029](../bank/linux-sysprog.md) · [TLPI cụm 03 §5](../../../15-book-summaries/the-linux-programming-interface/03-signals-and-timers.md) |
| **3** | **Bền vững dữ liệu khi mất điện** — thiếu mẫu `tmp + fsync + rename + fsync(dir)` | Thiết bị cầm tay tháo pin bất kỳ lúc nào. Biết `fsync` mà không biết `rename` vẫn mất sạch config | bank [LNX-007](../bank/linux-sysprog.md) · [file-io.md §2.2](../../../04-linux-system-programming/file-io.md) |

## 💡 Điểm mạnh & một quan sát về cách học

**Cơ chế fd và vòng đời process là vùng chắc.** Câu 4 (4đ), 10, 11 đều nắm đúng bảng fd, tham chiếu, kế thừa qua `fork`/`exec`, zombie. Câu 4(c) tự nêu race đa luồng là mức **trên mid**.

**📌 Quan sát quan trọng — mẫu lặp lại rõ trong cả phiên:** hỏi trống → *"chưa rõ"*; thu hẹp thành một **tình huống cụ thể** → trả lời đúng. Xảy ra ở câu **5(c), 6(e), 8(c), 11(c)**. Đây **không phải** yếu kiến thức mà là **thiếu đường vào**.

⇒ **Gợi ý cách ôn hợp với bạn:** đọc theo *triệu chứng → cơ chế*, đừng đọc theo mục lục khái niệm. Với mỗi mục tài liệu, tự đặt câu hỏi *"cái này hỏng thì khách thấy gì?"* trước khi đọc nội dung.

---
⬅️ [Phần 1 của buổi này](2026-08-13--rapid--linux-sysprog.md) · [Sessions index](README.md) · [weak-register](../weak-register.md) · [datalogic-plan §📍](../../study-plans/datalogic-plan.md)
