#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "netsec/ossl.hpp"

namespace netsec {

struct Peer {
    std::string spiffe_id;   // rỗng nếu cert không có đúng một SPIFFE ID
    std::string x5t_s256;    // base64url(SHA-256(DER của cert)), như claim cnf ở capstone
};

// server = true: bắt buộc client cert có EKU clientAuth. server = false: verify server cert có EKU serverAuth.
std::shared_ptr<SSL_CTX> make_tls_ctx(bool server, const std::string& cert_chain, const std::string& key,
                                      const std::string& ca);

// Tạo SSL_CTX mới khi file cert đổi. Connection đang mở giữ ctx cũ (SSL giữ tham chiếu tới SSL_CTX).
class ReloadingTlsCtx {
public:
    ReloadingTlsCtx(bool server, std::string cert_chain, std::string key, std::string ca);
    std::shared_ptr<SSL_CTX> get();

private:
    bool server_;
    std::string cert_, key_, ca_;
    std::mutex mu_;
    std::shared_ptr<SSL_CTX> ctx_;
    std::filesystem::file_time_type mtime_{};
};

Peer peer_identity(SSL* ssl);

using Handler = std::function<void(SSL*, const Peer&)>;
[[noreturn]] void serve_tls(std::uint16_t port, ReloadingTlsCtx& tls, const Handler& handler);
int connect_tcp(const std::string& host, std::uint16_t port);

}  // namespace netsec
