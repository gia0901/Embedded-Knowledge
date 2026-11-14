#!/usr/bin/env python3
"""mTLS server: authenticate client bằng certificate, rồi authorize theo SPIFFE ID. Chạy trong ~/netlab/m10."""
import socket
import ssl

PORT = 9444
POLICY = {  # SPIFFE ID → các path được phép. So nguyên chuỗi, không dùng "contains".
    "spiffe://netlab.local/ns/default/sa/svc-a": {"/token"},
    "spiffe://netlab.local/ns/default/sa/svc-b": {"/token", "/admin"},
}


def make_context() -> ssl.SSLContext:
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.load_cert_chain("pki/server-chain.crt", "pki/server.key")   # leaf + intermediate
    ctx.load_verify_locations("pki/root.crt")                       # tin client cert do CA nội bộ cấp
    ctx.verify_mode = ssl.CERT_REQUIRED                             # không có cert → handshake thất bại
    return ctx


def spiffe_id(cert: dict) -> str | None:
    """Lấy đúng một SPIFFE ID từ SAN; không có hoặc có nhiều hơn một thì coi như không có identity."""
    ids = [v for k, v in cert.get("subjectAltName", ()) if k == "URI" and v.startswith("spiffe://")]
    return ids[0] if len(ids) == 1 else None


def handle(tls: ssl.SSLSocket) -> None:
    ident = spiffe_id(tls.getpeercert())
    request_line = tls.recv(4096).decode(errors="replace").split("\r\n", 1)[0]
    method, path = (request_line.split() + ["", ""])[:2]
    if path in POLICY.get(ident, set()):                            # authorization
        status, body = "200 OK", f"xin chào {ident}, đây là {path}\n"
    else:
        status, body = "403 Forbidden", f"{ident} không được phép gọi {path}\n"
    print(f"   {ident} {method} {path} → {status}")
    data = body.encode()
    tls.sendall(f"HTTP/1.1 {status}\r\nContent-Length: {len(data)}\r\nConnection: close\r\n\r\n".encode() + data)


def main() -> None:
    ctx = make_context()
    with socket.create_server(("127.0.0.1", PORT)) as srv:
        print(f"mTLS server trên 127.0.0.1:{PORT}")
        while True:
            raw, (ip, port) = srv.accept()
            try:
                with ctx.wrap_socket(raw, server_side=True) as tls:   # handshake + verify client cert
                    print(f"{ip}:{port} {tls.version()} {tls.cipher()[0]}")
                    handle(tls)
            except ssl.SSLCertVerificationError as e:
                print(f"{ip}:{port} từ chối client cert: {e.verify_message}")
            except (ssl.SSLError, OSError) as e:
                print(f"{ip}:{port} handshake thất bại: {getattr(e, 'reason', None) or e}")


if __name__ == "__main__":
    main()
