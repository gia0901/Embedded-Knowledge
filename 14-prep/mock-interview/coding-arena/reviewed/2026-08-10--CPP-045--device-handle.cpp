// =============================================================================
// Phiên 2026-08-10 · weak-review · CPP-045 (`= delete`) → phát sinh CPP-054
// Điểm câu CPP-045: 3/4  (lý thuyết đạt, code có 1 UB)
//
// ✅ Đạt:
//   [A1] Nhận đúng: bản C++98 (private + không định nghĩa) hỏng ở giai đoạn LINK
//   [A2] Nhận đúng: gọi từ NGOÀI class → lỗi compile-time (vi phạm access control)
//   [A3] Chuyển đúng sang `= delete`, đặt ở `public` với lý do "rõ ý định"
//   [A4] Tự nhận ra thiếu destructor → mở rộng sang Rule of 3/5
//   [A5] Viết đủ cả move ctor + move assign, có `if (this != &other)` ở assign
//
// ❌ Cần vá:
//   [B1] 🔴 MOVE CTOR đọc `fd_` CHƯA khởi tạo → UB, có thể close() fd của module khác
//   [B2] 🟠 Thiếu `noexcept` trên move ops → move-only class KHÔNG dùng được trong vector
//   [B3] 🟡 `#include <bits/stdc++.h>` trong header thư viện: GCC-only, không portable,
//            và thiếu <unistd.h> cho close() (đang lọt nhờ bits/stdc++.h kéo vào)
//   [B4] 🟡 `int fd_;` không có default member initializer → mầm của [B1]
//
// Biên dịch kiểm chứng: g++ -std=c++17 -Wall -Wextra   (bản sửa: sạch)
// =============================================================================


// =============================================================================
// PHẦN 1 — BẢN ỨNG VIÊN NỘP (giữ nguyên từng dòng, chỉ chèn comment review)
// =============================================================================
#if 0   // không biên dịch phần này — chứa UB, giữ để đối chiếu

#pragma once
#include <bits/stdc++.h>          // ❌ [B3] header nội bộ libstdc++: clang/musl không có.
                                  //         Kéo toàn bộ STL vào MỌI translation unit → build time.
                                  //         Đúng ra: #include <unistd.h> + <utility>

class DeviceHandle {
public:
    explicit DeviceHandle(int fd) : fd_(fd) {}   // ✅ [A3] explicit — đúng, ctor 1 đối số

