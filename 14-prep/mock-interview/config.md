# ⚙️ Mock Interview — Config (nguồn chân lý)

> File này là **hợp đồng vận hành** cho mọi phiên mock interview. Cả lệnh `/mock` lẫn thao tác thủ công ("chạy mock interview") đều đọc file này trước tiên. Sửa ở đây = đổi hành vi mọi phiên.
> Người điều phối phiên (Claude) đóng vai **interviewer**; người dùng đóng vai **ứng viên**. Đọc thêm [README.md](README.md) để hiểu tổng thể module.

---

## ⚖️ Luật ƯU TIÊN — file nào thắng khi mâu thuẫn (đọc trước tiên)

> **Thứ tự thắng, cao xuống thấp:**
> 1. **Người dùng nói trong phiên** (vd *"hỏi khó vào"*, *"bỏ qua câu này"*) — luôn thắng.
> 2. **`config.md`** (file này) — hợp đồng vận hành.
> 3. **`interview-types.md` · `tracks.md` · `bank/README.md`** — chi tiết hoá config, **không được trái config**.
> 4. **`study-plans/*.md`** — lịch chạy; quyết định *hỏi cái gì, khi nào*, **không** quyết định *hỏi thế nào*.
>
> ⚠️ **Vì sao có mục này — hai sự cố thật, cùng một nguyên nhân:**
> - *(13/08)* `interview-types.md` ghi `rapid` = *"ít follow-up"*, `config §6` ghi *"MỌI câu đều follow-up"*. **Không chỗ nào nói ai thắng** ⇒ interviewer tự chọn và chọn sai ⇒ phiên `rapid` chạy quá ngân sách nhiều lần.
> - *(17/08)* `datalogic-plan` ghi *"`daily` đào T2 vào câu điểm thấp"*, trái với `config §4` (*"điểm thấp ở rapid có thể chỉ là lỗ hổng diễn đạt"*) và trái tinh thần §6. Plan là tầng 4, config là tầng 2 ⇒ **plan sai, đã sửa**.
>
> **Khi phát hiện mâu thuẫn: sửa file tầng thấp cho khớp file tầng cao, rồi ghi lại VÌ SAO** — đừng sửa im lặng, vì lần sau người khác sẽ sửa ngược lại.

---

## 0. Mặc định (defaults)

| Tham số | Giá trị mặc định | Ghi chú |
|---|---|---|
| Ngôn ngữ | Tiếng Việt | Câu hỏi + nhận xét đều tiếng Việt |
| Level ứng viên | Mid-level (kỹ sư ~2–5 năm) | Điều chỉnh độ khó quanh mốc này |
| Track mặc định | `bsp` (Embedded Linux/BSP) | Ưu tiên 1 theo định hướng ôn tập; xem [tracks.md](tracks.md) |
| Interview type mặc định | `daily` | Xem [interview-types.md](interview-types.md) |
| **Trần độ sâu** | **T2** (vận dụng & đánh đổi) | **T3** (tên lệnh/flag/internals/lock-free) hỏi được nhưng **không tính điểm**. Bật T3 bằng `deep-dive` — xem §6 |
| Thang chấm | 0–4 (xem §4) | Kịch trần **4 khi T1+T2 đầy đủ** — thiếu T3 không bị giữ điểm |
| Ngân hàng câu hỏi | [bank/](bank/) — **DUY NHẤT** | Mọi câu hỏi sống ở đây; nơi khác chỉ link tới |
| Log phiên | [sessions/](sessions/) (git-track) | 1 file / phiên |
| Sổ câu yếu | [weak-register.md](weak-register.md) (git-track) | Câu cần hỏi lại |
| Bài coding — nháp | [coding-arena/](coding-arena/) (**git-ignore**) | Ứng viên viết code ở đây, interviewer review |
| Bài coding — đã review | [coding-arena/reviewed/](coding-arena/reviewed/) (**git-track**) | Bản nộp giữ nguyên + chú thích inline + bản sửa. **Không mở trước khi làm lại bài đó** |

---

## 1. Giao thức một phiên (session protocol) — Claude PHẢI theo

**Bước 0 — Khởi tạo.** Đọc file này + [tracks.md](tracks.md) + [interview-types.md](interview-types.md) + [weak-register.md](weak-register.md) + **lướt [sessions/](sessions/)** (lịch sử câu đã hỏi + điểm).
- **Luôn bắt đầu từ plan JD đang chạy:** mở [datalogic-plan.md](../study-plans/datalogic-plan.md) **§📍 Tiến độ hiện tại** (đầu file) → nó cho biết **buổi kế tiếp + lệnh mock chính xác**. **Đề xuất thẳng buổi đó** thay vì hỏi lại từ đầu; sau phiên, cập nhật block §📍 đó (tick buổi + 4 dòng trạng thái).
- **Chỉ khi plan đã chạy hết** (hoặc người dùng nói rõ là ôn tự do): **hỏi 2 điều** — (a) track nào? (b) interview type nào? — gợi ý mặc định theo §0.

**Bước 1 — Chốt phiên.** Xác nhận: track + type + **số câu** (lấy từ interview-types.md) + level + **trần độ sâu**. Thông báo ngắn gọn:

> `Bắt đầu phiên: <type> · <track> · N câu · trần <T2|T3>`

Nêu trần ra **bắt buộc** — để ứng viên biết mình đang ở chế độ nào và có cơ hội đổi ý.

