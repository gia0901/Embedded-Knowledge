#!/usr/bin/env bash
# Dùng: ./httptime.sh URL
url=${1:?Dùng: ./httptime.sh URL}
fmt='%{time_namelookup} %{time_connect} %{time_appconnect} %{time_starttransfer} %{time_total} %{http_version} %{http_code}\n'
curl -s -o /dev/null -w "$fmt" "$url" | awk -v url="$url" '{
  dns   = $1 * 1000
  tcp   = ($2 - $1) * 1000
  tls   = ($3 > 0) ? ($3 - $2) * 1000 : 0          # http:// thì time_appconnect = 0
  ready = ($3 > 0) ? $3 : $2                       # lúc bắt đầu gửi request
  ttfb  = ($4 - ready) * 1000
  dl    = ($5 - $4) * 1000
  printf "%s  (HTTP/%s, status %s)\n", url, $6, $7
  printf "  DNS lookup     %7.1f ms\n", dns
  printf "  TCP connect    %7.1f ms   ≈ 1 RTT\n", tcp
  printf "  TLS handshake  %7.1f ms   ≈ 1 RTT với TLS 1.3, 2 RTT với TLS 1.2\n", tls
  printf "  Request → TTFB %7.1f ms   = 1 RTT + thời gian server xử lý\n", ttfb
  printf "  Download       %7.1f ms\n", dl
  printf "  TOTAL          %7.1f ms\n", $5 * 1000
}'
