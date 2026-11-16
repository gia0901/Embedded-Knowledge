// parse1.cpp: các hàm nhỏ quanh parse() để đọc assembly. Không có main; dùng với -c hoặc -S.
// Build: g++ -O2 -std=c++20 -I.. -c parse1.cpp && objdump -d --no-show-raw-insn -C parse1.o
#include "frame.h"

// 1. Kiểu "out-parameter": trả bool, ghi kết quả vào Frame& của người gọi
bool parse_out(const uint8_t* p, Frame& f) { return parse(p, f); }

// 2. Trả struct 12 byte theo giá trị: ABI cho phép trả trong hai thanh ghi rax:rdx
Frame parse_ret(const uint8_t* p) { Frame f{}; parse(p, f); return f; }

// 3. Chỉ đọc trường value (big-endian) ở offset 12
int32_t read_value(const uint8_t* p) { uint32_t v; std::memcpy(&v, p + 12, 4); return int32_t(ntohl(v)); }
