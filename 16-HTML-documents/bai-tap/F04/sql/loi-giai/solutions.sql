-- Bài tập F04-1: 15 câu SQL trên database mẫu (schema.sql). Viết câu trả lời dưới mỗi đề, rồi chạy:
--     python3 check.py answers.sql
-- Mỗi câu là MỘT câu lệnh kết thúc bằng dấu chấm phẩy. Tên cột không quan trọng, thứ tự cột và giá trị thì có.
-- Câu ghi "Thứ tự:" thì thứ tự dòng cũng được chấm; câu không ghi thì không.

-- @q01
-- Số thuê bao theo từng gói cước. Cột: plan, số thuê bao. Thứ tự: số thuê bao giảm dần.
SELECT plan, COUNT(*) AS n FROM subscriber GROUP BY plan ORDER BY n DESC;

-- @q02
-- IMSI của các thuê bao gold đăng ký trong tháng 3/2026. Cột: imsi. Thứ tự: imsi tăng dần.
SELECT imsi FROM subscriber
WHERE plan = 'gold' AND created >= '2026-03-01' AND created < '2026-04-01'
ORDER BY imsi;

-- @q03
-- Có bao nhiêu thuê bao chưa từng có phiên nào? Cột: số lượng.
SELECT COUNT(*) FROM subscriber s
LEFT JOIN pdu_session p ON p.subscriber_id = s.id
WHERE p.id IS NULL;

-- @q04
-- Với mỗi DNN: số phiên và số thuê bao KHÁC NHAU. Cột: dnn, số phiên, số thuê bao. Thứ tự: dnn tăng dần.
SELECT dnn, COUNT(*), COUNT(DISTINCT subscriber_id) FROM pdu_session GROUP BY dnn ORDER BY dnn;

-- @q05
-- Có bao nhiêu phiên chưa được cấp IP, và bao nhiêu phiên có IP KHÁC '10.45.0.1'? Cột: chưa có IP, có IP khác.
-- (Bẫy NULL: phiên chưa có IP không tính là "có IP khác".)
SELECT SUM(ue_ip IS NULL), SUM(ue_ip <> '10.45.0.1') FROM pdu_session;

-- @q06
-- 5 thuê bao dùng nhiều lưu lượng nhất (ul + dl, cả 7 ngày). Cột: imsi, tổng byte. Thứ tự: tổng giảm dần, imsi tăng dần.
SELECT s.imsi, SUM(u.ul_bytes + u.dl_bytes) AS total
FROM usage u
JOIN pdu_session p ON p.id = u.session_id
JOIN subscriber s ON s.id = p.subscriber_id
GROUP BY s.id
ORDER BY total DESC, s.imsi
LIMIT 5;

-- @q07
-- Tổng lưu lượng (ul + dl) theo vendor của gNB. Cột: vendor, tổng byte. Thứ tự: vendor tăng dần.
SELECT g.vendor, SUM(u.ul_bytes + u.dl_bytes)
FROM usage u
JOIN pdu_session p ON p.id = u.session_id
JOIN cell c ON c.id = p.cell_id
JOIN gnb g ON g.id = c.gnb_id
GROUP BY g.vendor ORDER BY g.vendor;

-- @q08
-- Có bao nhiêu thuê bao từng có phiên trên ít nhất 2 DNN khác nhau? Cột: số lượng.
SELECT COUNT(*) FROM (
    SELECT subscriber_id FROM pdu_session GROUP BY subscriber_id HAVING COUNT(DISTINCT dnn) >= 2
);

-- @q09
-- Với mỗi thuê bao có phiên, lấy phiên bắt đầu muộn nhất (nếu trùng thời điểm, lấy id lớn hơn).
-- Đếm số thuê bao mà phiên muộn nhất đó thuộc DNN 'ims'. Cột: số lượng. (Gợi ý: ROW_NUMBER)
SELECT COUNT(*) FROM (
    SELECT dnn, ROW_NUMBER() OVER (PARTITION BY subscriber_id ORDER BY started DESC, id DESC) AS rn
    FROM pdu_session
) WHERE rn = 1 AND dnn = 'ims';

