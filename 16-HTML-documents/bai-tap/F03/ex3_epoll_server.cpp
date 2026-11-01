// Bài tập F03-3: Echo server TCP một thread dùng epoll, socket non-blocking, đóng khung length-prefix.
// Server nhận khung nào thì gửi lại đúng khung đó. Phải xử lý: nhiều client cùng lúc, khung bị cắt vụn,
// partial write (client đọc chậm), và khung quá lớn (đóng kết nối đó, các client khác không bị ảnh hưởng).
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined -pthread ex3_epoll_server.cpp -o ex3 && ./ex3
#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <random>
#include <signal.h>
#include <string>
#include <string_view>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

constexpr uint32_t kMaxFrame = 1 << 20;

// Lời giải bài F03-2, dùng lại ở đây.
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
    bool feed(const uint8_t* data, size_t n, const std::function<void(std::string_view)>& on_frame) {
        if (broken_) return false;
        buf_.append((const char*)data, n);
        size_t pos = 0;
        while (buf_.size() - pos >= 4) {
            const uint8_t* p = (const uint8_t*)buf_.data() + pos;
            uint32_t len = (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3];
            if (len > max_len_) { broken_ = true; buf_.clear(); return false; }
            if (buf_.size() - pos - 4 < len) break;
            on_frame(std::string_view(buf_.data() + pos + 4, len));
            pos += 4 + len;
        }
        buf_.erase(0, pos);
        return true;
    }
private:
    uint32_t max_len_;
    std::string buf_;
    bool broken_ = false;
};

// ===================== PHẦN CỦA BẠN =====================
// listen_fd: socket đã bind + listen, đã đặt O_NONBLOCK.
// stop_fd:   eventfd; khi đọc được (readable) thì đóng mọi kết nối và return.
// Gợi ý cấu trúc: mỗi kết nối có FrameDecoder và một buffer chờ gửi (std::string out).
//  - EPOLLIN:  đọc tới khi EAGAIN; mỗi khung hoàn chỉnh -> nối encode_frame(khung) vào out; thử gửi ngay.
//  - Gửi:      send(..., MSG_NOSIGNAL) tới khi hết out hoặc EAGAIN; còn dư thì bật EPOLLOUT, hết thì tắt.
//  - Đóng:     recv trả 0, lỗi khác EAGAIN/EINTR, hoặc decoder trả false.
void run_server(int listen_fd, int stop_fd) {
    (void)listen_fd;
    uint64_t v;
    while (read(stop_fd, &v, sizeof v) < 0 && errno == EINTR) {}  // TODO: thay bằng vòng lặp epoll
}
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static bool write_all(int fd, const char* p, size_t n) {
    while (n) { ssize_t w = send(fd, p, n, MSG_NOSIGNAL); if (w <= 0) return false; p += w; n -= (size_t)w; }
    return true;
}
static bool read_all(int fd, char* p, size_t n) {
    while (n) { ssize_t r = recv(fd, p, n, 0); if (r <= 0) return false; p += r; n -= (size_t)r; }
    return true;
}
static int connect_to(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    timeval tv{5, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);   // test không treo nếu server không đọc
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (sockaddr*)&a, sizeof a) < 0) { close(fd); return -1; }
    return fd;
}
// Client gửi các khung, cắt vụn ngẫu nhiên; slow = gửi hết rồi mới đọc (ép server phải đệm và dùng EPOLLOUT).
static bool client(uint16_t port, int id, bool slow) {
    std::mt19937 rng(id);
    int fd = connect_to(port);
    if (fd < 0) return false;
    int nodelay = 1; setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof nodelay);
    std::vector<std::string> msgs;
    std::string stream;
    for (int i = 0; i < (slow ? 12 : 30); i++) {
        std::string m(slow ? 300000 + rng() % 200000 : rng() % 5000, '\0');
        for (auto& ch : m) ch = (char)rng();
        msgs.push_back(m); stream += encode_frame(m);
    }
    bool ok = true;
    auto check_replies = [&](size_t from, size_t to) {
        for (size_t i = from; i < to && ok; i++) {
            char h[4];
            if (!read_all(fd, h, 4)) { ok = false; break; }
            uint32_t len = (uint32_t)(uint8_t)h[0] << 24 | (uint32_t)(uint8_t)h[1] << 16 | (uint32_t)(uint8_t)h[2] << 8 | (uint8_t)h[3];
            std::string got(len, '\0');
            if (len != msgs[i].size() || !read_all(fd, got.data(), len) || got != msgs[i]) ok = false;
        }
    };
    if (slow) {
        std::thread w([&] { ok = write_all(fd, stream.data(), stream.size()) && ok; });
        std::this_thread::sleep_for(std::chrono::milliseconds(300));   // bắt đầu đọc muộn
        check_replies(0, msgs.size());
        w.join();
    } else {
        size_t pos = 0, done = 0, frame_end = 0, idx = 0;
        while (pos < stream.size() && ok) {
            size_t chunk = std::min<size_t>(1 + rng() % 700, stream.size() - pos);
            ok = write_all(fd, stream.data() + pos, chunk);
            pos += chunk;
            while (idx < msgs.size() && frame_end + 4 + msgs[idx].size() <= pos) { frame_end += 4 + msgs[idx].size(); idx++; }
            if (idx > done && rng() % 3 == 0) { check_replies(done, idx); done = idx; }
        }
        check_replies(done, msgs.size());
    }
    close(fd);
    return ok;
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    int lfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int one = 1; setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(lfd, (sockaddr*)&a, sizeof a);
    listen(lfd, 128);
    socklen_t al = sizeof a; getsockname(lfd, (sockaddr*)&a, &al);
    uint16_t port = ntohs(a.sin_port);
    int stop = eventfd(0, EFD_CLOEXEC);
    std::thread srv(run_server, lfd, stop);

    int fails = 0;
    // 1. Một client đơn giản
    if (!client(port, 1, false)) { std::printf("  FAIL: client đơn không nhận đúng phản hồi\n"); fails++; }
    // 2. Client gửi khung quá lớn phải bị đóng kết nối
    int bad = connect_to(port);
    std::string hdr = "\x7f\xff\xff\xff";
    write_all(bad, hdr.data(), 4);
    char x; ssize_t r = recv(bad, &x, 1, 0);
    if (r != 0) { std::printf("  FAIL: khung quá lớn mà server không đóng kết nối (recv=%zd)\n", r); fails++; }
    close(bad);
    // 3. 40 client đồng thời + 2 client đọc chậm với khung lớn
    std::vector<std::thread> ts;
    std::atomic<int> bad_clients{0};
    for (int i = 0; i < 42; i++)
        ts.emplace_back([&, i] { if (!client(port, 100 + i, i >= 40)) bad_clients++; });
    for (auto& t : ts) t.join();
    if (bad_clients) { std::printf("  FAIL: %d/42 client đồng thời nhận sai hoặc thiếu phản hồi\n", bad_clients.load()); fails++; }

    uint64_t one64 = 1; write(stop, &one64, sizeof one64);
    srv.join();
    close(lfd); close(stop);
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
