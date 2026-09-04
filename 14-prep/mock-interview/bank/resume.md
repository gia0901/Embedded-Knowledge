# 📄 RES — Câu hỏi bám RESUME

> **Domain `RES`.** Mọi câu ở đây bám vào **một dòng cụ thể trong [RESUME_embedded_linux.tex](../../../RESUME_embedded_linux.tex)**. Đây là phần **chắc chắn 100% bị hỏi** — khác với câu kỹ thuật thuần chỉ *có thể* bị hỏi. Phỏng vấn thật dành **30–50%** thời gian ở đây.
>
> ⚠️ **Đáp án ở file này KHÔNG phải "lời giải".** Chỉ bạn mới biết bạn đã làm gì. Đáp án là **KHUNG**, gồm 4 phần:
> 1. **Interviewer đang dò gì** — câu hỏi thật nằm sau câu hỏi bề mặt.
> 2. **Câu trả lời tốt gồm** — bộ khung để bạn tự điền chi tiết của mình vào.
> 3. **Nền kỹ thuật phải nắm** — thứ bạn buộc phải giải thích được, kèm link ôn.
> 4. **Bẫy** — chỗ ứng viên hay tự đào hố.
>
> **Cách luyện:** nói thành lời, bấm giờ **90 giây/câu**. Viết ra giấy không tính — phỏng vấn là nói.
> **Chạy phiên:** `/mock rapid track resume` (quét rộng) hoặc `/mock daily track resume` (đào T2).

---

#### RES-001 · 🟡 · concept · ⭐ · 🏗️ · [→ RESUME §Work Experience]
**"Em giới thiệu qua về công việc hiện tại và một project em tâm đắc nhất."**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn có **sở hữu** một mảng nào thật không, hay chỉ làm task được giao · ② khả năng kể một hệ thống **từ trên xuống** (bối cảnh → vấn đề → việc bạn làm → kết quả) · ③ chọn project nào — nó lộ ra bạn tự hào về cái gì.

**Câu trả lời tốt gồm (90 giây, không dài hơn):**
1. **Một câu định vị:** *"Em làm system software cho Display Enhancement trên TV Samsung — tầng giữa, từ C++ interface xuống kernel driver nói chuyện với SoC."*
2. **Chọn ĐÚNG project.** Với JD Datalogic (Embedded Linux), chọn **S-Box** (multi-device, POSIX mq, sensor) hoặc **HAL multi-chipset** — **không** chọn Windows Display Manager.
3. **Vấn đề → giải pháp → con số.** Ví dụ S-Box: *"nhiều màn ghép thành một, mỗi máy tự chỉnh sáng theo cảm biến thì chúng lệch nhau, nhìn thành từng ô"* → mq đồng bộ → *"ghép lại nhìn như một màn duy nhất"*.
4. **Chốt bằng phần bạn tự quyết định**, không phải phần được giao.

**Nền kỹ thuật phải nắm:** kiến trúc tầng của chính bạn — vẽ được lên whiteboard trong 30 giây:
```
App / Middleware
      | C++ API (interface thong nhat)
System SW  (shared library)   <-- ban o day
      | ioctl / sysfs
Kernel driver (HAL)
      | register / I2C
SoC (Dimming, FRC, TCON)
```

---

### 🎙️ BẢN NÓI — 45 giây, PHẢI đọc to bấm giờ

> ⚠️ **Vì sao câu này có bản nói mà các câu khác không:** `RES-001` hỏng **ba lần liên tiếp** cùng một kiểu (18/08 → 23/08 qua `BEH-001` → 29/08), và cả ba lần **nội dung đều có sẵn trong đầu** — hỏng ở khâu **dựng khung và nói ra**. Đó là loại lỗi **chỉ chữa được bằng nói to bấm giờ**, không chữa được bằng đọc thêm. *(Bằng chứng: `BEH` 2.42 → 3.42 sau đúng một ngày viết 5 khung STAR.)*
>
> 🔴 **Đây là bản để BẠN sửa lại theo giọng mình rồi thuộc** — không phải để đọc thuộc lòng nguyên văn. Giữ nguyên **thứ tự bốn nhịp**; đổi chữ thoải mái.

**Bản A — "project tâm đắc nhất" (mặc định):**

> *"Em làm system software cho display enhancement trên TV và màn hình doanh nghiệp — từ C++ interface xuống kernel driver nói chuyện với SoC.*
>
> *Thứ em tâm đắc nhất là S-Box. **Bài toán là nhiều màn ghép lại thành một màn lớn, mà mỗi máy tự chỉnh sáng theo cảm biến riêng của nó thì chúng lệch nhau — nhìn ra từng ô rõ rệt.***
>
> *Em làm phần đồng bộ độ sáng qua POSIX message queue, một máy làm master phân phối giá trị cho các máy còn lại. Chống nhấp nháy bằng cách chỉ tính lại target **4 giây một lần** rồi step dần tới đó.*
>
> ***Kết quả là ghép lại nhìn như một màn duy nhất, và cùng một binary chạy được cả chế độ đơn lẻ lẫn đồng bộ.**"*

**Bốn nhịp — và vì sao từng nhịp có mặt:**

| Nhịp | Nội dung | Chữa lỗi nào |
|---|---|---|
| 1 | **Định vị** — em đứng ở tầng nào, biên trên/biên dưới là gì | (đã làm tốt cả 3 lần, giữ) |
| 2 | 🔴 **VẤN ĐỀ trước, bằng hình ảnh** — *"nhìn ra từng ô"* | **Lỗi lặp #1:** ba lần đều nhảy thẳng vào công nghệ |
| 3 | Giải pháp — **gọi tên cơ chế, không liệt kê công nghệ** | Ba lần đều thành *"vsync 60Hz, message queue…"* |
| 4 | 🔴 **Kết quả có thể HÌNH DUNG hoặc ĐO** | **Lỗi lặp #2:** ba lần đóng câu **không có gì cụ thể** |

⏱️ **Cách luyện (30 phút, một lần là đủ):** bấm giờ, nói to, ghi âm. Nghe lại và chỉ kiểm **hai** thứ — ① câu thứ hai đã là **vấn đề** chưa ② câu cuối có **thứ đo được / hình dung được** chưa. Đạt cả hai hai lần liên tiếp thì gỡ khỏi sổ yếu.

**Bản B — dùng khi họ hỏi *"kể một vấn đề khó em đã giải quyết"* (KHÔNG phải "tâm đắc"):**

> Chuyển sang **tối ưu thời gian nạp driver** ([RES-013](resume.md)…[RES-015](resume.md)) — vì nó có **chuỗi chẩn đoán bằng bằng chứng** và **con số cứng**: `probe` 3–5 giây → `dmesg` xác nhận đoạn chậm chỉ là gán biến → **CPU 100%, tiến trình RT priority 94 từ giây thứ 4** → driver nền nạp tuần tự bằng `call_usermodehelper` ở **priority 120** ⇒ bị preempt. Kết: **3–4 giây xuống dưới 0,5 giây**, nghiệm thu **100 chu kỳ boot**.
>
> 📌 **Phân vai rõ:** S-Box chứng minh **sở hữu một tính năng end-to-end**; insmod chứng minh **chẩn đoán và đo đạc**. JD Datalogic nhấn debugging ⇒ nếu chỉ được kể **một** thứ trong cả buổi, kể cái thứ hai.

---

**Bẫy:** ① kể theo trình tự thời gian *"đầu tiên em được giao…"* — chán và không lộ năng lực · ② nhảy thẳng vào chi tiết kỹ thuật mà không dựng bối cảnh, interviewer không theo kịp · ③ chọn project Windows cho một JD Embedded Linux · ④ nói *"em làm theo yêu cầu team"* — triệt tiêu mọi cơ hội ghi điểm.
</details>

#### RES-002 · 🟠 · design · ⭐ · 🏗️ · [→ RESUME: "one interface across chipsets… selects its implementation at boot from board configuration… applications above stay unchanged"]
**Resume ghi *"một interface dùng chung cho nhiều chipset, thư viện chọn implementation lúc boot theo board configuration"*. Kể cơ chế đó — và điều gì xảy ra khi board configuration đọc ra sai?**

> 📌 *Neo lại 2026-09-04: resume bản mới **đã tự nói ra cơ chế** (chọn lúc boot + bảng con trỏ hàm), nên câu hỏi dịch trọng tâm sang **hệ quả và chế độ hỏng** — đó mới là chỗ interviewer khoan.*

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **câu quan trọng nhất trong resume của bạn** với JD này — nó chính là *"thiết kế C++ shared library"*. Họ dò: bạn hiểu **abstraction** hay chỉ viết `if (chipset == A)`?

