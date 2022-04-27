#pragma once

#include <type_traits>
#include <functional>

namespace lmj {
    template<class T>
    concept Number = std::is_integral_v<std::remove_const_t<std::remove_reference_t<T>>> ||
                     std::is_floating_point_v<std::remove_const_t<std::remove_reference_t<T>>>;

    template<class T> concept iterable = requires(T x) {
        std::begin(x);
        std::end(x);
    };

    template<class T> concept not_iterable = !iterable<T>;

    template<class T, class... G>
    constexpr bool all_number_helper() {
        if constexpr(sizeof...(G))
            return Number<T> && all_number_helper<G...>();
        else
            return Number<T>;
    }

    template<class... T> concept Numbers = all_number_helper<T...>();
}