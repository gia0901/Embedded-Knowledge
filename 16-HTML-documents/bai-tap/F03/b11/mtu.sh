#!/bin/bash
# Lab 11.2: đường dưới của đường hầm có MTU nhỏ hơn 1500 ở một chặng (veth-srv 1400, như một đường 4G
# hay PPPoE), vx0 vẫn để MTU mặc định 1450. Đo ping nhỏ, ping lớn, iperf3; rồi hạ MTU của vx0 và đo lại.
# Cần: net.sh up, vxlan.sh up. Chạy: sudo bash mtu.sh
do_test() {
  echo "-- vx0 mtu $(ip -n gw link show vx0 | grep -oE 'mtu [0-9]+' | cut -d' ' -f2)"
  echo "ping 56 byte : $(ip netns exec gw ping -c 2 -W 1 -q 192.168.50.2 | grep -oE '[0-9]+ received')"
  echo "ping 1400 byte: $(ip netns exec gw ping -c 2 -W 1 -q -s 1400 192.168.50.2 | grep -oE '[0-9]+ received')"
  ip netns exec srv iperf3 -s -D -1; sleep 0.5
  timeout 10 ip netns exec gw iperf3 -c 192.168.50.2 -t 3 -b 100M | grep -E 'receiver'
  pkill iperf3; sleep 0.3
}
ip -n srv link set veth-srv mtu 1400
do_test
ip -n gw link set vx0 mtu 1350; ip -n srv link set vx0 mtu 1350
do_test
echo "-- bo dem cua veth-srv (RX)"; ip -n srv -s link show veth-srv | sed -n '3,4p'
ip -n srv link set veth-srv mtu 1500
