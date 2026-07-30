# 🏦 Ngân hàng câu hỏi — DUY NHẤT

> **Đây là nguồn câu hỏi duy nhất của toàn repo.** Mọi bộ câu hỏi cũ ([11-interview-questions/](../../../11-interview-questions/), [technical_round/](../../technical_round/)) đã gộp về đây và giờ chỉ là con trỏ trỏ vào bank. Không tạo bank thứ hai; câu mới luôn thêm vào đây.
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
| `BSP` | [bsp.md](bsp.md) | BSP chuyên sâu: boot/DT probe/IRQ/DMA/flash/OTA/Yocto/power/RT/bring-up | [16 melp](../../../16-book-summaries/melp/) |
| `DBG` | [debugging.md](debugging.md) | Debugging & tools | [09](../../../09-debugging/) |
| `DP` | [design-patterns.md](design-patterns.md) | Design patterns & SOLID | [12](../../../12-design-patterns/) |
| `DSA` | [dsa.md](dsa.md) | Cấu trúc dữ liệu & giải thuật | [13](../../../13-dsa/) |
| `NET` | [networking.md](networking.md) | Networking | [14](../../../14-networking/) |
| `SD` | [system-design.md](system-design.md) | Tư duy & system design | [10](../../../10-thinking/), [07](../../../07-shared-libraries/) |
| `BEH` | [behavioral.md](behavioral.md) | Behavioral / HR (STAR) | — |
| `COD` | [coding.md](coding.md) | Bài coding tại chỗ | [13](../../../13-dsa/) |

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

## Cách phiên dùng bank

Interviewer lọc câu theo **track** ([../tracks.md](../tracks.md) → domain) × **type/level** ([../interview-types.md](../interview-types.md)), ưu tiên chèn câu trong [../weak-register.md](../weak-register.md). "Rapid-fire" = rút câu `concept` 🟢🟡. Xem giao thức đầy đủ trong [../config.md](../config.md).

## Đếm câu

| Domain | Dải ID đã dùng |
|---|---|
| CPP | 001–051 (040–051 = track `emc`, neo theo Item) |
| OS | 001–019 |
| LNX | 001–026 |
| DRV | 001–018 |
| BUS | 001–004 |
| EMB | 001–032 |
| BSP | 001–027 |
| DBG | 001–019 |
| DP | 001–013 |
| DSA | 001–012 |
| NET | 001–012 |
| SD | 001–011 |
| BEH | 001–009 |
| COD | 001–010 |

> Khi thêm câu mới, tăng số cuối dải tương ứng và cập nhật bảng này.
