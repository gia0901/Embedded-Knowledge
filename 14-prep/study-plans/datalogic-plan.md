# 🎯 Datalogic — Embedded Linux Engineer (C++17, Linux) · Plan 4 tuần

> Kế hoạch ôn **nhắm đúng JD** [Embedded Linux Engineer @ Datalogic Việt Nam](https://itviec.com/it-jobs/embedded-linux-engineer-c-linux-datalogic-viet-nam-2835). Dùng chung cả **4 tuần** (nền vững) lẫn **2 tuần** (nước rút — **gộp đôi khối lượng mỗi ngày**: làm 2 buổi/ngày thay vì 1).
> Không phải kiến thức mới — mọi mục **link tới tài liệu + bank câu hỏi** sẵn có. Ký hiệu: `- [ ]` chưa xong · `- [x]` xong · 🎯 xác suất gặp cao · 🎤 phiên mock · 🔁 ôn lặp lại.

---

## 📍 Tiến độ hiện tại — **RESUME Ở ĐÂY** (nguồn tracking DUY NHẤT, cập nhật mỗi buổi)

> **Conversation mới đọc block này TRƯỚC TIÊN** để biết đang ở đâu và làm gì tiếp. Đây là plan **đang chạy** và là **nguồn lịch duy nhất** (không còn daily-plan hay plan role nào khác).

- **Plan đang chạy:** 🎯 Datalogic (file này) · bắt đầu **2026-08-03** (T2). ⚠️ **Trễ 2 ngày**: B5 lẽ ra T6 07/08, thực tế chạy CN 09/08 → **CN và B5 dồn vào cùng ngày**. Tuần 2 nên bắt đầu **T2 10/08**; nếu không kịp buổi CN thì gộp nó vào khởi động buổi đầu Tuần 2, đừng đẩy cả plan lùi.
- **Vị trí:** ✅ **XONG Tuần 2 · Buổi 2** (10/10 câu). Tiếp theo là **Buổi 3** (debugging: gdb + tools + memory-bugs).
- **📊 CHẨN ĐOÁN QUAN TRỌNG NHẤT (từ 2 phiên liên tiếp 15–16/08):** `by-level` 🟠 (đo **T2** vận dụng) = **2.1** · `rapid` (đo **T1** biết & nói gọn) = **3.67**. ⇒ **Nền kiến thức CHẮC; chỗ hụt nằm ở tầng VẬN DỤNG, không phải tầng BIẾT.** Hệ quả cho cách ôn: **không cần đọc thêm tài liệu mới** — cần chuyển thứ đã biết thành *quyết định* + *dòng code sửa*. Ở phỏng vấn thật: vòng screen/khởi động sẽ tốt, áp lực dồn vào **phần giữa** khi interviewer hỏi *"giờ bạn làm gì"*.
- **Buổi gần nhất:** `rapid track debugging` (warm-up B3, trần T2 — thực tế chỉ T1) — **3.67/4**, 12 câu, **8/12 đạt 4** ([log](../mock-interview/sessions/2026-08-16--rapid--debugging.md)). **Cao nhất từ đầu plan**, ngang B5. Câu 3đ: DBG-005 (thiếu *phương pháp* dùng strace) · DBG-025 (thiếu nửa bộ công cụ C++) · DBG-027 (2/3 điều kiện phải probe mới ra) · **LNX-024 0→3** 🔼. Vá bank **DBG-027** (cụm *"gãy chuỗi"* nói lửng — ứng viên phát hiện). ⚠️ `rapid` ≠ bài test của B3 ⇒ **B3 chưa tick**.
- **Buổi trước đó:** `by-level 🟠 track linux-sysprog` (T2·B2, trần T2) — **2.1/4**, 10 câu ([log](../mock-interview/sessions/2026-08-15--by-level--linux-sysprog.md)). ⚠️ **Không so trực tiếp với 2.67 của B1** — B1 là `rapid` (thang chấm riêng, đo trôi chảy T1), B2 là 🟠 (tầng khó nhất còn tính điểm). Bỏ 2 câu repo không có tài liệu (Q8/Q10) thì **2.63**. **Phân bố:** LNX-023 **0→3** 🔼 · LNX-009/010/039/015 **3** · LNX-026/029/005 **2** · LNX-024 **0** · LNX-040 **0**. Sổ yếu **8 → 11**. Sinh 4 câu bank mới (**LNX-039** backpressure · **LNX-040** peer biến mất · **LNX-041** LT+EPOLLOUT busy-loop · **LNX-042** `PTHREAD_PROCESS_SHARED`), viết lại 2 đáp án (**LNX-024**, **LNX-026**). *(B1: 3.25 · B2: 3.3 · B3: 2.83 · B4: 3.17 · B5: 3.67 · CN-weak: 3.0 · CN-comp: 3.0 · T2B1: 2.67 · T2B2: 2.1)*
- **🔴 Phát hiện xuyên suốt phiên B2 — ưu tiên hơn cả điểm số:** lỗi lặp ở **4/10 câu** là **"chẩn đoán được, sửa không được"**. Đọc code tìm bug rất tốt (Q3a, Q6a chuẩn xác) rồi tắc ngay khi câu chuyển sang *"vậy bạn làm gì"*. Cách chữa đã chốt: **mỗi mục tài liệu đọc xong phải tự viết ra dòng code sửa**, không dừng ở mô tả.
- **🧪 TÀI SẢN MỚI (2026-08-17) — BỘ LAB ĐÃ ĐỦ, 7 bài:** bank có type thứ tư, **câu `lab` 🧪 "NGỒI MÁY LÀM"** (code có bug thật + nhiệm vụ + **output thật đã chạy** để đối chiếu; quy ước ở [bank/README.md](../mock-interview/bank/README.md) và [CLAUDE.md §6](../../CLAUDE.md)). Xếp theo **TRIỆU CHỨNG, không theo công cụ** — vì ngoài đời luôn bắt đầu từ triệu chứng, và đó đúng là lỗ hổng đã đo (*chẩn đoán được, chọn công cụ không được*):

    | Triệu chứng | Bài | Công cụ | Vá lỗ hổng |
    |---|---|---|---|
    | Chết ngay khi khởi động | [DBG-030](../mock-interview/bank/debugging.md) | `strace` | DBG-005 |
    | Rò tài nguyên (fd/RAM tăng dần) | [DBG-031](../mock-interview/bank/debugging.md) | `/proc/fd` | **LNX-023** (sổ yếu) |
    | Sai dữ liệu ngầm, không crash | [DBG-032](../mock-interview/bank/debugging.md) | ASan | DBG-024/025 |
    | Crash, mình không có mặt lúc đó | [DBG-033](../mock-interview/bank/debugging.md) | core dump + `gdb bt` | DBG-003 |
    | **Treo**, không crash không log | [DBG-034](../mock-interview/bank/debugging.md) | `/proc` `State`+`wchan` | LNX-023 |
    | Lúc được lúc không (đa luồng) | [DBG-035](../mock-interview/bank/debugging.md) | TSan | DBG-013 |
    | Đọc biến chưa khởi tạo | [DBG-036](../mock-interview/bank/debugging.md) | **valgrind** *(ASan mù)* | DBG-011 |

    **Bộ công cụ lõi đã chốt:** `gdb` · `strace` · ASan · `/proc`+`/sys` · valgrind (đúng một việc: biến chưa khởi tạo) · TSan (đa luồng). **Cắt hẳn:** `perf`, `ltrace`, `gdbserver`/`addr2line` bài riêng — giữ ở mức câu `concept` là đủ. Lý do cắt: phỏng vấn embedded hỏi *"nó crash/treo"* nhiều hơn hẳn *"nó chậm"*.
    **⚠️ Ba bẫy môi trường đã đo thật và viết thẳng vào bài** (đây là thứ làm người ta bỏ cuộc, không phải kiến thức debug): `core_pattern` pipe vào apport ⇒ `ulimit -c` một mình **không đủ**, apport còn **vứt luôn** core của binary tự build · `ptrace_scope=1` chặn `gdb -p` · **TSan chết ngay khi khởi động trên kernel 6.x** vì xung đột ASLR, phải `setarch -R`.
    **📌 Phát hiện đáng nhớ:** cùng một data race, build `-O2` thì **5/5 lần chạy đều ra kết quả ĐÚNG** (compiler gộp vòng lặp thành một phép cộng), `-O0` thì sai 4/5. Và ASan **làm lộ** giá trị rác (`0xbebebebe`) nhưng **không báo lỗi**, thoát mã 0.
- **▶️ LÀM TIẾP:** **Tuần 2 Buổi 3 — phần test chính vẫn còn nợ.** 🧪 **Làm 3 bài lab TRƯỚC khi vào mock** — chúng đúng là dạng luyện tầng vận dụng mà chẩn đoán chỉ ra. Warm-up `rapid` đã chạy 16/08 và cho thấy **T1 debugging đã vững** ⇒ **bỏ qua phần đọc lại khái niệm**, vào thẳng tầng vận dụng: đọc [gdb](../../09-debugging/gdb.md) + [tools](../../09-debugging/tools.md) + [memory-bugs](../../09-debugging/memory-bugs.md) **theo lối "mỗi mục đọc xong tự viết ra dòng lệnh/dòng code sẽ gõ thật"** (chữa lỗ hổng ① — chẩn đoán được, sửa không được) → `/mock daily track debugging`.
- **Nợ lại:**
  - ① ⬜ **Bài coding** `SocketHandle` (CPP-054) + `ring_buffer_v2` bản mutex (COD-006) — chạy `/mock coding track cpp-system`, làm từ file trống. *(Cần một phiên mock, không tự làm hộ được.)*
  - ② ✅ **Xong 2026-08-13** — viết lại bank **DP-011** (DIP/HAL): 465 → 3.915 ký tự, thêm sơ đồ đảo chiều phụ thuộc, 3 kiểu seam (virtual · template/CRTP · link-time) kèm đánh đổi, 4 bẫy.
  - ③ ✅ **Xong 2026-08-13** — vá cả 3 lỗ hổng tài liệu: **(i)** thêm hẳn mục mới [`tcp-ip.md §6` — *TCP là luồng byte, không có ranh giới message*](../../13-networking/tcp-ip.md) (bảng hứa/không hứa · short read vs dính gói · **vì sao lab không lộ** · 3 cách framing + đánh đổi · `read_full()`); **(ii)** sửa link **LNX-023** → `09-debugging/tools.md` + `kernel-userspace.md`; **(iii)** sửa link **LNX-029** → `TLPI cụm 03 §5`.
  - ④ 🟡 **Nợ chất lượng bank — đang co dần:** toàn bank 303 câu, **72 → 68 câu** 🟠🔴 còn đáp án < 700 ký tự (chưa đạt khung 5 phần của [bank/README.md](../mock-interview/bank/README.md)). *(35 câu 🟢 ngắn là **đúng chuẩn** — trần cứng có chủ đích, **đừng giãn**.)*
    - ✅ Đã xử lý 2026-08-13: `DBG-008` (quy trình debug — nay hỏi qua ticket thật, giải thích **vì sao thứ tự đó**), `DBG-012` (Heisenbug — thêm cơ chế *vì sao `printf` che bug* + **giá phải trả của từng sanitizer**), `DBG-014` (crash ngoài field — thêm **lưu symbol theo version**, đánh đổi khi log, phân biệt 4 kiểu chết, *đo thứ tăng dần trước khi crash*). Cả ba đều là mảng **debug — điểm yếu tự nêu ở [CLAUDE.md §2](../../CLAUDE.md)**.
    - **Không viết lại hàng loạt.** [config §1 Bước 4](../mock-interview/config.md) nay **bắt nâng cấp đáp án của câu ≤ 2 điểm ngay sau mỗi phiên**, nên nợ tự co lại đúng chỗ đang ôn.
    - Thứ tự làm tay nếu có thời gian, bám lịch plan: **Tuần 3** `DRV` (11 câu) + `EMB` (9) → **Tuần 4** `SD` (6) + `BLD` (2) → nền `CPP` (11), `OS` (5), `LNX` (5), `DP` (6), `NET` (6), `DSA` (5).
  - ⑥ ✅ **DI TRÚ "một đáp án, một chỗ" — XONG 262/262 (2026-08-13).** Rà soát phát hiện **262 câu hỏi có đáp án chép thẳng vào tài liệu topic 01–13**, song song với bank — tức 262 bản sao **không cơ chế đồng bộ**, và đã **trôi lệch thật** (bank `LNX-005` nâng lên 2.736 ký tự nhưng bản sao trong `file-io.md` vẫn 450). Quy ước đã chốt ở [CLAUDE.md §4.7](../../CLAUDE.md): **đáp án chỉ sống trong bank**, tài liệu chỉ có bảng link.
    - ✅ **Topic 03 xong** (30 câu): 7 câu mới (`OS-022`…`OS-028`), 6 câu bank làm dày (`OS-008/009/010/014/016`, `LNX-008`). *(`OS-002`/`OS-004` là 🟢 nên **giữ trần cứng 30–60 từ** — đúng chuẩn, không giãn.)*
    - ✅ **Topic 04 xong** (25 câu): 7 câu mới (`LNX-032`…`LNX-038` — read vs fread · vấn đề multiplexing · thiết kế IPC 60 MB/s · Unix socket · POSIX vs SysV · signal không bắt được · `volatile sig_atomic_t`), 6 câu làm dày (`LNX-004/006/007/015/017/020`).
    - ✅ **Topic 09 xong** (26 câu): 10 câu mới (`DBG-020`…`DBG-029` — debug kernel vs userspace · printk · ftrace · KASAN/LOCKDEP · các loại lỗi bộ nhớ · phòng ngừa bằng C++ hiện đại · sửa triệu chứng vs nguyên nhân · git bisect · strace vs ltrace · thiết kế logging), 2 câu làm dày (`DBG-004/006`).
    - ✅ **Topic 05 xong** (24 câu): 11 câu mới (`BUS-005`…`007` — pull-up I2C · clock stretching · debug I2C không phản hồi; `DRV-028`…`035` — DTS/DTB/dtc · `compatible` · driver đọc DT · DT vs driver · DT vs ACPI · tách kernel/user · `mmap` driver · đặt chức năng ở kernel hay user), 6 câu làm dày (`BUS-001/004`, `DRV-005/007/009/014`).
    - ✅ **Topic 02 xong** (24 câu): 4 câu mới (`CPP-056`…`059` — data race là UB · `~thread()` gọi `terminate()` & `jthread` · lambda → closure type · `std::function` type erasure), 1 câu làm dày (`CPP-020` Rule of 0/3/5 — thêm bẫy *destructor rỗng chặn move tự sinh*). Bank `CPP` vốn đã dày nên phần lớn chỉ cần trỏ link.
    - ✅ **Topic 07 xong** (23 câu): **15 câu mới** (`SD-017`…`SD-031`) — đây là mảng **ABI/linking mà bank gần như chưa phủ**, đúng trụ *"C++ shared library"* của JD: API vs ABI · thay đổi phá ABI · soname & symbol versioning · thiết kế giữ ABI · Pimpl · biên giới C API · sở hữu qua kiểu · exception vs mã lỗi · linking vs loading · symbol & undefined/multiple · loader + PLT/GOT + lazy binding · name mangling & `extern "C"` · `dlopen`/`dlsym` · `-fPIC` & chia sẻ khung trang · chọn static hay shared.
    - ✅ **Topic 08 xong** (22 câu): 7 câu mới (`EMB-033`…`036` — SoC vs PC · hard/soft realtime · realtime ở mức lập trình · giảm flash/RAM; `BSP-028`…`030` — RTOS+Linux kết hợp · initramfs · init PID 1), **7 câu làm dày** (`DRV-008/012/013/015/016/017/018`) — **5 trong số đó nằm luôn trong nợ ④**.
    - ✅ **Topic 01 xong** (19 câu): 5 câu mới (`CPP-060`…`064` — trả địa chỉ biến cục bộ · pure virtual/abstract · diamond problem · full vs partial specialization · non-type template parameter), 1 câu làm dày (`CPP-009`).
    - ✅ **Topic 06 xong** (17 câu): 9 câu mới (`BLD-011`…`019` — configure/build & out-of-source · `find_package` · cross-compile & build/host/target · **sysroot** · khó khăn cross-compile · các bước `.cpp`→exe · include guard · make incremental sai ở đâu · Makefile hay CMake). *(`undefined reference` và `Yocto vs Buildroot` đã có sẵn ở `SD-026`/`BSP-017`.)*
    - ✅ **Bốn topic cuối xong** (52 câu): `11` (16) · `12` (15) · `13` (11) · `10` (10). 7 câu mới (`DP-016`…`018` Adapter/Facade/Proxy · `DSA-013` phân biệt đầy–rỗng · `DSA-014` SPSC lock-free 🔺T3 · `SD-032` khi bế tắc · `SD-033` think aloud). `13-networking` **không cần câu mới nào** — bank đã phủ đủ.
    - 🎉 **HOÀN TẤT: 262/262 câu · 47 file / 13 topic · bank 303 → 386 · 0 câu inline còn lại · 0 link hỏng.**
    - **Từ nay:** thấy `<details>` chứa đáp án trong tài liệu topic = **nợ kỹ thuật mới phát sinh**, phải chuyển về bank ngay ([CLAUDE.md §4.7](../../CLAUDE.md)). ⚠️ **KHÔNG tự động hoá được**: matcher tự động ánh xạ nhầm *"CPU-bound vs I/O-bound"* → `CPP-028` (câu C++) và bỏ sót `OS-015` đang có sẵn — với thao tác **xoá nội dung** thì tỉ lệ sai đó không chấp nhận được. Phải ánh xạ tay từng topic, theo 3 tầng: **A** bank đã đủ → thay link · **B** tài liệu đầy hơn bank → gộp vào bank rồi link · **C** chưa có trong bank → thêm câu mới rồi link.
    - Thứ tự đề xuất theo lịch ôn: `04` (25) → `09` (26) → `05` (24) → `08` (22) → `02` (24) → `07` (23) → `01` (19) → `06` (17) → `11` (16) → `12` (15) → `13` (11) → `10` (10).
  - ⑤ 🟡 **Chất lượng tài liệu topic 01–13 — rà soát topic 04 ngày 2026-08-13** (ứng viên phản hồi *"ngắn gọn, chung chung, không giải thích tại sao / bản chất / khi nào dùng"*). Kết quả đo, **không đều giữa các file**:
    - ✅ `processes-signals.md` (2.069 từ, 6× "vì sao") và `file-io.md` (2.725 từ, 10 code block, 5 bẫy) — **đạt**; vấn đề của `file-io.md` là **thiếu mục** (`/proc`, clock), đã xử lý bằng cách trỏ lại link bank ở ③.
    - ✅ **`ipc-linux.md` — ĐÃ VIẾT LẠI 2026-08-13:** 9.229 → **27.709 ký tự (3×)**, 1.388 → 4.271 từ, 0 → **14 bẫy**, 0 → **6× "vì sao"**, 1 → **7 bảng**. Thêm: bản chất *2 cách chuyển byte* (kernel copy vs map chung), **4 trục quyết định** + sơ đồ chọn, **ca một bên chết** (stale lock, robust mutex — chạy thật ra `EOWNERDEAD`), `ftruncate`/`SIGBUS` (chạy thật ra signal 7), EOF của pipe, `SIGPIPE` giết process, `SOCK_SEQPACKET`, bảng **chi phí thật** (copy/syscall/ranh giới/chết), mục *"khi nào KHÔNG dùng"* cho từng cơ chế. Lý do ưu tiên file này: nó là bài đọc **Buổi 2**, và hôm nay **2 câu rơi trúng nó mà nó không đỡ được câu nào** (Q8 shm stale lock, Q10 pipe EOF).
    - ✅ **`io-multiplexing.md` — ĐÃ BỔ SUNG 2026-08-13:** 1.777 → **2.433 từ**. Thêm §7 *"Khi nào KHÔNG dùng epoll"* (kèm bẫy **file thường luôn báo sẵn sàng ⇒ epoll vô dụng với file**) và §8 với 7 bẫy: `close()` không gỡ fd khỏi epoll khi còn `dup` · thundering herd + `EPOLLEXCLUSIVE`/`SO_REUSEPORT` · `EPOLLONESHOT` cho đa luồng · quên vét cạn `EPOLLOUT` · **backpressure** (bug OOM ở nhà khách) · một handler chậm giết toàn bộ kết nối · `EINTR`.
    - ✅ **Rà soát `03` và `08` (2026-08-13).** `03`: **cả 5 file đều 0 bẫy, 3/5 không có con số nào** — yếu **đều**, là một khuôn viết (mỗi file ~1.500 từ, 8 mục, 6 câu). Đã thêm mục *"Chi phí thật & bẫy"* cho cả 5, mỗi mục 5–7 bẫy đánh số + bảng chi phí (`memory-management` từ 1 → **16** dẫn chứng định lượng; `sync-primitives` → 10). `08`: **phân cực** — `interrupts-bare-metal`/`architecture`/`rtos-*` đã tốt; đã bổ sung `boot-process` (thêm bảng **chẩn đoán theo chỗ chết** + 7 bẫy bring-up) và `constraints` (thêm bảng RAM/dòng điện/chu kỳ ghi flash, 23 dẫn chứng định lượng, 7 bẫy).
    - ⚠️ **`hardware-debug.md` — tôi chấm oan lúc đầu**: 886 từ nhưng **đặc** (sơ đồ scope đo latency vs thời gian ISR, lượng hoá 4 ns vs 700 µs, crash log `.noinit`). Đếm từ khoá không thay được việc đọc.
  - ⑦ 🔴 **NỢ TÀI LIỆU phát sinh từ phiên 2026-08-15 — 4 lỗ hổng, 2 cái nặng.** Kiểm bằng `grep`, không phải cảm tính. Đây là **cùng một mẫu** với LNX-023/LNX-029 hôm 13/08: *câu bank 🟠 trỏ tới file topic không chứa nội dung đó* — nay đếm được **lần thứ 3 và 4**, nên phải coi là **lỗi hệ thống của bank, không phải sự cố lẻ**.
    | # | Lỗ hổng | Bằng chứng | Việc phải làm |
    |---|---|---|---|
    | 1 | 🔴 **Không có tài liệu nào về namespaces/cgroups** trong toàn repo | `LNX-024` (🟠, nằm trong pool ôn Buổi 2) trỏ `→ ipc-linux.md`; file đó chỉ có 1 chữ "namespace" và là *abstract namespace của Unix socket* | Viết mục mới — đề xuất đặt ở [ipc-linux.md](../../04-linux-system-programming/ipc-linux.md) hoặc [03-operating-system/](../../03-operating-system/); nhấn **góc embedded: systemd unit (`MemoryMax=`, `PrivateTmp=`, `TasksMax=`)**, không phải Docker |
    | 2 | 🔴 **Không có tài liệu nào về phát hiện peer chết** | `grep -rn "keepalive\|ETIMEDOUT"` toàn repo → 0 kết quả liên quan | Thêm mục vào [tcp-ip.md](../../13-networking/tcp-ip.md) *(cạnh §6 luồng byte đã viết 13/08)*: `write()` thành công ≠ đã nhận · retransmit → `ETIMEDOUT` ~15–20′ · `SO_KEEPALIVE` vs **heartbeat** + đánh đổi |
    | 3 | 🟡 `io-multiplexing.md` thiếu **LT + `EPOLLOUT` thường trực = busy-loop 100% CPU** | §8④ chỉ nói ET phía ghi; bug phổ biến nhất của người mới viết event loop lại không có | Thêm vào §8, kèm số đo đã chạy thật: LT **5/5** vòng báo sẵn sàng · ET **1/5** |
    | 4 | 🟡 `file-io.md` nói về `mmap` đúng **1 dòng** (dòng 227) nhưng `LNX-026` trỏ vào đó | Thiếu hết: page fault, readahead, `SIGBUS`, `msync` | Bank `LNX-026` **đã viết lại đầy đủ 15/08** → hoặc viết mục cho `file-io.md`, hoặc **sửa link** sang chỗ đúng |
    | 5 | 🟡 **RSS bị nhắc lửng trong `09-debugging/`** *(phát hiện 16/08 — **ứng viên tự nêu**)* | `memory-bugs.md:85` chỉ có một dòng bảng *"theo dõi RSS qua thời gian"*; phần **vì sao** nằm ở topic khác và **không có link nối**: `free()` không trả RAM về OS, glibc giữ arena ⇒ **RSS không giảm là BÌNH THƯỜNG**, phải xem *xu hướng* chứ không xem một thời điểm ([03/memory-management.md](../../03-operating-system/memory-management.md)) | Thêm link chéo từ `memory-bugs.md` sang mục đó — đây là **bẫy chẩn đoán nhầm leak** rất hay gặp |
    > **Luật rút ra (áp từ nay):** trước khi đưa một câu 🟠🔴 vào pool ôn của một buổi, **kiểm link nguồn có thật sự chứa nội dung không**. Câu không có tài liệu nền ⇒ hỏi ra chỉ đo được *"đã từng gặp ngoài đời chưa"*, không đo được kết quả ôn tập.
- **Sổ yếu (11 câu):** **LNX-040 (0đ)** *(⚠️ repo chưa có tài liệu — viết doc trước khi hỏi lại)* · **LNX-026 (2đ)** · LNX-005 (2đ, **đứng yên 2 phiên** — lần sau **bắt viết code**) · LNX-029 (1→2đ) · CPP-016 · SD-016 · CPP-054 (chưa hỏi lại) · **LNX-024 (0→3** 🔼 1/2 lần ≥3 — nhưng mới đạt ở **T1**, lần sau phải hỏi T2**)** · LNX-023 (0→3, 1/2 lần ≥3) · OS-020 (1/2 lần ≥3) · CPP-032 (1/2 lần ≥3). Xem [weak-register](../mock-interview/weak-register.md).

**Checklist buổi (tick `[x]` khi xong — đây là "đang dang dở đến đâu"):**
- **Tuần 1** (C++17): `[x]` B1 · `[x]` B2 · `[x]` B3 · `[x]` B4 · `[x]` B5 · `[x]` CN *(weak-review 3.0 + comprehensive 3.0; nợ 2 bài coding)*
- **Tuần 2** (Linux sysprog+debug): `[x]` B1 *(2.67 — 12 câu, 2 lượt)* · `[x]` B2 *(2.1 — 10 câu 🟠)* · `[ ]` B3 ⬅️ **ĐANG Ở ĐÂY** · `[ ]` B4 · `[ ]` B5 · `[ ]` CN
- **Tuần 3** (kernel/drivers): `[ ]` toàn bộ
- **Tuần 4** (Yocto/design/behavioral): `[ ]` toàn bộ

> **Quy tắc cập nhật (cho Claude):** sau mỗi phiên mock thuộc plan này → (1) tick buổi tương ứng ở trên, (2) sửa 4 dòng "Vị trí / Buổi gần nhất / LÀM TIẾP / Sổ yếu", (3) ghi log `sessions/` như thường. Không cần sửa file nào khác — mọi nơi khác chỉ trỏ về block này.

---

## Hai nguyên tắc xuyên suốt (đọc trước)

**① Ưu tiên TƯ DUY, không phải cú pháp.** Đặc biệt với **build system (Yocto/CMake/CI)**: phỏng vấn hỏi *"layer/bbappend để làm gì", "sstate giải quyết gì", "DEPENDS vs RDEPENDS", "vì sao CMake target-based"* — tức **mental model** (vấn đề → kiến trúc → đánh đổi), **không** bắt viết recipe/CMakeLists chính xác. Đọc để *hiểu bức tranh*, cú pháp chỉ tra khi ngồi máy. Nguyên tắc này áp cho mọi mảng: nói được *bản chất & khi nào dùng* quan trọng hơn nhớ flag.

**② Xen kẽ + lặp lại ngắt quãng (chống quên tuần cũ).** Theme tuần chỉ định *trọng tâm học MỚI* — không có nghĩa bỏ tuần trước. Hai cơ chế kéo kiến thức cũ quay lại:
- **🔁 Khởi động trộn (~5–10') mỗi buổi:** trước khi vào chủ đề mới, chạy nhanh `/mock weak-review` (câu yếu) hoặc `/mock retention track datalogic` (câu tuần trước, đã trả lời tốt — spaced review). Trộn domain là *cố ý*: phỏng vấn thật cũng hỏi ngẫu nhiên.
  - **Câu retention lấy ở đâu:** bảng **[🔁 Lịch kiểm tra lại](../mock-interview/weak-register.md)** — mỗi câu đã gỡ khỏi sổ yếu đều có **hạn kiểm tra = tuần gỡ + 2** và **góc mới đề xuất** (cấm lặp góc cũ). Mở bảng, lấy câu đến hạn của tuần hiện tại. Tụt < 3 → kéo về sổ yếu.
  - Lịch hiện tại: ~~Tuần 2 → CPP-020, CPP-029~~ ✅ **đã kiểm tra 13/08 (4 và 3), dời sang Tuần 4** · **Tuần 3** → CPP-009, DP-002, OS-003, OS-007 · **Tuần 4** → CPP-019, CPP-024, CPP-045, CPP-020, CPP-029.
- **🔁 Cuối tuần ôn DỒN TÍCH (tăng dần):** CN mỗi tuần quét *tất cả* các tuần đã học, không chỉ tuần vừa rồi — xem cột "CN" trong từng bảng.

**③ Đọc/Hiểu liệt kê ĐẦY ĐỦ để ôn TRƯỚC; mock cứ để NGẪU NHIÊN.** Cột "Đọc/Hiểu" mỗi buổi nêu **cụ thể** doc (mục) + **pool câu hỏi bank** liên quan — đây là phần *chuẩn bị*, đọc kỹ trước khi vào mock. Mock thì **được phép hỏi ngẫu nhiên/lệch chủ đề trong ngày** (rút từ track rộng + weak-register + retention) — đó là **cố ý**, để luyện *phản ứng* như phỏng vấn thật (câu đến bất ngờ, trộn mảng). Không ép test chỉ trong phạm vi vừa đọc. Việc bị hỏi trúng chỗ chưa ôn là *tín hiệu* để ghi vào weak-register và ôn bù, không phải lỗi của plan.

> Nhờ đó cuối 4 tuần, kiến thức Tuần 1 đã được gặp lại ≥3 lần (khởi động + các buổi CN + tổng duyệt) thay vì học một lần rồi quên.

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

**Track mock riêng cho JD:** `/mock ... track datalogic` (trộn đúng các domain trên).

---

## Tuần 1 — C++17 & Modern C++ (+ CMake)

*Trọng tâm JD "C++17". Vá nền ngôn ngữ trước.* 🔁 Tuần đầu chưa có gì để trộn — cứ học sâu.

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [x] [raii-smart-pointers](../../02-modern-cpp/raii-smart-pointers.md) §6 (Rule 0/3/5) + smart pointer (unique/shared/weak, cyclic ref); [move-semantics](../../02-modern-cpp/move-semantics.md) (rvalue ref, `std::move`, perfect forwarding, emplace vs push_back). **Chạm nhanh:** [oop](../../01-cpp-fundamentals/oop.md) (struct/class, virtual/vtable), [memory-model](../../01-cpp-fundamentals/memory-model.md) (stack/heap). **Pool bank ôn trước:** CPP-001…008, 020, 024…029 | ✅ `rapid cpp-system` 3.25/4 (2026-08-03) |
| 2 | - [x] 🎯 EMC items nền: [effective-modern-cpp](../../15-book-summaries/effective-modern-cpp.md) — auto (Item 1–2), braced-init (7), nullptr (8), using vs typedef (9), `=delete` (11), override/final (12), make_unique (21), special-member generation (17). **Pool bank:** CPP-040…051, CPP-046 | ✅ `by-level 🟡 modern-cpp` 3.3/4 (2026-08-03) |
| 3 | - [x] [concurrency](../../02-modern-cpp/concurrency.md) (thread, atomic, memory_order, mutex, lock_guard/unique_lock) + [OS sync-primitives](../../03-operating-system/sync-primitives.md) (mutex vs semaphore + **ownership/priority inheritance**, spinlock, deadlock 4 Coffman). **Pool bank:** CPP-018/019/022/024; OS-003/004/006/007/012 | ✅ `daily cpp-system` 2.83/4 (2026-08-05) — ⚠️ nợ **memory order** (CPP-019) |
| 4 | - [x] [cmake](../../06-build-systems/cmake.md) — **tư duy target-based** (PUBLIC/PRIVATE/INTERFACE *nghĩa là gì*, toolchain file *giải quyết gì*), cú pháp tra sau | ✅ `daily build-systems` 3.17/4 (2026-08-06) — CMake đạt; nợ **lệnh Yocto** (cleansstate/devtool) |
| 5 | - [x] Coding: viết vào [coding-arena/](../mock-interview/coding-arena/); nền: [12-dsa/ring-buffer.md](../../12-dsa/ring-buffer.md) | ✅ `coding dsa` 3.67/4 (2026-08-09) — COD-001 **4** · COD-006 **3** · COD-003 **4**; nợ **hỏi spec đa luồng** + **cấp phát trong hot path**. [Bản đã review](../mock-interview/coding-arena/reviewed/) |
| CN | 🔁 Ôn dồn tích **T1** | ✅ `weak-review cpp-system` 3.0/4 (sổ yếu 4→2) + ✅ `comprehensive cpp-system` 3.0/4, 15/16 câu, trần T2 ([log](../mock-interview/sessions/2026-08-13--comprehensive--cpp-system.md)) — **nợ 2 bài coding** |

## Tuần 2 — Linux system programming (userspace + debug) 🎯

*Đúng chỗ bạn muốn tập trung ("Familiar with Linux userspace + debugging").* 🔁 **Khởi động mỗi buổi:** `/mock retention track cpp-system` (kéo C++ Tuần 1 quay lại).

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [x] [file-io](../../04-linux-system-programming/file-io.md) (fd, syscall, blocking/non-blocking, `O_NONBLOCK`), [processes-signals](../../04-linux-system-programming/processes-signals.md) (fork/exec/wait, zombie, signal handler async-safe). **Pool bank:** LNX-001…012, **LNX-030** | ✅ `rapid linux-sysprog` **2.67/4**, 12 câu (2026-08-13 — [phần 1](../mock-interview/sessions/2026-08-13--rapid--linux-sysprog.md) · [phần 2](../mock-interview/sessions/2026-08-13--rapid--linux-sysprog-2.md)). Đỉnh: LNX-012 **4**, LNX-017 **4**. Nợ: **LNX-023 (0)**, **LNX-029 (1)** |
| 2 | - [x] 🎯 [io-multiplexing](../../04-linux-system-programming/io-multiplexing.md) (select/poll/epoll, **LT vs ET**, event loop) + [ipc-linux](../../04-linux-system-programming/ipc-linux.md) (pipe, shm, mq, socket, so sánh). **Pool bank:** LNX-013…026 | ✅ `by-level 🟠 linux-sysprog` **2.1/4**, 10 câu (2026-08-15 — [log](../mock-interview/sessions/2026-08-15--by-level--linux-sysprog.md)). Đỉnh: LNX-023 **0→3** 🔼. Nợ: **LNX-024 (0)**, **LNX-040 (0)** — cả hai repo chưa có tài liệu |
| 3 | - [ ] 🎯 [09-debugging](../../09-debugging/): [gdb](../../09-debugging/gdb.md), [tools](../../09-debugging/tools.md) (strace/ltrace/perf), [memory-bugs](../../09-debugging/memory-bugs.md) (ASan/TSan/Valgrind), core dump. **Pool bank:** DBG-001…011, 017/018 | `/mock daily track debugging` |
| 4 | - [ ] [gdb](../../09-debugging/gdb.md) sâu (breakpoint/watchpoint/`bt`/frame, remote **gdbserver**, core dump post-mortem) — điểm yếu cần vá. **Pool bank:** DBG-009/010, 012…016 | `/mock deep-dive track debugging` |
| 5 | Coding: bài low-level C | `/mock coding track cpp-system` (memcpy/strlen, endianness) |
| CN | 🔁 Ôn dồn tích **T1+T2** | `/mock weak-review` + `/mock comprehensive track datalogic` (trộn C++ + Linux + debug) |

## Tuần 3 — Kernel & drivers (I2C/SPI + PCI/USB) 🎯

*Trụ kernel + drivers của JD. Vùng bạn mạnh (kernel/DT) + lấp PCI/USB.* 🔁 **Khởi động mỗi buổi:** `/mock retention track linux-sysprog` + `weak-review` (kéo Linux/debug Tuần 2 quay lại).

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] [driver-basics](../../05-drivers-device-tree/driver-basics.md), [kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md) · *bổ đệm* [bare-metal-c](../../08-embedded-systems/bare-metal-c.md) phần `volatile`/register/bit-manip ([EMB-A](../mock-interview/bank/embedded-fundamentals.md)) — nền cho MMIO `readl/writel` | `/mock daily track drivers-dt` |
| 2 | - [ ] 🎯 [device-tree](../../05-drivers-device-tree/device-tree.md) (cú pháp DTS, node/property, compatible→match, binding) + [kernel-userspace](../../05-drivers-device-tree/kernel-userspace.md) ôn lại. **Pool bank:** BSP-006…009 (probe, EPROBE_DEFER, pinctrl), DRV-001…010 | `/mock by-level 🟠 track bsp` |
| 3 | - [ ] 🎯 **[pci-usb-drivers](../../05-drivers-device-tree/pci-usb-drivers.md)** (PCI config/BAR/MSI, USB descriptor/URB/gadget) | `/mock ... track drivers-dt` (DRV-019…027) |
| 4 | - [ ] [interrupt/DMA](../mock-interview/bank/bsp.md) (BSP-010…013: threaded IRQ, coherent vs streaming DMA) · *bổ đệm* [interrupts-bare-metal](../../08-embedded-systems/interrupts-bare-metal.md) khái niệm ISR ([EMB-C](../mock-interview/bank/embedded-fundamentals.md)): không sleep trong ISR, critical section, reentrancy — chuyển thẳng sang top-half/bottom-half kernel. **Pool bank:** BSP-010…013, DRV-011…018 | `/mock deep-dive track bsp` |
| 5 | - [ ] [lkd](../../15-book-summaries/lkd/) cụm driver/interrupt (đọc thêm) | `/mock comprehensive track drivers-dt` |
| CN | 🔁 Ôn dồn tích **T1+T2+T3** | `/mock weak-review` + `/mock comprehensive track datalogic` (giờ trộn cả kernel/driver) |

> **Phạm vi `embedded-fundamentals` (EMB) cho JD này:** chỉ ôn **A** (C/thanh ghi/`volatile`/bit-manip) + **C** (khái niệm ISR) như *đệm* cho buổi 1 & 4 ở trên — vì chúng chuyển thẳng sang MMIO driver + kernel IRQ. **Hoãn** B (startup/linker chi tiết), **D (RTOS)**, F/G/H (CRC/MISRA, low-power, JTAG/SWD): JD là Embedded **Linux** (có MMU, không RTOS/bare-metal firmware) nên các phần này là "nice-to-have" ghi điểm nếu bị hỏi lan, không phải câu loại. Full EMB (domain `EMB` trong bank) để dành cho hồ sơ BSP tổng quát nếu sau này nhắm JD khác.

## Tuần 4 — Yocto/CI + Design + Behavioral + tổng duyệt

*Yocto (JD) + kỹ năng "lead/design/review" + dress rehearsal.* 🔁 **Khởi động mỗi buổi:** `/mock retention track datalogic` (trộn cả 3 tuần trước) — tuần này nghiêng hẳn về **ôn lại + tổng duyệt**.

| Buổi | Đọc/Hiểu | Test 🎤 |
|---|---|---|
| 1 | - [ ] 🎯 **[yocto](../../06-build-systems/yocto.md)** — **tư duy** (layer/bbappend *để làm gì*, sstate *giải quyết gì*, DEPENDS vs RDEPENDS, devtool/SDK), **không** học thuộc cú pháp recipe + [BSP-017…019](../mock-interview/bank/bsp.md) | `/mock ... track build-systems` (BLD-004…010) |
| 2 | - [ ] 🎯 [system-design](../../10-thinking/system-design.md) §5 hộp đồ nghề + §6 ví dụ giải mẫu (**OTA**, **máy quét mã vạch** — đúng chất Datalogic) + [07-shared-libraries](../../07-shared-libraries/) (ABI, API design) | `/mock deep-dive track system-design` (SD-012…016) |
| 3 | - [ ] 🎯 **Behavioral cho JD**: [BEH](../mock-interview/bank/behavioral.md) + phần "lead junior / code review" (§ dưới) | `/mock ... track behavioral` |
| 4 | Tổng duyệt vòng thật | `/mock comprehensive track datalogic` (16 câu) |
| 5 | Kiểm tra toàn diện | `/mock full-review` (quét mọi câu đã hỏi) |
| CN | Nghỉ + đọc lại [sessions/](../mock-interview/sessions/) | — |

---

## Behavioral riêng cho JD (JD nhấn "lead junior + code review")

Chuẩn bị STAR ([BEH](../mock-interview/bank/behavioral.md)) cho các câu **rất khả năng gặp** với JD này:
- **"Kể lần bạn mentor/hướng dẫn một junior"** → dùng chất liệu Samsung (hướng dẫn quy trình, AI skill, chip porting tool).
- **"Cách bạn review code / xử lý bất đồng kỹ thuật khi review"** → nêu tiêu chí review (đúng đắn, an toàn, đọc được), góp ý xây dựng, dựa dữ liệu.
- **"Thiết kế một subsystem phức tạp bạn từng làm"** → chọn case cross-layer (S-Box/brightness) hoặc multi-chipset HAL, trình bày theo khung [SD](../mock-interview/bank/system-design.md).
- **"Vì sao rời công ty cũ"** ([BEH-008](../mock-interview/bank/behavioral.md)) + **câu hỏi ngược** ([BEH-009](../mock-interview/bank/behavioral.md)) — hỏi về SoC/nền tảng, tỉ lệ kernel vs userspace, quy trình Yocto/CI, đội ngũ.
- Chuẩn bị nói **tiếng Anh** cho 2–3 câu ruột (JD yêu cầu English).

## Lịch nước rút 2 tuần

Gộp đôi mỗi ngày theo thứ tự ưu tiên: **Tuần A** = (Tuần 1 buổi 1–2–4) + (Tuần 2 buổi 2–3–4); **Tuần B** = (Tuần 3 buổi 2–3–4) + (Tuần 4 buổi 1–3–4). Bỏ các buổi coding/đọc-thêm nếu thiếu giờ, giữ **mọi buổi 🎯 + `comprehensive` + `full-review`**.

## Vá điểm yếu đã biết
Từ [weak-register](../mock-interview/weak-register.md): OS-003 (deadlock), CPP-029 (emplace), DP-002 (Singleton) — chèn vào các buổi `weak-review`. GDB (bạn tự nhận yếu) → ưu tiên Tuần 2 buổi 4.
