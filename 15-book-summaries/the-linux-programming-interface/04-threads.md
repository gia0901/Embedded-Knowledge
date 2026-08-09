# TLPI — Cụm 04: Threads (ch. 29–33) 🎯

> **Nguồn:** đã đối chiếu PDF; số trang là **trang sách** (PDF = sách + 44). Chương 29–33, tr. 617–698.
> **Vì sao cụm này 🎯:** bạn viết **C++ shared library đa luồng** — đây là cụm sát công việc nhất. Và câu *"thread-safe khác reentrant thế nào?"* là câu phân loại rất tốt vì hầu hết người trả lời gộp hai khái niệm làm một.

---

## 1. Thread là gì — cái gì chung, cái gì riêng (ch. 29, tr. 617–630)

### 1.1. Bản đồ bộ nhớ nhiều thread

```
   Địa chỉ ảo
   0xC0000000 ┌──────────────────────────────────────────────┐
              │  argv, environ                                │
              ├──────────────────────────────────────────────┤
              │  Stack thread chính              ▼           │ ┐
              ├──────────────────────────────────────────────┤ │
              │  Stack thread 3                  ▼           │ │  MỖI THREAD
              ├──────────────────────────────────────────────┤ ├─ MỘT STACK
              │  Stack thread 2                  ▼           │ │  RIÊNG
              ├──────────────────────────────────────────────┤ │  (biến cục bộ,
              │  Stack thread 1                  ▼           │ ┘   tham số hàm)
              ├──────────────────────────────────────────────┤
              │           (vùng chưa cấp phát)                │
              ├──────────────────────────────────────────────┤
              │  HEAP        ▲  malloc()                     │ ┐
              ├──────────────────────────────────────────────┤ │  DÙNG CHUNG
              │  BSS + DATA  — biến toàn cục / static        │ ├─ cho MỌI
              ├──────────────────────────────────────────────┤ │  thread
              │  TEXT        — mã lệnh                       │ ┘
   0x08048000 └──────────────────────────────────────────────┘
```

*(vẽ lại theo Figure 29-1, tr. 618)*

**Bảng phải thuộc** — đây là gốc của mọi câu hỏi về thread:

| Dùng CHUNG (mọi thread) | RIÊNG mỗi thread |
|---|---|
| PID, PPID, process group, session | **Thread ID** (`pthread_t`) |
| **Biến toàn cục và static** (`.data`, `.bss`) | **Stack** → biến cục bộ, tham số, địa chỉ trả về |
| **Heap** (`malloc`) | **`errno`** ⚠️ (xem dưới) |
| **Bảng file descriptor** | **Signal mask** |
| **Signal disposition** (handler) ⚠️ | Thread-specific data (TSD / TLS) |
| Thư mục làm việc, umask, UID/GID | Trạng thái cancellation, giá trị trả về |
| Giới hạn tài nguyên, timer | Ưu tiên và chính sách lập lịch |

> ⚠️ **Hai chỗ bất đối xứng dễ nhầm:**
> - **`errno` là per-thread** trong POSIX threads. Nếu nó toàn cục thật thì thread A gọi syscall lỗi sẽ ghi đè `errno` của thread B — không dùng được. glibc cài `errno` thành macro trỏ vào vùng riêng của thread.
> - **Signal handler dùng chung cả process, nhưng signal mask thì riêng mỗi thread.** Nghĩa là bạn không thể cài handler khác nhau cho từng thread, nhưng có thể **chặn** signal ở thread này và cho phép ở thread kia — đó là nền của mẫu "một thread chuyên nhận signal" (§4.3).

### 1.2. API cơ bản

```c
static void *threadFunc(void *arg) {         // ký hiệu bắt buộc: void* → void*
    char *s = (char *) arg;
    printf("%s", s);
    return (void *) strlen(s);               // giá trị trả về lấy được qua pthread_join
}

int main(void) {
    pthread_t t;
    void *res;

    int s = pthread_create(&t, NULL, threadFunc, "Xin chào\n");
    if (s != 0) errExitEN(s, "pthread_create");   // ⚠️ trả MÃ LỖI, KHÔNG đặt errno

    s = pthread_join(t, &res);                    // chờ + thu hoạch
    if (s != 0) errExitEN(s, "pthread_join");
    printf("Thread trả về %ld\n", (long) res);
    exit(EXIT_SUCCESS);
}
```

> ⚠️ **Khác biệt về xử lý lỗi:** hàm Pthreads **trả về mã lỗi trực tiếp** (0 = thành công), **không** trả `-1` và **không** đặt `errno`. Viết `if (pthread_create(...) == -1)` là **sai hoàn toàn** — nó không bao giờ trả `-1`. Đây là lý do TLPI có hàm riêng `errExitEN()`.

### 1.3. `pthread_join()` — và "zombie thread"

Kerrisk chỉ ra sự tương ứng với process nhưng nhấn mạnh **ba khác biệt** (§29.6, tr. 625–626):

