# Phiên mock — 2026-08-13 · rapid · track linux-sysprog

- **Level:** mid-level · **Số câu:** 12 dự kiến → **dừng ở câu 4** (ứng viên tạm ngưng), tính **3 câu**
- **Điểm trung bình:** **2.67 / 4** (3 · 2 · 3) · **Trần độ sâu: T2**
- **Bối cảnh:** Datalogic plan — **Tuần 2 · Buổi 1** (Linux sysprog: file-io + processes-signals). Mở màn bằng slot 🔁 khởi động với câu yếu **OS-020** đúng như §📍 chỉ định.
- **➡️ Phần tiếp theo:** [2026-08-13--rapid--linux-sysprog-2.md](2026-08-13--rapid--linux-sysprog-2.md) — 9 câu còn lại, hoàn tất Buổi 1 (điểm cả buổi **2.67/4**).
- **Đặc điểm phiên:** dù là `rapid`, câu 1 bị đào sâu 6 tầng (a→f) vì là câu trong sổ yếu — đúng [config §6 luật ①](../config.md) (câu weak: nén nền, dồn trọng số vào follow-up).

## Kết quả từng câu (nhìn nhanh)

| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| OS-020 | signal-safety trong daemon (handler `printf`/`free`) | **3** 🔼 | Nền đã vá xong (2→3). Mất điểm: **`SA_RESTART` là quyết định ngược** · `printf`+`_exit` mất log · không biết `TimeoutStopSec` |
| LNX-005 | short read trên socket (lab đúng, hiện trường sai) | **2** ⚠️ | Liệt kê `n` đủ 4 ca, nhưng **cơ chế TCP = luồng byte TRẮNG** → vào sổ yếu |
| LNX-028 | `O_APPEND` — hai process ghi chung log | **3** | Lõi đúng (khe hở `lseek`+`write`, `O_CREAT\|O_EXCL`). Mất điểm: khẳng định nguyên tử **vô điều kiện**, không biết biên `PIPE_BUF` / short write / NFS |
| LNX-012 | *(đã ra đề, chưa trả lời — không tính)* | — | fd rò qua `exec` → `EADDRINUSE`. **Hỏi lại đầu buổi sau** |

---

## 🔎 Chi tiết ôn

### Câu 1 · OS-020 · 🟡 · **3/4** 🔼 *(lần trước 2)*

Đây là code trong một daemon thật, chạy trên thiết bị của khách:

```c
static char* g_last_msg = NULL;

void on_sigterm(int sig) {
    printf("[shutdown] signal %d, last msg: %s\n", sig, g_last_msg);
    free(g_last_msg);
    g_last_msg = NULL;
    _exit(0);
}

int main(void) {
    struct sigaction sa = { .sa_handler = on_sigterm };
    sigaction(SIGTERM, &sa, NULL);
    for (;;) {
        g_last_msg = strdup(read_next_event());   // strdup -> malloc
        process(g_last_msg);
    }
}
```

QA báo: chạy 500 lần `systemctl stop` thì khoảng **3–4 lần** daemon **treo cứng**, không thoát, phải `SIGKILL`. Còn lại thoát bình thường.

**(a)** Những lời gọi nào là **cấm**, và cơ chế nào tạo ra vụ treo 3–4/500 đó?
**(b)** Viết lại handler cho đúng — nếu vẫn muốn **in được `last msg`** lúc shutdown thì làm bằng mẫu gì?

