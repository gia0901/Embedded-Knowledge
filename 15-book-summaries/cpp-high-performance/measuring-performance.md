# Analyzing & Measuring Performance — *C++ High Performance* ch. 3 (tr. 71–98)

> **Nguồn summary:** đã đối chiếu PDF *C++ High Performance, 2nd Edition* (Andrist & Sehr, Packt 12/2020). **Offset: trang PDF = trang sách + 27.**
> **Vì sao cụm này — và phạm vi CỐ Ý HẸP:** repo đã có `perf` ở [cpp-mindset ch.15](../cpp-mindset/scale-gpu-profiling.md) và [09-debugging/tools §4](../../09-debugging/tools.md). Cụm này **không chép lại** phần đó, mà lấy thứ hai nơi kia đều thiếu: **phương pháp** — cách phát biểu mục tiêu tối ưu, hai loại profiler khác nhau ở bản chất, **định luật Amdahl**, và **bảy cái bẫy của microbenchmark**.

---

## TL;DR cụm này

1. **"Nhanh hơn" là một câu chưa hoàn chỉnh.** Phải nói rõ đang tối ưu **latency hay throughput**, và đo bằng **median hay mean**.
2. **Speedup = T_cũ / T_bốn.** Speedup 2,5× nghĩa là "nhanh hơn 60%" — hai con số khác nhau, đừng lẫn.
3. **Hai loại profiler khác nhau về BẢN CHẤT**, không phải mức độ: instrumentation làm méo cái nó đo; sampling **mù với thread đang ngủ**.
4. **Amdahl's law là liều thuốc chống ảo tưởng.** Làm một hàm nhanh gấp đôi, nhưng hàm đó chỉ chiếm 1% thời gian ⇒ toàn chương trình nhanh lên **1,005×**.
5. **Microbenchmark là biện pháp cuối cùng**, không phải bước đầu tiên — và nó có ít nhất **7 cách nói dối**.

---

## Cụm 1 — Nói cho đúng ta đang đo gì (tr. 77–81)

### Nội dung chính

**Các thuộc tính hiệu năng** (tr. 78–79). Chọn sai thứ để đo thì tối ưu xong vẫn không giải quyết vấn đề:

| Thuộc tính | Nghĩa |
|---|---|
| **Latency / response time** | Thời gian giữa **yêu cầu và phản hồi** của **một** thao tác — vd xử lý xong **một** ảnh |
| **Throughput** | Số thao tác **mỗi đơn vị thời gian** — vd xử lý được bao nhiêu ảnh **mỗi giây** |
| **I/O bound / CPU bound** | Tác vụ nhanh lên nếu **CPU nhanh hơn** ⇒ CPU bound. Nhanh lên nếu **I/O nhanh hơn** ⇒ I/O bound. Còn có **memory bound** |
| **Power consumption** | 🎯 Rất quan trọng với thiết bị chạy pin. Muốn giảm điện ⇒ dùng phần cứng hiệu quả hơn, và **tránh polling tần suất cao** vì nó **chặn CPU vào trạng thái ngủ** |
| **Data aggregation** | **Mean** đôi khi đủ, nhưng **median thường nói nhiều hơn** vì nó **bền với ngoại lai**. Quan tâm ngoại lai thì đo min/max hoặc percentile |

> 🎯 **Dòng về power consumption đáng khoanh tròn cho JD embedded.** Nó nối trực tiếp sang [08/constraints.md](../../08-embedded-systems/constraints.md): một vòng lặp polling "vô hại" về CPU vẫn **giết pin**, vì nó không cho CPU vào idle state. 🆕 Đây cũng là lý do dùng `epoll`/blocking read thay vì poll-and-sleep trên thiết bị chạy pin.

**Latency ≠ throughput, và tối ưu cái này có thể làm hỏng cái kia.** 🆕 Gom lô (batching) làm **throughput tăng** nhưng **latency của item đầu tiên tăng theo**.

**Speedup — công thức và cách phát biểu** (tr. 79–80):

$$\text{Speedup} = \frac{T_{old}}{T_{new}} \qquad\qquad \%\text{Improvement} = 100\left(1 - \frac{1}{\text{Speedup}}\right)$$

