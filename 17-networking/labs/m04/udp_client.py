#!/usr/bin/env python3
"""UDP client có timeout + retry: UDP không retransmit, application tự lo."""
import socket
import sys
import time

SERVER = (sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1", 9000)
TIMEOUT, RETRIES = 0.5, 3


def request(sock: socket.socket, msg: bytes) -> bytes | None:
    for attempt in range(1, RETRIES + 1):
        t0 = time.perf_counter()
        sock.sendto(msg, SERVER)
        try:
            data, _ = sock.recvfrom(2048)
            print(f"  attempt {attempt}: RTT {(time.perf_counter() - t0) * 1000:.2f} ms")
            return data
        except TimeoutError:
            print(f"  attempt {attempt}: timeout sau {TIMEOUT}s")
    return None


with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    s.settimeout(TIMEOUT)
    for line in sys.stdin:
        reply = request(s, line.strip().encode())
        print(reply.decode() if reply else f"→ bỏ cuộc sau {RETRIES} lần")
