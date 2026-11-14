#!/usr/bin/env python3
"""In path, method và OAuth2 scope từ OpenAPI của 3GPP. Dùng: python3 sbi_paths.py *.yaml"""
import sys

import yaml

METHODS = ("get", "put", "post", "patch", "delete")

for path in sys.argv[1:]:
    with open(path) as f:
        api = yaml.safe_load(f)
    info = api.get("info", {})
    print(f"== {info.get('title')} {info.get('version')}")
    print(f"   server: {api.get('servers', [{}])[0].get('url')}")
    for p, ops in api.get("paths", {}).items():
        for m in METHODS:
            if m in ops:
                print(f"   {m.upper():<6} {p}")
    for name, scheme in api.get("components", {}).get("securitySchemes", {}).items():
        flow = scheme.get("flows", {}).get("clientCredentials", {})
        print(f"   OAuth2 ({name}): tokenUrl={flow.get('tokenUrl')} scopes={list(flow.get('scopes', {}))}")
    req = api.get("components", {}).get("schemas", {}).get("AccessTokenReq")
    if req:
        print(f"   AccessTokenReq: required={req.get('required')}")
        print(f"                   fields={list(req.get('properties', {}))[:10]} …")
