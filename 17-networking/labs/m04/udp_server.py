#!/usr/bin/env python3
"""UDP server: trả lại câu nhận được dưới dạng chữ hoa. Tham số: tỉ lệ cố ý drop (0..1)."""
import random
import socket
import sys

DROP = float(sys.argv[1]) if len(sys.argv) > 1 else 0.0
with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    s.bind(("0.0.0.0", 9000))
    print(f"UDP server on :9000, drop={DROP:.0%}")
    while True:
        data, addr = s.recvfrom(2048)        # mỗi lần nhận đúng một datagram + address người gửi
        if random.random() < DROP:
            print(f"  drop {data!r} from {addr}")
            continue
        s.sendto(data.upper(), addr)         # UDP: phải chỉ rõ gửi cho ai
