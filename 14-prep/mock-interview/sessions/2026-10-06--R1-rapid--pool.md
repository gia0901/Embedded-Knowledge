# Phiên mock — 2026-10-06 · **R1-rapid** · quét pool R1 · [resume-plan](../../study-plans/resume-plan.md)

- **Level:** mid-level · **Type:** `rapid` quét phủ ([interview-types](../interview-types.md)) — chỉ chấm **lõi T1**, thang rapid ([config §4](../config.md)) · **Số câu:** 7 (6 chấm, 1 bỏ) · mọi câu **Ⓖ**
- **Điểm:** **15/24 = 2.50**
- **Bối cảnh:** chạy ngay sau [R1 mock chính](2026-10-06--R1--kien-truc.md) (2.33).

---

## Kết quả từng câu

| # | ID | Câu | Điểm | Loại lỗ hổng |
|---|---|---|:-:|---|
| 1 | `RES-007` | Dimming / FRC / TCON cho người ngoài ngành | **2** | DIỄN ĐẠT + KIẾN THỨC (TCON) |
| 2 | `DP-038` | Bridge giải gì, khác Strategy chỗ nào | **2** | KIẾN THỨC — thuộc chữ, chưa hiểu |
| 3 | `DP-037` | Factory Method vs Abstract Factory | **3** | — |
| 4 | `DP-041` | 5 process — mấy object, mấy state, Singleton không, init vì sao không race | **2** | KIẾN THỨC |
| 5 | `SD-022` | Vì sao phơi C API + quy tắc ở biên giới C | **2** | KIẾN THỨC |
| 6 | `RES-003` | Bảng con trỏ hàm trong kernel ≈ gì của C++ | **4** | — |
| 7 | `DP-021` | Dimming vs video enhancement — hai pattern | — | 🚫 bỏ — ngoài phạm vi |

### 🔎 Chẩn đoán

Cùng mẫu với R1: **định nghĩa nói ra được, áp vào hệ của chính mình thì hụt.** Rõ nhất ở `DP-038`: đọc lại đúng câu bank *"thứ được cắm vào là nửa còn lại của một thứ"*, nhưng tự đánh dấu *"(???)"* và không chỉ ra được nửa đó trong dimming. `RES-003` (4 điểm) cho thấy ngược lại: chỗ nào **đã tự làm** thì nói gọn và đúng ngay.

### ⚖️ Ba góp ý của người học — phân xử

| Góp ý | Phân xử | Đã làm |
|---|---|---|
| `RES-007`: chiến lược kể đã đổi sang khung PQ + DC | **Nửa đồng ý.** Resume ghi nguyên văn *"dimming, frame-rate and timing control"* nên câu này vẫn bị hỏi đúng các từ đó. Nhưng đáp án nên **nối** vào khung PQ/DC | Thêm câu nối vào [bank RES-007](../bank/resume.md) |
| `DP-038`: đọc bank không hiểu, không có ví dụ | ✅ **Đồng ý.** Lần thứ hai trong hai ngày (sau `DP-005`/`DP-006`) | Nâng cấp [bank DP-038](../bank/design-patterns.md): ví dụ dimming + hai phép thử |
| `DP-021`: không làm video enhancement | ✅ **Đồng ý, không chấm.** Ngoài bán kính resume | Gỡ khỏi plan; ghi chú ở [bank DP-021](../bank/design-patterns.md) |

---

## 🔎 Chi tiết từng câu

### Câu 1 · `RES-007` · Ⓖ · 🟢 · **2/4** · DIỄN ĐẠT + KIẾN THỨC

**Dimming, FRC, TCON — em giải thích ngắn gọn cho người ngoài ngành hiểu được không?**

