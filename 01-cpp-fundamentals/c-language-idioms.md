# C — Preprocessor, linkage, byte & các idiom phải viết được trên giấy

> **TL;DR**
> - **Macro là thay chữ, không phải hàm.** Ba bẫy kinh điển: thiếu ngoặc (`SQ(a+1)` ra 5 thay vì 9), tham số **đánh giá hai lần** (`MAX(x++, y)` tăng `x` hai lần), macro nhiều câu lệnh vỡ `if/else` ⟹ bọc bằng **`do { … } while (0)`**. Có lựa chọn thì dùng **`static inline`**.
> - **Khai báo ≠ định nghĩa.** Header chứa `extern int g;` (khai báo), **đúng một** `.c` chứa `int g = 0;` (định nghĩa). Từ gcc 10, hai `.c` cùng viết `int g;` là **lỗi link**.
> - `static` có **hai nghĩa**: biến cục bộ `static` sống suốt chương trình và khởi tạo **một lần**; `static` ở phạm vi file nghĩa là **chỉ file này thấy**.
> - Dữ liệu ra dây/ra file: ghép byte bằng **phép dịch**, không ép kiểu con trỏ, không dựa vào `union` — đúng trên mọi CPU, mọi endianness.
> - Xử lý lỗi trong C (và trong kernel): **mã trả về âm + `goto` tới nhãn dọn dẹp theo thứ tự ngược**.
> - Mọi output là **output thật** (gcc 11.4, x86-64 little-endian, `-std=c11 -Wall -Wextra`).
>
> Đọc sau [c-pointers-arrays.md](c-pointers-arrays.md). Phần bit-manipulation viết tay nằm ở [08/bare-metal-c §2](../08-embedded-systems/bare-metal-c.md) — không chép lại ở đây.

---

## 1. Preprocessor & macro

Preprocessor chạy **trước** compiler, và chỉ làm việc với **chữ**: nó thay tên macro bằng phần thân, không biết kiểu, không biết thứ tự ưu tiên toán tử, không biết biểu thức có tác dụng phụ. Mọi bẫy của macro đều từ đó mà ra. Xem kết quả sau tiền xử lý bằng `gcc -E file.c`.

### 1.1 Ba bẫy kinh điển

```c
#define SQ_BAD(x)     x * x
#define SQ(x)         ((x) * (x))       /* ngoac quanh TUNG tham so + ca bieu thuc */
#define DOUBLE_BAD(x) (x) + (x)
#define MAX(a, b)     ((a) > (b) ? (a) : (b))
```

| Lời gọi | Sau khi thay chữ | Output thật | Lỗi |
|---|---|---|---|
| `SQ_BAD(a + 1)`, `a = 2` | `a + 1 * a + 1` | **5** (mong 9) | Thiếu ngoặc quanh **tham số** |
| `10 / SQ_BAD(2)` | `10 / 2 * 2` | **10** (mong 2) | Thiếu ngoặc quanh **cả biểu thức** |
| `2 * DOUBLE_BAD(3)` | `2 * (3) + (3)` | **9** (mong 12) | Như trên |
| `MAX(x++, y)`, `x = 5, y = 3` | `((x++) > (y) ? (x++) : (y))` | **6**, và `x` thành **7** | Tham số **đánh giá hai lần** |

- Ngoặc chữa được hai bẫy đầu, **không** chữa được bẫy thứ ba: không cách viết macro nào ngăn được việc một tham số xuất hiện hai lần trong thân.
- Tệ hơn: `SQ(i++)` thành `((i++) * (i++))` — hai lần sửa `i` không có điểm tuần tự xen giữa là **UB**, không chỉ là "tăng hai lần".

**Macro hay hàm `static inline`:**

| | Macro | `static inline` |
|---|---|---|
| Kiểm tra kiểu | ❌ | ✅ |
| Tham số đánh giá | Có thể **nhiều lần** | **Đúng một lần** |
| Debug, đặt breakpoint | ❌ không còn tồn tại sau tiền xử lý | ✅ |
| Chi phí gọi hàm | 0 | Thường 0 (compiler inline) |
| **Chỉ macro làm được** | `#`, `##`, `__FILE__`/`__LINE__` tại chỗ gọi, sinh code (X-macro), dùng được trong `#if`, hằng số cho kích thước mảng/`case` | — |