| | Process | Thread |
|---|---|---|
| Quan hệ | **Phân cấp** — chỉ cha `wait()` được con | **Ngang hàng** — thread bất kỳ join thread bất kỳ |
| "Chờ đứa nào cũng được" | `waitpid(-1, …)` ✅ | ❌ **Không có** |
| Chờ không chặn | `WNOHANG` ✅ | ❌ **Không có** |

> *"If a thread is not detached, then we must join with it using `pthread_join()`. If we fail to do this, then, when the thread terminates, it produces the thread equivalent of a zombie process... if enough thread zombies accumulate, we won't be able to create additional threads."* (tr. 626)

**Vì sao cố tình không có "join bất kỳ thread nào":** Kerrisk giải thích đây là **quyết định thiết kế** (tr. 626) — thread không có phân cấp, nên "join bất kỳ" có thể vô tình join phải **thread do một hàm thư viện tạo riêng**. Thư viện đó sau này join lại sẽ hỏng. *"A 'join with any thread' operation is incompatible with modular program design."*

**Thread không cần thu hoạch → `detach`:**

```c
pthread_detach(t);              // hoặc đặt attr PTHREAD_CREATE_DETACHED lúc tạo
// → tài nguyên tự giải phóng khi thread kết thúc; KHÔNG join được nữa
```

---

## 2. Đồng bộ hoá (ch. 30, tr. 631–654) 🎯

### 2.1. Vì sao cần — race trên một phép `++`

```
   glob = 0;  hai thread cùng chạy  loc = glob; loc++; glob = loc;

   Thread A                    Thread B                   glob
   ────────────────────────────────────────────────────────────
   loc = glob   (loc=0)                                     0
                               loc = glob   (loc=0)         0    ← đọc TRƯỚC khi A ghi
   loc++        (loc=1)                                     0
   glob = loc                                               1
                               loc++        (loc=1)         1
                               glob = loc                   1    ← MẤT một lần tăng
```

*(theo Figure 30-1, tr. 634)*

Không phải "lý thuyết" — Kerrisk chạy thật với 10 triệu vòng và cho ra kết quả sai lệch rất lớn. Nguyên nhân: `glob++` là **read-modify-write**, ba bước, và thread có thể bị preempt ở bất kỳ đâu giữa ba bước.

### 2.2. Mutex

```c
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;   // khởi tạo tĩnh
// hoặc động: pthread_mutex_init(&mtx, NULL);  ... pthread_mutex_destroy(&mtx);

pthread_mutex_lock(&mtx);
    glob++;                          // vùng tới hạn
pthread_mutex_unlock(&mtx);
```

**Ba tính chất phải nói được:**

1. **Ownership** — chỉ thread **đã khoá** mới được mở khoá. Đây là khác biệt bản chất với semaphore (bên nào cũng `post` được).
2. **`pthread_mutex_trylock()`** — thử khoá, bận thì trả `EBUSY` ngay thay vì chờ.
3. **Loại mutex** (§30.1.7, tr. 649) — `PTHREAD_MUTEX_NORMAL` (mặc định: tự khoá lại = **deadlock**), `ERRORCHECK` (trả lỗi thay vì deadlock — dùng khi debug), `RECURSIVE` (đếm số lần khoá; tiện nhưng thường là dấu hiệu thiết kế chưa rõ).

### 2.3. Deadlock và cách phá

```
   Thread A                        Thread B
   lock(mtx1)  ✅                  lock(mtx2)  ✅
   lock(mtx2)  ⏳ chờ B nhả  ◄──┐  lock(mtx1)  ⏳ chờ A nhả
                                └──────────────┘
                        VÒNG TRÒN CHỜ → treo vĩnh viễn
```

*(theo Figure 30-3, tr. 641)*

**Cách chữa chuẩn — lock ordering:** mọi thread khoá theo **cùng một thứ tự toàn cục** (vd theo địa chỉ mutex). Kerrisk nêu đúng điều này (tr. 641). Cách thay thế khi không áp được thứ tự: `pthread_mutex_trylock()` — lấy được cái thứ hai thì tiếp, không thì **nhả cái đầu** và thử lại từ đầu.

🆕 Trong C++17 đã có sẵn: **`std::scoped_lock`** khoá nhiều mutex **một lượt** bằng thuật toán tránh deadlock — không cần tự lo thứ tự. Đây là câu trả lời "hiện đại" đáng nêu ([02/concurrency](../../02-modern-cpp/concurrency.md), [OS-003](../../14-prep/mock-interview/bank/os.md)).

### 2.4. Condition variable — và vì sao PHẢI dùng `while`

Mutex bảo vệ dữ liệu; **condition variable** giải bài toán khác: *"chờ tới khi một điều kiện thành đúng"* mà **không** đốt CPU.

