# ABI & Versioning — Tương thích nhị phân

> **TL;DR**
> - **API** (Application Programming Interface): hợp đồng ở mức **source code** — tên hàm, tham số, kiểu. Tương thích API = code cũ *biên dịch lại* được.
> - **ABI** (Application Binary Interface): hợp đồng ở mức **nhị phân** — layout struct, kích thước/căn lề, calling convention, name mangling, layout vtable. Tương thích ABI = binary cũ *chạy với `.so` mới mà không cần biên dịch lại*.
> - **ABI break** = thay đổi khiến binary đã build với phiên bản cũ chạy sai/crash với `.so` mới (vd thêm field vào struct public, đổi chữ ký, thêm virtual function).
> - **soname** (`libfoo.so.1`) mã hóa **major version** = mức tương thích ABI. ABI break → tăng major (soname mới) để hai phiên bản cùng tồn tại.
> - Giữ ABI ổn định là kỹ năng cốt lõi khi maintain shared library — đặc biệt với C++ (rất dễ vô tình break).

---

## 1. API vs ABI — phân biệt cốt lõi

| | API | ABI |
|--|-----|-----|
| Mức | Source code | Binary |
| Quan tâm | Tên/chữ ký hàm, kiểu | Layout bộ nhớ, calling convention, mangling, vtable |
| Tương thích nghĩa là | Code cũ **biên dịch lại** được với header mới | Binary cũ **chạy được** với `.so` mới, **không** biên dịch lại |
| Phá vỡ bởi | Đổi tên hàm, đổi/bớt tham số | Thêm field struct, đổi thứ tự field, thêm virtual, đổi kích thước kiểu |

Một thay đổi có thể giữ API nhưng **phá ABI** (nguy hiểm vì biên dịch vẫn pass, lỗi chỉ lộ lúc chạy). Ví dụ kinh điển: thêm một field vào struct mà caller cấp phát theo `sizeof` cũ.

---

## 2. Điều gì gây ABI break (C++ rất nhạy)

Các thay đổi **phá ABI**:
- Thêm/bớt/đổi thứ tự **data member** của class/struct public (đổi `sizeof` và offset).
- Thêm/bớt/đổi thứ tự **virtual function** (đổi layout vtable → gọi nhầm hàm).
- Đổi chữ ký hàm (kiểu tham số, trả về, const, số lượng).
- Đổi kiểu cơ sở (vd `int` → `long`), đổi enum underlying type, đổi alignment/packing.
- Đổi từ inline thành non-inline (hoặc ngược lại) cho hàm đã được inline ở caller cũ.
- Thay đổi loại liên kết / bỏ symbol đang export.

Các thay đổi **thường an toàn ABI**:
- Thêm hàm **non-virtual** mới, thêm class mới.
- Thêm static member.
- Thêm enum value mới ở **cuối** (nếu không đổi underlying type và caller không exhaustive-switch theo binary cũ).
- Sửa thân hàm mà không đổi chữ ký/hành vi observable.

> Vì C++ phơi bày nhiều chi tiết (layout, vtable, template) ra ABI, giữ ABI ổn định rất khó → kỹ thuật **pimpl** và **biên giới C** được dùng để giảm bề mặt ABI (xem [api-design.md](api-design.md)).

---

## 3. soname & quy ước version

```
libfoo.so.1.4.2     # real name — bản cụ thể (major.minor.patch)
libfoo.so.1         # SONAME    — nhúng trong ELF, loader dùng lúc chạy
libfoo.so           # linker name — dùng lúc build (-lfoo), trỏ tới bản mới nhất
```

- **soname** chứa **major version** = "mức ABI". Binary build với `libfoo` ghi nhận cần `libfoo.so.1` (soname tại thời điểm build).
- **Tăng version theo nghĩa:**
  - **Patch** (1.4.2→1.4.3): sửa lỗi, không đổi API/ABI.
  - **Minor** (1.4→1.5): thêm tính năng, **tương thích ngược** (ABI cũ vẫn chạy). soname giữ nguyên.
  - **Major** (1→2): **phá ABI**. → **đổi soname** (`libfoo.so.2`) để bản cũ (`libfoo.so.1`) và mới cùng tồn tại, app cũ vẫn tìm thấy bản nó cần.

→ Nhờ soname, một hệ thống có thể giữ nhiều major version song song; app link đúng soname nó cần.

---

## 4. Symbol versioning — nhiều phiên bản trong một .so

Cơ chế (glibc dùng) cho phép một `.so` chứa **nhiều phiên bản của cùng một symbol**, gắn version node qua **version script**:

```
# version.map
LIBFOO_1.0 { global: foo; bar; local: *; };
LIBFOO_2.0 { global: baz; } LIBFOO_1.0;
```
```sh
g++ -shared -Wl,--version-script=version.map ...
```

- Binary cũ tiếp tục bind tới `foo@LIBFOO_1.0`; binary mới bind tới phiên bản mới hơn → giữ tương thích **mà không cần tăng soname**.
- Đây là cách `libc` cho phép `memcpy@GLIBC_2.2.5` và phiên bản mới cùng tồn tại.
- `local: *` ẩn mọi symbol không khai báo → kiểm soát chính xác bề mặt ABI (giảm rủi ro).

---

## 5. Chiến lược giữ tương thích khi maintain library

1. **Tối thiểu hóa bề mặt ABI**: chỉ export những gì cần (`-fvisibility=hidden` + đánh dấu export rõ ràng / version script).
2. **Dùng pimpl** cho class public → giấu data member, thêm field không phá ABI (xem api-design).
3. **Không đổi** chữ ký/layout đã phát hành — *thêm* hàm mới thay vì sửa hàm cũ.
4. **Tránh virtual function trong interface public** nếu cần ổn định cao (thêm virtual phá vtable); hoặc cố định layout ngay từ đầu.
5. **Biên giới C** (`extern "C"`, kiểu POD) cho API công khai khi cần tương thích chéo compiler/ngôn ngữ.
6. **Versioning kỷ luật**: tăng major + đổi soname khi buộc phải phá ABI; tài liệu hóa thay đổi.
7. Công cụ kiểm tra: **`abi-compliance-checker`**, `abidiff` (libabigail) so sánh ABI giữa hai bản.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [SD-017](../14-prep/mock-interview/bank/system-design.md) | Phân biệt API và ABI. |
| [SD-017](../14-prep/mock-interview/bank/system-design.md) | ABI break là gì? Cho ví dụ thay đổi giữ API nhưng phá ABI. |
| [SD-018](../14-prep/mock-interview/bank/system-design.md) | Những thay đổi nào trong C++ thường phá ABI? |
| [SD-019](../14-prep/mock-interview/bank/system-design.md) | soname là gì và liên quan thế nào tới tương thích ABI? |
| [SD-019](../14-prep/mock-interview/bank/system-design.md) | Symbol versioning là gì? Khác gì với tăng soname? |
| [SD-020](../14-prep/mock-interview/bank/system-design.md) | Làm sao giữ shared library tương thích ABI khi phát triển tiếp? |

---
⬅️ [linking-loading.md](linking-loading.md) · ➡️ Tiếp theo: [api-design.md](api-design.md)
