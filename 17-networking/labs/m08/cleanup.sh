#!/usr/bin/env bash
# Dọn dẹp. Chạy: sudo bash cleanup.sh
ip netns del red 2>/dev/null; ip netns del blue 2>/dev/null
ip link del br0 2>/dev/null
iptables -t nat -D POSTROUTING -s 10.50.0.0/24 ! -o br0 -j MASQUERADE 2>/dev/null
iptables -D FORWARD -i br0 -j ACCEPT 2>/dev/null
iptables -D FORWARD -o br0 -j ACCEPT 2>/dev/null
true
