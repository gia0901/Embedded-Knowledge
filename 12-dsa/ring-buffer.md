# Ring Buffer (Circular Buffer) — từ cơ bản đến lock-free

> **TL;DR**
> - Ring buffer = mảng **kích thước cố định** + hai chỉ số `head`/`tail` chạy vòng. Push/pop **O(1) worst-case**, **không cấp phát động**, bộ nhớ biết trước.
> - Bài toán nó giải: **producer nhanh — consumer chậm**, và ta **không được phép** để bộ nhớ phình vô hạn.
> - Rắc rối kinh điển: `head == tail` vừa là **rỗng** vừa là **đầy** → 3 cách xử lý (hy sinh 1 ô / biến `count` / chỉ số chạy tự do + mask).
> - Đầy thì làm gì là **quyết định thiết kế**, không phải chi tiết cài đặt: **từ chối** (backpressure) hay **đè cái cũ nhất** (drop-oldest).
> - Có **5 tầng** dùng: 1 luồng → chính sách đầy → luồng byte → mutex+condvar → lock-free SPSC. Lên tầng cao chỉ khi tầng dưới không đủ.
> - Đây là cấu trúc **Linux nhất**: `dmesg` (kernel ring buffer), `pipe()`, `kfifo`, ALSA, `perf`, `io_uring` đều là ring buffer.

---

## 1. Vì sao cần nó — và vì sao `std::queue` không đủ

Tình huống chuẩn: dữ liệu **chảy vào theo thời gian thực** (UART, socket, sensor, sự kiện), một bên xử lý chậm hơn bên kia lúc cao điểm.

```
   producer  ──►  ???  ──►  consumer
   1000 msg/s              800 msg/s xử lý được
```

Dùng `std::queue`/`std::deque` (không giới hạn):

```
   t=0s   [ 200 phần tử ]
   t=10s  [ 2 000 ]
   t=60s  [ 12 000 ]  ← RAM phình, cấp phát liên tục, rồi OOM
```

Trên máy chủ thì "chậm dần rồi chết". Trên thiết bị nhúng 128 MB RAM thì đó là **OOM killer bắn nhầm process khác**, và bug xảy ra lúc 3h sáng ngoài field.

Ring buffer đổi câu hỏi: thay vì *"chứa hết bằng mọi giá"*, nó hỏi *"chứa tối đa N, quá thì làm gì?"* — buộc bạn phải **quyết định** (§4), và quyết định đó luôn tốt hơn OOM.

| | Ring buffer | `std::queue`/`deque` | `std::list` |
|---|---|---|---|
| Bộ nhớ | **cố định, biết trước** | phình theo tải | phình + mỗi node một lần `new` |
| Cấp phát khi chạy | **không** | có (theo lô) | có (mỗi phần tử) |
| push/pop | **O(1) worst-case** | O(1) amortized | O(1) |
| Cache locality | **liền mạch, rất tốt** | tốt (theo block) | kém |
| Khi quá tải | **có chính sách rõ ràng** | phình tới chết | phình tới chết |
| Realtime / ISR / kernel | **dùng được** | ❌ (`new` không tất định, có thể ngủ) | ❌ |

> Đây chính là lý do kernel không dùng linked list cho đường dữ liệu nóng mà dùng `kfifo`: trong ngữ cảnh không được ngủ, không được cấp phát, **O(1) worst-case là bắt buộc**, không phải tối ưu.

---

## 2. Tầng 1 — Ring buffer một luồng

### 2.1 Ý tưởng: mảng thẳng nhưng chỉ số quay vòng

```
   Mảng thật (N = 8 ô), CHỈ SỐ quay vòng bằng % N

    idx:   0    1    2    3    4    5    6    7
         ┌────┬────┬────┬────┬────┬────┬────┬────┐
         │    │ C  │ D  │ E  │    │    │    │    │
         └────┴────┴────┴────┴────┴────┴────┴────┘
                ▲              ▲
              tail           head
          (đọc tiếp ở đây)  (ghi tiếp ở đây)

   push(F):  buf[head] = F;  head = (head+1) % 8
   pop():    out = buf[tail]; tail = (tail+1) % 8

   Sau vài lần, head "vượt qua mép" và quay về 0 — dữ liệu nằm VẮT qua biên:

         ┌────┬────┬────┬────┬────┬────┬────┬────┐
         │ J  │ K  │    │    │    │ G  │ H  │ I  │
         └────┴────┴────┴────┴────┴────┴────┴────┘
                ▲                   ▲
              head                tail
          ⇒ nội dung logic là:  G H I J K  (đọc từ tail, vòng qua 0)
```