⟹ **Mặc định dùng `static inline`; chỉ dùng macro cho đúng những việc ở dòng cuối.**

### 1.2 `do { … } while (0)` — vì sao macro nhiều câu lệnh phải bọc như vậy

```c
#define RESET_AND_LOG()  hw_reset(); log_evt()                      /* V1 */
#define RESET_AND_LOG()  { hw_reset(); log_evt(); }                 /* V2 */
#define RESET_AND_LOG()  do { hw_reset(); log_evt(); } while (0)    /* V3 */

if (fault)
    RESET_AND_LOG();
else
    printf("ok\n");
```

| Cách | Kết quả thật | Vì sao |
|---|---|---|
| V1 | `error: 'else' without a previous 'if'` | `if` chỉ ôm `hw_reset();`, `log_evt();` đứng riêng, `else` mồ côi |
| V2 | `error: 'else' without a previous 'if'` | `{ … }` cộng với dấu `;` người dùng gõ thêm thành `{ … };` — dấu `;` đó là một câu lệnh rỗng chen giữa `if` và `else` |
| **V3** | ✅ in `ok` | `do { … } while (0)` là **một câu lệnh** và **cần đúng một** dấu `;` — dùng y như một lời gọi hàm |

**Bản tệ nhất là V1 khi không có `else`:** vẫn compile, và `log_evt()` **luôn chạy** dù `fault = 0`. Output thật: `fault=0 nhung: n_reset=0 n_log=1`. gcc có bắt: `warning: macro expands to multiple statements [-Wmultistatement-macros]` — thêm một lý do để đọc warning.

### 1.3 `#` và `##`

```c
#define STR(x)    #x             /* bien token thanh chuoi */
#define XSTR(x)   STR(x)         /* mo rong x TRUOC, roi moi bien thanh chuoi */
#define CAT(a, b) a##b           /* dan hai token thanh mot */
#define FW_VERSION 3
```

Output thật:

```
STR(FW_VERSION)  = FW_VERSION      <- # dung tham so NGUYEN VAN, khong mo rong
XSTR(FW_VERSION) = 3               <- qua mot tang trung gian thi FW_VERSION duoc mo rong truoc
reg_7 = 42                         <- int CAT(reg_, 7) = 42;  ==>  int reg_7 = 42;
```

Luật: tham số đứng cạnh `#` hoặc `##` thì **không** được mở rộng trước. Muốn mở rộng thì đi qua một macro trung gian (`XSTR`). Thực tế: in phiên bản firmware dạng chuỗi, sinh tên thanh ghi/hàm theo số hiệu.

### 1.4 X-macro — một danh sách, nhiều thứ được sinh ra

Bài toán: một `enum` mã lỗi và một bảng chuỗi mô tả phải **luôn khớp nhau**. Viết tay hai nơi thì sớm muộn sẽ lệch.

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

```
0 -> ok
1 -> timeout
2 -> i2c nack
```

Thêm một mã lỗi = thêm **một dòng** trong `ERROR_LIST`; `enum`, bảng chuỗi và `ERR_COUNT` tự cập nhật. Đây đúng là nguyên tắc *"một sự thật, một chỗ"*. Cái giá: khó đọc với người chưa quen, và lỗi biên dịch chỉ vào chỗ mở rộng macro chứ không chỉ vào dòng bạn viết.

### 1.5 Biên dịch có điều kiện & include guard

```c
#ifndef COUNTER_H          /* include guard: header bi include nhieu lan van chi co hieu luc mot lan */
#define COUNTER_H
/* ... */
#endif

#if defined(CONFIG_CHIP_A)
#  include "chip_a_regs.h"
#elif defined(CONFIG_CHIP_B)
#  include "chip_b_regs.h"
#else
#  error "Chua chon chip"   /* that bai NGAY luc build, khong de lot toi runtime */
#endif
```

`#pragma once` ngắn hơn và gần như mọi compiler hỗ trợ, nhưng không có trong chuẩn ([BLD — include guard](../14-prep/mock-interview/bank/build-systems.md)). `#error` biến cấu hình thiếu thành **lỗi build** — rẻ hơn mọi bug runtime.

---

## 2. Linkage & vòng đời biến