**Câu trả lời tốt gồm:**
1. **Nêu vấn đề trước:** nhiều chipset, cùng một chức năng, chi tiết thanh ghi/luồng khác nhau. Nếu để lộ lên trên thì mỗi lần thêm chipset là sửa khắp nơi.
2. **Nói cơ chế bạn thật sự dùng** — trung thực. Nếu là `if/switch` theo model thì **nói thẳng**, rồi nói bạn thấy hạn chế gì và sẽ cải thiện ra sao. Interviewer quý sự trung thực + nhận thức hơn là mô tả đẹp mà hỏi sâu là vỡ.
3. **Gọi tên nguyên tắc:** interface do **tầng trên** định nghĩa, implementation nằm ở dưới — đảo chiều phụ thuộc (**DIP**). Ba cách tạo "khe cắm": **virtual** (chọn lúc chạy) · **template/CRTP** (chọn lúc biên dịch, không tốn vtable) · **link-time** (mỗi sản phẩm link một `.so` khác).
4. **Đánh đổi:** virtual tốn một lần gọi gián tiếp nhưng thêm chipset **không phải sửa code cũ**; `if/switch` nhanh hơn nhưng mỗi lần thêm là sửa vào file đang chạy tốt (rủi ro hồi quy).

**Nền kỹ thuật phải nắm:** [DP-011](design-patterns.md) DIP/HAL · [solid-principles](../../../11-design-patterns/solid-principles.md) (OCP: *mở để mở rộng, đóng để sửa đổi*) · [api-design](../../../07-shared-libraries/api-design.md) · nếu là `.so` giao cho bên khác thì thêm **ABI** ([SD-017…031](system-design.md)).

**⭐ Chế độ hỏng — phần resume KHÔNG nói, và là chỗ ăn điểm** *(đã hỏi 2026-08-29, đạt 4đ)*:
- **Dimming:** không có config khớp ⇒ rơi về **default an toàn** là *Global Dimming*, vốn chiếm hầu hết tỉ trọng sản phẩm. Nêu được *"có default an toàn"* quan trọng hơn nêu tên default.
- **FRC/TCON:** driver thật xác định **lúc chạy** từ **factory data / FMS key**, vì một board ghép được với nhiều loại panel. ⭐ Và data đó **thay đổi được** để bring-up/test bằng data giả — chi tiết này cho thấy bạn hiểu nhu cầu vận hành thật, không chỉ đường happy path.
- ⚠️ **Câu còn hụt lúc trả lời:** nếu **FMS key sai** (không phải thiếu) thì sao — load nhầm driver, hay từ chối load? Chốt sẵn câu này.

**Bẫy:** ① nói *"em dùng OOP"* rồi hết — phải nêu **ai chọn implementation, chọn lúc nào** · ② thổi phồng thành kiến trúc plugin động nếu thực tế chỉ là `#ifdef` — hỏi hai tầng là lộ · ③ quên rằng đây là **shared library**: thêm virtual function vào class đang phát hành là **phá ABI** ⇒ nêu được ý này là ghi điểm senior.
</details>

#### RES-003 · 🟠 · concept · ⭐ · [→ RESUME: "the kernel driver dispatches through a function-pointer table"]
**Resume ghi kernel driver *"điều phối qua một bảng con trỏ hàm"*. Kernel viết bằng C thuần — cơ chế đó hoạt động thế nào, và nó tương đương cái gì trong C++?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** câu này **rất dễ bị hỏi** vì nó nghe mâu thuẫn. Họ dò xem bạn **thật sự đọc code kernel** hay chỉ mượn chữ.

**Câu trả lời tốt gồm — vẽ được cái này ra là đạt:**
```c
struct panel_ops {                      // "lop truu tuong"
    int  (*init)(struct panel *p);
    int  (*set_brightness)(struct panel *p, int level);
    void (*release)(struct panel *p);
};

static const struct panel_ops chipA_ops = { .init = a_init, .set_brightness = a_set, ... };
static const struct panel_ops chipB_ops = { .init = b_init, .set_brightness = b_set, ... };

struct panel {                          // "doi tuong"
    const struct panel_ops *ops;        // <-- vtable thu cong
    void *priv;                         // <-- du lieu rieng cua tung chipset
};

p->ops->set_brightness(p, 50);          // <-- goi ao
```
- **Kế thừa** = struct lồng struct (struct con chứa struct cha ở đầu) + `container_of()` để đi ngược từ con về cha.
- **Đa hình** = bảng con trỏ hàm — đúng thứ C++ sinh tự động dưới tên **vtable**.
- **Đóng gói** = `void *priv`, tầng trên không cần biết bên trong.

**Nêu thêm để ghi điểm:** đây **không phải sáng tạo riêng**, chính kernel làm vậy khắp nơi — `file_operations`, `i2c_driver`, `platform_driver` đều là ops struct. Nói được điều này chứng minh bạn đọc kernel thật.

**Nền kỹ thuật phải nắm:** [oop.md](../../../01-cpp-fundamentals/oop.md) (vtable) · [driver-basics](../../../05-drivers-device-tree/driver-basics.md) (`file_operations`) · `container_of`.

**Bẫy:** ① nói *"em viết C++ trong kernel"* — kernel Linux **không dùng C++**, nói vậy là sai nghiêm trọng · ② không giải thích được **`void *priv` để làm gì** · ③ bị hỏi *"khác gì vtable của C++"* mà không trả lời được (khác: bạn **tự quản lý**, không có RTTI, không exception, và mỗi instance chỉ tốn một con trỏ).
</details>

#### RES-004 · 🟠 · concept · ⭐ · 🏗️ · [→ RESUME: "kernel migration (Linux 5.10 → 6.12)"]
**Em có làm kernel migration 5.10 lên 6.12. Cái gì vỡ? Em phát hiện và xử lý thế nào?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** 🎯 **Món ngon nhất trong resume của bạn cho JD Datalogic** (họ làm kernel/BSP). Câu này lộ ra bạn **có thật sự chạm kernel** hay chỉ sửa vài dòng. Kể được một ca cụ thể là ăn điểm rất mạnh.

**Câu trả lời tốt gồm:**
1. **Quy mô:** bao nhiêu driver, bao lâu, một mình hay team.
2. **Phân loại cái vỡ** — kể theo nhóm, không kể lan man:

   | Nhóm | Ví dụ điển hình |
   |---|---|
   | **API đổi chữ ký** | callback `remove` đổi kiểu trả về; `probe` bỏ tham số `id` |
   | **API bị xoá** | hàm cũ bị thay bằng API mới hẳn |
   | **Đổi cách khai báo** | macro đăng ký driver, thuộc tính device tree |
   | **Đổi hành vi ngầm** | thứ tự probe, timing, mặc định của subsystem — **loại khó nhất** |

3. **Cách phát hiện** — nhóm 1–3 thì **compiler báo**, dễ. Nhóm 4 **không báo gì**, chỉ lộ lúc chạy ⇒ phải **so sánh hành vi hai kernel**, đọc log boot hai bên, và đọc changelog/commit của subsystem liên quan.
4. **Con số cuối:** đưa sản phẩm boot được / bao nhiêu driver chạy lại.

**Nếu bạn chỉ tham gia một phần:** nói rõ phần của mình. *"Em phụ trách N driver trong mảng display"* — trung thực và vẫn ghi điểm.

**Nền kỹ thuật phải nắm:** kernel **không có ABI ổn định cho module ngoài cây** (đây là lý do việc này tồn tại — nói được là điểm cộng lớn) · [driver-basics](../../../05-drivers-device-tree/driver-basics.md) · [device-tree](../../../05-drivers-device-tree/device-tree.md) · [kernel-debugging](../../../09-debugging/kernel-debugging.md).


**⭐ Bổ sung 2026-09-04 — resume nay ghi thêm *"keeping backward compatibility with the 5.10 branch"*** ⇒ chắc chắn bị hỏi *"tương thích ngược nghĩa là gì — một source hay hai nhánh?"*

**Bản trả lời đã đạt 4đ (29/08):** ***một source duy nhất*** build hợp lệ cho **cả 5.10 lẫn 6.12** — vừa không ảnh hưởng sản phẩm đang bán, vừa sẵn sàng cho đời sau. Cách làm: kiểm **kernel version** rồi tách nhánh cài đặt bằng macro cho từng phiên bản, phần chung giữ đúng convention (khai báo trước, logic sau, luôn có function prototype).

**Nêu thêm được thì lên hẳn một bậc:** ① macro chuẩn là `LINUX_VERSION_CODE` so với `KERNEL_VERSION(a,b,c)` ② **cái giá** của một-source: `#ifdef` càng nhiều thì **nhánh nào cũng phải test riêng**, và code đọc khó dần ⇒ chỉ hợp khi số nhánh **ít và có hạn** ③ khi nào thì **nên tách hai nhánh** thay vì `#ifdef`: khi khác biệt lan ra kiến trúc chứ không còn là vài API.

**Bẫy:** ① trả lời chung chung *"em sửa lỗi build"* — không lộ được gì · ② **không** chuẩn bị được **một ca cụ thể có tên và có cách chẩn đoán** · ③ bị hỏi *"vì sao kernel không giữ ABI như glibc?"* mà tắc — chuẩn bị sẵn: **cố ý**, để kernel tự do refactor nội bộ, và để ép driver vào mainline.
</details>

#### RES-005 · 🟠 · design · ⭐ · 🏗️ · [→ RESUME: "POSIX message queue… synchronize brightness across multiple S-Box devices"]
**Em dùng POSIX message queue để đồng bộ độ sáng giữa nhiều thiết bị. Vì sao chọn message queue mà không phải socket hay shared memory?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn **chọn có lý do** hay dùng cái mình quen. Đây cũng là chỗ nối thẳng sang IPC — mảng bạn đã ôn kỹ.

