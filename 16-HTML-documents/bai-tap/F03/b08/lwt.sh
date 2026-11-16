#!/bin/bash
# Will message (LWT) và keepalive: thiết bị dev-007 (keepalive 5 s) mất theo ba cách.
# Cần broker đang chạy trong srv (mosq.log ở thư mục này). Chạy: sudo bash lwt.sh cut|kill9|clean
ip netns exec srv mosquitto_sub -t 'lab/status/#' -F '%I %t %p' > status.log &
MON=$!
ip netns exec dev mosquitto_sub -h 10.0.2.2 -i dev-007 -k 5 -t lab/cmd \
   --will-topic lab/status/dev-007 --will-payload offline > /dev/null &
DEV=$!
sleep 2
n=$(wc -l < mosq.log)
echo "$(date +%T) thiet bi mat: $1"
case "$1" in
  cut)   bash ../b05/cut.sh on > /dev/null ;;      # đứt đường, không ai gửi FIN hay RST
  kill9) kill -9 $DEV ;;                            # process chết, kernel đóng socket
  clean) kill -INT $DEV ;;                          # mosquitto_sub gửi DISCONNECT rồi thoát
esac
sleep 15
echo "--- status:"; cat status.log
echo "--- broker:"; tail -n +$((n + 1)) mosq.log | grep -E 'dev-007|lab/status' | head -4
bash ../b05/cut.sh off > /dev/null; kill $DEV $MON 2>/dev/null; wait 2>/dev/null
