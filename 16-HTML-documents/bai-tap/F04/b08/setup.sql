-- Bảng cho Bài 8. Chạy: sudo -u postgres psql -d fleet -q -f setup.sql   (chạy lại nhiều lần được)
SET client_min_messages = warning;
DROP TABLE IF EXISTS fw_downloads;
CREATE TABLE fw_downloads (                     -- số lượt tải mỗi bản firmware (Lab 8.1)
    version text PRIMARY KEY,
    n       int  NOT NULL DEFAULT 0,
    ver     int  NOT NULL DEFAULT 0             -- số phiên bản cho khoá lạc quan
);
INSERT INTO fw_downloads(version) SELECT version FROM firmware;
GRANT SELECT, INSERT, UPDATE, DELETE ON fw_downloads TO fleet_app;
