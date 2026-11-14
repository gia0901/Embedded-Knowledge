"""Dùng chung: TLS tự nạp lại cert, identity từ client cert, audit log, HTTP server mTLS."""
import base64
import hashlib
import json
import os
import ssl
import sys
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs

TLS_DIR = os.environ.get("TLS_DIR", "/tls")                 # tls.crt (leaf + chain), tls.key từ cert-manager
TRUST_FILE = os.environ.get("TRUST_FILE", "/trust/root.crt")


def log(event: str, **fields) -> None:
    """Audit log: mỗi dòng một JSON (kubectl logs … | jq)."""
    print(json.dumps({"ts": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()), "event": event, **fields}), flush=True)


class ReloadingTLS:
    """Giữ một SSLContext và tạo lại khi tls.crt đổi. Connection mới dùng cert mới, không restart."""

    def __init__(self, server_side: bool):
        self.server_side, self.mtime, self.ctx = server_side, None, None

    def get(self) -> ssl.SSLContext:
        crt, key = os.path.join(TLS_DIR, "tls.crt"), os.path.join(TLS_DIR, "tls.key")
        mtime = os.stat(crt).st_mtime
        if mtime != self.mtime:
            ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER if self.server_side else ssl.PROTOCOL_TLS_CLIENT)
            ctx.minimum_version = ssl.TLSVersion.TLSv1_3
            ctx.load_cert_chain(crt, key)
            ctx.load_verify_locations(TRUST_FILE)
            ctx.verify_mode = ssl.CERT_REQUIRED                 # server: bắt buộc client cert; client: verify server
            self.ctx, self.mtime = ctx, mtime
            log("tls_context_loaded", side="server" if self.server_side else "client", mtime=int(mtime))
        return self.ctx


def peer_identity(tls: ssl.SSLSocket) -> tuple[str | None, str]:
    """(SPIFFE ID duy nhất trong SAN, x5t#S256 = base64url(SHA-256(DER của client cert)))."""
    cert = tls.getpeercert() or {}
    ids = [v for k, v in cert.get("subjectAltName", ()) if k == "URI" and v.startswith("spiffe://")]
    der = tls.getpeercert(binary_form=True) or b""
    thumb = base64.urlsafe_b64encode(hashlib.sha256(der).digest()).rstrip(b"=").decode()
    return (ids[0] if len(ids) == 1 else None), thumb


class MTLSServer(ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self, addr, handler, tls: ReloadingTLS):
        self.tls = tls
        super().__init__(addr, handler)

    def get_request(self):
        sock, addr = self.socket.accept()
        return self.tls.get().wrap_socket(sock, server_side=True, do_handshake_on_connect=False), addr

    def handle_error(self, request, client_address):
        err = sys.exc_info()[1]
        log("connection_error", client=client_address[0], error=f"{type(err).__name__}: {err}")


class JSONHandler(BaseHTTPRequestHandler):
    def setup(self) -> None:
        self.request.do_handshake()                             # thiếu cert, CA lạ, sai EKU → lỗi ở đây
        super().setup()
        self.spiffe_id, self.thumb = peer_identity(self.request)

    def reply(self, code: int, body: dict, extra: dict | None = None) -> None:
        data = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Cache-Control", "no-store")
        for k, v in (extra or {}).items():
            self.send_header(k, v)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def read_form(self) -> dict:
        n = int(self.headers.get("Content-Length", 0))
        return {k: v[0] for k, v in parse_qs(self.rfile.read(n).decode()).items()}

    def log_message(self, *args) -> None:                       # thay log mặc định bằng audit log
        pass
