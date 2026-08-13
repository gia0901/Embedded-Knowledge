# LNX — Linux System Programming

> Domain `LNX`. Track dùng: `linux-sysprog`, `bsp`, `cpp-system`, `ostep`.

---

#### LNX-001 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**File descriptor là gì?**
<details><summary>Đáp án</summary>

Một số nguyên không âm, là chỉ mục vào bảng fd riêng của process; mỗi entry trỏ tới open file description trong kernel (offset, cờ) và tới đối tượng thực (inode, socket...). fd 0/1/2 = stdin/stdout/stderr. "Everything is a file" — file, pipe, socket, device đều thao tác qua fd với cùng syscall.
</details>

#### LNX-002 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md), [kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md)
**Syscall là gì và vì sao có chi phí?**
<details><summary>Đáp án</summary>

Là cách user yêu cầu kernel làm việc đặc quyền (I/O, cấp bộ nhớ): wrapper libc đặt số syscall + tham số, thực thi lệnh trap, CPU chuyển sang kernel mode, kernel thực hiện rồi trả về. Chi phí từ chuyển ngữ cảnh user↔kernel (đổi mức đặc quyền, lưu/khôi phục trạng thái) và đôi khi context switch.
</details>

#### LNX-003 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Blocking và non-blocking I/O khác nhau gì?**
<details><summary>Đáp án</summary>

Blocking (mặc định): `read` trên fd chưa có dữ liệu làm thread ngủ chờ. Non-blocking (`O_NONBLOCK`): trả về ngay, nếu chưa có dữ liệu thì `errno == EAGAIN`. Non-blocking cho phép một thread phục vụ nhiều fd (nền cho event loop với epoll).
</details>

#### LNX-004 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**fork và exec khác nhau? Vì sao dùng chung?**
<details><summary>Đáp án</summary>

`fork` tạo process con (bản sao COW). `exec` thay thế image hiện tại bằng chương trình khác, giữ PID, không trả về nếu thành công. Dùng chung để chạy chương trình mới mà vẫn giữ process cha; khoảng giữa cho con tùy biến môi trường (redirect fd, đổi uid) — chính là cách shell làm `cmd > file`.
</details>

#### LNX-005 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**read() trả về ít byte hơn yêu cầu — lỗi không? Xử lý sao?**
<details><summary>Đáp án</summary>

Không phải lỗi (short read). `read`/`write` trả số byte thực sự xử lý, có thể ít hơn (pipe/socket mới có một phần, bị signal ngắt). Xử lý bằng vòng lặp tiếp tục cho phần còn lại tới khi đủ/EOF/lỗi, và xử lý `EINTR` bằng thử lại.
</details>

#### LNX-006 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Vì sao nên dùng sigaction thay vì signal?**
<details><summary>Đáp án</summary>

`signal()` có ngữ nghĩa không thống nhất giữa nền tảng (có hệ reset handler về mặc định sau lần đầu) và không kiểm soát rõ mask/restart. `sigaction` xác định, di động: kiểm soát signal mask khi handler chạy, cờ `SA_RESTART` (tự thử lại syscall bị ngắt), `SA_SIGINFO`.
</details>

#### LNX-007 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Phân biệt stdio buffer, page cache, và fsync.**
<details><summary>Đáp án</summary>

stdio buffer ở user space (libc), gom dữ liệu giảm số syscall; `fflush` đẩy xuống kernel. Page cache ở kernel, cache nội dung file trong RAM; `write` thành công chỉ đảm bảo tới page cache. `fsync` ép kernel ghi page cache xuống disk vật lý — quan trọng cho độ bền (embedded mất điện).
</details>

#### LNX-008 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Cơ chế IPC nào nhanh nhất, đánh đổi gì?**
<details><summary>Đáp án</summary>

Shared memory — hai process map cùng vùng nhớ vật lý, đọc/ghi trực tiếp không copy qua kernel (các IPC khác copy 2 lần). Đánh đổi: kernel không tự đồng bộ, phải tự dùng semaphore/mutex để tránh race.
</details>

#### LNX-009 · 🟠 · concept · ⭐ · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**epoll khác select/poll thế nào? Vì sao scale tốt hơn?**
<details><summary>Đáp án</summary>

