# Memory Model — Mô hình bộ nhớ của chương trình C/C++

> **TL;DR**
> - Một process Linux có không gian địa chỉ ảo chia thành các vùng: **text** (code), **data** (biến global/static đã khởi tạo), **bss** (chưa khởi tạo), **heap** (cấp phát động, lớn dần lên), **stack** (lời gọi hàm, lớn dần xuống).
> - **Stack**: tự động, nhanh, vòng đời theo scope, nhỏ (vài MB). **Heap**: thủ công (`new`/`malloc`), linh hoạt, chậm hơn, phải tự giải phóng.
> - **Con trỏ** là một biến chứa địa chỉ — có thể null, đổi target, làm số học. **Tham chiếu** là một alias — không null, không đổi target sau khi bind.
> - Sai lầm chí mạng: dangling pointer, double free, leak, buffer overflow, dùng biến chưa khởi tạo → đa số là **Undefined Behavior (UB)**.

---

## 1. Bản chất: vì sao cần quan tâm bộ nhớ?

C/C++ không có garbage collector. Bạn **kiểm soát trực tiếp** vòng đời và vị trí của dữ liệu. Đây vừa là sức mạnh (hiệu năng, kiểm soát phần cứng — quan trọng với embedded) vừa là nguồn gốc của hầu hết bug nghiêm trọng (crash, memory corruption, security hole). Hiểu memory model là hiểu *dữ liệu của bạn sống ở đâu, sống bao lâu, ai chịu trách nhiệm dọn dẹp*.

---

## 2. Layout bộ nhớ của một process

```
Địa chỉ cao
┌─────────────────────────┐
│         Stack           │  ← lớn dần XUỐNG (địa chỉ giảm)
│           │             │     - biến local, tham số hàm, return address
│           ▼             │     - tự động cấp phát/giải phóng theo scope
├─────────────────────────┤
│           ▲             │
│           │             │
│          Heap           │  ← lớn dần LÊN (địa chỉ tăng)
│                         │     - new/malloc cấp phát ở đây
├─────────────────────────┤
│      BSS segment        │  ← global/static CHƯA khởi tạo (mặc định = 0)
├─────────────────────────┤
│      Data segment       │  ← global/static ĐÃ khởi tạo (giá trị ≠ 0)
├─────────────────────────┤
│   Text/Code segment     │  ← mã máy (thường read-only), hằng số literal
└─────────────────────────┘
Địa chỉ thấp
```

Ví dụ ánh xạ:

```cpp
int g_initialized = 42;     // data segment
int g_uninitialized;        // bss (mặc định = 0)
static int s_counter;       // bss

const char* msg = "hello";  // con trỏ 'msg' ở vùng tương ứng scope,
                            // chuỗi "hello" nằm ở vùng read-only (text/rodata)

void func() {
    int local = 1;          // stack
    int* p = new int(5);    // 'p' ở stack, vùng 4 byte chứa số 5 ở heap
    delete p;               // giải phóng vùng heap
}                           // 'local' và 'p' tự hủy khi ra khỏi scope
```

> **Lưu ý embedded:** trên hệ nhúng không MMU/RTOS, layout có thể khác (không có virtual memory), nhưng khái niệm stack/heap/data/bss vẫn áp dụng. Stack thường rất nhỏ → cẩn thận đệ quy sâu và mảng lớn trên stack.

---

## 3. Stack vs Heap

| Tiêu chí | Stack | Heap |
|----------|-------|------|
| Cấp phát | Tự động (compiler) | Thủ công (`new`/`malloc`) |
| Giải phóng | Tự động khi ra scope | Thủ công (`delete`/`free`) hoặc smart pointer |
| Tốc độ | Rất nhanh (chỉ dịch con trỏ stack) | Chậm hơn (quản lý free list, có thể fragmentation) |
| Kích thước | Nhỏ, cố định (thường ~1–8 MB) | Lớn (giới hạn bởi RAM/địa chỉ ảo) |
| Vòng đời | Theo scope `{ }` | Do lập trình viên kiểm soát |
| Lỗi điển hình | Stack overflow (đệ quy sâu, mảng lớn) | Leak, double free, fragmentation |

