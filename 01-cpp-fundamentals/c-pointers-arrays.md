# C — Con trỏ, mảng, chuỗi: đọc được bộ nhớ trên giấy

> **TL;DR**
> - Con trỏ = **địa chỉ + kiểu**. Kiểu quyết định `p + 1` nhảy bao nhiêu byte và `*p` đọc bao nhiêu byte. `a[i]` chỉ là cách viết khác của `*(a + i)`.
> - **Mảng không phải con trỏ.** Tên mảng *tự đổi* (**decay**) thành con trỏ tới phần tử đầu trong hầu hết biểu thức — **trừ** `sizeof`, `&` và khi khởi tạo mảng `char` bằng chuỗi. Truyền mảng vào hàm là truyền **một con trỏ**, nên `sizeof` trong hàm cho ra 8, không phải kích thước mảng.
> - Đọc khai báo theo **luật phải–trái**: `int *a[3]` là *mảng 3 con trỏ*; `int (*a)[3]` là *một con trỏ tới mảng 3 int*. Dấu ngoặc đổi hẳn nghĩa.
> - **`T **`** dùng cho ba việc: hàm sửa con trỏ của caller · mảng con trỏ (`argv`) · ma trận cấp phát từng hàng. **`int a[R][C]` KHÔNG phải `int **`** — truyền nhầm là crash.
> - `char s[] = "abc"` là **mảng sửa được**; `char *s = "abc"` là **con trỏ tới chuỗi chỉ đọc** — ghi vào là segfault.
> - Mọi output trong file này là **output thật** (gcc 11.4, x86-64, `-std=c11 -Wall -Wextra`).

---

## 0. Vì sao phần này vẫn bị hỏi ở vị trí C++/Embedded Linux

Driver, BSP, firmware và rất nhiều thư viện hệ thống vẫn là **C thuần**, và đọc code C là việc hằng ngày ngay cả khi bạn viết C++. Interviewer hỏi con trỏ hai chiều hay mảng con trỏ **không** để kiểm tra cú pháp. Họ kiểm tra xem bạn có **vẽ được bộ nhớ** không: biến nằm ở đâu, chiếm bao nhiêu byte, mũi tên trỏ vào đâu.

> 🖊️ **Cách trả lời trên giấy — dùng cho mọi câu trong file này:** vẽ từng biến thành một **ô**, ghi **địa chỉ giả** (`0x1000`, `0x1004`…) và **kiểu** bên cạnh, rồi mới tính biểu thức. Đoán bằng trực giác là chỗ sai nhiều nhất.

---

## 1. Con trỏ = địa chỉ + kiểu

```c
int  arr[5] = {10, 20, 30, 40, 50};
int  *p = arr;            /* p tro vao arr[0] */
char *c = (char *)arr;    /* cung dia chi, KHAC kieu */
```

```
dia chi   0x1000  0x1004  0x1008  0x100C  0x1010
          +-------+-------+-------+-------+-------+
arr       |  10   |  20   |  30   |  40   |  50   |      int = 4 byte
          +-------+-------+-------+-------+-------+
            ^       ^
            p       p+1    (nhay 4 byte vi *p la int)
            c  c+1         (nhay 1 byte vi *c la char)
```

Output thật:

```
p+1 - p   (byte) = 4
c+1 - c   (byte) = 1
arr[2]=30  *(arr+2)=30  2[arr]=30
```

- **`p + n` nhảy `n * sizeof(*p)` byte.** Đây là lý do con trỏ phải có kiểu: kiểu cho compiler biết bước nhảy và số byte cần đọc khi `*p`.
- **`a[i]` được định nghĩa là `*(a + i)`.** Phép cộng giao hoán nên `2[arr]` cũng hợp lệ. Không ai viết thế, nhưng nó chứng minh `[]` chỉ là số học con trỏ.
- `const` đặt ở đâu trong khai báo con trỏ: [memory-model §5](memory-model.md).

### 1.1 Thứ tự toán tử — `*p++` và họ hàng