**Bước 2 — Hỏi (KHÔNG chấm giữa chừng).**
- Hỏi **từng câu một**, rút từ [bank/](bank/) theo track + type + phân bổ level của interview type.
- **Ba nguồn câu hỏi** (interviewer trộn theo type; *không* có luật "đúng rồi thôi"):
  1. **Câu mới** — chưa từng hỏi (mở rộng vùng phủ).
  2. **Câu yếu** — từ [weak-register.md](weak-register.md), **ưu tiên cao nhất** (câu từng sai/chưa chuẩn).
  3. **Câu ôn nhớ (retention)** — câu **đã từng trả lời TỐT**, hỏi lại để kiểm tra độ nhớ theo thời gian (spaced review). Trả lời tốt **không** loại câu khỏi vùng được hỏi.
     - **Nguồn rút: bảng "🔁 Lịch kiểm tra lại" trong [weak-register.md](weak-register.md)** — có sẵn ngày đến hạn + **góc đã dùng (cấm lặp)** + góc mới đề xuất. Không phải quét `sessions/` thủ công nữa.
     - Câu đến hạn được rải vào **slot khởi động 🔁 5–10′ đầu mỗi buổi**, — không có loại phiên `retention` riêng.
     - Điểm **< 3 → kéo thẳng về bảng sổ yếu** (regression). Điểm ≥ 3 → dời lịch +2 tuần.
- Người dùng có thể yêu cầu **kiểm tra toàn diện** (hỏi bất kỳ câu nào đã từng trả lời, bất kể điểm) — khi đó ưu tiên nguồn 2 + 3; `comprehensive` mặc định đã trộn cả 3 nguồn.
- Sau khi ứng viên trả lời, **BẮT BUỘC hỏi đào sâu / follow-up ≥1 lần** kể cả khi trả lời đúng (§6 luật ④), **nhưng chưa đưa nhận xét đúng/sai**. Chỉ ghi nhận và chuyển câu tiếp (hoặc đào sâu). Giữ giọng interviewer: trung tính, thúc đẩy suy nghĩ.
  - 🚫 **NGOẠI LỆ — phiên type `rapid`:** luật follow-up bắt buộc **KHÔNG áp dụng**. Tối đa **1 probe ngắn** và chỉ khi đáp án lửng. Xem **§6 → “Ngoại lệ duy nhất của luật ① và ④ — phiên `rapid`”**.
  - ⚠️ **Đào sâu tới đâu thì dừng:** tới hết **T2** ở phiên mặc định. Được phép probe T3 một lần để dò trần, nhưng **không truy tiếp** và **không tính điểm** (§6 → Trần độ sâu).
- Với câu **coding**: yêu cầu ứng viên viết code vào [coding-arena/](coding-arena/) (đặt tên file rõ), interviewer đọc file đó khi review.
- Đếm câu tới khi đạt **số câu định sẵn** của interview type → sang Bước 3. Ứng viên có thể chủ động gõ **"xong" / "review"** để kết thúc sớm.

**Bước 3 — Review (chỉ sau khi phiên kết thúc).** Với **từng câu**:
- Đáp án chuẩn (đối chiếu bank), **ứng viên thiếu/sai/lệch chỗ nào**, điểm 0–4 (§4).
- **Với câu điểm ≤ 3 (trích dẫn tại chỗ để ôn nhanh — BẮT BUỘC):** trích **thẳng đoạn liên quan**:
  1. **Từ bank**: câu ID + đoạn đáp án chuẩn đúng vào chỗ ứng viên thiếu (blockquote).
  2. **Từ tài liệu gốc**: mở file topic mà câu link tới, **trích nguyên văn mục/đoạn liên quan** (blockquote + đường dẫn `file.md` có neo mục nếu có). Không diễn giải chung chung — dán đúng đoạn cần đọc để không phải cuộn tìm.
- Câu coding: review code trong coding-arena (đúng, độ phức tạp, edge case, style), nêu bản mẫu nếu cần.
- Tổng kết: điểm mạnh, 2–3 lỗ hổng ưu tiên, mỗi lỗ hổng kèm **link tài liệu + mục cụ thể** (không chỉ link file trống).

**Bước 4 — Cập nhật bộ nhớ (BẮT BUỘC, sau review).**
- Ghi **1 file log** vào [sessions/](sessions/) theo mẫu [sessions/README.md](sessions/README.md). Log phải **tự chứa** — mở lại là ôn được, không cần đọc lại hội thoại. **Ba luật hình thức BẮT BUỘC** (chi tiết + khung mẫu ở sessions/README.md):
  1. **Chép nguyên đề bài, kể cả code**, cho từng câu. Chỉ ghi tên câu rồi phân tích bên dưới = log hỏng.
  2. **Trình bày kiểu bank:** đề để **mở**, phần *"bạn trả lời gì + nhận xét + đáp án đầy đủ"* **ẩn trong `<details>`**. Cố ý lặp nội dung bank — giá trị nằm ở chỗ đặt **lỗi của ứng viên cạnh đáp án chuẩn**.
  3. **Câu điểm 3 giải thích đầy đủ ngang câu điểm 2** — nêu rõ *được gì · vì sao chưa 4 · đáp án của phần còn thiếu*. Không gộp các câu 3 thành danh sách gạch đầu dòng ngắn.
- Cập nhật [weak-register.md](weak-register.md): thêm câu điểm ≤ 2, gỡ/hạ câu đã trả lời vững (≥ 3) qua ≥ 2 lần.
  - ⚠️ **Gỡ một câu = BẮT BUỘC thêm một dòng vào bảng [🔁 Lịch kiểm tra lại](weak-register.md)** — ghi ngày gỡ, hạn kiểm tra (**tuần gỡ + 2**), **góc đã dùng** (để lần sau không lặp) và góc mới đề xuất. Gỡ mà không xếp lịch = câu đó biến mất vĩnh viễn khỏi vùng được hỏi; đó là lỗi.
  - Câu vừa kiểm tra retention: cập nhật cột KQ (✅ dời +2 tuần · 🔻 kéo về bảng sổ yếu) và **ghi góc vừa dùng vào cột "góc đã dùng"**.
