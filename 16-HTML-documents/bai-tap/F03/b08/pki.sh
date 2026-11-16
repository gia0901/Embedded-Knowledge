#!/bin/bash
# PKI nhỏ cho lab: một CA, chứng chỉ server backend.lab, chứng chỉ thiết bị dev-007. Khoá EC P-256.
# Chạy: bash pki.sh   (tạo trong thư mục pki/)
set -e
mkdir -p pki && cd pki
openssl req -x509 -new -nodes -newkey ec -pkeyopt ec_paramgen_curve:P-256 -keyout ca.key -out ca.crt \
  -days 3650 -subj "/CN=Lab CA" 2>/dev/null
mk() {   # mk TÊN SUBJECT EXT
  openssl req -new -nodes -newkey ec -pkeyopt ec_paramgen_curve:P-256 -keyout $1.key -out $1.csr -subj "$2" 2>/dev/null
  openssl x509 -req -in $1.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out $1.crt -days 365 -extfile <(printf "$3") 2>/dev/null
  rm $1.csr
}
mk srv "/CN=backend.lab" "subjectAltName=DNS:backend.lab\nextendedKeyUsage=serverAuth"
mk dev "/CN=dev-007"     "extendedKeyUsage=clientAuth"
ls
