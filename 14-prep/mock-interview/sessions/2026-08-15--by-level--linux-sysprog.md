# 🎤 Phiên mock · 2026-08-15 · `by-level` 🟠 · track `linux-sysprog`

- **Tham số:** `by-level` 🟠 · 10 câu · **trần T2** · Tuần 2 Buổi 2 của [datalogic-plan](../../study-plans/datalogic-plan.md)
- **Điểm tổng: 2.1/4** *(bỏ Q10 — câu repo không có tài liệu: 2.33 · bỏ cả Q8+Q10: 2.63)*
- **Bối cảnh:** trước phiên, trong cùng conversation đã giải thích **ET/vét cạn `EAGAIN` phía đọc** và **`SIGPIPE`/`SIG_IGN`** ⇒ hai góc đó **bị cấm hỏi lại**, phải hỏi ở mặt khác (Q3 hỏi phía **ghi**, Q10 hỏi ca **không có RST**).

> ⚠️ **2.1 KHÔNG so sánh trực tiếp được với 2.67 của phiên 13/08.** Phiên đó là `rapid` (đo trôi chảy T1, thang chấm riêng); phiên này là `by-level` 🟠 — tầng khó nhất còn tính điểm, mọi câu đều tình huống nhiều tầng.

## Kết quả từng câu

| # | Câu | Điểm | Một dòng |
|---|---|---|---|
| 1 | LNX-023 `/proc` `/sys` 🔴*weak* | **3** 🔼 | 0 → 3. Dùng được rồi; cơ chế "kernel sinh lúc `read()`" vẫn mờ |
| 2 | LNX-009 epoll vs poll | **3** | Phần review patch xuất sắc; né câu "đo cái gì" |
| 3 | LNX-010 LT/ET **phía ghi** | **3** | Chẩn đoán (a) chuẩn xác; không chuyển được ET sang phía `write` |
| 4 | LNX-026 mmap vs read | **2** ⚠️ | Cơ chế có trong đầu nhưng phải khoan 2 lượt mới ra |
| 5 | LNX-029 clock 🔴*weak* | **2** 🔼 | 1 → 2. Nền đã vá xong; vận dụng vẫn trắng |
| 6 | LNX-005 framing 🔴*weak* | **2** ➡️ | 2 → 2. Đọc code tốt, **không sửa được** |
| 7 | **LNX-039** 🆕 backpressure/OOM | **3** | (a)(b)(c) mạnh nhất phiên; hụt đúng câu chốt cơ chế |
| 8 | LNX-024 namespaces/cgroups | **0** ⚠️ | Trắng — kèm lỗ hổng tài liệu thật |
| 9 | LNX-015 shm + một bên chết | **3** | Sát 4. Tự đặt câu hỏi hay nhất phiên → sinh **LNX-042** |
| 10 | **LNX-040** 🆕 write sau khi peer biến mất | **0** ⚠️ | Trắng — repo **không có dòng nào** về keepalive/heartbeat |

---

## ⚖️ Phân định phản hồi *"quá mở rộng so với bank"* (ứng viên nêu 3 lần)

Kiểm bằng `grep`, không phán cảm tính. Kết quả **không đồng nhất**:

| Câu | Phán định | Bằng chứng |
|---|---|---|
| **Q7** backpressure | ❌ **Không đồng ý** | Nằm nguyên trong bài đọc hôm nay: `io-multiplexing.md` §8 bẫy ⑤ |
| **Q8** namespaces/cgroups | 🟡 **Nửa đồng ý** | Câu **có** trong pool §📍 (`LNX-013…026`) ⇒ hợp lệ. Nhưng **toàn repo không dạy nó**; LNX-024 trỏ `→ ipc-linux.md`, file đó chỉ có một chữ "namespace" và là *abstract namespace của Unix socket* — chuyện khác hẳn |
| **Q10** write sau khi peer biến mất | ✅ **Đồng ý hoàn toàn** | `grep -rn "keepalive\|ETIMEDOUT"` toàn repo → 0 kết quả liên quan. **Lỗi của interviewer** — ra câu repo không đỡ được |

**Quy tắc rút ra:** phản hồi "quá sâu" phải được **kiểm bằng grep trước khi trả lời**, và trả lời theo **từng câu một**, không gật/không cãi cả cụm.

---

## 🔎 Chi tiết ôn

### Câu 1 · LNX-023 · 🟠 (weak) · **3/4** 🔼 *(0 → 3)*

Daemon chạy trên thiết bị, sau **6 ngày** log `accept: Too many open files` rồi từ chối kết nối mới. Restart là hết, vài ngày sau tái diễn. Thiết bị chỉ có busybox — **không `lsof`, không `strace`, không cài thêm được gì**.

**(a)** Vào `/proc/<pid>/` xem cái gì để **khẳng định** đây là rò fd, và biết **rò loại fd nào**?
**(b)** Đồng nghiệp hỏi: *"sao mấy file trong `/proc` không bao giờ thấy trên `df`, mà `cat` ra vẫn có nội dung mới mỗi lần?"*

**🔁 Follow-up:** (c1) cái gì phân biệt "đang rò" với "đang bận hợp lệ"? (c2) trần fd là bao nhiêu, đọc ở đâu ra ngay trên thiết bị? (d) `ls -l /proc/<pid>/status` báo **size = 0** nhưng `cat` ra mấy chục dòng, hai lần cách nhau 1 giây cho số khác nhau — **mấy chục dòng đó nằm sẵn ở đâu**, **ai** sinh ra, **vào lúc nào**? (e1) `mount` ghi `type proc` chứ không phải `ext4` — nói lên gì? (e2) bạn viết driver, phơi tham số cấu hình ra userspace: chọn `/proc` hay `/sys`?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ xem `fd` và `fdinfo` (offset, inode), `ls -al` ở `/proc/<pid>/fd`.
- (b) ❌ *"chỉ là các biến entry, không tạo áp lực lên memory như df"* — lủng củng, không phải cơ chế.
- (c1) ✅ **tăng đơn điệu, không ổn định** — đúng dấu hiệu.
- (c2) ✅ `cat limits`. ⚠️ *"trần phụ thuộc loại máy"* — không chính xác, nó là `RLIMIT_NOFILE` **của process**, do systemd `LimitNOFILE=`/`ulimit` đặt.
- (d) ❌ **"Chưa rõ"**.
- (e1) 🟡 *"không nằm trên filesystem, mà phát sinh liên tục trên RAM theo thời gian"* — nửa đúng.
- (e2) ✅ chọn `/sys`, lý do *"sysfs là chuẩn cho cấu hình, proc là legacy, không quy ước"*.

**✅ Được:** ba thao tác chẩn đoán (a)(c1)(c2) đều **dùng được thật** trên thiết bị trần. Phần thiết kế `/proc` vs `/sys` (e2) đúng. So với lần 13/08 trắng cả hai lần hỏi, đây là tiến bộ lớn nhất phiên.

