#!/usr/bin/env bash
# 3 zone: ext (giả lập Internet) — fw — dmz, app. Chạy: sudo bash zones.sh   |   dọn: sudo bash zones.sh down
set -euo pipefail
if [[ ${1:-} == down ]]; then
  for n in ext dmz app fw; do ip netns del "$n" 2>/dev/null || true; done
  exit 0
fi
ip netns add fw
ip netns exec fw sysctl -qw net.ipv4.ip_forward=1
ip -n fw link set lo up
i=1
for z in ext dmz app; do
  ip netns add "$z"
  ip link add "fw-$z" type veth peer name eth0 netns "$z"
  ip link set "fw-$z" netns fw
  ip -n fw addr add "10.13.$i.1/24" dev "fw-$z"
  ip -n fw link set "fw-$z" up
  ip -n "$z" addr add "10.13.$i.10/24" dev eth0
  ip -n "$z" link set eth0 up
  ip -n "$z" link set lo up
  ip -n "$z" route add default via "10.13.$i.1"
  i=$((i + 1))
done
ip -n ext addr add 10.13.1.99/24 dev eth0      # bastion: một address thứ hai trong zone ext
modprobe nf_log_syslog 2>/dev/null || true
sysctl -qw net.netfilter.nf_log_all_netns=1 || echo "cảnh báo: không bật được log từ namespace"
echo "OK: ext=10.13.1.10 (+ bastion .99), dmz=10.13.2.10, app=10.13.3.10"
