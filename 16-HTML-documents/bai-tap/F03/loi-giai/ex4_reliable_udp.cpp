// Bài tập F03-4: Request/response tin cậy trên UDP, theo cách PFCP làm (TS 29.244 mục 6.4).
// Mạng giả lập mất 20% gói và nhân đôi 10% gói. Bạn viết:
//   Client::request(): gắn sequence number, gửi, chờ tối đa T1, gửi lại tối đa N1 lần, bỏ qua phản hồi sai seq.
//   Server::handle_one(): nhận request; nếu (địa chỉ client, seq) đã xử lý thì gửi lại phản hồi cũ đã lưu,
//                         KHÔNG thực hiện lại thao tác; nếu chưa thì gọi apply() và lưu phản hồi.
// Thao tác "add N" không idempotent: thực hiện hai lần là sai số. Test đếm chính xác số lần apply().
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined -pthread ex4_reliable_udp.cpp -o ex4 && ./ex4
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <tuple>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <optional>
#include <poll.h>
#include <random>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

// ---- Mạng giả lập (không sửa) ----
// Mọi lần gửi phải đi qua net_send. Gói: [type 1B][seq 4B big-endian][payload]; type 1 = request, 2 = response.
enum : uint8_t { REQ = 1, RESP = 2 };
static std::mutex g_rng_mu;
static std::mt19937 g_rng(12345);
static std::atomic<long> g_sent{0}, g_dropped{0};
void net_send(int fd, const std::string& pkt, const sockaddr_in& to) {
    int r;
    { std::lock_guard<std::mutex> lk(g_rng_mu); r = (int)(g_rng() % 100); }
    g_sent++;
    if (r < 20) { g_dropped++; return; }                       // mất gói
    int copies = r >= 90 ? 2 : 1;                                // nhân đôi gói
    for (int i = 0; i < copies; i++) sendto(fd, pkt.data(), pkt.size(), 0, (const sockaddr*)&to, sizeof to);
}
static std::string make_pkt(uint8_t type, uint32_t seq, const std::string& payload) {
    std::string p(5, '\0');
    p[0] = (char)type; p[1] = (char)(seq >> 24); p[2] = (char)(seq >> 16); p[3] = (char)(seq >> 8); p[4] = (char)seq;
    return p + payload;
}
static bool parse_pkt(const char* b, size_t n, uint8_t* type, uint32_t* seq, std::string* payload) {
    if (n < 5) return false;
    *type = (uint8_t)b[0];
    *seq = (uint32_t)(uint8_t)b[1] << 24 | (uint32_t)(uint8_t)b[2] << 16 | (uint32_t)(uint8_t)b[3] << 8 | (uint8_t)b[4];
    payload->assign(b + 5, n - 5);
    return true;
}
constexpr int T1_MS = 15;   // thời gian chờ phản hồi mỗi lần
constexpr int N1 = 12;      // số lần gửi lại tối đa

// ===================== PHẦN CỦA BẠN =====================
class Client {
public:
    Client(int fd, sockaddr_in server) : fd_(fd), server_(server) {}
    // Trả payload của phản hồi, hoặc nullopt nếu sau 1 + N1 lần gửi vẫn không có phản hồi đúng seq.
    // Dùng make_pkt/parse_pkt, net_send, và poll(fd_, POLLIN, thời gian còn lại) + recvfrom.
    std::optional<std::string> request(const std::string& payload) {
        uint32_t seq = next_seq_++;
        std::string pkt = make_pkt(REQ, seq, payload);
        char buf[2048];
        for (int attempt = 0; attempt <= N1; attempt++) {
            net_send(fd_, pkt, server_);
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(T1_MS);
            for (;;) {
                int left = (int)std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();
                if (left <= 0) break;
                pollfd p{fd_, POLLIN, 0};
                if (poll(&p, 1, left) <= 0) continue;           // hết giờ (hoặc EINTR): vòng lặp tự kiểm tra lại
                ssize_t n = recvfrom(fd_, buf, sizeof buf, 0, nullptr, nullptr);
                uint8_t type; uint32_t rseq; std::string rp;
                if (n > 0 && parse_pkt(buf, (size_t)n, &type, &rseq, &rp) && type == RESP && rseq == seq)
                    return rp;                                    // phản hồi cũ hoặc trùng có seq khác: bỏ qua
            }
        }
        return std::nullopt;
    }
private:
    int fd_;
    sockaddr_in server_;
    uint32_t next_seq_ = 1;
};

