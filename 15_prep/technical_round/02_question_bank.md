# 📗 FILE 2 — NGÂN HÀNG CÂU HỎI PHỎNG VẤN (có lời giải)
### BSP Engineer & C++ Engineer · Mid-level · Embedded/Linux

> **Cách luyện:** Che phần "Trả lời", tự nói thành tiếng, rồi đối chiếu. Mục tiêu là *giải thích được bản chất*, không học vẹt. ⭐ = câu rất hay ra với JD của bạn.

---

## PHẦN 1 — MODERN C++

**Q1. ⭐ Khác biệt `unique_ptr`, `shared_ptr`, `weak_ptr`? Khi nào dùng cái nào?**
> `unique_ptr` sở hữu độc quyền, move-only, gần như không overhead → mặc định cho mọi owning pointer. `shared_ptr` cho sở hữu *chia sẻ thật sự* qua đếm tham chiếu atomic (tốn control block). `weak_ptr` không sở hữu, quan sát một `shared_ptr` và dùng để **phá vòng tham chiếu**. Nguyên tắc: ưu tiên `unique_ptr`; chỉ dùng `shared_ptr` khi nhiều bên thực sự cùng sở hữu.

**Q2. ⭐ `shared_ptr` có thread-safe không?**
> Bộ đếm tham chiếu là atomic nên copy/destroy `shared_ptr` từ nhiều thread an toàn. Nhưng **object được trỏ tới không được bảo vệ** — ghi đồng thời vào object vẫn cần mutex. "Control block thread-safe, payload thì không."

**Q3. ⭐ Circular reference là gì, gây hậu quả gì, fix ra sao?**
> Hai object giữ `shared_ptr` lẫn nhau → strong_count không bao giờ về 0 → **memory leak**. Fix: chiều "tham chiếu ngược" dùng `weak_ptr`, muốn truy cập thì `.lock()`.

**Q4. ⭐ `std::move` thực sự làm gì?**
> Không di chuyển gì — chỉ `static_cast` một lvalue thành rvalue reference để overload **move constructor/assignment** được chọn. Việc di chuyển thật (cướp con trỏ nội bộ, để nguồn rỗng) nằm trong move ctor.

**Q5. Rule of 0/3/5 là gì?**
> Rule of 3: cần tự định nghĩa 1 trong {dtor, copy ctor, copy assign} thì thường cần cả 3. Rule of 5: thêm move ctor + move assign. Rule of 0 (ưu tiên): không tự quản tài nguyên thô, để compiler tự sinh — dùng smart pointer/STL.

**Q6. ⭐ Phân biệt rvalue reference và universal (forwarding) reference?**
> `Widget&&` với kiểu cụ thể (không suy luận) = rvalue ref thật → dùng `std::move`. `template<class T> f(T&& x)` với `T` *được suy luận* = universal ref, nhận cả lvalue lẫn rvalue (reference collapsing) → dùng `std::forward<T>(x)` để giữ nguyên value category. Đây là nền của **perfect forwarding**.

**Q7. Perfect forwarding để giải quyết vấn đề gì?**
> Khi viết wrapper (factory, `emplace`...), ta muốn truyền tham số xuống hàm đích mà **giữ nguyên** tính lvalue/rvalue để không mất cơ hội move. `Args&&... + std::forward<Args>(args)...` làm điều đó.

**Q8. ⭐ Vì sao base class cần virtual destructor?**
> `delete` object dẫn xuất qua con trỏ base mà base dtor không virtual → chỉ phần base bị huỷ, phần dẫn xuất không được dọn → undefined behavior/leak. Base đa hình phải có virtual destructor.

**Q9. vtable/vptr hoạt động thế nào?**
> Mỗi class có virtual function → một vtable (mảng con trỏ hàm). Mỗi object có vptr trỏ vtable của class thực. Gọi virtual = tra vtable lúc runtime → đúng override. Chi phí: +1 con trỏ/object, +1 lần gián tiếp/lời gọi, không inline được.

**Q10. ⭐ `volatile` dùng khi nào? Có giúp thread-safe không?**
> Dùng khi giá trị có thể đổi ngoài luồng nhìn thấy của compiler — đọc **thanh ghi phần cứng (MMIO)** hoặc biến bị **ISR** sửa; nó cấm compiler cache/tối ưu truy cập. **Không** đảm bảo atomicity/thread-safety — cái đó là việc của `std::atomic`/mutex.