**🔁 Follow-up (c):** `read_next_event()` thực tế là `read()` chặn trên socket, thiết bị rảnh 10 phút không có event. Lúc đó `systemctl stop` gửi SIGTERM. Chuyện gì xảy ra? systemd làm gì tiếp theo?
**🔁 Follow-up (d):** `printf(...)` rồi `_exit(0)` — có gì đáng ngại ở cặp này?
**🔁 Follow-up (e):** `sa_flags` ở đây bằng 0. Một `read()` đang chặn mà SIGTERM tới thì trả về **cái gì**? Có làm bạn sửa lại (c) không?
**🔁 Follow-up (f):** Chỉ có SIGTERM, `stdout` được journald thu qua **pipe**. Dòng log `[shutdown]...` có tới journald không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ✅ *"2 hàm không đảm bảo async-signal-safe: `printf` và `free`. Khi `strdup` đang giữ khoá heap thì SIGTERM chen vào, `free` khoá lại lần nữa (khoá heap chung với `malloc`) → khoá 2 lần, không tự giải phóng. Hy hữu khi SIGTERM tới đúng lúc `strdup` dở dang, nên phần lớn lần chạy không lỗi."* — **chuẩn cả cơ chế lẫn lý do tần suất thấp**.
- (b) ✅ Viết bản dùng `volatile sig_atomic_t isTerminated`, handler chỉ set cờ, `printf` chuyển xuống main loop.
- (c) 🟡 *"code vẫn treo ở `read` thay vì chạy xuống terminate"* — đúng, **nhưng không nói được systemd làm gì tiếp**. Kèm một câu lệch: *"`read` có tính async-signal-safe nên không phải điểm gây lỗi"*.
- (d) ❌ Trả lời về re-entrancy của `printf` (không áp dụng — `printf` giờ ở main loop, không ở handler). **Bỏ sót vấn đề thật: stdio buffer.**
- (e) 🟡 ✅ `EINTR` đúng — ❌ nhưng đề xuất **`SA_RESTART`**, là **quyết định ngược**.
- (f) ✅ Sau khi bị thu hẹp tới "pipe → journald" thì nêu đúng: *"vẫn có rủi ro `printf` đang cache chứ chưa thực sự write, nên flush trước khi exit"*.

**✅ Được (so với lần trước 13/08):** đúng chỗ hổng cũ đã vá. Lần trước sai *"`write()` không dùng được trong handler"* và tưởng họ `_r` cho signal-safety. Lần này gọi tên chính xác `printf`/`free` + cơ chế khoá heap + tự giải thích tần suất 3–4/500. **Checkpoint nền: đạt.**

**❌ Vì sao chưa 4 — ba chỗ, xếp theo mức nghiêm trọng:**

**① `SA_RESTART` chữa nhầm bệnh (nặng nhất — đây là quyết định SAI, tức lỗi T2).**
Đặt `SA_RESTART` vào chính daemon này = tự tay dựng lại con bug ban đầu: `read()` restart ⇒ vòng lặp không bao giờ chạy tới chỗ kiểm tra `isTerminated` ⇒ **SIGTERM vô hiệu**.

Đo thật (`gcc -std=c11 -Wall -Wextra`, pipe không ai ghi, con gửi SIGTERM cho cha sau 200 ms):
```
=== sa_flags = 0 ===
sa_flags=0 -> read tra ve -1, errno=Interrupted system call, stop=1
(exit=0)                          <- thoát sạch

=== sa_flags = SA_RESTART ===
Killed
(exit=137 = bi SIGKILL -> TREO, SIGTERM vo dung)
```
> 🔬 Chi tiết đáng nhớ: lần chạy đầu dùng `timeout 3` (gửi SIGTERM) — **không giết được process**, treo tới hết 2 phút timeout. Phải `timeout -s KILL`. Tái hiện đúng triệu chứng QA: *"phải SIGKILL"*.

**Quy tắc đúng:** `EINTR` **không** phải thứ để dập bằng cờ — nó chính là **cách signal đánh thức bạn ra khỏi chỗ đang chặn**, thứ bạn *cần* khi shutdown. Xử lý theo **ý định**:
```c
ssize_t n = read(fd, buf, len);
if (n < 0 && errno == EINTR) {
    if (stop) break;      // ✅ signal shutdown -> thoát vòng lặp
    continue;             // ✅ signal khác     -> thử lại
}
```

