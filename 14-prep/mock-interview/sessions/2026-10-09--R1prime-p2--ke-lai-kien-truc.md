# Phiên mock — 2026-10-09 · **R1′ phần 2** · kể lại kiến trúc theo chiến lược hiện tại · [resume-plan](../../study-plans/resume-plan.md)

- **Level:** mid-level · **Type:** mock chính, nối [R1′ phần 1](2026-10-08--R1prime--ke-lai-kien-truc.md) · **Trần:** T2, theo luật ⑤ của plan chủ yếu T1 + một chút T2
- **Số câu:** 7 — câu 8–12 của [plan §11](../../study-plans/resume-plan.md#11-r1--kể-kiến-trúc-theo-chiến-lược-hiện-tại) + `LNX-045` (weak) + `LNX-046` (mới)
- **Điểm:** **18/28 = 2.57** · **Cả R1′ (14 câu): 35/56 = 2.50**

---

## Kết quả từng câu

| # | ID | Câu | Dạng | Điểm | Lần trước | Loại lỗ hổng |
|---|---|---|:-:|:-:|:-:|---|
| 8 | `DP-041` | 5 process — mấy object, mấy state, Singleton không, vì sao init không race | Ⓖ | **3** | 2 | Trộn hai tầng (Service Locator vs Meyers) — một phần do A1 lệch, đã sửa |
| 9 | `DRV-006` | Vì sao driver không deref con trỏ user | Ⓖ | **2** | — | KIẾN THỨC — giá trị trả về, TOCTOU |
| 10 | `DP-043` | `panel_ops` vs `virtual` — hai rủi ro | Ⓖ | **3** | 2 | Chưa viết được dòng kiểm `size` |
| 11 | `DP-038` | Bridge vs Strategy trong hệ dimming | Ⓑ | **3** | 2 | Phép thử áp ngược ở (b); nói *"N + M"* |
| 12 | `RES-034` 🇬🇧 | Vẽ kiến trúc trong lúc kể | Ⓑ | **2** | 2 | ĐÓNG GÓI — lượt đầu thiếu C++ interface, thiếu lý do khoá |
| 13 | `LNX-045` | Process chết khi giữ semaphore | Ⓖ | **3** | 2 | Chủ còn sống thì vẫn cho API chạy không khoá |
| 14 | `LNX-046` | Vì sao semaphore thay mutex, timeout-reset hỏng ở đâu, sửa thế nào | Ⓖ | **2** | — | KIẾN THỨC — robust mutex, ghi kiểu commit |

### 🔎 Chẩn đoán

1. **Bốn câu weak lên 3 cùng lúc** (`DP-041`, `DP-043`, `DP-038`, `LNX-045`). Điểm chung: đọc bank và review hôm trước **có tác dụng ngay** — `DP-043` lần này tự nêu đủ hai rủi ro (lần trước không nêu được rủi ro nào), `LNX-045` nối được *"không có chủ ⟹ kernel không nhả hộ"*.
2. **Vẫn hụt ở chỗ phải VIẾT/SỬA, không phải chỗ phải HIỂU:** dòng kiểm `size` (`DP-043`), cách ghi state để chết giữa chừng không dở (`LNX-046`), giá trị trả về của `copy_from_user` (`DRV-006`). Hiểu vấn đề, chưa có công cụ để chữa.
3. **Bản tiếng Anh vẫn co ranh giới** (`RES-034`): lượt đầu bắt đầu từ API C, mất tầng C++ interface ở trên và mất lý do *"chỗ duy nhất lấy khoá"*. Đây là lần thứ hai, cùng bệnh với `RES-035` ở phần 1.
4. **Một mô hình sai lặp lại:** *"mỗi process định danh khoá khác nhau"* — hôm qua cho semaphore có tên (sai), hôm nay cho mutex. Với mutex thì **trúng một nửa**: thiếu `PTHREAD_PROCESS_SHARED`, hàng đợi ngủ đúng là của riêng từng process (output ở câu 14).

### ⚖️ Góp ý của người học — xử lý

| Góp ý | Phân xử | Đã làm |
|---|---|---|
| Câu 8: *"code trong A1 mục 5.7 là Meyers singleton"* | **Đúng một nửa — A1 có CẢ HAI, ở hai tầng khác nhau.** `DimmingFactory::GetDimmingInstance()` ([A1 §5.6](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md), được `lib_dimming` ở §5.7 gọi) đúng là Meyers Singleton. Còn `get_dimming_instance()` mà câu hỏi nhắc ([A1 §7.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)) là con trỏ toàn cục `static lib_dimming_interface *dimming`, gán trong `lib_init_modules()`. Tên gần giống nhau là cái bẫy; tài liệu chưa cảnh báo. Snippet interviewer đưa ra cũng rút gọn lệch (gán `DimmingFactory::create(...)` thay vì `new lib_dimming()`), góp phần gây nhầm | Thêm cảnh báo *"hai hàm tên gần giống, hai cơ chế"* vào A1 §7.1 + bẫy vào [bank DP-041](../bank/design-patterns.md). **Không trừ điểm** phần nhầm này |
| Câu 8: *"code A1 không dùng con trỏ raw `g_dimming`"* | ❌ **Không đúng.** A1 §7.1 và `src/lib_base.cpp` ở §11 đều dùng `static lib_dimming_interface* dimming = nullptr;` | — |
| Câu 8: *"`lib_dimming.cpp` trong mục full code trắng"* | ✅ **Đúng.** File chỉ có `#include`; toàn bộ code nằm inline trong `lib_dimming.h`, và lab để `DimmingFactory` làm **member** chứ không phải Meyers Singleton như §5.6 — mà bảng *"giản lược có chủ đích"* không ghi điều đó | Thêm comment vào `lib_dimming.cpp` + một dòng vào bảng giản lược ở [A1 §11](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) |

---

## 🔎 Chi tiết từng câu

### Câu 8 · `DP-041` · Ⓖ · 🟡 · **3/4**

**Library của em được nạp vào 5 process. Mỗi process có `get_dimming_instance()` trả về một con trỏ toàn cục. Hỏi: có bao nhiêu object dimming? Bao nhiêu bản state thật? Thứ đó là Singleton à? Và vì sao chỗ khởi tạo không có data race?**

**Follow-up** — với code:
```cpp
// lib_dimming.cpp
static IDimming* g_dimming = nullptr;          // con trỏ toàn cục, KHÔNG phải static local

IDimming* get_dimming_instance() { return g_dimming; }

void __attribute__((constructor)) lib_init() {
    g_dimming = DimmingFactory::create(read_board_config());   // new object cụ thể
}
```
① Có static local nào không? Cái gì đảm bảo `g_dimming` được gán trước khi hai thread cùng gọi? `lib_init` chạy lúc nào, ai gọi? ② Có gì ngăn đoạn code khác gọi `create(...)` lần hai? Đây có thật là Singleton không?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** 5 object, mỗi process một; một state duy nhất trong shared memory. *"Vẫn là một dạng singleton"* — tránh khởi tạo và dùng nhiều nơi trong cùng process. Khởi tạo tránh race bằng *"Meyers singleton ngay lúc library được load, đặt trong attribute constructor, khởi tạo an toàn trước khi bất kỳ API nào được dùng"*.
- ① Không phải Meyers, chỉ con trỏ là static. Vẫn thread-safe vì process load `.so` thì `lib_init()` tự chạy, gán `g_dimming` trước khi các thread kịp gọi.
- ② Không gì ngăn được ⟹ không phải Singleton; không nhớ tên pattern.

**✅ Được:** 5 object / 1 state ✅ · lý do không race **có ngay ở lượt đầu** (constructor của `.so`, trước mọi API) ✅ · sau follow-up tự kết luận *"không gì ép duy nhất ⟹ không phải Singleton"* ✅ — đúng lỗ hổng lần trước (06/10 nói *"cả 5 đều là Singleton"*, init *"nhờ semaphore"*).

**❌ Vì sao chưa 4:**
- Lượt đầu gọi là *"một dạng singleton"* và *"Meyers singleton"* — trộn hai tầng (xem phân xử ở trên). Tên **Service Locator** không nhớ ra — là nhãn (T3), không trừ; nhưng ý *"không gì ép tính duy nhất"* phải tự nói, không đợi hỏi.

**Đáp án ([bank DP-041](../bank/design-patterns.md)):**
> **Gọi tên đúng:** `get_*_instance()` trên con trỏ toàn cục là **Service Locator**, không phải Singleton GoF — không gì *ép* tính duy nhất, chỉ có quy ước "chỉ hàm init gán".
>
> **Vì sao không race:** khởi tạo chạy trong `__attribute__((constructor))`, tức **trước khi process tạo thread nào** ⟹ không có check-then-act cạnh tranh. Đây là cách chữa race khác magic statics: **dời khởi tạo ra khỏi vùng có cạnh tranh**.

**Tài liệu gốc ([A1 §7.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)), đoạn vừa thêm:**
> ⚠️ **Hai hàm tên gần giống, hai cơ chế khác nhau — đừng gộp:** `get_dimming_instance()` (ở đây, tầng `lib_base`) trả **con trỏ toàn cục** gán trong constructor của `.so` ⟹ Service Locator. `DimmingFactory::GetDimmingInstance()` (§5.6, bên trong `lib_dimming`) dùng **static local** ⟹ Meyers Singleton.

</details>

---

### Câu 9 · `DRV-006` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC

**Vì sao driver không được dereference con trỏ user trực tiếp? (`copy_from_user`/`copy_to_user`)** — bám hệ: library gọi `ioctl(fd, PANEL_SET_BACKLIGHT, &cfg)`, `cfg` là struct trên stack của process.

**Follow-up:** ① địa chỉ hợp lệ nhưng trang **không nằm trong RAM** — deref thẳng thì sao, `copy_from_user` khác gì? ② đồng nghiệp đọc `user_ptr->level` để kiểm rồi mới `copy_from_user` cả struct; process có nhiều thread — vấn đề gì? ③ `copy_from_user` trả gì, driver làm gì với nó?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** con trỏ user không đáng tin: địa chỉ không hợp lệ; địa chỉ kernel (cố ý để kernel thao tác trên vùng của chính nó) ⟹ phải qua `copy_*_user()` để kiểm hợp lệ trước.
- ① Page fault; `copy_from_user` xử lý bằng cách nạp đúng vùng đó lên RAM rồi copy.
- ② Truy cập `user_ptr` không verify ⟹ có thể UB, phải copy cả struct rồi mới verify. *"Chưa rõ nhiều thread ảnh hưởng gì, vì ioctl truyền struct là atomic."*
- ③ *"Trả về `void*`, cần cast lại đúng struct."*

**✅ Được:** hai lý do chính (không hợp lệ · địa chỉ kernel ⟹ leo thang đặc quyền) ✅ · ① đúng hướng ✅ · ② đúng kết luận *"copy trước, kiểm sau"*.

**❌ Vì sao 2:**
- ③ **sai**: trả về **số byte CHƯA chép được** — `0` là thành công, khác `0` ⟹ trả `-EFAULT`. Đây là bẫy số (1) của bank.
- ② đúng kết luận nhưng **sai lý do**: *"ioctl là atomic"* không đúng — `ioctl` không khoá bộ nhớ user; thread khác cùng process ghi vào `cfg` được trong lúc driver đang chạy ⟹ kiểm trên bản user rồi copy lại là TOCTOU.
- ① thiếu nửa *"deref thẳng thì sao"*: không có trong **bảng ngoại lệ** ⟹ oops; `copy_from_user` có trong bảng ngoại lệ nên fault ở đó được xử lý (nạp trang, hoặc bỏ cuộc sạch và trả lỗi).

**Đáp án ([bank DRV-006](../bank/drivers-embedded.md)):**
> ```c
>     if (len > sizeof(kbuf)) return -EINVAL;        // ✅ luôn kiểm tra len TRƯỚC
>     if (copy_from_user(kbuf, ubuf, len))           // trả về SỐ BYTE CHƯA chép được
>         return -EFAULT;                            // ✅ khác 0 = lỗi
> ```
> | **Xử lý page fault an toàn** | Địa chỉ user **hợp lệ nhưng chưa nằm trong RAM** (bị swap / cấp phát lười). Deref thẳng trong kernel gặp trang chưa map ⇒ **oops**. `copy_*_user` được đăng ký trong **bảng ngoại lệ**: fault tại đó không phải bug — kernel nạp trang rồi chạy tiếp, hoặc bỏ cuộc sạch |
>
> **Bẫy:** (1) tưởng giá trị trả về là số byte **đã** chép — ngược lại, **0 = thành công** … (4) **TOCTOU**: copy vào kernel rồi validate, đừng validate trên bộ nhớ user rồi mới copy — user có thể đổi giữa hai bước. `ioctl` **không** khoá bộ nhớ user …

**Tài liệu gốc:** [kernel-userspace](../../../05-drivers-device-tree/kernel-userspace.md) — mục `copy_from_user`.

</details>

---

### Câu 10 · `DP-043` · Ⓖ · 🟠 · **3/4**

**Trong kernel, driver nền không biết tên chip nào; nó gọi driver chip qua một struct con trỏ hàm `panel_ops` mà driver chip tự điền vào lúc init. So với `virtual` trong C++: giống gì, khác gì — và nó thừa hưởng HAI rủi ro nào của vtable?**

**Follow-up** — hai layout:
```c
/* bản cũ — .ko chip build năm ngoái */      /* bản mới — driver nền build năm nay */
struct panel_ops {                            struct panel_ops {
    size_t size;                                  size_t size;
    int (*init)(void);                            int (*init)(void);
    int (*set_brightness)(int);                   int (*set_brightness)(int);
};                                                int (*get_temp)(int *out);   /* thêm ở CUỐI */
                                                  };
```
① Driver nền gọi `ops->get_temp(&t)` trên bảng của `.ko` cũ — đọc ra gì, chuyện gì xảy ra? ② `size` cứu thế nào — viết dòng kiểm. ③ HAL + driver panel build chung một RPM — đủ an toàn chưa?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** Giống: đa hình, gọi từ base tự tới derived. Khác: `virtual` do compiler viết, `panel_ops` người viết tay; hàm không hỗ trợ thì `virtual` gọi bản base, `panel_ops` là `NULL`. Hai rủi ro: **slot không gán còn `NULL`** · **chèn thêm hàm vào struct ⟹ mọi slot lệch**, phải build khớp HAL và driver.
- ① UB — đọc vượt ranh giới `panel_ops` bản cũ.
- ② So `size` hai bên lệch là biết; *"chưa rõ viết kiểu gì"*.
- ③ Chặn được đường thường; vẫn lọt: hotfix `.ko` lẻ / rollback một phần package; `.ko` cũ còn sót trên máy và có thể bị nạp.

**✅ Được:** **tự nêu đủ hai rủi ro** (06/10: không nêu được rủi ro nào, đoán ngược hàm chạy) ✅ · ① đúng ✅ · ③ đúng và đủ hai đường lọt chính ✅.

**❌ Vì sao chưa 4:** ② chưa viết được dòng kiểm — đúng phần *"làm"* của câu. ① nói được *"UB"* nhưng chưa nói tiếp **nhảy tới đâu**: 8 byte nằm sau bảng của `.ko` cũ bị coi như con trỏ hàm ⟹ oops.

**Đáp án — dòng kiểm (đã thêm vào [bank DP-043](../bank/design-patterns.md)), chạy thật:**
```c
#define HAS_OP(ops, f) ((ops)->size >= offsetof(struct panel_ops, f) + sizeof((ops)->f) && (ops)->f)

static int core_get_temp(const struct panel_ops *ops, int *t) {
    if (!HAS_OP(ops, get_temp)) return -ENOTSUP;    /* bảng cũ, hoặc slot chưa điền */
    return ops->get_temp(t);
}
```
```
.ko cu : size=24 -> get_temp tra -95
.ko moi: size=32 -> get_temp tra 0, t=42
```
Kiểm `size` **trước** rồi mới đọc `ops->f` — đọc slot nằm ngoài bảng thì chính phép đọc đã sai.

</details>

---

### Câu 11 · `DP-038` · Ⓑ · 🟡 · **3/4**

Hệ dimming có `IDimmingAlgo` (thuật toán: `GlobalDimming`, `LocalDimming`) và `IDimmingBackend` (ghi xuống phần cứng từng chip). `lib_dimming` giữ `IDimmingAlgo*`, mỗi thuật toán giữ `IDimmingBackend*`.
**(a)** Xoá hẳn `IDimmingBackend` — `GlobalDimming` còn làm trọn việc của nó không? **(b)** Xoá hẳn `IDimmingAlgo`, `lib_dimming` gọi thẳng một thuật toán cố định — `lib_dimming` còn làm được việc của nó không? **(c)** Quan hệ nào là Bridge, quan hệ nào là Strategy? Phép thử là gì?

**Follow-up:** chỉ được dùng **một câu** để phân biệt, em hỏi câu gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) *"Có thể làm được việc của nó, nhưng không có phương tiện thực thi phía hw do backend đảm nhiệm."* `GlobalDimming` thuần tính toán, có thể thế backend để mock test.
- (b) `lib_dimming` không còn làm được việc như cũ — gắn liền một thuật toán, phải if-else mọi nơi.
- (c) Strategy: `lib_dimming` dùng `IDimmingAlgo` không cần biết loại, chọn đúng lúc runtime. Bridge: *"lib_dimming gồm 2 phần interface (algo) và implementor (backend)… chỉ cần N algo + M backend là tạo ra tất cả combination."*
- Follow-up: Strategy có **một trục** biến thiên (algo theo model); Bridge có **hai trục** (algo là gì, backend điều khiển nó là gì).

