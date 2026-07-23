# 16 — Book Summaries: Quy ước & Phương pháp

> **File này là quy ước mở rộng của `CLAUDE.md`** dành riêng cho phần tóm tắt sách chuyên ngành. CLAUDE.md giữ vai trò "bộ não" tổng; mọi chi tiết về cách viết book summary nằm ở đây để CLAUDE.md không phình to. Khi viết/sửa bất kỳ file nào trong thư mục này, đọc file này trước.

---

## 1. Định vị & mục đích

- **Mục đích:** ôn tập sách **toàn diện và đầy đủ với tốc độ nhanh nhất** so với đọc thông thường. Summary là bản thay thế việc đọc tuần tự, không phải bản trích lược.
- **Nguyên tắc nén: "đầy đủ ý, nén lời."** Giữ trọn nội dung kỹ thuật quan trọng — không lược bỏ, không tóm gọn quá đà. Chỉ cắt sự dông dài của văn sách: dẫn nhập kéo dài, lặp ý, ví dụ đời thường lan man.
- **Không giới hạn độ dài file `.md`.** Độ dài không phải tiêu chí — miễn đáp ứng các yêu cầu đã chốt. Tuyệt đối không cắt bớt nội dung kỹ thuật chỉ để file ngắn lại; file quá lớn thì tách file theo cụm (Mục 2), không nén nội dung.
- **Nội dung ít quan trọng / ít liên quan:** không viết chi tiết — chú thích 1 dòng nói nó là gì + tham chiếu (Mục 5) để tự tra khi cần.
- **Toàn diện về độ phủ, nghiêng về interview ở độ sâu.** Mọi cụm đều viết đầy đủ theo mạch sách (không bỏ cụm nào vì "ít bị hỏi"); riêng cụm 🎯 trọng tâm interview được đầu tư sâu hơn ở *Góc interview*, và mỗi cuốn có *Lộ trình ôn nhanh trước interview* để ôn lại đúng chỗ khi sát ngày.
- **Tài liệu tự chứa — cho phép lặp với topic 01–14.** Người đọc không phải nhảy sang tài liệu khác giữa chừng; giải thích đầy đủ tại chỗ, link tới file topic liên quan chỉ là *tùy chọn* đọc thêm. Khi nội dung hai nơi mâu thuẫn (do sửa một bên), **file topic 01–14 là chuẩn cho interview prep** — sửa book summary theo.

## 2. Cấu trúc thư mục

```
16-book-summaries/
├── README.md                          # File này: quy ước + danh sách sách + tiến độ
├── effective-modern-cpp.md            # Sách vừa/cấu trúc item rời → 1 file
├── ostep/                             # Sách dày → 1 thư mục, chia file theo cụm chương
│   ├── README.md                      # TL;DR cả cuốn, lộ trình ôn, map chương ↔ file
│   ├── virtualization.md
│   ├── concurrency.md
│   └── persistence.md
└── the-linux-programming-interface/
    ├── README.md
    └── ... (file theo cụm chương: processes, signals, threads, ipc, sockets...)
```

- Tên file **kebab-case**, tiếng Anh, như phần còn lại của repo.
- Sách ≤ ~400 trang hoặc cấu trúc item rời → **1 file**. Sách dày → **1 thư mục**, mỗi file một cụm chương; README của thư mục là điểm vào bắt buộc.
- Vì ưu tiên đầy đủ, một cuốn dày có thể ra nhiều file — chia sao cho mỗi file đọc trọn trong một buổi ôn.

## 3. Template một cuốn sách

**Đầu file (hoặc README nếu là thư mục):**

```markdown
# <Tên sách> — <Tác giả> (<edition/năm>)

> **Nguồn summary:** kiến thức Claude, chưa đối chiếu PDF | đã đối chiếu PDF <tên bản/edition PDF>
> **Vì sao đọc cuốn này:** 1-2 câu, gắn với background/mục tiêu người học.

## TL;DR — cuốn sách này dạy gì
## 🕐 Đọc gì nếu chỉ có N giờ                (cho lần đọc đầu khi ít thời gian)
## 🎯 Lộ trình ôn nhanh trước interview       (các cụm xếp theo ưu tiên ôn lại sát ngày phỏng vấn)
## 🗺️ Bản đồ: chương ↔ cụm chủ đề ↔ topic liên quan   (bảng)
```

