-- Hai role cho ứng dụng. Role là đối tượng của PostgreSQL, không phải user Linux.
-- Chạy sau fleet.sql: sudo -u postgres psql -d fleet -f roles.sql   (chạy lại nhiều lần được)
DO $$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'fleet_app') THEN
        CREATE ROLE fleet_app LOGIN PASSWORD 'lab';      -- dịch vụ backend: đọc, ghi
    END IF;
    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = 'fleet_ro') THEN
        CREATE ROLE fleet_ro LOGIN PASSWORD 'lab';       -- công cụ tra cứu: chỉ đọc
    END IF;
END $$;
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO fleet_app;
GRANT SELECT ON devices TO fleet_ro;
