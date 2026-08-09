# TLPI — Cụm 07: Sockets & Terminals (ch. 56–62, 64)

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 56–62 (tr. 1149–1324) và 64 (tr. 1375–1400).
> **Định vị cụm này:** đây là **phần tra cứu**, không phải cụm 🎯. Góc phỏng vấn của TCP/IP (handshake, TCP vs UDP, TIME_WAIT) đã có ở [13-networking](../../13-networking/). Cụm này tập trung vào phần **TLPI làm tốt hơn**: *API socket ở tầng syscall*, **UNIX domain socket** (thứ 13-networking không phủ), và **thiết kế server**.

---

## 1. Socket là gì — ba trục phân loại (ch. 56, tr. 1149–1164)

Một socket được xác định bởi **domain** (giao tiếp ở đâu) và **type** (ngữ nghĩa truyền).

### 1.1. Domain — Table 56-1 (tr. 1150)

| Domain | Giao tiếp giữa | Định dạng địa chỉ | Struct |
|---|---|---|---|
| **`AF_UNIX`** | **Cùng một máy** | **đường dẫn** trên filesystem | `sockaddr_un` |
| `AF_INET` | Máy nối qua mạng IPv4 | IPv4 32-bit + port 16-bit | `sockaddr_in` |
| `AF_INET6` | Máy nối qua mạng IPv6 | IPv6 128-bit + port 16-bit | `sockaddr_in6` |

> 🆕 Ghi chú lịch sử của Kerrisk (tr. 1150): bạn sẽ thấy cả `PF_UNIX` lẫn `AF_UNIX` trong code — *AF* = address family, *PF* = protocol family. Ban đầu người ta hình dung **một** protocol family có thể hỗ trợ **nhiều** address family, nhưng *"in practice, no protocol family supporting multiple address families has ever been defined"*. Mọi cài đặt đều định nghĩa `PF_*` **đồng nghĩa** `AF_*`. SUSv3 chỉ quy định `AF_*` → **dùng `AF_*`**.

### 1.2. Type — Table 56-2 (tr. 1151)

| Tính chất | **`SOCK_STREAM`** | **`SOCK_DGRAM`** |
|---|---|---|
| Giao đáng tin cậy? | **Có** | Không |
| Giữ ranh giới message? | **Không** (byte stream) | **Có** |
| Hướng kết nối? | **Có** | Không |
| Trong `AF_INET` là | **TCP** | **UDP** |
| Trong `AF_UNIX` là | luồng byte cục bộ | datagram cục bộ — ⚠️ **vẫn đáng tin cậy** |

> ⚠️ **Điểm dễ sai:** `SOCK_DGRAM` trong **UNIX domain** *có* đảm bảo giao đúng và đúng thứ tự — vì không đi qua mạng, không có mất gói. Bảng "không đáng tin cậy" ở trên nói về `AF_INET`. Nên UNIX domain datagram cho bạn **ranh giới message + độ tin cậy cùng lúc** — thứ UDP không có.

### 1.3. Luồng syscall — hình phải nhớ

```
        SERVER                                    CLIENT
   ┌─────────────────┐                      ┌─────────────────┐
   │   socket()      │                      │   socket()      │
   ├─────────────────┤                      └────────┬────────┘
   │   bind()        │  gán địa chỉ                  │
   ├─────────────────┤                               │
   │   listen()      │  chuyển sang PASSIVE          │
   ├─────────────────┤  + tạo hàng đợi backlog       │
   │   accept()      │  ⏸ chặn, chờ                  │
   │        ▲        │◄──────── 3-way handshake ─────┤  connect()
   │        │        │                               │
   │  trả về fd MỚI  │                               │
   │  cho kết nối này│                               │
   ├─────────────────┤                      ┌────────┴────────┐
   │  read()/write() │◄════ dữ liệu ═══════►│  write()/read() │
   ├─────────────────┤                      ├─────────────────┤
   │  close()        │                      │  close()        │
   └─────────────────┘                      └─────────────────┘
```

*(vẽ lại theo Figure 56-1, tr. 1156)*