**❌ Vì sao chưa 4:** cơ chế lõi vẫn mờ — đúng chỗ lần trước cũng thiếu. *"Phát sinh liên tục trên RAM"* **sai**: không ai duy trì sẵn nội dung đó ở đâu cả.

**Đáp án đầy đủ:**

> Cả hai là **virtual fs — kernel sinh nội dung ngay lúc đọc**, không nằm trên đĩa. — bank [LNX-023](../bank/linux-sysprog.md)

Lúc `cat`, kernel **chạy một hàm** dựng chuỗi tại chỗ từ struct đang sống, trả về, xong vứt. Vì thế `size = 0` (không có gì để đo trước) và hai lần `cat` cho số khác nhau.

**Hệ quả thực dụng — đáng nhớ hơn định nghĩa:** `/proc` không tốn disk, không tốn RAM lưu trữ, không bao giờ "cũ" ⇒ trên thiết bị busybox không công cụ, nó là **cửa sổ duy nhất nhìn vào kernel, chỉ bằng `cat`**.

Mảng chưa chạm: `/proc/<pid>/fd` cho biết **rò loại gì** (rừng `socket:[12345]` = rò socket · rừng đường dẫn = quên `fclose`); `/proc/<pid>/stack` + `wchan` cho biết process `D` kẹt ở syscall nào — **thay được `strace`**.

**Chốt:** *"`/proc`,`/sys` không phải file — là hàm kernel đội lốt file. Đọc là gọi hàm."*

**Lần sau sẽ hỏi:** cho một daemon `D` state không phản hồi `SIGTERM` → *"chỉ có `cat`, xác định nó kẹt ở đâu bằng file nào?"* (ép dùng `wchan`/`stack`), rồi *"vì sao `/sys` bắt một-giá-trị-một-file mà `/proc/meminfo` thì nhét cả trăm dòng?"*

**Ôn:** [09-debugging/tools.md](../../../09-debugging/tools.md) · [05/kernel-userspace.md](../../../05-drivers-device-tree/kernel-userspace.md) · bank [LNX-023](../bank/linux-sysprog.md)
</details>

---

### Câu 2 · LNX-009 · 🟠 ⭐ · **3/4**

Daemon gateway giữ ~**20 kết nối TCP**. Vòng lặp hiện tại:

```c
struct pollfd fds[MAX];
for (;;) {
    int n = poll(fds, nfds, timeout_ms);      // nfds ~ 20
    for (int i = 0; i < nfds; i++)
        if (fds[i].revents & POLLIN) handle(fds[i].fd);
}
```

Đồng nghiệp viết lại bằng `epoll`, lý do *"epoll nhanh hơn, O(1) thay vì O(n)"*. Đo lại: **CPU và latency không cải thiện, thậm chí nhích lên một chút**.

**(a)** Vì sao kết quả đo như vậy? **(b)** Đổi sang **5.000 kết nối**, vài chục cái có dữ liệu — cụ thể chi phí nào biến mất? **(c)** Vai người review: chấp nhận hay từ chối patch?

**🔁 Follow-up:** (d) kết quả là **chậm hơn**, không phải bằng — cái gì được *thêm vào*? (e) **ở mức bao nhiêu kết nối** thì đổi ý — **bạn đo cái gì** để quyết định, thay vì đoán?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) 🟡 *"20 kết nối thì O(20) không khác biệt nhiều"* — đúng nhưng mới là "không hơn".
- (b) ✅ đầy đủ: poll O(5000) kể cả fd không có gì; epoll chỉ báo fd có dữ liệu → O(vài chục).
- (c) ✅✅ **Từ chối.** Hai lý do: *"không đổi thứ đang chạy ổn định lấy cải thiện không đáng, phải chịu rủi ro viết lại"* + *"poll là POSIX, epoll chỉ có trên Linux, deploy đa nền tảng là tự làm khó"*.
- (d) 🟡 *"epoll_ctl tốn chi phí ban đầu"* ✅; *"quét qua event fd tốn kém hơn quét poll thường"* ❌ (ngược — `epoll_wait` chỉ trả về fd đã sẵn sàng nên quét **rẻ hơn**).
- (e) ❌ *"hàng ngàn trở lên, đa số idle"* — **né mất phần tôi hỏi**: đo cái gì.

**✅ Được:** (b) chuẩn. **(c) là phần mạnh nhất câu này** — đúng hai trục một reviewer thật cân nhắc: rủi ro thay đổi vs lợi ích đo được, và khoá nền tảng.

**❌ Vì sao chưa 4:** hai chỗ.

**① (a) thiếu vế "vì sao chậm hơn".** Cái *thêm vào* ở bản epoll: một syscall **`epoll_ctl`** mỗi lần add/mod/del, kernel phải chèn/xoá node trong cấu trúc theo dõi, giữ thêm một fd cho epoll instance. Với 20 fd, mảng `pollfd` gọn **nằm vừa cache**, quét 20 phần tử rẻ hơn phần chi phí quản lý đó.

**② (e) — quyết định bằng cảm tính thay vì phép đo.** Câu trả lời đúng không phải một con số mà là **một tỉ lệ**:

- Đo **`số fd sẵn sàng / tổng số fd`** mỗi vòng. Gần 1 (phần lớn fd đều có việc) ⇒ epoll **không giúp gì** dù 10.000 fd. Nhỏ (10/5000) ⇒ epoll thắng đậm.
- Cộng thêm: thời gian CPU nằm trong syscall (đo bằng profiler), và tần suất `epoll_ctl` (nếu phải MOD liên tục thì lợi thế bị ăn mòn).

> Ưu thế chỉ xuất hiện khi **n lớn và phần lớn idle**. — bank [LNX-009](../bank/linux-sysprog.md), Bẫy (1)

**Chốt:** *"epoll không nhanh hơn — nó rẻ hơn theo **số fd nhàn rỗi**. Không có fd nhàn rỗi thì không có gì để tiết kiệm."*

**Lần sau sẽ hỏi:** cho một profile thật (`n=8000`, trung bình 6.000 fd sẵn sàng mỗi vòng) → *"epoll có giúp không, vì sao?"* — ép dùng tỉ lệ thay vì ngưỡng.

**Ôn:** [io-multiplexing.md §1, §4](../../../04-linux-system-programming/io-multiplexing.md) · bank [LNX-009](../bank/linux-sysprog.md)
</details>

---

### Câu 3 · LNX-010 · 🟠 · **3/4**

Daemon dùng `epoll` **level-triggered**. Response lớn (vài trăm KB) nên phải ghi nhiều lượt. Đăng ký một lần lúc `accept`:

```c
struct epoll_event ev;
ev.events  = EPOLLIN | EPOLLOUT;        // đăng ký một lần, không đụng lại
ev.data.fd = conn_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);

int n = epoll_wait(epfd, events, MAX, -1);
for (int i = 0; i < n; i++) {
    if (events[i].events & EPOLLIN)  do_read(events[i].data.fd);
    if (events[i].events & EPOLLOUT) do_write(events[i].data.fd);   // không có gì gửi thì return luôn
}
```

