// mtls_hello <port> <cert_chain> <key> <ca> [SPIFFE ID được phép …]
// mTLS server: authenticate bằng client cert, rồi authorize theo SPIFFE ID (module 11: authN ≠ authZ).
#include <iostream>
#include <set>
#include <string>

#include "netsec/log.hpp"
#include "netsec/tls.hpp"

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Dùng: mtls_hello <port> <cert_chain> <key> <ca> [spiffe-id …]\n";
        return 2;
    }
    const std::set<std::string> allowed(argv + 5, argv + argc);
    netsec::ReloadingTlsCtx tls(true, argv[2], argv[3], argv[4]);
    const netsec::Handler handler = [&](SSL* ssl, const netsec::Peer& peer) {
        const bool ok = allowed.count(peer.spiffe_id) == 1;
        netsec::log_json(ok ? "allow" : "deny", {{"client", peer.spiffe_id}, {"x5t", peer.x5t_s256}});
        const std::string msg = ok ? "xin chào " + peer.spiffe_id + "\n"
                                   : "403: " + peer.spiffe_id + " không được phép\n";
        SSL_write(ssl, msg.data(), static_cast<int>(msg.size()));
    };
    try {
        netsec::serve_tls(static_cast<std::uint16_t>(std::stoi(argv[1])), tls, handler);
    } catch (const std::exception& e) {
        std::cerr << "lỗi: " << e.what() << "\n";
        return 1;
    }
}