**Khác biệt gốc: *ai giữ danh sách fd*.**
- `select`/`poll`: **stateless** — mỗi lần gọi, user phải truyền **toàn bộ** tập fd xuống kernel, kernel **quét tuyến tính tất cả** để xem cái nào sẵn sàng, rồi copy kết quả ngược lên. Làm lại từ đầu **mỗi vòng lặp**.
- `epoll`: **stateful** — đăng ký fd **một lần** bằng `epoll_ctl`; kernel giữ tập theo dõi trong một cấu trúc bền, và khi một fd sẵn sàng thì **callback của driver đẩy nó vào danh sách ready**. `epoll_wait` chỉ việc lấy danh sách ready ra.

| | `select` | `poll` | `epoll` |
|---|---|---|---|
| Giới hạn số fd | ~**1024** (`FD_SETSIZE`) | không | không |
| Chi phí mỗi lần gọi | **O(n)** | **O(n)** | **O(k)** — k = số fd **sẵn sàng** |
| Truyền tập fd mỗi lần | ✅ phải truyền lại | ✅ | ❌ đăng ký một lần |
| Tập fd bị phá sau khi gọi | ✅ phải dựng lại | ❌ | ❌ |
| Di động | POSIX | POSIX | **chỉ Linux** (BSD: `kqueue`) |

**Vì sao chênh lệch lớn đến vậy:** với 10.000 kết nối mà chỉ 10 cái có dữ liệu, `poll` vẫn duyệt đủ 10.000 mỗi vòng; `epoll` chạm đúng 10. Đây chính là bài toán **C10K**, và là lý do Nginx/Redis/libuv đều dựng trên epoll.

**Bẫy:** (1) nói "epoll nhanh hơn" mà không nêu điều kiện — với **ít fd** (vài chục) `poll` có thể **nhanh hơn** vì không tốn syscall `epoll_ctl` và cấu trúc kernel; ưu thế chỉ xuất hiện khi **n lớn và phần lớn idle**; (2) quên `epoll` **không di động** — code cần chạy trên BSD/macOS phải trừu tượng hoá (libevent/libuv); (3) sau `fork`, epoll instance được kế thừa và **chia sẻ** — hai process cùng `epoll_wait` một fd sinh thundering herd (dùng `EPOLLEXCLUSIVE`).

**Chốt:** *"select/poll hỏi lại toàn bộ danh sách mỗi lần; epoll đăng ký một lần rồi chỉ nhận về cái đã sẵn sàng. Chi phí đi từ O(tổng số fd) xuống O(số fd có việc)."*
</details>

#### LNX-010 · 🟠 · concept · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Level-triggered và edge-triggered khác nhau? ET cần lưu ý gì?**
<details><summary>Đáp án</summary>

**Ẩn dụ điện tử đúng nguyên nghĩa:** LT báo theo **mức** (còn dữ liệu là còn báo), ET báo theo **sườn** (chỉ báo lúc **chuyển trạng thái** từ "không có" sang "có").

| | **Level-triggered** (mặc định) | **Edge-triggered** (`EPOLLET`) |
|---|---|---|
| Khi nào báo | **Mỗi lần** `epoll_wait` nếu buffer còn dữ liệu | **Một lần duy nhất** khi dữ liệu mới đến |
| Đọc chưa hết thì | Lần sau vẫn được báo lại → an toàn | **Không báo lại** → dữ liệu kẹt, kết nối "treo" |
| fd non-blocking | Nên có | **Bắt buộc** |
| Độ khó | Dễ đúng | Dễ sai |

```c
// ✅ Mẫu ET BẮT BUỘC: vét cạn tới khi EAGAIN
for (;;) {
    ssize_t n = read(fd, buf, sizeof buf);
    if (n > 0)  { handle(buf, n); continue; }        // còn -> đọc tiếp
    if (n == 0) { close(fd); break; }                 // peer đóng
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;   // ✅ đã cạn, thoát đúng chỗ
    if (errno == EINTR) continue;
    perror("read"); break;
}
```

**Vì sao ET *bắt buộc* non-blocking:** vòng lặp trên phải chạy tới khi hết dữ liệu. Nếu fd là **blocking**, lần `read` cuối (khi đã cạn) sẽ **treo cả thread** — event loop đứng hình, mọi kết nối khác chết theo. Non-blocking mới trả `EAGAIN` để bạn thoát.

**Khi nào chọn ET:** giảm số lần `epoll_wait` quay lại cho cùng một fd → có ý nghĩa ở tải rất cao, hoặc khi dùng chung một epoll cho nhiều thread (LT gây **thundering herd**: mọi thread cùng được báo). Còn lại: **mặc định LT**, đúng trước rồi mới tối ưu.

