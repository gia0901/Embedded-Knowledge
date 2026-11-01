// Bài tập F04-4: Thêm compaction cho KV store kiểu log (Bài 9) và chứng minh nó chịu được crash.
// KvLog dưới đây là bản trong bài học, thiếu compact(). Viết compact() sao cho:
//   - File log chỉ còn một bản ghi cho mỗi khóa đang tồn tại (giá trị mới nhất).
//   - Bị kill -9 ở BẤT KỲ thời điểm nào (kể cả giữa lúc compact) thì mở lại vẫn có đủ mọi put đã trả về.
// Gợi ý: mẫu atomic replace của Bài 7 (file tạm, fsync, rename, fsync thư mục), rồi mở lại fd_ trên file mới.
// Test fork một process con ghi liên tục và compact định kỳ, giết nó bằng SIGKILL ở thời điểm ngẫu nhiên 60 lần.
// Lưu ý: SIGKILL kiểm tra crash của process; dữ liệu đã write() vẫn còn trong page cache. Mất điện thật còn khắt khe hơn.
// Biên dịch: g++ -std=c++20 -O1 -g ex4_kvlog_compact.cpp -o ex4 && ./ex4
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

static uint32_t crc32(const uint8_t* p, size_t n) {
    uint32_t c = 0xFFFFFFFFu;
    for (size_t i = 0; i < n; i++) { c ^= p[i]; for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1))); }
    return ~c;
}

class KvLog {
public:
    explicit KvLog(std::string path) : path_(std::move(path)) {
        unlink((path_ + ".tmp").c_str());                     // file tạm sót lại từ lần compact bị gián đoạn
        fd_ = open(path_.c_str(), O_RDWR | O_CREAT | O_APPEND, 0644);
        struct stat st; fstat(fd_, &st);
        std::vector<uint8_t> all((size_t)st.st_size);
        if (pread(fd_, all.data(), all.size(), 0) != (ssize_t)all.size()) throw 1;
        size_t off = 0;
        while (off + 8 <= all.size()) {
            uint32_t c; uint16_t kl, vl;
            std::memcpy(&c, &all[off], 4); std::memcpy(&kl, &all[off + 4], 2); std::memcpy(&vl, &all[off + 6], 2);
            size_t len = 8 + kl + (vl == kDel ? 0 : vl);
            if (off + len > all.size() || crc32(&all[off + 4], len - 4) != c) break;
            std::string k((char*)&all[off + 8], kl);
            if (vl == kDel) mem_.erase(k); else mem_[k] = std::string((char*)&all[off + 8 + kl], vl);
            off += len;
        }
        if (off < all.size() && ftruncate(fd_, (off_t)off) != 0) throw 2;
    }
    ~KvLog() { close(fd_); }
    void put(const std::string& k, const std::string& v) {
        auto r = record(k, &v);
        if (write(fd_, r.data(), r.size()) != (ssize_t)r.size()) throw 3;
        fdatasync(fd_);
        mem_[k] = v;
    }
    const std::string* get(const std::string& k) const { auto it = mem_.find(k); return it == mem_.end() ? nullptr : &it->second; }
    size_t file_size() const { struct stat st; fstat(fd_, &st); return (size_t)st.st_size; }