**Q11. Nêu vài kỹ thuật C++17 bạn hay dùng.**
> `std::optional` (giá trị có thể vắng, thay sentinel), `std::variant` + `std::visit` (union an toàn kiểu), `std::string_view` (cửa sổ chỉ-đọc, không copy — hợp embedded), structured bindings (`auto [a,b]=...`), `if constexpr` (rẽ nhánh compile-time trong template).

**Q12. ⭐ RAII giúp exception safety thế nào?**
> Khi exception ném, **stack unwinding** gọi destructor của mọi object đã khởi tạo → tài nguyên (mutex, file, memory) tự nhả, không leak dù thoát hàm theo đường bất thường.

---

## PHẦN 2 — SYSTEM PROGRAMMING

**Q13. ⭐ Vì sao dùng POSIX message queue cho S-Box thay vì shared memory?**
> Brightness là chuỗi giá trị theo thời gian tạo hiệu ứng *sáng/tối dần*; message queue cho ranh giới message rõ ràng, có priority, và decouple producer–consumer. Shared memory nhanh hơn nhưng phải tự lo mutex/semaphore và dễ race, trong khi throughput ở đây không phải nút thắt → message queue an toàn, dễ bảo trì hơn.

**Q14. ⭐ Message queue đầy thì xử lý sao?**
> Mặc định `mq_send` block; `O_NONBLOCK` trả `EAGAIN`. Nhưng với fade ramp chỉ **giá trị đích mới nhất** mới quan trọng, nên giải pháp đúng là **coalescing/latest-value-wins** (giữ ô giá trị đích, consumer nội suy ramp) hoặc **drop-oldest** (ghi đè giá trị cũ vì giá trị mới phản ánh môi trường chính xác hơn), kèm **rate limiting** ở producer. Tránh để block làm trễ phản ứng với ánh sáng.

**Q15. ⭐ 4 điều kiện deadlock & cách tránh?**
> Mutual exclusion, hold-and-wait, no-preemption, circular wait — đủ cả 4 mới deadlock. Phá phổ biến nhất: **lock ordering** (luôn khoá nhiều mutex theo cùng thứ tự) để phá circular wait; hoặc `try_lock`+timeout, `scoped_lock`.

**Q16. Race condition là gì?**
> ≥2 thread truy cập chung 1 dữ liệu, ít nhất 1 ghi, không đồng bộ → kết quả phụ thuộc thứ tự thực thi. Giải: mutex bảo vệ vùng tới hạn, hoặc atomic.

**Q17. ⭐ Vì sao `condition_variable.wait` phải kèm predicate (dùng while)?**
> Vì **spurious wakeup** (bị đánh thức khi điều kiện chưa đúng) và race giữa notify/wait. Kiểm tra predicate trong vòng lặp đảm bảo chỉ tiếp tục khi điều kiện thật sự đúng.

**Q18. So sánh các cơ chế IPC.**
> Pipe/FIFO: byte stream đơn giản, 1 chiều. Message queue: có ranh giới message + priority. Shared memory: nhanh nhất, tự đồng bộ. Socket: liên máy/mạng. Signal: báo sự kiện, ít dữ liệu. Chọn theo: tốc độ vs ranh giới message vs phạm vi (cùng máy/khác máy).

**Q19. Zombie vs orphan process?**
> Zombie: con chết nhưng cha chưa `wait()` → entry còn trong bảng tiến trình; dọn bằng `wait()/waitpid()`. Orphan: cha chết trước → con được init (PID 1) nhận nuôi và reap.

**Q20. `fork()` trả về gì?**
> Trả **0 cho process con**, **PID của con cho process cha**, **-1 nếu lỗi**. Không gian địa chỉ copy-on-write.

---

## PHẦN 3 — EMBEDDED LINUX (HAL · Driver · Device Tree)