> Phần định nghĩa ngắn của `static`/`extern`/`const`/`volatile` ở [EMB-004](../14-prep/mock-interview/bank/embedded-fundamentals.md) và [bare-metal-c §4](../08-embedded-systems/bare-metal-c.md). Mục này đi vào **hệ quả khi build nhiều file** — chỗ thực sự sinh bug.

### 2.1 Hai trục: vòng đời (storage duration) và ai thấy được (linkage)

| Khai báo | Sống bao lâu | Ai thấy | Nằm ở |
|---|---|---|---|
| Biến cục bộ thường | Trong lời gọi hàm | Chỉ khối đó | Stack |
| **`static` cục bộ** | **Suốt chương trình**, khởi tạo **một lần** | Chỉ hàm đó | `.data` / `.bss` |
| Biến toàn cục | Suốt chương trình | **Mọi file** (external linkage) | `.data` / `.bss` |
| **`static` toàn cục / hàm `static`** | Suốt chương trình | **Chỉ file đó** (internal linkage) | `.data` / `.bss` / `.text` |

⟹ Chữ `static` mang **hai nghĩa khác nhau** tuỳ vị trí: trong hàm thì đổi **vòng đời**, ngoài hàm thì đổi **phạm vi nhìn thấy**.

```c
int next_id(void) {
    static int id = 100;     /* khoi tao MOT lan, luc chuong trinh bat dau */
    return id++;
}
```

Gọi ba lần ở ba câu lệnh riêng: `next_id: 100 101 102`.

> ⚠️ **Bẫy phụ gặp ngay lúc chạy thử:** viết gọn `printf("%d %d %d", next_id(), next_id(), next_id());` thì gcc in **`102 101 100`** — thứ tự đánh giá đối số là *unspecified*, gcc chọn phải sang trái. Cùng bài học với [COD-026](../14-prep/mock-interview/bank/coding.md).

### 2.2 Khai báo vs định nghĩa — luật một định nghĩa

```c
/* counter.h */
extern int g_count;            /* KHAI BAO: "co mot bien ten nay o dau do" — khong cap bo nho */
int next_id(void);             /* khai bao ham */

/* a.c — DUNG MOT file */
int g_count = 0;               /* DINH NGHIA: cap bo nho */
```

**Hai file cùng viết `int g_count;` ở phạm vi file** (không `extern`, không khởi tạo — gọi là *tentative definition*):

```
gcc 11.4:            multiple definition of `g_count'      <- loi link
gcc 11.4 -fcommon:   link OK                               <- hanh vi cu
```

Từ **gcc 10**, mặc định là `-fno-common`: mỗi tentative definition là một định nghĩa thật, hai cái thì đụng nhau. Code cũ "chạy bao năm" có thể **không link được** khi nâng toolchain. Sửa đúng: một định nghĩa trong `.c`, `extern` trong header — không phải thêm `-fcommon`.

### 2.3 `static` trong header

```c
/* counter.h */
static int helper(void) { return 7; }
```

Mỗi `.c` include header này có **một bản riêng** của `helper` — đã kiểm: in địa chỉ `helper` từ hai file ra **2 địa chỉ khác nhau**. Với hàm nhỏ `static inline` thì đây là chủ ý (cách viết hàm tiện ích trong header của C). Với **biến** `static` trong header thì gần như luôn là bug: mỗi file sửa bản riêng của mình và tưởng là dùng chung.

### 2.4 `const` ở phạm vi file: C khác C++

| | C | C++ |
|---|---|---|
| `const int LIMIT = 10;` ở hai file | ❌ `multiple definition of 'LIMIT'` | ✅ link OK |
| Lý do | `const` toàn cục có **external** linkage | `const` toàn cục mặc định **internal** linkage |
| Cách đúng cho C | Header: `extern const int LIMIT;` + một `.c` định nghĩa; hoặc `#define` / `enum { LIMIT = 10 };` | Đặt thẳng trong header (hoặc `inline constexpr`) |

Cả hai dòng đều đã build thật bằng gcc/g++ 11.4. Đây là chỗ code C dán sang C++ (hoặc ngược lại) đổi hành vi link mà không đổi một chữ nào.

---

## 3. `union`, endianness & tuần tự hoá byte

### 3.1 Ghép byte bằng phép dịch — cách duy nhất không phụ thuộc CPU

