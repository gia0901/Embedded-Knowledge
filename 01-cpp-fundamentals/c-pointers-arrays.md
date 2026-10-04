# C — Con trỏ, mảng, chuỗi: đọc được bộ nhớ trên giấy

> **TL;DR**
> - Con trỏ = **địa chỉ + kiểu**. Kiểu quyết định `p + 1` nhảy bao nhiêu byte và `*p` đọc bao nhiêu byte. `a[i]` chỉ là cách viết khác của `*(a + i)`.
> - **Mảng không phải con trỏ.** Tên mảng *tự đổi* (**decay**) thành con trỏ tới phần tử đầu trong hầu hết biểu thức — **trừ** `sizeof`, `&` và khi khởi tạo mảng `char` bằng chuỗi. Truyền mảng vào hàm là truyền **một con trỏ**, nên `sizeof` trong hàm cho ra 8, không phải kích thước mảng.
> - Đọc khai báo theo **luật phải–trái**: bắt đầu từ tên, đọc `[]`/`()` bên phải **trước** `*` bên trái (vì chúng ưu tiên cao hơn), ngoặc nhóm ép đổi thứ tự. `int *a[3]` là *mảng 3 con trỏ*; `int (*a)[3]` là *một con trỏ tới mảng 3 int*.
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
int  *p = arr;            /* p trỏ vào arr[0] */
char *c = (char *)arr;    /* cùng địa chỉ, KHÁC kiểu */
```

```
địa chỉ   0x1000  0x1004  0x1008  0x100C  0x1010
          +-------+-------+-------+-------+-------+
arr       |  10   |  20   |  30   |  40   |  50   |      int = 4 byte
          +-------+-------+-------+-------+-------+
            ^       ^
            p       p+1    (nhảy 4 byte vì *p là int)
            c  c+1         (nhảy 1 byte vì *c là char)
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

Một biểu thức như `*p++` đặt ra **hai câu hỏi khác nhau**, hay bị trộn vào nhau:

| Câu hỏi | Ai trả lời | Với `*p++` |
|---|---|---|
| ① **Gom nhóm**: `*` và `++` áp vào **cái gì**? | **Độ ưu tiên** (precedence) | `*(p++)` — `++` áp vào `p`, rồi `*` áp vào kết quả của `p++` |
| ② **Thời điểm**: giá trị nào được dùng, khi nào tăng? | **Ngữ nghĩa của `++`** | `p++` trả về **giá trị cũ** của `p`, việc tăng `p` là tác dụng phụ |

**"Ưu tiên cao hơn" (hay "mạnh hơn") nghĩa là: khi không có ngoặc, compiler tự đặt ngoặc quanh toán tử đó trước.** Giống `2 + 3 * 4` được hiểu là `2 + (3 * 4)` vì `*` ưu tiên cao hơn `+`. Ở đây `++` hậu tố ưu tiên cao hơn `*`, nên `*p++` được hiểu là `*(p++)`, **không phải** `(*p)++`. Độ ưu tiên chỉ quyết định `++` tăng **con trỏ `p`** hay tăng **giá trị `*p`**; nó không nói gì về chuyện tăng trước hay sau.

Chuyện *"đọc xong rồi mới tăng"* đến từ câu hỏi ②: `p++` là một biểu thức có **giá trị bằng `p` cũ**. Nên `*(p++)` = `*(p cũ)` = đọc `a[0]`; sau đó `p` đã trỏ sang `a[1]`.

Còn `++` **tiền tố** và `*` **cùng mức ưu tiên**. Khi cùng mức thì xét **chiều kết hợp**: hai toán tử này kết hợp **từ phải sang trái**, tức là toán tử nằm **sát biến hơn** được gom trước. `++*p` thành `++(*p)`, `*++p` thành `*(++p)`.

```c
int a[] = {10, 20, 30};
int *p = a;
```

| Biểu thức | Compiler hiểu là | ++ tăng cái gì | Giá trị được dùng | Output thật (mỗi dòng bắt đầu lại từ `p = a`, `a[0] = 10`) |
|---|---|---|---|---|
| `x = *p++` | `*(p++)` | **con trỏ** `p` | `*` của `p` **cũ** ⟹ 10 | `x=10, *p=20, a[0]=10` |
| `x = (*p)++` | `(*p)++` — ngoặc ép | **giá trị** `a[0]` | `a[0]` **cũ** ⟹ 10 | `x=10, *p=11, a[0]=11` |
| `x = ++*p` | `++(*p)` | **giá trị** `a[0]` | `a[0]` **mới** ⟹ 11 | `x=11, *p=11, a[0]=11` |
| `x = *++p` | `*(++p)` | **con trỏ** `p` | `*` của `p` **mới** ⟹ 20 | `x=20, *p=20, a[0]=10` |

