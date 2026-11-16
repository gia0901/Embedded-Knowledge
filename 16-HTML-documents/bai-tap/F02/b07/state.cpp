// state: ghi file trạng thái state.json lặp lại theo hai cách, và kiểm tra file đọc được có nguyên vẹn không.
//   naive : open(O_TRUNC) rồi write từng mảnh 4 KB thẳng vào state.json
//   atomic: ghi state.json.tmp, fsync, rename đè state.json, fsync thư mục
// Lời giải Lab 7.3. Build: g++ -O2 -std=c++20 -o state state.cpp
// Chạy:  ./state write naive|atomic <số lần>      ./state check <giây>      ./state once
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>

static std::string build(long seq) {                      // ~120 KB: seq ở đầu và ở cuối phải khớp nhau
    std::string s = "{\"seq\":" + std::to_string(seq) + ",\"dev\":[";
    for (int i = 0; i < 20000; ++i) s += std::to_string(seq + i) + (i + 1 < 20000 ? "," : "");
    return s + "],\"end\":" + std::to_string(seq) + "}\n";
}

static void write_all(int fd, const std::string& s) {
    for (size_t off = 0; off < s.size(); off += 4096) {   // từng mảnh 4 KB như một thư viện JSON ghi dần
        size_t len = std::min<size_t>(4096, s.size() - off);
        if (write(fd, s.data() + off, len) < 0) { std::perror("write"); std::exit(1); }
    }
}

static void save_naive(const std::string& s) {
    int fd = open("state.json", O_WRONLY | O_CREAT | O_TRUNC, 0644);   // từ đây tới hết write, file dở dang
    write_all(fd, s);
    close(fd);
}

static void save_atomic(const std::string& s) {
    int fd = open("state.json.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write_all(fd, s);
    fsync(fd);                                            // dữ liệu của file tạm xuống disk trước
    close(fd);
    rename("state.json.tmp", "state.json");               // đổi tên là atomic: người đọc thấy file cũ hoặc mới
    int dfd = open(".", O_RDONLY | O_DIRECTORY);
    fsync(dfd);                                           // để chính việc đổi tên cũng bền khi mất điện
    close(dfd);
}

static int classify() {                                   // 0 = nguyên vẹn, 1 = rỗng/không có, 2 = dở dang
    int fd = open("state.json", O_RDONLY);
    if (fd < 0) return 1;
    std::string s; char buf[65536]; ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0) s.append(buf, n);
    close(fd);
    if (s.empty()) return 1;
    long a = -1, b = -2;
    size_t e = s.rfind("\"end\":");
    if (std::sscanf(s.c_str(), "{\"seq\":%ld", &a) != 1 || e == std::string::npos || s.back() != '\n') return 2;
    std::sscanf(s.c_str() + e, "\"end\":%ld", &b);
    return a == b ? 0 : 2;
}

int main(int argc, char** argv) {
    if (!std::strcmp(argv[1], "write")) {
        bool atomic = !std::strcmp(argv[2], "atomic");
        long n = std::atol(argv[3]);
        auto t0 = std::chrono::steady_clock::now();
        for (long i = 1; i <= n; ++i) (atomic ? save_atomic : save_naive)(build(i));
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
        std::printf("%s: %ld lan ghi, %.2f ms/lan\n", argv[2], n, ms / n);
    } else if (!std::strcmp(argv[1], "check")) {
        long cnt[3] = {0, 0, 0};
        auto end = std::chrono::steady_clock::now() + std::chrono::seconds(std::atoi(argv[2]));
        while (std::chrono::steady_clock::now() < end) ++cnt[classify()];
        std::printf("doc %ld lan: nguyen ven %ld, rong %ld, do dang %ld\n", cnt[0] + cnt[1] + cnt[2], cnt[0], cnt[1], cnt[2]);
    } else {
        const char* name[] = {"nguyen ven", "rong hoac khong co", "do dang"};
        std::printf("state.json: %s\n", name[classify()]);
    }
}
