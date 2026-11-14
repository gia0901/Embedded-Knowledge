#!/usr/bin/env python3
"""JWT HS256 tự viết để hiểu (production: dùng library, ví dụ PyJWT). So verifier đúng với verifier ngây thơ."""
import base64
import hashlib
import hmac
import json
import time

SECRET = b"netlab-demo-secret-please-change-me-32"


class InvalidToken(Exception):
    pass


def b64u(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode()


def b64u_dec(s: str) -> bytes:
    return base64.urlsafe_b64decode(s + "=" * (-len(s) % 4))


def compact(obj: dict) -> bytes:
    return json.dumps(obj, separators=(",", ":")).encode()


def sign(claims: dict, key: bytes = SECRET) -> str:
    signing_input = b64u(compact({"alg": "HS256", "typ": "JWT"})) + "." + b64u(compact(claims))
    return signing_input + "." + b64u(hmac.new(key, signing_input.encode(), hashlib.sha256).digest())


def verify(token: str, *, audience: str, issuer: str, key: bytes = SECRET, leeway: int = 30) -> dict:
    try:
        h64, p64, s64 = token.split(".")
        header, claims = json.loads(b64u_dec(h64)), json.loads(b64u_dec(p64))
    except ValueError as e:                                    # sai số phần, base64 hay JSON hỏng
        raise InvalidToken(f"token hỏng: {e}") from None
    if header.get("alg") != "HS256":                           # 1. allowlist, không tin header
        raise InvalidToken(f"alg không được phép: {header.get('alg')}")
    expected = hmac.new(key, f"{h64}.{p64}".encode(), hashlib.sha256).digest()
    if not hmac.compare_digest(expected, b64u_dec(s64)):       # 2. chữ ký, so constant-time
        raise InvalidToken("sai chữ ký")
    if claims.get("iss") != issuer:                            # 3. đúng nơi cấp
        raise InvalidToken(f"iss sai: {claims.get('iss')}")
    aud = claims.get("aud")
    if audience not in (aud if isinstance(aud, list) else [aud]):   # 4. token dành cho mình
        raise InvalidToken(f"aud sai: {aud}")
    now = time.time()
    if not isinstance(claims.get("exp"), (int, float)) or now > claims["exp"] + leeway:   # 5. còn hạn
        raise InvalidToken("hết hạn (hoặc thiếu exp)")
    if now + leeway < claims.get("nbf", 0):
        raise InvalidToken("chưa tới nbf")
    return claims


def naive_verify(token: str) -> dict:
    """SAI có chủ đích: tin alg trong header, không kiểm tra iss/aud/exp."""
    h64, p64, s64 = token.split(".")
    if json.loads(b64u_dec(h64)).get("alg") == "none":
        return json.loads(b64u_dec(p64))
    expected = hmac.new(SECRET, f"{h64}.{p64}".encode(), hashlib.sha256).digest()
    if not hmac.compare_digest(expected, b64u_dec(s64)):
        raise InvalidToken("sai chữ ký")
    return json.loads(b64u_dec(p64))


if __name__ == "__main__":
    now = int(time.time())
    iss = "https://auth.netlab.local"
    base = {"iss": iss, "sub": "svc-a", "aud": "orders-api", "scope": "orders:read", "iat": now, "exp": now + 300}
    good = sign(base)
    h64, _, s64 = good.split(".")
    admin_p64 = b64u(compact({**base, "scope": "orders:admin"}))
    cases = {
        "hợp lệ": good,
        "sửa scope, giữ chữ ký cũ": f"{h64}.{admin_p64}.{s64}",
        "alg=none": b64u(compact({"alg": "none", "typ": "JWT"})) + f".{admin_p64}.",
        "sai audience": sign({**base, "aud": "billing-api"}),
        "hết hạn": sign({**base, "iat": now - 7200, "exp": now - 3600}),
        "ký bằng secret khác": sign(base, key=b"guessed-secret"),
    }
    verifiers = (("naive", naive_verify), ("đúng ", lambda t: verify(t, audience="orders-api", issuer=iss)))
    for name, token in cases.items():
        for label, fn in verifiers:
            try:
                result = f"CHẤP NHẬN scope={fn(token)['scope']}"
            except InvalidToken as e:
                result = f"từ chối: {e}"
            print(f"{name:<26} {label}  {result}")
