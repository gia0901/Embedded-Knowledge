# Scheduling — Lập lịch CPU

> **TL;DR**
> - **Scheduler** quyết định process/thread nào chạy trên CPU và bao lâu, nhằm cân bằng: thông lượng (throughput), độ trễ (latency), công bằng (fairness), đáp ứng (responsiveness).
> - **Preemptive** (OS có thể ngắt tác vụ đang chạy — Linux) vs **cooperative** (tác vụ tự nhường — dễ bị treo nếu một tác vụ tham lam).
> - Linux desktop/server dùng **CFS** (Completely Fair Scheduler) — chia CPU công bằng theo trọng số (nice). Kernel mới hơn dùng **EEVDF**.
> - **Realtime** (embedded quan trọng): `SCHED_FIFO`, `SCHED_RR` — ưu tiên cao, tất định; nguy cơ **priority inversion** → cần priority inheritance.
> - Khái niệm phải nắm: CPU-bound vs I/O-bound, time slice/quantum, context switch overhead, starvation.

---

## 1. Vì sao cần scheduler?

Số process/thread thường nhiều hơn số core CPU. Scheduler là phần kernel quyết định *ai chạy, khi nào, bao lâu* để máy vừa tận dụng CPU vừa phản hồi tốt. Các mục tiêu thường **mâu thuẫn**:

| Mục tiêu | Ý nghĩa |
|----------|---------|
| Throughput | Số tác vụ hoàn thành/đơn vị thời gian |
| Latency / Response time | Thời gian từ lúc sẵn sàng đến lúc được chạy |
| Fairness | Không tác vụ nào bị bỏ đói (starvation) |
| Determinism | Tính tất định (quan trọng cho realtime) |

Tối ưu cái này thường hy sinh cái khác → mỗi loại hệ thống chọn chiến lược khác nhau.

---

## 2. Preemptive vs Cooperative

- **Cooperative (non-preemptive)**: tác vụ chạy đến khi tự nguyện nhường CPU (yield, hoặc block I/O). Đơn giản, ít context switch, nhưng một tác vụ "tham lam" hoặc bị bug vòng lặp vô tận → treo cả hệ thống. (RTOS đơn giản, một số coroutine runtime.)
- **Preemptive**: OS dùng **timer interrupt** để giành lại CPU sau một **time slice (quantum)**, hoặc khi tác vụ ưu tiên cao hơn sẵn sàng. Linux và đa số OS hiện đại dùng cách này — đảm bảo công bằng và đáp ứng.

---

## 3. Các thuật toán lập lịch cơ bản (nền tảng lý thuyết)

| Thuật toán | Ý tưởng | Ưu / Nhược |
|-----------|---------|-----------|
| **FCFS** (First Come First Served) | Hàng đợi theo thứ tự đến | Đơn giản; convoy effect (tác vụ dài chặn tác vụ ngắn) |
| **SJF** (Shortest Job First) | Chạy tác vụ ngắn nhất trước | Tối ưu thời gian chờ TB; cần biết trước thời lượng, gây starvation tác vụ dài |
| **Round Robin (RR)** | Mỗi tác vụ một time slice rồi xoay vòng | Công bằng, đáp ứng tốt; quantum nhỏ → nhiều context switch |
| **Priority** | Ưu tiên cao chạy trước | Linh hoạt; starvation tác vụ ưu tiên thấp → cần **aging** |
| **MLFQ** (Multi-Level Feedback Queue) | Nhiều hàng đợi theo mức ưu tiên, tác vụ "tụt hạng" nếu dùng nhiều CPU | Tự phân loại I/O-bound vs CPU-bound; phức tạp |

---

## 4. CPU-bound vs I/O-bound

- **CPU-bound**: chủ yếu tính toán, dùng hết time slice (vd nén dữ liệu, xử lý ảnh).
- **I/O-bound**: thường block chờ I/O, dùng ít CPU mỗi lần (vd đọc file, chờ mạng).

