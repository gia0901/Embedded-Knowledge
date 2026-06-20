# Câu hỏi phỏng vấn — Networking

> Topic gốc: [14 Networking](../14-networking/). Tự trả lời trước khi mở đáp án.

---

## 🟢 Cơ bản

<details><summary>1) TCP và UDP khác nhau thế nào? Khi nào chọn cái nào?</summary>

TCP: hướng kết nối (handshake trước), đảm bảo tới nơi/đúng thứ tự/không trùng, có flow & congestion control, luồng byte, overhead lớn hơn. UDP: không kết nối, không đảm bảo, không kiểm soát, datagram rời rạc, nhẹ/nhanh/độ trễ thấp. Chọn TCP khi cần dữ liệu nguyên vẹn đúng thứ tự (web, file, SSH); UDP khi cần độ trễ thấp, chấp nhận mất gói (video/voice call, game, DNS, streaming sensor). → [tcp-ip](../14-networking/tcp-ip.md)
</details>

<details><summary>2) Vì sao mạng tổ chức theo tầng (layering)?</summary>

Để chia độ phức tạp: mỗi tầng một trách nhiệm và che giấu chi tiết khỏi tầng trên (separation of concerns). Mô hình TCP/IP: Link (khung trên môi trường vật lý), Internet/IP (định tuyến packet qua IP), Transport (TCP/UDP qua port), Application (HTTP...). Lợi: thay đổi một tầng không ảnh hưởng tầng khác, phát triển độc lập, dễ chuẩn hóa. Dữ liệu đi xuống được mỗi tầng đóng gói thêm header (encapsulation). → [tcp-ip](../14-networking/tcp-ip.md)
</details>

<details><summary>3) Socket là gì? Luồng tạo TCP server?</summary>

Socket là endpoint giao tiếp, tổng quát hóa file descriptor cho mạng (cùng read/write/close). Server: `socket → bind (gắn IP:port) → listen → accept (chờ client, trả fd mới) → read/write → close`. Client: `socket → connect → read/write → close`. → [sockets-and-protocols](../14-networking/sockets-and-protocols.md)
</details>

---

## 🟡 Trung bình

<details><summary>4) Mô tả TCP three-way handshake. Vì sao cần ba bước?</summary>

Client gửi SYN (seq=x); server đáp SYN-ACK (seq=y, ack=x+1); client gửi ACK (ack=y+1) → kết nối thiết lập. Cần ba bước để cả hai bên trao đổi và xác nhận sequence number khởi đầu của nhau theo cả hai chiều, đảm bảo cùng sẵn sàng và đồng bộ trước khi truyền (TCP đánh số từng byte). Hai bước không đủ vì chiều server→client chưa được xác nhận. Đóng dùng four-way (mỗi hướng đóng độc lập). → [tcp-ip](../14-networking/tcp-ip.md)
</details>

<details><summary>5) TCP đảm bảo tin cậy trên nền IP best-effort bằng cách nào?</summary>

IP không đảm bảo gì; TCP tự xây tin cậy: mỗi byte có sequence number, bên nhận gửi ACK, không ACK thì retransmit; bên nhận sắp xếp lại theo sequence number và loại trùng. Thêm flow control (cửa sổ nhận — không gửi nhanh hơn bên nhận xử lý) và congestion control (điều tiết theo tắc nghẽn mạng). → [tcp-ip](../14-networking/tcp-ip.md)
</details>

<details><summary>6) HTTP "stateless" nghĩa là gì?</summary>

Mỗi request độc lập, server không tự nhớ trạng thái giữa các request trong giao thức. Trạng thái (đăng nhập...) duy trì qua cookie/session token/JWT gửi kèm mỗi request. Tính stateless giúp HTTP đơn giản và dễ scale ngang (request đi tới server nào cũng được), đổi lại phải truyền thông tin trạng thái mỗi lần. → [sockets-and-protocols](../14-networking/sockets-and-protocols.md)
</details>

---

## 🟠 Khó

<details><summary>7) TCP là luồng byte — gây vấn đề gì khi lập trình? Giải quyết?</summary>

Không có ranh giới message: một `read` có thể nhận một phần message, nhiều message gộp, hoặc một message rưỡi — không tương ứng một-một với `write` của bên gửi. Giả định "một read = một message" sẽ lỗi. Giải bằng framing: length prefix (tiền tố độ dài) hoặc delimiter, hoặc giao thức tự mô tả độ dài (HTTP dùng Content-Length); cộng xử lý short read (lặp tới đủ frame) và buffer phần dư. → [sockets-and-protocols](../14-networking/sockets-and-protocols.md)
</details>