**② `printf` + `_exit` = mất log.** Khi `stdout` không phải terminal, glibc chuyển sang **full buffering** (4 KB) thay vì line buffering. `_exit()` là syscall trần — **không chạy atexit, không flush stdio**. Dưới systemd, stdout đi qua **pipe tới journald** ⇒ luôn rơi vào ca này. Đúng dòng log cần nhất khi điều tra lại là dòng chắc chắn mất. Đo thật:
```
--- _exit(0), stdout la PIPE (giong journald) ---
[rong = MAT LOG]
--- exit(0), stdout la PIPE ---
[shutdown] last msg: hello
```
Sửa: `fflush(stdout)` trước `_exit`, hoặc `exit()`, hoặc bỏ stdio dùng `write(STDERR_FILENO, ...)`.

**③ Không nói được systemd làm gì tiếp.** systemd chờ **`TimeoutStopSec`** (mặc định **90 s**) rồi gửi **SIGKILL**. Hệ quả thật: mỗi `systemctl restart` treo 90 giây, OTA update rơi vào timeout, daemon bị giết cứng nên **không kịp flush trạng thái** — mất dữ liệu chứ không chỉ chậm.

**Một chỗ diễn đạt lệch cần sửa:** async-signal-safety trả lời câu hỏi *"gọi được từ TRONG handler không"*. `read()` ở main loop thì tính chất đó không liên quan — vấn đề của nó là **blocking**. Hai trục khác nhau, đừng trộn.

**Bản đúng hoàn chỉnh** (đã compile + chạy): [`coding-arena/reviewed/2026-08-13--OS-020--sigterm-handler.cpp`](../coding-arena/reviewed/2026-08-13--OS-020--sigterm-handler.cpp)

**Chốt:** *"Handler chỉ set cờ. Chỗ đang chặn phải để `EINTR` đánh thức — đừng dập bằng `SA_RESTART`. Và `_exit` không flush stdio."*

**Lần sau sẽ hỏi:** cho một daemon dùng `epoll_wait` + `signalfd` — *"vì sao kiến trúc này làm cả `EINTR` lẫn cờ `volatile` biến mất?"*, rồi bắt so sánh với self-pipe trick.

**Ôn:** [processes-signals.md §SA_RESTART KHÔNG cứu được mọi syscall](../../../04-linux-system-programming/processes-signals.md) · bank [OS-020](../bank/os.md), [LNX-011](../bank/linux-sysprog.md), [LNX-027](../bank/linux-sysprog.md), **[LNX-030 (mới, sinh từ phiên này)](../bank/linux-sysprog.md)**
</details>

---

### Câu 2 · LNX-005 · 🟡 · **2/4** ⚠️ *(mới → vào sổ yếu)*

Đoạn này nằm trong lớp truyền dữ liệu qua socket của một thiết bị đọc mã vạch:

```c
uint8_t hdr[8];
ssize_t n = read(sock, hdr, 8);
if (n != 8) { log_error("header lỗi"); close(sock); return -1; }
```

Chạy trên LAN phòng lab thì không bao giờ lỗi. Ra hiện trường qua Wi-Fi thì thỉnh thoảng khách báo *"mất kết nối, phải quét lại"*.

**(a)** Vì sao lab đúng mà hiện trường sai?
**(b)** `n` có thể mang những giá trị nào — kể hết và mỗi giá trị nghĩa là gì?

**🔁 Follow-up (c):** "Wi-Fi không ổn định" là hiện tượng. Ở tầng TCP, **cơ chế** nào khiến đúng 8 byte header bị tách làm hai lần `read`, trong khi lab thì không?
**🔁 Follow-up (d):** Sửa lại hai dòng đó. Ngoài lặp cho đủ 8 byte, còn `errno` nào bắt buộc xử lý riêng, không được coi là lỗi thật?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) ❌ *"Wi-Fi tín hiệu có thể thiếu ổn định ngoài hiện trường, không có gì đảm bảo socket hoạt động trơn tru như ở phòng lab."* — **hiện tượng, không phải cơ chế**.
- (b) ✅ **Đủ và đúng cả bốn ca**: `8` lý tưởng · `1..7` đọc chưa hết, cần đọc tiếp · `0` peer đóng · `-1` lỗi kèm `errno`.
- (c) ❌ **"Chưa rõ"** — trắng.
- (d) ✅ `EINTR` — *"không phải lỗi, cần `read()` lại"*.