`*` và `++` hậu tố khác độ ưu tiên: **`++` hậu tố mạnh hơn `*`**, còn `++` tiền tố và `*` cùng mức, đọc từ phải sang trái.

```c
int a[] = {10, 20, 30};
int *p = a;
```

| Biểu thức | Nghĩa | Output thật (mỗi dòng bắt đầu lại từ `p = a`, `a[0] = 10`) |
|---|---|---|
| `x = *p++` | Đọc `*p`, **rồi** tăng **con trỏ** | `x=10, *p=20, a[0]=10` |
| `x = (*p)++` | Đọc `*p`, rồi tăng **giá trị** | `x=10, *p=11, a[0]=11` |
| `x = ++*p` | Tăng **giá trị** trước, rồi đọc | `x=11, *p=11, a[0]=11` |
| `x = *++p` | Tăng **con trỏ** trước, rồi đọc | `x=20, *p=20, a[0]=10` |

> 💡 `*p++` là thành ngữ của C: `while (*dst++ = *src++);` là `strcpy` một dòng.

---

## 2. Mảng KHÔNG phải con trỏ — array decay

```c
int arr[5];
int *p = arr;
```

```
arr:  +----+----+----+----+----+      arr LA 20 byte du lieu (khong co o nao chua dia chi)
      |    |    |    |    |    |
      +----+----+----+----+----+
        ^
        |
p:    +------+                         p LA 8 byte chua mot dia chi
      |0x1000|
      +------+
```

**Decay:** trong hầu hết biểu thức, tên mảng `arr` được tự đổi thành `&arr[0]` (kiểu `int *`). Đổi nhưng **không** có ô nhớ nào chứa con trỏ đó. Ba chỗ **không** decay:

| Chỗ | Ví dụ | Kết quả |
|---|---|---|
| `sizeof` | `sizeof(arr)` | **20** — cả mảng |
| `&` | `&arr` | Con trỏ tới **cả mảng**, kiểu `int (*)[5]` |
| Khởi tạo mảng `char` bằng chuỗi | `char s[] = "abc";` | Copy 4 byte vào mảng |

### 2.1 `arr`, `&arr[0]`, `&arr` — cùng địa chỉ, khác kiểu

| Biểu thức | Kiểu | `+1` nhảy |
|---|---|---|
| `arr` (sau decay) | `int *` | 4 byte |
| `&arr[0]` | `int *` | 4 byte |
| `&arr` | `int (*)[5]` | **20 byte** — qua hết mảng |

Output thật:

```
arr == &arr[0] ? 1   (void*)arr == (void*)&arr ? 1
&arr+1 - &arr (byte) = 20
```

### 2.2 Truyền mảng vào hàm = truyền một con trỏ

```c
void print_len(int a[]) {              /* "int a[]" o tham so CHINH LA "int *a" */
    printf("trong ham: sizeof(a) = %zu\n", sizeof(a));
}
```

gcc cảnh báo luôn:

```
warning: 'sizeof' on array function parameter 'a' will return size of 'int *' [-Wsizeof-array-argument]
```

```
sizeof(arr) = 20, sizeof(p) = 8
trong ham: sizeof(a) = 8
```

⟹ Hàm nhận mảng **luôn** phải nhận thêm **số phần tử**: `void f(const int *a, size_t n)`. Đó là lý do mọi API C đều có cặp `(buf, len)`.

**Thêm hai hệ quả:**
- **Không gán mảng được** (`a = b;` lỗi compile): mảng không phải biến chứa địa chỉ để mà đổi. Copy bằng `memcpy(a, b, sizeof a)`, hoặc bọc mảng trong `struct` (struct gán được, và kéo theo cả mảng bên trong).
- Mảng trong `struct` **không** decay khi truyền struct theo giá trị: cả mảng được copy.

---

## 3. Đọc khai báo — luật phải–trái

