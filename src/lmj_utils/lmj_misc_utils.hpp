#pragma once

#include "../lmj_containers/lmj_containers.hpp"

namespace lmj {
    template<class T, typename = typename std::enable_if_t<std::is_integral_v<T>, void>>
    class constexpr_rand_generator { // based on xorshift random number generator by George Marsaglia
        unsigned long long x = 230849599040350201, y = 965937400815267857, z = 895234450760720011;

    public:
        constexpr constexpr_rand_generator() = default;

        constexpr explicit constexpr_rand_generator(T seed) {
            x ^= static_cast<unsigned long long>(seed);
            y ^= static_cast<unsigned long long>(seed);
            z ^= static_cast<unsigned long long>(seed);
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

    template<class T>
    constexpr int sign(T &&x) {
        return (x > 0) - (x < 0);
    }

    template<class T = unsigned long long, unsigned long long seed = 0, typename = typename std::enable_if_t<std::is_integral_v<T>, void>>
    auto rand() {
        static constexpr_rand_generator<T> gen{seed};
        return gen();
    }

    template<class T, class G>
    constexpr auto min(T &&a, G &&b) noexcept -> decltype(a + b) {
        return a < b ? a : b;
    }

    template<class T, class G>
    constexpr auto max(T &&a, G &&b) noexcept -> decltype(a + b) {
        return a > b ? a : b;
    }

    template<class T, class... G>
    constexpr auto min(T &&a, G &&... b) noexcept {
        return min(a, min(b...));
    }

    template<class T, class... G>
    constexpr auto max(T &&a, G &&... b) noexcept {
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