    // ===================== PHẦN CỦA BẠN =====================
    void compact() {
        std::string tmp = path_ + ".tmp";
        int t = open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
        if (t < 0) throw 10;
        std::vector<uint8_t> buf;
        for (auto& [k, v] : mem_) { auto r = record(k, &v); buf.insert(buf.end(), r.begin(), r.end()); }
        size_t off = 0;
        while (off < buf.size()) {
            ssize_t w = write(t, buf.data() + off, buf.size() - off);
            if (w <= 0) throw 11;
            off += (size_t)w;
        }
        if (fsync(t) != 0 || close(t) != 0) throw 12;          // 1. dữ liệu mới nằm trên đĩa
        if (rename(tmp.c_str(), path_.c_str()) != 0) throw 13; // 2. đổi tên nguyên tử: file cũ hoặc file mới, không có lưng chừng
        std::string dir = path_.find('/') == std::string::npos ? "." : path_.substr(0, path_.rfind('/'));
        int d = open(dir.c_str(), O_RDONLY | O_DIRECTORY);
        if (d >= 0) { fsync(d); close(d); }                   // 3. lưu thay đổi tên trong thư mục
        close(fd_);                                           // 4. fd_ cũ trỏ vào file đã bị thay; mở file mới để ghi tiếp
        fd_ = open(path_.c_str(), O_RDWR | O_APPEND | O_CLOEXEC);
        if (fd_ < 0) throw 14;
    }
    // ================== HẾT PHẦN CỦA BẠN ====================

private:
    static constexpr uint16_t kDel = 0xFFFF;
    std::string path_;
    int fd_ = -1;
    std::unordered_map<std::string, std::string> mem_;
    static std::vector<uint8_t> record(const std::string& k, const std::string* v) {
        std::vector<uint8_t> rec(8 + k.size() + (v ? v->size() : 0));
        uint16_t kl = (uint16_t)k.size(), vl = v ? (uint16_t)v->size() : kDel;
        std::memcpy(&rec[4], &kl, 2); std::memcpy(&rec[6], &vl, 2);
        std::memcpy(&rec[8], k.data(), k.size());
        if (v) std::memcpy(&rec[8 + k.size()], v->data(), v->size());
        uint32_t c = crc32(&rec[4], rec.size() - 4);
        std::memcpy(&rec[0], &c, 4);
        return rec;
    }
};

// ========================= TEST =========================
// Process con: put k<i % 40> = i, báo i qua pipe SAU khi put trả về; compact mỗi 150 lần put.
static void child(const char* path, int wfd, int start) {
    KvLog kv(path);
    for (int i = start;; i++) {
        kv.put("k" + std::to_string(i % 40), std::to_string(i));
        if (write(wfd, &i, sizeof i) != sizeof i) _exit(1);
        if (i % 150 == 0) kv.compact();
    }
}
int main() {
    const char* path = "ex4_kv.log";
    unlink(path);
    std::mt19937 rng(99);
    std::vector<int> acked(40, -1);                          // giá trị lớn nhất đã được xác nhận cho mỗi khóa
    int next = 1, fails = 0;
    for (int round = 0; round < 60 && !fails; round++) {
        int p[2]; if (pipe(p) != 0) return 2;
        pid_t pid = fork();
        if (pid == 0) { close(p[0]); child(path, p[1], next); }
        close(p[1]);
        usleep(2000 + rng() % 25000);
        kill(pid, SIGKILL);
        int i;
        while (read(p[0], &i, sizeof i) == sizeof i) { acked[(size_t)(i % 40)] = i; next = i + 1; }
        close(p[0]);
        waitpid(pid, nullptr, 0);
        KvLog kv(path);                                      // khôi phục và kiểm tra
        for (int k = 0; k < 40; k++) {
            if (acked[(size_t)k] < 0) continue;
            const std::string* v = kv.get("k" + std::to_string(k));
            if (!v || std::stoi(*v) < acked[(size_t)k]) {
                std::printf("  FAIL vòng %d: k%d phải >= %d, đọc được %s\n", round, k, acked[(size_t)k], v ? v->c_str() : "(không có)");
                fails++; break;
            }
        }
        next += 1000;                                        // giá trị vòng sau luôn lớn hơn
    }
    {
        KvLog kv(path);
        for (int i = 0; i < 2000; i++) kv.put("k" + std::to_string(i % 40), std::to_string(next + i));
        size_t before = kv.file_size();
        kv.compact();
        kv.put("k0", "sau-compact");                         // vẫn ghi tiếp được sau compact
        size_t after = kv.file_size();
        std::printf("kích thước log trước compact %zu byte, sau %zu byte\n", before, after);
        if (after > 2000) { std::printf("  FAIL: sau compact log phải chỉ còn khoảng 40 bản ghi\n"); fails++; }
    }
    {
        KvLog kv(path);
        if (!kv.get("k0") || *kv.get("k0") != "sau-compact") { std::printf("  FAIL: mất bản ghi ghi sau compact\n"); fails++; }
    }
    unlink(path);
    std::printf(fails ? "CHƯA ĐẠT: %d kiểm tra sai\n" : "ĐẠT tất cả kiểm tra\n", fails);
    return fails ? 1 : 0;
}