**Bẫy:** (1) quên vét cạn → bug "kết nối im lặng sau vài request", cực khó lần vì phụ thuộc kích thước dữ liệu; (2) chỉ vét cạn phía `read` mà quên phía `write` (`EPOLLOUT` cũng edge); (3) một fd "cạn" không có nghĩa các fd khác cũng vậy — vẫn phải duyệt hết mảng `epoll_wait` trả về.

**Chốt:** *"LT = 'còn hàng là còn nhắc'. ET = 'chỉ báo lúc hàng vừa tới, không nhắc lại' → bắt buộc non-blocking + đọc tới `EAGAIN`."*
</details>

#### LNX-011 · 🟠 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Vì sao trong signal handler chỉ được gọi hàm async-signal-safe?**
<details><summary>Đáp án</summary>

**Cơ chế:** signal handler **chen ngang** luồng chính tại một điểm **bất kỳ** — kể cả **giữa chừng** một hàm thư viện. Nó không phải một thread khác; nó chạy **trên cùng thread**, mượn stack của nó.

Kịch bản chết người:

```
main:    malloc(…)  ──► đã lấy khoá heap, đang sửa dở free-list
           ↓ SIGALRM chen vào
handler: printf(…)  ──► printf gọi malloc  ──► xin CÙNG khoá heap
                        khoá đang do main giữ, mà main KHÔNG chạy được (đang bị chen)
                        → DEADLOCK, hoặc free-list dở dang → HEAP CORRUPTION
```

Không phải chuyện lý thuyết: `printf`, `malloc`, `free`, `syslog` đều **không reentrant** vì giữ trạng thái toàn cục + khoá. Do đó POSIX chỉ đảm bảo một danh sách hàm **async-signal-safe**: `write`, `_exit`, `signal`, `sigaction`, `kill`, `read`… (không có `printf`).

**Mẫu an toàn — handler làm ít nhất có thể:**

```c
volatile sig_atomic_t g_stop = 0;      // ✅ sig_atomic_t: ghi nguyên tử, volatile: không bị tối ưu

void handler(int sig) { g_stop = 1; }  // ✅ chỉ đặt cờ, không làm gì khác

int main(void) {
    while (!g_stop) { do_work(); }     // ✅ xử lý THẬT ở main loop
}
```

**Hai lựa chọn tốt hơn nữa** (đưa signal về mô hình event loop, hợp kiến trúc server/daemon):
- **`signalfd`** — nhận signal qua **fd**, đọc bằng `read()`, cắm thẳng vào `epoll`. Không cần handler.
- **self-pipe trick** — handler chỉ `write(pipefd, "x", 1)` (`write` là async-signal-safe), main loop đọc từ pipe. Cách di động khi không có `signalfd`.

**Bẫy:** (1) dùng `int` thay `sig_atomic_t` cho cờ — không đảm bảo ghi nguyên tử; thiếu `volatile` → compiler cache biến vào thanh ghi, vòng lặp không bao giờ thấy cờ đổi; (2) `printf` gỡ lỗi trong handler — "chạy được" phần lớn thời gian, rồi treo ngẫu nhiên; dùng `write(2, msg, len)`; (3) handler **phải bảo toàn `errno`** (lưu đầu, khôi phục cuối) — nếu không nó ghi đè `errno` của main; (4) `SIGKILL`/`SIGSTOP` **không** bắt/chặn được.

**Chốt:** *"Handler chen vào giữa chương trình trên cùng thread — gọi hàm không reentrant là tự khoá chính mình. Chỉ set cờ, hoặc chuyển signal thành fd (`signalfd`/self-pipe) rồi xử lý ở main loop."*
</details>

#### LNX-012 · 🟠 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Điều gì xảy ra với fd qua fork và exec?**
<details><summary>Đáp án</summary>

**Phải tách hai tầng thì mới trả lời đúng:**
- **File descriptor** = số nguyên trong **bảng fd riêng của mỗi process**.
- **Open file description** = cấu trúc trong **kernel**, giữ **offset**, cờ trạng thái, và trỏ tới inode. Nhiều fd (nhiều process) có thể trỏ về **cùng một** description.

| | `fork` | `exec` |
|---|---|---|
| Bảng fd | Con **copy** bảng fd của cha | **Giữ nguyên** — không phải chương trình mới thì reset |
| Trỏ tới đâu | fd cha & con trỏ về **cùng open file description** | như trước |
| **Offset** | **Dùng chung** — con `read` thì offset của cha cũng dịch | như trước |
| Ngoại lệ | — | fd có cờ **close-on-exec** (`O_CLOEXEC`/`FD_CLOEXEC`) **tự đóng** |

