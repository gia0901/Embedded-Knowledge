// suci_check <hn_private_key_hex> <scheme_output_hex>: giải SUCI Profile A như SIDF trong UDM
#include <iostream>

#include "netsec/crypto.hpp"
#include "netsec/fiveg.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Dùng: suci_check <hn_private_key_hex> <scheme_output_hex>\n";
        return 2;
    }
    try {
        std::cout << "MSIN = " << netsec::suci_profile_a_deconceal(netsec::from_hex(argv[1]), netsec::from_hex(argv[2])) << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cout << "lỗi: " << e.what() << "\n";
        return 1;
    }
}
