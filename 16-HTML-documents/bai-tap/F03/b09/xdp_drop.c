// xdp_drop.c: XDP bỏ datagram UDP tới cổng 9000 ngay trong driver, trước khi kernel tạo skb.
// Build: clang -O2 -g -target bpf -I/usr/include/$(uname -m)-linux-gnu -c xdp_drop.c -o xdp_drop.o
//        thêm -DBO_KIEM_BIEN để bỏ một lần kiểm biên và xem verifier từ chối
// Gắn:   ip link set dev IF xdp obj xdp_drop.o sec xdp      Gỡ: ip link set dev IF xdp off
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

SEC("xdp")
int drop_udp9000(struct xdp_md* ctx) {
    void* data = (void*)(long)ctx->data;
    void* end = (void*)(long)ctx->data_end;
    struct ethhdr* eth = data;
    if ((void*)(eth + 1) > end || eth->h_proto != bpf_htons(ETH_P_IP)) return XDP_PASS;
    struct iphdr* ip = (void*)(eth + 1);                  // verifier bắt kiểm biên trước mỗi lần đọc
    if ((void*)(ip + 1) > end || ip->protocol != IPPROTO_UDP) return XDP_PASS;
    struct udphdr* udp = (void*)ip + ip->ihl * 4;
#ifndef BO_KIEM_BIEN
    if ((void*)(udp + 1) > end) return XDP_PASS;
#endif
    return udp->dest == bpf_htons(9000) ? XDP_DROP : XDP_PASS;
}
char LICENSE[] SEC("license") = "GPL";
