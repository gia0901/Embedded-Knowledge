# 🎯 Resume plan — C++ System Software · 4 buổi × 90′

> 📖 **Ôn trong một file:** [§14 Sổ trả lời mẫu](#14--sổ-trả-lời-mẫu--mọi-câu-mock-chính-r1r6) — 77 câu mock chính, mỗi câu đóng-mở · [§15 Sổ hình vẽ](#15--sổ-hình-vẽ--toàn-bộ-whiteboard-d0d5) — toàn bộ whiteboard D0–D5, có kịch bản nói Việt/Anh và trang gấp. Hai sổ tự sinh từ bank và whiteboard.

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
| **Buổi gần nhất** | ✅ **R1′ — xong 2026-10-09**, 14 câu, 35/56 = **2.50** (phần 1 08/10: 2.43 [log](../mock-interview/sessions/2026-10-08--R1prime--ke-lai-kien-truc.md) · phần 2 09/10: 2.57 [log](../mock-interview/sessions/2026-10-09--R1prime-p2--ke-lai-kien-truc.md)). Bốn câu weak lên 3 lần đầu: `DP-041`, `DP-043`, `DP-038`, `LNX-045` |
| ▶️ **Phiên kế** | **R5 — CI · unit test · code quality** ([§12](#12-r5--ci--unit-test--code-quality)), rồi **R6 — troubleshooting** ([§13](#13-r6--troubleshooting)), sau đó quay lại R2. *Đổi thứ tự theo yêu cầu người học (11/10): R5/R6 không phụ thuộc R2–R4; R5 trước vì lab sanitizer của R5 là nền cho `DBG-042` ở R6* |
| **Chẩn đoán mang vào** | **R1′ (08–09/10):** đọc bank + review **có tác dụng ngay** — 4 câu weak lên 3 lần đầu, `RES-001` hết câu có/không. Hai chỗ còn hụt: ① **bản kể kiến trúc** (Việt lẫn Anh) bắt đầu từ giữa, mất tầng C++ interface và câu *"chỗ duy nhất lấy khoá"* · ② **hiểu vấn đề nhưng chưa viết được cách chữa** (`HAS_OP`, ghi kiểu commit, giá trị trả về của `copy_from_user`). Một mô hình sai lặp lại: *"mỗi process định danh khoá khác nhau"* |
| 🔴 **Rủi ro lớn nhất** | **Bản kể kiến trúc chưa thuộc:** [`RES-035`](../mock-interview/bank/resume.md) bản 30″ kể ba **khối** thay vì ba **ranh giới**, thiếu câu trao quyền; [`RES-034`](../mock-interview/bank/resume.md) sang tiếng Anh ba ranh giới co còn một · **Nền dưới câu khoá liên process còn mỏng** (`DP-040`, `LNX-045`: chưa nối *"semaphore không có chủ"* sang hệ quả). [`RES-001`](../mock-interview/bank/resume.md) đã lần đầu đạt 3 (08/10) — còn thiếu vấn đề-bằng-hình-ảnh. Chữa bằng nói to + bấm giờ + chiến thuật ở [§10](#10--chiến-thuật-trả-lời--rút-ra-từ-r1-0610) |
| 📚 **Sổ trả lời mẫu** | [§14](#14--sổ-trả-lời-mẫu--mọi-câu-mock-chính-r1r6) — 77 câu mock chính, mỗi câu có trả lời mẫu dạng lời nói + câu hỏi tiếp + móc nền tảng. Mọi câu đã có dữ kiện thật (người học xác nhận 12/10). Dữ kiện S-Box đối chiếu thêm với tài liệu lắp đặt công khai của sản phẩm |

### ▶️ LÀM TIẾP — chạy đúng thứ tự

| # | Buổi | Trục | ~ | Mock chính | ⚡ Rapid pool |
|---|---|---|---|---|---|
| **R1** | 🗣️ **Kể kiến trúc** | Câu mở màn · ba ranh giới · PQ/Panel Control · Bridge · `panel_ops` — [§3](#3-r1--kể-kiến-trúc-a1) | 105′ | ✅ **2.33** ([06/10](../mock-interview/sessions/2026-10-06--R1--kien-truc.md)) | ✅ **2.50** ([06/10](../mock-interview/sessions/2026-10-06--R1-rapid--pool.md)) |
| **R1′** | 🔁 **Kể kiến trúc — làm lại** | Bản 30″/90″ · móc nền tảng ở mức cơ bản · người không làm TV vẫn hiểu — [§11](#11-r1--kể-kiến-trúc-theo-chiến-lược-hiện-tại) | 90′ | ✅ **2.50** ([08/10](../mock-interview/sessions/2026-10-08--R1prime--ke-lai-kien-truc.md) · [09/10](../mock-interview/sessions/2026-10-09--R1prime-p2--ke-lai-kien-truc.md)) | — |
| **R2** | 🔩 **Ranh giới C++ interface / `.so`** | ABI · vtable · Singleton & race · Null Object · ownership — [§4](#4-r2--ranh-giới-c-interface--so-a2) | 105′ | ⬜ | ⬜ |
| **R3** | 🧵 **S-Box & nhiều process** | Thread 60 Hz + condvar · sensor · POSIX mq · shm + semaphore — [§5](#5-r3--s-box-thread-60-hz-sensor-mq-nhiều-process) | 105′ | ⬜ | ⬜ |
| **R4** | 🧭 **Phần còn lại + tiếng Anh** | Preset · porting/AI · load-time · migration · debug · 🇬🇧 — [§6](#6-r4--phần-còn-lại-của-resume--tiếng-anh) | 105′ | ⬜ | ⬜ |
| **R5** | ⚙️ **CI · unit test · code quality** | Pipeline nhìn từ người dùng · GoogleTest/KUnit · coverage · các cổng chất lượng · 🧪 lab pipeline 5 cổng — [§12](#12-r5--ci--unit-test--code-quality) | 90′ | ⬜ | — |
| **R6** | 🔎 **Troubleshooting** | Ba tình huống crash · `addr2line` · coredump bằng gdb · oops · log — [§13](#13-r6--troubleshooting) | 90′ | ⬜ | — |

**Nhãn cột *Nguồn*:** `mới` = chưa từng hỏi · `đã hỏi` = từng hỏi ở phiên trước và đạt ⟹ **hỏi lại bình thường như câu mới** (luật ④ ở [§1](#1-năm-luật-của-plan)) · `🔴 weak` / `🔁 retention` = theo lệnh *"Lần sau hỏi"* ở weak-register.

**Gọi phiên:** gõ `/mock`. Interviewer đọc block này và đề xuất phiên kế tiếp theo thứ tự **R1′ → R2 → R2-rapid → R3 → R3-rapid → R4 → R4-rapid → R5 → R6**; danh sách câu nằm ở mục tương ứng bên dưới. R5 và R6 không phụ thuộc R2–R4, đổi thứ tự được nếu cần.

**⚡ Rapid pool — vì sao có, chạy thế nào:** mục tiêu của plan là **phủ đủ** (luật ④), mà mỗi buổi có 7–10 câu liên quan **không** chen được vào 12 câu chính. Đọc mà không bị hỏi thì không biết có nói ra được không ⟹ quét chúng bằng một phiên `rapid` **tách riêng**, chạy ngay sau mock chính (config cấm trộn rapid vào phiên sâu). Luật của rapid: hỏi thẳng đề bank, ~1 phút/câu, **tối đa 1 probe**, chấm theo [thang rapid](../mock-interview/config.md) — chỉ đo **lõi T1** (ý *Chốt* của đáp án, hoặc khung 60″ với câu `RES` 🏗️), không đào. Câu ≤ 2 vào weak-register phải ghi rõ lỗ hổng **DIỄN ĐẠT** hay **KIẾN THỨC**. Log: `YYYY-MM-DD--R<n>-rapid--pool.md`. Câu weak và retention hỏi theo dạng **Ⓖ/Ⓑ** ghi ở cột *"Lần sau hỏi"* của [weak-register](../mock-interview/weak-register.md). Plan **không** chép lại các lệnh đó.

### ⏭️ Trước R2 — ~30′ đọc theo [§4](#4-r2--ranh-giới-c-interface--so-a2), cộng 3 việc mang từ R1′

| # | Việc | ~ | Vì sao |
|---|---|---|---|
| 1 | 🎙️ Nói to **bản 30″ tiếng Anh** ở bank [RES-034](../mock-interview/bank/resume.md) + vẽ sơ đồ **bắt đầu từ app**, tới khi thuộc. Rồi bản 30″ tiếng Việt ([A1 §10.1](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)) | 5′ | `RES-035` và `RES-034` hai phiên liền mất tầng C++ interface và câu khoá |
| 2 | Chép tay **ba đoạn chữa**: `HAS_OP` ([DP-043](../mock-interview/bank/design-patterns.md)) · ghi kiểu commit + hai thuộc tính mutex ([LNX-046](../mock-interview/bank/linux-sysprog.md)) · `if (copy_from_user(...)) return -EFAULT;` ([DRV-006](../mock-interview/bank/drivers-embedded.md)) | 10′ | R1′ phần 2: hiểu vấn đề nhưng chưa **viết** được cách chữa |
| 3 | Đọc cảnh báo *"hai hàm tên gần giống"* ở [A1 §7.1](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) — `get_dimming_instance()` vs `DimmingFactory::GetDimmingInstance()` | 3′ | R2 câu 1 và 5 hỏi Singleton / race khởi tạo — đừng trộn hai tầng |

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
| 12 | `DP-044` | Vòng cập nhật định kỳ ngoài mặt tiền — hai vấn đề | mới | Đóng buổi bằng điểm yếu tự nêu |

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

**Tiến độ:** ✅ xong. Phần 1 (08/10) câu 1–7 → [log](../mock-interview/sessions/2026-10-08--R1prime--ke-lai-kien-truc.md). Phần 2 (09/10) câu 8–12 + `LNX-045` + [`LNX-046`](../mock-interview/bank/linux-sysprog.md) (câu sinh ra từ góp ý ở phần 1) → [log](../mock-interview/sessions/2026-10-09--R1prime-p2--ke-lai-kien-truc.md).

**Không hỏi ở R1′ (cố ý):** `DP-042` (chi tiết backend — luật ⑤), `DP-022`, `DP-023`, `DP-046` (thiết kế, đã ở weak-register — hỏi ở phiên daily/R2).

---

## 12. R5 — CI · unit test · code quality

**Câu hỏi của buổi:** *"Quy trình đưa code của em vào sản phẩm như thế nào, và em đảm bảo chất lượng ra sao?"* — mức hiểu cơ bản, bám việc thật (bạn là **người dùng** pipeline).

### 📖 Đọc + 🧪 lab (~30′, lab nên làm trước)

- [06/unit-test-and-code-quality](../../06-build-systems/unit-test-and-code-quality.md) — **toàn bộ**, đặc biệt §1 (pipeline của bạn) và §7 (nói thế nào mà không nói quá)
- [06/ci-and-test-farm](../../06-build-systems/ci-and-test-farm.md) §1, §3 (gated check-in), §5 (tháp test), §10 (flaky test — câu 12)
- Câu weak `RES-016` (câu 8, **Ⓖ bắt buộc**): đọc lại đáp án bank [RES-016](../mock-interview/bank/resume.md) + [BLD-043](../mock-interview/bank/build-systems.md); follow-up đã soạn sẵn ở weak-register
- 🧪 **Lab pipeline 5 cổng** ở §6 của tài liệu trên: chạy `./run_ci.sh`, làm bước 2 (mutation) — mã nguồn ở [A1 §11](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)

### 🎤 Mock — 12 câu (+1 dự phòng), ~60′, trần T1 + một chút T2

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
| 13 | `BLD-045` | Cổng ASan treo 25 phút thay vì đỏ — semaphore kẹt trong CI | mới · sinh ra từ lab 11/10, nối câu 12 và [LNX-045](../mock-interview/bank/linux-sysprog.md). Thiếu giờ thì bỏ câu này trước |

⚠️ **Interviewer lưu ý:** bạn chưa viết Jenkinsfile ở công ty và không đo coverage. Câu trả lời đúng ở các câu đó là *"hiểu, đã thử ở lab"* — **không** chấm thấp vì thiếu kinh nghiệm thật; **chấm thấp** nếu nói như đã làm ([unit-test-and-code-quality §7](../../06-build-systems/unit-test-and-code-quality.md)).

---

## 13. R6 — troubleshooting

**Câu hỏi của buổi:** *"Library của em crash trên TV — em làm gì?"* — bám ba tình huống thật: bản debug · bản release · không boot được.

### 📖 Đọc + 🧪 lab (~30′, lab nên làm trước)

- [09/crash-analysis-workflow](../../09-debugging/crash-analysis-workflow.md) — toàn bộ
- 🧪 [`DBG-043`](../mock-interview/bank/debugging.md) (log release → `addr2line`) và [`DBG-044`](../mock-interview/bank/debugging.md) (coredump đầu tiên bằng gdb) — **làm trên máy**, mỗi bài ~15′
- 🧪 [`DBG-033`](../mock-interview/bank/debugging.md), [`DBG-039`](../mock-interview/bank/debugging.md) — **bắt buộc trước câu 12**: weak-register ghi `DBG-042` phải làm 2 bài lab `DBG-030…041` trước khi hỏi lại
- Câu weak `RES-008` (câu 2, **Ⓖ bắt buộc**): đọc lại mục *"cắt đôi"* ở bank [RES-008](../mock-interview/bank/resume.md) rồi **luyện phản xạ**, không chỉ đọc: với 3 ca bất kỳ, nói câu đầu tiên là *"phép đo nào chia đôi được?"*
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

---

## 14. 📚 Sổ trả lời mẫu — mọi câu mock chính (R1–R6)

> **Một file để ôn.** Mỗi câu: đề bài + **🎙️ trả lời mẫu** (lời nói, khoảng 60–90 giây) + vài câu hỏi tiếp hay gặp + **móc nền tảng**. Không gồm rapid pool.
>
> ⚠️ **Đây là bản sao TỰ SINH.** Nguồn sự thật là khối `🎙️ Trả lời mẫu` trong [bank](../mock-interview/bank/). Muốn sửa: sửa ở bank, rồi chạy `python3 14-prep/study-plans/tools/build-answer-book.py`. Sửa tay ở đây sẽ bị ghi đè — đây là ngoại lệ có chủ đích của luật *"một đáp án, một chỗ"* (CLAUDE.md §4.7), giữ được vì phần chép là máy sinh.

**Tiêu chí của mọi câu trả lời mẫu:**

| # | Tiêu chí | Vì sao |
|---|---|---|
| 1 | **Câu chốt trước** → giải thích bằng ví dụ đời thường → một chi tiết từ việc thật → đánh đổi → **dừng** | Interviewer nghe câu đầu là biết hướng; phần sau là để họ chọn chỗ đào |
| 2 | **Người không làm TV vẫn hiểu.** Không FRC/TCON, không vsync/hsync, không frame-rate. Dùng: *Picture Quality = tính độ sáng*, *Panel Control = ra lệnh*, *panel driver*, *vòng cập nhật định kỳ* | Thuật ngữ ngành TV làm người nghe mất mạch và kéo câu hỏi vào chỗ không phải thế mạnh |
| 3 | **Hướng về nền tảng** — mỗi câu thả móc vào C++ hiện đại, shared library/ABI, IPC, concurrency, kernel/driver, debug, CI | Để interviewer có chỗ hỏi tiếp, đúng mảng mình đã ôn |
| 4 | **Ranh giới trung thực:** tách *"đã làm ở công ty"* · *"đã thử ở lab"* · *"hiểu nhưng chưa làm"* | Nói như đã làm mà bị hỏi sâu là mất điểm toàn bộ resume |
| 5 | **Khử nhạy cảm:** chỉ dùng tên trong A1/A2 (`libdisplay`, `lib_api_*`, `IDimmingAlgo`, `panel_ops`…) | Luật ② của plan |
| 6 | Câu có **⚠️ [cần bạn xác nhận]** là chỗ thiếu dữ kiện thật — điền vào bank trước khi đi phỏng vấn | Không bịa sự thật |

<!-- BEGIN ANSWER-BOOK (tự sinh bởi tools/build-answer-book.py — KHÔNG sửa tay) -->

### 📚 Sổ · 3. R1 — Kể kiến trúc (A1)

<details><summary><b>RES-001</b> · Giới thiệu + project tâm đắc</summary>

**"Em giới thiệu qua về công việc hiện tại và một project em tâm đắc nhất."**

> *"Em là kỹ sư phần mềm hệ thống, ba năm viết C/C++ trên Linux cho TV và màn hình doanh nghiệp, chạy trên SoC ARM. Việc của em đi xuyên tầng: từ **C++ interface** mà shared library đưa cho ứng dụng, xuống **library**, xuống **driver trong kernel** nói chuyện với phần cứng.*
>
> *Project em tâm đắc nhất là độ sáng tự động trên S-Box — hộp điều khiển màn hình LED quảng cáo; nhiều box ghép lại để chiếu một bức tường lớn. **Bài toán là nếu mỗi box tự chỉnh sáng theo cảm biến riêng thì mỗi ô sáng một kiểu, đổi sáng ở một lúc khác nhau — đứng xa nhìn ra rõ từng ô vuông.** Chính tài liệu lắp đặt của sản phẩm cũng ghi: các box không liên kết với nhau thì độ sáng mỗi box có thể khác nhau.*
>
> *Em làm thuật toán đọc cảm biến ánh sáng và phần đồng bộ: một box làm master, tính ra **một** mức sáng chung rồi phát qua mạng cho các box còn lại; mức sáng đổi dần từng bước nên mắt không thấy giật. Phần em tự quyết định là các lúc **chuyển trạng thái** — bật, tắt tính năng, và khởi động lại khi tính năng đang bật — những chỗ spec ban đầu bỏ trống.*
>
> *Kết quả là cả bức tường đổi sáng cùng lúc, cùng mức, nhìn như **một màn duy nhất**, và chỉ **một binary** chạy được cả chế độ một box lẫn chế độ ghép. Anh muốn em kể kỹ phần nào — thuật toán, phần đồng bộ, hay kiến trúc library bên dưới?"*

**Nếu bị hỏi tiếp**
- *"Không đồng bộ thì người xem thấy gì?"* — Mỗi ô đổi sáng ở một thời điểm và một mức khác nhau, bức tường loang lổ như bàn cờ.
- *"Em biết nó chạy đúng bằng cách nào?"* — Hai cách: **nhìn** — đổi ánh sáng phòng, các ô đổi cùng lúc và cùng mức; và **so log** mức sáng ghi xuống ở từng box — giá trị trùng nhau, thời điểm chỉ lệch trong một nhịp cập nhật của màn hình (1/60 giây), mắt không thấy được. Đó là phần em kiểm được ở tầng của mình; phía ứng dụng có thể còn cách kiểm khác.
- *"Phần nào là em tự quyết?"* — Chuyển trạng thái: bật thì chuyển dần chứ không nhảy; tắt thì giữ hay trả về mức cũ; khởi động lại khi đang bật thì khôi phục thế nào. Em kẻ bảng trạng thái × sự kiện để không sót ô nào ([RES-031](../mock-interview/bank/resume.md)).

**Vẽ:** [whiteboard D5](#hình-d5--s-box-nhiều-box--một-bức-tường-một-mức-sáng) — ba box, một master có cảm biến, một đường mạng, một bức tường.

**Móc nền tảng:** thread chạy theo chu kỳ + condition variable ([CPP-069](../mock-interview/bank/cpp.md), [CPP-070](../mock-interview/bank/cpp.md)) · POSIX message queue ([RES-005](../mock-interview/bank/resume.md)) · kiến trúc shared library ([RES-035](../mock-interview/bank/resume.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-001](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-002</b> · Một interface nhiều chipset — cơ chế; board config đọc sai thì sao</summary>

**Resume ghi *"một interface dùng chung cho nhiều chipset, thư viện chọn implementation lúc boot theo board configuration"*. Kể cơ chế đó — và điều gì xảy ra khi board configuration đọc ra sai?**

> *"Bài toán: cùng một chức năng nhưng mỗi dòng chip ghi xuống phần cứng một kiểu. Nếu để khác biệt đó lộ lên trên thì mỗi lần thêm chip là ứng dụng phải sửa.*
>
> *Bọn em giải ở **hai tầng**, cùng một ý tưởng. Ở user-space, ứng dụng chỉ thấy **một C++ interface**. Lúc khởi động, library đọc **cấu hình của board** để biết đây là model nào, rồi dựng đúng bộ cài đặt cho model đó — ứng dụng không biết và không cần biết. Ở kernel, driver chung không biết tên chip; nó gọi xuống driver của chip qua một **bảng con trỏ hàm** mà driver chip tự điền vào khi được nạp — giống bảng hàm ảo của C++ nhưng viết tay bằng C.*
>
> *Thêm một chip mới thì thêm một bộ cài đặt và một driver, **không sửa code đang chạy tốt**, và ứng dụng không phải build lại.*
>
> *Còn chế độ hỏng thì em nói thẳng: cấu hình **thiếu** thì library rơi về mặc định an toàn — thuật toán phổ biến nhất. Nhưng cấu hình **sai giá trị** thì library **nạp nhầm** cài đặt và **vẫn chạy tiếp**, không dừng lại. Lỗi khi đó có hai nguồn: phía phần cứng đưa spec sai, hoặc phía mình đọc cấu hình sai. Triệu chứng là hiển thị sai trên đúng một model — không crash, không log."*

**Nếu bị hỏi tiếp**
- *"Vậy em cải thiện thế nào?"* — Ba việc rẻ: **log rõ** cấu hình và cài đặt đã chọn ngay lúc khởi động, để lỗi lộ ở lần đọc log đầu tiên; **kiểm chéo** khi có thể — thông tin chip mà driver báo lên có khớp với cấu hình không, lệch thì cảnh báo; và đưa việc kiểm cấu hình vào **test nhà máy**. Dữ liệu cấu hình còn **sửa được** bằng dữ liệu giả, nên bring-up và test board mới không phải chờ phần cứng chuẩn.
- *"Sao không `if (chip == …)`?"* — Mỗi lần thêm chip là sửa vào file đang chạy tốt ở mọi chỗ có `if`; quên một chỗ là lỗi chỉ trên đúng model đó.
- *"Thêm hàm ảo vào interface đã phát hành được không?"* — Chỉ thêm ở **cuối**; chèn giữa làm lệch bảng hàm ảo, app cũ gọi nhầm hàm ([DP-027](../mock-interview/bank/design-patterns.md)).

**Móc nền tảng:** đa hình runtime / vtable ([CPP-006](../mock-interview/bank/cpp.md)) · Abstract Factory ([DP-022](../mock-interview/bank/design-patterns.md)) · bảng con trỏ hàm trong kernel ([DP-043](../mock-interview/bank/design-patterns.md)) · ABI ([SD-017](../mock-interview/bank/system-design.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-002](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>DP-040</b> · C API giữa hai vùng C++ — bảo vệ bằng hai lý do độc lập</summary>

**App của bạn viết C++, bên trong library cũng là C++ đa hình, nhưng giữa hai bên là một API `extern "C"` thuần. Đồng nghiệp chê "C++ → C → C++ vòng vo, phơi thẳng C++ interface cho gọn". Bảo vệ thiết kế này bằng HAI lý do độc lập — và nêu một ngoại lệ mà chính hệ của bạn có.**

> *"Em bảo vệ bằng **hai lý do độc lập**.*
>
> *Một là **ABI**. Library được nạp vào nhiều process, do nhiều team build ở những thời điểm khác nhau. Một C++ interface đưa qua ranh giới cả bố cục class, thứ tự hàm ảo, tên đã được compiler mã hoá — đổi một chút là app cũ gọi nhầm hàm mà không ai báo lỗi. Hàm C thì hợp đồng chỉ là **tên hàm và vài kiểu đơn giản**, giữ ổn định được nhiều năm.*
>
> *Hai là **khoá**. State của library nằm trong **shared memory** dùng chung cho mọi process. Mỗi hàm C là **cửa duy nhất** vào library và theo một khuôn: kiểm shared memory, lấy khoá, gọi xuống C++ bên trong, nhả khoá. Object C++ bên trong không ai ở ngoài với tới được, nên **không có đường nào đi vòng qua khoá** — cấu trúc ép, không phải kỷ luật. Nếu phơi thẳng C++ rồi khoá trong từng method thì method gọi method sẽ tự khoá lại chính mình, còn method mới quên khoá vẫn compile sạch.*
>
> *Ngoại lệ em tự nêu: vòng cập nhật định kỳ chạy **bên trong** library không đi qua cửa C, nên không được khoá đó bảo vệ. Đó là một điểm yếu em biết và có cách sửa."*

**Nếu bị hỏi tiếp**
- *"Chỉ xoá `extern "C"` thì sao?"* — Tên hàm bị mã hoá kèm kiểu tham số; app tìm tên trơn thì không thấy ⟹ lỗi **lúc nạp** (`undefined symbol`), lộ ngay. Ngược lại, với `extern "C"` mà đổi kiểu tham số thì **không ai báo** — lỗi im lặng. Nên ở biên C phải giữ chữ ký như hợp đồng.
- *"`extern "C"` có chặn truyền `std::string` qua không?"* — Không. Nó chỉ tắt việc mã hoá tên; quy tắc *"chỉ kiểu C qua biên"* là việc của người viết.
- *"ABI là gì, nói đơn giản?"* — *"API là hợp đồng bằng **tên**, ABI là hợp đồng bằng **vị trí**: byte nào ở offset nào, hàm nào ở ô thứ mấy."*

**Móc nền tảng:** ABI vs API ([SD-017](../mock-interview/bank/system-design.md)) · name mangling ([SD-028](../mock-interview/bank/system-design.md)) · mutex vs semaphore ([OS-007](../mock-interview/bank/os.md)) · process chết khi giữ khoá ([LNX-045](../mock-interview/bank/linux-sysprog.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-040](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-024</b> · Lệnh đơn để nguyên hàm — vì sao không bọc cho đồng bộ</summary>

**Trong library, dimming đi qua interface + factory. Còn Panel Control (bật tắt panel, đổi độ phân giải, đổi chế độ hiển thị) chỉ là lệnh cố định xuống panel driver, và bạn để nguyên hàm tự do gọi thẳng. Vì sao không bọc chúng cho đồng bộ? Nêu cái giá cụ thể.**

> *"Vì trừu tượng hoá phải trả tiền cho một biến thể **đã có thật**, không phải biến thể tưởng tượng. Lệnh của Panel Control — bật tắt màn, đổi chế độ hiển thị — không có thuật toán, không có state, và phần khác nhau theo chip **đã nằm trong driver của kernel**: một bộ `ioctl` chung, mỗi chip một driver. Bọc thêm interface và factory ở user-space thì không mua được gì.*
>
> *Cái giá thì cụ thể: thêm ba file và một lần gọi gián tiếp mà người đọc phải lần qua chỉ để thấy nó gọi đúng **một** `ioctl`; thêm một object và một bảng hàm ảo cho mỗi lệnh; và nặng nhất, thêm một **hợp đồng nhị phân** qua ranh giới `.so` phải giữ mãi mãi. Người đọc sau còn đi tìm một biến thể không tồn tại.*
>
> *Em dùng ba câu hỏi trước khi trừu tượng hoá: đã có **ít nhất hai** biến thể thật chưa; chúng khác nhau về **hành vi** hay chỉ khác **tham số**; có ai cần **hoán đổi** chúng không. Panel Control trượt cả ba; dimming đạt cả ba."*

**Nếu bị hỏi tiếp**
- *"Sau này có chip cần logic riêng ở user-space thì sao?"* — Lúc đó có biến thể thật thì mới tách ra interface; tách khi cần rẻ hơn mang abstraction rỗng từ đầu.

**Móc nền tảng:** YAGNI / chi phí của abstraction · ABI của virtual interface ([DP-027](../mock-interview/bank/design-patterns.md)) · `ioctl` ([DRV-006](../mock-interview/bank/drivers-embedded.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-024](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-023</b> · N thuật toán × M chip — không thành N×M lớp</summary>

**Bạn có 3 thuật toán dimming (global/local/oled) và 3 chip, mỗi chip ghi duty xuống phần cứng một kiểu khác nhau. Cách làm ngây thơ cho ra 9 class (`LocalDimmingChipA`, `OledDimmingChipB`…). Thiết kế lại thế nào? Và: pattern bạn dùng khác Strategy chỗ nào, khi code của chúng trông giống hệt nhau?**

> *"Ở đây có **hai trục thay đổi độc lập**: thuật toán (global, local, oled) và cách ghi xuống phần cứng theo chip. Kế thừa chỉ mô tả được một trục; ép cả hai vào kế thừa thì ra 3 × 3 = 9 lớp, và mỗi lớp chép lại cả thuật toán.*
>
> *Em dùng **Bridge**: giữ thuật toán là cây kế thừa, đẩy phần ghi phần cứng ra một interface riêng — `IDimmingBackend` — và mỗi thuật toán **giữ một con trỏ** tới backend. Trên sách là 3 + 3 = 6 lớp, ghép tổ hợp lúc chạy; thêm chip là thêm backend, không chép thuật toán.*
>
> *Khác Strategy chỗ nào khi code giống hệt: Strategy cắm vào một **thuật toán thay thế được** — bỏ nó ra, context vẫn là một thứ hoàn chỉnh. Bridge cắm vào **nửa còn lại của cùng một thứ** — bỏ backend ra thì thuật toán tính xong mà không chạm được phần cứng. Trong hệ của em có cả hai: thuật toán là Strategy của module dimming, còn backend là nửa Bridge của thuật toán.*
>
> *Em nói thêm cho đúng hệ thật: backend của em còn nhân theo thuật toán, vì phần cứng ghi một giá trị và ghi từng vùng khác nhau thật. Cái Bridge giữ được là **thuật toán chỉ viết một lần cho mọi chip**."*

**Nếu bị hỏi tiếp**
- *"Vậy backend nhân chip × thuật toán, vẫn là N×M?"* — Xem [DP-042](../mock-interview/bank/design-patterns.md): thứ không bị nhân là thuật toán, phần đắt nhất.

**Móc nền tảng:** Bridge vs Strategy ([DP-038](../mock-interview/bank/design-patterns.md)) · composition thay kế thừa · test thuật toán bằng fake backend ([BLD-040](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-023](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-042</b> · Backend nhân theo chip × thuật toán — còn là Bridge không</summary>

**Bạn kể dimming là Bridge: thuật toán (Global/Local/OLED) × backend theo chip. Interviewer chỉ vào tên lớp `DimmingBackendChipA_Global`, `DimmingBackendChipA_Local`… và nói: "backend của bạn nhân theo chip × thuật toán — vẫn là N×M, đâu phải Bridge". Trả lời thế nào?**

> *"Anh nói đúng một nửa, và em nói luôn trước: backend của em **nhân theo chip × thuật toán**, và **không mỏng** — backend cho thuật toán chia vùng dày ngang chính thuật toán. Lý do là phần cứng khác nhau thật: một kiểu ghi **một** giá trị cho cả màn, kiểu kia ghi **từng vùng**, và cách ghi từng vùng mỗi chip một khác.*
>
> *Nhưng thước đo của Bridge không phải con số N + M. Thứ Bridge bảo vệ là **thuật toán chỉ viết một lần cho mọi chip**. Không có Bridge, mỗi lớp kiểu `GlobalDimmingChipX` phải chứa cả thuật toán — hàng nghìn dòng bị chép theo từng chip, sửa một lỗi thuật toán phải sửa ở mọi chip. Có Bridge thì sửa một chỗ.*
>
> *Và chip được chốt **lúc build**, nên trong một binary chỉ có backend của đúng chip đó. Nếu làm lại, em tách interface backend riêng cho từng loại thuật toán, để ghép nhầm thuật toán với backend thành **lỗi compile**."*

**Nếu bị hỏi tiếp**
- *"Vậy sao không gọi là Bridge 'không chuẩn'?"* — Gọi vậy cũng được; điều quan trọng là nói đúng cái nó mua: thuật toán không bị nhân bản, hai trục đổi được độc lập.

**Móc nền tảng:** Bridge ([DP-023](../mock-interview/bank/design-patterns.md)) · chốt lúc build bằng CMake · biến lỗi runtime thành lỗi compile.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-042](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-022</b> · Hàm `makeX(ChipType)` rời vs một factory object</summary>

**Đồng nghiệp đề xuất: thay vì một `IPlatformFactory`, chỉ cần vài hàm rời — `makeDimmingAlgo(ChipType)`, `makeDimmingBackend(ChipType)`, `makePanelOps(ChipType)`. Ngắn hơn, không thêm class nào. Code compile sạch và chạy được trên bàn. Bạn phản đối bằng lý do gì? Nêu CỤ THỂ lỗi mà thiết kế đó cho phép xảy ra.**

> *"Em phản đối vì thiết kế đó **cho phép một lỗi compile sạch**: mỗi hàm nhận `ChipType` riêng, nên không gì buộc ba lời gọi dùng **cùng** một chip. Chỉ cần một đường khởi tạo truyền sai cho một trong ba — một nhánh sản phẩm, một bản merge — là bộ xử lý của chip A chạy cùng phần ghi phần cứng của chip B. Compiler không thấy gì sai, linker cũng không, board đang test có khi vẫn đúng.*
>
> *Ngoài hiện trường nó hiện ra thành sai màu hoặc một tính năng im lặng không chạy, trên **đúng một model**, không crash, không log — loại lỗi đắt nhất.*
>
> *Với một factory object, mỗi chip là một lớp factory tạo **cả họ**; có đúng một chỗ chọn factory, sau đó **không còn tham số nào để truyền sai**. Ràng buộc *"cả họ phải cùng chip"* chuyển từ kỷ luật sang hệ thống kiểu.*
>
> *Em nhượng bộ một điểm: nếu chỉ có **một** thứ cần tạo thì Abstract Factory là thừa — nó chỉ đáng khi có ít nhất hai thứ phải khớp nhau."*

**Nếu bị hỏi tiếp**
- *"Factory Method khác Abstract Factory?"* — Factory Method tạo **một** sản phẩm, lớp con quyết định loại; Abstract Factory tạo **một họ** sản phẩm phải khớp nhau.

**Móc nền tảng:** Abstract Factory ([DP-037](../mock-interview/bank/design-patterns.md)) · `DimmingFactory` tạo cặp thuật toán + backend ([A1 §5.6](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-022](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-036</b> · Null Object — đổi lấy gì, khi nào không dùng</summary>

**Null Object pattern là gì? Nó đổi lấy điều gì, và khi nào KHÔNG nên dùng?**

> *"Null Object là trả về một object **hợp lệ nhưng không làm gì** thay vì `nullptr`, để chỗ gọi không phải kiểm `null` ở mọi nơi. Trong library của em, trên sản phẩm không có tấm màn cần chỉnh độ sáng thì factory trả về chính lớp cơ sở — mọi hàm của nó trả thành công mà không làm gì. Hàng trăm chỗ gọi không cần một dòng `if` nào, và không bao giờ segfault vì quên kiểm.*
>
> *Cái giá là **lỗi không còn nổ ngay**: một việc không được làm sẽ trôi qua im lặng. Nên em không dùng nó khi việc bỏ qua âm thầm là nguy hiểm — lệnh phải chắc chắn tới phần cứng, ghi cấu hình an toàn — ở đó phải báo lỗi to.*
>
> *Và có một quyết định đi kèm: trả `OK` hay trả *"không hỗ trợ"*. Library của em trả `OK` vì hàng trăm chỗ gọi có sẵn sẽ bắt đầu báo lỗi nếu đổi; HAL phía C++ interface thì trả `-ENOTSUP` để caller phân biệt được *"đã làm"* với *"không làm được"*. Em thấy cách thứ hai đúng hơn cho code mới."*

**Nếu bị hỏi tiếp**
- *"Null Object có giúp tương thích khi interface thêm hàm mới không?"* — Ở mức mã nguồn có; ở mức nhị phân thì không — `.so` cũ có bảng hàm ảo ngắn hơn ([DP-028](../mock-interview/bank/design-patterns.md)).

**Móc nền tảng:** virtual function có thân mặc định · `-ENOTSUP` · tương thích nguồn vs nhị phân ([DP-028](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-036](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>RES-023</b> · Vì sao cần **hai** tầng bảng hàm; ai điền, lúc nào</summary>

**"CV nói kernel driver dispatch qua function-pointer table. Vì sao cần tới HAI tầng bảng đó? Ai điền bảng, điền lúc nào?"**

> *"Hai tầng vì **hai lý do khác nhau**.*
>
> *Tầng một là **ranh giới license**: driver nền của nền tảng là GPL, còn driver của từng chip là phần proprietary. Phần proprietary không được dùng trực tiếp các hàm kernel chỉ xuất cho GPL, nên giữa hai bên cần một lớp gọi qua bảng hàm thay vì link thẳng.*
>
> *Tầng hai là **đa hình theo chip**: driver nền không biết tên chip nào. Lúc khởi động, hệ thống xác định board dùng chip gì rồi nạp driver của chip đó; driver chip **tự điền hàm của nó vào bảng** — gọi là tự đăng ký. Từ đó mọi lời gọi từ driver nền đi qua bảng và tự tới đúng chip. Giống bảng hàm ảo của C++, chỉ là viết tay bằng C.*
>
> *Ai điền: chính driver chip, lúc nó được nạp. Driver nền chỉ cung cấp hàm đăng ký và gọi qua bảng."*

**Nếu bị hỏi tiếp**
- *"Bảng do người viết tay thì rủi ro gì?"* — Hai rủi ro của bảng hàm ảo: ô chưa điền (`NULL`) và chèn hàm vào giữa làm lệch ô ([DP-043](../mock-interview/bank/design-patterns.md)).
- *"Vì sao kernel không giữ ABI cho module?"* — Cố ý, để kernel được tự do sửa nội bộ; module phải build lại theo từng phiên bản.

**Móc nền tảng:** `EXPORT_SYMBOL` vs `EXPORT_SYMBOL_GPL` · self-registration ([DP-029](../mock-interview/bank/design-patterns.md)) · vtable ([CPP-006](../mock-interview/bank/cpp.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-023](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>DP-043</b> · `panel_ops` vs `virtual` — giống, khác, thừa hưởng rủi ro gì</summary>

**Trong kernel, driver nền không biết tên chip nào; nó gọi driver chip qua một struct con trỏ hàm `panel_ops` mà driver chip tự điền vào lúc init. So với `virtual` trong C++: giống gì, khác gì — và nó thừa hưởng HAI rủi ro nào của vtable?**

> *"Giống ở **ý tưởng**: driver nền gọi qua một bảng con trỏ hàm mà không biết chip nào ở phía sau — đúng như gọi hàm ảo qua con trỏ lớp cơ sở. Khác ở chỗ **ai làm**: với `virtual`, compiler tự dựng bảng, tự điền mọi ô, tự gắn bảng vào object. Với `panel_ops`, người viết driver tự khai báo struct, tự điền, tự đăng ký.*
>
> *Vì tự làm, nó thừa hưởng **hai rủi ro** của bảng hàm ảo mà không có compiler đỡ. Một: **ô chưa điền** — một chip không hỗ trợ một tính năng thì ô đó là `NULL`, gọi vào là kernel oops; driver nền phải kiểm và trả `-ENOTSUP`. Hai: **lệch ô** — ai đó chèn một hàm vào giữa struct; driver chip build trước đó vẫn điền theo bố cục cũ, nên driver nền gọi ô thứ hai lại chạy nhầm hàm khác, không báo lỗi gì.*
>
> *Cách chặn: chỉ thêm hàm vào **cuối** struct, và đặt một trường `size` ở đầu — driver nền kiểm lúc đăng ký, và trước mỗi lần gọi hàm mới thì kiểm ô đó có nằm trong bảng bên kia điền hay không."*

**Nếu bị hỏi tiếp**
- *"Thêm `get_temp` vào cuối, `.ko` cũ chưa build lại — gọi thì sao?"* — Bảng của `.ko` cũ ngắn hơn; đọc ô đó là đọc **ngoài** bảng, nhảy tới địa chỉ rác ⟹ oops. Kiểm trước khi gọi:
  ```c
  #define HAS_OP(ops, f) ((ops)->size >= offsetof(struct panel_ops, f) + sizeof((ops)->f) && (ops)->f)
  if (!HAS_OP(ops, get_temp)) return -ENOTSUP;
  ```
- *"HAL và driver build chung một RPM, cài một lần — đủ chưa?"* — Chặn được đường thường. Vẫn lọt khi hotfix một `.ko` lẻ, rollback một phần gói, hoặc `.ko` cũ còn sót trên máy. RPM là bảo đảm của **quy trình**; `size` là bảo đảm của **cấu trúc** — nên có cả hai.

**Móc nền tảng:** vtable ([CPP-006](../mock-interview/bank/cpp.md)) · chèn virtual vào giữa ([DP-027](../mock-interview/bank/design-patterns.md)) · kernel oops ([DBG-015](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-043](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-046</b> · 5 điểm yếu — làm lại thì sửa cái nào trước</summary>

**Bạn liệt kê 5 điểm yếu của hệ: interface ~150 method; ranh giới "chỉ PQ ghi độ sáng" chỉ là quy ước; logic chọn thuật toán lặp ở mỗi chip; base trả `LIB_OK` cho việc không làm; vòng cập nhật định kỳ chạm state ngoài khoá. Interviewer: "năm cái này có chung gốc không? Nếu chỉ được sửa hai cái trong quý này, bạn chọn cái nào?"**

> *"Năm điểm yếu gom về **hai bệnh**. Bệnh một: **một thứ có nhiều hơn một chủ** — độ sáng có hai đường ghi xuống phần cứng, logic chọn thuật toán lặp lại ở mỗi chip, và state dùng chung có một đường chạm vào mà không qua khoá. Bệnh hai: **hợp đồng không nói thật** — interface khoảng 150 hàm mà phần lớn không làm gì, và lớp cơ sở trả `OK` cho việc nó không làm.*
>
> *Nếu chỉ được sửa hai cái trong quý này, em chọn theo **rủi ro thấp mà chặn được cả một lớp lỗi**: thứ nhất, đưa khoá về **đi cùng state** — chỉ lấy được state qua một đối tượng RAII đã giữ khoá — đóng đúng đường đang chạm state ngoài khoá. Thứ hai, làm cho lệnh ghi độ sáng chỉ gọi được từ đúng một nơi, bằng một tham số kiểu mà chỉ nơi đó tạo được — compiler sẽ tự chỉ ra mọi chỗ đang vi phạm.*
>
> *Tách interface lớn thì làm dần vì nó là chuyện bảo trì; đổi `OK` thành *"không hỗ trợ"* thì để cuối vì nó **đổi hành vi** của API đang chạy, phải đo trước. Tóm lại: em không đập kiến trúc, em đổi **ai được làm gì**, theo thứ tự rủi ro tăng dần."*

**Nếu bị hỏi tiếp**
- *"Vì sao không sửa interface 150 hàm trước — nó trông tệ nhất?"* — Nó tệ về bảo trì, không gây sai; hai cái em chọn chặn lỗi đúng/sai.

**Móc nền tảng:** RAII lock guard · passkey idiom · ownership của state ([B1](../../11-design-patterns/in-practice/B1-redesign-architecture.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-046](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>RES-034</b> · *"Walk me through the architecture…"*</summary>

🇬🇧 **"Walk me through the architecture of the library you worked on."** — trả lời bằng tiếng Anh, ≤ 90 giây, vẽ sơ đồ song song.

**Bản 30 giây — nói trước:**
> *"It's a shared library that controls the display, used across product lines and more than ten chip families. Three boundaries: apps call a **C++ interface**; that interface calls into the library through a **plain C API**; the library talks to the **panel driver in the kernel** through `ioctl`. Inside, **Picture Quality computes** the brightness from the content, and **Panel Control issues the commands**. That's the 30-second version — which part would you like me to open up?"*

**Bản dài — khi được bảo *"tell me more"*, mỗi ranh giới thêm một câu "why":**
> *"First, apps call a C++ interface. That boundary is closed — same team, same toolchain — so C++ is fine there. Below it is a **plain C API**, because the library is loaded by many processes built at different times, and only C gives a **stable binary contract**. Each C entry point is also **the single place where we take the lock**, because the shared state lives in shared memory.*
>
> *Second, behind that C facade there are two parts: Picture Quality is algorithm-heavy and keeps state; Panel Control is simple commands that go straight down through `ioctl`. Picture Quality also writes its final brightness through Panel Control, so there is one path to the hardware.*
>
> *Third, in the kernel, a core driver that knows nothing about the chip **dispatches through a function-pointer table**, and each chip driver **registers itself** into that table when it loads. So the same idea appears twice: polymorphism with virtual functions in user space, and a hand-written vtable in the kernel."*

**If they ask** *"Why not expose the C++ classes directly?"* — *"Two independent reasons: the **ABI** — a C++ class leaks layout and vtable order across the boundary; and **locking** — the C functions are the only door into the library, so no caller can skip the lock."*

**Móc nền tảng:** cùng móc với [RES-035](../mock-interview/bank/resume.md) và [DP-040](../mock-interview/bank/design-patterns.md).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-034](../mock-interview/bank/resume.md)</sub>

</details>


### 📚 Sổ · 11. R1′ — kể kiến trúc theo chiến lược hiện tại

<details><summary><b>RES-007</b> · Picture quality và panel control — giải thích cho người ngoài ngành</summary>

**Picture quality (dimming) và panel control — em giải thích ngắn gọn cho người ngoài ngành hiểu được không?**

> *"Màn hình có hai câu hỏi.*
>
> *Câu một: **sáng bao nhiêu?** Đằng sau tấm màn là dàn đèn. Phần **picture quality** — chỗ em làm là dimming — tính xem đèn nên sáng bao nhiêu theo nội dung đang chiếu: cảnh tối thì hạ đèn ở vùng tối, nên màu đen sâu hơn và tốn ít điện hơn. Phần này là **tính toán**, mỗi dòng sản phẩm một thuật toán.*
>
> *Câu hai: **hiển thị thế nào?** Bật tắt màn, đổi chế độ hiển thị, và ghi con số độ sáng vừa tính xuống phần cứng. Đó là **panel control** — chỉ là **ra lệnh**, xuống driver của tấm màn trong kernel.*
>
> *Vì một bên là tính toán, một bên là ra lệnh, nên trong library em tách làm hai phần — đó cũng là hai nhánh của kiến trúc. Phần em viết là thuật toán dimming và đường đi của nó từ C++ interface xuống tới driver."*

**Nếu bị hỏi tiếp**
- *"Sao không để mỗi tính năng tự ghi thẳng xuống phần cứng?"* — Có **một cổng** xuống phần cứng thì dễ kiểm soát, dễ log, dễ khoá. Lệnh đơn đi thẳng panel control; tính năng nặng thuật toán thì tính xong mới đưa xuống cổng đó.

**Móc nền tảng:** kiến trúc library ([RES-035](../mock-interview/bank/resume.md)) · `ioctl` ([DRV-006](../mock-interview/bank/drivers-embedded.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-007](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-035</b> · Kể kiến trúc — bản 30″, rồi *"kể thêm"* sang bản 90″</summary>

**"Kể anh nghe kiến trúc phần library em làm."** — bắt đầu bằng bản **30 giây**; interviewer nói *"kể thêm"* thì mở sang bản **90 giây**.

**Bản 30 giây — thuộc nguyên văn:**
> *"Em làm một shared library điều khiển màn hình, dùng chung cho nhiều dòng sản phẩm và hơn mười dòng chip. Em kể theo **ba ranh giới**: app gọi một **C++ interface**; interface đó gọi xuống library qua **một API C**; library ra lệnh xuống **panel driver trong kernel** bằng `ioctl`. Trong library có hai phần: **Picture Quality tính độ sáng** theo nội dung hình, **Panel Control ra lệnh** cho tấm màn. Đó là bản 30 giây — phần nào anh muốn em mở ra thì em mở."*

**Bản 90 giây — khi được bảo *"kể thêm"*, mỗi ranh giới thêm một câu "vì sao":**
> *"Thứ nhất, app không gọi thẳng library mà gọi một C++ interface. Ranh giới đó **khép** — cùng team, cùng build — nên dùng C++ được. Nhưng library được nạp vào **nhiều process build lệch thời gian**, nên giữa interface và library em giữ một **API C thuần**: C là hợp đồng nhị phân ổn định. Mỗi hàm C đó còn là **chỗ duy nhất lấy khoá**, vì state dùng chung giữa các process nằm trong **shared memory**.*
>
> *Thứ hai, trong library: Picture Quality nặng thuật toán và có state; Panel Control chỉ là lệnh đơn đi thẳng xuống `ioctl`. Picture Quality ghi kết quả cuối cũng qua Panel Control, nên chỉ có **một đường** xuống phần cứng. Thuật toán tách khỏi phần ghi phần cứng theo chip, nên thêm chip không phải viết lại thuật toán.*
>
> *Thứ ba, trong kernel: driver chung không biết chip nào; nó gọi qua một **bảng con trỏ hàm** mà driver của chip **tự điền** khi được nạp. Cùng một ý tưởng xuất hiện hai lần: hàm ảo ở user-space, bảng hàm viết tay ở kernel."*

**Móc đã thả — chuẩn bị ở mức cơ bản:** `extern "C"`/ABI ([DP-040](../mock-interview/bank/design-patterns.md)) · shared memory + khoá ([LNX-045](../mock-interview/bank/linux-sysprog.md)) · library tự khởi tạo khi được nạp ([DP-041](../mock-interview/bank/design-patterns.md)) · `ioctl`/`copy_from_user` ([DRV-006](../mock-interview/bank/drivers-embedded.md)) · bảng con trỏ hàm ([DP-043](../mock-interview/bank/design-patterns.md)) · `virtual` ([CPP-006](../mock-interview/bank/cpp.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-035](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>CPP-006</b> · Đa hình runtime hoạt động thế nào (vtable/vptr)</summary>

**Đa hình runtime hoạt động thế nào (vtable/vptr)?**

> *"Có hai thứ. **vtable** là một bảng con trỏ hàm, mỗi **class** có một bảng, nằm ở vùng chỉ đọc của binary; ô nào class con override thì trỏ tới hàm của class con, ô nào không thì trỏ lại hàm của class cha. **vptr** là một con trỏ ẩn trong **mỗi object**, do constructor gán, trỏ tới vtable của class thật của object đó.*
>
> *Khi gọi `algo->compute(frame)` qua con trỏ lớp cơ sở, compiler **không biết** object thật là gì, nhưng nó biết `compute` nằm ở **ô số mấy** — con số đó cố định lúc compile. Nó sinh ra ba bước: đọc vptr từ object, đọc ô số đó trong bảng, gọi gián tiếp tới địa chỉ vừa đọc. Vì vậy đúng hàm của class con được chạy.*
>
> *Chi phí: mỗi object thêm 8 byte cho vptr trên máy 64-bit, mỗi lời gọi thêm một lần gián tiếp — nhưng cái giá lớn hơn là compiler **không inline được**. Và chính việc *"ô số mấy cố định lúc compile"* là lý do chèn hàm ảo vào giữa interface làm hỏng `.so` cũ."*

**Nếu bị hỏi tiếp**
- *"`sizeof` một class có vptr và một `int`?"* — 16 byte trên 64-bit: 8 vptr + 4 `int` + 4 padding để thẳng hàng 8.
- *"Gọi hàm ảo trong constructor?"* — Lúc đó vptr còn trỏ vtable của class **đang dựng**, nên chạy bản của lớp cơ sở.

**Móc nền tảng:** ABI và vtable qua `.so` ([DP-027](../mock-interview/bank/design-patterns.md)) · bảng con trỏ hàm trong kernel ([DP-043](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-006](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>OS-007</b> · Mutex vs semaphore</summary>

**Mutex và semaphore khác nhau?**

> *"Khác nhau ở **mục đích**, và từ mục đích sinh ra khác biệt về **chủ sở hữu**. Mutex để **bảo vệ** một đoạn code dùng chung: thread nào khoá thì chính thread đó mở — mutex **có chủ**. Semaphore là **bộ đếm để báo hiệu**: một bên chờ, bên kia báo, hoặc quản lý N tài nguyên — ai cũng tăng được, nó **không có chủ**.*
>
> *Ví dụ semaphore đúng chỗ: ngắt phần cứng báo có dữ liệu mới, task xử lý đang ngủ chờ trên semaphore thì thức dậy — bên báo và bên chờ là hai bên khác nhau.*
>
> *Vì mutex có chủ, hệ điều hành làm được **priority inheritance**: thread ưu tiên thấp đang giữ khoá mà thread ưu tiên cao phải chờ, thì kernel tạm nâng ưu tiên cho thread thấp để nó nhả khoá nhanh. Semaphore không biết ai đang giữ nên không làm được. Dùng semaphore nhị phân thay mutex để bảo vệ critical section là mất cái đó, và mất luôn khả năng phát hiện ai đó mở khoá nhầm."*

**Nếu bị hỏi tiếp**
- *"Library của em dùng semaphore làm khoá — vì sao, mất gì?"* — Xem [LNX-046](../mock-interview/bank/linux-sysprog.md): chọn vì đơn giản và legacy ổn định; mất chủ sở hữu ⟹ process chết khi giữ khoá thì không ai nhả.

**Móc nền tảng:** priority inversion · process chết khi giữ khoá ([LNX-045](../mock-interview/bank/linux-sysprog.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [OS-007](../mock-interview/bank/os.md)</sub>

</details>

<details><summary><b>LNX-045</b> · Process chết khi giữ named semaphore của library</summary>

**Library của bạn được nạp vào nhiều process; mọi hàm `lib_api_*` bọc giữa `sem_wait`/`sem_post` trên một named semaphore. Một tool test bị `kill -9` đúng lúc đang ở trong một hàm API. Các process còn lại ra sao? Vì sao không có `EOWNERDEAD` như robust mutex? Bạn xử lý thế nào?**

> *"Tool bị kill khi đã `sem_wait` mà chưa kịp `sem_post`, nên semaphore kẹt ở 0. Kernel dọn file, bộ nhớ của process đó, nhưng **không đụng tới semaphore**, vì semaphore là **bộ đếm không có chủ**: kernel không biết lần `wait` nào thuộc về ai, nên không biết nhả hộ cho ai. Process kế tiếp gọi bất kỳ hàm API nào sẽ **treo** ở `sem_wait`, rồi lần lượt cả hệ đứng hình.*
>
> *Robust mutex thì khác: mutex có chủ, kernel ghi nhận chủ chết, nên lần khoá sau trả `EOWNERDEAD` thay vì treo.*
>
> *Hệ thật của em bù bằng **timeout**: một API giữ khoá quá khoảng 7 giây thì semaphore bị reset để các API khác chạy tiếp, và trong shared memory có lịch sử process đã gọi để biết ai đang giữ. Em biết cách đó có lỗ: nó không phân biệt process **chết** với process chỉ **chậm**, và state đang ghi dở vẫn nằm đó. Nếu sửa, em dùng robust mutex trong shared memory, hoặc ít nhất ghi PID chủ cạnh semaphore và chỉ reset khi PID đó đã chết."*

**Nếu bị hỏi tiếp**
- *"Chủ không chết mà chỉ chậm 8 giây?"* — Luật 7 giây cho process thứ hai vào trong lúc chủ vẫn ở trong ⟹ hai bên cùng ghi; chủ xong thì `post` ⟹ semaphore lên **2**, từ đó khoá cho hai bên vào cùng lúc. Chạy thật ở [LNX-046](../mock-interview/bank/linux-sysprog.md).
- *"Chủ còn sống thì B làm gì?"* — **Không vào**: chờ tiếp hoặc trả lỗi, và log PID đang giữ.

**Móc nền tảng:** mutex vs semaphore ([OS-007](../mock-interview/bank/os.md)) · robust mutex, ghi state kiểu commit ([LNX-046](../mock-interview/bank/linux-sysprog.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [LNX-045](../mock-interview/bank/linux-sysprog.md)</sub>

</details>

<details><summary><b>DP-041</b> · Library nạp vào 5 process — mấy object, mấy state, vì sao khởi tạo không race</summary>

**Library của bạn được nạp vào 5 process. Mỗi process có `get_dimming_instance()` trả về một con trỏ toàn cục. Hỏi: có bao nhiêu object dimming? Bao nhiêu bản state thật? Thứ đó là Singleton à? Và vì sao chỗ khởi tạo không có data race?**

> *"Có **5 object**, mỗi process một bản, vì vùng dữ liệu của `.so` được map riêng cho từng process. Nhưng chỉ có **một state thật**: mức sáng và trạng thái thuật toán nằm trong **shared memory**, cả hệ một bản, được bảo vệ bằng một semaphore dùng chung. Em gói lại là: *object theo process, state theo hệ thống*.*
>
> *Nó **không phải Singleton**: `get_dimming_instance()` chỉ trả về một con trỏ toàn cục, không có gì **ép** chỉ có một object — chỉ là quy ước *"chỉ hàm khởi tạo được gán"*. Tên đúng là **Service Locator**.*
>
> *Khởi tạo không race vì nó chạy trong hàm `__attribute__((constructor))` của `.so`: hàm đó tự chạy **khi process nạp library, trước khi `main` tạo thread nào**. Không có hai thread cùng kiểm *"đã tạo chưa"*, nên không cần khoá hay magic static. Đó là một cách chữa race khác: **dời khởi tạo ra khỏi vùng có tranh chấp**."*

**Nếu bị hỏi tiếp**
- *"Bên trong có `DimmingFactory::GetDimmingInstance()` dùng static local — khác gì?"* — Đó là Meyers Singleton ở tầng dưới; nó an toàn nhờ magic static của C++11. Hai hàm tên gần giống nhưng hai cơ chế khác nhau.
- *"`.so` được `dlopen` lúc đã có nhiều thread?"* — Constructor chạy trong thread gọi `dlopen`, xong trước khi `dlopen` trả về; thread khác chưa có con trỏ tới hàm của library nên chưa gọi được.

**Móc nền tảng:** Meyers Singleton ([DP-014](../mock-interview/bank/design-patterns.md)) · race ở `getInstance()` ([DP-026](../mock-interview/bank/design-patterns.md)) · shared memory + semaphore.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-041](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DRV-006</b> · Vì sao driver không đọc thẳng con trỏ user</summary>

**Vì sao driver không được dereference con trỏ user trực tiếp? (`copy_from_user`/`copy_to_user`)**

> *"Vì con trỏ từ user-space là **một con số user đưa cho mình**, không phải một địa chỉ đáng tin. Có bốn vấn đề. Nó có thể **không hợp lệ** — chưa map, đã giải phóng — và kernel đọc vào là oops, sập cả hệ chứ không chỉ một process. Nó có thể hợp lệ nhưng **trang nhớ chưa nằm trong RAM**. Nó có thể **cố ý trỏ vào vùng của kernel** — kernel đọc ghi hộ thì user vừa sửa được bộ nhớ kernel. Và nó chỉ có nghĩa trong không gian địa chỉ của đúng process đó.*
>
> *`copy_from_user` làm ba việc: kiểm địa chỉ thuộc user-space; xử lý page fault an toàn — nó nằm trong bảng ngoại lệ của kernel nên fault ở đó thì nạp trang hoặc dừng sạch, không oops; và trả về **số byte chưa chép được**. Code đúng là:*
> ```c
> if (copy_from_user(&cfg, uarg, sizeof(cfg))) return -EFAULT;   // khác 0 = lỗi
> if (cfg.level < 0 || cfg.level > 100)          return -EINVAL;   // kiểm trên BẢN KERNEL
> ```
> *Thứ tự quan trọng: **chép trước, kiểm sau, rồi chỉ dùng bản đã chép**."*

**Nếu bị hỏi tiếp**
- *"Đọc `uarg->level` để kiểm rồi mới copy thì sao?"* — `ioctl` không khoá bộ nhớ user; thread khác cùng process có thể đổi struct giữa lúc kiểm và lúc copy ⟹ kiểm một giá trị, dùng một giá trị khác (TOCTOU).
- *"Gọi `copy_from_user` khi đang giữ spinlock?"* — Không được: nó có thể ngủ vì page fault.

**Móc nền tảng:** `ioctl` · page fault · ranh giới user/kernel ([kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DRV-006](../mock-interview/bank/drivers-embedded.md)</sub>

</details>

<details><summary><b>DP-038</b> · Bridge giải gì, khác Strategy chỗ nào</summary>

**Bridge pattern giải quyết vấn đề gì? Nó khác Strategy chỗ nào khi code trông giống hệt nhau?**

> *"Bridge giải bài toán **có hai trục thay đổi độc lập** mà kế thừa chỉ mô tả được một: tách một trục ra thành interface riêng, rồi giữ con trỏ tới nó. Không có Bridge thì số lớp nhân lên theo tích hai trục.*
>
> *Code của Bridge và Strategy giống hệt — đều là giữ con trỏ tới interface rồi uỷ nhiệm — nên em phân biệt bằng một phép thử: **bỏ phần được cắm vào đi, phần còn lại có còn là một thứ hoàn chỉnh không?**
>
> *Trong hệ dimming của em: bỏ `IDimmingAlgo` thì module dimming vẫn là một module hoàn chỉnh, chỉ mất khả năng đổi thuật toán — đó là **Strategy**. Bỏ `IDimmingBackend` thì thuật toán tính xong mà **không chạm được phần cứng** — backend là nửa còn lại của chính thuật toán — đó là **Bridge**.*
>
> *Lợi ích thực tế của Bridge với em là test: thay backend thật bằng một backend giả ghi lại các giá trị, là test được thuật toán trên máy build mà không cần phần cứng."*

**Nếu bị hỏi tiếp**
- *"Strategy đổi lúc chạy, Bridge thì sao?"* — Strategy thường đổi lúc chạy; Bridge thường gắn một lần lúc dựng object. Nhưng đó là hệ quả, không phải định nghĩa.

**Móc nền tảng:** composition · Abstract Factory tạo cặp ([DP-022](../mock-interview/bank/design-patterns.md)) · fake backend ([BLD-040](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-038](../mock-interview/bank/design-patterns.md)</sub>

</details>

*Câu đã có ở buổi trước: `RES-001` · `DP-040` · `DP-043` · `RES-034`*


### 📚 Sổ · 4. R2 — Ranh giới C++ interface / `.so` (A2)

<details><summary><b>DP-002</b> · Singleton</summary>

**Singleton là gì? Cách hiện đại trong C++?**

> *"Singleton bảo đảm một class chỉ có **một instance** và có một điểm truy cập chung. Cách hiện đại trong C++11 là **Meyers Singleton**: một biến `static` cục bộ trong hàm `instance()`.*
> ```cpp
> class Config {
> public:
>     static Config& instance() { static Config c; return c; }   // dựng ở lần gọi đầu, thread-safe
>     Config(const Config&) = delete;
>     Config& operator=(const Config&) = delete;
> private:
>     Config() = default;
> };
> ```
> *Từ C++11, chuẩn bảo đảm biến static cục bộ được khởi tạo **đúng một lần** kể cả khi nhiều thread gọi cùng lúc — compiler tự sinh một cờ bảo vệ. Constructor để private, copy thì `= delete`.*
>
> *Hai điều em luôn nói thêm. Một: chỉ phần **khởi tạo** là an toàn; gọi method của singleton từ nhiều thread vẫn cần khoá riêng. Hai: *"một instance"* là trong **một process** — library của em nạp vào năm process thì có năm bản; muốn một state cho cả hệ thì state phải nằm ở shared memory."*

**Nếu bị hỏi tiếp**
- *"Nhược điểm của Singleton?"* — Là biến toàn cục trá hình: phụ thuộc ẩn, khó test (không thay được bằng bản giả), thứ tự huỷ khó kiểm soát.
- *"Header chứa singleton được hai `.so` cùng include?"* — Số instance lúc đó do dynamic linker quyết định, không do C++ ([DP-020](../mock-interview/bank/design-patterns.md)).

**Móc nền tảng:** magic static ([DP-014](../mock-interview/bank/design-patterns.md)) · race ở `getInstance()` tự viết ([DP-026](../mock-interview/bank/design-patterns.md)) · object theo process, state theo hệ ([DP-041](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-002](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>CPP-045</b> · `= delete` vs private cũ</summary>

**`= delete` khác cách cũ (khai báo private không định nghĩa) thế nào?**

> *"Mục đích giống nhau: cấm một hàm, thường là cấm copy. Khác ở **lúc nào** và **ai** bị bắt.*
>
> *Cách cũ C++98 là khai báo copy constructor `private` và **không định nghĩa**. Nó có hai cửa: code **bên ngoài** class gọi vào thì bị chặn lúc **compile** vì `private`. Nhưng member và `friend` có quyền gọi hàm private, nên họ lọt qua compile, và chỉ chết ở bước **link** vì hàm không có thân — thông báo lỗi chỉ ra file `.o`, không chỉ ra dòng gọi. Em nhớ bằng câu: **private chặn người ngoài, thiếu định nghĩa chặn người nhà**.*
>
> *`= delete` của C++11 bắt **mọi người** ngay lúc compile, chỉ đúng dòng gọi, thông báo nói rõ *"use of deleted function"*. Nó còn xoá được hàm thường — ví dụ `void f(char) = delete;` để chặn ép kiểu ngầm từ `char` sang `int` — và nói rõ ý định ngay trong khai báo."*

**Nếu bị hỏi tiếp**
- *"Xoá copy rồi thì move còn không?"* — Khai báo copy (dù private hay `= delete`) làm compiler **không tự sinh move** ⟹ class không copy được và cũng không move được, trừ khi khai báo move tường minh.

**Móc nền tảng:** Rule of 0/3/5 · special member functions · compile vs link error.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-045](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>CPP-009</b> · Template ở header / trong `.so`</summary>

**Template hoạt động lúc nào? Vì sao định nghĩa phải nằm trong header — và có chi phí runtime không?**

> *"Template là **bản thiết kế**, chưa phải mã. Compiler chỉ sinh mã thật khi mình **dùng** nó với một kiểu cụ thể — gọi là instantiate — và việc đó xảy ra **lúc compile**, tại chính file đang dùng.*
>
> *Vì vậy định nghĩa phải nằm trong header: để sinh `max<int>`, compiler cần thấy **cả thân hàm** ở file đang dùng. Nếu thân nằm trong một `.cpp` khác thì file này chỉ thấy khai báo, không sinh được mã, và lỗi hiện ra ở bước **link** là *"undefined reference"*.*
>
> *Với library, có một cách giữ định nghĩa trong `.cpp`: nếu biết trước tập kiểu hữu hạn thì **explicit instantiation** — viết `template class Foo<int>;` trong `.cpp`. Cách này giấu được cài đặt và giảm thời gian build.*
>
> *Chi phí runtime: **không có** — mã sinh ra như viết tay, inline được, khác với hàm ảo. Cái giá trả ở chỗ khác: thời gian build, binary to ra vì mỗi kiểu một bản, và thông báo lỗi khó đọc."*

**Nếu bị hỏi tiếp**
- *"Template trong API của `.so` thì sao?"* — Phần template nằm ở header nên **build vào app**, không nằm trong `.so`; đổi template là app phải build lại — một rủi ro ABI.

**Móc nền tảng:** compile vs link · template vs virtual · code bloat trên thiết bị nhúng.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-009](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>DP-025</b> · `IDisplayBuilder` — Builder hay Factory Method</summary>

**Trong HAL của bạn, `IDisplayBuilder` chỉ có đúng một hàm `buildNewDisplayHandle()` trả về `IDisplay*`. Nó tên là "Builder" — nhưng theo GoF thì đó là pattern nào?**

> *"Theo GoF thì đó là **Factory Method**, không phải Builder, dù tên là `IDisplayBuilder`.*
>
> *Builder giải bài toán *"object có nhiều tham số, dựng qua nhiều bước"*: nhiều hàm đặt từng phần, cuối cùng gọi `build()`. Còn ở đây chỉ có **một lời gọi** trả về object hoàn chỉnh, và **lớp con** của builder — nằm trong `.so` — quyết định tạo class cụ thể nào. Đó đúng là định nghĩa Factory Method.*
>
> *Em nói rõ điều này vì nếu em nói *"em dùng Builder"*, câu tiếp theo chắc chắn là *"Builder khác Factory thế nào"*, và code em vừa kể sẽ không khớp. Tên `Builder` là quy ước có sẵn trong codebase."*

**Nếu bị hỏi tiếp**
- *"Vì sao cần factory ở đây, sao không `new` thẳng?"* — App không biết class cụ thể nằm trong `.so` nào; factory do `.so` cung cấp là cách duy nhất để app tạo object mà không phụ thuộc vào cài đặt.

**Móc nền tảng:** Factory Method vs Abstract Factory ([DP-037](../mock-interview/bank/design-patterns.md)) · self-registration ([DP-029](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-025](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-026</b> · Đọc `getInstance()` — race ở đâu, vì sao `static` không cứu</summary>

**Đọc đoạn này (trích từ HAL bạn viết). Nó có vấn đề gì khi hai thread cùng gọi lần đầu? Và vì sao chữ `static` ở dòng đầu không cứu được?**
```cpp
IDisplay* IDisplay::getInstance() {
    static IDisplay* p_instance = nullptr;
    if (p_instance == nullptr) {
        loadlib(LIB_PATH);
        if (builder) p_instance = builder->buildNewDisplayHandle();
        else         p_instance = new (std::nothrow) IDisplay();
    }
    return p_instance;
}
```

> *"Đây là mẫu **kiểm-rồi-làm** không có khoá: hai thread cùng gọi lần đầu, cả hai đều thấy `p_instance` còn `nullptr`, và cả hai cùng đi tạo. Kết quả là factory chạy **hai lần**: hai object cùng điều khiển một thiết bị, mỗi cái mở file thiết bị riêng, rò mất một file descriptor. TSan báo đúng một data race giữa dòng `if` và dòng gán.*
>
> *Chữ `static` không cứu vì bảo đảm thread-safe của C++11 chỉ áp cho **việc khởi tạo** biến static. Ở đây khởi tạo là `= nullptr` — một hằng số, compiler không cần cờ bảo vệ nào. Toàn bộ `if` và phép gán phía dưới là **code thường**.*
>
> *Sửa bằng cách để magic static làm việc: đưa toàn bộ logic tạo vào **biểu thức khởi tạo** của biến static.*
> ```cpp
> static IDisplay* p_instance = [] {
>     loadlib(LIB_PATH);
>     return builder ? builder->buildNewDisplayHandle() : new (std::nothrow) IDisplay();
> }();
> return p_instance;
> ```
> *Sau lần đầu, mỗi lần gọi chỉ còn một phép đọc nguyên tử — gọi triệu lần vẫn rẻ."*

**Nếu bị hỏi tiếp**
- *"Sao không dùng mutex?"* — Được, nhưng mỗi lần gọi đều phải khoá; magic static làm đúng việc đó mà chỉ trả giá ở lần đầu. Hoặc `std::call_once`.

**Móc nền tảng:** magic static ([DP-014](../mock-interview/bank/design-patterns.md)) · TSan ([DBG-042](../mock-interview/bank/debugging.md)) · khởi tạo trong constructor của `.so` ([DP-041](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-026](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-028</b> · API virtual thường + base trả `-ENOTSUP` — pattern gì</summary>

**Trong HAL của bạn, các API của `IDisplay` là `virtual` thường chứ không phải pure virtual, và bản base trả `-ENOTSUP`; khi không nạp được `.so` thì `getInstance()` trả về một object `IDisplay` dummy thay vì `nullptr`. Đó là pattern gì, và nó mua được HAI thứ nào?**

> *"Đó là **Null Object**: thay vì trả `nullptr` khi không nạp được `.so`, `getInstance()` trả một object hợp lệ mà mọi hàm của nó trả `-ENOTSUP`.*
>
> *Nó mua **hai thứ**. Thứ nhất, **an toàn phía người gọi**: không phải kiểm `null` ở mọi chỗ, quên kiểm cũng không segfault, và người gọi phân biệt được *"không hỗ trợ"* với *"lỗi thật"*. Thứ hai, vì các hàm là virtual **có thân** chứ không pure, khi interface thêm hàm mới thì cài đặt cũ **không phải sửa code** — hàm nào nó chưa override thì rơi về bản trả `-ENOTSUP`.*
>
> *Nhưng em phải nói chính xác thứ hai: đó là tương thích ở mức **mã nguồn** — `.so` build lại với header mới mà không sửa dòng nào. **Không phải** mức nhị phân: một `.so` đã build từ trước có bảng hàm ảo **ngắn hơn**, app mới gọi hàm mới là nhảy ra ngoài bảng và crash. Muốn gọi hàm mới an toàn trên `.so` cũ thì phải có cơ chế hỏi khả năng — version hay capability — có từ phiên bản đầu."*

**Nếu bị hỏi tiếp**
- *"Tương thích nguồn và nhị phân khác nhau thế nào, một câu?"* — Nguồn: build lại là chạy; nhị phân: không build lại vẫn chạy.

**Móc nền tảng:** Null Object ([DP-036](../mock-interview/bank/design-patterns.md)) · vtable là ABI ([DP-027](../mock-interview/bank/design-patterns.md)) · API vs ABI ([SD-017](../mock-interview/bank/system-design.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-028](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-027</b> · Chèn virtual vào giữa, giữ `.so` cũ</summary>

**Bạn thêm API mới `virtual int setBrightness(int)` vào `IDisplay`, đặt TRƯỚC `setPower` cho gọn nhóm. Rebuild app, giữ nguyên `libdisplay.so` cũ. Chạy: dòng log của `setPower` biến mất, thay vào đó destructor chạy, exit code 0, không crash. Giải thích chuyện gì đã xảy ra và rút ra luật.**

> *"Lời gọi hàm ảo được compiler dịch thành *"nhảy tới **ô số k** của bảng hàm ảo"*, và con số k **đóng băng vào binary** lúc compile.*
>
> *`.so` cũ được build khi `setPower` ở ô 0 và destructor ở ô 1. App mới được build với header đã chèn `setBrightness` lên trước, nên trong mắt app `setPower` giờ là ô 1. App gọi `setPower` thì nhảy tới ô 1 của object đến từ `.so` cũ — mà ô 1 ở đó là **destructor**. Nên destructor chạy, log của `setPower` biến mất, chương trình vẫn thoát êm với mã 0.*
>
> *Đáng sợ là **không có gì phát hiện được**: không thiếu symbol nào nên link sạch, nạp sạch, không cảnh báo — chỉ là một con số sai. Ngoài hiện trường nó hiện ra thành *"board này bật màn không lên"*.*
>
> *Luật rút ra: qua ranh giới `.so`, **bảng hàm ảo là ABI**. Hàm ảo mới chỉ được thêm vào **cuối**; không chèn giữa, không xoá, không đổi thứ tự, không thêm data member vào interface. Đó cũng là lý do người ta dùng biên `extern "C"` hoặc Pimpl."*

**Nếu bị hỏi tiếp**
- *"Bảng con trỏ hàm trong kernel có dính luật này không?"* — Có, y hệt: chèn một ô vào giữa struct ops thì driver build trước đó gọi nhầm hàm ([DP-043](../mock-interview/bank/design-patterns.md)).

**Móc nền tảng:** vtable/vptr ([CPP-006](../mock-interview/bank/cpp.md)) · API vs ABI ([SD-017](../mock-interview/bank/system-design.md)) · version không cứu được slot đảo ([DP-033](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-027](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-033</b> · Version khớp mà vẫn gọi nhầm hàm</summary>

**Bạn đã thêm `IDisplayBuilder::abiVersion()` để app hỏi `.so` xem nó build với header phiên bản nào, và app chỉ gọi API mới khi version đủ. Một hôm app in ra `impl ABI version = 2 (interface = 2)` — khớp hoàn toàn — nhưng gọi `setBrightness()` thì destructor chạy và hàm trả về số rác. Vì sao cơ chế version không cứu được?**

> *"Vì version chỉ nói *"tôi biết bao nhiêu API"*, nó **không nói bố cục** của các API đó.*
>
> *Ở đây cả app lẫn `.so` đều là *"phiên bản 2"*, cùng có `setBrightness`, nhưng hai bên build từ hai header khác nhau về **thứ tự**: một bên đặt `setBrightness` sau destructor, bên kia đặt trước. App dịch `setBrightness` thành *"ô số 1"*, còn ô số 1 trong `.so` là destructor. Version khớp hoàn toàn mà vẫn gọi nhầm hàm.*
>
> *Em phân biệt hai lớp lỗi: `.so` **thiếu** API mới thì version hay capability check chặn được. `.so` **đủ** API nhưng **thứ tự ô khác** thì không có cơ chế lúc chạy nào chặn được — chỉ chặn được ở **review** và ở **build**: luật *"chỉ thêm hàm ảo ở cuối, sau destructor"*, và một bước trong CI so bố cục bảng hàm ảo giữa hai bản. Hoặc tránh hẳn bằng biên C với struct có version.*
>
> *Triệu chứng đặc trưng đáng nhớ: hỏng **có chọn lọc** — hàm nằm trước chỗ chèn vẫn đúng, chỉ hàm phía sau lệch, không crash."*

**Nếu bị hỏi tiếp**
- *"Công cụ nào so ABI?"* — `abidiff` / `abi-compliance-checker` (tên công cụ là T3, chỉ cần biết là có bước đó trong CI).

**Móc nền tảng:** vtable là ABI ([DP-027](../mock-interview/bank/design-patterns.md)) · hợp đồng nội dung vs hợp đồng bố cục.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-033](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>DP-029</b> · `__attribute__((constructor))` tự đăng ký — đánh đổi so với link thẳng</summary>

**HAL của bạn: `.so` có một hàm `__attribute__((constructor))`, khi được `dlopen` thì tự gọi ngược lên app để đăng ký factory của mình. Vì sao làm vòng vèo vậy thay vì link thẳng `.so` vào app? Đánh đổi là gì — và điều kiện kỹ thuật nào khiến `.so` gọi được symbol nằm trong app?**

> *"Cơ chế là **tự đăng ký**: app `dlopen` cái `.so` của board; loader chạy hàm constructor của `.so`; hàm đó gọi ngược lên app để nộp factory của nó. Interface trong app **không hề biết** có bao nhiêu cài đặt; thêm cài đặt mới là thêm một `.so`, không sửa dòng nào của app.*
>
> *Vì sao vòng vèo: **một binary app chạy được trên nhiều board**. Board nào thì nạp `.so` của board đó, chọn lúc khởi động theo cấu hình. Thiếu `.so` thì app vẫn chạy ở chế độ Null Object chứ không chết ngay.*
>
> *Cái giá: lỗi mà link thẳng sẽ bắt **lúc link** — thiếu symbol, sai chữ ký — giờ chỉ lộ **lúc chạy**. ABI giữa app và `.so` phải giữ bằng tay. Debug cũng khó hơn một chút vì breakpoint trong `.so` chưa nạp chỉ là *pending*.*
>
> *Điều kiện kỹ thuật để `.so` gọi được hàm nằm trong app: app phải **xuất symbol** của nó cho dynamic linker — cờ `-rdynamic`, trong CMake là `ENABLE_EXPORTS`. Thiếu thì `dlopen` vẫn thành công nhưng `.so` không tìm thấy hàm đăng ký.*
>
> *Em tóm lại: đổi **an toàn lúc link** lấy **linh hoạt lúc khởi động** — đáng khi một binary phục vụ nhiều board."*

**Nếu bị hỏi tiếp**
- *"Constructor của `.so` có rủi ro gì?"* — Chạy trước `main`, thứ tự giữa các `.so` khó kiểm soát; không nên làm việc nặng hay phụ thuộc vào object toàn cục của `.so` khác.

**Móc nền tảng:** `dlopen`/`dlsym` ([SD-029](../mock-interview/bank/system-design.md)) · `__attribute__((constructor))` ([DP-041](../mock-interview/bank/design-patterns.md)) · Null Object ([DP-028](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-029](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>SD-017</b> · API vs ABI — một thay đổi giữ API mà phá ABI</summary>

**Phân biệt API và ABI. Cho một thay đổi **giữ nguyên API** nhưng **phá ABI**.**

> *"**API** là hợp đồng ở mức **mã nguồn**: tên hàm, kiểu tham số, ý nghĩa. **ABI** là hợp đồng ở mức **nhị phân**: kích thước và bố cục struct, offset từng trường, thứ tự bảng hàm ảo, tên symbol đã mã hoá, cách truyền tham số. Em hay nói gọn: *API là hợp đồng bằng tên, ABI là hợp đồng bằng vị trí*.*
>
> *Phá API thì **build lại là hỏng**, có lỗi compile rõ ràng. Phá ABI thì **vẫn build, vẫn chạy**, rồi hỏng im lặng — và nó chỉ quan trọng với người **thay `.so` mà không build lại app**.*
>
> *Ví dụ giữ API mà phá ABI: thêm một trường vào struct.*
> ```cpp
> struct Config { int timeout; };               // v1 — app build với bản này
> struct Config { int timeout; int retries; };  // v2 — mã nguồn dùng y hệt
> ```
> *`sizeof` từ 4 thành 8. App cũ cấp 4 byte, library mới ghi 8 byte ⟹ ghi đè bộ nhớ, crash ở chỗ chẳng liên quan. Ví dụ khác là chèn hàm ảo vào giữa interface.*
>
> *Dấu hiệu nhận ra ngoài thực tế: *"chép `.so` mới vào thì crash, chép lại bản cũ thì hết"*."*

**Nếu bị hỏi tiếp**
- *"Giữ ABI thế nào?"* — Biên C với struct mờ (opaque pointer) hoặc Pimpl; chỉ thêm hàm ảo ở cuối; soname đổi khi buộc phải phá.

**Móc nền tảng:** vì sao biên C ([DP-040](../mock-interview/bank/design-patterns.md)) · vtable là ABI ([DP-027](../mock-interview/bank/design-patterns.md)) · Pimpl ([SD-021](../mock-interview/bank/system-design.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [SD-017](../mock-interview/bank/system-design.md)</sub>

</details>

<details><summary><b>CPP-067</b> · Ownership từng member: `shared`/`unique`/raw</summary>

**Class trong shared library của bạn. Với TỪNG member, lựa chọn ownership đúng hay sai — và câu hỏi nào bạn tự đặt để quyết định?**
```cpp
class DisplayController {
    std::shared_ptr<Panel>    panel_;
    std::unique_ptr<IBackend> backend_;
    Logger*                   logger_;      // raw
    std::vector<IObserver*>   observers_;   // raw
};
```

> *"Em chỉ hỏi một câu cho từng member: **ai giữ cho nó sống?** — không phải *ai dùng nó*.*
>
> *`unique_ptr<IBackend>`: đúng, nếu controller là chủ duy nhất và backend chết theo controller — đây là ca phổ biến nhất.*
>
> *`shared_ptr<Panel>`: chỉ đúng nếu panel thật sự có **nhiều chủ** và không biết ai chết sau cùng. Nếu chỉ là *"nhiều nơi cùng dùng"* thì sai: `shared_ptr` dùng thừa che mất lỗi vòng đời thay vì chặn nó, và tốn một bộ đếm nguyên tử ở mỗi lần copy.*
>
> *`Logger*` thô: đúng, với điều kiện logger **sống lâu hơn** controller — thường là đối tượng toàn cục.*
>
> *`vector<IObserver*>` thô: đúng nếu mỗi observer tự quản vòng đời và **chắc chắn huỷ đăng ký trước khi chết**. Nếu không bảo đảm được điều đó thì vector giữ con trỏ treo, lần thông báo sau là lỗi không xác định — lúc đó em dùng `weak_ptr` hoặc một token RAII tự huỷ đăng ký trong destructor.*
>
> *Gọn lại: một chủ thì `unique_ptr`; nhiều chủ không đoán được ai chết sau thì `shared_ptr`; chỉ dùng trong lúc chủ còn sống thì con trỏ thô hoặc tham chiếu; cần biết nó còn sống không thì `weak_ptr`."*

**Nếu bị hỏi tiếp**
- *"Qua ranh giới `.so` có nên trả `shared_ptr`?"* — Chỉ khi cả hai bên cùng compiler và cùng thư viện chuẩn; với biên C thì không — trả handle và hàm huỷ.

**Móc nền tảng:** RAII · smart pointers · Observer và vòng đời ([DP-006](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-067](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>SD-024</b> · Library báo lỗi: exception hay mã lỗi — vì sao phải nhất quán</summary>

**Thư viện của bạn nên báo lỗi bằng exception hay mã lỗi? Vì sao phải chọn NHẤT QUÁN?**

> *"Em chọn theo ngữ cảnh. Library có **biên C** — như library của em — thì bắt buộc **mã lỗi**: exception đi qua hàm C là hành vi không xác định. Firmware hoặc dự án cấm exception cũng vậy. Library C++ thuần cho ứng dụng thường thì exception cho lỗi **thật sự bất thường**, còn lỗi **được mong đợi** — không tìm thấy, hết dữ liệu — thì trả `std::optional` hoặc kiểu kết quả.*
>
> *Nhưng điều quan trọng hơn là **nhất quán**. Trộn hai kiểu thì người dùng phải kiểm cả hai đường ở mọi lời gọi; thực tế họ sẽ quên một trong hai, và lỗi lọt qua im lặng. Một library dùng toàn mã lỗi, dù kém hơn, vẫn an toàn hơn library *"phần lớn trả mã lỗi nhưng vài hàm ném"*.*
>
> *Dù chọn cách nào em giữ ba quy tắc: hàm trả mã lỗi gắn `[[nodiscard]]` để compiler cảnh báo khi bị bỏ qua; constructor không trả được mã lỗi thì dùng factory trả kết quả; và destructor không bao giờ ném."*

**Nếu bị hỏi tiếp**
- *"Exception tốn gì?"* — Gần như không tốn khi không có lỗi; khi ném thì đắt và thời gian khó đoán — lý do hệ nhúng thời gian thực hay cấm.

**Móc nền tảng:** biên C ([DP-040](../mock-interview/bank/design-patterns.md)) · `-ENOTSUP` / mã lỗi kiểu kernel · `[[nodiscard]]`.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [SD-024](../mock-interview/bank/system-design.md)</sub>

</details>


### 📚 Sổ · 5. R3 — S-Box: thread 60 Hz, sensor, mq, nhiều process

<details><summary><b>LNX-029</b> · Clock cho timeout — MONOTONIC vs REALTIME</summary>

**Thiết bị nhúng KHÔNG có RTC pin nuôi — mất điện là đồng hồ về mốc mặc định, tới khi NTP đồng bộ được (30–60 s sau boot). Đoạn timeout này sinh ra HAI triệu chứng ngược nhau ngoài hiện trường. Vì sao?**

```c
time_t start = time(NULL);
send_scan_command();
while (!scan_done()) {
    if (time(NULL) - start > 5) { log("timeout"); return -1; }
}
```
- ① Đôi khi **timeout ngay lập tức** dù module trả lời bình thường.
- ② Đôi khi **treo hàng giờ** mới chịu timeout.

> *"Vì `time()` đọc **giờ theo lịch** — `CLOCK_REALTIME` — mà giờ theo lịch **nhảy được**. Thiết bị không có pin nuôi đồng hồ, nên lúc mới boot đồng hồ ở một mốc mặc định; 30 đến 60 giây sau NTP đồng bộ và **chỉnh giờ** đột ngột. Nếu cú chỉnh rơi vào giữa hai lần đọc `time()`:*
> - *chỉnh **tiến** ⟹ hiệu số vọt lên hàng nghìn giây ⟹ **timeout ngay lập tức** dù module trả lời bình thường;*
> - *chỉnh **lùi** ⟹ hiệu số âm hoặc rất nhỏ ⟹ **chờ hàng giờ** mới timeout.*
>
> *Hai triệu chứng ngược nhau từ một dòng code chính là dấu hiệu *"có ai sửa đồng hồ giữa chừng"*.*
>
> *Sửa: đo **khoảng thời gian** thì dùng **`CLOCK_MONOTONIC`** — đếm đều từ lúc boot, không ai chỉnh được, không bao giờ lùi; trong C++ là `std::chrono::steady_clock`. Quy tắc một câu: hỏi *"mấy giờ"* — để ghi log, hạn chứng chỉ — thì dùng giờ lịch; hỏi *"bao lâu"* — timeout, đo hiệu năng, chạy định kỳ — thì dùng monotonic."*

**Nếu bị hỏi tiếp**
- *"Máy ngủ (suspend) thì monotonic có đếm không?"* — Không; cần đếm cả lúc ngủ thì dùng `CLOCK_BOOTTIME`.
- *"Ở S-Box em dùng clock nào cho vòng chu kỳ?"* — `steady_clock`, cùng lý do ([CPP-069](../mock-interview/bank/cpp.md)).

**Móc nền tảng:** `steady_clock` vs `system_clock` · NTP · timeout của `condition_variable` ([CPP-070](../mock-interview/bank/cpp.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [LNX-029](../mock-interview/bank/linux-sysprog.md)</sub>

</details>

<details><summary><b>CPP-069</b> · Vòng 60 Hz bằng `sleep_for(16ms)` đo ra ~48 Hz</summary>

**Thread tính độ sáng của S-Box phải chạy 60 Hz. Đồng nghiệp viết như dưới, đo ngoài hiện trường chỉ được khoảng 48 Hz. Vì sao? Sửa thế nào — và nếu một chu kỳ `compute()` bị kẹt 80 ms thì bản sửa của bạn làm gì ở các chu kỳ sau?**
```cpp
while (running) {
    compute();                                         // ~3 ms
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}
```

> *"Vì `sleep_for(16ms)` ngủ đủ 16 ms **sau khi** `compute()` xong. Một chu kỳ thật là 3 ms tính + 16 ms ngủ + độ trễ đánh thức — khoảng 20 ms, tức chỉ hơn 48 lần mỗi giây. Thời gian làm việc **cộng dồn** vào chu kỳ thay vì nằm trong nó.*
>
> *Em sửa bằng cách hẹn theo **mốc tuyệt đối** trên `steady_clock`:*
> ```cpp
> auto next = steady_clock::now();
> while (running) {
>     compute();
>     next += period;                                  // mốc kế tiếp tính từ mốc trước
>     if (steady_clock::now() > next + period) {       // trễ quá một chu kỳ
>         ++overruns;  next = steady_clock::now();     // bỏ nhịp, không chạy bù
>     }
>     std::this_thread::sleep_until(next);
> }
> ```
> *Sai số không còn cộng dồn: em đo thật 120 chu kỳ, `sleep_for` ra 48,6 lần/giây, `sleep_until` ra đúng 60.*
>
> *Còn một chu kỳ bị kẹt 80 ms: nếu cứ `next += period` thì các mốc đã qua hết, thread chạy liền năm lần để đuổi, trên dữ liệu đã cũ. Em chọn **bỏ nhịp và đếm**: dời mốc về hiện tại, chạy tiếp đều nhịp, và tăng bộ đếm overrun để biết hệ có đang quá tải không."*

**Nếu bị hỏi tiếp**
- *"Sao không lấy 16 ms trừ thời gian compute?"* — Vẫn trôi, vì độ trễ đánh thức và làm tròn tích luỹ mỗi chu kỳ; mốc tuyệt đối thì không.
- *"Sao `steady_clock`?"* — `system_clock` nhảy theo NTP ([LNX-029](../mock-interview/bank/linux-sysprog.md)).

**Móc nền tảng:** `std::chrono` · `sleep_until` · thread dừng ngay ([CPP-070](../mock-interview/bank/cpp.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-069](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>CPP-070</b> · Thread chu kỳ phải dừng ngay — viết bằng condvar</summary>

**Thread 60 Hz ở trên dùng `std::atomic<bool> stop` + `sleep_until`. Khi service tắt, systemd chỉ chờ có hạn, và có lúc chu kỳ được cấu hình dài tới 1 giây. Viết lại để thread dừng gần như ngay lập tức mà vẫn giữ nhịp. Nêu ba chỗ dễ viết sai.**

> *"`sleep_until` **không đánh thức được giữa chừng**, nên cờ `stop` chỉ được nhìn thấy khi hết giấc ngủ hiện tại — chu kỳ 1 giây là chờ tới 1 giây. Em thay bằng `condition_variable::wait_until(lock, next, predicate)`: vừa hẹn đúng mốc, vừa thức ngay khi có `notify`.*
> ```cpp
> class Ticker {
>     std::mutex m_; std::condition_variable cv_; bool stop_ = false;
>     std::thread th_;                                   // khai báo CUỐI CÙNG
> public:
>     explicit Ticker(std::chrono::milliseconds period) : th_([this, period] {
>         std::unique_lock<std::mutex> lk(m_);
>         auto next = std::chrono::steady_clock::now();
>         for (;;) {
>             next += period;
>             if (cv_.wait_until(lk, next, [this] { return stop_; })) break;
>             lk.unlock(); tick(); lk.lock();            // không giữ khoá khi làm việc
>         }
>     }) {}
>     ~Ticker() {
>         { std::lock_guard<std::mutex> lk(m_); stop_ = true; }
>         cv_.notify_one();
>         th_.join();
>     }
> };
> ```
> *Đo thật với chu kỳ 1 giây: bản `sleep_until` dừng mất 900 ms, bản condvar dừng gần như 0 ms.*
>
> *Ba chỗ dễ sai: một, ghi `stop_` **không giữ mutex** — `notify` có thể lọt vào khe giữa *"kiểm cờ"* và *"đi ngủ"*, thread ngủ thêm trọn một chu kỳ. Hai, khai báo `std::thread` **trước** các member nó dùng — member được dựng theo thứ tự khai báo, nên thread đã chạy khi member sau chưa dựng xong; em từng bắt được đúng lỗi này bằng TSan. Ba, gọi `tick()` khi **vẫn giữ khoá** — destructor phải chờ `tick()` xong mới ghi được cờ."*

**Nếu bị hỏi tiếp**
- *"`std::jthread` + `stop_token` thì sao?"* — `jthread` chỉ *yêu cầu* dừng; thread đang ngủ trong `sleep_until` vẫn không thấy yêu cầu đó, và phần chờ có nhận `stop_token` là C++20.

**Móc nền tảng:** condvar cần mutex + predicate ([OS-012](../mock-interview/bank/os.md)) · vòng chu kỳ ([CPP-069](../mock-interview/bank/cpp.md)) · RAII.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-070](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>OS-012</b> · Condvar: vì sao phải kèm mutex + predicate</summary>

**Condition variable: vì sao phải đi kèm mutex và predicate (dùng while)?**

> *"`wait()` làm ba việc: **nhả mutex** rồi đưa thread đi ngủ — không tốn CPU; ngủ tới khi được đánh thức; **giành lại mutex** rồi mới trả về.*
>
> *Cần mutex vì hai lý do. Một, điều kiện mình chờ là dữ liệu dùng chung — hàng đợi, cờ — nên phải được bảo vệ. Hai, sâu hơn: cặp *"kiểm điều kiện rồi đi ngủ"* phải là **một bước**. Không có mutex thì `notify` của bên kia có thể lọt đúng vào khe giữa hai bước — mình thấy điều kiện sai, tín hiệu bay qua, rồi mình mới ngủ, và ngủ mãi. Đó là **lost wakeup**.*
>
> *Cần vòng `while` — hay predicate — vì `wait()` trả về **không có nghĩa** điều kiện đã đúng. Có thể là đánh thức giả mà chuẩn cho phép; hoặc phổ biến hơn, có `notify` thật nhưng thread khác giành khoá trước và lấy mất dữ liệu. Dùng `if` thì mình rơi thẳng vào đọc hàng đợi rỗng.*
> ```cpp
> std::unique_lock<std::mutex> lk(m);
> cv.wait(lk, [] { return !q.empty(); });   // tương đương while (q.empty()) cv.wait(lk);
> ```
> *Em mặc định dùng bản có predicate: nó kiểm điều kiện **trước** khi ngủ lần đầu, và không viết nhầm thành `if` được."*

**Nếu bị hỏi tiếp**
- *"Notify khi đang giữ khoá hay sau khi nhả?"* — Đều đúng; nhả trước rồi notify thì bên kia không phải thức dậy để chờ khoá. Điều bắt buộc là **ghi điều kiện khi giữ khoá**.

**Móc nền tảng:** thread dừng ngay ([CPP-070](../mock-interview/bank/cpp.md)) · producer–consumer ([OS-018](../mock-interview/bank/os.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [OS-012](../mock-interview/bank/os.md)</sub>

</details>

<details><summary><b>RES-006</b> · Luồng dữ liệu cảm biến → độ sáng; chống nhấp nháy</summary>

**Kể luồng dữ liệu từ cảm biến ánh sáng tới lúc màn hình đổi độ sáng. Làm sao để màn không nhấp nháy liên tục?**

> *"Chuỗi dữ liệu có bốn khúc. Một, **cảm biến ánh sáng nối qua I2C**; một library cảm biến riêng đọc nó theo nhịp của họ, 60 lần mỗi giây. Hai, phía library của em **lấy giá trị mới nhất mỗi 400 ms**. Ba, **lọc nhiễu**. Bốn, tính mức sáng mục tiêu rồi **đi dần** tới đó và ghi xuống phần cứng.*
>
> *Cách lọc nhiễu: em chia độ sáng môi trường thành **các khoảng** bằng một mảng ngưỡng. Mỗi lần đọc, giá trị rơi vào khoảng nào thì đếm cho khoảng đó. Nếu **10 lần đọc liên tiếp — tức 4 giây — cùng rơi vào một khoảng** thì coi là ổn định, lấy mức sáng của khoảng đó để tính mục tiêu mới; chỉ cần một lần rơi sang khoảng khác là đặt lại bộ đếm và bắt đầu một cửa sổ 4 giây mới. Một bàn tay lướt qua hay đèn flash không đủ 4 giây nên không làm đổi mục tiêu, và việc chia thành khoảng giúp giá trị dao động nhẹ quanh một mức không làm đổi sáng.*
>
> *Chống nhấp nháy thì có hai lớp: mục tiêu chỉ đổi khi ánh sáng đã ổn định 4 giây, và khi đổi thì một thread chạy theo chu kỳ đưa độ sáng **tiến từng bước nhỏ**, nên mắt thấy màn sáng lên từ từ chứ không nhảy bậc.*
>
> *Đánh đổi là **mượt hay nhạy**: bật đèn phòng thì màn phải mất ít nhất 4 giây mới bắt đầu đổi theo. Với biển quảng cáo, nơi ánh sáng thay đổi chậm, em chọn mượt."*

**Nếu bị hỏi tiếp**
- *"Ánh sáng dao động ngay ranh giới hai khoảng thì sao?"* — Cửa sổ cứ bị đặt lại nên mục tiêu giữ nguyên — đó là hành vi mong muốn, màn không nhảy qua lại. Cải tiến được nữa là dùng **hysteresis**: ngưỡng đi lên khác ngưỡng đi xuống.
- *"Mất liên lạc với cảm biến?"* — Độ sáng được giữ ở **mức mặc định** — tài liệu lắp đặt của sản phẩm cũng ghi đúng hành vi này; kiểm lại cáp và khởi động lại box thì chức năng chạy lại.
- *"Sao library của em không tự đọc I2C luôn?"* — Đọc cảm biến là việc của library cảm biến, chạy theo nhịp riêng; library của em chỉ cần **giá trị mới nhất** — đúng mô hình hộp thư một ô ([CPP-071](../mock-interview/bank/cpp.md)).

**Móc nền tảng:** thread chu kỳ + condvar ([CPP-069](../mock-interview/bank/cpp.md), [CPP-070](../mock-interview/bank/cpp.md)) · latest-value ([CPP-071](../mock-interview/bank/cpp.md)) · I2C · polling vs interrupt.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-006](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>CPP-071</b> · Đọc sensor chậm — tách thread, trao giá trị bằng gì</summary>

**Đọc cảm biến ánh sáng (polling, lọc nhiễu) mất 5–20 ms và đang nằm ngay trong vòng 60 Hz ⟹ vỡ nhịp. Bạn tách việc đọc sang một thread riêng. Hai thread trao giá trị lux cho nhau bằng gì — queue, `std::atomic`, hay mutex? Vì sao?**

> *"Câu quyết định là: giá trị lux là **trạng thái** hay **sự kiện**? Nó là trạng thái — vòng tính chỉ cần **giá trị mới nhất**; mẫu cũ đã sai so với hiện tại. Nên em không dùng hàng đợi: hàng đợi giữ mọi mẫu, vòng tính sẽ xử lý dữ liệu cũ. Em dùng **hộp thư một ô**: thread đọc cảm biến ghi đè, vòng tính lấy bản mới nhất.*
>
> *Nếu chỉ có **một số** thì `std::atomic<int32_t>` là đủ — không khoá, không chặn. Nhưng em muốn kèm **thời điểm đọc** và cờ **hợp lệ**, mà nhiều trường phải nhất quán với nhau thì hai atomic riêng không đủ — có thể đọc lux mới với thời điểm cũ. Lúc đó em dùng mutex bảo vệ một struct nhỏ, và phần khoá chỉ là **một phép copy**, gần như không tranh chấp.*
> ```cpp
> struct Sample { int32_t lux; steady_clock::time_point at; bool valid; };
> Sample latest() { std::lock_guard<std::mutex> lk(m_); return s_; }
> ```
> *Thời điểm đọc quan trọng vì hộp thư một ô **luôn** trả về một giá trị, kể cả khi thread cảm biến đã treo mười giây. Vòng tính kiểm *"mẫu này cũ bao lâu"* để chuyển sang chế độ an toàn — giữ độ sáng hiện tại và ghi log — thay vì chỉnh theo dữ liệu đã chết."*

**Nếu bị hỏi tiếp**
- *"Hệ thật của em làm thế nào?"* — Đúng mô hình này: một library cảm biến riêng đọc I2C 60 lần mỗi giây; library của em chỉ lấy **giá trị mới nhất** mỗi 400 ms rồi lọc bằng cửa sổ ổn định 4 giây ([RES-006](../mock-interview/bank/resume.md)).
- *"Lọc đặt ở đâu?"* — Khử nhiễu ở thread đọc, theo nhịp cảm biến; giới hạn tốc độ thay đổi ở vòng tính.

**Móc nền tảng:** `std::atomic` vs mutex ([CPP-018](../mock-interview/bank/cpp.md)) · latest-value-wins ([ipc-linux §5](../../04-linux-system-programming/ipc-linux.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [CPP-071](../mock-interview/bank/cpp.md)</sub>

</details>

<details><summary><b>DP-006</b> · Observer — dùng khi nào, rủi ro</summary>

**Observer pattern dùng khi nào? Rủi ro?**

> *"Observer là: một nguồn sự kiện giữ **danh sách callback** đã đăng ký; trạng thái đổi thì gọi lần lượt từng cái. Nguồn không cần biết ai đang nghe, nên thêm người nghe mới không phải sửa nguồn. Mọi API kiểu `register_callback()` đều là Observer.*
>
> *Dùng khi một nguồn mà **nhiều** bên quan tâm, và số bên đó thay đổi. Chỉ có một người nghe thì gọi thẳng là đủ.*
>
> *Rủi ro số một là **vòng đời**: người nghe bị huỷ mà quên huỷ đăng ký, nguồn vẫn giữ con trỏ và lần thông báo sau gọi vào object đã chết. Em giữ `weak_ptr` thay cho con trỏ thô: trước khi gọi thì `lock()`, chết rồi thì tự gỡ khỏi danh sách — biến crash thành tự dọn. Rủi ro thứ hai là đăng ký hay huỷ **ngay trong lúc** đang thông báo, làm hỏng vòng lặp trên danh sách — thông báo trên một bản sao.*
>
> *Một bẫy theo miền của em: nối thẳng *"cảm biến đổi → đặt độ sáng"* thì đúng pattern nhưng sai sản phẩm — màn sẽ nhấp nháy theo từng mẫu nhiễu; phía sau vẫn cần lọc và đi dần."*

**Nếu bị hỏi tiếp**
- *"Thông báo từ ngắt phần cứng?"* — Ngắt chỉ đẩy sự kiện vào hàng đợi; một thread thường mới gọi các callback.

**Móc nền tảng:** `weak_ptr` · ownership ([CPP-067](../mock-interview/bank/cpp.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-006](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>RES-005</b> · Vì sao POSIX mq, không phải socket hay shm</summary>

**Em dùng POSIX message queue để đồng bộ độ sáng giữa nhiều thiết bị. Vì sao chọn message queue mà không phải socket hay shared memory?**

> *"Em làm rõ phạm vi trước: message queue của POSIX chỉ chạy **trong một máy**. Ở S-Box có hai chặng. Chặng một **trong một box**: library của em đưa mức sáng chuẩn vào message queue, process ứng dụng trên cùng box nhận về. Chặng hai **giữa các box**: các box nối với nhau qua mạng LAN, mỗi box một IP; đường mạng đó mang cả nội dung hiển thị lẫn lệnh điều khiển, và mức sáng tự động là một trong các lệnh điều khiển. Phần mạng do tầng ứng dụng đảm nhận.*
>
> *Cho chặng library → ứng dụng, em chọn message queue vì bản tin **nhỏ và thưa** — một mức sáng, chỉ khi thay đổi. Message queue giữ **ranh giới bản tin** — không phải tự cắt gói như socket luồng byte; có **độ ưu tiên** cho từng bản tin; và có **hàng đợi sẵn** — bên nhận bận thì bản tin vẫn nằm đó chờ.*
>
> *Shared memory nhanh hơn nhưng **không có cơ chế báo hiệu** — phải tự thêm semaphore hay eventfd, tự đồng bộ, và một bên chết khi giữ khoá là bên kia kẹt; quá đắt cho vài byte. Socket trong máy thì dùng được, nhưng phải tự định khung bản tin và thừa cho giao tiếp một chiều trong một box."*

**Nếu bị hỏi tiếp**
- *"Hàng đợi đầy thì sao?"* — Mặc định chỉ khoảng 10 bản tin; đầy thì `mq_send` chặn, hoặc trả `EAGAIN` nếu mở non-blocking. Với mức sáng — một trạng thái — em ưu tiên bản tin mới nhất, bỏ bản cũ.
- *"Sao không dùng luôn socket cho cả hai chặng?"* — Mỗi chặng chọn theo bản chất của nó: trong máy thì message queue đơn giản và có sẵn ranh giới bản tin; giữa các máy thì bắt buộc là mạng.
- *"Máy con biết mình là con thế nào; master chết thì sao?"* — Xem [RES-027](../mock-interview/bank/resume.md).

**Vẽ:** [whiteboard D5](#hình-d5--s-box-nhiều-box--một-bức-tường-một-mức-sáng) — ba box, một master có cảm biến, một đường mạng, một bức tường.

**Móc nền tảng:** chọn IPC ([LNX-017](../mock-interview/bank/linux-sysprog.md)) · dừng thread đang chặn trong `mq_receive` ([LNX-044](../mock-interview/bank/linux-sysprog.md)) · shared memory + khoá ([LNX-045](../mock-interview/bank/linux-sysprog.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-005](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>LNX-044</b> · Thread chặn trong `mq_receive` — dừng thế nào</summary>

**S-Box: một thread nằm chặn trong `mq_receive()` để nhận lệnh đồng bộ độ sáng từ unit khác. Khi service tắt, code đặt `stop = true` rồi `join()` — và treo. Vì sao? Dừng thread đó thế nào?**

> *"Vì `mq_receive()` chặn **trong kernel** cho tới khi có bản tin. Cờ `stop` là một biến trong bộ nhớ, kernel không biết tới nó; thread không bao giờ quay lại vòng lặp để đọc cờ, nên `join()` chờ mãi. Muốn dừng thì phải **đánh thức nó bằng chính thứ nó đang chờ**.*
>
> *Cách em chọn là **bản tin sentinel**: gửi một bản tin *"QUIT"* với **độ ưu tiên cao nhất** — nó vượt lên trước mọi bản tin đang chờ, thread nhận được thì thoát vòng lặp. Đo thật: `join` xong trong khoảng 170 micro giây. Nếu hàng đợi đang đầy thì chính lệnh gửi sentinel bị chặn, nên em gửi bằng `mq_timedsend` hoặc một descriptor mở non-blocking riêng.*
>
> *Cách khác: `mq_timedreceive` thức dậy định kỳ để kiểm cờ — đơn giản nhưng dừng chậm bằng độ dài timeout, và hạn chót của nó tính theo giờ lịch nên dính vụ NTP chỉnh giờ. Nếu đã có event loop thì trên Linux message queue là một file descriptor, chờ chung với một `eventfd` dành cho việc dừng là sạch nhất."*

**Nếu bị hỏi tiếp**
- *"Sao không `pthread_cancel`?"* — Huỷ giữa chừng có thể bỏ qua destructor của object trên stack ⟹ rò tài nguyên, khoá bị giữ.
- *"Tính hạn chót `mq_timedreceive` từ `CLOCK_MONOTONIC`?"* — Sai: kernel hiểu nó là giờ lịch, hạn nằm trong quá khứ, hàm trả `ETIMEDOUT` ngay, vòng lặp quay tít đốt CPU.

**Móc nền tảng:** POSIX message queue ([RES-005](../mock-interview/bank/resume.md)) · clock ([LNX-029](../mock-interview/bank/linux-sysprog.md)) · `epoll` + `eventfd`.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [LNX-044](../mock-interview/bank/linux-sysprog.md)</sub>

</details>

<details><summary><b>DP-044</b> · Vòng cập nhật định kỳ ngoài mặt tiền — hai vấn đề</summary>

**Mọi lời gọi API của library đều đi qua một named semaphore ở mặt tiền C. Ngoài ra library còn một vòng cập nhật định kỳ chạy theo nhịp màn hình: khởi động khi `.so` được nạp, đi thẳng thuật toán → phần cứng, và khi một process khác nạp `.so` thì vòng này reset, chạy lại trong process mới. Nêu hai vấn đề thiết kế của vòng này và cách sửa.**

> *"Ngoài đường gọi API, library còn **một vòng chạy định kỳ theo nhịp màn hình** để độ sáng bám nội dung. Vòng này có hai vấn đề.*
>
> *Một: nó **chạm state ngoài khoá**. Vòng định kỳ và các hàm API cùng đọc ghi một state trong shared memory, nhưng chỉ đường API đi qua khoá ở mặt tiền C. Tức là an toàn của state đang phụ thuộc vào **đường gọi**, không phụ thuộc vào **state**. Em chạy TSan trên bản lab và nó báo đúng data race đó. Sửa: đưa khoá về **đi cùng state** — chỉ lấy được state qua một đối tượng RAII đã giữ khoá; vẫn một semaphore, chỉ đổi chỗ đặt.*
>
> *Hai: vòng này **không có chủ cố định**. Nó khởi động khi `.so` được nạp, nên nó chạy trong process nạp library **sau cùng** — một tool test mở library là kéo vòng đó về phía mình, tool thoát thì vòng đi theo. Sửa: constructor của `.so` không tự khởi động gì; chỉ **một service chủ** sống suốt đời thiết bị mới gọi hàm khởi động vòng đó.*
>
> *Hai vấn đề cùng một gốc: state và vòng chạy đều có **nhiều hơn một chủ**."*

**Nếu bị hỏi tiếp**
- *"Vì sao đổi process mà thuật toán không mất trí nhớ?"* — Vì state nằm trong shared memory; đó là luận cứ cho việc đặt state ở đó.

**Móc nền tảng:** data race · RAII lock guard ([DP-047](../mock-interview/bank/design-patterns.md)) · TSan ([DBG-042](../mock-interview/bank/debugging.md)) · `__attribute__((constructor))` ([DP-041](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-044](../mock-interview/bank/design-patterns.md)</sub>

</details>

*Câu đã có ở buổi trước: `OS-007` · `LNX-045`*


### 📚 Sổ · 6. R4 — Phần còn lại của resume + tiếng Anh

<details><summary><b>RES-008</b> · Debug xuyên tầng — một ca cụ thể</summary>

**Resume ghi em debug xuyên tầng user–kernel. Kể một ca cụ thể: triệu chứng ban đầu là gì, em khoanh vùng ra sao, gốc rễ nằm ở đâu?**

> *"Ca em hay kể là **TV khởi động lên hình chậm** trên model mới. Triệu chứng ở tầng người dùng, nhưng nguyên nhân có thể ở bất kỳ tầng nào: ứng dụng ra lệnh bật màn muộn, library chậm, hay driver trong kernel chưa sẵn sàng.*
>
> *Việc đầu tiên em làm là **một phép đo chia đôi**: đặt mốc thời gian ở hai phía ranh giới — lúc user-space phát lệnh bật màn, và lúc driver của màn được nạp xong trong log kernel. Kết quả: lệnh tới đúng giờ, nhưng **driver chưa nạp xong** — tức là lỗi nằm ở phía kernel, khỏi phải soi ứng dụng.*
>
> *Chia tiếp ở phía kernel: phần code khởi tạo của driver chạy nhanh, nhưng **thời điểm bắt đầu** của nó bị trễ. Log và tải CPU cho thấy từ giây thứ tư có một tiến trình **ưu tiên thời gian thực** chiếm CPU, trong khi các driver nền được nạp **lần lượt từng cái** với ưu tiên thường — nên bị chen ngang. Thời gian nạp từ dưới nửa giây thành 3–4 giây.*
>
> *Em sửa bằng cách nạp song song kết hợp nâng ưu tiên, nhờ team hiệu năng đánh giá tác động lên phần khởi động khác, và nghiệm thu qua **100 lần khởi động**. Để lần sau lộ sớm, mốc thời gian nạp driver được giữ trong log khởi động để so giữa các bản build."*

**Ca thứ hai (ngắn), nếu được hỏi thêm:** *màn bật, đèn nền sáng, nhưng hình **đen hoàn toàn**.* Đo ở ranh giới: lệnh bật đèn đã xuống tới phần cứng và đèn sáng thật — nên không phải lỗi đường bật màn. Gốc rễ là một khối xử lý hình phía trước vẫn đang bật **lớp che màu đen** — lớp dùng để che nhiễu lúc khởi động — vì thiếu lệnh gỡ lớp che trong trình tự bật. Sửa: thêm bước đó vào trình tự bật màn.

**Nếu bị hỏi tiếp**
- *"Ca treo, không crash, không log thì sao?"* — Vẫn là chia đôi: treo ở user hay kernel? `/proc/<pid>/wchan` và stack của thread cho biết nó đang chờ gì trong kernel; `gdb -p` + backtrace mọi thread cho phía user ([DBG-010](../mock-interview/bank/debugging.md)).
- *"Công cụ ở từng tầng?"* — User: log, gdb, core dump, strace. Ranh giới: giá trị trả về của `ioctl`, `errno`, sysfs. Kernel: `dmesg`/`printk` có mốc thời gian, ftrace.

**Móc nền tảng:** chia đôi không gian nghi ngờ ([09/mindset](../../09-debugging/mindset.md)) · debug cross-layer ([DBG-016](../mock-interview/bank/debugging.md)) · scheduling: ưu tiên thường vs thời gian thực ([RES-015](../mock-interview/bank/resume.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-008](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-016</b> · Dòng AI — chặn cách đọc bất lợi</summary>

**"Dòng về AI trong resume — nếu interviewer đọc nó theo hướng bất lợi cho em thì họ nghĩ gì? Em nói thêm câu nào để chặn?"**

> *"Họ có thể nghĩ: *phần nào là em làm, bỏ AI đi thì còn lại gì?* Em trả lời bằng cách **phân vai rõ**.*
>
> *AI giúp em ở phần **lặp lại**: sinh unit test từ scenario em viết, theo bộ khung có sẵn của dự án; đổi tên, thêm khai báo hàm, soát cú pháp khi port code. Phần **của em** là chỗ AI không làm thay được: đọc tài liệu chip, trao đổi với người sở hữu module, device tree, bootloader, quyết định thiết kế, và chẩn đoán — AI không đọc hộ em `dmesg` để thấy một tiến trình ưu tiên cao chen vào ở giây thứ tư khi em tối ưu thời gian nạp driver.*
>
> *Với test do AI sinh, em kiểm ba thứ: test có **kiểm kết quả** không hay chỉ gọi hàm cho chạy qua; **làm hỏng code** thì test có đỏ không; và scenario có **ca lỗi** không. Và một thay đổi chạy trên rất nhiều model nên test xanh vẫn phải qua test trên thiết bị.*
>
> *Còn việc nào cần **đúng y như lần trước** cho hàng chục tổ hợp — như porting — em viết **script**, không dùng AI, vì em cần một phép biến đổi xác định và review được từng dòng."*

**Nếu bị hỏi tiếp**
- *"AI sinh một hàm kernel chạy đúng nhưng em không hiểu hết — merge hay chặn?"* — **Chặn**. Code em không giải thích được thì em không bảo trì được và không debug được lúc nó hỏng ngoài hiện trường; em viết lại hoặc đọc tới khi hiểu rồi mới merge.

**Móc nền tảng:** chất lượng test AI sinh ([BLD-043](../mock-interview/bank/build-systems.md)) · test double ([BLD-040](../mock-interview/bank/build-systems.md)) · porting tool ([RES-009](../mock-interview/bank/resume.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-016](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-017</b> · Preset — Cancel quay về trạng thái cũ thế nào</summary>

**"Khi người dùng đang sửa một Preset rồi bấm Cancel, em quay lại trạng thái cũ bằng cách nào? Vì sao không cho sửa trên một bản nháp trong bộ nhớ cho an toàn?"**

> *"Phương án đầu tiên em làm là **bản nháp**: sửa vào một bản sao trong bộ nhớ, bấm Save mới ghi xuống màn hình. Nghe an toàn, nhưng em bỏ vì hai lý do khác loại nhau. Về **trải nghiệm**: người dùng kéo thanh độ sáng mà màn **không sáng lên**, tức là đang chỉnh mù — với app mà mục đích là nhìn thấy kết quả thì không chấp nhận được. Về **code**: mọi hàm đọc ghi setting phải rẽ nhánh *"đang sửa hay không"*, rải khắp nơi, rất dễ sót một chỗ.*
>
> *Phương án em chốt: **sửa thẳng lên màn hình thật**, nhưng trước khi vào chế độ sửa thì **chụp lại** cấu hình đang dùng; bấm Cancel thì ghi bản chụp đó trở lại.*
>
> *Cái giá em nhận: trạng thái cần khôi phục giờ nằm **trên phần cứng**, nên **mọi đường thoát** khỏi chế độ sửa phải được kiểm soát — Cancel, đóng app, rút cáp, chuyển sang màn khác. Em xử lý bằng cờ trạng thái; đúng nhưng dễ quên ở chỗ thêm mới. Sau này team chuyển sang **state machine**, và em thấy đó là lời giải tốt hơn: thêm một đường thoát mới thì bảng trạng thái buộc mình trả lời nó phải làm gì."*

**Nếu bị hỏi tiếp**
- *"Vì sao cờ không đủ?"* — Cờ đúng ở lúc viết, hỏng ở lúc người sau thêm đường thoát mới mà không biết phải xử lý cờ.

**Móc nền tảng:** snapshot/rollback (giống bản A/B khi cập nhật firmware) · State pattern · Memento ([DP-032](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-017](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>DP-032</b> · Memento sách vở thiếu gì cho *"export sang màn khác"*</summary>

**Feature "Preset" (lưu cấu hình màn hình, khôi phục sau restart, export sang màn hình khác) về bản chất là Memento. Memento sách vở thiếu ba thứ gì cho yêu cầu "export sang màn hình khác"?**

> *"Memento trong sách sống trong **một process, một phiên chạy** — để undo/redo: object tự chụp, tự khôi phục, người giữ bản chụp không đọc bên trong. Export sang màn hình khác sinh thêm ba việc pattern không nói tới.*
>
> *Một, **phiên bản**: preset lưu bằng app 1.0, mở bằng app 1.2. Phải ghi số phiên bản vào file, chỉ thêm trường mới chứ không đổi nghĩa trường cũ, trường thiếu thì lấy mặc định.*
>
> *Hai, **khả năng của màn đích**: màn kia không hỗ trợ một setting, hoặc dải giá trị khác. Em áp từng phần và **báo lại** những setting đã bỏ qua, thay vì thất bại cả gói hay lặng lẽ bỏ.*
>
> *Ba, **tính nguyên tử**: áp mười setting, cái thứ bảy hỏng — dừng và khôi phục lại, hay đi tiếp và báo cáo? Không có đáp án duy nhất; điều cần là **một quyết định rõ ràng và nhất quán**.*
>
> *Tóm lại Memento cho mình **đóng gói** — người giữ không cần biết bên trong; nó không cho **tính khả chuyển** — phần đó là phiên bản, khả năng và chính sách lỗi."*

**Nếu bị hỏi tiếp**
- *"Ghi file thế nào để mất điện giữa chừng không hỏng?"* — Ghi ra file tạm, flush, rồi `rename` đè lên file cũ — reader chỉ thấy bản cũ hoặc bản mới hoàn chỉnh ([RES-020](../mock-interview/bank/resume.md)).

**Móc nền tảng:** versioning (cùng luật ABI: chỉ thêm ở cuối) · Null Object / `-ENOTSUP` cho setting không hỗ trợ.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DP-032](../mock-interview/bank/design-patterns.md)</sub>

</details>

<details><summary><b>RES-021</b> · Save/Load khác thread — một `std::mutex` member đã đủ chưa</summary>

**"Save và Load có chạy khác thread không? Nếu sau này có, em bảo vệ thế nào — đặt một `std::mutex` làm member của class quản lý file là đủ chưa?"**

> *"Ở bản 1.0, Save và Load **không** chạy khác thread, và em không đặt khoá. Lý do không phải *"chưa gặp lỗi"* mà là **thiết kế**: mọi thao tác preset đi qua luồng giao diện, không có đường nào chạy song song. Đó là giả định em đang dựa vào, và em ghi nó ra.*
>
> *Nếu sau này giả định vỡ — có auto-save chạy nền, hay hai cửa sổ — thì một `std::mutex` làm member **chưa đủ**. Mutex member chỉ tuần tự hoá những lời gọi **đi qua đúng object đó**. Thứ cần bảo vệ là **cái file**, nằm ngoài object: hai object khác nhau có hai mutex khác nhau, và hai process — chạy app hai lần — thì mutex trong bộ nhớ không thấy nhau.*
>
> *Em chọn khoá theo **phạm vi của tài nguyên**: trong một object thì mutex member; toàn process thì một chủ sở hữu duy nhất giữ khoá; liên process thì khoá của hệ điều hành trên file. Và rẻ nhất là làm cho việc ghi **nguyên tử**: ghi ra file tạm rồi `rename` đè lên — người đọc không bao giờ thấy file dở dang, nên nhu cầu khoá giảm hẳn."*

**Nếu bị hỏi tiếp**
- *"Liên hệ với library Linux của em?"* — Cùng bài học: khoá phải cùng phạm vi với tài nguyên. State library nằm trong shared memory dùng chung mọi process, nên khoá là semaphore dùng chung, không phải mutex trong một object ([LNX-046](../mock-interview/bank/linux-sysprog.md)).

**Móc nền tảng:** phạm vi của khoá · `flock`/`fcntl` · ghi nguyên tử bằng `rename`.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-021](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-009</b> · Porting tool — con số đo thế nào</summary>

**Em ghi tool porting giảm hơn 50% công sức thủ công. Con số đó đo bằng cách nào?**

> *"Bài toán: mỗi năm có chip mới, phải port driver và library sang — phần lớn là việc **lặp lại**: chép source gốc, đổi tên theo chip mới, thêm nhánh cho chip mới vào các chỗ rẽ nhánh, sửa CMake, Makefile và cấu hình build. Nhiều file, hàng chục nghìn dòng, rất dễ sót.*
>
> *Tool Python của em làm phần lặp lại đó. Và em **cố ý để lại cho con người** những gì phụ thuộc phần cứng: địa chỉ thanh ghi, bus, device tree, phần porting kernel, và việc tạo tổ hợp chip với tấm màn.*
>
> *Con số **1–2 ngày xuống 2–3 giờ** là cho **phần thủ công của một lần port**, không phải cả dự án, và em nói thẳng đó là **ước lượng** của em qua các lần làm cả hai cách, không phải số đo chính thức.*
>
> *Kiểm tra bản port: review và build; ra đúng file driver cho tổ hợp mới là bước đầu; sau đó test trên model mới — khởi động được và nạp đúng cài đặt. Nếu làm tiếp, em cho tool **tự kiểm** ngay sau khi sinh: tên chip cũ phải biến mất hết, mọi tổ hợp khai báo đều có phần tương ứng; và chạy tool trên một chip đã port tay rồi so khác biệt."*

**Nếu bị hỏi tiếp**
- *"Sao không để AI làm?"* — Việc này phải chạy **y hệt** cho hàng chục tổ hợp và phải **review được từng dòng**; cần phép biến đổi xác định nên là script. AI hợp khi cần phán đoán.
- *"Ước lượng thì sao dám ghi vào resume?"* — Vì phạm vi đã ghi hẹp (*"manual effort"*) và em nói rõ cách ước lượng khi được hỏi; em không trình bày nó như một phép đo.

**Móc nền tảng:** CMake/Makefile · build nhiều tổ hợp trong CI ([BLD-021](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-009](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-015</b> · Load-time: chọn song song hay priority, rủi ro phương án bị loại</summary>

**"Em thử hai giải pháp: nạp song song và nâng scheduling priority. Cuối cùng chọn cái nào, vì sao, và rủi ro của phương án bị loại?"**

> *"Trước hết là chẩn đoán: thời gian nạp driver tăng từ dưới nửa giây lên 3–4 giây trên model mới. Log kernel cho thấy đoạn chậm không phải code của driver; cùng lúc đó CPU ở 100% vì một tiến trình **ưu tiên thời gian thực** của hệ thống chạy từ giây thứ tư, trong khi driver nền được nạp **lần lượt từng cái** với ưu tiên thường — nên bị chen ngang.*
>
> *Hai phương án sửa **hai nửa khác nhau**: nạp **song song** bỏ được phần tuần tự nhưng vẫn ưu tiên thường nên vẫn bị chen; **nâng ưu tiên** thì thắng được tranh chấp nhưng vẫn nạp lần lượt. Em đo từng cái: riêng lẻ chỉ đạt khoảng **7 trên 10** lần khởi động. Nên em **kết hợp cả hai**.*
>
> *Trước khi áp, em nhờ team hiệu năng đánh giá, vì song song cộng ưu tiên thời gian thực có thể thành điểm nghẽn cho phần khởi động khác. Nghiệm thu bằng **100 lần khởi động**, tất cả dưới nửa giây.*
>
> *Rủi ro của từng phương án: song song thì thứ tự phụ thuộc giữa driver không còn được bảo đảm — driver cần cái chưa sẵn sàng sẽ phải hoãn; ưu tiên thời gian thực thì task đó không bị chen, nếu nó bận lâu hoặc kẹt sẽ bỏ đói phần còn lại."*

**Nếu bị hỏi tiếp**
- *"Phát hiện bị chen ngang bằng cách nào?"* — Đặt mốc thời gian trong log kernel, rồi xem tải CPU và các tiến trình chạy ở đúng khoảng đó ([RES-014](../mock-interview/bank/resume.md)).
- *"Vì sao 100 lần là đủ?"* — Lỗi xuất hiện khoảng 3/10 lần; 100 lần sạch thì xác suất lỗi còn sót là rất nhỏ ([RES-029](../mock-interview/bank/resume.md)).

**Móc nền tảng:** scheduling: ưu tiên thường vs thời gian thực ([scheduling](../../03-operating-system/scheduling.md)) · priority inversion ([OS-007](../mock-interview/bank/os.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-015](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-004</b> · Migration 5.10 → 6.12 — cái gì vỡ</summary>

**Em có làm kernel migration 5.10 lên 6.12. Cái gì vỡ? Em phát hiện và xử lý thế nào?**

> *"Em làm migration từ 5.10 lên 6.12 cho mảng driver hiển thị của em, với yêu cầu **vẫn chạy trên nhánh 5.10** cho sản phẩm đang bán.*
>
> *Cái vỡ em chia bốn nhóm. **Đổi chữ ký hàm**: một số hàm đăng ký và tạo thiết bị đổi tham số — compiler báo hết, sửa dần. **Hàm bị xoá**: có API cũ bị gỡ hẳn, phải tìm cách thay. **Cảnh báo bị siết**: nhiều thứ trước chỉ là warning giờ thành lỗi build. Và nhóm khó nhất là **đổi hành vi ngầm** — thứ tự khởi tạo, thời điểm, mặc định của subsystem — compiler không báo gì, chỉ lộ lúc chạy, nên phải so log khởi động hai kernel và đọc lịch sử thay đổi của subsystem đó.*
>
> *Tương thích ngược em làm bằng **một source duy nhất** build được cho cả hai kernel: kiểm phiên bản bằng `LINUX_VERSION_CODE` so với `KERNEL_VERSION(...)`, tách nhánh cài đặt ở đúng chỗ khác nhau, phần chung giữ nguyên. Cái giá là nhánh nào cũng phải test riêng, và `#if` càng nhiều code càng khó đọc — nên chỉ hợp khi khác biệt ít và có hạn."*

**Nếu bị hỏi tiếp**
- *"Vì sao kernel không giữ ABI cho driver như glibc giữ cho app?"* — Cố ý: để kernel tự do sửa nội bộ, và khuyến khích driver vào mainline; module ngoài cây phải theo từng phiên bản.
- *"Khi nào nên tách hai nhánh thay vì `#if`?"* — Khi khác biệt lan ra kiến trúc chứ không còn là vài API.

**Móc nền tảng:** kernel module · device tree · kernel không có ABI ổn định cho module.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-004](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-032</b> · Opening bằng tiếng Anh</summary>

🇬🇧 **"Could you introduce yourself and walk me through what you're working on?"** — trả lời bằng tiếng Anh, ≤ 90 giây.

> *"I'm a software engineer at Samsung — three years building system software for TVs and enterprise display products on ARM SoCs.*
>
> *My work spans the full stack: from the **C++ interface** our shared library exposes, down to the **kernel drivers** talking to the hardware.*
>
> *Three things I'd highlight. First, **one C++ interface across many chipsets**: the library picks its implementation at boot from the board configuration, and the kernel driver dispatches through a function-pointer table, so applications above never change. Second, I **cut driver load time from three-to-four seconds to under half a second** — I measured two options and verified the fix over a hundred boot cycles. Third, I **port drivers to new chip generations** every year, including a kernel migration to 6.12 while keeping the older branch working.*
>
> *I'd like to go deeper on the system side and work in a more structured product process. **Which of those would you like me to go into?**"*

**If they follow up in English**
- *"What exactly is your part in the library?"* — *"The brightness algorithm and its path from the C++ interface down to the driver, plus porting and bring-up on new chips."*
- Tránh: *"I developed"* cho việc đang làm (dùng *"I develop / I've been developing"*); tránh kết bằng câu có/không.

**Móc nền tảng:** cùng ba móc với [RES-001](../mock-interview/bank/resume.md) Bản C.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-032](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-027</b> · Một binary hai mode — ai là master, master mất điện thì sao</summary>

**"Một binary hai mode: nó biết mình ở mode nào bằng cách nào, ai là master? Hai unit cùng bật adaptive thì sao? Master mất điện thì sao?"**

> *"Binary biết mình ở chế độ nào nhờ một **cờ cấu hình** do ứng dụng gửi xuống lúc chạy — chính là mục bật tính năng đồng bộ trên thiết bị. Máy nào bật độ sáng tự động thì thành **master**: nó đo, tính ra mức chuẩn và **đẩy** xuống các máy còn lại. Máy con không tự tính, chỉ áp giá trị nhận được.*
>
> *Vì luồng điều khiển là **đẩy từ master** chứ không phải mỗi máy tự kéo, nên *"hai master"* không phải là một chế độ hỏng thật: ứng dụng ở máy con không đọc hàng đợi của chính nó.*
>
> *Master mất điện: bọn em **chủ động chấp nhận không có failover**. Bức tường là thiết bị cố định, có người vận hành; mất master thì cả bức tường đứng ở mức sáng cuối cùng — sai lệch nhìn thấy được nhưng không nguy hiểm. Làm bầu master tự động thì phải thêm heartbeat và xử lý trường hợp hai bên cùng tưởng mình là master — chi phí không tương xứng với lợi ích ở sản phẩm này."*

**Nếu bị hỏi tiếp**
- *"Muốn có failover thì làm gì?"* — Heartbeat định kỳ từ master; mất heartbeat quá N giây thì máy con quay về tự chỉnh theo cảm biến của mình, hoặc một máy được chỉ định trước lên thay.

**Vẽ:** [whiteboard D5](#hình-d5--s-box-nhiều-box--một-bức-tường-một-mức-sáng) — ba box, một master có cảm biến, một đường mạng, một bức tường.

**Móc nền tảng:** POSIX message queue ([RES-005](../mock-interview/bank/resume.md)) · thiết kế chế độ hỏng · split-brain.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-027](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-031</b> · Requirement đến tay ở dạng chưa dùng được</summary>

**"Kể một lần requirement đến tay em ở dạng chưa dùng được. Nó hổng ở đâu, em tìm ra bằng phương pháp gì?"**

> *"Ở tính năng độ sáng tự động của S-Box, requirement ban đầu chỉ có **đường thành công**: bật tính năng, đọc cảm biến, tính giá trị, gửi đi — hết.*
>
> *Em không đợi lúc code mới gặp, mà **kẻ bảng trạng thái nhân sự kiện**: hàng là trạng thái — tắt, đang chuyển, đã ổn định; cột là sự kiện — bật, tắt, người dùng chỉnh tay, khởi động lại, mất cảm biến. Mỗi ô buộc phải có câu trả lời; **ô trống là một câu hỏi gửi lại bên yêu cầu**.*
>
> *Bảng tự sinh ra ba câu hỏi requirement bỏ trống: đang tắt mà bật thì nhảy ngay hay chuyển dần; tắt đi thì giữ mức hiện tại hay quay về mức trước khi bật; và bật rồi khởi động lại thì khôi phục thế nào. Nó còn chỉ ra hai ô mà cách *"gặp đâu xử lý đó"* không tìm ra: đang chuyển dở thì người dùng chỉnh tay, và cảm biến hỏng giữa chừng — ô này sản phẩm chốt là **giữ độ sáng ở mức mặc định**.*
>
> *Em mang bảng đó đi chốt lại với người viết spec, rồi mới code."*

**Nếu bị hỏi tiếp**
- *"Sao không hỏi lúc gặp?"* — Gặp lúc code thì đã muộn: quyết định bị đẩy cho người code đưa ra ngầm, và QA tìm thấy ở cuối.

**Móc nền tảng:** state machine · thiết kế theo chế độ hỏng ([RES-027](../mock-interview/bank/resume.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-031](../mock-interview/bank/resume.md)</sub>

</details>

<details><summary><b>RES-033</b> · *"Why do you need two levels of function-pointer table?"*</summary>

🇬🇧 **"You mentioned the kernel driver dispatches through a function-pointer table. Why do you need two levels of that? Walk me through it."**

> *"There are two levels because they solve **two different problems** — one is a **license boundary**, the other is **polymorphism**.*
>
> *The first level sits between the platform's core driver, which is GPL, and the proprietary part. The proprietary side can't use GPL-only kernel symbols directly, so the two talk through a table of function pointers instead of linking to each other.*
>
> *The second level is about chips. The core driver doesn't know which chip is on the board. At boot the system figures out the chip and loads its driver, and that chip-specific driver **registers its functions into the table**. From then on every call goes through the table and lands in the right chip. It's the same idea as a C++ vtable — just written by hand in C.*
>
> *So: one level for the license boundary, one level for polymorphism."*

**If they follow up**
- *"What can go wrong with a hand-written table?"* — *"Two things a compiler would protect you from: an empty slot — so we check for NULL and return 'not supported' — and inserting a function in the middle, which shifts every slot after it. We only append at the end and check a size field at registration."*

**Móc nền tảng:** [RES-023](../mock-interview/bank/resume.md) · [DP-043](../mock-interview/bank/design-patterns.md).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [RES-033](../mock-interview/bank/resume.md)</sub>

</details>


### 📚 Sổ · 12. R5 — CI · unit test · code quality

<details><summary><b>BLD-021</b> · Vì sao không có đường submit tay vào nhánh chính</summary>

**Dự án bắt buộc mọi thay đổi phải qua pipeline mới vào được source chính, không có đường submit tay. Mô hình này tên gì, được lợi gì, và cái giá là gì?**

> *"Mô hình đó gọi là **gated check-in** — kiểm tra trước khi gộp. Ở chỗ em, em là **người dùng** pipeline: lấy code, đưa lên review chéo trên Perforce, submit; thay đổi đi vào build tích hợp và hệ thống CI/CD nội bộ; chỉ khi qua hết mới vào nhánh chính. Không có đường nào khác.*
>
> *Được: **nhánh chính gần như không bao giờ hỏng**. Với nhiều nền tảng và nhiều team, nhánh chính hỏng không phải là một người build lỗi, mà là **cả chục kỹ sư bị chặn**, cộng thời gian đi tìm ai gây ra — người gây hỏng thường không phải người phát hiện.*
>
> *Cái giá: một là **chờ** — mỗi thay đổi phải đợi trọn pipeline, và hàng đợi giới hạn số thay đổi vào được mỗi ngày. Hai là khi gom nhiều thay đổi test một lượt cho nhanh, hỏng thì **không biết cái nào** gây ra, phải chia đôi lô để tìm. Ba là bị chặn oan khi lỗi là của người khác trong cùng lô. Và nếu cổng chậm quá, người ta sẽ đòi đường tắt cho *"bản gấp"* — lúc đó cổng bắt đầu mất tác dụng."*

**Nếu bị hỏi tiếp**
- *"Đối lập với gated là gì?"* — Gộp trước, hỏng thì revert. Hợp khi ít nền tảng và revert rẻ.

**Móc nền tảng:** flaky test trên cổng chặn ([BLD-026](../mock-interview/bank/build-systems.md)) · các tầng test ([BLD-033](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-021](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-031</b> · CI / Continuous Delivery / Continuous Deployment</summary>

**Phân biệt CI, Continuous Delivery và Continuous Deployment. Ở embedded thường dừng ở đâu, vì sao?**

> *"**CI** — tích hợp liên tục: mỗi thay đổi được gộp, build và test tự động. **Continuous Delivery**: thêm bước đóng gói thành một bản phát hành **sẵn sàng**, nhưng **người** bấm nút phát hành. **Continuous Deployment**: tự đưa ra môi trường thật luôn, không ai bấm.*
>
> *Ở embedded, *"đưa ra thật"* nghĩa là cập nhật firmware xuống **thiết bị của khách**. Web có lỗi thì đổi lại bản cũ trong vài giây; còn thiết bị mất điện giữa lúc cập nhật có thể thành **cục gạch** và phải thu hồi. Nên embedded gần như luôn dừng ở **Continuous Delivery**: máy lo tới mức *"đã build, đã ký, đã test, sẵn sàng"*, còn phát hành là quyết định của người, thường theo từng đợt nhỏ trước.*
>
> *Phần em chạm hằng ngày là CI: build tích hợp và các cổng kiểm tra trước khi vào nhánh chính."*

**Nếu bị hỏi tiếp**
- *"Phát hành theo đợt là gì?"* — Đưa cho một nhóm nhỏ thiết bị trước, theo dõi, ổn rồi mới mở rộng.

**Móc nền tảng:** gated check-in ([BLD-021](../mock-interview/bank/build-systems.md)) · cập nhật A/B và rollback.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-031](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-032</b> · Thuật ngữ pipeline: stage, job, artifact, trigger, gate</summary>

**Giải thích các thuật ngữ pipeline: stage, job, runner, artifact, trigger, gate, DUT, HIL — và chúng là cái gì trong ngữ cảnh embedded?**

> *"**Pipeline** là chuỗi bước tự động chạy sau một sự kiện — với em là: submit, build cho các nền tảng, rồi test. **Trigger** là cái làm nó chạy: một lần submit, lịch hằng đêm, hay người gọi tay. **Stage** là một nhóm bước, xong nhóm này mới sang nhóm sau — build, rồi test nhanh, rồi test dài. **Job** là một việc chạy độc lập, chạy song song được — ví dụ *"build cho nền tảng A"*. **Runner** hay agent là **máy** chạy job — máy build thường là x86, không chạy được binary ARM. **Artifact** là thứ giữ lại sau job: image, kernel, gói có symbol. **Gate** là điều kiện phải qua mới đi tiếp. **DUT** là thiết bị đang được test; **HIL** là test có phần cứng thật trong vòng lặp.*
>
> *Ở embedded em nhấn vào **artifact**: thiết bị bán ra sống năm đến mười năm. Hai năm sau có crash ngoài hiện trường thì cần **đúng image đó** và **đúng file symbol** của bản build đó để đọc backtrace. Không lưu artifact thì không điều tra được."*

**Nếu bị hỏi tiếp**
- *"Jenkinsfile là gì?"* — Mô tả pipeline dưới dạng code, nằm trong repo. Em chưa viết ở công ty; ở lab em giữ nó là vỏ mỏng, mỗi stage chỉ gọi một script trong repo để chạy lại y hệt trên máy mình ([unit-test-and-code-quality §2](../../06-build-systems/unit-test-and-code-quality.md)).

**Móc nền tảng:** symbol và crash ([DBG-045](../mock-interview/bank/debugging.md)) · test trên thiết bị thật ([BLD-035](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-032](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-033</b> · Các tầng test (unit/integration/system) và loại test (smoke/regression/soak)</summary>

**Unit / integration / system test và smoke / regression / soak test — quan hệ giữa hai bộ khái niệm này là gì?**

> *"Đó là **hai trục khác nhau**, không phải hai mức của cùng một thang.*
>
> *Trục **phạm vi** trả lời *"test cái gì"*: **unit** là một hàm, một class, tách khỏi phụ thuộc; **integration** là vài phần ghép lại — ví dụ app thật gọi qua `.so` thật; **system** là cả thiết bị.*
>
> *Trục **mục đích** trả lời *"test để biết gì"*: **smoke** — còn sống không, khởi động được và chức năng chính chạy không; **regression** — thứ trước đây đúng có còn đúng sau thay đổi; **soak** hay stress — chạy lâu, chạy nặng có rò rỉ hay treo không.*
>
> *Mỗi bài test có một vị trí trên cả hai trục: smoke thường ở mức system; regression có thể là unit hoặc system; soak gần như luôn là system. Hỏi *"smoke là unit hay system"* là hỏi nhầm trục.*
>
> *Ở chỗ em: unit test cho library và driver chạy trên máy build; sau đó test trên thiết bị, vì một thay đổi chạy trên rất nhiều model."*

**Nếu bị hỏi tiếp**
- *"Tầng nào nhiều test nhất?"* — Tháp test: nhiều unit test rẻ ở đáy, ít test hệ thống đắt ở đỉnh; cổng rẻ chạy trước.

**Móc nền tảng:** GoogleTest/KUnit ([BLD-044](../mock-interview/bank/build-systems.md)) · test trên thiết bị ([BLD-035](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-033](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-044</b> · GoogleTest vs KUnit</summary>

**GoogleTest ở library và KUnit ở driver — mỗi cái test cái gì, chạy ở đâu?**

> *"**GoogleTest** dành cho library C++: test là một chương trình user-space bình thường chạy trên máy build, gọi thẳng các class và hàm của library, và thay phần phụ thuộc phần cứng bằng bản giả. Ví dụ em test thuật toán độ sáng bằng một backend giả ghi lại các giá trị được yêu cầu ghi.*
>
> *Ví dụ test của library:*
> ```cpp
> TEST(GlobalDimming, VSyncRampsInStepsOf10) {
>     GlobalDimmingForShm shm{};  FakeBackend be;
>     GlobalDimming algo(&be, shm);
>     algo.SetBacklight(25);
>     for (int i = 0; i < 5; ++i) algo.t_vSyncCallBack();
>     EXPECT_EQ(be.writes, (std::vector<int32_t>{10, 20, 25}));
> }
> ```
>
> *Code trong kernel không chạy được như một chương trình thường, nên driver dùng **KUnit** — framework test **bên trong kernel**. Test được build cùng kernel hoặc thành module và thường chạy trên một kernel giả lập, không cần board. KUnit test được **logic** của driver — kiểm tham số, bảng hàm, máy trạng thái — **không** test được phần cứng thật.*
>
> *Cả hai đều không thay được test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Ai viết test ở chỗ em?"* — Em viết scenario, AI sinh test theo bộ khung có sẵn, em kiểm lại ([BLD-043](../mock-interview/bank/build-systems.md)).

**Móc nền tảng:** test double ([BLD-040](../mock-interview/bank/build-systems.md)) · Bridge tạo chỗ cắm cho test ([DP-038](../mock-interview/bank/design-patterns.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-044](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-040</b> · Test thuật toán không có phần cứng — fake/mock/stub</summary>

**Bạn muốn unit test thuật toán dimming (tính độ sáng theo nội dung hình) trên máy build, không có panel, không có chip. Làm thế nào? Fake, mock, stub khác nhau ở đâu?**

> *"Muốn test một phần mà không có phần cứng thì phần đó phải **gọi phần cứng qua một interface**. Thuật toán độ sáng của em giữ con trỏ tới `IDimmingBackend` — interface ghi xuống phần cứng — nên trong test em cắm vào một **backend giả** chỉ ghi lại các giá trị thuật toán yêu cầu ghi, rồi kiểm danh sách đó.*
>
> *Ba loại bản giả: **stub** chỉ trả một giá trị cố định để code chạy qua; **fake** là một cài đặt đơn giản nhưng chạy thật — như backend ghi vào vector — dùng khi muốn kiểm **kết quả**; **mock** kiểm **lời gọi** — được gọi mấy lần, với tham số gì — dùng khi muốn kiểm **tương tác**.*
>
> *Điểm em muốn nhấn: **test được hay không là hệ quả của thiết kế**. Nếu thuật toán gọi thẳng `ioctl` thì không test được khi không có thiết bị. Tách thuật toán khỏi phần ghi phần cứng là thứ tạo ra chỗ cắm.*
>
> *Hai bẫy: mock mọi thứ thì test chỉ kiểm lại cách cài đặt — đổi code là vỡ test dù hành vi vẫn đúng; và fake chạy đúng **không** chứng minh backend thật đúng — vẫn cần test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Coverage sau khi dùng fake?"* — Backend thật sẽ không được chạy trong unit test; coverage chỉ ra đúng chỗ đó ([BLD-041](../mock-interview/bank/build-systems.md)).

**Móc nền tảng:** Bridge / dependency inversion ([DP-038](../mock-interview/bank/design-patterns.md)) · GoogleTest ([BLD-044](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-040](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-043</b> · AI sinh unit test — kiểm chất lượng test thế nào</summary>

**"Mỗi lần submit em đưa scenario cho AI, AI sinh unit test theo bộ khung có sẵn." — Vậy em kiểm chất lượng của test đó thế nào? Test xanh có nghĩa là gì?**

> *"AI viết nhanh, nhưng quyết định test **đủ hay chưa** vẫn là của em. Em kiểm ba thứ.*
>
> *Một, test có **kiểm kết quả** không: em đọc các dòng `EXPECT` — chúng kiểm kết quả mong đợi của scenario, hay chỉ gọi hàm cho chạy qua? Test không có kiểm tra vẫn xanh, và vẫn làm coverage tăng.*
>
> *Hai, **làm hỏng code thì test có đỏ không**: em cố ý đổi một điều kiện, ví dụ `>` thành `>=`, rồi chạy lại. Vẫn xanh thì test không bảo vệ gì.*
>
> *Ba, scenario có **ca lỗi** không: AI viết theo scenario em đưa; scenario chỉ có đường thành công thì test chỉ có đường thành công. Tham số sai, phần cứng trả lỗi — đó là trách nhiệm của người viết scenario, tức là em.*
>
> *Còn *"test xanh"* chỉ có nghĩa là code làm đúng những gì test kiểm. Một thay đổi của em chạy trên rất nhiều model, nên unit test xanh vẫn phải qua test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Làm hỏng code để kiểm test có tên gì?"* — Mutation testing; ở lab em thử bằng tay — đổi `zones[3]` thành `zones[4]` và xem cổng nào bắt được ([unit-test-and-code-quality §6](../../06-build-systems/unit-test-and-code-quality.md)).

**Móc nền tảng:** dòng AI trên resume ([RES-016](../mock-interview/bank/resume.md)) · coverage ([BLD-041](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-043](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-041</b> · Coverage 90% nghĩa là gì</summary>

**Báo cáo coverage của library ghi 90% line coverage. Con số đó nói lên điều gì, KHÔNG nói lên điều gì? Bạn đọc báo cáo thế nào?**

> *"Line coverage 90% nghĩa là **90% số dòng đã chạy ít nhất một lần** trong lúc test. Nó cho biết **dòng nào chưa từng chạy** — tức chỗ chắc chắn chưa được test. Nó **không** nói những dòng đã chạy là đúng: một test không có kiểm tra nào vẫn làm coverage tăng.*
>
> *Nên em đọc cột *"dòng chưa chạy"*, không đọc con số tổng. Ở lab em đo được 89%, nghe tốt, nhưng mấy dòng chưa chạy lại chính là **backend thật** ghi xuống phần cứng: unit test dùng backend giả, còn integration test chạy model khác. Con đường thật *"thuật toán → backend → phần cứng"* chưa có test nào đi qua.*
>
> *Em cũng nói thẳng: ở công ty em không đo coverage; em hiểu nó đo gì và đã thử bằng gcov/gcovr ở lab. Em không đặt chỉ tiêu cứng như *"phải trên 80%"*, vì nó dễ sinh ra test chỉ để chạy qua code."*

**Nếu bị hỏi tiếp**
- *"Line khác branch coverage?"* — Line: dòng đã chạy; branch: mỗi nhánh `if`/`switch` đã đi qua chưa — một dòng `if` chạy rồi nhưng có thể mới đi một nhánh.

**Móc nền tảng:** chất lượng test ([BLD-043](../mock-interview/bank/build-systems.md)) · `--coverage` / gcov.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-041](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-042</b> · `-Werror`, static analysis, sanitizer, unit test — mỗi cái bắt gì</summary>

**`-Werror`, static analysis, sanitizer, unit test, code review — mỗi cái bắt loại lỗi nào? Vì sao không chọn một cái là đủ?**

> *"Mỗi công cụ bắt một **loại** lỗi và có điểm mù riêng, nên phải chồng lên nhau. **Review** bắt logic, thiết kế, đặt tên — thứ máy không hiểu. **`-Wall -Wextra -Werror`** bắt code đáng ngờ mà compiler nhìn thấy được lúc build. **Phân tích tĩnh** bắt mẫu lỗi qua luồng code — rò rỉ, dereference `NULL` — nhưng hay báo nhầm. **ASan/UBSan** bắt lỗi bộ nhớ và hành vi không xác định **lúc chạy**, chỉ trên đường code đã chạy. **TSan** bắt data race đã xảy ra khi chạy. **Unit test** bắt hành vi sai so với mong đợi — chỉ những gì test đã nghĩ tới.*
>
> *Em có một bằng chứng tự đo ở lab: cố ý đọc phần tử thứ năm của một mảng bốn phần tử. Build Debug thì **im lặng**, chạy vẫn ra kết quả; build Release có tối ưu cộng `-Werror` thì compiler báo vượt biên và build hỏng; ASan thì bắt lúc chạy, chỉ đúng dòng. Và TSan trên cùng library bắt được một data race mà không công cụ nào khác thấy.*
>
> *Thứ tự em xếp theo **chi phí**: rẻ thì chạy mọi lần — warning, unit test; đắt thì chạy sau hoặc định kỳ — sanitizer, test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Coding standard ở chỗ em?"* — Không có bộ quy tắc riêng: library theo Modern C++, driver theo kernel coding style; kernel có `checkpatch.pl` để kiểm tự động.

**Móc nền tảng:** chọn sanitizer ([DBG-042](../mock-interview/bank/debugging.md)) · `-Warray-bounds` chỉ bật khi có tối ưu.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-042](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-035</b> · Vì sao vẫn phải test trên thiết bị thật</summary>

**Vì sao embedded bắt buộc phải test trên board thật? Test trên host và QEMU không đủ ở chỗ nào — và nên chia việc thế nào?**

> *"Vì phần mềm embedded chỉ đúng khi đứng **cùng phần cứng** của nó. Có những lớp lỗi không thể lộ trên máy build: thời gian thật, ngắt thật, cảm biến và tấm màn thật, nhiệt độ, nguồn điện.*
>
> *Test trên máy build — unit test, sanitizer — bắt được logic thuần và lỗi bộ nhớ, rẻ và nhanh. Máy giả lập chạy được khởi động và phần lớn user-space, nhưng không có thời gian thật và ngoại vi thật. Chỉ thiết bị thật cho **hành vi thật và số đo thật** — đổi lại chậm, đắt, và thiết bị có thể hỏng.*
>
> *Nên em chia việc: **đẩy càng nhiều test xuống máy build càng tốt**, để dành thiết bị cho thứ chỉ phần cứng mới trả lời được. Thiết bị là tài nguyên đắt nhất trong pipeline; dùng nó cho việc máy build làm được là lãng phí, và còn làm hàng đợi dài thêm.*
>
> *Ở chỗ em, một thay đổi trong library chạy trên rất nhiều model, nên sau unit test luôn có bước test trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Chỉ vì ARM khác x86 thôi à?"* — Không; giả lập giải được chuyện kiến trúc CPU mà vẫn không thay được board.

**Móc nền tảng:** tháp test ([BLD-033](../mock-interview/bank/build-systems.md)) · gated check-in ([BLD-021](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-035](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-026</b> · Flaky test trên cổng gated nguy hiểm thế nào</summary>

**Flaky test là gì, và vì sao trên một cổng chặn (gated) nó nguy hiểm hơn hẳn trên CI thường? Xử lý thế nào?**

> *"Flaky test là test cho kết quả khác nhau trên **cùng một code** — lúc xanh lúc đỏ.*
>
> *Trên cổng chặn, nó nguy hiểm không phải vì chặn nhầm, mà vì nó làm cổng **ngừng chặn**. Chuỗi diễn ra thế này: test hỏng ngẫu nhiên chặn oan người vô tội; mọi người học được *"cứ bấm chạy lại"*; chạy lại thành phản xạ với **mọi** lỗi; đến khi có lỗi thật, nó cũng được chạy lại cho qua. Và nó cộng dồn rất nhanh: 200 test, mỗi cái chỉ 0,5% flaky, thì khoảng 63% số lần chạy có ít nhất một lỗi oan.*
>
> *Nguồn flaky đặc trưng ở embedded: dùng `sleep` cố định thay vì chờ một điều kiện; trạng thái còn sót từ job trước; một board trong dàn test hỏng dần; cáp lỏng, ánh sáng phòng.*
>
> *Cách xử lý: **đo** tỉ lệ pass/fail theo thời gian trước đã; **cách ly** test đó khỏi cổng, chuyển sang chạy hằng đêm, có hạn sửa; thay `sleep` bằng **chờ theo điều kiện** có timeout; kiểm sức khoẻ board trước mỗi job, board hỏng tự rút khỏi dàn; và tách *"hỏng do hạ tầng"* khỏi *"hỏng do code"* trên báo cáo."*

**Nếu bị hỏi tiếp**
- *"Một ví dụ trạng thái sót lại?"* — Ở lab của em: test bị giết khi đang giữ semaphore trong shared memory; lần chạy sau mở lại cùng tên và treo ([BLD-045](../mock-interview/bank/build-systems.md)).

**Móc nền tảng:** gated check-in ([BLD-021](../mock-interview/bank/build-systems.md)) · cô lập tài nguyên giữa các test.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-026](../mock-interview/bank/build-systems.md)</sub>

</details>

<details><summary><b>BLD-045</b> · Cổng ASan treo 25 phút thay vì đỏ — semaphore kẹt trong CI</summary>

**Một commit cài lỗi tràn mảng. Cổng ASan đáng lẽ đỏ sau 10 giây, nhưng pipeline treo 25 phút rồi mới báo timeout. Test của bạn có dùng shared memory + named semaphore của library. Chuyện gì đã xảy ra, và bạn sửa pipeline thế nào?**

> *"Chuyện này em gặp thật ở lab của mình. Commit cài lỗi tràn mảng **bên trong một hàm API của library**. ASan phát hiện và **giết process ngay**, đúng lúc nó **đang giữ semaphore** dùng chung. Semaphore không có chủ, nên kernel không nhả hộ — nó kẹt ở 0. Cổng ASan lúc đó chạy test **lần thứ hai** để lấy mã thoát, và integration test dùng **cùng một tên shared memory cố định**, nên process mới treo ở `sem_wait` ngay lúc library khởi tạo. Test không có timeout, nên chờ tới hạn mặc định 1500 giây — 25 phút.*
>
> *Em sửa ba lớp, mỗi lớp chặn một mắt xích: **mỗi lần chạy pipeline một tên shared memory riêng** và dọn khi xong, để rác của lần trước không lan sang lần sau; **timeout 60 giây cho mọi lần chạy test**, để treo biến thành đỏ nhanh; và cổng chỉ chạy test **một lần**, giữ mã thoát của chính lần đó.*
>
> *Bài học: timeout chỉ biến *treo* thành *đỏ*; gốc là **trạng thái dùng chung sống lâu hơn process** rò từ test này sang test khác. Và cùng một điểm yếu thiết kế của library — semaphore không có chủ — đã lộ ra trong CI trước khi lộ trên thiết bị."*

**Nếu bị hỏi tiếp**
- *"Sao không chỉ tăng timeout của job Jenkins?"* — Vẫn treo, chỉ treo lâu hơn hoặc ngắn hơn; và che mất lỗi thật.

**Móc nền tảng:** process chết khi giữ semaphore ([LNX-045](../mock-interview/bank/linux-sysprog.md)) · flaky test ([BLD-026](../mock-interview/bank/build-systems.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [BLD-045](../mock-interview/bank/build-systems.md)</sub>

</details>

*Câu đã có ở buổi trước: `RES-016`*


### 📚 Sổ · 13. R6 — troubleshooting

<details><summary><b>DBG-045</b> · Ba tình huống crash — bằng chứng gì, công cụ gì, cần thêm gì</summary>

**Library của bạn crash trên TV. Ba tình huống: bản debug, bản release, và máy không boot lên được. Với mỗi tình huống, bạn có bằng chứng gì, dùng công cụ gì — và cần thêm thứ gì để ra được dòng code?**

> *"Crash chỉ để lại **địa chỉ**. Đổi địa chỉ thành *"hàm nào, dòng nào"* luôn cần ba thứ: **offset** trong file — không phải địa chỉ tuyệt đối, vì mỗi lần chạy thư viện được nạp ở chỗ khác; **file symbol khớp đúng bản build** đó; và nhớ địa chỉ trả về chỉ vào lệnh **sau** lệnh gọi, nên lùi một.*
>
> *Ba tình huống ở chỗ em. **Bản debug**: log tự in backtrace kèm tên hàm — đọc là thấy; riêng hàm `static` vẫn chỉ hiện offset, nên vẫn cần `addr2line`. **Bản release**: binary đã bỏ symbol để nhỏ; có coredump, và **hệ thống nội bộ** phân tích nó — bên trong là gdb cùng symbol của đúng image đó. Em hiểu nó làm gì, và đã tự làm lại bằng gdb ở lab. **Không khởi động được**: chỉ có trace log — kernel oops dạng `hàm+offset/kích_thước`, hoặc log crash của user-space dạng `thư_viện(+offset)`; em dùng `addr2line` với file symbol của đúng image.*
>
> *Coredump cho nhiều hơn một điểm: cho cả **chuỗi giá trị** — biến sai từ khung nào truyền xuống — để đi ngược tới chỗ sai thật, không chỉ chỗ nổ."*

**Nếu bị hỏi tiếp**
- *"Dùng symbol của bản build khác thì sao?"* — Ra dòng **sai mà không báo lỗi** — nguy hiểm hơn ra `??`.

**Móc nền tảng:** coredump ([DBG-003](../mock-interview/bank/debugging.md)) · đọc oops ([DBG-015](../mock-interview/bank/debugging.md)) · lab `addr2line` và core ([DBG-043](../mock-interview/bank/debugging.md), [DBG-044](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-045](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-003</b> · Core dump là gì, phân tích thế nào</summary>

**Core dump là gì và dùng để làm gì? Phân tích thế nào?**

> *"Core dump là **ảnh chụp bộ nhớ của process tại lúc crash** — stack, heap, thanh ghi — ghi ra file. Giá trị của nó là phân tích **sau sự việc** mà không cần tái hiện lỗi; rất quý với lỗi hiếm hoặc lỗi ngoài hiện trường.*
>
> *Phân tích: mở bằng gdb cùng **đúng binary** đã sinh ra nó, có symbol:*
> ```
> gdb ./app core
> (gdb) bt               # chuỗi hàm tại lúc chết
> (gdb) frame 1          # lên khung của hàm đã gọi xuống
> (gdb) info locals      # biến cục bộ ở khung đó
> ```
> *Ở lab em làm đúng như vậy: không có symbol thì khung chỉ hiện `?? () from libsensor.so`; nạp đúng file symbol thì ra `sum_samples (buf=0x0, n=4) at sensor.c:8` — thấy ngay con trỏ `NULL` được truyền từ khung trên xuống.*
>
> *Điều kiện để có core: giới hạn kích thước core khác 0 (`ulimit -c`), và biết hệ thống đang gửi core đi đâu — trên Ubuntu là apport. Ở công ty, core của bản release do hệ thống nội bộ thu và phân tích."*

**Nếu bị hỏi tiếp**
- *"Binary đã bỏ symbol thì sao?"* — Giữ file symbol riêng theo từng bản build; gdb nạp nó qua liên kết debug ([DBG-045](../mock-interview/bank/debugging.md)).

**Móc nền tảng:** gdb · symbol · [DBG-044](../mock-interview/bank/debugging.md).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-003](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-004</b> · Segfault — điều tra thế nào</summary>

**Chương trình segfault. Bạn điều tra thế nào — và làm gì khi backtrace toàn `??`?**

> *"Bốn bước. **Lấy được crash đã**: bật core dump, hoặc chạy dưới gdb nếu tái hiện được. **Xem backtrace, đọc từ dưới lên**: khung trên cùng là chỗ nổ, thường không phải chỗ sai — đi xuống tới khung code **của mình** gần nhất. **In các con trỏ** ở khung đó: `0x0` hay địa chỉ lung tung là dấu hiệu. Và bước tìm ra bug thật: hỏi **"con trỏ này đến từ đâu?"** — giá trị trả về không được kiểm, thành viên của object đã huỷ, hay phần tử của vector vừa cấp phát lại.*
>
> *Backtrace toàn `??` có ba nguyên nhân. **Thiếu symbol** — binary đã bỏ symbol hoặc không đúng bản build: lấy đúng file symbol. Vài khung `??` xen kẽ: thư viện bên thứ ba không có symbol, cài gói debug của nó. Và nguy hiểm nhất: backtrace **vô lý** — hàm không thể gọi nhau mà lại nằm chồng lên nhau — nghĩa là **stack đã bị ghi đè**. Lúc đó đừng cố đọc nữa; backtrace vô lý chính là bằng chứng của tràn bộ đệm, chuyển sang ASan.*
>
> *Nếu không tái hiện được thì em không ngồi đoán: segfault chỉ là hậu quả muộn của một vi phạm xảy ra trước đó, nên bật ASan/UBSan để bắt nó ngay tại chỗ."*

**Nếu bị hỏi tiếp**
- *"Segfault trong library nhưng lỗi ở app?"* — Thường gặp: app truyền con trỏ đã giải phóng; đi ngược backtrace qua ranh giới API.

**Móc nền tảng:** core dump ([DBG-003](../mock-interview/bank/debugging.md)) · ASan ([DBG-042](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-004](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-019</b> · Oops vs panic</summary>

**Phân biệt kernel oops và panic?**

> *"**Oops** là kernel gặp lỗi nghiêm trọng — ví dụ một driver dereference con trỏ `NULL` — nó in thông tin lỗi ra log, giết tác vụ đang chạy đoạn code đó, và **cố chạy tiếp**. **Panic** là kernel **không thể chạy tiếp** an toàn — lỗi trong ngữ cảnh ngắt, lỗi ở phần lõi, hoặc hệ được cấu hình *"oops thì panic"* — nên dừng hẳn hoặc khởi động lại.*
>
> *Hai điều em lưu ý. Một, sau oops hệ vẫn chạy nhưng **trạng thái không còn tin được** — một khoá có thể đang bị giữ mãi, dữ liệu có thể đã hỏng — nên thu log xong thì khởi động lại. Hai, trên sản phẩm thường cấu hình **oops thì panic** rồi tự khởi động lại, kèm cơ chế lưu log lần chết, vì một thiết bị chạy tiếp ở trạng thái hỏng còn tệ hơn khởi động lại.*
>
> *Cả hai đều in thông tin ra log kernel: loại lỗi, hàm và offset, chuỗi gọi — đọc theo [DBG-015](../mock-interview/bank/debugging.md)."*

**Nếu bị hỏi tiếp**
- *"Khi máy không khởi động được thì lấy log ở đâu?"* — Console serial, hoặc vùng nhớ giữ log qua lần khởi động lại.

**Móc nền tảng:** đọc oops ([DBG-015](../mock-interview/bank/debugging.md)) · ngữ cảnh ngắt.

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-019](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-015</b> · Đọc kernel oops — thông tin gì</summary>

**Kernel oops: bạn đọc thông tin gì để tìm nguyên nhân?**

> *"Em đọc theo thứ tự năm thứ. Một, **loại lỗi và địa chỉ**: `NULL pointer dereference` với địa chỉ gần 0 là con trỏ chưa khởi tạo; địa chỉ dạng mẫu lặp như `6b6b6b6b` là dùng vùng đã giải phóng. Hai, **dòng PC** — dạng `my_driver_read+0x2c/0x80 [my_driver]`: chết ở hàm nào, offset bao nhiêu trong hàm, thuộc module nào. Đây là điểm neo. Ba, **Call Trace**: đường đi tới đó — vào từ system call nào, hay từ ngắt. Bốn, danh sách module đang nạp, xác nhận driver của mình có mặt. Năm, cờ **Tainted**: kernel đã oops trước đó chưa — nếu rồi thì dữ liệu lần này có thể không tin được.*
>
> *Bước quyết định là đổi `hàm+offset` ra dòng source, dùng **đúng file `.ko`** đã nạp, có debug info:*
> ```
> gdb my_driver.ko -ex 'list *(my_driver_read+0x2c)'
> ```
> *Lệch một lần build là offset chỉ sai hàm và dẫn đi lạc hoàn toàn.*
>
> *Hai bẫy: các dòng có dấu `?` trong Call Trace chỉ là đoán từ stack cũ; và oops trong **ngữ cảnh ngắt** gợi ý một kiểu lỗi khác hẳn — gọi hàm có thể ngủ trong ngắt."*

**Nếu bị hỏi tiếp**
- *"Bắt lỗi bộ nhớ trong kernel sớm hơn?"* — KASAN, bản kernel của ASan; LOCKDEP cho lỗi thứ tự khoá.

**Móc nền tảng:** oops vs panic ([DBG-019](../mock-interview/bank/debugging.md)) · `addr2line` ([DBG-045](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-015](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-021</b> · `printk`/`dmesg` trong driver</summary>

**`printk`/`dmesg` dùng thế nào trong driver? Vì sao nó khác `printf`, và khi nào KHÔNG được dùng?**

> *"`printk` ghi vào một **bộ đệm vòng trong kernel** có kích thước cố định; `dmesg` chỉ là công cụ đọc bộ đệm đó. Vì là vòng nên đầy thì **log cũ bị đè**.*
>
> *Khác `printf` ở ba điểm. Có **mức ưu tiên** — lỗi, cảnh báo, thông tin, debug — và console chỉ hiện các mức trên một ngưỡng; in mức debug mà không thấy gì thường là do ngưỡng, không phải code không chạy. Không có bộ đệm của thư viện C, ghi thẳng vào bộ đệm kernel nên sống sót tốt hơn khi crash. Và in ra console serial là thao tác **chậm và đồng bộ** — hàng mili giây cho một dòng.*
>
> *Nên em **không dùng** `printk` trong đường nóng hay trong ngắt: ngắt phải xong trong micro giây, một dòng in làm trễ cả hệ; và việc in **làm đổi thời gian**, có thể che mất chính lỗi đang tìm. Trong vòng lặp tần suất cao, log tự đè mất thông tin hữu ích.*
>
> *Thay vào đó: `printk_ratelimited` cho thông điệp có thể lặp; bật tắt log động thay vì build lại; và ftrace cho chỗ cần chi phí rất thấp."*

**Nếu bị hỏi tiếp**
- *"Ở resume em ghi debug bằng `dmesg/printk` — dùng thế nào?"* — Đặt log ở **ranh giới** — đầu hàm `ioctl`, chỗ ghi xuống phần cứng — để chia đôi lỗi ([RES-008](../mock-interview/bank/resume.md)).

**Móc nền tảng:** ngữ cảnh ngắt · heisenbug · logging ([DBG-029](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-021](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-029</b> · Logging ở production</summary>

**Vì sao logging là công cụ debug quan trọng nhất ở production? Một hệ thống log tốt cần gì?**

> *"Vì ở hiện trường không có debugger, không dừng được hệ thống, và nhiều lỗi không tái hiện được trong lab. Log là **bằng chứng duy nhất** còn lại sau sự việc — và lỗi hiếm thì vài ngày mới có một cơ hội.*
>
> *Một hệ log tốt cần: **mức** chỉnh được lúc chạy, không phải build lại; **hai loại thời gian** — đồng hồ đơn điệu để đo khoảng, giờ lịch để biết lúc nào; **mã định danh** — thiết bị, phiên, yêu cầu — để nối các dòng rời thành một câu chuyện, nhất là khi nhiều thread; **cấu trúc** khoá-giá trị để lọc và đếm được; **xoay vòng có trần dung lượng** để không làm đầy bộ nhớ; và một vùng giữ riêng **phút cuối trước khi chết**, vì xoay vòng hay nuốt đúng đoạn đó.*
>
> *Đánh đổi: log nhiều thì **mòn flash**, **đổi thời gian** chạy, và tốn băng thông. Nên em ghi **quyết định và đầu vào của quyết định** — kiểu *"chọn nhánh B vì nhiệt độ 71 vượt ngưỡng 70"* — thay vì *"đã tới đây"*.*
>
> *Và log không thay được số đo: rò bộ nhớ hay rò file descriptor lộ ra ở **xu hướng**, nên phải ghi định kỳ vài chỉ số — RAM trống, số fd, độ sâu hàng đợi."*

**Nếu bị hỏi tiếp**
- *"Library của em có gì cho debug?"* — Trong shared memory có lịch sử các process đã gọi API, để khi treo biết ai đang giữ khoá ([A1 §7.2](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)).

**Móc nền tảng:** `CLOCK_MONOTONIC` ([LNX-029](../mock-interview/bank/linux-sysprog.md)) · `printk` ([DBG-021](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-029](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-010</b> · Chương trình treo — điều tra thế nào</summary>

**Chương trình bị treo (hang). Điều tra thế nào?**

> *"Lợi thế của lỗi treo là **hiện trường còn nguyên** — đừng khởi động lại process. Bước đầu em xem **CPU**: `top -H -p <pid>` để biết nó đứng yên ở 0% hay quay ở 100% — câu đó chia đôi chẩn đoán ngay.*
>
> *Rồi gắn gdb vào và lấy backtrace của **mọi thread**: `gdb -p <pid>` rồi `thread apply all bt`.*
>
> *CPU gần 0%: nhiều thread kẹt ở khoá, thread này chờ khoá thread kia đang giữ — **deadlock**, truy thứ tự lấy khoá; một thread kẹt ở `read`, `recv` hay `poll` — **chờ I/O** mà thiếu timeout; kẹt ở chờ condition variable — có thể là **lost wakeup**. CPU gần 100%: lấy backtrace hai lần cách nhau vài giây — giống hệt thì **vòng lặp vô tận**, đổi qua lại vài trạng thái thì **livelock**.*
>
> *Thêm hai nguồn: `strace -p` cho biết đang kẹt ở system call nào, và `/proc/<pid>/wchan` hay stack của thread cho biết kernel đang chờ gì.*
>
> *Ở library của em có một ca đặc thù: process khác chết khi đang giữ semaphore dùng chung, mọi process sau treo ở `sem_wait` — backtrace của mọi thread chỉ thẳng vào đó."*

**Nếu bị hỏi tiếp**
- *"Treo hay chỉ chậm?"* — Hai backtrace cách nhau vài giây khác nhau thì nó đang chạy, chỉ chậm — chuyển sang `perf`.

**Móc nền tảng:** deadlock · condvar ([OS-012](../mock-interview/bank/os.md)) · semaphore kẹt ([LNX-045](../mock-interview/bank/linux-sysprog.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-010](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-018</b> · Target không có gdb đầy đủ</summary>

**Debug binary chạy trên thiết bị target không có GDB đầy đủ thì sao?**

> *"Em **tách đôi** công việc. Trên thiết bị chạy `gdbserver` — rất nhẹ, chỉ biết dừng, chạy, đọc ghi bộ nhớ. Trên máy host chạy **gdb đầy đủ** cùng **symbol và source**; hai bên nói chuyện qua mạng hoặc cổng serial.*
> ```
> # trên thiết bị
> gdbserver :2345 ./app            # hoặc: gdbserver :2345 --attach <pid>
> # trên host
> arm-linux-gnueabihf-gdb ./app
> (gdb) set sysroot /path/to/target/rootfs
> (gdb) target remote 192.168.1.10:2345
> ```
> *Ba điều kiện phải đúng: gdb trên host phải là gdb của **toolchain đúng kiến trúc** của thiết bị; binary trên host có symbol, binary trên thiết bị có thể đã bỏ symbol, nhưng **cùng một lần build**; và `sysroot` trỏ tới rootfs của thiết bị, nếu không thì thư viện hệ thống ra toàn `??`.*
>
> *Nếu ngay cả `gdbserver` cũng không chạy được thì em lấy **core dump** về host và phân tích sau; mức thấp hơn nữa — bootloader, trước khi có hệ điều hành — thì dùng JTAG."*

**Nếu bị hỏi tiếp**
- *"Biến hiện `<optimized out>`?"* — Bản build có tối ưu; cần thì build một bản debug riêng.

**Móc nền tảng:** cross toolchain · core dump ([DBG-003](../mock-interview/bank/debugging.md)) · symbol khớp bản build ([DBG-045](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-018](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-016</b> · Debug lỗi cross-layer</summary>

**Bạn debug một lỗi cross-layer thế nào?**

> *"Nguyên tắc của em là **chia đôi theo tầng**, không nhảy vào tầng mình quen nhất. Với một lỗi đi qua app, library, `ioctl`, driver và phần cứng, em **xác nhận dữ liệu ở từng ranh giới**: chỉ cần biết dữ liệu **đúng ở tầng N** và **sai ở tầng N+1** là khoanh được lỗi.*
>
> *Ở mỗi ranh giới em có một câu hỏi và một công cụ. App gửi đi đúng giá trị chưa — log, gdb, `strace` để xem system call thật sự phát ra. Lời gọi có xuống tới driver với tham số còn nguyên không — log ở đầu hàm `ioctl`. Driver nhận gì, ghi gì xuống thanh ghi — `dmesg`, ftrace, sysfs. Phần cứng có nhận đúng không — đọc lại thanh ghi.*
>
> *Nghi phạm hay gặp ở ranh giới user–kernel: sai kích thước khi copy, struct có padding khác nhau giữa hai bên, sai mã lệnh `ioctl`.*
>
> *Hai kỷ luật: **mỗi lần chỉ đổi một thứ** — sửa nhiều chỗ cùng lúc mà hết lỗi thì không biết cái nào chữa; và sửa xong thì **chạy lại từ app xuống tới phần cứng**, không chỉ test tầng vừa sửa. Với lỗi về thời gian, thêm log có thể làm lỗi biến mất — lúc đó dùng ftrace thay vì in log."*

**Nếu bị hỏi tiếp**
- *"Một ca cụ thể?"* — Xem [RES-008](../mock-interview/bank/resume.md).

**Móc nền tảng:** chia đôi ([09/mindset](../../09-debugging/mindset.md)) · `copy_from_user` ([DRV-006](../mock-interview/bank/drivers-embedded.md)) · `printk` ([DBG-021](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-016](../mock-interview/bank/debugging.md)</sub>

</details>

<details><summary><b>DBG-042</b> · Chọn sanitizer: ASan vs TSan vs valgrind</summary>

**"Bạn có ba lỗi: (a) đọc vượt biên mảng, (b) use-after-free, (c) data race giữa hai luồng. Với mỗi lỗi, bạn bật sanitizer nào? Cái nào bắt được cái gì?"**

> *"Em nhớ bằng một câu: **ASan trông địa chỉ, TSan trông thứ tự giữa các thread.**
>
> *(a) **Đọc vượt biên mảng** → **ASan**. Nó đặt vùng cấm quanh mỗi khối nhớ, chạm vào là báo ngay đúng dòng. TSan **không** bắt được lỗi này — em đã đo: TSan chạy qua vượt biên, in kết quả và thoát bình thường.*
>
> *(b) **Dùng sau khi giải phóng** → **ASan**, mặc định cho lỗi bộ nhớ. TSan cũng bắt được vì nó theo dõi vòng đời heap, nhưng đó không phải việc chính của nó.*
>
> *(c) **Data race** → **TSan**. Chỉ nó theo dõi được hai thread chạm cùng một địa chỉ mà không có đồng bộ. Ở lab em dùng TSan bắt đúng race giữa vòng cập nhật định kỳ và hàm API trong library.*
>
> *Ba điều thực tế: ASan và TSan **không bật chung** trong một lần build — phải build hai bản. Chúng tốn nhiều RAM — ASan khoảng ba lần, TSan nhiều hơn — nên thường chạy trên máy build chứ không trên thiết bị. Và trên kernel 6.x, TSan của gcc 11 cần chạy kèm `setarch -R` để tắt ngẫu nhiên hoá địa chỉ."*

**Nếu bị hỏi tiếp**
- *"Biến chưa khởi tạo?"* — MSan (của clang) hoặc valgrind memcheck.
- *"Lỗi chỉ xảy ra trên thiết bị thật?"* — Sanitizer không cứu được; dùng log ở ranh giới và core dump.

**Móc nền tảng:** sanitizer trong CI ([BLD-042](../mock-interview/bank/build-systems.md)) · segfault ([DBG-004](../mock-interview/bank/debugging.md)).

<sub>📚 Phân tích chi tiết, bẫy, output chạy thật: [DBG-042](../mock-interview/bank/debugging.md)</sub>

</details>

*Câu đã có ở buổi trước: `RES-008`*

<!-- END ANSWER-BOOK -->

---

## 15. 🖊️ Sổ hình vẽ — toàn bộ whiteboard (D0–D5)

> **Bản sao TỰ SINH** từ [whiteboard.md](../whiteboard.md) — sửa ở đó rồi chạy lại `python3 14-prep/study-plans/tools/build-answer-book.py`. Các link *"Vẽ: whiteboard Dx"* trong §14 nhảy thẳng tới đây.
>
> **Dùng thế nào:** mỗi hình có **nhịp vẽ** (vẽ tới đâu nói tới đó), **kịch bản nói** và **một câu đánh đổi**. Muốn ôn nhanh 10 phút: mở *Trang gấp* ở cuối.

<!-- BEGIN WHITEBOARD (tự sinh từ 14-prep/whiteboard.md — KHÔNG sửa tay) -->

### Whiteboard · 🎯 Luật vàng — đọc trước, quan trọng hơn cả bảy sơ đồ

<details><summary>Mở hình + kịch bản</summary>

**1. Vẽ ÍT, nói NHIỀU.** Whiteboard không phải để trình bày đủ, mà để **cho người nghe một cái neo**. Bốn ô và ba mũi tên là đủ cho 5 phút nói. Vẽ 12 ô là mất người nghe.

**2. Vẽ từ trên xuống: người dùng ở trên, phần cứng ở dưới.** Quy ước phổ quát, không cần giải thích.

**3. Đánh SỐ mũi tên khi kể một chuỗi.** Sau đó bạn **chỉ tay** thay vì nói lại — tiết kiệm nửa thời gian và nghe mạch lạc hơn hẳn.

**4. Ghi nhãn cho RANH GIỚI, không chỉ cho ô.** `C ABI`, `ioctl`, `GPL boundary` — ranh giới mới là chỗ interviewer quan tâm, vì đó là chỗ phát sinh quyết định thiết kế.

**5. Vừa vẽ vừa nói, đừng vẽ xong mới nói.** Im lặng 40 giây trước bảng là điểm trừ nặng. Nói *"trên cùng là app…"* trong lúc tay đang vẽ ô đầu tiên.

**6. Chừa chỗ trống bên phải và bên dưới.** Bạn **sẽ** phải thêm vào khi họ hỏi sâu. Vẽ kín bảng ngay từ đầu là tự chặn đường mình.

</details>

### Hình D0 · "Cái này để làm gì?" — 30 giây, cho người chưa biết display

<details><summary>Mở hình + kịch bản</summary>

⚠️ **Chỉ vẽ khi cần.** Dấu hiệu cần: họ hỏi *"dimming là gì?"*, hoặc mặt họ ngơ khi bạn nói về panel. Nếu họ gật đầu theo kịp thì **bỏ qua D0**, vào thẳng D1.

**Ý tưởng cốt lõi: đừng kể tên tính năng — kể HAI CÂU HỎI về cùng một tấm panel.**

```
                    Anh đang xem một tấm panel
                              |
             +----------------+----------------+
             |                                 |
      SÁNG BAO NHIÊU?                   HIỂN THỊ THẾ NÀO?
   (Picture Quality — dimming)          (Panel Control)
             |                                 |
   TÍNH độ sáng theo nội dung       RA LỆNH cho panel: bật/tắt,
   đang chiếu                       độ phân giải, ghi độ sáng
```

**Một câu cho mỗi câu hỏi — dùng phép so sánh, đừng dùng định nghĩa:**

| Câu hỏi | Câu nói (≤ 15 giây) |
|---|---|
| **Sáng bao nhiêu** — Picture Quality | *"Đằng sau tấm panel là một dàn đèn LED. Cảnh tối thì mình hạ đèn ở đúng vùng đó xuống — đen sâu hơn, tốn ít điện hơn. Phần này phải **tính toán** theo nội dung đang chiếu."* |
| **Hiển thị thế nào** — Panel Control | *"Bật tắt panel, đổi độ phân giải, đổi số hình mỗi giây, ghi độ sáng đã tính xuống phần cứng. Phần này chỉ là **ra lệnh**, không phải tính."* |

**Mini-glossary — mỗi từ một câu đời thường:**

| Từ | Câu nói |
|---|---|
| **Backlight** | *"Dàn đèn LED phía sau tấm panel."* |
| **Dimming** | *"Hạ hoặc tăng độ sáng đèn nền theo nội dung hình."* |
| **Panel driver** | *"Driver trong kernel nói chuyện trực tiếp với phần cứng của panel."* |
| **SoC** | *"Con chip chính của TV — CPU và các khối phần cứng trên cùng một chip."* |

> ⭐ **Vì sao cách kể này ăn điểm:** *"tính toán"* vs *"ra lệnh"* là phân biệt ai cũng hiểu, và nó **dọn đường cho D1**: hai câu hỏi chính là hai nhánh của kiến trúc.

</details>

### Hình D1 · Kiến trúc — sơ đồ MẶC ĐỊNH, vẽ đầu tiên trong 90% trường hợp

<details><summary>Mở hình + kịch bản</summary>

> 📐 **Luật hình thức:** trong ô **chỉ có DANH TỪ**. Mọi mô tả (*"logic nặng"*, *"command mỏng"*) nằm **ngoài** sơ đồ — trong lời nói hoặc bảng dưới. Ô nhồi chữ là ô không ai đọc kịp.

**Vẽ theo ba nhịp. Dừng sau mỗi nhịp và nhìn mặt họ.**

#### Nhịp 1 — chồng tầng, 20 giây · *"Em làm xuyên tầng, từ đây xuống đây."*

```
          +-------------------------+
          |    App / Middleware     |
          +-------------------------+
  - - - - - - - - - - - - - - - - - - - - - -   C ABI
          +-------------------------+
          |       libdisplay        |
          +-------------------------+
  - - - - - - - - - - - - - - - - - - - - - -   ioctl     user
  - - - - - - - - - - - - - - - - - - - - - -           ---------
          +-------------------------+                     kernel
          |      Kernel driver      |
          +-------------------------+
          +-------------------------+
          |           SoC           |
          +-------------------------+
```

**Nhãn nằm ở LỀ PHẢI, không nằm trong ô.** Ranh giới vẽ bằng nét đứt — nó không phải một tầng, nó là một **hợp đồng**.

#### Nhịp 2 — cái phễu, 20 giây · *"Vì sao chỗ đó gọi là chỗ hẹp."*

⭐ **Đây là nhịp mới, và là nhịp đáng giá nhất.** Vẽ nó **cạnh** nhịp 1, đừng xoá nhịp 1.

```
     App-1       App-2       App-3
        \          |          /
         \         |         /
          +--------+--------+
          |      C API      |        <-- CHỖ HẸP
          +--------+--------+
         /         |         \
        /          |          \
     chipA       chipB       chipC
```

> 🗣️ *"Trên có nhiều người dùng, dưới có hơn mười dòng chip — ở giữa là **đúng một hợp đồng**. Đó là lý do đổi chip thì app **không phải build lại**."*

**Vì sao hình phễu ăn hơn hình chồng tầng:** nó cho thấy **cái giá và cái lợi cùng lúc** — một hợp đồng phục vụ nhiều bên, nên nó **khó đổi**. Người nghe tự nghĩ ra câu hỏi tiếp theo (*"thế thêm tính năng cho một chip thì sao?"*), và đó chính là câu bạn muốn họ hỏi.

#### Nhịp 3 — hai nhánh, 25 giây · *"Bên dưới tách hai, vì hai bài toán khác nhau."*

```
                      C API
                        |
              +---------+---------+
              |                   |
        +-----+-----+       +------+--------+
        |  Dimming  |       | Panel Control |
        +-----+-----+       +------+--------+
              |                   |
            ioctl            panel_ops
              |                   |
              +------> SoC <------+
```

**Bảng đi kèm — nói, không vẽ:**

| | Dimming (Picture Quality) | Panel Control → panel driver |
|---|---|---|
| Lượng logic | **nặng** — thuật toán | **mỏng** — lệnh xuống phần cứng |
| Sống ở | userspace, C++ | hàm tự do ở library → `ioctl` → panel driver trong kernel |
| Đa hình đặt ở | Factory chọn `Algo` + `Backend` | bảng `panel_ops` |

> 🗣️ **Nguyên tắc chốt:** *"**Logic nặng đẩy lên userspace, thao tác thanh ghi mỏng giữ trong kernel.**"*

#### Nhịp 4 — chỉ vẽ khi họ hỏi *"chọn chip thế nào?"*

```
        Dimming
           |
       +---+---+
       |Factory|  <-- đọc board + panel config LÚC CHẠY
       +---+---+
           |
      +----+----+
      |         |
   +--+--+  +---+----+
   |Algo |  |Backend |   đổi chip: chỉ thay ô này
   +-----+  +--------+
```

#### 🗣️ Kịch bản nói — tiếng Việt, ~60 giây

> *"Trên cùng là app. Dưới nó là một **C API ổn định** — và đây là **chỗ hẹp**.*
>
> *[vẽ nhịp 2]* *Trên chỗ hẹp có nhiều người dùng, dưới nó có hơn mười dòng chip, ở giữa là **đúng một hợp đồng**. Nên đổi chip thì app **không phải build lại**.*
>
> *[vẽ nhịp 3]* *Bên dưới bọn em tách làm hai nhánh, vì **lượng logic khác nhau**: dimming nặng thuật toán nên nằm ở userspace C++; còn bật tắt panel, đổi độ phân giải chỉ là lệnh mỏng, đi thẳng xuống **panel driver** trong kernel.*
>
> *Nguyên tắc chung: **logic nặng đẩy lên userspace, thao tác thanh ghi mỏng giữ trong kernel**."*

> ⭐ Câu ăn điểm nhất là **"chỗ hẹp"** — nó cho thấy bạn nghĩ về **hợp đồng giữa các tầng**, không chỉ nghĩ về code. Nói nó **sớm**.

</details>

### Hình D2 · Sequence diagram — `lib_api_set_power(false)`

<details><summary>Mở hình + kịch bản</summary>

**Sơ đồ mạnh nhất của bộ.** Nó chứng minh ba component **thật sự phối hợp**, chứ không phải ba mảnh rời ghép vào slide.

> 🖊️ **Mẹo vẽ sequence diagram trên bảng — làm đúng thứ tự này:**
> **①** Viết tên **các bên** thành một hàng ngang ở trên cùng · **②** Kẻ **hết** các đường đời (vertical) xuống dưới — kẻ hết ngay, đừng kẻ dần · **③** Rồi mới điền mũi tên **từ trên xuống**, đánh số.
> Làm ngược lại thì hết chỗ ở dưới và phải vẽ lại. Kẻ đường đời trước = tự chừa chỗ.

```
   App            libdisplay        drv_panel_core     drv_panel_chipA
    |                  |                   |                   |
    | set_power(false) |                   |                   |
    |----------------->|                   |                   |
    |                  |                   |                   |
    |               +--+--+                |                   |
    |            1  | ramp|                |                   |
    |               +--+--+                |                   |
    |                  |                   |                   |
    |                  | 2 ioctl(BLANK)    |                   |
    |                  |------------------>|                   |
    |                  |                   | 3 ops->blank()    |
    |                  |                   |------------------>|
    |                  |                   |                +--+--+
    |                  |                   |                |blank|
    |                  |                   |                +--+--+
    |                  |                   |                   |
    |                  |                   | 4 ops->power_off()|
    |                  |                   |------------------>|
    |                  |                   |                +--+---+
    |                  |                   |                |panel |
    |                  |                   |                |back  |
    |                  |                   |                |logic |
    |                  |                   |                +--+---+
    |                  |                   |<- - - - 0 - - - - |
    |                  |<- - - - 0 - - - - |                   |
    |<- - - LIB_OK - - |                   |                   |
    v                  v                   v                   v

   ---->  gọi          - - ->  trả về         +--+  tự xử lý (không rời lifeline)
```

**Bốn bước, nói khi chỉ tay vào từng số:**

| # | Ai làm | Việc |
|---|---|---|
| **1** | `libdisplay` | hạ độ sáng **dần** về 0 — 10 bước, không cắt đột ngột |
| **2** | `libdisplay` → kernel | `ioctl(PANEL_BLANK)` — vượt ranh giới user/kernel |
| **3** | `core` → `chipA` | `ops->blank()` qua bảng con trỏ hàm — **core không biết tên chip** |
| **4** | `chipA` | cắt nguồn **theo thứ tự**: panel → backlight → logic |

#### ⭐ Phần ăn điểm: **vì sao thứ tự này, không phải thứ tự khác**

Đây là chỗ biến một sơ đồ mô tả thành một câu chuyện kỹ thuật:

| Đổi chỗ bước nào | Hậu quả |
|---|---|
| Bỏ **1**, cắt nguồn khi đèn còn sáng | người dùng **thấy một cú chớp** |
| **4 trước 3**, cắt nguồn khi panel còn quét | **rác hình** trên màn |
| Sai thứ tự **trong 4** (panel/backlight/logic) | dòng xung (inrush), hoặc treo ở lần bật sau |

> 🗣️ **Câu chốt:** *"Thứ tự này không do phần mềm quy định — nó do **ràng buộc vật lý** của panel. Bật lên thì chạy **ngược lại đúng thứ tự đó**."*
>
> Đây là câu phân biệt người **đã làm thật** với người **đọc tài liệu**. Người đọc tài liệu kể bốn bước; người làm thật biết **vì sao không đổi chỗ được**.

**Bật nguồn = đảo ngược** — không cần vẽ lại, chỉ cần một mũi tên ngược ở lề và nói:
> *"`set_power(true)` là đúng chuỗi này chạy ngược: cấp nguồn logic → backlight → panel, unblank, rồi mới **ramp độ sáng lên**."*

**Nếu họ hỏi *"bước 3 lỗi thì sao?"*** — câu về hợp đồng lỗi, trả lời thẳng:
> *"Đường tắt phải **đi hết**, kể cả khi một bước lỗi — dừng giữa chừng để lại phần cứng ở trạng thái nửa vời còn tệ hơn. Bọn em ghi lỗi lại, đi tiếp, rồi trả mã lỗi tổng hợp lên trên."*

</details>

### Hình D3 · Adaptive brightness — hai lớp thời gian chống nháy

<details><summary>Mở hình + kịch bản</summary>

**Đây là sơ đồ trả lời câu *"làm sao không bị nháy?"*.** Nó là một **thang thời gian**, không phải sơ đồ khối — và chính vì thế nó gây ấn tượng: ít ai vẽ trục thời gian trên whiteboard.

```
đọc     o    o    o    o    o    o    o    o    o    o      mỗi 400 ms lấy lux mới nhất
        <------------- 10 lần liên tiếp = 4 s ------------->
        cả 10 lần rơi vào CÙNG MỘT khoảng?
           có    -> ổn định -> tính target MỚI
           không -> đếm lại từ đầu, target giữ nguyên

target  ----------------------------------+
                                          +-------------    chỉ đổi khi ổn định 4 s
độ sáng                                  /
                              bò dần từng nấc nhỏ tới target
```

**Và cái mắt người thật sự thấy:**

```
độ sáng
  ^
  |                 target mới
  |                     .- - - - - - - -
  |                  _-'
  |               _-'      <-- từng nấc nhỏ, không nhảy bậc
  |  ____________-'
  |  ánh sáng phòng đổi, chờ đủ 4 s ổn định
  +--------------------------------------> t
```

#### 🗣️ Kịch bản nói — ~45 giây

> *"Có **hai lớp thời gian** tách biệt, và đó chính là chỗ chống nháy.*
>
> *Lớp chậm là **quyết định**: cứ 400 mili giây em lấy giá trị cảm biến mới nhất và xem nó rơi vào **khoảng** sáng nào. Chỉ khi **10 lần liên tiếp — 4 giây — cùng một khoảng** thì mới tính target mới; lệch một lần là đếm lại. Một bàn tay lướt qua hay đèn flash không đủ 4 giây nên không kéo được target đi.*
>
> *Lớp nhanh là **chuyển động**: khi target đổi, độ sáng **bò dần từng nấc nhỏ** tới đó, nên mắt không bắt được bước nhảy nào."*

#### Đánh đổi — phải tự nêu, đừng đợi họ hỏi

> *"Cái giá là **phản ứng chậm ít nhất 4 giây**. Bọn em chấp nhận vì biển quảng cáo đặt cố định, ánh sáng môi trường đổi chậm. Nếu là thiết bị cầm tay thì đánh đổi này **sai** — lúc đó em sẽ lấy mẫu nhanh rồi **lọc trung bình trượt**, để phản ứng nhanh với thay đổi bền mà vẫn bỏ qua xung."*

> ⭐ Câu cuối là câu senior: nó cho thấy bạn biết **quyết định của mình phụ thuộc bối cảnh nào**, và biết **phương án khác là gì**.

</details>

### Hình D5 · S-Box nhiều box — một bức tường, một mức sáng

<details><summary>Mở hình + kịch bản</summary>

**Đây là sơ đồ trả lời *"kể về S-Box"* và *"đồng bộ nhiều box thế nào?"*.** Ba box là đủ để thấy ý; router, bộ chia, cáp quang chi tiết thì bỏ.

#### Nhịp 1 — vấn đề, 10 giây · *"Nếu mỗi box tự chỉnh thì thế này."*

```
   KHÔNG đồng bộ                        CÓ đồng bộ
 +-------+-------+-------+          +-------+-------+-------+
 |▓▓▓▓▓▓▓|░░░░░░░|▒▒▒▒▒▒▒|          |▒▒▒▒▒▒▒|▒▒▒▒▒▒▒|▒▒▒▒▒▒▒|
 |▓▓▓▓▓▓▓|░░░░░░░|▒▒▒▒▒▒▒|          |▒▒▒▒▒▒▒|▒▒▒▒▒▒▒|▒▒▒▒▒▒▒|
 +-------+-------+-------+          +-------+-------+-------+
  mỗi ô sáng một kiểu                 nhìn như MỘT màn
```
> Vẽ nhanh bằng cách tô đậm/nhạt ba ô. Hình này **chính là bài toán** — tài liệu lắp đặt của sản phẩm cũng ghi: box không liên kết thì độ sáng mỗi box có thể khác nhau.

#### Nhịp 2 — hệ thống, 30 giây · *"Một box làm master, các box còn lại nghe theo."*

```
               PC (nguồn nội dung)
                  |
       =====+=====+=====+===========+======  mạng LAN
            | ②         |           |        PC -> mọi box: nội dung
            |           v           v        ② Box 1 -> Box 2, 3: mức sáng
sensor +---------+ +---------+ +---------+
 (*)-> |  Box 1  | |  Box 2  | |  Box 3  |
   ①   | MASTER  | |         | |         |
       +----+----+ +----+----+ +----+----+
            | ③         | ③         | ③
            v           v           v
      +-----------+-----------+-----------+
      |   cab 1   |   cab 2   |   cab 3   |  <- MỘT bức tường LED
      +-----------+-----------+-----------+
```
**Đánh số rồi chỉ tay:** ① cảm biến chỉ cắm vào **box master** · ② master tính **một** mức sáng, phát qua mạng cho các box còn lại — cùng đường mạng mà PC dùng để đưa nội dung xuống · ③ mỗi box ghi mức đó xuống phần tường của mình — kể cả master.

> Vẽ theo thứ tự: **tường trước** (đáy bảng) → ba box → đường mạng + PC → cảm biến cuối cùng. Cảm biến vẽ sau cùng để câu *"chỉ box này có cảm biến"* rơi đúng lúc tay đang vẽ nó.

#### Nhịp 3 — chỉ vẽ khi họ hỏi *"bên trong box chạy gì?"*

```
 Box 1 (MASTER)                                Box 2, Box 3
+--------------------------------------+      +------------------------+
| sensor --I2C--> libsensor (60 lần/s) |      |                        |
|                   | lux mới nhất     |      |                        |
|                   v mỗi 400 ms       |      |                        |
| libdisplay: lọc 4 s -> mức sáng      |      |                        |
|    |                                 |  LAN |                        |
|    +--POSIX mq--> app ---------------+----->+--> app                 |
|    |                                 |      |     |                  |
|    v                                 |      |     v                  |
| cabinet 1                            |      | libdisplay -> cabinet  |
+--------------------------------------+      +------------------------+
```
> Hai chặng, hai lựa chọn: **trong box** dùng POSIX message queue (một chiều, bản tin nhỏ — [RES-005](../mock-interview/bank/resume.md)); **giữa các box** dùng mạng, vì message queue chỉ chạy trong một máy.

#### 🗣️ Kịch bản nói — ~45 giây

> *"Nhiều box ghép thành một bức tường; nếu mỗi box tự chỉnh theo cảm biến riêng thì mỗi ô sáng một kiểu *(chỉ hình nhịp 1)*.*
>
> *Nên bọn em cho **một box làm master** *(vẽ Box 1)*: chỉ nó cắm cảm biến *(①)*. Nó lọc nhiễu, tính ra **một** mức sáng, rồi phát qua mạng cho các box còn lại *(②)*. Mỗi box ghi đúng mức đó xuống phần tường của mình *(③)* — nên cả bức tường đổi sáng cùng lúc, cùng mức.*
>
> *Và vẫn là **một binary**: box nào bật tính năng thì thành master, box còn lại chỉ nghe và áp. Anh muốn em kể phần lọc nhiễu, hay bên trong một box?"*

#### Đánh đổi — tự nêu trước khi bị hỏi

> *"Một master thì đơn giản và không bao giờ có hai nguồn mức sáng cãi nhau. Cái giá là **master mất thì cả tường đứng ở mức sáng cuối cùng** — bọn em chấp nhận, vì đó là thiết bị cố định có người vận hành; muốn tự bầu master mới thì phải thêm heartbeat và xử lý hai bên cùng tưởng mình là master, không đáng với sản phẩm này."* — chi tiết ở [RES-027](../mock-interview/bank/resume.md).

#### Câu đuổi hay gặp

| Họ hỏi | Trả lời một câu |
|---|---|
| *"Sao không để mỗi box tự đọc cảm biến rồi lấy trung bình?"* | Mỗi box phải nghe tất cả box khác và mọi box phải chạy cùng một phép tính trên cùng một dữ liệu đúng lúc — phức tạp hơn hẳn mà vẫn có thể lệch; một nguồn duy nhất thì hết lệch từ gốc. |
| *"Lệch thời gian giữa các box bao nhiêu?"* | Giá trị trùng nhau; thời điểm chỉ lệch trong một nhịp cập nhật 1/60 giây — mắt không thấy. Kiểm bằng nhìn và so log từng box. |
| *"Mất cảm biến thì sao?"* | Giữ độ sáng ở mức mặc định. |
| *"Thêm box thứ tư?"* | Không đổi gì ở master: thêm một bên nghe trên mạng. |

</details>

### Hình D4 · Panel driver trong kernel — ba phần và ranh giới license

<details><summary>Mở hình + kịch bản</summary>

⚠️ **Sơ đồ khó nhất, và ăn điểm cao nhất.** Chỉ vẽ khi họ hỏi về **kernel driver**, **bảng con trỏ hàm**, hoặc **GPL**. Đừng tự dựng lên — **D4 cần ~2 phút, cộng D4b nữa là ~4 phút**.

```
            +------------------------------+
            |        drv_panel_shim        |      1 .ko
            |  +------------------------+  |      proprietary
            |  |      shim_export       |  |      insmod TRƯỚC TIÊN
            |  +------------------------+  |
            |  |      shim_bridge       |  |
            |  |    panel_ops [][][]    |  |
            |  +------------------------+  |
            +----^--------------------^----+
                 |                    |
            (a) extern           (b) đăng ký
                 |                    |
       +---------+--------+  +--------+---------+
       | drv_panel_core   |  | drv_panel_chipA  |
       +------------------+  +------------------+
            GPL                 proprietary
                                hơn 10 loại

   (c) đường gọi:  core  ->  shim_export  ->  panel_ops  ->  chipA
```

**Ba quan hệ — nói khi chỉ tay, đừng viết vào ô:**

| | Ai → ai | Việc |
|---|---|---|
| **(a)** | `core` → `shim_export` | `extern` con trỏ hàm — **đây là chỗ vượt ranh giới license** |
| **(b)** | `chipA` → `shim_bridge` | tự **đăng ký** hàm của mình vào `panel_ops` lúc init |
| **(c)** | `core` → … → `chipA` | mọi lời gọi đi qua bảng; **core không biết tên chip nào** |

#### D4b · Sequence — thứ tự nạp ba module

⭐ **Vẽ cái này khi họ hỏi *"nạp sai thứ tự thì sao?"*** — và họ **sẽ** hỏi, đó là câu đầu tiên bất kỳ ai làm kernel nghĩ tới.

Sơ đồ khối ở trên cho thấy **ai nối với ai**. Sơ đồ này cho thấy **thứ tự theo thời gian** — và nó làm lộ ra một thứ mà sơ đồ khối giấu mất: **cửa sổ nguy hiểm**.

```
       drv_panel_shim        drv_panel_core        drv_panel_chipA
              |                     |                     |
 (1) insmod -->|                     |                     |
           +----+                   |                     |
           |init|                   |                     |
           +----+                   |                     |
              | panel_ops RỖNG      |                     |
              | EXPORT_SYMBOL       |                     |
              |                     |                     |
 (2) insmod --|-------------------->|                     |
              |                  +----+                   |
              |                  |init|                   |
              |                  +----+                   |
              |     extern ptr      |                     |
              |< - - - - - - - - - -|                     |
+---          |                     | đọc board config    |
|             |                     |                     |
|             |                     | (3) modprobe chipA  |
|             |                     |-------------------->|
| CỬA SỔ      |                     |                  +----+
| NGUY HIỂM   |                     |                  |init|
|             |                     |                  +----+
|             |        (4) đăng ký vào panel_ops          |
+---          |<------------------------------------------|
              |                     |                     |
              |  (5) ops->blank()   |                     |
              |<--------------------|                     |
              |                 dispatch                  |
              |------------------------------------------>|
              v                     v                     v

  [init] module init chạy   ---->  gọi   <- - -  trả về / đăng ký
```

**Năm bước — nói khi chỉ tay:**

| # | Chuyện gì xảy ra |
|---|---|
| **1** | `shim` nạp trước. Bảng `panel_ops` **tồn tại nhưng RỖNG**; symbol được export ra |
| **2** | `core` nạp. Trình nạp module **giải symbol** — đây là chỗ thứ tự được **ép buộc**, không phải do quy ước |
| **3** | `core` đọc board config rồi **tự nạp** driver chip đúng loại |
| **4** | `chipA` **điền hàm của nó vào `panel_ops`** — self-registration, `shim` không đi tìm ai cả |
| **5** | Từ đây: `core → shim → chipA`. **Không ai biết tên ai.** |

#### 🔴 Cửa sổ nguy hiểm — thứ chỉ sequence diagram mới cho thấy

Giữa **(2)** và **(4)**, `drv_panel_core` **đã sống** nhưng bảng `panel_ops` **vẫn rỗng**. Bất kỳ ai gọi vào bảng trong khoảng đó ⇒ **NULL pointer dereference**, tức kernel oops.

> 🗣️ **Cách nói khi họ hỏi:** *"Thứ tự giữa bước 1 và 2 thì **trình nạp module tự ép** — `core` extern symbol từ `shim`, nên `modprobe` đọc `modules.dep` và nạp `shim` trước. Dùng `insmod` thủ công mà sai thứ tự thì nó **báo lỗi rõ ràng** `Unknown symbol in module`, không crash âm thầm.*
>
> ***Chỗ thật sự rủi ro là giữa bước 2 và bước 4***  *— `core` đã chạy, bảng còn rỗng. Cách chặn là **Null Object**: điền sẵn cả bảng bằng stub trả `-ENOTSUP` và ghi log, rồi để chip driver ghi đè cái nó có. Biến một cú oops kernel thành một dòng log."*

⭐ **Vì sao đoạn này ăn điểm mạnh:** bạn **tự chỉ ra điểm yếu của hệ mình** *và* **biết tên giải pháp**. Đó là tín hiệu senior — người chỉ kể cái hay của hệ mình nghe như chưa từng vận hành nó.

#### ⭐ Câu chốt — nói SỚM, đừng chôn ở cuối

> *"Có **hai tầng** con trỏ hàm, và chúng tồn tại vì **hai lý do khác nhau**:*
> *— tầng `shim_export` là để đi qua **ranh giới license** — driver nền là GPL, driver chip là mã đóng;*
> *— tầng `shim_bridge` là để **đa hình theo chip** — đúng vai trò của vtable, nhưng viết bằng C."*

Người kể được **hai lý do khác nhau** cho hai tầng thì khác hẳn người chỉ nói *"bọn em dùng function pointer cho nó linh hoạt"*.

</details>

### Whiteboard · 💬 Nhận định về kiến trúc D4 — đọc trước khi vào phòng

<details><summary>Mở hình + kịch bản</summary>

> Bạn hỏi anh nghĩ gì về kiến trúc này. Dưới đây là đánh giá thẳng, gồm **cả chỗ interviewer sẽ khoan**. Biết trước thì trả lời được; không biết thì bị động.

#### ✅ Ba điểm mạnh — nói ra được thì ăn điểm

1. **Nó giải một ràng buộc THẬT, không phải over-engineering.** Ranh giới GPL là ràng buộc pháp lý có thật, và giải pháp là kỹ thuật. Interviewer thích nghe kiến trúc sinh ra từ ràng buộc hơn là kiến trúc sinh ra từ sở thích.
2. **Tách đúng hai mối quan tâm.** Một tầng cho license, một tầng cho đa hình. Nếu gộp làm một thì sau này gỡ bỏ ràng buộc license sẽ phải đập cả hai.
3. **`drv_panel_core` hoàn toàn không biết tên chip nào.** Hơn 10 chip, một codebase nền. Đây chính là giá trị kinh doanh, và nên nói bằng con số: *"hơn 10 dòng chip, một driver nền"*.

#### 🔍 Bốn chỗ interviewer sẽ khoan — và câu trả lời trung thực

**① *"Nếu nạp sai thứ tự thì sao?"***

Đây là câu đầu tiên bất kỳ ai làm kernel sẽ hỏi.
- Nếu `drv_panel_core` **extern symbol** từ `shim_export`, thì **trình nạp module tự bắt buộc thứ tự** — `modprobe` đọc `modules.dep` và nạp `shim` trước. Sai thứ tự chỉ xảy ra khi dùng **`insmod` thủ công**, và khi đó nó **báo lỗi rõ ràng** `Unknown symbol in module`, không phải crash âm thầm.
- 🗣️ **Trả lời tốt:** *"`modprobe` lo thứ tự vì nó có dependency theo symbol. Nếu script khởi động dùng `insmod` trực tiếp thì mình tự chịu trách nhiệm thứ tự — và đó là chỗ em sẽ chuyển sang `modprobe` nếu làm lại."*
- ⚠️ **Nhưng tầng 3 thì khác:** `drv_panel_chipA` được nạp **lúc chạy** theo board config, và bảng `panel_ops` **rỗng cho tới khi nó đăng ký xong**. Nếu có ai gọi vào bảng trong khoảng trống đó ⇒ **NULL pointer dereference**, tức kernel oops. Đây là rủi ro thật.

**② *"Slot chưa được điền thì gọi vào là gì?"***

Một bảng con trỏ hàm **không có giá trị mặc định** thì mọi slot trống là một quả mìn. Chip A hỗ trợ 40 hàm, chip B chỉ 30 — 10 slot còn lại là `NULL`.
- 🗣️ **Trả lời tốt:** *"Đúng chỗ yếu. Cách chữa là **Null Object** — điền sẵn cả bảng bằng stub mặc định **trả về `-ENOTSUP` và ghi log**, rồi để chip driver ghi đè cái nó có. Biến một cú oops kernel thành một dòng log và một mã lỗi."*
- Đây là câu **biến điểm yếu thành điểm mạnh**: bạn thừa nhận vấn đề **và** biết tên giải pháp.

**③ *"Thêm một hàm vào giữa bảng thì sao?"*** ⭐ **Câu nguy hiểm nhất**

Bảng con trỏ hàm **chính là một vtable viết tay** — nên nó thừa hưởng **đúng** vấn đề ABI của vtable: **chèn một slot vào giữa làm lệch toàn bộ slot phía sau**. `shim` mới + chip driver cũ (hoặc ngược lại) ⇒ gọi **nhầm hàm**, không phải lỗi nạp module. Nó **chạy**, và nó **sai**.
- ⚠️ **Một con số phiên bản khớp nhau cũng không cứu được** — version bảo vệ *"API có đủ không"*, **không** bảo vệ *"slot có đúng chỗ không"*.
- 🗣️ **Trả lời tốt:** *"Luật bọn em phải giữ là **chỉ thêm vào cuối bảng, không bao giờ chèn giữa, không bao giờ xoá**. Muốn chắc hơn thì thêm **trường `size`/số lượng slot** vào đầu struct để bên nạp kiểm tra được — đó là cách kernel làm với các struct ops có thể mở rộng."*

**④ *"Driver nền là GPL nhưng phụ thuộc module mã đóng — nó còn dùng được symbol GPL-only không?"***

Câu này **khó**, và nó là câu thật.
- Cơ chế: kernel đánh dấu `TAINT_PROPRIETARY_MODULE`, và module **phụ thuộc** module proprietary có thể **mất quyền** dùng symbol `EXPORT_SYMBOL_GPL`. Nghĩa là `drv_panel_core`, dù mang giấy phép GPL, có thể bị chặn dùng một số API lõi.
- 🗣️ **Trả lời trung thực:** *"Em biết kernel có lan truyền taint và có luật chặn symbol GPL-only theo hướng đó. Em **chưa kiểm cụ thể trên phiên bản kernel của bọn em** xem `drv_panel_core` có mất quyền nào không — đó là thứ em sẽ kiểm bằng cách thử link một symbol GPL-only và xem module có nạp được không."*
- ⭐ **Đừng đoán bừa ở câu này.** *"Em chưa kiểm, và đây là cách em sẽ kiểm"* mạnh hơn hẳn một khẳng định sai.

#### 🔧 Nếu được làm lại — câu trả lời cho *"em sẽ làm khác chỗ nào?"*

| Đổi gì | Được gì |
|---|---|
| **Nhóm `panel_ops` thành nhiều struct nhỏ theo chức năng** (power, timing, calibration) thay vì một bảng phẳng khổng lồ | Chip chỉ hỗ trợ một phần thì đăng ký đúng phần đó; versioning từng nhóm độc lập; và đây là **cách kernel làm sẵn** — `file_operations`, `net_device_ops` |
| **Null Object mặc định cho mọi slot** | Oops → log + `-ENOTSUP` |
| **Thêm trường `version` + `size`** vào đầu mỗi struct ops | Bắt được lệch ABI **lúc đăng ký**, không phải lúc gọi nhầm hàm |
| **`modprobe` + `MODULE_SOFTDEP`** thay vì insmod theo script | Thứ tự nạp do hệ lo, không do người |
| **Một fake chip driver đăng ký vào cùng bảng** | Test `drv_panel_core` **không cần phần cứng** — đây là thứ đội V&V sẽ rất thích, và nó khớp thẳng dòng JD *"drive the relation with V&V"* |

> ⭐ **Một câu đáng nói nếu có cơ hội:** *"Nhìn lại thì bọn em tự viết lại cái mà kernel vốn đã có — mô hình `struct ops` + `register/unregister`, giống `i2c_driver` hay `net_device_ops`. Nếu làm lại em sẽ bám sát khuôn đó ngay từ đầu, vì người mới vào đọc là hiểu luôn."*
>
> Câu này làm ba việc cùng lúc: cho thấy bạn **biết cách làm chuẩn của kernel**, bạn **tự đánh giá được việc mình làm**, và bạn nghĩ tới **người bảo trì sau**.

</details>

### Whiteboard · 🧭 Chiến lược: vẽ cái nào trước, mở rộng đi đâu

<details><summary>Mở hình + kịch bản</summary>

#### Cây quyết định

**Mọi câu đều bắt đầu từ một chỗ:**

```
     Họ hỏi bất kỳ câu nào về hệ thống
                    |
                    v
          +-------------------+
          |   D1  nhịp 1      |  chồng tầng, 20 giây
          |   (chồng tầng)    |  rồi DỪNG LẠI, nhìn mặt họ
          +---------+---------+
                    |
              họ hỏi tiếp gì?
                    |
                    v
             (tra bảng dưới)
```

⚠️ **Đừng bỏ qua bước này kể cả khi câu hỏi rất hẹp.** *"Để em vẽ nhanh bức tranh chung rồi đi vào chỗ anh hỏi"* — chưa ai từ chối câu đó, và nó làm mọi câu sau ngắn đi một nửa.

**Bảng tra — họ hỏi gì thì vẽ gì:**

| Họ hỏi | Vẽ | Câu mở đầu |
|---|---|---|
| *"dimming là gì?"* · mặt họ ngơ | **D0** rồi **quay lại D1** | *"Cứ hình dung hai câu hỏi về một tấm panel: sáng bao nhiêu, và hiển thị thế nào…"* |
| *"vì sao app không phải build lại?"* · *"API ổn định thế nào?"* | **D1 nhịp 2** (cái phễu) | *"Trên nhiều app, dưới hơn 10 chip, giữa một hợp đồng."* |
| *"các thành phần tương tác ra sao?"* | **D1 nhịp 3** (hai nhánh) | *"Tách hai vì lượng logic khác nhau."* |
| *"đổi chip thì sửa gì?"* | **D1 nhịp 4** (Factory) | *"Chỉ một backend mới. Không gì ở trên đổi."* |
| *"kể một luồng cụ thể"* · *"tắt màn hình thì sao?"* | **D2** (sequence) | *"Em kẻ bốn bên ra, thời gian chạy xuống."* |
| 🔆 *"kể về S-Box"* · *"đồng bộ nhiều box thế nào?"* | **D5** (nhiều box) | *"Nếu mỗi box tự chỉnh thì thế này — để em vẽ."* |
| *"adaptive brightness làm sao?"* · **"sao không bị nháy?"** | **D3** (thang thời gian) | *"Có hai lớp thời gian, và đó chính là chỗ chống nháy."* |
| *"phần kernel thế nào?"* · *"function pointer table?"* · *"GPL?"* | **D4** (ba module) | *"Hai tầng con trỏ, hai lý do khác nhau."* |
| *"nạp sai thứ tự thì sao?"* | **D4b** (sequence nạp) | *"Cái này dễ thấy hơn trên trục thời gian."* |
| *"em sẽ làm khác chỗ nào?"* | **không vẽ** — nói | §🔧 *Nếu được làm lại* ở trên |

> 📌 **D3 và D5 là hai sơ đồ dễ bị bỏ quên nhất** vì chúng không nằm trên trục kiến trúc — nhưng **S-Box là project bạn tự chọn để kể** ở câu mở màn, nên xác suất bị hỏi vào nó rất cao. Đừng để nó là sơ đồ duy nhất bạn chưa tập.

#### Bốn quy tắc điều hướng

**1. Luôn bắt đầu bằng **D1 nhịp 1**, kể cả khi họ hỏi câu hẹp.** Ba mươi giây dựng bối cảnh giúp mọi câu sau ngắn đi một nửa. *"Để em vẽ nhanh bức tranh chung rồi đi vào chỗ anh hỏi"* — chưa ai từ chối câu đó.

**2. D0 là **phản ứng**, không phải kế hoạch.** Chỉ vẽ khi thấy dấu hiệu họ không theo kịp. Vẽ D0 cho người đã biết là hạ thấp họ.

**3. D4 là **vũ khí, không phải mở màn**.** Nó cần 2–3 phút và nó kéo theo cả loạt câu hỏi khó (thứ tự nạp, ABI, GPL). Vẽ nó khi bạn **đang thắng** và muốn nâng độ sâu — đừng vẽ khi đang lúng túng.

**4. Mỗi sơ đồ chốt bằng **một đánh đổi**.** Sơ đồ mô tả *cái gì*; đánh đổi cho thấy bạn *đã cân nhắc*. Một câu là đủ:

| Sơ đồ | Câu đánh đổi để chốt |
|---|---|
| D1 | *"Chỗ hẹp làm app khỏi build lại, nhưng nó **khoá** hình dạng API — thêm khả năng mới cho một chip là chuyện khó."* |
| D2 | *"Thứ tự do **ràng buộc vật lý**, không phải do phần mềm."* |
| D3 | *"Đổi độ trễ 4 giây lấy khả năng chống nhiễu. Thiết bị cầm tay thì đánh đổi này sai."* |
| D5 | *"Một master thì không bao giờ có hai nguồn mức sáng; mất master thì cả tường đứng ở mức cuối — chấp nhận được."* |
| D4 | *"Hai tầng con trỏ trả giá bằng **một lần gọi gián tiếp nữa** và **khó debug hơn** — đổi lại là đi qua được ranh giới license."* |
| D4b | *"Nạp động cho phép một driver nền chạy với hơn 10 chip — cái giá là **một cửa sổ mà bảng còn rỗng**, và bọn em phải chặn nó bằng stub mặc định."* |

#### Ba câu chuyển cảnh — học thuộc, dùng để giành lại quyền dẫn

> *"Để em vẽ nhanh cái này, sẽ nhanh hơn là em kể."* — dùng khi câu hỏi mơ hồ.
> *"Cái này liên quan tới một chỗ khác, em vẽ thêm được không?"* — dùng để **mở rộng sang sơ đồ bạn mạnh**.
> *"Chỗ này em nhớ không chắc, nhưng cơ chế thì thế này…"* — dùng khi bị hỏi chi tiết không nhớ. **Đừng bịa.**

</details>

### Whiteboard · 🇬🇧 Bản tiếng Anh — vẽ và nói thế nào cho dễ hiểu nhất

<details><summary>Mở hình + kịch bản</summary>

#### Luật riêng khi vẽ bằng tiếng Anh

**1. Ô ghi DANH TỪ, mũi tên nói bằng ĐỘNG TỪ.** Viết `Factory` trong ô, **nói** *"the factory picks the backend"*. Viết câu dài lên bảng vừa chậm vừa dễ sai ngữ pháp.

**2. Dùng từ NGẮN, GỐC ANH.** Người nghe có thể cũng không phải người bản xứ.

| ❌ Đừng viết/nói | ✅ Dùng |
|---|---|
| *utilize · leverage* | **use** |
| *implement the functionality* | **do it** / **handle it** |
| *multi-chipset abstraction layer* | **one interface, many chips** |
| *display enhancement solution* | **picture quality features** |
| *in order to* | **to** |
| *at this point in time* | **then** / **now** |

**3. Ba từ chuyên ngành PHẢI nói đúng** — nói sai là mất tín nhiệm ngay:

| Từ | Nghĩa nên nói kèm lần đầu |
|---|---|
| **backlight** | *"the LEDs behind the panel"* |
| **panel driver** | *"the kernel driver that talks to the panel hardware"* |

**4. Nhãn tiếng Anh cho ranh giới — viết đúng cụm này:** `C ABI` · `user / kernel boundary` · `ioctl` · `GPL boundary` · `function pointer table`. Đây là thuật ngữ chuẩn, người nghe nhận ra ngay.

#### Kịch bản tiếng Anh cho từng sơ đồ

**D0 — hai câu hỏi, ~30 giây**
> *"Think of two questions about one panel. **How bright should it be?** That's picture quality — mainly dimming. There are LEDs behind the panel, and we dim them per region based on the content, so dark scenes look deeper and use less power. That part **computes** something as the picture changes. **How should the panel be driven?** That's panel control — turning the panel on and off, changing the resolution, writing the final brightness to the hardware. That part just **issues commands**."*

**D1 — kiến trúc, ~60 giây** *(ba nhịp — vẽ tới đâu nói tới đó)*

> *[nhịp 1 — chồng tầng]*
> *"At the top is the application. Below it we expose a **stable C API**. Below that, the shared library. Then this dashed line is the **user–kernel boundary** — we cross it with an `ioctl`. Then the kernel driver, then the SoC."*
>
> *[nhịp 2 — cái phễu]*
> *"Let me redraw that middle part, because it's the interesting bit. **Several applications above, more than ten chip families below — and exactly one contract in the middle.** That's what we call the **narrow waist**. It's the reason the application **never gets rebuilt when we change chips**."*
>
> *[nhịp 3 — hai nhánh]*
> *"Under the waist we split into two branches, because **the amount of logic is different**. **Dimming** is algorithm-heavy, so it lives in **user space, in C++**. **Panel control** — power, resolution — is just thin commands, so it goes straight down to the **panel driver in the kernel**.*
>
> *The rule is: **heavy logic goes up to user space, thin register work stays in the kernel**."*
>
> *[chỉ khi họ hỏi "how do you pick the chip?"]*
> *"A factory reads the board configuration **at runtime** and builds the right algorithm-and-backend pair. New chip means **one new backend** — nothing above changes."*

**D2 — sequence diagram, ~60 giây**

> *[vừa kẻ các đường đời vừa nói]*
> *"Let me put the four parties across the top — the app, the library, the base kernel driver, and the chip driver. Time runs **downward**."*
>
> *[điền mũi tên, chỉ vào từng số]*
> *"The app calls `set_power(false)`.*
> ***One** — the library **ramps the brightness down** over ten steps. We don't cut it, or the user sees a flash.*
> ***Two** — it crosses into the kernel with an `ioctl`.*
> ***Three** — the base driver calls `blank` **through the function pointer table**, so it **never knows which chip it's talking to**.*
> ***Four** — the chip driver cuts the power rails, in this order: panel, backlight, logic.*
>
> *And here's the part I'd highlight: **that order isn't a software choice — it's what the hardware physically needs.** Powering up runs the **exact same sequence in reverse**."*

> 💬 **Nếu họ hỏi *"what if step three fails?"*** — *"We **run the rest of the path anyway**. Stopping halfway leaves the hardware in a half-off state, which is worse. We record the failure, finish the sequence, and return an aggregated error code."*

**D3 — two time layers, ~45 giây**
> *"There are **two separate time constants**, and that's what removes the flicker.*
> *Every **four hundred milliseconds** we take the latest sensor value and check which brightness band it falls into. Only when **ten readings in a row — four seconds — stay in the same band** do we compute a new target; one reading outside resets the count. A hand waving past the sensor never lasts four seconds, so it can't move the target.*
> *Once the target changes, brightness **walks toward it in small steps**, so your eye never catches a jump.*
> *The cost is **at least a four-second reaction time**. We accepted that because signage is fixed and ambient light changes slowly. For a handheld device that trade-off would be wrong — there I'd sample fast and use a moving average instead."*

**D5 — multi-box wall, ~45 giây**
> *"Several boxes drive one LED wall. If each box adjusts to its own light sensor, every tile ends up a different brightness *(point at the 'not synced' picture)*.*
> *So **one box is the master** — only it has the sensor ①. It filters the readings, computes **one** brightness level, and sends it over the network to the other boxes ②. Every box writes that same level to its part of the wall ③, so the whole wall changes together.*
> *It's still **a single binary**: whichever box has the feature enabled becomes the master; the others just listen and apply. If the master goes down, the wall simply stays at the last level — an accepted trade-off for a fixed installation."*

**D4 — kernel, ~70 giây**
> *"There are **three modules**. The first one loaded is a **proprietary shim**: it exports a set of function pointers, and it holds an **ops table** that starts out empty.*
> *Second is the **base driver, which is GPL**. It externs those pointers from the shim.*
> *Third, at runtime, the base driver reads the board configuration and loads the **chip-specific driver**, which **registers its functions into the ops table**.*
> *From then on, a call goes base driver → shim → the right chip, and **nobody needs to know anybody's name**.*
> *There are **two levels of indirection, and they exist for two different reasons**: one crosses a **licence boundary** — the base driver is GPL, the chip drivers are closed — and the other gives us **polymorphism in C**, which is what a vtable does in C++."*


**D4b — load order, ~40 giây** *(chỉ khi họ hỏi "what if they load in the wrong order?")*

> *"Let me draw the **load order** — it's easier to see on a timeline.*
> ***One**, the shim loads first: the ops table exists but it's **empty**, and the symbols are exported.*
> ***Two**, the base driver loads. It externs those symbols, so **the module loader enforces the order for us** — `modprobe` reads the dependency and pulls the shim in first. If you use plain `insmod` in the wrong order you get a clear **`Unknown symbol in module`**, not a silent crash.*
> ***Three**, the base driver reads the board config and loads the chip driver.*
> ***Four**, the chip driver **registers its functions into the table**.*
>
> *And here's the part worth pointing at: **between step two and step four the base driver is alive but the table is still empty.** Calling into it there is a null dereference. The fix is a **null-object default** — prefill every slot with a stub that logs and returns `-ENOTSUP`, then let the chip driver override what it supports."*

#### Bốn câu cứu hộ bằng tiếng Anh — học thuộc

> *"Let me draw that — it'll be faster than explaining."*
> *"Can I add one more box here? It connects to what you just asked."*
> *"I'm not certain about that detail, but the mechanism is…"* — **đừng bịa; đây là câu thay thế.**
> *"Sorry — let me say that again more simply."* — dùng khi bạn tự thấy câu vừa nói lủng củng. Nói ra rồi nói lại **tốt hơn** là cố gỡ.

</details>

### Whiteboard · 📄 Trang gấp — liếc 10 phút trước khi vào phòng

<details><summary>Mở hình + kịch bản</summary>

> Bản thu nhỏ của cả bảy sơ đồ. Không đọc để học — **liếc để nhớ hình dạng**.

```
 D0  hai câu hỏi         D1.1 chồng tầng       D1.2 cái phễu
     |                        App                 \  |  /
 sáng bao nhiêu?              -- C API --          C API
 hiển thị thế nào?            libdisplay          /  |  \
                              -- ioctl --        chipA B C
                              driver / SoC

 D1.3 hai nhánh          D1.4 factory          D2  sequence set_power
        C API                 Factory          App  lib  core  chip
       /     \               /      \           |----->|
  Dimming   PanelCtl      Algo    Backend       | 1 ramp
   ioctl    panel_ops                           | 2 ioctl -->|
       \     /                                  |   3 blank -->|
         SoC                                    |   4 power_off->|

 D3  hai lớp thời gian            D4  ba module            D4b nạp
 đọc   o o o o o o o o o o  400ms       shim                 1 shim (RỖNG)
       10 lần cùng khoảng = 4s      (export+bridge)         2 core extern
 target ---+                       core(GPL)  chipA(prop)    3 core->chipA
           +--- bò dần từng nấc     (a)extern  (b)đăng ký   4 chipA đăng ký
                                                           ^ CỬA SỔ RỖNG

 D5  nhiều box
 sensor->[Box1 MASTER]--LAN-->[Box2] [Box3]
             |                  |      |
           [cab1]            [cab2] [cab3]   = MỘT bức tường
```

**Một câu đánh đổi cho mỗi sơ đồ — thuộc đúng bảy câu này:**

| | Câu |
|---|---|
| D1 | *Chỗ hẹp làm app khỏi build lại, nhưng nó **khoá** hình dạng API.* |
| D2 | *Thứ tự do **ràng buộc vật lý**, không phải do phần mềm.* |
| D3 | *Đổi **độ trễ 4 giây** lấy khả năng chống nhiễu. Thiết bị cầm tay thì sai.* |
| D5 | *Một master: không bao giờ có hai nguồn mức sáng — mất master thì tường **đứng ở mức cuối**.* |
| D4 | *Hai tầng con trỏ: một cho **license**, một cho **đa hình**.* |
| D4b | *Nạp động đổi lại bằng **một cửa sổ bảng còn rỗng** — chặn bằng stub mặc định.* |
| D0 | *(không có đánh đổi — chỉ để người nghe bắt kịp)* |

**Đường đi mặc định:** `D1.1` → *(họ hỏi gì thì rẽ đó)* → `D1.2 phễu` · `D1.3 hai nhánh` · `D2 sequence` · `D4 → D4b`.

**Ba câu giành lại quyền dẫn:** *"Để em vẽ nhanh, sẽ nhanh hơn là kể."* · *"Em vẽ thêm một ô nữa được không?"* · *"Chỗ này em nhớ không chắc, nhưng cơ chế thì thế này…"*

</details>

### Whiteboard · ✅ Checklist trước khi vào phòng

<details><summary>Mở hình + kịch bản</summary>

- [ ] Vẽ được **D1 nhịp 1** trong **20 giây** mà không nhìn file này — **ô chỉ có danh từ**, nhãn ở lề
- [ ] Vẽ được **D1 nhịp 2 (cái phễu)** và nói được câu *"nhiều app trên, hơn 10 chip dưới, một hợp đồng ở giữa"*
- [ ] **D2: kẻ HẾT 4 đường đời TRƯỚC**, rồi mới điền mũi tên từ trên xuống — tập đúng thứ tự này 3 lần
- [ ] Nói được **vì sao thứ tự 4 bước của D2 không đổi chỗ được** (ràng buộc vật lý, không phải phần mềm)
- [ ] Nói được **hai lớp thời gian** của D3 (đọc 400 ms, ổn định 4 s, bò dần) kèm **đánh đổi**
- [ ] Vẽ được **D5** trong 30 giây: hình *có/không đồng bộ* trước, rồi 3 box + sensor + LAN + tường, **đánh số ① ② ③**
- [ ] Nói được **hai lý do khác nhau** cho hai tầng con trỏ hàm ở D4
- [ ] Vẽ được **D4b** và chỉ ra được **cửa sổ nguy hiểm** giữa bước (2) và (4)
- [ ] Thuộc **một câu đánh đổi** cho mỗi sơ đồ
- [ ] 🇬🇧 Đọc to **D1 và D2 bằng tiếng Anh**, bấm giờ, mỗi cái **≤ 60 giây**
- [ ] Biết trước **bốn câu khoan** ở §nhận định D4 — nhất là câu ④ (GPL taint), câu **không được đoán bừa**

---

⬅️ [study-plans](README.md) · Từ vựng: [in-practice/README.md](../../11-design-patterns/in-practice/README.md) · Kiến trúc đầy đủ: [A1-baseline-libdisplay.md](../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)

</details>

<!-- END WHITEBOARD -->
