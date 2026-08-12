# ⚙️ Mock Interview — Config (nguồn chân lý)

> File này là **hợp đồng vận hành** cho mọi phiên mock interview. Cả lệnh `/mock` lẫn thao tác thủ công ("chạy mock interview") đều đọc file này trước tiên. Sửa ở đây = đổi hành vi mọi phiên.
> Người điều phối phiên (Claude) đóng vai **interviewer**; người dùng đóng vai **ứng viên**. Đọc thêm [README.md](README.md) để hiểu tổng thể module.

---

## 0. Mặc định (defaults)

| Tham số | Giá trị mặc định | Ghi chú |
|---|---|---|
| Ngôn ngữ | Tiếng Việt | Câu hỏi + nhận xét đều tiếng Việt |
| Level ứng viên | Mid-level (kỹ sư ~2–5 năm) | Điều chỉnh độ khó quanh mốc này |
| Track mặc định | `bsp` (Embedded Linux/BSP) | Ưu tiên 1 theo định hướng ôn tập; xem [tracks.md](tracks.md) |
| Interview type mặc định | `daily` | Xem [interview-types.md](interview-types.md) |
| **Trần độ sâu** | **T2** (vận dụng & đánh đổi) | **T3** (tên lệnh/flag/internals/lock-free) hỏi được nhưng **không tính điểm**. Bật T3 bằng `deep-dive` hoặc `--deep` — xem §6 |
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

Nêu trần ra **bắt buộc** — để ứng viên biết mình đang ở chế độ nào và có cơ hội đổi ý (thêm `--deep` nếu muốn khó hơn).

**Bước 2 — Hỏi (KHÔNG chấm giữa chừng).**
- Hỏi **từng câu một**, rút từ [bank/](bank/) theo track + type + phân bổ level của interview type.
- **Ba nguồn câu hỏi** (interviewer trộn theo type; *không* có luật "đúng rồi thôi"):
  1. **Câu mới** — chưa từng hỏi (mở rộng vùng phủ).
  2. **Câu yếu** — từ [weak-register.md](weak-register.md), **ưu tiên cao nhất** (câu từng sai/chưa chuẩn).
  3. **Câu ôn nhớ (retention)** — câu **đã từng trả lời TỐT**, hỏi lại để kiểm tra độ nhớ theo thời gian (spaced review). Trả lời tốt **không** loại câu khỏi vùng được hỏi.
     - **Nguồn rút: bảng "🔁 Lịch kiểm tra lại" trong [weak-register.md](weak-register.md)** — có sẵn ngày đến hạn + **góc đã dùng (cấm lặp)** + góc mới đề xuất. Không phải quét `sessions/` thủ công nữa.
     - Câu đến hạn được rải vào **slot khởi động 🔁 5–10′ đầu mỗi buổi**, không cần đợi phiên `retention` riêng.
     - Điểm **< 3 → kéo thẳng về bảng sổ yếu** (regression). Điểm ≥ 3 → dời lịch +2 tuần.
- Người dùng có thể yêu cầu **kiểm tra toàn diện** (hỏi bất kỳ câu nào đã từng trả lời, bất kể điểm) — khi đó ưu tiên nguồn 2 + 3; `comprehensive` mặc định đã trộn cả 3 nguồn.
- Sau khi ứng viên trả lời, **BẮT BUỘC hỏi đào sâu / follow-up ≥1 lần** kể cả khi trả lời đúng (§6 luật ④), **nhưng chưa đưa nhận xét đúng/sai**. Chỉ ghi nhận và chuyển câu tiếp (hoặc đào sâu). Giữ giọng interviewer: trung tính, thúc đẩy suy nghĩ.
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
- Ghi **1 file log** vào [sessions/](sessions/) theo mẫu [sessions/README.md](sessions/README.md): ngày, track, type, bảng câu (ID + điểm + ghi chú ngắn) **và mục 🔎 "Chi tiết ôn"** — **persist nguyên văn** phần review Bước 3 cho câu ≤3 (thiếu gì + trích bank + trích tài liệu gốc + câu chốt). Log phải **tự chứa** để mở lại là ôn được, không cần đọc lại hội thoại.
- Cập nhật [weak-register.md](weak-register.md): thêm câu điểm ≤ 2, gỡ/hạ câu đã trả lời vững (≥ 3) qua ≥ 2 lần.
  - ⚠️ **Gỡ một câu = BẮT BUỘC thêm một dòng vào bảng [🔁 Lịch kiểm tra lại](weak-register.md)** — ghi ngày gỡ, hạn kiểm tra (**tuần gỡ + 2**), **góc đã dùng** (để lần sau không lặp) và góc mới đề xuất. Gỡ mà không xếp lịch = câu đó biến mất vĩnh viễn khỏi vùng được hỏi; đó là lỗi.
  - Câu vừa kiểm tra retention: cập nhật cột KQ (✅ dời +2 tuần · 🔻 kéo về bảng sổ yếu) và **ghi góc vừa dùng vào cột "góc đã dùng"**.
