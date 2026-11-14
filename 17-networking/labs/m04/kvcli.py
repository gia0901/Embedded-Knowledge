#!/usr/bin/env python3
"""Client cho kvserver. Dùng: kvcli.py set KEY VALUE | get KEY | del KEY"""
import socket
import sys

from kvserver import recv_msg, send_msg     # dùng lại đúng hàm framing của server

op, key, *rest = sys.argv[1:]
req = {"op": op, "key": key}
if rest:
    req["value"] = rest[0]
with socket.create_connection(("127.0.0.1", 9100), timeout=3) as s:
    send_msg(s, req)
    print(recv_msg(s))
