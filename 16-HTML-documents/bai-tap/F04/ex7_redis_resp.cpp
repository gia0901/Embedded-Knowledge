// Bài tập F04-7: Client Redis tối giản viết thẳng trên socket TCP, rồi dùng nó để trừ quota an toàn
// khi nhiều client cùng sửa (khóa lạc quan bằng WATCH / MULTI / EXEC).
// Cần redis-server trong PATH (Ubuntu: sudo apt install redis-server; hoặc chạy bằng Docker và sửa start_redis).
// Test tự khởi động một redis-server tạm trên cổng ngẫu nhiên, không lưu đĩa.
// Biên dịch: g++ -std=c++20 -g -fsanitize=address,undefined -pthread ex7_redis_resp.cpp -o ex7 && ./ex7
//
// Giao thức RESP2 (https://redis.io/docs/latest/develop/reference/protocol-spec/):
//   Lệnh gửi đi là mảng bulk string:  *<số phần tử>\r\n  rồi mỗi phần tử  $<số byte>\r\n<dữ liệu>\r\n
//   Phản hồi:  +OK\r\n (simple string)   -ERR ...\r\n (lỗi)   :42\r\n (số nguyên)
//              $5\r\nhello\r\n (bulk string)   $-1\r\n (null)   *2\r\n...\r\n (mảng, lồng nhau được)   *-1\r\n (mảng null)
#include <arpa/inet.h>
#include <atomic>
#include <csignal>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <random>
#include <string>
#include <sys/socket.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

struct Resp {
    enum Type { SIMPLE, ERROR, INTEGER, BULK, NIL, ARRAY } type = NIL;
    std::string str;                 // SIMPLE, ERROR, BULK
    long long num = 0;               // INTEGER
    std::vector<Resp> elems;         // ARRAY
};

// ===================== PHẦN CỦA BẠN =====================
// Mã hóa một lệnh, ví dụ {"SET", "k", "v"} -> "*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$1\r\nv\r\n". Dữ liệu có thể chứa \r\n hoặc byte 0.
std::string resp_encode(const std::vector<std::string>& args) {
    (void)args;
    return "";  // TODO
}

class Redis {
public:
    explicit Redis(int fd) : fd_(fd) {}
    ~Redis() { close(fd_); }
    // Gửi lệnh rồi đọc đúng MỘT phản hồi. Trả Resp ERROR với str = "IO" nếu kết nối hỏng.
    Resp command(const std::vector<std::string>& args) {
        (void)args;
        return io_error();  // TODO
    }
private:
    int fd_;
    std::string buf_;                // byte đã nhận nhưng chưa dùng (TCP là dòng byte: phản hồi có thể đến từng mẩu)
    size_t pos_ = 0;
    static Resp io_error() { Resp r; r.type = Resp::ERROR; r.str = "IO"; return r; }
};

// Trừ amount khỏi trường "balance" của hash key nếu đủ. Trả true nếu đã trừ, false nếu không đủ.
// Nhiều client chạy đồng thời: dùng WATCH key, HGET, (không đủ thì UNWATCH, trả false),
// MULTI, HSET key balance <mới>, EXEC. EXEC trả mảng null nghĩa là key bị đổi giữa chừng: làm lại từ đầu.
bool deduct(Redis& r, const std::string& key, long long amount) {
    Resp b = r.command({"HGET", key, "balance"});                      // TODO: chưa an toàn khi đồng thời
    long long bal = b.type == Resp::BULK ? std::stoll(b.str) : 0;
    if (bal < amount) return false;
    r.command({"HSET", key, "balance", std::to_string(bal - amount)});
    return true;
}
// ================== HẾT PHẦN CỦA BẠN ====================