**✅ Được:** (c) **gán đúng** — Strategy = `lib_dimming` ↔ `IDimmingAlgo`, Bridge = thuật toán ↔ backend ✅. Ý *"thế backend để mock test thuật toán"* là lợi ích thật của Bridge ✅. Phép thử *"một trục / hai trục"* là cách phân biệt hợp lệ. Lần trước (06/10): *"chưa rõ"* nửa còn lại là gì — lần này chỉ ra ngay.

**❌ Vì sao chưa 4:**
- (b) **áp ngược phép thử của bank**: bỏ `IDimmingAlgo` thì `lib_dimming` **vẫn** là một context hoàn chỉnh — chỉ mất khả năng đổi thuật toán. Còn (a) mới là chỗ *"thành một nửa"*: tính xong mà không chạm được phần cứng. Câu trả lời (a) tự mâu thuẫn (*"làm được… nhưng không có phương tiện thực thi"*).
- *"N algo + M backend"* — đúng sách, **sai với hệ thật**: backend nhân theo chip × thuật toán (A1 §5.4). Chiến thuật #1 ở plan §10: rút gọn bằng cách bỏ bớt, không rút gọn thành sai.

**Đáp án ([bank DP-038](../bank/design-patterns.md)):**
> | ① **Bỏ phần được cắm vào thì phần kia còn làm được việc của nó không?** | Còn: `lib_dimming` vẫn là context nguyên vẹn, cắm base `IDimmingAlgo` (no-op) là chạy | **Không**: thuật toán tính xong mà không có backend thì **không chạm được phần cứng** — hai nửa của **một** tính năng |
>
> **Chốt:** *"Bỏ phần được cắm vào đi: nếu phần kia vẫn là một thứ hoàn chỉnh thì đó là Strategy; nếu nó thành một nửa không dùng được thì đó là Bridge."*