```c
static void put_be32(uint8_t *p, uint32_t v) {          /* big-endian tren day */
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >>  8); p[3] = (uint8_t)(v);
}
static uint32_t get_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}
```

Output thật trên x86 (little-endian):

```
wire = 11 22 33 44
get_be32 = 0x11223344
union b[0..3] = 44 33 22 11            <- thu tu trong bo nho cua CPU nay
memcpy wire -> uint32 = 0x44332211     <- copy thang: SAI tren little-endian
```

| Cách đọc 4 byte từ buffer | Endianness | Alignment | Hợp lệ |
|---|---|---|---|
| **Phép dịch** (`get_be32`) | ✅ Tự xử lý | ✅ Đọc từng byte | ✅ C và C++ |
| `memcpy` vào `uint32_t` | ❌ Lấy theo CPU, phải tự đảo | ✅ | ✅ |
| Ép kiểu `*(uint32_t *)buf` | ❌ | ❌ **Fault** trên CPU không cho đọc lệch | ❌ Vi phạm strict aliasing — xem [COD-025](../14-prep/mock-interview/bank/coding.md) |

`(uint32_t)p[0] << 24` — **phải ép trước khi dịch**: `p[0]` là `uint8_t`, bị nâng lên `int`; dịch `0x80 << 24` trên `int` là tràn dấu ⟹ UB ([integer promotion](../08-embedded-systems/bare-metal-c.md)).

### 3.2 `union` — dùng làm gì, và type punning

`union` cho các thành viên **dùng chung một vùng nhớ**; kích thước bằng thành viên lớn nhất. Hai công dụng chính đáng: tiết kiệm bộ nhớ cho dữ liệu "một trong nhiều loại" (đi kèm một trường `type` cho biết đang dùng loại nào — *tagged union*), và xem cùng một vùng nhớ dưới hai kiểu.

| Đọc thành viên khác với thành viên vừa ghi (type punning) | |
|---|---|
| **C** (C99 TC3 trở đi) | ✅ Được phép — byte được diễn giải lại theo kiểu mới |
| **C++** | ❌ **UB** — dùng `memcpy`, hoặc `std::bit_cast` (C++20) |

Cách xem bit của `float` hợp lệ ở **cả hai** ngôn ngữ là `memcpy`: `bit cua 1.0f = 0x3F800000`.

> ⚠️ Map **thanh ghi** bằng `union`/bitfield là chuyện khác và bị nhiều coding standard cấm: chuẩn không quy định thứ tự bit trong bitfield — [EMB-003](../14-prep/mock-interview/bank/embedded-fundamentals.md).

---

## 4. Xử lý lỗi kiểu C — mã trả về & `goto cleanup`

C không có exception và không có destructor. Quy ước phổ biến nhất (cũng là quy ước của **Linux kernel**): hàm trả **`0` khi thành công, số âm `-ERRNO` khi lỗi**, và mọi tài nguyên đã giành được phải được trả lại **theo thứ tự ngược** khi lỗi xảy ra giữa chừng.

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
    return 0;                 /* thanh cong: KHONG roi xuong cac nhan ben duoi */

err_c:
    free(b);                  /* nhan sau don cai gianh truoc — thu tu NGUOC */
err_b:
    free(a);
