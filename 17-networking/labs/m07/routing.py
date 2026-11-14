#!/usr/bin/env python3
"""Link-state (Dijkstra) và distance-vector (Bellman-Ford) trên các đồ thị trong Kurose & Ross Ch.5."""
import heapq

INF = float("inf")
FIG_5_3 = {("u", "v"): 2, ("u", "x"): 1, ("u", "w"): 5, ("v", "x"): 2, ("v", "w"): 3,
           ("x", "w"): 3, ("x", "y"): 1, ("w", "y"): 1, ("w", "z"): 5, ("y", "z"): 2}


def graph(links: dict) -> dict:
    g = {}
    for (a, b), c in links.items():
        g.setdefault(a, {})[b] = c
        g.setdefault(b, {})[a] = c
    return g


def dijkstra(g: dict, src: str):
    dist, prev, done = {src: 0}, {}, set()
    pq = [(0, src)]
    while pq:
        d, n = heapq.heappop(pq)
        if n in done:
            continue
        done.add(n)                                   # n vào N': cost tới n đã chắc chắn
        for m, c in g[n].items():
            if d + c < dist.get(m, INF):
                dist[m], prev[m] = d + c, n
                heapq.heappush(pq, (d + c, m))
    return dist, prev


def forwarding_table(prev: dict, src: str) -> dict:
    table = {}
    for dst in prev:
        hop = dst
        while prev[hop] != src:                       # đi ngược tới node ngay sau source
            hop = prev[hop]
        table[dst] = hop
    return table


def dv_round(g: dict, D: dict) -> dict:
    """Một round đồng bộ: mọi node x tính d_x(y) = min_v c(x,v) + d_v(y) từ vector cũ của neighbor."""
    return {x: {y: 0 if x == y else min(g[x][v] + D[v][y] for v in g[x]) for y in g} for x in g}


def distance_vector(g: dict, D: dict | None = None, max_rounds: int = 500):
    if D is None:                                     # ban đầu chỉ biết cost tới neighbor
        D = {x: {y: 0 if x == y else g[x].get(y, INF) for y in g} for x in g}
    for r in range(1, max_rounds + 1):
        new = dv_round(g, D)
        if new == D:
            return D, r - 1                           # số round còn có thay đổi
        D = new
    return D, max_rounds


if __name__ == "__main__":
    g = graph(FIG_5_3)
    dist, prev = dijkstra(g, "u")
    print("Dijkstra từ u:", dict(sorted(dist.items())))
    print("Forwarding table của u:", dict(sorted(forwarding_table(prev, "u").items())))

    D, rounds = distance_vector(g)
    print(f"DV hội tụ sau {rounds} round; D_u =", dict(sorted(D["u"].items())))
    assert D["u"] == dist, "DV và Dijkstra phải cho cùng kết quả"

    g3 = graph({("x", "y"): 4, ("y", "z"): 1, ("x", "z"): 50})
    D3, _ = distance_vector(g3)
    print(f"\nFigure 5.7 trước: D_y(x) = {D3['y']['x']}, D_z(x) = {D3['z']['x']}")
    g3["x"]["y"] = g3["y"]["x"] = 60                   # link x–y tăng cost đột ngột
    D3, rounds = distance_vector(g3, D3)
    print(f"Sau khi x–y = 60: {rounds} round mới hội tụ, D_y(x) = {D3['y']['x']}, D_z(x) = {D3['z']['x']}")
