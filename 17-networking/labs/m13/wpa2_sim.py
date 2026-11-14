#!/usr/bin/env python3
"""Mô phỏng phần key của WPA2-PSK 4-way handshake và dò passphrase offline.
Chỉ chạy trên dữ liệu tự tạo; bắt handshake của mạng không phải của bạn là vi phạm pháp luật."""
import hashlib
import hmac
import itertools
import os
import time


def pmk(passphrase: str, ssid: str) -> bytes:
    return hashlib.pbkdf2_hmac("sha1", passphrase.encode(), ssid.encode(), 4096, 32)


def prf512(key: bytes, label: bytes, data: bytes) -> bytes:
    return b"".join(hmac.new(key, label + b"\x00" + data + bytes([i]), hashlib.sha1).digest() for i in range(4))[:64]


def ptk(pmk_: bytes, aa: bytes, spa: bytes, anonce: bytes, snonce: bytes) -> bytes:
    data = min(aa, spa) + max(aa, spa) + min(anonce, snonce) + max(anonce, snonce)
    return prf512(pmk_, b"Pairwise key expansion", data)


def mic(kck: bytes, frame: bytes) -> bytes:
    return hmac.new(kck, frame, hashlib.sha1).digest()[:16]


def make_handshake(passphrase: str, ssid: str) -> dict:
    """Phía AP + client thật. Trả về đúng những gì kẻ nghe lén thấy trên sóng."""
    aa, spa = b"\x02" + os.urandom(5), b"\x02" + os.urandom(5)      # MAC của AP và client
    anonce, snonce = os.urandom(32), os.urandom(32)                 # Msg 1, Msg 2
    frame = b"\x01\x03\x00\x75" + snonce + b"\x00" * 16             # Msg 2 với field MIC để 0 khi tính
    kck = ptk(pmk(passphrase, ssid), aa, spa, anonce, snonce)[:16]
    return {"ssid": ssid, "aa": aa, "spa": spa, "anonce": anonce, "snonce": snonce,
            "frame": frame, "mic": mic(kck, frame)}


def crack(cap: dict, candidates) -> tuple[str | None, int, float]:
    t0, n = time.perf_counter(), 0
    for word in candidates:
        n += 1
        kck = ptk(pmk(word, cap["ssid"]), cap["aa"], cap["spa"], cap["anonce"], cap["snonce"])[:16]
        if hmac.compare_digest(mic(kck, cap["frame"]), cap["mic"]):
            return word, n, time.perf_counter() - t0
    return None, n, time.perf_counter() - t0


if __name__ == "__main__":
    cap = make_handshake("sunshine2024", "NetLab-WiFi")
    print(f"Bắt được: SSID={cap['ssid']} ANonce={cap['anonce'][:4].hex()}… MIC={cap['mic'].hex()[:16]}…")
    words = ["password", "netlab", "sunshine", "dragon", "letmein", "qwerty", "admin", "welcome", "iloveyou", "monkey"]
    wordlist = (w + suffix for w, suffix in itertools.product(words, ["", "123", "!", *map(str, range(2015, 2027))]))
    found, tries, secs = crack(cap, wordlist)
    rate = tries / secs
    print(f"Tìm thấy: {found!r} sau {tries} lần thử, {secs:.2f} giây (~{rate:,.0f} lần/giây trên 1 core Python)")
    for label, space in (("8 ký tự a-z0-9", 36 ** 8), ("20 ký tự ngẫu nhiên a-zA-Z0-9", 62 ** 20)):
        for who, r in (("CPU này", rate), ("GPU ~1 triệu/giây", 1e6)):
            years = space / r / 3600 / 24 / 365
            print(f"  {label:<30} {who:<18} ≈ {years:.3g} năm (thử hết)")