Ví dụ của sách: 10 ms → 4 ms.
- **Speedup = 10/4 = 2,5×**
- **% cải thiện = 100 × (1 − 1/2,5) = 60%**

⇒ *"bản mới chạy nhanh hơn 60%, tương ứng speedup 2,5×"*. **Speedup = 1 nghĩa là không cải thiện gì.**

**Performance counter** (tr. 80–81). CPU có bộ đếm phần cứng (**PMC**) cho: số lệnh, chu kỳ CPU, **branch misprediction**, **cache miss**. Công cụ theo nền tảng: Intel VTune, `pmcstat` (FreeBSD), DTrace/Instruments (macOS), Visual Studio (Windows), và **`perf` trên GNU/Linux**.

```
$ perf stat ./my-prog

 1 129,86 msec task-clock                #    1,000 CPUs utilized
          8      context-switches         #    0,007 K/sec
          0      cpu-migrations           #    0,000 K/sec
     97 810      page-faults              #    0,087 M/sec
3 968 043 041    cycles                   #    3,512 GHz
1 250 538 491    stalled-cycles-frontend  #   31,52% frontend cycles idle
  497 225 466    stalled-cycles-backend   #   12,53% backend cycles idle
6 237 037 204    instructions             #    1,57 insn per cycle
                                          #    0,20 stalled cycles per insn
1 853 556 742    branches                 # 1640,516 M/sec
      3 486 026  branch-misses            #    0,19% of all branches
```

🆕 **Cách đọc nhanh output này:**
- **`insn per cycle` (IPC)**: gần 4 là tốt, **dưới 1 là dấu hiệu đang chờ bộ nhớ**. Ở đây 1,57 — trung bình.
- **`stalled-cycles-frontend` 31%**: gần 1/3 thời gian CPU **rỗi vì chưa có lệnh để chạy** (thường do i-cache miss / branch misprediction).
- **`page-faults` 97 810**: cao thì nghi memory bound hoặc thrashing.
- **`branch-misses` 0,19%**: tốt. Trên 5% thì đáng điều tra.

**Bốn thực hành khi test hiệu năng** (tr. 81–82):
1. **Đo sớm**, và đưa test hiệu năng vào **nightly build** để bắt regression — người ta viết regression test cho tính đúng đắn nhưng hiếm khi cho hiệu năng.
2. **Hỏi về cỡ dữ liệu ngay từ đầu dự án.** *"Đừng thử với 100 phần tử rồi hi vọng code sẽ scale — hãy test nó!"*
3. **Chọn thuật toán và cấu trúc dữ liệu cẩn thận nếu phải xử lý input lớn, nhưng ĐỪNG tinh chỉnh code khi chưa có lý do chính đáng.**
4. **Vẽ đồ thị.** *"Đồ thị không cần đẹp mới hữu ích. Vẽ ra rồi thì bạn sẽ thấy các ngoại lai và mẫu hình mà một bảng đầy số thường giấu đi."*

---

## Cụm 2 — Tìm hot spot: hai loại profiler (tr. 82–87) 🎯

### Nội dung chính

**Nguyên lý Pareto (80/20)** (tr. 82): ~20% code chịu trách nhiệm cho ~80% tài nguyên. Sách nói rõ đây chỉ là **quy tắc ngón tay cái, đừng hiểu theo nghĩa đen**, nhưng hệ quả thì đúng và quan trọng:

> Đây thật ra là **tin tốt**: nghĩa là ta có thể viết **phần lớn code mà không cần tinh chỉnh gì**, tập trung giữ code sạch. Nhưng cũng nghĩa là **khi tối ưu, phải biết tối ưu Ở ĐÂU** — nếu không, khả năng cao là ta đang tối ưu đoạn code chẳng ảnh hưởng gì.

**Call graph** — profiler thường in ra ai gọi ai và bao nhiêu lần:
```
main() ──1 lần──► sort() ──50 lần──► swap()
                    (hình theo Figure 3.4)
```

#### Instrumentation profiler

**Nguyên lý:** **chèn code** vào chương trình để ghi lại mỗi lần vào/ra hàm.

`ScopedTimer` — sách nói đây là bản giản lược của class họ dùng thật trong dự án (tr. 84). **Class này được dùng lại trong toàn bộ phép đo ở chương 4 và 7**, nên nó đáng gõ lại:

```cpp
class ScopedTimer {
public:
    using ClockType = std::chrono::steady_clock;     // ⭐ steady, KHÔNG phải system_clock

    ScopedTimer(const char* func) : function_name_{func}, start_{ClockType::now()} {}
    ScopedTimer(const ScopedTimer&) = delete;        // cấm copy/move: nó đo MỘT scope cụ thể
    ScopedTimer(ScopedTimer&&) = delete;
    auto operator=(const ScopedTimer&) -> ScopedTimer& = delete;
    auto operator=(ScopedTimer&&) -> ScopedTimer& = delete;

    ~ScopedTimer() {                                  // RAII: đo từ lúc dựng tới lúc HUỶ
        using namespace std::chrono;
        auto stop = ClockType::now();
        auto ms = duration_cast<milliseconds>(stop - start_).count();
        std::cout << ms << " ms " << function_name_ << '\n';
    }
private:
    const char* function_name_{};
    const ClockType::time_point start_{};
};
```

> ⭐ **Vì sao `steady_clock` chứ không `system_clock`?** Sách nói rõ (tr. 84): `steady_clock` là **đơn điệu (monotonic)** — không bao giờ giảm giữa hai lần gọi `now()`. `system_clock` **có thể bị chỉnh bất cứ lúc nào** (NTP, người dùng đổi giờ) ⇒ đo ra khoảng thời gian **âm** hoặc nhảy cóc.
>
> 🆕 Đây đúng là bài học `CLOCK_MONOTONIC` vs `CLOCK_REALTIME` ở tầng POSIX — cùng một cái bẫy, hai tầng ngôn ngữ.

**Bọc bằng macro để bật/tắt được** (tr. 85) — sách thừa nhận nói chung không khuyến khích macro, *"nhưng đây có thể là một ca đáng dùng"*:

```cpp
#if USE_TIMER
#define MEASURE_FUNCTION() ScopedTimer timer{__func__}
#else
#define MEASURE_FUNCTION()
#endif

auto some_function() {
    MEASURE_FUNCTION();
    // ...
}                       // → in ra "2.3 ms some_function" khi hàm trả về
```

`__func__` có từ C++11. C++20 thêm `std::source_location` với `function_name()`, `file_name()`, `line()`, `column()`. Chưa có thì dùng `__FUNCTION__`, `__FILE__`, `__LINE__`.

**Hai nhược điểm của instrumentation** (tr. 83):
1. **Code chèn vào ảnh hưởng chính thứ đang đo** ⇒ profile có thể **sai lệch**.
2. Nó có thể **chặn tối ưu của compiler**, hoặc ngược lại **bị compiler tối ưu mất**.

#### Sampling profiler

**Nguyên lý:** **không sửa chương trình**. Cứ mỗi khoảng đều đặn (điển hình **10 ms**), nhìn xem **call stack đang là gì**.

```
thời gian →   t1  t2  t3  t4  t5  t6  t7  t8  t9  t10
main()      ├───────────────────────────────────────┤
  f1()          ├───────────────┤
    f4()             ├──┤            ← CHẠY nhưng KHÔNG bị lấy mẫu lần nào
  f2()                        ├──────────────┤
    f3()                          ├────┤  ├──┤
                                        (hình theo Figure 3.5)
```

Kết quả tổng hợp (Table 3.3):

| Hàm | **Total** | **Self** |
|---|---|---|
| `main()` | 100% | 10% |
| `f1()` | 80% | 10% |
| `f2()` | 70% | 30% |
| `f3()` | 50% | **50%** |
| `f4()` | **0%** | **0%** |

- **Total** = % số mẫu mà hàm đó **xuất hiện ở đâu đó trong call stack**.
- **Self** = % số mẫu mà hàm đó **nằm trên ĐỈNH stack** (đang thực sự chạy).

⇒ **`f3()` có Self cao nhất (50%) — đó mới là hot spot thật**, dù Total của nó chỉ 50% còn `main()` là 100%.

**Hai giới hạn phải biết** (tr. 86):

**① Sampling profiler MÙ với thread đang ngủ.** Nguyên văn: *"pure sampling profiler thường chỉ phát hiện các hàm đang được thực thi trong thread ở trạng thái running, vì thread đang ngủ không được lên lịch trên CPU. Nghĩa là nếu một hàm đang **chờ lock** khiến thread ngủ, thời gian đó **sẽ không xuất hiện trong time profile**."*

