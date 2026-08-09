# TLPI — Cụm 05: Alternative I/O Models (ch. 63) 🎯

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 63, tr. 1325–1374.
> **Vì sao cụm này ưu tiên số 1:** JD Datalogic ghi thẳng *"Familiar with Linux userspace"*, và câu hỏi thiết kế server/daemon gần như luôn rơi vào đây. Chương này cũng là chỗ **level-triggered vs edge-triggered** được giải thích rõ nhất trong toàn bộ tài liệu Linux.

---

## 1. Vấn đề mà cả chương này giải quyết

Mô hình I/O thông thường (`read()` chặn cho tới khi có dữ liệu) **sập đổ khi phải theo dõi NHIỀU file descriptor cùng lúc**. Kerrisk mở đầu bằng đúng tình huống đó (§63.1, tr. 1326): một chương trình cần đọc từ nhiều pipe/socket, không biết cái nào có dữ liệu trước.

Ba cách "ngây thơ" và vì sao đều hỏng:

```
   ❌ Cách 1 — đọc lần lượt, chặn ở từng cái
      read(fd1)  ← KẸT ở đây mãi mãi nếu fd1 không có dữ liệu,
      read(fd2)     trong khi fd2 đang đầy dữ liệu chờ xử lý

   ❌ Cách 2 — mỗi fd một process/thread
      Chạy đúng, nhưng 10 000 kết nối = 10 000 thread
      → tốn RAM (mỗi thread một stack), context switch ngập đầu

   ❌ Cách 3 — non-blocking + vòng lặp bận (busy polling)
      while (1) { thử đọc hết mọi fd; }
      → ĐỐT 100% CPU dù chẳng có gì xảy ra
```

Kerrisk viết thẳng về cách 3: *"polling in a tight loop wastes CPU time"* (§63.1, tr. 1326) — và đó là toàn bộ lý do tồn tại của I/O multiplexing.

**Bốn giải pháp chương này trình bày**, xếp theo thứ tự lịch sử:

| Mô hình | Có từ | Ý tưởng | Level/Edge |
|---|---|---|---|
| **I/O multiplexing** — `select()`, `poll()` | POSIX cổ | Đưa **cả danh sách fd** cho kernel mỗi lần gọi, kernel trả về cái nào sẵn sàng | **Level** |
| **Signal-driven I/O** | POSIX | Đăng ký một lần; kernel **gửi signal** khi fd sẵn sàng | **Edge** |
| **`epoll`** | Linux 2.6 | Kernel **giữ sẵn danh sách quan tâm**; mỗi lần gọi chỉ lấy phần sẵn sàng | **Cả hai** |
| **Multithreading / async I/O (`aio`)** | — | Nhắc qua để so sánh (§63.1) | — |

*(bảng theo Table 63-1, tr. 1329)*

> **Câu chốt cho cả chương:** ba mô hình đầu **không thực hiện I/O** — chúng chỉ trả lời câu hỏi *"gọi `read()` bây giờ có bị chặn không?"*. Việc đọc/ghi vẫn là của bạn. Hiểu điều này gỡ được rất nhiều nhầm lẫn.

---

## 2. Level-triggered vs Edge-triggered — phần quan trọng nhất chương 🎯

Đây là khái niệm Kerrisk đặt **ngay đầu chương** (§63.1.1, tr. 1329) *trước khi* nói về API nào, vì nó quyết định **cách bạn viết vòng lặp**, không chỉ cờ nào truyền vào.

### 2.1. Định nghĩa, phát biểu chính xác

- **Level-triggered:** thông báo được đưa ra **nếu fd đang ở trạng thái sẵn sàng** — tức *"gọi `read()` lúc này sẽ không bị chặn"*. Trạng thái còn thì còn báo.
- **Edge-triggered:** thông báo chỉ đưa ra **khi có SỰ KIỆN I/O mới xảy ra** kể từ lần kiểm tra trước. Trạng thái sẵn sàng kéo dài **không** sinh thêm thông báo.

Ví dụ Kerrisk dùng để tách hai khái niệm (§63.4.6, tr. 1366) — thuộc ví dụ này là đủ trả lời câu hỏi:

```
   ① Dữ liệu tới trên socket (giả sử 100 byte)
   ② epoll_wait() lần 1  →  BÁO SẴN SÀNG   (cả level lẫn edge đều báo)
   ③ Ta đọc 40 byte, còn 60 byte trong buffer kernel
   ④ epoll_wait() lần 2:

        LEVEL-TRIGGERED  →  BÁO TIẾP    (vì vẫn CÒN 60 byte = vẫn "sẵn sàng")
        EDGE-TRIGGERED   →  BỊ CHẶN ❌  (vì KHÔNG có dữ liệu MỚI tới kể từ lần trước)
                             → 60 byte kia nằm đó VĨNH VIỄN nếu không có gói mới
```

Sơ đồ hoá cùng một chuỗi sự kiện:

```
   dữ liệu trong buffer kernel
        │
   100B │      ┌──────────────┐
        │      │              │
    60B │      │              └──────────────────────────  (còn 60B, không ai đọc)
        │      │
     0B ├──────┘
        └──────┬──────────────┬──────────────┬────────────► thời gian
               ▲              ▲              ▲
            dữ liệu tới   epoll_wait #1   epoll_wait #2

   LEVEL:  ────────────────[BÁO]──────────[BÁO]────────   báo khi CÒN dữ liệu
   EDGE:   ───────────[▲]──[BÁO]────────────────────────   báo tại CẠNH LÊN, một lần
                       cạnh
```

### 2.2. Hệ quả: hai cách viết vòng lặp khác hẳn nhau

| | Level-triggered | Edge-triggered |
|---|---|---|
| Mỗi lần báo phải đọc bao nhiêu? | **Bao nhiêu cũng được** — kể cả không đọc gì. Lần sau vẫn được báo lại | **PHẢI đọc cho tới cạn** (`EAGAIN`), nếu không sẽ **mất dữ liệu vĩnh viễn** |
| Cần non-blocking fd? | Không bắt buộc | **Gần như bắt buộc** — vì phải đọc trong vòng lặp tới khi `EAGAIN` |
| Số lần gọi `epoll_wait` | Nhiều hơn | Ít hơn (mỗi sự kiện một lần) |
| Rủi ro viết sai | Thấp | **Cao** — quên đọc cạn là bug "treo ngẫu nhiên", rất khó tái hiện |

Kerrisk phát biểu quy tắc cho ET rõ ràng (§63.1.1, tr. 1329): *"After notification of an I/O event, the program should—at some point—perform as much I/O as possible... If the program fails to do this, then it might miss the opportunity to perform some I/O."*

**Khung chuẩn cho edge-triggered** (§63.4.6, tr. 1366):

```c
// ① Đặt MỌI fd theo dõi thành non-blocking — bắt buộc, xem lý do ở ③b
int flags = fcntl(fd, F_GETFL);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);

// ② Đăng ký với cờ EPOLLET
struct epoll_event ev;
ev.data.fd = fd;
ev.events  = EPOLLIN | EPOLLET;        // ← ET
if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
    errExit("epoll_ctl");

// ③ Vòng lặp xử lý
for (;;) {
    int nready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
    for (int j = 0; j < nready; j++) {
        // ③b ĐỌC CHO TỚI CẠN — điều kiện dừng là EAGAIN, KHÔNG phải "đọc một lần"
        for (;;) {
            ssize_t n = read(evlist[j].data.fd, buf, sizeof(buf));
            if (n == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;             // ✅ đã cạn — giờ mới được rời fd này
                if (errno == EINTR) continue;
                errExit("read");
            }
            if (n == 0) { /* peer đóng */ break; }
            process(buf, n);
        }
    }
}
```

> ⚠️ **Nếu fd còn blocking mà dùng ET:** vòng `for(;;) read()` sẽ **chặn vĩnh viễn** ở lần đọc sau khi đã cạn — cả server đứng hình. Đây là lý do "non-blocking" không phải tuỳ chọn mà là **điều kiện đúng đắn** của ET.

### 2.3. Bẫy thứ hai của ET: đói fd (starvation)

Kerrisk nêu một vấn đề mà nhiều tài liệu bỏ qua (§63.4.6, tr. 1366): nếu một fd có **luồng dữ liệu bất tận**, việc trung thành làm theo quy tắc "đọc cho tới cạn" sẽ khiến bạn **không bao giờ quay lại các fd khác**.

```
   fd=5 (kẻ nói nhiều)   ████████████████████████████████████████ đọc mãi không hết
   fd=6                  ▒ chờ...
   fd=7                  ▒ chờ...   ← ĐÓI: không được phục vụ
```