**Hệ quả thực tế của "chung offset":** cha và con cùng ghi vào một file đã mở trước `fork` sẽ **nối tiếp** nhau, không đè lên nhau — đó là lý do shell redirect (`cmd > file`) hoạt động đúng khi có nhiều process. Ngược lại, nếu mỗi bên `open()` riêng thì có **hai** description, **hai** offset độc lập → ghi đè nhau.

```c
int fd = open("f.txt", O_RDWR | O_CLOEXEC);   // ✅ mặc định nên có O_CLOEXEC
// fork() -> con vẫn có fd, chung offset
// exec() -> fd TỰ ĐÓNG nhờ O_CLOEXEC
```

**Vì sao `O_CLOEXEC` là mặc định nên dùng:** không có nó, **mọi** fd đang mở (socket, file cấu hình chứa secret, fd của thư viện khác) đều **rò sang chương trình con** — vừa là lỗ hổng bảo mật, vừa giữ tài nguyên không giải phóng (port vẫn bị chiếm dù server chính đã đóng). Trong chương trình **đa luồng**, còn phải dùng `O_CLOEXEC` **ngay trong `open()`** thay vì `fcntl` sau đó, vì giữa hai lời gọi có thể có thread khác `fork`.

**Bẫy:** (1) tưởng `fork` copy cả offset thành hai bản độc lập — **sai**, chung offset; (2) tưởng `exec` đóng hết fd — **sai**, giữ nguyên trừ CLOEXEC (đây chính là cách stdin/stdout/stderr sống sót qua exec); (3) sau `fork`, quên đóng **đầu không dùng của pipe** ở cả hai phía → `read` không bao giờ thấy EOF, treo vĩnh viễn.

**Chốt:** *"`fork` copy bảng fd nhưng dùng chung offset; `exec` giữ nguyên fd trừ khi có `O_CLOEXEC`. Cứ mở fd là đặt `O_CLOEXEC`, trừ khi cố ý muốn truyền sang con."*
</details>

#### LNX-013 · 🔴 · design · ⭐ · [→ io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Mô tả kiến trúc event loop. Nguyên tắc cốt lõi là gì?**
<details><summary>Đáp án</summary>

Vòng lặp gọi `epoll_wait` ngủ tới khi có fd sẵn sàng, rồi với mỗi fd: accept kết nối mới (đăng ký vào epoll) hoặc đọc/ghi non-blocking và xử lý, quay lại chờ. Một thread phục vụ rất nhiều kết nối → ít RAM, ít context switch (Nginx/Redis/Node). Nguyên tắc: **không bao giờ block** trong event loop; mọi I/O non-blocking, tác vụ CPU nặng đẩy sang thread riêng. Mở rộng: nhiều loop trên nhiều core + thread pool.
</details>

#### LNX-014 · 🔴 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Bạn dùng những cơ chế Linux nào để tích hợp signal/timer vào event loop?**
<details><summary>Đáp án</summary>

Các fd-based primitives: `signalfd` (nhận signal qua fd, tránh handler async-signal-safe), `timerfd` (timer phát sự kiện qua fd cho timeout), `eventfd` (đếm/báo hiệu nhẹ giữa thread/process), `memfd` (vùng nhớ ẩn danh có fd). Nhờ biến mọi nguồn sự kiện thành fd, event loop xử lý đồng nhất socket + timer + signal + notify chỉ qua epoll.
</details>

#### LNX-015 · 🔴 · design · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Thiết kế giao tiếp hiệu năng cao giữa hai process trên cùng máy?**
<details><summary>Đáp án</summary>

Dùng shared memory cho dữ liệu lớn/tần suất cao (zero-copy): `shm_open` + `mmap`, tổ chức một ring buffer trong vùng shm cho producer–consumer, đồng bộ bằng POSIX semaphore process-shared hoặc mutex `PTHREAD_PROCESS_SHARED` đặt trong header của vùng shm. Nếu cần truyền message có ranh giới/điều khiển, kết hợp Unix domain socket (truyền được fd qua SCM_RIGHTS) hoặc eventfd để báo hiệu. Cân nhắc cache line alignment để tránh false sharing.
</details>

#### LNX-016 · 🟡 · concept · ⭐ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Khi nào dùng POSIX message queue thay vì shared memory?**
<details><summary>Đáp án</summary>

Message queue cho **ranh giới message rõ ràng**, có priority, và decouple producer–consumer — hợp khi dữ liệu là chuỗi giá trị/sự kiện theo thời gian và throughput không phải nút thắt. Shared memory nhanh hơn nhưng phải tự lo mutex/semaphore và dễ race. Chọn message queue khi an toàn/dễ bảo trì quan trọng hơn tốc độ tối đa.
</details>