**🔁 Probe:** *"TCON đứng giữa cái gì và cái gì? Nói bằng một hình ảnh mà người ngoài ngành hình dung được."*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- *(Góp ý: chiến lược đã đổi sang PQ + DC.)*
- Dimming: điều khiển độ sáng backlight bằng PWM; có adaptive, dimming theo zone ✅ (⚠️ "PWM" là thuật ngữ trần với người ngoài ngành)
- FRC: đổi frame rate 60/120/165 Hz, nội suy film 24 Hz, giảm judder/blur ✅ (⚠️ "judder" — thuật ngữ)
- TCON: *"đảm bảo panel hoạt động đúng với nội dung xuất ra (cùng tần số, cùng resolution)"* ⚠️ mơ hồ
- Probe: *"IC điều khiển nằm trên panel, chịu trách nhiệm xuất nội dung chính xác; SoC điều khiển qua i2c hoặc thanh ghi"* ❌ vẫn chưa ra vai trò

**✅ Được:** dimming và FRC đúng ý chính.

**❌ Vì sao 2:** thang rapid — **thiếu một phần, gợi vẫn không ra**. TCON là một phần ba câu hỏi, và sau probe vẫn chỉ nói TCON **nằm ở đâu, ai điều khiển nó**, không nói nó **làm gì**. Câu này đo khả năng nói cho **người ngoài ngành**, mà PWM / judder / resolution là thuật ngữ trần. Bank còn chấm *"nói mình làm phần nào"* — chưa có.

**Đáp án ([bank RES-007](../bank/resume.md)):**
> - **TCON** (Timing Controller) — con chip nhận tín hiệu ảnh rồi phát đúng **thời điểm** cho từng hàng/cột điểm ảnh trên panel. Nó là cầu giữa xử lý ảnh và tấm nền vật lý.
>
> **Ghi điểm thêm:** nói **bạn động vào phần nào** trong ba cái, và động ở tầng nào.
>
> **Bẫy:** ① trả lời bằng thuật ngữ chồng thuật ngữ — nghe là biết chưa hiểu.

**Câu nối sang khung PQ/DC (thêm vào bank 06/10):** *"Dimming thuộc Picture Quality — phần phải tính toán theo nội dung; FRC và TCON là các lệnh Display Control gửi xuống chip."*

**Tài liệu gốc** ([A1 §4.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)):
> **Cần tính toán, giữ state, bám theo từng khung hình ⟹ đi qua PQ. Một lệnh, không state, gửi xong là xong ⟹ đi thẳng DC.**

**Chốt:** mỗi thứ một câu đời thường; TCON là *"người phát nhịp cho từng hàng điểm ảnh"*; rồi một câu nối vào PQ/DC và nói mình làm phần nào.
</details>

---

### Câu 2 · `DP-038` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC

**Bridge pattern giải quyết vấn đề gì? Nó khác Strategy chỗ nào khi code trông giống hệt nhau?**

**🔁 Probe:** *"Lấy ngay hệ dimming của bạn: 'nửa còn lại của một thứ' cụ thể là nửa nào, và thiếu nửa đó thì phần kia còn chạy được không?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** chống bùng nổ lớp con khi có 2 chiều thay đổi độc lập; giữ một trục ở cây kế thừa, đẩy trục kia ra implementor; N + M lớp, ghép N × M lúc runtime ✅ (đúng bank, từng chữ). Khác Strategy: *"có 2 thay vì 1 trục, thứ được cắm vào là nửa còn lại của một thứ (???), thường gắn lúc dựng"*. Probe: *"chưa rõ"* ❌. *(Góp ý: đọc bank không hiểu, không có ví dụ.)*

**❌ Vì sao 2:** vế một đúng nhưng là **đọc lại bank**; vế hai chính bạn đánh dấu *"(???)"*, và không áp được vào hệ mình — đúng chỗ phân biệt hiểu với thuộc. Cùng gốc với `DP-023` ở R1.

**Đáp án ([bank DP-038](../bank/design-patterns.md), bản nâng cấp 06/10):**
> | Phép thử | `IDimmingAlgo` đối với `lib_dimming` ⟹ **Strategy** | `IDimmingBackend` đối với `GlobalDimming` ⟹ **Bridge** |
> |---|---|---|
> | ① **Bỏ phần được cắm vào thì phần kia còn làm được việc của nó không?** | Còn: `lib_dimming` vẫn là context nguyên vẹn | **Không**: thuật toán tính xong mà không có backend thì **không chạm được phần cứng** |
> | ② **Có mấy trục biến thiên?** | **Một** | **Hai**: thuật toán theo model, backend theo chip |
>
> **Chốt:** *"Bỏ phần được cắm vào đi: nếu phần kia vẫn là một thứ hoàn chỉnh thì đó là Strategy; nếu nó thành một nửa không dùng được thì đó là Bridge."*

