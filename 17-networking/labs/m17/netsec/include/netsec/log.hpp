#pragma once
#include <string>

#include <nlohmann/json.hpp>

namespace netsec {
// Audit log: mỗi sự kiện một dòng JSON trên stdout (kubectl logs … | jq), cùng định dạng với capstone
void log_json(const std::string& event, nlohmann::json fields = nlohmann::json::object());
}