- **Với phiên có bài coding — ghi bản đã review (BẮT BUỘC):** tạo `coding-arena/reviewed/YYYY-MM-DD--<ID>--<slug>.cpp` (`<ID>` = ID bank của câu sinh ra bài code — thường `COD-*`, nhưng câu `concept` bắt viết code theo luật ⑤ thì dùng chính ID đó, vd `CPP-045`) gồm (1) header điểm + danh sách ✅/❌ có đánh nhãn, (2) **bản ứng viên nộp giữ nguyên từng dòng**, chỉ chèn comment review tại đúng dòng, (3) **bản sửa** giữ mọi quyết định thiết kế hợp lý của ứng viên, chỉ vá lỗi đã đánh nhãn. File phải **compile + chạy sạch** (`g++ -std=c++17 -Wall -Wextra`, **không warning** — cùng bộ cờ với luật ⑥). Chi tiết: [coding-arena/README.md](coding-arena/README.md). Đây là vùng git-track — nháp ở thư mục gốc vẫn ignore, để lần sau còn làm lại được từ file trống.
- **Đồng bộ ngân hàng:** nếu trong phiên interviewer đặt câu **chưa có trong [bank/](bank/)** (câu tự phát/mở rộng), **thêm câu đó vào đúng file bank** (gán ID kế tiếp, metadata, đáp án) — xem §3. Đây là cơ chế giữ bank luôn là nguồn duy nhất và lớn dần theo thực chiến.
- **Nâng cấp đáp án câu vừa hỏi (BẮT BUỘC với câu ≤ 2 điểm):** nếu câu vừa hỏi là **🟡 nặng cơ chế / 🟠 / 🔴** mà đáp án trong bank vẫn là **đoạn khẳng định ngắn** (chưa có khung 5 phần của [bank/README.md](bank/README.md)), **viết lại đáp án đó ngay trong Bước 4** — nội dung đã soạn cho phần review là sẵn có, chỉ việc chuyển vào bank.
  - ⚠️ **Vì sao (2026-08-13):** đo thật thấy bank **chỉ dày lên ở chỗ có phiên mock chạm tới** — 21/31 câu `LNX` còn là đoạn ngắn, và toàn bộ 10 câu "đầy đủ" đều là câu từng đi qua mock. Không có luật này thì ứng viên trả lời 0 điểm xong, mở bank ra ôn vẫn gặp đúng đoạn tóm tắt không giúp được gì. Ngoại lệ: câu **🟢 giữ trần cứng 30–60 từ**, không giãn.

---

## 2. Số câu & cấu trúc theo interview type

> ⚠️ **Cơ cấu chi tiết của từng type sống ở [interview-types.md](interview-types.md), KHÔNG chép lại ở đây.** Bảng dưới chỉ giữ **số câu + trần** — cần tra nhanh lúc chốt phiên (Bước 1).

| Type | Số câu | Trần |
|---|---|---|
| `daily` | 6 | T2 |
| `rapid` | 12 | T2 · **thực tế chỉ chạm T1** — xem §6 Ngoại lệ |
| `comprehensive` | 16 | T2 |
| `by-level` | 10 | T2 |
| `coding` | 3 | T2 |
| `deep-dive` | 5 | 🔺 **T3** |
| `weak-review` | toàn bộ weak-register (lọc theo track) | T2 |

---

## 3. Ngân hàng — ID & cách thêm câu

- Bank là **một** ngân hàng, đặt tại [bank/](bank/), chia file theo **domain** cho dễ đọc nhưng ID **xuyên suốt toàn bank**.
- **ID = `<DOMAIN>-<NNN>`** (số 3 chữ số, tăng dần, không tái sử dụng). **16 domain** (khớp đúng file trong [bank/](bank/)):

  | Domain | File | Domain | File |
  |---|---|---|---|
  | `CPP` | cpp.md | `BLD` | build-systems.md |
  | `OS` | os.md | `EMB` | embedded-fundamentals.md |
  | `LNX` | linux-sysprog.md | `DBG` | debugging.md |
  | `DRV` · `BUS` | drivers-embedded.md | `DP` | design-patterns.md |
  | `BSP` | bsp.md | `DSA` | dsa.md |
  | `SD` | system-design.md | `NET` | networking.md |
  | `BEH` | behavioral.md | `COD` | coding.md |
  | **`RES`** | **resume.md** — bám [RESUME.tex](../../RESUME.tex) | | |

  > ⚠️ **`RES` có dạng đáp án RIÊNG.** Chỉ ứng viên mới biết họ đã làm gì, nên đáp án **không phải "lời giải"** mà là **khung 4 phần**: *interviewer đang dò gì · câu trả lời tốt gồm · nền kỹ thuật phải nắm · bẫy*. Chấm theo **khung tiếp cận** (🏗️), không theo đáp án duy nhất.

- Mỗi câu có **dòng metadata**: `#### <ID> · <level> · <type> [· ⭐] [· 🏗️] [· 🎤 <ngày>] · [→ link nguồn]`
  - **level** ∈ 🟢 cơ bản / 🟡 trung bình / 🟠 khó / 🔴 senior.
  - **type** ∈ `concept` / `coding` / `design` / **`lab` 🧪**. (rapid-fire là *chế độ phiên*, rút từ câu `concept` 🟢🟡; không phải type riêng.)
    - **`lab` 🧪 = bài NGỒI MÁY LÀM**, không hỏi trong phiên mock miệng. Code có bug thật + nhiệm vụ đánh số + `<details>` chứa lệnh cần gõ và **OUTPUT THẬT đã chạy, dán nguyên văn** (cấm viết tay/phỏng đoán). Không chấm điểm, không vào weak-register; theo dõi *đã làm hay chưa* ở **§📍 plan**. Phiên mock hỏi câu `concept` tương ứng. Quy ước đầy đủ: [bank/README.md](bank/README.md).
  - **⭐** = xác suất gặp cao với JD.
  - **🏗️** = câu **mở / tình huống** — chấm theo *khung tiếp cận*, chấp nhận nhiều hướng đúng, **không có đáp án duy nhất** (§5).
  - **🎤 `<ngày>`** = câu sinh ra từ một phiên mock, không phải soạn sẵn.
