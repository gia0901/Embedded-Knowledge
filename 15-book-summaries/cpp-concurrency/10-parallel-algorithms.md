# Ch. 10 — Parallel algorithms (tr. 327–338) ✅ đã đọc sâu toàn văn

> Thuộc [C++ Concurrency in Action](README.md) · **[⏮ Ch. 9 — Advanced thread management](09-advanced-thread-management.md)** · **[Ch. 11 → Testing and debugging](11-testing-debugging.md)**
> Nguồn: đọc trực tiếp PDF (2nd ed, 2019), **trang sách = trang PDF − 23**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Chương ngắn nhất & thực dụng nhất (11 trang):** parallel algorithms của **C++17** — thay vì tự viết như ch. 8, chỉ cần thêm **execution policy** vào lời gọi STL algorithm. Giá trị interview: biết `seq`/`par`/`par_unseq` khác nhau thế nào và **ràng buộc code** của mỗi policy.

**Sách nêu (tr. 327):** parallel algorithms do C++17 thêm vào.

---

## Cụm 1 — Parallel STL & execution policy (tr. 327–331)

### 1.1 Ý tưởng: thêm một tham số đầu (tr. 327–328)

C++17 thêm **overload song song** cho nhiều thuật toán trên range (`std::find`, `std::transform`, `std::reduce`, `std::sort`...). Chữ ký giống hệt bản thường, chỉ **thêm execution policy làm tham số đầu:**
```cpp
std::vector<int> my_data;
std::sort(std::execution::par, my_data.begin(), my_data.end());   // cho phép chạy song song
```
**⚠️ Đây là CHO PHÉP, không phải BẮT BUỘC (tr. 328):** thư viện **vẫn có thể chạy một thread** nếu muốn. Và **độ phức tạp thuật toán đổi** — thường lỏng hơn bản tuần tự, vì parallel làm **nhiều việc tổng cộng hơn** để tận dụng song song (chia cho 100 processor → vẫn nhanh gấp 50 dù làm gấp đôi việc).

### 1.2 Ba execution policy (tr. 328–329)

Ba class trong `<execution>`, ba object tương ứng để truyền:
| Policy object | Class | Ý |
|---|---|---|
| `std::execution::seq` | `sequenced_policy` | **Không song song** — mọi thao tác trên thread gọi hàm |
| `std::execution::par` | `parallel_policy` | Song song cơ bản trên nhiều thread |
| `std::execution::par_unseq` | `parallel_unsequenced_policy` | Song song **+ vectorize** — ràng buộc chặt nhất |

**⚠️ Không tự tạo policy được** (chỉ copy ba object trên); implementation có thể thêm policy riêng.

### 1.3 Ba ảnh hưởng chung của execution policy (tr. 328–330)

1. **Độ phức tạp thay đổi (tr. 328–329):** nếu bản thường nói "chính xác/nhiều nhất N lần" thì bản có policy lỏng thành **O(N)** — có thể làm gấp một bội số (tùy internals thư viện).
2. **⚠️ Exception → `std::terminate` (tr. 329):** mọi standard policy **gọi `std::terminate` nếu có exception chưa bắt**. Ngoại lệ duy nhất được ném ra là **`std::bad_alloc`**. → **Khác biệt then chốt giữa `seq` và không có policy:** bản không policy **propagate exception**, bản `seq` thì **terminate**.
```cpp
std::for_each(v.begin(), v.end(), [](auto x){ throw my_exception(); });                    // propagate
std::for_each(std::execution::seq, v.begin(), v.end(), [](auto x){ throw my_exception(); }); // TERMINATE
```
3. **Nơi/khi/cách chạy các bước (tr. 329–330):** phần **khác nhau** giữa ba policy — dùng execution agent nào (thread thường, vector stream, GPU thread), ràng buộc thứ tự ra sao.

### Insight đáng nhớ (Cụm 1)
- **Execution policy = CHO PHÉP song song, không đảm bảo.** Thư viện tự quyết mức song song thật.
- **⚠️ Ngay cả `seq` cũng đổi hành vi exception:** có policy → exception chưa bắt = `terminate` (chỉ `bad_alloc` thoát ra). Đây là bẫy: `seq` ≠ "không có policy".

