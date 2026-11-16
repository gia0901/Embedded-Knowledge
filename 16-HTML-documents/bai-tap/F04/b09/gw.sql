-- gw.db: kho cục bộ của gateway (Bài 9). Chạy: sqlite3 gw.db < gw.sql
CREATE TABLE IF NOT EXISTS pending (            -- số đo chờ gửi lên backend
    id        INTEGER PRIMARY KEY,
    device_id INTEGER NOT NULL,
    ts        INTEGER NOT NULL,                 -- giây Unix
    temp      REAL    NOT NULL
);
CREATE TABLE IF NOT EXISTS config (key TEXT PRIMARY KEY, value TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS counters (name TEXT PRIMARY KEY, n INTEGER NOT NULL);
INSERT OR IGNORE INTO counters VALUES ('energy_wh', 0);