```c
static pthread_mutex_t mtx  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
static int avail = 0;                       // ← "predicate" nằm ở đây

/* ===== Consumer ===== */
pthread_mutex_lock(&mtx);
while (avail == 0)                          // ⚠️ WHILE, KHÔNG PHẢI IF
    pthread_cond_wait(&cond, &mtx);         // nhả mtx + ngủ; dậy thì khoá lại mtx
/* tới đây chắc chắn avail > 0 VÀ đang giữ mtx */
avail--;
pthread_mutex_unlock(&mtx);

/* ===== Producer ===== */
pthread_mutex_lock(&mtx);
avail++;
pthread_mutex_unlock(&mtx);                 // nhả TRƯỚC khi signal (xem ghi chú)
pthread_cond_signal(&cond);
```

**`pthread_cond_wait()` làm ba việc, và tính nguyên tử của chúng là mấu chốt:**

```
   ① NHẢ mutex          ┐
   ② ĐI NGỦ chờ signal  ┘ ← ① và ② là MỘT thao tác nguyên tử
   ③ khi được đánh thức: KHOÁ LẠI mutex rồi mới trả về
```

Nếu ① và ② **không** nguyên tử thì có khe hở: nhả mutex xong, producer chen vào `signal` **trước khi** consumer kịp ngủ → tín hiệu bay mất → consumer ngủ vĩnh viễn (**lost wakeup**). Cùng một mẫu race với `sigsuspend` ở [cụm 03 §4.2](03-signals-and-timers.md).

> ⚠️ **Vì sao `while` chứ không `if`** — ba lý do, nêu được cả ba là trả lời đầy đủ:
> 1. **Spurious wakeup** — POSIX cho phép `pthread_cond_wait()` trả về mà **không ai signal**. Đây là nhượng bộ để cài đặt hiệu quả hơn trên một số nền tảng.
> 2. **Stolen wakeup** — được signal thật, nhưng một thread khác dậy trước và **lấy mất** tài nguyên. Đến lượt mình thì `avail` đã về 0.
> 3. **`pthread_cond_broadcast()`** đánh thức **mọi** thread đang chờ, nhưng có thể chỉ đủ tài nguyên cho một.
>
> Với `if`, cả ba ca đều dẫn tới **chạy tiếp khi điều kiện SAI** — xử lý dữ liệu không tồn tại. Với `while`, thread chỉ **quay lại ngủ**. Xem [OS-012](../../14-prep/mock-interview/bank/os.md).

**`signal` hay `broadcast`?** `signal` đánh thức **ít nhất một**; `broadcast` đánh thức **tất cả**. Dùng `broadcast` khi các thread chờ những điều kiện **khác nhau**, hoặc khi một sự kiện làm nhiều thread chạy tiếp được (vd reader-writer). Dùng `signal` khi mọi thread chờ tương đương và một sự kiện chỉ phục vụ được một — `broadcast` lúc đó gây **thundering herd**.

---

## 3. Thread safety vs reentrancy (ch. 31, tr. 655–670) 🎯

### 3.1. Hai khái niệm KHÁC nhau

**Định nghĩa của sách** (§31.1, tr. 655): *"A function is said to be thread-safe if it can safely be invoked by multiple threads at the same time."*

```c
/* KHÔNG thread-safe — Kerrisk dùng chính ví dụ này (tr. 655) */
static int glob = 0;
static void incr(int loops) {
    int loc, j;
    for (j = 0; j < loops; j++) {
        loc = glob;  loc++;  glob = loc;   // ← trạng thái TOÀN CỤC bị nhiều thread ghi
    }
}
```

*"This function illustrates the typical reason that a function is not thread-safe: it employs global or static variables that are shared by all threads."*

| | **Thread-safe** | **Reentrant** |
|---|---|---|
| Câu hỏi | Nhiều **thread** gọi cùng lúc có an toàn không? | Bị cắt ngang giữa chừng rồi gọi lại chính nó có an toàn không? |
| Cách đạt được | Có thể **dùng khoá** | **KHÔNG được dùng khoá** — phải không có trạng thái chia sẻ |
| Ngữ cảnh điển hình | Đa luồng | **Signal handler**, ISR |

> **Quan hệ một chiều:** **reentrant ⇒ thread-safe**, nhưng **thread-safe ⇏ reentrant**. Một hàm bọc `pthread_mutex_lock` là thread-safe nhưng **không** reentrant — gọi nó từ signal handler khi luồng chính đang giữ khoá đó là **deadlock**. Đây chính là lý do danh sách *async-signal-safe* ([cụm 03 §2.2](03-signals-and-timers.md)) **hẹp hơn nhiều** so với danh sách thread-safe.

### 3.2. Hai cách làm cho thread-safe — và cái giá

Kerrisk trình bày rõ đánh đổi (tr. 655–656):

```
   ❌ Cách thô: một mutex bọc CẢ HÀM
      → đơn giản, nhưng "access to the function is serialized"
      → mất song song hoàn toàn: N thread chạy tuần tự như 1 thread

   ✅ Cách tinh: mutex gắn với BIẾN CHIA SẺ, chỉ khoá đúng vùng tới hạn
      → nhiều thread chạy song song, chỉ tuần tự đúng lúc đụng biến chung
```

**Cách tốt nhất là bỏ hẳn trạng thái chia sẻ** → hàm trở thành **reentrant**. Đó là lý do POSIX sinh ra loạt biến thể `_r`:

