# SOLID Principles — Nền tảng thiết kế hướng đối tượng

> **TL;DR**
> - SOLID là 5 nguyên lý giúp code **dễ bảo trì, mở rộng, test** — nền tảng để hiểu *vì sao* các design pattern tồn tại (pattern thường là cách hiện thực hóa SOLID).
> - **S**RP — một class một lý do để thay đổi. **O**CP — mở để mở rộng, đóng để sửa đổi. **L**SP — lớp con thay được lớp cha. **I**SP — interface nhỏ, chuyên biệt. **D**IP — phụ thuộc vào abstraction, không phải implementation.
> - Mục tiêu chung: **giảm coupling, tăng cohesion** → đổi một chỗ không vỡ chỗ khác.
> - Đừng giáo điều: SOLID là hướng dẫn, không phải luật. Cân bằng với đơn giản (YAGNI) — nhất là trên embedded.

---

## 1. S — Single Responsibility Principle (SRP)

**Một class chỉ nên có một lý do để thay đổi** — tức làm tốt một trách nhiệm.

```cpp
// ❌ Vi phạm: class làm 3 việc → sửa định dạng log cũng đụng vào logic sensor
class Sensor {
    double read();
    void saveToFile();        // trách nhiệm lưu trữ
    void formatReport();      // trách nhiệm trình bày
};

// ✅ Tách trách nhiệm
class Sensor       { double read(); };
class SensorStorage{ void save(double); };
class SensorReport { std::string format(double); };
```

Lợi: mỗi class nhỏ, dễ hiểu/test, thay đổi cô lập. Đây cũng là "separation of concerns" ở mức class ([system-design](../10-thinking/system-design.md)).

---

## 2. O — Open/Closed Principle (OCP)

**Mở để mở rộng, đóng để sửa đổi**: thêm hành vi mới bằng *thêm code*, không *sửa code cũ* (đã chạy đúng, đã test).

```cpp
// ❌ Thêm loại hình mới phải sửa hàm cũ (rủi ro phá vỡ cái đang chạy)
double area(const Shape& s) {
    if (s.type == CIRCLE) ...
    else if (s.type == SQUARE) ...   // mỗi loại mới → sửa ở đây
}

// ✅ Mở rộng qua abstraction: thêm loại mới = thêm class, không đụng code cũ
struct Shape { virtual double area() const = 0; };
struct Circle : Shape { double area() const override; };
struct Square : Shape { double area() const override; };   // thêm class mới là xong
```

Đạt được nhờ polymorphism/template. Là lý do tồn tại của Strategy, Factory...

---

## 3. L — Liskov Substitution Principle (LSP)

**Object lớp con phải thay thế được lớp cha mà không phá vỡ tính đúng đắn.** Lớp con không được làm yếu hợp đồng của lớp cha (không siết tiền điều kiện, không nới hậu điều kiện, không ném exception bất ngờ).

```cpp
// ❌ Vi phạm kinh điển: Square is-a Rectangle?
class Rectangle { virtual void setW(int); virtual void setH(int); };
class Square : Rectangle {  // setW cũng đổi H → phá kỳ vọng "đổi W không đổi H"
    void setW(int w) override { width = height = w; }   // bất ngờ với code dùng Rectangle
};
```

Kế thừa phải là quan hệ **is-a thực sự về hành vi**, không chỉ về dữ liệu. Vi phạm LSP → đa hình cho kết quả sai. Khi nghi ngờ, ưu tiên composition.

---

## 4. I — Interface Segregation Principle (ISP)

**Đừng ép client phụ thuộc vào method nó không dùng.** Nhiều interface nhỏ, chuyên biệt hơn một interface "béo".

```cpp
// ❌ Interface béo: máy in đơn giản bị ép implement cả fax/scan
struct IMachine { virtual void print()=0; virtual void scan()=0; virtual void fax()=0; };

// ✅ Tách nhỏ — client chỉ phụ thuộc cái cần
struct IPrinter { virtual void print()=0; };
struct IScanner { virtual void scan()=0; };
// Máy đa năng implement nhiều interface; máy in đơn giản chỉ IPrinter
```

Giảm coupling: thay đổi một khả năng không ảnh hưởng client không dùng nó.

---

## 5. D — Dependency Inversion Principle (DIP)

**Module cấp cao không phụ thuộc module cấp thấp; cả hai phụ thuộc abstraction.** Phụ thuộc vào *interface*, không vào *implementation cụ thể*.

```cpp
// ❌ Logic cấp cao gắn cứng với implementation cụ thể
class ReportService {
    MySQLDatabase db;     // gắn chặt MySQL — khó đổi, khó test
};

// ✅ Phụ thuộc abstraction, inject implementation
struct IDatabase { virtual void save(const Data&) = 0; };
class ReportService {
    IDatabase& db;        // không biết là MySQL hay mock
public:
    ReportService(IDatabase& d) : db(d) {}   // dependency injection
};
```

Cho phép **thay implementation** (MySQL → SQLite → mock để test) mà không sửa logic cấp cao. Nền tảng của testability ([system-design](../10-thinking/system-design.md)) và là tinh thần của nhiều pattern (Strategy, hardware abstraction layer trong embedded).

---

## 6. Cân bằng: SOLID vs đơn giản

SOLID giảm coupling nhưng thêm abstraction (interface, lớp) → phức tạp hơn. **Đừng áp dụng giáo điều**:
- Code nhỏ, ổn định, không cần linh hoạt → abstraction thừa là over-engineering.
- Embedded: mỗi interface/virtual có chi phí (vtable, gián tiếp) — cân nhắc.
- Áp SOLID **khi có nhu cầu thay đổi/mở rộng thực sự**, không phải "để cho đúng chuẩn".

> Kim chỉ nam: SOLID phục vụ mục tiêu *dễ thay đổi an toàn*; nếu một đoạn code không cần thay đổi thì đừng tốn công trừu tượng hóa nó.

---

## Câu hỏi phỏng vấn liên quan

> Đáp án sống trong [bank/](../14-prep/mock-interview/bank/) — **một đáp án, một chỗ** ([CLAUDE.md §4.7](../CLAUDE.md)). Tự trả lời trước khi mở.

| ID | Câu hỏi |
|----|---------|
| [DP-001](../14-prep/mock-interview/bank/design-patterns.md) | SOLID là gì? Mục tiêu chung của 5 nguyên lý? |
| [DP-008](../14-prep/mock-interview/bank/design-patterns.md) | Cho ví dụ vi phạm Liskov Substitution Principle. |
| [DP-011](../14-prep/mock-interview/bank/design-patterns.md) | Dependency Inversion Principle giúp ích gì cho việc test? |
| [DP-012](../14-prep/mock-interview/bank/design-patterns.md) | Áp dụng SOLID có nhược điểm gì? Khi nào không nên? |

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [creational.md](creational.md)