⚠️ **Làm rõ trước khi trả lời** (và việc *hỏi lại* này chính là điểm cộng): nhiều S-Box là **nhiều process trên cùng một máy** hay **nhiều máy nối mạng**? POSIX mq **chỉ chạy trong một máy** — nếu là nhiều máy vật lý thì phải có socket ở đâu đó. Trả lời đúng bối cảnh thật của bạn.

**Câu trả lời tốt gồm:**
1. **Yêu cầu thật:** bản tin nhỏ (một mức sáng), thưa (khi cảm biến đổi), cần **ranh giới bản tin rõ ràng**, không cần băng thông.
2. **Vì sao mq hợp:** giữ **ranh giới bản tin** (không phải luồng byte như TCP ⇒ không phải tự framing — xem [LNX-005](linux-sysprog.md)) · có **độ ưu tiên bản tin** · có **hàng đợi sẵn**, bên nhận bận thì bản tin vẫn nằm đó · cắm được vào `select`/`epoll` qua fd (Linux).
3. **Vì sao không cái kia:** **shm** nhanh nhưng *không có cơ chế báo hiệu* và phải tự đồng bộ, một bên chết là kẹt khoá ([LNX-015](linux-sysprog.md)) — quá đắt cho vài byte mỗi lần · **TCP socket** phải tự framing, thừa cho giao tiếp nội máy.
4. **Ca hỏng:** một thiết bị chết / hàng đợi đầy thì sao? (mq đầy ⇒ `mq_send` chặn hoặc `EAGAIN` — bạn chọn cái nào và vì sao?)

**Nền kỹ thuật phải nắm:** [LNX-016](linux-sysprog.md) mq · [LNX-035](linux-sysprog.md) POSIX vs SysV · [LNX-017](linux-sysprog.md) chọn IPC · [ipc-linux.md §4 trục quyết định](../../../04-linux-system-programming/ipc-linux.md).


**⭐ Bổ sung 2026-09-04 — nửa câu trả lời còn thiếu** *(hỏi 29/08, đạt 3đ)*:

Resume ghi *"a **single binary** serving both standalone and synchronized modes"* ⇒ interviewer sẽ hỏi: **binary đó biết mình đang ở chế độ nào bằng cách nào?**

Phần **đã trả lời được**: cờ **Multi S-Box** do **application** gửi xuống lúc **runtime** (chính là mục bật tính năng trên thiết bị); bật rồi thì máy đó thành **master** phân phối giá trị thanh ghi cho các máy con.

🔴 **Phần còn thiếu — và là nửa quan trọng hơn:** *máy CON biết mình là con bằng cách nào?* Một binary đóng **hai vai** thì phải kể được **cả hai vai**, nếu không thì chưa chứng minh được "một binary hai chế độ" thực sự vận hành ra sao — mà đó lại chính là **điểm hay nhất** của câu chuyện S-Box.

Chốt sẵn ba câu này trước khi đi phỏng vấn:
1. Máy con nhận vai **thụ động** (cứ nghe hàng đợi, có lệnh thì áp) hay được **cấu hình** thành con?
2. Ai quyết định **ai là master** — người dùng chọn, hay máy tự thoả thuận?
3. **Master chết thì sao** — các máy con giữ giá trị cuối, hay quay về tự chỉnh theo cảm biến của mình?

> 📌 Câu 3 là câu interviewer thích nhất, vì nó hỏi về **chế độ hỏng** — và ở hệ nhiều thiết bị thì chế độ hỏng mới là phần thiết kế thật.

**Bẫy:** ① nói *"vì mq dễ dùng"* — không phải lý do kỹ thuật · ② không biết mq **có giới hạn** (`/proc/sys/fs/mqueue/msg_max`, mặc định 10 bản tin) · ③ không nghĩ tới **thiết bị chết giữa chừng** — với sản phẩm ghép nhiều màn thì đây là ca thật · ④ nhầm mq là liên máy được.
</details>

#### RES-006 · 🟡 · concept · ⭐ · [→ RESUME: "adaptive brightness control based on Ambient Light Sensor"]
**Kể luồng dữ liệu từ cảm biến ánh sáng tới lúc màn hình đổi độ sáng. Làm sao để màn không nhấp nháy liên tục?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn nắm **cả chuỗi** hay chỉ một khúc · ② có nghĩ tới **chất lượng trải nghiệm** không, hay chỉ "đọc được giá trị là xong". Câu *"chống nhấp nháy"* là câu phân loại.

**Câu trả lời tốt gồm — vẽ chuỗi:**
```
ALS (I2C) -> driver doc dinh ky / interrupt -> loc & lam muot
          -> anh xa lux -> muc sang -> ghi xuong panel
```

**Phần chống nhấp nháy — nêu được ít nhất hai cơ chế:**
| Cơ chế | Chống được gì |
|---|---|
| **Hysteresis** (ngưỡng lên khác ngưỡng xuống) | Dao động quanh đúng một ngưỡng ⇒ bật/tắt liên tục |
| **Lọc trung bình trượt / EMA** | Nhiễu tức thời (bàn tay lướt qua, đèn flash) |
| **Chuyển dần theo thời gian (ramp)** | Nhảy bậc thấy rõ ⇒ đổi mượt trong vài trăm ms |
| **Chu kỳ lấy mẫu hợp lý** | Lấy quá dày tốn điện + nhiễu; quá thưa thì phản ứng chậm |

**Đánh đổi phải nêu:** lọc mạnh ⇒ ổn định nhưng **phản ứng chậm** (bật đèn phòng mà 5 giây sau màn mới sáng); lọc nhẹ ⇒ nhạy nhưng nhấp nháy. Đây là chỗ cho thấy bạn từng **chỉnh số thật**.

**Nền kỹ thuật phải nắm:** I2C ([bus-protocols](../../../05-drivers-device-tree/bus-protocols.md), [BUS-001…007](drivers-embedded.md)) · polling vs interrupt ([BSP-010…013](bsp.md)) · sysfs phơi tham số ra userspace ([DRV-035](drivers-embedded.md)).

**Bẫy:** ① chỉ kể *"đọc sensor rồi set brightness"* — bỏ mất phần khó nhất · ② không nói được **đọc bằng polling hay interrupt** và vì sao · ③ không biết cảm biến của mình nối bằng bus gì (chuẩn bị: I2C) · ④ quên phần **người dùng chỉnh tay** thì hệ tự động phải nhường ra sao.
</details>

#### RES-007 · 🟢 · concept · ⭐ · [→ RESUME: "display enhancement (dimming, frame-rate control, timing control)"]
**Dimming, FRC, TCON — em giải thích ngắn gọn cho người ngoài ngành hiểu được không?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn **hiểu** hay chỉ chép từ khoá vào resume · ② **năng lực diễn đạt** — giải thích được cái khó cho người không biết là dấu hiệu senior rõ nhất. Datalogic có JD nhắc *"lead junior / code review"*, nên đây là kỹ năng họ tìm.

**Câu trả lời tốt:** mỗi thứ **một câu**, ngôn ngữ đời thường trước, thuật ngữ sau:
- **Dimming** — điều khiển độ sáng của đèn nền. Sâu hơn: local dimming chia màn thành nhiều vùng, vùng nào ảnh tối thì giảm sáng vùng đó ⇒ tăng tương phản, tiết kiệm điện.
- **FRC** (Frame Rate Control) — nội dung và màn hình chạy khác tốc độ khung hình ⇒ phải chèn/bỏ/nội suy khung để hình mượt, không giật.
- **TCON** (Timing Controller) — con chip nhận tín hiệu ảnh rồi phát đúng **thời điểm** cho từng hàng/cột điểm ảnh trên panel. Nó là cầu giữa xử lý ảnh và tấm nền vật lý.

**Ghi điểm thêm:** nói **bạn động vào phần nào** trong ba cái, và động ở tầng nào (viết interface? viết driver? chỉnh tham số?).

**Bẫy:** ① trả lời bằng thuật ngữ chồng thuật ngữ — nghe là biết chưa hiểu · ② nói dài 3 phút cho một câu 🟢 · ③ liệt kê cả ba mà **không nói mình làm cái nào** — resume ghi cả ba thì phải phân định được.
</details>

#### RES-008 · 🟠 · concept · ⭐ · 🏗️ · [→ RESUME: "cross-layer debugging (user-space & kernel-space)"]
**Resume ghi em debug xuyên tầng user–kernel. Kể một ca cụ thể: triệu chứng ban đầu là gì, em khoanh vùng ra sao, gốc rễ nằm ở đâu?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ **Câu rủi ro cao nhất trong resume của bạn.** Bạn **tự liệt kê debug là kỹ năng**, nên họ sẽ khoan — trong khi [CLAUDE.md §2](../../../CLAUDE.md) ghi đây là điểm yếu tự nhận. Phải chuẩn bị **một ca thật, kể trôi chảy**.

