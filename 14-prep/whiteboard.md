# 🖊️ Whiteboard — bộ sơ đồ vẽ tay khi phỏng vấn

> **Mục đích:** **sáu** sơ đồ vẽ được **bằng tay, trong 30–90 giây**, đủ để kể hệ thống của bạn cho một người **chưa từng nghe về display enhancement**.
> Kèm **chiến lược chọn sơ đồ** (vẽ cái nào trước, mở rộng theo hướng nào) và **bản tiếng Anh** cho từng cái.
>
> ⚠️ **Tên trong file là TÊN TÀI LIỆU đã khử nhạy cảm**, theo [bộ từ vựng chuẩn](../11-design-patterns/in-practice/README.md). **Kiến trúc giữ nguyên 100%.** File này **tự chứa** — không link tới tư liệu nội bộ.

---

## 🎯 Luật vàng — đọc trước, quan trọng hơn cả sáu sơ đồ

**1. Vẽ ÍT, nói NHIỀU.** Whiteboard không phải để trình bày đủ, mà để **cho người nghe một cái neo**. Bốn ô và ba mũi tên là đủ cho 5 phút nói. Vẽ 12 ô là mất người nghe.

**2. Vẽ từ trên xuống: người dùng ở trên, phần cứng ở dưới.** Quy ước phổ quát, không cần giải thích.

**3. Đánh SỐ mũi tên khi kể một chuỗi.** Sau đó bạn **chỉ tay** thay vì nói lại — tiết kiệm nửa thời gian và nghe mạch lạc hơn hẳn.

**4. Ghi nhãn cho RANH GIỚI, không chỉ cho ô.** `C ABI`, `ioctl`, `GPL boundary` — ranh giới mới là chỗ interviewer quan tâm, vì đó là chỗ phát sinh quyết định thiết kế.

**5. Vừa vẽ vừa nói, đừng vẽ xong mới nói.** Im lặng 40 giây trước bảng là điểm trừ nặng. Nói *"trên cùng là app…"* trong lúc tay đang vẽ ô đầu tiên.

**6. Chừa chỗ trống bên phải và bên dưới.** Bạn **sẽ** phải thêm vào khi họ hỏi sâu. Vẽ kín bảng ngay từ đầu là tự chặn đường mình.

---

## D0 · "Cái này để làm gì?" — 30 giây, cho người chưa biết display

⚠️ **Chỉ vẽ khi cần.** Dấu hiệu cần: họ hỏi *"dimming là gì?"*, hoặc mặt họ ngơ khi bạn nói "FRC". Nếu họ gật đầu theo kịp thì **bỏ qua D0**, vào thẳng D1.

**Ý tưởng cốt lõi: đừng kể ba tính năng — kể BA NÚM VẶN trên cùng một tấm panel.**

```
                    Anh dang xem mot tam panel
                              |
        +---------------------+---------------------+
        |                     |                     |
   SANG BAO NHIEU?       MUOT KHONG?          DIEU KHIEN THE NAO?
     (dimming)              (FRC)                  (TCON)
        |                     |                     |
   chinh den nen        chen frame o giua      phat tin hieu timing
   phia sau panel       24/30fps -> 60Hz        cho hang/cot cua panel
```

**Một câu cho mỗi núm — dùng phép so sánh, đừng dùng định nghĩa:**

| Núm | Câu nói (≤ 15 giây) |
|---|---|
| **Dimming** | *"Đằng sau tấm panel là một dàn đèn LED. Cảnh tối thì mình hạ đèn ở đúng vùng đó xuống — đen sâu hơn, và tốn ít điện hơn."* |
| **FRC** | *"Phim quay 24 hình/giây, panel chạy 60 hoặc 120. FRC **đoán ra các hình ở giữa** để chuyển động không bị giật."* |
| **TCON** | *"TCON là con chip nói chuyện trực tiếp với tấm kính — nó biến dữ liệu điểm ảnh thành tín hiệu điện đúng thời điểm cho từng hàng, từng cột."* |

> ⭐ **Vì sao cách kể này ăn điểm:** ba tính năng nghe rời rạc, nhưng *"ba núm vặn trên một tấm panel"* thì ai cũng hình dung được ngay. Và nó **dọn đường cho D2** — vì khi tắt màn hình, bạn phải vặn **cả ba núm theo đúng thứ tự**.

---

## D1 · Kiến trúc — sơ đồ MẶC ĐỊNH, vẽ đầu tiên trong 90% trường hợp

> 📐 **Luật hình thức:** trong ô **chỉ có DANH TỪ**. Mọi mô tả (*"logic nặng"*, *"command mỏng"*) nằm **ngoài** sơ đồ — trong lời nói hoặc bảng dưới. Ô nhồi chữ là ô không ai đọc kịp.

**Vẽ theo ba nhịp. Dừng sau mỗi nhịp và nhìn mặt họ.**

### Nhịp 1 — chồng tầng, 20 giây · *"Em làm xuyên tầng, từ đây xuống đây."*

```
          +-------------------------+
          |    App / Middleware     |
          +-------------------------+
  - - - - - - - - - - - - - - - - - - - - - -   C ABI
          +-------------------------+
          |       libdisplay        |
          +-------------------------+
  - - - - - - - - - - - - - - - - - - - - - -   ioctl     user
  - - - - - - - - - - - - - - - - - - - - - -           ---------
          +-------------------------+                     kernel
          |      Kernel driver      |
          +-------------------------+
          +-------------------------+
          |           SoC           |
          +-------------------------+
```

**Nhãn nằm ở LỀ PHẢI, không nằm trong ô.** Ranh giới vẽ bằng nét đứt — nó không phải một tầng, nó là một **hợp đồng**.

### Nhịp 2 — cái phễu, 20 giây · *"Vì sao chỗ đó gọi là chỗ hẹp."*

