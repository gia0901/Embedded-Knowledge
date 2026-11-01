// Bài tập F03-2: Đóng khung message trên dòng byte TCP (length-prefix).
// Khung = 4 byte độ dài (big-endian, chưa tính 4 byte này) + payload.
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined ex2_framing.cpp -o ex2 && ./ex2
#include <cstdint>
#include <cstdio>
#include <functional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

// ===================== PHẦN CỦA BẠN =====================
std::string encode_frame(std::string_view payload) {
    std::string out(4 + payload.size(), '\0');
    uint32_t n = (uint32_t)payload.size();
    out[0] = (char)(n >> 24); out[1] = (char)(n >> 16); out[2] = (char)(n >> 8); out[3] = (char)n;
    out.replace(4, payload.size(), payload);
    return out;
}

class FrameDecoder {
public:
    explicit FrameDecoder(uint32_t max_len) : max_len_(max_len) {}
    // Nhận thêm n byte vừa đọc từ socket (có thể là nửa khung, nhiều khung, hoặc 1 byte).
    // Gọi on_frame cho mỗi khung hoàn chỉnh theo đúng thứ tự.
    // Trả false (và từ đó luôn trả false) nếu gặp khung dài hơn max_len: đây là lỗi giao thức, phải đóng kết nối.
    bool feed(const uint8_t* data, size_t n, const std::function<void(std::string_view)>& on_frame) {
        if (broken_) return false;
        buf_.append((const char*)data, n);
        size_t pos = 0;
        while (buf_.size() - pos >= 4) {
            const uint8_t* p = (const uint8_t*)buf_.data() + pos;
            uint32_t len = (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
            if (len > max_len_) { broken_ = true; buf_.clear(); return false; }
            if (buf_.size() - pos - 4 < len) break;          // chưa đủ khung
            on_frame(std::string_view(buf_.data() + pos + 4, len));
            pos += 4 + len;
        }
        buf_.erase(0, pos);                                    // giữ lại phần dở cho lần sau
        return true;
    }
    size_t buffered() const { return buf_.size(); }        // số byte đang giữ chờ đủ khung
private:
    uint32_t max_len_;
    std::string buf_;
    bool broken_ = false;
};
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static int fails = 0;
#define CHECK(c) do { if (!(c)) { std::printf("  FAIL dòng %d: %s\n", __LINE__, #c); fails++; } } while (0)

int main() {
    CHECK(encode_frame("abc") == std::string("\0\0\0\3abc", 7));
    CHECK(encode_frame("") == std::string(4, '\0'));

    std::mt19937 rng(2026);
    for (int round = 0; round < 300; round++) {
        std::vector<std::string> msgs;
        std::string stream;
        int k = 1 + (int)(rng() % 20);
        for (int i = 0; i < k; i++) {
            std::string m(rng() % 3 == 0 ? 0 : rng() % 3000, '\0');
            for (auto& c : m) c = (char)(rng() & 0xFF);          // có cả byte 0
            msgs.push_back(m);
            stream += encode_frame(m);
        }
        FrameDecoder dec(1 << 20);
        std::vector<std::string> got;
        size_t pos = 0;
        while (pos < stream.size()) {                             // cắt dòng byte ở vị trí ngẫu nhiên
            size_t chunk = round % 5 == 0 ? 1 : 1 + rng() % 1500;
            chunk = std::min(chunk, stream.size() - pos);
            bool ok = dec.feed((const uint8_t*)stream.data() + pos, chunk,
                               [&](std::string_view f) { got.emplace_back(f); });
            CHECK(ok);
            pos += chunk;
        }
        CHECK(got == msgs);
        CHECK(dec.buffered() == 0);
        if (fails) break;
    }
    FrameDecoder small(100);
    std::string big = encode_frame(std::string(101, 'x'));
    int called = 0;
    CHECK(!small.feed((const uint8_t*)big.data(), 4, [&](std::string_view) { called++; }));
    std::string ok = encode_frame("ok");
    CHECK(!small.feed((const uint8_t*)ok.data(), ok.size(), [&](std::string_view) { called++; }));
    CHECK(called == 0);

    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
