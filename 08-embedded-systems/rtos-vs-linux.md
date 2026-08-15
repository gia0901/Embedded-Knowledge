# RTOS vs Linux — Chọn hệ điều hành cho embedded

> **TL;DR**
> - **Realtime** không nghĩa là "nhanh" mà là **tất định (deterministic)**: đảm bảo đáp ứng trong một deadline xác định, *luôn luôn*.
> - **Hard realtime**: lỡ deadline = thất bại hệ thống (airbag, điều khiển động cơ). **Soft realtime**: lỡ deadline làm giảm chất lượng nhưng chấp nhận được (streaming video).
> - **RTOS** (FreeRTOS, Zephyr, QNX...): nhỏ, tất định, độ trễ ngắt thấp & dự đoán được, ít tính năng — cho điều khiển hard realtime trên MCU.
> - **Linux**: nhiều tính năng (mạng, filesystem, process, driver phong phú), nhưng kernel thường không tất định (jitter); cần **PREEMPT_RT** để cải thiện latency.
> - Nhiều hệ thực tế **kết hợp**: MCU/RTOS lo phần realtime cứng + MPU/Linux lo giao diện/mạng (heterogeneous, vd AMP).

---

## 1. Realtime nghĩa là gì? (hiểu sai phổ biến)

Realtime **≠ tốc độ cao**. Realtime = **tính tất định về thời gian**: hệ thống đảm bảo hoàn thành tác vụ trong một **deadline** xác định, một cách *có thể dự đoán được* mọi lúc. Một hệ chạy chậm nhưng luôn đáp ứng trong 10ms "realtime" hơn một hệ thường nhanh nhưng thỉnh thoảng trễ 200ms.

Chỉ số quan trọng: **worst-case latency** và **jitter** (độ dao động của thời gian đáp ứng), không phải throughput trung bình. Vẽ phân bố latency ra là thấy ngay vì sao "trung bình" đánh lừa:

```
  số lần
  đo được
    ▲
    │     ██                      Linux thường (không PREEMPT_RT)
    │    ████                     trung bình: 20 µs  ← nhìn rất đẹp
    │   ██████                    worst-case: 8000 µs ← nhưng ĐUÔI kéo dài
    │  ████████ ▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁█
    └──┴───────┴──────────────────────────────────────────────────┴──► latency
      10µs    50µs                                              8ms
                                                          ▲
                                             CHÍNH CÁI ĐUÔI NÀY giết bạn
                                             (deadline 1ms → trượt, dù 99.99% lần đo đều đạt)

    │      ██                     RTOS (hoặc Linux PREEMPT_RT + tuning)
    │     ████                    trung bình: 30 µs  ← CHẬM HƠN trung bình!
    │     ████                    worst-case: 80 µs  ← nhưng CÓ CHẶN TRÊN
    │    ██████│
    └────┴─────┴──────────────────────────────────────────────────────► latency
       20µs   80µs ◄── hết. Không có đuôi.
```

> **Hệ thứ hai "realtime" hơn dù trung bình chậm hơn.** Realtime không đo bằng *nhanh bao nhiêu* mà bằng *chậm nhất là bao nhiêu, và con số đó có chặn không*.

Đo thật trên Linux bằng `cyclictest` — nhìn cột **Max**, không phải Avg:

```bash
# chạy tối thiểu vài giờ; ép tải nặng song song để ép ra worst-case
sudo cyclictest -t1 -p 80 -i 1000 -m -D 4h
# T: 0 ( 1234) P:80 I:1000 C: 14400000 Min:  8 Act: 12 Avg: 14 Max:  67
#                                            ▲                    ▲
#                                    đẹp nhưng vô nghĩa      SỐ DUY NHẤT ĐÁNG TIN
```

| Loại | Hậu quả lỡ deadline | Ví dụ |
|------|---------------------|-------|
| **Hard realtime** | Hỏng hệ thống / nguy hiểm | Airbag, ABS phanh, điều khiển động cơ, máy tạo nhịp |
| **Firm realtime** | Kết quả vô dụng nhưng không nguy hiểm | Một số hệ điều khiển công nghiệp |
| **Soft realtime** | Giảm chất lượng, vẫn chấp nhận | Streaming, audio, UI |

---

## 2. RTOS — tối ưu cho tất định

RTOS (Real-Time Operating System: FreeRTOS, Zephyr, ThreadX, QNX, VxWorks) thiết kế để **tất định**:
- **Preemptive priority scheduling**: tác vụ ưu tiên cao **luôn** chạy ngay khi sẵn sàng, latency cực thấp & biết trước.
- **Độ trễ ngắt (interrupt latency) nhỏ và bị chặn (bounded)**.
- **Priority inheritance** để tránh priority inversion ([03/scheduling](../03-operating-system/scheduling.md)).
- Nhỏ gọn (KB), chạy trên MCU không MMU; thường **không bảo vệ bộ nhớ** giữa task (hoặc dùng MPU).
- Ít tính năng: không có process tách biệt, filesystem/mạng đầy đủ là tùy chọn thêm.

