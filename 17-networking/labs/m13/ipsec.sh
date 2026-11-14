#!/usr/bin/env bash
# ESP transport mode giữa 2 namespace bằng key tĩnh (không IKE; chỉ để học). sudo bash ipsec.sh | sudo bash ipsec.sh down
set -euo pipefail
if [[ ${1:-} == down ]]; then ip netns del left 2>/dev/null || true; ip netns del right 2>/dev/null || true; exit 0; fi
L=10.13.9.1; R=10.13.9.2
ip netns add left; ip netns add right
ip link add vl netns left type veth peer name vr netns right
ip -n left addr add $L/24 dev vl;  ip -n left link set vl up;  ip -n left link set lo up
ip -n right addr add $R/24 dev vr; ip -n right link set vr up; ip -n right link set lo up

K1=0x$(openssl rand -hex 20)    # chiều left → right: AES-128 key + 4 byte salt
K2=0x$(openssl rand -hex 20)    # chiều right → left
ALG="rfc4106(gcm(aes))"
for ns in left right; do        # SAD: cả hai bên đều có cả hai SA
  ip -n $ns xfrm state add src $L dst $R proto esp spi 0x1001 reqid 1 mode transport aead "$ALG" "$K1" 128
  ip -n $ns xfrm state add src $R dst $L proto esp spi 0x2002 reqid 2 mode transport aead "$ALG" "$K2" 128
done
# SPD: traffic giữa L và R bắt buộc đi qua ESP, cả chiều ra lẫn chiều vào
ip -n left  xfrm policy add src $L dst $R dir out tmpl proto esp reqid 1 mode transport
ip -n left  xfrm policy add src $R dst $L dir in  tmpl proto esp reqid 2 mode transport
ip -n right xfrm policy add src $R dst $L dir out tmpl proto esp reqid 2 mode transport
ip -n right xfrm policy add src $L dst $R dir in  tmpl proto esp reqid 1 mode transport
echo "ESP sẵn sàng: $L <-> $R"