err_a:
    return ret;
}
```

Chạy với lỗi giả lập ở từng bước:

```
loi o buoc 0 -> dev_init = 0
loi o buoc 1 -> dev_init = -12          <- -ENOMEM
loi o buoc 2 -> dev_init = -12
loi o buoc 3 -> dev_init = -12
All heap blocks were freed -- no leaks are possible       <- valgrind, ca 4 kich ban
```

| Cách | Vấn đề |
|---|---|
| `if` lồng nhau | Thụt lề sâu dần theo số tài nguyên, phần chính bị đẩy sang phải |
| Lặp lại `free(...)` ở mỗi nhánh lỗi | Thêm tài nguyên phải sửa N chỗ — chỗ dễ quên nhất |
| ✅ **`goto` tới nhãn dọn dẹp** | **Một** đường ra; thêm tài nguyên = thêm một cặp `if/goto` + một nhãn |

- `goto` ở đây chỉ nhảy **xuôi** tới phần dọn dẹp — không phải kiểu `goto` làm rối luồng mà người ta khuyên tránh. Nó là **RAII viết tay**: trong C++ destructor làm đúng việc này tự động.
- Trong C, `goto` nhảy qua khai báo biến thường thì hợp lệ (chỉ cấm nhảy vào phạm vi của VLA). Trong C++ nhảy qua một khởi tạo là **lỗi compile** — đã thử đúng file trên với g++: `error: jump to label 'err_b'` / `note: crosses initialization of 'void* c'`. Lý do kernel style không dán thẳng sang C++ được (C++ có RAII nên cũng không cần).
- **`errno`**: hàm thư viện chuẩn báo lỗi bằng giá trị trả về đặc biệt (`-1`, `NULL`) **và** ghi chi tiết vào `errno`. Chỉ đọc `errno` **ngay sau** khi hàm báo lỗi — hàm thành công không có nghĩa vụ xoá nó, và lời gọi kế tiếp có thể ghi đè ([LNX](../14-prep/mock-interview/bank/linux-sysprog.md)).

---

## 5. Tự viết hàm chuẩn trên giấy

Interviewer không cần bạn thuộc thư viện. Họ xem bạn **tự hỏi về ca biên** trước khi viết. `memcpy`/`strlen`/`memmove` có sẵn ở [COD-005](../14-prep/mock-interview/bank/coding.md), [COD-016](../14-prep/mock-interview/bank/coding.md); mục này bổ sung ba hàm còn lại hay gặp.

### 5.1 `atoi` — ca biên quyết định điểm

```c
bool my_atoi(const char *s, int *out) {
    while (isspace((unsigned char)*s)) s++;                 /* 1. bo khoang trang dau */
    int sign = 1;
    if (*s == '+' || *s == '-') { if (*s == '-') sign = -1; s++; }   /* 2. dau */
    if (!isdigit((unsigned char)*s)) return false;          /* 3. khong co chu so nao */
    long long acc = 0;
    while (isdigit((unsigned char)*s)) {
        acc = acc * 10 + (*s - '0');
        if (sign * acc > INT_MAX || sign * acc < INT_MIN) return false;   /* 4. tran so */
        s++;
    }
    *out = (int)(sign * acc);                                /* 5. dung o ky tu khong phai so */
    return true;
}
```

| Input | Output thật |
|---|---|
| `"  42"` | ok 42 |
| `"-17abc"` | ok −17 (dừng ở `a`) |
| `"2147483647"` | ok 2147483647 |
| `"2147483648"` | **LỖI** (tràn) |
| `"-2147483648"` | ok −2147483648 (`INT_MIN` hợp lệ, dù `-INT_MIN` thì không) |
| `"abc"`, `""` | **LỖI** |

So với `atoi` chuẩn: `atoi("2147483648")` in ra `-2147483648` — tràn số trong `atoi` là **UB** và **không có cách nào báo lỗi**. Code thật dùng `strtol` (trả `errno = ERANGE`, cho con trỏ tới ký tự dừng).

> `(unsigned char)*s` khi gọi `isdigit`/`isspace`: truyền `char` âm (byte ≥ 0x80 trên nền `char` có dấu) vào các hàm `<ctype.h>` là UB.

### 5.2 `itoa` — bẫy nằm ở `INT_MIN`

```c
char *my_itoa(int v, char *buf, size_t n) {
    char tmp[12];                 /* "-2147483648" + '\0' */
    int  i = 0;
    bool neg = v < 0;
    if (!neg) v = -v;             /* dua ve SO AM: -INT_MAX bieu dien duoc, -INT_MIN thi KHONG */
    do { tmp[i++] = (char)('0' - v % 10); v /= 10; } while (v);   /* do-while: v = 0 van in "0" */
    if (neg) tmp[i++] = '-';
    if ((size_t)i + 1 > n) return NULL;
    for (int k = 0; k < i; k++) buf[k] = tmp[i - 1 - k];          /* chu so sinh nguoc — dao lai */
    buf[i] = '\0';
    return buf;
}
```

Output thật: `0 -305 -2147483648`. Cách "đổi sang dương rồi xử lý" (`if (v < 0) v = -v;`) **hỏng đúng ở `INT_MIN`**: `-INT_MIN` tràn. Làm việc trên **số âm** tránh được, vì miền âm của `int` rộng hơn miền dương một giá trị. (`v % 10` với `v` âm cho kết quả âm từ C99, nên `'0' - v % 10` ra đúng chữ số.)

### 5.3 `strncpy` không phải "`strcpy` an toàn"

```c
char dst[4];
strncpy(dst, "abcdef", sizeof dst);    /* nguon dai hon -> KHONG co '\0' */
```

```
warning: 'strncpy' output truncated copying 4 bytes from a string of length 6 [-Wstringop-truncation]
strncpy: dst[3]='d' (khong phai '\0')
```

`strncpy` chép **tối đa n byte** và chỉ thêm `'\0'` nếu nguồn **ngắn hơn** n. Nguồn dài hơn ⟹ `dst` không còn là chuỗi, `printf("%s")` đọc tràn. (Nó còn **đệm `'\0'` tới hết n** khi nguồn ngắn — tốn công nếu buffer lớn.)

**Chép chuỗi có giới hạn, an toàn:** `snprintf(dst, sizeof dst, "%s", src);` — luôn kết thúc `'\0'`, cắt bớt nếu thiếu chỗ, và **trả về độ dài lẽ ra cần** để biết đã bị cắt. Output thật: `"abc"`. (`strlcpy` làm cùng việc, có trên BSD/glibc ≥ 2.38 nhưng không phải chuẩn C.)

---

## 6. Bẫy khi đọc code — "đoạn này in ra gì?"

Dạng câu hỏi đọc code trên giấy kiểm tra một việc: bạn đọc theo **luật của ngôn ngữ**, hay theo **ý định của người viết**. Mọi bẫy dưới đây đều compile được, và **gần như cái nào gcc cũng cảnh báo** — nên câu trả lời mạnh luôn kết thúc bằng *"và `-Wall -Wextra` bắt được nó bằng cờ…"*. Bảng chỉ để ôn nhanh; đáp án đầy đủ ở bank mục N.

### 6.1 Thứ tự ưu tiên & cú pháp "im lặng"

| Code | Đọc theo ý định | Đọc theo luật — output thật | gcc bắt bằng |
|---|---|---|---|
| `if (status & READY == 0)` | Bit READY tắt? | `==` **mạnh hơn** `&` ⟹ `status & (READY == 0)` = `status & 0` = 0 ⟹ luôn vào `else`: in `san sang -> gui lenh` khi bit đang **tắt** | `-Wparentheses` |
| `1 << 2 + 1` | 5 | `+` mạnh hơn `<<` ⟹ `1 << 3` = **8** | `-Wparentheses` |
| `if (x = 0)` | So sánh | **Gán** 0 rồi xét ⟹ luôn sai, và `x` bị đổi thành 0 | `-Wparentheses` |
| `if (a) if (b) f(); else g();` (thụt lề như `else` của `if (a)`) | `else` của `if (a)` | `else` gắn với `if` **gần nhất** ⟹ `a = 0` thì **không in gì** | `-Wdangling-else` |
| `case` không có `break` | Chỉ chạy một nhánh | **Rơi xuống** các `case` sau: `mode = 1` cho `power = 100` thay vì 50 | `-Wimplicit-fallthrough` |
| `for (unsigned i = 3; i >= 0; i--)` | 3, 2, 1, 0 | Không bao giờ `< 0` ⟹ `3 2 1 0 4294967295 4294967294 …` vô tận | `-Wtype-limits` |

Cố ý rơi xuống `case` sau thì ghi rõ bằng comment `/* fall through */` hoặc `__attribute__((fallthrough));` — cả người đọc lẫn compiler đều hiểu là chủ ý.

### 6.2 Kiểu & chuyển đổi ngầm

| Code | Output thật | Vì sao |
|---|---|---|
| `if (sizeof(int) > -1)` | `KHONG lon hon` | `sizeof` là `size_t` (không dấu) ⟹ `-1` đổi sang không dấu = số cực lớn. `-Wsign-compare` |
| `char c = 200; if (c > 100)` | x86: `c <= 100, c = -56` · `-funsigned-char`: `c > 100, c = 200` | `char` **có dấu hay không là do nền tảng quyết định**: x86 có dấu, ARM (theo ABI) không dấu. gcc **không cảnh báo** dòng gán. Byte đọc từ thiết bị thì dùng `uint8_t` |
| `float f = 0.1f; if (f == 0.1)` | `KHONG bang: f=0.1000000015` | `0.1` là `double`; `0.1f` làm tròn ở độ chính xác thấp hơn ⟹ hai giá trị khác nhau |
| Cộng `0.1f` mười lần rồi `== 1.0f` | `0` (sum = `1.00000012`) | Sai số làm tròn tích luỹ. So sánh bằng ngưỡng: `fabsf(a - b) < eps`. `-Wfloat-equal` (không có trong `-Wall -Wextra`) |

### 6.3 UB trông như có output

```c
int x = 5;
int y = x++ + ++x;          /* "x va y bang bao nhieu?" */
```

gcc in `x=7 y=12` ở cả `-O0` lẫn `-O2` — trông **ổn định**, nhưng đây là **UB**: `x` bị sửa hai lần không có điểm tuần tự xen giữa. gcc cảnh báo đúng chỗ: `operation on 'x' may be undefined [-Wsequence-point]`. Câu trả lời đúng ở phỏng vấn **không phải là một con số**: *"Đây là UB, compiler nào in gì cũng đúng; sửa bằng cách tách thành nhiều câu lệnh."*

### 6.4 Bộ nhớ

| Code | Chuyện gì xảy ra — output thật |
|---|---|
| `buf = realloc(buf, n);` | `realloc` thất bại trả `NULL` **và vùng cũ vẫn còn** ⟹ gán thẳng làm mất con trỏ duy nhất. Đã chạy: `buf sau = (nil)` · valgrind: `definitely lost: 64 bytes`. Đúng: `tmp = realloc(buf, n); if (!tmp) { /* xu ly, buf van hop le */ } else buf = tmp;` |
| `memcmp(&a, &b, sizeof a)` với struct có padding | Hai struct có **mọi field bằng nhau** vẫn ra `KHAC` — padding chứa byte rác khác nhau (`sizeof(struct { uint8_t; uint32_t; })` = 8). So sánh **từng field**, hoặc `memset` cả struct về 0 trước khi điền |
| `char name[5]; strcpy(name, "hello");` | Ghi 6 byte vào 5. Build thường **chạy như không có gì** (in `name = hello`, exit 0); gcc có cảnh báo `-Wstringop-overflow`; **ASan** bắt tại chỗ: `stack-buffer-overflow … WRITE of size 6` · `'name' (line 4) <== Memory access at offset 37 overflows this variable` ([09/memory-bugs](../09-debugging/memory-bugs.md)) |

### 6.5 `container_of` — từ con trỏ tới thành viên, lấy lại cả struct

Kernel không dùng danh sách liên kết chứa dữ liệu; nó **nhúng** một nút danh sách vào trong struct dữ liệu, rồi từ con trỏ tới nút tính ngược ra struct chứa nó.

```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct sensor { int id; char name[8]; struct list_node node; };

