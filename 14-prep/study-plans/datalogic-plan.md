# 🎯 Datalogic — Embedded Linux Engineer (C++17, Linux) · Plan 4 tuần

> Kế hoạch ôn **nhắm đúng JD** [Embedded Linux Engineer @ Datalogic Việt Nam](https://itviec.com/it-jobs/embedded-linux-engineer-c-linux-datalogic-viet-nam-2835). Dùng chung cả **4 tuần** (nền vững) lẫn **2 tuần** (nước rút — **gộp đôi khối lượng mỗi ngày**: làm 2 buổi/ngày thay vì 1).
> Không phải kiến thức mới — mọi mục **link tới tài liệu + bank câu hỏi** sẵn có. Ký hiệu: `- [ ]` chưa xong · `- [x]` xong · 🎯 xác suất gặp cao · 🎤 phiên mock · 🔁 ôn lặp lại.

---

## 📍 Tiến độ hiện tại — **RESUME Ở ĐÂY** (nguồn tracking DUY NHẤT, cập nhật mỗi buổi)

> **Conversation mới đọc block này TRƯỚC TIÊN** để biết đang ở đâu và làm gì tiếp. Đây là plan **đang chạy** và là **nguồn lịch duy nhất** (không còn daily-plan hay plan role nào khác).

- **Plan đang chạy:** 🎯 Datalogic (file này) · bắt đầu **2026-08-03** (T2). ⚠️ **Trễ 2 ngày**: B5 lẽ ra T6 07/08, thực tế chạy CN 09/08 → **CN và B5 dồn vào cùng ngày**. Tuần 2 nên bắt đầu **T2 10/08**; nếu không kịp buổi CN thì gộp nó vào khởi động buổi đầu Tuần 2, đừng đẩy cả plan lùi.
- **Vị trí:** ✅ **HẾT Tuần 1** (C++17 & Modern C++). Buổi CN đã chạy xong phần chính → sang **Tuần 2** (Linux sysprog + debug).
- **Buổi gần nhất:** `comprehensive track cpp-system` (CN, trần T2) — **3.0/4**, 15/16 câu ([log](../mock-interview/sessions/2026-08-13--comprehensive--cpp-system.md)). Sổ yếu **2 → 5** (+CPP-016 slicing, +OS-020 signal-safety, +SD-016 bộ nhớ tất định; CPP-032 lên 1/2). Retention CPP-020 (**4**) + CPP-029 (**3**) đạt → dời Tuần 4. Điểm sáng: CPP-014, OS-001, CPP-020 đều 4. *(B1: 3.25 · B2: 3.3 · B3: 2.83 · B4: 3.17 · B5: 3.67 · CN-weak: 3.0 · CN-comp: 3.0)*
- **▶️ LÀM TIẾP:** **Tuần 2 Buổi 1** — 🔁 khởi động 5–10′ (câu đến hạn Tuần 3 chưa tới, dùng sổ yếu: OS-020) → đọc [file-io](../../04-linux-system-programming/file-io.md) + [processes-signals](../../04-linux-system-programming/processes-signals.md) → `/mock rapid track linux-sysprog`. **OS-020 (signal-safety) sẽ được chèn lại ở đây** vì đúng domain.
- **Nợ lại từ Tuần 1:** ① **bài coding** `SocketHandle` (CPP-054) + `ring_buffer_v2` bản mutex (COD-006) — chạy `/mock coding track cpp-system`, làm từ file trống. ② Viết lại đáp án bank **DP-011** theo tiêu chuẩn mới (có code + cơ chế).
- **Sổ yếu (5 câu):** CPP-016 · OS-020 · SD-016 · CPP-032 (1/2 lần ≥3) · CPP-054 (chưa hỏi lại). Xem [weak-register](../mock-interview/weak-register.md).

**Checklist buổi (tick `[x]` khi xong — đây là "đang dang dở đến đâu"):**
- **Tuần 1** (C++17): `[x]` B1 · `[x]` B2 · `[x]` B3 · `[x]` B4 · `[x]` B5 · `[x]` CN *(weak-review 3.0 + comprehensive 3.0; nợ 2 bài coding)*
- **Tuần 2** (Linux sysprog+debug): `[ ]` B1 ⬅️ **ĐANG Ở ĐÂY** · `[ ]` B2 · `[ ]` B3 · `[ ]` B4 · `[ ]` B5 · `[ ]` CN
- **Tuần 3** (kernel/drivers): `[ ]` toàn bộ
- **Tuần 4** (Yocto/design/behavioral): `[ ]` toàn bộ

> **Quy tắc cập nhật (cho Claude):** sau mỗi phiên mock thuộc plan này → (1) tick buổi tương ứng ở trên, (2) sửa 4 dòng "Vị trí / Buổi gần nhất / LÀM TIẾP / Sổ yếu", (3) ghi log `sessions/` như thường. Không cần sửa file nào khác — mọi nơi khác chỉ trỏ về block này.

---

## Hai nguyên tắc xuyên suốt (đọc trước)

**① Ưu tiên TƯ DUY, không phải cú pháp.** Đặc biệt với **build system (Yocto/CMake/CI)**: phỏng vấn hỏi *"layer/bbappend để làm gì", "sstate giải quyết gì", "DEPENDS vs RDEPENDS", "vì sao CMake target-based"* — tức **mental model** (vấn đề → kiến trúc → đánh đổi), **không** bắt viết recipe/CMakeLists chính xác. Đọc để *hiểu bức tranh*, cú pháp chỉ tra khi ngồi máy. Nguyên tắc này áp cho mọi mảng: nói được *bản chất & khi nào dùng* quan trọng hơn nhớ flag.

**② Xen kẽ + lặp lại ngắt quãng (chống quên tuần cũ).** Theme tuần chỉ định *trọng tâm học MỚI* — không có nghĩa bỏ tuần trước. Hai cơ chế kéo kiến thức cũ quay lại:
- **🔁 Khởi động trộn (~5–10') mỗi buổi:** trước khi vào chủ đề mới, chạy nhanh `/mock weak-review` (câu yếu) hoặc `/mock retention track datalogic` (câu tuần trước, đã trả lời tốt — spaced review). Trộn domain là *cố ý*: phỏng vấn thật cũng hỏi ngẫu nhiên.
  - **Câu retention lấy ở đâu:** bảng **[🔁 Lịch kiểm tra lại](../mock-interview/weak-register.md)** — mỗi câu đã gỡ khỏi sổ yếu đều có **hạn kiểm tra = tuần gỡ + 2** và **góc mới đề xuất** (cấm lặp góc cũ). Mở bảng, lấy câu đến hạn của tuần hiện tại. Tụt < 3 → kéo về sổ yếu.
  - Lịch hiện tại: ~~Tuần 2 → CPP-020, CPP-029~~ ✅ **đã kiểm tra 13/08 (4 và 3), dời sang Tuần 4** · **Tuần 3** → CPP-009, DP-002, OS-003, OS-007 · **Tuần 4** → CPP-019, CPP-024, CPP-045, CPP-020, CPP-029.
- **🔁 Cuối tuần ôn DỒN TÍCH (tăng dần):** CN mỗi tuần quét *tất cả* các tuần đã học, không chỉ tuần vừa rồi — xem cột "CN" trong từng bảng.

**③ Đọc/Hiểu liệt kê ĐẦY ĐỦ để ôn TRƯỚC; mock cứ để NGẪU NHIÊN.** Cột "Đọc/Hiểu" mỗi buổi nêu **cụ thể** doc (mục) + **pool câu hỏi bank** liên quan — đây là phần *chuẩn bị*, đọc kỹ trước khi vào mock. Mock thì **được phép hỏi ngẫu nhiên/lệch chủ đề trong ngày** (rút từ track rộng + weak-register + retention) — đó là **cố ý**, để luyện *phản ứng* như phỏng vấn thật (câu đến bất ngờ, trộn mảng). Không ép test chỉ trong phạm vi vừa đọc. Việc bị hỏi trúng chỗ chưa ôn là *tín hiệu* để ghi vào weak-register và ôn bù, không phải lỗi của plan.

> Nhờ đó cuối 4 tuần, kiến thức Tuần 1 đã được gặp lại ≥3 lần (khởi động + các buổi CN + tổng duyệt) thay vì học một lần rồi quên.

---

## JD → tài liệu (bản đồ)

| Trụ JD | Đọc | Test (bank/mock) |
|---|---|---|
| **C++17** | [02-modern-cpp](../../02-modern-cpp/), [EMC summary](../../15-book-summaries/effective-modern-cpp.md) | `CPP-*` (nhất là 026, 037, 040–051 emc) |
| **Linux userspace + debug** 🎯 | [04-linux-system-programming](../../04-linux-system-programming/), [09-debugging](../../09-debugging/) | `LNX-*`, `DBG-*` |
| **Kernel space** | [05-drivers-device-tree](../../05-drivers-device-tree/), [lkd](../../15-book-summaries/lkd/) | `DRV-*`, `BSP-B` |
| **Drivers I2C/SPI/PCI/USB** 🎯 | [driver-basics](../../05-drivers-device-tree/driver-basics.md), [pci-usb-drivers](../../05-drivers-device-tree/pci-usb-drivers.md) | `BUS-*`, `DRV-019…027` |
| **Yocto + CMake/CI** *(học TƯ DUY, không cú pháp — xem nguyên tắc ①)* | [yocto](../../06-build-systems/yocto.md), [cmake](../../06-build-systems/cmake.md), [cross-compilation](../../06-build-systems/cross-compilation.md) | `BLD-*`, `BSP-017…019` |
| **Design subsystem + lead + code review** | [10/system-design](../../10-thinking/system-design.md), [07-shared-libraries](../../07-shared-libraries/) | `SD-*`, `BEH-*` |

**Track mock riêng cho JD:** `/mock ... track datalogic` (trộn đúng các domain trên).

---

## Tuần 1 — C++17 & Modern C++ (+ CMake)

*Trọng tâm JD "C++17". Vá nền ngôn ngữ trước.* 🔁 Tuần đầu chưa có gì để trộn — cứ học sâu.

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [x] [raii-smart-pointers](../../02-modern-cpp/raii-smart-pointers.md) §6 (Rule 0/3/5) + smart pointer (unique/shared/weak, cyclic ref); [move-semantics](../../02-modern-cpp/move-semantics.md) (rvalue ref, `std::move`, perfect forwarding, emplace vs push_back). **Chạm nhanh:** [oop](../../01-cpp-fundamentals/oop.md) (struct/class, virtual/vtable), [memory-model](../../01-cpp-fundamentals/memory-model.md) (stack/heap). **Pool bank ôn trước:** CPP-001…008, 020, 024…029 | ✅ `rapid cpp-system` 3.25/4 (2026-08-03) |
| 2 | - [x] 🎯 EMC items nền: [effective-modern-cpp](../../15-book-summaries/effective-modern-cpp.md) — auto (Item 1–2), braced-init (7), nullptr (8), using vs typedef (9), `=delete` (11), override/final (12), make_unique (21), special-member generation (17). **Pool bank:** CPP-040…051, CPP-046 | ✅ `by-level 🟡 modern-cpp` 3.3/4 (2026-08-03) |
| 3 | - [x] [concurrency](../../02-modern-cpp/concurrency.md) (thread, atomic, memory_order, mutex, lock_guard/unique_lock) + [OS sync-primitives](../../03-operating-system/sync-primitives.md) (mutex vs semaphore + **ownership/priority inheritance**, spinlock, deadlock 4 Coffman). **Pool bank:** CPP-018/019/022/024; OS-003/004/006/007/012 | ✅ `daily cpp-system` 2.83/4 (2026-08-05) — ⚠️ nợ **memory order** (CPP-019) |
| 4 | - [x] [cmake](../../06-build-systems/cmake.md) — **tư duy target-based** (PUBLIC/PRIVATE/INTERFACE *nghĩa là gì*, toolchain file *giải quyết gì*), cú pháp tra sau | ✅ `daily build-systems` 3.17/4 (2026-08-06) — CMake đạt; nợ **lệnh Yocto** (cleansstate/devtool) |
| 5 | - [x] Coding: viết vào [coding-arena/](../mock-interview/coding-arena/); nền: [12-dsa/ring-buffer.md](../../12-dsa/ring-buffer.md) | ✅ `coding dsa` 3.67/4 (2026-08-09) — COD-001 **4** · COD-006 **3** · COD-003 **4**; nợ **hỏi spec đa luồng** + **cấp phát trong hot path**. [Bản đã review](../mock-interview/coding-arena/reviewed/) |
| CN | 🔁 Ôn dồn tích **T1** | ✅ `weak-review cpp-system` 3.0/4 (sổ yếu 4→2) + ✅ `comprehensive cpp-system` 3.0/4, 15/16 câu, trần T2 ([log](../mock-interview/sessions/2026-08-13--comprehensive--cpp-system.md)) — **nợ 2 bài coding** |

## Tuần 2 — Linux system programming (userspace + debug) 🎯

*Đúng chỗ bạn muốn tập trung ("Familiar with Linux userspace + debugging").* 🔁 **Khởi động mỗi buổi:** `/mock retention track cpp-system` (kéo C++ Tuần 1 quay lại).

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] [file-io](../../04-linux-system-programming/file-io.md) (fd, syscall, blocking/non-blocking, `O_NONBLOCK`), [processes-signals](../../04-linux-system-programming/processes-signals.md) (fork/exec/wait, zombie, signal handler async-safe). **Pool bank:** LNX-001…012 | `/mock rapid track linux-sysprog` |
| 2 | - [ ] 🎯 [io-multiplexing](../../04-linux-system-programming/io-multiplexing.md) (select/poll/epoll, **LT vs ET**, event loop) + [ipc-linux](../../04-linux-system-programming/ipc-linux.md) (pipe, shm, mq, socket, so sánh). **Pool bank:** LNX-013…026 | `/mock by-level 🟠 track linux-sysprog` |
| 3 | - [ ] 🎯 [09-debugging](../../09-debugging/): [gdb](../../09-debugging/gdb.md), [tools](../../09-debugging/tools.md) (strace/ltrace/perf), [memory-bugs](../../09-debugging/memory-bugs.md) (ASan/TSan/Valgrind), core dump. **Pool bank:** DBG-001…011, 017/018 | `/mock daily track debugging` |
| 4 | - [ ] [gdb](../../09-debugging/gdb.md) sâu (breakpoint/watchpoint/`bt`/frame, remote **gdbserver**, core dump post-mortem) — điểm yếu cần vá. **Pool bank:** DBG-009/010, 012…016 | `/mock deep-dive track debugging` |
| 5 | Coding: bài low-level C | `/mock coding track cpp-system` (memcpy/strlen, endianness) |
| CN | 🔁 Ôn dồn tích **T1+T2** | `/mock weak-review` + `/mock comprehensive track datalogic` (trộn C++ + Linux + debug) |

## Tuần 3 — Kernel & drivers (I2C/SPI + PCI/USB) 🎯

*Trụ kernel + drivers của JD. Vùng bạn mạnh (kernel/DT) + lấp PCI/USB.* 🔁 **Khởi động mỗi buổi:** `/mock retention track linux-sysprog` + `weak-review` (kéo Linux/debug Tuần 2 quay lại).

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] [driver-basics](../../05-drivers-device-tree/driver-basics.md), [kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md) · *bổ đệm* [bare-metal-c](../../08-embedded-systems/bare-metal-c.md) phần `volatile`/register/bit-manip ([EMB-A](../mock-interview/bank/embedded-fundamentals.md)) — nền cho MMIO `readl/writel` | `/mock daily track drivers-dt` |
| 2 | - [ ] 🎯 [device-tree](../../05-drivers-device-tree/device-tree.md) (cú pháp DTS, node/property, compatible→match, binding) + [kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md) ôn lại. **Pool bank:** BSP-006…009 (probe, EPROBE_DEFER, pinctrl), DRV-001…010 | `/mock by-level 🟠 track bsp` |
| 3 | - [ ] 🎯 **[pci-usb-drivers](../../05-drivers-device-tree/pci-usb-drivers.md)** (PCI config/BAR/MSI, USB descriptor/URB/gadget) | `/mock ... track drivers-dt` (DRV-019…027) |
| 4 | - [ ] [interrupt/DMA](../mock-interview/bank/bsp.md) (BSP-010…013: threaded IRQ, coherent vs streaming DMA) · *bổ đệm* [interrupts-bare-metal](../../08-embedded-systems/interrupts-bare-metal.md) khái niệm ISR ([EMB-C](../mock-interview/bank/embedded-fundamentals.md)): không sleep trong ISR, critical section, reentrancy — chuyển thẳng sang top-half/bottom-half kernel. **Pool bank:** BSP-010…013, DRV-011…018 | `/mock deep-dive track bsp` |
| 5 | - [ ] [lkd](../../15-book-summaries/lkd/) cụm driver/interrupt (đọc thêm) | `/mock comprehensive track drivers-dt` |
| CN | 🔁 Ôn dồn tích **T1+T2+T3** | `/mock weak-review` + `/mock comprehensive track datalogic` (giờ trộn cả kernel/driver) |