**✅ Được:** phần liệt kê `n` không thiếu ý nào; biết short read tồn tại và biết `EINTR` không phải lỗi thật.

**❌ Vì sao 2 điểm:** T1 chắc, **T2 trắng**. Cái được đo ở đây không phải "short read là gì" (bạn thuộc) mà là **"vì sao lab không lộ mà khách lộ"** — tức năng lực chẩn đoán lớp bug phụ thuộc môi trường. Đúng định nghĩa mức 2 trong [config §4](../config.md): *"T1 có, T2 trắng"*.

**Đáp án đầy đủ của phần còn thiếu:**

**TCP là luồng byte, KHÔNG có ranh giới message.** TCP chỉ hứa hai điều: byte tới **đủ** và **đúng thứ tự**. Nó **không** hứa *"bên kia `write` 8 byte một lần thì bạn nhận 8 byte một lần"*. 8 byte có thể tới thành 3 + 5 vì:
- **MSS / phân mảnh** — header nằm vắt qua hai TCP segment.
- **Nagle + delayed ACK**, retransmit — segment thứ hai tới sau vài chục ms.
- `read()` trả về **ngay khi có byte nào đó** trong receive buffer, không đợi đủ `count`.

**Vì sao lab không lộ:** LAN RTT ~0.1 ms, không mất gói ⇒ 8 byte luôn gọn trong một segment và đã nằm sẵn trong receive buffer trước khi gọi `read()`. Wi-Fi hiện trường có retransmit, jitter, RTT vài chục ms ⇒ cửa sổ *"mới tới một phần"* mở ra thật.

⚠️ **Đây là mẫu bug đáng nhớ nhất của câu này: đúng trên lab, sai ở khách — vì lab không tạo được điều kiện race.** Cùng họ với vụ 3–4/500 ở câu 1.

**Hệ quả thiết kế phải tự rút ra:** mọi protocol trên TCP đều phải **tự framing** (length-prefix hoặc delimiter), và mọi `read` phải bọc vòng lặp:
```c
static int read_full(int fd, void* buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t n = read(fd, (char*)buf + got, len - got);
        if (n > 0)  { got += n; continue; }
        if (n == 0) return 0;              // peer đóng sớm
        if (errno == EINTR) continue;      // ✅ bạn đã nêu đúng
        return -1;
    }
    return 1;
}
```

**Trích bank [LNX-005](../bank/linux-sysprog.md):**
> Không phải lỗi (short read). `read`/`write` trả số byte thực sự xử lý, có thể ít hơn (**pipe/socket mới có một phần**, bị signal ngắt). Xử lý bằng vòng lặp tiếp tục cho phần còn lại tới khi đủ/EOF/lỗi, và xử lý `EINTR` bằng thử lại.

**Trích tài liệu gốc [file-io.md §1](../../../04-linux-system-programming/file-io.md):**
> `read`/`write` trả số byte **thực sự** xử lý (có thể **ít hơn** yêu cầu — *short read/write*); phải xử lý vòng lặp.
> `= -1` | Lỗi — riêng `EINTR` là *bị signal cắt*, thường phải **gọi lại**

**Trích [tcp-ip.md §3](../../../13-networking/tcp-ip.md)** — dòng cần nối vào:
> | Mô hình | Luồng byte (stream) | Gói rời rạc (datagram) |

