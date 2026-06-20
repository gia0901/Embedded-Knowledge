# Debugging Tools — strace, ltrace, perf, /proc & Logging

> **TL;DR**
> - Khi không biết chương trình "đang làm gì", dùng công cụ **quan sát** thay vì đọc code: hành vi thật quan trọng hơn giả định.
> - **`strace`**: theo dõi **syscall** (mở file, đọc/ghi, mạng) → tìm "nó gọi gì xuống kernel", lỗi `ENOENT`/`EACCES`, treo ở syscall nào.
> - **`ltrace`**: theo dõi lời gọi **thư viện** (libc, `.so`).
> - **`perf`**: profiling hiệu năng — CPU đang đốt ở hàm nào (hotspot), cache miss, cycle.
> - **`/proc`**: cửa sổ vào trạng thái process (fd, maps, status, stack) — không cần dừng chương trình.
> - **Logging tốt**: có cấu trúc, có mức (level), có timestamp — vũ khí debug số một ở production nơi không gắn debugger được.

---

## 1. Triết lý: quan sát hành vi thật

Đọc code cho biết chương trình *nên* làm gì; công cụ quan sát cho biết nó *thực sự* làm gì. Khoảng cách giữa hai cái thường chính là bug (giả định sai). Với hệ phức tạp/production/embedded nơi không gdb được, các công cụ này là chủ lực.

---

## 2. `strace` — theo dõi syscall

Mọi tương tác với "thế giới ngoài" (file, mạng, thiết bị, process) đều qua syscall. `strace` ghi lại chúng:

```sh
strace ./app                      # mọi syscall + tham số + kết quả
strace -f ./app                   # theo cả tiến trình con (fork)
strace -e trace=open,read,write ./app   # lọc theo loại syscall
strace -e trace=network ./app     # nhóm network
strace -p <PID>                   # attach process đang chạy
strace -T ./app                   # kèm thời gian mỗi syscall (tìm chậm)
strace -c ./app                   # thống kê tổng hợp (đếm, thời gian mỗi loại)
```

Dùng để trả lời:
- **"Sao mở file thất bại?"** → thấy `openat("/etc/foo", ...) = -1 ENOENT` (sai đường dẫn) hoặc `EACCES` (quyền).
- **"Chương trình treo ở đâu?"** → thấy syscall cuối đứng yên (vd `read(...)` blocking, `futex(...)` chờ lock, `connect(...)` chờ mạng).
- **"Nó thực sự đọc config nào?"** → thấy chính xác file/đường dẫn nó mở.

> Mẹo: bug "không tìm thấy file/thư viện" giải quyết rất nhanh bằng strace — thấy ngay nó tìm ở đâu và thiếu cái gì.

---

## 3. `ltrace` — theo dõi lời gọi thư viện

Tương tự strace nhưng cho **lời gọi hàm thư viện động** (libc, các `.so`):
```sh
ltrace ./app                      # thấy malloc/free, strcpy, hàm của .so...
ltrace -e 'malloc+free' ./app
```
Hữu ích khi muốn xem chương trình gọi API thư viện nào với tham số gì (vd kiểm tra cấp phát, lời gọi hàm của thư viện bên thứ ba) mà không cần symbol/source.

---

## 4. `perf` — profiling hiệu năng

Khi chương trình **chậm**, đừng đoán hotspot — đo bằng `perf`:

```sh
perf stat ./app                   # tổng quan: cycle, instruction, cache miss, branch miss
perf record ./app                 # lấy mẫu (sampling) call stack theo thời gian
perf report                       # xem hàm nào chiếm CPU nhiều nhất
perf record -g ./app              # kèm call graph
perf top                          # hotspot toàn hệ thống thời gian thực
```

- `perf` lấy mẫu định kỳ vị trí CPU → biết **hàm nào ngốn CPU** (hotspot) mà không cần sửa code.
- **Flame graph** (dựng từ `perf record`) trực quan hóa nơi tốn thời gian.
- Đo cache miss/branch miss giúp tối ưu mức thấp (quan trọng cho hot path/embedded).

