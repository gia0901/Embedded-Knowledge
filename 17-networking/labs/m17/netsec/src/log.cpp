#include "netsec/log.hpp"

#include <ctime>
#include <iostream>
#include <mutex>

namespace netsec {

void log_json(const std::string& event, nlohmann::json fields) {
    char ts[32];
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&t, &tm);
    std::strftime(ts, sizeof ts, "%Y-%m-%dT%H:%M:%SZ", &tm);
    nlohmann::json line = {{"ts", ts}, {"event", event}};
    if (fields.is_object()) line.update(fields);
    static std::mutex mu;
    std::lock_guard<std::mutex> lock(mu);
    // replace: dữ liệu từ client có thể không phải UTF-8 hợp lệ, không được làm dump() throw
    std::cout << line.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace) << std::endl;
}

}  // namespace netsec
