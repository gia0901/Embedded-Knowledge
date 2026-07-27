# C++ Mindset — Ch. 7–12: Dự án xuyên suốt "The Curious Cases of the Rubber Duckies" (tr. 163–283) ✅ đã đọc

> Thuộc [The C++ Programmer's Mindset](README.md) · **[⏮ Ch. 6 — Modularity](modularity.md)** · **[Ch. 13–15 → Scale/GPU/Profiling](scale-gpu-profiling.md)**
> Nguồn: đọc trực tiếp PDF, **trang sách = trang PDF − 25**.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · **⚠️ = lỗi/điểm cần cẩn trọng** · trích dẫn kèm `(tr. X)`.
> **Đây là nửa sau của sách: một dự án THẬT chạy xuyên 6 chương** để thấy toàn bộ phương pháp (computational thinking + C++ + machine sympathy của Phần 1) vận hành trên bài toán đời thực. **Giá trị nằm ở *cách nghĩ/cách làm*, không phải từng dòng code** — nên file này lướt phần code lặp, đào sâu (a) quy trình tiếp cận bài toán, (b) các quyết định thiết kế đắt giá, (c) vài mảng kỹ thuật trúng phỏng vấn (interface/Factory, regex, k-means, error handling).

**Bài toán (tr. 164):** khắp nơi có báo cáo "vịt cao su" thần bí trôi dạt; khách hàng cần **một CLI** nhận tập file `.txt/.csv/.json` chứa các lần nhìn thấy, và in ra **toạ độ các "điểm nóng"** (nơi vịt tụ tập). Dữ liệu đến từ 3 nguồn: app (JSON, có cấu trúc nhất), web form (CSV), và email (free-text, ít cấu trúc nhất).

---

## Cụm 1 — Ch. 7: Phác thảo bài toán trước khi viết dòng code nào (tr. 163–173)

### Nội dung chính

Chương này **không viết code** — nó dạy **quy trình tiếp cận** một bài toán mở, đúng thứ hay bị bỏ qua và cũng đúng thứ interviewer senior kiểm tra ("cho một brief mơ hồ, bạn làm gì đầu tiên?").

**Bước 1 — Nhìn dữ liệu thật trước (tr. 164).** *"It's always a good idea, when it is possible, to look at the data and figure out what you're working with"* (tr. 164). Tác giả mở từng định dạng mẫu:
- **JSON**: `{latitude, longitude, date, description}` — nhiều cấu trúc, dễ nhất.
- **CSV**: cùng 4 trường **nhưng không có header** → phải suy thứ tự cột (biết toạ độ London để đoán).
- **Email free-text**: *"June 10 2024: ... on the Thames River in London (51 30' 26''N, 7' 39''W)"* — toạ độ ở dạng **độ/phút/giây**, ngày dạng tương đối ("yesterday"), có thể thiếu hẳn location → **khó nhất**.

→ Quan sát dữ liệu **lộ ngay các ca biên** (thiếu field, ngày tương đối, cùng một lần nhìn báo hai nguồn) mà brief không nói.

**Bước 2 — Tách thành các thách thức lớn (tr. 166):** (1) đọc nhiều định dạng; (2) gộp thành một dataset thống nhất; (3) tìm các "epicenter". Cộng thêm ẩn số: **trả kết quả cho user thế nào?** — brief chỉ nói "in ra terminal". Tác giả suy theo **triết lý Unix**: output nên **dùng được bởi tool khác** → in bảng lat/long thô (tr. 169). Bài học: *"sometimes the brief will change over time"* — hỏi lại khách khi mơ hồ.

**Bước 3 — Quyết định biểu diễn dữ liệu nội bộ SỚM (tr. 166–167)** — vì nó chi phối cả cách đọc file lẫn cách truyền dữ liệu giữa các tầng:

```cpp
struct Coordinates { float latitude; float longitude; };

// Cân nhắc SoA (tách mảng) — nhưng lat/long luôn dùng CÙNG nhau (tính khoảng cách)
// → để chung (AoS) tốt hơn cho cache; SoA chỉ đáng khi có metadata rời:
struct RubberDuckyData {
    std::vector<Coordinates>               coordinates;
    std::vector<std::chrono::year_month_day> dates;    // C++20 calendar
    std::vector<std::string>               descriptions;
};
```

**Quyết định `float` vs `double` (tr. 167) — câu trả lời "machine sympathy" đắt giá:** dữ liệu chỉ chính xác cỡ thành phố → độ chính xác thừa của `double` vô nghĩa; `float` **nửa kích thước → gấp đôi throughput** (nhiều số trên một cache line + một lệnh SIMD). Đánh đổi: tính khoảng cách kém chính xác chút, có thể ảnh hưởng hội tụ clustering — nhưng ở quy mô "khác biệt giữa các thành phố" thì không đáng lo. ⚠️ Sách nhắc: *"you will need to check that this is really the case in your own work"* (tr. 167) — đừng bê nguyên.

**Bước 4 — Phác `main()` để thấy khung (tr. 172):**
```cpp
int main(int argc, char** argv) {
    auto files    = parse_args_and_environment(argc, argv);
    auto data     = parse_data_files(files);
    auto clusters = do_clustering(data);
    present_results(clusters, data);
    return 0;
}
```
Mỗi hàm là một chương tiếp theo. Còn cân nhắc sớm: **expandability** (giữ metadata date/description phòng khi khách đổi ý), **multithreading** (cho phép chỉnh số thread — `-j`, không mặc định nuốt hết core: *"Using too many cores might even hurt performance"*, tr. 170), **error handling** (CLI không được crash vì một dòng text hỏng — bắt exception, log, exit code khác 0), **caching** (dữ liệu tiền xử lý — flatbuffers nếu chạy nhiều lần).

### Insight đáng nhớ

- **Trình tự vàng của chương: xem dữ liệu → liệt kê thách thức → chốt biểu diễn nội bộ → phác `main()` → nhận diện ca biên & tính mở rộng.** Đây chính là "decomposition + abstraction" (ch. 1) áp lên bài toán thật — trả lời phỏng vấn "cho brief mơ hồ này bạn bắt đầu từ đâu" theo trình tự này là ăn điểm.
- **Chốt kiểu dữ liệu nội bộ sớm** vì nó lan toả ngược lên tầng đọc file và xuôi xuống tầng thuật toán — đổi muộn là refactor toàn hệ.
- `float` vs `double` không phải "chính xác hơn thì tốt hơn" mà là **cân độ chính xác cần thiết với throughput** (2× dữ liệu/cache line, 2× lane SIMD) — tư duy Ch. 4 áp vào quyết định thực.

### Góc interview

**Câu 1 (🎯 tư duy):** Được giao một brief mơ hồ ("đọc mấy file lộn xộn này, tìm điểm nóng"), bạn làm gì *trước khi* code?

<details><summary>Đáp án</summary>

- **Xem dữ liệu thật trước** — mở từng định dạng mẫu, ghi lại cấu trúc + **ca biên** (CSV thiếu header, ngày tương đối "yesterday", field trống, khả năng trùng bản ghi). Ca biên lộ ra từ dữ liệu chứ không từ brief.
- **Chẻ thành thách thức lớn** (đọc đa định dạng / gộp dataset / tìm cluster / trình bày output) và **hỏi lại khách chỗ mơ hồ** (output format? cần date không?). Chấp nhận brief sẽ đổi.
- **Chốt biểu diễn dữ liệu nội bộ sớm** (struct chung) vì nó chi phối cả tầng đọc lẫn tầng thuật toán; kèm quyết định kiểu số (`float` đủ chính xác + nhanh gấp đôi).
- **Phác `main()`** thành các pha rõ (parse args → read → cluster → present) để thấy khung và chia việc.
- Nghĩ sớm về **cross-cutting concerns**: threading (cho chỉnh số thread), error handling (CLI không crash), expandability, caching.
- Điểm cộng: nêu **triết lý Unix** cho output (bảng thô để tool khác dùng) — chứng tỏ tư duy thiết kế công cụ, không chỉ code.

</details>

---

## Cụm 2 — Ch. 8: Command-Line Interface (tr. 175–190)

### Nội dung chính

**Thư viện dùng:** **Boost.program_options** (parse CLI) + **spdlog** (logging nhẹ) + **OpenMP** (threading) — tất cả tìm qua CMake `find_package`. Sách dùng vcpkg làm ví dụ quản dependency.

**`main()` chia thành các pha tách bạch (tr. 178)** — separation of concerns cho dễ bảo trì:
```cpp
int main(int argc, char* argv[]) {
    setup_signals();
    const auto args = parse_config(argc, argv);   // args + env + config → một variables_map
    setup_logging(args);
    setup_threading(args);
    run_and_report(args);
    return EXIT_SUCCESS;
}
```

**Định nghĩa options (tr. 179):** `-h/--help`, `--version`, `-v/--verbose`, **`-j/--jobs`** (số thread — theo quy ước `make`), và **positional args** = danh sách file:
```cpp
po::options_description options{"Options"};
options.add_options()
    ("help,h",    po::bool_switch(), "produce help message")
    ("verbose,v", po::bool_switch(), "print logging to terminal")
    ("jobs,j",    po::value<int>(),  "max threads to use")
    ("paths",     po::value<std::vector<std::filesystem::path>>(), "paths to process");
po::positional_options_description positional;
positional.add("paths", -1);   // không giới hạn số file
```

**Parse có phòng thủ (tr. 180):** gói trong `try/catch`, **cũng đọc environment variable** (`DUCKIES_` prefix) — cho phép cấu hình qua env; malformed → in synopsis + usage rồi `EXIT_FAILURE`.

### Insight đáng nhớ

- **`main()` mỏng, uỷ thác cho các hàm pha** (signals → config → logging → threading → run): mỗi pha đổi độc lập, thêm option không đụng pha khác — đây là "modularity" (ch. 6) ở cấp ứng dụng.
- Quy ước CLI **giống công cụ Unix quen thuộc** (`-h`, `-v`, `-j`, positional files, đọc cả env var) → user không phải học lại; *"don't count on users reading the long-form documentation"* (tr. 170) — option quan trọng phải lộ ở CLI.
- 🆕 Với embedded/tooling: cùng khung này (parse → setup → run → report + exit code có nghĩa) là chuẩn cho mọi CLI tool nội bộ; exit code khác 0 khi lỗi để script/CI bắt được.

---

## Cụm 3 — Ch. 9: Đọc dữ liệu đa định dạng — thiết kế INTERFACE (tr. 191–211) 🎯

### Nội dung chính

Chương này **về interface, không về CSV/JSON** — *"if CSV and JSON are so well understood, what is the challenge here? The answer is the interface"* (tr. 168). Mục tiêu: đọc **mọi định dạng vào MỘT cấu trúc chung**, dispatch đúng reader theo phần mở rộng file.

**Abstract base class `FileReader` (tr. 202):**
```cpp
class FileReader {
public:
    using Data   = RubberDuckyData;
    using FSPath = std::filesystem::path;
    virtual ~FileReader();
    virtual void read_file(Data& data, const FSPath& path) const = 0;       // đọc file → nạp vào data
    virtual std::string_view supported_file_extension() const noexcept = 0; // ".csv", ".json", ".txt"
};
```
Mỗi định dạng = một lớp con (`CSVReader`, `JSONReader`, `TextReader`) hiện thực 2 hàm ảo. **Dispatch = Factory theo extension:** tra bảng `extension → reader` rồi gọi `read_file`. Đây chính là **runtime polymorphism** (ch. 2) dùng đúng chỗ: tập định dạng **mở** (có thể thêm reader qua shared library), chọn ở runtime theo dữ liệu.

**Dùng thư viện + `mmap` (tr. 204):** CSV/JSON qua thư viện header-only (rapidcsv/rapidjson/simdjson…) — *"whenever the opportunity arises, one should use well-tested library code rather than writing your own"* (tr. 168); **memory-map** file để đọc nhanh. ⚠️ Lệch interface giữa các lib (một lib nhận `ifstream`, lib khác không) là ma sát thật (tr. 198).

**Xử lý lỗi mềm (tr. 204):** một file/dòng hỏng → `spdlog::warn` + **bỏ qua**, không ném ra làm chết cả chương trình:
```cpp
void CSVReader::read_file(Data& data, const FSPath& path) const {
    Reader csv;
    if (!csv.mmap(path)) { spdlog::warn("failed to parse csv from file \"{}\"", path.c_str()); return; }
    try { read_csv(data, csv); }
    catch (std::invalid_argument& e) { spdlog::warn(e.what()); }   // dòng lỗi → cảnh báo, tiếp tục
}
```

### Insight đáng nhớ

- **Cái khó không phải parse CSV/JSON mà là thiết kế interface chung** để mọi reader đổ vào một `Data` + dispatch đúng reader. Đây là ví dụ mẫu mực của **dynamic polymorphism dùng đúng chỗ** (tập định dạng mở, chọn ở runtime) — đối lập với template/concepts (tập đóng, biết lúc compile — ch. 2).
- **Factory theo extension** tách "biết định dạng nào" khỏi "cách đọc từng định dạng" → thêm định dạng = thêm một lớp con + đăng ký, không sửa code cũ (Open/Closed).
- Error handling của **thư viện** (ném exception) khác của **ứng dụng CLI** (log + tiếp tục/exit code): boundary giữa hai tầng phải chuyển đổi exception → log (nối [ch.1 failure vs error](thinking-computationally.md), [EMC++ noexcept](../effective-modern-cpp.md)).

### Góc interview

**Câu 1 (🎯):** Cần đọc nhiều định dạng file (CSV/JSON/XML/…) vào cùng một cấu trúc. Thiết kế thế nào để dễ thêm định dạng mới?

<details><summary>Đáp án</summary>

- **Abstract base class** (`FileReader`) với hàm ảo `read_file(Data&, path)` + `supported_extension()`; mỗi định dạng một lớp con. **Factory/registry** ánh xạ `extension → reader`, dispatch ở runtime theo đuôi file. Thêm định dạng = thêm lớp con + đăng ký, **không sửa code hiện có** (Open/Closed principle).
- Vì sao **runtime polymorphism** (virtual) chứ không template/concepts: tập định dạng **mở và chọn theo dữ liệu lúc chạy** (đuôi file chỉ biết ở runtime), thậm chí nạp reader từ **plugin/shared library** — đúng ca dùng dynamic dispatch. Template hợp khi tập đóng, biết lúc compile.
- **Interface `Data` chung** quyết định sớm (Cụm 1) để mọi reader đổ vào; dùng **thư viện đã kiểm thử** cho từng định dạng thay vì tự parse; **mmap** cho tốc độ.
- **Error handling**: reader không ném ra ngoài — log cảnh báo + bỏ dòng/file hỏng, để CLI chạy tiếp và trả exit code có nghĩa. Ranh giới library (exception) ↔ application (log).
- Bẫy: nhét mọi format vào một hàm với `if/else` theo đuôi — vi phạm Open/Closed, phình dần, khó test từng reader.

</details>

---

## Cụm 4 — Ch. 10: Tìm thông tin trong text tự do — Regex (tr. 213–235)

### Nội dung chính

Thách thức khó nhất: trích **ngày + toạ độ** từ email free-text (toạ độ dạng độ/phút/giây, ngày dạng đủ kiểu). Công cụ: **`std::regex`**.

- **Named capture groups** cho từng thành phần ngày/toạ độ; **non-capture group** `(?:...)` cho phần gom mà không muốn bắt (tr. 208) — sách kể **quá trình thử–sai**: dùng repeated names gây lỗi phải đổi cách, dùng nhóm bắt nhầm gây bug phát hiện khi test (tr. 210). Bài học thật: *regex đúng phải viết cùng test, không phải viết một phát ăn ngay*.
- Toạ độ độ/phút/giây: mỗi số trong named group, độ và phút bọc non-capture group để ghép mẫu (tr. 208, 715). ⚠️ Raw string `R"(...)"` để khỏi escape rừng backslash.
- ⚠️ **Regex đắt**: `std::regex` nổi tiếng chậm; với khối lượng lớn nên **compile mẫu một lần** (không tạo lại `std::regex` mỗi dòng), và cân nhắc parser tay/`std::from_chars` khi hot path.

### Insight đáng nhớ

- **Regex là "code" — phải test như code.** Sách trung thực kể các bug (repeated named groups, capture nhầm) chỉ lộ khi test → quy tắc: mẫu regex phải đi kèm bộ test ca biên (thiếu field, định dạng lạ, chuỗi không khớp).
- **Tách phần khó ra chương riêng** (ch. 10) *sau khi* đã giải xong thiết kế interface (ch. 9): *"we will have already solved all the interface design problems and can focus on the task at hand"* (tr. 168) — chiến lược chia-để-trị ở cấp dự án.
- 🆕 `std::regex` chậm là điều nên biết khi phỏng vấn hiệu năng: text-heavy hot path thường thay bằng `std::from_chars`, RE2, hoặc parser thủ công.

---

## Cụm 5 — Ch. 11: Clustering dữ liệu — k-means (tr. 237–271) 🎯

### Nội dung chính

Bài toán lõi: rút tập lớn toạ độ về ít "điểm nóng". Từ khoá **clustered** → chọn họ **clustering algorithm**; sách dùng **k-means** (thuật toán Lloyd).

**Thuật toán k-means (tr. 238):**
```
1. Gán ngẫu nhiên mỗi điểm vào một trong k cụm.
2. Lặp đến khi các cụm KHÔNG đổi nữa:
   (a) Tính mean (centroid) μ_i của mỗi cụm.
   (b) Gán lại mỗi điểm vào cụm có centroid GẦN nhất.
```
- **Hội tụ nhưng dễ kẹt local minimum** (tr. 238): kết quả phụ thuộc khởi tạo ngẫu nhiên → **chạy nhiều lần với init khác nhau**, giữ kết quả tốt nhất (within-cluster sum of squares nhỏ nhất). Đây là **randomized algorithm** (ch. 3).
- **Embed toạ độ cầu vào không gian thực (tr. 238):** dữ liệu sống trên mặt cầu (Trái Đất), k-means cần không gian ℝ^d → **chuyển lat/long → vector 3D** (điểm trên mặt cầu). Với quy mô thành phố, sai khác giữa khoảng cách geodesic và Euclid 3D là bỏ qua được.
- **Chọn k bằng silhouette score (tr. 239):** đo độ "chặt" của cụm so với cụm gần nhất; ⚠️ **tốn O(N²)** — giảm bằng cách dùng khoảng cách tới centroid thay vì mọi cặp điểm. ⚠️ Sách còn thêm penalty cho cụm rỗng/một-điểm (tr. 10709) — silhouette hoạt động trên test nhưng *"by no means guaranteed"* (tr. 10640).

### Insight đáng nhớ

- **Bắt đầu bằng thuật toán chuẩn có thư viện tốt trước khi tự chế** (tr. 168): *"it is always best to start with standard, well-known algorithms ... before moving into bespoke technology"*. Sách tự viết k-means chỉ để dạy — thực chiến nên dùng thư viện.
- **k-means = randomized + iterative**: kẹt local minimum → nhiều restart; đây là ứng dụng cụ thể của "randomized algorithm" và "iteration đến hội tụ" ở ch. 3.
- **Embedding đúng không gian là bước tư duy dễ quên**: dữ liệu cầu không dùng thẳng được cho thuật toán giả định không gian phẳng — phải ánh xạ có ý nghĩa (lat/long → 3D) trước.

### Góc interview

**Câu 1 (🎯):** Giải thích k-means. Vì sao kết quả mỗi lần chạy khác nhau, và xử lý thế nào? Chọn k ra sao?

<details><summary>Đáp án</summary>

- **k-means (Lloyd):** khởi tạo k centroid (hoặc gán ngẫu nhiên) → lặp {gán mỗi điểm về centroid gần nhất; tính lại centroid = mean của cụm} đến khi không đổi. Tối thiểu hoá **within-cluster sum of squares**.
- **Khác nhau mỗi lần** vì phụ thuộc **khởi tạo ngẫu nhiên** và chỉ đảm bảo **local minimum**, không phải global. Xử lý: **chạy nhiều lần với init khác nhau**, giữ kết quả tốt nhất; hoặc dùng init thông minh (**k-means++**).
- **Chọn k**: không có k "đúng" tuyệt đối — dùng chỉ số như **silhouette score** (hoặc elbow method) quét vài giá trị k; lưu ý silhouette đầy đủ là **O(N²)** nên xấp xỉ bằng khoảng cách tới centroid, và thêm penalty cho cụm rỗng.
- Điểm cộng: nêu **độ phức tạp** O(N·k·d·iterations), giả định cụm **lồi/kích thước tương đương** (k-means kém với cụm hình dạng bất thường → cân nhắc DBSCAN/hierarchical); và với **dữ liệu địa lý phải embedding** lat/long → không gian phẳng/3D trước khi tính khoảng cách.

</details>

---

## Cụm 6 — Ch. 12: Nhìn lại những gì đã xây (tr. 273–283)

### Nội dung chính

Ghép mọi mảnh, chạy trên dữ liệu mẫu (`duckies samples/test.csv test.json test.txt`), test rằng tìm đúng số cụm kỳ vọng. Nửa sau chương là **reflection có hệ thống** — *"examine the good parts, the ugly parts, and the things we could have done better"* (tr. 273). Tác giả nhấn: *"You should always try to set aside some time to reflect, especially if things don't go to plan"* (tr. 274).

Vài điểm tự phê: silhouette không đảm bảo chọn đúng k trên mọi dữ liệu; với dữ liệu địa lý một **cách tiếp cận phân cấp (hierarchical)** có thể tốt hơn (tr. 10669); một số quyết định (giữ metadata, cách cache) có thể làm khác.

### Insight đáng nhớ

- **Reflection là một pha bắt buộc của giải quyết vấn đề, không phải tuỳ chọn** — ngay cả khi bài học "đến quá muộn cho bài toán này", nó nâng lần sau (*"at least the lessons can be taken"*, tr. 10672). Đây là mắt xích khép vòng lặp phương pháp của cả cuốn sách.
- **Test toàn pipeline trên dữ liệu có đáp án biết trước** (dữ liệu tổng hợp 2 cụm rõ) là cách kiểm thử một hệ thống nhiều tầng mà từng tầng khó test riêng.

---

## Tổng kết Phần 2 — phương pháp chạy trên bài toán thật

Sáu chương này là **một vòng lặp computational thinking hoàn chỉnh**: *xem dữ liệu → chẻ bài toán → chốt abstraction (Data chung) → nhận diện pattern (đây là clustering) → thiết kế thuật toán (k-means) → hiện thực từng tầng với interface sạch (Factory readers, regex parser) → ghép + test + **reflect***. Mỗi công cụ của Phần 1 xuất hiện đúng chỗ: static/dynamic polymorphism (ch. 2) ở readers, độ phức tạp & randomized (ch. 3) ở k-means, machine sympathy (ch. 4) ở quyết định `float`/AoS, cấu trúc dữ liệu (ch. 5) ở `RubberDuckyData`, modularity (ch. 6) ở tách library + CLI mỏng.

> 🆕 Với người ôn phỏng vấn: giá trị lớn nhất của phần này **không phải** chi tiết k-means hay cú pháp Boost.program_options, mà là **kịch bản trả lời "cho một bài toán mở, quy trình của bạn là gì"** — kể được trình tự 7 bước trên, có ví dụ cụ thể ở mỗi bước, là tín hiệu kỹ sư trưởng thành.

---

## Đọc thêm (tùy chọn — nội dung trên đã tự chứa)

- [thinking-computationally.md](thinking-computationally.md) — 4 thành phần computational thinking (bộ khung mà dự án này hiện thực hoá).
- [abstraction-in-detail.md](abstraction-in-detail.md) — static vs dynamic polymorphism (nền của Factory readers ở Cụm 3).
- [10-thinking/problem-solving.md](../../10-thinking/problem-solving.md) — phương pháp giải quyết vấn đề của repo.

**Chương tiếp theo:** [Ch. 13–15 — Scale, GPU & Profiling →](scale-gpu-profiling.md) (mở rộng tính toán: threading/IPC/bottleneck/cloud; GPU CUDA/Thrust/SYCL; profiling với perf).
