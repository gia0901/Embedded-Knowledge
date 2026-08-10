// =============================================================================
// Phiên 2026-08-10 · weak-review · CPP-032 (`explicit`) → phát sinh CPP-053
// Điểm: 2/4  → câu GIỮ LẠI trong weak-register, reset bộ đếm
//
// ✅ Đạt:
//   [A1] Nhận đúng `send(1024)` và `send("hello")` là implicit conversion qua ctor 1 đối số
//   [A2] Nêu đúng vì sao nguy hiểm: send("hello") TRÔNG NHƯ gửi chuỗi, thực chất dựng
//        Buffer tạm; send(1024) cấp phát 1KB rỗng rồi gửi đi
//
// ❌ Cần vá:
//   [B1] 🔴 Không biết `explicit` áp cho CẢ conversion operator → safe-bool problem.
//            Ứng viên trả lời "không rõ" ý (b) — mà đây là NỬA CÒN LẠI của chính câu bank.
//   [B2] 🔴 Ứng viên tự viết trúng một ca lỗi ngay trong file mà không nhận ra:
//            dòng `std::cout << "sent: " << buf` — Buffer KHÔNG có operator<<, dòng này
//            compile được chỉ vì buf ngầm hoá thành bool → in ra "1".
//   [B3] 🟡 Tiêu chí bỏ `explicit` chưa chặt: nói "vì dùng tự nhiên, đúng mục đích".
//   [B4] 🟡 `buffer_.reserve(size)` không đổi size() → operator bool luôn false với
//            ctor size_t (bug logic phụ, lộ ra khi chạy: in "buf is empty").
//
// Biên dịch kiểm chứng (đã chạy thật, không phải phỏng đoán):
//   g++ 11.4 -std=c++17 -Wall -Wextra  →  0 warning trên CẢ 5 biểu thức vô nghĩa
//   clang    -Wall -Wextra             →  bắt được ĐÚNG 1/5 (`a > 3`, qua
//                                          -Wtautological-constant-out-of-range-compare)
// ⚠️ Đây mới là điều đáng sợ: 4/5 ca lọt qua MỌI compiler không một tiếng động.
//    Không có công cụ nào cứu — chỉ `explicit` mới chặn được. Xem PHẦN 2.
// =============================================================================

#include <iostream>
#include <string>

// =============================================================================
// PHẦN 1 — BẢN ỨNG VIÊN NỘP (giữ nguyên từng dòng, chỉ chèn comment review)
// =============================================================================
namespace submitted {

class Buffer {
public:
    Buffer(size_t size) {                     // ❌ [B1] thiếu explicit → send(1024) lọt
        buffer_.reserve(size);                // ❌ [B4] reserve KHÔNG đổi size() → bool luôn false
        std::cout << "ctor buff size: " << size << std::endl;
    }

    Buffer(const char* data) : buffer_(data) { // ❌ [B1] thiếu explicit → send("hello") lọt
        std::cout << "copy string: " << data << std::endl;
    }

    operator bool() const {                   // ❌ [B1] 🔴 thiếu explicit — SAFE-BOOL PROBLEM
        return buffer_.size() > 0;            //     bool là integral type ⟹ Buffer → bool → int
    }                                         //     ⟹ Buffer tham gia MỌI phép toán số học

    size_t size() const { return buffer_.size(); }

private:
    std::string buffer_;
};

void send(const Buffer& buf) {
    if (buf)                                  // ✅ ý ĐỊNH đúng — và sẽ vẫn chạy sau khi sửa
                                              //    (contextual conversion to bool)
        std::cout << "sent: " << buf << std::endl;
        // ❌ [B2] 🔴 DÒNG NÀY LẼ RA PHẢI LỖI COMPILE.
        //     Buffer không có operator<<. Nó chạy được vì buf ngầm hoá thành bool
        //     → in ra "1" thay vì nội dung buffer.
        //     Ứng viên viết dòng này hoàn toàn tự nhiên mà không nhận ra
        //     → đúng cách lớp bug này lọt vào codebase thật.
    else
        std::cout << "buf is empty" << std::endl;
}

} // namespace submitted

// =============================================================================
// PHẦN 2 — CHỨNG MINH: những gì lọt qua compiler khi thiếu `explicit`
// gcc 11.4 -Wall -Wextra: 0/5 bị bắt.  clang -Wall -Wextra: 1/5 bị bắt (`a > 3`).
// ⟹ 4 ca còn lại im lặng tuyệt đối trên cả hai compiler.
// =============================================================================
static void demo_safe_bool_problem() {
    using submitted::Buffer;
    Buffer a("hello");
    Buffer b(1024);

    Buffer c("world");

    std::cout << "\n--- Những biểu thức VÔ NGHĨA mà compiler chấp nhận ---\n";
    std::cout << "cout << a  -> " << a         << "   // in 1, dù Buffer không có operator<<\n";
    std::cout << "a == c     -> " << (a == c)  << "   // \"hello\"==\"world\" ra TRUE:"
                                                  " so sánh 2 cái bool, không phải nội dung\n";
    std::cout << "a == b     -> " << (a == b)  << "   // và kết quả đổi theo bool, hoàn toàn vô nghĩa\n";
    std::cout << "a + 1      -> " << (a + 1)   << "   // cộng một Buffer với số\n";
    std::cout << "a > 3      -> " << (a > 3)   << "   // so sánh Buffer với số\n";
    int n = a;
    std::cout << "int n = a  -> " << n         << "   // gán Buffer vào int\n";
}