⭐ **Đây là nhịp mới, và là nhịp đáng giá nhất.** Vẽ nó **cạnh** nhịp 1, đừng xoá nhịp 1.

```
     App-1       App-2       App-3
        \          |          /
         \         |         /
          +--------+--------+
          |      C API      |        <-- CHO HEP
          +--------+--------+
         /         |         \
        /          |          \
     chipA       chipB       chipC
```

> 🗣️ *"Trên có nhiều người dùng, dưới có hơn mười dòng chip — ở giữa là **đúng một hợp đồng**. Đó là lý do đổi chip thì app **không phải build lại**."*

**Vì sao hình phễu ăn hơn hình chồng tầng:** nó cho thấy **cái giá và cái lợi cùng lúc** — một hợp đồng phục vụ nhiều bên, nên nó **khó đổi**. Người nghe tự nghĩ ra câu hỏi tiếp theo (*"thế thêm tính năng cho một chip thì sao?"*), và đó chính là câu bạn muốn họ hỏi.

### Nhịp 3 — hai nhánh, 25 giây · *"Bên dưới tách hai, vì hai bài toán khác nhau."*

```
                      C API
                        |
              +---------+---------+
              |                   |
        +-----+-----+       +-----+------+
        |  Dimming  |       | FRC / TCON |
        +-----+-----+       +-----+------+
              |                   |
            ioctl            panel_ops
              |                   |
              +------> SoC <------+
```

**Bảng đi kèm — nói, không vẽ:**

| | Dimming | FRC / TCON |
|---|---|---|
| Lượng logic | **nặng** — thuật toán | **mỏng** — lệnh thanh ghi |
| Sống ở | userspace, C++ | kernel |
| Đa hình đặt ở | Factory chọn `Algo` + `Backend` | bảng `panel_ops` |

> 🗣️ **Nguyên tắc chốt:** *"**Logic nặng đẩy lên userspace, thao tác thanh ghi mỏng giữ trong kernel.**"*

### Nhịp 4 — chỉ vẽ khi họ hỏi *"chọn chip thế nào?"*

```
        Dimming
           |
       +---+---+
       |Factory|  <-- doc board + panel config LUC CHAY
       +---+---+
           |
      +----+----+
      |         |
   +--+--+  +---+----+
   |Algo |  |Backend |   doi chip: chi thay o nay
   +-----+  +--------+
```

### 🗣️ Kịch bản nói — tiếng Việt, ~60 giây

> *"Trên cùng là app. Dưới nó là một **C API ổn định** — và đây là **chỗ hẹp**.*
>
> *[vẽ nhịp 2]* *Trên chỗ hẹp có nhiều người dùng, dưới nó có hơn mười dòng chip, ở giữa là **đúng một hợp đồng**. Nên đổi chip thì app **không phải build lại**.*
>
> *[vẽ nhịp 3]* *Bên dưới bọn em tách làm hai nhánh, vì **lượng logic khác nhau**: dimming nặng thuật toán nên nằm ở userspace C++; FRC và TCON chỉ là lệnh mỏng xuống SoC nên nằm hẳn trong kernel.*
>
> *Nguyên tắc chung: **logic nặng đẩy lên userspace, thao tác thanh ghi mỏng giữ trong kernel**."*

> ⭐ Câu ăn điểm nhất là **"chỗ hẹp"** — nó cho thấy bạn nghĩ về **hợp đồng giữa các tầng**, không chỉ nghĩ về code. Nói nó **sớm**.

---

## D2 · Sequence diagram — `lib_api_set_power(false)`

**Sơ đồ mạnh nhất của bộ.** Nó chứng minh ba component **thật sự phối hợp**, chứ không phải ba mảnh rời ghép vào slide.

> 🖊️ **Mẹo vẽ sequence diagram trên bảng — làm đúng thứ tự này:**
> **①** Viết tên **các bên** thành một hàng ngang ở trên cùng · **②** Kẻ **hết** các đường đời (vertical) xuống dưới — kẻ hết ngay, đừng kẻ dần · **③** Rồi mới điền mũi tên **từ trên xuống**, đánh số.
> Làm ngược lại thì hết chỗ ở dưới và phải vẽ lại. Kẻ đường đời trước = tự chừa chỗ.

```
   App            libdisplay        drv_panel_core     drv_panel_chipA
    |                  |                   |                   |
    | set_power(false) |                   |                   |
    |----------------->|                   |                   |
    |                  |                   |                   |
    |               +--+--+                |                   |
    |            1  | ramp|                |                   |
    |               +--+--+                |                   |
    |                  |                   |                   |
    |                  | 2 ioctl(BLANK)    |                   |
    |                  |------------------>|                   |
    |                  |                   | 3 ops->blank()    |
    |                  |                   |------------------>|
    |                  |                   |                +--+--+
    |                  |                   |                |blank|
    |                  |                   |                +--+--+
    |                  |                   |                   |
    |                  |                   | 4 ops->power_off()|
    |                  |                   |------------------>|
    |                  |                   |                +--+---+
    |                  |                   |                |panel |
    |                  |                   |                |back  |
    |                  |                   |                |logic |
    |                  |                   |                +--+---+
    |                  |                   |<- - - - 0 - - - - |
    |                  |<- - - - 0 - - - - |                   |
    |<- - - LIB_OK - - |                   |                   |
    v                  v                   v                   v

   ---->  goi          - - ->  tra ve         +--+  tu xu ly (khong roi lifeline)
```

**Bốn bước, nói khi chỉ tay vào từng số:**

| # | Ai làm | Việc |
|---|---|---|
| **1** | `libdisplay` | hạ độ sáng **dần** về 0 — 10 bước, không cắt đột ngột |
| **2** | `libdisplay` → kernel | `ioctl(PANEL_BLANK)` — vượt ranh giới user/kernel |
| **3** | `core` → `chipA` | `ops->blank()` qua bảng con trỏ hàm — **core không biết tên chip** |
| **4** | `chipA` | cắt nguồn **theo thứ tự**: panel → backlight → logic |

