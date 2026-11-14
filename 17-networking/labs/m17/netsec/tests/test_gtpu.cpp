#include <vector>

#include "check.hpp"
#include "netsec/gtpu.hpp"

using netsec::parse_gtpu;

static bool parses(const std::vector<std::uint8_t>& p) { return parse_gtpu(p.data(), p.size()).has_value(); }

int main() {
    std::cout << "Packet hợp lệ\n";
    const std::vector<std::uint8_t> simple = {0x30, 0xff, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x45, 0x00, 0x00, 0x00};
    const auto s = parse_gtpu(simple.data(), simple.size());
    CHECK_EQ(s.has_value(), true);
    CHECK_EQ(s->teid, 1u);
    CHECK_EQ(s->payload_offset, std::size_t{8});
    // E và S bật, sequence 7, một extension PDU Session Container (0x85) dài 4 byte, payload 2 byte
    const std::vector<std::uint8_t> ext = {0x36, 0xff, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x07, 0x00, 0x85,
                                           0x01, 0x10, 0x00, 0x00, 0xab, 0xcd};
    const auto e = parse_gtpu(ext.data(), ext.size());
    CHECK_EQ(e.has_value(), true);
    CHECK_EQ(e->teid, 42u);
    CHECK_EQ(e->seq.value_or(0), 7);
    CHECK_EQ(int(e->ext_types.size()), 1);
    CHECK_EQ(int(e->ext_types.at(0)), 0x85);
    CHECK_EQ(e->payload_offset, std::size_t{16});

    std::cout << "Packet hỏng phải bị từ chối (regression từ fuzzer)\n";
    std::vector<std::uint8_t> no_opt(ext.begin(), ext.begin() + 8);
    no_opt[3] = 0x00;                                             // flag bật nhưng không có 4 byte tuỳ chọn
    CHECK_EQ(parses(no_opt), false);
    auto long_len = simple;
    long_len[3] = 0x20;                                           // khai báo 32 byte, thật chỉ có 4
    CHECK_EQ(parses(long_len), false);
    auto zero_ext = ext;
    zero_ext[12] = 0x00;                                          // extension dài 0: bản lỗi lặp vô hạn
    CHECK_EQ(parses(zero_ext), false);
    auto big_ext = ext;
    big_ext[12] = 0xff;                                           // extension dài 1020 byte
    CHECK_EQ(parses(big_ext), false);
    return test_result();
}
