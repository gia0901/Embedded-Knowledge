# Phiên mock — 2026-08-13 · comprehensive · track cpp-system

- **Level:** mid-level · **Trần độ sâu:** **T2** · **Số câu:** 15/16 (dừng trước bài coding) · ~70 phút
- **Điểm trung bình:** **3.0 / 4**
- **Bối cảnh:** Buổi CN Tuần 1 — chốt Tuần 1, theo [datalogic-plan](../../study-plans/datalogic-plan.md). Chạy lại sau lần [tạm dừng 10/08](2026-08-10--comprehensive--cpp-system.md); **bộ câu hoàn toàn mới**, câu sổ yếu/retention đều đổi góc.
- **Câu 16 (coding `SocketHandle`) hoãn** — chưa làm, chưa chấm.

> **Cách dùng file này:** mỗi câu để **nguyên đề bài**, phần nhận xét + đáp án chuẩn **ẩn trong `<details>`** — tự trả lời lại trước khi mở. Cố ý lặp nội dung bank, vì đây là *lỗi của chính mình đặt cạnh đáp án chuẩn*.

## Kết quả tổng

| # | ID | Câu (tóm tắt) | Điểm | Kết quả |
|---|----|---------------|------|---------|
| 1 | OS-001 | process vs thread (daemon USB + log) | **4** | Đánh đổi độ tin cậy 24/7 tốt |
| 2 | CPP-030 | size/capacity + invalidation | **3** | (c) trắng, phải gợi ý |
| 3 | CPP-031 | `const` member + `mutable` | **3** | Thiếu *logical constness* |
| 4 | CPP-011 | move ctor `noexcept` | **3** | Thiếu *strong exception guarantee* |
| 5 | CPP-014 | rvalue vs universal reference | **4** | Chính xác cả điều kiện lẫn ca bẫy |
| 6 | CPP-016 | object slicing | **2** | 🔴 **VÀO SỔ** |
| 7 | CPP-049 | move không phải luôn rẻ | **3** | Tự mâu thuẫn, gỡ được khi bị chỉ ra |
| 8 | OS-020 | thread-safe vs reentrant | **2** | 🔴 **VÀO SỔ** — sai, không phải thiếu |
| 9 | CPP-017 | template vs virtual | **3** | Sai "19 bản không dùng" |
| 10 | SD-016 | bộ nhớ tất định | **2** | 🔴 **VÀO SỔ** |
| 11 | DP-011 | DIP / testability | **3** | Chưa trả lời "ai sở hữu interface" |
| 12 | **CPP-032** | explicit — góc *hai chiều ngầm* | **3** | 🔼 2→3, **vẫn trong sổ** (1/2) |
| 13 | **CPP-020** | Rule of 0 — góc *dtor vô hại* | **4** | ✅ retention đạt → dời Tuần 4 |
| 14 | **CPP-029** | emplace — góc *rò rỉ* | **3** | ✅ retention đạt → dời Tuần 4 |
| 15 | CPP-052 | chẩn đoán race `shared_ptr` | **3** | Đúng hướng, thiếu các bước |

**Sổ yếu: 2 → 5 câu.** **3 lỗ hổng ưu tiên:** ① async-signal-safety (câu 8) · ② thiết kế phòng ngừa ở class (câu 6) · ③ bộ nhớ tất định + `std::pmr` (câu 10).

---

# Câu hỏi & Feedback

---

### Câu 1 · OS-001 · 🟢 · **4/4**

Daemon quét mã vạch cần chạy 2 việc song song: đọc dữ liệu từ cổng USB, và ghi log lên thẻ nhớ.

**a)** Tách thành **2 thread** so với **2 process** khác nhau ở những gì? Nêu theo hướng *cái gì chia sẻ, cái gì không*.
**b)** `pthread_create()` rẻ hơn `fork()` — rẻ hơn ở **chỗ nào cụ thể**? Kernel phải làm gì ít hơn?
**c)** Bộ phận ghi log dính bug và **crash**. Hậu quả khác nhau thế nào giữa hai phương án? Điều đó ảnh hưởng gì tới lựa chọn cho thiết bị chạy 24/7?

**🔁 Follow-up:** `fork()` hiện đại dùng **copy-on-write** — không copy toàn bộ vùng nhớ. Vậy nếu bộ nhớ không bị copy, `fork()` còn đắt hơn `pthread_create()` ở **chỗ nào**? Cái giá đó rơi vào lúc `fork()` chạy hay rải ra **sau đó**?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) thread nhẹ hơn — chung memory map, chung global, context switch rẻ; process nặng — memory map riêng, phải dùng IPC để liên lạc. Ở ví dụ này 2 thread tiện vì chia sẻ data đọc từ USB, nhưng phải xử lý data race. (b) `pthread_create` không phải copy memory map và tài nguyên riêng, thread nằm chung memory map với process cha, kernel ít việc hơn. (c) 2 process: log crash không ảnh hưởng process kia; 2 thread: chung bộ nhớ nên log thread corrupt memory thì cả chương trình chết. Thiết bị 24/7 nên chọn **2 process**, chấp nhận tốn tài nguyên đổi lấy độ tin cậy.
**Follow-up:** giá rơi vào **sau đó** — lần ghi đầu tiên gây page fault, kernel phải cấp trang mới và copy.

**✅ Đạt mức 4.** Câu (c) là chỗ ăn điểm: bạn **tự đưa ra đánh đổi** *"đổi tài nguyên lấy độ tin cậy, log chết nhưng thiết bị vẫn quét được"* — đúng thứ interviewer đo ở T2, không phải liệt kê định nghĩa. Follow-up về COW cũng đúng cơ chế.

**Bổ sung — `fork()` còn đắt ở phần TRẢ NGAY, bạn mới nói phần trả sau:**

| Lúc `fork()` chạy (trả ngay) | Sau đó (trả dần) |
|---|---|
| Copy **page table** — process 1GB thì bảng trang cũng lớn, phải sao chép toàn bộ mục | **COW page fault** ở mỗi lần ghi đầu tiên vào một trang |
| Đánh dấu **mọi** trang của cả cha lẫn con thành **read-only** | Mỗi fault: trap vào kernel → cấp trang mới → copy 4KB → sửa page table |
| Sao chép **fd table**, signal handler, namespace, `mm_struct`, `task_struct` | Nếu con `exec()` ngay thì phần lớn công đó **bỏ đi** |

`pthread_create()` bỏ qua **toàn bộ** cột trái: thread dùng chung `mm_struct`, chung fd table, chỉ cấp stack + `task_struct`.

**Bổ sung (a) — bảng chia sẻ đầy đủ:**

| Chia sẻ giữa thread cùng process | Riêng từng thread |
|---|---|
| `.text`, `.data`/`.bss` (global/static), **heap** | **stack** riêng |
| file descriptor table, signal handler, cwd, PID | **thanh ghi** + program counter |
| memory mapping | **TLS** (`thread_local`), `errno` |

**Chốt:** *"Thread rẻ vì không copy gì; process đắt hai lần — page table lúc fork, và COW fault rải về sau. Nhưng process cho bạn **ranh giới lỗi**, thứ mà thiết bị chạy 24/7 cần hơn là vài trăm KB RAM."*

**Ôn:** [03-operating-system/process-thread.md](../../../03-operating-system/process-thread.md) · bank OS-001, OS-002, OS-013 (COW), OS-017
</details>

---

### Câu 2 · CPP-030 · 🟡 · **3/4**

```cpp
std::vector<Sample> samples;
samples.reserve(100);

Sample* first = &samples[0];        // (1)
for (int i = 0; i < 200; ++i)
    samples.push_back(readSensor());

first->timestamp = now();           // (2)
```

**a)** `size` và `capacity` khác nhau thế nào? Sau vòng lặp chúng bằng bao nhiêu (xấp xỉ)?
**b)** Dòng (2) sai. Sai từ **thời điểm nào**, vì sao? Nếu đổi `200` thành `50` thì code có đúng không — bạn có yên tâm giữ nó không?
**c)** Ngoài con trỏ, còn gì bị vô hiệu hoá khi vector reallocate? Trong daemon 24/7 đọc sensor, thiết kế thế nào để tránh hẳn lớp bug này?

**🔁 Follow-up:** `auto it = samples.begin(); Sample& r = samples[5];` rồi `push_back` gây reallocate — `it` và `r` còn dùng được không, cùng lớp vấn đề với `first` hay khác? Nếu **buộc** phải giữ tham chiếu trong lúc vẫn `push_back` thì đổi cấu trúc thế nào?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) size = số phần tử đang có, capacity = chỗ đã cấp; sau vòng lặp capacity ≈ 200. (b) sai từ khi vector reallocate — `first` giữ địa chỉ vùng cũ → UB. Đổi thành 50 thì đúng *"nhưng không thể an tâm, dễ mất kiểm soát"*. (c) **chưa rõ**.
**Follow-up:** đúng — `it` và `r` **cùng lớp vấn đề**; ba hướng: mảng tĩnh cố định · sức chứa cố định + ring buffer · cấu trúc không realloc (linked list, hash map).

**✅ (a)(b) chuẩn.** Đặc biệt nhận định *"đổi 200 thành 50 thì đúng nhưng không yên tâm"* là **đúng chất kỹ sư** — code đúng do may mắn về số liệu là code sẽ hỏng khi ai đó đổi hằng số.

**❌ Vì sao chỉ 3 điểm:** ý (c) — **tầng thiết kế, tức phần mang điểm** (§6 luật ①) — ban đầu **trắng**. Bạn chỉ trả lời được sau khi tôi đưa hẳn `it` và `r` ra. Ở phỏng vấn thật, người hỏi sẽ không gợi ý như vậy.

**Đáp án (b) — chính xác thời điểm:**
`reserve(100)` cấp chỗ cho 100. Phần tử thứ **101** làm vector cấp vùng mới (thường gấp đôi → 200), **move toàn bộ 100 phần tử sang**, rồi **giải phóng vùng cũ**. Từ khoảnh khắc đó `first` trỏ vào **bộ nhớ đã trả lại** — dòng (2) là UB, và là loại UB nguy hiểm nhất: thường **vẫn chạy** vì bộ nhớ chưa bị cấp lại cho ai, rồi hỏng ngẫu nhiên khi tải cao.

**Đáp án (c) — cái gì bị vô hiệu hoá:**

| Thao tác | Cái gì hỏng |
|---|---|
| Reallocate (`push_back` vượt capacity, `resize`, `reserve` lớn hơn) | **Tất cả** iterator, con trỏ, tham chiếu |
| `insert`/`erase` ở giữa | Mọi thứ **từ vị trí đó về sau** |
| `erase` cuối / `pop_back` | Chỉ phần tử bị xoá |