- **Với phiên có bài coding — ghi bản đã review (BẮT BUỘC):** tạo `coding-arena/reviewed/YYYY-MM-DD--<ID>--<slug>.cpp` (`<ID>` = ID bank của câu sinh ra bài code — thường `COD-*`, nhưng câu `concept` bắt viết code theo luật ⑤ thì dùng chính ID đó, vd `CPP-045`) gồm (1) header điểm + danh sách ✅/❌ có đánh nhãn, (2) **bản ứng viên nộp giữ nguyên từng dòng**, chỉ chèn comment review tại đúng dòng, (3) **bản sửa** giữ mọi quyết định thiết kế hợp lý của ứng viên, chỉ vá lỗi đã đánh nhãn. File phải **compile + chạy** (`g++ -std=c++17 -Wall`). Chi tiết: [coding-arena/README.md](coding-arena/README.md). Đây là vùng git-track — nháp ở thư mục gốc vẫn ignore, để lần sau còn làm lại được từ file trống.
- **Đồng bộ ngân hàng:** nếu trong phiên interviewer đặt câu **chưa có trong [bank/](bank/)** (câu tự phát/mở rộng), **thêm câu đó vào đúng file bank** (gán ID kế tiếp, metadata, đáp án) — xem §3. Đây là cơ chế giữ bank luôn là nguồn duy nhất và lớn dần theo thực chiến.

---

## 2. Số câu & cấu trúc theo interview type

Chi tiết ở [interview-types.md](interview-types.md). Bảng nhanh:

| Type | Số câu | Trần | Cơ cấu | Thời lượng ước |
|---|---|---|---|---|
| `daily` | 6 | T2 | 2 rapid + 3 concept (mix 🟢🟡) + 1 revisit (weak-register). Code = **snippet 5–10′** | 15–20′ |
| `rapid` | 12 | T2 | 🟢🟡 concept, đáp án ngắn | ~15′ |
| `comprehensive` | 16 | T2 | 1 track chính, đủ 🟢→🔴. **Cộng đúng 16:** 3🟢 + 5🟡 + 3 design + **1 coding cỡ vừa** + 2 revisit + 2 retention/xuyên-topic | ~60′ |
| `by-level` | 10 | T2 | cùng 1 level (🟢/🟡/🟠/🔴) trong track chọn | 20–40′ |
| `coding` | 3 | T2 | 3 bài **cỡ nhỏ 10–15′** viết vào coding-arena | 30–45′ |
| `deep-dive` | 5 | 🔺 **T3** | 🟠🔴 design/tình huống 1 track; bài coding được phép **cỡ lớn 40′+** | ~40′ |
| `weak-review` | toàn bộ weak-register (track chọn) | T2 | hỏi lại tới khi vững | linh hoạt |
| `retention` | 8 | T2 | câu đến hạn ở bảng 🔁 Lịch kiểm tra lại, spaced review | ~20′ |
| `full-review` | 12 | T2 | mọi câu đã hỏi (yếu + tốt) | ~45′ |

---

## 3. Ngân hàng — ID & cách thêm câu

- Bank là **một** ngân hàng, đặt tại [bank/](bank/), chia file theo **domain** cho dễ đọc nhưng ID **xuyên suốt toàn bank**.
- **ID = `<DOMAIN>-<NNN>`** (số 3 chữ số, tăng dần, không tái sử dụng). Domain: `CPP`, `OS`, `LNX`, `DRV`, `BUS`, `BSP`, `DBG`, `DP`, `DSA`, `NET`, `SD`, `BEH`, `COD`. Xem [bank/README.md](bank/README.md).
- Mỗi câu có **dòng metadata**: `#### <ID> · <level> · <type> · [· ⭐] · [→ link nguồn]`.
  - level ∈ 🟢 cơ bản / 🟡 trung bình / 🟠 khó / 🔴 senior.
  - type ∈ `concept` / `coding` / `design`. (rapid-fire là *chế độ phiên*, rút từ câu `concept` 🟢🟡; không phải type riêng.)
  - ⭐ = xác suất gặp cao với JD.
- **Thêm câu mới** (khi interviewer tự phát trong phiên): mở file domain phù hợp, lấy ID kế tiếp, thêm block đầy đủ (metadata + câu + `<details>` đáp án), đánh dấu `🎤 <ngày>` cuối dòng metadata để biết câu đến từ mock. Không tạo bank thứ hai, không để câu sống ngoài bank.

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
> **Kịch trần = 4 khi T1+T2 đầy đủ.** Không được giữ ứng viên ở 3 chỉ vì họ không biết `abidiff` hay `alignas(64)`. Muốn T3 tính điểm thì phải chạy `deep-dive` / `--deep`.

Ngưỡng: câu **≤ 2** → vào [weak-register.md](weak-register.md). Câu **≥ 3** hai lần liên tiếp → gỡ khỏi weak-register (**và bắt buộc xếp lịch kiểm tra lại** — §1 Bước 4).

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

