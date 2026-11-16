#!/bin/bash
# Backend chậm (3200 frame/s) trong khi sensor gửi 10000 frame/s trong 20 s.
# Bộ đệm socket hai đầu đặt nhỏ (64 KB) để hàng đợi của gw2 lộ ra. Chạy: sudo bash slow.sh [QMAX]
ip netns exec srv ./backend 7500 --slow-us 20000 --rcvbuf 65536 > be.log &
sleep 0.3
ip netns exec gw ./gw2 9000 10.0.2.2 7500 --sndbuf 65536 --qmax ${1:-1000000} > gw.log &
sleep 0.3
ip netns exec dev ../b04/sensor 10.0.1.1 9000 200000 --rate 10000
sleep 0.3; pkill -INT gw2; sleep 0.3; pkill -f 'backend 7500'; wait
cat gw.log; echo "--- backend:"; tail -1 be.log
