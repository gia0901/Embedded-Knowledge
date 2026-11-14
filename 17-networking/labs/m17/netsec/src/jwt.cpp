#include "netsec/jwt.hpp"

#include <algorithm>
#include <ctime>
#include <vector>

#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/ecdsa.h>
#include <openssl/params.h>

#include "netsec/crypto.hpp"
#include "netsec/log.hpp"

namespace netsec {

using nlohmann::json;

std::shared_ptr<EVP_PKEY> ec_p256_from_jwk(const json& jwk) {
    if (jwk.value("kty", "") != "EC" || jwk.value("crv", "") != "P-256") throw JwtError("JWK không phải EC P-256");
    Bytes x = b64url_decode(jwk.at("x").get<std::string>());
    Bytes y = b64url_decode(jwk.at("y").get<std::string>());
    if (x.size() != 32 || y.size() != 32) throw JwtError("JWK: x, y phải 32 byte");
    Bytes pub{0x04};                                   // điểm không nén: 04 ‖ x ‖ y
    pub.insert(pub.end(), x.begin(), x.end());
    pub.insert(pub.end(), y.begin(), y.end());
    char group[] = "prime256v1";
    OSSL_PARAM params[] = {OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, group, 0),
                           OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_PUB_KEY, pub.data(), pub.size()),
                           OSSL_PARAM_construct_end()};
    PkeyCtxPtr pctx(check_ptr(EVP_PKEY_CTX_new_from_name(nullptr, "EC", nullptr), "EVP_PKEY_CTX_new_from_name"));
    check(EVP_PKEY_fromdata_init(pctx.get()), "EVP_PKEY_fromdata_init");
    EVP_PKEY* raw = nullptr;
    check(EVP_PKEY_fromdata(pctx.get(), &raw, EVP_PKEY_PUBLIC_KEY, params), "EVP_PKEY_fromdata (điểm không nằm trên curve?)");
    return std::shared_ptr<EVP_PKEY>(raw, EVP_PKEY_free);
}

JwksCache::JwksCache(Fetch fetch, std::chrono::seconds max_age, std::chrono::seconds min_refresh)
    : fetch_(std::move(fetch)), max_age_(max_age), min_refresh_(min_refresh) {}

std::shared_ptr<EVP_PKEY> JwksCache::key(const std::string& kid) {
    std::lock_guard<std::mutex> lock(mu_);
    const auto now = std::chrono::steady_clock::now();
    const bool stale = !tried_ || now - fetched_ > max_age_;                           // TTL: key đã gỡ phải hết được tin
    const bool unknown = keys_.count(kid) == 0 && (!tried_ || now - fetched_ > min_refresh_);
    if (stale || unknown) {
        tried_ = true;
        fetched_ = now;
        try {
            refresh_locked();
        } catch (const std::exception& e) {
            log_json("jwks_refresh_failed", {{"error", e.what()}});                     // giữ cache cũ
        }
    }
    auto it = keys_.find(kid);
    return it == keys_.end() ? nullptr : it->second;
}

void JwksCache::refresh_locked() {
    const json doc = json::parse(fetch_());
    std::map<std::string, std::shared_ptr<EVP_PKEY>> fresh;
    for (const json& k : doc.at("keys")) {
        if (k.value("use", "sig") != "sig" || k.value("alg", "ES256") != "ES256") continue;
        fresh[k.at("kid").get<std::string>()] = ec_p256_from_jwk(k);
    }
    keys_.swap(fresh);
    std::vector<std::string> kids;
    for (const auto& kv : keys_) kids.push_back(kv.first);
    log_json("jwks_refreshed", {{"kids", kids}});
}