-- @q10
-- Lưu lượng dl theo giờ của cell 7, kèm chênh lệch so với giờ có dữ liệu liền trước (giờ đầu tiên: NULL).
-- Chỉ lấy 5 giờ đầu tiên. Cột: hour, tổng dl, chênh lệch. Thứ tự: hour tăng dần. (Gợi ý: LAG)
WITH h AS (
    SELECT u.hour, SUM(u.dl_bytes) AS dl
    FROM usage u JOIN pdu_session p ON p.id = u.session_id
    WHERE p.cell_id = 7
    GROUP BY u.hour
)
SELECT hour, dl, dl - LAG(dl) OVER (ORDER BY hour) FROM h ORDER BY hour LIMIT 5;

-- @q11
-- Các cell có tổng lưu lượng (ul + dl) lớn hơn trung bình tổng lưu lượng của các cell. Cột: cell_id. Thứ tự: cell_id.
WITH t AS (
    SELECT p.cell_id, SUM(u.ul_bytes + u.dl_bytes) AS total
    FROM usage u JOIN pdu_session p ON p.id = u.session_id
    GROUP BY p.cell_id
)
SELECT cell_id FROM t WHERE total > (SELECT AVG(total) FROM t) ORDER BY cell_id;

-- @q12
-- Với MỌI gói cước (kể cả gói chưa ai dùng): số thuê bao và doanh thu tháng (số thuê bao × price).
-- Cột: plan, số thuê bao, doanh thu. Thứ tự: plan tăng dần.
SELECT pl.name, COUNT(s.id), COUNT(s.id) * pl.price
FROM plan pl LEFT JOIN subscriber s ON s.plan = pl.name
GROUP BY pl.name ORDER BY pl.name;

-- @q13
-- Trong mỗi gói cước, thuê bao có tổng lưu lượng cao thứ 3 (bằng nhau thì imsi nhỏ hơn đứng trước).
-- Chỉ xét thuê bao có lưu lượng. Cột: plan, imsi, tổng byte. Thứ tự: plan tăng dần.
WITH t AS (
    SELECT s.plan, s.imsi, SUM(u.ul_bytes + u.dl_bytes) AS total
    FROM usage u
    JOIN pdu_session p ON p.id = u.session_id
    JOIN subscriber s ON s.id = p.subscriber_id
    GROUP BY s.id
), r AS (
    SELECT *, ROW_NUMBER() OVER (PARTITION BY plan ORDER BY total DESC, imsi) AS rn FROM t
)
SELECT plan, imsi, total FROM r WHERE rn = 3 ORDER BY plan;

-- @q14
-- Liệt kê đủ 24 giờ của ngày 2026-10-28 và tổng lưu lượng dl của cell 3 trong từng giờ, giờ không có dữ liệu ghi 0.
-- Cột: hour ('2026-10-28 HH:00'), tổng dl. Thứ tự: hour tăng dần. (Gợi ý: WITH RECURSIVE sinh 24 giờ, rồi LEFT JOIN)
WITH RECURSIVE hrs(k) AS (SELECT 0 UNION ALL SELECT k + 1 FROM hrs WHERE k < 23),
h AS (SELECT strftime('%Y-%m-%d %H:00', '2026-10-28 00:00', '+' || k || ' hours') AS hour FROM hrs),
d AS (
    SELECT u.hour, SUM(u.dl_bytes) AS dl
    FROM usage u JOIN pdu_session p ON p.id = u.session_id
    WHERE p.cell_id = 3 GROUP BY u.hour
)
SELECT h.hour, COALESCE(d.dl, 0) FROM h LEFT JOIN d ON d.hour = h.hour ORDER BY h.hour;

-- @q15
-- Câu lệnh SỬA dữ liệu: chuyển mọi thuê bao 'silver' có tổng lưu lượng (ul + dl) trên 150.000.000 byte sang 'gold'.
-- Chỉ viết câu UPDATE; trình chấm sẽ tự kiểm tra kết quả sau khi chạy.
UPDATE subscriber SET plan = 'gold'
WHERE plan = 'silver'
  AND id IN (
      SELECT p.subscriber_id
      FROM usage u JOIN pdu_session p ON p.id = u.session_id
      GROUP BY p.subscriber_id
      HAVING SUM(u.ul_bytes + u.dl_bytes) > 150000000
  );
