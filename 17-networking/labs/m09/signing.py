#!/usr/bin/env python3
"""HMAC request signing, Ed25519 signature, và RSA với số nhỏ."""
import hashlib
import hmac
import secrets
import time

from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey

# ---------- 1. HMAC request signing ----------
SHARED_KEY = secrets.token_bytes(32)            # CSPRNG, không dùng module random
MAX_AGE = 300                                   # giây


def sign_request(method: str, path: str, body: bytes, ts: int) -> str:
    msg = b"\n".join([method.encode(), path.encode(), str(ts).encode(), body])
    return hmac.new(SHARED_KEY, msg, hashlib.sha256).hexdigest()


def verify_request(method, path, body, ts, sig, now=None) -> str:
    now = now or int(time.time())
    if abs(now - ts) > MAX_AGE:
        return "từ chối: quá hạn (có thể là replay)"
    expected = sign_request(method, path, body, ts)
    if not hmac.compare_digest(expected, sig):  # constant-time
        return "từ chối: sai chữ ký"
    return "OK"


ts = int(time.time())
body = b'{"amount":100}'
sig = sign_request("POST", "/transfer", body, ts)
print("1a. request đúng:     ", verify_request("POST", "/transfer", body, ts, sig))
print("1b. body bị sửa:      ", verify_request("POST", "/transfer", b'{"amount":999}', ts, sig))
print("1c. gửi lại sau 10 ph:", verify_request("POST", "/transfer", body, ts, sig, now=ts + 600))

# ---------- 2. Ed25519 ----------
priv_a = Ed25519PrivateKey.generate()           # chỉ service A giữ
pub_a = priv_a.public_key()                     # service B chỉ có cái này
msg = b"role=reader;sub=svc-b"
sig = priv_a.sign(msg)
print(f"\n2. chữ ký {len(sig)} byte")
for label, m in [("message gốc", msg), ("message bị sửa", b"role=admin;sub=svc-b")]:
    try:
        pub_a.verify(sig, m)
        print(f"   {label}: chữ ký hợp lệ")
    except InvalidSignature:
        print(f"   {label}: InvalidSignature")
print("   B có hàm ký không?", hasattr(pub_a, "sign"))   # public key không ký được

# ---------- 3. RSA đồ chơi (KHÔNG dùng thật: không padding, số quá nhỏ) ----------
p, q, e = 61, 53, 17
n, phi = p * q, (p - 1) * (q - 1)
d = pow(e, -1, phi)                             # e·d ≡ 1 (mod φ)
m = 65
c = pow(m, e, n)                                # mã hoá bằng public key (e, n)
print(f"\n3. n={n} φ={phi} d={d}")
print(f"   encrypt {m} → {c}, decrypt → {pow(c, d, n)}")
s = pow(m, d, n)                                # ký bằng private key (d, n)
print(f"   sign {m} → {s}, verify: {pow(s, e, n) == m}")