> 🎯 **Đây là câu quan trọng nhất của cả chương.** Nút thắt cổ chai do **đồng bộ hoá** (mutex contention, chờ I/O) là loại vấn đề **vô hình** với sampling profiler. Chương trình chậm gấp 10 mà profiler báo "CPU rỗi" ⇒ nghi ngay chuyện chờ, không phải chuyện tính.

**② Hàm ngắn và ít gọi sẽ biến mất.** `f4()` chạy thật, giữa mẫu 2 và 3, nhưng **không lọt vào mẫu nào** ⇒ Total = 0%. Sách nói thường không sao — *"những hàm này hiếm khi là hàm bạn cần tinh chỉnh"* — nhưng phải **biết** nó có thể xảy ra.

**Ưu điểm bù lại:** tác động tối thiểu lên hiệu năng thật, và **build được ở release mode với đầy đủ tối ưu**.

`gprof` là ví dụ của **kiểu lai** cả hai.

### Góc interview

**Q1. Profiler báo chương trình chỉ dùng 5% CPU, nhưng nó chậm gấp 10 lần mong đợi. Bạn nghi gì?**
<details><summary>Đáp án</summary>

**Nghi thời gian CHỜ, không phải thời gian TÍNH** — và biết rằng **sampling profiler mù với chuyện chờ**.

**Cơ chế:** sampling profiler lấy mẫu call stack của thread **đang chạy trên CPU**. Thread đang **ngủ** (chờ mutex, chờ I/O, chờ condition variable, chờ syscall) **không được lên lịch** ⇒ **không xuất hiện trong profile**. Sách nói thẳng (tr. 86): *"nếu một hàm đang chờ lock khiến thread ngủ, thời gian đó sẽ không xuất hiện trong time profile."*

**Bốn nguyên nhân theo thứ tự nên kiểm:**
1. **Lock contention** — nhiều thread tranh một mutex.
2. **I/O bound** — chờ đĩa/mạng/thiết bị.
3. **Page fault / thrashing** — chờ nạp trang từ đĩa.
4. **Ngủ tường minh** — `sleep`, timeout, polling có delay.

**Công cụ đúng cho từng loại** (không phải sampling profiler):
- `perf stat` → nhìn **`context-switches`** (cao ⇒ tranh chấp/chờ) và **`page-faults`**.
- `strace -c -p <pid>` → syscall nào tốn thời gian nhất; `futex` chiếm ưu thế ⇒ contention.
- `/proc/<pid>/status` → **`State: D`** (uninterruptible sleep) ⇒ đang kẹt I/O; **`voluntary_ctxt_switches`** cao ⇒ tự nguyện ngủ (chờ).
- `/proc/<pid>/wchan` → **hàm kernel** mà thread đang ngủ trong đó — chỉ thẳng vào nguyên nhân.
- **Off-CPU profiling** (`perf sched`, eBPF) — loại profiler sinh ra chính xác cho bài toán này.

**Câu chốt:** *"CPU 5% mà chậm nghĩa là chương trình đang CHỜ chứ không phải đang TÍNH. Sampling profiler đo cái đang chạy, nên tôi phải đổi công cụ chứ không phải đọc kỹ hơn cùng một profile."*

Liên hệ [09-debugging/tools](../../09-debugging/tools.md), lab [DBG-034](../../14-prep/mock-interview/bank/debugging.md).
</details>

**Q2. Instrumentation profiler và sampling profiler khác nhau ở đâu? Khi nào dùng cái nào?**
<details><summary>Đáp án</summary>

**Khác về BẢN CHẤT cách lấy dữ liệu, không phải mức độ chi tiết.**

| | **Instrumentation** | **Sampling** |
|---|---|---|
| Cách làm | **Chèn code** ghi lại mỗi lần vào/ra hàm | **Không sửa** chương trình; chụp call stack mỗi ~10 ms |
| Độ chính xác | **Đếm chính xác** số lần gọi | **Thống kê** — gần đúng |
| Tác động | **Làm méo cái nó đo**; có thể chặn inline | **Tối thiểu** |
| Build | Thường cần build riêng | **Chạy được release build đủ tối ưu** |
| Hàm ngắn/hiếm gọi | Bắt được | **Có thể biến mất hoàn toàn** |
| Thread đang ngủ | Bắt được (nếu bọc đúng chỗ) | **MÙ** |