**Câu trả lời tốt gồm — theo đúng thứ tự này:**
1. **Triệu chứng như người dùng thấy** (*"màn chớp đen ~1 giây khi chuyển nguồn vào"*), không phải như log thấy.
2. **Chia đôi bài toán:** lỗi ở user hay kernel? Bằng chứng nào chia được? (log hai bên, `dmesg` timestamp, thử gọi thẳng ioctl bỏ qua tầng trên…)
3. **Thu hẹp dần** — mỗi bước loại được một nửa. Đây là phần được chấm.
4. **Gốc rễ + cách sửa + cách chặn tái diễn** (thêm log? thêm test? thêm kiểm tra tham số?).

**Nền kỹ thuật phải nắm — chuẩn bị gọi tên được công cụ ở từng tầng:**

| Tầng | Công cụ |
|---|---|
| Userspace | `strace`, `gdb`, core dump, ASan, log |
| Ranh giới | `ioctl` trả lỗi gì, `errno`, `/proc`, `/sys` |
| Kernel | `dmesg`/`printk`, `ftrace`, `/proc/<pid>/wchan` |

🧪 **Chuẩn bị bằng tay:** làm [DBG-030…036](debugging.md) — 7 bài lab đúng cho câu này, đặc biệt [DBG-033](debugging.md) (core dump), [DBG-034](debugging.md) (treo).


**⭐ Bổ sung 2026-09-04 — câu này hỏi CẮT ĐÔI, không hỏi quy trình** *(hỏi 29/08, đạt 3đ)*:

Tình huống điển hình: *userspace gọi API set độ sáng, **hàm trả về thành công**, nhưng **màn hình không đổi**.*

❌ **Cách trả lời được 3 điểm** — đưa một quy trình **quét tuần tự từ trên xuống**: khoanh vùng thời gian → đọc log userspace → nếu log sạch thì nghi kernel → điều tra kernel. Đúng, nhưng đó là **dò tuần tự**, chi phí O(số tầng).

✅ **Cách trả lời được 4 điểm — MỘT phép đo chia đôi không gian nghi ngờ:**

> *"Việc đầu tiên em làm là **đọc ngược giá trị ngay tại biên** — `printk` đúng chỗ driver ghi thanh ghi, hoặc đọc lại register/sysfs. Một phép đo đó trả lời dứt khoát:*
> - *giá trị **có** xuống tới kernel ⇒ lỗi nằm ở **kernel/SoC** (ghi sai thanh ghi, sai thời điểm, SoC không nhận)*
> - *giá trị **chưa** xuống ⇒ lỗi ở **userspace / đường truyền** (sai ioctl, bị nuốt ở tầng thư viện, gọi nhầm instance)"*

**Vì sao khác biệt này quan trọng chứ không phải tiểu tiết:** với stack nhiều tầng (app → C++ interface → shared lib → ioctl → driver → SoC), quét tuần tự tốn **O(n)** lần điều tra, chia đôi tốn **O(log n)**. Và mỗi lần điều tra ở kernel là một lần build + flash + reboot.

📌 **Đây trùng đúng điểm yếu tự nhận ở [CLAUDE.md §2](../../../CLAUDE.md)** (*"debug chủ yếu đọc log + so code + suy luận"*). Phân biệt cần nhớ: **đọc log là THU THẬP bằng chứng; chia đôi là THIẾT KẾ phép đo.** Interviewer hỏi *"bước đầu tiên"* là đang dò vế thứ hai.

**Ôn:** [09-debugging/mindset.md](../../../09-debugging/mindset.md) — chia đôi không gian nghi ngờ.

**Bẫy:** ① *"em đọc log rồi so code"* — đúng là cách bạn đang làm, nhưng nói trần trụi vậy nghe như **không có phương pháp**. Vẫn kể sự thật, nhưng kể **có cấu trúc**: *"em bắt đầu từ log để dựng mốc thời gian, rồi khoanh vùng bằng cách…"* · ② kể một ca mà **gốc rễ hoá ra người khác tìm ra** · ③ không nêu được **cách chặn tái diễn** — interviewer senior rất để ý phần này.
</details>

#### RES-009 · 🟡 · concept · 🏗️ · [→ RESUME: "Automated driver/library porting with Python tooling… reducing manual effort by more than 50%"]
**Em ghi tool porting giảm hơn 50% công sức thủ công. Con số đó đo bằng cách nào?**

> 📌 *Neo lại 2026-09-04: con số cũ **"giảm 70% thời gian làm việc" đã bị gỡ khỏi resume**. Câu hỏi giữ nguyên giá trị — chỉ đổi sang con số còn sống. **Không xoá ID**: kỹ năng *bảo vệ một con số mình tự viết ra* vẫn là câu lọc.*

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ **Đây là câu kiểm tra độ tin cậy, không phải câu kỹ thuật.** Mọi con số trong resume đều mời gọi câu hỏi này. Trả lời được ⇒ toàn bộ resume tăng độ tin. Ú ớ ⇒ interviewer bắt đầu nghi ngờ **mọi** con số khác.

**Câu trả lời tốt gồm:**
1. **Phạm vi hẹp lại cho chính xác:** 50% của **việc gì** — không phải toàn bộ công việc, mà là *"phần thủ công khi port một driver sang chipset mới: sửa CMake/Makefile + chỉnh source cho khớp nền tảng"*. ⚠️ Chữ *"manual effort"* trong resume đã hẹp sẵn — **bám vào nó**, đừng để interviewer hiểu thành *"nhanh gấp đôi cả dự án"*.
2. **Mốc so sánh:** trước đây làm tay mất bao lâu cho **một lần port**, sau khi có tool còn bao lâu — và **phần nào tool KHÔNG làm được** (vẫn phải review, vẫn phải sửa chỗ đặc thù chipset).
3. **Đo trên bao nhiêu mẫu:** *"em đo trên N module đã làm cả hai cách"* — hoặc **thừa nhận là ước lượng** nếu đúng là ước lượng.
4. **Giới hạn:** chỗ nào tool **không** giúp được (logic phức tạp vẫn phải viết tay; phải review lại output).

> **Trung thực là chiến lược tốt nhất ở đây.** *"Đây là ước lượng của em dựa trên N lần làm, không phải số đo chính thức"* — câu này **an toàn hơn** một con số cứng mà không có cơ sở.

**Nếu không bảo vệ được con số:** cân nhắc sửa resume thành phát biểu định tính (*"significantly reduced"*) hoặc thu hẹp phạm vi rõ ràng.

🔗 **Cùng loại câu, khác con số:** [RES-013](resume.md) (*3–4s → 0,5s* — con số **đo được**, dễ bảo vệ nhất) · [RES-016](resume.md) (*1 ngày → 1–2 giờ* — con số **rủi ro nhất**, vì nó nói về AI chứ không về bạn). Chuẩn bị cả ba, vì cả ba đều nằm trong resume.

**Bẫy:** ① bịa thêm chi tiết cho khớp — mâu thuẫn sẽ lộ ở câu sau · ② trả lời *"sếp em bảo vậy"* · ③ để lộ rằng 70% là **phỏng đoán một lần** nhưng lại viết như một phép đo.
</details>

#### RES-010 · 🟡 · concept · ⭐ · [→ RESUME: "device tree updates, new hardware support"]
**Em có làm device tree cho phần cứng mới. Cụ thể em sửa gì, và làm sao driver biết nó phải chạy cho node nào?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **kiến thức nền BSP** mà JD Datalogic cần. Câu này rất dễ đào sâu và bạn **nên** để nó được đào — đây là vùng mạnh tiềm năng của bạn.

**Câu trả lời tốt gồm:**
1. **DT dùng để làm gì:** mô tả **phần cứng không tự khai báo được** (khác PCI/USB tự liệt kê) ⇒ kernel biết có thiết bị gì, ở địa chỉ nào, dùng IRQ/GPIO/clock nào — **không phải biên dịch lại kernel** cho mỗi board.
2. **Bạn sửa gì cụ thể:** thêm node cho chip mới, đặt `compatible`, khai địa chỉ thanh ghi (`reg`), chân GPIO, IRQ, clock, và các tham số riêng.
3. **Cơ chế ghép:** chuỗi **`compatible`** trong node khớp với bảng `of_match_table` trong driver ⇒ kernel gọi **`probe()`** của driver đó. Nói được vòng đời `probe`/`remove` là đạt.
4. **Nếu có:** kể ca `EPROBE_DEFER` — driver cần một tài nguyên chưa sẵn sàng nên xin hoãn, kernel gọi lại sau. Nêu được là điểm cộng thật.

**Nền kỹ thuật phải nắm:** [device-tree.md](../../../05-drivers-device-tree/device-tree.md) · [DRV-028…034](drivers-embedded.md) (DTS/DTB/dtc, `compatible`, driver đọc DT) · [BSP-006…009](bsp.md) (probe, `EPROBE_DEFER`, pinctrl).

**Bẫy:** ① nhầm DT là **code chạy** — nó là **dữ liệu mô tả**, bootloader nạp `.dtb` rồi truyền cho kernel · ② không phân biệt được `.dts` (nguồn) / `.dtb` (đã biên dịch) / `dtc` (trình biên dịch) · ③ không giải thích được vì sao x86 **không** dùng DT (dùng ACPI).
</details>