</details>

---

### Câu 12 · `RES-034` 🇬🇧 · Ⓑ · 🟠 · **2/4** · ĐÓNG GÓI

**"Here's the whiteboard. Draw the architecture of your library while you explain it — keep it to about a minute."**

**Follow-up (EN):** ① What sits above that C API? Who calls it? ② App and library are both C++ — why not expose the C++ classes directly?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
```
C API --> (Picture quality) --> Panel control --> IOCTL --> HAL driver
```
*"Some one-command functions go directly from C API to Panel control without going through any picture quality logic: power on/off, set resolution, set frame rate."*
- ① *"The C++ interface calls it; the interface is the gateway for apps above."*
- ② Simple API is enough — no class, no virtual ⟹ fewer layout mismatches; C API has a better ABI; **narrow waist**: simple stable C API outside, free OOP C++ inside; the library changes continuously while the layers above stay stable.

**✅ Được:** sơ đồ có hai đường (qua PQ / thẳng tới Panel Control) — đúng ý *"tính toán vs ra lệnh"* ✅. ② tiếng Anh trôi, đủ ý ABI + narrow waist + bên trong đổi tự do ✅.

**❌ Vì sao 2 — tiêu chí của weak-register: *"đủ ba ranh giới ngay lượt đầu"*:**
| Ranh giới | Lượt đầu |
|---|---|
| App → **C++ interface** | ❌ không có — chỉ ra khi bị hỏi |
| C++ interface → **C API** | ✅ (là điểm bắt đầu của sơ đồ) |
| Library → **`ioctl`** → kernel | ✅ — nhưng kernel gọi là *"HAL driver"*; không nhắc bảng con trỏ hàm |