**Chỉ có thế.** Mọi thứ còn lại trong tài liệu này là hệ quả của một câu hỏi duy nhất ở §2.2.

### 2.2 Rắc rối cốt lõi: `head == tail` nghĩa là gì?

```
   RỖNG                              ĐẦY (đã push đủ 8 lần)
   ┌──┬──┬──┬──┬──┬──┬──┬──┐         ┌──┬──┬──┬──┬──┬──┬──┬──┐
   │  │  │  │  │  │  │  │  │         │A │B │C │D │E │F │G │H │
   └──┴──┴──┴──┴──┴──┴──┴──┘         └──┴──┴──┴──┴──┴──┴──┴──┘
     ▲                                 ▲
   head=tail=0                       head=0 (đã vòng), tail=0

   ⇒ HAI trạng thái NGƯỢC HẲN NHAU cho ra CÙNG một điều kiện head==tail.
```

Đây là chỗ 90% bản cài đặt sai lần đầu. Ba cách thoát:

| Cách | Sức chứa | Ưu | Nhược |
|---|---|---|---|
| **Hy sinh 1 ô** — coi là đầy khi `(head+1)%N == tail` | N−1 | chỉ 2 biến; **lock-free được** | mất 1 ô (không đáng kể nếu N lớn) |
| **Thêm biến `count`** | N | dùng hết ô, dễ đọc nhất | `count` bị **cả hai phía ghi** → phải có khoá → **không lock-free SPSC được** |
| **Chỉ số chạy tự do + mask** — `head`/`tail` đếm *tổng số lần* push/pop, không bao giờ modulo | N | dùng hết ô **và** lock-free được; `size = head − tail` | N phải là **luỹ thừa của 2**; phải hiểu unsigned wraparound |

### 2.3 Bản A — hy sinh một ô (dễ nhất, viết được trong phỏng vấn)

```cpp
template <typename T, std::size_t N>
class RingBuffer {                       // N ô THẬT → chứa được N-1 phần tử
    static_assert(N >= 2, "cần ít nhất 2 ô");
    T           buf_[N];
    std::size_t head_ = 0;               // ô SẼ GHI tiếp theo
    std::size_t tail_ = 0;               // ô SẼ ĐỌC tiếp theo
    static std::size_t next(std::size_t i) { return (i + 1) % N; }
public:
    bool empty() const { return head_ == tail_; }
    bool full()  const { return next(head_) == tail_; }        // ← chừa 1 ô để phân biệt
    std::size_t size() const { return (head_ + N - tail_) % N; } // +N tránh trừ âm

    bool push(const T& v) {
        if (full()) return false;                 // chính sách: TỪ CHỐI (xem §4)
        buf_[head_] = v;
        head_ = next(head_);
        return true;
    }
    bool pop(T& out) {
        if (empty()) return false;
        out   = std::move(buf_[tail_]);
        tail_ = next(tail_);
        return true;
    }
};
```

⚠️ `size()` viết `(head_ + N - tail_) % N` chứ **không** phải `(head_ - tail_) % N`: với `size_t` (unsigned), khi `head_ < tail_` phép trừ sẽ **wrap thành số cực lớn** rồi `% N` ra kết quả sai. Cộng `N` trước là mẹo chuẩn.

### 2.4 Bản B — chỉ số chạy tự do + mask (bản dùng thật)

