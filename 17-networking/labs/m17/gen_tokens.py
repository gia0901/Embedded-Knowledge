#!/usr/bin/env python3
"""Sinh jwks.json và tokens.txt (tên, kết quả mong đợi, token) để đối chiếu bản C++ với PyJWT."""
import base64
import json
import time

import jwt
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from jwt.algorithms import ECAlgorithm

ISS, AUD = "https://token.security.svc.cluster.local:8443", "orders-api"
k1, other = ec.generate_private_key(ec.SECP256R1()), ec.generate_private_key(ec.SECP256R1())
jwk = json.loads(ECAlgorithm.to_jwk(k1.public_key()))
jwk.update(kid="k1", use="sig", alg="ES256")
with open("jwks.json", "w") as f:
    json.dump({"keys": [jwk]}, f)


def b64(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode()


def j(obj: dict) -> str:
    return b64(json.dumps(obj, separators=(",", ":")).encode())


def es(claims: dict, key=k1, kid="k1") -> str:
    return jwt.encode(claims, key, algorithm="ES256", headers={"kid": kid})


now = int(time.time())
base = {"iss": ISS, "aud": AUD, "sub": "spiffe://netlab.local/ns/web/sa/frontend", "scope": "orders:read",
        "iat": now, "exp": now + 300, "cnf": {"x5t#S256": "abc"}}
good = es(base)
h, p, s = good.split(".")
der = k1.sign(f"{h}.{p}".encode(), ec.ECDSA(hashes.SHA256()))         # chữ ký đúng nhưng mã hoá DER, sai chuẩn JWT
cases = [
    ("valid", "ok", good),
    ("tampered_scope", "reject", f"{h}.{j({**base, 'scope': 'orders:write'})}.{s}"),
    ("alg_none", "reject", j({"alg": "none", "kid": "k1"}) + f".{p}."),
    ("alg_hs256", "reject", jwt.encode(base, "x" * 32, algorithm="HS256", headers={"kid": "k1"})),
    ("sig_der", "reject", f"{h}.{p}.{b64(der)}"),
    ("wrong_key", "reject", es(base, other)),
    ("unknown_kid", "reject", es(base, kid="k9")),
    ("expired", "reject", es({**base, "iat": now - 7200, "exp": now - 3600})),
    ("wrong_aud", "reject", es({**base, "aud": "billing-api"})),
    ("wrong_iss", "reject", es({**base, "iss": "https://evil.example"})),
    ("no_cnf", "reject", es({k: v for k, v in base.items() if k != "cnf"})),
]
with open("tokens.txt", "w") as f:
    for name, expect, token in cases:
        f.write(f"{name} {expect} {token}\n")
print(f"đã ghi jwks.json và {len(cases)} token vào tokens.txt")