**Tài liệu gốc** ([A1 §5.5](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)):
> | Vai | **Strategy** — thuật toán thay thế được theo model | **Bridge implementor** — nửa phần cứng của cùng một thứ |
> | Chạy được nếu thiếu? | Có bản base không làm gì | **Không** — thuật toán *cần* backend mới chạm được phần cứng |

⚠️ Khi kể **hệ thật**, đừng lặp *"N + M"* — backend còn nhân theo thuật toán ([DP-042](../bank/design-patterns.md), sửa 06/10).
</details>

---

### Câu 3 · `DP-037` · Ⓖ · 🟡 · **3/4**

**Factory Method và Abstract Factory khác nhau ở đâu? Dấu hiệu nào cho biết bạn cần cái thứ hai?**

**🔁 Probe:** *"Vế thứ hai của đề: nhìn vào code, dấu hiệu nào cho biết đã đến lúc cần Abstract Factory?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** Factory Method tạo một loại sản phẩm, lớp con quyết định class ✅. Abstract Factory tạo cả họ sản phẩm phải khớp nhau, nhiều `CreateX()` ✅. Probe: *"tạo một instance hoàn chỉnh cần nhiều thứ, vd Algo + Backend → Abstract Factory; chỉ một backend thì là Factory Method"* ✅.

**✅ Được:** phân biệt đúng, gọn; ví dụ đúng hệ của mình.

**❌ Vì sao chưa 4:** bỏ qua vế hai, phải nhắc mới trả lời. Câu dấu hiệu còn thiếu từ khoá quyết định: **trộn nhầm là bug**.

**Đáp án phần còn thiếu ([bank DP-037](../bank/design-patterns.md)):**
> | ≥ 2 thứ tạo ra **phải cùng một biến thể**, trộn nhầm là bug | **Abstract Factory** |
>
> Giá trị thật của Abstract Factory không phải "gom cho gọn" mà là **biến ràng buộc nhất quán họ thành ràng buộc kiểu** — chỉ có một object factory, nên không còn tham số nào để truyền sai.

Không vào weak-register (đạt 3).
</details>

---

### Câu 4 · `DP-041` · Ⓖ · 🟡 · **2/4** · KIẾN THỨC

**Library của bạn được nạp vào 5 process. Mỗi process có `get_dimming_instance()` trả về một con trỏ toàn cục. Hỏi: có bao nhiêu object dimming? Bao nhiêu bản state thật? Thứ đó là Singleton à? Và vì sao chỗ khởi tạo không có data race?**

**🔁 Probe:** *"Vế cuối hỏi chỗ khởi tạo con trỏ toàn cục trong mỗi process: nó chạy lúc nào, và vì sao lúc đó không có hai thread tranh nhau?"*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** 5 object ✅, một state trong shm ✅. *"Cả 5 đều là Singleton, 5 phiên bản độc lập"* ❌. Không race vì *"shm được semaphore bảo vệ"* ❌ (trả lời sang race của state). Probe: *"chạy ở lần gọi `get_dimming_instance` đầu tiên, từ một static pointer; chưa rõ lý do không race; nếu là Meyers thì guard bảo vệ"* ❌.

**⚖️ Đối chiếu (luật config §4 mới):** lời bạn (*"khởi tạo ở lần gọi đầu"*) khác A1. Đã đối chiếu tư liệu gốc của hệ thật: `get_dimming_instance()` **chỉ trả về** con trỏ; việc gán nằm trong hàm khởi tạo module, được gọi từ `__attribute__((constructor))`. Bạn cũng nói *"chưa rõ"* ⟹ không phải tranh chấp có bằng chứng, chấm theo bank.

**❌ Vì sao 2:** hai trong bốn vế sai.
1. **Không phải Singleton GoF**: không có gì *ép* tính duy nhất (con trỏ toàn cục, ai cũng gán được) ⟹ **Service Locator**.
2. **Không race vì thời điểm**, không phải vì khoá: khởi tạo chạy trong `__attribute__((constructor))`, khi `.so` vừa được nạp, **trước khi process tạo thread nào** ⟹ không có ai để tranh.

