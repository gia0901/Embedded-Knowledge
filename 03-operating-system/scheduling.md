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

## 8. 💰 Con số & ⚠️ bẫy

**Mốc cần nhớ trên Linux:**

| Thứ | Giá trị điển hình | Vì sao đáng nhớ |
|---|---|---|
| Ưu tiên realtime | **1–99** (99 cao nhất) | Mọi task RT **luôn** thắng mọi task `SCHED_OTHER`, bất kể `nice` |
| `nice` | **−20 … +19** | Chỉ đổi **trọng số** `vruntime`, **không** phải ưu tiên tuyệt đối |
| Trọng số theo `nice` | mỗi bậc ≈ **×1,25** CPU | Chênh 10 bậc ≈ **×10** thị phần CPU |
| Timer tick | 100 / 250 / **1000 Hz** | Quyết định độ hạt của lập lịch và timer |
| Context switch | ~1–5 µs | Time slice quá nhỏ ⇒ chi phí switch ăn hết ([process-thread.md](process-thread.md)) |

**⚠️ Bẫy:**

**① `SCHED_FIFO` + vòng lặp bận = TREO CẢ MÁY.** Task FIFO chạy **tới khi tự nhường**; ở ưu tiên cao nó không cho cả shell của bạn chạy. ⇒ Dùng RT thì **luôn** để lối thoát: `RLIMIT_RTTIME`, watchdog, hoặc chừa một core không dành cho RT.

**② Chuyển sang realtime KHÔNG chữa được phần lớn nguyên nhân trễ.** Nó chỉ chữa ca *bị task khác giành CPU*. Trễ do **chờ I/O** (`D` state), **page fault**, hay **priority inversion** thì đổi lớp lịch vô ích — thậm chí làm tệ hơn. ⇒ **Đo trước, đổi sau** ([OS-026](../14-prep/mock-interview/bank/os.md)).

**③ Realtime + mutex thường = priority inversion nặng hơn.** Task RT chờ khoá do task thường giữ, mà task thường **không bao giờ được lịch** để nhả khoá ⇒ kẹt vĩnh viễn. ⇒ Dùng RT thì mutex **bắt buộc** bật **priority inheritance** (`PTHREAD_PRIO_INHERIT`) — §6.

**④ `nice` không đảm bảo gì cả.** `nice -20` vẫn phải chia CPU với mọi tiến trình khác; nó **không** là ưu tiên cứng, và **không** giúp gì cho độ trễ đuôi.

**⑤ Ưu tiên tiến trình I/O-bound không phải "thiên vị".** Chúng chạy rất ngắn rồi ngủ; cho chạy trước gần như miễn phí mà giữ được thiết bị luôn bận ⇒ **tăng tổng thông lượng**. CFS đạt việc này tự nhiên: ngủ thì `vruntime` đứng yên ([OS-025](../14-prep/mock-interview/bank/os.md), [OS-016](../14-prep/mock-interview/bank/os.md)).

**⑥ Đo độ trễ bằng `CLOCK_REALTIME` là sai** — NTP chỉnh giờ làm phép đo ra số âm hoặc timeout hàng giờ. Dùng **`CLOCK_MONOTONIC`** ([LNX-029](../14-prep/mock-interview/bank/linux-sysprog.md)).

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [EMB-014](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Preemptive và cooperative scheduling khác nhau thế nào? |
| [OS-016](../14-prep/mock-interview/bank/os.md) | CFS hoạt động theo nguyên lý nào? |
| [OS-025](../14-prep/mock-interview/bank/os.md) | CPU-bound và I/O-bound khác nhau ra sao? Scheduler nên ưu tiên loại nào? |
| [OS-015](../14-prep/mock-interview/bank/os.md) | Priority inversion là gì và khắc phục thế nào? |
| [OS-026](../14-prep/mock-interview/bank/os.md) | Linux có những scheduling class nào? Khi nào dùng realtime? |
| [OS-027](../14-prep/mock-interview/bank/os.md) | Starvation là gì? Aging giải quyết ra sao? |

---
⬅️ [process-thread.md](process-thread.md) · ➡️ Tiếp theo: [memory-management.md](memory-management.md)