Và cả lượt đầu lẫn ② đều **thiếu lý do khoá**: *"each C function is the single place we take the lock, because the shared state lives in shared memory"*.

**Đáp án — bản 30″ tiếng Anh, bám [A1 §10.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) ([bank RES-034](../bank/resume.md)):**
> *"It's a shared library that controls the display, used across product lines and more than ten chip families. Three boundaries: apps call a **C++ interface**; that interface calls into the library through a **plain C API**; the library talks to the **panel driver in the kernel** through `ioctl`. Inside, **Picture Quality computes** the brightness from the content, and **Panel Control issues the commands**. That's the 30-second version — which part would you like me to open up?"*

</details>

---

### Câu 13 · `LNX-045` · Ⓖ · 🟠 · **3/4**

**Library của em được nạp vào nhiều process; mọi hàm `lib_api_*` đều bọc giữa `sem_wait`/`sem_post` trên một named semaphore. Một tool test bị `kill -9` đúng lúc đang ở trong một hàm API. Các process còn lại ra sao? Vì sao không có `EOWNERDEAD` như robust mutex? Em xử lý thế nào?**

**Follow-up:** ① chủ khoá **không chết** mà chỉ chậm 8 giây — luật 7 giây làm gì? giây thứ 8 nó `sem_post` thì semaphore bằng mấy? ② bắt buộc giữ semaphore — sửa luật 7 giây thế nào?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** các process khác gọi API bị treo vì sem đã bị lock mà chủ đã chết; sau 7 s, API kẹt tự reset sem và chạy tiếp. Không có `EOWNERDEAD` vì **sem không có ownership — kernel không biết ai sở hữu, đã chết hay chưa** ⟹ API phải tự phát hiện và tự cứu.
- ① B chờ 7 s rồi reset và chạy; A chậm xong ở giây 8 thì post ⟹ **sem = 2 ⟹ hỏng vĩnh viễn**.
- ② Kèm **kiểm PID** với luật 7 s: chỉ reset khi chủ **thật sự chết**; *"còn không thì cho API hiện tại đi tiếp mà không reset sem"*. Nhược: API kẹt lâu thì mọi API sau đều trễ 7 s, và rủi ro race vì không có sem bảo vệ.

