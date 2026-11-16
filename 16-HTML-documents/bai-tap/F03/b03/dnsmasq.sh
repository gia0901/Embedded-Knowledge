#!/bin/bash
# Chạy dnsmasq trong netns gw: DHCP cho LAN 10.0.1.0/24 và DNS cho tên *.lab
# (--local=/lab/: dnsmasq tự trả lời mọi tên .lab, tên không có thì NXDOMAIN).
# sudo bash dnsmasq.sh   (Ctrl-C để dừng; log ở terminal)
exec ip netns exec gw dnsmasq --no-daemon --conf-file=/dev/null --no-resolv \
  --interface=veth-gw1 --bind-interfaces \
  --dhcp-range=10.0.1.100,10.0.1.150,12h \
  --dhcp-option=option:router,10.0.1.1 --dhcp-option=option:dns-server,10.0.1.1 \
  --dhcp-leasefile=/tmp/f03-gw.leases \
  --local=/lab/ --host-record=backend.lab,10.0.2.2 --host-record=gw.lab,10.0.1.1 \
  --local-ttl=60 --log-queries --log-dhcp