| Không an toàn (trả con trỏ tĩnh) | Bản `_r` (caller cấp buffer) |
|---|---|
| `strtok()` | `strtok_r()` |
| `asctime()`, `ctime()`, `localtime()`, `gmtime()` | `asctime_r()`, `ctime_r()`, … |
| `getpwnam()`, `getgrnam()` | `getpwnam_r()`, `getgrnam_r()` |
| `readdir()` | `readdir_r()` (nay không khuyến nghị — dùng `readdir` với directory stream riêng) |

**Nguyên tắc chung của họ `_r`:** thay vì trả con trỏ tới **bộ nhớ tĩnh dùng chung**, chúng nhận **buffer do caller cấp**. Không còn trạng thái chia sẻ → reentrant.

### 3.3. Thread-specific data (TSD) và thread-local storage (TLS)

Khi **buộc phải** giữ trạng thái riêng cho từng thread (vd viết lại một API cũ mà không đổi chữ ký hàm):

```c
/* Cách hiện đại — TLS, đơn giản nhất, hỗ trợ từ gcc/glibc */
static __thread char buf[MAX_LEN];        // mỗi thread MỘT bản riêng
// C++11: thread_local char buf[MAX_LEN];

/* Cách POSIX chuẩn — TSD, phức tạp hơn nhưng có destructor */
static pthread_key_t key;
pthread_key_create(&key, free);           // destructor gọi khi thread kết thúc
pthread_setspecific(key, malloc(MAX_LEN));
char *p = pthread_getspecific(key);
```

🆕 **Chính `errno` được cài bằng cơ chế này.** Và đó là lý do `errno` là một **macro**, không phải biến — nó nở ra thành lời gọi lấy vùng riêng của thread hiện tại.

---

## 4. Cancellation & những chi tiết còn lại (ch. 32–33, tr. 671–698)

### 4.1. Huỷ thread — và vì sao nó khó

```c
pthread_cancel(t);         // GỬI yêu cầu huỷ — KHÔNG phải huỷ ngay
```

Ba trạng thái quyết định điều gì xảy ra:

| Thiết lập | Ý nghĩa |
|---|---|
| `PTHREAD_CANCEL_ENABLE` + **`DEFERRED`** (mặc định) | Chỉ huỷ tại **cancellation point** — các hàm có thể chặn (`read`, `write`, `sleep`, `pthread_cond_wait`, …) |
| `PTHREAD_CANCEL_ASYNCHRONOUS` | Huỷ **bất cứ lúc nào** — ⚠️ gần như không dùng được: thread có thể chết giữa lúc đang giữ mutex hoặc đang `malloc` |
| `PTHREAD_CANCEL_DISABLE` | Hoãn huỷ — bọc quanh vùng tới hạn |

**Vấn đề cốt lõi:** thread bị huỷ giữa chừng có thể **đang giữ mutex** hoặc **đang giữ bộ nhớ đã `malloc`** → khoá kẹt vĩnh viễn, rò rỉ bộ nhớ. Lời giải là **cleanup handler**:

```c
pthread_cleanup_push(free, buf);          // đăng ký hàm dọn dẹp
pthread_cleanup_push(cleanupUnlock, &mtx);
    /* ... công việc có thể bị huỷ ... */
pthread_cleanup_pop(1);                   // 1 = CHẠY handler; 0 = chỉ gỡ đăng ký
pthread_cleanup_pop(1);                   // chạy NGƯỢC thứ tự đăng ký (như stack)
```

> 🆕 **Trong C++ đừng dùng `pthread_cancel`.** Nó không chạy destructor của object trên stack theo cách đáng tin, nên **phá vỡ RAII**. Cách đúng: **cờ huỷ hợp tác** — thread tự kiểm tra định kỳ và tự thoát sạch. C++20 chuẩn hoá đúng mẫu này bằng **`std::jthread` + `std::stop_token`** ([EMC++ Item 37](../effective-modern-cpp.md)).

### 4.2. Thread và `fork()` — bẫy nghiêm trọng

Sau `fork()` trong chương trình đa luồng, **chỉ thread gọi `fork()` tồn tại trong process con**. Mọi thread khác biến mất — **nhưng trạng thái chúng để lại thì không**:

```
   TRƯỚC fork():  Thread A giữ mutex M, đang sửa dở cấu trúc dữ liệu
                  Thread B (gọi fork)
                          │
                          ▼  fork()
   CON:           Thread B tồn tại (thành thread duy nhất)
                  Mutex M: ĐANG BỊ KHOÁ, bởi một thread KHÔNG CÒN TỒN TẠI
                  → không ai nhả được → thread nào khoá M sẽ TREO VĨNH VIỄN
                  → cấu trúc dữ liệu đang sửa dở → hỏng
```

**Hệ quả:** giữa `fork()` và `exec()`, process con **chỉ được gọi hàm async-signal-safe**. Đặc biệt **`malloc()` có thể deadlock** — nó dùng mutex nội bộ mà một thread khác có thể đang giữ lúc `fork()`.

