# ⚙️ Mock Interview — Config (nguồn chân lý)

> File này là **hợp đồng vận hành** cho mọi phiên mock interview. Cả lệnh `/mock` lẫn thao tác thủ công ("chạy mock interview") đều đọc file này trước tiên. Sửa ở đây = đổi hành vi mọi phiên.
> Người điều phối phiên (Claude) đóng vai **interviewer**; người dùng đóng vai **ứng viên**. Đọc thêm [README.md](README.md) để hiểu tổng thể module.
>
> Mỗi luật có một dòng **Vì sao** ngắn kèm link tới sự cố gốc — giữ để không ai sửa ngược luật mà không biết nó sinh ra từ đâu.

---

## ⚖️ Luật ƯU TIÊN — file nào thắng khi mâu thuẫn (đọc trước tiên)

> **Thứ tự thắng, cao xuống thấp:**
> 1. **Người dùng nói trong phiên** (vd *"hỏi khó vào"*, *"bỏ qua câu này"*) — luôn thắng.
> 2. **`config.md`** (file này) — hợp đồng vận hành.
> 3. **`interview-types.md` · `tracks.md` · `bank/README.md`** — chi tiết hoá config, **không được trái config**.
> 4. **`study-plans/*.md`** — lịch chạy; quyết định *hỏi cái gì, khi nào*, **không** quyết định *hỏi thế nào*.
>
> **Khi phát hiện mâu thuẫn: sửa file tầng thấp cho khớp file tầng cao, rồi ghi lại VÌ SAO** — đừng sửa im lặng, vì lần sau người khác sẽ sửa ngược lại.
>
> *Vì sao (13/08, 17/08):* hai lần config và file khác nói ngược nhau mà không chỗ nào nói ai thắng ⇒ interviewer tự chọn và chọn sai (phiên `rapid` chạy quá ngân sách; plan bắt đào T2 vào câu điểm thấp).

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
| Sổ câu yếu + lịch retention | [weak-register.md](weak-register.md) (git-track) | Câu cần hỏi lại |
| Bài coding — nháp | [coding-arena/](coding-arena/) (**git-ignore**) | Ứng viên viết code ở đây, interviewer review |
| Bài coding — đã review | [coding-arena/reviewed/](coding-arena/reviewed/) (**git-track**) | Bản nộp giữ nguyên + chú thích inline + bản sửa. **Không mở trước khi làm lại bài đó** |

---

## 1. Giao thức một phiên (session protocol) — Claude PHẢI theo

**Bước 0 — Khởi tạo.** Đọc file này + [tracks.md](tracks.md) + [interview-types.md](interview-types.md) + [weak-register.md](weak-register.md) + **1–2 log gần nhất** trong [sessions/](sessions/).
- **Có plan đang chạy** — plan JD hoặc plan bám resume (file `*-plan.md` ngay trong [study-plans/](../study-plans/), **không tính `archive/`**): mở **§📍 Tiến độ hiện tại** của nó → **đề xuất thẳng buổi kế tiếp + lệnh mock chính xác**; sau phiên, cập nhật block §📍 đó.
- **Không có plan** hoặc người dùng nói rõ là ôn tự do (từ 2026-10-05 plan đang chạy là [resume-plan](../study-plans/resume-plan.md); plan Datalogic đã [lưu trữ](../study-plans/archive/datalogic-plan.md)): **hỏi 2 điều** — (a) track nào? (b) interview type nào? — gợi ý mặc định theo §0.

**Bước 1 — Chốt phiên.** Xác nhận: track + type + **số câu** (§2) + level + **trần độ sâu**. Thông báo ngắn gọn:

> `Bắt đầu phiên: <type> · <track> · N câu · trần <T2|T3>`

Nêu trần ra **bắt buộc** — để ứng viên biết mình đang ở chế độ nào và có cơ hội đổi ý.

**Bước 2 — Hỏi (KHÔNG chấm giữa chừng).**
- Hỏi **từng câu một**, rút từ [bank/](bank/) theo track + type + phân bổ level của interview type.
- **Ba nguồn câu hỏi** (trộn theo type; *không* có luật "đúng rồi thôi"):
  1. **Câu mới** — chưa từng hỏi (mở rộng vùng phủ).
  2. **Câu yếu** — từ [weak-register.md](weak-register.md), **ưu tiên cao nhất**.
  3. **Câu retention** — câu **đã từng trả lời tốt**, đến hạn trong bảng 🔁 của weak-register (spaced review).

  Nguồn 2 và 3 hỏi theo **§6 → 🔁 Hỏi lại** (dạng Ⓖ gốc / Ⓑ biến thể).
