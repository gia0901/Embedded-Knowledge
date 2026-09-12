# 🎯 Datalogic — Software Engineer (C/C++17, Embedded Linux) · SPRINT 3–4 NGÀY

> ♻️ **Viết lại hoàn toàn 2026-09-11.** Plan cũ (28 ngày, tối ưu **độ phủ bank**, kèm 3 bộ lab phần cứng) đã **bỏ** — nó nhắm sai mục tiêu cho một buổi phỏng vấn sau 3–4 ngày. Bản cũ còn trong git (`git show a254388:14-prep/study-plans/datalogic-plan.md`).
> Nguyên tắc mới, thay cho "phủ bank": **xác suất bị hỏi × mức độ sẽ ú ớ**. Không mở rộng kiến thức mới; chỉ chuyển thứ đã biết thành **câu nói ra được** và **dòng code viết tay được**.

---

## 📍 Tiến độ hiện tại — **RESUME Ở ĐÂY** (nguồn tracking DUY NHẤT)

> ⚠️ **TRẠNG THÁI, không phải nhật ký.** Sau mỗi buổi: **SỬA** ô `Xong?` + dòng *Buổi gần nhất* + *▶️ LÀM TIẾP*. **Không thêm dòng mới.**
> Diễn biến từng phiên → [`sessions/`](../mock-interview/sessions/) · câu yếu → [weak-register](../mock-interview/weak-register.md) · lỗ hổng tài liệu → [gap-register](gap-register.md).

### Trạng thái

| | |
|---|---|
| **Mục tiêu** | Buổi phỏng vấn kỹ thuật — **3–4 ngày nữa** tính từ 2026-09-11 |
| **Ngân sách** | ~2h/ngày × 4 buổi ≈ **8h**. Vượt ngân sách = cắt, không kéo dài |
| **Ngôn ngữ PV** | **50% Việt / 50% Anh** ⇒ mọi câu ruột phải nói được **cả hai thứ tiếng** |
| **Hình thức coding** | **Trên giấy** — 3 dạng: (1) coding problem · (2) implement DS cơ bản · (3) **đọc code tìm lỗi** |
| **Buổi gần nhất** | ✅ **B1 — 2026-09-12**, 42/60 = **3.00**, 15 câu ([log](../mock-interview/sessions/2026-09-12--B1--resume.md) — 856 dòng, tự chứa, có **§A–§E** là tài liệu ôn) |
| **Chẩn đoán còn hiệu lực** | **T1 3.67 · T2 2.1** ⇒ nền chắc, hụt ở **vận dụng**. Không đọc thêm tài liệu mới |

### ▶️ LÀM TIẾP — 4 buổi, chạy đúng thứ tự

| # | Buổi | Nội dung | ~ | Xong? |
|---|---|---|---|---|
| **B1** | 🗣️ **RESUME + dẫn chuyện + process JD** | §3 bản đồ phủ resume · §4 opening 3 móc · §5 behavioral theo JD mới | 120′ | ✅ **3.00** |
| **B2** | ✍️ **CODING GIẤY** | 3 bài implement viết tay + 6 snippet bug-hunt + DSA rapid (§6) | 120′ | ⬜ |
| **B3** | ⚙️ **C++17 + C-kernel + vá gap JD** | §7 C++17 ba tầng · C thuần · **I2C/SPI (cầu nối)** · §8 ba câu trả lời trung thực | 120′ | ⬜ |
| **B4** | 🎬 **Giả lập vòng thật** | `comprehensive` cấu hình riêng JD (75′) + coding từ **file trống** (45′) | 120′ | ⬜ |

### ⏭️ Trước khi vào B2 — 5 việc, ~60 phút

