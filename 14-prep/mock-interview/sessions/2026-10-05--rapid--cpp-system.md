# Phiên mock — 2026-10-05 · `rapid` · cpp-system

- **Level:** mid-level · **Số câu:** 12 (11 chấm, 1 bỏ qua theo yêu cầu ứng viên) · **Trần:** T2 (rapid ⇒ thực tế T1) · **Thang:** [rapid](../config.md) — đo độ trôi chảy T1
- **Điểm:** **26/44 = 2.36**
- **Bối cảnh:** phiên đầu tiên sau khi sprint Datalogic kết thúc (03/10), không có plan JD. Ôn tự do. Phiên đầu tiên chạm domain **`C`** (thêm 03/10, trước phiên phủ 0/51).
- **Chọn câu:** 4 câu weak (`CPP-020`, `CPP-045`, `DP-002` đều **Ⓖ bắt buộc**; `CPP-067` tung đồng xu ra Ⓖ nhưng **không hỏi** vì là câu 🟠 design, không hợp rapid) + 8 câu mới, nghiêng về `C` (0%) và `DP` (11%). Rapid không có suất retention ⇒ **8 câu retention vẫn quá hạn** (hạn 18–23/09).

---

## Kết quả từng câu

| # | ID | Dạng | Câu | Điểm | Loại lỗ hổng |
|---|---|:-:|---|:-:|---|
| 1 | `C-001` | Ⓖ | bước nhảy con trỏ, `a[i]` | **4** | — |
| 2 | `C-003` | Ⓖ | chưa khởi tạo / NULL / dangling | **2** | DIỄN ĐẠT |
| 3 | `CPP-020` 🔁 | Ⓖ | Rule of 0/3/5 | **3** | — |
| 4 | `DP-005` | Ⓖ | Strategy | **2** | KIẾN THỨC |
| 5 | `C-035` | Ⓖ | `static` trong C | **1** | KIẾN THỨC |
| 6 | `CPP-045` 🔁 | Ⓖ | `= delete` vs private cũ | **2** | KIẾN THỨC *(bank sai, xem câu 6)* |
| 7 | `DP-006` | Ⓖ | Observer | — | bỏ qua, không chấm |
| 8 | `C-020` | Ⓖ | `void *`, ép `malloc` | **2** | KIẾN THỨC |
| 9 | `OS-022` | Ⓖ | biến toàn cục: thread vs process | **3** | — |
| 10 | `CPP-057` | Ⓖ | `std::thread` joinable khi huỷ, `jthread` | **3** | — |
| 11 | `C-032` | Ⓖ | `strncpy(dst, src, sizeof dst)` | **2** | KIẾN THỨC |
| 12 | `DP-002` 🔁 | Ⓖ | Singleton | **2** | KIẾN THỨC |

🔁 = câu từ weak-register.

### 🔎 Một chẩn đoán xuyên suốt

Ba câu `C` mới (5, 8, 11) có **cùng một kiểu sai**: phần **định nghĩa** đúng, phần **hệ quả cơ học** thì đoán. *"static trong header ⇒ lỗi linker"*, *"strncpy vẫn ghi tràn"*, *"ép malloc chỉ là thừa"*. Đó là những thứ chỉ cần **chạy thử 30 giây** là biết. Đúng bệnh *"biết nhưng chưa dùng công cụ"* đã đo ngày 17/08 (T1 3.67 / T2 2.1). Cách chữa không phải đọc thêm: **gặp khẳng định nào về C thì gõ ra chạy thử**.

### ⚖️ Lỗi phía tài liệu / interviewer — ghi rõ để không đổ cho người học

1. **Bank `CPP-045` sai** (và bản tóm tắt EMC Item 11 cũng sai): cả hai đảo chiều compile/link. Ứng viên đảo đúng chiều đó **3 phiên liên tiếp**. Đã sửa 05/10.
2. **Câu 12, probe leo tầng trong rapid:** interviewer hỏi ca `.so` cùng process (T2), vi phạm *"rapid không leo tầng"*. Không tính vào điểm.
3. **Bank `DP-005`, `DP-006`, `C-020` quá mỏng** (đoạn khẳng định không ví dụ) ⇒ ứng viên phản hồi đúng. Đã nâng cấp cả ba.

---

## 🔎 Chi tiết từng câu

### Câu 1 · `C-001` · Ⓖ · 🟢 · **4/4**

**`int *p` và `char *c` cùng trỏ vào một địa chỉ. `p + 1` và `c + 1` cách địa chỉ ban đầu bao nhiêu byte? Và `a[i]` thực chất là gì?**

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** `p+1` cách 4 byte vì kiểu `int`, `c+1` cách 1 byte vì kiểu `char`. `a[i]` thực chất là `*(a+i)`. ✅✅

**Nhận xét:** gọn, chính xác, bật ra ngay, đúng tiêu chí 4 của thang rapid. Nói *"`sizeof(int)` byte, thường là 4"* thì chặt hơn nữa, nhưng không trừ điểm.

**Đáp án ([bank C-001](../bank/c-programming.md)):** `p + n` nhảy `n * sizeof(*p)` byte. `a[i]` được định nghĩa là `*(a + i)`. Phép cộng giao hoán nên `2[a]` cũng hợp lệ.
</details>

---

### Câu 2 · `C-003` · Ⓖ · 🟢 · **2/4** · DIỄN ĐẠT