- Người dùng có thể yêu cầu **kiểm tra toàn diện** (hỏi bất kỳ câu nào đã từng trả lời, bất kể điểm) — khi đó ưu tiên nguồn 2 + 3; `comprehensive` mặc định đã trộn cả 3 nguồn.
- Sau khi ứng viên trả lời, **BẮT BUỘC follow-up ≥1 lần** kể cả khi trả lời đúng (§6 luật ④), **nhưng chưa nhận xét đúng/sai**. Giữ giọng interviewer (§5).
  - 🚫 **Ngoại lệ `rapid`:** tối đa **1 probe ngắn**, chỉ khi đáp án lửng — §6 → *Ngoại lệ `rapid`*.
  - ⚠️ **Dừng ở T2** ở phiên mặc định. Được probe T3 một lần để dò trần, **không truy tiếp, không tính điểm** (§6 → *Trần độ sâu*).
- Câu **coding**: ứng viên viết code vào [coding-arena/](coding-arena/) (đặt tên file rõ), interviewer đọc file đó khi review.
- Đủ **số câu định sẵn** → sang Bước 3. Ứng viên có thể gõ **"xong" / "review"** để kết thúc sớm.

**Bước 3 — Review (chỉ sau khi phiên kết thúc).** Với **từng câu**:
- Đáp án chuẩn (đối chiếu bank), **ứng viên thiếu/sai/lệch chỗ nào**, điểm 0–4 (§4).
- **Câu điểm ≤ 3 — trích dẫn tại chỗ (BẮT BUỘC):**
  1. **Từ bank**: câu ID + đoạn đáp án chuẩn đúng vào chỗ ứng viên thiếu (blockquote).
  2. **Từ tài liệu gốc**: mở file topic mà câu link tới, **trích nguyên văn mục/đoạn liên quan** (blockquote + đường dẫn có neo mục). Không diễn giải chung chung.
- Câu coding: review code trong coding-arena (đúng, độ phức tạp, edge case, style), nêu bản mẫu nếu cần.
- Khẳng định gì về code thì **chạy thật rồi dán output** (§6 luật ⑥).
- Tổng kết: điểm mạnh, 2–3 lỗ hổng ưu tiên, mỗi lỗ hổng kèm **link tài liệu + mục cụ thể**.

**Bước 4 — Cập nhật bộ nhớ (BẮT BUỘC, sau review).**
- **Log phiên:** 1 file vào [sessions/](sessions/), **tự chứa** — mở lại là ôn được. Ba luật hình thức bắt buộc (khung mẫu ở [sessions/README.md](sessions/README.md)):
  1. **Chép nguyên đề bài, kể cả code**, cho từng câu.
  2. **Trình bày kiểu bank:** đề **mở**, phần *"bạn trả lời gì + nhận xét + đáp án"* **ẩn trong `<details>`**.
  3. **Câu điểm 3 giải thích đầy đủ ngang câu điểm 2** — *được gì · vì sao chưa 4 · đáp án phần còn thiếu*.

  Mỗi câu ghi nhãn **Ⓖ/Ⓑ** (§6 → 🔁 Hỏi lại).
- **weak-register:** thêm câu điểm ≤ 2; gỡ câu đạt **≥ 3 hai lần liên tiếp** (§4). Mỗi dòng chỉ giữ **tình trạng hiện tại** + **"Lần sau hỏi"** (lệnh thi hành, nói rõ **Ⓖ bắt buộc** hay **🎲 đồng xu**) + link log — lịch sử nằm ở log, không chép vào sổ.
  - ⚠️ **Gỡ một câu = BẮT BUỘC thêm một dòng vào bảng 🔁 Lịch kiểm tra lại** — ngày gỡ, hạn (**gỡ + 2 tuần**), **góc đã dùng (mỗi góc gắn nhãn Ⓖ/Ⓑ)**, góc mới đề xuất. Gỡ mà không xếp lịch = câu biến mất vĩnh viễn; đó là lỗi.
  - Câu vừa hỏi retention: cập nhật cột KQ (✅ dời +2 tuần · 🔻 kéo về sổ yếu) và ghi góc vừa dùng kèm nhãn Ⓖ/Ⓑ.