**✅ Được:** cả ba vế của đề ✅ — vế *"vì sao không `EOWNERDEAD`"* trắng hôm qua, nay đúng · ① đúng cả giá trị 2 lẫn hệ quả ✅ · ② đúng hướng PID ✅.

**❌ Vì sao chưa 4:** ② chọn sai nhánh khi chủ **còn sống**: *"cho API đi tiếp không khoá"* chính là tạo race mà mình vừa chữa. Đúng là **không vào** — chờ tiếp hoặc trả lỗi (`-EBUSY`) + log PID đang giữ (đã có lịch sử người gọi trong shm). Chưa nhắc cờ `dirty` cho state khi reset.

**Đáp án ([bank LNX-046](../bank/linux-sysprog.md), bậc ①):**
> | ① Giữ semaphore | Ngay sau `sem_wait`, ghi **PID chủ** vào shm. Quá hạn thì kiểm PID đó: **đã chết** mới reset; **còn sống** thì log (đã có lịch sử người gọi) và chờ tiếp / trả lỗi | Cờ `dirty` bật trước khi ghi, tắt sau khi ghi. Reset mà gặp `dirty` ⟹ nạp lại state từ driver (phần cứng là nguồn thật) hoặc về mặc định |

</details>

---

### Câu 14 · `LNX-046` · Ⓖ · 🟠 · **2/4** · KIẾN THỨC

