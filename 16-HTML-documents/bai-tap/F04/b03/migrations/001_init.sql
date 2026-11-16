-- Phiên bản 1: hàng đợi số đo chờ gửi và cấu hình.
CREATE TABLE outbox (seq INTEGER PRIMARY KEY, ts INTEGER NOT NULL, payload BLOB NOT NULL) STRICT;
CREATE TABLE config (key TEXT PRIMARY KEY, value TEXT NOT NULL) STRICT;
