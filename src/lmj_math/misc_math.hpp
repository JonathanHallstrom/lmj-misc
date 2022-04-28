#pragma once

#include "newton_raphson.hpp"
#include "../lmj_containers/container_helpers.hpp"
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
    constexpr auto ipow(auto base, integral auto exp) -> decltype(base) {
        if constexpr (signed_integral<decltype(exp)>)
            assert(exp >= 0);
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
    constexpr auto sqrt(number auto &&x) {
        if (x < 0)
            throw std::out_of_range("can't take square root of negative number");
        return newtons_method([=](long double _x) { return _x * _x - x; });
    }

    constexpr auto sum_squares(number auto &&x) {
        return x * x;
    }

    constexpr auto sum_squares(number auto &&x, numbers auto &&... pack) {
        return x * x + sum_squares(pack...);
    }

    constexpr auto hypot(numbers auto &&... v) {
        return lmj::sqrt(sum_squares(v...));
    }

    constexpr auto sigma(std::uint64_t n) {
        std::uint64_t res = 1;
        for (std::uint64_t p = 2; p * p <= n; p += 1 + (p & 1)) {
            std::uint64_t a = 0;
            while (n % p == 0)
                n /= p, ++a;
            if (a)
                res *= (ipow(p, a + 1) - 1) / (p - 1);
        }
        if (n > 1)
            res *= n + 1;
        return res;
    }

    constexpr auto euler_totient(std::uint64_t n) {
        std::uint64_t res = n;
        for (std::uint64_t p = 2; p * p <= n; p += 1 + (p & 1)) {
            std::uint64_t a = 0;
            while (n % p == 0)
                n /= p, ++a;
            if (a)
                res /= p, res *= p - 1;
        }
        if (n > 1)
            res /= n, res *= n - 1;
        return res;
    }
}