**Chọn thế nào:**
- **Sampling trước** — mặc định. Không xâm lấn, chạy được trên build release, đủ để tìm ra 20% code chiếm 80% thời gian. `perf record` / VTune / Instruments.
- **Instrumentation sau**, khi đã khoanh vùng và cần **con số chính xác** cho một hàm cụ thể — `ScopedTimer` kiểu RAII là đủ.
- **Cả hai đều mù với chuyện chờ** ở mức độ khác nhau ⇒ nghi contention thì phải dùng công cụ khác (`perf sched`, eBPF, `strace -c`).

**Bẫy của instrumentation tự viết:** đo bằng `system_clock` thay vì **`steady_clock`**. `system_clock` bị NTP/người dùng chỉnh ⇒ ra khoảng thời gian **âm**. Luôn dùng clock **đơn điệu** để đo khoảng.

**Nâng lên T2 — cách đọc bảng của sampling profiler:** phân biệt **Total** (hàm có mặt trong call stack) và **Self** (hàm ở **trên đỉnh** stack, đang thực sự chạy). **`main()` luôn có Total 100% nhưng vô nghĩa; hot spot thật là hàm có Self cao nhất.**
</details>

---

## Cụm 3 — Microbenchmark & định luật Amdahl (tr. 87–98) 🎯

### Nội dung chính

**Quy trình 4 bước** (tr. 87–88):

```
   ┌─────────────────────────────────────────────────────────┐
   │  1. Tìm hot spot cần chỉnh — TỐT NHẤT là bằng profiler   │
   │  2. Tách nó ra thành microbenchmark ĐỘC LẬP              │
   │  3. Tối ưu, dùng framework benchmark để đo mỗi bước       │
   │  4. Ghép lại vào chương trình và ĐO LẠI trong ngữ cảnh   │
   │     thật với dữ liệu thật                                 │
   └─────────────────────────────────────────────────────────┘
        Bước 4 là bước hay bị bỏ nhất — và là bước quyết định
                                       (hình theo Figure 3.6)
```

**Hai câu phải tự hỏi TRƯỚC khi bắt đầu** (tr. 88):
1. Thời gian nằm trong hàm này có **thật sự ảnh hưởng đáng kể** tới hiệu năng tổng thể không? *(⇒ profiler + Amdahl)*
2. Có thể **giảm số lần GỌI** hàm này không? — *"loại bỏ các lời gọi tới hàm đắt tiền thường là cách tối ưu hiệu quả nhất."*

> **Câu chốt của sách:** *"Tối ưu code bằng microbenchmark thường nên được xem là **biện pháp cuối cùng**. Mức tăng hiệu năng tổng thể kỳ vọng thường nhỏ."*

#### Định luật Amdahl

$$\text{Overall speedup} = \frac{1}{(1 - p) + \dfrac{p}{s}}$$

- **p** = tỉ lệ thời gian mà phần được tối ưu **chiếm trong tổng thời gian**
- **s** = speedup đạt được **trong riêng phần đó**

**Trực giác qua ba ca cực đoan** (Table 3.4):

| p | s | Overall speedup | Ý nghĩa |
|---|---|---|---|
| 0 | 5× | **1×** | Tối ưu phần **không chiếm thời gian nào** ⇒ vô ích, dù s bao nhiêu |
| 1 | 5× | **5×** | Tối ưu phần chiếm **toàn bộ** thời gian ⇒ được đúng bằng s |
| 0,5 | **∞** | **2×** | **Xoá sạch** phần chiếm nửa thời gian ⇒ **trần là 2×**, không hơn |

**Ví dụ đầy đủ của sách** (tr. 90) — con số này nên thuộc:

> Tối ưu một hàm **nhanh gấp đôi** (s = 2), hàm đó chiếm **1%** tổng thời gian (p = 0,01):
> $$\text{Overall} = \frac{1}{(1-0{,}01) + \frac{0{,}01}{2}} = \frac{1}{0{,}99 + 0{,}005} = \mathbf{1{,}005}$$
> **Toàn chương trình nhanh lên 0,5%.**