| # | Việc | ~ |
|---|---|---|
| 1 | 🎙️ Đọc to **[RES-001 Bản C](../mock-interview/bank/resume.md) bản Việt** 5 lần, bấm giờ, tới khi ổn định **≤ 90s** | 15′ |
| 2 | 🎙️ Đọc to **[RES-032](../mock-interview/bank/resume.md) bản English** 5 lần, ≤ 90s | 15′ |
| 3 | 🎙️ Đọc to **[BEH-016](../mock-interview/bank/behavioral.md)** (vì sao ứng tuyển — 3 nhịp) 3 lần | 10′ |
| 4 | Đọc **[OS-029](../mock-interview/bank/os.md)**, nhớ đúng 3 thứ: **`nice` ≠ ưu tiên** · **FIFO preempt sạch OTHER** · **`sched_rt_runtime_us` chừa 5%** | 20′ |
| 5 | Thuộc **quy tắc số ba** ([RES-029](../mock-interview/bank/resume.md)): 0 lỗi / n lần ⟹ tỉ lệ < 3/n ở 95% ⇒ **100 lần ⟹ < 3%** | 2′ |

### 📉 Ba lỗ hổng đo được ở B1 — mang sang các buổi sau

1. 🔴 **ĐÓNG GÓI, không phải kiến thức.** Cả 4 câu điểm 2 (`RES-001`, `BEH-016`, `RES-032`, và câu chốt của `RES-016`) đều là **nội dung có sẵn, không ra được hình dạng người nghe cần**. Bằng chứng đanh nhất: câu HAL đa chipset đạt **4 điểm**, nhưng khi phải **tự chọn** kể gì trong 90 giây thì đúng nội dung đó **không xuất hiện**. **Bài học #3 tái phát lần thứ tư.**
2. 🔴 **DEBUG — lỗi TRUY XUẤT, không phải lỗ hổng.** `RES-008` = 2đ, trong khi **bank đã dạy đúng phần "cắt đôi" từ trước phiên**. ⇒ **Đọc lại không chữa được.** Phải luyện phản xạ: nghe triệu chứng ⇒ hỏi ngay *"phép đo nào chia đôi được?"*. Cân nhắc **mở lại 2 bài 🧪 `DBG` ngay sau buổi PV**.
3. 🟠 **Chưa biến số đo thành lập luận.** *"100 lần"* mà không biết vì sao (§`RES-029`) · verify porting tool hoàn toàn thủ công ở cuối đường · V&V thì **xin** dữ liệu thay vì **gửi build có đo** (`BEH-015`). Cùng một hình dạng, và JD gọi đúng tên nó: *"drive the relation with V&V in a fruitful manner"*.

**Nguyên tắc khi phải cắt:** giữ **B1 và B2** bằng mọi giá. B3 cắt được phần C-kernel. B4 cắt được phần coding-file-trống. **Không bao giờ cắt B1** — 40% JD này nằm ở resume/process.

---

## 1. JD mới → gap map (đọc trước mọi buổi)

**JD này ~40% là process & communication**, không phải kỹ thuật thuần: *quản lý requirement · tham gia định nghĩa architecture · **tự quyết design rồi bảo vệ trong deep technical discussion** · làm việc với đội **V&V** · benchmark đối thủ · viết tài liệu · **peer review***. Đây là phần plan cũ bỏ trắng hoàn toàn.

| Trụ JD | Resume có | Trạng thái | Xử lý |
|---|---|---|---|
| C/C++17, Python | ✅ | 🟢 mạnh | §7 — nhưng có **rủi ro C++17**, xem cảnh báo |
| Linux userspace + debug | ✅ IPC/thread/shared lib, GDB | 🟢 mạnh | rapid ở B3 |
| Kernel space + driver | ✅ driver, DT, **5.10→6.12** | 🟢 **mạnh nhất** | móc #3 của opening |
| Driver **I2C / SPI** | ⚠️ có chạm, resume **không nêu tên bus** | 🟡 | ⭐ **B3 — cầu nối quan trọng nhất**, xem §8.A |
| Driver **Ethernet** | ❌ | 🔴 | §8.B — trả lời trung thực + khung netdev |
| **Yocto** | ❌ chỉ CMake/Makefile | 🔴 **gap lớn nhất** | §8.C — trung thực + mental model |
| **Jenkins / CI** | ❌ | 🟠 | §8.D gộp |
| **Oscilloscope / logic analyzer** | ❌ | 🟠 | §8.D gộp |
| Design subsystem · lead discussion · V&V · peer review | ✅ chất liệu có, **chưa đóng gói** | 🔴 **ăn điểm nhiều nhất** | §5 |

