#!/usr/bin/env python3
"""rdt 3.0 (stop-and-wait, alternating bit) trên UDP, có giả lập packet loss.
Dùng:  python3 rdt.py recv out.bin [loss]     (cửa sổ 1)
       python3 rdt.py send in.bin  [loss]     (cửa sổ 2)"""
import random
import socket
import struct
import sys
import time

ADDR = ("127.0.0.1", 9500)
CHUNK, TIMEOUT = 1000, 0.2
HDR = struct.Struct("!BBH")            # type (0=DATA, 1=ACK, 2=FIN), seq (0/1), payload length
DATA, ACK, FIN = 0, 1, 2


def lossy_send(sock: socket.socket, pkt: bytes, addr, loss: float) -> None:
    if random.random() >= loss:        # giả lập packet bị drop trên đường đi
        sock.sendto(pkt, addr)


def sender(path: str, loss: float) -> None:
    data = open(path, "rb").read()
    chunks = [data[i:i + CHUNK] for i in range(0, len(data), CHUNK)]
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    seq, sent, t0 = 0, 0, time.perf_counter()
    for i, chunk in enumerate(chunks + [b""]):
        kind = DATA if i < len(chunks) else FIN
        pkt = HDR.pack(kind, seq, len(chunk)) + chunk
        acked = False
        while not acked:
            lossy_send(s, pkt, ADDR, loss)
            sent += 1
            deadline = time.perf_counter() + TIMEOUT
            while (left := deadline - time.perf_counter()) > 0:   # chờ đúng ACK tới hết timer
                s.settimeout(left)
                try:
                    reply, _ = s.recvfrom(64)
                except TimeoutError:
                    break
                k, aseq, _ = HDR.unpack(reply[:HDR.size])
                if k == ACK and aseq == seq:
                    acked = True
                    break                  # ACK cũ (trùng) thì bỏ qua, chờ tiếp
        seq ^= 1
    dt = time.perf_counter() - t0
    print(f"{len(data)} byte, {len(chunks)} chunk, {sent} lần gửi "
          f"({sent - len(chunks) - 1} retransmit), {dt:.2f} s, {len(data) * 8 / dt / 1e6:.2f} Mbit/s")


def receiver(path: str, loss: float) -> None:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(ADDR)
    expect, out, done_at = 0, bytearray(), None
    print(f"receiver on {ADDR}, loss={loss:.0%}")
    while True:
        if done_at is not None:            # đã nhận FIN: nán lại 1 s để ACK lại FIN nếu bị gửi lại
            s.settimeout(max(0.01, done_at + 1 - time.time()))
        try:
            pkt, addr = s.recvfrom(2048)
        except TimeoutError:
            break
        kind, seq, n = HDR.unpack(pkt[:HDR.size])
        if seq == expect:                  # packet mới
            if kind == DATA:
                out += pkt[HDR.size:HDR.size + n]
            elif kind == FIN:
                done_at = time.time()
            expect ^= 1
        lossy_send(s, HDR.pack(ACK, seq, 0), addr, loss)   # mới hay trùng đều ACK lại
    open(path, "wb").write(out)
    print(f"đã ghi {len(out)} byte vào {path}")


if __name__ == "__main__":
    mode, path = sys.argv[1], sys.argv[2]
    loss = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0
    (sender if mode == "send" else receiver)(path, loss)
