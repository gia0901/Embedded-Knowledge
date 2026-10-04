# C — Lập trình C thuần: con trỏ, mảng, chuỗi, macro, linkage

> Domain `C`. Phần **C thuần** hay bị hỏi trên giấy ở vòng technical Embedded/System. Hai nửa: **A–G** đo năng lực **vẽ được bộ nhớ** (con trỏ 1–2 chiều, mảng, chuỗi, con trỏ hàm); **H–M** đo **những thứ chỉ C mới có** (preprocessor, bit viết tay, tự viết hàm chuẩn, tuần tự hoá byte, linkage, xử lý lỗi kiểu kernel); **N** là **đọc code** — *"in ra gì / sai ở đâu"*.
> Track dùng: `c`, `bsp`, `cpp-system`. 🏗️ = câu thiết kế/tình huống.
> Sinh ra từ phản hồi buổi phỏng vấn thật 2026-10 (*"C programming vẫn được hỏi: con trỏ 1–2 chiều, mảng con trỏ…"*).
>
> **Tài liệu học nền:** [c-pointers-arrays.md](../../../01-cpp-fundamentals/c-pointers-arrays.md) (A–G) · [c-language-idioms.md](../../../01-cpp-fundamentals/c-language-idioms.md) (H, J–N) · [bare-metal-c §2](../../../08-embedded-systems/bare-metal-c.md) (I). Mọi output trong đáp án là **output thật** (gcc 11.4, x86-64, `-std=c11 -Wall -Wextra`).
> 📑 Thứ tự theo **chủ đề** (mục A, B, C…), không theo số ID — thêm câu mới đặt vào đúng mục ([vì sao](README.md#-id--vị-trí-trong-file)).

| Mảng (câu) | Tài liệu học |
|---|---|
| A. Con trỏ & số học con trỏ (001–003) | [c-pointers-arrays §1](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| B. Mảng vs con trỏ — decay (004–006) | [§2](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| C. Đọc khai báo (007–008) | [§3](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| D. Con trỏ tới con trỏ (009–011) | [§4](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| E. Mảng hai chiều (012–014) | [§5](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| F. Chuỗi (015–017) | [§6](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| G. Con trỏ hàm & `void *` (018–020) | [§7](../../../01-cpp-fundamentals/c-pointers-arrays.md) |
| H. Preprocessor & macro (021–025) | [c-language-idioms §1](../../../01-cpp-fundamentals/c-language-idioms.md) |
| I. Thao tác bit viết tay (026–029) | [bare-metal-c §2.1](../../../08-embedded-systems/bare-metal-c.md) |
| J. Tự viết hàm chuẩn (030–032) | [c-language-idioms §5](../../../01-cpp-fundamentals/c-language-idioms.md) |
| K. `union` & tuần tự hoá byte (033–034) | [c-language-idioms §3](../../../01-cpp-fundamentals/c-language-idioms.md) |
| L. Linkage & vòng đời biến (035–036) | [c-language-idioms §2](../../../01-cpp-fundamentals/c-language-idioms.md) |
| M. Xử lý lỗi kiểu C (037) | [c-language-idioms §4](../../../01-cpp-fundamentals/c-language-idioms.md) |
| **N. 🔎 Đọc code — output là gì, sai ở đâu (038–051)** | [c-language-idioms §6](../../../01-cpp-fundamentals/c-language-idioms.md) |

**Câu C đã có ở domain khác (không chép lại — track `c` rút cả những câu này):** [CPP-003](cpp.md) `const` với con trỏ · [CPP-060](cpp.md) trả địa chỉ biến cục bộ · [CPP-033](cpp.md) `malloc` vs `new` · [EMB-001…004](embedded-fundamentals.md) set/clear bit, `stdint.h`, `volatile`, `static/extern` (mức định nghĩa) · [EMB-037/038](embedded-fundamentals.md) integer promotion, padding · [coding.md](coding.md): COD-005 `memcpy`/`strlen`, COD-009 đếm bit 1, COD-010 endianness, COD-016 `memcpy` chồng lấn, COD-018 `volatile` + ISR, COD-021 mask `1u << 32`, COD-025 ép kiểu gói tin, COD-026 thứ tự đánh giá đối số.

---

## A — Con trỏ & số học con trỏ

#### C-001 · 🟢 · concept · [→ c-pointers-arrays §1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**`int *p` và `char *c` cùng trỏ vào một địa chỉ. `p + 1` và `c + 1` cách địa chỉ ban đầu bao nhiêu byte? Và `a[i]` thực chất là gì?**
<details><summary>Đáp án</summary>

`p + n` nhảy `n * sizeof(*p)` byte: `p + 1` cách **4** byte, `c + 1` cách **1** byte — kiểu con trỏ quyết định bước nhảy và số byte `*p` đọc. `a[i]` được định nghĩa là `*(a + i)`; phép cộng giao hoán nên cả `2[a]` cũng hợp lệ.
</details>

#### C-002 · 🟡 · coding · ⭐ · [→ c-pointers-arrays §1.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Với `int a[] = {10, 20, 30}; int *p = a;` — mỗi dòng sau (chạy độc lập, bắt đầu lại từ `p = a`, `a[0] = 10`) cho `x`, `*p`, `a[0]` bằng bao nhiêu?**
```c
x = *p++;
x = (*p)++;
x = ++*p;
x = *++p;
```
<details><summary>Đáp án</summary>

**Cơ chế:** `++` **hậu tố** ưu tiên cao hơn `*`; `++` **tiền tố** và `*` cùng mức, gắn từ phải sang trái. Hậu tố trả giá trị **cũ**, tiền tố trả giá trị **mới**.

| Biểu thức | Tăng cái gì | Khi nào | Output thật |
|---|---|---|---|
| `*p++` | **con trỏ** | sau khi đọc | `x=10, *p=20, a[0]=10` |
| `(*p)++` | **giá trị** | sau khi đọc | `x=10, *p=11, a[0]=11` |
| `++*p` | **giá trị** | trước khi đọc | `x=11, *p=11, a[0]=11` |
| `*++p` | **con trỏ** | trước khi đọc | `x=20, *p=20, a[0]=10` |

**Bẫy:** tưởng `*p++` tăng giá trị. **Chốt:** *"`*p++` đọc rồi dời con trỏ — đó là lõi của `while (*d++ = *s++);`."*
</details>

#### C-003 · 🟢 · concept · [→ memory-model §6](../../../01-cpp-fundamentals/memory-model.md)
**Con trỏ chưa khởi tạo, con trỏ `NULL`, con trỏ treo (dangling) — khác nhau ra sao?**
<details><summary>Đáp án</summary>

**Chưa khởi tạo:** chứa rác, trỏ đâu không biết — ghi vào là phá bộ nhớ ngẫu nhiên. **NULL:** giá trị đặc biệt "không trỏ đâu", kiểm được bằng `if (p)`; dereference thường crash ngay. **Treo:** từng hợp lệ, nhưng vùng đã `free` hoặc hết scope. Cả ba khi dereference đều là **UB**; chỉ `NULL` kiểm tra được.
</details>

---

## B — Mảng vs con trỏ (array decay)

#### C-004 · 🟡 · coding · ⭐ · [→ c-pointers-arrays §2.2](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Đoạn này in ra gì? Vì sao? Sửa thế nào?**
```c
void print_len(int a[]) {
    printf("%zu\n", sizeof(a) / sizeof(a[0]));
}
int main(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    printf("%zu\n", sizeof(arr) / sizeof(arr[0]));
    print_len(arr);
}
```
<details><summary>Đáp án</summary>

**In ra `5` rồi `2`** (x86-64: con trỏ 8 byte / int 4 byte).

**Cơ chế:** tham số `int a[]` **chính là** `int *a` — không có tham số kiểu mảng trong C. Khi gọi `print_len(arr)`, `arr` **decay** thành `&arr[0]`, hàm chỉ nhận một con trỏ ⟹ `sizeof(a)` = 8. gcc cảnh báo đúng chỗ này:
```
warning: 'sizeof' on array function parameter 'a' will return size of 'int *' [-Wsizeof-array-argument]
```

**Sửa:** truyền số phần tử kèm theo — đây là lý do mọi API C có cặp `(buf, len)`:
```c
void print_len(const int *a, size_t n);              // ✅
print_len(arr, sizeof arr / sizeof arr[0]);          // tinh o NOI MANG con nguyen
```

**Bẫy:** viết macro `#define LEN(a) (sizeof(a)/sizeof((a)[0]))` rồi dùng **trong hàm** — vẫn ra 2, im lặng. **Chốt:** *"Truyền mảng là truyền con trỏ; kích thước phải đi kèm như một tham số riêng."*
</details>

#### C-005 · 🟠 · concept · [→ c-pointers-arrays §2.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Với `int arr[5];` — `arr`, `&arr[0]` và `&arr` có cùng giá trị không? Cùng kiểu không? `+1` của mỗi cái nhảy bao nhiêu?**
<details><summary>Đáp án</summary>

**Cơ chế:** cả ba **cùng địa chỉ** (đầu mảng), nhưng **khác kiểu**, và kiểu quyết định bước nhảy.

| Biểu thức | Kiểu | `+1` nhảy |
|---|---|---|
| `arr` (decay) | `int *` | 4 byte |
| `&arr[0]` | `int *` | 4 byte |
| `&arr` | `int (*)[5]` — con trỏ tới **cả mảng** | **20 byte** |

Output thật: `(void*)arr == (void*)&arr ? 1` và `&arr+1 - &arr (byte) = 20`.

**"Vì sao" tách tầng:**
- *Nông:* "`&arr` là địa chỉ của mảng".
- *Sâu:* decay có **ba ngoại lệ** — `sizeof arr`, `&arr`, và khởi tạo mảng `char` bằng chuỗi. Ở `&arr`, `arr` **không** decay nên toán tử `&` lấy địa chỉ của **object mảng**, kiểu `int (*)[5]`. Đây cũng là kiểu mà `int a[3][5]` decay thành (C-012).

**Bẫy:** nói "`arr` là con trỏ hằng" — sai: `sizeof arr` là 20, không phải 8, và không có ô nhớ nào chứa con trỏ đó. **Chốt:** *"Cùng địa chỉ, khác kiểu — và kiểu mới là thứ quyết định số học."*
</details>

#### C-006 · 🟢 · concept · [→ c-pointers-arrays §2.2](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Vì sao `int a[3], b[3]; a = b;` không compile? Copy mảng thế nào?**
<details><summary>Đáp án</summary>

Tên mảng không phải biến chứa địa chỉ để mà gán lại — nó là chính vùng dữ liệu, và trong biểu thức gán nó decay thành một giá trị không gán được. Copy bằng `memcpy(a, b, sizeof a)`, hoặc bọc mảng trong `struct`: struct gán được, kéo theo cả mảng bên trong.
</details>

---

## C — Đọc khai báo

#### C-007 · 🟡 · concept · ⭐ · [→ c-pointers-arrays §3](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Phân biệt `int *a[3]` và `int (*a)[3]`. Vẽ bộ nhớ, cho biết `sizeof(a)` của mỗi cái trên máy 64-bit.**
<details><summary>Đáp án</summary>

**Luật phải–trái:** bắt đầu từ tên, đọc sang phải tới `)` hoặc hết, rồi sang trái.
- `int *a[3]` — a → `[3]` → `*` → int: **mảng 3 phần tử, mỗi phần tử là con trỏ** tới int.
- `int (*a)[3]` — a → `)` → `*` → `[3]` → int: **một con trỏ**, trỏ tới **mảng 3 int**.

```
int *ap[3]:   [ o ][ o ][ o ]          int (*pa)[3]:   [ o ]----> [ 7 | 8 | 9 ]
                |    |    |
                v    v    v
                x    y    z
```

Output thật: `sizeof(ap) = 24` (3 con trỏ) · `sizeof(pa) = 8` (1 con trỏ) · `pa+1` nhảy **12** byte (qua cả mảng 3 int).

**Dùng ở đâu:** `int *a[N]` là bảng tra / `argv`. `int (*a)[N]` là kiểu của **một hàng** ma trận — đúng thứ `int m[R][N]` decay thành (C-012).

**Chốt:** *"Ngoặc quanh `*a` biến 'mảng các con trỏ' thành 'con trỏ tới mảng'."*
</details>

#### C-008 · 🟠 · concept · [→ c-pointers-arrays §3.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Đọc khai báo sau, nói nó là gì, rồi viết lại cho dễ đọc:**
```c
void (*signal(int sig, void (*h)(int)))(int);
```
<details><summary>Đáp án</summary>

**Đọc từng bước (luật phải–trái, bắt đầu từ `signal`):**
1. `signal(int sig, void (*h)(int))` — `signal` là **hàm** nhận `int` và `h`.
2. `h` là **con trỏ tới hàm** nhận `int`, trả `void`.
3. Phần còn lại bọc ngoài: `void (* … )(int)` — giá trị trả về của `signal` là **con trỏ tới hàm** nhận `int`, trả `void`.

⟹ *"`signal` nhận một số hiệu signal và một handler, **trả về handler cũ**."*

**Viết lại bằng `typedef`** — đặt tên cho kiểu lặp lại:
```c
typedef void (*sighandler_t)(int);
sighandler_t signal(int sig, sighandler_t h);   // cung kieu voi ban tho
```
Đã kiểm bằng gcc: gán hàm khai báo kiểu thô vào biến kiểu `sighandler_t (*)(int, sighandler_t)` không cảnh báo gì, so sánh hai con trỏ cho `1` — cùng một kiểu.

**"Vì sao" tách tầng:**
- *Nông:* "dùng `typedef` cho gọn".
- *Sâu:* kiểu trả về là con trỏ hàm nên nó phải **bọc quanh** tên hàm — đó là lý do cú pháp trông "lộn trong ra ngoài". `typedef` tách *"kiểu handler"* thành một tên, và khai báo trở về dạng `T f(args)` quen thuộc.

**Bẫy:** đọc từ trái sang phải và nói "`signal` trả `void`". **Chốt:** *"Gặp khai báo con trỏ hàm phức tạp: đọc từ tên ra ngoài, rồi đặt `typedef` cho kiểu con trỏ hàm."*
</details>

---

## D — Con trỏ tới con trỏ

#### C-009 · 🟡 · coding · ⭐ · [→ c-pointers-arrays §4.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Sau khi gọi, `buf` vẫn là `NULL`. Vì sao? Có hậu quả gì khác? Sửa thế nào?**
```c
void alloc_buf(char *p, size_t n) { p = malloc(n); }

char *buf = NULL;
alloc_buf(buf, 16);
```
<details><summary>Đáp án</summary>

**Cơ chế:** C truyền **theo giá trị** — `p` là **bản sao** của `buf`. Gán `p = malloc(n)` chỉ sửa bản sao; hàm return thì bản sao mất ⟹ `buf` vẫn `NULL`, và **16 byte bị rò** vì không ai giữ địa chỉ.

```
buf [NULL]          p [0x5000] ---> 16 byte   (p chet khi return => leak)
```

Output thật: `sau alloc_bad: buf = (nil)` · valgrind: `definitely lost: 16 bytes in 1 blocks`. gcc cũng nhắc: `parameter 'p' set but not used`.

**Sửa — muốn hàm sửa một `T` thì truyền `T *`; ở đây `T` là `char *`:**
```c
void alloc_buf(char **pp, size_t n) { *pp = malloc(n); }   // ✅ ghi thang vao buf
alloc_buf(&buf, 16);

char *alloc_buf2(size_t n) { return malloc(n); }           // ✅ hoac tra ve
```
Trong C++: `void alloc_buf(char *&p, size_t n)` — tham chiếu tới con trỏ.

**Chốt:** *"Đổi **nội dung** vùng trỏ tới thì `char *` đủ; đổi **chính con trỏ** thì cần `char **`."*
</details>

#### C-010 · 🟠 · coding · [→ c-pointers-arrays §4.2](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Viết `remove_val(Node **head, int v)` xoá node đầu tiên có giá trị `v` khỏi danh sách liên kết đơn — KHÔNG được viết nhánh riêng cho trường hợp node đầu.**
<details><summary>Đáp án</summary>

**Cơ chế:** đừng duyệt bằng con trỏ tới **node**; duyệt bằng con trỏ tới **ô đang chứa mũi tên tới node** (`Node **`). Ô đó lúc đầu là `head`, sau đó là `->next` của node trước. Xoá = ghi đè ô đó — node đầu hay node giữa đều như nhau.

```c
typedef struct Node { int val; struct Node *next; } Node;

void remove_val(Node **head, int v) {
    Node **pp = head;
    while (*pp && (*pp)->val != v)
        pp = &(*pp)->next;               // tro vao o "next" cua node hien tai
    if (*pp) {
        Node *dead = *pp;
        *pp = dead->next;                // noi tat qua node bi xoa
        free(dead);
    }
}
```

Output thật trên danh sách `4 3 2 1`: xoá `4` (node đầu) ⟹ `3 2 1`; xoá `2` (node giữa) ⟹ `3 1`. valgrind: `All heap blocks were freed`.

**So với cách thường:**

| | Duyệt bằng `Node *prev, *cur` | Duyệt bằng `Node **pp` |
|---|---|---|
| Xoá node đầu | **Nhánh riêng** — phải sửa `head` | Cùng đường code |
| Biến phải giữ | `prev` + `cur` | Một `pp` |
| Cần truyền `Node **head` | Vẫn cần (để sửa head) | Có |

**Bẫy:** truyền `Node *head` — xoá node đầu xong caller vẫn giữ con trỏ tới node **đã free**. **Chốt:** *"Con trỏ tới con trỏ cho phép sửa 'chỗ đang trỏ vào node' mà không cần biết chỗ đó là `head` hay `->next`."*
</details>

#### C-011 · 🟢 · concept · [→ c-pointers-arrays §4.3](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**`int main(int argc, char **argv)` — `argv` nằm trong bộ nhớ thế nào? `argv[argc]` bằng gì?**
<details><summary>Đáp án</summary>

`argv` trỏ vào phần tử đầu của một **mảng các `char *`**; mỗi phần tử trỏ tới một chuỗi kết thúc `'\0'` nằm riêng, dài ngắn tuỳ ý. Vì thế kiểu là `char **`. Chuẩn C bảo đảm `argv[argc]` là **con trỏ `NULL`** — duyệt được bằng `while (*argv)`.
</details>

---

## E — Mảng hai chiều

#### C-012 · 🟡 · concept · ⭐ · [→ c-pointers-arrays §5.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**`int a[3][4]` nằm trong bộ nhớ thế nào? Compiler tính địa chỉ `a[i][j]` ra sao? Và vì sao khi truyền vào hàm, số cột bắt buộc phải ghi, còn số hàng thì không?**
<details><summary>Đáp án</summary>

**Bố cục:** **một khối liền 48 byte**, xếp **theo hàng** (row-major): hết hàng 0 mới tới hàng 1.
```
[ a00 a01 a02 a03 | a10 a11 a12 a13 | a20 a21 a22 a23 ]
```
Output thật: `sizeof(a)=48 sizeof(a[0])=16` · `&a[1][0] - &a[0][0] = 4 phan tu`.

**Công thức:** `địa chỉ a[i][j] = địa chỉ a + (i * C + j) * sizeof(int)`. Công thức **cần `C`** (số cột) nhưng **không cần `R`**.

**Vì vậy:** `a` decay thành con trỏ tới **hàng đầu**, kiểu `int (*)[4]`. Ba chữ ký hợp lệ:
```c
void f(int rows, int m[][4]);
void f(int rows, int (*m)[4]);                       // y het dong tren
void f(int rows, int cols, int m[rows][cols]);       // C99 VLA — so cot luc chay
```

**Chốt:** *"Mảng 2 chiều là mảng của các mảng; decay chỉ bóc **một** tầng, nên kiểu còn lại `int (*)[C]` phải mang theo số cột."*
</details>

#### C-013 · 🟠 · coding · ⭐ · [→ c-pointers-arrays §5.2](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Đồng nghiệp viết hàm in ma trận nhận `int **m`, rồi gọi với `int a[3][4]`. Build có cảnh báo, chạy thì crash. Giải thích từng bước vì sao crash, và đưa ra cách sửa.**
```c
void print_bad(int **m) { printf("%d\n", m[1][3]); }
int a[3][4] = { {0,1,2,3}, {10,11,12,13}, {20,21,22,23} };
print_bad(a);
```
<details><summary>Đáp án</summary>

**Output thật:**
```
warning: passing argument 1 of 'print_bad' from incompatible pointer type [-Wincompatible-pointer-types]
note: expected 'int **' but argument is of type 'int (*)[4]'
Segmentation fault (core dumped)              <- exit 139
```

**Cơ chế — từng bước:**
1. `a` decay thành `int (*)[4]` — địa chỉ đầu một khối **48 byte số nguyên**, không có con trỏ nào bên trong.
2. Trong hàm, `m` được coi là `int **`: *"trỏ tới một mảng các **con trỏ** int"*.
3. `m[1]` = *"đọc 8 byte ở offset 8"* ⟹ đọc trúng `a[0][2]` và `a[0][3]` (giá trị `2` và `3`), ghép thành một **địa chỉ rác**.
4. `m[1][3]` dereference địa chỉ rác đó ⟹ segfault.

```
a thuc te:   [ 0 ][ 1 ][ 2 ][ 3 ][10]...      (so nguyen)
m nghi rang: [ ptr0    ][ ptr1    ]...        (con tro 8 byte)
                         ^ = byte cua 2 va 3 => dia chi rac
```

**Sửa — chọn theo việc:**

| Cách | Chữ ký | Dùng khi |
|---|---|---|
| Ghi số cột | `void print(int rows, int (*m)[4])` | Số cột cố định lúc compile |
| VLA (C99) | `void print(int r, int c, int m[r][c])` | Số cột lúc chạy, compiler hỗ trợ VLA |
| Con trỏ phẳng | `void print(const int *m, int r, int c)` + `m[i*c + j]`, gọi `print(&a[0][0], 3, 4)` | Chạy mọi nơi, kể cả C++, và dùng rất rộng rãi. Đọc chặt chuẩn thì đi quá hàng đầu qua `&a[0][0]` còn tranh luận — ma trận **cấp phát phẳng** từ đầu thì không có vấn đề này |

**"Vì sao" tách tầng:**
- *Nông:* "sai kiểu".
- *Sâu:* `int **` và `int[3][4]` có **bố cục bộ nhớ khác nhau**: một bên là bảng địa chỉ, một bên là khối số. Không ép kiểu nào biến cái này thành cái kia được. C chỉ **cảnh báo** chuyển đổi con trỏ không tương thích, nên đây là lý do nên build với `-Werror`.

**Bẫy:** "sửa" bằng cách ép `print_bad((int **)a)` — hết cảnh báo, vẫn crash. **Chốt:** *"`int a[R][C]` không phải `int **`; nó decay thành `int (*)[C]`."*
</details>

#### C-014 · 🟠 · coding · ⭐ · 🏗️ · [→ c-pointers-arrays §5.3](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Viết hàm cấp phát động một ma trận `int` kích thước R×C (R, C biết lúc chạy) và hàm giải phóng tương ứng. Có hai cách phổ biến — viết cả hai, rồi chọn một và nêu lý do.**
<details><summary>Đáp án</summary>

**Cách 1 — mảng con trỏ hàng:** `R + 1` lần cấp phát; phải dọn phần đã cấp nếu hỏng giữa chừng.
```c
int **alloc_rows(int R, int C) {
    int **m = malloc(R * sizeof *m);
    if (!m) return NULL;
    for (int i = 0; i < R; i++) {
        m[i] = malloc(C * sizeof **m);
        if (!m[i]) {                        // ✅ don phan da cap, khong leak
            while (i--) free(m[i]);
            free(m);
            return NULL;
        }
    }
    return m;
}
void free_rows(int **m, int R) {
    for (int i = 0; i < R; i++) free(m[i]); // hang truoc
    free(m);                                // mang con tro sau
}
```

**Cách 2 — một khối liền, vẫn viết `m[i][j]`:**
```c
int (*m)[C] = malloc(R * sizeof *m);        // sizeof *m = C * sizeof(int)
m[i][j] = 0;
free(m);                                    // MOT lan
```
*(Hoặc phẳng hoàn toàn: `int *m = malloc(R * C * sizeof *m);` và `m[i*C + j]`.)*

Output thật (R=3, C=4): cách 1 — hàng 1 cách hàng 0 **32 byte** (16 byte dữ liệu + metadata của `malloc`, không liền); cách 2 — **16 byte**, liền. valgrind cả hai: `All heap blocks were freed`.

| | Cách 1 `int **` | Cách 2 khối liền |
|---|---|---|
| Lần `malloc`/`free` | R + 1 | **1** |
| Liền khối, thân thiện cache, `memcpy` cả ma trận | ❌ | ✅ |
| Hàng dài khác nhau | ✅ | ❌ |
| Đổi chỗ hai hàng | O(1) — đổi con trỏ | Copy dữ liệu |
| Xử lý lỗi giữa chừng | Phải dọn | Không có |

**Chọn:** *khối liền* — một lần cấp, một lần giải phóng, không có trạng thái "cấp được một nửa", và duyệt theo hàng chạy tuần tự trong bộ nhớ. Chỉ chọn cách 1 khi hàng dài khác nhau hoặc cần hoán đổi hàng rẻ.

**Bẫy:** ① quên dọn khi `malloc` hỏng giữa vòng lặp ⟹ leak · ② `free(m)` trước các `free(m[i])` ⟹ mất địa chỉ các hàng · ③ cách 2 với `C` lúc chạy là kiểu **VLA** — bắt buộc ở C99, **tuỳ chọn** ở C11, **không có** trong C++ (khi đó dùng bản phẳng `m[i*C + j]`). **Chốt:** *"Mặc định một khối liền; mảng con trỏ chỉ khi cần hàng không đều."*
</details>

---

## F — Chuỗi

#### C-015 · 🟡 · concept · ⭐ · [→ c-pointers-arrays §6.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**`char s1[] = "abc";` và `char *s2 = "abc";` — mỗi cái nằm ở đâu, `sizeof` bằng bao nhiêu, và `s1[0] = 'X'` / `s2[0] = 'X'` có hợp lệ không?**
<details><summary>Đáp án</summary>

| | `char s1[] = "abc"` | `char *s2 = "abc"` |
|---|---|---|
| Là gì | **Mảng** 4 byte (gồm `'\0'`), **copy** từ literal | **Con trỏ** tới literal |
| Nằm ở đâu | Stack (nếu là biến cục bộ) | Con trỏ ở stack; chuỗi ở vùng **chỉ đọc** (`.rodata`) |
| `sizeof` | **4** | **8** |
| Ghi `[0] = 'X'` | ✅ | ❌ **UB** — thực tế segfault |

Output thật: `sizeof(s1)=4 sizeof(s2)=8` · `s1 = Xbc` · ghi `s2[0]` ⟹ `Segmentation fault`, exit 139.

**Phòng:** khai báo đúng kiểu `const char *s2 = "abc";` ⟹ ghi vào thành **lỗi compile**. Trong C++ literal có kiểu `const char[4]`; chuẩn C++11 cấm gán vào `char *`, nhưng g++ 11 chỉ cảnh báo `ISO C++ forbids converting a string constant to 'char*'`.

**Chốt:** *"Mảng thì copy và sửa được; con trỏ tới literal thì chỉ đọc — luôn khai `const char *`."*
</details>

#### C-016 · 🟡 · concept · [→ c-pointers-arrays §6.2](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**So sánh `char *names[] = {"go", "rust", "c"};` với `char names[][10] = {"go", "rust", "c"};` — bộ nhớ, `sizeof`, và khi nào chọn cái nào.**
<details><summary>Đáp án</summary>

- `char *names[]` là **mảng 3 con trỏ**, mỗi con trỏ tới một literal nằm riêng ở vùng chỉ đọc ⟹ `sizeof` = 3 × 8 = **24**.
- `char grid[][10]` là **khối 3 × 10 byte liền**, mỗi hàng chứa một bản copy có đệm ⟹ `sizeof` = **30**.

Output thật: `sizeof(names)=24 sizeof(grid)=30`.

| | `char *names[]` | `char grid[][10]` |
|---|---|---|
| Sửa ký tự | ❌ literal chỉ đọc | ✅ |
| Chuỗi dài hơn 9 ký tự | ✅ | ❌ |
| Lãng phí | Không đệm | Đệm tới 10 byte mỗi hàng |
| Đổi chỗ hai phần tử | O(1) — đổi con trỏ | Copy byte |

**Chọn:** bảng tên hằng (thông điệp lỗi, tên lệnh) ⟹ `const char *names[]`. Buffer cố định cần sửa (bảng tên thiết bị đọc từ cấu hình) ⟹ mảng 2 chiều.

**Chốt:** *"Cùng cú pháp khởi tạo, hai bố cục: bảng con trỏ so với khối ký tự."*
</details>

#### C-017 · 🟠 · coding · [→ c-pointers-arrays §6.3](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Hàm sau có lỗi gì? Đưa ra ba cách sửa và đánh đổi của từng cách.**
```c
char *fmt_id(int id) {
    char buf[16];
    snprintf(buf, sizeof buf, "ID-%04d", id);
    return buf;
}
```
<details><summary>Đáp án</summary>

**Lỗi:** `buf` là biến cục bộ — **chết khi hàm return**. Con trỏ trả về là con trỏ **treo**; dùng nó là **UB**.

**Output thật:** gcc cảnh báo `function returns address of local variable [-Wreturn-local-addr]`, và **gcc 11.4 cố ý trả về `NULL`** thay cho địa chỉ đó (in thử: `(nil)` ở cả `-O0` lẫn `-O2`) ⟹ `printf("%s")` segfault ngay. Compiler khác có thể trả địa chỉ stack thật ⟹ in rác, hoặc *"chạy đúng"* lúc test rồi hỏng khi có lời gọi hàm khác ghi đè stack (xem [CPP-060](cpp.md)).

**Ba cách sửa:**

| Cách | Chữ ký | Đánh đổi |
|---|---|---|
| ✅ **Caller cấp buffer** | `int fmt_id(int id, char *out, size_t n)` | Rõ ai sở hữu, không cấp phát; cách chuẩn của API C (`snprintf`, `strerror_r`) |
| `static char buf[16]` | Giữ nguyên | **Không reentrant, không thread-safe**; lời gọi sau ghi đè kết quả lời gọi trước — `printf("%s %s", fmt_id(1), fmt_id(2))` in hai lần cùng một chuỗi — output thật: `ID-0001 ID-0001` |
| `malloc` | Giữ nguyên, trả vùng heap | Caller phải nhớ `free` — dễ leak; tốn một lần cấp phát |

**"Vì sao" tách tầng:**
- *Nông:* "biến local chết khi return".
- *Sâu:* câu hỏi thật là **ai sở hữu bộ nhớ của kết quả**. Ba cách sửa là ba câu trả lời: caller sở hữu · không ai sở hữu (dùng chung) · callee cấp, caller giải phóng. Thư viện C chuẩn từng chọn `static` (`strtok`, `asctime`) và phải thêm bản `_r` sau đó chính vì thread-safety.

**Chốt:** *"Trong C, hàm trả chuỗi thì để **caller cấp buffer** — sở hữu rõ ràng, không cấp phát, thread-safe."*
</details>

---

## G — Con trỏ hàm & `void *`

#### C-018 · 🟡 · coding · ⭐ · [→ c-pointers-arrays §7.2](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Viết comparator cho `qsort` để sắp mảng `int` tăng dần. Vì sao cách viết ngắn gọn `return *(int*)a - *(int*)b;` nguy hiểm?**
<details><summary>Đáp án</summary>

```c
int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);                 // ✅ -1, 0, 1 — khong tran so
}
qsort(v, n, sizeof v[0], cmp_int);
```

**Vì sao `a - b` nguy hiểm:** hiệu hai `int` trái dấu và xa nhau **tràn số** (`1 - INT_MIN`). Tràn số có dấu là **UB**; thực tế nó quấn vòng thành số âm ⟹ comparator nói ngược ⟹ `qsort` cho kết quả sai, không báo gì.

Output thật, sắp `{INT_MIN, 1, INT_MAX, 0, -5}`:
```
cmp_bad: 1 2147483647 -2147483648 -5 0         <- sai
cmp_ok : -2147483648 -5 0 1 2147483647
```

**Ba chi tiết khác hay bị hỏi:** tham số là `const void *` vì `qsort` tổng quát cho mọi kiểu · sắp mảng **struct** theo một field thì ép sang `const struct T *` rồi so field đó · sắp **mảng con trỏ** (`char *names[]`) thì phần tử là `char *` ⟹ tham số thật là `char *const *`, phải dereference **thêm một lần** trước khi `strcmp`.

**Chốt:** *"Comparator trả dấu, không trả hiệu: `(x > y) - (x < y)`."*
</details>

#### C-019 · 🟡 · concept · [→ c-pointers-arrays §7.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**Thay một `switch` lớn theo mã lệnh bằng một mảng con trỏ hàm (bảng dispatch). Khi nào nên làm vậy, và nó mang theo rủi ro gì?**
<details><summary>Đáp án</summary>

```c
typedef int (*handler_t)(const uint8_t *payload, size_t len);
static const handler_t handlers[OPCODE_MAX] = {
    [OP_READ]  = handle_read,
    [OP_WRITE] = handle_write,
};
if (op < OPCODE_MAX && handlers[op]) handlers[op](p, n);   // ✅ kiem bien + kiem NULL
```

**Nên dùng khi:** nhiều mã lệnh, mỗi mã một hàm độc lập · cần **đăng ký/thay** hàm lúc chạy (driver điền bảng của nó) · muốn thêm lệnh mà không sửa hàm dispatch.

**Rủi ro — đúng hai bệnh của vtable, vì nó là vtable viết tay:**

| Rủi ro | Hậu quả | Chữa |
|---|---|---|
| Mã lệnh ngoài phạm vi | Đọc ngoài mảng ⟹ nhảy tới địa chỉ rác | Kiểm `op < OPCODE_MAX` |
| Slot chưa điền (`NULL`) | Gọi `NULL` ⟹ crash | Kiểm `NULL`, hoặc điền sẵn stub trả lỗi (**Null Object**) |
| Chèn slot vào giữa struct ops dùng chung giữa hai module build riêng | Mọi slot sau lệch ⟹ gọi nhầm hàm | Chỉ thêm vào **cuối**; thêm trường `size`/`version` |

Đây chính là mô hình `file_operations` của kernel và bảng `panel_ops` ở [in-practice/A1 §6](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) (câu [DP-043](design-patterns.md)).

**Chốt:** *"Bảng con trỏ hàm là đa hình của C — rẻ và mở rộng được, nhưng phải tự kiểm biên và slot rỗng mà vtable của C++ lo hộ."*
</details>

#### C-020 · 🟡 · concept · [→ c-pointers-arrays §7.3](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**`void *` là gì? Làm được gì, không làm được gì? Vì sao trong C không nên ép kiểu kết quả `malloc`, còn C++ thì bắt buộc?**
<details><summary>Đáp án</summary>

**`void *`** = địa chỉ **không mang kiểu** — dùng cho API tổng quát (`malloc`, `memcpy`, `qsort`, `pthread_create`).

| Làm được | Không làm được |
|---|---|
| Chứa địa chỉ của **mọi** kiểu dữ liệu | **Dereference** — không biết đọc bao nhiêu byte |
| Trong C: gán **ngầm** qua lại với `T *` | **Số học** — không biết bước nhảy. `p + 1` chỉ chạy nhờ **extension GNU**; gcc im lặng với `-Wall -Wextra`, chỉ cảnh báo khi bật `-Wpedantic` |

**`malloc` trong C:** `int *p = malloc(n * sizeof *p);` — **không ép**. Ép kiểu thừa, và ở C89 còn **che lỗi** quên `#include <stdlib.h>` (khi đó `malloc` bị ngầm coi là trả `int`, mất nửa trên địa chỉ 64-bit).

**Trong C++:** `void *` → `T *` **không** ngầm định — phải `static_cast`, vì đó là chuyển đổi làm mất an toàn kiểu. (Và C++ nên dùng `new`/container thay `malloc`.)

**Chốt:** *"`void *` là địa chỉ không có kiểu: chuyền đi được, không đọc và không cộng được cho tới khi gán lại cho một kiểu cụ thể."*
</details>

---

## H — Preprocessor & macro

#### C-021 · 🟡 · coding · ⭐ · [→ c-language-idioms §1.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**Với `#define SQ(x) x * x` và `int a = 2;` — `SQ(a + 1)` và `10 / SQ(2)` cho ra bao nhiêu? Sửa macro thế nào?**
<details><summary>Đáp án</summary>

**Cơ chế:** preprocessor **thay chữ** rồi compiler mới tính, theo thứ tự ưu tiên toán tử bình thường.

| Lời gọi | Sau khi thay chữ | Output thật |
|---|---|---|
| `SQ(a + 1)` | `a + 1 * a + 1` | **5** (mong 9) |
| `10 / SQ(2)` | `10 / 2 * 2` | **10** (mong 2) |

**Sửa — hai lớp ngoặc, mỗi lớp chữa một lỗi:**
```c
#define SQ(x) ((x) * (x))   // ngoac quanh TUNG tham so (loi 1) + quanh CA bieu thuc (loi 2)
```
Output thật sau khi sửa: `SQ(a+1) = 9`, `10/SQ(2) = 2`.

**Bẫy còn lại mà ngoặc không chữa được:** `SQ(i++)` thành `((i++) * (i++))` — sửa `i` hai lần không có điểm tuần tự ở giữa là **UB** (xem C-022).

**Chốt:** *"Macro là thay chữ: ngoặc quanh từng tham số, ngoặc quanh cả biểu thức — và đừng truyền biểu thức có tác dụng phụ."*
</details>

#### C-022 · 🟡 · concept · [→ c-language-idioms §1.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**`#define MAX(a, b) ((a) > (b) ? (a) : (b))` — đã đủ ngoặc. Với `x = 5, y = 3`, `m = MAX(x++, y)` cho `m` và `x` bằng bao nhiêu? Khi nào nên dùng macro thay vì hàm `static inline`?**
<details><summary>Đáp án</summary>

**Output thật:** `m = 6`, `x = 7`. Thay chữ ra `((x++) > (y) ? (x++) : (y))`: `x++` chạy một lần khi so sánh (5 > 3, `x` thành 6) và một lần nữa ở nhánh được chọn (trả 6, `x` thành 7). Ngoặc không chữa được — **tham số xuất hiện hai lần trong thân thì bị đánh giá hai lần**.

| | Macro | `static inline` |
|---|---|---|
| Kiểm kiểu | ❌ | ✅ |
| Tham số đánh giá | Có thể nhiều lần | Đúng một lần |
| Đặt breakpoint | ❌ | ✅ |
| **Chỉ macro làm được** | `#`, `##`, `__FILE__`/`__LINE__` tại chỗ gọi, X-macro, dùng trong `#if`, hằng cho kích thước mảng/`case` | — |

**Chốt:** *"Mặc định `static inline`; chỉ dùng macro cho những việc hàm không làm được."*
</details>

#### C-023 · 🟠 · concept · ⭐ · [→ c-language-idioms §1.2](../../../01-cpp-fundamentals/c-language-idioms.md)
**Một macro cần chạy hai câu lệnh. Vì sao người ta viết `#define F() do { a(); b(); } while (0)` mà không viết `a(); b()` hay `{ a(); b(); }`? Cho ví dụ cụ thể chỗ hai cách kia vỡ.**
<details><summary>Đáp án</summary>

**Bối cảnh vỡ:**
```c
if (fault)
    RESET_AND_LOG();
else
    printf("ok\n");
```

| Định nghĩa | Kết quả thật | Vì sao |
|---|---|---|
| `hw_reset(); log_evt()` | `error: 'else' without a previous 'if'` | `if` chỉ ôm câu đầu |
| `{ hw_reset(); log_evt(); }` | `error: 'else' without a previous 'if'` | `{ … }` cộng dấu `;` người dùng gõ ⟹ `{ … };` — dấu `;` là câu lệnh rỗng chen giữa `if` và `else` |
| `do { hw_reset(); log_evt(); } while (0)` | ✅ in `ok` | **Một câu lệnh**, cần **đúng một** `;` — dùng y như gọi hàm |

**Ca nguy hiểm nhất là ca KHÔNG lỗi compile:** bản đầu, không có `else`:
```c
if (fault)
    RESET_AND_LOG();      // fault = 0
```
Output thật: `n_reset=0 n_log=1` — `log_evt()` **luôn chạy**. gcc có cảnh báo `macro expands to multiple statements [-Wmultistatement-macros]`.

**"Vì sao" tách tầng:**
- *Nông:* "để gom nhiều câu thành một khối".
- *Sâu:* mục tiêu là macro **cư xử y hệt một lời gọi hàm** trong mọi ngữ cảnh cú pháp — đặc biệt là chấp nhận đúng một `;` phía sau. `{ }` gom được nhưng không nuốt được dấu `;`; `do … while (0)` thì nuốt được, và compiler bỏ vòng lặp chạy một lần nên không tốn gì.

**Chốt:** *"`do { } while (0)` biến macro nhiều câu lệnh thành đúng một câu lệnh cần một dấu `;`."*
</details>

#### C-024 · 🟡 · concept · [→ c-language-idioms §1.3](../../../01-cpp-fundamentals/c-language-idioms.md)
**Với `#define FW_VERSION 3` và `#define STR(x) #x` — `STR(FW_VERSION)` ra chuỗi gì? Làm sao để ra `"3"`? Và `##` dùng làm gì?**
<details><summary>Đáp án</summary>

`#` biến tham số thành chuỗi **nguyên văn**, không mở rộng trước ⟹ `STR(FW_VERSION)` = `"FW_VERSION"`.

Muốn mở rộng trước thì qua một macro trung gian — tham số được mở rộng khi đi qua `XSTR`, rồi mới tới `#`:
```c
#define XSTR(x) STR(x)
XSTR(FW_VERSION)          // "3"
```

`##` dán hai token thành một: `#define CAT(a, b) a##b` ⟹ `int CAT(reg_, 7) = 42;` thành `int reg_7 = 42;`.

Output thật: `STR(FW_VERSION) = FW_VERSION` · `XSTR(FW_VERSION) = 3` · `reg_7 = 42`.

**Luật:** tham số đứng cạnh `#`/`##` **không** được mở rộng trước. **Chốt:** *"`#` thì chuỗi hoá, `##` thì dán token; muốn mở rộng trước thì thêm một tầng macro."*
</details>

#### C-025 · 🟡 · design · 🏗️ · [→ c-language-idioms §1.4](../../../01-cpp-fundamentals/c-language-idioms.md)
**Driver có `enum` 30 mã lỗi và một bảng chuỗi mô tả tương ứng để log. Đã hai lần có người thêm mã lỗi mà quên thêm chuỗi — log in sai mô tả. Thiết kế để chuyện đó không xảy ra được.**
<details><summary>Đáp án</summary>

**Gốc vấn đề:** một sự thật (danh sách mã lỗi) sống ở **hai chỗ** không có cơ chế đồng bộ.

**X-macro — một danh sách, sinh ra cả hai:**
```c
#define ERROR_LIST(X)          \
    X(ERR_OK,      "ok")        \
    X(ERR_TIMEOUT, "timeout")   \
    X(ERR_NACK,    "i2c nack")
#define AS_ENUM(name, text)  name,
#define AS_TEXT(name, text)  text,

typedef enum { ERROR_LIST(AS_ENUM) ERR_COUNT } err_t;
static const char *const err_text[] = { ERROR_LIST(AS_TEXT) };
_Static_assert(sizeof err_text / sizeof err_text[0] == ERR_COUNT, "bang ten lech enum");
```
Output thật: `0 -> ok`, `1 -> timeout`, `2 -> i2c nack`.

**Các lựa chọn khác, nêu được là điểm cộng:**

| Cách | Được | Mất |
|---|---|---|
| ✅ X-macro | Thêm mã = thêm **một dòng** | Khó đọc với người chưa quen |
| Bảng khởi tạo chỉ định `[ERR_NACK] = "i2c nack"` + `_Static_assert` số phần tử | Dễ đọc | Vẫn hai chỗ — chỉ **phát hiện** lệch, không **ngăn** |
| Sinh code từ file mô tả (script lúc build) | Sinh được cả tài liệu, binding ngôn ngữ khác | Thêm một bước build |

**Chốt:** *"Hai bảng phải khớp nhau thì đừng viết hai bảng — sinh cả hai từ một danh sách, và để `_Static_assert` canh lúc build."*
</details>

---

## I — Thao tác bit viết tay

#### C-026 · 🟢 · coding · [→ bare-metal-c §2.1](../../../08-embedded-systems/bare-metal-c.md)
**Viết hàm kiểm tra một `uint32_t` có phải lũy thừa của 2 không, không dùng vòng lặp.**
<details><summary>Đáp án</summary>

`return x && !(x & (x - 1));` — `x - 1` lật bit 1 thấp nhất và mọi bit 0 bên phải nó, nên `x & (x - 1)` xoá bit 1 thấp nhất; còn 0 nghĩa là `x` chỉ có một bit 1. Phải loại `x = 0`. Output thật: `0=0 1=1 64=1 96=0`.
</details>

#### C-027 · 🟡 · coding · ⭐ · [→ bare-metal-c §2.1](../../../08-embedded-systems/bare-metal-c.md)
**Viết hàm đảo thứ tự bit của một `uint8_t` (bit 0 ↔ bit 7, bit 1 ↔ bit 6…). Nếu hàm này nằm trên đường nóng, tối ưu thế nào?**
<details><summary>Đáp án</summary>

```c
uint8_t reverse8(uint8_t b) {
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) {
        r = (uint8_t)((r << 1) | (b & 1u));   // day bit thap nhat cua b vao ben phai r
        b >>= 1;
    }
    return r;
}
```
Output thật: `reverse8(0x01)=0x80`, `reverse8(0xB0)=0x0D`.

**Tối ưu, theo thứ tự hay được nhắc:**

| Cách | Chi phí | Ghi chú |
|---|---|---|
| Vòng lặp | 8 vòng | Dễ đúng nhất trên giấy |
| Bảng tra 256 byte | 1 lần đọc bộ nhớ | 256 byte flash — trên MCU nhỏ cân nhắc; bảng 16 phần tử (theo nibble) + 2 lần tra là thoả hiệp |
| Đổi chỗ theo nhóm: nibble ↔ nibble, cặp ↔ cặp, bit ↔ bit | 3 bước, không rẽ nhánh | Xem code bên dưới bảng |
| Lệnh phần cứng | 1 lệnh | ARM có `RBIT` (qua `__RBIT()` của CMSIS) |

```c
b = (uint8_t)((b >> 4) | (b << 4));                       // doi 2 nibble
b = (uint8_t)(((b & 0xCC) >> 2) | ((b & 0x33) << 2));     // doi tung cap bit
b = (uint8_t)(((b & 0xAA) >> 1) | ((b & 0x55) << 1));     // doi tung bit ke nhau
```
Output thật: `0x01 -> 0x80`, `0xB0 -> 0x0D` — khớp bản vòng lặp.

**Bẫy:** dừng vòng lặp khi `b == 0` (`while (b)`) thay vì chạy đủ 8 lần — các bit 0 ở phía cao cũng phải được đảo. Output thật của bản sai: `0xB0 -> 0x0D` (**đúng**, nên test với một input sẽ lọt) nhưng `0x01 -> 0x01` (**sai**, phải là `0x80`). **Chốt:** *"Viết vòng lặp cho đúng trước, rồi nói bảng tra và lệnh phần cứng."*
</details>

#### C-028 · 🟡 · coding · ⭐ · [→ bare-metal-c §2.1](../../../08-embedded-systems/bare-metal-c.md)
**Viết hàm đảo thứ tự byte của `uint32_t` (`0x11223344` → `0x44332211`). Dùng nó vào việc gì trong driver?**
<details><summary>Đáp án</summary>

```c
uint32_t bswap32(uint32_t x) {
    return  (x >> 24)                |
           ((x >>  8) & 0x0000FF00u) |
           ((x <<  8) & 0x00FF0000u) |
            (x << 24);
}
```
Output thật: `bswap32(0x11223344)=0x44332211`, khớp `__builtin_bswap32`.

**Dùng vào:** đổi **endianness** — giao thức mạng là big-endian (`htonl`/`ntohl`), nhiều thiết bị I2C/SPI trả thanh ghi nhiều byte theo thứ tự ngược CPU. Compiler nhận ra mẫu này và sinh **một lệnh** (`bswap` trên x86, `REV` trên ARM).

**Nhưng cách tốt hơn khi đọc từ buffer:** đừng đọc thành `uint32_t` rồi đảo — ghép byte bằng phép dịch theo đúng thứ tự trên dây (C-034); code đó đúng trên **mọi** CPU, không cần biết CPU là little hay big-endian.

**Chốt:** *"Đảo byte = dịch rồi mask từng byte; còn đọc dữ liệu từ dây thì ghép byte theo thứ tự của giao thức."*
</details>

#### C-029 · 🟠 · coding · ⭐ · [→ bare-metal-c §2.1](../../../08-embedded-systems/bare-metal-c.md)
**Thanh ghi 32-bit có trường `MODE` rộng 4 bit ở vị trí bit 8. Viết macro/hàm để đọc trường đó và để ghi một giá trị mới vào nó mà không đụng các bit khác. Nêu những gì có thể sai.**
<details><summary>Đáp án</summary>

```c
#define FIELD_MASK(pos, width)        (((1u << (width)) - 1u) << (pos))
#define FIELD_GET(reg, pos, width)    (((reg) & FIELD_MASK(pos, width)) >> (pos))
#define FIELD_SET(reg, pos, width, v) \
    ((reg) = ((reg) & ~FIELD_MASK(pos, width)) | (((uint32_t)(v) << (pos)) & FIELD_MASK(pos, width)))
```

Output thật, `reg = 0xFFFF00FF`:
```
FIELD_GET(reg, 4, 4)           = 0xF
FIELD_SET(reg, 8, 4, 0xA)  ->  0xFFFF0AFF
FIELD_SET(reg, 8, 4, 0x1F) ->  0xFFFF0FFF     <- 0x1F qua rong: bi cat, khong lan sang truong khac
```

**Những gì có thể sai — đây là phần tính điểm:**

| Sai | Hậu quả |
|---|---|
| Chỉ OR, không xoá trường cũ | Bit 1 cũ còn nguyên ⟹ giá trị là OR của cũ và mới |
| Không AND giá trị với mask | Giá trị quá rộng **ghi đè trường bên cạnh** |
| `width = 32` | `1u << 32` là **UB** ([COD-021](coding.md)) |
| ISR cũng ghi thanh ghi này | Read-modify-write **không atomic** ⟹ mất ghi; cần critical section, hoặc dùng thanh ghi SET/CLR nếu SoC có |
| Truyền `*p++` làm `reg` | Macro đánh giá `reg` **hai lần** (C-022) |
| Thanh ghi có bit **ghi-1-để-xoá** (W1C) trong cùng word | RMW đọc ra 1 rồi ghi lại 1 ⟹ **vô tình xoá** cờ trạng thái khác |

**Chốt:** *"Xoá trường, mask giá trị, OR vào — và hỏi lại: ISR có đụng thanh ghi này không, có bit W1C không."* Kernel có sẵn `GENMASK` và `FIELD_GET`/`FIELD_PREP` cho đúng việc này.
</details>

---

## J — Tự viết hàm chuẩn

#### C-030 · 🟠 · coding · ⭐ · [→ c-language-idioms §5.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**Viết `atoi`. Trước khi viết, liệt kê các ca biên bạn sẽ hỏi lại interviewer.**
<details><summary>Đáp án</summary>

**Ca biên cần hỏi trước — phần này được chấm ngang với code:** khoảng trắng đầu · dấu `+`/`-` · ký tự không phải số giữa chừng (dừng hay báo lỗi) · chuỗi rỗng / không có chữ số · **tràn số** · `INT_MIN` · `NULL`.

```c
bool my_atoi(const char *s, int *out) {
    while (isspace((unsigned char)*s)) s++;
    int sign = 1;
    if (*s == '+' || *s == '-') { if (*s == '-') sign = -1; s++; }
    if (!isdigit((unsigned char)*s)) return false;              // khong co chu so nao
    long long acc = 0;
    while (isdigit((unsigned char)*s)) {
        acc = acc * 10 + (*s - '0');
        if (sign * acc > INT_MAX || sign * acc < INT_MIN) return false;   // tran — dung NGAY
        s++;
    }
    *out = (int)(sign * acc);
    return true;
}
```

| Input | Output thật |
|---|---|
| `"  42"` · `"-17abc"` · `"+0"` | ok 42 · ok −17 · ok 0 |
| `"2147483647"` · `"-2147483648"` | ok (biên trên, `INT_MIN`) |
| `"2147483648"` | **LỖI** — tràn |
| `"abc"` · `""` | **LỖI** |

**"Vì sao" tách tầng:**
- *Nông:* `acc = acc * 10 + c - '0'`.
- *Sâu:* hàm phải **báo được lỗi**. `atoi` chuẩn không làm được: `atoi("2147483648")` in ra `-2147483648` — tràn là UB, không có kênh báo lỗi. Đó là lý do code thật dùng `strtol` (`errno = ERANGE`, con trỏ tới ký tự dừng). Chữ ký trả `bool` + tham số ra là cách viết lại có báo lỗi.

**Hai chi tiết nhỏ nhưng là dấu hiệu kinh nghiệm:** kiểm tràn **trong vòng lặp** (không phải sau) · ép `(unsigned char)` trước khi gọi `isdigit`/`isspace` (byte ≥ 0x80 trên nền `char` có dấu là UB).

**Bẫy:** đổi sang dương rồi nhân dấu ở cuối bằng `int` ⟹ `"-2147483648"` tràn ở bước trung gian. **Chốt:** *"atoi khó ở ca biên chứ không ở vòng lặp — hỏi về tràn số trước khi viết."*
</details>

#### C-031 · 🟡 · coding · [→ c-language-idioms §5.2](../../../01-cpp-fundamentals/c-language-idioms.md)
**Viết `itoa(int v, char *buf, size_t n)` cơ số 10. Giá trị nào làm cách viết "đổi sang dương trước" hỏng?**
<details><summary>Đáp án</summary>

**`INT_MIN`**: `-INT_MIN` không biểu diễn được trong `int` (tràn là UB). Miền âm rộng hơn miền dương một giá trị ⟹ **làm việc trên số âm**:
```c
char *my_itoa(int v, char *buf, size_t n) {
    char tmp[12];                       // "-2147483648" + '\0'
    int  i = 0;
    bool neg = v < 0;
    if (!neg) v = -v;                   // dua ve AM — luon an toan
    do { tmp[i++] = (char)('0' - v % 10); v /= 10; } while (v);   // do-while: v = 0 van ra "0"
    if (neg) tmp[i++] = '-';
    if ((size_t)i + 1 > n) return NULL; // khong du cho
    for (int k = 0; k < i; k++) buf[k] = tmp[i - 1 - k];          // chu so sinh nguoc — dao lai
    buf[i] = '\0';
    return buf;
}
```
Output thật: `0 -305 -2147483648`.

**Ba ca biên phải tự nêu:** `0` (dùng `do-while`, không phải `while`) · `INT_MIN` · buffer quá nhỏ. Chi tiết: `v % 10` với `v` âm cho số âm từ C99, nên `'0' - v % 10` ra đúng ký tự.

**Chốt:** *"Bẫy của itoa là INT_MIN — xử lý trên số âm thì không có ngoại lệ nào."*
</details>

#### C-032 · 🟡 · concept · [→ c-language-idioms §5.3](../../../01-cpp-fundamentals/c-language-idioms.md)
**Đồng nghiệp thay mọi `strcpy` bằng `strncpy(dst, src, sizeof dst)` "cho an toàn". Có đúng không?**
<details><summary>Đáp án</summary>

**Không.** `strncpy` chép tối đa `n` byte và **chỉ thêm `'\0'` khi nguồn ngắn hơn `n`**. Nguồn dài hơn ⟹ `dst` không có `'\0'`, `printf("%s")`/`strlen` sau đó đọc tràn.

Output thật với `char dst[4]` và nguồn `"abcdef"`:
```
warning: 'strncpy' output truncated copying 4 bytes from a string of length 6 [-Wstringop-truncation]
strncpy: dst[3]='d' (khong phai '\0')
```
Thêm một bất lợi: nguồn ngắn thì nó **đệm `'\0'` tới hết `n`** — tốn công với buffer lớn. `strncpy` vốn sinh ra cho trường **cố định độ dài** (tên file trong cấu trúc thư mục cũ), không phải cho chuỗi.

**Thay bằng:**
```c
snprintf(dst, sizeof dst, "%s", src);   // luon co '\0', cat bot neu thieu cho
```
Output thật: `"abc"`. `snprintf` còn **trả về độ dài lẽ ra cần** ⟹ so với `sizeof dst` để biết đã bị cắt. (`strlcpy` làm cùng việc, có trên BSD và glibc ≥ 2.38, nhưng không phải chuẩn C.)

**Chốt:** *"`strncpy` không bảo đảm `'\0'` — chép chuỗi có giới hạn thì dùng `snprintf` và kiểm giá trị trả về."*
</details>

---

## K — `union` & tuần tự hoá byte

#### C-033 · 🟡 · concept · [→ c-language-idioms §3.2](../../../01-cpp-fundamentals/c-language-idioms.md)
**`union { uint32_t u; uint8_t b[4]; } x = { .u = 0x11223344 };` — `x.b[0]` bằng bao nhiêu? Đọc `b` sau khi ghi `u` có hợp lệ không, trong C và trong C++?**
<details><summary>Đáp án</summary>

**Phụ thuộc endianness của CPU.** Output thật trên x86 (little-endian): `b[0..3] = 44 33 22 11` — byte thấp nằm ở địa chỉ thấp. Trên CPU big-endian sẽ là `11 22 33 44`. Đây cũng là một cách **kiểm endianness** ([COD-010](coding.md)).

| Đọc thành viên khác thành viên vừa ghi | |
|---|---|
| **C** | ✅ Hợp lệ — byte được diễn giải lại theo kiểu mới |
| **C++** | ❌ **UB** — dùng `memcpy` hoặc `std::bit_cast` (C++20) |

`memcpy` hợp lệ ở cả hai: output thật `bit cua 1.0f = 0x3F800000`.

**Công dụng chính đáng của `union`:** *tagged union* — một trường `type` cho biết đang dùng thành viên nào (gói tin nhiều loại, giá trị cấu hình nhiều kiểu), tiết kiệm bộ nhớ. **Không** nên dùng `union`/bitfield để map thanh ghi ([EMB-003](embedded-fundamentals.md)).

**Chốt:** *"`union` cho thấy thứ tự byte của CPU — chính vì thế không dùng nó để đọc dữ liệu từ dây."*
</details>

#### C-034 · 🟠 · coding · ⭐ · [→ c-language-idioms §3.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**Gói tin nhận qua UART có trường độ dài 4 byte, big-endian, bắt đầu ở `buf + 3`. Viết code đọc nó thành `uint32_t`. Vì sao không viết `*(uint32_t *)(buf + 3)`?**
<details><summary>Đáp án</summary>

```c
static uint32_t get_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}
uint32_t len = get_be32(buf + 3);
```

**So ba cách — output thật trên x86 với dây `11 22 33 44`:**

| Cách | Kết quả | Vấn đề |
|---|---|---|
| ✅ Phép dịch | `0x11223344` | Không có — đúng trên mọi CPU |
| `memcpy(&v, p, 4)` | `0x44332211` | Lấy theo endianness **CPU**; phải tự đảo, và code thành phụ thuộc nền tảng |
| `*(uint32_t *)(buf + 3)` | — | ① endianness như trên · ② `buf + 3` **không căn 4** ⟹ fault trên CPU không cho đọc lệch · ③ vi phạm **strict aliasing** ⟹ UB ([COD-025](coding.md)) |

**Chi tiết bắt buộc:** `(uint32_t)p[0] << 24` — **ép trước khi dịch**. `p[0]` là `uint8_t` bị nâng lên `int`; `0x80 << 24` trên `int` là tràn dấu ⟹ UB ([EMB-037](embedded-fundamentals.md)).

**"Vì sao" tách tầng:**
- *Nông:* "ép kiểu có thể sai endianness".
- *Sâu:* phép dịch mô tả **ý nghĩa** (byte này là phần cao), không mô tả **bố cục bộ nhớ** — nên nó không phụ thuộc CPU, alignment hay luật aliasing. Compiler hiện đại nhận ra mẫu này và sinh lệnh nạp + đảo byte, nên không chậm hơn.

**Chốt:** *"Dữ liệu từ dây thì ghép bằng phép dịch theo thứ tự của giao thức — không ép kiểu con trỏ, không dựa vào union."*
</details>

---

## L — Linkage & vòng đời biến

#### C-035 · 🟡 · concept · ⭐ · [→ c-language-idioms §2.1, §2.3](../../../01-cpp-fundamentals/c-language-idioms.md)
**Chữ `static` có mấy nghĩa trong C? Một biến `static` khai báo trong hàm được khởi tạo khi nào? Và nếu đặt `static int helper(void) {…}` trong một header được nhiều file include thì sao?**
<details><summary>Đáp án</summary>

**Hai nghĩa, tuỳ vị trí:**

| Vị trí | Đổi cái gì | Hệ quả |
|---|---|---|
| **Trong hàm** | **Vòng đời** | Sống suốt chương trình, nằm ở `.data`/`.bss`, khởi tạo **một lần** trước khi chương trình chạy |
| **Ngoài hàm** (biến/hàm) | **Ai thấy được** (linkage) | Chỉ file đó thấy — đóng gói, tránh đụng tên |

```c
int next_id(void) { static int id = 100; return id++; }
```
Ba lời gọi ở ba câu lệnh riêng: `100 101 102`. *(Gọi gọn trong một `printf` thì gcc in `102 101 100` — thứ tự đánh giá đối số là unspecified, [COD-026](coding.md).)*

**`static` trong header:** mỗi `.c` include nó có **một bản riêng** — đã kiểm: địa chỉ `helper` in từ hai file là **2 địa chỉ khác nhau**.
- Với hàm nhỏ `static inline`: **chủ ý**, đây là cách viết hàm tiện ích trong header C.
- Với **biến** `static` trong header: gần như luôn là **bug** — mỗi file sửa bản riêng và tưởng là dùng chung.

**Chốt:** *"Trong hàm, `static` đổi vòng đời; ngoài hàm, `static` đổi phạm vi nhìn thấy."*
</details>

#### C-036 · 🟠 · concept · [→ c-language-idioms §2.2, §2.4](../../../01-cpp-fundamentals/c-language-idioms.md)
**Dự án cũ có `int g_count;` ở đầu hai file `.c` khác nhau, build bao năm không sao. Nâng toolchain lên gcc 11 thì báo `multiple definition of 'g_count'`. Vì sao? Sửa đúng thế nào — và `const int LIMIT = 10;` ở hai file thì sao?**
<details><summary>Đáp án</summary>

**Cơ chế:** `int g_count;` ở phạm vi file, không `extern`, không khởi tạo, là một *tentative definition*.
1. Trước gcc 10, mặc định `-fcommon`: các tentative definition cùng tên được gộp thành **một** "common symbol" lúc link ⟹ không lỗi.
2. Từ **gcc 10**, mặc định `-fno-common`: mỗi cái là một **định nghĩa thật** ⟹ hai định nghĩa ⟹ lỗi link.

Output thật (gcc 11.4): `multiple definition of 'g_count'`; thêm `-fcommon` thì link được.

**Sửa đúng — không phải thêm `-fcommon`:**
```c
/* counter.h */  extern int g_count;     // KHAI BAO, khong cap bo nho
/* a.c */        int g_count = 0;        // DINH NGHIA — dung MOT file
```

**`const int LIMIT = 10;` ở hai file:**

| | C | C++ |
|---|---|---|
| Kết quả thật | ❌ `multiple definition of 'LIMIT'` | ✅ link OK |
| Lý do | `const` toàn cục có **external** linkage | `const` toàn cục mặc định **internal** linkage |
| Cách đúng | `extern const int LIMIT;` trong header + một định nghĩa; hoặc `enum { LIMIT = 10 };` | Đặt thẳng trong header |

**"Vì sao" tách tầng:**
- *Nông:* "khai báo trùng".
- *Sâu:* phân biệt **khai báo** (nói "có một thứ tên này") với **định nghĩa** (cấp bộ nhớ). Header chỉ được chứa khai báo; định nghĩa đúng một nơi. Code cũ dựa vào một **mặc định của toolchain** — đổi mặc định là vỡ, đó là bài học về việc pin phiên bản toolchain trong build.

**Chốt:** *"`extern` trong header, định nghĩa trong đúng một `.c` — và `const` ở phạm vi file đổi linkage giữa C với C++."*
</details>

---

## M — Xử lý lỗi kiểu C

#### C-037 · 🟡 · coding · ⭐ · [→ c-language-idioms §4](../../../01-cpp-fundamentals/c-language-idioms.md)
**Viết hàm khởi tạo cần giành ba tài nguyên A, B, C theo thứ tự. Lỗi ở bước nào thì phải trả lại đúng những gì đã giành và trả mã lỗi. Viết theo kiểu Linux kernel.**
<details><summary>Đáp án</summary>

```c
int dev_init(void **out_a, void **out_b, void **out_c) {
    int ret = -ENOMEM;
    void *a = res_alloc(1, 16);
    if (!a) goto err_a;
    void *b = res_alloc(2, 16);
    if (!b) goto err_b;
    void *c = res_alloc(3, 16);
    if (!c) goto err_c;

    *out_a = a; *out_b = b; *out_c = c;
    return 0;                 // thanh cong: KHONG roi xuong cac nhan

err_c:
    free(b);                  // don theo thu tu NGUOC
err_b:
    free(a);
err_a:
    return ret;
}
```

Output thật, lỗi giả lập ở từng bước: `0`, `-12`, `-12`, `-12`; valgrind ở cả bốn kịch bản: `All heap blocks were freed`.

| Cách | Vấn đề |
|---|---|
| `if` lồng nhau | Thụt lề sâu dần theo số tài nguyên |
| Lặp `free(...)` ở mỗi nhánh lỗi | Thêm tài nguyên phải sửa N chỗ |
| ✅ `goto` tới nhãn dọn dẹp | **Một** đường ra; thêm tài nguyên = thêm một cặp `if/goto` + một nhãn |

**"Vì sao" tách tầng:**
- *Nông:* "goto cho gọn".
- *Sâu:* đây là **RAII viết tay**: nhãn đặt theo thứ tự ngược với thứ tự giành, nên rơi vào nhãn nào là dọn đúng những gì đã có. Trong C++ destructor làm việc này tự động — và C++ còn **cấm** `goto` nhảy qua khởi tạo (g++ báo `jump to label … crosses initialization`), nên kiểu này không dán sang C++ được.

**Bẫy:** quên `return 0` trước các nhãn ⟹ thành công cũng giải phóng hết. **Chốt:** *"Mã lỗi âm, một đường ra, nhãn dọn dẹp theo thứ tự ngược — đúng phong cách kernel."*
</details>

---

## N — 🔎 Đọc code: output là gì, sai ở đâu

> Dạng câu hỏi hay gặp nhất khi phỏng vấn **trên giấy**: đưa đoạn code ngắn, hỏi *"in ra gì?"* hoặc *"có vấn đề gì?"*. Đo việc bạn đọc theo **luật của ngôn ngữ** hay theo **ý định của người viết**. Câu trả lời mạnh có ba phần: **output** · **cơ chế** · **cách sửa / cờ compiler bắt được**. Tài liệu: [c-language-idioms §6](../../../01-cpp-fundamentals/c-language-idioms.md). Bug-hunt kiểu C++/hệ thống khác: [coding.md mục B](coding.md).

#### C-038 · 🟢 · coding · [→ c-pointers-arrays §6](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**In ra gì?**
```c
char s[] = "hi\0there";
printf("%zu %zu\n", sizeof s, strlen(s));
```
<details><summary>Đáp án</summary>

**`9 2`**. `sizeof` đếm cả mảng: 8 ký tự viết ra cộng `'\0'` tự thêm ở cuối = 9. `strlen` dừng ở `'\0'` **đầu tiên** — ngay sau `hi`. Dữ liệu nhị phân có byte 0 thì phải mang độ dài riêng, không dùng hàm chuỗi.
</details>

#### C-039 · 🟡 · coding · [→ c-pointers-arrays §2.1](../../../01-cpp-fundamentals/c-pointers-arrays.md)
**In ra gì?**
```c
int a[] = {1, 2, 3, 4, 5};
int *p = (int *)(&a + 1);
printf("%d %d\n", *(a + 1), *(p - 1));
```
<details><summary>Đáp án</summary>

**Output thật: `2 5`.**

- `*(a + 1)`: `a` decay thành `int *`, `+1` nhảy **4 byte** ⟹ `a[1]` = **2**.
- `&a` có kiểu `int (*)[5]` — con trỏ tới **cả mảng** ⟹ `&a + 1` nhảy **20 byte**, trỏ ngay **sau** phần tử cuối.
- Ép về `int *` rồi `p - 1` lùi **4 byte** ⟹ phần tử cuối = **5**.

```
a:  [ 1 ][ 2 ][ 3 ][ 4 ][ 5 ]
      ^    ^              ^    ^
      a   a+1           p-1   &a+1 (= p)
```

**Chốt:** *"Cùng địa chỉ, khác kiểu ⟹ khác bước nhảy: `a + 1` qua một `int`, `&a + 1` qua cả mảng."* (Con trỏ "ngay sau cuối mảng" là hợp lệ để tính toán và so sánh, chỉ không được dereference.)
</details>

#### C-040 · 🟡 · coding · [→ c-language-idioms §6.2](../../../01-cpp-fundamentals/c-language-idioms.md)
**In ra gì?**
```c
if (sizeof(int) > -1) printf("lon hon\n");
else                  printf("KHONG lon hon\n");
```
<details><summary>Đáp án</summary>

**Output thật: `KHONG lon hon`.**

**Cơ chế:** `sizeof` có kiểu `size_t` — **không dấu**. So sánh không dấu với `int` ⟹ `-1` bị đổi sang không dấu = `SIZE_MAX` (số lớn nhất). `4 > SIZE_MAX` là sai.

gcc bắt: `comparison of integer expressions of different signedness: 'long unsigned int' and 'int' [-Wsign-compare]`.

**Gặp ở đâu thật:** `if (len - 1 >= 0)` với `len` kiểu `size_t` luôn đúng · `if (read_bytes < sizeof hdr)` với `read_bytes = -1` (lỗi từ `read`) thành số cực lớn, lọt qua kiểm tra. Cùng gốc với [EMB-037](embedded-fundamentals.md) (*usual arithmetic conversions*).

**Chốt:** *"Có dấu gặp không dấu thì có dấu bị đổi sang không dấu — `-1` thành số lớn nhất."*
</details>

#### C-041 · 🟠 · coding · ⭐ · [→ c-language-idioms §6.2](../../../01-cpp-fundamentals/c-language-idioms.md)
**Driver đọc một byte trạng thái từ UART rồi kiểm tra. Trên PC test chạy một kiểu, trên board ARM chạy kiểu khác. In ra gì trên mỗi nơi, và vì sao?**
```c
char c = 200;               /* byte doc tu UART */
if (c > 100) printf("c > 100, c = %d\n", c);
else         printf("c <= 100, c = %d\n", c);
```
<details><summary>Đáp án</summary>

**Output thật:**
```
x86 (gcc mac dinh):            c <= 100, c = -56
-funsigned-char (mo phong ARM): c > 100, c = 200
```
*(Bản thứ hai mô phỏng bằng cờ `-funsigned-char` trên x86 — máy không có toolchain ARM. ABI của ARM quy định `char` không dấu.)*

**Cơ chế:**
1. Chuẩn C **không quy định** `char` có dấu hay không — đó là quyết định của **nền tảng**: x86 có dấu, ARM/PowerPC không dấu.
2. `char` có dấu chỉ chứa −128…127 ⟹ 200 không vừa ⟹ đổi thành **−56** (giá trị do implementation định nghĩa; gcc lấy 200 − 256).
3. `-56 > 100` sai ⟹ nhánh `else`.

⚠️ gcc **không cảnh báo** dòng `char c = 200;` kể cả với `-Wall -Wextra`.

**"Vì sao" tách tầng:**
- *Nông:* "tràn kiểu `char`".
- *Sâu:* đây là bug **phụ thuộc nền tảng**: unit test trên PC qua, chạy trên board thì khác — hoặc ngược lại. Nó còn lộ ra ở chỗ khác: `char` âm truyền vào `isdigit()` là UB (C-030), `char` âm làm chỉ số bảng tra ⟹ đọc ngoài mảng.

**Sửa:** dữ liệu là **byte** thì dùng `uint8_t`; chỉ dùng `char` cho **ký tự văn bản**. `signed char`/`unsigned char` khi thật sự cần số nhỏ có/không dấu.

**Chốt:** *"`char` không có dấu cố định — byte từ phần cứng thì luôn là `uint8_t`."*
</details>

#### C-042 · 🟢 · coding · [→ c-language-idioms §6.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**Vòng lặp này in ra gì?**
```c
for (unsigned int i = 3; i >= 0; i--)
    printf("%u ", i);
```
<details><summary>Đáp án</summary>

**Chạy vô tận**: `3 2 1 0 4294967295 4294967294 …`. `unsigned` không bao giờ `< 0`; `0 - 1` quấn vòng thành `UINT_MAX`. gcc: `comparison of unsigned expression in '>= 0' is always true [-Wtype-limits]`. Sửa: `for (unsigned i = 4; i-- > 0; )` hoặc dùng `int`.
</details>

#### C-043 · 🟡 · coding · ⭐ · [→ c-language-idioms §6.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**`status = 0x01` — bit READY (`0x04`) đang tắt. Đoạn này in ra gì?**
```c
#define READY 0x04u
if (status & READY == 0) printf("chua san sang\n");
else                     printf("san sang -> gui lenh\n");
```
<details><summary>Đáp án</summary>

**Output thật: `san sang -> gui lenh`** — sai, thiết bị chưa sẵn sàng mà vẫn gửi lệnh.

**Cơ chế:** `==` có độ ưu tiên **cao hơn** `&`. Biểu thức thật là `status & (READY == 0)` = `status & 0` = **0** ⟹ điều kiện luôn sai, bất kể `status`.

gcc bắt: `suggest parentheses around comparison in operand of '&' [-Wparentheses]`.

**Sửa:** `if ((status & READY) == 0)` hoặc gọn hơn `if (!(status & READY))`.

**Cùng họ, hay hỏi kèm:** `1 << 2 + 1` ra **8** (không phải 5) — `+` mạnh hơn `<<`. Output thật, cũng bị `-Wparentheses` bắt.

**Thứ tự nên nhớ:** số học (`* / + -`) > dịch (`<< >>`) > so sánh (`< > == !=`) > bit (`& ^ |`) > logic (`&& ||`). Toán tử bit nằm **dưới** so sánh là di sản từ C cũ — lý do gặp bit thì **luôn đặt ngoặc**.

**Chốt:** *"`&` yếu hơn `==` — kiểm bit thì luôn bọc ngoặc."*
</details>

#### C-044 · 🟡 · coding · [→ c-language-idioms §6.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**`link_up = 0`. Đoạn này in ra gì?**
```c
if (link_up)
    if (has_ip)
        printf("online\n");
else
    printf("link down\n");
printf("xong\n");
```
<details><summary>Đáp án</summary>

**Output thật: chỉ `xong`** — không có `link down`.

**Cơ chế:** `else` gắn với `if` **gần nhất chưa có `else`**, tức `if (has_ip)` — thụt lề không có ý nghĩa gì với compiler. Code thật là:
```c
if (link_up) {
    if (has_ip) printf("online\n");
    else        printf("link down\n");
}
```
`link_up = 0` ⟹ bỏ qua cả khối.

gcc bắt: `suggest explicit braces to avoid ambiguous 'else' [-Wdangling-else]`.

**Cùng họ:** `if (x = 0)` — **gán** chứ không so sánh, luôn sai và đổi `x` thành 0 (output thật: `nhanh else, x = 0`); gcc: `suggest parentheses around assignment used as truth value`.

**Chốt:** *"`else` theo `if` gần nhất, không theo thụt lề — luôn dùng ngoặc nhọn."* (MISRA bắt buộc ngoặc cho mọi thân `if`.)
</details>

#### C-045 · 🟢 · coding · [→ c-language-idioms §6.1](../../../01-cpp-fundamentals/c-language-idioms.md)
**`mode = 1`. `power` bằng bao nhiêu?**
```c
switch (mode) {
case 0: power = 10;
case 1: power = 50;
case 2: power = 100;
default: break;
}
```
<details><summary>Đáp án</summary>

**`power = 100`**. Thiếu `break` thì sau khi khớp `case 1` chương trình **rơi tiếp** xuống `case 2`. gcc: `this statement may fall through [-Wimplicit-fallthrough=]`. Cố ý rơi thì ghi `/* fall through */` hoặc `__attribute__((fallthrough));`.
</details>

#### C-046 · 🟡 · coding · ⭐ · [→ c-language-idioms §6.3](../../../01-cpp-fundamentals/c-language-idioms.md)
**In ra gì?**
```c
int x = 5;
int y = x++ + ++x;
printf("x=%d y=%d\n", x, y);
```
<details><summary>Đáp án</summary>

**Câu trả lời đúng: đây là Undefined Behavior — không có output "đúng".**

gcc 11.4 in `x=7 y=12` ở cả `-O0` lẫn `-O2` — trông **ổn định**, nhưng compiler khác, phiên bản khác, hay chỉ cần code xung quanh đổi, đều được phép in thứ khác. gcc cảnh báo đúng chỗ: `operation on 'x' may be undefined [-Wsequence-point]`.

**Cơ chế:** `x` bị **sửa hai lần** (`x++` và `++x`) trong một biểu thức mà không có **điểm tuần tự** xen giữa. Chuẩn nói: hai lần sửa không có thứ tự giữa chúng ⟹ UB. Không phải "unspecified chọn một trong hai" — mà **cả chương trình** mất ý nghĩa.

| Biểu thức | Loại |
|---|---|
| `x++ + ++x`, `i = i++`, `a[i] = i++` | **UB** — sửa và đọc/sửa không có thứ tự |
| `f(g(), h())` — `g` hay `h` chạy trước | **Unspecified** — một trong hai, chương trình vẫn hợp lệ ([COD-026](coding.md)) |
| `a && b`, `a , b`, `a ? b : c` | **Có thứ tự** — vế trái xong rồi mới tới vế phải |

**Bẫy:** trả lời một con số ("12") — đó chính là thứ interviewer muốn bắt. **Chốt:** *"Sửa cùng một biến hai lần trong một biểu thức là UB; câu trả lời là tách thành nhiều câu lệnh, không phải đoán output."*
</details>

#### C-047 · 🟡 · coding · [→ c-language-idioms §6.2](../../../01-cpp-fundamentals/c-language-idioms.md)
**Hai đoạn này in ra gì? Sửa phép so sánh thế nào?**
```c
float f = 0.1f;
if (f == 0.1) printf("bang\n"); else printf("KHONG bang\n");

float sum = 0.0f;
for (int i = 0; i < 10; i++) sum += 0.1f;
printf("%d\n", sum == 1.0f);
```
<details><summary>Đáp án</summary>

**Output thật:** `KHONG bang: f=0.1000000015` và `0` (`sum = 1.00000012`).

**Cơ chế:**
1. `0.1` không biểu diễn chính xác trong nhị phân. `0.1f` làm tròn ở độ chính xác `float`, còn `0.1` (không hậu tố) là `double` làm tròn ở độ chính xác cao hơn ⟹ khi so sánh, `f` được nâng lên `double` và **khác** `0.1`.
2. Mỗi phép cộng làm tròn thêm một lần ⟹ sai số tích luỹ.

**Sửa:** so bằng **ngưỡng** — `fabsf(sum - 1.0f) < 1e-6f` (output thật: `1`). Ngưỡng tuyệt đối hợp với giá trị quanh 1; giá trị rất lớn/nhỏ thì dùng ngưỡng **tương đối** theo độ lớn.

gcc bắt bằng `-Wfloat-equal` — **không** nằm trong `-Wall -Wextra`, phải bật riêng.

**Góc embedded:** MCU không có FPU thì `float` còn chậm ([EMB-024](embedded-fundamentals.md)) — đếm, so sánh và tích luỹ bằng **fixed-point** tránh luôn cả hai vấn đề.

**Chốt:** *"Không so `float` bằng `==`; so bằng ngưỡng, và đừng trộn hằng `double` với biến `float`."*
</details>

#### C-048 · 🟡 · coding · ⭐ · [→ c-language-idioms §6.4](../../../01-cpp-fundamentals/c-language-idioms.md)
**Đoạn mở rộng buffer này có vấn đề gì?**
```c
buf = realloc(buf, new_size);
if (!buf) return -ENOMEM;
```
<details><summary>Đáp án</summary>

**Rò bộ nhớ khi `realloc` thất bại.** `realloc` lỗi thì trả `NULL` **nhưng vùng cũ vẫn còn nguyên**. Gán thẳng vào `buf` ⟹ con trỏ duy nhất tới vùng cũ bị ghi đè bằng `NULL` ⟹ không ai `free` được nữa.

Output thật (cố ý xin `SIZE_MAX / 2` byte để `realloc` thất bại):
```
buf truoc = hop le
buf sau   = (nil)
definitely lost: 64 bytes in 1 blocks          <- valgrind
```

**Sửa:**
```c
char *tmp = realloc(buf, new_size);
if (!tmp) return -ENOMEM;     // ✅ buf van hop le — caller con free duoc / dung tiep
buf = tmp;
```

**Hai chi tiết khác của `realloc` hay bị hỏi:** vùng có thể bị **dời chỗ** ⟹ mọi con trỏ khác trỏ vào vùng cũ đều thành con trỏ treo · `realloc(p, 0)` có hành vi khác nhau giữa các thư viện — đừng dùng để giải phóng.

**Chốt:** *"`realloc` lỗi không giải phóng vùng cũ — luôn nhận kết quả vào biến tạm."*
</details>

#### C-049 · 🟠 · coding · [→ c-language-idioms §6.4](../../../01-cpp-fundamentals/c-language-idioms.md)
**Hàm kiểm tra cấu hình có đổi không trước khi ghi xuống flash. Hai cấu hình có mọi field bằng nhau mà vẫn bị coi là "khác" — vì sao?**
```c
struct cfg { uint8_t mode; uint32_t rate; };
bool cfg_changed(const struct cfg *a, const struct cfg *b) {
    return memcmp(a, b, sizeof *a) != 0;
}
```
<details><summary>Đáp án</summary>

**Cơ chế:** `uint32_t rate` phải căn 4 byte ⟹ compiler chèn **3 byte padding** sau `mode`; `sizeof(struct cfg)` = **8**, không phải 5. `memcmp` so **mọi byte**, kể cả padding — mà nội dung padding **không xác định**: biến trên stack chứa rác cũ, gán từng field không chạm tới nó.

Output thật (hai struct được `memset` khác nhau trước, rồi gán field giống hệt):
```
sizeof(struct cfg) = 8
memcmp = KHAC
so tung field = bang
```

**Hậu quả thật:** ghi flash thừa mỗi lần kiểm tra ⟹ **mòn flash** nhanh; hoặc ngược lại, checksum/CRC tính trên cả struct ra khác nhau với cùng dữ liệu.

**Sửa — chọn theo việc:**

| Cách | Khi nào |
|---|---|
| ✅ So **từng field** | Mặc định — đúng ý nghĩa, không phụ thuộc layout |
| `memset(&c, 0, sizeof c)` **trước** khi điền mọi struct, rồi `memcmp` | Khi struct lớn và luôn được tạo qua một hàm khởi tạo — kỷ luật dễ vỡ |
| `__attribute__((packed))` | Bỏ padding nhưng truy cập lệch căn — chậm hoặc fault trên một số CPU ([EMB-038](embedded-fundamentals.md)) |

**Chốt:** *"Struct có padding thì `memcmp` so cả rác — so từng field."* Cùng lý do, đừng ghi thẳng struct ra dây/ra file: padding và endianness đi theo ([C-034](c-programming.md)).
</details>

#### C-050 · 🟡 · coding · ⭐ · [→ c-language-idioms §6.4](../../../01-cpp-fundamentals/c-language-idioms.md)
**Đoạn này chạy, in đúng `name = hello`, exit 0. Có lỗi không? Chứng minh thế nào?**
```c
char name[5];
strcpy(name, "hello");
printf("name = %s\n", name);
```
<details><summary>Đáp án</summary>

**Có — tràn buffer.** `"hello"` cần **6** byte (5 ký tự + `'\0'`), `name` chỉ có 5 ⟹ byte `'\0'` ghi đè lên bộ nhớ ngay sau `name` trên stack. Đây là **UB**; chạy đúng chỉ vì byte bị đè tình cờ không ai dùng.

**Ba mức bằng chứng — output thật:**
1. **Đọc warning:** gcc 11.4 đã bắt lúc biên dịch: `'__builtin_memcpy' writing 6 bytes into a region of size 5 overflows the destination [-Wstringop-overflow=]`.
2. **Chạy thường:** `name = hello`, exit 0 — **không lộ gì**. Đây là lý do test thường không bắt được.
3. **Chạy với ASan** (`-fsanitize=address -g`) — bắt **tại chỗ**:
```
ERROR: AddressSanitizer: stack-buffer-overflow
WRITE of size 6 ...
    #1 ... in main q13.c:5
[32, 37) 'name' (line 4) <== Memory access at offset 37 overflows this variable
```
ASan chỉ đúng **dòng ghi** (`strcpy`), **biến** bị tràn (`name`) và **đúng 1 byte** tràn (offset 37 ngay ngoài vùng `[32, 37)`). *(Số dòng 4, 5 là của chương trình thử có `main` bọc ngoài đoạn code trên.)*

**Sửa:** buffer đủ chỗ cho `'\0'` (`char name[6]`), hoặc chép có giới hạn: `snprintf(name, sizeof name, "%s", src)` ([C-032](c-programming.md)).

**"Vì sao" tách tầng:**
- *Nông:* "thiếu chỗ cho `'\0'`".
- *Sâu:* **"chạy đúng" không chứng minh gì** với lỗi bộ nhớ — byte bị đè có thể là biến khác, canary, hay địa chỉ trả về; hỏng ở chỗ khác, lúc khác. Công cụ (warning + ASan trong CI) mới là thứ bắt được nó. Xem [09/memory-bugs](../../../09-debugging/memory-bugs.md).

**Chốt:** *"Đếm cả `'\0'`. Và lỗi bộ nhớ thì chứng minh bằng ASan, không bằng 'chạy thử thấy ổn'."*
</details>

#### C-051 · 🟠 · coding · ⭐ · [→ c-language-idioms §6.5](../../../01-cpp-fundamentals/c-language-idioms.md)
**Đây là macro dùng khắp Linux kernel. Đọc nó, giải thích nó làm gì, và cho biết đoạn dưới in ra gì (x86-64).**
```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct list_node { struct list_node *next; };
struct sensor { int id; char name[8]; struct list_node node; };

struct sensor s = { 7, "temp", { NULL } };
struct list_node *n = &s.node;
struct sensor *back = container_of(n, struct sensor, node);
printf("%zu %d %d %s\n", offsetof(struct sensor, node), back == &s, back->id, back->name);
```
<details><summary>Đáp án</summary>

**Output thật:** `offsetof(node) = 16` · `back == &s ? 1   id=7 name=temp`.

**Nó làm gì:** có con trỏ tới **một thành viên** (`node`), tính ngược ra địa chỉ của **struct chứa nó**: lùi lại đúng `offsetof(type, member)` byte.

```
dia chi:  s+0        s+4             s+12   s+16
          [ id  ][ name[8]          ][pad][ node ]
          ^                                 ^
          back = n - 16                     n
```
*(`offsetof` = 16: `id` 4 byte + `name` 8 byte = 12, rồi 4 byte padding để `node` — chứa một con trỏ — căn 8.)*

**Ba chi tiết phải nói được:**
1. **Ép sang `char *` trước khi trừ** — để phép trừ tính bằng **byte**. Trừ trên `struct list_node *` thì `- 16` nghĩa là lùi `16 * sizeof(struct list_node)` byte.
2. **Vì sao kernel làm vậy:** danh sách liên kết **không chứa dữ liệu**; dữ liệu **nhúng** nút danh sách vào trong nó (`struct list_head`). Một danh sách dùng được cho mọi loại struct, không cấp phát thêm, và một struct có thể nằm trong **nhiều** danh sách cùng lúc (nhiều nút).
3. **Rủi ro:** truyền nhầm `member` hoặc con trỏ không thật sự nằm trong `type` ⟹ ra địa chỉ rác, compiler không biết. Bản trong kernel thêm kiểm tra kiểu lúc biên dịch để bắt việc này.

**Chốt:** *"`container_of` lùi từ địa chỉ thành viên về địa chỉ struct bằng `offsetof` — nền của mọi danh sách liên kết trong kernel."*
</details>

---

⬅️ [Bank index](README.md)
