-- Dữ liệu một ngày của 200 thiết bị cho Bài 2. Chạy sau ../b01/fleet.sql:
--   sudo -u postgres psql -d fleet -q -f ../b01/fleet.sql -f gen.sql
-- Thiết bị 191..200 chưa bao giờ báo về. Thiết bị có id chia hết cho 10 mất kết nối một lần trong ngày.
INSERT INTO firmware VALUES
    ('2.0.0', '2026-03-01', repeat('a', 64)),
    ('2.1.0', '2026-06-15', repeat('b', 64)),
    ('2.2.0', '2026-10-20', repeat('c', 64));

INSERT INTO devices (serial, site, fw)
SELECT 'SN-' || (1000 + i),
       (ARRAY['hcm', 'hni', 'dng', 'hpg'])[1 + i % 4],
       CASE WHEN i % 17 = 0 THEN NULL ELSE (ARRAY['2.0.0', '2.1.0', '2.2.0'])[1 + i % 3] END
FROM generate_series(1, 200) AS i;

-- Mỗi phút một số đo, từ 00:00 tới 23:59 ngày 14/11/2026 giờ Việt Nam.
INSERT INTO readings (device_id, ts, temp)
SELECT d, timestamptz '2026-11-14 00:00+07' + m * interval '1 minute',
       round((25 + 5 * sin(m / 1440.0 * 2 * pi()) + (d % 10) * 0.3 + (d::bigint * m * 7919 % 100) / 100.0)::numeric, 2)
FROM generate_series(1, 190) AS d, generate_series(0, 1439) AS m
WHERE NOT (d % 10 = 0 AND m >= (d * 7) % 1200 AND m < (d * 7) % 1200 + 10 + (d % 7) * 5);

UPDATE devices SET last_seen = r.ts
FROM (SELECT device_id, max(ts) AS ts FROM readings GROUP BY device_id) AS r
WHERE r.device_id = devices.id;

INSERT INTO commands (device_id, kind, payload, created, sent_at)
SELECT i, CASE WHEN i % 3 = 0 THEN 'reboot' ELSE 'update_fw' END,
       CASE WHEN i % 3 = 0 THEN NULL ELSE '{"to": "2.2.0"}'::jsonb END,
       timestamptz '2026-11-14 08:00+07' + i * interval '1 minute',
       CASE WHEN i % 4 = 0 THEN NULL ELSE timestamptz '2026-11-14 08:00+07' + i * interval '1 minute' + interval '5 seconds' END
FROM generate_series(1, 200, 7) AS i;

ANALYZE;
