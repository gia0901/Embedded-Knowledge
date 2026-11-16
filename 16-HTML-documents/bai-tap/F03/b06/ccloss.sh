#!/bin/bash
# So CUBIC và BBR trên đường 10 Mbit/s, RTT 20 ms, mất gói ngẫu nhiên 0, 1, 5%.
# Cần topology emu (sudo bash ../net.sh up; sudo bash ../net.sh emu). Chạy: sudo bash ccloss.sh
EMU=../linkemu
for loss in 0 1 5; do
  $EMU --delay 10 --rate 10000 --queue 100 --loss $loss > emu.log 2>&1 &
  sleep 0.3
  for cc in cubic bbr; do
    ip netns exec srv iperf3 -s -1 -D; sleep 0.3
    r=$(ip netns exec dev iperf3 -c 10.0.2.2 -t 6 -C $cc | grep sender | awk '{print $7, $8, "retr", $9}')
    printf "mat %s%%  %-5s  %s\n" $loss $cc "$r"
    sleep 0.5
  done
  kill -INT %1; wait %1 2>/dev/null
done