**Giải pháp của sách:** ứng dụng tự giữ **một danh sách fd đã được báo sẵn sàng**, rồi chạy vòng lặp:
1. Gọi `epoll_wait()` để thêm fd mới sẵn sàng vào danh sách — nếu danh sách **đã có** fd, đặt timeout **nhỏ hoặc 0** để không kẹt ở bước này.
2. Duyệt danh sách, **mỗi fd chỉ làm một lượng I/O giới hạn** (round-robin), thay vì vắt kiệt một fd.

🆕 Đây chính là mẫu mà nginx/libevent gọi là *fairness*, và là lý do server thật không bao giờ chỉ là vòng `while(read() != EAGAIN)` ngây thơ.

---

## 3. `select()` và `poll()` — và vì sao chúng chậm (§63.2, tr. 1331–1346)

### 3.1. Hình dạng hai API

```c
/* select(): dùng BA tập bit, sửa TẠI CHỖ */
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(fd, &readfds);
struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
int ready = select(nfds, &readfds, NULL, NULL, &tv);
// ⚠️ readfds và tv đều BỊ SỬA → phải khởi tạo lại TRƯỚC MỖI lần gọi

/* poll(): dùng MẢNG struct, tách events (vào) khỏi revents (ra) */
struct pollfd fds[2];
fds[0].fd = fd1;  fds[0].events = POLLIN;
fds[1].fd = fd2;  fds[1].events = POLLIN | POLLOUT;
int ready = poll(fds, 2, 5000);        // timeout tính bằng mili-giây
// ✅ events KHÔNG bị sửa; chỉ revents được ghi → không phải dựng lại mảng
```

| | `select()` | `poll()` |
|---|---|---|
| Giới hạn số fd | **`FD_SETSIZE`, thường 1024** — trần cứng | **Không** giới hạn cố định |
| Cấu trúc | 3 tập bit riêng (read/write/except) | 1 mảng, mỗi phần tử một fd |
| Đầu vào bị sửa? | **Có** — cả `fd_set` lẫn `timeval` | **Không** (chỉ `revents`) |
| Độ phân giải timeout | micro-giây | mili-giây |
| Khả chuyển | Rộng nhất | POSIX, rất rộng |

> ⚠️ **Bẫy `FD_SETSIZE`:** vượt quá là **undefined behavior**, không phải lỗi trả về — thường là ghi đè bộ nhớ. Tăng nó đòi biên dịch lại glibc (chính Kerrisk phải làm vậy để chạy bài đo ở §63.4.5). Đây là lý do đủ để **không dùng `select()` cho server nhiều kết nối**.

### 3.2. Vì sao hiệu năng sập khi N lớn

Kerrisk chỉ ra ba nguyên nhân (§63.2.5 và §63.4.5, tr. 1345, 1365) — hiểu ba cái này là trả lời được câu "vì sao epoll nhanh hơn":

1. **Mỗi lần gọi phải truyền lại TOÀN BỘ danh sách fd** từ user sang kernel — sao chép O(N) mỗi lần.
2. **Kernel phải duyệt hết N fd** để kiểm tra từng cái, rồi **tháo bỏ** thông tin đó khi trả về — không có gì được giữ lại giữa các lần gọi.
3. **Chương trình phải duyệt lại toàn bộ N phần tử** để tìm cái nào sẵn sàng (kể cả khi chỉ 1 cái sẵn sàng).

Đối lập: `epoll` **giữ danh sách quan tâm bên trong kernel** (interest list) — đăng ký một lần bằng `epoll_ctl()`, và mỗi `epoll_wait()` chỉ trả về **đúng những fd sẵn sàng**.

### 3.3. Số đo thật của sách 🎯

Table 63-9 (tr. 1365) — 100 000 lượt theo dõi, Linux 2.6.25, mỗi lượt đúng 1 fd sẵn sàng:

| Số fd theo dõi (N) | `poll()` (giây) | `select()` (giây) | **`epoll`** (giây) |
|---|---|---|---|
| 10 | 0,61 | 0,73 | **0,41** |
| 100 | 2,9 | 3,0 | **0,42** |
| 1 000 | 35 | 35 | **0,53** |
| 10 000 | **990** | **930** | **0,66** |

**Đọc bảng này cho đúng — đây là chỗ ăn điểm:**

