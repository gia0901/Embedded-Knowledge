#!/bin/bash
# Lab 10.2: 1000 frame qua cặp pty (ttyA -> ttyB) với ba kiểu đóng khung, có và không có frame bị cắt.
# Chạy: bash framing.sh   (cần socat; uart_tx, uart_rx đã build)
socat pty,link=ttyA,raw,echo=0 pty,link=ttyB,raw,echo=0 2>/dev/null &
sleep 0.5
for m in "--len" "--len --cut 100" "--cobs" "--cobs --cut 100" "--cobs --cut 100 --lead0"; do
  echo "== $m"
  ./uart_rx ttyB ${m%% *} > rx.txt &      # ${m%% *}: chỉ lấy kiểu khung cho bên nhận
  sleep 0.2
  ./uart_tx ttyA 1000 $m
  wait $!; cat rx.txt
done
kill %1
