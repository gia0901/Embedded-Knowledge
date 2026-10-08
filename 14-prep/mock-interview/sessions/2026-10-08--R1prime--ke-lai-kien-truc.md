# Phiên mock — 2026-10-08 · **R1′ phần 1** · kể lại kiến trúc theo chiến lược hiện tại · [resume-plan](../../study-plans/resume-plan.md)

- **Level:** mid-level · **Type:** mock chính của buổi R1′ ([plan §11](../../study-plans/resume-plan.md#11-r1--kể-kiến-trúc-theo-chiến-lược-hiện-tại)) · **Trần:** T2, theo luật ⑤ của plan chủ yếu T1 + một chút T2
- **Số câu:** 7/12 — người học **dừng phiên** sau câu 7; câu 8–12 dời sang **R1′ phần 2**
- **Điểm:** **17/28 = 2.43**
- **Bối cảnh:** buổi đầu sau khi đổi chiến lược kể (Picture Quality + Panel Control, không FRC/TCON, kể ngắn trước, mỗi câu thả móc vào nền tảng).

---

## Kết quả từng câu

| # | ID | Câu | Dạng | Điểm | Loại lỗ hổng |
|---|---|---|:-:|:-:|---|
| 1 | `RES-001` | Giới thiệu + project tâm đắc | Ⓖ | **3** | ĐÓNG GÓI — còn thiếu vấn đề-bằng-hình-ảnh và kết quả đo được |
| 2 | `RES-007` | Picture quality + panel control cho người ngoài ngành | Ⓖ | **2** | DIỄN ĐẠT |
| 3 | `RES-035` | Kể kiến trúc — 30″ rồi 90″ | Ⓖ | **2** | ĐÓNG GÓI + thiếu một "vì sao" (khoá) |
| 4 | `DP-040` | Khoá trong từng method vs mặt tiền C | Ⓑ | **2** | KIẾN THỨC (named semaphore) |
| 5 | `CPP-006` | vtable/vptr | Ⓖ | **3** | — (thiếu "chỉ số ô cố định lúc compile") |
| 6 | `OS-007` | Mutex vs semaphore | Ⓖ · 🔁 retention | **3** | — (thiếu priority inheritance) |
| 7 | `LNX-045` | Process chết khi giữ semaphore của library | Ⓖ | **2** | KIẾN THỨC — vì sao kernel không nhả hộ |
| 8–12 | `DP-041` · `DRV-006` · `DP-043` · `DP-038` · `RES-034` | — | — | — | chưa hỏi, dời sang phần 2 |

### 🔎 Chẩn đoán

1. **Tiến bộ thật ở câu mở màn.** `RES-001` lần đầu đạt 3 sau 5 lần 2: **không còn câu có/không ở cuối**, kết quả đã hình dung được (*"nhiều panel chạy như một panel duy nhất, một binary chung"*), và nói rõ phần **tự quyết định** (đàm phán spec, chuyển trạng thái on/off/boot).
2. **Bản kể chưa thuộc.** Bản 30″ của `RES-035` có ba phần nhưng **thiếu API C và `ioctl`** (hai trong ba ranh giới) và **thiếu câu trao quyền**. Bản 90″ có "vì sao" của API C, nhưng **không có câu *"chỗ duy nhất lấy khoá"*** — đúng thứ `DP-040` hỏi ngay sau đó.
3. **Kiến thức hệ thật vượt tài liệu, kiến thức nền dưới nó còn mỏng.** Người học mang vào một sự thật mới (*"giữ khoá quá 7 giây thì reset semaphore"*) và tự thấy rủi ro state ghi dở. Nhưng hai chỗ nền hụt: tưởng mỗi process thấy một semaphore "khác tên" (sai — cùng tên là cùng một semaphore), và không nối được *"không có chủ"* (vừa nói đúng ở câu 6) sang *"kernel không nhả hộ"* ở câu 7.

### ⚖️ Góp ý của người học — xử lý

| Góp ý | Xử lý |
|---|---|
| Câu 7: *"Đây là vấn đề thiết kế thật. Giúp tôi trả lời câu vì sao dùng sem thay vì mutex thật toàn diện, và cách khắc phục race như ý 2"* | ✅ Thêm câu mới [**`LNX-046`**](../bank/linux-sysprog.md) — ba lý do chọn semaphore · bốn thứ mất · hai lỗ của timeout-reset · ba bậc sửa · bản nói 45″. Hai thí nghiệm chạy thật ở [cuối log](#-mã-nguồn-thí-nghiệm). Ghi sự thật *"timeout ~7 giây"* vào [A1 §7.2](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md); thêm *ghi kiểu commit* và cảnh báo timeout-reset vào [ipc-linux §4.3](../../../04-linux-system-programming/ipc-linux.md) |

---

## 🔎 Chi tiết từng câu

### Câu 1 · `RES-001` · Ⓖ · 🟡 · **3/4**

**"Em giới thiệu qua về công việc hiện tại và một project em tâm đắc nhất."**

**Follow-up:** ① không đồng bộ thì người đứng trước tường màn hình **nhìn thấy gì**? ② "mượt như một panel" — **kiểm bằng gì**, đo được gì? ③ phần nào **em tự quyết định**?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** 3 năm system software cho picture quality enhancement trên smart TV và enterprise; trải từ C++ interface, shared library tới kernel HAL driver. Project tâm đắc: S-Box signage — adaptive brightness theo ánh sáng môi trường, đồng bộ nhiều box trên nhiều panel. Kết quả: nhiều box, nhiều panel vẫn chạy mượt như một panel duy nhất, mọi box dùng chung một binary.
- ① *"Mỗi panel tự chỉnh riêng ⟹ mức sáng khác nhau, thời gian từng step không đồng nhất."*
- ② Một box làm master gắn cảm biến, đưa giá trị chuẩn vào message queue, app phân phối qua ethernet; mỗi thiết bị chạy 60 Hz (16.67 ms) nên lệch giữa các box không thấy bằng mắt.
- ③ Đàm phán lại spec; tự thiết kế chuyển trạng thái on→off, off→on, và boot khi tính năng đã bật từ trước. Phần còn lại theo spec.

**✅ Được (ba tiêu chí của weak-register):**
- **Không có câu có/không ở cuối** — lần đầu trong 6 lần.
- Kết quả **hình dung được**: *"như một panel duy nhất"* + *"một binary"*.
- ③ rất tốt: chuyển trạng thái khi bật/tắt/boot là chỗ dễ có bug và là quyết định của chính bạn — đúng loại chi tiết interviewer muốn đào.

**❌ Vì sao chưa 4:**
- **Câu 2 vẫn là giải pháp, chưa là vấn đề.** Vấn đề chỉ hiện ra khi bị hỏi ở ①, và ① vẫn là mô tả kỹ thuật (*"mức sáng khác nhau"*) chứ chưa là hình ảnh (*"nhìn ra từng ô"*).
- ② trả lời **cơ chế**, không trả lời **cách kiểm**. Câu hỏi là *"đo/nhìn được gì"*; đáp án cần một thứ như *"đặt các box cạnh nhau, đổi ánh sáng phòng, các ô đổi sáng cùng lúc, cùng mức"* hoặc một con số.

**Đáp án ([bank RES-001](../bank/resume.md), bản A):**
> *"Bài toán là nhiều màn ghép lại thành một màn lớn, mà mỗi máy tự chỉnh sáng theo cảm biến riêng của nó thì chúng lệch nhau — nhìn ra từng ô rõ rệt."*
>
> | 2 | 🔴 **VẤN ĐỀ trước, bằng hình ảnh** — *"nhìn ra từng ô"* | **Lỗi lặp #1:** ba lần đều nhảy thẳng vào công nghệ |
> | 4 | 🔴 **Kết quả có thể HÌNH DUNG hoặc ĐO** | **Lỗi lặp #2:** ba lần đóng câu **không có gì cụ thể** |

</details>

---

### Câu 2 · `RES-007` · Ⓖ · 🟢 · **2/4** · DIỄN ĐẠT

**Picture quality (dimming) và panel control — em giải thích ngắn gọn cho người ngoài ngành hiểu được không?**

**Follow-up:** ① người bán hàng hỏi *"dimming là gì, sao TV cần nó"* — trả lời trong 1–2 câu, không thuật ngữ. ② vì sao library tách **hai phần**, không để mỗi tính năng tự ghi xuống phần cứng?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** PQ gồm nhiều mảng — điều khiển độ sáng và hiệu ứng sáng (dimming, phần em làm), video-enhancer cải tiến màu, ambient cho chế độ tĩnh. Panel control là phương tiện thực thi điều khiển lên panel; các tính năng PQ tính xong thì dùng panel control ghi giá trị cuối; lệnh đơn (resolution, framerate, power) thì API gọi thẳng panel control.
- ① *"Độ sáng đèn nền là nền tảng của chất lượng hình ảnh ⟹ nội dung đủ sáng, đủ chi tiết theo vùng (local dimming); vai trò cho hiệu ứng chuyển tiếp: home → youtube/netflix/pc source."*
- ② *"Lệnh đơn đi thẳng qua panel control, dùng logic là không cần. Dimming nặng thuật toán, khác nhau trên nhiều model ⟹ cần PQ tính ra giá trị cuối rồi mới đưa xuống panel control."*

**✅ Được:** nói rõ **mình làm phần nào** (dimming). ② **đúng và gọn** — chính là câu *"một bên tính toán, một bên ra lệnh"*.

**❌ Vì sao 2:** câu này đo **nói cho người ngoài ngành**, và lượt đầu lẫn ① đều chưa đạt:
- Lượt đầu mở bằng **danh sách thuật ngữ** (video-enhancer, ambient), không phải hai câu hỏi về một tấm panel. Định nghĩa panel control (*"phương tiện thực thi các điều khiển"*) còn trừu tượng.
- ① vẫn dùng thuật ngữ (*local dimming*) và **không nói dimming làm gì cho người xem**: hạ đèn ở vùng tối ⟹ đen sâu hơn, tốn ít điện hơn.
- Câu nối sang kiến trúc (② ) chỉ ra khi bị hỏi; bank muốn nó nằm ở lượt đầu.

**Đáp án ([bank RES-007](../bank/resume.md)):**
> - **Picture quality — "sáng bao nhiêu?"**: *"đằng sau tấm panel là dàn đèn LED; dimming hạ đèn ở vùng ảnh tối — đen sâu hơn, tốn ít điện hơn. Phần này phải **tính toán** theo nội dung từng khung hình."*
> - **Panel control — "hiển thị thế nào?"**: *"bật tắt panel, đổi độ phân giải, đổi số hình mỗi giây, ghi độ sáng đã tính xuống phần cứng. Phần này chỉ là **ra lệnh**, xuống **panel driver** trong kernel."*
> - **Câu nối sang kiến trúc:** *"vì một bên là tính toán, một bên là ra lệnh, nên trong library em tách làm hai component — đó cũng là hai nhánh của kiến trúc."*

**Tài liệu gốc ([whiteboard D0](../../whiteboard.md)):** *"Ý tưởng cốt lõi: đừng kể tên tính năng — kể HAI CÂU HỎI về cùng một tấm panel."*

</details>

---

### Câu 3 · `RES-035` · Ⓖ · 🟡 · **2/4** · ĐÓNG GÓI

**"Kể anh nghe kiến trúc phần library em làm."** — bản 30″, rồi *"kể thêm"* sang bản 90″.

**Follow-up:** ở dưới `ioctl`, phần driver chung trong kernel gọi đúng code từng chip bằng cách nào mà không cần `if (chip == …)`?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời (30″):** ba phần — ① C++ interface để app/service thay đổi picture quality · ② library thực thi lời gọi qua hai phần picture quality và panel control · ③ HAL driver thực thi trên phần cứng thật, điều khiển trực tiếp một số hw (GPIO nguồn panel) và gửi lệnh đi nơi khác (SoC, dimming board).

**(90″):** ① giữa interface và library là API C + `.so` hẹp: interface ổn định, thuần tính năng, không cần biết model; library đổi liên tục ⟹ hàm C giúp kiểm soát tương thích, thay `.so` mà không build lại app. ② library ↔ driver dùng `ioctl` — hợp với lệnh đơn kèm biến/struct đơn.

**Follow-up:** kernel gồm HAL driver + panel driver, liên kết qua function pointer table tạo đa hình như C++ ⟹ gọi qua HAL là tự tới đúng chỗ.

**✅ Được:** không FRC/TCON; có PQ/Panel Control; "vì sao" của API C đúng ý (bên build lệch thời gian, thay `.so` không build lại app); bảng con trỏ hàm đúng.

**❌ Vì sao 2 — chấm theo ba tiêu chí của bank:**
| Tiêu chí | Kết quả |
|---|---|
| Đủ **ba ranh giới** ngay bản 30″ | ❌ Bản 30″ kể **ba khối**, không kể **ba ranh giới**: thiếu *API C* (giữa interface và library) và *`ioctl`* (giữa library và kernel) |
| Câu trao quyền ở cuối | ❌ Không có |
| Không FRC/TCON | ✅ |
| Ba "vì sao" ở bản 90″ | 1/3 tự nói (API C) · 1/3 sau probe (bảng con trỏ hàm) · **thiếu** *"mỗi hàm C là chỗ duy nhất lấy khoá vì state nằm trong shared memory"* |

"Vì sao" của `ioctl` (*"hợp lệnh đơn"*) không phải lý do của ranh giới — ranh giới thật là **user ↔ kernel**, và `ioctl` là cửa chuẩn để truyền lệnh kèm struct qua đó.

**Đáp án ([A1 §10.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)) — thuộc nguyên văn:**
> *"Em làm một shared library điều khiển màn hình, dùng chung cho nhiều dòng sản phẩm và hơn mười dòng chip. Em kể theo ba ranh giới: **app gọi một C++ interface**; interface đó gọi xuống library qua **một API C**; library ra lệnh xuống **panel driver trong kernel** bằng `ioctl`. Trong library có hai phần: **Picture Quality tính độ sáng** theo nội dung hình, **Panel Control ra lệnh** cho panel. Đó là bản 30 giây — phần nào anh muốn em mở ra thì em mở."*

**Câu bị thiếu ở bản 90″ ([A1 §10.2](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)):**
> *"Mỗi hàm C đó còn là **chỗ duy nhất lấy khoá**, vì state dùng chung giữa các process nằm trong **shared memory**."*

</details>

---

### Câu 4 · `DP-040` · Ⓑ · 🟠 · **2/4** · KIẾN THỨC

Đồng nghiệp đề xuất bỏ lớp API C, phơi thẳng một class C++; state vẫn trong shared memory, mỗi method tự khoá:

```cpp
class Display {
public:
    void set_backlight(int level) {
        sem_wait(sem_);              // named semaphore, dùng chung giữa các process
        shm_->backlight = level;
        apply_to_driver();           // ioctl xuống kernel
        sem_post(sem_);
    }

    void apply_preset(const Preset& p) {
        sem_wait(sem_);
        shm_->mode = p.mode;
        set_backlight(p.backlight);  // tái dùng method sẵn có
        sem_post(sem_);
    }

    void set_framerate(int hz);      // method do người khác thêm vào năm sau

private:
    sem_t*      sem_;
    SharedState* shm_;
    void apply_to_driver();
};
```

**(a)** App gọi `apply_preset(...)` — chuyện gì xảy ra? **(b)** Năm sau có người thêm `set_framerate()` — **cái gì bắt buộc** họ phải nhớ khoá? Khác thiết kế API C hiện tại chỗ nào?

**Follow-up:** ① `sem_` lấy từ `sem_open("/display_lock", ...)`; 5 process, mỗi process một object `Display` — có **bao nhiêu semaphore thật**? ② trong hệ thật, các class bên trong library có tự khoá không? `lib_api_apply_preset` gọi xuống nhiều class mà vì sao không deadlock?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) Deadlock — sem bị lấy hai lần. *"Đây là tự vi phạm quy tắc viết API — dùng C API vẫn có thể mắc lỗi này."*
- (b) Không có ràng buộc cụ thể, phải tự tuân thủ. Khác: method gọi method ⟹ deadlock; *"biến sem của riêng class không có tác dụng trong multi-process vì mỗi process nắm một object khác nhau ⟹ race"*; nếu dùng sem trong shm như C API thì chỉ khác ở chỗ bọc class dễ thiếu sót hơn.
- ① *"Một sem trong hệ thống — nhưng được định danh 5 tên khác nhau ở năm process, chúng không nhận diện ra nhau để tự tránh — lock nhiều lần có thể kẹt mãi mãi."*
- ② *"Mọi lời gọi `lib_api` đều phải đi qua sem này; ai cầm trước thì người sau chờ. An toàn ngay từ `lib_api`, không cần lock class ở dưới."*

**✅ Được:** (a) đúng — tự deadlock. (b) đúng ý *"chỉ có kỷ luật ép"*. ② đúng **lý do 2**: khoá một lần ở mặt tiền, class bên trong không khoá nên gọi nhau tự do.

**❌ Vì sao 2:**
- **Sai về named semaphore** (cả ở (b) lẫn ①). Cùng tên ⟹ **cùng một** semaphore trong kernel, mọi process đều thấy. Con trỏ `sem_` mỗi process một bản, nhưng nó trỏ vào cùng một thứ. Deadlock ở (a) **không** đến từ "không nhận diện ra nhau", mà vì **cùng một process** xin lại khoá nó đang giữ, và semaphore không có khái niệm "khoá này của tôi rồi". Chạy thật — process B mở lại bằng tên, thấy đúng giá trị A để lại:
  ```
  A: da giu khoa (dia chi sem_t trong A = 0x710596fe0000)
  B: gia tri semaphore B thay = 0 (dia chi trong B = 0x710596fe0000)
  Sau khi A chet: gia tri = 0
  ```
- *"Dùng C API vẫn có thể mắc lỗi này"* — nửa đúng. Trong thiết kế hiện tại, hàm C **không gọi hàm C khác**; nó gọi xuống các class **không khoá**. Cấu trúc làm lỗi này **khó viết ra**, không chỉ dựa vào kỷ luật. Bạn nói được điều này ở ②, nhưng chỉ sau khi bị hỏi.
- Ngoại lệ vsync (đường chạm state không qua mặt tiền) chưa được hỏi lần này.

**Đáp án ([bank DP-040](../bank/design-patterns.md), phản biện *"khoá trong từng method"*):**
> | # | Khoá trong từng method | Mặt tiền C |
> |---|---|---|
> | 1 | **Ai ép?** Kỷ luật. Method thứ 151 quên khoá vẫn **compile sạch** | Object C++ **không ai bên ngoài với tới được** ⟹ không có đường nào đi vòng qua khoá |
> | 2 | **Method gọi method** ⟹ xin lại chính khoá đang giữ ⟹ **tự deadlock**, vì semaphore không recursive | Khoá một lần ở mặt tiền; bên trong gọi nhau tự do |

> 📌 Hệ thật đặt `sem_t` trong struct shm ([A1 §7.2](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)); dù tạo bằng tên (`sem_open`) hay `sem_init(pshared=1)` trong shm thì kết luận không đổi: **một** semaphore cho cả hệ, **không có chủ**.

</details>

---

### Câu 5 · `CPP-006` · Ⓖ · 🟡 · **3/4**

**Đa hình runtime hoạt động thế nào (vtable/vptr)?**

**Follow-up** (với `IDimmingAlgo` / `GlobalDimming { int last_level; }`): ① `vptr` ở đâu — mỗi object hay mỗi class? vtable? `sizeof(GlobalDimming)` trên 64-bit? ② tại `algo->compute(frame)`, compiler sinh những bước gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** base có method virtual ⟹ compiler tạo entry trong vtable; derived "kế thừa vtable và override"; vptr là phương tiện tới vtable; con trỏ base trỏ derived ⟹ vptr của derived ⟹ vtable của derived ⟹ bản override.
- ① vptr mỗi object, vtable mỗi class; *"vptr 8 byte + int 8 byte ⟹ khoảng 16 byte"*.
- ② *"algo kiểm tra compute thấy là virtual ⟹ kiểm vtable ⟹ gọi vptr ⟹ vtable của GlobalDimming ⟹ compute của GlobalDimming."*

**✅ Được:** đúng chuỗi object → vptr → vtable của class thật → hàm override; đúng *mỗi object một vptr, mỗi class một vtable*; đúng tổng 16 byte.

**❌ Vì sao chưa 4:**
- `int` là **4** byte, không phải 8; 16 là vì **padding** cho thẳng hàng 8. Chạy thật:
  ```
  sizeof(int)=4 sizeof(void*)=8 sizeof(GlobalDimming)=16
  ```
- ② mô tả như thể **lúc chạy** chương trình "kiểm tra xem có virtual không". Thật ra quyết định đó xong **lúc compile**: compiler sinh lệnh *nạp vptr từ object → nạp ô số N → gọi gián tiếp*, với **N cố định lúc compile**. Chính "N cố định" là lý do thêm hàm virtual vào giữa interface làm hỏng `.so` cũ — móc sang `DP-043` ở phần 2.
- *"vtable được kế thừa"* — lỏng: mỗi class có vtable **riêng**; ô nào override thì trỏ hàm mới, ô nào không thì trỏ lại hàm base.

**Đáp án ([bank CPP-006](../bank/cpp.md)):**
> Lời gọi `p->area()` **không** biết class thật lúc compile — nó **tra bảng lúc chạy** (dynamic dispatch): lấy vptr từ object → nhảy tới ô tương ứng → gọi. Chỉ số ô là cố định lúc compile, nên chi phí là **hằng số**, không phải tìm kiếm.

</details>

---

### Câu 6 · `OS-007` · Ⓖ · 🔁 retention · 🟡 · **3/4**

**Mutex và semaphore khác nhau?**

**Follow-up:** ① ngoài ownership, mỗi loại sinh ra để giải gì? tình huống semaphore đúng còn mutex thì không? ② library dùng named semaphore làm khoá — vì sao không mutex, và mất gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** khác bản chất ở ownership — mutex chỉ owner unlock được, semaphore thì post/wait tự do.
- ① Mutex bảo vệ critical section; semaphore báo hiệu giữa luồng hoặc quản N tài nguyên. Ví dụ: shm bảo vệ bởi semaphore *"là cách chuẩn và đơn giản"*; luồng tính toán ADC ngủ trên sem, interrupt nhả sem đánh thức nó.
- ② Legacy ổn định, team giữ thiết kế; reset được ở bất cứ đâu ⟹ mỗi API có kiểm thời gian, cầm quá 7 giây thì reset để API khác chạy tiếp. Mất: phải dựa vào quy tắc viết code (khoá hai lần, quên khoá), một chỗ bất kỳ có thể nhả khoá bất ngờ vì không có ownership.

**✅ Được:** ownership + mục đích (bảo vệ vs báo hiệu) + ví dụ **ISR → task** chuẩn sách. ② trả lời bằng **hệ thật** và nêu được cái giá — tốt hơn đáp án sách.

**❌ Vì sao chưa 4:**
- Thiếu **priority inheritance** — hệ quả quan trọng nhất của ownership, là phần *Chốt* của bank.
- Ví dụ đầu ở ① (*"shm bảo vệ bởi semaphore là cách chuẩn"*) là **dùng semaphore như mutex** — đúng cái bẫy bank cảnh báo, và ngược với định nghĩa bạn vừa nói.
- ② chưa nói cái mất lớn nhất: **process chết khi giữ thì không ai biết** — chính câu 7.

**Đáp án ([bank OS-007](../bank/os.md)):**
> Với **mutex**, OS biết *ai đang giữ* (nhờ ownership) nên **tạm nâng ưu tiên của L lên bằng H** để L chạy xong và nhả lock nhanh → H đi tiếp. Với **semaphore**, kernel **không biết ai đang giữ** (ai signal cũng được) → **không thể** nâng ai → priority inversion không được xử lý.
>
> **Bẫy:** dùng **binary semaphore (0/1) thay mutex** để bảo vệ critical section — chạy đúng trong test, chết trong hệ real-time.

Bản đầy đủ cho ② (vì sao chọn semaphore, mất gì, sửa sao): [**LNX-046**](../bank/linux-sysprog.md).

</details>

---

### Câu 7 · `LNX-045` · Ⓖ · 🟠 · **2/4** · KIẾN THỨC

**Library của em được nạp vào nhiều process; mọi hàm `lib_api_*` đều bọc giữa `sem_wait`/`sem_post` trên một named semaphore. Một tool test bị `kill -9` đúng lúc đang ở trong một hàm API. Các process còn lại ra sao? Vì sao không có `EOWNERDEAD` như robust mutex? Em xử lý thế nào?**

**Follow-up:** ① nếu **không có** cơ chế 7 giây — các process còn lại ra sao, vì sao kernel không tự nhả hộ? ② tool chết khi đã ghi `mode` nhưng chưa ghi `backlight`; 7 giây sau process kế tiếp reset rồi chạy — nó đọc thấy gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** mỗi `lib_api` có kiểm thời gian; quá 7 s thì tự reset sem, lấy sem và chạy tiếp ⟹ hệ thống vẫn chạy, chấp nhận API bị kẹt không hoàn thành.
- ① *"Không rõ kernel thực hiện cơ chế đó như thế nào."*
- ② *"Mode mới, backlight cũ ⟹ thiết bị chạy sai. Không có cơ chế bảo vệ shared data sau khi vượt khoá ⟹ rủi ro."*
- *(Góp ý: đây là vấn đề thiết kế thật, giúp trả lời toàn diện vì sao sem thay mutex và cách khắc phục ⟹ thành câu [LNX-046](../bank/linux-sysprog.md).)*

**✅ Được:** biết hệ thật xử lý thế nào; tự thấy **state dở dang** — đúng tầng "vì sao sâu" của bank.

**❌ Vì sao 2:**
- **Vế "vì sao không có `EOWNERDEAD`" trắng** — mà đó là T1 của câu, và bạn đã có sẵn câu trả lời ở câu 6: semaphore **không có chủ** ⟹ kernel không biết lần `sem_wait` nào thuộc về ai ⟹ không biết nhả hộ ai.
- Không thấy lỗ của chính cơ chế 7 giây: **hết hạn không có nghĩa là đã chết**. Chạy thật — A chỉ chậm, B reset sau 1 s:
  ```
  A: vao vung gang, ghi mode=2... (dang cham, 3 s)
  B: qua han -> reset semaphore (sem_post) roi vao
  B: vao vung gang, so process dang o trong = 2, thay mode=2 backlight=50
  A: xong, sem_post
  Cuoi cung: gia tri semaphore = 2 (khoa nhi phan le ra chi duoc la 0 hoac 1)
  ```
- Chưa đề xuất được cách sửa nào ngoài cơ chế hiện có.

**Đáp án ([bank LNX-045](../bank/linux-sysprog.md)):**
> 3. Vì sao không đụng: semaphore là **bộ đếm, không có chủ sở hữu** ([OS-007](../bank/os.md)). Kernel không biết lần `sem_wait` nào thuộc về ai ⟹ không biết phải hoàn trả cho ai.
> 4. Process kế tiếp gọi bất kỳ `lib_api_*` nào ⟹ `sem_wait` **treo vĩnh viễn**.
>
> **Bẫy 2.** Sửa bằng cách *"tăng semaphore lên 1 khi thấy treo"*: không biết process cũ chết hay chỉ đang chậm ⟹ có thể cho hai bên cùng vào.

**Tài liệu gốc ([ipc-linux §4.3](../../../04-linux-system-programming/ipc-linux.md)):**
> | B chết đột ngột | Kernel đóng fd hộ ⇒ A nhận **EOF** hoặc **EPIPE** ⇒ A **biết** và xử lý được | Mutex nằm **trong vùng nhớ**, kernel không biết nó là gì ⇒ khoá **kẹt vĩnh viễn** ⇒ A gọi `lock()` và **treo mãi mãi** |

**Cách khắc phục (trả lời góp ý) — tóm tắt [LNX-046](../bank/linux-sysprog.md):**

| Bậc | Khoá | State dở dang |
|---|---|---|
| ① Giữ semaphore | Ghi **PID chủ** vào shm ngay sau `sem_wait`; quá hạn thì chỉ reset khi PID đó **đã chết** | Cờ `dirty` trước/sau khi ghi; gặp `dirty` ⟹ nạp lại từ driver hoặc mặc định |
| ② ⭐ Robust mutex | `EOWNERDEAD` **chỉ** khi chủ thật sự chết | **Ghi kiểu commit:** hai bản + đổi chỉ số bằng một phép ghi |
| ③ Daemon sở hữu state | Client không cầm khoá của nhau | Chỉ daemon ghi |

Chạy thật bậc ②:
```
A: giu khoa, ghi mode=2 backlight=80... bi kill -9 giua chung
B: pthread_mutex_lock -> EOWNERDEAD
B: cach ghi thang : mode=2 backlight=50  <- lan lon cu/moi
B: cach ghi commit: mode=1 backlight=50  <- nguyen ban cu, nhat quan
B: ghi lai tron ven -> mode=2 backlight=80
```

</details>

---

## 🎯 Ba lỗ hổng ưu tiên

1. **Thuộc nguyên văn bản 30″** ([A1 §10.1](../../../11-design-patterns/in-practice/A1-baseline-libdisplay.md)): ba **ranh giới** (C++ interface · API C · `ioctl`), không phải ba **khối**; câu cuối là câu trao quyền. Bản 90″ phải có câu *"chỗ duy nhất lấy khoá"*.
2. **Một gốc, ba câu: semaphore không có chủ.** Nối được thì trả lời liền mạch cả `OS-007` (mất priority inheritance), `DP-040` (tự deadlock), `LNX-045` (kernel không nhả hộ) và `LNX-046` (timeout-reset cho hai bên cùng vào). Đọc [LNX-046](../bank/linux-sysprog.md), tập bản nói 45″.
3. **Dimming cho người ngoài ngành, một câu:** *"hạ đèn ở vùng ảnh tối — đen sâu hơn, tốn ít điện hơn"* ([RES-007](../bank/resume.md)). Không mở bằng danh sách tính năng PQ.

---

## 📦 Mã nguồn thí nghiệm

<details><summary><code>onesem.c</code> — hai process mở cùng tên thì có mấy semaphore</summary>

```c
// Hai process mo cung ten "/r1p_lock": co bao nhieu semaphore that?
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    sem_unlink("/r1p_lock");
    sem_t *s = sem_open("/r1p_lock", O_CREAT, 0600, 1);
    if (fork() == 0) {                       // process A
        sem_t *a = sem_open("/r1p_lock", 0);
        sem_wait(a);
        printf("A: da giu khoa (dia chi sem_t trong A = %p)\n", (void*)a);
        sleep(2);
        _exit(0);                            // chet ma khong sem_post
    }
    sleep(1);
    if (fork() == 0) {                       // process B, mo lai bang ten
        sem_t *b = sem_open("/r1p_lock", 0);
        int v; sem_getvalue(b, &v);
        printf("B: gia tri semaphore B thay = %d (dia chi trong B = %p)\n", v, (void*)b);
        _exit(0);
    }
    while (wait(NULL) > 0) {}
    int v; sem_getvalue(s, &v);
    printf("Sau khi A chet: gia tri = %d\n", v);
    sem_unlink("/r1p_lock");
    return 0;
}
```
Build: `gcc -Wall -Wextra onesem.c -o onesem -pthread && ./onesem`
</details>

<details><summary><code>fix.c</code> — timeout-reset vs robust mutex + ghi kiểu commit</summary>

```c
// R1' — hai thi nghiem cho cau 7 (LNX-045)
//   ./fix reset  : khoa bang semaphore + "qua han thi reset" (thiet ke hien tai)
//   ./fix robust : robust mutex trong shm + ghi state kieu commit (cach khac phuc)
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct State { int mode; int backlight; };
struct Shm {
    pthread_mutex_t m;
    int inside;                 // so process dang o trong vung gang
    struct State inplace;       // cach ghi hien tai: ghi thang tung truong
    int active;                 // cach ghi commit: 2 ban, chi so ban dang dung
    struct State slot[2];
};

static struct Shm *shm;

static void reset_demo(void) {
    sem_unlink("/r1p_api");
    sem_t *s = sem_open("/r1p_api", O_CREAT, 0600, 1);
    if (fork() == 0) {                                   // A: CHAM, khong chet
        sem_wait(s);
        __atomic_add_fetch(&shm->inside, 1, __ATOMIC_SEQ_CST);
        printf("A: vao vung gang, ghi mode=2... (dang cham, 3 s)\n");
        shm->inplace.mode = 2;
        sleep(3);
        shm->inplace.backlight = 80;
        __atomic_sub_fetch(&shm->inside, 1, __ATOMIC_SEQ_CST);
        sem_post(s);
        printf("A: xong, sem_post\n");
        _exit(0);
    }
    usleep(200 * 1000);
    if (fork() == 0) {                                   // B: cho 1 s roi "reset"
        struct timespec dl; clock_gettime(CLOCK_REALTIME, &dl); dl.tv_sec += 1;
        if (sem_timedwait(s, &dl) == -1 && errno == ETIMEDOUT) {
            printf("B: qua han -> reset semaphore (sem_post) roi vao\n");
            sem_post(s);
            sem_wait(s);
        }
        int n = __atomic_add_fetch(&shm->inside, 1, __ATOMIC_SEQ_CST);
        printf("B: vao vung gang, so process dang o trong = %d, thay mode=%d backlight=%d\n",
               n, shm->inplace.mode, shm->inplace.backlight);
        __atomic_sub_fetch(&shm->inside, 1, __ATOMIC_SEQ_CST);
        sem_post(s);
        _exit(0);
    }
    while (wait(NULL) > 0) {}
    int v; sem_getvalue(s, &v);
    printf("Cuoi cung: gia tri semaphore = %d (khoa nhi phan le ra chi duoc la 0 hoac 1)\n", v);
    sem_unlink("/r1p_api");
}

static void commit_write(int mode, int backlight, int die_midway) {
    int next = 1 - shm->active;
    shm->slot[next] = shm->slot[shm->active];            // 1. chep ban dang dung sang ban nhap
    shm->slot[next].mode = mode;                         // 2. ghi vao ban nhap
    shm->inplace.mode = mode;                            //    (cach cu: ghi thang)
    if (die_midway) raise(SIGKILL);                      //    chet o day: chua ghi backlight, chua doi chi so
    shm->slot[next].backlight = backlight;
    shm->inplace.backlight = backlight;
    __atomic_store_n(&shm->active, next, __ATOMIC_RELEASE); // 3. MOT phep ghi = commit
}

static void robust_demo(void) {
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_setpshared(&a, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&a, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&shm->m, &a);

    if (fork() == 0) {                                   // A: chet giua chung
        pthread_mutex_lock(&shm->m);
        printf("A: giu khoa, ghi mode=2 backlight=80... bi kill -9 giua chung\n");
        commit_write(2, 80, 1);
        _exit(0);
    }
    wait(NULL);
    // B
    int rc = pthread_mutex_lock(&shm->m);
    printf("B: pthread_mutex_lock -> %s\n", rc == EOWNERDEAD ? "EOWNERDEAD" : strerror(rc));
    printf("B: cach ghi thang : mode=%d backlight=%d  <- lan lon cu/moi\n",
           shm->inplace.mode, shm->inplace.backlight);
    struct State cur = shm->slot[__atomic_load_n(&shm->active, __ATOMIC_ACQUIRE)];
    printf("B: cach ghi commit: mode=%d backlight=%d  <- nguyen ban cu, nhat quan\n",
           cur.mode, cur.backlight);
    if (rc == EOWNERDEAD) pthread_mutex_consistent(&shm->m); // tuyen bo da sua xong state
    commit_write(2, 80, 0);
    cur = shm->slot[shm->active];
    printf("B: ghi lai tron ven -> mode=%d backlight=%d\n", cur.mode, cur.backlight);
    pthread_mutex_unlock(&shm->m);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    shm = mmap(NULL, sizeof *shm, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    shm->inplace = (struct State){1, 50};
    shm->slot[0] = (struct State){1, 50};
    shm->active = 0;
    if (argc > 1 && strcmp(argv[1], "reset") == 0) reset_demo();
    else robust_demo();
    return 0;
}
```
Build: `gcc -Wall -Wextra fix.c -o fix -pthread && ./fix reset && ./fix robust` (gcc 11.4, Linux 6.8, 0 warning)
</details>
