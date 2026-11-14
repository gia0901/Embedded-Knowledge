#!/usr/bin/env python3
"""Key-value server: TCP + length-prefix framing + thread-per-connection."""
import json
import socket
import struct
import threading

HOST, PORT = "0.0.0.0", 9100
MAX_MSG = 64 * 1024          # chặn độ dài giả để DoS
IDLE_TIMEOUT = 30            # giây; chống client giữ connection mà không gửi gì
store, lock = {}, threading.Lock()


def recv_exact(conn: socket.socket, n: int) -> bytes:
    """Đọc đúng n byte. TCP là byte stream nên một recv() có thể trả ít hơn."""
    buf = bytearray()
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:                                # b"" = peer đã đóng connection
            raise ConnectionError("peer closed")
        buf += chunk
    return bytes(buf)


def recv_msg(conn: socket.socket) -> dict:
    (length,) = struct.unpack("!I", recv_exact(conn, 4))
    if length > MAX_MSG:
        raise ValueError(f"message {length} byte vượt giới hạn {MAX_MSG}")
    return json.loads(recv_exact(conn, length))


def send_msg(conn: socket.socket, obj: dict) -> None:
    data = json.dumps(obj).encode()
    conn.sendall(struct.pack("!I", len(data)) + data)   # sendall lặp tới khi gửi hết


def handle(req: dict) -> dict:
    op, key = req.get("op"), req.get("key")
    with lock:                                           # nhiều thread dùng chung store
        if op == "set":
            store[key] = req.get("value")
            return {"ok": True}
        if op == "get":
            return {"ok": key in store, "value": store.get(key)}
        if op == "del":
            return {"ok": store.pop(key, None) is not None}
    return {"ok": False, "error": f"unknown op {op!r}"}


def serve_client(conn: socket.socket, addr) -> None:
    conn.settimeout(IDLE_TIMEOUT)
    print(f"+ {addr} connected ({threading.active_count() - 1} clients)")
    try:
        with conn:                                       # luôn close, kể cả khi có exception
            while True:
                send_msg(conn, handle(recv_msg(conn)))
    except (ConnectionError, TimeoutError, ValueError) as e:
        print(f"- {addr} closed: {e}")


def main() -> None:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as ls:
        ls.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        ls.bind((HOST, PORT))
        ls.listen(128)
        print(f"listening on {HOST}:{PORT}")
        while True:
            conn, addr = ls.accept()                     # mỗi client một connection socket mới
            threading.Thread(target=serve_client, args=(conn, addr), daemon=True).start()


if __name__ == "__main__":
    main()
