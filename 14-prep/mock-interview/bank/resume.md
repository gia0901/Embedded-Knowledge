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

**Bẫy:** ① kể theo trình tự thời gian *"đầu tiên em được giao…"* — chán và không lộ năng lực · ② nhảy thẳng vào chi tiết kỹ thuật mà không dựng bối cảnh, interviewer không theo kịp · ③ chọn project Windows cho một JD Embedded Linux · ④ nói *"em làm theo yêu cầu team"* — triệt tiêu mọi cơ hội ghi điểm.
</details>

#### RES-002 · 🟠 · design · ⭐ · 🏗️ · [→ RESUME: "unified interface… multiple hardware configurations"]
**Resume ghi em làm "C++ interface thống nhất nhưng hỗ trợ nhiều cấu hình phần cứng khác nhau". Cụ thể em làm thế nào để một interface phục vụ được nhiều chipset?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** đây là **câu quan trọng nhất trong resume của bạn** với JD này — nó chính là *"thiết kế C++ shared library"*. Họ dò: bạn hiểu **abstraction** hay chỉ viết `if (chipset == A)`?

**Câu trả lời tốt gồm:**
1. **Nêu vấn đề trước:** nhiều chipset, cùng một chức năng, chi tiết thanh ghi/luồng khác nhau. Nếu để lộ lên trên thì mỗi lần thêm chipset là sửa khắp nơi.
2. **Nói cơ chế bạn thật sự dùng** — trung thực. Nếu là `if/switch` theo model thì **nói thẳng**, rồi nói bạn thấy hạn chế gì và sẽ cải thiện ra sao. Interviewer quý sự trung thực + nhận thức hơn là mô tả đẹp mà hỏi sâu là vỡ.
3. **Gọi tên nguyên tắc:** interface do **tầng trên** định nghĩa, implementation nằm ở dưới — đảo chiều phụ thuộc (**DIP**). Ba cách tạo "khe cắm": **virtual** (chọn lúc chạy) · **template/CRTP** (chọn lúc biên dịch, không tốn vtable) · **link-time** (mỗi sản phẩm link một `.so` khác).
4. **Đánh đổi:** virtual tốn một lần gọi gián tiếp nhưng thêm chipset **không phải sửa code cũ**; `if/switch` nhanh hơn nhưng mỗi lần thêm là sửa vào file đang chạy tốt (rủi ro hồi quy).

**Nền kỹ thuật phải nắm:** [DP-011](design-patterns.md) DIP/HAL · [solid-principles](../../../11-design-patterns/solid-principles.md) (OCP: *mở để mở rộng, đóng để sửa đổi*) · [api-design](../../../07-shared-libraries/api-design.md) · nếu là `.so` giao cho bên khác thì thêm **ABI** ([SD-017…031](system-design.md)).

**Bẫy:** ① nói *"em dùng OOP"* rồi hết — phải nêu **ai chọn implementation, chọn lúc nào** · ② thổi phồng thành kiến trúc plugin động nếu thực tế chỉ là `#ifdef` — hỏi hai tầng là lộ · ③ quên rằng đây là **shared library**: thêm virtual function vào class đang phát hành là **phá ABI** ⇒ nếu bạn nêu được ý này, đây là chỗ ghi điểm senior.
</details>

#### RES-003 · 🟠 · concept · ⭐ · [→ RESUME: "OOP-based mechanisms for multi-chipset support" (kernel HAL)]
**Resume ghi em dùng "cơ chế OOP" trong kernel driver. Kernel viết bằng C — làm sao có OOP ở đó?**

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

#### RES-007 · 🟢 · concept · ⭐ · [→ RESUME: "Dimming, FRC, and TCON"]
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

**Bẫy:** ① *"em đọc log rồi so code"* — đúng là cách bạn đang làm, nhưng nói trần trụi vậy nghe như **không có phương pháp**. Vẫn kể sự thật, nhưng kể **có cấu trúc**: *"em bắt đầu từ log để dựng mốc thời gian, rồi khoanh vùng bằng cách…"* · ② kể một ca mà **gốc rễ hoá ra người khác tìm ra** · ③ không nêu được **cách chặn tái diễn** — interviewer senior rất để ý phần này.
</details>

#### RES-009 · 🟡 · concept · 🏗️ · [→ RESUME: "reducing working time by around 70%"]
**Em ghi tự động hoá giảm 70% thời gian làm việc. Con số đó đo bằng cách nào?**

<details><summary>Khung trả lời</summary>

**Interviewer đang dò gì:** ⚠️ **Đây là câu kiểm tra độ tin cậy, không phải câu kỹ thuật.** Mọi con số trong resume đều mời gọi câu hỏi này. Trả lời được ⇒ toàn bộ resume tăng độ tin. Ú ớ ⇒ interviewer bắt đầu nghi ngờ **mọi** con số khác.

**Câu trả lời tốt gồm:**
1. **Phạm vi hẹp lại cho chính xác:** 70% của **việc gì** — không phải toàn bộ công việc, mà là *"thời gian viết unit test cho một module"* hoặc *"thời gian port một driver sang chipset mới"*.
2. **Mốc so sánh:** trước đây làm tay mất bao lâu (ví dụ ~2 ngày/module), sau khi có tool còn bao lâu (~4 tiếng).
3. **Đo trên bao nhiêu mẫu:** *"em đo trên N module đã làm cả hai cách"* — hoặc **thừa nhận là ước lượng** nếu đúng là ước lượng.
4. **Giới hạn:** chỗ nào tool **không** giúp được (logic phức tạp vẫn phải viết tay; phải review lại output).

> **Trung thực là chiến lược tốt nhất ở đây.** *"Đây là ước lượng của em dựa trên N lần làm, không phải số đo chính thức"* — câu này **an toàn hơn** một con số cứng mà không có cơ sở.

**Nếu không bảo vệ được con số:** cân nhắc sửa resume thành phát biểu định tính (*"significantly reduced"*) hoặc thu hẹp phạm vi rõ ràng.

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