```cpp
template <typename T, std::size_t N>
class RingBuffer2 {
    static_assert(N && (N & (N - 1)) == 0, "N phải là luỹ thừa của 2");
    static constexpr std::size_t MASK = N - 1;
    T           buf_[N];
    std::size_t head_ = 0;   // ĐẾM TỔNG số lần push — KHÔNG modulo, cho tràn tự nhiên
    std::size_t tail_ = 0;   // ĐẾM TỔNG số lần pop
public:
    std::size_t size()  const { return head_ - tail_; }   // đúng CẢ KHI head_ đã tràn qua 0
    bool        empty() const { return head_ == tail_; }
    bool        full()  const { return size() == N; }     // dùng được HẾT N ô

    bool push(const T& v) {
        if (full()) return false;
        buf_[head_ & MASK] = v;    // & MASK thay cho % N  (1 lệnh AND, không có phép chia)
        ++head_;                   // chỉ số cứ tăng, mặc kệ tràn
        return true;
    }
    bool pop(T& out) {
        if (empty()) return false;
        out = std::move(buf_[tail_ & MASK]);
        ++tail_;
        return true;
    }
};
```

**Vì sao `head_ - tail_` vẫn đúng khi `head_` tràn qua 0?** Số học unsigned trong C/C++ là modulo 2ⁿ. Nếu `head_` tràn về 0 còn `tail_` = 4 294 967 290 thì `head_ - tail_` cho **đúng hiệu thật** (6) chứ không ra số âm. Điều kiện để cách này đúng: **2ⁿ phải chia hết cho N** — luôn đúng khi N là luỹ thừa của 2. Đó là lý do thật sự của `static_assert`, không phải chỉ để dùng `& MASK`.

**Vì sao ép N là luỹ thừa của 2?** `% N` trên số không phải hằng luỹ thừa 2 biên dịch ra **lệnh chia** (~20–40 chu kỳ trên Cortex-M/A). `& (N-1)` là **1 chu kỳ**. Trên đường dữ liệu chạy triệu lần/giây, đây là khác biệt đo được — và là lý do `kfifo` trong kernel **bắt buộc** kích thước luỹ thừa 2.

---

## 3. Độ phức tạp & bộ nhớ

| Thao tác | Time | Ghi chú |
|---|---|---|
| `push` / `pop` | **O(1) worst-case** | không amortized — không có lần nào đột ngột O(n) |
| `size` / `empty` / `full` | O(1) | vài phép số học |
| truy cập phần tử thứ k | O(1) | `buf_[(tail_+k) & MASK]` |
| **tìm kiếm** | O(n) | ring buffer **không** để tra cứu — sai công cụ |
| Space | **Θ(N)**, cấp **một lần** | 0 byte cấp phát khi chạy |

> Điểm phải nói ra khi phỏng vấn: `vector::push_back` là O(1) **amortized** — có những lần lẻ tốn O(n) để realloc + copy. Ring buffer là O(1) **worst-case**, không có lần lẻ nào. Với hệ realtime, khác biệt này quan trọng hơn con số trung bình: cái giết deadline là **lần chậm nhất**, không phải lần trung bình ([08/rtos-vs-linux](../08-embedded-systems/rtos-vs-linux.md)).

---

## 4. Tầng 2 — Đầy thì làm gì? (quyết định thiết kế, không phải chi tiết cài đặt)

Hai chính sách, **chọn theo bản chất dữ liệu**:

```
   ① TỪ CHỐI (reject / backpressure)        ② ĐÈ CÁI CŨ NHẤT (overwrite / drop-oldest)
   push() trả về false                      push() luôn thành công, tail bị đẩy theo

   ┌──┬──┬──┬──┐   push(E)                  ┌──┬──┬──┬──┐   push(E)
   │A │B │C │D │  ──────►   false           │A │B │C │D │  ──────►  ┌──┬──┬──┬──┐
   └──┴──┴──┴──┘   (A B C D còn nguyên)     └──┴──┴──┴──┘           │E │B │C │D │
                                                                    └──┴──┴──┴──┘
                                                                    A BỊ MẤT, giữ mới nhất
```

```cpp
// Chính sách ②: đè cái cũ nhất — CHỈ dùng khi dữ liệu mới có giá trị hơn dữ liệu cũ
void push_overwrite(const T& v) {
    if (full()) ++tail_;          // ← vứt phần tử CŨ NHẤT để lấy chỗ
    buf_[head_ & MASK] = v;
    ++head_;
}
```