class Server {
public:
    // apply thực hiện thao tác và trả payload phản hồi. Không được gọi hai lần cho cùng một request.
    Server(int fd, std::string (*apply)(const sockaddr_in&, const std::string&)) : fd_(fd), apply_(apply) {}
    // Nhận và xử lý một gói (chặn tới khi có gói).
    void handle_one() {
        char buf[2048];
        sockaddr_in from{}; socklen_t fl = sizeof from;
        ssize_t n = recvfrom(fd_, buf, sizeof buf, 0, (sockaddr*)&from, &fl);
        uint8_t type; uint32_t seq; std::string payload;
        if (n <= 0 || !parse_pkt(buf, (size_t)n, &type, &seq, &payload) || type != REQ) return;
        Key k{from.sin_addr.s_addr, from.sin_port, seq};
        auto it = cache_.find(k);
        if (it == cache_.end()) {
            std::string resp = make_pkt(RESP, seq, apply_(from, payload));
            it = cache_.emplace(k, resp).first;
            order_.push_back(k);
            if (order_.size() > kCacheMax) { cache_.erase(order_.front()); order_.erase(order_.begin()); }
        }
        net_send(fd_, it->second, from);                        // gửi (lại) phản hồi đã lưu
    }
private:
    int fd_;
    std::string (*apply_)(const sockaddr_in&, const std::string&);
    struct Key {
        uint32_t ip; uint16_t port; uint32_t seq;
        bool operator<(const Key& o) const { return std::tie(ip, port, seq) < std::tie(o.ip, o.port, o.seq); }
    };
    static constexpr size_t kCacheMax = 4096;                  // PFCP giữ phản hồi đủ lâu để phủ hết các lần gửi lại
    std::map<Key, std::string> cache_;
    std::vector<Key> order_;
};
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static std::mutex g_mu;
static std::map<uint16_t, long> g_total;    // tổng theo cổng client
static std::atomic<long> g_applied{0};
static std::string apply_add(const sockaddr_in& from, const std::string& payload) {
    long n = std::stol(payload.substr(4));    // "add N"
    std::lock_guard<std::mutex> lk(g_mu);
    g_applied++;
    return std::to_string(g_total[from.sin_port] += n);
}
static int udp_socket(sockaddr_in* bound) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(fd, (sockaddr*)&a, sizeof a);
    socklen_t l = sizeof a; getsockname(fd, (sockaddr*)&a, &l);
    if (bound) *bound = a;
    return fd;
}

int main() {
    sockaddr_in saddr;
    int sfd = udp_socket(&saddr);
    std::atomic<bool> stop{false};
    std::thread srv([&] {
        Server s(sfd, apply_add);
        while (!stop) { pollfd p{sfd, POLLIN, 0}; if (poll(&p, 1, 50) > 0) s.handle_one(); }
    });
    const int C = 4, R = 150;
    std::atomic<int> bad{0}, lost{0};
    std::vector<std::thread> cs;
    for (int c = 0; c < C; c++) cs.emplace_back([&] {
        int fd = udp_socket(nullptr);
        Client cl(fd, saddr);
        for (int i = 1; i <= R; i++) {
            auto r = cl.request("add 1");
            if (!r) { lost++; continue; }
            if (*r != std::to_string(i)) bad++;               // tổng phải tăng đúng 1 mỗi request
        }
        close(fd);
    });
    for (auto& t : cs) t.join();
    stop = true; srv.join(); close(sfd);

    int fails = 0;
    std::printf("gửi %ld gói, mạng làm mất %ld; apply() chạy %ld lần cho %d request\n",
                g_sent.load(), g_dropped.load(), g_applied.load(), C * R);
    if (lost) { std::printf("  FAIL: %d request không nhận được phản hồi (thiếu gửi lại?)\n", lost.load()); fails++; }
    if (bad) { std::printf("  FAIL: %d phản hồi sai giá trị (phản hồi cũ bị nhận nhầm, hoặc thao tác chạy hai lần)\n", bad.load()); fails++; }
    if (g_applied != C * R) { std::printf("  FAIL: apply() chạy %ld lần, đúng ra là %d (thiếu phát hiện request lặp?)\n", g_applied.load(), C * R); fails++; }
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
