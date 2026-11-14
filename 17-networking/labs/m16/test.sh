#!/usr/bin/env bash
# Kiểm thử chấp nhận cho capstone: cả ca dương lẫn ca âm. Chạy: bash test.sh
set -uo pipefail
TOKEN_URL=https://token.security.svc.cluster.local:8443/token
API=https://orders.apps.svc.cluster.local:8443/orders
MTLS="--cacert /trust/root.crt --cert /tls/tls.crt --key /tls/tls.key"
pass=0; fail=0

check() {  # check <mô tả> <mong đợi> <thực tế>
  if [[ "$3" == "$2" ]]; then echo "  ✔ $1"; pass=$((pass + 1))
  else echo "  ✘ $1: mong đợi '$2', nhận '$3'"; fail=$((fail + 1)); fi
}
ex() { local ns=$1 pod=$2; shift 2; kubectl -n "$ns" exec "$pod" -- "$@" 2>/dev/null; }
token() { ex "$1" "$2" sh -c "curl -s -m 5 $MTLS -d grant_type=client_credentials -d 'scope=$3' $TOKEN_URL"; }
call() { ex "$1" "$2" sh -c "curl -s -m 5 -o /dev/null -w '%{http_code}' $MTLS -X $3 -H 'Authorization: Bearer $4' $API"; }
field() { python3 -c 'import sys, json; print(json.load(sys.stdin).get(sys.argv[1], ""))' "$1" 2>/dev/null; }

echo "1. Luồng hợp lệ"
T_FE=$(token web frontend orders:read | field access_token)
check "frontend lấy được token orders:read" yes "$([[ -n $T_FE ]] && echo yes || echo no)"
check "frontend GET /orders" 200 "$(call web frontend GET "$T_FE")"
T_RP=$(token batch reporter 'orders:read orders:write' | field access_token)
check "reporter POST /orders" 200 "$(call batch reporter POST "$T_RP")"

echo "2. Authorization"
check "frontend POST /orders → 403" 403 "$(call web frontend POST "$T_FE")"
check "frontend xin orders:write → invalid_scope" invalid_scope "$(token web frontend orders:write | field error)"

echo "3. Token bị đánh cắp hoặc bị sửa"
check "reporter dùng token của frontend → 401" 401 "$(call batch reporter GET "$T_FE")"
check "token bị sửa chữ ký → 401" 401 "$(call web frontend GET "${T_FE:0:-10}AAAAAAAAAA")"
check "không có token → 401" 401 "$(call web frontend GET '')"

echo "4. Không có cert, hoặc không được phép ở tầng mạng"
check "gọi token server không có client cert → lỗi TLS" 000 \
  "$(ex web frontend curl -s -m 5 -o /dev/null -w '%{http_code}' --cacert /trust/root.crt -d grant_type=client_credentials $TOKEN_URL)"
check "attacker (default) → bị chặn ở tầng mạng (curl exit 28 = timeout)" 28 \
  "$(kubectl exec attacker -- curl -sk -m 3 -o /dev/null $TOKEN_URL >/dev/null 2>&1; echo $?)"

echo; echo "Kết quả: $pass đạt, $fail không đạt"
[[ $fail -eq 0 ]]