- `poll`/`select` tăng **tuyến tính theo N**: từ 10 → 10 000 fd, thời gian tăng **~1600 lần**.
- `epoll` gần như **phẳng**: 0,41 → 0,66 giây, chỉ tăng ~1,6 lần (Kerrisk còn ghi chú phần tăng nhỏ này *có thể* do chạm giới hạn cache CPU, không phải do thuật toán).
- Ở N = 10 000: **epoll nhanh hơn ~1500 lần**. Đây là toàn bộ lý do C10K problem được giải bằng epoll.
- 🆕 Nhưng chú ý điều kiện thí nghiệm: **đúng 1 fd sẵn sàng mỗi lượt**. Nếu **hầu hết fd đều sẵn sàng** thì khoảng cách thu hẹp nhiều — epoll thắng lớn ở kịch bản *nhiều kết nối, ít hoạt động đồng thời*, đúng kịch bản web server/daemon thật.

---

## 4. `epoll` — API và cách dùng (§63.4, tr. 1354–1372) 🎯

### 4.1. Ba syscall và hai danh sách

```
   ┌──────────────────────── KERNEL ────────────────────────┐
   │                                                        │
   │   INTEREST LIST  (danh sách quan tâm)                   │
   │   ┌──────────────────────────┐                          │
   │   │ fd 4  → EPOLLIN          │  ← epoll_ctl(ADD/MOD/DEL)│
   │   │ fd 5  → EPOLLIN|EPOLLET  │     sửa danh sách này     │
   │   │ fd 9  → EPOLLOUT         │                          │
   │   │ ...  (giữ NGUYÊN giữa    │                          │
   │   │       các lần gọi)       │                          │
   │   └───────────┬──────────────┘                          │
   │               │ kernel tự cập nhật khi có sự kiện        │
   │   READY LIST  ▼ (tập con — cái nào ĐANG sẵn sàng)        │
   │   ┌──────────────────────────┐                          │
   │   │ fd 5, fd 9               │  ← epoll_wait() LẤY từ đây│
   │   └──────────────────────────┘                          │
   └────────────────────────────────────────────────────────┘
             ▲                                    │
             │ epoll_create() trả về một fd        ▼ chỉ trả về phần SẴN SÀNG
        (đóng bằng close())                   không phải toàn bộ N
```

*(sơ đồ 🆕, tổng hợp từ mô tả §63.4.1–63.4.3)*

```c
int epfd = epoll_create(size);          // size: gợi ý số fd (từ 2.6.8 bị bỏ qua)
                                        // hiện đại: epoll_create1(EPOLL_CLOEXEC)
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev);
        // op = EPOLL_CTL_ADD | EPOLL_CTL_MOD | EPOLL_CTL_DEL
int epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout);
        // trả về SỐ fd sẵn sàng; evlist chỉ chứa CHÍNH XÁC bấy nhiêu phần tử
```

**Điểm khiến `epoll` khác về bản chất:** `epfd` **tự nó là một file descriptor**. Nên bạn có thể đưa một epoll instance vào một epoll instance khác, hoặc vào `select()` — và đó là cách các thư viện event loop lồng nhau hoạt động.

```c
struct epoll_event {
    uint32_t     events;    // cờ EPOLLIN, EPOLLOUT, EPOLLET, EPOLLONESHOT...
    epoll_data_t data;      // union: giá trị TA tự đặt, kernel trả lại nguyên vẹn
};
typedef union epoll_data {
    void    *ptr;           // ← thường dùng nhất trong code thật:
    int      fd;            //    trỏ tới struct kết nối của mình
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;
```

🆕 `data` là chỗ thiết kế đẹp mà `poll()` không có: bạn nhét con trỏ tới `struct connection*` của mình vào, `epoll_wait()` trả lại y nguyên → **không phải tra bảng fd→connection**. Đây là lý do mọi event loop thật đều dùng `data.ptr`, không dùng `data.fd`.

### 4.2. Khung dùng đầy đủ (level-triggered — mặc định)

```c
#define MAX_EVENTS 64

int epfd = epoll_create1(EPOLL_CLOEXEC);   // ⚠️ CLOEXEC: xem ghi chú dưới
if (epfd == -1) errExit("epoll_create1");

struct epoll_event ev;
ev.events  = EPOLLIN;                      // level-triggered là MẶC ĐỊNH
ev.data.fd = listenFd;
if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenFd, &ev) == -1)
    errExit("epoll_ctl ADD");

struct epoll_event evlist[MAX_EVENTS];
for (;;) {
    int nready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);   // -1 = chờ vô hạn
    if (nready == -1) {
        if (errno == EINTR) continue;      // ⚠️ bị signal cắt → gọi lại, KHÔNG phải lỗi
        errExit("epoll_wait");
    }
    for (int j = 0; j < nready; j++) {     // CHỈ duyệt nready phần tử, không phải N
        if (evlist[j].events & EPOLLIN) {
            /* đọc được — level-triggered nên không bắt buộc đọc cạn */
        }
        if (evlist[j].events & (EPOLLHUP | EPOLLERR)) {
            /* peer đóng / lỗi → close() sẽ TỰ ĐỘNG gỡ fd khỏi interest list */
            close(evlist[j].data.fd);
        }
    }
}
```