### ⭐ Phần ăn điểm: **vì sao thứ tự này, không phải thứ tự khác**

Đây là chỗ biến một sơ đồ mô tả thành một câu chuyện kỹ thuật:

| Đổi chỗ bước nào | Hậu quả |
|---|---|
| Bỏ **1**, cắt nguồn khi đèn còn sáng | người dùng **thấy một cú chớp** |
| **4 trước 3**, cắt nguồn khi panel còn quét | **rác hình** trên màn |
| Sai thứ tự **trong 4** (panel/backlight/logic) | dòng xung (inrush), hoặc treo ở lần bật sau |

> 🗣️ **Câu chốt:** *"Thứ tự này không do phần mềm quy định — nó do **ràng buộc vật lý** của panel. Bật lên thì chạy **ngược lại đúng thứ tự đó**."*
>
> Đây là câu phân biệt người **đã làm thật** với người **đọc tài liệu**. Người đọc tài liệu kể bốn bước; người làm thật biết **vì sao không đổi chỗ được**.

**Bật nguồn = đảo ngược** — không cần vẽ lại, chỉ cần một mũi tên ngược ở lề và nói:
> *"`set_power(true)` là đúng chuỗi này chạy ngược: cấp nguồn logic → backlight → panel, unblank, rồi mới **ramp độ sáng lên**."*

**Nếu họ hỏi *"bước 3 lỗi thì sao?"*** — câu về hợp đồng lỗi, trả lời thẳng:
> *"Đường tắt phải **đi hết**, kể cả khi một bước lỗi — dừng giữa chừng để lại phần cứng ở trạng thái nửa vời còn tệ hơn. Bọn em ghi lỗi lại, đi tiếp, rồi trả mã lỗi tổng hợp lên trên."*

---
## D3 · vsync & adaptive brightness — hai hằng số thời gian

**Đây là sơ đồ trả lời câu *"làm sao không bị nháy?"*.** Nó là một **thang thời gian**, không phải sơ đồ khối — và chính vì thế nó gây ấn tượng: ít ai vẽ trục thời gian trên whiteboard.

```
vsync   ||||||||||||||||||||||||||||||||||||||||||||   60 Hz  -> 16.7 ms
        <------------------ 400 ms ------------------>
step    ^                       ^                       ^        moi 400 ms
        |                       |                       |        = 24 vsync
        do sang nhich MOT NAC ve phia target

        <-------------------- 4 s (10 step) -------------------->
target  ^                                                ^
        DOC sensor, TINH target moi
```

**Và cái mắt người thật sự thấy:**

```
do sang
  ^
  |        target moi
  |            .- - - - - - - - - -
  |         _-'
  |      _-'          <-- 10 nac, moi nac 400 ms
  |   _-'
  |_-'
  +-------------------------------------> t
   0        1s        2s        3s       4s
```

### 🗣️ Kịch bản nói — ~45 giây

> *"Có **hai hằng số thời gian** tách biệt, và đó chính là chỗ chống nháy.*
>
> *Cứ **4 giây** bọn em mới **đọc cảm biến và tính lại target** — đủ lâu để một cú chớp đèn phòng không kéo được target đi.*
>
> *Còn giữa hai lần đó, độ sáng **bò dần** tới target theo **10 nấc, mỗi nấc 400 mili giây** — nên mắt không bắt được bước nhảy nào.*
>
> *400 mili giây là **24 chu kỳ vsync** ở 60 Hz, nên mỗi nấc rơi đúng vào ranh giới khung hình — không có nấc nào rơi vào giữa lúc panel đang quét."*

### Đánh đổi — phải tự nêu, đừng đợi họ hỏi

> *"Cái giá là **phản ứng chậm 4 giây**. Bọn em chấp nhận vì thiết bị đặt cố định trong phòng họp, độ sáng môi trường vốn ổn định. Nếu là thiết bị cầm tay thì đánh đổi này **sai** — lúc đó em sẽ lấy mẫu nhanh rồi **lọc trung bình trượt**, để phản ứng nhanh với thay đổi bền mà vẫn bỏ qua xung."*

> ⭐ Câu cuối là câu senior: nó cho thấy bạn biết **quyết định của mình phụ thuộc bối cảnh nào**, và biết **phương án khác là gì**.

---

## D4 · FRC/TCON trong kernel — ba phần và ranh giới license

⚠️ **Sơ đồ khó nhất, và ăn điểm cao nhất.** Chỉ vẽ khi họ hỏi về **kernel driver**, **bảng con trỏ hàm**, hoặc **GPL**. Đừng tự dựng lên — **D4 cần ~2 phút, cộng D4b nữa là ~4 phút**.

```
            +------------------------------+
            |        drv_panel_shim        |      1 .ko
            |  +------------------------+  |      proprietary
            |  |      shim_export       |  |      insmod TRUOC TIEN
            |  +------------------------+  |
            |  |      shim_bridge       |  |
            |  |    panel_ops [][][]    |  |
            |  +------------------------+  |
            +----^--------------------^----+
                 |                    |
            (a) extern           (b) dang ky
                 |                    |
       +---------+--------+  +--------+---------+
       | drv_panel_core   |  | drv_panel_chipA  |
       +------------------+  +------------------+
            GPL                 proprietary
                                hon 10 loai

   (c) duong goi:  core  ->  shim_export  ->  panel_ops  ->  chipA
```

**Ba quan hệ — nói khi chỉ tay, đừng viết vào ô:**

