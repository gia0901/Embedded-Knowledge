-- fleet: database của backend quản lý đội thiết bị. Dùng suốt F04.
-- Chạy: sudo -u postgres psql -d fleet -f fleet.sql
SET client_min_messages = warning;                     -- bớt NOTICE khi chạy lại
DROP TABLE IF EXISTS commands, readings, devices, firmware;

CREATE TABLE firmware (
    version  text PRIMARY KEY,                         -- khoá tự nhiên: '2.1.0'
    released date NOT NULL,
    sha256   text NOT NULL UNIQUE CHECK (length(sha256) = 64)
);

CREATE TABLE devices (
    id        int  GENERATED ALWAYS AS IDENTITY PRIMARY KEY,   -- khoá thay thế
    serial    text NOT NULL UNIQUE,                    -- khoá tự nhiên, vẫn UNIQUE
    site      text NOT NULL,
    fw        text REFERENCES firmware(version),       -- NULL: thiết bị chưa báo firmware
    last_seen timestamptz
);

CREATE TABLE readings (
    device_id int         NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    ts        timestamptz NOT NULL,
    temp      real        NOT NULL CHECK (temp BETWEEN -40 AND 125),
    PRIMARY KEY (device_id, ts)                        -- mỗi thiết bị một số đo mỗi thời điểm
);

CREATE TABLE commands (
    id        bigint GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    device_id int    NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    kind      text   NOT NULL CHECK (kind IN ('reboot', 'set_config', 'update_fw')),
    payload   jsonb,
    created   timestamptz NOT NULL DEFAULT now(),
    sent_at   timestamptz                              -- NULL: chưa gửi
);
