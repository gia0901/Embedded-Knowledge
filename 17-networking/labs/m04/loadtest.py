#!/usr/bin/env python3
"""Mở N connection cùng lúc tới echo server, mỗi connection gửi một dòng và kiểm tra echo."""
import socket
import sys
import time

N = int(sys.argv[1]) if len(sys.argv) > 1 else 500
t0 = time.perf_counter()
socks = [socket.create_connection(("127.0.0.1", 9200)) for _ in range(N)]
for i, s in enumerate(socks):
    s.sendall(f"hello {i}\n".encode())
ok = sum(s.recv(64) == f"hello {i}\n".encode() for i, s in enumerate(socks))
print(f"{ok}/{N} echo đúng trong {time.perf_counter() - t0:.2f}s")
input("Enter để đóng hết connection (trước đó thử: ss -tn | grep -c 9200) ")
for s in socks:
    s.close()