> 📌 **Lỗ hổng tài liệu phát hiện trong phiên:** không file nào trong repo nói thẳng *"TCP không giữ ranh giới message ⇒ phải tự framing"* — ý này chỉ nằm rải trong bảng so sánh TCP/UDP. Đó là lý do ứng viên biết short read (câu (b) đúng hết) mà không nối được sang TCP. **Đáng bổ sung một mục vào `13-networking/tcp-ip.md`.**

**Chốt:** *"TCP giao đủ byte, đúng thứ tự — nhưng không giao đúng lô. Mọi `read` phải lặp, mọi protocol phải tự đóng khung."*

**Lần sau sẽ hỏi:** đưa code có `read_full` đã đúng, nhưng protocol dùng **delimiter `\n`** thay length-prefix → *"gói 1 MB không có `\n` thì chuyện gì xảy ra?"* (bắt nêu giới hạn buffer + DoS).

**Ôn:** [file-io.md §1](../../../04-linux-system-programming/file-io.md) · [tcp-ip.md §3](../../../13-networking/tcp-ip.md) · bank [LNX-005](../bank/linux-sysprog.md), [LNX-027](../bank/linux-sysprog.md)
</details>

---

### Câu 3 · LNX-028 · 🟡 ⭐ · **3/4**

Hai process cùng ghi vào một file log (`/var/log/scanner.log`): daemon chính và một tool phụ chạy định kỳ. Cả hai dùng chung hàm này:

```c
void append_line(int fd, const char* line, size_t len) {
    lseek(fd, 0, SEEK_END);
    write(fd, line, len);
}
```

Khách báo file log **thỉnh thoảng mất dòng**, và có lúc thấy một dòng bị cắt cụt rồi dính đuôi dòng khác.

**(a)** Vẽ ra chuỗi sự kiện dẫn tới việc **mất một dòng**.
**(b)** Sửa thế nào? Cơ chế nào ở tầng kernel làm cách sửa đó đúng?

**🔁 Follow-up (e):** `O_APPEND` đảm bảo *vị trí ghi* nguyên tử. Nó có đảm bảo **cả dòng** được ghi liền một mạch không? Nếu `len` = 200 KB thì câu trả lời có đổi không?
**🔁 Follow-up (f):** Mẫu bug ở đây là "hai bước, bị chen vào giữa". Kể **một cặp khác** trong Linux API dính đúng mẫu đó, và cờ/syscall gộp nào giải quyết.

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án đầy đủ</summary>

**Bạn trả lời:**
- (a) 🟡 *"daemon `lseek` về cuối và ghi được một ít ký tự, tại thời điểm này tool phụ gọi hàm, `lseek` ngay về cuối dòng mới nhất (dòng daemon viết dang dở) và ghi đè lên → 2 dòng từ 2 nơi dính nhau."* — khoanh **trúng khe hở**, nhưng trình tự chưa chuẩn.
- (b) ✅ *"chỉ dùng 1 `write` duy nhất với cờ `O_APPEND`, khi này seek về cuối và ghi dữ liệu được kernel đảm bảo atomic"*.
- (e) ❌ *"có, đây là đảm bảo từ kernel [...] nếu `len` = 200 KB và cố định thì không có lỗi"* — **khẳng định vô điều kiện**. (➕ tự thêm ý hay: *"nên truyền `len` cố định thay vì `strlen`"*.)
- (f) ✅ `O_CREAT | O_EXCL`, kèm mô tả race check-then-create đúng.

**✅ Được:** lõi cơ chế đúng — khe hở giữa `lseek` và `write`, và cách bịt bằng một syscall gộp. Nhận ra mẫu và tự tổng quát hoá sang `O_EXCL` là dấu hiệu hiểu chứ không thuộc.

**❌ Vì sao chưa 4 — hai chỗ:**

**① Trình tự ở (a) chưa chuẩn.** Ca *mất hẳn một dòng* là cả hai đọc **cùng một** offset cũ, không phải B đọc offset giữa chừng của A:
```
A: lseek → 1000
                    B: lseek → 1000          (CÙNG offset!)
                    B: write("dòng B")       file dài 1001
A: write("dòng A")  → ghi tại 1000, ĐÈ SẠCH dòng B → dòng B BIẾN MẤT
```

