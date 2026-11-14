#!/usr/bin/env python3
"""OAuth 2.0 token server tối giản: client credentials grant, access token JWT ES256, JWKS. Chỉ để học."""
import base64
import hmac
import json
import secrets
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs

import jwt
from cryptography.hazmat.primitives.asymmetric import ec
from jwt.algorithms import ECAlgorithm

ISSUER = "http://127.0.0.1:9000"
KEY = ec.generate_private_key(ec.SECP256R1())   # restart là đổi key; thực tế key nằm trong KMS/HSM
KID = secrets.token_hex(4)
CLIENTS = {  # client_id → (secret, scope được phép). Thực tế: lưu hash của secret, hoặc dùng mTLS / private_key_jwt.
    "svc-a": ("a-secret-123", {"orders:read"}),
    "svc-b": ("b-secret-456", {"orders:read", "orders:write"}),
}


class Handler(BaseHTTPRequestHandler):
    def reply(self, code: int, body: dict, extra: dict | None = None) -> None:
        data = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Cache-Control", "no-store")          # token không được cache
        for k, v in (extra or {}).items():
            self.send_header(k, v)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self) -> None:
        if self.path != "/.well-known/jwks.json":
            return self.reply(404, {"error": "not_found"})
        jwk = json.loads(ECAlgorithm.to_jwk(KEY.public_key()))
        jwk.update(kid=KID, use="sig", alg="ES256")
        self.reply(200, {"keys": [jwk]})

    def client_id(self) -> str | None:
        """client_secret_basic: Authorization: Basic base64(client_id:client_secret)"""
        auth = self.headers.get("Authorization", "")
        if not auth.startswith("Basic "):
            return None
        try:
            cid, _, secret = base64.b64decode(auth[6:]).decode().partition(":")
        except ValueError:
            return None
        known = CLIENTS.get(cid)
        return cid if known and hmac.compare_digest(known[0].encode(), secret.encode()) else None

    def do_POST(self) -> None:
        if self.path != "/token":
            return self.reply(404, {"error": "not_found"})
        form = parse_qs(self.rfile.read(int(self.headers.get("Content-Length", 0))).decode())
        if form.get("grant_type") != ["client_credentials"]:
            return self.reply(400, {"error": "unsupported_grant_type"})
        cid = self.client_id()
        if not cid:
            return self.reply(401, {"error": "invalid_client"}, {"WWW-Authenticate": 'Basic realm="token"'})
        requested, allowed = set(form.get("scope", [""])[0].split()), CLIENTS[cid][1]
        if not requested or not requested <= allowed:
            return self.reply(400, {"error": "invalid_scope", "allowed": sorted(allowed)})
        now = int(time.time())
        claims = {"iss": ISSUER, "sub": cid, "aud": "orders-api", "scope": " ".join(sorted(requested)),
                  "iat": now, "exp": now + 300, "jti": secrets.token_hex(8)}
        token = jwt.encode(claims, KEY, algorithm="ES256", headers={"kid": KID})
        print(f"[token] cấp cho {cid}: scope={claims['scope']}")
        self.reply(200, {"access_token": token, "token_type": "Bearer", "expires_in": 300, "scope": claims["scope"]})

    def log_message(self, *args) -> None:                      # tắt log mặc định của http.server
        pass


if __name__ == "__main__":
    print(f"token server {ISSUER}  kid={KID}")
    ThreadingHTTPServer(("127.0.0.1", 9000), Handler).serve_forever()