Cả ba (`first`, `it`, `r`) **cùng một lớp vấn đề**: đều là **địa chỉ vào vùng lưu trữ của vector**, mà vector có quyền dời vùng đó bất cứ lúc nào.

**Bốn hướng thiết kế** (bạn nêu được 3 sau gợi ý, bổ sung hướng chuẩn nhất):

| Hướng | Cách | Đánh đổi |
|---|---|---|
| **Lưu chỉ số thay vì con trỏ** ⭐ | `size_t idx` rồi `samples[idx]` | Đơn giản nhất, không đổi container. Nhưng chỉ số cũng hỏng nếu `erase` ở giữa |
| **Container ổn định tham chiếu** | `std::deque` (thêm ở hai đầu **không** làm hỏng tham chiếu), `std::list` (ổn định tuyệt đối) | Mất truy cập liền khối, kém cache locality — đáng cân nhắc trên embedded |
| `vector<unique_ptr<Sample>>` | Địa chỉ object nằm trên heap, vector chỉ dời con trỏ | Thêm một lần gián tiếp + một lần cấp phát mỗi phần tử |
| **Sức chứa cố định** ⭐ (hợp daemon sensor) | `reserve` đúng worst-case rồi **không bao giờ vượt**; hoặc **ring buffer** cấp phát một lần | Phải biết trước giới hạn — nhưng đó chính là điều thiết bị nhúng nên làm |

⚠️ Lưu ý về đề xuất *"hash map"* của bạn: `std::unordered_map` **không** ổn định iterator (rehash làm hỏng iterator) nhưng **có** ổn định con trỏ/tham chiếu tới phần tử. Nửa đúng — cần phân biệt hai thứ.

**Chốt:** *"Địa chỉ vào trong vector chỉ sống tới lần reallocate kế tiếp. Trong daemon chạy dài, hoặc lưu **chỉ số**, hoặc chọn container **không dời phần tử** — đừng dựa vào việc 'chắc là chưa đầy'."*

**Ôn:** [01-cpp-fundamentals/memory-model.md](../../../01-cpp-fundamentals/memory-model.md) · bank CPP-030
</details>

---

### Câu 3 · CPP-031 · 🟡 · **3/4**

```cpp
class SensorCache {
public:
    Reading get(int id) const;      // (A) — đọc, có cache bên trong
    size_t  hitCount() const;       // (B)
private:
    mutable std::map<int, Reading> cache_;
    mutable std::mutex             m_;
    size_t                         hits_ = 0;
};
```

**a)** Hàm `const` cam kết điều gì? Nói chính xác — nó **không** phải "không sửa gì cả".
**b)** Hai member `mutable`: mỗi cái phục vụ gì, vì sao **bắt buộc** phải `mutable`? (Chú ý `m_` trong `hitCount() const`.)
**c)** `hits_` không `mutable`. Nếu muốn `get() const` tăng `hits_` thì có nên thêm `mutable` không?

**🔁 Follow-up:** tiêu chí *"không phải data quan trọng"* hơi mềm. Từ **góc nhìn người gọi hàm**, cái gì phân biệt `mutable` **chính đáng** với `mutable` **lạm dụng**?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) `const` ngăn sửa data member, cố sửa → compile error; ngoại lệ là member `mutable`. (b) mutex để thread-safe, cache_ lưu trạng thái tạm; cả hai "không phải data quan trọng" nên `mutable` hợp lý. (c) nên, vì `hits_` là biến đo lường, không ảnh hưởng cache_.
**Follow-up:** lặp lại tiêu chí cũ — *"chính đáng = không đụng dữ liệu quan trọng; lạm dụng = cố sửa dữ liệu quan trọng qua hàm const"*.

**✅ Kết luận đúng cả ba ý.** ❌ **Vì sao chỉ 3:** bạn nắm **cơ chế** (compiler chặn gì) nhưng chưa nắm **khái niệm** đứng sau, nên tiêu chí ở follow-up vẫn mềm — hỏi lại vẫn ra cùng một câu trả lời.

**Khái niệm còn thiếu — `const` bảo vệ *logical constness*, không phải *bitwise constness*:**

| | Nghĩa | C++ dùng cái nào |
|---|---|---|
| **Bitwise constness** | Không byte nào trong object thay đổi | ❌ Compiler *kiểm tra* theo hướng này, nhưng `mutable` mở cửa thoát |
| **Logical constness** ⭐ | **Người gọi không quan sát được khác biệt nào** | ✅ Đây mới là hợp đồng thật |

**Tiêu chí chặt thay cho "quan trọng/không quan trọng":**

> Gọi hàm `const` hai lần với **cùng đầu vào và cùng trạng thái ngoài** — người gọi có **quan sát thấy** kết quả khác nhau không? **Không** ⟹ `mutable` chính đáng.

Áp vào lớp trên:
- `cache_` — `get(5)` gọi hai lần trả **cùng một `Reading`**, chỉ lần hai nhanh hơn. Tốc độ không phải giá trị quan sát được của hợp đồng ⟹ ✅ chính đáng.
- `m_` — khoá/mở không đổi kết quả trả về ⟹ ✅ chính đáng.
- Nếu ai đó `mutable` một member rồi để `get()` trả **giá trị khác nhau** giữa hai lần gọi ⟹ ❌ lạm dụng, dù họ nói member đó "không quan trọng".

**Ý (b) — lý do BẮT BUỘC `mutable` cho mutex, bạn chưa nói thẳng:**
`m_.lock()` **sửa chính object mutex** (đặt cờ owner, thêm thread vào hàng đợi). Trong `hitCount() const` thì `this` là `const SensorCache*`, nên `m_` là `const std::mutex` — mà `lock()` **không phải** hàm `const` ⟹ **lỗi compile**. `mutable` gỡ đúng chỗ đó. Đây là ca dùng `mutable` kinh điển nhất trong C++ hiện đại.

**Ý (c) — bổ sung phần bạn chưa nêu:** thêm `mutable` cho `hits_` là **chưa đủ** — nếu nhiều thread gọi `get() const` cùng lúc thì `hits_++` là **data race**. Phải `mutable std::atomic<size_t> hits_`, hoặc tăng nó **bên trong** vùng đã khoá bởi `m_`.

```cpp
mutable std::atomic<size_t> hits_{0};   // hoặc tăng trong lock_guard(m_)
```

**Chốt:** *"`const` không hứa 'không byte nào đổi' — nó hứa **người gọi không thấy khác biệt**. `mutable` chính đáng khi nó giữ được lời hứa đó; và nhớ rằng `lock()` sửa mutex nên mutex trong class có hàm const luôn phải `mutable`."*

**Ôn:** [01-cpp-fundamentals/oop.md](../../../01-cpp-fundamentals/oop.md) · bank CPP-031, CPP-024
</details>

---

### Câu 4 · CPP-011 · 🟠 · **3/4**

```cpp
class Frame {
public:
    Frame(Frame&& o) { buf_ = o.buf_; o.buf_ = nullptr; }   // KHÔNG noexcept
    ~Frame() { delete[] buf_; }
    Frame(const Frame&);            // copy sâu — đắt
    Frame& operator=(const Frame&);
private:
    uint8_t* buf_ = nullptr;
};

std::vector<Frame> frames;
for (int i = 0; i < 10000; ++i)
    frames.push_back(captureFrame());
```

**a)** Vòng lặp chạy đúng nhưng **chậm bất thường**. Vì sao? Chuyện gì xảy ra mỗi lần vector reallocate?
**b)** Vì sao `std::vector` **cố tình** làm vậy thay vì cứ dùng move? Nó đang bảo vệ điều gì?
**c)** Nếu `Frame` **cấm copy** (`= delete`) và move vẫn không `noexcept` — code còn chạy không? Đổi vào lúc nào, compile hay runtime?

**🔁 Follow-up:** vector move được 5000/10000 thì phần tử 5001 **ném**. (a) 5000 phần tử ở **vùng cũ** đang ở trạng thái gì? (b) Vì sao **copy** thì trả về nguyên trạng được mà **move** thì không? Gọi tên bảo đảm vector đang giữ.

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) thiếu `noexcept` nên vector dùng **copy** thay vì move khi realloc → đắt. (b) object có thể corrupt và không phục hồi được nên copy an toàn hơn. (c) không chạy được — copy `= delete`, move không `noexcept` → **lỗi compile**. ✅
**Follow-up:** (a) *"5000 phần tử chưa kịp chuyển bị leak, vector corrupt, hỏng vùng mới mất luôn vùng cũ"* ❌. (b) *"copy thì bản gốc vẫn nguyên vẹn, vector xử lý phần hỏng và copy lại"* ✅.

**✅ (a)(b)(c) đúng bản chất, (c) chính xác cả giai đoạn.** ❌ **Vì sao chỉ 3:** không gọi được tên **strong exception guarantee**, và mô tả sai trạng thái vùng cũ — mà đó chính là **lý do** của toàn bộ cơ chế.

**Đáp án follow-up (a) — 5000 phần tử ở vùng cũ KHÔNG bị leak, chúng bị RỖNG RUỘT:**

```
Trước:   [vùng cũ] F1  F2  F3 ... F5000  F5001 ...   (đầy đủ)
                    ↓ move từng cái sang vùng mới
Sau khi move 5000:
         [vùng cũ] ⌀   ⌀   ⌀  ...  ⌀     F5001 ...   ← buf_ = nullptr, ruột đã bị cướp
         [vùng mới] F1  F2  F3 ... F5000  ✖ ném ở đây
```
Move ctor của bạn làm `o.buf_ = nullptr` — tức nó **phá huỷ nguồn**. Vector giờ **không còn đường lui**: vùng cũ đã rỗng, vùng mới thì dở dang. Nó không thể trả `frames` về trạng thái trước khi `push_back`.

**Đáp án (b) — tên gọi: `strong exception guarantee`.**

| Bảo đảm | Nghĩa | `vector::push_back` |
|---|---|---|
| **No-throw** | Không bao giờ ném | dtor, `swap`, move `noexcept` |
| **Strong** ⭐ | Ném thì **mọi thứ y như chưa gọi hàm** (commit-or-rollback) | ✅ đây là thứ `push_back` hứa |
| **Basic** | Ném thì object vẫn hợp lệ, nhưng trạng thái không xác định | — |

Với **copy**, nguồn **không bị đụng tới**: ném thì vector chỉ cần huỷ những bản đã copy ở vùng mới, `delete` vùng mới, giữ nguyên vùng cũ → **rollback thành công**. Với **move phá huỷ**, rollback là bất khả thi.

