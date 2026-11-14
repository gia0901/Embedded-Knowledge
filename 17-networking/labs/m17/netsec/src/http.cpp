#include "netsec/http.hpp"

#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <sstream>

#include "netsec/tls.hpp"

namespace netsec {

std::optional<Request> read_request(SSL* ssl, std::size_t max_bytes) {
    std::string buf;
    char chunk[1024];
    while (buf.find("\r\n\r\n") == std::string::npos) {
        if (buf.size() > max_bytes) return std::nullopt;            // header quá lớn: bỏ
        int n = SSL_read(ssl, chunk, sizeof chunk);
        if (n <= 0) {
            ERR_clear_error();
            return std::nullopt;
        }
        buf.append(chunk, static_cast<std::size_t>(n));
    }
    std::istringstream in(buf.substr(0, buf.find("\r\n\r\n")));
    Request r;
    std::string line, version;
    if (!std::getline(in, line)) return std::nullopt;
    std::istringstream first(line);
    if (!(first >> r.method >> r.path >> version) || version.rfind("HTTP/1.", 0) != 0) return std::nullopt;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string name = line.substr(0, colon), value = line.substr(colon + 1);
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        value.erase(0, value.find_first_not_of(" \t"));
        r.headers[name] = value;
    }
    return r;
}

void write_response(SSL* ssl, int status, const nlohmann::json& body,
                    const std::vector<std::pair<std::string, std::string>>& extra) {
    static const std::map<int, const char*> reason = {{200, "OK"},          {400, "Bad Request"}, {401, "Unauthorized"},
                                                      {403, "Forbidden"},   {404, "Not Found"},   {500, "Internal Server Error"}};
    const auto it = reason.find(status);
    const std::string data = body.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    std::string out = "HTTP/1.1 " + std::to_string(status) + " " + (it != reason.end() ? it->second : "Error") + "\r\n";
    out += "Content-Type: application/json\r\nCache-Control: no-store\r\nConnection: close\r\n";
    out += "Content-Length: " + std::to_string(data.size()) + "\r\n";
    for (const auto& [name, value] : extra) out += name + ": " + value + "\r\n";
    out += "\r\n" + data;
    SSL_write(ssl, out.data(), static_cast<int>(out.size()));
}

std::string https_get(const std::string& host, std::uint16_t port, const std::string& path, SSL_CTX* ctx) {
    struct Fd {
        int fd;
        ~Fd() { if (fd >= 0) ::close(fd); }
    } sock{connect_tcp(host, port)};
    SslPtr ssl(check_ptr(SSL_new(ctx), "SSL_new"));
    check(SSL_set_fd(ssl.get(), sock.fd), "SSL_set_fd");
    check(SSL_set_tlsext_host_name(ssl.get(), host.c_str()), "SNI");
    check(SSL_set1_host(ssl.get(), host.c_str()), "SSL_set1_host");    // thiếu dòng này: nhận cert của bất kỳ tên nào
    if (SSL_connect(ssl.get()) != 1) throw_ossl("TLS tới " + host + " thất bại");
    const std::string req = "GET " + path + " HTTP/1.0\r\nHost: " + host + "\r\n\r\n";
    if (SSL_write(ssl.get(), req.data(), static_cast<int>(req.size())) <= 0) throw_ossl("SSL_write");
    std::string resp;
    char buf[4096];
    for (int n; (n = SSL_read(ssl.get(), buf, sizeof buf)) > 0;) resp.append(buf, static_cast<std::size_t>(n));
    ERR_clear_error();                                                  // EOF không có close_notify: bình thường ở đây
    const auto body = resp.find("\r\n\r\n");
    if (resp.rfind("HTTP/1.", 0) != 0 || resp.size() < 12 || resp.compare(9, 3, "200") != 0 || body == std::string::npos)
        throw std::runtime_error("GET " + path + ": phản hồi không phải 200");
    return resp.substr(body + 4);
}

}  // namespace netsec
