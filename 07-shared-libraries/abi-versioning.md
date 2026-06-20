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

<details><summary>1) Phân biệt API và ABI.</summary>

API là hợp đồng ở mức **source code**: tên hàm, danh sách/kiểu tham số, kiểu trả về, tên class — quyết định việc code nguồn cũ có **biên dịch lại** được với phiên bản thư viện mới hay không. ABI là hợp đồng ở mức **nhị phân**: layout và kích thước/căn lề của struct, calling convention (cách truyền tham số/trả về), quy ước name mangling, layout vtable — quyết định việc một binary đã biên dịch sẵn có **chạy đúng với `.so` mới mà không cần biên dịch lại** hay không. Một thay đổi có thể giữ API nhưng phá ABI (vẫn biên dịch được nhưng binary cũ chạy sai), nên cần phân biệt rõ khi maintain thư viện.
</details>

<details><summary>2) ABI break là gì? Cho ví dụ thay đổi giữ API nhưng phá ABI.</summary>

ABI break là thay đổi khiến binary được build với phiên bản thư viện cũ chạy sai hoặc crash khi liên kết với `.so` mới. Ví dụ giữ API nhưng phá ABI: thêm một data member vào một struct/class public — chữ ký hàm và tên không đổi nên code cũ vẫn **biên dịch lại** được (API còn nguyên), nhưng `sizeof` và offset các field thay đổi, nên binary cũ (đã hard-code offset/kích thước cũ) đọc/ghi sai vùng nhớ khi dùng `.so` mới. Tương tự, thêm một virtual function làm đổi layout vtable khiến lời gọi virtual của binary cũ nhảy sai hàm.
</details>

<details><summary>3) Những thay đổi nào trong C++ thường phá ABI?</summary>

Các thay đổi phổ biến phá ABI: thêm/bớt/đổi thứ tự data member của class public (đổi sizeof và offset); thêm/bớt/đổi thứ tự virtual function (đổi vtable); đổi chữ ký hàm (kiểu tham số/trả về, const, số lượng tham số → đổi cả mangled name); đổi kiểu cơ sở hoặc underlying type của enum, đổi alignment/packing; chuyển hàm giữa inline và non-inline khi caller cũ đã inline; xóa hoặc đổi linkage của symbol đang export. C++ đặc biệt nhạy vì phơi bày nhiều chi tiết triển khai (layout object, vtable, template instantiation) ra ABI.
</details>

<details><summary>4) soname là gì và liên quan thế nào tới tương thích ABI?</summary>

soname (vd `libfoo.so.1`) là tên được nhúng trong file ELF của shared library, mã hóa **major version** đại diện cho mức ABI. Khi build, executable ghi nhận cần soname tại thời điểm đó (`libfoo.so.1`), và loader dùng soname để tìm thư viện lúc chạy. Quy ước: thay đổi tương thích ngược (sửa lỗi, thêm tính năng không phá ABI) giữ nguyên soname; thay đổi **phá ABI** thì tăng major và **đổi soname** (`libfoo.so.2`). Nhờ vậy bản cũ và mới có soname khác nhau, cùng tồn tại trên hệ thống, và mỗi app vẫn nạp đúng bản ABI mà nó được build cùng — tránh app cũ vô tình dùng `.so` đã phá ABI.
</details>

<details><summary>5) Symbol versioning là gì? Khác gì với tăng soname?</summary>

Symbol versioning (cơ chế của glibc, qua version script) cho phép một shared library chứa **nhiều phiên bản của cùng một symbol** trong cùng một file, mỗi binary bind tới phiên bản tương ứng lúc nó được build. Nhờ vậy có thể thay đổi/cập nhật một hàm mà binary cũ vẫn dùng phiên bản cũ của symbol, giữ tương thích **mà không phải tăng soname** hay tách ra file `.so` mới — ví dụ glibc giữ `memcpy@GLIBC_2.2.5` cùng các phiên bản mới trong một `libc.so.6`. Tăng soname thì tạo ra một file thư viện major mới hoàn toàn (hai bản tách biệt cùng tồn tại); symbol versioning tinh vi hơn, giữ mọi thứ trong một file và chỉ phiên bản hóa ở mức từng symbol. Ngoài ra version script (`local: *`) còn giúp ẩn symbol nội bộ, thu nhỏ bề mặt ABI.
</details>

<details><summary>6) Làm sao giữ shared library tương thích ABI khi phát triển tiếp?</summary>

Các chiến lược chính: thu nhỏ bề mặt ABI bằng cách chỉ export symbol cần thiết (`-fvisibility=hidden` + version script); dùng kỹ thuật pimpl để giấu data member nên thêm field không đổi layout public; chỉ **thêm** hàm/class mới thay vì sửa chữ ký hoặc layout đã phát hành; thận trọng với virtual function trong interface public (thêm virtual phá vtable); dùng biên giới C (`extern "C"`, kiểu POD) cho API công khai khi cần tương thích chéo compiler; và áp dụng versioning kỷ luật — tăng major và đổi soname khi buộc phải phá ABI. Có thể dùng công cụ như `abidiff` (libabigail) hoặc `abi-compliance-checker` để tự động phát hiện ABI break giữa hai phiên bản trước khi phát hành.
</details>

---
⬅️ [linking-loading.md](linking-loading.md) · ➡️ Tiếp theo: [api-design.md](api-design.md)
