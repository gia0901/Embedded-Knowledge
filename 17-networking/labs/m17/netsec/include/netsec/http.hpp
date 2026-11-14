#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "netsec/ossl.hpp"

namespace netsec {

struct Request {
    std::string method, path;
    std::map<std::string, std::string> headers;   // tên header viết thường
};

std::optional<Request> read_request(SSL* ssl, std::size_t max_bytes = 8192);
void write_response(SSL* ssl, int status, const nlohmann::json& body,
                    const std::vector<std::pair<std::string, std::string>>& extra = {});
// GET qua TLS với ctx client (mTLS), có SNI và kiểm tra hostname. Trả về body nếu status 200.
std::string https_get(const std::string& host, std::uint16_t port, const std::string& path, SSL_CTX* ctx);

}  // namespace netsec
