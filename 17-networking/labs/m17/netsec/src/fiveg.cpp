#include "netsec/fiveg.hpp"

#include "netsec/crypto.hpp"

namespace netsec {

static Bytes xor_b(const Bytes& a, const Bytes& b) {
    Bytes r(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) r[i] = a[i] ^ b[i];
    return r;
}
static Bytes rot(const Bytes& x, std::size_t nbytes) {         // xoay trái nbytes byte (r = 8 × nbytes bit)
    Bytes r(16);
    for (std::size_t i = 0; i < 16; ++i) r[i] = x[(i + nbytes) % 16];
    return r;
}
static Bytes cst(unsigned char v) {
    Bytes c(16, 0);
    c[15] = v;
    return c;
}
static Bytes slice(const Bytes& b, std::size_t off, std::size_t n) { return Bytes(b.begin() + off, b.begin() + off + n); }
static Bytes cat(std::initializer_list<Bytes> parts) {
    Bytes r;
    for (const Bytes& p : parts) r.insert(r.end(), p.begin(), p.end());
    return r;
}
static Bytes bytes_of(const std::string& s) { return Bytes(s.begin(), s.end()); }

Bytes opc_from_op(const Bytes& k, const Bytes& op) { return xor_b(aes128_ecb_block(k, op), op); }

MilenageOut milenage(const Bytes& k, const Bytes& opc, const Bytes& rand, const Bytes& sqn, const Bytes& amf) {
    if (k.size() != 16 || opc.size() != 16 || rand.size() != 16 || sqn.size() != 6 || amf.size() != 2)
        throw std::invalid_argument("milenage: sai độ dài tham số");
    auto E = [&](const Bytes& in) { return aes128_ecb_block(k, in); };
    const Bytes temp = E(xor_b(rand, opc));
    const Bytes in1 = cat({sqn, amf, sqn, amf});
    const Bytes out1 = xor_b(E(xor_b(temp, rot(xor_b(in1, opc), 8))), opc);   // r1 = 64 bit, c1 = 0
    const Bytes t = xor_b(temp, opc);
    auto out = [&](std::size_t r, unsigned char c) { return xor_b(E(xor_b(rot(t, r), cst(c))), opc); };
    const Bytes o2 = out(0, 1), o3 = out(4, 2), o4 = out(8, 4), o5 = out(12, 8);
    return {slice(out1, 0, 8), slice(out1, 8, 8), slice(o2, 8, 8), o3, o4, slice(o2, 0, 6), slice(o5, 0, 6)};
}

HeAv generate_he_av(const Bytes& k, const Bytes& opc, const Bytes& sqn, const Bytes& amf, const Bytes& rand,
                    const std::string& snn) {
    MilenageOut m = milenage(k, opc, rand, sqn, amf);
    const Bytes sqn_ak = xor_b(sqn, m.ak);
    Bytes ckik = cat({m.ck, m.ik});
    const Bytes sn = bytes_of(snn);
    HeAv av{rand, cat({sqn_ak, amf, m.mac_a}), slice(kdf_3gpp(ckik, 0x6B, {sn, rand, m.res}), 16, 16),
            kdf_3gpp(ckik, 0x6A, {sn, sqn_ak})};
    wipe(ckik);
    wipe(m.ck);
    wipe(m.ik);
    return av;
}

Bytes hxres_star(const Bytes& rand, const Bytes& xres_star) { return slice(sha256(cat({rand, xres_star})), 16, 16); }

Bytes kseaf_from_kausf(const Bytes& kausf, const std::string& snn) { return kdf_3gpp(kausf, 0x6C, {bytes_of(snn)}); }

UeResult usim_authenticate(const Bytes& k, const Bytes& opc, const Bytes& rand, const Bytes& autn, const std::string& snn,
                           std::uint64_t& sqn_ms) {
    if (autn.size() != 16) throw AuthError("AUTN phải 16 byte");
    const Bytes ak = milenage(k, opc, rand, Bytes(6), Bytes(2)).ak;      // f5 chỉ phụ thuộc K, OPc, RAND
    const Bytes sqn = xor_b(slice(autn, 0, 6), ak);
    MilenageOut m = milenage(k, opc, rand, sqn, slice(autn, 6, 2));
    if (!equal_ct(m.mac_a, slice(autn, 8, 8))) throw AuthError("MAC failure: mạng không có K của SIM này");
    std::uint64_t s = 0;
    for (unsigned char b : sqn) s = s << 8 | b;
    if (s <= sqn_ms) throw AuthError("Synch failure: SQN cũ, có thể là replay");
    sqn_ms = s;
    Bytes ckik = cat({m.ck, m.ik});
    const Bytes sn = bytes_of(snn);
    Bytes kausf = kdf_3gpp(ckik, 0x6A, {sn, slice(autn, 0, 6)});
    UeResult r{slice(kdf_3gpp(ckik, 0x6B, {sn, rand, m.res}), 16, 16), kseaf_from_kausf(kausf, snn)};
    wipe(ckik);
    wipe(kausf);
    wipe(m.ck);
    wipe(m.ik);
    return r;
}

std::string suci_profile_a_deconceal(const Bytes& hn_private_raw, const Bytes& out) {
    if (hn_private_raw.size() != 32) throw std::invalid_argument("private key X25519 phải 32 byte");
    if (out.size() < 32 + 1 + 8) throw std::invalid_argument("scheme output quá ngắn");
    const Bytes eph(out.begin(), out.begin() + 32), ct(out.begin() + 32, out.end() - 8), tag(out.end() - 8, out.end());
    PkeyPtr priv(check_ptr(EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr, hn_private_raw.data(), 32), "X25519 private key"));
    PkeyPtr peer(check_ptr(EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr, eph.data(), 32), "X25519 public key"));
    PkeyCtxPtr dctx(check_ptr(EVP_PKEY_CTX_new(priv.get(), nullptr), "EVP_PKEY_CTX_new"));
    check(EVP_PKEY_derive_init(dctx.get()), "EVP_PKEY_derive_init");
    check(EVP_PKEY_derive_set_peer(dctx.get(), peer.get()), "EVP_PKEY_derive_set_peer");
    Bytes z(32);
    std::size_t zlen = z.size();
    check(EVP_PKEY_derive(dctx.get(), z.data(), &zlen), "EVP_PKEY_derive");
    Bytes k = x963_kdf_sha256(z, eph, 64);                          // AES key ‖ ICB ‖ MAC key
    wipe(z);
    const Bytes enc_key = slice(k, 0, 16), icb = slice(k, 16, 16);
    Bytes mac_key = slice(k, 32, 32);
    wipe(k);
    Bytes mac = hmac_sha256(mac_key, ct);
    wipe(mac_key);
    mac.resize(8);
    if (!equal_ct(mac, tag)) throw AuthError("MAC sai: SUCI đã bị sửa");
    const Bytes pt = aes128_ctr(enc_key, icb, ct);
    std::string msin;                                               // TBCD: chữ số đứng sau ở nibble cao, đệm F
    for (unsigned char b : pt) {
        const unsigned lo = b & 0x0Fu, hi = b >> 4;
        for (unsigned nib : {lo, hi}) {
            if (nib == 0xF) break;
            if (nib > 9) throw AuthError("MSIN không phải BCD");
            msin += static_cast<char>('0' + nib);
        }
    }
    return msin;
}

}  // namespace netsec