#### RES-011 · 🟡 · concept · 🏗️ · [→ RESUME: "Samsung Display Manager (Windows Application)", MVVM]
**Em có làm ứng dụng Windows với MVVM — kể về nó.** *(và cách lái về đúng thế mạnh)*

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** thường chỉ là **hỏi cho đủ resume**. Nhưng với JD Embedded Linux, đây là mục **ít liên quan nhất** — mục tiêu của bạn là trả lời tử tế rồi **lái về** mảng mạnh, không sa đà.

**Câu trả lời tốt gồm:**
1. **Ngắn** — 30–45 giây. Tính năng Preset, lưu cấu hình bằng JSON, kiến trúc MVVM.
2. **Rút ra thứ CHUYỂN ĐƯỢC sang embedded**, đây mới là phần đáng nói:
   - **MVVM = tách trạng thái khỏi hiển thị** — cùng tư duy với tách *business logic* khỏi *tầng phần cứng* trong HAL.
   - **Cấu hình bền vững qua khởi động lại** — bài toán y hệt trên thiết bị (lưu setting vào flash, lo mất điện giữa chừng).
   - **Modern C++ (11/14/17)** — trùng đúng yêu cầu "C++17" của JD.
3. **Tự lái:** *"phần này giúp em rõ hơn về tách tầng, nhưng mảng em đầu tư sâu là system software và driver ở phía Linux."*

🔗 **Đào sâu ở đâu:** [RES-017](resume.md) (bản nháp vs sửa trực tiếp) · [RES-018](resume.md) (DDC/CI ≈ I²C) · [RES-019](resume.md) (định danh màn hình) · [RES-020](resume.md) (ghi file & mất điện) · [RES-021](resume.md) (đồng bộ Save/Load).

---

### 🧭 KHUNG KỂ CHUNG cho mọi câu SDM — *"bản đầu tiên + hướng cải tiến"*

> ⚠️ **Đặt ở đây MỘT CHỖ**, các câu `RES-017…021` trỏ về. Đừng chép lại.

SDM là **thế hệ đầu**, làm để **kịp tiến độ dự án**. Nhiều phần **cố ý chưa tối ưu**. Đó là sự thật, và nói ra là **đúng** — nhưng cách nói quyết định bạn nghe như *kỹ sư biết đánh đổi* hay *người bào chữa*.

**Ba mức trả lời, xếp từ mạnh xuống yếu:**

| Mức | Dạng câu | Interviewer nghe thấy |
|---|---|---|
| ⭐⭐ **Senior** | *"Em **biết** rủi ro X. Chấp nhận vì ràng buộc Y **cụ thể**, và **bán kính ảnh hưởng** chỉ là Z. Hướng đúng là W."* | đánh đổi **có ý thức** |
| ⭐ **Mid, trung thực** | *"Lúc đó em **không nghĩ tới**. Sau này nhìn lại thì đúng ra nên W."* | thành thật, học được |
| 🔴 **Yếu** | *"Phần đó chưa tối ưu vì lúc đó gấp."* | **bào chữa** — và nếu lặp ở mọi câu thì thành mẫu xấu |

🔴 **Chỗ hỏng phải tránh:** nói *"v1 nên chưa tối ưu"* ở **mọi** câu. Một lần là bối cảnh; ba lần là **cái cớ mặc định**. Mỗi câu phải có **ràng buộc riêng** và **bán kính ảnh hưởng riêng**.

✅ **Và luôn có sẵn vế thứ hai:** *"đồng nghiệp cầm dự án sau này đã cải tiến — họ làm bằng W"* (như state machine ở [RES-017](resume.md)). Nó cho thấy bạn **theo dõi tiếp** sản phẩm sau khi rời tay, và bạn **đồng ý** với hướng đúng.


### ✅ Bản trả lời đã đạt 4đ (2026-08-29) — chốt lại, đừng dựng lại từ đầu

Góc hỏi: *"Bạn ứng tuyển Embedded Linux. Vì sao trong resume lại để một app Windows? Nó chứng minh điều gì mà bốn mục kia không chứng minh được?"*

**Ba lý do, theo đúng thứ tự này:**

1. **Khả năng thích ứng** — không ngại nhận một dự án ngoài chuyên môn trong một giai đoạn để team kịp tiến độ.
2. **Modern C++ vận dụng thật** — move semantics, smart pointer, OOP design; và nó **quay lại phục vụ** chất lượng code cho C++ interface / shared library ở phía embedded. *(Trùng đúng yêu cầu "C++17" của JD.)*
3. ⭐⭐ **MVVM → HAL, qua Dependency Inversion** — đây là câu ăn điểm, vì nó biến một mục "lạc đề" thành bằng chứng cho đúng thứ JD cần:

> *"MVVM giúp em hiểu thêm cách HAL hoạt động trên embedded: **lớp trên không biết hardware, hardware không biết lớp trên** — hai bên chỉ gặp nhau ở một interface trừu tượng. Kết quả là coupling thấp, và **mock test được** khi chưa có phần cứng."*

📌 **Vì sao (3) mạnh:** nó không phải phép so sánh cho vui. Trong chính project đó bạn **đã dùng** `MockAppViewModel` để dựng UI khi chưa có màn hình Samsung cắm vào — **đúng cùng một lý do** người ta tách HAL để test driver không cần board. Nêu được ví dụ `Mock` này là biến lý thuyết thành **bằng chứng đã làm**.

**Bẫy:** ① kể dài về UI/Windows trong buổi phỏng vấn Embedded Linux · ② để lộ rằng đây là project bạn thích nhất (interviewer sẽ nghi bạn không hợp vị trí embedded) · ③ ngược lại — **coi thường project của chính mình** (*"cái đó không liên quan đâu ạ"*) — luôn rút ra được điều gì đó.
</details>

#### RES-012 · 🟠 · concept · ⭐ · 🏗️ · [→ RESUME vs JD: thiếu Yocto, I2C/SPI, PCI/USB]
**"Anh thấy resume em không nhắc Yocto, cũng không thấy I2C/SPI hay PCI/USB. Bên anh dùng những thứ đó khá nhiều."**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ Đây **không** phải câu để loại bạn — đây là câu đo **cách bạn xử lý khoảng trống**. Phản ứng của bạn quan trọng hơn nội dung.

**Công thức ba bước — đừng chối, đừng xin lỗi:**

1. **Thừa nhận thẳng, không vòng vo.** *"Đúng ạ, em chưa làm Yocto trong sản phẩm thật."* — một câu, không kèm biện hộ.
2. **Bắc cầu sang thứ gần nhất bạn CÓ.** Đây là phần được chấm:
   - **Yocto** ← bạn có **cross-compilation**, **CMake/Makefile**, **build system cho nhiều nền tảng**, **port driver qua nhiều chipset**. Bạn hiểu *vấn đề* Yocto giải quyết (dựng cả bản phân phối tái lập được cho thiết bị), dù chưa gõ recipe.
   - **I2C/SPI** ← bạn làm **cảm biến ánh sáng** (gần như chắc chắn qua I2C) và **driver nói chuyện với SoC** qua thanh ghi.
   - **PCI/USB** ← trung thực là chưa; nêu bạn nắm nguyên lý chung (device enumeration, driver model) và học nhanh vì nền driver đã có.
3. **Nói cụ thể mình đang làm gì để lấp** — *"em đang đọc về layer/recipe/sstate của Yocto"* — cụ thể, không hứa suông.

**Nền kỹ thuật nên có trước khi đi phỏng vấn:** [yocto.md](../../../06-build-systems/yocto.md) ở mức **tư duy** (layer/bbappend để làm gì · sstate giải quyết gì · DEPENDS vs RDEPENDS) — [plan nguyên tắc ①](../../study-plans/datalogic-plan.md) ghi rõ **không cần thuộc cú pháp recipe** · [bus-protocols](../../../05-drivers-device-tree/bus-protocols.md) · [pci-usb-drivers](../../../05-drivers-device-tree/pci-usb-drivers.md).

**Bẫy:** ① nói *"em có biết Yocto"* khi chỉ đọc qua — hỏi hai câu là lộ, và **mất niềm tin cho cả buổi** · ② xin lỗi dài dòng, hạ thấp bản thân · ③ chỉ nói *"em học nhanh lắm"* mà không có bằng chứng — trong khi bạn **có** bằng chứng thật: đã port driver qua nhiều chipset và migrate kernel hai đời.
</details>

---

#### RES-017 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-04 · [→ RESUME: "Preset feature … users can save their preferred display settings"]
**"Khi người dùng đang sửa một Preset rồi bấm Cancel, em quay lại trạng thái cũ bằng cách nào? Vì sao không cho sửa trên một bản nháp trong bộ nhớ cho an toàn?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⭐ Đây là câu **đáng giá nhất** của cả project — vì nó là một quyết định thiết kế mà ứng viên **đã tự đảo ngược**. Họ dò: ① bạn có tự đánh giá lại thiết kế của mình không · ② bạn có nêu được **cái giá** của phương án mình chọn không · ③ bạn có biết **lời giải tốt hơn** mà mình đã không dùng không. Rất ít ứng viên mid-level trả lời được cả ba.