| | Từ chối (backpressure) | Đè cũ nhất |
|---|---|---|
| Dùng cho | **lệnh, giao dịch, gói tin** — mất là sai | **đo lường, log, telemetry, khung hình** — mới quan trọng hơn cũ |
| Producer phải | xử lý `false`: đợi / retry / báo lỗi / đếm drop | không phải làm gì |
| Rủi ro | producer bị chặn → dồn ngược lên trên | **mất dữ liệu âm thầm** nếu không đếm |
| Ví dụ thật | queue lệnh gửi thiết bị | `dmesg` (log cũ bị đẩy ra), buffer nhiệt độ mới nhất |

> ⚠️ Bẫy phổ biến nhất trong review code: chọn "đè cũ nhất" **mà không đếm số lần đè**. Hệ trông vẫn chạy êm, và bạn không bao giờ biết mình đang mất dữ liệu. **Luôn có một `dropped_count`** và đưa nó ra ngoài (log/metric/sysfs). Cả `dmesg` lẫn `perf` đều làm điều này — `perf` in thẳng "*n events lost*".

---

## 5. Tầng 3 — Ring buffer cho luồng byte (UART, socket, file)

Ở tầng 1 ta push từng phần tử. Với **luồng byte**, làm vậy là phí: mỗi byte một lần gọi hàm + kiểm tra. Bản dùng thật ghi/đọc **cả khối**, và phải xử lý chuyện khối bị **vắt qua biên**:

```
   Ghi 5 byte vào ring N=8, head=6:

         0    1    2    3    4    5    6    7
       ┌────┬────┬────┬────┬────┬────┬────┬────┐
       │ ③  │ ④  │ ⑤  │    │    │    │ ①  │ ②  │
       └────┴────┴────┴────┴────┴────┴────┴────┘
                                    ▲
                                  head=6
       ⇒ KHÔNG memcpy được 1 phát. Phải chia LÀM HAI:
         phần 1: 2 byte vào [6,8)   phần 2: 3 byte vào [0,3)
```

```c
// Bản C thuần — dạng hay gặp trong driver/HAL. Trả về số byte ĐÃ ghi được.
size_t rb_write(rb_t *rb, const uint8_t *src, size_t n) {
    size_t space = RB_SIZE - (rb->head - rb->tail);   // chỗ trống còn lại
    if (n > space) n = space;                          // ghi được bao nhiêu thì ghi

    size_t off   = rb->head & RB_MASK;                 // vị trí thật trong mảng
    size_t chunk = RB_SIZE - off;                      // tối đa còn được ghi liền tới mép
    if (chunk > n) chunk = n;

    memcpy(rb->buf + off, src, chunk);                 // phần 1: từ off tới mép
    memcpy(rb->buf, src + chunk, n - chunk);           // phần 2: phần vắt về đầu (n-chunk có thể = 0)
    rb->head += n;                                     // CÔNG BỐ sau khi dữ liệu đã nằm yên
    return n;
}
```

Ba điểm khiến bản này khác bản "for từng byte":

1. **Trả về số byte ghi được**, không phải bool — đúng ngữ nghĩa `write()`/`send()` của Linux, và cho phép caller ghi phần còn lại sau.
2. **`memcpy` thứ hai không cần `if`** — khi không vắt biên thì `n - chunk == 0`, `memcpy(_, _, 0)` hợp lệ và là no-op. Bỏ được một nhánh trong hot path.
3. `memcpy` khối tận dụng lệnh chép nhiều byte/lần + prefetch → nhanh hơn vòng lặp từng byte **hàng chục lần** cho khối lớn.

> Đây đúng là hình dạng bên trong `kfifo` của kernel (`kfifo_in`/`kfifo_out`) và bên trong buffer TX/RX của mọi driver UART.

---

## 6. Tầng 4 — Nhiều luồng: mutex + condition variable

Từ đây trở đi là **đồng thời**. Tầng này là câu trả lời mặc định đúng cho 90% trường hợp trên Linux userspace — và cũng là **thứ nên viết trước** trong phỏng vấn, trước khi ai đó hỏi tới lock-free.

