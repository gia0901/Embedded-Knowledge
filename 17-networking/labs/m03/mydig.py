#!/usr/bin/env python3
"""DNS client tối giản: tự build query, gửi qua UDP, parse A/AAAA/CNAME/NS trong answer."""
import ipaddress
import random
import socket
import struct
import sys

TYPES = {"A": 1, "NS": 2, "CNAME": 5, "MX": 15, "TXT": 16, "AAAA": 28}
TYPE_NAMES = {v: k for k, v in TYPES.items()}
RCODES = {0: "NOERROR", 1: "FORMERR", 2: "SERVFAIL", 3: "NXDOMAIN", 5: "REFUSED"}


def build_query(name: str, qtype: int) -> tuple[int, bytes]:
    qid = random.randint(0, 0xFFFF)              # ID ngẫu nhiên: chống đoán ID (cache poisoning)
    header = struct.pack("!HHHHHH", qid, 0x0100, 1, 0, 0, 0)   # RD=1, 1 question
    qname = b"".join(bytes([len(p)]) + p.encode() for p in name.rstrip(".").split(".")) + b"\x00"
    return qid, header + qname + struct.pack("!HH", qtype, 1)  # QCLASS 1 = IN


def read_name(msg: bytes, off: int) -> tuple[str, int]:
    """Đọc một domain name tại offset, xử lý compression pointer. Trả về (name, offset kế tiếp)."""
    labels, end = [], None
    while True:
        n = msg[off]
        if n == 0:
            off += 1
            break
        if n & 0xC0 == 0xC0:                     # 2 bit đầu = 11 → pointer 14 bit
            if end is None:
                end = off + 2                    # sau pointer, đọc tiếp từ đây
            off = struct.unpack("!H", msg[off:off + 2])[0] & 0x3FFF
            continue
        labels.append(msg[off + 1:off + 1 + n].decode())
        off += 1 + n
    return ".".join(labels) + ".", (end if end is not None else off)


def parse(msg: bytes, qid: int) -> None:
    rid, flags, qd, an, ns, ar = struct.unpack("!HHHHHH", msg[:12])
    if rid != qid:
        raise ValueError("ID của reply không khớp query: bỏ qua (có thể là reply giả mạo)")
    bit = lambda n: (flags >> n) & 1
    print(f"status={RCODES.get(flags & 0xF, flags & 0xF)}  "
          f"qr={bit(15)} aa={bit(10)} tc={bit(9)} rd={bit(8)} ra={bit(7)}  "
          f"answer={an} authority={ns} additional={ar}")
    off = 12
    for _ in range(qd):                          # bỏ qua question section
        _, off = read_name(msg, off)
        off += 4
    for _ in range(an):
        name, off = read_name(msg, off)
        rtype, _rclass, ttl, rdlen = struct.unpack("!HHIH", msg[off:off + 10])
        off += 10
        rdata = msg[off:off + rdlen]
        if rtype == 1:
            value = str(ipaddress.IPv4Address(rdata))
        elif rtype == 28:
            value = str(ipaddress.IPv6Address(rdata))
        elif rtype in (2, 5):
            value, _ = read_name(msg, off)       # NS/CNAME: rdata là một name (có thể bị nén)
        else:
            value = rdata.hex()
        print(f"  {name:<26} TTL={ttl:<6} {TYPE_NAMES.get(rtype, rtype):<6} {value}")
        off += rdlen


if __name__ == "__main__":
    name = sys.argv[1] if len(sys.argv) > 1 else "example.com"
    qtype = TYPES[sys.argv[2].upper()] if len(sys.argv) > 2 else TYPES["A"]
    server = sys.argv[3] if len(sys.argv) > 3 else "8.8.8.8"
    qid, query = build_query(name, qtype)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.settimeout(3)
        s.sendto(query, (server, 53))
        reply, _ = s.recvfrom(4096)
    print(f"query {len(query)} byte → reply {len(reply)} byte từ {server}")
    parse(reply, qid)