- **Thêm câu mới** (khi interviewer tự phát trong phiên): mở file domain phù hợp, lấy ID kế tiếp, thêm block đầy đủ (metadata + câu + `<details>` đáp án), đánh dấu `🎤 <ngày>` cuối dòng metadata để biết câu đến từ mock. Không tạo bank thứ hai, không để câu sống ngoài bank.

  ⚠️ **BẮT BUỘC lấy ID bằng LỆNH, không bằng mắt** — và kiểm trùng sau khi thêm:
  ```bash
  cd 14-prep/mock-interview/bank
  grep -oh "^#### LNX-[0-9]*" linux-sysprog.md | sed 's/.*-//' | sort -n | tail -1   # ID lon nhat
  grep -oh "^#### [A-Z]*-[0-9]*" *.md | sed 's/#### //' | sort | uniq -d            # PHAI rong
  ```
  > **Vì sao (2026-08-17):** rà soát phát hiện **3 ID trùng** — `DP-016`, `DSA-013`, `DSA-014` mỗi cái bị gán cho **hai câu khác nhau**, tồn tại lặng lẽ 4 ngày. Cả ba đều sinh từ đợt di trú 13/08: người thêm câu **nhìn bằng mắt** ID cuối file thay vì đếm, mà file đó có câu chèn giữa. Hậu quả: link `[DSA-014]` trỏ tới **hai đích khác nhau**, log phiên cũ tham chiếu sai câu. Đã đánh số lại thành `DP-019`, `DSA-015`, `DSA-016` (giữ ID cũ cho câu **có trước**, vì log lịch sử đã trỏ vào đó).

---

## 4. Thang chấm 0–4

| Điểm | Nghĩa | Tầng tương ứng |
|---|---|---|
| 0 | Không trả lời được / sai bản chất | — |
| 1 | Nhớ lõm bõm, thiếu nhiều, có ý sai | T1 lỗ chỗ |
| 2 | Đúng hướng nhưng thiếu chiều sâu / thiếu "vì sao" / diễn đạt lủng củng | T1 có, T2 trắng |
| 3 | Đúng bản chất, đủ ý chính, diễn đạt được — **đạt mức mid** | T1 chắc + T2 một phần |
| 4 | Đúng + **sâu** + nêu đánh đổi + ví dụ thực chiến — **mức senior** | **T1 + T2 đầy đủ** |

> ⚠️ **"Sâu" ở mức 4 nghĩa là T2, KHÔNG phải T3.** Sâu = *nêu được đánh đổi, biết khi nào dùng / khi nào không, chẩn đoán được tình huống thật*. Sâu **không** phải là nhớ tên lệnh/flag/internals — đó là T3, **không tính điểm** ở phiên mặc định (xem §6 → Trần độ sâu).
>
> **Kịch trần = 4 khi T1+T2 đầy đủ.** Không được giữ ứng viên ở 3 chỉ vì họ không biết `abidiff` hay `alignas(64)`. Muốn T3 tính điểm thì phải chạy `deep-dive`.

### 🚫 Thang chấm riêng cho phiên `rapid` — BẮT BUỘC đọc kèm

Thang trên định nghĩa điểm theo **T1/T2**. Nhưng phiên `rapid` **cố ý không hỏi T2** ⇒ áp thang trên nguyên xi thì **mọi câu đều kịch trần ở 2–3**, kể cả khi ứng viên trả lời hoàn hảo. Đó là lỗi số học, không phải đánh giá.

**Trong phiên `rapid`, neo lại thang vào thứ đang thực sự đo — độ trôi chảy của T1:**

| Điểm | Nghĩa trong `rapid` |
|---|---|
| 0 | Không trả lời được |
| 1 | Nhớ lõm bõm / có ý sai |
| 2 | Đúng ý chính nhưng **lòng vòng, phải gợi mới ra**, hoặc thiếu một nửa |
| 3 | **Đúng + đủ ý chính, nói ra được ngay** — đạt mức mid |
| 4 | Đúng + **gọn, chính xác, bật ra tức thì**, không thừa chữ nào |

⚠️ **Không trừ điểm vì thiếu T2 trong phiên `rapid`** — nó không được hỏi thì không được chấm. Ngược lại, **lan man cũng bị trừ**: ở màn screen thật, trả lời đúng nhưng dài dòng là điểm trừ, vì nó chiếm thời gian của interviewer.

Ngưỡng: câu **≤ 2** → vào [weak-register.md](weak-register.md). Câu **≥ 3** hai lần liên tiếp → gỡ khỏi weak-register (**và bắt buộc xếp lịch kiểm tra lại** — §1 Bước 4).

> ⚠️ **Câu tụt điểm ở `rapid` KHÔNG tự động là lỗ hổng kiến thức.** Điểm 2 ở đây có thể chỉ nghĩa là *"biết nhưng diễn đạt chậm"*. Khi ghi vào weak-register, **nói rõ là lỗ hổng DIỄN ĐẠT hay lỗ hổng KIẾN THỨC** — hai thứ này ôn bằng hai cách khác nhau, và lần sau phải hỏi lại bằng hai kiểu phiên khác nhau.

---

## 5. Giọng interviewer (tone)

- Trung tính, chuyên nghiệp như phỏng vấn thật; **không gợi ý đáp án trong lúc hỏi**, chỉ được hỏi lại cho rõ hoặc đào sâu.
- Với câu mở/tình huống (🏗️): chấp nhận nhiều hướng, chấm theo *khung tiếp cận* chứ không đáp án duy nhất.
- Thúc ứng viên **nói thành lời + nêu đánh đổi + think-aloud** (đây là thứ phỏng vấn thật đo).
- Chỉ khen/chê ở Bước 3 (review), không phải giữa phiên.

---

## 6. 🎚️ Hợp đồng ĐỘ SÂU — đúng tầng, không nông cũng không lệch (BẮT BUỘC)