> 📌 **YOE 2–4** ⇒ bar coding là **cơ bản → trung bình**. Điều này *xác nhận* quyết định giới hạn DSA ở §6.

---

## 2. Ba dạng bài coding trên giấy — và gap của repo

| Dạng | Bank hiện có | Trạng thái |
|---|---|---|
| Coding problem | [`COD-001…010`](../mock-interview/bank/coding.md) — 10 bài cỡ nhỏ | ✅ đủ, đúng cỡ |
| Implement DS cơ bản | + [`COD-011/012/013`](../mock-interview/bank/coding.md) — dynamic array · hash map chaining · queue bằng 2 stack | ✅ **đã soạn 11/09**, bản mẫu compile + chạy sạch |
| **Đọc code tìm lỗi** | [`COD-014…026`](../mock-interview/bank/coding.md) — **13 snippet**, mục mới `## B` | ✅ **đã soạn 11/09**, mỗi bài có **output thật đã chạy** |

**"Trên giấy" đổi QUY TRÌNH, không đổi nội dung** — không compiler, không autocomplete, không chạy test. Luyện và chấm đúng **5 nhịp**:

1. **Hỏi spec trước khi viết** — *"mảng có sorted không? có null không? in-place hay được cấp thêm? kích thước tối đa?"*
2. **Nói approach + O(time)/O(space)** trước khi đặt bút.
3. **Viết tay** — chữ đọc được, đặt tên biến tử tế.
4. **Tự dry-run bằng bảng giá trị** — 1 ca thường + 1 ca biên. *(Đây là nhịp thay thế compiler.)*
5. **Tự nêu cái mình biết là chưa xử lý.**

> ⭐ JD ghi *"take design choice autonomously"* và *"attention to details"* ⇒ nhịp **①** và **⑤** được chấm **ngang với code**. Người viết code đúng mà không hỏi spec vẫn thua người hỏi spec rồi viết code gần đúng.

**Bộ 13 snippet bug-hunt (`COD-014…026`), đã soạn:** off-by-one · buffer overflow (`sprintf`) · `memcpy` vùng chồng lấn · trả reference tới local · thiếu `volatile` (cờ ISR) · so sánh signed ↔ unsigned · `strncpy` không NUL · shift ≥ độ rộng kiểu · thiếu `break` · Rule of Three vỡ → double free · mutex không RAII → deadlock · struct padding & misalignment · **thứ tự đánh giá đối số hàm** *(C++17 đã sửa `<<`, `[]`, `=` — nhưng KHÔNG sửa đối số hàm)*.

> 📌 **Số liệu đo thật khi dựng bộ này** (`gcc/g++ 11.4.0`): `-Wall -Wextra` bắt **3/13 ở `-O0`**, **5/13 ở `-O2`** — **8 bài compile sạch ở mọi mức**, gồm double free, deadlock, đọc ngoài mảng. Và **chính cảnh báo cũng phụ thuộc `-O`**: build debug cảnh báo *ít hơn* build release. Ba bài `COD-016/018/023` **chạy đúng ở `-O0`, hỏng ở `-O2`** — nguyên mẫu *"lab thì đúng, hiện trường thì sai"*.

---

## 3. Bản đồ phủ RESUME — không được bỏ sót dòng nào (buổi B1)

> Bank [`RES`](../mock-interview/bank/resume.md) có **22 câu** nhưng neo vào `RESUME_embedded_linux.tex` (bản cũ). Cột **Việc** = phải làm gì cho `RESUME_current.tex`.

