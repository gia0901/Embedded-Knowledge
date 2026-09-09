# SOLID — Năm cách nói của MỘT mục tiêu

> **TL;DR**
> - SOLID không phải 5 luật rời. Cả 5 phục vụ **một mục tiêu duy nhất: cô lập cái sẽ thay đổi, để sửa nó không làm vỡ chỗ khác.**
> - Mỗi nguyên lý chống **một lực** khác nhau đẩy code về phía khó sửa: SRP chống *gộp trách nhiệm* · OCP chống *sửa code đang chạy* · LSP chống *đa hình dối trá* · ISP chống *phụ thuộc thừa* · DIP chống *gắn cứng vào cái cụ thể*.
> - **Pattern là hệ quả, không phải mục tiêu.** Gần như mọi pattern trong topic này là một cách hiện thực hoá OCP + DIP. Hiểu nguyên lý thì pattern tự suy ra được; học thuộc pattern mà không hiểu nguyên lý thì nhồi pattern sai chỗ.
> - **Chi phí có thật:** mỗi abstraction = một lần gián tiếp + một file phải đọc + (qua `.so`) **một hợp đồng ABI phải giữ vĩnh viễn**. SOLID áp *khi có thay đổi thật*, không áp cho đẹp.

---

## 0. Sợi chỉ đỏ: "đóng gói cái biến thiên"

Trước khi vào 5 chữ cái, nắm một câu — nó giải thích được cả SOLID lẫn toàn bộ pattern phía sau:

> **Tìm ra thứ sẽ thay đổi, tách nó ra khỏi thứ đứng yên, rồi cho hai bên nói chuyện qua một hợp đồng.**

Ba bước đó ánh xạ thẳng vào 5 nguyên lý:

| Bước | Câu hỏi phải trả lời | Nguyên lý phụ trách |
|---|---|---|
| ① Tách | *Trong class này có mấy thứ thay đổi vì lý do khác nhau?* | **SRP** |
| ② Cho phép mở rộng | *Thêm biến thể mới có phải sửa code cũ không?* | **OCP** |
| ③ Dựng hợp đồng | *Hợp đồng gồm gì, ai được phụ thuộc vào ai, thay thế có an toàn không?* | **ISP · DIP · LSP** |

⭐ **Vì sao đáng nhớ câu này hơn 5 định nghĩa:** ở phỏng vấn, câu hỏi thật gần như không bao giờ là *"SRP là gì"*. Nó là *"thiết kế phần này thế nào"* — và câu trả lời tốt bắt đầu bằng **"cái gì ở đây sẽ thay đổi?"**, đúng bước ①.

---

## 1. SRP — Single Responsibility

| | |
|---|---|
| **Bản chất** | Một class chỉ nên có **một lý do để thay đổi** — tức phục vụ **một nhóm người dùng / một mối quan tâm** |
| **Lực nó chống** | Class phình dần vì "tiện tay thêm vào đây" |
| **Dấu hiệu vi phạm** | Tên class có chữ "Manager"/"Helper"/"Util" · mô tả nó phải dùng chữ "và" · hai người sửa hai lý do khác nhau đụng cùng một file |
| **Cái giá khi áp** | Nhiều class nhỏ hơn ⟹ phải nhảy file nhiều hơn |

```cpp
// ❌ Ba lý do thay đổi trong một class:
//    đổi thuật toán đọc · đổi định dạng lưu · đổi cách log
class Sensor {
    double read();
    void   saveToFile();
    void   formatReport();
};

// ✅ Mỗi class một lý do
class Sensor        { double read(); };
class SensorStorage { void save(double); };
class SensorReport  { std::string format(double); };
```

> 💡 **"Một trách nhiệm" đo bằng LÝ DO THAY ĐỔI, không đo bằng số hàm.** Một class 20 hàm cùng phục vụ một mối quan tâm vẫn đúng SRP; một class 2 hàm mà một hàm đổi vì phần cứng, hàm kia đổi vì yêu cầu UI thì đã sai.

