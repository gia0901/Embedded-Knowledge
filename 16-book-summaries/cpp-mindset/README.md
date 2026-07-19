# The C++ Programmer's Mindset — Sam Morley (Packt, 1st edition, 11/2025)

> **Nguồn summary: ĐỌC TRỰC TIẾP TỪ PDF** `/home/gia/Desktop/Books/THE_C_PROGRAMMERS_MINDSET.pdf` (ISBN 978-1-83588-842-1). Sách xuất bản **sau knowledge cutoff của Claude** → không có kiến thức nền về sách này; mọi nội dung đều từ việc đọc PDF. Số trang `(tr. X)` theo số trang in của sách (trang PDF = trang sách + 25).
> **Vì sao đọc cuốn này:** đúng đánh giá của người học — sách dạy **cách nghĩ** (computational thinking) trước, C++ chỉ là phương tiện hiện thực hóa; rất hợp mục tiêu dài hạn "tư duy cốt lõi thay vì kiến thức bề nổi" của repo, và phần "hiểu máy" (ch. 4) + cấu trúc dữ liệu thực dụng (ch. 5) giao thoa trực tiếp với nghề embedded. Tác giả: research software engineer tại Oxford, dân toán, viết thư viện HPC C++/CUDA.
> **Quy ước:** nội dung không đánh dấu = từ sách; **🆕 = tôi bổ sung/liên hệ ngoài sách**; **⚠️ = điểm cần lưu ý/cân nhắc**.

---

## TL;DR — cuốn sách này dạy gì

Giải quyết vấn đề là một **quy trình lặp có phương pháp**, không phải cảm hứng. Sách dựng phương pháp đó quanh **4 thành phần của computational thinking** — decomposition (chẻ nhỏ), abstraction (trừu tượng hóa), pattern recognition (nhận diện mẫu), algorithm design — rồi lần lượt: nối chúng vào các cơ chế trừu tượng của C++ (hàm/class/template/concepts, ch. 2), vào phân tích thuật toán (ch. 3), vào **phần cứng thật** (cache, SIMD, branch prediction — ch. 4), vào lựa chọn cấu trúc dữ liệu (ch. 5) và tổ chức code thành thư viện (ch. 6). Nửa sau sách là **một dự án xuyên suốt** ("the curious cases of the rubber duckies" — phân tích dữ liệu phao nhựa trôi dạt: CLI, đọc CSV/JSON, regex, k-means clustering) để thấy toàn bộ phương pháp chạy trên bài toán thật, và khép lại bằng scale (threading/IPC), GPU (CUDA/SYCL), profiling (perf).

## 🕐 Đọc gì nếu chỉ có N giờ

- **~2 giờ:** Ch. 1 (khung tư duy — [thinking-computationally.md](thinking-computationally.md)) + lướt ch. 12 (bài học rút ra từ dự án).
- **~5 giờ:** thêm ch. 4 (Understanding the Machine) + ch. 5 (Data Structures) — phần "machine sympathy" giá trị nhất với embedded.
- **~8 giờ:** thêm ch. 2–3 (abstraction + complexity) và ch. 15 (perf).

## 🎯 Lộ trình ôn nhanh trước interview

Sách này **không phải sách luyện interview** — giá trị chính là nâng nền tư duy. Tuy vậy các phần sau chạm thẳng câu hỏi phỏng vấn:
1. Ch. 4 — cache line, SoA vs AoS, SIMD, branch prediction (trùng và bổ sung góc C++ cho [ostep/virtualization-memory.md](../ostep/virtualization-memory.md)).
2. Ch. 5 — chọn cấu trúc dữ liệu theo bộ nhớ: flat map, small vector, stable vector (bổ sung mạnh cho topic [13-dsa](../../13-dsa/)).
3. Ch. 6 — static/shared lib, export symbol, **stable interface/ABI** (trùng topic [07-shared-libraries](../../07-shared-libraries/) — đúng nghề hiện tại của bạn).
4. Ch. 1 — failure vs error, `std::expected`, concepts (câu hỏi C++ hiện đại — bổ sung C++20/23 cho [EMC++](../effective-modern-cpp.md) vốn dừng ở C++14).
5. Ch. 15 — perf stat/record/report (trùng [09/tools.md](../../09-debugging/tools.md), thêm góc benchmark function).

## 🗺️ Bản đồ: chương ↔ file ↔ trạng thái

| File | Chương (trang) | Nội dung | Trạng thái |
|------|----------------|----------|------------|
| [thinking-computationally.md](thinking-computationally.md) | 1 (tr. 1–35) | 4 thành phần computational thinking; modern C++ good practice (CMake, ranges, concepts, expected, testing) | ✅ **Đã đọc sâu toàn văn** |
| [abstraction-in-detail.md](abstraction-in-detail.md) | 2 (tr. 37–60) | 4 loại bài toán; standard algorithms (ví dụ grid signal); functions/classes/templates-concepts-traits — khi nào dùng gì; static vs dynamic polymorphism | ✅ **Đã đọc sâu toàn văn** (kèm 2 lỗi in của sách được đánh dấu ⚠️) |
| algorithmic-complexity.md | 3 (tr. 61–81) | Big-O, amortized, họ thuật toán, cache-aware/SIMD/branch | ⬜ Chờ đọc |
| machine-data-structures.md | 4 (tr. 83–118), 5 (tr. 119–143) | CPU/cache line/SIMD/speculation; VM & scheduler; allocator, SoA-AoS, small/static vector, span, flat & hash map | ⬜ Chờ đọc (**ưu tiên kế** — giá trị embedded cao nhất) |
| modularity.md | 6 (tr. 145–161) | Static/shared lib, export symbols, stable interface, memory qua boundary, C++ modules | ⬜ Chờ đọc |
| project-rubber-duckies.md | 7–12 (tr. 163–283) | Dự án xuyên suốt: CLI, CSV/JSON, regex parser, k-means, reflection | ⬜ Chờ đọc (sẽ lướt có chủ đích — giá trị nằm ở *cách làm*, không phải chi tiết code) |
| scale-gpu-profiling.md | 13–15 (tr. 285–350) | Mutex/IPC/bottlenecks/cloud; GPU CUDA/Thrust/SYCL; perf | ⬜ Chờ đọc (GPU lướt; perf kỹ) |

## Vị trí trong repo — tránh trùng lặp

Sách giao thoa nhiều topic sẵn có; summary sẽ **nghiêng về phần tư duy/cách nghĩ** (thứ repo chưa có nhiều) và link sang tài liệu sâu khi trùng: ch. 4 ↔ [OSTEP](../ostep/README.md); ch. 5 ↔ [13-dsa](../../13-dsa/); ch. 6 ↔ [07-shared-libraries](../../07-shared-libraries/); ch. 1 phần good practice ↔ [02-modern-cpp](../../02-modern-cpp/), [06-build-systems](../../06-build-systems/); tư duy tổng thể ↔ [10-thinking](../../10-thinking/problem-solving.md).