`pthread_atfork()` cho phép đăng ký handler chạy trước/sau `fork` để lo việc này, nhưng Kerrisk (và thực tế) khuyên: **cách đơn giản nhất là `exec()` ngay sau `fork()`**, hoặc `fork()` **trước khi** tạo thread nào.

### 4.3. Thread và signal — thiết kế đúng

> *"As a general principle, it is usually desirable to avoid the use of signals in multithreaded programs."* (§29.9, tr. 629)

Vì signal đặt ra hai bất đối xứng khó chịu: **handler dùng chung cả process** nhưng **mask riêng từng thread**; và signal gửi cho process sẽ được giao cho **một thread bất kỳ** không đang chặn nó.

**Mẫu chuẩn — một thread chuyên nhận signal:**

```c
/* ① Trong thread CHÍNH, TRƯỚC khi tạo bất kỳ thread nào: chặn signal quan tâm.
      Mask được KẾ THỪA bởi mọi thread tạo sau → không thread nào nhận signal. */
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
sigaddset(&set, SIGTERM);
pthread_sigmask(SIG_BLOCK, &set, NULL);

/* ② Một thread chuyên trách chờ signal ĐỒNG BỘ — không cần handler,
      nên thoát hẳn ràng buộc async-signal-safe */
static void *signalThread(void *arg) {
    int sig;
    for (;;) {
        sigwait(&set, &sig);            // chờ, lấy signal về như dữ liệu thường
        /* ✅ ở đây gọi được HÀM BẤT KỲ — đây không phải signal handler */
        printf("nhận signal %d, bắt đầu tắt máy\n", sig);
        shutdown_gracefully();
    }
}
```

🆕 Đây là mẫu mà mọi daemon đa luồng dùng, và là chị em với `signalfd()` ([cụm 03 §4.3](03-signals-and-timers.md)) — cả hai đều **biến signal bất đồng bộ thành sự kiện đồng bộ**, để không phải viết code trong signal handler.

### 4.4. Thread vs process — chọn thế nào

Kerrisk liệt kê cân bằng (§29.9, tr. 628–629):

| **Ưu của thread** | **Nhược của thread** |
|---|---|
| **Chia sẻ dữ liệu dễ** — cùng address space; process phải dựng shared memory hoặc pipe | **Phải lo thread-safety** cho mọi hàm gọi tới; đa tiến trình không cần |
| **Tạo nhanh hơn**, context switch có thể rẻ hơn | **Một bug là chết cả process** — con trỏ hỏng ở thread này phá dữ liệu mọi thread khác. Process **cô lập** tốt hơn |
| | **Chia nhau address space hữu hạn** — mỗi stack thread ăn một phần. Nhiều thread hoặc thread cần nhiều RAM là chạm trần (x86-32: ~3 GB). Process riêng thì mỗi cái có full không gian ảo |
| | **Signal trong đa luồng cần thiết kế cẩn thận** |
| | **Mọi thread phải chạy CÙNG một chương trình**; các process có thể chạy chương trình khác nhau |

> **Câu chốt:** chọn **thread** khi cần chia sẻ dữ liệu nhiều và tạo/huỷ thường xuyên; chọn **process** khi cần **cách ly lỗi** (một thành phần sập không kéo cả hệ thống) hoặc chạy chương trình khác nhau. 🆕 Đây đúng là lý do Chrome tách mỗi tab thành **process** chứ không phải thread — cách ly lỗi và bảo mật quan trọng hơn chi phí tạo.

---

## 5. Ít quan trọng — 1 dòng + tham chiếu

- **§29.2 — Kiểu dữ liệu Pthreads (Table 29-1, tr. 620)**: `pthread_t`, `pthread_mutex_t`… là kiểu **mờ (opaque)** — không được so sánh bằng `==`, dùng `pthread_equal()`.
- **§29.4 — `pthread_self()`, `pthread_equal()`** (tr. 622).
- **§29.5 — `pthread_exit()` vs `return` vs `exit()`** (tr. 623): ⚠️ **`exit()` trong bất kỳ thread nào cũng kết thúc CẢ process**; `pthread_exit()` chỉ kết thúc thread gọi nó. Thread chính `pthread_exit()` thì process sống tiếp tới khi thread cuối kết thúc.
- **§30.1.7 — Loại mutex, `pthread_mutexattr_settype()`** (tr. 649).
- **§30.2.3–30.2.4 — `pthread_cond_timedwait()`, mẫu producer-consumer đầy đủ** (tr. 645).
- **§31.2–31.3 — Danh sách hàm không thread-safe (Table 31-1, tr. 656)** và họ `_r`.
- **§32.4–32.6 — Cancellation point (Table 32-1), cleanup handler chi tiết** (tr. 675).
- **§33.1 — Thread và stack size**: `pthread_attr_setstacksize()` — mặc định 8 MB (ảo) trên Linux/x86-32; giảm khi cần **rất nhiều** thread.
- **§33.3 — NPTL vs LinuxThreads** (tr. 693): cài đặt cũ (LinuxThreads) không tuân chuẩn ở nhiều điểm; **NPTL** là bản hiện hành từ glibc 2.3.2. Lịch sử — không cần nhớ chi tiết.
- **§33.4 — Tương tác giữa thread và các API khác** (tr. 697).

