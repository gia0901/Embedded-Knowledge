#!/usr/bin/env python3
"""Mini NRF: NFRegister, NFDiscover, AccessToken (client credentials). Rút gọn từ TS 29.510, chỉ để học.
Thực tế NRF chạy sau mTLS và so nfInstanceId với identity trong client certificate."""
import json
import time
import uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlparse

import jwt
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec

NRF_ID = str(uuid.uuid4())
KEY = ec.generate_private_key(ec.SECP256R1())
with open("nrf_pub.pem", "wb") as f:             # producer dùng public key này để verify token
    f.write(KEY.public_key().public_bytes(serialization.Encoding.PEM,
                                          serialization.PublicFormat.SubjectPublicKeyInfo))
PROFILES: dict[str, dict] = {}                     # nfInstanceId → NFProfile


def services(profile: dict) -> set[str]:
    return {s["serviceName"] for s in profile.get("nfServices", [])}


def allows(producer: dict, requester_type: str) -> bool:
    return requester_type in producer.get("allowedNfTypes", [requester_type])   # không khai báo = cho tất cả


class NRF(BaseHTTPRequestHandler):
    def reply(self, code: int, body: dict, ctype: str = "application/json", extra: dict | None = None) -> None:
        data = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        for k, v in (extra or {}).items():
            self.send_header(k, v)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def problem(self, code: int, cause: str, detail: str) -> None:
        self.reply(code, {"status": code, "cause": cause, "detail": detail}, "application/problem+json")

    def body(self) -> bytes:
        return self.rfile.read(int(self.headers.get("Content-Length", 0)))

    def do_PUT(self) -> None:                                         # NFRegister
        parts = urlparse(self.path).path.strip("/").split("/")
        if parts[:3] != ["nnrf-nfm", "v1", "nf-instances"] or len(parts) != 4:
            return self.problem(404, "RESOURCE_URI_STRUCTURE_NOT_FOUND", self.path)
        try:
            prof = json.loads(self.body())
        except ValueError:
            return self.problem(400, "INVALID_MSG_FORMAT", "JSON hỏng")
        if prof.get("nfInstanceId") != parts[3] or not prof.get("nfType"):
            return self.problem(400, "MANDATORY_IE_INCORRECT", "nfInstanceId / nfType")
        prof["nfStatus"] = "REGISTERED"
        PROFILES[parts[3]] = prof
        print(f"[register] {prof['nfType']:<4} {parts[3][:8]} services={sorted(services(prof))}")
        self.reply(201, {**prof, "heartBeatTimer": 60}, extra={"Location": self.path})

    def do_GET(self) -> None:                                         # NFDiscover
        u = urlparse(self.path)
        if u.path != "/nnrf-disc/v1/nf-instances":
            return self.problem(404, "RESOURCE_URI_STRUCTURE_NOT_FOUND", u.path)
        q = {k: v[0] for k, v in parse_qs(u.query).items()}
        target, requester = q.get("target-nf-type"), q.get("requester-nf-type")
        if not target or not requester:
            return self.problem(400, "MANDATORY_QUERY_PARAM_MISSING", "target-nf-type, requester-nf-type")
        want = set(q["service-names"].split(",")) if "service-names" in q else set()
        found = [p for p in PROFILES.values()
                 if p["nfType"] == target and p["nfStatus"] == "REGISTERED" and allows(p, requester) and want <= services(p)]
        print(f"[discover] {requester} → {target} {sorted(want)}: {len(found)} kết quả")
        self.reply(200, {"validityPeriod": 60, "nfInstances": found})

    def do_POST(self) -> None:                                        # AccessToken
        if urlparse(self.path).path != "/oauth2/token":
            return self.problem(404, "RESOURCE_URI_STRUCTURE_NOT_FOUND", self.path)
        f = {k: v[0] for k, v in parse_qs(self.body().decode()).items()}
        if f.get("grant_type") != "client_credentials":
            return self.reply(400, {"error": "unsupported_grant_type"})
        consumer = PROFILES.get(f.get("nfInstanceId", ""))
        if not consumer or consumer["nfType"] != f.get("nfType"):   # NF phải đăng ký đúng loại đã khai
            return self.reply(400, {"error": "invalid_client"})
        scopes = set(f.get("scope", "").split())
        ok = [p for p in PROFILES.values()
              if p["nfType"] == f.get("targetNfType") and allows(p, consumer["nfType"]) and scopes and scopes <= services(p)]
        if not ok:
            print(f"[token] từ chối {consumer['nfType']} → {f.get('targetNfType')} scope={sorted(scopes)}")
            return self.reply(400, {"error": "invalid_scope"})
        now = int(time.time())
        claims = {"iss": NRF_ID, "sub": consumer["nfInstanceId"], "aud": f["targetNfType"],
                  "scope": " ".join(sorted(scopes)), "iat": now, "exp": now + 300}
        token = jwt.encode(claims, KEY, algorithm="ES256")
        print(f"[token] cấp cho {consumer['nfType']} → {claims['aud']} scope={claims['scope']}")
        self.reply(200, {"access_token": token, "token_type": "Bearer", "expires_in": 300, "scope": claims["scope"]})

    def log_message(self, *args) -> None:
        pass


if __name__ == "__main__":
    print(f"NRF {NRF_ID} trên http://127.0.0.1:8000")
    ThreadingHTTPServer(("127.0.0.1", 8000), NRF).serve_forever()
