#!/usr/bin/env python3
"""TOTP (RFC 6238) = HOTP (RFC 4226) với counter = thời gian / 30 giây."""
import base64
import hashlib
import hmac
import secrets
import struct
import sys
import time


def totp(secret_b32: str, t: float | None = None, step: int = 30, digits: int = 6) -> str:
    key = base64.b32decode(secret_b32.upper() + "=" * (-len(secret_b32) % 8))
    counter = int((time.time() if t is None else t) // step)
    mac = hmac.new(key, struct.pack(">Q", counter), hashlib.sha1).digest()
    offset = mac[-1] & 0x0F                                   # dynamic truncation
    code = (struct.unpack(">I", mac[offset:offset + 4])[0] & 0x7FFFFFFF) % 10 ** digits
    return f"{code:0{digits}d}"


if __name__ == "__main__":
    rfc = base64.b32encode(b"12345678901234567890").decode()
    assert totp(rfc, t=59, digits=8) == "94287082", "sai test vector RFC 6238"
    print("test vector RFC 6238: OK")
    secret = sys.argv[1] if len(sys.argv) > 1 else base64.b32encode(secrets.token_bytes(20)).decode()
    print("secret (nhập tay vào app authenticator):", secret)
    print("otpauth URI (để làm QR):", f"otpauth://totp/NetLab:alice?secret={secret}&issuer=NetLab")
    while True:
        print(f"\r{totp(secret)}   còn {30 - int(time.time()) % 30:2d} giây ", end="", flush=True)
        time.sleep(1)