⇒ Vector dùng `std::move_if_noexcept`: **chỉ** move khi move ctor `noexcept`; ngược lại **copy** để giữ lời hứa. Nếu copy cũng không có (`= delete`) → **không compile** (đúng ý (c) của bạn).

**Sửa — thêm một từ:**
```cpp
Frame(Frame&& o) noexcept : buf_(std::exchange(o.buf_, nullptr)) {}
```
`noexcept` ở đây **không phải trang trí** — nó là điều kiện để vector được phép move. Cùng bài học với [CPP-054](../bank/cpp.md) đang trong sổ yếu.

**Chốt:** *"Move phá huỷ nguồn nên không rollback được; vector hứa strong guarantee nên nó chỉ dám move khi bạn thề `noexcept`. Quên `noexcept` = âm thầm tụt về copy."*

**Ôn:** [02-modern-cpp/move-semantics.md](../../../02-modern-cpp/move-semantics.md) · bank CPP-011, CPP-054
</details>

---

### Câu 5 · CPP-014 · 🟠 · **4/4**

```cpp
void f(Frame&& x);                    // (1)
template <typename T> void g(T&& x);  // (2)
auto h = [](auto&& x) { ... };        // (3)

Frame fr;
f(fr);        // (4)
g(fr);        // (5)
```

**a)** `Frame&&` ở (1) và `T&&` ở (2) khác loại thế nào? Cái nào là *universal/forwarding reference*?
**b)** (4) và (5) — dòng nào compile được, vì sao?
**c)** Trong `g`, muốn chuyển tiếp `x` **giữ nguyên** tính lvalue/rvalue — `std::move` hay `std::forward`? Dùng nhầm thì hỏng ở đâu?

**🔁 Follow-up:** `template<typename T> void b(std::vector<T>&& v);` và `template<typename T> class Buf { void c(T&& x); };` — có phải universal reference không? **Điều kiện** để `T&&` thành universal reference là gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) (1) rvalue ref, (2) universal ref. (b) (4) không compile — `Frame&&` không nhận lvalue `fr`; (5) compile được vì `T` deduce thành `Frame&`. (c) `std::forward` — *"cast có điều kiện"*; `std::move` là *"cast vô điều kiện"*, dùng nhầm sẽ **cướp tài nguyên của một lvalue mà caller còn đang giữ**.
**Follow-up:** cả (X)(Y) đều **không** universal — (X) vì dạng là `vector<T>&&` chứ không phải `T&&`; (Y) vì `T` là tham số **của class**, đã cố định trước khi gọi hàm. Điều kiện: **T phải được deduce ngay tại lời gọi** VÀ **cú pháp đúng là `T&&`**.

**✅ 4/4 — câu tốt nhất phiên.** Trả lời đủ ba tầng không cần gợi ý, và follow-up nêu đúng **cả hai** điều kiện (nhiều người chỉ nhớ một). Câu (c) diễn đạt *"cast có điều kiện vs vô điều kiện"* là cách nói gọn và chuẩn.

**Ghi thêm cho đầy đủ (không trừ điểm):**

**Cơ chế đằng sau — reference collapsing.** `T&&` thành universal được là nhờ luật gộp tham chiếu khi `T` tự nó đã là một reference:

| `T` deduce ra | `T&&` gộp thành | Ca |
|---|---|---|
| `Frame&` (truyền lvalue) | `Frame& &&` → **`Frame&`** | `g(fr)` |
| `Frame` (truyền rvalue) | `Frame&&` | `g(Frame{})` |

Luật: **`& &` , `& &&` , `&& &` → `&`**; chỉ **`&& &&` → `&&`**.

**`std::forward` chính là cast dựa vào `T` đó:**
```cpp
template <typename T>
void g(T&& x) {
    inner(std::forward<T>(x));   // T=Frame&  → truyền lvalue
                                 // T=Frame   → truyền rvalue
}
```

**Kịch bản hỏng cụ thể khi dùng nhầm `std::move`:**
```cpp
Frame fr;
g(fr);            // g dùng std::move(x) thay vì forward
use(fr);          // ❌ fr đã bị cướp ruột — caller không hề biết
```
Caller truyền một **lvalue mình vẫn còn dùng**, hàm lại đối xử như rvalue.

**Quy tắc nhớ:** `std::move` cho **rvalue reference thật** (`T&&` không deduce) · `std::forward` cho **universal reference** (`T&&` có deduce). Và mỗi tham số chỉ được `move`/`forward` **một lần**, ở **lần dùng cuối cùng**.

**Ôn:** [02-modern-cpp/move-semantics.md](../../../02-modern-cpp/move-semantics.md) · bank CPP-013, CPP-014
</details>

---

### Câu 6 · CPP-016 · 🟠 · **2/4** 🔴 VÀO SỔ YẾU

```cpp
struct Device            { virtual void reset(); int id_; };
struct Scanner : Device  { void reset() override; Config cfg_; };

void handle(Device d);                    // (1) — nhận theo GIÁ TRỊ
std::vector<Device> devices;              // (2)

Scanner s;
handle(s);                                // (3)
devices.push_back(s);                     // (4)
```

**a)** Dòng (3) và (4) — chuyện gì xảy ra với phần `Scanner`? Gọi tên hiện tượng.
**b)** Trong `handle()`, `d.reset()` chạy phiên bản nào? Giải thích qua **vptr**.
**c)** Đây là lỗi **im lặng**. Thiết kế API thế nào để lớp bug này **không thể xảy ra**? ≥2 cách + đánh đổi.

**🔁 Follow-up:** cách sửa call site không ngăn được đồng nghiệp viết một `handle(Device d)` mới tuần sau. Có cách nào sửa ở **phía class `Device`** để truyền theo giá trị **không compile được**?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) phần `Scanner` bị cắt cho vừa `Device`; gọi đúng tên **object slicing** ✅. (b) chạy bản `Device` vì `s` bị slicing, vptr trỏ vtable Base. (c) hai cách: dùng `Device*`/`Device&` cho `handle()`; dùng **factory pattern** để tạo đúng type từ đầu.
**Follow-up:** **chưa rõ**.

**❌ Vì sao 2 điểm:** (a) đúng, (b) kết luận đúng nhưng cơ chế lệch, (c) — **tầng thiết kế, tức phần mang điểm** — trả lời bằng cách sửa **call site** chứ không phải **phòng ngừa**, và follow-up (chính là câu hỏi thật) **trắng**. Câu hỏi ghi rõ *"để lớp bug này **không thể** xảy ra"*.

**Chỉnh cơ chế (b) — vptr KHÔNG BAO GIỜ được copy:**

Bạn nói *"s bị slicing nên vptr trỏ vtable Base"*, nghe như vptr bị copy rồi sửa. Thực tế:

1. Tham số `Device d` có kích thước đúng **`sizeof(Device)`** — không đủ chỗ cho `cfg_`.
2. Cái chạy là **`Device::Device(const Device&)`** — copy ctor **của Device**.
3. Constructor của `Device` **tự đặt vptr** trỏ vtable `Device`. vptr không nằm trong danh sách được copy; **mỗi constructor tự thiết lập vptr của lớp mình**.
4. ⇒ `d` là một `Device` thuần chủng, không phải "Scanner bị cắt". `d.reset()` gọi `Device::reset` không phải vì "mất thông tin" mà vì **`d` chưa bao giờ là Scanner**.

Cùng lý do khiến gọi hàm virtual trong constructor chạy bản Base ([CPP-035](../bank/cpp.md)) — vptr được đặt theo **lớp đang được xây**.

**Đáp án (c) — chặn từ phía class, hai lớp:**

```cpp
struct Device {
    virtual void reset() = 0;                     // ① ABSTRACT
    virtual ~Device() = default;
protected:
    Device(const Device&)            = default;   // ② copy chỉ cho lớp con
    Device& operator=(const Device&) = default;
};
```

| Cách | Chặn được gì | Đánh đổi |
|---|---|---|
| ① **Abstract base** (≥1 pure virtual) | `void handle(Device d)` **lỗi ngay tại khai báo hàm**: *"cannot declare parameter to be of abstract type"*. `vector<Device>` cũng lỗi | Base không còn dùng độc lập được — thường là điều bạn muốn với interface |
| ② **Copy ctor `protected`** | Lớp con vẫn copy được lẫn nhau, người ngoài thì không | Dùng khi Base **buộc** phải cụ thể hoá được |
| ③ `Device(const Device&) = delete;` | Cấm tuyệt đối | Lớp con cũng mất copy — thường quá tay |

**Điểm mấu chốt:** cả ba đều làm lỗi nổ ở **chỗ định nghĩa hàm sai**, không phải ở chỗ gọi. Đồng nghiệp tuần sau **không viết được** hàm đó nữa — đó mới là *"không thể xảy ra"*.

**Về đề xuất "factory" của bạn:** factory **không** chặn slicing. `Device makeDevice()` trả về theo giá trị vẫn slice y hệt. Chỉ khi factory trả **`std::unique_ptr<Device>`** thì mới an toàn — và lúc đó cái cứu bạn là **con trỏ**, không phải factory. Đừng gán công cho pattern.

**Cách dùng đúng sau khi sửa:**
```cpp
void handle(const Device& d);                    // đa hình hoạt động
std::vector<std::unique_ptr<Device>> devices;    // lưu đa hình
devices.push_back(std::make_unique<Scanner>());
```

**Chốt:** *"Sửa call site là dán băng — người sau vẫn viết lại được. Base đa hình thì nên **abstract**, và copy nên **protected**: lúc đó slicing không phải bug cần nhớ tránh, mà là thứ compiler không cho viết ra."*

**Lần sau sẽ hỏi:** cho một base class đã có `virtual` nhưng vẫn copy được → *"đồng nghiệp vừa thêm `handle(Device d)`, chặn kiểu gì để lần sau không ai viết được nữa?"*

**Ôn:** [01-cpp-fundamentals/oop.md](../../../01-cpp-fundamentals/oop.md) · bank CPP-016, CPP-010, CPP-035
</details>

---

### Câu 7 · CPP-049 · 🟠 · **3/4**

Có câu cửa miệng: *"cứ `std::move` cho nhanh"*.

**a)** Kể **hai** kiểu mà `std::move` **không giúp gì** — move đắt đúng bằng copy. Vì sao?

```cpp
struct Packet {
    std::array<uint8_t, 1024> payload;   // (A)
    std::string               tag;       // (B)
    const std::string         id;        // (C)
};
Packet p2 = std::move(p1);
```