**Điểm hay bị hỏi:** `accept()` **trả về một fd MỚI** cho từng kết nối; fd đang `listen` vẫn tiếp tục nghe. Đó là lý do một server phục vụ được nhiều client — mỗi client một fd riêng, còn fd nghe thì bất biến.

**`listen(fd, backlog)`** — `backlog` giới hạn số kết nối **đã hoàn tất handshake nhưng chưa được `accept()`** (Figure 56-2, tr. 1157). Đầy thì client mới bị từ chối hoặc phải chờ.

---

## 2. UNIX domain socket — phần đáng giá nhất cụm này 🆕

Đây là chỗ TLPI cho giá trị mà tài liệu mạng thông thường không có.

```c
struct sockaddr_un addr;
memset(&addr, 0, sizeof(addr));
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, "/tmp/mysock", sizeof(addr.sun_path) - 1);

int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
unlink("/tmp/mysock");                      // ⚠️ bind() lỗi EADDRINUSE nếu file còn
bind(sfd, (struct sockaddr *) &addr, sizeof(addr));
listen(sfd, 5);
```

**Ba điều phải biết:**

1. **`bind()` TẠO một file trên filesystem** (loại socket, thấy bằng `ls -l` với ký tự `s`). File này **không tự xoá** khi process chết → lần chạy sau `bind()` trả `EADDRINUSE`. Phải `unlink()` trước khi `bind`, và nên `unlink()` khi thoát.
2. **Quyền truy cập theo quyền file** — đây là ưu điểm lớn: dùng `chmod`/`chown` trên đường dẫn socket để kiểm soát ai kết nối được. TCP socket không có cơ chế tương đương.
3. **`socketpair()`** tạo **cặp socket UNIX domain đã nối sẵn**, hai chiều — thay thế tốt hơn `pipe()` cho giao tiếp cha–con:
   ```c
   int sv[2];
   socketpair(AF_UNIX, SOCK_STREAM, 0, sv);   // sv[0] ↔ sv[1], HAI CHIỀU
   ```

### 2.1. Truyền file descriptor qua `SCM_RIGHTS` — cơ chế độc nhất

Đây là thứ **chỉ UNIX domain socket làm được** (§61.13.3):

```
   Process A                                   Process B
   ┌──────────────┐                            ┌──────────────┐
   │ fd 7 ────────┼──► file/socket X           │              │
   └──────┬───────┘                            └──────────────┘
          │  sendmsg() với control message SCM_RIGHTS
          └──────────────────────────────────────────►
   ┌──────────────┐                            ┌──────────────┐
   │ fd 7 ────────┼──► file/socket X ◄─────────┼──── fd 12    │
   └──────────────┘                            └──────────────┘
       ⇒ Kernel tạo fd MỚI trong B, trỏ vào CÙNG file description.
         KHÔNG phải gửi số 7 — số fd vô nghĩa với process khác.
```

**Vì sao không thể chỉ gửi số fd:** fd là **chỉ mục vào bảng riêng của từng process** ([cụm 01 §2.3](01-concepts-and-file-io.md)). Số 7 ở A chẳng liên quan gì tới số 7 ở B. Kernel phải **thật sự tạo một mục mới** trong bảng fd của B, trỏ vào cùng file description.

🆕 Ứng dụng thật: systemd **socket activation** (systemd mở socket nghe rồi trao fd cho service), và mô hình privilege separation (process đặc quyền mở file/port rồi trao fd cho process không đặc quyền).

### 2.2. Khi nào chọn UNIX domain thay vì TCP loopback

| | UNIX domain | TCP qua `127.0.0.1` |
|---|---|---|
| Tốc độ | **Nhanh hơn** — không qua stack TCP/IP, không checksum, không đóng gói | Chậm hơn |
| Kiểm soát truy cập | **Quyền file** (`chmod`/`chown`) | Chỉ theo địa chỉ/port |
| Truyền fd | **Được** (`SCM_RIGHTS`) | Không |
| Xác thực peer | **Được** (`SO_PEERCRED` → PID/UID/GID thật của bên kia) | Không |
| Qua mạng | Không | Được |

