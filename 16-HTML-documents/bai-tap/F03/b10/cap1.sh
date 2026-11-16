#!/bin/bash
# Lab 10.4: bắt gói của một lần GET kiểu CON ở phía srv, đo thời gian. Chạy vài lần tới khi thấy gửi lại.
# Cần như coaploss.sh. sudo bash cap1.sh
cd ~/oslab/f03/b10
ip netns exec srv tcpdump -i emu0 -nn -l udp port 5683 > cap.txt 2>/dev/null &
sleep 1
TIMEFORMAT='%R s'
time ip netns exec dev coap-client-notls -B 60 coap://10.0.2.2/time
sleep 0.5; pkill tcpdump; sleep 0.3
cat cap.txt