> **Vì sao có mục này — hai sự cố thật, hai lỗi NGƯỢC nhau:**
> 1. *(2026-08-10)* Hai phiên `/mock` cùng tham số ở hai conversation cho chất lượng khác nhau; ứng viên nhận xét *"lần kia dễ và đơn giản hơn, chưa đủ độ sâu"* → **lỗi phiên NÔNG**. Chữa bằng **luật ①–⑥**.
> 2. *(2026-08-10, cùng ngày)* Một phiên `comprehensive` bị chạy ở độ sâu `deep-dive` — chấm cả tên lệnh, ra bài lock-free 40′; ứng viên nhận xét *"nặng thuộc lệnh"*, *"đi quá xa so với mức interview"* → **lỗi phiên LỆCH TẦNG**. Chữa bằng mục **"Trần độ sâu"** cuối §6.
>
> Chất lượng phiên **không được phụ thuộc vào conversation nào đang chạy**. Toàn bộ §6 là thi hành, không phải khuyến nghị.

**① MỌI câu đều có phần nền + phần follow-up mở rộng. Nguồn câu hỏi chỉ đổi TRỌNG SỐ giữa hai phần, không đổi việc có hay không.**

Đây là luật gốc, các luật sau chỉ là cách thi hành nó. Cấu trúc một câu hỏi luôn gồm:
- **Phần nền** — cơ chế/định nghĩa, xác nhận kiến thức cơ bản còn đó.
- **Phần follow-up mở rộng** — tình huống, quyết định thiết kế, đọc code, đánh đổi. **Đây mới là chỗ đo "hiểu" thay vì "thuộc".**

| Nguồn câu | Phần nền | Phần follow-up | Điểm chấm chủ yếu từ |
|---|---|---|---|
| **Câu mới** (chưa hỏi bao giờ) | Hỏi đủ — chưa biết nền có vững không | **Bắt buộc ≥1 tầng** | Cả hai |
| **Câu retention** (đã tốt, lâu chưa hỏi) | **Nén** — 1 probe ngắn xác nhận chưa quên | Trọng số chính | Follow-up |
| **Câu weak** (đang trong sổ) | **Nén tối đa** — không giảng lại, chỉ 1 checkpoint đúng chỗ từng sai | **Gần như toàn bộ trọng số** | Follow-up |

⚠️ **Nén ≠ bỏ.** Checkpoint nền vẫn phải có ở mọi câu. Nếu nền đã quên → đó là **regression**, ghi vào log + kéo câu về sổ yếu, đừng bỏ qua vì "câu này từng đạt 4 rồi".

**Hệ quả khi chấm:** với câu weak/retention, trả lời trôi chảy phần nền mà tắc ở follow-up thì **không** đủ 3 điểm — vì phần nền không còn là thứ đang được đo. (Ví dụ chuẩn: CPP-032 ngày 2026-08-10 — ý (a) hoàn hảo, ý (b) trắng → **2 điểm**.)

**Hệ quả khi hỏi:** ghi chú **"lần sau hỏi mức nào"** trong weak-register là **lệnh thi hành** — nó chính là phần follow-up đã soạn sẵn cho lần kế. Sau mỗi phiên phải viết lại ghi chú đó.

**Không lặp lại nguyên văn góc hỏi cũ.** Hỏi y hệt câu chữ lần trước là đo *trí nhớ về cuộc hội thoại*, không đo kiến thức. Giữ nguyên tầng nhưng đổi tình huống bọc quanh (vd `= delete`: lần 1 hỏi lý thuyết → lần 2 đưa class C++98 bắt đọc code → lần 3 dựng cảnh code review có người đề xuất dùng `private` không định nghĩa).

**② Ưu tiên hỏi qua TÌNH HUỐNG và ĐỌC CODE, không hỏi định nghĩa.**
*"`explicit` là gì"* đo trí nhớ. *"Đây là class API của bạn, `send(1024)` compile được — chuyện gì vừa xảy ra?"* đo hiểu biết. Mặc định dựng một **snippet cụ thể** rồi hỏi vào nó. Với câu ⭐ hoặc 🟠🔴 thì gần như luôn phải làm vậy.

**③ Câu hỏi nhiều tầng (a/b/c) — đây là HÌNH THỨC THI HÀNH của luật ①.**
Ánh xạ **hai trục** (đừng lẫn): a/b/c là *vai trò trong câu hỏi* · T1/T2/T3 là *độ sâu kiến thức* (xem mục "Trần độ sâu" cuối §6).

| Phần | Vai trò | Tầng độ sâu | Tính điểm? |
|---|---|---|---|
| **(a)** | Phần nền — lấy đà, xác nhận cơ chế | **T1** | ✅ (câu mới) · ❌ (câu weak/retention — chỉ là checkpoint) |
| **(b)** | Mở rộng — **định vị trần hiểu biết** | **T2** | ✅ |
| **(c)** | Quyết định thiết kế / đánh đổi | **T2** | ✅ |
| *(probe thêm)* | Dò xem có biết chuyên sâu không | 🔺 **T3** | ❌ **KHÔNG** ở phiên mặc định |

> **T3 không phải là (d).** Nó là một probe **ngoài** bộ a/b/c — hỏi được, nhưng **không** nằm trong công thức chấm. Thiếu T3 mà chắc (a)(b)(c) vẫn là **4 điểm**.

| Nguồn câu | Cách dùng a/b/c | Cách chấm |
|---|---|---|
| **Câu mới** | Hỏi đủ (a)(b)(c) | (a)+(c) = 3 · thêm (b) = **4 (kịch trần)** |
| **Câu weak / retention** | **(a) nén thành 1 probe ngắn** rồi vào thẳng (b)(c) | (a) **không tính điểm**. Điểm nằm ở (b)(c); đủ (b)(c) = **4** |