> **Câu chốt:** hai process **cùng máy** thì gần như luôn nên dùng **UNIX domain socket**, không dùng TCP loopback — trừ khi bạn dự tính sau này tách sang máy khác.

---

## 3. Thiết kế server (ch. 60, tr. 1239–1252)

### 3.1. Ba kiến trúc

```
   ① ITERATIVE — một client tại một thời điểm
      accept() → phục vụ xong → accept() tiếp
      ✅ đơn giản nhất   ❌ client sau phải chờ hết client trước
      → chỉ hợp khi phục vụ mỗi client rất NHANH (vd UDP echo)

   ② CONCURRENT — mỗi client một process/thread
      accept() → fork() → con phục vụ, cha quay lại accept() ngay
      ✅ client không chặn nhau   ❌ chi phí tạo process/thread mỗi kết nối

   ③ PREFORKED / PRETHREADED — tạo sẵn một "server pool"
      lúc khởi động tạo sẵn N con → mỗi con tự accept() trên fd nghe CHUNG
      ✅ bỏ được chi phí tạo   ❌ phải quản lý kích thước pool
```

**Kerrisk nói rõ vì sao cần ③** (§60.4, tr. 1245): *"for very high-load servers (for example, web servers handling thousands of requests per minute), the cost of creating a new child (or even thread) for each client imposes a significant burden"* — và ông trỏ thẳng về §28.3, tức **Table 28-3** mà [cụm 02 §2.3](02-processes-and-exec.md) đã dùng: `fork+exec` mất 135 giây cho 100 000 lần.

**Cách preforked hoạt động** (tr. 1245–1246): cha tạo socket nghe **trước** khi `fork`, mỗi con **kế thừa fd nghe** qua `fork()`, rồi **tất cả cùng chặn trong `accept()`** trên fd đó. Khi có kết nối mới, **chỉ một** con hoàn tất `accept()`.

> ⚠️ Kerrisk lưu ý: trên một số cài đặt **cũ**, `accept()` không nguyên tử nên phải bọc bằng cơ chế loại trừ (vd file lock). Trên Linux hiện đại thì không cần. 🆕 Vấn đề còn lại là **thundering herd** — mọi con cùng được đánh thức nhưng chỉ một cái thắng; Linux hiện đại đã giảm nhiều, và **`SO_REUSEPORT`** giải triệt để bằng cách cho mỗi worker một socket nghe riêng, kernel tự chia đều kết nối.

**Hai việc cha phải làm trong mô hình ②:** thu hoạch con bằng handler `SIGCHLD` có **vòng lặp `waitpid(WNOHANG)`** (nếu không → zombie tích tụ, [cụm 03 §1.2](03-signals-and-timers.md)), và **đóng fd không dùng** — cha đóng fd kết nối, con đóng fd nghe.

### 3.2. `shutdown()` vs `close()` (§61.2, tr. 1231–1232)

```c
shutdown(fd, SHUT_WR);     // đóng NỬA GHI: peer nhận EOF, ta VẪN ĐỌC được
shutdown(fd, SHUT_RD);     // đóng nửa đọc (⚠️ không dùng được có nghĩa với TCP)
shutdown(fd, SHUT_RDWR);   // đóng cả hai chiều
```

Hai khác biệt so với `close()` mà Kerrisk nêu (tr. 1232):

1. **`shutdown()` đóng kênh bất kể còn fd khác trỏ tới socket** — tức nó tác động lên **file description**, không phải fd. `close()` chỉ thật sự đóng khi fd **cuối cùng** đóng.
2. **`shutdown()` KHÔNG đóng file descriptor** — kể cả `SHUT_RDWR`. Vẫn phải `close()` để giải phóng fd.

**Dùng để làm gì — "half-close":** client gửi hết yêu cầu rồi `shutdown(SHUT_WR)` → server nhận **EOF** nên biết đã hết dữ liệu và bắt đầu trả lời, trong khi client **vẫn đọc được** phản hồi. Không có `shutdown` thì client phải `close()`, mà `close()` cắt luôn đường đọc.

