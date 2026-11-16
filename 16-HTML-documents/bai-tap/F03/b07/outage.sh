#!/bin/bash
# Backend chết ở giây 3, chạy lại ở giây 6; sensor gửi 10000 frame/s trong 10 s qua gw2.
# Topology chung (net.sh up). Chạy: sudo bash outage.sh
ip netns exec srv ./backend 7500 > be1.log &
sleep 0.3
ip netns exec gw ./gw2 9000 10.0.2.2 7500 > gw.log &
sleep 0.3
ip netns exec dev ../b04/sensor 10.0.1.1 9000 100000 --rate 10000 &
sleep 3; pkill -f 'backend 7500'
sleep 3; ip netns exec srv ./backend 7500 > be2.log &
sleep 5; pkill -INT gw2; sleep 0.3; pkill -f 'backend 7500'; wait
cat gw.log; echo "--- backend lan 1:"; tail -1 be1.log; echo "--- backend lan 2:"; tail -1 be2.log