**Đáp án ([bank DP-041](../bank/design-patterns.md)):**
> **Gọi tên đúng:** `get_*_instance()` trên con trỏ toàn cục là **Service Locator**, không phải Singleton GoF — không gì *ép* tính duy nhất, chỉ có quy ước "chỉ hàm init gán".
>
> **Vì sao không race:** khởi tạo chạy trong `__attribute__((constructor))`, tức **trước khi process tạo thread nào** ⟹ không có check-then-act cạnh tranh. Đây là cách chữa race khác magic statics: **dời khởi tạo ra khỏi vùng có cạnh tranh**.

**Tài liệu gốc** ([A1 §7.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)):
> ⭐ **Không có data race**, khác `getInstance()` ở A2 §3.2, Lab 1 (race thật, đo được **178/200**). Lý do: khởi tạo chạy trong `__attribute__((constructor))`, **trước khi process tạo thread nào**.

**Chốt:** *"Object per-process, state per-system. Thứ trả con trỏ toàn cục là Service Locator; nó không race vì khởi tạo chạy trước khi có thread nào."*
</details>

---

### Câu 5 · `SD-022` · Ⓖ · 🟠 · **2/4** · KIẾN THỨC

**Vì sao thư viện hệ thống thường phơi **C API** dù bên trong viết bằng C++? Ở biên giới C phải tuân thủ quy tắc gì?**

**🔁 Probe:** *"Vế hai: đứng ở biên giới C, cụ thể phải tuân thủ những quy tắc gì? Kể ba điều."*

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** `.so` và app phát triển lệch thời gian; C++ ⟹ vtable xuyên biên giới; C API là điểm hẹp với ABI an toàn, bên trong vẫn C++ ✅. Probe: *"hợp đồng C API giữa app và library phải giống nhau (chung header); hai điều nữa chưa rõ"* ❌. *(Câu hỏi bổ sung: bỏ `extern "C"` thì sao — trả lời ở dưới.)*

**❌ Vì sao 2:** vế quy tắc là nửa đề; *"chung header"* đúng với **mọi** API, không riêng biên giới C.

**Đáp án ([bank SD-022](../bank/system-design.md)):**
> | **`extern "C"`** cho mọi hàm phơi ra | Tắt name mangling ⇒ symbol có tên ổn định |
> | **KHÔNG để exception thoát ra** | Ném xuyên qua biên C là **UB**. Bọc `try/catch(...)` ở **mọi** hàm phơi ra, đổi thành mã lỗi |
> | **Chỉ dùng kiểu POD & con trỏ mờ** | Không `std::string`, không `std::vector`, không lớp có vtable |
> | **Sở hữu đối xứng** | Thư viện cấp thì **thư viện giải phóng** (`foo_create`/`foo_destroy`) |
> | **Không truyền `bool`/`enum` C++ trần** | dùng kiểu số có độ rộng cố định |
> | **Struct công khai có trường `size`** | Để mở rộng sau này mà không phá ABI |

**Câu hỏi bổ sung của bạn — *"bỏ `extern "C"`, `lib_api_*` thành hàm C++ thường thì sao?"*** Đáp án đầy đủ ở [bank DP-040](../bank/design-patterns.md) (thêm 06/10). Chạy lại thật trên máy này (gcc 11.4):
```
co extern C:    0000000000001119 T lib_api_set_backlight
khong extern C: 0000000000001119 T _Z21lib_api_set_backlighti
--- .so v2 them tham so, app khong build lai ---
co extern C:
  lib v2: level=80 ramp_ms=-1598341848
  exit=0
khong extern C:
  ./app_cpp: symbol lookup error: ./app_cpp: undefined symbol: _Z21lib_api_set_backlighti
  exit=127
./v1-cpp/libdisplay.so: dlsym -> ./v1-cpp/libdisplay.so: undefined symbol: lib_api_set_backlight
```
Đọc kết quả:
1. **Caller C++ trên Linux vẫn chạy**: GCC/Clang mangling theo cùng chuẩn, tên symbol ổn định.
2. **Mất caller không phải C++**: C thuần, Python `ctypes`, `dlsym("lib_api_set_backlight")` đều không tìm thấy (dòng cuối).
3. **Bất ngờ:** đổi chữ ký mà không build lại app thì bản `extern "C"` **sai im lặng** (`ramp_ms` là rác, exit 0), còn bản mangling **chết to** (exit 127). Mangling mã hoá kiểu tham số vào tên symbol.
4. ⟹ Thứ làm ranh giới ổn định là **chỉ cho kiểu C đi qua** (và đã kiểm: `extern "C"` **không chặn** `std::string` trong chữ ký). `extern "C"` đổi lấy khả năng gọi từ mọi ngôn ngữ; giữ chữ ký C ổn định là việc của kỷ luật.

