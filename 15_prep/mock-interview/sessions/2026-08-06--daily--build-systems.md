# Phiên mock — 2026-08-06 · daily · track build-systems

- **Level:** mid-level · **Số câu:** 6 · **Thời lượng:** ~20 phút
- **Điểm trung bình:** 3.17 / 4
- **Bối cảnh:** Tuần 1 **Buổi 4** của [datalogic-plan](../../study-plans/datalogic-plan.md) — CMake **tư duy target-based** (không cú pháp) + chạm Yocto. Slot revisit dùng cho CPP-019 theo cờ ⚠️ của plan.

## Kết quả từng câu (nhìn nhanh)
| ID | Câu (tóm tắt) | Điểm | Ghi chú ngắn |
|----|---------------|------|--------------|
| BLD-001 | CMake vs make · Modern CMake | 4 | meta-build + target-based + generator; chỉ lệch chữ "generator là thành phần sinh file build, chọn bằng `-G`" |
| BLD-002 | PUBLIC/PRIVATE/INTERFACE | 4 | đủ 3, đúng trục "usage requirement lan cho ai"; nêu được hại của lạm dụng PUBLIC |
| BLD-003 | Cross-compile · toolchain file ⭐ | 3 | phần chính tốt; **lệch ca "muốn tìm ở host"** — đúng là *tool chạy lúc build* (protoc/flex), không phải mock test |
| BLD-006 | sstate-cache | 3 | chẩn đoán đúng (sửa `tmp/work`); thiếu **lệnh** `bitbake -c cleansstate` + `devtool modify` |
| BLD-009 | SDK / eSDK | 3 | đúng thành phần & người dùng; lý do mới ở mức "nhẹ/gọn", thiếu **tách vai** + **khớp đúng image** |
| CPP-019 | memory order (revisit) | 2 🔼 | từ 1 — kể đủ 3 mức + mặc định; **định nghĩa ngược**, `relaxed` gọi là "không an toàn", **không trả lời được release/acquire đảm bảo gì** |

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-019 — memory order (điểm 2, revisit từ 1) 🔴 *vẫn là câu yếu nhất plan*
- **Tiến bộ:** kể đủ 3 mức, đúng thứ tự chi phí, đúng mặc định `seq_cst` (lần trước không nhớ tên mức nào).
- **Còn lệch 3 chỗ:**
  1. **Định nghĩa ngược** — nói memory order *"là việc thay đổi thứ tự thực hiện lệnh"*. Reorder là thứ CPU/compiler **tự làm**; memory order là **ràng buộc để hạn chế** nó. Nói đúng: *"mức đảm bảo về thứ tự mà bạn yêu cầu quanh một thao tác atomic"*.
  2. **`relaxed` KHÔNG phải "không an toàn"** — nó **vẫn atomic** (không đọc ra giá trị rách), chỉ không hứa gì về **thứ tự**. Dùng đúng chỗ (counter thống kê độc lập) là hoàn toàn an toàn.
  3. ⭐ **Không trả lời được release/acquire đảm bảo gì** — đây là phần cốt lõi còn thiếu.
- **Bank:** > "**release** = 'công bố': mọi lệnh ghi **trước** nó (kể cả ghi **thường**, không atomic) được đảm bảo **nhìn thấy được** bởi thread nào `acquire` thành công **trên cùng biến atomic đó**." ([CPP-019](../bank/cpp.md) — bản viết lại 2026-08-05, có mẫu publish/subscribe)
  ```cpp
  data = 42;                                    // ghi THƯỜNG
  ready.store(true, std::memory_order_release); // ✅ công bố mọi ghi phía trên
  // thread khác:
  if (ready.load(std::memory_order_acquire))    // ✅ thấy true => CHẮC CHẮN thấy data == 42
      use(data);
  // ❌ cả hai dùng relaxed: thấy ready == true nhưng data vẫn có thể là rác
  ```
