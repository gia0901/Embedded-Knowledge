from math import comb
import random

LINK, PER_USER, P_ACTIVE = 1_000_000, 100_000, 0.1
CAP = LINK // PER_USER          # tối đa 10 user cùng lúc

def p_overload(n: int, p: float = P_ACTIVE, cap: int = CAP) -> float:
    """Xác suất có hơn `cap` user hoạt động cùng lúc trong n user."""
    return sum(comb(n, k) * p**k * (1 - p)**(n - k) for k in range(cap + 1, n + 1))

def simulate(n: int, trials: int = 200_000, p: float = P_ACTIVE, cap: int = CAP) -> float:
    over = 0
    for _ in range(trials):
        active = sum(random.random() < p for _ in range(n))
        over += active > cap
    return over / trials

print(f"Circuit switching phục vụ tối đa {CAP} user.\n")
print(f"{'users':>6} {'P(overload) công thức':>22} {'mô phỏng':>10}")
for n in (20, 35, 50, 70):
    print(f"{n:>6} {p_overload(n):>22.6f} {simulate(n, 50_000):>10.6f}")

# Bao nhiêu user nếu chấp nhận overload ≤ 1% thời gian?
n = CAP
while p_overload(n + 1) <= 0.01:
    n += 1
print(f"\nChấp nhận overload ≤ 1%: tối đa {n} user (gấp {n / CAP:.1f} lần circuit switching)")