#### LNX-017 · 🟡 · design · ⭐ · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Message queue đầy thì xử lý sao?**
<details><summary>Đáp án</summary>

Mặc định `mq_send` block; `O_NONBLOCK` trả `EAGAIN`. Nhưng nếu chỉ **giá trị đích mới nhất** quan trọng (vd ramp độ sáng), giải pháp đúng là **coalescing/latest-value-wins** (giữ ô giá trị đích, consumer nội suy) hoặc **drop-oldest** (ghi đè giá trị cũ vì giá trị mới phản ánh trạng thái chính xác hơn), kèm **rate limiting** ở producer. Tránh để block làm trễ phản ứng.
</details>

#### LNX-018 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**So sánh các cơ chế IPC.**
<details><summary>Đáp án</summary>

Pipe/FIFO: byte stream đơn giản, 1 chiều. Message queue: có ranh giới message + priority. Shared memory: nhanh nhất, tự đồng bộ. Socket: liên máy/mạng. Signal: báo sự kiện, ít dữ liệu. Chọn theo: tốc độ vs ranh giới message vs phạm vi (cùng máy/khác máy).
</details>

#### LNX-019 · 🟢 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**`mmap` là gì? Kể vài công dụng.**
<details><summary>Đáp án</summary>

`mmap` ánh xạ một vùng vào không gian địa chỉ process, trả con trỏ để truy cập như bộ nhớ thường (kernel lo demand-paging). Công dụng: (1) **map file** — đọc/ghi file qua con trỏ, không cần read/write, hệ tự đồng bộ qua page cache (nhanh cho truy cập ngẫu nhiên file lớn); (2) **anonymous mapping** — cấp vùng nhớ lớn (nền của malloc cho khối lớn); (3) **shared memory** giữa process (`MAP_SHARED`); (4) map **MMIO/`/dev/mem`** để chọc thanh ghi từ userspace (bring-up). Đồng bộ file: `msync`.
</details>

#### LNX-020 · 🟡 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Pipe và FIFO (named pipe) khác nhau thế nào?**
<details><summary>Đáp án</summary>

Cả hai là byte stream một chiều. **Pipe** (`pipe()`) ẩn danh — chỉ dùng được giữa các process **có quan hệ** (chia sẻ fd qua fork), sống theo fd, đó là cách shell nối `a | b`. **FIFO** (`mkfifo`) có **tên trên filesystem** → hai process **không quan hệ** cũng mở được qua đường dẫn. Chung: ghi vào pipe đầy thì block (hoặc EAGAIN nếu nonblock); ghi khi không còn reader → `SIGPIPE`/`EPIPE`; ghi ≤ `PIPE_BUF` là atomic.
</details>

#### LNX-021 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**`dup`/`dup2` để làm gì? Shell hiện thực `cmd > file` thế nào?**
<details><summary>Đáp án</summary>

`dup2(oldfd, newfd)` làm `newfd` trỏ tới **cùng open file description** với `oldfd` (đóng newfd nếu đang mở). Shell làm `cmd > file`: sau `fork`, trong con `open("file")` được fd f → `dup2(f, STDOUT_FILENO)` (fd 1 giờ trỏ file) → `close(f)` → `exec(cmd)`. Chương trình cứ ghi stdout như thường nhưng dữ liệu vào file. Cùng cơ chế cho `2>&1` (dup2 fd2 về fd1) và pipe (`dup2` đầu pipe vào stdin/stdout).

**Vế hay bị bỏ sót — hệ quả của "cùng file description":**

| | Chia sẻ gì | Offset |
|---|---|---|
| `dup`/`dup2` | cùng **file description** | **CHUNG** — ghi qua fd này, offset fd kia cũng nhảy |
| `fork()` | cùng **file description** | **CHUNG** giữa cha và con |
| `open()` cùng file **hai lần** | hai file description **khác nhau** | **RIÊNG**, độc lập |

Đó là lý do `cmd1 >> log` và `cmd2 >> log` chạy song song không đè nhau (cùng `O_APPEND`), còn hai lần `open` không `O_APPEND` thì có. ⚠️ `FD_CLOEXEC` nằm ở **tầng fd**, không phải file description → **`dup()` không sao chép nó**.
</details>

#### LNX-022 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Các trạng thái process trong Linux? `D` state nghĩa là gì và vì sao đáng ngại?**
<details><summary>Đáp án</summary>