---

## 6. Góc interview 🎯

<details><summary>1) Thread và process khác nhau thế nào? Cái gì chung, cái gì riêng?</summary>

**Process** là một đơn vị có **không gian địa chỉ riêng**; **thread** là luồng thực thi **bên trong** một process, và các thread **chia sẻ không gian địa chỉ đó**.

| Chung cho mọi thread | Riêng từng thread |
|---|---|
| **Biến toàn cục & static**, **heap** | **Stack** (biến cục bộ, tham số, địa chỉ trả về) |
| **Bảng file descriptor** | **Thread ID** |
| **Signal disposition** (handler) | **Signal mask** |
| PID/PPID, thư mục làm việc, UID/GID, umask | **`errno`**, TSD/TLS, trạng thái cancellation |
| Giới hạn tài nguyên, timer | Ưu tiên & chính sách lập lịch |

**Hai chỗ bất đối xứng hay bị hỏi bẫy:**
- **`errno` là per-thread** — nếu toàn cục thật thì thread A gọi syscall lỗi sẽ ghi đè `errno` của B. glibc cài nó thành macro trỏ vào vùng riêng của thread.
- **Signal handler dùng chung cả process, signal mask thì riêng từng thread** — nên không cài được handler khác nhau cho từng thread, nhưng chặn/mở signal theo từng thread thì được (nền của mẫu "thread chuyên nhận signal").

**Đánh đổi khi chọn:**
- Thread: chia sẻ dữ liệu **dễ**, tạo **nhanh** hơn, context switch rẻ hơn.
- Process: **cách ly lỗi** — một con trỏ hỏng ở thread này phá dữ liệu **mọi** thread khác và giết cả process; process thì độc lập. Ngoài ra process chạy được **chương trình khác nhau**, và mỗi process có **full không gian ảo** (nhiều thread thì chia nhau, mỗi stack ăn một phần).

Đó là lý do Chrome tách mỗi tab thành **process** chứ không phải thread: cách ly lỗi và bảo mật đáng giá hơn chi phí tạo.
</details>

<details><summary>2) Thread-safe và reentrant khác nhau ra sao?</summary>

Hai câu hỏi **khác nhau**, và đây là chỗ hầu hết người trả lời gộp làm một:

- **Thread-safe:** nhiều **thread** gọi **đồng thời** có an toàn không?
- **Reentrant:** hàm bị **cắt ngang giữa chừng** rồi được gọi lại (từ signal handler, hoặc đệ quy) có an toàn không?

**Khác biệt then chốt: cách đạt được.**
- Thread-safe **có thể dùng khoá** — bọc `pthread_mutex_lock` là xong.
- Reentrant **KHÔNG được dùng khoá** — vì hàm bị cắt ngang khi đang giữ khoá, rồi gọi lại chính nó, sẽ **tự deadlock**. Reentrant phải đạt bằng cách **không có trạng thái chia sẻ**.

⇒ **Reentrant ⇒ thread-safe, nhưng thread-safe ⇏ reentrant.** Một hàm bọc mutex là thread-safe nhưng **không** reentrant — gọi từ signal handler khi luồng chính đang giữ chính khoá đó là deadlock.

Đây chính là lý do danh sách **async-signal-safe hẹp hơn nhiều** so với danh sách thread-safe: `printf` là thread-safe (glibc khoá stdio) nhưng **không** async-signal-safe.

**Nguyên nhân điển hình khiến hàm không thread-safe** (TLPI): dùng **biến toàn cục/static**. Ví dụ của sách: `loc = glob; loc++; glob = loc;` — read-modify-write trên biến toàn cục, kết quả không đoán trước.

**Ba mức chữa, từ thô tới tinh:**
1. Mutex bọc cả hàm — đơn giản, nhưng *serialize* hoàn toàn, mất hết song song.
2. Mutex gắn với **biến chia sẻ**, chỉ khoá đúng vùng tới hạn — chạy song song trừ lúc đụng biến chung.
3. **Bỏ hẳn trạng thái chia sẻ** → thành reentrant. Đây là lý do có họ `_r`: `strtok_r`, `localtime_r`, `getpwnam_r` — thay vì trả con trỏ tới bộ nhớ tĩnh, chúng nhận **buffer do caller cấp**.
</details>

<details><summary>3) Vì sao <code>pthread_cond_wait()</code> phải nằm trong vòng <code>while</code>, không phải <code>if</code>?</summary>

Vì có **ba** tình huống khiến `pthread_cond_wait()` trả về mà **điều kiện vẫn sai**:

1. **Spurious wakeup** — POSIX cho phép nó trả về mà **không ai signal cả**. Đây là nhượng bộ có chủ đích để cài đặt hiệu quả hơn trên một số nền tảng.
2. **Stolen wakeup** — được signal thật, nhưng một thread khác dậy trước và **lấy mất** tài nguyên; đến lượt mình thì đã hết.
3. **`pthread_cond_broadcast()`** đánh thức **mọi** thread đang chờ, trong khi có thể chỉ đủ tài nguyên cho một.

