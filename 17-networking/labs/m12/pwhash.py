#!/usr/bin/env python3
"""So sánh SHA-256 với Argon2id cho việc lưu password. Cần: sudo apt install -y python3-argon2"""
import hashlib
import os
import time

from argon2 import PasswordHasher
from argon2.exceptions import VerifyMismatchError


def rate(fn, seconds: float = 1.0) -> float:
    n, t0 = 0, time.perf_counter()
    while time.perf_counter() - t0 < seconds:
        fn()
        n += 1
    return n / (time.perf_counter() - t0)


pw, salt = b"correct horse battery staple", os.urandom(16)
ph = PasswordHasher(time_cost=2, memory_cost=19456, parallelism=1)    # mức tối thiểu OWASP: 19 MiB
r_sha = rate(lambda: hashlib.sha256(salt + pw).digest())
r_argon = rate(lambda: ph.hash(pw))
print(f"SHA-256(salt+pw): {r_sha:12,.0f} lần/giây (1 core, Python; GPU nhanh hơn nhiều nghìn lần)")
print(f"Argon2id        : {r_argon:12,.1f} lần/giây → chậm hơn {r_sha / r_argon:,.0f} lần")

stored = ph.hash("hunter2")
print("Chuỗi lưu trong DB:", stored)
for guess in ("hunter1", "hunter2"):
    try:
        ph.verify(stored, guess)
        print(f"  verify {guess!r}: OK")
    except VerifyMismatchError:
        print(f"  verify {guess!r}: sai password")
print("  cùng password, hash khác nhau (nhờ salt):", ph.hash("hunter2") != ph.hash("hunter2"))
# Tăng tham số sau này: hash cũ vẫn verify được; lúc user login thành công thì hash lại bằng tham số mới
ph_new = PasswordHasher(time_cost=3, memory_cost=65536, parallelism=1)
print("  hash cũ cần rehash theo tham số mới?", ph_new.check_needs_rehash(stored))
