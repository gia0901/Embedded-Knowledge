#include "netsec/crypto.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>

#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/params.h>

namespace netsec {

Bytes from_hex(const std::string& hex) {
    if (hex.size() % 2 != 0) throw std::invalid_argument("chuỗi hex có số ký tự lẻ");
    Bytes out(hex.size() / 2);
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] = static_cast<unsigned char>(std::stoi(hex.substr(2 * i, 2), nullptr, 16));
    return out;
}

std::string to_hex(const Bytes& b) {
    std::ostringstream os;
    for (unsigned char c : b) os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    return os.str();
}

static const char kB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

std::string b64url_encode(const Bytes& b) {
    std::string out;
    std::size_t i = 0;
    for (; i + 2 < b.size(); i += 3) {
        std::uint32_t v = b[i] << 16 | b[i + 1] << 8 | b[i + 2];
        out += {kB64[v >> 18 & 63], kB64[v >> 12 & 63], kB64[v >> 6 & 63], kB64[v & 63]};
    }
    if (i + 1 == b.size()) {
        std::uint32_t v = b[i] << 16;
        out += {kB64[v >> 18 & 63], kB64[v >> 12 & 63]};
    } else if (i + 2 == b.size()) {
        std::uint32_t v = b[i] << 16 | b[i + 1] << 8;
        out += {kB64[v >> 18 & 63], kB64[v >> 12 & 63], kB64[v >> 6 & 63]};
    }
    return out;
}

Bytes b64url_decode(const std::string& s) {
    Bytes out;
    std::uint32_t acc = 0;
    int bits = 0;
    for (char c : s) {
        int v;
        if (c >= 'A' && c <= 'Z') v = c - 'A';
        else if (c >= 'a' && c <= 'z') v = c - 'a' + 26;
        else if (c >= '0' && c <= '9') v = c - '0' + 52;
        else if (c == '-') v = 62;
        else if (c == '_') v = 63;
        else throw std::invalid_argument("ký tự base64url không hợp lệ");
        acc = (acc << 6) | static_cast<std::uint32_t>(v);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<unsigned char>((acc >> bits) & 0xff));
        }
    }
    return out;
}

Bytes sha256(const Bytes& data) {
    Bytes out(EVP_MAX_MD_SIZE);
    unsigned int len = 0;
    check(EVP_Digest(data.data(), data.size(), out.data(), &len, EVP_sha256(), nullptr), "EVP_Digest");
    out.resize(len);
    return out;
}

Bytes hmac_sha256(const Bytes& key, const Bytes& data) {
    std::unique_ptr<EVP_MAC, Deleter<EVP_MAC_free>> mac(check_ptr(EVP_MAC_fetch(nullptr, "HMAC", nullptr), "EVP_MAC_fetch"));
    std::unique_ptr<EVP_MAC_CTX, Deleter<EVP_MAC_CTX_free>> ctx(check_ptr(EVP_MAC_CTX_new(mac.get()), "EVP_MAC_CTX_new"));
    char digest[] = "SHA256";
    OSSL_PARAM params[] = {OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST, digest, 0), OSSL_PARAM_construct_end()};
    check(EVP_MAC_init(ctx.get(), key.data(), key.size(), params), "EVP_MAC_init");
    check(EVP_MAC_update(ctx.get(), data.data(), data.size()), "EVP_MAC_update");
    Bytes out(32);
    std::size_t len = 0;
    check(EVP_MAC_final(ctx.get(), out.data(), &len, out.size()), "EVP_MAC_final");
    out.resize(len);
    return out;
}

// S = FC ‖ P0 ‖ L0 ‖ P1 ‖ L1 …, L là độ dài 2 byte big-endian; output = HMAC-SHA-256(key, S)
Bytes kdf_3gpp(const Bytes& key, unsigned char fc, std::initializer_list<Bytes> params) {
    Bytes s{fc};
    for (const Bytes& p : params) {
        s.insert(s.end(), p.begin(), p.end());
        s.push_back(static_cast<unsigned char>(p.size() >> 8));
        s.push_back(static_cast<unsigned char>(p.size() & 0xff));
    }
    return hmac_sha256(key, s);
}

// K = SHA-256(Z ‖ counter(4 byte, bắt đầu từ 1) ‖ SharedInfo) nối liên tiếp
Bytes x963_kdf_sha256(const Bytes& z, const Bytes& shared_info, std::size_t len) {
    Bytes out;
    for (std::uint32_t counter = 1; out.size() < len; ++counter) {
        Bytes block(z);
        for (int shift = 24; shift >= 0; shift -= 8) block.push_back(static_cast<unsigned char>(counter >> shift));
        block.insert(block.end(), shared_info.begin(), shared_info.end());
        Bytes h = sha256(block);
        out.insert(out.end(), h.begin(), h.end());
        wipe(block);
    }
    out.resize(len);
    return out;
}

Bytes aes128_ecb_block(const Bytes& key, const Bytes& block) {
    if (key.size() != 16 || block.size() != 16) throw std::invalid_argument("AES-128 cần key và block 16 byte");
    CipherCtxPtr ctx(check_ptr(EVP_CIPHER_CTX_new(), "EVP_CIPHER_CTX_new"));
    check(EVP_EncryptInit_ex2(ctx.get(), EVP_aes_128_ecb(), key.data(), nullptr, nullptr), "EVP_EncryptInit_ex2");
    EVP_CIPHER_CTX_set_padding(ctx.get(), 0);
    Bytes out(16);
    int len = 0;
    check(EVP_EncryptUpdate(ctx.get(), out.data(), &len, block.data(), 16), "EVP_EncryptUpdate");
    return out;
}

Bytes aes128_ctr(const Bytes& key, const Bytes& iv, const Bytes& data) {
    if (key.size() != 16 || iv.size() != 16) throw std::invalid_argument("AES-128-CTR cần key và IV 16 byte");
    CipherCtxPtr ctx(check_ptr(EVP_CIPHER_CTX_new(), "EVP_CIPHER_CTX_new"));
    check(EVP_EncryptInit_ex2(ctx.get(), EVP_aes_128_ctr(), key.data(), iv.data(), nullptr), "EVP_EncryptInit_ex2");
    Bytes out(data.size());
    int len = 0;
    if (!data.empty())
        check(EVP_EncryptUpdate(ctx.get(), out.data(), &len, data.data(), static_cast<int>(data.size())), "EVP_EncryptUpdate");
    return out;
}

bool equal_ct(const Bytes& a, const Bytes& b) {
    return a.size() == b.size() && CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
}

void wipe(Bytes& secret) {
    if (!secret.empty()) OPENSSL_cleanse(secret.data(), secret.size());
    secret.clear();
}

}  // namespace netsec
