-- Bài tập F04-2: Tối ưu truy vấn bằng index. Chạy: python3 check_index.py tuning.sql
-- Bảng event có 400.000 dòng (sự kiện báo hiệu: thuê bao, cell, thời điểm, mã nguyên nhân).
-- Bốn truy vấn A–D trong check_index.py đang chạy bằng SCAN cả bảng. Yêu cầu:
--   1. Viết tối đa 3 câu CREATE INDEX trong phần @ddl (mỗi index làm chậm mọi lần ghi, nên dùng ít mà đủ).
--   2. Truy vấn C bọc cột trong hàm nên không index nào giúp được: viết lại nó trong phần @qC, cùng kết quả.
-- Đạt khi: không truy vấn nào còn "SCAN event" (quét cả bảng, kể cả quét cả index), kết quả không đổi,
-- và mỗi truy vấn nhanh hơn ít nhất 10 lần so với ban đầu.

-- @ddl
CREATE INDEX idx_event_imsi_ts  ON event(imsi, ts);        -- A (bằng + sắp xếp) và C (khoảng trên imsi)
CREATE INDEX idx_event_cell_ts  ON event(cell_id, ts);     -- B: cột bằng trước, cột khoảng sau, covering
CREATE INDEX idx_event_ts_cause ON event(ts, cause);       -- D: khoảng trên ts, cause có sẵn trong index (covering)

-- @qC
-- Ban đầu: SELECT COUNT(*) FROM event WHERE substr(imsi, 1, 12) = '452040000012';
SELECT COUNT(*) FROM event WHERE imsi >= '452040000012' AND imsi < '452040000013';