**Q21. ⭐ HAL là gì, giải quyết vấn đề gì?**
> Lớp trừu tượng giữa ứng dụng và phần cứng, cho interface ổn định. Đổi chipset chỉ thay implementation HAL, tầng trên không đổi. Tôi dùng OOP: interface base + lớp dẫn xuất theo từng chipset; hỗ trợ chip mới chỉ cần lớp dẫn xuất mới → giảm chi phí porting.

**Q22. ⭐ Device Tree là gì, vì sao cần?**
> File mô tả phần cứng (.dts→.dtb) cho kernel đọc lúc boot, thay vì hard-code thông tin board trong source kernel. Một kernel image chạy nhiều board chỉ bằng .dtb khác nhau. Port board mới = cập nhật node (base address, IRQ, clock). Driver bind qua `compatible` string.

**Q23. Character device vs block device?**
> Character: truy cập theo byte/stream (UART, sensor). Block: theo khối, có buffer cache (ổ đĩa, eMMC). Đa số driver sensor/điều khiển là character device.

**Q24. `ioctl` để làm gì?**
> Kênh gửi lệnh điều khiển đặc thù thiết bị không hợp với read/write (vd set chế độ dimming, query trạng thái). Mỗi lệnh có một mã ioctl riêng.

**Q25. Vì sao kernel phải dùng `copy_from_user`/`copy_to_user`?**
> Con trỏ user-space không đáng tin (có thể không hợp lệ/độc hại). Các hàm này kiểm tra hợp lệ và xử lý page fault an toàn khi chép qua ranh giới user↔kernel; deref thẳng con trỏ user trong kernel là sai và nguy hiểm.

**Q26. ⭐ Kể quá trình kernel migration 5.10 → 6.12.**
> API kernel thay đổi/deprecate giữa các phiên bản → driver phải sửa khớp (chữ ký hàm đổi, header dời, cơ chế cũ bị gỡ). Quy trình của tôi: đọc changelog → build bắt lỗi compile → sửa từng API → test trên target → đảm bảo `dmesg` không còn warning. *(Kèm 1 ví dụ API cụ thể nếu bạn nhớ được.)*

---

## PHẦN 4 — BOOT & SECURE BOOT (BSP)

**Q27. ⭐ Mô tả luồng boot của một thiết bị Embedded Linux.**
> Boot ROM (trong SoC, gốc tin cậy) → SPL (khởi tạo DRAM) → U-Boot (khởi tạo phần cứng, nạp kernel+DTB, truyền bootargs) → Linux kernel (mount rootfs) → init/systemd (dịch vụ userspace).

**Q28. ⭐ Secure boot hoạt động thế nào?**
> Chain of trust: mỗi tầng xác minh **chữ ký** của tầng kế trước khi trao quyền chạy, bắt đầu từ Boot ROM tin cậy tuyệt đối (trong silicon). Nhà sản xuất ký bằng private key; thiết bị xác minh bằng public key nung trong eFuse/OTP. Sai chữ ký ở bất kỳ mắt xích nào → dừng boot → chống firmware giả mạo.

**Q29. ⭐ A/B partition để làm gì?**
> Hai slot hệ thống A/B. Update ghi vào slot rảnh rồi reboot sang đó; bootloader theo dõi boot thành công không, nếu fail thì **rollback** về slot cũ vẫn nguyên vẹn. Lợi: update OTA an toàn, không brick, không downtime. Đánh đổi: tốn gấp đôi dung lượng.

**Q30. Vì sao mọi package phải được ký (signed)?**
> Để thiết bị chỉ chạy phần mềm do nhà sản xuất phát hành — image không ký/sai chữ ký bị từ chối, ngăn cài firmware trái phép hoặc đã bị sửa đổi. Đây là một phần của chain of trust.

> 💡 *Nếu bị hỏi sâu hơn mức bạn biết:* "Tôi hiểu ở mức khái niệm và vận hành; phần triển khai crypto/fuse cụ thể tôi chưa làm trực tiếp nhưng sẵn sàng học." — trung thực ăn điểm hơn chém.

---

## PHẦN 5 — BUS PROTOCOLS

**Q31. ⭐ So sánh I2C, SPI, UART.**
> UART: bất đồng bộ, 2 dây, point-to-point, cần thoả thuận baud rate (console/log). I2C: đồng bộ, 2 dây dùng chung (SDA/SCL), đa thiết bị theo **địa chỉ**, tốc độ vừa, cần pull-up (sensor/EEPROM/RTC). SPI: đồng bộ, 4 dây, full-duplex, **nhanh nhất**, chọn slave bằng **CS** (flash/màn hình). Đánh đổi: số dây vs số thiết bị vs tốc độ.