Triệu chứng: **100% CPU một core ngay cả khi không client nào gửi gì**, `epoll_wait` trả về liên tục.

**(a)** Chuyện gì xảy ra? Chỉ đúng dòng. **(b)** Sửa thế nào — **hai** cách khác nhau về bản chất + đánh đổi.

**🔁 Follow-up:** (c) **cấm gọi `epoll_ctl` thêm lần nào sau `accept`** — vẫn phải hết busy-loop. Còn đường nào?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅✅ **chuẩn xác**: *"`EPOLLOUT` nghĩa là **sẵn sàng ghi**, không phải đã có dữ liệu để ghi. `epoll_wait` liên tục báo dù không có gì để ghi → 100% CPU."* Chỉ đúng dòng 2.
- (b) ✅ một cách: bỏ `EPOLLOUT` lúc đầu, khi có dữ liệu mới set. Nêu được giá: thêm `epoll_ctl` giữa vòng lặp đang chạy. ❌ *"chưa nghĩ ra cách 2"*.
- (c) ❌ **"chưa rõ"**.

**✅ Được:** (a) là một trong hai câu chẩn đoán tốt nhất phiên. Câu chốt *"sẵn sàng ghi ≠ có gì để ghi"* chính xác là bản chất bug này.

**❌ Vì sao chưa 4 — và đây là chỗ đáng tiếc nhất phiên:** cùng conversation, 30 phút trước đã bàn kỹ ET phía **đọc** (vét cạn tới `EAGAIN`). Khi ràng buộc cấm `epoll_ctl`, **không chuyển được ý niệm sang phía ghi** dù cơ chế giống hệt.

**Đáp án đầy đủ (cách 2):** đăng ký **`EPOLLOUT | EPOLLET`** một lần. ET chỉ báo lúc **chuyển trạng thái** — buffer gửi từ "đầy" sang "có chỗ". Không ghi gì ⇒ không có chuyển biến ⇒ không báo ⇒ hết busy-loop.

**Chạy thật** (`g++ -Wall -Wextra`, socketpair, đăng ký `EPOLLOUT`, **không ghi gì**, gọi `epoll_wait` 5 vòng):

```
LT  (EPOLLOUT):              epoll_wait bao san sang 5/5 vong   <- busy-loop
ET  (EPOLLOUT|EPOLLET):      epoll_wait bao san sang 1/5 vong   <- chi canh dau tien
```

> **④ Chỉ vét cạn phía đọc mà quên phía ghi.** Với `EPOLLET`, `EPOLLOUT` **cũng là edge**: ghi tới khi `EAGAIN` mới thôi. — [io-multiplexing.md §8](../../../04-linux-system-programming/io-multiplexing.md)

**Đánh đổi hai cách:**

| | `EPOLL_CTL_MOD` bật/tắt | `EPOLLOUT \| EPOLLET` |
|---|---|---|
| Chi phí | 2 syscall mỗi lượt gửi | 0 syscall thừa |
| Rủi ro | thấp, dễ đúng | quên vét cạn `write` tới `EAGAIN` ⇒ **kẹt vĩnh viễn** |
| Hợp khi | mặc định, tải vừa | tải rất cao, đã có kỷ luật vét cạn |

**Chốt:** *"LT báo theo **mức**: 'còn ghi được là còn nhắc' — mà socket thì gần như luôn ghi được. ET báo theo **sườn**, nên im lặng khi không có gì đổi."*

**📌 Nợ tài liệu:** `io-multiplexing.md` §8④ dạy ET phía ghi, nhưng **không chỗ nào nói LT + `EPOLLOUT` thường trực = busy-loop 100% CPU** — bug phổ biến nhất của người mới viết event loop. Đã ghi vào nợ plan.

**Lần sau sẽ hỏi:** đưa code ET phía ghi **thiếu vòng vét cạn** → *"gửi file 300 KB, gửi được 64 KB rồi đứng — vì sao?"*

**Ôn:** [io-multiplexing.md §5, §8](../../../04-linux-system-programming/io-multiplexing.md) · bank [LNX-010](../bank/linux-sysprog.md), [LNX-041](../bank/linux-sysprog.md)
</details>

---

### Câu 4 · LNX-026 · 🟠 · **2/4** ⚠️ *(mới → vào sổ yếu)*

File dữ liệu **300 MB** trên flash, quét **tuần tự** lúc khởi động bằng vòng `read()` buffer 64 KB. Đồng nghiệp viết lại bằng `mmap`: *"zero-copy, chắc chắn nhanh hơn"*. Kết quả: thời gian khởi động **không giảm**, và thỉnh thoảng process **chết bằng `SIGBUS`** ở hiện trường, **không tái hiện được ở bàn làm việc**.

**(a)** Vì sao "không copy" mà không nhanh hơn? Cơ chế nào của `read()` bù lại phần copy? **(b)** `SIGBUS` từ đâu ra? **(c)** Khi nào **thật sự** chọn `mmap` — cho một tiêu chí quyết định.

**🔁 Follow-up:** (d1) `mmap()` **trả địa chỉ hợp lệ**, chạy được ~200 MB rồi mới chết ⇒ giả thuyết "map không đủ lớn" bị loại. Chết **giữa chừng** lúc chạm một trang cụ thể nói lên điều gì — **ai** đưa dữ liệu vào trang đó tại khoảnh khắc bạn chạm, và có thể thất bại vì gì? (d2) bản `read()` gọi ~**4.800** syscall, bản `mmap` gọi **1** — vẫn không nhanh hơn, vậy `mmap` trả chi phí gì **lặp lại theo từng 4 KB**?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) 🟡 *"đọc một lần thì mmap không phát huy; mmap chỉ lợi khi giảm copy và dùng nhiều lần"* — đúng hướng, thiếu cơ chế bù trừ.
- (b) ❌ *"mmap có thể không được truncate đúng cách, hoặc memory map của process không đủ lớn"* — vế sau **sai** (thiếu địa chỉ thì lỗi ngay tại `mmap()`).
- (c) 🟡 *"dung lượng cho phép + đọc lặp đi lặp lại nhiều lần"* — thiếu chữ then chốt.
- (d1) 🟡 *"Chưa rõ. Có thể file bị chỉnh sửa ngay lúc đọc"* — **đúng hướng** nhưng gói trong phỏng đoán.
- (d2) 🟡 *"Chưa rõ. Vẫn cần synchronize từng page 4KB vào RAM khi động đến"* — **đây chính là page fault**, chỉ thiếu tên và thiếu phép nhân.

