#!/bin/bash
# Ba host h1 h2 h3 (10.0.9.1-3/24) nối vào namespace sw qua veth; sw không có bridge: miniswitch làm việc đó.
# sudo bash sw.sh up | down
set -e
if [ "$1" = up ]; then
  ip netns add sw; ip -n sw link set lo up
  for i in 1 2 3; do
    ip netns add h$i; ip -n h$i link set lo up
    ip link add eth0 netns h$i type veth peer name p$i netns sw
    ip -n h$i addr add 10.0.9.$i/24 dev eth0; ip -n h$i link set eth0 up; ip -n sw link set p$i up
  done
  echo "sw up: h1 h2 h3 = 10.0.9.1-3, cong p1 p2 p3 trong netns sw"
else
  for n in sw h1 h2 h3; do ip netns del $n 2>/dev/null || true; done; echo "sw down"
fi
