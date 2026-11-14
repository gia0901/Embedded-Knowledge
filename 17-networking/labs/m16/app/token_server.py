#!/usr/bin/env python3
"""Token server: OAuth 2.0 client credentials. Client authenticate bằng mTLS (RFC 8705 tls_client_auth);
access token JWT ES256 gắn với client cert (cnf.x5t#S256); JWKS chứa mọi key để xoay vòng."""
import glob
import json
import os
import secrets
import time
from pathlib import Path

import jwt
from cryptography.hazmat.primitives.serialization import load_pem_private_key
from jwt.algorithms import ECAlgorithm

from common import JSONHandler, MTLSServer, ReloadingTLS, log

ISSUER = os.environ.get("ISSUER", "https://token.security.svc.cluster.local:8443")
AUDIENCE = os.environ.get("AUDIENCE", "orders-api")
TTL = int(os.environ.get("TOKEN_TTL", "300"))
ACTIVE_KID = os.environ["ACTIVE_KID"]
KEYS = {Path(p).stem: load_pem_private_key(Path(p).read_bytes(), None)
        for p in glob.glob(os.path.join(os.environ.get("KEYS_DIR", "/keys"), "*.pem"))}
CLIENTS_FILE = os.environ.get("CLIENTS_FILE", "/config/clients.json")
_clients = {"mtime": None, "data": {}}


def clients() -> dict:
    """SPIFFE ID → danh sách scope. Đọc lại khi ConfigMap đổi: thu hồi client không cần restart."""
    mtime = os.stat(CLIENTS_FILE).st_mtime
    if mtime != _clients["mtime"]:
        _clients.update(mtime=mtime, data=json.loads(Path(CLIENTS_FILE).read_text()))
        log("clients_loaded", clients=sorted(_clients["data"]))
    return _clients["data"]


def jwks() -> dict:
    keys = []
    for kid, key in sorted(KEYS.items()):
        jwk = json.loads(ECAlgorithm.to_jwk(key.public_key()))
        jwk.update(kid=kid, use="sig", alg="ES256")
        keys.append(jwk)
    return {"keys": keys}


class Handler(JSONHandler):
    def do_GET(self) -> None:
        if self.path == "/.well-known/jwks.json":
            return self.reply(200, jwks())
        self.reply(404, {"error": "not_found"})

    def do_POST(self) -> None:
        if self.path != "/token":
            return self.reply(404, {"error": "not_found"})
        form, who = self.read_form(), self.spiffe_id
        if form.get("grant_type") != "client_credentials":
            return self.reply(400, {"error": "unsupported_grant_type"})
        allowed = clients().get(who)
        if allowed is None:                                      # cert hợp lệ nhưng client không được đăng ký
            log("token_denied", client=who, reason="invalid_client")
            return self.reply(401, {"error": "invalid_client"})
        requested = set(form.get("scope", "").split())
        if not requested or not requested <= set(allowed):
            log("token_denied", client=who, reason="invalid_scope", requested=sorted(requested))
            return self.reply(400, {"error": "invalid_scope"})
        now = int(time.time())
        claims = {"iss": ISSUER, "sub": who, "aud": AUDIENCE, "scope": " ".join(sorted(requested)),
                  "iat": now, "exp": now + TTL, "jti": secrets.token_hex(8),
                  "cnf": {"x5t#S256": self.thumb}}                # gắn token với đúng cert này
        token = jwt.encode(claims, KEYS[ACTIVE_KID], algorithm="ES256", headers={"kid": ACTIVE_KID})
        log("token_issued", client=who, scope=claims["scope"], jti=claims["jti"], kid=ACTIVE_KID)
        self.reply(200, {"access_token": token, "token_type": "Bearer", "expires_in": TTL, "scope": claims["scope"]})


if __name__ == "__main__":
    log("start", service="token-server", issuer=ISSUER, kids=sorted(KEYS), active_kid=ACTIVE_KID)
    MTLSServer(("0.0.0.0", 8443), Handler, ReloadingTLS(server_side=True)).serve_forever()