Scheduler tốt **ưu tiên I/O-bound** chạy ngay khi sẵn sàng (chúng sẽ nhanh chóng block lại) → giữ thiết bị I/O bận và hệ thống mượt, trong khi CPU-bound lấp khoảng trống. Đây là lý do hệ thống tương tác cần ưu tiên tác vụ hay ngủ.

---

## 5. Lập lịch trên Linux

Linux chia thành các **scheduling class**, xét theo độ ưu tiên giảm dần:

1. **Realtime**: `SCHED_FIFO`, `SCHED_RR` (ưu tiên 1–99). Luôn được chạy trước tác vụ thường.
   - `SCHED_FIFO`: chạy đến khi tự block/nhường hoặc bị tác vụ RT ưu tiên cao hơn preempt — không có time slice.
   - `SCHED_RR`: như FIFO nhưng có time slice xoay vòng giữa các tác vụ cùng mức ưu tiên.
2. **Normal**: `SCHED_OTHER` (CFS/EEVDF) — đa số tiến trình bình thường.
   - **CFS** (Completely Fair Scheduler): mô phỏng "CPU đa nhiệm lý tưởng" — theo dõi **vruntime** (thời gian chạy ảo) của mỗi tác vụ và luôn chọn tác vụ có vruntime nhỏ nhất → chia CPU công bằng. **nice** (-20..+19) điều chỉnh trọng số (nice thấp = ưu tiên cao). Kernel ≥6.6 thay bằng **EEVDF**.
3. **Idle**: `SCHED_IDLE` — chỉ chạy khi không còn gì khác.

> **Embedded/realtime:** dùng `SCHED_FIFO`/`SCHED_RR` cho tác vụ cần tất định (vd điều khiển động cơ, xử lý tín hiệu). Cân nhắc kernel có **PREEMPT_RT** patch để giảm latency. Phải cẩn thận: tác vụ RT chạy mãi có thể bỏ đói tác vụ thường.

---

## 6. Priority inversion — bẫy realtime kinh điển

```
Tác vụ ưu tiên THẤP (L) đang giữ một mutex.
Tác vụ ưu tiên CAO (H) cần mutex đó → phải chờ L.
Tác vụ ưu tiên TRUNG BÌNH (M) (không liên quan) preempt L.
→ H (cao nhất) gián tiếp bị M (trung bình) chặn vô thời hạn. ❌
```

Đây là **priority inversion** (sự cố nổi tiếng trên tàu Mars Pathfinder 1997). Giải pháp:
- **Priority inheritance**: khi H chờ mutex mà L đang giữ, L được *thừa kế tạm* ưu tiên của H để chạy xong và nhả lock nhanh. (Linux: `PTHREAD_PRIO_INHERIT`.)
- **Priority ceiling**: mutex có mức ưu tiên trần; ai giữ nó được nâng lên mức đó.

---

## 7. Khái niệm liên quan

- **Time slice/quantum**: lượng thời gian tối đa một tác vụ chạy trước khi bị preempt. Nhỏ → đáp ứng tốt nhưng nhiều context switch; lớn → ngược lại.
- **Starvation**: tác vụ không bao giờ được chạy (vd ưu tiên thấp mãi). Khắc phục bằng **aging** (tăng dần ưu tiên theo thời gian chờ).
- **Load balancing** (đa nhân): phân bố tác vụ giữa các core; cân nhắc **CPU affinity** (giữ tác vụ trên core cũ để tận dụng cache nóng).

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Preemptive và cooperative scheduling khác nhau thế nào?</summary>

Cooperative (non-preemptive): tác vụ chạy đến khi tự nguyện nhường CPU (yield hoặc block I/O); đơn giản, ít context switch nhưng một tác vụ tham lam/bug có thể treo cả hệ thống. Preemptive: OS chủ động giành lại CPU qua timer interrupt sau một time slice hoặc khi có tác vụ ưu tiên cao hơn; đảm bảo công bằng và đáp ứng, là cách Linux và hầu hết OS hiện đại dùng. Đánh đổi: đơn giản/tin tưởng tác vụ vs an toàn/phức tạp hơn.
</details>