---

## 4. Ít quan trọng — 1 dòng + tham chiếu

- **§56.6 — `sendto`/`recvfrom` cho datagram** (tr. 1160): datagram không cần `connect`; nhưng `connect()` một datagram socket vẫn có ích (đặt peer mặc định + nhận được lỗi ICMP bất đồng bộ).
- **§57 — UNIX domain chi tiết** (tr. 1165): `sun_path` giới hạn ~108 byte; **abstract socket namespace** của Linux (`sun_path[0] == '\0'`) — **không tạo file trên fs**, tự biến mất khi process cuối đóng.
- **§58 — Nền TCP/IP** (tr. 1179): tầng, đóng gói, MTU/phân mảnh, checksum, TCP state machine. **Đã có ở [13-networking/tcp-ip.md](../../13-networking/tcp-ip.md)** — đọc bên đó thay vì đây.
- **§59 — Internet domain socket** (tr. 1197): `sockaddr_in`, **byte order** (`htons`/`htonl` — big-endian là network byte order), và **`getaddrinfo()`** thay cho `gethostbyname()` đã lỗi thời (khả chuyển IPv4/IPv6).
- **§59.13 — `/etc/services`, `getservbyname()`** (tr. 1232).
- **§60.5 — `inetd`** (tr. 1249): superserver, nghe hộ nhiều dịch vụ rồi mới `fork`+`exec` khi có kết nối. Ngày nay là **systemd socket activation**.
- **§61.1 — Truyền dữ liệu ngoài băng (out-of-band)** (tr. 1229): TCP urgent data — hiếm dùng, thiết kế có vấn đề.
- **§61.3–61.5 — `recv`/`send` với cờ, `sendfile()`** (tr. 1233): **`sendfile()` là zero-copy** — chuyển dữ liệu file → socket **không qua userspace**. Đây là kỹ thuật lớn cho web server tĩnh.
- **§61.9 — `getsockopt`/`setsockopt`, `SO_REUSEADDR`** (tr. 1244): 🆕 `SO_REUSEADDR` cho phép `bind()` lại port khi socket cũ còn ở **TIME_WAIT** — gần như **bắt buộc** cho mọi server, nếu không thì restart server sẽ lỗi `EADDRINUSE`.
- **§61.11 — `SO_PEERCRED`** (tr. 1247): lấy PID/UID/GID **thật** của peer trên UNIX domain socket — cơ chế xác thực đáng tin vì kernel cung cấp.
- **§62 — Terminals** (tr. 1289): `termios`, chế độ canonical vs raw, `tcsetattr()`. Cần khi làm việc với **UART/serial console** — xem [05/bus-protocols §6](../../05-drivers-device-tree/bus-protocols.md).
- **§64 — Pseudoterminal** (tr. 1375): cặp master/slave giả lập terminal — nền của `ssh`, `screen`, `tmux`, emulator terminal.

---

## 5. Góc interview

<details><summary>1) Khi nào dùng UNIX domain socket thay vì TCP qua loopback?</summary>

**Gần như luôn dùng UNIX domain socket** khi hai process ở **cùng một máy**, trừ khi bạn dự tính tách chúng sang máy khác sau này.

| | UNIX domain | TCP `127.0.0.1` |
|---|---|---|
| Tốc độ | **Nhanh hơn** — không qua stack TCP/IP, không checksum, không đóng gói header | Chậm hơn |
| Kiểm soát truy cập | **Quyền file** — `chmod`/`chown` trên đường dẫn socket | Chỉ theo địa chỉ/port |
| Truyền **file descriptor** | **Được** — `SCM_RIGHTS` | Không |
| Xác thực peer | **`SO_PEERCRED`** cho PID/UID/GID **thật** (kernel cung cấp) | Không |
| Ranh giới message + tin cậy | **Có cả hai** với `SOCK_DGRAM` | UDP mất gói, TCP mất ranh giới |
| Qua mạng | Không | Được |