`R` running/runnable, `S` sleep ngắt được (chờ sự kiện, nhận signal), `D` **uninterruptible sleep** (chờ I/O, *không* nhận signal), `T` stopped, `Z` zombie. **`D` state**: process kẹt trong syscall I/O ở kernel không thể bị đánh thức/kill (kể cả `kill -9`) tới khi I/O xong — nếu I/O treo (NFS chết, disk hỏng) process **kẹt vĩnh viễn**, load average tăng vọt dù CPU rảnh. Thấy nhiều process `D` = nghi tầng storage/driver. Xem cột STAT trong `ps`, `/proc/<pid>/stack` để biết kẹt ở đâu.
</details>

#### LNX-023 · 🟡 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**`/proc` và `/sys` là gì, khác nhau thế nào?**
<details><summary>Đáp án</summary>

Cả hai là **virtual filesystem** (nội dung sinh bởi kernel lúc đọc, không nằm trên disk) — "everything is a file" áp cho trạng thái kernel. `/proc`: thông tin **process** (`/proc/<pid>/…`: maps, fd, status, cmdline) + nhiều thông tin hệ thống lịch sử (`/proc/meminfo`, `/proc/interrupts`, `/proc/cpuinfo`). `/sys` (sysfs): mô hình **thiết bị/driver** có cấu trúc (device model — bus/device/driver/class), một giá trị/file, dùng để đọc & cấu hình driver (gpio, pwm, cpufreq). Đại khái: `/proc` cũ + tạp, `/sys` mới + có cấu trúc cho device.
</details>

#### LNX-024 · 🟠 · concept · [→ ipc-linux](../../../04-linux-system-programming/ipc-linux.md)
**Namespaces và cgroups là gì? Vì sao là nền tảng của container?**
<details><summary>Đáp án</summary>

**Namespaces** = *cô lập tầm nhìn*: mỗi loại (pid, mount, net, uts, ipc, user) cho process một "vũ trụ" riêng — pid namespace khiến process thấy mình là PID 1 và không thấy process ngoài; net namespace cho stack mạng riêng. **cgroups** = *giới hạn & hạch toán tài nguyên*: đặt trần CPU/RAM/IO cho một nhóm process (memory limit + OOM cục bộ, cpu quota). Container = **namespaces (cô lập) + cgroups (giới hạn) + rootfs riêng** — không phải máy ảo, vẫn chung kernel host. Docker/LXC/systemd-nspawn đều dựng trên hai cơ chế này.
</details>

#### LNX-025 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Cách tạo một daemon đúng? systemd làm nhẹ việc này ra sao?**
<details><summary>Đáp án</summary>

Daemon "cổ điển" (double-fork): `fork` + parent thoát (con thành orphan, không phải leader), `setsid` (tách khỏi terminal, thành session leader), fork lần 2 (chắc chắn không giành lại tty), `chdir("/")`, đặt `umask`, đóng/redirect stdin/out/err về `/dev/null`, xử lý `SIGTERM` để tắt sạch. **Với systemd**: hầu hết việc trên là thừa — viết service kiểu `Type=simple` **chạy foreground**, systemd lo tách session, log (journald), restart (`Restart=on-failure`), phụ thuộc thứ tự (`After=`), sandbox, watchdog (`WatchdogSec`). Chỉ cần chương trình chạy tiền cảnh + xử lý SIGTERM. Đây là chuẩn hiện đại trên embedded Linux.
</details>

#### LNX-026 · 🟠 · concept · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Đọc/ghi file qua `mmap` lợi/hại gì so với `read`/`write`? Vai trò `msync`.**
<details><summary>Đáp án</summary>

`mmap` truy cập file như mảng bộ nhớ → **không copy user↔kernel mỗi lần** (read/write copy qua buffer), tốt cho **truy cập ngẫu nhiên** file lớn và chia sẻ giữa process; code gọn (con trỏ thay vì lseek+read). Hại: chi phí thiết lập mapping + page fault mỗi trang lần đầu chạm (kém cho quét tuần tự một lần — read tuần tự + readahead thắng); lỗi I/O biến thành **SIGBUS** khó xử; không hợp file nhỏ/streaming; ghi phải `msync` (đẩy trang bẩn xuống disk) để đảm bảo độ bền, và cẩn thận khi file bị truncate dưới chân mapping. Chọn theo mẫu truy cập: ngẫu nhiên/chia sẻ → mmap; tuần tự/streaming/nhỏ → read/write.
</details>

#### LNX-027 · 🟡 · concept · ⭐ · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**`EINTR` là gì? Đặt `SA_RESTART` có giải quyết hết không?**
<details><summary>Đáp án</summary>

