#include <cstdint>

#include "check.hpp"
#include "netsec/crypto.hpp"
#include "netsec/fiveg.hpp"

using namespace netsec;

int main() {
    const Bytes K = from_hex("465b5ce8b199b49faa5f0a2ee238a6bc");
    const Bytes OPC = opc_from_op(K, from_hex("cdc202d5123e20f62b6d676ac72cb318"));
    const Bytes RAND = from_hex("23553cbe9637a89d218ae64dae47bf35");
    const Bytes SQN = from_hex("ff9bb4d0b607"), AMF = from_hex("b9b9");
    const std::string SNN = "5G:mnc004.mcc452.3gppnetwork.org";

    std::cout << "Milenage Test Set 1 (3GPP TS 35.208)\n";
    CHECK_EQ(to_hex(OPC), "cd63cb71954a9f4e48a5994e37a02baf");
    const MilenageOut m = milenage(K, OPC, RAND, SQN, AMF);
    CHECK_EQ(to_hex(m.mac_a), "4a9ffac354dfafb3");
    CHECK_EQ(to_hex(m.mac_s), "01cfaf9ec4e871e9");
    CHECK_EQ(to_hex(m.res), "a54211d5e3ba50bf");
    CHECK_EQ(to_hex(m.ck), "b40ba9a3c58b2a05bbf0d987b21bf8cb");
    CHECK_EQ(to_hex(m.ik), "f769bcd751044604127672711c6d3441");
    CHECK_EQ(to_hex(m.ak), "aa689c648370");
    CHECK_EQ(to_hex(m.ak_s), "451e8beca43b");

    std::cout << "5G-AKA (cùng giá trị với Lab 15.3 và widget module 15)\n";
    const HeAv av = generate_he_av(K, OPC, SQN, AMF, RAND, SNN);
    CHECK_EQ(to_hex(av.autn), "55f328b43577b9b94a9ffac354dfafb3");
    CHECK_EQ(to_hex(av.xres_star), "a95d06cbfcc4eb30f2ef2f60089c5778");
    CHECK_EQ(to_hex(hxres_star(RAND, av.xres_star)), "a418f491067e7116695e560ebac36083");
    CHECK_EQ(to_hex(av.kausf), "32f3afc00d86a29bc8c929a7cc77053e386ab5977da13e5f00ce7e9acfaa6f24");
    CHECK_EQ(to_hex(kseaf_from_kausf(av.kausf, SNN)), "1070b0681638cff666490a2e76768380c98122d3bcb6c6cab2399b302b67d06c");

    std::cout << "USIM: hợp lệ, replay, mạng giả, sai serving network name\n";
    std::uint64_t sqn_ms = 0;
    const UeResult ue = usim_authenticate(K, OPC, RAND, av.autn, SNN, sqn_ms);
    CHECK_EQ(to_hex(ue.res_star), to_hex(av.xres_star));
    CHECK_EQ(to_hex(ue.kseaf), to_hex(kseaf_from_kausf(av.kausf, SNN)));
    CHECK_THROWS(usim_authenticate(K, OPC, RAND, av.autn, SNN, sqn_ms));          // replay: SQN không mới
    Bytes fake_k = K;
    fake_k[0] ^= 0x01;
    std::uint64_t fresh = 0;
    CHECK_THROWS(usim_authenticate(K, OPC, RAND, generate_he_av(fake_k, OPC, SQN, AMF, RAND, SNN).autn, SNN, fresh));
    std::uint64_t fresh2 = 0;
    const UeResult other = usim_authenticate(K, OPC, RAND, av.autn, "5G:mnc099.mcc452.3gppnetwork.org", fresh2);
    CHECK_EQ(equal_ct(other.res_star, av.xres_star), false);                      // AUSF sẽ từ chối
    return test_result();
}
