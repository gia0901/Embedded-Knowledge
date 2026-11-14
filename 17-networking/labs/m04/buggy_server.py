#!/usr/bin/env python3
"""Server có bug: đọc xong thì quên close() connection."""
import socket

ls = socket.socket()
ls.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
ls.bind(("127.0.0.1", 9300))
ls.listen()
leaked = []
while True:
    conn, _ = ls.accept()
    conn.recv(100)
    leaked.append(conn)          # giữ reference nên Python không tự đóng socket
    print("sockets đang giữ:", len(leaked))