Mẹo đọc: **bước 1** đặt ngoặc theo độ ưu tiên để biết `++` tăng cái gì; **bước 2** nhìn `++` đứng trước (dùng giá trị mới) hay đứng sau (dùng giá trị cũ).

> 💡 `*p++` là thành ngữ của C: `while (*dst++ = *src++);` là `strcpy` một dòng.

---

## 2. Mảng KHÔNG phải con trỏ — array decay

```c
int arr[5];
int *p = arr;
```

```
arr:  +----+----+----+----+----+      arr LÀ 20 byte dữ liệu (không có ô nào chứa địa chỉ)
      |    |    |    |    |    |
      +----+----+----+----+----+
        ^
        |
p:    +------+                         p LÀ 8 byte chứa một địa chỉ
      |0x1000|
      +------+
```

**Decay:** trong hầu hết biểu thức, tên mảng `arr` được tự đổi thành `&arr[0]` (kiểu `int *`). Đây là một **giá trị được tính ra** tại chỗ dùng (giống kết quả của `1 + 2`), **không** phải một biến nằm đâu đó trong bộ nhớ — sơ đồ trên không có ô nào chứa `0x1000` ngoài `p`. Ba chỗ **không** decay:

| Chỗ | Ví dụ | Kết quả |
|---|---|---|
| `sizeof` | `sizeof(arr)` | **20** — cả mảng |
| `&` | `&arr` | Con trỏ tới **cả mảng**, kiểu `int (*)[5]` |
| Chuỗi literal dùng để khởi tạo mảng `char` | `char s[] = "abc";` | Bản thân `"abc"` cũng là một mảng (`char[4]`). Ở chỗ này nó **không** decay thành con trỏ, mà 4 byte của nó được **copy** vào `s` |

### 2.1 `arr`, `&arr[0]`, `&arr` — cùng địa chỉ, khác kiểu

| Biểu thức | Kiểu | `+1` nhảy |
|---|---|---|
| `arr` (sau decay) | `int *` | 4 byte |
| `&arr[0]` | `int *` | 4 byte |
| `&arr` | `int (*)[5]` | **20 byte** — qua hết mảng |

Vì sao `&arr + 1` nhảy 20 byte: áp đúng quy tắc §1 — `p + 1` nhảy `sizeof(*p)` byte. Với `&arr`, thứ được trỏ tới là **cả mảng** `int[5]`, nên `sizeof` của nó là 20.

Output thật:

```
arr == &arr[0] ? 1   (void*)arr == (void*)&arr ? 1
&arr+1 - &arr (byte) = 20
```

### 2.2 Truyền mảng vào hàm = truyền một con trỏ

```c
void print_len(int a[]) {              /* "int a[]" ở tham số CHÍNH LÀ "int *a" */
    printf("trong hàm: sizeof(a) = %zu\n", sizeof(a));
}
```

gcc cảnh báo luôn:

```
warning: 'sizeof' on array function parameter 'a' will return size of 'int *' [-Wsizeof-array-argument]
```

```
sizeof(arr) = 20, sizeof(p) = 8
trong hàm: sizeof(a) = 8
```

⟹ Hàm nhận mảng **luôn** phải nhận thêm **số phần tử**: `void f(const int *a, size_t n)`. Đó là lý do mọi API C đều có cặp `(buf, len)`.

**Thêm hai hệ quả:**
- **Không gán mảng được** (`a = b;` lỗi compile): mảng không phải biến chứa địa chỉ để mà đổi. Copy bằng `memcpy(a, b, sizeof a)`, hoặc bọc mảng trong `struct` (struct gán được, và kéo theo cả mảng bên trong).
- Mảng trong `struct` **không** decay khi truyền struct theo giá trị: cả mảng được copy.

---

## 3. Đọc khai báo — luật phải–trái

### 3.0 Vì sao phải đọc sang phải trước

Khai báo trong C được thiết kế để **giống cách dùng**: `int *a[3];` nghĩa là *"biểu thức `*a[i]` cho ra một `int`"*. Mà trong biểu thức, `[]` và `()` đứng **sau** tên có độ ưu tiên **cao hơn** `*` đứng **trước** tên (cùng lý do như `*p++` ở §1.1). Nên khi gặp `*a[3]`, compiler gom `a[3]` trước:

```
int *a[3]      ==   int *(a[3])      a là MẢNG trước, rồi mới tới "con trỏ"
int (*a)[3]    ==   ngoặc ép *a      a là CON TRỎ trước, rồi mới tới "mảng"
```

⟹ Thứ đứng **bên phải** tên (`[]`, `()`) luôn được đọc **trước** thứ đứng **bên trái** (`*`) — trừ khi có ngoặc ép thứ tự khác. Đó là toàn bộ lý do của luật phải–trái.

### 3.1 Thuật toán đọc — từng bước

Có **hai loại ngoặc tròn**, phải phân biệt:
- **Ngoặc hàm** — đứng **ngay sau** một tên hoặc sau `)`, chứa danh sách tham số: `f(void)`, `(int, int)`. Đọc là *"hàm nhận … trả về"*.
- **Ngoặc nhóm** — **bao quanh** tên cùng dấu `*`: `(*f)`, `(*a)`. Không mang nghĩa gì, chỉ để ép thứ tự đọc.

Các bước:

1. Tìm **tên** đang được khai báo. Bắt đầu từ đó.
2. **Nhìn sang phải**, đọc lần lượt:
   - `[N]` ⟹ *"mảng N phần tử, mỗi phần tử là…"*
   - `( … )` (ngoặc hàm) ⟹ *"hàm nhận …, trả về…"*

   Dừng lại khi gặp **dấu `)` đóng ngoặc nhóm** hoặc **hết khai báo**.
3. **Nhìn sang trái**, đọc lần lượt:
   - `*` ⟹ *"con trỏ tới…"*

   Dừng lại khi gặp **dấu `(` mở ngoặc nhóm** hoặc **hết**.
4. Nếu vừa dừng ở một cặp ngoặc nhóm: coi cả cặp ngoặc như đã đọc xong, **đứng ở ngoài nó**, quay lại bước 2.
5. Cuối cùng đọc **kiểu cơ sở** ở tận cùng bên trái (`int`, `char`…).

**Áp vào từng ví dụ:**

`int *f(void)` — không có ngoặc nhóm:
```
bước 1:  f
bước 2:  phải của f là (void)  -> "f là hàm không nhận tham số, trả về..."   ; tiếp sang phải: hết
bước 3:  trái là *             -> "...con trỏ tới..."                        ; tiếp sang trái: chỉ còn kiểu
bước 5:  int                   -> "...int"
=> f là HÀM trả về con trỏ tới int
```

`int (*f)(void)` — có ngoặc nhóm quanh `*f`:
```
bước 1:  f
bước 2:  phải của f là )  -> đóng ngoặc nhóm, CHƯA đọc được gì, dừng
bước 3:  trái là *        -> "f là con trỏ tới..."  ; tiếp sang trái là (  -> mở ngoặc nhóm, dừng
bước 4:  đứng ngoài (*f), quay lại bước 2
bước 2:  phải là (void)   -> "...hàm không nhận tham số, trả về..."
bước 5:  int              -> "...int"
=> f là CON TRỎ tới hàm trả về int
```

`int *a[3]` và `int (*a)[3]` — cùng cách đó:
```
int *a[3]:    a -> phải [3] "mảng 3 phần tử, mỗi phần tử là" -> trái * "con trỏ tới" -> int
int (*a)[3]:  a -> phải ) dừng -> trái * "con trỏ tới" -> ( dừng -> ra ngoài -> phải [3] "mảng 3" -> int
```

`int (*ops[2])(int, int)`:
```
ops -> phải [2]   "mảng 2 phần tử, mỗi phần tử là"   -> gặp ) dừng
    -> trái *     "con trỏ tới"                       -> gặp ( dừng -> ra ngoài
    -> phải (int, int)  "hàm nhận (int, int), trả về"
    -> int
=> ops là MẢNG 2 CON TRỎ tới hàm nhận (int, int) trả về int
```

| Khai báo | Là |
|---|---|
| `int *a[3]` | **mảng 3** phần tử, mỗi phần tử là **con trỏ** tới int |
| `int (*a)[3]` | **một con trỏ**, trỏ tới **mảng 3 int** |
| `int *f(void)` | **hàm** trả về con trỏ tới int |
| `int (*f)(void)` | **con trỏ tới hàm** trả về int |
| `int (*ops[2])(int, int)` | **mảng 2 con trỏ hàm** |
| `char **argv` | con trỏ tới con trỏ tới char (bên phải `argv` không có gì ⟹ đọc thẳng sang trái) |