| Dòng resume | Câu | Việc |
|---|---|---|
| SUMMARY — 3 năm, TV + enterprise display, ARM SoC | `RES-001` | ⭐ thành **opening §4**. Đã **lỗi 3 lần** ⇒ ưu tiên #1 |
| Skills: IPC (mq, shm), thread, shared lib | `RES-005` | ✅ |
| Skills: **AI-assisted development** | `RES-016` | 🔴 **0 điểm.** JD có *"python, scripting"* ⇒ sẽ bị hỏi *"code AI sinh thì kiểm soát chất lượng sao"*. Thuộc **một câu + một ví dụ** |
| Skills: CMake/Makefile/Git/Perforce | — | + Jenkins khái niệm (§8.D) |
| Core: display enhancement (dimming/FRC/timing) | `RES-007` | ✅ |
| Core: **1 interface đa chipset + function-pointer table** | `RES-002`, `RES-003` | ⭐ **câu mạnh nhất** — khớp thẳng JD *"design complex subsystems"* |
| Core: port driver + DT + **kernel 5.10→6.12** giữ backward compat | `RES-004`, `RES-010` | ✅ câu nặng, ôn kỹ |
| Core: cross-layer debugging | `RES-008` | ✅ |
| Core: **công tác Hàn Quốc 2 lần/năm** | (hỏi ở B1, gộp vào `RES-031`) | ✅ đã hỏi — *"bring-up/test model chưa ra mắt, không có ở VN"* |
| S-Box: **ALS adaptive brightness không nháy** | `RES-006` | ⭐ **thêm câu về BUS của sensor** → §8.A, cầu nối duy nhất tới *"I2C/SPI driver"* |
| S-Box: sync qua POSIX mq, **1 binary 2 mode** | `RES-005` | ✅ |
| Driver load **3–4s → <0.5s**, verify 100 boot | `RES-013/014/015` | ⭐ khớp JD *"autonomous design choice"* **+ "V&V"** |
| SDM Preset (MVVM, Windows) | `RES-011`, `017–022` | ✅ |
| **Porting tool Python** | `RES-009` | xem dưới ⬇️ |
| Best Employee Q1/2026 · TOEIC 895 · DSA cert | ❌ | 🟡 1 câu ngắn; cert = **cơ bản**, nói được trong 1 câu là đủ |

### ⚠️ Porting tool — hỏi vừa đủ, nhưng **đừng hạ thấp**

JD ghi rõ *"python, scripting languages"* và *"anticipate needs… shows initiative"* ⇒ đây là dòng khớp JD **trực tiếp nhất ngoài kernel**. Họ sẽ **không** hỏi Python nâng cao. Chuẩn bị đúng **4 ý, không hơn**:

1. Bài toán thật là gì (port driver/library qua chipset: sửa build system + adapt source).
2. Tool tự động tới đâu.
3. ⭐ **Cái gì tool CỐ Ý không làm** — chỗ con người vẫn phải xem.
4. ⭐ **Làm sao verify bản port ra là đúng.**

> Ý **③** và **④** là chỗ dễ hụt nhất và cũng là chỗ ghi điểm senior. Người kể "tôi viết script tiết kiệm thời gian" thì bình thường; người nói được ranh giới tự động hoá và cách kiểm chứng thì khác hẳn.

---

## 4. 🗣️ Opening — thả 3 móc câu (75–90 giây)

**Mục tiêu không phải "giới thiệu hay".** Mục tiêu là **bạn chọn trước 3 chủ đề họ sẽ đào**, thay vì để họ tự bốc từ resume.

**① Định vị — 1 câu.** 3 năm system software C/C++ cho Smart TV & enterprise display trên SoC ARM.

**② Phạm vi dọc — đây là điểm khác biệt.** *"Tôi làm xuyên tầng: từ C++ interface của shared library, xuống HAL, xuống kernel driver nói chuyện với SoC."*

**③ Ba móc — mỗi móc đúng MỘT câu rồi DỪNG:**

