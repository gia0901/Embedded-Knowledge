#!/usr/bin/env python3
"""Resource server: verify access token bằng JWKS của token server, rồi kiểm tra scope."""
import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import jwt

ISSUER = "http://127.0.0.1:9000"
JWKS = jwt.PyJWKClient(ISSUER + "/.well-known/jwks.json")     # tải và cache key theo kid
ROUTES = {("GET", "/orders"): "orders:read", ("POST", "/orders"): "orders:write"}


class Handler(BaseHTTPRequestHandler):
    def reply(self, code: int, body: dict, extra: dict | None = None) -> None:
        data = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        for k, v in (extra or {}).items():
            self.send_header(k, v)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def api(self, method: str) -> None:
        need = ROUTES.get((method, self.path))
        if not need:
            return self.reply(404, {"error": "not_found"})
        auth = self.headers.get("Authorization", "")
        if not auth.startswith("Bearer "):                                    # AuthN: chưa có credential
            return self.reply(401, {"error": "missing_token"}, {"WWW-Authenticate": "Bearer"})
        token = auth[7:]
        try:
            key = JWKS.get_signing_key_from_jwt(token).key
            claims = jwt.decode(token, key, algorithms=["ES256"], audience="orders-api", issuer=ISSUER,
                                leeway=30, options={"require": ["exp", "iat", "iss", "aud", "sub"]})
        except jwt.PyJWTError as e:                                           # AuthN: credential sai
            return self.reply(401, {"error": "invalid_token", "detail": str(e)},
                              {"WWW-Authenticate": 'Bearer error="invalid_token"'})
        if need not in claims.get("scope", "").split():                       # AuthZ
            return self.reply(403, {"error": "insufficient_scope", "need": need},
                              {"WWW-Authenticate": f'Bearer error="insufficient_scope", scope="{need}"'})
        print(f"[api] {claims['sub']} {method} {self.path} (jti={claims.get('jti')})")   # audit log
        self.reply(200, {"ok": True, "caller": claims["sub"], "action": f"{method} {self.path}"})

    def do_GET(self) -> None:
        self.api("GET")

    def do_POST(self) -> None:
        self.api("POST")

    def log_message(self, *args) -> None:
        pass


if __name__ == "__main__":
    print("orders-api http://127.0.0.1:9001")
    ThreadingHTTPServer(("127.0.0.1", 9001), Handler).serve_forever()
