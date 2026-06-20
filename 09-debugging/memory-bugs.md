# Memory Bugs — Phát hiện & sửa lỗi bộ nhớ

> **TL;DR**
> - Lỗi bộ nhớ là nhóm bug nguy hiểm & khó nhất của C/C++: **leak**, **use-after-free (UAF)**, **buffer overflow**, **double free**, **uninitialized read**, **dangling pointer** — phần lớn là **Undefined Behavior**, có thể "im lặng" rồi crash chỗ khác.
> - **AddressSanitizer (ASan)**: bật bằng `-fsanitize=address` lúc build → bắt overflow/UAF/leak **ngay khi xảy ra**, in stack rõ ràng. Nhanh (~2x), nên dùng mặc định khi dev/test.
> - **Valgrind (memcheck)**: không cần build lại, bắt được nhiều lỗi, nhưng chậm (~10–50x). Tốt khi không recompile được.
> - **UBSan** (`-fsanitize=undefined`): bắt UB khác (overflow số, lệch alignment, null...). **TSan** (`-fsanitize=thread`): bắt data race.
> - Tốt nhất là **phòng ngừa**: RAII, smart pointer, container, `std::string_view`/`span` đúng cách — bug không tạo ra thì không phải tìm.

---

## 1. Các loại lỗi bộ nhớ (ôn nhanh)

(Chi tiết khái niệm ở [01/memory-model](../01-cpp-fundamentals/memory-model.md).)

| Lỗi | Mô tả | Hậu quả |
|-----|-------|---------|
| **Memory leak** | Cấp phát mà không giải phóng | Cạn RAM dần, OOM (nguy hiểm cho hệ chạy lâu/embedded) |
| **Use-after-free (UAF)** | Dùng con trỏ sau khi đã `free`/`delete` | Đọc/ghi rác, crash, lỗ hổng bảo mật |
| **Buffer overflow** | Đọc/ghi ngoài vùng cấp phát | Hỏng dữ liệu lân cận, crash, exploit |
| **Double free** | Giải phóng hai lần | Hỏng heap metadata, crash |
| **Uninitialized read** | Đọc biến chưa khởi tạo | Giá trị rác, hành vi không tất định |
| **Dangling pointer** | Trỏ tới vùng đã hết hạn (stack/heap) | Như UAF |

Điểm chung nguy hiểm: nhiều lỗi **không crash ngay tại chỗ sai** mà làm hỏng bộ nhớ rồi crash ở nơi khác sau đó → khó truy nguồn nếu chỉ nhìn chỗ crash.

---

## 2. AddressSanitizer (ASan) — nên dùng mặc định

Bật ngay lúc build (compiler chèn kiểm tra vào code):
```sh
g++ -fsanitize=address -g -O1 main.cpp -o app
./app      # khi gặp lỗi, ASan in báo cáo chi tiết rồi dừng
```

Bắt được: heap/stack/global buffer overflow, use-after-free, use-after-return, double free, và **memory leak** (LeakSanitizer, mặc định bật cùng). Báo cáo gồm:
- Loại lỗi, địa chỉ.
- **Stack trace nơi truy cập sai** + **nơi vùng nhớ được cấp phát/giải phóng** → thấy ngay "ai sai, vùng này từ đâu".

```
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x...
READ of size 4 at 0x... thread T0
    #0 0x... in process() main.cpp:20      ← chỗ dùng sai
freed by thread T0 here:
    #0 ... in operator delete                ← chỗ free
previously allocated by thread T0 here:
    #0 ... in operator new                   ← chỗ cấp phát
```

Ưu: nhanh (~2x), chính xác, bắt **ngay khi xảy ra** (không phải sau). → Bật ASan khi chạy test/CI là cách rẻ bắt được lượng lớn bug.

---

## 3. Valgrind (memcheck)

```sh
valgrind --leak-check=full --track-origins=yes ./app
```
- **Không cần build lại** (chạy trên binary thường) — tiện khi không recompile được.
- Bắt leak, UAF, đọc chưa khởi tạo (`--track-origins` cho biết giá trị rác *từ đâu*), overflow heap.
- Nhược: **rất chậm** (10–50x), tốn RAM; ít bắt được stack/global overflow tốt như ASan.