> Nguyên tắc: **đo trước khi tối ưu.** Trực giác về hotspot thường sai; perf cho dữ liệu thật.

---

## 5. `/proc` & `/sys` — cửa sổ vào trạng thái runtime

Filesystem ảo phơi bày trạng thái kernel/process, đọc được bằng `cat` mà không dừng chương trình:

```sh
cat /proc/<PID>/status            # trạng thái: state, mem (VmRSS), threads...
ls  /proc/<PID>/fd                # các file descriptor đang mở (tìm fd leak)
cat /proc/<PID>/maps              # bản đồ bộ nhớ (vùng nào map gì, .so ở đâu)
cat /proc/<PID>/stack             # call stack trong kernel (nếu cho phép)
cat /proc/<PID>/cmdline /environ  # tham số & biến môi trường thực tế
cat /proc/<PID>/limits            # giới hạn tài nguyên
```

Rất tiện để kiểm tra nhanh: process đang ngốn bao nhiêu RAM, mở bao nhiêu fd (rò fd?), thực sự chạy với env/arg nào, map những `.so` nào.

---

## 6. Logging — vũ khí debug ở production

Ở production/field (embedded) thường không gắn được gdb → **log là công cụ chính**. Log tốt cần:
- **Mức (level)**: ERROR / WARN / INFO / DEBUG / TRACE — bật/tắt theo nhu cầu, tránh ngập log.
- **Timestamp** (và monotonic clock) — để xếp thứ tự sự kiện, đo khoảng thời gian, phát hiện trễ.
- **Ngữ cảnh**: id request/thread, giá trị biến liên quan — đủ để tái dựng điều gì xảy ra.
- **Có cấu trúc** (key=value/JSON) nếu cần phân tích máy.
- **Không log dữ liệu nhạy cảm**; cân nhắc chi phí (log đồng bộ trong hot path làm chậm).

Trên Linux: `journalctl`/`syslog`; embedded thường ring buffer + xuất qua UART. Một dòng log đúng chỗ với đủ ngữ cảnh thường nhanh hơn cả buổi ngồi debugger.

---

## 7. Bảng chọn công cụ nhanh

| Triệu chứng | Công cụ |
|-------------|---------|
| Crash/segfault | gdb + core dump, ASan |
| "Không tìm thấy file/thư viện", lỗi I/O | **strace** |
| Treo, không rõ chờ gì | strace `-p`, gdb attach + `bt` |
| Gọi API thư viện sai | **ltrace** |
| Chậm, cần tìm hotspot | **perf** |
| Rò fd / ngốn RAM | `/proc/<PID>/fd`, `status`, `maps` |
| Memory leak/corruption | Valgrind, ASan ([memory-bugs.md](memory-bugs.md)) |
| Production không debugger | **logging** có cấu trúc |

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) strace dùng để làm gì? Cho ví dụ tình huống nó cứu bạn.</summary>

strace theo dõi mọi system call mà một tiến trình thực hiện cùng tham số và giá trị trả về — tức là mọi tương tác của chương trình với kernel/thế giới ngoài (file, mạng, thiết bị, IPC). Ví dụ điển hình: chương trình báo lỗi mơ hồ "không khởi động được"; chạy `strace` thấy `openat("/etc/app/config.yaml", O_RDONLY) = -1 ENOENT` → lộ ngay là nó tìm config sai đường dẫn (hoặc `EACCES` là lỗi quyền). Hoặc khi chương trình treo, strace cho thấy syscall cuối đang đứng (vd `read` blocking, `futex` chờ lock, `connect` chờ mạng) → biết nó kẹt ở đâu. strace đặc biệt mạnh cho lớp bug "thiếu file/thư viện/quyền/mạng" mà đọc code không thấy.
</details>

