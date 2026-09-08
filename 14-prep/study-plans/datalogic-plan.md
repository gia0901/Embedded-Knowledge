# 🎯 Datalogic — Embedded Linux Engineer (C++17, Linux) · Plan ôn

> Kế hoạch ôn **nhắm đúng JD** [Embedded Linux Engineer @ Datalogic Việt Nam](https://itviec.com/it-jobs/embedded-linux-engineer-c-linux-datalogic-viet-nam-2835).
> Không phải kiến thức mới — mọi mục **link tới tài liệu + bank câu hỏi** sẵn có. Ký hiệu: 🎯 xác suất gặp cao · 🎤 phiên mock · 🔁 ôn lặp lại · 🧪 lab ngồi máy.

---

## 📍 Tiến độ hiện tại — **RESUME Ở ĐÂY** (nguồn tracking DUY NHẤT)

> ⚠️ **Đây là TRẠNG THÁI, không phải nhật ký.** Sau mỗi phiên: **SỬA** đúng bốn chỗ — *Buổi gần nhất* · *Lịch sử điểm* · *▶️ LÀM TIẾP* · ô `Xong?` của bảng lab — và **không thêm dòng mới**.
> *(File này từng phình lên 16.000 từ vì mỗi phiên **thêm** một đoạn thay vì **sửa** dòng cũ, dẫn tới ba mốc "buổi gần nhất" mâu thuẫn nhau. Dọn 2026-09-08.)*
> Diễn biến từng phiên sống ở [`sessions/`](../mock-interview/sessions/) · câu yếu ở [weak-register](../mock-interview/weak-register.md) · lỗ hổng tài liệu ở [gap-register](gap-register.md). Ba nơi đó là lịch sử; chỗ này không phải.

### Trạng thái

| | |
|---|---|
| **Plan** | 🎯 Datalogic (file này), bắt đầu **2026-08-03** |
| **Buổi gần nhất** | `retention track all` — **2026-09-04**, 30/40 = **3.00**, 10 câu ([log](../mock-interview/sessions/2026-09-04--retention--all.md)) |
| **Retention kế tiếp** | **18–23/09** — 8 câu, góc hỏi đã ghi sẵn trong [weak-register](../mock-interview/weak-register.md). Không có gì quá hạn |
| **Độ phủ** | **đo bằng lệnh**, đừng chép số vào đây — cách đếm ở [bank/README](../mock-interview/bank/README.md). Lần đo 06/09: `178/457 = 38%`; **`NET` 0%** 🔴 · `BLD` 13% · `BSP` 29% |
| **Lịch sử điểm** | 3.25 · 3.3 · 2.83 · 3.17 · 3.67 · 3.0 · 3.0 · 2.67 · **2.1** · 3.67 · **3.92** · 3.67 · 3.33 · 3.0 · **3.42** · 3.33 · 3.00 *(theo thứ tự thời gian 03/08 → 04/09; ⚠️ **không so ngang** — `rapid` và `daily`/`by-level` dùng thang khác nhau, xem [config §4](../mock-interview/config.md))* |
| **Đang treo** | 🧪 lab BSP 6/8 chưa làm · 🧪 lab DBG 12/12 chưa làm · `NET` chưa hỏi một câu nào |

⭐ **Chẩn đoán còn hiệu lực (15–16/08):** `rapid` (đo **T1** — biết & nói gọn) = **3.67** · `by-level 🟠` (đo **T2** — vận dụng) = **2.1**. ⇒ **Nền kiến thức CHẮC, chỗ hụt nằm ở tầng VẬN DỤNG.** Hệ quả: **không cần đọc thêm tài liệu mới** — cần chuyển thứ đã biết thành *quyết định* và *dòng code sửa*. Ở phỏng vấn thật, áp lực sẽ dồn vào **phần giữa**, khi interviewer hỏi *"giờ bạn làm gì"*.

### ▶️ LÀM TIẾP

**Hàng đợi, theo thứ tự:**

| # | Việc | Vì sao xếp ở đây |
  |---|------|------------------|
| **1** | 🧪 `BSP-031` *(→ 032 → 033)* | phần cứng vừa thông, đừng để nguội |
| **2** | `/mock rapid track networking` | 🔴 **`NET` 0% — domain DUY NHẤT chưa từng hỏi một câu**, và đã bị gắn cờ *"rủi ro số 1"* từ **24/08** mà **13 ngày sau vẫn 0%**. 12 câu ~15′. Vá luôn `LNX-005` (*TCP luồng byte*) đang **đứng yên 2 phiên ở 2đ** và nằm đúng vùng này |
| **3** | 🗣️ `RES-016` — thuộc **một câu** chặn dòng AI | **0 điểm**, và là việc có **tỉ lệ giá trị/công sức cao nhất** trong cả plan: đúng một câu + một ví dụ. Xem [bank RES-016](../mock-interview/bank/resume.md) |
| **4** | 🗣️ `RES-001` — đọc to **bản nói 45 giây**, ghi âm | lỗi lặp **3 lần**; [bản nói 4 nhịp đã viết sẵn](../mock-interview/bank/resume.md), chỉ cần nói to và bấm giờ |
| **5** | `/mock rapid track build-systems` | `BLD` **13%** — vừa thêm **17 câu** CI/test-farm chưa hỏi câu nào, mà đó là trụ JD *"Yocto + CMake/CI"* |
| **6** | `/mock daily track bsp` ở **T2** | chỉ sau khi đã đọc lại; [config §6](../mock-interview/config.md) cấm đưa câu 0–2đ thẳng lên T2 |

📌 **Resume đổi 07/09 — mục SDM:** bỏ nhãn *"C++17"* (app build tới C++20, thứ C++17 duy nhất dùng là `std::optional` ⇒ nhãn đó tự mời câu hỏi không trả lời được) và **thêm thành tựu**: **Preset + import/export preset giữa các màn hình, xong trong 1 tháng, kịp release 1.0** *(phần lưu JSON đã có sẵn trước khi vào — không nhận)*. Dòng mới ⇒ câu hỏi mới **[RES-022](../mock-interview/bank/resume.md)** *(phạm vi · ranh giới công lao · đánh đổi đã chấp nhận)* — **chưa nói lần nào**, ghép vào việc **4** vì cùng là bài nói to bấm giờ. Sửa song song ở `RESUME_bosch.tex`; `RESUME_cpp_linux.tex` **chưa có** thành tựu này.

### 🧪 Hai bộ lab — trạng thái

**Bộ lab DBG — 12 bài** *(7 bài gốc 17/08 + 5 bài gdb 08/09)*: bank có type thứ tư, **câu `lab` 🧪 "NGỒI MÁY LÀM"** (code có bug thật + nhiệm vụ + **output thật đã chạy** để đối chiếu; quy ước ở [bank/README.md](../mock-interview/bank/README.md) và [CLAUDE.md §6](../../CLAUDE.md)). Xếp theo **TRIỆU CHỨNG, không theo công cụ** — vì ngoài đời luôn bắt đầu từ triệu chứng, và đó đúng là lỗ hổng đã đo (*chẩn đoán được, chọn công cụ không được*):

    | Triệu chứng | Bài | Công cụ | Vá lỗ hổng | Xong? |
    |---|---|---|---|---|
    | Chết ngay khi khởi động | [DBG-030](../mock-interview/bank/debugging.md) | `strace` | DBG-005 | ⬜ |
    | Rò tài nguyên (fd/RAM tăng dần) | [DBG-031](../mock-interview/bank/debugging.md) | `/proc/fd` | **LNX-023** (sổ yếu) | ⬜ |
    | Sai dữ liệu ngầm, không crash | [DBG-032](../mock-interview/bank/debugging.md) | ASan | DBG-024/025 | ⬜ |
    | Crash, mình không có mặt lúc đó | [DBG-033](../mock-interview/bank/debugging.md) | core dump + `gdb bt` | DBG-003 | ⬜ |
    | **Treo**, không crash không log | [DBG-034](../mock-interview/bank/debugging.md) | `/proc` `State`+`wchan` | LNX-023 | ⬜ |
    | Lúc được lúc không (đa luồng) | [DBG-035](../mock-interview/bank/debugging.md) | TSan | DBG-013 | ⬜ |
    | Đọc biến chưa khởi tạo | [DBG-036](../mock-interview/bank/debugging.md) | **valgrind** *(ASan mù)* | DBG-011 | ⬜ |
    | Lỗi ở lần lặp thứ N giữa hàng nghìn lần | [DBG-037](../mock-interview/bank/debugging.md) | `break … if` · `ignore` | DBG-002/017 | ⬜ |
    | Bug chỉ có ở `-O2`, `-O0` thì hết | [DBG-038](../mock-interview/bank/debugging.md) | `-Og` · `-fno-inline` · UBSan | **DBG-001** *(vốn chỉ dạy `-O0`)* | ⬜ |
    | Binary đã strip, chỉ còn core dump | [DBG-039](../mock-interview/bank/debugging.md) | `objcopy` · build-id · `set sysroot` | DBG-003 · nối [CI](../../06-build-systems/ci-and-test-farm.md) | ⬜ |
    | Treo đa luồng — phải chỉ ra vòng chờ | [DBG-040](../mock-interview/bank/debugging.md) | `thread apply all bt` | DBG-013 · OS-003 | ⬜ |
    | Breakpoint không ăn trong `.so` nạp lúc chạy | [DBG-041](../mock-interview/bank/debugging.md) | gdb pending bp | **DBG-018** · nối [linking-loading](../../07-shared-libraries/linking-loading.md) | ⬜ |

    **💻 Chạy ở đâu:** cả 12 bài là **userspace thuần** — chạy được trên **server Ubuntu bằng user thường**, không cần máy cá nhân, không cần root. Thứ duy nhất cần quyền là **cài gói**: `gcc/g++`, `gdb`, `strace`, `valgrind` *(ASan/TSan đi kèm gcc)*. Không cài được `valgrind` thì **chỉ DBG-036 chết**, mười một bài kia nguyên vẹn. ⭐ **DBG-041 khác các bài còn lại**: nó chạy trên **codebase nhiều file + `.so`** — dùng luôn `project_implementation/HAL_layer`, không viết file mới. Ba bẫy môi trường (`ptrace_scope=1` · `core_pattern` bơm vào apport · TSan xung đột ASLR kernel ≥6.5) đều đã có đường vòng ghi sẵn trong chính đề bài 033/034/035 — **không sửa sysctl trên máy dùng chung**.

**Bộ lab BSP trên phần cứng thật — 8 bài** ([BSP-031…038](../mock-interview/bank/bsp.md), mục *J — LAB NGỒI MÁY*). BeagleBone Black + 2 thẻ SD + **cáp USB–TTL 3.3V (bắt buộc)**.

    ⭐ **Cấu trúc KHÁC bộ DBG có chủ đích** — vì nhắm **lỗ hổng khác**:

    | | DBG lab | **BSP lab** |
    |---|---|---|
    | Lỗ hổng đã đo | *"chẩn đoán được, **chọn công cụ** không được"* | *"**thuộc bài ≠ hiểu bài**"* (19/08) |
    | Cấu trúc | **Triệu chứng trước** | **Khái niệm trước** — triệu chứng thành bước kiểm chứng |

    Ở DBG mô hình **đã có**, chỉ thiếu ánh xạ triệu chứng→công cụ. Ở BSP **mô hình chưa có**, nên đưa triệu chứng ra trước là bắt chẩn đoán một hệ thống chưa hình dung được. *(Lỗi này interviewer mắc lúc đầu khi sao chép cấu trúc DBG — ứng viên phát hiện và sửa.)*

    **Bốn bước, áp cho mọi bài:** ① **ĐỌC** khái niệm (ngắn) → ② **QUAN SÁT** nó chạy đúng, bắt log thật → ③ **PHÁ có chủ đích** (viết **dự đoán ra giấy TRƯỚC**) → ④ **ĐỐI CHIẾU** bảng tự dựng với bảng trong repo.
    ⇒ Bước ③ là chỗ khác biệt: **không chờ triệu chứng, mà gây ra nó** — biết ground truth nên mọi suy luận ở ④ kiểm chứng được. Chỗ **dự đoán sai** = chỗ mô hình còn hổng.

    | # | Bài | Vá lỗ hổng | Xong? |
    |---|---|---|---|
    | 🅰️ | [BSP-031](../mock-interview/bank/bsp.md) — 4 giai đoạn boot, nhận ra trên log thật | BSP-001/002/022 · **đóng vòng "thuộc bài ≠ hiểu bài"** | 🟡 **② XONG 06/09** (log đã dán, 5/5 ranh giới đúng) · ③④ chờ thẻ tự `dd` |
    | 🅰️ | [BSP-032](../mock-interview/bank/bsp.md) — U-Boot trao tay kernel (bootargs/loadaddr/DTB) | BSP-002 | ⬜ |
    | 🅰️ | [BSP-033](../mock-interview/bank/bsp.md) — rootfs & initramfs, tự chứng minh khi nào KHÔNG cần | 🔴 **BSP-029 (sổ yếu, 2đ)** | ⬜ |
    | 🅰️ | [BSP-034](../mock-interview/bank/bsp.md) — đo boot chậm *(bài duy nhất giữ dạng triệu chứng)* | câu chuyện tối ưu **không vướng NDA** | ⬜ |
    | 🅰️ | [BSP-035](../mock-interview/bank/bsp.md) — FIT + ký RSA, U-Boot **từ chối** image sửa 1 byte | **BSP-004** + gap 🟠 *secure boot* | ⬜ |
    | 🅱️ | [BSP-036](../mock-interview/bank/bsp.md) — Yocto dựng lại đúng thứ vừa làm tay, **so hai bên** | BSP-017 | ⬜ |
    | 🅱️ | [BSP-037](../mock-interview/bank/bsp.md) — bbappend + **bắt tận tay chỗ `shlibs` mù với `dlopen`** | 🔴 **BSP-018** *(19/08 trả lời "chưa rõ")* | ⬜ |
    | 🅱️ | [BSP-038](../mock-interview/bank/bsp.md) — sstate, đo ba phép đo thật | BSP-017/018 | ✅ **XONG 06/09** — 3/3 phép đo + xác minh |

    **🎯 TÁCH hai thẻ, ba giai đoạn — quyết định có lý do, không phải cho tiện:** Thẻ A làm chuỗi boot **bằng tay** trước, Thẻ B để **Yocto dựng lại** sau. Ba căn cứ: ① học hai thứ khó cùng lúc thì hỏng không biết hỏng ở đâu · ② vòng lặp phản hồi lệch ~100 lần (sửa env U-Boot: vài giây; qua bitbake: vài phút) · ③ ⭐ **làm tay trước khiến giá trị Yocto trở nên hiển nhiên** — nếu Yocto đi trước, U-Boot chỉ là hộp đen bitbake nhả ra. *(Máy build Ryzen 9 9950X/24 luồng ⇒ **thời gian build KHÔNG phải lý do tách**; lý do là sư phạm.)* Giữ **Thẻ A luôn boot được** làm thẻ cứu hộ.

    **🔧 SETUP — một chỗ duy nhất: [14-prep/lab-setup.md](../lab-setup.md)** *(phần cứng cần mua · đấu nối serial J1 · gói host · bố cục thẻ SD · Yocto host deps · **6 bẫy đã biết** · **checklist sẵn sàng**)*. Không chép vào từng lab — 8 bài dùng chung. ⭐ **Chưa tick đủ checklist thì đừng bắt đầu bài 031.**

    **⚠️ Ba cảnh báo phần cứng (chi tiết ở lab-setup):** ① **không có cáp USB–TTL 3.3V thì toàn bộ Thẻ A vô nghĩa** (cáp 5V làm hỏng board) · ② **eMMC của BBB tranh boot với thẻ SD** — không giữ nút **S2** lúc cấp nguồn thì triệu chứng giống hệt "image mình hỏng", đây là bẫy số 1 của người mới · ③ 🔴 **secure boot thật KHÔNG làm được trên BBB** — AM335x bán lẻ là **GP silicon**, eFuse chưa blow, không có chain of trust từ ROM. Làm được (và đúng thứ interview hỏi) là **U-Boot verified boot**. Nói đúng ranh giới này ở phỏng vấn là **điểm cộng**.

    **⬜ Hoãn có chủ đích: bộ lab DBG** — 12/12 chưa làm bài nào. Căn cứ: `DBG` **3.67** vs `BSP` **3.33** (thấp nhất), và BSP là trụ số 1 JD. Đã ghi vào [gap-register](gap-register.md) để lần sau không ai đảo ngược trong im lặng.

    **Bộ công cụ lõi đã chốt:** `gdb` · `strace` · ASan · `/proc`+`/sys` · valgrind (đúng một việc: biến chưa khởi tạo) · TSan (đa luồng). **Cắt hẳn:** `perf`, `ltrace`, `gdbserver`/`addr2line` bài riêng — giữ ở mức câu `concept` là đủ. Lý do cắt: phỏng vấn embedded hỏi *"nó crash/treo"* nhiều hơn hẳn *"nó chậm"*.
    **⚠️ Ba bẫy môi trường đã đo thật và viết thẳng vào bài** (đây là thứ làm người ta bỏ cuộc, không phải kiến thức debug): `core_pattern` pipe vào apport ⇒ `ulimit -c` một mình **không đủ**, apport còn **vứt luôn** core của binary tự build · `ptrace_scope=1` chặn `gdb -p` · **TSan chết ngay khi khởi động trên kernel 6.x** vì xung đột ASLR, phải `setarch -R`.
    **📌 Phát hiện đáng nhớ:** cùng một data race, build `-O2` thì **5/5 lần chạy đều ra kết quả ĐÚNG** (compiler gộp vòng lặp thành một phép cộng), `-O0` thì sai 4/5. Và ASan **làm lộ** giá trị rác (`0xbebebebe`) nhưng **không báo lỗi**, thoát mã 0.

### 📌 Bài học còn hiệu lực

*Chín điều rút từ các phiên đã chạy. Đây là phần duy nhất của lịch sử vẫn còn thay đổi cách ôn — chi tiết ở log gốc.*

| # | Bài học | Nguồn |
|---|---|---|
| 1 | **Nền chắc, hụt ở vận dụng** — T1 3.67 vs T2 2.1 ⇒ đọc thêm tài liệu không chữa được | [15/08](../mock-interview/sessions/2026-08-15--by-level--linux-sysprog.md) · [16/08](../mock-interview/sessions/2026-08-16--rapid--debugging.md) |
| 2 | **Độ phủ KHÔNG đo được độ vững** — `DRV` phủ 0% nhưng đạt **3.92** vì trùng việc hằng ngày ⇒ ưu tiên = *phủ thấp × **chưa có bằng chứng** về độ vững* | [17/08](../mock-interview/sessions/2026-08-17--rapid--drivers-dt.md) |
| 3 | **Lỗi ĐÓNG GÓI ≠ lỗ hổng kiến thức** — cùng một kiến thức, đổi khung câu hỏi (kỹ thuật → resume) thì không truy xuất được. Chữa bằng **nói to bấm giờ**, không phải đọc thêm | [18/08](../mock-interview/sessions/2026-08-18--rapid--resume.md) |
| 4 | **Thuộc bài ≠ hiểu bài** — tái hiện bank gần nguyên văn nhưng không hiểu chính cụm từ vừa nói. Nói ra một thuật ngữ là **mời interviewer hỏi vào đúng nó** | [19/08](../mock-interview/sessions/2026-08-19--rapid--bsp.md) |
| 5 | **"Repo không có tài liệu về X" trong khi repo CÓ** — 2 phiên liên tiếp. Không phải lỗ hổng kiến thức mà là **thói quen tra cứu**: chưa chắc thì `grep`, đừng kết luận | [18/08](../mock-interview/sessions/2026-08-18--rapid--resume.md) · [19/08](../mock-interview/sessions/2026-08-19--rapid--bsp.md) |
| 6 | **Rơi vì REPO hụt, không phải ứng viên quên** — mẫu lặp **4 lần**. Trước khi kết luận "quên", kiểm xem bank/tài liệu có thật sự dạy điều đó không | [21/08](../mock-interview/sessions/2026-08-21--daily--cpp-system.md) |
| 7 | **STAR phải kết bằng CON SỐ** — 0/12 câu tự đi tới R ⇒ **2.42**; dựng khung trong một ngày ⇒ **3.42**, số câu phải probe 6/12 → **0/12** | [23/08](../mock-interview/sessions/2026-08-23--rapid--behavioral.md) · [24/08](../mock-interview/sessions/2026-08-24--rapid--behavioral.md) |
| 8 | **Khẳng định không đối chiếu được** — nói *"đã build minimal image trên BBB"* trong khi tracking ghi 8/8 lab ⬜. Chốt 06/09: **đúng một nửa** (board + serial thật, "build image" sai). Là **gộp nhầm**, không phải bịa — nhưng interviewer không phân biệt được hai thứ đó | [29/08](../mock-interview/sessions/2026-08-29--rapid--resume.md) |
| 9 | **Số đo bác bỏ giả định, không phải ngược lại** — interviewer nghi phép đo `BSP-038` sai vì *"build U-Boot phải mất vài phút"*; chạy lại ra `real 0m20,096s`. Giả định sai, số đo đúng | [BSP-038](../mock-interview/bank/bsp.md) |

### Nợ lại

- ① ⬜ **Bài coding** — `SocketHandle` ([CPP-054](../mock-interview/bank/cpp.md)) + `ring_buffer_v2` bản mutex ([COD-006](../mock-interview/bank/coding.md)), làm từ file trống qua `/mock coding track cpp-system`. *(Cần một phiên mock, không tự làm hộ được.)*
- ② 🟡 **Nợ chất lượng bank — đang co dần:** ~68 câu 🟠🔴 còn đáp án < 700 ký tự (chưa đạt khung 5 phần của [bank/README](../mock-interview/bank/README.md)). **Không viết lại hàng loạt** — [config §1 Bước 4](../mock-interview/config.md) bắt nâng cấp đáp án của câu ≤ 2 điểm ngay sau mỗi phiên, nên nợ tự co đúng chỗ đang ôn. *(Câu 🟢 ngắn là **đúng chuẩn** — trần cứng có chủ đích, đừng giãn.)*
- ③ 🔴 **Nợ tài liệu** → đã chuyển hẳn sang [gap-register](gap-register.md). Giữ bảng chi tiết ở hai nơi chính là lỗi *"một sự thật, hai chỗ"*.
  > **Luật rút ra:** trước khi đưa một câu 🟠🔴 vào pool ôn, **kiểm link nguồn có thật sự chứa nội dung không**. Câu không có tài liệu nền thì hỏi ra chỉ đo được *"đã từng gặp ngoài đời chưa"*, không đo được kết quả ôn tập.

### Sổ yếu

**Nguồn duy nhất: [weak-register.md](../mock-interview/weak-register.md)** — trạng thái từng câu, cột *"lần sau hỏi mức nào"*, và bảng 🔁 *Lịch kiểm tra lại*. **Không chép danh sách câu ra đây** (đã từng lệch: bản chép còn ghi `OS-003`/`CPP-029`/`DP-002` là điểm yếu trong khi cả ba đã được gỡ).

---

## Hai nguyên tắc xuyên suốt (đọc trước)

**① Ưu tiên TƯ DUY, không phải cú pháp.** Đặc biệt với **build system (Yocto/CMake/CI)**: phỏng vấn hỏi *"layer/bbappend để làm gì", "sstate giải quyết gì", "DEPENDS vs RDEPENDS", "vì sao CMake target-based"* — tức **mental model** (vấn đề → kiến trúc → đánh đổi), **không** bắt viết recipe/CMakeLists chính xác. Nguyên tắc này áp cho mọi mảng: nói được *bản chất & khi nào dùng* quan trọng hơn nhớ flag.

**② Xen kẽ + lặp lại ngắt quãng.** Hai cơ chế kéo kiến thức cũ quay lại:
- **🔁 Khởi động trộn (~5–10′) mỗi buổi:** `/mock weak-review`, hoặc rút 2–3 câu đến hạn trong bảng [🔁 Lịch kiểm tra lại](../mock-interview/weak-register.md). Trộn domain là **cố ý** — phỏng vấn thật cũng hỏi ngẫu nhiên. Câu đến hạn tụt < 3 điểm ⇒ kéo về sổ yếu.
- **🔁 Ôn dồn tích:** mỗi đợt chốt quét *tất cả* phần đã học, không chỉ phần vừa rồi.

**③ Đọc/Hiểu chuẩn bị TRƯỚC; mock cứ để NGẪU NHIÊN.** Phần đọc là *chuẩn bị*; mock **được phép hỏi lệch chủ đề** (rút từ track rộng + weak-register + retention) để luyện **phản ứng**. Bị hỏi trúng chỗ chưa ôn là **tín hiệu** để ghi vào weak-register, không phải lỗi của plan.

---

## JD → tài liệu (bản đồ)

| Trụ JD | Đọc | Test (bank/mock) |
|---|---|---|
| **C++17** | [02-modern-cpp](../../02-modern-cpp/), [EMC summary](../../15-book-summaries/effective-modern-cpp.md) | `CPP-*` (nhất là 026, 037, 040–051 emc) |
| **Linux userspace + debug** 🎯 | [04-linux-system-programming](../../04-linux-system-programming/), [09-debugging](../../09-debugging/) | `LNX-*`, `DBG-*` |
| **Kernel space** | [05-drivers-device-tree](../../05-drivers-device-tree/), [lkd](../../15-book-summaries/lkd/) | `DRV-*`, `BSP-B` |
| **Drivers I2C/SPI/PCI/USB** 🎯 | [driver-basics](../../05-drivers-device-tree/driver-basics.md), [pci-usb-drivers](../../05-drivers-device-tree/pci-usb-drivers.md) | `BUS-*`, `DRV-019…027` |
| **Yocto + CMake/CI** *(học TƯ DUY, không cú pháp — xem nguyên tắc ①)* | [yocto](../../06-build-systems/yocto.md), [cmake](../../06-build-systems/cmake.md), [cross-compilation](../../06-build-systems/cross-compilation.md) | `BLD-*`, `BSP-017…019` |
| **Design subsystem + lead + code review** | [10/system-design](../../10-thinking/system-design.md), [07-shared-libraries](../../07-shared-libraries/) | `SD-*`, `BEH-*` |

**Chạy mock cho JD này:** dùng `track bsp` (trụ chính) hoặc `track all` khi muốn trộn đúng phổ domain liệt kê trên.

---

## 🔄 Ba luật vận hành phiên mock *(chốt 2026-08-17, vẫn đang áp dụng)*

> **Bối cảnh:** sau 15/28 ngày, `CPP` 71% và `LNX` 59% trong khi **`DRV` 0%**, `BSP` 6%, `NET`/`BEH`/`RES` 0%. Nguyên nhân: **độ sâu ăn hết ngân sách thời gian** — [config §6](../mock-interview/config.md) viết để chống phiên nông đã bị **áp quá tay**, trong khi §6 vốn **đã miễn trừ `rapid`**. Ba luật dưới đây sinh ra từ đó.

**① `rapid` là phương tiện chính, không phải phụ.** Phỏng vấn thật mở màn *"em có biết về X không"* rồi mới follow-up 1–2 tầng. `rapid` (12 câu/~15′) đúng là thứ luyện điều đó, và là cách duy nhất kéo phủ lên kịp.

**② TẠM DỪNG `by-level 🟠` và `deep-dive`** cho tới khi phủ ≥ 70%. Không phải vì chúng sai — vì **sai thời điểm**.

**③ Mỗi domain đi HAI LƯỢT — nhưng lượt 2 PHÂN LOẠI THEO ĐIỂM, không phải "đào sâu chỗ yếu".**

`rapid` quét rộng → rồi **một** `daily`, và `daily` đó **chỉ lấy câu đã đạt 3–4** để đào lên T2.

| Điểm ở `rapid` | Nghĩa | Đi đường nào |
|---|---|---|
| **0–1** | T1 trắng — chưa biết | **Đọc lại tài liệu** → hỏi lại ở **T1, góc khác**. ❌ **KHÔNG đào T2** |
| **2** | Biết nhưng lủng củng / thiếu nửa | Tách hai ca ([config §4](../mock-interview/config.md)): thiếu **kiến thức** → như trên · chỉ **diễn đạt chậm** → hỏi lại ở `rapid`, luyện nói gọn |
| **3–4** | ✅ T1 chắc | ⭐ **Đây mới là câu đưa vào `daily` để đào T2** |

> ⚠️ **Vì sao KHÔNG đào sâu câu điểm thấp** *(sửa 2026-08-17 — luật cũ ghi ngược, ứng viên phát hiện)*: hỏi T2 lên trên một nền T1 trống thì **không đo được gì và không dạy được gì** — người học chỉ ngồi im lần thứ hai. Đó đúng là căn bệnh của Giai đoạn 1: *"khái niệm chưa cứng đã phải trả lời câu chuyên sâu"*.
>
> Và nó bỏ phí đúng chỗ đáng đào nhất: **câu trả lời tốt mới là câu SẴN SÀNG cho T2**. Phỏng vấn thật cũng vận hành đúng vậy — trả lời khái niệm gọn ⇒ interviewer follow-up sâu hơn; ú ớ ở khái niệm ⇒ họ **chuyển câu khác**, không khoan tiếp.
>
> **Không câu nào bị bỏ rơi:** câu yếu → [weak-register](../mock-interview/weak-register.md), đọc lại rồi hỏi lại **T1 góc mới**. Câu mạnh → ngắn hạn lên T2 ở `daily`, dài hạn vào bảng 🔁 *Lịch kiểm tra lại* (retention, hạn = tuần gỡ + 2).

> **Đọc code / bài coding:** giữ **tối đa 1 snippet ngắn** mỗi phiên `comprehensive`, **không** ra bài code từ đầu ở phiên thường. Phần code nặng chuyển hẳn sang **[bộ lab 🧪 DBG](../mock-interview/bank/debugging.md)** — tự làm ngoài phiên, không tính điểm mock. Lý do: phỏng vấn embedded mid-level chủ yếu **giải thích miệng + whiteboard**, snippet ngắn có xuất hiện nhưng "ngồi code một class từ đầu" thì hiếm.

> **Phạm vi `embedded-fundamentals` (EMB) cho JD này:** chỉ ôn **A** (C/thanh ghi/`volatile`/bit-manip) + **C** (khái niệm ISR) như *đệm* — vì chúng chuyển thẳng sang MMIO driver + kernel IRQ. **Hoãn** B (startup/linker chi tiết), **D (RTOS)**, F/G/H (CRC/MISRA, low-power, JTAG/SWD): JD là Embedded **Linux** (có MMU, không RTOS/bare-metal firmware) nên các phần này là "nice-to-have" ghi điểm nếu bị hỏi lan, không phải câu loại.

---

## Behavioral riêng cho JD (JD nhấn "lead junior + code review")

Chuẩn bị STAR ([BEH](../mock-interview/bank/behavioral.md)) cho các câu **rất khả năng gặp** với JD này:
- **"Kể lần bạn mentor/hướng dẫn một junior"** → dùng chất liệu Samsung (hướng dẫn quy trình, AI skill, chip porting tool).
- **"Cách bạn review code / xử lý bất đồng kỹ thuật khi review"** → nêu tiêu chí review (đúng đắn, an toàn, đọc được), góp ý xây dựng, dựa dữ liệu.
- **"Thiết kế một subsystem phức tạp bạn từng làm"** → chọn case cross-layer (S-Box/brightness) hoặc multi-chipset HAL, trình bày theo khung [SD](../mock-interview/bank/system-design.md).
- **"Vì sao rời công ty cũ"** ([BEH-008](../mock-interview/bank/behavioral.md)) + **câu hỏi ngược** ([BEH-009](../mock-interview/bank/behavioral.md)) — hỏi về SoC/nền tảng, tỉ lệ kernel vs userspace, quy trình Yocto/CI, đội ngũ.
- Chuẩn bị nói **tiếng Anh** cho 2–3 câu ruột (JD yêu cầu English).

## Lịch nước rút

Không còn lịch cố định theo ngày — thứ tự việc nằm ở **▶️ LÀM TIẾP** phía trên, sắp lại sau mỗi phiên.

**Nguyên tắc khi phải cắt:** giữ mọi việc 🎯 (trụ JD) + **một** phiên `comprehensive` cuối. Bỏ trước: buổi đọc-thêm, domain 🟡, và lượt 2 của domain đã có bằng chứng vững.

## Vá điểm yếu đã biết

Nguồn duy nhất là [weak-register](../mock-interview/weak-register.md).

- **Sổ yếu** → chèn vào phiên `weak-review` (5′ khởi động).
- **Retention đến hạn** → bảng *🔁 Lịch kiểm tra lại*, rải vào slot khởi động.
- **Debug** (điểm yếu tự nhận, mà resume lại liệt kê là kỹ năng — xem [RES-008](../mock-interview/bank/resume.md)) → **12 bài [🧪 lab](../mock-interview/bank/debugging.md)**, ưu tiên làm sớm.