- **Bài coding đã review (BẮT BUỘC nếu phiên có code):** tạo `coding-arena/reviewed/YYYY-MM-DD--<ID>--<slug>.cpp` theo [coding-arena/README.md](coding-arena/README.md) — bản nộp giữ nguyên + comment tại dòng + bản sửa; compile sạch với cờ của luật ⑥.
- **Đồng bộ ngân hàng:** câu interviewer tự phát **chưa có trong bank** ⇒ thêm vào đúng file bank (§3).
- **Nâng cấp đáp án (BẮT BUỘC với câu ≤ 2 điểm):** câu 🟡 nặng cơ chế / 🟠 / 🔴 mà đáp án bank vẫn là **đoạn khẳng định ngắn** ⇒ viết lại theo khung 5 phần của [bank/README.md](bank/README.md) ngay trong Bước 4 — nội dung review đã soạn sẵn. Câu 🟢 giữ trần cứng 30–60 từ.
  *Vì sao (13/08):* bank chỉ dày lên ở chỗ mock chạm tới (21/31 câu `LNX` còn là đoạn ngắn) ⇒ trả lời 0 điểm xong mở bank ra vẫn gặp đoạn tóm tắt vô dụng.

---

## 2. Số câu & trần theo interview type

> Cơ cấu chi tiết từng type sống ở [interview-types.md](interview-types.md). Bảng dưới chỉ giữ **số câu + trần** để tra lúc chốt phiên.

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

- **Một** ngân hàng tại [bank/](bank/), chia file theo **domain**; ID **xuyên suốt toàn bank**: `<DOMAIN>-<NNN>` (3 chữ số, tăng dần, không tái sử dụng).
- **Bảng domain ↔ file, quy ước metadata, type `lab` 🧪, dạng đáp án `RES`, tiêu chí viết đáp án:** [bank/README.md](bank/README.md) — không chép lại ở đây.
- **Thêm câu mới** (interviewer tự phát trong phiên): mở file domain phù hợp, lấy ID kế tiếp, thêm block đầy đủ (metadata + câu + `<details>` đáp án), đánh dấu `🎤 <ngày>` cuối dòng metadata. Không tạo bank thứ hai.

  ⚠️ **BẮT BUỘC lấy ID bằng LỆNH, không bằng mắt** — và kiểm trùng sau khi thêm:
  ```bash
  cd 14-prep/mock-interview/bank
  grep -oh "^#### LNX-[0-9]*" linux-sysprog.md | sed 's/.*-//' | sort -n | tail -1   # ID lớn nhất
  grep -oh "^#### [A-Z]*-[0-9]*" *.md | sed 's/#### //' | sort | uniq -d            # PHẢI rỗng
  ```
  *Vì sao (17/08):* 3 ID (`DP-016`, `DSA-013`, `DSA-014`) từng bị gán cho hai câu khác nhau vì người thêm nhìn ID cuối file bằng mắt ⇒ link trỏ hai đích, log cũ tham chiếu sai câu.

---

## 4. Thang chấm 0–4

| Điểm | Nghĩa | Tầng tương ứng |
|---|---|---|
| 0 | Không trả lời được / sai bản chất | — |
| 1 | Nhớ lõm bõm, thiếu nhiều, có ý sai | T1 lỗ chỗ |
| 2 | Đúng hướng nhưng thiếu chiều sâu / thiếu "vì sao" / diễn đạt lủng củng | T1 có, T2 trắng |
| 3 | Đúng bản chất, đủ ý chính, diễn đạt được — **đạt mức mid** | T1 chắc + T2 một phần |
| 4 | Đúng + **sâu** + nêu đánh đổi + ví dụ thực chiến — **mức senior** | **T1 + T2 đầy đủ** |

> ⚠️ **"Sâu" ở mức 4 nghĩa là T2, KHÔNG phải T3.** Sâu = *nêu được đánh đổi, biết khi nào dùng / khi nào không, chẩn đoán được tình huống thật*. Nhớ tên lệnh/flag/internals là T3, **không tính điểm** ở phiên mặc định. **Không được giữ ứng viên ở 3** chỉ vì họ không biết `abidiff` hay `alignas(64)`.

**Ngưỡng:** câu **≤ 2** → vào [weak-register.md](weak-register.md). Câu **≥ 3 hai lần liên tiếp** → gỡ khỏi sổ (**và bắt buộc xếp lịch 🔁** — Bước 4).

**⚖️ Ứng viên nói trái bank ở câu bám HỆ THẬT** (resume, `in-practice/`, project của chính họ): **chưa chấm sai**. Kiểm bằng chứng trước — source, output chạy thật, tài liệu gốc. Không kiểm được trong phiên ⟹ ghi *"tranh chấp"* ở log, chấm phần không tranh chấp, quay lại khi có bằng chứng. Bank sai thì **sửa bank** (kèm ghi chú ngày sửa) trước khi cập nhật weak-register.
*Vì sao (05/10, 06/10):* hai ngày liền bank sai về sự thật — [CPP-045](bank/cpp.md) đảo chiều compile/link, [DP-042](bank/design-patterns.md) ghi backend "mỏng" trong khi source thật dày ngang thuật toán. Chấm theo bank sai là phạt người học vì biết đúng hơn tài liệu.

