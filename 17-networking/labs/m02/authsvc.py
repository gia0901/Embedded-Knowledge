#!/usr/bin/env python3
"""Mini REST service có Bearer token authentication. Chỉ dùng thư viện chuẩn."""
import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

# token → identity + scope. Module 12 sẽ thay bảng này bằng JWT có chữ ký.
TOKENS = {
    "tok-reader": {"sub": "svc-a", "scope": {"secrets:read"}},
    "tok-guest":  {"sub": "svc-b", "scope": set()},
}
SECRETS = [{"name": "db-password", "version": 3}, {"name": "tls-key", "version": 7}]
REALM = 'Bearer realm="netlab"'


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"          # persistent connection; bắt buộc có Content-Length
    server_version = "netlab-auth/0.1"
    sub = "-"                              # identity của request hiện tại, dùng cho log

    def send_json(self, status, body, headers=None):
        data = json.dumps(body).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        for k, v in (headers or {}).items():
            self.send_header(k, v)
        self.end_headers()
        if self.command != "HEAD":         # HEAD: chỉ header, không body
            self.wfile.write(data)

    def authenticate(self):
        """Trả về identity, hoặc None sau khi đã gửi 401."""
        scheme, _, token = self.headers.get("Authorization", "").partition(" ")
        if scheme.lower() != "bearer" or not token:
            self.send_json(401, {"error": "missing_token"}, {"WWW-Authenticate": REALM})
            return None
        ident = TOKENS.get(token)
        if ident is None:
            self.send_json(401, {"error": "invalid_token"},
                           {"WWW-Authenticate": REALM + ', error="invalid_token"'})
            return None
        self.sub = ident["sub"]
        return ident

    def do_GET(self):
        self.sub = "-"                     # persistent connection: một handler phục vụ nhiều request
        if self.path == "/health":
            return self.send_json(200, {"status": "ok"})
        if self.path == "/api/v1/secrets":
            ident = self.authenticate()
            if ident is None:
                return
            if "secrets:read" not in ident["scope"]:
                return self.send_json(403, {"error": "insufficient_scope"},
                    {"WWW-Authenticate": REALM + ', error="insufficient_scope", scope="secrets:read"'})
            return self.send_json(200, {"items": SECRETS})
        self.send_json(404, {"error": "not_found", "path": self.path})

    do_HEAD = do_GET

    def do_POST(self):
        self.sub = "-"
        if self.path == "/api/v1/secrets":
            return self.send_json(405, {"error": "method_not_allowed"}, {"Allow": "GET, HEAD"})
        self.send_json(404, {"error": "not_found", "path": self.path})

    def log_request(self, code="-", size="-"):
        print(f"{self.client_address[0]} {self.command} {self.path} -> {code} sub={self.sub}", flush=True)


if __name__ == "__main__":
    srv = ThreadingHTTPServer(("0.0.0.0", 8080), Handler)
    print("listening on :8080")
    srv.serve_forever()