### 4.3. Cờ trả về — ba cái hay bị hiểu sai

| Cờ | Ý nghĩa | Ghi chú |
|---|---|---|
| `EPOLLIN` | Đọc được mà không chặn | Bao gồm cả **EOF** (`read()` sẽ trả 0) |
| `EPOLLOUT` | Ghi được mà không chặn | Chỉ đăng ký khi **thật sự có gì để ghi**, nếu không sẽ báo liên tục (busy loop) |
| `EPOLLHUP` | Peer đóng **hoàn toàn** | **Luôn được báo dù không đăng ký** |
| `EPOLLERR` | Có lỗi | **Luôn được báo dù không đăng ký** |
| `EPOLLRDHUP` | Peer đóng **nửa ghi** (shutdown write) | Linux 2.6.17+; phân biệt "đóng lịch sự" với lỗi |
| `EPOLLONESHOT` | Báo **một lần** rồi tự vô hiệu hoá fd | Phải `EPOLL_CTL_MOD` để bật lại. **Mấu chốt cho epoll + thread pool** |

*(theo Table 63-8, tr. 1362)*

> 🆕 **`EPOLLONESHOT` dùng để làm gì:** với nhiều thread cùng `epoll_wait()` trên một epfd, **hai thread có thể cùng nhận sự kiện của một fd** và xử lý song song trên cùng một kết nối — race. `EPOLLONESHOT` đảm bảo mỗi fd chỉ được giao cho **một** thread tại một thời điểm; thread xử lý xong thì tự đăng ký lại. Đây là mẫu chuẩn của epoll + thread pool.

### 4.4. Ngữ nghĩa `close()` và bẫy `dup()`

Đóng một fd **tự động gỡ nó khỏi mọi interest list**. Nhưng Kerrisk nêu một chi tiết tinh (§63.4.4, tr. 1364) rất hay thành bug:

> Interest list thật ra chứa **file description** (tầng ②, xem [cụm 01 §2.3](01-concepts-and-file-io.md)), không phải fd. Nên nếu bạn `dup()` một fd rồi `close()` **bản gốc**, **file description vẫn sống** → sự kiện **vẫn tiếp tục được báo**, mà fd trong `data.fd` thì đã đóng. Kết quả: `read()` trên fd đã đóng → `EBADF`, hoặc tệ hơn là fd đó đã bị cấp lại cho kết nối khác → **đọc nhầm dữ liệu của kết nối khác**.

🆕 Đây là một trong những bug khó nhất của event loop. Quy tắc an toàn: **`EPOLL_CTL_DEL` tường minh trước khi đóng bất cứ fd nào có thể đã bị `dup`**.

---

## 5. Signal-driven I/O (§63.3, tr. 1346–1354)

Mô hình thứ ba: đăng ký một lần, kernel **gửi signal** (mặc định `SIGIO`) khi fd sẵn sàng.

```c
// Thiết lập (rút gọn):
fcntl(fd, F_SETOWN, getpid());              // ① ai nhận signal
int flags = fcntl(fd, F_GETFL);
fcntl(fd, F_SETFL, flags | O_ASYNC | O_NONBLOCK);  // ② bật báo hiệu bằng signal
// + cài handler cho SIGIO
```

Về lý thuyết hiệu quả (kernel không phải duyệt danh sách), nhưng Kerrisk nêu các lý do khiến nó **hầu như không được dùng trong thực tế**:

- **Signal là kênh nghèo nàn:** signal chuẩn **không xếp hàng** — nhiều sự kiện dồn có thể mất. Phải chuyển sang **realtime signal** (`F_SETSIG`) để có hàng đợi và biết fd nào gây ra.
- **Hàng đợi signal có giới hạn** (`RLIMIT_SIGPENDING`); tràn thì kernel quay về `SIGIO` và bạn **mất thông tin fd** → phải quét lại toàn bộ.
- **Chỉ có edge-triggered** (§63.3, tr. 1347), với mọi cái khó của ET nhưng không có `data.ptr` tiện lợi.
- Xử lý trong signal handler bị ràng buộc **async-signal-safe** (xem cụm 03).