- **Tài liệu:** > "Đồng bộ cặp đôi: release 'công bố', acquire 'nhìn thấy' — đủ cho hầu hết pattern producer/consumer" ([concurrency.md §6](../../../02-modern-cpp/concurrency.md#L121))
- **Chốt:** `release` = bấm nút **publish**, `acquire` = **subscribe**. Nội dung viết trước lúc publish thì người subscribe chắc chắn đọc được — **kể cả nội dung đó không phải biến atomic**. Đó chính là điều `relaxed` **không** cho. *(Lỗ hổng cũ vẫn chưa lấp: ai reorder = **CPU và compiler**, không chỉ compiler.)*

### BLD-003 — cross-compile · toolchain file (điểm 3)
- **Thiếu/sai:** phần chính tốt (`CMAKE_SYSTEM_*`, sysroot, cross compiler, chỉnh đường tìm, lý do tách khỏi `CMakeLists.txt`); triệu chứng khi thiếu `FIND_ROOT_PATH_MODE` đúng cả 3 mức. **Lệch ở câu "khi nào muốn tìm ở host"** — trả lời "để mock test trên host" (đó là use case build native khác), không phải ca mà cờ này phục vụ.
- **Ca đúng:** **công cụ chạy *trong lúc* build phải là bản của host** — code generator (`protoc`, `flex`, `bison`, tool tự viết) phải chạy trên máy build x86, không phải binary ARM.
- **Bank:** > ```cmake
  > set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BEFORE)  # chương trình: chạy trên HOST -> lấy host
  > set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # thư viện: CHỈ sysroot
  > set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # header:   CHỈ sysroot
  > ```
  ([BLD-003](../bank/build-systems.md))
- **Chốt:** `PROGRAM` ≠ `LIBRARY/INCLUDE` là **cố ý**. Project cần tool sinh code → tool đó phải build **cho host** → dấu hiệu cần chia build làm hai giai đoạn.

### BLD-006 — sstate-cache (điểm 3)
- **Thiếu/sai:** khái niệm + chẩn đoán đúng (sửa thẳng `tmp/work` nên hash không đổi → BitBake dùng lại cache). Thiếu **lệnh cụ thể** — "clear cache rồi build lại" là ý đúng nhưng phỏng vấn BSP chờ đúng chữ.
- **Bank:** > "Xử lý: sửa đúng chỗ (recipe/layer) để hash đổi, hoặc **`bitbake -c cleansstate <recipe>`** rồi build lại." ([BLD-006](../bank/build-systems.md))
- **Tài liệu:** > "**sstate stale**: build 'không nhận thay đổi' → `bitbake -c cleansstate <recipe>` rồi build lại." ([yocto.md:58](../../../06-build-systems/yocto.md#L58))
- **Chốt:** cặp từ khoá mức senior — **`cleansstate` để thoát, `devtool modify` để làm việc**. `cleansstate` là chữa cháy; cách đúng là sửa ở recipe/layer để hash tự đổi, còn muốn lặp nhanh khi đang phát triển thì `devtool modify <recipe>`.

### BLD-009 — SDK / eSDK (điểm 3)
- **Thiếu/sai:** đúng thành phần (toolchain + sysroot; eSDK thêm devtool) và đúng người dùng (đội app). Lý do nêu ra ("cây build nặng, phần nhiều không cần") là lý do **nhẹ nhất**.
- **Hai lý do mạnh hơn:** (1) **tách vai + không cần biết Yocto** — đội app `source environment-setup-*` rồi `cmake`/`make` như thường, không phải học BitBake, không dựng build tree hàng chục GB; (2) ⭐ **khớp đúng ảnh đang chạy** — SDK sinh **từ chính image đó** nên toolchain/libc/version thư viện trong sysroot khớp chính xác rootfs trên board → chặn lớp bug "máy em build chạy được, trên board thì không".
- **Bank:** > "Đây là cách **tách vai**: đội BSP dựng distro, đội app dùng SDK." ([BLD-009](../bank/build-systems.md))

### Ghi chú câu điểm 4
- **BLD-001:** chỉnh thuật ngữ — generator là **thành phần của CMake sinh ra file build**, chỉ định bằng `-G Ninja`; không phải "chọn build system phù hợp với nền tảng".
- **BLD-002:** không thiếu gì. Quy tắc quyết định đã đúng: foo trong **header công khai** → `PUBLIC`; chỉ trong `.cpp` → `PRIVATE`; mình không dùng nhưng consumer cần → `INTERFACE`.

## Tổng kết
- **Điểm mạnh:** CMake rất chắc (BLD-001, BLD-002 đều 4) và trả lời đúng theo **tư duy** — usage requirement lan cho ai, vì sao target-based — khớp đúng trọng tâm plan đặt cho buổi này. Cross-compile vững, chỉ lệch một nhánh phụ. **Mảng build-systems của JD coi như đạt.**
- **Lỗ hổng ưu tiên (top 3):**
  1. **CPP-019 memory order** — yếu nhất toàn plan (1 → 2). Phần chưa vào đầu: **release/acquire đảm bảo gì** → [CPP-019 bank](../bank/cpp.md) + [concurrency.md §6](../../../02-modern-cpp/concurrency.md#L114).
  2. **Yocto — từ khoá thực chiến**: `bitbake -c cleansstate`, `devtool modify`. Khái niệm đã hiểu, thiếu lệnh.
  3. **`FIND_ROOT_PATH_MODE_PROGRAM` vs `LIBRARY/INCLUDE`** — vì sao hai loại tìm ở hai nơi.
- **Câu mới thêm vào bank:** không có (mọi câu tự phát đều là follow-up của câu đã có).
- **Cập nhật weak-register:** CPP-019 lên 2 (1, 2 — **vẫn trong sổ**, chưa có lần nào ≥3); các câu khác giữ nguyên (không hỏi phiên này).

## Phiên kế đề xuất
- **Tuần 1 Buổi 5:** `/mock coding track dsa` (reverse list, ring buffer, two-sum) — viết vào [coding-arena/](../coding-arena/).
- ⚠️ Trước đó nên chạy `/mock weak-review` gọn: **CPP-019** (2 phiên liên tiếp chưa đạt 3) + tiện thể **CPP-024**, **CPP-045**, **CPP-032** đang chờ.