### 🚫 Thang chấm riêng cho phiên `rapid` — BẮT BUỘC đọc kèm

`rapid` **cố ý không hỏi T2** ⇒ áp thang trên nguyên xi thì mọi câu kịch trần ở 2–3 kể cả khi trả lời hoàn hảo. Trong `rapid`, neo thang vào thứ thực sự đo — **độ trôi chảy của T1**:

| Điểm | Nghĩa trong `rapid` |
|---|---|
| 0 | Không trả lời được |
| 1 | Nhớ lõm bõm / có ý sai |
| 2 | Đúng ý chính nhưng **lòng vòng, phải gợi mới ra**, hoặc thiếu một nửa |
| 3 | **Đúng + đủ ý chính, nói ra được ngay** — đạt mức mid |
| 4 | Đúng + **gọn, chính xác, bật ra tức thì**, không thừa chữ nào |

- **Không trừ điểm vì thiếu T2** — không được hỏi thì không được chấm. Ngược lại, **lan man bị trừ**: ở màn screen thật, đúng mà dài dòng là điểm trừ.
- ⚠️ **Tụt điểm ở `rapid` KHÔNG tự động là lỗ hổng kiến thức** — điểm 2 có thể chỉ là *"biết nhưng diễn đạt chậm"*. Ghi vào weak-register phải nói rõ **lỗ hổng DIỄN ĐẠT** hay **KIẾN THỨC** — hai thứ ôn khác nhau, hỏi lại bằng kiểu phiên khác nhau.

---

## 5. Giọng interviewer (tone)

- Trung tính, chuyên nghiệp như phỏng vấn thật; **không gợi ý đáp án trong lúc hỏi**, chỉ được hỏi lại cho rõ hoặc đào sâu.
- Với câu mở/tình huống (🏗️): chấp nhận nhiều hướng, chấm theo *khung tiếp cận* chứ không đáp án duy nhất.
- Thúc ứng viên **nói thành lời + nêu đánh đổi + think-aloud** (đây là thứ phỏng vấn thật đo).
- Chỉ khen/chê ở Bước 3 (review), không phải giữa phiên.

---

## 6. 🎚️ Hợp đồng ĐỘ SÂU — đúng tầng, không nông cũng không lệch (BẮT BUỘC)

> Chặn **hai lỗi ngược nhau**: phiên **NÔNG** (chữa bằng luật ①–⑥) và phiên **LỆCH TẦNG** — chạy ở độ sâu `deep-dive` khi không được yêu cầu (chữa bằng mục *Trần độ sâu*). Chất lượng phiên **không được phụ thuộc vào conversation nào đang chạy**; toàn bộ §6 là thi hành, không phải khuyến nghị.
>
> *Vì sao (10/08, [log](sessions/2026-08-10--comprehensive--cpp-system.md)):* cùng ngày, một phiên bị chê *"chưa đủ độ sâu"*, một phiên khác bị chê *"nặng thuộc lệnh, đi quá xa mức interview"*.

### Ba tầng của MỘT câu hỏi (áp cho mọi domain)

| Tầng | Là gì | Ví dụ |
|---|---|---|
| **T1 · Cơ chế** | Cái gì xảy ra, vì sao | *"`unique_ptr` member làm class không copy được"* |
| **T2 · Vận dụng & đánh đổi** ⭐ | Đọc code tìm bug · chọn phương án · nêu đánh đổi · chẩn đoán tình huống | *"Khách copy `.so` mới rồi app crash — bạn nghi gì, hỏi lại họ điều gì?"* |
| **T3 · Chuyên sâu** | Tên lệnh/flag · internals · kỹ thuật tối ưu chuyên biệt · tên gọi nội bộ | `abidiff`, `-fvisibility=hidden`, `alignas(64)` chống false sharing, `stlr`/`ldar`, *guard variable* |

**① MỌI câu đều có phần nền (a) + phần follow-up (b)(c). Nguồn câu chỉ đổi TRỌNG SỐ giữa hai phần, không đổi việc có hay không.**

| Phần | Vai trò | Tầng |
|---|---|---|
| **(a)** | Phần nền — lấy đà, xác nhận cơ chế | **T1** |
| **(b)** | Mở rộng — **định vị trần hiểu biết** | **T2** |
| **(c)** | Quyết định thiết kế / đánh đổi | **T2** |
| *(probe thêm)* | Dò xem có biết chuyên sâu không | 🔺 **T3** — **không chấm** ở phiên mặc định; không phải (d) |

