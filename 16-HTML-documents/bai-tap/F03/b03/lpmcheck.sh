#!/bin/bash
# So lpm với kernel: tạo netns rt có N route ngẫu nhiên, hỏi "ip route get" cho từng địa chỉ.
# Chạy: sudo bash lpmcheck.sh N [--first]
set -e
N=${1:-2000}; MODE=$2
ip netns del rt 2>/dev/null || true
ip netns add rt
ip -n rt link add r0 type veth peer name r1
ip -n rt addr add 192.168.0.254/24 dev r0
ip -n rt link set r0 up; ip -n rt link set r1 up
ip -n rt route add default via 192.168.0.1
./lpm gen "$N" 7 | ip -n rt -batch -
ip -n rt route show > routes.txt
./lpm addrs 1000 3 > addrs.txt
./lpm $MODE routes.txt < addrs.txt > mine.txt
while read a; do
  echo "$a $(ip -n rt route get "$a" | awk '{for(i=1;i<NF;i++) if($i=="via"){print $(i+1); exit}}')"
done < addrs.txt > kernel.txt
echo "route trong bang: $(grep -c via routes.txt)"
echo "khac kernel: $(diff mine.txt kernel.txt | grep -c '^<' || true) / $(wc -l < addrs.txt)"
ip netns del rt