**❌ Vì sao 2 điểm:** T1 có, T2 phải khoan hai lượt mới ló ra và không bao giờ đủ. Đáng chú ý về **cách trả lời**: hai lần bạn mở đầu bằng *"chưa rõ"* rồi nói ra thứ gần đúng — ở phỏng vấn thật, thói quen này khiến interviewer chấm **thấp hơn thực lực**.

**Đáp án đầy đủ:**

**(a) Cơ chế bù trừ = readahead.** Quét tuần tự thì kernel *đoán trước*, nạp sẵn block kế tiếp, I/O chồng lấn với xử lý ⇒ phần copy 64 KB gần như miễn phí. Còn `mmap` trả **một page fault mỗi trang 4 KB**: 300 MB / 4 KB = **~76.800 lần trap vào kernel**, cộng áp lực TLB. Có con số thì kết luận tự hiện ra.

**(b) `SIGBUS` — hai kịch bản thật:**
1. **File co lại dưới chân mapping** (`ftruncate`/tiến trình cập nhật ghi đè) → chạm trang quá EOF → `SIGBUS`.
2. **Lỗi đọc flash** ở đúng trang đó → `mmap` **không có giá trị trả về ở mỗi lần chạm** nên không còn đường báo lỗi nào ⇒ kernel bắn `SIGBUS`.

⇒ Giải thích luôn *"chỉ xảy ra ở hiện trường"*: flash mòn + tiến trình cập nhật chạy song song, cả hai đều không có ở bàn làm việc.

**Khác biệt bản chất đáng nhớ nhất:** `read()` trả lỗi **qua giá trị trả về** (xử lý được); `mmap` trả lỗi **qua signal** (mặc định là chết). **Đổi mô hình truy cập là đổi luôn mô hình báo lỗi.**

**(c) Tiêu chí đủ:**

> Chọn theo **mẫu truy cập**: **ngẫu nhiên / chia sẻ giữa process** → mmap; **tuần tự / streaming / file nhỏ** → read/write. — bank [LNX-026](../bank/linux-sysprog.md)

Chữ **ngẫu nhiên** là chìa khoá (bỏ được `lseek`+`read` mỗi lần nhảy); **chia sẻ** là lý do thứ hai (nhiều process map cùng file = **một** bản trong RAM vật lý). Ghi thì thêm `msync` để đảm bảo độ bền.

**Chốt:** *"mmap không phải 'read nhanh hơn'. Nó đổi copy lấy page fault, và đổi mã lỗi lấy signal. Tuần tự một lần thì cả hai vế đều lỗ."*

**📌 Nợ tài liệu:** LNX-026 trỏ `→ file-io.md`, mà cả file nói về `mmap` đúng **một dòng** (dòng 227) — không `SIGBUS`, không page fault, không readahead. **Lần thứ ba** gặp mẫu "câu bank 🟠 trỏ `file-io.md` mà file không dạy" (trước đó: LNX-023, LNX-029).

**Lần sau sẽ hỏi:** đảo chiều sang **ghi** — *"log ghi bằng `mmap` + `msync`, mất điện đột ngột thì mất gì so với `write`+`fsync`?"*

**Ôn:** bank [LNX-026](../bank/linux-sysprog.md) *(đã viết lại 2026-08-15)* · [file-io.md](../../../04-linux-system-programming/file-io.md) *(đang thiếu — xem nợ)*
</details>

---

### Câu 5 · LNX-029 · 🟠 (weak) · **2/4** 🔼 *(1 → 2, chưa qua ngưỡng gỡ)*

Thiết bị **không có RTC**, boot lên đồng hồ là 1970, NTP đồng bộ xong sau ~45 giây. Trong 45 giây đó daemon đã chạy và đã đo vài phép timeout.

**(a)** Đúng lúc NTP nhảy giờ, một `pthread_cond_timedwait(..., 5 giây)` đang chờ dở — hành xử thế nào, **cả hai chiều** (nhảy tiến / nhảy lùi), và phụ thuộc cái gì trong code?
**(b)** Log ghi timestamp giờ thật (đối chiếu với khách), đo hiệu năng ghi theo đồng hồ đơn điệu. Khách báo *"14:32:07 hệ thống đứng 3 giây"* — cần tìm **phép đo nào** ứng với dòng log đó. Thiết kế thế nào?

**🔁 Follow-up:** (c1) được in thêm bất cứ gì vào log — in **cái gì**, **bao lâu một lần**? (c2) còn đúng không sau khi NTP **nhảy thêm lần nữa** lúc 3 giờ sáng?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ **cả hai chiều đúng**: nhảy tiến ⇒ timeout ngay lập tức rồi check condition; nhảy lùi ⇒ treo rất lâu mới timeout. ✅ *"phụ thuộc clock đang sử dụng"*.
- (b) 🟡 *"xác định monotonic tại thời điểm bắt đầu treo và cộng dồn 3 giây"* — mô tả phép đo, không phải cách khớp. *"Thiết kế thế nào chưa rõ."*
- (c1) ❌ trả lời **nhầm câu**: mô tả cách *lọc* log chậm (elapsed > 500 ms thì in).
- (c2) ❌ **"chưa rõ"**.

**✅ Được — phần nền giờ đã chắc:** đúng thứ trắng lần trước. Lần 13/08 chỉ suy ra được **một** chiều và hỏi ngược *"NTP là gì?"*. Nay nêu đủ hai chiều + biết nó phụ thuộc clock của đối tượng chờ.

**❌ Vì sao vẫn 2:** theo [config §6 luật ①](../config.md), với câu weak thì **phần nền không còn là thứ được đo** — điểm nằm ở follow-up, và (b)(c) hụt cả cụm.

**Đáp án đầy đủ:**

**① Chỗ sửa cho (a)** — biết triệu chứng mà không biết chỗ sửa thì vẫn viết ra bug đó:
```c
pthread_condattr_t attr;
pthread_condattr_init(&attr);
pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);   // ✅ mặc định là CLOCK_REALTIME = cái SAI
pthread_cond_init(&cv, &attr);
```
Quy tắc: *"mấy giờ" → REALTIME · **"bao lâu" → MONOTONIC***. C++: `system_clock` vs **`steady_clock`**.

**② (b)(c) — dòng NEO chứa CẢ HAI mốc**, đọc gần như cùng lúc, in **định kỳ**:
```
[anchor] realtime=2026-08-15T14:31:00.000Z monotonic=91234.567
```
Có cặp neo ⇒ `offset = realtime − monotonic` ⇒ đổi qua lại cả hai chiều.

**(c2) chính là lý do phải in ĐỊNH KỲ chứ không phải một lần lúc boot:** mỗi lần NTP nhảy, **offset đổi**, neo cũ thành sai. In lại mỗi vài phút (hoặc ngay sau mỗi lần phát hiện nhảy) ⇒ mỗi đoạn log luôn có một neo còn hiệu lực gần đó. Thay thế: ghi cả hai mốc trên **mọi** dòng log (tốn chỗ), hoặc dùng `CLOCK_BOOTTIME` (đơn điệu, có tính cả thời gian suspend).