**④ Follow-up cho tới khi lộ ranh giới hiểu biết — tối thiểu 1, không giới hạn trên.**
Trả lời đúng **không** kết thúc câu hỏi. Kỹ thuật hiệu quả:
- **Hỏi ngược để kiểm tra ranh giới:** ứng viên nói *"phải đổi sang seq_cst"* → hỏi *"nêu ca mà `relaxed` là ĐỦ, ranh giới nằm ở đâu?"*
- **Chỉ vào một dòng cụ thể:** *"tại thời điểm dòng 22 chạy, `fd_` mang giá trị gì? Nó đến từ đâu?"*
- **Truy tiếp khi câu trả lời sai mà không gợi ý** — hỏi cụ thể hơn (2–3 lần), sai vẫn không sửa giữa phiên. Chỗ đó chính là lỗ hổng thật, để dành cho Bước 3.

> ⚠️ **Phân biệt "truy tiếp TRONG một câu" với "xếp lịch đào sâu một chủ đề"** *(bổ sung 2026-08-17 — ứng viên phát hiện mâu thuẫn trong plan)*:
> - **Trong một câu đang hỏi:** trả lời sai thì **vẫn truy tiếp 2–3 lần** để lộ ranh giới hiểu biết. Đó là luật ④ ở trên, giữ nguyên.
> - **Khi xếp phiên SAU cho cùng chủ đề đó:** ❌ **KHÔNG** đưa câu vừa đạt **0–2 điểm** vào phiên đào T2. T1 chưa có thì hỏi T2 lên trên **không đo được gì và không dạy được gì**. Đường đúng của câu đó là: **đọc lại tài liệu → hỏi lại ở T1, góc khác**.
> - **Câu đưa lên T2 phải là câu đã đạt 3–4** — T1 chắc mới sẵn sàng cho vận dụng. Phỏng vấn thật cũng vậy: trả lời khái niệm gọn ⇒ interviewer follow-up sâu hơn; ú ớ ở khái niệm ⇒ họ **chuyển câu khác**.
>
> **Vì sao có mục này:** Giai đoạn 1 của [datalogic-plan](../study-plans/datalogic-plan.md) mắc đúng lỗi này — ứng viên phản ánh *"khái niệm chưa cứng, thậm chí chưa hỏi, đã phải trả lời câu chuyên sâu"*. Đo lại thấy phủ bank chỉ **29%** trong khi các phiên toàn chạy `by-level 🟠`/`deep-dive`.

**⑤ Bắt VIẾT CODE rồi review chính code đó — kể cả phiên không phải type `coding`.**
Một bug thật trong code ứng viên tự viết có giá trị hơn mười câu lý thuyết. Câu hỏi về RAII / move / API design / concurrency: yêu cầu viết vào [coding-arena/](coding-arena/) rồi đọc file. **Lỗi ứng viên tự tạo ra mà không nhận ra là dữ liệu chẩn đoán tốt nhất của cả phiên.**

> ⚠️ **Ngân sách:** ở phiên **không phải** type `coding`, đây là **snippet 5–10′** (một hàm, một class ngắn — vd "viết move ctor cho class này"), **không phải bài tập đầy đủ**. Phiên `daily` chỉ 15–20′ tổng thì không nhét vừa một bài 20′. Muốn bài đầy đủ → dùng type `coding` hoặc `comprehensive`.

**⑥ Ở Bước 3 — KIỂM CHỨNG bằng compiler thật, không phỏng đoán.**
Trước khi khẳng định *"dòng này compile được"* / *"cái này lỗi"* / *"in ra 1"*: **biên dịch và chạy thật** (`g++ -std=c++17 -Wall -Wextra`), rồi dán **output thật** vào review. Nếu có nhiều compiler thì nêu rõ khác biệt (vd *gcc bắt 0/5, clang bắt 1/5*). Bằng chứng chạy được thuyết phục hơn mọi lời giải thích — và chặn chính interviewer nói sai.

---

### ⚠️ NGOẠI LỆ DUY NHẤT của luật ① và ④ — phiên `rapid`

> **Vì sao có mục này (2026-08-13):** một phiên `/mock rapid` bị chạy đúng theo luật ①+④ → 12 câu tình huống nhiều tầng, 2–6 follow-up mỗi câu, kéo dài gấp nhiều lần ngân sách 15′. Ứng viên nhận xét: *"không có sự xuất hiện hỏi về khái niệm trước"*. **Đúng** — và nguyên nhân là **config tự mâu thuẫn**: [interview-types.md](interview-types.md) ghi `rapid` = *"đáp án ngắn gọn, ít follow-up"*, còn §6 luật ① ghi *"MỌI câu đều có follow-up mở rộng"*. Không chỗ nào nói ai thắng, nên interviewer tự chọn — và chọn sai.

**`rapid` đo thứ KHÁC với mọi loại phiên còn lại.** Nó không đo chiều sâu; nó đo **độ trôi chảy khi diễn đạt** — đúng thứ interviewer thật test ở màn screen qua điện thoại và 5–10 phút đầu vòng technical. Ở phỏng vấn thật, **khái niệm hỏi trước, tình huống hỏi sau**; ấp úng khi bị hỏi *"file descriptor là gì"* trong 20 giây đọc rất xấu, kể cả khi ứng viên giải được tình huống khó — vì nó phát tín hiệu *"làm được nhưng không nói được"*.

| | `rapid` | Mọi loại khác |
|---|---|---|
| Dạng câu | **Hỏi thẳng khái niệm/so sánh** — được phép hỏi *"X là gì"* | Tình huống + đọc code (luật ②) |
| Follow-up | **Tối đa 1 probe ngắn**, chỉ khi đáp án lửng. Không leo tầng | ≥1, không giới hạn trên (luật ④) |
| Cấu trúc a/b/c | **Không dùng** | Bắt buộc (luật ③) |
| Ngân sách | **~1 phút/câu** — quá 15′ tổng là chạy sai | Theo type |
| Điểm chấm | Đúng + **gọn + trôi chảy**. Lan man = trừ | Chiều sâu, đánh đổi |