> **Kết luận thực tế:** biết nó tồn tại và biết **vì sao epoll thay thế nó** là đủ. 🆕 `signalfd()` ra đời chính để biến signal thành fd đọc được — tức là kéo signal *vào* mô hình epoll thay vì ngược lại.

---

## 6. Ít quan trọng — 1 dòng + tham chiếu

- **§63.2.1 — `select()` chi tiết, `FD_*` macro** (tr. 1331): cú pháp tra khi cần đọc code cũ.
- **§63.2.3 — `poll()` chi tiết, đủ bộ cờ `POLLxxx`** (tr. 1337): bảng cờ; `POLLNVAL` = fd không hợp lệ.
- **§63.2.6 — Hành vi khi theo dõi từng loại file** (tr. 1345): bảng "khi nào coi là ready" cho pipe/FIFO/socket/terminal/file thường. **File thường LUÔN được coi là sẵn sàng** — đó là lý do multiplexing vô dụng với file trên đĩa (và là lý do `io_uring` ra đời).
- **§63.5 — Chờ trên signal & fd cùng lúc** (tr. 1372): `pselect()`, `ppoll()`, `epoll_pwait()` — nhận `sigmask` nguyên tử, giải bài toán race giữa kiểm tra cờ và đi ngủ (self-pipe trick là cách cũ hơn).
- **§63.6 — Tổng kết & so sánh cuối chương** (tr. 1373).

---

## 7. Góc interview 🎯

<details><summary>1) So sánh <code>select()</code>, <code>poll()</code>, <code>epoll</code>. Vì sao epoll nhanh hơn hẳn khi nhiều kết nối?</summary>

**Cả ba trả lời cùng một câu hỏi:** *"trong các fd này, cái nào gọi `read()`/`write()` mà không bị chặn?"* Chúng **không** thực hiện I/O.

| | `select()` | `poll()` | `epoll` |
|---|---|---|---|
| Giới hạn fd | **`FD_SETSIZE` ~1024** | không | không |
| Đầu vào bị sửa | **có** (dựng lại mỗi lần) | không | không cần truyền lại |
| Độ phức tạp mỗi lần gọi | O(N) | O(N) | **O(số fd sẵn sàng)** |
| Level/Edge | level | level | **cả hai** |

**Ba lý do epoll nhanh hơn:**
1. `select`/`poll` **truyền lại toàn bộ danh sách N fd** từ user sang kernel **mỗi lần gọi**; epoll đăng ký **một lần** bằng `epoll_ctl()`, kernel giữ *interest list* lâu dài.
2. Kernel với `select`/`poll` phải **duyệt hết N fd** rồi vứt bỏ kết quả; epoll duy trì sẵn **ready list** được cập nhật bởi callback khi sự kiện xảy ra.
3. Chương trình với `select`/`poll` phải **quét lại N phần tử** để tìm cái sẵn sàng; `epoll_wait()` trả về **đúng những cái sẵn sàng** và số lượng của chúng.

**Số thật của TLPI** (Table 63-9, 100 000 lượt theo dõi): với N = 10 fd, ba cái tương đương (0,6 / 0,7 / 0,4 giây). Với **N = 10 000**: `poll` **990 giây**, `select` **930 giây**, `epoll` **0,66 giây** — nhanh hơn **~1500 lần**.

**Đánh đổi phải nêu:** `epoll` là **Linux-only** (BSD/macOS có `kqueue`, Windows có IOCP); `select`/`poll` khả chuyển. Và ở **N nhỏ** (vài chục fd) epoll không lợi gì đáng kể — thêm phức tạp mà không đổi lại gì.

**Điểm cộng:** với **file thường trên đĩa**, cả ba đều vô dụng vì file thường **luôn được báo là sẵn sàng** — đó là một trong những động lực sinh ra `io_uring`.
</details>

<details><summary>2) Level-triggered và edge-triggered khác nhau thế nào? Dùng sai gây bug gì?</summary>

- **Level-triggered:** báo **khi fd đang ở trạng thái sẵn sàng**. Còn dữ liệu chưa đọc thì lần `epoll_wait()` sau **vẫn báo tiếp**.
- **Edge-triggered (`EPOLLET`):** chỉ báo **khi có sự kiện MỚI** kể từ lần kiểm tra trước. Dữ liệu tồn đọng **không** sinh thông báo mới.