| | Ai → ai | Việc |
|---|---|---|
| **(a)** | `core` → `shim_export` | `extern` con trỏ hàm — **đây là chỗ vượt ranh giới license** |
| **(b)** | `chipA` → `shim_bridge` | tự **đăng ký** hàm của mình vào `panel_ops` lúc init |
| **(c)** | `core` → … → `chipA` | mọi lời gọi đi qua bảng; **core không biết tên chip nào** |

### D4b · Sequence — thứ tự nạp ba module

⭐ **Vẽ cái này khi họ hỏi *"nạp sai thứ tự thì sao?"*** — và họ **sẽ** hỏi, đó là câu đầu tiên bất kỳ ai làm kernel nghĩ tới.

Sơ đồ khối ở trên cho thấy **ai nối với ai**. Sơ đồ này cho thấy **thứ tự theo thời gian** — và nó làm lộ ra một thứ mà sơ đồ khối giấu mất: **cửa sổ nguy hiểm**.

```
       drv_panel_shim        drv_panel_core        drv_panel_chipA
              |                     |                     |
 (1) insmod -->|                     |                     |
           +----+                   |                     |
           |init|                   |                     |
           +----+                   |                     |
              | panel_ops RONG      |                     |
              | EXPORT_SYMBOL       |                     |
              |                     |                     |
 (2) insmod --|-------------------->|                     |
              |                  +----+                   |
              |                  |init|                   |
              |                  +----+                   |
              |     extern ptr      |                     |
              |< - - - - - - - - - -|                     |
+---          |                     | doc board config    |
|             |                     |                     |
|             |                     | (3) modprobe chipA  |
|             |                     |-------------------->|
| CUA SO      |                     |                  +----+
| NGUY HIEM   |                     |                  |init|
|             |                     |                  +----+
|             |        (4) dang ky vao panel_ops          |
+---          |<------------------------------------------|
              |                     |                     |
              |  (5) ops->blank()   |                     |
              |<--------------------|                     |
              |                 dispatch                  |
              |------------------------------------------>|
              v                     v                     v

  [init] module init chay   ---->  goi   <- - -  tra ve / dang ky
```

**Năm bước — nói khi chỉ tay:**

| # | Chuyện gì xảy ra |
|---|---|
| **1** | `shim` nạp trước. Bảng `panel_ops` **tồn tại nhưng RỖNG**; symbol được export ra |
| **2** | `core` nạp. Trình nạp module **giải symbol** — đây là chỗ thứ tự được **ép buộc**, không phải do quy ước |
| **3** | `core` đọc board config rồi **tự nạp** driver chip đúng loại |
| **4** | `chipA` **điền hàm của nó vào `panel_ops`** — self-registration, `shim` không đi tìm ai cả |
| **5** | Từ đây: `core → shim → chipA`. **Không ai biết tên ai.** |

### 🔴 Cửa sổ nguy hiểm — thứ chỉ sequence diagram mới cho thấy

Giữa **(2)** và **(4)**, `drv_panel_core` **đã sống** nhưng bảng `panel_ops` **vẫn rỗng**. Bất kỳ ai gọi vào bảng trong khoảng đó ⇒ **NULL pointer dereference**, tức kernel oops.

> 🗣️ **Cách nói khi họ hỏi:** *"Thứ tự giữa bước 1 và 2 thì **trình nạp module tự ép** — `core` extern symbol từ `shim`, nên `modprobe` đọc `modules.dep` và nạp `shim` trước. Dùng `insmod` thủ công mà sai thứ tự thì nó **báo lỗi rõ ràng** `Unknown symbol in module`, không crash âm thầm.*
>
> ***Chỗ thật sự rủi ro là giữa bước 2 và bước 4***  *— `core` đã chạy, bảng còn rỗng. Cách chặn là **Null Object**: điền sẵn cả bảng bằng stub trả `-ENOTSUP` và ghi log, rồi để chip driver ghi đè cái nó có. Biến một cú oops kernel thành một dòng log."*

⭐ **Vì sao đoạn này ăn điểm mạnh:** bạn **tự chỉ ra điểm yếu của hệ mình** *và* **biết tên giải pháp**. Đó là tín hiệu senior — người chỉ kể cái hay của hệ mình nghe như chưa từng vận hành nó.

### ⭐ Câu chốt — nói SỚM, đừng chôn ở cuối

> *"Có **hai tầng** con trỏ hàm, và chúng tồn tại vì **hai lý do khác nhau**:*
> *— tầng `shim_export` là để đi qua **ranh giới license** — driver nền là GPL, driver chip là mã đóng;*
> *— tầng `shim_bridge` là để **đa hình theo chip** — đúng vai trò của vtable, nhưng viết bằng C."*

Người kể được **hai lý do khác nhau** cho hai tầng thì khác hẳn người chỉ nói *"bọn em dùng function pointer cho nó linh hoạt"*.

---

## 💬 Nhận định về kiến trúc D4 — đọc trước khi vào phòng

> Bạn hỏi anh nghĩ gì về kiến trúc này. Dưới đây là đánh giá thẳng, gồm **cả chỗ interviewer sẽ khoan**. Biết trước thì trả lời được; không biết thì bị động.

### ✅ Ba điểm mạnh — nói ra được thì ăn điểm

1. **Nó giải một ràng buộc THẬT, không phải over-engineering.** Ranh giới GPL là ràng buộc pháp lý có thật, và giải pháp là kỹ thuật. Interviewer thích nghe kiến trúc sinh ra từ ràng buộc hơn là kiến trúc sinh ra từ sở thích.
2. **Tách đúng hai mối quan tâm.** Một tầng cho license, một tầng cho đa hình. Nếu gộp làm một thì sau này gỡ bỏ ràng buộc license sẽ phải đập cả hai.
3. **`drv_panel_core` hoàn toàn không biết tên chip nào.** Hơn 10 chip, một codebase nền. Đây chính là giá trị kinh doanh, và nên nói bằng con số: *"hơn 10 dòng chip, một driver nền"*.