→ Phù hợp: điều khiển vòng kín tần số cao, xử lý tín hiệu deadline cứng, thiết bị tài nguyên rất hạn chế.

---

## 3. Linux — nhiều tính năng, không tất định mặc định

Linux mạnh về **tính năng & hệ sinh thái**: multi-process với bảo vệ bộ nhớ (MMU), networking đầy đủ, filesystem, hàng nghìn driver, ngôn ngữ/thư viện phong phú. Nhưng kernel chuẩn **không tất định** cho hard realtime:
- Nhiều nguồn gây trễ không lường: vùng không thể preempt trong kernel, xử lý interrupt/softirq, page fault, hoạt động của scheduler, contention khóa.
- → **jitter** lớn ở worst case dù throughput cao.

Cải thiện:
- **`PREEMPT_RT`** (nay đã mainline phần lớn): biến hầu hết kernel thành preemptible, chuyển interrupt handler thành thread, dùng mutex có priority inheritance → giảm worst-case latency xuống mức hàng chục–trăm µs, đủ cho nhiều soft/firm realtime.
- **`SCHED_FIFO`/`SCHED_RR`**, CPU isolation (`isolcpus`), khóa bộ nhớ (`mlockall`), tránh page fault.

→ Linux phù hợp: cần mạng/giao diện/xử lý phức tạp, soft realtime; không lý tưởng cho hard realtime khắt khe nếu không có RTOS hỗ trợ.

---

## 4. So sánh

| Tiêu chí | RTOS | Linux |
|----------|------|-------|
| Tất định / worst-case latency | Rất tốt, bounded | Kém (tốt hơn với PREEMPT_RT) |
| Footprint (RAM/flash) | Nhỏ (KB–vài trăm KB) | Lớn (MB+) |
| MMU / bảo vệ bộ nhớ | Thường không (MPU) | Có (cô lập process) |
| Tính năng (mạng, FS, driver) | Tối thiểu, thêm dần | Rất phong phú |
| Mô hình | Task + scheduler đơn giản | Process/thread + userspace đầy đủ |
| Phát triển | Gần phần cứng, ít trừu tượng | Hệ sinh thái lớn, dễ tái dùng code |
| Phù hợp | Hard realtime, MCU, tài nguyên ít | App phức tạp, mạng/UI, soft realtime |

---

## 5. Kiến trúc kết hợp (thực tế phổ biến)

Nhiều sản phẩm dùng **cả hai** trên một hệ heterogeneous:
- **MCU/RTOS** (vd Cortex-M) xử lý phần **hard realtime** (điều khiển motor, sampling sensor, an toàn).
- **MPU/Linux** (vd Cortex-A) xử lý **giao diện, mạng, logic phức tạp, cập nhật**.
- Giao tiếp giữa hai bên qua shared memory/mailbox/RPMsg (AMP — Asymmetric Multi-Processing).

Ví dụ: SoC như STM32MP1, i.MX có sẵn cả core Cortex-A (Linux) và Cortex-M (RTOS). Bare-metal cũng là lựa chọn cho thiết bị cực đơn giản (không cần OS).

---

## 6. Bare-metal — không OS

Với thiết bị rất nhỏ/đơn giản, có thể chạy **bare-metal** (super-loop hoặc interrupt-driven, không OS):
```c
int main() {
    init_hardware();
    while (1) {          // super loop
        read_sensors();
        compute();
        update_outputs();
    }
}
```
Ưu: đơn giản nhất, tất định, footprint tối thiểu. Nhược: khó mở rộng khi logic phức tạp (không có scheduler, đa nhiệm phải tự quản qua state machine + interrupt). RTOS ra đời để giải quyết khi bare-metal trở nên khó quản.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [EMB-034](../14-prep/mock-interview/bank/embedded-fundamentals.md) | "Realtime" nghĩa là gì? Có phải là chạy nhanh không? |
| [EMB-034](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Phân biệt hard realtime và soft realtime. |
| [DRV-015](../14-prep/mock-interview/bank/drivers-embedded.md) | Khi nào chọn RTOS, khi nào chọn Linux? |
| [BSP-021](../14-prep/mock-interview/bank/bsp.md) | Vì sao Linux kernel mặc định không phù hợp hard realtime? PREEMPT_RT giúp gì? |
| [BSP-028](../14-prep/mock-interview/bank/bsp.md) | Kiến trúc kết hợp RTOS + Linux hoạt động thế nào và vì sao dùng? |

---
⬅️ [boot-process.md](boot-process.md) · ➡️ Tiếp theo: [constraints.md](constraints.md)