```cpp
template <typename T>
class BoundedQueue {
    std::vector<T>          buf_;
    std::size_t             head_ = 0, tail_ = 0, count_ = 0;
    std::mutex              m_;
    std::condition_variable not_full_, not_empty_;   // HAI cv — xem ghi chú dưới
    bool                    closed_ = false;
public:
    explicit BoundedQueue(std::size_t cap) : buf_(cap) {}

    bool push(T v) {                                  // trả false nếu hàng đã đóng
        std::unique_lock<std::mutex> lk(m_);
        not_full_.wait(lk, [this]{ return count_ < buf_.size() || closed_; });
        if (closed_) return false;
        buf_[head_] = std::move(v);
        head_ = (head_ + 1) % buf_.size();
        ++count_;
        lk.unlock();                                  // mở khoá TRƯỚC khi notify
        not_empty_.notify_one();
        return true;
    }

    bool pop(T& out) {                                // trả false khi đã đóng VÀ rỗng
        std::unique_lock<std::mutex> lk(m_);
        not_empty_.wait(lk, [this]{ return count_ > 0 || closed_; });
        if (count_ == 0) return false;                // đóng + rỗng ⇒ hết hàng thật
        out   = std::move(buf_[tail_]);
        tail_ = (tail_ + 1) % buf_.size();
        --count_;
        lk.unlock();
        not_full_.notify_one();
        return true;
    }

    void close() {                                    // đánh thức MỌI người đang chờ để thoát
        { std::lock_guard<std::mutex> lk(m_); closed_ = true; }
        not_empty_.notify_all();
        not_full_.notify_all();
    }
};
```

Bốn chi tiết quyết định đúng/sai, và là bốn thứ interviewer sẽ chọc:

| Chi tiết | Vì sao bắt buộc |
|---|---|
| `wait(lk, predicate)` — **không** dùng `if` | chống **spurious wakeup** và **stolen wakeup**; dạng có predicate là vòng `while` viết gọn ([OS-012](../14-prep/mock-interview/bank/os.md)) |
| **Hai** condition variable | một cv + `notify_all` vẫn *đúng* nhưng đánh thức nhầm phía (producer dậy vì consumer vừa lấy được hàng) → **thundering herd**, tốn CPU vô ích |
| `unlock()` trước `notify_one()` | notify khi còn giữ khoá → luồng vừa dậy chạm ngay mutex đang bị giữ → ngủ lại (hurry-up-and-wait). Không sai, nhưng phí một vòng switch |
| `close()` + cờ `closed_` | thiếu nó thì **shutdown treo vĩnh viễn**: consumer ngồi trong `wait()` chờ dữ liệu không bao giờ tới. Đây là bug thật hay gặp nhất ở tầng này |

⚠️ `count_` ở đây bị **cả hai phía ghi**, nên đúng như bảng §2.2: bản này **không** nâng lên lock-free được. Muốn lock-free phải bỏ `count_`, quay về chỉ số chạy tự do — đó là §7.

---

## 7. Tầng 5 — Lock-free SPSC (Single Producer, Single Consumer)

**SPSC = đúng MỘT luồng ghi, đúng MỘT luồng đọc.** Không phải "ít luồng", mà là **chính xác một mỗi phía**. Ràng buộc này chính là thứ mua được cả tính lock-free.

### 7.1 Vì sao ràng buộc đó lại đủ

```
   ┌──────────────┐                                  ┌──────────────┐
   │  PRODUCER    │                                  │  CONSUMER    │
   │  (1 luồng)   │                                  │  (1 luồng)   │
   ├──────────────┤                                  ├──────────────┤
   │ GHI  head    │ ───────► head ─────────────────► │ ĐỌC  head    │
   │ ĐỌC  tail    │ ◄─────────────── tail ◄───────── │ GHI  tail    │
   └──────────────┘                                  └──────────────┘

   ⇒ MỖI biến chỉ có ĐÚNG MỘT người ghi.
   ⇒ Không có read-modify-write tranh chấp ⇒ KHÔNG CẦN khoá, KHÔNG CẦN CAS.
     Chỉ cần load/store atomic đúng memory order.
```

Đối chiếu: nếu có **hai** producer, cả hai cùng làm `head++` — đó là read-modify-write trên biến chung, đúng dạng race của `counter++`. Lúc đó bắt buộc phải có mutex hoặc CAS loop (MPMC), và bản dưới đây **sai ngay**.

### 7.2 Cài đặt