**Luật:** bắt đầu từ **tên biến**, đọc sang **phải** tới khi gặp `)` hoặc hết; rồi đọc sang **trái**; gặp ngoặc thì nhảy ra ngoài và lặp lại. `[]` đọc là *"mảng của"*, `()` là *"hàm trả về"*, `*` là *"con trỏ tới"*.

| Khai báo | Đọc | Là |
|---|---|---|
| `int *a[3]` | a → `[3]` → `*` → int | **mảng 3** phần tử, mỗi phần tử là **con trỏ** tới int |
| `int (*a)[3]` | a → `)` → `*` → `[3]` → int | **một con trỏ**, trỏ tới **mảng 3 int** |
| `int *f(void)` | f → `()` → `*` → int | **hàm** trả về con trỏ tới int |
| `int (*f)(void)` | f → `)` → `*` → `()` → int | **con trỏ tới hàm** trả về int |
| `int (*ops[2])(int, int)` | ops → `[2]` → `*` → `()` → int | **mảng 2 con trỏ hàm** |
| `char **argv` | argv → `*` → `*` → char | con trỏ tới con trỏ tới char |

```c
int x = 1, y = 2, z = 3;
int row[3] = {7, 8, 9};
int *ap[3]   = {&x, &y, &z};   /* mang 3 con tro */
int (*pa)[3] = &row;           /* 1 con tro toi ca mang */
```

```
ap: +------+------+------+          pa: +------+        row: +---+---+---+
    | &x   | &y   | &z   |              | &row |------->     | 7 | 8 | 9 |
    +------+------+------+              +------+             +---+---+---+
      |      |      |
      v      v      v
      x=1    y=2    z=3
```

Output thật:

```
sizeof(ap) = 24  (3 con tro)
sizeof(pa) = 8  (1 con tro)
*ap[1] = 2    (*pa)[1] = 8
pa+1 nhay 12 byte
```

### 3.1 Khai báo khó đọc thì dùng `typedef`

Ví dụ kinh điển là `signal()` của POSIX: *hàm nhận một số int và một con trỏ hàm, trả về một con trỏ hàm*.

```c
void (*signal(int sig, void (*h)(int)))(int);      /* dang tho */

typedef void (*sighandler_t)(int);                  /* dat ten cho "con tro ham nhan int" */
sighandler_t signal(int sig, sighandler_t h);        /* cung kieu, doc duoc */
```

Cả hai là **cùng một kiểu** (gcc so sánh hai con trỏ hàm khai báo theo hai cách: bằng nhau). Ở phỏng vấn, viết được bản `typedef` và giải thích vì sao nó tương đương là đủ; không cần thuộc bản thô.

---

## 4. Con trỏ tới con trỏ (`T **`) — ba lý do tồn tại

### 4.1 Hàm cần sửa **con trỏ** của caller

C truyền **theo giá trị**: hàm nhận **bản sao** của mọi tham số, kể cả con trỏ. Muốn hàm đổi *con trỏ* của caller thì phải đưa **địa chỉ của con trỏ đó**.

```c
void alloc_bad(char *p, size_t n)   { p   = malloc(n); }   /* sua BAN SAO */
void alloc_ok (char **pp, size_t n) { *pp = malloc(n); }   /* sua con tro CUA CALLER */

char *buf = NULL;
alloc_bad(buf, 16);     /* buf van NULL, 16 byte bi mat */
alloc_ok(&buf, 16);     /* buf tro toi vung moi */
```

```
alloc_bad:  buf [NULL]        p [0x5000] ---> (16 byte, khong ai giu)  => LEAK
alloc_ok:   buf [0x6000] <--- pp [&buf]       *pp = malloc(...) ghi THANG vao buf
```

Output thật (gcc đã cảnh báo `parameter 'p' set but not used` ở `alloc_bad`; valgrind xác nhận leak):

```
sau alloc_bad: buf = (nil)
sau alloc_ok : buf != NULL
==6393==    definitely lost: 16 bytes in 1 blocks
```