**Áp vào hệ display:** `DisplayImpl` điều phối; thuật toán dimming nằm ở `IDimming`; đường xuống phần cứng nằm ở backend. Đổi thuật toán không đụng đường xuống HW, và ngược lại — đó là SRP ở mức kiến trúc ([in-practice/01](in-practice/01-display-stack.md)).

---

## 2. OCP — Open/Closed

| | |
|---|---|
| **Bản chất** | Thêm hành vi mới bằng **thêm code**, không bằng **sửa code đã chạy đúng và đã test** |
| **Lực nó chống** | Chuỗi `if/else` hoặc `switch` phình ra ở nhiều chỗ, mỗi biến thể mới lại phải sửa hết |
| **Dấu hiệu vi phạm** | Cùng một `switch (type)` xuất hiện ở ≥ 2 nơi · thêm loại mới phải grep khắp repo |
| **Cái giá khi áp** | Một abstraction + một điểm phân giải (factory / bảng) |

```cpp
// ❌ Mỗi loại mới sửa lại chính hàm đang chạy đúng
double area(const Shape& s) {
    if (s.type == CIRCLE) ...
    else if (s.type == SQUARE) ...
}

// ✅ Thêm loại = thêm class, không đụng code cũ
struct Shape  { virtual double area() const = 0; virtual ~Shape() = default; };
struct Circle : Shape { double area() const override; };
struct Square : Shape { double area() const override; };
```

⚠️ **Hiểu nhầm phổ biến — "OCP nghĩa là không được sửa file cũ".** Không phải. OCP nói *đừng sửa **logic đã đúng** để nhét biến thể mới*. Sửa để **vá bug** hay **đơn giản hoá** là chuyện khác hoàn toàn.

⚠️ **Và OCP không xoá được `switch`, nó chỉ DỒN `switch` về một chỗ.** Ở đâu đó vẫn phải ánh xạ *config → class cụ thể*. Mục tiêu là **đúng một** `switch` (trong factory), thay vì mười cái rải rác — xem [in-practice/01 §2.2](in-practice/01-display-stack.md).

---

## 3. LSP — Liskov Substitution

| | |
|---|---|
| **Bản chất** | Lớp con phải thay được lớp cha **về HÀNH VI**, không chỉ về kiểu — code viết cho lớp cha phải vẫn đúng |
| **Lực nó chống** | Dùng kế thừa để **tái sử dụng code** thay vì để mô hình hoá quan hệ *is-a* |
| **Dấu hiệu vi phạm** | Lớp con override rồi **ném exception** / **không làm gì** · caller phải `dynamic_cast` để biết mình đang cầm loại nào · comment kiểu *"đừng gọi hàm này với X"* |
| **Cái giá khi áp** | Đôi khi phải bỏ kế thừa, chuyển sang composition |

**Ba luật con, nói được là ăn điểm:** lớp con **không được siết tiền điều kiện**, **không được nới hậu điều kiện**, **không được ném exception mới**.

```cpp
// ❌ Kinh điển: Square is-a Rectangle về DỮ LIỆU, không về HÀNH VI
class Rectangle { virtual void setW(int); virtual void setH(int); };
class Square : Rectangle {
    void setW(int w) override { width = height = w; }   // pha ky vong "doi W khong doi H"
};
```

⭐ **Vi phạm LSP nguy hiểm hơn vi phạm các nguyên lý khác** — bốn nguyên lý kia hỏng thì code *khó sửa*; LSP hỏng thì code **chạy ra kết quả sai** qua đúng con đường đa hình mà bạn tin tưởng.

**Áp vào hệ display:** nếu `OledDimming` âm thầm bỏ qua `setTarget()` vì panel OLED không có backlight, đó là vi phạm LSP — lớp trên tưởng đã đặt xong. Đúng cách: trả **mã lỗi tường minh** (`-ENOTSUP`), tức biến "không làm gì" thành **một phần của hợp đồng** — chính là [Null Object](behavioral.md).

---

## 4. ISP — Interface Segregation