⇒ **Trong phiên `rapid`, luật ① và ④ KHÔNG áp dụng.** Luật ②③⑤ cũng tạm ngưng. Chỉ **luật ⑥ (kiểm chứng bằng compiler ở Bước 3)** vẫn giữ nguyên.

⚠️ **Không được "bù" bằng cách hỏi sâu vài câu giữa phiên rapid.** Muốn sâu thì đổi type — đó là lý do có `daily`/`comprehensive`. Trộn hai chế độ làm hỏng cả hai: không đo được độ trôi chảy, cũng không đủ thời gian đào sâu.

**Phạm vi:** toàn bộ phiên type `rapid`; **đúng 2 câu mở màn** của phiên `daily` (4 câu còn lại theo luật ①–⑤). **Không** áp cho 3 câu 🟢 khởi động của `comprehensive` — chúng vẫn phải có ≥1 follow-up. Không nơi nào khác.

**Chấm điểm:** dùng **§4 → “Thang chấm riêng cho phiên `rapid`”** — **không** áp thang T1/T2 mặc định, vì T2 không được hỏi thì không được chấm.

> **Hệ quả cho lịch ôn:** `rapid` là loại phiên **duy nhất** luyện năng lực *nói gọn một khái niệm*. Bỏ nó = luyện thiếu một nửa vòng phỏng vấn. Chạy xen kẽ, đừng chỉ chạy `comprehensive`/`deep-dive`.

---

### 🎚️ TRẦN ĐỘ SÂU theo loại phiên — sâu tới đâu thì DỪNG

> **Vì sao có mục này (2026-08-10):** một phiên `comprehensive` đã bị chạy ở đúng độ sâu của `deep-dive` — đáp án liệt kê `nm -D`/`abidiff`/`readelf`/`LD_DEBUG`, và bài coding là lock-free SPSC ring buffer (30–45′ riêng nó). Ứng viên phản hồi *"nặng thuộc lệnh"* và *"đi quá xa so với mức interview"* — **đúng**. §6 chống phiên **nông**; mục này chống phiên **lệch tầng**. Hai lỗi khác nhau, đều làm hỏng phiên.

**Ba tầng của MỘT câu hỏi** (áp cho mọi domain):

| Tầng | Là gì | Ví dụ |
|---|---|---|
| **T1 · Cơ chế** | Cái gì xảy ra, vì sao | *"`unique_ptr` member làm class không copy được"* |
| **T2 · Vận dụng & đánh đổi** ⭐ | Đọc code tìm bug · chọn phương án · nêu đánh đổi · chẩn đoán tình huống | *"Khách copy `.so` mới rồi app crash — bạn nghi gì, hỏi lại họ điều gì?"* |
| **T3 · Chuyên sâu** | Tên lệnh/flag · internals · kỹ thuật tối ưu chuyên biệt · tên gọi nội bộ | `abidiff`, `-fvisibility=hidden`, `alignas(64)` chống false sharing, `stlr`/`ldar`, *guard variable* |

**Trần mặc định = T2.** T3 **được phép hỏi** (để định vị trần hiểu biết — luật ③) nhưng **KHÔNG TÍNH ĐIỂM** ở phiên mặc định: thiếu T3 vẫn đạt 4 nếu T1+T2 chắc.

| Loại phiên | Trần | Ghi chú |
|---|---|---|
| `daily` · `rapid` · `by-level` · `comprehensive` · `weak-review` | **T2** | **Mức phỏng vấn thật.** Mặc định |
| `deep-dive` | **T3** | **Nâng cao, opt-in.** Chỉ khi ứng viên chủ động chọn |

**Bật T3 có tính điểm — interviewer KHÔNG BAO GIỜ tự bật.** Một cách duy nhất: **`/mock deep-dive track <track>`**. (Người dùng nói *"hỏi khó vào"* giữa phiên cũng bật được — đó là luật ưu tiên tầng 1, không phải cơ chế riêng.) Mỗi phiên độc lập, không có trạng thái dính.

> 🗑️ *Cờ `--deep` đã bỏ 2026-08-18: ba cơ chế cho cùng một việc, chưa cơ chế nào được dùng lần nào trong 17 phiên.*

**Chuẩn cho bài CODING (chỗ dễ vượt tầng nhất) — BA CỠ BÀI:**

| Cỡ | Thời lượng | Nội dung điển hình | Dùng ở phiên nào |
|---|---|---|---|
| **Nhỏ** | **10–15′** | Một hàm: reverse list, two-sum, `memcpy`/`strlen`, endianness | type `coding` (3 bài) · **luật ⑤** trong phiên concept |
| **Vừa** | **20–30′** | Một class có state: RAII wrapper, ring buffer **dùng mutex** | `comprehensive` (**1 bài**) · `coding` nếu rút còn 2 bài |
| **Lớn** 🔺 | **40′+** | Lock-free/SPSC, đa luồng, tối ưu cache | **CHỈ `deep-dive`** — không bao giờ ở phiên mặc định |

> ⚠️ **Ngân sách phải khớp số câu.** `comprehensive` là 16 câu / ~60′ ⟹ chỉ đủ chỗ cho **1 bài cỡ vừa** *hoặc* **2 bài cỡ nhỏ**, không phải 2 bài vừa. Ra đề vượt ngân sách là lỗi của interviewer, không phải của ứng viên.

| Tiêu chí chấm | Mặc định (T2) | `deep-dive` (T3) |
|---|---|---|
| Đồng bộ | một luồng, **hoặc mutex/`lock_guard`** | lock-free, SPSC, CAS |
| Tối ưu | đúng + O() hợp lý + edge case | cache line, false sharing, `alignas`, mask thay `%` |
| Hỏi spec trước khi code | ✅ **tính điểm ở cả hai mức** | ✅ |
| Ví dụ ring buffer | *"sức chứa cố định, không cấp phát trong `push`, đầy thì đè cái cũ + đếm mất, dùng mutex"* | *"SPSC lock-free, chỉ số chạy tự do, release/acquire, `alignas(64)`"* |

