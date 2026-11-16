// miniswitch: switch lớp 2 tự học trong user space. Mỗi cổng là một interface, đọc/ghi frame bằng AF_PACKET.
// Học: MAC nguồn -> cổng vào. Chuyển: MAC đích đã biết -> đúng một cổng; chưa biết hoặc broadcast -> flood.
// Build: g++ -O2 -std=c++20 -o miniswitch miniswitch.cpp
// Chạy (root, trong namespace sw): ./miniswitch [--hub] p1 p2 p3     (Ctrl-C để in thống kê)
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

using Mac = std::array<uint8_t, 6>;
static volatile sig_atomic_t stop = 0;

struct Port { std::string name; int fd; int idx; long rx = 0, tx = 0; };

int main(int argc, char** argv) {
    bool hub = false; std::vector<Port> ports;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--hub")) { hub = true; continue; }
        int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        sockaddr_ll sll{}; sll.sll_family = AF_PACKET; sll.sll_protocol = htons(ETH_P_ALL);
        sll.sll_ifindex = if_nametoindex(argv[i]);
        if (fd < 0 || !sll.sll_ifindex || bind(fd, (sockaddr*)&sll, sizeof sll) < 0) { perror(argv[i]); return 1; }
        packet_mreq mr{}; mr.mr_ifindex = sll.sll_ifindex; mr.mr_type = PACKET_MR_PROMISC;   // nhận cả frame không gửi cho mình
        setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof mr);
        ports.push_back({argv[i], fd, sll.sll_ifindex});
    }
    signal(SIGINT, [](int) { stop = 1; });
    std::map<Mac, std::pair<int, std::chrono::steady_clock::time_point>> fdb;   // bảng MAC -> (cổng, lúc thấy)
    long flooded = 0, forwarded = 0, filtered = 0;
    std::vector<pollfd> pfd;
    for (auto& p : ports) pfd.push_back({p.fd, POLLIN, 0});
    printf("miniswitch %s: %zu cong\n", hub ? "(hub, khong hoc)" : "(hoc MAC)", ports.size()); fflush(stdout);
    uint8_t f[2048];
    while (!stop) {
        if (poll(pfd.data(), pfd.size(), 200) <= 0) continue;
        for (size_t in = 0; in < ports.size(); in++) {
            if (!(pfd[in].revents & POLLIN)) continue;
            sockaddr_ll from{}; socklen_t fl = sizeof from;
            ssize_t n = recvfrom(ports[in].fd, f, sizeof f, 0, (sockaddr*)&from, &fl);
            if (n < 14 || from.sll_pkttype == PACKET_OUTGOING) continue;   // bỏ frame chính mình vừa gửi ra
            ports[in].rx++;
            Mac dst, src; memcpy(dst.data(), f, 6); memcpy(src.data(), f + 6, 6);
            auto now = std::chrono::steady_clock::now();
            if (!hub) fdb[src] = {(int)in, now};                         // học
            auto it = hub ? fdb.end() : fdb.find(dst);
            bool known = it != fdb.end() && !(dst[0] & 1) && now - it->second.second < std::chrono::seconds(300);
            if (known && it->second.first == (int)in) { filtered++; continue; }   // đích ở cùng cổng: bỏ
            for (size_t out = 0; out < ports.size(); out++) {
                if (out == in || (known && (int)out != it->second.first)) continue;
                if (send(ports[out].fd, f, n, 0) == n) ports[out].tx++;
            }
            known ? forwarded++ : flooded++;
        }
    }
    printf("\nframe: chuyen dung cong %ld, flood %ld, bo %ld\n", forwarded, flooded, filtered);
    for (auto& p : ports) printf("  %-6s nhan %4ld  gui %4ld\n", p.name.c_str(), p.rx, p.tx);
    printf("bang MAC:\n");
    for (auto& [m, v] : fdb)
        printf("  %02x:%02x:%02x:%02x:%02x:%02x -> %s\n", m[0], m[1], m[2], m[3], m[4], m[5], ports[v.first].name.c_str());
}
