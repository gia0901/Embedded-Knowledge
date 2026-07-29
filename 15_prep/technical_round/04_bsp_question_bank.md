# 📙 FILE 4 — Ngân hàng câu hỏi BSP chuyên sâu → đã gộp vào bank duy nhất

> ➡️ Toàn bộ câu hỏi BSP chuyên sâu (boot chain, device tree/probe, interrupt/DMA/MMIO, flash & OTA, Yocto, power, real-time, bring-up) đã **gộp về ngân hàng câu hỏi duy nhất**, domain **`BSP`**: **[../mock-interview/bank/bsp.md](../mock-interview/bank/bsp.md)**.

## Bản đồ chuyển hướng theo phần cũ

| Phần cũ (FILE 4) | ID trong bank | Đọc sâu |
|---|---|---|
| A — Boot chain & bootloader | `BSP-002`…`BSP-005` | [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) |
| B — Kernel & device tree | `BSP-006`…`BSP-009` | [melp/bootloader-kernel](../../16-book-summaries/melp/bootloader-kernel.md) |
| C — Interrupt, DMA, MMIO | `BSP-010`…`BSP-013` | [melp/drivers-init-power](../../16-book-summaries/melp/drivers-init-power.md) |
| D — Storage flash & OTA | `BSP-014`…`BSP-016` | [melp/storage-update](../../16-book-summaries/melp/storage-update.md) |
| E — Build system & Yocto | `BSP-017`…`BSP-019` | [melp/build-systems](../../16-book-summaries/melp/build-systems.md) |
| F — Power management | `BSP-020` | [melp/drivers-init-power](../../16-book-summaries/melp/drivers-init-power.md) |
| G — Real-time | `BSP-021`, `OS-015` (priority inversion) | [melp/debug-realtime](../../16-book-summaries/melp/debug-realtime.md) |
| H — Tình huống bring-up | `BSP-022`…`BSP-025` | theo từng câu |

## Dùng gì thay thế
- **Phỏng vấn thử BSP:** [mock interview](../mock-interview/) — `/mock`, track `bsp` hoặc `melp`, type `deep-dive`/`comprehensive`.
- Nền chung (C++/IPC/bus/GDB): xem các domain khác trong [bank/](../mock-interview/bank/). Lý thuyết: [01_theory.md](01_theory.md).