| | |
|---|---|
| **Bản chất** | Đừng ép client phụ thuộc vào method nó **không dùng** |
| **Lực nó chống** | Interface "béo" gom mọi khả năng, khiến impl đơn giản phải viết hàm rỗng |
| **Dấu hiệu vi phạm** | Impl có hàm chỉ `return -ENOTSUP;` **hàng loạt** · thêm method vào interface làm 5 class không liên quan phải sửa |
| **Cái giá khi áp** | Nhiều interface nhỏ ⟹ nhiều mảnh hơn để ghép |

```cpp
// ❌ Máy in đơn giản bị ép biết fax và scan
struct IMachine { virtual void print()=0; virtual void scan()=0; virtual void fax()=0; };

// ✅ Client chỉ phụ thuộc cái nó cần
struct IPrinter { virtual void print()=0; };
struct IScanner { virtual void scan()=0; };
```

🔴 **ISP đắt hơn bình thường khi interface đi qua ranh giới `.so`.** Ở đó, thêm một method vào interface **không chỉ** làm impl phải sửa — nó **đổi vtable**, tức là **ABI break**, và mọi `.so` đã build trước đó thành không dùng được ([in-practice/02 §3.3](in-practice/02-interface-impl-plugin.md)). Nói cách khác: **interface càng béo thì càng nhiều lý do phải đổi nó, mà mỗi lần đổi ở đây đắt gấp bội.**

> ⚖️ **Căng thẳng thật giữa ISP và Null Object.** Null Object *cố ý* cho phép impl không cài đặt một số API (rơi về `-ENOTSUP`) — nhìn qua là "interface béo". Ranh giới: **vài** API tuỳ chọn trên một interface gắn kết là chấp nhận được và mua được tương thích; **phần lớn** API trả `-ENOTSUP` nghĩa là bạn đã gộp hai thiết bị khác nhau vào một interface — lúc đó ISP thắng, phải tách.

---

## 5. DIP — Dependency Inversion

| | |
|---|---|
| **Bản chất** | Module cấp cao **không** phụ thuộc module cấp thấp; **cả hai** phụ thuộc abstraction |
| **Lực nó chống** | Logic nghiệp vụ dính cứng vào một implementation cụ thể ⟹ không đổi được, không test được |
| **Dấu hiệu vi phạm** | Class nghiệp vụ `#include` header driver · không viết được unit test nếu không có phần cứng |
| **Cái giá khi áp** | Một interface + phải quyết định **ai tiêm** implementation vào |

```cpp
// ❌ Gắn cứng
class ReportService { MySQLDatabase db; };

// ✅ Phụ thuộc abstraction, implementation được TIÊM vào
struct IDatabase { virtual void save(const Data&) = 0; virtual ~IDatabase() = default; };
class ReportService {
    IDatabase& db_;
public:
    explicit ReportService(IDatabase& d) : db_(d) {}   // dependency injection
};
```

⭐ **"Đảo ngược" ở đây đảo cái gì?** Không phải đảo chiều lời gọi — cấp cao vẫn gọi xuống. **Đảo chiều SỞ HỮU HỢP ĐỒNG:** trước đây cấp thấp định nghĩa API và cấp cao phải theo; giờ **cấp cao định nghĩa interface nó cần**, cấp thấp phải khớp vào. Nói được đúng câu này là khác biệt giữa 3 điểm và 4 điểm ở [DP-011](../14-prep/mock-interview/bank/design-patterns.md).

**Đây là nguyên lý sát công việc của bạn nhất.** HAL chính là DIP đóng gói thành kiến trúc: app phụ thuộc `IDisplay` (abstraction), `.so` cũng phụ thuộc `IDisplay`, và **không bên nào phụ thuộc bên kia**. Nhờ vậy một binary chạy nhiều board ([in-practice/02](in-practice/02-interface-impl-plugin.md)).

---

## 6. Từ nguyên lý ra pattern — bảng ánh xạ

Đây là chỗ SOLID nối vào ba file còn lại của topic. Đọc bảng này rồi thì pattern không còn là danh sách phải học thuộc.