**Ví dụ tách bạch hai cái:** 100 byte tới socket → `epoll_wait` báo → ta chỉ đọc 40 byte → gọi `epoll_wait` lần nữa:
- **Level:** báo tiếp (vẫn còn 60 byte).
- **Edge:** **bị chặn** — 60 byte đó nằm im cho tới khi có gói dữ liệu mới. Nếu client đang chờ phản hồi và không gửi thêm gì → **deadlock ứng dụng**.

**Bug kinh điển khi dùng ET sai:** đọc một lần rồi đi tiếp → dữ liệu tồn đọng bị bỏ quên → kết nối "treo ngẫu nhiên", tải thấp thì không thấy, tải cao mới lộ, và **rất khó tái hiện**.

**Quy tắc bắt buộc khi dùng ET:**
1. **Mọi fd phải non-blocking** — nếu không, vòng lặp đọc-cho-cạn sẽ chặn vĩnh viễn ở lần đọc cuối.
2. **Đọc/ghi trong vòng lặp cho tới khi `EAGAIN`/`EWOULDBLOCK`** — đó mới là điều kiện dừng đúng.
3. Đề phòng **starvation**: một fd có luồng dữ liệu bất tận sẽ chiếm hết lượt nếu bạn vắt kiệt nó. TLPI khuyên tự giữ danh sách fd sẵn sàng và **giới hạn lượng I/O mỗi fd một lượt** (round-robin).

**Khi nào chọn cái nào:** mặc định **level-triggered** — đúng, dễ, khó sai. Chỉ chuyển sang ET khi **đã đo** thấy số lần gọi `epoll_wait` là nút cổ chai. `select()`/`poll()` chỉ có level; signal-driven I/O chỉ có edge; **epoll có cả hai** — đó là một lý do nữa nó thắng.
</details>

<details><summary>3) Thiết kế một server xử lý 10 000 kết nối đồng thời — bạn chọn kiến trúc nào?</summary>

**Loại trừ trước:**
- **Một process/thread mỗi kết nối:** 10 000 thread × 8 MB stack ảo, context switch ngập; sập vì tài nguyên chứ không phải vì CPU tính toán.
- **`select()`:** chết ngay ở `FD_SETSIZE` = 1024, và vượt quá là **undefined behavior** chứ không phải lỗi trả về.
- **`poll()`:** không giới hạn fd nhưng O(N) mỗi lần gọi — theo số đo TLPI, 10 000 fd mất 990 giây cho 100 000 lượt.

**Chọn: event loop dùng `epoll` + non-blocking fd**, cụ thể:

1. **`epoll` level-triggered** làm mặc định — đúng và dễ bảo trì. Chỉ dùng `EPOLLET` nếu đo thấy cần.
2. **Non-blocking toàn bộ** (`O_NONBLOCK`), kể cả socket nghe.
3. **`ev.data.ptr` trỏ tới `struct connection*`** của mình → không phải tra bảng fd→connection.
4. **Đăng ký `EPOLLOUT` chỉ khi có dữ liệu chờ ghi**, gỡ ngay khi ghi hết — nếu để thường trực sẽ bị báo liên tục, thành busy loop.
5. **Nhiều lõi:** *N event loop × M worker* — mỗi lõi một event loop (dùng `SO_REUSEPORT` để kernel chia đều kết nối), việc nặng CPU/đĩa đẩy sang thread pool để không chặn loop. Nếu nhiều thread dùng chung một epfd thì **`EPOLLONESHOT`** để hai thread không cùng xử lý một kết nối.
6. **Xử lý `EPOLLRDHUP`/`EPOLLHUP`/`EPOLLERR`**, timeout kết nối nhàn rỗi, giới hạn số kết nối.
7. **`EPOLL_CTL_DEL` trước khi `close()`** nếu fd có thể đã bị `dup` — vì interest list gắn với *file description*, không phải fd.

**Đánh đổi phải nói ra:** kiến trúc này **Linux-only** (`kqueue` cho BSD/macOS, IOCP cho Windows) — nếu cần đa nền tảng thì dùng lớp trừu tượng (libevent/libuv/ASIO). Và event loop **không hợp việc nặng CPU**: một handler chạy lâu là chặn *toàn bộ* các kết nối khác — đó chính là lý do phải có thread pool.
</details>

<details><summary>4) Vì sao signal-driven I/O gần như không ai dùng, dù về lý thuyết nó hiệu quả?</summary>

