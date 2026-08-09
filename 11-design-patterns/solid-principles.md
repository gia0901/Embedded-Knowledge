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

<details><summary>1) SOLID là gì? Mục tiêu chung của 5 nguyên lý?</summary>

SOLID là 5 nguyên lý thiết kế hướng đối tượng: Single Responsibility (một class một trách nhiệm/lý do thay đổi), Open/Closed (mở để mở rộng, đóng để sửa đổi), Liskov Substitution (lớp con thay được lớp cha về hành vi), Interface Segregation (interface nhỏ chuyên biệt thay vì béo), Dependency Inversion (phụ thuộc abstraction chứ không implementation cụ thể). Mục tiêu chung là **giảm coupling và tăng cohesion** để code dễ bảo trì, mở rộng và test — thay đổi một chỗ không làm vỡ chỗ khác. Đây là nền tảng để hiểu vì sao nhiều design pattern tồn tại (pattern thường là cách hiện thực hóa các nguyên lý này).
</details>

<details><summary>2) Cho ví dụ vi phạm Liskov Substitution Principle.</summary>

Ví dụ kinh điển là Square kế thừa Rectangle: Rectangle có `setWidth` và `setHeight` độc lập, nhưng Square buộc width = height nên override `setWidth` để đổi cả hai. Code viết cho Rectangle giả định "đổi width không đổi height" sẽ cho kết quả sai khi nhận một Square — lớp con đã phá vỡ hợp đồng hành vi của lớp cha. Đây là vi phạm LSP: dù Square "is-a" Rectangle về mặt dữ liệu, nó không thay thế được Rectangle về mặt hành vi. Bài học: kế thừa phải là quan hệ is-a thực sự về hành vi, không chỉ dữ liệu; khi nghi ngờ nên dùng composition.
</details>

<details><summary>3) Dependency Inversion Principle giúp ích gì cho việc test?</summary>

DIP yêu cầu module cấp cao phụ thuộc vào abstraction (interface) thay vì implementation cụ thể, và inject implementation từ ngoài vào. Nhờ đó khi test, ta có thể thay implementation thật (vd database MySQL, driver phần cứng) bằng một mock/stub triển khai cùng interface, và kiểm thử logic cấp cao độc lập, nhanh, lặp lại được mà không cần tài nguyên thật. Đây chính là nền tảng của hardware abstraction layer trong embedded: logic gọi qua interface nên test được trên host với phần cứng giả lập. Không có DIP, logic gắn chặt với implementation cụ thể sẽ rất khó test.
</details>

<details><summary>4) Áp dụng SOLID có nhược điểm gì? Khi nào không nên?</summary>

SOLID giảm coupling nhưng đánh đổi bằng việc thêm abstraction — nhiều interface, lớp, tầng gián tiếp — làm code phức tạp hơn và khó theo dõi luồng hơn. Không nên áp dụng giáo điều: với code nhỏ, ổn định, không có nhu cầu thay đổi/mở rộng thực sự thì thêm abstraction là over-engineering, vi phạm YAGNI. Trong embedded còn phải tính chi phí runtime: mỗi interface/virtual function thêm vtable và một lần gọi gián tiếp, có thể đáng kể trên hệ hạn chế tài nguyên hoặc hot path. Nguyên tắc là áp dụng SOLID khi thực sự cần linh hoạt/dễ thay đổi an toàn, và cân bằng với sự đơn giản.
</details>

---
⬅️ [Về index topic](README.md) · ➡️ Tiếp theo: [creational.md](creational.md)