```c
int x = 1, y = 2, z = 3;
int row[3] = {7, 8, 9};
int *ap[3]   = {&x, &y, &z};   /* mảng 3 con trỏ */
int (*pa)[3] = &row;           /* 1 con trỏ tới cả mảng */
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
sizeof(ap) = 24  (3 con trỏ)
sizeof(pa) = 8  (1 con trỏ)
*ap[1] = 2    (*pa)[1] = 8
pa+1 nhảy 12 byte
```

`pa + 1` nhảy 12 byte cùng lý do với `&arr + 1` ở §2.1: thứ được trỏ tới là cả mảng `int[3]`.

### 3.2 Khai báo khó đọc thì dùng `typedef`

Ví dụ kinh điển là `signal()` của POSIX: *hàm nhận một số int và một con trỏ hàm, trả về một con trỏ hàm*.

```c
void (*signal(int sig, void (*h)(int)))(int);      /* dạng thô */

typedef void (*sighandler_t)(int);                  /* đặt tên cho "con trỏ hàm nhận int" */
sighandler_t signal(int sig, sighandler_t h);        /* cùng kiểu, đọc được */
```

Đọc bản thô theo thuật toán §3.1:

```
void (*signal(int sig, void (*h)(int)))(int);

1. tên: signal
2. phải: (int sig, void (*h)(int))  -> "signal là hàm nhận một int và một h, trả về..."
         (h tự đọc riêng: h -> phải ) dừng -> trái * "con trỏ tới" -> ra ngoài -> phải (int)
          "hàm nhận int" -> void  =>  h là con trỏ tới hàm nhận int, trả về void)
   tiếp sang phải: gặp ) đóng ngoặc nhóm, dừng
3. trái: *                           -> "...con trỏ tới..."   ; gặp ( dừng
4. ra ngoài cặp ngoặc nhóm (* signal(...))
2. phải: (int)                       -> "...hàm nhận int, trả về..."
5. void                              -> "...void"
=> signal là HÀM nhận (int, con trỏ hàm), trả về CON TRỎ tới hàm nhận int trả về void
```

Kiểu trả về là một con trỏ hàm, nên nó buộc phải **bọc quanh** tên `signal` — đó là lý do khai báo trông "lộn trong ra ngoài". `typedef` đặt tên cho kiểu *"con trỏ tới hàm nhận int trả void"*, nên khai báo quay về dạng quen thuộc `KiểuTrảVề tên(tham số)`.

Cả hai là **cùng một kiểu** (gcc so sánh hai con trỏ hàm khai báo theo hai cách: bằng nhau). Ở phỏng vấn, viết được bản `typedef` và giải thích vì sao nó tương đương là đủ; không cần thuộc bản thô.

---

## 4. Con trỏ tới con trỏ (`T **`) — ba lý do tồn tại

### 4.1 Hàm cần sửa **con trỏ** của caller

C truyền **theo giá trị**: hàm nhận **bản sao** của mọi tham số, kể cả con trỏ. Muốn hàm đổi *con trỏ* của caller thì phải đưa **địa chỉ của con trỏ đó**.

```c
void alloc_bad(char *p, size_t n)   { p   = malloc(n); }   /* sửa BẢN SAO */
void alloc_ok (char **pp, size_t n) { *pp = malloc(n); }   /* sửa con trỏ CỦA CALLER */

char *buf = NULL;
alloc_bad(buf, 16);     /* buf vẫn NULL, 16 byte bị mất */
alloc_ok(&buf, 16);     /* buf trỏ tới vùng mới */
```

```
alloc_bad:  buf [NULL]        p [0x5000] ---> (16 byte, không ai giữ)  => LEAK
alloc_ok:   buf [0x6000] <--- pp [&buf]       *pp = malloc(...) ghi THẲNG vào buf
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
    *head = n;                                 /* sửa head CỦA CALLER */
}

void remove_val(Node **head, int v) {
    Node **pp = head;                          /* pp trỏ vào Ô CHỨA con trỏ (head, rồi các ->next) */
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
  pp (lần 1)         pp (lần 2) = &n1->next
```

`pp` không trỏ vào **node**, mà trỏ vào **ô đang chứa mũi tên tới node**: lúc đầu là `head`, sau đó là `->next` của node trước. Xoá node chỉ là ghi đè ô đó, nên node đầu và node giữa đi chung **một** đường code.

