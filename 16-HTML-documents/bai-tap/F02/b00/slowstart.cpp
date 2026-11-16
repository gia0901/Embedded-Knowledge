// slowstart: bản "telemd-agent" khởi động chậm, dùng cho Lab 0.1. Đừng đọc code trước khi làm lab:
// nhiệm vụ là tìm ra nguyên nhân chỉ bằng strace.
// Build: g++ -O2 -std=c++20 -o slowstart slowstart.cpp
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static int try_config() {
    const char* paths[] = {"/etc/telemd/telemd.conf", "/usr/local/etc/telemd.conf", "./telemd.conf"};
    for (const char* p : paths) {
        int fd = open(p, O_RDONLY | O_CLOEXEC);
        if (fd >= 0) return fd;
    }
    return -1;   // không có file cấu hình: dùng mặc định, không phải lỗi
}

static int connect_ctl() {
    for (int attempt = 0; attempt < 5; ++attempt) {
        int s = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        sockaddr_un a{};
        a.sun_family = AF_UNIX;
        std::strcpy(a.sun_path, "/tmp/telemd-ctl.sock");
        if (connect(s, reinterpret_cast<sockaddr*>(&a), sizeof a) == 0) return s;
        close(s);
        usleep(400'000);   // chờ control socket sẵn sàng rồi thử lại
    }
    return -1;
}

int main() {
    int cfg = try_config();
    int ctl = connect_ctl();
    std::printf("agent san sang (config=%s, ctl=%s)\n", cfg >= 0 ? "co" : "mac dinh", ctl >= 0 ? "ket noi" : "khong co");
}