| Móc | Câu thả | Cắn vào JD dòng nào | STAR sẵn ở |
|---|---|---|---|
| 🅐 **Design** | *"một interface C++ chung cho nhiều chipset — library chọn implementation lúc boot theo board config, kernel dispatch qua function-pointer table"* | *"design and implement complex software subsystems"* | `RES-002/003` |
| 🅑 **Đo lường + V&V** | *"tối ưu thời gian nạp driver 3–4s xuống dưới 0.5s — đo hai phương án rồi verify qua 100 lần boot"* | *"take design choice autonomously"* · *"drive the relation with V&V"* | `RES-013/014/015` |
| 🅒 **Kernel** | *"port driver qua các thế hệ chipset và migrate kernel 5.10 → 6.12 mà vẫn giữ tương thích ngược"* | *"Linux kernel space and driver programming"* | `RES-004/010` |

**④ Đóng bằng lý do khớp JD.** Muốn đi sâu hơn kernel/driver và làm với quy trình sản phẩm bài bản (Yocto, Jenkins, V&V).

> 🎯 **Kỷ luật: thả móc rồi IM.** Không kể hết. Ba móc trên phủ đúng ba thứ JD hỏi nhiều nhất — interviewer cắn móc nào thì bạn mở STAR 2 phút cho móc đó.
> 🇬🇧 **Bản tiếng Anh bắt buộc** — PV 50/50. Luyện: nói Việt → nói Anh → bấm giờ cả hai, mỗi bản ≤ 90s.

---

## 5. Process & communication theo JD mới (buổi B1, phần chưa từng ôn)

Năm câu gần như chắc chắn gặp, mỗi câu **một ví dụ thật + một con số**:

| Câu | JD neo vào | Chất liệu từ resume |
|---|---|---|
| *"Kể một subsystem phức tạp bạn thiết kế"* | *design complex subsystems* | 🅐 interface đa chipset + function-pointer table |
| *"Lần bạn tự quyết một design choice rồi phải bảo vệ nó"* | *take design choice autonomously… lead deep technical discussion* | 🅑 chọn giữa parallel load vs raising priority — **có số đo** |
| *"Bạn làm việc với đội test/V&V thế nào khi họ báo bug bạn không tái hiện được"* | *drive the relation with V\&V in a fruitful manner* | cross-layer debugging + 100 boot cycles |
| *"Bạn review code của người khác theo tiêu chí gì"* | *peer-reviews of solution designs and related code* | tiêu chí: đúng đắn → an toàn tài nguyên → đọc được; góp ý dựa dữ liệu |
| *"Requirement mơ hồ thì bạn làm gì"* | *solicit, generate and manage requirements* | công tác HQ: làm rõ yêu cầu trực tiếp với R\&D HQ |

> ⚠️ **Bài học #7 còn hiệu lực: STAR phải kết bằng CON SỐ.** Đo được: 0/12 câu tự đi tới Result ⇒ **2.42 điểm**; dựng khung trong một ngày ⇒ **3.42**, số câu phải probe từ 6/12 xuống **0/12**. Câu nào không có số thì tìm số trước khi vào phòng.

---

## 6. DSA — phạm vi CHỐT (buổi B2)

> ✅ Cert *"Samsung Software Certificate (DSA) — Advanced"* xác nhận nội dung **cơ bản** (array/list/stack/queue/hash, sort, search) ⇒ **không mâu thuẫn** với phạm vi dưới. Cần nói được cert test gì trong **một câu** nếu bị hỏi.

| ✅ Phải trơn tay | 🟡 Nói được, không cần code trơn | ❌ Bỏ hẳn |
|---|---|---|
| Array/string tại chỗ · linked list (reverse, cycle, merge) · stack/queue · **ring buffer** · hash map (dùng + nguyên lý collision) · two-pointer · sliding window · bit manipulation · sort cơ bản + `std::sort`/`lower_bound` | Binary search trên mảng sorted · BFS/DFS đơn giản · duyệt cây iterative · **đệ quy 1 tầng** (reverse list đệ quy) | **DP** · backtracking · graph nâng cao (Dijkstra/union-find) · segment tree · trie · cây tự cân bằng |