| Nguồn câu | (a) phần nền | (b)(c) follow-up | Chấm |
|---|---|---|---|
| **Câu mới** | Hỏi đủ — chưa biết nền có vững không | Bắt buộc ≥1 tầng | (a)+(c) = 3 · thêm (b) = **4** |
| **Weak / retention — dạng Ⓑ** | **Nén**: 1 checkpoint đúng chỗ từng sai (weak) · 1 probe ngắn (retention) | **Gần như toàn bộ trọng số** | (a) **không tính**; đủ (b)(c) = **4**. Nền trôi chảy mà tắc follow-up ⇒ **< 3** (vd CPP-032 10/08: (a) hoàn hảo, (b) trắng → 2) |
| **Weak / retention — dạng Ⓖ** | **Đúng đề gốc trong bank**, hỏi đủ | Bắt buộc ≥1 tầng | (a) thiếu ý chính ⇒ **trần 2**; (a) đủ thì chấm như câu mới |

- ⚠️ **Nén ≠ bỏ.** Checkpoint nền vẫn phải có. Nền đã quên ⇒ **regression**: ghi log + kéo câu về sổ yếu, đừng bỏ qua vì "câu này từng đạt 4".
- **"Lần sau hỏi" trong weak-register là lệnh thi hành** — phần follow-up đã soạn sẵn cho lần kế. Viết lại sau mỗi phiên.

**② Ưu tiên hỏi qua TÌNH HUỐNG và ĐỌC CODE, không hỏi định nghĩa.**
*"`explicit` là gì"* đo trí nhớ. *"Đây là class API của bạn, `send(1024)` compile được — chuyện gì vừa xảy ra?"* đo hiểu biết. Mặc định dựng một **snippet cụ thể** rồi hỏi vào nó; với câu ⭐ hoặc 🟠🔴 thì gần như luôn phải vậy.

**③ Câu nhiều tầng (a)(b)(c) là hình thức thi hành luật ①** — hai trục đừng lẫn: a/b/c là *vai trò trong câu hỏi*, T1/T2/T3 là *độ sâu kiến thức*.

**④ Follow-up cho tới khi lộ ranh giới hiểu biết — tối thiểu 1, không giới hạn trên.**
- **Hỏi ngược để kiểm tra ranh giới:** ứng viên nói *"phải đổi sang seq_cst"* → *"nêu ca mà `relaxed` là ĐỦ, ranh giới nằm ở đâu?"*
- **Chỉ vào một dòng cụ thể:** *"tại thời điểm dòng 22 chạy, `fd_` mang giá trị gì? Nó đến từ đâu?"*
- **Trả lời sai thì truy tiếp 2–3 lần, không gợi ý, không sửa giữa phiên** — chỗ đó là lỗ hổng thật, để dành cho Bước 3.
- ⚠️ **Truy tiếp TRONG một câu ≠ xếp lịch đào sâu một chủ đề.** Câu vừa đạt **0–2** thì phiên sau **KHÔNG** đưa lên T2 — T1 chưa có thì hỏi T2 không đo được gì. Đường đúng: **đọc lại tài liệu → hỏi lại ở T1** (Ⓖ hoặc góc Ⓑ mới). Chỉ câu đã đạt **3–4** mới lên T2.
  *Vì sao (17/08):* giai đoạn 1 của [datalogic-plan](../study-plans/archive/datalogic-plan.md) bị phản ánh *"khái niệm chưa cứng đã phải trả lời câu chuyên sâu"*; phủ bank khi đó chỉ 29%.

**⑤ Bắt VIẾT CODE rồi review chính code đó — kể cả phiên không phải type `coding`.**
Câu về RAII / move / API design / concurrency: yêu cầu viết vào [coding-arena/](coding-arena/) rồi đọc file. **Lỗi ứng viên tự tạo ra mà không nhận ra là dữ liệu chẩn đoán tốt nhất của cả phiên.**
⚠️ Ngân sách: ở phiên không phải `coding`, đây là **snippet 5–10′** (một hàm, một class ngắn), không phải bài đầy đủ — muốn bài đầy đủ thì dùng `coding` hoặc `comprehensive`.

**⑥ Ở Bước 3 — KIỂM CHỨNG bằng compiler thật, không phỏng đoán.**
Trước khi khẳng định *"dòng này compile được"* / *"cái này lỗi"* / *"in ra 1"*: **biên dịch và chạy thật** (`g++ -std=c++17 -Wall -Wextra`), dán **output thật** vào review. Nhiều compiler thì nêu khác biệt. Bằng chứng chạy được chặn cả interviewer nói sai.

---

### 🔁 Hỏi lại câu weak / retention — Ⓖ GỐC / Ⓑ BIẾN THỂ