`EINTR` = syscall đang **chặn** thì bị signal handler cắt ngang → trả `-1` với `errno == EINTR`. **Không phải lỗi thật**, chỉ là "bị ngắt, chưa làm xong". Code coi mọi `-1` là lỗi rồi thoát sẽ **chết ngẫu nhiên** khi signal tới đúng lúc.

**`SA_RESTART` KHÔNG giải quyết hết** — đây là ý phân loại. Nhóm **không bao giờ** restart dù đặt cờ:
- **`select`, `poll`, `epoll_wait`** (và `pselect`/`ppoll`/`epoll_pwait`)
- **`sleep`, `nanosleep`**, **`pause`, `sigsuspend`**
- `semop`, `msgrcv`, `msgsnd`; `read()` từ fd inotify

⇒ **Vòng lặp `EINTR` quanh `epoll_wait`/`select`/`poll` là bắt buộc**, không cờ nào thay được:
```c
int n;
do { n = epoll_wait(epfd, ev, MAX, -1); } while (n == -1 && errno == EINTR);
```

**Hai chi tiết ăn điểm:**
1. `read`/`write` chỉ restart trên **"thiết bị chậm"** (terminal, pipe, FIFO, socket). **File trên đĩa không thuộc nhóm này** → bug này **không lộ khi test với file**, chỉ lộ với socket/pipe thật.
2. Nếu `read`/`write` **đã chuyển được một phần** rồi mới bị cắt thì **không** restart mà **trả về thành công với số byte đã chuyển** — thêm một nguồn của short read/write (LNX-005).

Cần chờ fd **và** signal cùng lúc không race → `epoll_pwait`/`ppoll` (đặt sigmask nguyên tử), hoặc kéo signal vào event loop bằng `signalfd` (LNX-014).
</details>

#### LNX-028 · 🟡 · concept · ⭐ · [→ file-io](../../../04-linux-system-programming/file-io.md)
**Vì sao cần `O_APPEND`? `lseek()` rồi `write()` sai ở đâu?**
<details><summary>Đáp án</summary>

Vì đó là **hai syscall**, và giữa chúng process khác chen vào được:
```
   A: lseek(fd,0,SEEK_END) → 1000
                              B: lseek → 1000 ; B: write("B") → file dài 1001
   A: write("A")          → ghi tại 1000, ĐÈ MẤT "B"
```
`O_APPEND` khiến kernel làm **"nhảy tới cuối rồi ghi" như MỘT thao tác nguyên tử**. Đó là lý do `>>` của shell và nhiều process cùng ghi một log file hoạt động đúng.

**Cùng mẫu tư duy ở hai chỗ nữa:**
- **`O_CREAT | O_EXCL`** — "tạo chỉ khi chưa tồn tại" nguyên tử, thay cho `open` rồi kiểm tra `ENOENT` (nền của lock file).
- **`pread`/`pwrite`** — đọc/ghi tại offset chỉ định **không đụng offset chung**; bắt buộc khi **nhiều thread dùng chung một fd**, vì thread chia sẻ offset nên `lseek`+`read` từ hai thread giẫm lên nhau.

**Chốt:** *hễ hai syscall không được để ai chen vào giữa, hãy tìm cờ hoặc syscall gộp cả hai.* Mẫu này còn sinh ra `O_CLOEXEC`, `accept4()`, `epoll_create1()`, `sigsuspend()`, `rename()`.
</details>

#### LNX-029 · 🟡 · concept · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Đo khoảng thời gian / đặt timeout nên dùng clock nào? Vì sao?**
<details><summary>Đáp án</summary>

**`CLOCK_MONOTONIC`**, không phải `CLOCK_REALTIME`.

- `CLOCK_REALTIME` = giờ thực theo lịch — **nhảy được**: NTP chỉnh, admin đổi giờ, đổi múi giờ/DST.
- `CLOCK_MONOTONIC` = đếm đơn điệu từ một mốc bất kỳ (thường là lúc boot) — **không bao giờ nhảy lùi**.

**Hỏng thế nào nếu chọn sai:** NTP chỉnh lùi 1 giây giữa hai lần đo → khoảng thời gian ra **số âm**, hoặc timeout chờ thêm rất lâu. Trên thiết bị nhúng **không có RTC pin**, đồng hồ **nhảy vọt** khi NTP đồng bộ lần đầu sau boot — đúng lúc các service đang khởi động với timeout.