**Câu trả lời tốt gồm — kể theo đúng trình tự đã xảy ra:**

1. **Phương án đầu (bản nháp):** sửa vào một bản sao trong bộ nhớ, bấm **Save** mới ghi xuống. Nghe an toàn.
2. **Hai lý do bỏ nó** — nêu **cả hai**, vì chúng thuộc hai loại khác nhau:
   - 🔴 **Sai về trải nghiệm:** người dùng kéo thanh **độ sáng** mà **màn hình không sáng lên** ⇒ đang **chỉnh mù**. Với một app mà toàn bộ mục đích là *nhìn thấy kết quả*, đó là lỗi chí mạng.
   - 🔴 **Sai về code:** để có bản nháp thì **mọi getter/setter phải rẽ nhánh** *"đang edit hay không"* — rải khắp nơi, **dễ sót một chỗ** và UI hiển thị sai.
3. **Phương án chốt:** **sửa thẳng lên màn hình thật**; trước khi vào chế độ edit thì **chụp lại trạng thái đang dùng** vào `currentPresetSettings` để **rollback nếu Cancel**.
4. ⭐ **Nêu cái giá mình đã nhận** — phần phân biệt ứng viên: *"đổi lại, trạng thái cần khôi phục giờ nằm **trên phần cứng**, nên **mọi đường thoát** khỏi chế độ edit đều phải được kiểm soát"* — Cancel, đóng app, rút cáp, chuyển màn hình khác…
5. ⭐⭐ **Nói thẳng lời giải tốt hơn:** *"em xử lý bằng **cờ trong mode** — không sai, nhưng **dễ quên ở chỗ gọi mới**. Team sau này làm bằng **state machine**, và đó là lời giải đúng hơn."*

**Vì sao (5) là câu ăn điểm chứ không phải câu tự hạ mình:** nó chứng minh bạn phân biệt được *"giải pháp chạy được"* và *"giải pháp không thể dùng sai"*. Đó đúng là khoảng cách **mid → senior** mà [plan §📍](../../study-plans/datalogic-plan.md) đã ghi (*"dừng ở sửa xong, chưa tới ngăn tái diễn"*) — ở câu này bạn **đi tới vế thứ hai**.

**Nền kỹ thuật nên nắm:** state machine vs cờ boolean rải rác ([behavioral.md — State](../../../11-design-patterns/behavioral.md)) · vì sao *"số trạng thái tăng thì số tổ hợp cờ tăng theo cấp số nhân"* · rollback/snapshot là mẫu chung: `currentPresetSettings` ở đây ≡ **bản cũ trong OTA A/B** ([BSP-015](bsp.md)) ≡ **transaction rollback**.

**Bẫy:** ① kể thành *"em làm sai rồi sửa"* với giọng xin lỗi — phải kể là *"em đo lại rồi đổi quyết định"* · ② nêu phương án chốt mà **quên cái giá** ⇒ nghe như chưa hiểu hết hệ quả · ③ khen state machine mà **không nói vì sao cờ không đủ** (câu trả lời: cờ đúng ở *thời điểm viết*, hỏng ở *thời điểm người sau thêm đường thoát mới*).
</details>

---

#### RES-018 · 🟡 · concept · ⭐ · 🎤 2026-09-04 · [→ RESUME: "Samsung Display Manager"; nối [bus-protocols](../../../05-drivers-device-tree/bus-protocols.md)]
**"App của em nói chuyện với màn hình bằng gì? Kênh đó có gì đáng lưu ý, và nó liên quan gì tới công việc embedded?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **cầu nối mạnh nhất** từ một project Windows sang JD Embedded Linux. Nếu chỉ nói *"em gọi API Windows"* thì mục này vô giá trị với họ; nói được tầng bus thì nó **thành bằng chứng kinh nghiệm bus**.

**Câu trả lời tốt gồm:**

1. **Kênh là DDC/CI** — chạy ghép trên chính dây **HDMI/DP**, và **về bản chất là một bus I²C** nằm sẵn trong đầu nối. Mỗi thiết lập (độ sáng, tương phản, response time…) là một **mã VCP** — đọc/ghi giống **thanh ghi**.
2. ⭐ **Bốn tính chất khiến nó *giống hệt* bài toán driver nhúng:**

| Tính chất DDC/CI | Đối ứng ở embedded |
|---|---|
| **Chậm** (I²C ~100 kHz) — một lần đọc/ghi tốn hàng chục ms | Không được làm I/O chậm khi đang **giữ khoá** / trong **ISR** |
| **Hay lỗi**, không bảo đảm trả lời | Phải **retry**, và retry phải **có giới hạn + backoff** |
| **Nhiều bên cùng dùng một bus** (driver GPU cũng đọc EDID trên đó) | Tranh chấp bus — phải **nhường**, không độc chiếm |
| Không có thời gian đáp ứng bảo đảm | Không được **block** luồng giao diện / luồng realtime |

3. **Việc bạn thật sự phải xử lý:** không được block UI, **retry từng bước**, và **nhường bus** cho bên khác.
4. **Câu chốt để lái về JD:** *"DDC/CI ở đây và I²C/SPI trên thiết bị nhúng là **cùng một lớp bài toán** — bus chậm, không tin cậy, dùng chung. Chỉ khác cái tên và chỗ em ngồi trong stack."*

**Nền kỹ thuật phải nắm trước khi nói câu này:** [bus-protocols.md](../../../05-drivers-device-tree/bus-protocols.md) — I²C: open-drain + pull-up · START/STOP · ACK/NACK · **clock stretching** · arbitration khi nhiều master. Nói *"giống I²C"* mà không giải thích được **clock stretching** hay **vì sao I²C cần pull-up** thì follow-up sẽ lộ.

**Bẫy:** ① nói *"DDC/CI là I²C"* rồi dừng — phải nêu **hệ quả thiết kế**, đó mới là thứ được chấm · ② thổi phồng thành *"em viết driver I²C"* — bạn ở **phía userspace gọi xuống**, nói đúng vị trí của mình · ③ quên mất rằng đây là chỗ **duy nhất** trong project Windows nối thẳng được sang JD, rồi kể toàn UI ([RES-011](resume.md)).
</details>

---

#### RES-019 · 🟡 · concept · ⭐ · 🎤 2026-09-04 · [→ RESUME: "Samsung Display Manager"; khung kể chung ở [RES-011](resume.md)]
**"App đang mở, người dùng rút dây một màn hình rồi cắm lại — hoặc đổi sang cổng khác. Preset còn gắn đúng màn đó không?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có hiểu **định danh thiết bị** là một bài toán riêng không — hay mặc định *"thiết bị thứ hai luôn là thiết bị thứ hai"*. Đây là câu hỏi **rất hay gặp ở embedded**, chỉ đổi vỏ.

**Sự thật của bản v1 — nói thẳng:** hỗ trợ tối đa **4 màn**, mỗi màn có bộ preset riêng, và khoá bằng **chỉ số** (`AtMonitorIndex`) — tức **vị trí trong danh sách liệt kê**, không phải định danh của chính cái màn hình. Mặc định là màn đang hiển thị app.

⇒ **Giới hạn đã biết:** nếu **thứ tự liệt kê đổi** (rút/cắm, đổi cổng, bật màn theo thứ tự khác), bộ preset có thể **gắn sang màn khác**. Người dùng thấy *"preset của tôi nhảy lung tung"*.

**Vì sao chấp nhận được ở v1 — nêu ràng buộc CỤ THỂ, đừng nói chung chung:** kịch bản mục tiêu là **bàn làm việc cố định**, cắm một lần rồi thôi; và **bán kính ảnh hưởng** chỉ là *preset gắn nhầm*, người dùng chọn lại được — **không mất dữ liệu, không hỏng thiết bị**.

**⭐ Hướng đúng — nói được cái này mới ăn điểm:** khoá theo **định danh ổn định lấy từ EDID** (mã nhà sản xuất + mã sản phẩm + **serial**), rớt về *model + cổng* khi màn không có serial. Mỗi lần danh sách thay đổi thì **so khớp lại theo ID**, không theo vị trí. Chỉ số chỉ còn là *cách hiển thị*, không phải *khoá lưu trữ*.

**⭐⭐ Cầu nối sang embedded — đây là lý do câu này đáng có trong bank:** **cùng một bài toán, ba chỗ khác nhau**

| Chỗ | Biểu hiện |
|---|---|
| `/dev/sda` vs `/dev/sdb` đảo nhau sau reboot | ⇒ dùng `/dev/disk/**by-id**` thay vì `by-path` |
| Thiết bị USB cắm lại ra số khác | ⇒ udev rule khớp theo **serial/VID:PID** |
| Nhiều cảm biến cùng loại trên một bus I²C | ⇒ phân biệt bằng **địa chỉ + node device tree**, không phải thứ tự probe |

**Câu chốt:** *"Bài học em rút ra: **vị trí không phải danh tính**. Cái gì cần sống qua lần cắm lại thì phải khoá bằng thứ thuộc về chính thiết bị."*