> **Phạm vi `embedded-fundamentals` (EMB) cho JD này:** chỉ ôn **A** (C/thanh ghi/`volatile`/bit-manip) + **C** (khái niệm ISR) như *đệm* cho buổi 1 & 4 ở trên — vì chúng chuyển thẳng sang MMIO driver + kernel IRQ. **Hoãn** B (startup/linker chi tiết), **D (RTOS)**, F/G/H (CRC/MISRA, low-power, JTAG/SWD): JD là Embedded **Linux** (có MMU, không RTOS/bare-metal firmware) nên các phần này là "nice-to-have" ghi điểm nếu bị hỏi lan, không phải câu loại. Full EMB (domain `EMB` trong bank) để dành cho hồ sơ BSP tổng quát nếu sau này nhắm JD khác.

## Tuần 4 — Yocto/CI + Design + Behavioral + tổng duyệt

*Yocto (JD) + kỹ năng "lead/design/review" + dress rehearsal.* 🔁 **Khởi động mỗi buổi:** `/mock retention track datalogic` (trộn cả 3 tuần trước) — tuần này nghiêng hẳn về **ôn lại + tổng duyệt**.

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] 🎯 **[yocto](../../06-build-systems/yocto.md)** — **tư duy** (layer/bbappend *để làm gì*, sstate *giải quyết gì*, DEPENDS vs RDEPENDS, devtool/SDK), **không** học thuộc cú pháp recipe + [BSP-017…019](../mock-interview/bank/bsp.md) | `/mock ... track build-systems` (BLD-004…010) |
| 2 | - [ ] 🎯 [system-design](../../10-thinking/system-design.md) §5 hộp đồ nghề + §6 ví dụ giải mẫu (**OTA**, **máy quét mã vạch** — đúng chất Datalogic) + [07-shared-libraries](../../07-shared-libraries/) (ABI, API design) | `/mock deep-dive track system-design` (SD-012…016) |
| 3 | - [ ] 🎯 **Behavioral cho JD**: [BEH](../mock-interview/bank/behavioral.md) + phần "lead junior / code review" (§ dưới) | `/mock ... track behavioral` |
| 4 | Tổng duyệt vòng thật | `/mock comprehensive track datalogic` (16 câu) |
| 5 | Kiểm tra toàn diện | `/mock full-review` (quét mọi câu đã hỏi) |
| CN | Nghỉ + đọc lại [sessions/](../mock-interview/sessions/) | — |