> **Vì sao có ngoặc ở `(*pp)->val` và không cần ở `&(*pp)->next`:** `->` (cũng như `[]`, `()`, `++` hậu tố) có độ ưu tiên **cao hơn** `*` và `&` đứng trước. Bỏ ngoặc thì `*pp->val` bị hiểu là `*(pp->val)` — lấy `->val` trên `pp` (kiểu `Node **`, không phải con trỏ tới struct) — và gcc báo: `error: '*pp' is a pointer; did you mean to use '->'?`. Còn `&(*pp)->next` được hiểu là `&((*pp)->next)` — đúng ý *"địa chỉ của ô `next`"* — nên không cần thêm ngoặc.

Output thật (valgrind: `All heap blocks were freed`):

```
4 3 2 1
3 2 1        <- xoá 4 (node đầu), không cần if riêng
3 1          <- xoá 2 (node giữa)
```

### 4.3 Mảng con trỏ — `argv`

```
argv ---> +-------+
          |   o---+---> "./app\0"
          +-------+
          |   o---+---> "-v\0"
          +-------+
          | NULL  |     <- argv[argc] luôn là NULL (chuẩn bảo đảm)
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
|  0 |  1 |  2 |  3 | 10  | 11  | 12  | 13  | 20  | 21  | 22  | 23  |   48 byte LIỀN KHỐI
+----+----+----+----+-----+-----+-----+-----+-----+-----+-----+-----+
địa chỉ của a[i][j] = địa chỉ a + (i * C + j) * sizeof(int)
```

Output thật:

```
sizeof(a)=48 sizeof(a[0])=16
&a[1][0] - &a[0][0] = 4 phần tử (liền khối, row-major)
a[1][3] = 13 = *(*(a+1)+3) = 13
```

- `a` là mảng của **3 phần tử**, mỗi phần tử là một **hàng** `int[4]`. Nên `a` decay thành con trỏ tới **hàng đầu**, kiểu **`int (*)[4]`**, chứ không phải `int **`.
- `*(*(a+1)+3)` tính ra `a[1][3]` theo từng bước:

| Bước | Biểu thức | Kiểu | Ý nghĩa |
|---|---|---|---|
| 1 | `a` | `int (*)[4]` | trỏ tới hàng 0 |
| 2 | `a + 1` | `int (*)[4]` | nhảy **một hàng** = 16 byte ⟹ trỏ tới hàng 1 |
| 3 | `*(a + 1)` | `int[4]` → decay thành `int *` | chính là hàng 1; decay thành con trỏ tới `a[1][0]` |
| 4 | `*(a + 1) + 3` | `int *` | nhảy **3 phần tử** = 12 byte ⟹ trỏ tới `a[1][3]` |
| 5 | `*(*(a + 1) + 3)` | `int` | giá trị **13** |

- Ở bước 2, compiler cần biết **một hàng rộng bao nhiêu** — tức `C` (số cột). Nó **không** cần biết có bao nhiêu hàng. Vì thế khi truyền mảng 2 chiều vào hàm, **chỉ chiều đầu được bỏ trống**.

**Ba chữ ký hợp lệ cho hàm nhận `int a[3][4]`:**