> Quy tắc nhớ: muốn hàm sửa một **`T`** thì truyền **`T *`**. `T` ở đây là `char *` ⟹ truyền `char **`. Trong C++ thì dùng `char *&` (tham chiếu tới con trỏ) cho gọn.

### 4.2 Danh sách liên kết: sửa "ô chứa con trỏ", không cần case riêng cho node đầu

```c
typedef struct Node { int val; struct Node *next; } Node;

void push_front(Node **head, int v) {
    Node *n = malloc(sizeof *n);
    n->val = v; n->next = *head;
    *head = n;                                 /* sua head CUA CALLER */
}

void remove_val(Node **head, int v) {
    Node **pp = head;                          /* pp tro vao O CHUA con tro (head, roi cac ->next) */
    while (*pp && (*pp)->val != v)
        pp = &(*pp)->next;
    if (*pp) { Node *dead = *pp; *pp = dead->next; free(dead); }
}
```

```
head          n1               n2
+----+      +----+----+      +----+----+
| o--+----> | 4  | o--+----> | 3  | o--+----> ...
+----+      +----+----+      +----+----+
  ^                  ^
  pp (lan 1)         pp (lan 2) = &n1->next
```

`pp` không trỏ vào **node**, mà trỏ vào **ô đang chứa mũi tên tới node**: lúc đầu là `head`, sau đó là `->next` của node trước. Xoá node chỉ là ghi đè ô đó, nên node đầu và node giữa đi chung **một** đường code.

Output thật (valgrind: `All heap blocks were freed`):

```
4 3 2 1
3 2 1        <- xoa 4 (node dau), khong can if rieng
3 1          <- xoa 2 (node giua)
```

### 4.3 Mảng con trỏ — `argv`

```
argv ---> +-------+
          |   o---+---> "./app\0"
          +-------+
          |   o---+---> "-v\0"
          +-------+
          | NULL  |     <- argv[argc] luon la NULL (chuan bao dam)
          +-------+
```

`argv` là `char **` vì nó trỏ vào phần tử đầu của một **mảng các `char *`**. Mỗi chuỗi nằm riêng một chỗ, dài ngắn tuỳ ý.

---

## 5. Mảng hai chiều — ba cách, ba bố cục bộ nhớ

### 5.1 `int a[R][C]` — một khối liền, theo hàng (row-major)

```c
int a[3][4] = { {0,1,2,3}, {10,11,12,13}, {20,21,22,23} };
```

```
          a[0]                a[1]                    a[2]
+----+----+----+----+-----+-----+-----+-----+-----+-----+-----+-----+
|  0 |  1 |  2 |  3 | 10  | 11  | 12  | 13  | 20  | 21  | 22  | 23  |   48 byte LIEN KHOI
+----+----+----+----+-----+-----+-----+-----+-----+-----+-----+-----+
dia chi cua a[i][j] = dia chi a + (i * C + j) * sizeof(int)
```

Output thật:

```
sizeof(a)=48 sizeof(a[0])=16
&a[1][0] - &a[0][0] = 4 phan tu (lien khoi, row-major)
a[1][3] = 13 = *(*(a+1)+3) = 13
```

- `a` decay thành con trỏ tới **hàng đầu**, kiểu **`int (*)[4]`**, chứ không phải `int **`.
- Để tính địa chỉ `a[i][j]`, compiler **phải biết `C`** (số cột). Vì thế khi truyền vào hàm, **chỉ chiều đầu được bỏ trống**.

**Ba chữ ký hợp lệ cho hàm nhận `int a[3][4]`:**

```c
void f(int rows, int m[][4]);         /* so cot co dinh luc compile */
void f(int rows, int (*m)[4]);        /* y het dong tren — viet ro rang */
void f(int rows, int cols, int m[rows][cols]);   /* C99 VLA: so cot luc chay */
```

### 5.2 ⚠️ Lỗi kinh điển: truyền `int a[3][4]` vào hàm nhận `int **`

