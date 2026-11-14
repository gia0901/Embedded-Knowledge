// orders-api bản C++: thay thẳng bản Python của capstone (module 16), phải qua nguyên test.sh.
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "netsec/http.hpp"
#include "netsec/jwt.hpp"
#include "netsec/log.hpp"
#include "netsec/tls.hpp"

using nlohmann::json;

static std::string env(const char* name, const char* fallback) {
    const char* v = std::getenv(name);
    return v ? v : fallback;
}

int main() {
    const std::string tls_dir = env("TLS_DIR", "/tls"), trust = env("TRUST_FILE", "/trust/root.crt");
    const std::string issuer = env("ISSUER", "https://token.security.svc.cluster.local:8443");
    const std::string audience = env("AUDIENCE", "orders-api");
    const std::string hostport = issuer.rfind("https://", 0) == 0 ? issuer.substr(8) : issuer;
    const auto colon = hostport.find(':');
    const std::string jwks_host = hostport.substr(0, colon);
    const auto jwks_port = static_cast<std::uint16_t>(colon == std::string::npos ? 443 : std::stoi(hostport.substr(colon + 1)));

    netsec::ReloadingTlsCtx server_tls(true, tls_dir + "/tls.crt", tls_dir + "/tls.key", trust);
    netsec::ReloadingTlsCtx client_tls(false, tls_dir + "/tls.crt", tls_dir + "/tls.key", trust);   // cert của orders-api
    netsec::JwksCache jwks([&] { return netsec::https_get(jwks_host, jwks_port, "/.well-known/jwks.json", client_tls.get().get()); });
    const netsec::VerifyOptions opt{issuer, audience, 30};
    const std::map<std::pair<std::string, std::string>, std::string> routes = {{{"GET", "/orders"}, "orders:read"},
                                                                             {{"POST", "/orders"}, "orders:write"}};

    const netsec::Handler handler = [&](SSL* ssl, const netsec::Peer& peer) {
        const auto req = netsec::read_request(ssl);
        if (!req) return;
        const std::string action = req->method + " " + req->path;
        const auto route = routes.find({req->method, req->path});
        if (route == routes.end()) return netsec::write_response(ssl, 404, {{"error", "not_found"}});
        const std::string& need = route->second;
        auto deny = [&](int status, const std::string& error, const std::string& detail, const std::string& sub = "") {
            netsec::log_json("deny", {{"client", peer.spiffe_id}, {"action", action}, {"status", status},
                                      {"error", error}, {"detail", detail}, {"sub", sub}});
            std::string www = "Bearer error=\"" + error + "\"";
            if (status == 403) www += ", scope=\"" + need + "\"";
            netsec::write_response(ssl, status, {{"error", error}, {"detail", detail}}, {{"WWW-Authenticate", www}});
        };
        const auto h = req->headers.find("authorization");
        const std::string auth = h == req->headers.end() ? "" : h->second;
        if (auth.rfind("Bearer ", 0) != 0) return deny(401, "invalid_request", "thiếu Bearer token");
        json claims;
        try {
            claims = netsec::verify_es256(auth.substr(7), jwks, opt);
        } catch (const netsec::JwtError& e) {
            return deny(401, "invalid_token", e.what());
        }
        const std::string sub = claims["sub"].is_string() ? claims["sub"].get<std::string>() : "";
        const json& cnf = claims["cnf"];
        const bool bound = cnf.is_object() && cnf.contains("x5t#S256") && cnf["x5t#S256"].is_string() &&
                           cnf["x5t#S256"].get<std::string>() == peer.x5t_s256;
        if (!bound) return deny(401, "invalid_token", "token không gắn với client cert này", sub);   // token bị đánh cắp
        std::istringstream scopes(claims["scope"].is_string() ? claims["scope"].get<std::string>() : "");
        bool allowed = false;
        for (std::string s; scopes >> s;) allowed = allowed || s == need;
        if (!allowed) return deny(403, "insufficient_scope", "", sub);
        netsec::log_json("allow", {{"client", peer.spiffe_id}, {"sub", sub}, {"action", action},
                                   {"jti", claims.value("jti", "")}});
        netsec::write_response(ssl, 200, {{"ok", true}, {"caller", sub}, {"action", action}});
    };

    try {
        netsec::log_json("start", {{"service", "orders-api (C++)"}, {"issuer", issuer}, {"audience", audience}});
        netsec::serve_tls(8443, server_tls, handler);
    } catch (const std::exception& e) {
        netsec::log_json("fatal", {{"error", e.what()}});
        return 1;
    }
}