### 🔍 Bốn chỗ interviewer sẽ khoan — và câu trả lời trung thực

**① *"Nếu nạp sai thứ tự thì sao?"***

Đây là câu đầu tiên bất kỳ ai làm kernel sẽ hỏi.
- Nếu `drv_panel_core` **extern symbol** từ `shim_export`, thì **trình nạp module tự bắt buộc thứ tự** — `modprobe` đọc `modules.dep` và nạp `shim` trước. Sai thứ tự chỉ xảy ra khi dùng **`insmod` thủ công**, và khi đó nó **báo lỗi rõ ràng** `Unknown symbol in module`, không phải crash âm thầm.
- 🗣️ **Trả lời tốt:** *"`modprobe` lo thứ tự vì nó có dependency theo symbol. Nếu script khởi động dùng `insmod` trực tiếp thì mình tự chịu trách nhiệm thứ tự — và đó là chỗ em sẽ chuyển sang `modprobe` nếu làm lại."*
- ⚠️ **Nhưng tầng 3 thì khác:** `drv_panel_chipA` được nạp **lúc chạy** theo board config, và bảng `panel_ops` **rỗng cho tới khi nó đăng ký xong**. Nếu có ai gọi vào bảng trong khoảng trống đó ⇒ **NULL pointer dereference**, tức kernel oops. Đây là rủi ro thật.

**② *"Slot chưa được điền thì gọi vào là gì?"***

Một bảng con trỏ hàm **không có giá trị mặc định** thì mọi slot trống là một quả mìn. Chip A hỗ trợ 40 hàm, chip B chỉ 30 — 10 slot còn lại là `NULL`.
- 🗣️ **Trả lời tốt:** *"Đúng chỗ yếu. Cách chữa là **Null Object** — điền sẵn cả bảng bằng stub mặc định **trả về `-ENOTSUP` và ghi log**, rồi để chip driver ghi đè cái nó có. Biến một cú oops kernel thành một dòng log và một mã lỗi."*
- Đây là câu **biến điểm yếu thành điểm mạnh**: bạn thừa nhận vấn đề **và** biết tên giải pháp.

**③ *"Thêm một hàm vào giữa bảng thì sao?"*** ⭐ **Câu nguy hiểm nhất**

Bảng con trỏ hàm **chính là một vtable viết tay** — nên nó thừa hưởng **đúng** vấn đề ABI của vtable: **chèn một slot vào giữa làm lệch toàn bộ slot phía sau**. `shim` mới + chip driver cũ (hoặc ngược lại) ⇒ gọi **nhầm hàm**, không phải lỗi nạp module. Nó **chạy**, và nó **sai**.
- ⚠️ **Một con số phiên bản khớp nhau cũng không cứu được** — version bảo vệ *"API có đủ không"*, **không** bảo vệ *"slot có đúng chỗ không"*.
- 🗣️ **Trả lời tốt:** *"Luật bọn em phải giữ là **chỉ thêm vào cuối bảng, không bao giờ chèn giữa, không bao giờ xoá**. Muốn chắc hơn thì thêm **trường `size`/số lượng slot** vào đầu struct để bên nạp kiểm tra được — đó là cách kernel làm với các struct ops có thể mở rộng."*

**④ *"Driver nền là GPL nhưng phụ thuộc module mã đóng — nó còn dùng được symbol GPL-only không?"***

Câu này **khó**, và nó là câu thật.
- Cơ chế: kernel đánh dấu `TAINT_PROPRIETARY_MODULE`, và module **phụ thuộc** module proprietary có thể **mất quyền** dùng symbol `EXPORT_SYMBOL_GPL`. Nghĩa là `drv_panel_core`, dù mang giấy phép GPL, có thể bị chặn dùng một số API lõi.
- 🗣️ **Trả lời trung thực:** *"Em biết kernel có lan truyền taint và có luật chặn symbol GPL-only theo hướng đó. Em **chưa kiểm cụ thể trên phiên bản kernel của bọn em** xem `drv_panel_core` có mất quyền nào không — đó là thứ em sẽ kiểm bằng cách thử link một symbol GPL-only và xem module có nạp được không."*
- ⭐ **Đừng đoán bừa ở câu này.** *"Em chưa kiểm, và đây là cách em sẽ kiểm"* mạnh hơn hẳn một khẳng định sai.

### 🔧 Nếu được làm lại — câu trả lời cho *"em sẽ làm khác chỗ nào?"*

| Đổi gì | Được gì |
|---|---|
| **Nhóm `panel_ops` thành nhiều struct nhỏ theo chức năng** (power, timing, calibration) thay vì một bảng phẳng khổng lồ | Chip chỉ hỗ trợ một phần thì đăng ký đúng phần đó; versioning từng nhóm độc lập; và đây là **cách kernel làm sẵn** — `file_operations`, `net_device_ops` |
| **Null Object mặc định cho mọi slot** | Oops → log + `-ENOTSUP` |
| **Thêm trường `version` + `size`** vào đầu mỗi struct ops | Bắt được lệch ABI **lúc đăng ký**, không phải lúc gọi nhầm hàm |
| **`modprobe` + `MODULE_SOFTDEP`** thay vì insmod theo script | Thứ tự nạp do hệ lo, không do người |
| **Một fake chip driver đăng ký vào cùng bảng** | Test `drv_panel_core` **không cần phần cứng** — đây là thứ đội V&V sẽ rất thích, và nó khớp thẳng dòng JD *"drive the relation with V&V"* |

