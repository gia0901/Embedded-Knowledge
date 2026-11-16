-- Lab 7.3: mỗi transaction ghi một số đo, như một collector commit từng bản tin.
-- Chạy: pgbench -n -f ingest.sql -c CLIENTS -T 10 fleet   (bảng ingest tạo trong Lab 7.3)
\set d random(1, 200)
INSERT INTO ingest (device_id, temp) VALUES (:d, 20 + random() * 10);
