#!/usr/bin/env python3
"""Tự verify certificate chain từng bước (để học; production hãy dùng library). Dùng: leaf inter root hostname"""
import datetime as dt
import sys

from cryptography import x509
from cryptography.x509.oid import ExtendedKeyUsageOID

UTC = dt.timezone.utc


def load(path: str) -> x509.Certificate:
    with open(path, "rb") as f:
        return x509.load_pem_x509_certificate(f.read())


def validity(c: x509.Certificate):
    try:                                                     # cryptography ≥ 42
        return c.not_valid_before_utc, c.not_valid_after_utc
    except AttributeError:                                   # cryptography 41 (Ubuntu 24.04)
        return c.not_valid_before.replace(tzinfo=UTC), c.not_valid_after.replace(tzinfo=UTC)


def check_chain(leaf, inter, root, hostname: str) -> str:
    chain, now = [leaf, inter, root], dt.datetime.now(UTC)
    for i, cert in enumerate(chain):
        name = cert.subject.rfc4514_string()
        nb, na = validity(cert)
        if not nb <= now <= na:
            return f"✘ [{i}] {name}: ngoài thời hạn ({nb:%Y-%m-%d} → {na:%Y-%m-%d})"
        issuer = chain[i + 1] if i + 1 < len(chain) else root          # root tự ký
        try:
            cert.verify_directly_issued_by(issuer)                     # tên issuer + chữ ký
        except Exception as e:
            return f"✘ [{i}] {name}: không được ký bởi {issuer.subject.rfc4514_string()} ({type(e).__name__})"
        if i > 0:                                                      # cert này đóng vai issuer
            bc = cert.extensions.get_extension_for_class(x509.BasicConstraints).value
            if not bc.ca:
                return f"✘ [{i}] {name}: không phải CA nhưng lại ký cert khác"
            if bc.path_length is not None and bc.path_length < i - 1:
                return f"✘ [{i}] {name}: vượt pathlen={bc.path_length}"
        print(f"  ✔ [{i}] {name}  (hạn tới {na:%Y-%m-%d})")

    try:
        eku = leaf.extensions.get_extension_for_class(x509.ExtendedKeyUsage).value
    except x509.ExtensionNotFound:
        return "✘ leaf không có EKU"
    if ExtendedKeyUsageOID.SERVER_AUTH not in eku:
        return "✘ leaf không có EKU serverAuth: không dùng làm server cert được"
    names = leaf.extensions.get_extension_for_class(x509.SubjectAlternativeName).value.get_values_for_type(x509.DNSName)
    if hostname not in names:
        return f"✘ hostname '{hostname}' không có trong SAN {names}"
    return f"✔ chain hợp lệ cho {hostname}"


if __name__ == "__main__":
    leaf_p, inter_p, root_p, host = sys.argv[1:5]
    print(check_chain(load(leaf_p), load(inter_p), load(root_p), host))
