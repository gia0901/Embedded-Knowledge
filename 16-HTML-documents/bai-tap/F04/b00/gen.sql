-- Lab 0.1: sinh 1 triệu số đo của 1000 thiết bị trong 1000 phút, rồi xuất ra CSV.
-- Chạy: sqlite3 fleet0.db < gen.sql
DROP TABLE IF EXISTS readings;
CREATE TABLE readings(dev TEXT, ts INTEGER, temp REAL);
WITH RECURSIVE n(i) AS (SELECT 0 UNION ALL SELECT i + 1 FROM n WHERE i < 999999)
INSERT INTO readings
SELECT printf('dev-%04d', i % 1000),                -- 1000 thiết bị
       1767225600 + (i / 1000) * 60,                -- mỗi phút một vòng, từ 2026-01-01 00:00 UTC
       round(20 + (i * 7919 % 1500) / 100.0, 2)     -- nhiệt độ giả 20,00 .. 34,99
FROM n;
.mode csv
.output readings.csv
SELECT * FROM readings;
.output stdout
