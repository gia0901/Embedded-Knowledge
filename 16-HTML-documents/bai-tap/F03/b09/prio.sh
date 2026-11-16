#!/bin/bash
# tc htb trên cổng ra veth-gw2 của gw: 10 Mbit/s, hàng đợi lớn. So ping khi tải nặng,
# không phân lớp và có phân lớp (ICMP vào lớp ưu tiên). Topology chung. sudo bash prio.sh
D="ip netns exec gw tc"
setup() {
  $D qdisc del dev veth-gw2 root 2>/dev/null
  $D qdisc add dev veth-gw2 root handle 1: htb default 20
  $D class add dev veth-gw2 parent 1: classid 1:1 htb rate 10mbit
  $D class add dev veth-gw2 parent 1:1 classid 1:10 htb rate 1mbit ceil 10mbit prio 0
  $D class add dev veth-gw2 parent 1:1 classid 1:20 htb rate 9mbit ceil 10mbit prio 1
  $D qdisc add dev veth-gw2 parent 1:10 pfifo limit 100
  $D qdisc add dev veth-gw2 parent 1:20 pfifo limit 700         # 700 gói, có GSO thì mỗi gói nhiều segment: hàng đợi rất lớn
}
run() {
  ip netns exec srv iperf3 -s -1 -D; sleep 0.3
  ip netns exec dev iperf3 -c 10.0.2.2 -t 10 -C cubic > ip.txt &
  sleep 2
  p=$(ip netns exec dev ping -c 30 -i 0.2 10.0.2.2 | tail -1 | cut -d= -f2)
  wait
  echo "$1: ping min/avg/max/mdev =$p | $(grep receiver ip.txt | awk '{print $7, $8}')"
}
setup; run "khong phan lop"
$D filter add dev veth-gw2 parent 1: protocol ip prio 1 u32 match ip protocol 1 0xff flowid 1:10
run "ICMP vao 1:10 "
$D -s class show dev veth-gw2 | grep -A1 'class htb 1:[12]0' | grep -E 'class|Sent'
$D qdisc del dev veth-gw2 root
