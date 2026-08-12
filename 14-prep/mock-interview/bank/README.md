# 🏦 Ngân hàng câu hỏi — DUY NHẤT

> **Đây là nguồn câu hỏi duy nhất của toàn repo.** Các bộ câu hỏi cũ (`11-interview-questions/`, `14-prep/technical_round/`) đã gộp hết về đây; bộ cũ đã **xoá** (2026-08-09) vì chỉ còn là con trỏ. Không tạo bank thứ hai; câu mới luôn thêm vào đây.
> Chia file theo **domain** cho dễ đọc, nhưng **ID xuyên suốt** toàn bank. Cách vận hành phiên: [../config.md](../config.md).

---

## Sơ đồ ID

`<DOMAIN>-<NNN>` — số 3 chữ số, tăng dần trong mỗi domain, **không tái sử dụng** (xóa câu thì để trống ID, không gán lại).

| Domain | File | Nội dung | Topic gốc |
|---|---|---|---|
| `CPP` | [cpp.md](cpp.md) | C/C++ & Modern C++ | [01](../../../01-cpp-fundamentals/), [02](../../../02-modern-cpp/) |
| `OS` | [os.md](os.md) | Hệ điều hành (process/thread/sched/mem/sync) | [03](../../../03-operating-system/) |
| `LNX` | [linux-sysprog.md](linux-sysprog.md) | Linux system programming (fd/syscall/IPC/epoll/signal) | [04](../../../04-linux-system-programming/) |
| `DRV` | [drivers-embedded.md](drivers-embedded.md) | Driver, device tree, embedded (general) | [05](../../../05-drivers-device-tree/), [08](../../../08-embedded-systems/) |
| `BUS` | [drivers-embedded.md](drivers-embedded.md#bus) | I2C/SPI/UART | [08](../../../08-embedded-systems/) |
| `EMB` | [embedded-fundamentals.md](embedded-fundamentals.md) | Embedded SW nền tảng: C/thanh ghi, bộ nhớ bare-metal/startup, ngắt, RTOS, firmware, số học/độ tin cậy, low-power, debug HW | [08](../../../08-embedded-systems/) |
| `BSP` | [bsp.md](bsp.md) | BSP chuyên sâu: boot/DT probe/IRQ/DMA/flash/OTA/Yocto/power/RT/bring-up | [15 melp](../../../15-book-summaries/melp/) |
| `BLD` | [build-systems.md](build-systems.md) | CMake, Yocto, cross-compile, CI | [06](../../../06-build-systems/) |
| `DBG` | [debugging.md](debugging.md) | Debugging & tools | [09](../../../09-debugging/) |
| `DP` | [design-patterns.md](design-patterns.md) | Design patterns & SOLID | [11](../../../11-design-patterns/) |
| `DSA` | [dsa.md](dsa.md) | Cấu trúc dữ liệu & giải thuật | [12](../../../12-dsa/) |
| `NET` | [networking.md](networking.md) | Networking | [13](../../../13-networking/) |
| `SD` | [system-design.md](system-design.md) | Tư duy & system design | [10](../../../10-thinking/), [07](../../../07-shared-libraries/) |
| `BEH` | [behavioral.md](behavioral.md) | Behavioral / HR (STAR) | — |
| `COD` | [coding.md](coding.md) | Bài coding tại chỗ | [12](../../../12-dsa/) |

## Quy ước metadata mỗi câu

```
#### <ID> · <level> · <type> [· ⭐] [· 🏗️] [· 🎤 ngày] · [→ nguồn](link)
**Câu hỏi in đậm.**
<details><summary>Đáp án</summary>
...
</details>
```

- **level:** 🟢 cơ bản · 🟡 trung bình · 🟠 khó · 🔴 senior.
- **type:** `concept` · `coding` · `design`.
- **⭐** = xác suất gặp cao với JD (BSP / C++ system). **🏗️** = câu mở/tình huống, chấm theo khung. **🎤 ngày** = câu được thêm từ một phiên mock.
- **→ nguồn** = link tài liệu để ôn lại (dùng khi review).

## ✍️ Tiêu chí viết đáp án (áp cho câu mới **và** khi sửa câu cũ)

> **Nguyên tắc:** đáp án phải **tự giải nén được sau 2 tuần**. Nén quá tay thì đọc lại chỉ thấy các từ khoá đúng mà không dựng lại được lập luận — vô dụng đúng lúc cần. Nhưng cũng **không giãn đều mọi câu**: độ dài phải theo level.

**Độ dài:**

| Level | Giới hạn | Tính chất |
|---|---|---|
| 🟢 | **30–60 từ**, 1 đoạn, không bảng | **Trần cứng.** Đây là nguồn cho phiên `rapid` (12 câu/15′, đáp án 1–2 câu) — giãn ra là **phá** công dụng đó |
| 🟡 *nhẹ* (định nghĩa, so sánh 1 chiều) | ~1 đoạn + tối đa 1 bảng | Đủ nêu cơ chế + một so sánh |
| 🟡 *nặng cơ chế* · 🟠 · 🔴 | **Không có trần từ** — vừa **một màn hình** là được | Viết đủ để hiểu, dừng khi hết ý. Dùng khung 5 phần dưới |

> ⚠️ **Đã thử trần cứng 250 từ cho 🟠🔴 và BỎ** (2026-08-05). Lý do: khi cắt cho vừa số, bản rút gọn *đọc lủng củng, mất mạch* — đúng cái bệnh mà tiêu chí này sinh ra để chữa. Vài chục từ không phải chi phí đáng kể, còn mạch lạc thì có. **Đừng tái lập trần từ cho nhóm này.**
>
> Thay bằng **kiểm tra chức năng** — mỗi phần phải có việc của nó. Xoá phần nào mà đáp án *không mất gì* thì phần đó thừa. Dấu hiệu thật sự cần cắt: lặp ý đã nói ở phần trên · giải thích kiến thức nền không liên quan tới câu hỏi · liệt kê cho đủ thay vì cho đúng · sa đà chi tiết API mà interviewer không hỏi. **Dài vì nhiều ý ≠ dài dòng.**

**Khung 5 phần cho câu 🟠🔴 (và 🟡 nặng cơ chế)** — mẫu tham chiếu: **[OS-012](os.md)** (condition variable), [CPP-052](cpp.md) (shared_ptr 3 tầng).

1. **Cơ chế trước** — thứ gì thực sự xảy ra, đánh số bước nếu là quy trình. Hiểu bước máy chạy thì tự suy ra phần còn lại.
2. **"Vì sao" tách tầng nông / sâu** — hầu hết câu có một lý do ai cũng nói được và một lý do thật sự phân biệt ứng viên. Nêu **cả hai**, gắn nhãn rõ.
3. **Bảng phân biệt các ca** — khi đáp án có ≥3 trường hợp / mức / lựa chọn, dùng **bảng** thay vì câu văn liệt kê. Quét mắt được khi ôn.
4. **Code ngắn khi API dễ viết nhầm** — đặt dạng **đúng và sai cạnh nhau**, comment `// ✅ / // ❌`. Vài dòng, không phải chương trình chạy được.
5. **Bẫy + câu chốt** — nêu thẳng **hiểu lầm phổ biến** (vd *"count là atomic nên shared_ptr an toàn"*), rồi một dòng **Chốt:** bật ra được ngay khi bị hỏi thật.

**Bắt buộc:** đối chiếu tài liệu topic gốc khi viết/sửa, **không viết từ trí nhớ** — nếu không bank sẽ trôi lệch khỏi [01–13](../../../). Link `→ nguồn` phải trỏ đúng mục đã dùng.

**Không** làm: nhồi mọi thứ biết được; lặp lại nguyên văn tài liệu gốc (bank là *bản rút gọn có cấu trúc*, không phải bản sao); giãn câu 🟢.

## Cách phiên dùng bank

Interviewer lọc câu theo **track** ([../tracks.md](../tracks.md) → domain) × **type/level** ([../interview-types.md](../interview-types.md)), ưu tiên chèn câu trong [../weak-register.md](../weak-register.md). "Rapid-fire" = rút câu `concept` 🟢🟡. Xem giao thức đầy đủ trong [../config.md](../config.md).

## Đếm câu

| Domain | Dải ID đã dùng |
|---|---|
| CPP | 001–055 (040–051 = track `emc`, neo theo Item; 052 = shared_ptr 3 tầng thread-safety; 053 = safe-bool / `explicit operator bool`; 054 = move ctor vs move assign; 055 = chuyển ngầm hai chiều + nhập nhằng) |
| OS | 001–021 (020 = thread-safe vs reentrant, 021 = `fork()` trong chương trình đa luồng) |
| LNX | 001–029 (027 = `EINTR`/`SA_RESTART`, 028 = `O_APPEND` vs `lseek`+`write`, 029 = chọn clock đo thời gian) |
| DRV | 001–027 (019–027 = PCI/USB) |
| BUS | 001–004 |
| EMB | 001–032 |
| BSP | 001–027 |
| BLD | 001–010 (CMake/Yocto/CI) |
| DBG | 001–019 |
| DP | 001–016 (014 = Meyers thread-safe/DCLP, 015 = object pool, 016 = decorator) |
| DSA | 001–014 (013 = chọn sức chứa N của ring buffer, 014 = consumer biết mình mất dữ liệu — cả hai 🎤 2026-08-09) |
| NET | 001–012 |
| SD | 001–016 (012 = OTA, 013 = barcode scanner, 014 = UART framing, 015 = driver subsystem, 016 = memory tất định) |
| BEH | 001–009 |
| COD | 001–010 |

> Khi thêm câu mới, tăng số cuối dải tương ứng và cập nhật bảng này.
