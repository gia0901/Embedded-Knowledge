#!/bin/sh
# Giả lập thiết bị sau NAT: gửi "hello", rồi nếu có đối số N thì gửi keepalive mỗi N giây.
# Tổng thời gian 11 s. Dùng làm stdin cho socat.
echo "hello tu dev"
t=0
while [ $t -lt 11 ]; do
  if [ -n "$1" ]; then sleep "$1"; t=$((t + $1)); echo "keepalive"; else sleep 11; t=11; fi
done