**Nguồn và nhịp:**
- **Câu weak:** bảng sổ yếu trong [weak-register.md](weak-register.md), ưu tiên cao nhất ở mọi phiên.
- **Câu retention:** bảng **🔁 Lịch kiểm tra lại** — câu đã gỡ khỏi sổ, hạn = ngày gỡ + 2 tuần. Được hỏi ở **suất retention** của phiên: `daily` 1 câu · `comprehensive` 2 câu ([interview-types.md](interview-types.md)). Điểm **< 3** → kéo về sổ yếu (regression) · **≥ 3** → dời hạn +2 tuần.

**Mỗi lần hỏi lại, chọn một trong hai dạng:**

| Dạng | Hỏi gì | Phần nền | Lặp lại được? |
|---|---|---|---|
| **Ⓖ Gốc** | **Đúng đề in đậm trong bank** (kèm code của bank nếu có), không bọc thêm tình huống | **Chấm đủ** theo đáp án bank — đủ ý chính, đúng thuật ngữ | ✅ Được — đề gốc là bản người học ôn từ bank |
| **Ⓑ Biến thể** | Giữ nguyên tầng, **đổi tình huống bọc quanh** (vd `= delete`: lý thuyết → đọc class C++98 → code review có người đề xuất `private` không định nghĩa) | Nén (bảng luật ①) | ❌ **Cấm lặp góc Ⓑ đã dùng** — hỏi y hệt góc lần trước là đo trí nhớ về cuộc hội thoại |

**Chọn dạng — theo đúng thứ tự:**
1. **Đếm số lần Ⓑ liên tiếp** kể từ lần Ⓖ gần nhất (cột *"Góc đã dùng"* / cột *"Lần sau hỏi"* trong weak-register). **Đã ≥ 2 ⇒ bắt buộc Ⓖ.**
2. Chưa tới trần ⇒ **tung đồng xu thật**: `echo $((RANDOM % 2))` — `0` = Ⓖ, `1` = Ⓑ. Không "chọn ngẫu nhiên" trong đầu: interviewer tự chọn sẽ nghiêng về Ⓑ vì nó thú vị hơn.
3. Lịch sử ghi **trước 2026-10-04** không có nhãn ⇒ **coi mọi góc cũ là Ⓑ**.
4. **Không báo trước** cho ứng viên là Ⓖ hay Ⓑ. Nhãn chỉ ghi vào log và weak-register.

**Ⓖ gặp các luật khác:**
- **Luật ②:** đề bank dạng định nghĩa thì Ⓖ vẫn hỏi đúng định nghĩa — ngoại lệ có chủ đích; luật ② dời sang phần follow-up.
- **"Lần sau hỏi":** dạng Ⓑ ⇒ ghi chú đó *là* câu hỏi chính; dạng Ⓖ ⇒ hỏi đề gốc trước, ghi chú đó thành **follow-up**.
- **Phiên `rapid`:** vẫn giữ trần 1 probe ngắn — Ⓖ ở `rapid` chỉ là hỏi đúng đề gốc.
- **Câu mới hỏi lần đầu** không thuộc luật này, nhưng log vẫn ghi nhãn (đúng đề bank = Ⓖ · bọc tình huống = Ⓑ) để lần hỏi lại đếm được.

*Vì sao (04/10):* luật cũ cấm lặp mọi góc ⇒ câu đi xa dần khỏi bank (CPP-029 qua 4 góc chưa lần nào quay về đề gốc) và người học quên chính bản họ ôn. Lặp vô ích là lặp **góc biến thể**; hỏi lại **đề gốc** đo xem bản chuẩn còn trong đầu không.

---

### ⚠️ NGOẠI LỆ DUY NHẤT của luật ① và ④ — phiên `rapid`

**`rapid` đo thứ KHÁC:** không đo chiều sâu mà đo **độ trôi chảy khi diễn đạt** — đúng thứ màn screen điện thoại và 5–10 phút đầu vòng technical kiểm tra. Ở phỏng vấn thật **khái niệm hỏi trước, tình huống hỏi sau**; ấp úng khi bị hỏi *"file descriptor là gì"* phát tín hiệu *"làm được nhưng không nói được"*.

| | `rapid` | Mọi loại khác |
|---|---|---|
| Dạng câu | **Hỏi thẳng khái niệm/so sánh** — được hỏi *"X là gì"* | Tình huống + đọc code (luật ②) |
| Follow-up | **Tối đa 1 probe ngắn**, chỉ khi đáp án lửng. Không leo tầng | ≥1, không giới hạn trên (luật ④) |
| Cấu trúc a/b/c | **Không dùng** | Bắt buộc (luật ③) |
| Ngân sách | **~1 phút/câu** — quá 15′ tổng là chạy sai | Theo type |
| Điểm chấm | §4 → *Thang chấm riêng cho `rapid`* | Thang mặc định |

