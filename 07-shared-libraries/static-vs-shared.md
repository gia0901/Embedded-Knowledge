# Static vs Shared Library

> **TL;DR**
> - **Static library** (`.a`): tập hợp object file; lúc **link build** được **copy** vào executable. Binary tự chứa, chạy độc lập, nhưng lớn và phải rebuild để cập nhật thư viện.
> - **Shared library** (`.so`): nạp lúc **runtime**; nhiều chương trình **dùng chung một bản** trong bộ nhớ. Binary nhỏ, cập nhật thư viện không cần rebuild app, nhưng phụ thuộc thư viện đúng phải tồn tại lúc chạy ("DLL hell").
> - Trade-off cốt lõi: static = đơn giản & độc lập & lớn; shared = tiết kiệm & cập nhật được & ràng buộc runtime + ABI.
> - Embedded: cân nhắc theo số lượng app dùng chung, dung lượng flash, nhu cầu update field, và độ phức tạp triển khai.

---

## 1. Library là gì?

Library là tập hợp code (hàm/class) đã biên dịch, đóng gói để tái sử dụng nhiều chương trình mà không phải copy source. Có hai dạng liên kết:
- **Static**: nhúng vào binary lúc build.
- **Shared/dynamic**: tách riêng, liên kết lúc tải/chạy.

```sh
# Static
g++ -c math.cpp -o math.o
ar rcs libmath.a math.o            # gói object thành .a
g++ main.cpp -L. -lmath -o app     # nội dung libmath copy vào app

# Shared
g++ -fPIC -c math.cpp -o math.o    # -fPIC: position-independent code
g++ -shared math.o -o libmath.so   # tạo .so
g++ main.cpp -L. -lmath -o app     # app chỉ GHI NHẬN cần libmath.so
```

---

## 2. So sánh chi tiết

| Tiêu chí | Static (`.a`) | Shared (`.so`) |
|----------|--------------|----------------|
| Thời điểm liên kết | Lúc build (copy vào binary) | Lúc load/runtime |
| Kích thước mỗi binary | Lớn (chứa cả thư viện) | Nhỏ (chỉ tham chiếu) |
| Bộ nhớ khi nhiều app dùng | Mỗi process một bản | **Chia sẻ một bản** (code) giữa các process |
| Cập nhật thư viện | Phải **rebuild & redeploy** app | Thay `.so` là xong (nếu ABI tương thích) |
| Phụ thuộc runtime | Không (tự chứa) | Có — `.so` phải tồn tại & đúng phiên bản |
| Tốc độ khởi động | Nhanh (không phải resolve) | Chậm hơn chút (load + relocate) |
| Rủi ro phiên bản | Thấp | "DLL hell" nếu ABI lệch |
| `-fPIC` bắt buộc? | Không | Có |

---

## 3. Vì sao shared library tiết kiệm bộ nhớ?

Phần **code (text)** của một `.so` chỉ tồn tại **một bản trong RAM vật lý**, được **map (chia sẻ) read-only** vào address space của mọi process dùng nó (qua virtual memory — xem [03/memory-management](../03-operating-system/memory-management.md)). 100 process dùng `libc.so` → vẫn chỉ một bản code libc trong RAM. Static thì mỗi binary mang bản sao riêng → tốn cả disk lẫn RAM khi nhiều app.

(Dữ liệu có thể ghi của thư viện thì mỗi process có bản riêng qua copy-on-write.)

---

## 4. Khi nào dùng static, khi nào shared?

**Dùng static khi:**
- Muốn binary **tự chứa**, dễ triển khai (một file, không lo thiếu `.so`).
- Hệ thống chỉ có **một/ít** chương trình dùng thư viện (không tận dụng được chia sẻ).
- Cần **tất định** tuyệt đối về phiên bản (không bị thay `.so` ngoài ý muốn).
- Tối ưu hiệu năng: linker có thể loại bỏ code chết, inline cross-module (LTO).

**Dùng shared khi:**
- **Nhiều** chương trình dùng chung thư viện (tiết kiệm RAM/flash đáng kể).
- Cần **vá lỗi/cập nhật** thư viện mà không rebuild mọi app (vd vá bảo mật `libssl`).
- Thư viện lớn, dùng plugin (`dlopen`), hoặc license yêu cầu liên kết động.

> **Góc embedded:** flash hạn chế → nếu chỉ một app, static có thể nhỏ hơn (không mang overhead của cơ chế động + chỉ lấy phần dùng tới). Nếu nhiều app chia sẻ thư viện lớn → shared tiết kiệm tổng thể. Khả năng **update từng phần ngoài hiện trường** (OTA) cũng nghiêng về shared.

---

## 5. Quy ước tên & vị trí trên Linux

