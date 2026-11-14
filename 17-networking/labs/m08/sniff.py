#!/usr/bin/env python3
"""Mini sniffer: đọc Ethernet frame thô bằng AF_PACKET (Linux). Chạy: sudo python3 sniff.py eth0 20"""
import socket
import struct
import sys

ETH_P_ALL = 0x0003
ETHERTYPES = {0x0800: "IPv4", 0x0806: "ARP", 0x86DD: "IPv6", 0x8100: "802.1Q"}
PROTOS = {1: "ICMP", 6: "TCP", 17: "UDP", 132: "SCTP"}


def mac(b: bytes) -> str:
    return ":".join(f"{x:02x}" for x in b)


def describe(frame: bytes) -> str:
    dst, src = frame[0:6], frame[6:12]
    etype = struct.unpack("!H", frame[12:14])[0]
    payload, tag = frame[14:], ""
    if etype == 0x8100:                                   # 802.1Q: 4 byte chèn trước EtherType thật
        vid = struct.unpack("!H", payload[0:2])[0] & 0x0FFF
        etype = struct.unpack("!H", payload[2:4])[0]
        payload, tag = payload[4:], f" vlan={vid}"
    line = f"{mac(src)} > {mac(dst)} {ETHERTYPES.get(etype, hex(etype))}{tag} len={len(frame)}"

    if etype == 0x0800:                                   # IPv4
        ihl = (payload[0] & 0x0F) * 4
        proto = payload[9]
        sip, dip = socket.inet_ntoa(payload[12:16]), socket.inet_ntoa(payload[16:20])
        line += f" | {sip} > {dip} {PROTOS.get(proto, proto)} ttl={payload[8]}"
        if proto in (6, 17):
            sport, dport = struct.unpack("!HH", payload[ihl:ihl + 4])
            line += f" {sport}→{dport}"
    elif etype == 0x0806:                                 # ARP
        op = struct.unpack("!H", payload[6:8])[0]
        spa, tpa = socket.inet_ntoa(payload[14:18]), socket.inet_ntoa(payload[24:28])
        line += (f" | who-has {tpa} tell {spa}" if op == 1
                 else f" | {spa} is-at {mac(payload[8:14])}")
    return line


def main() -> None:
    ifname = sys.argv[1] if len(sys.argv) > 1 else "eth0"
    count = int(sys.argv[2]) if len(sys.argv) > 2 else 20
    with socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.ntohs(ETH_P_ALL)) as s:
        s.bind((ifname, 0))
        for _ in range(count):
            frame, _ = s.recvfrom(65535)
            print(describe(frame))


if __name__ == "__main__":
    main()