// JWT (RFC 7518) dùng r ‖ s, mỗi số 32 byte. EVP cần cấu trúc DER ECDSA-Sig-Value.
static Bytes raw_to_der(const Bytes& sig) {
    if (sig.size() != 64) throw JwtError("chữ ký ES256 phải đúng 64 byte (r ‖ s)");
    std::unique_ptr<ECDSA_SIG, Deleter<ECDSA_SIG_free>> s(check_ptr(ECDSA_SIG_new(), "ECDSA_SIG_new"));
    BIGNUM* r = BN_bin2bn(sig.data(), 32, nullptr);
    BIGNUM* ss = BN_bin2bn(sig.data() + 32, 32, nullptr);
    if (r == nullptr || ss == nullptr || ECDSA_SIG_set0(s.get(), r, ss) != 1) {   // set0 thành công thì s sở hữu r, ss
        BN_free(r);
        BN_free(ss);
        throw_ossl("ECDSA_SIG_set0");
    }
    unsigned char* der = nullptr;
    int len = i2d_ECDSA_SIG(s.get(), &der);
    if (len <= 0) throw_ossl("i2d_ECDSA_SIG");
    Bytes out(der, der + len);
    OPENSSL_free(der);
    return out;
}

static bool es256_verify(EVP_PKEY* key, const std::string& signing_input, const Bytes& der_sig) {
    MdCtxPtr md(check_ptr(EVP_MD_CTX_new(), "EVP_MD_CTX_new"));
    check(EVP_DigestVerifyInit(md.get(), nullptr, EVP_sha256(), nullptr, key), "EVP_DigestVerifyInit");
    int rc = EVP_DigestVerify(md.get(), der_sig.data(), der_sig.size(),
                              reinterpret_cast<const unsigned char*>(signing_input.data()), signing_input.size());
    ERR_clear_error();
    return rc == 1;                                    // KHÔNG viết `return rc;`: -1 (lỗi) sẽ thành true
}

static json verify_impl(const std::string& token, JwksCache& jwks, const VerifyOptions& opt) {
    const auto d1 = token.find('.');
    const auto d2 = d1 == std::string::npos ? std::string::npos : token.find('.', d1 + 1);
    if (d1 == std::string::npos || d2 == std::string::npos || token.find('.', d2 + 1) != std::string::npos)
        throw JwtError("token phải có đúng 3 phần");
    const std::string h64 = token.substr(0, d1), p64 = token.substr(d1 + 1, d2 - d1 - 1), s64 = token.substr(d2 + 1);
    auto text = [](const Bytes& b) { return std::string(b.begin(), b.end()); };

    const json header = json::parse(text(b64url_decode(h64)));
    const std::string alg = header.value("alg", "");
    if (alg != "ES256") throw JwtError("alg không được phép: " + alg);                    // allowlist trước mọi thứ
    const std::string kid = header.value("kid", "");
    std::shared_ptr<EVP_PKEY> key = jwks.key(kid);
    if (!key) throw JwtError("kid không có trong JWKS: " + kid);
    if (!es256_verify(key.get(), h64 + "." + p64, raw_to_der(b64url_decode(s64)))) throw JwtError("sai chữ ký");

    const json claims = json::parse(text(b64url_decode(p64)));
    if (claims.value("iss", "") != opt.issuer) throw JwtError("iss sai: " + claims.value("iss", ""));
    const json aud = claims.value("aud", json());
    const bool aud_ok = aud.is_string() ? aud == opt.audience
                                        : aud.is_array() && std::find(aud.begin(), aud.end(), opt.audience) != aud.end();
    if (!aud_ok) throw JwtError("aud sai");
    const long long now = static_cast<long long>(std::time(nullptr));
    if (!claims.contains("exp") || !claims["exp"].is_number() || now > claims["exp"].get<long long>() + opt.leeway)
        throw JwtError("hết hạn hoặc thiếu exp");
    if (claims.contains("nbf") && claims["nbf"].is_number() && now + opt.leeway < claims["nbf"].get<long long>())
        throw JwtError("chưa tới nbf");
    for (const char* name : {"sub", "scope", "cnf"})
        if (!claims.contains(name)) throw JwtError(std::string("thiếu claim ") + name);
    return claims;
}

json verify_es256(const std::string& token, JwksCache& jwks, const VerifyOptions& opt) {
    try {
        return verify_impl(token, jwks, opt);
    } catch (const JwtError&) {
        throw;
    } catch (const std::exception& e) {                // JSON hỏng, base64url hỏng, lỗi OpenSSL…
        throw JwtError(std::string("token hỏng: ") + e.what());
    }
}

}  // namespace netsec