---

## Cụm 2 — Ba policy chi tiết + ràng buộc code (tr. 330–331)

### 2.1 `seq` — tuần tự, nhưng thứ tự KHÔNG xác định (tr. 330)

Mọi thao tác trên **cùng thread**, **không interleave**, nhưng **thứ tự không xác định** (có thể khác bản không-policy). Ví dụ điền vector 1–1000 bằng `for_each(seq, ...)` → điền đủ 1–1000 nhưng **thứ tự bất kỳ** (bản không-policy thì đúng thứ tự). **Ràng buộc lỏng:** callable được dùng đồng bộ, dựa vào "cùng thread" — chỉ không được dựa vào **thứ tự**.

### 2.2 `par` — song song cơ bản (tr. 330–331)

Thao tác chạy trên thread gọi HOẶC thread thư viện tạo. Trên một thread thì **theo thứ tự, không interleave**; một thao tác chạy trọn trên **một thread cố định**. **⚠️ Ràng buộc thêm:** callable **không được gây data race** khi chạy song song, **không được dựa vào** chạy cùng/khác thread với thao tác khác.
```cpp
std::for_each(std::execution::par, v.begin(), v.end(), [](auto& x){ ++x; });   // OK: độc lập

// ⚠️ UNDEFINED BEHAVIOR: count sửa từ mọi lambda → data race
int count = 0;
std::for_each(std::execution::par, v.begin(), v.end(), [&](int& x){ x = ++count; });
```
**⚠️ Điểm tinh vi (tr. 331):** UB là **thuộc tính TĨNH của lời gọi**, không phụ thuộc thư viện có thật sự dùng nhiều thread hay không. Sửa: `count` thành `std::atomic<int>` hoặc mutex (nhưng khi đó serialize hết → mất ý nghĩa song song).

### 2.3 `par_unseq` — song song + vectorize, ràng buộc chặt nhất (tr. 331)

Cho thư viện **nhiều tự do nhất** đổi lấy ràng buộc **chặt nhất**. Thao tác chạy trên thread **không xác định, không thứ tự, không sequenced**: có thể **interleave trên một thread** (bắt đầu thao tác 2 trước khi thao tác 1 xong) và **migrate giữa thread** (bắt đầu ở thread này, chạy tiếp ở thread khác, xong ở thread thứ ba).

**⚠️ Ràng buộc chí mạng (tr. 331):** callable **KHÔNG được dùng bất kỳ đồng bộ nào** — không mutex, không atomic, không gì synchronize. Chỉ được thao tác trên **phần tử liên quan** và dữ liệu truy cập được từ nó; **không sửa state chung** giữa thread/phần tử.

🆕 Lý do: `par_unseq` cho phép **vectorization (SIMD)** — interleave các thao tác trên một thread như lệnh vector; mà lock trong lệnh vector → deadlock (thread chờ chính nó).

### Insight đáng nhớ (Cụm 2)
- **Ba policy = ba mức ràng buộc code tăng dần:** `seq` (dựa cùng-thread ok, chỉ không thứ tự) < `par` (không data race, không dựa vào thread nào) < `par_unseq` (**cấm mọi đồng bộ** — không mutex/atomic).
- **🎯 `par` cho phép mutex/atomic; `par_unseq` thì CẤM.** Đây là ranh giới quan trọng nhất giữa hai policy — dùng nhầm = UB.
- **⚠️ UB là thuộc tính tĩnh của lời gọi**, không phụ thuộc thư viện có dùng nhiều thread thật hay không.

---

## Cụm 3 — Danh sách thuật toán, iterator, và ví dụ (tr. 331–338)

### 3.1 Thuật toán được song song hóa + Forward Iterator (tr. 331–334)

Hầu hết thuật toán trong `<algorithm>` và `<numeric>` có overload nhận policy (`for_each`, `find`, `transform`, `sort`, `reduce`, `transform_reduce`, `inclusive_scan`...). **⚠️ Ngoại lệ đáng chú ý: `std::accumulate` KHÔNG có bản song song** (tuần tự chặt) — dùng **`std::reduce`** thay thế (⚠️ nếu phép reduce **không associative + commutative** → kết quả **không xác định** do thứ tự bất kỳ).