> Repo đã tách sẵn 5 tầng cho ring buffer ở [12-dsa/ring-buffer.md](../../12-dsa/ring-buffer.md): §1–§6 là **T2**, §7 (lock-free SPSC) là **T3**. Hỏi đúng tầng thay vì hỏi tầng cao nhất.

**Khi ứng viên tự nêu T3** (vd tự nói *"chỗ này em sẽ cân nhắc false sharing"*): ghi nhận là **điểm cộng vượt mong đợi**, đào sâu thoải mái — nhưng đó là do họ mở cửa, không phải do interviewer ép.

---

### ⚠️ Lan can: SÂU ≠ TRIVIA — cách phân biệt T2 với T3

**Ranh giới T2 / T3, phát biểu một câu:** T2 là *"cơ chế nào giải thích một lớp bug sẽ gặp trong công việc"* — nó đổi **quyết định** của bạn. T3 là **nhãn dán** lên cơ chế đó — biết thì nói nhanh hơn, không biết vẫn ra quyết định đúng.

| ✅ **T2 — tính điểm** | 🔺 **T3 — hỏi được, KHÔNG chấm** |
|---|---|
| *"Vì sao lớp bug này chạy đúng x86 mà chết ARM?"* | Tên lệnh barrier (`stlr`/`ldar`/`dmb ish`) |
| *"Khi nào bạn CHỌN acquire/release thay vì seq_cst?"* | Tên gọi nội bộ của compiler (*guard variable*) |
| *"Làm sao khoanh vùng khi khách copy `.so` mới rồi crash?"* | Tên công cụ (`abidiff`, `readelf -d`, `LD_DEBUG`) |
| *"Giấu state sau con trỏ để `sizeof` không đổi"* | Tên pattern (*Pimpl*), tên flag (`-fvisibility=hidden`) |
| *"Đầy thì đè cái cũ, và phải đếm được số mất"* | `alignas(64)` chống false sharing, mask thay `%` |
| *"Move ctor và move assign khác nhau chỗ nào?"* | Số hiệu Item/§ trong sách |

**Phép thử nhanh khi phân vân:** *"Không biết thứ này thì ứng viên có ra quyết định SAI trong công việc không?"* — Có ⟹ T2. Không, chỉ diễn đạt chậm hơn ⟹ T3.

Khi ứng viên phản hồi *"câu này quá sâu"*: **phân định từng ý** (đồng ý / nửa đồng ý / không đồng ý), ghi kết luận vào [weak-register.md](weak-register.md) mục calibration, và **điều chỉnh thang chấm** — như đã làm ngày 2026-08-07 và 2026-08-10. Đừng gật đại, cũng đừng bảo vệ câu hỏi bằng mọi giá.

---

## 7. 📊 ĐỘ PHỦ — chống "đào sâu một góc, bỏ trắng phần còn lại" (BẮT BUỘC)

> **Vì sao có mục này (2026-08-17):** rà soát tình cờ phát hiện sau **15/28 ngày** của plan, phủ bank chỉ **114/397 = 29%** — và lệch nặng: `CPP` **71%**, `LNX` **59%**, nhưng **`DRV` 0%** (42 câu, **trụ lớn nhất của JD**, chưa hỏi một câu nào), `BSP` 6%, `NET`/`BEH`/`RES` 0%.
>
> **Không có luật nào bắt đo, nên không ai đo.** Mỗi phiên đều "tốt" khi nhìn riêng lẻ; chỉ khi cộng lại mới thấy hai tuần đã đi vào một góc. Đây là lỗi **chỉ nhìn thấy ở mức tổng**, nên phải có luật riêng.

**Ba tình huống BẮT BUỘC đo:**
1. **Trước khi bắt đầu một giai đoạn/tuần mới** của plan.
2. **Mỗi 5 phiên** — dù plan có nói gì.
3. **Khi ứng viên hỏi "ôn tới đâu rồi"** — trả lời bằng số, không bằng cảm nhận.

**Lệnh đo:**
```bash
cd 14-prep/mock-interview
# So cau DA TUNG HOI (rut tu log phien)
grep -oh "\b\(CPP\|OS\|LNX\|DRV\|BUS\|BSP\|SD\|BEH\|BLD\|EMB\|DBG\|DP\|DSA\|NET\|COD\|RES\)-[0-9]\{3\}" \
  sessions/*.md | sort -u | sed 's/-[0-9]*//' | sort | uniq -c
# So cau CO trong bank
grep -c "^#### " bank/*.md
```

**Đọc kết quả — ngưỡng hành động:**

| Tình trạng | Nghĩa | Việc phải làm |
|---|---|---|
| Domain 🎯 **trụ JD** phủ **< 20%** | 🔴 Rủi ro cao nhất | **Dừng đào sâu**, chuyển sang `rapid` quét rộng domain đó ngay |
| Một domain > 60% trong khi domain khác < 10% | 🟠 Lệch | Cân lại lịch — xem [datalogic-plan §🔄](../study-plans/datalogic-plan.md) |
| Tổng phủ < 50% khi đã dùng > 50% quỹ thời gian | 🔴 Không kịp | Cắt `deep-dive`/`by-level`, ưu tiên `rapid` + `daily` |

⚠️ **Phủ KHÔNG phải mục tiêu tự thân.** Hỏi qua loa 397 câu vô dụng ngang hỏi sâu 100 câu rồi bỏ trắng phần còn lại. Luật này chỉ để **phát hiện lệch**, còn quyết định vẫn theo *xác suất bị hỏi × độ yếu hiện tại*. Domain ngoài JD (vd `EMB` cụm RTOS/bare-metal) **cố ý** để phủ thấp — ghi rõ lý do ở plan thay vì cố kéo lên.

**Ghi kết quả ở đâu:** bảng phủ trong **§📍 của plan đang chạy** — một chỗ duy nhất, không chép số sang file khác.