**Khi nào dùng gì?**
- **Mặc định dùng stack** — nhanh, an toàn, tự dọn. Hầu hết biến nên ở stack.
- **Dùng heap khi:** dữ liệu phải sống lâu hơn scope tạo ra nó; kích thước chỉ biết lúc runtime; object quá lớn cho stack; cần polymorphism qua con trỏ base.
- Trong C++ hiện đại, **đừng `new`/`delete` thủ công** — dùng `std::vector`, `std::string`, smart pointer (xem [02-modern-cpp/raii-smart-pointers.md](../02-modern-cpp/raii-smart-pointers.md)).

---

## 4. Con trỏ (pointer) vs Tham chiếu (reference)

```cpp
int a = 10, b = 20;

int* p = &a;     // p chứa địa chỉ của a
*p = 11;         // a == 11 (dereference)
p = &b;          // p trỏ sang b — được phép đổi target
p = nullptr;     // được phép null

int& r = a;      // r là alias của a — phải bind ngay, không null
r = 99;          // a == 99 (KHÔNG phải "r trỏ sang"; là gán giá trị cho a)
// int& r2;      // LỖI: reference phải khởi tạo
r = b;           // KHÔNG đổi target — gán giá trị b vào a (a == 20)
```

| | Con trỏ | Tham chiếu |
|--|---------|-----------|
| Có thể null? | Có (`nullptr`) | Không |
| Đổi target? | Có | Không (bind 1 lần) |
| Số học (pointer arithmetic)? | Có (`p+1`) | Không |
| Cần khởi tạo ngay? | Không | Có |
| Cú pháp truy cập | `*p`, `p->x` | dùng như biến gốc |

**Lựa chọn:**
- **Reference** khi: tham số hàm bắt buộc tồn tại (không null), không cần reseat → an toàn, code sạch. Dùng `const T&` để truyền object lớn mà không copy.
- **Pointer** khi: có thể không có giá trị (optional), cần đổi target, cần pointer arithmetic, làm việc với C API, hoặc cấp phát động.

---

## 5. Const & con trỏ — đọc từ phải sang trái

```cpp
const int* p;        // con trỏ tới int hằng — không sửa *p được, đổi p được
int* const p;        // con trỏ hằng tới int — sửa *p được, không đổi p
const int* const p;  // cả hai đều hằng
```

Mẹo: đọc ngược từ tên biến. `int* const p` → "p là const pointer to int".

---

## 6. Các lỗi bộ nhớ kinh điển (rất hay được hỏi)

```cpp
// 1) Dangling pointer — trỏ tới vùng đã giải phóng / hết scope
int* dangling() {
    int x = 5;
    return &x;        // ❌ x chết khi hàm return → UB khi dùng
}

// 2) Memory leak — cấp phát mà không giải phóng
void leak() {
    int* p = new int(5);
    return;           // ❌ quên delete → rò rỉ bộ nhớ
}

// 3) Double free
int* p = new int(5);
delete p;
delete p;             // ❌ UB (giải phóng 2 lần). Nên gán p = nullptr sau delete

// 4) Buffer overflow
int arr[3];
arr[5] = 1;           // ❌ ghi ra ngoài vùng hợp lệ → corruption/UB

// 5) Use of uninitialized
int x;
if (x == 0) {...}     // ❌ x có giá trị rác
```

> Công cụ phát hiện: **AddressSanitizer (ASan)**, **Valgrind** — xem chi tiết ở [09-debugging/memory-bugs.md](../09-debugging/memory-bugs.md).

---

## 7. Undefined Behavior (UB) — khái niệm cốt lõi

**UB** là tình huống mà chuẩn C++ *không định nghĩa* điều gì xảy ra. Compiler được tự do làm bất cứ gì: crash, ra kết quả sai, hoặc *trông như chạy đúng* rồi hỏng ở nơi khác. Nguy hiểm vì nó có thể "may mắn đúng" lúc test rồi sập lúc deploy.

Ví dụ UB phổ biến: dereference null/dangling, truy cập ngoài mảng, integer overflow (signed), data race, dùng biến chưa khởi tạo, vi phạm strict aliasing.

**Tư duy đúng:** không bao giờ dựa vào "lúc thử thấy chạy được". UB = sai, kể cả khi chưa crash.

