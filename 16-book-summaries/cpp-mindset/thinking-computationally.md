# C++ Mindset — Ch. 1: Thinking Computationally (tr. 1–35) ✅ đã đọc sâu toàn văn

> Thuộc [The C++ Programmer's Mindset](README.md). Nguồn: đọc trực tiếp PDF, trang sách = trang PDF − 25.
> Ký hiệu: không đánh dấu = từ sách · **🆕 = liên hệ/bổ sung của người viết summary** · trích dẫn kèm `(tr. X)`.

---

## Cụm 1 — Bốn thành phần của computational thinking (tr. 2–8)

### Nội dung chính

**Định nghĩa nền (tr. 2):** trong sách, một "problem" = **nhiệm vụ cụ thể với ràng buộc (constraints) và tham số (parameters) xác định rõ**. Ràng buộc có thể thuộc quản lý (deadline), kỹ thuật (hiệu năng), hay đặc thù lĩnh vực (giới hạn vật lý); tham số = dữ liệu/thông tin được cấp. Nhiệm vụ còn mơ hồ thì chưa phải "problem" — phải làm rõ trước đã.

**Bốn thành phần (tr. 3):** (1) **Decomposition** — chẻ nhỏ; (2) **Abstraction** — giữ cốt lõi, bỏ nhiễu; (3) **Pattern recognition** — nhận ra bài con là "ca" của lớp bài đã có lời giải; (4) **Algorithm design** — thiết kế chuỗi bước rồi ghép lại.

Ba cảnh báo của tác giả — phần "mindset" thật sự:
- **Không phải quy trình 1→2→3→4**: *"they are not, in themselves, the steps that one must follow... You must consider all the components at once"* (tr. 3) — không thể decompose nếu chưa nhận ra cấu trúc, không thể abstraction nếu chưa thấy thành phần cốt lõi.
- **Lặp và động** (tr. 3–4): sẽ có nhiều hướng đi thất bại — *"this is a necessary part of the process"*; việc cần làm là hiểu vì sao hướng đó fail và mang bài học sang vòng sau; *"Don't be afraid to go back to the beginning... Sometimes this is the only way to make progress"* (tr. 4).
- **Lựa chọn sớm chi phối phần sau** (tr. 3): ví dụ của sách — cùng bài toán, giải bằng Python thì nghiêng về ghép thư viện hiệu năng cao có sẵn; bằng C++ thì có tự do (và trách nhiệm) **tự viết primitive hiệu năng cao** đúng nhu cầu.

**Lời khuyên chung (tr. 4):**
1. **Giữ sổ tay pattern/abstraction** để tra lại — kinh nghiệm sẽ nội hóa dần nhưng luôn có thứ quên. (🆕 Repo Embedded-Interview này chính là "sổ tay" đó.)
2. **Luyện bài nhỏ** (sách nêu đích danh LeetCode): mục đích là tích kho pattern — *"the problems don't need to be identical, just similar enough to provide inspiration"* (tr. 4).
3. **Nói chuyện với người khác về bài toán** — hoặc với **con vịt cao su**: *"talking the problem through with your rubber duck can help – and they are always willing to listen"* (tr. 4). (🆕 Đây là "rubber duck debugging" kinh điển; và là lý do dự án xuyên suốt của sách đặt tên "rubber duckies".)

**Ví dụ "quá toán" có chủ đích (tr. 5–8):** tác giả giải phương trình vi phân **y″ − 3y′ + 2y = x² + x**, điều kiện đầu y(0) = y′(0) = 1 — và nói rõ *"the actual mathematics used here is not the important part; the way that we work through the problem is"* (tr. 5). Quy trình đáng chép lại:

```
Bài gốc (không giải thẳng được)
├── Nhánh 1: nghiệm tổng quát của phương trình thuần (vế phải = 0)
│     → NHẬN RA MẪU: nghiệm dạng y = e^mx  →  thay vào ra "auxiliary equation"
│       m² − 3m + 2 = 0  → bài quen: PHƯƠNG TRÌNH BẬC HAI (tr. 6)
│     → nghiệm m=1, m=2  →  y = Ae^x + Be^2x
│     → áp điều kiện đầu → bài quen: HỆ PHƯƠNG TRÌNH (A+B=1, A+2B=1) → A=1, B=0
├── Nhánh 2: nghiệm riêng "dịch chuyển" theo vế phải x² + x
│     → NHẬN RA MẪU: thử dạng ax² + bx + c  →  thay vào, "SO SÁNH HỆ SỐ" (tr. 7)
│       → lại ra HỆ PHƯƠNG TRÌNH → a=1/2, b=2, c=5/2
└── GHÉP LẠI: y = e^x + ½x² + 2x + 5/2
```

Hai bài học sách rút tường minh: (1) *"we have actually decomposed it into a number of smaller problems, which required far less knowledge of mathematics"* (tr. 7) — chẻ đúng làm **mỗi bước cần ít kiến thức hơn hẳn bài gốc**; (2) bước ghép lại *"is actually a crucial step, and often non-trivial"* nhất là khi các phần phụ thuộc nhau (tr. 7). Về abstraction trong ví dụ này (tr. 8): factorization và Gaussian elimination chính là **kỹ thuật tổng quát** cho lớp bài trừu tượng, được áp cho ca cụ thể.

### Insight đáng nhớ

- Câu định vị cả sách, neo vào chính lời tác giả *"you must consider all the components at once"* (tr. 3): **bốn thành phần là bốn góc nhìn giữ đồng thời, không phải checklist**. Người mới hỏi "bước 1 làm gì?" — sách trả lời: không có bước 1 cố định, có 4 câu hỏi thường trực (chẻ được không? bỏ nhiễu được gì? giống bài nào từng gặp? thuật toán cho từng mảnh?).
- Ví dụ vi phân (tr. 5–8) minh họa vòng "decompose → nhận mẫu → bài con quen thuộc" chạy **đệ quy**: trong nhánh con lại nhận mẫu tiếp (bậc hai → hệ phương trình). Kho mẫu càng dày, chuỗi này dừng càng sớm — đúng lý do lời khuyên "luyện bài nhỏ + giữ sổ tay" (tr. 4) đứng ngay đầu sách.
- 🆕 Liên hệ repo: khung này là bản đầy đủ của [10-thinking/problem-solving.md](../../10-thinking/problem-solving.md) — nên đọc cặp.

---

## Cụm 2 — Decomposition: chẻ bài toán thế nào (tr. 8–12)

### Nội dung chính

**Vì sao người ta né việc chẻ (tr. 8):** hai lý do — người mới **thiếu tự tin** mình chẻ đúng; người kinh nghiệm hơn **sợ mất bức tranh lớn** (*"a client would not accept a piece of software that solves one aspect of their problem but not the whole"* — tr. 8). Phải cân bằng: tìm đường đi khả thi mà không lạc mất bài gốc.

**Hai loại phụ thuộc giữa bài con (tr. 9)** — thuật ngữ của sách:
- **Temporal dependency**: bài A phải được giải *trước* bài B.
- **Technical dependency**: *lời giải* của A phụ thuộc/được định hình bởi lời giải của B (chú ý: phụ thuộc vào **lời giải**, không phải **kết quả** — tr. 9).

**Nguyên tắc "chẻ tối thiểu" (tr. 9):** mỗi tầng decomposition **tăng số mảnh phải ghép lại** — *"Aim to decompose a problem as minimally as possible to avoid adding complexity"*. Khi bài có cách chẻ hiển nhiên → *"these easily defined sub-problems are almost always the best way to proceed"* (tr. 9).

**Thí nghiệm tư duy bữa tối (tr. 9–10):** starter + main + dessert + drinks — các liên kết đều *yếu* (rượu theo món chính = technical; khai vị dọn trước = temporal — "both are weak" vì về nguyên tắc rượu nào đi với món nào cũng được). Chiến lược sách chọn: **chọn main course trước**, các phần khác xoay quanh — *"we reduce the likelihood of having to backtrack... the main course is the most important component, so solve this problem first"* (tr. 9). 🆕 Tổng quát hóa: heuristic *most-constrained-first* — giải phần ràng buộc nặng nhất trước để giảm quay lui.

**Khung 3 phần của hầu hết bài toán lập trình (tr. 10)** — nguyên văn ý sách: *"Most coding problems have at least three components"*:

```
[1] Acquire  — lấy dữ liệu từ người dùng + chuyển về dạng dùng được
[2] Work     — phần việc thật sự của bài toán
[3] Return   — hậu xử lý + trả kết quả
```

Câu hỏi mồi để gom bài con (tr. 10): phần nào **khó nhất**? phần đó cần **thông tin gì**? có cần **tiền xử lý** trước khi dùng? có cần **hậu xử lý** trước khi trả?

**Ví dụ xuyên suốt: chênh lệch nhiệt độ trung bình năm (tr. 10–12).** Bài: từ database nhiệt độ các vùng đo định kỳ nhiều năm, tính mức thay đổi năm-qua-năm mỗi vùng. Chuỗi tinh chỉnh — minh họa "lặp và động" bằng số liệu cụ thể:

1. **Ngây thơ:** trung bình mỗi năm rồi trừ nhau → sách chỉ ra hỏng: *"accounting for seasonal variation... merely computing the differences between yearly averages would not account for this"* (tr. 11) — mất sạch cấu trúc mùa.
2. **Khá hơn:** trừ **các giá trị tương ứng cùng thời điểm** giữa hai năm liên tiếp, rồi trung bình các hiệu — nhưng vẫn dính nhiễu thời tiết ngắn hạn.
3. **Tốt (đủ dữ liệu):** tính **trung bình cửa sổ trượt** quanh mỗi thời điểm để "smooth out" biến động cục bộ, giá trị đã mượt mới đem so năm-với-năm (tr. 11).

Kèm bài học **đơn vị đo** (tr. 11): Celsius/Fahrenheit lẫn lộn — *"Many errors have been caused throughout history by failing to account for units of measurement, sometimes with catastrophic consequences"*; lý tưởng: đổi hết về **Kelvin** trước mọi tính toán → đây chính là việc của tầng [1] tiền xử lý.

Decomposition chốt (Figure 1.1, tr. 12):

```
Data → Preprocess → Sliding-window averages → Annual window differences → ...
                          │ (độc lập nhau            │ (cũng song song
                          │  → SONG SONG HÓA được)   │  hóa được)
                          └────────────┬─────────────┘
                     Average annual window difference per region
```

Và điểm dừng đúng lúc: *"We don't need to decompose further; each of the sub-problems already seems feasible... However, the option remains open to us should we need it"* (tr. 12).

### Insight đáng nhớ

- Ba heuristic gói cụm này, mỗi cái đều có neo: **chẻ tối thiểu** (tr. 9), **most-constrained-first** (bữa tối, tr. 9–10), **khung vào–làm–ra** (tr. 10). 🆕 Áp thẳng vào nghề: một driver cũng là vào–làm–ra (đọc phần cứng → xử lý → báo lên tầng trên).
- Ví dụ nhiệt độ cho thấy **yêu cầu thật lộ ra qua các lời giải hỏng**: "trung bình năm" hỏng vì mùa, "so cùng thời điểm" hỏng vì nhiễu ngắn hạn — mỗi lần hỏng là một mảnh spec mới. 🆕 Trong interview system design, trình bày theo mạch "naive → vì sao thiếu → tinh chỉnh" đúng như sách làm ở tr. 11 sẽ ăn điểm hơn nhảy thẳng đáp án cuối.

---

## Cụm 3 — Abstraction & Pattern recognition (tr. 12–17)

### Nội dung chính

**Abstraction là gì (tr. 12):** bài toán thật thì "messy" — thừa thông tin gây nhiễu. Abstraction = *"a generalized view of a concept or data that keeps only the essential information and properties"*. Tốn công xây nhưng *"the payoff can be dramatic"*: áp được cho cả lớp bài, thành công cụ lâu dài. Và lưu ý: bạn đã dùng abstraction mọi lúc — hàm, class, bộ nhớ hệ thống, file system, cả `malloc/free` của runtime (tr. 13).

**Ba chỗ abstraction "ăn tiền" (tr. 13):**
1. **Cửa nhận dữ liệu:** chương trình lấy dữ liệu từ đâu — *"a Unix command-line tool that reads directly from the terminal (stdin) or a file on the disk? ... a GUI application...? ... part of a library that exposes an interface...?"* — mỗi dạng trông khác nhau, nhưng *"with a well-designed abstraction, you could be flexible enough to answer any of these design objectives"*. (🆕 Đây là mầm của bài "file reader + strategy" mà dự án ch. 9 làm thật.)
2. **Ép dữ liệu vào đúng yêu cầu:** lại chuyện đơn vị — abstraction ở interface cho phép làm việc với dữ liệu *đã đúng đơn vị* bất kể nguồn.
3. **Thiết kế thuật toán:** dữ liệu nhúng được vào **cấu trúc toán học** (vd vector space) → thừa hưởng thao tác của cấu trúc đó + *"can be used to make strong guarantees about the algorithm"* + có sẵn cài đặt tối ưu.

**Nhìn DỮ LIỆU qua tính chất trừu tượng (tr. 13–14):** dữ liệu số → có thứ tự tự nhiên, có `==`, có số học (+,−,×,÷) — *"each of these is an abstract property that numbers happen to possess"*; bài toán có khi chỉ cần "so sánh được + có thứ tự" (search/sort), không cần "là số". Text → chuỗi ký tự trên một "alphabet" (ASCII 1 byte, UTF-8 đa byte) — cái quan trọng là tách/so sánh ký tự được, không phải bảng mã nào. Tổng quát (tr. 14): *"many problems can be generalized by considering what traits parts of the data must satisfy (having ordered, equality comparable, iterable, etc.)"* — 🆕 chữ **traits** ở đây gieo mầm thẳng cho C++ **concepts** ở cụm 5. Sách cũng thật thà: có khi phải giải bản cụ thể xong mới thấy đường tổng quát; có bài không tổng quát hóa được (tr. 14).

**Nhìn BÀI TOÁN qua khung lý thuyết (tr. 14–15):** ví dụ đắt nhất — tìm pattern trong text → regex → và *"the computational theory of regular expressions is deterministic finite automata (DFA), which are the abstraction that we seek"* (tr. 14). Nhận ra một phần bài khớp một khung lý thuyết thì: (1) cả kho kết quả của khung dùng được; (2) *"you can start to look for other parts of the problem that also fit this model"* (tr. 15). Nên nắm các lý thuyết nền: automata/state machine, sort/search, giải hệ tuyến tính, tối ưu ràng buộc, đồ thị/shortest path — *"numerical algorithms have a habit of turning up in unexpected places"* (tr. 15).

**Digression Sudoku + cờ (tr. 15–16):** Sudoku khó đòi mẫu phức hợp như **X-wing** — cặp hàng mà một chữ số chỉ còn khả dĩ ở đúng một cặp cột → suy ra loại chữ số đó khỏi mọi ô khác của hai cột đó (tr. 15). Note về cờ chốt ý (tr. 16): *"Solving problems is no different. Being an effective programmer requires you to learn some of the common patterns that appear and how to use or counter them."*

**Danh mục mẫu — hai họ (tr. 16–17):**
- **Functional patterns** (*bài cần làm gì*): sort (theo predicate), search, filter (+ random sampling), shuffle, permutation; bài quy về **numerical** (hệ phương trình, tối ưu ràng buộc, graph algorithms); **thống kê** — *"any time there is uncertainty in measurements or methodology, statistics will probably be needed"*; **combinatorics** với ba ánh xạ cụ thể của sách (tr. 16): dependency resolution = đồ thị có hướng, **scheduling = bài graph coloring**, route planning = shortest path/traversal.
- **Structural patterns** (*tổ chức lời giải thế nào* — xuất hiện ở khâu thiết kế/cài đặt; sách dùng nghĩa RỘNG hơn GoF — tr. 17): **strategy/policy** — *"one might define various algorithms for reading data from a file as strategies, and change the strategy based on the file type. We will make use of this pattern several times later in this book"* (tr. 17); họ đổi-giao-diện: adapter, facade, flyweight, proxy; decorator (bọc để mở rộng). Note riêng về **template method**: *"the skeleton of an algorithm is described in terms of some generic (placeholder) operations... The C++ standard template library is full of examples of these kinds of patterns, especially in the algorithm header"* (tr. 17).

### Insight đáng nhớ

- Cặp phân loại **functional / structural** (tr. 16–17) là cách xếp kho mẫu theo *vai trò trong quy trình giải* — functional lộ ra khi phân tích bài, structural lộ ra khi thiết kế code. Ba ánh xạ "dependency = digraph, scheduling = coloring, routing = shortest path" (tr. 16) đáng thuộc lòng vì chúng là cầu nối một-câu từ bài thực tế sang thuật toán chuẩn.
- Câu "STL `<algorithm>` = template method pattern" (note tr. 17) đổi cách nhìn thứ dùng hằng ngày: `std::sort(begin, end, comp)` chính là khung-thuật-toán-với-bước-tùy-biến. 🆕 [12-design-patterns](../../12-design-patterns/) của repo có chi tiết từng pattern; giá trị thêm của sách là đặt pattern vào *thời điểm nhận diện* trong quy trình giải.

---

## Cụm 4 — Algorithm: đọc, viết, tính chất, và đệ quy vs lặp (tr. 17–25)

### Nội dung chính

**Định nghĩa (tr. 17):** algorithm = tập chỉ dẫn nhận dữ liệu thỏa **preconditions**, sinh output thỏa **postconditions**; viết bằng **pseudocode** trung lập ngôn ngữ.

**Hai mô hình tính toán (tr. 18):** **sequential** (máy Turing — từng bước một) và **functional** (lambda calculus — đệ quy), tương đương về năng lực. Ngôn ngữ thật pha trộn: *"C++ is primarily sequential, though C++ templates are functional"*; Haskell thuần functional. Nên hiểu cả hai.

**Ví dụ 1 — max của list.** Pseudocode của sách (tr. 18–19, quy ước: UPPERCASE là keyword, `<-` là gán để phân biệt với `=` so sánh; INPUT/OUTPUT khai pre/postconditions):

```
INPUT: L is a list of numbers with at least one element
OUTPUT: Maximum value of L

max <- first element of L
WHILE not at end of L
  current <- next element of L
  IF current > max
    max <- current
  END
END
RETURN max
```

Bản C++ của sách (tr. 19) — template nhận "container" tổng quát có begin/end và `value_type` so sánh được bằng `<`:

```cpp
template <typename Container>
typename Container::value_type max_element(const Container& container) {
    auto begin = container.begin();
    auto end   = container.end();

    if (begin == end) {                       // container rỗng: max KHÔNG định nghĩa
        throw std::invalid_argument("container must be non-empty");
    }

    auto max = *begin;
    ++begin;
    for (; begin != end; ++begin) {
        const auto& current = *begin;
        if (max < current) {                  // chỉ đòi hỏi value_type có operator<
            max = current;
        }
    }
    return max;
}
```

Bình luận của sách (tr. 20) — đáng giữ từng ý:
- Container rỗng: `throw` là *một* cách "correct"; **cách khác: trả `optional<...>`** — và lợi thế được nói rõ: *"This has the advantage of potentially allowing for noexcept to be added to the function declaration, reducing the runtime cost of launching this function"* (🆕 nối [EMC++ Item 14](../effective-modern-cpp.md) về noexcept).
- *"This implementation is for demonstration only; you should use the constrained algorithm `std::max_element` from the algorithm header instead."*
- Nên **chú thích code theo bước thuật toán** để người sau (gồm "future self") đối chiếu cài đặt với thiết kế.
- Tổng quát hóa comparison operator thì phải cẩn thận: có ordering mà max **không xác định duy nhất** (không phải mọi cặp so sánh được) — "beyond our capabilities at the moment".

**5 tính chất của một algorithm đúng nghĩa (tr. 20–21):** **finiteness** (kết thúc sau hữu hạn bước), **definiteness** (bước mô tả chính xác, không mơ hồ), **inputs** (thuộc tập xác định bởi preconditions), **outputs**, **effectiveness** (mỗi bước đủ cơ bản để làm bằng giấy bút). Chứng minh tính chất bằng **quy nạp**; preconditions nên được kiểm tra trong cài đặt — *"implicitly by means of static types... or explicitly by conditional statements"* (tr. 21). Sách tự soi ví dụ max: đúng n bước cho n phần tử (finite), input một list non-empty, output thỏa định nghĩa max (m ∈ S và mọi s ≤ m) (tr. 21).

**Ví dụ 2 — đệ quy (tr. 22–25).** Grammar tí hon:

```
letter ::= 'a' | 'b'
word   ::= letter | '[' word ',' word ']'
-- từ hợp lệ:  a   [a,a]   [a,[a,b]]   [[a,a],[a,[a,b]]]
```

*"Notice that this language is recursive in nature... it is natural that algorithms to work with this language might also be recursive"* (tr. 22). Bài: tìm vị trí ký tự cuối của word đầu tiên. Bản C++ đệ quy của sách (tr. 23–24):

```cpp
size_t end_of_first_word(std::string_view s) noexcept {
    if (!s.starts_with('[')) {           // word là letter đơn → kết thúc ngay tại 0
        return 0;
    }
    size_t position = 0;
    assert(s[position] == '[');          // các assert = dịch comment pseudocode
    position += 1;                       //   thành BẤT BIẾN kiểm tra được

    auto a = s.substr(position);
    auto i = end_of_first_word(a);       // ĐỆ QUY lần 1: word con thứ nhất
    position += i;

    position += 1;
    assert(s[position] == ',');
    position += 1;

    auto b = s.substr(position);
    auto j = end_of_first_word(b);       // ĐỆ QUY lần 2: word con thứ hai
    position += j;

    position += 1;
    assert(s[position] == ']');
    return position;
}
```

Ba quyết định thiết kế sách phân tích (tr. 24):
- **`std::string_view`** (C++17): *"a better way to work with non-owning strings than using raw const char*"* — giúp không truy cập ngoài biên chuỗi + có sẵn `substr/starts_with`.
- **Precondition đặt lên caller**: hàm `noexcept`, không check lỗi ngoài `assert` — gọi trên chuỗi không bắt đầu bằng word hợp lệ là **UB**; *"This might be necessary on the 'hot path' of a program, where checking for invalid strings might be too costly."*
- **Đệ quy vs C++**: *"languages like C++ are not designed to work in this way. Recursive implementations might perform worse... calling functions in C++ can be an expensive operation"*; compiler có thể inline/tail-call-optimize, *"however, if the number of recursions cannot be known at compile time, the options are limited"* (tr. 24).

Bản **lặp** không đệ quy của sách (tr. 25) — đếm độ sâu ngoặc:

```cpp
size_t end_of_first_word(std::string_view s) noexcept {
    size_t position = 0;
    int depth = 0;
    for (const auto& c : s) {
        switch (c) {
            case '[': ++depth; break;
            case ']': --depth;
            default:
                if (depth == 0) { return position; }
        }
        ++position;
    }
    return position;
}
```

Nhận xét của sách về bản lặp (tr. 25): *"more difficult to reason about. Moreover, this implementation cannot so easily be generalized if some other operation needs to be performed"* — nhanh hơn nhưng cứng hơn. Kết luận nguyên tắc: *"one should never optimize until the performance is measured and the algorithm is known to underperform; **measure twice, cut once**"* (tr. 25).

### Insight đáng nhớ

- Bộ ba trong bản đệ quy — precondition (caller lo) → `assert` (bất biến, biến mất ở release) → `noexcept` (cam kết) — là bài mẫu về **thiết kế hợp đồng hàm cho hot path** (tr. 24). 🆕 Đúng ngữ pháp "wide vs narrow contract" của [EMC++ Item 14](../effective-modern-cpp.md).
- Cặp hai bản cài đặt (tr. 23–25) là minh chứng cụ thể cho trade-off *flexibility vs performance*: bản đệ quy phản chiếu thẳng grammar (dễ kiểm chứng, dễ thêm thao tác khác), bản lặp nhanh hơn nhưng "khó suy luận, khó mở rộng" — và phán quyết nằm ở **đo đạc**, không ở khiếu thẩm mỹ ("measure twice, cut once", tr. 25).

---

## Cụm 5 — Modern C++ & good practice: bộ đồ nghề để lặp nhanh (tr. 25–34)

### Nội dung chính

**Vì sao C++, và hai kỷ luật đi kèm (tr. 25–26):** lý do chính chọn C++ = *"you need to make high-performance solutions"* mà không mất kiểm soát primitive tầng thấp (tr. 25). Nhưng:
1. **Đừng micro-optimize** (tr. 26): *"Modern compilers are far better at producing optimized machine code than even very experienced programmers writing hand-crafted assembly code. Trust the compiler."* Và chi phí cơ hội: *"over-optimizing one part of the code probably means that you are neglecting another"*. Ghi nhận sẵn chỗ hiệu năng thật sự quan trọng: tight loop trên dữ liệu lớn — có; hàm lấy record từ database — không (*"always be constrained by the connection"*).
2. **Không quản bộ nhớ bằng tay** (tr. 26): *"Do not manage your memory by hand with new and delete – or worse, with malloc and free. This is a recipe for creating memory leaks and invoking undefined behavior."* Dùng `std::vector`, `unique_ptr/shared_ptr`, hoặc Boost/Abseil — *"especially true if you make use of multithreading"*.

**CMake (tr. 26–28):** build-system generator — sinh Makefile/Ninja từ `CMakeLists.txt`. Khung tối thiểu của sách (tr. 27):

```cmake
cmake_minimum_required(VERSION 3.30)
project(MyProject)                       # điểm CMake tìm compiler, check settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)      # compiler không đủ chuẩn → báo lỗi ngay
add_executable(MyExecutable main.cpp)

target_link_libraries(MyExecutable PRIVATE MyDep)  # MyDep: từ add_library/find_package
# PRIVATE = thông tin link không lan truyền theo target
#   (hợp executable; với library thì cân nhắc lại — tr. 27)
```

Chạy (tr. 28): `cmake -B out/Release -S . -DCMAKE_BUILD_TYPE=Release` rồi `cmake --build out/Release --config=Release` (vế `--config` cho hệ đa cấu hình như MSBuild). Gắn được package manager (**vcpkg**, **conan**); nhận định của tác giả: CMake *"more feature-complete and easier to use than some of the similar tools... such as Bazel and Meson"* (tr. 28). 🆕 [06/cmake.md](../../06-build-systems/cmake.md) của repo chi tiết hơn; sách chỉ cần mức đủ theo dự án.

**Views, ranges, algorithms (tr. 28–29):** C++20 mang về *"features... that had been standard in other languages for many years"*:
- **`string_view`/`span`** — view **không copy** trên phần tử liền kề trong bộ nhớ. `string_view` bất biến — lý do được nêu cụ thể: *"Modifying strings in-place is dangerous because a new UTF-8 character might require more space than the character that it replaces, forcing a new allocation"* (tr. 28); `span` cho truy cập sửa được hoặc không.
- **Range** = trừu tượng hóa trên iterator (*"any object that exposes a begin and end"*), sức mạnh nằm ở **compose với views**. Ví dụ thật của sách (tr. 29) — viết lại bản lặp của parser bằng `std::views::enumerate` (C++23):

```cpp
size_t end_of_first_word(string_view s) noexcept {
    int depth = 0;
    for (const auto [position, ch] : std::views::enumerate(s)) {
        switch (ch) {
            case '[': ++depth; break;
            case ']': --depth;
            default:
                if (depth == 0) { return position; }
        }
    }
    // ...
}
```

Đánh giá của sách (tr. 29): không khác về chất, nhưng *"it does make the intent clearer"* — position gắn chặt vòng lặp, *"decontaminating the surrounding scope"*.

**Templates & concepts (tr. 29–31):**
- Template: *"arguably one of the best features of C++, and also one of the most difficult and frustrating"*; là **ngôn ngữ hoàn chỉnh chạy lúc compile** (tính giá trị compile-time → chi phí runtime ≈ 0). Cái giá: instantiation **đệ quy**, gặp biểu thức không hợp lệ mới báo lỗi — *"the error could have been caused far away from the first place where it is detected"* (tr. 30).
- **Concepts** = khai báo yêu cầu **ngay cửa vào**: *"the compiler does not need to recursively instantiate the template to find out whether it is valid or not; it just checks whether the concept is valid for the type"* → error message tốt hơn, có khi compile nhanh hơn (tr. 30). Hai cách viết của sách:

```cpp
// Cách 1 — requires-expression tự viết (tr. 30):
template <typename T>
concept OrderableContainer = requires(const T& t) {
    std::totally_ordered<typename T::value_type>;   // value_type so sánh thứ tự được
    t.begin();                                      // có begin/end gọi được trên const T&
    t.end();
};

// Cách 2 — GHÉP TỪ CONCEPT CHUẨN (sách khuyên — tr. 30-31):
template <typename T>
concept OrderableContainer = std::input_range<const T>
    && std::totally_ordered<std::range_value_t<const T>>
    && std::copy_constructible<std::range_value_t<const T>>;
```

Sách chỉ rõ cách 2 **tổng quát hơn**: range không nhất thiết có *method* begin/end → thân hàm cũng nên dùng `std::ranges::begin/end` thay vì gọi method (tr. 31). Bản `max_element` chốt:

```cpp
template <OrderableContainer Container>
std::range_value_t<Container> max_element(const Container& container) {
    auto begin     = std::ranges::begin(container);
    const auto end = std::ranges::end(container);
    if (begin == end) {                              // vẫn PHẢI check runtime!
        throw std::invalid_argument("Container must be non-empty");
    }
    // ... như cũ
}
```

Giới hạn nói thẳng (tr. 31): *"we still have to check that the container is not empty. This can only be tested at runtime, whereas concepts are a compile-time construction."* Lời khuyên chốt: *"the more concepts you use, the better experience you will have debugging large, complex bodies of templated code."*

**Error handling (tr. 32–33)** — phần đắt nhất cụm:
- **Failure vs error** — định nghĩa nguyên văn tinh thần của sách (tr. 32): search không tìm thấy phần tử thỏa điều kiện — *"This is a failure, but not an error. An error occurs when the program enters an invalid state or encounters a problem that it cannot handle."* Cách xử lý tương ứng: *"Failures should be handled as a routine problem using constructions such as `std::optional` or returning the end iterator for a failed search. Errors should be propagated to a point where they can be handled gracefully or terminate the application."*
- Hai cơ chế truyền thống (tr. 32): **exception** — toàn cục, không catch là terminate kèm thông điệp (đôi khi đúng ý muốn), nhưng *"fairly expensive"*, compiler phải thêm nhiều mã phụ trợ quanh lời gọi hàm, và *"particularly problematic on API boundaries or when working with remote procedure calls"*; **mã lỗi kiểu C** (0 = OK, ≠0 = lỗi) — *"extremely lightweight"* nhưng *"the amount of information that can be conveyed... is extremely limited"*.
- **`std::expected` (C++23)** (tr. 32–33): object chứa **hoặc** kết quả hợp lệ **hoặc** giá trị lỗi, *"never empty"* — *"lightweight, like the C-style error codes, but expressive and flexible, like the exception model"*; *"remains local (unless explicitly propagated)... especially on interface boundaries"*. Chưa có C++23: Abseil `Status`, Boost Leaf `result` (note tr. 33).
- **Logging** (tr. 33): thêm log bằng thư viện chuẩn (sách nêu **spdlog**) — *"a low-effort way of providing a wealth of debugging information to users who cannot simply launch a debugger"*; tránh chỗ hiệu năng nhạy cảm. Nhớ: code đã "ship" thì ai đó (gồm future you) phải bảo trì.

**Testing (tr. 33–34):** *"Even very simple software projects should have tests. Every new feature should add new tests. Every reported bug should be confirmed by adding tests. This is the only way to know that your code is 'correct'."* Ba tầng: **unit** (hàm/class đơn lẻ — nhỏ, nhanh, đông, chạy thường xuyên) → **integration** (cụm chức năng phối hợp — chạy khi xong feature/trước deploy) → **end-to-end** (trọn vòng đời phần mềm — ít, chạy trước release). Framework: **GoogleTest** (linh hoạt, cần link `gtest.so`) vs **Catch2** (đơn giản hơn, **header-only** không cần runtime ngoài); code của sách dùng GoogleTest (tr. 34).

**Version control (tr. 34):** git/GitHub chuẩn de facto — kể cả dự án nhỏ, hai lý do: quay lại quá khứ khi hỏng/regression, và chia sẻ (kể cả giữa các máy của chính mình); GitHub/GitLab CI chạy test đa nền tảng — *"No single computer can test all of these configurations on its own."*

### Insight đáng nhớ

- Cụm này trả lời câu hỏi treo từ cụm 1: **muốn "lặp nhanh, sai rẻ" thì cần hạ tầng gì?** Sách tự nói ở mở chương: *"good software practices that will enable you to iterate quickly and easily on your designs"* (tr. 1) — cụ thể hóa thành: build tái lập (CMake), an toàn mặc định (container/smart pointer thay new-delete, tr. 26), sai biết sớm (concepts tr. 30, test tr. 33, CI tr. 34), sai ở hiện trường có dấu vết (logging tr. 33, expected tr. 32).
- **Failure vs error** (tr. 32) là cặp khái niệm sắc nhất chương vì nó chọn *cơ chế* thay cho tranh cãi khẩu vị: failure → `optional`/`expected` (nhánh bình thường của logic); error → exception/terminate (thoát khỏi logic thường). Nhiều codebase rối chính vì ném exception cho failure thường ngày.
- 🆕 Đối chiếu [EMC++](../effective-modern-cpp.md): EMC++ dừng ở C++11/14 — cụm này nối dài bộ từ vựng phỏng vấn lên C++17/20/23: `string_view/span`, ranges/views, concepts, `std::expected`.

### Góc interview

**Câu 1 (🎯):** Phân biệt "failure" và "error" trong thiết kế API C++. Với mỗi loại, cơ chế nào phù hợp — `optional`, `expected`, exception, error code?

<details><summary>Đáp án</summary>

- **Failure** = kết cục nằm **trong dự liệu** của bài toán (định nghĩa sách tr. 32 — ví dụ chuẩn: search không tìm thấy). Là *nhánh bình thường* của logic → xử lý tại chỗ: **`std::optional<T>`** (chỉ cần có/không), trả **end iterator** (STL search — cách sách nêu đích danh), hoặc **`std::expected<T, E>`** khi caller cần biết *vì sao* fail.
- **Error** = chương trình vào **trạng thái không hợp lệ**/gặp vấn đề không xử lý nổi (tr. 32) — vi phạm precondition, cạn tài nguyên, bất biến vỡ → lan truyền tới nơi đủ ngữ cảnh xử lý hoặc terminate: **exception** (toàn cục; không catch → terminate kèm thông điệp — *"desirable in some cases"* tr. 32), hoặc `expected` lan truyền tường minh nơi exception có vấn đề.
- Vì sao không exception cho tất cả (theo sách tr. 32): đắt, compiler sinh nhiều mã phụ trợ quanh lời gọi, và *"particularly problematic on API boundaries or RPC"* — 🆕 đúng mối quan tâm ABI của [07/api-design.md](../../07-shared-libraries/api-design.md): interface C ổn định qua biên .so không cho exception bay qua.
- Vì sao không error code cho tất cả: nghèo thông tin (tr. 32), dễ bị lờ (không check return), bẩn chữ ký hàm.
- `std::expected` là điểm giữa (tr. 32–33): nhẹ như error code + giàu thông tin như exception + **cục bộ trừ khi chủ động propagate**. Chưa có C++23: Abseil Status / Boost Leaf.
- Chốt: **phân loại trước, chọn cơ chế sau** — "không tìm thấy file" là failure với tool quét thư mục, là error với file cấu hình bắt buộc lúc khởi động: cơ chế đi theo *vai trò trong bài toán*, không theo sự kiện.

</details>

**Câu 2:** Concepts (C++20) giải quyết vấn đề gì của template? Nó có thay được kiểm tra runtime không? Viết một concept ví dụ.

<details><summary>Đáp án</summary>

- Vấn đề gốc (tr. 30): template instantiate **đệ quy** — chỉ khi gặp biểu thức không hợp lệ ở tầng sâu mới báo lỗi, mà *"the error could have been caused far away from the first place where it is detected"* → error message dài, chỉ sai chỗ.
- **Concepts**: khai yêu cầu **ngay cửa vào** → compiler *"just checks whether the concept is valid for the type"* — không cần instantiate sâu → lỗi báo tại call site bằng tên concept, có khi compile nhanh hơn (tr. 30); concept còn tham gia overload resolution (🆕 — sách chưa nói ở chương này).
- Ví dụ (theo sách tr. 30–31), và bài học "ghép từ concept chuẩn thay vì tự viết":

```cpp
template <typename T>
concept OrderableContainer = std::input_range<const T>
    && std::totally_ordered<std::range_value_t<const T>>
    && std::copy_constructible<std::range_value_t<const T>>;

template <OrderableContainer C>
std::range_value_t<C> max_element(const C& c);   // lỗi kiểu → báo ngay tại đây
```

Ghép chuẩn tổng quát *hơn* tự viết requires (range không nhất thiết có method begin/end → dùng `std::ranges::begin/end` trong thân hàm — tr. 31).
- **Không thay được runtime check** — sách nói thẳng (tr. 31): concept là thuộc tính của **KIỂU tại compile time**; "container không rỗng" là thuộc tính của **GIÁ TRỊ tại runtime** → vẫn if/throw/expected. Cặp đúng: concept gác cổng kiểu, precondition check gác cổng giá trị.
- 🆕 Điểm cộng: concepts là lời giải chính chủ cho vấn đề mà [EMC++ Items 26–27](../effective-modern-cpp.md) phải xử lý bằng `enable_if` — so được hai thời kỳ là hiểu tiến hóa ngôn ngữ.

</details>

**Câu 3:** So sánh bản đệ quy và bản lặp của cùng một parser (ví dụ grammar `[a,[a,b]]`). Khi nào chọn bản nào trong C++?

<details><summary>Đáp án</summary>

- Hai bản của sách (tr. 23–25): bản **đệ quy** gọi lại chính nó cho từng word con — code **phản chiếu thẳng grammar** (grammar đệ quy → thuật toán đệ quy, tr. 22), dễ kiểm chứng đúng, dễ mở rộng (muốn làm thêm việc trên từng word con là có sẵn chỗ); bản **lặp** đếm depth theo `[`/`]` — nhanh hơn nhưng sách đánh giá: *"more difficult to reason about... cannot so easily be generalized if some other operation needs to be performed"* (tr. 25).
- Cái giá của đệ quy trong C++ (tr. 24): lời gọi hàm đắt; *"languages like C++ are not designed to work in this way"*; tail-call optimization **không được bảo đảm**, và *"if the number of recursions cannot be known at compile time, the options are limited"* — 🆕 thêm rủi ro **tràn stack** với input sâu (embedded: stack thread nhỏ, kernel stack 16KB — đệ quy không chặn độ sâu là lỗi thiết kế).
- Trình tự quyết định theo sách: **giải đúng trước** (thường là bản đệ quy) → **đo** → chỉ đổi sang bản lặp khi *"the performance is measured and the algorithm is known to underperform; measure twice, cut once"* (tr. 25). 🆕 Mẹo thực chiến: giữ bản đệ quy làm tài liệu tham chiếu + oracle cho test của bản lặp.
- Điểm cộng: nêu bộ ba hợp đồng của bản đệ quy — precondition trên caller + `assert` bất biến + `noexcept` (tr. 24) — và điều kiện dùng nó: hot path, nơi validate đầu vào quá đắt.

</details>

### Đọc thêm (tùy chọn)

- [10-thinking/problem-solving.md](../../10-thinking/problem-solving.md) — khung giải quyết vấn đề của repo, đọc cặp với cụm 1–2.
- [02-modern-cpp/](../../02-modern-cpp/) + [effective-modern-cpp.md](../effective-modern-cpp.md) — nền C++11/14 mà cụm 5 nối dài.
- [06/cmake.md](../../06-build-systems/cmake.md), [12-design-patterns/](../../12-design-patterns/) — CMake và pattern chi tiết.