**Library của em khoá liên process bằng semaphore chứ không phải mutex. Vì sao chọn vậy, và mất gì? Hệ bù bằng luật *"giữ khoá quá 7 giây thì reset semaphore để API khác chạy tiếp"* — luật đó hỏng ở đâu? Nếu được sửa, em làm gì với cả khoá lẫn state đang ghi dở?**

**Follow-up:** ① `pthread_mutex_t` bình thường trong shm có tự thu hồi khi chủ chết không, hay cần điều kiện gì? ② làm sao để process chết giữa chừng **không bao giờ** để lại tổ hợp `mode` mới + `backlight` cũ?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** *"Kiến trúc dùng nhiều năm nên em không rõ ý đồ, dựa trên kiến thức em suy đoán."* Được: sem đơn giản (init = 1, wait/post); không ownership nên API kẹt tự reset được, hệ chạy tiếp dù có rủi ro sem = 2. Mất: an toàn của mutex (ai lock người đó unlock); **tự thu hồi nếu chủ chết**; priority inheritance. Luật hỏng khi chủ chỉ chạy lâu ⟹ post ⟹ hỏng sem. Giải pháp như câu 13.
- ① Không. *"Mặc định mỗi process định danh nó khác nhau… cần bật SHARED."*
- ② *"Chưa rõ."*