**ASan vs Valgrind:** ASan nhanh hơn nhiều và bắt stack overflow tốt hơn, nhưng cần recompile. Valgrind không cần recompile và rất tỉ mỉ với uninitialized reads. Dùng ASan trong vòng phát triển/CI; Valgrind khi cần kiểm tra binary có sẵn hoặc bắt loại lỗi ASan bỏ sót.

---

## 4. UBSan & TSan — anh em sanitizer

```sh
g++ -fsanitize=undefined -g main.cpp -o app   # UBSan
g++ -fsanitize=thread -g main.cpp -o app      # TSan
```

- **UBSan** (UndefinedBehaviorSanitizer): bắt UB ngoài bộ nhớ: signed integer overflow, chia 0, shift quá lớn, dereference null, lệch alignment, ép kiểu enum/bool sai... In rõ dòng vi phạm.
- **TSan** (ThreadSanitizer): bắt **data race** — phát hiện hai thread truy cập cùng dữ liệu không đồng bộ kể cả khi chưa biểu hiện lỗi. Vũ khí số một cho bug concurrency (xem [02/concurrency](../02-modern-cpp/concurrency.md)). Overhead lớn (~5–15x, RAM nhiều).
- Lưu ý: ASan và TSan **không chạy chung** (xung khắc); chạy lần lượt.

---

## 5. Quy trình bắt từng loại bug

| Bug | Cách tốt nhất |
|-----|---------------|
| Leak | ASan/LeakSanitizer (hoặc `valgrind --leak-check=full`); theo dõi RSS qua thời gian |
| UAF / dangling | **ASan** (chỉ thẳng nơi free & nơi alloc) |
| Buffer overflow | **ASan** (stack & heap); gdb watchpoint nếu biết biến bị đè |
| Uninitialized read | Valgrind `--track-origins`, MSan, `-Wmaybe-uninitialized` |
| Double free | ASan |
| Data race | **TSan** |
| Số học UB | UBSan |

---

## 6. Phòng ngừa > chữa (quan trọng nhất)

Bug bộ nhớ tốt nhất là bug **không bao giờ được tạo ra**:
- **RAII + smart pointer**: `unique_ptr`/`shared_ptr` thay `new`/`delete` thủ công → không leak, không double free, không quên dọn (xem [02/raii-smart-pointers](../02-modern-cpp/raii-smart-pointers.md)).
- **Container chuẩn**: `std::vector`/`std::array`/`std::string` thay mảng C thô → bounds tự quản, không tự `malloc`.
- **`.at()`** hoặc kiểm tra biên thay vì `[]` ở nơi nghi ngờ; `std::span` (C++20) mang theo kích thước.
- **`std::string_view`/`span` cẩn thận lifetime**: chúng *không sở hữu* → đừng giữ quá thời gian sống của nguồn (dễ tạo dangling).
- Khởi tạo biến ngay (`int x{};`), bật cảnh báo `-Wall -Wextra`, bật ASan trong CI.
- Gán `nullptr` sau `delete` (nếu dùng con trỏ thô); tránh sở hữu mơ hồ trong API ([07/api-design](../07-shared-libraries/api-design.md)).

> Embedded lưu ý: ASan/Valgrind có thể không chạy trên target hạn chế → test trên host (build cùng code) hoặc dùng phiên bản nhẹ; kết hợp review kỹ và phòng ngừa bằng thiết kế.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Liệt kê các loại lỗi bộ nhớ thường gặp trong C/C++ và vì sao chúng nguy hiểm.</summary>

Các loại chính: memory leak (cấp phát không giải phóng → cạn RAM dần, OOM), use-after-free/dangling pointer (dùng con trỏ sau khi vùng nhớ đã giải phóng/hết hạn → đọc-ghi rác), buffer overflow (truy cập ngoài vùng cấp phát → hỏng dữ liệu lân cận, lỗ hổng bảo mật), double free (giải phóng hai lần → hỏng heap metadata), và uninitialized read (đọc biến chưa khởi tạo → giá trị rác). Chúng nguy hiểm vì phần lớn là **Undefined Behavior**: chương trình có thể trông như chạy đúng trong test rồi hỏng ở môi trường khác, và quan trọng là nhiều lỗi **không crash ngay tại chỗ sai** mà làm hỏng bộ nhớ rồi gây crash hoặc kết quả sai ở nơi khác về sau — khiến việc truy nguồn rất khó nếu chỉ nhìn vào điểm crash. Nhiều lỗi loại này còn là cửa cho khai thác bảo mật.
</details>

