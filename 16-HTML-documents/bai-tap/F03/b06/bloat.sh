#!/bin/bash
# Bufferbloat: tbf 10 Mbit/s trên cổng ra veth-gw2 của gw, hàng đợi lớn (1 MB) hoặc nhỏ (30 KB).
# Đo RTT của ping trong lúc iperf3 chạy, với CUBIC và BBR. Topology chung (net.sh up). sudo bash bloat.sh
echo -n "khong tai: "; ip netns exec dev ping -c 10 -i 0.2 10.0.2.2 | tail -1
for lim in 1mb 30kb; do
  ip netns exec gw tc qdisc replace dev veth-gw2 root tbf rate 10mbit burst 16kb limit $lim
  for cc in cubic bbr; do
    ip netns exec srv iperf3 -s -1 -D; sleep 0.3
    ip netns exec dev iperf3 -c 10.0.2.2 -t 10 -C $cc > ip.txt &
    sleep 2
    p=$(ip netns exec dev ping -c 30 -i 0.2 10.0.2.2 | tail -1 | cut -d= -f2)
    wait
    t=$(grep sender ip.txt | awk '{print $7, $8, "retr", $9}')
    printf "limit %-5s %-5s  ping min/avg/max/mdev =%s | %s\n" $lim $cc "$p" "$t"
    sleep 0.5
  done
done
ip netns exec gw tc qdisc del dev veth-gw2 root