**Con trỏ chưa khởi tạo, con trỏ `NULL`, con trỏ treo (dangling) — khác nhau ra sao?**

**🔁 Probe:** *"Con trỏ chưa khởi tạo thực ra đang **chứa giá trị gì**? Và `if (p != NULL)` có bắt được nó không?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- Chưa khởi tạo: *"là con trỏ chưa trỏ đến bất kỳ địa chỉ nào"* ❌
- NULL: được gán giá trị xác nhận không trỏ đi đâu, an toàn khi `delete`, kiểm được ✅
- Dangling: trỏ vào vùng đã giải phóng, truy cập là UB ✅
- *Probe:* *"không, nó có thể chứa giá trị rác, trỏ tới địa chỉ không biết trước, thoát khỏi check NULL, gây UB"* ✅

**✅ Được:** NULL và dangling chuẩn. Sau probe ra đủ ý chính.

**❌ Vì sao 2:** câu đầu tiên mô tả con trỏ chưa khởi tạo bằng **chính định nghĩa của NULL** ("không trỏ đâu"). Đó là chỗ phân biệt cốt lõi của câu hỏi. Thang rapid: *"phải gợi mới ra"* = 2. Nhưng ra **ngay** khi được hỏi lại ⇒ lỗ hổng **diễn đạt**, không phải kiến thức.

**Đáp án ([bank C-003](../bank/c-programming.md)):**
> **Chưa khởi tạo:** chứa rác, trỏ đâu không biết — ghi vào là phá bộ nhớ ngẫu nhiên. **NULL:** giá trị đặc biệt "không trỏ đâu", kiểm được bằng `if (p)`; dereference thường crash ngay. **Treo:** từng hợp lệ, nhưng vùng đã `free` hoặc hết scope. Cả ba khi dereference đều là **UB**; chỉ `NULL` kiểm tra được.

**Tài liệu gốc** ([memory-model §6](../../../01-cpp-fundamentals/memory-model.md)):
> ```cpp
> // 5) Use of uninitialized
> int x;
> if (x == 0) {...}     // ❌ x có giá trị rác
> ```

**Chốt:** *"Chưa khởi tạo = rác, trỏ đâu không biết, không kiểm được. NULL = cố ý không trỏ đâu, kiểm được. Dangling = từng đúng, giờ chết."*
</details>

---

### Câu 3 · `CPP-020` · Ⓖ · 🟡 · **3/4** · 🔁 weak

**Rule of 0/3/5 là gì? Vì sao Rule of 0 được khuyến nghị?**

*(Không probe: câu trả lời không lửng, và rapid cấm leo tầng sang follow-up `unique_ptr` ghi trong weak-register.)*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- Rule of 3: *"khi sửa dtor/copy ctor/copy assign thì **compiler xác định** ta đang tự quản lý tài nguyên ⇒ cần viết cả 3"* — ✅ ý, ⚠️ sai chủ ngữ
- Rule of 5: định nghĩa 1 trong 3 thì move không được sinh ⇒ move gán, vector reallocate, truyền rvalue sẽ **copy âm thầm** ⇒ tự viết move ✅
- Rule of 0: dùng thành phần đã RAII sẵn ⇒ compiler sinh đủ và đúng ⇒ code sạch, không lỗi ✅

**✅ Được:** đủ cả ba luật, đúng lý do *"move âm thầm thành copy"* (bẫy chính của bank), đúng tinh thần Rule of 0.

**❌ Vì sao chưa 4:**
1. *"Compiler xác định ta đang quản lý tài nguyên"*: Rule of 3 là **hướng dẫn cho người viết**, compiler không suy luận gì. Cái compiler thật sự làm: khai báo dtor/copy ⇒ **ngừng sinh move**. Bạn nói đúng ý này ở Rule of 5, chỉ đặt nhầm chủ ngữ ở Rule of 3.
2. Rule of 0 chỉ nêu *"STL container"*. Nên nêu cả **`unique_ptr`/`shared_ptr`**, vì đó chính là ca regression 04/09 (member `unique_ptr` ⇒ copy bị xoá ngầm).
3. Hơi dài so với rapid.

**Đáp án phần còn thiếu ([bank CPP-020](../bank/cpp.md)):**
> Khai báo **destructor** (kể cả một destructor rỗng, kể cả `= default`) sẽ **CHẶN compiler sinh move constructor/move assignment**. Class vẫn biên dịch, vẫn chạy đúng — nhưng mọi phép move **lặng lẽ thành copy**.
>
> **Tách trách nhiệm.** Cần quản lý tài nguyên thô ⇒ tách ra **một class chỉ làm mỗi việc đó** (hoặc dùng sẵn `unique_ptr`/`vector`/`string`).

**Tài liệu gốc** ([raii-smart-pointers §6](../../../02-modern-cpp/raii-smart-pointers.md)):
> | **Rule of 3** | **ĐÚNG/SAI** — tránh leak, double-free, aliasing | class ôm tài nguyên thô, cần copy |
> | **Rule of 5** | **NHANH/CHẬM** — thêm đường move O(1) thay vì copy O(n) | như trên + quan tâm hiệu năng |
> | **Rule of 0** | **không phải viết gì** — để RAII member lo | mặc định nên hướng tới |

**Weak-register:** 1/2 lần ≥3, nhưng ở T1. **Điều kiện gỡ:** lần kế phải qua follow-up `unique_ptr` (gõ thử, dán output).
</details>

---

### Câu 4 · `DP-005` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC

**Strategy pattern là gì? C++ hiện đại hiện thực gọn thế nào?**

**🔁 Probe:** *"Phác 2–3 dòng: member trong class khai báo thế nào, caller truyền strategy vào ra sao?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"đóng gói thuật toán/hành vi sau một interface chung đơn giản, chọn/đổi lúc runtime mà không bắt caller sửa"* ✅. *"C++ hiện đại thường dùng `std::function` + lambda (bank không cho ví dụ nên hơi khó hiểu)"*. Probe: *"chưa rõ, bank không cho ví dụ"* ❌

**✅ Được:** định nghĩa đúng ý chính.

**❌ Vì sao 2:** thang rapid, *"thiếu một nửa"*. Nửa *"hiện thực thế nào"* chỉ có tên, không có hình. ⚖️ Phản hồi đúng: bank lúc đó chỉ một đoạn. Nhưng [behavioral §1](../../../11-design-patterns/behavioral.md) **có** code bản cây class. Bank đã nâng cấp.

**Đáp án ([bank DP-005](../bank/design-patterns.md), bản nâng cấp 05/10):**
```cpp
class Backlight {                                       // CONTEXT
    std::function<uint32_t(int32_t lux)> curve_;        // khe cắm
public:
    explicit Backlight(std::function<uint32_t(int32_t)> c) : curve_(std::move(c)) {}
    void setCurve(std::function<uint32_t(int32_t)> c) { curve_ = std::move(c); }
    uint32_t onLux(int32_t lux) { return curve_(lux); } // không biết thuật toán nào
};
Backlight b([](int32_t lux) { return static_cast<uint32_t>(lux / 8); });  // 50 với lux=400
b.setCurve([](int32_t lux) { return lux > 300 ? 100u : 20u; });           // đổi lúc runtime → 100
```
Output thật (`g++ -std=c++17 -Wall -Wextra`): `linear: 50` · `step: 100` · `tmpl: 200` (biến thể template parameter).

| Cách | Hợp khi |
|---|---|
| Cây class (`unique_ptr<IAlgo>`) | Thuật toán **có state** — dimming có bộ lọc, lịch sử zone |
| `std::function` + lambda | Thuật toán **thuần hàm** |
| Template parameter | Chốt lúc compile, đường nóng |

**Tài liệu gốc** ([behavioral §1](../../../11-design-patterns/behavioral.md)):
> ⭐ **Điểm cốt lõi hay bị bỏ sót:** giá trị của Strategy **không** nằm ở chỗ "có nhiều class kế thừa", mà ở chỗ **context không đổi một dòng nào** khi thêm thuật toán thứ tư.
>
> ⚠️ **Nhưng biết chỗ KHÔNG dùng lambda cũng là một điểm:** dimming **có state** (bộ lọc, lịch sử zone, timer chống nhấp nháy) ⟹ class là đúng. `std::function` hợp với strategy **thuần hàm**.

**Chốt:** *"Strategy = context giữ một khe cắm và uỷ nhiệm vào đó. Thuần hàm thì `std::function` + lambda; có state thì cây class; chốt lúc compile thì template."*
</details>

---

### Câu 5 · `C-035` · Ⓖ · 🟡 · **1/4** · KIẾN THỨC

**Chữ `static` có mấy nghĩa trong C? Một biến `static` khai báo trong hàm được khởi tạo khi nào? Và nếu đặt `static int helper(void) {…}` trong một header được nhiều file include thì sao?**

**🔁 Probe:** *"Lỗi linker đó cụ thể báo gì? Ví dụ có 3 file `.c` cùng include header này."*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- 2 nghĩa tuỳ vị trí ✅
- Trong hàm: *"tạo 1 lần **ngay lần đầu hàm gọi**, sống suốt chương trình, ở `.data`/`.bss`"* — ✅ vòng đời, ❌ thời điểm
- Ngoài hàm: chỉ file đó thấy ✅. *"Do đó nếu `static int helper` include nơi khác sẽ **báo lỗi linker**"* ❌
- Probe: *"không rõ, có thể báo là đang dùng private function"* ❌

**✅ Được:** khung 2 nghĩa (vòng đời / phạm vi nhìn thấy) đúng.

**❌ Vì sao 1:** hai trong ba vế của đề sai, và vế thứ ba **mâu thuẫn với chính định nghĩa bạn vừa nêu**. Đã nói *"chỉ file đó thấy"* thì linker làm sao thấy hai bản để báo trùng?

