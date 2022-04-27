#pragma once

#include "../lmj_utils/concepts.hpp"
#include <limits>

namespace lmj {
    template<class Func, class T = long double>
    requires number<T>
    constexpr auto derivative(Func &&f, T h = 1e-5l) {
        return [=](auto x) {
            return (f(x + h) - f(x - h)) / (2 * h);
        };
    }

    template<class Func, class T = long double>
    requires number<T>
    constexpr T newtons_method(Func &&f, T x = 2.0l, T epsilon = 1e-5l) {
        const auto f_prim = derivative(f);
        long double dx;
        do {
            dx = f(x) / f_prim(x);
            x -= dx;
        } while (dx > epsilon || -dx > epsilon);

        auto extra_iters = 4;
        while (extra_iters--) {
            dx = f(x) / f_prim(x);
            x -= dx;
        }
        return x;
    }
}