```c
void print_bad(int **m) { printf("%d\n", m[1][3]); }
print_bad(a);
```

```
warning: passing argument 1 of 'print_bad' from incompatible pointer type [-Wincompatible-pointer-types]
note: expected 'int **' but argument is of type 'int (*)[4]'
Segmentation fault (core dumped)          <- exit 139
```

**Vì sao crash:** `m[1]` với `m` kiểu `int **` nghĩa là *"đọc ô thứ 1 trong một mảng các **con trỏ**, lấy giá trị đó làm địa chỉ"*. Nhưng `a` không chứa con trỏ nào, chỉ chứa số. Các số nguyên (`2`, `3`…) bị đọc như **địa chỉ** ⟹ dereference rác. Đây là chỗ gcc chỉ **cảnh báo** (C cho phép đổi kiểu con trỏ ngầm), nên **luôn build với `-Wall -Werror`** hoặc ít nhất đọc warning.

### 5.3 Cấp phát động — hai cách

**Cách 1 — mảng con trỏ hàng (`int **`)**: `R + 1` lần `malloc`.

```c
int **alloc_rows(int R, int C) {
    int **m = malloc(R * sizeof *m);           /* mang R con tro */
    if (!m) return NULL;
    for (int i = 0; i < R; i++) {
        m[i] = malloc(C * sizeof **m);         /* moi hang mot vung rieng */
        if (!m[i]) {                           /* that bai giua chung: don phan da cap */
            while (i--) free(m[i]);
            free(m);
            return NULL;
        }
    }
    return m;
}
void free_rows(int **m, int R) {
    for (int i = 0; i < R; i++) free(m[i]);    /* hang truoc... */
    free(m);                                    /* ...mang con tro sau */
}
```

**Cách 2 — một khối liền, vẫn viết `m[i][j]`**: một lần `malloc`.

```c
int (*m)[C] = malloc(R * sizeof *m);    /* sizeof *m = C * sizeof(int) */
m[2][3] = 23;
free(m);                                /* MOT lan free */
```

Output thật (cùng `R=3, C=4`):

```
cach 1: m1[2][3]=23, hang 1 cach hang 0: 32 byte     <- KHONG lien khoi (16 byte du lieu + metadata cua malloc)
cach 2: m2[2][3]=23, hang 1 cach hang 0: 16 byte, sizeof *m2=16
All heap blocks were freed -- no leaks are possible
```

| | Cách 1: `int **` từng hàng | Cách 2: một khối `int (*)[C]` | Cách 3: `int *` phẳng, `m[i*C + j]` |
|---|---|---|---|
| Số lần `malloc` / `free` | R + 1 | **1** | **1** |
| Liền khối (cache, `memcpy` cả ma trận) | ❌ | ✅ | ✅ |
| Hàng dài khác nhau (jagged) | ✅ | ❌ | ❌ |
| Đổi chỗ hai hàng | ✅ đổi 2 con trỏ, O(1) | ❌ copy dữ liệu | ❌ copy dữ liệu |
| Dọn dẹp khi `malloc` hỏng giữa chừng | Phức tạp | Không có | Không có |
| Ghi chú | | `C` chạy lúc runtime ⟹ kiểu VLA: bắt buộc ở C99, **tuỳ chọn** ở C11, **không có** trong C++ | Chạy mọi nơi, kể cả C++ |

> 🎯 **Câu trả lời mặc định ở phỏng vấn:** *"Một khối liền — một lần cấp, một lần giải phóng, thân thiện cache. Chỉ dùng mảng con trỏ khi các hàng dài khác nhau hoặc cần đổi chỗ hàng rẻ."*

---

## 6. Chuỗi

### 6.1 `char s[]` vs `char *s`

```c
char  s1[] = "abc";     /* MANG 4 byte (gom '\0') tren stack, COPY tu literal */
char *s2   = "abc";     /* CON TRO toi literal nam o vung CHI DOC (.rodata) */
```