<details><summary>2) CFS hoạt động theo nguyên lý nào?</summary>

CFS (Completely Fair Scheduler) mô phỏng một CPU đa nhiệm "lý tưởng" chia đều cho mọi tác vụ. Nó theo dõi **vruntime** — thời gian CPU ảo mỗi tác vụ đã dùng (có trọng số theo giá trị nice) — và luôn chọn chạy tác vụ có vruntime nhỏ nhất, dùng cấu trúc cây đỏ-đen để tìm nhanh. Tác vụ nice thấp (ưu tiên cao) có vruntime tăng chậm hơn nên được nhiều CPU hơn. Kết quả là chia sẻ CPU công bằng theo trọng số thay vì time slice cố định. (Kernel mới thay bằng EEVDF.)
</details>

<details><summary>3) CPU-bound và I/O-bound khác nhau ra sao? Scheduler nên ưu tiên loại nào?</summary>

CPU-bound dùng hết time slice cho tính toán; I/O-bound thường block chờ I/O và dùng ít CPU mỗi lần. Scheduler tốt nên ưu tiên cho I/O-bound được chạy nhanh khi sẵn sàng, vì chúng sẽ sớm block lại — như vậy giữ thiết bị I/O luôn bận và hệ thống phản hồi mượt, còn tác vụ CPU-bound lấp các khoảng CPU rảnh. Đây là lý do tác vụ tương tác (hay ngủ) được ưu tiên hơn tác vụ tính toán nặng.
</details>

<details><summary>4) Priority inversion là gì và khắc phục thế nào?</summary>

Priority inversion xảy ra khi một tác vụ ưu tiên cao bị chặn gián tiếp bởi tác vụ ưu tiên thấp hơn: tác vụ thấp (L) giữ một mutex mà tác vụ cao (H) cần; trong khi đó tác vụ trung bình (M) không liên quan lại preempt L, khiến L không nhả được lock và H bị chặn vô thời hạn bởi M. Khắc phục: **priority inheritance** (L tạm thừa kế ưu tiên của H để chạy xong và nhả lock nhanh) hoặc **priority ceiling** (nâng ai giữ mutex lên mức trần định trước). Đây là sự cố nổi tiếng của Mars Pathfinder.
</details>

<details><summary>5) Linux có những scheduling class nào? Khi nào dùng realtime?</summary>

Theo ưu tiên giảm dần: lớp **realtime** (`SCHED_FIFO`, `SCHED_RR`, ưu tiên 1–99) luôn chạy trước; **normal** (`SCHED_OTHER`, dùng CFS/EEVDF) cho tiến trình thường, điều chỉnh bằng nice; **idle** (`SCHED_IDLE`) chỉ chạy khi rảnh. Dùng realtime cho tác vụ cần tất định về thời gian (điều khiển, xử lý tín hiệu, audio): `SCHED_FIFO` chạy đến khi tự nhường/bị RT cao hơn preempt, `SCHED_RR` thêm time slice xoay vòng cùng mức. Phải cẩn thận vì tác vụ RT có thể bỏ đói tác vụ thường; embedded khắt khe thường dùng kernel PREEMPT_RT.
</details>

<details><summary>6) Starvation là gì? Aging giải quyết ra sao?</summary>

Starvation là khi một tác vụ không bao giờ (hoặc rất lâu mới) được cấp CPU — thường do luôn có tác vụ ưu tiên cao hơn trong hệ priority-based, hoặc tác vụ dài bị các tác vụ ngắn chen mãi (SJF). Aging khắc phục bằng cách **tăng dần độ ưu tiên** của tác vụ theo thời gian nó chờ; chờ càng lâu ưu tiên càng cao, cuối cùng chắc chắn được chạy, đảm bảo tiến triển (progress) cho mọi tác vụ.
</details>

---
⬅️ [process-thread.md](process-thread.md) · ➡️ Tiếp theo: [memory-management.md](memory-management.md)