> ⭐ **Một câu đáng nói nếu có cơ hội:** *"Nhìn lại thì bọn em tự viết lại cái mà kernel vốn đã có — mô hình `struct ops` + `register/unregister`, giống `i2c_driver` hay `net_device_ops`. Nếu làm lại em sẽ bám sát khuôn đó ngay từ đầu, vì người mới vào đọc là hiểu luôn."*
>
> Câu này làm ba việc cùng lúc: cho thấy bạn **biết cách làm chuẩn của kernel**, bạn **tự đánh giá được việc mình làm**, và bạn nghĩ tới **người bảo trì sau**.

---

## 🧭 Chiến lược: vẽ cái nào trước, mở rộng đi đâu

### Cây quyết định

**Mọi câu đều bắt đầu từ một chỗ:**

```
     Ho hoi bat ky cau nao ve he thong
                    |
                    v
          +-------------------+
          |   D1  nhip 1      |  chong tang, 20 giay
          |   (chong tang)    |  roi DUNG LAI, nhin mat ho
          +---------+---------+
                    |
              ho hoi tiep gi?
                    |
                    v
             (tra bang duoi)
```

⚠️ **Đừng bỏ qua bước này kể cả khi câu hỏi rất hẹp.** *"Để em vẽ nhanh bức tranh chung rồi đi vào chỗ anh hỏi"* — chưa ai từ chối câu đó, và nó làm mọi câu sau ngắn đi một nửa.

**Bảng tra — họ hỏi gì thì vẽ gì:**

| Họ hỏi | Vẽ | Câu mở đầu |
|---|---|---|
| *"dimming là gì?"* · mặt họ ngơ | **D0** rồi **quay lại D1** | *"Cứ hình dung ba núm vặn trên một tấm panel…"* |
| *"vì sao app không phải build lại?"* · *"API ổn định thế nào?"* | **D1 nhịp 2** (cái phễu) | *"Trên nhiều app, dưới hơn 10 chip, giữa một hợp đồng."* |
| *"các thành phần tương tác ra sao?"* | **D1 nhịp 3** (hai nhánh) | *"Tách hai vì lượng logic khác nhau."* |
| *"đổi chip thì sửa gì?"* | **D1 nhịp 4** (Factory) | *"Chỉ một backend mới. Không gì ở trên đổi."* |
| *"kể một luồng cụ thể"* · *"tắt màn hình thì sao?"* | **D2** (sequence) | *"Em kẻ bốn bên ra, thời gian chạy xuống."* |
| 🔆 *"kể về S-Box"* · *"adaptive brightness làm sao?"* · **"sao không bị nháy?"** | **D3** (thang thời gian) | *"Có hai hằng số thời gian, và đó chính là chỗ chống nháy."* |
| *"phần kernel thế nào?"* · *"function pointer table?"* · *"GPL?"* | **D4** (ba module) | *"Hai tầng con trỏ, hai lý do khác nhau."* |
| *"nạp sai thứ tự thì sao?"* | **D4b** (sequence nạp) | *"Cái này dễ thấy hơn trên trục thời gian."* |
| *"em sẽ làm khác chỗ nào?"* | **không vẽ** — nói | §🔧 *Nếu được làm lại* ở trên |

> 📌 **D3 là sơ đồ dễ bị bỏ quên nhất** vì nó không nằm trên trục kiến trúc — nhưng **S-Box là project bạn tự chọn để kể** ở câu mở màn, nên xác suất bị hỏi vào nó rất cao. Đừng để nó là sơ đồ duy nhất bạn chưa tập.

### Bốn quy tắc điều hướng

**1. Luôn bắt đầu bằng **D1 nhịp 1**, kể cả khi họ hỏi câu hẹp.** Ba mươi giây dựng bối cảnh giúp mọi câu sau ngắn đi một nửa. *"Để em vẽ nhanh bức tranh chung rồi đi vào chỗ anh hỏi"* — chưa ai từ chối câu đó.

**2. D0 là **phản ứng**, không phải kế hoạch.** Chỉ vẽ khi thấy dấu hiệu họ không theo kịp. Vẽ D0 cho người đã biết là hạ thấp họ.

**3. D4 là **vũ khí, không phải mở màn**.** Nó cần 2–3 phút và nó kéo theo cả loạt câu hỏi khó (thứ tự nạp, ABI, GPL). Vẽ nó khi bạn **đang thắng** và muốn nâng độ sâu — đừng vẽ khi đang lúng túng.

**4. Mỗi sơ đồ chốt bằng **một đánh đổi**.** Sơ đồ mô tả *cái gì*; đánh đổi cho thấy bạn *đã cân nhắc*. Một câu là đủ:

| Sơ đồ | Câu đánh đổi để chốt |
|---|---|
| D1 | *"Chỗ hẹp làm app khỏi build lại, nhưng nó **khoá** hình dạng API — thêm khả năng mới cho một chip là chuyện khó."* |
| D2 | *"Thứ tự do **ràng buộc vật lý**, không phải do phần mềm."* |
| D3 | *"Đổi độ trễ 4 giây lấy khả năng chống nhiễu. Thiết bị cầm tay thì đánh đổi này sai."* |
| D4 | *"Hai tầng con trỏ trả giá bằng **một lần gọi gián tiếp nữa** và **khó debug hơn** — đổi lại là đi qua được ranh giới license."* |
| D4b | *"Nạp động cho phép một driver nền chạy với hơn 10 chip — cái giá là **một cửa sổ mà bảng còn rỗng**, và bọn em phải chặn nó bằng stub mặc định."* |

### Ba câu chuyển cảnh — học thuộc, dùng để giành lại quyền dẫn

> *"Để em vẽ nhanh cái này, sẽ nhanh hơn là em kể."* — dùng khi câu hỏi mơ hồ.
> *"Cái này liên quan tới một chỗ khác, em vẽ thêm được không?"* — dùng để **mở rộng sang sơ đồ bạn mạnh**.
> *"Chỗ này em nhớ không chắc, nhưng cơ chế thì thế này…"* — dùng khi bị hỏi chi tiết không nhớ. **Đừng bịa.**