**② Câu (e) — được hỏi về BIÊN của đảm bảo, lại trả lời vô điều kiện.** Đây mới là chỗ tách mid với senior: biết một đảm bảo tồn tại là T1; biết **nó hết hiệu lực khi nào** là T2.

Đo thật (`gcc -std=c11 -Wall -Wextra`, 2 process × 200 write × 1 MB):
```
FILE + O_APPEND: doc 419430400 byte, so dong bi TRON: 0
```
✅ **File thường: bạn đúng** — Linux giữ inode lock nên `write()` nguyên tử với nhau, kể cả 1 MB.

Nhưng đổi fd thành **pipe** (chính là `stdout` → journald ở câu 1!), reader hút chậm, 4 writer:
```
write=  4096 byte (PIPE_BUF=4096) | dai khong phai boi cua len: 0     (nguyên vẹn)
write=  8192 byte (PIPE_BUF=4096) | dai khong phai boi cua len: 104   <-- BI XE
write= 65536 byte (PIPE_BUF=4096) | dai khong phai boi cua len: 944   <-- BI XE
```
Ranh giới là **`PIPE_BUF` = 4096**: dưới ngưỡng nguyên tử, trên ngưỡng kernel được phép xé và hai nguồn trộn vào nhau.
> 🔬 Ghi chú trung thực: hai lần đo đầu ra **0** vì reader hút quá nhanh, hai writer không thực sự chồng nhau — phải làm reader chậm + 4 writer mới ép ra được. Tức là bug này **hiếm nhưng thật**, đúng kiểu 3–4/500 của câu 1.

**Hai biên còn lại — đều đổi quyết định:**
- **Short write** — `write()` được phép trả về **< len**; phần còn lại phải gọi `write()` lần hai ⇒ **khe hở mở lại**. Chính là khái niệm đã trả lời đúng ở câu 2 cho `read`, nhưng chưa nối sang `write`.
- **NFS** — [file-io.md](../../../04-linux-system-programming/file-io.md) đã ghi:
  > ⚠️ Lưu ý: `O_APPEND` **không** đảm bảo nguyên tử trên **NFS** ở các bản cũ, vì NFS không hỗ trợ ngữ nghĩa append phía server.

**Quy tắc rút ra:** *một bản ghi log = một `write()`, và giữ nó nhỏ (≤ 4 KB)*. Đó là lý do mọi thư viện log nghiêm túc format vào buffer trước rồi bắn **một** `write`.

**Trích tài liệu gốc [file-io.md §2.2 Atomicity](../../../04-linux-system-programming/file-io.md):**
> ```c
> /* ❌ SAI: hai lệnh, có KHE HỞ ở giữa */
> lseek(fd, 0, SEEK_END);        // ① tìm tới cuối
>                                //    ← process khác ghi chen vào ĐÂY
> write(fd, buf, len);           // ② ghi vào offset đã CŨ → ĐÈ MẤT dữ liệu bên kia
>
> /* ✅ ĐÚNG: một syscall, kernel đảm bảo nguyên tử */
> fd = open(path, O_WRONLY | O_APPEND);
> write(fd, buf, len);           // seek-tới-cuối + ghi là MỘT thao tác không chia cắt
> ```

**Chốt:** *"`O_APPEND` nguyên tử — trên file thường, trong một `write()`. Ra khỏi ba điều kiện đó (pipe > PIPE_BUF, short write, NFS) thì đảm bảo hết hiệu lực."*

**Lần sau sẽ hỏi:** góc thứ ba — *"nhiều thread trong CÙNG một process ghi qua cùng một `fd`: `O_APPEND` còn đủ không? `pwrite` khác gì?"* (offset chung giữa thread).