```
libmath.so.1.2.3     # file thật:  lib<name>.so.<major>.<minor>.<patch>
libmath.so.1         # soname (symlink) — major version, dùng lúc runtime
libmath.so           # linker name (symlink) — dùng lúc build (-lmath)
```

- Tìm `.so` lúc chạy: theo thứ tự `RPATH/RUNPATH` trong binary → `LD_LIBRARY_PATH` → cache `ldconfig` (`/etc/ld.so.cache`) → đường mặc định (`/lib`, `/usr/lib`).
- Công cụ: `ldd app` (liệt kê `.so` phụ thuộc), `nm`/`objdump`/`readelf` (xem symbol). (Chi tiết loader ở [linking-loading.md](linking-loading.md).)

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Static library và shared library khác nhau thế nào?</summary>

Static library (`.a`) là tập hợp object file; lúc **build**, linker copy phần cần dùng vào executable, nên binary tự chứa và chạy độc lập, nhưng kích thước lớn và muốn cập nhật thư viện phải rebuild lại app. Shared library (`.so`) được liên kết lúc **load/runtime**: executable chỉ ghi nhận cần thư viện nào, hệ thống nạp `.so` khi chạy; nhiều process chia sẻ chung một bản code trong RAM, binary nhỏ, và có thể cập nhật `.so` mà không rebuild app (miễn ABI tương thích) — đổi lại có phụ thuộc runtime (phải có đúng `.so`) và rủi ro lệch phiên bản.
</details>

<details><summary>2) Vì sao shared library tiết kiệm bộ nhớ khi nhiều chương trình dùng?</summary>

Vì phần code (text segment) của một `.so` chỉ cần tồn tại **một bản duy nhất trong RAM vật lý**, và được map read-only vào không gian địa chỉ ảo của mọi process dùng nó thông qua virtual memory. Do đó dù 100 process cùng dùng `libc.so`, RAM chỉ giữ một bản code libc. Với static library, mỗi executable nhúng bản sao riêng của code thư viện, nên tốn cả dung lượng đĩa lẫn RAM tỉ lệ với số chương trình. (Phần dữ liệu ghi được của thư viện thì mỗi process vẫn có bản riêng qua copy-on-write.)
</details>

<details><summary>3) -fPIC là gì và vì sao shared library cần nó?</summary>

`-fPIC` (Position-Independent Code) sinh mã có thể chạy đúng bất kể được nạp vào địa chỉ nào trong bộ nhớ, bằng cách truy cập biến/hàm toàn cục qua bảng gián tiếp (GOT/PLT) thay vì địa chỉ tuyệt đối cố định. Shared library cần PIC vì nó có thể được map vào địa chỉ khác nhau trong mỗi process (và với ASLR thì địa chỉ còn ngẫu nhiên), nên không thể giả định một địa chỉ nạp cố định; nhờ PIC, cùng một bản code `.so` chia sẻ được giữa các process với các vị trí map khác nhau. Static link vào executable thì thường không bắt buộc PIC (trừ khi build PIE).
</details>

<details><summary>4) Khi nào nên chọn static, khi nào chọn shared?</summary>

Chọn static khi muốn binary tự chứa và triển khai đơn giản (một file, không lo thiếu `.so`), khi chỉ một/ít chương trình dùng thư viện (không tận dụng được chia sẻ bộ nhớ), khi cần tất định tuyệt đối về phiên bản, hoặc khi muốn tối ưu (loại code chết, LTO inline cross-module). Chọn shared khi nhiều chương trình dùng chung thư viện (tiết kiệm RAM/disk), khi cần vá lỗi/cập nhật thư viện mà không rebuild mọi app (đặc biệt vá bảo mật), hoặc khi dùng plugin/`dlopen`. Trong embedded còn cân nhắc dung lượng flash và nhu cầu cập nhật từng phần qua OTA.
</details>

<details><summary>5) "DLL hell" / vấn đề phiên bản của shared library là gì?</summary>

Là các vấn đề phát sinh từ phụ thuộc runtime vào shared library: app cần một `.so` với phiên bản/ABI tương thích phải có mặt lúc chạy, nhưng hệ thống có thể thiếu nó, có phiên bản sai, hoặc một bản cập nhật làm thay đổi ABI khiến app cũ crash hoặc hành xử sai. Nhiều app cần các phiên bản khác nhau của cùng thư viện cũng gây xung đột. Giải pháp gồm: versioning đúng (soname theo major version), symbol versioning, giữ ABI tương thích ngược, và các cơ chế cô lập (container, rpath, hoặc liên kết tĩnh khi cần độc lập tuyệt đối).
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [linking-loading.md](linking-loading.md)
