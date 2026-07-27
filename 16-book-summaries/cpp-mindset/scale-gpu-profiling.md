# C++ Mindset — Ch. 13–15: Scale, GPU & Profiling (tr. 285–352) ✅ đã đọc

> Thuộc [The C++ Programmer's Mindset](README.md) · **[⏮ Ch. 7–12 — Dự án Rubber Duckies](project-rubber-duckies.md)** · **⏭ (hết sách)**
> Nguồn: đọc trực tiếp PDF, **trang sách = trang PDF − 25**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Ba chương khép sách — "khi một máy/một core không đủ".** Ch. 13: mở rộng tính toán (thread → process/MPI → cloud) và tìm bottleneck. Ch. 14: đẩy việc xuống GPU/phần cứng chuyên dụng (**lướt** — sách tự nói không phải giáo trình GPU). Ch. 15: **profiling với `perf`** — chương **đầu tư kỹ nhất** vì trúng thẳng điểm yếu "debug/đo hiệu năng" của bạn (↔ [09/tools.md](../../09-debugging/tools.md)).

---

## Cụm 1 — Ch. 13: The Problems of Scale (tr. 285–303)

### Nội dung chính

**Thang mở rộng:** một core → nhiều core (thread) → nhiều process/máy (MPI) → cloud. Mỗi nấc thêm chi phí giao tiếp và một lớp đồng bộ mới.

**(a) Bộ công cụ thread (tr. 288–291)** — chọn primitive theo nhu cầu:

- **`std::mutex` bảo vệ dữ liệu chung.** Ví dụ đắt của sách: **cache kết quả tính toán tốn kém** bằng `recursive_mutex`, kỳ vọng *"most calls will result in a cache hit and thus a very fast return path"* (tr. 285) → mutex hiếm khi bị giữ lâu:
```cpp
class Cache {
    mutable std::recursive_mutex lock_;
    // ... map lưu kết quả đã tính ...
    Result get(Key k) const {
        std::lock_guard g(lock_);      // đa số lần: hit → trả nhanh
        if (auto it = cache_.find(k); it != cache_.end()) return it->second;
        return cache_[k] = expensive_compute(k);   // miss: tính rồi lưu
    }
};
```
- **`std::condition_variable` — chờ một điều kiện** (không busy-wait): `wait(lock, predicate)` ngủ đến khi predicate đúng; nối [OSTEP CV](../ostep/concurrency.md).
- **`std::barrier` (C++20) — điểm hẹn (tr. 290):** mọi worker phải tới cùng điểm mới đi tiếp — đúng cho **thuật toán lặp** (bước sau dùng toàn bộ kết quả bước trước, như k-means/simulation): `sync.arrive_and_wait()` mỗi vòng, tránh race giữa các iteration.
- **Thread pool:** thay vì tự tạo thread, submit task vào pool (kiểm soát số thread — nối `-j` của [dự án](project-rubber-duckies.md)). 🆕 C++20 `std::jthread` (auto-join + stop_token) là mặc định hiện đại hơn `std::thread`.

**(b) Phân tán với MPI (tr. 292–296):** khi nhiều core một máy không đủ → nhiều process trên nhiều máy. **MPI** (OpenMPI/MSMPI) là chuẩn: mỗi node có **rank** (0..N−1), giao tiếp bằng `MPI_Send`/`MPI_Recv`, gộp bằng `MPI_Reduce` (vd `MPI_SUM` về root):
```cpp
MPI_Init(&argc, &argv);
int rank, n_nodes;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // tôi là node số mấy
MPI_Comm_size(MPI_COMM_WORLD, &n_nodes);
// ... tính phần của mình ...
MPI_Reduce(out.data(), reduced.data(), out.size(), MPI_INT, MPI_SUM, /*root=*/0, MPI_COMM_WORLD);
MPI_Finalize();
```
⚠️ **Object phức tạp phải serialize (tr. 296):** MPI gửi được kiểu cơ bản/mảng dễ dàng, nhưng class có Pimpl/con trỏ phải **serialize → bytes → deserialize** ở đầu kia. Sách nhân đó dạy **endianness**: đa số CPU little-endian (byte thấp trước) → gửi binary qua mạng phải thống nhất byte order (network byte order):
```cpp
int num = 0x1A2B3C4D;
std::array<std::byte, 4> b{};
std::memcpy(b.data(), &num, 4);   // in ra: 4D 3C 2B 1A trên máy little-endian
```

**(c) Hiểu bottleneck (tr. 299–300):** *"Many bottlenecks occur from moving data around"* (tr. 299) — cùng bài học Ch. 4: RAM→register đã chậm hơn tính toán, **mạng còn chậm hơn nhiều bậc**. Ba loại:
- **Data-movement**: MPI *bắt buộc tường minh* việc chuyển data → dễ thấy chỗ nghẽn (điểm cộng của mô hình explicit).
- **Computation / load imbalance**: chia việc không đều → một gói chạy lâu làm cả hệ **stall** (ví dụ mô phỏng khí động học: vùng hình học phức tạp tính lâu, nhiều node idle chờ). Giảm nhẹ: **khởi động task lâu sớm nhất có thể**, đủ việc khác lấp chỗ trống.
- **External / contention**: đĩa, service ngoài, và **tranh chấp tài nguyên chung** (kể cả mạng nối các node) — ưu điểm hệ nhiều node là mỗi node **tiến độc lập** *nếu* ít cần giao tiếp.
- **Cloud (tr. 289):** cân nhắc riêng — không mặc định nuốt hết core (server trăm core ≠ laptop), chi phí khởi động, tính co giãn.

### Insight đáng nhớ

- **Chọn primitive theo bài toán, không theo thói quen:** mutex (bảo vệ dữ liệu) / condition_variable (chờ điều kiện) / barrier (đồng bộ theo pha, thuật toán lặp) / thread pool (kiểm soát số thread). Kể được *khi nào dùng barrier vs condition_variable* là dấu hiệu hiểu thật.
- **Bottleneck của scale = di chuyển dữ liệu**, không phải tính toán — cùng một chân lý xuyên suốt sách (Ch. 4 nói ở tầng cache, Ch. 13 nói ở tầng mạng). Thiết kế phân tán = **tối thiểu hoá giao tiếp**, cho node chạy độc lập nhất có thể.
- **Serialization + endianness** là vấn đề thật khi gửi object qua mạng/lưu đĩa — nối thẳng nghề: protocol nhị phân, ABI qua boundary ([modularity.md](modularity.md)).

### Góc interview

**Câu 1 (🎯):** Chương trình đa luồng của bạn không nhanh hơn khi thêm core. Các nguyên nhân và cách khoanh vùng?

<details><summary>Đáp án</summary>

- **Phần tuần tự (Amdahl):** speedup bị chặn bởi tỉ lệ code chạy tuần tự — nếu 20% tuần tự thì trần speedup là 5× dù vô hạn core. Đo tỉ lệ song song hoá được trước.
- **Contention trên tài nguyên chung:** mutex nóng (nhiều thread tranh một lock — serialize lại), **false sharing** (hai biến khác nhau chung cache line → cache ping-pong giữa core, Ch. 4), cấp phát bộ nhớ chung (malloc lock). Khoanh bằng profiler (perf: `cache-misses`, thời gian trong lock).
- **Load imbalance:** chia việc không đều → core idle chờ core chậm. Giảm bằng chia nhỏ hơn/dynamic scheduling, khởi động task lâu sớm.
- **Bottleneck data-movement / bandwidth:** bài toán memory-bound thì thêm core không giúp (băng thông RAM bão hoà) — phải giảm data movement (blocking, SoA), không phải thêm thread.
- **Oversubscription:** nhiều thread hơn core → context switch ăn lợi ích (cho chỉnh số thread — `-j`, `OMP_NUM_THREADS`).
- Cách làm chuẩn: **đo bằng perf/profiler** (Cụm 3) thay vì đoán — xác định memory-bound hay compute-bound, lock nào nóng, core nào idle.

</details>

---

## Cụm 2 — Ch. 14: GPU & phần cứng chuyên dụng (tr. 305–329) — lướt

### Nội dung chính

⚠️ Sách tự giới hạn: *"This chapter is not intended to be a fully informative discussion of how to program a GPU"* (tr. 305) → file này **chỉ lấy bức tranh + tiêu chí quyết định**, không đi sâu CUDA.

- **Vì sao GPU (tr. 305):** khi CPU đa dụng không đủ throughput → GPU cho **throughput compute cực cao + băng thông bộ nhớ lớn**, hợp bài toán **data-parallel** (cùng một phép tính trên rất nhiều phần tử — SIMT). Đổi lại: latency mỗi thao tác cao, kém với code rẽ nhánh nhiều/tuần tự.
- **Các cách viết code cho GPU (tr. 306):**
  - **CUDA C++** (NVIDIA) — phổ biến nhất, `__global__` kernel + quản lý host/device memory tường minh;
  - **Thrust** — thư viện thuật toán kiểu STL chạy trên GPU (transform/reduce/sort) — nâng tầm trừu tượng;
  - **OpenMP offloading** — `#pragma omp target` đẩy vòng lặp xuống device;
  - **SYCL** — chuẩn mở, một mã nguồn chạy **nhiều loại device** (GPU nhiều hãng, CPU, FPGA).
- **⚠️ Chi phí quyết định = truyền host↔device:** dữ liệu phải copy CPU→GPU rồi kết quả về — với bài nhỏ, **thời gian copy lấn át** thời gian tính. GPU chỉ đáng khi khối tính đủ lớn và **data-parallel**, giảm thiểu qua-lại.

### Insight đáng nhớ

- **GPU là bài toán throughput-vs-latency + data-movement**, cùng khung tư duy Ch. 4/13: chỉ thắng khi công việc **song song ồ ạt trên nhiều dữ liệu** và **chi phí truyền host↔device được khấu hao**. Câu hỏi quyết định: "phần này có data-parallel không, và khối đủ lớn để bù copy không?"
- 🆕 Với embedded: nhiều SoC có GPU/NPU/DSP — cùng nguyên tắc (offload việc data-parallel, đo chi phí truyền); SYCL/OpenCL là đường portable khi không khoá vào CUDA.

---

## Cụm 3 — Ch. 15: Profiling Your Code với `perf` (tr. 331–352) 🎯

### Nội dung chính

Triết lý: **"đo, đừng đoán"** — trước khi tối ưu phải biết *chỗ nào* chậm và *vì sao* (compute-bound hay memory-bound?).

**(a) Micro-benchmark đúng cách — Google Benchmark (tr. 334):**
```cpp
static void bench_saxpy(benchmark::State& state) {
    std::vector<float> x(N, 1.0f), y(N, 2.0f);   // setup NGOÀI vòng đo
    for (auto _ : state) {                        // vòng lặp được benchmark
        saxpy(a, x, y);
        benchmark::DoNotOptimize(y.data());       // cấm optimizer xoá code "vô dụng"
        benchmark::ClobberMemory();               // fence: ép mọi ghi pending hoàn tất
    }
}
BENCHMARK(bench_saxpy);
```
- **`DoNotOptimize`**: inline asm giữ biến/kết quả lại để compiler **không loại bỏ** phép tính vì không ai dùng — *"we always want to benchmark our code with all of the optimizations turned on"* (tr. 334). Đây là bẫy số 1: benchmark code đã bị optimizer xoá.
- **`ClobberMemory`**: chèn fence ép ghi bộ nhớ hoàn tất (cache có thể trì hoãn) — quan trọng khi đo hàm thao tác khối nhỏ.
- ⚠️ **Bẫy benchmark**: cùng buffer lặp lại → luôn trong TLB/cache → số đẹp giả (không đo được chi phí allocation/cache-miss thật) — phải hỏi "điều mình đo có phản ánh workflow thật không?" (vd đo `push_back` thì allocation mới là điểm chính).

**(b) `perf` — bộ ba stat / record / report (tr. 335–340):**

- **Chuẩn bị:** `sudo sysctl -w kernel.perf_event_paranoid=2` (cấp quyền đọc performance counter); **build Release** (tối ưu như production); **`-fno-omit-frame-pointer`** để dựng lại call graph (compiler thường tái dụng frame-pointer register — mất nó là mất call stack; cờ này *"won't hurt performance in most cases"*, tr. 340, áp cho cả static/shared lib).

- **`perf stat -d ./bench`** — đọc **counter phần cứng** của cả tiến trình: `instructions`, `cycles` (→ **IPC**), **`cache-misses`**, **`branch-misses`**, `LLC-load-misses`… → phân loại **compute-bound** (IPC cao, ít miss) vs **memory-bound** (nhiều cache-miss) vs **branch-bound** (nhiều branch-miss):
```bash
perf stat -d  ./bench_saxpy --benchmark_filter="avx512"     # counter tổng quan
perf stat -dd ./bench_outer_product --benchmark_filter=4096 # -dd: thêm counter cache chi tiết
```
- **`perf record -g ./bench`** → ghi **sampling** (lấy mẫu định kỳ vị trí PC + call stack) vào `perf.data`; **`perf report`** mở ra xem **hàm nào nóng** + cây gọi. ⚠️ Sampling **không chính xác tuyệt đối** (có độ trễ giữa sự kiện và lúc lấy mẫu) — dùng để tìm hotspot, không phải đếm chính xác.

**(c) Quy trình đọc (tr. 340+):** `perf stat` khoanh **loại** bottleneck (miss cache? miss branch? IPC thấp?) → `perf record/report` chỉ **hàm/dòng** → sửa (blocking cache, SoA, bỏ nhánh, SIMD — Ch. 4) → **đo lại**. Ví dụ sách: xác nhận nghi ngờ về cache bằng `perf stat -dd` trên bench outer-product.

### Insight đáng nhớ

- **Hai câu hỏi, hai công cụ:** *"loại bottleneck gì?"* → `perf stat` (counter: cache-miss / branch-miss / IPC); *"ở hàm/dòng nào?"* → `perf record` + `report` (sampling + call graph). Trả lời phỏng vấn hiệu năng theo cặp này là đúng nghề.
- **Benchmark sai dễ hơn đúng:** phải bật tối ưu + `DoNotOptimize`/`ClobberMemory` để optimizer không xoá code, và cảnh giác cache/TLB "nóng sẵn" cho số đẹp giả. *Đo cái mình tưởng đang đo.*
- **`-fno-omit-frame-pointer` + Release** là cấu hình profiling chuẩn — thiếu frame pointer thì call graph vô dụng; profile bản Debug thì đo nhầm code không tối ưu.
- 🆕 Nối repo: `perf` ở đây bổ sung góc **benchmark một hàm** cho [09/tools.md](../../09-debugging/tools.md); counter cache-miss/branch-miss là bằng chứng định lượng cho các tối ưu machine-sympathy ở [understanding-the-machine.md](understanding-the-machine.md).

### Góc interview

**Câu 1 (🎯):** Một hàm chậm nhưng bạn không biết vì sao. Quy trình dùng `perf` để chẩn đoán?

<details><summary>Đáp án</summary>

- **Chuẩn bị đúng:** build **Release** (tối ưu như production) + **`-fno-omit-frame-pointer`** (để có call graph); nếu benchmark riêng thì dùng Google Benchmark với `DoNotOptimize`/`ClobberMemory` để optimizer không xoá code cần đo; `kernel.perf_event_paranoid` đủ thấp để đọc counter.
- **Bước 1 — phân loại (`perf stat -d`):** đọc `cache-misses`, `branch-misses`, IPC (instructions/cycles). IPC thấp + nhiều cache-miss → **memory-bound** (sửa: cache blocking, SoA, prefetch); nhiều branch-miss → **branch-bound** (sửa: branchless/`std::clamp`, sắp xếp dữ liệu); IPC cao ít miss nhưng vẫn chậm → **compute-bound** (sửa: thuật toán tốt hơn, SIMD).
- **Bước 2 — định vị (`perf record -g` + `perf report`):** tìm **hàm/dòng nóng** và cây gọi dẫn tới nó (lưu ý sampling gần đúng, tìm hotspot chứ không đếm chính xác).
- **Bước 3 — sửa rồi ĐO LẠI:** đối chiếu counter trước/sau để xác nhận (vd cache-miss giảm sau khi blocking). Lặp đến khi đạt mục tiêu.
- Điểm cộng: nêu "đo, đừng đoán" + phân biệt memory-bound vs compute-bound bằng **con số counter** (không phải cảm giác); và profile phải dưới **workload đại diện** (cache nóng sẵn cho số sai).

</details>

**Câu 2:** Vì sao benchmark một hàm dễ cho kết quả sai? Kể vài bẫy.

<details><summary>Đáp án</summary>

- **Optimizer xoá code:** nếu kết quả không được dùng, compiler loại bỏ cả phép tính → benchmark đo "không làm gì". Chống bằng `benchmark::DoNotOptimize(result)` (giữ biến "sống") — và luôn benchmark với **tối ưu bật** (Release).
- **Ghi bị trì hoãn bởi cache:** với hàm thao tác khối nhỏ, ghi có thể chưa hoàn tất khi vòng đo kết thúc → dùng `ClobberMemory()` (fence) để ép hoàn tất.
- **Cache/TLB nóng sẵn:** lặp trên cùng buffer khiến trang luôn trong TLB, cache luôn ấm → số nhanh giả, không phản ánh lần gọi "nguội" thực tế; nếu allocation là điểm chính (vd `push_back`) thì phải đưa nó vào vòng đo.
- **Warmup / nhiễu hệ thống:** lần đầu chậm (cache lạnh, lazy page fault), CPU frequency scaling/turbo dao động → cần nhiều lần lặp + môi trường ổn định (pin tần số, isolcpus cho đo nghiêm túc).
- **Không đại diện workload:** dữ liệu đều/nhỏ cho kết quả khác dữ liệu thật (branch predictor học được pattern giả) — đo trên dữ liệu giống production.

</details>

---

## Tổng kết Phần 3 & cả cuốn

Ba chương này khép vòng "machine sympathy" của sách: khi một core không đủ → **scale** (thread → MPI → cloud), khi CPU không đủ → **GPU**, và **luôn luôn: đo bằng `perf` trước khi tối ưu**. Sợi chỉ đỏ xuyên suốt cả cuốn hiện rõ nhất ở đây — **bottleneck gần như luôn là di chuyển dữ liệu** (register↔cache↔RAM↔mạng↔host/device), và **quyết định kỹ thuật phải dựa trên số đo, không phải trực giác**.

> 🆕 Với mục tiêu của bạn: Ch. 15 (perf) là phần **dùng được ngay** — quy trình `stat → record → report`, phân biệt memory/compute/branch-bound bằng counter, benchmark đúng cách — trực tiếp lấp điểm yếu "đo hiệu năng/debug". Ch. 13 (barrier, MPI rank/reduce, serialization+endian) trúng nghề System Software; Ch. 14 chỉ cần nắm *tiêu chí* offload GPU.

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [understanding-the-machine.md](understanding-the-machine.md) — cache/SIMD/branch: các tối ưu mà `perf` counter (cache-miss/branch-miss) đo được.
- [09-debugging/tools.md](../../09-debugging/tools.md) — perf/valgrind/strace bản đầy đủ của repo (chương này thêm góc benchmark một hàm).
- [ostep/concurrency.md](../ostep/concurrency.md) — mutex/condition variable/barrier nhìn từ tầng OS.
- [effective-modern-cpp.md](../effective-modern-cpp.md) cụm 7 — concurrency API C++11/14 (std::thread/async/atomic).
