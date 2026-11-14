#!/usr/bin/env python3
"""Milenage (TS 35.206) + 5G-AKA (TS 33.501 Annex A). Mô phỏng ARPF/UDM, AUSF, SEAF và USIM. Chỉ để học."""
import hashlib
import hmac
import os

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


def aes(k: bytes, block: bytes) -> bytes:
    e = Cipher(algorithms.AES(k), modes.ECB()).encryptor()
    return e.update(block) + e.finalize()


def xor(a: bytes, b: bytes) -> bytes:
    return bytes(x ^ y for x, y in zip(a, b))


def rot(x: bytes, nbytes: int) -> bytes:
    return x[nbytes:] + x[:nbytes]


def opc_of(k: bytes, op: bytes) -> bytes:
    return xor(aes(k, op), op)


def milenage(k: bytes, opc: bytes, rand: bytes, sqn: bytes, amf: bytes) -> dict:
    temp = aes(k, xor(rand, opc))
    out1 = xor(aes(k, xor(temp, rot(xor(sqn + amf + sqn + amf, opc), 8))), opc)    # c1 = 0
    t = xor(temp, opc)
    out = lambda r, c: xor(aes(k, xor(rot(t, r), bytes(15) + bytes([c]))), opc)
    o2, o3, o4 = out(0, 1), out(4, 2), out(8, 4)
    return {"mac_a": out1[:8], "res": o2[8:], "ak": o2[:6], "ck": o3, "ik": o4}


def kdf(key: bytes, fc: int, *params: bytes) -> bytes:                       # TS 33.220 Annex B
    s = bytes([fc]) + b"".join(p + len(p).to_bytes(2, "big") for p in params)
    return hmac.new(key, s, hashlib.sha256).digest()


class AuthError(Exception):
    pass


def udm_generate(k, opc, sqn, amf, rand, snn) -> dict:                       # ARPF trong UDM
    m = milenage(k, opc, rand, sqn, amf)
    sqn_ak = xor(sqn, m["ak"])
    ckik = m["ck"] + m["ik"]
    return {"rand": rand, "autn": sqn_ak + amf + m["mac_a"],
            "xres_star": kdf(ckik, 0x6B, snn, rand, m["res"])[16:],
            "kausf": kdf(ckik, 0x6A, snn, sqn_ak)}


def ausf_prepare(he_av: dict, snn: bytes) -> tuple[dict, bytes]:
    hxres = hashlib.sha256(he_av["rand"] + he_av["xres_star"]).digest()[16:]
    return {"rand": he_av["rand"], "autn": he_av["autn"], "hxres_star": hxres}, kdf(he_av["kausf"], 0x6C, snn)


class USIM:
    def __init__(self, k: bytes, opc: bytes):
        self.k, self.opc, self.sqn_ms = k, opc, -1                              # SQN cao nhất đã chấp nhận

    def authenticate(self, rand: bytes, autn: bytes, snn: bytes) -> tuple[bytes, bytes]:
        ak = milenage(self.k, self.opc, rand, bytes(6), bytes(2))["ak"]        # f5 chỉ phụ thuộc K, OPc, RAND
        sqn, amf, mac = xor(autn[:6], ak), autn[6:8], autn[8:]
        m = milenage(self.k, self.opc, rand, sqn, amf)
        if not hmac.compare_digest(m["mac_a"], mac):
            raise AuthError("MAC failure: mạng không có K của SIM này")
        if int.from_bytes(sqn, "big") <= self.sqn_ms:
            raise AuthError("Synch failure: SQN cũ, có thể là replay")
        self.sqn_ms = int.from_bytes(sqn, "big")
        ckik = m["ck"] + m["ik"]
        res_star = kdf(ckik, 0x6B, snn, rand, m["res"])[16:]
        kseaf = kdf(kdf(ckik, 0x6A, snn, xor(sqn, ak)), 0x6C, snn)
        return res_star, kseaf


def run(name, usim, net_k, opc, sqn, snn_ue, snn_net, rand=None, replay=None):
    print(f"\n# {name}")
    try:
        he_av = replay or udm_generate(net_k, opc, sqn, bytes.fromhex("b9b9"), rand or os.urandom(16), snn_net)
        se_av, kseaf_net = ausf_prepare(he_av, snn_net)
        res_star, kseaf_ue = usim.authenticate(se_av["rand"], se_av["autn"], snn_ue)
        if hashlib.sha256(se_av["rand"] + res_star).digest()[16:] != se_av["hxres_star"]:
            raise AuthError("SEAF: HXRES* không khớp")
        if not hmac.compare_digest(res_star, he_av["xres_star"]):
            raise AuthError("AUSF: RES* ≠ XRES*")
        print(f"  ✔ thành công, KSEAF giống nhau: {kseaf_ue == kseaf_net}  KSEAF={kseaf_ue.hex()[:16]}…")
        return he_av
    except AuthError as e:
        print(f"  ✘ {e}")


if __name__ == "__main__":
    K = bytes.fromhex("465b5ce8b199b49faa5f0a2ee238a6bc")
    OPC = opc_of(K, bytes.fromhex("cdc202d5123e20f62b6d676ac72cb318"))
    RAND, SQN = bytes.fromhex("23553cbe9637a89d218ae64dae47bf35"), bytes.fromhex("ff9bb4d0b607")
    m = milenage(K, OPC, RAND, SQN, bytes.fromhex("b9b9"))
    expect = {"mac_a": "4a9ffac354dfafb3", "res": "a54211d5e3ba50bf", "ak": "aa689c648370",
              "ck": "b40ba9a3c58b2a05bbf0d987b21bf8cb", "ik": "f769bcd751044604127672711c6d3441"}
    assert OPC.hex() == "cd63cb71954a9f4e48a5994e37a02baf" and all(m[k].hex() == v for k, v in expect.items())
    print("Milenage Test Set 1 (TS 35.208): OK")

    SNN = b"5G:mnc004.mcc452.3gppnetwork.org"
    he = udm_generate(K, OPC, SQN, bytes.fromhex("b9b9"), RAND, SNN)
    print("AUTN =", he["autn"].hex(), "\nXRES* =", he["xres_star"].hex(), "\nKAUSF =", he["kausf"].hex())

    usim = USIM(K, OPC)
    good = run("Hợp lệ (Test Set 1)", usim, K, OPC, SQN, SNN, SNN, rand=RAND)
    run("Replay RAND/AUTN cũ", usim, K, OPC, SQN, SNN, SNN, replay=good)
    run("Mạng giả không có K", USIM(K, OPC), os.urandom(16), OPC, SQN, SNN, SNN)
    run("Mạng khai sai serving network name", USIM(K, OPC), K, OPC, SQN, SNN, b"5G:mnc099.mcc452.3gppnetwork.org")
