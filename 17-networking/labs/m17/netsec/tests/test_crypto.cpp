#include <cstring>

#include "check.hpp"
#include "netsec/crypto.hpp"

using namespace netsec;

static Bytes str(const char* s) { return Bytes(s, s + std::strlen(s)); }

int main() {
    std::cout << "SHA-256, HMAC (RFC 4231), AES (FIPS-197)\n";
    CHECK_EQ(to_hex(sha256(str("abc"))), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK_EQ(to_hex(hmac_sha256(Bytes(20, 0x0b), str("Hi There"))),
             "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
    CHECK_EQ(to_hex(hmac_sha256(str("Jefe"), str("what do ya want for nothing?"))),
             "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
    CHECK_EQ(to_hex(aes128_ecb_block(from_hex("000102030405060708090a0b0c0d0e0f"),
                                     from_hex("00112233445566778899aabbccddeeff"))),
             "69c4e0d86a7b0430d8cdb78070b4c55a");

    std::cout << "base64url\n";
    CHECK_EQ(b64url_encode(Bytes{0xfb, 0xff}), "-_8");
    CHECK_EQ(to_hex(b64url_decode("-_8")), "fbff");
    CHECK_THROWS(b64url_decode("ab+c"));

    std::cout << "Kiểm tra đầu vào và so sánh constant-time\n";
    CHECK_EQ(equal_ct(str("abc"), str("abc")), true);
    CHECK_EQ(equal_ct(str("abc"), str("abd")), false);
    CHECK_EQ(equal_ct(str("abc"), str("ab")), false);
    CHECK_THROWS(aes128_ecb_block(Bytes(15), Bytes(16)));
    Bytes secret = str("k");
    wipe(secret);
    CHECK_EQ(secret.size(), std::size_t{0});
    return test_result();
}