**b)** Với (A)(B)(C): cái nào thực sự move, cái nào bị copy? Đặc biệt giải thích (C).
**c)** Embedded dùng nhiều `std::array` và POD. Điều đó nói gì về việc *"thiết kế để move có ý nghĩa"*?

**🔁 Follow-up:** ở (a) bạn nói `array` move là O(N) không cướp được gì; ở (b) lại xếp (A) vào *"thực sự move"*. Có mâu thuẫn không? `Packet p2 = std::move(p1)` **thực tế** làm gì với 1024 byte?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) `std::array` — nằm trên stack, không có con trỏ để cướp, move từng phần tử, O(N); `const std::string` — dữ liệu là const nên không cướp được, phải copy. (b) (A)(B) move, (C) copy. (c) embedded dùng bộ nhớ tĩnh để tránh phân mảnh; move có nghĩa khi chuyển giao dữ liệu nặng, ví dụ `pop` của ring buffer.
**Follow-up:** tự gỡ đúng — *"cướp 1 byte không khác gì copy 1 byte nên move và copy không khác biệt"*.

**✅ (a)(c) tốt, (b) tự sửa được khi bị chỉ ra.** ❌ **Vì sao 3:** (a) và (b) **mâu thuẫn nhau** trong cùng một câu trả lời — bạn viết đúng ở (a) rồi phủ định chính mình ở (b). Ở phỏng vấn, tự mâu thuẫn làm người nghe nghi ngờ cả phần đúng.

**Đáp án (b) chuẩn — ba member, ba số phận:**

| Member | Move? | Vì sao |
|---|---|---|
| **(A)** `std::array<uint8_t,1024>` | ❌ **Copy 1024 byte** | `array` là **khối liền trong object**, không có con trỏ ra heap. "Move" một `uint8_t` chính là copy nó. `memcpy` 1KB, không hơn không kém |
| **(B)** `std::string tag` | ✅ **Move thật** | Nếu chuỗi dài (vượt SSO): cướp con trỏ heap + size + capacity, để nguồn rỗng — **O(1)**. ⚠️ Nếu chuỗi **ngắn** (SSO, ~15 ký tự) thì nội dung nằm **trong** object → vẫn là copy |
| **(C)** `const std::string id` | ❌ **Copy** | Xem dưới |

**Vì sao (C) copy — cơ chế chính xác, không phải "vi phạm nguyên tắc":**
```cpp
const std::string id;
std::move(id)  →  const std::string&&        // ép sang rvalue nhưng VẪN const
```
Overload resolution:
- `string(string&&)` — ❌ không nhận được, `const string&&` không bind vào `string&&`
- `string(const string&)` — ✅ nhận được (`const&` bind mọi thứ)

⇒ **rơi âm thầm về copy ctor**. Không lỗi, không cảnh báo, chỉ chậm. Đây là lý do **member `const` phá move semantics** — bài học thiết kế: đừng đánh `const` cho data member, dùng getter không setter để đạt cùng mục đích.

**Bổ sung — nhóm "move vô ích" đầy đủ:**

| Kiểu | Vì sao move = copy |
|---|---|
| Kiểu built-in (`int`, con trỏ thô) | Không có gì để cướp |
| `std::array<T,N>`, struct POD | Dữ liệu nằm trong object |
| Bất kỳ member nào là `const` | `std::move` cho ra `const T&&` → rơi về copy |
| `std::string` ngắn (SSO) | Nội dung nằm trong object |
| Class **không có** move ctor | Rơi về copy (và nếu có dtor tự viết thì move **không được sinh** — xem câu 13) |

**Ý (c) — phát biểu chặt hơn:** move chỉ có nghĩa khi object **sở hữu tài nguyên ngoài thân nó** (bộ nhớ heap, fd, handle). Embedded ưa dữ liệu tĩnh phẳng ⟹ phần lớn kiểu **không hưởng lợi** từ move. Chỗ move thực sự đáng giá trên embedded là các **RAII wrapper quanh tài nguyên hệ điều hành** — `SocketHandle`, `FileHandle`, `MmapRegion` — chính là bài coding [CPP-054](../bank/cpp.md) còn nợ.

**Chốt:** *"Move chỉ rẻ khi có con trỏ để cướp. `array` và POD không có gì để cướp; member `const` thì không cho cướp. Trên embedded, move đáng bận tâm ở handle tài nguyên, không ở buffer tĩnh."*

**Ôn:** [02-modern-cpp/move-semantics.md](../../../02-modern-cpp/move-semantics.md) · bank CPP-049, CPP-008
</details>

---

### Câu 8 · OS-020 · 🟡 · **2/4** 🔴 VÀO SỔ YẾU

```cpp
int counter = 0;
void logEvent(const char* msg) {
    std::lock_guard<std::mutex> lk(m);
    counter++;
    write(fd, msg, strlen(msg));
}
```

**a)** `logEvent` có **thread-safe** không? Có **reentrant** không? Hai khái niệm khác nhau ở chỗ nào?
**b)** `logEvent` được gọi từ **signal handler** trong khi luồng chính đang chạy dở chính nó. Chuyện gì xảy ra?
**c)** Viết phiên bản **dùng được trong signal handler**. Phải bỏ những gì, vì sao?

**🔁 Follow-up:** sau khi bỏ mutex, vẫn còn `write()` và `strlen()`. Hai hàm đó gọi được trong signal handler không? **Căn cứ vào đâu** để biết một hàm bất kỳ có được phép gọi?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) có thread-safe, không reentrant; *"thread-safe = nhiều thread gọi đồng thời có an toàn không; reentrant = bị cắt ngang rồi gọi lại có an toàn không"* ✅. (b) **deadlock** — đang giữ mutex, handler chen vào lock lần nữa ✅. (c) bỏ mutex, `counter` thành atomic, dùng hàm họ `_r`.
**Follow-up:** *"`write` và `strlen` không đảm bảo reentrant; cần dùng họ `_r` để có buffer riêng từ caller"* ❌.

**✅ (a)(b) chuẩn** — định nghĩa đúng và nhận ra self-deadlock. **❌ Vì sao 2 điểm:** phần **vận dụng** — tức tầng mang điểm — **sai hai chỗ**, và sai theo hướng dẫn tới quyết định kỹ thuật sai trong công việc.

**❌ Sai ①: `write()` KHÔNG phải hàm cấm — nó LÀ async-signal-safe.**
`write()` nằm trong danh sách chuẩn POSIX (`man 7 signal-safety`). Lý do: nó là **syscall trần** — không lấy khoá nào, không dùng buffer trong userspace. Bạn loại bỏ đúng công cụ nên dùng.

**❌ Sai ②: họ `_r` giải quyết THREAD-SAFETY, không phải SIGNAL-SAFETY.**

| | Vấn đề nó chữa | Ví dụ |
|---|---|---|
| **Họ `_r`** | Hàm dùng **static buffer dùng chung** → không thread-safe | `strtok_r`, `localtime_r`, `getpwnam_r` |
| **Async-signal-safe** | Hàm bị **chen ngang giữa chừng** bởi handler | danh sách POSIX riêng |

Hai tập **giao nhau nhưng không trùng**: `localtime_r` thread-safe nhưng **vẫn không** async-signal-safe. Suy ra *"cứ `_r` là gọi được trong handler"* là sai.

**Bảng tra:**

| ✅ Async-signal-safe | ❌ KHÔNG (dù thread-safe) | Vì sao cấm |
|---|---|---|
| `write`, `read`, `open`, `close` | `printf`, `fprintf`, `puts` | buffer + khoá stdio |
| `_exit`, `kill`, `signal`, `sigaction` | `malloc`, `free`, `new`, `delete` | **khoá heap** — handler chen giữa lúc heap dở dang |
| `time`, `sem_post`, `waitpid` | `localtime`, `strerror`, `getpwnam` | trả con trỏ tới **static buffer** |

**Nguyên tắc tự suy (thay cho học thuộc danh sách) — đây là thứ đáng nhớ:**

> Handler chen vào **giữa** một thao tác của luồng chính. Nên hàm nào **lấy khoá** hoặc **đụng trạng thái toàn cục có thể đang dở dang** thì cấm.
> `malloc` giữ khoá heap → handler gọi `malloc` = deadlock hoặc hỏng heap.
> `printf` giữ khoá stdio + buffer dở → tương tự.
> `write` chỉ là một `syscall` → an toàn.

**Bản đúng cho (c):**
```cpp
volatile sig_atomic_t counter = 0;      // kiểu chuẩn cho dữ liệu chia sẻ với handler

void logEventSafe(const char* msg, size_t len) {
    counter++;                          // KHÔNG khoá
    write(STDERR_FILENO, msg, len);     // ✅ hợp lệ
}
```
Bỏ mutex (nguồn của deadlock), truyền sẵn `len` để không phụ thuộc `strlen`. Dùng `sig_atomic_t` vì chuẩn C bảo đảm đọc/ghi nó không bị cắt ngang giữa chừng.

**Mẫu thực chiến — self-pipe trick** (thứ mà production code dùng):
```cpp
void handler(int) {
    char b = 1;
    write(pipefd[1], &b, 1);   // chỉ báo hiệu, KHÔNG xử lý
}
// event loop chính: epoll thấy pipefd[0] readable → xử lý phần nặng ở đó
```
Handler chỉ làm việc tối thiểu; mọi thứ cần khoá/cấp phát để vòng lặp chính làm.

**Chốt:** *"Thread-safe được phép dùng khoá; reentrant thì không — vì kẻ chen ngang chính là mình. Vì thế danh sách signal-safe hẹp hơn hẳn, `write` nằm trong đó còn `printf`/`malloc` thì không, và `_r` không phải vé vào cửa."*

**Lần sau sẽ hỏi:** cho một handler gọi `malloc`/`printf` → *"cái nào cấm, vì sao, sửa bằng mẫu gì?"*

> Bank [OS-020](../bank/os.md) **đã được viết lại trong phiên này** theo góp ý của bạn (đáp án cũ sơ sài, thiếu code) — nay có ví dụ đầy đủ, bảng tra và cảnh báo `_r` ≠ signal-safe.

**Ôn:** [04-linux-system-programming/processes-signals.md](../../../04-linux-system-programming/processes-signals.md) · bank OS-020, LNX-011
</details>

---

### Câu 9 · CPP-017 · 🟠 · **3/4**

```cpp
// Hướng A — đa hình runtime
struct ISensor { virtual Reading read() = 0; virtual ~ISensor() = default; };
void process(ISensor& s);

// Hướng B — template, đa hình compile-time
template <typename S> void process(S& s) { auto r = s.read(); ... }
```

