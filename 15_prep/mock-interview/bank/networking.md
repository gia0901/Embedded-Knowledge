# NET — Networking

> Domain `NET`. Từ [11/networking.md](../../../11-interview-questions/networking.md). Track dùng: `networking`, `cpp-system`, `bsp`.

---

#### NET-001 · 🟢 · concept · [→ tcp-ip](../../../14-networking/tcp-ip.md)
**TCP và UDP khác nhau thế nào? Khi nào chọn cái nào?**
<details><summary>Đáp án</summary>

TCP: hướng kết nối (handshake trước), đảm bảo tới nơi/đúng thứ tự/không trùng, có flow & congestion control, luồng byte, overhead lớn hơn. UDP: không kết nối, không đảm bảo, không kiểm soát, datagram rời rạc, nhẹ/nhanh/độ trễ thấp. Chọn TCP khi cần dữ liệu nguyên vẹn đúng thứ tự (web, file, SSH); UDP khi cần độ trễ thấp, chấp nhận mất gói (video/voice, game, DNS, streaming sensor).
</details>

#### NET-002 · 🟢 · concept · [→ tcp-ip](../../../14-networking/tcp-ip.md)
**Vì sao mạng tổ chức theo tầng (layering)?**
<details><summary>Đáp án</summary>

Để chia độ phức tạp: mỗi tầng một trách nhiệm và che giấu chi tiết khỏi tầng trên. Mô hình TCP/IP: Link (khung trên môi trường vật lý), Internet/IP (định tuyến packet qua IP), Transport (TCP/UDP qua port), Application (HTTP...). Lợi: thay đổi một tầng không ảnh hưởng tầng khác, phát triển độc lập, dễ chuẩn hóa. Dữ liệu đi xuống được mỗi tầng đóng gói thêm header (encapsulation).
</details>

#### NET-003 · 🟢 · concept · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md)
**Socket là gì? Luồng tạo TCP server?**
<details><summary>Đáp án</summary>

Socket là endpoint giao tiếp, tổng quát hóa file descriptor cho mạng (cùng read/write/close). Server: `socket → bind (gắn IP:port) → listen → accept (chờ client, trả fd mới) → read/write → close`. Client: `socket → connect → read/write → close`.
</details>

#### NET-004 · 🟡 · concept · [→ tcp-ip](../../../14-networking/tcp-ip.md)
**Mô tả TCP three-way handshake. Vì sao cần ba bước?**
<details><summary>Đáp án</summary>

Client gửi SYN (seq=x); server đáp SYN-ACK (seq=y, ack=x+1); client gửi ACK (ack=y+1) → kết nối thiết lập. Cần ba bước để cả hai bên trao đổi và xác nhận sequence number khởi đầu của nhau theo cả hai chiều, đảm bảo cùng sẵn sàng và đồng bộ trước khi truyền. Hai bước không đủ vì chiều server→client chưa được xác nhận. Đóng dùng four-way.
</details>

#### NET-005 · 🟡 · concept · [→ tcp-ip](../../../14-networking/tcp-ip.md)
**TCP đảm bảo tin cậy trên nền IP best-effort bằng cách nào?**
<details><summary>Đáp án</summary>

IP không đảm bảo gì; TCP tự xây tin cậy: mỗi byte có sequence number, bên nhận gửi ACK, không ACK thì retransmit; bên nhận sắp xếp lại theo sequence number và loại trùng. Thêm flow control (cửa sổ nhận) và congestion control (điều tiết theo tắc nghẽn mạng).
</details>

#### NET-006 · 🟡 · concept · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md)
**HTTP "stateless" nghĩa là gì?**
<details><summary>Đáp án</summary>

Mỗi request độc lập, server không tự nhớ trạng thái giữa các request trong giao thức. Trạng thái (đăng nhập...) duy trì qua cookie/session token/JWT gửi kèm mỗi request. Tính stateless giúp HTTP đơn giản và dễ scale ngang (request đi tới server nào cũng được), đổi lại phải truyền thông tin trạng thái mỗi lần.
</details>

#### NET-007 · 🟠 · concept · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md)
**TCP là luồng byte — gây vấn đề gì khi lập trình? Giải quyết?**
<details><summary>Đáp án</summary>