**Chốt:** *"Hai đồng hồ không quy đổi được cho nhau vì hiệu số của chúng thay đổi. Muốn khớp thì phải **ghi lại hiệu số đó theo thời gian**."*

**Lần sau sẽ hỏi:** *(giữ tầng vận dụng, đổi vỏ)* — *"thiết bị suspend 2 tiếng rồi wake; `steady_clock` đo được 3 giây, thực tế trôi 2 tiếng. Timeout của bạn hành xử ra sao, chọn clock nào?"* (`CLOCK_MONOTONIC` vs `CLOCK_BOOTTIME`).

**Ôn:** [TLPI cụm 03 §5](../../../15-book-summaries/the-linux-programming-interface/03-signals-and-timers.md) · bank [LNX-029](../bank/linux-sysprog.md)
</details>

---

### Câu 6 · LNX-005 · 🟠 (weak) · **2/4** ➡️ *(2 → 2, chưa nhúc nhích)*

Giả sử đã có `read_full()` đúng. Protocol dùng **ký tự phân tách**: message kết thúc bằng `'\n'`.

```c
char buf[4096];
size_t len = 0;
for (;;) {
    ssize_t n = read(fd, buf + len, sizeof(buf) - len);
    if (n <= 0) break;
    len += n;
    char *nl = memchr(buf, '\n', len);
    if (nl) { handle_message(buf, nl - buf); /* dồn phần dư về đầu */ }
}
```

**(a)** Client gửi **1 MB liên tục không có `'\n'`** — chuyện gì xảy ra, ở dòng nào? **(b)** Sửa thế nào? Với **200 kết nối** cùng làm vậy thì hệ quả tầng hệ thống là gì? **(c)** Nếu đổi sang **length-prefix** (4 byte độ dài) thì lớp lỗi này còn không — phải cẩn thận điều gì mới?

**🔁 Follow-up:** (d1) buffer đầy ⇒ `read(fd, buf+4096, 0)` — **count = 0** trả về gì, kết nối kết cục thế nào, người vận hành thấy gì trong log? (d2) đọc 4 byte độ dài ra **`0xFFFFFFFF`** — dòng code tiếp theo là gì, và chuyện gì xảy ra trên thiết bị **256 MB RAM**?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ lần đúng: `memchr` luôn `NULL`, `handle_message` không bao giờ chạy, `len` tăng tuyến tính tới đầy.
- (b) ❌ **"Chưa rõ."**
- (c) 🟡 *"tôi nghĩ là không còn, vì đã có quy ước mới; gia tăng gánh nặng xử lý"* — vế sau không phải trọng tâm.
- (d1) ✅ trả về **0**. 🟡 *"người vận hành sẽ nghĩ là đang có lỗi"* — thực tế **ngược lại**.
- (d2) 🟡 *"phải kiểm tra range để xác nhận length hợp lệ"* ✅ đúng hướng; *"chuyện xảy ra trên 256MB RAM chưa rõ"* ❌.

**✅ Được:** (a) đọc code chuẩn xác — cùng với Q3(a) là hai câu chẩn đoán tốt nhất phiên. (d1) đúng giá trị trả về.

**❌ Vì sao vẫn 2:** **chẩn đoán được, sửa không được.** (b) trắng hoàn toàn — mà (b) mới là phần được trả lương.

**Đáp án đầy đủ:**

**① Hệ quả thật của `read(..., 0)` — chỗ chết người bạn chưa lần tới.** Kiểm chứng bằng compiler (`gcc -Wall -Wextra`, pipe có sẵn 11 byte):
```
read(count=0)  -> n=0  errno=0     <- khong phai loi, khong phai EOF
read(count=16) -> n=11             <- du lieu VAN CON NGUYEN do
```
`n <= 0` ⇒ `break` ⇒ kết nối bị đóng. Dữ liệu **vẫn còn nguyên trong kernel**, không lỗi nào được ghi, `errno` sạch. Log trông **y hệt một client tự ngắt kết nối bình thường** ⇒ người vận hành **sẽ không nghĩ gì cả** — đó mới là điều tệ, không phải "nghĩ là có lỗi".

**② (b) Cách sửa:**
1. Đặt **trần cho một message** (vd 64 KB). Buffer đầy mà chưa thấy `'\n'` ⇒ **protocol violation** ⇒ log rõ + đóng kết nối. **Không bao giờ để buffer lớn theo dữ liệu do bên kia điều khiển.**
2. **200 kết nối:** buffer cố định 4 KB ⇒ 800 KB, chịu được. Nhưng nếu ai đó "sửa bug" bằng cách cho buffer **tự lớn** ⇒ 200 × 1 MB = **200 MB trên máy 256 MB** ⇒ OOM killer. Đây là **DoS bằng đúng một kết nối rẻ tiền**, không cần lưu lượng lớn.

**③ (c) Bẫy mới của length-prefix:**
```c
uint32_t len = ntohl(hdr_len);   // = 0xFFFFFFFF
char *buf = malloc(len);          // ❌ xin 4 GB trên máy 256 MB
read_full(fd, buf, len);
```
`malloc` thất bại ⇒ không kiểm `NULL` thì crash ngay; bật overcommit thì **được cấp rồi chết lúc chạm**, kéo OOM killer giết cả process khác. **Kẻ tấn công chỉ cần gửi 4 byte.** Luật: **so `len` với trần TRƯỚC khi cấp phát**. Bẫy phụ: **endianness** của trường độ dài phải là quy ước rõ (`ntohl`).

> **TCP chỉ hứa đủ byte + đúng thứ tự, KHÔNG hứa đúng lô** ⇒ mọi protocol trên TCP phải **tự framing**. — [tcp-ip.md §6](../../../13-networking/tcp-ip.md)

**Chốt:** *"Framing nào cũng phải có **trần**. Delimiter không trần = quét vô hạn; length-prefix không trần = cấp phát vô hạn. Cùng một lỗ hổng, hai bộ mặt."*

**Lần sau sẽ hỏi:** ép **viết code** (luật ⑤) — *"viết hàm `read_message(fd, buf, max)` xử lý đủ: short read, thiếu delimiter, vượt trần, `EINTR`, EOF."* Đã chẩn đoán hai lần rồi, lần này phải ra được mã sửa.

**Ôn:** [tcp-ip.md §6](../../../13-networking/tcp-ip.md) · bank [LNX-005](../bank/linux-sysprog.md)
</details>

---

### Câu 7 · LNX-039 🆕 · 🟠 · **3/4**

Gateway nhận từ nhiều đầu đọc rồi đẩy lên uplink, event loop `epoll`, mỗi kết nối có hàng đợi gửi ra:

```c
void on_readable(int in_fd) {
    ssize_t n = read(in_fd, buf, sizeof buf);
    if (n > 0) queue_push(&uplink_queue, buf, n);   // xếp vào hàng đợi userspace
}
void on_writable(int uplink_fd) {
    // lấy từ uplink_queue ra, write() ra uplink
}
```

