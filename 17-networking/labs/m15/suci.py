#!/usr/bin/env python3
"""SUCI Profile A (TS 33.501 Annex C): UE che MSIN bằng ECIES X25519; SIDF trong UDM giải ra SUPI. Chỉ để học."""
import hashlib
import hmac

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric.x25519 import X25519PrivateKey, X25519PublicKey
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.kdf.x963kdf import X963KDF
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat


def raw(pub: X25519PublicKey) -> bytes:
    return pub.public_bytes(Encoding.Raw, PublicFormat.Raw)


def derive(shared: bytes, eph_pub: bytes) -> tuple[bytes, bytes, bytes]:
    k = X963KDF(algorithm=hashes.SHA256(), length=64, sharedinfo=eph_pub).derive(shared)
    return k[:16], k[16:32], k[32:]                         # AES key, ICB, MAC key


def bcd(digits: str) -> bytes:                              # "0123456789" → 10 32 54 76 98
    digits += "F" * (len(digits) % 2)
    return bytes(int(digits[i + 1], 16) << 4 | int(digits[i], 16) for i in range(0, len(digits), 2))


def unbcd(b: bytes) -> str:
    return "".join(f"{x & 0xF:X}{x >> 4:X}" for x in b).rstrip("F")


def conceal(msin: str, hn_pub: X25519PublicKey) -> bytes:   # chạy trong USIM / UE
    eph = X25519PrivateKey.generate()                       # mới cho MỖI lần che
    eph_pub = raw(eph.public_key())
    key, icb, mac_key = derive(eph.exchange(hn_pub), eph_pub)
    e = Cipher(algorithms.AES(key), modes.CTR(icb)).encryptor()
    ct = e.update(bcd(msin)) + e.finalize()
    return eph_pub + ct + hmac.new(mac_key, ct, hashlib.sha256).digest()[:8]


def deconceal(out: bytes, hn_priv: X25519PrivateKey) -> str:   # chạy trong SIDF (UDM), key nên nằm trong HSM
    eph_pub, ct, tag = out[:32], out[32:-8], out[-8:]
    key, icb, mac_key = derive(hn_priv.exchange(X25519PublicKey.from_public_bytes(eph_pub)), eph_pub)
    if not hmac.compare_digest(hmac.new(mac_key, ct, hashlib.sha256).digest()[:8], tag):
        raise ValueError("MAC sai: SUCI đã bị sửa")
    d = Cipher(algorithms.AES(key), modes.CTR(icb)).decryptor()
    return unbcd(d.update(ct) + d.finalize())


if __name__ == "__main__":
    MCC, MNC, MSIN = "452", "04", "0123456789"
    hn_priv = X25519PrivateKey.generate()                   # home network giữ private key
    hn_pub = hn_priv.public_key()                           # public key được ghi vào USIM khi phát hành
    print(f"SUPI: imsi-{MCC}{MNC}{MSIN}")
    for i in (1, 2):
        out = conceal(MSIN, hn_pub)
        print(f"SUCI #{i}: suci-0-{MCC}-{MNC}-0000-1-1-{out.hex()[:24]}…  ({len(out)} byte)")
        print(f"   SIDF giải ra: imsi-{MCC}{MNC}{deconceal(out, hn_priv)}")
    bad = bytearray(out)
    bad[33] ^= 0x01                                         # lật 1 bit trong ciphertext
    try:
        deconceal(bytes(bad), hn_priv)
    except ValueError as e:
        print("SUCI bị sửa 1 bit:", e)