```cpp
template <typename T, std::size_t N>
class SpscRing {
    static_assert(N && (N & (N - 1)) == 0, "N phải là luỹ thừa của 2");
    static constexpr std::size_t MASK = N - 1;

    alignas(64) std::atomic<std::size_t> head_{0};   // CHỈ producer ghi
    alignas(64) std::atomic<std::size_t> tail_{0};   // CHỈ consumer ghi
    alignas(64) T                        buf_[N];
public:
    bool push(const T& v) {
        const auto h = head_.load(std::memory_order_relaxed);   // biến của MÌNH → relaxed là đủ
        const auto t = tail_.load(std::memory_order_acquire);   // biến của NGƯỜI KIA → acquire
        if (h - t == N) return false;                           // đầy
        buf_[h & MASK] = v;                        // ① ghi DỮ LIỆU trước
        head_.store(h + 1, std::memory_order_release);  // ② rồi mới CÔNG BỐ chỉ số
        return true;
    }

    bool pop(T& out) {
        const auto t = tail_.load(std::memory_order_relaxed);   // của mình
        const auto h = head_.load(std::memory_order_acquire);   // ② cặp với release ở push
        if (h == t) return false;                               // rỗng
        out = std::move(buf_[t & MASK]);           // ① thấy ĐÚNG dữ liệu nhờ acquire ở trên
        tail_.store(t + 1, std::memory_order_release);  // báo producer: ô này đã trống
        return true;
    }
};
```

### 7.3 Có **hai** cặp release/acquire, mỗi cặp công bố một thứ

```
   Cặp 1 — công bố DỮ LIỆU:
     producer:  ghi buf_[h]  ──►  head_.store(release)
                                        ╲
                                         ╲ synchronizes-with
                                          ▼
     consumer:              head_.load(acquire)  ──►  đọc buf_[t]  ✅ thấy dữ liệu đúng

   Cặp 2 — công bố CHỖ TRỐNG:
     consumer:  đọc xong buf_[t]  ──►  tail_.store(release)
                                             ╲
                                              ▼
     producer:                 tail_.load(acquire)  ──►  ghi đè ô đó  ✅ không đè khi chưa đọc xong
```

Thiếu cặp 1 → consumer thấy `head` mới nhưng đọc phải **dữ liệu cũ/rác**. Thiếu cặp 2 → producer đè lên ô mà consumer **đang đọc dở**. Cả hai đều là bug im lặng, và **cả hai đều KHÔNG tái hiện trên x86** (memory model x86 mạnh, gần như cho không acquire/release) nhưng **hỏng trên ARM**. Đây là lý do rất thực tế để viết đúng ngay từ đầu thay vì "test thấy chạy là được" — máy dev x86, thiết bị Datalogic ARM.

Chi tiết còn lại:

- **`alignas(64)`** — không có nó, `head_` và `tail_` rơi cùng một cache line. Mỗi lần producer ghi `head_`, cache line bị invalidate ở core của consumer dù nó **không hề dùng** `head_`. Đó là **false sharing**: đúng về logic, chậm 2–10 lần. 64 byte là kích thước cache line phổ biến (x86-64 và Cortex-A).
- **`h - t == N` là đầy** — dùng hết N ô (chỉ số chạy tự do, §2.4), không phải hy sinh 1 ô.
- **Không có bài toán ABA** ở đây, vì không có CAS. Chỉ SPSC mới đơn giản được như vậy; MPMC phức tạp hơn một bậc.
- **Tối ưu batching** (bản production): cache lại chỉ số của phía kia vào biến thường, chỉ `load(acquire)` lại **khi tưởng là đầy/rỗng**. Giảm hẳn lưu lượng cache coherency.

### 7.4 Mutex hay lock-free?

| | Mutex + condvar (§6) | Lock-free SPSC (§7) |
|---|---|---|
| Số luồng | **bất kỳ** (MPMC) | **đúng 1 + 1** |
| Consumer rảnh | **ngủ**, 0% CPU | phải **poll** (hoặc thêm eventfd/futex để báo) |
| Latency | có syscall khi tranh chấp (futex) | **không syscall**, đường nhanh vài chục ns |
| Dùng trong ISR / ngữ cảnh không được ngủ | ❌ | ✅ |
| Rủi ro viết sai | thấp | **cao** — sai memory order không lộ trên x86 |
| Debug | dễ (`gdb`, TSan hiểu mutex) | khó; TSan hiểu atomic nhưng bug chỉ hiện trên ARM/tải cao |

