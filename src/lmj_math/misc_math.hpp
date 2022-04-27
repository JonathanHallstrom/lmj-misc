#pragma once

#include "newton_raphson.hpp"
#include <cmath>
#include <ranges>
#include <cassert>

namespace lmj {
    /**
     * @param base
     * @param exp
     * @throws std::out_of_range if exp = base = 0
     * @return base ^ exp
     */
    constexpr auto ipow(auto base, unsigned exp) -> decltype(base) {
        if (exp == 1)
            return base;
        if (exp == 0 && base != 0.0)
            return 1;
        if (exp == 0 && base == 0)
            throw std::out_of_range("0^0 is undefined");
        decltype(base) result = 1;
        while (exp) {
            result *= 1 + (exp & 1) * (base - 1);
            exp >>= 1;
            base *= base;
        }
        return result;
    }

    /**
     * @param x
     * @return |x|
     */
    constexpr auto abs(auto &&x) {
        return x > 0 ? x : -x;
    }

#if defined(__GNUC__)
    using biggest_float = __float128;
#else
    using biggest_float = long double;
#endif

    constexpr biggest_float _exp_small(biggest_float x) {
        biggest_float sum = 1.0l;
        int n = 32;
        while (--n)
            sum = 1.0l + x * sum / (biggest_float) n;
        return sum;
    }

    /**
     * @brief only use if std::exp is not available for compile time programming, this is much slower than std::exp
     * @param x
     * @return e ^ x
     */
    constexpr long double exp(long double x) {
        assert(-11356 <= x && x <= 11356);
        if (x == 0)
            return 1;
        if (x < 0)
            return 1.0l / lmj::exp(-x);
        if (x > 1) {
            constexpr auto e = _exp_small(1);
            const auto whole_part = unsigned(x);
            const auto fractional_part = x - whole_part;
            return (long double) (ipow(e, whole_part) * _exp_small(fractional_part));
        }
        // only if x >= 0 and x <= 1
        return (long double) _exp_small(x);
    }

    /**
     * @param x
     * @throws std::out_of_range if x < 0
     * @return square root of x
     */
    constexpr auto sqrt(Number auto && x) {
        if (x < 0)
            throw std::out_of_range("can't take square root of negative number");
        long double root = x;
        long double dx = x;
        do {
            dx = (root * root - x) / (2 * root);
            root -= dx;
        } while (dx > 1e-5 || -dx > 1e-5);
        auto extra_iterations = 4;
        while (extra_iterations--)
            root -= (root * root - x) / (2 * root);
        return root;
    }

    constexpr auto sum_squares(Number auto &&x) {
        return x * x;
    }

    constexpr auto sum_squares(Number auto &&x, Numbers auto &&... pack) {
        return x * x + sum_squares(pack...);
    }

    constexpr auto hypot(Numbers auto &&... v) {
        return lmj::sqrt(sum_squares(v...));
    }

    template<class T = unsigned long long>
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
}