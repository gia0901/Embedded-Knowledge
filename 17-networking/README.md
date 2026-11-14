# Core Net Lab

Khoá học mạng máy tính 5 tuần, 18 module (m00–m17), dựa trên Kurose & Ross "Computer Networking: A Top-Down Approach" 8e và mở rộng theo hướng security, authentication, Kubernetes, 5G core và C/C++ với OpenSSL 3.

- Tuần 1–4 (m00–m16): khái niệm và lab chủ yếu bằng Python/Bash.
- Tuần 5 (m17): viết lại phần security bằng C++17 + OpenSSL 3 trong project `labs/m17/netsec`. Có thể học song song: 17.1 sau m09, 17.2 sau m11, 17.3 sau m12, 17.4 và 17.5 sau m15, 17.6 sau m16.

## Có gì trong gói này

| Đường dẫn | Nội dung |
|---|---|
| `CoreNetLab-offline.html` | Toàn bộ khoá học trong **một file**. Mở trực tiếp bằng browser (double-click), không cần server. |
| `web/` | Bản gốc gồm `index.html` và `modules/*.html`. Cần chạy qua HTTP server (xem bên dưới). |
| `labs/` | Code của các bài tập (Python, C, Bash, YAML…), tách sẵn theo đường dẫn `~/netlab/...` dùng trong bài. |

## Cách dùng

**Cách 1, đơn giản nhất:** mở `CoreNetLab-offline.html` bằng Chrome, Edge hoặc Firefox.

**Cách 2, bản web:** trong WSL hoặc bất kỳ máy nào có Python:

```bash
cd core-net-lab/web
python3 -m http.server 8000
```

Rồi mở http://localhost:8000.

**Code bài tập:** chép thư mục `labs/` thành `~/netlab` trong WSL:

```bash
cp -r labs ~/netlab
```

```bash
find ~/netlab -name '*.sh' -exec chmod +x {} +
```

## Lưu ý

- Font chữ (Google Fonts) và tô màu code (highlight.js) tải từ Internet. Khi offline, trang vẫn đọc được đầy đủ, chỉ dùng font mặc định và code không có màu.
- Tiến độ (module đã hoàn thành, điểm quiz, checklist) lưu trong localStorage của browser, riêng cho từng cách mở. Mở bằng file và mở qua server là hai nơi lưu khác nhau.
- Code trong `labs/` được trích từ phần "Lời giải". Một vài khối trong bài gộp nhiều file YAML (ví dụ `deny.yaml + allow-and.yaml`); khi đó file trong `labs/` chứa cả hai phần, tách theo dấu `---` và comment tên file.
- Code bài tập chưa được chạy trên Linux khi soạn; output mẫu trong bài (IP, thời gian, serial…) chỉ để minh hoạ. Milenage ở module 15 đã được kiểm chứng với test vector 3GPP TS 35.208.
- Các lab cần WSL2 Ubuntu 24.04 + Docker (module 00), kind và kubectl (module 14). Module 17 cần thêm `sudo apt install -y g++ cmake libssl-dev nlohmann-json3-dev clang`, rồi build bằng `cmake -S . -B build && cmake --build build && (cd build && ctest)` trong `~/netlab/m17/netsec`. File `fuzz/fuzz_gtpu.cpp` build riêng bằng clang (xem Lab 17.5).
- Code C++ của module 17 chưa được biên dịch khi soạn; các unit test dùng test vector chuẩn (RFC 4231, FIPS-197, 3GPP TS 35.208) để bạn tự kiểm chứng.