**⚠️ Đổi Input/Output Iterator → Forward Iterator (tr. 332–334):** bản có policy yêu cầu **Forward Iterator** (bản thường cho Input/Output Iterator). Vì:
- Input Iterator là **single-pass** (chỉ truy cập phần tử hiện tại, không lưu iterator trước) → nếu cho phép, các thread phải **serialize** truy cập vào một iterator duy nhất → giết song song.
- Forward Iterator là **multipass**, copy được, tăng không làm invalidate copy khác → mỗi thread thao tác trên **copy iterator riêng** → song song thật.

🆕 Năm loại iterator (tr. 333): Input (single-pass đọc) < Output (single-pass ghi) < Forward (multipass một chiều) < Bidirectional (đi lùi được) < Random Access (nhảy offset, `[]`).

### 3.2 Ví dụ: parallel for (embarrassingly parallel) (tr. 334–335)

Thay OpenMP `#pragma omp parallel for` bằng:
```cpp
std::for_each(std::execution::par, v.begin(), v.end(), do_stuff);   // mỗi phần tử độc lập
```
**Chọn policy (tr. 334–335):** **`par` là mặc định** — dùng cho hầu hết trường hợp thay bản không-policy. `par_unseq` chỉ khi code không đồng bộ gì (cho thư viện reorder/interleave → hiệu năng cao hơn, đổi lấy ràng buộc chặt).

**Listing 10.1 vs 10.2 — minh họa ranh giới `par` / `par_unseq`:**
```cpp
// Listing 10.1: mutex per-element → dùng par được, par_unseq là UB
class X { mutable std::mutex m; int data; public: void increment(){ std::lock_guard g(m); ++data; } };
void increment_all(std::vector<X>& v) {
    std::for_each(std::execution::par, v.begin(), v.end(), [](X& x){ x.increment(); });  // par OK
    // std::execution::par_unseq → UB (có mutex bên trong!)
}

// Listing 10.2: bỏ mutex per-element, khóa CẢ container bên ngoài → par_unseq OK
class Y { int data; public: void increment(){ ++data; } };   // KHÔNG đồng bộ
class ProtectedY { std::mutex m; std::vector<Y> v; /* lock/unlock/get_vec */ };
void increment_all(ProtectedY& data) {
    std::lock_guard guard(data);                             // khóa NGOÀI lời gọi parallel
    std::for_each(std::execution::par_unseq, data.get_vec().begin(), data.get_vec().end(),
        [](Y& y){ y.increment(); });                         // par_unseq OK: không đồng bộ bên trong
}
```
**Bài học (tr. 335–336):** `par_unseq` đòi **không đồng bộ bên trong** → chuyển đồng bộ ra **ngoài** lời gọi parallel (khóa cả container). Đổi lại: thread ngoài phải chờ **cả thao tác** xong (không còn granularity per-element như 10.1).

### 3.3 Ví dụ thực tế: đếm visit website với `transform_reduce` (Listing 10.3, tr. 336–338)

Phân tích log triệu dòng: **map** (parse mỗi dòng) + **reduce** (gộp kết quả). Đây là bài toán `transform_reduce` sinh ra để giải:
```cpp
using visit_map_type = std::unordered_map<std::string, unsigned long long>;
visit_map_type count_visits_per_page(std::vector<std::string> const& log_lines) {
    struct combine_visits {
        // 4 overload operator() — vì reduce phải gộp mọi cặp kiểu: (map,map), (log,map), (map,log), (log,log)
        visit_map_type operator()(visit_map_type lhs, visit_map_type rhs) const { /* merge */ }
        visit_map_type operator()(log_info log, visit_map_type map) const { ++map[log.page]; return map; }
        visit_map_type operator()(visit_map_type map, log_info log) const { ++map[log.page]; return map; }
        visit_map_type operator()(log_info l1, log_info l2) const { /* map với 2 page */ }
    };
    return std::transform_reduce(
        std::execution::par, log_lines.begin(), log_lines.end(),
        visit_map_type(), combine_visits(), parse_log_line);   // transform=parse, reduce=combine
}
```
**⚠️ Vì sao 4 overload (tr. 337–338):** reduce có thể gộp bất kỳ cặp nào — hai map, một log + một map (cả hai chiều), hai log → cần đủ 4 overload (không dùng lambda đơn được). **Lợi ích:** giao việc song song hóa khó cho thư viện, mình chỉ lo kết quả. (So ch. 8/9 — viết tay `partial_sum` song song rất khó.)

