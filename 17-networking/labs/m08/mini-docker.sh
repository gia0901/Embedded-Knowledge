#!/usr/bin/env bash
# Dựng 2 "container" bằng network namespace + veth + bridge + NAT. Chạy: sudo bash mini-docker.sh
set -euo pipefail

ip link add br0 type bridge
ip addr add 10.50.0.1/24 dev br0
ip link set br0 up

i=2
for ns in red blue; do
  ip netns add "$ns"
  ip link add "veth-$ns" type veth peer name "br-$ns"     # một sợi "cáp" hai đầu
  ip link set "veth-$ns" netns "$ns"                       # đầu 1 vào namespace
  ip link set "br-$ns" master br0 up                       # đầu 2 cắm vào "switch"
  ip -n "$ns" addr add "10.50.0.$i/24" dev "veth-$ns"
  ip -n "$ns" link set "veth-$ns" up
  ip -n "$ns" link set lo up
  ip -n "$ns" route add default via 10.50.0.1              # (1) default route
  i=$((i + 1))
done

sysctl -qw net.ipv4.ip_forward=1                           # (2) host làm router
iptables -t nat -A POSTROUTING -s 10.50.0.0/24 ! -o br0 -j MASQUERADE   # (3) SNAT ra ngoài
iptables -I FORWARD -i br0 -j ACCEPT                       # (4) mở FORWARD (Docker để DROP)
iptables -I FORWARD -o br0 -j ACCEPT

echo "== red → blue";   ip netns exec red ping -c 2 -W 1 10.50.0.3
echo "== red → 1.1.1.1"; ip netns exec red ping -c 2 -W 2 1.1.1.1
echo "== MAC table của br0"; bridge fdb show br br0 | grep -v permanent
