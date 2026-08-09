// ═══════════════════════════════════════════════════════════════════════════
//  COD-006 · Ring Buffer                 Phiên: 2026-08-09 · coding · dsa
//  ĐIỂM: 3/4   (không chấm phần tái tạo code — chấm câu hỏi ngược + lựa chọn)
//
//  ✅ Hỏi ngược 3 câu trước khi code, có câu sizeof(T) nặng — câu của người
//     đã nghĩ tới RAM thật.
//  ✅ Suy ra đúng trục quyết định: count_ bị CẢ HAI phía ghi → cần khoá.
//  ✅ Chọn đúng chính sách đè-cũ cho dữ liệu cảm biến, lý do đúng (độ tươi).
//  ✅ Trả lời ĐÚNG câu bẫy về tính liên tục của lô (interviewer tưởng sẽ sai).
//
//  ❌ [A] KHÔNG hỏi "mấy luồng?" — câu spec số MỘT. Mutex bị đặt vào như
//         mặc định không xét, không phải lựa chọn.
//  ❌ [B] Race TOCTOU ở bản 1; sửa đúng khi được chỉ, nhưng KHÔNG tổng quát
//         hoá → empty()/full() vẫn public + không khoá.
//  ❌ [C] Bản cuối cấp phát unordered_map TRONG hot path — phá đúng lý do
//         tồn tại của ring buffer.
//  ❌ [D] push() luôn return true → caller không biết đã mất mẫu.
// ═══════════════════════════════════════════════════════════════════════════

#include <cstddef>
#include <mutex>
#include <utility>

// ###########################################################################
// ### PHẦN 1 — BẢN BẠN NỘP, giữ nguyên, chỉ chèn comment review           ###
// ###########################################################################
#if 0   // tắt biên dịch, giữ để đối chiếu

template<typename T, size_t N>
class RingBuffer {
    T buf_[N];
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
    mutex mtx_;
public:
    bool empty() const {
        return head_ == tail_ && count_ == 0;
        // ❌ [B] public + KHÔNG khoá. main() gọi `while (!rb.empty())` từ
        //    thread khác = đúng loại race bạn vừa vá ở pop(). Vá một chỗ rồi
        //    phải quét cả class xem còn chỗ nào cùng dạng.
        // ⚠️ `head_ == tail_ &&` là thừa: đã có count_ thì count_ == 0 là đủ.
    }

    bool full() const {
        return count_ == N;        // ✅ đã sửa đúng trong phiên
    }

    bool push(const T& v) {
        unordered_map<int, pair<T,T>> overriden;
        // ❌❌ [C] LỖI NẶNG NHẤT. Dựng + huỷ một hash map TRÊN HEAP mỗi lần
        //    push. Ở 1000 mẫu/giây = 1000 lần malloc/free mỗi giây, để chứa
        //    TỐI ĐA MỘT phần tử. Ring buffer sinh ra để KHÔNG cấp phát khi
        //    chạy — dòng này xoá sạch lý do dùng nó.
        //    → Đúng phải là 2 biến stack:  T old; bool dropped = false;

        {
            lock_guard<mutex> lck(mtx_);
            if (full()) {
                overriden[head_] = {buf_[head_], v};
                buf_[head_] = v;
                head_ = (head_ + 1) % N;
                tail_ = (tail_ + 1) % N;
            }
            else {
                buf_[head_] = v;
                head_ = (head_ + 1) % N;
                count_++;
            }
        }

        if (full()) {
        // ❌ [D] SAI VỊ CHỌN ĐIỀU KIỆN. Câu hỏi đúng là "tôi vừa drop chưa?",
        //    không phải "buffer có đầy không?". Nếu một pop() chen vào giữa,
        //    full() thành false và lần drop này BỊ NUỐT — mất dữ liệu im lặng,
        //    đúng thứ mà việc ghi log sinh ra để chống.
            for (auto [idx, items] : overriden)
                cout << "Override i:" << idx << ...;
            // ❌ in cout MỖI lần drop: 1000 dòng/giây khi consumer treo.
            //    Đúng là ĐẾM, xuất theo chu kỳ.
        }

        return true;
        // ❌ [D] luôn true → caller không có cách nào biết vừa mất một mẫu.
        //    Bạn đã từng sửa thành `false` rồi đổi lại — bản false cũng chưa
        //    ổn vì lẫn với ngữ nghĩa "từ chối". Cần kênh riêng, xem PHẦN 2.
    }

    bool pop(T& out) {
        lock_guard<mutex> lck(mtx_);   // ✅ [B] đã chuyển lên TRƯỚC empty() — đúng
        if (empty()) return false;
        out = move(buf_[tail_]);
        tail_ = (tail_ + 1) % N;
        count_--;
        return true;
    }
};

#endif

// ###########################################################################
// ### PHẦN 2 — BẢN SỬA. Giữ nguyên MỌI quyết định thiết kế của bạn         ###
// ###   (count_ vì T nặng · đè-cũ vì dữ liệu cảm biến · mutex)             ###
// ###   chỉ sửa 4 lỗi ở trên.                                             ###
// ###########################################################################

template <typename T, std::size_t N>
class RingBuffer {
    static_assert(N >= 1, "N phải >= 1");