Điểm ít người biết: **`SOCK_DGRAM` trong UNIX domain vẫn đáng tin cậy** — không qua mạng nên không mất gói. Nên bạn có **ranh giới message *và* độ tin cậy** cùng lúc, thứ mà UDP không cho.

**Ba chi tiết vận hành phải nhớ:**
1. `bind()` **tạo một file** trên filesystem; file **không tự xoá** khi process chết → `unlink()` trước khi `bind`, và `unlink()` lúc thoát, nếu không lần chạy sau lỗi `EADDRINUSE`.
2. **Abstract namespace** của Linux (`sun_path[0] = '\0'`) tránh được vấn đề trên — không tạo file, tự biến mất khi process cuối đóng. Nhưng là mở rộng riêng của Linux.
3. **`socketpair()`** cho một cặp đã nối sẵn, **hai chiều** — tốt hơn `pipe()` cho giao tiếp cha–con.
</details>

<details><summary>2) Làm sao truyền một file descriptor từ process này sang process khác?</summary>

**Chỉ có một cách: `sendmsg()`/`recvmsg()` với control message `SCM_RIGHTS` trên UNIX domain socket.**

**Vì sao không thể chỉ gửi con số:** fd là **chỉ mục vào bảng file descriptor RIÊNG của từng process**. Số 7 trong process A chẳng liên quan gì tới số 7 trong process B — nó có thể là fd đã đóng, hoặc trỏ vào file hoàn toàn khác.

Kernel phải **thật sự tạo một mục mới** trong bảng fd của B, trỏ vào **cùng file description** mà fd của A đang trỏ tới. Số fd nhận được ở B thường **khác** số ở A, nhưng cả hai chia sẻ offset và status flags — giống hệt quan hệ sau `fork()` ([cụm 02 §2.4](02-processes-and-exec.md)).

**Dùng để làm gì trong thực tế:**
- **systemd socket activation** — systemd mở socket nghe lúc boot, giữ nó, rồi trao fd cho service khi service khởi động. Nhờ đó service restart mà **không mất kết nối đang chờ**, và không có cửa sổ "port chưa mở".
- **Privilege separation** — một process chạy `root` mở file hoặc bind port < 1024, rồi trao fd cho process không đặc quyền xử lý. Process xử lý **không bao giờ** cần quyền root.
- **Cân bằng tải** — process nhận `accept()` rồi phân phát fd kết nối cho các worker.

Đây là một trong những lý do mạnh nhất để chọn UNIX domain socket thay vì TCP loopback.
</details>

<details><summary>3) So sánh iterative, concurrent và preforked server. Chọn cái nào?</summary>

**① Iterative** — phục vụ trọn một client rồi mới `accept()` client tiếp.
- ✅ Đơn giản nhất, không lo đồng bộ.
- ❌ Client sau **chờ hết** client trước.
- Hợp khi: mỗi yêu cầu phục vụ **rất nhanh và có giới hạn thời gian rõ** — ví dụ UDP echo, hoặc dịch vụ tra cứu nhỏ.

**② Concurrent (fork/thread mỗi client)** — `accept()` rồi `fork()`, con phục vụ, cha quay lại `accept()` ngay.
- ✅ Client không chặn nhau; code vẫn đơn giản (mỗi con viết như chương trình tuần tự).
- ❌ **Chi phí tạo process/thread cho mỗi kết nối**.
- Hai việc bắt buộc: cha phải thu hoạch con bằng handler `SIGCHLD` có **vòng lặp `waitpid(WNOHANG)`** (không thì zombie tích tụ), và **đóng fd không dùng** ở mỗi bên.

**③ Preforked/prethreaded** — tạo sẵn **server pool** lúc khởi động; mỗi con phục vụ xong thì lấy client tiếp theo thay vì kết thúc.
- ✅ Bỏ hẳn chi phí tạo. TLPI dẫn thẳng số đo: `fork+exec` mất **135 giây cho 100 000 lần**.
- ❌ Phải **quản lý kích thước pool** — cha theo dõi số con rảnh, tải cao thì tăng, tải thấp thì giảm (thừa process cũng làm chậm hệ thống).
- Cách cài: cha tạo socket nghe **trước** khi `fork`, mỗi con kế thừa fd nghe và **cùng chặn trong `accept()`**; chỉ một con hoàn tất.

