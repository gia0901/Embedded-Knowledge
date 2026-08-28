# 🕳️ Gap Register — Lỗ hổng tài liệu & Backlog

> Danh sách **"chủ đề đáng có mà repo chưa có (hoặc còn mỏng)"**, xếp theo ưu tiên. Mục đích: để repo lớn lên **có chủ đích**, không ngẫu hứng. Cập nhật mỗi khi phát hiện lỗ hổng lúc ôn (theo [quy trình](README.md): ghi vào đây, không dừng mạch ôn để lấp ngay).
>
> **Cột trạng thái:** ⬜ chưa có · 🟡 có rải rác trong book summary/question bank nhưng **chưa có topic doc riêng** · ✅ đã lấp.
> **Ưu tiên:** 🔴 cao (hay hỏi + đang thiếu) · 🟠 vừa · 🟢 thấp/dài hạn.

---

## Nhóm BSP (Embedded / kernel / phần cứng)

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **DMA & cache coherency** | 🔴 | 🟡 | bank [BSP-011](../mock-interview/bank/bsp.md) (coherent vs streaming), rải trong [lkd/memory](../../15-book-summaries/lkd/04-memory.md) | Cần topic doc: coherent vs streaming, `dma_map_*`, cache flush/invalidate, IOMMU/SMMU, scatter-gather, dma_addr_t. Hay hỏi ở BSP. |
| **Yocto / Buildroot** | 🔴 | ✅ **đã lấp** | [06/yocto.md](../../06-build-systems/yocto.md) + bank [BLD](../mock-interview/bank/build-systems.md) (CMake/Yocto/CI) + [BSP-017…019](../mock-interview/bank/bsp.md) | Đã thêm topic doc Yocto (BitBake, recipe/layer/bbappend, machine/BSP layer, sstate, devtool, SDK, CVE) + domain BLD trong bank. |
| **CI/CD & automated test farm** | 🔴 | ✅ **đã lấp 2026-08-28** | 🆕 **[06/ci-and-test-farm.md](../../06-build-systems/ci-and-test-farm.md)** + bank [BLD-020…036](../mock-interview/bank/build-systems.md) (mục E: 1 → **18 câu**, gồm **§1 nền** [BLD-030…035](../mock-interview/bank/build-systems.md)) | ⚠️ **Gap này CHƯA TỪNG có trong sổ** — dù [bản đồ JD](datalogic-plan.md) ghi rõ trụ *"Yocto + CMake/**CI**"*. Trước 28/08 repo có **0 tài liệu** và **đúng 1 câu** ([BLD-010](../mock-interview/bank/build-systems.md)) cho cả mảng này. Lộ ra khi người học kể **tình huống thật ở Samsung** (gated check-in, 10 platform, test farm TV, Perforce↔Git) chứ không phải khi rà soát. Nay có doc riêng: **§1 khái niệm nền** (CI & *integration hell* · CI/Delivery/**Deployment** — embedded dừng ở Delivery vì "deploy" = OTA · từ vựng pipeline/DUT/HIL · host vs QEMU vs board · **hai trục phân loại test**) · CI embedded ≠ CI web (**board là tài nguyên phải bảo trì, không phải runner vứt đi**) · gated vs post-merge + **nút cổ chai & bisect lô** · build matrix (`#ifdef` tắt = **chưa từng được compiler đọc**) · **tháp test** smoke/robustness/soak + thứ tự cổng theo chi phí · **6 khối test farm** · ⭐ **quan sát ngoài vs test daemon** (pipeline dev vs nhà máy) · **đo latency phải từ ngoài** (hiệu ứng quan sát viên) · thu log & định tuyến defect theo chữ ký · **flaky làm cổng NGỪNG chặn** · Git↔Perforce + **patch chồng = mô hình Yocto** · 12 bẫy. 📌 **Bài học cho chính sổ này:** gap-register mới chỉ soi *"repo thiếu chủ đề gì so với sách"*, chưa soi *"repo thiếu gì so với **việc thật người học đã làm**"* — vốn là nguồn câu trả lời phân biệt nhất. |
| **PCI & USB drivers** | 🔴 | ✅ **đã lấp** | [05/pci-usb-drivers.md](../../05-drivers-device-tree/pci-usb-drivers.md) + bank [DRV-019…027](../mock-interview/bank/drivers-embedded.md) | JD Datalogic yêu cầu (I2C/SPI/**PCI/USB**). Trước repo chỉ có I2C/SPI/UART; nay thêm PCI (config space/BAR/MSI-X/bus master) + USB (descriptor/URB/gadget). |
| **Power management** | 🟠 | 🟡 | [melp/drivers-init-power](../../15-book-summaries/melp/drivers-init-power.md) cụm 3 | Chưa có topic doc. 4 tầng (cpufreq/cpuidle/runtime PM/system suspend), wakeup source, OPP, DVFS. |
| **Secure boot / chuỗi tin cậy** | 🟠 | ✅ **đã lấp 2026-08-28** | 🆕 **[08/secure-boot.md](../../08-embedded-systems/secure-boot.md)** + bank [BSP-026](../mock-interview/bank/bsp.md) (viết lại) · 🆕 [BSP-039](../mock-interview/bank/bsp.md) · 🧪 [BSP-035](../mock-interview/bank/bsp.md) | Trước đây **không có topic doc**, chỉ rải rác ở [melp/bootloader-kernel](../../15-book-summaries/melp/bootloader-kernel.md). Nay có doc riêng: bản chất *(toàn vẹn+xác thực, **KHÔNG** phải mã hoá)* · chuỗi tin cậy + **vì sao gốc phải bất biến** (mask ROM + eFuse) · TF-A BL1→BL33 · **verified boot ≠ secure boot** · cơ chế hash+ký · ⭐ **§4 ba tình huống thực tế của người học ở Samsung** (build image cần khoá → HSM ký hash, không giao khoá · flash 2 lớp kiểm → vì sao cần A/B · **build RPM rời lúc dev phải ký**) · **dm-verity vs IMA/EVM** · khoá DEV vs PRODUCTION · **anti-rollback** · 9 bẫy. ⚠️ **Còn thiếu có chủ đích:** chi tiết BL1–BL33 của TF-A và quy trình blow eFuse — **không thực hành được** (BBB là GP silicon) nên giữ ở mức khái niệm. |
| **Bus protocol chiều sâu (I2C/SPI/UART)** | 🟠 | ✅ **đã lấp** | [05/bus-protocols.md](../../05-drivers-device-tree/bus-protocols.md) + bank [BUS](../mock-interview/bank/drivers-embedded.md) | Đã có topic doc riêng (2026-08-09): bảng so sánh, khung UART + vì sao sai baud ra rác, open-drain/pull-up, START/STOP · ACK/NACK · **clock stretching** · arbitration, CPOL/CPHA 4 mode có waveform, flow control, góc device tree/Linux + 5 câu phỏng vấn. |
| **Bare-metal / MCU / RTOS fundamentals** | 🔴 | ✅ **đã lấp** | [08-embedded-systems](../../08-embedded-systems/): [bare-metal-c](../../08-embedded-systems/bare-metal-c.md), [memory-and-startup](../../08-embedded-systems/memory-and-startup.md), [interrupts-bare-metal](../../08-embedded-systems/interrupts-bare-metal.md), [rtos-programming](../../08-embedded-systems/rtos-programming.md), [hardware-debug](../../08-embedded-systems/hardware-debug.md) + bank [EMB](../mock-interview/bank/embedded-fundamentals.md) | Trước đây topic 08 chỉ có góc SoC/Linux; nay đủ nền Embedded SW chung: C/thanh ghi, startup/linker, ISR bare-metal, RTOS programming, debug HW. |
| **🧪 Bộ lab DBG (DBG-030…036)** | 🟡 | ⬜ **hoãn có chủ đích** | [bank/debugging.md](../mock-interview/bank/debugging.md) | 7 bài tạo 2026-08-17, **chưa làm bài nào**. **2026-08-27: chọn ưu tiên bộ lab BSP (`BSP-031…038`) TRƯỚC.** Căn cứ đo được: `DBG` **3.67** vs `BSP` **3.33** (thấp nhất mọi phiên `rapid`), và `BSP` là **trụ số 1 của JD**. Ghi lại để **lần sau không ai đảo ngược trong im lặng** — cùng cách đã làm với LDD3. Sau khi xong bộ BSP thì quay lại bộ này. |
| **LDD3 / Bootlin summary** | 🟠 | ⬜ **hoãn có chủ đích** | — (trong hàng đợi sách) | Char driver, ioctl, interrupt, DMA từ góc viết driver thật. **2026-08-18: người học chọn làm *C++ High Performance* TRƯỚC LDD3.** Ghi lại để lần sau không ai đảo ngược trong im lặng. Vẫn là gap 🟠 chưa lấp. |
| **pinctrl / clock / regulator framework** | 🟢 | 🟡 | [melp/bootloader-kernel](../../15-book-summaries/melp/bootloader-kernel.md) (nhắc pinctrl) | Chi tiết framework: consumer/provider, clk_get/prepare/enable, regulator_get. |
| **Device model kernel sâu (kobject/sysfs/uevent)** | 🟢 | 🟡 | [lkd/modules-debug](../../15-book-summaries/lkd/06-modules-debug.md) cụm 1 | Có khung; có thể đào sâu thêm cho vị trí driver-heavy. |

---

## Nhóm C++ / System Software

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **Memory model & lock-free** (atomic ordering, happens-before, ABA, hazard pointer) | 🟠 | ✅ **đã lấp** | **Ch. 5** ([05-memory-model](../../15-book-summaries/cpp-concurrency/05-memory-model.md) — happens-before, synchronizes-with, 6 memory_order, CAS, fences) + **Ch. 7** ([07-lock-free-structures](../../15-book-summaries/cpp-concurrency/07-lock-free-structures.md) — lock-free vs wait-free, 3 kỹ thuật reclamation, hazard pointer, **ABA problem**) | Trước đây **không tài liệu nào trong repo phủ**. Đã lấp trọn bằng ch. 5 + ch. 7 của C++ Concurrency in Action. |
| **C++20/23 depth** (ranges, concepts, coroutines) | 🟠 | 🟡 | [cpp-mindset](../../15-book-summaries/cpp-mindset/README.md) (concepts/ranges chạm nhẹ), [EMC++](../../15-book-summaries/effective-modern-cpp.md) (dừng C++14) | EMC++ chỉ tới C++14. Cần bổ sung: ranges/views sâu, concepts thay SFINAE, `std::expected`, `std::span`; **coroutines chưa có ở đâu**. **2026-08-18:** *C++ High Performance* có đủ ch. 6 (ranges), 8 (concepts), 12–13 (coroutines) nhưng **cố ý BỎ** vì JD Datalogic là **C++17**. Chuyển C++20 thì ba chương đó là nguồn sẵn có, không cần tìm sách mới. |
| **Template metaprogramming sâu** (SFINAE, CRTP, type traits) | 🟠 | 🟡 | [01/templates](../../01-cpp-fundamentals/templates.md) (cơ bản), [cpp-mindset/abstraction-in-detail](../../15-book-summaries/cpp-mindset/abstraction-in-detail.md) (traits) | Còn mỏng. CRTP (static polymorphism), tag dispatch, `enable_if`, variadic template, fold expression. |
| **Coroutines (C++20)** | 🟢 | ⬜ | — | Chưa có. co_await/co_yield, generator, task; ít hỏi ở embedded nhưng xuất hiện ở vị trí C++ hiện đại. |
| **TLPI summary** (userspace API sâu) | 🟢 | ✅ **đã lấp** | [the-linux-programming-interface/](../../15-book-summaries/the-linux-programming-interface/) — README + **10 cụm phủ đủ 64 chương**, đối chiếu PDF | *Sổ này ghi ⬜ tới 2026-08-18 dù sách đã xong từ trước — sửa khi rà soát. Bài học: gap-register phải cập nhật cùng lúc với bảng tiến độ sách.* |
| **Tối ưu bộ nhớ & cache (C++)** — arena/PMR, alignment & padding, cache locality có SỐ ĐO | 🟠 | ✅ **đã lấp 2026-08-18** | [cpp-high-performance/](../../15-book-summaries/cpp-high-performance/) — ch. 7 (memory) + ch. 4 (cache) + ch. 3 (đo) | **Gap này trước đây KHÔNG có trong sổ** — chỉ lộ ra khi đánh giá sách mới: repo nói *"tránh heap trong đường nóng"* mà chưa nói *dùng gì thay thế*; và cache locality chỉ có **1 đoạn văn** ở [12-dsa §5](../../12-dsa/complexity-and-structures.md). |
| **Networking chiều sâu** | 🟢 | 🟡 | [13-networking](../../13-networking/) | Topic 14 còn nhẹ. TLS handshake, HTTP/2, MQTT chi tiết nếu JD liên quan mạng. |

### 🧱 Rà soát TẦNG NỀN (2026-08-28) — *"có câu hỏi sâu" ≠ "có đường vào"*

Trục gap **thứ ba**, sau *độ phủ* (17/08) và *độ sâu tài liệu* (27–28/08). Câu hỏi rà: **"mục này có câu 🟢/🟡 mở đường vào không, hay bắt đầu thẳng từ giữa?"**

| Phát hiện | Kết quả |
|---|---|
| **Thiếu nền thật** — `bsp.md` không có câu vào cho **DMA · flash · MMIO · OTA** | ✅ lấp: 🆕 `BSP-040…044`; `BSP` **39 → 44**, **0 → 4 câu 🟢** |
| **Sai nhãn** — `DRV-015/016/017` là câu định nghĩa nhưng dán 🔴 ⇒ `rapid` không bao giờ rút tới | ✅ sửa 🔴 → 🟡 |
| **Trôi quy ước** — 6 file dùng blockquote + *"Ôn tập (bank)"* thay TL;DR + *"Câu hỏi phỏng vấn liên quan"* | ✅ chuẩn hoá; **56/56** tài liệu topic đủ cả hai |

📌 **Bài học phương pháp (lần thứ 3):** số đo cho **7 báo động giả** — `os.md C`, `linux C`, `drivers B`, `networking B`, `design-patterns B`, `system-design C`, `cpp I` đều **có** câu nền, chỉ là nhãn 🟡 chứ không phải 🟢. **Đếm nhãn không thay được việc đọc đề bài.**

📌 **Bài học cho bank:** `bsp.md` là domain duy nhất từng có **0 câu 🟢** ⇒ **sàn độ khó của `rapid track bsp` cao hơn track khác** ⇒ điểm **3.33** không so ngang được với domain khác. Khi so điểm giữa các domain, **kiểm phân bố level trước**.

### 🔬 Rà soát ĐỘ SÂU tài liệu (2026-08-28) — *"có tài liệu" ≠ "tài liệu đủ dày"*

> **Trục gap MỚI.** Sổ này từ trước tới nay chỉ theo dõi **độ phủ** (*có tài liệu về X chưa?*). Đợt này lộ ra trục thứ hai: **độ sâu**. `yocto.md` và `pci-usb-drivers.md` đều đang ghi **✅ đã lấp** ở bảng trên — mà thực tế là **698 và 954 từ, 0 bảng, 0 bẫy**, chủ yếu là **khẳng định**: không code mẫu, không *"vì sao"*, không *"khi nào dùng / khi nào không"*.
>
> ⇒ Đây đúng là bài học *"độ phủ không đo được độ vững"* (plan §📍 17/08) **áp cho tài liệu** thay vì cho bank. Ô ✅ chỉ nói *"đã viết gì đó"*, không nói *"viết đủ để ôn"*.

**Chuẩn tham chiếu:** `ipc-linux.md` sau khi viết lại 13/08 — 4.271 từ · 7 bảng · 14 bẫy · 6× *"vì sao"*.

| File | Trước | Sau | Ghi chú |
|---|---|---|---|
| [yocto.md](../../06-build-systems/yocto.md) | 698 từ · 0 bảng · 0 *vì sao* · 1 bẫy | **3.238 · 75 · 10 · 10** | Thêm: bản chất *(tái lập được, không phải "build Linux")* · **khi nào KHÔNG dùng** · cú pháp override `+=` vs `:append` + **cú pháp `_` cũ im lặng không tác dụng** · sstate theo hash · 4 lệnh gỡ rối |
| [pci-usb-drivers.md](../../05-drivers-device-tree/pci-usb-drivers.md) | 954 · 7 · 2 · 0 | **2.138 · 38 · 4 · 9** | ⭐ **`self-enumeration` ≠ `hotplug`** (bảng riêng) · vì sao I2C/SPI không tự liệt kê được · **host vs device (gadget)** · MSI-X chống đua với DMA |
| [device-tree.md](../../05-drivers-device-tree/device-tree.md) | 1.128 · 14 · 1 · 0 | **2.190 · 28 · — · 8** | ⭐ **`status="okay"` KHÔNG đủ** (4 nhóm tài nguyên) · **`-EPROBE_DEFER`** · overlay · **quy trình debug DT** + bảng chẩn theo triệu chứng |
| [cross-compilation.md](../../06-build-systems/cross-compilation.md) | 917 · 19 · 3 · 0 | **1.474 · 29 · — · 6** | ⭐ Cơ chế **`not found` = thiếu interpreter** (`PT_INTERP`) · 3 nguyên nhân · phép thử `-static` một bước |
| [rtos-vs-linux.md](../../08-embedded-systems/rtos-vs-linux.md) | 1.217 · 21 · 3 · 0 | **1.686 · 26 · — · 5** | ⭐ **Cái GIÁ của PREEMPT_RT**: throughput −5–20% · driver ẩu vỡ · **chỉ là điều kiện cần** (thiếu isolcpus/ghim IRQ/`mlockall` thì đuôi vẫn dài) |

**Bank nâng theo, 12 câu** — hết câu 🟠🔴 dưới 700 ký tự trong `BLD`/`BSP`/`DRV`:
`BLD-004/006/007/008` · `BSP-017/019/027` · `DRV-020/022/024/025/026`.

#### ⚖️ Hai bài học phương pháp — ghi để không lặp

**① Chuỗi nhân quả *tài liệu mỏng → bank mỏng → trả lời sai* đã đo được.**
`pci-usb-drivers.md` (954 từ) gánh **9 câu bank**, trong đó **5 câu mỏng** — *tất cả* đều trỏ về nó. Và đó chính là chỗ ứng viên sai ở [RES-012](../mock-interview/bank/resume.md) ngày 18/08 (*"PCI/USB là hotplug tự động"*).
⚠️ Điểm tinh tế: doc **không sai** — §1.1 có nói self-enumeration. Nó thiếu **sự phân biệt**: không đâu tách `self-enumeration` khỏi `hotplug`. ⇒ **Doc đúng vẫn dạy sai nếu không nêu ranh giới với khái niệm dễ nhầm.**

**② 🔴 ĐẾM TỪ KHOÁ ĐỂ XẾP HẠNG, NHƯNG PHẢI ĐỌC ĐỂ QUYẾT ĐỊNH — lần thứ HAI.**
`rtos-vs-linux.md` đo ra **0 bẫy** nên bị xếp ưu tiên 3; đọc thật thì nó **tốt**: biểu đồ phân bố latency, output `cyclictest` thật, bảng hard/firm/soft đầy đủ. Chỉ thiếu **cái giá** của PREEMPT_RT ⇒ bổ sung ít hơn hẳn hai file kia.
*(Lần đầu: `hardware-debug.md` — plan ghi "tôi chấm oan lúc đầu… 886 từ nhưng ĐẶC".)*

⇒ **Hệ quả cho `BSP-021` (2đ, sổ yếu): KHÔNG phải lỗ hổng tài liệu.** Doc có sẵn đáp án, kể cả định nghĩa *firm RT* mà ứng viên hỏi. Đây đúng là **"thuộc bài ≠ hiểu bài"** (19/08) — chữa bằng **đọc lại + tự diễn đạt**, không phải bằng viết thêm doc.

#### ⬜ Còn lại sau đợt này

| File | Số đo | Vì sao chưa làm |
|---|---|---|
| [cmake.md](../../06-build-systems/cmake.md) | 795 từ · 1 *vì sao* · 0 bẫy | Mỏng thật, nhưng CMake **đã đo là đạt** (`daily build-systems` 3.17, *"CMake đạt; nợ lệnh Yocto"*) ⇒ ưu tiên thấp |
| [kernel-userspace.md](../../05-drivers-device-tree/kernel-userspace.md) | 1.146 · 4 · 0 bẫy | Trung bình; chưa có phiên mock nào chỉ ra lỗ hổng |
| [rtos-programming.md](../../08-embedded-systems/rtos-programming.md) · [memory-and-startup.md](../../08-embedded-systems/memory-and-startup.md) | 1.429 / 1.331 · 0 bẫy | Phần lớn **ngoài JD** (bare-metal/MCU) — `EMB` 3% là **cố ý** ([config §7](../mock-interview/config.md)) |
| ❌ **KHÔNG flag** [hardware-debug.md](../../08-embedded-systems/hardware-debug.md) | 886 từ · 0 bẫy | **Đã phán định là ĐẶC**, không mỏng. Đừng xếp hạng lại nó bằng số đo |

---

### 🔎 Phát hiện từ phiên mock 15–17/08 — *lỗ hổng lộ ra khi bị hỏi, không phải khi đọc*

> Đặc điểm chung: **câu bank 🟠 trỏ tới file topic không chứa nội dung đó**. Mẫu này đã lặp **4 lần** ⇒ luật mới ở [config §7](../mock-interview/config.md): kiểm link nguồn có thật sự chứa nội dung **trước khi** đưa câu vào pool ôn.

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú / nên bổ sung |
|--------|---------|-----------|---------------|----------------------|
| **namespaces & cgroups** | 🔴 | ⬜ | **Không có tài liệu nào** — chỉ có bank [LNX-024](../mock-interview/bank/linux-sysprog.md) (đã viết lại 15/08) | `grep` toàn repo ra 0 kết quả. LNX-024 trỏ `→ ipc-linux.md` nhưng file đó chỉ có chữ "abstract namespace" của Unix socket — **chuyện khác hẳn**. Cần mục mới: ns (pid/mount/net/uts/ipc/user) vs cgroup (memory/cpu/io/pids), container = ns+cgroup+rootfs, **góc embedded: systemd unit** (`MemoryMax=`, `PrivateTmp=`, `TasksMax=`) chứ không phải Docker. Ứng viên **0 điểm** vì repo không dạy. |
| **Phát hiện peer chết trên TCP** | 🔴 | ⬜ | **Không có tài liệu nào** — chỉ có bank [LNX-040](../mock-interview/bank/linux-sysprog.md) | `grep -rn "keepalive\|ETIMEDOUT"` toàn repo ra 0 kết quả liên quan. Cần thêm vào [13/tcp-ip.md](../../13-networking/tcp-ip.md) cạnh §6: `write()` thành công ≠ đã nhận · retransmit → `ETIMEDOUT` **~15–20 phút** · `SO_KEEPALIVE` + `TCP_KEEPIDLE/INTVL/CNT` vs **heartbeat** tầng ứng dụng + đánh đổi. Lớp lỗi thật của thiết bị cầm tay rời sóng. |
| **LT + `EPOLLOUT` = busy-loop 100% CPU** | 🟡 | 🟡 | [04/io-multiplexing §8④](../../04-linux-system-programming/io-multiplexing.md) chỉ nói ET phía ghi | Bug phổ biến nhất của người mới viết event loop mà tài liệu **không nói**. Đã có bank [LNX-041](../mock-interview/bank/linux-sysprog.md) kèm số đo chạy thật (LT **5/5** vòng báo sẵn sàng · ET **1/5**). Nên đưa vào §8 của topic doc. |
| **`mmap` chiều sâu** | 🟡 | 🟡 | [04/file-io.md](../../04-linux-system-programming/file-io.md) nói về `mmap` đúng **1 dòng** (dòng 227) | Thiếu: page fault theo trang (300 MB = ~76.800 fault), **readahead** của `read()`, `SIGBUS` (truncate dưới chân mapping · lỗi đọc flash), `msync` & thứ tự ghi khi mất điện. Bank [LNX-026](../mock-interview/bank/linux-sysprog.md) **đã viết đủ 15/08** ⇒ hoặc viết mục cho topic, hoặc **sửa link** cho trỏ đúng chỗ. |
| **RSS nhắc lửng — bẫy chẩn đoán nhầm leak** | 🟡 | 🟡 | [09/memory-bugs.md:85](../../09-debugging/memory-bugs.md) một dòng bảng; phần *vì sao* nằm ở [03/memory-management.md](../../03-operating-system/memory-management.md) **không có link nối** | `free()` không trả RAM về OS (glibc giữ arena) ⇒ **RSS không giảm là BÌNH THƯỜNG**, phải xem *xu hướng* chứ không một thời điểm. Chỉ cần **thêm link chéo**. *(Ứng viên tự phát hiện khi ôn 16/08.)* |

---

## Nhóm chung (kỹ năng phỏng vấn)

| Chủ đề | Ưu tiên | Trạng thái | Hiện có ở đâu | Ghi chú |
|--------|---------|-----------|---------------|---------|
| **Behavioral / STAR stories** | 🟠 | ✅ **đã lấp** | bank [BEH-001…009](../mock-interview/bank/behavioral.md) + track `behavioral` của `/mock` | Kiểm tra đã đủ câu chuyện STAR chưa (conflict, failure, leadership, khó khăn kỹ thuật). |
| **Mock interview scaffolding** | 🟢 | ✅ **đã lấp** | [14-prep/mock-interview/](../mock-interview/) | Module phỏng vấn thử tương tác (`/mock`) + ngân hàng câu hỏi duy nhất (bank/) + track/type/sessions/weak-register/coding-arena(+reviewed/). |
| **System design practice (embedded)** | 🟠 | 🟡 | [10/system-design](../../10-thinking/system-design.md), bank [SD](../mock-interview/bank/system-design.md) | Có khung; nên thêm 2–3 bài tập system design nhúng đã giải mẫu (vd: thiết kế firmware update, data pipeline sensor). |

---

## Cách lấp một lỗ hổng

Khi quyết định lấp một mục:
1. Nếu là **kiến thức nền** → viết vào **topic** tương ứng (vd DMA → thêm file vào [05-drivers-device-tree/](../../05-drivers-device-tree/) hoặc topic mới; Yocto → bổ sung [06-build-systems/](../../06-build-systems/)).
2. Nếu là **chiều sâu từ một cuốn sách** → làm book summary (theo [quy ước 16/README](../../15-book-summaries/README.md)).
3. Cập nhật lại ô *Đọc/Hiểu* tương ứng trong [datalogic-plan](datalogic-plan.md): bỏ dấu 🕳️, trỏ link tới tài liệu mới.
4. Đổi trạng thái ở bảng trên thành ✅.
