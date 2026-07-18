# C++ Mindset — Ch. 1: Thinking Computationally (tr. 1–35) ✅ đã đọc sâu toàn văn

> Thuộc [The C++ Programmer's Mindset](README.md). Nguồn: đọc trực tiếp PDF, trang sách = trang PDF − 25.

---

## Cụm 1 — Bốn thành phần của computational thinking (tr. 2–8)

### Nội dung chính

**Định nghĩa nền trước đã (tr. 2):** trong sách này, một "problem" = **một nhiệm vụ cụ thể với ràng buộc (constraints) và tham số (parameters) được xác định rõ**. Ràng buộc có thể thuộc quản lý (deadline), kỹ thuật (hiệu năng), hay đặc thù lĩnh vực (giới hạn vật lý); tham số là dữ liệu/thông tin được cấp để giải. Nhiệm vụ mơ hồ thì chưa phải problem — phải làm rõ trước.

**Bốn thành phần (tr. 3):**

1. **Decomposition** — chẻ bài toán lớn thành các bài nhỏ dễ xử lý hơn.
2. **Abstraction** — rút ra cái nhìn tổng quát, giữ phần cốt lõi, bỏ nhiễu.
3. **Pattern recognition** — nhận ra bài toán con là "ca" của một lớp bài đã có lời giải chuẩn.
4. **Algorithm design** — thiết kế chuỗi bước cụ thể giải từng phần rồi ghép lại.

Ba cảnh báo quan trọng của tác giả (tr. 3–4) — đây là phần "mindset" thật sự:
- Bốn thành phần **không phải quy trình tuần tự bước 1-2-3-4**; chúng đan xen và phải cân nhắc đồng thời (không thể decompose nếu chưa nhận ra cấu trúc; không thể abstraction nếu chưa hiểu thành phần cốt lõi).
- Giải bài toán là quy trình **lặp và động**: sẽ có nhiều đường đi thất bại — đó là *một phần tất yếu*, việc cần làm là hiểu vì sao hướng đó fail và mang bài học sang vòng lặp sau; đôi khi phải đập đi làm lại từ đầu.
- Lựa chọn sớm ảnh hưởng mọi thứ sau: ví dụ **chọn ngôn ngữ** — giải bằng Python thì nghiêng về ghép thư viện có sẵn; bằng C++ thì có tự do (và trách nhiệm) tự viết primitive hiệu năng cao (tr. 3).

**Lời khuyên chung (tr. 4):** (1) **giữ một sổ tay pattern/abstraction** để tra lại — kinh nghiệm sẽ nội hóa dần nhưng sổ vẫn quý (🆕 chính là vai trò của repo Embedded-Interview này); (2) luyện bài nhỏ có chủ đích (LeetCode) để tích lũy kho pattern — bài không cần giống hệt, chỉ cần "đủ gần để gợi ý"; (3) nói chuyện với đồng nghiệp — hoặc **con vịt cao su** (rubber duck debugging: giải thích to bài toán cho một vật vô tri, thường tự vỡ ra lời giải).

**Ví dụ "quá toán" có chủ đích (tr. 5–8):** tác giả giải một phương trình vi phân (y″ − 3y′ + 2y = x² + x) — *không cần hiểu toán*, cái cần thấy là **quy trình**: bài lớn không giải thẳng được → chẻ đôi (nghiệm tổng quát + nghiệm riêng "dịch chuyển") → trong mỗi nhánh liên tục **nhận diện mẫu quen** (phương trình bậc hai → tìm nghiệm; hệ phương trình tuyến tính → giải chuẩn; so sánh hệ số) → mỗi mẫu quy về bài con đã biết cách giải → **ghép lại** — và tác giả nhấn: bước ghép lại là "crucial và thường không tầm thường" khi các phần phụ thuộc nhau (tr. 7). Bài học nêu tường minh: *decomposition làm mỗi bước cần ÍT kiến thức hơn hẳn bài gốc* (tr. 7).

### Insight đáng nhớ

- Câu định vị cả cuốn sách: **"bốn thành phần không phải checklist — chúng là bốn góc nhìn phải giữ đồng thời."** Người mới hay hỏi "bước 1 làm gì?"; câu trả lời của sách: không có bước 1 cố định, có bốn câu hỏi thường trực (chẻ được không? trừu tượng hóa được gì? giống bài nào đã gặp? thuật toán cho từng mảnh?).
- 🆕 Liên hệ repo: đây chính là khung mà [10-thinking/problem-solving.md](../../10-thinking/problem-solving.md) hướng tới — sách này cho bản đầy đủ và có hệ thống hơn; hai tài liệu nên đọc cùng nhau.

---

## Cụm 2 — Decomposition: chẻ bài toán thế nào (tr. 8–12)

### Nội dung chính

**Vì sao người ta không chịu chẻ (tr. 8):** hai lý do phổ biến — người mới **thiếu tự tin** mình chẻ đúng; người có kinh nghiệm hơn thì **sợ mất bức tranh lớn**. Cả hai đều thật: phải cân bằng giữa tìm đường đi khả thi và không lạc mất bài toán gốc (khách hàng không nhận phần mềm giải "một khía cạnh").

**Hai loại phụ thuộc giữa các bài con (tr. 9)** — thuật ngữ đáng giữ:
- **Temporal dependency** (phụ thuộc thời gian): bài A phải xong *trước* bài B.
- **Technical dependency** (phụ thuộc kỹ thuật): *lời giải* của A phụ thuộc/được định hình bởi *lời giải* của B (không phải kết quả của B).

**Nguyên tắc "chẻ tối thiểu" (tr. 9):** mỗi tầng decomposition **tăng tổng độ phức tạp** (nhiều mảnh hơn phải ghép lại) → chẻ vừa đủ để giải được, đừng chẻ vụn; đừng sa vào chi tiết mà lạc mất bài lớn. Khi bài toán có sẵn cách chẻ hiển nhiên → gần như luôn đi theo nó.

**Thí nghiệm tư duy bữa tối (tr. 9–10):** kế hoạch một bữa tối = starter + main + dessert + drinks — các phần liên kết *yếu* (rượu phụ thuộc món chính = technical dependency yếu; món khai vị dọn trước = temporal). Chiến lược: **giải phần ràng buộc nhất trước** (chọn main course) rồi các phần khác xoay quanh nó → giảm xác suất phải quay lui (backtracking). 🆕 Đây là heuristic tổng quát đáng nhớ: *most-constrained-first*.

**Khung 3 phần của gần như mọi bài toán lập trình (tr. 10)** — đơn giản mà dùng được ngay:

```
[1] Thu nhận dữ liệu (acquire + chuyển về dạng dùng được)
[2] Làm việc chính (the actual work)
[3] Hậu xử lý + trả kết quả cho người dùng
```

Câu hỏi mồi để gom bài con (tr. 10): phần nào **khó nhất**? cần **thông tin gì** cho phần đó? có cần **tiền xử lý** dữ liệu trước khi dùng? có cần **hậu xử lý** kết quả trước khi trả?

**Ví dụ xuyên suốt: chênh lệch nhiệt độ trung bình năm (tr. 10–12).** Bài: từ database nhiệt độ theo vùng, tính mức thay đổi trung bình năm-qua-năm mỗi vùng. Chuỗi tinh chỉnh lời giải — đọc kỹ vì nó minh họa "lặp và động":
1. Ngây thơ: trung bình cả năm rồi trừ nhau → **mất sạch biến động mùa** trong năm.
2. Tốt hơn: trừ **các giá trị tương ứng cùng thời điểm** giữa hai năm liên tiếp, rồi trung bình các hiệu đó.
3. Tốt hơn nữa (đủ dữ liệu): làm mượt bằng **trung bình cửa sổ trượt** quanh mỗi thời điểm để khử nhiễu thời tiết ngắn hạn, rồi mới so năm-với-năm.
- Cộng thêm bài học **đơn vị đo** (tr. 11): Celsius/Fahrenheit lẫn lộn từng gây thảm họa thật — chuẩn hóa (về Kelvin) **trước khi** mọi tính toán = việc của tầng tiền xử lý.
- Decomposition cuối (Figure 1.1, tr. 12): `Data → Preprocess → Sliding-window averages → Annual differences → Average per region` — và nhận xét quan trọng: các cửa sổ trượt **độc lập nhau → ứng viên song song hóa**; sách dừng đúng lúc: "mỗi bài con đã khả thi, không cần chẻ tiếp — nhưng cửa vẫn mở nếu cần" (tr. 12).

### Insight đáng nhớ

- **"Chẻ tối thiểu + giải phần ràng buộc nhất trước + luôn hỏi 3 tầng (vào–làm–ra)"** — ba heuristic gói cả cụm, áp được ngay vào mọi task hằng ngày (kể cả thiết kế driver: acquire = đọc phần cứng, work = xử lý, return = báo lên tầng trên).
- Ví dụ nhiệt độ cho thấy **định nghĩa bài toán tinh chỉnh dần qua các lời giải hỏng** — "lời giải ngây thơ sai ở đâu" chính là cách khám phá yêu cầu thật. 🆕 Trong interview system design, trình bày theo mạch "naive → vì sao thiếu → tinh chỉnh" như vậy ăn điểm hơn nhảy thẳng vào đáp án cuối.

---

## Cụm 3 — Abstraction & Pattern recognition (tr. 12–17)

### Nội dung chính

**Abstraction là gì (tr. 12):** bài toán thật thì "bừa bộn" — thừa thông tin, nhiễu. Abstraction = **cái nhìn tổng quát hóa chỉ giữ thông tin và tính chất cốt lõi**. Xây tốn công nhưng lãi lớn: một abstraction tốt áp được cho **cả một lớp bài toán** và thành công cụ lâu dài. Lập trình viên dùng abstraction mọi lúc mà không để ý: hàm, class, bộ nhớ hệ thống, file system đều là abstraction (tr. 13).

**Ba chỗ abstraction "ăn tiền" nhất (tr. 13):**
1. **Cửa nhận dữ liệu:** stdin? file? GUI? hay API thư viện cho dev khác? — một abstraction tốt ở cửa vào giúp *một* thiết kế phục vụ được mọi nguồn (🆕 đây chính là mầm của bài "file reader + strategy pattern" mà dự án ch. 9 sẽ làm thật).
2. **Chuẩn hóa dữ liệu vào đúng yêu cầu:** lại chuyện đơn vị đo — abstraction ở interface cho phép làm việc với dữ liệu *đã đúng đơn vị* bất kể nguồn.
3. **Thiết kế thuật toán:** nếu dữ liệu nhúng được vào một **cấu trúc toán học** (vd vector space) thì thừa hưởng cả kho thao tác + **bảo đảm đã được chứng minh** + các cài đặt tối ưu sẵn có.

**Nhận diện điểm chung trong DỮ LIỆU (tr. 13–14):** đặt câu hỏi về *tính chất trừu tượng* thay vì dạng cụ thể: dữ liệu số → có **thứ tự tự nhiên**, có `==`, có số học (+−×÷) — bài toán có khi chỉ cần "so sánh được và có thứ tự" (search/sort) chứ không cần "là số". Text → chuỗi ký tự trên một "bảng chữ cái" (ASCII 1 byte, UTF-8 đa byte) — quan trọng là biết tách/so sánh ký tự, không phải bảng mã nào. Tổng quát: **nhìn dữ liệu qua bộ traits** — ordered? equality-comparable? iterable? (🆕 chính là tư duy dẫn thẳng đến C++ **concepts** ở cụm 6 — sách cố ý gieo mầm ở đây.) Có khi phải giải bản cụ thể xong mới thấy đường tổng quát hóa; có bài không tổng quát hóa được — đều bình thường (tr. 14).

**Nhận diện cấu trúc trong BÀI TOÁN (tr. 14–15):** đôi khi cái trừu tượng hóa được không phải dữ liệu mà là **chính bài toán** — nhận ra một phần bài là "ca" của một khung lý thuyết. Ví dụ đắt: tìm pattern trong text → regex → và abstraction thật sự phía sau regex là **DFA (deterministic finite automata)**; nhận ra rồi thì cả kho lý thuyết automata/state machine dùng được, và biết soi tiếp phần nào khác của bài cũng khớp mô hình đó. Nắm các lý thuyết nền (regex/automata, sort/search, giải hệ tuyến tính, tối ưu có ràng buộc, đồ thị/shortest path) giúp "nhìn phát ra ngay" (tr. 15).

**Digression Sudoku + cờ (tr. 15–16):** Sudoku khó đòi nhận diện mẫu phức hợp như "X-wing" (cặp hàng mà một chữ số chỉ còn đúng một cặp cột khả dĩ → loại chữ số đó khỏi phần còn lại của hai cột); cờ vua cũng vậy — **chơi giỏi = kho mẫu lớn + biết dùng**. "Giải bài toán không khác gì: lập trình viên hiệu quả phải học các mẫu phổ biến, công dụng của chúng, và cách lắp vào lời giải" (tr. 16).

**Danh mục mẫu — hai họ (tr. 16–17):**
- **Functional patterns** (mẫu chức năng — *cái bài toán cần làm*): sort, search (theo predicate), filter, random sampling, shuffle, permutation; các bài quy về **số học** (hệ phương trình, tối ưu ràng buộc); **thống kê** (cứ có bất định trong đo đạc là cần); **combinatorics/đồ thị** — ba ví dụ ăn tiền: dependency resolution = đồ thị có hướng, **scheduling = graph coloring**, route planning = shortest path/traversal.
- **Structural patterns** (mẫu cấu trúc — *tổ chức lời giải thế nào*, xuất hiện ở khâu thiết kế/cài đặt): sách dùng nghĩa RỘNG hơn GoF (tr. 17). Nêu tên: **strategy/policy** (đóng gói họ phương pháp thay thế nhau — ví dụ của sách: các cách đọc file theo định dạng, đổi strategy theo file type — sẽ dùng nhiều trong dự án); adapter, facade, flyweight, proxy (họ "đổi giao diện cho hợp ngữ cảnh"); decorator (bọc để mở rộng); và note riêng: **template method** — "khung thuật toán với các bước placeholder tùy biến được" — chính là linh hồn của header `<algorithm>` trong STL.

### Insight đáng nhớ

- Cặp phân loại **functional (bài cần gì) / structural (code tổ chức sao)** là cách xếp kho pattern gọn hơn hẳn học vẹt danh sách GoF — và câu "STL `<algorithm>` = template method pattern ở quy mô công nghiệp" (tr. 17) là góc nhìn đáng nhớ về thứ bạn dùng hằng ngày.
- 🆕 Đối chiếu repo: [12-design-patterns](../../12-design-patterns/) đã có chi tiết từng pattern; giá trị thêm của sách là **đặt pattern vào quy trình giải bài** (nhận diện *khi nào* cần nó) thay vì trình bày pattern như danh mục.

---

## Cụm 4 — Algorithm: đọc, viết, và tính chất (tr. 17–25)

### Nội dung chính

**Định nghĩa chuẩn (tr. 17):** algorithm = tập chỉ dẫn nhận dữ liệu thỏa **preconditions** (điều kiện tiên quyết) và sinh output thỏa **postconditions**. Viết bằng **pseudocode** — ngôn ngữ trung lập ai biết lập trình cơ bản đều đọc được.

**Hai mô hình tính toán (tr. 18)** — nền lý thuyết đáng biết: **sequential** (máy Turing — làm từng bước từ đầu đến cuối) và **functional** (lambda calculus — giải bằng các lời gọi đệ quy), hai mô hình **tương đương** về năng lực. Ngôn ngữ thực tế pha trộn: C++ chủ đạo sequential nhưng **template của C++ là functional**; Haskell thuần functional. Nên hiểu cả hai cách vận hành.

**Ví dụ 1 — max của list (tr. 18–21):** pseudocode (INPUT/OUTPUT khai pre/postconditions, `<-` là gán) → bản C++ template nhận `Container` tổng quát. Các điểm bình luận của sách đáng giữ:
- Container rỗng: max **không định nghĩa** → hai cách "đúng": `throw std::invalid_argument`, hoặc đổi kiểu trả về **`optional<...>`** — cách sau còn cho phép đánh dấu hàm `noexcept` (giảm chi phí runtime — 🆕 nối [EMC++ Item 14](../effective-modern-cpp.md)). Thực tế thì **dùng `std::max_element` có sẵn** — bài này chỉ để học (tr. 20).
- Nên **chú thích code theo từng bước thuật toán** — người sau (gồm chính mình) đối chiếu được cài đặt với thiết kế (tr. 20).
- **5 tính chất của một algorithm đúng nghĩa (tr. 20–21):** finiteness (kết thúc hữu hạn), definiteness (bước mô tả chính xác không mơ hồ), inputs (thuộc tập xác định bởi preconditions), outputs, effectiveness (mỗi bước đủ cơ bản để làm được bằng giấy bút). Chứng minh bằng quy nạp; preconditions nên được **kiểm tra trong cài đặt** — ngầm qua static type hoặc tường minh qua điều kiện (tr. 21).

**Ví dụ 2 — đệ quy: tìm cuối "word" đầu tiên (tr. 22–25).** Grammar tí hon: `word ::= 'a'|'b' | '[' word ',' word ']'` (từ = chữ đơn, hoặc cặp từ trong ngoặc — **đệ quy trong định nghĩa** → thuật toán đệ quy là tự nhiên). Bản C++ dùng `std::string_view` (C++17 — cách đúng để làm việc với chuỗi **không sở hữu**, thay `const char*`: an toàn biên + có sẵn `substr/starts_with`), đánh `noexcept`, và **`assert` tại các điểm bất biến** (dịch comment của pseudocode thành assertion — tr. 24). Điểm thiết kế: hàm **không check lỗi đầu vào** ngoài assert — precondition đặt lên caller, hợp lý trên "hot path" nơi check tốn kém (tr. 24).

**Đệ quy vs lặp (tr. 24–25)** — trade-off được nói thẳng:
- Viết đệ quy thường **dễ hơn**, nhưng C++ "không được thiết kế để chạy kiểu đó": lời gọi hàm đắt; compiler có thể inline/tail-call-optimize nhưng **độ sâu không biết trước lúc compile thì hết cửa**.
- Bản lặp (đếm depth theo `[`/`]`) **khó suy luận hơn và khó mở rộng hơn** (muốn làm thêm việc gì khác ngoài "tìm vị trí cuối" là bí).
- Kết luận nguyên văn tinh thần (tr. 25): flexibility vs performance là dằng xé thường trực; nếu chắc chắn không cần mở rộng thì cứ tối ưu; nhưng **"không bao giờ tối ưu trước khi ĐO thấy chậm — measure twice, cut once."**

### Insight đáng nhớ

- Bộ ba **precondition → assert → noexcept** trong ví dụ parser là một bài mini về **thiết kế hợp đồng hàm**: điều kiện thuộc về ai (caller), kiểm tra bằng gì (assert — biến mất ở release), và cam kết gì (noexcept). 🆕 Đúng ngữ pháp "wide vs narrow contract" mà [EMC++ Item 14](../effective-modern-cpp.md) đã đặt nền.
- "Đệ quy dễ viết — lặp dễ chạy" + "đo rồi mới tối ưu" là hai câu trả lời phỏng vấn sẵn dùng khi được hỏi về recursion trong C++ (kèm chi tiết: tail-call không được bảo đảm trong C++, khác các ngôn ngữ functional).

---

## Cụm 5 — Modern C++ & good practice: bộ đồ nghề để lặp nhanh (tr. 25–34)

### Nội dung chính

**Vì sao C++ và kỷ luật đi kèm (tr. 25–26):** lý do chính chọn C++ = **hiệu năng cao mà không mất quyền kiểm soát tầng thấp**. Nhưng hai kỷ luật đặt ra ngay:
1. **Đừng micro-optimize** — compiler hiện đại sinh mã tốt hơn cả người viết assembly lão luyện; chỉ tối ưu tay khi *thật sự* cần, và nhớ: dồn sức tối ưu một chỗ thường là bỏ bê chỗ khác. Ghi nhận sẵn **chỗ nào hiệu năng thật sự quan trọng** (tight loop trên dữ liệu lớn: có; hàm chờ database: không — bị nghẽn ở kết nối rồi).
2. **Không quản lý bộ nhớ bằng tay** — không `new/delete`, càng không `malloc/free`: "công thức tạo leak và UB". Dùng container chuẩn (`std::vector`), smart pointer (`unique_ptr/shared_ptr`), hoặc thư viện chất lượng (Boost, Abseil); đặc biệt bắt buộc khi đa luồng (tr. 26).

**CMake (tr. 26–28):** build-system generator — sinh Makefile/Ninja từ `CMakeLists.txt`. Tư duy CMake hiện đại = **targets** (executable/library) + `find_package` tìm dependency + `target_link_libraries(MyExe PRIVATE MyDep)` (PRIVATE = thông tin link không lan truyền theo target — hợp executable, cân nhắc lại với library). Khung tối thiểu: `cmake_minimum_required` → `project()` → `set(CMAKE_CXX_STANDARD 20)` + `REQUIRED ON` → `add_executable`. Chạy: `cmake -B out/Release -S . -DCMAKE_BUILD_TYPE=Release` rồi `cmake --build`. Gắn được package manager (**vcpkg**, **conan**); so với Bazel/Meson thì CMake đầy đủ tính năng và dễ dùng hơn (nhận định của tác giả). 🆕 Đối chiếu: [06/cmake.md](../../06-build-systems/cmake.md) của repo chi tiết hơn — sách chỉ cần mức "đủ để theo dự án".

**Views, ranges, algorithms (tr. 28–29):** C++20 mang về những thứ "ngôn ngữ khác có từ lâu":
- **`string_view` / `span`** — **view không copy** trên khối phần tử liền kề trong bộ nhớ; `string_view` bất biến (sửa chuỗi tại chỗ nguy hiểm với UTF-8 — ký tự thay thế có thể cần nhiều byte hơn), `span` cho truy cập được-phép-sửa hoặc không.
- **Range** = trừu tượng hóa trên iterator: bất kỳ object nào expose begin/end cho duyệt tuần tự. Sức mạnh ở chỗ **compose được với views** — ví dụ sách viết lại parser bằng `std::views::enumerate` (C++23) + structured binding `[position, char]`: không khác về chất, nhưng **ý định rõ hơn hẳn** (position gắn chặt vòng lặp, không "rò" ra scope ngoài — tr. 29).

**Templates & concepts (tr. 29–31):**
- Template = "một trong những tính năng hay nhất và gây bực nhất": viết một lần chạy mọi kiểu; bản thân template là **ngôn ngữ hoàn chỉnh chạy lúc compile** (tính giá trị compile-time → chi phí runtime về 0). Cái giá: **instantiation đệ quy** — lỗi phát hiện ở tít sâu, "cách rất xa nơi gây ra" → error message kinh hoàng.
- **Concepts** = khai báo **tập yêu cầu của template ngay cửa vào** → compiler check concept cho kiểu *trước*, không cần instantiate sâu để phát hiện sai → **error message tốt hơn**, có khi compile nhanh hơn. Hai cách viết được minh họa: `requires`-expression tự viết (đòi `value_type` totally_ordered, có begin/end), và — bài học hay hơn — **ghép từ concept chuẩn**: `std::input_range<const T> && std::totally_ordered<std::range_value_t<const T>> && std::copy_constructible<...>` — tổng quát *hơn* bản tự viết (range không nhất thiết có method begin/end — nên dùng `std::ranges::begin/end` thay vì gọi method, tr. 31).
- Giới hạn nói rõ: concept là **compile-time** — "container không rỗng" vẫn phải check runtime (tr. 31). Lời khuyên: "càng dùng nhiều concepts, debug code template lớn càng dễ thở."

**Error handling (tr. 32–33)** — phần đáng giá nhất cụm:
- **Phân biệt failure vs error** (định nghĩa của sách): **failure** = kết cục hợp lệ nằm trong dự liệu của bài toán (search không tìm thấy — xử lý như chuyện thường ngày, bằng `std::optional` hay trả end iterator); **error** = chương trình rơi vào **trạng thái không hợp lệ**/gặp vấn đề không xử lý nổi — phải lan truyền đến nơi xử lý được hoặc terminate.
- Hai cơ chế truyền thống và điểm yếu: **exception** — toàn cục, không catch là terminate kèm thông điệp (đôi khi là điều muốn), nhưng **đắt**, compiler phải sinh nhiều mã phụ trợ, và "đặc biệt rắc rối ở API boundary / RPC"; **mã lỗi kiểu C** — siêu nhẹ nhưng nghèo thông tin.
- **`std::expected` (C++23)** = điểm giữa lý tưởng: một object chứa **hoặc** kết quả hợp lệ **hoặc** giá trị lỗi, không bao giờ rỗng — nhẹ như error code, giàu thông tin như exception, **cục bộ trừ khi chủ động lan truyền** — "đặc biệt tốt ở interface boundary". Chưa có C++23 thì: Abseil `Status`, Boost Leaf `result` (tr. 33).
- Kèm: **logging** bằng thư viện chuẩn (spdlog) — rẻ mà cứu bạn khi code đã "ship" và người dùng không thể chạy debugger (tránh chỗ hiệu năng nhạy cảm).

**Testing (tr. 33–34):** dự án nào cũng phải có test — **mỗi feature mới thêm test, mỗi bug được xác nhận bằng một test** (đó là cách duy nhất biết code "đúng"). Ba tầng: **unit** (hàm/class đơn lẻ — nhỏ, nhanh, chạy thường xuyên) → **integration** (cụm chức năng phối hợp — chạy khi xong feature/trước deploy) → **end-to-end** (trọn vòng đời phần mềm — ít, chạy trước release). Framework: **GoogleTest** (linh hoạt, cần link gtest.so) vs **Catch2** (đơn giản hơn, header-only); code sách dùng GoogleTest.

**Version control (tr. 34):** git/GitHub là chuẩn de facto — kể cả dự án cá nhân (lý do 1: quay lại quá khứ khi hỏng/regression; lý do 2: chia sẻ); GitHub/GitLab CI test tự động đa nền tảng — "không máy đơn lẻ nào tự test hết các cấu hình".

### Insight đáng nhớ

- Cụm này thực chất trả lời: **"muốn LẶP NHANH (tinh thần cụm 1) thì cần hạ tầng gì?"** — build tái lập (CMake), an toàn mặc định (RAII/container thay new-delete), sai thì biết sớm (concepts, test, CI), sai ở hiện trường thì có dấu vết (logging, expected). Mindset ≠ chỉ cách nghĩ — còn là **bộ đồ nghề cho phép nghĩ sai mà rẻ**.
- **Failure vs error** là cặp khái niệm sắc bén nhất chương — nó quyết định *cơ chế*: failure → `optional`/`expected` (nhánh bình thường của logic); error → exception/terminate (thoát khỏi logic thường). Nhiều codebase rối vì ném exception cho failure thường ngày.
- 🆕 Đối chiếu [EMC++](../effective-modern-cpp.md): EMC++ dừng ở C++11/14 — chương này chính là bản "nối dài" lên C++17/20/23 cho bộ từ vựng phỏng vấn: `string_view/span`, ranges, concepts, `std::expected`.

### Góc interview

**Câu 1 (🎯):** Phân biệt "failure" và "error" trong thiết kế API C++. Với mỗi loại, cơ chế nào phù hợp — `optional`, `expected`, exception, error code?

<details><summary>Đáp án</summary>

- **Failure** = kết cục nằm **trong dự liệu** của bài toán — search không thấy, parse gặp record không hợp lệ trong dòng dữ liệu bẩn, cache miss. Nó là một *nhánh bình thường* của logic → xử lý tại chỗ, không phá luồng: **`std::optional<T>`** (chỉ cần "có/không"), trả end iterator (STL search), hoặc **`std::expected<T, E>`** (C++23) khi caller cần *biết vì sao* fail.
- **Error** = chương trình vào **trạng thái không hợp lệ** hoặc gặp thứ không xử lý nổi ở tầng này — vi phạm precondition, hết tài nguyên nghiêm trọng, bất biến nội bộ vỡ. Cần lan truyền tới nơi đủ ngữ cảnh xử lý hoặc terminate: **exception** (ưu điểm: toàn cục, không catch là chết to rõ ràng kèm thông điệp) hoặc — ở nơi exception có vấn đề — `expected` lan truyền tường minh, và **assert/terminate** cho vi phạm hợp đồng lập trình (bug chứ không phải tình huống runtime).
- Vì sao không exception cho tất cả: đắt (mã phụ trợ, unwinding), khó đoán trên đường nóng, và **rắc rối tại API/ABI boundary** (qua biên shared library C, qua RPC — exception không truyền qua được ranh giới đó một cách an toàn — 🆕 đúng mối quan tâm của topic [07/api-design.md](../../07-shared-libraries/api-design.md): interface C ổn định thường buộc error code/expected-style).
- Vì sao không error code cho tất cả: nghèo thông tin, dễ bị lờ đi (không check return), làm bẩn chữ ký hàm.
- Chốt: **phân loại trước, chọn cơ chế sau** — cùng một sự kiện "không tìm thấy file" có thể là failure (tool quét thư mục) hoặc error (nạp file cấu hình bắt buộc lúc khởi động) tùy ngữ cảnh bài toán; cơ chế đi theo vai trò, không theo sự kiện.

</details>

**Câu 2:** Concepts (C++20) giải quyết vấn đề gì của template? Nó có thay được kiểm tra runtime không?

<details><summary>Đáp án</summary>

- Vấn đề gốc: template được **instantiate đệ quy** — compiler thay kiểu vào, đi sâu dần, và chỉ phát hiện "kiểu này không hợp lệ" ở tận đáy chuỗi instantiation → error message dài hàng trang, chỉ vào chỗ **cách rất xa nguyên nhân** (nơi gọi sai chỉ là dòng đầu của trang lỗi thứ ba...).
- **Concepts** = khai báo tập yêu cầu **ngay cửa vào** template (`template <OrderableContainer C>`): compiler check yêu cầu cho kiểu *trước khi* instantiate → sai là báo **ngay tại call site, bằng tên concept** ("C không thỏa OrderableContainer vì thiếu totally_ordered") — lỗi đúng chỗ, đúng ngôn ngữ thiết kế; phụ: có thể compile nhanh hơn (đỡ instantiate vô vọng), và concept tham gia **overload resolution** (chọn bản cài theo năng lực của kiểu).
- Cách viết tốt: **ghép từ concept chuẩn** (`std::input_range`, `std::totally_ordered`, `std::copy_constructible`) thay vì tự viết requires-expression — vừa đúng tổng quát hơn (range không nhất thiết có *method* begin/end → dùng `std::ranges::begin`), vừa tự document.
- **Không thay được runtime check**: concept là thuộc tính của **KIỂU tại compile time** — "container không rỗng", "index trong biên", "file mở được" là thuộc tính của **GIÁ TRỊ tại runtime** → vẫn cần if/assert/expected. Cặp đôi đúng: concept gác cổng kiểu, precondition-check gác cổng giá trị.
- 🆕 Liên hệ: concepts là lời giải "chính chủ" cho vấn đề mà [EMC++ Items 26–27](../effective-modern-cpp.md) phải xử lý bằng `enable_if` — so sánh được hai thời kỳ là điểm cộng lớn.

</details>

**Câu 3:** Vì sao "giải bằng đệ quy rồi mới tối ưu" là lời khuyên hợp lý trong C++? Khi nào bản đệ quy phải bị thay?

<details><summary>Đáp án</summary>

- Trình tự đúng của sách: **giải đúng trước — tối ưu sau, và chỉ sau khi ĐO** ("measure twice, cut once"). Đệ quy thường là bản *dễ viết đúng nhất* khi bài toán có cấu trúc đệ quy (grammar lồng nhau, cây, chia-để-trị) — code phản chiếu thẳng định nghĩa bài toán, dễ kiểm chứng, dễ mở rộng.
- Cái giá trong C++: lời gọi hàm không miễn phí (stack frame, cản inline); **tail-call optimization không được ngôn ngữ bảo đảm** (khác họ functional) — và khi độ sâu đệ quy **không biết trước tại compile time**, compiler gần như hết cách; độ sâu theo kích thước input → nguy cơ **tràn stack** với input lớn/thù địch (🆕 embedded: stack thread nhỏ, kernel stack 16KB — đệ quy không chặn sâu là lỗi thiết kế).
- Thay bằng bản lặp khi: (1) profiling chỉ ra nó là hotspot thật; (2) input không kiểm soát được độ sâu; (3) môi trường cấm đệ quy (coding standard safety-critical). Chấp nhận cái giá đã được sách nêu: bản lặp **khó suy luận và khó mở rộng hơn** — nên giữ bản đệ quy làm tài liệu/định nghĩa tham chiếu (và làm oracle cho test của bản lặp).
- Trả lời có cấu trúc "đúng trước → đo → đổi có lý do + giữ oracle" thể hiện đúng mindset mà chương này dạy.

</details>

### Đọc thêm (tùy chọn)

- [10-thinking/problem-solving.md](../../10-thinking/problem-solving.md) — khung giải quyết vấn đề của repo, đọc cặp với cụm 1–2.
- [02-modern-cpp/](../../02-modern-cpp/) + [effective-modern-cpp.md](../effective-modern-cpp.md) — nền C++11/14 mà cụm 5 nối dài lên C++20/23.
- [06/cmake.md](../../06-build-systems/cmake.md), [12-design-patterns/](../../12-design-patterns/) — chi tiết CMake và pattern.
