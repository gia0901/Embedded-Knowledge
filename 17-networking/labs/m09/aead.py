#!/usr/bin/env python3
"""AES-GCM: tag, AAD, và vì sao nonce không bao giờ được lặp lại."""
import json
import os

from cryptography.exceptions import InvalidTag
from cryptography.hazmat.primitives.ciphers.aead import AESGCM

key = AESGCM.generate_key(bit_length=256)
aes = AESGCM(key)

# 1. Mã hoá token, AAD = header không mã hoá nhưng được xác thực
token = json.dumps({"sub": "svc-a", "scope": "secrets:read", "exp": 1794700000}).encode()
aad = b'{"kid":"k1","alg":"A256GCM"}'
nonce = os.urandom(12)                          # 96 bit ngẫu nhiên, mới cho mỗi message
ct = aes.encrypt(nonce, token, aad)
print(f"plaintext {len(token)} byte → ciphertext {len(ct)} byte (thêm 16 byte tag)")
print("giải mã:", aes.decrypt(nonce, ct, aad).decode())

# 2. Sửa 1 bit ciphertext, hoặc đổi AAD → bị phát hiện
bad = bytearray(ct)
bad[5] ^= 0x01
for label, c, a in [("lật 1 bit ciphertext", bytes(bad), aad), ("đổi kid trong AAD", ct, b'{"kid":"k2","alg":"A256GCM"}')]:
    try:
        aes.decrypt(nonce, c, a)
        print(f"{label}: ?! không bị phát hiện")
    except InvalidTag:
        print(f"{label}: InvalidTag → từ chối")

# 3. Nonce reuse: kẻ tấn công biết p1 (cấu trúc HTTP quen thuộc), có c1 và c2
fixed_nonce = b"\x00" * 12                      # LỖI: dùng lại nonce
p1 = b"GET /index.html HTTP/1.1\r\nHost: api.example.internal\r\n\r\n"
p2 = b"password=Tr0ub4dor&3;user=admin"
c1 = aes.encrypt(fixed_nonce, p1, None)
c2 = aes.encrypt(fixed_nonce, p2, None)
xor = bytes(a ^ b for a, b in zip(c1, c2))      # = p1 XOR p2 (keystream triệt tiêu nhau)
recovered = bytes(a ^ b for a, b in zip(xor, p1))[:len(p2)]
print("kẻ tấn công khôi phục được:", recovered.decode())