1. **Thời điểm khởi tạo:** *"lần đầu hàm gọi"* là mô tả của **C++** (dynamic init, chính là cơ chế Meyers' Singleton ở câu 12). Trong C, initializer phải là **hằng**, giá trị nằm sẵn trong `.data` **trước `main`**. Không có code nào chạy.
2. **`static` trong header:** **không lỗi**. Mỗi file một bản riêng. `multiple definition` là ca **ngược lại** (không `static`).

**Chạy thật (gcc 11.4):**
```
--- static int x = f(); trong hàm ---
C   (gcc -std=c11):   init.c:2:30: error: initializer element is not constant
C++ (g++ -std=c++17): compile OK (dynamic init lúc gọi lần đầu)

--- static helper trong header, 3 file .c include ---
a: helper @ 0x59c045eeb167
b: helper @ 0x59c045eeb19f
c: helper @ 0x59c045eeb1d7
exit=0                          <- link OK, 3 bản, 3 địa chỉ
```

**Đáp án ([bank C-035](../bank/c-programming.md)):**
> | **Trong hàm** | **Vòng đời** | Sống suốt chương trình, nằm ở `.data`/`.bss`, khởi tạo **một lần** trước khi chương trình chạy |
>
> **`static` trong header:** mỗi `.c` include nó có **một bản riêng** — đã kiểm: địa chỉ `helper` in từ hai file là **2 địa chỉ khác nhau**.

**Tài liệu gốc** ([c-language-idioms §2.1](../../../01-cpp-fundamentals/c-language-idioms.md), [§2.3](../../../01-cpp-fundamentals/c-language-idioms.md)):
> ```c
> int next_id(void) {
>     static int id = 100;     /* khởi tạo MỘT lần, lúc chương trình bắt đầu */
>     return id++;
> }
> ```
> Mỗi `.c` include header này có **một bản riêng** của `helper` … Với hàm nhỏ `static inline` thì đây là chủ ý (cách viết hàm tiện ích trong header của C). Với **biến** `static` trong header thì gần như luôn là bug: mỗi file sửa bản riêng của mình và tưởng là dùng chung.

**Chốt:** *"Trong hàm, `static` đổi vòng đời, và trong C giá trị có sẵn trước `main`. Ngoài hàm, `static` đổi phạm vi nhìn thấy, nên đặt trong header thì mỗi file một bản, không lỗi link."*
</details>

---

### Câu 6 · `CPP-045` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC · 🔁 weak · ⚖️ bank sai

**`= delete` khác cách cũ (khai báo private không định nghĩa) thế nào?**

**🔁 Probe:** *"Với **cách cũ**, lỗi xuất hiện lúc compile hay lúc link? Có trường hợp nào lọt qua được compile không?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- Luôn lỗi ở compile nếu cố dùng ✅
- Lỗi rõ ràng *"use of deleted function"* thay vì *"use private function"* ✅
- Hàm thường cũng `delete` được ✅
- Khai báo/delete dtor, copy hoặc move ⇒ compiler không sinh move nữa ✅ *(đúng chỗ hụt 04/09)*
- Probe: *"lỗi compile nếu gọi hàm private ở **friend, member**. Nếu tạo và dùng object ở nơi khác và build rời thì có thể lọt compile"* ❌ **ngược**

**✅ Được:** cả bốn ý về `= delete` đều đúng. Tự nêu được *"khai báo copy ⇒ không sinh move"*, chính là follow-up hụt ngày 04/09. Tiến bộ thật.

**❌ Vì sao 2:** vế *"cách cũ"*, nửa còn lại của câu so sánh, **đảo ngược lần thứ ba liên tiếp**. Member/friend **qua được** access check ⇒ chết ở **link**. Code ngoài class ⇒ chết ở **compile**.

**⚖️ Nhưng lỗi này có nguồn:** bank cũ ghi *"dùng nhầm chỉ lỗi lúc **link** (hoặc runtime với friend/member)"*, bản tóm tắt EMC Item 11 ghi *"chỉ chặn ở link time và chỉ với code ngoài class"*. **Cả hai đều đảo.** Bạn ôn đúng thứ bị viết sai. Đã sửa cả hai, 05/10.

**Chạy thật (gcc 11.4, `-std=c++17`):**
```
// gọi từ ngoài:   Fd b = a;
old_out.cpp:7:27: error: 'Fd::Fd(const Fd&)' is private within this context      <- COMPILE

// gọi từ member:  Fd clone() const { return *this; }
undefined reference to `Fd::Fd(Fd const&)'                                        <- LINK
collect2: error: ld returned 1 exit status
```

**Đáp án ([bank CPP-045](../bank/cpp.md), bản sửa 05/10):**
> | Ai gọi | Cách cũ: `private` + không định nghĩa | `= delete` |
> |---|---|---|
> | Code **ngoài** class | ❌ **compile** — `is private within this context` | ❌ compile — `use of deleted function` |
> | **Member / `friend`** | ❌ **link** — `undefined reference`, báo ở file `.o`, không chỉ ra dòng gọi | ❌ **compile**, chỉ đúng dòng gọi |
>
> Mẹo nhớ: ***private chặn NGƯỜI NGOÀI, thiếu định nghĩa chặn NGƯỜI NHÀ.***

**Tài liệu gốc** ([EMC Item 11](../../../15-book-summaries/effective-modern-cpp.md), bản sửa 05/10):
> Cách C++98 (khai báo private, không implement) chặn theo **hai cửa**: code **ngoài** class bị chặn lúc **compile** (access check, vì `private`); còn **member/friend** qua được access check nên chỉ bị chặn lúc **link** (thiếu định nghĩa).

**Chốt:** *"Cách cũ: người ngoài chết lúc compile vì `private`, người nhà lọt tới link mới chết vì thiếu định nghĩa. `= delete` bắt tất cả ngay lúc compile."*
</details>

---

### Câu 7 · `DP-006` · Ⓖ · 🟡 · **bỏ qua — không chấm**

**Observer pattern dùng khi nào? Rủi ro?**

<details><summary>Bạn trả lời gì · Phân xử · Đáp án</summary>

**Bạn trả lời:** *"bank thiếu ví dụ minh hoạ mà toàn câu khẳng định, nên tôi chưa hiểu. Resume tôi không làm về phần này, cân nhắc không hỏi / hỏi dễ hơn."*

**Phân xử** (chi tiết ở calibration cuối [weak-register](../weak-register.md)):
- ✅ **Bank mỏng:** đồng ý. Đã nâng cấp.
- ❌ **"Không làm":** không đồng ý. `DisplayController` ở [CPP-067](../bank/cpp.md) có `std::vector<IObserver*> observers_`. [in-practice/B2](../../../11-design-patterns/in-practice/B2-redesign-events.md) dựng Observer cho cảm biến ánh sáng. Mọi `register_callback` / vsync callback bạn từng viết là Observer chưa gọi tên.

**Đáp án ([bank DP-006](../bank/design-patterns.md), bản nâng cấp 05/10):**
- **Cơ chế:** subject giữ **danh sách callback**, đổi trạng thái thì gọi từng cái, không biết ai đang nghe.
- **Rủi ro số một: vòng đời.** Chạy thật bản con trỏ thô, `delete` observer rồi `publish`:
  ```
  ERROR: AddressSanitizer: heap-use-after-free ...
      #0 ... in SensorRaw::publish(int) obs.cpp:11
  freed by thread T0 here:
      #1 ... in Dimming::~Dimming() obs.cpp:5
  ```
  Bản `weak_ptr`: `Dimming nhận 100` → `observer đã chết -> gỡ`.

**Chốt:** *"Observer = subject giữ danh sách callback, không biết ai nghe. Rủi ro số một là vòng đời. Giữ `weak_ptr` để biến crash thành tự dọn."*
</details>

---

### Câu 8 · `C-020` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC

**`void *` là gì? Làm được gì, không làm được gì? Vì sao trong C không nên ép kiểu kết quả `malloc`, còn C++ thì bắt buộc?**

**🔁 Probe:** *"Nếu chỉ là thừa thì vô hại, vậy vì sao lại nói là **không nên**? Ép kiểu ở đó có thể che giấu lỗi gì?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- Địa chỉ không kiểu, cho API tổng quát (`malloc`, `memcpy`) ✅
- Chứa địa chỉ mọi kiểu, ngầm chuyển sang kiểu khác ✅
- Không dereference vì không biết đọc bao nhiêu byte ✅
- Ép trong C là thừa ✅ (tầng nông)
- C++ bắt buộc vì ngầm định không type-safe, cần `static_cast` ✅. *"Giả sử không cast thì sao???"*
- Probe: *"không rõ, nên giải thích kỹ hơn câu này ở bank"* ❌

**✅ Được:** khung đúng, lý do C++ đúng.

**❌ Vì sao 2:**
1. Thiếu *"không làm được **số học**"* (không biết bước nhảy, `p + 1` chỉ chạy nhờ extension GNU).
2. *"Thừa"* là tầng nông. Tầng sâu: cast **che lỗi quên `#include <stdlib.h>`**.
3. Câu bạn tự hỏi: C++ không cast thì **không compile**.

**Chạy thật (gcc 11.4, x86-64):**
```
--- C++, không ép ---
error: invalid conversion from 'void*' to 'int*' [-fpermissive]

--- C89, quên <stdlib.h>, CÓ ép (int *)malloc(16), rồi *p = 1 ---
p = 0xffffffff992392a0          <- 32 bit thấp, sign-extend: nửa trên địa chỉ đã mất
Segmentation fault (core dumped)

--- cảnh báo gcc 11 (không cần -Wall) ---
không ép: initialization of 'int *' from 'int' makes pointer from integer without a cast
có ép:    cast to pointer from integer of different size
```
⟹ Một sắc thái trung thực: gcc hiện đại **vẫn cảnh báo** cả hai trường hợp. Nhưng bản không ép nói **thẳng ra bệnh** (*"pointer from integer"*), còn với compiler cũ hoặc khi cảnh báo bị tắt thì bản có ép **im hoàn toàn**. Lời khuyên sinh ra từ đó.

**Đáp án ([bank C-020](../bank/c-programming.md)):**
> **`malloc` trong C:** `int *p = malloc(n * sizeof *p);` — **không ép**. … *Tầng sâu:* **cast là lệnh "im đi" gửi compiler**. Quên `#include <stdlib.h>` trong C89 thì `malloc` bị **ngầm khai báo là trả `int`** ⟹ địa chỉ 64-bit bị **cắt còn 32 bit**.

**Tài liệu gốc** ([c-pointers-arrays §7.3](../../../01-cpp-fundamentals/c-pointers-arrays.md)):
> | **Dereference** — không biết đọc bao nhiêu byte |
> | **Số học** — không biết bước nhảy (`void *p; p + 1` chỉ chạy nhờ **extension của GNU**, không phải chuẩn C) |
>
> - Trong C, `int *p = malloc(n);` **không cần ép kiểu**. Ép kiểu còn có hại ở C cũ: che mất lỗi quên `#include <stdlib.h>`.

**Chốt:** *"`void *` chuyền đi được, không đọc và không cộng được. Trong C đừng ép `malloc`, vì cast tắt tiếng compiler đúng lúc nó cần báo quên include. Trong C++ không ép thì không compile."*
</details>

---

### Câu 9 · `OS-022` · Ⓖ · 🟢 · **3/4**

**Hai thread trong cùng process trao đổi dữ liệu chỉ bằng một biến toàn cục. Hai process thì không làm vậy được. Vì sao — và cái "không được" đó nằm ở đâu?**

**🔁 Probe:** *"Biến đó ở hai process có thể mang **cùng một địa chỉ** (`&g` in ra giống hệt nhau). Vậy cơ chế nào khiến cùng một địa chỉ lại trỏ tới hai chỗ khác nhau?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"hai process có memory map riêng, mỗi process giữ một bản biến toàn cục, không liên hệ"* ✅. Probe: *"MMU, OS xử lý page table ánh xạ địa chỉ (vốn giống nhau) của từng process vào các page độc lập trên RAM. Chung địa chỉ nhưng là ảo"* ✅

**✅ Được:** đúng cơ chế bảng trang, phân biệt ảo/vật lý.

**❌ Vì sao chưa 4:** vế *"nằm ở đâu"* là một nửa đề, phải probe mới ra. Thiếu chiều ngược lại: thread **chung** bảng trang nên chia sẻ là mặc định. Thiếu bẫy: process **có** chia sẻ được (`MAP_SHARED`), chỉ là **không mặc định**.

**🎁 Bằng chứng thật, tiện từ câu 12:** demo singleton chạy 2 process (tắt ASLR bằng `setarch -R`):
```
--- process 1 ---                    --- process 2 ---
libB: 0x7ffff7f9f024 count=1         libB: 0x7ffff7f9f024 count=1
libC: 0x7ffff7f9f024 count=2         libC: 0x7ffff7f9f024 count=2
```
**Cùng địa chỉ ảo, count đếm lại từ 1** ⇒ hai khung trang vật lý khác nhau. Đúng câu trả lời của bạn.

**Đáp án ([bank OS-022](../bank/os.md)):**
> **Bẫy:** nói *"process không chia sẻ được bộ nhớ"* — **sai**, `mmap(MAP_SHARED)`/shared memory chính là việc **cố ý map hai bảng trang về cùng khung vật lý**. Đúng phải là: *không chia sẻ **mặc định**, phải yêu cầu tường minh*.

**Tài liệu gốc** ([ipc](../../../03-operating-system/ipc.md)):
> Mỗi process có không gian địa chỉ riêng (cô lập) — đây là điểm mạnh nhưng cũng nghĩa là chúng *không thể* đọc bộ nhớ của nhau trực tiếp. … (Thread cùng process thì chia sẻ bộ nhớ sẵn, chỉ cần đồng bộ.)

**Chốt:** *"Thread chung bảng trang nên chia sẻ là mặc định; process riêng bảng trang nên con trỏ mất nghĩa. IPC là cơ chế bắc cầu."*
</details>

---

### Câu 10 · `CPP-057` · Ⓖ · 🟡 · **3/4**

**`std::thread` bị huỷ khi vẫn còn `joinable()` thì chuyện gì xảy ra? `std::jthread` khác gì?**

**🔁 Probe:** *"`jthread` join **vào lúc nào** chính xác? Nếu thread đó đang chạy một vòng lặp vô hạn thì destructor có bị treo không?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"terminate chương trình; destructor không làm gì phù hợp được — tự join thì block chờ, detach thì nguy cơ dùng biến cục bộ đã hết vòng đời ⇒ terminate để báo lỗi rõ"* ✅✅. *"jthread tự động join **khi thread kết thúc**"* ⚠️. Probe: *"không rõ"* ❌

**✅ Được:** vế 1 đạt mức 4: nêu được cả lý do vì sao chuẩn chọn `terminate()`, đúng ý ⭐ của bank.

**❌ Vì sao chưa 4:** `jthread` join **khi object bị huỷ**, không phải *"khi thread kết thúc"*. Và trước khi join nó **`request_stop()`**. Đó là nửa thứ hai của sự khác biệt.

> ⚖️ **Ghi nhận trọng số:** `jthread` là **C++20**, nằm ngoài mốc C++17 mặc định của repo ⇒ vế này thấp ưu tiên.

**Chạy thật (g++ 11.4, `-std=c++20`):**
```
--- std::thread huỷ khi joinable ---
terminate called without an active exception
Aborted (core dumped)   exit=134

--- jthread, worker kiểm stop_token ---
main: ra khỏi scope -> ~jthread() = request_stop() + join()
worker: thấy stop_requested -> thoát
main: kết thúc

--- jthread, worker BỎ QUA stop_token, chạy với timeout 1s ---
exit=124 (bị timeout giết ⇒ destructor TREO)
```
⟹ Trả lời probe: `jthread` **có thể treo** y như tự join, nếu thread không chịu kiểm `stop_token`. `request_stop()` chỉ là **lời đề nghị**. Huỷ là **hợp tác**, không phải cưỡng bức.

**Đáp án ([bank CPP-057](../bank/cpp.md)):**
> | Destructor khi còn joinable | **`terminate()`** | **Tự `request_stop()` rồi `join()`** |
> | Hỗ trợ huỷ | Không có | **`std::stop_token`** — hợp tác, thread tự kiểm và thoát |

**Tài liệu gốc** ([concurrency](../../../02-modern-cpp/concurrency.md)):
> - Một `std::thread` bị hủy khi vẫn **joinable** (chưa join/detach) → gọi `std::terminate()` (crash). Luôn join/detach.
> - Modern: dùng **`std::jthread`** (C++20) — tự join khi hủy + hỗ trợ stop token.

**Chốt:** *"`~thread()` gọi `terminate()` nếu còn joinable, cố ý ồn ào. `jthread` khi bị huỷ thì `request_stop()` rồi `join()`, nhưng thread phải tự kiểm `stop_token`, không thì vẫn treo."*
</details>

---

### Câu 11 · `C-032` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC

**Đồng nghiệp thay mọi `strcpy` bằng `strncpy(dst, src, sizeof dst)` "cho an toàn". Có đúng không?**

**🔁 Probe:** *"Nếu đoạn đó nằm trong hàm `void f(char *dst, const char *src)` thì `sizeof dst` bằng bao nhiêu?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"chưa chắc đúng. strncpy không đảm bảo an toàn, chỉ đảm bảo copy n byte, **nó vẫn ghi kể cả khi dst không đủ chứa src**, và không đảm bảo có null terminate"*. Probe: *"8 byte, size con trỏ, không phải size mảng"* ✅

**✅ Được:** *"không bảo đảm `'\0'`"*, ý chính số một. Bẫy `sizeof` con trỏ trả lời ngay.

**❌ Vì sao 2:**
1. *"Vẫn ghi kể cả khi dst không đủ"*: **sai** với `n = sizeof dst`. Nó dừng ở `n` byte. Lỗi không ở **lúc ghi** mà ở **lúc đọc sau** (thiếu `'\0'` ⇒ `strlen`/`printf` chạy tràn).
2. Không nêu cách thay: `snprintf(dst, sizeof dst, "%s", src)` và **kiểm giá trị trả về** để biết bị cắt.

**Chạy thật (gcc 11.4, `-Wall -Wextra`):**
```
warning: 'strncpy' output truncated copying 4 bytes from a string of length 6 [-Wstringop-truncation]
canary sau strncpy = "ZZZ" (nguyên vẹn => KHÔNG tràn)
dst[3] = 'd' (không phải '\0')
sizeof dst trong f = 8
```

**Đáp án ([bank C-032](../bank/c-programming.md)):**
> **Không.** `strncpy` chép tối đa `n` byte và **chỉ thêm `'\0'` khi nguồn ngắn hơn `n`**. Nguồn dài hơn ⟹ `dst` không có `'\0'`, `printf("%s")`/`strlen` sau đó đọc tràn.
>
> **Thay bằng:** `snprintf(dst, sizeof dst, "%s", src);` — luôn có `'\0'`, cắt bớt nếu thiếu chỗ. `snprintf` còn **trả về độ dài lẽ ra cần** ⟹ so với `sizeof dst` để biết đã bị cắt.

**Tài liệu gốc** ([c-language-idioms §5.3](../../../01-cpp-fundamentals/c-language-idioms.md)):
> `strncpy` chép **tối đa n byte** và chỉ thêm `'\0'` nếu nguồn **ngắn hơn** n. Nguồn dài hơn ⟹ `dst` không còn là chuỗi, `printf("%s")` đọc tràn. (Nó còn **đệm `'\0'` tới hết n** khi nguồn ngắn — tốn công nếu buffer lớn.)

**Chốt:** *"`strncpy` không bảo đảm `'\0'`. Chép chuỗi có giới hạn thì dùng `snprintf` và kiểm giá trị trả về. `sizeof` tham số con trỏ là 8."*
</details>

---

### Câu 12 · `DP-002` · Ⓖ · 🟢 · **2/4** · KIẾN THỨC · 🔁 weak

**Singleton là gì? Cách hiện đại trong C++?**

**🔁 Probe** *(⚖️ leo tầng, vi phạm luật rapid, không tính điểm)*: *"Singleton này nằm trong một thư viện `.so`, và trong **cùng một process** có hai `.so` khác cùng link tới nó. Lúc chạy có mấy instance, và điều gì quyết định con số đó?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"class chỉ có 1 instance duy nhất, truy cập global. Meyers' singleton: static local trong `instance()`, khởi tạo lazy lần đầu gọi, **luôn** thread-safe từ C++11 (guard variable)"*.

Probe: phản hồi *"câu này khá nâng cao"*, tự đặt câu thay: *"nhiều process cùng dùng `.so` có singleton thì mỗi process 1 bản hay 1 bản duy nhất?"* Trả lời: *"mỗi process 1 bản vì `.so` map vào memory riêng"* ✅. *"An toàn multiprocess là nhờ shared memory + semaphore"* ✅. *"Do đó singleton **chỉ có tác dụng tránh nhiều instance cho nội bộ 1 process khi chúng có nhiều thread**"* ❌

**✅ Được:** định nghĩa + Meyers + lazy + guard variable. Câu tự đặt chính là [DP-041](../bank/design-patterns.md), trả lời đúng vế chính (*object per-process, state per-system*).

**❌ Vì sao 2** (dạng Ⓖ: phần nền thiếu ý chính ⇒ trần 2):
1. Không nêu **ctor `private` + copy `= delete`**. Đó là thứ thật sự *ép* tính duy nhất. `static` local chỉ là *cách tạo*, không *ngăn* ai tự `Logger x;`.
2. *"Luôn thread-safe"*: chỉ **khởi tạo** thread-safe. Hai thread cùng gọi method trên instance vẫn cần khoá.
3. *"Singleton chỉ có tác dụng khi nhiều thread"*: Singleton **không liên quan tới thread**. Nó tồn tại vì tài nguyên **vật lý** chỉ có một. Process một luồng vẫn cần nó.

**Về probe:** ca bị hỏi đơn giản hơn bạn nghĩ. Định nghĩa `instance()` chỉ nằm trong `libA.so` ⇒ `libA.so` nạp **một lần** mỗi process ⇒ **1 instance**. Chạy thật:
```
pid 7134
libB: 0x7ffff7f9f024 count=1
libC: 0x7ffff7f9f024 count=2     <- cùng địa chỉ, count đi tiếp ⇒ 1 instance trong process
số lần libA.so được init trong 1 process: 1
```
Phần **khó** (singleton nằm ở header inline, nhiều `.so` cùng include, `-fvisibility=hidden` lật kết quả) là [DP-020](../bank/design-patterns.md). Phần tên cờ là T3, đồng ý không chấm.

**Đáp án ([bank DP-002](../bank/design-patterns.md)):**
> Đảm bảo một class chỉ có một instance + điểm truy cập toàn cục. C++11+ dùng Meyers' Singleton: `static` local trong hàm `instance()` — khởi tạo **lazy** và **thread-safe theo chuẩn**. Cấm copy (`= delete`), constructor private.

**Tài liệu gốc** ([creational §3](../../../11-design-patterns/creational.md)):
> | **Vấn đề nó thật sự giải** | Tài nguyên **vật lý** chỉ có một (một thiết bị, một bus, một vùng nhớ ánh xạ) |
> ```cpp
>     Logger(const Logger&)            = delete;
>     Logger& operator=(const Logger&) = delete;
> private:
>     Logger() = default;
> ```

**Chốt:** *"Singleton = ctor private + copy deleted + một điểm truy cập. Meyers chỉ bảo đảm khởi tạo thread-safe. Một `.so` nạp một lần mỗi process ⇒ một instance mỗi process, nhiều process thì mỗi process một bản."*
</details>

---

## Tổng kết

**Điểm mạnh:**
- **OS / concurrency có nền thật:** câu 9 (bảng trang), câu 10 (vì sao chuẩn chọn `terminate()`) đều nói ra cơ chế, không chỉ kết luận.
- **CPP-045 tiến bộ ở đúng chỗ đã hụt** (khai báo copy ⇒ không sinh move). Phần còn sai có nguồn là bank sai.
- **Tự đặt câu hỏi thay thế** ở câu 12 rất đúng hướng (= DP-041).
- **Phản hồi bank có căn cứ:** cả ba chỗ bạn chỉ ra đều mỏng thật.

**3 lỗ hổng ưu tiên:**

| # | Lỗ hổng | Ôn ở đâu | Cách ôn |
|---|---|---|---|
| 1 | **C: đoán hệ quả cơ học thay vì chạy thử** (`static` header, `strncpy`, cast `malloc`) | [c-language-idioms §2](../../../01-cpp-fundamentals/c-language-idioms.md), [§5.3](../../../01-cpp-fundamentals/c-language-idioms.md) · [c-pointers-arrays §7.3](../../../01-cpp-fundamentals/c-pointers-arrays.md) | Mỗi khẳng định: **gõ 5 dòng, chạy**. Bank C có sẵn output thật để đối chiếu |
| 2 | **C khác C++ chỗ khởi tạo `static`** — mang luật C++ sang C | [bank C-035](../bank/c-programming.md) bảng *"Khởi tạo khi nào"* | Nối với Meyers' Singleton: *"lazy lần đầu gọi"* là đặc quyền C++ |
| 3 | **Compile vs link của cách C++98** | [bank CPP-045](../bank/cpp.md) **bản sửa 05/10** | Mẹo: *private chặn người ngoài, thiếu định nghĩa chặn người nhà* |

**Phiên kế đề xuất:** `daily` hoặc `comprehensive` track `cpp-system`, để **xả 8 câu retention quá hạn** (CPP-029, CPP-009, OS-003, OS-007, CPP-019, CPP-024, CPP-032, LNX-023 — hạn 18–23/09). Rapid không có suất retention.

---

## 📊 Độ phủ bank (config §7 — đo sau phiên này)

Đo bằng lệnh §7 **sau khi** ghi log này. Đây là phiên thứ 28 (lần đo bắt buộc mỗi 5 phiên). Chỉ liệt kê domain của track `cpp-system`.

| Domain | Đã hỏi / bank | Phủ | Ghi chú |
|---|---|---|---|
| `CPP` | 51 / 68 | **75%** | 🟠 phủ dày nhất |
| `LNX` | 26 / 43 | 60% | |
| `OS` | 15 / 29 | 52% | |
| `DSA` | 7 / 16 | 44% | |
| `DBG` | 18 / 42 | 43% | |
| `SD` | 7 / 33 | 21% | |
| `DP` | 8 / 47 | 17% | có lẫn ID chỉ được **nhắc tới** (DP-020, DP-041) và câu bỏ qua (DP-006) ⇒ phủ thật thấp hơn |
| `BLD` | 5 / 39 | 13% | |
| **`C`** | **5 / 51** | **10%** | 🟠 từ 0% lên. Domain mới 03/10, thêm sau phản hồi PV thật *"C thuần vẫn bị hỏi"* |

⚠️ Lệnh §7 đếm **mọi ID xuất hiện trong log**, kể cả ID chỉ được nhắc ⇒ số phủ là **trần trên**.

**Ngưỡng §7:** `CPP` > 60% trong khi `C` = 10% ⇒ 🟠 **lệch**. Không có plan JD ⇒ đề xuất: xen `rapid track c` (5 câu C đầu tiên: 4·2·1·2·2 — bốn câu dưới 3) với `daily track cpp-system` để xả retention.
