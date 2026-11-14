#!/usr/bin/env python3
"""Longest prefix match trên routing table thật + kiểm tra CIDR trùng nhau."""
import ipaddress
import itertools
import subprocess
import sys


def load_routes():
    routes = []
    for line in subprocess.run(["ip", "-4", "route"], capture_output=True, text=True).stdout.splitlines():
        parts = line.split()
        prefix = "0.0.0.0/0" if parts[0] == "default" else parts[0]
        if "/" not in prefix:
            prefix += "/32"                       # route tới đúng một host
        via = parts[parts.index("via") + 1] if "via" in parts else "(trực tiếp)"
        dev = parts[parts.index("dev") + 1]
        routes.append((ipaddress.ip_network(prefix, strict=False), via, dev))
    return routes


def lpm(routes, dst: str):
    ip = ipaddress.ip_address(dst)
    matches = [r for r in routes if ip in r[0]]
    return max(matches, key=lambda r: r[0].prefixlen) if matches else None


def check_overlaps(plan: dict) -> None:
    nets = {name: ipaddress.ip_network(cidr) for name, cidr in plan.items()}
    clean = True
    for (a, na), (b, nb) in itertools.combinations(nets.items(), 2):
        if na.overlaps(nb):
            print(f"  ✘ {a} {na} TRÙNG {b} {nb}")
            clean = False
    if clean:
        print("  ✔ không có dải nào trùng")


if __name__ == "__main__":
    routes = load_routes()
    for dst in sys.argv[1:] or ["1.1.1.1", "172.17.0.2", "127.0.0.1"]:
        r = lpm(routes, dst)
        print(f"{dst:<15} → {r[0]} via {r[1]} dev {r[2]}" if r else f"{dst}: không có route")

    print("\nKiểm tra kế hoạch address cho cluster mới:")
    check_overlaps({
        "node network": "10.20.0.0/22",
        "pod CIDR": "10.244.0.0/16",
        "service CIDR": "10.96.0.0/12",
        "docker bridge": "172.17.0.0/16",
        "site HCM": "10.20.2.0/24",
        "site HN": "10.100.0.0/16",
    })