**Bẫy:** ① nói *"em xử lý rồi"* trong khi v1 khoá theo chỉ số — [hỏi hai câu là lộ](resume.md) · ② nói giới hạn mà **không nêu hướng đúng** ⇒ nghe như chưa nghĩ tới · ③ quên nêu **bán kính ảnh hưởng** — nó là thứ biến "bug" thành "đánh đổi có ý thức".
</details>

---

#### RES-020 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-09-04 · [→ RESUME: "restore them across restarts"; khung kể chung ở [RES-011](resume.md)]
**"Preset lưu xuống JSON. (a) Mất điện đúng lúc đang ghi file thì sao? (b) Sau này em đổi định dạng file — file cũ của người dùng thế nào?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⭐ Đây là câu **chuyển thẳng sang embedded** — cấu hình trên flash, OTA, mất điện giữa chừng. Ở desktop mất điện là hiếm; **ở thiết bị nhúng nó là chuyện thường ngày**. Trả lời tốt câu này chứng minh bạn nghĩ được như người làm firmware.

**Sự thật bản v1 — nói thẳng:** **refresh và ghi đè toàn bộ file**, **không có trường `version`**.

### (a) Mất điện lúc đang ghi

**Cơ chế hỏng:** ghi đè tại chỗ có một **cửa sổ** mà file đã bị cắt cụt nhưng chưa ghi xong ⇒ lần mở sau **parse JSON thất bại** ⇒ **mất toàn bộ preset**. Không phải hỏng một dòng — hỏng **cả file**.

**Ràng buộc biện minh cho v1 (cụ thể, không nói chung chung):** app **desktop**, mất điện đột ngột hiếm; và **bán kính ảnh hưởng** là *thiết lập hiển thị của người dùng*, **không phải chức năng thiết bị** — cùng lắm là chỉnh lại.

**⭐ Hướng đúng — ghi nguyên tử bằng đổi tên:**
```
1. ghi toàn bộ nội dung mới vào  presets.json.tmp
2. fsync()  ← ép dữ liệu xuống đĩa THẬT, không nằm trong page cache
3. rename("presets.json.tmp", "presets.json")   ← ATOMIC trên cùng filesystem
```
⇒ Ở mọi thời điểm, `presets.json` **hoặc là bản cũ nguyên vẹn, hoặc là bản mới nguyên vẹn** — không bao giờ là một nửa. *(Bước 2 hay bị bỏ: `rename` nguyên tử về **metadata**, nhưng nếu nội dung file mới còn trong page cache thì mất điện vẫn ra file rỗng.)*

### (b) Đổi định dạng file

**Không có `version` ⇒ hai kiểu hỏng, kiểu thứ hai tệ hơn:** ① parse thất bại ⇒ mất preset · 🔴 ② parse **thành công** nhưng thiếu trường mới ⇒ nhận giá trị mặc định **im lặng**, người dùng tưởng preset còn nguyên mà thực ra đã đổi.

**⭐ Hướng đúng:** đặt `"version": N` **ngay từ bản đầu** *(rẻ nhất lúc chưa có người dùng nào, đắt nhất khi đã có)*. Lúc load: `version` cũ hơn ⇒ chạy **migration** rồi ghi lại; `version` mới hơn ⇒ **từ chối và giữ nguyên file**, đừng cố đọc.

### ⭐⭐ Cầu nối sang embedded

| SDM | Thiết bị nhúng |
|---|---|
| ghi đè `presets.json` | ghi đè partition cấu hình trên flash |
| tmp + `fsync` + `rename` | **A/B partition** — ghi vào bank không chạy rồi mới chuyển ([BSP-015](bsp.md)) |
| `version` + migration | schema cấu hình qua nhiều đời firmware |
| mất điện là **hiếm** | mất điện là **bình thường** — thiết kế phải giả định nó xảy ra |

**Câu chốt:** *"Ở desktop em coi mất điện là ngoại lệ nên ghi đè thẳng. Ở thiết bị nhúng thì nó là **ca mặc định** — và lời giải cùng một hình dạng: **ghi ra chỗ khác, xong xuôi mới chuyển sang**."*

**Bẫy:** ① nói *"em ghi nguyên tử bằng rename"* — **v1 KHÔNG làm thế**, nói vậy là khẳng định sai · ② nêu `rename` mà **quên `fsync`** · ③ chỉ lo (a) mà bỏ (b) — schema evolution mới là thứ cắn về lâu dài · ④ nói *"chưa tối ưu vì gấp"* mà không nêu **ràng buộc + bán kính ảnh hưởng** ([khung kể chung](resume.md)).
</details>

---

#### RES-021 · 🟡 · concept · ⭐ · 🎤 2026-09-04 · [→ RESUME: "Samsung Display Manager"; khung kể chung ở [RES-011](resume.md)]
**"Save và Load có chạy khác thread không? Nếu sau này có, em bảo vệ thế nào — đặt một `std::mutex` làm member của class quản lý file là đủ chưa?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① bạn có phân biệt *"chưa gặp lỗi"* với *"đã an toàn"* không · ② ⭐ bạn có hiểu **phạm vi của một mutex** không — đây là chỗ phân biệt thật.

**Sự thật bản v1 — nói thẳng và nói cho ĐÚNG cách:** **chưa có mutex**, và **chưa gặp lỗi**. Nhưng lý do phải nói là **cấu trúc**, không phải may mắn:

> *"Truy cập được **tuần tự hoá bởi thiết kế** — mọi thao tác preset đi qua luồng UI, không có đường nào chạy song song. Nên v1 không cần khoá."*

⚠️ **Khác biệt quan trọng:** *"chưa gặp lỗi nên chắc ổn"* = **yếu**. *"tuần tự hoá bởi thiết kế, và đây là giả định em đang dựa vào"* = **mạnh** — vì bạn nêu được **điều kiện** để nó còn đúng.

**Rủi ro khi giả định đó vỡ** (thêm auto-save nền, đồng bộ cloud, mở hai cửa sổ): reader đọc **file đang ghi dở**, hoặc hai writer **xen kẽ** nhau.

### ⭐ Câu chính: mutex làm member — đủ chưa? **Chưa.**

**Cơ chế:** một `std::mutex` member chỉ tuần tự hoá các truy cập **đi qua đúng object đó**. Nhưng thứ cần bảo vệ **không phải object** — mà là **cái file**, một tài nguyên nằm **ngoài** object và có phạm vi **toàn hệ thống**.

| Ai đụng vào file | Mutex member có chặn được? |
|---|---|
| 2 thread cùng dùng **một** instance | ✅ có |
| 2 **instance** khác nhau của cùng class | ❌ **không** — hai mutex khác nhau |
| 2 **tiến trình** (mở 2 lần app, app + tool cài đặt) | ❌ **không** — mutex nằm trong không gian địa chỉ riêng |

**Ba mức khoá, chọn theo phạm vi của tài nguyên:**

| Phạm vi cần bảo vệ | Công cụ |
|---|---|
| Trong một object | `std::mutex` member |
| Toàn tiến trình (một file, nhiều nơi gọi) | **một chủ sở hữu duy nhất** (singleton) giữ khoá — hoặc khoá tĩnh gắn với *đường dẫn file* |
| **Liên tiến trình** | khoá của HĐH: `flock`/`fcntl` (Linux) · `LockFileEx` (Windows) |

📌 **Và cách rẻ nhất vẫn là (a) của [RES-020](resume.md):** ghi **tmp + `rename`** làm reader **không bao giờ** nhìn thấy file dở dang — **giảm hẳn nhu cầu khoá** thay vì thêm khoá.

**Cầu nối sang embedded:** đúng bài toán *"nhiều bên cùng ghi một tài nguyên"* trên thiết bị — hai tiến trình cùng ghi file cấu hình, hoặc userspace tool và driver cùng đụng một device node. Và [LNX-042](linux-sysprog.md): đặt `pthread_mutex_t` vào shared memory **không tự động** dùng được liên tiến trình — phải bật `PTHREAD_PROCESS_SHARED`. Cùng một bài học: **khoá phải cùng phạm vi với tài nguyên**.

**Bẫy:** ① nói *"em dùng mutex"* trong khi v1 không có · ② *"chưa gặp lỗi nên ổn"* — không nêu được **giả định** đang dựa vào · ③ trả lời *"đủ rồi"* cho câu mutex member — đây chính là chỗ câu hỏi nhắm vào · ④ thêm khoá mà quên rằng **ghi nguyên tử** giải quyết phần lớn vấn đề rẻ hơn.
</details>

---

#### RES-013 · 🟡 · concept · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "Cut driver startup time from 3–4 seconds back to under 0.5 second"]
**"Con số 3–4 giây xuống dưới 0,5 giây — em đo bằng gì? Đo từ mốc nào tới mốc nào?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** con số trong resume là **đo được** hay **nghe nói**. Đây là câu **lọc** — không bảo vệ được một con số mình tự viết ra thì mọi con số còn lại trong resume đều mất giá trị.

