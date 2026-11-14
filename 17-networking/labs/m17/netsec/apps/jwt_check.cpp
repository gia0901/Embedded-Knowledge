// jwt_check <jwks.json> <tokens.txt>: mỗi dòng "tên kết_quả_mong_đợi token". So kết quả của C++ với kỳ vọng.
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "netsec/jwt.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Dùng: jwt_check jwks.json tokens.txt\n";
        return 2;
    }
    std::ifstream jf(argv[1]);
    std::stringstream buf;
    buf << jf.rdbuf();
    const std::string jwks_body = buf.str();
    netsec::JwksCache cache([&] { return jwks_body; });
    const netsec::VerifyOptions opt{"https://token.security.svc.cluster.local:8443", "orders-api", 30};

    std::ifstream in(argv[2]);
    std::string name, expect, token;
    int failed = 0;
    while (in >> name >> expect >> token) {
        std::string got, detail;
        try {
            const auto claims = netsec::verify_es256(token, cache, opt);
            got = "ok";
            detail = claims.value("scope", "");
        } catch (const netsec::JwtError& e) {
            got = "reject";
            detail = e.what();
        }
        const bool pass = got == expect;
        failed += pass ? 0 : 1;
        std::cout << (pass ? "  ✔ " : "  ✘ ") << std::left << std::setw(16) << name << std::setw(8) << got << detail << "\n";
    }
    std::cout << (failed ? "THẤT BẠI\n" : "OK: C++ cho cùng kết quả với kỳ vọng\n");
    return failed ? 1 : 0;
}