> ⚠️ **Sửa so với ý ban đầu:** *recursion* **không** nằm nhóm bỏ. Đảo linked list đệ quy và duyệt cây cơ bản là câu **rẻ, hay ra**. Thứ đáng bỏ là **DP + backtracking + graph nâng cao**.

**Cộng thêm phần embedded thực sự đo** (quan trọng ngang thuật toán): `vector` vs `list` vs `map` vs `unordered_map` — cache locality, cấp phát động, footprint · vì sao embedded ngại `std::map`/`std::any` · khi nào mảng tĩnh thắng container · `reserve()`.

---

## 7. ⚙️ C++17 — ôn theo BA TẦNG, không theo feature list (buổi B3)

Họ không đo bạn thuộc feature. Họ đo: **code có an toàn tài nguyên không**, và **bạn có chọn feature vì lý do kỹ thuật không**.

### Tầng 1 — NỀN (~70% điểm, bị hỏi dưới nhãn "C++17")
RAII & ownership (`unique_ptr`/`shared_ptr`/`weak_ptr`; **khi nào raw pointer vẫn đúng** — non-owning observer) · rule of 0/3/5 · move semantics và **khi nào move KHÔNG xảy ra** · const-correctness & lifetime/dangling · virtual destructor + chi phí vtable · exception vs codebase tắt exception (câu đánh đổi rất hợp embedded).
→ Chỗ bạn **mạnh nhất** vì bạn viết shared-library interface. **Đừng ôn ít phần này để dành giờ cho feature mới.** [02-modern-cpp](../../02-modern-cpp/) · bank `CPP-A/C/D`.

### Tầng 2 — C++17 ESSENTIAL (mỗi cái phải kèm *"tôi dùng ở đâu"*, không phải định nghĩa)

| Feature | Vì sao embedded quan tâm | Bẫy phải biết |
|---|---|---|
| `std::optional` | thay sentinel `-1`/`nullptr`/bool out-param | ✅ **bạn đã dùng thật ở SDM** |
| `std::string_view` | truyền chuỗi **không cấp phát**, hợp parse config | 🔴 **dangling** + **không đảm bảo NUL** → đừng đưa thẳng vào API C |
| structured bindings | `auto [it, ok] = map.insert(...)`, duyệt map | — |
| `if constexpr` | bỏ SFINAE, gọn trong template HAL | — |
| `std::variant` + `visit` | thay tagged union kiểu C | cost of `visit` |
| `std::scoped_lock` | khoá **nhiều** mutex không deadlock | thay `lock_guard` + `std::lock` |
| `std::shared_mutex` | reader/writer | writer starvation |
| `[[nodiscard]]` `[[maybe_unused]]` `[[fallthrough]]` | ⭐ rất hợp **API library của bạn** | — |
| inline variable · nested namespace | header-only constant, hết ODR trick | — |
| **guaranteed copy elision** | thay đổi **thật** của C++17 | trả về type không copy/move được |
| `std::byte` · `std::clamp` · `std::size` | tiện, hợp embedded | — |

### Tầng 3 — chỉ NHẬN DIỆN, không đào
PMR · `std::launder` · parallel algorithms · fold expression · CTAD · `std::any` (**và vì sao embedded tránh** — cấp phát động).

### 🔴 Rủi ro cụ thể — xử lý TRƯỚC buổi PV

Resume ghi *"Modern C++ (11/14/17)"*. Ghi nhận 07/09: app SDM build tới C++20 và thứ C++17 **duy nhất dùng thật là `std::optional`**. Bị hỏi *"kể C++17 bạn dùng trong công việc"* mà chỉ có `optional` là **hụt ngay**. Chọn một:

1. **Rà lại codebase Samsung** tìm cho ra **3 ví dụ thật** (`string_view`? structured binding? `scoped_lock`? `[[nodiscard]]`?).
2. **Trả lời thẳng:** *"Codebase chính chốt ở C++11 do toolchain SoC; phần tôi tự chủ được thì dùng `optional`, structured binding. C++17 tôi nắm và đọc được nhưng chưa áp diện rộng."* → **Trung thực + nêu cái đã áp luôn tốt hơn kể vẹt** — JD có *"attention to details, open minded"*, họ nhận ra ngay khi bạn đọc thuộc lòng.