<details><summary>2) strace và ltrace khác nhau thế nào?</summary>

strace theo dõi **system call** — ranh giới giữa chương trình và kernel (open, read, write, mmap, futex, socket...). ltrace theo dõi **lời gọi hàm thư viện động** trong user space (libc và các `.so`: malloc, free, strcpy, hàm của thư viện bên thứ ba...). Nói cách khác, strace cho thấy chương trình yêu cầu gì từ hệ điều hành, còn ltrace cho thấy nó gọi API thư viện nào (nhiều lời gọi thư viện cuối cùng dẫn tới syscall, nhưng ltrace bắt ở tầng cao hơn). Dùng strace cho vấn đề I/O/hệ thống/treo; dùng ltrace khi muốn kiểm tra tương tác với thư viện (vd theo dõi cấp phát bộ nhớ hoặc lời gọi tới một lib cụ thể).
</details>

<details><summary>3) Khi chương trình chạy chậm, bạn tìm nguyên nhân thế nào?</summary>

Không đoán hotspot mà **đo** bằng profiler như perf. Bắt đầu với `perf stat` để có bức tranh tổng quan (số cycle, instruction, tỉ lệ cache miss, branch miss). Sau đó `perf record -g` để lấy mẫu call stack theo thời gian và `perf report` (hoặc dựng flame graph) để xem hàm nào chiếm nhiều CPU nhất — đó là nơi đáng tối ưu. Nếu chậm do chờ I/O chứ không phải CPU, perf sẽ cho thấy CPU không bận và cần nhìn sang strace (`-T` đo thời gian mỗi syscall) để tìm syscall chậm, hoặc kiểm tra blocking/lock contention. Nguyên tắc cốt lõi: đo trước khi tối ưu, vì trực giác về hotspot thường sai.
</details>

<details><summary>4) /proc/<PID>/ cung cấp thông tin gì hữu ích cho debug?</summary>

`/proc/<PID>` là cửa sổ vào trạng thái runtime của tiến trình mà không cần dừng nó: `status` cho biết trạng thái, dung lượng bộ nhớ (VmRSS), số thread; `fd/` liệt kê các file descriptor đang mở (rất hữu ích để phát hiện fd leak — số fd tăng dần không đóng); `maps` cho bản đồ vùng nhớ (vùng nào ánh xạ thư viện nào, phát hiện thư viện sai/địa chỉ); `cmdline` và `environ` cho biết tham số và biến môi trường **thực tế** đang chạy (hữu ích khi nghi ngờ chạy sai cấu hình); `limits` cho giới hạn tài nguyên. Đây là cách kiểm tra nhanh "process đang ở tình trạng nào" trong production.
</details>

<details><summary>5) Vì sao logging quan trọng và một hệ thống log tốt cần gì?</summary>

Ở môi trường production hoặc thiết bị embedded ngoài field, thường không thể gắn debugger hay tái hiện bug, nên log là công cụ điều tra chính — đôi khi là duy nhất. Một hệ log tốt cần: **mức độ (level)** ERROR/WARN/INFO/DEBUG/TRACE để điều chỉnh độ chi tiết và tránh ngập log; **timestamp** (kèm clock đơn điệu) để xếp thứ tự sự kiện, đo khoảng thời gian và phát hiện trễ; **ngữ cảnh** đủ để tái dựng tình huống (id request/thread, giá trị biến liên quan); nên **có cấu trúc** (key=value/JSON) nếu cần phân tích bằng máy. Cũng phải lưu ý không ghi dữ liệu nhạy cảm và cân nhắc chi phí (log đồng bộ trong hot path làm chậm). Một dòng log đặt đúng chỗ với đủ ngữ cảnh thường tiết kiệm hơn nhiều so với ngồi dò bằng debugger.
</details>

---
⬅️ [gdb.md](gdb.md) · ➡️ Tiếp theo: [memory-bugs.md](memory-bugs.md)
