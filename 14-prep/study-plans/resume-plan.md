# 🎯 Resume plan — C++ System Software · 4 buổi × 90′

> **Plan bám RESUME, không bám JD.** Chưa có JD cụ thể ⇒ ôn đúng phần **chắc chắn 100% bị hỏi**: từng dòng trong [`RESUME_current.tex`](../../RESUME_current.tex), cùng phần nền kỹ thuật **ngay dưới** dòng đó. Trọng số nghiêng **C++ System Software**: C++ interface · shared library · ABI · concurrency · design pattern. Phần kernel giữ ở mức *kể được và trả lời được câu đuổi đầu tiên*.
>
> **Nguồn sự thật cho phần kiến trúc:** [A1 — `libdisplay`](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) (hệ thật đã khử nhạy cảm) và [A2 — C++ interface/HAL](../../11-design-patterns/in-practice/A2-cpp-interface-hal.md). Tư liệu nội bộ còn tên thật (`shared_lib.md`) bị `.gitignore`: **không trích tên nào từ đó vào plan, bank, log**.
>
> Viết 2026-10-05. Khuôn: [study-plans/README](README.md) · plan trước đã lưu trữ: [datalogic-plan](archive/datalogic-plan.md).
>
> **Chiến lược kể** (luật ⑤ ở [§1](#1-năm-luật-của-plan)): **người không làm TV vẫn hiểu** — library = **Picture Quality + Panel Control**, kernel = **panel driver** · **kể ngắn trước, mở dần** · mỗi câu **thả một móc** vào kiến thức nền · chỉ ôn **ở mức hiểu cơ bản** những gì đã làm. Ngoài R1–R4 bám resume còn **R1′** (kể kiến trúc theo chiến lược này), **R5** (CI · unit test · code quality) và **R6** (troubleshooting), mỗi buổi kèm tài liệu + lab.

---

## 📍 Tiến độ hiện tại — **RESUME Ở ĐÂY** (nguồn tracking DUY NHẤT)

> ⚠️ **TRẠNG THÁI, không phải nhật ký.** Sau mỗi buổi: **SỬA** ô `Xong?` + dòng *Buổi gần nhất* + *⏭️ Trước buổi kế*. **Không thêm dòng mới.**
> Diễn biến từng phiên → [`sessions/`](../mock-interview/sessions/) (tên file `YYYY-MM-DD--R<n>--<slug>.md`) · câu yếu → [weak-register](../mock-interview/weak-register.md) · lỗ hổng tài liệu → [gap-register](gap-register.md).

### Trạng thái

| | |
|---|---|
| **Mục tiêu** | Trả lời được **mọi dòng** của resume tới tầng T2, nghiêng C++ System SW. Không có hạn chót cứng |
| **Ngân sách** | **R1–R4 × ~105′** (đọc ~30′ + mock chính ~60′, 12 câu + ⚡ rapid pool ~15′) · **R1′, R5, R6 × ~90′** (đọc/lab ~30′ + mock ~60′) |
| **Ngôn ngữ** | Tiếng Việt, **có vòng tiếng Anh** (R1 câu 12 · R4 câu 9 và 12) |
| **Buổi gần nhất** | 🟡 **R1′ phần 1 — 2026-10-08**, 7/12 câu, 17/28 = **2.43** ([log](../mock-interview/sessions/2026-10-08--R1prime--ke-lai-kien-truc.md)) — người học dừng sau câu 7 · trước đó R1-rapid 2.50, R1 mock chính 2.33 |
| ▶️ **Phiên kế** | **R1′ phần 2** — câu 8–12 ở [§11](#11-r1--kể-kiến-trúc-theo-chiến-lược-hiện-tại) (`DP-041` · `DRV-006` · `DP-043` · `DP-038` · `RES-034`) + `LNX-045` (weak, Ⓑ) + `LNX-046` (mới), ~45′; rồi R2 |
| **Chẩn đoán mang vào** | **R1 (06/10):** hiểu hệ thật **sâu hơn tài liệu** (sửa được bank `DP-042`, đọc đúng race vsync trong code thật), nhưng **chưa bảo vệ được quyết định thiết kế**: biết *cái gì*, chưa nói được *vì sao chọn phương án này thay vì phương án kia*. Vẫn mẫu **T1 ổn, T2 hụt**. Phiên [rapid 05/10](../mock-interview/sessions/2026-10-05--rapid--cpp-system.md) 2.36: hệ quả cơ học thì đoán (R1 lặp lại ở `DP-043`) |
| 🔴 **Rủi ro lớn nhất** | **Bản kể kiến trúc chưa thuộc:** [`RES-035`](../mock-interview/bank/resume.md) bản 30″ kể ba **khối** thay vì ba **ranh giới**, thiếu câu trao quyền; [`RES-034`](../mock-interview/bank/resume.md) sang tiếng Anh ba ranh giới co còn một · **Nền dưới câu khoá liên process còn mỏng** (`DP-040`, `LNX-045`: chưa nối *"semaphore không có chủ"* sang hệ quả). [`RES-001`](../mock-interview/bank/resume.md) đã lần đầu đạt 3 (08/10) — còn thiếu vấn đề-bằng-hình-ảnh. Chữa bằng nói to + bấm giờ + chiến thuật ở [§10](#10--chiến-thuật-trả-lời--rút-ra-từ-r1-0610) |

### ▶️ LÀM TIẾP — chạy đúng thứ tự

| # | Buổi | Trục | ~ | Mock chính | ⚡ Rapid pool |
|---|---|---|---|---|---|
| **R1** | 🗣️ **Kể kiến trúc** | Câu mở màn · ba ranh giới · PQ/Panel Control · Bridge · `panel_ops` — [§3](#3-r1--kể-kiến-trúc-a1) | 105′ | ✅ **2.33** ([06/10](../mock-interview/sessions/2026-10-06--R1--kien-truc.md)) | ✅ **2.50** ([06/10](../mock-interview/sessions/2026-10-06--R1-rapid--pool.md)) |
| **R1′** | 🔁 **Kể kiến trúc — làm lại** | Bản 30″/90″ · móc nền tảng ở mức cơ bản · người không làm TV vẫn hiểu — [§11](#11-r1--kể-kiến-trúc-theo-chiến-lược-hiện-tại) | 90′ | 🟡 phần 1 **2.43** ([08/10](../mock-interview/sessions/2026-10-08--R1prime--ke-lai-kien-truc.md)) · phần 2 ⬜ | — |
| **R2** | 🔩 **Ranh giới C++ interface / `.so`** | ABI · vtable · Singleton & race · Null Object · ownership — [§4](#4-r2--ranh-giới-c-interface--so-a2) | 105′ | ⬜ | ⬜ |
| **R3** | 🧵 **S-Box & nhiều process** | Thread 60 Hz + condvar · sensor · POSIX mq · shm + semaphore — [§5](#5-r3--s-box-thread-60-hz-sensor-mq-nhiều-process) | 105′ | ⬜ | ⬜ |
| **R4** | 🧭 **Phần còn lại + tiếng Anh** | Preset · porting/AI · load-time · migration · debug · 🇬🇧 — [§6](#6-r4--phần-còn-lại-của-resume--tiếng-anh) | 105′ | ⬜ | ⬜ |
| **R5** | ⚙️ **CI · unit test · code quality** | Pipeline nhìn từ người dùng · GoogleTest/KUnit · coverage · các cổng chất lượng · 🧪 lab pipeline 5 cổng — [§12](#12-r5--ci--unit-test--code-quality) | 90′ | ⬜ | — |
| **R6** | 🔎 **Troubleshooting** | Ba tình huống crash · `addr2line` · coredump bằng gdb · oops · log — [§13](#13-r6--troubleshooting) | 90′ | ⬜ | — |

**Nhãn cột *Nguồn*:** `mới` = chưa từng hỏi · `đã hỏi` = từng hỏi ở phiên trước và đạt ⟹ **hỏi lại bình thường như câu mới** (luật ④ ở [§1](#1-năm-luật-của-plan)) · `🔴 weak` / `🔁 retention` = theo lệnh *"Lần sau hỏi"* ở weak-register.

**Gọi phiên:** gõ `/mock`. Interviewer đọc block này và đề xuất phiên kế tiếp theo thứ tự **R1′ → R2 → R2-rapid → R3 → R3-rapid → R4 → R4-rapid → R5 → R6**; danh sách câu nằm ở mục tương ứng bên dưới. R5 và R6 không phụ thuộc R2–R4, đổi thứ tự được nếu cần.

**⚡ Rapid pool — vì sao có, chạy thế nào:** mục tiêu của plan là **phủ đủ** (luật ④), mà mỗi buổi có 7–10 câu liên quan **không** chen được vào 12 câu chính. Đọc mà không bị hỏi thì không biết có nói ra được không ⟹ quét chúng bằng một phiên `rapid` **tách riêng**, chạy ngay sau mock chính (config cấm trộn rapid vào phiên sâu). Luật của rapid: hỏi thẳng đề bank, ~1 phút/câu, **tối đa 1 probe**, chấm theo [thang rapid](../mock-interview/config.md) — chỉ đo **lõi T1** (ý *Chốt* của đáp án, hoặc khung 60″ với câu `RES` 🏗️), không đào. Câu ≤ 2 vào weak-register phải ghi rõ lỗ hổng **DIỄN ĐẠT** hay **KIẾN THỨC**. Log: `YYYY-MM-DD--R<n>-rapid--pool.md`. Câu weak và retention hỏi theo dạng **Ⓖ/Ⓑ** ghi ở cột *"Lần sau hỏi"* của [weak-register](../mock-interview/weak-register.md). Plan **không** chép lại các lệnh đó.

### ⏭️ Trước R1′ phần 2 — 4 việc, ~30′

| # | Việc | ~ | Vì sao |
|---|---|---|---|
| 1 | 🎙️ Nói to **bản 30″** ([A1 §10.1](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)) tới khi thuộc nguyên văn. Kiểm: **ba ranh giới** (C++ interface · API C · `ioctl`), không phải ba khối; câu cuối là câu trao quyền. Bản 90″ phải có câu *"chỗ duy nhất lấy khoá"* | 5′ | `RES-035` phần 1: bản 30″ thiếu hai ranh giới và câu trao quyền |
| 2 | Đọc [LNX-046](../mock-interview/bank/linux-sysprog.md), tập **bản nói 45″**. Một gốc cho bốn câu: *semaphore không có chủ* ⟹ mất priority inheritance (`OS-007`) · tự deadlock (`DP-040`) · kernel không nhả hộ (`LNX-045`) · timeout-reset cho hai bên cùng vào | 10′ | Góp ý của người học ở phiên 08/10; `LNX-045` 2 điểm |
| 3 | Đọc [A1 §7.1](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) (khởi tạo trong `__attribute__((constructor))`) và bank [DP-038](../mock-interview/bank/design-patterns.md) (phép thử *"bỏ phần cắm vào"*) | 10′ | `DP-041`, `DP-038` đều 🔴 weak, hỏi ở phần 2 |
| 4 | Xem output slot lệch ở bank [DP-043](../mock-interview/bank/design-patterns.md) — nối với ý *"chỉ số ô vtable cố định lúc compile"* ([CPP-006](../mock-interview/bank/cpp.md)) | 5′ | `CPP-006` phần 1 thiếu đúng ý này |

---

## 1. Năm luật của plan

| # | Luật | Thi hành |
|---|---|---|
| ① | **Bán kính một bước.** Mỗi câu phải trả lời được *"dòng resume nào kéo câu này ra?"*. Tối đa một bước xuống dưới: dòng resume → cơ chế nằm ngay dưới nó. Không đi tiếp xuống T3 | Câu nào không chỉ ra được dòng resume ⇒ không vào plan. Cột *"Dòng resume"* ở [§2](#2-bản-đồ-phủ-resume_currenttex--mọi-dòng--câu--buổi) là bằng chứng |
| ② | **Khử nhạy cảm.** Chỉ dùng tên trong A1/A2 (`libdisplay`, `lib_api_*`, `IDimmingAlgo`, `panel_ops`…) | Áp cho plan, bank, log phiên, và **cho chính câu trả lời của bạn** khi luyện |
| ③ | **Nói to, bấm giờ.** Câu resume là câu **nói**, viết ra giấy không tính | Mỗi câu 🏗️: ≤ 90 giây cho câu trả lời đầu, rồi mới tới follow-up |
| ④ | **Phủ trước, mới lạ sau.** Mục tiêu của plan là **phủ đủ nội dung resume**, không phải tránh lặp. Câu đã hỏi và đạt được **hỏi lại bình thường**: đúng đề bank, phần nền hỏi đủ, follow-up ≥ 1, chấm như câu mới ([config §6 luật ①](../mock-interview/config.md)). Không tung đồng xu, không cần đổi góc, follow-up cũ dùng lại được | Không trái config: luật Ⓖ/Ⓑ ở config §6 → 🔁 chỉ áp cho câu **weak** và **retention**. Câu weak/retention trong plan vẫn theo weak-register. Câu đã hỏi mà lần này ≤ 2 điểm ⟹ vào weak-register như mọi câu khác (**regression**) |
| ⑤ | **Người ngoài ngành vẫn hiểu · kể ngắn trước · móc nền tảng · mức cơ bản.** Không FRC/TCON; library = Picture Quality + Panel Control; kernel = panel driver. Dimming chỉ kể *"thuật toán + backend điều khiển phần cứng"* — chi tiết backend nhân theo chip × thuật toán là **câu dự phòng** ([A1 §10.4](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)) | Interviewer **không đào** chi tiết backend. Ở R1′, R5, R6: trần **T1 + một chút T2**; tên công cụ, flag, plugin là T3 — **không chấm**. Câu hỏi vào kiến thức nền chỉ đi theo **móc đã thả** ở bản kể ([A1 §10.3](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)) |

---

## 2. Bản đồ phủ `RESUME_current.tex` — mọi dòng → câu → buổi

> Không được bỏ sót dòng nào. Dòng nào **cố ý** để mỏng thì ghi lý do ở cột cuối.

| Dòng resume | Câu trong plan | Buổi | Ghi chú |
|---|---|---|---|
| **Summary** — 3 năm C/C++ system SW, TV & enterprise display, ARM SoC | `RES-001` · `RES-034` 🇬🇧 | R1 | Câu mở màn. 🔴 weak |
| **Skills** — C, Modern C++ (11/14/17) | rải khắp R2, R3 | R2 · R3 | C thuần hỏi riêng bằng `/mock rapid track c` ([§7](#7-cố-ý-không-đưa-vào-plan--và-vì-sao)) |
| **Skills** — IPC (POSIX mq, shm), multi-threading, shared libraries | `LNX-044` · `LNX-045` · `CPP-069…071` · `OS-012` · R2 toàn bộ | R2 · R3 | Trụ của plan |
| **Skills** — HAL, device drivers, device tree | `DP-043` · `RES-023` | R1 | Kernel giữ ở mức *"cùng ý tưởng vtable, viết tay bằng C"* |
| **Skills** — debugging cross-layer, GDB, dmesg | `RES-008` | R4 | 🔴 weak — phản xạ *cắt đôi* |
| **Skills** — AI-assisted development | `RES-016` | R4 | 🔴 weak. Dòng đã chuyển xuống Skills và bỏ con số |
| **Skills** — CMake, Makefile, Git, Perforce | `RES-009` | R4 | Đi kèm porting tool |
| **Core 1** — PQ (dimming, frame-rate, timing), C++ interface → kernel HAL | `RES-002` · `DP-040` · `DP-024` | R1 | |
| **Core 2** — một C++ interface cho nhiều chipset, chọn impl lúc boot, kernel dispatch qua bảng hàm | `RES-002` · `RES-023` · `DP-023` · `DP-042` · `DP-022` · R2 toàn bộ | R1 · R2 | ⭐ dòng mạnh nhất |
| **Core 3** — port driver, DT, migration 5.10 → 6.12 | `RES-004` | R4 | Một câu, mức kể được |
| **Core 4** — debug xuyên tầng | `RES-008` | R4 | |
| **Core 5** — công tác R&D HQ Hàn Quốc 2 lần/năm | `RES-031` | R4 | |
| **S-Box 1** — adaptive brightness theo cảm biến, không nháy | `RES-006` · `CPP-069` · `CPP-070` · `CPP-071` · `DP-006` | R3 | Theo mô tả của bạn: thread 60 Hz + condvar, polling chống nhiễu |
| **S-Box 2** — đồng bộ nhiều unit qua POSIX mq, một binary hai mode | `RES-005` · `LNX-044` · `RES-027` | R3 · R4 | |
| **Load-time** — 3–4 s → < 0,5 s, song song vs priority, 100 lần boot | `RES-015` | R4 | Một câu thiết kế. `RES-013/014/029` ở pool đọc |
| **Preset** — Windows, MVVM, save/restore/export | `RES-017` · `RES-021` · `DP-032` | R4 | Memento + concurrency phía C++ |
| **Porting tool** — Python, 1–2 ngày → 2–3 giờ | `RES-009` | R4 | 4 ý ở [archive §3](archive/datalogic-plan.md) (mục *Porting tool*) |
| Best Employee Q1/2026 · TOEIC · chứng chỉ DSA | — | — | 🟡 Chuẩn bị **một câu** cho mỗi mục, không vào bank |

---

## 3. R1 — Kể kiến trúc (A1)

**Câu hỏi của buổi:** *"Kể kiến trúc library bạn làm"*. Mở màn của mọi vòng technical khi resume có dòng shared library.

### 📖 Đọc (~30′)

- [A1](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) **§2** (sơ đồ) · **§3** (narrow waist, 3.3 hai lý do) · **§4.1, §4.4** (PQ/Panel Control, cố ý không pattern) · **§5.0, §5.4, §5.6, §5.7** · **§6.3** (bảng hai tầng) · **§7.2** · **§9** (5 điểm yếu) · **§10** (bản nói 75″ — **đọc to**)
- Bank: khung `RES-001` Bản C · `RES-034` bản mẫu tiếng Anh

### 🎤 Mock — 12 câu, ~60′

| # | ID | Câu (tóm tắt) | Nguồn | Ghi chú cho interviewer |
|---|---|---|---|---|
| 1 | `RES-001` | Giới thiệu + project tâm đắc | 🔴 weak | Theo weak-register. Chấm cả **thời lượng** và **móc thả ra** |
| 2 | `RES-002` | Một interface nhiều chipset — cơ chế; board config đọc sai thì sao | đã hỏi | Phải ra cả hai tầng: user (chọn impl) và kernel (bảng hàm) |
| 3 | `DP-040` | C API giữa hai vùng C++ — bảo vệ bằng hai lý do độc lập | mới | Lý do 2 (điểm khoá duy nhất) mới phân biệt ứng viên |
| 4 | `DP-024` | Lệnh đơn để nguyên hàm — vì sao không bọc cho đồng bộ | mới | Ý chốt: biến thể đã bị **driver** hấp thụ |
| 5 | `DP-023` | N thuật toán × M chip — không thành N×M lớp | mới | |
| 6 | `DP-042` | Backend nhân theo chip × thuật toán — còn là Bridge không | mới | Phần đắt viết một lần |
| 7 | `DP-022` | Hàm `makeX(ChipType)` rời vs một factory object | mới | Lỗi *cặp không khớp vẫn compile* |
| 8 | `DP-036` | Null Object — đổi lấy gì, khi nào không dùng | mới | Nối `LIB_OK` (A1) vs `-ENOTSUP` (A2) |
| 9 | `RES-023` | Vì sao cần **hai** tầng bảng hàm; ai điền, lúc nào | đã hỏi | |
| 10 | `DP-043` | `panel_ops` vs `virtual` — giống, khác, thừa hưởng rủi ro gì | mới | Slot `NULL` · chèn giữa bảng |
| 11 | `DP-046` | 5 điểm yếu — làm lại thì sửa cái nào trước | mới | Xếp theo **rủi ro**, không phải *"làm lại hết"* |
| 12 | `RES-034` 🇬🇧 | *"Walk me through the architecture…"* | mới | ≤ 90″, ba ranh giới phải còn đủ ba |

### ⚡ R1-rapid — 6 câu, ~10′

| # | ID | Câu (tóm tắt) | Nối với |
|---|---|---|---|
| 1 | `RES-007` | Picture quality và panel control — giải thích cho người ngoài ngành | Core 1 — Picture Quality + Panel Control |
| 2 | `DP-038` | Bridge giải gì, khác Strategy chỗ nào khi code giống hệt | R1 câu 5–6 (Bridge) |
| 3 | `DP-037` | Factory Method vs Abstract Factory | R1 câu 7 (`DP-022`) |
| 4 | `DP-041` | Library nạp vào 5 process — mấy object, mấy state | A1 §7.2 · phiên 05/10 câu 12 |
| 5 | `SD-022` | Vì sao library hệ thống phơi C API | R1 câu 3 (`DP-040`) |
| 6 | `RES-003` | Bảng con trỏ hàm trong kernel ≈ cái gì của C++ | R1 câu 9–10 |

---

## 4. R2 — Ranh giới C++ interface / `.so` (A2)

**Câu hỏi của buổi:** *"Qua ranh giới `.so`, cái gì thực sự là hợp đồng?"* Trả lời đúng: **vtable là ABI**.

### 📖 Đọc (~30′)

- [A2](../../11-design-patterns/in-practice/A2-cpp-interface-hal.md) **§1** (luồng `getInstance`, `-rdynamic`) · **§2 + §2.1** (nguồn vs nhị phân) · **§3.1–3.3** · **§4** (link thẳng vs `dlopen`) · **§5** (bản nói 60″)
- [07/abi-versioning](../../07-shared-libraries/abi-versioning.md) · [07/api-design](../../07-shared-libraries/api-design.md) · [creational §3](../../11-design-patterns/creational.md) (Singleton, vì sao Meyers thread-safe)
- 🧪 *Không bắt buộc:* [A2 §7](../../11-design-patterns/in-practice/A2-cpp-interface-hal.md) Lab 1 (race) + Lab 3a (chèn vtable), ~60′. Làm được thì R2 sẽ khác hẳn: phiên mock miệng không thay được việc **tận mắt thấy destructor chạy khi gọi `setPower`**

### 🎤 Mock — 12 câu, ~60′

| # | ID | Câu (tóm tắt) | Nguồn | Ghi chú cho interviewer |
|---|---|---|---|---|
| 1 | `DP-002` | Singleton | 🔴 weak | Theo weak-register (Ⓑ có **viết code**, ~5′) |
| 2 | `CPP-045` | `= delete` vs private cũ | 🔴 weak | Kiểm đúng chiều: người ngoài chết lúc compile, member/friend chết lúc link |
| 3 | `CPP-009` | Template ở header / trong `.so` | 🔁 retention (quá hạn) | Dạng **Ⓖ** (mọi góc cũ tính là Ⓑ) |
| 4 | `DP-025` | `IDisplayBuilder` — Builder hay Factory Method | mới | |
| 5 | `DP-026` | Đọc `getInstance()` — race ở đâu, vì sao `static` không cứu | mới | Nối thẳng DP-002 câu 1 |
| 6 | `DP-028` | API virtual thường + base trả `-ENOTSUP` — pattern gì | mới | Bẫy: tương thích **nguồn** ≠ **nhị phân** |
| 7 | `DP-027` | Chèn virtual vào giữa, giữ `.so` cũ | mới | Phải ra: không crash, exit 0, **destructor chạy** |
| 8 | `DP-033` | Version khớp mà vẫn gọi nhầm hàm | mới | Version bảo vệ API **thiếu**, không bảo vệ slot **đảo** |
| 9 | `DP-029` | `__attribute__((constructor))` tự đăng ký — đánh đổi so với link thẳng | mới | *An toàn lúc link* đổi lấy *linh hoạt lúc boot* |
| 10 | `SD-017` | API vs ABI — một thay đổi giữ API mà phá ABI | mới | |
| 11 | `CPP-067` | Ownership từng member: `shared`/`unique`/raw | 🔴 weak | Theo weak-register. Câu tự đặt: *"ai giữ cho nó sống"* |
| 12 | `SD-024` | Library báo lỗi: exception hay mã lỗi — vì sao phải nhất quán | mới | Nối `CPP-068` (đã hỏi 14/09) |

### ⚡ R2-rapid — 7 câu, ~12′

| # | ID | Câu (tóm tắt) | Nối với |
|---|---|---|---|
| 1 | `SD-028` | Name mangling, `extern "C"` để làm gì | R2 · A1 §3.1 |
| 2 | `SD-029` | `dlopen`/`dlsym` khác liên kết động thường | R2 câu 9 (`DP-029`) |
| 3 | `SD-018` | Thay đổi C++ nào thường phá ABI | R2 câu 7–8, 10 |
| 4 | `CPP-023` | Vì sao C++ ABI không ổn định giữa compiler | R2 câu 10 (`SD-017`) |
| 5 | `SD-021` | Pimpl — giải gì, cái giá | R2 · A2 §3.3 (cuối) |
| 6 | `DP-014` | Vì sao Meyers' Singleton thread-safe | R2 câu 1, 5 |
| 7 | `SD-020` 🏗️ | Library cho khách chỉ thay `.so` — thiết kế để không vỡ | Core 2 · R2 toàn bộ |

*`DP-020` để ngoài (xem [§7](#7-cố-ý-không-đưa-vào-plan--và-vì-sao)). `CPP-020` là câu weak có điều kiện gỡ cần follow-up — rapid không làm được, chuyển sang phiên daily ở [§8](#8-retention-quá-hạn--8-câu-xếp-ở-đâu).*

---

## 5. R3 — S-Box: thread 60 Hz, sensor, mq, nhiều process

**Câu hỏi của buổi:** *"Phần concurrency trong project của bạn — kể và bảo vệ từng lựa chọn."* Đây là chỗ C++ System SW và resume gặp nhau rõ nhất.

> Theo mô tả của bạn: một thread chạy **60 Hz** (chu kỳ tính toán) chờ bằng **condition variable**, **polling + lọc nhiễu** khi đọc cảm biến, **POSIX mq** để đồng bộ giữa các unit. Năm câu mới thêm ngày 05/10 bám đúng các mảnh này: `CPP-069…071`, `LNX-044`, `LNX-045`.

### 📖 Đọc (~30′)

- [concurrency §7 + **§7.1** (mới — vòng chu kỳ, dừng ngay)](../../02-modern-cpp/concurrency.md) · [os bank `OS-012`](../mock-interview/bank/os.md) (condvar: mutex + predicate)
- [ipc-linux **§4.3**](../../04-linux-system-programming/ipc-linux.md) (một bên chết khi giữ khoá) · **§5** (mq, latest-value-wins) · **§5.1** (mới — dừng thread `mq_receive`)
- [A1 §5.8](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) (vòng vsync) · **§7.2–7.3** (shm + semaphore, hai khoá hai tầng) · **§9 điểm yếu #5**
- [behavioral §3, §3.1](../../11-design-patterns/behavioral.md) (Observer, chỗ Observer không giúp được) · [in-practice/B2 §1](../../11-design-patterns/in-practice/B2-redesign-events.md)

### 🎤 Mock — 12 câu, ~60′

| # | ID | Câu (tóm tắt) | Nguồn | Ghi chú cho interviewer |
|---|---|---|---|---|
| 1 | `LNX-029` | Clock cho timeout — MONOTONIC vs REALTIME | 🔴 weak | Theo weak-register. Là nền của câu 3 và 9 |
| 2 | `OS-007` | Mutex vs semaphore | 🔁 retention (quá hạn) | **Ⓖ**. Vế *ownership* là nền của câu 10 |
| 3 | `CPP-069` | Vòng 60 Hz bằng `sleep_for(16ms)` đo ra ~48 Hz | mới | Hỏi tiếp: một chu kỳ kẹt 80 ms thì sao |
| 4 | `CPP-070` | Thread chu kỳ phải dừng ngay — viết bằng condvar | mới | **Luật ⑤ — viết code** vào coding-arena, ~10′ |
| 5 | `OS-012` | Condvar: vì sao phải kèm mutex + predicate | đã hỏi | Hỏi **sau** câu 4: soi chính code vừa viết |
| 6 | `RES-006` | Luồng dữ liệu cảm biến → độ sáng; chống nhấp nháy | đã hỏi | |
| 7 | `CPP-071` | Đọc sensor chậm — tách thread, trao giá trị bằng gì | mới | Lux là **trạng thái** ⟹ hộp thư một ô |
| 8 | `DP-006` | Observer — dùng khi nào, rủi ro | đã hỏi, **bỏ qua** 05/10 | Mức 🟢 theo calibration 05/10 |
| 9 | `RES-005` | Vì sao POSIX mq, không phải socket hay shm | đã hỏi | |
| 10 | `LNX-044` | Thread chặn trong `mq_receive` — dừng thế nào | mới | |
| 11 | `LNX-045` | Process chết khi giữ named semaphore của library | mới | Nối câu 2: semaphore **không có chủ** |
| 12 | `DP-044` | Vòng vsync ngoài mặt tiền — hai vấn đề | mới | Đóng buổi bằng điểm yếu tự nêu |

### ⚡ R3-rapid — 10 câu, ~15′

| # | ID | Câu (tóm tắt) | Nối với |
|---|---|---|---|
| 1 | `LNX-016` | Khi nào mq thay vì shm | R3 câu 9 (`RES-005`) |
| 2 | `LNX-017` | Sensor 200 mẫu/s, consumer 50/s, `mq_send` bắt đầu chặn | R3 câu 7 (`CPP-071`) |
| 3 | `CPP-018` | `atomic` đủ chưa hay cần mutex | R3 câu 7 |
| 4 | `OS-018` | Producer–consumer an toàn giữa thread | R3 câu 4–5 |
| 5 | `RES-026` | mq là IPC một máy, video wall là nhiều máy — khớp thế nào | S-Box 2 |
| 6 | `RES-025` 🏗️ | Chống nháy: chặn ở đâu, tham số nào | S-Box 1 · R3 câu 6 |
| 7 | `DP-030` 🏗️ | Observer + `onLux → setBrightness` thẳng thì sao | R3 câu 8 · behavioral §3.1 |
| 8 | `DP-031` 🏗️ | Command qua mq mà các màn vẫn đổi lệch nhau | S-Box 2 · R4 câu 10 |
| 9 | `DP-048` | Khoá ở mặt tiền rồi, driver còn khoá — thừa không | A1 §7.3 · R3 câu 11 |
| 10 | `DP-047` | `ShmGuard` RAII cho vòng vsync | R3 câu 12 (`DP-044`) |

---

## 6. R4 — Phần còn lại của resume + tiếng Anh

**Câu hỏi của buổi:** quét nốt các dòng resume chưa chạm, và hai câu tiếng Anh. Cấu trúc gần một vòng thật: câu nhẹ xen câu nặng.

### 📖 Đọc (~30′)

- Bank `RES` — đọc **khung** (không học thuộc): `RES-008` · `RES-016` · `RES-017…022` · `RES-009` · `RES-013…015` · `RES-029` · `RES-004` · `RES-027` · `RES-031`
- [behavioral §6 Memento](../../11-design-patterns/behavioral.md) + bank `DP-032`, `DP-039`
- 🇬🇧 `RES-032`, `RES-033` — đọc to bản mẫu, bấm giờ

### 🎤 Mock — 12 câu, ~60′

| # | ID | Câu (tóm tắt) | Nguồn | Ghi chú cho interviewer |
|---|---|---|---|---|
| 1 | `RES-008` | Debug xuyên tầng — một ca cụ thể | 🔴 weak | Theo weak-register. Câu đầu tiên phải là **phép cắt đôi** |
| 2 | `RES-016` | Dòng AI — chặn cách đọc bất lợi | 🔴 weak | Dạng mới: *"code AI sinh ra thì kiểm soát chất lượng sao?"* |
| 3 | `RES-017` | Preset — Cancel quay về trạng thái cũ thế nào | mới | |
| 4 | `DP-032` | Memento sách vở thiếu gì cho *"export sang màn khác"* | mới | |
| 5 | `RES-021` | Save/Load khác thread — một `std::mutex` member đã đủ chưa | mới | Nối R3: khoá bảo vệ **bất biến** nào |
| 6 | `RES-009` | Porting tool — con số đo thế nào | đã hỏi | Đủ 4 ý, nhất là ③ tool **cố ý không làm** gì, ④ verify |
| 7 | `RES-015` | Load-time: chọn song song hay priority, rủi ro phương án bị loại | đã hỏi | |
| 8 | `RES-004` | Migration 5.10 → 6.12 — cái gì vỡ | đã hỏi | Mức kể được, không đào kernel internals |
| 9 | `RES-032` 🇬🇧 | Opening bằng tiếng Anh | 🔴 weak | Theo weak-register |
| 10 | `RES-027` | Một binary hai mode — ai là master, master mất điện thì sao | mới | |
| 11 | `RES-031` | Requirement đến tay ở dạng chưa dùng được | đã hỏi | Gắn với dòng công tác R&D HQ |
| 12 | `RES-033` 🇬🇧 | *"Why do you need two levels of function-pointer table?"* | đã hỏi | Nối R1 câu 9 |

### ⚡ R4-rapid — 10 câu, ~15′

| # | ID | Câu (tóm tắt) | Nối với |
|---|---|---|---|
| 1 | `RES-013` | 3–4 s → < 0,5 s đo bằng gì, từ mốc nào | Load-time |
| 2 | `RES-014` | Phát hiện *"bị chen ngang"* bằng cách nào | Load-time |
| 3 | `RES-029` | Vì sao 100 lần boot là đủ | Load-time |
| 4 | `RES-028` 🏗️ | `SCHED_FIFO` bị chặn — còn phương án nào | R4 câu 7 (`RES-015`) |
| 5 | `RES-018` | App nói chuyện với màn hình bằng gì | Preset |
| 6 | `RES-019` | Rút dây / đổi cổng — Preset còn gắn đúng màn không | Preset |
| 7 | `RES-020` | JSON: mất điện lúc ghi · đổi định dạng | Preset · R4 câu 5 |
| 8 | `RES-022` | *"Kịp 1.0"* là nhờ em hay nhờ team | Preset |
| 9 | `RES-011` | Ứng dụng Windows MVVM — kể và lái về thế mạnh | Preset |
| 10 | `RES-030` | Dòng không phải Linux — liên quan gì vị trí này | Preset |

---

## 7. Cố ý KHÔNG đưa vào plan — và vì sao

| Phần | Vì sao để ngoài | Nếu muốn ôn |
|---|---|---|
| Video enhancement (`DP-021`) | Người học **không làm** phần này ⟹ ngoài bán kính resume | — |
| Device tree, I2C/SPI của sensor (`RES-010`, `RES-024`), Yocto (`RES-012`) | Nghiêng BSP; plan chọn trọng số C++ System SW. Resume hiện tại đã bỏ Yocto | `/mock daily track bsp` |
| C thuần trên giấy (domain `C`, phủ 10%) | Không bám một dòng resume cụ thể; đã đo là yếu ở phiên 05/10 | Xen `/mock rapid track c` giữa các buổi |
| Symbol interposition / `-fvisibility` (`DP-020`) | Phần tên cờ là T3 (calibration 05/10); `DP-002` còn ở 2 điểm ⇒ [config §6 luật ④](../mock-interview/config.md): chưa lên T2 | Sau khi `DP-002` ≥ 3 |
| 5 lab A2 | Bạn chọn không bắt buộc | Ghi chú ở R2 |
| Lab BSP, lab DBG | Ngoài trọng số plan | [archive §10](archive/datalogic-plan.md) — vẫn còn nguyên trạng thái |

---

## 8. Retention quá hạn — 8 câu, xếp ở đâu

Bảng 🔁 của weak-register có **8 câu hạn 18–23/09**, chưa hỏi. Plan chỉ hấp thụ được hai câu (bám resume): `CPP-009` → R2, `OS-007` → R3. **Sáu câu còn lại** (`CPP-029`, `CPP-019`, `CPP-024`, `OS-003`, `CPP-032`, `LNX-023`) ⇒ chạy **một** `/mock daily track cpp-system` xen giữa R2 và R3. Mỗi phiên daily có một suất retention, nên một phiên chỉ xả được một câu; số còn lại chuyển sang các phiên daily sau. Mọi câu ở đây hỏi dạng **Ⓖ**: góc cũ trước 04/10 tính là Ⓑ.

---

## 9. 📌 Bài học còn hiệu lực (từ [datalogic-plan §9](archive/datalogic-plan.md))

| # | Bài học | Áp vào buổi nào |
|---|---|---|
| 3 | **Lỗi đóng gói ≠ lỗ hổng kiến thức.** Đổi khung câu hỏi (kỹ thuật → resume) thì không truy xuất được. Chữa bằng **nói to, bấm giờ** | R1 · R4 |
| 4 | **Nói ra một thuật ngữ là mời interviewer hỏi vào đúng nó.** Nói *"Builder"* thì sẽ bị hỏi *"Builder khác Factory thế nào"* | R1 · R2 |
| 7 | **STAR phải kết bằng con số.** Có khung ⇒ 2.42 → 3.42 | R4 |
| 8 | **Khẳng định không đối chiếu được.** Gộp *"dựng được môi trường"* với *"làm được việc"* — interviewer không phân biệt gộp nhầm với bịa | Mọi buổi |
| 🆕 | **Câu sai lặp lại nhiều lần thì kiểm bank trước** (05/10: `CPP-045` sai trong bank, ứng viên đảo chiều 3 phiên liền) | Mọi buổi |

---

## 10. 🎯 Chiến thuật trả lời — rút ra từ R1 (06/10)

> Không phải kiến thức mới. Đây là **cách nói** để kiến thức đã có không bị rơi ở khâu trình bày. Mỗi điểm gắn với một câu đã hụt ở [R1](../mock-interview/sessions/2026-10-06--R1--kien-truc.md).

| # | Chiến thuật | Sinh ra từ |
|---|---|---|
| 1 | **Rút gọn bằng cách BỎ BỚT, không rút gọn THÀNH SAI.** Không nói *"N + M"* khi thật là thuật toán × backend | `DP-042` |
| 2 | **Báo trước là bản rút gọn:** *"Em kể bản 30 giây, phần nào anh muốn em mở ra thì em mở."* | `DP-042` · `RES-001` |
| 3 | **Kể kiến trúc theo ba ranh giới, mỗi ranh giới một câu "vì sao".** Sang tiếng Anh vẫn phải đủ ba | `RES-034` |
| 4 | **Chỗ hệ thật lệch sách thì tự nói ra trước** khi bị vặn | `DP-042` |
| 5 | **Chỉ gọi tên pattern khi bảo vệ được.** Chưa phân biệt được Strategy với Bridge thì nói *"tách thuật toán khỏi phần ghi phần cứng"* | `DP-023` |
| 6 | **Câu chốt trước, chi tiết sau, rồi dừng.** Mẫu `RES-023` (~25″): *"Hai tầng vì hai lý do khác nhau. Tầng một là ranh giới license giữa driver nền GPL và phần proprietary. Tầng hai là đa hình: chip driver insmod thì tự đăng ký hàm vào bảng, driver nền chỉ gọi qua bảng."* | `RES-023` |
| 7 | **Mở màn:** vấn đề bằng **một hình ảnh**, kết quả bằng **thứ đo được**, câu cuối là *"anh muốn nghe kỹ phần nào?"* — **không** phải câu có/không. Không nói *"em được giao"*; nói phần **mình tự quyết định** | `RES-001` |
| 8 | **Câu quyết định thiết kế** trả lời theo mẫu: *"Phương án kia **cho phép** lỗi X; phương án này làm X **không viết ra được**, vì Y."* | `DP-022` · `DP-040` |
| 9 | **Câu cơ học (slot, layout, ABI):** vẽ **hai layout cạnh nhau, đánh số slot**, rồi mới trả lời | `DP-043` |

---

## 11. R1′ — kể kiến trúc theo chiến lược hiện tại

**Câu hỏi của buổi:** *"Kể kiến trúc library bạn làm"* — nhưng lần này đo ba thứ khác R1: **kể ngắn đúng 30 giây** · **người không làm TV hiểu được** · **mỗi móc vừa thả có đỡ được ở mức cơ bản không**. Không đào chi tiết backend, không hỏi thiết kế lại (đã có ở R1 và weak-register).

### 📖 Đọc (~30′) — xem *"Trước R1′"* ở §📍

### 🎤 Mock — 12 câu, ~60′, trần T1 + một chút T2

| # | ID | Câu (tóm tắt) | Nguồn | Móc / ghi chú cho interviewer |
|---|---|---|---|---|
| 1 | `RES-001` | Giới thiệu + project tâm đắc | 🔴 weak | Theo weak-register. Câu cuối **không** được là câu có/không |
| 2 | `RES-007` | Picture quality và panel control — giải thích cho người ngoài ngành | 🔴 weak | Hai câu hỏi về một tấm panel; có câu nối sang kiến trúc |
| 3 | `RES-035` | Kể kiến trúc — bản 30″, rồi *"kể thêm"* sang bản 90″ | mới | Chấm: đủ ba ranh giới · câu trao quyền · không FRC/TCON |
| 4 | `DP-040` | Vì sao giữa hai vùng C++ lại là API C | 🔴 weak | Móc *"API C"*. Mức cơ bản: ABI + **chỗ duy nhất lấy khoá** |
| 5 | `CPP-006` | Đa hình runtime hoạt động thế nào (vtable/vptr) | đã hỏi | Móc *"C++ interface"* |
| 6 | `OS-007` | Mutex vs semaphore | 🔁 retention | Móc *"lấy khoá"*. Dẫn sang câu 7 |
| 7 | `LNX-045` | Process chết khi giữ named semaphore của library | mới | Móc *"shared memory + khoá"*. Mức cơ bản: semaphore **không có chủ** |
| 8 | `DP-041` | Library nạp vào 5 process — mấy object, mấy state, vì sao khởi tạo không race | 🔴 weak | Móc *"tự khởi tạo khi được nạp"* |
| 9 | `DRV-006` | Vì sao driver không đọc thẳng con trỏ user | mới | Móc *"ioctl xuống kernel"* |
| 10 | `DP-043` | `panel_ops` vs `virtual` — hai rủi ro | 🔴 weak | Móc *"bảng con trỏ hàm"*. Có output thật ở bank |
| 11 | `DP-038` | Bridge giải gì, khác Strategy chỗ nào | 🔴 weak | Móc *"tách thuật toán khỏi backend"*. Phép thử *"bỏ phần cắm vào"* |
| 12 | `RES-034` 🇬🇧 | *"Walk me through the architecture…"* | 🔴 weak | Ba ranh giới **còn đủ ba** khi sang tiếng Anh |

**Tiến độ:** phần 1 (08/10) hỏi câu 1–7 → [log](../mock-interview/sessions/2026-10-08--R1prime--ke-lai-kien-truc.md). Phần 2 hỏi câu 8–12, thêm `LNX-045` (weak, theo weak-register) và [`LNX-046`](../mock-interview/bank/linux-sysprog.md) (*vì sao semaphore thay mutex, timeout-reset hỏng ở đâu, sửa thế nào* — câu sinh ra từ góp ý ở phần 1).

**Không hỏi ở R1′ (cố ý):** `DP-042` (chi tiết backend — luật ⑤), `DP-022`, `DP-023`, `DP-046` (thiết kế, đã ở weak-register — hỏi ở phiên daily/R2).

---

## 12. R5 — CI · unit test · code quality

**Câu hỏi của buổi:** *"Quy trình đưa code của em vào sản phẩm như thế nào, và em đảm bảo chất lượng ra sao?"* — mức hiểu cơ bản, bám việc thật (bạn là **người dùng** pipeline).

### 📖 Đọc + 🧪 lab (~30′, lab nên làm trước)

- [06/unit-test-and-code-quality](../../06-build-systems/unit-test-and-code-quality.md) — **toàn bộ**, đặc biệt §1 (pipeline của bạn) và §7 (nói thế nào mà không nói quá)
- [06/ci-and-test-farm](../../06-build-systems/ci-and-test-farm.md) §1, §3 (gated check-in), §5 (tháp test)
- 🧪 **Lab pipeline 5 cổng** ở §6 của tài liệu trên: chạy `./run_ci.sh`, làm bước 2 (mutation) — mã nguồn ở [A1 §11](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)

### 🎤 Mock — 12 câu, ~60′, trần T1 + một chút T2

| # | ID | Câu (tóm tắt) | Nguồn |
|---|---|---|---|
| 1 | `BLD-021` | Vì sao không có đường submit tay vào nhánh chính | mới |
| 2 | `BLD-031` | CI / Continuous Delivery / Continuous Deployment | mới |
| 3 | `BLD-032` | Thuật ngữ pipeline: stage, job, artifact, trigger, gate | mới |
| 4 | `BLD-033` | Các tầng test (unit/integration/system) và loại test (smoke/regression/soak) | mới |
| 5 | `BLD-044` | GoogleTest vs KUnit | mới |
| 6 | `BLD-040` | Test thuật toán không có phần cứng — fake/mock/stub | mới |
| 7 | `BLD-043` | AI sinh unit test — kiểm chất lượng test thế nào | mới |
| 8 | `RES-016` | Dòng AI trên resume — chặn cách đọc bất lợi | 🔴 weak |
| 9 | `BLD-041` | Coverage 90% nghĩa là gì | mới |
| 10 | `BLD-042` | `-Werror`, static analysis, sanitizer, unit test — mỗi cái bắt gì | mới |
| 11 | `BLD-035` | Vì sao vẫn phải test trên thiết bị thật | mới |
| 12 | `BLD-026` | Flaky test trên cổng gated nguy hiểm thế nào | mới |

⚠️ **Interviewer lưu ý:** bạn chưa viết Jenkinsfile ở công ty và không đo coverage. Câu trả lời đúng ở các câu đó là *"hiểu, đã thử ở lab"* — **không** chấm thấp vì thiếu kinh nghiệm thật; **chấm thấp** nếu nói như đã làm ([unit-test-and-code-quality §7](../../06-build-systems/unit-test-and-code-quality.md)).

---

## 13. R6 — troubleshooting

**Câu hỏi của buổi:** *"Library của em crash trên TV — em làm gì?"* — bám ba tình huống thật: bản debug · bản release · không boot được.

### 📖 Đọc + 🧪 lab (~30′, lab nên làm trước)

- [09/crash-analysis-workflow](../../09-debugging/crash-analysis-workflow.md) — toàn bộ
- 🧪 [`DBG-043`](../mock-interview/bank/debugging.md) (log release → `addr2line`) và [`DBG-044`](../mock-interview/bank/debugging.md) (coredump đầu tiên bằng gdb) — **làm trên máy**, mỗi bài ~15′
- *(Nếu còn thời gian)* 🧪 [`DBG-033`](../mock-interview/bank/debugging.md), [`DBG-039`](../mock-interview/bank/debugging.md)
- [09/kernel-debugging](../../09-debugging/kernel-debugging.md) — phần oops/panic

### 🎤 Mock — 12 câu, ~60′, trần T1 + một chút T2

| # | ID | Câu (tóm tắt) | Nguồn |
|---|---|---|---|
| 1 | `DBG-045` | Ba tình huống crash — bằng chứng gì, công cụ gì, cần thêm gì | mới |
| 2 | `RES-008` | Debug xuyên tầng — một ca cụ thể | 🔴 weak |
| 3 | `DBG-003` | Core dump là gì, phân tích thế nào | đã hỏi |
| 4 | `DBG-004` | Segfault — điều tra thế nào | mới |
| 5 | `DBG-019` | Oops vs panic | đã hỏi |
| 6 | `DBG-015` | Đọc kernel oops — thông tin gì | mới |
| 7 | `DBG-021` | `printk`/`dmesg` trong driver | mới |
| 8 | `DBG-029` | Logging ở production | mới |
| 9 | `DBG-010` | Chương trình treo — điều tra thế nào | mới |
| 10 | `DBG-018` | Target không có gdb đầy đủ | đã hỏi |
| 11 | `DBG-016` | Debug lỗi cross-layer | đã hỏi |
| 12 | `DBG-042` | Chọn sanitizer: ASan vs TSan vs valgrind | 🔴 weak |

⚠️ **Interviewer lưu ý:** bạn chưa dùng gdb với coredump ở công ty (hệ thống nội bộ làm). Câu trả lời đúng là *"hệ thống làm, em hiểu nó làm gì, em đã tự làm lại ở lab"* — giống lưu ý ở R5.
