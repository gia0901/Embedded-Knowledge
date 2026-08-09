# 🧑‍💻 Coding Arena — Nơi viết code khi làm bài coding

> Khi chạy mock interview **type `coding`** (hoặc câu coding trong `comprehensive`), interviewer ra đề và bạn **viết code trực tiếp vào thư mục này** (mỗi bài một file). Interviewer đọc file ở đây để **review**: đúng/chạy, độ phức tạp, edge case, style, bản mẫu.

## 📁 Hai vùng, hai mục đích khác hẳn nhau

```
coding-arena/
├── README.md              ← git-track
├── <bài đang làm>.cpp     ← 🚫 GIT-IGNORE — vùng NHÁP, xoá/ghi đè thoải mái
└── reviewed/              ← ✅ GIT-TRACK — bản đã review, có chú thích, GIỮ MÃI
    └── YYYY-MM-DD--<ID>--<slug>.cpp
```

| | Gốc `coding-arena/` | `reviewed/` |
|---|---|---|
| Git | **ignore** | **track** |
| Chứa gì | bài đang làm dở, nháp, file test | bản nộp **giữ nguyên** + chú thích review inline + **bản sửa** |
| Vòng đời | ghi đè mỗi phiên | không bao giờ sửa lại, chỉ thêm file mới |
| Mục đích | làm bài | **học từ chính mình** |

## ⛔ Luật quan trọng nhất: KHÔNG mở `reviewed/` trước khi làm lại bài đó

Bank có luật *"không có luật đúng rồi thôi"* ([config.md §1](../config.md)) — mọi bài đều sẽ được hỏi lại. Nếu mở lời giải đã sửa ra xem trước, lần đó không còn là *làm bài* mà là *chép lại*, và bạn mất đúng cái tín hiệu cần đo: **lần này mình có tự tránh được lỗi cũ không?**

Trình tự đúng khi gặp lại một bài đã có trong `reviewed/`:
1. Làm lại **từ file trống** ở thư mục gốc, không mở `reviewed/`.
2. Interviewer review xong → ghi file mới `reviewed/<ngày mới>--<ID>--<slug>.cpp`.
3. **Lúc đó** mới mở hai bản cạnh nhau: `git log --follow -p -- reviewed/` cho thấy chính xác lỗi nào đã hết, lỗi nào lặp lại.

> Lỗi **lặp lại** qua hai lần cách nhau vài tuần là tín hiệu mạnh hơn nhiều so với điểm số — nó nói lên đó là *thói quen*, không phải *sơ suất*.

## Quy ước

**Khi làm bài** (thư mục gốc): mỗi bài một file, tên interviewer đưa — vd `reverse_list.cpp`, `ring_buffer.cpp`. Ngôn ngữ tuỳ đề (C/C++ mặc định cho track embedded/system). Cứ nháp, chạy thử, sửa thoải mái.

**Khi review xong** (interviewer làm ở Bước 4, không phải bạn): tạo `reviewed/YYYY-MM-DD--<COD-ID>--<slug>.cpp` gồm 3 phần theo thứ tự:

1. **Header block** — ID, ngày, điểm, và danh sách ✅ làm được / ❌ lỗi (đánh nhãn `[A]`, `[B]`… để comment bên dưới trỏ ngược lên).
2. **Bản bạn nộp, GIỮ NGUYÊN từng dòng** — chỉ chèn comment `// ❌ [X] …` / `// ✅ …` tại đúng dòng. Không sửa code ở phần này; giá trị nằm ở chỗ thấy được chính xác mình đã viết gì.
3. **Bản sửa** — giữ **mọi quyết định thiết kế của bạn** (nếu chúng hợp lý), chỉ sửa đúng các lỗi đã đánh nhãn. Kèm đoạn "bài học lớn nhất" nếu có lỗi thuộc về *lớp bug* chứ không phải *một dòng*.

Yêu cầu: file trong `reviewed/` phải **biên dịch và chạy được** (`g++ -std=c++17 -Wall`), có `main()` test các ca biên đã bàn trong phiên. Phần bản-nộp-nguyên-trạng bọc trong `#if 0 … #endif` nếu nó không compile.

## Liên quan
- Đề coding rút từ [../bank/coding.md](../bank/coding.md) (`COD-*`) + [../bank/dsa.md](../bank/dsa.md).
- Log phiên tương ứng ở [../sessions/](../sessions/) — file `reviewed/` là *code*, log là *lý do*; đọc kèm nhau.
