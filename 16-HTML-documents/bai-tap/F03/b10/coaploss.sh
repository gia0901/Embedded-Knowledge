#!/bin/bash
# 20 lần GET coap://10.0.2.2/time qua linkemu mất 30% mỗi chiều, kiểu CON rồi NON.
# Cần: net.sh emu, linkemu --delay 10 --loss 30, coap-server-notls -A 10.0.2.2 trong srv. sudo bash coaploss.sh
for mode in CON NON; do
  [ $mode = NON ] && opt="-N -B 5" || opt="-B 60"
  ok=0; ts=""
  for i in $(seq 20); do
    t0=$(date +%s.%N)
    r=$(ip netns exec dev coap-client-notls $opt coap://10.0.2.2/time 2>/dev/null)
    t=$(echo "$(date +%s.%N) - $t0" | bc)
    if [ -n "$r" ]; then ok=$((ok + 1)); ts="$ts $t"; fi
  done
  echo "$mode: thanh cong $ok/20, thoi gian (s):$(for x in $ts; do printf ' %.2f' $x; done)"
done
