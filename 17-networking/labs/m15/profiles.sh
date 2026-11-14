#!/usr/bin/env bash
# Sinh udm.json, amf.json, nef.json với nfInstanceId ngẫu nhiên
set -euo pipefail
mk() {  # mk <file> <nfType> <ip> <allowedNfTypes JSON> <service...>
  local file=$1 type=$2 ip=$3 allowed=$4; shift 4
  local svcs; svcs=$(printf '{"serviceInstanceId":"%s","serviceName":"%s","scheme":"https","nfServiceStatus":"REGISTERED","versions":[{"apiVersionInUri":"v1","apiFullVersion":"1.0.0"}]},' $(for s in "$@"; do echo "$s $s"; done))
  printf '{"nfInstanceId":"%s","nfType":"%s","nfStatus":"REGISTERED","ipv4Addresses":["%s"],"allowedNfTypes":%s,"nfServices":[%s]}\n' \
    "$(cat /proc/sys/kernel/random/uuid)" "$type" "$ip" "$allowed" "${svcs%,}" > "$file"
}
mk udm.json UDM 10.0.5.10 '["AMF","AUSF"]' nudm-uecm nudm-ueau nudm-sdm
mk amf.json AMF 10.0.5.20 '["SMF","AUSF"]' namf-comm
mk nef.json NEF 10.0.5.30 '["AF"]' nnef-eventexposure
echo "OK: udm.json amf.json nef.json"