    T           buf_[N];
    std::size_t head_  = 0;          // ô SẼ GHI tiếp
    std::size_t tail_  = 0;          // ô SẼ ĐỌC tiếp
    std::size_t count_ = 0;          // [A] CHỌN count_ vì T nặng (128B):
                                     //     hy sinh 1 ô = phí 128 byte.
                                     //     Cái giá: count_ bị cả hai phía ghi
                                     //     → BẮT BUỘC khoá, không lock-free được.
    std::size_t dropped_ = 0;        // ✅ [D] số mẫu đã bị đè. KHÔNG BAO GIỜ
                                     //     thiếu khi chính sách là ghi đè.

    mutable std::mutex mtx_;         // `mutable` để hàm const vẫn khoá được

    // ─── Phiên bản KHÔNG khoá, chỉ gọi khi ĐANG giữ khoá ───────────────────
    bool empty_unlocked() const { return count_ == 0; }
    bool full_unlocked()  const { return count_ == N; }

public:
    // ⚠️ [A] Bản này giả định NHIỀU luồng (vì có mutex). Nếu spec là MỘT
    //    luồng thì bỏ hết mutex đi — đừng trả tiền cho thứ không dùng.
    //    Nếu producer là ISR / ngữ cảnh không được ngủ thì mutex KHÔNG dùng
    //    được, phải sang lock-free SPSC (13-dsa/ring-buffer.md §7).
    //    ĐÂY LÀ CÂU PHẢI HỎI TRƯỚC KHI GÕ DÒNG ĐẦU TIÊN.

    struct PushResult {
        bool dropped_oldest;         // ✅ [D] caller biết ngay mình vừa mất mẫu
        std::size_t total_dropped;   //     và biết tổng từ đầu tới giờ
    };

    PushResult push(const T& v) {
        std::lock_guard<std::mutex> lk(mtx_);

        bool dropped = full_unlocked();
        if (dropped) {
            ++dropped_;              // ✅ [C][D] ĐẾM, không in, không cấp phát.
                                     //    Giá trị cũ bị bỏ luôn — nếu thật sự
                                     //    cần nội dung mẫu bị mất thì copy ra
                                     //    biến stack `T old = std::move(buf_[head_]);`
                                     //    rồi trả ra NGOÀI vùng khoá.
            tail_ = (tail_ + 1) % N; // đẩy cái cũ nhất ra
        } else {
            ++count_;
        }

        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        return { dropped, dropped_ };
    }

    bool pop(T& out) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (empty_unlocked()) return false;   // ✅ [B] check VÀ hành động cùng
                                              //     một vùng khoá — hết TOCTOU
        out   = std::move(buf_[tail_]);
        tail_ = (tail_ + 1) % N;
        --count_;
        return true;
    }

    // ✅ [B] Các hàm truy vấn giờ KHOÁ. Nhưng đọc kỹ cảnh báo dưới.
    bool        empty()   const { std::lock_guard<std::mutex> lk(mtx_); return count_ == 0; }
    bool        full()    const { std::lock_guard<std::mutex> lk(mtx_); return count_ == N; }
    std::size_t size()    const { std::lock_guard<std::mutex> lk(mtx_); return count_; }
    std::size_t dropped() const { std::lock_guard<std::mutex> lk(mtx_); return dropped_; }
};

// ═══════════════════════════════════════════════════════════════════════════
//  ⚠️ BÀI HỌC LỚN NHẤT — đọc kỹ đoạn này hơn cả code
//
//  Khoá empty()/full() KHÔNG làm chúng an toàn để RA QUYẾT ĐỊNH:
//
//      if (!rb.empty())     // đúng tại thời điểm này...
//          rb.pop(out);     // ...nhưng thread khác đã pop mất rồi
//
//  Giá trị vừa trả về đã CŨ ngay khi khoá được nhả. Đây chính là bug [B] mặc
//  áo khác. Cách đúng: KHÔNG hỏi trạng thái rồi hành động — để hành động tự
//  báo kết quả:
//
//      T out;
//      while (rb.pop(out)) { ... }     // ✅ pop tự nói "hết rồi" bằng false
//
//  Quy tắc tổng quát mang đi mọi nơi:
//      "kiểm tra trạng thái" và "hành động dựa trên nó" phải nằm TRONG CÙNG
//      MỘT vùng khoá — nếu không, thứ bạn kiểm tra chỉ là một tấm ảnh quá khứ.
//  Cùng lớp bug với: access() rồi open(), stat() rồi unlink() (TOCTOU kinh điển).
//
//  → 13-dsa/ring-buffer.md §10 (checklist) · §6 (bản mutex+condvar có close())
// ═══════════════════════════════════════════════════════════════════════════

#include <cstdio>
int main() {
    RingBuffer<int, 5> rb;
    for (int i = 1; i <= 7; ++i) {
        auto r = rb.push(i);
        if (r.dropped_oldest)
            std::printf("drop! tong so mau da mat = %zu\n", r.total_dropped);
    }
    int out;
    while (rb.pop(out)) std::printf("%d ", out);   // ✅ không hỏi empty() nữa
    std::printf("\n(da mat %zu mau)\n", rb.dropped());
    // kỳ vọng: 3 4 5 6 7   (da mat 2 mau)
}
