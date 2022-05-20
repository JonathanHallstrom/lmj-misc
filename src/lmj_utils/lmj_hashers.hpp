#pragma once

#include <type_traits>

namespace lmj {
    template<class T>
    struct hash {
        constexpr auto operator()(T x) const -> std::enable_if_t<std::is_integral_v<T>, T> {
            // based on xorshift random number generators by George Marsaglia
            if constexpr (sizeof(T) > 2) // if T is a short or char this is UB
                x ^= x << 16;
            x ^= x >> 5;
            x ^= x << 1;
            return x;
        }
    };
}