| Nguyên lý | Vấn đề cụ thể | Pattern hiện thực hoá |
|---|---|---|
| **OCP** | Thêm thuật toán mới không sửa context | [Strategy](behavioral.md) · [Template Method](behavioral.md) |
| **OCP + DIP** | Client không được biết class cụ thể | [Factory Method](creational.md) · [Abstract Factory](creational.md) |
| **DIP** | Cấp cao không dính cấp thấp; test được không cần HW | Interface + DI · [Bridge](structural.md) |
| **SRP** | Hai chiều biến thiên trộn vào một cây kế thừa | [Bridge](structural.md) |
| **SRP + OCP** | Ai quan tâm thì tự đăng ký, subject không cần biết | [Observer](behavioral.md) |
| **LSP** | "Không hỗ trợ" phải là hợp đồng, không phải im lặng | [Null Object](behavioral.md) |
| **SRP** | Tách *thời điểm phát lệnh* khỏi *thời điểm thi hành* | [Command](behavioral.md) |
| **SRP (encapsulation)** | Chụp state mà không phơi bên trong | [Memento](behavioral.md) |
| **ISP + information hiding** | Header không được lộ chi tiết triển khai | [Pimpl](structural.md) · [07/api-design](../07-shared-libraries/api-design.md) |

---

## 7. Khi nào KHÔNG áp SOLID

Phần này quan trọng ngang 5 phần trên — và là câu hỏi 🔴 [DP-012](../14-prep/mock-interview/bank/design-patterns.md).

**Chi phí của một abstraction, kể cụ thể:**

| Chi phí | Cụ thể |
|---|---|
| Đọc hiểu | Thêm file phải mở, thêm một lần "nhảy" để tìm code thật sự chạy |
| Runtime | Virtual call không inline được (~vài ns) + con trỏ vtable mỗi object |
| **ABI** | Interface qua `.so` = **hợp đồng nhị phân giữ vĩnh viễn** — đắt nhất, và hay bị quên khi tính |
| Nhận thức | Abstraction **nói dối** nếu chỉ có một implementation: người đọc sau đi tìm biến thể không tồn tại |

**Ba câu hỏi lọc — "không" ở bất kỳ câu nào thì đừng trừu tượng hoá:**
1. Đã có **≥ 2 biến thể thật** chưa (không phải "sau này biết đâu")?
2. Chúng khác nhau về **hành vi**, hay chỉ khác **tham số**? *(Chỉ khác tham số ⟹ truyền tham số.)*
3. Có ai thật sự cần **hoán đổi** chúng không?

> **Ví dụ đối chứng trong chính hệ display:** `dimming` đạt cả ba ⟹ interface + factory. `frc`/`tcon` trượt cả ba ⟹ **để nguyên hàm gọi thẳng**. Nói được chỗ mình *cố tình không* áp SOLID là tín hiệu senior mạnh hơn kể tên năm pattern ([in-practice/01 §4](in-practice/01-display-stack.md)).

**Riêng embedded, thêm hai lưu ý:** ① virtual trên đường **mỗi khung hình** thì đáng đo, trên đường **cấu hình** thì miễn phí — chi phí nằm ở *tần suất gọi*, không ở việc có dùng abstraction hay không; ② **cấm cấp phát động trên đường nóng** kể cả khi pattern gợi ý làm vậy.

> **Kim chỉ nam:** SOLID phục vụ mục tiêu *dễ thay đổi an toàn*. Đoạn code không cần thay đổi thì đừng tốn công trừu tượng hoá — và hãy nói ra rằng đó là **quyết định**, không phải sơ suất.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-001](../14-prep/mock-interview/bank/design-patterns.md) | SOLID là gì? |
| [DP-008](../14-prep/mock-interview/bank/design-patterns.md) | Cho ví dụ vi phạm Liskov Substitution Principle. |
| [DP-011](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | Logic đọc mã vạch gọi thẳng driver I2C, phải test được không cần phần cứng — DIP "đảo ngược" cái gì? |
| [DP-012](../14-prep/mock-interview/bank/design-patterns.md) | Khi nào KHÔNG nên dùng design pattern / áp SOLID? |
| [DP-024](../14-prep/mock-interview/bank/design-patterns.md) ⭐ | `frc`/`tcon` chỉ một command cố định — vì sao KHÔNG bọc pattern? Nêu cái giá cụ thể. |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [creational.md](creational.md)