struct list_node *n = &s.node;                              /* danh sach chi giu node */
struct sensor *back = container_of(n, struct sensor, node); /* lui lai offsetof byte */
```

```
offsetof(node) = 16
back == &s ? 1   id=7 name=temp
```

- **Ép sang `char *` trước khi trừ** — để phép trừ tính bằng **byte**. Trừ trên `struct list_node *` thì trừ theo bội số `sizeof(struct list_node)`.
- Một struct có thể chứa **nhiều** nút (nằm trong nhiều danh sách cùng lúc) — đó là lý do kernel chọn cách nhúng thay vì danh sách chứa con trỏ dữ liệu.
- Bản trong kernel thêm kiểm tra kiểu lúc biên dịch để bắt lỗi truyền nhầm `member`.

---

## 7. Nhận diện — biết tên là đủ

| Từ khoá / kỹ thuật | Là gì | Một câu để nói |
|---|---|---|
| `restrict` (C99) | Hứa với compiler: vùng nhớ con trỏ này trỏ tới **không bị con trỏ khác truy cập** trong phạm vi đó | Cho phép tối ưu mạnh hơn; vi phạm lời hứa là UB. Chữ ký `memcpy(void *restrict, const void *restrict, size_t)` ghi thẳng điều kiện "hai vùng không chồng lấn" vào kiểu — chồng lấn thì dùng `memmove` |
| Flexible array member | `struct packet { uint16_t len; uint8_t data[]; };` — mảng không kích thước, **phải ở cuối** struct | Cấp **một lần** cho header + payload: `malloc(sizeof *p + n)`. Đã chạy: `sizeof(struct packet)=2` — `data[]` không tính. Kernel dùng khắp nơi |
| `inline` theo C99 | Khác C++: `inline` không kèm `static` ở C đòi một định nghĩa ngoài ở đâu đó | Trong header C, viết **`static inline`** để khỏi bận tâm |
| `setjmp` / `longjmp` | Nhảy ngược về một điểm đã lưu, xuyên qua nhiều tầng hàm | "Exception" của C; bỏ qua mọi dọn dẹp ở giữa, cấm dùng trong code C++ có destructor |
| `_Static_assert` (C11) | Kiểm điều kiện **lúc biên dịch** | Kiểm `sizeof` struct giao thức, kiểm bảng khớp `enum` (§1.4) |
| Hàm biến số đối số (`...`, `va_list`) | Kiểu `printf`: `va_start` → `vsnprintf(buf, n, fmt, ap)` → `va_end` | Hàm **không biết** số và kiểu đối số — chỉ biết qua format. Thêm `__attribute__((format(printf, 2, 3)))` để gcc kiểm như `printf` thật: đã chạy, truyền chuỗi cho `%d` ⟹ `warning: format '%d' expects argument of type 'int', but argument 3 has type 'char *'`. Đây là cách viết wrapper log trong firmware |

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở. Domain: [bank/c-programming.md](../14-prep/mock-interview/bank/c-programming.md) mục H–N.

| ID | Câu hỏi |
|----|---------|
| [C-021](../14-prep/mock-interview/bank/c-programming.md) ⭐ | `SQ(a+1)`, `10/SQ(2)` với macro thiếu ngoặc — in ra gì? |
| [C-022](../14-prep/mock-interview/bank/c-programming.md) | `MAX(x++, y)` — `x` tăng mấy lần? Macro hay `static inline`? |
| [C-023](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Vì sao macro nhiều câu lệnh phải bọc `do { } while (0)`? |
| [C-024](../14-prep/mock-interview/bank/c-programming.md) | `#`, `##` và vì sao cần macro trung gian `XSTR` |
| [C-025](../14-prep/mock-interview/bank/c-programming.md) | Giữ `enum` mã lỗi và bảng chuỗi luôn khớp — X-macro |
| [C-026…029](../14-prep/mock-interview/bank/c-programming.md) | Bài bit viết tay — tài liệu ở [08/bare-metal-c §2.1](../08-embedded-systems/bare-metal-c.md) |
| [C-030](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Viết `atoi` — các ca biên |
| [C-031](../14-prep/mock-interview/bank/c-programming.md) | Viết `itoa` — vì sao `INT_MIN` là bẫy |
| [C-032](../14-prep/mock-interview/bank/c-programming.md) | `strncpy` có phải `strcpy` an toàn không? |
| [C-033](../14-prep/mock-interview/bank/c-programming.md) | `union` và type punning — C khác C++ thế nào? |
| [C-034](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Đọc một `uint32_t` big-endian từ buffer nhận được |
| [C-035](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Hai nghĩa của `static`; `static` trong header |
| [C-036](../14-prep/mock-interview/bank/c-programming.md) | `int g;` ở hai file — vì sao nâng gcc lên 10 thì không link được? |
| [C-037](../14-prep/mock-interview/bank/c-programming.md) ⭐ | Khởi tạo ba tài nguyên, lỗi ở bước thứ hai — `goto cleanup` |
| [C-038…051](../14-prep/mock-interview/bank/c-programming.md) ⭐ | 🔎 **Đọc code** — đoạn này in ra gì / sai ở đâu (bank mục N, theo §6) |
| [EMB-004](../14-prep/mock-interview/bank/embedded-fundamentals.md) | Vai trò `static` / `const` / `volatile` / `extern` (mức định nghĩa) |

---
⬅️ [c-pointers-arrays.md](c-pointers-arrays.md) · [Về index topic](README.md) · ➡️ Tiếp theo: [oop.md](oop.md)
