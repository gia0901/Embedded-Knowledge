# Phiên mock — 2026-08-03 · rapid · track cpp-system

- **Điểm trung bình:** 3.25 / 4
- **Bối cảnh:** Buổi 1 của [datalogic-plan](../../study-plans/datalogic-plan.md) (Tuần 1 — C++17). Warm-up cpp-system.

| ID | Câu (tóm tắt) | Tự đánh giá | Điểm | Ghi chú (thiếu gì / lỗ hổng) |
|----|---------------|-------------|------|------------------------------|
| CPP-002 | struct vs class | ok | 3 | đúng default access; khung nên là "chỉ khác default, còn lại giống hệt" thay vì "data dễ/phức tạp" |
| DP-001 | SOLID | ok | 4 | đủ 5, bản chất gọn |
| CPP-009 | template ở header | lúng túng | 2 | **nói "runtime" trước khi tự sửa** → phải phản xạ "compile-time instantiation"; "inline" gây nhầm |
| CPP-029 | emplace vs push_back | ok | 4 | forwarding + construct-in-place + case dựng-sẵn (huề) + initializer_list — **gỡ khỏi weak** |
| CPP-027 | RAII exception safety | ok | 4 | nêu stack unwinding rõ |
| CPP-046 | override/final | ok | 4 | đúng lợi ích: compiler bắt sai chữ ký |
| OS-005 | context switch | ok | 3 | ý đúng nhưng gọi "flush data cache" — chính xác là đổi page table + **flush TLB** |
| OS-003 | deadlock Coffman | ok | 3 | 🔼 từ 0 → đủ 4 điều kiện + lock ordering phá circular wait |
| OS-007 | mutex vs semaphore | một phần | 2 | thiếu **ownership (ai lock nấy unlock)** + **priority inheritance**; "chưa rõ" priority inversion |
| DP-002 | Singleton hiện đại | ok | 3 | 🔼 từ 2 → nêu đúng hành vi; thiếu tên "guard variable" + private ctor/=delete |
| DBG-001 | -g / -O0 | ok | 3 | đúng bản chất symbol + tắt optimize |
| BLD-001 | CMake / modern | ok | 4 | meta-build + target-based (tránh biến global) — tư duy chuẩn |

## 🔎 Chi tiết ôn — câu điểm ≤ 3

### CPP-009 — template ở header (điểm 2)
- **Thiếu/sai:** nói "runtime" trước khi tự sửa; dùng từ "inline" (đúng là *instantiation*).
- **Bank:** > "Template được compiler instantiate thành code chuyên biệt cho từng kiểu lúc **biên dịch** (không chi phí runtime). Định nghĩa phải ở header vì compiler cần thấy toàn bộ định nghĩa tại điểm sử dụng để sinh code; tách vào .cpp → các TU khác chỉ có khai báo → lỗi linker."
- **Tài liệu:** > "Compiler chỉ sinh code cho một instantiation khi nó **thấy đầy đủ định nghĩa** template tại điểm sử dụng." ([templates.md §6](../../../01-cpp-fundamentals/templates.md))
- **Chốt:** compile-time *instantiation* (không phải inline, không phải runtime); thiếu định nghĩa ở TU → **linker undefined reference**.

### OS-007 — mutex vs semaphore (điểm 2)
- **Thiếu/sai:** nêu được "đếm/signaling" nhưng trượt **ownership** + **priority inheritance** ("chưa rõ" priority inversion).
- **Bank:** > "Mutex có ownership (ai lock thì chính nó unlock)... Semaphore là bộ đếm wait/signal **không ownership**... Dùng binary semaphore thay mutex làm **mất priority inheritance** và dễ lỗi."
- **Tài liệu:** > "Mutex là cơ chế khóa có **ownership**: thread nào lock thì **chính nó phải unlock**... Semaphore... không có ownership — bất kỳ thread nào cũng có thể signal." ([sync-primitives.md:146](../../../03-operating-system/sync-primitives.md#L146)); > "dùng binary semaphore thay mutex... mất priority inheritance" ([:74](../../../03-operating-system/sync-primitives.md#L74))
- **Chốt:** mutex = *lock có chủ*; semaphore = *đếm/báo hiệu không chủ*. **Priority inheritance**: task giữ mutex được nâng tạm ưu tiên để nhả nhanh → tránh priority inversion; semaphore không có.

### OS-005 — context switch (điểm 3)
- **Thiếu/sai:** gọi "flush data cache" — chính xác là đổi **page table** + **flush TLB**.
- **Bank:** > "Switch giữa hai process còn phải đổi không gian địa chỉ (**đổi page table**) và thường **flush TLB**, làm cache/TLB lạnh → đắt. Thread cùng process dùng chung address space nên bỏ qua bước này."
- **Chốt:** thuật ngữ đúng = "đổi page table + flush TLB", không phải data cache.

## Tổng kết
- **Điểm mạnh:** modern C++ (RAII/move/emplace/override), SOLID, CMake tư duy rất chắc; deadlock tiến bộ lớn (0→3).
- **Lỗ hổng ưu tiên:**
  1. Template = **compile-time instantiation** (CPP-009) — nói nhầm runtime.
  2. Mutex vs semaphore — **ownership + priority inheritance** (OS-007).
  3. Thuật ngữ context switch — **TLB/page table** không phải data cache (OS-005).
- **Cập nhật weak-register:** gỡ CPP-029 (4,4 hai lần liên tiếp); OS-003 3 (cần 1 lần ≥3 nữa); DP-002 3 (cần 1 lần ≥3 nữa); +CPP-009 (2); +OS-007 (2).
- **Bank:** không phát sinh câu mới (cả 12 câu đều có sẵn trong bank).
- **Gợi ý buổi kế:** theo plan Tuần 1 buổi 2 (`/mock by-level 🟡 track modern-cpp`); xen `/mock weak-review` để vá CPP-009, OS-007.