### Insight đáng nhớ (Cụm 3)
- **`par` là mặc định.** Dùng `par_unseq` chỉ khi chắc chắn không đồng bộ bên trong → chuyển khóa ra ngoài lời gọi.
- **Forward Iterator bắt buộc** cho parallel — Input Iterator single-pass sẽ serialize các thread.
- **`std::reduce`/`transform_reduce` thay `accumulate`** cho song song — nhưng phép gộp phải **associative + commutative** (không thì kết quả bất định). `transform_reduce` = map-reduce chuẩn của C++.

---

## Tóm tắt chương (theo sách, tr. 338)

Parallel algorithms C++17: các execution policy, ảnh hưởng của policy lên hành vi + ràng buộc lên code, và ví dụ dùng thực tế.

**🆕 Bảng chốt — ba execution policy:**
| Policy | Song song? | Interleave/migrate? | Cho phép mutex/atomic? | Iterator |
|---|---|---|---|---|
| `seq` | Không | Không | Có | Forward |
| `par` | Có (nhiều thread) | Không (thao tác trọn trên 1 thread) | **Có** | Forward |
| `par_unseq` | Có + vectorize | **Có** (SIMD) | **KHÔNG** (UB) | Forward |
| *(không policy)* | Không | — | Có | Input/Output cũng được; exception **propagate** (không terminate) |

---

## Góc interview

**Câu 1 (🎯 hay hỏi với C++17):** Phân biệt `std::execution::seq`, `par`, `par_unseq`. Ràng buộc code của mỗi cái? Khi nào dùng cái nào?

<details><summary>Đáp án</summary>

**Ba policy (C++17, header `<execution>`):**
| | `seq` | `par` | `par_unseq` |
|---|---|---|---|
| Song song | **Không** (thread gọi hàm) | **Có** (nhiều thread) | **Có + vectorize (SIMD)** |
| Interleave trên 1 thread | Không | Không (thao tác trọn trên 1 thread cố định) | **Có** (bắt đầu op2 trước khi op1 xong) |
| Migrate giữa thread | — | Không | **Có** (op chạy trên nhiều thread) |
| **Cho phép mutex/atomic** | Có | **Có** | **KHÔNG — UB nếu dùng** |
| Ràng buộc | Chỉ không dựa vào **thứ tự** | Không **data race**, không dựa vào thread nào | **Cấm mọi đồng bộ** |

**Ràng buộc tăng dần:**
- **`seq`:** callable được dùng đồng bộ, dựa vào cùng-thread — chỉ **không được dựa vào thứ tự** (thứ tự bất kỳ, có thể khác bản không-policy).
- **`par`:** callable **không được gây data race**, không dựa vào chạy cùng/khác thread với op khác. Được dùng mutex/atomic (nhưng nếu serialize hết thì mất ý nghĩa).
- **`par_unseq`:** callable **không được dùng BẤT KỲ đồng bộ nào** (không mutex, không atomic) — vì cho phép vectorize/interleave; lock trong lệnh vector → deadlock. Chỉ thao tác trên phần tử + dữ liệu từ nó, không sửa state chung.

**Khi nào dùng:**
- **`par` là mặc định** — dùng cho hầu hết trường hợp muốn song song hóa STL algorithm.
- **`par_unseq`** khi chắc chắn không đồng bộ bên trong → chuyển đồng bộ **ra ngoài** lời gọi parallel (khóa cả container). Cho thư viện reorder/interleave → hiệu năng cao hơn.
- **`seq`** khi cần độ phức tạp/exception behavior của policy nhưng không muốn song song.

