#!/usr/bin/env python3
"""Phép thử framing: gửi request từng byte một, nghỉ 20 ms giữa các byte."""
import json
import socket
import struct
import time

from kvserver import recv_msg

body = json.dumps({"op": "get", "key": "token"}).encode()
msg = struct.pack("!I", len(body)) + body
with socket.create_connection(("127.0.0.1", 9100)) as s:
    for b in msg:
        s.send(bytes([b]))       # mỗi lần send một byte → server nhận rất nhiều mảnh nhỏ
        time.sleep(0.02)
    print(recv_msg(s))
