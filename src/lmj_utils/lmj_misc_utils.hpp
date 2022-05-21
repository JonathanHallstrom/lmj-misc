#pragma once

#include <bit>
#include "lmj_concepts.hpp"
#include "../lmj_containers/lmj_containers.hpp"

namespace lmj {
    template<class T> requires integral<T>
    class constexpr_rand_generator { // based on xorshift random number generator by George Marsaglia
        unsigned long long x = 230849599040350201, y = 965937400815267857, z = 895234450760720011;

    public:
        constexpr constexpr_rand_generator() = default;

        constexpr explicit constexpr_rand_generator(T seed) {
            x ^= std::rotr(static_cast<unsigned long long>(seed), 1);
            y ^= std::rotr(static_cast<unsigned long long>(seed), 2);
            z ^= std::rotr(static_cast<unsigned long long>(seed), 3);
        }

        constexpr auto operator()() {
            x ^= x << 16;
            x ^= x >> 5;
            x ^= x << 1;

            auto t = x;
            x = y;
            y = z;
            z = t ^ x ^ y;

            return static_cast<T>(z);
        }
    };

    constexpr int sign(number auto &&x) {
        return (x > 0) - (x < 0);
    }

    template<class T = unsigned long long, unsigned long long seed = 0>
    requires integral<T>
    auto rand() {
        constinit static constexpr_rand_generator<T> gen{seed};
        return gen();
    }

    constexpr auto min(number auto &&a, number auto &&b) noexcept -> decltype(a + b) {
        return a < b ? a : b;
    }

    constexpr auto max(number auto &&a, number auto &&b) noexcept -> decltype(a + b) {
        return a > b ? a : b;
    }

    constexpr auto min(number auto &&a, numbers auto &&... b) noexcept {
        return min(a, min(b...));
    }

    constexpr auto max(number auto &&a, numbers auto &&... b) noexcept {
        return max(a, max(b...));
    }

    // tests (very incomplete)

    static_assert(min(1, 2) == 1);
    static_assert(min(1, 2, 3) == 1);

    static_assert(max(1, 2) == 2);
    static_assert(max(1, 2, 3) == 3);

    static_assert(sign(0) == 0);
    static_assert(sign(1) == 1);
    static_assert(sign(-1) == -1);
}