**Chọn lock-free chỉ khi có một trong ba lý do:** (1) một đầu là **ISR/ngữ cảnh không được ngủ**; (2) đã **đo** và mutex đúng là điểm nghẽn; (3) yêu cầu latency **tail** chặt (đo bằng p99, không phải trung bình).

> Câu nên nói trong phỏng vấn: *"Mặc định tôi viết mutex + condvar vì đúng và dễ bảo trì. Tôi chỉ chuyển sang lock-free SPSC khi có ràng buộc thật — ví dụ đầu producer là ISR, không được phép ngủ — hoặc khi profiler chỉ ra contention."* Đây là câu trả lời của người từng bảo trì code, không phải người vừa đọc bài blog lock-free.

---

## 8. Ring buffer trong Linux thật

Điểm mạnh nhất khi trả lời phỏng vấn Embedded Linux: **gọi tên chỗ nó đang chạy**, không chỉ cài đặt được nó.

| Nơi | Vai trò | Chi tiết đáng nhớ |
|---|---|---|
| **`printk` / `dmesg`** | log kernel | Ring buffer trong kernel, **đè cái cũ nhất** khi đầy → đó là lý do log khởi động biến mất nếu bạn xem quá muộn (`dmesg -w` để bắt realtime) |
| **`pipe()` / FIFO** | IPC luồng byte | Kernel giữ một vòng các trang; đầy → `write()` **block** (backpressure §4 ①); reader chết → `SIGPIPE` |
| **`kfifo`** (kernel) | FIFO chuẩn trong driver | **Bắt buộc luỹ thừa 2**; `kfifo_in`/`kfifo_out`; **lockless khi 1 producer + 1 consumer** — chính là §7 |
| **Driver UART/tty** | buffer TX/RX | ISR đẩy byte vào ring, tầng trên lấy ra — mẫu ở [08/interrupts-bare-metal §3.1](../08-embedded-systems/interrupts-bare-metal.md) |
| **ALSA** (audio) | vòng DMA | **Phần cứng là producer** (DMA ghi thẳng RAM), app là consumer; `hw_ptr`/`appl_ptr` đúng là head/tail. Đuổi kịp nhau = **xrun** (underrun/overrun) |
| **`perf` / ftrace** | thu sự kiện | Ring buffer **mmap** chia sẻ kernel↔user, per-CPU (né tranh chấp); đầy thì báo "*events lost*" |
| **`io_uring`** | I/O bất đồng bộ | **Hai** ring mmap chung: SQ (user ghi request, kernel đọc) + CQ (kernel ghi kết quả, user đọc) → SPSC hai chiều, **né syscall mỗi thao tác** |
| **shm giữa 2 process** | zero-copy IPC | Ring đặt trong vùng `shm_open`+`mmap`; đồng bộ bằng semaphore process-shared hoặc atomic ([linux-sysprog](../14-prep/mock-interview/bank/linux-sysprog.md)) |

> Mẫu chung của toàn bảng: **hai bên chạy ở tốc độ khác nhau, không bên nào được chặn bên kia, bộ nhớ phải hữu hạn.** Nhận ra mẫu này là nhận ra chỗ cần ring buffer.

---

## 9. Chọn tầng nào

```
   Có nhiều hơn một luồng đụng vào không?
     │
     ├─ KHÔNG ──► Tầng 1 (§2). Xong. Đừng thêm atomic cho "chắc ăn".
     │
     └─ CÓ ──► Một đầu có phải ISR / ngữ cảnh KHÔNG ĐƯỢC NGỦ không?
                 │
                 ├─ CÓ ──► bắt buộc Tầng 5 lock-free SPSC (§7) — mutex không dùng được
                 │
                 └─ KHÔNG ──► Đúng 1 producer + 1 consumer, và ĐÃ ĐO thấy mutex là nghẽn?
                                │
                                ├─ CÓ ──► Tầng 5 (§7)
                                └─ KHÔNG ──► Tầng 4 mutex + condvar (§6)  ◄── MẶC ĐỊNH
```

Trực giao với lựa chọn trên: dữ liệu là **byte stream** → dùng dạng §5 (`memcpy` hai khối); là **bản ghi/message** → dạng theo phần tử (§2).