**Q32. Khi nào chọn I2C, khi nào SPI?**
> I2C khi cần nối nhiều thiết bị tốc độ thấp với ít dây (tiết kiệm chân). SPI khi cần băng thông cao, full-duplex và chấp nhận tốn dây (mỗi slave thêm 1 CS).

**Q33. UART cần hai bên thống nhất gì? Sai thì sao?**
> Thống nhất **baud rate** (và data bits/parity/stop bit) vì không có dây clock. Sai baud → nhận ký tự rác.

**Q34. CS trong SPI để làm gì? CPOL/CPHA là gì?**
> CS (Chip Select) chọn slave nào đang giao tiếp — mỗi slave một dây CS. CPOL/CPHA quy định cực tính và pha của clock (SPI mode 0–3); master và slave phải cùng mode mới hiểu nhau.

---

## PHẦN 6 — DEBUGGING

**Q35. ⭐ Bạn debug một lỗi cross-layer thế nào?**
> Lần theo dữ liệu qua từng tầng: user-space (log/GDB xác nhận giá trị gửi) → ranh giới (ioctl có xuống driver không) → kernel (`dmesg`/`printk` xem driver nhận gì) → tìm gốc (vd sai offset thanh ghi/`copy_from_user`) → fix → verify lại từ app xuống driver. Không đoán, luôn xác nhận ở mỗi tầng.

**Q36. ⭐ Các lệnh GDB cốt lõi?**
> `break` đặt breakpoint, `run`, `next`/`step` (không/có vào hàm), `continue`, `finish`, `print` in biến, **`backtrace`/`bt`** in call stack, `frame N`, `info locals`, **`watch`** dừng khi biến đổi giá trị.

**Q37. Core dump là gì, phân tích thế nào?**
> Ảnh chụp bộ nhớ process lúc crash, lưu ra file. Mở post-mortem: `gdb ./prog core` rồi `bt` xem chết ở đâu. Cần binary có symbol (`-g`). Hệ thống nội bộ thường tự thu core+symbol và chạy phân tích.

**Q38. Debug binary chạy trên thiết bị target không có GDB đầy đủ thì sao?**
> Dùng **remote debug**: chạy `gdbserver :PORT ./prog` trên target, GDB đầy đủ trên host kết nối `target remote ip:port`. Debug trên phần cứng thật mà không cần cài cả GDB lên thiết bị.

**Q39. Phân biệt kernel oops và panic?**
> Oops: lỗi nghiêm trọng nhưng kernel cố tiếp tục (kill process lỗi). Panic: không thể tiếp tục, dừng hệ thống. Cả hai in stack trace ra `dmesg`.

---

## PHẦN 7 — CODING (implementation hay bị yêu cầu code tại chỗ)

> Mỗi bài: *ý tưởng → code → độ phức tạp*. Luyện viết tay được, giải thích được từng dòng.

### C1. ⭐ Đảo Linked List
**Ý tưởng:** đi qua list, đổi hướng con trỏ `next` từng node. Giữ 3 con trỏ prev/cur/next.
```cpp
struct Node { int val; Node* next; };

Node* reverse(Node* head) {
    Node* prev = nullptr;
    while (head) {
        Node* nxt = head->next;  // lưu node kế
        head->next = prev;       // đảo hướng
        prev = head;             // tiến prev
        head = nxt;              // tiến head
    }
    return prev;                 // prev là head mới
}
```
**O(n) time, O(1) space.** *(Follow-up: bản đệ quy.)*

### C2. ⭐ Phát hiện vòng trong Linked List (Floyd)
**Ý tưởng:** hai con trỏ nhanh (2 bước) / chậm (1 bước); nếu có vòng chúng sẽ gặp nhau.
```cpp
bool hasCycle(Node* head) {
    Node *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}
```
**O(n) time, O(1) space.**

