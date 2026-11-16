#!/bin/bash
# Giả lập "đứt cáp" giữa gw và srv: gw bỏ lặng lẽ mọi gói chuyển tiếp (không RST, không ICMP).
# sudo bash cut.sh on | off
case "$1" in
  on)  ip netns exec gw nft add table ip cut
       ip netns exec gw nft add chain ip cut fw '{ type filter hook forward priority 0; policy drop; }'
       echo "cut: gw bo moi goi chuyen tiep" ;;
  off) ip netns exec gw nft delete table ip cut 2>/dev/null; echo "cut: noi lai" ;;
  *)   echo "dung: $0 on|off"; exit 1 ;;
esac