---

## 10. Bẫy — checklist review code

1. **`head == tail` mơ hồ** — không chọn một trong 3 cách ở §2.2 → hoặc mất 1 phần tử, hoặc đọc phải rác khi đầy.
2. **`(head - tail) % N` với unsigned** — wrap thành số khổng lồ. Dùng `(head + N - tail) % N`, hoặc chỉ số chạy tự do.
3. **Đè cái cũ nhất mà không đếm** — mất dữ liệu âm thầm. Luôn có `dropped_count` và đưa ra ngoài.
4. **`if` thay vì `while`/predicate với condvar** — spurious/stolen wakeup ([OS-012](../14-prep/mock-interview/bank/os.md)).
5. **Không có đường shutdown** — consumer treo trong `wait()` mãi mãi. Cần cờ `closed_` + `notify_all()`.
6. **Công bố chỉ số TRƯỚC khi ghi dữ liệu** — phía kia đọc phải rác. Luôn: ghi dữ liệu → *rồi mới* tăng chỉ số (release).
7. **Thiếu `alignas`** ở bản lock-free → false sharing, chậm vài lần mà vẫn "chạy đúng".
8. **Dùng bản SPSC với 2 producer** — sai ngay từ giả định, không phải bug cài đặt.
9. **`% N` với N không phải luỹ thừa 2** trong hot path — lệnh chia, và **phá luôn** tính đúng của chỉ số chạy tự do.
10. **Kích thước chọn bừa** — N phải xuất phát từ *burst lớn nhất × thời gian consumer bận nhất*, có ghi lại lý do. "Chọn 1024 cho chắc" là chỗ để bug ngồi chờ.

---

## 11. Trong phỏng vấn coding — viết theo thứ tự nào

Đề "cài ring buffer" gần như luôn có ~20 phút. Thứ tự an toàn:

1. **Hỏi trước khi code** (2 phút, ăn điểm nhiều nhất): *một luồng hay nhiều luồng? đầy thì từ chối hay đè? byte stream hay phần tử? kích thước cố định compile-time hay runtime?* — chỉ hỏi thế là đã hơn phần lớn ứng viên.
2. **Nói ra chỗ khó trước khi gặp**: *"Cạm bẫy chính là `head == tail` vừa là rỗng vừa là đầy; tôi sẽ hy sinh một ô để phân biệt."*
3. **Viết bản §2.3** — đơn giản, chạy đúng, dễ giải thích.
4. **Nêu complexity**: O(1) worst-case cả hai chiều, Θ(N) bộ nhớ, không cấp phát khi chạy.
5. **Test mồm ngay 3 ca biên**: rỗng → pop; đầy → push; push/pop xen kẽ để **vòng qua biên** (ca này lộ hầu hết bug).
6. **Chỉ khi còn thời gian / bị hỏi tới** mới nâng: chính sách đè cũ (§4) → mutex+condvar (§6) → lock-free SPSC (§7).

> Sai lầm hay gặp: nhảy thẳng vào lock-free để gây ấn tượng, rồi sa lầy memory order và không kịp xong bài. **Bản đúng và chạy được luôn thắng bản tham vọng dang dở.**

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [COD-006](../14-prep/mock-interview/bank/coding.md) | Ring buffer là gì và giải quyết vấn đề gì mà queue thường không giải quyết được? |
| [DSA-013](../14-prep/mock-interview/bank/dsa.md) | Làm sao phân biệt "đầy" với "rỗng" khi cả hai đều cho head == tail? |
| [COD-006](../14-prep/mock-interview/bank/coding.md) | Vì sao ring buffer thường ép kích thước là luỹ thừa của 2? |
| [DSA-014](../14-prep/mock-interview/bank/dsa.md) | SPSC lock-free hoạt động thế nào, và vì sao "một producer một consumer" lại đủ để bỏ khoá? |
| [DSA-014](../14-prep/mock-interview/bank/dsa.md) | Khi nào dùng lock-free, khi nào dùng mutex + condition variable? |
| [LNX-017](../14-prep/mock-interview/bank/linux-sysprog.md) | Buffer đầy thì nên làm gì? |

---
⬅️ [Về index topic](README.md) · Trước đó: [algorithm-patterns.md](algorithm-patterns.md)
