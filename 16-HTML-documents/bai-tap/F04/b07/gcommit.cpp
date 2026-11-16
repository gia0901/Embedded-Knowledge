// gcommit: nhiều luồng cùng commit bản ghi vào một file log, mỗi commit phải nằm trên đĩa trước khi trả về.
//   ./gcommit each  T N   mỗi commit tự write + fdatasync, giữ khoá suốt lúc chờ đĩa
//   ./gcommit group T N   group commit: luồng đầu tiên thấy hàng chờ làm "leader", ghi và fdatasync
//                         mọi bản ghi đang chờ trong một lần; các luồng khác chỉ chờ tới khi bản ghi của mình đã bền
//   ./gcommit check       đọc log.bin, đếm bản ghi có CRC đúng, bản ghi hỏng, phần dư cuối file
// T luồng, mỗi luồng N commit. Bản ghi 64 byte có CRC32 để phát hiện ghi dở khi crash.
// Build: g++ -O2 -std=c++20 -Wall -pthread -o gcommit gcommit.cpp
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct Rec { uint64_t seq; uint32_t thread, len; char data[44]; uint32_t crc; };
static_assert(sizeof(Rec) == 64);

static uint32_t crc32(const void* p, size_t n) {
    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < n; i++) {
        c ^= ((const uint8_t*)p)[i];
        for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320 & -(c & 1));
    }
    return ~c;
}

static int fd;
static std::mutex mu;
static std::condition_variable cv;
static long syncs = 0;
// group commit
static std::vector<Rec> pending;         // bản ghi đã nhận, chưa ghi
static uint64_t next_seq = 0;            // số thứ tự cấp cho bản ghi tiếp theo
static uint64_t durable = 0;             // mọi bản ghi có seq < durable đã fdatasync xong
static bool leader_busy = false;

static Rec make(uint32_t t, uint64_t seq) {
    Rec r{}; r.seq = seq; r.thread = t; r.len = snprintf(r.data, sizeof r.data, "luong %u ban ghi %llu", t, (unsigned long long)seq);
    r.crc = crc32(&r, offsetof(Rec, crc));
    return r;
}

static void commit_each(uint32_t t) {
    std::lock_guard<std::mutex> g(mu);
    Rec r = make(t, next_seq++);
    if (write(fd, &r, sizeof r) != sizeof r || fdatasync(fd) != 0) { perror("ghi"); exit(1); }
    syncs++;
}

static void commit_group(uint32_t t) {
    std::unique_lock<std::mutex> lk(mu);
    uint64_t my = next_seq++;
    pending.push_back(make(t, my));
    while (durable <= my) {
        if (!leader_busy) {                          // không ai đang ghi: mình làm leader
            leader_busy = true;
            std::vector<Rec> batch; batch.swap(pending);
            uint64_t upto = next_seq;
            lk.unlock();                             // ghi và chờ đĩa ngoài khoá: luồng khác vẫn xếp hàng được
            size_t n = batch.size() * sizeof(Rec);
            if (write(fd, batch.data(), n) != (ssize_t)n || fdatasync(fd) != 0) { perror("ghi"); exit(1); }
            lk.lock();
            syncs++; durable = upto; leader_busy = false;
            cv.notify_all();
        } else {
            cv.wait(lk);
        }
    }
}

int main(int argc, char** argv) {
    if (argc >= 2 && !strcmp(argv[1], "check")) {
        int f = open("log.bin", O_RDONLY); if (f < 0) { perror("log.bin"); return 1; }
        Rec r; long ok = 0, bad = 0; ssize_t k;
        while ((k = read(f, &r, sizeof r)) == sizeof r) (crc32(&r, offsetof(Rec, crc)) == r.crc ? ok : bad)++;
        printf("check: %ld ban ghi CRC dung, %ld ban ghi hong, %zd byte du o cuoi\n", ok, bad, k > 0 ? k : 0);
        return bad ? 1 : 0;
    }
    if (argc < 4) { fprintf(stderr, "dung: %s each|group THREADS N | check\n", argv[0]); return 1; }
    bool group = !strcmp(argv[1], "group");
    int T = atoi(argv[2]), N = atoi(argv[3]);
    fd = open("log.bin", O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    auto t0 = std::chrono::steady_clock::now();
    std::vector<std::thread> th;
    for (int t = 0; t < T; t++)
        th.emplace_back([=] { for (int i = 0; i < N; i++) group ? commit_group(t) : commit_each(t); });
    for (auto& x : th) x.join();
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    long total = (long)T * N;
    printf("%-5s %2d luong: %6ld commit, %6ld fdatasync, %5.1f commit/fdatasync, %7.0f commit/s\n",
           argv[1], T, total, syncs, (double)total / syncs, total / s);
    close(fd);
}