### C thuần — phần kernel driver (JD ghi "C/C++ 17")
`volatile` · bit field · alignment & padding · `static`/`inline` · bẫy macro · **integer promotion** · `restrict`.
→ Đây **cũng chính là nguyên liệu** của bộ snippet bug-hunt §2. Ôn một lần, dùng hai chỗ.

---

## 8. Bốn câu trả lời cho gap JD — soạn sẵn, NÓI TO, bấm giờ (buổi B3)

> **Khung chung cho mọi gap:** ① nói thẳng chưa làm · ② chứng minh **có mental model** · ③ nối về **việc thật tương đương** đã làm · ④ cam kết cụ thể. Bốn nhịp, ≤ 60 giây.
> ⚠️ **Bài học #8:** đừng gộp nhầm thành *"em có làm rồi"* khi mới chỉ đọc/quan sát. Interviewer không phân biệt được "gộp nhầm" với "bịa".

### 🅐 I2C / SPI — **không phải gap, mà là CẦU NỐI chưa khai thác** ⭐

Đây là việc **giá trị cao nhất** của buổi B3. JD ghi rõ *"device drivers for I2C, SPI, Ethernet"*; resume của bạn **không nêu tên bus nào** dù ambient light sensor của S-Box gần như chắc chắn đi qua **I2C**.

**Phải chốt trước buổi PV:** sensor đó nói chuyện qua bus gì · ai viết driver cho nó (bạn / vendor / có sẵn trong kernel) · bạn chạm tới tầng nào (đọc register qua sysfs? gọi API? sửa device tree node cho nó?). Có câu trả lời cụ thể ⇒ **một gap JD biến thành một điểm mạnh**.

