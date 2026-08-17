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

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DBG-005](../14-prep/mock-interview/bank/debugging.md) | strace dùng để làm gì? Cho ví dụ tình huống nó cứu bạn. |
| [DBG-028](../14-prep/mock-interview/bank/debugging.md) | strace và ltrace khác nhau thế nào? |
| [DBG-006](../14-prep/mock-interview/bank/debugging.md) | Khi chương trình chạy chậm, bạn tìm nguyên nhân thế nào? |
| [LNX-023](../14-prep/mock-interview/bank/linux-sysprog.md) | /proc/<PID>/ cung cấp thông tin gì hữu ích cho debug? |
| [DBG-029](../14-prep/mock-interview/bank/debugging.md) | Vì sao logging quan trọng và một hệ thống log tốt cần gì? |

### 🧪 Bài NGỒI MÁY LÀM

> Khác với bảng trên: đây **không phải câu trả lời miệng**. Mỗi bài có code có bug thật + nhiệm vụ + **output thật đã chạy** để bạn đối chiếu với màn hình của mình. Làm 10–15′/bài, chạy trên Linux.

| ID | Bài | Vá lỗ hổng |
|----|-----|-----------|
| [DBG-030](../14-prep/mock-interview/bank/debugging.md) 🧪 | Daemon chết câm khi khởi động → dùng `strace` tìm file thiếu trong 60″ (và phân biệt `ENOENT` thật với `ENOENT` của loader) | DBG-005 |
| [DBG-031](../14-prep/mock-interview/bank/debugging.md) 🧪 | Chứng minh rò fd + chỉ ra **rò loại fd nào**, chỉ bằng `/proc`, không `lsof` | LNX-023 |

---
⬅️ [gdb.md](gdb.md) · ➡️ Tiếp theo: [memory-bugs.md](memory-bugs.md)
