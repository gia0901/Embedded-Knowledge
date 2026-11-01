-- Bài tập F04-1: 15 câu SQL trên database mẫu (schema.sql). Viết câu trả lời dưới mỗi đề, rồi chạy:
--     python3 check.py answers.sql
-- Mỗi câu là MỘT câu lệnh kết thúc bằng dấu chấm phẩy. Tên cột không quan trọng, thứ tự cột và giá trị thì có.
-- Câu ghi "Thứ tự:" thì thứ tự dòng cũng được chấm; câu không ghi thì không.

-- @q01
-- Số thuê bao theo từng gói cước. Cột: plan, số thuê bao. Thứ tự: số thuê bao giảm dần.


-- @q02
-- IMSI của các thuê bao gold đăng ký trong tháng 3/2026. Cột: imsi. Thứ tự: imsi tăng dần.


-- @q03
-- Có bao nhiêu thuê bao chưa từng có phiên nào? Cột: số lượng.


-- @q04
-- Với mỗi DNN: số phiên và số thuê bao KHÁC NHAU. Cột: dnn, số phiên, số thuê bao. Thứ tự: dnn tăng dần.


-- @q05
-- Có bao nhiêu phiên chưa được cấp IP, và bao nhiêu phiên có IP KHÁC '10.45.0.1'? Cột: chưa có IP, có IP khác.
-- (Bẫy NULL: phiên chưa có IP không tính là "có IP khác".)


-- @q06
-- 5 thuê bao dùng nhiều lưu lượng nhất (ul + dl, cả 7 ngày). Cột: imsi, tổng byte. Thứ tự: tổng giảm dần, imsi tăng dần.


-- @q07
-- Tổng lưu lượng (ul + dl) theo vendor của gNB. Cột: vendor, tổng byte. Thứ tự: vendor tăng dần.


-- @q08
-- Có bao nhiêu thuê bao từng có phiên trên ít nhất 2 DNN khác nhau? Cột: số lượng.


-- @q09
-- Với mỗi thuê bao có phiên, lấy phiên bắt đầu muộn nhất (nếu trùng thời điểm, lấy id lớn hơn).
-- Đếm số thuê bao mà phiên muộn nhất đó thuộc DNN 'ims'. Cột: số lượng. (Gợi ý: ROW_NUMBER)


-- @q10
-- Lưu lượng dl theo giờ của cell 7, kèm chênh lệch so với giờ có dữ liệu liền trước (giờ đầu tiên: NULL).
-- Chỉ lấy 5 giờ đầu tiên. Cột: hour, tổng dl, chênh lệch. Thứ tự: hour tăng dần. (Gợi ý: LAG)


-- @q11
-- Các cell có tổng lưu lượng (ul + dl) lớn hơn trung bình tổng lưu lượng của các cell. Cột: cell_id. Thứ tự: cell_id.


-- @q12
-- Với MỌI gói cước (kể cả gói chưa ai dùng): số thuê bao và doanh thu tháng (số thuê bao × price).
-- Cột: plan, số thuê bao, doanh thu. Thứ tự: plan tăng dần.


-- @q13
-- Trong mỗi gói cước, thuê bao có tổng lưu lượng cao thứ 3 (bằng nhau thì imsi nhỏ hơn đứng trước).
-- Chỉ xét thuê bao có lưu lượng. Cột: plan, imsi, tổng byte. Thứ tự: plan tăng dần.


-- @q14
-- Liệt kê đủ 24 giờ của ngày 2026-10-28 và tổng lưu lượng dl của cell 3 trong từng giờ, giờ không có dữ liệu ghi 0.
-- Cột: hour ('2026-10-28 HH:00'), tổng dl. Thứ tự: hour tăng dần. (Gợi ý: WITH RECURSIVE sinh 24 giờ, rồi LEFT JOIN)


-- @q15
-- Câu lệnh SỬA dữ liệu: chuyển mọi thuê bao 'silver' có tổng lưu lượng (ul + dl) trên 150.000.000 byte sang 'gold'.
-- Chỉ viết câu UPDATE; trình chấm sẽ tự kiểm tra kết quả sau khi chạy.
