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

Chỉ số quan trọng: **worst-case latency** và **jitter** (độ dao động của thời gian đáp ứng), không phải throughput trung bình.

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

<details><summary>1) "Realtime" nghĩa là gì? Có phải là chạy nhanh không?</summary>

Không — realtime nghĩa là **tất định về thời gian**: hệ thống đảm bảo hoàn thành tác vụ trong một deadline xác định, một cách dự đoán được trong **mọi** trường hợp, kể cả worst case. Một hệ realtime quan tâm tới worst-case latency và jitter (độ dao động thời gian đáp ứng), không phải throughput hay tốc độ trung bình. Một hệ chậm nhưng luôn đáp ứng trong giới hạn được coi là realtime hơn một hệ nhanh trung bình nhưng đôi khi trễ bất ngờ. Tốc độ giúp dễ đạt deadline nhưng không phải là bản chất của realtime.
</details>

<details><summary>2) Phân biệt hard realtime và soft realtime.</summary>

Hard realtime: lỡ deadline được coi là **thất bại của hệ thống**, có thể gây nguy hiểm hoặc hư hỏng — ví dụ điều khiển airbag, ABS, động cơ, máy tạo nhịp tim; những hệ này yêu cầu đảm bảo tuyệt đối về worst-case timing. Soft realtime: lỡ deadline làm **giảm chất lượng** nhưng vẫn chấp nhận được, không gây hậu quả nghiêm trọng — ví dụ streaming video/audio, giao diện người dùng (thỉnh thoảng giật vẫn dùng được). (Có thêm mức firm: lỡ deadline khiến kết quả vô dụng nhưng không nguy hiểm.) Phân loại này quyết định lựa chọn OS và mức độ nghiêm ngặt khi thiết kế.
</details>

<details><summary>3) Khi nào chọn RTOS, khi nào chọn Linux?</summary>

Chọn RTOS khi cần tính tất định cao (hard realtime), worst-case latency nhỏ và bị chặn, chạy trên MCU tài nguyên hạn chế (không MMU, RAM/flash ít), và yêu cầu tính năng tối thiểu — vd điều khiển vòng kín tần số cao, xử lý tín hiệu deadline cứng, hệ an toàn. Chọn Linux khi cần hệ sinh thái phong phú: networking đầy đủ, filesystem, đa process với bảo vệ bộ nhớ, nhiều driver/thư viện, giao diện phức tạp — và yêu cầu realtime chỉ ở mức soft/firm (hoặc dùng PREEMPT_RT để cải thiện). Nếu vừa cần hard realtime vừa cần tính năng phức tạp, thường kết hợp cả hai trên hệ heterogeneous.
</details>

<details><summary>4) Vì sao Linux kernel mặc định không phù hợp hard realtime? PREEMPT_RT giúp gì?</summary>

Linux tối ưu cho throughput và sử dụng tài nguyên trung bình, không cho worst-case latency. Có nhiều nguồn gây trễ không lường trước: các vùng không thể preempt trong kernel, xử lý interrupt và softirq, page fault, hoạt động cân bằng tải của scheduler, tranh chấp khóa — gây jitter lớn ở worst case dù hiệu năng trung bình cao, nên không đảm bảo deadline cứng. PREEMPT_RT (phần lớn đã được merge vào mainline) biến gần như toàn bộ kernel thành preemptible, chuyển phần lớn interrupt handler thành các kernel thread có thể lập lịch, và thay nhiều khóa bằng mutex có priority inheritance; kết quả là worst-case latency giảm xuống mức hàng chục đến trăm microsecond, đủ cho nhiều ứng dụng soft/firm và một số hard realtime ít khắt khe.
</details>

<details><summary>5) Kiến trúc kết hợp RTOS + Linux hoạt động thế nào và vì sao dùng?</summary>

Trên SoC heterogeneous (vd STM32MP1, i.MX có cả core Cortex-A và Cortex-M), người ta chạy Linux trên core ứng dụng (Cortex-A) để lo giao diện, mạng, logic phức tạp và cập nhật, đồng thời chạy RTOS hoặc bare-metal trên core realtime (Cortex-M) để xử lý phần hard realtime như điều khiển motor, lấy mẫu sensor, chức năng an toàn — nơi cần tất định mà Linux không đảm bảo. Hai bên giao tiếp qua shared memory, mailbox hoặc giao thức như RPMsg (mô hình AMP — Asymmetric Multi-Processing). Cách này tận dụng thế mạnh của cả hai: tính năng phong phú của Linux và tính tất định của RTOS, thay vì phải thỏa hiệp với một hệ duy nhất.
</details>

---
⬅️ [boot-process.md](boot-process.md) · ➡️ Tiếp theo: [constraints.md](constraints.md)
