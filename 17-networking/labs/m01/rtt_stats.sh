#!/usr/bin/env bash
# Dùng: ./rtt_stats.sh <host> [số_lần]
host=${1:?Cần tên host, ví dụ: ./rtt_stats.sh 1.1.1.1}
n=${2:-20}
ping -c "$n" -i 0.2 "$host" | awk -v n="$n" -v host="$host" -F'time=' '
  /time=/ {
    split($2, a, " "); r = a[1] + 0
    s += r; c++
    if (c == 1 || r < mn) mn = r
    if (r > mx) mx = r
    if (c > 1) { d = r - prev; j += (d < 0 ? -d : d) }
    prev = r
  }
  END {
    if (!c) { print host ": không nhận được reply nào"; exit 1 }
    printf "%-16s sent/received %d/%d  loss %.0f%%\n", host, n, c, 100 * (n - c) / n
    printf "%-16s RTT min/avg/max = %.2f/%.2f/%.2f ms  jitter = %.2f ms\n",
           "", mn, s / c, mx, (c > 1 ? j / (c - 1) : 0)
  }'
