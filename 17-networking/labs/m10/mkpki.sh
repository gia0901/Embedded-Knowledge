#!/usr/bin/env bash
# PKI nội bộ 3 tầng: root CA → intermediate CA → leaf (server + client). Chạy: bash mkpki.sh
set -euo pipefail
mkdir -p pki && cd pki
newkey() { openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:P-256 -out "$1"; }

# 1. Root CA: self-signed, chỉ để ký intermediate (thực tế: giữ offline)
newkey root.key
openssl req -x509 -new -key root.key -sha256 -days 3650 -subj "/O=NetLab/CN=NetLab Root CA" \
  -addext "basicConstraints=critical,CA:TRUE" \
  -addext "keyUsage=critical,keyCertSign,cRLSign" -out root.crt

# 2. Intermediate CA: ký bởi root; pathlen:0 = không được tạo thêm CA bên dưới
newkey inter.key
openssl req -new -key inter.key -subj "/O=NetLab/CN=NetLab Issuing CA 1" -out inter.csr
openssl x509 -req -in inter.csr -CA root.crt -CAkey root.key -CAcreateserial -days 1825 -sha256 \
  -extfile <(printf '%s\n' "basicConstraints=critical,CA:TRUE,pathlen:0" \
                           "keyUsage=critical,keyCertSign,cRLSign" \
                           "subjectKeyIdentifier=hash" "authorityKeyIdentifier=keyid") \
  -out inter.crt

# 3. Server cert cho service auth (namespace security)
newkey server.key
openssl req -new -key server.key -subj "/CN=auth.security.svc.cluster.local" -out server.csr
openssl x509 -req -in server.csr -CA inter.crt -CAkey inter.key -CAcreateserial -days 30 -sha256 \
  -extfile <(printf '%s\n' "basicConstraints=critical,CA:FALSE" "keyUsage=critical,digitalSignature" \
                           "extendedKeyUsage=serverAuth" \
                           "subjectAltName=DNS:auth.security.svc.cluster.local,DNS:auth.security,DNS:localhost,IP:127.0.0.1") \
  -out server.crt

# 4. Client cert cho svc-a (dùng cho mTLS ở module 11); identity là SPIFFE ID trong SAN
newkey client.key
openssl req -new -key client.key -subj "/CN=svc-a" -out client.csr
openssl x509 -req -in client.csr -CA inter.crt -CAkey inter.key -CAcreateserial -days 30 -sha256 \
  -extfile <(printf '%s\n' "basicConstraints=critical,CA:FALSE" "keyUsage=critical,digitalSignature" \
                           "extendedKeyUsage=clientAuth" \
                           "subjectAltName=URI:spiffe://netlab.local/ns/default/sa/svc-a") \
  -out client.crt

# 5. File chain để gửi đi: leaf + intermediate (không gửi root)
cat server.crt inter.crt > server-chain.crt
cat client.crt inter.crt > client-chain.crt
chmod 600 ./*.key
openssl verify -CAfile root.crt -untrusted inter.crt server.crt client.crt