Ý tưởng đúng: đăng ký một lần (`F_SETOWN` + `O_ASYNC`), kernel gửi `SIGIO` khi fd sẵn sàng — kernel **không phải duyệt danh sách**, nên về lý thuyết co giãn tốt như epoll.

Nhưng signal là **kênh truyền tin nghèo nàn**:

1. **Signal chuẩn không xếp hàng.** Nhiều sự kiện dồn dập trong lúc signal đang bị chặn chỉ còn **một** — mất thông báo. Phải chuyển sang **realtime signal** (`F_SETSIG`) mới có hàng đợi và mới biết **fd nào** gây ra sự kiện (qua `siginfo_t`).
2. **Hàng đợi realtime signal có trần** (`RLIMIT_SIGPENDING`). Tràn thì kernel **quay về `SIGIO`** và bạn mất luôn thông tin fd → phải quét lại toàn bộ, tức là mất sạch lợi thế.
3. **Chỉ có edge-triggered**, với đủ mọi cái khó của ET (phải non-blocking, phải đọc cho cạn) nhưng **không có** `data.ptr` để gắn ngữ cảnh.
4. **Xử lý trong signal handler bị trói bởi async-signal-safe** — gần như không gọi được gì hữu ích; thực tế phải đẩy sang main loop, tức là quay về... một vòng lặp sự kiện.

**Kết luận:** `epoll` cho cùng lợi ích co giãn mà không mượn cơ chế signal, lại có cả level-triggered và ngữ cảnh gắn kèm. 🆕 Chiều ngược lại mới là chiều thắng: **`signalfd()`** biến signal thành một fd đọc được, để **signal được kéo vào epoll** thay vì I/O bị đẩy vào signal.
</details>

<details><summary>5) <code>epoll_wait()</code> trả về -1 với <code>EINTR</code> — xử lý sao? Và vì sao <code>epoll_create1(EPOLL_CLOEXEC)</code>?</summary>

**`EINTR`:** lời gọi bị **signal handler cắt ngang** trước khi có sự kiện nào. Đây **không phải lỗi** — cách xử lý đúng là **gọi lại**:

```c
int nready;
do {
    nready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
} while (nready == -1 && errno == EINTR);
if (nready == -1) errExit("epoll_wait");
```

Lưu ý `SA_RESTART` **không cứu được** `epoll_wait`/`select`/`poll` — nhóm syscall này nằm trong danh sách **luôn** trả `EINTR` bất kể `SA_RESTART` (xem cụm 03). Vòng lặp lại là bắt buộc.

Nếu cần **chờ fd và signal cùng lúc mà không có race**, dùng `epoll_pwait()` (hoặc `ppoll`/`pselect`): nó đặt signal mask **nguyên tử** với việc đi ngủ, tránh khe hở giữa "kiểm tra cờ" và "gọi wait".

**`EPOLL_CLOEXEC`:** đặt `FD_CLOEXEC` cho `epfd` **ngay lúc tạo**. Cần vì:
- Nếu không, `epfd` **rò rỉ sang tiến trình con** qua `fork()` + `exec()` — con giữ tham chiếu tới file description, làm tài nguyên không được giải phóng, và là **lỗ hổng bảo mật** nếu con là chương trình ít tin cậy.
- Làm **nguyên tử** trong `epoll_create1()` thay vì `epoll_create()` rồi `fcntl(F_SETFD)` — vì giữa hai bước đó, một thread khác có thể `fork()`+`exec()` và fd rò ra. Cùng lý do tồn tại của `O_CLOEXEC`, `SOCK_CLOEXEC`, `accept4()`.
</details>

---

## 8. Đọc thêm (tùy chọn)

- Topic repo: [04/io-multiplexing.md](../../04-linux-system-programming/io-multiplexing.md) — bản gọn cùng chủ đề.
- Bank: `LNX-*` (epoll, LT vs ET) trong [linux-sysprog.md](../../14-prep/mock-interview/bank/linux-sysprog.md); `SD-*` cho câu thiết kế server.
- Góc kernel: [LKD](../lkd/) — cơ chế wait queue đứng sau `epoll_wait()`.
- Ring buffer làm nền cho `io_uring`: [12-dsa/ring-buffer.md §8](../../12-dsa/ring-buffer.md).

---
⬅️ [Cụm 01 — File I/O](01-concepts-and-file-io.md) · [README cụm TLPI](README.md)
