#!/bin/sh
# Script cho busybox udhcpc (giống /usr/share/udhcpc/default.script trên rootfs nhúng).
# udhcpc gọi script với $1 = deconfig | bound | renew | leasefail và các biến ip, subnet, router, dns.
case "$1" in
  deconfig) ip addr flush dev "$interface"; ip link set "$interface" up ;;
  bound|renew)
    ip addr flush dev "$interface"
    ip addr add "$ip/$mask" dev "$interface"
    [ -n "$router" ] && ip route replace default via "${router%% *}" dev "$interface"
    echo "$1: $ip/$mask router $router dns $dns lease $lease s" ;;
esac
