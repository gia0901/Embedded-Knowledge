#!/bin/bash
# Lab 11.3, 11.4: đường hầm GTP-U trong user space. gw đóng vai gNB (UE là dev, dev2), srv đóng vai UPF;
# "mạng dữ liệu" (DN) là địa chỉ 10.99.0.1 trên srv. MTU của gtp0 = 1500 - 36 (IP 20 + UDP 8 + GTP-U 8).
# Chạy: sudo bash gtp.sh up | down
case "$1" in
up)
  ip netns exec gw  ./gtpu gtp0 10.0.2.1 gnb.sess 2> gnb.log &
  ip netns exec srv ./gtpu gtp0 10.0.2.2 upf.sess 2> upf.log &
  sleep 0.5
  ip -n gw  link set gtp0 mtu 1464 up; ip -n gw  route add 10.99.0.0/24 dev gtp0
  ip -n srv link set gtp0 mtu 1464 up; ip -n srv addr add 10.99.0.1/32 dev lo
  for ue in $(awk '!/^#/ && NF {print $1}' upf.sess); do ip -n srv route add $ue/32 dev gtp0; done
  echo "gtp0: gNB (gw 10.0.2.1) <-> UPF (srv 10.0.2.2) qua UDP 2152; DN 10.99.0.1";;
down)
  pkill -TERM -x gtpu; sleep 0.3
  ip -n srv addr del 10.99.0.1/32 dev lo 2>/dev/null; echo "gtpu da tat";;
*) echo "dung: $0 up|down"; exit 1;;
esac
