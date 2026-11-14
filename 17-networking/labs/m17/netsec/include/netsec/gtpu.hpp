#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace netsec {

struct GtpuHeader {
    std::uint8_t version = 0, pt = 0, flags = 0, msg_type = 0;
    std::uint16_t length = 0;                    // số byte sau 8 byte header bắt buộc
    std::uint32_t teid = 0;
    std::optional<std::uint16_t> seq;
    std::vector<std::uint8_t> ext_types;         // ví dụ 0x85: PDU Session Container
    std::size_t payload_offset = 0;              // vị trí bắt đầu packet IP của UE
};

// Không bao giờ đọc ngoài [data, data + len); input hỏng → std::nullopt
std::optional<GtpuHeader> parse_gtpu(const std::uint8_t* data, std::size_t len);

}  // namespace netsec