// =============================================================================
// PHẦN 3 — BẢN SỬA
// Giữ nguyên thiết kế của ứng viên (Buffer bọc std::string, có operator bool,
// API send nhận const&). Chỉ vá 4 lỗi đã đánh nhãn.
// =============================================================================
namespace fixed {

class Buffer {
public:
    // ✅ [B1] explicit BẮT BUỘC: số 1024 KHÔNG PHẢI một buffer — nó *cấp phát* 1024 byte.
    //         Conversion này "làm một việc gì đó" ⟹ explicit.
    // ✅ [B4] resize (cấp + đặt size) thay vì reserve (chỉ cấp capacity)
    explicit Buffer(size_t size) { buffer_.resize(size); }

    // ✅ [B1][B3] CŨNG explicit. Nghe thì tự nhiên, nhưng nó COPY toàn bộ dữ liệu —
    //             send("hello") giấu một lần cấp phát + copy trong hot path.
    //             Cần tiện thì thêm hàm CÓ TÊN RÕ (xem from() bên dưới).
    explicit Buffer(const char* data) : buffer_(data) {}

    static Buffer from(const char* data) { return Buffer(data); }   // ý đồ hiện rõ tại call site

    // ✅ [B1] 🔴 CHỖ SỬA QUAN TRỌNG NHẤT.
    //    `if (buf)` VẪN CHẠY nhờ CONTEXTUAL CONVERSION TO BOOL (C++11):
    //    ở vị trí ngôn ngữ đằng nào cũng cần bool, conversion operator explicit
    //    vẫn được gọi tự động. Danh sách vị trí đó:
    //      if · while · for(;;) · !x · x && y · x || y · x ? a : b · static_assert
    //    Mọi cách dùng khác (cout <<, ==, +, gán vào int) → LỖI COMPILE. Đúng ý muốn.
    //    STL đều làm vậy: unique_ptr, shared_ptr, optional, function, ifstream.
    explicit operator bool() const { return !buffer_.empty(); }

    size_t size() const { return buffer_.size(); }
    const char* data() const { return buffer_.data(); }

private:
    std::string buffer_;
};

void send(const Buffer& buf) {
    if (buf)                                     // ✅ vẫn chạy — contextual conversion
        std::cout << "sent " << buf.size() << " bytes\n";   // ✅ gọi hàm CÓ TÊN, không ngầm hoá
    else
        std::cout << "buf is empty\n";
    // std::cout << buf;   // ❌ giờ đã LỖI COMPILE — đúng như mong muốn
}

} // namespace fixed

int main() {
    demo_safe_bool_problem();

    std::cout << "\n--- Bản sửa ---\n";
    fixed::send(fixed::Buffer::from("hello"));   // ý đồ hiện rõ ngay tại chỗ gọi
    fixed::send(fixed::Buffer(1024));            // muốn cấp phát thì phải nói ra
    // fixed::send(1024);      // ❌ lỗi compile — đúng như mong muốn
    // fixed::send("hello");   // ❌ lỗi compile — đúng như mong muốn
    return 0;
}

// =============================================================================
// TIÊU CHÍ CHỐT — khi nào bỏ `explicit`  (vá [B3])
//
//   Để IMPLICIT khi conversion là "cùng một thứ, đổi cách biểu diễn"
//     — lossless, không bất ngờ, KHÔNG LÀM GÌ CẢ.
//   Đánh EXPLICIT khi conversion *làm một việc gì đó*
//     — cấp phát, mở tài nguyên, nhận quyền sở hữu, đổi ngữ nghĩa.
//
//   Đối chiếu STL:
//     std::string : implicit  cho const char*  (chỉ đổi cách biểu diễn cùng một chuỗi)
//     std::vector : EXPLICIT  cho size_t       (10 không phải một vector — nó cấp phát)
//     unique_ptr  : EXPLICIT  cho con trỏ thô  (nhận con trỏ = NHẬN QUYỀN SỞ HỮU)
//
//   Ctor 1 đối số MẶC ĐỊNH explicit; bỏ explicit là quyết định phải BIỆN MINH ĐƯỢC.
//
// Bank: CPP-032 (explicit) · CPP-053 (safe-bool / explicit operator bool)
//       → 14-prep/mock-interview/bank/cpp.md
// =============================================================================