```
stack:   s1 [a][b][c][\0]          s2 [0x4010]--+
                                                |
.rodata:                       0x4010 [a][b][c][\0]   <- chi doc
```

Output thật:

```
sizeof(s1)=4 sizeof(s2)=8
s1 = Xbc                                    <- s1[0] = 'X' OK
Segmentation fault (core dumped)            <- s2[0] = 'X' ghi vao vung chi doc, exit 139
```

> Kiểu đúng của con trỏ tới literal là **`const char *`**. Viết `const` thì `s2[0] = 'X'` thành **lỗi compile** thay vì crash lúc chạy. (Trong C++ literal có kiểu `const char[4]` và chuẩn C++11 **cấm** gán vào `char *`, nhưng g++ 11 vẫn chỉ cảnh báo: `ISO C++ forbids converting a string constant to 'char*' [-Wwrite-strings]`.)

### 6.2 Mảng con trỏ chuỗi vs mảng 2 chiều `char`

```c
char *names[]   = {"go", "rust", "c"};   /* 3 con tro, chuoi nam o .rodata */
char  grid[][10] = {"go", "rust", "c"};  /* 3 x 10 byte lien khoi, sua duoc */
```

```
names: [o][o][o]           grid: [g o \0 . . . . . . .]
        |  |  |                  [r u s t \0 . . . . . ]
        v  v  v                  [c \0 . . . . . . . . ]
      "go" "rust" "c"
```

```
sizeof(names)=24 sizeof(grid)=30
```

| | `char *names[]` | `char grid[][10]` |
|---|---|---|
| Bộ nhớ | N con trỏ + từng chuỗi riêng | N × 10 byte, lãng phí phần đệm |
| Sửa nội dung chuỗi | ❌ (literal chỉ đọc) | ✅ |
| Chuỗi dài hơn 9 ký tự | ✅ | ❌ cắt / lỗi compile |
| Đổi chỗ hai phần tử | O(1) — đổi con trỏ | Copy byte |

### 6.3 Trả về con trỏ tới mảng cục bộ

```c
char *fmt_id(int id) {
    char buf[16];
    snprintf(buf, sizeof buf, "ID-%04d", id);
    return buf;                 /* buf chet khi ham return */
}
```

```
warning: function returns address of local variable [-Wreturn-local-addr]
```

Trên máy này, **gcc 11.4 cố ý trả về `NULL`** thay cho địa chỉ đó (đã in thử: `con tro tra ve = (nil)` ở cả `-O0` lẫn `-O2`), nên `printf("%s")` segfault ngay. Compiler khác, hoặc khi gcc không chứng minh được, sẽ trả về địa chỉ stack thật: đọc ra **rác**, hoặc tệ hơn là *"chạy đúng"* lúc test (xem [CPP-060](../14-prep/mock-interview/bank/cpp.md)). Đây là **UB** ở mọi trường hợp.

**Ba cách sửa, ba đánh đổi:**

| Cách | Code | Đánh đổi |
|---|---|---|
| ✅ **Caller cấp buffer** | `int fmt_id(int id, char *out, size_t n)` | Rõ ai sở hữu; không cấp phát. Cách chuẩn của API C (`snprintf`, `strerror_r`) |
| `static char buf[16]` | Giữ nguyên chữ ký | **Không reentrant, không thread-safe**; lần gọi sau ghi đè kết quả lần trước |
| `malloc` | Trả về vùng heap | Caller **phải nhớ `free`** — dễ leak |

### 6.4 `'\0'` và các lỗi lệch một

- `strlen("abc") == 3`, nhưng cần **4** byte. `malloc(strlen(s))` rồi `strcpy` là **ghi lố 1 byte**.
- `char s[3] = "abc";` hợp lệ trong C — gcc không cảnh báo gì — nhưng không có chỗ cho `'\0'` ⟹ **không còn là chuỗi**. Trong C++ là lỗi: `initializer-string for 'char [3]' is too long`.
- So sánh chuỗi bằng `==` là so sánh **địa chỉ**; dùng `strcmp`.