**Thân bài — mỗi cụm chủ đề** (gom các chương liền nhau cùng chủ đề khi hợp lý; ghi rõ cụm ứng với chương nào để tra ngược về sách):

```markdown
## <Tên cụm> (ch. X–Y)   [🎯 nếu là trọng tâm interview]

- **Nội dung chính:** đầy đủ các ý kỹ thuật quan trọng — viết đủ để không cần mở sách.
- **Insight đáng nhớ:** lý do thiết kế, trade-off, kinh nghiệm thực chiến, câu quote đắt.
- **Ít quan trọng:** liệt kê 1 dòng/mục + tham chiếu §/trang để tự tra nếu cần.
- **Góc interview:** câu hỏi có thể bị hỏi mà cụm này trả lời được (xem quy ước đáp án bên dưới).
- **Đọc thêm (tùy chọn):** link file topic 01–14 liên quan.
```

## 4. Quy ước trình bày nội dung

**Ngôn ngữ** (theo CLAUDE.md Mục 4): tiếng Việt để giải thích, **giữ nguyên khái niệm & thuật ngữ tiếng Anh**; dùng bảng khi so sánh/lựa chọn.

**Hình & sơ đồ:**
- **Giữ lại các hình minh họa quan trọng của sách** bằng cách **vẽ lại** dưới dạng ASCII diagram hoặc Mermaid, giữ đúng ý đồ của hình gốc và ghi kèm tham chiếu (vd: `(hình theo Figure 6.3)`). Không nhúng ảnh chụp/trích xuất từ PDF — vấn đề bản quyền và giữ repo thuần text.
- **Chủ động bổ sung** hình/sơ đồ khi giúp hình dung (memory layout, luồng xử lý, state machine...), kể cả khi sách không có hình ở đoạn đó.

**Code mẫu — bắt buộc có khi sách có ví dụ đáng giá:**
- Rút gọn về đúng trọng tâm: lược header, boilerplate, error-handling lặp lại, full source không cần thiết.
- **Comment giải thích bằng tiếng Việt** tại các dòng quan trọng.
- Mặc định C++17 với sách C++; giữ C với sách Linux/OS khi ví dụ gốc là C.
- **MÔ TẢ code không thay được CHO XEM code**: sách có listing đáng giá thì summary phải chứa code thật (đã rút gọn), không được tóm bằng văn xuôi — kể cả ở chương "lý thuyết".

**Insight/đúc kết — phải có neo:**
- Mọi kết luận trong "Insight đáng nhớ" phải **bám vào ví dụ, đoạn code hoặc trích dẫn cụ thể** của sách (kèm số trang) — không đúc kết "chay".
- Ý kiến/liên hệ riêng của người viết summary phải đánh dấu 🆕 để tách khỏi ý của sách.

**Góc interview — câu hỏi & đáp án:**
- Mỗi câu hỏi kèm **đáp án đầy đủ** (không phải gợi ý ngắn), có thể gồm code, hình vẽ, sơ đồ để làm rõ.
- Đáp án **ẩn trong `<details><summary>Đáp án</summary>`** (đúng quy ước CLAUDE.md Mục 4) để người học tự trả lời trước, mở ra đối chiếu sau.
- Với cụm 🎯: nhiều câu hỏi hơn, kèm **bẫy thường gặp khi trả lời**; link câu hỏi tương ứng ở topic 11 nếu có.
- Câu hỏi nằm **trong từng cụm** (không gom một mục cuối file như các topic khác) — để ôn đến đâu tự kiểm tra đến đó.

## 5. Tham chiếu về sách (để tự tra cứu)

- **Tham chiếu chính: số chương/section/item** — `(§3.2)`, `(Item 21)`, `(ch. 44)`. Ổn định giữa mọi edition/bản in, điền được ngay cả khi viết từ kiến thức sẵn có.
- **Số trang:** thêm khi đã đối chiếu PDF, viết dạng `(§3.2, tr. 45)`; số trang tính theo **đúng bản PDF ghi ở dòng "Nguồn summary"** đầu file.