Đầu vào ~**2 MB/s**, uplink (4G sóng yếu) chỉ tiêu thụ **500 KB/s**. Sau vài phút process bị **OOM killer** giết. Thiết bị **256 MB RAM**.

**(a)** Gọi tên vấn đề + cơ chế — vì sao RAM tăng tới chết chứ không tự cân bằng? **(b)** TCP **vốn đã có** cửa sổ nhận để điều tiết — vì sao nó không cứu được? Kiến trúc trên **vô hiệu hoá** nó ở chỗ nào? **(c)** Hai hướng xử lý khác nhau về bản chất + mỗi hướng **hy sinh** gì. Chọn hướng nào cho sản phẩm đọc mã vạch?

**🔁 Follow-up:** (d1) **hành động cụ thể nào** trong code làm cửa sổ nhận không bao giờ đóng? Không làm nó thì kernel sẽ tự làm gì? (d2) *"đẩy việc lên trên"* dịch sang code trong event loop là gì? Đầu đọc bên kia **cảm nhận** ra sao; nếu nó cũng đầy thì sao?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ *"queue phình liên tục do phía tiêu thụ không kịp, queue không giới hạn số phần tử push vào."*
- (b) ✅ *"vô hiệu hoá ở phần queue — tự push/pop theo chủ ý."* Đúng bản chất, còn thô.
- (c) ✅✅ hai hướng đúng: ① **ring buffer đè cái cũ**, giữ dữ liệu mới nhất — *"dữ liệu sẽ bị thất thoát"*; ② **queue giới hạn / không đè**, *"máy đọc mã vạch không được phép mất dữ liệu, chỉ xử lý trong khả năng, đọc không kịp thì đẩy việc lên trên"*. **Chọn ②.**
- (d1) ❌ *"tôi đoán việc dùng 2 hàm và copy biến fd vào sử dụng là nguyên nhân"*.
- (d2) 🟡 mô tả `EAGAIN` của **hàng đợi của mình** trả ngược lên lớp trên — đúng ở tầng API nội bộ, **bỏ mất tầng kernel**.

**✅ Được — (a)(b)(c) là phần mạnh nhất cả phiên.** Đặc biệt (c): nêu hai hướng **khác nhau về bản chất** (mất dữ liệu vs chặn nguồn), nói được cái hy sinh, và **chọn đúng theo đặc thù sản phẩm**. Đó là lập luận của kỹ sư có kinh nghiệm sản phẩm, không phải học thuộc.

**❌ Vì sao chưa 4:** hụt đúng câu chốt cơ chế.

**(d1) Hành động đó chính là `read()` của bạn.** Bạn hút sạch buffer nhận của kernel rồi cất sang bộ nhớ của mình ⇒ kernel **luôn thấy buffer trống** ⇒ quảng cáo cửa sổ đầy ⇒ bên gửi cứ gửi hết tốc lực.

> **TCP flow control vẫn hoạt động hoàn hảo — bạn chỉ vừa dời chỗ tắc từ kernel (có kiểm soát, có giới hạn cứng) sang heap của mình (không giới hạn).**

Không gọi `read()` thì kernel để dữ liệu tồn trong buffer nhận, buffer đầy ⇒ **cửa sổ co về 0** ⇒ bên gửi tự dừng. Đó là backpressure **có sẵn, miễn phí** — chỉ cần đừng phá nó.

**(d2)** Trong event loop: **`epoll_ctl(EPOLL_CTL_MOD)` gỡ `EPOLLIN` khỏi fd nguồn**, gắn lại khi hàng đợi rút xuống dưới ngưỡng thấp (**hysteresis**, kẻo bật/tắt liên tục). Đầu đọc bên kia cảm nhận bằng `write()` chậm dần rồi `EAGAIN`/blocking. Nếu nó **cũng đầy** thì áp lực tiếp tục lùi về tận sensor — đó là hệ thống **thiết kế đúng**: chỗ tắc hiện ra ở nơi **có thể xử lý** (bỏ frame ở nguồn, nơi biết frame nào bỏ được).

> ⇒ Đặt **trần cho buffer gửi mỗi kết nối**; vượt trần thì **ngừng đọc** từ nguồn (**bỏ `EPOLLIN` khỏi fd nguồn**) để đẩy áp lực ngược về phía gửi — hoặc ngắt kết nối. — [io-multiplexing.md §8⑤](../../../04-linux-system-programming/io-multiplexing.md)

**Chốt:** *"Mọi buffer userspace không có trần đều là một quả bom hẹn giờ. TCP đã cho sẵn backpressure — `read()` vô điều kiện chính là hành động vứt nó đi."*

**Lần sau sẽ hỏi:** *"gắn lại `EPOLLIN` ngay khi queue vơi một byte thì bị gì?"* (bật/tắt liên tục, syscall storm ⇒ hysteresis hai ngưỡng).

**Ôn:** [io-multiplexing.md §8⑤](../../../04-linux-system-programming/io-multiplexing.md) · bank [LNX-039](../bank/linux-sysprog.md)
</details>

---

### Câu 8 · LNX-024 · 🟠 · **0/4** ⚠️ *(mới → vào sổ yếu; kèm nợ tài liệu)*

Thiết bị chạy firmware của bạn + **hai app của đối tác**. Agent đối tác rò bộ nhớ, RAM cạn, **OOM killer giết nhầm daemon chính của bạn**. Khách yêu cầu: *"app đối tác hỏng thì hỏng một mình nó."*

**(a)** Dùng cơ chế nào của kernel Linux? Mỗi cơ chế giải quyết **phần nào** của yêu cầu? **(b)** Đối tác còn ghi log đầy `/tmp` và mở cổng TCP trùng cổng của bạn — cùng nhóm cơ chế đó xử lý ra sao?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ❌ *"chưa rõ, tôi chưa thấy phần này ở tài liệu nào cho buổi hôm nay."*

**⚖️ Phán định:** câu **có** trong pool §📍 hôm nay (`LNX-013…026`) nên hợp lệ và điểm là thật. Nhưng **repo không có tài liệu nào dạy nội dung này** — LNX-024 trỏ `→ ipc-linux.md`, file đó chỉ chứa một chữ "namespace" và là *abstract namespace của Unix socket*, chuyện khác hẳn. **Lỗ hổng tài liệu trước, lỗ hổng của ứng viên sau.**

**Đáp án đầy đủ — hai cơ chế, hai việc khác nhau:**

| | **Namespaces** | **cgroups** |
|---|---|---|
| Giải quyết | **cô lập tầm nhìn** — mỗi nhóm process thấy một "vũ trụ" riêng | **giới hạn & hạch toán tài nguyên** |
| Loại | pid, mount, net, uts, ipc, user | memory, cpu, io, pids |
| Với (a) | app đối tác không **thấy**, không **giết** được process của bạn | **memory limit** ⇒ OOM **cục bộ trong nhóm đó**: kernel giết đúng app đối tác, không đụng daemon của bạn |
| Với (b) | **mount ns** ⇒ `/tmp` riêng, đầy cũng đầy của nó · **net ns** ⇒ stack mạng riêng, cổng không đụng nhau | io limit ⇒ ghi log không làm nghẽn flash |

