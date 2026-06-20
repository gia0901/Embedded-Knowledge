# Behavioral Patterns — Strategy, Observer, State, Command

> **TL;DR**
> - Behavioral pattern giải quyết **cách object tương tác & phân chia trách nhiệm/hành vi**.
> - **Strategy**: đóng gói các thuật toán hoán đổi được sau một interface → đổi hành vi lúc runtime. C++ hiện đại thường dùng `std::function`/lambda.
> - **Observer**: khi một object đổi trạng thái, tự động thông báo các "người quan sát" đăng ký → event/callback (rất phổ biến trong embedded: sự kiện sensor, GPIO).
> - **State**: object đổi hành vi khi trạng thái nội bộ đổi → state machine (cốt lõi của firmware/protocol).
> - **Command**: đóng gói một yêu cầu thành object → queue, undo, log, gửi qua IPC.
> - Lưu ý embedded: ưu tiên giải pháp nhẹ (function pointer, enum state machine) thay vì hệ phân cấp virtual nặng khi tài nguyên hạn chế.

---

## 1. Strategy — thuật toán hoán đổi được

Đóng gói mỗi thuật toán/hành vi sau một interface chung, cho phép chọn/đổi lúc runtime mà không sửa code dùng.

```cpp
// OOP cổ điển
struct ICompressor { virtual std::vector<uint8_t> compress(const Data&) = 0; };
struct ZipCompressor : ICompressor { ... };
struct LzwCompressor : ICompressor { ... };

class Archiver {
    ICompressor& strategy_;          // chọn thuật toán qua interface
public:
    Archiver(ICompressor& s) : strategy_(s) {}
    void save(const Data& d) { auto c = strategy_.compress(d); ... }
};

// C++ hiện đại: thường gọn hơn với std::function/lambda
class Archiver2 {
    std::function<std::vector<uint8_t>(const Data&)> compress_;
public:
    Archiver2(std::function<...> f) : compress_(std::move(f)) {}
};
// Archiver2 a([](const Data& d){ return zip(d); });
```

- Là hiện thực trực tiếp của Open/Closed + Dependency Inversion.
- **C++ hiện đại:** với strategy đơn giản (một hàm), `std::function`/lambda/template gọn hơn nhiều so với hệ phân cấp class ([lambdas-functional](../02-modern-cpp/lambdas-functional.md)).

---

## 2. Observer — thông báo khi trạng thái đổi

Một "subject" giữ danh sách "observer"; khi subject đổi trạng thái, nó **thông báo tất cả** observer đã đăng ký. Đây là nền của mô hình event-driven/callback.

```cpp
struct IObserver { virtual void onTemperatureChanged(double) = 0; };

class TemperatureSensor {                         // subject
    std::vector<IObserver*> observers_;
public:
    void subscribe(IObserver* o)   { observers_.push_back(o); }
    void notifyAll(double temp) {                 // báo mọi observer
        for (auto* o : observers_) o->onTemperatureChanged(temp);
    }
};
```

- Tách subject khỏi observer: subject không cần biết cụ thể ai lắng nghe → coupling thấp, thêm observer mới dễ.
- Cực phổ biến trong **embedded/UI**: sự kiện sensor, nhấn nút, GPIO interrupt → thông báo các thành phần quan tâm.
- **Lưu ý lifetime**: observer phải hủy đăng ký trước khi bị hủy (dangling pointer). Dùng `weak_ptr` hoặc cơ chế unsubscribe an toàn.

---

## 3. State — máy trạng thái

Object thay đổi **hành vi** khi trạng thái nội bộ thay đổi — như thể đổi class. Là cách hiện thực hóa **state machine**, cốt lõi của firmware, giao thức, UI flow.

```cpp
// Cách nhẹ (hợp embedded): enum + switch — tất định, không heap, không virtual
enum class State { Idle, Running, Error };
class Machine {
    State state_ = State::Idle;
public:
    void event(Event e) {
        switch (state_) {
            case State::Idle:    if (e == Event::Start) state_ = State::Running; break;
            case State::Running: if (e == Event::Fault) state_ = State::Error;
                                 else if (e == Event::Stop) state_ = State::Idle; break;
            case State::Error:   if (e == Event::Reset) state_ = State::Idle; break;
        }
    }
};

// Cách OOP (State pattern thuần): mỗi state là một class, xử lý sự kiện + chuyển state
// → linh hoạt khi logic mỗi state phức tạp, nhưng nặng hơn (virtual, object mỗi state)
```

- **Embedded thường ưu tiên enum + switch/table** (tất định, không cấp phát động, dễ đọc) thay vì State pattern OOP đầy đủ — đúng tinh thần [constraints](../08-embedded-systems/constraints.md).
- State machine rõ ràng giúp tránh "trạng thái lỡ" và là cách xử lý giao thức/firmware đáng tin cậy ([system-design](../10-thinking/system-design.md)).

---

## 4. Command — đóng gói yêu cầu thành object

Bọc một yêu cầu (hành động + tham số) thành một object → có thể lưu, xếp hàng, hoàn tác (undo), ghi log, hoặc gửi đi.

```cpp
struct ICommand { virtual void execute() = 0; virtual ~ICommand() = default; };
struct MoveCommand : ICommand {
    Motor& m; int steps;
    void execute() override { m.move(steps); }
};
// Xếp hàng lệnh, thực thi sau, hoặc gửi qua IPC/queue
std::queue<std::unique_ptr<ICommand>> commandQueue;
```