**⑤ Bắt VIẾT CODE rồi review chính code đó — kể cả phiên không phải type `coding`.**
Một bug thật trong code ứng viên tự viết có giá trị hơn mười câu lý thuyết. Câu hỏi về RAII / move / API design / concurrency: yêu cầu viết vào [coding-arena/](coding-arena/) rồi đọc file. **Lỗi ứng viên tự tạo ra mà không nhận ra là dữ liệu chẩn đoán tốt nhất của cả phiên.**

> ⚠️ **Ngân sách:** ở phiên **không phải** type `coding`, đây là **snippet 5–10′** (một hàm, một class ngắn — vd "viết move ctor cho class này"), **không phải bài tập đầy đủ**. Phiên `daily` chỉ 15–20′ tổng thì không nhét vừa một bài 20′. Muốn bài đầy đủ → dùng type `coding` hoặc `comprehensive`.

**⑥ Ở Bước 3 — KIỂM CHỨNG bằng compiler thật, không phỏng đoán.**
Trước khi khẳng định *"dòng này compile được"* / *"cái này lỗi"* / *"in ra 1"*: **biên dịch và chạy thật** (`g++ -std=c++17 -Wall -Wextra`), rồi dán **output thật** vào review. Nếu có nhiều compiler thì nêu rõ khác biệt (vd *gcc bắt 0/5, clang bắt 1/5*). Bằng chứng chạy được thuyết phục hơn mọi lời giải thích — và chặn chính interviewer nói sai.

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
| `daily` · `rapid` · `by-level` · `comprehensive` · `weak-review` · `retention` · `full-review` | **T2** | **Mức phỏng vấn thật.** Mặc định |
| `deep-dive` | **T3** | **Nâng cao, opt-in.** Chỉ khi ứng viên chủ động chọn |

**Bật T3 có tính điểm — interviewer KHÔNG BAO GIỜ tự bật.** Ba cách, chi tiết ở [interview-types.md → Cách bật chế độ chuyên sâu](interview-types.md):

| Cách | Lệnh | Tác dụng |
|---|---|---|
| Loại phiên chuyên sâu | `/mock deep-dive track <track>` | Đổi **cả nội dung**: 5 câu design/tình huống, toàn phiên T3 |
| Cờ trên loại bất kỳ | `/mock comprehensive track <track> --deep` | Giữ nguyên cơ cấu, chỉ **nâng trần + thang chấm** lên T3 |
| Nói bằng lời | *"hỏi khó vào"*, *"cho tôi mức senior"* | Như cờ `--deep` |

Không có trạng thái dính: bỏ cờ đi là về lại **T2**. Mỗi phiên độc lập.

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

Hai lỗi ngược nhau, §6 chặn cả hai:
- **Phiên nông** — dừng ở T1, chỉ hỏi định nghĩa → luật ①–⑥ chữa.
- **Phiên lệch tầng** — nhảy lên T3, chấm điểm thuật ngữ → mục "Trần độ sâu" chữa.

**Ranh giới T2 / T3, phát biểu một câu:** T2 là *"cơ chế nào giải thích một lớp bug sẽ gặp trong công việc"* — nó đổi **quyết định** của bạn. T3 là **nhãn dán** lên cơ chế đó — biết thì nói nhanh hơn, không biết vẫn ra quyết định đúng.

| ✅ **T2 — tính điểm** | 🔺 **T3 — hỏi được, KHÔNG chấm** |
|---|---|
| *"Vì sao lớp bug này chạy đúng x86 mà chết ARM?"* | Tên lệnh barrier (`stlr`/`ldar`/`dmb ish`) |
| *"Khi nào bạn CHỌN acquire/release thay vì seq_cst?"* | Tên gọi nội bộ của compiler (*guard variable*) |
| *"Làm sao khoanh vùng khi khách copy `.so` mới rồi crash?"* | Tên công cụ (`abidiff`, `readelf -d`, `LD_DEBUG`) |
| *"Giấu state sau con trỏ để `sizeof` không đổi"* | Tên pattern (*Pimpl*), tên flag (`-fvisibility=hidden`) |
| *"Đầy thì đè cái cũ, và phải đếm được số mất"* | `alignas(64)` chống false sharing, mask thay `%` |

**Phép thử nhanh khi phân vân:** *"Không biết thứ này thì ứng viên có ra quyết định SAI trong công việc không?"* — Có ⟹ T2. Không, chỉ diễn đạt chậm hơn ⟹ T3.
| *"Move ctor và move assign khác nhau chỗ nào?"* | Số hiệu Item/§ trong sách |

Khi ứng viên phản hồi *"câu này quá sâu"*: **phân định từng ý** (đồng ý / nửa đồng ý / không đồng ý), ghi kết luận vào [weak-register.md](weak-register.md) mục calibration, và **điều chỉnh thang chấm** — như đã làm ngày 2026-08-07 và 2026-08-10. Đừng gật đại, cũng đừng bảo vệ câu hỏi bằng mọi giá.