// ========================= TEST =========================
static int connect_to(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_port = htons((uint16_t)port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (sockaddr*)&a, sizeof a) < 0) { close(fd); return -1; }
    int one = 1; setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    timeval tv{5, 0}; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    return fd;
}
static pid_t start_redis(int port) {
    pid_t pid = fork();
    if (pid == 0) {
        std::string p = std::to_string(port);
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) { dup2(devnull, 1); dup2(devnull, 2); }
        execlp("redis-server", "redis-server", "--port", p.c_str(), "--save", "", "--appendonly", "no",
               "--bind", "127.0.0.1", "--loglevel", "warning", (char*)nullptr);
        _exit(127);
    }
    return pid;
}
int main() {
    signal(SIGPIPE, SIG_IGN);
    int fails = 0;
    if (resp_encode({"SET", "k", "a\r\nb"}) != "*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$4\r\na\r\nb\r\n") { std::printf("  FAIL: resp_encode\n"); fails++; }
    int port = 20000 + (int)(getpid() % 20000);
    pid_t rp = start_redis(port);
    int fd = -1;
    for (int i = 0; i < 100 && fd < 0; i++) { usleep(50000); fd = connect_to(port); }
    if (fd < 0) { std::printf("Không chạy được redis-server (đã cài chưa?)\nCHƯA ĐẠT\n"); kill(rp, SIGKILL); return 1; }
    {
        Redis r(fd);
        Resp x = r.command({"PING"});
        if (x.type != Resp::SIMPLE || x.str != "PONG") { std::printf("  FAIL: PING\n"); fails++; }
        std::string big(100000, 'z'); big[500] = '\r'; big[501] = '\n'; big[9000] = '\0';
        r.command({"SET", "big", big});
        x = r.command({"GET", "big"});
        if (x.type != Resp::BULK || x.str != big) { std::printf("  FAIL: GET giá trị 100 KB có \\r\\n và byte 0\n"); fails++; }
        x = r.command({"GET", "khong-co"});
        if (x.type != Resp::NIL) { std::printf("  FAIL: GET khóa không tồn tại phải là NIL\n"); fails++; }
        x = r.command({"INCRBY", "n", "41"}); x = r.command({"INCR", "n"});
        if (x.type != Resp::INTEGER || x.num != 42) { std::printf("  FAIL: INCR\n"); fails++; }
        x = r.command({"LPUSH", "l", "a"}); x = r.command({"INCR", "l"});
        if (x.type != Resp::ERROR || x.str.rfind("WRONGTYPE", 0) != 0) { std::printf("  FAIL: lỗi WRONGTYPE\n"); fails++; }
        r.command({"HSET", "h", "f1", "v1", "f2", ""});
        x = r.command({"HGETALL", "h"});
        if (x.type != Resp::ARRAY || x.elems.size() != 4) { std::printf("  FAIL: HGETALL phải là mảng 4 phần tử\n"); fails++; }
        r.command({"MULTI"}); r.command({"INCR", "n"}); r.command({"LRANGE", "l", "0", "-1"});
        x = r.command({"EXEC"});
        if (x.type != Resp::ARRAY || x.elems.size() != 2 || x.elems[1].type != Resp::ARRAY || x.elems[1].elems.size() != 1) {
            std::printf("  FAIL: EXEC trả mảng lồng mảng\n"); fails++; }
        r.command({"HSET", "quota:imsi-1", "balance", "5000"});
    }
    std::atomic<long long> deducted{0};
    std::vector<std::thread> ts;
    for (int t = 0; t < 8; t++) ts.emplace_back([&, t] {
        int cfd = connect_to(port);
        if (cfd < 0) return;
        Redis r(cfd);
        std::mt19937 rng(t);
        for (int i = 0; i < 300; i++) { long long a = 1 + rng() % 5; if (deduct(r, "quota:imsi-1", a)) deducted += a; }
    });
    for (auto& th : ts) th.join();
    {
        Redis r(connect_to(port));
        Resp b = r.command({"HGET", "quota:imsi-1", "balance"});
        long long bal = b.type == Resp::BULK ? std::stoll(b.str) : -1;
        std::printf("đã trừ thành công %lld, balance còn %lld (phải là %lld)\n", deducted.load(), bal, 5000 - deducted.load());
        if (bal != 5000 - deducted || bal < 0) { std::printf("  FAIL: lost update giữa các client\n"); fails++; }
    }
    kill(rp, SIGTERM);
    waitpid(rp, nullptr, 0);
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