**Ôn:** [file-io.md §2.2](../../../04-linux-system-programming/file-io.md) · bank [LNX-028](../bank/linux-sysprog.md), [LNX-021](../bank/linux-sysprog.md)
</details>

---

### Câu 4 · LNX-012 — **đã ra đề, chưa trả lời** (không tính điểm)

> Giữ nguyên đề để buổi sau hỏi lại từ trạng thái trống.

Daemon mở socket lắng nghe cổng 9100, mỗi khi có job in thì `fork()` + `exec()` một script xử lý ảnh (`/usr/bin/convert`). Script chạy vài giây rồi thoát.

Một hôm bạn `systemctl restart scanner-daemon`. Daemon chết sạch, nhưng process mới **không bind được cổng 9100**: `EADDRINUSE`. `ss -ltnp` cho thấy cổng vẫn bị giữ — bởi một process `convert` đang chạy dở.

**(a)** Vì sao `convert` — chương trình hoàn toàn không liên quan — lại đang giữ socket của bạn?
**(b)** Sửa bằng gì, và đặt ở đâu?

---

## 🎯 Ba lỗ hổng ưu tiên

| # | Lỗ hổng | Vì sao quan trọng với JD | Ôn ở đâu |
|---|---|---|---|
| **1** | **`EINTR` xử lý theo *ý định*, không dập bằng cờ** — `SA_RESTART` chữa nhầm bệnh và tạo bug mới | JD ghi thẳng *"Linux userspace + debugging"*. Daemon không tắt được bằng SIGTERM là bug kinh điển | [processes-signals.md §SA_RESTART](../../../04-linux-system-programming/processes-signals.md) · bank LNX-027, **LNX-030** |
| **2** | **TCP là luồng byte, không có ranh giới message** ⇒ mọi `read` phải lặp, mọi protocol phải tự framing | Thiết bị đọc mã vạch nói chuyện qua socket. Lớp bug *"lab đúng, khách sai"* — thứ dùng để đo kinh nghiệm thật | [file-io.md §1](../../../04-linux-system-programming/file-io.md) · [tcp-ip.md §3](../../../13-networking/tcp-ip.md) · bank LNX-005 |
| **3** | **Biên của một đảm bảo nguyên tử** — `PIPE_BUF`, short write, NFS | Phân biệt *"biết cờ"* với *"biết khi nào cờ hết tác dụng"* — đúng ranh giới T2 tách mid với senior | [file-io.md §2.2](../../../04-linux-system-programming/file-io.md) · bank LNX-028 |

## 💡 Điểm sáng

Phần nền signal-safety **thực sự chắc lại** so với phiên 13/08: tự dựng được kịch bản deadlock heap, tự giải thích được vì sao lỗi hiếm (3–4/500). Đó là tiến bộ thật, không phải may — nền OS-020 coi như đã vá.

## 🔬 Chủ đề xuyên suốt của phiên (đáng nhớ nhất)

Cả **ba** câu đều là **cùng một lớp bug**: đúng ở môi trường dev, sai hiếm ở hiện trường, vì cửa sổ race hẹp.

| Câu | Cửa sổ race | Vì sao dev không thấy |
|---|---|---|
| OS-020 | SIGTERM tới đúng lúc `malloc` giữ khoá heap | 3–4/500 lần |
| LNX-005 | header vắt qua hai TCP segment | LAN RTT 0.1 ms, không bao giờ tách |
| LNX-028 | hai `lseek` cùng đọc một offset | một process thì không có ai chen |

⇒ **Bài học chung: "test không lỗi" không chứng minh được gì về code có race. Phải suy luận theo cơ chế, không theo kết quả chạy.** Đây cũng đúng là điều tôi phải tự áp dụng khi đo `PIPE_BUF` — hai lần đầu ra 0 không có nghĩa là không có bug.

---
⬅️ [Sessions index](README.md) · [weak-register](../weak-register.md) · [datalogic-plan §📍](../../study-plans/datalogic-plan.md)