---

## 🇬🇧 Bản tiếng Anh — vẽ và nói thế nào cho dễ hiểu nhất

### Luật riêng khi vẽ bằng tiếng Anh

**1. Ô ghi DANH TỪ, mũi tên nói bằng ĐỘNG TỪ.** Viết `Factory` trong ô, **nói** *"the factory picks the backend"*. Viết câu dài lên bảng vừa chậm vừa dễ sai ngữ pháp.

**2. Dùng từ NGẮN, GỐC ANH.** Người nghe có thể cũng không phải người bản xứ.

| ❌ Đừng viết/nói | ✅ Dùng |
|---|---|
| *utilize · leverage* | **use** |
| *implement the functionality* | **do it** / **handle it** |
| *multi-chipset abstraction layer* | **one interface, many chips** |
| *display enhancement solution* | **picture quality features** |
| *in order to* | **to** |
| *at this point in time* | **then** / **now** |

**3. Ba từ chuyên ngành PHẢI nói đúng** — nói sai là mất tín nhiệm ngay:

| Từ | Nghĩa nên nói kèm lần đầu |
|---|---|
| **backlight** | *"the LEDs behind the panel"* |
| **frame rate conversion** | *"filling in frames between the ones you actually have"* |
| **timing controller (TCON)** | *"the chip that drives the rows and columns of the glass"* |

**4. Nhãn tiếng Anh cho ranh giới — viết đúng cụm này:** `C ABI` · `user / kernel boundary` · `ioctl` · `GPL boundary` · `function pointer table`. Đây là thuật ngữ chuẩn, người nghe nhận ra ngay.

### Kịch bản tiếng Anh cho từng sơ đồ

**D0 — ba núm vặn, ~30 giây**
> *"Think of it as three knobs on one panel. **How bright** — that's dimming; there are LEDs behind the panel and we dim them per region, so dark scenes look deeper and use less power. **How smooth** — that's frame rate conversion; the content is twenty-four frames a second, the panel runs at sixty, so we generate the frames in between. And **how the glass is driven** — that's the timing controller, the chip that turns pixel data into the electrical timing the panel needs."*

**D1 — kiến trúc, ~60 giây** *(ba nhịp — vẽ tới đâu nói tới đó)*

> *[nhịp 1 — chồng tầng]*
> *"At the top is the application. Below it we expose a **stable C API**. Below that, the shared library. Then this dashed line is the **user–kernel boundary** — we cross it with an `ioctl`. Then the kernel driver, then the SoC."*
>
> *[nhịp 2 — cái phễu]*
> *"Let me redraw that middle part, because it's the interesting bit. **Several applications above, more than ten chip families below — and exactly one contract in the middle.** That's what we call the **narrow waist**. It's the reason the application **never gets rebuilt when we change chips**."*
>
> *[nhịp 3 — hai nhánh]*
> *"Under the waist we split into two branches, because **the amount of logic is different**. **Dimming** is algorithm-heavy, so it lives in **user space, in C++**. **Frame rate conversion and the timing controller** are thin register commands, so they stay **in the kernel**.*
>
> *The rule is: **heavy logic goes up to user space, thin register work stays in the kernel**."*
>
> *[chỉ khi họ hỏi "how do you pick the chip?"]*
> *"A factory reads the board configuration **at runtime** and builds the right algorithm-and-backend pair. New chip means **one new backend** — nothing above changes."*

**D2 — sequence diagram, ~60 giây**

> *[vừa kẻ các đường đời vừa nói]*
> *"Let me put the four parties across the top — the app, the library, the base kernel driver, and the chip driver. Time runs **downward**."*
>
> *[điền mũi tên, chỉ vào từng số]*
> *"The app calls `set_power(false)`.*
> ***One** — the library **ramps the brightness down** over ten steps. We don't cut it, or the user sees a flash.*
> ***Two** — it crosses into the kernel with an `ioctl`.*
> ***Three** — the base driver calls `blank` **through the function pointer table**, so it **never knows which chip it's talking to**.*
> ***Four** — the chip driver cuts the power rails, in this order: panel, backlight, logic.*
>
> *And here's the part I'd highlight: **that order isn't a software choice — it's what the hardware physically needs.** Powering up runs the **exact same sequence in reverse**."*

> 💬 **Nếu họ hỏi *"what if step three fails?"*** — *"We **run the rest of the path anyway**. Stopping halfway leaves the hardware in a half-off state, which is worse. We record the failure, finish the sequence, and return an aggregated error code."*

**D3 — vsync, ~45 giây**
> *"There are **two separate time constants**, and that's what removes the flicker.*
> *Every **four seconds** we read the sensor and compute a new target — long enough that someone flicking a light switch doesn't move it.*
> *In between, brightness **walks toward the target in ten steps of four hundred milliseconds** each, so your eye never catches a jump.*
> *Four hundred milliseconds is **twenty-four vsync periods** at sixty hertz, so every step lands on a frame boundary.*
> *The cost is a **four-second reaction time**. We accepted that because the device is fixed in a meeting room. For a handheld device that trade-off would be wrong — there I'd sample fast and use a moving average instead."*

**D4 — kernel, ~70 giây**
> *"There are **three modules**. The first one loaded is a **proprietary shim**: it exports a set of function pointers, and it holds an **ops table** that starts out empty.*
> *Second is the **base driver, which is GPL**. It externs those pointers from the shim.*
> *Third, at runtime, the base driver reads the board configuration and loads the **chip-specific driver**, which **registers its functions into the ops table**.*
> *From then on, a call goes base driver → shim → the right chip, and **nobody needs to know anybody's name**.*
> *There are **two levels of indirection, and they exist for two different reasons**: one crosses a **licence boundary** — the base driver is GPL, the chip drivers are closed — and the other gives us **polymorphism in C**, which is what a vtable does in C++."*


