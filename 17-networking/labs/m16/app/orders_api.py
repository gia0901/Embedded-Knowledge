#!/usr/bin/env python3
"""orders-api: resource server. mTLS bắt buộc; token verify bằng JWKS của token server;
kiểm tra iss, aud, exp, scope và cnf (token phải đi cùng đúng client cert đã xin nó)."""
import json
import os
import threading
import time
import urllib.request

import jwt

from common import JSONHandler, MTLSServer, ReloadingTLS, log

ISSUER = os.environ.get("ISSUER", "https://token.security.svc.cluster.local:8443")
AUDIENCE = os.environ.get("AUDIENCE", "orders-api")
ROUTES = {("GET", "/orders"): "orders:read", ("POST", "/orders"): "orders:write"}
CLIENT_TLS = ReloadingTLS(server_side=False)                # cert của chính orders-api khi gọi token server


class JWKSCache:
    def __init__(self):
        self.keys, self.fetched, self.lock = {}, 0.0, threading.Lock()

    def refresh(self) -> None:
        with urllib.request.urlopen(ISSUER + "/.well-known/jwks.json", context=CLIENT_TLS.get(), timeout=3) as r:
            data = json.load(r)
        self.keys = {k["kid"]: jwt.PyJWK(k).key for k in data["keys"]}
        self.fetched = time.time()
        log("jwks_refreshed", kids=sorted(self.keys))

    def key(self, kid: str):
        with self.lock:
            if kid not in self.keys and time.time() - self.fetched > 10:      # kid lạ: tải lại, tối đa 10 s/lần
                self.refresh()
            return self.keys.get(kid)


JWKS = JWKSCache()


class Handler(JSONHandler):
    def deny(self, code: int, error: str, need: str, detail: str = "", **extra) -> None:
        log("deny", client=self.spiffe_id, action=f"{self.command} {self.path}", status=code, error=error, detail=detail, **extra)
        hdr = f'Bearer error="{error}"' + (f', scope="{need}"' if code == 403 else "")
        self.reply(code, {"error": error, "detail": detail}, {"WWW-Authenticate": hdr})

    def api(self, method: str) -> None:
        need = ROUTES.get((method, self.path))
        if not need:
            return self.reply(404, {"error": "not_found"})
        auth = self.headers.get("Authorization", "")
        if not auth.startswith("Bearer "):
            return self.deny(401, "invalid_request", need, "thiếu Bearer token")
        token = auth[7:]
        try:
            kid = jwt.get_unverified_header(token).get("kid", "")
            key = JWKS.key(kid)
            if key is None:
                raise jwt.InvalidTokenError(f"kid không có trong JWKS: {kid}")
            claims = jwt.decode(token, key, algorithms=["ES256"], audience=AUDIENCE, issuer=ISSUER, leeway=30,
                                options={"require": ["exp", "iat", "sub", "scope", "cnf"]})
        except (jwt.PyJWTError, OSError) as e:
            return self.deny(401, "invalid_token", need, str(e))
        if claims["cnf"].get("x5t#S256") != self.thumb:                       # token đánh cắp, dùng với cert khác
            return self.deny(401, "invalid_token", need, "token không gắn với client cert này", sub=claims["sub"])
        if need not in claims["scope"].split():                               # authorization
            return self.deny(403, "insufficient_scope", need, sub=claims["sub"])
        log("allow", client=self.spiffe_id, sub=claims["sub"], action=f"{method} {self.path}", jti=claims.get("jti"))
        self.reply(200, {"ok": True, "caller": claims["sub"], "action": f"{method} {self.path}"})

    def do_GET(self) -> None:
        self.api("GET")

    def do_POST(self) -> None:
        self.api("POST")


if __name__ == "__main__":
    log("start", service="orders-api", issuer=ISSUER, audience=AUDIENCE)
    MTLSServer(("0.0.0.0", 8443), Handler, ReloadingTLS(server_side=True)).serve_forever()