**Nền lý thuyết cần chắc** ([bus-protocols.md](../../05-drivers-device-tree/bus-protocols.md), bank `BUS-*`): I2C 2 dây/địa chỉ/ACK/**clock stretching** · SPI 4 dây/CPOL-CPHA/full-duplex/chip-select · **khi nào chọn cái nào** (số dây, tốc độ, số slave, khoảng cách) · driver model Linux: `i2c_driver`/`spi_driver` + `probe()` + match qua **device tree compatible** · vì sao không được ngủ/`msleep` trong ngữ cảnh atomic.

### 🅑 Ethernet driver — chưa làm
*"Sản phẩm tôi làm không có driver Ethernet — phần mạng ở tầng userspace, S-Box sync qua POSIX message queue. Về driver mạng tôi nắm khung: netdev đăng ký qua `net_device_ops`, `ndo_start_xmit` đẩy `sk_buff` xuống, **NAPI** để chuyển từ interrupt-per-packet sang polling khi tải cao tránh interrupt storm, DMA ring cho TX/RX. Mô hình đó gần với thứ tôi đã làm ở driver display: đăng ký, probe theo device tree, xử lý interrupt và bottom half."*
→ Nền: [driver-basics.md](../../05-drivers-device-tree/driver-basics.md) · [lkd/02-interrupts-bottomhalves](../../15-book-summaries/lkd/02-interrupts-bottomhalves.md).

### 🅒 Yocto — gap lớn nhất, trả lời bằng MENTAL MODEL
*"Tôi chưa dùng Yocto trong sản phẩm — build ở chỗ tôi là CMake/Makefile cộng hệ nội bộ. Mô hình thì tôi nắm: BitBake đọc **recipe** mô tả fetch/configure/compile/install một package; **layer** tách phần BSP của vendor khỏi distro và khỏi ứng dụng để nâng cấp độc lập; **bbappend** sửa recipe của layer khác mà không phải fork nó; **sstate-cache** cache kết quả từng task nên build lại chỉ chạy phần đổi; **SDK** sinh toolchain + sysroot cho người viết app. Việc tương đương tôi đã làm là cross-compilation và quản lý sysroot bằng tay, cộng port driver qua nhiều chipset — cùng bài toán 'một source, nhiều board', khác công cụ."*
→ Nền: [yocto.md](../../06-build-systems/yocto.md) · [cross-compilation.md](../../06-build-systems/cross-compilation.md) · bank `BLD-*`.
⚠️ **Học TƯ DUY, không cú pháp.** Họ hỏi *"layer để làm gì", "sstate giải quyết gì", "DEPENDS vs RDEPENDS"* — không bắt viết recipe.

### 🅓 Oscilloscope / logic analyzer + Jenkins — gộp một câu ngắn
Scope: *"tôi debug ở tầng phần mềm — GDB, dmesg/printk, cross-layer. Chưa tự cầm scope; khi cần đo tín hiệu tôi làm việc với đội hardware. Tôi hiểu dùng nó để làm gì: xác nhận tín hiệu bus có thật sự ra chân không, đo timing, tách bạch lỗi phần mềm với lỗi phần cứng — đúng lúc log không kết luận được."*
Jenkins: nói bằng khái niệm CI đã có ([ci-and-test-farm.md](../../06-build-systems/ci-and-test-farm.md)) — gated check-in, build matrix, smoke test, test farm — và nối về việc thật: **verify qua 100 boot cycles** chính là tư duy automated regression.

---

## 9. 📌 Bài học còn hiệu lực (giữ lại từ plan cũ — 5 điều đổi cách chạy sprint này)

| # | Bài học | Áp vào đâu |
|---|---|---|
| 1 | **Nền chắc, hụt ở vận dụng** (T1 3.67 vs T2 2.1) ⇒ đọc thêm tài liệu **không** chữa được | Cả sprint: không đọc mới, chỉ luyện nói + viết |
| 3 | **Lỗi ĐÓNG GÓI ≠ lỗ hổng kiến thức** — cùng kiến thức, đổi khung (kỹ thuật → resume) thì không truy xuất được. Chữa bằng **nói to bấm giờ** | B1 toàn bộ |
| 4 | **Thuộc bài ≠ hiểu bài** — nói ra một thuật ngữ là **mời interviewer hỏi vào đúng nó** | §7 tầng 2: mỗi feature phải kèm ví dụ thật |
| 7 | **STAR phải kết bằng CON SỐ** — 0/12 câu tới Result ⇒ 2.42; có khung ⇒ **3.42** | §5 + §4 ba móc |
| 8 | **Khẳng định không đối chiếu được** — từng nói *"đã build minimal image trên BBB"* trong khi thực tế chưa. **Gộp nhầm**, không phải bịa — nhưng interviewer không phân biệt | §8 toàn bộ, nhất là 🅐 |

---

## 10. ❄️ Đóng băng — việc dài hạn tạm dừng, KHÔNG mất

Ba bộ lab của plan cũ **hoãn tới sau buổi PV** (không kịp và không sinh giá trị cho 3–4 ngày tới). Ghi lại trạng thái để không biến mất trong im lặng:

- 🧪 **Lab BSP phần cứng (BeagleBone)** — `BSP-031…038`: **1/8 xong** (`BSP-038` sstate ✅ 06/09) · `BSP-031` 🟡 bước ② xong. Setup: [lab-setup.md](../lab-setup.md).
- 🧪 **Lab DBG** — `DBG-030…041`: **0/12**.
- 🧪 **Lab DP** — 5 bài ở [A2 §7](../../11-design-patterns/in-practice/A2-cpp-interface-hal.md): **0/5**.
- 📚 **Nợ chất lượng bank** — ~68 câu 🟠🔴 đáp án < 700 ký tự; tự co theo [config §1 Bước 4](../mock-interview/config.md).
- 📉 **Độ phủ bank** — lần đo 06/09: `178/457 = 38%`. **Không phải mục tiêu của sprint này**; đừng để nó lái quyết định.