---

## 8. `malloc/free` (C) vs `new/delete` (C++)

| | `malloc`/`free` | `new`/`delete` |
|--|-----------------|----------------|
| Gọi constructor/destructor? | **Không** | **Có** |
| Trả về | `void*` (phải cast) | con trỏ đúng kiểu |
| Báo lỗi khi hết bộ nhớ | trả `NULL` | ném `std::bad_alloc` |
| Cấp mảng | `malloc(n*sizeof)` | `new T[n]` / `delete[]` |

Quy tắc: cặp đôi phải khớp — `new`↔`delete`, `new[]`↔`delete[]`, `malloc`↔`free`. Trộn lẫn (vd `delete` cho `malloc`) là UB.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Sự khác nhau giữa stack và heap? Khi nào dùng cái nào?</summary>

Stack cấp phát/giải phóng tự động theo scope, rất nhanh, kích thước nhỏ, vòng đời theo `{}`. Heap cấp phát thủ công (`new`/`malloc`), linh hoạt về vòng đời và kích thước, chậm hơn, có thể fragmentation. Dùng stack mặc định; dùng heap khi dữ liệu cần sống lâu hơn scope, kích thước biết lúc runtime, object lớn, hoặc cần polymorphism. C++ hiện đại nên dùng container/smart pointer thay vì quản lý heap trực tiếp.
</details>

<details><summary>2) Con trỏ và tham chiếu khác nhau thế nào?</summary>

Con trỏ là biến chứa địa chỉ: có thể null, đổi target, làm pointer arithmetic, có thể không khởi tạo. Tham chiếu là alias của một object đã tồn tại: không null, phải bind ngay khi khai báo, không reseat được. Reference an toàn và sạch hơn cho tham số hàm bắt buộc tồn tại; pointer cần khi giá trị có thể không có (optional), cần reseat, làm việc với C API hoặc cấp phát động.
</details>

<details><summary>3) Đoạn code sau có vấn đề gì?
```cpp
int* getValue() {
    int x = 42;
    return &x;
}
```
</summary>

Trả về địa chỉ của biến local `x`. Khi hàm return, `x` ra khỏi scope và bị hủy khỏi stack → con trỏ trả về là **dangling pointer**. Dereference nó là **Undefined Behavior** (có thể đọc giá trị rác, hoặc trông đúng rồi hỏng về sau). Sửa: trả về theo giá trị (`int`), hoặc cấp phát trên heap và quản lý bằng smart pointer.
</details>

<details><summary>4) Undefined Behavior là gì? Vì sao nguy hiểm?</summary>

UB là hành vi mà chuẩn C++ không định nghĩa — compiler được phép làm bất cứ điều gì. Nguy hiểm vì chương trình có thể *trông như chạy đúng* trong lúc test (do may mắn về layout bộ nhớ/optimization) rồi crash hoặc cho kết quả sai khi đổi compiler, mức tối ưu, hoặc môi trường. Ví dụ: dereference null/dangling, truy cập ngoài mảng, signed integer overflow, data race. Quy tắc: UB là sai kể cả khi chưa thấy lỗi.
</details>

<details><summary>5) `new`/`delete` khác `malloc`/`free` ở đâu?</summary>

`new` gọi constructor và trả về con trỏ đúng kiểu, ném `std::bad_alloc` khi thất bại; `malloc` chỉ cấp vùng nhớ thô, trả `void*`, trả `NULL` khi thất bại, không gọi constructor. `delete` gọi destructor; `free` thì không. Phải khớp cặp: `new`↔`delete`, `new[]`↔`delete[]`, `malloc`↔`free`; trộn lẫn là UB.
</details>

<details><summary>6) Phân biệt `const int* p`, `int* const p`, `const int* const p`.</summary>

- `const int* p`: con trỏ tới int **hằng** — không sửa `*p`, được đổi `p`.
- `int* const p`: con trỏ **hằng** tới int — được sửa `*p`, không đổi `p`.
- `const int* const p`: cả giá trị trỏ tới lẫn con trỏ đều không sửa được.

Mẹo đọc từ phải sang trái quanh tên biến.
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [oop.md](oop.md)
