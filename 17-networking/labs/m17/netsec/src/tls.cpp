#include "netsec/tls.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <thread>

#include <openssl/x509v3.h>

#include "netsec/crypto.hpp"
#include "netsec/log.hpp"

namespace netsec {

std::shared_ptr<SSL_CTX> make_tls_ctx(bool server, const std::string& cert_chain, const std::string& key,
                                      const std::string& ca) {
    std::shared_ptr<SSL_CTX> ctx(check_ptr(SSL_CTX_new(server ? TLS_server_method() : TLS_client_method()), "SSL_CTX_new"),
                                 SSL_CTX_free);
    check(SSL_CTX_set_min_proto_version(ctx.get(), TLS1_3_VERSION), "SSL_CTX_set_min_proto_version");
    check(SSL_CTX_use_certificate_chain_file(ctx.get(), cert_chain.c_str()), "SSL_CTX_use_certificate_chain_file");
    check(SSL_CTX_use_PrivateKey_file(ctx.get(), key.c_str(), SSL_FILETYPE_PEM), "SSL_CTX_use_PrivateKey_file");
    check(SSL_CTX_check_private_key(ctx.get()), "cert và key không khớp");
    check(SSL_CTX_load_verify_locations(ctx.get(), ca.c_str(), nullptr), "SSL_CTX_load_verify_locations");
    // Thiếu SSL_VERIFY_FAIL_IF_NO_PEER_CERT thì client không gửi cert vẫn qua handshake
    SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_PEER | (server ? SSL_VERIFY_FAIL_IF_NO_PEER_CERT : 0), nullptr);
    // EKU: server chỉ nhận client cert (clientAuth), client chỉ nhận server cert (serverAuth)
    check(SSL_CTX_set_purpose(ctx.get(), server ? X509_PURPOSE_SSL_CLIENT : X509_PURPOSE_SSL_SERVER), "SSL_CTX_set_purpose");
    return ctx;
}

ReloadingTlsCtx::ReloadingTlsCtx(bool server, std::string cert_chain, std::string key, std::string ca)
    : server_(server), cert_(std::move(cert_chain)), key_(std::move(key)), ca_(std::move(ca)) {}

std::shared_ptr<SSL_CTX> ReloadingTlsCtx::get() {
    std::lock_guard<std::mutex> lock(mu_);
    try {
        auto mtime = std::filesystem::last_write_time(cert_);
        if (!ctx_ || mtime != mtime_) {
            ctx_ = make_tls_ctx(server_, cert_, key_, ca_);
            mtime_ = mtime;
            log_json("tls_context_loaded", {{"side", server_ ? "server" : "client"}, {"cert", cert_}});
        }
    } catch (const std::exception& e) {
        if (!ctx_) throw;                              // lần đầu mà hỏng: không chạy được
        log_json("tls_reload_failed", {{"error", e.what()}});   // file đang ghi dở: dùng tiếp ctx cũ
    }
    return ctx_;
}

Peer peer_identity(SSL* ssl) {
    Peer peer;
    X509Ptr cert(SSL_get1_peer_certificate(ssl));
    if (!cert) return peer;
    std::unique_ptr<GENERAL_NAMES, Deleter<GENERAL_NAMES_free>> sans(
        static_cast<GENERAL_NAMES*>(X509_get_ext_d2i(cert.get(), NID_subject_alt_name, nullptr, nullptr)));
    int found = 0;
    std::string id;
    for (int i = 0; sans && i < sk_GENERAL_NAME_num(sans.get()); ++i) {
        const GENERAL_NAME* gn = sk_GENERAL_NAME_value(sans.get(), i);
        if (gn->type != GEN_URI) continue;
        const ASN1_IA5STRING* s = gn->d.uniformResourceIdentifier;
        std::string uri(reinterpret_cast<const char*>(ASN1_STRING_get0_data(s)), static_cast<std::size_t>(ASN1_STRING_length(s)));
        if (uri.rfind("spiffe://", 0) == 0) {
            id = uri;
            ++found;
        }
    }
    if (found == 1) peer.spiffe_id = id;               // 0 hoặc nhiều hơn 1: coi như không có identity
    unsigned char* der = nullptr;
    int len = i2d_X509(cert.get(), &der);
    if (len > 0) {
        Bytes d(der, der + len);
        OPENSSL_free(der);
        peer.x5t_s256 = b64url_encode(sha256(d));
    }
    return peer;
}

static int listen_tcp(std::uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket() thất bại");
    int one = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0 || ::listen(fd, 128) < 0)
        throw std::runtime_error("không bind/listen được port " + std::to_string(port));
    return fd;
}

void serve_tls(std::uint16_t port, ReloadingTlsCtx& tls, const Handler& handler) {
    std::signal(SIGPIPE, SIG_IGN);                     // ghi vào socket đã đóng không được giết process
    tls.get();                                         // nạp cert ngay, hỏng thì dừng sớm
    int lfd = listen_tcp(port);
    log_json("listening", {{"port", port}});
    for (;;) {
        sockaddr_in addr{};
        socklen_t alen = sizeof addr;
        int fd = ::accept(lfd, reinterpret_cast<sockaddr*>(&addr), &alen);
        if (fd < 0) continue;
        char ip[INET_ADDRSTRLEN] = "?";
        ::inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof ip);
        std::shared_ptr<SSL_CTX> ctx = tls.get();
        std::thread([fd, ctx, &handler, client = std::string(ip)] {
            timeval tv{10, 0};                         // client im lặng 10 giây thì bỏ
            ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
            SslPtr ssl(SSL_new(ctx.get()));
            if (ssl && SSL_set_fd(ssl.get(), fd) == 1 && SSL_accept(ssl.get()) == 1) {
                try {
                    handler(ssl.get(), peer_identity(ssl.get()));
                } catch (const std::exception& e) {
                    log_json("handler_error", {{"client", client}, {"error", e.what()}});
                }
                SSL_shutdown(ssl.get());
            } else {
                long vr = ssl ? SSL_get_verify_result(ssl.get()) : X509_V_OK;
                log_json("tls_reject", {{"client", client},
                                        {"verify", X509_verify_cert_error_string(vr)},
                                        {"error", ossl_errors()}});
            }
            ::close(fd);
        }).detach();
    }
}

int connect_tcp(const std::string& host, std::uint16_t port) {
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = nullptr;
    if (int rc = ::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res); rc != 0)
        throw std::runtime_error("getaddrinfo " + host + ": " + ::gai_strerror(rc));
    std::unique_ptr<addrinfo, decltype(&::freeaddrinfo)> guard(res, ::freeaddrinfo);
    for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
        int fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        timeval tv{5, 0};
        ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
        if (::connect(fd, p->ai_addr, p->ai_addrlen) == 0) return fd;
        ::close(fd);
    }
    throw std::runtime_error("không kết nối được " + host + ":" + std::to_string(port));
}

}  // namespace netsec