**⚠️ Bẫy nên nêu:**
1. **Policy là CHO PHÉP, không BẮT BUỘC** — thư viện vẫn có thể chạy 1 thread.
2. **Ngay cả `seq` đổi hành vi exception:** có policy → exception chưa bắt = **`std::terminate`** (chỉ `bad_alloc` thoát); bản không-policy thì **propagate**. → `seq` ≠ "không có policy".
3. **UB là thuộc tính TĨNH của lời gọi** — `par` với data race là UB dù thư viện có thật sự dùng nhiều thread hay không.
4. **Parallel overload đòi Forward Iterator** (không Input/Output) — Input single-pass sẽ serialize các thread.

</details>

**Câu 2 (🟠):** Cần đếm tần suất từ hàng triệu dòng log song song. Dùng thuật toán STL nào? Vì sao không dùng `std::accumulate`?

<details><summary>Đáp án</summary>

**Dùng `std::transform_reduce` với `std::execution::par` (Listing 10.3)** — đây là **map-reduce** chuẩn của C++: **transform** (parse mỗi dòng, độc lập) + **reduce** (gộp kết quả):
```cpp
using visit_map = std::unordered_map<std::string, unsigned long long>;
visit_map count_visits(std::vector<std::string> const& lines) {
    struct combine {  // reduce phải gộp mọi cặp kiểu → 4 overload operator()
        visit_map operator()(visit_map a, visit_map b) const { /* merge b vào a */ }
        visit_map operator()(log_info l, visit_map m) const { ++m[l.page]; return m; }
        visit_map operator()(visit_map m, log_info l) const { ++m[l.page]; return m; }
        visit_map operator()(log_info a, log_info b) const { /* map 2 page */ }
    };
    return std::transform_reduce(std::execution::par,
        lines.begin(), lines.end(), visit_map(), combine(), parse_log_line);
}
```

**Vì sao `transform_reduce` hợp:** parse mỗi dòng **độc lập hoàn toàn** (embarrassingly parallel), gộp kết quả **từng phần** (miễn tổng cuối đúng) → thư viện tự chia cho hardware, mình chỉ lo kết quả.

**⚠️ Vì sao 4 overload:** reduce gộp bất kỳ cặp nào — (map,map), (log,map), (map,log), (log,log) → cần đủ 4 (lambda đơn không đủ).

**Vì sao KHÔNG `std::accumulate` (tr. 331):**
- `std::accumulate` **không có bản song song** — nó là **tuần tự chặt** (accumulate trái sang phải theo thứ tự cố định).
- Thay bằng **`std::reduce`** (bản khái quát, có song song). **⚠️ Cảnh báo:** nếu phép reduce **không associative + commutative** → kết quả **không xác định** (do thứ tự gộp bất kỳ). Ở đây phép gộp map là associative + commutative (cộng tần suất) → an toàn.

**Điểm cộng:** viết tay `partial_sum`/reduce song song rất khó (ch. 8) — parallel STL giao việc đó cho implementer thư viện.

</details>

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [08-designing-concurrent-code.md](08-designing-concurrent-code.md) — parallel `for_each`/`find`/`partial_sum` viết TAY (chương này giao cho thư viện); false sharing, Amdahl.
- [02-managing-threads.md](02-managing-threads.md) — `parallel_accumulate` tay (Listing 2.9) — so với `std::reduce` một dòng.
- [09-advanced-thread-management.md](09-advanced-thread-management.md) — thread pool mà parallel algorithm dùng bên dưới.
- [cpp-mindset/algorithmic-complexity.md](../cpp-mindset/algorithmic-complexity.md) — độ phức tạp thuật toán (parallel làm nhiều việc hơn nhưng nhanh hơn nhờ chia core).
- [EMC++ cụm 7](../effective-modern-cpp.md) — concurrency API C++11/14 (nền của C++17 parallel algorithms).

**Chương tiếp theo:** [Ch. 11 — Testing and debugging multithreaded applications →](11-testing-debugging.md) (các loại bug concurrency; định vị bug qua test + code review; thiết kế test đa luồng; test hiệu năng) — chương cuối, kỹ năng debug (điểm yếu cần đầu tư theo CLAUDE.md).
