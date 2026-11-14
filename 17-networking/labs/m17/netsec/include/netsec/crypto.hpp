#pragma once
#include <cstddef>
#include <initializer_list>
#include <string>

#include "netsec/ossl.hpp"

namespace netsec {

Bytes from_hex(const std::string& hex);
std::string to_hex(const Bytes& b);
std::string b64url_encode(const Bytes& b);            // không padding, như JWT
Bytes b64url_decode(const std::string& s);            // throw std::invalid_argument nếu ký tự lạ

Bytes sha256(const Bytes& data);
Bytes hmac_sha256(const Bytes& key, const Bytes& data);
Bytes kdf_3gpp(const Bytes& key, unsigned char fc, std::initializer_list<Bytes> params);   // TS 33.220 Annex B
Bytes x963_kdf_sha256(const Bytes& z, const Bytes& shared_info, std::size_t len);          // ANSI X9.63, cho SUCI
Bytes aes128_ecb_block(const Bytes& key, const Bytes& block);                               // một block, cho Milenage
Bytes aes128_ctr(const Bytes& key, const Bytes& iv, const Bytes& data);

bool equal_ct(const Bytes& a, const Bytes& b);        // so sánh constant-time
void wipe(Bytes& secret);                             // OPENSSL_cleanse rồi xoá

}  // namespace netsec