<details><summary>2) AddressSanitizer hoạt động thế nào và bắt được gì?</summary>

ASan được bật lúc biên dịch (`-fsanitize=address`); compiler chèn mã kiểm tra và dùng "shadow memory" để theo dõi vùng nào hợp lệ, cùng "red zones" quanh các vùng cấp phát để phát hiện truy cập ra ngoài. Khi chương trình truy cập một địa chỉ không hợp lệ, ASan dừng ngay và in báo cáo gồm loại lỗi, stack trace **nơi truy cập sai**, và nơi vùng nhớ được **cấp phát** cũng như **giải phóng** — nên thấy ngay nguyên nhân. Nó bắt heap/stack/global buffer overflow, use-after-free, use-after-return, double free, và (kèm LeakSanitizer) cả memory leak. Ưu điểm là bắt lỗi **ngay tại thời điểm xảy ra** với overhead thấp (~2x), nên rất phù hợp bật trong test/CI.
</details>

<details><summary>3) ASan và Valgrind khác nhau thế nào? Khi nào dùng cái nào?</summary>

ASan cần **biên dịch lại** với cờ sanitizer (chèn kiểm tra vào code), chạy nhanh (~2x), bắt stack/global overflow tốt và báo lỗi ngay khi xảy ra. Valgrind (memcheck) chạy bằng cách diễn giải binary trên một CPU ảo nên **không cần build lại** và rất tỉ mỉ (đặc biệt với uninitialized reads qua `--track-origins`), nhưng **chậm hơn nhiều** (10–50x), tốn RAM và bắt stack/global overflow kém hơn. Dùng ASan trong vòng phát triển và CI vì nhanh và chính xác; dùng Valgrind khi không thể recompile (chỉ có binary), khi cần kiểm tra kỹ uninitialized memory, hoặc để bắt loại lỗi ASan bỏ sót. Cả hai bổ sung cho nhau.
</details>

<details><summary>4) Làm sao phát hiện data race? Vì sao race khó debug bằng cách thông thường?</summary>

Dùng ThreadSanitizer (`-fsanitize=thread`): nó theo dõi các truy cập bộ nhớ và quan hệ đồng bộ (happens-before) giữa các thread, phát hiện khi hai thread truy cập cùng một vùng nhớ mà ít nhất một là ghi, không có đồng bộ — kể cả khi lần chạy đó chưa biểu hiện lỗi rõ ràng. Race khó debug theo cách thông thường vì nó **không tất định**: phụ thuộc timing/lập lịch nên có thể không tái hiện, và thêm câu lệnh in hay chạy dưới debugger thường thay đổi timing khiến lỗi biến mất (Heisenbug). Vì vậy quan sát thụ động (printf) không đáng tin; cần công cụ phát hiện chủ động như TSan không phụ thuộc vào việc race có thực sự "trúng" trong lần chạy đó hay không. Lưu ý TSan không chạy chung với ASan và có overhead lớn.
</details>

<details><summary>5) Làm sao phòng ngừa lỗi bộ nhớ ngay từ đầu trong C++ hiện đại?</summary>

Cách hiệu quả nhất là không tạo ra lỗi bằng thiết kế: dùng RAII và smart pointer (`unique_ptr`/`shared_ptr`) thay cho `new`/`delete` thủ công để vòng đời tự quản — loại bỏ leak, double free, quên giải phóng; dùng container chuẩn (`vector`, `array`, `string`) thay mảng C thô để biên được quản lý; dùng `.at()` hoặc kiểm tra biên ở nơi nghi ngờ và `std::span` (mang theo kích thước) thay con trỏ trần; khởi tạo biến ngay khi khai báo; bật cảnh báo `-Wall -Wextra` và bật ASan/UBSan trong CI. Cẩn thận với `string_view`/`span` vì chúng không sở hữu dữ liệu — không được giữ chúng quá thời gian sống của nguồn để tránh dangling. Trong embedded nơi sanitizer khó chạy trên target, nên test trên host với cùng code và bù bằng review kỹ và thiết kế ownership rõ ràng.
</details>

---
⬅️ [tools.md](tools.md) · ➡️ Tiếp theo: [kernel-debugging.md](kernel-debugging.md)