**④ (Ngoài phạm vi TLPI, nên nêu thêm) Event loop + `epoll`** — một luồng xử lý hàng nghìn kết nối non-blocking. Đây là câu trả lời cho **C10K**, xem [cụm 05 §7 câu 3](05-alternative-io-models.md).

**Chọn thế nào:** yêu cầu ngắn + số client ít → ①. Yêu cầu dài, số client vừa, cần code đơn giản → ②. Tải cao, kết nối ngắn → ③. **Rất nhiều kết nối đồng thời, phần lớn nhàn rỗi** → ④ (thường ghép: N event loop × M worker, mỗi lõi một loop, dùng `SO_REUSEPORT`).
</details>

<details><summary>4) <code>shutdown()</code> khác <code>close()</code> thế nào? "Half-close" để làm gì?</summary>

**`close()`** giảm số tham chiếu tới fd; socket chỉ thật sự đóng khi fd **cuối cùng** trỏ tới nó được đóng. Nó đóng **cả hai chiều**.

**`shutdown(fd, how)`** đóng **một hoặc cả hai chiều** của socket, với hai khác biệt bản chất:
1. Nó tác động lên **file description**, nên **đóng kênh ngay cả khi còn fd khác** trỏ tới socket đó (vd sau `fork()` hoặc `dup()`).
2. Nó **KHÔNG đóng file descriptor** — kể cả `SHUT_RDWR`. Vẫn phải gọi `close()` để giải phóng fd.

**Half-close (`SHUT_WR`) — công dụng chính:**
```
   Client                                Server
   write(yêu cầu...)  ──────────────►   read() ...
   shutdown(fd, SHUT_WR) ───────────►   read() trả 0 = EOF
                                        → "client gửi xong rồi, giờ mình trả lời"
   read(phản hồi) ◄─────────────────    write(phản hồi)
   ✅ client VẪN ĐỌC ĐƯỢC vì chỉ đóng nửa GHI
```

Không có `shutdown`, client muốn báo "hết dữ liệu" thì chỉ còn cách `close()` — mà `close()` **cắt luôn đường đọc**, nên không nhận được phản hồi. Đây đúng là mẫu của các giao thức kiểu "gửi hết rồi mới nhận" (một số dạng HTTP, `sort`, các bộ lọc dữ liệu).

⚠️ `SHUT_RD` **không dùng được có nghĩa với TCP** — TCP không có cách báo cho peer biết bạn ngừng nhận.

**Liên quan thường bị hỏi kèm — `SO_REUSEADDR`:** sau khi server đóng, socket ở trạng thái **TIME_WAIT** khiến `bind()` lại cùng port trả `EADDRINUSE`. `setsockopt(SO_REUSEADDR)` cho phép bind lại — gần như **bắt buộc** cho mọi server, nếu không thì restart là lỗi.
</details>

---

## 6. Đọc thêm (tùy chọn)

- **Góc phỏng vấn của TCP/IP** (handshake, TCP vs UDP, TIME_WAIT, cửa sổ trượt): [13-networking/tcp-ip.md](../../13-networking/tcp-ip.md) và [sockets-and-protocols.md](../../13-networking/sockets-and-protocols.md) — đọc bên đó, gọn hơn cho ôn thi.
- Bank: `NET-*` trong [networking.md](../../14-prep/mock-interview/bank/networking.md); `SD-*` cho câu thiết kế server.
- Event loop cho server nhiều kết nối: [cụm 05](05-alternative-io-models.md).
- Terminal/serial thực chiến: [05/bus-protocols.md §6](../../05-drivers-device-tree/bus-protocols.md).

---
⬅️ [Cụm 06 — IPC](06-ipc.md) · [README cụm TLPI](README.md)
