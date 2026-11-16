#!/bin/bash
# migrate.sh: nâng lược đồ gw.db trên gateway. Số phiên bản lưu trong PRAGMA user_version của chính file.
# Mỗi migration chạy trong MỘT transaction cùng với lệnh tăng user_version: lỗi giữa chừng (hay mất điện)
# thì cả bước bị huỷ, file giữ nguyên phiên bản cũ.
# Chạy: bash migrate.sh gw.db            (migration nằm trong migrations/NNN_ten.sql)
DB=${1:?file db}
v=$(sqlite3 "$DB" "PRAGMA user_version")
echo "$DB: phien ban $v"
for f in migrations/[0-9][0-9][0-9]_*.sql; do
    n=$((10#$(basename "$f" | cut -c1-3)))
    [ "$n" -le "$v" ] && continue
    if { echo "BEGIN;"; cat "$f"; echo "PRAGMA user_version = $n;"; echo "COMMIT;"; } | sqlite3 -bail "$DB"; then
        echo "  $f: xong, phien ban $n"; v=$n
    else
        echo "  $f: LOI, dung o phien ban $(sqlite3 "$DB" "PRAGMA user_version")"; exit 1
    fi
done
