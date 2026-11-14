#!/usr/bin/env python3
"""Kiểm tra hạn và tính hợp lệ của certificate. Exit 1 nếu có cert sắp hết hạn hoặc verify thất bại."""
import argparse
import socket
import ssl
import sys
import time


def check(host: str, port: int, ctx: ssl.SSLContext, warn_days: int) -> tuple[bool, str]:
    try:
        with socket.create_connection((host, port), timeout=5) as raw:
            with ctx.wrap_socket(raw, server_hostname=host) as tls:   # SNI + verify chain + hostname
                cert = tls.getpeercert()
                version = tls.version()
    except ssl.SSLCertVerificationError as e:
        return False, f"VERIFY FAIL  {e.verify_message}"
    except (OSError, ssl.SSLError) as e:
        return False, f"CONNECT FAIL {e}"
    days = (ssl.cert_time_to_seconds(cert["notAfter"]) - time.time()) / 86400
    san = ", ".join(v for k, v in cert.get("subjectAltName", ()) if k in ("DNS", "IP Address", "URI"))
    issuer = dict(x[0] for x in cert["issuer"]).get("commonName", "?")
    ok = days >= warn_days
    return ok, f"{'OK' if ok else 'EXPIRING':<12} {days:6.1f} ngày  {version}  issuer={issuer}  SAN={san[:60]}"


def main() -> None:
    ap = argparse.ArgumentParser(description="Giám sát hạn certificate")
    ap.add_argument("targets", nargs="+", help="host:port")
    ap.add_argument("--cafile", help="root CA nội bộ (mặc định: trust store của hệ thống)")
    ap.add_argument("--warn-days", type=int, default=21)
    args = ap.parse_args()

    ctx = ssl.create_default_context(cafile=args.cafile)
    all_ok = True
    for target in args.targets:
        host, _, port = target.partition(":")
        ok, msg = check(host, int(port or 443), ctx, args.warn_days)
        all_ok &= ok
        print(f"{target:<28} {msg}")
    sys.exit(0 if all_ok else 1)


if __name__ == "__main__":
    main()