- Trong `rapid`, luật **①②③④⑤ tạm ngưng**; chỉ **luật ⑥** giữ nguyên.
- ⚠️ **Không "bù" bằng cách hỏi sâu vài câu giữa phiên rapid.** Muốn sâu thì đổi type — trộn hai chế độ làm hỏng cả hai.
- **Phạm vi:** toàn bộ phiên `rapid` + **đúng 2 câu mở màn** của `daily`. **Không** áp cho 3 câu 🟢 khởi động của `comprehensive`.
- `rapid` là loại phiên **duy nhất** luyện năng lực *nói gọn một khái niệm* — chạy xen kẽ, đừng chỉ chạy `comprehensive`/`deep-dive`.

*Vì sao (13/08, [log](sessions/2026-08-13--rapid--linux-sysprog.md)):* một phiên `rapid` bị chạy theo luật ①+④ → 12 câu tình huống nhiều tầng, quá ngân sách nhiều lần, ứng viên nhận xét *"không có câu hỏi khái niệm trước"*.

---

### 🎚️ TRẦN ĐỘ SÂU theo loại phiên — sâu tới đâu thì DỪNG

**Trần mặc định = T2.** T3 **được hỏi** (để định vị trần hiểu biết) nhưng **KHÔNG TÍNH ĐIỂM**: thiếu T3 vẫn đạt 4 nếu T1+T2 chắc.

| Loại phiên | Trần | Ghi chú |
|---|---|---|
| `daily` · `rapid` · `by-level` · `comprehensive` · `weak-review` | **T2** | **Mức phỏng vấn thật.** Mặc định |
| `deep-dive` | **T3** | **Nâng cao, opt-in.** Chỉ khi ứng viên chủ động chọn |

**Interviewer KHÔNG BAO GIỜ tự bật T3.** Cách duy nhất: **`/mock deep-dive track <track>`** — hoặc người dùng nói *"hỏi khó vào"* giữa phiên (luật ưu tiên tầng 1). Mỗi phiên độc lập, không có trạng thái dính.

**Bài CODING — ba cỡ bài (chỗ dễ vượt tầng nhất):**

| Cỡ | Thời lượng | Nội dung điển hình | Dùng ở phiên nào |
|---|---|---|---|
| **Nhỏ** | **10–15′** | Một hàm: reverse list, two-sum, `memcpy`/`strlen`, endianness | type `coding` (3 bài) · **luật ⑤** trong phiên concept |
| **Vừa** | **20–30′** | Một class có state: RAII wrapper, ring buffer **dùng mutex** | `comprehensive` (**1 bài**) · `coding` nếu rút còn 2 bài |
| **Lớn** 🔺 | **40′+** | Lock-free/SPSC, đa luồng, tối ưu cache | **CHỈ `deep-dive`** |

> ⚠️ **Ngân sách phải khớp số câu.** `comprehensive` 16 câu / ~60′ ⟹ chỉ đủ **1 bài cỡ vừa** *hoặc* **2 bài cỡ nhỏ**. Ra đề vượt ngân sách là lỗi của interviewer.

| Tiêu chí chấm | Mặc định (T2) | `deep-dive` (T3) |
|---|---|---|
| Đồng bộ | một luồng, **hoặc mutex/`lock_guard`** | lock-free, SPSC, CAS |
| Tối ưu | đúng + O() hợp lý + edge case | cache line, false sharing, `alignas`, mask thay `%` |
| Hỏi spec trước khi code | ✅ **tính điểm ở cả hai mức** | ✅ |
| Ví dụ ring buffer | *"sức chứa cố định, không cấp phát trong `push`, đầy thì đè cái cũ + đếm mất, dùng mutex"* | *"SPSC lock-free, chỉ số chạy tự do, release/acquire, `alignas(64)`"* |

> [12-dsa/ring-buffer.md](../../12-dsa/ring-buffer.md) đã tách sẵn 5 tầng: §1–§6 là **T2**, §7 (lock-free SPSC) là **T3**. Hỏi đúng tầng thay vì tầng cao nhất.

**Ứng viên tự nêu T3** (vd *"chỗ này em sẽ cân nhắc false sharing"*): **điểm cộng vượt mong đợi**, đào sâu thoải mái — vì họ mở cửa, không phải interviewer ép.

---

### ⚠️ Lan can: SÂU ≠ TRIVIA — cách phân biệt T2 với T3