## 6. Nguồn & độ tin cậy (chế độ hybrid)

1. Với sách kinh điển, Claude viết trước **từ kiến thức sẵn có**; đầu file ghi rõ `Nguồn summary: kiến thức Claude, chưa đối chiếu PDF`.
2. Chi tiết không chắc chắn (số chương giữa các edition, số liệu cụ thể, tên API hiếm) → đánh dấu **⚠️ chưa chắc** ngay tại chỗ, không viết bừa.
3. Khi người học cung cấp PDF: đối chiếu, sửa các chỗ ⚠️, bổ sung số trang (Mục 5), cập nhật dòng nguồn thành `đã đối chiếu PDF <bản>`. PDF để **ngoài repo** (hoặc thêm `.gitignore`) — không commit file sách có bản quyền.

## 7. Quy trình khi thêm/viết một cuốn

1. Thêm sách vào bảng tiến độ (Mục 8) với trạng thái ⬜.
2. Dựng khung: TL;DR + lộ trình ôn + bản đồ chương ↔ cụm để người học duyệt cấu trúc trước.
3. Viết từng cụm **theo thứ tự đọc của sách** (cuốn rất dày, nếu lịch phỏng vấn gần, có thể thống nhất làm cụm 🎯 trước ở bước dựng khung).
4. Xong cuốn nào cập nhật bảng Mục 8; **bảng tiến độ tổng ở CLAUDE.md Mục 7 chỉ giữ 1 dòng cho cả topic 16**, chi tiết theo sách nằm ở đây.

## 8. Danh sách sách & tiến độ

| Sách | Trạng thái | Ghi chú |
|------|-----------|---------|
| Effective Modern C++ — Scott Meyers | ✅ Xong (chưa đối chiếu PDF) | Đủ 8 cụm / 42 items; sẽ đối chiếu khi có PDF |
| Operating Systems: Three Easy Pieces (OSTEP) — Arpaci-Dusseau | ✅ Xong (chưa đối chiếu PDF) | Thư mục `ostep/`: README + 4 file (CPU, Memory, Concurrency, Persistence) |
| Mastering Embedded Linux Programming — Simmonds (**1st ed 2015** — PDF người học) | ✅ Xong; **ch. 3–4 đã đối chiếu sâu toàn văn** | Thư mục `melp/`: README + 6 file; `bootloader-kernel.md` đối chiếu sâu + số trang thật; các file khác kiến thức + mục lục, đánh dấu 🆕 phần ngoài sách |
| Linux Kernel Development — Robert Love (3rd ed) | ✅ Xong (đối chiếu mục lục + số trang PDF) | Thư mục `lkd/`: README + 6 file; mỗi cụm có mục "⚠️ Đã thay đổi so với sách" cập nhật kernel ~6.x |
| LDD3 / Bootlin materials | ⬜ Chưa bắt đầu | Ưu tiên 3 (BSP): đọc chọn lọc char driver, interrupt, DMA; Bootlin slides miễn phí cập nhật hơn |
| The C++ Programmer's Mindset — Sam Morley (Packt 2025) | 🟡 Đang viết | Thư mục `cpp-mindset/`: sách MỚI (sau cutoff) → đọc 100% từ PDF; **Phần 1 (ch. 1–6) ✅ xong**, còn ch. 7–15 (dự án + scale/GPU/perf) |
| C++ Concurrency in Action — Anthony Williams (2nd ed 2019) | 🟡 Khung xong | Thư mục `cpp-concurrency/`: đã dựng README + bản đồ 11 chương (offset trang xác minh). **Giá trị riêng: ch. 5 memory model/ordering + ch. 7 lock-free — repo chưa có ở đâu.** Chờ đọc sâu |
| The Linux Programming Interface — Michael Kerrisk | ⬜ Chưa bắt đầu | Lùi ưu tiên (userspace API — ít trúng BSP); giá trị dài hạn vẫn giữ |

Ký hiệu: ✅ Xong · 🟡 Đang viết · ⬜ Chưa bắt đầu