**Chốt:** *"C API ổn định về ABI, gọi được từ mọi ngôn ngữ. Đổi lại phải chặn exception ở biên, chỉ dùng POD và con trỏ mờ, giữ sở hữu đối xứng."*
</details>

---

### Câu 6 · `RES-003` · Ⓖ · 🟠 · **4/4**

**Resume ghi kernel driver *"điều phối qua một bảng con trỏ hàm"*. Kernel viết bằng C thuần — cơ chế đó hoạt động thế nào, và nó tương đương cái gì trong C++?**

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** driver HAL sở hữu bảng con trỏ hàm tương ứng các API; driver panel được insmod thì tự gắn hàm của nó vào bảng; HAL gọi qua con trỏ là tới đúng hàm của driver thật ✅. Tương đương polymorphism C++ (virtual + override) ✅.

**Nhận xét:** gọn, đúng, bật ra ngay — đúng mức 4 của thang rapid. Muốn sắc hơn nữa: gọi thẳng là *"vtable viết tay"*, và nêu `file_operations` là cùng khuôn chuẩn của kernel ([bank RES-003](../bank/resume.md)).
</details>

---

### Câu 7 · `DP-021` · 🚫 bỏ — không chấm

**Trong library display của bạn: dimming chia thành global / local / oled (cùng xuất phát từ một class gốc), còn video enhancement chia theo loại chip (cũng kế thừa từ một class gốc). Nhìn qua thì cả hai đều là "kế thừa + đa hình". Vì sao đây lại là HAI pattern khác nhau — và là hai cái nào?**

<details><summary>Phân xử</summary>

Người học: *"video enhancement không nên được đề cập vì tôi không làm phần này"*, kèm trả lời ngắn (*"VE đi theo chip + backend, không có nhiều thuật toán như dimming"*). ✅ Đồng ý: câu nằm ngoài bán kính resume. **Không chấm, không vào weak-register.** Đã gỡ khỏi plan và ghi chú ở bank. Ý muốn kiểm tra (Strategy vs Abstract Factory) đã được hỏi qua `DP-022` (R1) và `DP-037` (câu 3 ở trên).
</details>

---

## Tổng kết

**Điểm mạnh:** chỗ nào đã tự làm thì nói gọn, đúng ngay (`RES-003` 4, ví dụ Algo + Backend ở `DP-037`).

**3 lỗ hổng ưu tiên:**

| # | Lỗ hổng | Ôn ở đâu |
|---|---|---|
| 1 | **Bridge vs Strategy** — thuộc chữ, không áp được vào hệ mình (lặp từ `DP-023` ở R1) | [bank DP-038](../bank/design-patterns.md) bản mới · [A1 §5.5](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) |
| 2 | **Khởi tạo không race nhờ thời điểm** (`__attribute__((constructor))`) và tên đúng *Service Locator* | [A1 §7.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md) · đối chiếu [A2 §3.2](../../../11-design-patterns/in-practice/A2-cpp-interface-hal.md) |
| 3 | **Quy tắc ở biên giới C** | [bank SD-022](../bank/system-design.md) · phần *"bỏ `extern "C"`"* ở [DP-040](../bank/design-patterns.md) |

**Phiên kế:** R2 mock chính — việc chuẩn bị ở [plan §📍](../../study-plans/resume-plan.md).