**✅ Được:** hai lý do chọn ✅ · ba thứ mất, có **priority inheritance** (phần 1 thiếu ý này ở `OS-007`) ✅ · lỗ *chậm ≠ chết* ✅ · ① đúng là cần `PROCESS_SHARED`, và trực giác *"mỗi process một hàng đợi"* **đúng** với mutex private:
```
mac dinh (private)   B: timedlock -> Connection timed out sau 3.0 s
PROCESS_SHARED       B: timedlock -> OK                   sau 1.0 s
```
(A nhả khoá sau 1 giây; mutex private thì B không bao giờ được đánh thức.)

**❌ Vì sao 2:**
- *"Mutex tự thu hồi nếu chủ chết"* — chỉ đúng với **robust** mutex. ① nói `PROCESS_SHARED` nhưng không nói `ROBUST` — hai thuộc tính giải hai việc khác nhau.
- ② **trắng** — ghi kiểu commit là nửa *"state"* của đề, và đã có trong review phần 1.
- Mở đầu bằng *"em không rõ ý đồ"*: thật thà là tốt, nhưng ở phỏng vấn nên đổi thành *"theo em hiểu, có ba lý do…"* rồi nói như người hiểu đánh đổi (bẫy 5 của bank).

**Đáp án ([bank LNX-046](../bank/linux-sysprog.md)):**
> | ② ⭐ Robust mutex trong shm | `PTHREAD_PROCESS_SHARED` + `PTHREAD_MUTEX_ROBUST` ⟹ **chỉ** khi chủ thật sự chết mới trả `EOWNERDEAD`; chủ chậm thì cứ chờ | **Ghi kiểu commit:** chép bản đang dùng sang bản nháp → ghi vào nháp → đổi chỉ số bằng **một** phép ghi. Chết trước bước cuối ⟹ bản cũ còn nguyên |
>
> | `PTHREAD_PROCESS_SHARED` | … **hàng đợi ngủ là của riêng từng process** ⟹ bên chờ ở process khác **không bao giờ được đánh thức** khi chủ nhả khoá |
> | `PTHREAD_MUTEX_ROBUST` | Chủ chết khi đang giữ ⟹ mutex kẹt y như semaphore. Mutex thường **không** tự thu hồi; chỉ robust mutex mới trả `EOWNERDEAD` |

**Tài liệu gốc ([ipc-linux §4.3](../../../04-linux-system-programming/ipc-linux.md)):**
> **Cách làm cho `repair_shared_state()` gần như không phải làm gì — ghi kiểu commit:** giữ **hai bản** state và một chỉ số `active`. Muốn ghi: chép bản đang dùng sang bản nháp → sửa bản nháp → đổi `active` bằng **một** phép ghi.

</details>

---

## 🎯 Ba lỗ hổng ưu tiên

