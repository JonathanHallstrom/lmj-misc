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
    constexpr auto ipow(auto base, std::uint64_t exp) -> decltype(base) {
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

#ifdef __GNUC__
    using biggest_float = __float128;
#else
    using biggest_float = long double;
#endif

    constexpr biggest_float _exp_small(biggest_float x, int n = 32) {
        biggest_float sum = 1.0l;
        while (--n)
            sum = 1.0l + x * sum / (biggest_float) n;
        return sum;
    }

    /**
     * @brief only use if std::exp is not available for constexpr, this is much slower than std::exp
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
            const auto whole_part = static_cast<unsigned>(x);
            const auto fractional_part = x - static_cast<long double>(whole_part);
            return static_cast<long double>(ipow(e, whole_part) * _exp_small(fractional_part));
        }
        // only if x >= 0 and x <= 1
        return static_cast<long double>(_exp_small(x));
    }

    /**
     * @param x
     * @throws std::out_of_range if x < 0
     * @return square root of x
     */
    constexpr auto sqrt(number auto &&x) {
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

    constexpr auto sum_squares(number auto &&x) {
        return x * x;
    }

    constexpr auto sum_squares(number auto &&x, numbers auto &&... nums) {
        return x * x + sum_squares(nums...);
    }

    constexpr auto hypot(numbers auto &&... nums) {
        return lmj::sqrt(sum_squares(nums...));
    }

    /**
     * @param f integrand
     * @param low lower limit of integration
     * @param high upper limit of integration
     * @param steps number of steps between limits
     * @return integral of f from low to high
     */
    constexpr auto integrate(auto &&f, long double low, long double high, int steps = 1e6) {
        long double sum = 0;
        long double last_y = f(low);
        const long double step_size = (high - low) / static_cast<long double>(steps);
        for (int step = 1; step < steps; ++step) {
            const long double y = f(low + step_size * step);
            sum += y + last_y;
            last_y = y;
        }
        return sum * step_size / 2.0l;
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