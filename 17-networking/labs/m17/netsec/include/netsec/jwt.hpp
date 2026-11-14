#pragma once
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "netsec/ossl.hpp"

namespace netsec {

struct JwtError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

std::shared_ptr<EVP_PKEY> ec_p256_from_jwk(const nlohmann::json& jwk);

class JwksCache {
public:
    using Fetch = std::function<std::string()>;   // trả về body JSON của JWKS
    explicit JwksCache(Fetch fetch, std::chrono::seconds max_age = std::chrono::seconds(300),
                       std::chrono::seconds min_refresh = std::chrono::seconds(10));
    std::shared_ptr<EVP_PKEY> key(const std::string& kid);

private:
    void refresh_locked();
    Fetch fetch_;
    std::chrono::seconds max_age_, min_refresh_;
    std::mutex mu_;
    std::map<std::string, std::shared_ptr<EVP_PKEY>> keys_;
    std::chrono::steady_clock::time_point fetched_{};
    bool tried_ = false;
};

struct VerifyOptions {
    std::string issuer;
    std::string audience;
    long leeway = 30;
};

// Trả về claims nếu hợp lệ, throw JwtError (kèm lý do) nếu không
nlohmann::json verify_es256(const std::string& token, JwksCache& jwks, const VerifyOptions& opt);

}  // namespace netsec
