#pragma once

#include "concepts.hpp"

namespace lmj {
    int sign(number auto &&x) {
        return (x > 0) - (x < 0);
    }

    template<class T = unsigned long long>
    requires integral<T>
    auto rand() { // based on xorshift random number generator by George Marsaglia
        static unsigned long long x = 230849599040350201, y = 965937400815267857, z = 895234450760720011;
        unsigned long long t;
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;

        return (T) z;
    }

    constexpr auto min(number auto const &a, number auto const &b) noexcept -> decltype(a + b) {
        return a < b ? a : b;
    }

    constexpr auto max(number auto const &a, number auto const &b) noexcept -> decltype(a + b) {
        return a > b ? a : b;
    }

    constexpr auto min(number auto const &a, numbers auto const &... b) noexcept {
        return min(a, min(b...));
    }

    constexpr auto max(number auto const &a, numbers auto const &... b) noexcept {
        return max(a, max(b...));
    }
}