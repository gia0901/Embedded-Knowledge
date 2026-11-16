// gtpu: một đầu GTP-U chạy trong user space, đủ để đóng vai gNB hay UPF đơn giản: TUN <-> UDP 2152.
// Build: g++ -O2 -std=c++20 -Wall -o gtpu gtpu.cpp
// Chạy:  ./gtpu TUN LOCAL_IP SESSIONS      (tự tạo TUN; script cấu hình địa chỉ, route sau)
// SESSIONS: mỗi dòng "ue_ip  teid_nhan  peer_ip  teid_gui" (một session = một UE, như PDR + FAR rút gọn)
//   Gói đọc từ TUN có src hoặc dst là ue_ip  -> bọc GTP-U với teid_gui, gửi tới peer_ip:2152.
//   Gói GTP-U tới có TEID = teid_nhan và IP bên trong thuộc ue_ip -> bóc, ghi vào TUN.
// SIGHUP: đọc lại SESSIONS (như PFCP sửa hay xoá session). SIGUSR1: in bộ đếm.
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

struct Session {
    uint32_t ue, teid_in, peer, teid_out;              // địa chỉ ở dạng network order
    uint64_t ul_pkt = 0, ul_byte = 0, dl_pkt = 0, dl_byte = 0;
};
static std::vector<Session> sess;
static uint64_t drop_teid = 0, drop_inner = 0, drop_nosess = 0, drop_bad = 0;
static volatile sig_atomic_t want_reload = 0, want_dump = 0;
static const char* sess_file;

static void load() {
    std::vector<Session> s;
    std::ifstream in(sess_file);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string ue, peer; uint32_t ti, to;
        if (!(ss >> ue >> ti >> peer >> to)) continue;
        Session x{};
        inet_pton(AF_INET, ue.c_str(), &x.ue); inet_pton(AF_INET, peer.c_str(), &x.peer);
        x.teid_in = ti; x.teid_out = to;
        for (auto& o : sess) if (o.ue == x.ue) { x.ul_pkt = o.ul_pkt; x.ul_byte = o.ul_byte; x.dl_pkt = o.dl_pkt; x.dl_byte = o.dl_byte; }
        s.push_back(x);
    }
    sess = s;
    fprintf(stderr, "gtpu: %zu session\n", sess.size());
}

static void dump() {
    char a[INET_ADDRSTRLEN];
    for (auto& s : sess) {
        inet_ntop(AF_INET, &s.ue, a, sizeof a);
        fprintf(stderr, "  ue %-10s teid nhan %u gui %u | len %lu goi %lu byte | xuong %lu goi %lu byte\n", a,
                s.teid_in, s.teid_out, s.ul_pkt, s.ul_byte, s.dl_pkt, s.dl_byte);
    }
    fprintf(stderr, "  bo: teid la %lu, ip trong sai %lu, khong session %lu, hong %lu\n",
            drop_teid, drop_inner, drop_nosess, drop_bad);
}

static int tun_open(const char* name) {
    int fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) { perror("/dev/net/tun"); return -1; }
    ifreq r{};
    r.ifr_flags = IFF_TUN | IFF_NO_PI;                  // gói IP trần, không header phụ
    strncpy(r.ifr_name, name, IFNAMSIZ - 1);
    if (ioctl(fd, TUNSETIFF, &r) < 0) { perror("TUNSETIFF"); return -1; }
    return fd;
}

int main(int argc, char** argv) {
    if (argc < 4) { fprintf(stderr, "dung: %s TUN LOCAL_IP SESSIONS\n", argv[0]); return 1; }
    sess_file = argv[3];
    load();
    signal(SIGHUP, [](int) { want_reload = 1; });
    signal(SIGUSR1, [](int) { want_dump = 1; });
    signal(SIGTERM, [](int) { want_dump = 2; });
    int tun = tun_open(argv[1]);
    int u = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in me{AF_INET, htons(2152), {}, {}};
    inet_pton(AF_INET, argv[2], &me.sin_addr);
    if (tun < 0 || bind(u, (sockaddr*)&me, sizeof me) < 0) { perror("bind"); return 1; }
    uint8_t buf[2048];
    pollfd p[2] = {{tun, POLLIN, 0}, {u, POLLIN, 0}};
    for (;;) {
        if (want_reload) { want_reload = 0; load(); }
        if (want_dump) { dump(); if (want_dump == 2) return 0; want_dump = 0; }
        if (poll(p, 2, -1) < 0) { if (errno == EINTR) continue; perror("poll"); return 1; }
        if (p[0].revents & POLLIN) {                   // TUN -> bọc -> UDP
            ssize_t n = read(tun, buf + 8, sizeof buf - 8);
            if (n < 20 || (buf[8] >> 4) != 4) { drop_bad++; continue; }
            uint32_t src, dst;
            memcpy(&src, buf + 8 + 12, 4); memcpy(&dst, buf + 8 + 16, 4);
            Session* s = nullptr;
            for (auto& x : sess) if (x.ue == src || x.ue == dst) { s = &x; break; }
            if (!s) { drop_nosess++; continue; }
            if (s->ue == src) { s->ul_pkt++; s->ul_byte += n; } else { s->dl_pkt++; s->dl_byte += n; }
            buf[0] = 0x30;                              // version 1, PT = 1 (GTP), không E, S, PN
            buf[1] = 0xFF;                              // G-PDU: gói người dùng
            uint16_t len = htons(n); memcpy(buf + 2, &len, 2);
            uint32_t teid = htonl(s->teid_out); memcpy(buf + 4, &teid, 4);
            sockaddr_in to{AF_INET, htons(2152), {s->peer}, {}};
            sendto(u, buf, n + 8, 0, (sockaddr*)&to, sizeof to);
        }
        if (p[1].revents & POLLIN) {                   // UDP -> bóc -> TUN
            ssize_t n = recv(u, buf, sizeof buf, 0);
            if (n < 8 + 20 || (buf[0] & 0xE0) != 0x20 || buf[1] != 0xFF) { drop_bad++; continue; }
            uint32_t teid; memcpy(&teid, buf + 4, 4); teid = ntohl(teid);
            Session* s = nullptr;
            for (auto& x : sess) if (x.teid_in == teid) { s = &x; break; }
            if (!s) { drop_teid++; continue; }
            uint32_t src, dst;
            memcpy(&src, buf + 8 + 12, 4); memcpy(&dst, buf + 8 + 16, 4);
            if (s->ue != src && s->ue != dst) { drop_inner++; continue; }
            if (s->ue == src) { s->ul_pkt++; s->ul_byte += n - 8; } else { s->dl_pkt++; s->dl_byte += n - 8; }
            if (write(tun, buf + 8, n - 8) < 0) drop_bad++;
        }
    }
}
