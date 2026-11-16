-- Lab 4.1: 100 000 số đo của 100 thiết bị, mỗi phút một vòng, và một index (dev, ts).
-- Chạy: sqlite3 r.db < gen.sql
CREATE TABLE r (dev INTEGER, ts INTEGER, temp REAL);
WITH RECURSIVE n(i) AS (SELECT 0 UNION ALL SELECT i + 1 FROM n WHERE i < 99999)
INSERT INTO r SELECT i % 100, 1794704400 + (i / 100) * 60, round(20 + (i * 7919 % 1500) / 100.0, 2) FROM n;
CREATE INDEX r_dev ON r (dev, ts);