```c
void f(int rows, int m[][4]);         /* số cột cố định lúc compile */
void f(int rows, int (*m)[4]);        /* y hệt dòng trên — viết rõ ràng */
void f(int rows, int cols, int m[rows][cols]);   /* C99 VLA: số cột lúc chạy */
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

**Vì sao crash:** với `m` kiểu `int **`, `m[1]` = `*(m + 1)` nghĩa là *"bỏ qua 1 con trỏ (8 byte), đọc 8 byte tiếp theo, coi đó là một **địa chỉ**"*. Nhưng vùng nhớ của `a` chỉ chứa **số `int` 4 byte**, không có con trỏ nào. 8 byte ở offset 8 chính là `a[0][2]` (= 2) và `a[0][3]` (= 3) ghép lại ⟹ một "địa chỉ" rác `0x0000000300000002`. `m[1][3]` dereference địa chỉ đó ⟹ segfault. Đây là chỗ gcc chỉ **cảnh báo** (C cho phép đổi kiểu con trỏ ngầm), nên **luôn build với `-Wall -Werror`** hoặc ít nhất đọc warning.

### 5.3 Cấp phát động — hai cách

**Cách 1 — mảng con trỏ hàng (`int **`)**: `R + 1` lần `malloc`.

```c
int **alloc_rows(int R, int C) {
    int **m = malloc(R * sizeof *m);           /* mảng R con trỏ */
    if (!m) return NULL;
    for (int i = 0; i < R; i++) {
        m[i] = malloc(C * sizeof **m);         /* mỗi hàng một vùng riêng */
        if (!m[i]) {                           /* thất bại giữa chừng: dọn phần đã cấp */
            while (i--) free(m[i]);
            free(m);
            return NULL;
        }
    }
    return m;
}
void free_rows(int **m, int R) {
    for (int i = 0; i < R; i++) free(m[i]);    /* hàng trước... */
    free(m);                                    /* ...mảng con trỏ sau */
}
```

**Cách 2 — một khối liền, vẫn viết `m[i][j]`**: một lần `malloc`.

```c
int (*m)[C] = malloc(R * sizeof *m);    /* sizeof *m = C * sizeof(int) */
m[2][3] = 23;
free(m);                                /* MỘT lần free */
```

Output thật (cùng `R=3, C=4`):

```
cách 1: m1[2][3]=23, hàng 1 cách hàng 0: 32 byte     <- KHÔNG liền khối (16 byte dữ liệu + metadata của malloc)
cách 2: m2[2][3]=23, hàng 1 cách hàng 0: 16 byte, sizeof *m2=16
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
char  s1[] = "abc";     /* MẢNG 4 byte (gồm '\0') trên stack, COPY từ literal */
char *s2   = "abc";     /* CON TRỎ tới literal nằm ở vùng CHỈ ĐỌC (.rodata) */
```

```
stack:   s1 [a][b][c][\0]          s2 [0x4010]--+
                                                |
.rodata:                       0x4010 [a][b][c][\0]   <- chỉ đọc
```

Output thật:

```
sizeof(s1)=4 sizeof(s2)=8
s1 = Xbc                                    <- s1[0] = 'X' OK
Segmentation fault (core dumped)            <- s2[0] = 'X' ghi vào vùng chỉ đọc, exit 139
```

> Kiểu đúng của con trỏ tới literal là **`const char *`**. Viết `const` thì `s2[0] = 'X'` thành **lỗi compile** thay vì crash lúc chạy. (Trong C++ literal có kiểu `const char[4]` và chuẩn C++11 **cấm** gán vào `char *`, nhưng g++ 11 vẫn chỉ cảnh báo: `ISO C++ forbids converting a string constant to 'char*' [-Wwrite-strings]`.)

### 6.2 Mảng con trỏ chuỗi vs mảng 2 chiều `char`

```c
char *names[]   = {"go", "rust", "c"};   /* 3 con trỏ, chuỗi nằm ở .rodata */
char  grid[][10] = {"go", "rust", "c"};  /* 3 x 10 byte liền khối, sửa được */
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
    return buf;                 /* buf chết khi hàm return */
}
```

```
warning: function returns address of local variable [-Wreturn-local-addr]
```

Trên máy này, **gcc 11.4 cố ý trả về `NULL`** thay cho địa chỉ đó (đã in thử: `con trỏ trả về = (nil)` ở cả `-O0` lẫn `-O2`), nên `printf("%s")` segfault ngay. Compiler khác, hoặc khi gcc không chứng minh được, sẽ trả về địa chỉ stack thật: đọc ra **rác**, hoặc tệ hơn là *"chạy đúng"* lúc test (xem [CPP-060](../14-prep/mock-interview/bank/cpp.md)). Đây là **UB** ở mọi trường hợp.

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
static int (*ops[])(int, int) = { op_add, op_sub };   /* mảng con trỏ hàm */

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
    return (x > y) - (x < y);                 /* -1, 0, 1 — không tràn số */
}
```

Sắp `{INT_MIN, 1, INT_MAX, 0, -5}`, output thật:

```
cmp_bad: 1 2147483647 -2147483648 -5 0         <- SAI thứ tự
cmp_ok : -2147483648 -5 0 1 2147483647
```

`(x > y) - (x < y)`: mỗi phép so sánh cho ra `1` (đúng) hoặc `0` (sai), nên hiệu của chúng chỉ có thể là `1`, `0` hoặc `-1` — không bao giờ tràn. Output thật với `x = 3, y = 7`: `(x>y)=0 (x<y)=1 -> -1`.

Còn `a - b` **tràn số** khi hai giá trị trái dấu và xa nhau (`1 - INT_MIN`). Tràn số có dấu là **UB**; thực tế nó quấn vòng thành số âm, comparator nói ngược, và `qsort` cho kết quả sai mà không báo gì.

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
