// colscan: cùng dữ liệu lưu theo dòng (rows.bin) và theo cột (mỗi cột một file), rồi tính avg(temp).
//   gen N   sinh N số đo: rows.bin (64 byte mỗi dòng) và dev.bin ts.bin temp.bin hum.bin fw.bin status.bin
//   row     đọc rows.bin, lấy temp của mỗi dòng
//   col     đọc temp.bin
// Build: g++ -O2 -std=c++20 -Wall -o colscan colscan.cpp
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

struct Row {                 // 64 byte: một dòng như row store cất (đã bỏ header)
    int32_t dev;
    int32_t fw;
    int64_t ts;
    float temp;
    float hum;
    int32_t status;
    char note[36];           // cột chữ ngắn, như mô tả hay mã lỗi
};
static_assert(sizeof(Row) == 64);

template <class T> static void put(const char* name, const std::vector<T>& v) {
    FILE* f = fopen(name, "wb"); fwrite(v.data(), sizeof(T), v.size(), f); fclose(f);
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "dung: %s gen N | row | col\n", argv[0]); return 1; }
    if (!strcmp(argv[1], "gen")) {
        long n = atol(argv[2]);
        std::vector<Row> rows(n);
        std::vector<int32_t> dev(n), fw(n), st(n); std::vector<int64_t> ts(n); std::vector<float> temp(n), hum(n);
        for (long i = 0; i < n; i++) {
            Row& r = rows[i];
            memset(&r, 0, sizeof r);
            r.dev = i % 1000; r.fw = 200 + r.dev % 3; r.ts = 1794704400 + (i / 1000) * 60;
            r.temp = 20 + (i * 7919 % 1500) / 100.0f; r.hum = 40 + (i * 104729 % 4000) / 100.0f; r.status = (i % 997 == 0);
            snprintf(r.note, sizeof r.note, "ok");
            dev[i] = r.dev; fw[i] = r.fw; ts[i] = r.ts; temp[i] = r.temp; hum[i] = r.hum; st[i] = r.status;
        }
        put("rows.bin", rows); put("dev.bin", dev); put("fw.bin", fw); put("ts.bin", ts);
        put("temp.bin", temp); put("hum.bin", hum); put("status.bin", st);
        printf("da sinh %ld dong\n", n);
        return 0;
    }
    bool row = !strcmp(argv[1], "row");
    auto t0 = std::chrono::steady_clock::now();
    int fd = open(row ? "rows.bin" : "temp.bin", O_RDONLY);
    std::vector<char> buf(1 << 20);                      // đọc từng khối 1 MiB, như đọc page theo lô
    double sum = 0; long n = 0, bytes = 0; ssize_t k;
    while ((k = read(fd, buf.data(), buf.size())) > 0) {
        bytes += k;
        if (row) { auto* r = (const Row*)buf.data(); for (size_t i = 0; i < k / sizeof(Row); i++) sum += r[i].temp; n += k / sizeof(Row); }
        else     { auto* t = (const float*)buf.data(); for (size_t i = 0; i < k / sizeof(float); i++) sum += t[i]; n += k / sizeof(float); }
    }
    close(fd);
    double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    printf("%s: %ld dong, doc %ld MB, avg(temp) = %.4f, %.3f s\n", row ? "row" : "col", n, bytes >> 20, sum / n, s);
}