**Câu chốt:** container = **namespaces (cô lập) + cgroups (giới hạn) + rootfs riêng** — vẫn **chung kernel host**, không phải máy ảo.

**Trên embedded, thứ gặp thật thường không phải Docker mà là systemd** — chính hai cơ chế này được gói lại trong unit file:
```ini
[Service]
MemoryMax=64M            # cgroup memory  -> OOM cuc bo, khong giet daemon khac
PrivateTmp=yes           # mount namespace -> /tmp rieng
RestrictAddressFamilies=AF_UNIX   # han che mang
TasksMax=32              # cgroup pids -> chong fork bomb
```

**Lần sau sẽ hỏi:** T1 lại (lần này trắng) — *"app đối tác rò RAM, làm sao để nó chết một mình?"*, rồi lên T2: *"`MemoryMax=64M` đặt rồi mà daemon của bạn vẫn chết — nghi gì?"*

**Ôn:** bank [LNX-024](../bank/linux-sysprog.md) *(đã viết lại 2026-08-15)*. ⚠️ **Repo chưa có tài liệu topic** — xem nợ ở plan.
</details>

---

### Câu 9 · LNX-015 · 🟠 · **3/4** — *sát 4*

`capture` và `decode` đẩy **frame 2 MB, 30 fps** qua **shared memory + mutex đặt trong vùng shm**. Chạy ổn định cho tới khi `decode` **crash đúng lúc đang giữ mutex**.

**(a)** Chuyện gì xảy ra với `capture`? **(b)** Đây là lớp vấn đề socket/pipe **không có** — khác biệt bản chất ở đâu? **(c)** Vẫn cần shm vì băng thông (60 MB/s) — làm sao để một bên chết không treo bên kia, và giá phải trả?

**🔁 Follow-up:** (d) *(từ chính câu hỏi ngược của ứng viên)* — `pthread_mutex_t` **về mặt vật lý** là gì? Hai process thao tác lên **một** thứ hay **hai bản sao**? Vậy vì sao POSIX vẫn bắt khai báo `PTHREAD_PROCESS_SHARED` — cờ đó **đổi cái gì**, quên thì hỏng ở đâu? Mặc định là gì?

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:**
- (a) ✅ *"capture lấy mutex nhưng mutex đã mất vĩnh viễn → treo vĩnh viễn."*
- (b) ✅ *"shm: tự map, tự quyết định hành vi, tự xử lý race/deadlock, tự dọn dẹp. Khác ở chỗ ta tự làm, thay vì kernel giúp (socket, pipe)."*
- (c) ✅ **robust mutex**, bên kia nhận lại khoá qua `consistent` và chạy tiếp. Giá: *"thiết kế khó, phải xử lý khôi phục phần dữ liệu dang dở."*
- **Câu hỏi ngược tự đặt** 👏: *"nếu mutex không phải process_shared thì sao? Mỗi process có bản riêng?"*
- (d) ❌ đoán **"hai bản sao"**; ❌ mặc định "không rõ".

**✅ Được — (a)(b)(c) đủ cả ba tầng.** (b) trúng bản chất: với socket/pipe, **trạng thái do kernel sở hữu** nên kernel biết một bên chết và biến nó thành sự kiện xử lý được (`EOF`, `EPIPE`). Với shm, khoá nằm trong **bộ nhớ người dùng** — không ai sở hữu, không ai dọn.

Khớp tài liệu (kết quả chạy thật có sẵn trong repo):
```
mutex thuong    lock() -> ETIMEDOUT    <-- dung lock() thuong la TREO VINH VIEN
ROBUST          lock() -> EOWNERDEAD   <-- cuu duoc, goi mutex_consistent()
                -> da khoi phuc, data = 42
```
— [ipc-linux.md §4.3](../../../04-linux-system-programming/ipc-linux.md)

**❌ Vì sao chưa 4 — (d) là T2 thật, không phải trivia:** hai process `mmap` cùng vùng ⇒ **cùng một trang vật lý** ⇒ **MỘT** đối tượng duy nhất, không phải hai bản sao.

Vậy `PTHREAD_PROCESS_SHARED` đổi gì? Nó báo cho thư viện **không được dùng các tối ưu chỉ đúng trong một process** — điển hình là futex "private", vốn được đánh khoá theo **không gian địa chỉ**, nên hai process **không khớp khoá nhau** ⇒ **mất wakeup**. Mặc định là **`PTHREAD_PROCESS_PRIVATE`**.

> ⚠️ Thiếu ① thì mutex chỉ đúng trong **một** process — hai process vẫn chạy song song vào vùng dữ liệu mà **không hề báo lỗi**. Đây là **bug im lặng**, rất khó lần. — [ipc-linux.md §4.3](../../../04-linux-system-programming/ipc-linux.md)

Bổ sung nên biết: shm **không có cơ chế báo hiệu** — thực tế luôn là *shm cho dữ liệu + `eventfd`/socket cho tín hiệu*, để cắm được vào `epoll`.

**Chốt:** *"Kernel sở hữu trạng thái ⇒ kernel dọn khi bạn chết. Bạn sở hữu trạng thái ⇒ bạn phải tự thiết kế đường chết."*

👏 **Câu hỏi tự đặt là câu hỏi hay nhất phiên** — nhìn ra "đặt mutex vào shm" có một giả định chưa nói ra rồi tự truy vào. Phản xạ senior thật. Đã thêm vào bank thành **LNX-042**.

**Lần sau sẽ hỏi:** *"vùng shm còn nguyên nhưng bạn không biết nó dở dang tới đâu — thiết kế cái gì trong header để `repair_shared_state()` làm được việc?"* (sequence number / generation counter / double buffering).

**Ôn:** [ipc-linux.md §4.3, §4.4](../../../04-linux-system-programming/ipc-linux.md) · bank [LNX-015](../bank/linux-sysprog.md), [LNX-042](../bank/linux-sysprog.md)
</details>

---

### Câu 10 · LNX-040 🆕 · 🟠 · **0/4** ⚠️ *(lỗi interviewer — repo không đỡ được)*

Server đã làm đúng mọi thứ: `SIGPIPE` `SIG_IGN`, mọi `write()` kiểm `errno`, `EPIPE` xử lý sạch. Một đầu đọc **bị rút dây mạng** — không `close()`, không FIN, không RST, không gì gửi về nữa. Server tiếp tục `write()`.

