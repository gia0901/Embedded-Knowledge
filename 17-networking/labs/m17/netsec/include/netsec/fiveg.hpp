#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>

#include "netsec/ossl.hpp"

namespace netsec {

struct MilenageOut {
    Bytes mac_a, mac_s, res, ck, ik, ak, ak_s;   // f1, f1*, f2, f3, f4, f5, f5*
};
Bytes opc_from_op(const Bytes& k, const Bytes& op);
MilenageOut milenage(const Bytes& k, const Bytes& opc, const Bytes& rand, const Bytes& sqn, const Bytes& amf);

struct HeAv {
    Bytes rand, autn, xres_star, kausf;          // 5G HE AV do ARPF (UDM) tạo
};
HeAv generate_he_av(const Bytes& k, const Bytes& opc, const Bytes& sqn, const Bytes& amf, const Bytes& rand,
                    const std::string& snn);
Bytes hxres_star(const Bytes& rand, const Bytes& xres_star);    // AUSF gửi cho SEAF
Bytes kseaf_from_kausf(const Bytes& kausf, const std::string& snn);

struct AuthError : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct UeResult {
    Bytes res_star, kseaf;
};
// Phía USIM/UE. sqn_ms: SQN cao nhất đã chấp nhận, được cập nhật khi thành công.
UeResult usim_authenticate(const Bytes& k, const Bytes& opc, const Bytes& rand, const Bytes& autn, const std::string& snn,
                           std::uint64_t& sqn_ms);

// SIDF: scheme output = ephemeral public key (32) ‖ ciphertext ‖ MAC (8) → MSIN
std::string suci_profile_a_deconceal(const Bytes& hn_private_raw, const Bytes& scheme_output);

}  // namespace netsec