---

## 7. Con trỏ hàm và `void *`

### 7.1 Con trỏ hàm — bảng dispatch

```c
static int op_add(int a, int b) { return a + b; }
static int op_sub(int a, int b) { return a - b; }
static int (*ops[])(int, int) = { op_add, op_sub };   /* mang con tro ham */

ops[1](5, 3);    /* = 2 */
```

```
ops[0](5,3)=8 ops[1](5,3)=2 sizeof(ops)=16
```

Mảng / struct con trỏ hàm là cách C làm **đa hình**: `file_operations` trong kernel, hay bảng `panel_ops` ở [in-practice/A1 §6](../11-design-patterns/in-practice/A1-baseline-libdisplay.md). Nó là **vtable viết tay**, nên thừa hưởng đúng hai rủi ro của vtable: slot `NULL` bị gọi thì crash, chèn slot vào giữa thì mọi slot sau lệch.

### 7.2 Callback — `qsort` và comparator

```c
int cmp_bad(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }
int cmp_ok (const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);                 /* -1, 0, 1 — khong tran so */
}
```

Sắp `{INT_MIN, 1, INT_MAX, 0, -5}`, output thật:

```
cmp_bad: 1 2147483647 -2147483648 -5 0         <- SAI thu tu
cmp_ok : -2147483648 -5 0 1 2147483647
```

`a - b` **tràn số** khi hai giá trị trái dấu và xa nhau (`1 - INT_MIN`). Tràn số có dấu là **UB**; thực tế nó quấn vòng thành số âm, comparator nói ngược, và `qsort` cho kết quả sai mà không báo gì.

### 7.3 `void *` — con trỏ "không kiểu"

| Làm được | Không làm được |
|---|---|
| Nhận địa chỉ của **mọi** kiểu dữ liệu, gán ngầm qua lại với `T *` (trong C) | **Dereference** — không biết đọc bao nhiêu byte |
| Truyền qua API tổng quát: `malloc`, `memcpy`, `qsort`, `pthread_create` | **Số học** — không biết bước nhảy (`void *p; p + 1` chỉ chạy nhờ **extension của GNU**, không phải chuẩn C; gcc im lặng với `-Wall -Wextra`, chỉ cảnh báo khi bật `-Wpedantic`/`-Wpointer-arith`) |

- Trong C, `int *p = malloc(n);` **không cần ép kiểu**. Ép kiểu còn có hại ở C cũ: che mất lỗi quên `#include <stdlib.h>`.
- Trong C++, `void *` → `T *` **phải ép tường minh** (`static_cast`), vì đó là chuyển đổi mất kiểu.

---

## 8. Bảng ôn nhanh — bẫy hay gặp

| Bẫy | Trông như | Thật ra |
|---|---|---|
| `sizeof(a)` trong hàm nhận `int a[]` | Kích thước mảng | Kích thước **con trỏ** (8) |
| `int a[3][4]` truyền vào `int **` | Ma trận | **Sai kiểu** — crash (§5.2) |
| `char *s = "abc"; s[0] = 'X';` | Sửa chuỗi | Ghi vào **vùng chỉ đọc** — segfault |
| `*p++` | Tăng giá trị | Tăng **con trỏ** |
| `void f(char *p) { p = malloc(n); }` | Cấp cho caller | Chỉ sửa **bản sao** — leak |
| `return buf;` (`buf` là mảng cục bộ) | Trả chuỗi | Con trỏ **treo** — UB |
| `malloc(strlen(s))` | Đủ chỗ | **Thiếu 1 byte** cho `'\0'` |
| `if (s1 == s2)` với hai chuỗi | So nội dung | So **địa chỉ** |
| `return a - b;` trong comparator | Ngắn gọn | **Tràn số** với giá trị trái dấu |
| `int *p; *p = 5;` | Gán | Con trỏ **chưa khởi tạo** — ghi vào địa chỉ rác |
| Trừ hai con trỏ của **hai mảng khác nhau** | Khoảng cách | **UB** — chỉ hợp lệ trong cùng một mảng (kết quả kiểu `ptrdiff_t`) |

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở. Domain riêng: [bank/c-programming.md](../14-prep/mock-interview/bank/c-programming.md).