### C3. ⭐ Two Sum (hash map)
```cpp
std::vector<int> twoSum(std::vector<int>& a, int target) {
    std::unordered_map<int,int> seen;       // value -> index
    for (int i = 0; i < (int)a.size(); ++i) {
        int need = target - a[i];
        if (seen.count(need)) return {seen[need], i};
        seen[a[i]] = i;
    }
    return {};
}
```
**O(n) time, O(n) space.**

### C4. Kiểm tra ngoặc cân bằng (stack)
```cpp
bool isValid(const std::string& s) {
    std::stack<char> st;
    std::unordered_map<char,char> match{{')','('},{']','['},{'}','{'}};
    for (char c : s) {
        if (c=='('||c=='['||c=='{') st.push(c);
        else {
            if (st.empty() || st.top()!=match[c]) return false;
            st.pop();
        }
    }
    return st.empty();
}
```
**O(n) time, O(n) space.**

### C5. ⭐ Tự cài `memcpy` & `strlen` (rất hay hỏi C/embedded)
```c
void* my_memcpy(void* dst, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) *d++ = *s++;      // chú ý: không xử lý vùng chồng lấn (đó là memmove)
    return dst;
}
size_t my_strlen(const char* s) {
    const char* p = s;
    while (*p) ++p;              // đếm tới ký tự '\0'
    return (size_t)(p - s);
}
```
**Follow-up hay gặp:** *khác biệt `memcpy` vs `memmove`?* → `memmove` xử lý đúng khi src/dst chồng lấn (chép lùi nếu cần); `memcpy` thì không đảm bảo.

### C6. ⭐ Ring Buffer (xem chi tiết File 1 mục G.3)
```cpp
template <typename T, size_t N>
class RingBuffer {
    T buf_[N]; size_t head_=0, tail_=0;
public:
    bool empty() const { return head_==tail_; }
    bool full()  const { return (head_+1)%N == tail_; }
    bool push(const T& v){ if(full()) return false; buf_[head_]=v; head_=(head_+1)%N; return true; }
    bool pop(T& out){ if(empty()) return false; out=buf_[tail_]; tail_=(tail_+1)%N; return true; }
};
```

### C7. Reverse string in-place / Palindrome
```cpp
void reverseStr(std::string& s) {
    int i=0, j=(int)s.size()-1;
    while (i<j) std::swap(s[i++], s[j--]);   // two pointers
}
bool isPalindrome(const std::string& s) {
    int i=0, j=(int)s.size()-1;
    while (i<j) if (s[i++]!=s[j--]) return false;
    return true;
}
```

### C8. BFS & DFS trên đồ thị
```cpp
// graph: adjacency list  vector<vector<int>>
void bfs(const std::vector<std::vector<int>>& g, int start) {
    std::vector<bool> vis(g.size(), false);
    std::queue<int> q; q.push(start); vis[start]=true;
    while (!q.empty()) {
        int u=q.front(); q.pop();
        // process u
        for (int v : g[u]) if (!vis[v]) { vis[v]=true; q.push(v); }
    }
}
void dfs(const std::vector<std::vector<int>>& g, int u, std::vector<bool>& vis) {
    vis[u]=true;
    // process u
    for (int v : g[u]) if (!vis[v]) dfs(g, v, vis);
}
```

### C9. Đếm số bit 1 (Brian Kernighan)
```cpp
int popcount(unsigned x) {
    int c=0;
    while (x) { x &= (x-1); ++c; }   // mỗi lần xoá bit 1 thấp nhất
    return c;
}
```

### C10. Kiểm tra endianness
```c
int is_little_endian() {
    unsigned int x = 1;
    return *(char*)&x == 1;   // byte thấp nằm ở địa chỉ thấp -> little-endian
}
```

---

## CÁCH TRẢ LỜI KHI BÍ (rất quan trọng)
1. **Làm rõ đề trước khi code:** hỏi input/output, ràng buộc, edge case (mảng rỗng? null? trùng?).
2. **Nói ý tưởng trước, code sau:** interviewer chấm cách tư duy, không chỉ code chạy.
3. **Nêu độ phức tạp** time & space sau khi xong.
4. **Tự test edge case:** rỗng, 1 phần tử, biên.
5. **Bí thì nói to suy nghĩ** + nêu hướng brute-force trước rồi tối ưu — im lặng là điểm trừ.