**a)** Mỗi hướng trả giá ở đâu — **runtime** và **kích thước binary**? Cơ chế cụ thể.
**b)** Khi nào **buộc** phải chọn A? Cho tình huống mà B **không thể** làm được.
**c)** Firmware MCU, RAM 64KB, danh sách sensor **biết trước lúc compile**, yêu cầu thời gian đáp ứng chặt. Chọn hướng nào, **đánh đổi** cái gì?

**🔁 Follow-up:** trên MCU code nằm ở **flash**, cũng hữu hạn. 20 loại sensor → template sinh 20 bản `process()`. (a) Cái bạn **đánh đổi** là gì? (b) Có cách giữ lợi ích compile-time mà **giảm bloat** không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) runtime trả giá ở vtable/vptr nhưng binary gọn; template không overhead runtime nhưng sinh nhiều bản → binary phình. ✅ (b) buộc chọn A khi **chỉ biết loại sensor lúc runtime** — ví dụ đọc địa chỉ qua I2C rồi mới biết là chip gì ✅ (ví dụ rất tốt). (c) chọn template vì overhead thấp, biết trước lúc compile.
**Follow-up:** (a) *"đánh đổi 19 bản không được sử dụng và chiếm dung lượng"* ❌. (b) **chưa rõ**.

**✅ (a)(b) tốt, ví dụ I2C đúng chất thực chiến.** ❌ **Vì sao 3:** (c) câu hỏi bảo *"nêu rõ cái bạn đánh đổi"* nhưng bạn nêu **lý do chọn**; và follow-up thì hiểu sai bản chất bloat + không biết kỹ thuật giảm.

**❌ Sửa: "19 bản không được sử dụng" là hiểu sai.**
Có 20 loại sensor thì **cả 20 bản đều được dùng** — mỗi loại một bản. Không bản nào thừa. Bloat không đến từ *bản thừa* mà từ: **mỗi bản là một bản sao đầy đủ của toàn bộ thuật toán**, dù 90% thân hàm **giống hệt nhau** và chẳng phụ thuộc kiểu.

```
Hướng A (virtual):   1 bản process()  +  20 vtable nhỏ
Hướng B (template): 20 bản process()  — mỗi bản chép lại toàn bộ logic
```

**Đánh đổi thật của hướng B:**

| Được | Mất |
|---|---|
| Không vtable lookup, **inline được** toàn bộ | **Flash × 20** cho phần thân hàm |
| Compiler biết kiểu cụ thể → tối ưu sâu | **Không** thêm loại sensor mới lúc runtime |
| Không vptr trong object (tiết kiệm 4–8 byte/object) | Build lâu hơn, lỗi template khó đọc |
| Thời gian đáp ứng **tất định** (không gián tiếp) | Đổi thuật toán = build lại toàn bộ |

**Đáp án follow-up (b) — kỹ thuật *thin template / fat function*:**

Tách phần **không phụ thuộc kiểu** ra một hàm thường; template chỉ còn vỏ mỏng.

```cpp
// FAT — biên dịch MỘT lần, nằm trong flash một bản duy nhất
void processImpl(const Reading& r) {
    /* toàn bộ logic lọc, kiểm CRC, đóng gói, gửi... */
}

// THIN — sinh 20 bản, nhưng mỗi bản chỉ vài lệnh
template <typename S>
inline void process(S& s) {
    processImpl(s.read());      // chỉ phần này thực sự phụ thuộc kiểu
}
```
20 bản `process` giờ mỗi bản chỉ là *"gọi `s.read()` rồi nhảy vào `processImpl`"* — vài chục byte thay vì vài KB. Vẫn giữ được: không vtable, `s.read()` **inline được**.

> Cùng ý tưởng với việc thư viện chuẩn cài `std::vector<T*>` qua `vector<void*>` bên trong để tránh sinh trùng lặp.

**Bổ sung (b) — ca nữa buộc dùng hướng A:** cần **lưu chung một container** các loại khác nhau (`vector<unique_ptr<ISensor>>`), hoặc cần **plugin nạp lúc chạy** (`dlopen`), hoặc muốn **giữ ABI ổn định** cho thư viện chia sẻ (template phơi toàn bộ implementation ra header — xem CPP-023).

**Chốt:** *"Virtual trả giá mỗi lời gọi; template trả giá mỗi kiểu — và trả bằng flash. Muốn cả hai thì đẩy phần chung xuống một hàm thường, để template chỉ còn cái vỏ."*

**Ôn:** [01-cpp-fundamentals/templates.md](../../../01-cpp-fundamentals/templates.md) · bank CPP-017, CPP-023
</details>

---

### Câu 10 · SD-016 · 🔴 🏗️ · **2/4** 🔴 VÀO SỔ YẾU

**Đề:** Thiết bị quét mã vạch chạy **liên tục nhiều tháng không reboot**. Yêu cầu: không chậm dần, không chết vì hết bộ nhớ.

**a)** `new`/`delete` rải rác gây **hai** vấn đề khác nhau cho thiết bị chạy dài ngày. Kể cả hai và phân biệt rõ.
**b)** Vấn đề thứ hai **không** sửa được bằng "cẩn thận `delete` đầy đủ". Vì sao? Chuyện gì xảy ra bên trong allocator?
**c)** Thiết kế chiến lược bộ nhớ thế nào? **Cách làm cụ thể** + đánh đổi. Nếu code đã lỡ dùng **STL khắp nơi** thì xử lý ra sao?

**🔁 Follow-up:** code `delete` hoàn hảo, không rò rỉ. Cấp/giải phóng xen kẽ khối 100 và 1000 byte, rồi free hết khối 1000. Xin 5000 byte — tổng trống thừa sức nhưng vẫn thất bại. **Vì sao allocator không gom các mảnh trống lại?**

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) ① **phân mảnh** ngày càng nhiều → chết dù tổng vùng trống vẫn đủ ✅ ② **thời gian bất định** ✅ (b) **chưa rõ**. (c) **pool/arena** cấp cố định một lần, cho mượn và thu lại ✅; phần STL **chưa rõ**.
**Follow-up:** sau gợi ý mạnh, nêu được — *"không gom được vì trên các phần rải rác còn có con trỏ hiện hữu"* ✅.

**✅ Ý (a) hoàn chỉnh** — nêu đúng **cả hai** vấn đề và phân biệt được chúng; đây là phần nhiều người chỉ nói được một. Hướng pool/arena ở (c) cũng đúng.
**❌ Vì sao 2 điểm:** (b) chỉ ra được sau khi tôi đưa gần hết dữ kiện, và (c) phần *"code đã lỡ dùng STL"* — tình huống **thực tế nhất** của câu hỏi — bỏ trống.

**Đáp án (b) — vì sao "delete cẩn thận" không chữa được phân mảnh:**

```
Cấp phát xen kẽ rồi giải phóng HẾT các khối 1000 (không rò rỉ một byte):

 [100][1000][100][1000][100][1000][100]
        ↓ free hết khối 1000
 [100][    ][100][    ][100][    ][100]
        900        900        900          ← tổng trống 2700 byte

 Xin 1 khối 2000  →  ❌ THẤT BẠI (không mảnh nào đủ 2000)
```

**Vì sao allocator không gom ba mảnh 900 lại?** Muốn gom thì phải **dời** các khối 100 đang dùng. Nhưng C/C++ phơi bày **địa chỉ thật** — chương trình đang giữ con trỏ trỏ thẳng vào đó. Dời khối = mọi con trỏ ấy thành rác, và allocator **không có cách nào tìm ra chúng** để cập nhật.

> Java/Go **nén được** heap chính vì chúng **không** cho bạn giữ địa chỉ trần — chúng dùng handle mà runtime sửa được. C++ đánh đổi khả năng đó lấy hiệu năng và quyền kiểm soát.

⇒ **Phân mảnh là hệ quả của MẪU CẤP PHÁT, không phải của KỶ LUẬT GIẢI PHÓNG.** Sửa bằng cách đổi mẫu, không phải bằng cách "cẩn thận hơn". Đây là câu chốt của toàn bộ câu hỏi.

**Đáp án (c) phần 1 — chiến lược:**

| Chiến lược | Cách làm | Hợp với |
|---|---|---|
| **Cấp hết lúc init rồi khoá** | Mọi buffer/object cấp trong `init()`, sau đó **cấm** `new` | Firmware, hệ RT cứng |
| **Object pool** | Mảng slot cố định + free-list; mượn/trả **O(1)**, cạn thì trả `nullptr` (tất định) thay vì `bad_alloc` | Object cùng kiểu, số lượng có trần |
| **Arena / monotonic** | Cấp bằng cách đẩy con trỏ; **không** trả lẻ, reset cả khối theo chu kỳ | Dữ liệu tạm theo **khung**: mỗi lần quét mã vạch reset một lần |
| **Fixed-block theo size-class** | Vài pool cho vài cỡ (32/64/256 byte) | Kích thước đa dạng nhưng có giới hạn |

**Đáp án (c) phần 2 — "đã lỡ dùng STL khắp nơi" ⟹ `std::pmr` (C++17):**

Giữ nguyên container, **chỉ đổi nguồn cấp phát**. Đã biên dịch và chạy thật:

```cpp
#include <memory_resource>

static std::array<std::byte, 64*1024> pool;                    // bộ nhớ TĨNH, cấp 1 lần
std::pmr::monotonic_buffer_resource res{pool.data(), pool.size()};

std::pmr::vector<Sample>          samples{&res};                // vẫn là vector
std::pmr::string                  name{&res};
std::pmr::unordered_map<int, Log> logs{&res};

res.release();     // "giải phóng" cả pool trong O(1) — chỉ reset con trỏ
```

| Memory resource | Hành vi | Hợp với |
|---|---|---|
| `monotonic_buffer_resource` | Chỉ cấp, không trả lẻ, reset cả khối | Dữ liệu theo khung/vòng lặp |
| `unsynchronized_pool_resource` | Pool theo size-class, có trả lẻ | Vòng đời lẫn lộn, **một luồng** |
| `synchronized_pool_resource` | Như trên + khoá | Nhiều luồng dùng chung |

**Đánh đổi phải nói ra:** `pmr` thêm **một lần gián tiếp qua vtable** mỗi lần cấp phát (memory resource là interface ảo). Đổi lại: tất định, không phân mảnh heap toàn cục, **không phải sửa logic**. Với hệ chạy dài ngày, gần như luôn là giao dịch có lợi.

