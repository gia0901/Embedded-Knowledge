// RAII cho handle của OpenSSL và cách biến error queue thành exception có nghĩa.
#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

namespace netsec {

using Bytes = std::vector<unsigned char>;

// Deleter<SSL_CTX_free>, Deleter<EVP_PKEY_free>…: không bao giờ gọi *_free bằng tay
template <auto Fn>
struct Deleter {
    template <class T>
    void operator()(T* p) const noexcept { Fn(p); }
};
using SslPtr = std::unique_ptr<SSL, Deleter<SSL_free>>;
using X509Ptr = std::unique_ptr<X509, Deleter<X509_free>>;
using PkeyPtr = std::unique_ptr<EVP_PKEY, Deleter<EVP_PKEY_free>>;
using PkeyCtxPtr = std::unique_ptr<EVP_PKEY_CTX, Deleter<EVP_PKEY_CTX_free>>;
using MdCtxPtr = std::unique_ptr<EVP_MD_CTX, Deleter<EVP_MD_CTX_free>>;
using CipherCtxPtr = std::unique_ptr<EVP_CIPHER_CTX, Deleter<EVP_CIPHER_CTX_free>>;

// Đọc hết error queue (riêng cho từng thread). Không đọc thì lỗi cũ sẽ hiện ra ở lần gọi sau.
inline std::string ossl_errors() {
    std::string out;
    char buf[256];
    for (unsigned long e = ERR_get_error(); e != 0; e = ERR_get_error()) {
        ERR_error_string_n(e, buf, sizeof buf);
        out += "\n  ";
        out += buf;
    }
    return out;
}

[[noreturn]] inline void throw_ossl(const std::string& what) { throw std::runtime_error(what + ossl_errors()); }

// Phần lớn API của OpenSSL trả 1 khi thành công. Không dùng if (rc): có hàm trả -1 khi lỗi.
inline void check(int rc, const char* what) {
    if (rc != 1) throw_ossl(what);
}

template <class T>
T* check_ptr(T* p, const char* what) {
    if (p == nullptr) throw_ossl(what);
    return p;
}

}  // namespace netsec