Bình luận của sách: *"không phải nói rằng speedup này nhất thiết là không đáng kể, nhưng ta phải liên tục quay lại nhìn thành quả **trong tỉ lệ với bức tranh lớn**."*

🆕 **Hệ quả ngược, hữu ích hơn:** Amdahl cũng cho ta biết **trần** của việc tối ưu. Phần **không tối ưu được** chiếm 90% thời gian ⇒ dù phần còn lại nhanh vô hạn, tổng thể **không thể vượt 1,11×**. Biết trần trước khi bỏ công là cách tiết kiệm thời gian tốt nhất.

#### Bảy cái bẫy của microbenchmark (tr. 90–91)

| # | Bẫy | Vì sao nguy hiểm |
|---|---|---|
| **1** | **Kết quả bị tổng quát hoá quá đà**, coi như chân lý phổ quát | Đo trên một máy, một compiler, một cỡ dữ liệu |
| **2** | **Compiler tối ưu code cô lập KHÁC với trong chương trình đầy đủ** | Hàm được **inline** trong benchmark nhưng không inline trong bản thật; hoặc compiler **tính trước** được một phần |
| **3** | **Giá trị trả về không dùng ⇒ compiler XOÁ luôn hàm cần đo** | Benchmark đo được 0 ns cho một hàm không hề chạy |
| **4** | **Dữ liệu test tĩnh cho compiler lợi thế phi thực tế** | Hardcode số vòng lặp là bội của 8 ⇒ compiler **vectorize** bỏ qua prologue/epilogue. Thay bằng giá trị runtime là tối ưu đó **biến mất** |
| **5** | **Dữ liệu test phi thực tế làm sai branch prediction** | Dữ liệu quá đều ⇒ nhánh đoán đúng 100%, thực tế thì không |
| **6** | **Nhiễu giữa các lần đo** | frequency scaling, cache pollution, process khác được lên lịch |
| **7** | **Nút thắt thật là CACHE MISS, không phải thời gian chạy lệnh** | *"trong nhiều kịch bản, một luật quan trọng của microbenchmarking là bạn **phải xoá sạch cache trước khi đo**, nếu không bạn chẳng đo được gì cả"* |

> Sách thừa nhận thẳng: *"Tôi ước mình có một công thức đơn giản để tránh mọi cái bẫy trên, nhưng tiếc là không."* Cách giảm thiểu: dùng **thư viện benchmark** (Google Benchmark) — nó lo việc lặp đủ số lần, chống tối ưu-mất, và báo phương sai.

### Insight đáng nhớ

- **Thứ tự ưu tiên của cả chương, phát biểu ngược lại cho dễ nhớ:** trước khi tinh chỉnh một hàm, hãy hỏi (1) *có gọi nó ít đi được không?* (2) *nó có chiếm đủ tỉ lệ thời gian để đáng công không?* — chỉ khi cả hai đều "không giúp được" thì mới microbenchmark.
- **Bẫy 3 và 4 là lý do các thư viện benchmark tồn tại.** Chúng cung cấp `benchmark::DoNotOptimize()` để ép compiler giữ lại kết quả — thứ không thể tự làm đúng bằng tay.
- 🆕 **Bẫy 7 gắn thẳng với chương 4:** nếu bottleneck là cache miss thì microbenchmark chạy trên dữ liệu bé xíu đã nằm sẵn trong L1 sẽ cho kết quả **đẹp và vô nghĩa**.

### Góc interview

**Q3. Bạn tối ưu một hàm nhanh gấp đôi nhưng chương trình gần như không nhanh lên. Vì sao, và lẽ ra phải làm gì trước?**
<details><summary>Đáp án</summary>

**Vì hàm đó chiếm tỉ lệ nhỏ trong tổng thời gian — định luật Amdahl.**

$$\text{Overall speedup} = \frac{1}{(1-p) + \frac{p}{s}}$$

với **p** = tỉ lệ thời gian phần đó chiếm, **s** = speedup cục bộ.

Ví dụ của sách: **s = 2, p = 0,01** ⇒ overall = 1/(0,99 + 0,005) = **1,005** — toàn chương trình nhanh lên **0,5%**.

**Lẽ ra phải làm gì trước — theo đúng thứ tự:**

