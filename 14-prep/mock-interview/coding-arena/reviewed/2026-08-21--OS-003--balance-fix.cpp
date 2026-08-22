// ============================================================================
//  OS-003 · balance() deadlock  ·  Phiên 2026-08-21 daily/cpp-system
//  ĐIỂM: 3/4
//
//  ✅ [A1] Chẩn đoán đúng: circular wait giữa hai lời gọi khoá ngược thứ tự.
//  ✅ [A2] Bản 1 (một mutex global) là cách sửa AN TOÀN THẬT — không phải sai.
//  ✅ [A3] Tự nêu được cái giá của bản 1 mà không cần gợi ý:
//           mất song song + phải sửa lan ra mọi chỗ đang khoá Panel::m.
//  ✅ [A4] Bản 2 dùng std::scoped_lock — đúng, compile sạch.
//  ❌ [B1] Giải thích SAI cơ chế bên trong của scoped_lock:
//           nói "có thuật toán đảm bảo thứ tự lock phù hợp".
//           Thực tế std::lock chạy TRY-AND-BACKOFF, không sắp thứ tự gì cả.
//  ❌ [B2] Gọi SAI điều kiện Coffman bị phá: nói "circular wait".
//           scoped_lock phá HOLD-AND-WAIT (không bao giờ giữ lock này mà chờ
//           lock kia). Lock ordering thủ công mới là cái phá circular wait.
//  ⚠️ [C1] Bản nộp thiếu #include <string> dù dùng std::string
//           (chạy được nhờ include bắc cầu từ <mutex> — không nên dựa vào).
// ============================================================================

// ---------------------------------------------------------------------------
//  PHẦN 1 — BẢN ỨNG VIÊN NỘP (giữ nguyên từng dòng)
// ---------------------------------------------------------------------------
#if 0
#include <mutex>

struct Panel {
    std::mutex m;
    int  brightness = 50;
    std::string name;        // [C1] thiếu #include <string>
};

// Cân bằng độ sáng: kéo hai panel về mức trung bình của cả hai
void balance(Panel& a, Panel& b) {
    std::scoped_lock lck(a.m, b.m);   // [A4] ✅ đúng
    int avg = (a.brightness + b.brightness) / 2;
    a.brightness = avg;
    b.brightness = avg;
}

int main() {

}
#endif

// ---------------------------------------------------------------------------
//  PHẦN 2 — BẢN SỬA
//  Giữ nguyên mọi quyết định thiết kế của ứng viên (scoped_lock, mutex riêng
//  từng Panel). Chỉ vá [C1] và bổ sung phần chứng minh + phần giải thích cơ
//  chế mà [B1]/[B2] còn hụt.
// ---------------------------------------------------------------------------
#include <mutex>
#include <string>        // [C1] vá: khai báo tường minh thứ mình dùng
#include <thread>
#include <vector>
#include <cstdio>

struct Panel {
    std::mutex  m;
    int         brightness = 50;
    std::string name;
};

// ✅ std::scoped_lock lấy TẤT CẢ hoặc KHÔNG GÌ.
//
//    Bên trong (std::lock): khoá một cái, try_lock phần còn lại; hễ một cái
//    trượt thì NHẢ SẠCH tất cả rồi thử lại từ đầu. Nó KHÔNG sắp thứ tự  [B1]
//    => vì không bao giờ giữ lock này trong lúc chờ lock kia,
//       thứ nó phá là HOLD-AND-WAIT, không phải circular wait.          [B2]
//
//    Hệ quả thực tế của phân biệt này:
//      - lock ordering thủ công  -> phá circular wait, nhưng đòi CẢ CODEBASE
//                                   tuân thủ cùng một thứ tự, mãi mãi.
//      - scoped_lock             -> phá hold-and-wait, đúng NGAY TẠI CHỖ DÙNG,
//                                   không cần ai khác hợp tác.
//    => vì vậy nó mở rộng thẳng sang balance3(a, b, c) mà không cần quy ước gì.
void balance(Panel& a, Panel& b) {
    std::scoped_lock lk(a.m, b.m);
    int avg = (a.brightness + b.brightness) / 2;
    a.brightness = avg;
    b.brightness = avg;
}

// Mở rộng 3 khoá — không cần thêm quy ước nào, đây là điểm mạnh của cách này.
void balance3(Panel& a, Panel& b, Panel& c) {
    std::scoped_lock lk(a.m, b.m, c.m);
    int avg = (a.brightness + b.brightness + c.brightness) / 3;
    a.brightness = b.brightness = c.brightness = avg;
}

// Hàm ở nơi khác trong service vẫn khoá mutex RIÊNG của panel.
// Đây chính là thứ bản 1 (mutex global) làm hỏng — xem [A3].
int read_brightness(Panel& p) {
    std::lock_guard<std::mutex> lk(p.m);
    return p.brightness;
}

int main() {
    Panel left{{}, 10, "left"};
    Panel right{{}, 90, "right"};
    Panel top{{}, 50, "top"};

    // Ép đúng kịch bản gây treo ở bản gốc: hai luồng gọi ngược thứ tự.
    std::vector<std::thread> ts;
    for (int i = 0; i < 4; ++i) {
        ts.emplace_back([&] { for (int k = 0; k < 20000; ++k) balance(left, right); });
        ts.emplace_back([&] { for (int k = 0; k < 20000; ++k) balance(right, left); });
    }
    ts.emplace_back([&] { for (int k = 0; k < 20000; ++k) balance3(left, right, top); });
    for (auto& t : ts) t.join();

    std::printf("khong treo. left=%d right=%d top=%d\n",
                read_brightness(left), read_brightness(right), read_brightness(top));
    return 0;
}