**Chốt:** *"Phân mảnh sinh ra từ mẫu cấp phát chứ không từ việc quên free — nên phải đổi mẫu, không phải đổi kỷ luật. Cấp một lần lúc init rồi chỉ mượn-trả trong pool; code cũ đầy STL thì bọc bằng `std::pmr` thay vì viết lại."*

**Lần sau sẽ hỏi:** *"cho một daemon dùng `std::vector`/`std::string` khắp nơi — chuyển sang bộ nhớ tất định mà không viết lại logic, làm thế nào?"*

> Bank [SD-016](../bank/system-design.md) **đã viết lại trong phiên này** — thêm sơ đồ phân mảnh, giải thích cơ chế, code `pmr` đã chạy.

**Ôn:** [08-embedded-systems/constraints.md](../../../08-embedded-systems/constraints.md) · [11-design-patterns/creational.md](../../../11-design-patterns/creational.md) (Object Pool)
</details>

---

### Câu 11 · DP-011 · 🔴 🏗️ · **3/4**

```cpp
class BarcodeReader {
    Scan scan() {
        uint8_t buf[256];
        int fd = ::open("/dev/scanner0", O_RDONLY);   // (1)
        ::read(fd, buf, sizeof buf);                  // (2)
        ::close(fd);
        return decode(buf);
    }
};
```
Team muốn **unit test** `decode` và logic retry, chạy trên **CI không có phần cứng**.

**a)** Vì sao class này **không test được**? Chỉ ra chính xác chỗ tạo phụ thuộc.
**b)** Áp dụng **Dependency Inversion** để test được. **Ai sở hữu interface** — module cấp cao hay driver? Vì sao đó mới là "inversion"?
**c)** Bạn vừa thêm một lớp trừu tượng vào đường dữ liệu nóng. Chi phí là gì, xử lý thế nào? Có cách **không** trả chi phí runtime không?

**🔁 Follow-up:** (a) `ISensorPort.h` **thuộc về** module cấp cao hay tầng driver? "Inversion" đảo ngược cái gì? (b) Hướng đa hình bạn chọn ở câu 9 có áp được vào đây để có DIP mà không tốn runtime không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) `open/read/close` gọi thẳng phần cứng qua fd, không có phần cứng thì lệnh thất bại ✅. (b) tách ra `HALReadFile` để mock ✅. (c) chi phí: nhiều class/wrapper hơn, tăng binary, tốn flash/RAM; cách xử lý **chưa rõ**.
**Follow-up:** (a) *"cả cấp cao và cấp thấp đều phụ thuộc abstraction"* — đúng công thức SOLID nhưng **không trả lời ai sở hữu file**. (b) nêu được **template** làm nền, nhưng ban đầu nói *"mock test vẫn không giải quyết được"*; sau nhắc thì nhận ra cần **instantiate một bản riêng cho test**.

**✅ (a)(b) đúng hướng, và bạn tự tìm ra template injection ở follow-up.** ❌ **Vì sao 3:** câu hỏi *"ai sở hữu interface"* hỏi hai lần vẫn chưa trả lời trúng; và (c) nêu được chi phí nhưng không có giải pháp cho tới khi được nhắc.

**Đáp án — "ai sở hữu" mới là toàn bộ ý nghĩa của chữ *inversion*:**

```
❌ Cách thông thường (dependency đi XUỐNG):
   BarcodeReader  ──phụ thuộc──▶  ScannerDriver
   Tầng thấp định nghĩa cái nó CUNG CẤP; tầng cao phải theo.
   Đổi driver ⟹ sửa BarcodeReader.

✅ Sau khi INVERT (mũi tên phụ thuộc bị đảo):
   BarcodeReader ──▶ ISensorPort ◀── ScannerDriver
                     ▲                    │
        SỞ HỮU bởi tầng cao          implement
   Tầng cao khai báo cái nó CẦN; tầng thấp phải khớp.
```

**File `ISensorPort.h` thuộc về module cấp cao** — nằm cùng thư mục/target với `BarcodeReader`, không nằm trong thư mục driver. Đây là điểm mấu chốt và cũng là chỗ đa số làm sai: họ tạo interface rồi đặt nó cạnh driver — lúc đó vẫn là tầng cao phụ thuộc tầng thấp, chỉ thêm một lớp gián tiếp vô ích.

Cái bị **đảo ngược** là **chiều của quan hệ phụ thuộc lúc biên dịch**, không phải chiều gọi hàm (`BarcodeReader` vẫn gọi xuống driver).

```cpp
// ── high-level/ISensorPort.h  ← module cấp cao SỞ HỮU file này
struct ISensorPort {
    virtual int read(uint8_t* buf, size_t n) = 0;
    virtual ~ISensorPort() = default;
};

// ── high-level/BarcodeReader.h
class BarcodeReader {
public:
    explicit BarcodeReader(ISensorPort& port) : port_(port) {}   // inject
    Scan scan() { uint8_t b[256]; port_.read(b, sizeof b); return decode(b); }
private:
    ISensorPort& port_;
};

// ── driver/ScannerDriver.h  ← tầng thấp phải KHỚP với interface của tầng cao
class ScannerDriver : public ISensorPort { int read(uint8_t*, size_t) override; };

// ── test/MockPort.h
class MockPort : public ISensorPort {
    int read(uint8_t* b, size_t n) override { /* dữ liệu dựng sẵn, mô phỏng lỗi để test retry */ }
};
```

**Đáp án (c) — hai mức, tuỳ ràng buộc:**

| | Runtime injection (virtual) | Compile-time injection (template) |
|---|---|---|
| Cú pháp | `BarcodeReader(ISensorPort&)` | `template<class Port> class BarcodeReader` |
| Chi phí | 1 lần gián tiếp qua vtable mỗi lời gọi + vptr trong object | **0** — inline được toàn bộ |
| Đổi implementation | **lúc chạy** (`dlopen`, cấu hình) | **lúc compile** |
| Test | `MockPort` là lớp con | `BarcodeReader<MockPort>` trong bản build test |
| Hợp với | App Linux, plugin | Firmware MCU, hot path |

Bản template — chính là hướng B của câu 9:
```cpp
template <typename Port>
class BarcodeReader {
public:
    explicit BarcodeReader(Port& p) : port_(p) {}
    Scan scan() { uint8_t b[256]; port_.read(b, sizeof b); return decode(b); }
private:
    Port& port_;
};

BarcodeReader<ScannerDriver> reader{drv};   // production
BarcodeReader<MockPort>      reader{mock};  // test — CÙNG mã nguồn, khác lần build
```
Mock **vẫn dùng được** (chỗ bạn nghi ngờ ban đầu): test và production là **hai lần biên dịch riêng**, mỗi bên instantiate kiểu của mình. Chi phí runtime bằng **không**.

**Về ý (c) "chi phí là nhiều class hơn":** đúng, nhưng chi phí đáng kể hơn nằm ở chỗ khác — **một lần gián tiếp qua vtable trên mỗi lời gọi** trong hot path, cộng thêm **vptr 4–8 byte trong mỗi object**. Trên MCU, cả hai đều đo được.

**Chốt:** *"DIP không phải là 'thêm một interface' — mà là **interface thuộc về ai**. Tầng cao khai báo cái nó cần, tầng thấp phải khớp. Và nếu không muốn trả phí runtime thì tiêm bằng template thay vì virtual."*

**Ôn:** [11-design-patterns/solid-principles.md](../../../11-design-patterns/solid-principles.md) · [08-embedded-systems/hardware-debug.md](../../../08-embedded-systems/hardware-debug.md) · bank DP-011, SD-007

> ⚠️ Bank DP-011 **còn nợ viết lại** theo tiêu chuẩn mới (có code + cơ chế) — bạn đã góp ý đúng, tôi chưa làm kịp trong phiên.
</details>

---

### Câu 12 · CPP-032 · 🟠 · **3/4** — SỔ YẾU (1/2 lần ≥3)

```cpp
class Timeout {
public:
    Timeout(int ms);              // (1) không explicit
    operator int() const;         // (2) không explicit
private:
    int ms_;
};
void wait(Timeout t);
void retry(int times);
```
Lần trước xét lớp chỉ có **một** chiều chuyển ngầm. Lớp này có **cả hai**: `int → Timeout` và `Timeout → int`.

**a)** `Timeout a(100), b(200);` — `a + b` có compile không? Kết quả **kiểu gì**, giá trị bao nhiêu? `wait(a + b)` nghĩa là gì?
**b)** `retry(a)` xảy ra chuyện gì? Bug kiểu gì — vì sao **tệ hơn** bug ở lớp `Status` hôm trước?
**c)** Có **cả hai** chiều còn sinh ra một lớp lỗi mà chỉ một chiều thì không có. Là gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) có; cả hai ngầm chuyển sang `int`, kết quả kiểu `int` ✅ (b) `retry(a)` — `a` chuyển sang `int`, hàm bị dùng sai mục đích; *"code vẫn chạy, kết quả vẫn rõ ràng… khó phát hiện hơn `Status` chỉ thuần 0/1"* ✅ **rất đúng**. (c) tự nêu ca `Timeout c(a)` gọi `Timeout(int)` thay vì copy ctor ❌; sau khi được đưa bằng chứng thì giải thích **đúng** ca nhập nhằng ✅.

**✅ (a)(b) tốt** — đặc biệt (b): bạn nêu đúng **vì sao tệ hơn safe-bool**, đó là phần phân tích chứ không phải ghi nhớ. **❌ Vì sao chưa 4:** ca tự nêu ở (c) **sai**, và cách bạn kiểm chứng nó cũng sai (xem phần phương pháp cuối file).

**Chạy thật — bằng chứng:**
```
File của bạn:                  Cùng file + thêm log ở copy ctor:
  ctor 1 arg  ← a                ctor 1 arg  ← a
  ctor 1 arg  ← b                ctor 1 arg  ← b
  ctor 1 arg  ← "c(a)"?          COPY ctor   ← Timeout c(a)   ✅
  300                            ctor 1 arg  ← wait(a+b): 300 → Timeout
                                 300
```
**`Timeout c(a)` gọi COPY CTOR.** Copy ctor là **exact match**, luôn thắng chuỗi chuyển đổi do người dùng định nghĩa. Dòng log thứ ba thực ra là của `wait(a+b)` — khi `int 300` chuyển **ngược** về `Timeout`.

**Ba lớp lỗi của "hai chiều ngầm":**

**① Số học im lặng trên kiểu không phải số**
```cpp
int s = a + b;     // = 300, kiểu int. Cộng hai "thời hạn" ra một số vô nghĩa
wait(a + b);       // 300 quay ngược thành Timeout(300ms) — vòng tròn khép kín
```