1. **Bản kể kiến trúc — cả Việt lẫn Anh — phải bắt đầu từ app.** Ba ranh giới: C++ interface · API C · `ioctl`, cộng câu *"the single place we take the lock"*. Hai phiên liền hụt cùng chỗ (`RES-035`, `RES-034`). Luyện: nói to bản 30″ tiếng Anh ở trên tới khi thuộc.
2. **Từ "hiểu vấn đề" sang "viết được cách chữa":** dòng `HAS_OP` (`DP-043`), ghi kiểu commit (`LNX-046`), `if (copy_from_user(...)) return -EFAULT;` (`DRV-006`). Mỗi thứ chép tay một lần.
3. **Mutex liên process cần hai thuộc tính:** `PROCESS_SHARED` (để gặp nhau ở cùng hàng đợi) + `ROBUST` (để biết chủ chết). Không trộn hai việc.

---

## 📦 Mã nguồn thí nghiệm

<details><summary><code>mtx.c</code> — mutex trong shm, có và không có <code>PTHREAD_PROCESS_SHARED</code></summary>

```c
// Mutex trong shm, KHONG bat PTHREAD_PROCESS_SHARED: A giu 1 s roi unlock; B cho toi da 3 s.
#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static void run(int pshared) {
    pthread_mutex_t *m = mmap(NULL, sizeof *m, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pthread_mutexattr_t a; pthread_mutexattr_init(&a);
    if (pshared) pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(m, &a);
    pthread_mutex_lock(m);                                   // A (cha) giu khoa
    if (fork() == 0) {                                       // B (con)
        struct timespec t0, t1, dl; clock_gettime(CLOCK_REALTIME, &t0); dl = t0; dl.tv_sec += 3;
        int rc = pthread_mutex_timedlock(m, &dl);
        clock_gettime(CLOCK_REALTIME, &t1);
        printf("%-20s B: timedlock -> %-20s sau %.1f s\n", pshared ? "PROCESS_SHARED" : "mac dinh (private)",
               rc ? strerror(rc) : "OK", (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9);
        _exit(0);
    }
    sleep(1);
    pthread_mutex_unlock(m);                                 // A nha khoa sau 1 s
    wait(NULL);
}
int main(void) { setvbuf(stdout, NULL, _IONBF, 0); run(0); run(1); return 0; }
```
Build: `gcc -Wall -Wextra mtx.c -o mtx -pthread && ./mtx` (gcc 11.4, Linux 6.8, 0 warning)
</details>

<details><summary><code>ops.c</code> — driver nền kiểm <code>size</code> trước khi gọi slot mới</summary>

```c
/* panel_ops: driver nen kiem 'size' truoc khi goi slot moi them o cuoi */
#include <stddef.h>
#include <stdio.h>
#define ENOTSUP 95
struct panel_ops_old { size_t size; int (*init)(void); int (*set_brightness)(int); };
struct panel_ops     { size_t size; int (*init)(void); int (*set_brightness)(int); int (*get_temp)(int *out); };

/* slot f co mat trong bang ben kia dien KHONG, va da duoc dien chua */
#define HAS_OP(ops, f) ((ops)->size >= offsetof(struct panel_ops, f) + sizeof((ops)->f) && (ops)->f)

static int core_get_temp(const struct panel_ops *ops, int *t) {
    if (!HAS_OP(ops, get_temp)) return -ENOTSUP;    /* bang cu, hoac slot chua dien */
    return ops->get_temp(t);
}
static int ok(void) { return 0; }
static int sb(int v) { return v; }
static int gt(int *o) { *o = 42; return 0; }
int main(void) {
    static struct panel_ops_old old_ko = { sizeof old_ko, ok, sb };       /* .ko cu dien bang ngan */
    static struct panel_ops     new_ko = { sizeof new_ko, ok, sb, gt };
    int t = 0, rc;
    rc = core_get_temp((const struct panel_ops *)&old_ko, &t);
    printf(".ko cu : size=%zu -> get_temp tra %d\n", old_ko.size, rc);
    rc = core_get_temp(&new_ko, &t);
    printf(".ko moi: size=%zu -> get_temp tra %d, t=%d\n", new_ko.size, rc, t);
    return 0;
}
```
Build: `gcc -Wall -Wextra ops.c -o ops && ./ops` (0 warning)
</details>
