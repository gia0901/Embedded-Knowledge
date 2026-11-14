// clang++ -std=c++17 -g -O1 -fsanitize=fuzzer,address,undefined -Iinclude fuzz/fuzz_gtpu.cpp src/gtpu.cpp -o fuzz_gtpu
#include "netsec/gtpu.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (auto h = netsec::parse_gtpu(data, size)) {
        // Bất biến: payload nằm trong packet và trong phần length đã khai báo
        if (h->payload_offset > size || h->payload_offset > 8u + h->length) __builtin_trap();
    }
    return 0;
}