**D4b — load order, ~40 giây** *(chỉ khi họ hỏi "what if they load in the wrong order?")*

> *"Let me draw the **load order** — it's easier to see on a timeline.*
> ***One**, the shim loads first: the ops table exists but it's **empty**, and the symbols are exported.*
> ***Two**, the base driver loads. It externs those symbols, so **the module loader enforces the order for us** — `modprobe` reads the dependency and pulls the shim in first. If you use plain `insmod` in the wrong order you get a clear **`Unknown symbol in module`**, not a silent crash.*
> ***Three**, the base driver reads the board config and loads the chip driver.*
> ***Four**, the chip driver **registers its functions into the table**.*
>
> *And here's the part worth pointing at: **between step two and step four the base driver is alive but the table is still empty.** Calling into it there is a null dereference. The fix is a **null-object default** — prefill every slot with a stub that logs and returns `-ENOTSUP`, then let the chip driver override what it supports."*

### Bốn câu cứu hộ bằng tiếng Anh — học thuộc

> *"Let me draw that — it'll be faster than explaining."*
> *"Can I add one more box here? It connects to what you just asked."*
> *"I'm not certain about that detail, but the mechanism is…"* — **đừng bịa; đây là câu thay thế.**
> *"Sorry — let me say that again more simply."* — dùng khi bạn tự thấy câu vừa nói lủng củng. Nói ra rồi nói lại **tốt hơn** là cố gỡ.

---

## 📄 Trang gấp — liếc 10 phút trước khi vào phòng

> Bản thu nhỏ của cả sáu sơ đồ. Không đọc để học — **liếc để nhớ hình dạng**.

```
 D0  ba num van          D1.1 chong tang       D1.2 cai pheu
     |                        App                 \  |  /
 sang? muot? dieu khien?      -- C API --          C API
     den  frame   timing      libdisplay          /  |  \
                              -- ioctl --        chipA B C
                              driver / SoC

 D1.3 hai nhanh          D1.4 factory          D2  sequence set_power
        C API                 Factory          App  lib  core  chip
       /     \               /      \           |----->|
  Dimming   FRC/TCON      Algo    Backend       | 1 ramp
   ioctl    panel_ops                           | 2 ioctl -->|
       \     /                                  |   3 blank -->|
         SoC                                    |   4 power_off->|

 D3  hai hang so thoi gian        D4  ba module            D4b nap
 vsync |||||||||| 16.7ms              shim                 1 shim (RONG)
 step  ^      ^      ^  400ms      (export+bridge)         2 core extern
 target^             4s          core(GPL)  chipA(prop)    3 core->chipA
       doc sensor, tinh lai        (a)extern  (b)dang ky   4 chipA dang ky
                                                           ^ CUA SO RONG
```

**Một câu đánh đổi cho mỗi sơ đồ — thuộc đúng sáu câu này:**

| | Câu |
|---|---|
| D1 | *Chỗ hẹp làm app khỏi build lại, nhưng nó **khoá** hình dạng API.* |
| D2 | *Thứ tự do **ràng buộc vật lý**, không phải do phần mềm.* |
| D3 | *Đổi **độ trễ 4 giây** lấy khả năng chống nhiễu. Thiết bị cầm tay thì sai.* |
| D4 | *Hai tầng con trỏ: một cho **license**, một cho **đa hình**.* |
| D4b | *Nạp động đổi lại bằng **một cửa sổ bảng còn rỗng** — chặn bằng stub mặc định.* |
| D0 | *(không có đánh đổi — chỉ để người nghe bắt kịp)* |

**Đường đi mặc định:** `D1.1` → *(họ hỏi gì thì rẽ đó)* → `D1.2 phễu` · `D1.3 hai nhánh` · `D2 sequence` · `D4 → D4b`.

**Ba câu giành lại quyền dẫn:** *"Để em vẽ nhanh, sẽ nhanh hơn là kể."* · *"Em vẽ thêm một ô nữa được không?"* · *"Chỗ này em nhớ không chắc, nhưng cơ chế thì thế này…"*

---

## ✅ Checklist trước khi vào phòng

- [ ] Vẽ được **D1 nhịp 1** trong **20 giây** mà không nhìn file này — **ô chỉ có danh từ**, nhãn ở lề
- [ ] Vẽ được **D1 nhịp 2 (cái phễu)** và nói được câu *"nhiều app trên, hơn 10 chip dưới, một hợp đồng ở giữa"*
- [ ] **D2: kẻ HẾT 4 đường đời TRƯỚC**, rồi mới điền mũi tên từ trên xuống — tập đúng thứ tự này 3 lần
- [ ] Nói được **vì sao thứ tự 4 bước của D2 không đổi chỗ được** (ràng buộc vật lý, không phải phần mềm)
- [ ] Nói được **hai hằng số thời gian** của D3 kèm **đánh đổi**
- [ ] Nói được **hai lý do khác nhau** cho hai tầng con trỏ hàm ở D4
- [ ] Vẽ được **D4b** và chỉ ra được **cửa sổ nguy hiểm** giữa bước (2) và (4)
- [ ] Thuộc **một câu đánh đổi** cho mỗi sơ đồ
- [ ] 🇬🇧 Đọc to **D1 và D2 bằng tiếng Anh**, bấm giờ, mỗi cái **≤ 60 giây**
- [ ] Biết trước **bốn câu khoan** ở §nhận định D4 — nhất là câu ④ (GPL taint), câu **không được đoán bừa**

---

⬅️ [study-plans](study-plans/README.md) · Từ vựng: [in-practice/README.md](../11-design-patterns/in-practice/README.md) · Kiến trúc đầy đủ: [A1-baseline-libdisplay.md](../11-design-patterns/in-practice/A1-baseline-libdisplay.md)
