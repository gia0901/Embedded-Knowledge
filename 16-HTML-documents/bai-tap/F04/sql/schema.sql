-- Database mẫu cho bài tập SQL F04-1: thuê bao, phiên, cell, lưu lượng theo giờ trong 7 ngày.
-- Dữ liệu sinh tất định bằng recursive CTE, ai chạy cũng ra cùng kết quả.
PRAGMA foreign_keys = ON;

CREATE TABLE plan (
    name       TEXT PRIMARY KEY,
    speed_mbps INTEGER NOT NULL,
    price      INTEGER NOT NULL           -- nghìn đồng mỗi tháng
);
INSERT INTO plan VALUES ('iot', 1, 20), ('silver', 100, 150), ('gold', 1000, 300), ('platinum', 2000, 500);

CREATE TABLE gnb (
    id     INTEGER PRIMARY KEY,
    vendor TEXT NOT NULL,
    city   TEXT NOT NULL
);
CREATE TABLE cell (
    id     INTEGER PRIMARY KEY,
    gnb_id INTEGER NOT NULL REFERENCES gnb(id),
    band   TEXT NOT NULL
);
CREATE TABLE subscriber (
    id      INTEGER PRIMARY KEY,
    imsi    TEXT NOT NULL UNIQUE,
    plan    TEXT NOT NULL REFERENCES plan(name),
    created TEXT NOT NULL                -- 'YYYY-MM-DD'
);
CREATE TABLE pdu_session (
    id            INTEGER PRIMARY KEY,
    subscriber_id INTEGER NOT NULL REFERENCES subscriber(id),
    dnn           TEXT NOT NULL,
    cell_id       INTEGER NOT NULL REFERENCES cell(id),
    ue_ip         TEXT,                  -- NULL: chưa cấp IP
    started       TEXT NOT NULL          -- 'YYYY-MM-DD HH:MM'
);
CREATE TABLE usage (                     -- lưu lượng của một phiên trong một giờ
    session_id INTEGER NOT NULL REFERENCES pdu_session(id),
    hour       TEXT NOT NULL,            -- 'YYYY-MM-DD HH:00'
    ul_bytes   INTEGER NOT NULL,
    dl_bytes   INTEGER NOT NULL,
    PRIMARY KEY (session_id, hour)
);

WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i + 1 FROM n WHERE i < 10)
INSERT INTO gnb SELECT i, CASE i % 3 WHEN 0 THEN 'VendorA' WHEN 1 THEN 'VendorB' ELSE 'VendorC' END,
                       CASE WHEN i <= 6 THEN 'Ha Noi' ELSE 'Da Nang' END FROM n;

WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i + 1 FROM n WHERE i < 50)
INSERT INTO cell SELECT i, 1 + (i - 1) / 5, CASE i % 2 WHEN 0 THEN 'n78' ELSE 'n41' END FROM n;

WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i + 1 FROM n WHERE i < 2000)
INSERT INTO subscriber
SELECT i,
       '45204' || printf('%010d', (i * 7919) % 9999991),
       CASE WHEN (i * 37) % 100 < 15 THEN 'gold' WHEN (i * 37) % 100 < 70 THEN 'silver' ELSE 'iot' END,
       date('2026-01-01', '+' || ((i * 13) % 300) || ' days')
FROM n;

WITH RECURSIVE n(i) AS (SELECT 1 UNION ALL SELECT i + 1 FROM n WHERE i < 5000)
INSERT INTO pdu_session
SELECT i,
       1 + (i * 7) % 1700,                                   -- thuê bao 1701..2000 không có phiên
       CASE WHEN (i + i / 1700) % 10 < 7 THEN 'internet' WHEN (i + i / 1700) % 10 < 9 THEN 'ims' ELSE 'iot.corp' END,
       1 + (i * 11) % 50,
       CASE WHEN i % 97 = 0 THEN NULL ELSE '10.45.' || (i / 250) || '.' || (i % 250 + 1) END,
       strftime('%Y-%m-%d %H:%M', '2026-10-25 00:00', '+' || ((i * 17) % 10080) || ' minutes')
FROM n;

-- Mỗi phiên có lưu lượng trong 1..6 giờ đầu tiên kể từ lúc bắt đầu
WITH RECURSIVE h(k) AS (SELECT 0 UNION ALL SELECT k + 1 FROM h WHERE k < 5)
INSERT INTO usage
SELECT s.id,
       strftime('%Y-%m-%d %H:00', s.started, '+' || h.k || ' hours'),
       ((s.id * 31 + h.k * 17) % 1000) * 1000,
       ((s.id * 53 + h.k * 29) % 5000) * 4000
FROM pdu_session s JOIN h ON h.k < 1 + s.id % 6
WHERE s.ue_ip IS NOT NULL;
