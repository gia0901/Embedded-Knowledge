#!/bin/sh
# Chạy dưới socat UDP4-LISTEN: stdin/stdout là socket UDP đã nối tới thiết bị.
# Đọc gói đầu tiên của thiết bị, rồi "đẩy lệnh" xuống sau 1 s và sau 9 s.
read line
sleep 1; echo "lenh 1 (sau 1 s)"
sleep 8; echo "lenh 2 (sau 9 s)"