Không có ranh giới message: một `read` có thể nhận một phần message, nhiều message gộp, hoặc một message rưỡi — không tương ứng một-một với `write` của bên gửi. Giả định "một read = một message" sẽ lỗi. Giải bằng framing: length prefix hoặc delimiter, hoặc giao thức tự mô tả độ dài (HTTP dùng Content-Length); cộng xử lý short read và buffer phần dư.
</details>

#### NET-008 · 🟠 · concept · [→ tcp-ip](../../../14-networking/tcp-ip.md)
**Phân biệt flow control và congestion control.**
<details><summary>Đáp án</summary>

Cả hai điều tiết tốc độ gửi nhưng bảo vệ đối tượng khác. Flow control bảo vệ **bên nhận**: bên nhận báo cửa sổ nhận (chỗ trống trong bộ đệm) để bên gửi không gửi nhanh hơn nó xử lý. Congestion control bảo vệ **mạng**: qua slow start/congestion avoidance, TCP điều chỉnh tốc độ theo mức tắc nghẽn (suy từ mất gói/độ trễ) để không làm nghẽn router.
</details>

#### NET-009 · 🟠 · design · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md), [io-multiplexing](../../../04-linux-system-programming/io-multiplexing.md)
**Server xử lý hàng nghìn kết nối đồng thời thế nào?**
<details><summary>Đáp án</summary>

Không dùng một thread blocking mỗi kết nối (tốn RAM + context switch). Dùng mô hình event-driven: socket non-blocking + epoll để một thread theo dõi nhiều fd, chỉ xử lý cái sẵn sàng. Vòng lặp `epoll_wait` → accept kết nối mới hoặc đọc/ghi non-blocking (Nginx/Redis). Nguyên tắc: không block trong loop; tác vụ CPU nặng đẩy sang thread pool; mở rộng bằng nhiều loop trên nhiều core.
</details>

#### NET-010 · 🔴 · concept · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md)
**Điều gì xảy ra khi bạn gõ một URL vào trình duyệt?**
<details><summary>Đáp án</summary>

(1) DNS phân giải tên miền → IP (chủ yếu UDP). (2) TCP three-way handshake tới IP:443. (3) TLS handshake: thỏa thuận mã hóa, xác thực certificate, tạo khóa phiên. (4) Gửi HTTP request (GET / + headers). (5) Server xử lý, trả HTTP response. (6) Trình duyệt render, tải thêm tài nguyên (CSS/JS/ảnh — mỗi cái lặp quy trình). (7) Keep-alive hoặc đóng. Câu này xâu chuỗi toàn bộ stack DNS→TCP→TLS→HTTP→ứng dụng.
</details>

#### NET-011 · 🔴 · concept · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md)
**TLS cung cấp gì? HTTPS hoạt động ở mức cao thế nào?**
<details><summary>Đáp án</summary>

TLS cung cấp ba đảm bảo: bảo mật (mã hóa), toàn vẹn (phát hiện sửa đổi), xác thực (xác minh danh tính qua certificate). HTTPS = HTTP trên TLS. Sau khi thiết lập TCP, TLS handshake: server gửi certificate (CA ký) để client xác thực, hai bên thỏa thuận thuật toán và trao đổi tạo **khóa phiên đối xứng**, rồi toàn bộ dữ liệu HTTP được mã hóa. Embedded thường dùng mbedTLS.
</details>

#### NET-012 · 🔴 · concept · [→ sockets-and-protocols](../../../14-networking/sockets-and-protocols.md)
**Giao thức nào phù hợp IoT/embedded và vì sao?**
<details><summary>Đáp án</summary>

MQTT phổ biến nhất: pub/sub nhẹ trên TCP, thiết bị publish lên broker, bên quan tâm subscribe theo topic — tiết kiệm băng thông, hỗ trợ mạng chập chờn (QoS, last-will), tách rời gửi/nhận. CoAP: giống HTTP nhưng trên UDP, cực nhẹ. Lý do chung: HTTP + TCP stack đầy đủ quá nặng về băng thông/bộ nhớ/điện cho nhiều thiết bị nhúng; giao thức IoT tối giản để vừa tài nguyên và chịu mạng không ổn định. Embedded cũng dùng stack nhẹ (lwIP + mbedTLS).
</details>

---
⬅️ [Bank index](README.md)
