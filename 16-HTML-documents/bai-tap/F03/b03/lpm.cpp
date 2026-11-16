// lpm: longest prefix match bằng binary trie, đối chiếu với bảng route của Linux.
//   ./lpm gen N SEED            in N lệnh "route add P/L via 192.168.0.X" (cho ip -batch)
//   ./lpm addrs N SEED          in N địa chỉ unicast ngẫu nhiên, phần lớn trong 10.0.0.0/8
//   ./lpm [--first] ROUTES      đọc địa chỉ từ stdin, in "addr gw" (--first: lấy route khớp đầu tiên)
//   ./lpm bench ROUTES          đo ns/lookup: trie và quét tuyến tính
// ROUTES là output của "ip route show": "default via G ..." hoặc "P/L via G ...".
#include <arpa/inet.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct Route { uint32_t net; int len; std::string gw; };

static uint32_t mask(int len) { return len == 0 ? 0 : ~uint32_t(0) << (32 - len); }

static uint32_t parse_ip(const std::string& s) {
    in_addr a{};
    if (inet_pton(AF_INET, s.c_str(), &a) != 1) { fprintf(stderr, "dia chi sai: %s\n", s.c_str()); exit(1); }
    return ntohl(a.s_addr);
}
static std::string ip_str(uint32_t v) {
    in_addr a{htonl(v)}; char b[INET_ADDRSTRLEN]; inet_ntop(AF_INET, &a, b, sizeof b); return b;
}

struct Node { std::unique_ptr<Node> kid[2]; const Route* r = nullptr; };

struct Trie {
    Node root;
    void insert(const Route& r) {
        Node* n = &root;
        for (int i = 0; i < r.len; i++) {
            int bit = (r.net >> (31 - i)) & 1;
            if (!n->kid[bit]) n->kid[bit] = std::make_unique<Node>();
            n = n->kid[bit].get();
        }
        n->r = &r;
    }
    const Route* lookup(uint32_t a) const {          // đi xuống theo từng bit, nhớ route sâu nhất gặp được
        const Node* n = &root; const Route* best = root.r;
        for (int i = 0; i < 32 && n; i++) {
            n = n->kid[(a >> (31 - i)) & 1].get();
            if (n && n->r) best = n->r;
        }
        return best;
    }
};

static const Route* linear_longest(const std::vector<Route>& rt, uint32_t a) {
    const Route* best = nullptr;
    for (auto& r : rt) if ((a & mask(r.len)) == r.net && (!best || r.len > best->len)) best = &r;
    return best;
}
static const Route* first_match(const std::vector<Route>& rt, uint32_t a) {
    for (auto& r : rt) if ((a & mask(r.len)) == r.net) return &r;
    return nullptr;
}

static std::vector<Route> load(const char* path) {
    std::ifstream f(path); std::string line; std::vector<Route> rt;
    if (!f) { perror(path); exit(1); }
    while (std::getline(f, line)) {
        std::istringstream ss(line); std::string dst, via, gw;
        ss >> dst >> via >> gw;
        if (via != "via") continue;                       // bỏ route "dev ... scope link"
        Route r; r.gw = gw;
        if (dst == "default") { r.net = 0; r.len = 0; }
        else {
            auto slash = dst.find('/');
            r.len = slash == std::string::npos ? 32 : std::stoi(dst.substr(slash + 1));
            r.net = parse_ip(dst.substr(0, slash)) & mask(r.len);
        }
        rt.push_back(r);
    }
    return rt;
}

int main(int argc, char** argv) {
    if (argc >= 4 && !strcmp(argv[1], "gen")) {
        int n = atoi(argv[2]); std::mt19937 g(atoi(argv[3])); std::set<std::pair<uint32_t,int>> seen;
        while ((int)seen.size() < n) {
            int len = 8 + g() % 25;                        // /8 tới /32
            uint32_t net = ((10u << 24) | (g() & 0x00ffffff)) & mask(len);
            if (!seen.insert({net, len}).second) continue;
            printf("route add %s/%d via 192.168.0.%u\n", ip_str(net).c_str(), len, unsigned(1 + g() % 250));
        }
        return 0;
    }
    if (argc >= 4 && !strcmp(argv[1], "addrs")) {
        int n = atoi(argv[2]); std::mt19937 g(atoi(argv[3]));
        for (int i = 0; i < n; i++) {
            uint32_t a = g() % 10 ? (10u << 24) | (g() & 0x00ffffff) : g();
            if (a >> 28 >= 14) { i--; continue; }          // bỏ 224.0.0.0 trở lên (multicast, dự trữ)
            printf("%s\n", ip_str(a).c_str());
        }
        return 0;
    }
    if (argc >= 3 && !strcmp(argv[1], "bench")) {
        auto rt = load(argv[2]); Trie t; for (auto& r : rt) t.insert(r);
        std::mt19937 g(1); std::vector<uint32_t> q(200000);
        for (auto& a : q) a = (10u << 24) | (g() & 0x00ffffff);
        auto run = [&](auto fn, const char* name) {
            auto t0 = std::chrono::steady_clock::now(); volatile uintptr_t sink = 0;
            for (auto a : q) sink = sink + (uintptr_t)fn(a);
            double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / q.size();
            printf("%-10s %zu route: %8.1f ns/lookup\n", name, rt.size(), ns);
        };
        run([&](uint32_t a) { return t.lookup(a); }, "trie");
        run([&](uint32_t a) { return linear_longest(rt, a); }, "tuyen-tinh");
        return 0;
    }
    bool first = argc >= 3 && !strcmp(argv[1], "--first");
    if (argc < 2 + first) { fprintf(stderr, "dung: lpm [--first] ROUTES < addrs | gen N SEED | addrs N SEED | bench ROUTES\n"); return 1; }
    auto rt = load(argv[1 + first]); Trie t; for (auto& r : rt) t.insert(r);
    std::string s;
    while (std::cin >> s) {
        const Route* r = first ? first_match(rt, parse_ip(s)) : t.lookup(parse_ip(s));
        printf("%s %s\n", s.c_str(), r ? r->gw.c_str() : "unreachable");
    }
}