<details><summary>8) Phân biệt flow control và congestion control.</summary>

Cả hai điều tiết tốc độ gửi nhưng bảo vệ đối tượng khác. Flow control bảo vệ **bên nhận**: bên nhận báo cửa sổ nhận (chỗ trống trong bộ đệm) để bên gửi không gửi nhanh hơn nó xử lý. Congestion control bảo vệ **mạng**: qua slow start/congestion avoidance, TCP điều chỉnh tốc độ theo mức tắc nghẽn (suy từ mất gói/độ trễ) để không làm nghẽn router và sụp throughput chung. → [tcp-ip](../14-networking/tcp-ip.md)
</details>

<details><summary>9) Server xử lý hàng nghìn kết nối đồng thời thế nào?</summary>

Không dùng một thread blocking mỗi kết nối (tốn RAM + context switch). Dùng mô hình event-driven: socket non-blocking + epoll để một thread theo dõi nhiều fd, chỉ xử lý cái sẵn sàng. Vòng lặp `epoll_wait` → accept kết nối mới hoặc đọc/ghi non-blocking — một thread phục vụ rất nhiều kết nối (Nginx/Redis). Nguyên tắc: không block trong loop; tác vụ CPU nặng đẩy sang thread pool; mở rộng bằng nhiều loop trên nhiều core. → [sockets-and-protocols](../14-networking/sockets-and-protocols.md), [io-multiplexing](../04-linux-system-programming/io-multiplexing.md)
</details>

---

## 🔴 Senior

<details><summary>10) Điều gì xảy ra khi bạn gõ một URL vào trình duyệt?</summary>

(1) DNS phân giải tên miền → IP (chủ yếu UDP). (2) TCP three-way handshake thiết lập kết nối tới IP:443. (3) TLS handshake: thỏa thuận mã hóa, xác thực certificate, tạo khóa phiên (với HTTPS). (4) Gửi HTTP request (GET / + headers). (5) Server xử lý, trả HTTP response. (6) Trình duyệt render, tải thêm tài nguyên (CSS/JS/ảnh — mỗi cái lặp quy trình), hiển thị. (7) Giữ kết nối (keep-alive) hoặc đóng. Câu này xâu chuỗi toàn bộ stack DNS→TCP→TLS→HTTP→ứng dụng. → [sockets-and-protocols](../14-networking/sockets-and-protocols.md)
</details>

<details><summary>11) TLS cung cấp gì? HTTPS hoạt động ở mức cao thế nào?</summary>

TLS cung cấp ba đảm bảo: bảo mật (mã hóa), toàn vẹn (phát hiện sửa đổi), xác thực (xác minh danh tính qua certificate). HTTPS = HTTP trên TLS. Sau khi thiết lập TCP, TLS handshake: server gửi certificate (CA ký) để client xác thực, hai bên thỏa thuận thuật toán và trao đổi tạo **khóa phiên đối xứng**, rồi toàn bộ dữ liệu HTTP được mã hóa bằng khóa đó. Embedded thường dùng mbedTLS. → [sockets-and-protocols](../14-networking/sockets-and-protocols.md)
</details>

<details><summary>12) Giao thức nào phù hợp IoT/embedded và vì sao?</summary>

MQTT phổ biến nhất: pub/sub nhẹ trên TCP, thiết bị publish lên broker, bên quan tâm subscribe theo topic — tiết kiệm băng thông, hỗ trợ mạng chập chờn (QoS, last-will), tách rời gửi/nhận, hợp thiết bị ít tài nguyên. CoAP: giống HTTP nhưng trên UDP, cực nhẹ cho thiết bị hạn chế hơn. Lý do chung: HTTP đầy đủ + TCP stack đầy đủ quá nặng về băng thông/bộ nhớ/điện cho nhiều thiết bị nhúng; giao thức IoT tối giản để vừa tài nguyên và chịu mạng không ổn định. Embedded cũng dùng stack nhẹ (lwIP + mbedTLS). → [sockets-and-protocols](../14-networking/sockets-and-protocols.md)
</details>

---
⬅️ [Về index câu hỏi](README.md)
