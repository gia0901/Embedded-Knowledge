// Khung test tối giản, không cần thư viện ngoài
#pragma once
#include <exception>
#include <iostream>

inline int g_failures = 0;

#define CHECK_EQ(actual, expected)                                                              \
    do {                                                                                        \
        auto a_ = (actual);                                                                     \
        auto e_ = (expected);                                                                   \
        if (a_ == e_) {                                                                         \
            std::cout << "  ✔ " #actual "\n";                                                  \
        } else {                                                                                \
            std::cout << "  ✘ " #actual "\n      nhận:     " << a_ << "\n      mong đợi: " << e_ << "\n"; \
            ++g_failures;                                                                       \
        }                                                                                       \
    } while (0)

#define CHECK_THROWS(expr)                                                                      \
    do {                                                                                        \
        bool t_ = false;                                                                        \
        try { (void)(expr); } catch (const std::exception&) { t_ = true; }                      \
        if (t_) std::cout << "  ✔ throw: " #expr "\n";                                          \
        else { std::cout << "  ✘ không throw: " #expr "\n"; ++g_failures; }                     \
    } while (0)

inline int test_result() {
    std::cout << (g_failures ? "THẤT BẠI\n" : "OK\n");
    return g_failures ? 1 : 0;
}