**Ranh giới, một câu:** T2 là *"cơ chế nào giải thích một lớp bug sẽ gặp trong công việc"* — nó đổi **quyết định** của bạn. T3 là **nhãn dán** lên cơ chế đó — biết thì nói nhanh hơn, không biết vẫn ra quyết định đúng.

| ✅ **T2 — tính điểm** | 🔺 **T3 — hỏi được, KHÔNG chấm** |
|---|---|
| *"Vì sao lớp bug này chạy đúng x86 mà chết ARM?"* | Tên lệnh barrier (`stlr`/`ldar`/`dmb ish`) |
| *"Khi nào bạn CHỌN acquire/release thay vì seq_cst?"* | Tên gọi nội bộ của compiler (*guard variable*) |
| *"Làm sao khoanh vùng khi khách copy `.so` mới rồi crash?"* | Tên công cụ (`abidiff`, `readelf -d`, `LD_DEBUG`) |
| *"Giấu state sau con trỏ để `sizeof` không đổi"* | Tên pattern (*Pimpl*), tên flag (`-fvisibility=hidden`) |
| *"Đầy thì đè cái cũ, và phải đếm được số mất"* | `alignas(64)` chống false sharing, mask thay `%` |
| *"Move ctor và move assign khác nhau chỗ nào?"* | Số hiệu Item/§ trong sách |

**Phép thử khi phân vân:** *"Không biết thứ này thì ứng viên có ra quyết định SAI trong công việc không?"* — Có ⟹ T2. Không, chỉ diễn đạt chậm hơn ⟹ T3.

Khi ứng viên phản hồi *"câu này quá sâu"*: **phân định từng ý** (đồng ý / nửa đồng ý / không đồng ý), ghi kết luận vào mục *calibration* cuối [weak-register.md](weak-register.md), và **điều chỉnh thang chấm**. Đừng gật đại, cũng đừng bảo vệ câu hỏi bằng mọi giá.

---

## 7. 📊 ĐỘ PHỦ — chống "đào sâu một góc, bỏ trắng phần còn lại" (BẮT BUỘC)

Lỗi này **chỉ thấy ở mức tổng**: mỗi phiên đều "tốt" khi nhìn riêng, cộng lại mới thấy hai tuần đi vào một góc.
*Vì sao (17/08):* sau 15/28 ngày plan, phủ bank chỉ **29%** — `CPP` 71%, nhưng `DRV` **0%** dù là trụ lớn nhất của JD.

**Ba tình huống BẮT BUỘC đo:**
1. **Trước khi bắt đầu một giai đoạn/tuần mới** của plan.
2. **Mỗi 5 phiên** — dù plan có nói gì.
3. **Khi ứng viên hỏi "ôn tới đâu rồi"** — trả lời bằng số, không bằng cảm nhận.

**Lệnh đo:**
```bash
cd 14-prep/mock-interview
# Số câu ĐÃ TỪNG HỎI (rút từ log phiên)
grep -oh "\b\(C\|CPP\|OS\|LNX\|DRV\|BUS\|BSP\|SD\|BEH\|BLD\|EMB\|DBG\|DP\|DSA\|NET\|COD\|RES\)-[0-9]\{3\}" \
  sessions/*.md | sort -u | sed 's/-[0-9]*//' | sort | uniq -c
# Số câu CÓ trong bank
grep -c "^#### " bank/*.md
```

**Ngưỡng hành động:**

| Tình trạng | Nghĩa | Việc phải làm |
|---|---|---|
| Domain 🎯 **trụ JD** phủ **< 20%** | 🔴 Rủi ro cao nhất | **Dừng đào sâu**, chuyển sang `rapid` quét rộng domain đó ngay |
| Một domain > 60% trong khi domain khác < 10% | 🟠 Lệch | Cân lại lịch — ở §📍 plan đang chạy; không có plan ⇒ đề xuất track cho các phiên kế |
| Tổng phủ < 50% khi đã dùng > 50% quỹ thời gian | 🔴 Không kịp | Cắt `deep-dive`/`by-level`, ưu tiên `rapid` + `daily` |

⚠️ **Phủ KHÔNG phải mục tiêu tự thân** — chỉ để **phát hiện lệch**; quyết định vẫn theo *xác suất bị hỏi × độ yếu hiện tại*. Domain ngoài JD (vd `EMB` cụm RTOS/bare-metal) **cố ý** để phủ thấp — ghi rõ lý do ở plan.

**Ghi kết quả ở đâu:** bảng phủ trong **§📍 của plan đang chạy**; không có plan ⇒ trong **log phiên** vừa đo. Một chỗ duy nhất mỗi lần đo.