Với `if`, cả ba ca đều dẫn tới **chạy tiếp khi điều kiện SAI** — xử lý dữ liệu không tồn tại, `avail--` xuống âm. Với `while`, thread chỉ **quay lại ngủ** — vô hại.

```c
pthread_mutex_lock(&mtx);
while (avail == 0)                  /* ✅ while, không phải if */
    pthread_cond_wait(&cond, &mtx);
avail--;
pthread_mutex_unlock(&mtx);
```

**Vì sao `wait` cần cả mutex — và tính nguyên tử là mấu chốt.** `pthread_cond_wait()` làm ba việc: ① **nhả mutex**, ② **đi ngủ** — hai việc này **nguyên tử với nhau** — và ③ khi dậy thì **khoá lại mutex** trước khi trả về.

Nếu ① và ② không nguyên tử thì có khe hở: consumer nhả mutex xong, producer chen vào `signal` **trước khi** consumer kịp ngủ → tín hiệu bay mất → consumer ngủ vĩnh viễn (**lost wakeup**). Đây cùng một mẫu race với `sigprocmask`+`pause` (chữa bằng `sigsuspend`).

**`signal` hay `broadcast`?** `signal` khi mọi thread chờ điều kiện tương đương và một sự kiện chỉ phục vụ được một (dùng `broadcast` ở đây gây **thundering herd**). `broadcast` khi các thread chờ **điều kiện khác nhau**, hoặc một sự kiện làm nhiều thread chạy tiếp được.

🆕 Trong C++, `std::condition_variable::wait(lk, predicate)` chính là vòng `while` này viết gọn — dùng dạng có predicate thì không thể quên.
</details>

<details><summary>4) Deadlock xảy ra thế nào với mutex? Phòng bằng cách nào?</summary>

**Kịch bản kinh điển — khoá ngược thứ tự:**
```
   Thread A               Thread B
   lock(mtx1) ✅          lock(mtx2) ✅
   lock(mtx2) ⏳ chờ B    lock(mtx1) ⏳ chờ A     → vòng tròn chờ, treo vĩnh viễn
```

**Bốn điều kiện Coffman phải thoả ĐỒNG THỜI:** mutual exclusion · hold and wait · no preemption · circular wait. Phá **một** là đủ.

**Cách chuẩn — lock ordering:** mọi thread khoá theo **cùng một thứ tự toàn cục** (vd sắp theo địa chỉ mutex). Phá "circular wait". Đây là cách TLPI khuyến nghị và cũng là cách thực tế nhất vì nó là **quy ước tĩnh, kiểm tra được khi review code**.

**Các cách khác:**
- **`pthread_mutex_trylock()`** — lấy được cái thứ hai thì tiếp; không thì **nhả cái đầu** rồi thử lại. Phá "hold and wait". ⚠️ Coi chừng **livelock** — hai thread cứ nhả rồi thử lại đồng bộ với nhau; thêm backoff ngẫu nhiên.
- **Khoá có timeout** — `pthread_mutex_timedlock()`. Phá "no preemption".
- **Giảm phạm vi khoá** — giữ khoá càng ngắn càng tốt, không gọi hàm lạ khi đang giữ khoá (hàm đó có thể khoá thứ khác).

🆕 **Trong C++17 có sẵn lời giải:** `std::scoped_lock lk(m1, m2);` khoá nhiều mutex **một lượt** bằng thuật toán tránh deadlock — không cần tự lo thứ tự. Đây là câu trả lời hiện đại đáng nêu, cùng với `std::lock_guard`/`unique_lock` cho RAII.

**Bẫy phụ:** mutex mặc định là `PTHREAD_MUTEX_NORMAL` — **tự khoá lại chính nó là deadlock**. Khi debug, đổi sang `PTHREAD_MUTEX_ERRORCHECK` để nó trả lỗi thay vì treo, dễ tìm ra chỗ sai.
</details>

<details><summary>5) Gọi <code>fork()</code> trong chương trình đa luồng — chuyện gì xảy ra?</summary>

**Chỉ thread gọi `fork()` tồn tại trong process con.** Mọi thread khác **biến mất** — nhưng **trạng thái chúng để lại thì vẫn còn nguyên**, và đó chính là vấn đề.

```
   TRƯỚC fork:  Thread A đang giữ mutex M, đang sửa dở một cấu trúc dữ liệu
                Thread B gọi fork()
                        ▼
   CON:         chỉ còn Thread B
                Mutex M vẫn ĐANG BỊ KHOÁ — bởi một thread KHÔNG CÒN TỒN TẠI
                → không ai nhả được → thread nào khoá M sẽ treo VĨNH VIỄN
                → cấu trúc dữ liệu đang sửa dở → HỎNG
```