- Tách *người phát lệnh* khỏi *người thực thi*; cho phép undo/redo (lưu lịch sử command), macro, hoãn thực thi.
- Trong embedded: hàng đợi command giữa các thread/process, hoặc giữa core (RTOS task gửi lệnh) — kết hợp message queue ([ipc](../03-operating-system/ipc.md)).

---

## 5. Khi nào KHÔNG dùng pattern (quan trọng)

- Vấn đề đơn giản → một hàm/lambda là đủ; nhồi pattern làm code khó hơn.
- C++ hiện đại thay nhiều pattern bằng tính năng ngôn ngữ: `std::function`/lambda (Strategy, Command đơn giản), template (Strategy compile-time), `std::variant` + `std::visit` (một số State/Visitor).
- Embedded: cân nhắc chi phí virtual/heap; thường có bản "nhẹ" (function pointer, enum table) cho cùng ý tưởng.

> Pattern là **từ vựng chung** để trao đổi thiết kế ("dùng Observer ở đây") và là giải pháp đã được kiểm chứng — nhưng chỉ áp dụng khi vấn đề thật sự khớp.

---

## Câu hỏi phỏng vấn liên quan

<details><summary>1) Strategy pattern là gì? C++ hiện đại hiện thực thế nào cho gọn?</summary>

Strategy đóng gói mỗi thuật toán/hành vi sau một interface chung và cho phép chọn hoặc hoán đổi chúng lúc runtime mà không sửa code sử dụng — ví dụ các thuật toán nén khác nhau dùng chung interface `compress`. Nó là hiện thực trực tiếp của Open/Closed (thêm thuật toán mới không sửa client) và Dependency Inversion (client phụ thuộc interface). Cách cổ điển là một hệ phân cấp class với interface abstract; nhưng C++ hiện đại, với strategy đơn giản (chỉ là một hàm), thường dùng `std::function` + lambda gọn hơn nhiều: lưu một `std::function` làm member và truyền lambda vào constructor, không cần định nghĩa cả cây class. Khi cần hiệu năng và biết lúc compile, có thể dùng template parameter cho strategy (zero overhead).
</details>

<details><summary>2) Observer pattern dùng khi nào? Rủi ro cần lưu ý?</summary>

Observer dùng khi một object (subject) thay đổi trạng thái và cần tự động thông báo nhiều object khác (observer) quan tâm, mà không gắn chặt với chúng — nền của mô hình event-driven/callback, rất phổ biến trong embedded và UI (sự kiện sensor, nhấn nút, GPIO interrupt thông báo các thành phần). Subject giữ danh sách observer đăng ký và gọi callback của tất cả khi có sự kiện, nên coupling thấp và dễ thêm observer mới. Rủi ro chính là **lifetime/dangling**: nếu một observer bị hủy mà chưa hủy đăng ký khỏi subject, subject sẽ gọi vào con trỏ chết → UB; cần cơ chế unsubscribe an toàn hoặc dùng `weak_ptr` để subject kiểm tra observer còn sống. Ngoài ra cần cẩn thận thứ tự thông báo và tránh vòng lặp thông báo vô tận.
</details>

<details><summary>3) State pattern là gì? Vì sao embedded thường dùng enum + switch thay vì State pattern OOP?</summary>

State pattern cho một object thay đổi hành vi khi trạng thái nội bộ thay đổi, như thể nó đổi class — là cách hiện thực state machine, cốt lõi của firmware, giao thức và luồng UI. Bản OOP thuần biểu diễn mỗi state là một class xử lý sự kiện và quyết định chuyển state, linh hoạt khi logic mỗi state phức tạp. Tuy nhiên embedded thường ưu tiên cách nhẹ — một `enum` trạng thái với `switch`/bảng chuyển trạng thái — vì nó **tất định, không cấp phát động, không chi phí virtual**, footprint biết trước, và dễ đọc/review; rất hợp với ràng buộc tài nguyên và yêu cầu tin cậy của hệ nhúng. State machine rõ ràng (dù bằng enum) còn giúp tránh "trạng thái lỡ" và xử lý giao thức đáng tin cậy. Chọn bản OOP chỉ khi logic mỗi state đủ phức tạp để đáng tách thành class.
</details>

<details><summary>4) Command pattern giải quyết vấn đề gì?</summary>

Command đóng gói một yêu cầu (hành động cùng tham số của nó) thành một object, tách người phát lệnh khỏi người thực thi. Nhờ là object, yêu cầu có thể được lưu trữ, xếp vào hàng đợi để thực thi sau, ghi log, hoàn tác (lưu lịch sử command để undo/redo), gộp thành macro, hoặc gửi đi nơi khác. Trong embedded/hệ thống đồng thời, command thường được đẩy vào một hàng đợi giữa các thread/process hoặc giữa các core (vd RTOS task hoặc giao diện gửi lệnh tới module điều khiển), kết hợp với message queue. Nó hữu ích khi cần tách thời điểm/nơi phát lệnh khỏi thời điểm/nơi thực thi, hoặc cần khả năng undo/replay/audit các thao tác.
</details>

---
⬅️ [structural.md](structural.md) · ➡️ Tiếp theo: [13-dsa/](../13-dsa/)