| ID | Câu hỏi |
|----|---------|
| [C-001](../14-prep/mock-interview/bank/c-programming.md) | `p + 1` với `int *` và `char *` nhảy bao nhiêu byte? `a[i]` thực chất là gì? |
| [C-002](../14-prep/mock-interview/bank/c-programming.md) ⭐ | `*p++`, `(*p)++`, `++*p`, `*++p` — mỗi cái làm gì? |
| [C-003](../14-prep/mock-interview/bank/c-programming.md) | Con trỏ chưa khởi tạo, con trỏ `NULL`, con trỏ treo — khác nhau ra sao? |
| [C-004](../14-prep/mock-interview/bank/c-programming.md) ⭐ | `sizeof(a)/sizeof(a[0])` trong hàm nhận `int a[]` in ra gì? Sửa thế nào? |
| [C-005](../14-prep/mock-interview/bank/c-programming.md) | `arr`, `&arr[0]`, `&arr` — giá trị, kiểu, `+1` |
| [C-006](../14-prep/mock-interview/bank/c-programming.md) | Vì sao không gán mảng được? Copy mảng thế nào? |
| [C-007](../14-prep/mock-interview/bank/c-programming.md) ⭐ | `int *a[3]` vs `int (*a)[3]` — vẽ bộ nhớ, `sizeof` |
| [C-008](../14-prep/mock-interview/bank/c-programming.md) | Đọc khai báo `signal()` và viết lại bằng `typedef` |
| [C-009](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Hàm `alloc(char *p, n)` không cấp được cho caller — vì sao, sửa thế nào? |
| [C-010](../14-prep/mock-interview/bank/c-programming.md) | Xoá node trong linked list bằng `Node **` — không cần case riêng cho head |
| [C-011](../14-prep/mock-interview/bank/c-programming.md) | `char **argv` nằm trong bộ nhớ thế nào? `argv[argc]` là gì? |
| [C-012](../14-prep/mock-interview/bank/c-programming.md) ⭐ | `int a[3][4]` nằm thế nào trong bộ nhớ? Vì sao truyền vào hàm phải ghi số cột? |
| [C-013](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Truyền `int a[3][4]` vào hàm nhận `int **` — chuyện gì xảy ra? |
| [C-014](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Viết hàm cấp phát ma trận R×C động — hai cách, so sánh |
| [C-015](../14-prep/mock-interview/bank/c-programming.md) ⭐ | `char s[] = "abc"` vs `char *s = "abc"` |
| [C-016](../14-prep/mock-interview/bank/c-programming.md) | `char *names[]` vs `char names[][10]` |
| [C-017](../14-prep/mock-interview/bank/c-programming.md) | Hàm trả về chuỗi format trong mảng cục bộ — lỗi gì, ba cách sửa |
| [C-018](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Viết comparator cho `qsort`; vì sao `return a - b` nguy hiểm? |
| [C-019](../14-prep/mock-interview/bank/c-programming.md) | Bảng con trỏ hàm thay `switch` — khi nào, rủi ro gì? |
| [C-020](../14-prep/mock-interview/bank/c-programming.md) | `void *` làm được gì, không làm được gì? |
| [CPP-003](../14-prep/mock-interview/bank/cpp.md) | `const int *p` / `int *const p` / `const int *const p` |
| [CPP-060](../14-prep/mock-interview/bank/cpp.md) | Trả về địa chỉ biến cục bộ — vì sao "chạy được" lúc thử? |

---
⬅️ [memory-model.md](memory-model.md) · [Về index topic](README.md) · ➡️ Tiếp theo: [c-language-idioms.md](c-language-idioms.md) *(macro, linkage, byte, `goto cleanup`)*
