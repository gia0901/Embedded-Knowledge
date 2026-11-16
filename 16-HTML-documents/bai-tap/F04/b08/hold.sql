-- Một phiên đọc dài: giữ một snapshot 6 giây (Lab 8.2). Chạy nền: pq -f hold.sql > hold.out &
BEGIN ISOLATION LEVEL REPEATABLE READ;
SELECT 'dau', last_seen FROM devices WHERE id = 1;
SELECT pg_sleep(6);
SELECT 'cuoi', last_seen FROM devices WHERE id = 1;
COMMIT;