**① Chạy profiler để tìm hot spot thật.** Không có profiler thì đang đoán, và Pareto nói ~20% code chiếm ~80% tài nguyên — xác suất đoán trúng thấp.

**② Hỏi "có gọi nó ít đi được không?" trước khi hỏi "có làm nó nhanh hơn không?"** Sách nói rõ: *"loại bỏ các lời gọi tới hàm đắt tiền thường là cách tối ưu hiệu quả nhất"*. Cache kết quả, gộp lời gọi, hoặc bỏ hẳn việc — đều thắng việc tinh chỉnh hàm.

**③ Dùng Amdahl để tính TRẦN trước khi bỏ công.** Nếu p = 0,01 thì dù s = ∞, trần chỉ là 1,01×. Biết trước con số này là biết ngay nên hay không nên làm.

**④ Kiểm lại xem có phải bottleneck ở chỗ khác không** — thuật toán sai (ch. 5), cấu trúc dữ liệu sai/cache-unfriendly (ch. 4), hay tranh chấp khoá (ch. 11). Sách xếp ba thứ này **trước** microbenchmark: *"những vấn đề lớn hơn này phải được xử lý trước."*

**Câu chốt:** *"Microbenchmark là biện pháp cuối cùng, không phải bước đầu tiên. Trước nó là: đo xem thời gian nằm ở đâu, rồi tìm cách làm ít việc đi, rồi mới tìm cách làm nhanh hơn."*
</details>

**Q4. Microbenchmark báo hàm của bạn chạy 0 ns. Chuyện gì xảy ra?**
<details><summary>Đáp án</summary>

**Compiler đã XOÁ hoàn toàn lời gọi hàm** — bẫy số 3 trong danh sách của sách.

**Cơ chế:** giá trị trả về không được dùng, hàm không có side effect quan sát được ⇒ theo **as-if rule**, compiler được phép loại bỏ toàn bộ. Ở `-O2` việc này gần như chắc chắn xảy ra.

```cpp
for (int i = 0; i < N; ++i) {
    compute(data);          // ✗ kết quả bị vứt ⇒ có thể bị xoá sạch
}
```

**Cách chữa:**
```cpp
for (auto _ : state) {
    benchmark::DoNotOptimize(compute(data));   // ép compiler giữ lại kết quả
    benchmark::ClobberMemory();                // ép flush ghi nhớ ra bộ nhớ
}
```
Tự làm tay thì cộng dồn kết quả vào một biến `volatile`, hoặc in ra sau vòng lặp.

**Cách phát hiện sớm:** con số **quá đẹp** (0 ns, hoặc nhanh hơn một lần truy cập RAM ~100 ns cho việc rõ ràng phải chạm bộ nhớ) là dấu hiệu. Kiểm chắc chắn bằng cách **đọc assembly** (`-S`, hoặc godbolt) xem lời gọi còn không.

**Ba bẫy anh em cùng họ, nên nêu luôn:**
- **Dữ liệu test tĩnh** cho compiler lợi thế phi thực tế — hardcode số vòng lặp là bội của 8 ⇒ vectorize bỏ qua prologue/epilogue; thay bằng giá trị runtime là tối ưu đó biến mất.
- **Inline khác nhau** giữa benchmark cô lập và chương trình thật.
- **Không xoá cache trước khi đo** — nếu bottleneck thật là cache miss, benchmark chạy trên dữ liệu đã nằm sẵn trong L1 cho kết quả **đẹp và vô nghĩa**.

**Câu chốt:** *"Benchmark nói dối theo hướng có lợi. Con số quá đẹp thì phải nghi trước, và bước cuối luôn là ghép lại vào chương trình thật rồi đo lại."*
</details>

---

## Đọc thêm (tuỳ chọn — nội dung trên đã tự chứa)

- [cpp-mindset/scale-gpu-profiling.md](../cpp-mindset/scale-gpu-profiling.md) — **`perf` chi tiết** (cụm này cố ý không lặp)
- [09-debugging/tools.md](../../09-debugging/tools.md) — `perf`, `strace`, `/proc` ở góc debug
- [data-structures-cache.md](data-structures-cache.md) — `ScopedTimer` trên được dùng cho mọi phép đo ở đó
- [08/constraints.md](../../08-embedded-systems/constraints.md) — power consumption & tính tất định