**Hệ quả nghiêm trọng nhất:** trong con, giữa `fork()` và `exec()`, bạn **chỉ được gọi hàm async-signal-safe**. Đặc biệt **`malloc()` có thể deadlock** — nó dùng mutex nội bộ mà một thread khác có thể đang giữ đúng lúc `fork()`. Điều này bất ngờ với nhiều người vì `malloc` trông vô hại.

**Ba cách xử lý, theo thứ tự nên ưu tiên:**
1. **`exec()` ngay sau `fork()`** — `exec` thay toàn bộ không gian nhớ nên mọi trạng thái hỏng biến mất. Đây là cách đơn giản và đáng tin nhất.
2. **`fork()` trước khi tạo thread nào** — mẫu phổ biến: process khởi động, `fork` hết các worker cần thiết, *rồi mới* mỗi worker tạo thread của mình.
3. **`pthread_atfork()`** — đăng ký handler chạy trước/sau `fork` để khoá và nhả mutex cho đúng. Đúng về lý thuyết nhưng **rất khó làm cho đủ** trong thực tế, nhất là với mutex nằm trong thư viện bên thứ ba.

**Liên quan:** cũng vì lý do này, con sau `fork()` nên gọi **`_exit()`** thay vì `exit()` — `exit()` chạy handler `atexit()` và xả buffer stdio (bản sao của cha), gây in trùng và phá tài nguyên chung.

**Câu chốt:** *"`fork()` trong chương trình đa luồng chỉ an toàn khi con `exec()` ngay."*
</details>

<details><summary>6) Xử lý signal trong chương trình đa luồng nên thiết kế thế nào?</summary>

TLPI nói thẳng: *"it is usually desirable to avoid the use of signals in multithreaded programs."*

**Vì sao khó:** signal đặt ra hai bất đối xứng —
- **Signal disposition (handler) dùng chung cả process**, không cài riêng cho từng thread được.
- **Signal mask thì riêng từng thread.**
- Signal gửi cho process sẽ được giao cho **một thread bất kỳ** không đang chặn nó — không đoán trước được là thread nào.

Cộng thêm ràng buộc **async-signal-safe** vốn đã ngặt, viết handler đúng trong môi trường đa luồng là rất khó.

**Mẫu chuẩn — một thread chuyên nhận signal, dùng `sigwait()`:**

```c
/* ① Trong thread CHÍNH, TRƯỚC khi tạo bất kỳ thread nào */
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
sigaddset(&set, SIGTERM);
pthread_sigmask(SIG_BLOCK, &set, NULL);   /* mask được KẾ THỪA bởi mọi thread
                                             tạo sau → không thread nào nhận signal */

/* ② Thread chuyên trách chờ ĐỒNG BỘ */
static void *signalThread(void *arg) {
    int sig;
    for (;;) {
        sigwait(&set, &sig);              /* lấy signal về như dữ liệu thường */
        /* ✅ đây KHÔNG phải signal handler → gọi được hàm BẤT KỲ,
           kể cả printf, malloc, hay hàm dọn dẹp phức tạp */
        shutdown_gracefully();
    }
}
```

**Vì sao mẫu này thắng:** nó **biến signal bất đồng bộ thành sự kiện đồng bộ**. Không có handler → không còn ràng buộc async-signal-safe → không còn câu hỏi "thread nào nhận".

Hai biến thể cùng ý tưởng: **`signalfd()`** (biến signal thành fd, đưa thẳng vào `epoll` cùng mọi fd khác — hợp nhất với event loop) và **self-pipe trick** (cách cổ điển, khả chuyển).

**Chi tiết cần nhớ:** dùng **`pthread_sigmask()`**, không phải `sigprocmask()` — trong chương trình đa luồng hành vi của `sigprocmask()` là không xác định.
</details>

---

## 7. Đọc thêm (tùy chọn)

- Topic repo: [03/process-thread.md](../../03-operating-system/process-thread.md), [03/sync-primitives.md](../../03-operating-system/sync-primitives.md), [02/concurrency.md](../../02-modern-cpp/concurrency.md).
- Bank: `OS-003` (deadlock), `OS-007` (mutex vs semaphore), `OS-012` (predicate loop), `CPP-018/019/022/024` trong [os.md](../../14-prep/mock-interview/bank/os.md) · [cpp.md](../../14-prep/mock-interview/bank/cpp.md).
- **Góc C++ của cùng chủ đề:** [C++ Concurrency in Action](../cpp-concurrency/) — `std::thread`, `std::scoped_lock`, memory model. TLPI dạy **tầng POSIX**; cuốn kia dạy **tầng C++ dựng trên nó**.
- Nối với signal: [cụm 03 §4.3](03-signals-and-timers.md) — `signalfd` và mẫu chờ signal đồng bộ.
- Góc lý thuyết: [OSTEP concurrency](../ostep/concurrency.md) — race condition, lock, condition variable từ đầu.

---
⬅️ [Cụm 03 — Signals](03-signals-and-timers.md) · [README cụm TLPI](README.md) · ➡️ [Cụm 05 — Alternative I/O models](05-alternative-io-models.md)
