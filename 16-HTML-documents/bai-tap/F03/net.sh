#!/bin/bash
# Topology dùng chung cho F03: dev (10.0.1.2) <-> gw (10.0.1.1 | 10.0.2.1) <-> srv (10.0.2.2)
# Chạy bằng root: sudo bash net.sh up | lan | emu | down | show
set -e
up() {
  for n in dev gw srv; do ip netns add $n; ip -n $n link set lo up; done
  ip link add veth-dev type veth peer name veth-gw1
  ip link set veth-dev netns dev; ip link set veth-gw1 netns gw
  ip link add veth-gw2 type veth peer name veth-srv
  ip link set veth-gw2 netns gw; ip link set veth-srv netns srv
  ip -n dev addr add 10.0.1.2/24 brd + dev veth-dev
  ip -n gw  addr add 10.0.1.1/24 brd + dev veth-gw1
  ip -n gw  addr add 10.0.2.1/24 brd + dev veth-gw2
  ip -n srv addr add 10.0.2.2/24 brd + dev veth-srv
  for p in "dev veth-dev" "gw veth-gw1" "gw veth-gw2" "srv veth-srv"; do set -- $p; ip -n $1 link set $2 up; done
  ip -n dev route add default via 10.0.1.1
  ip -n srv route add default via 10.0.2.1
  ip netns exec gw sysctl -qw net.ipv4.ip_forward=1
  echo "topology up: dev 10.0.1.2 <-> gw 10.0.1.1|10.0.2.1 <-> srv 10.0.2.2"
}
lan() {   # thêm dev2 (10.0.1.3) vào cùng LAN với dev qua bridge br0 trong gw
  ip netns add dev2; ip -n dev2 link set lo up
  ip link add veth-dev2 type veth peer name veth-gw3
  ip link set veth-dev2 netns dev2; ip link set veth-gw3 netns gw
  ip -n gw link add br0 type bridge
  ip -n gw addr del 10.0.1.1/24 dev veth-gw1
  ip -n gw link set veth-gw1 master br0; ip -n gw link set veth-gw3 master br0
  ip -n gw addr add 10.0.1.1/24 dev br0
  ip -n gw link set br0 up; ip -n gw link set veth-gw3 up
  ip -n dev2 addr add 10.0.1.3/24 dev veth-dev2; ip -n dev2 link set veth-dev2 up
  ip -n dev2 route add default via 10.0.1.1
  echo "lan: dev 10.0.1.2 + dev2 10.0.1.3 -- br0 (gw 10.0.1.1)"
}
emu() {   # thay dây veth gw<->srv bằng hai TUN emu0, nối qua chương trình linkemu
  ip -n gw link del veth-gw2 2>/dev/null || true
  ip netns exec gw ip tuntap add dev emu0 mode tun
  ip netns exec srv ip tuntap add dev emu0 mode tun
  ip -n gw addr add 10.0.2.1/24 brd + dev emu0; ip -n srv addr add 10.0.2.2/24 brd + dev emu0
  ip -n gw link set emu0 up; ip -n srv link set emu0 up
  ip -n srv route replace default via 10.0.2.1
  echo "gw <-> srv qua emu0; chạy ./linkemu để nối hai đầu"
}
down() { for n in dev dev2 gw srv; do ip netns del $n 2>/dev/null || true; done; echo "topology down"; }
show() { for n in dev dev2 gw srv; do ip netns list | grep -q "^$n " || continue; echo "== $n"; ip -n $n -br addr; ip -n $n route; done; }
case "$1" in up) up;; lan) lan;; emu) emu;; down) down;; show) show;; *) echo "dùng: $0 up|lan|emu|down|show"; exit 1;; esac