---

## Behavioral riêng cho JD (JD nhấn "lead junior + code review")

Chuẩn bị STAR ([BEH](../mock-interview/bank/behavioral.md)) cho các câu **rất khả năng gặp** với JD này:
- **"Kể lần bạn mentor/hướng dẫn một junior"** → dùng chất liệu Samsung (hướng dẫn quy trình, AI skill, chip porting tool).
- **"Cách bạn review code / xử lý bất đồng kỹ thuật khi review"** → nêu tiêu chí review (đúng đắn, an toàn, đọc được), góp ý xây dựng, dựa dữ liệu.
- **"Thiết kế một subsystem phức tạp bạn từng làm"** → chọn case cross-layer (S-Box/brightness) hoặc multi-chipset HAL, trình bày theo khung [SD](../mock-interview/bank/system-design.md).
- **"Vì sao rời công ty cũ"** ([BEH-008](../mock-interview/bank/behavioral.md)) + **câu hỏi ngược** ([BEH-009](../mock-interview/bank/behavioral.md)) — hỏi về SoC/nền tảng, tỉ lệ kernel vs userspace, quy trình Yocto/CI, đội ngũ.
- Chuẩn bị nói **tiếng Anh** cho 2–3 câu ruột (JD yêu cầu English).

## Lịch nước rút 2 tuần

Gộp đôi mỗi ngày theo thứ tự ưu tiên: **Tuần A** = (Tuần 1 buổi 1–2–4) + (Tuần 2 buổi 2–3–4); **Tuần B** = (Tuần 3 buổi 2–3–4) + (Tuần 4 buổi 1–3–4). Bỏ các buổi coding/đọc-thêm nếu thiếu giờ, giữ **mọi buổi 🎯 + `comprehensive` + `full-review`**.

## Vá điểm yếu đã biết
Từ [weak-register](../mock-interview/weak-register.md): OS-003 (deadlock), CPP-029 (emplace), DP-002 (Singleton) — chèn vào các buổi `weak-review`. GDB (bạn tự nhận yếu) → ưu tiên Tuần 2 buổi 4.