**② Dùng nhầm API mà giá trị trông "hợp lý"** — chỗ bạn phân tích đúng
```cpp
retry(a);          // chạy: retry(100 LẦN), trong khi a nghĩa là 100 MILLISECOND
```
Với `operator bool` sai thì giá trị chỉ `0/1` — bất thường dễ thấy trong log. Ở đây giá trị **đúng con số người viết nghĩ tới**, chỉ **sai đơn vị** ⟹ log trông bình thường, bug sống rất lâu.

**③ Nhập nhằng — CHỈ có khi đủ hai chiều** (đây là ý (c))
```cpp
bool operator==(const Timeout&, const Timeout&);
bool r = (a == 100);
// error: ambiguous overload for 'operator==' (operand types are 'Timeout' and 'int')
```
Hai đường **cùng hạng**: `100 → Timeout` rồi dùng `operator==` của bạn · hoặc `a → int` rồi dùng `==` dựng sẵn. Không luật nào ưu tiên → lỗi. **Thiếu một trong hai chiều thì chỉ còn một đường, không nhập nhằng.**

Tương tự với hai conversion operator cùng hạng:
```cpp
operator int() const;  operator long() const;
a + 1;   // error: ambiguous overload for 'operator+'
```

**Sửa:**
```cpp
explicit Timeout(int ms);
explicit operator int() const;      // muốn lấy số thì gọi tên rõ: t.count()
```
Tốt hơn nữa cho API đo thời gian: dùng **kiểu mạnh** `std::chrono::milliseconds` — hệ thống kiểu chặn lẫn đơn vị ngay, `wait(500ms)` không thể nhầm với `retry(500)`.

**Chốt:** *"Một chiều ngầm đã dễ sai; hai chiều thì object của bạn thành số nguyên trá hình — vừa lọt phép toán vô nghĩa, vừa sinh nhập nhằng mà một chiều không có."*

**Lần sau sẽ hỏi (góc thứ tư):** `explicit` cho ctor **nhiều đối số** (C++11 cho phép) + braced-init.

**Câu mới đã thêm bank: [CPP-055](../bank/cpp.md).** **Ôn:** bank CPP-032, CPP-053, CPP-055
</details>

---

### Câu 13 · CPP-020 · 🟡 · **4/4** — RETENTION ✅ (dời Tuần 4)

```cpp
class Logger {
public:
    Logger(std::string path) : path_(std::move(path)) {}
    ~Logger() { std::cout << "closing " << path_ << "\n"; }   // chỉ ghi log, KHÔNG quản tài nguyên thô
private:
    std::string        path_;
    std::vector<char>  buf_;
};

std::vector<Logger> loggers;
loggers.push_back(Logger("/var/log/a"));
```

**a)** Class **không** quản lý con trỏ thô nào — mọi member tự dọn. Vậy destructor "vô hại" kia có làm mất gì không? Hậu quả **đo được** là gì?
**b)** Rule of 0 nói gì về class này? Nếu **thật sự cần** in log lúc huỷ thì làm sao vừa giữ được nó vừa không mất thứ ở (a)?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) chỉ cần **viết** dtor là compiler xem như class tự quản tài nguyên, **không sinh move** → `push_back` phải **copy**, rất đắt ✅ (b) Rule of 0 đề xuất không viết gì; nếu cần log thì ① giữ dtor + viết Rule of 5 kèm `= default`, hoặc ② tách hàm log riêng ✅

**✅ 4/4 — retention đạt, dời lịch kiểm tra sang Tuần 4.** Nhận ra ngay cái bẫy *"dtor vô hại"* mà không cần gợi ý, và đưa được **hai** hướng sửa. Đây là góc hỏi hoàn toàn mới so với hai lần trước (định nghĩa Rule 0/3/5, và class `unique_ptr` + `char*`), nên điểm 4 phản ánh hiểu thật chứ không phải nhớ đề cũ.

**Ghi thêm cho đầy đủ — bảng "cái gì chặn cái gì":**

| Bạn khai báo | Compiler NGỪNG sinh |
|---|---|
| **destructor** | move ctor, move assign |
| copy ctor **hoặc** copy assign | move ctor, move assign |
| move ctor **hoặc** move assign | copy ctor, copy assign (**bị `delete`**) |
| bất kỳ ctor nào | default ctor |
| *(member không copy được)* | copy của class bị **`delete`** |

**Hậu quả đo được ở (a):** `loggers.push_back(...)` và mỗi lần vector **realloc** đều sao chép **toàn bộ `std::string` + `std::vector<char>`** của mọi phần tử thay vì cướp con trỏ. Với 1000 logger, mỗi cái giữ buffer 4KB → **4MB copy thừa** mỗi lần realloc, cộng thêm hàng nghìn lần cấp phát heap. Không sai kết quả, chỉ chậm và tốn — đúng loại bug không ai để ý cho tới lúc profile.

**Bản sửa cho hướng ① (viết đủ Rule of 5):**
```cpp
class Logger {
public:
    Logger(std::string path) : path_(std::move(path)) {}
    ~Logger() { std::cout << "closing " << path_ << "\n"; }

    Logger(Logger&&) noexcept            = default;   // khôi phục move
    Logger& operator=(Logger&&) noexcept = default;
    Logger(const Logger&)                = default;   // (khai báo lại vì đã có move)
    Logger& operator=(const Logger&)     = default;
private:
    std::string path_;  std::vector<char> buf_;
};
```
⚠️ Bẫy kèm theo: object **moved-from** vẫn chạy destructor → sẽ in `closing ` với `path_` **rỗng**. Nếu log lúc huỷ là quan trọng thì phải xử lý (`if (!path_.empty())`), hoặc chọn hướng ②.

**Hướng ② (giữ Rule of 0 thật):** bỏ dtor khỏi `Logger`; đưa việc in log vào một **member RAII nhỏ** chuyên trách, hoặc ghi log ở nơi đóng file. Class chính không viết gì → compiler sinh đủ 5 hàm tối ưu.

**Chốt:** *"Destructor không chỉ dọn dẹp — nó là **lời khai báo** rằng class tự quản tài nguyên, và compiler đáp lại bằng cách rút move đi. Một dtor chỉ để in log cũng đủ biến class của bạn thành class chỉ copy."*

**Ôn:** [02-modern-cpp/raii-smart-pointers.md](../../../02-modern-cpp/raii-smart-pointers.md) §6 · bank CPP-020, CPP-048
</details>

---

### Câu 14 · CPP-029 · 🟡 · **3/4** — RETENTION ✅ (dời Tuần 4)

```cpp
std::vector<std::shared_ptr<Widget>> v;
Widget w;   std::string s = "abc";

v.emplace_back(new Widget);                      // (1)
v.push_back(std::make_shared<Widget>());         // (2)

std::vector<Widget> vw;   vw.emplace_back(w);    // (3)
std::vector<std::string> vs;   vs.emplace_back(s);  // (4)
```

**a)** Dòng (3)(4): `emplace_back` có nhanh hơn `push_back` không? Cái gì quyết định?
**b)** Dòng (1) gọn hơn (2) nhưng **nguy hiểm hơn**. Kịch bản nào khiến (1) **rò rỉ bộ nhớ**? Thời điểm cụ thể.
**c)** Nguyên tắc: khi nào `emplace_back` thực sự thắng, khi nào chỉ là thói quen gõ phím?

**🔁 Follow-up (chưa trả lời — phiên chuyển sang câu 15):** `new Widget` **thành công** rồi mới có exception. Exception đó đến từ **đâu**, và lúc đó ai đang giữ con trỏ thô?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) không nhanh hơn — `emplace_back` lợi khi **construct tại chỗ** từ đối số; (3)(4) truyền object hoàn chỉnh nên giống hệt `push_back` ✅ (b) *"(1) gồm 2 bước, tạo temp bằng `new` rồi construct vào vector; nếu có exception thì leak"* — đúng hướng nhưng chưa chỉ đúng nguồn. (c) `emplace_back` thắng khi construct tại chỗ; ca `initializer_list` thì **bắt buộc** `push_back` vì emplace không deduce được ✅

**✅ Retention đạt (≥3), dời lịch sang Tuần 4.** (a) và (c) chuẩn, kể cả ca braced-init-list — chi tiết mà nhiều người bỏ sót.
**❌ Vì sao chưa 4:** (b) chưa xác định được exception **đến từ đâu**, mà đó mới là chỗ giải thích vì sao lỗi này hiếm và khó thấy.

**Đáp án (b) — cửa sổ rò rỉ nằm ở đâu:**

```cpp
v.emplace_back(new Widget);
```
Trình tự thực tế:
1. **`new Widget`** — cấp phát thành công. Giờ có một **con trỏ thô** đang "bay giữa không trung", **chưa** ai sở hữu.
2. `emplace_back` chạy → thấy vector **đầy** → **realloc**: xin vùng mới, move/copy toàn bộ phần tử cũ sang.
3. Bước realloc **ném `std::bad_alloc`** (hoặc move ctor của phần tử ném).
4. Stack unwinding chạy. Nhưng con trỏ thô ở bước 1 **không phải object có destructor** — không ai gọi `delete` cho nó.
5. ⇒ **`Widget` rò rỉ vĩnh viễn.** Không ai nhận ra vì chương trình vẫn chạy tiếp.

**Vì sao (2) an toàn:**
```cpp
v.push_back(std::make_shared<Widget>());
```
`make_shared` trả về một **`shared_ptr` tạm — một object có destructor**. Nếu `push_back` ném, stack unwinding **huỷ shared_ptr tạm** → nó `delete` Widget → **không rò rỉ**.

> Cùng nguyên lý với [CPP-047](../bank/cpp.md) (`make_unique`/`make_shared` vs `new` trần): **đừng để con trỏ thô sống ngoài một object RAII, dù chỉ một biểu thức**.

**Bonus của (2):** `make_shared` cấp **một lần** cho cả object + control block, thay vì 2 lần với `new` — nhanh hơn, ít phân mảnh (đáng kể với câu 10).

**Đáp án (c) — bảng quyết định:**

