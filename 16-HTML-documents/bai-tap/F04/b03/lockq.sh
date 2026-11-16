#!/bin/bash
# lockq.sh: tái hiện hàng đợi khoá khi migration. Ba phiên chạy song song trên bảng readings của fleet:
#   A  một transaction đọc dài (giữ khoá ACCESS SHARE 6 s), như một báo cáo đang chạy
#   B  migration ALTER TABLE (cần khoá ACCESS EXCLUSIVE), có thể kèm lock_timeout
#   C  truy vấn bình thường của ứng dụng, đến sau B
# Chạy: bash lockq.sh "ALTER TABLE ..." [lock_timeout, vd 1s]
SQL=${1:?cau ALTER}; LT=${2:-0}
PSQL="sudo -u postgres psql -d fleet -X -q -At"
T0=$(date +%s.%N)
t() { printf '%5.2f s  %s\n' "$(echo "$(date +%s.%N) - $T0" | bc)" "$*"; }

$PSQL -c "BEGIN; SELECT count(*) FROM readings; SELECT pg_sleep(6); COMMIT;" > /dev/null &
A=$!; t "A: bat dau doc (6 s)"; sleep 1

( t "B: $SQL (lock_timeout=$LT)"
  if out=$($PSQL -c "SET lock_timeout = '$LT'" -c "$SQL" 2>&1); then t "B: xong"; else t "B: ${out#ERROR:  }"; fi ) &
B=$!; sleep 1

( t "C: SELECT cua ung dung"; $PSQL -c "SELECT count(*) FROM readings WHERE device_id = 1" > /dev/null; t "C: xong" ) &
C=$!; sleep 0.5

$PSQL -c "SELECT 'cho khoa: ' || count(*) FROM pg_stat_activity WHERE wait_event_type = 'Lock'" | while read -r l; do t "$l"; done
wait $A; t "A: xong"; wait $B $C
