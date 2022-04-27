#pragma once

#include <type_traits>
#include <functional>

namespace lmj {
    template<class T>
    concept number = std::is_integral_v<std::remove_reference_t<std::remove_const_t<T>>> ||
                     std::is_floating_point_v<std::remove_reference_t<std::remove_const_t<T>>>;

    template<class T> concept iterable = requires(T x) {
        std::begin(x);
        std::end(x);
    };

    template<class T> concept not_iterable = !iterable<T>;

    template<class T, class... G>
    constexpr bool number_helper() {
        if constexpr (sizeof...(G)) {
            return number<T> && number_helper<G...>();
        } else {
            return number<T>;
        }
    }

    template<class... T> concept numbers = number_helper<T...>();
}