**Câu trả lời tốt gồm:**
1. **Công cụ đo + nó nằm ở đâu** — đo **trong kernel** bằng `jiffies` (hoặc `ktime_get()` cho độ phân giải ns), không phải bấm giờ bằng mắt.
2. **Hai mốc rõ ràng** — từ lúc vào `probe()` của driver nền → hết vòng nạp các driver con → thoát `probe()`. Nói được **vì sao chọn hai mốc đó**: `probe` bao trọn phần khởi tạo + nạp driver con, nên nó là đơn vị đo có nghĩa.
3. **Cùng một phép đo cho trước và sau** — nếu baseline đo kiểu khác thì con số cải thiện vô nghĩa.
4. ⭐ **Biết giới hạn phép đo của mình:** `jiffies` có độ phân giải `1/HZ` (thường 1–10 ms) — thừa cho thang **giây**, nhưng nói ra được điều đó cho thấy bạn hiểu công cụ chứ không chỉ dùng nó.

**Nền kỹ thuật phải nắm:** `jiffies` / `HZ` / `time_before()` · `ktime_get()` vs `jiffies` (độ phân giải) · `probe()` chạy ở đâu trong tiến trình boot ([BSP-006](bsp.md)) · `initcall_debug` + `systemd-analyze` như cách đo **độc lập** để đối chiếu ([BSP-034](bsp.md)).

**Bẫy:** ① nói *"khoảng 3–4 giây"* mà không nêu mốc — interviewer sẽ hỏi *"tính từ đâu?"* và bạn phải nghĩ tại chỗ · ② quên rằng baseline phải đo **cùng cách** · ③ nói con số mà **không có đơn vị đo lặp lại được** (đo một lần vs trung bình N lần boot).
</details>

---

#### RES-014 · 🟠 · concept · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "interrupted by other startup work"]
**"'Driver bị chen ngang bởi việc khởi động khác' — em phát hiện ra điều đó bằng cách nào?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là câu **phân biệt mạnh nhất** trong nhóm resume — nó đo bạn **chẩn đoán bằng bằng chứng** hay **đoán rồi thử**. Ứng viên mid-level thường kể *"em thử cái này không được thì thử cái kia"*; ứng viên tốt kể **chuỗi suy luận có số liệu**.

**Câu trả lời tốt gồm — bốn mắt xích, phải đủ cả bốn:**
1. **Triệu chứng có số:** `probe` kéo dài 3–5 s.
2. **Loại trừ nguyên nhân hiển nhiên trước:** đọc `dmesg`, xác nhận đoạn chậm chỉ là **gán biến**, không gọi SoC, không chờ I/O ⇒ **không phải code chậm**.
3. ⭐ **Bằng chứng về tranh chấp CPU:** CPU **100%**, và **hàng loạt tiến trình `SCHED_FIFO` priority RT xuất hiện từ giây thứ 4** — đúng cửa sổ thời gian bị chậm.
4. ⭐ **Nối được hai đầu:** driver nền nạp driver con **tuần tự** bằng `call_usermodehelper()`, và tiến trình đó chạy ở **priority thường (120)** ⇒ **bị preempt** bởi các tiến trình RT. Kết luận rút ra từ số liệu, không phải phỏng đoán.

**Nền kỹ thuật phải nắm:** thang priority trong kernel (**số nhỏ = ưu tiên cao**; RT 0–99, `SCHED_OTHER` map vào 100–139, mặc định **120**) · `SCHED_FIFO`/`SCHED_RR` preempt mọi task `SCHED_OTHER` ([OS-026](os.md)) · `call_usermodehelper()` chạy ở **userspace** với priority thường · `ps -eo pid,cls,rtprio,comm` để nhìn class + rtprio.

**Bẫy:** ① dừng ở *"CPU cao nên chậm"* — chưa nói được **ai** chiếm và **vì sao mình thua** · ② nhầm chiều thang priority · ③ kể giải pháp trước khi kể bằng chứng ⇒ nghe như đoán trúng.
</details>

---

#### RES-015 · 🟠 · design · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "measuring two solutions — loading in parallel and raising scheduling priority"]
**"Em thử hai giải pháp: nạp song song và nâng scheduling priority. Cuối cùng chọn cái nào, vì sao, và rủi ro của phương án bị loại?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ① có **đo** từng phương án hay chọn theo cảm tính · ② có nhìn ra **tác động lên phần còn lại của hệ thống** không — đây là ranh giới mid → senior · ③ có dám nói *"cả hai đều chưa đủ"* không.

**Câu trả lời tốt gồm:**
1. **Mỗi phương án chữa một nửa khác nhau:** nạp song song bỏ được phần **tuần tự** nhưng vẫn ở priority 120 ⇒ vẫn bị preempt. Nâng priority thắng được tranh chấp nhưng **vẫn nạp tuần tự** ⇒ vẫn dài.
2. ⭐ **Có ngưỡng chấp nhận và dám nói phương án đơn lẻ trượt:** riêng lẻ chỉ đạt ~**7/10** lần boot ⇒ **kết hợp cả hai**.
3. ⭐⭐ **Tự kiểm tác động hệ thống:** *"song song + RT priority có thể trở thành điểm nghẽn cho phần khởi động khác, nên em nhờ team Performance đánh giá trước khi áp"*. Rất ít ứng viên mid-level nghĩ tới bước này.
4. **Nghiệm thu bằng số lần lặp:** 100 chu kỳ boot, không phải "thử vài lần thấy ổn".

**Rủi ro của mỗi phương án — phải nêu được:**

| Phương án | Rủi ro |
|---|---|
| **Nạp song song** | Thứ tự phụ thuộc giữa driver không còn được bảo đảm ⇒ `-EPROBE_DEFER` hoặc race lúc init; tăng đỉnh CPU/RAM cùng lúc |
| **Nâng priority (RT)** | Task RT **không bị preempt** ⇒ nếu nó bận rộn hoặc kẹt sẽ **bỏ đói** phần còn lại; đẩy jitter sang subsystem khác |
| **Cả hai** | Cộng dồn cả hai rủi ro ⇒ **bắt buộc** phải đo lại toàn hệ, không chỉ đo thời gian nạp |

**Bẫy:** ① nói *"chọn cái nhanh hơn"* mà không nêu giá phải trả · ② quên rằng RT priority là **đánh đổi toàn cục**, không phải tối ưu cục bộ · ③ viết resume là *"chose"* trong khi thực tế **dùng cả hai** — sai sự thật và **tự mở một câu hỏi khó**.
</details>

---

#### RES-016 · 🟡 · concept · ⭐ · 🏗️ · 🎤 2026-08-29 · [→ RESUME: "Applied AI … cutting authoring time from about 1 day to 1–2 hours"]
**"Dòng về AI trong resume — nếu interviewer đọc nó theo hướng bất lợi cho em thì họ nghĩ gì? Em nói thêm câu nào để chặn?"**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** bạn có **tự đọc được resume của mình bằng mắt người tuyển** không. Đây là câu đo **nhận thức rủi ro**, không đo kiến thức.

**Suy nghĩ bất lợi họ có — nói thẳng ra được là đã ăn nửa điểm:**
> *"Vậy phần nào là **em** làm? Bỏ AI đi thì còn lại gì?"*

Với công ty đang tuyển **kỹ sư C++ nhúng**, dòng đó có thể đọc thành: bạn là **người điều phối công cụ**, không phải người **giải được vấn đề**. Rủi ro càng lớn khi con số (1 ngày → 1–2 giờ) **ấn tượng hơn** mọi con số khác trong resume.

**Câu chặn — một câu, không thanh minh dài:**
> *"AI giúp em đi nhanh ở phần **lặp lại**, nên phần em muốn đầu tư là chỗ **nó không làm thay được**."*

⭐ **Rồi nối ngay vào bằng chứng của chính mình** — đây là phần biến câu phòng thủ thành câu ghi điểm:
> *"— như bài tối ưu thời gian nạp driver: AI không đọc hộ em `dmesg` để thấy tiến trình RT chen vào giây thứ 4."*

**Nguyên tắc chung áp cho mọi dòng resume:** mỗi dòng nên tự hỏi *"câu này mở ra câu hỏi nào?"*. Dòng nói về **công cụ** luôn mở ra câu hỏi *"còn năng lực của bạn ở đâu?"*; dòng nói về **chẩn đoán** thì không.

**Bẫy:** ① nói *"code/test hoàn toàn có thể do AI làm tốt"* — nói với người đang tuyển kỹ sư viết code là **rủi ro thật** · ② thanh minh dài dòng ⇒ nghe như đang chột dạ · ③ chối bỏ dùng AI ⇒ mâu thuẫn với chính resume.
</details>

---

## Cách dùng file này

| Việc | Lệnh / cách làm |
|---|---|
| Quét rộng, luyện nói gọn | `/mock rapid track resume` |
| Đào sâu T2 vào 3–4 câu | `/mock daily track resume` |
| Tự luyện | Bấm giờ **90 giây/câu**, **nói thành tiếng** |

> **Ưu tiên nếu ít thời gian:** RES-002 (interface đa chipset) · RES-004 (kernel migration) · RES-008 (cross-layer debug) · RES-012 (khoảng trống Yocto/I2C). Bốn câu này gần như chắc chắn xuất hiện và có sức nặng lớn nhất.

⬅️ [Bank index](README.md)
