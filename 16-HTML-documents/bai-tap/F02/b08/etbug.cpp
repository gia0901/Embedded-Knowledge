// etbug: lỗi kinh điển của epoll edge-triggered. Client gửi request 200 byte rồi chờ trả lời;
// server đọc mỗi lần 64 byte. Ba chế độ server:
//   lt1    level-triggered, mỗi sự kiện đọc 1 lần   → epoll báo lại vì còn dữ liệu: đúng
//   et1    edge-triggered, mỗi sự kiện đọc 1 lần    → phần còn lại nằm trong buffer, không có cạnh mới: treo
//   etloop edge-triggered, đọc tới khi EAGAIN       → đúng, ít lần thức dậy hơn
// Lời giải Lab 8.2. Build: g++ -O2 -std=c++20 -pthread -o etbug etbug.cpp
// Chạy:  ./etbug lt1|et1|etloop [số request=1000]
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "lt1";
    int reqs = argc > 2 ? std::atoi(argv[2]) : 1000;
    bool et = mode != "lt1", loop = mode == "etloop";
    int sv[2];
    socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv);
    fcntl(sv[1], F_SETFL, O_NONBLOCK);                  // ET bắt buộc fd non-blocking

    int answered = 0;
    std::thread client([&] {
        std::string req(199, 'r'); req += '\n';         // 200 byte, gửi bằng một lần write
        for (int i = 0; i < reqs; ++i) {
            if (write(sv[0], req.data(), req.size()) != (ssize_t)req.size()) break;
            pollfd p{sv[0], POLLIN, 0};
            if (poll(&p, 1, 1000) == 0) {               // chờ trả lời tối đa 1 giây
                std::printf("request %d: khong co tra loi sau 1 s (treo)\n", i);
                break;
            }
            char ok[8];
            if (read(sv[0], ok, sizeof ok) <= 0) break;
            ++answered;
        }
        shutdown(sv[0], SHUT_WR);                       // báo server: hết request
    });

    int ep = epoll_create1(EPOLL_CLOEXEC);
    epoll_event ev{};
    ev.events = EPOLLIN | (et ? EPOLLET : 0);
    epoll_ctl(ep, EPOLL_CTL_ADD, sv[1], &ev);
    long wakeups = 0, reads = 0, have = 0;
    bool eof = false;
    while (!eof) {
        epoll_event out;
        int n = epoll_wait(ep, &out, 1, 1500);
        if (n == 0) break;                              // client đã bỏ cuộc
        ++wakeups;
        do {
            char buf[64];
            ssize_t r = read(sv[1], buf, sizeof buf);
            ++reads;
            if (r == 0) { eof = true; break; }
            if (r < 0) break;                           // EAGAIN: đã đọc hết
            for (ssize_t i = 0; i < r; ++i)
                if (buf[i] == '\n') { if (write(sv[1], "ok\n", 3) != 3) return 1; }
            have += r;
        } while (loop);
    }
    client.join();
    std::printf("%-6s tra loi %d/%d request, %ld lan epoll_wait, %ld lan read, %ld byte\n",
                mode.c_str(), answered, reqs, wakeups, reads, have);
    return answered == reqs ? 0 : 1;
}