| Tình huống | Dùng gì | Vì sao |
|---|---|---|
| `v.emplace_back(arg1, arg2)` — có sẵn **đối số**, chưa có object | ✅ **emplace** | Construct thẳng trong vector, **không tạo temporary** |
| `v.emplace_back(existingObj)` | ⚖️ Như nhau | Vẫn gọi copy ctor. Dùng `push_back` cho rõ ý |
| `v.push_back(std::move(obj))` | ✅ **push_back** | Ý định "chuyển giao" hiện rõ hơn |
| `v.push_back({1, 2, 3})` — braced-init-list | ✅ **BẮT BUỘC push_back** | `emplace` deduce theo template, không suy được `{...}` |
| `v.emplace_back(new T)` | ❌ **KHÔNG BAO GIỜ** | Rò rỉ như trên. Dùng `push_back(make_shared<T>())` |
| Kiểu có ctor `explicit` | ⚠️ emplace **lọt qua** `explicit` | Tiện, nhưng mất một lớp lưới an toàn (góc hỏi 10/08) |

**Chốt:** *"`emplace_back` thắng khi bạn có **nguyên liệu** chứ chưa có món ăn. Có sẵn object rồi thì hai hàm như nhau — và tuyệt đối đừng đưa con trỏ trần cho nó."*

**Ôn:** [02-modern-cpp/move-semantics.md](../../../02-modern-cpp/move-semantics.md) · bank CPP-029, CPP-047
</details>

---

### Câu 15 · CPP-052 · 🟠 · **3/4**

```cpp
std::shared_ptr<Config> g_cfg = std::make_shared<Config>();

// Thread A (reader, hot path)          // Thread B (writer, hiếm)
auto local = g_cfg;                     g_cfg = std::make_shared<Config>(load());
use(local->timeout);
```
Sáng nay bạn đã giải ở mức *"sửa thế nào"*. Giờ hỏi ngược — **mức chẩn đoán**:

**a)** Thiết bị chạy 3 ngày rồi crash. Backtrace trỏ vào `free()` bên trong `~shared_ptr`, **không liên quan** gì tới `Config`. Bạn lập luận thế nào để lần ngược về đoạn code trên?
**b)** Vì sao lỗi này **hiếm** và **khó tái hiện**? Khoảng thời gian hai thread chen được vào nhau nằm ở đâu — theo các bước mà `g_cfg = ...` phải thực hiện?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** (a) writer đang viết `g_cfg` giữa chừng, reader copy `g_cfg` **chưa hoàn thiện** ra `local` rồi dùng và giải phóng; cả hai cùng giải phóng tài nguyên cũ → **double free** ✅ (b) hiếm vì writer rất ít khi chạy, chủ yếu là read. *"Không hiểu 'cửa sổ đua' là gì"*.

**✅ Lập luận (a) đúng hướng và đúng kết luận** — nhận ra double free và truy được về việc reader đọc `g_cfg` giữa lúc writer đang ghi. (b) nêu đúng lý do thống kê.
**❌ Vì sao 3:** chưa mô tả được **các bước** của phép gán, mà đó là thứ biến "tôi đoán là race" thành "tôi chỉ được chỗ nó hỏng".

*(Lỗi diễn đạt của tôi: "cửa sổ đua" là tự dịch từ "race window", không phải thuật ngữ chuẩn tiếng Việt. Ý là **khoảng thời gian giữa các bước, trong đó thread kia chen vào được**.)*

**Đáp án (b) — `g_cfg = other` KHÔNG phải một thao tác:**

Một `shared_ptr` là **hai con trỏ** (payload + control block). Phép gán gồm:

```
  ① đọc control block CŨ, giảm strong count  (atomic)
  ② ghi con trỏ payload  MỚI                 ← ⚠️ chen được vào đây
  ③ ghi con trỏ control  MỚI                 ← ⚠️ và đây
  ④ tăng strong count của control block MỚI  (atomic)
```

**Từng bước là atomic, nhưng CẢ KHỐI thì không.** Reader chạy `auto local = g_cfg;` chen vào giữa ② và ③ sẽ đọc được:

```
  local.payload → Config MỚI        (đã ghi ở ②)
  local.control → control block CŨ  (chưa kịp ghi ở ③)
```

`local` giờ là một **shared_ptr lai**: đếm tham chiếu của object cũ, nhưng trỏ dữ liệu của object mới. Khi `local` hết scope, nó giảm count của control block **cũ** → count về 0 → gọi deleter → **`delete` object CŨ lần thứ hai** (writer đã xoá rồi) → **double free**.

**Vì sao khớp triệu chứng ở (a):**
- Cửa sổ ② → ③ chỉ vài nhịp lệnh, mà writer chạy **1 lần/giờ** ⟹ xác suất cực thấp ⟹ **3 ngày mới nổ**.
- Bộ nhớ bị hỏng **từ lâu trước khi crash**; chương trình chỉ ngã khi allocator đụng vào metadata hỏng ⟹ **backtrace trỏ vào `free()` ở chỗ không liên quan**. Đây là dấu hiệu kinh điển của **heap corruption**, không phải bug tại chỗ crash.

**Chuỗi lập luận nên trình bày ở phỏng vấn:**
1. Backtrace trong `free()`/`~shared_ptr` mà không dính tới dữ liệu nghiệp vụ ⟹ nghi **heap corruption**, không phải lỗi logic tại đó.
2. Hỏng heap thường do double-free / dùng sau khi giải phóng / ghi tràn.
3. Tìm chỗ nào có **`shared_ptr` toàn cục bị ghi từ một thread trong khi thread khác đọc** ⟹ ra ngay đoạn trên.
4. Xác nhận: chạy dưới **ThreadSanitizer** (`-fsanitize=thread`) — nó bắt được data race **kể cả khi không crash**, không cần đợi 3 ngày.

**Cách sửa (đã bàn phiên 10/08):** C++17 dùng `std::atomic_load/atomic_store` trên `shared_ptr` (deprecated ở C++20); C++20 dùng `std::atomic<std::shared_ptr<T>>`. Không dùng mutex vì reader ở hot path sẽ bị xếp hàng.

**Chốt:** *"`shared_ptr` gồm hai con trỏ nên phép gán có khoảng hở giữa hai lần ghi. Thread kia chen vào đó sẽ cầm một shared_ptr lai — đếm nhầm control block — và bug nổ ở `free()` nhiều ngày sau, cách xa nguyên nhân."*

**Ôn:** [02-modern-cpp/concurrency.md](../../../02-modern-cpp/concurrency.md) · [09-debugging/memory-bugs.md](../../../09-debugging/memory-bugs.md) · bank CPP-052, CPP-024
</details>

---

# 📌 Ghi chú chung

## ⚠️ Lỗ hổng phương pháp — lặp lại lần thứ 2

| Phiên | Sự việc |
|---|---|
| **10/08 câu 6** | Comment mất dòng `unique_ptr` — **chính biến đang đo** — rồi kết luận *"không có vấn đề gì khi compile"* |
| **13/08 câu 12** | Log chỉ đặt ở `ctor(int)`; copy ctor **ngầm** không in gì. Ba dòng log khớp ba object ⟹ kết luận `Timeout c(a)` gọi `ctor(int)`. Thực ra dòng thứ ba là của `wait(a+b)` |

**Cùng một dạng:** kết luận từ phép đo **không đủ sức phân biệt hai giả thuyết**.

**Câu hỏi tự chặn — hỏi TRƯỚC khi kết luận:**
> *"Nếu giả thuyết ngược lại đúng, kết quả tôi vừa thấy có **khác đi** không?"*
> Không khác ⟹ phép đo vô nghĩa, đừng kết luận.

**Công cụ rẻ, dùng ngay:**
```cpp
static_assert(std::is_copy_constructible_v<T>);
static_assert(std::is_move_constructible_v<T>);
static_assert(std::is_nothrow_move_constructible_v<T>);   // kiểm noexcept
```
Trả lời thẳng câu hỏi mà không cần chạy, không cần thêm log, không đụng vào class.

Đây là kỹ năng debug — mảng bạn tự nhận là điểm yếu; xuất hiện **2/2 phiên** nên tách thành mục theo dõi riêng.

## 💬 Góp ý của bạn về chất lượng bank (3 lần trong phiên)

> *"câu trả lời trong bank còn sơ sài, chỉ khẳng định, thiếu code, đọc khó hiểu"* — nêu ở OS-020, SD-016, DP-011.

✅ **Đồng ý.** Đã viết lại **OS-020** và **SD-016** ngay trong phiên. **DP-011 còn nợ.**

**Tiêu chuẩn cho đáp án bank từ nay:** câu 🟠🔴 phải có (1) **cơ chế** giải thích *vì sao*, không chỉ kết luận; (2) **code minh hoạ đã biên dịch được**; (3) **bảng đối chiếu** khi có nhiều phương án; (4) **câu chốt** một dòng.

## Tổng kết

**Điểm mạnh:**
- **CPP-014 (4)** — universal reference: đúng cả hai điều kiện, không cần gợi ý.
- **OS-001 (4)** — tự đưa ra đánh đổi *"2 process cho thiết bị 24/7"*, đúng chất T2.
- **CPP-020 (4)** — nhận ngay dtor "vô hại" giết move; retention qua ở góc hỏi hoàn toàn mới.
- **CPP-032 (b)** — phân tích được *vì sao* bug đơn vị tệ hơn bug bool, không phải ghi nhớ.
- Bị chỉ ra sai thì **gỡ nhanh và tổng quát hoá được** (câu 7, câu 12).

**3 lỗ hổng ưu tiên:**
1. 🔴 **async-signal-safety** (câu 8) — [OS-020](../bank/os.md), [processes-signals.md](../../../04-linux-system-programming/processes-signals.md)
2. 🔴 **Thiết kế phòng ngừa — chặn ở class, không vá ở call site** (câu 6) — [CPP-016](../bank/cpp.md), [oop.md](../../../01-cpp-fundamentals/oop.md)
3. 🟠 **Bộ nhớ tất định + `std::pmr`** (câu 10) — [SD-016](../bank/system-design.md), [constraints.md](../../../08-embedded-systems/constraints.md)

**Cập nhật đã làm:** sổ yếu **2 → 5** (+CPP-016, +OS-020, +SD-016; CPP-032 lên 1/2) · retention CPP-020 (4) và CPP-029 (3) đạt → dời Tuần 4 · bank **+CPP-055**, viết lại **OS-020** + **SD-016** · plan §📍.

**Nợ:** viết lại bank **DP-011** · **câu 16 coding `SocketHandle`** chưa làm.

## Phiên kế đề xuất
- **Buổi sau:** `/mock coding track cpp-system` — `SocketHandle` (CPP-054, sổ yếu) + `ring_buffer_v2` bản **mutex** (COD-006). Làm từ **file trống**, không mở `reviewed/`.
- **Tuần 2 Buổi 1:** `/mock rapid track linux-sysprog` — OS-020 sẽ được chèn lại ở đó (đúng domain).
