#!/bin/bash
# Lab 11.1, 11.2: đường hầm VXLAN giữa gw và srv, mạng phủ 192.168.50.0/24 trên đường 10.0.2.0/24.
# Chạy: sudo bash vxlan.sh up [VNI_SRV]   (mặc định cả hai đầu VNI 42)  |  sudo bash vxlan.sh down
case "$1" in
up)
  ip -n gw  link add vx0 type vxlan id 42 local 10.0.2.1 remote 10.0.2.2 dstport 4789 dev veth-gw2
  ip -n srv link add vx0 type vxlan id ${2:-42} local 10.0.2.2 remote 10.0.2.1 dstport 4789 dev veth-srv
  ip -n gw  addr add 192.168.50.1/24 dev vx0; ip -n gw  link set vx0 up
  ip -n srv addr add 192.168.50.2/24 dev vx0; ip -n srv link set vx0 up
  echo "vx0: gw 192.168.50.1 (VNI 42) <-> srv 192.168.50.2 (VNI ${2:-42}) qua UDP 4789";;
down)
  ip -n gw link del vx0 2>/dev/null; ip -n srv link del vx0 2>/dev/null; echo "vx0 da xoa";;
*) echo "dung: $0 up [VNI_SRV] | down"; exit 1;;
esac