**(a)** `write()` **đầu tiên** sau khi thiết bị biến mất trả về gì? Vì sao? **(b)** Ghi tiếp — **bao lâu** server mới biết? Cơ chế nào phát hiện, báo bằng lỗi gì? **(c)** Con số đó không chấp nhận được — hai cách rút ngắn, mỗi cách ở **tầng nào** + đánh đổi.

<details><summary>Bạn trả lời gì · Nhận xét · Đáp án</summary>

**Bạn trả lời:** ❌ (a) đoán `EPIPE` *"vì ghi vào pipe không được"* — **sai**. (b)(c) **"chưa rõ"**. Kèm phản hồi *"khá nâng cao so với những gì tôi đọc"*.

**⚖️ Phán định: ✅ đồng ý hoàn toàn** — `grep -rn "keepalive\|ETIMEDOUT"` toàn repo ra 0 kết quả liên quan. **Lỗi của interviewer**: ra câu repo không có tài liệu nào đỡ. Điểm ghi nhận nhưng **trách nhiệm thuộc về repo**, đã ghi thành nợ.

**Đáp án đầy đủ (đáng nắm — đúng lớp lỗi của thiết bị cầm tay rời sóng):**

**(a) `write()` đầu tiên: THÀNH CÔNG**, trả về đủ số byte. `EPIPE` **sai** vì `EPIPE` cần một **RST bay về**, mà ở đây **không có gì bay về cả**. `write()` chỉ có nghĩa *"đã chép vào buffer gửi của kernel"*, **không** phải *"bên kia đã nhận"*. Ghi tiếp cho tới khi buffer gửi đầy ⇒ lúc đó mới `EAGAIN`/blocking.

**(b)** Kernel **retransmit theo thang tăng gấp đôi** (~200 ms, 400 ms, 800 ms…) tới hết số lần cho phép, cuối cùng `write()`/`read()` trả **`ETIMEDOUT`**. Thời gian: **~15–20 phút** với mặc định Linux. Với máy quét mã vạch là vô nghĩa — kết nối chết 15 phút mà server vẫn tưởng sống, vẫn giữ tài nguyên, vẫn định tuyến việc cho nó.

**(c) Hai tầng:**

| Tầng | Cách | Đánh đổi |
|---|---|---|
| **Kernel/socket** | `SO_KEEPALIVE` + `TCP_KEEPIDLE`/`KEEPINTVL`/`KEEPCNT` (vd 30 s + 5 s × 3 ⇒ phát hiện ~45 s) | Không phải sửa protocol; nhưng là tham số per-socket của Linux, và **chỉ phát hiện đường chết** — không biết app bên kia có treo không |
| **Application** | **Heartbeat**: hai bên ping định kỳ, không thấy N nhịp thì tự đóng | Phải sửa protocol; đổi lại **phát hiện được cả app treo mà TCP vẫn sống**, và di động qua mọi OS |

**Chốt:** *"TCP không có tín hiệu 'đối tác còn sống'. **Im lặng và khoẻ mạnh trông giống hệt nhau.** Muốn biết bên kia còn sống thì phải chủ động hỏi."*

**Lần sau sẽ hỏi:** chỉ sau khi đã viết tài liệu — khi đó hỏi T2: *"đặt keepalive 5 s cho 5.000 kết nối trên 4G tính tiền theo byte — vấn đề gì?"*

**Ôn:** ⚠️ **chưa có tài liệu** — nợ ghi ở plan. Tạm: bank [LNX-040](../bank/linux-sysprog.md)
</details>

---

## 🎯 Ba lỗ hổng ưu tiên

**① Chẩn đoán được, sửa không được — Q3(c), Q5(c), Q6(b), Q7(d).**
Đây là **mẫu hình rõ nhất của phiên**, không phải bốn sự cố rời rạc. Đọc code tìm bug rất tốt (Q3a, Q6a đều chuẩn xác), rồi tắc ngay khi câu chuyển từ *"chuyện gì đang xảy ra"* sang *"bạn làm gì bây giờ"*. Ở phỏng vấn thật, **nửa sau mới là nửa được trả lương**.
→ **Cách luyện:** mỗi lần đọc xong một mục tài liệu, tự viết ra *"mã sửa trông như thế nào"* — dòng code cụ thể (`epoll_ctl(MOD)`, `pthread_condattr_setclock`, `if (len > MAX) drop`), không phải mô tả.

**② Quyết định bằng cảm tính thay vì phép đo — Q2(e), Q4(a).**
Quyết định đúng (từ chối patch, giữ `read()`) nhưng căn cứ là *"hàng ngàn kết nối"*, *"con số rất lớn"*. Đổi thành đại lượng đo được: **tỉ lệ fd sẵn sàng / tổng fd**; **số page fault = kích thước / 4 KB**. Cùng kết luận, khác hẳn sức thuyết phục.

**③ Không chuyển ý niệm sang ngữ cảnh kế bên — Q3(c).**
Nắm chắc ET phía đọc nhưng không áp sang phía ghi dù cơ chế **giống hệt**. Khi học một cơ chế, tự hỏi *"nó còn áp vào đâu nữa?"* — đây là thứ phân biệt "thuộc" với "hiểu", và là thứ `by-level` 🟠 đo.

**Điểm mạnh cần giữ:** Q9 và Q7(c) — khi câu hỏi ở tầng **đánh đổi thiết kế sản phẩm**, ứng viên mạnh: chọn robust mutex kèm giá phải trả, chọn chặn-nguồn thay vì đè-dữ-liệu vì đặc thù máy quét. Nhiều ứng viên mid ngược lại (thuộc cơ chế nhưng không dám chọn). Phản xạ tự đặt câu hỏi ở Q9 là phản xạ senior thật.

**Quan sát về cách trả lời:** nhiều lần mở đầu bằng *"chưa rõ"* rồi nói ra thứ **gần đúng hoặc đúng** (Q4 d1/d2 rõ nhất — mô tả trúng page fault mà tự cho là không biết). Ở phỏng vấn thật, cách này khiến interviewer chấm **thấp hơn thực lực**. Nên đổi thành: *"tôi chưa chắc, nhưng tôi nghĩ theo hướng…"* rồi nói tiếp.

---

## 📌 Nợ phát sinh từ phiên (chuyển sang plan)

| # | Nợ | Mức |
|---|---|---|
| 1 | **Không có tài liệu nào về namespaces/cgroups** trong toàn repo, mà LNX-024 nằm trong pool ôn | 🔴 cao |
| 2 | **Không có tài liệu nào về phát hiện peer chết** (keepalive/heartbeat/`ETIMEDOUT` khi peer biến mất) | 🔴 cao |
| 3 | `io-multiplexing.md` thiếu mục **LT + `EPOLLOUT` thường trực = busy-loop 100% CPU** (chỉ có ET phía ghi ở §8④) | 🟡 |
| 4 | `file-io.md` nói về `mmap` đúng **1 dòng** nhưng LNX-026 trỏ vào đó — **lần thứ 3** gặp mẫu này (sau LNX-023, LNX-029) | 🟡 |