    ~DeviceHandle() {                            // ✅ [A4] có dtor — đúng, đây là gốc Rule of 3/5
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;                            // (thừa nhưng vô hại: object sắp biến mất)
        }
    }

    void dump() const {
        //DeviceHandle copy = *this;   // (3)     // ✅ dòng gốc của đề — nay đã bị = delete chặn
    }

    DeviceHandle(const DeviceHandle&)            = delete;  // ✅ [A3]
    DeviceHandle& operator=(const DeviceHandle&) = delete;  // ✅ [A3]

    // ---------------------------------------------------------------------
    // ❌ [B1] 🔴 UB Ở ĐÂY — lỗi nghiêm trọng nhất của bài
    // ---------------------------------------------------------------------
    DeviceHandle(DeviceHandle&& other) {         // ❌ [B2] thiếu `noexcept`
        if (fd_ != -1) {                         // ❌ [B1] ĐỌC fd_ CỦA OBJECT ĐANG ĐƯỢC XÂY.
            close(fd_);                          //     `DeviceHandle b = std::move(a);`
        }                                        //     → ctor DeviceHandle(int) KHÔNG chạy cho b.
                                                 //     Không ai gán b.fd_ trước dòng if này.
                                                 //     fd_ = rác trên stack. Nếu rác != -1
                                                 //     → close() một fd CỦA MODULE KHÁC.
                                                 //     Triệu chứng: socket/file ở nơi không liên
                                                 //     quan đột nhiên chết, backtrace vô hại.
        fd_ = other.fd_;
        other.fd_ = -1;                          // ✅ vô hiệu hoá nguồn — phần này đúng
    }

    DeviceHandle& operator=(DeviceHandle&& other) {  // ❌ [B2] thiếu `noexcept`
        if (this != &other) {                        // ✅ [A5] self-assignment guard — đúng
            if (fd_ != -1) {                         // ✅ ĐÚNG ở ĐÂY: object đã tồn tại,
                close(fd_);                          //    fd_ hợp lệ, PHẢI dọn kẻo leak fd
            }
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

private:
    int fd_;                                     // ❌ [B4] không có initializer → mầm của [B1]
};

#endif  // PHẦN 1


// =============================================================================
// PHẦN 2 — BẢN SỬA
// Giữ nguyên mọi quyết định thiết kế hợp lý của ứng viên:
//   - explicit ctor · `= delete` đặt public · move-only · self-assign guard ở move assign
// Chỉ vá đúng 4 lỗi đã đánh nhãn [B1]–[B4].
// =============================================================================

#include <unistd.h>     // ✅ [B3] close() — khai báo đúng nguồn, không mượn bits/stdc++.h
#include <utility>      // ✅ [B3] std::exchange

class DeviceHandle {
public:
    explicit DeviceHandle(int fd) noexcept : fd_(fd) {}

    ~DeviceHandle() {
        if (fd_ != -1) ::close(fd_);
    }

    DeviceHandle(const DeviceHandle&)            = delete;
    DeviceHandle& operator=(const DeviceHandle&) = delete;

    // ✅ [B1] Move ctor: object đích CHƯA tồn tại → KHÔNG có gì để dọn.
    //         Chỉ cướp fd rồi vô hiệu hoá nguồn, làm luôn ở member-init-list
    //         nên không tồn tại khoảnh khắc nào fd_ chưa khởi tạo.
    // ✅ [B2] noexcept
    DeviceHandle(DeviceHandle&& o) noexcept
        : fd_(std::exchange(o.fd_, -1)) {}

    // ✅ Move assign: object đích ĐÃ tồn tại và đang giữ fd → PHẢI dọn trước.
    //    Đây chính là chỗ khác nhau giữa hai hàm (xem bảng cuối file).
    DeviceHandle& operator=(DeviceHandle&& o) noexcept {
        if (this != &o) {
            if (fd_ != -1) ::close(fd_);
            fd_ = std::exchange(o.fd_, -1);
        }
        return *this;
    }

    int get() const noexcept { return fd_; }

    // release(): nhả quyền sở hữu mà không đóng — API thường cần, giống unique_ptr::release
    int release() noexcept { return std::exchange(fd_, -1); }

private:
    int fd_ = -1;   // ✅ [B4] default member initializer — diệt tận gốc lớp bug [B1]
};

// =============================================================================
// PHẦN 3 — TEST các ca biên đã bàn trong phiên
// =============================================================================
#include <cassert>
#include <cstdio>
#include <vector>

int main() {
    // Ca 1 — move ctor: đích CHƯA tồn tại. Bản gốc đọc fd_ rác ở đây (UB).
    // Dùng fd hợp lệ (dup của stderr) để nếu close() nhầm thì lộ ra ngay.
    {
        DeviceHandle a(::dup(2));
        int raw = a.get();
        DeviceHandle b = std::move(a);
        assert(b.get() == raw && "move ctor phải cướp đúng fd");
        assert(a.get() == -1  && "nguồn phải bị vô hiệu hoá");
    }

    // Ca 2 — move assign: đích ĐÃ tồn tại và đang giữ fd → phải dọn, không leak.
    {
        DeviceHandle dst(::dup(2));
        DeviceHandle src(::dup(2));
        int raw = src.get();
        dst = std::move(src);
        assert(dst.get() == raw);
        assert(src.get() == -1);
    }

    // Ca 3 — self-move-assign không được đóng fd của chính mình.
    {
        DeviceHandle h(::dup(2));
        int raw = h.get();
        DeviceHandle& alias = h;
        h = std::move(alias);
        assert(h.get() == raw && "self-assign guard phải giữ nguyên fd");
    }

    // Ca 4 — move-only trong vector: CHỈ compile được nhờ move ctor noexcept [B2].
    //         Bỏ noexcept → vector fallback sang copy → copy đã = delete → lỗi compile.
    {
        std::vector<DeviceHandle> v;
        v.reserve(1);
        v.emplace_back(::dup(2));
        v.emplace_back(::dup(2));   // realloc ở đây: cần move noexcept
        v.emplace_back(::dup(2));
        assert(v.size() == 3);
        for (const auto& h : v) assert(h.get() != -1);
    }

    // Ca 5 — release(): nhả quyền sở hữu mà không đóng.
    {
        DeviceHandle h(::dup(2));
        int raw = h.release();
        assert(h.get() == -1);
        assert(::close(raw) == 0 && "fd phải còn mở sau release()");
    }

    std::puts("✅ Tất cả ca biên PASS — move ctor/assign, self-assign, vector, release");
    return 0;
}

// =============================================================================
// BẢNG CHỐT — move ctor vs move assign (học thuộc bảng này là đủ)
//
//                        | Move CONSTRUCTOR          | Move ASSIGNMENT
//   ---------------------+---------------------------+---------------------------
//   Object đích          | CHƯA tồn tại (đang xây)   | ĐÃ tồn tại, đang giữ tài nguyên
//   Member lúc vào hàm   | RÁC (kiểu built-in)       | Giá trị hợp lệ
//   Dọn tài nguyên cũ?   | ❌ Không có gì để dọn      | ✅ BẮT BUỘC, không thì leak
//   Cần this != &other?  | ❌ Không thể tự-move-ctor  | ✅ Có
//
// Vì sao `noexcept` không phải trang trí:
//   std::vector<DeviceHandle> khi realloc chỉ dùng move nếu move ctor là noexcept
//   (strong exception guarantee). Không noexcept → nó fallback sang COPY, mà copy
//   đã `= delete` → KHÔNG COMPILE. Move-only + quên noexcept = vô dụng trong container.
//
// Chốt một câu: "Move ctor xây từ số 0 nên không dọn gì; move assign phải dọn tài
//                nguyên đang giữ. Cả hai noexcept, cả hai để nguồn ở trạng thái huỷ được."
//
// Bank: CPP-045 (= delete) · CPP-054 (move ctor vs move assign) — 14-prep/mock-interview/bank/cpp.md
// =============================================================================