**Quy tắc:** cần *"lúc mấy giờ"* (timestamp log, mtime) → `REALTIME`; cần *"bao lâu"* (timeout, đo hiệu năng, lập lịch định kỳ) → `MONOTONIC`.

Trong C++ đúng cùng nguyên tắc: **`std::chrono::steady_clock`** cho đo khoảng, `system_clock` cho thời điểm theo lịch — `steady_clock` tồn tại chính vì lý do này.
</details>

#### LNX-030 · 🟡 · concept · ⭐ · 🎤 2026-08-13 · [→ processes-signals](../../../04-linux-system-programming/processes-signals.md)
**Daemon dưới systemd không tắt được bằng `systemctl stop` — phải `SIGKILL`. Handler đã viết đúng chuẩn "chỉ set cờ". Vấn đề ở đâu?**

```c
volatile sig_atomic_t stop = 0;
void on_sigterm(int sig) { stop = 1; }            // ✅ chuẩn sách vở

int main(void) {
    struct sigaction sa = { .sa_handler = on_sigterm };
    sa.sa_flags = SA_RESTART;                     // "cho chắc, tránh EINTR"
    sigaction(SIGTERM, &sa, NULL);

    for (;;) {
        char* ev = read_next_event();             // read() CHẶN trên socket
        process(ev);
        if (stop) break;
    }
}
```

<details><summary>Đáp án</summary>

**Handler đúng, nhưng `SA_RESTART` vô hiệu hoá chính nó.**

`SA_RESTART` khiến `read()` **tự chạy lại** sau khi handler chạy xong. Thiết bị lúc rảnh không có event ⇒ `read()` chặn vĩnh viễn ⇒ vòng lặp **không bao giờ tới dòng `if (stop)`**. Cờ được set nhưng không ai đọc.

**Đây là lỗi tư duy ngược:** `EINTR` bị coi là "phiền toái phải dập". Thật ra nó là **cơ chế duy nhất để signal đánh thức bạn ra khỏi chỗ đang chặn** — đúng thứ shutdown cần.

| | `sa_flags = 0` | `sa_flags = SA_RESTART` |
|---|---|---|
| `read()` khi có signal | trả `-1`, `errno = EINTR` | tự gọi lại, **không trả về** |
| Vòng lặp kiểm tra được cờ | ✅ | ❌ **không bao giờ** |
| `systemctl stop` | thoát sạch | treo tới `TimeoutStopSec` (**mặc định 90 s**) rồi **SIGKILL** |

**Hệ quả thực tế** (chứ không chỉ "chậm"): mỗi lần restart treo 90 giây; OTA update rơi vào timeout; bị SIGKILL nên **không kịp flush trạng thái** → mất dữ liệu.

**Sửa — xử lý `EINTR` theo *ý định*, không dập bằng cờ:**
```c
sa.sa_flags = 0;                       // ✅ để EINTR đánh thức
...
ssize_t n = read(fd, buf, len);
if (n < 0 && errno == EINTR) {
    if (stop) break;                   // ✅ signal shutdown → thoát
    continue;                          // ✅ signal khác     → thử lại
}
```

**Kiến trúc tốt hơn:** kéo signal vào event loop bằng **`signalfd`** ([LNX-014](linux-sysprog.md)) hoặc **self-pipe trick** — khi đó `epoll_wait` thấy signal như một fd bình thường, không còn cờ `volatile` lẫn `EINTR` để xử lý.

⚠️ **Phân biệt với [LNX-027](linux-sysprog.md):** LNX-027 nói *"`SA_RESTART` không cứu được mọi syscall"* (thiếu). Câu này là vế ngược, ít người biết hơn: *"`SA_RESTART` còn có hại — nó phá shutdown"*.

**Bẫy liên quan cùng đoạn code:** `printf(...)` rồi `_exit(0)` lúc thoát ⇒ `_exit` **không flush stdio**; dưới systemd `stdout` là **pipe tới journald** nên full-buffered ⇒ **mất đúng dòng log shutdown**. Dùng `fflush` + `_exit`, hoặc `exit()`, hoặc `write(2, ...)`.

**Chốt:** *"`EINTR` không phải lỗi cần dập — nó là cách signal gọi bạn dậy. Dập nó bằng `SA_RESTART` là tự làm daemon không tắt được."*

> 🔬 Kiểm chứng chạy thật + bản sửa đầy đủ: [coding-arena/reviewed/2026-08-13--OS-020--sigterm-handler.cpp](../coding-arena/reviewed/2026-08-13--OS-020--sigterm-handler.cpp)
</details>

---
⬅️ [Bank index](README.md)
